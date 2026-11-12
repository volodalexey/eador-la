//! UI_Input (Interface.cpp:981): a one-line text field of up to `max_len`
//! characters.  Characters are drawn 9 px apart, with the feather cursor
//! (interface.dat 82Feather) after the last one while the field is active;
//! a centred field shifts by (max_len - length) * 9 / 2.  Only characters
//! cp1251 can store are accepted, as the profile keeps cp1251.
const std = @import("std");
const rl = @import("raylib");
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const cp1251 = @import("../formats/cp1251.zig");

const gui_cursor = 82;
const step = 9;

pub const Result = enum { idle, active, done };

pub const Input = struct {
    x: f32,
    y: f32,
    max_len: usize,
    color: rl.Color,
    centre: bool = false,
    active: bool = false,
    buf: [96]u8 = undefined,
    len: usize = 0,
    chars: usize = 0,

    pub fn text(self: *const Input) []const u8 {
        return self.buf[0..self.len];
    }

    pub fn clear(self: *Input) void {
        self.len = 0;
        self.chars = 0;
    }

    pub fn set(self: *Input, utf8: []const u8) void {
        self.clear();
        var at: usize = 0;
        while (at < utf8.len) self.put(cp1251.nextCodepoint(utf8, &at));
    }

    fn put(self: *Input, cp: u21) void {
        if (self.chars >= self.max_len or cp < 32 or cp1251.fromCodepoint(cp) == null) return;
        var tmp: [4]u8 = undefined;
        const n = std.unicode.utf8Encode(cp, &tmp) catch return;
        if (self.len + n > self.buf.len) return;
        @memcpy(self.buf[self.len..][0..n], tmp[0..n]);
        self.len += n;
        self.chars += 1;
    }

    fn backspace(self: *Input) void {
        if (self.len == 0) return;
        var n = self.len - 1;
        while (n > 0 and (self.buf[n] & 0xC0) == 0x80) n -= 1;
        self.len = n;
        self.chars -= 1;
    }

    /// Check(true): typing while active; Enter or Esc deactivates.
    pub fn check(self: *Input, f: *const input.Frame) Result {
        if (!self.active) return .idle;
        for (f.charList()) |cp| self.put(cp);
        for (f.keyList()) |k| switch (k) {
            .backspace => self.backspace(),
            .enter, .kp_enter, .escape => {
                self.active = false;
                return .done;
            },
            else => {},
        };
        return .active;
    }

    pub fn draw(self: *const Input, font: *const gfx.Font, iface: *gfx.Sheet) void {
        const shift: f32 = if (self.centre) @floatFromInt(@divTrunc((@as(i32, @intCast(self.max_len)) - @as(i32, @intCast(self.chars))) * step, 2)) else 0;
        const ox = self.x + shift;
        var at: usize = 0;
        var i: f32 = 0;
        const s = self.text();
        while (at < s.len) : (i += 1) {
            const start = at;
            _ = cp1251.nextCodepoint(s, &at);
            font.draw(s[start..at], ox + i * step, self.y + 5, self.color);
        }
        if (self.active) gfx.drawTex(iface.texIndex(gui_cursor), ox + i * step, self.y);
    }
};

test "typing, limit and backspace" {
    var f: input.Frame = .{};
    var in: Input = .{ .x = 0, .y = 0, .max_len = 3, .color = .black, .active = true };
    for ([_]u21{ 'a', 0x0416, 0x4E2D, 'b', 'c' }, 0..) |cp, i| f.chars[i] = cp;
    f.n_chars = 5;
    try std.testing.expectEqual(Result.active, in.check(&f));
    // The Chinese character has no cp1251 code; 'c' is past the limit.
    try std.testing.expectEqualStrings("aЖb", in.text());
    f = .{};
    f.keys[0] = .backspace;
    f.keys[1] = .backspace;
    f.n_keys = 2;
    _ = in.check(&f);
    try std.testing.expectEqualStrings("a", in.text());
    f.keys[0] = .enter;
    f.n_keys = 1;
    try std.testing.expectEqual(Result.done, in.check(&f));
    try std.testing.expect(!in.active);
}
