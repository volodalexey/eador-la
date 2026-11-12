//! Sites of a province: dcProvince::PlaceSite, GetFreeSpace and
//! ShopRoller (Province.cpp:7, :3142, :3204; GOG FUN_00459850,
//! FUN_00455350, FUN_00455d70).  The rand() calls follow GOG.
const std = @import("std");
const Rand = @import("../data/rand.zig").Rand;
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const province = @import("province.zig");
const Province = province.Province;
const guard_mod = @import("guard.zig");
const Guard = guard_mod.Guard;
const Roller = @import("unit.zig").Roller;

/// SHOP_SIZE, MAX_PLAYERS.
pub const shop_size = 108;

/// dcShop.
pub const Shop = struct {
    kind: i32,
    prices: [province.max_players]i32,
    /// (item, count; 0 is unlimited).
    items: [shop_size][2]i32 = @splat(.{ 0, 0 }),
};

/// Site ability ids used here.
const ab_guard_level = 8;
const ab_shop = 13;
const ab_shop_size = 14;
const ab_quest = 15;
/// Quest kinds: 7 random item, 8 kill monsters.
const quest_item = 7;
const quest_kill = 8;
/// arSite[3] of a site that gives a kill quest.
pub const quest_kill_marker = 9999;

/// The world PlaceSite writes to.
pub const World = struct {
    gpa: std.mem.Allocator,
    pr: *const Proto,
    r: *Rand,
    guards: *std.ArrayList(Guard),
    shops: *std.ArrayList(Shop),
    /// CurGame.Map.Neutrals.
    neutrals: i32,

    fn below(self: *World, n: i32) i32 {
        if (n == 0) return 0;
        return @bitCast(self.r.random(@bitCast(n)));
    }

    /// rand() % m compared unsigned with `v`, as GOG does.
    fn chance(self: *World, m: u32, v: i32) bool {
        return self.r.rand() % m < @as(u32, @bitCast(v));
    }

    fn taken(pv: *const Province, pos: i32) bool {
        for (pv.sites) |s| if (s.index > 0 and s.pos == pos) return true;
        return false;
    }

    /// GetFreeSpace: a free position on the exploration scale, above the
    /// explored part (hidden > 0), inside it (< 0) or anywhere; 0 if none.
    pub fn freeSpace(self: *World, pv: *const Province, hidden: i32) i32 {
        if (hidden != 0) {
            const lo: i32 = if (hidden > 0) pv.explore + 1 else 1;
            const hi: i32 = if (hidden > 0) 99 else pv.explore;
            var any = false;
            var p = lo;
            while (p <= hi) : (p += 1) any = any or !taken(pv, p);
            if (!any) return 0;
        }
        while (true) {
            var v: i32 = @intCast(self.r.rand() % 99 + 1);
            if (v < 50) v = @intCast(self.r.rand() % 99 + 1);
            const ok = hidden == 0 or (hidden > 0 and v > pv.explore) or (hidden < 0 and v <= pv.explore);
            if (ok and !taken(pv, v)) return v;
        }
    }

    /// ShopRoller: items a shop of `kind` sells up to `level`.
    pub fn shopRoller(self: *const World, kind: i32, level: i32, rare: i32, equal: bool) Roller {
        var rl: Roller = .{};
        var n: usize = 0;
        for (self.pr.items[1..], 1..) |it, k| {
            const add = switch (kind) {
                3, 4, 5, 7, 8, 10, 11, 12, 13 => it.class == kind,
                6 => it.class == kind or it.slot == 5,
                1, 2, 9 => it.class == 1 or it.class == 2 or it.class == 9,
                -1 => it.class == 0 and switch (it.slot) {
                    3, 4, 6, 7, 8 => true,
                    10 => it.durability > 1,
                    else => false,
                },
                -2 => it.class == 0 and (it.slot == 9 or it.slot == 11 or (it.slot == 10 and it.durability == 1)),
                else => false,
            };
            if (add and it.shop_level <= level and it.rarity >= rare and (it.shop_level == level or !equal)) {
                rl.ids[n] = @intCast(k);
                rl.weights[n] = 8 - it.rarity;
                n += 1;
            }
        }
        return rl;
    }

    /// PlaceSiteGuard.
    pub fn placeSiteGuard(self: *World, pv: *Province, slot: usize, guardian: i32) !void {
        try self.guards.append(self.gpa, guard_mod.place(self.pr, self.r, guardian, 0));
        pv.site_guards[slot] = @intCast(self.guards.items.len - 1);
    }

    /// PlaceSite: puts site `index` of `level` into the first free slot;
    /// `hidden` 0 takes the site's own placement.  `guard` > 0 forces a
    /// guard, < 0 forbids it.  Returns the slot or null.
    pub fn place(self: *World, pv: *Province, index: i32, level_in: i32, quest_in: i32, hidden_in: i32, guard: i32) !?usize {
        const level = @max(level_in, 0);
        const slot = for (pv.sites, 0..) |s, k| {
            if (s.index <= 0) break k;
        } else return null;
        const site = &self.pr.sites[@intCast(index)];
        const hidden = if (hidden_in == 0) site.place - 1 else hidden_in;
        const pos = self.freeSpace(pv, hidden);
        if (pos == 0) return null;
        const st = &pv.sites[slot];
        st.* = .{ .index = index, .pos = pos, .quest = quest_in };
        st.level = @as(i32, @intCast(self.r.rand() & 1)) + level * 2;
        if (self.r.rand() % 40 + 60 < @as(u32, @bitCast(pos))) st.level += self.below(@divTrunc(pos - 60, 11));
        st.level = @min(st.level + site.treasure, 10);

        if ((guard > 0 and site.guarded > 0) or (self.chance(100, site.guarded) and guard >= 0)) {
            var rl: Roller = .{};
            var n: usize = 0;
            for (site.guards[0..15]) |g| if (g.id > 0) {
                rl.ids[n] = g.id;
                rl.weights[n] = g.value;
                n += 1;
            };
            const gt = rl.roll(self.r);
            st.guard = gt;
            const types = self.pr.guard_types[@intCast(gt)].guards;
            var lvl: i32 = -1;
            for (site.abilities) |a| if (a.id == ab_guard_level) {
                lvl = a.p1;
            };
            if (lvl < 0) {
                // GOG subtracts the first Random(Level+1) and adds the second.
                const r1 = self.below(level + 1);
                const r2 = self.below(level + 1);
                var power = r2 + 1 + level * 3 - r1;
                if (self.r.rand() % 30 + 60 < @as(u32, @bitCast(pos))) power += self.below(level + 4);
                power = @min(power, self.neutrals);
                lvl = 0;
                for (1..4) |k| {
                    const cur = self.pr.guards[@intCast(types[@intCast(lvl)])].power;
                    if (@abs(self.pr.guards[@intCast(types[k])].power - power) < @abs(cur - power)) lvl = @intCast(k);
                }
            } else lvl = @min(lvl, 3);
            st.level = site.treasure + self.pr.guards[@intCast(types[@intCast(lvl)])].power;
            try self.placeSiteGuard(pv, slot, types[@intCast(lvl)]);
        }

        if (quest_in == 0) {
            var quest: i32 = 0;
            for (site.abilities) |a| {
                if (a.id == ab_quest and self.chance(100, a.p2)) quest = a.p1;
            }
            if (quest > 0) {
                st.quest = quest;
                const q = &self.pr.quests[@intCast(quest)];
                if (q.kind == quest_item and q.tasks[0].index == 1) {
                    const spells: i32 = @intCast(self.pr.spells.len - 1);
                    st.data = self.below(spells - 3) + 3 + @as(i32, @intCast(self.pr.max_pure_item));
                }
            } else for (self.pr.quests[1..]) |q| {
                if (q.giver == index and q.kind == quest_kill) st.quest = quest_kill_marker;
            }
        }

        var shop: ?i32 = null;
        var shop_level: i32 = 0;
        var items: i32 = 8;
        var prices: i32 = 300;
        for (site.abilities) |a| {
            if (a.id == ab_shop) {
                shop = a.p1;
                shop_level = a.p2;
                if (self.r.rand() % 10 == 0) {
                    shop_level += 1;
                    if (self.r.rand() % 10 == 0) shop_level += 1;
                }
            }
            if (a.id == ab_shop_size) {
                items = a.p1;
                prices = a.p2;
            }
        }
        if (shop) |kind| {
            st.level = @intCast(self.shops.items.len);
            var sh: Shop = .{ .kind = kind, .prices = @splat(prices) };
            var rl = self.shopRoller(kind, shop_level, 0, false);
            for (sh.items[0..@intCast(@max(0, @min(items, shop_size)))]) |*it| {
                const item = rl.rollExclude(self.r);
                const rarity = self.pr.items[@intCast(item)].rarity;
                it.* = .{ item, if (rarity == 1) 0 else 1 + self.below(6 - rarity) };
            }
            try self.shops.append(self.gpa, sh);
        }
        return slot;
    }
};
