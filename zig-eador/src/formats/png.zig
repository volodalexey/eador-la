//! Minimal PNG encoder for the importer: 8-bit RGB or RGBA, one IDAT,
//! "Up" row filter, zlib from std.compress.flate.
const std = @import("std");
const flate = std.compress.flate;

/// Encodes `rgba` (w*h*4 bytes).  Images without transparent pixels are
/// stored as RGB, which is a quarter smaller.
pub fn encodeRgba(gpa: std.mem.Allocator, w: u32, h: u32, rgba: []const u8) ![]u8 {
    const n = @as(usize, w) * h;
    if (rgba.len < n * 4) return error.BadImage;
    const opaque_ = for (0..n) |i| {
        if (rgba[i * 4 + 3] != 255) break false;
    } else true;
    const bpp: usize = if (opaque_) 3 else 4;
    const stride = @as(usize, w) * bpp;

    // Filtered scanlines: filter byte 2 ("Up"), then cur - prev per byte.
    const raw = try gpa.alloc(u8, h * (stride + 1));
    defer gpa.free(raw);
    const row = try gpa.alloc(u8, stride * 2);
    defer gpa.free(row);
    const cur = row[0..stride];
    const prev = row[stride..];
    @memset(prev, 0);
    for (0..h) |y| {
        for (0..w) |x| {
            const s = rgba[(y * w + x) * 4 ..][0..4];
            @memcpy(cur[x * bpp ..][0..bpp], s[0..bpp]);
        }
        const dst = raw[y * (stride + 1) ..][0 .. stride + 1];
        dst[0] = 2;
        for (dst[1..], cur, prev) |*d, c, p| d.* = c -% p;
        @memcpy(prev, cur);
    }

    var z: std.Io.Writer.Allocating = try .initCapacity(gpa, raw.len / 3 + 64);
    defer z.deinit();
    const window = try gpa.alloc(u8, flate.max_window_len);
    defer gpa.free(window);
    const c = try gpa.create(flate.Compress);
    defer gpa.destroy(c);
    c.* = try .init(&z.writer, window, .zlib, .level_4);
    try c.writer.writeAll(raw);
    try c.finish();
    const idat = z.written();

    var out: std.ArrayList(u8) = try .initCapacity(gpa, idat.len + 64);
    errdefer out.deinit(gpa);
    out.appendSliceAssumeCapacity("\x89PNG\r\n\x1a\n");
    var ihdr: [13]u8 = undefined;
    std.mem.writeInt(u32, ihdr[0..4], w, .big);
    std.mem.writeInt(u32, ihdr[4..8], h, .big);
    ihdr[8..13].* = .{ 8, if (opaque_) 2 else 6, 0, 0, 0 };
    try chunk(gpa, &out, "IHDR", &ihdr);
    try chunk(gpa, &out, "IDAT", idat);
    try chunk(gpa, &out, "IEND", "");
    return out.toOwnedSlice(gpa);
}

fn chunk(gpa: std.mem.Allocator, out: *std.ArrayList(u8), tag: *const [4]u8, data: []const u8) !void {
    var len: [4]u8 = undefined;
    std.mem.writeInt(u32, &len, @intCast(data.len), .big);
    try out.appendSlice(gpa, &len);
    try out.appendSlice(gpa, tag);
    try out.appendSlice(gpa, data);
    var crc = std.hash.Crc32.init();
    crc.update(tag);
    crc.update(data);
    var sum: [4]u8 = undefined;
    std.mem.writeInt(u32, &sum, crc.final(), .big);
    try out.appendSlice(gpa, &sum);
}

pub const Image = struct {
    w: u32,
    h: u32,
    /// RGBA8.
    rgba: []u8,
};

/// Decodes the importer's PNGs (8-bit RGB or RGBA, not interlaced) without
/// raylib, for code that runs in the tests.
pub fn decode(gpa: std.mem.Allocator, png: []const u8) !Image {
    if (png.len < 33 or !std.mem.eql(u8, png[0..8], "\x89PNG\r\n\x1a\n")) return error.BadPng;
    var p: usize = 8;
    var w: u32 = 0;
    var h: u32 = 0;
    var bpp: usize = 0;
    var idat: std.ArrayList(u8) = .empty;
    defer idat.deinit(gpa);
    while (p + 8 <= png.len) {
        const len = std.mem.readInt(u32, png[p..][0..4], .big);
        const tag = png[p + 4 ..][0..4];
        if (p + 12 + len > png.len) return error.BadPng;
        const data = png[p + 8 ..][0..len];
        if (std.mem.eql(u8, tag, "IHDR")) {
            w = std.mem.readInt(u32, data[0..4], .big);
            h = std.mem.readInt(u32, data[4..8], .big);
            if (data[8] != 8 or data[12] != 0) return error.UnsupportedPng;
            bpp = switch (data[9]) {
                2 => 3,
                6 => 4,
                else => return error.UnsupportedPng,
            };
        } else if (std.mem.eql(u8, tag, "IDAT")) {
            try idat.appendSlice(gpa, data);
        } else if (std.mem.eql(u8, tag, "IEND")) break;
        p += 12 + len;
    }
    if (bpp == 0) return error.BadPng;
    const stride = @as(usize, w) * bpp;
    const raw = try gpa.alloc(u8, h * (stride + 1));
    defer gpa.free(raw);
    var in: std.Io.Reader = .fixed(idat.items);
    const window = try gpa.alloc(u8, flate.max_window_len);
    defer gpa.free(window);
    var d: flate.Decompress = .init(&in, .zlib, window);
    try d.reader.readSliceAll(raw);
    const rgba = try gpa.alloc(u8, @as(usize, w) * h * 4);
    errdefer gpa.free(rgba);
    const prev = try gpa.alloc(u8, stride);
    defer gpa.free(prev);
    @memset(prev, 0);
    for (0..h) |y| {
        const line = raw[y * (stride + 1) ..][0 .. stride + 1];
        const cur = line[1..];
        for (cur, 0..) |*c, i| {
            const a: u8 = if (i >= bpp) cur[i - bpp] else 0;
            const b = prev[i];
            const cc: u8 = if (i >= bpp) prev[i - bpp] else 0;
            c.* +%= switch (line[0]) {
                0 => 0,
                1 => a,
                2 => b,
                3 => @intCast((@as(u16, a) + b) / 2),
                4 => paeth(a, b, cc),
                else => return error.BadPng,
            };
        }
        @memcpy(prev, cur);
        for (0..w) |x| {
            const s = cur[x * bpp ..][0..bpp];
            rgba[(y * w + x) * 4 ..][0..4].* = .{ s[0], s[1], s[2], if (bpp == 4) s[3] else 255 };
        }
    }
    return .{ .w = w, .h = h, .rgba = rgba };
}

fn paeth(a: u8, b: u8, c: u8) u8 {
    const p: i16 = @as(i16, a) + b - c;
    const pa = @abs(p - a);
    const pb = @abs(p - b);
    const pc = @abs(p - c);
    if (pa <= pb and pa <= pc) return a;
    if (pb <= pc) return b;
    return c;
}

test "png decode reads what encodeRgba writes" {
    const gpa = std.testing.allocator;
    const px = [_]u8{ 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 255, 9, 9, 9, 255 };
    const png = try encodeRgba(gpa, 2, 2, &px);
    defer gpa.free(png);
    const img = try decode(gpa, png);
    defer gpa.free(img.rgba);
    try std.testing.expectEqual(@as(u32, 2), img.w);
    try std.testing.expectEqualSlices(u8, &px, img.rgba);
}

test "png round trip through the zlib stream" {
    const gpa = std.testing.allocator;
    const px = [_]u8{ 255, 0, 0, 255, 0, 255, 0, 0, 0, 0, 255, 255, 9, 9, 9, 255 };
    const png = try encodeRgba(gpa, 2, 2, &px);
    defer gpa.free(png);
    try std.testing.expectEqualSlices(u8, "\x89PNG\r\n\x1a\n", png[0..8]);
    // IHDR: 2x2, 8-bit RGBA (one pixel is transparent).
    try std.testing.expectEqual(@as(u8, 6), png[8 + 8 + 9]);
    const idat_len = std.mem.readInt(u32, png[33..37], .big);
    try std.testing.expectEqualSlices(u8, "IDAT", png[37..41]);
    var in: std.Io.Reader = .fixed(png[41..][0..idat_len]);
    var buf: [flate.max_window_len]u8 = undefined;
    var d: flate.Decompress = .init(&in, .zlib, &buf);
    var raw: [2 * (1 + 8)]u8 = undefined;
    try d.reader.readSliceAll(&raw);
    // Row 0 is stored as-is (Up from zero), row 1 as the difference.
    try std.testing.expectEqualSlices(u8, &.{ 2, 255, 0, 0, 255, 0, 255, 0, 0 }, raw[0..9]);
    try std.testing.expectEqual(@as(u8, 2), raw[9]);
    try std.testing.expectEqual(@as(u8, @as(u8, 0) -% 255), raw[10]);
}
