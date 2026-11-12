//! Rituals of a player: the pRitual / pRitualCD stocks (in the saved lists
//! after pDefender and pOuterBld), GetRitualGoldPrice / GetRitualGemPrice
//! (Player.cpp:590, :611), dcProvince::CanCast (Province.cpp:912) and the
//! immediate effects of dcPlayer::CastRitual (Player.cpp:1044).
//!
//! Not ported: the timed rituals (dcRitual, iTime / ipEvent: the barrier,
//! the attitude and income bonuses), effect 5 (an encounter), and the
//! campaign.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const mapgen = @import("mapgen.zig");
const Shard = mapgen.Shard;
const province = @import("province.zig");
const economy = @import("economy.zig");
const player = @import("player.zig");
const Market = player.Market;
const movement = @import("movement.zig");
const event_mod = @import("event.zig");
const rand_mod = @import("../data/rand.zig");
const Rand = rand_mod.Rand;

/// Castle abilities: rituals cost less (7), cooldown is shorter (53), the
/// hero may cast (54), buildings grant rituals (27).
const abil_cost = 7;
const abil_cooldown = 53;
const abil_cast = 54;
/// Ritual abilities (clRitual::GetAbility).
pub const abil_hero = 20;
const abil_own_capital = 18;
const abil_besieged = 19;
const abil_outer_place = 22;
const abil_no_building = 23;
const abil_population = 24;
const abil_no_defender = 25;
const abil_barrier = 12;

/// One saved list entry: pRitual then pRitualCD.
pub const entry_size = 8;

fn listOf(s: *Shard, pr: *const Proto, k: usize) ![]u8 {
    if (s.saved) |sv| {
        if (k < sv.lists.len and sv.lists[k].len > 0) return @constCast(sv.lists[k]);
    }
    if (s.ritual_lists[k]) |l| return l;
    const a = s.arena.allocator();
    const d = pr.defenders.len;
    const o = pr.outer_buildings.len;
    const r = pr.rituals.len;
    const b = try a.alloc(u8, 4 + d * 4 + 4 + o * 4 + 4 + r * entry_size);
    @memset(b, 0);
    var p: usize = 0;
    for ([_]struct { n: usize, size: usize }{
        .{ .n = d, .size = 4 },
        .{ .n = o, .size = 4 },
        .{ .n = r, .size = entry_size },
    }) |l| {
        std.mem.writeInt(i32, b[p..][0..4], @intCast(l.n - 1), .little);
        p += 4 + l.n * l.size;
    }
    s.ritual_lists[k] = b;
    return b;
}

/// The saved lists of a player, in file order: pDefender, pOuterBld,
/// pRitual (with pRitualCD).
pub const List = enum { defender, outer, ritual };

fn listOffset(s: *Shard, pr: *const Proto, k: usize, which: List, idx: i32) !usize {
    const b = try listOf(s, pr, k);
    var p: usize = 0;
    for ([_]struct { l: List, size: usize }{
        .{ .l = .defender, .size = 4 },
        .{ .l = .outer, .size = 4 },
        .{ .l = .ritual, .size = entry_size },
    }) |l| {
        if (p + 4 > b.len) return error.BadRitualList;
        const n: i32 = std.mem.readInt(i32, b[p..][0..4], .little);
        if (l.l == which) {
            if (idx < 0 or idx > n) return error.BadRitual;
            const at = p + 4 + @as(usize, @intCast(idx)) * l.size;
            if (at + l.size > b.len) return error.BadRitualList;
            return at;
        }
        if (n < 0) return error.BadRitualList;
        p += 4 + @as(usize, @intCast(n + 1)) * l.size;
    }
    return error.BadRitualList;
}

fn ritualOffset(s: *Shard, pr: *const Proto, k: usize, idx: i32) !usize {
    return listOffset(s, pr, k, .ritual, idx);
}

/// pDefender[idx], pOuterBld[idx] or pRitual[idx] of player `k`: 0 not
/// available, -1 unlimited, > 0 the copies left.
pub fn listStock(s: *Shard, pr: *const Proto, k: usize, which: List, idx: i32) i32 {
    const at = listOffset(s, pr, k, which, idx) catch return 0;
    const b = listOf(s, pr, k) catch return 0;
    return std.mem.readInt(i32, b[at..][0..4], .little);
}

pub fn setListStock(s: *Shard, pr: *const Proto, k: usize, which: List, idx: i32, v: i32) void {
    const at = listOffset(s, pr, k, which, idx) catch return;
    const b = listOf(s, pr, k) catch return;
    std.mem.writeInt(i32, b[at..][0..4], v, .little);
}

/// Inner building abilities 11, 15 and 27 (Player.cpp:948): an outer
/// building, a defender or a ritual becomes available (p2 0: without a
/// limit, else p2 more copies).
pub fn grantList(s: *Shard, pr: *const Proto, k: usize, which: List, a: proto_mod.Ability3) void {
    const cur = listStock(s, pr, k, which, a.p1);
    if (a.p2 == 0) setListStock(s, pr, k, which, a.p1, -1) else if (cur >= 0) setListStock(s, pr, k, which, a.p1, cur + a.p2);
}

pub fn stock(s: *Shard, pr: *const Proto, k: usize, idx: i32) i32 {
    const at = ritualOffset(s, pr, k, idx) catch return 0;
    const b = listOf(s, pr, k) catch return 0;
    return std.mem.readInt(i32, b[at..][0..4], .little);
}

pub fn cooldown(s: *Shard, pr: *const Proto, k: usize, idx: i32) i32 {
    const at = ritualOffset(s, pr, k, idx) catch return 0;
    const b = listOf(s, pr, k) catch return 0;
    return std.mem.readInt(i32, b[at + 4 ..][0..4], .little);
}

pub fn setStock(s: *Shard, pr: *const Proto, k: usize, idx: i32, v: i32) void {
    const at = ritualOffset(s, pr, k, idx) catch return;
    const b = listOf(s, pr, k) catch return;
    std.mem.writeInt(i32, b[at..][0..4], v, .little);
}

pub fn setCooldown(s: *Shard, pr: *const Proto, k: usize, idx: i32, v: i32) void {
    const at = ritualOffset(s, pr, k, idx) catch return;
    const b = listOf(s, pr, k) catch return;
    std.mem.writeInt(i32, b[at + 4 ..][0..4], v, .little);
}

/// Inner building ability 27: grants a ritual (p2 0: a single copy).
pub fn grant(s: *Shard, pr: *const Proto, k: usize, a: proto_mod.Ability3) void {
    grantList(s, pr, k, .ritual, a);
}

/// Count of the ritual's effects with id `index` (GetAbility).
pub fn ability(pr: *const Proto, idx: i32, index: i32) i32 {
    if (idx <= 0 or idx >= pr.rituals.len) return 0;
    var r: i32 = 0;
    for (pr.rituals[@intCast(idx)].effects) |e| {
        if (e.id == index) r += e.p1;
    }
    return r;
}

/// GetRitualGoldPrice / GetRitualGemPrice.
pub fn price(m: Market, idx: i32) player.Price {
    if (idx <= 0 or idx >= m.pr.rituals.len) return .{ 0, 0 };
    const r = &m.pr.rituals[@intCast(idx)];
    const cut = m.bldParam1(abil_cost);
    var out: player.Price = .{ r.gold_cost, r.gem_cost };
    for (&out) |*v| v.* -= @divTrunc(v.* * cut, 100);
    for (r.res) |res| {
        if (res <= 0) continue;
        const add = m.resPrice(res);
        out[0] += add[0];
        out[1] += add[1];
    }
    return out;
}

/// CanCast (Province.cpp:912) for ritual `idx` of player `pl` in `prov`.
pub fn canCast(s: *Shard, pr: *const Proto, prov: i32, idx: i32, pl: i32) bool {
    if (idx <= 0 or idx >= pr.rituals.len or prov <= 0 or prov >= s.provinces.len) return false;
    const r = &pr.rituals[@intCast(idx)];
    const pv = &s.provinces[@intCast(prov)];
    const own = if (pl > 0 and pl < s.capitals.len) s.capitals[@intCast(pl)] else 0;
    if (!r.on_enemy and pl != pv.player) return false;
    if (!r.on_ally and pl == pv.player and movement.siege(s, prov) < 0) return false;
    if (ability(pr, idx, abil_own_capital) != 0 and own == prov) return false;
    if (ability(pr, idx, abil_besieged) != 0 and movement.siege(s, prov) >= 0) return false;
    if (ability(pr, idx, abil_hero) != 0) {
        if (!heroIn(s, pv, if (r.on_ally) pl else pv.player)) return false;
    }
    if (ability(pr, idx, abil_outer_place) != 0) {
        if (own == prov) return false;
        var n: usize = 0;
        for (pv.buildings[0..province.max_bld]) |b| n += @intFromBool(b > 0);
        if (n == province.max_bld) return false;
    }
    const bld = ability(pr, idx, abil_no_building);
    if (bld > 0) {
        for (pv.buildings[0..province.max_bld]) |b| {
            if (b == bld) return false;
        }
    }
    if (ability(pr, idx, abil_population) != 0 and pv.population <= 0) return false;
    if (ability(pr, idx, abil_no_defender) != 0 and pv.defender > 0 and pv.defender < pr.defenders.len and pr.defenders[@intCast(pv.defender)].no_dismiss) return false;
    return true;
}

fn heroIn(s: *Shard, pv: *const province.Province, pl: i32) bool {
    for (s.heroes.items) |*h| {
        if (h.owner == pl and h.alive() and h.province == pv.number) return true;
    }
    return false;
}

/// The rituals of the hero ritual list: a stock and ability 20.
pub fn heroRituals(s: *Shard, pr: *const Proto, k: usize, out: []i32) []i32 {
    var n: usize = 0;
    for (1..pr.rituals.len) |i| {
        if (n == out.len) break;
        const idx: i32 = @intCast(i);
        if (stock(s, pr, k, idx) == 0) continue;
        if (ability(pr, idx, abil_hero) <= 0) continue;
        out[n] = idx;
        n += 1;
    }
    return out[0..n];
}

/// What a cast left for the map to show.
pub const Cast = struct {
    /// Effect 9: the dialog and the value of its %d.
    dialog: ?usize = null,
    dialog_value: i32 = 0,
    /// Effect 3: a province event of the player.
    event: ?i32 = null,
    /// Effect 4: the extra gold or gems.
    gold: i32 = 0,
    gem: i32 = 0,
};

/// dcPlayer::CastRitual: the price, the stock, the cooldown, the karma and
/// the immediate effects (effects 3, 4, 7, 9, 11, 13, 14).
pub fn cast(m: Market, idx: i32, prov: i32, hero: ?usize, r: *Rand) !Cast {
    const s = m.s;
    const pr = m.pr;
    if (idx <= 0 or idx >= pr.rituals.len) return error.BadRitual;
    const rit = &pr.rituals[@intCast(idx)];
    const pl: usize = @intCast(std.math.clamp(m.player, 0, mapgen.max_players));
    var out: Cast = .{};
    // pRitual / pRitualCD.
    if (stock(s, pr, pl, idx) > 0) setStock(s, pr, pl, idx, stock(s, pr, pl, idx) - 1);
    const cd = rit.cooldown - m.bldParam1(abil_cooldown);
    if (cd > 0) setCooldown(s, pr, pl, idx, cd);
    const cost = price(m, idx);
    m.pay(cost);
    m.ruler().karma += rit.karma;
    m.holdings().can_cast -= 1;
    m.raiseResPrices(&rit.res);
    if (rit.time > 0 or rit.event > 0) {
        std.log.info("ritual {d}: the timed effect is not ported", .{idx});
    }
    _ = hero;
    const pv = &s.provinces[@intCast(prov)];
    const own = std.math.clamp(pv.player, 0, mapgen.max_players);
    var value: i32 = 0;
    for (rit.effects) |e| switch (e.id) {
        3 => out.event = e.p1,
        4 => {
            if (pv.population > 0) {
                const dead = @divTrunc(pv.population * e.p1, 100);
                pv.population -= dead;
                if (e.p2 == 1) {
                    value = 20 + dead * 2;
                    m.ruler().gold += value;
                    out.gold += value;
                }
                if (e.p2 == 2) {
                    value = 10 + @divTrunc(dead, 3);
                    m.ruler().gem += value;
                    out.gem += value;
                }
            }
        },
        7 => {
            var morale = e.p1;
            if (e.p2 != 0) {
                if (pv.player == m.player) morale += e.p2 else morale -= e.p2;
            }
            if (pv.guard >= 0) event_mod.changeMorale(pr, &s.guards.items[@intCast(pv.guard)].squad, morale, 0);
            if (pv.garrison >= 0) event_mod.changeMorale(pr, &s.garrisons.items[@intCast(pv.garrison)].squad, morale, 0);
            for (s.heroes.items) |*h| {
                if (h.owner < 1 or !h.alive() or h.province != prov) continue;
                event_mod.changeMorale(pr, &h.squad, morale, 0);
            }
        },
        9 => {
            out.dialog = @intCast(@max(e.p1, 0));
            out.dialog_value = if (e.p2 == 1) value else 0;
        },
        11 => {
            if (pv.fort > 0) pv.fort = @max(pv.fort - e.p1, 0);
            var slot: ?usize = null;
            for (pv.buildings[0..province.max_bld], 0..) |b, j| {
                if (b <= 0 or b >= pr.outer_buildings.len) continue;
                if (pr.outer_buildings[@intCast(b)].wall != 0) continue;
                if (slot == null or r.random(100) > 50) slot = j;
            }
            if (slot) |j| pv.buildings[j] = 0;
        },
        13 => pv.unrest[@intCast(own)] = 500,
        14 => {
            const un = e.p1 + @as(i32, @bitCast(r.random(@bitCast(e.p2))));
            if (un > 0) {
                pv.unrest[@intCast(own)] = @max(pv.unrest[@intCast(own)] - un, 0);
            } else {
                pv.unrest[@intCast(own)] = @min(pv.unrest[@intCast(own)] + @divTrunc(@as(i32, @intCast(@abs(un))) * 100, 100 + unrestSlowdown(pr, pv)), 500);
            }
        },
        5, 20 => std.log.info("ritual {d}: the encounter effect is not ported", .{idx}),
        abil_barrier => {},
        else => {},
    };
    return out;
}

/// GetUnrestSlowdown (Province.cpp:566): inner building ability 30 and the
/// temple of the race (not ported).
fn unrestSlowdown(pr: *const Proto, pv: *const province.Province) i32 {
    _ = pr;
    _ = pv;
    return 0;
}

/// EndMainTurn: the cooldowns of every player go down by one.
pub fn turnCooldowns(s: *Shard, pr: *const Proto) void {
    for (1..s.n_players + 1) |k| {
        if (!s.in_game[k]) continue;
        for (1..pr.rituals.len) |i| {
            const idx: i32 = @intCast(i);
            const cd = cooldown(s, pr, k, idx);
            if (cd > 0) setCooldown(s, pr, k, idx, cd - 1);
        }
    }
}

/// BeginMainTurn: iCanCast = 1 + the castle's ability 54.
pub fn resetCasts(s: *Shard, pr: *const Proto) void {
    const e: economy.Economy = .{ .s = s, .pr = pr };
    for (1..s.n_players + 1) |k| {
        if (!s.in_game[k]) continue;
        s.rulers[k].holdings.can_cast = 1 + e.param1(@intCast(k), abil_cast);
    }
}
