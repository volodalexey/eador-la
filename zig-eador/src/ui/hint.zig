//! Pop-up hints: MakeStringHint (a phrase on interface.dat 71StrHint,
//! stretched to the text) and MakeTextHint (a message.txt text on
//! A137SmallHint).  The original set bHint during a frame's checks and drew
//! the hint beside the mouse sprite (ShowActiveObjects); a scene does the
//! same with `Hint`: reset it each update, set it, draw it last.
const std = @import("std");
const rl = @import("raylib");
const gfx = @import("../platform/gfx.zig");
const text = @import("text.zig");
const Ctx = @import("../app.zig").Ctx;

const gui_str_hint = 71;
const gui_small_hint = 137;
const max_w = 400;
const black: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };

pub const Hint = struct {
    what: What = .none,
    pos: rl.Vector2 = .{ .x = 0, .y = 0 },

    pub const What = union(enum) {
        none,
        /// A phrase (MakeStringHint).
        string: []const u8,
        /// A message.txt text (MakeTextHint).
        text: []const u8,
    };

    pub fn clear(self: *Hint) void {
        self.what = .none;
    }

    pub fn phrase(self: *Hint, c: *Ctx, i: u32, at: rl.Vector2) void {
        if (i == 0) return;
        self.what = .{ .string = c.proto.phrase(i) };
        self.pos = at;
    }

    pub fn message(self: *Hint, c: *Ctx, i: usize, at: rl.Vector2) void {
        if (i == 0 or i >= c.proto.messages.len) return;
        self.what = .{ .text = c.proto.messages[i] };
        self.pos = at;
    }

    pub fn draw(self: *const Hint, c: *Ctx, iface: *gfx.Sheet) void {
        switch (self.what) {
            .none => {},
            .string => |s| {
                const bg = iface.texIndex(gui_str_hint) orelse return;
                const w: f32 = @min(c.font1.measure(s) + 60, max_w);
                const h: f32 = @floatFromInt(bg.height);
                const at = self.place(c, w, h);
                const bw: f32 = @floatFromInt(bg.width);
                rl.drawTextureRec(bg, .{ .x = 0, .y = 0, .width = w - 23, .height = h }, at, .white);
                rl.drawTextureRec(bg, .{ .x = bw - 23, .y = 0, .width = 23, .height = h }, .{ .x = at.x + w - 23, .y = at.y }, .white);
                c.font1.drawCentered(s, at.x + w / 2, at.y + 9, black);
            },
            .text => |s| {
                const bg = iface.texIndex(gui_small_hint) orelse return;
                const at = self.place(c, @floatFromInt(bg.width), @floatFromInt(bg.height));
                gfx.drawTex(bg, at.x, at.y);
                var buf: [text.max_lines][]const u8 = undefined;
                const lines = text.wrap(&c.font2, s, 234, &buf).len;
                const ln: f32 = if (lines < 6) @floatFromInt((6 - lines) * 10) else 0;
                const box = text.Box.init(at.x + 15, at.y + 21 + ln, 234, 107, &c.font2, black);
                _ = box.draw(s);
            },
        }
    }

    /// Below and right of the mouse sprite, moved inside the frame.
    pub fn place(self: *const Hint, c: *Ctx, w: f32, h: f32) rl.Vector2 {
        const cur = c.mouse.texIndex(c.cursor);
        const cw = gfx.texW(cur);
        const x = @floor(self.pos.x) + cw;
        const y = @floor(self.pos.y) + gfx.texH(cur);
        if (x + w <= 1024 and y + h <= 768) return .{ .x = x, .y = y };
        if (x + w <= 1024) return .{ .x = x, .y = 768 - h };
        if (y + h <= 768) return .{ .x = 1024 - w, .y = y };
        return .{ .x = x - w - cw, .y = 768 - h };
    }
};
