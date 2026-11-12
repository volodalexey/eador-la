//! Battlefield of InitBattle (Battlefield.cpp:5037): the 8×8 hex field and
//! the screen placement of its cells and units (BF_Unit.cpp:16, TransX /
//! TransY; Battlefield.cpp:446, GetTileType).
//!
//! The field is rolled column by column with Roll(0, i, j, P1, P2): keyed
//! numbers of the seed, no rand() is used.  On the border of two provinces
//! the left columns favour the attacker's terrain and the right ones the
//! defender's; inside a province both sides share it.  A castle assault
//! clears columns 4..7 and puts the wall line on column 5.  One random
//! row is then made passable through columns 2..5.
//!
//! The battle itself (BF_Unit.cpp, Battlefield.cpp): the units of both
//! sides with their battle stats, the move, path and shot maps, melee with
//! first strike and counterattack, shots, damage, death, the start of a
//! side's turn and the outcome (EndBattle).  Every visible step is put in
//! `anims` for the screen (the G_Object animations of the original).
//!
//! The computer side is battle_ai.zig (BattleAI.cpp); it presses the
//! crush, circular attack and double shot buttons through `crush`,
//! `round_attack` and `double_shot`.
//!
//! Not ported yet: towers, the special unit
//! buttons for a human player, item wear, the frightened-enemy victory dialog.
const std = @import("std");
const proto = @import("../data/proto.zig");
const rand = @import("../data/rand.zig");
const Rand = rand.Rand;
const unit_mod = @import("unit.zig");
const hero_mod = @import("hero.zig");
const battle_ai = @import("battle_ai.zig");
const battle_spell = @import("battle_spell.zig");
const province_mod = @import("province.zig");

pub const size = 8;

/// arGrid Land values above the bf_object.var records: the castle wall
/// (9), a tower (10) and a breach (11).
pub const land_wall = 9;
pub const land_tower = 10;
pub const land_breach = 11;

pub const Cell = struct {
    /// bf_object.var record (1 is the plain field) or land_wall..breach.
    land: i32 = 1,
    /// Pass: no unit stands here and it is not a tower.
    pass: bool = true,
};

pub const Field = struct {
    cells: [size][size]Cell = @splat(@splat(.{})),

    pub fn at(self: *const Field, x: i32, y: i32) *const Cell {
        return &self.cells[@intCast(x)][@intCast(y)];
    }

    /// GetTileType: the land of the cell holding screen point (x, y).
    pub fn tileType(self: *const Field, x_in: i32, y: i32) i32 {
        var x = x_in;
        var j = @divTrunc(y - @rem(y, 90), 90);
        if (@rem(j, 2) != 0) x -= 52;
        var i = @divTrunc(x - @rem(x, 104), 104);
        if (@rem(y, 90) < 30) {
            if (@as(i32, @intCast(@abs(@rem(x, 104) - 52))) > @divTrunc(@rem(y, 90) * 52, 30)) {
                if (@rem(j, 2) != 0 and @rem(x, 104) > 52) i += 1;
                if (@rem(j, 2) == 0 and @rem(x, 104) < 52) i -= 1;
                j -= 1;
            }
        }
        i = std.math.clamp(i, 0, size - 1);
        j = std.math.clamp(j, 0, size - 1);
        return self.cells[@intCast(i)][@intCast(j)].land;
    }
};

/// TransX: screen x of a unit picture `w` wide in cell (x, y).
pub fn transX(x: i32, y: i32, w: i32) i32 {
    const base: i32 = (if (@rem(y, 2) != 0) @as(i32, 70) else 18) + x * 104;
    return if (w > 64) base - @divTrunc(w - 64, 2) else base;
}

/// TransY: screen y of a unit picture `h` high in cell (x, y).
pub fn transY(_: i32, y: i32, h: i32) i32 {
    return if (h > 80) 15 + y * 90 - (h - 80) else 15 + y * 90;
}

/// The top-left corner of a cell's hexagon (Grid[i][j], objGrid).
pub fn cellX(x: i32, y: i32) i32 {
    return x * 104 + (if (@rem(y, 2) != 0) @as(i32, 52) else 0);
}

pub fn cellY(y: i32) i32 {
    return y * 90;
}

/// RollerN / RollerP with Roll(0, P1, P2, P3, P4).
const Roller = struct {
    ids: [64]i32 = @splat(0),
    w: [64]i32 = @splat(0),
    n: usize = 0,

    fn add(self: *Roller, id: i32, weight: i32) void {
        self.ids[self.n] = id;
        self.w[self.n] = weight;
        self.n += 1;
    }

    /// The sum stops at the first id 0 (a terrain without a battlefield
    /// object ends the list early, as in the original); the pick does not.
    fn roll(self: *const Roller, seed: u32, p1: i32, p2: i32, p3: i32, p4: i32) i32 {
        var sum: u32 = 0;
        for (self.ids[0..self.n], self.w[0..self.n]) |id, w| {
            if (id == 0) break;
            sum +%= @bitCast(w);
        }
        const v = rand.keyed4(seed, sum, p1, p2, p3, p4) +% 1;
        var acc: u32 = 0;
        var k: usize = 0;
        var index: usize = 0;
        while (acc < v and k < self.ids.len) : (k += 1) {
            acc +%= @bitCast(self.w[k]);
            index = k;
        }
        while (index + 1 < self.ids.len and self.w[index] == 0) index += 1;
        return self.ids[index];
    }
};

/// The 1 (45) and terrain entries, then every passable object at `rest`.
fn roller(pr: *const proto.Proto, heads: []const [2]i32, rest: i32) Roller {
    var r: Roller = .{};
    r.add(1, 45);
    for (heads) |h| r.add(h[0], h[1]);
    var i: usize = 2;
    while (i < pr.bf_objects.len and r.n < r.ids.len) : (i += 1) {
        if (pr.bf_objects[i].move_cost != 0) r.add(@intCast(i), rest);
    }
    return r;
}

/// A province taking part: its number and terrain.
pub const Side = struct { number: i32, terrain: i32 };

/// The fortifications of a castle assault: Fort as a percent of the wall
/// and the number of towers.
pub const Fort = struct { percent: i32, towers: i32 };

pub const Setup = struct {
    /// CurGame.iSeed.
    seed: u32,
    /// Prov1: where the battle is (the attacker's province on a border).
    prov1: Side,
    /// Prov2: the defender's province for a border battle.
    prov2: ?Side = null,
    /// The site index (100 for a castle).
    site: i32 = 0,
    castle: ?Fort = null,
};

fn bfPass(pr: *const proto.Proto, t: i32) i32 {
    return if (t >= 0 and t < pr.terrains.len) pr.terrains[@intCast(t)].bf_pass else 0;
}

fn bfImpass(pr: *const proto.Proto, t: i32) i32 {
    return if (t >= 0 and t < pr.terrains.len) pr.terrains[@intCast(t)].bf_impass else 0;
}

/// The field of InitBattle.
pub fn generate(pr: *const proto.Proto, s: Setup) Field {
    var f: Field = .{};
    const seed = s.seed;
    const n1 = s.prov1.number;
    if (s.prov2) |p2| {
        const lp = bfPass(pr, s.prov1.terrain);
        const li = bfImpass(pr, s.prov1.terrain);
        const rp = bfPass(pr, p2.terrain);
        const ri = bfImpass(pr, p2.terrain);
        const n2 = p2.number;
        // Columns 0, 1.
        var r = roller(pr, &.{.{ lp, 40 }}, 5);
        for (0..2) |i| for (0..size) |j| {
            f.cells[i][j] = .{ .land = r.roll(seed, @intCast(i), @intCast(j), n1, n2) };
        };
        // Columns 2 and 3.
        r = roller(pr, &.{ .{ lp, 30 }, .{ rp, 6 }, .{ li, 7 }, .{ ri, 3 } }, 3);
        for (0..size) |j| f.cells[2][j] = .{ .land = r.roll(seed, 2, @intCast(j), n1, n2) };
        r = roller(pr, &.{ .{ lp, 26 }, .{ rp, 10 }, .{ li, 6 }, .{ ri, 4 } }, 3);
        for (0..size) |j| f.cells[3][j] = .{ .land = r.roll(seed, 3, @intCast(j), n1, n2) };
        // Columns 6, 7.
        r = roller(pr, &.{.{ rp, 40 }}, 5);
        for (6..size) |i| for (0..size) |j| {
            f.cells[i][j] = .{ .land = r.roll(seed, @intCast(i), @intCast(j), n1, n2) };
        };
        // Columns 5 and 4; column 4 rolls with the keys of column 3.
        r = roller(pr, &.{ .{ rp, 30 }, .{ lp, 6 }, .{ ri, 7 }, .{ li, 3 } }, 3);
        for (0..size) |j| f.cells[5][j] = .{ .land = r.roll(seed, 5, @intCast(j), n1, n2) };
        r = roller(pr, &.{ .{ rp, 26 }, .{ lp, 10 }, .{ ri, 6 }, .{ li, 4 } }, 3);
        for (0..size) |j| f.cells[4][j] = .{ .land = r.roll(seed, 3, @intCast(j), n1, n2) };
    } else {
        const pass = bfPass(pr, s.prov1.terrain);
        const impass = bfImpass(pr, s.prov1.terrain);
        const site = s.site;
        var r = roller(pr, &.{.{ pass, 40 }}, 5);
        for ([_]usize{ 0, 1, 6, 7 }) |i| for (0..size) |j| {
            f.cells[i][j] = if (s.castle != null and i >= 6) .{} else .{ .land = r.roll(seed, @intCast(i), @intCast(j), n1, site) };
        };
        r = roller(pr, &.{ .{ pass, 36 }, .{ impass, 10 } }, 3);
        for (2..6) |i| for (0..size) |j| {
            f.cells[i][j] = if (s.castle != null and i > 3) .{} else .{ .land = r.roll(seed, @intCast(i), @intCast(j), n1, site) };
        };
        if (s.castle) |c| fortify(&f, c);
    }
    // Random(8, Prov1->iNumber, CurGame.iSeed).
    const row: usize = @min(rand.keyed(seed, size, n1, @bitCast(seed)), size - 1);
    for (2..6) |i| {
        const c = &f.cells[i][row];
        if (c.land < land_wall and c.land >= 0 and c.land < pr.bf_objects.len and pr.bf_objects[@intCast(c.land)].move_cost == 0) c.land = 1;
    }
    return f;
}

/// The wall line on column 5: a breach where the fort is too damaged, a
/// tower where there are enough of them (towers are not passable), the
/// wall elsewhere.
fn fortify(f: *Field, c: Fort) void {
    // (row, fort limit of a breach, towers needed or 0 for none).
    const line = [_][3]i32{ .{ 0, 0, 4 }, .{ 6, 10, 3 }, .{ 2, 25, 2 }, .{ 4, 35, 1 }, .{ 1, 30, 0 }, .{ 3, 50, 0 }, .{ 5, 40, 0 }, .{ 7, 20, 0 } };
    for (line) |l| {
        const cell = &f.cells[5][@intCast(l[0])];
        const breach = if (l[0] == 0) c.percent == 0 else c.percent <= l[1];
        if (breach) {
            cell.land = land_breach;
        } else if (l[2] > 0 and c.towers >= l[2]) {
            cell.* = .{ .land = land_tower, .pass = false };
        } else cell.land = land_wall;
    }
}

// ---------------------------------------------------------------------
// Units and the battle
// ---------------------------------------------------------------------

/// MAX_SQUAD; the summons (MAX_SUMMONS = SPELLBOOK_SIZE) take the slots
/// after the squad.
pub const max_squad = 16;
pub const spellbook_size = 21;
pub const max_summons = spellbook_size;
pub const n_slots = max_squad + max_summons;
pub const max_effect = 48;
/// POISON_DURATION.
pub const poison_duration = 3;

/// Unit kinds without blood or corpse (undead, constructs).
const kind_undead = 2;
const kind_construct = 4;

/// Ability ids (unit_upg.var types) used here.
pub const ab = struct {
    pub const lesser_undead = 13;
    pub const flying = 14;
    pub const low_flying = 15;
    pub const first_strike = 16;
    pub const armor_piercing_shot = 17;
    pub const tireless = 18;
    pub const fearless = 19;
    pub const recovery = 21;
    pub const ammo_regen = 22;
    pub const healer = 24;
    pub const berserk = 25;
    pub const agility = 26;
    pub const magic_strike = 27;
    pub const magic_shot = 28;
    pub const march = 29;
    pub const knowledge_first = 32;
    pub const regen_on_rest = 35;
    pub const immobile = 36;
    pub const charge = 37;
    pub const no_combat = 38;
    pub const armor_piercing = 39;
    pub const stun = 40;
    pub const poison = 41;
    pub const poison_immunity = 42;
    pub const curse = 43;
    pub const steal_ammo = 44;
    pub const break_armor = 45;
    pub const petrify = 46;
    pub const web = 47;
    pub const regeneration = 48;
    pub const intimidation = 50;
    pub const cripple = 51;
    pub const break_weapon = 52;
    pub const parry = 53;
    pub const soul_steal = 57;
    pub const fire_arrows = 60;
    pub const smite_evil = 61;
    pub const roots = 67;
    pub const drain = 68;
    pub const heavy_shot = 69;
    pub const precise_strike = 76;
    pub const precise_shot = 77;
    pub const poison_shot = 78;
    pub const vulnerability = 79;
    pub const disease = 80;
    pub const trample = 63;
    pub const raise_corpse = 64;
    pub const round_attack = 65;
    pub const rooted = 88;
    pub const energy_control = 95;
    pub const reincarnation = 97;
    pub const poison_flesh = 96;
    pub const smite_good = 200;
    pub const web_immunity = 201;
    pub const petrify_immunity = 202;
    pub const roots_immunity = 203;
    pub const spell_strike = 206;
    pub const spell_shot = 207;
};

/// BF_Effect types of the spells the model handles itself.
pub const eff_charm = 73;
pub const eff_polymorph = 90;
pub const eff_reincarnate = 74;
pub const eff_dark_pact = 91;
/// The demons of the dark pact by the level of the fallen unit.
const demon_types = [4]i32{ 21, 37, 56, 65 };

/// Stat ids of the getters (also BF_Effect types).
pub const stat = struct {
    pub const life = 1;
    pub const attack = 2;
    pub const counter = 3;
    pub const defence = 4;
    pub const ranged_defence = 5;
    pub const resist = 6;
    pub const speed = 7;
    pub const ranged = 8;
    pub const range = 9;
    pub const ammo = 10;
    pub const stamina = 11;
    pub const morale = 12;
};

/// BF_Effect: (type, power, turns left; 0 for ever), the spell that cast
/// it (pSpell, 0 none) and bDestroyOnHit.
pub const Effect = struct {
    kind: i32,
    power: i32,
    time: i32,
    visible: bool = false,
    spell: i32 = 0,
    destroy_on_hit: bool = false,
    /// pAbility is a poison (41 or 78): the spell effect 93 removes it.
    poison: bool = false,
    /// pAbility: the unit_upg.var record whose picture (skills.dat) the
    /// effect shows; 0 for none.
    ability: i32 = 0,
};

/// pAbility of the hit effects (unit_upg.var records).
pub const upg_icon = struct {
    pub const poison = 71;
    pub const curse = 73;
    pub const break_armor = 75;
    pub const petrify = 76;
    pub const web = 77;
    pub const cripple = 81;
    pub const break_weapon = 82;
    pub const roots = 109;
    pub const heavy_shot = 114;
    pub const berserk = 55;
};

/// The counters EndBattle shares the experience by.
pub const Stats = struct {
    melee_done: i32 = 0,
    range_done: i32 = 0,
    melee_taken: i32 = 0,
    range_taken: i32 = 0,
    spell_taken: i32 = 0,
    poison_taken: i32 = 0,
    healed: i32 = 0,
    spells: i32 = 0,
    shot: i32 = 0,
    slain: i32 = 0,
    stamina_used: i32 = 0,
    survived: i32 = 0,

    fn work(s: Stats) i32 {
        return s.melee_done * 2 + s.range_done + s.melee_taken + s.range_taken + s.spell_taken + s.poison_taken +
            s.healed * 2 + s.spells * 3 + s.shot * 10 + s.slain * 20 + s.stamina_used + 1;
    }
};

pub const Ref = struct { side: u1, slot: u8 };

/// BF_Unit.
pub const Fighter = struct {
    /// pUnit: the squad's unit (its life is the one before the battle).
    unit: unit_mod.Unit,
    /// bInBattle: on the field (dead ones stay as corpses).
    in_battle: bool = false,
    /// pUnit->pHero: the hero's own unit (slot 0 of a hero with a class).
    hero_unit: bool = false,
    life: i32 = 0,
    morale: i32 = 0,
    /// iFear: the panic chance.
    fear: i32 = 0,
    stamina: i32 = 0,
    speed: i32 = 0,
    ammo: i32 = 0,
    x: i32 = 0,
    y: i32 = 0,
    can_act: bool = false,
    resting: bool = false,
    march: bool = false,
    parry: bool = false,
    /// bSummoned: a summon in a slot after the squad.
    summoned: bool = false,
    /// iSurvived = 1 of a dead summon: the slot is not used again.
    used: bool = false,
    /// pTrueForm: the unit before a polymorph (effect 90).
    true_form: ?unit_mod.Unit = null,
    /// The side whose squad the unit came from (its pHero's bonuses stay
    /// when a spell moves it) and iNumm, its slot there.
    home: u1 = 0,
    num: u8 = 0,
    st: Stats = .{},
    effects: [max_effect]Effect = undefined,
    n_eff: usize = 0,

    pub fn alive(f: *const Fighter) bool {
        return f.in_battle and f.life > 0;
    }

    /// pUnit != NULL: a unit holds the slot (a dead summon leaves it).
    pub fn present(f: *const Fighter) bool {
        return f.unit.type != 0 and !f.used;
    }

    pub fn addEffect(f: *Fighter, e: Effect) void {
        if (f.n_eff == max_effect) return;
        f.effects[f.n_eff] = e;
        f.n_eff += 1;
    }

    pub fn removeEffect(f: *Fighter, i: usize) void {
        for (i..f.n_eff - 1) |k| f.effects[k] = f.effects[k + 1];
        f.n_eff -= 1;
    }

    pub fn effectList(f: *const Fighter) []const Effect {
        return f.effects[0..f.n_eff];
    }
};

/// One side as InitBattle gets it (Heroes[i], BF_Player[i]).
pub const Army = struct {
    /// The hero whose bonuses the squad gets; null for a guard.  EndBattle
    /// writes its experience, quests and inventory.
    hero: ?*hero_mod.Hero = null,
    /// iExp of a guard (dcHero): its guard.var index, -index of a
    /// defender.var guard; 0 when it drops no loot.
    guard: i32 = 0,
    /// ipName of a guard: the karma its killer loses.
    guard_karma: i32 = 0,
    /// CheckHeroBonus(999): the initiative (iTask of a guard).
    initiative: i32 = 0,
    player: i32 = 0,
    /// arPlayer[BF_Player[i]].Gem: the crystals the spells cost.
    gem: i32 = 0,
    /// LifeMod[i] in percent.
    life_mod: i32 = 100,
    /// ProvinceMoraleBonus (side 1 only).
    morale_bonus: i32 = 0,
    /// IsPlayed[i]: a human gives the orders.
    human: bool = false,
    /// arPlayer[ipPlayer].iKarma when the squad belongs to a player
    /// (ipPlayer > 0), else null.
    player_karma: ?i32 = null,
    squad: [max_squad]?unit_mod.Unit = @splat(null),
};

/// What the screen shows of a step.
pub const Anim = union(enum) {
    /// G_Obj->Move along TruePath (cells from the start).
    move: struct { who: Ref, cells: [64][2]i8, n: u8, step: i32 },
    /// AnimAttack: `striker` lunges at `target`, then the hit shows and
    /// the target dies when `life` is 0.  `notes` are the clEffQue of that
    /// blow (the damage number, the berserk text, the crush text); the
    /// scene plays them when the blow lands.
    melee: struct { striker: Ref, target: Ref, hit: i32, life: i32, trample: ?[2]i32 = null, notes: [max_notes]Note = @splat(.{}), n_notes: u8 = 0 },
    /// AnimShoot: a missile from `shooter` to `target`.  `notes` play when
    /// the missile hits (the berserk text of a shot).
    shoot: struct { shooter: Ref, target: Ref, hit: i32, life: i32, missile: i32, sound_shoot: i32, sound_hit: i32, wait: i32, speed: i32, notes: [max_notes]Note = @splat(.{}), n_notes: u8 = 0 },
    /// AnimNumber over the unit.  `notes` come with it (the berserk of a
    /// side target of a circular attack).
    number: struct { who: Ref, value: i32, color: [3]u8, sound_unit: u32 = 0, notes: [max_notes]Note = @splat(.{}), n_notes: u8 = 0 },
    /// AnimText (phrase) over the unit.
    text: struct { who: Ref, phrase: u32, color: [3]u8, sound_unit: u32 = 0 },
    /// AnimDeath outside an attack (poison, spells).
    death: Ref,
    /// A spell: its sound and, for the computer side, AnimText of its
    /// name over the caster.
    cast: struct { who: Ref, spell: i32, name: bool = true },
    /// The unit stands on its cell at once (a summon, a raised corpse, a
    /// teleport).  A summon also rises from below (`G_Raise`) under the
    /// purple stars of `Make_Stars`; a corpse spell and a teleport do not.
    appear: struct { who: Ref, rise: bool = false },
    /// ChangeSide: the picture of `from` now belongs to `to`.
    swap: struct { from: Ref, to: Ref },
    /// bReincarnate: the unit rises after its death animation (the
    /// stars of Make_Stars), no corpse stays.
    revive: Ref,
    /// Replacement shots have no missile: sound, stars and a binding image.
    binding: struct { shooter: Ref, target: Ref, stone: bool, success: bool },
};

pub const col_damage: [3]u8 = .{ 220, 50, 5 };
pub const col_poison: [3]u8 = .{ 200, 50, 40 };
pub const col_heal: [3]u8 = .{ 60, 230, 20 };
pub const col_soul: [3]u8 = .{ 20, 220, 50 };
/// wavSoul in unit_sound.dat (Prototype.cpp).
pub const sound_soul = 130;
pub const col_panic: [3]u8 = .{ 220, 50, 20 };
pub const col_berserk: [3]u8 = .{ 250, 150, 20 };
/// Phrases: 1 "Не двигается", 2 "Паника", 3 "Берсерк".
pub const ph_stays = 1;
pub const ph_flees = 2;
pub const ph_berserk = 3;
/// AnimText of GoMarch (67), corpse eating (68) and gathering ammo (262).
pub const ph_march = 67;
pub const ph_eat = 68;
pub const ph_gather = 262;
/// COLOR_2 and COLOR_3 of the texts, the numbers of eating and gathering.
pub const col_text: [3]u8 = .{ 220, 220, 220 };
pub const col_gather: [3]u8 = .{ 220, 220, 20 };
pub const col_eat: [3]u8 = .{ 50, 250, 0 };
pub const col_ammo: [3]u8 = .{ 250, 250, 50 };
/// The vampire number of a blow (HitEffect, ability 68).
pub const col_drain: [3]u8 = .{ 20, 220, 20 };
/// The crush text (69) and the circular-attack text (70).
pub const col_crush: [3]u8 = .{ 220, 220, 50 };
pub const col_round: [3]u8 = .{ 220, 220, 70 };
pub const ph_crush = 69;
pub const ph_round = 70;
/// The broken weapon (371) and the broken armour (372).
pub const ph_weapon = 371;
pub const ph_armor = 372;
pub const col_break: [3]u8 = .{ 220, 150, 20 };
/// unit_sound.dat (Globals.h, Prototype.cpp).
pub const sound_root = 4;
pub const sound_eat = 6;
pub const sound_armor = 9;
pub const sound_weapon = 10;
pub const sound_heal = 13;
pub const sound_round = 25;
pub const sound_berserk = 82;
pub const sound_march = 83;
pub const sound_gather = 84;
pub const sound_crush = 125;

/// One floating number or phrase of a blow (clEffQue of AnimAttack).
pub const Note = struct {
    who: Ref = .{ .side = 0, .slot = 0 },
    /// A phrase index, or 0 when `value` is shown instead.
    phrase: u32 = 0,
    value: i32 = 0,
    color: [3]u8 = .{ 0, 0, 0 },
    /// unit_sound.dat; 0 plays nothing (wavSound of clEffQue).
    sound_unit: u32 = 0,
    /// iDelay of clEffQue, in ticks after the blow lands.
    delay: u8 = 0,
};

/// How many notes one blow carries (the damage, the berserk, the crush
/// and the circular attack, plus a spare).
pub const max_notes = 8;

/// What one blow shows, gathered while the blow is resolved.
pub const BlowFx = struct {
    notes: [max_notes]Note = @splat(.{}),
    n: u8 = 0,
    /// Vampire of HitEffect: the blow shows one sum, not one number a hit.
    drain: i32 = 0,
    /// bCanRound: only the main target of a blow adds to `drain`.
    round: bool = false,
    /// The delay of the berserk text: 20 for the main blow (AddAttackEnemy
    /// Effect(2,20)), 15 for a side target of a circular attack.
    berserk_delay: u8 = 20,

    fn note(self: *BlowFx, who: Ref, phrase: u32, value: i32, color: [3]u8, sound_unit: u32, delay: u8) void {
        if (self.n >= max_notes) return;
        self.notes[self.n] = .{ .who = who, .phrase = phrase, .value = value, .color = color, .sound_unit = sound_unit, .delay = delay };
        self.n += 1;
    }

    /// The berserk of a shot or of a side target comes before its number.
    fn prefix(self: *const BlowFx, a: *Anim) void {
        switch (a.*) {
            inline .number, .shoot => |*step| {
                step.notes = self.notes;
                step.n_notes = self.n;
            },
            else => {},
        }
    }
};

/// Field[i][j] of the map functions.
pub const MapCell = struct { land: i32 = 0, pass: bool = false, shoot: u8 = 0 };

/// GetMoraleBonus: the percent the morale adds to attacks.
pub fn moraleBonus(m_in: i32) i32 {
    var m = m_in;
    var result: i32 = 0;
    if (m < 6) result = m * 10 - 60;
    if (m > 15) {
        m -= 15;
        var bonus: i32 = 1;
        while (m >= bonus) {
            result += 1;
            m -= bonus;
            bonus += 1;
        }
        result *= 5;
    }
    return result;
}

/// Random(Max) of the game's rand() stream; 0 for Max <= 0.
pub fn rnd(r: *Rand, max: i32) i32 {
    if (max <= 0) return 0;
    return @intCast(r.random(@intCast(max)));
}

/// GetHit with bSimulate: no roll; a blow at or below the defence hits
/// for 1 when it falls short by less than 5.
pub fn simHit(att_in: i32, def_in: i32) i32 {
    const def = @max(def_in, 0);
    const att = @max(att_in, 1);
    if (def >= att) return if (def - att < 5) 1 else 0;
    return att - def;
}

/// GetHit: one blow of `att` against `def_in`.
pub fn getHit(r: *Rand, att_in: i32, def_in: i32) i32 {
    const def = @max(def_in, 0);
    var add: i32 = 1;
    var max: i32 = 3;
    if (att_in > 4) {
        add = @divTrunc(att_in, 5);
        max = add * 2 + 1;
    }
    var att = att_in + add - rnd(r, max);
    if (att < 1) att = 1;
    if (def >= att) {
        if (def - att < 10 and rnd(r, 20 + att - def) >= 10) return 1;
        return 0;
    }
    return att - def;
}

/// The six neighbours in the order of the map functions.
pub fn neighbours(x: i32, y: i32) [6][2]i32 {
    const p = @rem(y, 2);
    return .{ .{ x - 1 + p, y - 1 }, .{ x + p, y - 1 }, .{ x - 1 + p, y + 1 }, .{ x + p, y + 1 }, .{ x - 1, y }, .{ x + 1, y } };
}

pub fn inside(x: i32, y: i32) bool {
    return x >= 0 and y >= 0 and x < size and y < size;
}

pub const Outcome = struct {
    /// InitBattle's result: 1 side 0 won, 0 side 1 won, -1 both fell,
    /// -2 the time ran out.
    result: i32,
    gold: i32 = 0,
    gem: i32 = 0,
    exp: [max_squad]i32 = @splat(0),
    /// The winner's karma change (a guard's ipName).
    karma: i32 = 0,
    /// Reward[i]: the medal EndBattle rolled for squad slot i.
    medals: [max_squad]i32 = @splat(0),
    /// Loot[9]: (item, durability) as EndBattleScreen shows them.
    loot: [max_loot][2]i32 = @splat(.{ 0, 0 }),
    /// The loot TakeLoot found no room for (WindowTakeLoot is not ported).
    loot_left: [max_loot][2]i32 = @splat(.{ 0, 0 }),
};

/// Loot[9] of EndBattle.
pub const max_loot = 9;

/// What EndBattle reads outside the battle.
pub const EndCtx = struct {
    /// pFirstProvince: the sites of quest type 8.
    provinces: []const province_mod.Province = &.{},
    /// arDifficulty[..].iLoot of a human winner; null for the computer.
    loot_pct: ?i32 = null,
    /// GetBldParam1(41) of the winner's castle.
    trophy_bonus: i32 = 0,
    /// srand(ipProvince + Site * 10 + iSeed / 9) of bStaticLoot.
    static_loot_seed: ?u32 = null,
};

pub const Battle = struct {
    gpa: std.mem.Allocator,
    pr: *const proto.Proto,
    r: *Rand,
    field: Field,
    armies: [2]Army,
    units: [2][n_slots]Fighter = undefined,
    castle: bool = false,
    /// AA: the side whose turn it is.
    side: u1 = 0,
    /// iTurn: side turns so far.
    turn: i32 = 0,
    max_duration: i32 = 40,
    /// iBattleSpeed of the profile.
    battle_speed: i32 = 0,
    /// ActiveUnit.
    active: ?Ref = null,
    map: [size][size]MapCell = @splat(@splat(.{})),
    /// bFirstMove and CanPass of the map functions.
    first_move: bool = false,
    can_pass: [3]i32 = .{ 0, 0, 0 },
    anims: std.ArrayList(Anim) = .empty,
    // StepPath / FlyPath state.
    min_weight: i32 = 200,
    min_stam: i32 = 200,
    stack: [64][2]i8 = undefined,
    depth: usize = 0,
    best: [64][2]i8 = undefined,
    best_n: usize = 0,
    found: bool = false,
    /// BattleAI.cpp state of the side's turn.
    ai: battle_ai.State = .{},
    /// bSimulate: CompareStrength1's battle (battle_sim.zig).
    simulate: bool = false,
    /// buttCrush, buttRoundAtt and buttDoubleShot as the AI presses them.
    crush: bool = false,
    round_attack: bool = false,
    double_shot: bool = false,
    /// The blow being resolved; null outside one.  TakeDamage and HitEffect
    /// add to it what the screen shows of the blow.
    fx: ?*BlowFx = null,
    /// Spells[side][i]: (spell index, still castable this battle).
    book: [2][spellbook_size][2]i32 = @splat(@splat(.{ 0, 0 })),
    /// SideCanCast: 0 no spells, 1 no sacrifices, 2 all.
    can_cast: [2]i32 = .{ 2, 2 },
    /// The side's crystals during the battle, GemSpent and the karma the
    /// spells add.
    gem: [2]i32 = .{ 0, 0 },
    gem_spent: [2]i32 = .{ 0, 0 },
    karma: [2]i32 = .{ 0, 0 },
    /// bArchmage: the hero may cast once more this turn.
    archmage: bool = false,
    /// Gomunculus: the unit a corpse or tile spell just raised.
    gomunculus: ?Ref = null,
    /// SummonExp, SummonSurv, SummonWork: what the dead summons of a side
    /// give EndBattle.
    summon_exp: [2]i32 = .{ 0, 0 },
    summon_surv: [2]i32 = .{ 0, 0 },
    summon_work: [2]i32 = .{ 0, 0 },
    /// Heroes[side]->GetSquadMorale(i), counted from armies[side].squad.
    squad_morale: [2][max_squad]i32 = @splat(@splat(0)),

    /// InitBattle's unit placement: the squads stand where their units
    /// were put (side 1 mirrored), slot 0 of a hero is the hero.
    pub fn init(gpa: std.mem.Allocator, pr: *const proto.Proto, r: *Rand, field: Field, armies: [2]Army) Battle {
        var b: Battle = .{ .gpa = gpa, .pr = pr, .r = r, .field = field, .armies = armies };
        for (0..2) |s| {
            for (&b.units[s]) |*f| f.* = .{ .unit = .{ .type = 0, .life = 0, .exp = 0 } };
            b.countSquadMorale(@intCast(s));
            b.place(@intCast(s));
            b.gem[s] = armies[s].gem;
            // Heroes[Sd]->arSpell[1..SPELLBOOK_SIZE], packed in order.
            if (armies[s].hero) |h| {
                var n: usize = 0;
                for (1..spellbook_size + 1) |i| {
                    const sp = h.spell(i);
                    if (sp <= 0 or sp >= pr.spells.len) continue;
                    b.book[s][n] = .{ sp, 1 };
                    n += 1;
                }
            }
        }
        return b;
    }

    /// GetSquadMorale for every slot of side `s`.
    pub fn countSquadMorale(b: *Battle, s: u1) void {
        const a = &b.armies[s];
        for (0..max_squad) |i| {
            b.squad_morale[s][i] = hero_mod.squadMorale(b.pr, &a.squad, i, a.player_karma, a.hero);
        }
    }

    /// GetHit of the battle: without the roll when simulated.
    pub fn rollHit(b: *Battle, att: i32, def: i32) i32 {
        return if (b.simulate) simHit(att, def) else getHit(b.r, att, def);
    }

    pub fn deinit(b: *Battle) void {
        b.anims.deinit(b.gpa);
    }

    pub fn fighter(b: *Battle, ref: Ref) *Fighter {
        return &b.units[ref.side][ref.slot];
    }

    pub fn emit(b: *Battle, a: Anim) void {
        b.anims.append(b.gpa, a) catch {};
    }

    /// UnitPlacement's first part: units on one cell are moved to a free
    /// cell of the first two columns, then BF_Unit::Init.
    fn place(b: *Battle, s: u1) void {
        const sq = &b.armies[s].squad;
        for (0..max_squad - 1) |i| {
            const ui = sq[i] orelse continue;
            if (ui.life <= 0) continue;
            for (i + 1..max_squad) |j| {
                const uj = sq[j] orelse continue;
                if (uj.life <= 0 or uj.x != ui.x or uj.y != ui.y) continue;
                for (0..2) |cx| for (0..size) |cy| {
                    const free = for (sq) |o| {
                        if (o) |u| if (u.life > 0 and u.x == cx and u.y == cy) break false;
                    } else true;
                    if (free) {
                        sq[j].?.x = @intCast(cx);
                        sq[j].?.y = @intCast(cy);
                    }
                };
            }
        }
        for (sq, 0..) |slot, i| {
            const u = slot orelse continue;
            if (u.life <= 0) continue;
            const f = &b.units[s][i];
            f.* = .{ .unit = u, .in_battle = true, .home = s, .num = @intCast(i) };
            f.hero_unit = i == 0 and b.armies[s].hero != null and b.armies[s].hero.?.class1 > 0;
            f.x = if (s == 1) 7 - std.math.clamp(u.x, 0, 7) else std.math.clamp(u.x, 0, 7);
            f.y = std.math.clamp(u.y, 0, 7);
            b.initFighter(.{ .side = s, .slot = @intCast(i) });
        }
        for (&b.units[s]) |*f| {
            if (f.in_battle) f.ammo = b.maxAmmo(f);
        }
    }

    /// BF_Unit::Init.
    fn initFighter(b: *Battle, ref: Ref) void {
        const f = b.fighter(ref);
        const a = &b.armies[ref.side];
        const ul = b.typeStat(f, stat.life);
        const pct = 100 - @divTrunc((ul - f.unit.life) * 100, @max(ul, 1));
        f.life = @divTrunc(f.unit.life * a.life_mod, 100) + @divTrunc(b.heroUnitBonus(ref.side, stat.life) * pct, 100);
        b.setPass(f, false);
        f.morale = b.typeStat(f, stat.morale);
        f.stamina = b.typeStat(f, stat.stamina) + b.heroUnitBonus(ref.side, stat.stamina);
        f.speed = b.typeStat(f, stat.speed);
        if (b.skill(f, ab.rooted) > 0) f.speed = 0;
        f.can_act = true;
        f.march = true;
        if (b.skill(f, ab.fearless) == 0) {
            f.morale += @divTrunc(f.unit.morale, 2) + b.heroUnitBonus(ref.side, stat.morale) + b.squad_morale[ref.side][ref.slot];
            if (ref.side == 1) f.morale += a.morale_bonus;
        }
        if (f.morale < 1) f.morale = 1;
        f.life = @min(f.life, b.maxLife(f));
        // Ability 97: the unit rises once, as under the spell 59.
        if (b.skill(f, ab.reincarnation) > 0) f.addEffect(.{ .kind = eff_reincarnate, .power = 1, .time = 0, .spell = 59, .visible = true });
    }

    // -- stats --------------------------------------------------------

    pub fn sideOf(b: *const Battle, f: *const Fighter) u1 {
        const p = @intFromPtr(f);
        const s1 = @intFromPtr(&b.units[1][0]);
        return if (p >= s1) 1 else 0;
    }

    fn refOf(b: *const Battle, f: *const Fighter) Ref {
        const side = b.sideOf(f);
        return .{ .side = side, .slot = @intCast((@intFromPtr(f) - @intFromPtr(&b.units[side][0])) / @sizeOf(Fighter)) };
    }

    /// The hero's CheckUnitBonus for the squad (0 for a guard).
    fn heroUnitBonus(b: *const Battle, s: u1, index: i32) i32 {
        const h = b.armies[s].hero orelse return 0;
        return h.unitBonus(b.pr, index);
    }

    /// The hero bonus applies while the hero (slot 0) lives.
    fn squadBonus(b: *const Battle, f: *const Fighter, index: i32) i32 {
        if (f.summoned) return 0;
        const s = b.sideOf(f);
        if (b.units[s][0].life <= 0) return 0;
        return b.heroUnitBonus(s, index);
    }

    /// pUnit->CheckUpg: the unit's upgrades and medals and, for the
    /// hero's own unit, CheckHeroBonus.
    fn upg(b: *const Battle, f: *const Fighter, index: i32) i32 {
        var r = f.unit.checkUpg(b.pr, index);
        r += b.heroBonus(f, index);
        return r;
    }

    /// pHero->CheckHeroBonus of the hero's own unit, else 0.
    pub fn heroBonus(b: *const Battle, f: *const Fighter, index: i32) i32 {
        if (!f.hero_unit) return 0;
        const h = b.armies[f.home].hero orelse return 0;
        return h.heroBonus(b.pr, index);
    }

    pub fn utype(b: *const Battle, f: *const Fighter) *const proto.UnitType {
        return &b.pr.units[@intCast(f.unit.type)];
    }

    /// dcUnit::Get*: the type's value, the upgrades and the abilities.
    pub fn typeStat(b: *const Battle, f: *const Fighter, index: i32) i32 {
        const t = b.utype(f);
        const base: i32 = switch (index) {
            stat.life => t.life,
            stat.attack => t.attack,
            stat.counter => t.counter_attack,
            stat.defence => t.defence,
            stat.ranged_defence => t.ranged_defence,
            stat.resist => t.resist,
            stat.speed => t.speed,
            stat.ranged => t.ranged_attack,
            stat.range => t.shooting_range,
            stat.ammo => t.ammo,
            stat.stamina => t.stamina,
            stat.morale => t.morale,
            else => 0,
        };
        return base + b.upg(f, index) + f.unit.checkAbility(b.pr, index);
    }

    /// CheckEffect.
    pub fn effect(f: *const Fighter, index: i32) i32 {
        var r: i32 = 0;
        for (f.effectList()) |e| {
            if (e.kind == index) r += e.power;
        }
        return r;
    }

    /// CheckSkill.
    pub fn skill(b: *const Battle, f: *const Fighter, index: i32) i32 {
        if (index >= ab.knowledge_first and index <= ab.knowledge_first + 2 and b.skill(f, ab.flying) > 0) return 0;
        var r = effect(f, index);
        if (r <= 0 or index != ab.berserk) r += f.unit.checkAbility(b.pr, index) + b.upg(f, index);
        r += b.squadBonus(f, index);
        return r;
    }

    pub fn bfo(b: *const Battle, land: i32) proto.BfObject {
        if (land < 0 or land >= b.pr.bf_objects.len) return .{ .name = "", .move_cost = 0, .stamina_cost = 0, .counter_attack = 0, .defence = 0, .ranged_defence = 0, .shooting_range = 0 };
        return b.pr.bf_objects[@intCast(land)];
    }

    fn landOf(b: *const Battle, f: *const Fighter) i32 {
        return b.field.cells[@intCast(f.x)][@intCast(f.y)].land;
    }

    /// GetLife: the maximum life.
    pub fn maxLife(b: *const Battle, f: *const Fighter) i32 {
        const s = b.sideOf(f);
        var r = b.typeStat(f, stat.life) + (if (f.summoned) 0 else b.heroUnitBonus(s, stat.life));
        r = @divTrunc(r * b.armies[s].life_mod, 100);
        return @max(r, 1);
    }

    /// The wound, fatigue and morale part of the attack getters.
    fn tail(b: *const Battle, f: *const Fighter, v: i32) i32 {
        var r = v * 100;
        const ml = b.maxLife(f);
        if (f.life * 2 < ml and b.skill(f, ab.lesser_undead) == 0) {
            r -= @divTrunc(r * @divTrunc((@divTrunc(ml, 2) - f.life) * 100, ml), 100);
        }
        if (f.stamina < 6) r -= @divTrunc(r * (6 - f.stamina), 10);
        r = @divTrunc(r, 100);
        r += @divTrunc(r * moraleBonus(f.morale), 100);
        return @max(r, 1);
    }

    /// Terrain knowledge (abilities 32..34 on forest, hill, swamp).
    fn knowledge(b: *const Battle, f: *const Fighter, div: i32) i32 {
        var r: i32 = 0;
        const land = b.landOf(f);
        for (0..3) |i| {
            const k = b.skill(f, ab.knowledge_first + @as(i32, @intCast(i)));
            if (land == 2 + @as(i32, @intCast(i)) and k != 0) r += @divTrunc(k - @rem(k, div), div);
        }
        return r;
    }

    pub fn attack(b: *const Battle, f: *const Fighter) i32 {
        if (b.skill(f, ab.no_combat) != 0) return 0;
        const r = b.typeStat(f, stat.attack) + effect(f, stat.attack) + b.squadBonus(f, stat.attack);
        return b.tail(f, r);
    }

    pub fn counterAttack(b: *const Battle, f: *const Fighter) i32 {
        if (b.skill(f, ab.no_combat) != 0) return 0;
        var r = b.typeStat(f, stat.counter) + effect(f, stat.counter) + b.squadBonus(f, stat.counter);
        if (f.life > 0) {
            if (b.skill(f, ab.flying) == 0) r += b.bfo(b.landOf(f)).counter_attack;
            r += b.knowledge(f, 8);
        }
        return b.tail(f, r);
    }

    pub fn defence(b: *const Battle, f: *const Fighter) i32 {
        var r = b.typeStat(f, stat.defence) + effect(f, stat.defence) + b.squadBonus(f, stat.defence);
        if (f.life > 0) {
            if (b.skill(f, ab.flying) == 0) r += b.bfo(b.landOf(f)).defence;
            r += b.knowledge(f, 4);
        }
        if (f.stamina == 0) r = @divTrunc(r, 2);
        return @max(r, 0);
    }

    pub fn rangedDefence(b: *const Battle, f: *const Fighter) i32 {
        var r = b.typeStat(f, stat.ranged_defence) + effect(f, stat.ranged_defence) + b.squadBonus(f, stat.ranged_defence);
        if (f.life > 0) {
            if (b.skill(f, ab.flying) == 0) {
                r += b.bfo(b.landOf(f)).ranged_defence;
                if (b.castle and f.x > 5) r += 3;
            }
            r += b.knowledge(f, 8);
        }
        if (f.stamina == 0) r = @divTrunc(r, 2);
        return @max(r, 0);
    }

    pub fn resist(b: *const Battle, f: *const Fighter) i32 {
        return @max(b.typeStat(f, stat.resist) + effect(f, stat.resist) + b.squadBonus(f, stat.resist), 0);
    }

    fn speedSum(b: *const Battle, f: *const Fighter) i32 {
        var r = b.typeStat(f, stat.speed) + effect(f, stat.speed) + b.squadBonus(f, stat.speed);
        if (f.stamina < 5 and r > 1) r -= 1;
        if (f.stamina < 3 and r > 1) r -= 1;
        return r;
    }

    /// GetSpeed: the move points of a turn.
    pub fn maxSpeed(b: *const Battle, f: *const Fighter) i32 {
        if (b.typeStat(f, stat.speed) == 0) return 0;
        return @max(b.speedSum(f), 1);
    }

    /// GetFatigure: 1 when the speed would be below 1.
    pub fn fatigue(b: *const Battle, f: *const Fighter) i32 {
        return if (b.speedSum(f) < 1) 1 else 0;
    }

    fn shooterBonus(b: *const Battle, f: *const Fighter, index: i32) i32 {
        if (b.skill(f, ab.healer) != 0 or b.utype(f).unit_class != 2) return 0;
        return b.squadBonus(f, index);
    }

    pub fn rangedAttack(b: *const Battle, f: *const Fighter) i32 {
        var r = b.typeStat(f, stat.ranged);
        if (r == 0) return 0;
        if (b.skill(f, ab.flying) == 0 and f.life > 0 and b.castle and f.x > 5) r += 1;
        r += effect(f, stat.ranged) + b.shooterBonus(f, stat.ranged);
        return b.tail(f, r);
    }

    pub fn shootingRange(b: *const Battle, f: *const Fighter) i32 {
        var r = b.typeStat(f, stat.range);
        if (r > 1) {
            if (b.skill(f, ab.flying) == 0 and f.life > 0) {
                r += b.bfo(b.landOf(f)).shooting_range;
                if (b.castle and f.x > 5) r += 1;
            }
            r += effect(f, stat.range) + b.shooterBonus(f, stat.range);
        }
        return @max(r, 0);
    }

    /// CheckCaster: the unit casts a spell by ability 2000 + i.
    pub fn isCaster(b: *const Battle, f: *const Fighter) bool {
        for (1..b.pr.spells.len) |i| {
            if (b.skill(f, 2000 + @as(i32, @intCast(i))) != 0) return true;
        }
        return false;
    }

    pub fn maxAmmo(b: *const Battle, f: *const Fighter) i32 {
        if (b.typeStat(f, stat.ranged) < 1 and b.skill(f, ab.healer) == 0 and b.skill(f, 46) == 0 and b.skill(f, 47) == 0 and !b.isCaster(f)) return 0;
        var r = b.typeStat(f, stat.ammo);
        if (r > 0) {
            r += effect(f, stat.ammo);
            if (r > 1 and b.typeStat(f, stat.ranged) > 0) r += b.shooterBonus(f, stat.ammo);
        }
        return @max(r, 0);
    }

    pub fn maxStamina(b: *const Battle, f: *const Fighter) i32 {
        return @max(b.typeStat(f, stat.stamina) + b.squadBonus(f, stat.stamina), 0);
    }

    pub fn maxMorale(b: *const Battle, f: *const Fighter) i32 {
        var r = b.typeStat(f, stat.morale) + effect(f, stat.morale);
        if (b.skill(f, ab.fearless) == 0 and !f.summoned) {
            const s = b.sideOf(f);
            if (b.armies[s].hero != null) r += b.squadBonus(f, stat.morale);
            // Heroes[iSide]->GetSquadMorale(iNumm): a guard is a dcHero
            // too; a charmed unit keeps its old slot number.
            r += b.squad_morale[s][f.num];
            if (s == 1) r += b.armies[s].morale_bonus;
        }
        return @max(r, 1);
    }

    pub fn unitKind(b: *const Battle, f: *const Fighter) i32 {
        return b.utype(f).unit_kind;
    }

    fn flies(b: *const Battle, f: *const Fighter) bool {
        return b.skill(f, ab.flying) > 0 or b.skill(f, ab.low_flying) > 0;
    }

    // -- morale and stamina -------------------------------------------

    pub fn changeMorale(b: *const Battle, f: *Fighter, n: i32) void {
        if (b.skill(f, ab.fearless) != 0) return;
        f.morale += n;
        if (f.morale <= 0) {
            f.fear = if (f.fear > 0) f.fear - f.morale * 10 else 50 - f.morale * 10;
            f.morale = 0;
        }
    }

    pub fn useStamina(b: *const Battle, f: *Fighter, n_in: i32) void {
        if (b.skill(f, ab.tireless) != 0) return;
        var n = n_in;
        if (f.stamina < n) {
            b.changeMorale(f, f.stamina - n);
            n = f.stamina;
        }
        f.stamina -= n;
        f.st.stamina_used += n;
    }

    // -- the field maps -----------------------------------------------

    pub fn setPass(b: *Battle, f: *const Fighter, v: bool) void {
        b.field.cells[@intCast(f.x)][@intCast(f.y)].pass = v;
    }

    pub fn grid(b: *const Battle, x: i32, y: i32) *const Cell {
        return &b.field.cells[@intCast(x)][@intCast(y)];
    }

    pub fn mc(b: *Battle, x: i32, y: i32) *MapCell {
        return &b.map[@intCast(x)][@intCast(y)];
    }

    fn canPassLand(b: *const Battle, land: i32) bool {
        return b.can_pass[0] == land or b.can_pass[1] == land or b.can_pass[2] == land;
    }

    /// GetMoveCost / GetStamCost.
    pub fn moveCost(b: *const Battle, x: i32, y: i32) i32 {
        const land = b.grid(x, y).land;
        return if (b.canPassLand(land)) 1 else b.bfo(land).move_cost;
    }

    fn stamCost(b: *const Battle, x: i32, y: i32) i32 {
        const land = b.grid(x, y).land;
        return if (b.canPassLand(land)) 0 else b.bfo(land).stamina_cost;
    }

    pub fn clearMap(b: *Battle) void {
        b.map = @splat(@splat(.{}));
    }

    fn tryMap(b: *Battle, x: i32, y: i32, weight: i32, stam: i32, speed: i32) bool {
        if (!inside(x, y) or !b.grid(x, y).pass) return false;
        const m = b.mc(x, y);
        const cost = b.moveCost(x, y);
        if (!((m.land == 0 and (cost <= speed or (weight == 0 and b.first_move))) or weight + cost < m.land)) return false;
        const o = b.bfo(b.grid(x, y).land);
        return o.stamina_cost <= stam and o.move_cost != 0;
    }

    /// StepMap: the cells a walking unit reaches this turn.
    pub fn stepMap(b: *Battle, x: i32, y: i32, weight_in: i32, speed_in: i32, stam_cost_in: i32, stam_left: i32) void {
        var weight = weight_in;
        var speed = speed_in;
        var stam_cost = stam_cost_in;
        if (weight < 0) {
            weight = 0;
        } else {
            weight += b.moveCost(x, y);
            speed -= b.moveCost(x, y);
            stam_cost += b.stamCost(x, y);
            b.mc(x, y).* = .{ .land = weight, .pass = true, .shoot = b.mc(x, y).shoot };
            if (speed < 1) return;
        }
        for (neighbours(x, y)) |n| {
            if (b.tryMap(n[0], n[1], weight, stam_left - stam_cost, speed)) b.stepMap(n[0], n[1], weight, speed, stam_cost, stam_left);
        }
    }

    fn flyTry(b: *Battle, x: i32, y: i32, weight: i32, cross: bool) bool {
        if (!inside(x, y)) return false;
        if (b.bfo(b.grid(x, y).land).move_cost == 0 and !cross) return false;
        const m = b.mc(x, y);
        return m.land == 0 or weight + 1 < m.land;
    }

    /// FlyMap: every cell costs 1; `cross` also crosses impassable land.
    pub fn flyMap(b: *Battle, x: i32, y: i32, weight_in: i32, speed_in: i32, cross: bool) void {
        var weight = weight_in;
        var speed = speed_in;
        if (weight < 0) {
            weight = 0;
        } else {
            weight += 1;
            speed -= 1;
            b.mc(x, y).land = weight;
            b.mc(x, y).pass = true;
            if (speed < 1) return;
        }
        for (neighbours(x, y)) |n| {
            if (b.flyTry(n[0], n[1], weight, cross)) b.flyMap(n[0], n[1], weight, speed, cross);
        }
    }

    fn shootTry(b: *Battle, x: i32, y: i32, weight: i32) bool {
        if (!inside(x, y)) return false;
        const m = b.mc(x, y);
        return m.shoot < 1 or (m.shoot > 1 and m.land > weight);
    }

    /// ShootMap: shoot 1 next to the unit, 2 further within range.
    pub fn shootMap(b: *Battle, x: i32, y: i32, weight_in: i32, range_in: i32) void {
        var weight = weight_in;
        var range = range_in;
        if (weight < 0) {
            weight = 0;
            b.mc(x, y).shoot = 1;
        } else {
            weight += 1;
            range -= 1;
            b.mc(x, y).shoot = if (weight == 1) 1 else 2;
            b.mc(x, y).land = weight;
            if (range == 0) return;
        }
        for (neighbours(x, y)) |n| {
            if (b.shootTry(n[0], n[1], weight)) b.shootMap(n[0], n[1], weight, range);
        }
    }

    fn found_path(b: *Battle, weight: i32, stam: i32) void {
        if (b.found and weight >= b.min_weight) return;
        @memcpy(b.best[0..b.depth], b.stack[0..b.depth]);
        b.best_n = b.depth;
        b.found = true;
        b.min_weight = weight;
        b.min_stam = stam;
    }

    fn tryPath(b: *Battle, x: i32, y: i32, weight: i32, stam: i32) bool {
        if (!inside(x, y) or !b.grid(x, y).pass) return false;
        const m = b.mc(x, y);
        if (!(m.land == 0 or weight + b.moveCost(x, y) < m.land)) return false;
        return b.stamCost(x, y) <= stam and b.bfo(b.grid(x, y).land).move_cost != 0 and m.pass;
    }

    /// StepPath: the cheapest way to (x1, y1) through the move map.
    fn stepPath(b: *Battle, x: i32, y: i32, weight_in: i32, x1: i32, y1: i32, stam_in: i32, stam_left: i32) void {
        const weight = weight_in + b.moveCost(x, y);
        if (weight > b.min_weight or b.depth == b.stack.len) return;
        const stam = stam_in + b.stamCost(x, y);
        b.mc(x, y).land = weight;
        b.stack[b.depth] = .{ @intCast(x), @intCast(y) };
        b.depth += 1;
        defer b.depth -= 1;
        if (x == x1 and y == y1) return b.found_path(weight, stam);
        for (neighbours(x, y)) |n| {
            if (b.tryPath(n[0], n[1], weight, stam_left - stam)) b.stepPath(n[0], n[1], weight, x1, y1, stam, stam_left);
        }
    }

    fn flyTryPath(b: *Battle, x: i32, y: i32, weight: i32) bool {
        if (!inside(x, y)) return false;
        const m = b.mc(x, y);
        return m.pass and (m.land == 0 or weight + 1 < m.land);
    }

    fn flyPath(b: *Battle, x: i32, y: i32, weight_in: i32, x1: i32, y1: i32) void {
        const weight = weight_in + 1;
        if (weight > b.min_weight or b.depth == b.stack.len) return;
        b.mc(x, y).land = weight;
        b.stack[b.depth] = .{ @intCast(x), @intCast(y) };
        b.depth += 1;
        defer b.depth -= 1;
        if (x == x1 and y == y1) return b.found_path(weight, 0);
        for (neighbours(x, y)) |n| {
            if (b.flyTryPath(n[0], n[1], weight)) b.flyPath(n[0], n[1], weight, x1, y1);
        }
    }

    /// The maps of SelectUnit for `ref`: the shot map, then the move map.
    pub fn select(b: *Battle, ref: Ref) void {
        b.active = ref;
        const f = b.fighter(ref);
        b.clearMap();
        if (b.shootingRange(f) != 0 and f.ammo != 0) {
            b.shootMap(f.x, f.y, -1, b.shootingRange(f));
            for (&b.map) |*col| for (col) |*m| {
                m.land = 0;
            };
        }
        b.first_move = f.speed >= b.maxSpeed(f);
        if (!f.march and f.speed <= b.maxSpeed(f)) b.first_move = false;
        b.can_pass = .{ 0, 0, 0 };
        for (0..3) |i| {
            if (b.skill(f, ab.knowledge_first + @as(i32, @intCast(i))) != 0) b.can_pass[i] = 2 + @as(i32, @intCast(i));
        }
        if (f.speed > 0 and f.stamina > 0) {
            if (b.skill(f, ab.flying) != 0) {
                b.flyMap(f.x, f.y, -1, f.speed, true);
            } else if (b.skill(f, ab.low_flying) != 0) {
                b.flyMap(f.x, f.y, -1, f.speed, false);
            } else b.stepMap(f.x, f.y, -1, f.speed, 0, f.stamina - b.fatigue(f));
        }
    }

    /// Is an enemy of `f` next to it (FindNeighbour(1 - side) > 0)?
    fn enemyNear(b: *const Battle, f: *const Fighter) bool {
        return b.neighbourCount(f, 1 - b.sideOf(f)) > 0;
    }

    fn neighbourCount(b: *const Battle, f: *const Fighter, s: u1) usize {
        var n: usize = 0;
        for (&b.units[s]) |*o| {
            if (o.life == 0) continue;
            for (neighbours(f.x, f.y)) |c| {
                if (o.x == c[0] and o.y == c[1]) n += 1;
            }
        }
        return n;
    }

    /// The unit at a cell (alive), of side `s`.
    pub fn unitAt(b: *const Battle, s: u1, x: i32, y: i32) ?Ref {
        for (&b.units[s], 0..) |*f, k| {
            if (f.life != 0 and f.x == x and f.y == y) return .{ .side = s, .slot = @intCast(k) };
        }
        return null;
    }

    // -- actions ------------------------------------------------------

    /// BF_Unit::Move.
    pub fn moveTo(b: *Battle, ref: Ref, x1: i32, y1: i32) void {
        const f = b.fighter(ref);
        b.found = false;
        b.min_weight = 200;
        b.min_stam = 200;
        b.depth = 0;
        for (&b.map) |*col| for (col) |*m| {
            m.land = 0;
        };
        if (b.flies(f)) {
            b.flyPath(f.x, f.y, -1, x1, y1);
        } else {
            const cur = @max(f.stamina - b.fatigue(f), 1);
            b.stepPath(f.x, f.y, -b.moveCost(f.x, f.y), x1, y1, -b.stamCost(f.x, f.y), cur);
        }
        if (!b.found) return;
        b.setPass(f, true);
        var a: Anim = .{ .move = .{ .who = ref, .cells = undefined, .n = @intCast(b.best_n), .step = 2 + b.maxSpeed(f) + b.battle_speed } };
        @memcpy(a.move.cells[0..b.best_n], b.best[0..b.best_n]);
        b.emit(a);
        b.useStamina(f, b.min_stam + b.fatigue(f));
        f.x = x1;
        f.y = y1;
        f.speed = @max(f.speed - b.min_weight, 0);
        b.setPass(f, false);
        if (f.speed == 0 and !b.enemyNear(f)) f.can_act = false;
    }

    /// GetMeleeHit.
    pub fn meleeHit(b: *Battle, att_u: *const Fighter, def_u: *const Fighter, attacking: bool) i32 {
        var att = if (attacking) b.attack(att_u) else b.counterAttack(att_u);
        var def = b.defence(def_u);
        if (attacking and b.crush) att += @divTrunc(att, 2);
        const karma = b.utype(def_u).karma;
        if (b.skill(att_u, ab.smite_evil) > 0 and karma < 0) att += b.skill(att_u, ab.smite_evil) * -karma;
        if (b.skill(att_u, ab.smite_good) > 0 and karma > 0) att += b.skill(att_u, ab.smite_good) * karma;
        if (b.skill(att_u, ab.magic_strike) != 0) {
            def = b.resist(def_u);
        } else {
            if (b.skill(att_u, ab.armor_piercing) != 0) def = @divTrunc(def, 2);
            if (attacking and def_u.parry and def_u.stamina > 0 and !def_u.resting and b.skill(def_u, ab.immobile) == 0) def += b.skill(def_u, ab.parry);
            if (attacking) def -= b.skill(att_u, ab.precise_strike);
        }
        return b.rollHit(att, def);
    }

    /// GetRangedHit.
    pub fn rangedHit(b: *Battle, att_u: *const Fighter, def_u: *const Fighter) i32 {
        if (b.skill(att_u, ab.magic_shot) != 0) return b.rollHit(b.rangedAttack(att_u), b.resist(def_u) - b.skill(att_u, ab.energy_control));
        const rd = if (b.skill(att_u, ab.armor_piercing_shot) != 0) @divTrunc(b.rangedDefence(def_u), 2) else b.rangedDefence(def_u);
        var r = b.rollHit(b.rangedAttack(att_u), rd - b.skill(att_u, ab.precise_shot));
        const fire = b.skill(att_u, ab.fire_arrows);
        if (fire > b.resist(def_u)) r += fire - b.resist(def_u);
        return r;
    }

    /// The timed effect of a hit; only the first of a group is shown.
    fn timed(f: *Fighter, kind: i32, power: i32, time: i32, icon: i32, visible: bool) void {
        f.addEffect(.{ .kind = kind, .power = power, .time = time, .visible = visible, .ability = icon });
    }

    /// HitEffect; the circular attack itself is handled by roundAttack.
    /// `fx` collects what the blow shows (the vampire number, the stolen
    /// ammo, the roots); null drops it, as bShowAnim does.
    pub fn hitEffect(b: *Battle, a: *Fighter, d: *Fighter, hit: i32, fx: ?*BlowFx) void {
        if (b.skill(a, ab.poison) != 0 and b.skill(d, ab.poison_immunity) == 0) d.addEffect(.{ .kind = stat.life, .power = -b.skill(a, ab.poison), .time = poison_duration, .visible = true, .poison = true, .ability = upg_icon.poison });
        if (b.skill(a, ab.poison_immunity) == 0 and b.skill(d, ab.poison_flesh) != 0) timed(a, stat.life, -b.skill(d, ab.poison_flesh), poison_duration, upg_icon.poison, true);
        if (b.skill(a, ab.stun) != 0) b.useStamina(d, b.skill(a, ab.stun));
        if (b.skill(a, ab.intimidation) != 0) b.changeMorale(d, -b.skill(a, ab.intimidation));
        if (a == d) return;
        if (b.skill(a, ab.steal_ammo) != 0 and d.ammo > 0) {
            d.ammo = @max(d.ammo - b.skill(a, ab.steal_ammo), 0);
            // Make_Stars over the victim; the picture itself is not ported.
            if (fx) |f| f.note(b.refOf(d), 0, 0, .{ 150, 200, 50 }, 0, 0);
        }
        if (b.skill(a, ab.break_armor) != 0) timed(d, stat.defence, -1, b.skill(a, ab.break_armor), upg_icon.break_armor, true);
        const roots = b.skill(a, ab.roots);
        if (roots != 0 and d.life > hit and b.skill(d, ab.roots_immunity) == 0) {
            var found = false;
            for (d.effects[0..d.n_eff]) |*e| {
                if (e.ability == upg_icon.roots) {
                    e.time = @max(e.time, roots);
                    found = true;
                }
            }
            if (!found) timed(d, ab.rooted, 1, roots, upg_icon.roots, true);
            // The stars and the root picture stay with the visual effects.
            if (fx) |f| f.note(b.refOf(d), 0, 0, .{ 150, 200, 50 }, sound_root, 0);
        }
        if (b.skill(a, ab.break_weapon) != 0) {
            timed(d, stat.attack, -1, b.skill(a, ab.break_weapon), upg_icon.break_weapon, true);
            timed(d, stat.counter, -1, b.skill(a, ab.break_weapon), upg_icon.break_weapon, false);
        }
        if (b.skill(a, ab.cripple) != 0) {
            var found = false;
            for (d.effects[0..d.n_eff]) |*e| {
                if (e.ability == upg_icon.cripple) {
                    e.time = 2;
                    found = true;
                }
            }
            if (!found) timed(d, stat.speed, -1, 2, upg_icon.cripple, true);
        }
        const curse = b.skill(a, ab.curse) - b.resist(d);
        if (curse > 0 and b.skill(d, ab.fire_arrows) == 0) {
            var found = false;
            for (d.effects[0..d.n_eff]) |*e| {
                if (e.ability == upg_icon.curse) {
                    e.time = @max(e.time, curse);
                    found = true;
                }
            }
            if (!found) {
                for ([_]i32{ stat.attack, stat.defence, stat.ranged_defence, stat.resist }, 0..) |k, n| timed(d, k, -1, curse, upg_icon.curse, n == 0);
            }
        }
        b.hitSpells(a, d);
        const kind = b.unitKind(d);
        if (b.skill(a, ab.drain) != 0 and kind != kind_undead and kind != kind_construct and a.life < b.maxLife(a)) {
            const drain = @min(@divTrunc(hit * b.skill(a, ab.drain), 100), b.maxLife(a) - a.life);
            a.life += drain;
            // One number for the whole blow, so a circular attack adds up.
            if (fx) |f| {
                if (f.round) f.drain += drain;
            }
        }
    }

    /// BF_Unit::HitEffect: vulnerability, disease and ability 206.  The
    /// first two accept equal resistance; a spell strike requires less.
    fn hitSpells(b: *Battle, a: *Fighter, d: *Fighter) void {
        const caster = b.refOf(a);
        const target = b.refOf(d);
        if (b.skill(a, ab.vulnerability) > 0 and b.pr.spells[21].effects[0].duration >= b.resist(d) and b.skill(d, ab.fire_arrows) == 0) {
            battle_spell.underSpell(b, target, 21, caster);
        }
        const kind = b.unitKind(d);
        if (b.skill(a, ab.disease) > 0 and b.pr.spells[30].effects[0].duration >= b.resist(d) and b.skill(d, ab.fire_arrows) == 0 and kind != kind_undead and kind != kind_construct) {
            battle_spell.underSpell(b, target, 30, caster);
        }
        b.attackSpell(a, d, b.skill(a, ab.spell_strike));
    }

    /// HitEffect/ShootEffect: abilities 206/207 share the same spell gate.
    fn attackSpell(b: *Battle, a: *Fighter, d: *Fighter, spell: i32) void {
        if (spell <= 0 or spell >= b.pr.spells.len) return;
        const sp = &b.pr.spells[@intCast(spell)];
        if (sp.effects[0].duration <= b.resist(d)) return;
        for (sp.immune_kinds) |k| {
            if (k == b.unitKind(d)) return;
        }
        // Unlike ordinary spell targeting, ability 205 does not bypass
        // these immunities in HitEffect or ShootEffect.
        for (sp.anti_effects) |ability| {
            if (ability > 0 and b.skill(d, ability) > 0) return;
        }
        battle_spell.underSpell(b, b.refOf(d), spell, b.refOf(a));
    }

    /// TakeDamage: true when the unit dies.  A berserk shows its phrase and
    /// sound (wavBerz) only with the animation, as bGoBerz does.
    pub fn takeDamage(b: *Battle, ref: Ref, hit: i32, kind: u2) bool {
        const f = b.fighter(ref);
        switch (kind) {
            0 => f.st.melee_taken += hit,
            1 => f.st.range_taken += hit,
            2 => f.st.spell_taken += hit,
            3 => f.st.poison_taken += hit,
        }
        var k: usize = 0;
        while (k < f.n_eff) {
            if (f.effects[k].destroy_on_hit) f.removeEffect(k) else k += 1;
        }
        f.life -= hit;
        if (f.life <= 0) {
            f.life = 0;
            b.death(ref);
            return true;
        }
        if ((hit >= @divTrunc(b.maxLife(f), 4) or hit > 9) and kind != 3) {
            const bz = b.skill(f, ab.berserk);
            if (bz != 0 and effect(f, ab.berserk) == 0) {
                if (b.fx) |fx| fx.note(ref, ph_berserk, 0, col_berserk, sound_berserk, fx.berserk_delay);
                if (f.morale < b.maxMorale(f)) f.morale = b.maxMorale(f);
                f.addEffect(.{ .kind = stat.attack, .power = bz, .time = 0 });
                f.addEffect(.{ .kind = ab.berserk, .power = bz, .time = 0, .visible = true, .ability = upg_icon.berserk });
                f.addEffect(.{ .kind = ab.lesser_undead, .power = 1, .time = 0 });
                f.addEffect(.{ .kind = ab.fearless, .power = 1, .time = 0 });
            } else b.changeMorale(f, -1);
        }
        return false;
    }

    /// BF_Unit::Death: the summons' morale when the hero falls, the
    /// neighbours' morale, the effects (a charmed unit goes back, a
    /// polymorph ends), the corpse; a summon leaves the field.  Under
    /// reincarnation (74) the unit rises with full life; under the dark
    /// pact (91) it rises as the demon of its true form's level, which
    /// also takes its place in the hero's squad.  TakeDamage still counts
    /// both as a kill.
    pub fn death(b: *Battle, ref: Ref) void {
        const f = b.fighter(ref);
        if (f.hero_unit) {
            for (b.units[ref.side][max_squad..]) |*o| {
                if (o.present() and o.life > 0) b.changeMorale(o, -1);
            }
        }
        for (0..2) |s| {
            for (&b.units[s]) |*o| {
                if (o.life == 0) continue;
                for (neighbours(f.x, f.y)) |c| {
                    if (o.x == c[0] and o.y == c[1]) b.changeMorale(o, if (s == ref.side) -1 else 1);
                }
            }
        }
        var change: ?u8 = null;
        var reincarnate = false;
        var dark_pact = false;
        for (f.effectList()) |e| {
            if (e.kind == eff_charm) change = @intCast(std.math.clamp(e.power, 0, n_slots - 1));
            if (e.kind == eff_reincarnate) reincarnate = true;
            if (e.kind == eff_dark_pact) dark_pact = true;
            if (e.kind == eff_polymorph) if (f.true_form) |tf| {
                f.unit = tf;
                f.true_form = null;
                f.speed = @min(f.speed, b.maxSpeed(f));
                f.ammo = @min(f.ammo, b.maxAmmo(f));
            };
        }
        f.n_eff = 0;
        if (reincarnate) {
            f.life = b.maxLife(f);
            f.fear = 0;
            b.emit(.{ .revive = ref });
            if (change) |n| b.changeSide(ref, n);
            return;
        }
        if (dark_pact) {
            // pTrueForm is pUnit unless polymorphed, and the polymorph
            // has just ended.
            const tf = f.true_form orelse f.unit;
            const level = b.pr.units[@intCast(tf.type)].level;
            const demon_type: i32 = demon_types[@intCast(std.math.clamp(level, 1, 4) - 1)];
            var demon = unit_mod.Unit.init(b.pr, b.r, demon_type, 0);
            demon.x = tf.x;
            demon.y = tf.y;
            f.unit = demon;
            f.true_form = null;
            if (!f.summoned and !b.simulate) {
                b.armies[f.home].squad[f.num] = demon;
                b.countSquadMorale(f.home);
            }
            f.life = b.maxLife(f);
            f.ammo = b.maxAmmo(f);
            f.stamina = b.maxStamina(f);
            f.morale = b.maxMorale(f);
            f.fear = 0;
            b.emit(.{ .revive = ref });
            if (change) |n| b.changeSide(ref, n);
            return;
        }
        b.setPass(f, true);
        f.can_act = false;
        if (f.summoned) {
            const t = b.utype(f);
            const s = ref.side;
            b.summon_exp[s] += t.exp + @divTrunc(t.exp * f.unit.level, 10);
            b.summon_surv[s] += f.st.survived;
            b.summon_work[s] += f.st.work() - f.st.spells * 3 - 1;
            f.in_battle = false;
            f.used = true;
            return;
        }
        if (change) |n| return b.changeSide(ref, n);
        const kind = b.unitKind(f);
        if (kind == kind_undead or kind == kind_construct or b.bfo(b.landOf(f)).move_cost <= 0) f.in_battle = false;
    }

    /// BF_Unit::ChangeSide: the unit moves to slot `num` of the other side;
    /// its old slot is cleared.
    pub fn changeSide(b: *Battle, ref: Ref, num: u8) void {
        const to: Ref = .{ .side = 1 - ref.side, .slot = num };
        const f = b.fighter(ref);
        b.units[to.side][to.slot] = f.*;
        const x = f.x;
        const y = f.y;
        f.* = .{ .unit = .{ .type = 0, .life = 0, .exp = 0 }, .x = x, .y = y };
        b.emit(.{ .swap = .{ .from = ref, .to = to } });
    }

    /// The end of a polymorph (effect 90): the true form with the same
    /// share of life.
    pub fn restoreForm(b: *Battle, f: *Fighter) void {
        const tf = f.true_form orelse return;
        f.life = @divTrunc(f.life * tf.maxLife(b.pr), @max(f.unit.maxLife(b.pr), 1));
        if (f.life <= 0) f.life = 1;
        f.unit = tf;
        f.true_form = null;
        f.life = @min(f.life, b.maxLife(f));
        f.ammo = @min(f.ammo, b.maxAmmo(f));
        f.speed = @min(f.speed, b.maxSpeed(f));
    }

    /// A summon slot of side `s` never used in this battle.
    pub fn freeSummonSlot(b: *const Battle, s: u1) ?u8 {
        for (max_squad..n_slots) |i| {
            const f = &b.units[s][i];
            if (f.unit.type == 0 and !f.used) return @intCast(i);
        }
        return null;
    }

    /// BF_Unit::Summon: a unit of type `t` with the levels `power` gives.
    pub fn summon(b: *Battle, ref: Ref, t: i32, power: i32, x: i32, y: i32) void {
        if (t <= 0 or t >= b.pr.units.len) return;
        const u = unit_mod.Unit.init(b.pr, b.r, t, unit_mod.expForLevel(b.pr, t, @max(power, 0)));
        const f = b.fighter(ref);
        f.* = .{ .unit = u, .in_battle = true, .summoned = true, .home = ref.side, .x = x, .y = y };
        f.life = u.maxLife(b.pr);
        b.setPass(f, false);
        f.morale = b.typeStat(f, stat.morale);
        f.stamina = b.typeStat(f, stat.stamina) + b.heroUnitBonus(ref.side, stat.stamina);
        f.speed = b.typeStat(f, stat.speed);
        if (u.checkAbility(b.pr, ab.rooted) > 0) f.speed = 0;
        f.ammo = b.typeStat(f, stat.ammo);
        f.can_act = true;
        f.march = true;
        if (f.morale < 1) f.morale = 1;
        f.life = @min(f.life, b.maxLife(f));
        // Innate reincarnation (97), the same effect Init gives a squad
        // unit: one rise, as under spell 59.
        if (b.skill(f, ab.reincarnation) > 0) f.addEffect(.{ .kind = eff_reincarnate, .power = 1, .time = 0, .spell = 59, .visible = true, .destroy_on_hit = false });
        b.emit(.{ .appear = .{ .who = ref, .rise = true } });
    }

    /// BF_Unit::RaiseCorpse (ability 64), after Death has resolved the
    /// victim's true form, reincarnation and change of side.
    fn raiseCorpse(b: *Battle, sr: Ref, target: *Fighter) void {
        const s = b.fighter(sr);
        if (b.skill(s, ab.raise_corpse) <= 0 or !target.present() or target.summoned or target.life > 0) return;
        const kind = b.unitKind(target);
        if (kind == kind_undead or kind == kind_construct or b.bfo(b.landOf(target)).move_cost <= 0) return;
        const slot = b.freeSummonSlot(sr.side) orelse return;
        const level = if (s.hero_unit)
            b.heroBonus(s, 907) + b.heroBonus(s, 909)
        else
            @divTrunc(s.unit.level, 4);
        b.summon(.{ .side = sr.side, .slot = slot }, 19, level, target.x, target.y);
        // bInvisOnDeath: the scene keeps the death animation, then hides
        // the corpse. It is no longer available to corpse spells/eating.
        target.in_battle = false;
    }

    /// SoulSteal runs after Death: polymorph has ended, charmed units have
    /// left their slot, and a dead summon's pUnit has been cleared.
    fn soulSteal(b: *Battle, sr: Ref, target: *const Fighter) void {
        if (target.unit.type == 0 or (target.summoned and !target.in_battle)) return;
        const kind = b.unitKind(target);
        if (kind == kind_undead or kind == kind_construct) return;
        const s = b.fighter(sr);
        const healed = @min(b.skill(s, ab.soul_steal), b.maxLife(s) - s.life);
        if (healed <= 0) return;
        s.life += healed;
        b.emit(.{ .number = .{ .who = sr, .value = healed, .color = col_soul, .sound_unit = sound_soul } });
    }

    /// One melee blow and its effects; true when the target dies.
    fn blow(b: *Battle, sr: Ref, tr: Ref, kind: enum { attack, counter, first_strike }, bonus: i32) bool {
        const attacking = kind == .attack;
        const s = b.fighter(sr);
        const t = b.fighter(tr);
        var hit = b.meleeHit(s, t, attacking) + bonus;
        if (attacking) t.parry = false;
        hit = @min(hit, t.life);
        var trample: ?[2]i32 = null;
        var fx: BlowFx = .{ .round = true };
        // BF_Unit::Attack: a strict threshold, only for the main blow.
        // Resurrection prevents advancing, but not the finishing damage.
        const trample_power = b.skill(s, ab.trample);
        if (attacking and trample_power > 0 and t.life - hit < trample_power and
            b.skill(t, ab.flying) == 0 and b.skill(s, ab.rooted) == 0)
        {
            hit = t.life;
            if (b.skill(t, eff_reincarnate) == 0 and b.skill(t, eff_dark_pact) == 0) {
                b.setPass(s, true);
                s.x = t.x;
                s.y = t.y;
                trample = .{ s.x, s.y };
            }
        }
        s.st.melee_done += hit;
        if (attacking) {
            b.useStamina(s, if (s.speed < b.maxSpeed(s)) 2 else 1);
            if (b.round_attack) b.useStamina(s, b.skill(s, 66));
            if (b.crush) b.useStamina(s, b.skill(s, 59));
        } else {
            b.useStamina(s, 1);
            s.can_act = false;
        }
        b.fx = &fx;
        b.hitEffect(s, t, hit, &fx);
        b.roundAttack(sr, tr, attacking);
        if (attacking and b.crush) fx.note(sr, ph_crush, 0, col_crush, sound_crush, 0);
        // The broken weapon and armour (phrases 371/372) join here once
        // DamageWeapon and DamageArmor are ported.
        if (fx.drain > 0) fx.note(sr, 0, fx.drain, col_drain, 0, 0);
        // The blow is shown before the damage so a dying victim's revive
        // follows it; TakeDamage may add the berserk text afterwards, so the
        // notes are copied into the queued animation once more.
        const anim_index = b.anims.items.len;
        b.emit(.{ .melee = .{ .striker = sr, .target = tr, .hit = hit, .life = t.life - hit, .trample = trample, .notes = fx.notes, .n_notes = fx.n } });
        const killed = b.takeDamage(tr, hit, 0);
        switch (b.anims.items[anim_index]) {
            .melee => |*m| {
                m.notes = fx.notes;
                m.n_notes = fx.n;
            },
            else => {},
        }
        b.fx = null;
        if (killed) {
            s.st.slain += 1;
            b.changeMorale(s, 1);
            // Death frees the victim's cell; a trampling attacker now owns it.
            if (attacking) b.setPass(s, false);
            // The first-strike branch checks iLife after TakeDamage, unlike
            // the main blow/counter, which also steal a reincarnated soul.
            if (kind != .first_strike or t.life == 0) b.soulSteal(sr, t);
            b.raiseCorpse(sr, t);
            return true;
        }
        return false;
    }

    /// Hit every other enemy next to the primary target of a circular attack.
    ///
    /// HitEffect in the original is called with bCanRound=false for these
    /// secondary hits, and TakeDamage uses damage type 1 (the same type as a
    /// shot).  The primary target is deliberately not hit a second time.
    fn roundAttack(b: *Battle, sr: Ref, tr: Ref, attacking: bool) void {
        const a = b.fighter(sr);
        // Ability 65 is always active.  `round_attack` is the transient
        // button pressed by the AI (and later by the human battle panel), but
        // a button cannot trigger a circular counterattack.
        if (b.skill(a, ab.round_attack) == 0 and !(b.round_attack and attacking)) return;

        var around: [6]Ref = undefined;
        var n: usize = 0;
        const enemy_side: u1 = 1 - sr.side;
        for (neighbours(a.x, a.y)) |cell| {
            if (!inside(cell[0], cell[1])) continue;
            for (&b.units[enemy_side], 0..) |*o, i| {
                if (o.life > 0 and o.x == cell[0] and o.y == cell[1]) {
                    if (n < around.len) {
                        around[n] = .{ .side = enemy_side, .slot = @intCast(i) };
                        n += 1;
                    }
                    break;
                }
            }
        }
        // FindNeighbour must find the primary target and at least one more
        // unit before the attack has a secondary effect.
        if (n <= 1) return;

        var struck = false;
        for (around[0..n]) |nr| {
            if (nr.side == tr.side and nr.slot == tr.slot) continue;
            const d = b.fighter(nr);
            if (d.life <= 0) continue;
            const hit = @min(b.meleeHit(a, d, attacking), d.life);
            // A side target has its own blow, so its berserk is not part of
            // the main one; bCanRound is false for it, and its text waits 15
            // ticks (AddAttackEnemyEffect(2,15,arN[i])).
            var side: BlowFx = .{ .berserk_delay = 15 };
            const saved = b.fx;
            b.fx = &side;
            b.hitEffect(a, d, hit, &side);
            if (b.takeDamage(nr, hit, 1)) b.emit(.{ .death = nr });
            b.fx = saved;
            var note: Anim = .{ .number = .{ .who = nr, .value = hit, .color = .{ 220, 50, 50 } } };
            side.prefix(&note);
            b.emit(note);
            struck = true;
        }
        // Only the pressed button announces the attack, and only when it hit.
        if (struck and b.round_attack and attacking) if (b.fx) |f| f.note(sr, ph_round, 0, col_round, sound_round, 0);
    }

    /// BF_Unit::Attack from cell (x1, y1); false when the attacker dies.
    pub fn attackFrom(b: *Battle, ref: Ref, x1: i32, y1: i32, er: Ref) bool {
        const f = b.fighter(ref);
        const e = b.fighter(er);
        f.can_act = false;
        var bonus: i32 = 0;
        if (x1 != f.x or y1 != f.y) {
            if (b.skill(f, ab.charge) != 0) bonus = @max(@as(i32, @intCast(@abs(f.x - e.x) + @abs(f.y - e.y))) - 2, 0);
            b.moveTo(ref, x1, y1);
        }
        const strike_first = b.skill(e, ab.first_strike) != 0 and b.skill(f, ab.first_strike) == 0 and b.skill(f, ab.agility) == 0 and b.skill(e, ab.no_combat) == 0 and !e.resting;
        if (strike_first) {
            if (e.stamina > 0 and b.blow(er, ref, .first_strike, 0)) return false;
            _ = b.blow(ref, er, .attack, bonus);
        } else {
            if (!b.blow(ref, er, .attack, bonus)) {
                if (e.stamina > 0 and b.skill(f, ab.agility) == 0 and b.skill(e, ab.no_combat) == 0 and !e.resting) {
                    if (b.blow(er, ref, .counter, 0)) return false;
                }
            }
        }
        f.speed = 0;
        return true;
    }

    /// BF_Unit::Shoot: petrification replaces the ordinary shot. Repeated
    /// applications prolong every part of the effect without stacking it.
    fn petrifyShot(b: *Battle, ref: Ref, er: Ref) void {
        const f = b.fighter(ref);
        const e = b.fighter(er);
        const time = b.skill(f, ab.petrify) - b.resist(e);
        if (time <= 0 or b.skill(e, ab.petrify_immunity) != 0) {
            b.emit(.{ .binding = .{ .shooter = ref, .target = er, .stone = true, .success = false } });
            b.emit(.{ .text = .{ .who = er, .phrase = battle_spell.ph_resisted, .color = battle_spell.col_resisted } });
            return;
        }
        var found = false;
        for (e.effects[0..e.n_eff]) |*ef| {
            if (ef.ability == upg_icon.petrify) {
                ef.time = @max(ef.time, time);
                found = true;
            }
        }
        if (!found) {
            timed(e, ab.immobile, 1, time, upg_icon.petrify, true);
            timed(e, stat.defence, 3, time, upg_icon.petrify, false);
            timed(e, stat.ranged_defence, 3, time, upg_icon.petrify, false);
            timed(e, ab.no_combat, 1, time, upg_icon.petrify, false);
            timed(e, stat.resist, 3, time, upg_icon.petrify, false);
            e.speed = 0;
            e.can_act = false;
        }
        b.emit(.{ .binding = .{ .shooter = ref, .target = er, .stone = true, .success = true } });
    }

    /// Ability 47 ignores resistance. Both its own web and spell 18's web
    /// gain one turn on a repeat shot, regardless of the new shot's power.
    fn webShot(b: *Battle, ref: Ref, er: Ref) void {
        const f = b.fighter(ref);
        const e = b.fighter(er);
        if (b.skill(e, ab.web_immunity) != 0) {
            b.emit(.{ .binding = .{ .shooter = ref, .target = er, .stone = false, .success = false } });
            b.emit(.{ .text = .{ .who = er, .phrase = battle_spell.ph_resisted, .color = battle_spell.col_resisted } });
            return;
        }
        const time: i32 = if (b.skill(f, ab.web) > b.attack(e)) 3 else 2;
        var found = false;
        for (e.effects[0..e.n_eff]) |*ef| {
            if (ef.ability == upg_icon.web or ef.spell == 18) {
                ef.time += 1;
                found = true;
            }
        }
        if (!found) {
            timed(e, ab.immobile, 1, time, upg_icon.web, true);
            timed(e, stat.defence, 1, time, upg_icon.web, false);
            timed(e, stat.ranged_defence, 1, time, upg_icon.web, false);
            timed(e, ab.no_combat, 1, time, upg_icon.web, false);
            e.speed = 0;
            e.can_act = false;
        }
        b.emit(.{ .binding = .{ .shooter = ref, .target = er, .stone = false, .success = true } });
    }

    /// AnimShoot uses the equipped weapon's projectile and sounds for a
    /// class hero: item class 3 uses unit 41, every other weapon unit 43.
    fn shotType(b: *const Battle, f: *const Fighter) *const proto.UnitType {
        if (f.hero_unit) {
            if (b.classHero(f.home)) |h| {
                const weapon = h.inventory[1][0];
                if (weapon > 0 and weapon < b.pr.items.len) {
                    const id: usize = if (b.pr.items[@intCast(weapon)].class == 3) 41 else 43;
                    return &b.pr.units[id];
                }
            }
        }
        return b.utype(f);
    }

    /// BF_Unit::Shoot, including replacement attacks and double shots.
    pub fn shoot(b: *Battle, ref: Ref, er: Ref) void {
        const f = b.fighter(ref);
        const e = b.fighter(er);
        // buttDoubleShot: two shots while the target lives.
        var shots: i32 = @min(@as(i32, if (b.double_shot) 2 else 1), f.ammo);
        var wait: i32 = 0;
        // The original checks these independently: a unit with both
        // abilities applies both and spends one projectile on each.
        if (b.skill(f, ab.petrify) != 0) {
            b.petrifyShot(ref, er);
            f.ammo -= 1;
            shots = 0;
        }
        if (b.skill(f, ab.web) != 0) {
            b.webShot(ref, er);
            f.ammo -= 1;
            shots = 0;
        }
        while (shots > 0) : (shots -= 1) {
            const hit = @min(b.rangedHit(f, e), e.life);
            f.st.range_done += hit;
            f.ammo -= 1;
            const shot_type = b.shotType(f);
            var fx: BlowFx = .{};
            b.fx = &fx;
            var missile: Anim = .{ .shoot = .{
                .shooter = ref,
                .target = er,
                .hit = hit,
                .life = e.life - hit,
                .missile = shot_type.missile,
                .sound_shoot = shot_type.sound_shoot,
                .sound_hit = shot_type.sound_shoot_hit,
                .wait = wait * 10,
                .speed = 15 + b.battle_speed * 3,
            } };
            fx.prefix(&missile);
            const anim_index = b.anims.items.len;
            b.emit(missile);
            wait += 1;
            if (b.skill(f, ab.poison_shot) != 0 and b.skill(e, ab.poison_immunity) == 0) e.addEffect(.{ .kind = stat.life, .power = -b.skill(f, ab.poison_shot), .time = poison_duration, .visible = true, .poison = true, .ability = upg_icon.poison });
            if (b.skill(f, ab.heavy_shot) != 0) timed(e, stat.ranged_defence, -1, b.skill(f, ab.heavy_shot), upg_icon.heavy_shot, true);
            b.attackSpell(f, e, b.skill(f, ab.spell_shot));
            const killed = b.takeDamage(er, hit, if (b.skill(f, ab.magic_shot) != 0) 2 else 1);
            // TakeDamage may add the berserk text; the missile carries it.
            switch (b.anims.items[anim_index]) {
                .shoot => |*m| {
                    m.notes = fx.notes;
                    m.n_notes = fx.n;
                },
                else => {},
            }
            b.fx = null;
            if (killed) {
                f.st.shot += 1;
                b.changeMorale(f, 1);
                b.raiseCorpse(ref, e);
                break;
            }
        }
        b.useStamina(f, if (f.speed < b.maxSpeed(f)) 2 else 1);
        if (b.double_shot) b.useStamina(f, b.skill(f, 20));
        f.speed = 0;
        f.can_act = false;
    }

    /// BF_Unit::Heal.
    pub fn heal(b: *Battle, ref: Ref, tr: Ref) void {
        const f = b.fighter(ref);
        const t = b.fighter(tr);
        const roll: i32 = if (b.simulate) 0 else 1 - rnd(b.r, 3);
        const h = @min(b.skill(f, ab.healer) + roll, b.maxLife(t) - t.life);
        f.st.healed += h;
        t.life += h;
        // The green stars over both units stay with the visual effects.
        b.emit(.{ .number = .{ .who = tr, .value = h, .color = col_heal, .sound_unit = sound_heal } });
        b.useStamina(f, 1);
        f.ammo -= 1;
        f.speed = 0;
        f.can_act = false;
    }

    /// BF_Unit::GoMarch: one more move point for the march's stamina.
    pub fn goMarch(b: *Battle, ref: Ref) void {
        const f = b.fighter(ref);
        b.useStamina(f, b.skill(f, ab.march));
        f.march = false;
        f.speed += 1;
        b.emit(.{ .text = .{ .who = ref, .phrase = ph_march, .color = col_text, .sound_unit = sound_march } });
    }

    /// buttRest.
    pub fn rest(b: *Battle, ref: Ref) void {
        const f = b.fighter(ref);
        f.resting = true;
        f.speed = 0;
        f.can_act = false;
    }

    /// The rest button is active (SelectUnit).
    pub fn canRest(b: *const Battle, f: *const Fighter) bool {
        if (f.resting) return false;
        const needs = f.stamina < b.maxStamina(f) or (b.skill(f, ab.ammo_regen) != 0 and f.ammo < b.maxAmmo(f)) or
            (b.skill(f, ab.regen_on_rest) != 0 and f.life < b.maxLife(f));
        return needs and (f.speed >= b.maxSpeed(f) or (f.can_act and b.skill(f, ab.rooted) != 0)) and
            b.skill(f, ab.tireless) == 0 and (f.march or b.skill(f, ab.march) == 0);
    }

    /// CanAttack: may the active unit strike from (x, y)?
    pub fn canAttackFrom(b: *Battle, x: i32, y: i32) bool {
        const ref = b.active orelse return false;
        const f = b.fighter(ref);
        if (!inside(x, y) or !f.can_act) return false;
        if (f.x == x and f.y == y) return true;
        return b.grid(x, y).pass and b.mc(x, y).pass and f.speed > 0 and f.stamina != 0;
    }

    /// The target can be shot from where the unit stands.
    pub fn canShoot(b: *Battle, ref: Ref, tx: i32, ty: i32) bool {
        const f = b.fighter(ref);
        return f.can_act and f.ammo != 0 and b.mc(tx, ty).shoot > 1 and (f.speed > 0 or b.skill(f, ab.rooted) != 0) and f.stamina > 0 and b.rangedAttack(f) > 0;
    }

    /// FindNextUnit: the next unit of the side that may act, after the
    /// active one.
    pub fn nextUnit(b: *const Battle) ?Ref {
        const s = b.side;
        const cur: ?usize = if (b.active) |a| (if (a.side == s) @as(usize, a.slot) else null) else null;
        const start = if (cur) |a| a + 1 else 0;
        const n: usize = if (cur != null) n_slots - 1 else n_slots;
        for (0..n) |k| {
            const i = (start + k) % n_slots;
            const f = &b.units[s][i];
            if (f.can_act and f.life > 0) return .{ .side = s, .slot = @intCast(i) };
        }
        return null;
    }

    /// Loss: the first living unit of a side, null when none.
    pub fn firstAlive(b: *const Battle, s: u1) ?u8 {
        for (&b.units[s], 0..) |*f, i| {
            if (f.life != 0) return @intCast(i);
        }
        return null;
    }

    pub fn over(b: *const Battle) bool {
        return b.firstAlive(0) == null or b.firstAlive(1) == null or @divTrunc(b.turn, 2) > b.max_duration;
    }

    /// The first side to move: the higher initiative, side 0 on a tie.
    pub fn firstSide(b: *const Battle) u1 {
        return if (b.armies[0].initiative < b.armies[1].initiative) 1 else 0;
    }

    /// BF_Unit::Panic.
    pub fn panic(b: *Battle, ref: Ref) void {
        const f = b.fighter(ref);
        defer {
            f.speed = 0;
            f.can_act = false;
        }
        if (b.simulate or rnd(b.r, 100) > f.fear or b.skill(f, ab.immobile) > 0 or b.skill(f, ab.rooted) > 0 or f.stamina <= 0 or f.x == @as(i32, ref.side) * 7) {
            b.emit(.{ .text = .{ .who = ref, .phrase = ph_stays, .color = col_panic } });
            return;
        }
        b.clearMap();
        b.first_move = true;
        b.can_pass = .{ 0, 0, 0 };
        if (b.skill(f, ab.flying) != 0) {
            b.flyMap(f.x, f.y, -1, f.speed, true);
        } else if (b.skill(f, ab.low_flying) != 0) {
            b.flyMap(f.x, f.y, -1, f.speed, false);
        } else b.stepMap(f.x, f.y, -1, f.speed, 0, f.stamina);
        var cells: [8][2]i32 = undefined;
        var n: usize = 0;
        var i: i32 = @as(i32, ref.side) * 7;
        while (i != 7 - @as(i32, ref.side) * 7 and n == 0) : (i += 1 - @as(i32, ref.side) * 2) {
            for (0..size) |j| {
                const jj: i32 = @intCast(j);
                if (b.mc(i, jj).pass and b.grid(i, jj).pass) {
                    cells[n] = .{ i, jj };
                    n += 1;
                }
            }
        }
        if (n == 0) {
            b.emit(.{ .text = .{ .who = ref, .phrase = ph_stays, .color = col_panic } });
            return;
        }
        const k: usize = @intCast(rnd(b.r, @intCast(n)));
        b.emit(.{ .text = .{ .who = ref, .phrase = ph_flees, .color = col_panic } });
        b.moveTo(ref, cells[k][0], cells[k][1]);
    }

    /// EndTurn(true, …): the other side's turn begins; resting, effects,
    /// poison and regeneration, the move points and the panic checks.
    pub fn endTurn(b: *Battle) void {
        b.active = null;
        if (b.over()) return;
        b.side = 1 - b.side;
        const s = b.side;
        b.archmage = if (b.armies[s].hero) |h| h.heroBonus(b.pr, 984) != 0 else false;
        for (0..n_slots) |i| {
            const ref: Ref = .{ .side = s, .slot = @intCast(i) };
            const f = b.fighter(ref);
            if (f.life <= 0 or !f.in_battle) continue;
            var change: ?u8 = null;
            f.st.survived += 1;
            if (b.skill(f, ab.parry) > 0) f.parry = true;
            var hit: i32 = 0;
            if (f.resting or (f.can_act and f.speed == b.maxSpeed(f))) {
                f.stamina = @min(f.stamina + 2 + b.skill(f, ab.recovery), b.maxStamina(f));
                f.ammo = @min(f.ammo + b.skill(f, ab.ammo_regen), b.maxAmmo(f));
                hit += b.skill(f, ab.regen_on_rest);
                f.resting = false;
            }
            var k: usize = 0;
            while (k < f.n_eff) {
                const e = &f.effects[k];
                if (e.kind == stat.life) hit += e.power;
                if (e.kind == stat.stamina and b.skill(f, ab.tireless) < 1) f.stamina = std.math.clamp(f.stamina + e.power, 0, b.maxStamina(f));
                if (e.time > 0) {
                    e.time -= 1;
                    if (e.time == 0) {
                        if (e.kind == eff_charm) change = @intCast(std.math.clamp(e.power, 0, n_slots - 1));
                        if (e.kind == eff_polymorph) b.restoreForm(f);
                        f.removeEffect(k);
                        continue;
                    }
                }
                k += 1;
            }
            hit += b.skill(f, ab.regeneration);
            if (hit + f.life < 0) hit = -f.life;
            if (hit + f.life > b.maxLife(f) and hit > 0) hit = b.maxLife(f) - f.life;
            if (hit < 0) {
                b.emit(.{ .number = .{ .who = ref, .value = -hit, .color = col_poison } });
                if (b.takeDamage(ref, -hit, 3)) {
                    b.emit(.{ .death = ref });
                    continue;
                }
            } else if (hit > 0) {
                f.life += hit;
                b.emit(.{ .number = .{ .who = ref, .value = hit, .color = col_heal } });
            }
            if (f.life > 0 and b.skill(f, ab.immobile) == 0) {
                f.can_act = true;
                if (b.skill(f, ab.rooted) == 0) {
                    f.speed = b.maxSpeed(f);
                    f.march = true;
                } else f.march = false;
                if (f.stamina < 1) {
                    f.can_act = false;
                    f.resting = true;
                    f.speed = 0;
                }
                if (f.morale <= 0) {
                    // bSimulate: no roll, the fear decides.
                    const calm = if (b.simulate) f.fear < 30 else f.fear < 50 and rnd(b.r, 100) > f.fear;
                    if (calm) {
                        b.changeMorale(f, 1);
                        f.fear = 0;
                    } else {
                        f.fear -= 10;
                        b.panic(ref);
                    }
                }
            }
            // The charm is over: tired and shaken, the unit goes back.
            if (change) |n| if (f.life > 0) {
                if (f.stamina > 0) b.useStamina(f, @divTrunc(f.stamina, 2));
                if (f.morale > 0) b.changeMorale(f, -@divTrunc(f.morale, 2));
                b.changeSide(ref, n);
            };
        }
        b.turn += 1;
        b.ai = .{};
    }

    // -- the computer side ---------------------------------------------

    /// One order of the computer side (BattleAI.cpp); false when the
    /// side's turn is over.
    pub fn aiStep(b: *Battle) bool {
        return battle_ai.step(b);
    }

    // -- the end --------------------------------------------------------

    /// The outcome of InitBattle.
    pub fn result(b: *const Battle) i32 {
        const a0 = b.firstAlive(0) != null;
        const a1 = b.firstAlive(1) != null;
        if (!a0 and a1) return 0;
        if (a0 and !a1) return 1;
        if (!a0 and !a1) return -1;
        return -2;
    }

    /// EndBattle (Battlefield.cpp:958): the experience of the winner's
    /// squad, the medals, the hunt quests, the trophies and the loot, the
    /// life and morale both squads keep (written to `armies[i].squad`),
    /// the level-ups of the winner's units; in a draw one dead unit of a
    /// guard lives on.  The winner's hero gets the quests and the loot
    /// here; its experience, gold and karma go to the caller.
    ///
    /// Not ported: the encounters of the quests, the campaign's demon
    /// count, WorldRules, the frightened-enemy victory (FearVictory), the
    /// player's arLoot counters and stocks of contracts, blueprints and
    /// rituals, necromancy, the hero's level-up.  A human's unit takes the
    /// rolled upgrade (WindowUnitLvlUp lets the player choose).
    pub fn finish(b: *Battle, ec: EndCtx) Outcome {
        var out: Outcome = .{ .result = b.result() };
        const win: ?u1 = switch (out.result) {
            1 => 0,
            0 => 1,
            else => null,
        };
        // The charmed units go back as dead, the polymorphs end.
        for (0..2) |sd| for (0..n_slots) |i| {
            const ref: Ref = .{ .side = @intCast(sd), .slot = @intCast(i) };
            const f = b.fighter(ref);
            if (!f.present()) continue;
            var found: ?u8 = null;
            for (f.effectList()) |e| {
                if (e.kind == eff_charm) {
                    found = @intCast(std.math.clamp(e.power, 0, n_slots - 1));
                } else if (e.kind == eff_polymorph) if (f.true_form) |tf| {
                    f.life = @max(@divTrunc(f.life * tf.maxLife(b.pr), @max(f.unit.maxLife(b.pr), 1)), 1);
                    f.unit = tf;
                    f.true_form = null;
                    f.life = @min(f.life, b.maxLife(f));
                    f.ammo = @min(f.ammo, b.maxAmmo(f));
                };
            }
            f.n_eff = 0;
            if (found) |n| {
                b.changeSide(ref, n);
                b.units[1 - sd][n].life = 0;
            }
        };
        if (win) |w| {
            b.finishWin(w, ec, &out);
        } else {
            b.keepSquads(0, false);
            b.keepSquads(1, false);
            for (0..2) |j| {
                const jj: u1 = @intCast(j);
                if (b.classHero(jj)) |h| {
                    var hits: [hero_mod.quest_limit]i32 = @splat(0);
                    for (b.units[1 - jj][0..max_squad]) |*f| {
                        if (f.unit.type != 0 and f.life == 0) b.huntQuests(h, ec, f.unit.type, false, &hits);
                    }
                    b.checkQuests(h, ec, &.{ 3, 8 });
                }
            }
            // A guard keeps one of its dead, the low levels likelier.
            for (0..2) |j| {
                const jj: u1 = @intCast(j);
                if (b.classHero(jj) != null) continue;
                var rl: unit_mod.Roller = .{};
                var n: usize = 0;
                for (&b.armies[jj].squad, 0..) |*slot, i| {
                    const u = &(slot.* orelse continue);
                    if (b.units[jj][i].unit.type == 0 or u.life != 0) continue;
                    rl.ids[n] = @intCast(i + 1);
                    rl.weights[n] = 5 - b.pr.units[@intCast(u.type)].level;
                    n += 1;
                }
                if (n > 0) {
                    const lucky: usize = @intCast(rl.roll(b.r) - 1);
                    b.armies[jj].squad[lucky].?.life = 1;
                }
            }
        }
        return out;
    }

    /// Heroes[s] with ipClass1 > 0 (a guard is a dcHero without a class).
    fn classHero(b: *const Battle, s: u1) ?*hero_mod.Hero {
        const h = b.armies[s].hero orelse return null;
        return if (h.class1 > 0) h else null;
    }

    /// The quests of `h` that count killed units of type `t`: types 3 and,
    /// for the winner, 2 (with its Param chance), and the site guard of a
    /// type 8 quest.
    fn huntQuests(b: *Battle, h: *hero_mod.Hero, ec: EndCtx, t: i32, winner: bool, hits: *[hero_mod.quest_limit]i32) void {
        for (0..hero_mod.quest_limit) |qi| {
            var q = h.quest(qi);
            if (q.index < 0 or q.index >= b.pr.quests.len) continue;
            const qp = &b.pr.quests[@intCast(q.index)];
            if (qp.kind == 3 or (winner and qp.kind == 2)) {
                for (qp.tasks[0..hero_mod.quest_tasks], 0..) |task, j| {
                    if (task.index != t or q.complete) continue;
                    if (!winner or task.param == 0 or rnd(b.r, 100) < task.param) q.progress[j] += 1;
                    hits[qi] += 1;
                }
            }
            if (qp.kind == 8 and !q.complete) {
                if (siteOf(ec, q)) |site| if (site.level == t) {
                    q.progress[0] += 1;
                    hits[qi] += 1;
                };
            }
            h.setQuest(qi, q);
        }
    }

    fn siteOf(ec: EndCtx, q: hero_mod.Quest) ?province_mod.Site {
        if (q.province < 0 or q.province >= ec.provinces.len) return null;
        if (q.site < 0 or q.site >= province_mod.max_site) return null;
        return ec.provinces[@intCast(q.province)].sites[@intCast(q.site)];
    }

    /// CheckQuestDone for the quests of the listed types.
    fn checkQuests(b: *Battle, h: *hero_mod.Hero, ec: EndCtx, kinds: []const i32) void {
        for (0..hero_mod.quest_limit) |qi| {
            var q = h.quest(qi);
            if (q.index < 0 or q.index >= b.pr.quests.len) continue;
            const qp = &b.pr.quests[@intCast(q.index)];
            if (std.mem.indexOfScalar(i32, kinds, qp.kind) == null) continue;
            if (!q.complete and qp.kind < 7) {
                var done = true;
                for (qp.tasks[0..hero_mod.quest_tasks], 0..) |task, j| {
                    if (task.index > 0 and q.progress[j] < task.quantity) done = false;
                }
                if (done) q.complete = true;
            }
            if (qp.kind == 8 and !q.complete) {
                if (siteOf(ec, q)) |site| if (site.data <= q.progress[0]) {
                    q.complete = true;
                };
            }
            h.setQuest(qi, q);
        }
    }

    /// The life and morale squad `s` keeps (EndBattle's last loop; the
    /// fearless retain their pre-battle morale only on the winner's side).
    fn keepSquads(b: *Battle, s: u1, winner: bool) void {
        for (&b.armies[s].squad, 0..) |*slot, i| {
            const u = &(slot.* orelse continue);
            const f = &b.units[s][i];
            if (f.unit.type == 0) continue;
            const ml = b.maxLife(f);
            const bonus = b.heroUnitBonus(s, stat.life);
            u.life = @divTrunc(f.life * 100, b.armies[s].life_mod) - @divTrunc(bonus * (100 - @divTrunc((ml - f.life) * 100, ml)), 100);
            if (u.life <= 0 and f.life > 0) u.life = 1;
            if (u.life < 0) u.life = 0;
            if (!winner or u.checkAbility(b.pr, ab.fearless) == 0) {
                const mm = b.maxMorale(f);
                const m = @min(f.morale, mm + 10);
                u.morale = (m - mm) * 2;
            }
        }
    }

    fn finishWin(b: *Battle, w: u1, ec: EndCtx, out: *Outcome) void {
        const l = 1 - w;
        const win_hero = b.classHero(w);
        const lose_hero = b.classHero(l);
        var pool: i32 = b.summon_exp[l];
        if (lose_hero) |h| if (b.units[l][0].life == 0) {
            pool += h.level * 10;
            out.gold += h.level * (10 - h.class1);
            out.gem += h.level * h.class1;
        };
        var hits: [2][hero_mod.quest_limit]i32 = @splat(@splat(0));
        for (0..max_squad) |i| {
            const f = &b.units[l][i];
            if (f.unit.type != 0 and f.life == 0) {
                const t = b.utype(f);
                pool += t.exp + @divTrunc(t.exp * f.unit.level, 10);
                out.gold += t.gold_price + @divTrunc(t.gold_price * f.unit.level, 10);
                out.gem += t.gem_price + @divTrunc(t.gem_price * f.unit.level, 10);
                if (win_hero) |h| b.huntQuests(h, ec, f.unit.type, true, &hits[w]);
            }
            const fw = &b.units[w][i];
            if (fw.unit.type != 0 and fw.life == 0) {
                if (lose_hero) |h| b.huntQuests(h, ec, fw.unit.type, false, &hits[l]);
            }
        }
        const hero_alive = win_hero != null and b.units[w][0].life > 0;
        if (hero_alive) pool += @divTrunc(pool * b.skill(&b.units[w][0], 990), 100);
        if (win_hero != null) out.exp[0] += @divTrunc(pool * 3, 10);
        pool -= @divTrunc(pool * 3, 10);
        var work: i32 = b.summon_surv[w];
        for (&b.units[w]) |*f| {
            if (f.present()) work += f.st.survived;
        }
        for (b.units[w][0..max_squad], 0..) |*f, i| {
            if (f.unit.type != 0 and work > 0) out.exp[i] += @divTrunc(pool * 4 * f.st.survived, 10 * work);
        }
        pool -= @divTrunc(pool * 4, 10);
        work = b.summon_work[w];
        for (&b.units[w]) |*f| {
            if (f.present()) work += f.st.work();
        }
        for (b.units[w][0..max_squad], 0..) |*f, i| {
            if (f.unit.type == 0) continue;
            if (work > 0) out.exp[i] += @divTrunc(pool * f.st.work(), work);
            if ((i > 0 or win_hero == null) and f.life > 0 and b.unitKind(f) != kind_undead) out.medals[i] = b.rollMedal(w, i);
        }
        if (hero_alive) {
            const hu = &b.units[w][0];
            out.exp[0] += @divTrunc(out.exp[0] * b.skill(hu, 991), 100);
            out.gold += @divTrunc(out.gold * b.skill(hu, 992), 10);
            out.gem += @divTrunc(out.gem * b.skill(hu, 992), 10);
        }
        out.gold += @divTrunc(out.gold * ec.trophy_bonus, 100);
        out.gem += @divTrunc(out.gem * ec.trophy_bonus, 100);

        for (&b.armies[w].squad, 0..) |*slot, i| {
            if (slot.*) |*u| if (b.units[w][i].unit.type != 0 and (i > 0 or win_hero == null)) {
                u.exp += out.exp[i];
            };
        }
        b.keepSquads(w, true);
        for (b.units[w][0..max_squad]) |*f| {
            if (f.unit.type != 0 and f.life > 0 and b.skill(f, 56) > 0) {
                out.gold -= @divTrunc(out.gold * b.skill(f, 56), 100);
                out.gem -= @divTrunc(out.gem * b.skill(f, 56), 100);
            }
        }
        b.keepSquads(l, false);

        // The loot.
        const looter: i32 = if (hero_alive) b.skill(&b.units[w][0], 992) else 0;
        if (win_hero) |wh| {
            if (ec.loot_pct) |pct| {
                out.gold = @divTrunc(out.gold * pct, 100);
                out.gem = @divTrunc(out.gem * pct, 100);
            }
            out.gold = @divTrunc(out.gold, 5);
            out.gem = @divTrunc(out.gem, 5);
            if (ec.static_loot_seed) |sd| b.r.seed(sd);
            if (lose_hero) |lh| {
                if (b.units[l][0].life == 0) {
                    out.loot[0] = hero_mod.dropLoot(b.r, &lh.inventory, looter);
                    if (out.loot[0][0] > 0) {
                        out.loot[1] = hero_mod.dropLoot(b.r, &lh.inventory, looter);
                        if (out.loot[1][0] == out.loot[0][0]) out.loot[1][0] = 0;
                    } else out.loot[0] = hero_mod.dropLoot(b.r, &lh.inventory, looter);
                }
            } else {
                out.karma -= b.armies[l].guard_karma;
                b.guardLoot(l, looter, out);
            }
            // Level-ups and medals of the winner's units.
            for (&b.armies[w].squad, 0..) |*slot, i| {
                const u = &(slot.* orelse continue);
                if (u.life <= 0 or i == 0) continue;
                while (u.level < u.countLevel(b.pr)) u.makeLevel(b.pr, u.levelUp(b.pr, b.r, u.level + 1, 0));
                if (out.medals[i] > 0) for (&u.medals) |*m| if (m.* == 0) {
                    m.* = out.medals[i];
                    break;
                };
            }
            var k: usize = 0;
            for (out.loot) |it| {
                if (it[0] <= 0) continue;
                if (!hero_mod.takeLoot(b.pr, wh, &wh.inventory, it[0], it[1])) {
                    out.loot_left[k] = it;
                    k += 1;
                }
            }
            b.checkQuests(wh, ec, &.{ 2, 3, 8 });
        } else {
            out.gold = 0;
            out.gem = 0;
            // A guard's units level up too.
            for (&b.armies[w].squad, 0..) |*slot, i| {
                const u = &(slot.* orelse continue);
                if (u.life <= 0) continue;
                while (u.level < u.countLevel(b.pr)) u.makeLevel(b.pr, u.levelUp(b.pr, b.r, u.level + 1, 0));
                if (out.medals[i] > 0) for (&u.medals) |*m| if (m.* == 0) {
                    m.* = out.medals[i];
                    break;
                };
            }
        }
        if (lose_hero) |lh| b.checkQuests(lh, ec, &.{ 3, 8 });
    }

    /// The loot of a guard: LootPoss (+ the looter's skill) for a drop,
    /// up to LootNum items, each from the loot table of a random unit of
    /// the guard up to its level.
    fn guardLoot(b: *Battle, l: u1, looter: i32, out: *Outcome) void {
        const g = b.armies[l].guard;
        if (g == 0) return;
        var poss: i32 = 0;
        var num: i32 = 0;
        if (g > 0) {
            if (g >= b.pr.guards.len) return;
            poss = b.pr.guards[@intCast(g)].loot_poss + looter;
            num = b.pr.guards[@intCast(g)].loot_num;
        } else {
            if (-g >= b.pr.defenders.len) return;
            poss = b.pr.defenders[@intCast(-g)].loot_poss + looter;
            num = b.pr.defenders[@intCast(-g)].loot_num;
        }
        if (rnd(b.r, 100) >= poss) return;
        var items: i32 = 1;
        while (items < num and rnd(b.r, 100) + looter > 50) items += 1;
        const squad = &b.armies[l].squad;
        var r_i: usize = 0;
        while (r_i < @as(usize, @intCast(items)) and r_i < max_loot) : (r_i += 1) {
            var rl: unit_mod.Roller = .{};
            var n: usize = 0;
            for (squad, 0..) |slot, i| {
                const u = slot orelse continue;
                if (b.pr.units[@intCast(u.type)].loot[0][0].id <= 0) continue;
                rl.ids[n] = @intCast(i + 1);
                rl.weights[n] = 10;
                n += 1;
            }
            if (n == 0) continue;
            const dropper = squad[@intCast(rl.roll(b.r) - 1)].?;
            rl = .{};
            n = 0;
            const ut = &b.pr.units[@intCast(dropper.type)];
            var lv: usize = 0;
            while (lv <= @as(usize, @intCast(@max(dropper.level, 0))) and lv < proto.unit_levels) : (lv += 1) {
                for (ut.loot[lv]) |e| {
                    if (e.id <= 0) continue;
                    rl.ids[n] = e.id;
                    rl.weights[n] = e.value * 2 + @as(i32, @intCast(lv));
                    n += 1;
                }
            }
            const item = rl.roll(b.r);
            if (item <= 0 or item >= b.pr.items.len) continue;
            out.loot[r_i] = .{ item, rnd(b.r, b.pr.items[@intCast(item)].durability) + 1 };
        }
    }

    /// Reward[i] of EndBattle: the last medal whose roll passes.
    fn rollMedal(b: *Battle, w: u1, i: usize) i32 {
        const f = &b.units[w][i];
        const st = f.st;
        const r = b.r;
        const kills = st.shot + st.slain;
        const taken = st.melee_taken + st.range_taken + st.spell_taken + st.poison_taken;
        const half = @divTrunc(f.life, 2);
        var reward: i32 = 0;
        const stamina = b.typeStat(f, stat.stamina);
        if (st.stamina_used >= stamina and rndU(r, 21 + st.stamina_used - stamina) >= 20) reward = 5;
        const orig_morale: i32 = if (b.armies[w].squad[i]) |u| u.morale else 0;
        const mm = b.maxMorale(f);
        if (f.morale - mm - @divTrunc(orig_morale, 2) > 5 and kills > 0 and rndU(r, 21 + f.morale - mm) > 30) reward = 1;
        if (kills > 0 and st.range_taken > half and rndU(r, 21 + st.range_taken - half) > 20) reward = 11;
        if (st.slain > 0 and st.melee_taken > half and rndU(r, 21 + st.melee_taken - half) > 20) reward = 12;
        if (kills > 0 and st.spell_taken > half and rndU(r, 21 + st.spell_taken - half) > 20) reward = 13;
        if (kills > 0) {
            var survived: i32 = 0;
            var count: i32 = 0;
            const has_hero = b.classHero(w) != null;
            for (b.units[w][0..max_squad], 0..) |*o, j| {
                if (o.unit.type == 0) continue;
                count += 1;
                if (o.life > 0 and (j > 0 or !has_hero)) survived += 1;
            }
            if (survived > 0 and @divTrunc(count, survived) > 5 and rndU(r, 21 + count + kills) > 20) reward = 10;
        }
        const ml = b.maxLife(f);
        if (taken >= ml and kills > 0 and rndU(r, 21 + taken - ml) >= 20) reward = 2;
        const mel = (b.typeStat(f, stat.attack) + b.typeStat(f, stat.counter)) * 3;
        if (st.melee_done > mel and rndU(r, 21 + st.melee_done - mel) > 20) reward = 4;
        const hl = b.skill(f, ab.healer);
        if (st.healed >= hl * 3 and hl > 0 and rndU(r, 21 + st.healed - hl * 3) >= 20) {
            if (rnd(r, 1 + medalCount(f, 9)) == 0) reward = 9;
        }
        const ammo = b.typeStat(f, stat.ammo);
        const ranged = b.typeStat(f, stat.ranged);
        const magic_shot = b.skill(f, ab.magic_shot);
        if (ammo > 2 and ranged > 0 and st.range_done >= ranged * 3 and magic_shot == 0 and b.skill(f, 46) == 0 and b.skill(f, 47) == 0 and st.shot > 1) {
            if (rndU(r, 21 + st.range_done + st.shot * 5 - ranged * 3) >= 30 and rnd(r, 1 + medalCount(f, 3)) == 0) reward = 3;
        }
        if (st.spells >= 3 and st.shot > 0 and rndU(r, 10 + st.spells + st.shot) >= 10) {
            if (rnd(r, 1 + medalCount(f, 14)) == 0) reward = 14;
        }
        if (st.range_done >= ranged * 3 and magic_shot > 0 and st.shot > 1) {
            if (rndU(r, 21 + st.range_done + st.shot * 5 - ranged * 3) >= 30 and rnd(r, 1 + medalCount(f, 7)) == 0) reward = 7;
        }
        if (kills > 4 and taken >= @divTrunc(ml * 2, 3) and rndU(r, 11 + kills) >= 13) reward = 8;
        if (ammo > 2 and ranged > 0 and st.range_done >= ranged * 3 and magic_shot == 0 and st.shot > 1 and taken >= @divTrunc(ml * 2, 3)) {
            if (rndU(r, 21 + st.range_done + st.shot * 5 - ranged * ammo * 3) >= 30 and rnd(r, 1 + medalCount(f, 15)) == 0) reward = 15;
        }
        return reward;
    }

    fn medalCount(f: *const Fighter, m: i32) i32 {
        var n: i32 = 0;
        for (f.unit.medals) |x| n += @intFromBool(x == m);
        return n;
    }
};

/// Random(Max) with Max taken as the original's unsigned long: a negative
/// argument becomes a huge bound instead of 0.
fn rndU(r: *Rand, max: i32) u32 {
    return r.random(@bitCast(max));
}

/// Hex distance on rows shifted by half a cell (odd rows to the right).
pub fn hexDistance(x1: i32, y1: i32, x2: i32, y2: i32) i32 {
    const q1 = x1 - @divFloor(y1 - @rem(y1, 2), 2);
    const q2 = x2 - @divFloor(y2 - @rem(y2, 2), 2);
    const dq = q1 - q2;
    const dr = y1 - y2;
    return @divTrunc(@as(i32, @intCast(@abs(dq) + @abs(dr) + @abs(dq + dr))), 2);
}

// testdata/hero_move: the hero in province 25 against the guard of the
// neutral province 35, both sides played by aiStep until the end.
test "a whole battle of the hero against a province guard" {
    const tst = std.testing;
    const gpa = tst.allocator;
    const GameFs = @import("../platform/fs.zig").GameFs;
    const packfile = @import("../formats/packfile.zig");
    const loadgame = @import("loadgame.zig");
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try proto.Proto.load(gpa, &fs);
    defer pr.deinit();
    const read = struct {
        fn f(a: std.mem.Allocator, path: []const u8) ![]u8 {
            const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, a, .unlimited) catch return error.SkipZigTest;
            defer a.free(bytes);
            return packfile.unpack(a, bytes);
        }
    }.f;
    const mdata = try read(gpa, "testdata/hero_move/map0.map");
    defer gpa.free(mdata);
    const gdata = try read(gpa, "testdata/hero_move/game0.sav");
    defer gpa.free(gdata);
    var g = try loadgame.fromBytes(gpa, &pr, mdata, gdata);
    defer g.deinit();
    const s = &g.shard;
    const h = &s.heroes.items[0];
    const p1 = &s.provinces[@intCast(h.province)];
    const p2 = &s.provinces[35];
    try tst.expect(p2.guard >= 0);
    const gd = &s.guards.items[@intCast(p2.guard)];

    var armies: [2]Army = .{
        .{ .hero = h, .initiative = h.heroBonus(&pr, hero_mod.bonus.initiative), .player = h.owner, .squad = h.squad },
        .{ .initiative = gd.initiative, .squad = gd.squad },
    };
    var r: Rand = .{};
    const field = generate(&pr, .{ .seed = 111, .prov1 = .{ .number = p1.number, .terrain = p1.terrain }, .prov2 = .{ .number = p2.number, .terrain = p2.terrain } });
    for (field.cells) |col| for (col) |c| try tst.expect(c.land >= 1 and c.land < pr.bf_objects.len);
    var b = Battle.init(gpa, &pr, &r, field, armies);
    defer b.deinit();
    try tst.expect(b.firstAlive(0) != null and b.firstAlive(1) != null);
    // Every unit stands on its own cell.
    for (0..2) |sa| for (&b.units[sa]) |*f| {
        if (!f.alive()) continue;
        try tst.expect(!b.grid(f.x, f.y).pass);
        try tst.expect(b.maxLife(f) >= f.life and f.life > 0);
    };
    b.side = 1 - b.firstSide();
    b.endTurn();
    var steps: usize = 0;
    while (!b.over() and steps < 5000) : (steps += 1) {
        if (!b.aiStep()) b.endTurn();
    }
    try tst.expect(b.over());
    try tst.expect(b.anims.items.len > 0);
    const out = b.finish(.{});
    try tst.expect(out.result >= -2 and out.result <= 1);
    armies = b.armies;
    for (armies) |a| for (a.squad) |slot| if (slot) |u| try tst.expect(u.life >= 0);
}

// The same battle with each spell of spell.var in the hero's book: the
// computer side casts it without breaking the model, and a damage spell
// is cast at all.
test "every spell in the hero's book" {
    const tst = std.testing;
    const gpa = tst.allocator;
    const GameFs = @import("../platform/fs.zig").GameFs;
    const packfile = @import("../formats/packfile.zig");
    const loadgame = @import("loadgame.zig");
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try proto.Proto.load(gpa, &fs);
    defer pr.deinit();
    const read = struct {
        fn f(a: std.mem.Allocator, path: []const u8) ![]u8 {
            const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, a, .unlimited) catch return error.SkipZigTest;
            defer a.free(bytes);
            return packfile.unpack(a, bytes);
        }
    }.f;
    const mdata = try read(gpa, "testdata/hero_move/map0.map");
    defer gpa.free(mdata);
    const gdata = try read(gpa, "testdata/hero_move/game0.sav");
    defer gpa.free(gdata);
    var g = try loadgame.fromBytes(gpa, &pr, mdata, gdata);
    defer g.deinit();
    const s = &g.shard;
    var hero = s.heroes.items[0];
    if (hero.class1 <= 0) return error.SkipZigTest;
    const p1 = &s.provinces[@intCast(hero.province)];
    const p2 = &s.provinces[35];
    const gd = &s.guards.items[@intCast(p2.guard)];
    var damage_cast = false;
    for (1..pr.spells.len) |sp_u| {
        const sp: i32 = @intCast(sp_u);
        @memset(hero.rec[0x210 .. 0x210 + 4 * (spellbook_size + 1)], 0);
        std.mem.writeInt(i32, hero.rec[0x214..][0..4], sp, .little);
        const armies: [2]Army = .{
            .{ .hero = &hero, .initiative = 100, .player = hero.owner, .gem = 10000, .squad = hero.squad },
            .{ .initiative = gd.initiative, .squad = gd.squad },
        };
        var r: Rand = .{};
        const field = generate(&pr, .{ .seed = 111, .prov1 = .{ .number = p1.number, .terrain = p1.terrain }, .prov2 = .{ .number = p2.number, .terrain = p2.terrain } });
        var b = Battle.init(gpa, &pr, &r, field, armies);
        defer b.deinit();
        try tst.expectEqual(sp, b.book[0][0][0]);
        b.side = 1 - b.firstSide();
        b.endTurn();
        var steps: usize = 0;
        var cast = false;
        while (!b.over() and steps < 5000) : (steps += 1) {
            if (!b.aiStep()) b.endTurn();
            for (b.anims.items) |a| if (a == .cast) {
                cast = true;
            };
            b.anims.clearRetainingCapacity();
            for (0..2) |sd| for (&b.units[sd]) |*f| {
                if (!f.alive()) continue;
                try tst.expect(f.life <= b.maxLife(f));
                try tst.expect(inside(f.x, f.y));
            };
        }
        try tst.expect(b.over());
        _ = b.finish(.{});
        const sd = &pr.spells[sp_u];
        if (cast and sd.target == 1 and sd.on_enemy and sd.effects[0].id == 1 and sd.effects[0].duration == -1) damage_cast = true;
        if (cast) try tst.expect(b.gem_spent[0] > 0 or sd.cost == 0);
    }
    try tst.expect(damage_cast);
}

// Reincarnation brings a unit back with full life; the dark pact turns
// it into the demon of its level and puts the demon in the hero's squad.
test "reincarnation and the dark pact on death" {
    const tst = std.testing;
    const gpa = tst.allocator;
    const GameFs = @import("../platform/fs.zig").GameFs;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try proto.Proto.load(gpa, &fs);
    defer pr.deinit();
    var r: Rand = .{};
    var t: i32 = 1;
    while (t < pr.units.len and pr.units[@intCast(t)].level != 2) t += 1;
    if (t >= pr.units.len) return error.SkipZigTest;
    var armies: [2]Army = .{ .{}, .{} };
    for (0..2) |s| {
        var u = unit_mod.Unit.init(&pr, &r, t, 0);
        u.x = 0;
        u.y = 0;
        armies[s].squad[0] = u;
        u.y = 1;
        armies[s].squad[1] = u;
    }
    var b = Battle.init(gpa, &pr, &r, .{}, armies);
    defer b.deinit();
    const ra: Ref = .{ .side = 0, .slot = 0 };
    b.fighter(ra).addEffect(.{ .kind = eff_reincarnate, .power = 1, .time = 0 });
    try tst.expect(b.takeDamage(ra, b.fighter(ra).life, 0));
    try tst.expectEqual(b.maxLife(b.fighter(ra)), b.fighter(ra).life);
    try tst.expectEqual(@as(usize, 0), b.fighter(ra).n_eff);

    const rp: Ref = .{ .side = 0, .slot = 1 };
    b.fighter(rp).addEffect(.{ .kind = eff_dark_pact, .power = 1, .time = 0 });
    try tst.expect(b.takeDamage(rp, b.fighter(rp).life, 0));
    try tst.expectEqual(demon_types[1], b.fighter(rp).unit.type);
    try tst.expect(b.fighter(rp).life > 0);
    try tst.expectEqual(demon_types[1], b.armies[0].squad[1].?.type);

    // An ordinary death stays.
    const rd: Ref = .{ .side = 1, .slot = 0 };
    try tst.expect(b.takeDamage(rd, b.fighter(rd).life, 0));
    try tst.expectEqual(@as(i32, 0), b.fighter(rd).life);
}

// GetSquadMorale: units of one karma sign cheer each other, mixed signs
// quarrel, a neutral unit takes half the mean of the others.
test "squad morale by karma" {
    const tst = std.testing;
    const gpa = tst.allocator;
    const GameFs = @import("../platform/fs.zig").GameFs;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try proto.Proto.load(gpa, &fs);
    defer pr.deinit();
    var good: i32 = 0;
    var evil: i32 = 0;
    for (pr.units[1..], 1..) |u, i| {
        const probe: unit_mod.Unit = .{ .type = @intCast(i), .life = 1, .exp = 0 };
        if (probe.checkAbility(&pr, hero_mod.ability_fearless) + probe.checkAbility(&pr, hero_mod.ability_no_discord) > 0) continue;
        if (u.karma == 1 and good == 0) good = @intCast(i);
        if (u.karma == -1 and evil == 0) evil = @intCast(i);
    }
    if (good == 0 or evil == 0) return error.SkipZigTest;
    const U = unit_mod.Unit;
    var sq: [max_squad]?U = @splat(null);
    sq[1] = .{ .type = good, .life = 1, .exp = 0 };
    sq[2] = .{ .type = good, .life = 1, .exp = 0 };
    // Same sign, no neutral: +1 and Bonus = |karma| - 1 = 0.
    try tst.expectEqual(@as(i32, 1), hero_mod.squadMorale(&pr, &sq, 1, null, null));
    sq[3] = .{ .type = evil, .life = 1, .exp = 0 };
    // Anti 1: -|karma| - 1/3.
    try tst.expectEqual(@as(i32, -1), hero_mod.squadMorale(&pr, &sq, 1, null, null));
    // The player's karma against the unit: -|karma|/50.
    try tst.expectEqual(@as(i32, -3), hero_mod.squadMorale(&pr, &sq, 1, -100, null));
}

// The human side's spell book: every spell of spell.var picked from the
// book, its target clicked where one fits, and paid for once.
test "a human casts every spell of the book" {
    const tst = std.testing;
    const gpa = tst.allocator;
    const bs = @import("battle_spell.zig");
    const GameFs = @import("../platform/fs.zig").GameFs;
    const packfile = @import("../formats/packfile.zig");
    const loadgame = @import("loadgame.zig");
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try proto.Proto.load(gpa, &fs);
    defer pr.deinit();
    const read = struct {
        fn f(a: std.mem.Allocator, path: []const u8) ![]u8 {
            const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, a, .unlimited) catch return error.SkipZigTest;
            defer a.free(bytes);
            return packfile.unpack(a, bytes);
        }
    }.f;
    const mdata = try read(gpa, "testdata/hero_move/map0.map");
    defer gpa.free(mdata);
    const gdata = try read(gpa, "testdata/hero_move/game0.sav");
    defer gpa.free(gdata);
    var g = try loadgame.fromBytes(gpa, &pr, mdata, gdata);
    defer g.deinit();
    const s = &g.shard;
    var hero = s.heroes.items[0];
    if (hero.class1 <= 0) return error.SkipZigTest;
    const p1 = &s.provinces[@intCast(hero.province)];
    const p2 = &s.provinces[35];
    const gd = &s.guards.items[@intCast(p2.guard)];
    var cast_n: usize = 0;
    for (1..pr.spells.len) |sp_u| {
        const sp: i32 = @intCast(sp_u);
        @memset(hero.rec[0x210 .. 0x210 + 4 * (spellbook_size + 1)], 0);
        std.mem.writeInt(i32, hero.rec[0x214..][0..4], sp, .little);
        const armies: [2]Army = .{
            .{ .hero = &hero, .initiative = 100, .player = hero.owner, .gem = 10000, .human = true, .squad = hero.squad },
            .{ .initiative = gd.initiative, .squad = gd.squad },
        };
        var r: Rand = .{};
        const field = generate(&pr, .{ .seed = 111, .prov1 = .{ .number = p1.number, .terrain = p1.terrain }, .prov2 = .{ .number = p2.number, .terrain = p2.terrain } });
        var b = Battle.init(gpa, &pr, &r, field, armies);
        defer b.deinit();
        b.side = 1;
        b.endTurn();
        const ar: Ref = .{ .side = 0, .slot = 0 };
        b.select(ar);
        try tst.expect(bs.hasBook(&b, ar));
        try tst.expect(bs.canOpenBook(&b, ar));
        const e = bs.book(&b, ar)[0];
        try tst.expectEqual(sp, e.spell);
        var pd = bs.pick(&b, ar, 0) orelse continue;
        var done = bs.prepare(&b, ar, &pd);
        // Click the first cell that takes it, twice for a sacrifice or a
        // teleport.
        var tries: usize = 0;
        while (!done and tries < 3) : (tries += 1) {
            var hit = false;
            outer: for (0..size) |x| for (0..size) |y| {
                const xi: i32 = @intCast(x);
                const yi: i32 = @intCast(y);
                if (bs.click(&b, ar, &pd, xi, yi, true) == .none) continue;
                const res = bs.click(&b, ar, &pd, xi, yi, false);
                try tst.expect(res != .none);
                done = res == .cast;
                hit = true;
                break :outer;
            };
            if (!hit) break;
        }
        if (!done) continue;
        cast_n += 1;
        try tst.expectEqual(@as(i32, 0), b.book[0][0][1]);
        try tst.expect(b.gem_spent[0] > 0 or pr.spells[sp_u].cost == 0);
        try tst.expect(!bs.book(&b, ar)[0].castable);
        for (0..2) |sd| for (&b.units[sd]) |*f| {
            if (!f.alive()) continue;
            try tst.expect(f.life <= b.maxLife(f));
        };
    }
    try tst.expect(cast_n > pr.spells.len / 2);
}

// Two heroes: the winner takes the loser's level as experience, gets its
// trophies and loot drops out of the loser's inventory.
test "a battle of two heroes" {
    const tst = std.testing;
    const gpa = tst.allocator;
    const GameFs = @import("../platform/fs.zig").GameFs;
    const packfile = @import("../formats/packfile.zig");
    const loadgame = @import("loadgame.zig");
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try proto.Proto.load(gpa, &fs);
    defer pr.deinit();
    const read = struct {
        fn f(a: std.mem.Allocator, path: []const u8) ![]u8 {
            const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, a, .unlimited) catch return error.SkipZigTest;
            defer a.free(bytes);
            return packfile.unpack(a, bytes);
        }
    }.f;
    const mdata = try read(gpa, "testdata/hero_move/map0.map");
    defer gpa.free(mdata);
    const gdata = try read(gpa, "testdata/hero_move/game0.sav");
    defer gpa.free(gdata);
    var g = try loadgame.fromBytes(gpa, &pr, mdata, gdata);
    defer g.deinit();
    var h1 = g.shard.heroes.items[0];
    if (h1.class1 <= 0) return error.SkipZigTest;
    var h2 = h1;
    // The second hero is weak: its own unit only, with an item to lose.
    for (h2.squad[1..]) |*u| u.* = null;
    h2.squad[0].?.life = 1;
    var item: i32 = 1;
    while (item < pr.items.len and pr.items[@intCast(item)].class != 0) item += 1;
    h2.inventory[16] = .{ item, 5 };
    var r: Rand = .{};
    const field = generate(&pr, .{ .seed = 111, .prov1 = .{ .number = 1, .terrain = 1 } });
    var b = Battle.init(gpa, &pr, &r, field, .{
        .{ .hero = &h1, .initiative = 100, .squad = h1.squad },
        .{ .hero = &h2, .squad = h2.squad },
    });
    defer b.deinit();
    b.side = 1;
    b.endTurn();
    var steps: usize = 0;
    while (!b.over() and steps < 5000) : (steps += 1) {
        if (!b.aiStep()) b.endTurn();
        b.anims.clearRetainingCapacity();
    }
    const out = b.finish(.{});
    try tst.expectEqual(@as(i32, 1), out.result);
    try tst.expect(out.exp[0] >= @divTrunc(h2.level * 10 * 3, 10));
    try tst.expect(out.gold >= 0 and out.gem >= 0);
    // A hero's loot is the inventory of the fallen one.
    for (out.loot) |it| if (it[0] > 0) try tst.expectEqual(item, it[0]);
}

test "EndBattle preserves fearless morale only for the winner" {
    const tst = std.testing;
    // Self-contained prototypes, so this regression needs no game assets.
    var types: [3]proto.UnitType = @splat(std.mem.zeroes(proto.UnitType));
    for (types[1..]) |*t| {
        t.life = 10;
        t.morale = 10;
        t.stamina = 10;
        t.exp_mod = 100;
        t.unit_kind = 1;
    }
    var upgrades: [2]proto.UnitUpgrade = @splat(std.mem.zeroes(proto.UnitUpgrade));
    upgrades[1].upg[0] = .{ .id = ab.fearless, .value = 1 };
    types[2].abilities[0] = 1;
    var spells: [1]proto.Spell = @splat(std.mem.zeroes(proto.Spell));
    var pr: proto.Proto = .{ .arena = .init(tst.allocator), .units = &types, .unit_upgrades = &upgrades, .spells = &spells };
    defer pr.arena.deinit();

    for ([_]i32{ 1, 0, -1, -2 }) |result| {
        for ([_]i32{ -4, 0, 6, 10, 14 }) |delta| {
            var r: Rand = .{};
            var army: Army = .{};
            army.squad[0] = .{ .type = 1, .life = 10, .exp = 0, .morale = 7 };
            army.squad[1] = .{ .type = 2, .life = 10, .exp = 0, .morale = 7 };
            var b = Battle.init(tst.allocator, &pr, &r, .{}, .{ army, army });
            defer b.deinit();
            for (0..2) |s| {
                const wins = result == 1 - @as(i32, @intCast(s));
                for (b.units[s][0..2]) |*f| {
                    f.life = if (wins or result == -2) 10 else 0;
                    f.morale = b.maxMorale(f) + delta;
                }
            }
            const out = b.finish(.{});
            try tst.expectEqual(result, out.result);
            for (0..2) |s| {
                const wins = result == 1 - @as(i32, @intCast(s));
                const expected = @min(delta, 10) * 2;
                try tst.expectEqual(expected, b.armies[s].squad[0].?.morale);
                try tst.expectEqual(if (wins) 7 else expected, b.armies[s].squad[1].?.morale);
            }
        }
    }
}

test "soul steal heals melee kills, including counters, but not shots or vanished summons" {
    const tst = std.testing;
    var types: [3]proto.UnitType = @splat(std.mem.zeroes(proto.UnitType));
    for (types[1..]) |*t| {
        t.life = 30;
        t.attack = 100;
        t.counter_attack = 100;
        t.ranged_attack = 100;
        t.ammo = 5;
        t.stamina = 10;
        t.morale = 10;
        t.unit_kind = 1;
        t.exp_mod = 100;
    }
    var pr: proto.Proto = .{ .arena = .init(tst.allocator), .units = &types };
    defer pr.arena.deinit();
    const Case = struct {
        mode: enum { attack, counter, first_strike, shot, circular } = .attack,
        kind: i32 = 1,
        power: i32 = 7,
        life: i32 = 10,
        target_life: i32 = 1,
        summoned: bool = false,
        reincarnate: bool = false,
        heal: i32 = 7,
    };
    const cases = [_]Case{
        .{},
        .{ .mode = .counter },
        .{ .mode = .first_strike },
        .{ .life = 28, .heal = 2 },
        .{ .life = 30, .heal = 0 },
        .{ .life = 32, .heal = 0 },
        .{ .power = 0, .heal = 0 },
        .{ .kind = kind_undead, .heal = 0 },
        .{ .kind = kind_construct, .heal = 0 },
        .{ .kind = 3 },
        .{ .summoned = true, .heal = 0 },
        .{ .reincarnate = true },
        .{ .summoned = true, .reincarnate = true },
        .{ .mode = .first_strike, .reincarnate = true, .heal = 0 },
        .{ .target_life = 1000, .heal = 0 },
        .{ .mode = .shot, .heal = 0 },
        .{ .mode = .circular, .heal = 0 },
    };
    for ([_]u1{ 0, 1 }) |side| for (cases) |case| {
        types[2].unit_kind = case.kind;
        var r: Rand = .{};
        var armies: [2]Army = .{ .{}, .{} };
        armies[side].squad[0] = .{ .type = 1, .life = 30, .exp = 0 };
        armies[1 - side].squad[0] = .{ .type = 2, .life = 30, .exp = 0 };
        var b = Battle.init(tst.allocator, &pr, &r, .{}, armies);
        defer b.deinit();
        b.simulate = true;
        const sr: Ref = .{ .side = side, .slot = 0 };
        const tr: Ref = .{ .side = 1 - side, .slot = 0 };
        const s = b.fighter(sr);
        const t = b.fighter(tr);
        s.x = 3;
        s.y = 3;
        t.x = 4;
        t.y = 3;
        s.life = case.life;
        t.life = case.target_life;
        t.summoned = case.summoned;
        s.addEffect(.{ .kind = ab.soul_steal, .power = case.power, .time = 0 });
        if (case.reincarnate) t.addEffect(.{ .kind = eff_reincarnate, .power = 1, .time = 0 });
        switch (case.mode) {
            .attack => {
                t.stamina = 0;
                _ = b.attackFrom(sr, s.x, s.y, tr);
            },
            .counter, .first_strike => {
                // The victim initiates combat; the soul stealer kills with
                // a counter/first strike. Its initial blow does no damage.
                types[2].attack = 0;
                defer types[2].attack = 100;
                s.addEffect(.{ .kind = stat.defence, .power = 1000, .time = 0 });
                if (case.mode == .first_strike) s.addEffect(.{ .kind = ab.first_strike, .power = 1, .time = 0 });
                _ = b.attackFrom(tr, t.x, t.y, sr);
            },
            .shot => b.shoot(sr, tr),
            .circular => {
                // Keep a separate primary target so only the secondary dies.
                const primary: Ref = .{ .side = tr.side, .slot = 1 };
                b.fighter(primary).* = t.*;
                b.fighter(primary).x = 3;
                b.fighter(primary).y = 4;
                s.addEffect(.{ .kind = ab.round_attack, .power = 1, .time = 0 });
                b.roundAttack(sr, primary, true);
                try tst.expectEqual(@as(i32, 0), t.life);
            },
        }
        try tst.expectEqual(case.life + case.heal, s.life);
        var heal_numbers: usize = 0;
        for (b.anims.items) |animation| switch (animation) {
            .number => |n| if (n.sound_unit == sound_soul) {
                heal_numbers += 1;
                try tst.expectEqualDeep(sr, n.who);
                try tst.expectEqual(case.heal, n.value);
                try tst.expectEqualDeep(col_soul, n.color);
            },
            else => {},
        };
        try tst.expectEqual(@as(usize, @intFromBool(case.heal > 0)), heal_numbers);
        // SoulSteal is not the healer action and does not award healing work.
        try tst.expectEqual(@as(i32, 0), s.st.healed);
    };
}

test "trample finishes only main blows and advances unless the victim resurrects" {
    const tst = std.testing;
    var types: [22]proto.UnitType = @splat(std.mem.zeroes(proto.UnitType));
    for (types[1..]) |*t| {
        t.life = 100;
        t.attack = 10;
        t.counter_attack = 10;
        t.ranged_attack = 10;
        t.ammo = 5;
        t.stamina = 10;
        t.morale = 10;
        t.unit_kind = 1;
        t.exp_mod = 100;
    }
    var upgrades: [93]proto.UnitUpgrade = @splat(std.mem.zeroes(proto.UnitUpgrade));
    var pr: proto.Proto = .{ .arena = .init(tst.allocator), .units = &types, .unit_upgrades = &upgrades };
    defer pr.arena.deinit();
    const Case = struct {
        mode: enum { attack, after_first_strike, counter, first_strike, shot, circular } = .attack,
        remainder: i32 = 4,
        power: i32 = 5,
        flying: bool = false,
        rooted: bool = false,
        resurrection: i32 = 0,
        summoned: bool = false,
        finish: bool = true,
        advance: bool = true,
    };
    const cases = [_]Case{
        .{},
        .{ .remainder = 5, .finish = false, .advance = false },
        .{ .remainder = 6, .finish = false, .advance = false },
        .{ .remainder = 0 },
        .{ .power = 0, .finish = false, .advance = false },
        .{ .power = 0, .remainder = 0, .advance = false },
        .{ .flying = true, .finish = false, .advance = false },
        .{ .flying = true, .remainder = 0, .advance = false },
        .{ .rooted = true, .finish = false, .advance = false },
        .{ .rooted = true, .remainder = 0, .advance = false },
        .{ .resurrection = eff_reincarnate, .advance = false },
        .{ .resurrection = eff_dark_pact, .advance = false },
        .{ .summoned = true },
        .{ .mode = .after_first_strike },
        .{ .mode = .counter, .finish = false, .advance = false },
        .{ .mode = .counter, .remainder = 0, .advance = false },
        .{ .mode = .first_strike, .finish = false, .advance = false },
        .{ .mode = .first_strike, .remainder = 0, .advance = false },
        .{ .mode = .shot, .finish = false, .advance = false },
        .{ .mode = .circular, .finish = false, .advance = false },
    };
    for ([_]u1{ 0, 1 }) |side| for (cases) |case| {
        var r: Rand = .{};
        var armies: [2]Army = .{ .{}, .{} };
        armies[side].squad[0] = .{ .type = 1, .life = 100, .exp = 0 };
        armies[1 - side].squad[0] = .{ .type = 2, .life = 100, .exp = 0 };
        var b = Battle.init(tst.allocator, &pr, &r, .{}, armies);
        defer b.deinit();
        b.simulate = true;
        const sr: Ref = .{ .side = side, .slot = 0 };
        const tr: Ref = .{ .side = 1 - side, .slot = 0 };
        const s = b.fighter(sr);
        const t = b.fighter(tr);
        b.setPass(s, true);
        b.setPass(t, true);
        s.x = 3;
        s.y = 3;
        t.x = 4;
        t.y = 3;
        b.setPass(s, false);
        b.setPass(t, false);
        s.addEffect(.{ .kind = ab.trample, .power = case.power, .time = 0 });
        // The opposing blow cannot damage the trampler or change its morale.
        s.addEffect(.{ .kind = stat.defence, .power = 1000, .time = 0 });
        if (case.flying) t.addEffect(.{ .kind = ab.flying, .power = 1, .time = 0 });
        if (case.rooted) s.addEffect(.{ .kind = ab.rooted, .power = 1, .time = 0 });
        if (case.resurrection != 0) t.addEffect(.{ .kind = case.resurrection, .power = 1, .time = 0 });
        t.summoned = case.summoned;
        const hit = if (case.mode == .shot) b.rangedHit(s, t) else b.meleeHit(s, t, case.mode != .counter and case.mode != .first_strike);
        t.life = hit + case.remainder;
        const initial_life = t.life;
        switch (case.mode) {
            .attack, .after_first_strike => {
                if (case.mode == .after_first_strike) t.addEffect(.{ .kind = ab.first_strike, .power = 1, .time = 0 });
                try tst.expect(b.attackFrom(sr, s.x, s.y, tr));
                try tst.expectEqual(@as(i32, 0), s.speed);
                try tst.expect(!s.can_act);
            },
            .counter, .first_strike => {
                if (case.mode == .first_strike) s.addEffect(.{ .kind = ab.first_strike, .power = 1, .time = 0 });
                _ = b.attackFrom(tr, t.x, t.y, sr);
            },
            .shot => b.shoot(sr, tr),
            .circular => {
                const primary: Ref = .{ .side = tr.side, .slot = 1 };
                b.fighter(primary).* = t.*;
                b.fighter(primary).x = 3;
                b.fighter(primary).y = 4;
                s.addEffect(.{ .kind = ab.round_attack, .power = 1, .time = 0 });
                b.roundAttack(sr, primary, true);
            },
        }
        try tst.expectEqual(@as(i32, if (case.advance) 4 else 3), s.x);
        try tst.expectEqual(@as(i32, 3), s.y);
        try tst.expectEqual(case.advance, b.field.at(3, 3).pass);
        try tst.expectEqual(case.finish and !case.advance and case.resurrection == 0, b.field.at(4, 3).pass);
        try tst.expectEqual(@as(i32, if (case.resurrection != 0) 100 else if (case.finish) 0 else case.remainder), t.life);
        if (case.mode != .shot and case.mode != .circular) {
            try tst.expectEqual(if (case.finish) initial_life else hit, s.st.melee_done);
            try tst.expectEqual(@as(i32, @intFromBool(case.finish)), s.st.slain);
        }
        var tramples: usize = 0;
        for (b.anims.items) |animation| switch (animation) {
            .melee => |m| if (m.trample) |cell| {
                tramples += 1;
                try tst.expectEqualDeep(sr, m.striker);
                try tst.expectEqualDeep(tr, m.target);
                try tst.expectEqualDeep([2]i32{ 4, 3 }, cell);
                try tst.expectEqual(initial_life, m.hit);
                try tst.expectEqual(@as(i32, 0), m.life);
            },
            else => {},
        };
        try tst.expectEqual(@as(usize, @intFromBool(case.advance)), tramples);
    };
}

test "raise corpse follows kills, terrain, summon limits and the killer's level" {
    const tst = std.testing;
    var types: [20]proto.UnitType = @splat(std.mem.zeroes(proto.UnitType));
    for (types[1..]) |*t| {
        t.life = 30;
        t.attack = 100;
        t.counter_attack = 100;
        t.ranged_attack = 100;
        t.ammo = 5;
        t.stamina = 10;
        t.morale = 10;
        t.unit_kind = 1;
        t.exp_mod = 100;
    }
    types[19].unit_kind = kind_undead;
    var upgrades: [93]proto.UnitUpgrade = @splat(std.mem.zeroes(proto.UnitUpgrade));
    var terrain: [2]proto.BfObject = @splat(std.mem.zeroes(proto.BfObject));
    var skills: [2]proto.Skill = @splat(std.mem.zeroes(proto.Skill));
    skills[1].hero[0][0] = .{ .id = 907, .value = 2 };
    skills[1].hero[0][1] = .{ .id = 909, .value = 3 };
    var items: [1]proto.Item = @splat(std.mem.zeroes(proto.Item));
    var pr: proto.Proto = .{
        .arena = .init(tst.allocator),
        .units = &types,
        .unit_upgrades = &upgrades,
        .bf_objects = &terrain,
        .skills = &skills,
        .items = &items,
    };
    defer pr.arena.deinit();
    const Case = struct {
        mode: enum { attack, counter, first_strike, shot, circular } = .attack,
        kind: i32 = 1,
        ability: bool = true,
        target_life: i32 = 1,
        summoned: bool = false,
        reincarnate: bool = false,
        polymorph: bool = false,
        charm: bool = false,
        move_cost: i32 = 1,
        full: bool = false,
        skip_slot: bool = false,
        hero: bool = false,
        hero_unit: bool = false,
        level: i32 = 0,
        raised_level: ?i32 = 0,
    };
    const cases = [_]Case{
        .{},
        .{ .mode = .counter },
        .{ .mode = .first_strike },
        .{ .mode = .shot },
        .{ .mode = .circular, .raised_level = null },
        .{ .ability = false, .raised_level = null },
        .{ .kind = kind_undead, .raised_level = null },
        .{ .kind = kind_construct, .raised_level = null },
        .{ .kind = 3 },
        .{ .summoned = true, .raised_level = null },
        .{ .reincarnate = true, .raised_level = null },
        .{ .mode = .first_strike, .reincarnate = true, .raised_level = null },
        .{ .mode = .shot, .reincarnate = true, .raised_level = null },
        .{ .target_life = 1000, .raised_level = null },
        .{ .move_cost = 0, .raised_level = null },
        .{ .move_cost = -1, .raised_level = null },
        .{ .full = true, .raised_level = null },
        .{ .skip_slot = true },
        .{ .level = 3 },
        .{ .level = 4, .raised_level = 1 },
        .{ .level = 11, .raised_level = 2 },
        .{ .hero = true, .hero_unit = true, .level = 12, .raised_level = 5 },
        .{ .hero = true, .level = 12, .raised_level = 3 },
        // Death restores the true form before testing its kind.
        .{ .polymorph = true, .kind = kind_undead },
        .{ .charm = true, .raised_level = null },
    };
    for ([_]u1{ 0, 1 }) |side| for (cases) |case| {
        types[2].unit_kind = case.kind;
        terrain[1].move_cost = case.move_cost;
        var h = std.mem.zeroes(hero_mod.Hero);
        h.class1 = 1;
        h.skills[0] = .{ 1, 1 };
        var r: Rand = .{};
        var armies: [2]Army = .{ .{}, .{} };
        armies[side].squad[0] = .{ .type = 1, .life = 30, .exp = 0 };
        armies[1 - side].squad[0] = .{ .type = 2, .life = 30, .exp = 0 };
        var b = Battle.init(tst.allocator, &pr, &r, .{}, armies);
        defer b.deinit();
        b.simulate = true;
        const sr: Ref = .{ .side = side, .slot = 0 };
        const tr: Ref = .{ .side = 1 - side, .slot = 0 };
        const s = b.fighter(sr);
        const t = b.fighter(tr);
        s.x = 3;
        s.y = 3;
        t.x = 4;
        t.y = 3;
        t.life = case.target_life;
        t.summoned = case.summoned;
        s.unit.level = case.level;
        s.hero_unit = case.hero_unit;
        if (case.hero) b.armies[side].hero = &h;
        if (case.ability) s.addEffect(.{ .kind = ab.raise_corpse, .power = 1, .time = 0 });
        if (case.reincarnate) t.addEffect(.{ .kind = eff_reincarnate, .power = 1, .time = 0 });
        if (case.polymorph) {
            t.true_form = .{ .type = 1, .life = 30, .exp = 0 };
            t.addEffect(.{ .kind = eff_polymorph, .power = 1, .time = 0 });
        }
        if (case.charm) t.addEffect(.{ .kind = eff_charm, .power = 1, .time = 0 });
        if (case.full) for (b.units[side][max_squad..]) |*f| {
            f.used = true;
        };
        if (case.skip_slot) b.units[side][max_squad].used = true;
        switch (case.mode) {
            .attack => {
                t.stamina = 0;
                _ = b.attackFrom(sr, s.x, s.y, tr);
            },
            .counter, .first_strike => {
                s.addEffect(.{ .kind = stat.defence, .power = 1000, .time = 0 });
                if (case.mode == .first_strike) s.addEffect(.{ .kind = ab.first_strike, .power = 1, .time = 0 });
                _ = b.attackFrom(tr, t.x, t.y, sr);
            },
            .shot => b.shoot(sr, tr),
            .circular => {
                const primary: Ref = .{ .side = tr.side, .slot = 1 };
                b.fighter(primary).* = t.*;
                b.fighter(primary).x = 3;
                b.fighter(primary).y = 4;
                s.addEffect(.{ .kind = ab.round_attack, .power = 1, .time = 0 });
                b.roundAttack(sr, primary, true);
                try tst.expectEqual(@as(i32, 0), t.life);
            },
        }
        var appearances: usize = 0;
        for (b.anims.items) |animation| switch (animation) {
            .appear => |shown| {
                appearances += 1;
                try tst.expect(shown.rise);
                try tst.expectEqual(side, shown.who.side);
                try tst.expectEqual(@as(u8, max_squad) + @intFromBool(case.skip_slot), shown.who.slot);
            },
            else => {},
        };
        try tst.expectEqual(@as(usize, @intFromBool(case.raised_level != null)), appearances);
        if (case.raised_level) |level| {
            const skeleton = &b.units[side][@as(usize, max_squad) + @intFromBool(case.skip_slot)];
            try tst.expectEqual(@as(i32, 19), skeleton.unit.type);
            try tst.expectEqual(level, skeleton.unit.level);
            try tst.expect(skeleton.alive() and skeleton.summoned and skeleton.can_act);
            try tst.expectEqual(@as(i32, 4), skeleton.x);
            try tst.expectEqual(@as(i32, 3), skeleton.y);
            try tst.expect(!b.field.at(4, 3).pass);
            try tst.expect(!t.in_battle);
            try tst.expectEqual(@as(i32, 0), t.life);
        } else {
            for (b.units[side][max_squad..]) |f| try tst.expectEqual(@as(i32, 0), f.unit.type);
            if (case.full or !case.ability or case.mode == .circular) try tst.expect(t.in_battle);
        }
    };
}

// Summon gives ability 97 the same effect 74 Init gives a squad unit,
// and the screen event asks for the rise and the purple stars. A type
// without the ability gets neither.
test "a summon with innate reincarnation rises once" {
    const tst = std.testing;
    var types: [3]proto.UnitType = @splat(std.mem.zeroes(proto.UnitType));
    for (types[1..]) |*t| {
        t.life = 20;
        t.stamina = 8;
        t.morale = 6;
        t.speed = 3;
        t.unit_kind = 1;
        t.exp_mod = 100;
    }
    var upgrades: [2]proto.UnitUpgrade = @splat(std.mem.zeroes(proto.UnitUpgrade));
    upgrades[1].upg[0] = .{ .id = ab.reincarnation, .value = 1 };
    var pr: proto.Proto = .{ .arena = .init(tst.allocator), .units = &types, .unit_upgrades = &upgrades };
    defer pr.arena.deinit();
    var r: Rand = .{};
    var b = Battle.init(tst.allocator, &pr, &r, .{}, .{ .{}, .{} });
    defer b.deinit();

    const born: Ref = .{ .side = 0, .slot = max_squad };
    types[1].abilities[0] = 1;
    b.summon(born, 1, 0, 4, 2);
    const f = b.fighter(born);
    try tst.expect(f.summoned and f.alive());
    try tst.expectEqual(@as(usize, 1), f.n_eff);
    const e = f.effects[0];
    try tst.expectEqual(eff_reincarnate, e.kind);
    try tst.expectEqual(@as(i32, 1), e.power);
    try tst.expectEqual(@as(i32, 0), e.time);
    try tst.expectEqual(@as(i32, 59), e.spell);
    try tst.expect(e.visible and !e.destroy_on_hit);
    try tst.expectEqual(@as(usize, 1), b.anims.items.len);
    switch (b.anims.items[0]) {
        .appear => |shown| {
            try tst.expect(shown.rise);
            try tst.expectEqualDeep(born, shown.who);
        },
        else => return error.TestExpectedEqual,
    }

    // The effect is spent by the first death, as it is for a squad unit.
    try tst.expect(b.takeDamage(born, f.life, 0));
    try tst.expectEqual(b.maxLife(f), f.life);
    try tst.expectEqual(@as(usize, 0), f.n_eff);
    try tst.expect(b.takeDamage(born, f.life, 0));
    try tst.expectEqual(@as(i32, 0), f.life);

    b.anims.clearRetainingCapacity();
    const plain: Ref = .{ .side = 1, .slot = max_squad };
    types[2].abilities[0] = 0;
    b.summon(plain, 2, 0, 3, 5);
    try tst.expectEqual(@as(usize, 0), b.fighter(plain).n_eff);
    // Every summon rises on the screen; only ability 97 adds the effect.
    switch (b.anims.items[0]) {
        .appear => |shown| try tst.expect(shown.rise),
        else => return error.TestExpectedEqual,
    }
}

test "morale bonus" {
    try std.testing.expectEqual(@as(i32, -60), moraleBonus(0));
    try std.testing.expectEqual(@as(i32, -10), moraleBonus(5));
    try std.testing.expectEqual(@as(i32, 0), moraleBonus(10));
    // 15 + 1 + 2 + 3 = 21: three steps.
    try std.testing.expectEqual(@as(i32, 15), moraleBonus(21));
}

test "a blow never goes below 0 and a strong one passes the defence" {
    var r: Rand = .{};
    for (0..200) |_| {
        const h = getHit(&r, 3, 20);
        try std.testing.expect(h == 0);
        const g = getHit(&r, 20, 5);
        try std.testing.expect(g >= 11 and g <= 19);
    }
}

// Ability 65 damages every other enemy next to the primary target.  The
// primary target itself must not receive the melee hit twice.
test "circular melee attack hits the other neighbours" {
    const tst = std.testing;
    const gpa = tst.allocator;
    const GameFs = @import("../platform/fs.zig").GameFs;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = proto.Proto.load(gpa, &fs) catch return error.SkipZigTest;
    defer pr.deinit();

    var r: Rand = .{};
    var round_type: i32 = 0;
    var target_type: i32 = 0;
    for (1..pr.units.len) |i| {
        var probe = unit_mod.Unit.init(&pr, &r, @intCast(i), 0);
        if (round_type == 0 and probe.checkAbility(&pr, ab.round_attack) > 0) round_type = @intCast(i);
        if (target_type == 0) target_type = @intCast(i);
        if (round_type != 0 and target_type != 0) break;
    }
    if (round_type == 0 or target_type == 0) return error.SkipZigTest;

    var attacker = unit_mod.Unit.init(&pr, &r, round_type, 0);
    attacker.x = 3;
    attacker.y = 3;
    var primary = unit_mod.Unit.init(&pr, &r, target_type, 0);
    primary.x = 3;
    primary.y = 4;
    var secondary = primary;
    secondary.x = 4;
    secondary.y = 4;
    const armies: [2]Army = .{
        .{ .squad = .{ attacker, null } ++ @as([max_squad - 2]?unit_mod.Unit, @splat(null)) },
        .{ .squad = .{ primary, secondary } ++ @as([max_squad - 2]?unit_mod.Unit, @splat(null)) },
    };
    var b = Battle.init(gpa, &pr, &r, .{}, armies);
    defer b.deinit();
    b.simulate = true;
    const sr: Ref = .{ .side = 0, .slot = 0 };
    const tr: Ref = .{ .side = 1, .slot = 0 };
    const nr: Ref = .{ .side = 1, .slot = 1 };
    b.fighter(sr).x = 3;
    b.fighter(sr).y = 3;
    b.fighter(tr).x = 3;
    b.fighter(tr).y = 4;
    b.fighter(nr).x = 4;
    b.fighter(nr).y = 4;
    const primary_life = b.fighter(tr).life;
    const secondary_life = b.fighter(nr).life;
    b.roundAttack(sr, tr, true);
    try tst.expectEqual(primary_life, b.fighter(tr).life);
    var secondary_numbers: usize = 0;
    for (b.anims.items) |animation| switch (animation) {
        .number => |number| {
            if (number.who.side == nr.side and number.who.slot == nr.slot) secondary_numbers += 1;
        },
        else => {},
    };
    try tst.expectEqual(@as(usize, 1), secondary_numbers);
    try tst.expect(b.fighter(nr).life <= secondary_life);
}

test "roots on hit respect immunity, survival and the existing ability effect" {
    const tst = std.testing;
    const gpa = tst.allocator;
    const GameFs = @import("../platform/fs.zig").GameFs;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try proto.Proto.load(gpa, &fs);
    defer pr.deinit();
    // Keep the unit's stats, but supply only the abilities under test.
    pr.units[1].abilities = @splat(0);

    const Case = struct {
        power: i32 = 3,
        hit: i32 = 0,
        immune: bool = false,
        self: bool = false,
        previous: ?Effect = null,
        count: usize = 1,
        time: i32 = 3,
    };
    const cases = [_]Case{
        .{},
        .{ .power = 0, .count = 0 },
        .{ .immune = true, .count = 0 },
        .{ .hit = 10, .count = 0 },
        .{ .hit = 11, .count = 0 },
        .{ .hit = 9 },
        .{ .self = true, .count = 0 },
        .{ .previous = .{ .kind = ab.rooted, .power = 1, .time = 1, .visible = true, .ability = upg_icon.roots } },
        .{ .previous = .{ .kind = ab.rooted, .power = 1, .time = 5, .visible = true, .ability = upg_icon.roots }, .time = 5 },
        .{ .previous = .{ .kind = ab.rooted, .power = 1, .time = 3, .visible = true, .ability = upg_icon.roots } },
        // Spell roots have no ability icon: they must remain independent.
        .{ .previous = .{ .kind = ab.rooted, .power = 1, .time = 7, .spell = 1 }, .count = 2 },
    };
    for (cases) |case| {
        var r: Rand = .{};
        const u = unit_mod.Unit.init(&pr, &r, 1, 0);
        const army: Army = .{ .squad = .{u} ++ @as([max_squad - 1]?unit_mod.Unit, @splat(null)) };
        var b = Battle.init(gpa, &pr, &r, .{}, .{ army, army });
        defer b.deinit();
        const a = b.fighter(.{ .side = 0, .slot = 0 });
        const d = b.fighter(.{ .side = 1, .slot = 0 });
        a.addEffect(.{ .kind = ab.roots, .power = case.power, .time = 0 });
        d.life = 10;
        if (case.immune) d.addEffect(.{ .kind = ab.roots_immunity, .power = 1, .time = 0 });
        if (case.previous) |e| d.addEffect(e);
        const before = d.n_eff;
        b.hitEffect(a, if (case.self) a else d, case.hit, null);
        try tst.expectEqual(case.count + @as(usize, @intFromBool(case.immune)), d.n_eff);
        if (case.previous) |e| {
            if (e.ability != upg_icon.roots) try tst.expectEqualDeep(e, d.effects[0]);
        }
        if (case.count > 0) {
            const e = d.effects[d.n_eff - 1];
            try tst.expectEqualDeep(Effect{
                .kind = ab.rooted,
                .power = 1,
                .time = case.time,
                .visible = true,
                .ability = upg_icon.roots,
            }, e);
            try tst.expectEqual(@as(i32, if (case.count == 2) 2 else 1), b.skill(d, ab.rooted));
        } else {
            try tst.expectEqual(before, d.n_eff);
            try tst.expectEqual(@as(i32, 0), b.skill(a, ab.rooted));
            try tst.expectEqual(@as(i32, 0), b.skill(d, ab.rooted));
        }
    }
}

test "crippling bite and curse refresh their ability effects without stacking" {
    const tst = std.testing;
    var types: [2]proto.UnitType = @splat(std.mem.zeroes(proto.UnitType));
    types[1].life = 10;
    types[1].unit_kind = 1;
    types[1].exp_mod = 100;
    types[1].resist = 3;
    var pr: proto.Proto = .{ .arena = .init(tst.allocator), .units = &types };
    defer pr.arena.deinit();

    const Case = struct {
        ability: i32 = ab.curse,
        power: i32 = 6,
        previous: ?i32 = null,
        time: i32 = 3,
        repeat_time: i32 = 4,
        applies: bool = true,
        immune: bool = false,
        self: bool = false,
        side: u1 = 0,
    };
    const cases = [_]Case{
        .{},
        .{ .side = 1 },
        .{ .previous = 1, .time = 4 },
        .{ .previous = 7, .time = 7, .repeat_time = 7 },
        .{ .power = 3, .applies = false },
        .{ .power = 2, .applies = false },
        .{ .power = 0, .applies = false },
        .{ .immune = true, .applies = false },
        .{ .immune = true, .previous = 1, .applies = false },
        .{ .self = true, .applies = false },
        .{ .ability = ab.cripple, .time = 2, .repeat_time = 2 },
        .{ .ability = ab.cripple, .previous = 1, .time = 2, .repeat_time = 2 },
        // Bite resets to exactly two turns, unlike curse's maximum.
        .{ .ability = ab.cripple, .previous = 7, .time = 2, .repeat_time = 2 },
        .{ .ability = ab.cripple, .immune = true, .time = 2, .repeat_time = 2 },
        .{ .ability = ab.cripple, .power = 0, .applies = false },
        .{ .ability = ab.cripple, .self = true, .applies = false },
    };
    for (cases) |case| {
        var r: Rand = .{};
        var b = Battle.init(tst.allocator, &pr, &r, .{}, .{ .{}, .{} });
        defer b.deinit();
        const a = b.fighter(.{ .side = case.side, .slot = 2 });
        const d = b.fighter(.{ .side = 1 - case.side, .slot = 3 });
        a.unit = .{ .type = 1, .life = 10, .exp = 0 };
        d.unit = a.unit;
        a.life = 10;
        d.life = 10;
        a.addEffect(.{ .kind = case.ability, .power = case.power, .time = 0 });
        if (case.immune) d.addEffect(.{ .kind = ab.fire_arrows, .power = 1, .time = 0 });
        const stats: []const i32 = if (case.ability == ab.cripple) &.{stat.speed} else &.{ stat.attack, stat.defence, stat.ranged_defence, stat.resist };
        const icon: i32 = if (case.ability == ab.cripple) upg_icon.cripple else upg_icon.curse;
        // A spell affecting the same stat must not be refreshed by the bite/curse.
        const spell: Effect = .{ .kind = stats[0], .power = -1, .time = 9, .spell = 1, .visible = true };
        d.addEffect(spell);
        const start = d.n_eff;
        if (case.previous) |time| {
            for (stats, 0..) |kind, n| Battle.timed(d, kind, -1, time, icon, n == 0);
        }
        const before = d.n_eff;
        for (0..2) |strike| {
            b.hitEffect(a, if (case.self) a else d, 0, null);
            const has_effect = case.applies or case.previous != null;
            try tst.expectEqual(if (has_effect) start + stats.len else before, d.n_eff);
            try tst.expectEqualDeep(spell, d.effects[start - 1]);
            if (has_effect) {
                const time = if (!case.applies) case.previous.? else if (strike == 0) case.time else case.repeat_time;
                for (stats, 0..) |kind, n| {
                    try tst.expectEqualDeep(Effect{
                        .kind = kind,
                        .power = -1,
                        .time = time,
                        .ability = icon,
                        .visible = n == 0,
                    }, d.effects[start + n]);
                    try tst.expectEqual(@as(i32, if (n == 0) -2 else -1), Battle.effect(d, kind));
                }
            }
            try tst.expectEqual(@as(u8, 1), a.n_eff);
        }
    }
}

test "hit spells preserve resistance boundaries and their distinct immunities" {
    const tst = std.testing;
    // Self-contained prototypes: these regressions run without imported assets.
    var types: [2]proto.UnitType = @splat(std.mem.zeroes(proto.UnitType));
    types[1].life = 10;
    types[1].unit_kind = 1;
    types[1].exp_mod = 100;
    var spells: [31]proto.Spell = @splat(std.mem.zeroes(proto.Spell));
    for ([_]usize{ 1, 21, 30 }) |i| {
        spells[i].negative = true;
        spells[i].effects[0] = .{ .id = stat.defence, .power = -1, .duration = 3 };
        spells[i].n_effects = 1;
    }
    var pr: proto.Proto = .{ .arena = .init(tst.allocator), .units = &types, .spells = &spells };
    defer pr.arena.deinit();

    const Case = struct {
        ability: i32 = ab.vulnerability,
        spell: i32 = 21,
        resist: i32 = 2,
        kind: i32 = 1,
        fire: bool = false,
        anti: bool = false,
        immune_kind: bool = false,
        bypass: bool = false,
        self: bool = false,
        enabled: bool = true,
        casts: bool = true,
        side: u1 = 0,
        resist_duration: bool = false,
        magic_bonus: bool = false,
        time: i32 = 3,
        power: i32 = -1,
    };
    const cases = [_]Case{
        .{},
        .{ .resist = 3 },
        .{ .resist = 4, .casts = false },
        .{ .fire = true, .casts = false },
        .{ .enabled = false, .casts = false },
        .{ .self = true, .casts = false },
        .{ .kind = kind_undead },
        .{ .kind = kind_construct },
        // These two abilities do not consult the spell's immunity lists.
        .{ .anti = true, .immune_kind = true },
        .{ .ability = ab.disease, .spell = 30 },
        .{ .ability = ab.disease, .spell = 30, .resist = 3 },
        .{ .ability = ab.disease, .spell = 30, .resist = 4, .casts = false },
        .{ .ability = ab.disease, .spell = 30, .fire = true, .casts = false },
        .{ .ability = ab.disease, .spell = 30, .kind = kind_undead, .casts = false },
        .{ .ability = ab.disease, .spell = 30, .kind = kind_construct, .casts = false },
        .{ .ability = ab.disease, .spell = 30, .kind = 3 },
        .{ .ability = ab.disease, .spell = 30, .kind = 5 },
        .{ .ability = ab.spell_strike, .spell = 1 },
        .{ .ability = ab.spell_strike, .spell = 1, .resist = 3, .casts = false },
        .{ .ability = ab.spell_strike, .spell = 1, .resist = 4, .casts = false },
        .{ .ability = ab.spell_strike, .spell = 1, .fire = true },
        .{ .ability = ab.spell_strike, .spell = 1, .anti = true, .casts = false },
        .{ .ability = ab.spell_strike, .spell = 1, .immune_kind = true, .casts = false },
        .{ .ability = ab.spell_strike, .spell = 1, .anti = true, .bypass = true, .casts = false },
        .{ .ability = ab.spell_strike, .spell = 1, .immune_kind = true, .bypass = true, .casts = false },
        // Resolve both the side and the actual slot, not the hero in slot 0.
        .{ .side = 1 },
        .{ .ability = ab.spell_strike, .spell = 1, .side = 1 },
        // UnderSpell still applies resistance and the attacker's magic bonuses.
        .{ .resist_duration = true, .time = 1 },
        .{ .ability = ab.disease, .spell = 30, .resist_duration = true, .time = 1 },
        .{ .ability = ab.spell_strike, .spell = 1, .resist_duration = true, .time = 1 },
        .{ .magic_bonus = true, .power = -2, .time = 4 },
    };
    for (cases) |case| {
        types[1].unit_kind = case.kind;
        spells[@intCast(case.spell)].anti_effects[0] = if (case.anti) ab.poison_immunity else 0;
        spells[@intCast(case.spell)].immune_kinds[0] = if (case.immune_kind) case.kind else 0;
        spells[@intCast(case.spell)].resist_duration = case.resist_duration;
        spells[@intCast(case.spell)].resist_mod = 100;
        spells[@intCast(case.spell)].power_mod = 100;
        spells[@intCast(case.spell)].duration_mod = 100;
        var r: Rand = .{};
        var b = Battle.init(tst.allocator, &pr, &r, .{}, .{ .{}, .{} });
        defer b.deinit();
        // Hit spells are not normal casts: no book, gems or casting permission.
        b.can_cast = .{ 0, 0 };
        const a = b.fighter(.{ .side = case.side, .slot = 2 });
        const d = if (case.self) a else b.fighter(.{ .side = 1 - case.side, .slot = 3 });
        a.unit = .{ .type = 1, .life = 10, .exp = 0 };
        d.unit = a.unit;
        a.life = 10;
        d.life = 10;
        if (case.enabled) a.addEffect(.{ .kind = case.ability, .power = if (case.ability == ab.spell_strike) case.spell else 1, .time = 0 });
        if (case.magic_bonus) {
            a.addEffect(.{ .kind = 905, .power = 1, .time = 0 });
            a.addEffect(.{ .kind = 906, .power = 1, .time = 0 });
        }
        d.addEffect(.{ .kind = stat.resist, .power = case.resist, .time = 0 });
        if (case.fire) d.addEffect(.{ .kind = ab.fire_arrows, .power = 1, .time = 0 });
        if (case.anti) d.addEffect(.{ .kind = ab.poison_immunity, .power = 1, .time = 0 });
        if (case.bypass) d.addEffect(.{ .kind = 205, .power = case.spell, .time = 0 });
        const before = d.n_eff;
        b.hitEffect(a, d, 0, null);
        try tst.expectEqual(before + @intFromBool(case.casts), d.n_eff);
        if (case.casts) {
            try tst.expectEqualDeep(Effect{
                .kind = stat.defence,
                .power = case.power,
                .time = case.time,
                .spell = case.spell,
                .visible = true,
            }, d.effects[d.n_eff - 1]);
            // Repeated strikes reuse UnderSpell's non-cumulative effect.
            b.hitEffect(a, d, 0, null);
            try tst.expectEqual(before + 1, d.n_eff);
        }
        if (!case.self) try tst.expectEqual(@as(i32, 0), Battle.effect(a, stat.defence));
    }
}

test "spell shots apply on each shot with resistance and immunity checks" {
    const tst = std.testing;
    var types: [2]proto.UnitType = @splat(std.mem.zeroes(proto.UnitType));
    types[1].life = 100;
    types[1].unit_kind = 1;
    types[1].exp_mod = 100;
    var spells: [2]proto.Spell = @splat(std.mem.zeroes(proto.Spell));
    spells[1].negative = true;
    spells[1].effects[0] = .{ .id = stat.defence, .power = -1, .duration = 3 };
    spells[1].n_effects = 1;
    spells[1].resist_mod = 100;
    spells[1].power_mod = 100;
    spells[1].duration_mod = 100;
    var pr: proto.Proto = .{ .arena = .init(tst.allocator), .units = &types, .spells = &spells };
    defer pr.arena.deinit();

    const Case = struct {
        spell: i32 = 1,
        resist: i32 = 2,
        immune_kind: bool = false,
        anti: bool = false,
        bypass: bool = false,
        side: u1 = 0,
        double: bool = false,
        ammo: i32 = 2,
        cumulative: bool = false,
        bonus: bool = false,
        resist_duration: bool = false,
        casts: usize = 1,
        power: i32 = -1,
        time: i32 = 3,
    };
    const cases = [_]Case{
        .{},
        .{ .resist = 3, .casts = 0 },
        .{ .resist = 4, .casts = 0 },
        .{ .spell = 0, .casts = 0 },
        .{ .spell = -1, .casts = 0 },
        .{ .spell = 2, .casts = 0 },
        .{ .immune_kind = true, .casts = 0 },
        .{ .anti = true, .casts = 0 },
        .{ .immune_kind = true, .bypass = true, .casts = 0 },
        .{ .anti = true, .bypass = true, .casts = 0 },
        .{ .side = 1 },
        .{ .double = true },
        .{ .double = true, .cumulative = true, .casts = 2 },
        .{ .double = true, .cumulative = true, .ammo = 1 },
        .{ .ammo = 0, .casts = 0 },
        .{ .bonus = true, .power = -2, .time = 4 },
        // The initial gate uses raw duration, before the caster's bonuses.
        .{ .resist = 3, .bonus = true, .casts = 0 },
        .{ .resist_duration = true, .time = 1 },
    };
    for (cases) |case| {
        spells[1].immune_kinds[0] = if (case.immune_kind) 1 else 0;
        spells[1].anti_effects[0] = if (case.anti) ab.roots_immunity else 0;
        spells[1].cumulative = case.cumulative;
        spells[1].resist_duration = case.resist_duration;
        var r: Rand = .{};
        var b = Battle.init(tst.allocator, &pr, &r, .{}, .{ .{}, .{} });
        defer b.deinit();
        b.can_cast = .{ 0, 0 };
        b.double_shot = case.double;
        const sr: Ref = .{ .side = case.side, .slot = 2 };
        const tr: Ref = .{ .side = 1 - case.side, .slot = 3 };
        const a = b.fighter(sr);
        const d = b.fighter(tr);
        a.unit = .{ .type = 1, .life = 100, .exp = 0 };
        d.unit = a.unit;
        a.life = 100;
        d.life = 100;
        a.ammo = case.ammo;
        a.stamina = 10;
        d.stamina = 10;
        a.addEffect(.{ .kind = ab.spell_shot, .power = case.spell, .time = 0 });
        // Existing shot effects must still work alongside the spell.
        a.addEffect(.{ .kind = ab.poison_shot, .power = 1, .time = 0 });
        a.addEffect(.{ .kind = ab.heavy_shot, .power = 2, .time = 0 });
        if (case.bonus) {
            a.addEffect(.{ .kind = 905, .power = 1, .time = 0 });
            a.addEffect(.{ .kind = 906, .power = 1, .time = 0 });
        }
        d.addEffect(.{ .kind = stat.resist, .power = case.resist, .time = 0 });
        if (case.anti) d.addEffect(.{ .kind = ab.roots_immunity, .power = 1, .time = 0 });
        if (case.bypass) d.addEffect(.{ .kind = 205, .power = 1, .time = 0 });
        const before = d.n_eff;
        b.shoot(sr, tr);
        const shots = @min(case.ammo, @as(i32, if (case.double) 2 else 1));
        try tst.expectEqual(case.ammo - shots, a.ammo);
        try tst.expectEqual(@as(usize, before) + @as(usize, @intCast(shots * 2)) + case.casts, d.n_eff);
        var casts: usize = 0;
        for (d.effectList()) |e| {
            if (e.spell != 1) continue;
            casts += 1;
            try tst.expectEqual(case.power, e.power);
            try tst.expectEqual(case.time, e.time);
        }
        try tst.expectEqual(case.casts, casts);
        try tst.expectEqual(@as(i32, 0), Battle.effect(a, stat.defence));
        try tst.expect(!a.can_act);
    }
}

test "petrification and web shots replace damage and prolong their effects" {
    const tst = std.testing;
    var types: [2]proto.UnitType = @splat(std.mem.zeroes(proto.UnitType));
    types[1].life = 100;
    types[1].exp_mod = 100;
    types[1].speed = 3;
    types[1].attack = 5;
    var pr: proto.Proto = .{ .arena = .init(tst.allocator), .units = &types };
    defer pr.arena.deinit();
    const Case = struct {
        ability: i32 = ab.petrify,
        power: i32 = 6,
        resist: i32 = 2,
        immune: bool = false,
        side: u1 = 0,
        double: bool = false,
        previous: ?i32 = null,
        spell_web: bool = false,
        applies: bool = true,
        time: i32 = 4,
    };
    const cases = [_]Case{
        .{},
        .{ .side = 1, .double = true },
        .{ .resist = 6, .applies = false },
        .{ .resist = 7, .applies = false },
        .{ .immune = true, .applies = false },
        // The existing stone form itself adds three resistance.
        .{ .previous = 2, .time = 2 },
        .{ .previous = 2, .power = 10, .time = 5 },
        .{ .previous = 8, .time = 8 },
        .{ .ability = ab.web, .time = 3 },
        .{ .ability = ab.web, .power = 5, .time = 2 },
        .{ .ability = ab.web, .power = 4, .time = 2 },
        .{ .ability = ab.web, .resist = 100, .time = 3 },
        .{ .ability = ab.web, .immune = true, .applies = false },
        .{ .ability = ab.web, .side = 1, .double = true, .time = 3 },
        .{ .ability = ab.web, .previous = 7, .time = 8 },
        .{ .ability = ab.web, .previous = 1, .spell_web = true, .time = 2 },
        .{ .ability = ab.web, .previous = 7, .spell_web = true, .time = 8 },
    };
    for (cases) |case| {
        var r: Rand = .{};
        var b = Battle.init(tst.allocator, &pr, &r, .{}, .{ .{}, .{} });
        defer b.deinit();
        b.double_shot = case.double;
        const sr: Ref = .{ .side = case.side, .slot = 2 };
        const tr: Ref = .{ .side = 1 - case.side, .slot = 3 };
        const a = b.fighter(sr);
        const d = b.fighter(tr);
        for ([_]*Fighter{ a, d }) |f| {
            f.unit = .{ .type = 1, .life = 100, .exp = 0 };
            f.life = 100;
            f.stamina = 10;
            f.speed = 3;
            f.morale = 10;
            f.can_act = true;
        }
        a.ammo = 2;
        a.addEffect(.{ .kind = case.ability, .power = case.power, .time = 0 });
        // Replacement attacks must not trigger ordinary ShootEffect.
        a.addEffect(.{ .kind = ab.poison_shot, .power = 1, .time = 0 });
        a.addEffect(.{ .kind = ab.heavy_shot, .power = 2, .time = 0 });
        a.addEffect(.{ .kind = 20, .power = 2, .time = 0 });
        d.addEffect(.{ .kind = stat.resist, .power = case.resist, .time = 0 });
        if (case.immune) d.addEffect(.{ .kind = if (case.ability == ab.web) ab.web_immunity else ab.petrify_immunity, .power = 1, .time = 0 });
        // An unrelated spell effect must survive unchanged.
        d.addEffect(.{ .kind = stat.defence, .power = 2, .time = 9, .spell = 7 });
        const before = d.n_eff;
        const stone = case.ability == ab.petrify;
        const icon: i32 = if (stone) upg_icon.petrify else upg_icon.web;
        const kinds = [_]i32{ ab.immobile, stat.defence, stat.ranged_defence, ab.no_combat, stat.resist };
        const powers = if (stone) [_]i32{ 1, 3, 3, 1, 3 } else [_]i32{ 1, 1, 1, 1, 0 };
        const count: usize = if (stone) 5 else 4;
        if (case.previous) |time| {
            for (kinds[0..count], powers[0..count], 0..) |kind, power, i| {
                d.addEffect(.{ .kind = kind, .power = power, .time = time, .visible = i == 0, .ability = if (case.spell_web) 0 else icon, .spell = if (case.spell_web) 18 else 0 });
            }
            d.speed = 0;
            d.can_act = false;
        }
        b.shoot(sr, tr);
        try tst.expectEqual(@as(i32, 100), d.life);
        try tst.expectEqual(@as(i32, 0), a.st.range_done);
        try tst.expectEqual(@as(i32, 0), d.st.range_taken);
        try tst.expectEqual(@as(i32, 1), a.ammo);
        try tst.expectEqual(@as(i32, if (case.double) 7 else 9), a.stamina);
        try tst.expectEqual(@as(i32, 0), a.speed);
        try tst.expect(!a.can_act);
        try tst.expectEqual(before + (if (case.applies) count else @as(usize, 0)), d.n_eff);
        try tst.expectEqual(@as(i32, 9), d.effects[before - 1].time);
        try tst.expectEqual(!case.applies, d.can_act);
        try tst.expectEqual(@as(i32, if (case.applies) 0 else 3), d.speed);
        for (d.effects[before..d.n_eff], 0..) |ef, i| {
            try tst.expectEqual(kinds[i], ef.kind);
            try tst.expectEqual(powers[i], ef.power);
            try tst.expectEqual(case.time, ef.time);
            try tst.expectEqual(i == 0, ef.visible);
            try tst.expectEqual(@as(i32, if (case.spell_web) 18 else 0), ef.spell);
            try tst.expectEqual(@as(i32, if (case.spell_web) 0 else icon), ef.ability);
        }
        try tst.expectEqualDeep(Anim{ .binding = .{ .shooter = sr, .target = tr, .stone = stone, .success = case.applies } }, b.anims.items[0]);
        try tst.expectEqual(@as(usize, if (case.applies) 1 else 2), b.anims.items.len);
        if (!case.applies) {
            try tst.expectEqualDeep(Anim{ .text = .{ .who = tr, .phrase = battle_spell.ph_resisted, .color = battle_spell.col_resisted } }, b.anims.items[1]);
        }
    }
}

test "web spell and ability prolong one another without stacking" {
    const tst = std.testing;
    var types: [2]proto.UnitType = @splat(std.mem.zeroes(proto.UnitType));
    types[1].life = 100;
    types[1].exp_mod = 100;
    var spells: [19]proto.Spell = @splat(std.mem.zeroes(proto.Spell));
    spells[18].effects[0] = .{ .id = 47, .power = 1, .duration = 4 };
    spells[18].n_effects = 1;
    spells[18].resist_mod = 100;
    var pr: proto.Proto = .{ .arena = .init(tst.allocator), .units = &types, .spells = &spells };
    defer pr.arena.deinit();
    for ([_]bool{ false, true }) |spell_first| {
        var r: Rand = .{};
        var b = Battle.init(tst.allocator, &pr, &r, .{}, .{ .{}, .{} });
        defer b.deinit();
        const sr: Ref = .{ .side = 0, .slot = 2 };
        const tr: Ref = .{ .side = 1, .slot = 3 };
        const a = b.fighter(sr);
        const d = b.fighter(tr);
        a.unit = .{ .type = 1, .life = 100, .exp = 0 };
        d.unit = a.unit;
        a.life = 100;
        d.life = 100;
        a.ammo = 3;
        a.stamina = 10;
        a.addEffect(.{ .kind = ab.web, .power = 6, .time = 0 });
        if (spell_first) battle_spell.underSpell(&b, tr, 18, sr) else b.shoot(sr, tr);
        try tst.expectEqual(@as(usize, 4), d.n_eff);
        const time = d.effects[0].time;
        if (spell_first) b.shoot(sr, tr) else battle_spell.underSpell(&b, tr, 18, sr);
        try tst.expectEqual(@as(usize, 4), d.n_eff);
        for (d.effectList()) |ef| {
            try tst.expectEqual(time + 1, ef.time);
            try tst.expectEqual(@as(i32, if (spell_first) 18 else 0), ef.spell);
            try tst.expectEqual(@as(i32, if (spell_first) 0 else upg_icon.web), ef.ability);
        }
        // The spell still respects resistance even when ability web exists.
        d.addEffect(.{ .kind = stat.resist, .power = 4, .time = 0 });
        battle_spell.underSpell(&b, tr, 18, sr);
        try tst.expectEqual(time + 1, d.effects[0].time);
    }
}

test "hex distance" {
    try std.testing.expectEqual(@as(i32, 1), hexDistance(0, 0, 0, 1));
    try std.testing.expectEqual(@as(i32, 1), hexDistance(1, 1, 1, 0));
    try std.testing.expectEqual(@as(i32, 7), hexDistance(0, 3, 7, 3));
}

test "unit and cell placement" {
    try std.testing.expectEqual(@as(i32, 18), transX(0, 0, 64));
    try std.testing.expectEqual(@as(i32, 70 + 104), transX(1, 1, 64));
    try std.testing.expectEqual(@as(i32, 18 - 8), transX(0, 0, 80));
    try std.testing.expectEqual(@as(i32, 15 + 90), transY(0, 1, 80));
    try std.testing.expectEqual(@as(i32, 15 + 90 - 20), transY(0, 1, 100));
}

test "tile type of screen points" {
    var f: Field = .{};
    f.cells[0][0].land = 2;
    f.cells[1][1].land = 3;
    f.cells[7][7].land = 4;
    try std.testing.expectEqual(@as(i32, 2), f.tileType(52, 45));
    try std.testing.expectEqual(@as(i32, 3), f.tileType(52 + 104 + 52, 90 + 45));
    try std.testing.expectEqual(@as(i32, 4), f.tileType(2000, 2000));
}

test "castle wall line" {
    var f: Field = .{};
    fortify(&f, .{ .percent = 100, .towers = 4 });
    try std.testing.expectEqual(@as(i32, land_tower), f.cells[5][0].land);
    try std.testing.expect(!f.cells[5][0].pass);
    try std.testing.expectEqual(@as(i32, land_wall), f.cells[5][1].land);
    f = .{};
    fortify(&f, .{ .percent = 0, .towers = 0 });
    for (0..size) |j| try std.testing.expectEqual(@as(i32, land_breach), f.cells[5][j].land);
}

/// The notes a blow shows (HitEffect, TakeDamage, the crush and the
/// circular attack), flattened in the order the screen plays them.
fn blowNotes(b: *const Battle) [16]Note {
    var out: [16]Note = @splat(.{});
    var n: usize = 0;
    for (b.anims.items) |a| {
        const notes: []const Note = switch (a) {
            .melee => |m| m.notes[0..m.n_notes],
            .shoot => |m| m.notes[0..m.n_notes],
            .number => |m| m.notes[0..m.n_notes],
            else => continue,
        };
        for (notes) |note| {
            if (n < out.len) out[n] = note;
            n += 1;
        }
    }
    return out;
}

test "a blow shows its vampire, roots, berserk, crush and circular attack" {
    const tst = std.testing;
    var types: [3]proto.UnitType = @splat(std.mem.zeroes(proto.UnitType));
    for (types[1..]) |*t| {
        t.life = 400;
        t.attack = 100;
        // Leaves the rolled blow around 10: above 9 and below a quarter of
        // the 400 life, so the target survives and shakes into a berserk.
        t.defence = 90;
        t.counter_attack = 100;
        t.ranged_attack = 100;
        t.ammo = 5;
        t.stamina = 10;
        t.morale = 10;
        t.unit_kind = 1;
        t.exp_mod = 100;
    }
    var upgrades: [2]proto.UnitUpgrade = @splat(std.mem.zeroes(proto.UnitUpgrade));
    // Ability 25 on type 2: the berserk of TakeDamage, not an active effect.
    upgrades[1].upg[0] = .{ .id = ab.berserk, .value = 5 };
    types[2].abilities[0] = 1;
    var pr: proto.Proto = .{ .arena = .init(tst.allocator), .units = &types, .unit_upgrades = &upgrades };
    defer pr.arena.deinit();

    // A plain blow drains and roots, and the berserk waits 20 ticks.
    for ([_]u1{ 0, 1 }) |side| {
        var r: Rand = .{};
        var armies: [2]Army = .{ .{}, .{} };
        armies[side].squad[0] = .{ .type = 1, .life = 40, .exp = 0 };
        armies[1 - side].squad[0] = .{ .type = 2, .life = 40, .exp = 0 };
        var b = Battle.init(tst.allocator, &pr, &r, .{}, armies);
        defer b.deinit();
        const sr: Ref = .{ .side = side, .slot = 0 };
        const tr: Ref = .{ .side = 1 - side, .slot = 0 };
        const s = b.fighter(sr);
        const t = b.fighter(tr);
        s.x = 3;
        s.y = 3;
        t.x = 4;
        t.y = 3;
        s.life = 200;
        t.life = 400;
        // No stamina: the victim does not counter, so the drain alone moves
        // the attacker's life.  The defence is 0 either way.
        t.stamina = 0;
        t.ammo = 4;
        s.addEffect(.{ .kind = ab.drain, .power = 50, .time = 0 });
        s.addEffect(.{ .kind = ab.roots, .power = 3, .time = 0 });
        s.addEffect(.{ .kind = ab.steal_ammo, .power = 2, .time = 0 });
        _ = b.attackFrom(sr, s.x, s.y, tr);
        try tst.expect(t.life > 0);
        const notes = blowNotes(&b);
        // The stolen ammo and the roots, then the drain, then the berserk.
        try tst.expectEqual(tr, notes[0].who);
        try tst.expectEqual(@as(u32, 0), notes[0].sound_unit);
        try tst.expectEqual(tr, notes[1].who);
        try tst.expectEqual(sound_root, notes[1].sound_unit);
        try tst.expectEqual(@as(i32, 2), t.ammo);
        try tst.expectEqual(sr, notes[2].who);
        try tst.expectEqual(@as(u32, 0), notes[2].phrase);
        try tst.expect(notes[2].value > 0);
        try tst.expectEqual(col_drain, notes[2].color);
        try tst.expectEqual(@as(i32, 200) + notes[2].value, s.life);
        try tst.expectEqual(ph_berserk, notes[3].phrase);
        try tst.expectEqual(sound_berserk, notes[3].sound_unit);
        try tst.expectEqual(@as(u8, 20), notes[3].delay);
        try tst.expectEqual(col_berserk, notes[3].color);
    }

    // The pressed buttons announce the blow only when a side target is hit.
    {
        var r: Rand = .{};
        var armies: [2]Army = .{ .{}, .{} };
        armies[0].squad[0] = .{ .type = 1, .life = 40, .exp = 0 };
        armies[1].squad[0] = .{ .type = 2, .life = 40, .exp = 0 };
        armies[1].squad[1] = .{ .type = 2, .life = 40, .exp = 0 };
        var b = Battle.init(tst.allocator, &pr, &r, .{}, armies);
        defer b.deinit();
        b.simulate = true;
        const sr: Ref = .{ .side = 0, .slot = 0 };
        const tr: Ref = .{ .side = 1, .slot = 0 };
        const nr: Ref = .{ .side = 1, .slot = 1 };
        const s = b.fighter(sr);
        s.x = 3;
        s.y = 3;
        b.fighter(tr).x = 4;
        b.fighter(tr).y = 3;
        b.fighter(nr).x = 3;
        b.fighter(nr).y = 4;
        for ([_]*Fighter{ b.fighter(tr), b.fighter(nr) }) |f| f.stamina = 0;
        s.addEffect(.{ .kind = ab.drain, .power = 100, .time = 0 });
        b.crush = true;
        b.round_attack = true;
        _ = b.attackFrom(sr, s.x, s.y, tr);
        const notes = blowNotes(&b);
        var crush_n: usize = 0;
        var round_n: usize = 0;
        var drain: i32 = 0;
        for (notes) |note| {
            if (note.phrase == ph_crush) {
                crush_n += 1;
                try tst.expectEqual(sound_crush, note.sound_unit);
                try tst.expectEqual(sr, note.who);
            }
            if (note.phrase == ph_round) {
                round_n += 1;
                try tst.expectEqual(sound_round, note.sound_unit);
            }
            if (note.color[0] == col_drain[0] and note.phrase == 0) drain += note.value;
        }
        try tst.expectEqual(@as(usize, 1), crush_n);
        try tst.expectEqual(@as(usize, 1), round_n);
        // The main target feeds the one vampire number of the blow.
        try tst.expect(drain > 0);
    }

    // A shot that shakes the target carries the berserk with the missile.
    {
        var r: Rand = .{};
        var armies: [2]Army = .{ .{}, .{} };
        armies[0].squad[0] = .{ .type = 1, .life = 40, .exp = 0 };
        armies[1].squad[0] = .{ .type = 2, .life = 40, .exp = 0 };
        var b = Battle.init(tst.allocator, &pr, &r, .{}, armies);
        defer b.deinit();
        b.simulate = true;
        const sr: Ref = .{ .side = 0, .slot = 0 };
        const tr: Ref = .{ .side = 1, .slot = 0 };
        b.fighter(sr).ammo = 3;
        // Full life: the shot does not kill, so the berserk shows.
        b.fighter(tr).life = 400;
        b.shoot(sr, tr);
        const shot = for (b.anims.items) |a| {
            if (a == .shoot) break a.shoot;
        } else unreachable;
        try tst.expectEqual(@as(u8, 1), shot.n_notes);
        try tst.expectEqual(ph_berserk, shot.notes[0].phrase);
        try tst.expectEqual(sound_berserk, shot.notes[0].sound_unit);
        try tst.expectEqual(tr, shot.notes[0].who);
    }

    // Healing and the march name their sounds.
    {
        var r: Rand = .{};
        var armies: [2]Army = .{ .{}, .{} };
        armies[0].squad[0] = .{ .type = 1, .life = 40, .exp = 0 };
        armies[0].squad[1] = .{ .type = 1, .life = 40, .exp = 0 };
        var b = Battle.init(tst.allocator, &pr, &r, .{}, armies);
        defer b.deinit();
        b.simulate = true;
        const sr: Ref = .{ .side = 0, .slot = 0 };
        const tr: Ref = .{ .side = 0, .slot = 1 };
        b.fighter(sr).addEffect(.{ .kind = ab.healer, .power = 6, .time = 0 });
        b.fighter(sr).ammo = 2;
        b.fighter(tr).life = 10;
        b.heal(sr, tr);
        b.fighter(sr).addEffect(.{ .kind = ab.march, .power = 2, .time = 0 });
        b.goMarch(sr);
        var heal_sound: u32 = 0;
        var march_sound: u32 = 0;
        for (b.anims.items) |a| switch (a) {
            .number => |n| heal_sound = n.sound_unit,
            .text => |tx| march_sound = tx.sound_unit,
            else => {},
        };
        try tst.expectEqual(sound_heal, heal_sound);
        try tst.expectEqual(sound_march, march_sound);
        try tst.expectEqual(@as(i32, 16), b.fighter(tr).life);
    }
}
