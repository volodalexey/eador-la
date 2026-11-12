//! Coats of arms and banners in a profile's colours: DrawHerb
//! (Player.cpp:1623) and DrawTownFlag (Windower.cpp:4860).
//!
//! draw_lit_sprite with set_trans_blender(r, g, b, 0) at 255 paints the
//! opaque pixels in that colour: a tinted white mask (Sheet.maskIndex).
//! The final draw_trans_sprite at 128 lays the shaded picture over it at
//! half opacity.
const std = @import("std");
const rl = @import("raylib");
const gfx = @import("../platform/gfx.zig");
const GameFs = @import("../platform/fs.zig").GameFs;
const profile = @import("../data/profile.zig");

/// flag.dat: bmpHerbMain, bmpHerbBorder, bmpHerbRandom, bmpTownFlag[0][1].
const flag_herb_main = 0;
const flag_herb_border = 1;
pub const flag_herb_random = 5;
const flag_town = 7;
/// bmpFlagMain, bmpFlagCol1, bmpFlagCol2 (39×70).
const flag_main = 2;
const flag_col1 = 3;
const flag_col2 = 4;
pub const flag_w = 39;
pub const flag_h = 70;
/// bmpHerbMain (31×36).
pub const herb_w = 31;
pub const herb_h = 36;
const half: rl.Color = .{ .r = 255, .g = 255, .b = 255, .a = 128 };

pub fn color(i: i32) rl.Color {
    const c = profile.herb_colors[@intCast(std.math.clamp(i, 0, profile.max_colors - 1))];
    return .{ .r = c[0], .g = c[1], .b = c[2], .a = 255 };
}

fn signIndex(sign: i32) usize {
    return @intCast(std.math.clamp(sign, 0, profile.max_signs - 1));
}

fn tinted(t: ?rl.Texture2D, x: f32, y: f32, c: rl.Color) void {
    if (t) |tx| rl.drawTextureV(tx, .{ .x = x, .y = y }, c);
}

pub const Heraldry = struct {
    /// flag.dat, herb.dat (bmpHerbSign), herbS.dat (bmpHerbSignS).
    flag: gfx.Sheet,
    herb: gfx.Sheet,
    herbs: gfx.Sheet,

    pub fn load(fs: *GameFs) !Heraldry {
        var flag = try gfx.Sheet.load(fs, "flag");
        errdefer flag.deinit();
        var herb = try gfx.Sheet.load(fs, "herb");
        errdefer herb.deinit();
        return .{ .flag = flag, .herb = herb, .herbs = try gfx.Sheet.load(fs, "herbs") };
    }

    pub fn deinit(self: *Heraldry) void {
        self.flag.deinit();
        self.herb.deinit();
        self.herbs.deinit();
    }

    pub fn drawHerb(self: *Heraldry, c1: i32, c2: i32, sign: i32, x: f32, y: f32) void {
        gfx.drawTex(self.flag.texIndex(flag_herb_border), x, y);
        tinted(self.flag.maskIndex(flag_herb_main), x, y, color(c1));
        tinted(self.herb.maskIndex(signIndex(sign)), x, y, color(c2));
        tinted(self.flag.texIndex(flag_herb_main), x, y, half);
    }

    /// Cut at `clip` (the window the banner was drawn on).
    pub fn drawTownFlag(self: *Heraldry, c1: i32, c2: i32, sign: i32, x: f32, y: f32, clip: rl.Rectangle) void {
        gfx.drawClipped(self.flag.maskIndex(flag_town), x, y, color(c1), clip);
        gfx.drawClipped(self.herbs.maskIndex(signIndex(sign)), x + 50, y + 15, color(c2), clip);
        gfx.drawClipped(self.flag.texIndex(flag_town), x, y, half, clip);
    }

    /// dcPlayer::MakeFlags, bmpFlag[c][1 + k]: the town banner of level
    /// group k (0 always, 1 above level 5, 2 above level 8).  The first
    /// carries the sign in herbS, the others the second colour.
    pub fn drawCityFlag(self: *Heraldry, k: usize, c1: i32, c2: i32, sign: i32, x: f32, y: f32) void {
        const base = flag_town - 1 + 3 * k;
        gfx.drawTex(self.flag.texIndex(base), x, y);
        tinted(self.flag.maskIndex(base + 1), x, y, color(c1));
        if (k == 0) {
            tinted(self.herbs.maskIndex(signIndex(sign)), x + 50, y + 15, color(c2));
        } else tinted(self.flag.maskIndex(base + 2), x, y, color(c2));
        tinted(self.flag.texIndex(base + 1), x, y, half);
    }

    /// DrawFlag (Player.cpp:1634), bmpFlag[c][0]: the banner over a hero
    /// on the map.
    pub fn drawFlag(self: *Heraldry, c1: i32, c2: i32, x: f32, y: f32) void {
        gfx.drawTex(self.flag.texIndex(flag_main), x, y);
        tinted(self.flag.maskIndex(flag_col1), x, y, color(c1));
        tinted(self.flag.maskIndex(flag_col2), x, y, color(c2));
        tinted(self.flag.texIndex(flag_main), x, y, half);
    }

    /// The arms of a profile.
    pub fn drawHuman(self: *Heraldry, h: *const profile.Human, x: f32, y: f32) void {
        self.drawHerb(h.get(.color1), h.get(.color2), h.get(.sign), x, y);
    }
};

test "colour clamping" {
    try std.testing.expectEqual(@as(u8, 239), color(-3).r);
    try std.testing.expectEqual(@as(u8, 97), color(99).r);
}
