//! The rules of the new-game screen (NewGame, GameMenu.cpp:1295) without
//! drawing: the players, their rulers and difficulties, the shard size.
//!
//! Slot i is arPlayer[i + 1].  Slot 0 is the profile in use.  A computer's
//! `ruler` is ipProfile (players.var record, 0 = random).  A human's
//! `human` is arHumanID: an index into the profiles found in save/, -1 for
//! a new player.
const std = @import("std");
const profile = @import("profile.zig");
const Rand = @import("rand.zig").Rand;

pub const max_slots = 16;
pub const min_players = 2;
/// Rulers of players.var (records 1..16).
pub const max_ruler = 16;
pub const max_difficulty = 6;

pub const Kind = enum(u8) { human = 1, computer = 2 };

pub const Type = enum(u8) {
    /// Одиночная игра (GameType 1).
    single = 1,
    /// Несколько игроков → один компьютер (GameType 3).
    hotseat = 3,
};

pub const Slot = struct {
    kind: Kind = .computer,
    ruler: i32 = 0,
    difficulty: i32 = 0,
    human: i32 = -1,
};

/// What «Создать карту» hands to the map generator (CurGame and the
/// arPlayer fields NewGame sets).
pub const Game = struct {
    type: Type,
    seed: u32,
    map: profile.Map,
    slots: [max_slots]Slot,
    /// CurGame.arPlayerSet[1..]: the rulers as chosen, before the random
    /// ones are drawn.
    player_set: [max_slots]i32,
    diplomacy: bool,
    fog: bool,

    pub fn players(self: *const Game) usize {
        return @intCast(self.map.players);
    }
};

pub const Setup = struct {
    map: profile.Map,
    /// Shard size: shard.var record, (Map.X - 6) / 2.
    size: i32,
    slots: [max_slots]Slot,
    type: Type,
    /// MaxHumans: the profiles a hot-seat human can take.
    n_profiles: usize = 0,

    /// `own`: the index of the profile in use among the profiles.  NewGame
    /// starts arHumanID[0] at 1 and changes it only when the profile file
    /// is found.
    pub fn init(map: profile.Map, difficulty: i32, t: Type, n_profiles: usize, own: ?usize) Setup {
        var s: Setup = .{
            .map = map,
            .size = std.math.clamp(@divTrunc(map.x - 6, 2), 0, 7),
            .slots = @splat(.{ .difficulty = difficulty }),
            .type = t,
            .n_profiles = if (t == .hotseat) n_profiles else 0,
        };
        s.map.players = std.math.clamp(map.players, min_players, max_slots);
        s.slots[0] = .{ .kind = .human, .difficulty = difficulty, .human = if (own) |o| @intCast(o) else 1 };
        return s;
    }

    pub fn players(self: *const Setup) usize {
        return @intCast(self.map.players);
    }

    fn active(self: *Setup) []Slot {
        return self.slots[0..self.players()];
    }

    /// IsPlayerPicked: another computer has the ruler.
    pub fn picked(self: *const Setup, ruler: i32) bool {
        if (ruler == 0) return false;
        var n: usize = 0;
        for (self.slots[0..self.players()]) |s| {
            if (s.kind == .computer and s.ruler == ruler) n += 1;
        }
        return n > 1;
    }

    /// Humans: the hot-seat opponents that have a profile.
    pub fn humans(self: *const Setup) usize {
        var n: usize = 0;
        for (self.slots[1..self.players()]) |s| {
            if (s.kind == .human and s.human >= 0) n += 1;
        }
        return n;
    }

    /// The ruler arrows of a slot are shown (buttRuleL/R Activate(1));
    /// slot 0 shows them inactive.
    pub fn rulerArrows(self: *const Setup, i: usize) bool {
        if (self.slots[i].kind == .computer) return true;
        return self.type == .hotseat and self.humans() < self.n_profiles;
    }

    pub fn sizeDown(self: *Setup, shard_players: []const i32) void {
        if (self.size <= 0) return;
        self.size -= 1;
        const cap = shard_players[@intCast(self.size)];
        if (self.map.players > cap) self.map.players = cap;
    }

    pub fn sizeUp(self: *Setup, shard_players: []const i32) void {
        if (self.size < @as(i32, @intCast(shard_players.len)) - 1) self.size += 1;
    }

    pub fn playersDown(self: *Setup) void {
        if (self.map.players > min_players) self.map.players -= 1;
    }

    /// The new slot drops a ruler or a profile someone already has; the
    /// shard grows until it holds everyone.
    pub fn playersUp(self: *Setup, shard_players: []const i32) void {
        if (self.map.players >= max_slots) return;
        self.map.players += 1;
        const n = self.players();
        const s = &self.slots[n - 1];
        if (s.kind == .computer and s.ruler > 0 and self.picked(s.ruler)) s.ruler = 0;
        if (s.kind == .human and s.human >= 0) {
            for (self.slots[0 .. n - 1]) |o| if (o.human == s.human) {
                s.human = -1;
                break;
            };
        }
        while (self.size < @as(i32, @intCast(shard_players.len)) - 1 and self.map.players > shard_players[@intCast(self.size)]) self.size += 1;
    }

    /// mskPlayer: human and computer swap (hot seat, not slot 0).
    pub fn toggleKind(self: *Setup, i: usize) void {
        if (self.type != .hotseat or i == 0) return;
        const s = &self.slots[i];
        s.kind = if (s.kind == .human) .computer else .human;
        s.human = -1;
        s.ruler = 0;
    }

    /// buttRuleL/R: a computer cycles 0..16 skipping taken rulers; a human
    /// cycles -1 and the profiles skipping those another slot has.
    pub fn rulerStep(self: *Setup, i: usize, up: bool) void {
        if (i == 0) return;
        const s = &self.slots[i];
        if (s.kind == .computer) {
            while (true) {
                if (up) {
                    s.ruler = if (s.ruler < max_ruler) s.ruler + 1 else 0;
                } else {
                    s.ruler = if (s.ruler > 0) s.ruler - 1 else max_ruler;
                }
                if (!self.picked(s.ruler)) return;
            }
        }
        const last: i32 = @as(i32, @intCast(self.n_profiles)) - 1;
        while (true) {
            if (up) {
                s.human = if (s.human < last) s.human + 1 else -1;
            } else {
                s.human = if (s.human >= 0) s.human - 1 else last;
            }
            if (s.human < 0) return;
            const taken = for (self.active(), 0..) |o, j| {
                if (j != i and o.human == s.human) break true;
            } else false;
            if (!taken) return;
        }
    }

    pub fn difficultyStep(self: *Setup, i: usize, up: bool) void {
        const s = &self.slots[i];
        if (up and s.difficulty < max_difficulty) s.difficulty += 1;
        if (!up and s.difficulty > 0) s.difficulty -= 1;
    }

    /// «Создать карту»: srand(seed), the random computers draw
    /// 1 + Random(16) until the ruler is free.  `files[k]` is the file
    /// index of profile k: a human's ruler becomes it (ipProfile), -1 for
    /// a new player.
    pub fn finish(self: *Setup, rand: *Rand, seed: u32, shard: [3]i32, files: []const i32, active_file: i32, diplomacy: bool, fog: bool) Game {
        rand.seed(seed);
        var set: [max_slots]i32 = @splat(0);
        for (self.active(), 0..) |*s, i| {
            if (s.kind == .human) {
                // A new player keeps the file index the caller set after
                // its profile window (WindowProfile(i, false)).
                if (i == 0) {
                    s.ruler = active_file;
                } else if (s.human >= 0 and s.human < files.len) s.ruler = files[@intCast(s.human)];
            }
            set[i] = s.ruler;
            if (s.kind == .computer and s.ruler == 0) {
                while (true) {
                    s.ruler = 1 + @as(i32, @intCast(rand.random(max_ruler)));
                    if (!self.picked(s.ruler)) break;
                }
            }
        }
        var map = self.map;
        map.x = shard[0];
        map.y = shard[1];
        return .{ .type = self.type, .seed = seed, .map = map, .slots = self.slots, .player_set = set, .diplomacy = diplomacy, .fog = fog };
    }
};

/// WindowMapSettings: the ten 0..4 sliders and back.
pub const MapSliders = struct {
    v: [10]i32,

    pub fn from(m: profile.Map) MapSliders {
        return .{ .v = .{
            @divTrunc(m.isles, 5),
            @divTrunc(m.dissolve, 10) - 1,
            @divTrunc(m.water - 5, 10),
            @divTrunc(m.forest - 10, 5),
            @divTrunc(m.hill - 15, 5),
            @divTrunc(m.swamp - 10, 5),
            m.population,
            switch (m.neutrals) {
                1 => 0,
                4 => 1,
                8 => 2,
                12 => 3,
                // The C++ leaves arSet[7] unset for other values.
                else => 4,
            },
            m.resource,
            m.site,
        } };
    }

    pub fn apply(self: MapSliders, m: *profile.Map) void {
        m.isles = self.v[0] * 5;
        m.dissolve = (self.v[1] + 1) * 10;
        m.water = self.v[2] * 10 + 5;
        m.forest = self.v[3] * 5 + 10;
        m.hill = self.v[4] * 5 + 15;
        m.swamp = self.v[5] * 5 + 10;
        m.population = self.v[6];
        m.neutrals = switch (self.v[7]) {
            0 => 1,
            1 => 4,
            2 => 8,
            3 => 12,
            else => 20,
        };
        m.resource = self.v[8];
        m.site = self.v[9];
    }

    pub const defaults: MapSliders = .{ .v = .{ 2, 2, 2, 2, 2, 2, 2, 4, 2, 2 } };
};

const shards_test = [_]i32{ 2, 3, 5, 8, 11, 14, 16, 16 };

fn defaultMap() profile.Map {
    return profile.Human.empty().map();
}

test "default map settings give the default sliders" {
    const m = defaultMap();
    const s = MapSliders.from(m);
    try std.testing.expectEqualSlices(i32, &MapSliders.defaults.v, &s.v);
    var back = m;
    s.apply(&back);
    try std.testing.expectEqual(m, back);
}

test "players and shard size limit each other" {
    var s = Setup.init(defaultMap(), 0, .single, 0, null);
    try std.testing.expectEqual(@as(usize, 4), s.players());
    try std.testing.expectEqual(@as(i32, 2), s.size);
    s.sizeDown(&shards_test);
    try std.testing.expectEqual(@as(usize, 3), s.players());
    s.sizeDown(&shards_test);
    s.sizeDown(&shards_test);
    try std.testing.expectEqual(@as(i32, 0), s.size);
    try std.testing.expectEqual(@as(usize, 2), s.players());
    s.playersDown();
    try std.testing.expectEqual(@as(usize, 2), s.players());
    s.playersUp(&shards_test);
    try std.testing.expectEqual(@as(i32, 1), s.size);
    for (0..20) |_| s.playersUp(&shards_test);
    try std.testing.expectEqual(@as(usize, 16), s.players());
    try std.testing.expectEqual(@as(i32, 6), s.size);
}

test "computer rulers stay unique" {
    var s = Setup.init(defaultMap(), 0, .single, 0, null);
    s.rulerStep(1, true);
    try std.testing.expectEqual(@as(i32, 1), s.slots[1].ruler);
    s.rulerStep(2, true);
    try std.testing.expectEqual(@as(i32, 2), s.slots[2].ruler);
    s.rulerStep(2, false);
    try std.testing.expectEqual(@as(i32, 0), s.slots[2].ruler);
    s.rulerStep(2, false);
    try std.testing.expectEqual(@as(i32, 16), s.slots[2].ruler);
    s.rulerStep(0, true);
    try std.testing.expectEqual(@as(i32, 0), s.slots[0].ruler);
    s.slots[4].ruler = 1;
    s.playersUp(&shards_test);
    try std.testing.expectEqual(@as(i32, 0), s.slots[4].ruler);
}

test "random rulers: the original's draw for seed 8097" {
    // recomp saves/new_single: players 2..4 got rulers 1, 15, 11.
    var r: Rand = .{};
    var s = Setup.init(defaultMap(), 0, .single, 0, null);
    const g = s.finish(&r, 8097, .{ 10, 10, 5 }, &.{}, 0, true, true);
    try std.testing.expectEqual([3]i32{ 1, 15, 11 }, [3]i32{ g.slots[1].ruler, g.slots[2].ruler, g.slots[3].ruler });
    try std.testing.expectEqual(@as(i32, 0), g.player_set[1]);
    try std.testing.expectEqual(@as(i32, 10), g.map.x);
}

test "random rulers are drawn without repeats" {
    var r: Rand = .{};
    var s = Setup.init(defaultMap(), 0, .single, 0, null);
    s.map.players = 16;
    s.slots[3].ruler = 5;
    const g = s.finish(&r, 7, .{ 20, 20, 16 }, &.{}, 0, true, true);
    var seen: [17]bool = @splat(false);
    for (g.slots[1..16]) |sl| {
        try std.testing.expect(sl.ruler >= 1 and sl.ruler <= 16);
        try std.testing.expect(!seen[@intCast(sl.ruler)]);
        seen[@intCast(sl.ruler)] = true;
    }
    try std.testing.expectEqual(@as(i32, 5), g.slots[3].ruler);
    try std.testing.expectEqual(@as(i32, 5), g.player_set[3]);
}

test "hot seat humans take free profiles" {
    // Three profiles, the one in use is 0.
    var s = Setup.init(defaultMap(), 0, .hotseat, 3, 0);
    s.toggleKind(1);
    try std.testing.expectEqual(Kind.human, s.slots[1].kind);
    try std.testing.expectEqual(@as(i32, -1), s.slots[1].human);
    try std.testing.expect(s.rulerArrows(1));
    s.rulerStep(1, true);
    try std.testing.expectEqual(@as(i32, 1), s.slots[1].human);
    s.toggleKind(2);
    s.rulerStep(2, true);
    try std.testing.expectEqual(@as(i32, 2), s.slots[2].human);
    // As in NewGame, Humans counts only the opponents: with two of three
    // profiles taken by them the arrows still show, but the profile in use
    // is taken too, so stepping comes back to a new player.
    s.toggleKind(3);
    try std.testing.expect(s.rulerArrows(3));
    s.rulerStep(3, true);
    try std.testing.expectEqual(@as(i32, -1), s.slots[3].human);
    s.rulerStep(1, false);
    try std.testing.expectEqual(@as(i32, -1), s.slots[1].human);
    s.rulerStep(1, false);
    try std.testing.expectEqual(@as(i32, 1), s.slots[1].human);
    var r: Rand = .{};
    s.slots[3].ruler = 12;
    const g = s.finish(&r, 1, .{ 10, 10, 5 }, &.{ 4, 7, 9 }, 4, true, true);
    try std.testing.expectEqual(@as(i32, 4), g.slots[0].ruler);
    try std.testing.expectEqual(@as(i32, 7), g.slots[1].ruler);
    try std.testing.expectEqual(@as(i32, 9), g.slots[2].ruler);
    try std.testing.expectEqual(@as(i32, 12), g.slots[3].ruler);
    try std.testing.expectEqual(@as(i32, 12), g.player_set[3]);
}

test "single player has no player types" {
    var s = Setup.init(defaultMap(), 0, .single, 5, 0);
    s.toggleKind(1);
    try std.testing.expectEqual(Kind.computer, s.slots[1].kind);
    try std.testing.expectEqual(@as(usize, 0), s.n_profiles);
}
