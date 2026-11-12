//! Меню боя: WarMenu (BF_Unit.cpp:5238).
//!
//! Три полосы: «Быстрый бой (F10)», «Автобой (F11)», «Сдаться»; галочки
//! сетки, инфо-панелей и автопередачи хода (bWarGrid, bWarBar,
//! bWarEndTurn); ползунок скорости движения воинов (iBattleSpeed 0..12).
//! Галочки и скорость меняют профиль сразу, при закрытии он сохраняется.
//! Перед быстрым боем и автобоем спрашивается ограничение магии
//! (диалог 1460, SideCanCast = ответ - 1), перед сдачей диалог 1194.
//! «Быстрый бой» неактивен, если за другую сторону тоже играет человек.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const profile = @import("../data/profile.zig");
const Button = @import("../ui/button.zig").Button;
const Mask = @import("../ui/mask.zig").Mask;
const ScrollBar = @import("../ui/scrollbar.zig").ScrollBar;
const Hint = @import("../ui/hint.zig").Hint;
const Dialog = @import("dialog.zig").Dialog;

/// battlefield.dat: bmpWarMenu, bmpCheckBox, bmpCheckBird.
const bf_war_menu = 10;
const bf_check_box = 11;
const bf_check_bird = 12;
/// interface.dat: bmpGUI_MenuScrN, bmpGUI_MenuScrA.
const gui_strip_n = 86;
const gui_strip_a = 87;
/// phrases.var.
const ph_items = 255; // 255..257
const ph_options = 258; // 258..260
const ph_speed = 397;
/// dialog.var.
pub const dlg_magic = 1460;
const dlg_surrender = 1194;

pub const max_battle_speed = 12;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };
const grey: rl.Color = .{ .r = 100, .g = 100, .b = 100, .a = 255 };

const flags = [3]profile.Flag{ .war_grid, .war_bar, .war_end_turn };

/// WarMenu's return value.
pub const Result = enum(u8) { none = 0, fast = 1, auto = 2, surrender = 3 };

pub const Out = struct {
    result: Result = .none,
    /// SideCanCast[AA] from dialog 1460, when it was asked.
    can_cast: ?i32 = null,
};

pub const WarMenu = struct {
    bf: gfx.Sheet,
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    strip_shape: rl.Image,
    x: f32,
    y: f32,
    exit: Button,
    speed: ScrollBar,
    strips: [3]Mask,
    opts: [3]Mask,
    /// IsPlayed[1-AA]: the fast battle is off.
    other_human: bool,
    out: *Out,
    asking: ?usize = null,
    dialog_result: ?usize = null,
    hint: Hint = .{},
    old_cursor: u32,

    pub fn create(c: *Ctx, other_human: bool, out: *Out) !scene.Scene {
        const self = try c.gpa.create(WarMenu);
        errdefer c.gpa.destroy(self);
        var bf = try gfx.Sheet.load(c.fs, "battlefield");
        errdefer bf.deinit();
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer buttons.deinit();
        const win = bf.texIndex(bf_war_menu) orelse return error.MissingWarMenu;
        const shape = iface.imageIndex(gui_strip_n) orelse return error.MissingMenuStrip;
        errdefer rl.unloadImage(shape);
        const x: f32 = @floatFromInt(512 - @divTrunc(win.width, 2));
        const y: f32 = @floatFromInt(384 - @divTrunc(win.height, 2));
        var exit = try Button.init(&buttons, x + 167, y + 289, 305, 306, 307);
        errdefer exit.deinit();
        exit.inactive = 308;
        exit.hint = 135;
        exit.hotkey = .escape;
        const speed = try ScrollBar.init(&buttons, x + 91, y + 252, 200, 7, c.human.get(.battle_speed), 0, max_battle_speed);
        out.* = .{};
        self.* = .{
            .bf = bf,
            .iface = iface,
            .buttons = buttons,
            .strip_shape = shape,
            .x = x,
            .y = y,
            .exit = exit,
            .speed = speed,
            .strips = undefined,
            .opts = undefined,
            .other_human = other_human,
            .out = out,
            .old_cursor = c.cursor,
        };
        for (&self.strips, &self.opts, 0..) |*s, *o, i| {
            const fi: f32 = @floatFromInt(i);
            s.* = .{ .x = x + 57, .y = y + 35 + 28 * fi, .shape = &self.strip_shape };
            o.* = .rect(x + 68, y + 154 + 23 * fi, 262, 20);
        }
        c.setCursor(0);
        return scene.Scene.of(WarMenu, self, true);
    }

    pub fn deinit(self: *WarMenu, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        // PRF->Save().
        c.human.save(c.fs) catch |e| std.log.err("cannot save the profile: {t}", .{e});
        self.speed.deinit();
        self.exit.deinit();
        rl.unloadImage(self.strip_shape);
        self.buttons.deinit();
        self.iface.deinit();
        self.bf.deinit();
    }

    pub fn update(self: *WarMenu, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        if (self.asking) |i| {
            self.asking = null;
            const ans = self.dialog_result orelse 0;
            self.dialog_result = null;
            if (i == 2) {
                if (ans != 1) return .none;
            } else self.out.can_cast = @max(@as(i32, @intCast(ans)) - 1, 0);
            self.out.result = @enumFromInt(i + 1);
            return .pop;
        }
        if (self.exit.check(f)) return .pop;
        if (self.exit.hot()) self.hint.phrase(c, self.exit.hint, f.pointer.pos);
        _ = self.speed.check(f);
        c.human.set(.battle_speed, self.speed.value);
        for (&self.strips, 0..) |*m, i| {
            if (i == 0 and self.other_human) continue;
            if (m.check(f.pointer) != .click) continue;
            sound.click();
            self.asking = i;
            self.dialog_result = null;
            const d = if (i == 2) Dialog.create(c, dlg_surrender, .{ true, true, false, false }, &self.dialog_result) else Dialog.create(c, dlg_magic, .{ true, true, true, false }, &self.dialog_result);
            return .{ .push = d catch {
                self.asking = null;
                return .none;
            } };
        }
        for (&self.opts, flags) |*m, fl| {
            if (m.check(f.pointer) == .click) c.human.setFlag(fl, !c.human.flag(fl));
        }
        return .none;
    }

    pub fn draw(self: *WarMenu, c: *Ctx) void {
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 128 });
        const x = self.x;
        const y = self.y;
        gfx.drawTex(self.bf.texIndex(bf_war_menu), x, y);
        const f1 = &c.font1;
        for (self.strips, 0..) |m, i| {
            const fi: f32 = @floatFromInt(i);
            const off = i == 0 and self.other_human;
            if (!off and m.hot()) gfx.drawTex(self.iface.texIndex(gui_strip_a), m.x, m.y);
            f1.drawCentered(c.proto.phrase(ph_items + i), x + 195, y + 44 + 28 * fi, if (off) grey else color1);
        }
        for (self.opts, flags, 0..) |m, fl, i| {
            const fi: f32 = @floatFromInt(i);
            gfx.drawTex(self.bf.texIndex(bf_check_box), x + 300, y + 156 + 23 * fi);
            if (m.hot()) rl.drawRectangleV(.{ .x = m.x, .y = m.y }, .{ .x = m.w, .y = m.h }, .{ .r = 0, .g = 0, .b = 0, .a = 128 });
            f1.draw(c.proto.phrase(ph_options + i), x + 72, y + 160 + 23 * fi, if (m.hot()) color2 else color1);
            if (c.human.flag(fl)) gfx.drawTex(self.bf.texIndex(bf_check_bird), x + 300, y + 152 + 23 * fi);
        }
        f1.drawCentered(c.proto.phrase(ph_speed), x + 190, y + 237, color1);
        self.speed.draw(&self.iface, &self.buttons, c.tick);
        self.exit.draw(&self.buttons, c.tick);
        self.hint.draw(c, &self.iface);
    }
};
