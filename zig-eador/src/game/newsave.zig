//! save\game%d.sav / .hot of a new game as SaveGame (GameMenu.cpp:330,
//! GOG FUN_004aa0c0) writes it right after GenerateMap, before packing:
//!   "ok", mode byte, 0x13C CurGame, 0x194 WorldRules,
//!   per province the record, guard, garrison and 30 site guard blocks,
//!   the 17 player records, per player 8 hero blocks and its lists,
//!   iMaxPureItem, the shops, the rituals and the 0x120 Tutorial block.
//! The campaign (mode 1, Campaign and the shards) is not written yet.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const mapgen = @import("mapgen.zig");
const province = @import("province.zig");
const guard_mod = @import("guard.zig");
const player_mod = @import("player.zig");

pub const header_size = 0x13c;
pub const world_rules_size = 0x194;
pub const player_size = 0x4a34;
pub const max_records = 17;
pub const max_hero = 8;
pub const shop_record = 0x3a8;
pub const tutorial_size = 0x120;
/// dcPlayer::bInGame and arBld[MAX_BLD].
const player_in_game = 0x10f;
const player_bld = 0x113;
const max_bld = 1000;

/// What the game file needs besides the shard.
pub const Info = struct {
    seed: u32,
    /// CurGame.iGameType: 1 single, 3 hot seat.
    game_type: i32,
    players: []const mapgen.Player,
    /// CurGame.arPlayerSet[1..]: the rulers as chosen.
    requested: []const i32 = &.{},
    diplomacy: bool = true,
    fog: bool = true,
    /// eador.cfg "Resource price modifier".
    price_mod: i32 = 5,
    /// CurGame.iCurrentTurn, iActivePlayer and bAutoSave (RunGame sets
    /// it for the save at the end of a turn).
    turn: i32 = 0,
    active_player: i32 = 1,
    auto_save: bool = false,
    /// pFog of player k by province number (index 0 unused) where the
    /// caller has a newer one than the shard's file.
    fogs: [mapgen.max_players + 1]?[]const u8 = @splat(null),
};

fn setI(b: []u8, off: usize, v: i32) void {
    std.mem.writeInt(i32, b[off..][0..4], v, .little);
}

fn putI(gpa: std.mem.Allocator, out: *std.ArrayList(u8), v: i32) !void {
    var b: [4]u8 = undefined;
    std.mem.writeInt(i32, &b, v, .little);
    try out.appendSlice(gpa, &b);
}

/// CurGame (DAT_0051e658): a loaded game's with the turn, the active
/// player and the autosave flag, else a new one.
fn header(s: *const mapgen.Shard, info: Info) [header_size]u8 {
    if (s.saved) |sv| {
        var h: [header_size]u8 = sv.header[0..header_size].*;
        setI(&h, 0x04, info.turn);
        setI(&h, 0x0c, s.res_buy);
        for (s.res_prices, 0..) |v, k| setI(&h, 0x10 + 4 * k, v);
        setI(&h, 0x3c, info.active_player);
        h[0xfd] = @intFromBool(info.auto_save);
        return h;
    }
    var h: [header_size]u8 = @splat(0);
    const n: i32 = @intCast(info.players.len);
    setI(&h, 0x00, @bitCast(if (info.seed == 0) @as(u32, 111) else info.seed));
    setI(&h, 0x04, info.turn);
    setI(&h, 0x08, info.game_type);
    setI(&h, 0x0c, info.price_mod - @divTrunc(n, 5));
    for (0..10) |k| setI(&h, 0x10 + 4 * k, 100);
    setI(&h, 0x38, n);
    // A new game is saved with iActivePlayer 1 (FUN_004ae4c0).
    setI(&h, 0x3c, info.active_player);
    for (info.requested, 1..) |v, i| if (0x40 + 4 * i < 0x84) setI(&h, 0x40 + 4 * i, v);
    for (0..30) |r| setI(&h, 0x84 + 4 * r, s.race_quest[r]);
    h[0xfc] = @intFromBool(info.diplomacy);
    h[0xfd] = @intFromBool(info.auto_save);
    h[0xfe] = @intFromBool(info.fog);
    inline for (@typeInfo(@TypeOf(s.settings)).@"struct".field_names, 0..) |f, k|
        setI(&h, 0x100 + 4 * k, @field(s.settings, f));
    setI(&h, 0x108, s.x);
    setI(&h, 0x10c, s.y);
    return h;
}

fn writeGuard(gpa: std.mem.Allocator, out: *std.ArrayList(u8), s: *const mapgen.Shard, g: i32) !void {
    if (g < 0) return out.append(gpa, 0);
    try s.guards.items[@intCast(g)].writeBlock(out, gpa);
}

/// arPlayer[0..16]: a loaded game's records, else as the constructor,
/// Init and GenerateMap leave them; then the shard's rulers over them.
fn players(pr: *const Proto, s: *const mapgen.Shard, info: Info, recs: []u8) void {
    if (s.saved) |sv| @memcpy(recs, sv.players) else newPlayers(pr, s, info, recs);
    for (1..s.n_players + 1) |k| {
        const rec = recs[k * player_size ..][0..player_size];
        const r = s.rulers[k];
        setI(rec, 0x00, @backingInt(r.kind));
        setI(rec, 0x0c, r.difficulty);
        setI(rec, 0x10, r.corrupt);
        setI(rec, 0x14, r.gold_inc);
        setI(rec, 0x18, r.gem_inc);
        setI(rec, 0x24, r.karma);
        setI(rec, 0x28, r.gold_add);
        setI(rec, 0x2c, r.gem_add);
        setI(rec, 0x38, r.calc_corrupt);
        setI(rec, 0x100, r.gold);
        setI(rec, 0x104, r.gem);
        player_mod.writeWall(rec, r.wall);
        rec[player_in_game] = @intFromBool(s.in_game[k]);
        // A new game's Holdings start as Init leaves them; Build fills them.
        r.holdings.write(rec);
        const bld = rec[player_bld..][0..max_bld];
        if (s.castle[k].len == max_bld) {
            for (bld, s.castle[k]) |*b, v| b.* = @intFromBool(v);
        }
    }
}

fn newPlayers(pr: *const Proto, s: *const mapgen.Shard, info: Info, recs: []u8) void {
    @memset(recs, 0);
    for (0..max_records) |k| {
        const rec = recs[k * player_size ..][0..player_size];
        setI(rec, 0, 2);
        setI(rec, 0x20, 15);
        rec[0x110] = 1;
        rec[0x113] = 1;
        for (0..max_records) |j| setI(rec, 0x924 + 4 * j, 100);
    }
    // arPlayer[0], the neutrals.
    setI(recs, 0x104, 10000);
    for (info.players, 1..) |pl, k| {
        const rec = recs[k * player_size ..][0..player_size];
        setI(rec, 0x00, @backingInt(pl.kind));
        setI(rec, 0x04, @intCast(k));
        setI(rec, 0x08, pl.ruler);
        setI(rec, 0x0c, pl.difficulty);
        setI(rec, 0x24, if (pl.kind == .human) 0 else pr.player_profiles[@intCast(pl.ruler)].karma);
        for (0..4) |i| setI(rec, 0x3c + 4 * i, 1);
        setI(rec, 0x50, @intCast(k - 1));
        setI(rec, 0x54, s.capitals[k]);
        setI(rec, 0x58, 100);
        setI(rec, 0x60, 10);
        setI(rec, 0x64, 12);
        if (pl.kind == .human) {
            const d = pr.difficulties[@intCast(std.math.clamp(pl.difficulty, 0, proto_mod.max_difficulty - 1))];
            setI(rec, 0x100, d.gold);
            setI(rec, 0x104, d.gem);
        } else {
            setI(rec, 0x100, (pl.difficulty + 10) * 50);
            setI(rec, 0x104, pl.difficulty * 5 + 20);
        }
        rec[0x10f] = 1;
        rec[0x112] = 1;
        // Relations of two computers: players.var Attitude.
        if (pl.kind == .computer) for (info.players, 1..) |other, j| {
            if (j == k or other.kind != .computer) continue;
            const a = pr.player_profiles[@intCast(pl.ruler)].attitude[@intCast(other.ruler)];
            setI(rec, 0x1080 + 0x38c * (j - 1), a);
        };
    }
}

/// CurGame.iSeed of the shard's file (111, the seed of seed 0, without one).
pub fn seedOf(s: *const mapgen.Shard) u32 {
    const sv = s.saved orelse return 111;
    return std.mem.readInt(u32, sv.header[0..4], .little);
}

/// CurGame.iGameType of the shard's file (0 without one).
pub fn gameType(s: *const mapgen.Shard) i32 {
    const sv = s.saved orelse return 0;
    return std.mem.readInt(i32, sv.header[0x08..][0..4], .little);
}

/// SaveGame of a game in progress: the shard holds its file
/// (Shard.saved); `fog` is the active player's pFog by province number.
pub const Progress = struct {
    turn: i32,
    active_player: i32,
    auto_save: bool,
    player: usize,
    fog: []const u8,
};

pub fn progressBytes(gpa: std.mem.Allocator, pr: *const Proto, s: *const mapgen.Shard, p: Progress) ![]u8 {
    const sv = s.saved orelse return error.NotSaved;
    var info: Info = .{
        .seed = std.mem.readInt(u32, sv.header[0..4], .little),
        .game_type = gameType(s),
        .players = &.{},
        .turn = p.turn,
        .active_player = p.active_player,
        .auto_save = p.auto_save,
    };
    if (p.fog.len == s.provinces.len) info.fogs[p.player] = p.fog;
    return gameBytes(gpa, pr, s, info);
}

/// The unpacked game file.  `gpa` owns the result.
pub fn gameBytes(gpa: std.mem.Allocator, pr: *const Proto, s: *const mapgen.Shard, info: Info) ![]u8 {
    // A new campaign shard needs Campaign and the shards, which only a
    // loaded file has.
    if (info.game_type == 2 and s.saved == null) return error.CampaignNotSupported;
    var out: std.ArrayList(u8) = .empty;
    errdefer out.deinit(gpa);
    try out.appendSlice(gpa, "ok");
    try out.append(gpa, if (s.saved) |sv| sv.mode else 0);
    try out.appendSlice(gpa, &header(s, info));
    if (s.saved) |sv| try out.appendSlice(gpa, sv.world_rules) else try out.appendNTimes(gpa, 0, world_rules_size);
    for (s.provinces[1..]) |*pv| {
        try out.appendSlice(gpa, &pv.toRecord());
        try writeGuard(gpa, &out, s, pv.guard);
        if (pv.garrison >= 0) {
            try s.garrisons.items[@intCast(pv.garrison)].writeBlock(&out, gpa);
        } else try out.append(gpa, 0);
        for (pv.site_guards) |g| try writeGuard(gpa, &out, s, g);
    }
    const recs = try out.addManyAsSlice(gpa, max_records * player_size);
    players(pr, s, info, recs);
    for (1..s.n_players + 1) |k| {
        for (0..max_hero) |slot| {
            const h = for (s.heroes.items) |*h| {
                if (h.owner == @as(i32, @intCast(k)) and h.slot == slot) break h;
            } else null;
            if (h) |x| try x.writeBlock(&out, gpa) else try out.append(gpa, 0);
        }
        if (s.saved) |sv| try out.appendSlice(gpa, sv.lists[k]) else if (s.ritual_lists[k]) |l| try out.appendSlice(gpa, l) else for ([_]struct { n: usize, size: usize }{
            .{ .n = pr.defenders.len, .size = 4 },
            .{ .n = pr.outer_buildings.len, .size = 4 },
            .{ .n = pr.rituals.len, .size = 8 },
        }) |l| {
            try putI(gpa, &out, @intCast(l.n - 1));
            try out.appendNTimes(gpa, 0, l.n * l.size);
        }
        if (info.fogs[k]) |f| {
            for (f[1..]) |v| try putI(gpa, &out, v);
        } else if (s.saved) |sv| {
            try out.appendSlice(gpa, sv.fog[k]);
        } else {
            // pFog: 0 for the capital and its neighbours, 2 elsewhere.
            const home = &s.provinces[@intCast(s.capitals[k])];
            for (1..s.provinces.len) |p| {
                const pi: i32 = @intCast(p);
                const seen = pi == s.capitals[k] or std.mem.indexOfScalar(i32, home.neighbourList(), pi) != null;
                try putI(gpa, &out, if (seen) 0 else 2);
            }
        }
        if (s.rulers[k].story.story.len > 0) {
            try s.rulers[k].story.write(gpa, &out);
        } else if (s.saved) |sv| try out.appendSlice(gpa, sv.events[k]) else {
            // pStory -1 and pPlace 0 for every event.
            try putI(gpa, &out, @intCast(pr.events.len - 1));
            for (1..pr.events.len) |_| {
                try putI(gpa, &out, -1);
                try putI(gpa, &out, 0);
            }
        }
    }
    try putI(gpa, &out, @intCast(pr.max_pure_item));
    try putI(gpa, &out, @intCast(s.shops.items.len));
    for (s.shops.items) |sh| {
        const b = try out.addManyAsSlice(gpa, shop_record);
        setI(b, 0, sh.kind);
        for (sh.prices, 0..) |v, i| setI(b, 4 + 4 * i, v);
        for (sh.items, 0..) |it, i| {
            setI(b, 0x48 + 8 * i, it[0]);
            setI(b, 0x4c + 8 * i, it[1]);
        }
    }
    if (s.saved) |sv| try out.appendSlice(gpa, sv.tail) else {
        // pRitual: no ritual is cast yet.
        try putI(gpa, &out, 0);
        try out.appendNTimes(gpa, 0, tutorial_size);
    }
    return out.toOwnedSlice(gpa);
}

const tst = std.testing;
const GameFs = @import("../platform/fs.zig").GameFs;
const packfile = @import("../formats/packfile.zig");
const savefile = @import("../formats/savefile.zig");
const profile = @import("../data/profile.zig");
const Rand = @import("../data/rand.zig").Rand;

fn readTestFile(gpa: std.mem.Allocator, path: []const u8) ![]u8 {
    const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, gpa, .unlimited) catch return error.SkipZigTest;
    defer gpa.free(bytes);
    return packfile.unpack(gpa, bytes);
}

fn checkGame(dir: []const u8, game: []const u8, n_prov: usize) !void {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    var pb: [96]u8 = undefined;
    const want = try readTestFile(gpa, try std.fmt.bufPrint(&pb, "testdata/{s}/{s}", .{ dir, game }));
    defer gpa.free(want);
    var save = try savefile.parse(gpa, want, n_prov);
    defer save.deinit(gpa);
    const hdr = want[3..][0..header_size];
    var settings: profile.Map = undefined;
    inline for (@typeInfo(profile.Map).@"struct".field_names, 0..) |f, k|
        @field(settings, f) = savefile.int(hdr, 0x100 + 4 * k);
    const n_players: usize = @intCast(savefile.int(hdr, 0x38));
    var pls: [mapgen.max_players]mapgen.Player = undefined;
    var req: [mapgen.max_players]i32 = undefined;
    for (pls[0..n_players], 1..) |*pl, k| {
        const rec = save.player(k);
        pl.* = .{ .kind = @fromBackingInt(@intCast(savefile.int(rec, 0))), .ruler = savefile.int(rec, 8), .difficulty = savefile.int(rec, 0xc) };
        req[k - 1] = savefile.int(hdr, 0x40 + 4 * k);
    }
    var rand: Rand = .{};
    var s = try mapgen.generate(gpa, &pr, &rand, .{
        .seed = @bitCast(savefile.int(hdr, 0)),
        .map = settings,
        .game_type = savefile.int(hdr, 8),
        .players = pls[0..n_players],
    });
    defer s.deinit();
    const got = try gameBytes(gpa, &pr, &s, .{
        .seed = @bitCast(savefile.int(hdr, 0)),
        .game_type = savefile.int(hdr, 8),
        .players = pls[0..n_players],
        .requested = req[0..n_players],
    });
    defer gpa.free(got);
    try tst.expectEqual(want.len, got.len);
    try compareMasked(want, got, n_prov, n_players, pls[0..n_players]);
}

/// Word `o` of a province record the generator does not decide: pointers
/// (sName, pGuard, pGarrison, arSiteGuard, pictures), PicX/PicY (DrawPic)
/// and the landmark slot arBuilding/arBldPlace[3].
fn recordSkip(o: usize) bool {
    return o == 0 or o == 0x48 or o == 0x4c or o == 0x14c or o == 0x150 or o == 0x160 or o == 0x17c or o == 0x180 or o >= 0x454;
}

/// dcHero() leaves ipPortrait, ipProvince, iTarget, the pointers,
/// dcQuest::bComplete and wavDeath uninitialised.
fn heroSkip(o: usize) bool {
    const quest = o >= 0xa8 and o < 0x210 and (o - 0xa8) % 0x24 == 0x10;
    return o == 0x04 or o == 0x34 or o == 0x3c or (o >= 0x50 and o < 0xa8) or quest or o == 0x3a0;
}

/// The original's file was saved when the first turn was left: player 1
/// has the turn's income (GoldInc, GemInc, Gold, Gem), and MainScreen set
/// iMember and iCondition.  Past +0x4A18 the record holds pointers.
fn playerSkip(k: usize, human: bool, o: usize) bool {
    if (o >= 0x4a18) return true;
    if (k == 0 or !human) return false;
    return o == 0x14 or o == 0x18 or o == 0x1c or o == 0x20 or o == 0x100 or o == 0x104;
}

fn eqWords(want: []const u8, got: []const u8, base: usize, comptime skip: anytype, ctx: anytype, what: []const u8) !void {
    var o: usize = 0;
    while (o < want.len) : (o += 4) {
        if (skip(ctx, o)) continue;
        if (std.mem.eql(u8, want[o..@min(o + 4, want.len)], got[o..@min(o + 4, got.len)])) continue;
        std.debug.print("\n{s} at file 0x{x} (+0x{x}): want {any} got {any}\n", .{ what, base + o, o, want[o..@min(o + 4, want.len)], got[o..@min(o + 4, got.len)] });
        return error.TestExpectedEqual;
    }
}

pub fn compareMasked(want: []const u8, got: []const u8, n_prov: usize, n_players: usize, pls: []const mapgen.Player) !void {
    const Skip = struct {
        /// Player 1's capital grew during the first turn (iPopulation).
        fn rec(first_capital: bool, o: usize) bool {
            return recordSkip(o) or (first_capital and o == 0x2c);
        }
        fn hero(_: void, o: usize) bool {
            return heroSkip(o);
        }
        fn none(_: void, _: usize) bool {
            return false;
        }
        fn unit(_: void, o: usize) bool {
            // dcUnit::pHero.
            return o == 0xa0;
        }
        const P = struct { k: usize, human: bool };
        fn player(c: P, o: usize) bool {
            return playerSkip(c.k, c.human, o);
        }
    };
    var p: usize = 3 + header_size + world_rules_size;
    try eqWords(want[0..p], got[0..p], 0, Skip.none, {}, "header");
    // The army blocks: the presence bytes must match, then the records.
    const block = struct {
        fn f(w: []const u8, g: []const u8, at: *usize) !void {
            if (w[at.*] != g[at.*]) {
                std.debug.print("\nblock presence at 0x{x}\n", .{at.*});
                return error.TestExpectedEqual;
            }
            const present = w[at.*] != 0;
            at.* += 1;
            if (!present) return;
            try eqWords(w[at.*..][0..guard_mod.hero_size], g[at.*..][0..guard_mod.hero_size], at.*, Skip.hero, {}, "hero");
            at.* += guard_mod.hero_size;
            for (0..guard_mod.max_squad) |_| {
                if (w[at.*] != g[at.*]) {
                    std.debug.print("\nunit presence at 0x{x}\n", .{at.*});
                    return error.TestExpectedEqual;
                }
                const u = w[at.*] != 0;
                at.* += 1;
                if (!u) continue;
                try eqWords(w[at.*..][0..0xa4], g[at.*..][0..0xa4], at.*, Skip.unit, {}, "unit");
                at.* += 0xa4;
            }
        }
    }.f;
    for (0..n_prov) |_| {
        const first_capital = savefile.int(want[p..], 0x18) == 1 and savefile.int(want[p..], 0x14) == province.type_castle;
        try eqWords(want[p..][0..0x4e4], got[p..][0..0x4e4], p, Skip.rec, first_capital, "province");
        p += 0x4e4;
        for (0..32) |_| try block(want, got, &p);
    }
    for (0..max_records) |k| {
        const human = k > 0 and k <= n_players and pls[k - 1].kind == .human;
        try eqWords(want[p..][0..player_size], got[p..][0..player_size], p, Skip.player, Skip.P{ .k = k, .human = human }, "player");
        p += player_size;
    }
    // Per player: the hero blocks, then the lists, pFog and the events
    // (a count n and n + 1 items, the events n items).
    for (0..n_players) |_| {
        for (0..max_hero) |_| try block(want, got, &p);
        const start = p;
        for ([_]struct { size: usize, extra: usize }{
            .{ .size = 4, .extra = 1 }, .{ .size = 4, .extra = 1 }, .{ .size = 8, .extra = 1 },
        }) |l| {
            const n: usize = @intCast(savefile.int(want, p));
            p += 4 + (n + l.extra) * l.size;
        }
        p += 4 * n_prov;
        const ev: usize = @intCast(savefile.int(want, p));
        p += 4 + ev * 8;
        try eqWords(want[start..p], got[start..p], start, Skip.none, {}, "lists, fog, events");
    }
    try eqWords(want[p..], got[p..], p, Skip.none, {}, "shops, rituals, tutorial");
}

test "new game file against the original: single player" {
    try checkGame("new_single", "game0.sav", 100);
}

test "new game file against the original: hot seat" {
    try checkGame("new_hot", "game0.hot", 144);
}
