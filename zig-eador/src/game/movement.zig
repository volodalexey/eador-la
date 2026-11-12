//! Hero movement on the strategic map (MainProcess.cpp:28-430):
//! MoveMap fills the turns needed to reach each province, MovePath finds
//! the route to a target over that map, and the movement part of
//! EndMainTurn (MainProcess.cpp:7281) moves the heroes with an order.
//! The original's globals (MovePlayer, WalkType, WalkSpeed, GoPath,
//! MinLength, LastMoves, PathKicker) are the fields of `Walker`.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const mapgen = @import("mapgen.zig");
const Shard = mapgen.Shard;
const province = @import("province.zig");
const terrain = province.terrain;
const hero_mod = @import("hero.zig");
const Hero = hero_mod.Hero;

/// Building abilities: 2 stable (outer), 10 stable and 44 port (castle),
/// 37 hero mobility (castle).
const abil_stable = 2;
const abil_castle_stable = 10;
const abil_castle_port = 44;
pub const abil_mobility = 37;
pub const abil_siege = 42;
/// WalkType of a hero that sails (Pathfinding never reaches it).
const walk_sail = 10;

/// A node of GoPath: the province and the turn it is reached on.
pub const Step = struct { province: i32, turn: i32 };

/// GetBldParam1 (`inclusive`, i <= iMaxInnerBld) or the loops of GetPort
/// and GetStable (i < iMaxInnerBld): ability `ability` over the castle
/// buildings of `player`.
pub fn castleParam(s: *const Shard, pr: *const Proto, player: i32, ability: i32, inclusive: bool) i32 {
    if (player <= 0 or player > mapgen.max_players) return 0;
    const bld = s.castle[@intCast(player)];
    const n = pr.inner_buildings.len;
    if (n < 2) return 0;
    const last = if (inclusive) n - 1 else n - 2;
    var r: i32 = 0;
    for (1..last + 1) |i| {
        if (i >= bld.len or !bld[i]) continue;
        const b = &pr.inner_buildings[i];
        for (b.abilities[0..b.n_abilities]) |a| {
            if (a.id == ability) r += a.p1;
        }
    }
    return r;
}

/// dcProvince::CheckBldAbil: ability `ability` over the outer buildings.
pub fn outerParam(pr: *const Proto, pv: *const province.Province, ability: i32) i32 {
    var r: i32 = 0;
    for (pv.buildings[0..province.max_bld]) |b| {
        if (b <= 0 or b >= pr.outer_buildings.len) continue;
        const ob = &pr.outer_buildings[@intCast(b)];
        for (ob.abilities[0..ob.n_abilities]) |a| {
            if (a.id == ability) r += a.p1;
        }
    }
    return r;
}

fn isHome(s: *const Shard, pv: *const province.Province) bool {
    return pv.player >= 0 and pv.player <= mapgen.max_players and s.capitals[@intCast(pv.player)] == pv.number;
}

/// dcProvince::GetPort.
pub fn port(s: *const Shard, pr: *const Proto, p: i32) i32 {
    const pv = &s.provinces[@intCast(p)];
    if (isHome(s, pv)) return castleParam(s, pr, pv.player, abil_castle_port, false);
    var r: i32 = 0;
    for (pv.buildings[0..province.max_bld]) |b| {
        if (b <= 0 or b >= pr.outer_buildings.len) continue;
        r = @max(r, pr.outer_buildings[@intCast(b)].port);
    }
    return r;
}

/// dcProvince::GetStable.
pub fn stable(s: *const Shard, pr: *const Proto, p: i32) i32 {
    const pv = &s.provinces[@intCast(p)];
    if (isHome(s, pv)) return castleParam(s, pr, pv.player, abil_castle_stable, false);
    return outerParam(pr, pv, abil_stable);
}

/// dcProvince::GetSiegeIndex: 0 when a foreign guard holds the province,
/// the player whose hero stands in it, -1 otherwise.
pub fn siege(s: *const Shard, p: i32) i32 {
    const pv = &s.provinces[@intCast(p)];
    if (pv.guard >= 0 and s.guards.items[@intCast(pv.guard)].player != pv.player) return 0;
    for (1..s.n_players + 1) |pl| {
        const pi: i32 = @intCast(pl);
        if (pi == pv.player or !s.in_game[pl]) continue;
        for (s.heroes.items) |*h| {
            if (h.owner == pi and h.province == p and h.alive()) return pi;
        }
    }
    return -1;
}

pub const Walker = struct {
    s: *const Shard,
    pr: *const Proto,
    /// MAX_PATH (eador.cfg).
    max_path: i32,
    /// MovePlayer, WalkType, WalkSpeed.
    player: i32 = 0,
    walk_type: i32 = 1,
    walk_speed: i32 = 1,
    /// pMoveMap (turns, -1 unreached), pRunMap, pMobilMap by province.
    move: []i32,
    run: []i32,
    mobil: []i32,
    /// The route being walked (the clPath chain) and GoPath.
    road: std.ArrayList(Step) = .empty,
    path: std.ArrayList(Step) = .empty,
    found: bool = false,
    min_length: i32 = 0,
    last_moves: i32 = 0,
    kicker: i32 = 0,

    pub fn init(gpa: std.mem.Allocator, s: *const Shard, pr: *const Proto, max_path: i32) !Walker {
        const n = s.provinces.len;
        const move = try gpa.alloc(i32, n);
        errdefer gpa.free(move);
        const run = try gpa.alloc(i32, n);
        errdefer gpa.free(run);
        const mobil = try gpa.alloc(i32, n);
        var w: Walker = .{ .s = s, .pr = pr, .max_path = max_path, .move = move, .run = run, .mobil = mobil };
        w.clear();
        return w;
    }

    pub fn deinit(self: *Walker, gpa: std.mem.Allocator) void {
        self.road.deinit(gpa);
        self.path.deinit(gpa);
        gpa.free(self.move);
        gpa.free(self.run);
        gpa.free(self.mobil);
    }

    /// ClearMoveMap.
    pub fn clear(self: *Walker) void {
        @memset(self.move, -1);
        @memset(self.run, -1);
        @memset(self.mobil, -100);
    }

    /// ClearMoveMap, MovePlayer, WalkType, WalkSpeed and MoveMap from the
    /// hero's province, as MainScreen (Condition 20) and EndMainTurn do.
    pub fn forHero(self: *Walker, h: *const Hero) void {
        self.clear();
        self.player = h.owner;
        self.walk_type = 1 + h.checkSkill(hero_mod.skill_pathfinding);
        self.walk_speed = h.mobility(self.pr, castleParam(self.s, self.pr, h.player, abil_mobility, true));
        self.moveMap(h.province, -1, self.walk_speed, 0, 0);
    }

    fn prov(self: *const Walker, p: i32) *const province.Province {
        return &self.s.provinces[@intCast(p)];
    }

    /// GetMoveCost.
    fn cost(self: *const Walker, p: i32) i32 {
        const t = self.prov(p).terrain;
        const r = self.pr.terrains[@intCast(t)].bf_pass;
        if (r <= 1) return 1;
        if (t <= self.walk_type) return 1;
        if (stable(self.s, self.pr, p) > 0) return 1;
        return r;
    }

    /// The march stops in a land province that is not the mover's or is
    /// besieged.
    fn blocks(self: *const Walker, p: i32) bool {
        const pv = self.prov(p);
        return self.player > 0 and terrain.land(pv.terrain) and (pv.player != self.player or siege(self.s, p) >= 0);
    }

    /// MoveMap (MainProcess.cpp:72).  Weight < 0 starts from `p`.
    pub fn moveMap(self: *Walker, p: i32, weight_in: i32, moves_in: i32, port_in: i32, add_in: i32) void {
        var weight = weight_in;
        var moves = moves_in;
        var prt = port_in;
        var add_moves = add_in;
        var first = false;
        if (weight > self.max_path) return;
        const pv = self.prov(p);
        const i: usize = @intCast(p);
        if (weight < 0) {
            first = true;
            weight = 1;
            self.move[i] = weight;
            if (self.walk_type < walk_sail) moves += stable(self.s, self.pr, p);
            add_moves = 0;
            if (self.player > 0 and pv.player != self.player) moves = 1;
            self.mobil[i] = moves;
        } else {
            var swim = false;
            if (pv.terrain == terrain.water and prt > 0) {
                self.mobil[i] = prt;
                prt -= 1;
                swim = true;
            } else if (moves <= 0) {
                weight += 1;
                moves = self.walk_speed - self.cost(p) + add_moves;
                self.mobil[i] = moves;
                add_moves = 0;
            } else {
                self.mobil[i] = moves;
                add_moves = 0;
                moves -= self.cost(p);
                if (moves <= 0 and self.walk_type < walk_sail) add_moves = stable(self.s, self.pr, p);
            }
            const left = if (swim) prt else moves;
            if (self.move[i] == weight) {
                if (self.run[i] < left) self.run[i] = left else return;
            }
            if (self.move[i] < 0 or self.move[i] > weight) {
                self.move[i] = weight;
                self.run[i] = left;
            }
            if (self.blocks(p)) return;
        }
        if (pv.terrain > 0) {
            prt = port(self.s, self.pr, p);
            if (self.walk_type == walk_sail and prt < moves) prt = 0;
        }
        for (pv.neighbours) |nb| {
            if (nb <= 0) continue;
            const pp = self.prov(nb);
            const n: usize = @intCast(nb);
            var go = true;
            if (pp.terrain == terrain.water and prt < 1 and self.walk_type < walk_sail) go = false;
            if (pp.terrain == terrain.mountain and (self.walk_type < walk_sail or moves < 2)) go = false;
            const embark = prt > 0 and pp.terrain == terrain.water and pv.terrain > 0;
            var add = weight;
            if (moves <= 0 or (embark and moves < self.walk_speed)) add += 1;
            if (self.move[n] >= 0 and self.move[n] < add) go = false;
            add = if (prt > 0 and pp.terrain == terrain.water) prt else moves;
            if (self.move[n] <= weight and self.mobil[n] >= add) go = false;
            // No attack out of a province besieged or held by another.
            if (go and self.player > 0 and pv.player != self.player and pp.player != self.player and pv.terrain > 0 and pv.terrain != terrain.mountain) go = false;
            if (!go) continue;
            if (embark) {
                self.moveMap(nb, if (first) weight else weight + 1, 1, prt, 0);
            } else self.moveMap(nb, weight, moves, prt, add_moves);
        }
    }

    /// MovePath from `from` to `target` over the move map, retried with
    /// PathKicker = 1 when nothing is found.  The route is in `path`
    /// (empty when unreachable), from the start to the target.
    pub fn route(self: *Walker, gpa: std.mem.Allocator, from: i32, target: i32) !void {
        self.path.clearRetainingCapacity();
        self.found = false;
        for ([_]i32{ 0, 1 }) |k| {
            self.kicker = k;
            self.road.clearRetainingCapacity();
            try self.movePath(gpa, from, 1, false, target, self.walk_speed, 0, 0, 0);
            if (self.found) return;
        }
    }

    /// MovePath (MainProcess.cpp:247).  `has_path` is Path != NULL.
    fn movePath(self: *Walker, gpa: std.mem.Allocator, p: i32, weight_in: i32, has_path: bool, target: i32, moves_in: i32, port_in: i32, length_in: i32, add_in: i32) !void {
        var weight = weight_in;
        var moves = moves_in;
        var prt = port_in;
        var length = length_in;
        var add_moves = add_in;
        if (self.found and length >= self.min_length) return;
        const pv = self.prov(p);
        if (has_path) {
            if (pv.terrain == terrain.water and prt > 0) {
                prt -= 1;
            } else if (moves <= 0) {
                weight += 1;
                moves = self.walk_speed - self.cost(p) + add_moves;
                add_moves = 0;
            } else {
                add_moves = 0;
                moves -= self.cost(p);
                if (moves <= 0 and self.walk_type < walk_sail) add_moves = stable(self.s, self.pr, p);
            }
            if (weight > self.move[@intCast(target)]) return;
        } else {
            self.last_moves = 0;
            if (self.walk_type < walk_sail) moves += stable(self.s, self.pr, p);
            add_moves = 0;
            if (self.player > 0 and pv.player != self.player) moves = 1;
        }
        try self.road.append(gpa, .{ .province = p, .turn = weight });
        defer _ = self.road.pop();
        length += 1;
        if (p == target) {
            if (self.found and !(length < self.min_length or (length == self.min_length and moves > self.last_moves))) return;
            self.path.clearRetainingCapacity();
            try self.path.appendSlice(gpa, self.road.items);
            self.found = true;
            self.min_length = length;
            self.last_moves = moves;
            return;
        }
        if (has_path and self.blocks(p)) return;
        if (pv.terrain > 0) {
            prt = port(self.s, self.pr, p);
            if (self.walk_type == walk_sail and prt < moves) prt = 0;
        }
        for (pv.neighbours) |nb| {
            if (nb <= 0) continue;
            const pp = self.prov(nb);
            const n: usize = @intCast(nb);
            var go = true;
            if (pp.terrain == terrain.water and prt < 1 and self.walk_type < walk_sail) go = false;
            if (pp.terrain == terrain.mountain and (self.walk_type < walk_sail or moves < 2)) go = false;
            var add = weight;
            if (self.kicker == 0) {
                if (self.move[n] >= 0 and self.move[n] < @max(add - self.kicker, 0)) go = false;
                add = if (prt > 0 and pp.terrain == terrain.water) prt else moves;
                if (weight == self.move[n] and self.mobil[n] > add) go = false;
            } else {
                var checker = false;
                if (self.move[n] >= 0 and self.move[n] < add and moves < self.walk_speed) go = false;
                if (self.move[n] >= 0 and self.move[n] < add) checker = true;
                add = if (prt > 0 and pp.terrain == terrain.water) prt else moves;
                if (weight >= self.move[n] and self.mobil[n] > add and checker) go = false;
            }
            if (go and self.player > 0 and pv.player != self.player and pp.player != self.player and pv.terrain > 0 and pv.terrain != terrain.mountain) go = false;
            if (!go) continue;
            if (prt > 0 and pp.terrain == terrain.water and pv.terrain > 0) {
                try self.movePath(gpa, nb, if (has_path) weight + 1 else weight, true, target, 1, prt, length, 0);
            } else try self.movePath(gpa, nb, weight, true, target, moves, prt, length, add_moves);
        }
    }
};

/// What happened to a hero with a "go" order at the end of the turn.
pub const Outcome = enum {
    /// No route; the order stays.
    none,
    /// The hero moved (and the order ended if the target was reached).
    moved,
    /// The way leads into a province that is not the hero's player's:
    /// the hero waits at the last own province of this turn for the
    /// battle event (ipEvent 1); the battle is not ported.
    attack,
};

/// The "Движение героя" part of EndMainTurn for one hero
/// (MainProcess.cpp:7281).  The blizzard check needs the rituals and is
/// not ported.
pub fn endTurnMove(gpa: std.mem.Allocator, w: *Walker, s: *Shard, k: usize) !Outcome {
    const h = &s.heroes.items[k];
    defer h.move_bonus = 0;
    if (!h.alive() or h.task != hero_mod.Task.go) return .none;
    w.forHero(h);
    try w.route(gpa, h.province, h.target);
    if (w.path.items.len == 0) return .none;
    var comes: i32 = 0;
    for (w.path.items) |st| {
        if (st.turn == 1 and terrain.land(s.provinces[@intCast(st.province)].terrain)) comes = st.province;
    }
    const pv = &s.provinces[@intCast(comes)];
    const sg = siege(s, comes);
    if ((pv.player == h.owner and sg < 0) or sg == h.owner) {
        h.event = 0;
        h.province = comes;
        if (h.target == comes) {
            h.task = hero_mod.Task.none;
            h.target = 0;
        }
        return .moved;
    }
    var from: i32 = 0;
    h.move_prov = h.province;
    h.task = hero_mod.Task.none;
    for (w.path.items) |st| {
        const q = &s.provinces[@intCast(st.province)];
        if (st.turn != 1 or !terrain.land(q.terrain) or q.player != h.owner) continue;
        const qs = siege(s, st.province);
        if (qs < 0 or qs == h.owner) from = st.province;
    }
    if (from > 0) h.province = from;
    h.event = 1;
    return .attack;
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

test "route of the hired hero to the neighbouring province" {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const mdata = try readTestFile(gpa, "testdata/hero_move/map0.map");
    defer gpa.free(mdata);
    const gdata = try readTestFile(gpa, "testdata/hero_move/game0.sav");
    defer gpa.free(gdata);
    var g = try loadgame.fromBytes(gpa, &pr, mdata, gdata);
    defer g.deinit();
    var w = try Walker.init(gpa, &g.shard, &pr, 20);
    defer w.deinit(gpa);
    const h = &g.shard.heroes.items[0];
    w.forHero(h);
    try tst.expectEqual(@as(i32, 1), w.move[25]);
    // A neutral neighbour is reached but the march stops there.
    try tst.expect(w.move[35] >= 1);
    try w.route(gpa, h.province, h.target);
    try tst.expectEqual(@as(usize, 2), w.path.items.len);
    try tst.expectEqual(Step{ .province = 25, .turn = 1 }, w.path.items[0]);
    try tst.expectEqual(@as(i32, 35), w.path.items[1].province);
    // Every province reached is the start or touches a reached one.
    for (g.shard.provinces[1..], 1..) |*pv, i| {
        if (w.move[i] < 0 or i == 25) continue;
        const near = for (pv.neighbourList()) |nb| {
            if (w.move[@intCast(nb)] >= 0) break true;
        } else false;
        try tst.expect(near);
    }
    // End of turn: 35 is neutral, so the hero stays for the battle.
    try tst.expectEqual(Outcome.attack, try endTurnMove(gpa, &w, &g.shard, 0));
    try tst.expectEqual(@as(i32, 25), h.province);
    try tst.expectEqual(@as(i32, 1), h.event);
    try tst.expectEqual(@as(i32, hero_mod.Task.none), h.task);
    try tst.expectEqual(@as(i32, 35), h.target);
}

// testdata/hero_route: turn 2, the hero in province 35 (taken) ordered to
// province 15, two provinces away through the capital 25.
test "route through the capital to a neutral province" {
    const gpa = tst.allocator;
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
    var w = try Walker.init(gpa, &g.shard, &pr, 20);
    defer w.deinit(gpa);
    const h = &g.shard.heroes.items[0];
    try tst.expectEqual(@as(i32, 35), h.province);
    try tst.expectEqual(@as(i32, 15), h.target);
    w.forHero(h);
    try w.route(gpa, h.province, h.target);
    // Mobility 2, forest costs 2: 25 on the first turn, 15 on the second.
    try tst.expectEqual(@as(i32, 2), w.walk_speed);
    try tst.expectEqualSlices(Step, &.{
        .{ .province = 35, .turn = 1 },
        .{ .province = 25, .turn = 1 },
        .{ .province = 15, .turn = 2 },
    }, w.path.items);
    // End of turn: the last land step of turn 1 is the own capital, so the
    // hero walks there and keeps the order.
    try tst.expectEqual(Outcome.moved, try endTurnMove(gpa, &w, &g.shard, 0));
    try tst.expectEqual(@as(i32, 25), h.province);
    try tst.expectEqual(@as(i32, hero_mod.Task.go), h.task);
    try tst.expectEqual(@as(i32, 15), h.target);
    try tst.expectEqual(@as(i32, 0), h.event);

    // The original after one "End turn" from this save (turn 3).
    const ndata = try readTestFile(gpa, "testdata/hero_route/game0_turn3.sav");
    defer gpa.free(ndata);
    var next = try loadgame.fromBytes(gpa, &pr, mdata, ndata);
    defer next.deinit();
    try tst.expectEqual(@as(i32, 3), next.turn);
    const want = &next.shard.heroes.items[0];
    try tst.expectEqual(want.province, h.province);
    try tst.expectEqual(want.task, h.task);
    try tst.expectEqual(want.target, h.target);
    try tst.expectEqual(want.event, h.event);
    try tst.expectEqual(want.move_prov, h.move_prov);
    try tst.expectEqual(want.move_bonus, h.move_bonus);
}
