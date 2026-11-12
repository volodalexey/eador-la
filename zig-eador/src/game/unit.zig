//! dcUnit (Unit.cpp): a concrete soldier.  Creation levels the unit up
//! through LevelUp/Roll, so it consumes the game's random numbers in the
//! original order (GOG FUN_004331f0, FUN_00432b60, FUN_00433130).
const std = @import("std");
const Rand = @import("../data/rand.zig").Rand;
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;

/// UNIT_LEVEL_CAP, EXP_FIRST_LEVEL, EXP_ADD, MAX_ROLL (Globals.h).
pub const level_cap = 30;
pub const exp_first_level = 20;
pub const exp_add = 10;
pub const max_roll = 1000;
/// GOG reads 5 ability slots (unit record +0x8C, FUN_004328f0); the C++
/// code reads MAX_UNIT_ABIL = 10.  unit.var never has more than 5.
pub const gog_abilities = 5;
/// unit_upg.var effect that turns the unit into unit type `value`.
pub const upg_change_type = 62;
/// The Exclude argument of LevelUp in the constructor.
const exclude_on_create = 92;

/// RollerN/RollerP with Roll(false) (Randomizer.cpp:90, GOG FUN_00454e80).
pub const Roller = struct {
    ids: [max_roll]i32 = @splat(0),
    weights: [max_roll]i32 = @splat(0),

    /// Entries before the first id 0.
    pub fn len(self: *const Roller) usize {
        return std.mem.indexOfScalar(i32, &self.ids, 0) orelse max_roll;
    }

    pub fn roll(self: *const Roller, r: *Rand) i32 {
        return self.pick(r.random(self.sum()) +% 1);
    }

    /// Sum of the weights (the Max of Roll's Random).
    pub fn sum(self: *const Roller) u32 {
        var s: u32 = 0;
        for (self.weights[0..self.len()]) |w| s +%= @bitCast(w);
        return s;
    }

    /// The entry for Random(Sum) + 1 = `v` (Roll with parameters takes a
    /// keyed Random).
    pub fn pick(self: *const Roller, v: u32) i32 {
        var acc: u32 = 0;
        var k: usize = 0;
        while (acc < v and k < max_roll) : (k += 1) acc +%= @bitCast(self.weights[k]);
        k -|= 1;
        while (k + 1 < max_roll and self.weights[k] == 0) k += 1;
        return self.ids[k];
    }

    /// Roll(true): the drawn id cannot come again.
    pub fn rollExclude(self: *Roller, r: *Rand) i32 {
        const v = self.roll(r);
        for (self.ids[0..self.len()], 0..) |id, i| {
            if (id == v) self.weights[i] = 0;
        }
        return v;
    }
};

/// GetUnitExp: experience for `level` levels of unit type `t`.
pub fn expForLevel(pr: *const Proto, t: i32, level: i32) i32 {
    const mod = pr.units[@intCast(t)].exp_mod;
    var result: i32 = 0;
    var adder: i32 = exp_first_level;
    var i: i32 = 0;
    while (i < level) : (i += 1) {
        result += @divTrunc(adder * mod, 100);
        adder += exp_add;
    }
    return result;
}

/// Sum of effect `stat` of one unit_upg.var record (stops at type 0).
fn upgBonus(pr: *const Proto, upg: i32, stat: i32) i32 {
    if (upg < 0 or upg >= pr.unit_upgrades.len) return 0;
    var total: i32 = 0;
    for (pr.unit_upgrades[@intCast(upg)].upg) |e| {
        if (e.id == 0) break;
        if (e.id == stat) total += e.value;
    }
    return total;
}

pub const Unit = struct {
    type: i32,
    life: i32,
    morale: i32 = 0,
    exp: i32,
    level: i32 = 0,
    x: i32 = 0,
    y: i32 = 3,
    upgrades: [level_cap]i32 = @splat(0),
    medals: [3]i32 = @splat(0),

    /// dcUnit(U_Type, Expirience): takes every level the experience gives.
    pub fn init(pr: *const Proto, r: *Rand, t: i32, exp: i32) Unit {
        var u: Unit = .{ .type = t, .life = pr.units[@intCast(t)].life, .exp = exp };
        while (u.level < u.countLevel(pr)) u.makeLevel(pr, u.levelUp(pr, r, u.level + 1, exclude_on_create));
        return u;
    }

    /// CountLevel.
    pub fn countLevel(self: *const Unit, pr: *const Proto) i32 {
        const mod = pr.units[@intCast(self.type)].exp_mod;
        var result: i32 = 0;
        var current: i32 = 0;
        var adder: i32 = exp_first_level;
        while (@divTrunc((current + adder) * mod, 100) <= self.exp) {
            result += 1;
            current += adder;
            adder += exp_add;
            if (result > level_cap) break;
        }
        return @min(result, level_cap);
    }

    /// CheckUpg without the hero bonus: taken upgrades and medals.
    pub fn checkUpg(self: *const Unit, pr: *const Proto, stat: i32) i32 {
        var total: i32 = 0;
        for (self.upgrades[0..@intCast(self.level)]) |u| {
            if (u < 1000) total += upgBonus(pr, u, stat);
        }
        for (self.medals) |m| {
            if (m <= 0 or m >= pr.medals.len) continue;
            for (pr.medals[@intCast(m)].effects) |e| {
                if (e.id == stat) total += e.value;
            }
        }
        return total;
    }

    /// CheckAbility: the unit type's own abilities.
    pub fn checkAbility(self: *const Unit, pr: *const Proto, stat: i32) i32 {
        var total: i32 = 0;
        for (pr.units[@intCast(self.type)].abilities[0..gog_abilities]) |a| total += upgBonus(pr, a, stat);
        return total;
    }

    /// GetLife: the maximum life.
    pub fn maxLife(self: *const Unit, pr: *const Proto) i32 {
        return pr.units[@intCast(self.type)].life + self.checkUpg(pr, 1) + self.checkAbility(pr, 1);
    }

    /// MakeLevel.
    pub fn makeLevel(self: *Unit, pr: *const Proto, upg: i32) void {
        const old_life = self.maxLife(pr);
        const first = pr.unit_upgrades[@intCast(upg)].upg[0];
        if (first.id == upg_change_type) {
            self.type = first.value;
            self.exp = @divTrunc(self.exp, 2);
            self.level = self.countLevel(pr);
            @memset(self.upgrades[@intCast(self.level)..], 0);
        } else {
            self.upgrades[@intCast(self.level)] = upg;
            self.level += 1;
            self.life += self.maxLife(pr) - old_life;
        }
    }

    /// LevelUp: rolls the upgrade for `level`, weighting the candidates of
    /// all levels up to it; `exclude` removes upgrades of the same kind.
    pub fn levelUp(self: *const Unit, pr: *const Proto, r: *Rand, level: i32, exclude: i32) i32 {
        var rl: Roller = .{};
        var count: [max_roll]i8 = @splat(0);
        const ut = &pr.units[@intCast(self.type)];
        const upgs = pr.unit_upgrades;
        var lv: usize = 0;
        while (lv < level) : (lv += 1) {
            // The upgrade taken on the previous level loses its share.
            if (lv > 0) {
                var k: usize = 0;
                while (rl.ids[k] != self.upgrades[lv - 1] and rl.ids[k] > 0) k += 1;
                if (rl.ids[k] > 0) {
                    if (count[k] > 0) {
                        rl.weights[k] -= @divTrunc(rl.weights[k], @as(i32, count[k]));
                        count[k] -= 1;
                    }
                    if (count[k] == 0) rl.weights[k] = 0;
                }
            }
            // Older candidates grow by their count.
            var k: usize = 0;
            while (rl.ids[k] > 0) : (k += 1) {
                if (rl.weights[k] > 0) rl.weights[k] += @as(i32, count[k]);
            }
            if (lv >= proto_mod.unit_levels) continue;
            for (ut.upgrades[lv]) |c| {
                if (c.id <= 0) continue;
                const ok = for (upgs[@intCast(c.id)].need) |need| {
                    if (need > 0 and self.checkAbility(pr, need) + self.checkUpg(pr, need) == 0) break false;
                } else true;
                if (!ok) continue;
                var j: usize = 0;
                while (rl.ids[j] > 0 and rl.ids[j] != c.id) j += 1;
                rl.ids[j] = c.id;
                rl.weights[j] = c.value;
                count[j] += 1;
            }
        }
        if (exclude > 0) {
            const ex = upgs[@intCast(exclude)].upg;
            for (0..max_roll) |i| {
                const u = upgs[@intCast(rl.ids[i])].upg;
                if (u[0].id == ex[0].id or u[1].id == ex[0].id or u[0].id == ex[1].id) rl.weights[i] = 0;
            }
        }
        return rl.roll(r);
    }

    /// Byte layout of a unit instance in .sav (0xA4 bytes, FUN_004331f0).
    pub const size = 0xA4;

    pub fn fromBytes(b: []const u8) Unit {
        var u: Unit = .{ .type = rd(b, 0x00), .life = rd(b, 0x04), .exp = rd(b, 0x0c) };
        u.morale = rd(b, 0x08);
        u.level = rd(b, 0x10);
        u.x = rd(b, 0x14);
        u.y = rd(b, 0x18);
        for (&u.upgrades, 0..) |*x, i| x.* = rd(b, 0x1c + 4 * i);
        for (&u.medals, 0..) |*x, i| x.* = rd(b, 0x94 + 4 * i);
        return u;
    }

    /// Inverse of fromBytes: the 0xA4 dcUnit record on disk.
    pub fn toBytes(self: *const Unit) [unit_size]u8 {
        var b: [unit_size]u8 = @splat(0);
        wr(&b, 0x00, self.type);
        wr(&b, 0x04, self.life);
        wr(&b, 0x08, self.morale);
        wr(&b, 0x0c, self.exp);
        wr(&b, 0x10, self.level);
        wr(&b, 0x14, self.x);
        wr(&b, 0x18, self.y);
        for (self.upgrades, 0..) |x, i| wr(&b, 0x1c + 4 * i, x);
        for (self.medals, 0..) |x, i| wr(&b, 0x94 + 4 * i, x);
        return b;
    }

    fn rd(b: []const u8, off: usize) i32 {
        return std.mem.readInt(i32, b[off..][0..4], .little);
    }

    fn wr(b: []u8, off: usize, v: i32) void {
        std.mem.writeInt(i32, b[off..][0..4], v, .little);
    }
};

pub const unit_size = 0xA4;

const tst = std.testing;
const GameFs = @import("../platform/fs.zig").GameFs;
const mapfile = @import("../formats/mapfile.zig");
const savefile = @import("../formats/savefile.zig");
const packfile = @import("../formats/packfile.zig");

fn readTestFile(gpa: std.mem.Allocator, path: []const u8) ![]u8 {
    const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, gpa, .unlimited) catch return error.SkipZigTest;
    defer gpa.free(bytes);
    return packfile.unpack(gpa, bytes);
}

test "expForLevel and countLevel agree" {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    for (1..pr.units.len) |t| {
        var l: i32 = 0;
        while (l <= level_cap) : (l += 1) {
            const u: Unit = .{ .type = @intCast(t), .life = 0, .exp = expForLevel(&pr, @intCast(t), l) };
            try tst.expectEqual(l, u.countLevel(&pr));
        }
    }
}

// Guards made by the original: levels follow the experience, every
// upgrade is a candidate of its level and life is the base plus the
// taken life upgrades.
test "guard units of the original are consistent" {
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
    for (save.guards) |g| {
        const army = g orelse continue;
        for (army.list()) |b| {
            const u = Unit.fromBytes(b);
            try tst.expectEqual(u.countLevel(&pr), u.level);
            try tst.expectEqual(@as(i32, 0), u.morale);
            for (u.upgrades[0..@intCast(u.level)], 0..) |upg, lv| {
                if (lv >= proto_mod.unit_levels) break;
                var found = false;
                for (pr.units[@intCast(u.type)].upgrades[0 .. lv + 1]) |cs| {
                    for (cs) |c| found = found or c.id == upg;
                }
                try tst.expect(found);
            }
            try tst.expectEqual(pr.units[@intCast(u.type)].life + u.checkUpg(&pr, 1), u.life);
            checked += 1;
        }
    }
    try tst.expect(checked > 100);
}
