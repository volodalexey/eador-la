//! The game's 1024×768 frame (Allegro's screen1) as a render texture,
//! scaled to the window with letterboxing.
const std = @import("std");
const rl = @import("raylib");

pub const width = 1024;
pub const height = 768;

pub const Canvas = struct {
    target: rl.RenderTexture2D,

    pub fn init() !Canvas {
        const t = try rl.loadRenderTexture(width, height);
        rl.setTextureFilter(t.texture, .point);
        return .{ .target = t };
    }

    pub fn deinit(self: *Canvas) void {
        rl.unloadRenderTexture(self.target);
    }

    /// The frame stays opaque like Allegro's screen1: colours blend by the
    /// source alpha, the frame's own alpha only grows towards 1.
    pub fn begin(self: *Canvas) void {
        rl.beginTextureMode(self.target);
        const g = rl.gl;
        g.rlSetBlendFactorsSeparate(g.rl_src_alpha, g.rl_one_minus_src_alpha, g.rl_one, g.rl_one_minus_src_alpha, g.rl_func_add, g.rl_func_add);
        rl.beginBlendMode(.custom_separate);
    }

    pub fn end(_: *Canvas) void {
        rl.endBlendMode();
        rl.endTextureMode();
    }

    /// Where the frame lands in the window.
    pub fn viewport() rl.Rectangle {
        return fit(@floatFromInt(rl.getScreenWidth()), @floatFromInt(rl.getScreenHeight()));
    }

    pub fn present(self: *Canvas) void {
        rl.clearBackground(.black);
        const src: rl.Rectangle = .{ .x = 0, .y = 0, .width = width, .height = -height };
        rl.drawTexturePro(self.target.texture, src, viewport(), .{ .x = 0, .y = 0 }, 0, .white);
    }
};

/// The largest 4:3 rectangle centred in a `win_w`×`win_h` window.
pub fn fit(win_w: f32, win_h: f32) rl.Rectangle {
    const scale = @min(win_w / width, win_h / height);
    const w = width * scale;
    const h = height * scale;
    return .{ .x = (win_w - w) / 2, .y = (win_h - h) / 2, .width = w, .height = h };
}

/// Window coordinates -> frame coordinates.
pub fn toFrame(vp: rl.Rectangle, p: rl.Vector2) rl.Vector2 {
    return .{
        .x = (p.x - vp.x) * width / vp.width,
        .y = (p.y - vp.y) * height / vp.height,
    };
}

test "letterbox keeps 4:3 and maps the corners" {
    const vp = fit(1920, 1080);
    try std.testing.expectEqual(@as(f32, 1440), vp.width);
    try std.testing.expectEqual(@as(f32, 240), vp.x);
    const p = toFrame(vp, .{ .x = 240 + 1440, .y = 1080 });
    try std.testing.expectEqual(@as(f32, width), p.x);
    try std.testing.expectEqual(@as(f32, height), p.y);
}
