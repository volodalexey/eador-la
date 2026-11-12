//! A ruler's heroes: hiring (dcPlayer::RecruitHero, dcHero(Player, Class),
//! Hero.cpp:70), the prices (GetHeroGoldPrice, GetResurrectGoldPrice,
//! Player.cpp:632..700), the death of a hero (dcHero::Death, Hero.cpp:1578),
//! the resurrection (ResurrectWindow, MainProcess.cpp:8860) and the burial
//! (~dcHero, Hero.cpp:262).
//!
//! The heroes live in Shard.heroes by player and slot.  Hiring inserts one
//! and burying removes one, so indices into the list change: holders of an
//! index (the map's selected hero) must drop it when the length changes.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const Rand = @import("../data/rand.zig").Rand;
const mapgen = @import("mapgen.zig");
const Shard = mapgen.Shard;
const hero_mod = @import("hero.zig");
const Hero = hero_mod.Hero;
const Unit = @import("unit.zig").Unit;
const player = @import("player.zig");
const Market = player.Market;
const Price = player.Price;
const movement = @import("movement.zig");

pub const max_hero = hero_mod.max_hero;
/// The most heroes of one base class (ScreenHeroRecruit, dialog 1089).
pub const max_class_heroes = 4;
/// MAX_BODY_DISTANCE: the most turns to bring a body to the castle.
pub const max_body_distance = 6;

/// Castle building abilities: the hero discount (crystals), the
/// resurrection discount.
const abil_hero_discount = 13;
const abil_resurrect_discount = 7;

/// Gold >= price and Gem >= price, as the castle screens check it.
pub fn canPay(m: Market, p: Price) bool {
    const r = m.ruler();
    return r.gold >= p[0] and r.gem >= p[1];
}

/// The heroes of `owner`, dead ones included.
pub fn count(s: *const Shard, owner: usize) usize {
    var n: usize = 0;
    for (s.heroes.items) |*h| n += @intFromBool(h.owner == @as(i32, @intCast(owner)));
    return n;
}

pub fn classCount(s: *const Shard, owner: usize, class: i32) usize {
    var n: usize = 0;
    for (s.heroes.items) |*h| n += @intFromBool(h.owner == @as(i32, @intCast(owner)) and h.class1 == class);
    return n;
}

/// GetHeroGoldPrice / GetHeroGemPrice of base class `class` (1..4):
/// HeroN by the number of heroes (at most 3), times Multiplier for each
/// hero of the class, less ability 13 in percent.  The products wrap like
/// the original's 32-bit int.
pub fn price(m: Market, class: i32) Price {
    const pr = m.pr;
    const n = @min(count(m.s, @intCast(m.player)), 3);
    const same = classCount(m.s, @intCast(m.player), class);
    const cut = m.bldParam1(abil_hero_discount);
    return scaled(.{ pr.hero_price[n].id, pr.hero_price[n].value }, same, pr.hero_price_mult, cut);
}

fn scaled(base: Price, same: usize, mult: i32, cut: i32) Price {
    var out = base;
    for (&out) |*v| {
        for (0..same) |_| v.* *%= mult;
        v.* -%= @divTrunc(v.* *% cut, 100);
    }
    return out;
}

/// GetResurrectGoldPrice / GetResurrectGemPrice: Resurrect + level ·
/// ResurrectAdd, less ability 7 in percent.
pub fn resurrectPrice(m: Market, h: *const Hero) Price {
    const pr = m.pr;
    const cut = m.bldParam1(abil_resurrect_discount);
    const base = pr.hero_resurrect[0];
    const add = pr.hero_resurrect[1];
    var out: Price = .{ base.id +% h.level *% add.id, base.value +% h.level *% add.value };
    for (&out) |*v| v.* -%= @divTrunc(v.* *% cut, 100);
    return out;
}

fn freeSlot(s: *const Shard, owner: usize) ?u8 {
    var used: [max_hero]bool = @splat(false);
    for (s.heroes.items) |*h| {
        if (h.owner == @as(i32, @intCast(owner)) and h.slot < max_hero) used[h.slot] = true;
    }
    const i = std.mem.indexOfScalar(bool, &used, false) orelse return null;
    return @intCast(i);
}

fn taken(s: *const Shard, owner: usize, class: i32, comptime field: []const u8, v: i32) bool {
    for (s.heroes.items) |*h| {
        if (h.owner == @as(i32, @intCast(owner)) and h.class1 == class and @field(h, field) == v) return true;
    }
    return false;
}

fn item(pr: *const Proto, inv: *hero_mod.Inventory, slot: usize, id: i32) void {
    inv[slot] = .{ id, pr.items[@intCast(id)].durability };
}

fn unitAt(pr: *const Proto, r: *Rand, t: i32, x: i32, y: i32) Unit {
    var u = Unit.init(pr, r, t, 0);
    u.x = x;
    u.y = y;
    return u;
}

/// dcHero(Player, Class): a new hero of base class `class` in the
/// owner's capital with the class's unit, two units of types 1 and 2 and
/// the starting skill, items, spells and troops.
pub fn make(s: *const Shard, pr: *const Proto, r: *Rand, owner: usize, slot: u8, class: i32) Hero {
    var rec: [hero_mod.record_size]u8 = @splat(0);
    std.mem.writeInt(i32, rec[0x4c..][0..4], -1, .little);
    for (0..4) |k| std.mem.writeInt(i32, rec[0x390 + 4 * k ..][0..4], -1, .little);
    var h = Hero.fromArmy(pr, &.{ .rec = &rec });
    const pl: i32 = @intCast(owner);
    h.owner = pl;
    h.slot = slot;
    h.player = pl;
    h.province = s.capitals[owner];
    h.class1 = class;
    const names: u32 = @intCast(pr.hero_names[@intCast(class - 1)].len);
    while (true) {
        h.name = @intCast(r.random(names));
        if (!taken(s, owner, class, "name", h.name)) break;
    }
    while (true) {
        h.portrait = (class - 1) * 4 + @as(i32, @intCast(r.random(4)));
        if (!taken(s, owner, class, "portrait", h.portrait)) break;
    }
    h.squad[0] = unitAt(pr, r, class + 39, 0, 3);
    h.squad[1] = unitAt(pr, r, 1, 1, 4);
    h.squad[2] = unitAt(pr, r, 2, 0, 4);
    const own = &h.squad[0].?;
    switch (class) {
        1 => {
            h.health = 1;
            own.x = 1;
            h.skills[0] = .{ 8 + @as(i32, @intCast(r.random(7))), 1 };
            item(pr, &h.inventory, 1, 9 + @as(i32, @intCast(r.random(3))) * 4);
            item(pr, &h.inventory, 2, 41 + @as(i32, @intCast(r.random(2))));
        },
        2 => {
            h.health = 1;
            own.x = 0;
            own.upgrades[0] = 62 + @as(i32, @intCast(r.random(3)));
            own.level += 1;
            h.skills[0] = .{ 22 + @as(i32, @intCast(r.random(7))), 1 };
            item(pr, &h.inventory, 1, 29);
            item(pr, &h.inventory, 2, 46);
        },
        3 => {
            h.leader = 1;
            own.x = 1;
            const sk = 1 + @as(i32, @intCast(r.random(7)));
            h.skills[0] = .{ sk, 1 };
            switch (sk) {
                1 => {
                    h.squad[3] = unitAt(pr, r, 3, 1, 2);
                    item(pr, &h.inventory, 3, 51);
                },
                2 => {
                    h.squad[3] = unitAt(pr, r, 1, 1, 2);
                    h.squad[4] = unitAt(pr, r, 1, 1, 5);
                },
                5 => h.squad[3] = unitAt(pr, r, 2, 0, 2),
                else => h.squad[3] = unitAt(pr, r, 3, 1, 2),
            }
        },
        4 => {
            h.magic = 1;
            own.x = 0;
            const sk = 15 + @as(i32, @intCast(r.random(7)));
            h.skills[0] = .{ sk, 1 };
            h.setSpell(1, 1);
            h.setSpell(2, 2 + @as(i32, @intCast(r.random(2))));
            h.setSpell(3, 4);
            h.setSpell(4, 4 + @as(i32, @intCast(r.random(10))));
            switch (sk) {
                15 => item(pr, &h.inventory, 1, 5),
                16 => h.setSpell(2, 11),
                17 => {
                    h.setSpell(3, 19);
                    item(pr, &h.inventory, 3, 63);
                },
                18 => h.setSpell(2, 13),
                19 => h.setSpell(5, @as(i32, @intCast(r.random(3))) + 1),
                20 => item(pr, &h.inventory, 1, 37),
                21 => {
                    h.setSpell(3, 14);
                    item(pr, &h.inventory, 1, 1);
                },
                else => {},
            }
        },
        else => {},
    }
    h.squad[0].?.life += h.heroBonus(pr, hero_mod.bonus.life);
    return h;
}

/// RecruitHero: the hero goes to the first free slot of the owner.
/// Returns its index in Shard.heroes.
pub fn recruit(s: *Shard, pr: *const Proto, r: *Rand, owner: usize, class: i32) !usize {
    const slot = freeSlot(s, owner) orelse return error.NoFreeHeroSlot;
    const h = make(s, pr, r, owner, slot, class);
    const pl: i32 = @intCast(owner);
    var at: usize = 0;
    while (at < s.heroes.items.len) : (at += 1) {
        const o = &s.heroes.items[at];
        if (o.owner > pl or (o.owner == pl and o.slot > slot)) break;
    }
    try s.heroes.insert(s.gpa, at, h);
    return at;
}

/// HeroRecruitWindow's "Нанять": the price is paid and the hero hired.
/// Returns the hero's slot.
pub fn hire(m: Market, r: *Rand, class: i32) !u8 {
    const p = price(m, class);
    if (!canPay(m, p)) return error.NotEnoughMoney;
    const k = try recruit(m.s, m.pr, r, @intCast(m.player), class);
    m.pay(p);
    return m.s.heroes.items[k].slot;
}

/// The hero's own unit after a resurrection: half its life, plus one.
fn revive(pr: *const Proto, h: *Hero) void {
    h.dead = -1;
    if (h.squad[0]) |*u| u.life = @divTrunc(h.life(pr)[1], 2) + 1;
}

/// The resurrection for its price: in the capital (ResurrectWindow and
/// the turn's dialog 1239) or where the hero fell (Death).
pub fn resurrect(m: Market, k: usize, home: bool) void {
    const h = &m.s.heroes.items[k];
    m.pay(resurrectPrice(m, h));
    revive(m.pr, h);
    if (home) h.province = m.s.capitals[@intCast(m.player)];
}

/// The start of Death: the order is dropped and the turns to carry the
/// body to the capital measured (MoveMap with WalkType 1, 2..6).
pub fn deathDistance(m: Market, w: *movement.Walker, k: usize) i32 {
    const h = &m.s.heroes.items[k];
    h.task = hero_mod.Task.none;
    const d = m.unitDistance(w, h.province);
    if (d < 0 or d > max_body_distance) return max_body_distance;
    return @max(d, 2);
}

/// The body goes to the capital: the hero waits there `distance` + 1
/// turns (the next turn change takes one).
pub fn deliver(s: *Shard, k: usize, distance: i32) void {
    const h = &s.heroes.items[k];
    h.dead = distance + 1;
    h.province = s.capitals[@intCast(h.owner)];
}

/// The computer's part of Death: after a won battle (1) or a draw (-2)
/// the hero is resurrected on the spot when the ruler can pay, else
/// carried home; after a defeat a hero with experience, or one of
/// several, is carried home and a lone new one buried.
pub fn computerDeath(m: Market, k: usize, win: i32, distance: i32) void {
    const h = &m.s.heroes.items[k];
    if (win == 1 or win == -2) {
        if (canPay(m, resurrectPrice(m, h))) resurrect(m, k, false) else deliver(m.s, k, distance);
        return;
    }
    if (h.exp > 0 or count(m.s, @intCast(m.player)) > 1) deliver(m.s, k, distance) else bury(m.s, m.pr, k);
}

/// ~dcHero: the quests are abandoned (a giver's site gets its quest
/// back), the hero and his squad leave, the later slots of his owner move
/// up.
pub fn bury(s: *Shard, pr: *const Proto, k: usize) void {
    const h = &s.heroes.items[k];
    for (0..hero_mod.quest_limit) |i| {
        const q = h.quest(i);
        if (q.index <= 0) break;
        if (q.index >= pr.quests.len or pr.quests[@intCast(q.index)].giver <= 0) continue;
        if (q.province <= 0 or q.province >= s.provinces.len or q.site < 0 or q.site >= s.provinces[0].sites.len) continue;
        const site = &s.provinces[@intCast(q.province)].sites[@intCast(q.site)];
        site.quest = -site.quest;
    }
    const owner = h.owner;
    const slot = h.slot;
    _ = s.heroes.orderedRemove(k);
    for (s.heroes.items) |*o| {
        if (o.owner == owner and o.slot > slot) o.slot -= 1;
    }
}

const tst = std.testing;

test "hero prices as on recomp's saves/turn253" {
    // The formula of recomp/src/orig/castle.zig on its numbers: five
    // heroes (classes 3, 2, 4, 1, 1), 45 % off: Hero4 = 5000, 500.
    try tst.expectEqual(Price{ 275000, 27500 }, scaled(.{ 5000, 500 }, 2, 10, 45));
    try tst.expectEqual(Price{ 27500, 2750 }, scaled(.{ 5000, 500 }, 1, 10, 45));
}

fn readTestFile(gpa: std.mem.Allocator, path: []const u8) ![]u8 {
    const packfile = @import("../formats/packfile.zig");
    const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, gpa, .unlimited) catch return error.SkipZigTest;
    defer gpa.free(bytes);
    return packfile.unpack(gpa, bytes);
}

test "hire, resurrect and bury a hero" {
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
    const m: Market = .{ .s = s, .pr = &pr, .player = 1 };
    var r: Rand = .{};
    const before = count(s, 1);
    m.ruler().gold = 100000;
    m.ruler().gem = 10000;

    for (1..5) |cl| {
        const class: i32 = @intCast(cl);
        const want = price(m, class);
        const gold = m.ruler().gold;
        const slot = try hire(m, &r, class);
        try tst.expectEqual(gold - want[0], m.ruler().gold);
        const k = for (s.heroes.items, 0..) |*h, i| {
            if (h.owner == 1 and h.slot == slot) break i;
        } else return error.TestUnexpectedResult;
        const h = &s.heroes.items[k];
        try tst.expectEqual(class, h.class1);
        try tst.expect(h.alive());
        try tst.expectEqual(s.capitals[1], h.province);
        try tst.expectEqual(class + 39, h.squad[0].?.type);
        try tst.expectEqual(h.life(&pr)[1], h.squad[0].?.life);
        try tst.expect(h.squad[1] != null and h.squad[2] != null);
        try tst.expect(h.skills[0][0] > 0);
    }
    try tst.expectEqual(before + 4, count(s, 1));
    // The list stays in player and slot order.
    for (s.heroes.items[1..], 1..) |*h, i| {
        const o = &s.heroes.items[i - 1];
        try tst.expect(o.owner < h.owner or (o.owner == h.owner and o.slot < h.slot));
    }

    // A fallen hero is carried home, then resurrected for the price.
    const k = for (s.heroes.items, 0..) |*h, i| {
        if (h.owner == 1) break i;
    } else return error.TestUnexpectedResult;
    s.heroes.items[k].squad[0].?.life = 0;
    deliver(s, k, 3);
    try tst.expectEqual(@as(i32, 4), s.heroes.items[k].dead);
    s.heroes.items[k].dead = 0;
    const cost = resurrectPrice(m, &s.heroes.items[k]);
    const gold = m.ruler().gold;
    resurrect(m, k, true);
    const h = &s.heroes.items[k];
    try tst.expect(h.alive());
    try tst.expectEqual(gold - cost[0], m.ruler().gold);
    try tst.expectEqual(@divTrunc(h.life(&pr)[1], 2) + 1, h.squad[0].?.life);

    // Burying slot 0 moves the others up.
    const n = count(s, 1);
    bury(s, &pr, k);
    try tst.expectEqual(n - 1, count(s, 1));
    var slots: [max_hero]bool = @splat(false);
    for (s.heroes.items) |*o| if (o.owner == 1) {
        slots[o.slot] = true;
    };
    for (slots[0 .. n - 1]) |on| try tst.expect(on);
}
