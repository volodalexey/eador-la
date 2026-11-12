//! save\game%d.sav / .cmp as FUN_004aa0c0 writes it (after packfile.unpack).
//! Only the parts used by the map prototype are kept; see docs/analysis.md
//! "Сохранения" for the full layout.
//!   "ok", flag byte, 0x13C game state (DAT_0051e658; +0x38 = player count
//!   DAT_0051e690), 0x194 bytes (DAT_00557238),
//!   per province 1..N: 0x4E4 record + 32 army blocks,
//!   0x4ED74 player records (17 × 0x4A34),
//!   per player 1..count: 8 hero blocks, three lists, 4 bytes per province,
//!   a list of 8-byte pairs.
//! N is not stored in the save (it comes from the map file).
const std = @import("std");

pub const province_size = 0x4E4;
pub const hero_size = 0x3AC;
pub const unit_size = 0xA4;
pub const shop_size = 0x3A8;
pub const player_size = 0x4A34;
pub const max_players = 17;
pub const heroes_per_player = 8;

/// A saved army block: the 0x3AC hero/guard record and its units.
pub const Army = struct {
    rec: []const u8,
    /// Present unit instances (0xA4 bytes, +0 = unit.var type), slot order.
    units: [16][]const u8 = @splat(@as([]const u8, &.{})),
    /// Army slot (0..15) of each present unit: slot 0 is the hero's own
    /// unit, 1..6 row I, 7..10 row II, 11..13 row III, 14..15 row IV.
    slots: [16]u8 = @splat(0),
    n: u32 = 0,

    pub fn list(self: *const Army) []const []const u8 {
        return self.units[0..self.n];
    }

    /// The unit instance in army slot `slot` (0..15), or null.
    pub fn at(self: *const Army, slot: usize) ?[]const u8 {
        for (self.slots[0..self.n], 0..) |s, k| if (s == slot) return self.units[k];
        return null;
    }

    /// Puts `unit` into `slot` (null empties it), keeping slot order.
    pub fn set(self: *Army, slot: usize, unit: ?[]const u8) void {
        var units: [16][]const u8 = undefined;
        var slots: [16]u8 = undefined;
        var n: u32 = 0;
        for (0..16) |k| {
            const u = if (k == slot) unit else self.at(k);
            if (u) |x| {
                units[n] = x;
                slots[n] = @intCast(k);
                n += 1;
            }
        }
        self.units = @splat(@as([]const u8, &.{}));
        @memcpy(self.units[0..n], units[0..n]);
        @memcpy(self.slots[0..n], slots[0..n]);
        self.n = n;
    }
};

pub const Hero = struct {
    army: Army,
    player: i32,
    /// +0x10: level as shown on the hero screen.
    level: i32,
    class: i32,
    /// +0x34: province the hero stands in.
    province: i32,
    /// Slot 0..7 of the player's hero list (player +0xCC8 + 4·slot).
    slot: u8 = 0,
    /// The player whose list the hero was saved in.
    owner: u8 = 0,
    /// Army: unit.var type of each present slot (slot 0 is the hero's own
    /// unit, types 40..43), in slot order; `n_units` are valid.
    unit_types: [16]i32 = @splat(0),
    n_units: u32 = 0,
};

pub const Save = struct {
    /// Battle state follows when > 0 (1 = campaign, 2 = battle save).
    flag: u8,
    players: i32,
    /// Province records, index 0 = province 1. Slices of the save data.
    provinces: []const [province_size]u8,
    heroes: []Hero,
    /// Per province (index i = province i + 1): the guard block (+0x14C)
    /// and the garrison block (+0x150).
    guards: []?Army = &.{},
    garrisons: []?Army = &.{},
    /// Per province: bit k set when slot k has a guard object (+0x454 + 4k,
    /// the 30 blocks after the garrison).
    slot_guards: []u32 = &.{},
    /// The site guard blocks themselves: province i, slot k at i * 30 + k.
    site_guards: []?Army = &.{},
    /// Player record +0x54: home province of player k (index k).
    home: [max_players]i32 = @splat(0),
    /// All 17 player records (slice of the save data).
    player_recs: []const u8 = &.{},
    /// Per player (index = player number): the exploration list saved
    /// after the heroes (player +0x4A28), one i32 per province: 0 explored,
    /// 1 seen (translucent fog), 2 unknown (black).
    explore: [max_players][]const u8 = @splat(&.{}),
    /// Modes 0 and 1: iMaxPureItem and the shop records (0x3A8 each) that
    /// follow the players.
    max_pure_item: i32 = 0,
    shops: []const u8 = &.{},
    /// The raw parts SaveGame must write back unchanged: WorldRules, each
    /// province's garrison block, per player the defender, outer building
    /// and ritual lists (before the fog) and the event list (after it),
    /// and everything past the shops (rituals, Tutorial).  Slices of the
    /// save data.
    world_rules: []const u8 = &.{},
    garrison_blocks: [][]const u8 = &.{},
    lists: [max_players][]const u8 = @splat(&.{}),
    events: [max_players][]const u8 = @splat(&.{}),
    tail: []const u8 = &.{},
    /// Unit instances created in this session (hired troops), owned here.
    pool: std.ArrayList(*[unit_size]u8) = .empty,

    /// A zeroed unit instance that lives as long as the save.
    pub fn newUnit(self: *Save, gpa: std.mem.Allocator) ![]u8 {
        const u = try gpa.create([unit_size]u8);
        errdefer gpa.destroy(u);
        @memset(u, 0);
        try self.pool.append(gpa, u);
        return u;
    }

    pub fn player(self: *const Save, k: usize) []const u8 {
        return self.player_recs[k * player_size ..][0..player_size];
    }

    pub fn deinit(self: *Save, gpa: std.mem.Allocator) void {
        gpa.free(self.guards);
        gpa.free(self.garrisons);
        gpa.free(self.garrison_blocks);
        gpa.free(self.slot_guards);
        gpa.free(self.site_guards);
        for (self.pool.items) |u| gpa.destroy(u);
        self.pool.deinit(gpa);
        gpa.free(self.provinces);
        gpa.free(self.heroes);
    }
};

pub fn int(b: []const u8, off: usize) i32 {
    return std.mem.readInt(i32, b[off..][0..4], .little);
}

pub fn setInt(b: []u8, off: usize, v: i32) void {
    std.mem.writeInt(i32, b[off..][0..4], v, .little);
}

const Reader = struct {
    d: []const u8,
    p: usize = 0,

    fn take(self: *Reader, n: usize) ![]const u8 {
        if (n > self.d.len - self.p) return error.Truncated;
        const s = self.d[self.p..][0..n];
        self.p += n;
        return s;
    }

    fn byte(self: *Reader) !u8 {
        return (try self.take(1))[0];
    }

    fn i32le(self: *Reader) !i32 {
        return int(try self.take(4), 0);
    }

    /// Byte "present", record, 16 × (byte, unit instance).
    fn block(self: *Reader) !?Army {
        if ((try self.byte()) == 0) return null;
        var a: Army = .{ .rec = try self.take(hero_size) };
        for (0..16) |slot| {
            if ((try self.byte()) != 0) {
                a.units[a.n] = try self.take(unit_size);
                a.slots[a.n] = @intCast(slot);
                a.n += 1;
            }
        }
        return a;
    }

    /// Count n, then (n + 1) items of `size` bytes.
    fn list(self: *Reader, size: usize, plus_one: bool) !void {
        const n = try self.i32le();
        if (n < -1 or n > 1 << 20) return error.BadList;
        const items: usize = @intCast(n + @intFromBool(plus_one));
        _ = try self.take(items * size);
    }
};

/// Scans the province records to count them: each record's +0x04 must be
/// its 1-based index, so the sequence stops at the first mismatch. This is
/// cheaper than loading the map file and matches dump.py.
pub fn provinceCount(data: []const u8) !usize {
    var r: Reader = .{ .d = data };
    if (!std.mem.eql(u8, try r.take(2), "ok")) return error.NotASave;
    _ = try r.byte();
    _ = try r.take(0x13C);
    _ = try r.take(0x194);
    var count: usize = 0;
    while (r.p + province_size <= r.d.len) {
        const rec = r.d[r.p..][0..province_size];
        if (int(rec, 4) != @as(i32, @intCast(count + 1))) break;
        r.p += province_size;
        _ = try r.block();
        _ = try r.block();
        for (0..30) |_| _ = try r.block();
        count += 1;
    }
    return count;
}

/// `data` must stay alive while the result is used. `n_provinces` comes
/// from the map file (mapfile.zig) or from `provinceCount`.
pub fn parse(gpa: std.mem.Allocator, data: []const u8, n_provinces: usize) !Save {
    var r: Reader = .{ .d = data };
    if (!std.mem.eql(u8, try r.take(2), "ok")) return error.NotASave;
    const flag = try r.byte();
    if (flag >= 2) return error.Unsupported;
    const state = try r.take(0x13C);
    const world_rules = try r.take(0x194);
    const provinces = try gpa.alloc([province_size]u8, n_provinces);
    errdefer gpa.free(provinces);
    const guards = try gpa.alloc(?Army, n_provinces);
    errdefer gpa.free(guards);
    const garrisons = try gpa.alloc(?Army, n_provinces);
    errdefer gpa.free(garrisons);
    const garrison_blocks = try gpa.alloc([]const u8, n_provinces);
    errdefer gpa.free(garrison_blocks);
    const slot_guards = try gpa.alloc(u32, n_provinces);
    errdefer gpa.free(slot_guards);
    const site_guards = try gpa.alloc(?Army, n_provinces * 30);
    errdefer gpa.free(site_guards);
    for (provinces, 0..) |*p, i| {
        p.* = (try r.take(province_size))[0..province_size].*;
        if (int(p, 4) != @as(i32, @intCast(i + 1))) return error.BadProvince;
        guards[i] = try r.block();
        const garrison_at = r.p;
        garrisons[i] = try r.block();
        garrison_blocks[i] = data[garrison_at..r.p];
        slot_guards[i] = 0;
        for (0..30) |k| {
            site_guards[i * 30 + k] = try r.block();
            if (site_guards[i * 30 + k] != null) slot_guards[i] |= @as(u32, 1) << @intCast(k);
        }
    }
    const player_recs = try r.take(max_players * player_size);
    var home: [max_players]i32 = undefined;
    for (&home, 0..) |*h, k| h.* = int(player_recs, k * player_size + 0x54);
    const players = int(state, 0x38);
    if (players < 0 or players >= max_players) return error.BadPlayers;
    var heroes: std.ArrayList(Hero) = .empty;
    errdefer heroes.deinit(gpa);
    var explore: [max_players][]const u8 = @splat(&.{});
    var lists: [max_players][]const u8 = @splat(&.{});
    var events: [max_players][]const u8 = @splat(&.{});
    for (0..@intCast(players)) |pl| {
        for (0..heroes_per_player) |slot| {
            const b = try r.block() orelse continue;
            var types: [16]i32 = @splat(0);
            for (b.list(), 0..) |u, j| types[j] = int(u, 0);
            try heroes.append(gpa, .{
                .army = b,
                .player = int(b.rec, 0x08),
                .level = int(b.rec, 0x10),
                .class = int(b.rec, 0x18),
                .province = int(b.rec, 0x34),
                .slot = @intCast(slot),
                .owner = @intCast(pl + 1),
                .unit_types = types,
                .n_units = b.n,
            });
        }
        const lists_at = r.p;
        try r.list(4, true);
        try r.list(4, true);
        try r.list(8, true);
        lists[pl + 1] = data[lists_at..r.p];
        explore[pl + 1] = try r.take(4 * n_provinces);
        const events_at = r.p;
        try r.list(8, false);
        events[pl + 1] = data[events_at..r.p];
    }
    var max_pure_item: i32 = 0;
    var shops: []const u8 = &.{};
    // SaveGame writes the shops for modes 0 and 1 (MODE < 2); the
    // campaign's Campaign block and shards follow the rituals.
    if (flag < 2) {
        max_pure_item = try r.i32le();
        const n = try r.i32le();
        if (n < 0 or n > 1 << 16) return error.BadList;
        shops = try r.take(@as(usize, @intCast(n)) * shop_size);
    }
    return .{ .world_rules = world_rules, .garrison_blocks = garrison_blocks, .lists = lists, .events = events, .tail = data[r.p..], .max_pure_item = max_pure_item, .shops = shops, .explore = explore, .flag = flag, .players = players, .provinces = provinces, .heroes = try heroes.toOwnedSlice(gpa), .home = home, .player_recs = player_recs, .guards = guards, .garrisons = garrisons, .slot_guards = slot_guards, .site_guards = site_guards };
}
