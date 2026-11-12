//! CompareStrength1 (Simulation.cpp:4): the strength of two armies by a
//! simulated battle on an empty field.  Nothing rolls (Battle.simulate),
//! the units see only the columns (battle_ai.simulateUnit) and nothing is
//! drawn.  The result is 100 for an even match, above 100 when the first
//! army keeps more of its worth.
//!
//! Not ported: the bAIBattle branch (two computer players fight it out
//! for real and EndBattle runs); the caller sets LifeMod in the armies.
const std = @import("std");
const proto = @import("../data/proto.zig");
const Rand = @import("../data/rand.zig").Rand;
const bt = @import("battle.zig");
const battle_ai = @import("battle_ai.zig");

/// CompareStrength1(Hero1, Hero2) for armies[0] and armies[1].
pub fn compareStrength(gpa: std.mem.Allocator, pr: *const proto.Proto, r: *Rand, armies_in: [2]bt.Army) i32 {
    var armies = armies_in;
    // The squads stand in their own two columns; side 1 is mirrored.
    for (&armies) |*a| for (&a.squad) |*slot| if (slot.*) |*u| {
        if (u.y < 0 or u.y > 7) u.y = 3;
        if (u.x < 0 or u.x > 1) u.x = 0;
    };
    var b = bt.Battle.init(gpa, pr, r, .{}, armies);
    defer b.deinit();
    b.simulate = true;
    b.side = 1 - b.firstSide();
    while (b.firstAlive(0) != null and b.firstAlive(1) != null and @divTrunc(b.turn, 2) <= b.max_duration) {
        b.endTurn();
        battle_ai.simulateDanger(&b);
        battle_ai.simulatePower(&b);
        for (0..bt.n_slots) |i| {
            const ref: bt.Ref = .{ .side = b.side, .slot = @intCast(i) };
            const f = b.fighter(ref);
            if (f.present() and f.life > 0 and f.can_act and f.in_battle) battle_ai.simulateUnit(&b, ref);
        }
        b.anims.clearRetainingCapacity();
    }
    // The worth kept: 10 for the hero, the unit level for the others.
    var power: [2]i32 = .{ 1, 1 };
    var kept: [2]i32 = .{ 1, 1 };
    for (0..2) |s| {
        const has_hero = if (armies[s].hero) |h| h.class1 > 0 else false;
        for (armies[s].squad, 0..) |slot, j| {
            const u = slot orelse continue;
            const price: i32 = if (j == 0 and has_hero) 10 else pr.units[@intCast(u.type)].level;
            if (u.life > 0) power[s] += price;
            if (b.units[s][j].life > 0) kept[s] += price;
        }
    }
    return 100 + @divTrunc(100 * kept[0], power[0]) - @divTrunc(100 * kept[1], power[1]);
}

// testdata/hero_move: the hero against the guard of province 35, both
// ways round, and the hero against itself.
test "simulated battle of the hero and a guard" {
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
    const gd = &s.guards.items[@intCast(s.provinces[35].guard)];
    const before = h.squad;
    const hero: bt.Army = .{ .hero = h, .initiative = h.heroBonus(&pr, @import("hero.zig").bonus.initiative), .player = h.owner, .gem = 100, .squad = h.squad };
    const guard: bt.Army = .{ .initiative = gd.initiative, .guard = gd.guard, .squad = gd.squad };
    var r: Rand = .{};
    const a = compareStrength(gpa, &pr, &r, .{ hero, guard });
    const b2 = compareStrength(gpa, &pr, &r, .{ guard, hero });
    try tst.expect(a >= 0 and a <= 200);
    try tst.expect(b2 >= 0 and b2 <= 200);
    // The rolls do not decide: the same battle gives the same strength.
    var r2: Rand = .{ .state = 12345 };
    try tst.expectEqual(a, compareStrength(gpa, &pr, &r2, .{ hero, guard }));
    // The armies outside are untouched.
    for (before, h.squad) |x, y| try tst.expectEqual(x == null, y == null);
    if (before[0]) |u| try tst.expectEqual(u.life, h.squad[0].?.life);
}
