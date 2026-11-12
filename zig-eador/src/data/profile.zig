//! clHumanProfile: the human player's profile, save/player<N>.prf, a raw
//! dump of the 212-byte 32-bit struct (Prototype.h:718) in a packfile.
//! The bytes are the only state, so fields the port does not use yet are
//! written back unchanged.
const std = @import("std");
const GameFs = @import("../platform/fs.zig").GameFs;
const packfile = @import("../formats/packfile.zig");
const cp1251 = @import("../formats/cp1251.zig");
const Rand = @import("rand.zig").Rand;

pub const size = 212;
const off_name = 0x00;
const off_land = 0x15;
const name_len = 21;
const off_map_icons = 0x81;
const off_messages = 0x8b;

pub const max_map_icons = 10;
pub const max_messages = 10;
/// GOG FUN_0041c020: the arrows wrap at these counts.
pub const max_colors = 16;
pub const max_signs = 26;
pub const max_avatars = 21;
pub const max_difficulty = 6;
/// SCROLL_SPEED.
pub const default_scroll_speed = 9;
/// Profile slots looked at when listing (the original has no limit).
pub const max_slots = 256;

/// arHerbColors.  Entry 7 has blue 274 in the C++ table; makecol truncates
/// nothing useful there, the recomp table uses 255.
pub const herb_colors = [max_colors][3]u8{
    .{ 239, 43, 22 },   .{ 45, 45, 233 },  .{ 11, 126, 22 }, .{ 0, 0, 0 },
    .{ 255, 255, 255 }, .{ 226, 244, 46 }, .{ 101, 3, 170 }, .{ 76, 237, 255 },
    .{ 254, 135, 29 },  .{ 106, 63, 25 },  .{ 96, 250, 79 }, .{ 247, 81, 233 },
    .{ 72, 126, 98 },   .{ 113, 23, 5 },   .{ 20, 5, 113 },  .{ 97, 102, 24 },
};

/// The int fields, by their offset in the record.
pub const Field = enum(usize) {
    file_index = 0x2c,
    difficulty = 0x30,
    color1 = 0x34,
    color2 = 0x38,
    sign = 0x3c,
    avatar = 0x40,
    victory = 0x44,
    lost = 0x48,
    single = 0x4c,
    campaign = 0x50,
    hint = 0x54,
    snd_volume = 0x58,
    mus_volume = 0x5c,
    scroll_speed = 0x60,
    battle_speed = 0x64,
    map_borders = 0x68,
    enter_data = 0x6c,
    auto_lvl_up = 0x70,
    auto_combat = 0x74,
    // clMap.
    map_players = 0x98,
    map_difficulty = 0x9c,
    map_x = 0xa0,
    map_y = 0xa4,
    map_type = 0xa8,
    map_water = 0xac,
    map_hill = 0xb0,
    map_forest = 0xb4,
    map_swamp = 0xb8,
    map_isles = 0xbc,
    map_dissolve = 0xc0,
    map_neutrals = 0xc4,
    map_resource = 0xc8,
    map_site = 0xcc,
    map_population = 0xd0,
};

/// clMap (Prototype.h:649): the settings of a generated map, 15 ints at
/// 0x98 of the profile.
pub const Map = struct {
    players: i32,
    difficulty: i32,
    x: i32,
    y: i32,
    type: i32,
    water: i32,
    hill: i32,
    forest: i32,
    swamp: i32,
    isles: i32,
    dissolve: i32,
    neutrals: i32,
    resource: i32,
    site: i32,
    population: i32,
};
const off_map = @intFromEnum(Field.map_players);

/// The bool fields, by their offset in the record.
pub const Flag = enum(usize) {
    finish_tutorial = 0x78,
    war_grid = 0x79,
    war_bar = 0x7a,
    war_end_turn = 0x7b,
    herb_on_map = 0x7c,
    autosave = 0x7d,
    diplomacy = 0x7e,
    fog = 0x7f,
    tutorial_mode = 0x80,
};

pub const Human = struct {
    raw: [size]u8 = @splat(0),

    /// The constructor (GameMenu.cpp:4, GOG FUN_0046de90) without a file;
    /// clearNames fills in the names, colours and the sign.
    pub fn empty() Human {
        var h: Human = .{};
        h.set(.file_index, -1);
        h.set(.map_players, 4);
        h.set(.map_x, 10);
        h.set(.map_y, 10);
        h.setFlag(.tutorial_mode, true);
        h.set(.snd_volume, 225);
        h.set(.mus_volume, 255);
        h.resetCommon();
        return h;
    }

    /// ClearNames (GOG FUN_004aaad0).  Volumes stay: the original takes them
    /// from the current iSoundVolume and iMusicVolume, which the caller
    /// keeps in these fields.  Three rand() calls, as in GOG.
    pub fn clearNames(self: *Human, new_name: []const u8, new_land: []const u8, rand: *Rand) void {
        self.setName(new_name);
        self.setLand(new_land);
        self.resetCommon();
        self.set(.color1, @intCast(rand.rand() & 0xf));
        self.set(.color2, @intCast(rand.rand() & 0xf));
        self.set(.sign, @intCast(rand.rand() % max_signs));
    }

    fn resetCommon(self: *Human) void {
        for ([_]Field{ .victory, .lost, .hint, .battle_speed, .map_borders, .enter_data, .auto_lvl_up, .auto_combat, .difficulty }) |f|
            self.set(f, 0);
        self.set(.battle_speed, 2);
        self.set(.scroll_speed, default_scroll_speed);
        self.set(.avatar, 17);
        for ([_]Flag{ .finish_tutorial, .war_grid, .war_bar, .war_end_turn, .herb_on_map }) |f| self.setFlag(f, false);
        for ([_]Flag{ .autosave, .diplomacy, .fog }) |f| self.setFlag(f, true);
        self.setDefaultNotices();
        // clMap::SetDefault.
        self.set(.map_type, 2);
        self.set(.map_water, 25);
        self.set(.map_hill, 25);
        self.set(.map_forest, 20);
        self.set(.map_swamp, 20);
        self.set(.map_isles, 10);
        self.set(.map_dissolve, 30);
        self.set(.map_neutrals, 20);
        self.set(.map_resource, 2);
        self.set(.map_site, 2);
        self.set(.map_population, 2);
    }

    /// arMap and arMess as the "default" button of WindowSettings sets them.
    pub fn setDefaultNotices(self: *Human) void {
        for (0..max_messages) |i| self.setMessage(i, true);
        for (0..max_map_icons) |i| self.setMapIcon(i, i >= 1 and i <= 5);
    }

    pub fn get(self: *const Human, f: Field) i32 {
        return std.mem.readInt(i32, self.raw[@intFromEnum(f)..][0..4], .little);
    }

    pub fn set(self: *Human, f: Field, v: i32) void {
        std.mem.writeInt(i32, self.raw[@intFromEnum(f)..][0..4], v, .little);
    }

    pub fn map(self: *const Human) Map {
        var m: Map = undefined;
        inline for (@typeInfo(Map).@"struct".field_names, 0..) |fname, i|
            @field(m, fname) = std.mem.readInt(i32, self.raw[off_map + 4 * i ..][0..4], .little);
        return m;
    }

    pub fn setMap(self: *Human, m: Map) void {
        inline for (@typeInfo(Map).@"struct".field_names, 0..) |fname, i|
            std.mem.writeInt(i32, self.raw[off_map + 4 * i ..][0..4], @field(m, fname), .little);
    }

    pub fn flag(self: *const Human, f: Flag) bool {
        return self.raw[@intFromEnum(f)] != 0;
    }

    pub fn setFlag(self: *Human, f: Flag, v: bool) void {
        self.raw[@intFromEnum(f)] = @intFromBool(v);
    }

    pub fn mapIcon(self: *const Human, i: usize) bool {
        return self.raw[off_map_icons + i] != 0;
    }

    pub fn setMapIcon(self: *Human, i: usize, v: bool) void {
        self.raw[off_map_icons + i] = @intFromBool(v);
    }

    pub fn message(self: *const Human, i: usize) bool {
        return self.raw[off_messages + i] != 0;
    }

    pub fn setMessage(self: *Human, i: usize, v: bool) void {
        self.raw[off_messages + i] = @intFromBool(v);
    }

    pub fn fileIndex(self: *const Human) i32 {
        return self.get(.file_index);
    }

    /// sName as UTF-8 in `buf`.
    pub fn name(self: *const Human, buf: []u8) []const u8 {
        return decode(self.raw[off_name..][0..name_len], buf);
    }

    pub fn land(self: *const Human, buf: []u8) []const u8 {
        return decode(self.raw[off_land..][0..name_len], buf);
    }

    pub fn setName(self: *Human, utf8: []const u8) void {
        encode(self.raw[off_name..][0..name_len], utf8);
    }

    pub fn setLand(self: *Human, utf8: []const u8) void {
        encode(self.raw[off_land..][0..name_len], utf8);
    }

    /// clHumanProfile::Save: a profile without a file takes the first free
    /// slot.
    pub fn save(self: *Human, fs: *GameFs) !void {
        if (self.fileIndex() == -1) self.set(.file_index, freeSlot(fs));
        var pb: [64]u8 = undefined;
        const path = try std.fmt.bufPrint(&pb, "save/player{d}.prf", .{self.fileIndex()});
        const encoded = try packfile.pack(fs.gpa, &self.raw);
        defer fs.gpa.free(encoded);
        try fs.write(path, encoded);
    }
};

fn decode(field: []const u8, buf: []u8) []const u8 {
    const end = std.mem.indexOfScalar(u8, field, 0) orelse field.len;
    var n: usize = 0;
    for (field[0..end]) |b| {
        var tmp: [4]u8 = undefined;
        const len = std.unicode.utf8Encode(cp1251.toCodepoint(b), &tmp) catch continue;
        if (n + len > buf.len) break;
        @memcpy(buf[n..][0..len], tmp[0..len]);
        n += len;
    }
    return buf[0..n];
}

/// The field stays zero-terminated; other characters become '?'.
fn encode(field: []u8, utf8: []const u8) void {
    @memset(field, 0);
    var i: usize = 0;
    var at: usize = 0;
    while (at < utf8.len and i + 1 < field.len) : (i += 1) {
        field[i] = cp1251.fromCodepoint(cp1251.nextCodepoint(utf8, &at)) orelse '?';
    }
}

fn slotPath(buf: []u8, slot: i32) ?[]const u8 {
    return std.fmt.bufPrint(buf, "save/player{d}.prf", .{slot}) catch null;
}

fn freeSlot(fs: *GameFs) i32 {
    var slot: i32 = 0;
    var pb: [64]u8 = undefined;
    while (fs.exists(slotPath(&pb, slot) orelse return slot)) slot += 1;
    return slot;
}

/// save/player<slot>.prf, null if it is missing or damaged.
pub fn load(fs: *GameFs, slot: i32) ?Human {
    var pb: [64]u8 = undefined;
    const packed_bytes = fs.read(slotPath(&pb, slot) orelse return null) catch return null;
    defer fs.gpa.free(packed_bytes);
    const raw = packfile.unpack(fs.gpa, packed_bytes) catch return null;
    defer fs.gpa.free(raw);
    if (raw.len < size) return null;
    var h: Human = .{};
    @memcpy(&h.raw, raw[0..size]);
    return h;
}

/// The existing profile slots in ascending order, for WindowChoosePlayer.
/// The original lists player*.prf in directory order.
pub fn listSlots(fs: *GameFs, out: *[max_slots]i32) []const i32 {
    var n: usize = 0;
    var pb: [64]u8 = undefined;
    for (0..max_slots) |slot| {
        const s: i32 = @intCast(slot);
        if (fs.exists(slotPath(&pb, s) orelse continue)) {
            out[n] = s;
            n += 1;
        }
    }
    return out[0..n];
}

/// The end of ProfileOperator: save/game.con is a plain int.
pub fn saveActiveIndex(fs: *GameFs, index: i32) !void {
    var bytes: [4]u8 = undefined;
    std.mem.writeInt(i32, &bytes, index, .little);
    try fs.write("save/game.con", &bytes);
}

/// The end of Loader() (Prototype.cpp:5134, 5188): save/game.con holds
/// iActivePlayer (plain int), save/player<N>.prf its profile.  Without the
/// profile the game uses an empty one.
pub fn loadActive(fs: *GameFs) Human {
    const active: i32 = blk: {
        const con = fs.read("save/game.con") catch break :blk 0;
        defer fs.gpa.free(con);
        if (con.len < 4) break :blk 0;
        break :blk std.mem.readInt(i32, con[0..4], .little);
    };
    return load(fs, active) orelse .empty();
}

test "profile fields" {
    var h = Human.empty();
    try std.testing.expectEqual(@as(i32, -1), h.fileIndex());
    @memcpy(h.raw[0..4], "\xc8\xe3\xee\x00");
    h.set(.file_index, 2);
    var buf: [64]u8 = undefined;
    try std.testing.expectEqualStrings("Иго", h.name(&buf));
    try std.testing.expectEqual(@as(i32, 2), h.fileIndex());
}

test "names are stored in cp1251 and truncated to the field" {
    var h = Human.empty();
    h.setName("Анонимус");
    try std.testing.expectEqualSlices(u8, "\xc0\xed\xee\xed\xe8\xec\xf3\xf1\x00", h.raw[0..9]);
    h.setLand("123456789012345678901234");
    try std.testing.expectEqual(@as(u8, 0), h.raw[off_land + name_len - 1]);
    var buf: [64]u8 = undefined;
    try std.testing.expectEqualStrings("12345678901234567890", h.land(&buf));
    // The neighbouring field is untouched.
    try std.testing.expectEqual(@as(i32, -1), h.fileIndex());
}

test "clearNames matches FUN_004aaad0" {
    var h = Human.empty();
    h.set(.snd_volume, 100);
    h.set(.mus_volume, 50);
    h.set(.victory, 7);
    var rand: Rand = .{};
    rand.seed(1);
    var check: Rand = .{};
    check.seed(1);
    h.clearNames("А", "Б", &rand);
    try std.testing.expectEqual(@as(i32, @intCast(check.rand() & 0xf)), h.get(.color1));
    try std.testing.expectEqual(@as(i32, @intCast(check.rand() & 0xf)), h.get(.color2));
    try std.testing.expectEqual(@as(i32, @intCast(check.rand() % max_signs)), h.get(.sign));
    try std.testing.expectEqual(@as(i32, 100), h.get(.snd_volume));
    try std.testing.expectEqual(@as(i32, 50), h.get(.mus_volume));
    try std.testing.expectEqual(@as(i32, 0), h.get(.victory));
    try std.testing.expectEqual(@as(i32, 17), h.get(.avatar));
    try std.testing.expectEqual(@as(i32, 9), h.get(.scroll_speed));
    try std.testing.expectEqual(@as(i32, 2), h.get(.map_type));
    try std.testing.expectEqual(@as(i32, 2), h.get(.map_population));
    try std.testing.expect(h.flag(.autosave));
    try std.testing.expect(!h.mapIcon(0));
    try std.testing.expect(h.mapIcon(5));
    try std.testing.expect(!h.mapIcon(6));
    try std.testing.expect(h.message(9));
}

test "clMap lies at 0x98" {
    var h = Human.empty();
    const m = h.map();
    try std.testing.expectEqual(@as(i32, 4), m.players);
    try std.testing.expectEqual(@as(i32, 10), m.x);
    try std.testing.expectEqual(@as(i32, 20), m.neutrals);
    try std.testing.expectEqual(@as(i32, 2), m.population);
    var n = m;
    n.population = 4;
    h.setMap(n);
    try std.testing.expectEqual(@as(i32, 4), h.get(.map_population));
}
