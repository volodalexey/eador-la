//! clDialog::Show (Windower.cpp:11) for windows 1..4: a message with up
//! to four answers over the darkened screen.  The chosen answer (1-based,
//! as Show returns it) is written to `result` when the dialog closes.
//! Window 3 shows an avatar and wraps its answers; window 4 is window 2
//! with the treasure of an event (Treasurez): the gold and gems and up to
//! five items, whose card shows under the pointer and whose window
//! (WindowItemInfo) a click opens.
//!
//! Not ported yet: the question pagination of window 5, the hero advice panel, the
//! scouting/bribe answers ("?#...").
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const proto = @import("../data/proto.zig");
const Mask = @import("../ui/mask.zig").Mask;
const text = @import("../ui/text.zig");
const castle_ui = @import("../ui/castle_ui.zig");
const ItemInfo = @import("item_info.zig").ItemInfo;

/// interface.dat: bmpDialogWindow[i] = 52 + i, gold and crystal icons,
/// bmpGUI_SpellMask and bmpGUI_SpellSelect of the loot.
const gui_dialog_window = 52;
const gui_gold = 76;
const gui_crystal = 77;
const gui_spell_mask = 0;
const gui_spell_select = 1;

/// COLOR_1, COLOR_2 and the answer highlight of Show.
const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };
const hot_back: rl.Color = .{ .r = 30, .g = 30, .b = 30, .a = 255 };
const hot_text: rl.Color = .{ .r = 250, .g = 250, .b = 30, .a = 255 };
const disabled: rl.Color = .{ .r = 150, .g = 150, .b = 150, .a = 255 };

/// Window 3: the answers start at AY 298; below 464 the window is cut
/// after a 16-pixel strip of its bottom edge.
const w3_answers_y = 298;
const w3_full = 464;

/// Answer line with "$" (gold) and "&" (crystal) markers replaced by spaces.
const Answer = struct {
    buf: [256]u8 = undefined,
    len: usize = 0,
    /// Character count (CH, at most 61) and icon offsets (GL, CR).
    chars: i32 = 0,
    gold: i32 = 0,
    crystal: i32 = 0,

    fn str(self: *const Answer) []const u8 {
        return self.buf[0..self.len];
    }

    fn parse(src: []const u8) Answer {
        var a: Answer = .{};
        var i: usize = 0;
        while (i < src.len and a.chars < 61) {
            const n = std.unicode.utf8ByteSequenceLength(src[i]) catch 1;
            const end = @min(i + n, src.len);
            const c = src[i];
            if (c == '$' or c == '&') {
                if (c == '$' and a.gold == 0) a.gold = a.chars * 9 - 9;
                if (c == '&' and a.crystal == 0) a.crystal = a.chars * 9 - 9;
                if (a.len < a.buf.len) {
                    a.buf[a.len] = ' ';
                    a.len += 1;
                }
            } else if (a.len + (end - i) <= a.buf.len) {
                @memcpy(a.buf[a.len..][0 .. end - i], src[i..end]);
                a.len += end - i;
            }
            a.chars += 1;
            i = end;
        }
        return a;
    }
};

/// What Show gets besides the dialog and its answers.
pub const Options = struct {
    /// sNewText: replaces the dialog's own text (copied).
    text: ?[]const u8 = null,
    /// DialogDX: the window is moved right by this much.
    dx: i32 = 0,
    /// Treasurez for window 4.
    treasure: [6][2]i32 = @splat(.{ 0, 0 }),
    /// Pic of Show: a portraits.dat object (a hero's Pic) instead of the
    /// dialog's own picture.
    portrait: ?usize = null,
    /// Read the dialog from campaign_dialog.var.  Campaign window 5 uses
    /// the same portrait frame as window 3; its question pagination is still
    /// handled by the campaign event runner.
    campaign: bool = false,
    /// Window 5: A1..A4 of Show.  > 0 a question.var record (1-based),
    /// < 0 the dialog's own answer |n|, 0 none.
    questions: ?[proto.max_answer]i32 = null,
};

pub const Dialog = struct {
    d: *const proto.Dialog,
    enabled: [proto.max_answer]bool,
    result: *?usize,
    iface: gfx.Sheet,
    illustration: ?gfx.Sheet = null,
    portraits: ?gfx.Sheet = null,
    /// Window 4: the item pictures and the item card.
    sheets: ?castle_ui.Sheets = null,
    loot_mask: ?rl.Image = null,
    win: usize,
    x: f32,
    y: f32,
    masks: [proto.max_answer]Mask = undefined,
    answers: [proto.max_answer]Answer = undefined,
    /// Window 3: the line count of each answer and the window's height.
    lines: [proto.max_answer]usize = @splat(0),
    height: f32 = 0,
    treasure: [6][2]i32 = @splat(.{ 0, 0 }),
    loot: [5]Mask = undefined,
    hover_loot: ?usize = null,
    pointer: rl.Vector2 = .{ .x = 0, .y = 0 },
    old_cursor: u32,
    portrait: ?usize = null,
    text_buf: [1024]u8 = undefined,
    text: []const u8 = "",
    /// The answer texts as Show draws them (window 5 takes questions).
    texts: [proto.max_answer][]const u8 = @splat(""),

    /// Show with sNewText: `body` (copied) replaces the dialog's text.
    pub fn createText(c: *Ctx, index: usize, enabled: [proto.max_answer]bool, result: *?usize, body: []const u8) !scene.Scene {
        return createWith(c, index, enabled, result, .{ .text = body });
    }

    /// `index` in dialog.var; `enabled[i]` is bAnswer[i].
    pub fn create(c: *Ctx, index: usize, enabled: [proto.max_answer]bool, result: *?usize) !scene.Scene {
        return createWith(c, index, enabled, result, .{});
    }

    pub fn createWith(c: *Ctx, index: usize, enabled: [proto.max_answer]bool, result: *?usize, o: Options) !scene.Scene {
        const dialogs = if (o.campaign) c.proto.campaign_dialogs else c.proto.dialogs;
        if (index >= dialogs.len) return error.NoSuchDialog;
        const d = &dialogs[index];
        if (d.window < 1 or d.window > 4 and !(o.campaign and d.window == 5))
            return error.DialogWindowNotPorted;
        // Campaign window 5 has the portrait/question layout of window 3.
        const win: usize = if (o.campaign and d.window == 5) 2 else @intCast(d.window - 1);
        const self = try c.gpa.create(Dialog);
        errdefer c.gpa.destroy(self);
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        const wt = iface.texIndex(gui_dialog_window + win) orelse return error.MissingDialogWindow;
        self.* = .{
            .d = d,
            .enabled = enabled,
            .result = result,
            .iface = iface,
            .win = win,
            .x = @floatFromInt(512 - @divTrunc(wt.width, 2) + o.dx),
            .y = @floatFromInt(384 - @divTrunc(wt.height, 2)),
            .height = @floatFromInt(wt.height),
            .treasure = o.treasure,
            .old_cursor = c.cursor,
            .portrait = o.portrait,
        };
        self.text = d.text;
        if (o.text) |body| {
            const n = @min(body.len, self.text_buf.len);
            @memcpy(self.text_buf[0..n], body[0..n]);
            self.text = self.text_buf[0..n];
        }
        errdefer self.freeSheets();
        switch (win) {
            1, 3 => self.illustration = gfx.Sheet.load(c.fs, "illustration") catch null,
            2 => self.portraits = gfx.Sheet.load(c.fs, "portraits") catch null,
            else => {},
        }
        if (o.portrait != null and self.portraits == null) self.portraits = gfx.Sheet.load(c.fs, "portraits") catch null;
        if (win == 3) {
            self.sheets = try castle_ui.Sheets.load(c);
            self.loot_mask = self.iface.imageIndex(gui_spell_mask);
            for (&self.loot, 0..) |*m, i| {
                m.* = .{ .x = self.x + 181 + 60 * @as(f32, @floatFromInt(i)), .y = self.y + 290, .w = 58, .h = 58 };
            }
        }
        for (&self.texts, 0..) |*t, i| {
            if (o.questions) |q| {
                const code = q[i];
                if (code > 0 and code <= c.proto.questions.len) {
                    t.* = c.proto.questions[@intCast(code - 1)].text;
                } else if (code < 0 and @abs(code) <= d.n_answers) t.* = d.answers[@intCast(@abs(code) - 1)];
                self.enabled[i] = code != 0;
            } else if (i < d.n_answers) t.* = d.answers[i];
        }
        const dy: f32 = if (win == 3) 5 + 66 else 5;
        var ay: f32 = w3_answers_y;
        for (&self.masks, &self.answers, &self.lines, 0..) |*m, *a, *ln, i| {
            const fi: f32 = @floatFromInt(i);
            const src = self.texts[i];
            a.* = Answer.parse(src);
            m.* = switch (win) {
                0 => Mask.rect(self.x + 18, self.y + 162 + fi * 25, 266, 20),
                2 => blk: {
                    if (!self.enabled[i] or src.len == 0) break :blk Mask.rect(0, 0, 0, 0);
                    var buf: [text.max_lines][]const u8 = undefined;
                    ln.* = @max(text.wrap(&c.font1, src, 467, &buf).len, 1);
                    const h = @as(f32, @floatFromInt(ln.*)) * (c.font1.lineHeight() + 2) + 5;
                    const r = Mask.rect(self.x + 15, self.y + ay - 4, 470, h);
                    ay += 5 + h;
                    break :blk r;
                },
                else => Mask.rect(self.x + 16, self.y + 284 + fi * 25 + dy, 470, 20),
            };
        }
        if (win == 2 and ay < w3_full) self.height = ay + 16;
        c.setCursor(0);
        return scene.Scene.of(Dialog, self, true);
    }

    fn freeSheets(self: *Dialog) void {
        if (self.loot_mask) |img| rl.unloadImage(img);
        if (self.sheets) |*s| s.deinit();
        if (self.portraits) |*s| s.deinit();
        if (self.illustration) |*s| s.deinit();
    }

    pub fn deinit(self: *Dialog, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.freeSheets();
        self.iface.deinit();
    }

    pub fn update(self: *Dialog, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.pointer = f.pointer.pos;
        self.hover_loot = null;
        if (self.win == 3) for (&self.loot, 1..) |*m, i| {
            if (self.treasure[i][0] <= 0) continue;
            if (self.loot_mask) |*img| m.shape = img;
            const r = m.check(f.pointer);
            if (m.hot()) self.hover_loot = i;
            // A click on an item: WindowItemInfo.
            if (r != .none) return .{ .push = ItemInfo.create(c, self.treasure[i][0], self.treasure[i][1]) catch |e| {
                std.log.err("item window: {t}", .{e});
                continue;
            } };
        };
        for (&self.masks, self.enabled, 0..) |*m, on, i| {
            if (!on) continue;
            if (m.check(f.pointer) == .click) {
                self.result.* = i + 1;
                return .pop;
            }
        }
        return .none;
    }

    pub fn draw(self: *Dialog, c: *Ctx) void {
        // draw_trans_sprite(black, screen, 190): the screen at 190/255.
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 255 - 190 });
        const wt = self.iface.texIndex(gui_dialog_window + self.win);
        switch (self.win) {
            0 => {
                gfx.drawTex(wt, self.x, self.y);
                self.drawWindow1(c);
            },
            2 => {
                self.drawCut(wt);
                self.drawWindow3(c);
            },
            else => {
                gfx.drawTex(wt, self.x, self.y);
                self.drawWindow2(c);
            },
        }
    }

    /// Window 3 shorter than 464: the top part, then the bottom strip.
    fn drawCut(self: *Dialog, wt: ?rl.Texture2D) void {
        const t = wt orelse return;
        const full: f32 = @floatFromInt(t.height);
        if (self.height >= full) return gfx.drawTex(t, self.x, self.y);
        const w: f32 = @floatFromInt(t.width);
        const top = self.height - 16;
        rl.drawTextureRec(t, .{ .x = 0, .y = 0, .width = w, .height = top }, .{ .x = self.x, .y = self.y }, .white);
        rl.drawTextureRec(t, .{ .x = 0, .y = w3_full, .width = w, .height = 16 }, .{ .x = self.x, .y = self.y + top }, .white);
    }

    fn drawWindow1(self: *Dialog, c: *Ctx) void {
        var box = text.Box.init(self.x + 26, self.y + 37, 247, 96, &c.font2, color1);
        _ = box.draw(self.text);
        for (self.answers, self.enabled, self.masks, 0..) |a, on, m, i| {
            if (!on) continue;
            const fi: f32 = @floatFromInt(i);
            if (m.hot()) {
                rl.drawRectangle(@intFromFloat(m.x), @intFromFloat(m.y), 266, 20, hot_back);
                c.font1.drawCentered(a.str(), m.x + 133, m.y + 4, hot_text);
            } else c.font1.drawCentered(a.str(), self.x + 150, self.y + 166 + fi * 25, color2);
        }
    }

    fn picture(self: *Dialog) ?rl.Texture2D {
        if (self.portrait) |i| return if (self.portraits) |*s| s.texIndex(i) else null;
        return switch (self.d.pic) {
            .illustration => |i| if (self.illustration) |*s| s.texIndex(i) else null,
            .avatar => |i| if (self.portraits) |*s| s.texIndex(proto.DialogPic.portraitIndex(i)) else null,
        };
    }

    /// Windows 3: the avatar, the text and the answers wrapped and centred.
    fn drawWindow3(self: *Dialog, c: *Ctx) void {
        gfx.drawTex(self.picture(), self.x + 12, self.y + 19);
        var box = text.Box.init(self.x + 151, self.y + 23, 330, 250, &c.font2, color2);
        _ = box.draw(self.text);
        for (self.enabled, self.masks, 0..) |on, m, i| {
            if (!on or self.texts[i].len == 0) continue;
            var ab = text.Box.init(self.x + 17, m.y + 4, 467, 100, &c.font1, color2);
            ab.centre = true;
            if (m.hot()) {
                rl.drawRectangle(@intFromFloat(m.x), @intFromFloat(m.y), 470, @intFromFloat(m.h), hot_back);
                ab.color = hot_text;
            }
            _ = ab.draw(self.texts[i]);
        }
    }

    /// Windows 2 and 4.
    fn drawWindow2(self: *Dialog, c: *Ctx) void {
        gfx.drawTex(self.picture(), self.x + 16, self.y + 20);
        var box = text.Box.init(self.x + 192, self.y + 24, 286, 254, &c.font2, color2);
        _ = box.draw(self.text);
        var dy: f32 = 5;
        if (self.win == 3) {
            dy += 66;
            self.drawTreasure(c);
        }
        for (self.answers, self.enabled, self.masks, 0..) |a, on, m, i| {
            if (i >= self.d.n_answers) break;
            // "?#..." answers are generated (scouting, bribe): not ported.
            if (a.len > 1 and a.buf[1] == '#') continue;
            const fi: f32 = @floatFromInt(i);
            const half: f32 = @floatFromInt(@divTrunc(a.chars * 9, 2));
            if (on and m.hot()) {
                rl.drawRectangle(@intFromFloat(m.x), @intFromFloat(m.y), 470, 20, hot_back);
                const tx = m.x + 235 - half;
                c.font1.draw(a.str(), tx, m.y + 4, hot_text);
                self.icons(a, tx, m.y, 0, -2, 255);
            } else {
                const tx = self.x + 251 - half;
                const ty = self.y + 288 + fi * 25 + dy;
                c.font1.draw(a.str(), tx, ty, if (on) color2 else disabled);
                self.icons(a, tx, ty, -4, -6, if (on) 255 else 128);
            }
        }
        if (self.hover_loot) |i| if (self.sheets) |*sh| {
            castle_ui.itemCard(c, sh, self.pointer, self.treasure[i][0], self.treasure[i][1], null, 0, .{ 0, 0 }, .{ 0, 0 });
        };
    }

    /// Window 4: the gold and gems, the items with the highlight under the
    /// pointer.  Castle buildings (negative) are not shown.
    fn drawTreasure(self: *Dialog, c: *Ctx) void {
        var buf: [16]u8 = undefined;
        c.font1.drawCentered(std.fmt.bufPrint(&buf, "{d}", .{self.treasure[0][0]}) catch "", self.x + 100, self.y + 304, color2);
        c.font1.drawCentered(std.fmt.bufPrint(&buf, "{d}", .{self.treasure[0][1]}) catch "", self.x + 100, self.y + 329, color2);
        const sh = if (self.sheets) |*s| s else return;
        for (1..6) |i| {
            const id = self.treasure[i][0];
            if (id <= 0) continue;
            const fi: f32 = @floatFromInt(i - 1);
            gfx.drawTex(sh.item(c.proto, id), self.x + 187 + fi * 60, self.y + 296);
            if (self.hover_loot == i) gfx.drawTex(self.iface.texIndex(gui_spell_select), self.x + 181 + fi * 60, self.y + 290);
        }
    }

    fn icons(self: *Dialog, a: Answer, tx: f32, ty: f32, gold_dy: f32, crystal_dy: f32, alpha: u8) void {
        const tint: rl.Color = .{ .r = 255, .g = 255, .b = 255, .a = alpha };
        if (a.gold > 0) if (self.iface.texIndex(gui_gold)) |t|
            rl.drawTextureV(t, .{ .x = tx + @as(f32, @floatFromInt(a.gold)), .y = ty + gold_dy }, tint);
        if (a.crystal > 0) if (self.iface.texIndex(gui_crystal)) |t|
            rl.drawTextureV(t, .{ .x = tx + @as(f32, @floatFromInt(a.crystal)), .y = ty + crystal_dy }, tint);
    }
};

test "answer markers" {
    const a = Answer.parse("Дать $100");
    try std.testing.expectEqualStrings("Дать  100", a.str());
    try std.testing.expectEqual(@as(i32, 9), a.chars);
    try std.testing.expectEqual(@as(i32, 5 * 9 - 9), a.gold);
}
