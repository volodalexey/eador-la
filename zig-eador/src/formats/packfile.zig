//! Allegro 4 packfile decoder (pack_fopen with "wp": "slh!" + LZSS), the
//! format of Eador's saves (save\game%d.sav, map%d.map, ...).
//! LZSS: 4096-byte window, matches of 3..18 bytes, the window starts at
//! N − F with zeroes before it. Files without the magic are stored as is.
const std = @import("std");

pub const magic = "slh!";
const N = 4096;
const F = 18;
const threshold = 2;

/// Returns the decoded bytes (owned by `gpa`).
pub fn unpack(gpa: std.mem.Allocator, data: []const u8) ![]u8 {
    if (!std.mem.startsWith(u8, data, magic)) return gpa.dupe(u8, data);
    const src = data[magic.len..];
    var out: std.ArrayList(u8) = .empty;
    errdefer out.deinit(gpa);
    try out.ensureTotalCapacity(gpa, src.len * 4);
    var buf: [N]u8 = @splat(0);
    var r: usize = N - F;
    var p: usize = 0;
    var flags: u32 = 0;
    while (true) {
        flags >>= 1;
        if (flags & 0x100 == 0) {
            if (p >= src.len) break;
            flags = @as(u32, src[p]) | 0xFF00;
            p += 1;
        }
        if (flags & 1 != 0) {
            if (p >= src.len) break;
            const c = src[p];
            p += 1;
            try out.append(gpa, c);
            buf[r] = c;
            r = (r + 1) & (N - 1);
        } else {
            if (p + 1 >= src.len) break;
            const i: usize = @as(usize, src[p]) | (@as(usize, src[p + 1] & 0xF0) << 4);
            const j: usize = @as(usize, src[p + 1] & 0x0F) + threshold;
            p += 2;
            for (0..j + 1) |k| {
                const c = buf[(i + k) & (N - 1)];
                try out.append(gpa, c);
                buf[r] = c;
                r = (r + 1) & (N - 1);
            }
        }
    }
    return out.toOwnedSlice(gpa);
}

/// Allegro's lzss_write (src/lzss.c: Okumura's LZSS with binary search
/// trees) over the whole buffer, after the magic: the same bytes
/// pack_fopen(name, "wp") gives for the game and map files.
pub fn packAllegro(gpa: std.mem.Allocator, data: []const u8) ![]u8 {
    const Z = struct {
        text: [N + F - 1]u8 = @splat(0),
        lson: [N + 1]u32 = @splat(N),
        rson: [N + 257]u32 = @splat(N),
        dad: [N + 1]u32 = @splat(N),
        match_position: u32 = 0,
        match_length: u32 = 0,

        fn insert(z: *@This(), r: u32) void {
            var cmp: i32 = 1;
            var p: u32 = N + 1 + @as(u32, z.text[r]);
            z.rson[r] = N;
            z.lson[r] = N;
            z.match_length = 0;
            while (true) {
                if (cmp >= 0) {
                    if (z.rson[p] != N) p = z.rson[p] else {
                        z.rson[p] = r;
                        z.dad[r] = p;
                        return;
                    }
                } else {
                    if (z.lson[p] != N) p = z.lson[p] else {
                        z.lson[p] = r;
                        z.dad[r] = p;
                        return;
                    }
                }
                var i: u32 = 1;
                while (i < F) : (i += 1) {
                    cmp = @as(i32, z.text[r + i]) - @as(i32, z.text[p + i]);
                    if (cmp != 0) break;
                }
                if (i > z.match_length) {
                    z.match_position = p;
                    z.match_length = i;
                    if (i >= F) break;
                }
            }
            z.dad[r] = z.dad[p];
            z.lson[r] = z.lson[p];
            z.rson[r] = z.rson[p];
            z.dad[z.lson[p]] = r;
            z.dad[z.rson[p]] = r;
            if (z.rson[z.dad[p]] == p) z.rson[z.dad[p]] = r else z.lson[z.dad[p]] = r;
            z.dad[p] = N;
        }

        fn delete(z: *@This(), p: u32) void {
            if (z.dad[p] == N) return;
            var q: u32 = undefined;
            if (z.rson[p] == N) q = z.lson[p] else if (z.lson[p] == N) q = z.rson[p] else {
                q = z.lson[p];
                if (z.rson[q] != N) {
                    while (z.rson[q] != N) q = z.rson[q];
                    z.rson[z.dad[q]] = z.lson[q];
                    z.dad[z.lson[q]] = z.dad[q];
                    z.lson[q] = z.lson[p];
                    z.dad[z.lson[p]] = q;
                }
                z.rson[q] = z.rson[p];
                z.dad[z.rson[p]] = q;
            }
            z.dad[q] = z.dad[p];
            if (z.rson[z.dad[p]] == p) z.rson[z.dad[p]] = q else z.lson[z.dad[p]] = q;
            z.dad[p] = N;
        }
    };
    const z = try gpa.create(Z);
    defer gpa.destroy(z);
    z.* = .{};
    var out: std.ArrayList(u8) = .empty;
    errdefer out.deinit(gpa);
    try out.ensureTotalCapacity(gpa, data.len / 2 + 16);
    try out.appendSlice(gpa, magic);

    var code: [17]u8 = undefined;
    code[0] = 0;
    var code_ptr: usize = 1;
    var mask: u8 = 1;
    var s: u32 = 0;
    var r: u32 = N - F;
    var pos: usize = 0;
    var len: u32 = 0;
    while (len < F and pos < data.len) : (len += 1) {
        z.text[r + len] = data[pos];
        pos += 1;
    }
    if (len == 0) return out.toOwnedSlice(gpa);
    var i: u32 = 1;
    while (i <= F) : (i += 1) z.insert(r - i);
    z.insert(r);
    while (true) {
        if (z.match_length > len) z.match_length = len;
        if (z.match_length <= threshold) {
            z.match_length = 1;
            code[0] |= mask;
            code[code_ptr] = z.text[r];
            code_ptr += 1;
        } else {
            code[code_ptr] = @truncate(z.match_position);
            code[code_ptr + 1] = @truncate(((z.match_position >> 4) & 0xF0) | (z.match_length - (threshold + 1)));
            code_ptr += 2;
        }
        mask <<= 1;
        if (mask == 0) {
            try out.appendSlice(gpa, code[0..code_ptr]);
            code[0] = 0;
            code_ptr = 1;
            mask = 1;
        }
        const last_match_length = z.match_length;
        i = 0;
        while (i < last_match_length and pos < data.len) : (i += 1) {
            const c = data[pos];
            pos += 1;
            z.delete(s);
            z.text[s] = c;
            if (s < F - 1) z.text[s + N] = c;
            s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
            z.insert(r);
        }
        while (i < last_match_length) : (i += 1) {
            z.delete(s);
            s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
            len -= 1;
            if (len != 0) z.insert(r);
        }
        if (len == 0) break;
    }
    if (code_ptr > 1) try out.appendSlice(gpa, code[0..code_ptr]);
    return out.toOwnedSlice(gpa);
}

/// Pack a small object using literal LZSS tokens.  The original writer may
/// use back-references, but literal tokens are fully compatible and keep the
/// profile writer deterministic.
pub fn pack(gpa: std.mem.Allocator, data: []const u8) ![]u8 {
    var out: std.ArrayList(u8) = .empty;
    errdefer out.deinit(gpa);
    try out.appendSlice(gpa, magic);
    var at: usize = 0;
    while (at < data.len) {
        const n = @min(data.len - at, 8);
        try out.append(gpa, @as(u8, @intCast((@as(u16, 1) << @intCast(n)) - 1)));
        try out.appendSlice(gpa, data[at .. at + n]);
        at += n;
    }
    return out.toOwnedSlice(gpa);
}
