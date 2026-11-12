//! Spells on the battlefield: BF_Unit::UnderSpell, CorpseSpell, TileSpell,
//! BF_Unit::Replace and SpellCastAI (what a spell of the computer side
//! costs its caster).
//!
//! Reincarnation (74) and the dark pact (91) act in Battle.death.
//!
//! Not ported: the stars and the ice, web and stone pictures.
const std = @import("std");
const proto = @import("../data/proto.zig");
const unit_mod = @import("unit.zig");
const bt = @import("battle.zig");
const Battle = bt.Battle;
const Fighter = bt.Fighter;
const Ref = bt.Ref;
const ab = bt.ab;
const stat = bt.stat;

/// AnimText(4): "Сопротивление".
pub const ph_resisted = 4;
pub const col_resisted: [3]u8 = .{ 250, 150, 20 };
pub const col_spell_hit: [3]u8 = .{ 250, 50, 180 };
pub const col_ammo: [3]u8 = .{ 250, 250, 150 };
pub const col_tired: [3]u8 = .{ 120, 130, 30 };
pub const col_fear: [3]u8 = .{ 0, 50, 180 };
pub const col_inspire: [3]u8 = .{ 60, 60, 250 };
pub const col_life_cost: [3]u8 = .{ 250, 50, 40 };

/// Unit kind 1: living (the only kind a sacrifice takes).
const kind_living = 1;

pub fn spellOf(b: *const Battle, i: i32) *const proto.Spell {
    return &b.pr.spells[@intCast(i)];
}

fn abs(v: i32) i32 {
    return if (v < 0) -v else v;
}

fn b2i(v: bool) i32 {
    return @intFromBool(v);
}

/// The target ignores the spell (arAntiEffect, arUnitKind) unless its
/// ability 205 names the spell.
pub fn immune(b: *const Battle, sp_i: i32, f: *const Fighter) bool {
    const sp = spellOf(b, sp_i);
    var anti: i32 = 0;
    for (sp.anti_effects) |a| {
        if (a != 0) anti += a * b.skill(f, a);
    }
    const kind = b.unitKind(f);
    for (sp.immune_kinds) |k| {
        if (k == kind) anti = 1;
    }
    return anti != 0 and b.skill(f, 205) != sp_i;
}

fn number(b: *Battle, who: Ref, v: i32, col: [3]u8) void {
    b.emit(.{ .number = .{ .who = who, .value = v, .color = col } });
}

fn resisted(b: *Battle, who: Ref) void {
    b.emit(.{ .text = .{ .who = who, .phrase = ph_resisted, .color = col_resisted } });
}

/// dcUnit::GetSpeed.
fn unitSpeed(pr: *const proto.Proto, u: *const unit_mod.Unit) i32 {
    return pr.units[@intCast(u.type)].speed + u.checkUpg(pr, stat.speed) + u.checkAbility(pr, stat.speed);
}

/// A new dcUnit(t, 0) of a polymorph.
fn newForm(b: *Battle, t: i32) ?unit_mod.Unit {
    if (t <= 0 or t >= b.pr.units.len) return null;
    return unit_mod.Unit.init(b.pr, b.r, t, 0);
}

/// BF_Unit::UnderSpell: spell `sp_i` of the unit `cr` on the unit `ref`.
pub fn underSpell(b: *Battle, ref: Ref, sp_i: i32, cr: Ref) void {
    const sp = spellOf(b, sp_i);
    const hero = b.fighter(cr);
    const csd = cr.side;
    var resist = @divTrunc((b.resist(b.fighter(ref)) - b.skill(hero, 908)) * sp.resist_mod, 100);
    if (resist < 0) resist = 0;
    const rp = b2i(sp.resist_power);
    const rd = b2i(sp.resist_duration);
    const dp = b2i(sp.defence_power);
    var destroy_on_hit = false;
    var drain = false;
    var friendly = false;
    for (sp.effects, 0..) |e, i| {
        const f = b.fighter(ref);
        var power = e.power;
        if (e.power > 0) power = e.power + @divTrunc(b.skill(hero, 905) * sp.power_mod, 100);
        if (e.power < 0) power = e.power - @divTrunc(b.skill(hero, 905) * sp.power_mod, 100);
        var dur = e.duration;
        var index = e.id;
        if (dur > 0) {
            dur += @divTrunc(b.skill(hero, 906) * sp.duration_mod, 100);
            if (index == 1) index = -1;
            if (index == 11) index = -11;
        }
        switch (index) {
            0 => {},
            1 => {
                if (sp.negative and (ref.side != csd or !friendly)) {
                    if (e.duration != -1) continue;
                    // Damage.
                    var hit = b.rollHit(power, resist * rp + b.defence(f) * dp);
                    hit = @min(hit, f.life);
                    number(b, ref, hit, col_spell_hit);
                    if (drain) {
                        const heal = @min(hit, b.maxLife(hero) - hero.life);
                        if (heal > 0) {
                            hero.life += heal;
                            number(b, cr, heal, bt.col_heal);
                        }
                    }
                    if (b.takeDamage(ref, hit, 2)) {
                        b.emit(.{ .death = ref });
                        return;
                    }
                } else {
                    const kind = b.unitKind(f);
                    if (e.duration == -1 and (kind == kind_living or ((kind == 3 or kind == 5) and sp.life_cost == 0))) {
                        // Healing.
                        const hit = @min(power, b.maxLife(f) - f.life);
                        number(b, ref, hit, bt.col_heal);
                        f.life += hit;
                    }
                }
            },
            10 => {
                if (e.duration != -1) continue;
                if (sp.negative) {
                    // The ammo burns.
                    var hit = b.rollHit(power, resist * rp);
                    hit = @min(hit, f.ammo);
                    if (hit > 0) {
                        f.ammo -= hit;
                        hit = @min(hit, f.life);
                        number(b, ref, hit, col_spell_hit);
                        if (b.takeDamage(ref, hit, 2)) {
                            b.emit(.{ .death = ref });
                            return;
                        }
                    }
                } else {
                    // More ammo.
                    if (sp.restore_only and power > b.maxAmmo(f) - f.ammo) power = @max(b.maxAmmo(f) - f.ammo, 0);
                    f.ammo += power;
                    if (power > 0) number(b, ref, power, col_ammo);
                }
            },
            11 => {
                if (e.duration != -1) continue;
                if (sp.negative) {
                    // Tiredness.
                    var hit = power - resist * rp;
                    if (hit > 0) b.useStamina(f, hit) else hit = 0;
                    number(b, ref, hit, col_tired);
                } else {
                    if (sp.restore_only and power > b.maxStamina(f) - f.stamina) power = @max(b.maxStamina(f) - f.stamina, 0);
                    f.stamina += power;
                    number(b, ref, power, col_ammo);
                }
            },
            12 => {
                if (e.duration != -1) continue;
                if (sp.negative) {
                    // Fear.
                    const hit = power - resist * rp;
                    if (hit > 0) b.changeMorale(f, -hit);
                    number(b, ref, hit, col_fear);
                } else {
                    if (sp.restore_only) power = @max(@min(power, b.maxMorale(f) - f.morale), 0);
                    b.changeMorale(f, power);
                    number(b, ref, power, col_inspire);
                }
            },
            31 => {
                // Dispel: the turns of the enemy's (or, on an ally, the
                // harmful) spells run out.
                power += b.skill(hero, 908);
                const own = csd == ref.side;
                var change: ?u8 = null;
                var k: usize = 0;
                while (k < f.n_eff) {
                    const ef = &f.effects[k];
                    var destroy = false;
                    if (ef.spell > 0) {
                        const es = spellOf(b, ef.spell);
                        const target = if (own) es.negative and ef.kind != bt.eff_charm else !es.negative or ef.kind == bt.eff_charm;
                        if (target and es.effects[0].id != 30 and ef.time > 0) {
                            ef.time -= power;
                            if (ef.time <= 0) {
                                destroy = true;
                                if (!own and ef.kind == bt.eff_charm) change = @intCast(std.math.clamp(ef.power, 0, bt.n_slots - 1));
                            }
                        }
                    }
                    if (destroy) {
                        if (ef.kind == bt.eff_polymorph) b.restoreForm(f);
                        f.removeEffect(k);
                    } else k += 1;
                }
                if (!own) {
                    // The summons take damage.
                    const p2 = power + e.power * 2;
                    if (f.summoned and p2 > resist) {
                        const hit = @min(b.rollHit(p2, resist), f.life);
                        number(b, ref, hit, col_spell_hit);
                        if (b.takeDamage(ref, hit, 2)) {
                            b.emit(.{ .death = ref });
                            return;
                        }
                    }
                    if (change) |n| {
                        b.changeSide(ref, n);
                        return;
                    }
                }
            },
            47 => {
                // The web.
                if (resist > 3 or b.skill(f, ab.web_immunity) > 0) {
                    resisted(b, ref);
                    continue;
                }
                const d: i32 = if (dur > b.attack(f)) 3 else 2;
                var curse = true;
                for (f.effects[0..f.n_eff]) |*ef| {
                    if (ef.spell == sp_i or ef.ability == bt.upg_icon.web) {
                        ef.time += 1;
                        curse = false;
                    }
                }
                if (curse) {
                    f.addEffect(.{ .kind = ab.immobile, .power = 1, .time = d, .visible = true, .spell = sp_i });
                    f.addEffect(.{ .kind = stat.defence, .power = 1, .time = d, .spell = sp_i });
                    f.addEffect(.{ .kind = stat.ranged_defence, .power = 1, .time = d, .spell = sp_i });
                    f.addEffect(.{ .kind = ab.no_combat, .power = 1, .time = d, .spell = sp_i });
                    f.speed = 0;
                    f.can_act = false;
                }
            },
            70 => destroy_on_hit = true,
            71 => drain = true,
            72 => friendly = true,
            bt.eff_charm => {
                // Mind control.
                var d: i32 = 1;
                const ok = if (e.duration == -1) power >= resist else blk: {
                    d = dur - resist * rd;
                    break :blk d > 0;
                };
                if (!ok) {
                    resisted(b, ref);
                    continue;
                }
                charm(b, ref, csd, sp_i, d);
                return;
            },
            75 => {
                // One more move.
                if (b.skill(f, ab.immobile) != 0) continue;
                f.can_act = true;
                if (b.skill(f, ab.rooted) == 0) {
                    f.speed = b.maxSpeed(f);
                    f.march = true;
                }
                if (f.stamina < 1) {
                    f.stamina = 1;
                    f.resting = false;
                }
                if (f.morale <= 0) {
                    if (f.fear < 60 and (if (b.simulate) f.fear < 30 else bt.rnd(b.r, 100) > f.fear)) {
                        b.changeMorale(f, 1);
                        f.fear = 0;
                    } else {
                        f.fear -= 10;
                        b.panic(ref);
                    }
                }
            },
            92 => {
                // The unit strikes itself.
                if (resist * rp > power) {
                    resisted(b, ref);
                    continue;
                }
                const hit = @min(b.meleeHit(f, f, true) * 2, f.life);
                b.hitEffect(f, f, hit, null);
                number(b, ref, hit, col_spell_hit);
                b.useStamina(f, 1);
                if (b.takeDamage(ref, hit, 2)) {
                    b.emit(.{ .death = ref });
                    return;
                }
            },
            93 => {
                // The poison goes.
                var k: usize = 0;
                while (k < f.n_eff) {
                    if (f.effects[k].poison) f.removeEffect(k) else k += 1;
                }
            },
            else => enchant(b, ref, sp_i, e, i, power, dur, resist, csd, destroy_on_hit),
        }
    }
}

/// The default branch of UnderSpell: a timed effect (or a polymorph).
fn enchant(b: *Battle, ref: Ref, sp_i: i32, e: proto.SpellEffect, i: usize, power: i32, dur_in: i32, resist: i32, csd: u1, destroy_on_hit: bool) void {
    const sp = spellOf(b, sp_i);
    const f = b.fighter(ref);
    var dur = dur_in;
    if (sp.negative and csd != ref.side) dur -= resist * b2i(sp.resist_duration);
    if (dur <= 0 or power == 0) return resisted(b, ref);
    const visible = i == 0 or (destroy_on_hit and i == 1);
    if (sp.cumulative) {
        if (e.id == stat.speed) {
            if (power > 0) f.speed += power;
            if (power < 0 and f.speed > 0) f.speed = @max(f.speed + power, 1);
        }
        f.addEffect(.{ .kind = e.id, .power = power, .time = dur, .spell = sp_i, .destroy_on_hit = destroy_on_hit, .visible = i == 0 });
        if (e.id == ab.immobile) f.speed = 0;
        if (e.id == ab.no_combat) f.can_act = false;
        return;
    }
    var found = false;
    for (f.effects[0..f.n_eff]) |*ef| {
        if (ef.spell == sp_i and ef.kind == e.id) {
            found = true;
            if (dur > ef.time or abs(power) > abs(ef.power)) {
                ef.time = dur;
                ef.power = power;
            }
        }
        if (e.id > 1000 and ef.kind == bt.eff_polymorph) {
            found = true;
            if (dur > ef.time or abs(power) > abs(ef.power)) {
                ef.time = dur;
                const nu = newForm(b, e.id - 1000) orelse continue;
                f.life = @max(@divTrunc(nu.maxLife(b.pr) * f.life, @max(f.unit.maxLife(b.pr), 1)), 1);
                f.speed = @divTrunc(unitSpeed(b.pr, &nu) * f.speed, @max(unitSpeed(b.pr, &f.unit), 1));
                f.unit = nu;
            }
        }
    }
    if (found) return;
    f.addEffect(.{ .kind = e.id, .power = power, .time = dur, .spell = sp_i, .destroy_on_hit = destroy_on_hit, .visible = visible });
    if (e.id == stat.speed) {
        if (power > 0 and f.can_act and b.skill(f, ab.rooted) == 0 and b.skill(f, ab.immobile) == 0) f.speed = @min(f.speed + power, b.maxSpeed(f));
        if (power < 0 and f.speed > 0) f.speed = @max(f.speed + power, 1);
    }
    if (e.id == ab.immobile) f.speed = 0;
    if (e.id == ab.no_combat) f.can_act = false;
    if (e.id > 1000) {
        // Polymorph.
        f.effects[f.n_eff - 1].kind = bt.eff_polymorph;
        const nu = newForm(b, e.id - 1000) orelse return;
        f.life = @max(@divTrunc(nu.maxLife(b.pr) * f.life, @max(f.unit.maxLife(b.pr), 1)), 1);
        if (f.true_form == null) f.true_form = f.unit;
        f.unit = nu;
        f.ammo = b.typeStat(f, stat.ammo);
        f.life = @min(f.life, b.maxLife(f));
    }
}

/// Mind control: the unit moves to the first free slot after the squad
/// of side `csd` with effect 73 (its old slot, `time` turns).
fn charm(b: *Battle, ref: Ref, csd: u1, sp_i: i32, time: i32) void {
    const f = b.fighter(ref);
    var k: usize = 0;
    while (k < f.n_eff) {
        if (f.effects[k].kind == bt.eff_charm) f.removeEffect(k) else k += 1;
    }
    var num: usize = bt.max_squad;
    while (num < bt.n_slots and b.units[csd][num].present()) num += 1;
    if (num == bt.n_slots or csd == ref.side) return;
    if (b.skill(f, ab.immobile) == 0) {
        f.can_act = true;
        f.speed = b.maxSpeed(f);
        if (f.stamina < 1) f.stamina = 1;
        f.march = true;
        if (f.morale <= 0) {
            b.changeMorale(f, 1);
            f.fear = 0;
        }
    }
    const old = ref.slot;
    b.changeSide(ref, @intCast(num));
    b.units[csd][num].addEffect(.{ .kind = bt.eff_charm, .power = old, .time = time, .spell = sp_i, .visible = true });
}

/// CorpseSpell: a summon from the corpse, or the corpse raised.
pub fn corpseSpell(b: *Battle, sp_i: i32, corpse: Ref, cr: Ref) void {
    const sp = spellOf(b, sp_i);
    const a = b.fighter(cr);
    const e0 = sp.effects[0];
    if (e0.id > 1000) {
        if (b.freeSummonSlot(cr.side)) |slot| {
            const power = e0.power + @divTrunc(b.skill(a, 907), 2) + b.skill(a, 909);
            const c = b.fighter(corpse);
            const to: Ref = .{ .side = cr.side, .slot = slot };
            b.summon(to, @mod(e0.id, 1000), power, c.x, c.y);
            b.gomunculus = to;
            c.in_battle = false;
        }
    }
    if (e0.id == 1) {
        b.gomunculus = corpse;
        const power = @min(e0.power + @divTrunc(b.skill(a, 905) * sp.power_mod, 10), 100);
        const c = b.fighter(corpse);
        c.life = @max(@divTrunc(b.maxLife(c) * power, 100), 1);
        b.setPass(c, false);
        c.fear = 0;
        c.speed = b.typeStat(c, stat.speed);
        c.can_act = true;
        c.resting = false;
        c.in_battle = true;
        c.n_eff = 0;
        c.march = b.skill(c, ab.march) != 0;
        b.emit(.{ .appear = .{ .who = corpse } });
    }
}

/// TileSpell: a summon on a free cell (the teleport of a human's spell
/// is not ported).
pub fn tileSpell(b: *Battle, sp_i: i32, x: i32, y: i32, cr: Ref) void {
    const e0 = spellOf(b, sp_i).effects[0];
    if (e0.id <= 1000) return;
    const slot = b.freeSummonSlot(cr.side) orelse return;
    const power = e0.power + b.skill(b.fighter(cr), 907);
    const to: Ref = .{ .side = cr.side, .slot = slot };
    b.summon(to, @mod(e0.id, 1000), power, x, y);
    b.gomunculus = to;
}

/// BF_Unit::Replace: the unit stands on (x, y) at once.
pub fn replace(b: *Battle, ref: Ref, x: i32, y: i32) void {
    const f = b.fighter(ref);
    b.setPass(f, true);
    f.x = x;
    f.y = y;
    b.setPass(f, false);
    b.emit(.{ .appear = .{ .who = ref } });
}

/// The price of a spell for a hero (CheckHeroBonus 983 lowers it).
pub fn price(b: *const Battle, f: *const Fighter, sp_i: i32) i32 {
    return @divTrunc(spellOf(b, sp_i).cost * (100 - b.heroBonus(f, 983)), 100);
}

/// SpellCastAI: the caster pays (crystals, the book slot or the ammo, the
/// stamina and the life); `victim` dies for a sacrifice.
pub fn castAI(b: *Battle, cr: Ref, sp_i: i32, victim: ?u8) void {
    const sp = spellOf(b, sp_i);
    const a = b.fighter(cr);
    const s = cr.side;
    if (a.hero_unit) {
        const p = price(b, a, sp_i);
        b.gem[s] -= p;
        b.gem_spent[s] += p;
        b.karma[s] += sp.karma;
        for (&b.book[s]) |*slot| {
            if (slot[0] == sp_i and slot[1] > 0) {
                slot[1] = 0;
                break;
            }
        }
    } else a.ammo -= b.skill(a, 2000 + sp_i);
    if (a.life > 0) {
        b.useStamina(a, sp.stamina_cost);
        if (!(b.archmage and a.hero_unit and a.stamina > 0)) {
            a.can_act = false;
            a.speed = 0;
        }
        if (sp.life_cost > 0) {
            const hit = @min(sp.life_cost, a.life);
            if (b.takeDamage(cr, hit, 3)) b.emit(.{ .death = cr });
            number(b, cr, hit, col_life_cost);
        }
    }
    if (b.archmage and a.hero_unit) b.archmage = false;
    if (sp.sacrifice) if (victim) |v| {
        const vr: Ref = .{ .side = s, .slot = v };
        const vf = b.fighter(vr);
        if (vf.life > 0) {
            vf.life = 0;
            b.emit(.{ .death = vr });
            b.death(vr);
        }
    };
}

// ---------------------------------------------------------------------
// The spells of a human player (Spellbook, SpellPrepare, the clicks of
// the main loop and SpellCast, Battlefield.cpp)
// ---------------------------------------------------------------------

/// A cell of Spellbook(): the spell and whether it can be cast now.
pub const Entry = struct {
    spell: i32 = 0,
    castable: bool = false,
    /// Spells[AA][1][i] == 0: cast in this battle already.
    used: bool = false,
};

/// The spell the player picked (ActiveSpell, pCast).
pub const Pending = struct {
    spell: i32,
    /// The Spells[AA] slot of a hero's spell; null for a caster's.
    slot: ?usize = null,
    /// bSacrifice: the victim is picked first.
    sacrifice: bool = false,
    victim: ?u8 = null,
    /// pTeleportUnit: the unit a teleport moves; its cell comes next.
    teleport: ?Ref = null,
};

pub const Click = enum { none, cast, advanced };

fn fighterC(b: *const Battle, r: Ref) *const Fighter {
    return &b.units[r.side][r.slot];
}

/// The spell button is shown: a hero with a class or a caster.
pub fn hasBook(b: *const Battle, r: Ref) bool {
    const a = fighterC(b, r);
    return a.hero_unit or b.isCaster(a);
}

/// The spell button is active.
pub fn canOpenBook(b: *const Battle, r: Ref) bool {
    const a = fighterC(b, r);
    return hasBook(b, r) and a.can_act and (a.speed > 0 or b.skill(a, ab.rooted) != 0) and a.stamina > 0;
}

/// The cells of Spellbook(): a hero's book in its slot order, a caster's
/// spells of abilities 2000 + i packed.
pub fn book(b: *const Battle, r: Ref) [bt.spellbook_size]Entry {
    var out: [bt.spellbook_size]Entry = @splat(.{});
    const a = fighterC(b, r);
    const s = r.side;
    if (a.hero_unit) {
        for (b.book[s], 0..) |slot, i| {
            if (slot[0] <= 0) continue;
            const sp = spellOf(b, slot[0]);
            out[i] = .{ .spell = slot[0], .used = slot[1] == 0, .castable = slot[1] > 0 and (sp.cost <= 0 or b.gem[s] >= price(b, a, slot[0])) };
        }
    } else {
        var n: usize = 0;
        for (1..b.pr.spells.len) |i| {
            const k = b.skill(a, 2000 + @as(i32, @intCast(i)));
            if (k == 0 or n == out.len) continue;
            out[n] = .{ .spell = @intCast(i), .castable = a.ammo >= k };
            n += 1;
        }
    }
    return out;
}

/// The pending spell of book cell `i`.
pub fn pick(b: *const Battle, r: Ref, i: usize) ?Pending {
    const e = book(b, r)[i];
    if (!e.castable) return null;
    const sp = spellOf(b, e.spell);
    return .{ .spell = e.spell, .slot = if (fighterC(b, r).hero_unit) i else null, .sacrifice = sp.sacrifice };
}

fn castOnSide(b: *Battle, sp_i: i32, ts: u1, cr: Ref, cell: ?[2]i32) void {
    for (0..bt.n_slots) |k| {
        const t = &b.units[ts][k];
        if (!t.present() or t.life <= 0) continue;
        if (cell) |c| if (t.x != c[0] or t.y != c[1]) continue;
        if (immune(b, sp_i, t)) continue;
        underSpell(b, .{ .side = ts, .slot = @intCast(k) }, sp_i, cr);
    }
}

fn begin(b: *Battle, cr: Ref, sp_i: i32) void {
    b.emit(.{ .cast = .{ .who = cr, .spell = sp_i, .name = false } });
}

/// SpellPrepare: the range map, and a spell without a target is cast at
/// once (true).  Call it again after the sacrifice's victim is picked.
pub fn prepare(b: *Battle, cr: Ref, p: *Pending) bool {
    if (p.sacrifice) {
        for (&b.map) |*col| for (col) |*m| {
            m.shoot = 1;
        };
        return false;
    }
    const sp = spellOf(b, p.spell);
    const a = b.fighter(cr);
    if (sp.target == 5 or sp.target == 3) {
        b.clearMap();
        b.shootMap(a.x, a.y, -1, @divTrunc(b.skill(a, 907), 2) + 1);
    } else for (&b.map) |*col| for (col) |*m| {
        m.shoot = 1;
    };
    if (sp.target != 0) return false;
    begin(b, cr, p.spell);
    if (sp.area == 2) {
        if (sp.on_ally) castOnSide(b, p.spell, cr.side, cr, null);
        if (sp.on_enemy) castOnSide(b, p.spell, 1 - cr.side, cr, null);
    }
    if (sp.area == 0) underSpell(b, cr, p.spell, cr);
    cast(b, cr, p);
    return true;
}

/// The cell takes the summon of target 5 (and the teleported unit).
pub fn freeCell(b: *Battle, x: i32, y: i32) bool {
    return b.mc(x, y).shoot > 0 and b.grid(x, y).pass and b.bfo(b.grid(x, y).land).move_cost > 0;
}

/// A corpse of side `s` on (x, y) the spell may take.
fn corpseAt(b: *const Battle, s: u1, x: i32, y: i32, target: i32) ?Ref {
    for (&b.units[s], 0..) |*c, k| {
        if (!c.present() or c.life != 0 or !c.in_battle or c.x != x or c.y != y) continue;
        if (target == 2 or b.grid(x, y).pass) return .{ .side = s, .slot = @intCast(k) };
    }
    return null;
}

/// A click on cell (x, y) with the pending spell; `dry` only tells
/// whether the click would do something (the magic cursor).
pub fn click(b: *Battle, cr: Ref, p: *Pending, x: i32, y: i32, dry: bool) Click {
    const s = cr.side;
    const es: u1 = 1 - s;
    if (p.sacrifice) {
        // The victim: a living unit of the side (not undead, demon or construct).
        const k = b.unitAt(s, x, y) orelse return .none;
        const t = b.fighter(k);
        const kind = b.unitKind(t);
        if (kind >= 2 and kind <= 4 and b.skill(t, 205) != p.spell) return .none;
        if (dry) return .advanced;
        p.sacrifice = false;
        p.victim = k.slot;
        return if (prepare(b, cr, p)) .cast else .advanced;
    }
    const sp = spellOf(b, p.spell);
    if (p.teleport) |tr| {
        // Teleport.iTarget = 5 on the map SpellPrepare left.
        if (!freeCell(b, x, y)) return .none;
        if (dry) return .cast;
        begin(b, cr, p.spell);
        replace(b, tr, x, y);
        cast(b, cr, p);
        return .cast;
    }
    switch (sp.target) {
        3 => {
            if (dry) return .cast;
            begin(b, cr, p.spell);
            b.clearMap();
            b.shootMap(x, y, -1, sp.radius);
            if (sp.on_ally or sp.on_enemy) for (0..bt.size) |i| for (0..bt.size) |j| {
                const xx: i32 = @intCast(i);
                const yy: i32 = @intCast(j);
                if (b.mc(xx, yy).shoot == 0 and !(xx == x and yy == y)) continue;
                if (sp.on_ally) castOnSide(b, p.spell, s, cr, .{ xx, yy });
                if (sp.on_enemy) castOnSide(b, p.spell, es, cr, .{ xx, yy });
            };
            cast(b, cr, p);
            return .cast;
        },
        5 => {
            if (!freeCell(b, x, y)) return .none;
            if (dry) return .cast;
            begin(b, cr, p.spell);
            tileSpell(b, p.spell, x, y, cr);
            cast(b, cr, p);
            return .cast;
        },
        2, 4 => {
            for ([2]u1{ s, es }) |cs| {
                if (!(if (cs == s) sp.on_ally else sp.on_enemy)) continue;
                const corpse = corpseAt(b, cs, x, y, sp.target) orelse continue;
                if (dry) return .cast;
                begin(b, cr, p.spell);
                corpseSpell(b, p.spell, corpse, cr);
                cast(b, cr, p);
                return .cast;
            }
            return .none;
        },
        1 => {
            for ([2]u1{ s, es }) |ts| {
                if (!(if (ts == s) sp.on_ally else sp.on_enemy)) continue;
                const tr = b.unitAt(ts, x, y) orelse continue;
                if (immune(b, p.spell, b.fighter(tr))) continue;
                if (dry) return if (sp.effects[0].id == 89) .advanced else .cast;
                if (sp.effects[0].id == 89) {
                    if (ts == s or b.skill(b.fighter(cr), 908) >= b.resist(b.fighter(tr))) {
                        p.teleport = tr;
                        return .advanced;
                    }
                    resisted(b, tr);
                    begin(b, cr, p.spell);
                    cast(b, cr, p);
                    return .cast;
                }
                begin(b, cr, p.spell);
                underSpell(b, tr, p.spell, cr);
                cast(b, cr, p);
                return .cast;
            }
            return .none;
        },
        else => return .none,
    }
}

/// SpellCast: the caster pays (crystals and the book slot, or the ammo),
/// spends the stamina (one more after moving), gets iSpellCast; the
/// archmage keeps half its move points; the sacrifice's victim dies.
pub fn cast(b: *Battle, cr: Ref, p: *const Pending) void {
    const sp = spellOf(b, p.spell);
    const a = b.fighter(cr);
    const s = cr.side;
    if (a.hero_unit) {
        const pz = price(b, a, p.spell);
        b.gem[s] -= pz;
        b.gem_spent[s] += pz;
        b.karma[s] += sp.karma;
        if (p.slot) |k| b.book[s][k][1] = 0;
    } else a.ammo -= b.skill(a, 2000 + p.spell);
    if (a.life > 0) {
        b.useStamina(a, sp.stamina_cost);
        a.st.spells += sp.stamina_cost;
        if (a.speed < b.maxSpeed(a)) b.useStamina(a, 1);
        if (b.archmage and a.hero_unit and a.stamina > 0 and a.speed > 1) {
            a.speed = @divTrunc(a.speed, 2);
        } else {
            a.can_act = false;
            a.speed = 0;
        }
        if (sp.life_cost > 0) {
            const hit = @min(sp.life_cost, a.life);
            if (b.takeDamage(cr, hit, 3)) b.emit(.{ .death = cr });
            number(b, cr, hit, col_life_cost);
        }
    }
    if (b.archmage and a.hero_unit) b.archmage = false;
    if (sp.sacrifice) if (p.victim) |v| {
        const vr: Ref = .{ .side = s, .slot = v };
        const vf = b.fighter(vr);
        if (vf.life > 0) {
            vf.life = 0;
            b.emit(.{ .death = vr });
            b.death(vr);
        }
    };
}
