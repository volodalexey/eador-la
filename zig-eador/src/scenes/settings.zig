//! Окно настроек: WindowSettings (Windower.cpp:6688).
//!
//! Громкость звука, музыки и скорость прокрутки (ползунки), автоулучшение
//! воинов и автобой (стрелки), значки на карте и сообщения (галочки).
//! Громкости меняются сразу, как iSoundVolume/iMusicVolume оригинала;
//! «Отмена» возвращает громкости профиля, «Принять» пишет всё в профиль и
//! сохраняет его.  `changed` (bResult) становится true, если после «Принять»
//! изменились значки на карте: внутриигровому меню это говорит перерисовать
//! карту.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const profile = @import("../data/profile.zig");
const button = @import("../ui/button.zig");
const Button = button.Button;
const Mask = @import("../ui/mask.zig").Mask;
const ScrollBar = @import("../ui/scrollbar.zig").ScrollBar;
const Hint = @import("../ui/hint.zig").Hint;

/// interface.dat A134Settings; battlefield.dat bmpCheckBox, bmpCheckBird.
const gui_settings = 134;
const bf_check_box = 11;
const bf_check_bird = 12;

/// phrases.var.
const ph_title = 206;
const ph_left_title = 408;
const ph_right_title = 407;
const ph_bars = 409; // 409..412
const ph_combat = 413;
const ph_never = 414; // 414..416: никогда, с нейтралами, всегда
const ph_all = 416;
const ph_rank = 10;
const ph_map_icons = 417; // 417..426
const ph_messages = 427; // 427..433
/// message.txt hints.
const msg_map_icons = 56;
const msg_messages = 66;
const msg_level = 73;
const msg_combat = 74;

const shown_messages = 7;
const max_auto_lvl = 4;
const max_auto_combat = 2;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };
/// The hot row: the window under draw_trans_sprite of black at 128.
const row_dark: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 128 };

const Btn = enum { accept, cancel, default, lvl_l, lvl_r, batt_l, batt_r };
const specs: std.enums.EnumArray(Btn, button.Spec) = .init(.{
    .accept = .{ .x = 386, .y = 351, .normal = 305, .active = 306, .pressed = 307, .inactive = 308, .hint = 354 },
    .cancel = .{ .x = 321, .y = 351, .normal = 84, .active = 85, .pressed = 86, .inactive = 86, .hint = 106 },
    .default = .{ .x = 255, .y = 351, .normal = 87, .active = 88, .pressed = 90, .inactive = 89, .hint = 355 },
    .lvl_l = button.left(253, 253),
    .lvl_r = button.right(421, 253),
    .batt_l = button.left(253, 315),
    .batt_r = button.right(421, 315),
});

const Bar = enum { sound, music, scroll };

pub const Settings = struct {
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    bf: gfx.Sheet,
    x: f32,
    y: f32,
    b: std.enums.EnumArray(Btn, Button),
    bars: std.enums.EnumArray(Bar, ScrollBar),
    map: [profile.max_map_icons]bool,
    mess: [shown_messages]bool,
    map_masks: [profile.max_map_icons]Mask,
    mess_masks: [shown_messages]Mask,
    level_mask: Mask,
    combat_mask: Mask,
    auto_lvl: i32,
    auto_combat: i32,
    map_changed: bool = false,
    changed: ?*bool,
    old_cursor: u32,
    hint: Hint = .{},

    pub fn create(c: *Ctx, changed: ?*bool) !scene.Scene {
        const self = try c.gpa.create(Settings);
        errdefer c.gpa.destroy(self);
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer buttons.deinit();
        var bf = try gfx.Sheet.load(c.fs, "battlefield");
        errdefer bf.deinit();
        const win = iface.texIndex(gui_settings) orelse return error.MissingSettingsWindow;
        const x: f32 = @floatFromInt(512 - @divTrunc(win.width, 2));
        const y: f32 = @floatFromInt(384 - @divTrunc(win.height, 2));
        const h = &c.human;
        self.* = .{
            .iface = iface,
            .buttons = buttons,
            .bf = bf,
            .x = x,
            .y = y,
            .b = undefined,
            .bars = undefined,
            .map = undefined,
            .mess = undefined,
            .map_masks = undefined,
            .mess_masks = undefined,
            .level_mask = .rect(x + 264, y + 234, 168, 46),
            .combat_mask = .rect(x + 264, y + 294, 168, 46),
            .auto_lvl = std.math.clamp(h.get(.auto_lvl_up), 0, max_auto_lvl),
            .auto_combat = std.math.clamp(h.get(.auto_combat), 0, max_auto_combat),
            .changed = changed,
            .old_cursor = c.cursor,
        };
        try button.initAll(Btn, &self.buttons, x, y, specs, &self.b);
        errdefer for (&self.b.values) |*k| k.deinit();
        const values = [_][3]i32{
            .{ h.get(.snd_volume), 0, 255 },
            .{ h.get(.mus_volume), 0, 255 },
            .{ h.get(.scroll_speed), 1, 20 },
        };
        var done: usize = 0;
        errdefer for (self.bars.values[0..done]) |*s| s.deinit();
        for (&self.bars.values, values, 0..) |*s, v, i| {
            s.* = try ScrollBar.init(&self.buttons, x + 267, y + 99 + @as(f32, @floatFromInt(i * 54)), 145, 7, v[0], v[1], v[2]);
            done += 1;
        }
        for (&self.map, &self.map_masks, 0..) |*on, *m, i| {
            on.* = h.mapIcon(i);
            m.* = .rect(x + 40, y + 97 + @as(f32, @floatFromInt(i * 21)), 195, 20);
        }
        for (&self.mess, &self.mess_masks, 0..) |*on, *m, i| {
            on.* = h.message(i);
            m.* = .rect(x + 460, y + 97 + @as(f32, @floatFromInt(i * 21)), 195, 20);
        }
        self.updateArrows();
        c.setCursor(0);
        return scene.Scene.of(Settings, self, true);
    }

    pub fn deinit(self: *Settings, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        for (&self.bars.values) |*s| s.deinit();
        for (&self.b.values) |*k| k.deinit();
        self.bf.deinit();
        self.buttons.deinit();
        self.iface.deinit();
    }

    fn updateArrows(self: *Settings) void {
        self.b.getPtr(.lvl_l).on = self.auto_lvl > 0;
        self.b.getPtr(.lvl_r).on = self.auto_lvl < max_auto_lvl;
        self.b.getPtr(.batt_l).on = self.auto_combat > 0;
        self.b.getPtr(.batt_r).on = self.auto_combat < max_auto_combat;
    }

    /// iSoundVolume and iMusicVolume follow the bars.
    fn applyVolumes(self: *Settings, c: *Ctx) void {
        sound.setVolume(self.bars.get(.sound).value);
        c.music.setVolume(self.bars.get(.music).value);
    }

    pub fn update(self: *Settings, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        const p = f.pointer;
        // Active: while a thumb is dragged only its bar is checked.
        for (&self.bars.values) |*s| if (s.state == .dragging) {
            _ = s.check(f);
            self.applyVolumes(c);
            return .none;
        };

        const h = &c.human;
        if (self.b.getPtr(.cancel).check(f)) {
            sound.setVolume(h.get(.snd_volume));
            c.music.setVolume(h.get(.mus_volume));
            return .pop;
        }
        if (self.b.getPtr(.lvl_l).check(f)) self.auto_lvl -= 1;
        if (self.b.getPtr(.lvl_r).check(f)) self.auto_lvl += 1;
        if (self.b.getPtr(.batt_l).check(f)) self.auto_combat -= 1;
        if (self.b.getPtr(.batt_r).check(f)) self.auto_combat += 1;
        self.auto_lvl = std.math.clamp(self.auto_lvl, 0, max_auto_lvl);
        self.auto_combat = std.math.clamp(self.auto_combat, 0, max_auto_combat);
        if (self.b.getPtr(.accept).check(f)) {
            self.accept(c);
            return .pop;
        }
        if (self.b.getPtr(.default).check(f)) self.setDefault(c);
        self.updateArrows();

        for (&self.bars.values) |*s| {
            if (@intFromEnum(s.check(f)) > 2) self.applyVolumes(c);
        }
        for (&self.map, &self.map_masks, 0..) |*on, *m, i| {
            if (m.check(p) == .click) {
                on.* = !on.*;
                self.map_changed = true;
            }
            if (m.hot()) self.hint.message(c, msg_map_icons + i, p.pos);
        }
        for (&self.mess, &self.mess_masks, 0..) |*on, *m, i| {
            if (m.check(p) == .click) on.* = !on.*;
            if (m.hot()) self.hint.message(c, msg_messages + i, p.pos);
        }
        _ = self.level_mask.check(p);
        if (self.level_mask.hot()) self.hint.message(c, msg_level, p.pos);
        _ = self.combat_mask.check(p);
        if (self.combat_mask.hot()) self.hint.message(c, msg_combat, p.pos);
        for (&self.b.values) |*k| if (k.hint > 0 and k.hot()) self.hint.phrase(c, k.hint, p.pos);
        return .none;
    }

    fn accept(self: *Settings, c: *Ctx) void {
        const h = &c.human;
        h.set(.snd_volume, self.bars.get(.sound).value);
        h.set(.mus_volume, self.bars.get(.music).value);
        h.set(.scroll_speed, self.bars.get(.scroll).value);
        h.set(.auto_lvl_up, self.auto_lvl);
        h.set(.auto_combat, self.auto_combat);
        for (self.map, 0..) |on, i| h.setMapIcon(i, on);
        for (self.mess, 0..) |on, i| h.setMessage(i, on);
        h.save(c.fs) catch |e| std.log.err("cannot save the profile: {t}", .{e});
        self.applyVolumes(c);
        if (self.changed) |r| r.* = self.map_changed;
    }

    /// buttDefault: the constructor's values.  arMess[7..9] are not shown
    /// but are reset too; they are written only on «Принять».
    fn setDefault(self: *Settings, c: *Ctx) void {
        self.bars.getPtr(.sound).setValue(225);
        self.bars.getPtr(.music).setValue(255);
        self.bars.getPtr(.scroll).setValue(profile.default_scroll_speed);
        self.applyVolumes(c);
        self.auto_lvl = 0;
        self.auto_combat = 0;
        var d = profile.Human.empty();
        d.setDefaultNotices();
        for (&self.map, 0..) |*on, i| on.* = d.mapIcon(i);
        for (&self.mess) |*on| on.* = true;
    }

    pub fn draw(self: *Settings, c: *Ctx) void {
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 128 });
        const x = self.x;
        const y = self.y;
        gfx.drawTex(self.iface.texIndex(gui_settings), x, y);
        const ph = c.proto;
        const f1 = &c.font1;
        f1.drawCentered(ph.phrase(ph_title), x + 349, y + 20, color1);
        f1.drawCentered(ph.phrase(ph_left_title), x + 133, y + 65, color1);
        f1.drawCentered(ph.phrase(ph_right_title), x + 556, y + 65, color1);
        for (0..4) |i| f1.drawCentered(ph.phrase(ph_bars + i), x + 346, y + 77 + @as(f32, @floatFromInt(i * 54)), color1);
        f1.drawCentered(ph.phrase(ph_combat), x + 346, y + 300, color1);

        var buf: [96]u8 = undefined;
        const lvl = switch (self.auto_lvl) {
            0 => ph.phrase(ph_never),
            1 => std.fmt.bufPrint(&buf, "1 {s}", .{ph.phrase(ph_rank)}) catch "",
            2 => std.fmt.bufPrint(&buf, "1-2 {s}", .{ph.phrase(ph_rank)}) catch "",
            3 => std.fmt.bufPrint(&buf, "1-3 {s}", .{ph.phrase(ph_rank)}) catch "",
            else => ph.phrase(ph_all),
        };
        f1.drawCentered(lvl, x + 346, y + 263, color2);
        f1.drawCentered(ph.phrase(ph_never + @as(usize, @intCast(self.auto_combat))), x + 346, y + 326, color2);

        self.drawChecks(c, &self.map, &self.map_masks, ph_map_icons, 45, 210);
        self.drawChecks(c, &self.mess, &self.mess_masks, ph_messages, 465, 630);

        for (&self.b.values) |*k| k.draw(&self.buttons, c.tick);
        for (&self.bars.values) |*s| s.draw(&self.iface, &self.buttons, c.tick);
        self.hint.draw(c, &self.iface);
    }

    fn drawChecks(self: *Settings, c: *Ctx, on: []const bool, masks: []const Mask, first: usize, tx: f32, bx: f32) void {
        const box = self.bf.texIndex(bf_check_box);
        const bird = self.bf.texIndex(bf_check_bird);
        for (on, masks, 0..) |v, m, i| {
            const dy: f32 = @floatFromInt(i * 21);
            c.font1.draw(c.proto.phrase(first + i), self.x + tx, self.y + 101 + dy, color1);
            gfx.drawTex(box, self.x + bx, self.y + 100 + dy);
            if (m.hot()) rl.drawRectangleV(.{ .x = m.x, .y = m.y }, .{ .x = m.w, .y = m.h }, row_dark);
            if (v) gfx.drawTex(bird, self.x + bx, self.y + 96 + dy);
        }
    }
};
