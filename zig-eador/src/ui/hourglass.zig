//! RunHourGlass and AnimHourglass (Audio.cpp:349, 394): the panel with the
//! hourglass shown while a game is created, loaded or saved.
//!
//! interface.dat 79NewTurn centred on the screen, the phrase in black
//! font01 centred at +234, +46, anim.dat 00HGlass..12HGlass at +48, +43
//! (the next frame every 200 ms timer tick, from 0 when the panel opens),
//! and the bar: one A115ProgressFill column at +122 + i, +67 for every
//! i ≤ NewProgress / 100 below 220, under A114ProgressMark at +121, +70.
//! The original draws columns only once NewProgress / 100 grows past
//! OldProgress / 100, so nothing shows below 100.  A new phrase keeps the
//! bar and the hourglass frame (bHourGlass is still set).
const std = @import("std");
const rl = @import("raylib");
const gfx = @import("../platform/gfx.zig");

/// interface.dat: bmpGUI_NewTurn, bmpGUI_ProgressMark, bmpGUI_ProgressFill.
const gui_new_turn = 79;
const gui_progress_mark = 114;
const gui_progress_fill = 115;
/// bmpHourGlass[0..12] (anim.dat 0..12).
const frames = 13;
const tick_s = 0.2;
const bar_columns = 220;

/// NewProgress at a full bar.
pub const full = 22000;

pub const Hourglass = struct {
    /// pFirstPhrase index of the caption.
    phrase: usize,
    /// NewProgress.
    progress: i32 = 0,
    /// iHourGlass.
    frame: u8 = 0,
    acc: f32 = 0,

    pub fn update(self: *Hourglass, dt: f32) void {
        self.acc += dt;
        while (self.acc >= tick_s) {
            self.acc -= tick_s;
            self.frame = (self.frame + 1) % frames;
        }
    }

    /// The columns of the bar.
    pub fn columns(self: *const Hourglass) usize {
        if (self.progress < 100) return 0;
        return @min(@as(usize, @intCast(@divTrunc(self.progress, 100))) + 1, bar_columns);
    }

    pub fn draw(self: *const Hourglass, iface: *gfx.Sheet, anim: *gfx.Sheet, font: *const gfx.Font, caption: []const u8) void {
        const panel = iface.texIndex(gui_new_turn) orelse return;
        const bx: f32 = @floatFromInt(512 - @divTrunc(panel.width, 2));
        const by: f32 = @floatFromInt(384 - @divTrunc(panel.height, 2));
        gfx.drawTex(panel, bx, by);
        font.drawCentered(caption, bx + 234, by + 46, .{ .r = 0, .g = 0, .b = 0, .a = 255 });
        gfx.drawTex(anim.texIndex(self.frame), bx + 48, by + 43);
        const n = self.columns();
        if (n == 0) return;
        if (iface.texIndex(gui_progress_fill)) |fill| {
            const w: f32 = @floatFromInt(fill.width);
            const h: f32 = @floatFromInt(fill.height);
            rl.drawTexturePro(fill, .{ .x = 0, .y = 0, .width = w, .height = h }, .{ .x = bx + 122, .y = by + 67, .width = w * @as(f32, @floatFromInt(n)), .height = h }, .{ .x = 0, .y = 0 }, 0, .white);
        }
        gfx.drawTex(iface.texIndex(gui_progress_mark), bx + 121, by + 70);
    }
};

test "hourglass bar and frames" {
    var g: Hourglass = .{ .phrase = 153 };
    try std.testing.expectEqual(@as(usize, 0), g.columns());
    g.progress = 99;
    try std.testing.expectEqual(@as(usize, 0), g.columns());
    g.progress = 150;
    try std.testing.expectEqual(@as(usize, 2), g.columns());
    g.progress = 21900;
    try std.testing.expectEqual(@as(usize, 220), g.columns());
    g.progress = full;
    try std.testing.expectEqual(@as(usize, 220), g.columns());
    g.update(0.19);
    try std.testing.expectEqual(@as(u8, 0), g.frame);
    g.update(0.02);
    try std.testing.expectEqual(@as(u8, 1), g.frame);
    g.update(0.2 * 12);
    try std.testing.expectEqual(@as(u8, 0), g.frame);
}
