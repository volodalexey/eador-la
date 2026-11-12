//! The deterministic part of the turn change: BeginMainTurn's income,
//! growth, upkeep, healing and corruption (MainProcess.cpp:8407-9175) and
//! the attitude timers of EndMainTurn (MainProcess.cpp:6596), with the
//! province formulas of Province.cpp (GetGoldIncome :366, GetGemIncome
//! :501, GetGrow :685, GetMood :752, GetHeal :642, GetCorruption :3883)
//! and dcHero::RestoreUnits (Hero.cpp:3393).
//!
//! The random events are in event.zig (Economy.events).
//!
//! Not ported: rituals (GetRitualBonus is 0), unrest, sieges, trade, quests that
//! fail, resurrection, the defenders' own units (pGuard->RestoreUnits),
//! the overpopulation message (dialog 25).
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const mapgen = @import("mapgen.zig");
const Shard = mapgen.Shard;
const province = @import("province.zig");
const Province = province.Province;
const terrain = province.terrain;
const hero_mod = @import("hero.zig");
const Hero = hero_mod.Hero;

/// ATTITUDE_TIME.
pub const attitude_time = 20;

/// Building, site and defender ability ids used here.
const ab = struct {
    const castle_gold_gem = 5;
    const castle_resource_gold = 9;
    const castle_grow = 3;
    const castle_mood = 12;
    const castle_heal = 21;
    const castle_income_perc = 22;
    const castle_opponent_gold = 29;
    const castle_profit = 32;
    const castle_terrain_gold = 14;
    const castle_race_gold = 49;
    const castle_race_gem = 55;
    const castle_corrupt = 51;
    const outer_mood = 3;
    const outer_gold = 6;
    const outer_corrupt = 9;
    const site_resource = 1;
    const site_income = 2;
    const site_gold_perc = 6;
    const site_grow = 7;
    const site_mood = 19;
    const site_corrupt = 20;
    const def_income_perc = 1;
    const def_mood = 2;
    const def_heal = 6;
    const def_fourage = 4;
    const castle_fourage = 28;
    const def_resource_gold = 7;
    const def_grow = 9;
    const unit_heal = 35;
    const unit_heal2 = 48;
    const unit_healer = 24;
    const unit_pay_perc = 85;
    const unit_pay_less = 54;
    const unit_gold_skill = 58;
    const undead_heal = 21;
    const hero_pay_perc = 997;
    const hero_gold = 81;
    const hero_gem = 82;
};

/// Unit kinds: 2 undead (healed by the hero's skill), 4 not healed by the
/// squad's healers.
const kind_undead = 2;
const kind_no_squad_heal = 4;

pub const Economy = struct {
    s: *Shard,
    pr: *const Proto,
    /// eador.cfg MAX_PROFIT.
    max_profit: i32 = 500,
    /// The random events of BeginMainTurn; null leaves them out.
    events: ?*@import("event.zig").Runner = null,

    fn castle(self: *const Economy, player: i32) []const bool {
        if (player <= 0 or player > mapgen.max_players) return &.{};
        return self.s.castle[@intCast(player)];
    }

    /// The castle buildings in [first, last] (inner_build.var indices):
    /// p1 or p2 of ability `id`, only where p1 == `match` if given.
    fn castleSum(self: *const Economy, player: i32, first: usize, last: usize, id: i32, comptime field: enum { p1, p2 }, match: ?i32) i32 {
        const bld = self.castle(player);
        var r: i32 = 0;
        var i = first;
        while (i <= last and i < bld.len and i < self.pr.inner_buildings.len) : (i += 1) {
            if (!bld[i]) continue;
            const b = &self.pr.inner_buildings[i];
            for (b.abilities[0..b.n_abilities]) |a| {
                if (a.id != id) continue;
                if (match) |m| if (a.p1 != m) continue;
                r += @field(a, @tagName(field));
            }
        }
        return r;
    }

    /// iMaxInnerBld: the last inner_build.var record.
    fn maxInner(self: *const Economy) usize {
        return self.pr.inner_buildings.len -| 1;
    }

    /// GetBldParam1 / GetBldParam2 (1..iMaxInnerBld).
    pub fn param1(self: *const Economy, player: i32, id: i32) i32 {
        return self.castleSum(player, 1, self.maxInner(), id, .p1, null);
    }

    pub fn param2(self: *const Economy, player: i32, id: i32, match: ?i32) i32 {
        return self.castleSum(player, 1, self.maxInner(), id, .p2, match);
    }

    /// The loops of GetGrow, GetHeal and GetStable: 1..iMaxInnerBld-1.
    fn castleShort(self: *const Economy, player: i32, id: i32, comptime field: enum { p1, p2 }) i32 {
        return self.castleSum(player, 1, self.maxInner() -| 1, id, if (field == .p1) .p1 else .p2, null);
    }

    /// dcPlayer::CompareBldParam1: a castle building with ability `id`
    /// and p1 == `index`.
    pub fn compareBldParam1(self: *const Economy, player: i32, id: i32, index: i32) bool {
        const bld = self.castle(player);
        for (bld, 0..) |built, i| {
            if (!built or i == 0 or i >= self.pr.inner_buildings.len) continue;
            const b = &self.pr.inner_buildings[i];
            for (b.abilities[0..b.n_abilities]) |a| if (a.id == id and a.p1 == index) return true;
        }
        return false;
    }

    fn has(self: *const Economy, player: i32, bld: i32) bool {
        const b = self.castle(player);
        return bld >= 0 and bld < b.len and b[@intCast(bld)];
    }

    fn isHome(self: *const Economy, pv: *const Province) bool {
        return pv.player >= 0 and pv.player <= mapgen.max_players and self.s.capitals[@intCast(pv.player)] == pv.number;
    }

    fn outer(self: *const Economy, pv: *const Province) OuterIter {
        return .{ .pr = self.pr, .pv = pv };
    }

    const OuterIter = struct {
        pr: *const Proto,
        pv: *const Province,
        k: usize = 0,

        fn next(it: *OuterIter) ?*const proto_mod.OuterBuilding {
            while (it.k < province.max_bld) {
                const b = it.pv.buildings[it.k];
                it.k += 1;
                if (b > 0 and b < it.pr.outer_buildings.len) return &it.pr.outer_buildings[@intCast(b)];
            }
            return null;
        }
    };

    fn defender(self: *const Economy, pv: *const Province) ?*const proto_mod.Defender {
        if (pv.defender <= 0 or pv.defender >= self.pr.defenders.len) return null;
        return &self.pr.defenders[@intCast(pv.defender)];
    }

    /// The working sites of GetSiteParam1 and the income loops: guarded
    /// ones that work guarded, free ones that work free, found or working
    /// hidden.
    pub fn siteWorks(self: *const Economy, pv: *const Province, st: province.Site, guard_rule: bool) bool {
        if (st.index <= 0 or st.index >= self.pr.sites.len) return false;
        const site = &self.pr.sites[@intCast(st.index)];
        const g = if (guard_rule)
            (st.guard > 0 and site.work_guard) or (st.guard == 0 and site.work_free)
        else
            (st.guard == 0 or site.work_guard) and (st.guard > 0 or site.work_free);
        return g and (st.pos <= pv.explore or site.work_hide);
    }

    /// GetSiteParam1.
    pub fn siteParam1(self: *const Economy, pv: *const Province, id: i32) i32 {
        var r: i32 = 0;
        for (pv.sites) |st| {
            if (!self.siteWorks(pv, st, false)) continue;
            const site = &self.pr.sites[@intCast(st.index)];
            for (site.abilities[0..site.n_abilities]) |a| {
                if (a.id == id) r += a.p1;
            }
        }
        return r;
    }

    /// dcProvince::GetResource(Status): the resource of the first site
    /// that gives one; Status 1 needs it found and unguarded.
    pub fn resource(self: *const Economy, pv: *const Province, status: i32) i32 {
        for (pv.sites) |st| {
            if (st.index <= 0 or st.index >= self.pr.sites.len) continue;
            const site = &self.pr.sites[@intCast(st.index)];
            for (site.abilities[0..site.n_abilities]) |a| {
                if (a.id != ab.site_resource) continue;
                if (status < 0) return a.p1;
                if (st.pos <= pv.explore and (status == 0 or st.guard == 0)) return a.p1;
            }
        }
        return 0;
    }

    /// dcPlayer::GetResource without trade: the player's provinces with
    /// resource `index`.
    pub fn playerResource(self: *const Economy, player: i32, index: i32) i32 {
        var r: i32 = 0;
        for (self.s.provinces[1..]) |*pv| {
            if (pv.player == player and self.resource(pv, 1) == index) r += 1;
        }
        return r;
    }

    /// GetLevel (1..MAX_PROVINCE_LEVEL).
    pub fn level(self: *const Economy, pop: i32) i32 {
        const lv = self.pr.province_levels;
        for (1..proto_mod.max_province_level) |i| {
            if (pop <= lv[i].max_population) return @intCast(i);
        }
        return proto_mod.max_province_level;
    }

    fn race(self: *const Economy, pv: *const Province) *const proto_mod.Race {
        return &self.pr.races[@intCast(std.math.clamp(pv.race, 0, @as(i32, @intCast(self.pr.races.len)) - 1))];
    }

    /// GetMood (rituals not ported).
    pub fn mood(self: *const Economy, pv: *const Province) i32 {
        if (pv.population <= 0 or pv.player == 0) return 0;
        const pl = pv.player;
        var r = pv.attitude[@intCast(pl)][0];
        if (self.isHome(pv)) {
            r += self.param1(pl, ab.castle_mood);
        } else {
            var it = self.outer(pv);
            while (it.next()) |b| {
                r += b.unrest;
                for (b.abilities[0..b.n_abilities]) |a| {
                    if (a.id == ab.outer_mood and self.has(pl, a.p2)) r += a.p1;
                }
            }
        }
        r += self.param2(pl, ab.castle_mood, null);
        if (self.defender(pv)) |d| for (d.abilities[0..d.n_abilities]) |a| {
            if (a.id == ab.def_mood) r += a.p1;
        };
        const karma = self.s.rulers[@intCast(pl)].karma;
        const rk = self.race(pv).karma;
        if (@abs(karma) >= 50 and rk != 0) r += std.math.sign(karma) * std.math.sign(rk);
        r += self.siteParam1(pv, ab.site_mood);
        return r;
    }

    /// GetHeal.
    pub fn heal(self: *const Economy, pv: *const Province) i32 {
        var r: i32 = 0;
        if (self.isHome(pv)) {
            r += self.castleShort(pv.player, ab.castle_heal, .p1);
        } else {
            var it = self.outer(pv);
            while (it.next()) |b| r += b.heal;
        }
        if (self.defender(pv)) |d| for (d.abilities[0..d.n_abilities]) |a| {
            if (a.id == ab.def_heal) r += a.p1;
        };
        return r;
    }

    /// GetGrow.
    pub fn grow(self: *const Economy, pv: *const Province) i32 {
        if (pv.population <= 0) return 1;
        const rc = self.race(pv);
        var r: i32 = 10 + @divTrunc(pv.population, 50);
        r = @divTrunc(r * rc.grow, 100);
        if (pv.terrain != rc.terra) r = @divTrunc(r * self.pr.terrains[@intCast(pv.terrain)].population_mod, 100);
        r += @divTrunc(@as(i32, @intCast(@abs(r))) * self.mood(pv), 10);
        var add: i32 = 0;
        if (self.isHome(pv)) {
            add += self.castleShort(pv.player, ab.castle_grow, .p1);
        } else {
            var it = self.outer(pv);
            while (it.next()) |b| add += b.grow;
        }
        add += self.castleShort(pv.player, ab.castle_grow, .p2);
        if (self.defender(pv)) |d| for (d.abilities[0..d.n_abilities]) |a| {
            if (a.id == ab.def_grow) add += a.p1;
        };
        add += self.siteParam1(pv, ab.site_grow);
        r += add;
        if (r > 0) r += @divTrunc(r * add, 100);
        return r;
    }

    /// GetGoldIncome.  GetUnitSkill(58) counts the garrison too, which the
    /// loader does not keep.
    pub fn goldIncome(self: *const Economy, pv: *const Province) i32 {
        if (pv.terrain == terrain.water or pv.terrain == terrain.mountain) return 0;
        const pl = pv.player;
        var r: i32 = if (pv.gold_inc <= 0) 1 else pv.gold_inc;
        r += @divTrunc(r * self.pr.province_levels[@intCast(self.level(pv.population))].income, 100);
        if (r == 0) return 0;
        r = @divTrunc(r * self.race(pv).gold, 100);
        r += @divTrunc(r * self.mood(pv), 20);
        r -= @divTrunc(r * (100 - pv.explore), 500);
        if (pv.explore == 100) r += 2;
        const res = self.resource(pv, 1);
        if (!self.isHome(pv)) {
            var it = self.outer(pv);
            while (it.next()) |b| {
                r += b.gold_add;
                for (b.abilities[0..b.n_abilities]) |a| {
                    if (a.id == ab.outer_gold and self.has(pl, a.p2)) r += a.p1;
                }
                for (b.res_bonus[0..b.n_res_bonus]) |rb| {
                    if (rb.id > 0 and rb.id == res) r += rb.value;
                }
            }
        }
        // Castle buildings 0..iMaxInnerBld-1.
        {
            const bld = self.castle(pl);
            var i: usize = 0;
            while (i < self.maxInner() and i < bld.len) : (i += 1) {
                if (!bld[i]) continue;
                const b = &self.pr.inner_buildings[i];
                for (b.abilities[0..b.n_abilities]) |a| {
                    if (a.id == ab.castle_terrain_gold and (a.p1 == 0 or a.p1 == pv.terrain or (a.p1 == 8 and pv.coastal))) r += a.p2;
                }
            }
        }
        r += self.param2(pl, ab.castle_race_gold, pv.race);
        for (pv.sites) |st| {
            if (!self.siteWorks(pv, st, true)) continue;
            const site = &self.pr.sites[@intCast(st.index)];
            for (site.abilities[0..site.n_abilities]) |a| {
                if (a.id == ab.site_income) r += a.p1;
            }
        }
        var it = self.outer(pv);
        while (it.next()) |b| r += @divTrunc(r * b.gold_perc, 100);
        if (self.defender(pv)) |d| for (d.abilities[0..d.n_abilities]) |a| {
            if (a.id == ab.def_resource_gold and a.p1 == res) r += a.p2;
            if (a.id == ab.def_income_perc) r += @divTrunc(r * a.p1, 100);
        };
        r += @divTrunc(r * self.siteParam1(pv, ab.site_gold_perc), 100);
        r -= @divTrunc(r * self.unitSkill(pv, ab.unit_gold_skill), 100);
        return @max(r, 0);
    }

    /// GetGemIncome.  The mood is added twice, as in the C++ code.
    pub fn gemIncome(self: *const Economy, pv: *const Province) i32 {
        if (pv.terrain == terrain.water or pv.terrain == terrain.mountain) return 0;
        if (pv.race < 0) return 0;
        const pl = pv.player;
        var r = pv.gem_inc;
        var perc: i32 = 0;
        r += @divTrunc(r * self.pr.province_levels[@intCast(self.level(pv.population))].income, 100);
        r = @divTrunc(r * self.race(pv).gem, 100);
        const m = self.mood(pv);
        r += @divTrunc(r * m, 20);
        r -= @divTrunc(r * (100 - pv.explore), 500);
        r += @divTrunc(r * m, 20);
        if (!self.isHome(pv)) {
            var it = self.outer(pv);
            while (it.next()) |b| {
                r += b.gem_add;
                perc += b.gem_perc;
            }
        }
        r += self.param2(pl, ab.castle_race_gem, pv.race);
        for (pv.sites) |st| {
            if (!self.siteWorks(pv, st, false)) continue;
            const site = &self.pr.sites[@intCast(st.index)];
            for (site.abilities[0..site.n_abilities]) |a| {
                if (a.id == ab.site_income) r += a.p2;
            }
        }
        if (self.defender(pv)) |d| for (d.abilities[0..d.n_abilities]) |a| {
            if (a.id == ab.def_income_perc) r += @divTrunc(r * a.p2, 100);
        };
        r += @divTrunc(r * perc, 100);
        return r;
    }

    /// GetCorruption.
    pub fn corruption(self: *const Economy, pv: *const Province) i32 {
        var bld: i32 = 0;
        var it = self.outer(pv);
        while (it.next()) |b| for (b.abilities[0..b.n_abilities]) |a| {
            if (a.id == ab.outer_corrupt) bld += a.p1;
        };
        return @divTrunc((self.race(pv).corrupt + self.siteParam1(pv, ab.site_corrupt)) * (100 + bld), 100);
    }

    /// GetUnitSkill: the garrison and the owner's heroes in the province.
    /// The garrison's units have no pHero, so CheckUpg adds no hero bonus.
    fn unitSkill(self: *const Economy, pv: *const Province, id: i32) i32 {
        var r: i32 = 0;
        if (pv.garrison >= 0) {
            for (self.s.garrisons.items[@intCast(pv.garrison)].squad) |slot| {
                const u = &(slot orelse continue);
                r += u.checkAbility(self.pr, id) + u.checkUpg(self.pr, id);
            }
        }
        for (self.s.heroes.items) |*h| {
            if (h.owner != pv.player or h.province != pv.number or h.dead > 0) continue;
            for (h.squad, 0..) |slot, k| {
                const u = &(slot orelse continue);
                r += u.checkAbility(self.pr, id) + self.upg(h, k, id);
            }
        }
        return r;
    }

    /// dcUnit::CheckUpg of squad slot `k`: slot 0 has pHero set.
    fn upg(self: *const Economy, h: *const Hero, k: usize, id: i32) i32 {
        const u = &(h.squad[k] orelse return 0);
        var r = u.checkUpg(self.pr, id);
        if (k == 0) r += h.heroBonus(self.pr, id);
        return r;
    }

    fn maxLife(self: *const Economy, h: *const Hero, k: usize) i32 {
        const u = &(h.squad[k] orelse return 0);
        return self.pr.units[@intCast(u.type)].life + self.upg(h, k, 1) + u.checkAbility(self.pr, 1);
    }

    /// dcUnit::GetGoldPayment and GetGemPayment.
    pub fn payment(self: *const Economy, h: *const Hero, k: usize) [2]i32 {
        const u = &(h.squad[k] orelse return .{ 0, 0 });
        const t = &self.pr.units[@intCast(u.type)];
        var gold = t.gold_payment;
        var gem = t.gem_payment;
        for (u.medals) |m| {
            if (m <= 0 or m >= self.pr.medals.len) continue;
            gold += self.pr.medals[@intCast(m)].gold_spent;
            gem += self.pr.medals[@intCast(m)].gem_spent;
        }
        gold += @divTrunc(gold * self.upg(h, k, ab.unit_pay_perc), 100);
        gold -= self.upg(h, k, ab.unit_pay_less);
        return .{ @max(gold, 0), gem };
    }

    /// dcHero::RestoreUnits(bHomeLand, ProvBonus, false).
    pub fn restoreUnits(self: *const Economy, h: *Hero, homeland: bool, prov_bonus: i32) void {
        const pr = self.pr;
        var squad_heal: i32 = 0;
        for (h.squad, 0..) |slot, k| {
            const u = &(slot orelse continue);
            if (u.life <= 0) continue;
            const on = if (k == 0 and h.class1 > 0) h.heroBonus(pr, ab.unit_healer) > 0 else u.checkAbility(pr, ab.unit_healer) + self.upg(h, k, ab.unit_healer) > 0;
            if (on) squad_heal += 1;
        }
        for (&h.squad, 0..) |*slot, k| {
            const u = &(slot.* orelse continue);
            if (u.life <= 0) continue;
            const kind = pr.units[@intCast(u.type)].unit_kind;
            const max = self.maxLife(h, k);
            if (kind == kind_undead) {
                if (h.class1 > 0) u.life = @min(u.life + @divTrunc(max * h.checkSkill(ab.undead_heal), 10), max);
                continue;
            }
            const bonus = if (k > 0 or h.class1 == 0)
                u.checkAbility(pr, ab.unit_heal) + self.upg(h, k, ab.unit_heal) + (u.checkAbility(pr, ab.unit_heal2) + self.upg(h, k, ab.unit_heal2)) * 2
            else
                h.health + h.heroBonus(pr, ab.unit_heal) + h.heroBonus(pr, ab.unit_heal2) * 2;
            const squad = if (kind != kind_undead and kind != kind_no_squad_heal) squad_heal else 0;
            if (homeland) {
                if (u.life < max) {
                    const add = @max(@divTrunc(max * (10 + prov_bonus), 100), 1) + bonus + squad;
                    u.life = @min(u.life + add, max);
                }
                if (u.morale < 0) u.morale += 1;
                if (u.morale > 0) u.morale -= 1;
            } else {
                if (u.life < max) u.life = @min(u.life + bonus + squad, max);
                if (u.morale > -15) u.morale -= 1;
            }
        }
    }

    /// The attitude timers of EndMainTurn: every ATTITUDE_TIME turns the
    /// attitude to a ruler moves one step towards 0.
    pub fn endTurn(self: *Economy) void {
        for (self.s.provinces[1..]) |*pv| {
            for (pv.attitude[1..]) |*a| {
                if (a[0] == 0) continue;
                a[1] -= 1;
                if (a[1] == 0) {
                    a[0] -= std.math.sign(a[0]);
                    a[1] = attitude_time;
                }
            }
        }
    }

    /// BeginMainTurn(player) without events: the income lands in Gold/Gem
    /// and GoldInc/GemInc.
    pub fn beginTurn(self: *Economy, player: usize, current_turn: i32) void {
        const pl: i32 = @intCast(player);
        const pr = self.pr;
        const r = &self.s.rulers[player];
        var gold_inc: i32 = 0;
        var gold_spent: i32 = 0;
        var gem_inc: i32 = 0;
        var gem_spent: i32 = 0;
        var prov_num: i32 = 0;
        for (self.s.provinces[1..]) |*pv| {
            if (pv.player != pl) continue;
            if (pv.population > 0) prov_num += self.corruption(pv);
            // DefFourage: the defender's upkeep cut for the garrison.
            var fourage: i32 = 0;
            if (self.defender(pv)) |d| for (d.abilities[0..d.n_abilities]) |a| {
                if (a.id == ab.def_fourage) fourage = a.p1;
            };
            const besieged = @import("movement.zig").siege(self.s, pv.number) >= 0;
            if (besieged) fourage = 0;
            if (pv.garrison >= 0) {
                const g = &self.s.garrisons.items[@intCast(pv.garrison)];
                var pay: i32 = 0;
                for (0..hero_mod.max_squad) |k| {
                    const p = self.payment(g, k);
                    pay += p[0];
                    gem_spent += p[1];
                }
                self.restoreUnits(g, true, self.heal(pv) + 10);
                pay -= @divTrunc(pay * (fourage + self.param1(pl, ab.castle_fourage)), 100);
                if (pay > 0) gold_spent += pay;
            }
            income: {
                if (besieged) break :income;
                gold_inc += self.goldIncome(pv);
                gem_inc += self.gemIncome(pv);
                const old = self.level(pv.population);
                if (old < proto_mod.max_province_level) {
                    const g = self.grow(pv);
                    if (pv.explore >= pr.province_levels[@intCast(old)].explored or g < 0) {
                        if (!(g < 0 and pv.population + g <= 0)) {
                            pv.population += g;
                            pv.warning = true;
                        }
                    } else if (pv.warning and r.kind == .human) {
                        // Dialog 25 "the province is overpopulated" is not shown.
                        pv.warning = false;
                    }
                }
                if (self.defender(pv)) |d| if (r.kind == .human and d.gold_payment >= 0) {
                    gold_spent += d.gold_payment;
                    gem_spent += d.gem_payment;
                };
            }
            // "События..." (MainProcess.cpp:8646): after the garrison.
            if (self.events) |ev| ev.atProvince(player, pv.number, current_turn);
        }
        var perc: i32 = 0;
        for (self.castle(pl), 0..) |on, i| {
            if (!on or i == 0 or i > self.maxInner()) continue;
            const b = &pr.inner_buildings[i];
            for (b.abilities[0..b.n_abilities]) |a| switch (a.id) {
                ab.castle_gold_gem => {
                    gold_inc += a.p1;
                    gem_inc += a.p2;
                },
                ab.castle_resource_gold => gold_inc += self.playerResource(pl, a.p1) * a.p2,
                ab.castle_income_perc => perc += a.p1,
                else => {},
            };
        }
        const profit = @min(@divTrunc(r.gold * self.param1(pl, ab.castle_profit), 100), self.max_profit);
        gold_inc += profit + r.gold_add;
        gem_inc += r.gem_add;
        for (1..self.s.n_players + 1) |o| {
            if (o == player or !self.s.in_game[o]) continue;
            gold_inc += self.param1(@intCast(o), ab.castle_opponent_gold);
        }
        for (self.s.heroes.items) |*h| {
            if (h.owner != pl) continue;
            if (h.dead > 0) {
                h.dead -= 1;
            } else if (h.task == hero_mod.Task.explore and self.s.provinces[@intCast(h.province)].explore >= 100) {
                h.task = hero_mod.Task.none;
            }
            var pay: i32 = 0;
            for (0..hero_mod.max_squad) |k| {
                const p = self.payment(h, k);
                pay += p[0];
                gem_spent += p[1];
            }
            const here = &self.s.provinces[@intCast(h.province)];
            self.restoreUnits(h, here.player == pl, self.heal(here));
            if (h.dead < 0) {
                pay -= @divTrunc(pay * h.heroBonus(pr, ab.hero_pay_perc), 100);
                gold_inc += h.heroBonus(pr, ab.hero_gold);
                gem_inc += h.heroBonus(pr, ab.hero_gem);
            }
            gold_spent += pay;
        }
        gold_inc += @divTrunc(gold_inc * perc, 100);
        const d = &pr.difficulties[@intCast(std.math.clamp(r.difficulty, 0, proto_mod.max_difficulty - 1))];
        var corrupt = @divTrunc(r.corrupt + prov_num, 10) - d.corrupt_start;
        if (r.kind == .human and corrupt > 0) {
            corrupt = @min(corrupt, d.corrupt_cap);
            corrupt = @divTrunc(corrupt * (100 + self.param1(pl, ab.castle_corrupt)), 100);
            r.calc_corrupt = corrupt;
            gold_spent += @divTrunc(gold_inc * corrupt, 100);
            gem_spent += @divTrunc(gem_inc * corrupt, 100);
        }
        r.gold += gold_inc - gold_spent;
        r.gold_inc = gold_inc - gold_spent;
        r.gem += gem_inc - gem_spent;
        r.gem_inc = gem_inc - gem_spent;
        if (r.kind != .human) {
            const g = 5 + r.difficulty * 8;
            const e = @divTrunc(current_turn, 20) + r.difficulty * 2;
            r.gold += g;
            r.gem += e;
            r.gold_inc += g;
            r.gem_inc += e;
        }
    }
};

const tst = std.testing;
const GameFs = @import("../platform/fs.zig").GameFs;
const packfile = @import("../formats/packfile.zig");
const loadgame = @import("loadgame.zig");
const movement = @import("movement.zig");

fn readTestFile(gpa: std.mem.Allocator, path: []const u8) ![]u8 {
    const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, gpa, .unlimited) catch return error.SkipZigTest;
    defer gpa.free(bytes);
    return packfile.unpack(gpa, bytes);
}

// testdata/hero_route: game0_turn2_auto.sav is the autosave at the end of
// turn 2 (the original's game0.bks), game0_turn3.sav the same game at the
// start of turn 3.  The human player 1 gets +28 gold and +14 gems, the
// capital grows 1177 -> 1200, the taken province 35 0 -> 1; the hero's
// squad recovers morale.
// testdata/camp_heal: the campaign shard at the end of turn 4 and the
// start of turn 5.  The unit in the capital's garrison heals 8 -> 11.
// The hero fought on a site that turn, so only the garrison is compared.
test "the garrison heals at the turn change" {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const mdata = try readTestFile(gpa, "testdata/camp_heal/map0.cmp");
    defer gpa.free(mdata);
    const adata = try readTestFile(gpa, "testdata/camp_heal/game0.bkc");
    defer gpa.free(adata);
    const bdata = try readTestFile(gpa, "testdata/camp_heal/game0.cmp");
    defer gpa.free(bdata);
    var a = try loadgame.fromBytes(gpa, &pr, mdata, adata);
    defer a.deinit();
    var b = try loadgame.fromBytes(gpa, &pr, mdata, bdata);
    defer b.deinit();
    const ga = a.shard.provinces[14].garrison;
    const gb = b.shard.provinces[14].garrison;
    try tst.expect(ga >= 0 and gb >= 0);
    try tst.expectEqual(@as(i32, 8), a.shard.garrisons.items[@intCast(ga)].squad[2].?.life);

    var eco: Economy = .{ .s = &a.shard, .pr = &pr };
    eco.beginTurn(1, 5);
    const got = &a.shard.garrisons.items[@intCast(ga)];
    const want = &b.shard.garrisons.items[@intCast(gb)];
    try tst.expectEqual(@as(i32, 11), got.squad[2].?.life);
    for (got.squad, want.squad) |x, y| {
        try tst.expectEqual(y == null, x == null);
        const u = x orelse continue;
        try tst.expectEqual(y.?.life, u.life);
        try tst.expectEqual(y.?.morale, u.morale);
    }
}

test "income and growth from turn 2 to turn 3" {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const mdata = try readTestFile(gpa, "testdata/hero_route/map0.map");
    defer gpa.free(mdata);
    const adata = try readTestFile(gpa, "testdata/hero_route/game0_turn2_auto.sav");
    defer gpa.free(adata);
    const bdata = try readTestFile(gpa, "testdata/hero_route/game0_turn3.sav");
    defer gpa.free(bdata);
    var a = try loadgame.fromBytes(gpa, &pr, mdata, adata);
    defer a.deinit();
    var b = try loadgame.fromBytes(gpa, &pr, mdata, bdata);
    defer b.deinit();

    var w = try movement.Walker.init(gpa, &a.shard, &pr, 20);
    defer w.deinit(gpa);
    _ = try movement.endTurnMove(gpa, &w, &a.shard, 0);
    var eco: Economy = .{ .s = &a.shard, .pr = &pr };
    eco.endTurn();
    eco.beginTurn(1, 3);

    const got = a.shard.rulers[1];
    const want = b.shard.rulers[1];
    // The values themselves, so the test cannot pass on unchanged data.
    try tst.expectEqual(@as(i32, 884), b.shard.rulers[1].gold - 28);
    try tst.expectEqual(@as(i32, 912), got.gold);
    try tst.expectEqual(@as(i32, 136), got.gem);
    try tst.expectEqual(@as(i32, 1200), a.shard.provinces[25].population);
    try tst.expectEqual(@as(i32, 1), a.shard.provinces[35].population);
    try tst.expectEqual(want.gold_inc, got.gold_inc);
    try tst.expectEqual(want.gem_inc, got.gem_inc);
    try tst.expectEqual(want.gold, got.gold);
    try tst.expectEqual(want.gem, got.gem);
    for (a.shard.provinces[1..], b.shard.provinces[1..]) |*x, *y| {
        if (y.player != 1) continue;
        try tst.expectEqual(y.population, x.population);
        try tst.expectEqual(y.warning, x.warning);
    }
    const hx = &a.shard.heroes.items[0];
    const hy = &b.shard.heroes.items[0];
    for (hx.squad, hy.squad) |ux, uy| {
        try tst.expectEqual(uy == null, ux == null);
        const p = ux orelse continue;
        try tst.expectEqual(uy.?.life, p.life);
        try tst.expectEqual(uy.?.morale, p.morale);
    }
}
