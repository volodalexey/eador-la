//! LoadGame and LoadMap (GameMenu.cpp:658, Mapper.cpp:3300) for single and
//! hot-seat games: the shard from save\map%d and save\game%d after
//! packfile.unpack.  The map file gives the picture, the fog shapes, the
//! names and the border cells; the game file the province records, the
//! guards, the players, the fog and the shops.  Heroes, events and rituals
//! are not loaded yet.
const std = @import("std");
const mapgen = @import("mapgen.zig");
const province = @import("province.zig");
const Province = province.Province;
const guard_mod = @import("guard.zig");
const hero_mod = @import("hero.zig");
const player_mod = @import("player.zig");
const Proto = @import("../data/proto.zig").Proto;
const site_mod = @import("site.zig");
const newsave = @import("newsave.zig");
const profile = @import("../data/profile.zig");
const mapfile = @import("../formats/mapfile.zig");
const savefile = @import("../formats/savefile.zig");
const cp1251 = @import("../formats/cp1251.zig");
const newmap = @import("newmap.zig");

pub const max_players = mapgen.max_players;
pub const Start = union(enum) { province: i32, hero: u8 };
/// dcPlayer::arBld (player record +0x113, MAX_BLD bools).
const player_bld = 0x113;
/// dcPlayer::bInGame.
const player_in_game = 0x10f;
const max_bld = 1000;

/// What arPlayer[k] keeps that the screen and SaveGame need.
pub const Ruler = struct {
    kind: mapgen.Kind = .computer,
    ruler: i32 = 0,
    difficulty: i32 = 0,
    gold: i32 = 0,
    gem: i32 = 0,
    /// ipProvince, iMember, iCondition.
    home: i32 = 0,
    member: i32 = 0,
    condition: i32 = 0,
};

pub const Game = struct {
    shard: mapgen.Shard,
    /// The picture and the fog shapes of the map file (names are not
    /// valid: they pointed into the file data).
    map: mapfile.Map,
    /// CurGame.
    seed: u32,
    turn: i32,
    game_type: i32,
    active_player: i32,
    diplomacy: bool,
    fog_of_war: bool,
    price_mod: i32,
    n_players: usize,
    /// arPlayerSet[1..].
    requested: [max_players]i32 = @splat(0),
    /// Index k = player k (0 is the neutrals).
    rulers: [max_players + 1]Ruler = @splat(.{}),
    /// arPlayer[k].pFog by province number (index 0 unused): 0 explored,
    /// 1 seen, 2 unknown.
    fog: [max_players + 1][]u8 = @splat(&.{}),

    pub fn deinit(self: *Game) void {
        const gpa = self.shard.gpa;
        for (self.fog) |f| gpa.free(f);
        self.map.deinit(gpa);
        self.shard.deinit();
    }

    /// The saved map picture: terrain and water are already drawn on it.
    pub fn mapBitmap(self: *const Game, gpa: std.mem.Allocator) !newmap.Bitmap {
        const img = self.map.image;
        const px = try gpa.alloc(u32, img.rgba.len / 4);
        for (0..px.len) |i| {
            px[i] = (@as(u32, img.rgba[i * 4]) << 16) |
                (@as(u32, img.rgba[i * 4 + 1]) << 8) | img.rgba[i * 4 + 2];
        }
        return .{ .w = img.w, .h = img.h, .px = px };
    }

    /// The province fog shapes loaded with the map, positioned at their
    /// saved bounding boxes.
    pub fn fogShapes(self: *const Game, gpa: std.mem.Allocator) ![]newmap.Shape {
        const out = try gpa.alloc(newmap.Shape, self.map.provinces.len + 1);
        out[0] = .{ .x0 = 0, .y0 = 0, .x1 = 0, .y1 = 0 };
        for (out[1..], self.map.provinces) |*sh, p| {
            const img = p.shape;
            sh.* = .{
                .x0 = p.x0,
                .y0 = p.y0,
                .x1 = p.x1,
                .y1 = p.y1,
                .bmp = try newmap.Bitmap.init(gpa, img.w, img.h, newmap.magenta),
            };
            for (0..img.h) |y| for (0..img.w) |x| {
                const o = (y * img.w + x) * 4;
                if (img.rgba[o + 3] != 0)
                    sh.bmp.put(@intCast(x), @intCast(y), (@as(u32, img.rgba[o]) << 16) | (@as(u32, img.rgba[o + 1]) << 8) | img.rgba[o + 2]);
            };
        }
        return out;
    }

    /// Hands over the shard and player k's fog and frees the rest; `self`
    /// is invalid afterwards.
    pub fn release(self: *Game, k: usize) struct { shard: mapgen.Shard, fog: []u8 } {
        const gpa = self.shard.gpa;
        const fog = self.fog[k];
        for (self.fog, 0..) |f, j| if (j != k) gpa.free(f);
        self.map.deinit(gpa);
        return .{ .shard = self.shard, .fog = fog };
    }

    /// The players as GenerateMap and SaveGame see them; `buf` holds them.
    pub fn players(self: *const Game, buf: *[max_players]mapgen.Player) []mapgen.Player {
        for (buf[0..self.n_players], 1..) |*p, k| {
            const r = self.rulers[k];
            p.* = .{ .kind = r.kind, .ruler = r.ruler, .difficulty = r.difficulty };
        }
        return buf[0..self.n_players];
    }

    /// Where MainScreen opens (MainProcess.cpp:1388): the hero in slot
    /// iMember when iCondition > 15, the province iMember while iCondition
    /// is below 15 and the player owns it, else ipProvince.
    pub fn start(self: *const Game, k: usize) Start {
        const r = self.rulers[k];
        const n: i32 = @intCast(self.shard.count());
        if (r.condition > 15 and r.member >= 0 and r.member < hero_mod.max_hero) return .{ .hero = @intCast(r.member) };
        if (r.condition < 15 and r.member > 0 and r.member <= n and self.shard.provinces[@intCast(r.member)].player == @as(i32, @intCast(k))) return .{ .province = r.member };
        return .{ .province = r.home };
    }
};

fn int(b: []const u8, off: usize) i32 {
    return savefile.int(b, off);
}

/// Both files unpacked; neither needs to outlive the call.
pub fn fromBytes(gpa: std.mem.Allocator, pr: *const Proto, map_data: []const u8, game_data: []const u8) !Game {
    var map = try mapfile.parse(gpa, map_data);
    errdefer map.deinit(gpa);
    const n = map.provinces.len;
    var save = try savefile.parse(gpa, game_data, n);
    defer save.deinit(gpa);
    // 0 single / hot seat, 1 campaign; 2 (astral battle) is not loaded.
    if (save.flag > 1) return error.Unsupported;
    const hdr = game_data[3..][0..newsave.header_size];

    var settings: profile.Map = undefined;
    inline for (@typeInfo(profile.Map).@"struct".field_names, 0..) |f, k|
        @field(settings, f) = int(hdr, 0x100 + 4 * k);
    const x = int(hdr, 0x108);
    const y = int(hdr, 0x10c);
    if (x < 1 or x > 99 or y < 2 or y > 99) return error.BadMapSize;

    var s: mapgen.Shard = .{
        .gpa = gpa,
        .arena = .init(gpa),
        .settings = settings,
        .x = x,
        .y = y,
        .provinces = &.{},
        .grid = try .init(gpa, @intCast((y + 2) * mapgen.map_h)),
        .dist = &.{},
    };
    errdefer s.deinit();
    s.provinces = try gpa.alloc(Province, n + 1);
    @memset(s.provinces, .{});
    s.dist = try gpa.alloc(i32, n + 1);
    @memset(s.dist, -1);
    for (0..30) |r| s.race_quest[r] = int(hdr, 0x84 + 4 * r);

    const arena = s.arena.allocator();
    for (save.provinces, 1..) |*rec, i| {
        const pv = &s.provinces[i];
        pv.* = Province.fromRecord(rec);
        if (pv.number != @as(i32, @intCast(i))) return error.BadProvince;
        // LoadGame takes the name of the map's province iNumber.
        const mp = for (map.provinces) |*m| {
            if (m.index == pv.number) break m;
        } else return error.BadProvince;
        pv.name = try cp1251.toUtf8Alloc(arena, mp.name);
        pv.guard = try addGuard(&s, save.guards[i - 1]);
        if (save.garrisons[i - 1]) |*a| {
            var gh = hero_mod.Hero.fromArmy(pr, a);
            gh.owner = gh.player;
            try s.garrisons.append(gpa, gh);
            pv.garrison = @intCast(s.garrisons.items.len - 1);
        }
        for (&pv.site_guards, 0..) |*g, k| g.* = try addGuard(&s, save.site_guards[(i - 1) * province.max_site + k]);
    }
    try loadCells(&s, map.cells);

    var g: Game = .{
        .shard = s,
        .map = map,
        .seed = @bitCast(int(hdr, 0x00)),
        .turn = int(hdr, 0x04),
        .game_type = int(hdr, 0x08),
        .active_player = int(hdr, 0x3c),
        .diplomacy = hdr[0xfc] != 0,
        .fog_of_war = hdr[0xfe] != 0,
        .n_players = @intCast(save.players),
        .price_mod = 0,
    };
    // From here on `g` owns the shard and the map.
    errdefer for (g.fog) |f| gpa.free(f);
    g.shard.n_players = g.n_players;
    g.price_mod = int(hdr, 0x0c) + @divTrunc(save.players, 5);
    for (1..g.n_players + 1) |k| {
        const rec = save.player(k);
        const kind = int(rec, 0x00);
        if (kind < 1 or kind > 3) return error.BadPlayer;
        g.rulers[k] = .{
            .kind = @fromBackingInt(kind),
            .ruler = int(rec, 0x08),
            .difficulty = int(rec, 0x0c),
            .member = int(rec, 0x1c),
            .condition = int(rec, 0x20),
            .home = int(rec, 0x54),
            .gold = int(rec, 0x100),
            .gem = int(rec, 0x104),
        };
        if (0x40 + 4 * k < 0x84) g.requested[k - 1] = int(hdr, 0x40 + 4 * k);
        g.shard.capitals[k] = g.rulers[k].home;
        g.shard.rulers[k] = .{
            .kind = @fromBackingInt(kind),
            .difficulty = int(rec, 0x0c),
            .corrupt = int(rec, 0x10),
            .gold_inc = int(rec, 0x14),
            .gem_inc = int(rec, 0x18),
            .karma = int(rec, 0x24),
            .gold_add = int(rec, 0x28),
            .gem_add = int(rec, 0x2c),
            .calc_corrupt = int(rec, 0x38),
            .gold = int(rec, 0x100),
            .gem = int(rec, 0x104),
            .profile = int(rec, 0x08),
            .color = int(rec, 0x50),
            .wall = player_mod.readWall(rec),
            .chronicle = player_mod.Chronicle.read(rec),
        };
        g.shard.in_game[k] = rec[player_in_game] != 0;
        // arBld[MAX_BLD] after the seven bool flags.
        g.shard.castle[k] = try g.shard.arena.allocator().dupe(bool, @ptrCast(rec[player_bld..][0..max_bld]));
        const fog = try gpa.alloc(u8, n + 1);
        g.fog[k] = fog;
        fog[0] = 2;
        for (fog[1..], 0..) |*f, i| f.* = @intCast(std.math.clamp(int(save.explore[k], 4 * i), 0, 2));
    }
    for (save.heroes) |*h| {
        var hr = hero_mod.Hero.fromArmy(pr, &h.army);
        hr.owner = h.owner;
        hr.slot = h.slot;
        try g.shard.heroes.append(gpa, hr);
    }
    try keepSaved(&g.shard, &save, hdr);
    try g.shard.shops.ensureTotalCapacity(gpa, save.shops.len / savefile.shop_size);
    var off: usize = 0;
    while (off < save.shops.len) : (off += savefile.shop_size) {
        const b = save.shops[off..][0..savefile.shop_size];
        var sh: site_mod.Shop = .{ .kind = int(b, 0), .prices = undefined };
        for (&sh.prices, 0..) |*p, i| p.* = int(b, 4 + 4 * i);
        for (&sh.items, 0..) |*it, i| it.* = .{ int(b, 0x48 + 8 * i), int(b, 0x4c + 8 * i) };
        g.shard.shops.appendAssumeCapacity(sh);
    }
    return g;
}

/// Copies the parts of the game file that are not ported into the
/// shard's arena (Shard.saved) for SaveGame.
fn keepSaved(s: *mapgen.Shard, save: *const savefile.Save, hdr: []const u8) !void {
    const ar = s.arena.allocator();
    var sv: mapgen.Saved = .{
        .mode = save.flag,
        .header = try ar.dupe(u8, hdr[0..newsave.header_size]),
        .world_rules = try ar.dupe(u8, save.world_rules),
        .players = try ar.dupe(u8, save.player_recs),
        .tail = try ar.dupe(u8, save.tail),
    };
    s.res_buy = int(hdr, 0x0c);
    for (&s.res_prices, 0..) |*v, i| v.* = int(hdr, 0x10 + 4 * i);
    for (s.player_set[1..], 1..) |*v, k| {
        if (0x40 + 4 * k < 0x84) v.* = int(hdr, 0x40 + 4 * k);
    }
    for (1..@as(usize, @intCast(save.players)) + 1) |k| {
        s.rulers[k].holdings = player_mod.Holdings.read(save.player(k));
        sv.lists[k] = try ar.dupe(u8, save.lists[k]);
        sv.fog[k] = try ar.dupe(u8, save.explore[k]);
        sv.events[k] = try ar.dupe(u8, save.events[k]);
        s.rulers[k].story = try player_mod.Story.read(ar, save.events[k]);
    }
    s.saved = sv;
}

/// After SaveGame of a new game: the shard keeps the file it was saved
/// as, so later saves start from it.  `game_data` is unpacked.
pub fn attachSaved(s: *mapgen.Shard, game_data: []const u8) !void {
    var save = try savefile.parse(s.gpa, game_data, s.count());
    defer save.deinit(s.gpa);
    try keepSaved(s, &save, game_data[3..][0..newsave.header_size]);
}

fn addGuard(s: *mapgen.Shard, a: ?savefile.Army) !i32 {
    const army = a orelse return -1;
    try s.guards.append(s.gpa, guard_mod.fromArmy(&army));
    return @intCast(s.guards.items.len - 1);
}

/// LoadMap appends each cell at the end of its row's list.
fn loadCells(s: *mapgen.Shard, cells: []const mapfile.Cell) !void {
    const g = &s.grid;
    const tails = try s.gpa.alloc(i32, g.rows());
    defer s.gpa.free(tails);
    @memset(tails, -1);
    try g.cells.ensureTotalCapacity(s.gpa, cells.len);
    for (cells) |c| {
        if (c.y < 0 or c.y >= g.rows()) return error.BadCell;
        const row: usize = @intCast(c.y);
        const k: i32 = @intCast(g.cells.items.len);
        g.cells.appendAssumeCapacity(.{ .x = c.x, .a = c.a, .b = c.b, .c = c.c });
        if (tails[row] < 0) g.heads[row] = k else g.cells.items[@intCast(tails[row])].next = k;
        tails[row] = k;
    }
}

const tst = std.testing;
const packfile = @import("../formats/packfile.zig");
const GameFs = @import("../platform/fs.zig").GameFs;

fn readTestFile(gpa: std.mem.Allocator, path: []const u8) ![]u8 {
    const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, gpa, .unlimited) catch return error.SkipZigTest;
    defer gpa.free(bytes);
    return packfile.unpack(gpa, bytes);
}

/// Load the original's files and write the game file back: it must match
/// as the generator's own file does (newsave.compareMasked).
fn roundTrip(dir: []const u8, map_name: []const u8, game_name: []const u8) !void {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    var pb: [96]u8 = undefined;
    const mdata = try readTestFile(gpa, try std.fmt.bufPrint(&pb, "testdata/{s}/{s}", .{ dir, map_name }));
    defer gpa.free(mdata);
    const gdata = try readTestFile(gpa, try std.fmt.bufPrint(&pb, "testdata/{s}/{s}", .{ dir, game_name }));
    defer gpa.free(gdata);
    var g = try fromBytes(gpa, &pr, mdata, gdata);
    defer g.deinit();
    try tst.expectEqual(g.map.provinces.len, g.shard.count());
    try tst.expectEqual(@as(i32, @intCast(g.map.image.w)), g.shard.width());
    // Every point of the map picture is the province the cells give.
    for (g.map.provinces) |*mp| {
        const pv = &g.shard.provinces[@intCast(mp.index)];
        if (pv.terrain == province.terrain.water) continue;
        try tst.expectEqual(mp.index, g.shard.lookup(pv.x, pv.y));
    }
    var buf: [max_players]mapgen.Player = undefined;
    const pls = g.players(&buf);
    const got = try newsave.gameBytes(gpa, &pr, &g.shard, .{
        .seed = g.seed,
        .game_type = g.game_type,
        .players = pls,
        .requested = g.requested[0..g.n_players],
        .diplomacy = g.diplomacy,
        .fog = g.fog_of_war,
        .price_mod = g.price_mod,
        .turn = g.turn,
        .active_player = g.active_player,
        .auto_save = gdata[3 + 0xfd] != 0,
    });
    defer gpa.free(got);
    try tst.expectEqual(gdata.len, got.len);
    try newsave.compareMasked(gdata, got, g.shard.count(), g.n_players, pls);
}

test "load a new game of the original: single player" {
    try roundTrip("new_single", "map0.map", "game0.sav");
}

// testdata/camp_heal: a campaign shard (seed 635) at the end of turn 4
// (game0.bkc, the autosave) and at the start of turn 5; testdata/tutorial
// the first tutorial map.  The garrison of the capital has a unit.
test "load and write back campaign shards" {
    try roundTrip("camp_heal", "map0.cmp", "game0.bkc");
    try roundTrip("camp_heal", "map0.cmp", "game0.cmp");
    try roundTrip("tutorial", "map0.cmp", "game0.cmp");
}

test "write later turns back: heroes, castle, rulers, fog" {
    try roundTrip("hero_taken", "map0.map", "game0.bks");
    try roundTrip("hero_taken", "map0.map", "game0.sav");
    try roundTrip("hero_route", "map0.map", "game0.sav");
    try roundTrip("hero_route", "map0.map", "game0_turn2_auto.sav");
    try roundTrip("hero_route", "map0.map", "game0_turn3.sav");
}

test "load a new game of the original: hot seat" {
    try roundTrip("new_hot", "map0.hot", "game0.hot");
}

// testdata/hero_move: the single-player map of new_single after hiring a
// hero in the capital (province 25) and ordering it to province 35.
test "load a hero with an order" {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const mdata = try readTestFile(gpa, "testdata/hero_move/map0.map");
    defer gpa.free(mdata);
    const gdata = try readTestFile(gpa, "testdata/hero_move/game0.sav");
    defer gpa.free(gdata);
    var g = try fromBytes(gpa, &pr, mdata, gdata);
    defer g.deinit();
    try tst.expectEqual(@as(usize, 1), g.shard.heroes.items.len);
    const h = &g.shard.heroes.items[0];
    try tst.expectEqual(@as(i32, 1), h.owner);
    try tst.expectEqual(@as(u8, 0), h.slot);
    try tst.expectEqual(@as(i32, 2), h.class1);
    try tst.expectEqual(@as(i32, 25), h.province);
    try tst.expectEqual(@as(i32, hero_mod.Task.go), h.task);
    try tst.expectEqual(@as(i32, 35), h.target);
    try tst.expect(h.alive());
    try tst.expectEqual(@as(i32, 41), h.squad[0].?.type);
    try tst.expectEqual(@as(i32, 1), h.squad[1].?.type);
    try tst.expectEqual(@as(i32, 2), h.squad[2].?.type);
    try tst.expect(h.nameText(&pr).len > 0);
    try tst.expect(h.mobility(&pr, 0) >= 1);
    try tst.expectEqual(@as(i32, 21), g.rulers[1].condition);

    // The block written back equals the saved one but for the pointers.
    var save = try savefile.parse(gpa, gdata, g.shard.count());
    defer save.deinit(gpa);
    var out: std.ArrayList(u8) = .empty;
    defer out.deinit(gpa);
    try h.writeBlock(&out, gpa);
    const rec = out.items[1..][0..hero_mod.record_size];
    var o: usize = 0;
    while (o < hero_mod.record_size) : (o += 4) {
        if (o >= 0x50 and o < 0xa8) continue;
        try tst.expectEqual(savefile.int(save.heroes[0].army.rec, o), savefile.int(rec, o));
    }
    var p: usize = 1 + hero_mod.record_size;
    for (0..hero_mod.max_squad) |slot| {
        const want = save.heroes[0].army.at(slot);
        try tst.expectEqual(want != null, out.items[p] != 0);
        p += 1;
        if (want) |u| {
            try tst.expectEqualSlices(u8, u[0..0xa0], out.items[p..][0..0xa0]);
            p += savefile.unit_size;
        }
    }
}

// testdata/hero_taken: the same game two turns later (game0.sav, turn 2:
// the hero took province 35) and the backup of turn 1 (game0.bks: the
// hero still in 25 with the order, a unit hired).  The computer players
// have heroes too.
test "load later turns with heroes of every player" {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const mdata = try readTestFile(gpa, "testdata/hero_taken/map0.map");
    defer gpa.free(mdata);
    const cases = [_]struct { file: []const u8, turn: i32, prov: i32, task: i32, owner35: i32 }{
        .{ .file = "testdata/hero_taken/game0.bks", .turn = 1, .prov = 25, .task = hero_mod.Task.go, .owner35 = 0 },
        .{ .file = "testdata/hero_taken/game0.sav", .turn = 2, .prov = 35, .task = hero_mod.Task.none, .owner35 = 1 },
    };
    for (cases) |cs| {
        const gdata = try readTestFile(gpa, cs.file);
        defer gpa.free(gdata);
        var g = try fromBytes(gpa, &pr, mdata, gdata);
        defer g.deinit();
        try tst.expectEqual(cs.turn, g.turn);
        try tst.expectEqual(@as(usize, 4), g.shard.heroes.items.len);
        for (g.shard.heroes.items, 1..) |*h, k| try tst.expectEqual(@as(i32, @intCast(k)), h.owner);
        const h = &g.shard.heroes.items[0];
        try tst.expectEqual(cs.prov, h.province);
        try tst.expectEqual(cs.task, h.task);
        try tst.expectEqual(cs.owner35, g.shard.provinces[35].player);
    }
}
