//! save\map%d.map of a new game as GenerateMap (GOG FUN_0046eae0 and
//! FUN_0046d7f0) writes it, before packing: the map PCX, iputw(0),
//! iputw(N), per province the 0x18 box, its shape PCX, the 0x4E4 record
//! and the CP1251 name, then (iputw(y), 0x14 cell) for every border cell
//! and iputw(0xFFFF).
//!
//! The map picture is the first layer only (000Grass tiled over the map)
//! with the borders; the landscape, water and sprite layers are not drawn
//! yet.  Taken from recomp orig/mapwrite.zig.
const std = @import("std");
const mapgen = @import("mapgen.zig");
const cp1251 = @import("../formats/cp1251.zig");

pub const black: u32 = 0x000000;
pub const magenta: u32 = 0xff00ff;

/// An RGB bitmap (Allegro 32-bit: 0xRRGGBB).
pub const Bitmap = struct {
    w: u32,
    h: u32,
    px: []u32,

    pub fn init(gpa: std.mem.Allocator, w: u32, h: u32, fill: u32) !Bitmap {
        const px = try gpa.alloc(u32, @as(usize, w) * h);
        @memset(px, fill);
        return .{ .w = w, .h = h, .px = px };
    }

    pub fn deinit(self: *Bitmap, gpa: std.mem.Allocator) void {
        gpa.free(self.px);
    }

    pub fn get(self: *const Bitmap, x: i32, y: i32) ?u32 {
        if (x < 0 or y < 0 or x >= self.w or y >= self.h) return null;
        return self.px[@as(usize, @intCast(y)) * self.w + @as(usize, @intCast(x))];
    }

    pub fn put(self: *Bitmap, x: i32, y: i32, c: u32) void {
        if (x < 0 or y < 0 or x >= self.w or y >= self.h) return;
        self.px[@as(usize, @intCast(y)) * self.w + @as(usize, @intCast(x))] = c;
    }

    /// Allegro's _blender_trans24 with n = 0..255 (draw_trans_sprite).
    pub fn blend(self: *Bitmap, x: i32, y: i32, c: u32, n_in: u32) void {
        const d = self.get(x, y) orelse return;
        var n: u32 = n_in;
        if (n != 0) n += 1;
        const rb = (((c & 0xff00ff) -% (d & 0xff00ff)) *% n / 256 +% d) & 0xff00ff;
        const g = (((c & 0xff00) -% (d & 0xff00)) *% n / 256 +% (d & 0xff00)) & 0xff00;
        self.put(x, y, rb | g);
    }
};

/// A province's bounding box (pFirstFog) and its shape: the province's
/// pixels and a one-pixel rim in black on magenta.
pub const Shape = struct {
    x0: i32,
    y0: i32,
    x1: i32,
    y1: i32,
    bmp: Bitmap = .{ .w = 0, .h = 0, .px = &.{} },
};

/// The area GenerateMap scans: X·Map_w + Map_w·7/6 by Y·(Map_h + Map_h/3)/2 + Map_h.
fn areaHeight(s: *const mapgen.Shard) i32 {
    return @divTrunc(s.y * (mapgen.map_h + @divTrunc(mapgen.map_h, 3)), 2) + mapgen.map_h;
}

/// Bounding boxes: start at (W, H, −10, −10) and grow over every map
/// point of the province.
pub fn bounds(gpa: std.mem.Allocator, s: *const mapgen.Shard) ![]Shape {
    const W = s.width();
    const H = areaHeight(s);
    const out = try gpa.alloc(Shape, s.provinces.len);
    for (out) |*b| b.* = .{ .x0 = W, .y0 = H, .x1 = -10, .y1 = -10 };
    var y: i32 = 0;
    while (y < H) : (y += 1) {
        var x: i32 = 0;
        while (x < W) : (x += 1) {
            const p = s.lookup(x, y);
            if (p <= 0) continue;
            const b = &out[@intCast(p)];
            b.x0 = @min(b.x0, x);
            b.y0 = @min(b.y0, y);
            b.x1 = @max(b.x1, x);
            b.y1 = @max(b.y1, y);
        }
    }
    return out;
}

/// The "Fog" step: (x1−x0+3)×(y1−y0+3), magenta, black where a point or
/// one of its ten neighbours belongs to the province, or the map is black.
pub fn shapes(gpa: std.mem.Allocator, s: *const mapgen.Shard, map: *const Bitmap, out: []Shape) !void {
    for (out[1..], 1..) |*b, i| try shapeOf(gpa, s, map, b, i);
}

/// The shape of province `i`.
fn shapeOf(gpa: std.mem.Allocator, s: *const mapgen.Shard, map: *const Bitmap, b: *Shape, i: usize) !void {
    const probes = [11][2]i32{ .{ 0, 0 }, .{ -1, 0 }, .{ 0, -1 }, .{ 1, 0 }, .{ 0, 1 }, .{ 1, 1 }, .{ 1, 2 }, .{ 2, 1 }, .{ 2, 2 }, .{ 0, 2 }, .{ 2, 0 } };
    const pi: i32 = @intCast(i);
    b.bmp = try Bitmap.init(gpa, @intCast(@max(b.x1 - b.x0 + 3, 1)), @intCast(@max(b.y1 - b.y0 + 3, 1)), magenta);
    var px = b.x0 - 2;
    while (px < b.x1 + 1) : (px += 1) {
        var py = b.y0 - 2;
        while (py < b.y1 + 1) : (py += 1) {
            if (px < 0 or py < 0 or px >= map.w or py >= map.h) continue;
            const hit = for (probes) |d| {
                if (s.lookup(px + d[0], py + d[1]) == pi) break true;
            } else false;
            if (!hit and map.get(px, py) != black) continue;
            b.bmp.put(px - b.x0 + 1, py - b.y0 + 1, black);
        }
    }
}

/// The map picture and the province shapes, made a province at a time so
/// that the loading screens can redraw between the steps.
pub const Pictures = struct {
    map: Bitmap,
    shapes: []Shape,
    /// The next province whose shape is still to be made.
    next: usize = 1,

    /// The first layer and the bounding boxes.
    pub fn init(gpa: std.mem.Allocator, s: *const mapgen.Shard, tile: ?Tile) !Pictures {
        var map = try baseImage(gpa, s, tile);
        errdefer map.deinit(gpa);
        return .{ .map = map, .shapes = try bounds(gpa, s) };
    }

    /// Everything at once, borders included.
    pub fn make(gpa: std.mem.Allocator, s: *const mapgen.Shard, tile: ?Tile, border_style: i32) !Pictures {
        var p = try init(gpa, s, tile);
        errdefer p.deinit(gpa);
        while (!try p.step(gpa, s)) {}
        borders(&p.map, s, border_style);
        return p;
    }

    pub fn deinit(self: *Pictures, gpa: std.mem.Allocator) void {
        freeShapes(gpa, self.shapes);
        self.map.deinit(gpa);
    }

    /// The shape of the next province; true when all are made.  The
    /// borders go on the map only after the last shape.
    pub fn step(self: *Pictures, gpa: std.mem.Allocator, s: *const mapgen.Shard) !bool {
        if (self.next < self.shapes.len) {
            try shapeOf(gpa, s, &self.map, &self.shapes[self.next], self.next);
            self.next += 1;
        }
        return self.next >= self.shapes.len;
    }
};

pub fn freeShapes(gpa: std.mem.Allocator, s: []Shape) void {
    for (s) |*x| if (x.bmp.px.len > 0) x.bmp.deinit(gpa);
    gpa.free(s);
}

/// The first layer: black, then nature.dat 000Grass (RGBA) tiled over the
/// scanned area.
pub fn baseImage(gpa: std.mem.Allocator, s: *const mapgen.Shard, tile: ?Tile) !Bitmap {
    var b = try Bitmap.init(gpa, @intCast(s.width()), @intCast(s.height()), black);
    const t = tile orelse return b;
    if (t.w == 0 or t.h == 0) return b;
    const area_h: u32 = @intCast(areaHeight(s));
    var tx: u32 = 0;
    while (tx < b.w) : (tx += t.w) {
        var ty: u32 = 0;
        while (ty < area_h) : (ty += t.h) {
            for (0..t.h) |dy| for (0..t.w) |dx| {
                const o = (dy * t.w + dx) * 4;
                const c = (@as(u32, t.rgba[o]) << 16) | (@as(u32, t.rgba[o + 1]) << 8) | t.rgba[o + 2];
                b.put(@intCast(tx + dx), @intCast(ty + dy), c);
            };
        }
    }
    return b;
}

pub const Tile = struct { w: u32, h: u32, rgba: []const u8 };

/// The "Borders" step (profile iMapBorders): 1 and 2 single pixels
/// makecol(200, 50, 50), 3 none, 0 a red pixel and a black one up-right
/// of it at 50%.  2 (the dotted sprite) is drawn as 1.
pub fn borders(b: *Bitmap, s: *const mapgen.Shard, style: i32) void {
    if (style == 3) return;
    for (0..@min(b.h, s.grid.rows())) |y| {
        var i = s.grid.heads[y];
        while (i >= 0) {
            const c = s.grid.cells.items[@intCast(i)];
            i = c.next;
            if (c.a == 0 or c.b == 0) continue;
            const yy: i32 = @intCast(y);
            if (style == 1 or style == 2) {
                b.put(c.x, yy, 0xc83232);
            } else {
                b.blend(c.x, yy, 0xc83232, 0x80);
                b.blend(c.x + 1, yy - 1, black, 0x80);
            }
        }
    }
}

/// The first 16 colours of the palette in the original's files (the
/// standard VGA ones, as _rgb_scale_6 gives them).
pub const vga16 = [48]u8{ 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0x00, 0xaa, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0xaa, 0x00, 0xaa, 0xaa, 0x55, 0x00, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55, 0x55, 0xff, 0x55, 0xff, 0x55, 0x55, 0xff, 0xff, 0xff, 0x55, 0x55, 0xff, 0x55, 0xff, 0xff, 0xff, 0x55, 0xff, 0xff, 0xff };

/// save_pcx_pf of a 32-bit bitmap: three planes per line, Allegro's RLE.
pub fn writePcx(gpa: std.mem.Allocator, out: *std.ArrayList(u8), b: *const Bitmap) !void {
    var h: [128]u8 = @splat(0);
    h[0] = 10;
    h[1] = 5;
    h[2] = 1;
    h[3] = 8;
    std.mem.writeInt(u16, h[8..10], @intCast(b.w - 1), .little);
    std.mem.writeInt(u16, h[10..12], @intCast(b.h - 1), .little);
    std.mem.writeInt(u16, h[12..14], 320, .little);
    std.mem.writeInt(u16, h[14..16], 200, .little);
    @memcpy(h[16..64], &vga16);
    h[65] = 3;
    std.mem.writeInt(u16, h[66..68], @intCast(b.w), .little);
    std.mem.writeInt(u16, h[68..70], 1, .little);
    std.mem.writeInt(u16, h[70..72], @intCast(b.w), .little);
    std.mem.writeInt(u16, h[72..74], @intCast(b.h), .little);
    try out.appendSlice(gpa, &h);
    for (0..b.h) |y| {
        const row = b.px[y * b.w ..][0..b.w];
        var run: u32 = 0;
        var ch: u8 = 0;
        for (0..3) |plane| {
            const shift: u5 = @intCast(16 - 8 * plane);
            for (row) |c| {
                const v: u8 = @truncate(c >> shift);
                if (run == 0) {
                    run = 1;
                    ch = v;
                } else if (v != ch or run >= 0x3f) {
                    if (run > 1 or ch & 0xc0 == 0xc0) try out.append(gpa, @intCast(0xc0 | run));
                    try out.append(gpa, ch);
                    run = 1;
                    ch = v;
                } else run += 1;
            }
        }
        if (run > 1 or ch & 0xc0 == 0xc0) try out.append(gpa, @intCast(0xc0 | run));
        try out.append(gpa, ch);
    }
}

fn putW(gpa: std.mem.Allocator, out: *std.ArrayList(u8), v: u16) !void {
    var b: [2]u8 = undefined;
    std.mem.writeInt(u16, &b, v, .little);
    try out.appendSlice(gpa, &b);
}

fn putI(gpa: std.mem.Allocator, out: *std.ArrayList(u8), v: i32) !void {
    var b: [4]u8 = undefined;
    std.mem.writeInt(i32, &b, v, .little);
    try out.appendSlice(gpa, &b);
}

/// FUN_0046d7f0, unpacked.
pub fn mapBytes(gpa: std.mem.Allocator, s: *const mapgen.Shard, map: *const Bitmap, sh: []const Shape) ![]u8 {
    var out: std.ArrayList(u8) = .empty;
    errdefer out.deinit(gpa);
    try writePcx(gpa, &out, map);
    try putW(gpa, &out, 0);
    try putW(gpa, &out, @intCast(s.count()));
    for (s.provinces[1..], 1..) |*pv, i| {
        try putI(gpa, &out, @intCast(i));
        for ([4]i32{ sh[i].x0, sh[i].y0, sh[i].x1, sh[i].y1 }) |v| try putI(gpa, &out, v);
        try putI(gpa, &out, 0);
        try writePcx(gpa, &out, &sh[i].bmp);
        try out.appendSlice(gpa, &pv.toRecord());
        const name = try cp1251.fromUtf8Alloc(gpa, pv.name);
        defer gpa.free(name);
        try out.appendSlice(gpa, name);
        try out.append(gpa, 0);
    }
    for (0..s.grid.rows()) |y| {
        var i = s.grid.heads[y];
        while (i >= 0) {
            const c = s.grid.cells.items[@intCast(i)];
            i = c.next;
            try putW(gpa, &out, @intCast(y));
            for ([4]i32{ c.x, c.a, c.b, c.c }) |v| try putI(gpa, &out, v);
            try putI(gpa, &out, 0);
        }
    }
    try putW(gpa, &out, 0xffff);
    return out.toOwnedSlice(gpa);
}

/// Picture, fog shapes and borders, then the file.  `gpa` owns the result.
pub fn build(gpa: std.mem.Allocator, s: *const mapgen.Shard, tile: ?Tile, border_style: i32) ![]u8 {
    var p = try Pictures.make(gpa, s, tile, border_style);
    defer p.deinit(gpa);
    return mapBytes(gpa, s, &p.map, p.shapes);
}

const tst = std.testing;
const GameFs = @import("../platform/fs.zig").GameFs;
const packfile = @import("../formats/packfile.zig");
const mapfile = @import("../formats/mapfile.zig");
const savefile = @import("../formats/savefile.zig");
const profile = @import("../data/profile.zig");
const proto_mod = @import("../data/proto.zig");
const Rand = @import("../data/rand.zig").Rand;
const png = @import("../formats/png.zig");

fn readTestFile(gpa: std.mem.Allocator, path: []const u8) ![]u8 {
    const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, gpa, .unlimited) catch return error.SkipZigTest;
    defer gpa.free(bytes);
    return packfile.unpack(gpa, bytes);
}

fn checkMap(dir: []const u8, game: []const u8, map: []const u8) !void {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try proto_mod.Proto.load(gpa, &fs);
    defer pr.deinit();
    var pb: [96]u8 = undefined;
    const gdata = try readTestFile(gpa, try std.fmt.bufPrint(&pb, "testdata/{s}/{s}", .{ dir, game }));
    defer gpa.free(gdata);
    const mdata = try readTestFile(gpa, try std.fmt.bufPrint(&pb, "testdata/{s}/{s}", .{ dir, map }));
    defer gpa.free(mdata);
    var ref = try mapfile.parse(gpa, mdata);
    defer ref.deinit(gpa);
    var save = try savefile.parse(gpa, gdata, ref.provinces.len);
    defer save.deinit(gpa);
    const hdr = gdata[3..][0..0x13c];
    var settings: profile.Map = undefined;
    inline for (@typeInfo(profile.Map).@"struct".field_names, 0..) |f, k|
        @field(settings, f) = savefile.int(hdr, 0x100 + 4 * k);
    const n_players: usize = @intCast(savefile.int(hdr, 0x38));
    var pls: [mapgen.max_players]mapgen.Player = undefined;
    for (pls[0..n_players], 1..) |*pl, k| {
        const rec = save.player(k);
        pl.* = .{ .kind = @fromBackingInt(@intCast(savefile.int(rec, 0))), .ruler = savefile.int(rec, 8), .difficulty = savefile.int(rec, 0xc) };
    }
    // The human's capital takes the profile's land name.
    var lands: [mapgen.max_players][]const u8 = undefined;
    for (pls[0..n_players], 0..) |*pl, k| if (pl.kind == .human) {
        const p: usize = @intCast(save.home[k + 1]);
        lands[k] = try cp1251.toUtf8Alloc(gpa, std.mem.sliceTo(ref.provinces[p - 1].name, 0));
        pl.land = lands[k];
    };
    defer for (pls[0..n_players], 0..) |pl, k| if (pl.kind == .human) gpa.free(lands[k]);
    var rand: Rand = .{};
    var s = try mapgen.generate(gpa, &pr, &rand, .{
        .seed = @bitCast(savefile.int(hdr, 0)),
        .map = settings,
        .game_type = savefile.int(hdr, 8),
        .players = pls[0..n_players],
    });
    defer s.deinit();
    const tile_png = try fs.read("sprites/nature/000_000Grass.png");
    defer gpa.free(tile_png);
    const tile = try png.decode(gpa, tile_png);
    defer gpa.free(tile.rgba);
    const raw = try build(gpa, &s, .{ .w = tile.w, .h = tile.h, .rgba = tile.rgba }, 0);
    defer gpa.free(raw);
    var got = try mapfile.parse(gpa, raw);
    defer got.deinit(gpa);
    try tst.expectEqual(ref.image.w, got.image.w);
    try tst.expectEqual(ref.image.h, got.image.h);
    try tst.expectEqual(ref.provinces.len, got.provinces.len);
    for (ref.provinces, got.provinces) |*w, *g| {
        try tst.expectEqual([5]i32{ w.index, w.x0, w.y0, w.x1, w.y1 }, [5]i32{ g.index, g.x0, g.y0, g.x1, g.y1 });
        try tst.expectEqualStrings(w.name, g.name);
        try tst.expectEqual(w.shape.w, g.shape.w);
        try tst.expectEqual(w.shape.h, g.shape.h);
        // The shape: black where the province is; the original's map also
        // has black where its landscape is black, so only check the province.
        var o: usize = 0;
        while (o < g.shape.rgba.len) : (o += 4) {
            if (g.shape.rgba[o + 3] != 0 and w.shape.rgba[o + 3] == 0) {
                std.debug.print("\nprovince {d}: shape pixel {d} set, not in the original\n", .{ w.index, o / 4 });
                return error.TestExpectedEqual;
            }
        }
        var off: usize = 0;
        while (off < mapfile.record_size) : (off += 4) {
            const skip = off == 0 or off == 0x48 or off == 0x4c or off == 0x14c or off == 0x150 or off == 0x160 or off == 0x17c or off == 0x180 or off >= 0x454;
            if (skip) continue;
            if (w.int(off) != g.int(off)) {
                std.debug.print("\nprovince {d} +0x{x}: want {d} got {d}\n", .{ w.index, off, w.int(off), g.int(off) });
                return error.TestExpectedEqual;
            }
        }
    }
    try tst.expectEqual(ref.cells.len, got.cells.len);
    for (ref.cells, got.cells) |w, g| try tst.expectEqual(w, g);
}

test "new map file against the original: single player" {
    try checkMap("new_single", "game0.sav", "map0.map");
}

test "new map file against the original: hot seat" {
    try checkMap("new_hot", "game0.hot", "map0.hot");
}
