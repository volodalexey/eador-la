//! Touchpad taps deliver the press and the release of a button within one
//! frame.  raylib compares the button state once per frame, so it saw "up"
//! before and after and the click was lost.  The GLFW mouse-button callback
//! is wrapped: events are queued and handed to raylib at most one per button
//! per frame, so a tap becomes "pressed" in one frame and "released" in the
//! next.  Button handling elsewhere (release over the button) is unchanged.
const std = @import("std");

const GLFWwindow = opaque {};
const ButtonFn = *const fn (?*GLFWwindow, c_int, c_int, c_int) callconv(.c) void;
extern fn glfwGetCurrentContext() ?*GLFWwindow;
extern fn glfwSetMouseButtonCallback(?*GLFWwindow, ?ButtonFn) ?ButtonFn;

pub const max_buttons = 8;

pub const Event = struct { action: c_int, mods: c_int = 0 };

/// FIFO of one button's events.
pub const Queue = struct {
    buf: [32]Event = undefined,
    head: usize = 0,
    len: usize = 0,

    pub fn push(q: *Queue, e: Event) void {
        if (q.len == q.buf.len) return;
        q.buf[(q.head + q.len) % q.buf.len] = e;
        q.len += 1;
    }

    pub fn pop(q: *Queue) ?Event {
        if (q.len == 0) return null;
        const e = q.buf[q.head];
        q.head = (q.head + 1) % q.buf.len;
        q.len -= 1;
        return e;
    }
};

var window: ?*GLFWwindow = null;
var raylib_cb: ?ButtonFn = null;
var queues: [max_buttons]Queue = @splat(.{});

fn onButton(w: ?*GLFWwindow, button: c_int, action: c_int, mods: c_int) callconv(.c) void {
    if (button < 0 or button >= max_buttons) {
        if (raylib_cb) |f| f(w, button, action, mods);
        return;
    }
    queues[@intCast(button)].push(.{ .action = action, .mods = mods });
}

/// After InitWindow: raylib's context is current, its callback is kept.
pub fn install() void {
    window = glfwGetCurrentContext() orelse return;
    raylib_cb = glfwSetMouseButtonCallback(window, &onButton);
}

/// Once per frame, after the input poll of the previous frame (EndDrawing)
/// and before the frame reads the mouse.
pub fn update() void {
    const f = raylib_cb orelse return;
    for (&queues, 0..) |*q, b| if (q.pop()) |e| f(window, @intCast(b), e.action, e.mods);
}

test "a tap within one poll is seen as press, then release" {
    // raylib's per-frame state: previous = current at the poll, then the
    // events of the poll set current.
    const State = struct {
        prev: c_int = 0,
        cur: c_int = 0,
        fn pressed(s: @This()) bool {
            return s.prev == 0 and s.cur == 1;
        }
        fn released(s: @This()) bool {
            return s.prev == 1 and s.cur == 0;
        }
    };
    var s: State = .{};
    var q: Queue = .{};

    // Poll 1: press and release of a tap arrive together.
    s.prev = s.cur;
    q.push(.{ .action = 1 });
    q.push(.{ .action = 0 });
    if (q.pop()) |e| s.cur = e.action;
    try std.testing.expect(s.pressed());

    // Poll 2: nothing new; the queued release is handed over.
    s.prev = s.cur;
    if (q.pop()) |e| s.cur = e.action;
    try std.testing.expect(s.released());

    // Poll 3: idle.
    s.prev = s.cur;
    try std.testing.expect(q.pop() == null);
    try std.testing.expect(!s.pressed() and !s.released());
}

test "two quick taps give two clicks" {
    var q: Queue = .{};
    for (0..2) |_| {
        q.push(.{ .action = 1 });
        q.push(.{ .action = 0 });
    }
    var prev: c_int = 0;
    var clicks: usize = 0;
    while (q.pop()) |e| {
        if (prev == 1 and e.action == 0) clicks += 1;
        prev = e.action;
    }
    try std.testing.expectEqual(@as(usize, 2), clicks);
}
