//! UI_TextBox (Interface.cpp:728, 778): word-wrapped text in a box.
//!
//! Wrapping follows Activate(): words end at ' ', '\n' or the end of the
//! text; a word that does not fit after the current line starts a new one
//! (also when the line is empty, which leaves an empty line before a word
//! wider than the box).  '\n' and the end of the text close the line.
//! Lines are `lineHeight + 2` apart; the box height is rounded to 14 px.
const std = @import("std");
const rl = @import("raylib");
const gfx = @import("../platform/gfx.zig");

pub const max_lines = 64;

/// Splits `text` into lines no wider than `w`.  Slices point into `text`
/// and may keep a trailing space, as sLine does.
pub fn wrap(font: *const gfx.Font, text: []const u8, w: f32, out: *[max_lines][]const u8) []const []const u8 {
    var n: usize = 0;
    var line_start: usize = 0;
    var word_start: usize = 0;
    var i: usize = 0;
    while (true) : (i += 1) {
        const at_end = i >= text.len;
        const c: u8 = if (at_end) 0 else text[i];
        if (c != ' ' and c != '\n' and !at_end) continue;
        if (font.measure(text[line_start..word_start]) + font.measure(text[word_start..i]) > w) {
            if (n < out.len) out[n] = text[line_start..word_start];
            n += 1;
            line_start = word_start;
        }
        if (c == ' ') {
            word_start = i + 1;
        } else {
            if (n < out.len) out[n] = text[line_start..i];
            n += 1;
            line_start = i + 1;
            word_start = i + 1;
        }
        if (at_end) break;
    }
    return out[0..@min(n, out.len)];
}

/// Box height after Init's rounding to whole 14-px steps.
pub fn boxHeight(h: i32) i32 {
    const dh = @mod(h, 14);
    return h - dh + (if (dh > 7) @as(i32, 14) else 0);
}

pub const Box = struct {
    x: f32,
    y: f32,
    w: f32,
    h: i32,
    font: *const gfx.Font,
    color: rl.Color,
    centre: bool = false,

    pub fn init(x: f32, y: f32, w: f32, h: i32, font: *const gfx.Font, color: rl.Color) Box {
        return .{ .x = x, .y = y, .w = w, .h = boxHeight(h), .font = font, .color = color };
    }

    pub fn lineStep(self: *const Box) f32 {
        return self.font.lineHeight() + 2;
    }

    /// Draws the lines that start inside the box; returns the line count.
    pub fn draw(self: *const Box, text: []const u8) usize {
        var buf: [max_lines][]const u8 = undefined;
        const lines = wrap(self.font, text, self.w, &buf);
        const h: f32 = @floatFromInt(self.h);
        for (lines, 0..) |line, k| {
            const ly = @as(f32, @floatFromInt(k)) * self.lineStep();
            if (ly + self.font.lineHeight() / 2 > h) break;
            if (self.centre)
                self.font.drawCentered(line, self.x + self.w / 2 + 3, self.y + ly, self.color)
            else
                self.font.draw(line, self.x, self.y + ly, self.color);
        }
        return lines.len;
    }
};

test "box height rounding" {
    try std.testing.expectEqual(@as(i32, 98), boxHeight(96));
    try std.testing.expectEqual(@as(i32, 252), boxHeight(254));
    try std.testing.expectEqual(@as(i32, 112), boxHeight(118));
}
