//! Province events: dcProvince::StaticEvent (Province.cpp:2458),
//! GetEventPoss (:1105), Rebel and clEvent::CalcEffect (Event.cpp:2427,
//! :2517), and the event part of BeginMainTurn: the timed events of pStory
//! (MainProcess.cpp:8177) and the random event of a province
//! (MainProcess.cpp:8646).
//!
//! The original shows the dialog of an event in the middle of the turn and
//! goes on with the answer.  Here BeginMainTurn queues the events as jobs;
//! the map screen asks the Runner for the next dialog (`next`), shows it and
//! hands the answer back (`answered`).  What a StaticEvent still has to do
//! after its dialog (the attack, the next event of effect 11, the event of
//! a battle result) waits on a stack of frames.
//!
//! Not ported: the battle of an attack or a rebellion when the province has
//! a defender or heroes (the defenders hold, result 0), rituals
//! (GetRitualBonus is 0), the world rules (43, the world events), the
//! campaign effects (33, 34, 35, 37) and condition 26, Victory (46), the
//! computer players' answers (BeginMainTurn gives them no StaticEvent), the
//! unrest branch of BeginMainTurn.
const std = @import("std");
const rand_mod = @import("../data/rand.zig");
const Rand = rand_mod.Rand;
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const mapgen = @import("mapgen.zig");
const Shard = mapgen.Shard;
const province = @import("province.zig");
const Province = province.Province;
const economy = @import("economy.zig");
const Economy = economy.Economy;
const site_mod = @import("site.zig");
const guard_mod = @import("guard.zig");
const Guard = guard_mod.Guard;
const hero_mod = @import("hero.zig");
const unit_mod = @import("unit.zig");
const Unit = unit_mod.Unit;
const Roller = unit_mod.Roller;
const army = @import("army.zig");
const player_mod = @import("player.zig");
const newsave = @import("newsave.zig");
const movement = @import("movement.zig");

/// MAX_UNREST, MAX_PROVINCE_BLD, TREASURY_SIZE.
pub const max_unrest = 500;
const max_prov_bld = province.max_bld;
const treasury_size = player_mod.treasury_size;
/// A job stops after this many StaticEvent calls (a loop in the data).
const max_steps = 200;

/// clEvent::arEffect[i][0].
const eff = struct {
    const gold = 1;
    const attitude = 2;
    const population = 3;
    const answer = 4;
    const gold_inc = 5;
    const gem = 6;
    const karma = 7;
    const condition = 8;
    const site = 9;
    const unrest = 10;
    const next = 11;
    const outer_stock = 12;
    const scroll = 13;
    const item_roll = 14;
    const attack_level = 15;
    const item = 16;
    const story = 17;
    const place = 18;
    const defender = 19;
    const raze = 20;
    const construct = 21;
    const corrupt = 22;
    const gem_inc = 23;
    const remove_site = 24;
    const raze_count = 25;
    const race_attitude = 26;
    const res_price = 27;
    const life = 29;
    const morale = 30;
    const rebel = 31;
    const fort = 32;
    const camp_event = 37;
    const guard_gold = 38;
    const neutral = 39;
    const shop_item = 40;
    const take_item = 41;
    const defender_stock = 42;
    const if_resource = 44;
    const guarded_site = 45;
    const victory = 46;
};

/// Building and item abilities used here.
const ab = struct {
    const outer_garrison = 4;
    const outer_site = 12;
    const outer_event = 7;
    const castle_guard_gold = 33;
    const castle_event = 45;
    const castle_karma = 38;
    const castle_defender_exp = 20;
    const item_defender = 86;
    const item_outer = 87;
    const no_morale = 19;
};

/// Unit kinds ChangeLife spares when Targets is set.
const kind_undead = 2;
const kind_no_life = 4;

/// sprintf argument.
pub const Arg = union(enum) { s: []const u8, d: i64 };

/// sprintf(buf, fmt, ...) for the %s, %d and %i of the dialogs.
pub fn format(buf: []u8, fmt: []const u8, args: []const Arg) []const u8 {
    var w: std.Io.Writer = .fixed(buf);
    var k: usize = 0;
    var n: usize = 0;
    while (k < fmt.len) {
        if (fmt[k] == '%' and k + 1 < fmt.len) {
            switch (fmt[k + 1]) {
                '%' => {
                    w.writeByte('%') catch break;
                    k += 2;
                    continue;
                },
                's', 'd', 'i' => {
                    if (n < args.len) switch (args[n]) {
                        .s => |s| w.writeAll(s) catch break,
                        .d => |d| w.print("{d}", .{d}) catch break,
                    };
                    n += 1;
                    k += 2;
                    continue;
                },
                else => {},
            }
        }
        w.writeByte(fmt[k]) catch break;
        k += 1;
    }
    return w.buffered();
}

/// Treasurez: [0] the gold and gems of the event, [1..5] (item, durability);
/// a negative item is a castle building.
pub const Treasure = [6][2]i32;

/// An event to run: StaticEvent(event, player) of province `prov`.
pub const Job = struct { player: usize, prov: i32, event: i32 };

/// A dialog to show.  `decision` is the dialog of a StaticEvent: its answer
/// picks the next event.  The others (Rebel's) only inform.
pub const Show = struct {
    dialog: usize,
    enabled: [proto_mod.max_answer]bool,
    treasure: Treasure,
    prov: i32,
    /// EventDX: the map is centred 200 px off the province and the dialog
    /// moved as far the other way.
    dx: i32,
    decision: bool,
    buf: [1024]u8 = undefined,
    len: usize = 0,

    pub fn text(self: *const Show) []const u8 {
        return self.buf[0..self.len];
    }
};

const Stage = enum { begin, attack, chain };

const Frame = struct {
    player: usize,
    prov: i32,
    event: i32,
    stage: Stage = .begin,
    /// Effect 15: the level of the attacking guard.
    att_level: i32 = -1,
};

pub const Runner = struct {
    gpa: std.mem.Allocator,
    pr: *const Proto,
    s: *Shard,
    r: *Rand,
    /// CurGame.iCurrentTurn.
    turn: i32 = 0,
    /// eador.cfg EVENT_POSSIBILITY (of 1000) and bStaticEvent.
    possibility: i32 = 30,
    static: bool = false,
    jobs: std.ArrayList(Job) = .empty,
    frames: std.ArrayList(Frame) = .empty,
    /// Dialogs to show, first one first.
    shows: std.ArrayList(Show) = .empty,
    /// The StaticEvent whose dialog is the last of `shows`.
    waiting: ?Frame = null,
    treasure: Treasure = @splat(.{ 0, 0 }),
    /// Provinces the events changed (their pictures are redrawn).
    touched: std.ArrayList(i32) = .empty,
    steps: u32 = 0,
    dx: i32 = 200,

    pub fn deinit(self: *Runner) void {
        self.jobs.deinit(self.gpa);
        self.frames.deinit(self.gpa);
        self.shows.deinit(self.gpa);
        self.touched.deinit(self.gpa);
    }

    pub fn idle(self: *const Runner) bool {
        return self.jobs.items.len == 0 and self.frames.items.len == 0 and self.shows.items.len == 0 and self.waiting == null;
    }

    /// Queues StaticEvent(event, player) in province `prov`.
    pub fn add(self: *Runner, player: usize, prov: i32, event: i32) !void {
        if (event <= 0 or event >= self.pr.events.len or prov <= 0 or prov >= self.s.provinces.len) return;
        try self.jobs.append(self.gpa, .{ .player = player, .prov = prov, .event = event });
    }

    /// The next dialog to show, running the events until one comes; null
    /// when every event is done.
    pub fn next(self: *Runner) ?*const Show {
        while (true) {
            if (self.shows.items.len > 0) return &self.shows.items[0];
            if (self.waiting != null) return null;
            if (self.frames.pop()) |f| {
                self.run(f) catch |e| {
                    std.log.err("event {d}: {t}", .{ f.event, e });
                    self.frames.clearRetainingCapacity();
                };
                continue;
            }
            if (self.jobs.items.len == 0) return null;
            const j = self.jobs.orderedRemove(0);
            self.steps = 0;
            self.dx = self.dxOf(j.prov);
            self.frames.append(self.gpa, .{ .player = j.player, .prov = j.prov, .event = j.event }) catch return null;
        }
    }

    /// The answer (0-based) to the dialog `next` gave.
    pub fn answered(self: *Runner, answer: usize) void {
        if (self.shows.items.len == 0) return;
        const sh = self.shows.orderedRemove(0);
        if (!sh.decision) return;
        var f = self.waiting orelse return;
        self.waiting = null;
        const ev = self.eventOf(f.event) orelse return;
        const a = @min(answer, proto_mod.max_answer - 1);
        const to = if (a < ev.n_answers) ev.answers[a] else 0;
        if (to > 0) {
            self.push(.{ .player = f.player, .prov = f.prov, .event = to });
        } else {
            f.stage = .attack;
            self.push(f);
        }
    }

    fn push(self: *Runner, f: Frame) void {
        if (f.event <= 0 or f.event >= self.pr.events.len) return;
        self.frames.append(self.gpa, f) catch std.log.err("event {d}: out of memory", .{f.event});
    }

    fn run(self: *Runner, f: Frame) !void {
        switch (f.stage) {
            .begin => try self.begin(f),
            .attack => try self.attack(f),
            .chain => self.chain(f),
        }
    }

    fn eventOf(self: *const Runner, i: i32) ?*const proto_mod.Event {
        if (i <= 0 or i >= self.pr.events.len) return null;
        return &self.pr.events[@intCast(i)];
    }

    fn eco(self: *const Runner) Economy {
        return .{ .s = self.s, .pr = self.pr };
    }

    fn world(self: *Runner) site_mod.World {
        return .{ .gpa = self.s.gpa, .pr = self.pr, .r = self.r, .guards = &self.s.guards, .shops = &self.s.shops, .neutrals = self.s.settings.neutrals };
    }

    fn random(self: *Runner, max: i32) i32 {
        return @bitCast(self.r.random(@bitCast(max)));
    }

    /// pv->ipPlayer as an index of arPlayer.
    fn owner(pv: *const Province) usize {
        return @intCast(std.math.clamp(pv.player, 0, mapgen.max_players));
    }

    fn dxOf(self: *const Runner, prov: i32) i32 {
        return if (self.s.provinces[@intCast(prov)].x + 400 > self.s.width()) -200 else 200;
    }

    fn touch(self: *Runner, prov: i32) !void {
        if (std.mem.indexOfScalar(i32, self.touched.items, prov) == null) try self.touched.append(self.gpa, prov);
    }

    fn storyOf(self: *Runner, k: usize) !*player_mod.Story {
        const st = &self.s.rulers[k].story;
        if (st.story.len == 0) st.* = try player_mod.Story.init(self.s.arena.allocator(), self.pr.events.len);
        return st;
    }

    // -- BeginMainTurn ----------------------------------------------------

    /// The timed events of a human (MainProcess.cpp:8177): each counter
    /// above 0 goes down, and at 0 the event happens in its province or in
    /// a random one of the player's.
    pub fn storyTurn(self: *Runner, player: usize) !void {
        const st = try self.storyOf(player);
        const pl: i32 = @intCast(player);
        for (1..st.story.len) |i| {
            if (st.story[i] <= 0) continue;
            st.story[i] -= 1;
            if (st.story[i] != 0) continue;
            var target: i32 = 0;
            const place = st.place[i];
            if (place > 0) {
                if (place < self.s.provinces.len and self.s.provinces[@intCast(place)].player == pl) target = place;
            } else {
                // for (Prov = 1; Prov < MapSize; Prov++): the last province
                // is left out.
                var rl: Roller = .{};
                var n: usize = 0;
                for (1..self.s.count()) |p| {
                    if (self.s.provinces[p].player != pl or n == unit_mod.max_roll) continue;
                    rl.ids[n] = @intCast(p);
                    rl.weights[n] = 10;
                    n += 1;
                }
                if (n > 0) target = rl.roll(self.r);
            }
            st.place[i] = 0;
            if (target > 0) try self.add(player, target, @intCast(i));
        }
    }

    /// The random event of province `prov` of `player` in BeginMainTurn
    /// (MainProcess.cpp:8646).  A human gets an event job, a computer a
    /// little gold and gems.
    pub fn atProvince(self: *Runner, player: usize, prov: i32, turn: i32) void {
        const seed = newsave.seedOf(self.s);
        const rnd: u32 = if (self.static) rand_mod.keyed(seed, 1000, turn, prov) else self.r.random(1000);
        if (turn <= 2 or tutorialMap(self.s) or rnd >= @as(u32, @bitCast(self.possibility))) return;
        const r = &self.s.rulers[player];
        const pv = &self.s.provinces[@intCast(prov)];
        if (r.kind == .human) {
            const ev = self.rollEvent(player, prov, if (self.static) .{ prov, turn } else null) catch return;
            if (ev > 0) self.add(player, prov, ev) catch {};
            return;
        }
        const pp = self.pr.player_profiles;
        const karma = pp[@intCast(std.math.clamp(r.profile, 0, @as(i32, @intCast(pp.len)) - 1))].karma;
        // Random(100) is unsigned: a negative sum is above 50 too.
        const v: i64 = @as(i64, self.r.random(100)) - @divTrunc(karma, 10) + @as(i64, self.eco().mood(pv)) * 10;
        if (v > 50 or v < 0) {
            if (karma == 0) {
                r.gold += self.random(100);
                r.gem += self.random(20);
            }
            if (karma > 0) r.gold += self.random(200);
            if (karma < 0) r.gem += self.random(40);
            r.gold += self.random(200);
            r.gem += self.random(40);
            if (self.s.capitals[player] != prov) changeAttitude(pv, player, -1);
        } else changeAttitude(pv, player, 1);
    }

    /// Roll among the events possible in the province that have not
    /// happened (pStory < 0); `key` is (Param1, Param2) of the keyed Roll
    /// of bStaticEvent.  0 when none can happen.
    pub fn rollEvent(self: *Runner, player: usize, prov: i32, key: ?[2]i32) !i32 {
        const st = try self.storyOf(player);
        var rl: Roller = .{};
        var n: usize = 0;
        for (1..self.pr.events.len) |j| {
            var poss = self.eventPoss(prov, @intCast(j));
            if (j < st.story.len and st.story[j] >= 0) poss = 0;
            if (poss <= 0 or n == unit_mod.max_roll) continue;
            rl.ids[n] = @intCast(j);
            rl.weights[n] = poss;
            n += 1;
        }
        if (n == 0) return 0;
        const k = key orelse return rl.roll(self.r);
        return rl.pick(rand_mod.keyed(newsave.seedOf(self.s), rl.sum(), k[0], k[1]) +% 1);
    }

    /// GetEventPoss: the weight of event `ev` in province `prov`, 0 when a
    /// condition (effect 8) fails.
    pub fn eventPoss(self: *Runner, prov: i32, ev_i: i32) i32 {
        const pr = self.pr;
        const s = self.s;
        const ev = self.eventOf(ev_i) orelse return 0;
        const pv = &s.provinces[@intCast(prov)];
        const own = owner(pv);
        const r = &s.rulers[own];
        const e = self.eco();
        if (movement.siege(s, prov) >= 0) return 0;
        for (ev.effects) |x| {
            if (x.id != eff.condition) continue;
            const v = x.p1;
            const fail = switch (x.power) {
                1 => e.level(pv.population) < v,
                2 => pv.terrain != v,
                3 => r.gold < v,
                4 => blk: {
                    var w = self.world();
                    break :blk w.freeSpace(pv, 1) == 0;
                },
                5 => e.mood(pv) > v,
                6 => e.mood(pv) < v,
                7 => e.level(pv.population) > v,
                8 => blk: {
                    var free: i32 = 0;
                    for (r.holdings.treasury) |t| free += @intFromBool(t[0] == 0);
                    break :blk free < v;
                },
                9 => r.karma > v,
                10 => r.karma < v,
                11 => s.capitals[own] == prov,
                12 => !pv.coastal,
                13 => pv.race != v,
                14 => blk: {
                    var none = !self.heroIn(pv, pv.player);
                    if (v > 0 and pv.guard >= 0 and s.guards.items[@intCast(pv.guard)].player == pv.player) none = false;
                    break :blk none;
                },
                15 => s.capitals[own] != prov,
                16 => e.resource(pv, -1) > 0,
                17 => blk: {
                    const slot = for (pv.sites) |st| {
                        if (st.index == 0) break true;
                    } else false;
                    if (!slot) break :blk true;
                    var w = self.world();
                    break :blk w.freeSpace(pv, v) == 0;
                },
                18 => if (pv.guard >= 0) (v == 0 or v != pv.defender) else v > 0,
                19 => std.mem.indexOfScalar(i32, pv.buildings[0..max_prov_bld], v) == null,
                20 => s.capitals[own] == prov or std.mem.indexOfScalar(i32, pv.buildings[0..max_prov_bld], 0) == null,
                21 => std.mem.indexOfScalar(i32, pv.buildings[0..max_prov_bld], v) != null,
                22 => r.calc_corrupt < v,
                23 => r.gem < v,
                24 => e.playerResource(pv.player, v) == 0,
                25 => e.playerResource(pv.player, v) > 0,
                // The campaign's Campaign.arDialog is not ported.
                26 => true,
                27 => self.heroIn(pv, pv.player),
                28 => for (pv.sites) |st| {
                    if (st.index == v) break true;
                } else false,
                29 => for (r.holdings.treasury) |t| {
                    if (t[0] == v) break false;
                } else true,
                else => false,
            };
            if (fail) return 0;
        }
        var result = ev.possibility;
        for (pv.neighbourList()) |nb| {
            const q = &s.provinces[@intCast(nb)];
            if (q.player != 0 or q.type < 0 or q.type >= pr.province_types.len) continue;
            const pt = &pr.province_types[@intCast(q.type)];
            for (pt.events) |pe| {
                if (pe.id == ev_i) result += pe.value;
            }
        }
        result += self.siteParam2(pv, 4, ev_i);
        if (result <= 0) return 0;
        if (s.capitals[own] == prov) {
            result += @divTrunc(result * e.param2(pv.player, ab.castle_event, ev_i), 100);
        } else {
            for (pv.buildings[1..max_prov_bld]) |b| {
                if (b <= 0 or b >= pr.outer_buildings.len) continue;
                const ob = &pr.outer_buildings[@intCast(b)];
                for (ob.abilities[0..ob.n_abilities]) |a| {
                    if (a.id == ab.outer_event and a.p1 == ev_i) result += @divTrunc(result * a.p2, 100);
                }
            }
        }
        const k = r.karma + e.param1(pv.player, ab.castle_karma);
        if (std.math.sign(k) == std.math.sign(ev.kind)) result += @divTrunc(result * @as(i32, @intCast(@abs(k))), 100);
        return @max(result, 1);
    }

    /// GetSiteParam2(Index, Param1): p2 of the working sites' abilities.
    fn siteParam2(self: *const Runner, pv: *const Province, id: i32, p1: i32) i32 {
        const e = self.eco();
        var r: i32 = 0;
        for (pv.sites) |st| {
            if (!e.siteWorks(pv, st, false)) continue;
            const site = &self.pr.sites[@intCast(st.index)];
            for (site.abilities[0..site.n_abilities]) |a| {
                if (a.id == id and a.p1 == p1) r += a.p2;
            }
        }
        return r;
    }

    fn heroIn(self: *const Runner, pv: *const Province, pl: i32) bool {
        for (self.s.heroes.items) |*h| {
            if (h.owner == pl and h.province == pv.number and h.alive()) return true;
        }
        return false;
    }

    // -- StaticEvent ------------------------------------------------------

    /// clEvent::CalcEffect(Index, Mult, Value).  Random takes an unsigned
    /// long: a negative bound is a huge one.
    fn calc(self: *Runner, ev: *const proto_mod.Event, index: i32, mult: i32, value: i32) i32 {
        var result: i32 = 0;
        for (ev.effects) |e| {
            if (e.id != index) continue;
            switch (e.p2) {
                1 => result +%= e.power *% mult +% self.random(e.p1 *% mult),
                2 => {
                    const perc = e.power +% self.random(e.p1);
                    result +%= @intCast(@divTrunc(@as(i64, value) * perc, 100));
                },
                else => result +%= e.power +% self.random(e.p1),
            }
        }
        return result;
    }

    /// StaticEvent up to its dialog.
    fn begin(self: *Runner, f: Frame) !void {
        self.steps += 1;
        if (self.steps > max_steps) return error.EventLoop;
        const pr = self.pr;
        const s = self.s;
        const ev = self.eventOf(f.event) orelse return;
        const pv = &s.provinces[@intCast(f.prov)];
        try self.touch(f.prov);
        const pl = f.player;
        const e = self.eco();
        self.treasure = @splat(.{ 0, 0 });
        var lootz: usize = 1;
        var att_level: i32 = -1;
        var guard_gold = false;
        {
            const r = &s.rulers[owner(pv)];
            r.karma += ev.karma;
            r.karma += self.calc(ev, eff.karma, 1, r.karma);
        }
        var mult: i32 = 0;
        for (ev.effects, 0..) |x, k| switch (x.id) {
            eff.site => if (x.power > 0 and x.power < pr.sites.len) {
                var w = self.world();
                _ = try w.place(pv, x.power, @divTrunc(self.turn, 20), 0, x.p1, x.p2);
            },
            eff.outer_stock => if (self.stock(owner(pv), .outer, x.power)) |st| if (st.get() >= 0) {
                st.add(x.p1);
                if (lootz < 6) {
                    self.treasure[lootz] = .{ itemWith(pr, ab.item_outer, x.power), 1 };
                    lootz += 1;
                }
            },
            eff.scroll => {
                const id: i32 = @as(i32, @intCast(pr.max_pure_item)) + x.power;
                var i: i32 = 0;
                while (i < x.p1) : (i += 1) {
                    if (lootz < 6) {
                        self.treasure[lootz] = .{ id, 1 };
                        lootz += 1;
                    }
                    _ = self.addTreasury(owner(pv), id, 1);
                }
            },
            eff.item_roll => {
                var rl: Roller = .{};
                var n: usize = 0;
                for (1..pr.max_pure_item + 1) |i| {
                    const it = &pr.items[i];
                    if (it.shop_level != x.power or it.rarity < x.p1 or n == unit_mod.max_roll) continue;
                    rl.ids[n] = @intCast(i);
                    rl.weights[n] = 7 - it.rarity;
                    n += 1;
                }
                if (n == 0) continue;
                const id = rl.roll(self.r);
                const dur = pr.items[@intCast(id)].durability;
                if (lootz < 6) {
                    self.treasure[lootz] = .{ id, dur };
                    lootz += 1;
                }
                _ = self.addTreasury(owner(pv), id, dur);
            },
            eff.attack_level => att_level = x.power,
            eff.item => if (x.power > 0 and x.power < pr.items.len) {
                const dur = @divTrunc(pr.items[@intCast(x.power)].durability * x.p2, 100);
                var i: i32 = 0;
                while (i < x.p1) : (i += 1) {
                    if (!self.addTreasury(owner(pv), x.power, dur)) continue;
                    if (lootz < 6) {
                        self.treasure[lootz] = .{ x.power, dur };
                        lootz += 1;
                    }
                }
            },
            eff.story => {
                const st = try self.storyOf(pl);
                if (x.power > 0 and x.power < st.story.len) st.story[@intCast(x.power)] = x.p1 + self.random(x.p2);
            },
            eff.place => {
                const st = try self.storyOf(pl);
                if (x.power > 0 and x.power < st.place.len) st.place[@intCast(x.power)] = switch (x.p1) {
                    1 => pv.number,
                    2 => s.capitals[pl],
                    else => 0,
                };
            },
            eff.defender => if (x.power > 0) try self.placeDefender(pv, x.power) else if (pv.guard >= 0) dismissDefender(pv),
            eff.raze => for (0..max_prov_bld) |i| {
                if (pv.buildings[i] == x.power) self.raze(pv, i);
            },
            eff.construct => try self.construct(pv, x.power),
            eff.remove_site => {
                var found: ?usize = null;
                for (pv.sites, 0..) |st, i| {
                    if (st.index == x.power and st.pos <= pv.explore) found = i;
                }
                if (found == null) for (pv.sites, 0..) |st, i| {
                    if (st.index == x.power) found = i;
                };
                if (found) |i| {
                    pv.sites[i].index = 0;
                    if (pv.site_guards[i] >= 0) {
                        pv.site_guards[i] = -1;
                        pv.sites[i].guard = 0;
                    }
                }
            },
            eff.raze_count => {
                var left = x.power;
                var i: usize = max_prov_bld;
                while (i > 0) {
                    i -= 1;
                    const b = pv.buildings[i];
                    if (b > 0 and left > 0 and b < pr.outer_buildings.len and pr.outer_buildings[@intCast(b)].wall <= 0) {
                        self.raze(pv, i);
                        left -= 1;
                    }
                }
            },
            eff.race_attitude => for (s.provinces[1..]) |*q| {
                if (q.race == x.power) changeAttitude(q, owner(pv), x.p1);
            },
            eff.res_price => {
                const w = x.p1 + self.random(x.p2);
                if (x.power >= 0 and x.power < s.res_prices.len) s.res_prices[@intCast(x.power)] += w;
                if (k == 0) mult = @intCast(@abs(w));
            },
            eff.life => {
                if (pv.guard >= 0) changeLife(pr, &s.guards.items[@intCast(pv.guard)].squad, x.power, x.p1, x.p2);
                if (pv.garrison >= 0) changeLife(pr, &s.garrisons.items[@intCast(pv.garrison)].squad, x.power, x.p1, x.p2);
                for (s.heroes.items) |*h| {
                    if (h.owner < 1 or !s.in_game[@intCast(h.owner)] or !h.alive() or h.province != pv.number) continue;
                    changeLife(pr, &h.squad, x.power, x.p1, x.p2);
                }
            },
            eff.morale => {
                if (pv.garrison >= 0) changeMorale(pr, &s.garrisons.items[@intCast(pv.garrison)].squad, x.power + self.random(x.p1), x.p2);
                if (pv.guard >= 0) changeMorale(pr, &s.guards.items[@intCast(pv.guard)].squad, x.power + self.random(x.p1), x.p2);
                for (s.heroes.items) |*h| {
                    if (h.owner < 1 or !s.in_game[@intCast(h.owner)] or !h.alive() or h.province != pv.number) continue;
                    changeMorale(pr, &h.squad, x.power + self.random(x.p1), x.p2);
                }
            },
            eff.rebel => try self.rebel(pv),
            eff.fort => pv.fort = std.math.clamp(pv.fort + x.power + self.random(x.p1), 0, @max(self.wall(pv), 0)),
            eff.camp_event => if (x.power > 0) return,
            eff.guard_gold => guard_gold = true,
            eff.neutral => {
                dismissDefender(pv);
                try self.makeNeutral(pv);
            },
            eff.shop_item => {
                const w = self.world();
                const rl = w.shopRoller(x.power, x.p1, x.p2, true);
                if (rl.len() == 0) continue;
                const id = rl.roll(self.r);
                const it = &pr.items[@intCast(id)];
                if (lootz < 6) {
                    self.treasure[lootz] = .{ id, it.durability };
                    lootz += 1;
                }
                const which: ?StockList = switch (it.class) {
                    11 => .defender,
                    12 => .outer,
                    13 => .ritual,
                    else => null,
                };
                if (which) |l| {
                    if (self.stock(pl, l, it.effects[0].p1)) |st| if (st.get() >= 0) st.add(1);
                } else _ = self.addTreasury(pl, id, it.durability);
            },
            eff.take_item => for (&s.rulers[pl].holdings.treasury) |*t| {
                if (t[0] == x.power) {
                    t.* = .{ 0, 0 };
                    break;
                }
            },
            eff.defender_stock => if (self.stock(owner(pv), .defender, x.power)) |st| if (st.get() >= 0) {
                st.add(x.p1);
                if (lootz < 6) {
                    self.treasure[lootz] = .{ itemWith(pr, ab.item_defender, x.power), 1 };
                    lootz += 1;
                }
            },
            eff.if_resource => if (x.power == 24 and e.playerResource(@intCast(pl), x.p1) > 0) {
                self.push(.{ .player = pl, .prov = f.prov, .event = x.p2 });
                return;
            },
            eff.guarded_site => if (x.power > 0 and x.power < pr.sites.len and x.p1 > 0 and x.p1 < pr.guard_types.len) {
                var w = self.world();
                if (try w.place(pv, x.power, @divTrunc(self.turn, 20), 0, -1, 0)) |slot| {
                    pv.sites[slot].data = -1;
                    try w.placeSiteGuard(pv, slot, pr.guard_types[@intCast(x.p1)].guards[@intCast(std.math.clamp(x.p2, 0, proto_mod.max_guard_level - 1))]);
                    pv.sites[slot].guard = x.p1;
                }
            },
            eff.victory => {
                std.log.info("event {d}: Victory is not ported", .{f.event});
                return;
            },
            else => {},
        };

        // The common effects.
        const own = owner(pv);
        const r = &s.rulers[own];
        var work = self.calc(ev, eff.gold, e.level(pv.population), r.gold);
        if (work < 0 and guard_gold) work -= @divTrunc(work * e.param1(pv.player, ab.castle_guard_gold), 10);
        r.gold += work;
        self.treasure[0][0] = work;
        if (ev.effects[0].id == eff.gold) mult = @intCast(@abs(work));
        changeAttitude(pv, own, self.calc(ev, eff.attitude, e.level(pv.population), pv.attitude[own][0]));
        pv.population += @intCast(@divTrunc(@as(i64, pv.population) * self.calc(ev, eff.population, e.level(pv.population), pv.population), 100));
        var inc1 = e.goldIncome(pv);
        pv.gold_inc = @max(pv.gold_inc + self.calc(ev, eff.gold_inc, e.level(pv.population), pv.gold_inc), 0);
        var inc2 = e.goldIncome(pv);
        if (ev.effects[0].id == eff.gold_inc) mult = inc2 - inc1;
        work = self.calc(ev, eff.gem, e.level(pv.population), r.gem);
        r.gem += work;
        self.treasure[0][1] = work;
        if (ev.effects[0].id == eff.gem) mult = @intCast(@abs(work));
        pv.unrest[own] = std.math.clamp(pv.unrest[own] + self.calc(ev, eff.unrest, e.level(pv.population), pv.unrest[own]), 0, max_unrest);
        s.rulers[pl].corrupt += self.calc(ev, eff.corrupt, e.level(pv.population), s.rulers[pl].corrupt);
        inc1 = e.gemIncome(pv);
        pv.gem_inc = @max(pv.gem_inc + self.calc(ev, eff.gem_inc, e.level(pv.population), pv.gem_inc), 0);
        inc2 = e.gemIncome(pv);
        if (ev.effects[0].id == eff.gem_inc) mult = inc2 - inc1;

        var next_f = f;
        next_f.att_level = att_level;
        if (ev.dialog > 0 and ev.dialog < pr.dialogs.len) {
            mult = @intCast(@abs(mult));
            if (mult == 0) mult = 1;
            const d = &pr.dialogs[@intCast(ev.dialog)];
            var sh: Show = .{ .dialog = @intCast(ev.dialog), .enabled = @splat(false), .treasure = self.treasure, .prov = f.prov, .dx = self.dx, .decision = true };
            const name: Arg = .{ .s = pv.name };
            const m: Arg = .{ .d = mult };
            const args: []const Arg = switch (ev.dlg_param1) {
                1 => if (ev.dlg_param2 == 2) &.{ name, m } else &.{name},
                2 => if (ev.dlg_param2 == 1) &.{ m, name } else &.{m},
                else => &.{},
            };
            sh.len = format(&sh.buf, d.text, args).len;
            sh.enabled = self.answers(ev, d, pv);
            try self.shows.append(self.gpa, sh);
            self.waiting = next_f;
            return;
        }
        next_f.stage = .attack;
        self.push(next_f);
    }

    /// An[i] of StaticEvent: the answers that are not generated ("?#")
    /// and whose conditions (effect 4) hold.
    fn answers(self: *const Runner, ev: *const proto_mod.Event, d: *const proto_mod.Dialog, pv: *const Province) [proto_mod.max_answer]bool {
        var on: [proto_mod.max_answer]bool = @splat(false);
        const r = &self.s.rulers[owner(pv)];
        for (&on, 0..) |*a, i| {
            if (i >= d.n_answers) continue;
            const t = d.answers[i];
            if (t.len > 1 and t[1] == '#') continue;
            a.* = true;
            for (ev.effects) |x| {
                if (x.id != eff.answer or x.power != @as(i32, @intCast(i + 1))) continue;
                switch (x.p1) {
                    1 => if (r.gold < x.p2) {
                        a.* = false;
                    },
                    2 => if (r.gem < x.p2) {
                        a.* = false;
                    },
                    3 => if (!self.heroIn(pv, pv.player) and pv.defender <= 0) {
                        a.* = false;
                    },
                    else => {},
                }
            }
        }
        return on;
    }

    /// The attack of the event (EV->ipAttacker), then its result's event.
    fn attack(self: *Runner, f: Frame) !void {
        const pr = self.pr;
        const ev = self.eventOf(f.event) orelse return;
        var chain_f = f;
        chain_f.stage = .chain;
        self.push(chain_f);
        if (ev.attacker <= 0 or ev.attacker >= pr.guard_types.len) return;
        const gt = &pr.guard_types[@intCast(ev.attacker)];
        var lvl: i32 = @intCast(self.r.random(@bitCast(self.turn)) / 50);
        if (f.att_level >= 0) lvl = f.att_level;
        lvl = std.math.clamp(lvl, 0, proto_mod.max_guard_level - 1);
        while (lvl > 0 and pr.guards[@intCast(gt.guards[@intCast(lvl)])].power > self.s.settings.neutrals) lvl -= 1;
        const fake = guard_mod.place(pr, self.r, gt.guards[@intCast(lvl)], gt.karma);
        const pv = &self.s.provinces[@intCast(f.prov)];
        const then: Frame = .{ .player = f.player, .prov = f.prov, .event = 0 };
        switch (self.underAttack(pv)) {
            -1 => self.push(withEvent(then, ev.event_draw)),
            1 => {
                if (ev.prov_type > 0) {
                    pv.defender = 0;
                    pv.guard = try self.addGuard(fake);
                    pv.type = ev.prov_type;
                    if (self.garrisonEmpty(pv)) {
                        self.conquer(pv);
                        self.push(withEvent(then, ev.event_lose));
                    } else pv.event = ev.event_lose;
                } else self.push(withEvent(then, ev.event_lose));
            },
            else => self.push(withEvent(then, ev.event_win)),
        }
    }

    /// Effect 11: the next event, with a chance to take the other one.
    fn chain(self: *Runner, f: Frame) void {
        const ev = self.eventOf(f.event) orelse return;
        for (ev.effects) |x| {
            if (x.id != eff.next) continue;
            if (x.p1 != 0 and self.random(100) > x.p2) {
                if (x.p1 > 0) return self.push(.{ .player = f.player, .prov = f.prov, .event = x.p1 });
            } else return self.push(.{ .player = f.player, .prov = f.prov, .event = x.power });
        }
    }

    /// UnderAttack by a neutral squad: 1 the attacker takes the province.
    /// Only the case without defenders is ported; with a defender or heroes
    /// the battle is not, and the defenders hold.
    fn underAttack(self: *Runner, pv: *const Province) i32 {
        if (pv.player == 0) return 0;
        if (movement.siege(self.s, pv.number) >= 0) return 0;
        var n: usize = 0;
        if (pv.defender > 0 and pv.guard >= 0 and self.s.guards.items[@intCast(pv.guard)].player != 0) n += 1;
        if (self.heroIn(pv, pv.player)) n += 1;
        if (n == 0) return 1;
        std.log.info("attack on {s}: the battle is not ported, the defenders hold", .{pv.name});
        return 0;
    }

    /// Rebel (Event.cpp:2427): the province's own race rises against the
    /// owner.
    fn rebel(self: *Runner, pv: *Province) !void {
        const pr = self.pr;
        try self.inform(pv, 62, &.{.{ .s = pv.name }});
        var adder = std.math.clamp(@divTrunc(self.eco().level(pv.population) - 3, 2), 0, 3);
        const new_type = pr.races[@intCast(std.math.clamp(pv.race, 0, @as(i32, @intCast(pr.races.len)) - 1))].prov_type;
        const grd = pr.province_types[@intCast(new_type)].guards[0].guard_type;
        const gt = &pr.guard_types[@intCast(grd)];
        while (adder > 0 and pr.guards[@intCast(gt.guards[@intCast(adder)])].power > self.s.settings.neutrals) adder -= 1;
        const fake = guard_mod.place(pr, self.r, gt.guards[@intCast(adder)], gt.karma);
        if (self.underAttack(pv) != 1) return;
        if (self.garrisonEmpty(pv)) {
            try self.inform(pv, 65, &.{ .{ .s = pv.name }, .{ .s = gt.name } });
            self.conquer(pv);
        }
        pv.type = new_type;
        pv.guard = try self.addGuard(fake);
        pv.defender = 0;
    }

    /// A message for the province's owner if a human plays him.
    fn inform(self: *Runner, pv: *const Province, dialog: usize, args: []const Arg) !void {
        if (self.s.rulers[owner(pv)].kind != .human or dialog >= self.pr.dialogs.len) return;
        var sh: Show = .{ .dialog = dialog, .enabled = .{ true, true, false, false }, .treasure = self.treasure, .prov = pv.number, .dx = self.dx, .decision = false };
        sh.len = format(&sh.buf, self.pr.dialogs[dialog].text, args).len;
        try self.shows.append(self.gpa, sh);
    }

    fn addGuard(self: *Runner, g: Guard) !i32 {
        try self.s.guards.append(self.s.gpa, g);
        return @intCast(self.s.guards.items.len - 1);
    }

    fn garrisonEmpty(self: *const Runner, pv: *const Province) bool {
        if (pv.garrison < 0) return true;
        for (self.s.garrisons.items[@intCast(pv.garrison)].squad) |u| if (u != null) return false;
        return true;
    }

    /// Conquer(0, NULL): the neutrals take the province.  CheckPlayerCondition
    /// of the loser is not ported.
    fn conquer(self: *Runner, pv: *Province) void {
        pv.player = 0;
        if (pv.garrison >= 0) self.s.garrisons.items[@intCast(pv.garrison)].player = 0;
    }

    /// MakeNeutral (Province.cpp:3888).
    fn makeNeutral(self: *Runner, pv: *Province) !void {
        const pr = self.pr;
        const lvl = self.eco().level(pv.population);
        var new_type = pr.races[@intCast(std.math.clamp(pv.race, 0, @as(i32, @intCast(pr.races.len)) - 1))].prov_type;
        if (lvl == 1) new_type = 15;
        if (lvl == 2) new_type = 5;
        pv.player = 0;
        pv.type = 21;
        pv.defender = 0;
        if (pv.guard < 0) {
            pv.type = new_type;
            const adder = std.math.clamp(@divTrunc(lvl - 3, 2), 0, 3);
            const grd = pr.province_types[@intCast(new_type)].guards[0].guard_type;
            const gt = &pr.guard_types[@intCast(grd)];
            pv.guard = try self.addGuard(guard_mod.place(pr, self.r, gt.guards[@intCast(adder)], gt.karma));
        } else self.s.guards.items[@intCast(pv.guard)].player = 0;
        if (pv.garrison >= 0) {
            const g = &self.s.garrisons.items[@intCast(pv.garrison)];
            g.player = 0;
            g.squad = @splat(null);
        }
    }

    /// PlaceDefender (Province.cpp:210).
    pub fn placeDefender(self: *Runner, pv: *Province, d: i32) !void {
        if (d <= 0 or d >= self.pr.defenders.len) return;
        if (pv.defender > 0) dismissDefender(pv);
        const bonus = if (pv.player > 0) self.eco().param1(pv.player, ab.castle_defender_exp) else 0;
        pv.guard = try self.addGuard(guard_mod.placeDefender(self.pr, self.r, d, pv.player, bonus));
        pv.defender = d;
    }

    /// RazeBuilding (Province.cpp:3557).
    pub fn raze(self: *Runner, pv: *Province, slot: usize) void {
        const b = pv.buildings[slot];
        if (b <= 0 or b >= self.pr.outer_buildings.len) return;
        const ob = &self.pr.outer_buildings[@intCast(b)];
        for (ob.abilities[0..ob.n_abilities]) |a| {
            if (a.id == ab.outer_garrison) pv.garrison = -1;
        }
        pv.buildings[slot] = 0;
        self.s.rulers[owner(pv)].karma -= ob.karma;
    }

    /// ConstructBuilding(Build, Redraw, false) (Province.cpp:3374).
    pub fn construct(self: *Runner, pv: *Province, build: i32) !void {
        const pr = self.pr;
        if (build <= 0 or build >= pr.outer_buildings.len) return;
        const bld = &pr.outer_buildings[@intCast(build)];
        var slot: usize = 0;
        while (slot < max_prov_bld and pv.buildings[slot] != bld.upgrade) slot += 1;
        if (slot >= max_prov_bld) {
            if (bld.upgrade == 0) return;
            slot = 0;
            while (slot < max_prov_bld and pv.buildings[slot] > 0) slot += 1;
            if (slot >= max_prov_bld) return;
        }
        if (bld.wall > 0) {
            const old = pv.buildings[slot];
            pv.fort = if (old > 0 and old < pr.outer_buildings.len) pv.fort + bld.wall - pr.outer_buildings[@intCast(old)].wall else bld.wall;
        }
        pv.buildings[slot] = build;
        self.s.rulers[owner(pv)].karma += bld.karma;
        var garrison = false;
        for (bld.abilities[0..bld.n_abilities]) |a| {
            if (a.id == ab.outer_garrison) garrison = true;
            if (a.id == ab.outer_site and a.p1 > 0 and a.p1 < pr.sites.len) {
                var w = self.world();
                _ = try w.place(pv, a.p1, a.p2, 0, 0, 0);
            }
        }
        if (!garrison) return;
        if (pv.garrison < 0) {
            var g = hero_mod.Hero.garrison(pr, pv.player, pv.number);
            for (0..4) |i| std.mem.writeInt(i32, g.rec[0x390 + 4 * i ..][0..4], -1, .little);
            try self.s.garrisons.append(self.s.gpa, g);
            pv.garrison = @intCast(self.s.garrisons.items.len - 1);
        }
        const g = &self.s.garrisons.items[@intCast(pv.garrison)];
        for (bld.abilities[0..bld.n_abilities]) |a| {
            if (a.id == ab.outer_garrison and a.p1 >= 1 and a.p1 <= 4) std.mem.writeInt(i32, g.rec[0x390 + 4 * @as(usize, @intCast(a.p1 - 1)) ..][0..4], a.p2, .little);
        }
    }

    /// GetWall.
    fn wall(self: *const Runner, pv: *const Province) i32 {
        const own = owner(pv);
        if (self.s.capitals[own] == pv.number and own > 0) return self.s.rulers[own].wall;
        var w: i32 = 0;
        for (pv.buildings[0..max_prov_bld]) |b| {
            if (b > 0 and b < self.pr.outer_buildings.len) w += self.pr.outer_buildings[@intCast(b)].wall;
        }
        return w;
    }

    /// arTreasury: the first free place gets the item.
    fn addTreasury(self: *Runner, k: usize, id: i32, dur: i32) bool {
        for (&self.s.rulers[k].holdings.treasury) |*t| {
            if (t[0] > 0) continue;
            t.* = .{ id, dur };
            return true;
        }
        return false;
    }

    const StockList = enum { defender, outer, ritual };

    const Stock = struct {
        b: *[4]u8,
        fn get(self: Stock) i32 {
            return std.mem.readInt(i32, self.b, .little);
        }
        fn add(self: Stock, v: i32) void {
            std.mem.writeInt(i32, self.b, self.get() + v, .little);
        }
    };

    /// pDefender, pOuterBld or pRitual of player `k`: they stay in the
    /// saved lists (Shard.saved), which are changed in place.
    fn stock(self: *Runner, k: usize, which: StockList, idx: i32) ?Stock {
        const sv = if (self.s.saved) |*x| x else return null;
        const b: []u8 = @constCast(sv.lists[k]);
        var p: usize = 0;
        for ([_]StockList{ .defender, .outer, .ritual }) |l| {
            if (p + 4 > b.len) return null;
            const n: i32 = std.mem.readInt(i32, b[p..][0..4], .little);
            if (n < 0) return null;
            const size: usize = if (l == .ritual) 8 else 4;
            if (l == which) {
                if (idx < 0 or idx > n) return null;
                const at = p + 4 + @as(usize, @intCast(idx)) * size;
                if (at + 4 > b.len) return null;
                return .{ .b = b[at..][0..4] };
            }
            p += 4 + @as(usize, @intCast(n + 1)) * size;
        }
        return null;
    }
};

fn withEvent(f: Frame, ev: i32) Frame {
    var x = f;
    x.event = ev;
    return x;
}

/// The item shown for a stock effect: the last one (k < iMaxItem) whose
/// first effect is `id` with Power `p1`.  The C++ loop sets only the item
/// under the condition, so 0 when none.
fn itemWith(pr: *const Proto, id: i32, p1: i32) i32 {
    var found: i32 = 0;
    for (1..pr.items.len -| 1) |k| {
        const e = pr.items[k].effects[0];
        if (e.id == id and e.p1 == p1) found = @intCast(k);
    }
    return found;
}

/// ChangeAttitude.
pub fn changeAttitude(pv: *Province, pl: usize, change: i32) void {
    if (change == 0) return;
    const a = &pv.attitude[pl];
    if (a[0] == 0) a[1] = economy.attitude_time;
    a[0] += change;
}

/// dcHero::ChangeLife (Hero.cpp:1713).
pub fn changeLife(pr: *const Proto, sq: *[hero_mod.max_squad]?Unit, power: i32, kind: i32, targets: i32) void {
    for (sq) |*slot| {
        const u = if (slot.*) |*x| x else continue;
        if (u.life <= 0) continue;
        const uk = pr.units[@intCast(u.type)].unit_kind;
        if (targets != 0 and (uk == kind_undead or uk == kind_no_life)) continue;
        const max = u.maxLife(pr);
        var hit = if (kind == 0) power else if (power > 0) @divTrunc(power * max, 100) + 1 else @divTrunc(power * u.life, 100) - 1;
        if (hit < 0) {
            hit += pr.units[@intCast(u.type)].resist + u.checkUpg(pr, 6) + u.checkAbility(pr, 6);
            if (hit > 0) hit = 0;
        }
        u.life = std.math.clamp(u.life + hit, 1, @max(max, 1));
    }
}

/// dcHero::ChangeMorale (Hero.cpp:1745).
pub fn changeMorale(pr: *const Proto, sq: *[hero_mod.max_squad]?Unit, power: i32, dkarma: i32) void {
    for (sq) |*slot| {
        const u = if (slot.*) |*x| x else continue;
        if (u.life <= 0 or u.checkAbility(pr, ab.no_morale) != 0) continue;
        if (dkarma == 0) {
            u.morale += power * 2;
            continue;
        }
        const k = pr.units[@intCast(u.type)].karma;
        if (k == 0) continue;
        if (std.math.sign(dkarma) == std.math.sign(k)) u.morale += power * 2 else u.morale -= power * 2;
    }
}

/// DismissDefender.  The guard stays in the shard's list unused.
fn dismissDefender(pv: *Province) void {
    pv.guard = -1;
    pv.defender = 0;
}

/// Tutorial.Map: the second byte of dcTutorial, the last 0x120 bytes of a
/// game file.
pub fn tutorialMap(s: *const Shard) bool {
    const sv = s.saved orelse return false;
    if (sv.tail.len < newsave.tutorial_size) return false;
    return sv.tail[sv.tail.len - newsave.tutorial_size + 1] != 0;
}

const tst = std.testing;

test "dialog format" {
    var buf: [64]u8 = undefined;
    try tst.expectEqualStrings("В провинции Ольм 25%.", format(&buf, "В провинции %s %d%%.", &.{ .{ .s = "Ольм" }, .{ .d = 25 } }));
    try tst.expectEqualStrings("a  b", format(&buf, "a %s b", &.{}));
}

test "famine relief: effects, dialog and the next event" {
    const gpa = tst.allocator;
    const GameFs = @import("../platform/fs.zig").GameFs;
    const packfile = @import("../formats/packfile.zig");
    const loadgame = @import("loadgame.zig");
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const read = struct {
        fn f(a: std.mem.Allocator, path: []const u8) ![]u8 {
            const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, a, .unlimited) catch return error.SkipZigTest;
            defer a.free(bytes);
            return packfile.unpack(a, bytes);
        }
    }.f;
    const mdata = try read(gpa, "testdata/hero_route/map0.map");
    defer gpa.free(mdata);
    const gdata = try read(gpa, "testdata/hero_route/game0_turn3.sav");
    defer gpa.free(gdata);
    var g = try loadgame.fromBytes(gpa, &pr, mdata, gdata);
    defer g.deinit();
    const s = &g.shard;
    // The pStory of the file: nothing has happened yet.
    try tst.expectEqual(pr.events.len, s.rulers[1].story.story.len);
    try tst.expectEqual(@as(i32, -1), s.rulers[1].story.story[1]);

    var r: Rand = .{};
    var run: Runner = .{ .gpa = gpa, .pr = &pr, .s = s, .r = &r, .turn = 3 };
    defer run.deinit();
    const cap = s.capitals[1];
    const pv = &s.provinces[@intCast(cap)];
    const gold = s.rulers[1].gold;
    const karma = s.rulers[1].karma;
    const att = pv.attitude[1][0];
    // /2 "Голод (помощь)": -100 gold, attitude +1, karma +5, dialog 67.
    try run.add(1, cap, 2);
    const sh = run.next() orelse return error.NoDialog;
    try tst.expectEqual(@as(usize, 67), sh.dialog);
    try tst.expect(sh.decision);
    try tst.expectEqual([4]bool{ true, true, false, false }, sh.enabled);
    try tst.expectEqual(gold - 100, s.rulers[1].gold);
    try tst.expectEqual(karma + 5, s.rulers[1].karma);
    try tst.expectEqual(att + 1, pv.attitude[1][0]);
    try tst.expectEqual(@as(i32, -100), sh.treasure[0][0]);
    // Answer 1 leads to /13: karma -1 + Random(2), no dialog.
    run.answered(0);
    try tst.expect(run.next() == null);
    try tst.expect(run.idle());
    const dk = s.rulers[1].karma - (karma + 5);
    try tst.expect(dk == -1 or dk == 0);
    try tst.expect(std.mem.indexOfScalar(i32, run.touched.items, cap) != null);

    // The possibilities do not depend on the order they are asked in.
    var total: i32 = 0;
    for (1..pr.events.len) |j| total += @intFromBool(run.eventPoss(cap, @intCast(j)) > 0);
    try tst.expect(total > 0);
    // Condition 15 (only in the capital) holds there, 11 (not there) fails.
    for (pr.events[1..], 1..) |ev, j| for (ev.effects) |x| {
        if (x.id == eff.condition and x.power == 11) try tst.expectEqual(@as(i32, 0), run.eventPoss(cap, @intCast(j)));
    };
}
