//! The orders of the province panel on the map (MainProcess.cpp:539-964,
//! 2689-4087): the lists of outer buildings, defenders, rituals, sites and
//! the active rituals (BuildList, RazeList, HireList, SiteList, RitualList,
//! EnchantList), dcProvince::CanBuild / CanHire (Province.cpp:859, :900),
//! GetOutBldGoldPrice / GetDefenderGoldPrice (Player.cpp:468, :550) and
//! what the buttons do: ConstructBuilding with Pay, RazeBuilding, the
//! hire of a defender (MainProcess.cpp:3642) and DismissDefender.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const mapgen = @import("mapgen.zig");
const Shard = mapgen.Shard;
const province = @import("province.zig");
const Province = province.Province;
const player = @import("player.zig");
const Market = player.Market;
const Price = player.Price;
const movement = @import("movement.zig");
const ritual = @import("ritual.zig");
const event_mod = @import("event.zig");

/// BAR_SIZE: the rows of a list.
pub const bar_size = 6;
const max_bld = province.max_bld;

/// message.txt texts of txtError.
pub const msg = struct {
    pub const no_outer_left = 2;
    pub const nothing_to_build = 3;
    pub const no_place = 4;
    pub const capital = 5;
    pub const no_defenders = 6;
    pub const no_hire_left = 7;
    pub const no_rituals = 8;
    pub const no_cast_left = 9;
    pub const build_siege = 18;
    pub const hire_siege = 19;
    pub const no_dismiss = 26;
};

/// Outer building ability 1: the building cannot be razed.
const ab_no_raze = 1;

fn outerOf(pr: *const Proto, b: i32) ?*const proto_mod.OuterBuilding {
    if (b <= 0 or b >= pr.outer_buildings.len) return null;
    return &pr.outer_buildings[@intCast(b)];
}

fn defenderOf(pr: *const Proto, d: i32) ?*const proto_mod.Defender {
    if (d <= 0 or d >= pr.defenders.len) return null;
    return &pr.defenders[@intCast(d)];
}

fn besieged(s: *const Shard, pv: *const Province) bool {
    return movement.siege(s, pv.number) >= 0;
}

/// arTerrain[ipTerrain-1] of a building or a defender.
fn terrainOk(t: [proto_mod.max_terrain]bool, terrain: i32) bool {
    if (terrain < 1 or terrain > proto_mod.max_terrain) return false;
    return t[@intCast(terrain - 1)];
}

/// dcProvince::CanBuild.
pub fn canBuild(s: *const Shard, pr: *const Proto, pv: *const Province, b: i32) bool {
    if (besieged(s, pv)) return false;
    const bl = outerOf(pr, b) orelse return false;
    if (!terrainOk(bl.terrain, pv.terrain)) return false;
    if (bl.coastal == 1 and !pv.coastal) return false;
    if (bl.coastal == 2 and pv.coastal) return false;
    var n: usize = 0;
    var out = true;
    for (pv.buildings[0..max_bld]) |have| {
        if (have == b) return false;
        var up = if (outerOf(pr, have)) |x| x.upgrade else 0;
        var guard: usize = 0;
        while (up > 0 and guard < pr.outer_buildings.len) : (guard += 1) {
            if (b == up) return false;
            up = if (outerOf(pr, up)) |x| x.upgrade else 0;
        }
        if (have > 0) {
            if (bl.upgrade == have) out = false;
            n += 1;
        }
    }
    if (n == max_bld and bl.upgrade == 0) return false;
    if (bl.upgrade > 0 and out) return false;
    return true;
}

/// dcProvince::CanHire.
pub fn canHire(s: *const Shard, pr: *const Proto, pv: *const Province, d: i32) bool {
    if (besieged(s, pv)) return false;
    const df = defenderOf(pr, d) orelse return false;
    if (!terrainOk(df.terrain, pv.terrain)) return false;
    if (defenderOf(pr, pv.defender)) |cur| if (cur.no_dismiss) return false;
    return true;
}

/// GetOutBldGoldPrice / GetOutBldGemPrice.
pub fn outerPrice(m: Market, b: i32) Price {
    const bl = outerOf(m.pr, b) orelse return .{ 0, 0 };
    return m.withRes(.{ bl.gold_cost, bl.gem_cost }, &bl.res);
}

/// GetDefenderGoldPrice / GetDefenderGemPrice.
pub fn defenderPrice(m: Market, d: i32) Price {
    const df = defenderOf(m.pr, d) orelse return .{ 0, 0 };
    return m.withRes(.{ df.gold_cost, df.gem_cost }, &df.res);
}

/// A list of the panel: the entries from row `shift`, their count, or the
/// txtError message shown instead.
pub const List = struct {
    items: [bar_size]i32 = @splat(0),
    /// arBuildNum: the site slot of each row (SiteList).
    slots: [bar_size]usize = @splat(0),
    /// Timer of each row (EnchantList).
    timers: [bar_size]i32 = @splat(0),
    /// MaxBld: all the entries, not only the shown ones.
    total: usize = 0,
    err: ?usize = null,

    pub fn shown(self: *const List) []const i32 {
        return self.items[0..@min(self.total, bar_size)];
    }

    fn add(self: *List, shift: usize, v: i32, slot: usize, timer: i32) void {
        if (self.total >= shift and self.total < shift + bar_size) {
            self.items[self.total - shift] = v;
            self.slots[self.total - shift] = slot;
            self.timers[self.total - shift] = timer;
        }
        self.total += 1;
    }
};

/// BuildList.
pub fn buildList(m: Market, prov: i32, shift: usize) List {
    const s = m.s;
    const pr = m.pr;
    const pv = &s.provinces[@intCast(prov)];
    const k: usize = @intCast(m.player);
    const h = m.holdings();
    var l: List = .{};
    if (s.capitals[k] == prov) return .{ .err = msg.capital };
    if (h.can_outer_bld <= 0) return .{ .err = msg.no_outer_left };
    if (besieged(s, pv)) return .{ .err = msg.build_siege };
    for (1..pr.outer_buildings.len) |i| {
        const b: i32 = @intCast(i);
        if (ritual.listStock(s, pr, k, .outer, b) == 0) continue;
        if (h.bld_filter and !canBuild(s, pr, pv, b)) continue;
        l.add(shift, b, 0, 0);
    }
    if (l.total == 0) {
        const space = std.mem.indexOfScalar(i32, pv.buildings[0..max_bld], 0) != null;
        l.err = if (space) msg.nothing_to_build else msg.no_place;
    }
    return l;
}

/// RazeList: the buildings of the province.
pub fn razeList(pv: *const Province, shift: usize) List {
    var l: List = .{};
    for (pv.buildings[0..max_bld], 0..) |b, slot| {
        if (b > 0) l.add(shift, b, slot, 0);
    }
    return l;
}

/// HireList.
pub fn hireList(m: Market, prov: i32, shift: usize) List {
    const s = m.s;
    const pr = m.pr;
    const pv = &s.provinces[@intCast(prov)];
    const k: usize = @intCast(m.player);
    const h = m.holdings();
    var l: List = .{};
    if (h.can_hire <= 0) return .{ .err = msg.no_hire_left };
    if (besieged(s, pv)) return .{ .err = msg.hire_siege };
    if (defenderOf(pr, pv.defender)) |cur| if (cur.no_dismiss) return .{ .err = msg.no_dismiss };
    for (1..pr.defenders.len) |i| {
        const d: i32 = @intCast(i);
        if (ritual.listStock(s, pr, k, .defender, d) == 0) continue;
        if (h.def_filter and !canHire(s, pr, pv, d)) continue;
        l.add(shift, d, 0, 0);
    }
    if (l.total == 0) l.err = msg.no_defenders;
    return l;
}

/// SiteList(Province, Shift, Target, false, NULL): the explored sites.
pub fn siteList(pr: *const Proto, pv: *const Province, shift: usize) List {
    var l: List = .{};
    for (pv.sites, 0..) |st, slot| {
        if (st.index <= 0 or st.index >= pr.sites.len or st.pos > pv.explore) continue;
        l.add(shift, st.index, slot, 0);
    }
    return l;
}

/// RitualList.
pub fn ritualList(m: Market, prov: i32, shift: usize) List {
    const s = m.s;
    const pr = m.pr;
    const k: usize = @intCast(m.player);
    const h = m.holdings();
    var l: List = .{};
    if (h.can_cast <= 0) return .{ .err = msg.no_cast_left };
    for (1..pr.rituals.len) |i| {
        const r: i32 = @intCast(i);
        if (ritual.stock(s, pr, k, r) == 0) continue;
        if (h.rit_filter and !ritual.canCast(s, pr, prov, r, m.player)) continue;
        l.add(shift, r, 0, 0);
    }
    if (l.total == 0) l.err = msg.no_rituals;
    return l;
}

/// One dcRitual of the saved pRitual list (Unit.h:514): Index, ipPlayer,
/// ipProvince, Timer, iEvent, pNext.
const enchant_size = 24;

/// EnchantList: the rituals of player `pl` still working on `prov`.  The
/// list is the one read from the file (LoadGame puts each record before
/// the previous ones); the timed rituals cast since are not ported.
pub fn enchantList(s: *const Shard, prov: i32, pl: i32, shift: usize) List {
    var l: List = .{};
    const sv = s.saved orelse return l;
    const t = sv.tail;
    if (t.len < 4) return l;
    const n: usize = @intCast(@max(std.mem.readInt(i32, t[0..4], .little), 0));
    if (t.len < 4 + n * enchant_size) return l;
    var i = n;
    while (i > 0) {
        i -= 1;
        const r = t[4 + i * enchant_size ..][0..enchant_size];
        const idx = std.mem.readInt(i32, r[0..4], .little);
        const who = std.mem.readInt(i32, r[4..8], .little);
        const where = std.mem.readInt(i32, r[8..12], .little);
        const timer = std.mem.readInt(i32, r[12..16], .little);
        if (where == prov and who == pl and timer > 0) l.add(shift, idx, 0, timer);
    }
    return l;
}

/// The province has an active ritual of `pl` (objEnchant on the panel).
pub fn enchanted(s: *const Shard, prov: i32, pl: i32) bool {
    return enchantList(s, prov, pl, 0).total > 0;
}

/// buttBuild2 can be pressed: CanBuild and the price.
pub fn canAffordBuild(m: Market, prov: i32, b: i32) bool {
    return canBuild(m.s, m.pr, &m.s.provinces[@intCast(prov)], b) and m.affords(outerPrice(m, b));
}

/// buttGuard2 can be pressed.
pub fn canAffordHire(m: Market, prov: i32, d: i32) bool {
    return canHire(m.s, m.pr, &m.s.provinces[@intCast(prov)], d) and m.affords(defenderPrice(m, d));
}

/// buttRaze is active: no ability 1.
pub fn canRaze(pr: *const Proto, b: i32) bool {
    const bl = outerOf(pr, b) orelse return false;
    for (bl.abilities[0..bl.n_abilities]) |a| if (a.id == ab_no_raze) return false;
    return true;
}

/// ConstructBuilding(Choosen, true, true): the building, then iCanOuterBld,
/// the stock, the price and the resource prices.
pub fn construct(m: Market, run: *event_mod.Runner, prov: i32, b: i32) !void {
    const pv = &m.s.provinces[@intCast(prov)];
    const bl = outerOf(m.pr, b) orelse return error.BadBuilding;
    const cost = outerPrice(m, b);
    try run.construct(pv, b);
    const k: usize = @intCast(m.player);
    m.holdings().can_outer_bld -= 1;
    const st = ritual.listStock(m.s, m.pr, k, .outer, b);
    if (st > 0) ritual.setListStock(m.s, m.pr, k, .outer, b, st - 1);
    m.pay(cost);
    m.raiseResPrices(&bl.res);
}

/// RazeBuilding of the slot holding `b`.
pub fn raze(run: *event_mod.Runner, pv: *Province, b: i32) void {
    const slot = std.mem.indexOfScalar(i32, pv.buildings[0..max_bld], b) orelse return;
    run.raze(pv, slot);
}

/// The hire of defender `d` (MainProcess.cpp:3652): the karma of the old
/// and the new one, iCanHire, PlaceDefender, the stock, the price and the
/// resource prices.
pub fn hire(m: Market, run: *event_mod.Runner, prov: i32, d: i32) !void {
    const pr = m.pr;
    const pv = &m.s.provinces[@intCast(prov)];
    const df = defenderOf(pr, d) orelse return error.BadDefender;
    const cost = defenderPrice(m, d);
    const r = m.ruler();
    if (defenderOf(pr, pv.defender)) |old| r.karma -= old.karma;
    r.karma += df.karma;
    m.holdings().can_hire -= 1;
    try run.placeDefender(pv, d);
    const k: usize = @intCast(m.player);
    const st = ritual.listStock(m.s, pr, k, .defender, d);
    if (st > 0) ritual.setListStock(m.s, pr, k, .defender, d, st - 1);
    m.pay(cost);
    m.raiseResPrices(&df.res);
}

/// buttDismiss: the karma back and DismissDefender.
pub fn dismiss(m: Market, pv: *Province) void {
    if (defenderOf(m.pr, pv.defender)) |df| m.ruler().karma -= df.karma;
    pv.guard = -1;
    pv.defender = 0;
}

/// The capital of a player in the game in `prov` (HomeCastle), 0 for none.
pub fn homeCastle(s: *const Shard, prov: i32) usize {
    var home: usize = 0;
    for (1..s.n_players + 1) |k| {
        if (s.in_game[k] and s.capitals[k] == prov) home = k;
    }
    return home;
}

/// The fort of the province (Condition 1 and 11): the castle of a home
/// province or the outer building with walls; null for none.
pub const Fort = struct {
    /// 100·iFort / wall.
    percent: i32,
    towers: i32,
    /// The picture: innerbld.dat 149 for a castle, the outer building's
    /// icon otherwise.
    building: i32,
    castle: bool,
};

pub fn fort(s: *const Shard, pr: *const Proto, pv: *const Province) ?Fort {
    const home = homeCastle(s, pv.number);
    if (home > 0) {
        const r = &s.rulers[home];
        return .{ .percent = @divTrunc(100 * pv.fort, @max(r.wall, 1)), .towers = r.holdings.tower, .building = 0, .castle = true };
    }
    var out: ?Fort = null;
    for (pv.buildings[0..max_bld]) |b| {
        const bl = outerOf(pr, b) orelse continue;
        if (bl.wall > 0) out = .{ .percent = @divTrunc(100 * pv.fort, bl.wall), .towers = bl.tower_type, .building = b, .castle = false };
    }
    return out;
}

/// The guard's life in percent of its maximum (the defender's icon).
pub fn guardLife(s: *const Shard, pr: *const Proto, pv: *const Province) i32 {
    if (pv.guard < 0) return 0;
    var cur: i32 = 0;
    var max: i32 = 0;
    for (s.guards.items[@intCast(pv.guard)].squad) |slot| {
        const u = slot orelse continue;
        max += u.maxLife(pr);
        cur += u.life;
    }
    return if (max > 0) @divTrunc(100 * cur, max) else 0;
}

const tst = std.testing;

test "the list keeps the page and counts everything" {
    var l: List = .{};
    for (0..10) |i| l.add(3, @intCast(i + 1), i, 0);
    try tst.expectEqual(@as(usize, 10), l.total);
    try tst.expectEqualSlices(i32, &.{ 4, 5, 6, 7, 8, 9 }, l.shown());
}

test "enchantments from the saved list, newest first" {
    var s: Shard = undefined;
    var tail: [4 + 3 * enchant_size]u8 = @splat(0);
    std.mem.writeInt(i32, tail[0..4], 3, .little);
    for ([_][4]i32{ .{ 5, 1, 7, 2 }, .{ 6, 2, 7, 3 }, .{ 8, 1, 7, 4 } }, 0..) |r, i| {
        const at = 4 + i * enchant_size;
        for (r, 0..) |v, j| std.mem.writeInt(i32, tail[at + 4 * j ..][0..4], v, .little);
    }
    s.saved = .{ .header = &.{}, .world_rules = &.{}, .players = &.{}, .tail = &tail };
    const l = enchantList(&s, 7, 1, 0);
    try tst.expectEqualSlices(i32, &.{ 8, 5 }, l.shown());
    try tst.expectEqual(@as(i32, 4), l.timers[0]);
    try tst.expect(!enchanted(&s, 8, 1));
}
