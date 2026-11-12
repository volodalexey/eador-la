//! UI_Mask (Interface.cpp:83): a button shaped by a mask picture or a plain
//! rectangle.  The pointer is over it where the mask is opaque (magenta in
//! the original, alpha 0 after import).  A click is the release of a button
//! that was held over the mask.
const std = @import("std");
const rl = @import("raylib");
const input = @import("../platform/input.zig");

pub const Result = enum { none, click, right_click };

pub const Mask = struct {
    x: f32,
    y: f32,
    /// Null: the whole `w`×`h` rectangle is the mask.
    shape: ?*const rl.Image = null,
    w: f32 = 0,
    h: f32 = 0,
    /// flag of UI_Mask: 1 visible, 2 hot, 3 pressed, 4 right pressed.
    state: State = .visible,

    pub const State = enum { visible, hot, pressed, right_pressed };

    pub fn rect(x: f32, y: f32, w: f32, h: f32) Mask {
        return .{ .x = x, .y = y, .w = w, .h = h };
    }

    pub fn over(self: *const Mask, p: input.Pointer) bool {
        if (!p.hover) return false;
        const lx = @floor(p.pos.x - self.x);
        const ly = @floor(p.pos.y - self.y);
        if (lx < 0 or ly < 0) return false;
        const img = self.shape orelse return lx < self.w and ly < self.h;
        if (lx >= @as(f32, @floatFromInt(img.width)) or ly >= @as(f32, @floatFromInt(img.height))) return false;
        return rl.getImageColor(img.*, @intFromFloat(lx), @intFromFloat(ly)).a > 0;
    }

    pub fn check(self: *Mask, p: input.Pointer) Result {
        const old = self.state;
        self.state = if (!self.over(p)) .visible else if (p.primary.down) .pressed else if (p.secondary.down) .right_pressed else .hot;
        if (self.state == .hot and old == .pressed) return .click;
        if (self.state == .hot and old == .right_pressed) return .right_click;
        return .none;
    }

    /// The active picture is shown (G_Obj on its layer).
    pub fn hot(self: *const Mask) bool {
        return self.state != .visible;
    }
};

test "press and release over the mask" {
    var px = [_]u8{ 255, 255, 255, 255, 0, 0, 0, 0 };
    const img: rl.Image = .{ .data = &px, .width = 2, .height = 1, .mipmaps = 1, .format = .uncompressed_r8g8b8a8 };
    var m: Mask = .{ .x = 10, .y = 10, .shape = &img };
    var p: input.Pointer = .{ .pos = .{ .x = 10.5, .y = 10.5 }, .hover = true };
    try std.testing.expectEqual(Result.none, m.check(p));
    try std.testing.expect(m.hot());
    p.primary.down = true;
    try std.testing.expectEqual(Result.none, m.check(p));
    p.primary.down = false;
    try std.testing.expectEqual(Result.click, m.check(p));
    // The transparent pixel is outside.
    p.pos.x = 11.5;
    _ = m.check(p);
    try std.testing.expect(!m.hot());
}

test "rectangle mask" {
    var m = Mask.rect(0, 0, 10, 5);
    var p: input.Pointer = .{ .pos = .{ .x = 9, .y = 4 }, .hover = true, .primary = .{ .down = true } };
    _ = m.check(p);
    p.primary.down = false;
    try std.testing.expectEqual(Result.click, m.check(p));
    p.pos.x = 10;
    _ = m.check(p);
    try std.testing.expect(!m.hot());
}
