//! raylib-side helpers over the imported files: sprite sheets
//! (sprites/<sheet>/NNN_<name>.png + index.txt) and BMFont text fonts
//! (fonts/fontNN.fnt + .png).  All text handled here is UTF-8.
const std = @import("std");
const rl = @import("raylib");
const gamefs = @import("fs.zig");
const GameFs = gamefs.GameFs;

pub fn textureFromRgba(w: u32, h: u32, pixels: []u8) !rl.Texture2D {
    const img: rl.Image = .{
        .data = pixels.ptr,
        .width = @intCast(w),
        .height = @intCast(h),
        .mipmaps = 1,
        .format = .uncompressed_r8g8b8a8,
    };
    return rl.loadTextureFromImage(img);
}

/// Loads a PNG through the asset lookup (language dir first, then common/).
pub fn loadPng(fs: *GameFs, rel: []const u8) ?rl.Texture2D {
    const bytes = fs.read(rel) catch return null;
    defer fs.gpa.free(bytes);
    const img = rl.loadImageFromMemory(".png", bytes) catch return null;
    defer rl.unloadImage(img);
    return rl.loadTextureFromImage(img) catch null;
}

/// One former datafile: its bitmap objects as lazily uploaded textures.
/// Objects keep their datafile position (`index`) and name.
pub const Sheet = struct {
    fs: *GameFs,
    gpa: std.mem.Allocator,
    dir: []u8,
    index_text: []u8,
    entries: []gamefs.IndexEntry,
    cache: std.AutoHashMapUnmanaged(u32, rl.Texture2D) = .empty,
    /// White silhouettes for Allegro's draw_lit_sprite (see `maskIndex`).
    masks: std.AutoHashMapUnmanaged(u32, rl.Texture2D) = .empty,

    /// `name` is the sheet ("interface", "town/town1").
    pub fn load(fs: *GameFs, name: []const u8) !Sheet {
        const gpa = fs.gpa;
        const dir = try std.fmt.allocPrint(gpa, "sprites/{s}", .{name});
        errdefer gpa.free(dir);
        var pb: [256]u8 = undefined;
        const text = try fs.read(try std.fmt.bufPrint(&pb, "{s}/index.txt", .{dir}));
        errdefer gpa.free(text);
        const entries = try gamefs.parseIndex(gpa, text);
        return .{ .fs = fs, .gpa = gpa, .dir = dir, .index_text = text, .entries = entries };
    }

    pub fn deinit(self: *Sheet) void {
        var it = self.cache.valueIterator();
        while (it.next()) |t| rl.unloadTexture(t.*);
        self.cache.deinit(self.gpa);
        var mt = self.masks.valueIterator();
        while (mt.next()) |t| rl.unloadTexture(t.*);
        self.masks.deinit(self.gpa);
        self.gpa.free(self.entries);
        self.gpa.free(self.index_text);
        self.gpa.free(self.dir);
    }

    pub fn entryTexture(self: *Sheet, e: gamefs.IndexEntry) ?rl.Texture2D {
        if (self.cache.get(e.index)) |t| return t;
        var pb: [320]u8 = undefined;
        const path = std.fmt.bufPrint(&pb, "{s}/{d:0>3}_{s}.png", .{ self.dir, e.index, e.name }) catch return null;
        const t = loadPng(self.fs, path) orelse return null;
        self.cache.put(self.gpa, e.index, t) catch {};
        return t;
    }

    /// Texture by object NAME (case-insensitive, like Allegro), null if missing.
    pub fn tex(self: *Sheet, name: []const u8) ?rl.Texture2D {
        for (self.entries) |e| {
            if (std.ascii.eqlIgnoreCase(e.name, name)) return self.entryTexture(e);
        }
        return null;
    }

    /// Texture by object POSITION in the original datafile (innerbld.dat:
    /// index = building id, the first object is "000Empty").
    pub fn texIndex(self: *Sheet, i: usize) ?rl.Texture2D {
        for (self.entries) |e| {
            if (e.index == i) return self.entryTexture(e);
        }
        return null;
    }

    /// The object as a CPU image (RGBA8) for pixel tests (getpixel on masks).
    /// The caller unloads it.
    pub fn imageIndex(self: *Sheet, i: usize) ?rl.Image {
        for (self.entries) |e| {
            if (e.index != i) continue;
            var pb: [320]u8 = undefined;
            const path = std.fmt.bufPrint(&pb, "{s}/{d:0>3}_{s}.png", .{ self.dir, e.index, e.name }) catch return null;
            const bytes = self.fs.read(path) catch return null;
            defer self.gpa.free(bytes);
            var img = rl.loadImageFromMemory(".png", bytes) catch return null;
            rl.imageFormat(&img, .uncompressed_r8g8b8a8);
            return img;
        }
        return null;
    }

    /// The object's opaque pixels in white.  Drawn with a tint it gives
    /// Allegro's draw_lit_sprite at full strength (a solid colour).
    pub fn maskIndex(self: *Sheet, i: usize) ?rl.Texture2D {
        const key: u32 = @intCast(i);
        if (self.masks.get(key)) |t| return t;
        for (self.entries) |e| {
            if (e.index != i) continue;
            var pb: [320]u8 = undefined;
            const path = std.fmt.bufPrint(&pb, "{s}/{d:0>3}_{s}.png", .{ self.dir, e.index, e.name }) catch return null;
            const bytes = self.fs.read(path) catch return null;
            defer self.gpa.free(bytes);
            var img = rl.loadImageFromMemory(".png", bytes) catch return null;
            defer rl.unloadImage(img);
            rl.imageFormat(&img, .uncompressed_r8g8b8a8);
            const n: usize = @intCast(img.width * img.height);
            const px: [*]u8 = @ptrCast(img.data);
            for (0..n) |k| {
                if (px[k * 4 + 3] != 0) px[k * 4 ..][0..3].* = .{ 255, 255, 255 };
            }
            const t = rl.loadTextureFromImage(img) catch return null;
            self.masks.put(self.gpa, key, t) catch {};
            return t;
        }
        return null;
    }
};

/// Decodes one code point; invalid bytes yield U+FFFD.
fn nextCp(s: []const u8, i: *usize) u21 {
    const len = std.unicode.utf8ByteSequenceLength(s[i.*]) catch {
        i.* += 1;
        return 0xFFFD;
    };
    if (i.* + len > s.len) {
        i.* = s.len;
        return 0xFFFD;
    }
    const cp = std.unicode.utf8Decode(s[i.*..][0..len]) catch 0xFFFD;
    i.* += len;
    return cp;
}

pub const Glyph = struct { x: u32, y: u32, w: u32, h: u32, advance: u32 };

/// The fields of an AngelCode BMFont text file that the importer writes.
pub const FontData = struct {
    line_h: u32 = 0,
    page: []const u8 = "",
    map: std.AutoHashMapUnmanaged(u21, Glyph) = .empty,

    pub fn deinit(self: *FontData, gpa: std.mem.Allocator) void {
        self.map.deinit(gpa);
    }
};

fn field(line: []const u8, key: []const u8) ?[]const u8 {
    var it = std.mem.tokenizeScalar(u8, line, ' ');
    while (it.next()) |tok| {
        if (tok.len > key.len and std.mem.startsWith(u8, tok, key) and tok[key.len] == '=')
            return std.mem.trim(u8, tok[key.len + 1 ..], "\"");
    }
    return null;
}

fn num(line: []const u8, key: []const u8) u32 {
    return std.fmt.parseInt(u32, field(line, key) orelse return 0, 10) catch 0;
}

/// `page` points into `text`.
pub fn parseFnt(gpa: std.mem.Allocator, text: []const u8) !FontData {
    var f: FontData = .{};
    errdefer f.deinit(gpa);
    var lines = std.mem.tokenizeScalar(u8, text, '\n');
    while (lines.next()) |l| {
        if (std.mem.startsWith(u8, l, "common ")) {
            f.line_h = num(l, "lineHeight");
        } else if (std.mem.startsWith(u8, l, "page ")) {
            f.page = field(l, "file") orelse "";
        } else if (std.mem.startsWith(u8, l, "char ")) {
            const cp = std.math.cast(u21, num(l, "id")) orelse continue;
            try f.map.put(gpa, cp, .{
                .x = num(l, "x"),
                .y = num(l, "y"),
                .w = num(l, "width"),
                .h = num(l, "height"),
                .advance = num(l, "xadvance"),
            });
        }
    }
    return f;
}

pub const Font = struct {
    data: FontData,
    text: []u8,
    texture: rl.Texture2D,

    /// `name` is "font01" or "font02": fonts/<name>.fnt and its page PNG.
    pub fn load(fs: *GameFs, name: []const u8) !Font {
        const gpa = fs.gpa;
        var pb: [64]u8 = undefined;
        const text = try fs.read(try std.fmt.bufPrint(&pb, "fonts/{s}.fnt", .{name}));
        errdefer gpa.free(text);
        var data = try parseFnt(gpa, text);
        errdefer data.deinit(gpa);
        const t = loadPng(fs, try std.fmt.bufPrint(&pb, "fonts/{s}", .{data.page})) orelse return error.FontAtlas;
        return .{ .data = data, .text = text, .texture = t };
    }

    pub fn deinit(self: *Font, gpa: std.mem.Allocator) void {
        rl.unloadTexture(self.texture);
        self.data.deinit(gpa);
        gpa.free(self.text);
    }

    fn glyph(self: *const Font, cp: u21) ?Glyph {
        return self.data.map.get(cp) orelse self.data.map.get('?');
    }

    pub fn measure(self: *const Font, s: []const u8) f32 {
        var w: f32 = 0;
        var i: usize = 0;
        while (i < s.len) {
            const g = self.glyph(nextCp(s, &i)) orelse continue;
            w += @floatFromInt(g.advance);
        }
        return w;
    }

    pub fn draw(self: *const Font, s: []const u8, x: f32, y: f32, color: rl.Color) void {
        var cx = x;
        var i: usize = 0;
        while (i < s.len) {
            const g = self.glyph(nextCp(s, &i)) orelse continue;
            const src: rl.Rectangle = .{
                .x = @floatFromInt(g.x),
                .y = @floatFromInt(g.y),
                .width = @floatFromInt(g.w),
                .height = @floatFromInt(g.h),
            };
            rl.drawTextureRec(self.texture, src, .{ .x = @round(cx), .y = @round(y) }, color);
            cx += @floatFromInt(g.advance);
        }
    }

    pub fn drawCentered(self: *const Font, s: []const u8, cx: f32, y: f32, color: rl.Color) void {
        self.draw(s, cx - self.measure(s) / 2, y, color);
    }

    /// Word-wrapped text inside `max_w`. Returns the y after the last line.
    pub fn drawWrapped(self: *const Font, s: []const u8, x: f32, y0: f32, max_w: f32, color: rl.Color) f32 {
        return self.drawWrappedClip(s, x, y0, max_w, std.math.inf(f32), color);
    }

    /// As drawWrapped, but lines starting at or below `max_y` are skipped
    /// (the original's text boxes clip to a whole number of lines).
    pub fn drawWrappedClip(self: *const Font, s: []const u8, x: f32, y0: f32, max_w: f32, max_y: f32, color: rl.Color) f32 {
        var y = y0;
        var paras = std.mem.splitScalar(u8, s, '\n');
        while (paras.next()) |para| {
            var start: usize = 0;
            var last_space: ?usize = null;
            var i: usize = 0;
            while (i <= para.len) : (i += 1) {
                const at_end = i == para.len;
                if (!at_end and para[i] != ' ') continue;
                if (self.measure(para[start..i]) > max_w) {
                    if (last_space) |sp| {
                        if (y + self.lineHeight() <= max_y) self.draw(para[start..sp], x, y, color);
                        y += self.lineHeight() + 2;
                        start = sp + 1;
                        last_space = null;
                        i = sp;
                        continue;
                    }
                }
                if (at_end) break;
                last_space = i;
            }
            if (y + self.lineHeight() <= max_y) self.draw(para[start..], x, y, color);
            y += self.lineHeight() + 2;
        }
        return y;
    }

    pub fn lineHeight(self: *const Font) f32 {
        return @floatFromInt(self.data.line_h);
    }
};

/// Set by a screen that draws its own mouse sprite this frame (a held unit
/// in the garrison): the normal cursor is skipped, then the flag resets.
pub var hide_cursor: bool = false;

pub fn drawTex(t: ?rl.Texture2D, x: f32, y: f32) void {
    if (t) |tx| rl.drawTextureV(tx, .{ .x = x, .y = y }, .white);
}

/// drawTex with a tint (draw_trans_sprite: alpha < 255).
pub fn drawTex2(t: ?rl.Texture2D, x: f32, y: f32, tint: rl.Color) void {
    if (t) |tx| rl.drawTextureV(tx, .{ .x = x, .y = y }, tint);
}

/// Draws the part of `t` at (x, y) that lies inside `clip`: a sprite drawn
/// on a window bitmap (bmpWindow) is cut at the window's edges.
pub fn drawClipped(t: ?rl.Texture2D, x: f32, y: f32, tint: rl.Color, clip: rl.Rectangle) void {
    const tx = t orelse return;
    const x0 = @max(x, clip.x);
    const y0 = @max(y, clip.y);
    const x1 = @min(x + @as(f32, @floatFromInt(tx.width)), clip.x + clip.width);
    const y1 = @min(y + @as(f32, @floatFromInt(tx.height)), clip.y + clip.height);
    if (x1 <= x0 or y1 <= y0) return;
    rl.drawTextureRec(tx, .{ .x = x0 - x, .y = y0 - y, .width = x1 - x0, .height = y1 - y0 }, .{ .x = x0, .y = y0 }, tint);
}

pub fn texW(t: ?rl.Texture2D) f32 {
    return if (t) |tx| @floatFromInt(tx.width) else 0;
}

pub fn texH(t: ?rl.Texture2D) f32 {
    return if (t) |tx| @floatFromInt(tx.height) else 0;
}

test "bmfont lines" {
    const gpa = std.testing.allocator;
    var f = try parseFnt(gpa,
        \\info face="font01" size=13
        \\common lineHeight=13 base=13 scaleW=256 scaleH=128 pages=1 packed=0
        \\page id=0 file="font01.png"
        \\chars count=1
        \\char id=1040 x=10 y=20 width=7 height=13 xoffset=0 yoffset=0 xadvance=8 page=0 chnl=15
        \\
    );
    defer f.deinit(gpa);
    try std.testing.expectEqual(@as(u32, 13), f.line_h);
    try std.testing.expectEqualStrings("font01.png", f.page);
    const g = f.map.get(0x410).?;
    try std.testing.expectEqual(@as(u32, 8), g.advance);
    try std.testing.expectEqual(@as(u32, 20), g.y);
}
