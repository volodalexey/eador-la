//! Minimal Inno Setup extractor, ported from innoextract for the two GOG
//! "Eador: Genesis" installers: setup data 5.5.7 (unicode), single exe,
//! unencrypted; header blocks stored/LZMA1, file chunks stored/LZMA1/LZMA2
//! (these installers use LZMA2 for chunks).  GOG Galaxy multi-part files
//! (zlib-deflated parts named after their MD5) are reassembled like
//! innoextract does by default.
const std = @import("std");
const Io = std.Io;
const Allocator = std.mem.Allocator;
const mem = std.mem;
const lzma = std.compress.lzma;
const flate = std.compress.flate;

pub const Summary = struct { files: usize, bytes: u64, skipped_filtered: usize };

const Loader = struct { header_offset: u32, data_offset: u32 };

const File = struct {
    dest: []const u8,
    location: u32,
    after: []const u8,
    before: []const u8,
    /// Data locations of further GOG Galaxy parts appended to this file.
    parts: std.ArrayList(u32) = .empty,
    continuation: bool = false,
};

const Data = struct {
    first_slice: u32,
    last_slice: u32,
    chunk_offset: u32,
    file_offset: u64,
    size: u64,
    chunk_size: u64,
    sha1: [20]u8,
    flags: u16,
    /// Size after inflating a GOG Galaxy part; `size` otherwise.
    unpacked: u64,
    zlib: bool = false,

    // Bit positions of the 5.5.7 data entry flag set.
    const call_instruction_optimized = 1 << 4;
    const chunk_encrypted = 1 << 6;
    const chunk_compressed = 1 << 7;
};

const Output = struct { path: []const u8, locs: []const u32, size: u64 };
const Target = struct { out: u32, offset: u64 };

/// Extracts every file of an Inno Setup 5.5.7 (unicode) installer.
/// `exe` is the whole installer in memory; files are written into `out`
/// (subdirectories created as needed, existing files overwritten).
pub fn extract(gpa: Allocator, io: Io, exe: []const u8, out: Io.Dir) !Summary {
    var arena_state: std.heap.ArenaAllocator = .init(gpa);
    defer arena_state.deinit();
    const arena = arena_state.allocator();

    const loader = try findLoader(exe);
    var c: Cursor = .{ .buf = exe, .pos = loader.header_offset };
    try checkVersion(try c.take(64));
    const setup = try readBlock(arena, exe, &c.pos);
    const data_block = try readBlock(arena, exe, &c.pos);

    // Main header.
    var h: Cursor = .{ .buf = setup };
    var gog = false;
    for (0..32) |i| {
        const s = try h.str();
        // app_publisher, app_publisher_url, app_support_url, app_updates_url
        if (i == 4 or i == 5 or i == 7 or i == 8) {
            const u = try utf8(arena, s);
            const needle = if (i == 4) "GOG.com" else "www.gog.com";
            if (std.ascii.findIgnoreCase(u, needle) != null) gog = true;
        }
    }
    var counts: [16]u32 = undefined;
    for (&counts) |*n| n.* = try h.int(u32);
    // Fixed part: compression enum at 74, 47 option flags (6 bytes) at 87;
    // EncryptionUsed is flag 37.
    const rest = try h.take(93);
    const compression = rest[74];
    const encrypted = rest[87 + 4] & 0x20 != 0;
    if (encrypted) return error.EncryptedInstaller;

    // Sections in stream order; only files are kept.  Layouts are
    // {length-prefixed strings, fixed bytes}; see innoextract setup/*.cpp.
    // Most fixed parts include a 20-byte Windows version range.
    const languages, const messages, const permissions, const types, const components, const tasks = counts[0..6].*;
    const directories, const nfiles, const ndata, const icons, const inis, const registry, const deletes, const udeletes, const runs, const uruns = counts[6..16].*;
    try skipEntries(&h, languages, 10, 21);
    try skipEntries(&h, messages, 2, 4);
    try skipEntries(&h, permissions, 1, 0);
    try skipEntries(&h, types, 4, 30);
    try skipEntries(&h, components, 5, 42);
    try skipEntries(&h, tasks, 6, 26);
    try skipEntries(&h, directories, 7, 27);
    const files = try arena.alloc(File, nfiles);
    for (files) |*f| {
        // source, destination, font, assembly, then item conditions:
        // components, tasks, languages, check, after_install, before_install
        var s: [10][]const u8 = undefined;
        for (&s) |*x| x.* = try h.str();
        const fixed = try h.take(43);
        f.* = .{
            .dest = try utf8(arena, s[1]),
            .location = mem.readInt(u32, fixed[20..24], .little),
            .after = try utf8(arena, s[8]),
            .before = try utf8(arena, s[9]),
        };
    }
    try skipEntries(&h, icons, 13, 32);
    try skipEntries(&h, inis, 10, 21);
    try skipEntries(&h, registry, 9, 29);
    try skipEntries(&h, deletes, 7, 21);
    try skipEntries(&h, udeletes, 7, 21);
    try skipEntries(&h, runs, 13, 27);
    try skipEntries(&h, uruns, 13, 27);
    try skipEntries(&h, 2, 1, 0); // wizard image + small wizard image
    if (h.pos != setup.len) return error.CorruptHeader;

    var d: Cursor = .{ .buf = data_block };
    const data = try arena.alloc(Data, ndata);
    for (data) |*e| {
        e.* = .{
            .first_slice = try d.int(u32),
            .last_slice = try d.int(u32),
            .chunk_offset = try d.int(u32),
            .file_offset = try d.int(u64),
            .size = try d.int(u64),
            .chunk_size = try d.int(u64),
            .sha1 = (try d.take(20))[0..20].*,
            .flags = blk: {
                try d.skip(8 + 8); // filetime, file version
                break :blk try d.int(u16);
            },
            .unpacked = undefined,
        };
        e.unpacked = e.size;
    }
    if (d.pos != data_block.len) return error.CorruptHeader;

    if (gog) try joinGalaxyParts(arena, files, data);

    // Resolve output files; on path collisions the later entry wins.
    var outputs: std.ArrayList(Output) = .empty;
    var by_path: std.StringHashMapUnmanaged(usize) = .empty;
    var skipped_filtered: usize = 0;
    for (files) |*f| {
        if (f.continuation or f.location >= data.len) continue;
        const path = try convertPath(arena, f.dest);
        if (path.len == 0) continue;
        const locs = try arena.alloc(u32, 1 + f.parts.items.len);
        locs[0] = f.location;
        @memcpy(locs[1..], f.parts.items);
        var size: u64 = 0;
        var filtered = false;
        for (locs) |l| {
            if (l >= data.len) return error.CorruptHeader;
            size += data[l].unpacked;
            if (data[l].flags & Data.call_instruction_optimized != 0) filtered = true;
        }
        if (filtered) {
            skipped_filtered += 1;
            continue;
        }
        const key = try std.ascii.allocLowerString(arena, path);
        const gop = try by_path.getOrPut(arena, key);
        if (gop.found_existing) {
            outputs.items[gop.value_ptr.*] = .{ .path = path, .locs = locs, .size = size };
        } else {
            gop.value_ptr.* = outputs.items.len;
            try outputs.append(arena, .{ .path = path, .locs = locs, .size = size });
        }
    }

    // Map each used data location to the (file, offset) pairs it fills.
    const targets = try arena.alloc(std.ArrayList(Target), data.len);
    @memset(targets, .empty);
    var used: std.ArrayList(u32) = .empty;
    var total: u64 = 0;
    for (outputs.items, 0..) |o, i| {
        if (std.fs.path.dirnamePosix(o.path)) |dir| try out.createDirPath(io, dir);
        (try out.createFile(io, o.path, .{})).close(io);
        var off: u64 = 0;
        for (o.locs) |l| {
            if (targets[l].items.len == 0) try used.append(arena, l);
            try targets[l].append(arena, .{ .out = @intCast(i), .offset = off });
            off += data[l].unpacked;
        }
        total += o.size;
    }
    mem.sort(u32, used.items, data, struct {
        fn lt(ds: []const Data, a: u32, b: u32) bool {
            if (ds[a].chunk_offset != ds[b].chunk_offset) return ds[a].chunk_offset < ds[b].chunk_offset;
            return ds[a].file_offset < ds[b].file_offset;
        }
    }.lt);

    // Decompress each chunk once, up to the end of its last used file.
    var i: usize = 0;
    while (i < used.items.len) {
        const first = data[used.items[i]];
        var j = i;
        var need: u64 = 0;
        while (j < used.items.len and data[used.items[j]].chunk_offset == first.chunk_offset) : (j += 1) {
            const e = data[used.items[j]];
            need = @max(need, e.file_offset + e.size);
        }
        const chunk = try readChunk(gpa, exe, loader, first, compression, std.math.cast(usize, need) orelse return error.Unsupported);
        defer if (chunk.owned) gpa.free(chunk.bytes);
        for (used.items[i..j]) |l| {
            const e = data[l];
            const raw = chunk.bytes[@intCast(e.file_offset)..][0..@intCast(e.size)];
            var sha: [20]u8 = undefined;
            std.crypto.hash.Sha1.hash(raw, &sha, .{});
            if (!mem.eql(u8, &sha, &e.sha1)) return error.ChecksumMismatch;
            const bytes = if (e.zlib) try inflate(gpa, raw, e.unpacked) else raw;
            defer if (e.zlib) gpa.free(bytes);
            for (targets[l].items) |t| {
                var f = try out.openFile(io, outputs.items[t.out].path, .{ .mode = .write_only });
                defer f.close(io);
                try f.writePositionalAll(io, bytes, t.offset);
            }
        }
        i = j;
    }
    return .{ .files = outputs.items.len, .bytes = total, .skipped_filtered = skipped_filtered };
}

const Cursor = struct {
    buf: []const u8,
    pos: usize = 0,

    fn take(c: *Cursor, n: usize) ![]const u8 {
        if (c.pos > c.buf.len or c.buf.len - c.pos < n) return error.UnexpectedEnd;
        defer c.pos += n;
        return c.buf[c.pos..][0..n];
    }

    fn skip(c: *Cursor, n: usize) !void {
        _ = try c.take(n);
    }

    fn int(c: *Cursor, comptime T: type) !T {
        return mem.readInt(T, (try c.take(@sizeOf(T)))[0..@sizeOf(T)], .little);
    }

    /// Length-prefixed byte string (UTF-16LE text in unicode installers).
    fn str(c: *Cursor) ![]const u8 {
        return c.take(try c.int(u32));
    }
};

fn skipEntries(c: *Cursor, count: usize, strings: usize, fixed: usize) !void {
    for (0..count) |_| {
        for (0..strings) |_| _ = try c.str();
        try c.skip(fixed);
    }
}

fn readAt(comptime T: type, buf: []const u8, off: usize) !T {
    var c: Cursor = .{ .buf = buf, .pos = off };
    return c.int(T);
}

/// Since 5.1.5 the loader table is PE resource RT_RCDATA (10) / 11111.
fn findLoader(exe: []const u8) !Loader {
    const off = findResource(exe, 10, 11111) catch return error.NotInnoSetup;
    var c: Cursor = .{ .buf = exe, .pos = off };
    const magic = try c.take(12);
    if (!mem.eql(u8, magic, "rDlPtS\xcd\xe6\xd7{\x0b*") and !mem.eql(u8, magic, "nS5W7dT\x83\xaa\x1b\x0fj"))
        return error.NotInnoSetup;
    // revision, ?, exe offset, exe size, exe crc, header offset, data offset, crc
    const fields = try c.take(7 * 4);
    if (std.hash.Crc32.hash(exe[off..][0 .. 12 + 28]) != try c.int(u32)) return error.CorruptHeader;
    return .{
        .header_offset = mem.readInt(u32, fields[20..24], .little),
        .data_offset = mem.readInt(u32, fields[24..28], .little),
    };
}

fn findResource(exe: []const u8, typ: u32, name: u32) !u32 {
    const pe = try readAt(u32, exe, 0x3c);
    if (try readAt(u32, exe, pe) != 0x4550) return error.NotPe;
    const nsections = try readAt(u16, exe, pe + 6);
    const opt = pe + 24;
    const sections = opt + try readAt(u16, exe, pe + 20);
    const dirs = opt + @as(u32, if (try readAt(u16, exe, opt) == 0x20b) 112 else 96);
    if (try readAt(u32, exe, dirs - 4) < 3) return error.NotPe;
    const toFile = struct {
        fn f(buf: []const u8, secs: u32, n: u16, rva: u32) !u32 {
            for (0..n) |k| {
                const s = secs + @as(u32, @intCast(k)) * 40;
                const vsize = try readAt(u32, buf, s + 8);
                const va = try readAt(u32, buf, s + 12);
                if (rva >= va and rva - va < vsize) return rva - va + try readAt(u32, buf, s + 20);
            }
            return error.NotPe;
        }
    }.f;
    const root = try toFile(exe, sections, nsections, try readAt(u32, exe, dirs + 16));
    const t = try resourceEntry(exe, root, typ);
    if (t & 0x8000_0000 == 0) return error.NotPe;
    const n = try resourceEntry(exe, root + (t & 0x7fff_ffff), name);
    if (n & 0x8000_0000 == 0) return error.NotPe;
    // First language entry, which must be a leaf.
    const leaf = try readAt(u32, exe, root + (n & 0x7fff_ffff) + 16 + 4);
    if (leaf & 0x8000_0000 != 0) return error.NotPe;
    return toFile(exe, sections, nsections, try readAt(u32, exe, root + leaf));
}

fn resourceEntry(exe: []const u8, dir: u32, id: u32) !u32 {
    const names = try readAt(u16, exe, dir + 12);
    const ids = try readAt(u16, exe, dir + 14);
    for (0..ids) |k| {
        const e = dir + 16 + (names + @as(u32, @intCast(k))) * 8;
        if (try readAt(u32, exe, e) == id) return readAt(u32, exe, e + 4);
    }
    return error.NotPe;
}

fn checkVersion(id: []const u8) !void {
    const name = id[0 .. mem.findScalar(u8, id, 0) orelse id.len];
    if (!mem.eql(u8, name, "Inno Setup Setup Data (5.5.7) (u)") and
        !mem.eql(u8, name, "Inno Setup Setup Data (5.5.7) (U)"))
        return error.UnsupportedInnoVersion; // only 5.5.7 unicode
}

/// Header block: CRC'd {size, compressed} prefix, then 4096-byte pieces each
/// preceded by their CRC32; the concatenation is optionally LZMA1.
fn readBlock(arena: Allocator, exe: []const u8, pos: *usize) ![]u8 {
    var c: Cursor = .{ .buf = exe, .pos = pos.* };
    const crc = try c.int(u32);
    const prefix = try c.take(5);
    if (std.hash.Crc32.hash(prefix) != crc) return error.CorruptHeader;
    const stored = mem.readInt(u32, prefix[0..4], .little);
    var body: Cursor = .{ .buf = try c.take(stored) };
    pos.* = c.pos;

    var joined: std.ArrayList(u8) = try .initCapacity(arena, stored);
    while (body.pos < body.buf.len) {
        const piece_crc = try body.int(u32);
        const piece = try body.take(@min(4096, body.buf.len - body.pos));
        if (std.hash.Crc32.hash(piece) != piece_crc) return error.CorruptHeader;
        joined.appendSliceAssumeCapacity(piece);
    }
    if (prefix[4] == 0) return joined.items;
    return lzmaDecode(arena, joined.items, null);
}

const Chunk = struct { bytes: []const u8, owned: bool };

fn readChunk(gpa: Allocator, exe: []const u8, loader: Loader, e: Data, compression: u8, need: usize) !Chunk {
    if (e.first_slice != 0 or e.last_slice != 0) return error.Unsupported;
    if (e.flags & Data.chunk_encrypted != 0) return error.EncryptedInstaller;
    var c: Cursor = .{ .buf = exe, .pos = @as(usize, loader.data_offset) + e.chunk_offset };
    if (!mem.eql(u8, try c.take(4), "zlb\x1a")) return error.BadChunkMagic;
    const src = try c.take(std.math.cast(usize, e.chunk_size) orelse return error.Unsupported);
    if (e.flags & Data.chunk_compressed == 0) {
        if (src.len < need) return error.UnexpectedEnd;
        return .{ .bytes = src, .owned = false };
    }
    // stored_compression_method_3: 0 stored, 1 zlib, 2 bzip2, 3 LZMA1, 4 LZMA2
    return switch (compression) {
        3 => .{ .bytes = try lzmaDecode(gpa, src, need), .owned = true },
        4 => .{ .bytes = try lzma2Decode(gpa, src, need), .owned = true },
        else => error.UnsupportedCompression,
    };
}

/// Inno LZMA2: one dictionary-size byte, then a standard LZMA2 chunk stream.
fn lzma2Decode(gpa: Allocator, src: []const u8, need: usize) ![]u8 {
    if (src.len < 1 or src[0] > 40) return error.CorruptLzma;
    var in: Io.Reader = .fixed(src[1..]);
    var w: Io.Writer.Allocating = try .initCapacity(gpa, need);
    errdefer w.deinit();
    var dec: std.compress.lzma2.Decode = try .init(gpa);
    defer dec.deinit(gpa);
    _ = dec.decompress(&in, &w) catch |err| switch (err) {
        error.OutOfMemory => return error.OutOfMemory,
        else => return error.CorruptLzma,
    };
    if (w.written().len < need) return error.CorruptLzma;
    return w.toOwnedSlice();
}

/// Inno LZMA1: 5-byte properties header, then a raw stream with no size and
/// usually no end marker.  Decodes until `want` bytes are available or, if
/// `want` is null, until the input is consumed and the range coder is done.
fn lzmaDecode(gpa: Allocator, src: []const u8, want: ?usize) ![]u8 {
    if (src.len < 5 or src[0] >= 9 * 5 * 5) return error.CorruptLzma;
    const props: lzma.Decode.Properties = .{
        .lc = @intCast(src[0] % 9),
        .lp = @intCast(src[0] / 9 % 5),
        .pb = @intCast(src[0] / 45),
    };
    var in: Io.Reader = .fixed(src[5..]);
    var rc: lzma.RangeDecoder = try .init(&in);
    var dec: lzma.Decode = try .init(gpa, props);
    defer dec.deinit(gpa);
    var acc: std.compress.lzma2.AccumBuffer = .init(std.math.maxInt(usize));
    errdefer acc.deinit(gpa);
    if (want) |n| try acc.buf.ensureTotalCapacity(gpa, n);
    // AccumBuffer keeps everything itself; this writer stays empty.
    var sink: Io.Writer.Allocating = .init(gpa);
    defer sink.deinit();
    var n_read: u64 = 0;
    while (true) {
        if (want) |n| {
            if (acc.len >= n) break;
        } else if (in.seek == in.end and rc.isFinished()) break;
        if (try dec.process(&in, &sink, &acc, &rc, &n_read) == .finished) break;
    }
    if (want) |n| if (acc.len < n) return error.CorruptLzma;
    return acc.buf.toOwnedSlice(gpa);
}

fn inflate(gpa: Allocator, src: []const u8, size: u64) ![]u8 {
    var in: Io.Reader = .fixed(src);
    var window: [flate.max_window_len]u8 = undefined;
    var z: flate.Decompress = .init(&in, .zlib, &window);
    var w: Io.Writer.Allocating = try .initCapacity(gpa, std.math.cast(usize, size) orelse return error.Unsupported);
    defer w.deinit();
    _ = z.reader.streamRemaining(&w.writer) catch return error.CorruptZlib;
    if (w.written().len != size) return error.CorruptZlib;
    return w.toOwnedSlice();
}

fn utf8(arena: Allocator, raw: []const u8) ![]u8 {
    if (raw.len % 2 != 0) return error.CorruptHeader;
    const units = try arena.alloc(u16, raw.len / 2);
    for (units, 0..) |*u, k| u.* = mem.readInt(u16, raw[2 * k ..][0..2], .little);
    return std.unicode.utf16LeToUtf8Alloc(arena, units);
}

/// GOG Galaxy: `before_install('md5', 'name', parts)` starts a file whose
/// parts follow, each with `after_install('md5', packed, unpacked)` and
/// zlib-deflated data (cli/goggalaxy.cpp).
fn joinGalaxyParts(arena: Allocator, files: []File, data: []Data) !void {
    var start: ?*File = null;
    var remaining: usize = 0;
    for (files) |*f| {
        const begin = try parseCall(arena, f.before, "before_install") orelse
            try parseCall(arena, f.before, "before_install_dependency");
        if (begin) |args| {
            if (args.len >= 2 and args[1].len > 0) f.dest = args[1];
            remaining = 1;
            if (args.len >= 3) remaining = @max(1, std.fmt.parseInt(usize, args[2], 10) catch 1);
            start = f;
        }
        const part = try parseCall(arena, f.after, "after_install") orelse
            try parseCall(arena, f.after, "after_install_dependency");
        if (part) |args| {
            if (remaining == 0 or f.location >= data.len or args.len < 3) {
                remaining = 0;
                continue;
            }
            remaining -= 1;
            const e = &data[f.location];
            e.unpacked = std.fmt.parseInt(u64, args[2], 10) catch return error.CorruptHeader;
            e.zlib = true;
            if (start.? != f) {
                f.continuation = true;
                try start.?.parts.append(arena, f.location);
            }
        } else if (begin == null) remaining = 0;
    }
}

/// Parses `name(arg, 'quoted ''arg''', ...)`; null if `code` calls something else.
fn parseCall(arena: Allocator, code: []const u8, name: []const u8) !?[]const []const u8 {
    const ws = " \t\r\n";
    const sep = " \t\r\n(),'";
    var p = mem.findNone(u8, code, ws) orelse return null;
    const end = mem.findAnyPos(u8, code, p, sep) orelse return null;
    const paren = mem.findNonePos(u8, code, end, ws) orelse return null;
    if (code[paren] != '(' or !mem.eql(u8, code[p..end], name)) return null;
    var args: std.ArrayList([]const u8) = .empty;
    p = paren + 1;
    while (true) {
        p = mem.findNonePos(u8, code, p, ws) orelse break;
        if (code[p] == '\'') {
            var s: std.ArrayList(u8) = .empty;
            p += 1;
            while (true) {
                const q = mem.findScalarPos(u8, code, p, '\'') orelse code.len;
                try s.appendSlice(arena, code[p..q]);
                p = @min(q + 1, code.len);
                if (p < code.len and code[p] == '\'') {
                    try s.append(arena, '\'');
                    p += 1;
                } else break;
            }
            try args.append(arena, s.items);
        } else {
            const q = mem.findAnyPos(u8, code, p, sep) orelse code.len;
            try args.append(arena, code[p..q]);
            p = q;
        }
        p = mem.findNonePos(u8, code, p, ws) orelse break;
        if (code[p] != ',') break;
        p += 1;
    }
    return args.items;
}

/// innoextract's default name mapping: `{const}` becomes `const`, `{{` is
/// `{`, unsafe characters become `$`, and `\`, `/`, `.`, `..` are normalized.
fn convertPath(arena: Allocator, path: []const u8) ![]u8 {
    var expanded: std.ArrayList(u8) = .empty;
    var k: usize = 0;
    while (k < path.len) : (k += 1) {
        const ch = path[k];
        if (ch == '{') {
            if (k + 1 < path.len and path[k + 1] == '{') {
                try expanded.append(arena, '{');
                k += 1;
                continue;
            }
            const close = mem.findScalarPos(u8, path, k + 1, '}') orelse path.len;
            for (path[k + 1 .. close]) |v| try expanded.append(arena, safe(v));
            k = close;
        } else try expanded.append(arena, if (ch == '}') ch else safe(ch));
    }
    var result: std.ArrayList(u8) = .empty;
    var it = mem.tokenizeAny(u8, expanded.items, "\\/");
    while (it.next()) |seg| {
        if (mem.eql(u8, seg, ".")) continue;
        if (mem.eql(u8, seg, "..")) {
            result.items.len = mem.findScalarLast(u8, result.items, '/') orelse 0;
            continue;
        }
        if (result.items.len != 0) try result.append(arena, '/');
        try result.appendSlice(arena, seg);
    }
    return result.items;
}

fn safe(ch: u8) u8 {
    return if (ch < 32 or mem.findScalar(u8, "<>:\"|?*", ch) != null) '$' else ch;
}

test convertPath {
    var arena_state: std.heap.ArenaAllocator = .init(std.testing.allocator);
    defer arena_state.deinit();
    const a = arena_state.allocator();
    try std.testing.expectEqualStrings("app/dat/Ability.dat", try convertPath(a, "{app}\\dat\\Ability.dat"));
    try std.testing.expectEqualStrings("dat/Town/Town1.dat", try convertPath(a, "dat\\Town\\.\\Town1.dat"));
    try std.testing.expectEqualStrings("tmp/b", try convertPath(a, "{tmp}\\a\\..\\b"));
    try std.testing.expectEqualStrings("{x}/a$b", try convertPath(a, "{{x}\\a:b"));
    try std.testing.expectEqualStrings("", try convertPath(a, ""));
}

test parseCall {
    var arena_state: std.heap.ArenaAllocator = .init(std.testing.allocator);
    defer arena_state.deinit();
    const a = arena_state.allocator();
    const args = (try parseCall(a, " before_install('abc', 'dat\\it''s.dat', 3);", "before_install")).?;
    try std.testing.expectEqual(3, args.len);
    try std.testing.expectEqualStrings("abc", args[0]);
    try std.testing.expectEqualStrings("dat\\it's.dat", args[1]);
    try std.testing.expectEqualStrings("3", args[2]);
    try std.testing.expect(try parseCall(a, "after_install('x', 1, 2)", "before_install") == null);
    try std.testing.expect(try parseCall(a, "", "before_install") == null);
}

test "block framing and version check" {
    var raw: [9 + 4 + 3]u8 = undefined;
    mem.writeInt(u32, raw[4..8], 4 + 3, .little);
    raw[8] = 0; // stored
    mem.writeInt(u32, raw[0..4], std.hash.Crc32.hash(raw[4..9]), .little);
    mem.writeInt(u32, raw[9..13], std.hash.Crc32.hash("abc"), .little);
    @memcpy(raw[13..16], "abc");
    var arena_state: std.heap.ArenaAllocator = .init(std.testing.allocator);
    defer arena_state.deinit();
    var pos: usize = 0;
    try std.testing.expectEqualStrings("abc", try readBlock(arena_state.allocator(), &raw, &pos));
    try std.testing.expectEqual(raw.len, pos);
    var id: [64]u8 = @splat(0);
    @memcpy(id[0..33], "Inno Setup Setup Data (5.5.7) (u)");
    try checkVersion(&id);
    id[24] = '6';
    try std.testing.expectError(error.UnsupportedInnoVersion, checkVersion(&id));
}
