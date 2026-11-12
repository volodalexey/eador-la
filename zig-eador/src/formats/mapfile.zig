//! save\map%d.map as FUN_0046e080 reads it (after packfile.unpack):
//! PCX of the whole map, u16 province count (0 → a second u16, version
//! 0x67 without guard blocks; else version 0x66), then per province a 0x18
//! header (+0 index, +4..+0x10 map rectangle x0 y0 x1 y1), the province
//! PCX (its shape on the map), the 0x4E4 record and a NUL-terminated
//! CP1251 name; version 0x66 adds 31 guard/slot blocks.
const std = @import("std");

pub const record_size = 0x4E4;

pub const Image = struct {
    w: u32,
    h: u32,
    /// RGBA8.
    rgba: []u8,
};

pub const Province = struct {
    index: i32,
    x0: i32,
    y0: i32,
    x1: i32,
    y1: i32,
    /// Shape: RGBA, transparent where the PCX is magenta (Allegro mask).
    shape: Image,
    record: [record_size]u8,
    /// CP1251 bytes (slice of the map data).
    name: []const u8,

    pub fn int(self: *const Province, off: usize) i32 {
        return std.mem.readInt(i32, self.record[off..][0..4], .little);
    }
};

/// A border cell (DAT_00524338[y] lists): map point (x, y) between
/// provinces a and b (c = a third one at a corner, else 0).
pub const Cell = struct { x: i32, y: i32, a: i32, b: i32, c: i32 };

pub const Map = struct {
    image: Image,
    provinces: []Province,
    cells: []Cell = &.{},

    pub fn deinit(self: *Map, gpa: std.mem.Allocator) void {
        gpa.free(self.image.rgba);
        for (self.provinces) |p| gpa.free(p.shape.rgba);
        gpa.free(self.provinces);
        gpa.free(self.cells);
    }
};

const Reader = struct {
    d: []const u8,
    p: usize = 0,

    fn take(self: *Reader, n: usize) ![]const u8 {
        if (self.p + n > self.d.len) return error.Truncated;
        const s = self.d[self.p..][0..n];
        self.p += n;
        return s;
    }

    fn byte(self: *Reader) !u8 {
        return (try self.take(1))[0];
    }

    fn u16le(self: *Reader) !u16 {
        return std.mem.readInt(u16, (try self.take(2))[0..2], .little);
    }
};

/// Allegro load_pcx_pf: 8-bit with a 256-colour palette after the data, or
/// 24-bit (three 8-bit planes per line). Magenta becomes transparent.
fn readPcx(gpa: std.mem.Allocator, r: *Reader) !Image {
    const h = try r.take(128);
    if (h[0] != 0x0A or h[3] != 8) return error.UnsupportedPcx;
    const xmin = std.mem.readInt(u16, h[4..6], .little);
    const ymin = std.mem.readInt(u16, h[6..8], .little);
    const xmax = std.mem.readInt(u16, h[8..10], .little);
    const ymax = std.mem.readInt(u16, h[10..12], .little);
    if (xmax < xmin or ymax < ymin) return error.UnsupportedPcx;
    const planes: usize = h[65];
    if (planes != 1 and planes != 3) return error.UnsupportedPcx;
    const bpl: usize = std.mem.readInt(u16, h[66..68], .little);
    const w: u32 = @as(u32, xmax) - xmin + 1;
    const ht: u32 = @as(u32, ymax) - ymin + 1;
    if (bpl < w) return error.UnsupportedPcx;

    const line = try gpa.alloc(u8, bpl * planes);
    defer gpa.free(line);
    const raw = try gpa.alloc(u8, @as(usize, w) * ht * planes);
    defer gpa.free(raw);
    for (0..ht) |y| {
        var x: usize = 0;
        while (x < line.len) {
            const c = try r.byte();
            if (c & 0xC0 == 0xC0) {
                const v = try r.byte();
                const n = @min(@as(usize, c & 0x3F), line.len - x);
                @memset(line[x..][0..n], v);
                x += n;
            } else {
                line[x] = c;
                x += 1;
            }
        }
        for (0..planes) |pl| @memcpy(raw[(y * planes + pl) * w ..][0..w], line[pl * bpl ..][0..w]);
    }
    var pal: [768]u8 = undefined;
    if (planes == 1) {
        // Palette: 0x0C marker and 768 bytes right after the image data.
        if ((try r.byte()) != 12) return error.UnsupportedPcx;
        @memcpy(&pal, try r.take(768));
    }
    const rgba = try gpa.alloc(u8, @as(usize, w) * ht * 4);
    for (0..ht) |y| {
        for (0..w) |x| {
            var c: [3]u8 = undefined;
            if (planes == 3) {
                for (0..3) |k| c[k] = raw[(y * 3 + k) * w + x];
            } else {
                const i: usize = raw[y * w + x];
                c = pal[i * 3 ..][0..3].*;
            }
            const o = (y * w + x) * 4;
            const masked = c[0] == 255 and c[1] == 0 and c[2] == 255;
            rgba[o..][0..4].* = .{ c[0], c[1], c[2], if (masked) 0 else 255 };
        }
    }
    return .{ .w = w, .h = ht, .rgba = rgba };
}

/// Skips a PCX without decoding its pixels (the RLE stream must still be
/// walked: its packed length is not stored).
fn skipPcx(r: *Reader) !void {
    const h = try r.take(128);
    if (h[0] != 0x0A or h[3] != 8) return error.UnsupportedPcx;
    const ymin = std.mem.readInt(u16, h[6..8], .little);
    const ymax = std.mem.readInt(u16, h[10..12], .little);
    if (ymax < ymin) return error.UnsupportedPcx;
    const planes: usize = h[65];
    const line: usize = @as(usize, std.mem.readInt(u16, h[66..68], .little)) * planes;
    for (0..@as(usize, ymax) - ymin + 1) |_| {
        var x: usize = 0;
        while (x < line) {
            const c = try r.byte();
            if (c & 0xC0 == 0xC0) {
                _ = try r.byte();
                x += c & 0x3F;
            } else x += 1;
        }
    }
    if (planes == 1) {
        if ((try r.byte()) != 12) return error.UnsupportedPcx;
        _ = try r.take(768);
    }
}

/// Province names only (CP1251 slices of `data`, index i = province i + 1),
/// without decoding the map and shape images.
pub fn provinceNames(gpa: std.mem.Allocator, data: []const u8) ![][]const u8 {
    var r: Reader = .{ .d = data };
    try skipPcx(&r);
    var n: usize = try r.u16le();
    var with_guards = true;
    if (n == 0) {
        n = try r.u16le();
        with_guards = false;
    }
    const names = try gpa.alloc([]const u8, n);
    errdefer gpa.free(names);
    for (names) |*name| {
        _ = try r.take(0x18);
        try skipPcx(&r);
        _ = try r.take(record_size);
        const start = r.p;
        while ((try r.byte()) != 0) {}
        name.* = data[start .. r.p - 1];
        if (with_guards) {
            for (0..31) |_| try skipBlock(&r);
        }
    }
    return names;
}

fn skipBlock(r: *Reader) !void {
    if ((try r.byte()) == 0) return;
    _ = try r.take(0x3A4);
    for (0..16) |_| {
        if ((try r.byte()) != 0) _ = try r.take(0xA4);
    }
}

/// `data` must stay alive while the map is used (names point into it).
pub fn parse(gpa: std.mem.Allocator, data: []const u8) !Map {
    var r: Reader = .{ .d = data };
    const image = try readPcx(gpa, &r);
    errdefer gpa.free(image.rgba);
    var n: usize = try r.u16le();
    var with_guards = true;
    if (n == 0) {
        n = try r.u16le();
        with_guards = false;
    }
    var list: std.ArrayList(Province) = .empty;
    errdefer {
        for (list.items) |p| gpa.free(p.shape.rgba);
        list.deinit(gpa);
    }
    for (0..n) |_| {
        const hdr = try r.take(0x18);
        const shape = try readPcx(gpa, &r);
        errdefer gpa.free(shape.rgba);
        var p: Province = .{
            .index = std.mem.readInt(i32, hdr[0..4], .little),
            .x0 = std.mem.readInt(i32, hdr[4..8], .little),
            .y0 = std.mem.readInt(i32, hdr[8..12], .little),
            .x1 = std.mem.readInt(i32, hdr[12..16], .little),
            .y1 = std.mem.readInt(i32, hdr[16..20], .little),
            .shape = shape,
            .record = undefined,
            .name = undefined,
        };
        @memcpy(&p.record, try r.take(record_size));
        const start = r.p;
        while ((try r.byte()) != 0) {}
        p.name = data[start .. r.p - 1];
        if (with_guards) {
            for (0..31) |_| try skipBlock(&r);
        }
        try list.append(gpa, p);
    }
    // (u16 y, 0x14 record: x, a, b, c, next) pairs until y >= 4000.
    var cells: std.ArrayList(Cell) = .empty;
    errdefer cells.deinit(gpa);
    while (r.p + 2 <= data.len) {
        const y = try r.u16le();
        if (y >= 4000) break;
        const c = try r.take(0x14);
        const rd = struct {
            fn f(b: []const u8, o: usize) i32 {
                return std.mem.readInt(i32, b[o..][0..4], .little);
            }
        }.f;
        try cells.append(gpa, .{ .x = rd(c, 0), .y = y, .a = rd(c, 4), .b = rd(c, 8), .c = rd(c, 12) });
    }
    const provinces = try list.toOwnedSlice(gpa);
    errdefer gpa.free(provinces);
    return .{ .image = image, .provinces = provinces, .cells = try cells.toOwnedSlice(gpa) };
}
