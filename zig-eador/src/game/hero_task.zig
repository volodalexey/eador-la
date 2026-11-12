//! The hero's orders of EndMainTurn that need dialogs and battles: explore
//! (MainProcess.cpp:6810), pillage (:6890), enter a building (:6935) and
//! attack a guard (:6985), the site events of dcEvent::Call (Event.cpp:618)
//! and WindowTreasure (Windower.cpp:3187).
//!
//! The original runs these in EndMainTurn with nested loops (dialog, battle,
//! WindowTreasure).  Here a Runner queues the jobs; the map screen asks it
//! for the next dialog / battle / treasure and hands the answer back.  The
//! effects are applied when the request is made, as the original applies
//! them before its dialog.
//!
//! Not ported: encounters (CallEncounter: `site.dialog2` of a site that was
//! rolled as guarded, the bribe and scouting answers), the hero and unit
//! level-up windows, the arena, the world rules and the campaign.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const mapgen = @import("mapgen.zig");
const Shard = mapgen.Shard;
const province = @import("province.zig");
const Province = province.Province;
const hero_mod = @import("hero.zig");
const Hero = hero_mod.Hero;
const unit_mod = @import("unit.zig");
const Unit = unit_mod.Unit;
const economy = @import("economy.zig");
const player = @import("player.zig");
const Market = player.Market;
const event_mod = @import("event.zig");
const rand_mod = @import("../data/rand.zig");
const Rand = rand_mod.Rand;
const newsave = @import("newsave.zig");

/// EXPLORE_EXP, UNIT_LEVEL_CAP, MAX_LOOT.
const explore_exp = 4;
const level_cap = unit_mod.level_cap;
pub const max_loot = 5;

/// An order shown as a dialog: dialog.var `dialog` with `enabled` answers
/// and the text of a sprintf.
pub const Show = struct {
    dialog: usize,
    enabled: [proto_mod.max_answer]bool = .{ true, false, false, false },
    text_buf: [1024]u8 = undefined,
    len: usize = 0,
    prov: i32,
    dx: i32,
    /// Window 4: the gold, gems and items the effects gave.
    treasure: [6][2]i32 = @splat(.{ 0, 0 }),

    pub fn text(self: *const Show) []const u8 {
        return self.text_buf[0..self.len];
    }
};

pub const Battle = struct {
    /// Owner and slot of the hero.
    owner: u8,
    slot: u8,
    prov: i32,
    /// The slot of the site guard when the defender is one.
    site: ?usize = null,
    /// A guard of the hero's own province (task 5, no conquest).
    own: bool = false,
};

pub const Treasure = struct {
    level: i32,
    site: i32,
    gold: i32,
    gem: i32,
    items: [max_loot][2]i32 = @splat(.{ 0, 0 }),
    n_items: usize = 0,
};

pub const Request = union(enum) {
    dialog: *const Show,
    battle: Battle,
    treasure: Treasure,
};

const Stage = enum { start, answer, battle, treasure, done };

const Decision = enum { none, explore_found, explore_report, site_free, site_guard, pillage, pillage_final, attack };

const Job = struct {
    owner: u8,
    slot: u8,
    task: i32,
    stage: Stage = .start,
    decision: Decision = .none,
    /// The slot of the site in the province and the dialog its event uses.
    site_slot: usize = 0,
    site_dialog: usize = 0,
    /// After the dialog: open a treasure window.
    then_treasure: bool = false,
    /// The site was found with a guard.
    guarded: bool = false,
    /// The gold of a pillage.
    loot: i32 = 0,
    /// The site destroys itself after its treasure.
    destroy_after: bool = false,
};

pub const Runner = struct {
    gpa: std.mem.Allocator,
    pr: *const Proto,
    s: *Shard,
    r: *Rand,
    /// arDifficulty[..].iLoot of a human winner.
    loot_pct: i32 = 100,
    static_loot: bool = false,
    seed: u32 = 111,
    jobs: std.ArrayList(Job) = .empty,
    show: Show = undefined,
    /// A request the next call of `next` returns.
    pending: ?Request = null,
    /// The pending dialog waits for its answer.
    waiting: bool = false,
    desc_buf: [256]u8 = undefined,
    /// The site or own-province guard of the open battle.
    battle_site: ?usize = null,
    battle_own: bool = false,
    /// Provinces whose picture must be redrawn.
    touched: std.ArrayList(i32) = .empty,

    pub fn init(gpa: std.mem.Allocator, pr: *const Proto, s: *Shard, r: *Rand) Runner {
        return .{ .gpa = gpa, .pr = pr, .s = s, .r = r, .seed = newsave.seedOf(s) };
    }

    pub fn deinit(self: *Runner) void {
        self.jobs.deinit(self.gpa);
        self.touched.deinit(self.gpa);
    }

    pub fn idle(self: *const Runner) bool {
        return self.jobs.items.len == 0 and self.pending == null;
    }

    /// Queues the order of hero `k` when it still needs the screen.
    pub fn add(self: *Runner, k: usize) !void {
        const h = &self.s.heroes.items[k];
        if (!h.alive()) return;
        if (h.task < hero_mod.Task.explore or h.task > hero_mod.Task.attack) return;
        try self.jobs.append(self.gpa, .{ .owner = @intCast(h.owner), .slot = h.slot, .task = h.task });
    }

    /// The living hero of owner and slot; a burial shifts indices but not
    /// slots within the player's list.
    pub fn heroOf(s: *Shard, owner: u8, slot: u8) ?*Hero {
        for (s.heroes.items) |*h| {
            if (h.owner == @as(i32, @intCast(owner)) and h.slot == slot and h.alive()) return h;
        }
        return null;
    }

    fn hero(self: *Runner, j: *const Job) ?*Hero {
        return heroOf(self.s, j.owner, j.slot);
    }

    fn market(self: *Runner, j: *const Job) Market {
        return .{ .s = self.s, .pr = self.pr, .player = @intCast(j.owner) };
    }

    fn rnd(self: *Runner, max: i32) i32 {
        return @bitCast(self.r.random(@bitCast(max)));
    }

    fn touch(self: *Runner, prov: i32) void {
        if (std.mem.indexOfScalar(i32, self.touched.items, prov) == null) self.touched.append(self.gpa, prov) catch {};
    }

    fn dxOf(self: *const Runner, prov: i32) i32 {
        return if (self.s.provinces[@intCast(prov)].x + 400 > self.s.width()) -200 else 200;
    }

    /// The next request; null when a dialog waits for its answer or a
    /// battle / treasure window is open, or when every job is done.
    pub fn next(self: *Runner) ?Request {
        if (self.pending) |req| {
            self.pending = null;
            return req;
        }
        while (self.jobs.items.len > 0) {
            const j = &self.jobs.items[0];
            switch (j.stage) {
                .start => {
                    j.stage = .answer;
                    self.begin(j);
                    if (self.pending) |req| {
                        self.pending = null;
                        return req;
                    }
                    if (j.stage == .answer) j.stage = .done;
                },
                .answer => return null,
                .battle, .treasure => return null,
                .done => _ = self.jobs.orderedRemove(0),
            }
        }
        return null;
    }

    /// The answer (0-based) to the dialog `next` gave.
    pub fn answered(self: *Runner, a: usize) void {
        if (!self.waiting) return;
        self.waiting = false;
        if (self.jobs.items.len == 0) return;
        const j = &self.jobs.items[0];
        const answer = @min(a + 1, 4);
        switch (j.decision) {
            .explore_found => switch (answer) {
                1 => {
                    if (j.guarded) {
                        self.startSiteBattle(j);
                    } else self.siteEvent(j, j.site_slot, j.site_dialog);
                },
                3 => {
                    if (self.hero(j)) |h| h.task = hero_mod.Task.none;
                    j.stage = .done;
                },
                else => j.stage = .done,
            },
            .explore_report => {
                if (self.show.dialog == 21 and answer == 2) {
                    if (self.hero(j)) |h| h.task = hero_mod.Task.none;
                }
                j.stage = .done;
            },
            .site_free => {
                if (answer == 2) {
                    if (self.hero(j)) |h| h.task = hero_mod.Task.none;
                }
                if (j.then_treasure) {
                    self.treasureOf(j, j.site_slot);
                } else {
                    self.finishSite(j);
                    j.stage = .done;
                }
            },
            .site_guard => switch (answer) {
                1 => self.startSiteBattle(j),
                else => j.stage = .done,
            },
            .pillage => {
                if (answer == 2) {
                    if (self.hero(j)) |h| h.task = hero_mod.Task.none;
                }
                j.stage = .done;
            },
            .pillage_final => {
                if (self.hero(j)) |h| h.task = hero_mod.Task.none;
                if (self.provinceOf(j)) |pv| switch (answer) {
                    1 => {
                        pv.population = -9;
                        self.touch(pv.number);
                    },
                    2 => {
                        const m = self.market(j);
                        if (m.ruler().gem >= 15) {
                            m.ruler().gem -= 15;
                            pv.population = -19;
                            self.touch(pv.number);
                        }
                    },
                    else => {},
                };
                j.stage = .done;
            },
            .attack => switch (answer) {
                1 => self.startGuardBattle(j),
                else => j.stage = .done,
            },
            .none => j.stage = .done,
        }
    }

    pub fn battleDone(self: *Runner, result: i32) void {
        if (self.jobs.items.len == 0) return;
        const j = &self.jobs.items[0];
        if (j.stage != .battle) return;
        if (result == 1) {
            if (self.battle_site) |slot| {
                if (self.provinceOf(j)) |pv| {
                    // DismissSiteGuard, then the site's treasure.
                    pv.site_guards[slot] = -1;
                    pv.sites[slot].guard = 0;
                    const index = pv.sites[slot].index;
                    if (index > 0) _ = self.wonSite(j, slot, index);
                }
            } else if (self.battle_own) {
                if (self.provinceOf(j)) |pv| {
                    pv.guard = -1;
                    pv.defender = 0;
                }
            }
        }
        self.battle_site = null;
        self.battle_own = false;
        if (self.pending == null) j.stage = .done;
    }

    pub fn treasureDone(self: *Runner) void {
        if (self.jobs.items.len == 0) return;
        const j = &self.jobs.items[0];
        self.finishSite(j);
        j.stage = .done;
    }

    /// bSelfDestroy after the treasure of a site.
    fn finishSite(self: *Runner, j: *Job) void {
        if (!j.destroy_after) return;
        j.destroy_after = false;
        if (self.provinceOf(j)) |pv| {
            if (j.site_slot < pv.sites.len and pv.sites[j.site_slot].index > 0) {
                pv.sites[j.site_slot].index = 0;
                pv.site_guards[j.site_slot] = -1;
                self.touch(pv.number);
            }
        }
    }

    // -- the orders -------------------------------------------------------

    fn provinceOf(self: *Runner, j: *const Job) ?*Province {
        const h = self.hero(j) orelse return null;
        if (h.province <= 0 or h.province >= self.s.provinces.len) return null;
        return &self.s.provinces[@intCast(h.province)];
    }

    fn begin(self: *Runner, j: *Job) void {
        const h = self.hero(j) orelse return;
        const pv = &self.s.provinces[@intCast(h.province)];
        switch (j.task) {
            hero_mod.Task.explore => self.explore(j, h, pv),
            hero_mod.Task.pillage => self.pillage(j, h, pv),
            hero_mod.Task.enter => self.enter(j, h, pv),
            hero_mod.Task.attack => self.attack(j, h, pv),
            else => {},
        }
    }

    fn explore(self: *Runner, j: *Job, h: *Hero, pv: *Province) void {
        const pr = self.pr;
        const e: economy.Economy = .{ .s = self.s, .pr = pr };
        const m = self.market(j);
        if (pv.explore >= 100) {
            h.task = hero_mod.Task.none;
            j.stage = .done;
            return;
        }
        if (h.level < level_cap) h.exp += explore_exp + @divTrunc(h.level, 2);
        const bonus = h.heroBonus(pr, 996);
        var inc: i64 = 1;
        {
            const expl: i64 = @divTrunc(@as(i64, 15 + bonus) * @as(i64, 100 + e.param1(@intCast(j.owner), 25)), 100);
            const tmod: i64 = if (pv.terrain >= 0 and pv.terrain < pr.terrains.len) pr.terrains[@intCast(pv.terrain)].explore_mod else 100;
            inc += @divTrunc(@divTrunc(expl * 20, 20 + @as(i64, pv.explore)) * tmod, 100);
        }
        // The castle buildings with ability 25 (the original counts it both
        // through GetBldParam1(25) and here).
        var ibonus: i64 = 0;
        for (1..pr.inner_buildings.len) |i| {
            if (!built(self.s, j.owner, @intCast(i))) continue;
            const b = &pr.inner_buildings[i];
            for (b.abilities[0..b.n_abilities]) |a| {
                if (a.id == 25) ibonus += a.p1;
            }
        }
        if (ibonus != 0) inc += @divTrunc(inc * ibonus, 100);
        const old = pv.explore;
        var found: ?usize = null;
        var left = inc;
        while (left > 0 and found == null) : (left -= 1) {
            pv.explore += 1;
            m.holdings().explored += 1;
            for (pv.sites, 0..) |st, k| {
                if (st.index > 0 and st.pos == pv.explore) {
                    found = k;
                    break;
                }
            }
        }
        var i_expl: i64 = inc;
        if (i_expl + old > 100) i_expl = 100 - old;
        if (found) |slot| {
            if (pv.explore >= 100) pv.explore = 100;
            self.foundSite(j, h, pv, slot);
            return;
        }
        if (pv.explore >= 100) {
            pv.explore = 100;
            h.task = hero_mod.Task.none;
            self.setShow(26, &.{ .{ .s = h.nameText(pr) }, .{ .s = pv.name } }, .{ true, false, false, false }, pv.number);
            j.decision = .explore_report;
            return;
        }
        self.setShow(21, &.{ .{ .s = h.nameText(pr) }, .{ .d = i_expl }, .{ .s = pv.name } }, .{ true, true, false, false }, pv.number);
        j.decision = .explore_report;
    }

    fn foundSite(self: *Runner, j: *Job, h: *Hero, pv: *Province, slot: usize) void {
        const pr = self.pr;
        const st = &pv.sites[slot];
        const index = st.index;
        if (index <= 0 or index >= pr.sites.len) {
            j.stage = .done;
            return;
        }
        const site = &pr.sites[@intCast(index)];
        j.site_slot = slot;
        if (pv.site_guards[slot] >= 0) {
            // Охраняемый сайт: dialog 22, then the guard's event.
            self.setShow(22, &.{ .{ .s = pv.name }, .{ .s = h.nameText(pr) }, .{ .s = site.name } }, .{ true, true, true, false }, pv.number);
            j.decision = .explore_found;
            j.guarded = true;
            j.site_dialog = if (site.dialog > 0) @intCast(site.dialog) else 0;
            return;
        }
        if (site.guarded > 0 and !site.can_enter) {
            // Rolled as guarded but without a guard: an encounter.
            std.log.info("site {d}: the encounter is not ported", .{index});
            j.stage = .done;
            return;
        }
        self.setShow(22, &.{ .{ .s = pv.name }, .{ .s = h.nameText(pr) }, .{ .s = site.name } }, .{ true, true, true, false }, pv.number);
        j.decision = .explore_found;
        j.guarded = false;
        j.site_dialog = if (site.dialog2 > 0) @intCast(site.dialog2) else 0;
    }

    /// The abilities of a never-guarded site (dcEvent index 2/4).
    fn siteEvent(self: *Runner, j: *Job, slot: usize, dialog: usize) void {
        const pr = self.pr;
        const h = self.hero(j) orelse {
            j.stage = .done;
            return;
        };
        const pv = self.provinceOf(j) orelse {
            j.stage = .done;
            return;
        };
        if (slot >= pv.sites.len) {
            j.stage = .done;
            return;
        }
        const st = &pv.sites[slot];
        const index = st.index;
        if (index <= 0 or index >= pr.sites.len) {
            j.stage = .done;
            return;
        }
        const site = &pr.sites[@intCast(index)];
        const m = self.market(j);
        const ab: proto_mod.Ability3 = if (site.n_abilities > 0) site.abilities[0] else .{};
        const work = ab.p1 + ab.p2 * st.level;
        j.site_slot = slot;
        j.site_dialog = dialog;
        j.decision = .site_free;
        j.then_treasure = false;
        switch (ab.id) {
            9 => {
                h.exp += work;
                self.setShow(dialog, &.{ .{ .s = pv.name }, .{ .s = h.nameText(pr) }, .{ .d = work } }, .{ true, true, false, false }, pv.number);
            },
            10 => {
                m.ruler().gold += work;
                self.setShow(dialog, &.{ .{ .s = pv.name }, .{ .s = h.nameText(pr) }, .{ .d = work } }, .{ true, true, false, false }, pv.number);
            },
            11 => {
                m.ruler().gem += work;
                self.setShow(dialog, &.{ .{ .s = pv.name }, .{ .s = h.nameText(pr) }, .{ .d = work } }, .{ true, true, false, false }, pv.number);
            },
            12 => {
                self.setShow(dialog, &.{ .{ .s = pv.name }, .{ .s = h.nameText(pr) } }, .{ true, true, false, false }, pv.number);
                j.then_treasure = true;
            },
            16 => {
                std.log.info("site {d}: the arena is not ported", .{index});
                j.stage = .done;
                return;
            },
            else => {
                std.log.info("site {d}: the encounter is not ported", .{index});
                j.stage = .done;
                return;
            },
        }
        j.destroy_after = site.self_destroy;
        if (dialog == 0) {
            self.finishSite(j);
            j.stage = .done;
        }
    }

    /// Task 4: enter the site in `h.target`.
    fn enter(self: *Runner, j: *Job, h: *Hero, pv: *Province) void {
        const pr = self.pr;
        if (h.target < 0 or h.target >= province.max_site) {
            j.stage = .done;
            return;
        }
        const slot: usize = @intCast(h.target);
        h.task = hero_mod.Task.none;
        const st = &pv.sites[slot];
        if (st.index <= 0 or st.index >= pr.sites.len) {
            j.stage = .done;
            return;
        }
        const site = &pr.sites[@intCast(st.index)];
        j.site_slot = slot;
        j.site_dialog = if (site.dialog > 0) @intCast(site.dialog) else 0;
        if (pv.site_guards[slot] >= 0) {
            if (j.site_dialog == 0) {
                // A guard without a dialog: straight to the battle.
                self.startSiteBattle(j);
                return;
            }
            const g = &self.s.guards.items[@intCast(pv.site_guards[slot])];
            const desc = self.describeGuard(pr, &g.squad);
            self.setShow(j.site_dialog, &.{ .{ .s = h.nameText(pr) }, .{ .d = @intCast(countSquad(&g.squad)) }, .{ .s = desc } }, .{ true, true, false, true }, pv.number);
            j.decision = .site_guard;
            j.guarded = true;
            return;
        }
        self.siteEvent(j, slot, j.site_dialog);
    }

    fn startSiteBattle(self: *Runner, j: *Job) void {
        const pv = self.provinceOf(j) orelse {
            j.stage = .done;
            return;
        };
        j.stage = .battle;
        j.decision = .none;
        j.guarded = false;
        self.battle_site = j.site_slot;
        self.battle_own = false;
        self.pending = .{ .battle = .{ .owner = j.owner, .slot = j.slot, .prov = pv.number, .site = j.site_slot } };
    }

    /// The win of a site battle: its treasure and self-destruction.
    fn wonSite(self: *Runner, j: *Job, slot: usize, index: i32) bool {
        const pr = self.pr;
        const pv = self.provinceOf(j) orelse return false;
        const st = &pv.sites[slot];
        const site = &pr.sites[@intCast(index)];
        var shop = false;
        for (site.abilities[0..site.n_abilities]) |a| {
            if (a.id == 13) shop = true;
        }
        if (st.level > 0 or shop) {
            j.destroy_after = site.self_destroy;
            self.treasureOf(j, slot);
            return true;
        }
        j.destroy_after = site.self_destroy;
        self.finishSite(j);
        return false;
    }

    fn pillage(self: *Runner, j: *Job, h: *Hero, pv: *Province) void {
        const pr = self.pr;
        const m = self.market(j);
        const e: economy.Economy = .{ .s = self.s, .pr = pr };
        if (pv.population <= 0) {
            h.task = hero_mod.Task.none;
            j.stage = .done;
            return;
        }
        var power: i32 = h.health + h.heroBonus(pr, 50);
        for (1..hero_mod.max_squad) |i| {
            const u = h.squad[i] orelse continue;
            power += pr.units[@intCast(u.type)].level;
            if (u.checkAbility(pr, 58) + u.checkUpg(pr, 58) > 0) power += 1;
            if (u.checkAbility(pr, 50) + u.checkUpg(pr, 50) > 0) power += 1;
        }
        var killed = 5 + power + @divTrunc(pv.population * (power + 1), 100);
        killed = @min(killed, pv.population);
        var loot = @divTrunc(killed + self.rnd(killed), 7) + 20 + self.rnd(50);
        loot += @divTrunc(loot * h.heroBonus(pr, 982), 100);
        loot += @divTrunc(loot * e.param1(@intCast(j.owner), 41), 100);
        m.ruler().karma -= 5;
        m.ruler().gold += loot;
        m.holdings().pillage += loot;
        pv.population -= killed;
        h.exp += @min(1 + @divTrunc(killed, 40), 10 + h.level);
        event_mod.changeAttitude(pv, j.owner, -@min(3, 1 + @divTrunc(killed, 200)));
        if (pv.attitude[j.owner][0] < -10) pv.attitude[j.owner][0] = -10;
        self.touch(pv.number);
        j.loot = loot;
        if (pv.population > 0) {
            self.setShow(63, &.{ .{ .s = h.nameText(pr) }, .{ .s = pv.name }, .{ .d = loot } }, .{ true, true, false, false }, pv.number);
            j.decision = .pillage;
        } else {
            const gem_ok = m.ruler().gem >= 15;
            self.setShow(64, &.{ .{ .s = pv.name }, .{ .d = loot }, .{ .s = h.nameText(pr) } }, .{ true, gem_ok, true, false }, pv.number);
            j.decision = .pillage_final;
        }
    }

    /// Task 5: attack the guard of the hero's own province.
    fn attack(self: *Runner, j: *Job, h: *Hero, pv: *Province) void {
        const pr = self.pr;
        h.task = hero_mod.Task.none;
        if (pv.guard < 0 or pv.defender <= 0) {
            j.stage = .done;
            return;
        }
        self.setShow(32, &.{ .{ .s = h.nameText(pr) }, .{ .s = pv.name } }, .{ true, false, false, false }, pv.number);
        j.decision = .attack;
    }

    fn startGuardBattle(self: *Runner, j: *Job) void {
        const pv = self.provinceOf(j) orelse {
            j.stage = .done;
            return;
        };
        j.stage = .battle;
        j.decision = .none;
        self.battle_own = true;
        self.battle_site = null;
        self.pending = .{ .battle = .{ .owner = j.owner, .slot = j.slot, .prov = pv.number, .own = true } };
    }

    // -- treasures --------------------------------------------------------

    /// WindowTreasure of site `slot`: the loot and its effects.
    fn treasureOf(self: *Runner, j: *Job, slot: usize) void {
        const pr = self.pr;
        const pv = self.provinceOf(j) orelse {
            j.stage = .done;
            return;
        };
        if (slot >= pv.sites.len) {
            j.stage = .done;
            return;
        }
        const st = &pv.sites[slot];
        const index = st.index;
        if (index <= 0 or index >= pr.sites.len) {
            j.stage = .done;
            return;
        }
        const site = &pr.sites[@intCast(index)];
        var shop = false;
        for (site.abilities[0..site.n_abilities]) |a| {
            if (a.id == 13 or a.id == 22) shop = true;
        }
        var t: Treasure = .{ .level = st.level, .site = index, .gold = 0, .gem = 0 };
        if (shop) {
            t.gold = 400 + self.rnd(400);
            t.gem = 80 + self.rnd(80);
            var dropped: usize = @intCast(3 + self.rnd(3));
            if (st.level > 0 and @as(usize, @intCast(st.level)) < self.s.shops.items.len) {
                const sh = &self.s.shops.items[@intCast(st.level)];
                var roll: unit_mod.Roller = .{};
                var n: usize = 0;
                for (sh.items) |it| {
                    if (it[0] <= 0 or n == roll.ids.len) continue;
                    roll.ids[n] = it[0];
                    roll.weights[n] = 10;
                    n += 1;
                }
                if (n == 0) dropped = 0;
                if (dropped > n) dropped = n;
                var i: usize = 0;
                while (i < dropped and i < max_loot) : (i += 1) {
                    const id = roll.rollExclude(self.r);
                    t.items[i] = .{ id, pr.items[@intCast(id)].durability };
                    t.n_items += 1;
                }
            }
        } else {
            const lvl = @max(st.level, 1);
            t.gold = lvl * 50 + self.rnd(1 + lvl * 50);
            t.gem = (lvl - 1) * 5 + self.rnd(10 + lvl);
            var dropped: usize = 0;
            var drop_level: i32 = 1;
            switch (lvl) {
                1 => dropped = @intCast(self.rnd(2)),
                2 => if (self.rnd(10) > 4) {
                    dropped = 2;
                    drop_level = 1;
                } else {
                    dropped = 1;
                    drop_level = 2;
                },
                3 => if (self.rnd(10) > 2) {
                    dropped = 2;
                    drop_level = 2;
                } else {
                    dropped = 1;
                    drop_level = 3;
                },
                4 => if (self.rnd(10) > 6) {
                    dropped = 2;
                    drop_level = 2;
                } else {
                    dropped = 1;
                    drop_level = 3;
                },
                5 => if (self.rnd(10) > 2) {
                    dropped = 2;
                    drop_level = 3;
                } else {
                    dropped = 1;
                    drop_level = 4;
                },
                6 => if (self.rnd(10) > 6) {
                    dropped = 2;
                    drop_level = 3;
                } else {
                    dropped = 1;
                    drop_level = 4;
                },
                7 => {
                    dropped = 1;
                    drop_level = 4;
                },
                8 => if (self.rnd(10) > 2) {
                    dropped = 2;
                    drop_level = 4;
                } else {
                    dropped = 1;
                    drop_level = 5;
                },
                9 => if (self.rnd(10) > 4) {
                    dropped = 2;
                    drop_level = 4;
                } else {
                    dropped = 1;
                    drop_level = 5;
                },
                10 => if (self.rnd(10) > 6) {
                    dropped = 2;
                    drop_level = 4;
                    t.gold += 200;
                    t.gem += 25;
                } else {
                    dropped = 1;
                    drop_level = 5;
                },
                11 => {
                    dropped = 1;
                    drop_level = 5;
                },
                12 => if (self.rnd(10) > 2) {
                    dropped = 1;
                    drop_level = 5;
                    t.gold += 400;
                    t.gem += 50;
                } else {
                    dropped = 1;
                    drop_level = 6;
                },
                13 => if (self.rnd(10) > 6) {
                    dropped = 2;
                    drop_level = 5;
                } else {
                    dropped = 1;
                    drop_level = 6;
                },
                else => {
                    dropped = @intCast(1 + @divTrunc(lvl - 13, 2) + self.rnd(@divTrunc(lvl - 12, 2)));
                    drop_level = 6;
                },
            }
            if (dropped > max_loot) dropped = max_loot;
            var roll: unit_mod.Roller = .{};
            var n: usize = 0;
            for (pr.items[1..], 1..) |it, k| {
                if (it.shop_level != drop_level or n == roll.ids.len) continue;
                if (!stocked(self.s, pr, j.owner, @intCast(k), it)) continue;
                roll.ids[n] = @intCast(k);
                roll.weights[n] = 7 - it.rarity;
                n += 1;
            }
            if (n == 0) dropped = 0;
            var i: usize = 0;
            while (i < dropped and i < max_loot) : (i += 1) {
                const id = roll.rollExclude(self.r);
                const dur = pr.items[@intCast(id)].durability;
                var d = dur;
                if (dur > 1 and self.rnd(30) > 10) d = @divTrunc(dur, 2) + self.rnd(1 + @divTrunc(dur, 2));
                t.items[i] = .{ id, d };
                t.n_items += 1;
            }
            if (self.hero(j)) |h| {
                if (h.squad[0] != null and h.squad[0].?.life > 0) {
                    const b = h.heroBonus(pr, 987);
                    t.gold += @divTrunc(@divTrunc(t.gold * b, 10), 10);
                    t.gem += @divTrunc(@divTrunc(t.gem * b, 10), 10);
                }
            }
            if (self.loot_pct != 100) {
                t.gold = @divTrunc(t.gold * self.loot_pct, 100);
                t.gem = @divTrunc(t.gem * self.loot_pct, 100);
            }
            for (site.abilities[0..site.n_abilities]) |a| {
                if (a.id == 3 and a.p1 == 2) {
                    t.gem += @divTrunc(t.gold * 2, 5);
                    t.gold = 0;
                }
            }
        }
        self.applyTreasure(j, t);
        j.stage = .treasure;
        self.pending = .{ .treasure = t };
    }

    fn applyTreasure(self: *Runner, j: *Job, t: Treasure) void {
        const pr = self.pr;
        const m = self.market(j);
        m.ruler().gold += t.gold;
        m.ruler().gem += t.gem;
        const h = self.hero(j) orelse return;
        for (t.items[0..t.n_items]) |it| {
            if (it[0] <= 0) continue;
            if (hero_mod.takeLoot(pr, h, &h.inventory, it[0], it[1])) continue;
            var placed = false;
            for (&m.ruler().holdings.treasury) |*slot| {
                if (slot[0] == 0) {
                    slot.* = it;
                    placed = true;
                    break;
                }
            }
            if (!placed) std.log.info("treasure: no room for item {d}", .{it[0]});
        }
    }

    // -- helpers ----------------------------------------------------------

    fn setShow(self: *Runner, dialog: usize, args: []const event_mod.Arg, enabled: [proto_mod.max_answer]bool, prov: i32) void {
        const pr = self.pr;
        if (dialog == 0 or dialog >= pr.dialogs.len) return;
        var sh: Show = .{ .dialog = dialog, .enabled = .{ false, false, false, false }, .prov = prov, .dx = self.dxOf(prov) };
        const d = &pr.dialogs[@intCast(dialog)];
        sh.len = event_mod.format(&sh.text_buf, d.text, args).len;
        for (d.answers, 0..) |a, i| {
            if (i >= proto_mod.max_answer) break;
            if (a.len > 1 and a[1] == '#') continue;
            sh.enabled[i] = enabled[i];
        }
        self.show = sh;
        self.waiting = true;
        self.pending = .{ .dialog = &self.show };
    }

    /// MakeDescription: the living units' types.
    fn describeGuard(self: *Runner, pr: *const Proto, squad: *const [hero_mod.max_squad]?Unit) []const u8 {
        var w: std.Io.Writer = .fixed(&self.desc_buf);
        var first = true;
        for (squad) |u| {
            const unit = u orelse continue;
            if (unit.life <= 0) continue;
            if (!first) w.writeAll(", ") catch break;
            w.writeAll(pr.units[@intCast(unit.type)].name) catch break;
            first = false;
        }
        if (first) w.writeAll("?") catch {};
        return w.buffered();
    }

    fn countSquad(squad: *const [hero_mod.max_squad]?Unit) usize {
        var n: usize = 0;
        for (squad) |u| {
            if (u) |x| n += @intFromBool(x.life > 0);
        }
        return n;
    }
};

/// arBld: the castle building of player `k`.
fn built(s: *Shard, k: u8, id: i32) bool {
    if (k >= s.castle.len) return false;
    const b = s.castle[k];
    return id >= 0 and id < b.len and b[@intCast(id)];
}

/// The stocks a human keeps: contracts (11) need a defender still on sale,
/// blueprints (12) an outer building, ritual scrolls (13) a ritual.
fn stocked(s: *Shard, pr: *const Proto, k: u8, item: i32, it: proto_mod.Item) bool {
    if (item <= 0 or item >= pr.items.len) return false;
    const need: ?i32 = switch (it.class) {
        11, 12, 13 => if (it.n_effects > 0) it.effects[0].p1 else null,
        else => null,
    };
    const p = need orelse return true;
    const sv = s.saved orelse return true;
    if (k >= sv.lists.len) return true;
    const list = sv.lists[k];
    var off: usize = 0;
    for ([_]struct { n: usize, size: usize, class: i32 }{
        .{ .n = pr.defenders.len, .size = 4, .class = 11 },
        .{ .n = pr.outer_buildings.len, .size = 4, .class = 12 },
        .{ .n = pr.rituals.len, .size = 8, .class = 13 },
    }) |l| {
        if (off + 4 > list.len) return true;
        const n: i32 = std.mem.readInt(i32, list[off..][0..4], .little);
        if (n < 0) return true;
        const end = off + 4 + @as(usize, @intCast(n + 1)) * l.size;
        if (end > list.len) return true;
        if (it.class == l.class and p >= 0 and p <= n) {
            const at = off + 4 + @as(usize, @intCast(p)) * l.size;
            return std.mem.readInt(i32, list[at..][0..4], .little) >= 0;
        }
        off = end;
    }
    return true;
}
