//! The economy table of ScreenStatistics (Screens.cpp:6127): what player
//! `k` gets and pays in a turn, by source.  The same sums as
//! BeginMainTurn (economy.beginTurn), with the C++ screen's differences:
//! the guards' upkeep counts for every player, a guard paid by its units
//! (iGoldPayment < 0) costs GetDefenderGoldPayment, and the opponents'
//! rows hold the trade deals and inner ability 29.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const mapgen = @import("mapgen.zig");
const Shard = mapgen.Shard;
const economy = @import("economy.zig");
const movement = @import("movement.zig");
const hero_mod = @import("hero.zig");
const player = @import("player.zig");

/// Inner building, defender and hero abilities of the table.
const ab = struct {
    const castle_gold_gem = 5;
    const castle_resource_gold = 9;
    const castle_income_perc = 22;
    const castle_fourage = 28;
    const castle_opponent_gold = 29;
    const castle_profit = 32;
    const castle_corrupt = 51;
    const def_fourage = 4;
    const hero_pay_perc = 997;
    const hero_gold = 81;
    const hero_gem = 82;
};

/// (gold, gems).
pub const Pair = struct {
    gold: i32 = 0,
    gem: i32 = 0,

    fn add(self: *Pair, o: Pair) void {
        self.gold += o.gold;
        self.gem += o.gem;
    }
};

pub const Report = struct {
    /// Income: phrases 55 castle, 56 provinces, 57 heroes, 58 opponents.
    castle: Pair = .{},
    provinces: Pair = .{},
    heroes: Pair = .{},
    opponents_in: Pair = .{},
    income: Pair = .{},
    /// Expenses: 266 the heroes' troops, 59 guards, 60 garrisons,
    /// 58 opponents, 263 corruption.
    troops: Pair = .{},
    guards: Pair = .{},
    garrisons: Pair = .{},
    opponents_out: Pair = .{},
    corruption: Pair = .{},
    expenses: Pair = .{},

    pub fn balance(self: Report) Pair {
        return .{ .gold = self.income.gold - self.expenses.gold, .gem = self.income.gem - self.expenses.gem };
    }
};

/// The sums of ScreenStatistics for player `k`.
pub fn report(s: *Shard, pr: *const Proto, k: usize, max_profit: i32) Report {
    const eco: economy.Economy = .{ .s = s, .pr = pr, .max_profit = max_profit };
    const pl: i32 = @intCast(k);
    const r = &s.rulers[k];
    var out: Report = .{};
    var prov_num: i32 = 0;

    for (s.provinces[1..]) |*pv| {
        if (pv.player != pl) continue;
        if (pv.population > 0) prov_num += eco.corruption(pv);
        var fourage: i32 = 0;
        if (movement.siege(s, pv.number) < 0) {
            out.provinces.add(.{ .gold = eco.goldIncome(pv), .gem = eco.gemIncome(pv) });
            if (pv.defender > 0 and pv.defender < pr.defenders.len) {
                const d = &pr.defenders[@intCast(pv.defender)];
                out.guards.add(if (d.gold_payment < 0) defenderPayment(s, pr, pv.guard) else .{ .gold = d.gold_payment, .gem = d.gem_payment });
                for (d.abilities[0..d.n_abilities]) |a| {
                    if (a.id == ab.def_fourage) fourage = a.p1;
                }
            }
        }
        if (pv.garrison >= 0) {
            const g = &s.garrisons.items[@intCast(pv.garrison)];
            var gold: i32 = 0;
            for (0..hero_mod.max_squad) |j| {
                const p = eco.payment(g, j);
                gold += p[0];
                out.garrisons.gem += p[1];
            }
            gold -= @divTrunc(gold * (fourage + eco.param1(pl, ab.castle_fourage)), 100);
            out.garrisons.gold += @max(gold, 0);
        }
    }

    var perc: i32 = 0;
    const bld = s.castle[k];
    for (pr.inner_buildings[1..], 1..) |*b, i| {
        if (i >= bld.len or !bld[i]) continue;
        for (b.abilities[0..b.n_abilities]) |a| switch (a.id) {
            ab.castle_gold_gem => out.castle.add(.{ .gold = a.p1, .gem = a.p2 }),
            ab.castle_resource_gold => out.castle.gold += eco.playerResource(pl, a.p1) * a.p2,
            ab.castle_income_perc => perc += a.p1,
            else => {},
        };
    }
    out.castle.gold += @min(@divTrunc(r.gold * eco.param1(pl, ab.castle_profit), 100), max_profit);
    out.castle.add(.{ .gold = r.gold_add, .gem = r.gem_add });

    for (1..s.n_players + 1) |o| {
        if (o == k or !s.in_game[o]) continue;
        const mine = r.chronicle.trade[o];
        if (mine.on) {
            const theirs = s.rulers[o].chronicle.trade[k];
            out.opponents_out.add(.{ .gold = mine.gold, .gem = mine.gem });
            out.opponents_in.add(.{ .gold = theirs.gold, .gem = theirs.gem });
        }
        const obld = s.castle[o];
        for (pr.inner_buildings[1..], 1..) |*b, i| {
            if (i >= obld.len or !obld[i]) continue;
            for (b.abilities[0..b.n_abilities]) |a| {
                if (a.id != ab.castle_opponent_gold) continue;
                if (a.p1 > 0) out.opponents_in.gold += a.p1 else out.opponents_out.gold -= a.p1;
            }
        }
    }

    for (s.heroes.items) |*h| {
        if (h.owner != pl) continue;
        var pay: i32 = 0;
        for (0..hero_mod.max_squad) |j| {
            const p = eco.payment(h, j);
            pay += p[0];
            out.troops.gem += p[1];
        }
        if (h.alive()) {
            pay -= @divTrunc(pay * h.heroBonus(pr, ab.hero_pay_perc), 100);
            out.heroes.add(.{ .gold = h.heroBonus(pr, ab.hero_gold), .gem = h.heroBonus(pr, ab.hero_gem) });
        }
        out.troops.gold += pay;
    }

    out.income = out.provinces;
    out.income.add(out.castle);
    out.income.add(out.heroes);
    out.income.add(out.opponents_in);
    const extra = @divTrunc(out.income.gold * perc, 100);
    out.castle.gold += extra;
    out.income.gold += extra;

    const d = &pr.difficulties[@intCast(std.math.clamp(r.difficulty, 0, proto_mod.max_difficulty - 1))];
    var corrupt = @divTrunc(r.corrupt + prov_num, 10) - d.corrupt_start;
    if (r.kind == .human and corrupt > 0) {
        corrupt = @min(corrupt, d.corrupt_cap);
        corrupt = @divTrunc(corrupt * (100 + eco.param1(pl, ab.castle_corrupt)), 100);
        out.corruption = .{ .gold = @divTrunc(out.income.gold * corrupt, 100), .gem = @divTrunc(out.income.gem * corrupt, 100) };
    }
    out.expenses = out.guards;
    out.expenses.add(out.garrisons);
    out.expenses.add(out.troops);
    out.expenses.add(out.opponents_out);
    out.expenses.add(out.corruption);
    return out;
}

/// dcHero::GetDefenderGoldPayment / GemPayment: 1 + 1/20 of the guard's
/// units' gold upkeep, 1/20 of their gems.
pub fn defenderPayment(s: *const Shard, pr: *const Proto, guard: i32) Pair {
    var gold: i32 = 0;
    var gem: i32 = 0;
    if (guard >= 0) for (s.guards.items[@intCast(guard)].squad) |slot| {
        const u = slot orelse continue;
        gold += pr.units[@intCast(u.type)].gold_payment;
        gem += pr.units[@intCast(u.type)].gem_payment;
    };
    return .{ .gold = 1 + @divTrunc(gold, 20), .gem = @divTrunc(gem, 20) };
}

/// The middle column: the player's provinces, heroes in the list and
/// rulers defeated (arKills[1..16] > 0).
pub fn provinces(s: *const Shard, k: usize) i32 {
    var n: i32 = 0;
    for (s.provinces[1..]) |*pv| n += @intFromBool(pv.player == @as(i32, @intCast(k)));
    return n;
}

pub fn heroes(s: *const Shard, k: usize) i32 {
    var n: i32 = 0;
    for (s.heroes.items) |*h| n += @intFromBool(h.owner == @as(i32, @intCast(k)));
    return n;
}

pub fn defeated(s: *const Shard, k: usize) i32 {
    var n: i32 = 0;
    for (s.rulers[k].chronicle.kills[1..17]) |v| n += @intFromBool(v > 0);
    return n;
}

const tst = std.testing;
const GameFs = @import("../platform/fs.zig").GameFs;
const packfile = @import("../formats/packfile.zig");
const loadgame = @import("loadgame.zig");

fn readTestFile(gpa: std.mem.Allocator, path: []const u8) ![]u8 {
    const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, gpa, .unlimited) catch return error.SkipZigTest;
    defer gpa.free(bytes);
    return packfile.unpack(gpa, bytes);
}

// testdata/sav140: recomp/saves/sav140 (a single game in RU at turn
// 140); its README holds the statistics screen of the game.
test "statistics on turn 140" {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const mdata = try readTestFile(gpa, "testdata/sav140/map0.map");
    defer gpa.free(mdata);
    const gdata = try readTestFile(gpa, "testdata/sav140/game0.sav");
    defer gpa.free(gdata);
    var g = try loadgame.fromBytes(gpa, &pr, mdata, gdata);
    defer g.deinit();
    const s = &g.shard;
    const r = report(s, &pr, 1, 500);
    try tst.expectEqual(Pair{ .gold = 49, .gem = 15 }, r.castle);
    try tst.expectEqual(Pair{ .gold = 329, .gem = 78 }, r.provinces);
    try tst.expectEqual(Pair{ .gold = 20, .gem = 0 }, r.heroes);
    try tst.expectEqual(Pair{}, r.opponents_in);
    try tst.expectEqual(Pair{ .gold = 398, .gem = 93 }, r.income);
    try tst.expectEqual(Pair{ .gold = 487, .gem = 12 }, r.troops);
    try tst.expectEqual(Pair{ .gold = 54, .gem = 1 }, r.guards);
    try tst.expectEqual(Pair{ .gold = 25, .gem = 0 }, r.garrisons);
    try tst.expectEqual(Pair{}, r.opponents_out);
    try tst.expectEqual(Pair{ .gold = 3, .gem = 0 }, r.corruption);
    try tst.expectEqual(Pair{ .gold = 569, .gem = 13 }, r.expenses);
    try tst.expectEqual(Pair{ .gold = -171, .gem = 80 }, r.balance());

    // The rest of the screen.
    const ch = s.rulers[1].chronicle;
    try tst.expectEqual(@as(i32, 188), ch.win);
    try tst.expectEqual(@as(i32, 5), ch.lose);
    try tst.expectEqual(@as(i32, 21), provinces(s, 1));
    try tst.expectEqual(@as(i32, 4), heroes(s, 1));
    try tst.expectEqual(@as(i32, 0), defeated(s, 1));
    try tst.expectEqual([3]i32{ 5, 3, 0 }, ch.loot);
    try tst.expectEqual(@as(i32, 0), ch.quest);
    try tst.expectEqual(@as(i32, 4), ch.time_shift);

    // Resources: count, gold and gem price.
    const m: player.Market = .{ .s = s, .pr = &pr, .player = 1 };
    const want = [_][3]i32{
        .{ 2, 55, 0 },  .{ 0, 60, 0 },  .{ 2, 36, 0 },   .{ 1, 15, 8 },   .{ 2, 54, 22 },
        .{ 1, 124, 0 }, .{ 1, 90, 12 }, .{ 2, 480, 40 }, .{ 0, 352, 88 },
    };
    for (want, 1..) |w, i| {
        const ri: i32 = @intCast(i);
        const p = m.resPriceStat(ri);
        try tst.expectEqual(w, [3]i32{ m.resource(ri), p[0], p[1] });
    }
}
