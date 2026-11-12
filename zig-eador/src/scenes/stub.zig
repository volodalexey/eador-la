//! Placeholder for a screen that is not ported yet: dims the scene below and
//! shows its name; any click or key closes it.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");

const text_color: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };

pub const Stub = struct {
    title: [128]u8 = undefined,
    title_len: usize = 0,

    /// `title` is copied.
    pub fn create(c: *Ctx, title: []const u8) !scene.Scene {
        const self = try c.gpa.create(Stub);
        self.* = .{};
        self.title_len = @min(title.len, self.title.len);
        @memcpy(self.title[0..self.title_len], title[0..self.title_len]);
        return scene.Scene.of(Stub, self, true);
    }

    pub fn update(_: *Stub, _: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        return if (f.any()) .pop else .none;
    }

    pub fn draw(self: *Stub, c: *Ctx) void {
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 160 });
        c.font1.drawCentered(self.title[0..self.title_len], 512, 360, text_color);
        c.font1.drawCentered(if (std.mem.eql(u8, c.fs.lang(), "en")) "not ported yet" else "ещё не перенесено", 512, 384, text_color);
    }
};
