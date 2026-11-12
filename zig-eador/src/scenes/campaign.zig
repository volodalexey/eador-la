//! Вступление кампании (NewCampaign, Campaign.cpp:1434–1628) и экран
//! Астрала (Astral, Campaign.cpp:2936–4073).
//!
//! Вступление: фон interface.dat 80, окно ввода имени (interface.dat 119,
//! аватар 17, фраза 330), диалоги 1/2/3/4/5/6 или событие 119 по ответу 4
//! диалога 3. Ввод имени пропускается, если `human.enter_data != 0`.
//!
//! Астрал: фон astral.dat 0, осколки по Terrain (1..6) и главный с гербом
//! (7), панель interface.dat 102, инфобар 103/116, подсказка обучения 22.
//! Кнопки buttons.dat: меню 104..106 (8, Low+62), Зарр 283..285 (805,
//! Low+65), статистика 98..100 (865, Low+65), конец хода 101..103 (954,
//! Low+62), атака/диалог 279..282 (Bar+100, Bar+287).
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const app = @import("../app.zig");
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Mask = @import("../ui/mask.zig").Mask;
const text = @import("../ui/text.zig");
const Button = @import("../ui/button.zig").Button;
const Dialog = @import("dialog.zig").Dialog;
const DialogOptions = @import("dialog.zig").Options;
const Loading = @import("loading.zig").Loading;
const campaign_mod = @import("../game/campaign.zig");
const Campaign = campaign_mod.Campaign;
const State = campaign_mod.State;
const Runner = campaign_mod.Runner;
const mapgen = @import("../game/mapgen.zig");
const proto_mod = @import("../data/proto.zig");
const profile = @import("../data/profile.zig");
const cp1251 = @import("../formats/cp1251.zig");

/// interface.dat: фон вступления, окно ввода, панель астрала, инфобар,
/// подсказка обучения.
const gui_screen01 = 80;
const gui_enter_window = 119;
const gui_astral_low = 102;
const gui_shard_bar = 103;
const gui_shard_bar2 = 116;
const gui_hint = 22;

/// astral.dat: фон, осколки по Terrain, главный осколок.
const astral_bg = 0;
const astral_shard = 1; // +0..5 = Terrain
const astral_main = 7;

/// buttons.dat.
const butt_menu = 104;
const butt_zarr = 283;
const butt_stat = 98;
const butt_end = 101;
const butt_attack = 279;
const butt_exit = 226;

/// Фразы (phrases.var).
const ph_enter_name = 330;
const ph_default_name = 216;
const ph_menu = 96;
const ph_zarr = 329;
const ph_stat = 99;
const ph_end_turn = 100;
const ph_attack = 200;
const ph_dialog = 350;
const ph_energy = 467;
const ph_income = 488;
const ph_size = 489;
const ph_terrain = 490;
const ph_journal = 480;

/// campaign_dialog.var.
const dlg_class = 1;
const dlg_intro2 = 2;
const dlg_intro3 = 3;
const dlg_path1 = 4;
const dlg_path2 = 5;
const dlg_path3 = 6;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };

const Stage = enum { enter_name, dialog_1, dialog_2, dialog_3, event_119, path, tutorial_map };

pub const CampaignIntro = struct {
    stage: Stage = .enter_name,
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    portraits: gfx.Sheet,
    /// Окно ввода имени.
    enter_window: ?rl.Texture2D,
    /// Кнопка выхода из окна ввода.
    butt_exit: Button,
    /// Поле ввода имени.
    input_buf: [21]u8 = @splat(0),
    input_len: usize = 0,
    input_cursor: bool = false,
    input_tick: f32 = 0,
    /// Результат диалога.
    dialog_result: ?usize = null,
    /// Выбранный класс героя (dlg_class).
    hero_class: usize = 1,
    /// Бегущий раннер событий.
    runner: Runner = .{},
    /// Состояние кампании.
    st: *State,
    old_cursor: u32,

    pub fn create(c: *Ctx) !scene.Scene {
        c.rand.seed(c.seed);
        const st = try State.create(c.gpa, c.proto);
        errdefer st.destroy();
        try campaign_mod.newCampaign(st, c.proto, &c.rand);
        const self = try c.gpa.create(CampaignIntro);
        errdefer c.gpa.destroy(self);
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer buttons.deinit();
        var portraits = try gfx.Sheet.load(c.fs, "portraits");
        errdefer portraits.deinit();
        const wt = iface.texIndex(gui_enter_window);
        const wx: f32 = if (wt) |t| 512 - @as(f32, @floatFromInt(t.width)) / 2 else 412;
        const wy: f32 = if (wt) |t| 384 - @as(f32, @floatFromInt(t.height)) / 2 else 284;
        self.* = .{
            .iface = iface,
            .buttons = buttons,
            .portraits = portraits,
            .enter_window = wt,
            .butt_exit = try Button.init(&buttons, wx + 116, wy + 237, butt_exit, butt_exit + 1, butt_exit + 2),
            .st = st,
            .old_cursor = c.cursor,
        };
        if (c.human.get(.enter_data) != 0) self.stage = .dialog_1;
        c.setCursor(0);
        return scene.Scene.of(CampaignIntro, self, true);
    }

    pub fn deinit(self: *CampaignIntro, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.st.destroy();
        self.portraits.deinit();
        self.buttons.deinit();
        self.iface.deinit();
    }

    pub fn update(self: *CampaignIntro, c: *Ctx, f: *const input.Frame, dt: f32) scene.Action {
        switch (self.stage) {
            .enter_name => {
                self.input_tick += dt;
                if (self.input_tick >= 0.5) {
                    self.input_tick = 0;
                    self.input_cursor = !self.input_cursor;
                }
                if (self.butt_exit.check(f) or f.key(.escape)) return self.finishEnter(c);
                if (f.key(.backspace) and self.input_len > 0) self.input_len -= 1;
                if (f.key(.enter) or f.key(.kp_enter)) return self.finishEnter(c);
                for (f.charList()) |ch| {
                    if (self.input_len >= 20) break;
                    if (ch < 32 or ch > 127) continue;
                    self.input_buf[self.input_len] = @intCast(ch);
                    self.input_len += 1;
                }
                return .none;
            },
            .dialog_1 => {
                if (self.dialog_result) |a| {
                    self.dialog_result = null;
                    self.hero_class = a;
                    self.stage = .dialog_2;
                }
                return .none;
            },
            .dialog_2 => {
                if (self.dialog_result != null) {
                    self.dialog_result = null;
                    self.stage = .dialog_3;
                }
                return .none;
            },
            .dialog_3 => {
                if (self.dialog_result) |a| {
                    self.dialog_result = null;
                    if (a == 4) {
                        self.runner.start(119, 0, 0);
                        self.stage = .event_119;
                    } else if (a == 1) {
                        self.stage = .path;
                        return self.showPath(c, dlg_path1);
                    } else {
                        self.stage = .path;
                        return self.showPath(c, dlg_path2);
                    }
                }
                return .none;
            },
            .event_119 => {
                if (self.dialog_result) |a| {
                    self.dialog_result = null;
                    self.runner.answer(c.proto, a);
                }
                if (self.runner.step(self.st, c.proto, &c.rand, c.human.name(&.{}))) |req| {
                    return self.showRequest(c, req);
                }
                // Событие 119 закончено.
                if (self.st.camp.dialog[124] > 0) {
                    self.st.turn = -1;
                    return .{ .replace = Astral.create(c, self.st) catch return .none };
                }
                return .none;
            },
            .path => {
                if (self.dialog_result) |a| {
                    self.dialog_result = null;
                    if (a == 1) return self.showPath(c, dlg_path3);
                    return self.startTutorialMap(c);
                }
                return .none;
            },
            .tutorial_map => return .none,
        }
    }

    fn finishEnter(self: *CampaignIntro, c: *Ctx) scene.Action {
        if (self.input_len == 0) {
            const def = c.proto.phrase(ph_default_name);
            const n = @min(def.len, 20);
            @memcpy(self.input_buf[0..n], def[0..n]);
            self.input_len = n;
        }
        c.human.setName(self.input_buf[0..self.input_len]);
        c.human.set(.enter_data, 1);
        c.human.set(.color1, 2);
        c.human.set(.color2, 4);
        c.human.set(.sign, 7);
        c.human.set(.avatar, 17);
        c.human.save(c.fs) catch |e| std.log.err("save human: {}", .{e});
        profile.saveActiveIndex(c.fs, c.human.get(.file_index)) catch {};
        self.stage = .dialog_1;
        return self.showDialog1(c);
    }

    fn showDialog1(self: *CampaignIntro, c: *Ctx) scene.Action {
        // Диалог 1: класс героя. Подстановка названия провинции 26 сделана в
        // newCampaign через mapgen, но текст диалога берём из ресурсов как есть.
        self.dialog_result = null;
        return .{ .push = Dialog.createWith(c, dlg_class, .{ true, true, true, true }, &self.dialog_result, .{ .campaign = true }) catch return .none };
    }

    fn showPath(self: *CampaignIntro, c: *Ctx, idx: usize) scene.Action {
        self.dialog_result = null;
        return .{ .push = Dialog.createWith(c, idx, .{ true, true, true, false }, &self.dialog_result, .{ .campaign = true }) catch return .none };
    }

    fn showRequest(self: *CampaignIntro, c: *Ctx, req: Runner.Request) scene.Action {
        self.dialog_result = null;
        const en: [4]bool = .{ req.codes[0] != 0, req.codes[1] != 0, req.codes[2] != 0, req.codes[3] != 0 };
        const opt: DialogOptions = if (req.question) .{ .campaign = true, .questions = req.codes } else .{ .campaign = true, .text = req.text };
        return .{ .push = Dialog.createWith(c, req.dialog, en, &self.dialog_result, opt) catch return .none };
    }

    fn startTutorialMap(self: *CampaignIntro, c: *Ctx) scene.Action {
        self.stage = .tutorial_map;
        std.log.warn("tutorial map: not ported yet", .{});
        // TODO: создание героя, магазин, библиотека, PlaceDefender, запуск карты.
        _ = c;
        return .pop;
    }

    pub fn draw(self: *CampaignIntro, c: *Ctx) void {
        gfx.drawTex(self.iface.texIndex(gui_screen01), 0, 0);
        if (self.stage == .enter_name) self.drawEnter(c);
    }

    fn drawEnter(self: *CampaignIntro, c: *Ctx) void {
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 128 });
        const wt = self.enter_window orelse return;
        const wx: f32 = 512 - @as(f32, @floatFromInt(wt.width)) / 2;
        const wy: f32 = 384 - @as(f32, @floatFromInt(wt.height)) / 2;
        if (self.portraits.texIndex(17)) |av| rl.drawTextureV(av, .{ .x = wx + 87, .y = wy + 19 }, .white);
        rl.drawTextureV(wt, .{ .x = wx, .y = wy }, .white);
        c.font1.drawCentered(c.proto.phrase(ph_enter_name), wx + 152, wy + 183, color1);
        const inp = self.input_buf[0..self.input_len];
        c.font1.drawCentered(inp, wx + 152, wy + 209, color2);
        // Мигающий курсор (упрощённо).
        if (self.input_cursor and self.input_len < 20) {
            c.font1.drawCentered("_", wx + 160 + @as(f32, @floatFromInt(self.input_len * 5)), wy + 209, color2);
        }
        self.butt_exit.draw(&self.buttons, c.tick);
    }
};

pub const Astral = struct {
    st: *State,
    iface: gfx.Sheet,
    astral: gfx.Sheet,
    buttons: gfx.Sheet,
    /// Нижняя панель с энергией.
    low_panel: ?rl.Texture2D,
    /// Инфобар выбранного осколка.
    info_bar: ?rl.Texture2D,
    /// Подсказка обучения.
    hint_win: ?rl.Texture2D,
    /// Кнопки.
    butt_menu: Button,
    butt_zarr: Button,
    butt_stat: Button,
    butt_end: Button,
    /// Выбранный осколок.
    selected: ?usize = null,
    /// Маски осколков.
    shard_masks: std.ArrayList(Mask),
    /// Результат диалога.
    dialog_result: ?usize = null,
    /// Бегущий раннер событий.
    runner: Runner = .{},
    /// Подсказка обучения: фраза и видимость.
    hint_phrase: ?usize = null,
    hint_visible: bool = false,
    old_cursor: u32,
    tick: u32 = 0,

    pub fn create(c: *Ctx, st: *State) !scene.Scene {
        const self = try c.gpa.create(Astral);
        errdefer c.gpa.destroy(self);
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var astral = try gfx.Sheet.load(c.fs, "astral");
        errdefer astral.deinit();
        var buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer buttons.deinit();
        const low_y: f32 = blk: {
            const lt = iface.texIndex(gui_astral_low);
            break :blk if (lt) |t| 768 - @as(f32, @floatFromInt(t.height)) else 668;
        };
        self.* = .{
            .st = st,
            .iface = iface,
            .astral = astral,
            .buttons = buttons,
            .low_panel = iface.texIndex(gui_astral_low),
            .info_bar = iface.texIndex(gui_shard_bar),
            .hint_win = iface.texIndex(gui_hint),
            .butt_menu = try Button.init(&buttons, 8, low_y + 62, butt_menu, butt_menu + 1, butt_menu + 2),
            .butt_zarr = try Button.init(&buttons, 805, low_y + 65, butt_zarr, butt_zarr + 1, butt_zarr + 2),
            .butt_stat = try Button.init(&buttons, 865, low_y + 65, butt_stat, butt_stat + 1, butt_stat + 2),
            .butt_end = try Button.init(&buttons, 954, low_y + 62, butt_end, butt_end + 1, butt_end + 2),
            .shard_masks = undefined,
            .old_cursor = c.cursor,
        };
        errdefer {
            self.buttons.deinit();
            self.astral.deinit();
            self.iface.deinit();
        }
        self.shard_masks = .empty;
        errdefer self.shard_masks.deinit();
        c.setCursor(0);
        c.music.setMode(c.fs, .gremlin);
        return scene.Scene.of(Astral, self, true);
    }

    pub fn deinit(self: *Astral, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.shard_masks.deinit(c.gpa);
        self.buttons.deinit();
        self.astral.deinit();
        self.iface.deinit();
    }

    pub fn update(self: *Astral, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.tick += 1;
        if (self.tick >= 900) self.tick = 0;

        if (self.dialog_result) |a| {
            self.dialog_result = null;
            self.runner.answer(c.proto, a);
        }
        if (self.runner.step(self.st, c.proto, &c.rand, c.human.name(&.{}))) |req| {
            return self.showRequest(c, req);
        }

        if (self.butt_menu.check(f) or f.key(.escape)) {
            std.log.warn("campaign menu: not ported", .{});
        }
        if (self.butt_zarr.check(f)) {
            std.log.warn("zarr events: not ported", .{});
        }
        if (self.butt_stat.check(f)) {
            std.log.warn("campaign statistics: not ported", .{});
        }
        if (self.butt_end.check(f) or f.key(.e)) {
            campaign_mod.astralTurn(self.st, c.proto, &c.rand, false);
        }

        return .none;
    }

    fn showRequest(self: *Astral, c: *Ctx, req: Runner.Request) scene.Action {
        self.dialog_result = null;
        const en: [4]bool = .{ req.codes[0] != 0, req.codes[1] != 0, req.codes[2] != 0, req.codes[3] != 0 };
        const opt: DialogOptions = if (req.question) .{ .campaign = true, .questions = req.codes } else .{ .campaign = true, .text = req.text };
        return .{ .push = Dialog.createWith(c, req.dialog, en, &self.dialog_result, opt) catch return .none };
    }

    pub fn draw(self: *Astral, c: *Ctx) void {
        gfx.drawTex(self.astral.texIndex(astral_bg), 0, 0);
        self.drawShards(c);
        gfx.drawTex(self.low_panel, 0, 768 - if (self.low_panel) |t| @as(f32, @floatFromInt(t.height)) else 100);
        self.drawEnergy(c);
        if (self.selected != null) self.drawInfoBar(c);
        if (self.hint_visible) self.drawHint(c);
        self.butt_menu.draw(&self.buttons, c.tick);
        self.butt_zarr.draw(&self.buttons, c.tick);
        self.butt_stat.draw(&self.buttons, c.tick);
        self.butt_end.draw(&self.buttons, c.tick);
    }

    fn drawShards(self: *Astral, c: *Ctx) void {
        const world_size = campaign_mod.worldSize(self.st, c.proto, 0);
        const add_y: f32 = @as(f32, @floatFromInt((world_size - 1) * 50));
        // Главный осколок игрока (400, 768-200).
        if (self.astral.texIndex(astral_main)) |mt| {
            rl.drawTextureV(mt, .{ .x = 400, .y = 768 - 200 }, .white);
            // Герб: пока не рисуем, в campaign.zig его нет.
        }
        // Нейтральные осколки.
        for (self.st.shards.items) |*sh| {
            if (sh.player != -1) continue;
            const terra: usize = @intCast(std.math.clamp(sh.terrain, 0, 5));
            const st = self.astral.texIndex(astral_shard + terra);
            const fx: f32 = @floatFromInt(sh.x);
            const fy: f32 = 768 + add_y - @as(f32, @floatFromInt(sh.y));
            gfx.drawTex(st, fx, fy);
        }
    }

    fn drawEnergy(self: *Astral, c: *Ctx) void {
        const p0 = self.st.player(0);
        var buf: [32]u8 = undefined;
        const low_y: f32 = blk: {
            const lt = self.low_panel;
            break :blk if (lt) |t| 768 - @as(f32, @floatFromInt(t.height)) else 668;
        };
        c.font1.draw(std.fmt.bufPrint(&buf, "{d}", .{p0.energy}) catch "", 135, low_y + 79, color2);
    }

    fn drawInfoBar(self: *Astral, c: *Ctx) void {
        _ = self;
        _ = c;
        // TODO: инфобар выбранного осколка.
    }

    fn drawHint(self: *Astral, c: *Ctx) void {
        if (self.hint_phrase) |ph| {
            gfx.drawTex(self.hint_win, 20, 20);
            var box = text.Box.init(40, 50, 297, 200, &c.font2, color1);
            _ = box.draw(c.proto.phrase(ph));
        }
    }
};

/// Меню кампании в главном меню: создание новой или загрузка.
pub fn menuNewCampaign(c: *Ctx, has_save: bool) scene.Action {
    if (!has_save) return .{ .push = CampaignIntro.create(c) catch return .none };
    // Диалог подтверждения 1193.
    std.log.warn("campaign: confirm overwrite dialog not ported", .{});
    return .none;
}

pub fn menuContinueCampaign(c: *Ctx) scene.Action {
    std.log.warn("campaign: load astral save not ported", .{});
    _ = c;
    return .none;
}
