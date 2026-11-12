//! GenerateMap (Mapper.cpp:521) up to the landscape picture: everything
//! that decides the shard before its picture is drawn.
//!
//!   race quests   one Roll per race over its quests
//!   grid          jittered corner points joined by DrawLine into the
//!                 border cells of each map row (Map[y])
//!   shapes        neighbours, size, centre and building places
//!   terrain       water (sea round an island or lakes), islands, hills and
//!                 mountains, forests, swamps by Flooder
//!   capitals      start places far apart (MakeMap distances), the castles
//!   resources     simple resources next to each castle, then the rest
//!   population    type, population, exploration, name, guard, income,
//!                 quest and ordinary sites of every land province
//!   campaign      quests of Campaign.arQuest by distance from player 1
//!
//! The rand() stream is MSVC's, seeded with the game seed.  The GOG
//! executable does the same (recomp orig/mapgen.zig), and the tests check
//! the result against maps the original game made (testdata/).
//! The map picture is a later stage.
const std = @import("std");
const Rand = @import("../data/rand.zig").Rand;
const profile = @import("../data/profile.zig");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const province = @import("province.zig");
const Province = province.Province;
const terrain = province.terrain;
const guard_mod = @import("guard.zig");
const Guard = guard_mod.Guard;
const unit_mod = @import("unit.zig");
const Unit = unit_mod.Unit;
const site_mod = @import("site.zig");
const hero_mod = @import("hero.zig");
const player_mod = @import("player.zig");
const rand_mod = @import("../data/rand.zig");

/// Map_w, Map_h: the size of a generated province in pixels.
pub const map_w = 175;
pub const map_h = 175;
pub const max_players = 16;

/// A border cell (Map_Border): map point (x, row) between provinces a and
/// b; c is a third one at a corner.  `next` links the cells of a row,
/// newest first, as the original's lists.
pub const Cell = struct { x: i32, a: i32, b: i32, c: i32, next: i32 = -1 };

pub const Grid = struct {
    cells: std.ArrayList(Cell) = .empty,
    /// The first cell of each row, -1 for none.
    heads: []i32,

    pub fn init(gpa: std.mem.Allocator, n_rows: usize) !Grid {
        const heads = try gpa.alloc(i32, n_rows);
        @memset(heads, -1);
        return .{ .heads = heads };
    }

    pub fn deinit(self: *Grid, gpa: std.mem.Allocator) void {
        self.cells.deinit(gpa);
        gpa.free(self.heads);
    }

    pub fn rows(self: *const Grid) usize {
        return self.heads.len;
    }

    fn find(self: *Grid, x: i32, y: usize) ?*Cell {
        var i = self.heads[y];
        while (i >= 0) {
            const c = &self.cells.items[@intCast(i)];
            if (c.x == x) return c;
            i = c.next;
        }
        return null;
    }

    fn addSide(c: *Cell, v: i32) void {
        if (c.a != v and c.b != v and c.c != v) c.c = v;
    }

    fn put(self: *Grid, gpa: std.mem.Allocator, x: i32, y: i32, a: i32, b: i32, c: i32, first: bool) !void {
        if (y < 0 or y >= self.heads.len) return error.MapTooLarge;
        const row: usize = @intCast(y);
        if (self.find(x, row)) |cell| {
            addSide(cell, a);
            addSide(cell, b);
            if (first) addSide(cell, c);
            return;
        }
        try self.cells.append(gpa, .{ .x = x, .a = a, .b = b, .c = if (first) c else 0, .next = self.heads[row] });
        self.heads[row] = @intCast(self.cells.items.len - 1);
    }

    /// DrawLine: a border from (x1, y1) to (x2, y2) between a and b; c
    /// joins the first cell.  The minor axis is a float stepped by the
    /// slope and truncated; the end point is not drawn.
    pub fn line(self: *Grid, gpa: std.mem.Allocator, x1: i32, y1: i32, x2: i32, y2: i32, a: i32, b: i32, c: i32) !void {
        if (@abs(x1 - x2) > @abs(y1 - y2)) {
            const step = std.math.sign(x2 - x1);
            const dy: f32 = @as(f32, @floatFromInt(y2 - y1)) / @as(f32, @floatFromInt(@abs(x2 - x1)));
            var fy: f32 = @floatFromInt(y1);
            var y = y1;
            var x = x1;
            while (x != x2) : (x += step) {
                try self.put(gpa, x, y, a, b, c, x == x1);
                fy += dy;
                y = @intFromFloat(fy);
            }
        } else {
            if (y1 == y2) return;
            const step = std.math.sign(y2 - y1);
            const dx: f32 = @as(f32, @floatFromInt(x2 - x1)) / @as(f32, @floatFromInt(@abs(y2 - y1)));
            var fx: f32 = @floatFromInt(x1);
            var x = x1;
            var y = y1;
            while (y != y2) : (y += step) {
                try self.put(gpa, x, y, a, b, c, y == y1);
                fx += dx;
                x = @intFromFloat(fx);
            }
        }
    }

    /// GetProvinceIndex: the province at (x, y), 0 outside the map.
    /// `limit` is the province count.
    pub fn lookup(self: *const Grid, x: i32, y: i32, limit: i32) i32 {
        if (y < 0 or y >= self.heads.len or x < 0) return 0;
        var l: ?Cell = null;
        var r: ?Cell = null;
        var left: i32 = -5000;
        var right: i32 = 5000;
        var l_node: i32 = 1;
        var r_node: i32 = 1;
        var i = self.heads[@intCast(y)];
        while (i >= 0) {
            const c = self.cells.items[@intCast(i)];
            i = c.next;
            if (x - c.x > 0) {
                if (x - c.x < x - left) {
                    if (l != null and l.?.c > 0 and @abs(left - c.x) == l_node) l_node += 1 else {
                        left = c.x;
                        l = c;
                        l_node = 1;
                    }
                }
            } else if (c.x - x < right - x) {
                if (r != null and r.?.c > 0 and @abs(right - c.x) == r_node) r_node += 1 else {
                    right = c.x;
                    r = c;
                    r_node = 1;
                }
            }
        }
        const lc = l orelse return 0;
        const rc = r orelse return 0;
        var res: i32 = 0;
        for ([3]i32{ rc.a, rc.b, rc.c }) |p| for ([3]i32{ lc.a, lc.b, lc.c }) |q| {
            if (p == q and p > 0) res = p;
        };
        return if (res < 0 or res > limit) 0 else res;
    }
};

/// arPlayer[i].Control; `lord` is a campaign local lord (MakeLocalLord).
pub const Kind = enum(i32) { human = 1, computer = 2, lord = 3 };

/// arPlayer[i] as GenerateMap reads it.
pub const Player = struct {
    kind: Kind,
    /// ipProfile: players.var record of a computer, profile file of a human.
    ruler: i32,
    difficulty: i32,
    /// A human's sLand: the capital's name.
    land: []const u8 = "",
};

/// The part of arPlayer[k] the turn needs (Unit.h:589): Control,
/// iDifficulty, iCorrupt, GoldInc, GemInc, iKarma, iGoldAdd, iGemAdd,
/// iCalcCorrupt, Gold, Gem.
pub const Ruler = struct {
    kind: Kind = .computer,
    difficulty: i32 = 0,
    corrupt: i32 = 0,
    gold_inc: i32 = 0,
    gem_inc: i32 = 0,
    karma: i32 = 0,
    gold_add: i32 = 0,
    gem_add: i32 = 0,
    calc_corrupt: i32 = 0,
    gold: i32 = 0,
    gem: i32 = 0,
    /// ipProfile: players.var record of a computer, profile file of a human.
    profile: i32 = 0,
    /// iColor (Init's Color: the slot, 0-based).
    color: i32 = 0,
    /// iWall: the strength of the capital's walls.
    wall: i32 = 100,
    /// arRecruit, arLibrary, arTreasury, arGroup, Shop, arRes, bEnter.
    holdings: player_mod.Holdings = .{},
    /// iWin, iLose, iQuest, iTimeShift, arKills, arLoot, Politic.
    chronicle: player_mod.Chronicle = .{},
    /// pStory, pPlace (in the shard's arena); empty until loaded or made.
    story: player_mod.Story = .{},
};

pub const Params = struct {
    seed: u32,
    /// CurGame.Map as NewGame set it.
    map: profile.Map,
    /// CurGame.iGameType: 1 single, 2 campaign, 3 hot seat.
    game_type: i32 = 1,
    /// Players 1..n (index 0 is player 1).
    players: []const Player,
    /// bLakeShip (eador.cfg): lakes count as coast.
    lake_ships: bool = false,
    /// Campaign.arQuest (game_type 2): the generator fills in the
    /// province and site of each quest it places.
    campaign_quests: []CampQuest = &.{},
    /// Tutorial.Map: the fixed map of the first campaign mission
    /// (Mapper.cpp:1031) instead of the landscape, castles and resources.
    tutorial: bool = false,
    /// arHumans[0].sLand when iEnterData is 3: the player's castle keeps
    /// this name instead of MakeCastleName.
    tutorial_land: ?[]const u8 = null,
};

/// Campaign.arQuest[Q] (GOG DAT_005349a8, 16 bytes, MAX_CAMP_QUEST = 20).
pub const CampQuest = struct {
    /// Index into quest.var; 0 is an empty slot.
    quest: i32 = 0,
    /// 0 waiting, 1 active, 2 done; -1 no province was found.
    state: i32 = 0,
    province: i32 = 0,
    site: i32 = 0,
};

/// The parts of a game file that are not ported yet (in the shard's
/// arena): SaveGame writes them back as they were and puts the ported
/// state over them (newsave.gameBytes).
pub const Saved = struct {
    /// The MODE byte: 0 single or hot seat, 1 campaign.
    mode: u8 = 0,
    /// CurGame (0x13C), WorldRules, arPlayer[0..16].
    header: []const u8,
    world_rules: []const u8,
    players: []const u8,
    /// Per player: pDefender, pOuterBld, pRitual (before pFog), pFog,
    /// pStory/pPlace (after it).
    lists: [max_players + 1][]const u8 = @splat(&.{}),
    fog: [max_players + 1][]const u8 = @splat(&.{}),
    events: [max_players + 1][]const u8 = @splat(&.{}),
    /// Everything after the shops: pRitual, in the campaign Campaign and
    /// the shards, then Tutorial.
    tail: []const u8,
};

/// The generated shard.  Provinces are 1-based (index 0 is the empty
/// province 0).
pub const Shard = struct {
    gpa: std.mem.Allocator,
    arena: std.heap.ArenaAllocator,
    /// CurGame.Map: the settings before GenerateMap changed them.
    settings: profile.Map,
    /// Columns and rows (rows made even).
    x: i32,
    y: i32,
    provinces: []Province,
    grid: Grid,
    /// pMoveMap after the castles: steps to the nearest castle (8 where
    /// none reaches).  In the campaign campaignQuests replaces it with
    /// MakeMap steps from player 1's capital (-1 where none reaches).
    dist: []i32,
    capitals: [max_players + 1]i32 = @splat(0),
    /// CurGame.arRaceQuest.
    race_quest: [proto_mod.max_race + 1]i32 = @splat(0),
    /// arRes: resources placed so far, by resource.
    res_count: [32]i32 = @splat(0),
    /// Province and site guards; provinces refer to them by index.
    guards: std.ArrayList(Guard) = .empty,
    /// pFirstShop: shops of the sites; a shop site keeps its index.
    shops: std.ArrayList(site_mod.Shop) = .empty,
    /// MedSize: the average province size.
    med_size: i32 = 1,
    /// arPlayer[k].arHero of all players, in player and slot order.
    heroes: std.ArrayList(hero_mod.Hero) = .empty,
    /// pGarrison of the provinces (a dcHero without class).
    garrisons: std.ArrayList(hero_mod.Hero) = .empty,
    /// arPlayer[k].arBld: the castle buildings of player k (index =
    /// inner_build.var record); empty for a new game.
    castle: [max_players + 1][]bool = @splat(&.{}),
    /// CurGame.iMaxPlayers and arPlayer[k].bInGame.
    n_players: usize = 0,
    in_game: [max_players + 1]bool = @splat(false),
    rulers: [max_players + 1]Ruler = @splat(.{}),
    /// CurGame.ResBuy and ResPrices (header +0x0C, +0x10).
    res_buy: i32 = 0,
    res_prices: [player_mod.max_res]i32 = @splat(100),
    /// CurGame.arPlayerSet (header +0x40): the ruler asked for in the new
    /// game's slot, 0 for a random one.
    player_set: [max_players + 1]i32 = @splat(0),
    /// The game file this shard was loaded from or saved as.
    saved: ?Saved = null,
    /// pDefender / pOuterBld / pRitual lists of a game that was not saved
    /// yet (game/ritual.zig makes them on first use).
    ritual_lists: [max_players + 1]?[]u8 = @splat(null),

    pub fn deinit(self: *Shard) void {
        self.heroes.deinit(self.gpa);
        self.garrisons.deinit(self.gpa);
        self.guards.deinit(self.gpa);
        self.shops.deinit(self.gpa);
        self.grid.deinit(self.gpa);
        self.gpa.free(self.provinces);
        self.gpa.free(self.dist);
        self.arena.deinit();
    }

    pub fn count(self: *const Shard) usize {
        return self.provinces.len - 1;
    }

    pub fn at(self: *Shard, p: anytype) *Province {
        return &self.provinces[@intCast(p)];
    }

    pub fn terrainOf(self: *const Shard, p: anytype) i32 {
        return self.provinces[@intCast(p)].terrain;
    }

    pub fn lookup(self: *const Shard, x: i32, y: i32) i32 {
        return self.grid.lookup(x, y, self.settings.x * self.settings.y);
    }

    /// bmpMap: X·Map_w + Map_w·7/6 wide; Y·(Map_h + Map_h/3)/2 + Map_h
    /// high, at least 750.
    pub fn width(self: *const Shard) i32 {
        return self.x * map_w + @divTrunc(map_w * 7, 6);
    }

    pub fn height(self: *const Shard) i32 {
        return @max(750, @divTrunc(self.y * (map_h + @divTrunc(map_h, 3)), 2) + map_h);
    }
};

fn below(r: *Rand, n: i32) i32 {
    if (n <= 0) return 0;
    return @intCast(r.random(@intCast(n)));
}

/// Roll(false) over (id, weight) pairs, as filled by ClearRoll and
/// RollerN/RollerP.
pub fn roll(r: *Rand, ids: []const i32, weights: []const i32) i32 {
    var sum: u32 = 0;
    for (weights) |w| sum +%= @bitCast(w);
    const v = r.random(sum) + 1;
    var acc: u32 = 0;
    var k: usize = 0;
    for (weights, 0..) |w, i| {
        acc +%= @bitCast(w);
        k = i;
        if (acc >= v) break;
    }
    while (k + 1 < weights.len and weights[k] == 0) k += 1;
    return ids[k];
}

const Gen = struct {
    s: *Shard,
    r: *Rand,
    pr: *const Proto,
    p: Params,
    /// mpMap: the working copy of the settings.
    m: profile.Map,
    /// WaterPlaced, HillPlaced, ForestPlaced, SwampPlaced, MountainPlaced.
    placed: struct { water: i32 = 0, hill: i32 = 0, forest: i32 = 0, swamp: i32 = 0, mountain: i32 = 0 } = .{},
    /// PlainCastle, ForestCastle, HillCastle, SwampCastle by terrain 1..4.
    castles_on: [5]i32 = @splat(0),
    /// pNames, inverted: province names already taken, shared by the local
    /// lords' castles and the population stage.
    used_names: []bool,

    fn n(self: *const Gen) i32 {
        return @intCast(self.s.count());
    }

    /// MakeCastleName (Mapper.cpp:489): a free name that lists the castle
    /// type explicitly and fits the terrain.
    fn castleName(self: *Gen, pv: *Province) void {
        var rl: unit_mod.Roller = .{};
        var cnt: usize = 0;
        const names = self.pr.province_names;
        for (names[1..], 1..) |*pn, i| {
            if (self.used_names[i] or !pn.checkTerrain(pv.terrain)) continue;
            if (std.mem.indexOfScalar(i32, &pn.types, province.type_castle) == null) continue;
            rl.ids[cnt] = @intCast(i);
            rl.weights[cnt] = pn.rarity;
            cnt += 1;
        }
        const k: usize = @intCast(rl.roll(self.r));
        pv.name = names[k].name;
        self.used_names[k] = true;
    }

    fn pick(self: *Gen, comptime pred: fn (i32) bool) !usize {
        var tries: usize = 0;
        while (tries < 1_000_000) : (tries += 1) {
            const p: usize = @intCast(below(self.r, self.n()) + 1);
            if (pred(self.s.terrainOf(p))) return p;
        }
        return error.GenerationStuck;
    }

    fn raceQuests(self: *Gen) void {
        for (self.pr.races, 0..) |race, i| {
            if (i >= self.s.race_quest.len) break;
            self.s.race_quest[i] = 1;
            var ids: [proto_mod.max_race_quest]i32 = undefined;
            var ws: [proto_mod.max_race_quest]i32 = undefined;
            var k: usize = 0;
            for (race.quests) |q| {
                if (q <= 0) continue;
                ids[k] = q;
                ws[k] = if (q < self.pr.quests.len) self.pr.quests[@intCast(q)].possibility else 0;
                k += 1;
            }
            if (k > 0) self.s.race_quest[i] = roll(self.r, ids[0..k], ws[0..k]);
        }
    }

    /// The corner rows Row1 and Row2: point and the three provinces
    /// around it.
    const Pt = [5]i32;

    fn jitter(self: *Gen, k: usize, shift: i32, y: i32) [2]i32 {
        const ki: i32 = @intCast(k);
        const px = ki * map_w + below(self.r, @divTrunc(map_w, 3)) + shift;
        const py = y + below(self.r, @divTrunc(map_h, 3) - 2) - @divTrunc(map_h, 6) + 1;
        return .{ px, py };
    }

    fn grid(self: *Gen) !void {
        const gpa = self.s.gpa;
        const g = &self.s.grid;
        const X: usize = @intCast(self.s.x);
        const xi = self.s.x;
        var row1: [100]Pt = @splat(@splat(0));
        var row2: [100]Pt = @splat(@splat(0));
        if (X + 1 > row1.len) return error.MapTooLarge;
        const offset = @divTrunc(map_w, 6) + @divTrunc(map_w, 3);
        var y: i32 = @divTrunc(map_h, 6);
        for (0..X) |i| {
            const pt = self.jitter(i, offset, y);
            row1[i] = .{ pt[0], pt[1], 0, 0, @intCast(i + 1) };
        }
        const third = @divTrunc(map_h, 3);
        const lines: usize = @intCast(@divTrunc(self.s.y, 2));
        for (0..lines) |ln| {
            const line: i32 = @intCast(ln);
            y += third;
            for (0..X + 1) |i| {
                const pt = self.jitter(i, 0, y);
                row2[i][0] = pt[0];
                row2[i][1] = pt[1];
                row2[i][2] = if (i == 0) 0 else row1[i - 1][4];
                if (i == X) {
                    row2[i][3] = 0;
                    row2[i][4] = row1[i - 1][3];
                } else {
                    row2[i][3] = row1[i][4];
                    row2[i][4] = row1[i][2];
                }
            }
            for (0..X) |i| {
                const a = row1[i];
                try g.line(gpa, a[0], a[1], row2[i][0], row2[i][1], a[2], a[4], a[3]);
                try g.line(gpa, a[0], a[1], row2[i + 1][0], row2[i + 1][1], a[3], a[4], a[2]);
            }
            if (line > 0) {
                const a = row1[X];
                try g.line(gpa, a[0], a[1], row2[X][0], row2[X][1], a[2], a[4], a[3]);
            }

            y += third;
            for (0..X + 1) |i| {
                const pt = self.jitter(i, 0, y);
                row1[i] = .{ pt[0], pt[1], row2[i][2], row2[i][3], if (i == 0) 0 else line * 2 * xi + xi + @as(i32, @intCast(i)) };
                const b = row2[i];
                try g.line(gpa, b[0], b[1], row1[i][0], row1[i][1], b[2], b[3], b[4]);
            }

            y += third;
            for (0..X + 1) |i| {
                const pt = self.jitter(i, offset, y);
                row2[i][0] = pt[0];
                row2[i][1] = pt[1];
                row2[i][2] = row1[i][4];
                if (i == X) {
                    row2[i][3] = 0;
                    row2[i][4] = 0;
                } else {
                    row2[i][3] = row1[i + 1][4];
                    row2[i][4] = row1[i][3];
                }
            }
            for (1..X + 1) |i| {
                const a = row1[i];
                try g.line(gpa, a[0], a[1], row2[i - 1][0], row2[i - 1][1], a[2], a[4], a[3]);
                try g.line(gpa, a[0], a[1], row2[i][0], row2[i][1], a[3], a[4], a[2]);
            }
            {
                const a = row1[0];
                try g.line(gpa, a[0], a[1], row2[0][0], row2[0][1], a[3], a[4], a[3]);
            }

            y += third;
            for (0..X + 1) |i| {
                const pt = self.jitter(i, offset, y);
                row1[i] = .{ pt[0], pt[1], row2[i][2], row2[i][3], if (i == X) 0 else (line + 1) * xi * 2 + @as(i32, @intCast(i)) + 1 };
                const b = row2[i];
                try g.line(gpa, b[0], b[1], row1[i][0], row1[i][1], b[2], b[3], b[4]);
            }
        }
        y += third;
        for (0..X + 1) |i| {
            const pt = self.jitter(i, 0, y);
            row2[i][0] = pt[0];
            row2[i][1] = pt[1];
            row2[i][2] = if (i == 0) 0 else row1[i - 1][4];
            if (i == X) {
                row2[i][3] = 0;
                row2[i][4] = row1[i - 1][2];
            } else {
                row2[i][3] = row1[i][4];
                row2[i][4] = row1[i][3];
            }
        }
        if (X > 1) for (1..X) |i| {
            const a = row1[i];
            try g.line(gpa, a[0], a[1], row2[i][0], row2[i][1], a[2], 0, a[3]);
            try g.line(gpa, a[0], a[1], row2[i + 1][0], row2[i + 1][1], a[3], 0, a[2]);
        };
        try g.line(gpa, row1[0][0], row1[0][1], row2[1][0], row2[1][1], row1[0][3], 0, row1[0][2]);
        try g.line(gpa, row1[X][0], row1[X][1], row2[X][0], row2[X][1], row1[X][2], 0, row1[X][3]);
    }

    /// The first and the last border cell of province `i` in row `y`
    /// (x1, x2 of the original; -1 for none).
    fn span(g: *const Grid, y: usize, i: i32) [2]i32 {
        var x1: i32 = -1;
        var x2: i32 = -1;
        var k = g.heads[y];
        while (k >= 0) {
            const c = g.cells.items[@intCast(k)];
            k = c.next;
            if (c.a != i and c.b != i and c.c != i) continue;
            if (x1 >= 0) x2 = c.x else x1 = c.x;
        }
        return .{ x1, x2 };
    }

    fn rowWidth(sp: [2]i32) i32 {
        if (sp[0] < 0) return 0;
        return if (sp[1] >= 0) @intCast(@abs(sp[1] - sp[0])) else 1;
    }

    /// Neighbours, sizes, centres and building places.
    fn shapes(self: *Gen) void {
        const s = self.s;
        const g = &s.grid;
        const h: usize = @intCast((self.s.y + 1) * map_h);
        var full: i64 = 0;
        for (s.provinces[1..], 1..) |*pv, pu| {
            const i: i32 = @intCast(pu);
            pv.number = i;
            pv.neighbours = @splat(0);
            pv.player = 0;
            pv.size = 0;
            var nb: usize = 0;
            for (0..h) |y| {
                var k = g.heads[y];
                while (k >= 0) {
                    const c = g.cells.items[@intCast(k)];
                    k = c.next;
                    if (c.a != i and c.b != i and c.c != i) continue;
                    for ([3]i32{ c.a, c.b, c.c }) |v| {
                        if (v == i or v <= 0 or nb >= province.max_neighbour) continue;
                        if (std.mem.indexOfScalar(i32, pv.neighbours[0..nb], v) != null) continue;
                        pv.neighbours[nb] = v;
                        nb += 1;
                    }
                }
                pv.size += rowWidth(span(g, y, i));
            }
            full += pv.size;
            var acc: i32 = 0;
            for (0..h) |y| {
                const sp = span(g, y, i);
                acc += rowWidth(sp);
                if (acc >= @divTrunc(pv.size, 2)) {
                    pv.y = @intCast(y);
                    pv.x = @divTrunc(sp[0] + sp[1], 2);
                    break;
                }
            }
            self.buildingPlaces(pv);
        }
        s.med_size = @intCast(@max(1, @divTrunc(full, self.n())));
    }

    const bld_offsets = [10][2]i32{ .{ -50, 25 }, .{ -70, -5 }, .{ -50, -35 }, .{ 50, 25 }, .{ 70, -5 }, .{ 50, -35 }, .{ -25, -55 }, .{ 35, -55 }, .{ -30, 50 }, .{ 30, 50 } };

    /// The terrain is not generated yet: every other province counts as
    /// water.
    fn buildingPlaces(self: *Gen, pv: *Province) void {
        const s = self.s;
        var score: [10]i32 = @splat(0);
        for (bld_offsets, 0..) |o, k| {
            const bx = pv.x + o[0];
            const by = pv.y + o[1];
            if (s.lookup(bx, by) == pv.number) score[k] += 15;
            var go: i32 = 10;
            while (go < 40) : (go += 10) {
                const probes = [8][2]i32{ .{ go, 0 }, .{ go, -go }, .{ go, go }, .{ 0, -go }, .{ 0, go }, .{ -go, -go }, .{ -go, 0 }, .{ -go, go } };
                for (probes) |d| {
                    const pp = s.lookup(bx + d[0], by + d[1]);
                    if (pp == pv.number) {
                        score[k] += @divTrunc(30, go);
                    } else if (pp == 0 or s.terrainOf(pp) == terrain.water) score[k] -= @divTrunc(30, go);
                }
            }
        }
        for (&pv.bld_place) |*place| {
            var best: usize = 0;
            var max: i32 = -100;
            for (score, 0..) |v, k| if (v > max) {
                max = v;
                best = k;
            };
            place.* = .{ pv.x + bld_offsets[best][0], pv.y + bld_offsets[best][1] };
            score[best] = -1000;
        }
    }

    const Flood = struct { suit: i32, to: i32, max: i32, deep: i32 };

    /// Flooder: turns `p` and, recursively, neighbours that pass
    /// Random(deep) < poss from `suit` into `to`.  Random returns an
    /// unsigned long, so a negative `poss` always passes.
    fn flood(self: *Gen, f: Flood, p: usize, poss: i32, step_in: i32) i32 {
        var step = step_in;
        const pv = &self.s.provinces[p];
        if (pv.terrain == f.suit) {
            pv.terrain = f.to;
            step += 1;
        }
        if (step >= f.max) return step;
        for (pv.neighbours) |nb| {
            if (nb <= 0) continue;
            if (self.r.random(@intCast(@max(f.deep, 0))) >= @as(u32, @bitCast(poss))) continue;
            if (self.s.terrainOf(nb) != f.suit) continue;
            step = self.flood(f, @intCast(nb), poss - 1, step);
            if (step >= f.max) return step;
        }
        return step;
    }

    fn isWater(t: i32) bool {
        return t <= 0;
    }
    fn notWater(t: i32) bool {
        return t != 0;
    }
    fn isPlain(t: i32) bool {
        return t == terrain.plain;
    }

    fn landscape(self: *Gen) !void {
        const s = self.s;
        const m = &self.m;
        const size = self.n();
        for (s.provinces[1..]) |*pv| pv.terrain = terrain.plain;
        m.water = @divTrunc(size * m.water, 100);
        const dissolve = @max(m.dissolve, 1);
        var water: i32 = 0;
        switch (m.type) {
            1 => {
                const X: usize = @intCast(s.x);
                const N = s.count();
                for (1..X + 1) |i| {
                    s.provinces[i].terrain = terrain.water;
                    s.provinces[1 + N - i].terrain = terrain.water;
                }
                for (0..@intCast(s.y)) |i| {
                    s.provinces[1 + i * X].terrain = terrain.water;
                    s.provinces[(i + 1) * X].terrain = terrain.water;
                }
                water = s.x * 2 + (s.y - 1) * 2;
                const deep = @divTrunc(m.water, 10) + 5;
                while (water < m.water) {
                    const p = try self.pick(isWater);
                    water += self.flood(.{ .suit = 1, .to = 0, .max = m.water - water, .deep = deep }, p, @divTrunc(deep * 15, dissolve), 0);
                }
            },
            2 => {
                const deep = @divTrunc(m.water, 10) + 5;
                while (water < m.water) {
                    const p: usize = @intCast(below(self.r, size) + 1);
                    water += self.flood(.{ .suit = 1, .to = 0, .max = m.water - water, .deep = deep }, p, @divTrunc(deep * 15, dissolve), 0);
                }
            },
            else => {},
        }
        m.isles = @divTrunc(m.isles * m.water, 100);
        while (m.isles > 0) : (m.isles -= 1) {
            s.provinces[try self.pick(isWater)].terrain = terrain.plain;
            s.provinces[try self.pick(notWater)].terrain = terrain.water;
        }

        m.hill = @divTrunc((size - water) * m.hill, 100);
        const mountain = @divTrunc(m.hill, 5);
        m.hill -= mountain;
        var hills: i32 = 0;
        var mountains: i32 = 0;
        while (hills < m.hill or mountains < mountain) {
            const p = try self.pick(isPlain);
            if (mountains < mountain)
                mountains += self.flood(.{ .suit = 1, .to = terrain.mountain, .max = mountain - mountains, .deep = 6 + @divTrunc(mountain, 10) }, p, 1, 0);
            if (hills < m.hill) {
                const deep = @divTrunc(m.hill, 10) + 6;
                hills += self.flood(.{ .suit = 1, .to = terrain.hill, .max = m.hill - hills, .deep = deep }, p, @divTrunc(deep * 10, dissolve), 0);
            }
        }

        m.forest = @divTrunc((size - water) * m.forest, 100);
        if (m.forest + hills + water > size) m.forest = 0;
        var forests: i32 = 0;
        {
            const deep = @divTrunc(m.forest, 10) + 6;
            while (forests < m.forest) {
                const p = try self.pick(isPlain);
                forests += self.flood(.{ .suit = 1, .to = terrain.forest, .max = m.forest - forests, .deep = deep }, p, @divTrunc(deep * 10, dissolve), 0);
            }
        }

        m.swamp = @divTrunc((size - water) * m.swamp, 100);
        if (m.swamp + forests + hills + water > size) m.swamp = @divTrunc(size - (forests + hills + water), 2);
        var swamps: i32 = 0;
        {
            const deep = @divTrunc(m.swamp, 10) + 6;
            while (swamps < m.swamp) {
                const p = try self.pick(isPlain);
                swamps += self.flood(.{ .suit = 1, .to = terrain.swamp, .max = m.swamp - swamps, .deep = deep }, p, @divTrunc(deep * 10, dissolve), 0);
            }
        }
        self.placed = .{ .water = water, .hill = hills, .forest = forests, .swamp = swamps, .mountain = mountains };
    }

    /// MakeMap: steps from `p` over land; water and mountains stop it.
    fn makeMap(self: *Gen, p: usize, weight_in: i32) void {
        const weight = weight_in + 1;
        const d = self.s.dist;
        if (d[p] < 0 or d[p] > weight) d[p] = weight;
        for (self.s.provinces[p].neighbours) |nb| {
            if (nb <= 0) continue;
            const u: usize = @intCast(nb);
            const t = self.s.terrainOf(u);
            if (t == terrain.water or t == terrain.mountain) continue;
            if (d[u] >= 0 and d[u] <= weight + 1) continue;
            self.makeMap(u, weight);
        }
    }

    fn clearMoveMap(self: *Gen) void {
        @memset(self.s.dist, -1);
    }

    /// Land neighbours of `p`, and whether one of them has more than three.
    fn landAround(self: *const Gen, p: usize) struct { count: i32, big: bool } {
        var c: i32 = 0;
        var big = false;
        for (self.s.provinces[p].neighbours) |nb| {
            if (nb <= 0 or !terrain.land(self.s.terrainOf(nb))) continue;
            var m: i32 = 0;
            for (self.s.provinces[@intCast(nb)].neighbours) |q| {
                if (q > 0 and terrain.land(self.s.terrainOf(q))) m += 1;
            }
            if (m > 3) big = true;
            c += 1;
        }
        return .{ .count = c, .big = big };
    }

    /// The castles: a start needs `min_neigh` land neighbours, one of them
    /// with more than three, and `min_dist` steps to the previous castle;
    /// the rules relax until every player fits.  iFort and Work of the
    /// original are `fort` and `work` here.
    fn castles(self: *Gen) !void {
        const s = self.s;
        const gpa = s.gpa;
        const N = s.count();
        const players = self.p.players.len;
        const fort = try gpa.alloc(i32, N + 1);
        defer gpa.free(fort);
        // long Work: 32 bits in MSVC, the products wrap.
        const work = try gpa.alloc(i32, N + 1);
        defer gpa.free(work);
        var min_neigh: i32 = 6;
        var min_dist: i32 = @divTrunc(s.x + s.y, 2) + 1 - @as(i32, @intCast(players / 2));
        var cycles: usize = 0;
        while (true) : (cycles += 1) {
            if (cycles > 10_000) return error.GenerationStuck;
            for (s.provinces[1..]) |*pv| pv.warning = true;
            var repeater: i32 = 0;
            while (true) {
                var work_ok = true;
                var places: i32 = 0;
                var placed: usize = 0;
                var castle: [max_players + 1]i32 = @splat(0);
                for (1..N + 1) |i| {
                    fort[i] = 0;
                    if (!terrain.land(s.terrainOf(i))) continue;
                    const la = self.landAround(i);
                    if (la.count >= min_neigh and la.big) {
                        fort[i] = 1;
                        places += 1;
                    }
                }
                if (places < players) break;
                var try_p: usize = @intCast(below(self.r, @intCast(N)) + 1);
                var tries: usize = 0;
                while (fort[try_p] != 1 or !s.provinces[try_p].warning) : (tries += 1) {
                    if (tries > 1_000_000) return error.GenerationStuck;
                    try_p = @intCast(below(self.r, @intCast(N)) + 1);
                }
                castle[1] = @intCast(try_p);
                fort[try_p] = 2;
                s.provinces[try_p].warning = false;
                placed += 1;
                @memset(work, 1);
                for (2..players + 1) |pl| {
                    if (!work_ok) continue;
                    self.clearMoveMap();
                    self.makeMap(@intCast(castle[pl - 1]), 0);
                    for (1..N + 1) |j| {
                        if (fort[j] != 1) continue;
                        const d = s.dist[j];
                        if (d < 0) {
                            if (min_neigh > 3 and self.p.game_type != 2) {
                                work[j] *%= @divTrunc(s.x + s.y, 2);
                            } else {
                                work[j] = 0;
                                fort[j] = 0;
                            }
                        } else if (d < min_dist) {
                            work[j] = 0;
                            fort[j] = 0;
                        } else {
                            // The original multiplies by the distance of
                            // the previous province (pMoveMap + j - 1).
                            work[j] *%= s.dist[j - 1];
                        }
                    }
                    var found: usize = 0;
                    var max: i32 = 0;
                    for (1..N + 1) |i| if (fort[i] == 1 and work[i] > max) {
                        found = i;
                        max = work[i];
                    };
                    if (found > 0) {
                        fort[found] = 2;
                        castle[pl] = @intCast(found);
                        placed += 1;
                    } else work_ok = false;
                }
                if (placed >= players) {
                    s.capitals = castle;
                    for (s.provinces[1..], work[1..]) |*pv, w| pv.work = w;
                    return;
                }
                if (places < repeater + 3) break;
                repeater += 1;
            }
            if (min_neigh > 4 or (min_neigh > 3 and min_dist < 6)) {
                min_neigh -= 1;
            } else {
                min_neigh = 6;
                min_dist -= 1;
            }
        }
    }

    /// The castles' provinces: owner, type, names, income.  MakeMap from
    /// every castle without clearing in between gives the distances.
    fn capitalProvinces(self: *Gen) !void {
        const s = self.s;
        for (s.provinces[1..]) |*pv| {
            pv.warning = true;
            pv.fort = 0;
        }
        self.clearMoveMap();
        const a = s.arena.allocator();
        for (self.p.players, 1..) |pl, i| {
            const c: usize = @intCast(s.capitals[i]);
            const pv = &s.provinces[c];
            self.makeMap(c, 0);
            pv.player = @intCast(i);
            pv.type = province.type_castle;
            try s.garrisons.append(s.gpa, hero_mod.Hero.garrison(self.pr, @intCast(i), @intCast(c)));
            pv.garrison = @intCast(s.garrisons.items.len - 1);
            pv.explore = 50;
            switch (pl.kind) {
                .human => pv.name = try a.dupe(u8, pl.land),
                .lord => self.castleName(pv),
                .computer => {
                    const r: usize = @intCast(std.math.clamp(pl.ruler, 0, @as(i32, @intCast(self.pr.player_profiles.len)) - 1));
                    pv.name = self.pr.player_profiles[r].land;
                    var w = self.world();
                    switch (pl.ruler) {
                        5 => pv.terrain = terrain.forest,
                        9 => _ = try w.place(pv, 35, 3, 0, -1, 0),
                        12 => {
                            pv.terrain = terrain.forest;
                            _ = try w.place(pv, 43, 3, 0, -1, 0);
                        },
                        13 => {
                            pv.terrain = terrain.swamp;
                            _ = try w.place(pv, 45, 0, 0, -1, 0);
                        },
                        14 => {
                            pv.terrain = terrain.hill;
                            _ = try w.place(pv, 44, 3, 0, -1, 0);
                        },
                        else => {},
                    }
                },
            }
            pv.race = 1;
            pv.population = 10;
            // iFort = arPlayer[i].iWall, 100 for a new player.
            pv.fort = 100;
            const inc: [3]i32 = switch (pv.terrain) {
                terrain.forest => .{ 10, 5, 45 },
                terrain.hill => .{ 12, 4, 50 },
                terrain.swamp => .{ 8, 6, 40 },
                else => .{ 15, 3, 60 },
            };
            pv.gold_inc = inc[0];
            pv.gem_inc = inc[1];
            pv.explore = inc[2];
            self.castles_on[
                switch (pv.terrain) {
                    terrain.forest, terrain.hill, terrain.swamp => @intCast(pv.terrain),
                    else => terrain.plain,
                }
            ] += 1;
            if (pl.kind == .human) {
                const d = self.pr.difficulties[@intCast(std.math.clamp(pl.difficulty, 0, proto_mod.max_difficulty - 1))];
                pv.gold_inc += d.gold_add;
                pv.gem_inc += d.gem_add;
                pv.population += d.population;
            }
            var nei: i32 = 0;
            for (pv.neighbours) |nb| {
                if (nb > 0 and terrain.land(s.terrainOf(nb))) nei += 1;
            }
            if (nei < 3) for (pv.neighbours) |nb| {
                if (nb <= 0) continue;
                const u = s.at(nb);
                if (nei < 3 and u.terrain == terrain.water) {
                    u.terrain = terrain.plain;
                    s.dist[@intCast(nb)] = 2;
                    nei += 1;
                }
                if (nei < 3 and u.terrain == terrain.mountain) {
                    u.terrain = terrain.hill;
                    s.dist[@intCast(nb)] = 2;
                    nei += 1;
                }
            };
        }
        for (s.dist[1..]) |*d| if (d.* < 0) {
            d.* = 8;
        };
    }

    /// The simple resource of a terrain: plain 3, forest 2, hill 1,
    /// swamp 4.
    fn terrainResource(t: i32) i32 {
        return switch (t) {
            terrain.plain => 3,
            terrain.forest => 2,
            terrain.hill => 1,
            else => 4,
        };
    }

    fn placeResource(self: *Gen, p: usize, t: i32) void {
        const r = terrainResource(t);
        const site = &self.s.provinces[p].sites[0];
        site.* = .{ .index = r, .pos = 1 };
        self.s.res_count[@intCast(r)] += 1;
    }

    /// Up to two simple resources next to each castle, and a second land
    /// type where the castle has only one around it.
    fn castleResources(self: *Gen) void {
        const s = self.s;
        for (1..self.p.players.len + 1) |pl| {
            const home: i32 = s.capitals[pl];
            var near: [5]bool = @splat(false);
            var ring: [5]bool = @splat(false);
            var neigh: [110]usize = undefined;
            var nei: usize = 0;
            for (s.at(home).neighbours) |nb| {
                if (nb <= 0 or !terrain.land(s.terrainOf(nb))) continue;
                neigh[nei] = @intCast(nb);
                near[@intCast(s.terrainOf(nb))] = true;
                nei += 1;
            }
            var nei1 = nei;
            for (neigh[0..nei]) |p| for (s.provinces[p].neighbours) |nb| {
                if (nb <= 0 or nb == home) continue;
                const u: usize = @intCast(nb);
                if (s.provinces[u].sites[0].index != 0 or !terrain.land(s.terrainOf(u))) continue;
                if (std.mem.indexOfScalar(usize, neigh[0..nei1], u) != null) continue;
                neigh[nei1] = u;
                ring[@intCast(s.terrainOf(u))] = true;
                nei1 += 1;
            };
            var count: usize = 0;
            var land: i32 = 0;
            for (1..5) |k| if (near[k] or ring[k]) {
                count += 1;
                land = @intCast(k);
            };
            if (count < 2) {
                var morph = below(self.r, 4) + 1;
                while (morph == land) morph = below(self.r, 4) + 1;
                const k: usize = @intCast(below(self.r, @as(i32, @intCast(nei1)) - 1));
                s.provinces[neigh[k]].terrain = morph;
                if (k >= nei) ring[@intCast(morph)] = true else near[@intCast(morph)] = true;
            }
            count = 0;
            land = 0;
            for (1..5) |k| if (ring[k]) {
                count += 1;
                land = @intCast(k);
            };
            const outer: i32 = @intCast(nei1 - nei);
            if (count > 1) {
                var placed: usize = 0;
                land = 0;
                while (placed < 2) {
                    const t = below(self.r, 4) + 1;
                    if (!ring[@intCast(t)] or t == land) continue;
                    placed += 1;
                    land = t;
                    var k: usize = nei + @as(usize, @intCast(below(self.r, outer)));
                    while (s.terrainOf(neigh[k]) != land) k = nei + @as(usize, @intCast(below(self.r, outer)));
                    self.placeResource(neigh[k], land);
                }
            } else if (land > 0) {
                var k: usize = nei + @as(usize, @intCast(below(self.r, outer)));
                while (s.terrainOf(neigh[k]) != land) k = nei + @as(usize, @intCast(below(self.r, outer)));
                self.placeResource(neigh[k], land);
                k = @intCast(below(self.r, @intCast(nei)));
                while (s.terrainOf(neigh[k]) == land or !near[@intCast(s.terrainOf(neigh[k]))]) k = @intCast(below(self.r, @intCast(nei)));
                const p = neigh[k];
                if (s.dist[p] == 2) s.dist[p] += 1;
                self.placeResource(p, s.terrainOf(p));
            }
        }
    }
    fn world(self: *Gen) site_mod.World {
        return .{
            .gpa = self.s.gpa,
            .pr = self.pr,
            .r = self.r,
            .guards = &self.s.guards,
            .shops = &self.s.shops,
            .neutrals = self.m.neutrals,
        };
    }

    /// "Остальные ресурсы" (Mapper.cpp:1979): each resource is dropped on
    /// free provinces of its terrain until MaxDrop; rare ones near castles
    /// get a site guard.
    fn restResources(self: *Gen) !void {
        const s = self.s;
        const pl = self.placed;
        const size = self.n();
        const on = self.castles_on;
        const res = &s.res_count;
        for (1..self.pr.resources.len) |i| {
            const rarity = self.pr.resources[i].rarity;
            var max_drop = 1 + below(self.r, 2);
            var land: i32 = 0;
            var free: i32 = 0;
            switch (i) {
                1, 6, 7 => {
                    max_drop += @divTrunc(@divTrunc(pl.hill, 7), rarity + 1);
                    land = terrain.hill;
                    free = pl.hill - res[1] - res[6] - res[7] - on[terrain.hill];
                },
                2, 8 => {
                    max_drop += @divTrunc(@divTrunc(pl.forest, 7), rarity + 1);
                    land = terrain.forest;
                    free = pl.forest - res[2] - res[8] - on[terrain.forest];
                },
                4, 5, 9 => {
                    max_drop += @divTrunc(@divTrunc(pl.swamp, 7), rarity + 1);
                    land = terrain.swamp;
                    free = pl.swamp - res[4] - res[5] - res[9] - on[terrain.swamp];
                },
                3 => {
                    const plain = size - pl.hill - pl.forest - pl.swamp - pl.mountain - pl.water;
                    max_drop += @divTrunc(plain, 15);
                    land = terrain.plain;
                    free = plain - res[3] - on[terrain.plain];
                },
                else => {},
            }
            max_drop += @divTrunc(self.m.resource, 3);
            max_drop += @divTrunc(max_drop * (self.m.resource - 2), 5);
            while (res[i] < max_drop and free > 2) {
                var t: usize = @intCast(below(self.r, size) + 1);
                while (s.provinces[t].terrain != land or s.provinces[t].sites[0].index > 0 or s.provinces[t].type > 0)
                    t = @intCast(below(self.r, size) + 1);
                const pv = &s.provinces[t];
                const calc = below(self.r, 20) + 40 + rarity * 30 - s.dist[t] * 10;
                pv.sites[0] = .{ .index = @intCast(i), .pos = std.math.clamp(calc, 0, 100) };
                if (rarity * 2 + 3 > s.dist[t]) {
                    const site = &self.pr.sites[i];
                    var rl: unit_mod.Roller = .{};
                    var k: usize = 0;
                    for (site.guards) |gp| {
                        if (gp.id <= 0) continue;
                        rl.ids[k] = gp.id;
                        rl.weights[k] = gp.value;
                        k += 1;
                    }
                    const gt = rl.roll(self.r);
                    pv.sites[0].guard = gt;
                    const lvl: usize = @intCast(std.math.clamp(rarity, 0, 3));
                    var w = self.world();
                    try w.placeSiteGuard(pv, 0, self.pr.guard_types[@intCast(gt)].guards[lvl]);
                }
                res[i] += 1;
                free -= 1;
            }
        }
    }

    /// Province names of the water and the mountains (phrases 324..327).
    fn waterNames(self: *Gen) void {
        const s = self.s;
        s.provinces[0].name = self.pr.phrase(327);
        for (s.provinces[1..]) |*pv| {
            if (pv.terrain == terrain.water) {
                const lake = for (pv.neighbours) |nb| {
                    if (nb > 0 and s.terrainOf(nb) == terrain.water) break false;
                } else true;
                pv.coastal = if (lake) self.p.lake_ships else true;
                pv.name = self.pr.phrase(if (lake) 325 else 324);
            }
            if (pv.terrain == terrain.mountain) pv.name = self.pr.phrase(326);
        }
    }

    /// The province type: Roll is called after every candidate, so the
    /// last call decides (Mapper.cpp:2151, GOG the same).
    fn provinceType(self: *Gen, pv: *const Province, level: i32) i32 {
        var rl: unit_mod.Roller = .{};
        var cnt: usize = 0;
        var result: i32 = 0;
        for (self.pr.province_types[1..], 1..) |pt, j| {
            const wgt = pt.terrainWeight(@intCast(pv.terrain));
            if (wgt > 0 and pt.difficult < level - 1) {
                rl.ids[cnt] = @intCast(j);
                rl.weights[cnt] = wgt;
                cnt += 1;
            }
            result = rl.roll(self.r);
        }
        return result;
    }

    fn provinceName(self: *Gen, pv: *Province, used: []bool) !void {
        const s = self.s;
        var rl: unit_mod.Roller = .{};
        var cnt: usize = 0;
        const isle = for (pv.neighbours) |nb| {
            if (nb > 0 and s.terrainOf(nb) != terrain.water) break false;
        } else true;
        const names = self.pr.province_names;
        if (isle) for (names[1..], 1..) |pn, i| {
            if (used[i] or !pn.checkType(pv.type) or !pn.checkTerrain(pv.terrain) or pn.coast != 2) continue;
            rl.ids[cnt] = @intCast(i);
            rl.weights[cnt] = pn.rarity;
            cnt += 1;
        };
        if (cnt == 0) for (names[1..], 1..) |pn, i| {
            if (used[i] or !pn.checkType(pv.type) or !pn.checkTerrain(pv.terrain)) continue;
            if (pn.coast != 0 and !(pn.coast == 1 and pv.coastal)) continue;
            rl.ids[cnt] = @intCast(i);
            rl.weights[cnt] = pn.rarity;
            cnt += 1;
        };
        if (cnt > 0) {
            const k: usize = @intCast(rl.roll(self.r));
            pv.name = names[k].name;
            used[k] = true;
        } else {
            const pre = self.pr.prefixes[@intCast(below(self.r, @intCast(self.pr.prefixes.len)))];
            const suf = self.pr.suffixes[@intCast(below(self.r, @intCast(self.pr.suffixes.len)))];
            pv.name = try std.mem.concat(s.arena.allocator(), u8, &.{ pre, suf });
        }
    }

    /// The neutral guard of a province (Mapper.cpp:2233).  GOG subtracts
    /// the first Random(Level-2) and adds the second.
    fn provinceGuard(self: *Gen, pv: *Province, level: i32) !void {
        const pt = &self.pr.province_types[@intCast(pv.type)];
        var rl: unit_mod.Roller = .{};
        for (pt.guards, 0..) |g, k| {
            if (g.guard_type <= 0) break;
            rl.ids[k] = @intCast(k + 1);
            rl.weights[k] = g.possibility;
        }
        const grd: usize = @intCast(rl.roll(self.r) - 1);
        pv.defender = @intCast(grd);
        var power: i32 = 15;
        if (level >= 0) {
            const r1 = below(self.r, level - 2);
            const r2 = below(self.r, level - 2);
            power = level * 3 - 5 - r1 + r2;
        }
        power = @min(power, self.m.neutrals);
        const gt = &self.pr.guard_types[@intCast(pt.guards[grd].guard_type)];
        var lvl: usize = 0;
        for (1..4) |k| {
            const cur = self.pr.guards[@intCast(gt.guards[lvl])].power;
            if (@abs(self.pr.guards[@intCast(gt.guards[k])].power - power) <= @abs(cur - power)) lvl = k;
        }
        try self.s.guards.append(self.s.gpa, guard_mod.place(self.pr, self.r, gt.guards[lvl], gt.karma));
        pv.guard = @intCast(self.s.guards.items.len - 1);
    }

    /// The village roll of the tutorial map (Mapper.cpp:1159): a free name
    /// that lists `kind` among its types and fits type and terrain.  The
    /// provinces are not coastal yet at this point.
    fn tutorialName(self: *Gen, pv: *Province, kind: i32) void {
        var rl: unit_mod.Roller = .{};
        var cnt: usize = 0;
        const names = self.pr.province_names;
        for (names[1..], 1..) |*pn, i| {
            if (self.used_names[i] or !pn.checkType(pv.type) or !pn.checkTerrain(pv.terrain)) continue;
            if (std.mem.indexOfScalar(i32, &pn.types, kind) == null) continue;
            if (pn.coast != 0 and !(pn.coast == 1 and pv.coastal)) continue;
            rl.ids[cnt] = @intCast(i);
            rl.weights[cnt] = pn.rarity;
            cnt += 1;
        }
        const k: usize = @intCast(rl.roll(self.r));
        pv.name = names[k].name;
        self.used_names[k] = true;
    }

    /// Tutorial.Map (Mapper.cpp:1031..1284): the enemy castle 7, the
    /// player's castle 23, the fixed terrains, the villages 26 and 27, the
    /// shrines of 27 and the town 21.  The rest is populated as usual.
    fn tutorialMap(self: *Gen) !void {
        const s = self.s;
        const pr = self.pr;
        const r = self.r;
        for (s.dist[1..]) |*d| d.* = 2;

        // The enemy castle.
        const e = s.at(7);
        e.terrain = terrain.plain;
        e.type = province.type_castle;
        e.explore = 50;
        e.player = 2;
        e.race = 1;
        e.population = 10;
        var eg = hero_mod.Hero.garrison(pr, 2, 7);
        e.gem_inc = 5;
        e.gold_inc = 20;
        s.capitals[2] = 7;
        s.rulers[2].wall = 50;
        e.fort = 50;
        try s.guards.append(s.gpa, guard_mod.placeDefender(pr, r, 6, 2, 0));
        e.guard = @intCast(s.guards.items.len - 1);
        e.defender = 6;
        // Build(39), the Fort: no rand(); garrison slot 2 holds unit 4,
        // the fort and the wall +50, one tower.
        e.fort += 50;
        s.rulers[2].wall += 50;
        for (0..6) |i| eg.squad[i] = Unit.init(pr, r, 4, 30);
        try s.garrisons.append(s.gpa, eg);
        e.garrison = @intCast(s.garrisons.items.len - 1);
        self.castleName(e);

        // The player's castle.
        const h = s.at(23);
        h.terrain = terrain.plain;
        h.type = province.type_castle;
        h.explore = 50;
        h.player = 0;
        h.race = 1;
        h.population = 10;
        try s.garrisons.append(s.gpa, hero_mod.Hero.garrison(pr, 1, 23));
        h.garrison = @intCast(s.garrisons.items.len - 1);
        h.fort = 100;
        h.gem_inc = 5;
        h.gold_inc = 20;
        s.capitals[1] = 23;
        if (self.p.tutorial_land) |land| {
            h.name = try s.arena.allocator().dupe(u8, land);
        } else self.castleName(h);

        const W = terrain.water;
        const M = terrain.mountain;
        const F = terrain.forest;
        const P = terrain.plain;
        const H = terrain.hill;
        const S = terrain.swamp;
        const fixed = [_]struct { usize, i32 }{
            .{ 1, M },  .{ 2, M },  .{ 3, W },  .{ 4, M },  .{ 5, F },  .{ 6, F },
            .{ 8, H },  .{ 9, W },  .{ 10, F }, .{ 11, P }, .{ 12, P }, .{ 13, H },
            .{ 14, H }, .{ 15, M }, .{ 16, M }, .{ 17, W }, .{ 18, P }, .{ 19, M },
            .{ 20, M }, .{ 21, F }, .{ 22, W }, .{ 24, F }, .{ 25, M }, .{ 26, P },
            .{ 27, P }, .{ 28, W }, .{ 29, W }, .{ 30, S }, .{ 31, W }, .{ 32, W },
            .{ 33, W }, .{ 34, W }, .{ 35, S }, .{ 36, F },
        };
        for (fixed) |f| s.at(f[0]).terrain = f[1];
        s.at(13).player = 2;
        s.at(14).player = 2;
        s.at(8).player = 2;
        s.at(8).buildings[0] = 10;

        // The villages 26 and 27.
        for ([_]usize{ 26, 27 }) |pp| {
            const pv = s.at(pp);
            pv.type = 1;
            pv.race = 1;
            pv.population = 300;
            pv.explore = 1;
            self.tutorialName(pv, 1);
            const tr = &pr.terrains[@intCast(pv.terrain)];
            const pt = &pr.province_types[@intCast(pv.type)];
            pv.gold_inc = @divTrunc((tr.gold + pt.gold) * pv.size, s.med_size);
            pv.gold_inc += 2 - below(r, 5);
            if (pv.gold_inc <= 0) pv.gold_inc = 1;
            pv.gem_inc = @divTrunc((tr.gem + pt.gem) * pv.size, s.med_size);
            pv.gem_inc = @max(0, pv.gem_inc - below(r, 4));
        }

        // The shrines of 27: their guards are replaced by fixed squads.
        const v = s.at(27);
        var vg = hero_mod.Hero.garrison(pr, 1, 27);
        vg.squad = @splat(null);
        try s.garrisons.append(s.gpa, vg);
        v.garrison = @intCast(s.garrisons.items.len - 1);
        v.population = 10000;
        v.buildings[0] = 7;
        v.buildings[1] = 18;
        v.buildings[2] = 19;
        var w = self.world();
        const shrines = [_]struct { i32, i32 }{ .{ 11, 4 }, .{ 12, 6 }, .{ 10, 5 } };
        for (shrines) |sh| {
            const slot = (try w.place(v, sh[0], 1, 0, 0, 0)) orelse return error.TutorialShrine;
            v.sites[slot].guard = sh[1];
        }
        v.fort = 100;
        for (0..3) |k| {
            if (v.site_guards[k] < 0) {
                try s.guards.append(s.gpa, .{ .guard = 0, .initiative = 0 });
                v.site_guards[k] = @intCast(s.guards.items.len - 1);
            }
            const g = &s.guards.items[@intCast(v.site_guards[k])];
            g.initiative = 0;
            for (&g.squad) |*u| if (u.* != null) {
                u.* = null;
                v.sites[k].pos = @intCast(k + 1);
            };
        }
        const squads = [3][]const struct { usize, i32 }{
            &.{ .{ 0, 13 }, .{ 1, 13 }, .{ 2, 13 } },
            &.{ .{ 0, 14 }, .{ 1, 14 }, .{ 2, 14 } },
            &.{ .{ 0, 19 }, .{ 2, 20 }, .{ 1, 19 }, .{ 3, 20 } },
        };
        for (squads, 0..) |list, k| {
            const g = &s.guards.items[@intCast(v.site_guards[k])];
            for (list) |e2| g.squad[e2[0]] = Unit.init(pr, r, e2[1], 0);
        }

        // The town 21.
        const t = s.at(21);
        t.type = 2;
        t.race = 1;
        t.population = 100;
        t.explore = 30;
        self.tutorialName(t, 2);
        t.gold_inc = 8;
        t.gem_inc = 0;
        try s.guards.append(s.gpa, guard_mod.place(pr, r, 5, 0));
        t.guard = @intCast(s.guards.items.len - 1);
        t.buildings[0] = 10;
    }

    /// Mapper.cpp:2065..2362: water names, then every land province:
    /// type, population, exploration, name, guard, income (not for the
    /// castles), quest and ordinary sites.
    fn populate(self: *Gen) !void {
        const s = self.s;
        const pr = self.pr;
        const quests = try s.gpa.alloc(bool, pr.quests.len);
        defer s.gpa.free(quests);
        for (pr.quests, quests) |q, *f| f.* = q.giver > 0 and q.kind != 7 and q.kind != 8;
        const used = self.used_names;

        self.waterNames();
        const seed: u32 = if (self.p.seed == 0) 111 else self.p.seed;
        const max_quest: i32 = @intCast(pr.quests.len - 1);
        for (s.provinces[1..], 1..) |*pv, i| {
            if (!terrain.land(pv.terrain)) continue;
            const level = s.dist[i];
            pv.coastal = for (pv.neighbours) |nb| {
                if (nb > 0 and s.terrainOf(nb) == terrain.water and s.at(nb).coastal) break true;
            } else false;
            if (pv.type == 0) {
                pv.type = self.provinceType(pv, level);
                const pt = &pr.province_types[@intCast(pv.type)];
                const tr = &pr.terrains[@intCast(pv.terrain)];
                const level1 = std.math.clamp(level - pt.difficult, 0, 3);
                pv.race = pt.race;
                pv.population = pt.population + level1 * pt.level_pop;
                if (pv.population > 0) {
                    pv.population = @divTrunc(pv.population * tr.population_mod, 100);
                    pv.population += below(self.r, @divTrunc(pv.population, 5));
                    pv.population += @divTrunc(pv.population * (self.m.population - 2), 5);
                }
                pv.explore = pt.explored + level1 * pt.level_explored + below(self.r, 5);
                pv.explore = @divTrunc(pv.explore * tr.explore_mod, 100);
                pv.explore += @divTrunc(pv.explore * (self.m.population - 2), 5);
                try self.provinceName(pv, used);
                try self.provinceGuard(pv, level);
                pv.gold_inc = @divTrunc((tr.gold + pt.gold) * pv.size, s.med_size);
                pv.gold_inc += 2 - below(self.r, 5);
                if (pv.gold_inc <= 0) pv.gold_inc = 1;
                pv.gem_inc = @divTrunc((tr.gem + pt.gem) * pv.size, s.med_size);
                pv.gem_inc = @max(0, pv.gem_inc - below(self.r, 4));
            }

            var w = self.world();
            const area = s.settings.x * s.settings.y;
            const possib = std.math.clamp(100 - @divTrunc((max_quest - 13) * 150, area), 50, 98);
            if (rand_mod.keyed(seed, 100, @bitCast(seed), @intCast(i)) > @as(u32, @intCast(possib))) {
                var rl: unit_mod.Roller = .{};
                var cnt: usize = 0;
                for (quests, 0..) |f, q| if (f and q > 0) {
                    rl.ids[cnt] = @intCast(q);
                    rl.weights[cnt] = pr.quests[q].possibility;
                    cnt += 1;
                };
                if (cnt > 0) {
                    const q: usize = @intCast(rl.roll(self.r));
                    quests[q] = false;
                    _ = try w.place(pv, pr.quests[q].giver, level - 2, @intCast(q), 0, 0);
                }
            }

            var sites = pr.province_types[@intCast(pv.type)].sites + below(self.r, 11) - 5;
            sites += @divTrunc(sites * (self.m.site - 1), 10);
            sites = @min(sites, @min(province.max_site - 3, 99 - pv.explore));
            for (pr.sites[1..], 1..) |st, k| for (st.abilities) |a| {
                if (a.id == 5 and a.p1 == pv.type) _ = try w.place(pv, @intCast(k), level - 2, 0, 0, 0);
            };
            var k: i32 = 0;
            while (k < sites) : (k += 1) {
                var rl: unit_mod.Roller = .{};
                var cnt: usize = 0;
                for (pr.sites[1..], 1..) |st, j| {
                    const wgt = st.terrain[@intCast(pv.terrain - 1)];
                    if (wgt != 0 and st.place > 0) {
                        rl.ids[cnt] = @intCast(j);
                        rl.weights[cnt] = wgt;
                        cnt += 1;
                    }
                }
                _ = try w.place(pv, rl.roll(self.r), level - 2, 0, 0, 0);
            }
        }
    }

    /// Campaign quests (Mapper.cpp:2364, GOG 0046eae0:2784): each quest
    /// takes the land province whose MakeMap distance from player 1's
    /// capital is nearest to Quantity[0]; a tie goes to the later one with
    /// rand() % 10 < 5.  The province takes the quest's name and a guarded
    /// site Target[0] in its first free slot.
    fn campaignQuests(self: *Gen) !void {
        if (self.p.game_type != 2) return;
        const s = self.s;
        self.clearMoveMap();
        self.makeMap(@intCast(s.capitals[1]), 0);
        var w = self.world();
        for (self.p.campaign_quests) |*cq| {
            if (cq.quest <= 0 or cq.state >= 2) continue;
            const q = &self.pr.quests[@intCast(cq.quest)];
            const task = q.tasks[0];
            if (task.place == 0) continue;
            var best: i32 = 100;
            var chosen: usize = 0;
            for (s.provinces[1..], 1..) |*pv, i| {
                const d = s.dist[i];
                if (pv.terrain == terrain.water or pv.terrain == terrain.mountain or pv.type == 7) continue;
                if (d <= 0 or pv.sites[province.max_site - 1].index != 0) continue;
                if (task.place >= 0 and pv.terrain != task.place) continue;
                const diff: i32 = @intCast(@abs(d - task.quantity));
                if (diff < best or (diff == best and self.r.rand() % 10 < 5)) {
                    best = diff;
                    chosen = i;
                }
            }
            if (chosen == 0) {
                cq.state = -1;
                continue;
            }
            const pv = &s.provinces[chosen];
            pv.name = q.name;
            for (&pv.sites, 0..) |*st, k| if (st.index == 0) {
                st.index = task.target;
                const gt = &self.pr.guard_types[@intCast(task.index)];
                try w.placeSiteGuard(pv, k, gt.guards[@intCast(q.exp)]);
                st.pos = 0;
                st.guard = task.index;
                cq.site = @intCast(k);
                break;
            };
            cq.province = @intCast(chosen);
        }
    }
};

/// GenerateMap without the picture.  `rand` is
/// reseeded with the game seed (0 becomes 111) and left where the
/// generator stopped.
pub fn generate(gpa: std.mem.Allocator, pr: *const Proto, rand: *Rand, p: Params) !Shard {
    const m = p.map;
    const y = m.y - @rem(m.y, 2);
    if (m.x < 1 or m.x > 99 or y < 2 or p.players.len < 1 or p.players.len > max_players) return error.BadSettings;
    const N: usize = @intCast(m.x * y);
    var s: Shard = .{
        .gpa = gpa,
        .arena = .init(gpa),
        .settings = m,
        .x = m.x,
        .y = y,
        .provinces = undefined,
        .grid = try Grid.init(gpa, @intCast((y + 2) * map_h)),
        .dist = undefined,
    };
    errdefer s.arena.deinit();
    errdefer s.grid.deinit(gpa);
    s.provinces = try gpa.alloc(Province, N + 1);
    errdefer gpa.free(s.provinces);
    // The GOG constructor zeroes the record: terrain 0 until the landscape.
    @memset(s.provinces, .{});
    s.dist = try gpa.alloc(i32, N + 1);
    errdefer gpa.free(s.dist);
    @memset(s.dist, -1);

    s.n_players = p.players.len;
    for (s.in_game[1 .. p.players.len + 1]) |*f| f.* = true;
    // arPlayer as Init and GenerateMap leave them (newsave.players).
    for (p.players, 1..) |pl, k| {
        const r = &s.rulers[k];
        r.kind = pl.kind;
        r.difficulty = pl.difficulty;
        r.profile = pl.ruler;
        r.color = @intCast(k - 1);
        if (pl.kind == .human) {
            const d = pr.difficulties[@intCast(std.math.clamp(pl.difficulty, 0, proto_mod.max_difficulty - 1))];
            r.gold = d.gold;
            r.gem = d.gem;
        } else {
            r.karma = pr.player_profiles[@intCast(pl.ruler)].karma;
            r.gold = (pl.difficulty + 10) * 50;
            r.gem = pl.difficulty * 5 + 20;
        }
    }
    rand.seed(if (p.seed == 0) 111 else p.seed);
    const used_names = try gpa.alloc(bool, pr.province_names.len);
    defer gpa.free(used_names);
    @memset(used_names, false);
    var g: Gen = .{ .s = &s, .r = rand, .pr = pr, .p = p, .m = m, .used_names = used_names };
    g.m.y = y;
    g.raceQuests();
    try g.grid();
    g.shapes();
    if (p.tutorial) {
        try g.tutorialMap();
    } else {
        try g.landscape();
        try g.castles();
        try g.capitalProvinces();
        g.castleResources();
        try g.restResources();
    }
    try g.populate();
    try g.campaignQuests();
    // Mapper.cpp:2795: the sprite pass moves the first building place of
    // each mountain below its peak — the furthest point still inside the
    // province, in steps of 5 from y+40 down to y.
    for (s.provinces[1..]) |*pv| {
        if (pv.terrain != terrain.mountain) continue;
        var dy: i32 = 40;
        while (dy >= 0) : (dy -= 5) {
            if (s.lookup(pv.x, pv.y + dy) == pv.number) {
                pv.bld_place[0][1] = pv.y + dy;
                break;
            }
        }
    }
    return s;
}

const tst = std.testing;
const GameFs = @import("../platform/fs.zig").GameFs;
const packfile = @import("../formats/packfile.zig");
const mapfile = @import("../formats/mapfile.zig");
const savefile = @import("../formats/savefile.zig");
const cp1251 = @import("../formats/cp1251.zig");

fn readTestFile(gpa: std.mem.Allocator, path: []const u8) ![]u8 {
    const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, gpa, .unlimited) catch return error.SkipZigTest;
    defer gpa.free(bytes);
    return packfile.unpack(gpa, bytes);
}

/// The generator against a map made by the original game: the seed,
/// settings and players come from the game file (header +0, +0x100,
/// player records), the expected provinces and border cells from the map.
fn checkAgainst(dir: []const u8, game: []const u8, map: []const u8) !void {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    var pb: [96]u8 = undefined;
    const gdata = try readTestFile(gpa, try std.fmt.bufPrint(&pb, "testdata/{s}/{s}", .{ dir, game }));
    defer gpa.free(gdata);
    const mdata = try readTestFile(gpa, try std.fmt.bufPrint(&pb, "testdata/{s}/{s}", .{ dir, map }));
    defer gpa.free(mdata);
    var ref = try mapfile.parse(gpa, mdata);
    defer ref.deinit(gpa);
    var save = try savefile.parse(gpa, gdata, ref.provinces.len);
    defer save.deinit(gpa);

    const hdr = gdata[3..][0..0x13c];
    var settings: profile.Map = undefined;
    inline for (@typeInfo(profile.Map).@"struct".field_names, 0..) |f, k|
        @field(settings, f) = savefile.int(hdr, 0x100 + 4 * k);
    const n_players: usize = @intCast(savefile.int(hdr, 0x38));
    var players: [max_players]Player = undefined;
    for (players[0..n_players], 1..) |*pl, k| {
        const rec = save.player(k);
        pl.* = .{ .kind = @fromBackingInt(@intCast(savefile.int(rec, 0))), .ruler = savefile.int(rec, 8), .difficulty = savefile.int(rec, 0xc) };
    }
    var rand: Rand = .{};
    var s = try generate(gpa, &pr, &rand, .{
        .seed = @bitCast(savefile.int(hdr, 0)),
        .map = settings,
        .game_type = savefile.int(hdr, 8),
        .players = players[0..n_players],
    });
    defer s.deinit();
    try tst.expectEqual(ref.provinces.len, s.count());
    try tst.expectEqual(@as(i32, @intCast(ref.image.w)), s.width());
    try tst.expectEqual(@as(i32, @intCast(ref.image.h)), s.height());
    for (ref.provinces, 1..) |*rp, i| {
        const pv = s.provinces[i];
        for (0..province.max_neighbour) |k| try tst.expectEqual(rp.int(0x11c + 4 * k), pv.neighbours[k]);
        try tst.expectEqual(rp.int(0x20), pv.size);
        try tst.expectEqual(rp.int(0x08), pv.x);
        try tst.expectEqual(rp.int(0x0c), pv.y);
        try tst.expectEqual(rp.int(0x18), pv.player);
        for (0..province.max_bld + 1) |k| {
            // A landmark on water puts the last place at the centre
            // (Mapper.cpp:2736); landmark assignment depends on the rand()
            // stream during the bitmap render and is not ported.
            const wx = rp.int(0x164 + 8 * k);
            const wy = rp.int(0x168 + 8 * k);
            if (k == province.max_bld and wx == pv.x and wy == pv.y) continue;
            try tst.expectEqual(wx, pv.bld_place[k][0]);
            try tst.expectEqual(wy, pv.bld_place[k][1]);
        }
        try tst.expectEqual(rp.int(0x10), pv.terrain);
        const fields = [_]struct { off: usize, v: i32, what: []const u8 }{
            .{ .off = 0x14, .v = pv.type, .what = "type" },
            .{ .off = 0x2c, .v = pv.population, .what = "population" },
            .{ .off = 0x34, .v = pv.explore, .what = "explore" },
            .{ .off = 0x24, .v = pv.gold_inc, .what = "gold" },
            .{ .off = 0x28, .v = pv.gem_inc, .what = "gem" },
        };
        for (fields) |f| if (rp.int(f.off) != f.v) {
            std.debug.print("\nprovince {d} terrain {d}: {s} want {d} got {d}\n", .{ i, pv.terrain, f.what, rp.int(f.off), f.v });
            return error.TestExpectedEqual;
        };
        const ref_name = try cp1251.toUtf8Alloc(gpa, std.mem.sliceTo(rp.name, 0));
        defer gpa.free(ref_name);
        if (pv.player == 0 and !std.mem.eql(u8, ref_name, pv.name)) {
            std.debug.print("\nprovince {d} terrain {d}: name want {any} got {any}\n", .{ i, pv.terrain, rp.name, pv.name });
            return error.TestExpectedEqual;
        }
        try checkGuard(&s, save.guards[i - 1], pv.guard, i);
        for (0..province.max_site) |k| {
            const st = pv.sites[k];
            const got = [6]i32{ st.index, st.pos, st.level, st.quest, st.guard, st.data };
            var want: [6]i32 = undefined;
            for (&want, 0..) |*w, j| w.* = rp.int(0x184 + 0x18 * k + 4 * j);
            if (!std.mem.eql(i32, &want, &got)) {
                std.debug.print("\nprovince {d} type {d} site {d}: want {any} got {any}\n", .{ i, pv.type, k, want, got });
                return error.TestExpectedEqual;
            }
            try checkGuard(&s, save.site_guards[(i - 1) * province.max_site + k], pv.site_guards[k], i);
        }
        try checkRecord(&rp.record, &pv.toRecord(), i);
        try checkBlock(&s, save.guards[i - 1], pv.guard, i);
        for (0..province.max_site) |k| try checkBlock(&s, save.site_guards[(i - 1) * province.max_site + k], pv.site_guards[k], i);
    }
    // The border cells: every row in the original's list order.
    var ci: usize = 0;
    for (0..s.grid.rows()) |y| {
        var k = s.grid.heads[y];
        while (k >= 0) : (ci += 1) {
            const c = s.grid.cells.items[@intCast(k)];
            k = c.next;
            const r = ref.cells[ci];
            try tst.expectEqual(@as(i32, @intCast(y)), r.y);
            try tst.expectEqual([4]i32{ r.x, r.a, r.b, r.c }, [4]i32{ c.x, c.a, c.b, c.c });
        }
    }
    try tst.expectEqual(ref.cells.len, ci);
}

/// Province.toRecord against the saved record.  Not compared: pointers
/// (sName, pGuard, pGarrison, arSiteGuard, pictures), PicX/PicY (DrawPic,
/// the picture stage) and the landmark slot arBuilding/arBldPlace[3].
fn checkRecord(want: []const u8, got: []const u8, where: usize) !void {
    var o: usize = 0;
    while (o + 4 <= want.len) : (o += 4) {
        if (o == 0 or o == 0x48 or o == 0x4c or o == 0x14c or o == 0x150 or o == 0x160 or o == 0x17c or o == 0x180 or o >= 0x454) continue;
        const a = savefile.int(want, o);
        const b = savefile.int(got, o);
        if (a == b) continue;
        std.debug.print("\nprovince {d} record +0x{x}: want {d} got {d}\n", .{ where, o, a, b });
        return error.TestExpectedEqual;
    }
}

/// Guard.writeBlock against the saved block.  dcHero() leaves ipPortrait,
/// ipProvince, iTarget, the pointers, dcQuest::bComplete and wavDeath
/// uninitialised: those words are heap garbage in the original's files.
fn checkBlock(s: *const Shard, want: ?savefile.Army, got: i32, where: usize) !void {
    const army = want orelse return;
    var out: std.ArrayList(u8) = .empty;
    defer out.deinit(tst.allocator);
    try s.guards.items[@intCast(got)].writeBlock(&out, tst.allocator);
    const rec = out.items[1..][0..savefile.hero_size];
    var o: usize = 0;
    while (o < savefile.hero_size) : (o += 4) {
        const quest = o >= 0xa8 and o < 0x210 and (o - 0xa8) % 0x24 == 0x10;
        if (o == 0x04 or o == 0x34 or o == 0x3c or (o >= 0x50 and o < 0xa8) or quest or o == 0x3a0) continue;
        const a = savefile.int(army.rec, o);
        const b = savefile.int(rec, o);
        if (a == b) continue;
        std.debug.print("\nprovince {d} guard +0x{x}: want {d} got {d}\n", .{ where, o, a, b });
        return error.TestExpectedEqual;
    }
    // The units follow in slot order; compare them byte for byte.
    var p: usize = 1 + savefile.hero_size;
    var k: usize = 0;
    for (0..16) |slot| {
        const present = out.items[p] != 0;
        p += 1;
        const saved = army.at(slot);
        if (present != (saved != null)) {
            std.debug.print("\nprovince {d} guard slot {d}: presence differs\n", .{ where, slot });
            return error.TestExpectedEqual;
        }
        if (!present) continue;
        // dcUnit::pHero at +0xA0 is a pointer.
        if (!std.mem.eql(u8, saved.?[0..0xa0], out.items[p..][0..0xa0])) {
            std.debug.print("\nprovince {d} guard unit {d} differs\n", .{ where, slot });
            return error.TestExpectedEqual;
        }
        p += savefile.unit_size;
        k += 1;
    }
    try tst.expectEqual(@as(usize, army.n), k);
}

/// A generated guard against the saved one: record fields, then every
/// unit instance byte for byte (type, life, exp, level, place, upgrades).
fn checkGuard(s: *const Shard, want: ?savefile.Army, got: i32, where: usize) !void {
    const army = want orelse {
        if (got >= 0) {
            std.debug.print("\nprovince {d}: unexpected guard\n", .{where});
            return error.TestExpectedEqual;
        }
        return;
    };
    if (got < 0) {
        std.debug.print("\nprovince {d}: guard missing\n", .{where});
        return error.TestExpectedEqual;
    }
    const g = s.guards.items[@intCast(got)];
    const rec = [3]i32{ savefile.int(army.rec, 0x00), savefile.int(army.rec, 0x0c), savefile.int(army.rec, 0x38) };
    const mine = [3]i32{ g.karma, g.guard, g.initiative };
    if (!std.mem.eql(i32, &rec, &mine)) {
        std.debug.print("\nprovince {d}: guard (karma, guard, initiative) want {any} got {any}\n", .{ where, rec, mine });
        return error.TestExpectedEqual;
    }
    if (army.n != g.count()) {
        std.debug.print("\nprovince {d}: guard units want {d} got {d}\n", .{ where, army.n, g.count() });
        return error.TestExpectedEqual;
    }
    for (army.list(), army.slots[0..army.n]) |b, slot| {
        const u = g.squad[slot].?;
        const w = unit_mod.Unit.fromBytes(b);
        if (w.type != u.type or w.life != u.life or w.exp != u.exp or w.level != u.level or w.x != u.x or w.y != u.y or
            !std.mem.eql(i32, &w.upgrades, &u.upgrades))
        {
            std.debug.print("\nprovince {d} unit {d}: want {any}\n got {any}\n", .{ where, slot, w, u });
            return error.TestExpectedEqual;
        }
    }
}

test "map generator matches the original: single player, seed 8097" {
    try checkAgainst("new_single", "game0.sav", "map0.map");
}

test "map generator matches the original: hot seat, seed 3219" {
    try checkAgainst("new_hot", "game0.hot", "map0.hot");
}

// testdata/tutorial: the first campaign mission saved by the original.
// The save was made during play, so only what GenerateMap decides is
// compared: terrain, type and names.  Province 23 may take sLand.
test "tutorial map matches the original: terrain, types and names" {
    const gpa = tst.allocator;
    // The save comes from the English version.
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "en") catch return error.SkipZigTest;
    defer fs.close();
    // Names come from the language files; compare them only for "en".
    const same_lang = std.mem.eql(u8, fs.lang(), "en");
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const gdata = try readTestFile(gpa, "testdata/tutorial/game0.cmp");
    defer gpa.free(gdata);
    const mdata = try readTestFile(gpa, "testdata/tutorial/map0.cmp");
    defer gpa.free(mdata);
    var ref = try mapfile.parse(gpa, mdata);
    defer ref.deinit(gpa);
    var save = try savefile.parse(gpa, gdata, ref.provinces.len);
    defer save.deinit(gpa);

    const hdr = gdata[3..][0..0x13c];
    var settings: profile.Map = undefined;
    inline for (@typeInfo(profile.Map).@"struct".field_names, 0..) |f, k|
        @field(settings, f) = savefile.int(hdr, 0x100 + 4 * k);
    const n_players: usize = @intCast(savefile.int(hdr, 0x38));
    var players: [max_players]Player = undefined;
    for (players[0..n_players], 1..) |*pl, k| {
        const rec = save.player(k);
        pl.* = .{ .kind = @fromBackingInt(@intCast(savefile.int(rec, 0))), .ruler = savefile.int(rec, 8), .difficulty = savefile.int(rec, 0xc) };
    }
    var rand: Rand = .{};
    var s = try generate(gpa, &pr, &rand, .{
        .seed = @bitCast(savefile.int(hdr, 0)),
        .map = settings,
        .game_type = 2,
        .players = players[0..n_players],
        .tutorial = true,
    });
    defer s.deinit();
    try tst.expectEqual(ref.provinces.len, s.count());
    for (ref.provinces, 1..) |*rp, i| {
        const pv = s.provinces[i];
        if (rp.int(0x10) != pv.terrain or rp.int(0x14) != pv.type) {
            std.debug.print("\nprovince {d}: terrain/type want {d}/{d} got {d}/{d}\n", .{ i, rp.int(0x10), rp.int(0x14), pv.terrain, pv.type });
            return error.TestExpectedEqual;
        }
        if (i == 23 or !same_lang) continue;
        const ref_name = try cp1251.toUtf8Alloc(gpa, std.mem.sliceTo(rp.name, 0));
        defer gpa.free(ref_name);
        if (!std.mem.eql(u8, ref_name, pv.name)) {
            std.debug.print("\nprovince {d}: name want {s} got {s}\n", .{ i, ref_name, pv.name });
            return error.TestExpectedEqual;
        }
    }
}

test "roll picks by weight and skips zero weights" {
    var r: Rand = .{};
    r.seed(5);
    for (0..50) |_| {
        const v = roll(&r, &.{ 7, 8, 9 }, &.{ 0, 5, 0 });
        try tst.expectEqual(@as(i32, 8), v);
    }
}
