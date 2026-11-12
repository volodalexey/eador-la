//! Новый осколок: NewGame (GameMenu.cpp:1295).
//!
//! Число игроков и размер осколка, правитель и сложность каждого игрока,
//! дипломатия, исследование и настройки карты.  В hot-seat игроков можно
//! переключать между человеком и компьютером; человек берёт один из
//! профилей в save/ или создаётся новый: после «Создать карту» для каждого
//! нового игрока открывается окно профиля без списка (WindowProfile(i,
//! false)).  Правила выбора лежат в data/newgame.zig.
//!
//! «Создать карту» сохраняет настройки в профиль и открывает экран
//! сотворения (scenes/loading.zig): он создаёт осколок, пишет
//! save/map{N}.map и save/game{N}.sav (.hot в hot-seat) и открывает
//! стратегическую карту (scenes/main_screen.zig).
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const profile = @import("../data/profile.zig");
const Human = profile.Human;
const ng = @import("../data/newgame.zig");
const button = @import("../ui/button.zig");
const Button = button.Button;
const Mask = @import("../ui/mask.zig").Mask;
const Hint = @import("../ui/hint.zig").Hint;
const heraldry = @import("../ui/heraldry.zig");
const Profile = @import("profile.zig").Profile;
const difficultyName = @import("profile.zig").difficultyName;
const MapSettings = @import("map_settings.zig").MapSettings;
const mapgen = @import("../game/mapgen.zig");
const main_screen = @import("main_screen.zig");
const Creating = @import("loading.zig").Creating;

/// interface.dat: bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_NewGame,
/// bmpGUI_NGPlayer.
const gui_spell_mask = 0;
const gui_spell_select = 1;
const gui_new_game = 88;
const gui_ng_player = 89;
/// stuff.dat.
const stuff_peace = 0;
const stuff_fog = 31;
const stuff_player_type = 55;
const stuff_human = 56;
const stuff_computer = 57;
const max_hero_pic = 16;
/// bmpSmallAvatar[16].
const random_avatar = 16;

const ph_title = 227;
const ph_size = 228;
const ph_players = 229;
const ph_random = 230;
const ph_new_player = 212;
const ph_diplomacy = 98;
const ph_fog = 389;
const ph_map = 390;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };

const Btn = enum { accept, exit, map_l, map_r, pl_l, pl_r };
const specs: std.enums.EnumArray(Btn, button.Spec) = .init(.{
    .accept = .{ .x = 929, .y = 698, .normal = 226, .active = 227, .pressed = 228, .inactive = 228, .hint = 226 },
    .exit = .{ .x = 851, .y = 698, .normal = 309, .active = 310, .pressed = 311, .inactive = 311, .hint = 106 },
    .map_l = button.left(742, 37),
    .map_r = button.right(921, 37),
    .pl_l = button.left(92, 37),
    .pl_r = button.right(272, 37),
});

const SlotBtn = enum { rule_l, rule_r, diff_l, diff_r };

fn slotOffset(i: usize) [2]f32 {
    return .{ @floatFromInt((i % 4) * 248), @floatFromInt((i / 4) * 130) };
}

fn slotSpecs(i: usize) std.enums.EnumArray(SlotBtn, button.Spec) {
    _ = i;
    return .init(.{
        .rule_l = button.left(91, 97),
        .rule_r = button.right(171, 97),
        .diff_l = button.left(48, 169),
        .diff_r = button.right(222, 169),
    });
}

pub const NewGame = struct {
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    stuff: gfx.Sheet,
    smallport: gfx.Sheet,
    arms: heraldry.Heraldry,
    spell_mask: rl.Image,
    type_shape: rl.Image,
    b: std.enums.EnumArray(Btn, Button),
    sb: [ng.max_slots]std.enums.EnumArray(SlotBtn, Button),
    player_masks: [ng.max_slots]Mask,
    diplo_mask: Mask,
    fog_mask: Mask,
    map_mask: Mask,
    setup: ng.Setup,
    seed: u32,
    /// FileBase: the profiles in save/ (hot seat).
    profiles: []Human,
    /// arHumans[1..]: the new hot-seat players get their profiles here.
    new_humans: [ng.max_slots]Human = undefined,
    /// The new players whose profile windows are still to come.
    pending: [ng.max_slots]u8 = undefined,
    n_pending: usize = 0,
    next_pending: usize = 0,
    child_closed: bool = false,
    finishing: bool = false,
    /// EADOR_SCREEN=shard: «Создать карту» on the first frame.
    accept_now: bool = false,
    old_cursor: u32,
    hint: Hint = .{},

    pub fn create(c: *Ctx, t: ng.Type) !scene.Scene {
        const self = try c.gpa.create(NewGame);
        errdefer c.gpa.destroy(self);
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer buttons.deinit();
        var stuff = try gfx.Sheet.load(c.fs, "stuff");
        errdefer stuff.deinit();
        var smallport = try gfx.Sheet.load(c.fs, "smallport");
        errdefer smallport.deinit();
        var arms = try heraldry.Heraldry.load(c.fs);
        errdefer arms.deinit();
        const spell_mask = iface.imageIndex(gui_spell_mask) orelse return error.MissingSpellMask;
        errdefer rl.unloadImage(spell_mask);
        const type_shape = stuff.imageIndex(stuff_player_type) orelse return error.MissingPlayerType;
        errdefer rl.unloadImage(type_shape);

        // FileBase and arHumanID[0] (hot seat only).
        var profiles: []Human = &.{};
        var own: ?usize = null;
        if (t == .hotseat) {
            var slots: [profile.max_slots]i32 = undefined;
            const list = profile.listSlots(c.fs, &slots);
            profiles = try c.gpa.alloc(Human, list.len);
            var n: usize = 0;
            for (list) |slot| {
                const h = profile.load(c.fs, slot) orelse continue;
                if (slot == c.human.fileIndex()) own = n;
                profiles[n] = h;
                n += 1;
            }
            profiles = profiles[0..n];
        }
        errdefer c.gpa.free(profiles);

        const map = c.human.map();
        self.* = .{
            .iface = iface,
            .buttons = buttons,
            .stuff = stuff,
            .smallport = smallport,
            .arms = arms,
            .spell_mask = spell_mask,
            .type_shape = type_shape,
            .b = undefined,
            .sb = undefined,
            .player_masks = undefined,
            .diplo_mask = undefined,
            .fog_mask = undefined,
            .map_mask = undefined,
            .setup = .init(map, c.human.get(.difficulty), t, profiles.len, own),
            .seed = c.seed,
            .profiles = profiles,
            .old_cursor = c.cursor,
        };
        try button.initAll(Btn, &self.buttons, 0, 0, specs, &self.b);
        errdefer for (&self.b.values) |*k| k.deinit();
        self.b.getPtr(.exit).hotkey = .escape;
        var done: usize = 0;
        errdefer for (self.sb[0..done]) |*row| for (&row.values) |*k| k.deinit();
        for (&self.sb, &self.player_masks, 0..) |*row, *m, i| {
            const o = slotOffset(i);
            try button.initAll(SlotBtn, &self.buttons, o[0], o[1], slotSpecs(i), row);
            done += 1;
            m.* = .{ .x = 181 + o[0], .y = 70 + o[1], .shape = &self.type_shape };
        }
        self.diplo_mask = .{ .x = 100 - 6, .y = 661 - 6, .shape = &self.spell_mask };
        self.fog_mask = .{ .x = 297 - 6, .y = 661 - 6, .shape = &self.spell_mask };
        self.map_mask = .{ .x = 623 - 6, .y = 661 - 6, .shape = &self.spell_mask };
        c.setCursor(1);
        return scene.Scene.of(NewGame, self, false);
    }

    pub fn deinit(self: *NewGame, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        for (&self.sb) |*row| for (&row.values) |*k| k.deinit();
        for (&self.b.values) |*k| k.deinit();
        c.gpa.free(self.profiles);
        rl.unloadImage(self.type_shape);
        rl.unloadImage(self.spell_mask);
        self.arms.deinit();
        self.smallport.deinit();
        self.stuff.deinit();
        self.buttons.deinit();
        self.iface.deinit();
    }

    fn shardPlayers(c: *Ctx, buf: *[8]i32) []const i32 {
        for (c.proto.shard_sizes, 0..) |s, i| buf[i] = s.players;
        return buf[0..c.proto.shard_sizes.len];
    }

    pub fn update(self: *NewGame, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        if (self.finishing) return self.nextProfile(c);
        if (self.accept_now) {
            self.accept_now = false;
            return self.accept(c);
        }
        const s = &self.setup;
        var sp: [8]i32 = undefined;
        const shards = shardPlayers(c, &sp);
        const b = &self.b;
        const n = s.players();
        b.getPtr(.pl_l).on = n > ng.min_players;
        b.getPtr(.pl_r).on = n < ng.max_slots;
        b.getPtr(.map_l).on = s.size > 0;
        b.getPtr(.map_r).on = s.size < @as(i32, @intCast(shards.len)) - 1;

        if (b.getPtr(.map_l).check(f)) s.sizeDown(shards);
        if (b.getPtr(.map_r).check(f)) s.sizeUp(shards);
        if (b.getPtr(.pl_l).check(f)) s.playersDown();
        if (b.getPtr(.pl_r).check(f)) s.playersUp(shards);

        const p = f.pointer;
        const h = &c.human;
        if (self.map_mask.check(p) == .click) {
            return .{ .push = MapSettings.create(c, &s.map) catch |e| {
                std.log.err("map settings: {t}", .{e});
                return .none;
            } };
        }
        if (self.fog_mask.check(p) == .click) {
            h.setFlag(.fog, !h.flag(.fog));
            if (!h.flag(.fog)) h.setFlag(.diplomacy, false);
        }
        if (self.diplo_mask.check(p) == .click) {
            h.setFlag(.diplomacy, !h.flag(.diplomacy));
            if (h.flag(.diplomacy)) h.setFlag(.fog, true);
        }
        if (b.getPtr(.exit).check(f)) return .pop;
        if (b.getPtr(.accept).check(f)) return self.accept(c);

        for (0..s.players()) |i| {
            const row = &self.sb[i];
            if (i > 0 and s.type == .hotseat and self.player_masks[i].check(p) == .click) s.toggleKind(i);
            const arrows = i == 0 or s.rulerArrows(i);
            row.getPtr(.rule_l).enabled = arrows;
            row.getPtr(.rule_r).enabled = arrows;
            row.getPtr(.rule_l).on = i > 0;
            row.getPtr(.rule_r).on = i > 0;
            row.getPtr(.diff_l).on = s.slots[i].difficulty > 0;
            row.getPtr(.diff_r).on = s.slots[i].difficulty < ng.max_difficulty;
            if (row.getPtr(.rule_l).check(f)) s.rulerStep(i, false);
            if (row.getPtr(.rule_r).check(f)) s.rulerStep(i, true);
            if (row.getPtr(.diff_l).check(f)) s.difficultyStep(i, false);
            if (row.getPtr(.diff_r).check(f)) s.difficultyStep(i, true);
        }
        for (&b.values) |*k| if (k.hint > 0 and k.hot()) self.hint.phrase(c, k.hint, p.pos);
        return .none;
    }

    /// «Создать карту»: the new hot-seat players make their profiles first.
    fn accept(self: *NewGame, c: *Ctx) scene.Action {
        const s = &self.setup;
        self.n_pending = 0;
        for (s.slots[1..s.players()], 1..) |sl, i| {
            if (sl.kind != .human or sl.human >= 0) continue;
            var h = Human.empty();
            h.clearNames(c.proto.phrase(216), c.proto.phrase(217), &c.rand);
            h.set(.difficulty, sl.difficulty);
            self.new_humans[i] = h;
            self.pending[self.n_pending] = @intCast(i);
            self.n_pending += 1;
        }
        self.next_pending = 0;
        self.finishing = true;
        return self.nextProfile(c);
    }

    fn nextProfile(self: *NewGame, c: *Ctx) scene.Action {
        if (self.next_pending > 0) {
            // The window of pending[next_pending - 1] has closed.
            const i = self.pending[self.next_pending - 1];
            self.setup.slots[i].ruler = self.new_humans[i].fileIndex();
        }
        if (self.next_pending < self.n_pending) {
            const i = self.pending[self.next_pending];
            self.next_pending += 1;
            self.child_closed = false;
            return .{ .push = Profile.createFor(c, &self.new_humans[i], false, &self.child_closed) catch |e| {
                std.log.err("profile window: {t}", .{e});
                return .pop;
            } };
        }
        return self.generate(c);
    }

    fn generate(self: *NewGame, c: *Ctx) scene.Action {
        const s = &self.setup;
        var files: [profile.max_slots]i32 = undefined;
        for (self.profiles, 0..) |h, k| files[k] = h.fileIndex();
        const sh = c.proto.shard_sizes[@intCast(s.size)];
        const h = &c.human;
        const game = s.finish(&c.rand, self.seed, .{ sh.x, sh.y, sh.players }, files[0..self.profiles.len], h.fileIndex(), h.flag(.diplomacy), h.flag(.fog));
        h.setMap(game.map);
        h.save(c.fs) catch |e| std.log.err("cannot save the profile: {t}", .{e});
        if (c.verbose) {
            std.debug.print("new game: type {t}, seed {d}, {d} players, {d}x{d}\n", .{ game.type, game.seed, game.players(), game.map.x, game.map.y });
            for (game.slots[0..game.players()], 0..) |sl, i|
                std.debug.print("  {d}: {t} ruler {d} difficulty {d}\n", .{ i + 1, sl.kind, sl.ruler, sl.difficulty });
        }
        return self.generateMap(c, &game);
    }

    /// GenerateMap(Map, 0), SaveMap/SaveGame, then MainScreen in place of
    /// RunGame(true), under the creation screen (scenes/loading.zig).
    fn generateMap(self: *NewGame, c: *Ctx, game: *const ng.Game) scene.Action {
        var players: [ng.max_slots]mapgen.Player = undefined;
        var lands: [ng.max_slots][96]u8 = undefined;
        for (game.slots[0..game.players()], 0..) |sl, i| {
            players[i] = .{ .kind = if (sl.kind == .human) .human else .computer, .ruler = sl.ruler, .difficulty = sl.difficulty };
            if (sl.kind == .human) players[i].land = self.humanOf(c, i).land(&lands[i]);
        }
        // Mapper.cpp:569: the next hint of the profile in use.
        const hp = &c.human;
        const hints = c.proto.hints;
        const no: usize = @intCast(std.math.clamp(hp.get(.hint), 0, @as(i32, @intCast(@max(hints.len, 1))) - 1));
        const hint = if (no < hints.len) hints[no] else "";
        hp.set(.hint, if (no + 1 >= hints.len) 0 else @intCast(no + 1));
        hp.save(c.fs) catch |e| std.log.err("cannot save the profile: {t}", .{e});
        // Herb colours and ExpForLevel's factor; Gold and Gem are in
        // shard.rulers.
        var pls: [mapgen.max_players + 1]main_screen.Player = @splat(.{});
        for (players[0..game.players()], 1..) |pl, k| {
            if (pl.kind == .human) {
                const d = c.proto.difficulties[@intCast(std.math.clamp(pl.difficulty, 0, @as(i32, @intCast(c.proto.difficulties.len)) - 1))];
                pls[k] = main_screen.Player.ofHuman(self.humanOf(c, k - 1), d.exp);
            } else {
                pls[k] = main_screen.Player.ofComputer(c.proto, pl.ruler, pl.difficulty);
            }
        }
        return .{ .replace = Creating.create(c, game.*, players[0..game.players()], &lands, pls, hint) catch |e| {
            std.log.err("creating screen: {t}", .{e});
            return .pop;
        } };
    }

    /// The profile of human slot i: the one in use, a profile from save/
    /// or a new player's.
    fn humanOf(self: *NewGame, c: *Ctx, i: usize) *const Human {
        if (i == 0) return &c.human;
        const sl = self.setup.slots[i];
        if (sl.human >= 0 and sl.human < self.profiles.len) return &self.profiles[@intCast(sl.human)];
        return &self.new_humans[i];
    }

    pub fn draw(self: *NewGame, c: *Ctx) void {
        rl.clearBackground(.black);
        gfx.drawTex(self.iface.texIndex(gui_new_game), 0, 0);
        const s = &self.setup;
        const ph = c.proto;
        const f1 = &c.font1;
        f1.drawCentered(ph.phrase(ph_title), 512, 21, color1);
        f1.drawCentered(ph.phrase(ph_players), 193, 21, color1);
        var nb: [16]u8 = undefined;
        f1.drawCentered(std.fmt.bufPrint(&nb, "{d}", .{s.players()}) catch "", 193, 48, color1);
        f1.drawCentered(ph.phrase(ph_size), 842, 21, color1);
        f1.drawCentered(ph.shard_sizes[@intCast(s.size)].name, 842, 48, color1);
        f1.drawCentered(ph.phrase(ph_diplomacy), 128, 628, color1);
        f1.drawCentered(ph.phrase(ph_fog), 330, 628, color1);
        f1.drawCentered(ph.phrase(ph_map), 653, 628, color1);
        if (c.human.flag(.diplomacy)) gfx.drawTex(self.stuff.texIndex(stuff_peace), 100, 661);
        if (c.human.flag(.fog)) gfx.drawTex(self.stuff.texIndex(stuff_fog), 297, 661);
        for ([_]*const Mask{ &self.diplo_mask, &self.fog_mask, &self.map_mask }) |m|
            if (m.hot()) gfx.drawTex(self.iface.texIndex(gui_spell_select), m.x, m.y);

        for (0..s.players()) |i| self.drawSlot(c, i);
        for (&self.b.values) |*k| k.draw(&self.buttons, c.tick);
        self.hint.draw(c, &self.iface);
    }

    fn drawSlot(self: *NewGame, c: *Ctx, i: usize) void {
        const s = &self.setup;
        const sl = s.slots[i];
        const o = slotOffset(i);
        const dx = o[0];
        const dy = o[1];
        gfx.drawTex(self.iface.texIndex(gui_ng_player), 31 + dx, 81 + dy);
        if (s.type == .hotseat) {
            gfx.drawTex(self.stuff.texIndex(stuff_player_type), 181 + dx, 70 + dy);
            gfx.drawTex(self.stuff.texIndex(if (sl.kind == .computer) stuff_computer else stuff_human), 232 + dx, 95 + dy);
        }
        const random = self.arms.flag.texIndex(heraldry.flag_herb_random);
        var nb: [96]u8 = undefined;
        const human: ?*const Human = if (sl.kind == .human) blk: {
            if (i == 0) break :blk &c.human;
            if (sl.human >= 0 and sl.human < self.profiles.len) break :blk &self.profiles[@intCast(sl.human)];
            break :blk null;
        } else null;
        if (sl.kind == .computer and sl.ruler == 0 or sl.kind == .human and human == null) {
            c.font1.drawCentered(c.proto.phrase(if (sl.kind == .computer) ph_random else ph_new_player), 140 + dx, 150 + dy, color1);
            gfx.drawTex(self.smallport.texIndex(random_avatar + max_hero_pic), 114 + dx, 86 + dy);
            gfx.drawTex(random, 54 + dx, 106 + dy);
            gfx.drawTex(random, 194 + dx, 106 + dy);
        } else if (human) |h| {
            c.font1.drawCentered(h.name(&nb), 140 + dx, 150 + dy, color1);
            const av: usize = @intCast(std.math.clamp(h.get(.avatar), 0, profile.max_avatars - 1));
            gfx.drawTex(self.smallport.texIndex(av + max_hero_pic), 114 + dx, 86 + dy);
            self.arms.drawHuman(h, 54 + dx, 106 + dy);
            self.arms.drawHuman(h, 194 + dx, 106 + dy);
        } else {
            const r: usize = @intCast(std.math.clamp(sl.ruler, 1, ng.max_ruler));
            const pp = &c.proto.player_profiles[r];
            c.font1.drawCentered(pp.name, 140 + dx, 150 + dy, color1);
            // GetSmallAvatar: bmpSmallAvatar[ipProfile - 1].
            gfx.drawTex(self.smallport.texIndex(r - 1 + max_hero_pic), 114 + dx, 86 + dy);
            self.arms.drawHerb(pp.color1, pp.color2, pp.sign, 54 + dx, 106 + dy);
            self.arms.drawHerb(pp.color1, pp.color2, pp.sign, 194 + dx, 106 + dy);
        }
        c.font1.drawCentered(difficultyName(c, sl.difficulty), 140 + dx, 180 + dy, color1);
        for (&self.sb[i].values) |*k| k.draw(&self.buttons, c.tick);
    }
};
