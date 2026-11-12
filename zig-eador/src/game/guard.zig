//! Province and site guards: dcProvince::PlaceGuard / PlaceSiteGuard
//! (Province.cpp:132, :257).  A guard is a dcHero in the original; only
//! the fields a guard uses are kept here.
const std = @import("std");
const Rand = @import("../data/rand.zig").Rand;
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const unit_mod = @import("unit.zig");
const Unit = unit_mod.Unit;
const savefile = @import("../formats/savefile.zig");

/// MAX_SQUAD.
pub const max_squad = 16;
pub const hero_size = 0x3AC;

pub const Guard = struct {
    /// guard.var index (iExp of the guard hero), for the loot.
    guard: i32,
    /// Karma of the guard type (ipName); 0 for site guards.
    karma: i32 = 0,
    /// Initiative (iTask).
    initiative: i32,
    /// ipPlayer: 0 for the neutrals, else a hired defender's owner.
    player: i32 = 0,
    squad: [max_squad]?Unit = @splat(null),
    /// The loaded record; the fields not decoded go back unchanged.
    rec: ?[hero_size]u8 = null,

    pub fn count(self: *const Guard) usize {
        var n: usize = 0;
        for (self.squad) |u| n += @intFromBool(u != null);
        return n;
    }

    /// Write the army block as savefile reads it: 1 byte present,
    /// hero_size record, 16 × (1 byte present, unit_size bytes).
    pub fn writeBlock(self: *const Guard, out: *std.ArrayList(u8), gpa: std.mem.Allocator) !void {
        try out.append(gpa, 1);
        var hero: [hero_size]u8 = undefined;
        if (self.rec) |r| hero = r else {
            hero = @splat(0);
            // dcHero(): iDead = -1, arGarrison[4] = -1.
            setI(&hero, 0x4c, -1);
            for (0..4) |k| setI(&hero, 0x390 + 4 * k, -1);
        }
        setI(&hero, 0x00, self.karma);
        setI(&hero, 0x08, self.player);
        setI(&hero, 0x0c, self.guard);
        setI(&hero, 0x38, self.initiative);
        try out.appendSlice(gpa, &hero);
        for (self.squad) |slot| {
            if (slot) |u| {
                try out.append(gpa, 1);
                const b = u.toBytes();
                try out.appendSlice(gpa, &b);
            } else {
                try out.append(gpa, 0);
            }
        }
    }
};

/// A saved block back into a guard: the inverse of writeBlock.
pub fn fromArmy(a: *const savefile.Army) Guard {
    var g: Guard = .{
        .karma = getI(a.rec, 0x00),
        .guard = getI(a.rec, 0x0c),
        .initiative = getI(a.rec, 0x38),
        .player = getI(a.rec, 0x08),
        .rec = a.rec[0..hero_size].*,
    };
    for (a.list(), a.slots[0..a.n]) |u, slot| g.squad[slot] = Unit.fromBytes(u);
    return g;
}

fn getI(b: []const u8, off: usize) i32 {
    return std.mem.readInt(i32, b[off..][0..4], .little);
}

fn setI(b: []u8, off: usize, v: i32) void {
    std.mem.writeInt(i32, b[off..][0..4], v, .little);
}

/// Level of a guard unit: the guard's level ±3, clamped (Random(6)-3).
fn unitLevel(r: *Rand, base: i32) i32 {
    const lvl = base + @as(i32, @intCast(r.random(6))) - 3;
    return std.math.clamp(lvl, 0, unit_mod.level_cap);
}

fn makeUnit(pr: *const Proto, r: *Rand, gu: proto_mod.GuardUnit, i: usize) Unit {
    const lvl = unitLevel(r, gu.level);
    var u = Unit.init(pr, r, gu.unit, unit_mod.expForLevel(pr, gu.unit, lvl));
    u.x = @intCast(i / 8);
    u.y = @intCast(i % 8);
    return u;
}

/// PlaceGuard (karma set) or PlaceSiteGuard (karma 0): `num` units by the
/// minimum counts of the unit kinds, then Random(num_add + 1) extra units
/// rolled by possibility.
pub fn place(pr: *const Proto, r: *Rand, guardian: i32, karma: i32) Guard {
    const g = &pr.guards[@intCast(guardian)];
    var out: Guard = .{
        .guard = guardian,
        .karma = karma,
        .initiative = g.quick + @as(i32, @intCast(r.random(@intCast(g.quick_add + 1)))),
    };
    const num: usize = @intCast(@max(0, @min(g.num, max_squad)));
    for (0..num) |i| {
        var kind: usize = 0;
        var total: i32 = g.units[0].min_count;
        while (i >= total and kind + 1 < proto_mod.max_guard_unit) {
            kind += 1;
            total += g.units[kind].min_count;
        }
        out.squad[i] = makeUnit(pr, r, g.units[kind], i);
    }
    const add: usize = r.random(@intCast(g.num_add + 1));
    var i = num;
    while (i < num + add and i < max_squad) : (i += 1) {
        var rl: unit_mod.Roller = .{};
        for (g.units, 0..) |gu, j| {
            if (gu.unit > 0) {
                rl.ids[j] = @intCast(j + 1);
                rl.weights[j] = gu.possibility;
            }
        }
        const kind: usize = @intCast(rl.roll(r) - 1);
        out.squad[i] = makeUnit(pr, r, g.units[kind], i);
    }
    return out;
}

/// PlaceDefender (Province.cpp:210): defender.var record `d` for `player`;
/// iExp is -d (the loot of a defender), `exp_bonus` GetBldParam1(20).
pub fn placeDefender(pr: *const Proto, r: *Rand, d: i32, player: i32, exp_bonus: i32) Guard {
    const def = &pr.defenders[@intCast(d)];
    var out: Guard = .{ .guard = -d, .initiative = def.initiative, .player = player };
    for (0..max_squad) |i| {
        var kind: usize = 0;
        var total: i32 = def.units[0].count;
        while (i >= total and def.units[kind].unit > 0 and kind + 1 < proto_mod.max_guard_unit) {
            kind += 1;
            total += def.units[kind].count;
        }
        const du = def.units[kind];
        if (du.unit <= 0) continue;
        const lvl = unitLevel(r, du.level);
        var u = Unit.init(pr, r, du.unit, unit_mod.expForLevel(pr, du.unit, lvl) + exp_bonus);
        u.x = @intCast(i / 8);
        u.y = @intCast(i % 8);
        out.squad[i] = u;
    }
    return out;
}

const tst = std.testing;
const GameFs = @import("../platform/fs.zig").GameFs;
const mapfile = @import("../formats/mapfile.zig");
const packfile = @import("../formats/packfile.zig");

fn readTestFile(gpa: std.mem.Allocator, path: []const u8) ![]u8 {
    const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, gpa, .unlimited) catch return error.SkipZigTest;
    defer gpa.free(bytes);
    return packfile.unpack(gpa, bytes);
}

fn rd(b: []const u8, off: usize) i32 {
    return std.mem.readInt(i32, b[off..][0..4], .little);
}

// Province guards of the original: the record keeps the guard index
// (+0x0C) and initiative (+0x38) in range, units fill slots 0.. in order
// and their number is num .. num + num_add.
test "province guards of the original fit their guard.var record" {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const mdata = try readTestFile(gpa, "testdata/new_single/map0.map");
    defer gpa.free(mdata);
    const gdata = try readTestFile(gpa, "testdata/new_single/game0.sav");
    defer gpa.free(gdata);
    var ref = try mapfile.parse(gpa, mdata);
    defer ref.deinit(gpa);
    var save = try savefile.parse(gpa, gdata, ref.provinces.len);
    defer save.deinit(gpa);
    var checked: usize = 0;
    for (save.guards) |ga| {
        const army = ga orelse continue;
        const gi = rd(army.rec, 0x0c);
        if (gi <= 0) continue; // defenders keep -index
        const g = pr.guards[@intCast(gi)];
        const ini = rd(army.rec, 0x38);
        try tst.expect(ini >= g.quick and ini <= g.quick + g.quick_add);
        try tst.expect(army.n >= g.num and army.n <= g.num + g.num_add);
        for (army.slots[0..army.n], 0..) |s, k| try tst.expectEqual(@as(u8, @intCast(k)), s);
        for (army.list(), 0..) |b, k| {
            const u = Unit.fromBytes(b);
            try tst.expectEqual(@as(i32, @intCast(k / 8)), u.x);
            try tst.expectEqual(@as(i32, @intCast(k % 8)), u.y);
            var known = false;
            for (g.units) |gu| known = known or (gu.unit > 0 and gu.unit == u.type);
            // A type-changing upgrade (62) may have replaced the type.
            if (!known) for (g.units) |gu| {
                if (gu.unit <= 0) continue;
                for (pr.units[@intCast(gu.unit)].upgrades) |lvl| for (lvl) |c| {
                    if (c.id > 0 and pr.unit_upgrades[@intCast(c.id)].upg[0].id == unit_mod.upg_change_type and
                        pr.unit_upgrades[@intCast(c.id)].upg[0].value == u.type) known = true;
                };
            };
            try tst.expect(known);
        }
        checked += 1;
    }
    try tst.expect(checked > 20);
}

test "place gives a squad within the record's limits" {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    var r: Rand = .{};
    r.seed(42);
    for (pr.guards[1..], 1..) |g, gi| {
        const out = place(&pr, &r, @intCast(gi), 7);
        const n: i32 = @intCast(out.count());
        try tst.expect(n >= @min(g.num, max_squad) and n <= g.num + g.num_add);
        try tst.expect(out.initiative >= g.quick and out.initiative <= g.quick + g.quick_add);
        for (out.squad[0..@intCast(n)]) |u| try tst.expect(u != null);
    }
}
