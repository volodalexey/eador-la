//! BattleAI.cpp: the orders of the computer side of a battle.
//!
//! A side's turn (the AI branch of Battlefield.cpp's EndTurn): AI_CalcPower,
//! then AI_Main(false) up to (MAX_SQUAD + MAX_SUMMONS) * 2 times (heals,
//! shots and blows only), then AI_Main(true) up to four times as often
//! until a unit twice in a row finds nothing to do (IndexAI -100).
//!
//! AI_Main takes the next unit that may act and rates every heal, shot and
//! blow it can make this turn.  The ratings roll GetMeleeHit / GetRangedHit
//! like the original, so the rand() stream advances the same way.  Without
//! a target the unit gathers ammo, meditates, rests, eats a corpse or walks
//! towards the nearest target over the turn-weighted AI map (StepAIMap,
//! FlyAIMap, StepAIPath, FlyAIPath).
//!
//! The magic part comes first: every spell of the hero's book (or every
//! spell ability 2000 + i of a caster unit) is rated on each target with
//! AI_GetEnemySpellTaste / AI_GetAllySpellTaste, the summons on the danger
//! zone of AI_CalcDangerZone, minus AI_CalcSpellPenalty; the spell wins
//! when its hit is not below the heal, the shot and the blow.  The tastes
//! roll GetHit like the original.
//!
//! AI_Simulate, AI_SimulatePower and AI_SimulateDangerZone of the
//! simulated battle (battle_sim.zig) are at the end.
const std = @import("std");
const bt = @import("battle.zig");
const bs = @import("battle_spell.zig");
const Battle = bt.Battle;
const Fighter = bt.Fighter;
const Ref = bt.Ref;
const ab = bt.ab;
const size = bt.size;
const n_units = bt.n_slots;

/// MAX_SQUAD + MAX_SUMMONS (SPELLBOOK_SIZE): the turn's loops count them.
const squad_total = 16 + 21;

/// unit_upg.var abilities only the AI looks at.
const sk = struct {
    const double_shot = 20;
    const meditation = 22;
    const gather = 23;
    const stoning = 46;
    const web = 47;
    const corpse_eater = 49;
    const soul_steal = 57;
    const crush = 59;
    const trample = 63;
    const round_attack = 65;
    const round_button = 66;
    /// Lowers the interest in a target (an effect: divides by its turns).
    const shunned = 70;
    /// Mind control: a controlled enemy is spared (GetEffectDuration 1..2).
    const charmed = 73;
    const web_immune = 201;
    const stone_immune = 202;
};

const Path = struct {
    stack: [256][2]i8 = undefined,
    depth: usize = 0,
    best: [256][2]i8 = undefined,
    n: usize = 0,
    found: bool = false,
    min_weight: i32 = 0,

    fn record(p: *Path, weight: i32) void {
        @memcpy(p.best[0..p.depth], p.stack[0..p.depth]);
        p.n = p.depth;
        p.found = true;
        p.min_weight = weight;
    }
};

pub const Phase = enum { start, strike, move };

pub const State = struct {
    phase: Phase = .start,
    calls: usize = 0,
    index_ai: i32 = -1,
    /// arUnitPower: 0 heal, 1 shot, 2 melee, 3..6 under a magic shot, a
    /// shot, a magic blow, a blow.
    power: [2][n_units][7]i32 = @splat(@splat(@splat(0))),
    try_fly: [size][size]bool = @splat(@splat(true)),
    path: Path = .{},
    /// DangerZone: the steps from the nearest enemy (100 out of reach).
    danger: [size][size]i32 = @splat(@splat(100)),
    /// iAISacVictim: the unit the last sacrifice rating picked.
    sac_victim: ?u8 = null,
};

/// One order of the computer side; false when its turn is over.
pub fn step(b: *Battle) bool {
    const st = &b.ai;
    while (!b.over()) {
        switch (st.phase) {
            .start => {
                calcPower(b);
                calcDangerZone(b);
                b.active = null;
                st.phase = .strike;
                st.calls = 0;
            },
            .strike, .move => {
                const can_move = st.phase == .move;
                const limit: usize = if (can_move) squad_total * 4 else squad_total * 2;
                if (st.calls >= limit or (can_move and st.index_ai == -100)) {
                    if (can_move) return false;
                    b.active = null;
                    st.index_ai = -1;
                    st.phase = .move;
                    st.calls = 0;
                    continue;
                }
                st.calls += 1;
                const before = b.anims.items.len;
                aiMain(b, can_move);
                if (b.anims.items.len > before) return true;
            },
        }
    }
    return false;
}

// -- helpers ------------------------------------------------------------

fn abs(v: i32) i32 {
    return if (v < 0) -v else v;
}

/// The six cells around (x, y) in the order of FindNeighbour and the
/// melee search.
fn meleeCells(x: i32, y: i32) [6][2]i32 {
    const p = @rem(y, 2);
    return .{ .{ x - 1 + p, y - 1 }, .{ x + p, y - 1 }, .{ x + 1, y }, .{ x + p, y + 1 }, .{ x - 1 + p, y + 1 }, .{ x - 1, y } };
}

fn isNeighbour(f: *const Fighter, x: i32, y: i32) bool {
    for (meleeCells(f.x, f.y)) |c| {
        if (c[0] == x and c[1] == y) return true;
    }
    return false;
}

const Neighbours = struct { refs: [6]Ref = undefined, n: usize = 0 };

/// FindNeighbour(s) of a unit standing on (x, y).
fn neighbourList(b: *const Battle, x: i32, y: i32, s: u1) Neighbours {
    var r: Neighbours = .{};
    for (&b.units[s], 0..) |*o, k| {
        if (o.life == 0) continue;
        for (meleeCells(x, y)) |c| {
            if (o.x == c[0] and o.y == c[1] and r.n < 6) {
                r.refs[r.n] = .{ .side = s, .slot = @intCast(k) };
                r.n += 1;
            }
        }
    }
    return r;
}

/// ipClass1 of the hero whose own unit `f` is (pUnit->pHero), else 0.
fn heroClass(b: *const Battle, f: *const Fighter) i32 {
    if (!f.hero_unit) return 0;
    const h = b.armies[f.home].hero orelse return 0;
    return h.class1;
}

fn bloodless(b: *const Battle, f: *const Fighter) bool {
    const k = b.unitKind(f);
    return k == 2 or k == 4;
}

/// GetEffectDuration: the newest effect of the kind (the original list
/// grows at the head).
fn effectDuration(f: *const Fighter, index: i32) i32 {
    var i = f.n_eff;
    while (i > 0) {
        i -= 1;
        if (f.effects[i].kind == index) return f.effects[i].time;
    }
    return 0;
}

fn roundButton(b: *const Battle, f: *const Fighter) bool {
    const r = b.skill(f, sk.round_button);
    return r > 0 and f.stamina > r;
}

fn clearShootLand(b: *Battle) void {
    for (&b.map) |*col| for (col) |*m| {
        m.shoot = 0;
        m.land = 0;
    };
}

fn resetTryFly(b: *Battle) void {
    b.ai.try_fly = @splat(@splat(true));
}

fn tryFly(b: *Battle, x: i32, y: i32) *bool {
    return &b.ai.try_fly[@intCast(x)][@intCast(y)];
}

/// The shooting range from (x, y): hills add one, leaving them takes one.
fn rangeFrom(b: *const Battle, f: *const Fighter, x: i32, y: i32) i32 {
    var r = b.shootingRange(f);
    if (r > 1 and b.skill(f, ab.flying) <= 0) {
        const here = b.grid(f.x, f.y).land;
        const there = b.grid(x, y).land;
        if (here == 3 and there != 3) r -= 1;
        if (here != 3 and there == 3) r += 1;
    }
    return r;
}

/// MoveMap of the target searches: 2 for a cell reached with `less` move
/// points, 1 for one reached at `lim` with a march.
fn moveMap(b: *Battle, f: *const Fighter, lim: i32, less: i32) [size][size]i32 {
    var mm: [size][size]i32 = @splat(@splat(0));
    for (0..size) |i| for (0..size) |j| {
        const x: i32 = @intCast(i);
        const y: i32 = @intCast(j);
        const m = b.mc(x, y);
        if (m.land > 0 and m.land <= lim and m.pass and b.grid(x, y).pass) {
            if (m.land == lim and f.march and mm[i][j] < 1) mm[i][j] = 1;
            if (m.land < less) mm[i][j] = 2;
        }
    };
    return mm;
}

/// A wounded living ally a healer reaches on the current shot map.
fn healable(b: *Battle, o: *const Fighter) bool {
    return o.life > 0 and b.mc(o.x, o.y).shoot != 0 and o.life < b.maxLife(o) and !bloodless(b, o);
}

fn shootable(b: *Battle, e: *const Fighter) bool {
    return e.life > 0 and b.mc(e.x, e.y).shoot > 1;
}

fn findCorpse(b: *const Battle, s: u1, x: i32, y: i32) ?Ref {
    for (&b.units[s], 0..) |*o, k| {
        if (o.unit.type != 0 and o.life == 0 and o.in_battle and o.x == x and o.y == y) return .{ .side = s, .slot = @intCast(k) };
    }
    return null;
}

fn stuck(b: *Battle, num: i32) void {
    const st = &b.ai;
    if (st.index_ai == num) st.index_ai = -100;
    if (st.index_ai == -1) st.index_ai = num;
}

// -- the hits the AI expects --------------------------------------------

/// AI_CalcMeleeHit: -100 when the target's first strike would kill.
fn aiMeleeHit(b: *Battle, a: *const Fighter, d: *const Fighter) i32 {
    if (b.skill(d, ab.first_strike) != 0 and !d.resting and b.skill(d, ab.immobile) == 0 and d.stamina > 0 and
        b.skill(a, ab.agility) == 0 and b.skill(a, ab.first_strike) == 0 and b.meleeHit(d, a, false) >= a.life) return -100;
    return b.meleeHit(a, d, true);
}

/// CalcMeleeHit: the hit without the roll.
fn calcMeleeHit(b: *const Battle, a: *const Fighter, d: *const Fighter, attacking: bool) i32 {
    var att = if (attacking) b.attack(a) else b.counterAttack(a);
    var def = b.defence(d);
    const crush = b.skill(a, sk.crush);
    if (attacking and crush > 0 and a.stamina > crush) att += @divTrunc(att, 2);
    const karma = b.utype(d).karma;
    if (b.skill(a, ab.smite_evil) > 0 and karma < 0) att += b.skill(a, ab.smite_evil) * -karma;
    if (b.skill(a, ab.smite_good) > 0 and karma > 0) att += b.skill(a, ab.smite_good) * karma;
    if (b.skill(a, ab.magic_strike) != 0) {
        def = b.resist(d);
    } else {
        if (b.skill(a, ab.armor_piercing) != 0) def = @divTrunc(def, 2);
        if (attacking and d.parry and d.stamina > 0 and !d.resting) def += b.skill(d, ab.parry);
        if (attacking) def -= b.skill(a, ab.precise_strike);
    }
    return @max(att - @max(def, 0), 0);
}

/// CalcRangedHit.
fn calcRangedHit(b: *const Battle, a: *const Fighter, d: *const Fighter) i32 {
    if (b.skill(a, ab.magic_shot) != 0) {
        return b.rangedAttack(a) - @max(b.resist(d) - b.skill(a, ab.energy_control), 0);
    }
    const rd = if (b.skill(a, ab.armor_piercing_shot) != 0) @divTrunc(b.rangedDefence(d), 2) else b.rangedDefence(d);
    var r = b.rangedAttack(a) - @max(rd - b.skill(a, ab.precise_shot), 0);
    const fire = b.skill(a, ab.fire_arrows);
    if (fire > b.resist(d)) r += fire - b.resist(d);
    return r;
}

// -- the maps ---------------------------------------------------------------

/// AI_SelectUnit: the shot map and this turn's move map (a march adds a
/// move point when `can_run`); the unit becomes the active one.
fn aiSelect(b: *Battle, ref: Ref, can_run: bool) void {
    const f = b.fighter(ref);
    b.clearMap();
    if (b.shootingRange(f) != 0 and f.ammo != 0) {
        b.shootMap(f.x, f.y, -1, b.shootingRange(f));
        for (&b.map) |*col| for (col) |*m| {
            m.land = 0;
        };
    }
    const ms = b.maxSpeed(f);
    const march = b.skill(f, ab.march);
    b.can_pass = .{ 0, 0, 0 };
    b.first_move = f.speed >= ms;
    if (!f.march and march > 0 and f.speed <= ms) b.first_move = false;
    setCanPass(b, f);
    if (march == 0) f.march = false;
    if (f.speed > 0 and f.stamina > 0) {
        var speed = f.speed;
        var stam = f.stamina - b.fatigue(f);
        if (f.march and march < stam and can_run) {
            speed += 1;
            stam -= march;
        }
        if (b.skill(f, ab.flying) != 0) {
            b.flyMap(f.x, f.y, -1, speed, true);
        } else if (b.skill(f, ab.low_flying) != 0) {
            b.flyMap(f.x, f.y, -1, speed, false);
        } else b.stepMap(f.x, f.y, -1, speed, 0, stam);
    }
    b.active = ref;
}

fn setCanPass(b: *Battle, f: *const Fighter) void {
    for (0..3) |i| {
        const k: i32 = @intCast(i);
        if (b.skill(f, ab.knowledge_first + k) != 0) b.can_pass[i] = 2 + k;
    }
}

/// The turns map of the walk: Field.Land is the turn a cell is reached on.
fn aiMap(b: *Battle, f: *const Fighter) void {
    const ms = b.maxSpeed(f);
    b.can_pass = .{ 0, 0, 0 };
    b.first_move = f.speed >= ms;
    if (!f.march and f.speed <= ms) b.first_move = false;
    setCanPass(b, f);
    if (b.skill(f, ab.flying) != 0) {
        flyAIMap(b, f.x, f.y, -1, ms, f.speed, true);
    } else if (b.skill(f, ab.low_flying) != 0) {
        flyAIMap(b, f.x, f.y, -1, ms, f.speed, false);
    } else stepAIMap(b, f.x, f.y, -1, ms, f.speed);
}

fn tryAIMap(b: *Battle, x: i32, y: i32, weight: i32) bool {
    if (!bt.inside(x, y) or !b.grid(x, y).pass) return false;
    const m = b.mc(x, y);
    return (m.land == 0 or weight <= m.land) and b.bfo(b.grid(x, y).land).move_cost != 0;
}

/// StepAIMap.
fn stepAIMap(b: *Battle, x: i32, y: i32, weight_in: i32, speed: i32, left_in: i32) void {
    var weight = weight_in;
    var left = left_in;
    if (weight < 0) {
        b.mc(x, y).land = weight;
        weight = 1;
    } else {
        const cost = b.moveCost(x, y);
        if (left < cost and speed > left) {
            weight += 1;
            left = speed;
        }
        const m = b.mc(x, y);
        if (weight > m.land and m.land != 0) return;
        left -= cost;
        m.land = weight;
        m.pass = true;
        if (left <= 0) {
            weight += 1;
            left = speed;
        }
    }
    for (bt.neighbours(x, y)) |n| {
        if (tryAIMap(b, n[0], n[1], weight)) stepAIMap(b, n[0], n[1], weight, speed, left);
    }
}

fn tryAIPath(b: *Battle, x: i32, y: i32, weight: i32) bool {
    if (!bt.inside(x, y)) return false;
    const m = b.mc(x, y);
    return b.grid(x, y).pass and m.pass and weight <= m.land;
}

/// StepAIPath: the way of the fewest turns to (x1, y1).
fn stepAIPath(b: *Battle, x: i32, y: i32, weight_in: i32, x1: i32, y1: i32, speed: i32, left_in: i32) void {
    const p = &b.ai.path;
    var weight = weight_in;
    var left = left_in;
    if (weight < 0) {
        weight = 1;
    } else {
        const cost = b.moveCost(x, y);
        if (left < cost and speed > left) {
            weight += 1;
            left = speed;
        }
        left -= cost;
    }
    if (p.depth == p.stack.len) return;
    p.stack[p.depth] = .{ @intCast(x), @intCast(y) };
    p.depth += 1;
    defer p.depth -= 1;
    if (x == x1 and y == y1) {
        if ((p.found and weight < p.min_weight) or (!p.found and weight == p.min_weight)) p.record(weight);
        return;
    }
    if (left <= 0) {
        weight += 1;
        left = speed;
        if (weight > p.min_weight) return;
    }
    for (bt.neighbours(x, y)) |n| {
        if (tryAIPath(b, n[0], n[1], weight)) stepAIPath(b, n[0], n[1], weight, x1, y1, speed, left);
    }
}

/// FlyAIXYmap: a flyer may end a turn only on a free cell.
fn flyAIXYMap(b: *Battle, xp: i32, yp: i32, x: i32, y: i32, weight: i32, speed: i32, left: i32, over: bool) void {
    if (!bt.inside(x, y)) return;
    if (b.bfo(b.grid(x, y).land).move_cost == 0 and !over) return;
    const m = b.mc(x, y);
    if (!(m.land == 0 or weight < m.land or (weight == m.land and tryFly(b, x, y).*))) return;
    if (b.grid(x, y).pass) tryFly(b, x, y).* = speed != left;
    flyAIMap(b, x, y, weight, speed, left, over);
    if (left < speed and !b.grid(x, y).pass and b.grid(xp, yp).pass and tryFly(b, x, y).*) {
        tryFly(b, x, y).* = false;
        flyAIMap(b, x, y, weight + 1, speed, speed, over);
    }
}

/// FlyAIMap.
fn flyAIMap(b: *Battle, x: i32, y: i32, weight_in: i32, speed: i32, left_in: i32, over: bool) void {
    var weight = weight_in;
    var left = left_in;
    if (weight < 0) {
        b.mc(x, y).land = weight;
        weight = 1;
        tryFly(b, x, y).* = false;
    } else {
        left -= 1;
        if (b.grid(x, y).pass or !tryFly(b, x, y).*) b.mc(x, y).land = weight;
        b.mc(x, y).pass = true;
        if (left <= 0) {
            if (!b.grid(x, y).pass) return;
            weight += 1;
            left = speed;
        }
    }
    for (bt.neighbours(x, y)) |n| flyAIXYMap(b, x, y, n[0], n[1], weight, speed, left, over);
}

fn flyAIXYPath(b: *Battle, xp: i32, yp: i32, x: i32, y: i32, weight: i32, x1: i32, y1: i32, speed: i32, left: i32) void {
    if (!bt.inside(x, y)) return;
    const m = b.mc(x, y);
    if (!(m.pass and (weight < m.land or m.land == 0 or (weight == m.land and tryFly(b, x, y).*)))) return;
    if (b.grid(x, y).pass) tryFly(b, x, y).* = speed != left;
    flyAIPath(b, x, y, weight, x1, y1, speed, left);
    if (left < speed and !b.grid(x, y).pass and b.grid(xp, yp).pass and tryFly(b, x, y).*) {
        tryFly(b, x, y).* = false;
        flyAIPath(b, x, y, weight + 1, x1, y1, speed, speed);
    }
}

/// FlyAIPath.
fn flyAIPath(b: *Battle, x: i32, y: i32, weight_in: i32, x1: i32, y1: i32, speed: i32, left_in: i32) void {
    const p = &b.ai.path;
    var weight = weight_in;
    var left = left_in;
    if (weight < 0) {
        weight = 1;
        tryFly(b, x, y).* = false;
    } else {
        left -= 1;
        if (left <= 0) {
            if (!b.grid(x, y).pass) return;
            weight += 1;
            left = speed;
        }
        if (weight > p.min_weight + 1) return;
    }
    if (p.depth == p.stack.len) return;
    p.stack[p.depth] = .{ @intCast(x), @intCast(y) };
    p.depth += 1;
    defer p.depth -= 1;
    if (x == x1 and y == y1) {
        if ((p.found and weight < p.min_weight) or (!p.found and weight <= p.min_weight + 1)) p.record(weight);
        return;
    }
    for (bt.neighbours(x, y)) |n| flyAIXYPath(b, x, y, n[0], n[1], weight, x1, y1, speed, left);
}

// -- AI_CalcPower -------------------------------------------------------------

/// AI_InMeleeRange (this turn's move points) / AI_InMeleeRangePower (the
/// full ones): 3 from the own cell, 2 within reach, 1 with a march.
fn inMeleeRange(b: *Battle, f: *const Fighter, x: i32, y: i32, full: bool) i32 {
    if (!bt.inside(x, y)) return 0;
    if (f.x == x and f.y == y) return 3;
    if (!b.grid(x, y).pass or !b.mc(x, y).pass) return 0;
    const lim = if (full) b.maxSpeed(f) else f.speed;
    if (b.mc(x, y).land > lim and f.march and !isNeighbour(f, x, y)) {
        if (full) return 1;
        return if (f.stamina - b.skill(f, ab.march) >= 5) 1 else 0;
    }
    return 2;
}

fn markHeal(b: *Battle, f: *const Fighter, s: u1, unit: usize) void {
    for (&b.units[s]) |*o| {
        if (healable(b, o)) b.ai.power[s][unit][0] = if (f.ammo > 0) 2 else 1;
    }
}

fn markShots(b: *Battle, f: *const Fighter, s: u1, unit: usize) void {
    const es: u1 = 1 - s;
    for (&b.units[es], 0..) |*e, i| {
        if (!shootable(b, e)) continue;
        if (f.ammo > 0) {
            b.ai.power[s][unit][1] = 2;
            b.ai.power[es][i][if (b.skill(f, ab.magic_shot) != 0) 3 else 4] = 1;
        } else b.ai.power[s][unit][1] = 1;
    }
}

/// AI_CalcPower: what every unit can do this turn and what threatens it.
fn calcPower(b: *Battle) void {
    b.ai.power = @splat(@splat(@splat(0)));
    for (0..2) |sd| for (0..n_units) |u| {
        const s: u1 = @intCast(sd);
        const ref: Ref = .{ .side = s, .slot = @intCast(u) };
        const f = b.fighter(ref);
        if (f.life <= 0) continue;
        aiSelect(b, ref, true);
        const ms = b.maxSpeed(f);
        if (b.skill(f, ab.healer) != 0) {
            aiSelect(b, ref, true);
            markHeal(b, f, s, u);
            if (ms > 1 and b.ai.power[s][u][0] == 0) {
                const mm = moveMap(b, f, ms, f.speed);
                for (0..size) |i| for (0..size) |j| {
                    if (mm[i][j] == 0) continue;
                    const x: i32 = @intCast(i);
                    const y: i32 = @intCast(j);
                    b.clearMap();
                    b.shootMap(x, y, -1, rangeFrom(b, f, x, y));
                    markHeal(b, f, s, u);
                };
            }
        }
        if (b.rangedAttack(f) > 0) {
            aiSelect(b, ref, true);
            markShots(b, f, s, u);
            if (ms > 1) {
                const mm = moveMap(b, f, ms, ms);
                for (0..size) |i| for (0..size) |j| {
                    if (mm[i][j] == 0) continue;
                    const x: i32 = @intCast(i);
                    const y: i32 = @intCast(j);
                    b.clearMap();
                    b.shootMap(x, y, -1, rangeFrom(b, f, x, y));
                    markShots(b, f, s, u);
                };
            }
        }
        aiSelect(b, ref, true);
        const es: u1 = 1 - s;
        for (&b.units[es], 0..) |*e, i| {
            if (e.life == 0) continue;
            for (meleeCells(e.x, e.y)) |c| {
                if (inMeleeRange(b, f, c[0], c[1], true) > 0) {
                    b.ai.power[s][u][2] = 2;
                    b.ai.power[es][i][if (b.skill(f, ab.magic_strike) != 0) 5 else 6] = 1;
                }
            }
        }
    };
}

// -- AI_Main ------------------------------------------------------------------

/// AI_BetterPlace: (x, y) suits the unit better than (x1, y1).
fn betterPlace(b: *Battle, ref: Ref, x: i32, y: i32, x1: i32, y1: i32, mov: i32, mov1: i32) bool {
    const f = b.fighter(ref);
    const es: u1 = 1 - ref.side;
    if (b.skill(f, sk.round_attack) > 0 or roundButton(b, f)) {
        if (neighbourList(b, x, y, es).n > neighbourList(b, x1, y1, es).n) return true;
    }
    const here = b.grid(x, y).land;
    const there = b.grid(x1, y1).land;
    for (0..3) |i| {
        const k: i32 = @intCast(i);
        if (b.skill(f, ab.knowledge_first + k) > 3 and here == 2 + k and there != 2 + k) return true;
    }
    if (b.skill(f, ab.tireless) == 0) {
        if (mov1 > mov) return true;
        if (mov1 < mov) return false;
    }
    if (here == 3 and there != 3) return true;
    if (ref.side == 0 and x > x1) return true;
    if (ref.side == 1 and x < x1) return true;
    return false;
}

fn aiMain(b: *Battle, can_move: bool) void {
    if (b.nextUnit()) |r| aiSelect(b, r, true);
    const ref = b.active orelse return;
    const f = b.fighter(ref);
    if (!f.can_act or (f.speed <= 0 and b.skill(f, ab.rooted) == 0)) return;
    const s = ref.side;
    const es: u1 = 1 - s;

    const spell_choice = chooseSpell(b, ref);

    var can_heal: [n_units]i32 = @splat(0);
    var heal_at: [n_units][2]i32 = undefined;
    var can_range: [n_units]i32 = @splat(0);
    var range_at: [n_units][2]i32 = undefined;
    var can_melee: [n_units]i32 = @splat(0);
    var melee_at: [n_units][2]i32 = undefined;

    // Heal targets.
    if (b.skill(f, ab.healer) != 0 and f.ammo > 0) {
        aiSelect(b, ref, true);
        for (&b.units[s], 0..) |*o, i| {
            if (!healable(b, o)) continue;
            can_heal[i] = 3;
            heal_at[i] = .{ f.x, f.y };
        }
        if (f.speed > 1) {
            const mm = moveMap(b, f, f.speed, f.speed);
            for (0..size) |i| for (0..size) |j| {
                if (mm[i][j] == 0) continue;
                const x: i32 = @intCast(i);
                const y: i32 = @intCast(j);
                b.clearMap();
                b.shootMap(x, y, -1, rangeFrom(b, f, x, y));
                for (&b.units[s], 0..) |*o, k| {
                    if (can_heal[k] >= mm[i][j] or !healable(b, o)) continue;
                    can_heal[k] = mm[i][j];
                    heal_at[k] = .{ x, y };
                }
            };
        }
    }

    // Shot targets.
    if (f.ammo > 0 and b.rangedAttack(f) > 0) {
        aiSelect(b, ref, true);
        for (&b.units[es], 0..) |*e, i| {
            if (!shootable(b, e)) continue;
            can_range[i] = 3;
            range_at[i] = .{ f.x, f.y };
        }
        if (f.speed > 1) {
            const mm = moveMap(b, f, f.speed, f.speed);
            for (0..size) |i| for (0..size) |j| {
                if (mm[i][j] == 0) continue;
                const x: i32 = @intCast(i);
                const y: i32 = @intCast(j);
                b.clearMap();
                b.shootMap(x, y, -1, rangeFrom(b, f, x, y));
                for (&b.units[es], 0..) |*e, k| {
                    if (can_range[k] >= mm[i][j] or !shootable(b, e)) continue;
                    can_range[k] = mm[i][j];
                    range_at[k] = .{ x, y };
                }
            };
        }
    }

    // The cell to strike each enemy from.
    aiSelect(b, ref, true);
    for (&b.units[es], 0..) |*e, i| {
        if (e.life == 0) continue;
        for (meleeCells(e.x, e.y)) |c| {
            const r = inMeleeRange(b, f, c[0], c[1], false);
            if (r <= 0) continue;
            if (can_melee[i] == 0 or betterPlace(b, ref, c[0], c[1], melee_at[i][0], melee_at[i][1], can_melee[i], r)) {
                can_melee[i] = r;
                melee_at[i] = c;
            }
        }
    }

    var will_strike: ?usize = null;
    var melee_hit: i32 = 0;
    var will_shoot: ?usize = null;
    var shoot_hit: i32 = 0;
    var will_heal: ?usize = null;
    var heal_hit: i32 = 0;
    var taste: i32 = -1000;
    var kill_shoot = false;
    var kill_melee = false;
    aiSelect(b, ref, true);

    var enemy_army: i32 = 0;
    var our_army: i32 = 0;
    for (0..n_units) |i| {
        const e = &b.units[es][i];
        if (e.life > 0 and (calcMeleeHit(b, f, e, true) > 0 or calcRangedHit(b, f, e) > 0)) enemy_army += 1;
        if (b.units[s][i].life > 0) our_army += 1;
    }

    // Rate the heals.
    if (b.skill(f, ab.healer) != 0 and f.ammo > 0) {
        for (0..n_units) |i| {
            if (can_heal[i] <= 0) continue;
            const o = &b.units[s][i];
            const hit = @min(b.skill(f, ab.healer), b.maxLife(o) - o.life);
            var t = hit * 5 + b.defence(o) + b.rangedDefence(o) + b.resist(o) + (can_heal[i] - 1) * 2;
            t += hit * @divTrunc(heroClass(b, o), 2);
            if (can_heal[i] == 1) t = @divTrunc(t, 2);
            if (b.skill(o, sk.charmed) != 0) t = -2000;
            if (t > taste) {
                taste = t;
                will_heal = i;
                heal_hit = hit;
            }
        }
    }

    // Rate the shots.
    if (b.rangedAttack(f) > 0 and f.ammo > 0) {
        taste = -1000;
        for (0..n_units) |i| {
            if (can_range[i] <= 0) continue;
            const e = &b.units[es][i];
            var my_taste: i32 = 0;
            var my_hit: i32 = 0;
            const hero = e.hero_unit;
            if (b.skill(f, sk.web) > 0) {
                if (our_army > 2) {
                    my_taste = @divTrunc(e.life, 2);
                    my_hit = @divTrunc(e.life, 4);
                    if (hero) {
                        my_taste *= 2;
                        my_hit *= 2;
                    }
                    if (b.skill(f, sk.web) > b.attack(e)) my_taste *= 2;
                    if (b.skill(e, ab.immobile) > 0) {
                        my_taste = @divTrunc(my_taste, 3);
                        my_hit = @divTrunc(my_hit, 3);
                    }
                    if (b.skill(e, sk.web_immune) > 0) {
                        my_taste = -1000;
                        my_hit = 0;
                    }
                }
            } else if (b.skill(f, sk.stoning) > 0) {
                const stone = b.skill(f, sk.stoning);
                const res = b.resist(e);
                if (b.skill(e, ab.immobile) == 0 and our_army > 2 and stone > res) {
                    my_taste = (stone - res - 1) * (@divTrunc(e.life, 2) + b.attack(e));
                    my_hit = (stone - res - 1) * (@divTrunc(e.life, 4) + b.attack(e));
                    if (hero) {
                        my_taste *= 2;
                        my_hit *= 2;
                    }
                }
                if (stone <= res or b.skill(e, sk.stone_immune) > 0) {
                    my_taste = -2000;
                    my_hit = -3;
                }
            } else {
                my_hit = b.rangedHit(f, e);
                const double = b.skill(f, sk.double_shot);
                if (my_hit < e.life and can_range[i] == 3 and double > 0 and f.stamina >= double) {
                    my_hit *= 2;
                    can_range[i] = 4;
                }
                if (b.skill(e, ab.poison_immunity) == 0 and my_hit < e.life) my_hit += b.skill(f, ab.poison_shot) * 2;
                my_taste = my_hit * 2 - abs(my_hit - e.life - 1);
                var adder = @divTrunc(b.attack(e) + b.counterAttack(e), 5);
                adder += @divTrunc(b.rangedAttack(e) * e.ammo, 5);
                adder += my_hit * @divTrunc(heroClass(b, e), 2);
                if (my_hit >= @divTrunc(shoot_hit, 2) or my_hit >= e.life) my_taste += adder;
                if (adder > 0 and my_hit >= e.life) my_taste += adder;
                if (can_range[i] == 1) my_taste = @divTrunc(my_taste, 2);
                const ch = effectDuration(e, sk.charmed);
                if (ch > 0 and ch < 3) my_taste = -2000;
                if (my_taste > 0 and b.skill(e, sk.charmed) != 0) my_taste = @divTrunc(my_taste, 10);
                if (my_taste > 0 and b.skill(e, sk.shunned) != 0) my_taste = @divTrunc(my_taste, 2);
            }
            if ((will_shoot == null and enemy_army == 0) or (my_hit > 0 and my_taste > taste)) {
                taste = my_taste;
                will_shoot = i;
                shoot_hit = my_hit;
                kill_shoot = my_hit >= e.life;
            }
        }
    }

    // Rate the blows.
    if (b.skill(f, ab.no_combat) == 0) {
        taste = -1000;
        for (0..n_units) |i| {
            if (can_melee[i] <= 0) continue;
            const e = &b.units[es][i];
            var hit = aiMeleeHit(b, f, e);
            const crush = b.skill(f, sk.crush);
            if (hit < e.life and crush != 0 and f.stamina > crush and can_melee[i] > 1) {
                b.crush = true;
                hit = aiMeleeHit(b, f, e);
                b.crush = false;
                can_melee[i] = 5;
            }
            // A vampire's counterattack heals it.
            if ((hit < e.life or b.skill(e, ab.first_strike) != 0) and b.skill(e, ab.drain) > 0 and b.skill(f, ab.agility) == 0 and
                !bloodless(b, f) and e.stamina > 0 and !e.resting)
            {
                var anti = b.counterAttack(e) - (if (b.skill(e, ab.magic_strike) != 0) b.resist(f) else b.defence(f));
                anti = @min(anti, f.life);
                anti = @divTrunc(anti * b.skill(e, ab.drain), 100);
                if (anti > 0) hit -= anti;
            }
            var my_taste = hit * 2 - abs(hit - e.life - 1);
            const ch = effectDuration(e, sk.charmed);
            if (ch > 0 and ch < 3) my_taste = -2000;
            if (hit < e.life and b.skill(e, ab.poison_immunity) == 0) my_taste += b.skill(f, ab.poison) * 3;
            if (hit < e.life and b.skill(f, sk.trample) >= e.life - hit and b.skill(e, ab.flying) == 0) {
                hit = e.life;
                my_taste = hit * 2;
            }
            if (hit > 0 and !bloodless(b, e) and f.life < b.maxLife(f)) {
                const missing = b.maxLife(f) - f.life;
                if (b.skill(f, ab.drain) > 0) my_taste += @min(@divTrunc(hit * b.skill(f, ab.drain), 100), missing) * 2;
                if (b.skill(f, sk.soul_steal) > 0 and hit >= e.life) my_taste += @min(b.skill(f, sk.soul_steal), missing) * 2;
            }
            if (b.skill(f, sk.round_attack) > 0 or roundButton(b, f)) {
                const ox = f.x;
                const oy = f.y;
                f.x = melee_at[i][0];
                f.y = melee_at[i][1];
                const nl = neighbourList(b, f.x, f.y, es);
                if (nl.n > 1) {
                    if (roundButton(b, f)) can_melee[i] += 100;
                    for (nl.refs[0..nl.n]) |nr| {
                        if (nr.slot == i) continue;
                        const o = b.fighter(nr);
                        hit += aiMeleeHit(b, f, o);
                        my_taste += aiMeleeHit(b, f, o) * 2;
                    }
                }
                f.x = ox;
                f.y = oy;
            }
            var adder = b.skill(e, ab.immobile) * 5 + @divTrunc(b.attack(e), 2);
            if (hit < e.life and b.skill(f, ab.agility) == 0 and e.stamina > 0 and !e.resting) {
                const anti = b.counterAttack(e) - (if (b.skill(e, ab.magic_strike) != 0) b.resist(f) else b.defence(f));
                if (anti > 0) adder -= @min(anti, f.life);
            }
            adder += @divTrunc(b.rangedAttack(e) * e.ammo, 5);
            adder += hit * @divTrunc(heroClass(b, e), 2);
            if (hit >= @divTrunc(melee_hit, 2) or hit >= e.life) my_taste += adder;
            if (adder > 0 and hit >= e.life) my_taste += adder;
            if (my_taste > 0 and b.skill(e, sk.charmed) != 0) my_taste = @divTrunc(my_taste, 10);
            const sh = effectDuration(e, sk.shunned);
            if (my_taste > 0 and sh > 1) my_taste = @divTrunc(my_taste, sh);
            if (can_melee[i] == 1 and my_taste > 0) my_taste = @divTrunc(my_taste, 2);
            if ((will_strike == null and enemy_army == 0) or (hit > 0 and my_taste > taste)) {
                taste = my_taste;
                will_strike = i;
                melee_hit = hit;
                kill_melee = hit >= e.life;
            }
        }
    }

    // The order.
    if (spell_choice) |ch| {
        if (ch.hit >= shoot_hit and ch.hit >= melee_hit and ch.hit >= heal_hit) {
            b.ai.index_ai = -1;
            castChosen(b, ref, ch);
            return;
        }
    }
    if (will_heal) |wh| {
        if (heal_hit >= shoot_hit and heal_hit >= melee_hit) {
            b.ai.index_ai = -1;
            if (f.x != heal_at[wh][0] or f.y != heal_at[wh][1]) {
                if (can_heal[wh] == 1) b.goMarch(ref);
                b.moveTo(ref, heal_at[wh][0], heal_at[wh][1]);
            }
            b.heal(ref, .{ .side = s, .slot = @intCast(wh) });
            return;
        }
    }
    if (will_shoot) |ws| {
        const better = shoot_hit >= melee_hit or kill_shoot or
            (b.maxAmmo(f) > 3 and b.attack(f) < b.rangedAttack(f) and shoot_hit >= @divTrunc(melee_hit, 2));
        if (better and (!kill_melee or kill_shoot)) {
            b.ai.index_ai = -1;
            const tr: Ref = .{ .side = es, .slot = @intCast(ws) };
            if (f.x == range_at[ws][0] and f.y == range_at[ws][1]) {
                b.double_shot = can_range[ws] == 4;
                b.shoot(ref, tr);
                b.double_shot = false;
            } else {
                if (can_range[ws] == 1) b.goMarch(ref);
                b.moveTo(ref, range_at[ws][0], range_at[ws][1]);
                b.shoot(ref, tr);
            }
            return;
        }
    }
    if (will_strike) |wk| {
        b.ai.index_ai = -1;
        if (can_melee[wk] == 1) b.goMarch(ref);
        b.crush = @rem(can_melee[wk], 100) == 5;
        b.round_attack = can_melee[wk] > 99;
        _ = b.attackFrom(ref, melee_at[wk][0], melee_at[wk][1], .{ .side = es, .slot = @intCast(wk) });
        b.crush = false;
        b.round_attack = false;
        return;
    }
    if (can_move) noTarget(b, ref, enemy_army);
}

// -- the spells -----------------------------------------------------------------

/// AI_CalcDangerZone: the steps from the nearest enemy of the side.
fn calcDangerZone(b: *Battle) void {
    b.ai.danger = @splat(@splat(100));
    for (&b.units[1 - b.side]) |*e| {
        if (e.present() and e.life > 0) stepZone(b, e.x, e.y, 0);
    }
}

fn stepZone(b: *Battle, x: i32, y: i32, w: i32) void {
    b.ai.danger[@intCast(x)][@intCast(y)] = w;
    for (bt.neighbours(x, y)) |n| {
        if (bt.inside(n[0], n[1]) and b.ai.danger[@intCast(n[0])][@intCast(n[1])] > w + 1) stepZone(b, n[0], n[1], w + 1);
    }
}

fn danger(b: *const Battle, x: i32, y: i32) i32 {
    return b.ai.danger[@intCast(x)][@intCast(y)];
}

/// BF_Unit::pHero: slot 0 of a squad (a guard's too).
fn hasHero(f: *const Fighter) bool {
    return f.num == 0 and !f.summoned and f.present();
}

/// AI_GetUnitTaste: the unit's worth to its side.
fn unitTaste(b: *const Battle, u: *const Fighter) i32 {
    var r = b.attack(u) + @divTrunc(b.counterAttack(u), 2) + @divTrunc(b.rangedAttack(u) * u.ammo, 4) + @divTrunc(b.skill(u, ab.healer) * u.ammo, 4);
    r += b.utype(u).level * 2 + u.unit.level;
    if (hasHero(u)) {
        // A guard's dcHero has class 0 and level 0.
        const h = b.armies[u.home].hero;
        const class1: i32 = if (h) |x| x.class1 else 0;
        const level: i32 = if (h) |x| x.level else 0;
        r += (class1 - 1) * 3 + level;
    }
    return r;
}

/// GetTeleportTaste: the blows `u` could strike from (x, y).
fn teleportTaste(b: *Battle, u: *Fighter, x: i32, y: i32) i32 {
    if (!b.grid(x, y).pass or b.bfo(b.grid(x, y).land).move_cost <= 0) return 0;
    const ox = u.x;
    const oy = u.y;
    u.x = x;
    u.y = y;
    defer {
        u.x = ox;
        u.y = oy;
    }
    const nl = neighbourList(b, x, y, 1 - b.side);
    if (nl.n == 0) return 0;
    const r66 = b.skill(u, sk.round_button);
    const round = b.skill(u, sk.round_attack) > 0 or (r66 > 0 and u.stamina >= r66);
    var result: i32 = 0;
    for (nl.refs[0..nl.n]) |nr| {
        const n = b.fighter(nr);
        var hit = aiMeleeHit(b, u, n);
        if (n.life <= hit) hit = 5 + n.life * 2;
        if (n.ammo < 2) hit = @divTrunc(hit, 2);
        if (round) result += hit else result = @max(result, hit);
    }
    return result;
}

/// The taste and iGainSpellDamage of a spell on one unit.
const Taste = struct { taste: i32 = 0, gain: i32 = 0 };

/// A shooter or healer that nobody can reach.
fn shooterOnly(p: [7]i32) bool {
    return (p[0] == 2 or p[1] == 2) and p[3] == 0 and p[4] == 0 and p[5] == 0 and p[6] == 0;
}

/// AttBuff of a killed or hurt target.
fn attBuff(b: *const Battle, t: *const Fighter, p: [7]i32) i32 {
    var r: i32 = 0;
    if (shooterOnly(p)) {
        r += @divTrunc(b.rangedAttack(t) * t.ammo, 4);
        r += @divTrunc(b.skill(t, ab.healer) * t.ammo, 4);
    }
    return r + @divTrunc(b.rangedAttack(t) * t.ammo, 5) + @divTrunc(b.attack(t) + b.defence(t) + b.rangedDefence(t), 2);
}

/// `v * (skill / 7)` of the fear and tiredness tastes.
fn sevenths(hit: i32, v: i32) i32 {
    return hit * @divTrunc(v, 7);
}

fn hasSpellEffect(t: *const Fighter, sp_i: i32) bool {
    for (t.effectList()) |e| {
        if (e.spell == sp_i) return true;
    }
    return false;
}

fn unitType(b: *const Battle, t: i32) ?*const @import("../data/proto.zig").UnitType {
    if (t <= 0 or t >= b.pr.units.len) return null;
    return &b.pr.units[@intCast(t)];
}

/// AI_GetEnemySpellTaste.
fn enemyTaste(b: *Battle, a: *Fighter, sp_i: i32, tr: Ref) Taste {
    const sp = bs.spellOf(b, sp_i);
    const t = b.fighter(tr);
    if (bs.immune(b, sp_i, t)) return .{};
    const p = b.ai.power[tr.side][tr.slot];
    // arUnitPower[AA][Unit] of the polymorph (sic).
    const pa = b.ai.power[1 - tr.side][tr.slot];
    const rp: i32 = @intFromBool(sp.resist_power);
    const rd: i32 = @intFromBool(sp.resist_duration);
    const dp: i32 = @intFromBool(sp.defence_power);
    var gain: i32 = 0;
    var posessed = true;
    var buff: i32 = 0;
    var att_buff: i32 = 0;
    var base_damage: i32 = 0;
    // Hit is not initialised in the original; it keeps its last value.
    var hit: i32 = 0;
    var self_heal: i32 = 0;
    var drain = false;
    var resist = b.resist(t) - b.skill(a, 908) - b.heroBonus(a, 908);
    resist = @max(@divTrunc(resist * sp.resist_mod, 100), 0);
    for (sp.effects) |e| {
        if (e.id <= 0) continue;
        var power = e.power;
        const pb = @divTrunc(b.skill(a, 905) * sp.power_mod, 100) + @divTrunc(b.heroBonus(a, 905) * sp.power_mod, 100);
        if (e.power > 0) power += pb;
        if (e.power < 0) power -= pb;
        var dur = e.duration;
        var index = e.id;
        if (dur > 0) {
            dur += @divTrunc(b.skill(a, 906) * sp.duration_mod, 100) + @divTrunc(b.heroBonus(a, 906) * sp.duration_mod, 100);
            if (index == 1) index = -1;
        }
        var fall = false;
        switch (index) {
            1 => if (e.duration == -1) {
                base_damage += power;
                hit = b.rollHit(power, resist * rp + b.defence(t) * dp);
                gain += hit;
                att_buff += attBuff(b, t, p);
                if (drain) self_heal = @min(self_heal + hit, b.maxLife(a) - a.life);
            },
            10 => if (sp.negative and e.duration == -1) {
                base_damage += power;
                hit = @min(b.rollHit(power, resist * rp), t.ammo);
                if (hit > 0) {
                    gain = hit;
                    buff += @divTrunc(hit * (b.rangedAttack(t) + b.skill(t, ab.healer)), 2);
                    att_buff += attBuff(b, t, p);
                }
            },
            11 => if (b.skill(t, ab.tireless) == 0 and sp.negative and e.duration == -1) {
                hit += power - resist * rp;
                if (hit > 0) {
                    if (t.stamina < hit) hit = t.stamina * 2;
                    if (p[0] == 2) hit += sevenths(hit, b.skill(t, ab.healer));
                    if (p[1] == 2) hit += sevenths(hit, b.rangedAttack(t));
                    if (p[2] == 2) hit += sevenths(hit, b.attack(t));
                    buff += hit;
                }
            },
            12 => if (b.skill(t, ab.fearless) == 0 and sp.negative and e.duration == -1) {
                hit = power - resist * rp;
                if (hit > 0 and t.morale > 0) {
                    if (t.morale < hit) {
                        hit = 20 + t.morale * 4;
                    } else if (t.morale < hit + 5) hit = @divTrunc(hit * 3, 2);
                    if (p[0] == 2) hit += sevenths(hit, b.skill(t, ab.healer));
                    if (p[1] == 2) hit += sevenths(hit, b.rangedAttack(t));
                    if (p[2] == 2) hit += sevenths(hit, b.attack(t));
                    buff += hit;
                }
            },
            31 => {
                // Dispel.
                power = e.power + b.skill(a, 908) + b.heroBonus(a, 908);
                for (t.effectList()) |ef| {
                    if (ef.spell > 0 and !bs.spellOf(b, ef.spell).negative) {
                        buff += if (ef.time > power) power * ef.power else ef.time * ef.power;
                    }
                    if (ef.kind == bt.eff_charm) {
                        buff += 50;
                        gain += b.attack(t) * (ef.time - 1);
                        posessed = false;
                    }
                    if (ef.kind == bt.eff_polymorph) {
                        buff += 50;
                        gain += b.attack(t) * (ef.time - 1);
                    }
                }
                if (t.summoned and power > resist) {
                    base_damage += power;
                    hit = b.rollHit(power + e.power * 2, resist);
                    gain += hit;
                    att_buff += attBuff(b, t, p);
                }
            },
            47 => if (resist < 4 and b.skill(t, ab.immobile) == 0) {
                // The web.
                const d: i32 = if (dur > b.attack(t)) 2 else 1;
                if (p[5] > 0 or p[6] > 0) {
                    buff += b.counterAttack(t) * d;
                    if (hasHero(t)) buff += 10;
                }
            },
            71 => drain = true,
            bt.eff_charm => {
                const can = b.skill(t, ab.immobile) == 0 and (p[0] == 2 or p[1] == 2 or p[2] == 2);
                if (e.duration == -1) {
                    if (power > resist and can) {
                        gain += b.attack(t) + b.rangedAttack(t);
                        buff += b.attack(t) + b.rangedAttack(t);
                    }
                } else {
                    dur -= resist * rd;
                    if (dur > 0 and can) {
                        gain += (b.attack(t) + b.rangedAttack(t)) * dur;
                        buff += b.attack(t) + b.rangedAttack(t) + t.life;
                    }
                }
            },
            92 => {
                // The suicide falls through to the enchantments.
                if (resist * rp <= power) {
                    gain += b.meleeHit(t, t, true);
                    att_buff += attBuff(b, t, p);
                }
                fall = true;
            },
            else => fall = true,
        }
        if (!fall) continue;
        dur -= resist * rd;
        if (hasSpellEffect(t, sp_i)) dur = 0;
        if (dur <= 1) continue;
        if (index > 1000 and (b.utype(t).level > 2 or hasHero(t))) {
            // Polymorph.
            if (unitType(b, index - 1000)) |nt| {
                if (p[2] == 2) buff += (b.attack(t) - nt.attack) * dur;
                buff += b.rangedAttack(t) * t.ammo;
                if (pa[5] > 0 or p[6] > 0) {
                    gain += t.life;
                    buff += (t.life - nt.life) * dur;
                }
            }
        }
        switch (index) {
            -1 => {
                // Poison.
                buff -= dur * power;
                gain -= dur * power;
                if (shooterOnly(p) and power < 0) {
                    buff += @divTrunc(b.rangedAttack(t) * t.ammo, 4);
                    buff += @divTrunc(b.skill(t, ab.healer) * t.ammo, 4);
                }
            },
            2 => if (p[2] == 2) {
                buff += if (abs(power) > b.attack(t) and power < 0) (b.attack(t) - 1) * dur else -dur * power;
            },
            3 => if (p[5] > 0 or p[6] > 0) {
                buff += if (abs(power) > b.counterAttack(t) and power < 0) (b.counterAttack(t) - 1) * dur else -dur * power;
            },
            4 => if (p[6] > 0) {
                buff += if (abs(power) > b.defence(t) and power < 0) b.defence(t) * dur else -dur * power;
            },
            5 => if (p[4] > 0) {
                buff += if (abs(power) > b.rangedDefence(t) and power < 0) b.rangedDefence(t) * dur else -dur * power;
            },
            6 => if (p[3] > 0 or p[5] > 0) {
                buff += if (abs(power) > b.resist(t) and power < 0) b.resist(t) * dur else -dur * power;
            },
            7 => if (b.maxSpeed(t) > 1 and p[0] == 0 and p[1] == 0 and p[2] == 0) {
                // Slowness.
                if (abs(power) > b.maxSpeed(t) and power < 0) {
                    buff += (b.resist(t) + @divTrunc(b.attack(t), 5)) * dur;
                } else buff -= dur * (power - @divTrunc(b.attack(t), 5));
            },
            8 => if (p[1] == 2) {
                buff += if (abs(power) > b.rangedAttack(t) and power < 0) (b.rangedAttack(t) - 1) * dur else -dur * power;
            },
            36, 38 => {
                // No moves or no blows.
                if (p[1] == 2 or p[2] == 2) {
                    gain += b.attack(t) + b.rangedAttack(t);
                    buff += b.maxLife(t) * dur;
                } else buff += 2 * dur;
            },
            else => {},
        }
    }
    var taste = buff;
    const ch = effectDuration(t, sk.charmed);
    if (gain > 0 and posessed and ch > 0 and ch < 3) gain = -gain;
    if (gain >= t.life) {
        gain = t.life;
        if (self_heal > t.life) gain *= 2 else gain += self_heal;
        taste += gain * 6;
        taste += att_buff;
    } else {
        gain += self_heal;
        // A sleeping target is worth less.
        const sleep = effectDuration(t, sk.shunned);
        if (sleep > 1) taste += @divTrunc(gain, sleep) else taste += gain * 3;
        if (gain > 5 or gain >= @divTrunc(base_damage, 2)) taste += att_buff;
    }
    taste += @divTrunc(taste * unitTaste(b, t), 50);
    return .{ .taste = taste, .gain = gain };
}

/// AI_GetAllySpellTaste.
fn allyTaste(b: *Battle, a: *Fighter, sp_i: i32, tr: Ref) Taste {
    const sp = bs.spellOf(b, sp_i);
    const t = b.fighter(tr);
    if (bs.immune(b, sp_i, t)) return .{};
    const p = b.ai.power[tr.side][tr.slot];
    const rp: i32 = @intFromBool(sp.resist_power);
    const rd: i32 = @intFromBool(sp.resist_duration);
    const dp: i32 = @intFromBool(sp.defence_power);
    var gain: i32 = 0;
    var buff: i32 = 0;
    var friendly = false;
    const resist = @max(b.resist(t) - b.heroBonus(a, 908), 0);
    for (sp.effects) |e| {
        if (e.id <= 0) continue;
        var power = e.power;
        const pb = @divTrunc(b.heroBonus(a, 905) * sp.power_mod, 100);
        if (e.power > 0) power += pb;
        if (e.power < 0) power -= pb;
        var dur = e.duration;
        var index = e.id;
        if (dur > 0) {
            dur += @divTrunc(b.heroBonus(a, 906) * sp.duration_mod, 100);
            if (index == 1) index = -1;
        }
        var fall = false;
        switch (index) {
            1 => if (e.duration == -1) {
                if (sp.negative and !friendly) {
                    gain -= b.rollHit(power, resist * rp + b.defence(t) * dp);
                } else gain = power;
            },
            10 => if (!sp.negative and e.duration == -1 and (p[0] > 0 or p[1] > 0)) {
                // More ammo.
                const per = b.rangedAttack(t) + b.skill(t, ab.healer);
                buff += @min(power, b.maxAmmo(t) - t.ammo) * per;
                if (t.ammo == 0) gain += per;
            },
            11 => if (b.skill(t, ab.tireless) == 0 and !sp.negative and e.duration == -1) {
                const missing = b.maxStamina(t) - t.stamina;
                const hit: i32 = if (missing > power) power else if (t.stamina >= 5) 0 else missing;
                if (hit > 0) {
                    buff += hit;
                    if (t.stamina == 0) {
                        buff += hit * b.attack(t);
                    } else if (t.stamina < 5) {
                        buff += @divTrunc(hit * b.attack(t), 5);
                        if (t == a) gain += b.attack(t);
                    }
                }
            },
            12 => if (b.skill(t, ab.fearless) == 0 and !sp.negative and e.duration == -1) {
                power = @max(@min(power, b.maxMorale(t) - t.morale), 0);
                buff += power;
                if (t.morale == 0) gain += b.attack(t) * power;
                if (t.morale < 5) buff += power;
            },
            31 => {
                // Dispel of the curses.
                if (hasHero(a)) power = e.power + b.heroBonus(a, 908);
                for (t.effectList()) |ef| {
                    if (ef.spell > 0 and bs.spellOf(b, ef.spell).negative) {
                        buff += if (ef.time > power) power * ef.power else ef.time * ef.power;
                    }
                }
            },
            72 => friendly = true,
            75 => if (b.skill(t, ab.immobile) == 0 and t.morale > 0 and !t.can_act) {
                // One more move.
                if (p[0] == 2) gain += b.skill(t, ab.healer);
                if (p[1] == 2) gain += b.rangedAttack(t);
                if (p[2] == 2) gain += b.attack(t);
            },
            89 => if (b.attack(t) > 5 and t.ammo == 0 and p[2] < 2 and t.can_act) {
                // Teleport.
                var big: i32 = 0;
                for (0..size) |x| for (0..size) |y| {
                    big = @max(big, teleportTaste(b, t, @intCast(x), @intCast(y)));
                };
                gain = big;
            },
            91 => if (t.life <= @divTrunc(b.maxLife(t), 4) and (p[5] > 0 or p[6] > 0)) {
                // The dark pact.
                const lvl = b.utype(t).level;
                gain = 5 + (lvl - 1) * 10;
                buff += lvl * 50;
            },
            else => fall = true,
        }
        if (!fall) continue;
        dur -= resist * rd;
        if (hasSpellEffect(t, sp_i)) dur = 0;
        if (dur <= 1) continue;
        if (index > 1000) {
            if (!t.can_act) dur -= 1;
            if (unitType(b, index - 1000)) |nt| {
                const own = b.typeStat(t, stat_attack);
                gain = nt.attack - own;
                if (t.ammo >= 2) gain -= b.rangedAttack(t) * 2;
                gain = @divTrunc(gain * b.attack(t), own + 1) * dur;
                if (hasHero(t)) gain = @divTrunc(gain * 2, 3);
            }
        }
        switch (index) {
            -1 => if (t.life < b.maxLife(t)) {
                // Regeneration.
                buff += @divTrunc(b.rangedAttack(t) * t.ammo, 20);
                buff += @divTrunc(b.attack(t), 2);
                if (p[3] > 0 or (p[4] > 0 and p[5] > 0) or p[6] > 0) buff += dur * power;
            },
            2 => if (p[2] == 2) {
                buff += dur * power;
            },
            3 => if (p[5] > 0 or p[6] > 0) {
                buff += dur * power;
            },
            4 => if (p[6] > 0) {
                buff += dur * power;
            },
            5 => if (p[4] > 0) {
                buff += dur * power;
            },
            6 => if (p[3] > 0 or p[5] > 0) {
                buff += dur * power;
            },
            7 => if (p[0] == 0 and p[1] == 0 and p[2] == 0) {
                buff += dur * (power + @divTrunc(b.attack(t), 5));
            },
            8 => if (p[1] == 2) {
                buff += dur * power;
            },
            ab.drain => if (t.life < b.maxLife(t) and p[2] == 2) {
                buff += @divTrunc(b.attack(t) * power, 100) * dur;
                gain += @divTrunc(b.attack(t) * power, 100);
            },
            else => {},
        }
    }
    var taste = buff;
    if (gain < 0) {
        if (abs(gain) >= t.life) {
            gain = -t.life;
            taste += gain * 6;
        } else {
            if (Battle.effect(t, sk.shunned) > 0) {
                const sleep = effectDuration(t, sk.shunned);
                gain += sleep - 1;
                taste += 3 * (sleep - 1);
            }
            taste += gain * 3;
        }
    } else {
        if (sp.effects[0].id == 1 and gain >= b.maxLife(t) - t.life) gain = b.maxLife(t) - t.life;
        taste += gain * 3;
        taste += @divTrunc(gain * b.utype(t).level, 2);
        if (hasHero(t)) taste += gain;
    }
    taste += @divTrunc(taste * unitTaste(b, t), 50);
    return .{ .taste = taste, .gain = gain };
}

/// dcUnit::GetAttack of the stats getter.
const stat_attack = 2;

/// AI_CalcSpellPenalty: the price of the spell, the sacrifice victim.
fn spellPenalty(b: *Battle, a: *Fighter, s: u1, sp_i: i32) i32 {
    const sp = bs.spellOf(b, sp_i);
    var t: i32 = 0;
    if (sp.life_cost >= a.life) t -= a.life * 6 + 40 else t -= sp.life_cost * 3;
    t -= @divTrunc(sp.stamina_cost, 2);
    if (sp.sacrifice) {
        b.ai.sac_victim = null;
        var v2: i32 = 0;
        for (b.units[s][0..bt.max_squad], 0..) |*o, i| {
            if (!o.present() or o.life <= 0 or b.unitKind(o) != 1) continue;
            var v = o.life * b.utype(o).level + o.unit.level * 2;
            if (b.skill(o, sk.charmed) != 0) v = -v;
            if (b.ai.sac_victim == null or v < v2) {
                v2 = v;
                b.ai.sac_victim = @intCast(i);
            }
        }
        t -= if (b.ai.sac_victim == null) 1000 else v2;
    }
    if (a.hero_unit) {
        t -= sp.cost;
    } else t -= @divTrunc(b.skill(a, 2000 + sp_i) * (b.rangedAttack(a) + b.skill(a, ab.healer)), 2);
    return t;
}

/// The spell AI_Main chose: the target is a unit (side, slot), a corpse
/// or a cell (x, y), as SpellTarget1 / SpellTarget2.
const Choice = struct { sp: i32, t1: i32 = -1, t2: i32 = -1, hit: i32 = 0 };

const Best = struct {
    choice: ?Choice = null,
    taste: i32 = 0,
    hit: i32 = 0,

    fn offer(self: *Best, taste: i32, gain: i32, c: Choice) void {
        if (taste > self.taste and (gain > 0 or taste > 0)) {
            self.taste = taste;
            self.hit = gain;
            self.choice = c;
        }
    }
};

/// The summon a corpse or a free cell gives: (Buff, Damage).
fn summonTaste(b: *const Battle, idx: i32, power: i32, dz: i32, buff_less: i32, dz_mul: i32, speed_type: i32, dz_less: i32) [2]i32 {
    const nt = unitType(b, idx) orelse return .{ 0, 0 };
    const buff = (nt.level + 1) * 10 + power * 3 - buff_less - dz * dz_mul + nt.life;
    var dmg = nt.attack + @divTrunc(power, 2);
    if (dz >= 2) {
        const sp = if (unitType(b, speed_type)) |st| st.speed else 0;
        dmg = @max(dmg - dz - dz_less + sp, 1);
    }
    return .{ buff, dmg };
}

/// The magic part of AI_Main: the spell worth casting, if any.
fn chooseSpell(b: *Battle, ref: Ref) ?Choice {
    const a = b.fighter(ref);
    const s = ref.side;
    const es: u1 = 1 - s;
    var cast: [bt.spellbook_size]i32 = @splat(0);
    var count: usize = 0;
    if (a.hero_unit) {
        if (b.can_cast[s] > 0) for (b.book[s], 0..) |slot, i| {
            if (slot[0] > 0 and slot[1] > 0 and b.gem[s] >= bs.price(b, a, slot[0])) {
                cast[i] = slot[0];
                count += 1;
            }
        };
    } else {
        for (1..b.pr.spells.len) |i| {
            const k = b.skill(a, 2000 + @as(i32, @intCast(i)));
            if (k != 0 and a.ammo >= k and count < cast.len) {
                cast[count] = @intCast(i);
                count += 1;
            }
        }
    }
    if (count == 0) return null;
    aiSelect(b, ref, true);
    var best: Best = .{};
    for (cast) |sp_i| {
        if (sp_i <= 0) continue;
        const sp = bs.spellOf(b, sp_i);
        if (sp.sacrifice and b.can_cast[s] <= 1) continue;
        const e0 = sp.effects[0];
        // A single unit.
        if (sp.target == 1) {
            if (sp.on_enemy) for (0..n_units) |u| {
                const t = &b.units[es][u];
                if (!t.present() or t.life <= 0) continue;
                const tt = enemyTaste(b, a, sp_i, .{ .side = es, .slot = @intCast(u) });
                best.offer(tt.taste + spellPenalty(b, a, s, sp_i), tt.gain, .{ .sp = sp_i, .t1 = es, .t2 = @intCast(u) });
            };
            if (sp.on_ally) for (0..n_units) |u| {
                const t = &b.units[s][u];
                if (!t.present() or t.life <= 0) continue;
                const tt = allyTaste(b, a, sp_i, .{ .side = s, .slot = @intCast(u) });
                best.offer(tt.taste + spellPenalty(b, a, s, sp_i), tt.gain, .{ .sp = sp_i, .t1 = s, .t2 = @intCast(u) });
            };
        }
        // The whole field.
        if (sp.target == 0 and sp.area == 2) {
            var taste: i32 = 0;
            var dmg: i32 = 0;
            if (sp.on_ally) for (0..n_units) |u| {
                const t = &b.units[s][u];
                if (!t.present() or t.life <= 0) continue;
                const tt = allyTaste(b, a, sp_i, .{ .side = s, .slot = @intCast(u) });
                taste += tt.taste;
                dmg += tt.gain;
            };
            if (sp.on_enemy) for (0..n_units) |u| {
                const t = &b.units[es][u];
                if (!t.present() or t.life <= 0) continue;
                const tt = enemyTaste(b, a, sp_i, .{ .side = es, .slot = @intCast(u) });
                taste += tt.taste;
                dmg += tt.gain;
            };
            best.offer(taste + spellPenalty(b, a, s, sp_i), dmg, .{ .sp = sp_i });
        }
        // The caster.
        if (sp.target == 0 and sp.area == 0) {
            const tt = allyTaste(b, a, sp_i, ref);
            best.offer(tt.taste + spellPenalty(b, a, s, sp_i), tt.gain, .{ .sp = sp_i });
        }
        // A corpse.
        if (sp.target == 2 or sp.target == 4) {
            for ([2]u1{ es, s }) |cs| {
                const ally = cs == s;
                if (!(if (ally) sp.on_ally else sp.on_enemy)) continue;
                for (0..n_units) |u| {
                    const c = &b.units[cs][u];
                    if (!c.present() or c.life != 0 or !c.in_battle) continue;
                    if (sp.target == 4 and !b.grid(c.x, c.y).pass) continue;
                    var buff: i32 = 0;
                    var dmg: i32 = 0;
                    if (e0.id > 1000) {
                        var power = e0.power;
                        if (ally or hasHero(a)) power += @divTrunc(b.skill(a, 907), 2) + b.skill(a, 909);
                        power += @divTrunc(b.heroBonus(a, 907), 2) + b.heroBonus(a, 909);
                        const v = summonTaste(b, e0.id - 1000, power, danger(b, c.x, c.y), @intFromBool(ally), 1, e0.id - 1000, 1);
                        buff = v[0];
                        dmg = v[1];
                    }
                    if (ally and e0.id == 1) {
                        // Resurrection.
                        buff = b.utype(c).level * 15 + c.unit.level * 2;
                        dmg = b.utype(c).attack;
                    }
                    const taste = buff + dmg * 3 + spellPenalty(b, a, s, sp_i);
                    if (taste > best.taste and (dmg > 0 or buff > 0)) {
                        best.taste = taste;
                        best.hit = dmg;
                        best.choice = .{ .sp = sp_i, .t1 = cs, .t2 = @intCast(u) };
                    }
                }
            }
        }
        // A summon on a free cell.
        if (sp.target == 5) {
            b.clearMap();
            b.shootMap(a.x, a.y, -1, @divTrunc(b.skill(a, 907), 2) + 1);
            for (0..size) |i| for (0..size) |j| {
                const x: i32 = @intCast(i);
                const y: i32 = @intCast(j);
                if (b.mc(x, y).shoot == 0 or !b.grid(x, y).pass or b.bfo(b.grid(x, y).land).move_cost <= 0) continue;
                if (e0.id <= 1000) continue;
                const power = e0.power + b.skill(a, 907) + b.heroBonus(a, 907);
                // The speed is taken of the unit type before (sic).
                const v = summonTaste(b, e0.id - 1000, power, danger(b, x, y), 0, 2, e0.id - 1001, 0);
                const taste = v[0] + v[1] * 3 + spellPenalty(b, a, s, sp_i);
                if (taste > best.taste and (v[1] > 0 or v[0] > 0)) {
                    best.taste = taste;
                    best.hit = v[1];
                    best.choice = .{ .sp = sp_i, .t1 = x, .t2 = y };
                }
            };
        }
        // An area around a cell.
        if (sp.target == 3) {
            for (0..size) |i| for (0..size) |j| {
                const ci: i32 = @intCast(i);
                const cj: i32 = @intCast(j);
                var taste: i32 = 0;
                var dmg: i32 = 0;
                b.clearMap();
                b.shootMap(ci, cj, -1, sp.radius);
                if (sp.on_ally or sp.on_enemy) for (0..size) |x| for (0..size) |y| {
                    const xx: i32 = @intCast(x);
                    const yy: i32 = @intCast(y);
                    if (b.mc(xx, yy).shoot == 0 and !(x == i and y == j)) continue;
                    for ([2]u1{ s, es }) |ts| {
                        if (!(if (ts == s) sp.on_ally else sp.on_enemy)) continue;
                        for (0..n_units) |k| {
                            const t = &b.units[ts][k];
                            if (!t.present() or t.life <= 0 or t.x != xx or t.y != yy) continue;
                            const tr: Ref = .{ .side = ts, .slot = @intCast(k) };
                            const tt = if (ts == s) allyTaste(b, a, sp_i, tr) else enemyTaste(b, a, sp_i, tr);
                            taste += tt.taste;
                            dmg += tt.gain;
                        }
                    }
                };
                best.offer(taste + spellPenalty(b, a, s, sp_i), dmg, .{ .sp = sp_i, .t1 = ci, .t2 = cj });
            };
        }
    }
    var c = best.choice orelse return null;
    c.hit = if (best.hit == 0) @divTrunc(best.taste, 10) else best.hit;
    return c;
}

/// The units of side `ts` the spell reaches that are not immune to it.
fn castOn(b: *Battle, sp_i: i32, ts: u1, cr: Ref, cell: ?[2]i32) void {
    for (0..n_units) |k| {
        const t = &b.units[ts][k];
        if (!t.present() or t.life <= 0) continue;
        if (cell) |c| if (t.x != c[0] or t.y != c[1]) continue;
        if (bs.immune(b, sp_i, t)) continue;
        bs.underSpell(b, .{ .side = ts, .slot = @intCast(k) }, sp_i, cr);
    }
}

/// "Заклинание": the chosen spell is cast and paid for.
fn castChosen(b: *Battle, ref: Ref, ch: Choice) void {
    const sp = bs.spellOf(b, ch.sp);
    const s = ref.side;
    const es: u1 = 1 - s;
    b.emit(.{ .cast = .{ .who = ref, .spell = ch.sp } });
    switch (sp.target) {
        1 => {
            const tr: Ref = .{ .side = @intCast(ch.t1), .slot = @intCast(ch.t2) };
            if (sp.effects[0].id == 89) {
                // Teleport to the cell of the best blow.
                const t = b.fighter(tr);
                var big: i32 = 0;
                var to: ?[2]i32 = null;
                for (0..size) |x| for (0..size) |y| {
                    const h = teleportTaste(b, t, @intCast(x), @intCast(y));
                    if (h > big) {
                        big = h;
                        to = .{ @intCast(x), @intCast(y) };
                    }
                };
                if (to) |c| bs.replace(b, tr, c[0], c[1]);
            } else bs.underSpell(b, tr, ch.sp, ref);
        },
        2, 4 => bs.corpseSpell(b, ch.sp, .{ .side = @intCast(ch.t1), .slot = @intCast(ch.t2) }, ref),
        5 => bs.tileSpell(b, ch.sp, ch.t1, ch.t2, ref),
        3 => {
            b.clearMap();
            b.shootMap(ch.t1, ch.t2, -1, sp.radius);
            if (sp.on_ally or sp.on_enemy) for (0..size) |x| for (0..size) |y| {
                const xx: i32 = @intCast(x);
                const yy: i32 = @intCast(y);
                if (b.mc(xx, yy).shoot == 0 and !(xx == ch.t1 and yy == ch.t2)) continue;
                if (sp.on_ally) castOn(b, ch.sp, s, ref, .{ xx, yy });
                if (sp.on_enemy) castOn(b, ch.sp, es, ref, .{ xx, yy });
            };
        },
        0 => {
            if (sp.area == 2) {
                if (sp.on_ally) castOn(b, ch.sp, s, ref, null);
                if (sp.on_enemy) castOn(b, ch.sp, es, ref, null);
            }
            if (sp.area == 0) bs.underSpell(b, ref, ch.sp, ref);
        },
        else => {},
    }
    bs.castAI(b, ref, ch.sp, b.ai.sac_victim);
}

/// Corpse eating: the corpse leaves the field, the eater heals.
fn eat(b: *Battle, ref: Ref, corpse: Ref) void {
    const f = b.fighter(ref);
    b.fighter(corpse).in_battle = false;
    const e = b.skill(f, sk.corpse_eater);
    const amount = @min(@divTrunc(e, 2) + bt.rnd(b.r, @divTrunc(e, 2) + 1), b.maxLife(f) - f.life);
    f.life += amount;
    b.emit(.{ .text = .{ .who = ref, .phrase = bt.ph_eat, .color = bt.col_text, .sound_unit = bt.sound_eat } });
    b.emit(.{ .number = .{ .who = ref, .value = amount, .color = bt.col_eat } });
    f.speed = 0;
    f.can_act = false;
}

/// "Цель не найдена": ammo, rest, corpses, then the walk.
fn noTarget(b: *Battle, ref: Ref, enemy_army: i32) void {
    const f = b.fighter(ref);
    const s = ref.side;
    const es: u1 = 1 - s;
    if (b.skill(f, sk.gather) != 0 and f.ammo == 0) {
        f.speed = 0;
        f.can_act = false;
        const add = bt.rnd(b.r, b.skill(f, sk.gather) + 1);
        f.ammo += add;
        b.emit(.{ .text = .{ .who = ref, .phrase = bt.ph_gather, .color = bt.col_gather } });
        b.emit(.{ .number = .{ .who = ref, .value = add, .color = bt.col_ammo, .sound_unit = bt.sound_gather } });
        return;
    }
    if (b.skill(f, sk.meditation) != 0 and f.ammo == 0) return b.rest(ref);
    const pw = b.ai.power[s][ref.slot];
    const calm = pw[3] == 0 and pw[4] == 0;
    const fresh = f.speed == b.maxSpeed(f);
    if (f.stamina <= @divTrunc(b.maxStamina(f), 2) and fresh and calm) return b.rest(ref);
    if (f.life < b.maxLife(f) and fresh and b.skill(f, ab.regen_on_rest) != 0 and calm) return b.rest(ref);
    if (b.skill(f, sk.corpse_eater) != 0 and f.life <= @divTrunc(b.maxLife(f), 2)) {
        if (findCorpse(b, es, f.x, f.y) orelse findCorpse(b, s, f.x, f.y)) |c| return eat(b, ref, c);
        aiSelect(b, ref, false);
        var run: i32 = 0;
        var to: [2]i32 = .{ 0, 0 };
        var corpse: Ref = undefined;
        for (0..size) |i| for (0..size) |j| {
            const x: i32 = @intCast(i);
            const y: i32 = @intCast(j);
            const m = b.mc(x, y);
            if (!b.grid(x, y).pass or !m.pass) continue;
            if (run != 0 and m.land >= f.speed) continue;
            if (findCorpse(b, es, x, y) orelse findCorpse(b, s, x, y)) |c| {
                corpse = c;
                to = .{ x, y };
                run = if (m.land < f.speed) 2 else 1;
            }
        };
        if (run > 0) {
            b.moveTo(ref, to[0], to[1]);
            if (run > 1) {
                b.ai.index_ai = -1;
                eat(b, ref, corpse);
            }
            return;
        }
    }
    walk(b, ref, enemy_army);
}

/// The walk to the cell of a future shot, blow or heal, else towards the
/// nearest enemy.
fn walk(b: *Battle, ref: Ref, enemy_army: i32) void {
    const f = b.fighter(ref);
    const s = ref.side;
    const es: u1 = 1 - s;
    const num: i32 = ref.slot;
    b.clearMap();
    resetTryFly(b);
    if (f.speed < 1) {
        f.can_act = false;
        if (f.stamina < b.maxStamina(f) and f.speed == b.maxSpeed(f)) f.resting = true;
        return;
    }
    aiMap(b, f);
    var go: [size][size]i32 = undefined;
    for (0..size) |i| for (0..size) |j| {
        const x: i32 = @intCast(i);
        const y: i32 = @intCast(j);
        go[i][j] = if (b.mc(x, y).pass and b.grid(x, y).pass) b.mc(x, y).land else 0;
    };
    const ox = f.x;
    const oy = f.y;
    var way: i32 = 1000;
    var x1: i32 = size;
    var y1: i32 = size;
    var need_go = true;
    var army: i32 = 0;
    for (&b.units[s]) |*o| {
        if (o.life > 0 and b.attack(o) + b.rangedAttack(o) * o.ammo > b.skill(o, ab.healer) * o.ammo) army += 1;
    }
    for (0..size) |i| for (0..size) |j| {
        const g = go[i][j];
        if (g <= 0 or g >= way) continue;
        const x: i32 = @intCast(i);
        const y: i32 = @intCast(j);
        f.x = x;
        f.y = y;
        const heals = b.skill(f, ab.healer) * f.ammo;
        if (heals <= 0 or b.attack(f) + b.rangedAttack(f) * f.ammo > heals or army <= 1) {
            if (b.rangedAttack(f) * f.ammo > 0) {
                clearShootLand(b);
                b.shootMap(x, y, -1, b.shootingRange(f));
                for (&b.units[es]) |*e| {
                    if (e.life > 0 and b.mc(e.x, e.y).shoot == 2 and (enemy_army == 0 or calcRangedHit(b, f, e) > 0)) {
                        x1 = x;
                        y1 = y;
                        way = g;
                    }
                }
            }
            const nl = neighbourList(b, x, y, es);
            for (nl.refs[0..nl.n]) |nr| {
                if (enemy_army == 0 or calcMeleeHit(b, f, b.fighter(nr), true) > 0) {
                    x1 = x;
                    y1 = y;
                    way = g;
                }
            }
        } else {
            need_go = false;
            clearShootLand(b);
            b.shootMap(x, y, -1, b.shootingRange(f));
            for (&b.units[s]) |*o| {
                if (o.life > 0 and !bloodless(b, o) and o.life < b.maxLife(o) and b.mc(o.x, o.y).shoot > 0) {
                    x1 = x;
                    y1 = y;
                    way = g;
                }
            }
        }
    };
    f.x = ox;
    f.y = oy;
    if (x1 == size and need_go) {
        var dist: i32 = 50;
        var target: ?*const Fighter = null;
        for (&b.units[es]) |*e| {
            if (e.life <= 0) continue;
            const d = abs(e.x - f.x) + abs(e.y - f.y);
            if (d < dist) {
                dist = d;
                target = e;
            }
        }
        if (target) |t| for (0..size) |i| for (0..size) |j| {
            const x: i32 = @intCast(i);
            const y: i32 = @intCast(j);
            const d = abs(t.x - x) + abs(t.y - y);
            if (go[i][j] > 0 and d < dist) {
                dist = d;
                x1 = x;
                y1 = y;
            }
        };
    }
    if (x1 == size) return stuck(b, num);

    b.clearMap();
    resetTryFly(b);
    aiMap(b, f);
    const p = &b.ai.path;
    p.* = .{ .min_weight = go[@intCast(x1)][@intCast(y1)] };
    resetTryFly(b);
    const ms = b.maxSpeed(f);
    if (b.skill(f, ab.flying) != 0 or b.skill(f, ab.low_flying) != 0) {
        flyAIPath(b, f.x, f.y, -1, x1, y1, ms, f.speed);
        if (!p.found) return;
        aiSelect(b, ref, false);
        // TruePath walked GoSpeed + 1 steps, then back while out of reach.
        var gs = f.speed;
        while (gs > 0) : (gs -= 1) {
            const k = @min(@as(usize, @intCast(gs + 1)), p.n - 1);
            const c = p.best[k];
            if (b.mc(c[0], c[1]).pass and b.grid(c[0], c[1]).pass) {
                b.ai.index_ai = -1;
                b.moveTo(ref, c[0], c[1]);
                return;
            }
        }
        return stuck(b, num);
    }
    stepAIPath(b, f.x, f.y, -1, x1, y1, ms, f.speed);
    // The original dereferences a missing TruePath here.
    if (!p.found) return stuck(b, num);
    aiSelect(b, ref, false);
    var k: usize = 0;
    while (k + 1 < p.n) : (k += 1) {
        const c = p.best[k + 1];
        if (!b.mc(c[0], c[1]).pass or !b.grid(c[0], c[1]).pass) break;
    }
    const c = p.best[k];
    if (c[0] != f.x or c[1] != f.y) {
        b.ai.index_ai = -1;
        b.moveTo(ref, c[0], c[1]);
        return;
    }
    stuck(b, num);
}

// -- AI_Simulate (CompareStrength1's battle, battle_sim.zig) -------------
//
// The simulated battle looks at the columns only: a unit reaches what is
// within its move points plus the range, moves along x and nothing else
// blocks it.  The blows do not roll (Battle.simulate).

/// AI_SimulatePower: the power marks by the x distance.
pub fn simulatePower(b: *Battle) void {
    b.ai.power = @splat(@splat(@splat(0)));
    for (0..2) |sd| {
        const s: u1 = @intCast(sd);
        for (0..n_units) |u| {
            const f = &b.units[s][u];
            if (!f.present() or f.life <= 0) continue;
            const p = &b.ai.power[s][u];
            if (b.skill(f, ab.healer) != 0) p[0] = if (f.ammo > 0) 2 else 1;
            if (b.rangedAttack(f) > 0) for (0..n_units) |i| {
                const e = &b.units[1 - s][i];
                if (!e.present() or e.life <= 0 or abs(e.x - f.x) >= b.shootingRange(f) + f.speed) continue;
                if (f.ammo > 0) {
                    p[1] = 2;
                    b.ai.power[1 - s][i][if (b.skill(f, ab.magic_shot) != 0) 3 else 4] = 1;
                } else p[1] = 1;
            };
            for (0..n_units) |i| {
                const e = &b.units[1 - s][i];
                if (!e.present() or e.life == 0 or abs(e.x - f.x) > f.speed + 1) continue;
                p[2] = 2;
                b.ai.power[1 - s][i][if (b.skill(f, ab.magic_strike) != 0) 5 else 6] = 1;
            }
        }
    }
}

/// AI_SimulateDangerZone: 0 up to the farthest enemy column, then the
/// columns beyond it.
pub fn simulateDanger(b: *Battle) void {
    var max: i32 = 0;
    for (&b.units[1 - b.side]) |*e| {
        if (e.present() and e.life > 0 and e.x > max) max = e.x;
    }
    for (0..size) |i| for (0..size) |j| {
        const x: i32 = @intCast(i);
        b.ai.danger[i][j] = if (x <= max) 0 else x - max;
    };
}

/// GetTeleportTaste(Unit, x): the best blow from column x.
fn teleportTasteX(b: *Battle, u: *const Fighter, x: i32) i32 {
    var r: i32 = 0;
    for (&b.units[1 - b.side]) |*e| {
        if (!e.present() or e.life <= 0 or abs(e.x - x) >= 2) continue;
        var hit = aiMeleeHit(b, u, e);
        if (e.life <= hit) hit = 5 + e.life * 2;
        if (e.ammo < 2) hit = @divTrunc(hit, 2);
        r = @max(r, hit);
    }
    return r;
}

/// The column a unit goes to so that `to` is within `reach`; `moved` when
/// it had to move.
fn approach(from: i32, to: i32, reach: i32) struct { x: i32, moved: bool } {
    if (abs(to - from) <= reach) return .{ .x = from, .moved = false };
    const add: i32 = if (to < from) -1 else 1;
    var x = from;
    while (abs(to - x) > reach) x += add;
    return .{ .x = x, .moved = true };
}

/// The unit stands on column x of its row at once.
fn moveX(b: *Battle, f: *Fighter, x: i32) void {
    const nx = std.math.clamp(x, 0, size - 1);
    if (nx == f.x) return;
    b.setPass(f, true);
    f.x = nx;
    b.setPass(f, false);
}

fn cellIndex(v: i32) i32 {
    return std.math.clamp(v, 0, size - 1);
}

/// The magic part of AI_Simulate.
fn simSpell(b: *Battle, ref: Ref) ?Choice {
    const a = b.fighter(ref);
    const s = ref.side;
    const es: u1 = 1 - s;
    var cast: [bt.spellbook_size]i32 = @splat(0);
    var count: usize = 0;
    if (a.hero_unit) {
        for (b.book[s], 0..) |slot, i| {
            if (slot[0] > 0 and slot[1] > 0 and b.gem[s] >= bs.price(b, a, slot[0])) {
                cast[i] = slot[0];
                count += 1;
            }
        }
    } else {
        for (1..b.pr.spells.len) |i| {
            const k = b.skill(a, 2000 + @as(i32, @intCast(i)));
            if (k != 0 and a.ammo >= k and count < cast.len) {
                cast[count] = @intCast(i);
                count += 1;
            }
        }
    }
    if (count == 0) return null;
    var best: Best = .{};
    for (cast) |sp_i| {
        if (sp_i <= 0) continue;
        const sp = bs.spellOf(b, sp_i);
        const e0 = sp.effects[0];
        if (sp.target == 1) {
            if (sp.on_enemy) for (0..n_units) |u| {
                const t = &b.units[es][u];
                if (!t.present() or t.life <= 0) continue;
                const tt = enemyTaste(b, a, sp_i, .{ .side = es, .slot = @intCast(u) });
                best.offer(tt.taste + spellPenalty(b, a, s, sp_i), tt.gain, .{ .sp = sp_i, .t1 = es, .t2 = @intCast(u) });
            };
            if (sp.on_ally) for (0..n_units) |u| {
                const t = &b.units[s][u];
                if (!t.present() or t.life <= 0) continue;
                const tt = allyTaste(b, a, sp_i, .{ .side = s, .slot = @intCast(u) });
                best.offer(tt.taste + spellPenalty(b, a, s, sp_i), tt.gain, .{ .sp = sp_i, .t1 = s, .t2 = @intCast(u) });
            };
        }
        if (sp.target == 0 and sp.area == 2) {
            var taste: i32 = 0;
            var dmg: i32 = 0;
            for ([2]u1{ s, es }) |ts| {
                if (!(if (ts == s) sp.on_ally else sp.on_enemy)) continue;
                for (0..n_units) |u| {
                    const t = &b.units[ts][u];
                    if (!t.present() or t.life <= 0) continue;
                    const tr: Ref = .{ .side = ts, .slot = @intCast(u) };
                    const tt = if (ts == s) allyTaste(b, a, sp_i, tr) else enemyTaste(b, a, sp_i, tr);
                    taste += tt.taste;
                    dmg += tt.gain;
                }
            }
            best.offer(taste + spellPenalty(b, a, s, sp_i), dmg, .{ .sp = sp_i });
        }
        if (sp.target == 0 and sp.area == 0) {
            const tt = allyTaste(b, a, sp_i, ref);
            best.offer(tt.taste + spellPenalty(b, a, s, sp_i), tt.gain, .{ .sp = sp_i });
        }
        if (sp.target == 2 or sp.target == 4) {
            for ([2]u1{ es, s }) |cs| {
                const ally = cs == s;
                if (!(if (ally) sp.on_ally else sp.on_enemy)) continue;
                for (0..n_units) |u| {
                    const c = &b.units[cs][u];
                    if (!c.present() or c.life != 0 or !c.in_battle) continue;
                    if (sp.target == 4 and !b.grid(c.x, c.y).pass) continue;
                    var buff: i32 = 0;
                    var dmg: i32 = 0;
                    if (e0.id > 1000) {
                        const power = e0.power + b.heroBonus(a, 907) + b.heroBonus(a, 909);
                        // Both loops read the danger at the enemy of the
                        // same slot (sic).
                        const z = &b.units[es][u];
                        const dz = danger(b, cellIndex(z.x), cellIndex(z.y));
                        const v = summonTaste(b, e0.id - 1000, power, dz, @intFromBool(ally), 1, e0.id - 1000, 1);
                        buff = v[0];
                        dmg = v[1];
                    }
                    if (ally and e0.id == 1) {
                        buff = b.utype(c).level * 15 + c.unit.level * 2;
                        dmg = b.utype(c).attack;
                    }
                    const taste = buff + dmg * 3 + spellPenalty(b, a, s, sp_i);
                    if (taste > best.taste and (dmg > 0 or buff > 0)) {
                        best.taste = taste;
                        best.hit = dmg;
                        best.choice = .{ .sp = sp_i, .t1 = cs, .t2 = @intCast(u) };
                    }
                }
            }
        }
        if (sp.target == 5 and e0.id > 1000) {
            b.clearMap();
            b.shootMap(a.x, a.y, -1, @divTrunc(b.skill(a, 907), 2) + 1);
            for (0..size) |i| for (0..size) |j| {
                const x: i32 = @intCast(i);
                const y: i32 = @intCast(j);
                if (b.mc(x, y).shoot == 0 or !b.grid(x, y).pass or b.bfo(b.grid(x, y).land).move_cost <= 0) continue;
                const power = e0.power + b.heroBonus(a, 907);
                const v = summonTaste(b, e0.id - 1000, power, danger(b, x, y), 0, 2, e0.id - 1001, 0);
                const taste = v[0] + v[1] * 3 + spellPenalty(b, a, s, sp_i);
                if (taste > best.taste and (v[1] > 0 or v[0] > 0)) {
                    best.taste = taste;
                    best.hit = v[1];
                    best.choice = .{ .sp = sp_i, .t1 = x, .t2 = y };
                }
            };
        }
        if (sp.target == 3) {
            const ts: u1 = if (sp.negative) es else s;
            var targets: i32 = 0;
            for (&b.units[ts]) |*t| {
                if (t.present() and t.life > 0) targets += 1;
            }
            const max_t = areaTargets(sp.radius, targets);
            for (0..n_units) |k| {
                const t = &b.units[ts][k];
                if (!t.present() or t.life <= 0) continue;
                var taste: i32 = 0;
                var dmg: i32 = 0;
                var n: i32 = 0;
                for (0..n_units + 1) |step_i| {
                    // The target first, then the others in reach.
                    const i = if (step_i == 0) k else step_i - 1;
                    if (step_i > 0 and (i == k or n >= max_t)) continue;
                    const o = &b.units[ts][i];
                    if (!o.present() or o.life <= 0) continue;
                    if (step_i > 0 and abs(o.x - t.x) > sp.radius) continue;
                    const tr: Ref = .{ .side = ts, .slot = @intCast(i) };
                    const tt = if (sp.negative) enemyTaste(b, a, sp_i, tr) else allyTaste(b, a, sp_i, tr);
                    taste += tt.taste;
                    dmg += tt.gain;
                    n += 1;
                }
                best.offer(taste + spellPenalty(b, a, s, sp_i), dmg, .{ .sp = sp_i, .t1 = ts, .t2 = @intCast(k) });
            }
        }
    }
    var c = best.choice orelse return null;
    c.hit = if (best.hit == 0) @divTrunc(best.taste, 10) else best.hit;
    return c;
}

/// MaxTargets of an area spell in the simulation.
fn areaTargets(radius: i32, targets: i32) i32 {
    return switch (radius) {
        1 => @divTrunc(targets + 2, 3),
        2 => @divTrunc(targets + 1, 2),
        else => 1,
    };
}

/// The cast of AI_Simulate.
fn simCast(b: *Battle, ref: Ref, ch: Choice) void {
    const sp = bs.spellOf(b, ch.sp);
    const s = ref.side;
    const es: u1 = 1 - s;
    switch (sp.target) {
        1 => {
            const tr: Ref = .{ .side = @intCast(ch.t1), .slot = @intCast(ch.t2) };
            if (sp.effects[0].id == 89) {
                var big: i32 = 0;
                var to: ?i32 = null;
                for (0..size) |x| {
                    const h = teleportTasteX(b, b.fighter(tr), @intCast(x));
                    if (h > big) {
                        big = h;
                        to = @intCast(x);
                    }
                }
                if (to) |x| moveX(b, b.fighter(tr), x);
            } else bs.underSpell(b, tr, ch.sp, ref);
        },
        2, 4 => bs.corpseSpell(b, ch.sp, .{ .side = @intCast(ch.t1), .slot = @intCast(ch.t2) }, ref),
        5 => bs.tileSpell(b, ch.sp, ch.t1, ch.t2, ref),
        3 => {
            const ts: u1 = @intCast(ch.t1);
            const k: usize = @intCast(ch.t2);
            var targets: i32 = 0;
            for (&b.units[ts]) |*t| {
                if (t.present() and t.life > 0) targets += 1;
            }
            const max_t = areaTargets(sp.radius, targets);
            const cx = b.units[ts][k].x;
            bs.underSpell(b, .{ .side = ts, .slot = @intCast(k) }, ch.sp, ref);
            var n: i32 = 1;
            for (0..n_units) |i| {
                if (i == k or n >= max_t) continue;
                const t = &b.units[ts][i];
                if (!t.present() or t.life <= 0 or abs(t.x - cx) > sp.radius) continue;
                bs.underSpell(b, .{ .side = ts, .slot = @intCast(i) }, ch.sp, ref);
                n += 1;
            }
        },
        0 => {
            if (sp.area == 2) {
                if (sp.on_ally) castOn(b, ch.sp, s, ref, null);
                if (sp.on_enemy) castOn(b, ch.sp, es, ref, null);
            }
            if (sp.area == 0) bs.underSpell(b, ref, ch.sp, ref);
        },
        else => {},
    }
    // iAISacVictim: the living unit worth least.
    var victim: ?u8 = null;
    if (sp.sacrifice) {
        var v2: i32 = 0;
        for (b.units[s][0..bt.max_squad], 0..) |*o, i| {
            if (!o.present() or o.life <= 0) continue;
            const kind = b.unitKind(o);
            if (kind >= 2 and kind <= 4) continue;
            var v = o.life * b.utype(o).level + o.unit.level * 2;
            if (hasHero(o)) v += 30;
            if (victim == null or v < v2) {
                v2 = v;
                victim = @intCast(i);
            }
        }
    }
    bs.castAI(b, ref, ch.sp, victim);
}

/// AI_Simulate: one order of the unit `ref` in the simulated battle.
pub fn simulateUnit(b: *Battle, ref: Ref) void {
    const a = b.fighter(ref);
    if (!a.can_act) return;
    b.active = ref;
    const s = ref.side;
    const es: u1 = 1 - s;
    const num = ref.slot;
    const spell = simSpell(b, ref);

    // The reachable targets.
    var can_heal: [n_units]i32 = @splat(0);
    var heal_x: [n_units]i32 = @splat(0);
    var can_range: [n_units]i32 = @splat(0);
    var range_x: [n_units]i32 = @splat(0);
    var can_melee: [n_units]i32 = @splat(0);
    var melee_x: [n_units]i32 = @splat(0);
    const range = b.shootingRange(a);
    if (b.skill(a, ab.healer) != 0 and a.ammo > 0) for (0..n_units) |i| {
        const o = &b.units[s][i];
        if (o.life <= 0 or abs(o.x - a.x) >= range + a.speed or o.life >= b.maxLife(o) or bloodless(b, o)) continue;
        const ap = approach(a.x, o.x, range);
        can_heal[i] = if (ap.moved) 1 else 3;
        heal_x[i] = ap.x;
    };
    if (a.ammo > 0 and b.rangedAttack(a) > 0) for (0..n_units) |i| {
        const e = &b.units[es][i];
        if (!e.present() or e.life <= 0 or abs(e.x - a.x) >= range + a.speed) continue;
        const ap = approach(a.x, e.x, range);
        can_range[i] = if (ap.moved) 1 else 3;
        range_x[i] = ap.x;
    };
    for (0..n_units) |i| {
        const e = &b.units[es][i];
        if (!e.in_battle or !e.present() or e.life <= 0 or abs(e.x - a.x) > a.speed + 1) continue;
        const ap = approach(a.x, e.x, 1);
        can_melee[i] = if (ap.moved) 1 else 3;
        melee_x[i] = ap.x;
    }

    var will_heal: ?usize = null;
    var heal_hit: i32 = 0;
    var will_shoot: ?usize = null;
    var shoot_hit: i32 = 0;
    var will_strike: ?usize = null;
    var melee_hit: i32 = 0;
    var kill_shoot = false;
    var kill_melee = false;

    // The heal.
    var taste: i32 = -1000;
    if (b.skill(a, ab.healer) != 0 and a.ammo > 0) for (0..n_units) |i| {
        if (can_heal[i] == 0) continue;
        const o = &b.units[s][i];
        const hit = @min(b.skill(a, ab.healer), b.maxLife(o) - o.life);
        var t1 = hit * 5 + b.defence(o) + b.rangedDefence(o) + b.resist(o) + (can_heal[i] - 1) * 2;
        t1 += hit * @divTrunc(heroClass(b, o), 2);
        if (can_heal[i] == 1) t1 = @divTrunc(t1, 2);
        if (t1 > taste) {
            taste = t1;
            will_heal = i;
            heal_hit = hit;
        }
    };

    // The shot.
    if (b.rangedAttack(a) > 0 and a.ammo > 0) {
        taste = -1000;
        for (0..n_units) |i| {
            if (can_range[i] == 0) continue;
            const e = &b.units[es][i];
            var my_taste: i32 = 0;
            var my_hit: i32 = 0;
            if (b.skill(a, sk.web) > 0) {
                my_taste = e.life;
                my_hit = @divTrunc(e.life, 4);
                if (e.hero_unit) {
                    my_taste *= 2;
                    my_hit *= 2;
                }
                if (b.skill(a, sk.web) > b.attack(e)) my_taste *= 2;
                if (b.skill(e, ab.immobile) > 0) {
                    my_taste = @divTrunc(my_taste, 3);
                    my_hit = @divTrunc(my_hit, 3);
                }
                if (b.skill(e, sk.web_immune) > 0) {
                    my_taste = -1000;
                    my_hit = 0;
                }
            } else if (b.skill(a, sk.stoning) > 0 and b.skill(e, ab.immobile) == 0) {
                const st = b.skill(a, sk.stoning) - b.resist(e) - 1;
                my_taste = st * (e.life + b.attack(e));
                my_hit = st * (@divTrunc(e.life, 4) + b.attack(e));
                if (e.hero_unit) {
                    my_taste *= 2;
                    my_hit *= 2;
                }
                if (b.skill(a, sk.stoning) <= b.resist(e) or b.skill(e, sk.stone_immune) > 0) {
                    my_taste = -2000;
                    my_hit = -3;
                }
            } else {
                my_hit = b.rangedHit(a, e);
                const ds = b.skill(a, sk.double_shot);
                if (my_hit < e.life and can_range[i] == 3 and ds > 0 and a.stamina >= ds) {
                    my_hit *= 2;
                    can_range[i] = 4;
                }
                my_taste = my_hit * 2 - abs(my_hit - e.life - 1);
                if (my_hit < e.life) my_taste += b.skill(a, ab.poison_shot) * 2;
                var adder = @divTrunc(b.attack(e) + b.counterAttack(e), 5);
                adder += @divTrunc(b.rangedAttack(e) * e.ammo, 5);
                adder -= @as(i32, @intFromBool(e.summoned)) * 3;
                adder += my_hit * @divTrunc(heroClass(b, e), 2);
                if (my_hit >= @divTrunc(shoot_hit, 2) or my_hit >= e.life) my_taste += adder;
                if (adder > 0 and my_hit >= e.life) my_taste += adder;
                if (can_range[i] == 1) my_taste = @divTrunc(my_taste, 2);
                if (my_taste > 0 and b.skill(e, sk.charmed) != 0) my_taste = @divTrunc(my_taste, 10);
                if (my_taste > 0 and b.skill(e, sk.shunned) != 0) my_taste = @divTrunc(my_taste, 2);
            }
            if (my_hit > 0 and my_taste > taste) {
                taste = my_taste;
                will_shoot = i;
                shoot_hit = my_hit;
                kill_shoot = my_hit >= e.life;
            }
        }
    }

    // The blow.
    if (b.skill(a, ab.no_combat) == 0) {
        taste = -1000;
        for (0..n_units) |i| {
            if (can_melee[i] == 0) continue;
            const e = &b.units[es][i];
            var hit = aiMeleeHit(b, a, e);
            const cr = b.skill(a, sk.crush);
            if (hit < e.life and cr > 0 and a.stamina > cr and can_melee[i] > 1) {
                b.crush = true;
                hit = aiMeleeHit(b, a, e);
                b.crush = false;
                can_melee[i] = 5;
            }
            var my_taste = hit * 2 - abs(hit - e.life - 1);
            if (hit < e.life) {
                my_taste += b.skill(a, ab.poison) * 3;
                // Not at an enemy turned into a dragon.
                if (e.unit.type == 60 and e.true_form != null and hit < 20) my_taste = -2000;
            }
            if (hit < e.life and b.skill(a, sk.trample) >= e.life - hit) {
                hit = e.life;
                my_taste = hit * 2;
            }
            const r66 = b.skill(a, sk.round_button);
            if (b.skill(a, sk.round_attack) > 0 or (r66 > 0 and a.stamina > r66)) {
                const nl = neighbourList(b, melee_x[i], a.y, es);
                if (nl.n > 1) {
                    if (r66 > 0 and a.stamina > r66) can_melee[i] += 100;
                    for (nl.refs[0..nl.n]) |nr| {
                        if (nr.slot == i) continue;
                        const h2 = aiMeleeHit(b, a, b.fighter(nr));
                        hit += h2;
                        my_taste += h2 * 2;
                    }
                }
            }
            var adder = b.skill(e, ab.immobile) * 5 + @divTrunc(b.attack(e), 2) - @as(i32, @intFromBool(e.summoned)) * 3;
            if (hit <= e.life and b.skill(a, ab.agility) == 0 and e.stamina > 0 and !e.resting) adder -= @divTrunc(b.counterAttack(e), 2);
            adder += @divTrunc(b.rangedAttack(e) * e.ammo, 5);
            adder += hit * @divTrunc(heroClass(b, e), 2);
            if (hit >= @divTrunc(melee_hit, 2) or hit >= e.life) my_taste += adder;
            if (adder > 0 and hit >= e.life) my_taste += adder;
            if (my_taste > 0 and b.skill(e, sk.charmed) != 0) my_taste = @divTrunc(my_taste, 10);
            if (my_taste > 0 and b.skill(e, sk.shunned) != 0) my_taste = @divTrunc(my_taste, 2);
            if (can_melee[i] == 1 and my_taste > 0) my_taste = @divTrunc(my_taste, 2);
            if (hit > 0 and my_taste > taste) {
                taste = my_taste;
                will_strike = i;
                melee_hit = hit;
                kill_melee = hit >= e.life;
            }
        }
    }

    // The order.
    if (spell) |ch| if (ch.hit >= shoot_hit and ch.hit >= melee_hit and ch.hit >= heal_hit) {
        b.ai.index_ai = -1;
        simCast(b, ref, ch);
        return;
    };
    if (will_heal) |i| if (heal_hit >= shoot_hit and heal_hit >= melee_hit) {
        moveX(b, a, heal_x[i]);
        b.heal(ref, .{ .side = s, .slot = @intCast(i) });
        return;
    };
    if (will_shoot) |i| if ((shoot_hit >= melee_hit or kill_shoot) and (!kill_melee or kill_shoot)) {
        b.double_shot = can_range[i] == 4;
        moveX(b, a, range_x[i]);
        b.shoot(ref, .{ .side = es, .slot = @intCast(i) });
        b.double_shot = false;
        return;
    };
    if (will_strike) |i| {
        b.crush = @mod(can_melee[i], 100) == 5;
        b.round_attack = can_melee[i] > 99;
        moveX(b, a, melee_x[i]);
        _ = b.attackFrom(ref, a.x, a.y, .{ .side = es, .slot = @intCast(i) });
        b.crush = false;
        b.round_attack = false;
        return;
    }

    // No target.
    const p = b.ai.power[s][num];
    const shot_at = p[3] != 0 or p[4] != 0;
    if (b.skill(a, sk.gather) != 0 and a.ammo == 0) {
        a.speed = 0;
        a.can_act = false;
        a.ammo += b.skill(a, sk.gather) - 1;
        return;
    }
    if (b.skill(a, sk.meditation) != 0 and a.ammo == 0) return b.rest(ref);
    if (a.stamina <= @divTrunc(b.maxStamina(a), 2) and a.speed == b.maxSpeed(a) and !shot_at) return b.rest(ref);
    if (a.life < b.maxLife(a) and a.speed == b.maxSpeed(a) and b.skill(a, ab.regen_on_rest) != 0 and !shot_at) return b.rest(ref);
    if (b.skill(a, sk.corpse_eater) != 0 and a.life <= @divTrunc(b.maxLife(a), 2)) {
        var corpse: ?Ref = null;
        for ([2]u1{ es, s }) |cs| {
            var yy: i32 = 0;
            while (corpse == null and yy < size) : (yy += 1) corpse = findCorpse(b, cs, a.x, yy);
            if (corpse != null) break;
        }
        if (corpse) |cr| {
            b.fighter(cr).in_battle = false;
            const e = b.skill(a, sk.corpse_eater);
            a.life += @min(@divTrunc(e, 2) + @divTrunc(e, 4), b.maxLife(a) - a.life);
            a.speed = 0;
            a.can_act = false;
            return;
        }
    }
    // Towards the nearest enemy column.
    var add: i32 = 0;
    var dist: i32 = 10;
    for (&b.units[es]) |*e| {
        if (!e.present() or e.life <= 0 or abs(e.x - a.x) >= dist) continue;
        dist = abs(e.x - a.x);
        add = if (e.x > a.x) a.speed else -a.speed;
    }
    if (add != 0) moveX(b, a, if (abs(add) > dist) a.x + dist * std.math.sign(add) else a.x + add);
}
