//! UI_ScrollBar (Interface.cpp:1156), the horizontal form with buttons:
//! a thumb (interface.dat A126ScrollN / A127ScrollA) dragged along `length`
//! pixels, and -/+ buttons that step the value by one.  The value lives in
//! the scroll bar instead of behind AttachValue's pointer.
//!
//! Check returns the original flag: 1 visible, 2 hot, 3 dragging, 4 stepped
//! by a button.  Flag 4 lasts one frame here; the original kept it while the
//! pointer stayed over the bar, which only repeated the caller's redraw.
const std = @import("std");
const rl = @import("raylib");
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Button = @import("button.zig").Button;

const gui_scroll_n = 126;
const gui_scroll_a = 127;
/// buttons.dat: - N/A/P/inactive, + N/A/P/inactive.
const butt_sub = 293;
const butt_add = 289;

pub const State = enum(u8) { visible = 1, hot = 2, dragging = 3, stepped = 4 };

pub const ScrollBar = struct {
    x: f32,
    y: f32,
    length: i32,
    width: f32,
    value: i32,
    min: i32,
    max: i32,
    state: State = .visible,
    sub: Button,
    add: Button,

    pub fn init(buttons: *gfx.Sheet, x: f32, y: f32, length: i32, width: f32, value: i32, min: i32, max: i32) !ScrollBar {
        var sub = try Button.init(buttons, x - 15, y - 3, butt_sub, butt_sub + 1, butt_sub + 2);
        errdefer sub.deinit();
        sub.inactive = butt_sub + 3;
        var add = try Button.init(buttons, x + @as(f32, @floatFromInt(length)) + 14, y - 3, butt_add, butt_add + 1, butt_add + 2);
        add.inactive = butt_add + 3;
        var s: ScrollBar = .{ .x = x, .y = y, .length = length, .width = width, .value = value, .min = min, .max = max, .sub = sub, .add = add };
        s.clamp();
        return s;
    }

    pub fn deinit(self: *ScrollBar) void {
        self.sub.deinit();
        self.add.deinit();
    }

    /// Show(): keeps the value in range.
    pub fn setValue(self: *ScrollBar, v: i32) void {
        self.value = v;
        self.clamp();
    }

    fn clamp(self: *ScrollBar) void {
        self.value = std.math.clamp(self.value, self.min, @max(self.min, self.max));
    }

    pub fn check(self: *ScrollBar, f: *const input.Frame) State {
        const p = f.pointer;
        if (self.state == .stepped) self.state = .hot;
        const len: f32 = @floatFromInt(self.length);
        const over = p.hover and p.pos.x >= self.x and p.pos.y >= self.y - 2 and
            p.pos.x < self.x + len and p.pos.y < self.y + self.width;
        if (over or (self.state == .dragging and p.primary.down)) {
            if (self.state == .visible) self.state = .hot;
            if (p.primary.down) {
                self.state = .dragging;
                if (self.max > self.min) {
                    const dx: i32 = @intFromFloat(@floor(p.pos.x - self.x));
                    self.value = self.min + @divTrunc((self.max - self.min) * dx, self.length);
                    self.clamp();
                }
            } else if (self.state == .dragging) self.state = .hot;
        } else self.state = .visible;

        self.sub.on = self.value > self.min;
        self.add.on = self.value < self.max;
        if (self.state != .dragging) {
            if (self.sub.check(f)) {
                self.value -= 1;
                self.clamp();
                self.state = .stepped;
            }
            if (self.add.check(f)) {
                self.value += 1;
                self.clamp();
                self.state = .stepped;
            }
        }
        return self.state;
    }

    pub fn thumbX(self: *const ScrollBar) f32 {
        if (self.max <= self.min) return self.x + @as(f32, @floatFromInt(@divTrunc(self.length, 2)));
        return self.x + @as(f32, @floatFromInt(@divTrunc((self.value - self.min) * self.length, self.max - self.min)));
    }

    pub fn draw(self: *const ScrollBar, iface: *gfx.Sheet, buttons: *gfx.Sheet, tick: u16) void {
        const pic: usize = if (self.state == .visible) gui_scroll_n else gui_scroll_a;
        gfx.drawTex(iface.texIndex(pic), self.thumbX() - 2, self.y - 2);
        self.sub.draw(buttons, tick);
        self.add.draw(buttons, tick);
    }
};

test "thumb position and clamping" {
    var s: ScrollBar = .{ .x = 100, .y = 0, .length = 145, .width = 7, .value = 300, .min = 0, .max = 255, .sub = undefined, .add = undefined };
    s.clamp();
    try std.testing.expectEqual(@as(i32, 255), s.value);
    try std.testing.expectEqual(@as(f32, 245), s.thumbX());
    s.setValue(128);
    try std.testing.expectEqual(@as(f32, 100 + 72), s.thumbX());
}
