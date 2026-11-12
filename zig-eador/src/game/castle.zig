//! Castle building of a player: dcPlayer::Build (Player.cpp:844) with
//! GetInBldGoldPrice / GetInBldGemPrice (:508), AddMercenary (:1012) and
//! the mercenary timer of EndMainTurn (MainProcess.cpp:6630).
//!
//! Build fills what the castle tabs show: the shop (ability 1), the
//! library (2), the capital's garrison slots (4), the mercenary (6) and
//! the units for hire (8).  Also ported: 35 (the capital's iFort and
//! iWall) and 46 (karma).  Not ported: 11, 15, 27 (pOuterBld,
//! pDefender, pRitual are in the saved lists), 30, 36, 39, 40 (dcPlayer
//! fields not in Ruler), 48 (pStory), 50 (PlaceSite).
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const Rand = @import("../data/rand.zig").Rand;
const mapgen = @import("mapgen.zig");
const Shard = mapgen.Shard;
const player = @import("player.zig");
const ritual_mod = @import("ritual.zig");
const Roller = @import("unit.zig").Roller;

/// Inner building abilities used by Build.
pub const abil = struct {
    pub const shop = 1;
    pub const library = 2;
    pub const garrison = 4;
    pub const mercenary = 6;
    pub const recruit = 8;
    /// Buildings that make an outer building or a defender available.
    pub const outer_stock = 11;
    pub const defender_stock = 15;
    pub const build_discount = 16;
    pub const fort = 35;
    pub const tower = 36;
    pub const can_hire = 39;
    pub const can_outer_bld = 40;
    pub const karma = 46;
    /// A building that makes a ritual available (Player.cpp:968).
    pub const ritual = 27;
};

/// arGarrison of a garrison record (dcHero +0x390).
const rec_garrison = 0x390;

/// GetInBldGoldPrice / GetInBldGemPrice.
pub fn innerPrice(m: player.Market, id: i32) player.Price {
    const b = &m.pr.inner_buildings[@intCast(id)];
    const cut = m.bldParam1(abil.build_discount);
    var out: player.Price = .{ b.gold_cost, b.gem_cost };
    for (&out) |*v| v.* -= @divTrunc(v.* * cut, 100);
    for (b.res) |r| if (r > 0) {
        const add = m.resPrice(r);
        out[0] += add[0];
        out[1] += add[1];
    };
    return out;
}

/// arBld of player `k`; a new game gets it here (arBld[0] set by Init).
pub fn buildings(s: *Shard, k: usize) ![]bool {
    if (s.castle[k].len != proto_mod.max_bld) {
        const b = try s.arena.allocator().alloc(bool, proto_mod.max_bld);
        @memset(b, false);
        b[0] = true;
        s.castle[k] = b;
    }
    return s.castle[k];
}

/// dcPlayer::Build: player `k` pays for and builds inner building `id`.
pub fn build(s: *Shard, pr: *const Proto, r: *Rand, k: usize, id: i32) !void {
    if (id <= 0 or id >= pr.inner_buildings.len) return error.BadBuilding;
    const m: player.Market = .{ .s = s, .pr = pr, .player = @intCast(k) };
    const bld = try buildings(s, k);
    m.pay(innerPrice(m, id));
    const h = m.holdings();
    h.can_inner_bld -= 1;
    bld[@intCast(id)] = true;
    const b = &pr.inner_buildings[@intCast(id)];
    m.raiseResPrices(&b.res);
    for (b.abilities[0..b.n_abilities]) |a| switch (a.id) {
        abil.shop => addShopItems(pr, r, h, id, a),
        abil.library => {
            var at = std.mem.indexOfScalar(i32, &h.library, 0) orelse continue;
            for (pr.spells[1..], 1..) |sp, j| {
                if (sp.building != id) continue;
                if (at >= h.library.len) break;
                h.library[at] = @intCast(j);
                at += 1;
            }
        },
        abil.garrison => setGarrison(s, k, a),
        abil.mercenary => if (h.recruit[player.merc_slot] == 0) addMercenary(m, r),
        abil.recruit => {
            const at = if (a.p2 == 0) std.mem.indexOfScalar(i32, &h.recruit, 0) orelse continue else player.merc_slot;
            h.recruit[at] = a.p1;
        },
        abil.fort => {
            const cap = s.capitals[k];
            if (cap > 0) s.at(cap).fort += a.p1;
            m.ruler().wall += a.p1;
        },
        abil.outer_stock => ritual_mod.grantList(s, pr, k, .outer, a),
        abil.defender_stock => ritual_mod.grantList(s, pr, k, .defender, a),
        abil.tower => h.tower += a.p1,
        abil.can_hire => h.can_hire += a.p1,
        abil.can_outer_bld => h.can_outer_bld += a.p1,
        abil.karma => m.ruler().karma += a.p1,
        abil.ritual => ritual_mod.grant(s, pr, k, a),
        else => {},
    };
}

/// Ability 1: p1 0 / 1 puts the items made by the building (unlimited /
/// one of each); p1 >= 2 rolls 7 - p2 items of shop level p2 and rarity
/// 2..p2, weighted 7 - rarity, each once.
fn addShopItems(pr: *const Proto, r: *Rand, h: *player.Holdings, id: i32, a: proto_mod.Ability3) void {
    const items = &h.shop.items;
    var at: usize = 0;
    while (at < items.len and items[at][0] > 0) at += 1;
    if (a.p1 < 2) {
        for (pr.items[1..], 1..) |it, j| {
            if (it.building != id) continue;
            if (at >= items.len) return;
            items[at] = .{ @intCast(j), if (a.p1 == 0) 0 else 1 };
            at += 1;
        }
        return;
    }
    var roll: Roller = .{};
    var n: usize = 0;
    for (pr.items[1..], 1..) |it, j| {
        if (it.shop_level != a.p2 or it.class >= 10 or it.rarity <= 1 or it.rarity > a.p2) continue;
        if (n == roll.ids.len) break;
        roll.ids[n] = @intCast(j);
        roll.weights[n] = 7 - it.rarity;
        n += 1;
    }
    var i: i32 = 0;
    while (i < 7 - a.p2 and at < items.len) : (i += 1) {
        items[at] = .{ roll.rollExclude(r), 1 };
        at += 1;
    }
}

/// Ability 4: arGarrison[p1 - 1] = p2 of the capital's garrison.
fn setGarrison(s: *Shard, k: usize, a: proto_mod.Ability3) void {
    if (a.p1 < 1 or a.p1 > 4) return;
    const cap = s.capitals[k];
    if (cap < 1) return;
    const g = s.provinces[@intCast(cap)].garrison;
    if (g < 0) return;
    const o: usize = rec_garrison + 4 * @as(usize, @intCast(a.p1 - 1));
    std.mem.writeInt(i32, s.garrisons.items[@intCast(g)].rec[o..][0..4], a.p2, .little);
}

/// AddMercenary: arRecruit[11] rolled from mercenary.var levels
/// 1..GetBldParam1(6).
pub fn addMercenary(m: player.Market, r: *Rand) void {
    const lv = m.bldParam1(player.abil.merc_level);
    if (lv <= 0) return;
    var roll: Roller = .{};
    const n = @min(@as(usize, @intCast(lv)), proto_mod.max_merc_level);
    for (m.pr.mercenaries[0..n], 0..) |level, i| for (level, 0..) |p, j| {
        roll.ids[i * proto_mod.max_mercenary + j] = p.id;
        roll.weights[i * proto_mod.max_mercenary + j] = p.value;
    };
    m.holdings().recruit[player.merc_slot] = roll.roll(r);
}

/// MAX_BUILDING_NEED, MAX_BUILDING_RES.
pub const max_need = proto_mod.max_building_need;
pub const max_res = proto_mod.max_building_res;

/// Inner building ability 47: needs the alliance with race p1.
pub const abil_union = 47;
/// Slots of ScreenCastleBuild: 0 the planned building, 1 unused, 2..62
/// the quarters.
pub const n_slots = 63;
/// Needs >= 1000 (IsParent, IsRestricted) / > MAX_BLD (CanBuild): the
/// whole group n - 1000 built.
const group_need = 1000;
/// Guard against a cycle in inner_build.var needs.
const max_depth = 64;

/// The castle of a player for the building rules: GroupIsFull, IsParent,
/// IsRestricted (Castle.cpp:3..110), dcPlayer::CanBuild (Player.cpp:716),
/// NoAllyFlag (Windower.cpp:3874).  The campaign (iGameType 2: shard and
/// campaign buildings) and tutorial restrictions are not ported.
pub const Rules = struct {
    pr: *const Proto,
    bld: []const bool,
    union_race: i32,

    pub fn of(s: *Shard, pr: *const Proto, k: usize) !Rules {
        return .{ .pr = pr, .bld = try buildings(s, k), .union_race = s.rulers[k].holdings.union_race };
    }

    pub fn built(self: Rules, b: i32) bool {
        return b >= 0 and b < self.bld.len and self.bld[@intCast(b)];
    }

    fn maxInner(self: Rules) usize {
        return self.pr.inner_buildings.len -| 1;
    }

    pub fn info(self: Rules, b: i32) *const proto_mod.InnerBuilding {
        const i: usize = if (b > 0 and b < self.pr.inner_buildings.len) @intCast(b) else 0;
        return &self.pr.inner_buildings[i];
    }

    /// arBldGroup[g].
    pub fn groupMax(self: Rules, g: i32) i32 {
        return if (g >= 0 and g < self.pr.build_groups.len) self.pr.build_groups[@intCast(g)].max else 0;
    }

    /// Built buildings of group `g` among 1..`upto`.
    pub fn groupCount(self: Rules, g: i32, upto: usize) i32 {
        var n: i32 = 0;
        for (self.pr.inner_buildings[1..@min(upto + 1, self.pr.inner_buildings.len)], 1..) |b, i| {
            if (b.group == g and self.bld[i]) n += 1;
        }
        return n;
    }

    pub fn groupFull(self: Rules, g: i32) bool {
        return self.groupCount(g, self.maxInner()) >= self.groupMax(g);
    }

    fn needsUnion(self: Rules, b: *const proto_mod.InnerBuilding) bool {
        for (b.abilities[0..b.n_abilities]) |a| {
            if (a.id == abil_union and self.union_race != a.p1) return true;
        }
        return false;
    }

    /// dcPlayer::CanBuild: not built, the needs are built (a group need:
    /// arBldGroup buildings of it), the group has room, the alliance is
    /// there.  Its group loops stop before iMaxInnerBld, as in C++.
    pub fn canBuild(self: Rules, id: i32) bool {
        if (self.built(id)) return false;
        const b = self.info(id);
        const short = self.maxInner() -| 1;
        for (b.needs) |n| {
            if (n > proto_mod.max_bld) {
                const g = @mod(n, proto_mod.max_bld);
                if (self.groupCount(g, short) < self.groupMax(g)) return false;
            } else if (!self.built(n)) return false;
        }
        if (b.group > 0 and self.groupCount(b.group, short) >= self.groupMax(b.group)) return false;
        return !self.needsUnion(b);
    }

    /// IsRestricted: never buildable (the alliance is missing, the group
    /// is full, or so is a need's).
    pub fn isRestricted(self: Rules, id: i32) bool {
        return self.restricted(id, 0);
    }

    fn restricted(self: Rules, id: i32, depth: usize) bool {
        if (self.built(id) or depth > max_depth) return false;
        const b = self.info(id);
        if (self.needsUnion(b)) return true;
        if (b.group > 0 and self.groupFull(b.group)) return true;
        for (b.needs) |n| {
            if (n > 0 and n < group_need and self.restricted(n, depth + 1)) return true;
        }
        return false;
    }

    /// IsParent: 1 `father` is a need of `son` (directly or deeper), 2
    /// through a group need whose group is not full, 0 neither.
    pub fn isParent(self: Rules, father: i32, son: i32) u2 {
        return self.parent(father, son, 0);
    }

    fn parent(self: Rules, father: i32, son: i32, depth: usize) u2 {
        if (depth > max_depth) return 0;
        for (self.info(son).needs) |n| {
            if (n <= 0) continue;
            if (n < group_need) {
                if (n == father or self.parent(father, n, depth + 1) != 0) return 1;
                continue;
            }
            const g = n - group_need;
            if (self.groupFull(g)) continue;
            for (self.pr.inner_buildings[1..], 1..) |ib, j| {
                if (ib.group != g) continue;
                const ji: i32 = @intCast(j);
                if (ji == father or self.parent(father, ji, depth + 1) != 0) return 2;
            }
        }
        return 0;
    }

    /// NoAllyFlag: neither the building nor a need needs a missing
    /// alliance.
    pub fn noAllyFlag(self: Rules, id: i32) bool {
        return self.noAlly(id, 0);
    }

    fn noAlly(self: Rules, id: i32, depth: usize) bool {
        if (depth > max_depth) return true;
        const b = self.info(id);
        if (self.needsUnion(b)) return false;
        for (b.needs) |n| {
            if (n > 0 and n < group_need and !self.noAlly(n, depth + 1)) return false;
        }
        return true;
    }

    /// A building of the same group in an earlier slot: `k` itself or an
    /// upgrade step below it.
    fn taken(self: Rules, out: *const [n_slots]i32, i: usize, k: i32) bool {
        for (out[2..i]) |m| {
            if (m == k) return true;
            var bl = m;
            var d: usize = 0;
            while (self.info(bl).upgrade > 0 and d < max_depth) : (d += 1) {
                if (self.info(bl).upgrade == k) return true;
                bl = self.info(bl).upgrade;
            }
        }
        return false;
    }

    /// Of a full group: the first built building of `group` by slot that
    /// no earlier slot shows.  `stop_outside`: the search ends at the
    /// first slot of another group after the group's ones.
    fn builtOfGroup(self: Rules, out: *const [n_slots]i32, i: usize, group: i32, slot_end: i32, stop_outside: bool) ?i32 {
        var slot: i32 = 2;
        var inside = false;
        while (slot < slot_end) : (slot += 1) {
            // Search=false does not end the scan of the current slot.
            var stop = false;
            for (self.pr.inner_buildings[1..], 1..) |b, k| {
                if (b.slot != slot) continue;
                if (b.group != group) {
                    if (stop_outside and inside) stop = true;
                    continue;
                }
                if (!stop_outside) inside = true;
                if (!self.bld[k]) continue;
                inside = true;
                if (!self.taken(out, i, @intCast(k))) return @intCast(k);
            }
            if (stop) return null;
        }
        return null;
    }

    /// The building of each slot of ScreenCastleBuild (Castle.cpp:443):
    /// the military quarter 4..16 by the groups 1, 3, 7, 11 (a full group
    /// shows its built buildings from the first slots), the allied
    /// quarter 60..62, the rest by slot (a full group again shows what is
    /// built); then the highest upgrade built or allowed.
    pub fn slots(self: Rules, out: *[n_slots]i32) void {
        @memset(out, 0);
        const ibs = self.pr.inner_buildings;
        for (2..n_slots) |i| {
            const si: i32 = @intCast(i);
            if (i >= 4 and i <= 16) {
                const groups = [_][2]i32{ .{ 8, 1 }, .{ 11, 3 }, .{ 13, 7 }, .{ 14, 11 } };
                for (groups) |gr| {
                    if (self.groupFull(gr[1])) {
                        if (si >= gr[0]) continue;
                        if (self.builtOfGroup(out, i, gr[1], 17, false)) |k| {
                            out[i] = k;
                            break;
                        }
                        continue;
                    }
                    for (ibs[1..], 1..) |b, j| {
                        if (b.slot == si and b.upgrade == 0 and b.group == gr[1] and !self.isRestricted(@intCast(j))) {
                            out[i] = @intCast(j);
                            break;
                        }
                    }
                    break;
                }
            } else if (i > 59) {
                for (ibs[1..], 1..) |b, j| {
                    if (b.slot == si and !self.isRestricted(@intCast(j))) {
                        out[i] = @intCast(j);
                        break;
                    }
                }
            } else {
                // No break: the last matching building wins, as in C++.
                for (ibs[1..], 1..) |b, j| {
                    if (b.slot != si or b.upgrade != 0 or self.isRestricted(@intCast(j))) continue;
                    if (b.group > 0 and self.groupFull(b.group)) {
                        if (self.builtOfGroup(out, i, b.group, max_slot_search, true)) |k| out[i] = k;
                    } else out[i] = @intCast(j);
                }
            }
            self.upgradeSlot(out, i);
        }
    }

    /// While the slot's building is built: its first upgrade that is built,
    /// or not hidden-and-unbuildable, not restricted and with room in its
    /// group.
    fn upgradeSlot(self: Rules, out: *[n_slots]i32, i: usize) void {
        var d: usize = 0;
        while (out[i] > 0 and self.built(out[i]) and d < max_depth) : (d += 1) {
            const next = for (self.pr.inner_buildings[1..], 1..) |b, k| {
                if (b.upgrade != out[i]) continue;
                const ki: i32 = @intCast(k);
                var add = if (b.group > 0 and self.groupFull(b.group)) self.bld[k] else true;
                if (((b.hidden != 0 and !self.canBuild(ki)) or self.isRestricted(ki)) and !self.bld[k]) add = false;
                if (add) break ki;
            } else break;
            out[i] = next;
        }
    }
};

/// The slot search of a full group in the other quarters has no bound in
/// C++; it stops at the first slot of another group.
const max_slot_search = 1000;

/// BeginMainTurn (MainProcess.cpp:8410): one castle building a turn if
/// anything can be built.
pub fn resetTurn(s: *Shard, pr: *const Proto, k: usize) !void {
    const r = try Rules.of(s, pr, k);
    const h = &s.rulers[k].holdings;
    h.can_inner_bld = 0;
    // BeginMainTurn: iCanHire, iCanOuterBld and iCanCast are 1 plus the
    // castle's abilities 39, 40 and 54.
    const m: player.Market = .{ .s = s, .pr = pr, .player = @intCast(k) };
    h.can_hire = 1 + m.bldParam1(39);
    h.can_outer_bld = 1 + m.bldParam1(40);
    h.can_cast = 1 + m.bldParam1(54);
    for (1..r.maxInner() + 1) |i| {
        const id: i32 = @intCast(i);
        if (r.canBuild(id) and !r.isRestricted(id)) {
            h.can_inner_bld = 1;
            return;
        }
    }
}

/// EndMainTurn: a new mercenary every MERCENARY_TIME turns, or after half
/// of it when the slot is empty and the castle has ability 24.
pub fn mercenaryTurn(s: *Shard, pr: *const Proto, r: *Rand, merc_time: i32) void {
    for (1..s.n_players + 1) |k| {
        if (!s.in_game[k]) continue;
        const m: player.Market = .{ .s = s, .pr = pr, .player = @intCast(k) };
        const h = m.holdings();
        h.merc_time += 1;
        if (h.merc_time == merc_time) {
            addMercenary(m, r);
            h.merc_time = 0;
        } else if (h.recruit[player.merc_slot] == 0 and m.bldParam1(player.abil.dismiss_refund) > 0 and h.merc_time > @divTrunc(merc_time, 2)) {
            addMercenary(m, r);
            h.merc_time = 0;
        }
    }
}

const tst = std.testing;

fn readTestFile(gpa: std.mem.Allocator, path: []const u8) ![]u8 {
    const packfile = @import("../formats/packfile.zig");
    const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, gpa, .unlimited) catch return error.SkipZigTest;
    defer gpa.free(bytes);
    return packfile.unpack(gpa, bytes);
}

/// The first building not built yet with ability `id` (and p2 == `p2`).
fn withAbility(pr: *const Proto, bld: []const bool, id: i32, p2: ?i32) ?struct { i32, proto_mod.Ability3 } {
    for (pr.inner_buildings[1..], 1..) |b, i| {
        if (bld[i]) continue;
        for (b.abilities[0..b.n_abilities]) |a| {
            if (a.id != id) continue;
            if (p2) |v| if (a.p2 != v) continue;
            return .{ @intCast(i), a };
        }
    }
    return null;
}

test "Build fills recruits, library, garrison slots and shop" {
    const gpa = tst.allocator;
    const GameFs = @import("../platform/fs.zig").GameFs;
    const loadgame = @import("loadgame.zig");
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const mdata = try readTestFile(gpa, "testdata/hero_route/map0.map");
    defer gpa.free(mdata);
    const gdata = try readTestFile(gpa, "testdata/hero_route/game0.sav");
    defer gpa.free(gdata);
    var g = try loadgame.fromBytes(gpa, &pr, mdata, gdata);
    defer g.deinit();
    const s = &g.shard;
    var r: Rand = .{};
    const h = &s.rulers[1].holdings;
    const bld = try buildings(s, 1);

    if (withAbility(&pr, bld, abil.recruit, 0)) |e| {
        const free = std.mem.indexOfScalar(i32, &h.recruit, 0).?;
        const gold = s.rulers[1].gold;
        const price = innerPrice(.{ .s = s, .pr = &pr, .player = 1 }, e[0]);
        try build(s, &pr, &r, 1, e[0]);
        try tst.expect(bld[@intCast(e[0])]);
        try tst.expectEqual(e[1].p1, h.recruit[free]);
        try tst.expectEqual(gold - price[0], s.rulers[1].gold);
    }
    if (withAbility(&pr, bld, abil.library, null)) |e| {
        const before = std.mem.indexOfScalar(i32, &h.library, 0).?;
        try build(s, &pr, &r, 1, e[0]);
        const after = std.mem.indexOfScalar(i32, &h.library, 0) orelse h.library.len;
        try tst.expect(after > before);
        for (h.library[before..after]) |sp| try tst.expectEqual(e[0], pr.spells[@intCast(sp)].building);
    }
    if (withAbility(&pr, bld, abil.garrison, null)) |e| {
        try build(s, &pr, &r, 1, e[0]);
        const gi = s.provinces[@intCast(s.capitals[1])].garrison;
        const army = @import("army.zig");
        try tst.expectEqual(e[1].p2, army.unitSlots(&s.garrisons.items[@intCast(gi)], @intCast(e[1].p1)));
    }
    if (withAbility(&pr, bld, abil.shop, null)) |e| {
        const before = for (h.shop.items, 0..) |it, i| {
            if (it[0] == 0) break i;
        } else h.shop.items.len;
        try build(s, &pr, &r, 1, e[0]);
        try tst.expect(h.shop.items[before][0] > 0);
    }
}

test "building rules: slots, CanBuild, IsRestricted, planned" {
    const gpa = tst.allocator;
    const GameFs = @import("../platform/fs.zig").GameFs;
    const loadgame = @import("loadgame.zig");
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const mdata = try readTestFile(gpa, "testdata/hero_route/map0.map");
    defer gpa.free(mdata);
    const gdata = try readTestFile(gpa, "testdata/hero_route/game0.sav");
    defer gpa.free(gdata);
    var g = try loadgame.fromBytes(gpa, &pr, mdata, gdata);
    defer g.deinit();
    const s = &g.shard;
    const rules = try Rules.of(s, &pr, 1);
    var out: [n_slots]i32 = undefined;
    rules.slots(&out);
    var shown: usize = 0;
    for (out[2..]) |b| {
        if (b <= 0) continue;
        shown += 1;
        const ib = &pr.inner_buildings[@intCast(b)];
        // A slot shows a building of a quarter slot or its upgrade.
        try tst.expect(ib.slot >= 2 or ib.upgrade > 0);
        // A building that can be built is never restricted.
        if (rules.canBuild(b)) try tst.expect(!rules.isRestricted(b));
    }
    try tst.expect(shown > 0);
    // Something can be built at the start: BeginMainTurn allows one.
    try resetTurn(s, &pr, 1);
    try tst.expectEqual(@as(i32, 1), s.rulers[1].holdings.can_inner_bld);
    // A building whose needs are its parents: IsParent sees them.
    for (pr.inner_buildings[1..], 1..) |ib, i| {
        const n = ib.needs[0];
        if (n <= 0 or n >= group_need) continue;
        try tst.expectEqual(@as(u2, 1), rules.isParent(n, @intCast(i)));
        break;
    }
}
