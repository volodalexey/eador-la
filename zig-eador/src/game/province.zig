//! dcProvince (Unit.h:29): a province of the strategic map.  The fields
//! follow the class; pointers (guard, garrison, site guards, pictures)
//! become indices or are kept by their owners.
const std = @import("std");

pub const max_neighbour = 10;
/// MAX_PROVINCE_BLD: the arrays have one more slot.
pub const max_bld = 3;
pub const max_site = 30;
pub const max_players = 17;

/// Terrain ids of terrain.var used by the code (Globals.h:45).
pub const terrain = struct {
    pub const water = 0;
    pub const plain = 1;
    pub const forest = 2;
    pub const hill = 3;
    pub const swamp = 4;
    pub const mountain = 7;

    /// Land a castle can stand on: not water, not mountains or beyond.
    pub fn land(t: i32) bool {
        return t > 0 and t < mountain;
    }
};

/// ipType 7: a ruler's castle.
pub const type_castle = 7;

/// arSite[i]: 0 index, 1 position on the exploration scale, 2 level (shop
/// number for shops, index for quests), 3 quest, 4 guard type, 5 random
/// quest data (-1 for a world-rule site).
pub const Site = struct {
    index: i32 = 0,
    pos: i32 = 0,
    level: i32 = 0,
    quest: i32 = 0,
    guard: i32 = 0,
    data: i32 = 0,
};

pub const Province = struct {
    name: []const u8 = "",
    number: i32 = 0,
    /// Centre on the map.
    x: i32 = 0,
    y: i32 = 0,
    terrain: i32 = 0,
    type: i32 = 0,
    player: i32 = 0,
    conquered: i32 = 0,
    size: i32 = 100,
    gold_inc: i32 = 0,
    gem_inc: i32 = 0,
    population: i32 = 0,
    race: i32 = 1,
    explore: i32 = 1,
    max_buildings: i32 = 0,
    defender: i32 = 0,
    fort: i32 = 0,
    event: i32 = 0,
    pic_x: i32 = 0,
    pic_y: i32 = 0,
    unrest: [max_players]i32 = @splat(0),
    /// (value, turns) per ruler.
    attitude: [max_players][2]i32 = @splat(.{ 0, 0 }),
    neighbours: [max_neighbour]i32 = @splat(0),
    coastal: bool = false,
    warning: bool = true,
    /// Work: the castle placement score GenerateMap leaves (it is saved).
    work: i32 = 0,
    buildings: [max_bld + 1]i32 = @splat(0),
    bld_place: [max_bld + 1][2]i32 = @splat(.{ 0, 0 }),
    sites: [max_site]Site = @splat(.{}),
    /// pGuard and arSiteGuard: indices into Shard.guards, -1 for none.
    guard: i32 = -1,
    site_guards: [max_site]i32 = @splat(-1),
    /// pGarrison: index into Shard.garrisons, -1 for none.
    garrison: i32 = -1,
    /// The loaded record: padding and fields not decoded go back
    /// unchanged.
    rec: ?[record_size]u8 = null,

    pub fn neighbourList(self: *const Province) []const i32 {
        const n = std.mem.indexOfScalar(i32, &self.neighbours, 0) orelse max_neighbour;
        return self.neighbours[0..n];
    }

    /// Serialise into the 0x4E4 dcProvince record on disk.
    pub fn toRecord(self: *const Province) [record_size]u8 {
        var b: [record_size]u8 = self.rec orelse @splat(0);
        setI(&b, 0x04, self.number);
        setI(&b, 0x08, self.x);
        setI(&b, 0x0c, self.y);
        setI(&b, 0x10, self.terrain);
        setI(&b, 0x14, self.type);
        setI(&b, 0x18, self.player);
        setI(&b, 0x1c, self.conquered);
        setI(&b, 0x20, self.size);
        setI(&b, 0x24, self.gold_inc);
        setI(&b, 0x28, self.gem_inc);
        setI(&b, 0x2c, self.population);
        setI(&b, 0x30, self.race);
        setI(&b, 0x34, self.explore);
        setI(&b, 0x38, self.max_buildings);
        setI(&b, 0x3c, self.defender);
        setI(&b, 0x40, self.fort);
        setI(&b, 0x44, self.event);
        setI(&b, 0x48, self.pic_x);
        setI(&b, 0x4c, self.pic_y);
        for (self.unrest, 0..) |v, i| setI(&b, 0x50 + 4 * i, v);
        for (self.attitude, 0..) |a, i| {
            setI(&b, 0x94 + 8 * i, a[0]);
            setI(&b, 0x98 + 8 * i, a[1]);
        }
        for (self.neighbours, 0..) |v, i| setI(&b, 0x11c + 4 * i, v);
        b[0x144] = @intFromBool(self.coastal);
        b[0x145] = @intFromBool(self.warning);
        setI(&b, 0x148, self.work);
        for (self.buildings, 0..) |v, i| setI(&b, 0x154 + 4 * i, v);
        for (self.bld_place, 0..) |p, i| {
            setI(&b, 0x164 + 8 * i, p[0]);
            setI(&b, 0x168 + 8 * i, p[1]);
        }
        for (self.sites, 0..) |s, i| {
            setI(&b, 0x184 + 0x18 * i, s.index);
            setI(&b, 0x188 + 0x18 * i, s.pos);
            setI(&b, 0x18c + 0x18 * i, s.level);
            setI(&b, 0x190 + 0x18 * i, s.quest);
            setI(&b, 0x194 + 0x18 * i, s.guard);
            setI(&b, 0x198 + 0x18 * i, s.data);
        }
        return b;
    }

    /// The inverse of toRecord.  The name and the guard indices are the
    /// caller's: the record holds pointers there.
    pub fn fromRecord(b: *const [record_size]u8) Province {
        var p: Province = .{
            .number = getI(b, 0x04),
            .x = getI(b, 0x08),
            .y = getI(b, 0x0c),
            .terrain = getI(b, 0x10),
            .type = getI(b, 0x14),
            .player = getI(b, 0x18),
            .conquered = getI(b, 0x1c),
            .size = getI(b, 0x20),
            .gold_inc = getI(b, 0x24),
            .gem_inc = getI(b, 0x28),
            .population = getI(b, 0x2c),
            .race = getI(b, 0x30),
            .explore = getI(b, 0x34),
            .max_buildings = getI(b, 0x38),
            .defender = getI(b, 0x3c),
            .fort = getI(b, 0x40),
            .event = getI(b, 0x44),
            .pic_x = getI(b, 0x48),
            .pic_y = getI(b, 0x4c),
            .coastal = b[0x144] != 0,
            .warning = b[0x145] != 0,
            .work = getI(b, 0x148),
            .rec = b.*,
        };
        for (&p.unrest, 0..) |*v, i| v.* = getI(b, 0x50 + 4 * i);
        for (&p.attitude, 0..) |*a, i| a.* = .{ getI(b, 0x94 + 8 * i), getI(b, 0x98 + 8 * i) };
        for (&p.neighbours, 0..) |*v, i| v.* = getI(b, 0x11c + 4 * i);
        for (&p.buildings, 0..) |*v, i| v.* = getI(b, 0x154 + 4 * i);
        for (&p.bld_place, 0..) |*v, i| v.* = .{ getI(b, 0x164 + 8 * i), getI(b, 0x168 + 8 * i) };
        for (&p.sites, 0..) |*s, i| {
            const o = 0x184 + 0x18 * i;
            s.* = .{ .index = getI(b, o), .pos = getI(b, o + 4), .level = getI(b, o + 8), .quest = getI(b, o + 12), .guard = getI(b, o + 16), .data = getI(b, o + 20) };
        }
        return p;
    }
};

pub const record_size = 0x4E4;

fn getI(b: []const u8, off: usize) i32 {
    return std.mem.readInt(i32, b[off..][0..4], .little);
}

fn setI(b: []u8, off: usize, v: i32) void {
    std.mem.writeInt(i32, b[off..][0..4], v, .little);
}
