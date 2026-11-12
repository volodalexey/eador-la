//! The application icon of a Windows executable (what Wine shows for
//! Eador.exe): the largest image of its first RT_GROUP_ICON resource,
//! decoded from the RT_ICON DIB (1/4/8/24/32 bpp + AND mask) to RGBA.
const std = @import("std");

pub const Image = struct {
    w: u32,
    h: u32,
    /// RGBA8, top row first.
    rgba: []u8,
};

const rt_icon = 3;
const rt_group_icon = 14;

fn rd16(d: []const u8, o: usize) !u16 {
    if (o + 2 > d.len) return error.BadExe;
    return std.mem.readInt(u16, d[o..][0..2], .little);
}

fn rd32(d: []const u8, o: usize) !u32 {
    if (o + 4 > d.len) return error.BadExe;
    return std.mem.readInt(u32, d[o..][0..4], .little);
}

const Pe = struct {
    d: []const u8,
    sec: usize,
    n_sec: usize,
    res_rva: u32,
    res_off: usize,

    fn open(d: []const u8) !?Pe {
        if (d.len < 0x40 or d[0] != 'M' or d[1] != 'Z') return error.BadExe;
        const pe = try rd32(d, 0x3C);
        if (try rd32(d, pe) != 0x4550) return error.BadExe;
        const n_sec = try rd16(d, pe + 6);
        const opt_size = try rd16(d, pe + 20);
        const opt = pe + 24;
        const dirs: usize = if (try rd16(d, opt) == 0x20B) opt + 112 else opt + 96;
        const res_rva = try rd32(d, dirs + 2 * 8);
        if (res_rva == 0) return null;
        var p: Pe = .{ .d = d, .sec = opt + opt_size, .n_sec = n_sec, .res_rva = res_rva, .res_off = 0 };
        p.res_off = try p.offset(res_rva);
        return p;
    }

    /// File offset of an RVA.
    fn offset(self: Pe, rva: u32) !usize {
        for (0..self.n_sec) |i| {
            const s = self.sec + 40 * i;
            const vsize = try rd32(self.d, s + 8);
            const va = try rd32(self.d, s + 12);
            const rsize = try rd32(self.d, s + 16);
            const raw = try rd32(self.d, s + 20);
            if (rva >= va and rva < va + @max(vsize, rsize)) return raw + (rva - va);
        }
        return error.BadExe;
    }

    /// Entry `id` (or the first one when null) of a resource directory;
    /// returns (offset in the resource section, is a subdirectory).
    fn entry(self: Pe, dir: usize, id: ?u32) !?struct { off: usize, sub: bool } {
        const base = self.res_off + dir;
        const n = @as(usize, try rd16(self.d, base + 12)) + try rd16(self.d, base + 14);
        for (0..n) |k| {
            const e = base + 16 + 8 * k;
            const name = try rd32(self.d, e);
            const to = try rd32(self.d, e + 4);
            if (id) |want| if (name != want) continue;
            return .{ .off = to & 0x7FFF_FFFF, .sub = to & 0x8000_0000 != 0 };
        }
        return null;
    }

    /// Data of resource (type, name or the first one), first language.
    fn resource(self: Pe, ty: u32, name: ?u32) !?[]const u8 {
        const t = try self.entry(0, ty) orelse return null;
        if (!t.sub) return null;
        const n = try self.entry(t.off, name) orelse return null;
        if (!n.sub) return null;
        const l = try self.entry(n.off, null) orelse return null;
        if (l.sub) return null;
        const rva = try rd32(self.d, self.res_off + l.off);
        const size = try rd32(self.d, self.res_off + l.off + 4);
        const o = try self.offset(rva);
        if (o + size > self.d.len) return error.BadExe;
        return self.d[o..][0..size];
    }
};

/// Decodes the executable's icon, or null if it has none (or only a PNG
/// one, which is returned through `png_out` unchanged).
pub fn extract(gpa: std.mem.Allocator, exe: []const u8, png_out: *?[]const u8) !?Image {
    png_out.* = null;
    const pe = try Pe.open(exe) orelse return null;
    const grp = try pe.resource(rt_group_icon, null) orelse return null;
    const count = try rd16(grp, 4);
    var best: ?u16 = null;
    var best_score: u64 = 0;
    for (0..count) |k| {
        const e = 6 + 14 * k;
        if (e + 14 > grp.len) break;
        const w: u64 = if (grp[e] == 0) 256 else grp[e];
        const h: u64 = if (grp[e + 1] == 0) 256 else grp[e + 1];
        const score = w * h * 64 + try rd16(grp, e + 6);
        if (score > best_score) {
            best_score = score;
            best = try rd16(grp, e + 12);
        }
    }
    const id = best orelse return null;
    const dib = try pe.resource(rt_icon, id) orelse return null;
    if (dib.len >= 8 and std.mem.eql(u8, dib[0..8], "\x89PNG\r\n\x1a\n")) {
        png_out.* = dib;
        return null;
    }
    return try decodeDib(gpa, dib);
}

fn decodeDib(gpa: std.mem.Allocator, dib: []const u8) !Image {
    const hs = try rd32(dib, 0);
    const w = try rd32(dib, 4);
    const h = (try rd32(dib, 8)) / 2;
    const bpp = try rd16(dib, 14);
    if (w == 0 or h == 0 or w > 1024 or h > 1024) return error.BadIcon;
    var ncol: usize = try rd32(dib, 32);
    if (ncol == 0 and bpp <= 8) ncol = @as(usize, 1) << @intCast(bpp);
    const pal = hs;
    const xor_off = pal + 4 * ncol;
    const stride = ((w * bpp + 31) / 32) * 4;
    const and_off = xor_off + stride * h;
    const mstride = ((w + 31) / 32) * 4;
    if (and_off + mstride * h > dib.len) return error.BadIcon;

    const px = try gpa.alloc(u8, @as(usize, w) * h * 4);
    errdefer gpa.free(px);
    var any_alpha = false;
    for (0..h) |y| {
        const row = xor_off + (h - 1 - y) * stride;
        for (0..w) |x| {
            const o = (y * w + x) * 4;
            var c: [4]u8 = .{ 0, 0, 0, 255 };
            switch (bpp) {
                1, 4, 8 => {
                    const bit = x * bpp;
                    const byte = dib[row + bit / 8];
                    const shift: u3 = @intCast(8 - bpp - bit % 8);
                    const mask: u8 = @intCast((@as(u16, 1) << @intCast(bpp)) - 1);
                    const idx: usize = (byte >> shift) & mask;
                    if (idx < ncol) {
                        const p = dib[pal + 4 * idx ..][0..4];
                        c = .{ p[2], p[1], p[0], 255 };
                    }
                },
                24 => {
                    const p = dib[row + 3 * x ..][0..3];
                    c = .{ p[2], p[1], p[0], 255 };
                },
                32 => {
                    const p = dib[row + 4 * x ..][0..4];
                    c = .{ p[2], p[1], p[0], p[3] };
                    if (p[3] != 0) any_alpha = true;
                },
                else => return error.BadIcon,
            }
            @memcpy(px[o..][0..4], &c);
        }
    }
    // The AND mask gives transparency unless a 32-bit icon has alpha.
    if (!(bpp == 32 and any_alpha)) {
        for (0..h) |y| {
            const row = and_off + (h - 1 - y) * mstride;
            for (0..w) |x| {
                const t = (dib[row + x / 8] >> @intCast(7 - x % 8)) & 1;
                px[(y * w + x) * 4 + 3] = if (t != 0) 0 else 255;
            }
        }
    }
    return .{ .w = w, .h = h, .rgba = px };
}
