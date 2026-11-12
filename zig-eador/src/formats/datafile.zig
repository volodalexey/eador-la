//! Allegro 4 datafile ("slh!"/"slh." + "ALL." object tree) and BMP object decoding.
const std = @import("std");
const lzss = @import("lzss.zig");

pub const Object = struct {
    kind: [4]u8,
    name: []const u8,
    data: []const u8,
    children: []const Object = &.{},

    pub fn isBitmap(self: Object) bool {
        return std.mem.eql(u8, &self.kind, "BMP ") or std.mem.eql(u8, &self.kind, "RLE ");
    }
    pub fn isOgg(self: Object) bool {
        return std.mem.endsWith(u8, &self.kind, "GG ");
    }
};

pub const Datafile = struct {
    arena: std.heap.ArenaAllocator,
    raw: []u8,
    gpa: std.mem.Allocator,
    objects: []const Object,

    pub fn parse(gpa: std.mem.Allocator, file_bytes: []const u8) !Datafile {
        if (file_bytes.len < 4) return error.BadDatafile;
        const magic = file_bytes[0..4];
        const raw = if (std.mem.eql(u8, magic, "slh!"))
            try lzss.decode(gpa, file_bytes[4..], file_bytes.len * 3, null)
        else if (std.mem.eql(u8, magic, "slh."))
            try gpa.dupe(u8, file_bytes[4..])
        else
            return error.BadDatafile;
        errdefer gpa.free(raw);

        var arena = std.heap.ArenaAllocator.init(gpa);
        errdefer arena.deinit();
        var pos: usize = 0;
        const objs = try parseList(arena.allocator(), raw, &pos);
        return .{ .arena = arena, .raw = raw, .gpa = gpa, .objects = objs };
    }

    pub fn deinit(self: *Datafile) void {
        self.arena.deinit();
        self.gpa.free(self.raw);
    }

    pub fn find(self: *const Datafile, name: []const u8) ?*const Object {
        for (self.objects) |*o| {
            if (std.ascii.eqlIgnoreCase(o.name, name)) return o;
        }
        return null;
    }
};

fn readBe(d: []const u8, pos: *usize) !i32 {
    if (pos.* + 4 > d.len) return error.BadDatafile;
    const v = std.mem.readInt(i32, d[pos.*..][0..4], .big);
    pos.* += 4;
    return v;
}

fn take(d: []const u8, pos: *usize, n: i32) ![]const u8 {
    if (n < 0) return error.BadDatafile;
    const len: usize = @intCast(n);
    if (pos.* + len > d.len) return error.BadDatafile;
    const s = d[pos.*..][0..len];
    pos.* += len;
    return s;
}

fn parseList(a: std.mem.Allocator, d: []const u8, pos: *usize) ![]const Object {
    if (pos.* + 4 > d.len or !std.mem.eql(u8, d[pos.*..][0..4], "ALL.")) return error.BadDatafile;
    pos.* += 4;
    const count = try readBe(d, pos);
    if (count < 0) return error.BadDatafile;
    const objs = try a.alloc(Object, @intCast(count));
    for (objs) |*o| {
        var name: []const u8 = "";
        while (pos.* + 12 <= d.len and std.mem.eql(u8, d[pos.*..][0..4], "prop")) {
            const ptype = d[pos.* + 4 ..][0..4];
            pos.* += 8;
            const sz = try readBe(d, pos);
            const val = try take(d, pos, sz);
            if (std.mem.eql(u8, ptype, "NAME")) name = val;
        }
        if (pos.* + 12 > d.len) return error.BadDatafile;
        const kind = d[pos.*..][0..4].*;
        pos.* += 4;
        const psz = try readBe(d, pos);
        const usz = try readBe(d, pos);
        var data: []const u8 = undefined;
        if (psz < 0) {
            const packed_bytes = try take(d, pos, -psz);
            const n: usize = @intCast(@max(usz, 0));
            data = try lzss.decode(a, packed_bytes, n, n);
        } else {
            data = try take(d, pos, psz);
        }
        var children: []const Object = &.{};
        if (std.mem.eql(u8, &kind, "FILE")) {
            var p: usize = 0;
            children = try parseList(a, data, &p);
        }
        o.* = .{ .kind = kind, .name = name, .data = data, .children = children };
    }
    return objs;
}

pub const Rgba = struct {
    w: u32,
    h: u32,
    pixels: []u8,
};

pub const BitmapInfo = struct { bits: i16, w: u32, h: u32 };

pub fn bitmapInfo(data: []const u8) !BitmapInfo {
    if (data.len < 6) return error.BadBitmap;
    const bits = std.mem.readInt(i16, data[0..2], .big);
    const w = std.mem.readInt(i16, data[2..4], .big);
    const h = std.mem.readInt(i16, data[4..6], .big);
    if (w <= 0 or h <= 0) return error.BadBitmap;
    return .{ .bits = bits, .w = @intCast(w), .h = @intCast(h) };
}

/// Decode an Allegro BMP object to RGBA8. Magenta (255,0,255) is Allegro's
/// mask colour and becomes fully transparent. 8-bit images carry no palette
/// in these files, so they are shown as grayscale.
pub fn decodeBitmap(gpa: std.mem.Allocator, data: []const u8) !Rgba {
    const info = try bitmapInfo(data);
    const n = @as(usize, info.w) * info.h;
    const src = data[6..];
    const out = try gpa.alloc(u8, n * 4);
    errdefer gpa.free(out);
    switch (info.bits) {
        24, 32, -32 => {
            const bpp: usize = if (info.bits == 24) 3 else 4;
            if (src.len < n * bpp) return error.BadBitmap;
            for (0..n) |i| {
                const r = src[i * bpp];
                const g = src[i * bpp + 1];
                const b = src[i * bpp + 2];
                const masked = r == 255 and g == 0 and b == 255;
                const a: u8 = if (masked) 0 else if (info.bits == -32) src[i * bpp + 3] else 255;
                out[i * 4 ..][0..4].* = .{ r, g, b, a };
            }
        },
        8 => {
            if (src.len < n) return error.BadBitmap;
            for (0..n) |i| {
                const v = src[i];
                out[i * 4 ..][0..4].* = .{ v, v, v, if (v == 0) 0 else 255 };
            }
        },
        else => return error.UnsupportedBitmap,
    }
    return .{ .w = info.w, .h = info.h, .pixels = out };
}
