//! WindowInfo (Windower.cpp:2202): a title, a text and a picture (shown
//! left and right) in bmpGUI_AbilityInfo over the darkened screen, closed
//! by its button or Esc.  The text is a UI_TextBox with scroll arrows
//! (Interface.cpp:752, 937): the arrows and the wheel over the box move it
//! by a line while it is taller than the box.
//!
//! The same window with another layout is WindowAbilityInfo,
//! WindowMedalInfo, WindowRaceInfo, WindowEffectInfo (a line with the
//! duration above a lower box) and WindowSkillInfo (bmpGUI_SkillInfo); see
//! info_windows.zig.  The window keeps copies of its strings.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Button = @import("../ui/button.zig").Button;
const text = @import("../ui/text.zig");

/// buttons.dat: exit 226..228, the text box arrows 48..51 (up) and 52..55
/// (down).
const butt_exit = 226;
const butt_up = 48;
const butt_down = 52;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };

/// Lines hidden below the box (ScrollSize) when the text is `n` lines of
/// `step` pixels; 0 when it fits (Pic1->h - 15 - h <= 3).
pub fn scrollSize(n: usize, step: i32, h: i32) i32 {
    const extra = @as(i32, @intCast(n)) * step - h;
    if (extra <= 3) return 0;
    return @divTrunc(extra, step) + @intFromBool(@mod(extra, step) > 3);
}

/// UI_TextBox with its two arrows: Font2 text, COLOR_1, lines
/// lineHeight + 2 apart.
pub const ScrollBox = struct {
    x: f32,
    y: f32,
    w: f32,
    /// The height after Init's rounding.
    h: i32,
    up: Button,
    down: Button,
    /// CurrentLine and ScrollSize.
    line: i32 = 0,
    scroll: i32 = 0,

    /// TextBox.Init(x, y, w, h, ..., up, down) + Activate(body).
    pub fn init(c: *Ctx, buttons: *gfx.Sheet, x: f32, y: f32, w: f32, h: i32, up_at: [2]f32, down_at: [2]f32, body: []const u8) !ScrollBox {
        var up = try Button.init(buttons, up_at[0], up_at[1], butt_up, butt_up + 1, butt_up + 2);
        errdefer up.deinit();
        up.inactive = butt_up + 3;
        var down = try Button.init(buttons, down_at[0], down_at[1], butt_down, butt_down + 1, butt_down + 2);
        errdefer down.deinit();
        down.inactive = butt_down + 3;
        var buf: [text.max_lines][]const u8 = undefined;
        const n = text.wrap(&c.font2, body, w, &buf).len;
        const bh = text.boxHeight(h);
        const scroll = scrollSize(n, lineStep(c), bh);
        up.on = scroll > 0;
        down.on = scroll > 0;
        return .{ .x = x, .y = y, .w = w, .h = bh, .up = up, .down = down, .scroll = scroll };
    }

    pub fn deinit(self: *ScrollBox) void {
        self.down.deinit();
        self.up.deinit();
    }

    fn lineStep(c: *Ctx) i32 {
        return @intFromFloat(c.font2.lineHeight() + 2);
    }

    /// TextBox.Check: the arrows and the wheel over the box.
    pub fn update(self: *ScrollBox, f: *const input.Frame) void {
        if (self.up.check(f) and self.line > 0) self.line -= 1;
        if (self.down.check(f) and self.line < self.scroll) self.line += 1;
        const p = f.pointer;
        const bh: f32 = @floatFromInt(self.h);
        if (self.scroll > 0 and p.hover and p.pos.x >= self.x - 5 and p.pos.x < self.x + self.w + 5 and p.pos.y > self.y - 10 and p.pos.y < self.y + bh + 10) {
            if (p.wheel > 0 and self.line > 0) self.line -= 1;
            if (p.wheel < 0 and self.line < self.scroll) self.line += 1;
        }
    }

    pub fn draw(self: *const ScrollBox, c: *Ctx, buttons: *gfx.Sheet, body: []const u8) void {
        const step: f32 = @floatFromInt(lineStep(c));
        const bh: f32 = @floatFromInt(self.h);
        var buf: [text.max_lines][]const u8 = undefined;
        const lines = text.wrap(&c.font2, body, self.w, &buf);
        rl.beginScissorMode(@intFromFloat(self.x), @intFromFloat(self.y), @intFromFloat(self.w), self.h);
        for (lines, 0..) |line, k| {
            const row = @as(f32, @floatFromInt(@as(i32, @intCast(k)) - self.line)) * step;
            if (row + step <= 0) continue;
            if (row >= bh) break;
            c.font2.draw(line, self.x, self.y + row, color1);
        }
        rl.endScissorMode();
        self.up.draw(buttons, c.tick);
        self.down.draw(buttons, c.tick);
    }
};

/// Where a window of this family puts its parts, relative to its corner.
pub const Layout = struct {
    /// interface.dat object of the background.
    gui: usize,
    pics: [2][2]f32,
    exit: [2]f32,
    title: [2]f32,
    /// The line under the title (WindowEffectInfo, WindowSkillInfo).
    sub_y: f32 = 0,
    /// x, y, w, h of the text box.
    box: [4]f32,
    up: [2]f32,
    down: [2]f32,
};

/// bmpGUI_AbilityInfo (75AbilityInfo): WindowInfo, WindowAbilityInfo,
/// WindowMedalInfo, WindowRaceInfo.
pub const ability_layout: Layout = .{
    .gui = 75,
    .pics = .{ .{ 21, 20 }, .{ 328, 20 } },
    .exit = .{ 168, 342 },
    .title = .{ 200, 36 },
    .box = .{ 31, 112, 342, 193 },
    .up = .{ 185, 86 },
    .down = .{ 185, 313 },
};

/// WindowEffectInfo: the duration at y 112, the box from 132.
pub const effect_layout: Layout = blk: {
    var l = ability_layout;
    l.sub_y = 112;
    l.box = .{ 31, 132, 342, 173 };
    break :blk l;
};

/// bmpGUI_SkillInfo (73SkillInfo): WindowSkillInfo.
pub const skill_layout: Layout = .{
    .gui = 73,
    .pics = .{ .{ 22, 23 }, .{ 397, 23 } },
    .exit = .{ 205, 352 },
    .title = .{ 240, 29 },
    .sub_y = 59,
    .box = .{ 35, 114, 405, 205 },
    .up = .{ 224, 86 },
    .down = .{ 224, 324 },
};

pub const WindowInfo = struct {
    layout: Layout,
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    pics: gfx.Sheet,
    /// Object of `pics` shown left and right; null for none.
    pic: ?usize,
    title_buf: [256]u8 = undefined,
    title: []const u8 = "",
    sub_buf: [128]u8 = undefined,
    sub: []const u8 = "",
    body_buf: [4096]u8 = undefined,
    body: []const u8 = "",
    x: f32,
    y: f32,
    exit: Button,
    box: ScrollBox,
    old_cursor: u32,

    /// WindowInfo(title, body, bmpStuff[pic]).
    pub fn create(c: *Ctx, title: []const u8, body: []const u8, pic: usize) !scene.Scene {
        return createFrom(c, title, body, "stuff", pic);
    }

    /// The picture is object `pic` of sheet `sheet` (WindowRaceInfo:
    /// races.dat).
    pub fn createFrom(c: *Ctx, title: []const u8, body: []const u8, sheet: []const u8, pic: usize) !scene.Scene {
        return open(c, ability_layout, title, "", body, sheet, pic);
    }

    pub fn open(c: *Ctx, layout: Layout, title: []const u8, sub: []const u8, body: []const u8, sheet: []const u8, pic: ?usize) !scene.Scene {
        const self = try c.gpa.create(WindowInfo);
        errdefer c.gpa.destroy(self);
        self.* = .{
            .layout = layout,
            .iface = undefined,
            .buttons = undefined,
            .pics = undefined,
            .pic = pic,
            .x = 0,
            .y = 0,
            .exit = undefined,
            .box = undefined,
            .old_cursor = c.cursor,
        };
        self.title = copy(&self.title_buf, title);
        self.sub = copy(&self.sub_buf, sub);
        self.body = copy(&self.body_buf, body);
        self.iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer self.iface.deinit();
        self.buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer self.buttons.deinit();
        self.pics = try gfx.Sheet.load(c.fs, sheet);
        errdefer self.pics.deinit();
        const wt = self.iface.texIndex(layout.gui) orelse return error.MissingInfoWindow;
        const x: f32 = @floatFromInt(512 - @divTrunc(wt.width, 2));
        const y: f32 = @floatFromInt(384 - @divTrunc(wt.height, 2));
        self.x = x;
        self.y = y;
        self.exit = try Button.init(&self.buttons, x + layout.exit[0], y + layout.exit[1], butt_exit, butt_exit + 1, butt_exit + 2);
        errdefer self.exit.deinit();
        self.exit.hotkey = .escape;
        const b = layout.box;
        self.box = try ScrollBox.init(c, &self.buttons, x + b[0], y + b[1], b[2], @intFromFloat(b[3]), .{ x + layout.up[0], y + layout.up[1] }, .{ x + layout.down[0], y + layout.down[1] }, self.body);
        c.setCursor(0);
        return scene.Scene.of(WindowInfo, self, true);
    }

    fn copy(buf: []u8, s: []const u8) []const u8 {
        const n = @min(s.len, buf.len);
        @memcpy(buf[0..n], s[0..n]);
        return buf[0..n];
    }

    pub fn deinit(self: *WindowInfo, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.box.deinit();
        self.exit.deinit();
        self.pics.deinit();
        self.buttons.deinit();
        self.iface.deinit();
    }

    pub fn update(self: *WindowInfo, _: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        if (self.exit.check(f)) return .pop;
        self.box.update(f);
        return .none;
    }

    pub fn draw(self: *WindowInfo, c: *Ctx) void {
        const l = self.layout;
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 255 - 190 });
        gfx.drawTex(self.iface.texIndex(l.gui), self.x, self.y);
        if (self.pic) |pic| for (l.pics) |at| gfx.drawTex(self.pics.texIndex(pic), self.x + at[0], self.y + at[1]);
        c.font1.drawCentered(self.title, self.x + l.title[0], self.y + l.title[1], color1);
        if (l.sub_y > 0) c.font1.drawCentered(self.sub, self.x + l.title[0], self.y + l.sub_y, color1);
        self.box.draw(c, &self.buttons, self.body);
        self.exit.draw(&self.buttons, c.tick);
    }
};

test "text box scroll size" {
    // 196-pixel box, 14-pixel lines: 14 lines fit.
    try std.testing.expectEqual(@as(i32, 0), scrollSize(14, 14, 196));
    try std.testing.expectEqual(@as(i32, 1), scrollSize(15, 14, 196));
    try std.testing.expectEqual(@as(i32, 3), scrollSize(17, 14, 196));
    // 3 pixels over do not scroll; 4 add a line.
    try std.testing.expectEqual(@as(i32, 0), scrollSize(1, 199, 196));
    try std.testing.expectEqual(@as(i32, 1), scrollSize(1, 200, 196));
}
