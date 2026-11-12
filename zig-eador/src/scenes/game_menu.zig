//! Меню игры на карте: WindowMenu (Windower.cpp:6223).
//!
//! Пять полос: «Продолжить» (и Esc), «Настройки» (WindowSettings),
//! «В прошлое», «Сдаться», «Выход».  Что выбрано, окно пишет в `out`;
//! MainScreen делает остальное, как RunGame по bExitGame и iExitFlag.
//!
//! «В прошлое» активно после конца хода в этой сессии (bTimeShift) и не в
//! hot-seat и не на обучающей карте; «Сдаться» не на обучающей карте.
//! Кампания (iGameMode 1, DifficultyWindow, FinishCampaign, диалог 1245
//! с возвратом в Астрал) не перенесена: в кампании эти пункты неактивны,
//! как и «Сдаться» в hot-seat (CheckPlayerCondition с bLose не перенесён).
//!
//! Логотип: в C++ только тень bmpStuff[37] под окном; GOG 1.05.02 (recomp,
//! FUN_0042fc10) рисует тень на 50 % и 036Name над ней, как здесь.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const Mask = @import("../ui/mask.zig").Mask;
const Dialog = @import("dialog.zig").Dialog;
const Settings = @import("settings.zig").Settings;

/// interface.dat: bmpGUI_Menu, bmpGUI_MenuScrN, bmpGUI_MenuScrA.
const gui_menu = 133;
const gui_strip_n = 86;
const gui_strip_a = 87;
/// stuff.dat: the logo and its shadow.
const stuff_name = 36;
const stuff_name_shadow = 37;

/// phrases.var.
const ph_continue = 199;
const ph_settings = 206;
const ph_past = 395;
const ph_surrender = 393;
const ph_exit = 135;
/// dialog.var 1163: «Вы уверены, что хотите сдаться…».
const dlg_surrender = 1163;

const n_items = 5;
const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const grey: rl.Color = .{ .r = 120, .g = 120, .b = 120, .a = 255 };

pub const Result = enum {
    /// «Продолжить», Esc.
    none,
    /// bExitGame, iExitFlag -1: LoadGame of the same game.
    past,
    /// Lose(Player) of a single game: iExitFlag 1.
    surrender,
    /// bExitGame, iExitFlag 0: SaveGame and the main menu.
    exit,
};

pub const Out = struct {
    result: Result = .none,
    /// WindowSettings returned true: the map icons changed.
    redraw: bool = false,
};

pub const Enabled = struct { past: bool, surrender: bool };

pub const GameMenu = struct {
    iface: gfx.Sheet,
    stuff: gfx.Sheet,
    strip_shape: rl.Image,
    masks: [n_items]Mask,
    enabled: [n_items]bool,
    x: f32,
    y: f32,
    out: *Out,
    settings_changed: bool = false,
    dialog_result: ?usize = null,
    asking: bool = false,
    /// bEsc: the Esc that opened the window has been released.
    esc_armed: bool = false,
    old_cursor: u32,

    pub fn create(c: *Ctx, en: Enabled, out: *Out) !scene.Scene {
        const self = try c.gpa.create(GameMenu);
        errdefer c.gpa.destroy(self);
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var stuff = try gfx.Sheet.load(c.fs, "stuff");
        errdefer stuff.deinit();
        const win = iface.texIndex(gui_menu) orelse return error.MissingMenuWindow;
        const shape = iface.imageIndex(gui_strip_n) orelse return error.MissingMenuStrip;
        const x: f32 = @floatFromInt(512 - @divTrunc(win.width, 2));
        const y: f32 = @floatFromInt(384 - @divTrunc(win.height, 2));
        out.* = .{};
        self.* = .{
            .iface = iface,
            .stuff = stuff,
            .strip_shape = shape,
            .masks = undefined,
            .enabled = .{ true, true, en.past, en.surrender, true },
            .x = x,
            .y = y,
            .out = out,
            .old_cursor = c.cursor,
        };
        for (&self.masks, 0..) |*m, i| m.* = .{ .x = x + 38, .y = y + 134 + 30 * @as(f32, @floatFromInt(i)), .shape = &self.strip_shape };
        c.setCursor(0);
        return scene.Scene.of(GameMenu, self, true);
    }

    pub fn deinit(self: *GameMenu, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        rl.unloadImage(self.strip_shape);
        self.stuff.deinit();
        self.iface.deinit();
    }

    pub fn update(self: *GameMenu, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        if (self.settings_changed) {
            self.settings_changed = false;
            self.out.redraw = true;
        }
        if (self.asking) {
            self.asking = false;
            const yes = self.dialog_result == 1;
            self.dialog_result = null;
            if (yes) {
                self.out.result = .surrender;
                return .pop;
            }
        }
        if (!rl.isKeyDown(.escape)) self.esc_armed = true;
        if (self.esc_armed and f.key(.escape)) {
            sound.click();
            return .pop;
        }
        for (&self.masks, self.enabled, 0..) |*m, on, i| {
            if (!on) continue;
            if (m.check(f.pointer) != .click) continue;
            sound.click();
            switch (i) {
                0 => return .pop,
                1 => return .{ .push = Settings.create(c, &self.settings_changed) catch |e| {
                    std.log.err("settings window: {t}", .{e});
                    return .none;
                } },
                2 => {
                    self.out.result = .past;
                    return .pop;
                },
                3 => {
                    self.asking = true;
                    self.dialog_result = null;
                    return .{ .push = Dialog.create(c, dlg_surrender, .{ true, true, false, false }, &self.dialog_result) catch {
                        self.asking = false;
                        return .none;
                    } };
                },
                else => {
                    self.out.result = .exit;
                    return .pop;
                },
            }
        }
        return .none;
    }

    pub fn draw(self: *GameMenu, c: *Ctx) void {
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 128 });
        const x = self.x;
        const y = self.y;
        gfx.drawTex(self.iface.texIndex(gui_menu), x, y);
        gfx.drawTex2(self.stuff.texIndex(stuff_name_shadow), x + 44, y + 4, .{ .r = 255, .g = 255, .b = 255, .a = 128 });
        gfx.drawTex(self.stuff.texIndex(stuff_name), x + 40, y);
        const labels = [n_items]usize{ ph_continue, ph_settings, ph_past, ph_surrender, ph_exit };
        for (labels, self.masks, self.enabled) |ph, m, on| {
            const hot = on and m.hot();
            gfx.drawTex(self.iface.texIndex(if (hot) gui_strip_a else gui_strip_n), m.x, m.y);
            c.font1.drawCentered(c.proto.phrase(ph), x + 177, m.y + 9, if (on or hot) color1 else grey);
        }
    }
};
