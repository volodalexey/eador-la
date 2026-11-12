//! Главное меню (ScreenMenu, GameMenu.cpp:1882).
//!
//! Режимы оригинала: 0 главное, 1 кампания, 2 одиночная игра, 3 выбор
//! сетевой игры, 4 hot-seat.  Анимация (панорама, логотип, подпись) шла по
//! таймеру ItIsTime раз в iRefreshDelay мс; здесь это `tick`.  Пункты,
//! которые ведут в ещё не перенесённые экраны, открывают заглушку.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const app = @import("../app.zig");
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Mask = @import("../ui/mask.zig").Mask;
const Stub = @import("stub.zig").Stub;
const Dialog = @import("dialog.zig").Dialog;
const Credits = @import("credits.zig").Credits;
const Profile = @import("profile.zig").Profile;
const Settings = @import("settings.zig").Settings;
const NewGame = @import("new_game.zig").NewGame;
const Loading = @import("loading.zig").Loading;
const Campaign = @import("campaign.zig");
const ng = @import("../data/newgame.zig");
const profile = @import("../data/profile.zig");

/// interface.dat (Eador.cpp:378).
const gui_main_menu = 84;
const gui_eador = 85;
const gui_menu_strip_n = 86;
const gui_menu_strip_a = 87;
/// stuff.dat: logo, its shadow, the "Сотворение" caption and its shadow.
const stuff_name = 36;
const stuff_name_shadow = 37;
const stuff_creat = 38;
const stuff_creat_shadow = 39;
/// anim.dat: bmpAnName[i] = 22 + i (16 frames), bmpAnCrea[i] = 38 + i (15).
const anim_name = 22;
const anim_crea = 38;

/// Phrases (phrases.var).
const ph_new_player = 212;
const ph_main_first = 203; // 203..208: кампания .. выход
const ph_new = 209;
const ph_continue = 210;
const ph_back = 211;
const ph_hall_of_fame = 482;
const ph_book = 299;
const ph_multi_first = 445; // 445..447: один компьютер, сеть, интернет
/// dialog.var 1242: «Сетевая игра недоступна».
const dlg_no_network = 1242;
/// dialog.var 1193: «Начать новую игру? Сохранение будет потеряно».
const dlg_overwrite = 1193;
/// message.txt: site and "v%s".
const msg_site = 113;
const msg_version = 37;

const strip_x = 372;
const strip_y = 503;
const strip_step = 28;
const max_items = 7;

const black: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const grey: rl.Color = .{ .r = 100, .g = 100, .b = 100, .a = 255 };
const footer: rl.Color = .{ .r = 120, .g = 120, .b = 120, .a = 255 };

const Mode = enum { main, campaign, single, multi, hotseat };

const Item = struct { phrase: u32, enabled: bool = true };

pub const Menu = struct {
    iface: gfx.Sheet,
    stuff: gfx.Sheet,
    anim: gfx.Sheet,
    strip_shape: rl.Image,
    masks: [max_items]Mask,
    mode: Mode = .main,
    /// Answer of the last dialog opened from the menu.
    dialog_result: ?usize = null,
    /// The profile window has closed: the save files may differ now.
    profile_closed: bool = false,
    /// A new game waits for the answer to dialog 1193.
    pending_new: ?ng.Type = null,
    tick_s: f32,
    acc: f32 = 0,

    // Save files of the active profile (CheckCmpLoad etc.).
    cmp_file: bool = false,
    game_file: bool = false,
    hot_file: bool = false,

    // Animation counters, named as in ScreenMenu.
    tick: u32 = 0,
    scroll: i32 = 0,
    blend: i32 = 0,
    namer: i32 = 0,
    creator: i32 = 0,
    an_name: i32 = 15,
    an_crea: i32 = 14,
    b_an_name: bool = false,
    b_an_crea: bool = false,
    name: NamePic = .empty,
    name_opacity: ?u8 = 1,
    name_shadow_opacity: u8 = 1,
    name_shadow_dx: i32 = 0,
    creat_frame: ?usize = null,
    creat_opacity: ?u8 = 1,
    creat_shadow_opacity: u8 = 1,

    /// What bmpName holds.
    const NamePic = union(enum) {
        empty,
        /// draw_lit_sprite of the logo towards grey `v`.
        lit: u8,
        /// The logo lit towards white by `a` (fading out).
        white: u8,
        /// The logo or an animation frame (anim.dat index).
        normal: ?usize,
    };

    pub fn create(c: *Ctx) !scene.Scene {
        const self = try c.gpa.create(Menu);
        errdefer c.gpa.destroy(self);
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var stuff = try gfx.Sheet.load(c.fs, "stuff");
        errdefer stuff.deinit();
        var anim = try gfx.Sheet.load(c.fs, "anim");
        errdefer anim.deinit();
        const shape = iface.imageIndex(gui_menu_strip_n) orelse return error.MissingMenuStrip;
        self.* = .{
            .iface = iface,
            .stuff = stuff,
            .anim = anim,
            .strip_shape = shape,
            .masks = undefined,
            .tick_s = @as(f32, @floatFromInt(c.cfg.refresh_delay)) / 1000,
        };
        for (&self.masks, 0..) |*m, i| m.* = .{
            .x = strip_x,
            .y = @floatFromInt(strip_y + strip_step * i),
            .shape = &self.strip_shape,
        };
        self.checkSaves(c);
        // ScreenMenu: iGameMode 0, Menu01 (GameMenu.cpp:1938).
        c.music.setMode(c.fs, .menu);
        return scene.Scene.of(Menu, self, false);
    }

    pub fn deinit(self: *Menu, _: *Ctx) void {
        rl.unloadImage(self.strip_shape);
        self.iface.deinit();
        self.stuff.deinit();
        self.anim.deinit();
    }

    fn checkSaves(self: *Menu, c: *Ctx) void {
        const idx = c.human.fileIndex();
        var pb: [64]u8 = undefined;
        const exists = struct {
            fn f(fs: anytype, buf: []u8, comptime fmt: []const u8, i: i32) bool {
                const p = std.fmt.bufPrint(buf, fmt, .{i}) catch return false;
                return fs.exists(p);
            }
        }.f;
        self.cmp_file = exists(c.fs, &pb, "save/game{d}.cmp", idx);
        self.game_file = exists(c.fs, &pb, "save/map{d}.map", idx) and exists(c.fs, &pb, "save/game{d}.sav", idx);
        self.hot_file = exists(c.fs, &pb, "save/map{d}.hot", idx) and exists(c.fs, &pb, "save/game{d}.hot", idx);
    }

    fn items(self: *const Menu, buf: *[max_items]Item) []const Item {
        const list: []const Item = switch (self.mode) {
            .main => &.{
                .{ .phrase = ph_new_player },     .{ .phrase = ph_main_first },     .{ .phrase = ph_main_first + 1 },
                .{ .phrase = ph_main_first + 2 }, .{ .phrase = ph_main_first + 3 }, .{ .phrase = ph_main_first + 4 },
                .{ .phrase = ph_main_first + 5 },
            },
            .campaign => &.{ .{ .phrase = ph_new }, .{ .phrase = ph_continue, .enabled = self.cmp_file }, .{ .phrase = ph_hall_of_fame }, .{ .phrase = ph_back } },
            .single => &.{ .{ .phrase = ph_new }, .{ .phrase = ph_continue, .enabled = self.game_file }, .{ .phrase = ph_book }, .{ .phrase = ph_back } },
            .multi => &.{ .{ .phrase = ph_multi_first }, .{ .phrase = ph_multi_first + 1 }, .{ .phrase = ph_multi_first + 2 }, .{ .phrase = ph_back } },
            .hotseat => &.{ .{ .phrase = ph_new }, .{ .phrase = ph_continue, .enabled = self.hot_file }, .{ .phrase = ph_back } },
        };
        @memcpy(buf[0..list.len], list);
        return buf[0..list.len];
    }

    /// The first main-menu entry: the profile's name, "Новый игрок" without
    /// a profile file.
    fn label(c: *Ctx, it: Item, buf: []u8) []const u8 {
        if (it.phrase == ph_new_player and c.human.fileIndex() != -1) return c.human.name(buf);
        return c.proto.phrase(it.phrase);
    }

    fn setMode(self: *Menu, m: Mode) void {
        self.mode = m;
        for (&self.masks) |*k| k.state = .visible;
    }

    fn newGame(c: *Ctx, t: ng.Type) scene.Action {
        return .{ .push = NewGame.create(c, t) catch |e| {
            std.log.err("new game: {t}", .{e});
            return .none;
        } };
    }

    /// The first item of Одиночная игра / hot seat: with a save the player
    /// confirms first.
    fn askNewGame(self: *Menu, c: *Ctx, t: ng.Type, has_save: bool) scene.Action {
        if (!has_save) return newGame(c, t);
        self.pending_new = t;
        self.dialog_result = null;
        return .{ .push = Dialog.create(c, dlg_overwrite, .{ true, true, false, false }, &self.dialog_result) catch return .none };
    }

    /// LoadGame(1) / LoadGame(3): the loading screen reads save\map%d and
    /// save\game%d and opens MainScreen.
    pub fn loadContinue(c: *Ctx, game_type: i32) scene.Action {
        return .{ .push = Loading.create(c, game_type) catch |e| {
            std.log.err("loading screen: {t}", .{e});
            return .none;
        } };
    }

    fn stub(c: *Ctx, title: []const u8) scene.Action {
        return .{ .push = Stub.create(c, title) catch return .none };
    }

    pub fn update(self: *Menu, c: *Ctx, f: *const input.Frame, dt: f32) scene.Action {
        // iSeed++ ran once per pass of the menu loop; here once per frame.
        c.seed += 1;
        if (c.seed > 10000) c.seed = 1;
        // A game closed back to the menu: Menu01 again (GameMenu.cpp:1938).
        c.music.setMode(c.fs, .menu);
        if (self.profile_closed or c.saves_changed) {
            self.profile_closed = false;
            c.saves_changed = false;
            self.checkSaves(c);
        }
        if (self.pending_new) |t| {
            self.pending_new = null;
            const yes = self.dialog_result == 1;
            self.dialog_result = null;
            if (yes) return newGame(c, t);
        }

        self.acc += dt;
        while (self.acc >= self.tick_s) : (self.acc -= self.tick_s) self.step(c, f.pointer);

        if (f.key(.escape) and self.mode != .main)
            self.setMode(if (self.mode == .hotseat) .multi else .main);

        var buf: [max_items]Item = undefined;
        const list = self.items(&buf);
        var clicked: ?usize = null;
        for (list, 0..) |it, i| {
            if (!it.enabled) continue;
            if (self.masks[i].check(f.pointer) == .click) clicked = i;
        }
        const i = clicked orelse return .none;
        return switch (self.mode) {
            .main => switch (i) {
                // ProfileOperator(0), then CheckCmpLoad() and the rest.
                0 => .{ .push = Profile.create(c, &self.profile_closed) catch |e| {
                    std.log.err("profile window: {t}", .{e});
                    return .none;
                } },
                1 => blk: {
                    self.setMode(.campaign);
                    break :blk .none;
                },
                2 => blk: {
                    self.setMode(.single);
                    break :blk .none;
                },
                3 => blk: {
                    self.setMode(.multi);
                    break :blk .none;
                },
                4 => .{ .push = Settings.create(c, null) catch |e| {
                    std.log.err("settings window: {t}", .{e});
                    return .none;
                } },
                5 => blk: {
                    // srand(iSeed); ScreenCredits();
                    c.rand.seed(c.seed);
                    break :blk .{ .push = Credits.create(c) catch return .none };
                },
                else => .quit,
            },
            .campaign => switch (i) {
                0 => Campaign.menuNewCampaign(c, self.cmp_file),
                1 => Campaign.menuContinueCampaign(c),
                3 => blk: {
                    self.setMode(.main);
                    break :blk .none;
                },
                else => stub(c, c.proto.phrase(list[i].phrase)),
            },
            .single => switch (i) {
                0 => self.askNewGame(c, .single, self.game_file),
                1 => loadContinue(c, 1),
                3 => blk: {
                    self.setMode(.main);
                    break :blk .none;
                },
                else => stub(c, c.proto.phrase(list[i].phrase)),
            },
            .multi => switch (i) {
                0 => blk: {
                    self.setMode(.hotseat);
                    break :blk .none;
                },
                1, 2 => .{ .push = Dialog.create(c, dlg_no_network, .{ true, true, false, false }, &self.dialog_result) catch return .none },
                else => blk: {
                    self.setMode(.main);
                    break :blk .none;
                },
            },
            .hotseat => switch (i) {
                0 => self.askNewGame(c, .hotseat, self.hot_file),
                1 => loadContinue(c, 3),
                2 => blk: {
                    self.setMode(.multi);
                    break :blk .none;
                },
                else => stub(c, c.proto.phrase(list[i].phrase)),
            },
        };
    }

    /// One `if (Timer)` pass of ScreenMenu (GameMenu.cpp:2396).
    fn step(self: *Menu, c: *Ctx, p: input.Pointer) void {
        self.tick += 1;
        if (self.tick == 900) self.tick = 0;
        if (self.blend > 100 and self.namer == 0) self.namer = 1;

        if (self.namer > 0) {
            if (self.namer < 50) {
                self.name = .{ .lit = @intCast(self.namer * 5) };
                self.name_opacity = @intCast(self.namer * 5);
                self.name_shadow_opacity = @intCast(@divTrunc(self.namer * 3, 2));
                self.name_shadow_dx = @divTrunc(self.namer, 7);
                self.namer += 4;
            } else if (self.namer < 100) {
                self.name = .{ .white = @intCast(250 - (self.namer - 50) * 5) };
                self.name_opacity = null;
                self.name_shadow_opacity = @intCast(@divTrunc(self.namer * 3, 2));
                self.namer += 1;
            }
            if (self.namer == 100) {
                self.namer = 101;
                self.creator = 1;
                self.name = .{ .normal = null };
                self.creat_frame = stuff_creat;
                self.b_an_name = true;
            }
        }
        if (self.creator > 0 and self.creator < 50) {
            self.creat_opacity = @intCast(self.creator * 5);
            self.creat_shadow_opacity = @intCast(self.creator * 3);
            self.creator += 1;
        }
        if (self.creator == 50) {
            self.creator = 51;
            self.creat_opacity = null;
        }
        if (self.b_an_name and !c.english()) {
            if (self.tick % 2 == 0) {
                if (self.an_name >= 0) {
                    self.name = .{ .normal = @intCast(anim_name + self.an_name) };
                    self.an_name -= 1;
                } else {
                    self.name = .{ .normal = null };
                    self.an_name = 15;
                    self.b_an_name = false;
                    self.b_an_crea = true;
                    self.an_crea = 14;
                }
            }
        } else if (self.namer == 101 and c.rand.random(400) == 0) self.b_an_name = true;
        if (self.b_an_crea and self.tick % 2 == 0) {
            if (self.an_crea >= 0) {
                self.creat_frame = @intCast(anim_crea + self.an_crea);
                self.an_crea -= 1;
            } else {
                self.creat_frame = stuff_creat;
                self.b_an_crea = false;
            }
        }

        // Panorama scrolling by the pointer position.
        const pano_w: i32 = if (self.iface.texIndex(gui_eador)) |t| t.width else 1024;
        const max_scroll = pano_w - 1024;
        const mx = if (p.hover) p.pos.x else 512;
        if (mx < 200) {
            if (self.scroll > 0) self.scroll -= 1;
        } else if (mx > 824) {
            if (self.scroll < max_scroll) self.scroll += 1;
        } else {
            if (self.scroll > @divTrunc(max_scroll, 2)) self.scroll -= 1;
            if (self.scroll < @divTrunc(max_scroll, 2)) self.scroll += 1;
        }
        if (self.blend < 255) self.blend += 2;
    }

    fn alpha(a: ?u8) rl.Color {
        return .{ .r = 255, .g = 255, .b = 255, .a = a orelse 255 };
    }

    pub fn draw(self: *Menu, c: *Ctx) void {
        rl.clearBackground(.black);
        if (self.iface.texIndex(gui_eador)) |t| {
            const a: ?u8 = if (self.blend < 255) @intCast(self.blend) else null;
            rl.drawTextureV(t, .{ .x = @floatFromInt(-self.scroll), .y = 23 }, alpha(a));
        }
        gfx.drawTex(self.iface.texIndex(gui_main_menu), 0, 0);
        c.font1.drawCentered(msgText(c, msg_site), 512, 724, footer);
        var vb: [64]u8 = undefined;
        c.font1.drawCentered(versionText(c, &vb), 512, 744, footer);

        var buf: [max_items]Item = undefined;
        var nb: [64]u8 = undefined;
        for (self.items(&buf), 0..) |it, i| {
            const y: f32 = @floatFromInt(strip_y + strip_step * i);
            const hot = it.enabled and self.masks[i].hot();
            gfx.drawTex(self.iface.texIndex(if (hot) gui_menu_strip_a else gui_menu_strip_n), strip_x, y);
            c.font1.drawCentered(label(c, it, &nb), 512, y + 9, if (it.enabled) black else grey);
        }

        // Layer 3: the logo's shadow; layer 4: caption shadow, caption, logo.
        if (self.stuff.texIndex(stuff_name_shadow)) |t|
            rl.drawTextureV(t, .{ .x = @floatFromInt(373 + self.name_shadow_dx), .y = @floatFromInt(10 + self.name_shadow_dx) }, alpha(self.name_shadow_opacity));
        const en = c.english();
        const cx: f32 = if (en) 421 + 30 else 421;
        const cy: f32 = if (en) 129 - 35 else 129;
        if (self.stuff.texIndex(stuff_creat_shadow)) |t| rl.drawTextureV(t, .{ .x = cx + 6, .y = cy + 6 }, alpha(self.creat_shadow_opacity));
        if (self.creat_frame) |fr| {
            const t = if (fr == stuff_creat) self.stuff.texIndex(fr) else self.anim.texIndex(fr);
            if (t) |tx| rl.drawTextureV(tx, .{ .x = cx, .y = cy }, alpha(self.creat_opacity));
        }
        self.drawName();
    }

    fn drawName(self: *Menu) void {
        const pos: rl.Vector2 = .{ .x = 373, .y = 10 };
        switch (self.name) {
            .empty => {},
            .lit => |v| if (self.stuff.maskIndex(stuff_name)) |t|
                rl.drawTextureV(t, pos, .{ .r = v, .g = v, .b = v, .a = self.name_opacity orelse 255 }),
            .white => |a| {
                gfx.drawTex(self.stuff.texIndex(stuff_name), pos.x, pos.y);
                if (self.stuff.maskIndex(stuff_name)) |t| rl.drawTextureV(t, pos, .{ .r = 250, .g = 250, .b = 250, .a = a });
            },
            .normal => |fr| {
                const t = if (fr) |i| self.anim.texIndex(i) else self.stuff.texIndex(stuff_name);
                gfx.drawTex(t, pos.x, pos.y);
            },
        }
    }
};

fn msgText(c: *Ctx, i: usize) []const u8 {
    return if (i < c.proto.messages.len) c.proto.messages[i] else "";
}

/// message.txt 37 is "v%s" with stVersion.
fn versionText(c: *Ctx, buf: []u8) []const u8 {
    const m = msgText(c, msg_version);
    const at = std.mem.indexOf(u8, m, "%s") orelse return m;
    return std.fmt.bufPrint(buf, "{s}{s}{s}", .{ m[0..at], app.version, m[at + 2 ..] }) catch m;
}
