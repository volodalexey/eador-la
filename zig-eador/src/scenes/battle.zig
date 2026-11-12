//! The battle screen of InitBattle (Battlefield.cpp:5037): the field
//! picture of MakeBackground, the units with their shadows and bars, the
//! side panel of the selected unit (ShowUnit), the placement of the
//! player's units (UnitPlacement), the player's orders (move, strike,
//! shoot, rest, next, end of turn), the computer side and the result
//! window (EndBattleScreen).
//!
//! The animations follow G_Object (G_Object.cpp): a move runs along the
//! cell path at 2 + speed + iBattleSpeed pixels a tick, a melee strike
//! lunges a third of the way to the target and back at 5, a missile flies
//! at 15 + 3·iBattleSpeed and turns towards its target (some spin), the
//! damage rises 30 pixels above the target, and a death (G_Death type 1)
//! fades the unit out under twelve clouds and a skull while the corpse
//! fades in.  One tick is iRefreshDelay milliseconds.
//!
//! The human player's spell book (Spellbook, SpellPrepare, the target
//! click), the unit window (unit_info.zig) on a right click and the
//! effect icons of the panel are here too.
//! Alt over another unit shows ShowEnemy (stats, abilities and effects);
//! while choosing a spell target it also works over the active unit.
//!
//! The war menu (WarMenu, war_menu.zig) opens with the menu button or O;
//! G toggles the grid, F10 is the fast battle (no animation), F11 the
//! auto battle (Ctrl first asks the limit of magic, dialog 1460), Esc
//! gives the side back to the player at its next turn.  The profile's
//! iAutoCombat > 0 starts a fast battle against the neutrals at once.
//!
//! Not ported: the castle walls and towers, the hints of
//! the parameters, the unit pictures of the result window (unit icons are
//! shown instead), the windows of the loot left over (WindowTakeLoot) and
//! of the level-ups and medals; battles with heroes and hired defenders
//! are not opened from the map.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const fx = @import("battle_fx.zig");
const proto = @import("../data/proto.zig");
const rand = @import("../data/rand.zig");
const bt = @import("../game/battle.zig");
const bs = @import("../game/battle_spell.zig");
const mapgen = @import("../game/mapgen.zig");
const hero_mod = @import("../game/hero.zig");
const Button = @import("../ui/button.zig").Button;
const Hint = @import("../ui/hint.zig").Hint;
const Mask = @import("../ui/mask.zig").Mask;
const castle_ui = @import("../ui/castle_ui.zig");
const text = @import("../ui/text.zig");
const WindowInfo = @import("window_info.zig").WindowInfo;
const UnitInfo = @import("unit_info.zig").UnitInfo;
const war_menu = @import("war_menu.zig");
const WarMenu = war_menu.WarMenu;
const Dialog = @import("dialog.zig").Dialog;

/// battlefield.dat objects (LoadBFPics).
const bf_mask = 0;
const bf_hexx = 1;
const bf_panel = 2;
const bf_grass = 3;
const bf_swamp = 6;
const bf_unit_bar = 13;
const bf_target = 14;
const bf_corpse = 15;
const bf_select = 16;
const bf_spellbook = 17;
const bf_target_heal = 18;
const bf_spell_shadow = 20;
const bf_spell_panel = 21;
const bf_grid = 19;
const bf_missile = 28;
const bf_missile_last = 111;
const bf_chasm = 51;
const bf_thicket = 53;
const bf_mount = 55;
const bf_fen = 57;
const bf_forest = 59;
/// anim.dat: bmpSkull, bmpJaw, bmpCloud[0..7].
const anim_skull = 68;
const anim_jaw = 69;
const anim_cloud = 70;
const anim_web = 53;
/// interface.dat: bmpGUI_Victory, bmpGUI_Lose.
const gui_victory = 36;
const gui_lose = 37;
/// interface.dat: bmpGUI_Hint, used by the end-of-battle warning too.
const gui_hint = 22;
/// interface.dat: bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_NoGem.
const gui_spell_mask = 0;
const gui_spell_select = 1;
const gui_no_gem = 93;
/// stuff.dat: the two pictures over a spell cast in this battle.
const stuff_dark1 = 40;
const stuff_dark2 = 41;
/// Cursors of the spell mode.
const cur_magic = 15;
const cur_no_magic = 16;
/// buttons.dat.
const butt_rest = 0;
const butt_next = 4;
const butt_end = 8;
const butt_start = 36;
const butt_exit = 226;
const butt_spell = 21;
const butt_cancel_spell = 25;
const butt_close = 312;
/// buttMenu: normal, active, pressed, inactive.
const butt_menu = 87;
const hint_menu = 351;
/// bmpLowString: phrase 491 on interface.dat 71StrHint.
const ph_auto_combat = 491;
const gui_str_hint = 71;
/// Phrases of the buttons and the windows.
const hint_rest = 119;
const hint_next = 120;
const hint_end = 100;
const hint_start = 127;
const hint_spell = 123;
/// txtSpTarget: "Выберите жертву" / "Выберите цель".
const ph_sacrifice = 253;
const ph_target = 254;
/// The power line of the spell book: power, duration, summon level.
const ph_power = 552;
const ph_duration = 553;
const ph_summon = 554;
const ph_params = 252;
/// The field is 884 pixels wide, the panel takes the rest.
const field_w = 884;
/// The warning is the same parchment used by the tutorial hint.
const time_warning_x: f32 = 20;
const time_warning_y: f32 = 20;
const msg_time_warning = 25;
const dlg_time_limit = 1093;
/// stuff.dat pictures and messages of WindowInfo for the terrain.
const msg_terrain = 37;
const stuff_terrain = 42;
const ph_obstacle = 434;
const msg_obstacle = 35;
const stuff_obstacle = 47;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };

fn f32i(v: i32) f32 {
    return @floatFromInt(v);
}

fn rgb(c: [3]u8) rl.Color {
    return .{ .r = c[0], .g = c[1], .b = c[2], .a = 255 };
}

/// CalcColor: yellow to red below the maximum, green above it.
fn calcColor(cur_in: i32, max_in: i32) rl.Color {
    var cur = cur_in;
    var max = max_in;
    if (max == 0) {
        max = 1;
        cur += 1;
    }
    if (cur < max) return .{ .r = 220, .g = @intCast(std.math.clamp(220 - @divTrunc((max - cur) * 210, max), 0, 255)), .b = 50, .a = 255 };
    if (cur <= max + 5) return .{ .r = @intCast(std.math.clamp(220 - (cur - max) * 40, 0, 255)), .g = 220, .b = 50, .a = 255 };
    return .{ .r = 230, .g = 230, .b = 230, .a = 255 };
}

/// G_Cloud of G_Death.
const Cloud = struct {
    x: f32,
    y: f32,
    dx: f32,
    dy: f32,
    angle: i32,
    d_angle: i32,
    trans: i32 = 20,
};

/// G_Death (type 1).
const Death = struct {
    timer: i32 = 0,
    x: f32,
    y: f32,
    clouds: [12]Cloud,
};

/// What the screen shows of a unit (G_Obj).
const Vis = struct {
    x: f32 = 0,
    y: f32 = 0,
    /// The screen shows the unit alive: the logic may already have killed
    /// it while the blow or the missile is still on its way; AnimDeath
    /// clears it.
    alive: bool = false,
    /// The corpse picture replaced the unit.
    corpse: bool = false,
    /// bInvisOnDeath: nothing stays.
    hidden: bool = false,
    death: ?Death = null,
    /// The unit is moving (drawn above the others).
    moving: bool = false,
    stone: ?fx.Stone = null,
    web: u8 = 0,
    stars: ?fx.Stars = null,
    /// G_Raise of a summon: the picture grows up from the cell.
    raise: ?fx.Raise = null,

    fn clearBindings(self: *Vis) void {
        if (self.stone) |*stone| stone.deinit();
        self.stone = null;
        self.web = 0;
    }

    fn clearRaise(self: *Vis) void {
        if (self.raise) |*raise| raise.deinit();
        self.raise = null;
    }
};

/// A blocking animation.
const Act = union(enum) {
    walk: struct { who: bt.Ref, pts: [64]rl.Vector2, n: usize, i: usize, step: f32 },
    lunge: struct { striker: bt.Ref, target: bt.Ref, from: rl.Vector2, to: rl.Vector2, back: bool, hit: i32, life: i32, trample: bool, notes: [bt.max_notes]bt.Note = @splat(.{}), n_notes: u8 = 0 },
    missile: struct { pos: rl.Vector2, to: rl.Vector2, speed: f32, angle: f32, rotate: f32, tex: ?rl.Texture2D, target: bt.Ref, hit: i32, life: i32, sound_hit: i32, notes: [bt.max_notes]bt.Note = @splat(.{}), n_notes: u8 = 0 },
};

/// A note of a blow waiting for its iDelay (clEffQue).
const Later = struct { note: bt.Note, left: i32 };

/// A rising number or text (AnimNumber, AnimText).
const Float = struct {
    buf: [48]u8 = undefined,
    len: usize = 0,
    x: f32,
    y: f32,
    left: i32 = 30,
    color: rl.Color,
};

pub const Phase = enum { placing, fighting, ending, result };

/// Where the battle takes place on the map and whose it is.
pub const Where = struct {
    shard: *mapgen.Shard,
    /// shard.heroes index of the attacker.
    hero: usize,
    /// The province attacked (its guard is the defender).
    target: i32,
    /// Set when the battle screen closes.
    done: *bool,
    /// InitBattle's result, for dcHero::Death on the map.
    result: ?*i32 = null,
    /// The site slot when the defender is the site's guard (no conquest).
    site: ?usize = null,
    /// The guard of the hero's own province (task 5, no conquest).
    own: bool = false,
};

pub const BattleScene = struct {
    b: bt.Battle,
    where: Where,
    bf: gfx.Sheet,
    units: gfx.Sheet,
    shadow: gfx.Sheet,
    shadowf: gfx.Sheet,
    anim: gfx.Sheet,
    buttons: gfx.Sheet,
    iface: gfx.Sheet,
    icons: gfx.Sheet,
    items: gfx.Sheet,
    spells: gfx.Sheet,
    stuff: gfx.Sheet,
    skills: gfx.Sheet,
    abilities: gfx.Sheet,
    medals: gfx.Sheet,
    bg: rl.Texture2D,
    mask: ?rl.Image,
    vis: [2][bt.n_slots]Vis = @splat(@splat(.{})),
    act: ?Act = null,
    next_anim: usize = 0,
    floats: std.ArrayList(Float) = .empty,
    /// Notes whose delay has not run out yet.
    later: std.ArrayList(Later) = .empty,
    tick_acc: f32 = 0,
    /// Cosmetic randomness must not change battle rolls.
    fx_random: std.Random.DefaultPrng = std.Random.DefaultPrng.init(0),
    phase: Phase = .placing,
    human: [2]bool,
    /// Placement: the unit picked to move.
    picked: ?u8 = null,
    rest: Button,
    next: Button,
    end: Button,
    start: Button,
    exit: Button,
    spell: Button,
    cancel_spell: Button,
    close_book: Button,
    menu: Button,
    /// bAutoCombat: the computer plays the human's side too.
    auto: bool = false,
    /// !bShowBattle: the battle runs to its end without animation.
    fast: bool = false,
    /// Esc during the auto battle: bAutoCombat goes off at the next turn
    /// of a human's side.
    stop_auto: bool = false,
    /// WarMenu is open; its answer.
    menu_open: bool = false,
    menu_out: war_menu.Out = .{},
    /// Ctrl+F10 / Ctrl+F11: dialog 1460 is open, then this order runs.
    magic_for: ?war_menu.Result = null,
    magic_answer: ?usize = null,
    /// Spellbook() is open.
    book_open: bool = false,
    book_hover: ?usize = null,
    /// ActiveSpell: the spell waiting for its target.
    pending: ?bs.Pending = null,
    /// objTarget / objTargetH of the spell mode: 1 area, 2 free cell.
    spell_marks: [8][8]u2 = @splat(@splat(0)),
    hint: Hint = .{},
    hover: ?[2]i32 = null,
    strike_cell: ?[2]i32 = null,
    outcome: bt.Outcome = .{ .result = 0 },
    /// Delay of the end (Delay=500 ticks at most for the animations).
    end_wait: i32 = 0,
    /// The next `iTurn / 2` at which the original shows message 25.
    warning_turn: i32 = 0,
    warning_visible: bool = false,
    warning_shape: ?rl.Image = null,
    warning_mask: Mask = .{ .x = time_warning_x, .y = time_warning_y },
    warning_text: [1024]u8 = undefined,
    warning_text_len: usize = 0,
    timeout_dialog_shown: bool = false,
    dialog_result: ?usize = null,
    old_cursor: u32,

    pub fn create(c: *Ctx, w: Where, auto: bool) !scene.Scene {
        const s = w.shard;
        const h = &s.heroes.items[w.hero];
        const p1 = &s.provinces[@intCast(h.province)];
        const p2 = &s.provinces[@intCast(w.target)];
        const gidx: i32 = if (w.site) |slot| p2.site_guards[slot] else p2.guard;
        if (gidx < 0) return error.NoGuard;
        const gd = &s.guards.items[@intCast(gidx)];
        const pr = c.proto;
        const pl = std.math.clamp(h.owner, 0, mapgen.max_players);
        const ruler = s.rulers[@intCast(pl)];
        const human0 = ruler.kind == .human;
        const diff: usize = @intCast(std.math.clamp(ruler.difficulty, 0, proto.max_difficulty - 1));
        const armies: [2]bt.Army = .{
            .{ .hero = h, .initiative = h.heroBonus(pr, hero_mod.bonus.initiative), .player = h.owner, .gem = ruler.gem, .human = human0, .player_karma = if (h.owner > 0) ruler.karma else null, .squad = h.squad },
            // LifeMod of the neutrals against a human: the difficulty's iNeutralLife.
            .{ .initiative = gd.initiative, .guard = gd.guard, .guard_karma = gd.karma, .life_mod = if (human0) pr.difficulties[diff].neutral_life else 100, .squad = gd.squad },
        };
        const field = bt.generate(pr, .{
            .seed = seedOf(s),
            .prov1 = .{ .number = p1.number, .terrain = p1.terrain },
            .prov2 = if (w.site != null or w.own) null else .{ .number = p2.number, .terrain = p2.terrain },
            .site = if (w.site) |slot| @intCast(slot) else 0,
        });

        const self = try c.gpa.create(BattleScene);
        errdefer c.gpa.destroy(self);
        var bf = try gfx.Sheet.load(c.fs, "battlefield");
        errdefer bf.deinit();
        var units = try gfx.Sheet.load(c.fs, "units");
        errdefer units.deinit();
        var shadow = try gfx.Sheet.load(c.fs, "unit_shadow");
        errdefer shadow.deinit();
        var shadowf = try gfx.Sheet.load(c.fs, "unit_shadowf");
        errdefer shadowf.deinit();
        var anim = try gfx.Sheet.load(c.fs, "anim");
        errdefer anim.deinit();
        var buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer buttons.deinit();
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var icons = try gfx.Sheet.load(c.fs, "unit_icons");
        errdefer icons.deinit();
        var items = try gfx.Sheet.load(c.fs, "items");
        errdefer items.deinit();
        var spells = try gfx.Sheet.load(c.fs, "spells");
        errdefer spells.deinit();
        var stuff = try gfx.Sheet.load(c.fs, "stuff");
        errdefer stuff.deinit();
        var skills = try gfx.Sheet.load(c.fs, "skills");
        errdefer skills.deinit();
        var abilities = try gfx.Sheet.load(c.fs, "ability");
        errdefer abilities.deinit();
        var medals = try gfx.Sheet.load(c.fs, "medal_small");
        errdefer medals.deinit();
        const panel_x: f32 = 1024 - gfx.texW(bf.texIndex(bf_panel));
        var rest = try Button.init(&buttons, panel_x + 14, 247, butt_rest, butt_rest + 1, butt_rest + 2);
        errdefer rest.deinit();
        rest.inactive = butt_rest + 3;
        rest.hotkey = .r;
        rest.hint = hint_rest;
        var next = try Button.init(&buttons, panel_x + 14 + 62, 247, butt_next, butt_next + 1, butt_next + 2);
        errdefer next.deinit();
        next.inactive = butt_next + 3;
        next.hotkey = .n;
        next.hint = hint_next;
        var end = try Button.init(&buttons, panel_x + 71, 700, butt_end, butt_end + 1, butt_end + 2);
        errdefer end.deinit();
        end.inactive = butt_end + 3;
        end.hotkey = .e;
        end.hint = hint_end;
        var start = try Button.init(&buttons, panel_x + 71, 700, butt_start, butt_start + 1, 254);
        errdefer start.deinit();
        start.hotkey = .e;
        start.hint = hint_start;
        var exit = try Button.init(&buttons, 0, 0, butt_exit, butt_exit + 1, butt_exit + 2);
        errdefer exit.deinit();
        exit.hotkey = .escape;
        var spell = try Button.init(&buttons, panel_x + 14, 247 + 56, butt_spell, butt_spell + 1, butt_spell + 2);
        errdefer spell.deinit();
        spell.inactive = butt_spell + 3;
        spell.hotkey = .s;
        spell.hint = hint_spell;
        var cancel_spell = try Button.init(&buttons, panel_x + 38, 694, butt_cancel_spell, butt_cancel_spell + 1, butt_cancel_spell + 2);
        errdefer cancel_spell.deinit();
        var close_book = try Button.init(&buttons, 262 + 431, 112 + 549, butt_close, butt_close + 1, butt_close + 2);
        errdefer close_book.deinit();
        close_book.hotkey = .escape;
        var menu = try Button.init(&buttons, panel_x + 4, 718, butt_menu, butt_menu + 1, butt_menu + 3);
        errdefer menu.deinit();
        menu.inactive = butt_menu + 2;
        menu.hotkey = .o;
        menu.hint = hint_menu;

        self.* = .{
            .b = bt.Battle.init(c.gpa, pr, &c.rand, field, armies),
            .where = w,
            .bf = bf,
            .units = units,
            .shadow = shadow,
            .shadowf = shadowf,
            .anim = anim,
            .buttons = buttons,
            .iface = iface,
            .icons = icons,
            .items = items,
            .spells = spells,
            .stuff = stuff,
            .skills = skills,
            .abilities = abilities,
            .medals = medals,
            .bg = undefined,
            .mask = bf.imageIndex(bf_mask),
            .human = .{ human0 and !auto, false },
            .rest = rest,
            .next = next,
            .end = end,
            .start = start,
            .exit = exit,
            .spell = spell,
            .cancel_spell = cancel_spell,
            .close_book = close_book,
            .menu = menu,
            .old_cursor = c.cursor,
        };
        errdefer self.b.deinit();
        errdefer if (self.mask) |m| rl.unloadImage(m);
        self.b.max_duration = c.cfg.max_battle_duration;
        self.warning_turn = self.b.max_duration - 5;
        self.b.battle_speed = c.human.get(.battle_speed);
        self.bg = try self.makeBackground(c, p1.number);
        self.warning_shape = self.iface.imageIndex(gui_hint);
        self.syncAll();
        // iAutoCombat > 0 against the neutrals: a fast battle at once.
        if (self.human[0] and c.human.get(.auto_combat) > 0) {
            self.auto = true;
            self.fast = true;
        }
        if (self.human[0] and !self.auto) {
            // InitBattle: iGameMode 4 (Prepare01), Battlefield.cpp:5591.
            c.music.setMode(c.fs, .prepare);
            self.phase = .placing;
        } else self.startFight(c);
        c.setCursor(0);
        return scene.Scene.of(BattleScene, self, false);
    }

    pub fn deinit(self: *BattleScene, c: *Ctx) void {
        for (&self.vis) |*side| for (side) |*v| {
            v.clearBindings();
            v.clearRaise();
        };
        c.setCursor(self.old_cursor);
        if (self.warning_shape) |img| rl.unloadImage(img);
        self.floats.deinit(c.gpa);
        self.later.deinit(c.gpa);
        self.menu.deinit();
        if (self.mask) |m| rl.unloadImage(m);
        rl.unloadTexture(self.bg);
        self.close_book.deinit();
        self.cancel_spell.deinit();
        self.spell.deinit();
        self.exit.deinit();
        self.start.deinit();
        self.end.deinit();
        self.next.deinit();
        self.rest.deinit();
        self.medals.deinit();
        self.abilities.deinit();
        self.skills.deinit();
        self.stuff.deinit();
        self.spells.deinit();
        self.items.deinit();
        self.icons.deinit();
        self.iface.deinit();
        self.buttons.deinit();
        self.anim.deinit();
        self.shadowf.deinit();
        self.shadow.deinit();
        self.units.deinit();
        self.bf.deinit();
        self.b.deinit();
    }

    fn seedOf(s: *const mapgen.Shard) u32 {
        return @import("../game/newsave.zig").seedOf(s);
    }

    // -- the field picture ------------------------------------------------

    /// MakeBackground without the castle: grass, the swamp and forest
    /// spots, the impassable cells and the map objects of each layer.
    fn makeBackground(self: *BattleScene, c: *Ctx, num: i32) !rl.Texture2D {
        const pr = c.proto;
        const s: u32 = seedOf(self.where.shard);
        var img = rl.genImageColor(1024, 768, .black);
        defer rl.unloadImage(img);
        var bfi: Images = .{ .sheet = &self.bf };
        defer bfi.deinit();
        var nature_sheet = try gfx.Sheet.load(c.fs, "nature");
        defer nature_sheet.deinit();
        var nat: Images = .{ .sheet = &nature_sheet };
        defer nat.deinit();
        for (0..12) |i| for (0..8) |j| {
            const k = rand.keyed3(s, 3, @intCast(i), @intCast(j), @bitCast(s));
            blit(&img, bfi.get(bf_grass + @as(usize, @min(k, 2))), @intCast(i * 100), @intCast(j * 100), 255);
        };
        const f = &self.b.field;
        const r = self.b.r;
        // The swamp texture and the forest spots.
        var y: i32 = 0;
        while (y < 768) : (y += 5) {
            var x: i32 = 0;
            while (x < field_w) : (x += 4) {
                const t = f.tileType(x, y);
                if ((t == 4 or t == 8) and r.random(100) > 96) blit(&img, bfi.get(bf_swamp + r.random(4)), x - 25, y - 25, 255);
                if (t == 2 and r.random(100) > 98) blit(&img, bfi.get(bf_forest + @as(usize, @intCast(@rem(x, 2)))), x - 25, y - 25, 255);
            }
        }
        var impass: [8][8]bool = undefined;
        for (0..8) |i| for (0..8) |j| {
            const land = f.cells[i][j].land;
            impass[i][j] = land > 4 and land < 9;
        };
        var cand: [256]usize = undefined;
        var weights: [256]i32 = undefined;
        for (0..proto.max_terr_layer) |layer_u| {
            const layer: i32 = @intCast(layer_u);
            y = 0;
            while (y < 768) : (y += 5) {
                const row = @divTrunc(y - 15, 90);
                if (@rem(y, 90) == 15 and row < 8) {
                    for (0..8) |i| {
                        if (!impass[i][@intCast(row)]) continue;
                        impass[i][@intCast(row)] = false;
                        const xx: i32 = if (@rem(@divTrunc(y, 90), 2) != 0) 52 + @as(i32, @intCast(i)) * 104 else @as(i32, @intCast(i)) * 104;
                        var pic = bf_chasm + r.random(2);
                        switch (f.cells[i][@intCast(row)].land) {
                            5 => pic = bf_chasm + r.random(2),
                            6 => pic = bf_thicket + r.random(2),
                            7 => pic = bf_mount + r.random(2),
                            8 => pic = bf_fen + r.random(2),
                            else => {},
                        }
                        if (bfi.get(pic)) |p| blit(&img, p, xx - @divTrunc(p.width - 104, 2), y - @divTrunc(p.height - 100, 2), 255);
                    }
                }
                var x: i32 = 0;
                while (x < field_w) : (x += 5) {
                    var centre = false;
                    if (@rem(y, 90) == 15 and row < 10) {
                        centre = if (@rem(@divTrunc(y, 90), 2) != 0) @rem(x + 52, 104) < 5 else @rem(x, 104) < 5;
                    }
                    var pt = f.tileType(x, y);
                    if (pt > 4) pt = 0;
                    if (pt <= 0) continue;
                    const density = pr.terr_layer[@intCast(pt)][layer_u];
                    if (density <= 0) continue;
                    if (!(@as(i32, @intCast(rand.keyed4(s, 800, x + y, layer, num, y))) < density or (centre and pt == 3))) continue;
                    var n: usize = 0;
                    for (pr.map_objects[1..], 1..) |*o, i| {
                        if (o.layer != layer or o.landmark or o.land[@intCast(pt)] <= 0) continue;
                        var ok = true;
                        for ([3][2]i32{ .{ x - @divTrunc(o.dx, 2), y - @divTrunc(o.dy, 2) }, .{ x + @divTrunc(o.dx, 2), y - @divTrunc(o.dy, 2) }, .{ x, y - @divTrunc(o.dy, 2) } }) |q| {
                            const p1 = f.tileType(q[0], q[1]);
                            if (p1 > 4 or p1 < 0 or o.land[@intCast(p1)] <= 0) ok = false;
                        }
                        if ((ok or centre) and n < cand.len) {
                            cand[n] = i;
                            weights[n] = o.land[@intCast(pt)];
                            n += 1;
                        }
                    }
                    if (n == 0) continue;
                    // Roll(false, iNum, y, x, Layer).
                    var sum: u32 = 0;
                    for (weights[0..n]) |wv| sum +%= @bitCast(wv);
                    const v = rand.keyed4(s, sum, num, y, x, layer) +% 1;
                    var acc: u32 = 0;
                    var k: usize = 0;
                    var idx: usize = 0;
                    while (acc < v and k < n) : (k += 1) {
                        acc +%= @bitCast(weights[k]);
                        idx = k;
                    }
                    while (idx + 1 < n and weights[idx] == 0) idx += 1;
                    const obj = cand[idx];
                    const o = &pr.map_objects[obj];
                    if (o.n_pics == 0) continue;
                    const vi = rand.keyed4(s, @intCast(o.n_pics), y, num, layer, x);
                    if (vi >= o.n_pics) continue;
                    const pic = nat.get(@intCast(o.pics[vi])) orelse continue;
                    const xp = x - @divTrunc(pic.width, 2);
                    const yp = y - pic.height;
                    for (pr.map_objects[1..]) |*sh| {
                        if (sh.kind - 1000 != @as(i32, @intCast(obj))) continue;
                        if (sh.n_pics == 0) continue;
                        blit(&img, nat.get(@intCast(sh.pics[0])), xp, yp, if (sh.layer == 0) 255 else 128);
                    }
                    blit(&img, pic, xp, yp, 255);
                }
            }
        }
        return rl.loadTextureFromImage(img);
    }

    fn blit(dst: *rl.Image, src: ?rl.Image, x: i32, y: i32, alpha: u8) void {
        const s = src orelse return;
        const w = f32i(s.width);
        const h = f32i(s.height);
        dst.drawImage(s, .{ .x = 0, .y = 0, .width = w, .height = h }, .{ .x = f32i(x), .y = f32i(y), .width = w, .height = h }, .{ .r = 255, .g = 255, .b = 255, .a = alpha });
    }

    // -- units on the screen --------------------------------------------

    fn picOf(self: *BattleScene, f: *const bt.Fighter) ?rl.Texture2D {
        const k = proto.UnitType.picIndex(@intCast(f.unit.type)) orelse return null;
        return self.units.texIndex(k);
    }

    fn picSize(self: *BattleScene, f: *const bt.Fighter) [2]i32 {
        const t = self.picOf(f) orelse return .{ 64, 80 };
        return .{ t.width, t.height };
    }

    /// TransX / TransY of a cell for the unit's picture.
    fn cellPos(self: *BattleScene, f: *const bt.Fighter, x: i32, y: i32) rl.Vector2 {
        const sz = self.picSize(f);
        return .{ .x = f32i(bt.transX(x, y, sz[0])), .y = f32i(bt.transY(x, y, sz[1])) };
    }

    /// BF_RefreshScreen: units not moving stand on their cells.
    fn syncAll(self: *BattleScene) void {
        for (0..2) |s| for (&self.b.units[s], 0..) |*f, i| {
            if (f.unit.type == 0) continue;
            const v = &self.vis[s][i];
            v.alive = f.life > 0;
            if (f.life > 0) {
                const p = self.cellPos(f, f.x, f.y);
                v.x = p.x;
                v.y = p.y;
                v.corpse = false;
            }
        };
    }

    fn visOf(self: *BattleScene, r: bt.Ref) *Vis {
        return &self.vis[r.side][r.slot];
    }

    fn stoneForm(self: *BattleScene, r: bt.Ref) void {
        const v = self.visOf(r);
        if (v.stone != null) return;
        const k = proto.UnitType.picIndex(@intCast(self.b.fighter(r).unit.type)) orelse return;
        v.stone = fx.Stone.init(self.units.imageIndex(k) orelse return);
    }

    /// Also covers spell 18 and removal by expiry, dispel or death. Only
    /// reconcile once the queued actions have caught up with game state.
    fn syncBindings(self: *BattleScene) void {
        for (0..2) |s| for (&self.b.units[s], 0..) |*f, i| {
            const v = &self.vis[s][i];
            var stone = false;
            var web = false;
            if (f.life > 0 and f.in_battle) for (f.effects[0..f.n_eff]) |e| {
                if (e.ability == bt.upg_icon.petrify) stone = true;
                if (e.ability == bt.upg_icon.web or e.spell == 18) web = true;
            };
            if (!stone) {
                if (v.stone) |*st| st.deinit();
                v.stone = null;
            } else self.stoneForm(.{ .side = @intCast(s), .slot = @intCast(i) });
            if (!web) v.web = 0 else if (v.web == 0) v.web = 1;
        };
    }

    fn startDeath(self: *BattleScene, r: bt.Ref) void {
        const v = self.visOf(r);
        v.clearBindings();
        const f = self.b.fighter(r);
        const sz = self.picSize(f);
        const x = v.x + f32i(@divTrunc(sz[0], 2) - 22);
        const y = v.y + f32i(@divTrunc(sz[1], 2) - 30);
        const targets = [12][2]f32{ .{ -21, -10 }, .{ 7, -14 }, .{ 37, -10 }, .{ 41, 23 }, .{ 37, 53 }, .{ 7, 57 }, .{ -21, 53 }, .{ -25, 23 }, .{ -10, -2 }, .{ 27, -2 }, .{ 27, 40 }, .{ -10, 40 } };
        var d: Death = .{ .x = x, .y = y, .clouds = undefined };
        const rr = self.b.r;
        for (&d.clouds, targets) |*cl, t| {
            const x1 = x + 11 + f32i(@intCast(rr.random(20)));
            const y1 = y + 20 + f32i(@intCast(rr.random(20)));
            cl.* = .{
                .x = x1,
                .y = y1,
                .dx = (x + t[0] - x1) / 60,
                .dy = (y + t[1] - y1) / 60,
                .angle = @intCast(rr.random(256)),
                .d_angle = @as(i32, @intCast(rr.random(5))) - 2,
            };
        }
        v.death = d;
        v.corpse = true;
        v.alive = false;
    }

    fn addFloat(self: *BattleScene, c: *Ctx, x: f32, y: f32, s: []const u8, color: rl.Color) void {
        var fl: Float = .{ .x = x + 20, .y = @max(y, 30), .color = color };
        fl.len = @min(s.len, fl.buf.len);
        @memcpy(fl.buf[0..fl.len], s[0..fl.len]);
        self.floats.append(c.gpa, fl) catch {};
    }

    fn addNumber(self: *BattleScene, c: *Ctx, x: f32, y: f32, n: i32, color: rl.Color) void {
        var buf: [16]u8 = undefined;
        self.addFloat(c, x, y, std.fmt.bufPrint(&buf, "{d}", .{n}) catch "", color);
    }

    /// One clEffQue: a number or a phrase, and its sound.
    fn showNote(self: *BattleScene, c: *Ctx, n: bt.Note) void {
        const v = self.visOf(n.who);
        if (n.phrase != 0) {
            self.addFloat(c, v.x, v.y, c.proto.phrase(n.phrase), rgb(n.color));
        } else if (n.value != 0 or n.sound_unit == 0) self.addNumber(c, v.x, v.y, n.value, rgb(n.color));
        if (n.sound_unit > 0) sound.play(.unit, n.sound_unit);
    }

    /// The notes of a blow, when it lands (Proceed of pUnitEffect).
    fn release(self: *BattleScene, c: *Ctx, notes: []const bt.Note) void {
        for (notes) |n| {
            if (n.delay == 0) self.showNote(c, n) else self.later.append(c.gpa, .{ .note = n, .left = n.delay }) catch {};
        }
    }

    /// The next step of b.anims.
    fn startAnim(self: *BattleScene, c: *Ctx, a: bt.Anim) void {
        switch (a) {
            .binding => |binding| {
                const t = &c.proto.units[@intCast(self.b.fighter(binding.shooter).unit.type)];
                if (t.sound_shoot > 0) sound.play(.unit, @intCast(t.sound_shoot));
                if (binding.stone or binding.success) self.visOf(binding.shooter).stars = .{ .timer = 20, .count = 20 };
                if (binding.success) {
                    if (t.sound_shoot_hit > 0) sound.play(.unit, @intCast(t.sound_shoot_hit));
                    const v = self.visOf(binding.target);
                    v.stars = .{ .timer = 30, .count = 50 };
                    if (binding.stone) self.stoneForm(binding.target) else if (v.web == 0) {
                        v.web = 1;
                    }
                }
            },
            .move => |m| {
                const f = self.b.fighter(m.who);
                var w: Act = .{ .walk = .{ .who = m.who, .pts = undefined, .n = m.n, .i = 1, .step = f32i(m.step) } };
                for (m.cells[0..m.n], 0..) |cell, k| w.walk.pts[k] = self.cellPos(f, cell[0], cell[1]);
                self.visOf(m.who).moving = true;
                self.act = w;
            },
            .melee => |m| {
                const from = self.visOf(m.striker);
                const to = self.visOf(m.target);
                const p0: rl.Vector2 = .{ .x = from.x, .y = from.y };
                self.act = .{ .lunge = .{
                    .striker = m.striker,
                    .target = m.target,
                    .from = if (m.trample) |cell| self.cellPos(self.b.fighter(m.striker), cell[0], cell[1]) else p0,
                    .to = .{ .x = p0.x + @trunc((to.x - p0.x) / 3), .y = p0.y + @trunc((to.y - p0.y) / 3) },
                    .back = false,
                    .hit = m.hit,
                    .life = m.life,
                    .trample = m.trample != null,
                    .notes = m.notes,
                    .n_notes = m.n_notes,
                } };
                self.visOf(m.striker).moving = true;
            },
            .shoot => |m| {
                const from = self.visOf(m.shooter);
                const to = self.visOf(m.target);
                const p0: rl.Vector2 = .{ .x = from.x + 20, .y = from.y + 30 };
                const p1: rl.Vector2 = .{ .x = to.x + 20, .y = to.y + 30 };
                // AnimShoot's angle in Allegro units (256 a turn).
                const dx = p1.x - p0.x;
                const dy = p1.y - p0.y;
                const dist = @sqrt(dx * dx + dy * dy);
                var ang: f32 = if (dist > 0) @trunc(128 * std.math.acos(@abs(dx) / dist) / 3.1415) else 0;
                if (p0.x > p1.x and p0.y > p1.y) {
                    ang += 128;
                } else if (p0.y > p1.y) {
                    ang = 256 - ang;
                } else if (p0.x > p1.x) ang = 128 - ang;
                const miss = m.missile;
                const tex = if (miss >= 0 and miss < 11) self.bf.texIndex(bf_missile + @as(usize, @intCast(miss))) else if (miss == 11) self.bf.texIndex(bf_missile_last) else null;
                const rot: f32 = switch (miss) {
                    1 => 10,
                    3 => 20,
                    4 => 4,
                    else => 0,
                };
                self.act = .{ .missile = .{ .pos = p0, .to = p1, .speed = f32i(m.speed), .angle = ang, .rotate = rot, .tex = tex, .target = m.target, .hit = m.hit, .life = m.life, .sound_hit = m.sound_hit, .notes = m.notes, .n_notes = m.n_notes } };
                if (m.sound_shoot > 0) @import("../platform/sound.zig").play(.unit, @intCast(m.sound_shoot));
            },
            .number => |n| {
                const v = self.visOf(n.who);
                self.addNumber(c, v.x, v.y, n.value, rgb(n.color));
                if (n.sound_unit > 0) sound.play(.unit, n.sound_unit);
                self.release(c, n.notes[0..n.n_notes]);
            },
            .text => |t| {
                const v = self.visOf(t.who);
                self.addFloat(c, v.x, v.y, c.proto.phrase(t.phrase), rgb(t.color));
                if (t.sound_unit > 0) sound.play(.unit, t.sound_unit);
            },
            .death => |r| self.startDeath(r),
            .cast => |cs| {
                const pr = c.proto;
                if (cs.spell <= 0 or cs.spell >= pr.spells.len) return;
                const sp = &pr.spells[@intCast(cs.spell)];
                if (sp.sound >= 0) @import("../platform/sound.zig").play(.spell, @intCast(sp.sound));
                if (!cs.name) return;
                const v = self.visOf(cs.who);
                self.addFloat(c, v.x, v.y, sp.name, .{ .r = 200, .g = 60, .b = 250, .a = 255 });
            },
            .appear => |shown| {
                const f = self.b.fighter(shown.who);
                const v = self.visOf(shown.who);
                const p = self.cellPos(f, f.x, f.y);
                v.clearBindings();
                v.clearRaise();
                v.* = .{ .x = p.x, .y = p.y, .alive = true };
                if (!shown.rise) return;
                // Summon: G_Raise and Make_Stars(80, 30, 10, 20, 44, 40,
                // 180, 100, 250). A missing picture still gets the stars.
                if (proto.UnitType.picIndex(@intCast(f.unit.type))) |k| {
                    if (self.units.imageIndex(k)) |img| v.raise = fx.Raise.init(img);
                }
                v.stars = .{ .timer = 80, .count = 30, .ox = 10, .oy = 20, .color = .{ 180, 100, 250 } };
            },
            .swap => |sw| {
                self.visOf(sw.to).clearBindings();
                self.visOf(sw.to).clearRaise();
                self.vis[sw.to.side][sw.to.slot] = self.vis[sw.from.side][sw.from.slot];
                self.vis[sw.from.side][sw.from.slot] = .{};
            },
            .revive => |r| {
                const v = self.visOf(r);
                v.corpse = false;
                v.hidden = false;
                v.alive = true;
            },
        }
    }

    /// The hit lands: the number and, at 0 life, the death.
    fn landHit(self: *BattleScene, c: *Ctx, striker: ?bt.Ref, target: bt.Ref, hit: i32, life: i32) void {
        const v = self.visOf(target);
        self.addNumber(c, v.x, v.y, hit, .{ .r = 220, .g = 50, .b = 5, .a = 255 });
        const snd = @import("../platform/sound.zig");
        if (striker) |s| {
            const t = &c.proto.units[@intCast(self.b.fighter(s).unit.type)];
            if (t.sound_hit > 0) snd.play(.unit, @intCast(t.sound_hit));
        }
        if (life == 0) {
            const t = &c.proto.units[@intCast(self.b.fighter(target).unit.type)];
            if (t.sound_death > 0) snd.play(.unit, @intCast(t.sound_death));
            self.startDeath(target);
        }
        if (self.act) |a| switch (a) {
            .lunge => |l| self.release(c, l.notes[0..l.n_notes]),
            .missile => |m| self.release(c, m.notes[0..m.n_notes]),
            else => {},
        };
    }

    /// Moves `p` towards `to` by `step` (|dx| + |dy| = step a tick, as
    /// G_Object::Move); true on arrival.
    fn advance(p: *rl.Vector2, to: rl.Vector2, step: f32) bool {
        const dx = to.x - p.x;
        const dy = to.y - p.y;
        const d = @abs(dx) + @abs(dy);
        if (d <= step) {
            p.* = to;
            return true;
        }
        p.x += step * dx / d;
        p.y += step * dy / d;
        return false;
    }

    /// One tick of every animation.
    fn tick(self: *BattleScene, c: *Ctx) void {
        if (self.idle()) self.syncBindings();
        for (&self.vis) |*side| for (side) |*v| {
            if (v.stone) |*stone| stone.tick();
            if (v.web > 0 and v.web < 18) v.web += 1;
            if (v.stars) |*stars| {
                if (stars.tick(self.fx_random.random())) v.stars = null;
            }
            if (v.raise) |*raise| if (raise.tick()) v.clearRaise();
        };
        var k: usize = 0;
        while (k < self.later.items.len) {
            const p = &self.later.items[k];
            p.left -= 1;
            if (p.left <= 0) {
                self.showNote(c, p.note);
                _ = self.later.swapRemove(k);
                continue;
            }
            k += 1;
        }
        var i: usize = 0;
        while (i < self.floats.items.len) {
            const fl = &self.floats.items[i];
            fl.y -= 1;
            fl.left -= 1;
            if (fl.left <= 0) {
                _ = self.floats.swapRemove(i);
                continue;
            }
            i += 1;
        }
        for (&self.vis) |*side| for (side) |*v| {
            const d = &(v.death orelse continue);
            d.timer += 1;
            for (&d.clouds) |*cl| {
                cl.angle += cl.d_angle;
                cl.x += cl.dx;
                cl.y += cl.dy;
                if (d.timer < 50) cl.trans += 5 else cl.trans -= 10;
                cl.trans = std.math.clamp(cl.trans, 0, 200);
            }
            if (d.timer >= 70) v.death = null;
        };
        if (self.act) |*a| switch (a.*) {
            .walk => |*w| {
                const v = self.visOf(w.who);
                var p: rl.Vector2 = .{ .x = v.x, .y = v.y };
                if (advance(&p, w.pts[w.i], w.step)) w.i += 1;
                v.x = p.x;
                v.y = p.y;
                if (w.i >= w.n) {
                    v.moving = false;
                    self.act = null;
                }
            },
            .lunge => |*l| {
                const v = self.visOf(l.striker);
                var p: rl.Vector2 = .{ .x = v.x, .y = v.y };
                if (!l.back) {
                    if (advance(&p, l.to, 5)) {
                        l.back = true;
                        self.landHit(c, l.striker, l.target, l.hit, l.life);
                        if (l.trample) sound.play(.unit, 135); // wavTrample
                    }
                } else if (advance(&p, l.from, 5)) {
                    v.moving = false;
                    self.act = null;
                }
                v.x = p.x;
                v.y = p.y;
            },
            .missile => |*m| {
                m.angle += m.rotate;
                if (advance(&m.pos, m.to, m.speed)) {
                    if (m.sound_hit > 0) @import("../platform/sound.zig").play(.unit, @intCast(m.sound_hit));
                    self.landHit(c, null, m.target, m.hit, m.life);
                    self.act = null;
                }
            },
        };
        if (self.act == null) {
            if (self.next_anim < self.b.anims.items.len) {
                const a = self.b.anims.items[self.next_anim];
                self.next_anim += 1;
                self.startAnim(c, a);
            } else if (self.next_anim > 0) {
                self.b.anims.clearRetainingCapacity();
                self.next_anim = 0;
                self.syncAll();
                self.syncBindings();
            }
        }
    }

    /// Nothing is moving and no step waits.
    fn idle(self: *const BattleScene) bool {
        return self.act == null and self.next_anim >= self.b.anims.items.len;
    }

    fn deathsDone(self: *const BattleScene) bool {
        for (self.vis) |side| for (side) |v| if (v.death != null or v.stars != null) return false;
        return self.floats.items.len == 0;
    }

    // -- the battle flow --------------------------------------------------

    fn startFight(self: *BattleScene, c: *Ctx) void {
        self.phase = .fighting;
        // iGameMode 3: a random War track (Battlefield.cpp:5939).
        c.music.setMode(c.fs, .war);
        self.b.side = 1 - self.b.firstSide();
        self.b.endTurn();
        self.afterTurn();
    }

    /// After a side's turn began: select its first unit when a human
    /// plays it.
    fn afterTurn(self: *BattleScene) void {
        if (self.b.over()) return;
        if (self.stop_auto and self.human[self.b.side]) {
            self.stop_auto = false;
            self.auto = false;
        }
        if (self.playerTurn()) self.selectNext();
    }

    /// The human plays the side whose turn it is (IsPlayed[AA] and not
    /// bAutoCombat).
    fn playerTurn(self: *const BattleScene) bool {
        return self.human[self.b.side] and !self.auto;
    }

    fn selectNext(self: *BattleScene) void {
        if (self.b.nextUnit()) |r| return self.b.select(r);
        if (self.b.active) |a| if (self.b.fighter(a).life > 0) return self.b.select(a);
        if (self.b.firstAlive(self.b.side)) |k| self.b.select(.{ .side = self.b.side, .slot = k });
    }

    fn endSideTurn(self: *BattleScene, c: *Ctx) void {
        self.warning_visible = false;
        self.b.endTurn();
        self.checkTimeWarning(c);
        self.afterTurn();
    }

    /// TimeWarning in InitBattle's loop: once at 5, 3 and 1 turns left.
    fn checkTimeWarning(self: *BattleScene, c: *Ctx) void {
        if (self.fast or self.warning_visible or self.warning_turn < 0) return;
        const turn = @divTrunc(self.b.turn, 2);
        if (turn != self.warning_turn) return;
        self.warning_turn += 2;
        const message = if (msg_time_warning < c.proto.messages.len) c.proto.messages[msg_time_warning] else "";
        self.warning_text_len = castle_ui.formatInt(&self.warning_text, message, self.b.max_duration - turn).len;
        self.warning_mask.state = .visible;
        self.warning_visible = true;
        sound.play(.unit, 156);
    }

    fn beginEnding(self: *BattleScene, c: *Ctx) scene.Action {
        self.phase = .ending;
        self.warning_visible = false;
        self.end_wait = 500;
        self.book_open = false;
        self.pending = null;
        c.setCursor(0);
        // Battlefield.cpp shows this only for a visible battle.  Fast combat
        // still ends in a draw, but does not interrupt the result transition.
        if (!self.fast and @divTrunc(self.b.turn, 2) > self.b.max_duration and !self.timeout_dialog_shown) {
            self.timeout_dialog_shown = true;
            self.dialog_result = null;
            if (dlg_time_limit < c.proto.dialogs.len) {
                return .{ .push = Dialog.create(c, dlg_time_limit, .{ true, true, false, false }, &self.dialog_result) catch |e| {
                    std.log.err("time-limit dialog: {t}", .{e});
                    return .none;
                } };
            }
        }
        return .none;
    }

    fn updateTimeWarning(self: *BattleScene, f: *const input.Frame) bool {
        if (!self.warning_visible) return false;
        self.warning_mask.shape = if (self.warning_shape) |*img| img else return false;
        const hit = self.warning_mask.check(f.pointer);
        if (hit != .none) {
            self.warning_visible = false;
            return true;
        }
        return true;
    }

    pub fn update(self: *BattleScene, c: *Ctx, f: *const input.Frame, dt: f32) scene.Action {
        self.hint.clear();
        const tick_s = f32i(c.cfg.refresh_delay) / 1000;
        self.tick_acc += dt;
        var n: usize = 0;
        while (self.tick_acc >= tick_s and n < 8) : (n += 1) {
            self.tick_acc -= tick_s;
            self.tick(c);
        }
        if (n == 8) self.tick_acc = 0;
        const p = f.pointer;
        self.hover = if (p.hover and p.pos.x < field_w) self.cellAt(p.pos) else null;
        self.strike_cell = null;
        switch (self.phase) {
            .placing => return self.updatePlacing(c, f),
            .fighting => {
                if (self.b.over()) {
                    return self.beginEnding(c);
                }
                if (self.fast) {
                    self.runFast();
                    return .none;
                }
                self.checkTimeWarning(c);
                const warning_hot = self.updateTimeWarning(f);
                if (warning_hot) {
                    c.setCursor(0);
                    return .none;
                }
                if (self.auto and self.human[0] and f.key(.escape)) self.stop_auto = true;
                if (!self.idle()) return .none;
                if (!self.playerTurn()) {
                    if (!self.b.aiStep()) self.endSideTurn(c);
                    return .none;
                }
                return self.updateHuman(c, f);
            },
            .ending => {
                self.end_wait -= 1;
                if ((self.idle() and self.deathsDone()) or self.end_wait <= 0) {
                    self.outcome = self.b.finish(self.endCtx(c));
                    self.phase = .result;
                    // EndBattle: the victory/defeat jingle, then the map
                    // music (Battlefield.cpp:1598/1611).
                    c.music.jingle(c.fs, self.outcome.result == 1);
                    const win = self.iface.texIndex(if (self.outcome.result == 1) gui_victory else gui_lose);
                    const wx = 512 - gfx.texW(win) / 2;
                    const wy = 384 - gfx.texH(win) / 2;
                    self.exit.mask.x = wx + 359;
                    self.exit.mask.y = wy + 361;
                }
                return .none;
            },
            .result => {
                c.setCursor(0);
                if (self.exit.check(f)) {
                    self.apply(c);
                    return .pop;
                }
                return .none;
            },
        }
    }

    /// The hex under a point (Grid[i][j] masks).
    fn cellAt(self: *const BattleScene, pos: rl.Vector2) ?[2]i32 {
        const m = self.mask orelse return null;
        for (0..8) |i| for (0..8) |j| {
            const x: i32 = @intCast(i);
            const y: i32 = @intCast(j);
            const lx: i32 = @intFromFloat(@floor(pos.x - f32i(bt.cellX(x, y))));
            const ly: i32 = @intFromFloat(@floor(pos.y - f32i(bt.cellY(y))));
            if (lx < 0 or ly < 0 or lx >= m.width or ly >= m.height) continue;
            if (rl.getImageColor(m, lx, ly).a > 0) return .{ x, y };
        };
        return null;
    }

    /// UnitPlacement: pick a unit of the first two columns and put it on
    /// another cell there (swapping with a unit standing on it).
    fn updatePlacing(self: *BattleScene, c: *Ctx, f: *const input.Frame) scene.Action {
        if (self.updateOrders(c, f)) |a| return a;
        if (self.auto or self.start.check(f)) {
            self.endPlacing(c);
            return .none;
        }
        if (self.start.hot()) self.hint.phrase(c, self.start.hint, f.pointer.pos);
        const cell = self.hover orelse {
            c.setCursor(0);
            return .none;
        };
        const in_zone = cell[0] < 2;
        const here = self.b.unitAt(0, cell[0], cell[1]);
        c.setCursor(if (here != null and in_zone) 1 else if (self.picked != null and in_zone) 2 else 0);
        if (f.pointer.secondary.released) {
            if (self.b.unitAt(0, cell[0], cell[1]) orelse self.b.unitAt(1, cell[0], cell[1])) |r| return self.unitInfo(c, r);
            return self.terrainInfo(c, cell);
        }
        if (!f.pointer.primary.released or !in_zone) return .none;
        if (self.picked) |k| {
            const u = &self.b.units[0][k];
            if (here) |h| {
                const o = &self.b.units[0][h.slot];
                o.x = u.x;
                o.y = u.y;
            }
            self.b.field.cells[@intCast(u.x)][@intCast(u.y)].pass = here != null;
            u.x = cell[0];
            u.y = cell[1];
            self.b.field.cells[@intCast(u.x)][@intCast(u.y)].pass = false;
            self.picked = null;
            self.b.active = null;
            self.syncAll();
        } else if (here) |h| {
            self.picked = h.slot;
            self.b.active = h;
        }
        return .none;
    }

    /// The placement is over: the squad keeps the cells.
    fn endPlacing(self: *BattleScene, c: *Ctx) void {
        self.picked = null;
        self.b.active = null;
        for (self.b.units[0][0..bt.max_squad], 0..) |*u, i| {
            if (u.life > 0 and self.b.armies[0].squad[i] != null) {
                self.b.armies[0].squad[i].?.x = u.x;
                self.b.armies[0].squad[i].?.y = u.y;
            }
        }
        self.startFight(c);
    }

    /// buttMenu is active: the placement or the player's orders, nothing
    /// moving and no spell being aimed.
    fn menuReady(self: *const BattleScene) bool {
        return switch (self.phase) {
            .placing => !self.auto,
            .fighting => self.playerTurn() and self.idle() and !self.book_open and self.pending == null,
            else => false,
        };
    }

    /// The war menu, its keys and the grid key of the placement and of the
    /// player's turn (Battlefield.cpp:4184, 7346).  Non-null: a window
    /// opens.
    fn updateOrders(self: *BattleScene, c: *Ctx, f: *const input.Frame) ?scene.Action {
        const side = self.b.side;
        const me: u1 = if (self.phase == .placing) 0 else side;
        if (self.menu_open) {
            self.menu_open = false;
            if (self.menu_out.can_cast) |v| self.b.can_cast[me] = v;
            self.applyOrder(me, self.menu_out.result);
        }
        if (self.magic_for) |what| {
            self.magic_for = null;
            self.b.can_cast[me] = @max(@as(i32, @intCast(self.magic_answer orelse 0)) - 1, 0);
            self.magic_answer = null;
            self.applyOrder(me, what);
        }
        if (self.auto) return null;
        if (f.key(.g)) {
            c.human.setFlag(.war_grid, !c.human.flag(.war_grid));
            c.human.save(c.fs) catch |e| std.log.err("cannot save the profile: {t}", .{e});
        }
        const other_human = self.human[1 - me];
        const keyed: ?war_menu.Result = if (f.key(.f10) and !other_human) .fast else if (f.key(.f11)) .auto else null;
        if (keyed) |what| {
            if (f.ctrl) {
                self.magic_for = what;
                self.magic_answer = null;
                return .{ .push = Dialog.create(c, war_menu.dlg_magic, .{ true, true, true, false }, &self.magic_answer) catch {
                    self.magic_for = null;
                    return .none;
                } };
            }
            self.applyOrder(me, what);
            return null;
        }
        self.menu.on = self.menuReady();
        if (self.menu.hot()) self.hint.phrase(c, self.menu.hint, f.pointer.pos);
        if (self.menu.check(f)) {
            self.menu_open = true;
            return .{ .push = WarMenu.create(c, other_human, &self.menu_out) catch |e| {
                std.log.err("war menu: {t}", .{e});
                self.menu_open = false;
                return .none;
            } };
        }
        return null;
    }

    /// WarMenu's answer: 1 the fast battle, 2 the auto battle (or the
    /// other human's side plays alone), 3 the side's units all die.
    fn applyOrder(self: *BattleScene, me: u1, what: war_menu.Result) void {
        switch (what) {
            .none => {},
            .fast => {
                self.auto = true;
                self.fast = true;
            },
            .auto => if (self.human[1 - me]) {
                self.human[me] = false;
            } else {
                self.auto = true;
            },
            .surrender => {
                for (&self.b.units[me], &self.vis[me]) |*u, *v| {
                    if (u.unit.type != 0 and u.life > 0 and u.in_battle) v.corpse = true;
                    v.alive = false;
                    u.life = 0;
                }
                self.auto = true;
            },
        }
        if (what != .none and self.phase == .fighting) {
            self.book_open = false;
            self.b.active = null;
        }
    }

    /// !bShowBattle: both sides fight to the end without animation; the
    /// field shows the survivors and the corpses afterwards.
    fn runFast(self: *BattleScene) void {
        self.act = null;
        self.floats.clearRetainingCapacity();
        self.later.clearRetainingCapacity();
        var n: usize = 0;
        while (!self.b.over() and n < 1_000_000) : (n += 1) {
            if (!self.b.aiStep()) self.b.endTurn();
            self.b.anims.clearRetainingCapacity();
        }
        self.b.anims.clearRetainingCapacity();
        self.next_anim = 0;
        self.b.active = null;
        for (0..2) |s| for (&self.b.units[s], &self.vis[s]) |*u, *v| {
            v.death = null;
            v.moving = false;
            v.clearRaise();
            if (u.unit.type != 0 and u.life <= 0 and u.in_battle) v.corpse = true;
        };
        self.syncAll();
    }

    /// The info click on a unit: ShowInfoScreen.
    fn unitInfo(self: *BattleScene, c: *Ctx, r: bt.Ref) scene.Action {
        const s = self.where.shard;
        const f = self.b.fighter(r);
        var pct: i32 = 100;
        if (self.b.armies[f.home].hero) |h| {
            const pl = std.math.clamp(h.owner, 0, mapgen.max_players);
            const ruler = s.rulers[@intCast(pl)];
            const diff: usize = @intCast(std.math.clamp(ruler.difficulty, 0, proto.max_difficulty - 1));
            pct = if (ruler.kind == .human) c.proto.difficulties[diff].exp else 100 - ruler.difficulty * 5;
        }
        return .{ .push = UnitInfo.create(c, &self.b, r, pct) catch return .none };
    }

    /// The info click on an empty cell (Battlefield.cpp:6921): plain,
    /// forest, hill and swamp have their window (named by the terrain of
    /// the same index, as the original does), the rest the obstacle one.
    fn terrainInfo(self: *BattleScene, c: *Ctx, cell: [2]i32) scene.Action {
        const pr = c.proto;
        const land = self.b.grid(cell[0], cell[1]).land;
        const msg = struct {
            fn m(p: *const proto.Proto, i: usize) []const u8 {
                return if (i < p.messages.len) p.messages[i] else "";
            }
        }.m;
        const w = switch (land) {
            1...4 => WindowInfo.create(c, if (land < pr.terrains.len) pr.terrains[@intCast(land)].name else "", msg(pr, msg_terrain + @as(usize, @intCast(land))), stuff_terrain + @as(usize, @intCast(land))),
            bt.land_tower => WindowInfo.create(c, pr.phrase(436), msg(pr, 43), 48),
            else => WindowInfo.create(c, pr.phrase(ph_obstacle), msg(pr, msg_obstacle), stuff_obstacle),
        };
        return .{ .push = w catch return .none };
    }

    /// CalcDistance: the pointer to the centre of a cell.
    fn pointerDistance(p: rl.Vector2, x: i32, y: i32) f32 {
        const cx = f32i(52 * @rem(y, 2) + 52 + x * 104);
        const cy = f32i(y * 90 + 45);
        return (cx - p.x) * (cx - p.x) + (cy - p.y) * (cy - p.y);
    }

    /// The cell next to the target the active unit strikes from: the one
    /// closest to the pointer, in the original's order.
    fn strikeFrom(self: *BattleScene, p: rl.Vector2, tx: i32, ty: i32) ?[2]i32 {
        const order = [6][2]i32{ .{ tx - 1, ty }, .{ tx - 1 + @rem(ty, 2), ty - 1 }, .{ tx + @rem(ty, 2), ty - 1 }, .{ tx + 1, ty }, .{ tx + @rem(ty, 2), ty + 1 }, .{ tx - 1 + @rem(ty, 2), ty + 1 } };
        var best: ?[2]i32 = null;
        var bd: f32 = 1000000;
        var k: usize = 0;
        for (order) |o| {
            defer k += 1;
            if (!self.b.canAttackFrom(o[0], o[1])) continue;
            const d = pointerDistance(p, o[0], o[1]);
            if (k == 0 or d < bd) {
                if (best == null or d < bd) {
                    bd = d;
                    best = o;
                }
            }
        }
        return best;
    }

    fn updateHuman(self: *BattleScene, c: *Ctx, f: *const input.Frame) scene.Action {
        const ar = self.b.active orelse {
            self.selectNext();
            return .none;
        };
        const a = self.b.fighter(ar);
        if (self.book_open) return self.updateBook(c, f, ar);
        if (self.pending != null) return self.updateSpell(c, f, ar);
        if (self.updateOrders(c, f)) |act| return act;
        if (!self.playerTurn()) return .none;
        self.spell.enabled = bs.hasBook(&self.b, ar);
        self.spell.on = bs.canOpenBook(&self.b, ar);
        if (self.spell.hot()) self.hint.phrase(c, self.spell.hint, f.pointer.pos);
        if (self.spell.check(f)) {
            self.book_open = true;
            self.book_hover = null;
            return .none;
        }
        self.rest.on = self.b.canRest(a);
        self.next.on = self.b.nextUnit() != null;
        for ([_]*Button{ &self.rest, &self.next, &self.end }) |btn| if (btn.hot()) self.hint.phrase(c, btn.hint, f.pointer.pos);
        if (self.rest.check(f)) {
            self.b.rest(ar);
            self.selectNext();
            return .none;
        }
        if (self.next.check(f) or f.key(.space)) {
            if (self.b.nextUnit()) |r| self.b.select(r);
            return .none;
        }
        if (self.end.check(f)) {
            self.endSideTurn(c);
            return .none;
        }
        // bWarEndTurn: the turn ends when no unit can act.
        const any = self.b.nextUnit() != null or a.can_act;
        if (!any and c.human.flag(.war_end_turn)) {
            self.endSideTurn(c);
            return .none;
        }

        const p = f.pointer;
        const cell = self.hover orelse {
            c.setCursor(0);
            return .none;
        };
        const enemy = self.b.unitAt(1 - self.b.side, cell[0], cell[1]);
        const own = self.b.unitAt(self.b.side, cell[0], cell[1]);
        const ctrl = rl.isKeyDown(.left_control) or rl.isKeyDown(.right_control);
        var cursor: u32 = 0;
        if (enemy) |e| {
            const ef = self.b.fighter(e);
            if (a.can_act) {
                const ranged = a.ammo != 0 and !ctrl and self.b.map[@intCast(ef.x)][@intCast(ef.y)].shoot != 1 and self.b.rangedAttack(a) > 0;
                var melee = !ranged;
                if (ranged) {
                    if (self.b.canShoot(ar, ef.x, ef.y)) cursor = 4 else {
                        cursor = 5;
                        melee = true;
                    }
                }
                if (melee) {
                    if (self.strikeFrom(p.pos, cell[0], cell[1])) |from| {
                        if (self.b.skill(a, bt.ab.no_combat) == 0) {
                            cursor = 6 + @as(u32, @intCast(self.dirIndex(cell, from)));
                            self.strike_cell = from;
                        } else cursor = 12;
                    } else if (cursor != 5) cursor = 12;
                }
            }
        } else if (own != null) {
            cursor = 1;
        } else if (a.can_act) {
            cursor = if (self.b.map[@intCast(cell[0])][@intCast(cell[1])].pass) 2 else 3;
        }
        c.setCursor(cursor);

        if (p.secondary.released) {
            if (own orelse enemy) |r| return self.unitInfo(c, r);
            return self.terrainInfo(c, cell);
        }
        if (!p.primary.released) return .none;
        if (enemy) |e| {
            if (!a.can_act) return .none;
            const ef = self.b.fighter(e);
            if (self.b.canShoot(ar, ef.x, ef.y) and !ctrl) {
                self.b.shoot(ar, e);
                self.selectAfter();
            } else if (self.b.skill(a, bt.ab.no_combat) == 0) {
                if (self.strikeFrom(p.pos, cell[0], cell[1])) |from| {
                    _ = self.b.attackFrom(ar, from[0], from[1], e);
                    self.selectAfter();
                }
            }
            return .none;
        }
        if (own) |o| {
            self.b.select(o);
            return .none;
        }
        if (a.can_act and self.b.map[@intCast(cell[0])][@intCast(cell[1])].pass) {
            self.b.moveTo(ar, cell[0], cell[1]);
            if (self.b.nextUnit() != null and !a.can_act) self.selectNext() else self.b.select(ar);
        }
        return .none;
    }

    /// The sword cursor of a direction: Choose 6..11 for the strike cell
    /// left, up-left, up-right, right, down-right, down-left of the target.
    fn dirIndex(_: *const BattleScene, t: [2]i32, from: [2]i32) usize {
        const p = @rem(t[1], 2);
        const order = [6][2]i32{ .{ t[0] - 1, t[1] }, .{ t[0] - 1 + p, t[1] - 1 }, .{ t[0] + p, t[1] - 1 }, .{ t[0] + 1, t[1] }, .{ t[0] + p, t[1] + 1 }, .{ t[0] - 1 + p, t[1] + 1 } };
        for (order, 0..) |o, k| if (o[0] == from[0] and o[1] == from[1]) return k;
        return 0;
    }

    /// Spellbook(): a click on a castable spell takes it; a spell without
    /// a target is cast at once.
    fn updateBook(self: *BattleScene, c: *Ctx, f: *const input.Frame, ar: bt.Ref) scene.Action {
        if (self.close_book.check(f)) {
            self.book_open = false;
            c.setCursor(0);
            return .none;
        }
        const entries = bs.book(&self.b, ar);
        const mask = self.iface.texIndex(gui_spell_mask);
        const mw = gfx.texW(mask);
        const mh = gfx.texH(mask);
        self.book_hover = null;
        const p = f.pointer.pos;
        for (entries, 0..) |e, i| {
            if (e.spell <= 0) continue;
            const x = 296 + f32i(@intCast((i % 3) * 64));
            const y = 140 + f32i(@intCast((i / 3) * 66));
            if (p.x >= x and p.y >= y and p.x < x + mw and p.y < y + mh) self.book_hover = i;
        }
        c.setCursor(if (self.book_hover != null) 1 else 0);
        if (!f.pointer.primary.released) return .none;
        const i = self.book_hover orelse return .none;
        var pd = bs.pick(&self.b, ar, i) orelse return .none;
        self.book_open = false;
        c.setCursor(0);
        if (bs.prepare(&self.b, ar, &pd)) {
            self.selectAfterCast(ar);
        } else self.pending = pd;
        return .none;
    }

    /// The main loop while ActiveSpell >= 0: the target cursor, the marks
    /// and the click on the target.
    fn updateSpell(self: *BattleScene, c: *Ctx, f: *const input.Frame, ar: bt.Ref) scene.Action {
        const pd = &self.pending.?;
        if (self.cancel_spell.check(f)) {
            self.pending = null;
            self.spell_marks = @splat(@splat(0));
            self.b.select(ar);
            return .none;
        }
        const sp = bs.spellOf(&self.b, pd.spell);
        self.spell_marks = @splat(@splat(0));
        if (!pd.sacrifice and (sp.target == 5 or pd.teleport != null)) {
            for (0..8) |i| for (0..8) |j| {
                if (bs.freeCell(&self.b, @intCast(i), @intCast(j))) self.spell_marks[i][j] = 2;
            };
        }
        const cell = self.hover orelse {
            c.setCursor(0);
            return .none;
        };
        if (!pd.sacrifice and pd.teleport == null and sp.target == 3 and sp.radius > 0) {
            self.b.clearMap();
            self.b.shootMap(cell[0], cell[1], -1, sp.radius);
            for (0..8) |i| for (0..8) |j| {
                if (self.b.map[i][j].shoot > 0) self.spell_marks[i][j] = 1;
            };
        }
        const can = bs.click(&self.b, ar, pd, cell[0], cell[1], true) != .none;
        c.setCursor(if (can) cur_magic else cur_no_magic);
        if (!f.pointer.primary.released or !can) return .none;
        if (bs.click(&self.b, ar, pd, cell[0], cell[1], false) == .cast) {
            self.pending = null;
            self.spell_marks = @splat(@splat(0));
            self.selectAfterCast(ar);
        }
        return .none;
    }

    /// The end of SpellCast: an archmage acts again, a raised or summoned
    /// unit is selected, else the next one.
    fn selectAfterCast(self: *BattleScene, ar: bt.Ref) void {
        const a = self.b.fighter(ar);
        if (a.can_act and a.life > 0) return self.b.select(ar);
        if (self.b.gomunculus) |g| {
            self.b.gomunculus = null;
            if (self.b.fighter(g).life > 0) return self.b.select(g);
        }
        self.selectAfter();
    }

    fn selectAfter(self: *BattleScene) void {
        if (self.b.nextUnit()) |r| return self.b.select(r);
        if (self.b.active) |a| if (self.b.fighter(a).life > 0) return self.b.select(a);
        if (self.b.firstAlive(self.b.side)) |k| self.b.select(.{ .side = self.b.side, .slot = k });
    }

    /// What EndBattle reads from the map for the attacking hero.
    fn endCtx(self: *BattleScene, c: *Ctx) bt.EndCtx {
        const s = self.where.shard;
        const h = &s.heroes.items[self.where.hero];
        const pl = std.math.clamp(h.owner, 0, mapgen.max_players);
        const ruler = s.rulers[@intCast(pl)];
        const diff: usize = @intCast(std.math.clamp(ruler.difficulty, 0, proto.max_difficulty - 1));
        const eco: @import("../game/economy.zig").Economy = .{ .s = s, .pr = c.proto };
        return .{
            .provinces = s.provinces,
            .loot_pct = if (ruler.kind == .human) c.proto.difficulties[diff].loot else null,
            .trophy_bonus = eco.param1(h.owner, 41),
            // Site 0: a province guard.
            .static_loot_seed = if (c.cfg.static_loot) @bitCast(h.province +% @as(i32, @bitCast(seedOf(s) / 9))) else null,
        };
    }

    /// EndBattle's results on the map: the squads, the trophies, the
    /// experience and, on a victory, the province.
    fn apply(self: *BattleScene, c: *Ctx) void {
        const s = self.where.shard;
        const h = &s.heroes.items[self.where.hero];
        const pv = &s.provinces[@intCast(self.where.target)];
        const gidx: i32 = if (self.where.site) |slot| pv.site_guards[slot] else pv.guard;
        if (gidx < 0) {
            self.where.done.* = true;
            return;
        }
        const gd = &s.guards.items[@intCast(gidx)];
        const res = self.outcome.result;
        h.squad = self.b.armies[0].squad;
        gd.squad = self.b.armies[1].squad;
        // The crystals and the karma of the hero's spells.
        const owner: usize = @intCast(std.math.clamp(h.owner, 0, mapgen.max_players));
        s.rulers[owner].gem -= self.b.gem_spent[0];
        s.rulers[owner].karma += self.b.karma[0];
        // The dead of the hero's squad leave it whatever the outcome.
        for (1..bt.max_squad) |i| {
            if (h.squad[i]) |u| if (u.life == 0) {
                h.squad[i] = null;
            };
        }
        if (res == 1 or res == 0) {
            const win_hero = res == 1;
            for (0..bt.max_squad) |i| {
                if (gd.squad[i]) |u| if (u.life == 0 or !win_hero) {
                    if (u.life == 0) gd.squad[i] = null;
                };
            }
        }
        if (res == 1) {
            const pl: usize = @intCast(std.math.clamp(h.owner, 0, mapgen.max_players));
            h.exp += self.outcome.exp[0];
            s.rulers[pl].gold += self.outcome.gold;
            s.rulers[pl].gem += self.outcome.gem;
            s.rulers[pl].karma += self.outcome.karma;
            if (self.where.site == null and !self.where.own) {
                // Conquer: a neutral province.
                if (pv.type >= 0 and pv.type < c.proto.province_types.len) s.rulers[pl].karma += c.proto.province_types[@intCast(pv.type)].conq_karma;
                pv.player = h.owner;
                pv.guard = -1;
                pv.defender = 0;
                h.province = pv.number;
            }
        }
        h.event = 0;
        h.target = 0;
        // A fallen hero (his unit's life 0): the map runs dcHero::Death.
        if (self.where.result) |r| r.* = res;
        self.where.done.* = true;
        // EndBattle: back to the map (iGameMode 2, Battlefield.cpp:7560).
        c.music.setMode(c.fs, .map);
    }

    // -- drawing --------------------------------------------------------

    pub fn draw(self: *BattleScene, c: *Ctx) void {
        rl.drawTextureV(self.bg, .{ .x = 0, .y = 0 }, .white);
        const grid_on = c.human.flag(.war_grid);
        if (grid_on) for (0..8) |i| for (0..8) |j| {
            const x: i32 = @intCast(i);
            const y: i32 = @intCast(j);
            const dim = self.phase == .placing and x > 1;
            gfx.drawTex2(self.bf.texIndex(bf_grid), f32i(bt.cellX(x, y)), f32i(bt.cellY(y)), .{ .r = 255, .g = 255, .b = 255, .a = if (dim) 70 else 255 });
        };
        if (self.hover) |h| if (self.phase != .result) gfx.drawTex(self.bf.texIndex(bf_hexx), f32i(bt.cellX(h[0], h[1])), f32i(bt.cellY(h[1])));
        if (self.strike_cell) |h| gfx.drawTex(self.bf.texIndex(bf_hexx), f32i(bt.cellX(h[0], h[1])), f32i(bt.cellY(h[1])));
        self.drawTargets();
        self.drawUnits(c);
        self.drawMissile();
        for (self.floats.items) |fl| {
            const s = fl.buf[0..fl.len];
            c.font1.draw(s, fl.x + 1, fl.y + 1, color1);
            c.font1.draw(s, fl.x, fl.y, fl.color);
        }
        self.drawPanel(c);
        if (self.sideBarUnit()) |r| self.drawSideBar(c, self.b.fighter(r));
        // objLowString while the computer plays the human's side.
        if (self.auto and !self.fast and self.human[0] and self.phase == .fighting) {
            if (self.iface.texIndex(gui_str_hint)) |t| {
                const x = 512 - f32i(t.width) / 2;
                const y = 768 - f32i(t.height) - 3;
                gfx.drawTex(t, x, y);
                c.font1.drawCentered(c.proto.phrase(ph_auto_combat), 512, y + 8, color1);
            }
        }
        if (self.book_open) self.drawBook(c);
        if (self.phase == .result) self.drawResult(c);
        self.drawTimeWarning(c);
        self.hint.draw(c, &self.iface);
    }

    fn drawTimeWarning(self: *BattleScene, c: *Ctx) void {
        if (!self.warning_visible) return;
        const hint = self.iface.texIndex(gui_hint) orelse return;
        gfx.drawTex(hint, time_warning_x, time_warning_y);
        const body = self.warning_text[0..self.warning_text_len];
        var box = text.Box.init(time_warning_x + 20, time_warning_y + 30, 297, 200, &c.font2, color1);
        _ = box.draw(body);
    }

    /// objTarget: the enemies the active unit can shoot.
    fn drawTargets(self: *BattleScene) void {
        if (self.phase != .fighting or !self.idle()) return;
        const ar = self.b.active orelse return;
        if (!self.human[ar.side] or self.auto) return;
        if (self.pending != null) {
            for (0..8) |i| for (0..8) |j| {
                const m = self.spell_marks[i][j];
                if (m == 0) continue;
                const x: i32 = @intCast(i);
                const y: i32 = @intCast(j);
                const xx: i32 = (if (@rem(y, 2) != 0) @as(i32, 70) else 18) + x * 104 - 5;
                gfx.drawTex(self.bf.texIndex(if (m == 1) bf_target else bf_target_heal), f32i(xx), f32i(15 + y * 90 + 60));
            };
            return;
        }
        if (self.book_open) return;
        for (&self.b.units[1 - ar.side]) |*e| {
            if (e.life <= 0 or !e.in_battle) continue;
            if (!self.b.canShoot(ar, e.x, e.y)) continue;
            const xx: i32 = (if (@rem(e.y, 2) != 0) @as(i32, 70) else 18) + e.x * 104 - 5;
            gfx.drawTex(self.bf.texIndex(bf_target), f32i(xx), f32i(15 + e.y * 90 + 60));
        }
    }

    fn drawUnits(self: *BattleScene, c: *Ctx) void {
        // Corpses (layer 1), then the selection, then the living units by
        // their row, moving ones on top.
        for (0..2) |s| for (&self.b.units[s], 0..) |*f, i| {
            const v = &self.vis[s][i];
            if (f.unit.type == 0 or !v.corpse or v.hidden) continue;
            if (!f.in_battle and v.death == null) continue;
            const tex = self.bf.texIndex(bf_corpse);
            const a: u8 = if (v.death) |d| @intCast(std.math.clamp(d.timer * 4 + 1, 0, 255)) else 255;
            drawFlipped(tex, v.x, v.y, s == 1, .{ .r = 255, .g = 255, .b = 255, .a = a });
        };
        if (self.b.active) |ar| if (self.phase != .result) {
            const f = self.b.fighter(ar);
            const v = self.visOf(ar);
            if (f.life > 0) {
                const sz = self.picSize(f);
                const add_x: i32 = if (sz[0] > 64) @divTrunc(sz[0] - 64, 2) else 0;
                const add_y: i32 = if (sz[1] > 80) sz[1] - 80 else 0;
                gfx.drawTex(self.bf.texIndex(bf_select), v.x - 5 + f32i(add_x), v.y + 60 + f32i(add_y));
            }
        };
        var order: [2 * bt.n_slots]bt.Ref = undefined;
        var n: usize = 0;
        for (0..2) |s| for (&self.b.units[s], 0..) |*f, i| {
            const v = &self.vis[s][i];
            if (f.unit.type == 0) continue;
            if (v.alive or v.death != null) {
                order[n] = .{ .side = @intCast(s), .slot = @intCast(i) };
                n += 1;
            }
        };
        const Ctxs = struct {
            sc: *BattleScene,
            fn less(ctx: @This(), a: bt.Ref, b2: bt.Ref) bool {
                const va = ctx.sc.visOf(a);
                const vb = ctx.sc.visOf(b2);
                if (va.moving != vb.moving) return vb.moving;
                return va.y < vb.y;
            }
        };
        std.mem.sort(bt.Ref, order[0..n], Ctxs{ .sc = self }, Ctxs.less);
        const show_bars = c.human.flag(.war_bar) or rl.isKeyDown(.left_alt) or rl.isKeyDown(.right_alt);
        for (order[0..n]) |r| {
            const f = self.b.fighter(r);
            const v = self.visOf(r);
            const flip = r.side == 1;
            const pic = self.picOf(f);
            if (v.death) |d| {
                const a: i32 = std.math.clamp((51 - d.timer) * 5, 1, 255);
                drawFlipped(pic, v.x, v.y, flip, .{ .r = 255, .g = 255, .b = 255, .a = @intCast(a) });
                self.drawDeath(d);
                if (v.stars) |stars| stars.draw(v.x, v.y);
                continue;
            }
            if (proto.UnitType.picIndex(@intCast(f.unit.type))) |k| {
                const sh = if (flip) self.shadowf.texIndex(k) else self.shadow.texIndex(k);
                gfx.drawTex2(sh, v.x, v.y, .{ .r = 255, .g = 255, .b = 255, .a = 128 });
            }
            if (v.raise) |raise| raise.draw(v.x, v.y, flip) else drawFlipped(if (v.stone) |stone| stone.texture else pic, v.x, v.y, flip, .white);
            if (v.stars) |stars| stars.draw(v.x, v.y);
            if (v.web > 0) {
                const web = self.anim.texIndex(anim_web + v.web / 3);
                const sz = self.picSize(f);
                drawFlipped(web, v.x + @trunc((f32i(sz[0]) - gfx.texW(web)) / 2), v.y + f32i(sz[1]) - gfx.texH(web), !flip, .white);
            }
            if (show_bars) self.drawBar(c, f, v);
        }
    }

    /// rotate_sprite_v_flip(…, 128): the picture mirrored left to right.
    fn drawFlipped(t: ?rl.Texture2D, x: f32, y: f32, flip: bool, tint: rl.Color) void {
        const tx = t orelse return;
        const w = f32i(tx.width);
        const h = f32i(tx.height);
        rl.drawTextureRec(tx, .{ .x = 0, .y = 0, .width = if (flip) -w else w, .height = h }, .{ .x = x, .y = y }, tint);
    }

    fn drawDeath(self: *BattleScene, d: Death) void {
        for (d.clouds) |cl| {
            const num: usize = @intCast(std.math.clamp(@divTrunc(d.timer, 6), 0, 7));
            const t = self.anim.texIndex(anim_cloud + num) orelse continue;
            const w = f32i(t.width);
            const h = f32i(t.height);
            const deg = f32i(cl.angle) * 360 / 256;
            rl.drawTexturePro(t, .{ .x = 0, .y = 0, .width = w, .height = h }, .{ .x = cl.x - 5 + w / 2, .y = cl.y - 3 + h / 2, .width = w, .height = h }, .{ .x = w / 2, .y = h / 2 }, deg, .{ .r = 255, .g = 255, .b = 255, .a = @intCast(cl.trans) });
        }
        var jaw: i32 = 0;
        if (d.timer > 30) jaw = @min(@divTrunc(d.timer - 30, 2), 10);
        const a: i32 = std.math.clamp(if (d.timer < 50) 30 + 5 * d.timer else (76 - d.timer) * 5, 1, 255);
        const tint: rl.Color = .{ .r = 255, .g = 255, .b = 255, .a = @intCast(a) };
        gfx.drawTex2(self.anim.texIndex(anim_skull), d.x, d.y, tint);
        gfx.drawTex2(self.anim.texIndex(anim_jaw), d.x + 1, d.y + 35 + f32i(jaw), tint);
    }

    /// DrawBar: life, stamina and morale under the unit, the move points.
    fn drawBar(self: *BattleScene, c: *Ctx, f: *const bt.Fighter, v: *const Vis) void {
        const ml = self.b.maxLife(f);
        const ms = self.b.maxStamina(f);
        const mm = self.b.maxMorale(f);
        if (f.life <= 0 or ml <= 0 or ms <= 0 or mm <= 0) return;
        const bar = self.bf.texIndex(bf_unit_bar) orelse return;
        const sz = self.picSize(f);
        const bx = v.x + f32i(@divTrunc(sz[0] - bar.width, 2));
        const by = v.y + f32i(sz[1]);
        gfx.drawTex(bar, bx, by);
        const ix: i32 = @intFromFloat(bx);
        const iy: i32 = @intFromFloat(by);
        rl.drawRectangle(ix + 15, iy + 2, @min(@divTrunc(30 * f.life, ml), 30) + 1, 2, .{ .r = 200, .g = 0, .b = 0, .a = 255 });
        rl.drawRectangle(ix + 15, iy + 7, @min(@divTrunc(30 * f.stamina, ms), 30) + 1, 3, .{ .r = 200, .g = 200, .b = 0, .a = 255 });
        rl.drawRectangle(ix + 15, iy + 12, @min(@divTrunc(30 * f.morale, mm), 30) + 1, 4, .{ .r = 50, .g = 50, .b = 250, .a = 255 });
        var buf: [8]u8 = undefined;
        const col: rl.Color = if (f.can_act) .{ .r = 20, .g = 200, .b = 20, .a = 255 } else .{ .r = 200, .g = 20, .b = 20, .a = 255 };
        c.font1.draw(std.fmt.bufPrint(&buf, "{d}", .{f.speed}) catch "", bx + 3, by + 3, col);
    }

    fn drawMissile(self: *BattleScene) void {
        const a = self.act orelse return;
        switch (a) {
            .missile => |m| {
                const t = m.tex orelse return;
                const w = f32i(t.width);
                const h = f32i(t.height);
                rl.drawTexturePro(t, .{ .x = 0, .y = 0, .width = w, .height = h }, .{ .x = m.pos.x + w / 2, .y = m.pos.y + h / 2, .width = w, .height = h }, .{ .x = w / 2, .y = h / 2 }, m.angle * 360 / 256, .white);
            },
            else => {},
        }
    }

    /// ShowUnit on objPanel, and the buttons.
    fn drawPanel(self: *BattleScene, c: *Ctx) void {
        const panel = self.bf.texIndex(bf_panel);
        const px: f32 = 1024 - gfx.texW(panel);
        gfx.drawTex(panel, px, 0);
        const show: ?bt.Ref = if (self.phase == .placing) (if (self.picked) |k| bt.Ref{ .side = 0, .slot = k } else null) else self.b.active;
        if (self.pending) |pd| {
            self.drawSpellPanel(c, pd, px);
            if (self.idle()) self.cancel_spell.draw(&self.buttons, c.tick);
            return;
        }
        if (show) |r| {
            const f = self.b.fighter(r);
            if (f.life > 0) self.drawStats(c, f, px, false);
        }
        // buttMenu: inactive outside the player's orders (HideButtons).
        if (self.phase == .placing or self.phase == .fighting) {
            self.menu.on = self.menuReady();
            self.menu.draw(&self.buttons, c.tick);
        }
        switch (self.phase) {
            .placing => self.start.draw(&self.buttons, c.tick),
            .fighting => if (self.b.active != null and self.playerTurn() and self.idle()) {
                self.rest.draw(&self.buttons, c.tick);
                self.next.draw(&self.buttons, c.tick);
                self.spell.draw(&self.buttons, c.tick);
                self.end.draw(&self.buttons, c.tick);
            },
            else => {},
        }
    }

    /// ShowEnemy: Prepare's hover inspects allies; during the player's
    /// orders either side is visible. ActiveSpell also permits the caster.
    fn sideBarUnit(self: *BattleScene) ?bt.Ref {
        if (!rl.isKeyDown(.left_alt) and !rl.isKeyDown(.right_alt)) return null;
        if (self.book_open or !self.idle()) return null;
        const cell = self.hover orelse return null;
        const r = switch (self.phase) {
            .placing => self.b.unitAt(0, cell[0], cell[1]),
            .fighting => if (self.playerTurn()) self.b.unitAt(0, cell[0], cell[1]) orelse self.b.unitAt(1, cell[0], cell[1]) else null,
            else => null,
        } orelse return null;
        const active: ?bt.Ref = if (self.phase == .placing) (if (self.picked) |k| .{ .side = 0, .slot = k } else null) else self.b.active;
        if (active) |a| {
            if (a.side == r.side and a.slot == r.slot and self.pending == null) return null;
        }
        return r;
    }

    /// bmpSideBar covers only the top 700 pixels, keeping the turn/menu
    /// buttons below it. The ability grid replaces the action buttons.
    fn drawSideBar(self: *BattleScene, c: *Ctx, f: *const bt.Fighter) void {
        const panel = self.bf.texIndex(bf_panel);
        const px: f32 = 1024 - gfx.texW(panel);
        rl.beginScissorMode(@intFromFloat(px), 0, @intFromFloat(gfx.texW(panel)), 700);
        defer rl.endScissorMode();
        gfx.drawTex(panel, px, 0);
        self.drawStats(c, f, px, true);
        var n: usize = 0;
        if (f.summoned) {
            self.drawSideAbility(c, px, n, 103, 0);
            n += 1;
        }
        var skill: i32 = 13;
        while (skill < 3000 and n < 8) : (skill += 1) {
            const power = self.b.skill(f, skill);
            if (power <= 0) continue;
            for (c.proto.abilities[1..], 1..) |a, i| {
                if (a.num != skill) continue;
                self.drawSideAbility(c, px, n, i, if (a.numeric) power else 0);
                n += 1;
                break;
            }
        }
    }

    fn drawSideAbility(self: *BattleScene, c: *Ctx, px: f32, slot: usize, ability: usize, power: i32) void {
        const x = px + 14 + 62 * f32i(@intCast(slot % 2));
        const y = 247 + 56 * f32i(@intCast(slot / 2));
        gfx.drawTex(self.abilities.texIndex(ability), x, y);
        if (power <= 0) return;
        // ShowIconNumber: one- and two-digit plates from stuff.dat.
        var buf: [16]u8 = undefined;
        const s = std.fmt.bufPrint(&buf, "{d}", .{power}) catch "";
        if (power < 10) {
            gfx.drawTex(self.stuff.texIndex(32), x + 37, y + 32);
            c.font1.draw(s, x + 40, y + 36, color1);
        } else {
            gfx.drawTex(self.stuff.texIndex(33), x + 29, y + 33);
            c.font1.drawCentered(s, x + 39, y + 37, color1);
        }
    }

    fn spellPic(self: *BattleScene, sp: i32) ?rl.Texture2D {
        return if (sp > 0) self.spells.texIndex(@intCast(sp - 1)) else null;
    }

    /// bmpSpellPanel with txtSpName, txtSpTarget and txtSpDesc.
    fn drawSpellPanel(self: *BattleScene, c: *Ctx, pd: bs.Pending, px: f32) void {
        gfx.drawTex(self.bf.texIndex(bf_spell_panel), px, 0);
        const sp = bs.spellOf(&self.b, pd.spell);
        gfx.drawTex(self.spellPic(pd.spell), px + 44, 21);
        gfx.drawTex(self.spellPic(pd.spell), px + 44, 213);
        _ = c.font1.drawWrapped(sp.name, px + 11, 106, 122, color1);
        _ = c.font1.drawWrapped(c.proto.phrase(if (pd.sacrifice) ph_sacrifice else ph_target), px + 11, 144, 122, color1);
        _ = c.font2.drawWrappedClip(sp.text, px + 11, 300, 122, 660, color1);
    }

    /// Spellbook(): the spells, the crystals or the ammo, and the hovered
    /// spell's name, power, text, price and stamina.
    fn drawBook(self: *BattleScene, c: *Ctx) void {
        const ar = self.b.active orelse return;
        const a = self.b.fighter(ar);
        gfx.drawTex(self.bf.texIndex(bf_spellbook), 262, 112);
        const entries = bs.book(&self.b, ar);
        const half: rl.Color = .{ .r = 255, .g = 255, .b = 255, .a = 128 };
        for (entries, 0..) |e, i| {
            if (e.spell <= 0) continue;
            const x = 302 + f32i(@intCast((i % 3) * 64));
            const y = 146 + f32i(@intCast((i / 3) * 66));
            gfx.drawTex(self.spellPic(e.spell), x, y);
            if (!e.castable) {
                if (!a.hero_unit) {
                    gfx.drawTex(self.bf.texIndex(bf_spell_shadow), x, y);
                } else if (e.used) {
                    rl.drawRectangle(@intFromFloat(x), @intFromFloat(y), 52, 52, .{ .r = 0, .g = 0, .b = 0, .a = 128 });
                    gfx.drawTex2(self.stuff.texIndex(stuff_dark1), x, y, half);
                    gfx.drawTex2(self.stuff.texIndex(stuff_dark2), x, y, half);
                } else {
                    rl.drawRectangle(@intFromFloat(x), @intFromFloat(y), 52, 52, .{ .r = 100, .g = 0, .b = 0, .a = 128 });
                    gfx.drawTex(self.iface.texIndex(gui_no_gem), x + 23, y + 2);
                }
            }
            if (self.book_hover == i) gfx.drawTex(self.iface.texIndex(gui_spell_select), x - 6, y - 6);
        }
        var buf: [64]u8 = undefined;
        const font = &c.font1;
        if (a.hero_unit) {
            font.drawCentered(std.fmt.bufPrint(&buf, "{d}", .{self.b.gem[ar.side]}) catch "", 405, 612, color1);
        } else {
            const t = std.fmt.bufPrint(&buf, "{d}", .{a.ammo}) catch "";
            font.drawCentered(t, 406, 613, color1);
            font.drawCentered(t, 405, 612, .{ .r = 20, .g = 130, .b = 150, .a = 255 });
        }
        if (self.book_hover) |i| {
            const sp_i = entries[i].spell;
            const sp = bs.spellOf(&self.b, sp_i);
            const shadow: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
            font.drawCentered(sp.name, 530 + 95 + 1, 152 + 1, shadow);
            font.drawCentered(sp.name, 530 + 95, 152, .{ .r = 50, .g = 250, .b = 250, .a = 255 });
            const line = self.powerLine(c, a, sp, &buf);
            if (line.text.len > 0) {
                font.drawCentered(line.text, 530 + 95 + 1, 172 + 1, shadow);
                font.drawCentered(line.text, 530 + 95, 172, .{ .r = 250, .g = 100, .b = 10, .a = 255 });
            }
            _ = c.font2.drawWrappedClip(sp.text, 530, line.text_y, 189, 607, color1);
            var nb: [16]u8 = undefined;
            const cost = if (a.hero_unit) bs.price(&self.b, a, sp_i) else self.b.skill(a, 2000 + sp_i);
            font.drawCentered(std.fmt.bufPrint(&nb, "{d}", .{cost}) catch "", 262 + 316, 112 + 498, color1);
            font.drawCentered(std.fmt.bufPrint(&nb, "{d}", .{sp.stamina_cost}) catch "", 262 + 425, 112 + 498, color1);
        }
        self.close_book.draw(&self.buttons, c.tick);
    }

    /// PowerBox of Spellbook() and where TextBox starts.
    fn powerLine(self: *BattleScene, c: *Ctx, a: *const bt.Fighter, sp: *const proto.Spell, buf: []u8) struct { text: []const u8, text_y: f32 } {
        const b = &self.b;
        const e = sp.effects;
        var eff: i32 = 0;
        var power: i32 = 0;
        var y: f32 = 212;
        switch (e[0].id) {
            31, 47, 89, 92 => y = 182,
            71, 72 => {
                eff = e[1].id;
                power = e[1].power;
            },
            else => if (e[0].id == 73 and sp.duration_mod == 0) {
                y = 182;
            } else {
                eff = e[0].id;
                power = e[0].power;
            },
        }
        if (eff <= 0) return .{ .text = "", .text_y = y };
        var ph: u32 = ph_power;
        if (eff > 1000 and sp.duration_mod <= 0) {
            ph = ph_summon;
            power = b.skill(a, 907);
            const t = @mod(eff, 1000);
            if (t < c.proto.units.len and c.proto.units[@intCast(t)].unit_kind == 2) power = @divTrunc(power, 2) + b.skill(a, 909);
        } else if (sp.duration_mod > 0) {
            ph = ph_duration;
            power = e[0].duration + @divTrunc(b.skill(a, 906) * sp.duration_mod, 100);
        } else power += @divTrunc(b.skill(a, 905) * sp.power_mod, 100);
        const tilde = if (eff == 1 and sp.target == 4) "~" else "";
        return .{ .text = std.fmt.bufPrint(buf, "{s}: {d}{s}", .{ c.proto.phrase(ph), power, tilde }) catch "", .text_y = y };
    }

    fn drawStats(self: *BattleScene, c: *Ctx, f: *const bt.Fighter, px: f32, side_bar: bool) void {
        const b = &self.b;
        gfx.drawTex(self.picOf(f), px + 7, 15);
        var buf: [16]u8 = undefined;
        const num = struct {
            fn s(bf: []u8, v: i32) []const u8 {
                return std.fmt.bufPrint(bf, "{d}", .{v}) catch "";
            }
        }.s;
        const font = &c.font1;
        font.drawCentered(num(&buf, f.life), px + 115, 26, calcColor(f.life, b.maxLife(f)));
        font.drawCentered(num(&buf, f.stamina), px + 115, 52, calcColor(f.stamina, b.maxStamina(f)));
        font.drawCentered(num(&buf, f.morale), px + 115, 78, calcColor(f.morale, b.maxMorale(f)));
        const lvl = if (f.hero_unit) (if (b.armies[f.home].hero) |h| h.level else f.unit.level) else f.unit.level;
        font.drawCentered(num(&buf, lvl), px + 33, 104, .{ .r = 210, .g = 20, .b = 250, .a = 255 });
        // ShowUnit: SmallPic, preserving each of the three medal slots.
        for (f.unit.medals, 0..) |md, i| {
            if (md > 0) gfx.drawTex(self.medals.texIndex(@intCast(md - 1)), px + 45 + 13 * f32i(@intCast(i)), 102);
        }
        if (side_bar) {
            font.drawCentered(std.fmt.bufPrint(&buf, "{d}  ", .{f.speed}) catch "", px + 115, 104, calcColor(f.speed, b.maxSpeed(f)));
            font.drawCentered(std.fmt.bufPrint(&buf, " /{d}", .{b.maxSpeed(f)}) catch "", px + 115, 104, .{ .r = 220, .g = 220, .b = 50, .a = 255 });
        } else font.drawCentered(num(&buf, f.speed), px + 115, 104, calcColor(f.speed, b.maxSpeed(f)));
        const rows = [4][2][2]i32{
            .{ .{ b.attack(f), b.typeStat(f, 2) }, .{ b.counterAttack(f), b.typeStat(f, 3) } },
            .{ .{ b.rangedAttack(f), b.typeStat(f, 8) }, .{ b.shootingRange(f), b.typeStat(f, 9) } },
            .{ .{ f.ammo, b.maxAmmo(f) }, .{ b.resist(f), b.typeStat(f, 6) } },
            .{ .{ b.defence(f), b.typeStat(f, 4) }, .{ b.rangedDefence(f), b.typeStat(f, 5) } },
        };
        for (rows, 0..) |row, k| {
            const y = 132 + 31 * @as(f32, @floatFromInt(k));
            font.drawCentered(num(&buf, row[0][0]), px + 50, y, calcColor(row[0][0], row[0][1]));
            font.drawCentered(num(&buf, row[1][0]), px + 111, y, calcColor(row[1][0], row[1][1]));
        }
        var lb: [64]u8 = undefined;
        font.drawCentered(std.fmt.bufPrint(&lb, "{s}:", .{c.proto.phrase(ph_params)}) catch "", px + 71, 482, color1);
        // The visible effects, six at most (dX 13, dY 498, dW 62, dH 68).
        var n: usize = 0;
        for (f.effectList()) |e| {
            if (!e.visible) continue;
            if (n == 6) break;
            const ex = px + 13 + 62 * f32i(@intCast(n % 2));
            const ey = 498 + 68 * f32i(@intCast(n / 2));
            if (e.spell > 0) {
                gfx.drawTex(self.spellPic(e.spell), ex, ey);
            } else if (e.ability > 0) gfx.drawTex(self.skills.texIndex(@intCast(e.ability)), ex, ey);
            if (e.time > 0) {
                const t = num(&buf, e.time);
                font.draw(t, ex + 24, ey + 54, color1);
                font.draw(t, ex + 23, ey + 53, color2);
            }
            n += 1;
        }
    }

    /// EndBattleScreen.
    fn drawResult(self: *BattleScene, c: *Ctx) void {
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 255 - 190 });
        const res = self.outcome.result;
        const win = self.iface.texIndex(if (res == 1) gui_victory else gui_lose);
        const x = 512 - gfx.texW(win) / 2;
        const y = 384 - gfx.texH(win) / 2;
        gfx.drawTex(win, x, y);
        const pr = c.proto;
        const font = &c.font1;
        switch (res) {
            1 => {
                font.drawCentered(pr.phrase(18), x + 166, y + 38, color1);
                font.drawCentered(pr.phrase(19), x + 611, y + 38, color1);
                font.drawCentered(pr.phrase(22), x + 390, y + 13, color1);
                font.drawCentered(pr.phrase(20), x + 390, y + 71, color1);
                var buf: [16]u8 = undefined;
                font.drawCentered(std.fmt.bufPrint(&buf, "{d}", .{self.outcome.gold}) catch "", x + 400, y + 104, color2);
                font.drawCentered(std.fmt.bufPrint(&buf, "{d}", .{self.outcome.gem}) catch "", x + 400, y + 150, color2);
                // Loot[0..3] (LootY 185).
                for (self.outcome.loot[0..4], 0..) |it, i| {
                    if (it[0] <= 0 or it[0] >= pr.items.len) continue;
                    const pic = pr.items[@intCast(it[0])].pic orelse continue;
                    if (pic.sheet != .items) continue;
                    gfx.drawTex(self.items.texIndex(pic.index), x + 333 + f32i(@intCast(60 * (i % 2))), y + 185 + f32i(@intCast(60 * (i / 2))));
                }
            },
            0 => {
                font.drawCentered(pr.phrase(19), x + 166, y + 38, color1);
                font.drawCentered(pr.phrase(18), x + 611, y + 38, color1);
                font.drawCentered(pr.phrase(23), x + 380, y + 13, color1);
                font.drawCentered(pr.phrase(21), x + 390, y + 71, color1);
            },
            else => {
                font.drawCentered(pr.phrase(18), x + 166, y + 38, color1);
                font.drawCentered(pr.phrase(19), x + 611, y + 38, color1);
                font.drawCentered(pr.phrase(49), x + 390, y + 13, color1);
            },
        }
        // The squads: the winner left, the loser right.
        const win_side: u1 = if (res == 0) 1 else 0;
        for ([2]u1{ win_side, 1 - win_side }, [2]f32{ 26, 471 }) |s, ox| {
            var k: usize = 0;
            for (self.b.units[s][0..bt.max_squad]) |*f| {
                if (f.unit.type == 0) continue;
                const icon = if (proto.UnitType.picIndex(@intCast(f.unit.type))) |pi| self.icons.texIndex(pi) else null;
                const tint: rl.Color = if (f.life > 0) .white else .{ .r = 90, .g = 90, .b = 90, .a = 255 };
                gfx.drawTex2(icon, x + ox + f32i(@intCast((k % 4) * 72)), y + 64 + f32i(@intCast((k / 4) * 88)), tint);
                k += 1;
            }
        }
        self.exit.draw(&self.buttons, c.tick);
    }
};

/// A sheet's objects as CPU images for the background, loaded once.
const Images = struct {
    sheet: *gfx.Sheet,
    map: std.AutoHashMapUnmanaged(usize, ?rl.Image) = .empty,

    fn get(self: *Images, k: usize) ?rl.Image {
        if (self.map.get(k)) |v| return v;
        const img = self.sheet.imageIndex(k);
        self.map.put(self.sheet.gpa, k, img) catch {};
        return img;
    }

    fn deinit(self: *Images) void {
        var it = self.map.valueIterator();
        while (it.next()) |v| if (v.*) |img| rl.unloadImage(img);
        self.map.deinit(self.sheet.gpa);
    }
};

test "calc color" {
    try std.testing.expectEqual(@as(u8, 220), calcColor(5, 5).g);
    try std.testing.expectEqual(@as(u8, 10), calcColor(0, 5).g);
    try std.testing.expectEqual(@as(u8, 230), calcColor(20, 5).r);
}
