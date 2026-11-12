//! Game bitmap fonts: fontNN.pcx (8-bit PCX, RLE).
//!
//! The PCX is an Allegro grabber grid: palette index 255 forms the cell
//! borders, index 0 is background, any other index is glyph ink. Cells are
//! read left-to-right, top-to-bottom. Which character each cell holds comes
//! from fontNN.txt in the original ("font01.pcx 0x20 0xFF", CP1251 codes);
//! the importer turns that into fontNN.chars (UTF-8).
const std = @import("std");

pub const Glyph = struct {
    x: u32,
    y: u32,
    w: u32,
    h: u32,
    /// Cell width: Allegro's import_bitmap_font makes every glyph its box,
    /// so the text advances by it (9 px in font01/font02).
    cell_w: u32 = 0,
};

pub const Font = struct {
    /// RGBA8 atlas, same size as the PCX. Ink is white, tinted at draw time.
    width: u32,
    height: u32,
    pixels: []u8,
    /// Glyphs in cell order.
    glyphs: []Glyph,
    line_h: u32,

    pub fn deinit(self: *Font, gpa: std.mem.Allocator) void {
        gpa.free(self.pixels);
        gpa.free(self.glyphs);
    }
};

pub const Pcx = struct {
    w: u32,
    h: u32,
    index: []u8,
};

pub fn decodePcx(gpa: std.mem.Allocator, d: []const u8) !Pcx {
    if (d.len < 128 + 769 or d[0] != 0x0A or d[3] != 8 or d[65] != 1) return error.UnsupportedPcx;
    const xmin = std.mem.readInt(u16, d[4..6], .little);
    const ymin = std.mem.readInt(u16, d[6..8], .little);
    const xmax = std.mem.readInt(u16, d[8..10], .little);
    const ymax = std.mem.readInt(u16, d[10..12], .little);
    const bpl: usize = std.mem.readInt(u16, d[66..68], .little);
    if (xmax < xmin or ymax < ymin) return error.UnsupportedPcx;
    const w: u32 = @as(u32, xmax) - xmin + 1;
    const h: u32 = @as(u32, ymax) - ymin + 1;
    if (bpl < w) return error.UnsupportedPcx;

    const raw = try gpa.alloc(u8, bpl * h);
    defer gpa.free(raw);
    @memset(raw, 0);
    var o: usize = 0;
    var i: usize = 128;
    const end = d.len - 769;
    while (o < raw.len and i < end) {
        const c = d[i];
        i += 1;
        if (c >= 0xC0) {
            if (i >= end) break;
            const n = @min(@as(usize, c & 0x3F), raw.len - o);
            @memset(raw[o..][0..n], d[i]);
            i += 1;
            o += n;
        } else {
            raw[o] = c;
            o += 1;
        }
    }
    const index = try gpa.alloc(u8, @as(usize, w) * h);
    for (0..h) |y| @memcpy(index[y * w ..][0..w], raw[y * bpl ..][0..w]);
    return .{ .w = w, .h = h, .index = index };
}

/// Parses the original "font01.pcx 0x20 0xFF" descriptor (used by the importer).
pub fn parseRange(txt: []const u8) !struct { first: u8, last: u8 } {
    var it = std.mem.tokenizeAny(u8, txt, " \t\r\n");
    _ = it.next() orelse return error.BadFontDesc;
    const a = it.next() orelse return error.BadFontDesc;
    const b = it.next() orelse return error.BadFontDesc;
    const first = try std.fmt.parseInt(u8, a, 0);
    const last = try std.fmt.parseInt(u8, b, 0);
    if (last < first) return error.BadFontDesc;
    return .{ .first = first, .last = last };
}

/// Loads up to `count` glyph cells from the PCX grid.
pub fn load(gpa: std.mem.Allocator, pcx_bytes: []const u8, count: usize) !Font {
    const pcx = try decodePcx(gpa, pcx_bytes);
    defer gpa.free(pcx.index);
    const W = pcx.w;
    const H = pcx.h;
    const at = struct {
        fn f(p: Pcx, x: usize, y: usize) u8 {
            return p.index[y * p.w + x];
        }
    }.f;

    // Grid lines: rows/cols made entirely of index 255.
    var rows: std.ArrayList(u32) = .empty;
    defer rows.deinit(gpa);
    var cols: std.ArrayList(u32) = .empty;
    defer cols.deinit(gpa);
    for (0..H) |y| {
        const full = for (0..W) |x| {
            if (at(pcx, x, y) != 255) break false;
        } else true;
        if (full) try rows.append(gpa, @intCast(y));
    }
    for (0..W) |x| {
        const full = for (0..H) |y| {
            if (at(pcx, x, y) != 255) break false;
        } else true;
        if (full) try cols.append(gpa, @intCast(x));
    }

    var glyphs: std.ArrayList(Glyph) = .empty;
    errdefer glyphs.deinit(gpa);
    var line_h: u32 = 0;
    outer: for (0..rows.items.len -| 1) |ri| {
        const y0 = rows.items[ri] + 1;
        const y1 = rows.items[ri + 1];
        if (y1 <= y0) continue;
        for (0..cols.items.len -| 1) |ci| {
            if (glyphs.items.len == count) break :outer;
            const x0 = cols.items[ci] + 1;
            const x1 = cols.items[ci + 1];
            if (x1 <= x0) continue;
            var right: u32 = 0;
            for (y0..y1) |y| for (x0..x1) |x| {
                const v = at(pcx, x, y);
                if (v != 0 and v != 255) right = @max(right, @as(u32, @intCast(x)) - x0 + 1);
            };
            // Empty cells (space) get ~40% of the cell width.
            const w = if (right == 0) @max(1, (x1 - x0) * 2 / 5) else right;
            try glyphs.append(gpa, .{ .x = x0, .y = y0, .w = w, .h = y1 - y0, .cell_w = x1 - x0 });
            line_h = @max(line_h, y1 - y0);
        }
    }

    const pixels = try gpa.alloc(u8, @as(usize, W) * H * 4);
    for (pcx.index, 0..) |v, i| {
        pixels[i * 4 ..][0..4].* = if (v == 0 or v == 255) .{ 0, 0, 0, 0 } else .{ 255, 255, 255, 255 };
    }
    return .{
        .width = W,
        .height = H,
        .pixels = pixels,
        .glyphs = try glyphs.toOwnedSlice(gpa),
        .line_h = line_h,
    };
}
