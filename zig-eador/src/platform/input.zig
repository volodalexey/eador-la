//! Per-frame input for scenes.  Scenes never read the mouse or touch screen
//! directly: they see one `Pointer` in frame coordinates (1024×768).
//!
//! Desktop: the mouse; left button = primary, right button = secondary.
//! Touch (Android later): touch point 0 is the pointer; a hold of
//! `hold_seconds` without moving gives `secondary` (the original's right
//! click: info and hints), and while the finger is down `hover` is true.
const std = @import("std");
const rl = @import("raylib");
const canvas = @import("canvas.zig");
const mouse_glfw = @import("mouse_glfw.zig");

pub const hold_seconds = 0.5;
/// Movement (frame pixels) that cancels a hold.
pub const hold_slop = 8.0;

pub const Button = struct {
    down: bool = false,
    pressed: bool = false,
    released: bool = false,
};

pub const Pointer = struct {
    pos: rl.Vector2 = .{ .x = 0, .y = 0 },
    /// The pointer is over the frame and hover effects apply (always for a
    /// mouse, only while touching for a touch screen).
    hover: bool = false,
    primary: Button = .{},
    secondary: Button = .{},
    wheel: f32 = 0,

    pub fn in(self: Pointer, x: f32, y: f32, w: f32, h: f32) bool {
        return self.hover and self.pos.x >= x and self.pos.x < x + w and self.pos.y >= y and self.pos.y < y + h;
    }

    /// A primary click (release) inside the rectangle.
    pub fn clicked(self: Pointer, x: f32, y: f32, w: f32, h: f32) bool {
        return self.primary.released and self.in(x, y, w, h);
    }
};

pub const Source = enum { mouse, touch };

pub const Frame = struct {
    pointer: Pointer = .{},
    /// Keys pressed this frame (with repeats), raylib order.
    keys: [16]rl.KeyboardKey = undefined,
    n_keys: usize = 0,
    /// Typed characters this frame (Unicode).
    chars: [16]u21 = undefined,
    n_chars: usize = 0,
    /// key[KEY_LCONTROL] || key[KEY_RCONTROL].
    ctrl: bool = false,

    pub fn keyList(self: *const Frame) []const rl.KeyboardKey {
        return self.keys[0..self.n_keys];
    }

    pub fn charList(self: *const Frame) []const u21 {
        return self.chars[0..self.n_chars];
    }

    pub fn key(self: *const Frame, k: rl.KeyboardKey) bool {
        return std.mem.indexOfScalar(rl.KeyboardKey, self.keyList(), k) != null;
    }

    /// The frame without keyboard input: while a text field has the
    /// keyboard (bInput) button hotkeys do not fire.
    pub fn withoutKeys(self: *const Frame) Frame {
        var g = self.*;
        g.n_keys = 0;
        g.n_chars = 0;
        return g;
    }

    /// Allegro's keypressed(): any key or click this frame.
    pub fn any(self: *const Frame) bool {
        return self.n_keys > 0 or self.pointer.primary.released or self.pointer.secondary.released;
    }
};

/// Touch hold tracker, independent of raylib for testing.
pub const Hold = struct {
    start: ?rl.Vector2 = null,
    t: f32 = 0,
    fired: bool = false,

    /// Returns the secondary button state for this frame.
    pub fn step(self: *Hold, touching: bool, pos: rl.Vector2, dt: f32) Button {
        var b: Button = .{};
        if (!touching) {
            if (self.fired) b.released = true;
            self.* = .{};
            return b;
        }
        const s = self.start orelse {
            self.start = pos;
            return b;
        };
        if (self.fired) {
            b.down = true;
            return b;
        }
        if (@abs(pos.x - s.x) > hold_slop or @abs(pos.y - s.y) > hold_slop) {
            self.start = pos;
            self.t = 0;
            return b;
        }
        self.t += dt;
        if (self.t >= hold_seconds) {
            self.fired = true;
            b.pressed = true;
            b.down = true;
        }
        return b;
    }
};

pub const Input = struct {
    source: Source = .mouse,
    hold: Hold = .{},
    touch_was_down: bool = false,

    /// After the window is created.
    pub fn init(source: Source) Input {
        if (source == .mouse) mouse_glfw.install();
        return .{ .source = source };
    }

    /// Once per frame before scenes update.
    pub fn poll(self: *Input, dt: f32) Frame {
        var f: Frame = .{};
        const vp = canvas.Canvas.viewport();
        switch (self.source) {
            .mouse => {
                mouse_glfw.update();
                const p = canvas.toFrame(vp, rl.getMousePosition());
                f.pointer.pos = p;
                f.pointer.hover = p.x >= 0 and p.y >= 0 and p.x < canvas.width and p.y < canvas.height;
                f.pointer.primary = mouseButton(.left);
                f.pointer.secondary = mouseButton(.right);
                f.pointer.wheel = rl.getMouseWheelMove();
            },
            .touch => {
                const touching = rl.getTouchPointCount() > 0;
                const p = if (touching) canvas.toFrame(vp, rl.getTouchPosition(0)) else f.pointer.pos;
                f.pointer.pos = p;
                f.pointer.hover = touching;
                f.pointer.secondary = self.hold.step(touching, p, dt);
                // A hold that became a secondary click does not also click.
                f.pointer.primary = .{
                    .down = touching and !self.hold.fired,
                    .pressed = touching and !self.touch_was_down,
                    .released = !touching and self.touch_was_down and !f.pointer.secondary.released,
                };
                self.touch_was_down = touching;
            },
        }
        f.ctrl = rl.isKeyDown(.left_control) or rl.isKeyDown(.right_control);
        while (f.n_keys < f.keys.len) {
            const k = rl.getKeyPressed();
            if (k == .null) break;
            f.keys[f.n_keys] = k;
            f.n_keys += 1;
        }
        while (f.n_chars < f.chars.len) {
            const c = rl.getCharPressed();
            if (c <= 0) break;
            f.chars[f.n_chars] = std.math.cast(u21, c) orelse continue;
            f.n_chars += 1;
        }
        return f;
    }
};

fn mouseButton(b: rl.MouseButton) Button {
    return .{
        .down = rl.isMouseButtonDown(b),
        .pressed = rl.isMouseButtonPressed(b),
        .released = rl.isMouseButtonReleased(b),
    };
}

test "touch hold becomes a secondary click" {
    var h: Hold = .{};
    const p: rl.Vector2 = .{ .x = 100, .y = 100 };
    _ = h.step(true, p, 0);
    var b = h.step(true, p, 0.3);
    try std.testing.expect(!b.pressed);
    b = h.step(true, .{ .x = 103, .y = 101 }, 0.3);
    try std.testing.expect(b.pressed and b.down);
    b = h.step(true, p, 0.1);
    try std.testing.expect(b.down and !b.pressed);
    b = h.step(false, p, 0.1);
    try std.testing.expect(b.released);
}

test "moving the finger cancels the hold" {
    var h: Hold = .{};
    _ = h.step(true, .{ .x = 0, .y = 0 }, 0);
    _ = h.step(true, .{ .x = 0, .y = 0 }, 0.4);
    const b = h.step(true, .{ .x = 50, .y = 0 }, 0.4);
    try std.testing.expect(!b.pressed);
    try std.testing.expect(!h.fired);
}
