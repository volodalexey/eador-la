//! The castle part of dcPlayer (Unit.h:589): arRecruit, arLibrary,
//! arTreasury, arGroup, Shop, arRes, bEnter, iUnion and iPlanned, and the
//! prices of
//! Player.cpp that use them: GetResGoldPrice (:287), GetItemGoldPrice
//! (:338), GetItemGoldRepPrice (:380), GetUnitGoldPrice (:406), SellItem
//! (:1032).  dcPlayer::GetResource without trade is economy.playerResource.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const mapgen = @import("mapgen.zig");
const Shard = mapgen.Shard;
const site = @import("site.zig");
const economy = @import("economy.zig");
const movement = @import("movement.zig");

/// MAX_RECRUIT, LIBRARY_SIZE, TREASURY_SIZE, SHOP_SIZE.
pub const max_recruit = 12;
pub const library_size = 33;
pub const treasury_size = 36;
pub const shop_size = site.shop_size;
pub const max_group = proto_mod.max_group;
pub const max_group_build = proto_mod.max_group_build;
/// arRecruit[11]: the mercenary (AddMercenary), hired at double price.
pub const merc_slot = 11;
/// The player resources arRes / ResPrices are indexed by (1..9).
pub const max_res = 10;

/// Castle building abilities (GetBldParam1).
pub const abil = struct {
    pub const merc_level = 6;
    pub const res_discount = 17;
    pub const unit_discount = 18;
    pub const recruit_exp = 20;
    pub const dismiss_refund = 24;
};

/// Offsets in the player record.
const off = struct {
    const union_race = 0x30;
    const merc_time = 0x34;
    const can_inner_bld = 0x3c;
    const can_outer_bld = 0x40;
    const can_hire = 0x44;
    const can_cast = 0x48;
    const tower = 0x5c;
    const bld_filter = 0x10c;
    const def_filter = 0x10d;
    const rit_filter = 0x10e;
    const planned = 0x4c;
    const explored = 0x78;
    const pillage = 0x108;
    const wall = 0x58;
    const win = 0x68;
    const lose = 0x6c;
    const quest = 0x70;
    const time_shift = 0x7c;
    const res = 0x80;
    const kills = 0xa8;
    const loot = 0xec;
    const politic = 0xcec;
    const politic_size = 0x38c;
    const enter = 0x110;
    const recruit = 0x4fc;
    const library = 0x52c;
    const treasury = 0x5b0;
    const group = 0x6d0;
    const shop = 0x920;
};

pub const Holdings = struct {
    /// Unit types for hire: 0..5 the bottom row, 6..11 the top one.
    recruit: [max_recruit]i32 = @splat(0),
    /// Spells of the library (spell.var indices).
    library: [library_size]i32 = @splat(0),
    /// (item, durability).
    treasury: [treasury_size][2]i32 = @splat(.{ 0, 0 }),
    /// arGroup: the building shown in each place of a castle group.
    group: [max_group][max_group_build]i32 = @splat(@splat(0)),
    /// The castle shop (Type, PriceModifier, Items).
    shop: site.Shop = .{ .kind = 0, .prices = @splat(100) },
    /// arRes: the price cut of each resource in %.
    res_mod: [max_res]i32 = @splat(0),
    /// bEnter: the castle screen was not opened since the last building.
    enter: bool = true,
    /// iCanInnerBld: castle buildings left this turn.
    can_inner_bld: i32 = 1,
    /// iMercTime: turns since the last mercenary.
    merc_time: i32 = 0,
    /// iUnion: the race of the alliance (inner building ability 47).
    union_race: i32 = 0,
    /// iPlanned: the planned castle building (0: none).
    planned: i32 = 0,
    /// iCanOuterBld and iCanHire: outer buildings and defenders left this
    /// turn.
    can_outer_bld: i32 = 1,
    can_hire: i32 = 1,
    /// iCanCast: rituals left this turn.
    can_cast: i32 = 1,
    /// iTower: the towers of the capital.
    tower: i32 = 0,
    /// bBldFilter, bDefFilter, bRitFilter: the lists of the province
    /// panel show only what can be built, hired or cast there.
    bld_filter: bool = false,
    def_filter: bool = false,
    rit_filter: bool = false,
    /// iExplore and iPillage: land explored and gold pillaged (statistics).
    explored: i32 = 0,
    pillage: i32 = 0,

    pub fn read(rec: []const u8) Holdings {
        var h: Holdings = .{ .shop = .{ .kind = int(rec, off.shop), .prices = undefined } };
        for (&h.recruit, 0..) |*v, i| v.* = int(rec, off.recruit + 4 * i);
        for (&h.library, 0..) |*v, i| v.* = int(rec, off.library + 4 * i);
        for (&h.treasury, 0..) |*v, i| v.* = .{ int(rec, off.treasury + 8 * i), int(rec, off.treasury + 8 * i + 4) };
        for (&h.group, 0..) |*g, i| for (g, 0..) |*v, j| {
            v.* = int(rec, off.group + 4 * (i * max_group_build + j));
        };
        for (&h.shop.prices, 0..) |*v, i| v.* = int(rec, off.shop + 4 + 4 * i);
        for (&h.shop.items, 0..) |*v, i| v.* = .{ int(rec, off.shop + 0x48 + 8 * i), int(rec, off.shop + 0x4c + 8 * i) };
        for (&h.res_mod, 0..) |*v, i| v.* = int(rec, off.res + 4 * i);
        h.enter = rec[off.enter] != 0;
        h.can_inner_bld = int(rec, off.can_inner_bld);
        h.merc_time = int(rec, off.merc_time);
        h.union_race = int(rec, off.union_race);
        h.planned = int(rec, off.planned);
        h.can_cast = int(rec, off.can_cast);
        h.can_outer_bld = int(rec, off.can_outer_bld);
        h.can_hire = int(rec, off.can_hire);
        h.tower = int(rec, off.tower);
        h.bld_filter = rec[off.bld_filter] != 0;
        h.def_filter = rec[off.def_filter] != 0;
        h.rit_filter = rec[off.rit_filter] != 0;
        h.explored = int(rec, off.explored);
        h.pillage = int(rec, off.pillage);
        return h;
    }

    pub fn write(self: *const Holdings, rec: []u8) void {
        for (self.recruit, 0..) |v, i| setI(rec, off.recruit + 4 * i, v);
        for (self.library, 0..) |v, i| setI(rec, off.library + 4 * i, v);
        for (self.treasury, 0..) |v, i| {
            setI(rec, off.treasury + 8 * i, v[0]);
            setI(rec, off.treasury + 8 * i + 4, v[1]);
        }
        for (self.group, 0..) |g, i| for (g, 0..) |v, j| setI(rec, off.group + 4 * (i * max_group_build + j), v);
        setI(rec, off.shop, self.shop.kind);
        for (self.shop.prices, 0..) |v, i| setI(rec, off.shop + 4 + 4 * i, v);
        for (self.shop.items, 0..) |v, i| {
            setI(rec, off.shop + 0x48 + 8 * i, v[0]);
            setI(rec, off.shop + 0x4c + 8 * i, v[1]);
        }
        for (self.res_mod, 0..) |v, i| setI(rec, off.res + 4 * i, v);
        rec[off.enter] = @intFromBool(self.enter);
        setI(rec, off.can_inner_bld, self.can_inner_bld);
        setI(rec, off.merc_time, self.merc_time);
        setI(rec, off.union_race, self.union_race);
        setI(rec, off.planned, self.planned);
        setI(rec, off.can_cast, self.can_cast);
        setI(rec, off.can_outer_bld, self.can_outer_bld);
        setI(rec, off.can_hire, self.can_hire);
        setI(rec, off.tower, self.tower);
        rec[off.bld_filter] = @intFromBool(self.bld_filter);
        rec[off.def_filter] = @intFromBool(self.def_filter);
        rec[off.rit_filter] = @intFromBool(self.rit_filter);
        setI(rec, off.explored, self.explored);
        setI(rec, off.pillage, self.pillage);
    }

    /// The end of ScreenTreasury (Castle.cpp:1781): the shop's items move
    /// up over the empty places.
    pub fn compactShop(self: *Holdings) void {
        compact(&self.shop);
    }
};

/// The end of ScreenTreasury and ScreenHeroShop: the shop's items move up
/// over the empty places.
pub fn compact(shop: *site.Shop) void {
    const items = &shop.items;
    var i: usize = 0;
    while (true) {
        while (i < shop_size - 1 and items[i][0] > 0) i += 1;
        if (i >= shop_size - 1) return;
        const more = for (items[i + 1 ..]) |it| {
            if (it[0] > 0) break true;
        } else false;
        if (!more) return;
        std.mem.copyForwards([2]i32, items[i .. shop_size - 1], items[i + 1 ..]);
        items[shop_size - 1] = .{ 0, 0 };
    }
}

/// MAX_PLAYERS: Politic[0..16].
/// arPlayer[k].pStory and pPlace, index 1..iMaxEvent (0 unused): -1 an
/// event that has not happened, 0 one that has, > 0 the turns left before
/// it happens; pPlace is its province (0 any of the player's).
pub const Story = struct {
    story: []i32 = &.{},
    place: []i32 = &.{},

    /// The saved list: a count n, then n pairs (story, place).
    pub fn read(a: std.mem.Allocator, rec: []const u8) !Story {
        if (rec.len < 4) return .{};
        const n: usize = @intCast(@max(int(rec, 0), 0));
        if (rec.len < 4 + 8 * n) return error.BadEvents;
        const s: Story = .{ .story = try a.alloc(i32, n + 1), .place = try a.alloc(i32, n + 1) };
        s.story[0] = -1;
        s.place[0] = 0;
        for (1..n + 1) |i| {
            s.story[i] = int(rec, 4 + 8 * (i - 1));
            s.place[i] = int(rec, 8 + 8 * (i - 1));
        }
        return s;
    }

    /// A new game's list: nothing has happened (GameMenu.cpp:1060).
    pub fn init(a: std.mem.Allocator, n_events: usize) !Story {
        const s: Story = .{ .story = try a.alloc(i32, n_events), .place = try a.alloc(i32, n_events) };
        @memset(s.story, -1);
        @memset(s.place, 0);
        return s;
    }

    pub fn write(self: *const Story, gpa: std.mem.Allocator, out: *std.ArrayList(u8)) !void {
        const n = self.story.len -| 1;
        var b: [4]u8 = undefined;
        std.mem.writeInt(i32, &b, @intCast(n), .little);
        try out.appendSlice(gpa, &b);
        for (1..n + 1) |i| {
            std.mem.writeInt(i32, &b, self.story[i], .little);
            try out.appendSlice(gpa, &b);
            std.mem.writeInt(i32, &b, self.place[i], .little);
            try out.appendSlice(gpa, &b);
        }
    }
};

pub const max_politic = 17;

/// dcTrade (Unit.h:533) at +0x18 of dcPolitics: what the player pays
/// every turn under a trade deal.
pub const Trade = struct { gold: i32 = 0, gem: i32 = 0, on: bool = false };

/// The counters and relations of dcPlayer that ScreenStatistics shows:
/// iWin, iLose, iQuest, iTimeShift, arKills, arLoot and Politic[k].Treaty
/// and Trade.  No ported code changes them, so they are only read; SaveGame
/// keeps their bytes from the loaded file.
pub const Chronicle = struct {
    win: i32 = 0,
    lose: i32 = 0,
    quest: i32 = 0,
    time_shift: i32 = 0,
    /// arKills[1..16]: rulers defeated (index = player).
    kills: [17]i32 = @splat(0),
    /// arLoot: rare items found (rarity 4, 5, 6).
    loot: [3]i32 = @splat(0),
    /// Politic[k].Treaty: 0 no contact, 1 war, 2 peace, 3 alliance.
    treaty: [max_politic]i32 = @splat(0),
    trade: [max_politic]Trade = @splat(.{}),

    pub fn read(rec: []const u8) Chronicle {
        var ch: Chronicle = .{
            .win = int(rec, off.win),
            .lose = int(rec, off.lose),
            .quest = int(rec, off.quest),
            .time_shift = int(rec, off.time_shift),
        };
        for (&ch.kills, 0..) |*v, i| v.* = int(rec, off.kills + 4 * i);
        for (&ch.loot, 0..) |*v, i| v.* = int(rec, off.loot + 4 * i);
        for (&ch.treaty, &ch.trade, 0..) |*t, *tr, i| {
            const p = off.politic + off.politic_size * i;
            t.* = int(rec, p);
            tr.* = .{ .gold = int(rec, p + 0x18), .gem = int(rec, p + 0x1c), .on = rec[p + 0x60] != 0 };
        }
        return ch;
    }
};

/// iWall: the strength of the capital's walls (castle ability 35).
pub fn readWall(rec: []const u8) i32 {
    return int(rec, off.wall);
}

pub fn writeWall(rec: []u8, v: i32) void {
    setI(rec, off.wall, v);
}

/// (gold, gems).
pub const Price = [2]i32;

/// The prices of player `player`.
pub const Market = struct {
    s: *Shard,
    pr: *const Proto,
    player: i32,

    pub fn ruler(self: Market) *mapgen.Ruler {
        return &self.s.rulers[@intCast(self.player)];
    }

    pub fn holdings(self: Market) *Holdings {
        return &self.ruler().holdings;
    }

    fn eco(self: Market) economy.Economy {
        return .{ .s = self.s, .pr = self.pr };
    }

    /// GetResource.
    pub fn resource(self: Market, r: i32) i32 {
        return self.eco().playerResource(self.player, r);
    }

    /// GetBldParam1.
    pub fn bldParam1(self: Market, id: i32) i32 {
        return self.eco().param1(self.player, id);
    }

    /// Gold >= price.gold and (Gem >= price.gem or the price has no gems).
    pub fn affords(self: Market, p: Price) bool {
        const r = self.ruler();
        return r.gold >= p[0] and (r.gem >= p[1] or p[1] == 0);
    }

    pub fn pay(self: Market, p: Price) void {
        const r = self.ruler();
        r.gold -= p[0];
        r.gem -= p[1];
    }

    /// GetResGoldPrice / GetResGemPrice: the surcharge of a resource the
    /// player does not have.
    pub fn resPrice(self: Market, r: i32) Price {
        if (r < 1 or r >= max_res or r >= self.pr.resources.len or self.resource(r) != 0) return .{ 0, 0 };
        return self.resPriceStat(r);
    }

    /// GetResGoldPriceStat / GetResGemPriceStat: the same price whether
    /// the player has the resource or not.
    pub fn resPriceStat(self: Market, r: i32) Price {
        if (r < 1 or r >= max_res or r >= self.pr.resources.len) return .{ 0, 0 };
        const i: usize = @intCast(r);
        const res = &self.pr.resources[i];
        const cut = self.bldParam1(abil.res_discount);
        var out: Price = undefined;
        for (&out, [2]i32{ res.gold, res.gem }) |*o, base| {
            var v = @divTrunc(base * self.s.res_prices[i], 100);
            v -= @divTrunc(v * self.holdings().res_mod[i], 100);
            v -= @divTrunc(v * cut, 100);
            o.* = v;
        }
        return out;
    }

    /// `base` plus the surcharge of every resource the player lacks.
    pub fn withRes(self: Market, base: Price, res: []const i32) Price {
        var out = base;
        for (res) |r| if (r > 0) {
            const add = self.resPrice(r);
            out[0] += add[0];
            out[1] += add[1];
        };
        return out;
    }

    /// GetItemGoldPrice / GetItemGemPrice.
    pub fn itemPrice(self: Market, item: i32, shop: *const site.Shop) Price {
        const it = &self.pr.items[@intCast(item)];
        var out = self.withRes(.{ it.gold_cost, it.gem_cost }, &it.res);
        const m = shop.prices[@intCast(self.player)];
        for (&out) |*v| v.* = @divTrunc(v.* * m, 100);
        return out;
    }

    /// GetItemGoldRepPrice / GetItemGemRepPrice.
    pub fn repairPrice(self: Market, item: i32, dur: i32, shop: *const site.Shop) Price {
        const max = self.pr.items[@intCast(item)].durability;
        if (dur >= max) return .{ 0, 0 };
        const p = self.itemPrice(item, shop);
        const gold = @divTrunc(@divTrunc(p[0], 2) * (max - dur), max);
        return .{ @max(gold, 1), @divTrunc(@divTrunc(p[1], 2) * (max - dur), max) };
    }

    /// GetUnitGoldPrice / GetUnitGemPrice; `distance` is unitDistance of
    /// the province the unit is hired in.
    pub fn unitPrice(self: Market, unit: i32, distance: i32) Price {
        const u = &self.pr.units[@intCast(unit)];
        var base: Price = .{ u.gold_price, u.gem_price };
        const cut = self.bldParam1(abil.unit_discount);
        for (&base) |*v| v.* -= @divTrunc(v.* * cut, 100);
        var out = self.withRes(base, &u.res);
        var d = distance;
        if (d < 0 or d > 11) d = 11;
        d -= 1;
        for (&out) |*v| v.* += @divTrunc(v.* * d, 10);
        return out;
    }

    /// The MoveMap of GetUnitGoldPrice: turns from `prov` to the capital
    /// (MovePlayer = Num, WalkType 1, WalkSpeed 1).
    pub fn unitDistance(self: Market, w: *movement.Walker, prov: i32) i32 {
        w.clear();
        w.player = self.player;
        w.walk_type = 1;
        w.walk_speed = 1;
        w.moveMap(prov, -1, 1, 0, 0);
        const home = self.s.capitals[@intCast(self.player)];
        if (home < 1 or home >= w.move.len) return -1;
        return w.move[@intCast(home)];
    }

    /// CurGame.ResPrices[r] += ResBuy for every resource of a purchase the
    /// player does not have.
    pub fn raiseResPrices(self: Market, res: []const i32) void {
        for (res) |r| {
            if (r > 0 and r < max_res and self.resource(r) == 0) self.s.res_prices[@intCast(r)] += self.s.res_buy;
        }
    }
};

/// SellItem: a quarter of the cost less an eighth of the worn share.
pub fn sellPrice(pr: *const Proto, item: i32, dur: i32) Price {
    const it = &pr.items[@intCast(item)];
    var out: Price = undefined;
    for (&out, [2]i32{ it.gold_cost, it.gem_cost }) |*o, cost| {
        o.* = @divTrunc(cost, 4);
        if (it.durability > 0) o.* -= @divTrunc(@divTrunc(cost * (it.durability - dur), it.durability), 8);
    }
    return out;
}

fn int(b: []const u8, o: usize) i32 {
    return std.mem.readInt(i32, b[o..][0..4], .little);
}

fn setI(b: []u8, o: usize, v: i32) void {
    std.mem.writeInt(i32, b[o..][0..4], v, .little);
}

test "holdings round trip and shop compaction" {
    var rec: [0x4a34]u8 = @splat(0);
    var h: Holdings = .{};
    h.recruit[0] = 3;
    h.recruit[merc_slot] = 40;
    h.library[32] = 7;
    h.treasury[35] = .{ 12, 5 };
    h.group[6][3] = 99;
    h.shop.items[0] = .{ 1, 0 };
    h.shop.items[2] = .{ 5, 2 };
    h.shop.items[107] = .{ 9, 1 };
    h.res_mod[9] = 20;
    h.enter = false;
    h.merc_time = 4;
    h.union_race = 3;
    h.planned = 17;
    h.write(&rec);
    try std.testing.expectEqual(@as(i32, 17), int(&rec, 0x4c));
    try std.testing.expectEqual(@as(i32, 100), int(&rec, 0x924 + 4 * 16));
    const back = Holdings.read(&rec);
    try std.testing.expectEqualDeep(h, back);
    h.compactShop();
    try std.testing.expectEqual([2]i32{ 1, 0 }, h.shop.items[0]);
    try std.testing.expectEqual([2]i32{ 5, 2 }, h.shop.items[1]);
    try std.testing.expectEqual([2]i32{ 9, 1 }, h.shop.items[2]);
    try std.testing.expectEqual([2]i32{ 0, 0 }, h.shop.items[107]);
}
