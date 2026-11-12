//! UI_Button1 (Interface.cpp:429): a picture button with normal, active
//! (hover) and pressed images.  The normal image is also the mask.  A click
//! is the release over the button; the hotkey clicks on its own.  The click
//! plays wavSound (0: the click, spell_sound.dat 0 at half volume).
//!
//! The original fires a hotkey when the key is released; here it fires when
//! the key is pressed (Frame only has key-press events).
const std = @import("std");
const rl = @import("raylib");
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const Mask = @import("mask.zig").Mask;

/// Init arguments of a UI_Button1, relative to its window.
pub const Spec = struct { x: f32, y: f32, normal: usize, active: usize, pressed: usize, inactive: usize, hint: u32 = 0 };

/// The arrow buttons of the windows (buttons.dat 236.. and 233..).
pub fn left(x: f32, y: f32) Spec {
    return .{ .x = x, .y = y, .normal = 236, .active = 237, .pressed = 238, .inactive = 288 };
}

pub fn right(x: f32, y: f32) Spec {
    return .{ .x = x, .y = y, .normal = 233, .active = 234, .pressed = 235, .inactive = 287 };
}

/// The buttons of a window at (x, y), one per field of `E`.
pub fn initAll(comptime E: type, sheet: *gfx.Sheet, x: f32, y: f32, specs: std.enums.EnumArray(E, Spec), out: *std.enums.EnumArray(E, Button)) !void {
    var done: usize = 0;
    errdefer for (out.values[0..done]) |*b| b.deinit();
    for (&out.values, specs.values) |*b, s| {
        b.* = try Button.init(sheet, x + s.x, y + s.y, s.normal, s.active, s.pressed);
        b.inactive = s.inactive;
        b.hint = s.hint;
        done += 1;
    }
}

pub const Button = struct {
    mask: Mask,
    shape: rl.Image,
    /// Objects of the sheet passed to init.
    normal: usize,
    active: usize,
    pressed: usize,
    hotkey: ?rl.KeyboardKey = null,
    /// wavSound: > 0 unit_sound.dat n, <= 0 spell_sound.dat |n|.
    sound: i32 = 0,
    /// Activate(-1): hidden.
    enabled: bool = true,
    /// Activate(0): the inactive picture (bmpInactive), no clicks.
    on: bool = true,
    inactive: ?usize = null,
    /// iHint: phrases.var string shown while the pointer is over it.
    hint: u32 = 0,
    /// bBlink: alternate normal/active while idle (Interface.cpp:506).
    blink: bool = false,

    pub fn init(sheet: *gfx.Sheet, x: f32, y: f32, normal: usize, active: usize, pressed: usize) !Button {
        var b: Button = .{
            .mask = .{ .x = x, .y = y },
            .shape = sheet.imageIndex(normal) orelse return error.MissingButton,
            .normal = normal,
            .active = active,
            .pressed = pressed,
        };
        b.mask.shape = null;
        return b;
    }

    pub fn deinit(self: *Button) void {
        rl.unloadImage(self.shape);
    }

    /// Returns true on a click.  `self` must not move after the first call
    /// (the mask points to the shape).
    pub fn check(self: *Button, f: *const input.Frame) bool {
        if (!self.enabled or !self.on) {
            self.mask.state = .visible;
            return false;
        }
        self.mask.shape = &self.shape;
        const hit = (self.hotkey != null and f.key(self.hotkey.?)) or self.mask.check(f.pointer) == .click;
        if (hit) sound.button(self.sound);
        return hit;
    }

    fn imageIndex(self: *const Button, tick: u16) ?usize {
        if (!self.enabled) return null;
        return if (!self.on) self.inactive orelse self.normal else switch (self.mask.state) {
            .visible => if (self.blink and tick % 4 > 1) self.active else self.normal,
            .hot, .right_pressed => self.active,
            .pressed => self.pressed,
        };
    }

    pub fn draw(self: *const Button, sheet: *gfx.Sheet, tick: u16) void {
        const i = self.imageIndex(tick) orelse return;
        gfx.drawTex(sheet.texIndex(i), self.mask.x, self.mask.y);
    }

    /// The pointer is over an active button (for its hint).
    pub fn hot(self: *const Button) bool {
        return self.enabled and self.on and self.mask.hot();
    }
};

test "button blink uses the shared tick only while idle and active" {
    var b: Button = .{
        .mask = .{ .x = 0, .y = 0 },
        .shape = undefined, // Image selection needs neither raylib nor assets.
        .normal = 1,
        .active = 2,
        .pressed = 3,
        .inactive = 4,
    };
    for (0..8) |tick| try std.testing.expectEqual(@as(?usize, 1), b.imageIndex(@intCast(tick)));
    b.blink = true;
    for (0..8) |tick| try std.testing.expectEqual(@as(?usize, if (tick % 4 > 1) 2 else 1), b.imageIndex(@intCast(tick)));

    b.mask.state = .hot;
    try std.testing.expectEqual(@as(?usize, 2), b.imageIndex(0));
    b.mask.state = .right_pressed;
    try std.testing.expectEqual(@as(?usize, 2), b.imageIndex(0));
    b.mask.state = .pressed;
    try std.testing.expectEqual(@as(?usize, 3), b.imageIndex(2));

    b.mask.state = .visible;
    b.on = false;
    try std.testing.expectEqual(@as(?usize, 4), b.imageIndex(2));
    b.inactive = null;
    try std.testing.expectEqual(@as(?usize, 1), b.imageIndex(2));
    b.enabled = false;
    try std.testing.expectEqual(@as(?usize, null), b.imageIndex(2));
}
