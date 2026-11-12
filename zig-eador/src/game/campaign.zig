//! The campaign state and its rules (Campaign.cpp, Unit.h:294..449).
//!
//! dcCampaign, dcCampPlayer, dcShard and dcTutorial are kept as extern
//! structs with the 32-bit layout of the GOG executable, so the campaign
//! part of a save is a plain copy (GameMenu.cpp:614, :1196).  Pointer
//! fields are stored as they were read and written back unchanged.
//!
//! Ported: Clear, the data part of NewCampaign (Campaign.cpp:1434),
//! MakeShards, the dcShard constructor, FillBonus, GetWorldSize,
//! GetBuildingStatus, IsBuildingAllowed, IsBuildingFree, IsParent (player
//! -1), AddBuild, GetAttitude, GetEnergyIncome, AddEvent, NoEvents,
//! BanishPlayer, AstralTurn, RunCampaignEvent as a resumable runner, and the
//! astral save (MODE 2).
//!
//! The GOG constructor of dcShard does not multiply the income by
//! arCampDiff[iDifficulty][4] (the C++ source does; the column is 0).
const std = @import("std");
const Rand = @import("../data/rand.zig").Rand;
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const profile = @import("../data/profile.zig");
const unit_mod = @import("unit.zig");
const Roller = unit_mod.Roller;
const event = @import("event.zig");
const cp1251 = @import("../formats/cp1251.zig");

/// MAX_PLAYERS, MAX_CAMP_QUEST, MAX_SHARD_BONUS, MAX_SHARD_EVENT,
/// MAX_SHARD_BUILD, DIALOG_ALLOWED.
pub const max_players = 17;
pub const camp_players = max_players + 16;
pub const max_camp_quest = 20;
pub const max_shard_bonus = 5;
pub const max_shard_event = 5;
pub const max_shard_build = 4;
pub const max_events = 50;
const dialog_allowed = 6;

/// The first bytes of a save (MODE 111, version 107) and the MODE of the
/// astral.
pub const save_magic = [2]u8{ 111, 107 };
pub const mode_astral = 2;

pub const CampPlayer = extern struct {
    energy: i32 = 0,
    energy_bonus: i32 = 0,
    energy_penalty: i32 = 0,
    energy_lose: i32 = 0,
    energy_drain: i32 = 0,
    size_bonus: i32 = 0,
    profile: i32 = 0,
    karma: i32 = 0,
    score: i32 = 0,
    treaty: i32 = 0,
    treaty1: i32 = 0,
    x: i32 = 0,
    y: i32 = 0,
    attitude: [max_players]i32 = @splat(0),
    bld: [proto_mod.max_bld]i32 = @splat(0),
    map_score: i32 = 0,
    quest_score: i32 = 0,
    dip_score: i32 = 0,
    kill_score: i32 = 0,
    first_meet: u8 = 0,
    shard_meet: u8 = 0,
    in_game: u8 = 0,
    _pad: u8 = 0,
    /// ShardPic, ShardMask.
    shard_pic: u32 = 0,
    shard_mask: u32 = 0,

    /// dcCampPlayer::Clear.
    pub fn clear(self: *CampPlayer, pr: *const Proto) void {
        for (pr.inner_buildings[1..], 1..) |b, i| self.bld[i] = @intFromBool(b.level <= 0);
        self.first_meet = 0;
        self.shard_meet = 0;
        self.in_game = 1;
        self.treaty = 0;
        self.treaty1 = 0;
        self.energy_bonus = 0;
        self.energy_penalty = 0;
        self.energy_lose = 0;
        self.energy_drain = 0;
        self.size_bonus = 0;
        self.score = 0;
        self.map_score = 0;
        self.quest_score = 0;
        self.dip_score = 0;
        self.kill_score = 0;
        self.profile = 0;
    }
};

/// clMap inside dcShard.
pub const MapSettings = extern struct {
    players: i32 = 4,
    difficulty: i32 = 0,
    x: i32 = 10,
    y: i32 = 10,
    type: i32 = 2,
    water: i32 = 25,
    hill: i32 = 25,
    forest: i32 = 20,
    swamp: i32 = 20,
    isles: i32 = 10,
    dissolve: i32 = 30,
    neutrals: i32 = 20,
    resource: i32 = 2,
    site: i32 = 2,
    population: i32 = 2,

    pub fn toMap(self: MapSettings) profile.Map {
        var m: profile.Map = undefined;
        inline for (@typeInfo(profile.Map).@"struct".field_names) |f| @field(m, f) = @field(self, f);
        return m;
    }
};

pub const Shard = extern struct {
    /// sName (cp1251, 30 bytes and the padding).
    name: [32]u8 = @splat(0),
    x: i32 = 0,
    y: i32 = 0,
    num: i32 = 0,
    level: i32 = 0,
    seed: i32 = 0,
    income: i32 = 0,
    terrain: i32 = 0,
    player: i32 = -1,
    /// (type, value).
    bonus: [max_shard_bonus][2]i32 = @splat(.{ 0, 0 }),
    event: [max_shard_event]i32 = @splat(0),
    building: [max_shard_build]i32 = @splat(0),
    map: MapSettings = .{},
    /// pNext, Pic, Select, UI_Mask Mask.
    next: u32 = 0,
    pic: u32 = 0,
    select: u32 = 0,
    mask: [28]u8 = @splat(0),

    /// sName as UTF-8 in `buf`.
    pub fn nameUtf8(self: *const Shard, buf: []u8) []const u8 {
        const end = std.mem.indexOfScalar(u8, &self.name, 0) orelse self.name.len;
        var n: usize = 0;
        for (self.name[0..end]) |b| {
            var tmp: [4]u8 = undefined;
            const len = std.unicode.utf8Encode(cp1251.toCodepoint(b), &tmp) catch continue;
            if (n + len > buf.len) break;
            @memcpy(buf[n..][0..len], tmp[0..len]);
            n += len;
        }
        return buf[0..n];
    }

    fn setName(self: *Shard, utf8: []const u8) void {
        @memset(&self.name, 0);
        var i: usize = 0;
        var at: usize = 0;
        while (at < utf8.len and i + 1 < 30) : (i += 1)
            self.name[i] = cp1251.fromCodepoint(cp1251.nextCodepoint(utf8, &at)) orelse '?';
    }
};

pub const Campaign = extern struct {
    current_turn: i32,
    active_shard: i32,
    difficulty: i32,
    max_shards: i32,
    time_bonus: i32,
    reserv: i32,
    /// 980.. rewards, 978 the turn limit, 979 the lowest difficulty.
    journal: [1000]i32,
    dialog: [2000]i32,
    items: [20]i32,
    events: [max_events]i32,
    /// (quest, state, province, site).
    quest: [max_camp_quest][4]i32,
    defender: u8,
    _pad: [3]u8,
    players: [camp_players]CampPlayer,
};

/// dcTutorial: two bools, then 55 counters and 16 button states.
pub const Tutorial = extern struct {
    mode: u8 = 0,
    map: u8 = 0,
    _pad: [2]u8 = .{ 0, 0 },
    v: [71]i32 = @splat(0),

    pub const F = enum(usize) {
        move, trade, hire, library, site, task, turn, mercenary, garrison, scroll, egg,
        prv_build, prv_guard, castles, ritual_hero, ritual_prov, ritual_cast, repair, death,
        battle, build, hero, population, resource, groups, spell_rank, unit_rank, stamina,
        ranged, heal, morale, ammo, orc, fortify, magic, bonus, alchemy, diplomacy, corrupt,
        quest, castle_defense, no_dip, dip_trade, exchange, war, peace, @"union", broken,
        squad_morale, lost_shard, defend_world, defend_castle, unit_price, add1, add2,
        butt_end, butt_castle, butt_stat, butt_diplo, butt_next, butt_build, butt_guard,
        butt_enter, butt_ritual, butt_rit_hero, butt_shop, butt_library, butt_garrison,
        butt_pillage, butt_attack, butt_explore,
    };

    pub fn get(self: *const Tutorial, f: F) i32 {
        return self.v[@intFromEnum(f)];
    }

    pub fn set(self: *Tutorial, f: F, x: i32) void {
        self.v[@intFromEnum(f)] = x;
    }

    /// dcTutorial::Init.
    pub fn init(self: *Tutorial) void {
        self.* = .{ .mode = 1, .map = 1 };
        const ones = [_]F{ .move, .battle, .stamina, .ranged, .heal, .morale, .ammo, .orc, .magic, .broken, .butt_next };
        for (ones) |f| self.set(f, 1);
        self.set(.site, -1);
    }
};

comptime {
    std.debug.assert(@sizeOf(CampPlayer) == 0x1034);
    std.debug.assert(@sizeOf(Shard) == 0xF0);
    std.debug.assert(@sizeOf(Campaign) == 0x24808);
    std.debug.assert(@sizeOf(Tutorial) == 0x120);
}

fn rnd(r: *Rand, n: i32) i32 {
    // Random takes an unsigned long.
    return @bitCast(r.random(@bitCast(n)));
}

pub const State = struct {
    gpa: std.mem.Allocator,
    camp: *Campaign,
    /// pFirstShard first.
    shards: std.ArrayList(Shard) = .empty,
    tutorial: Tutorial = .{},
    /// CurGame.iCurrentTurn: -1 in the astral.
    turn: i32 = 0,
    /// bInvasion, bJournal, bTurn.
    invasion: bool = false,
    journal: bool = false,
    attack_turn: bool = false,

    /// The constructor of dcCampaign.
    pub fn create(gpa: std.mem.Allocator, pr: *const Proto) !*State {
        const self = try gpa.create(State);
        errdefer gpa.destroy(self);
        const camp = try gpa.create(Campaign);
        camp.* = std.mem.zeroes(Campaign);
        self.* = .{ .gpa = gpa, .camp = camp };
        self.clear(pr);
        return self;
    }

    pub fn destroy(self: *State) void {
        self.shards.deinit(self.gpa);
        self.gpa.destroy(self.camp);
        self.gpa.destroy(self);
    }

    /// dcCampaign::Clear (iActiveShard and the event queue stay).
    pub fn clear(self: *State, pr: *const Proto) void {
        const c = self.camp;
        c.current_turn = 0;
        c.max_shards = 0;
        c.difficulty = 0;
        c.time_bonus = 0;
        c.reserv = 0;
        self.shards.clearRetainingCapacity();
        c.journal = @splat(0);
        c.journal[980] = -1;
        c.journal[979] = -1;
        c.journal[978] = pr.camp_diff[0][3];
        c.dialog = @splat(0);
        for (c.dialog[0 .. dialog_allowed + 1]) |*d| d.* = 1;
        c.items = @splat(0);
        c.quest = @splat(@splat(0));
    }

    pub fn player(self: *State, k: anytype) *CampPlayer {
        return &self.camp.players[@intCast(k)];
    }

    fn validPlayer(k: i32) bool {
        return k >= 0 and k < camp_players;
    }

    /// The shard of building 17 (the first one) is the player's home.
    pub fn shardIndex(self: *const State, num: i32) ?usize {
        for (self.shards.items, 0..) |sh, i| if (sh.num == num) return i;
        return null;
    }
};

/// NewCampaign up to Tutorial.Init: the rulers' places and attitudes and
/// the first shards.  The caller seeds `r` (srand(iSeed)).
pub fn newCampaign(st: *State, pr: *const Proto, r: *Rand) !void {
    st.clear(pr);
    var ax = [3]i32{ 2, -1, -1 };
    for (0..max_players) |i| {
        const p = st.player(i);
        const pp = &pr.player_profiles[i];
        p.clear(pr);
        p.energy = 100;
        p.profile = @intCast(i);
        p.karma = pp.karma;
        p.x = 0;
        while (p.x == 0) {
            var nx = rnd(r, 5);
            for (ax) |v| if (nx == v) {
                nx = -1;
            };
            if (nx >= 0) {
                p.x = nx * 200 + 20;
                ax[2] = ax[1];
                ax[1] = nx;
            }
        }
        p.y = (pp.y - 1) * 50;
        p.score = 0;
        p.attitude = pp.attitude;
        p.first_meet = 0;
        p.in_game = 1;
    }
    const p0 = st.player(0);
    p0.first_meet = 1;
    p0.x = 512;
    p0.y = 50;
    for (pr.inner_buildings[1..], 1..) |b, i| if (b.level <= 0) {
        p0.bld[i] = 1;
    };
    try makeShards(st, pr, r, 0);
    for (pr.inner_buildings[1..], 1..) |b, i| if (b.level < 0) {
        p0.bld[i] = 0;
    };
    st.tutorial.init();
}

/// GetWorldSize.
pub fn worldSize(st: *State, pr: *const Proto, k: usize) i32 {
    const p = st.player(k);
    var result = p.size_bonus;
    if (k == 0) {
        for (st.shards.items) |sh| result += @intFromBool(sh.player == 0);
        return result;
    }
    var n: i32 = 0;
    for (pr.inner_buildings[1..], 1..) |b, i| n += @intFromBool(p.bld[i] != 0 and b.level > 0);
    return @divTrunc(n, 3) - result;
}

fn shardHas(st: *const State, k: i32, b: usize) bool {
    for (st.shards.items) |sh| if (sh.player == k) {
        for (sh.building) |x| if (x == b) return true;
    };
    return false;
}

/// GetBuildingStatus: 0 not available, 1 available (or on a neutral
/// shard), 2 the player has it.
pub fn buildingStatus(st: *State, pr: *const Proto, b: usize, k: usize) i32 {
    const ib = &pr.inner_buildings[b];
    if (ib.level < 1) return 2;
    if (st.player(k).bld[b] > 0) return 2;
    if (shardHas(st, @intCast(k), b)) return 2;
    for (ib.needs) |need| {
        if (need > 0 and need < 1000) {
            if (buildingStatus(st, pr, @intCast(need), k) < 2) return 0;
        }
        if (need > 1000) {
            const g = need - 1000;
            var sum: i32 = 0;
            for (pr.inner_buildings[1..], 1..) |x, bl| {
                if (x.group != g) continue;
                if (x.level < 1 or st.player(k).bld[bl] > 0 or shardHas(st, @intCast(k), bl)) sum += 1;
            }
            if (g < pr.build_groups.len and sum < pr.build_groups[@intCast(g)].max) return 0;
        }
    }
    return 1;
}

/// IsBuildingAllowed.
fn buildingAllowed(st: *State, pr: *const Proto, b: usize, k: usize) i32 {
    const ib = &pr.inner_buildings[b];
    if (ib.level < 1) return 2;
    if (st.player(k).bld[b] > 0) return 2;
    for (ib.needs) |need| if (need > 0 and need < 1000) {
        if (buildingAllowed(st, pr, @intCast(need), k) < 2) return 0;
    };
    return 1;
}

/// IsBuildingFree: no neutral shard (and none of the player's) offers it.
pub fn isBuildingFree(st: *const State, b: usize) bool {
    for (st.shards.items) |sh| if (sh.player < 1) {
        for (sh.building) |x| if (x == b) return false;
    };
    return true;
}

/// IsParent with Player -1 (GroupIsFull is false): 1 when `son` needs
/// `father`, 2 through a group.
fn isParent(pr: *const Proto, father: i32, son: i32) i32 {
    for (pr.inner_buildings[@intCast(son)].needs) |need| {
        if (need <= 0) continue;
        if (need < 1000) {
            if (need == father or isParent(pr, father, need) != 0) return 1;
        } else for (pr.inner_buildings[1..], 1..) |x, j| {
            if (x.group != need - 1000) continue;
            if (j == father or isParent(pr, father, @intCast(j)) != 0) return 2;
        }
    }
    return 0;
}

/// dcCampPlayer::AddBuild for player `k`.
pub fn addBuild(st: *State, pr: *const Proto, r: *Rand, k: usize) void {
    const p = st.player(k);
    const prof: usize = @intCast(p.profile);
    var min_lvl: i32 = 20;
    const pp: ?*const proto_mod.PlayerProfile = if (prof < 16 and prof < pr.player_profiles.len) &pr.player_profiles[prof] else null;
    if (pp) |x| for (x.buildings) |b| {
        if (b > 0 and p.bld[@intCast(b)] == 0 and pr.inner_buildings[@intCast(b)].level < min_lvl) min_lvl = pr.inner_buildings[@intCast(b)].level;
    };
    var rl: Roller = .{};
    var n: usize = 0;
    for (1..pr.inner_buildings.len) |i| {
        if (buildingAllowed(st, pr, i, prof) != 1) continue;
        rl.ids[n] = @intCast(i);
        rl.weights[n] = 10;
        if (pp) |x| {
            for (x.buildings) |b| {
                if (b <= 0) continue;
                if (b == i) {
                    rl.weights[n] += 100;
                } else if (pr.inner_buildings[@intCast(b)].level == min_lvl and isParent(pr, @intCast(i), b) != 0) rl.weights[n] += 20;
            }
            if (x.karma != 0) {
                const ib = &pr.inner_buildings[i];
                for (ib.abilities[0..ib.n_abilities]) |a| if (a.id == 46) {
                    if (std.math.sign(x.karma) == std.math.sign(a.p1)) rl.weights[n] *= 2 else rl.weights[n] = @divTrunc(rl.weights[n], 3);
                };
            }
        }
        n += 1;
    }
    if (n > 0) p.bld[@intCast(rl.roll(r))] = 1;
}

/// dcCampPlayer::GetAttitude.
pub fn attitude(st: *State, k: usize) i32 {
    const p = st.player(k);
    const k0 = st.player(0).karma;
    var result = p.attitude[0];
    if (p.karma >= 20) result += @divTrunc(@divTrunc(k0, 10) * @divTrunc(p.karma, 10), 5);
    if (p.karma <= -20) result += @divTrunc(@divTrunc(k0, 10) * @divTrunc(p.karma, 10), 10);
    if (@abs(k0) < 20 and @abs(p.karma) < 20) result += 5 - @as(i32, @intCast(@divTrunc(@abs(k0 - p.karma), 10)));
    for (1..max_players) |i| {
        const o = st.player(i);
        if (i != p.profile and o.in_game != 0) result += @divTrunc(o.attitude[0], 10) * @divTrunc(p.attitude[i], 50);
    }
    return result;
}

/// GetEnergyIncome.
pub fn energyIncome(st: *State) i32 {
    var result: i32 = 0;
    for (st.shards.items) |sh| if (sh.player == 0) {
        result += sh.income;
    };
    const p0 = st.player(0);
    result -= @divTrunc(result * p0.energy_lose, 100);
    result -= p0.energy_drain;
    return @max(result, 0);
}

/// AddEvent.
pub fn addEvent(st: *State, index: i32) void {
    for (st.camp.events) |e| if (e == index) return;
    for (&st.camp.events) |*e| if (e.* == 0) {
        e.* = index;
        return;
    };
}

pub fn noEvents(st: *const State) bool {
    for (st.camp.events) |e| if (e > 0) return false;
    return true;
}

/// BanishPlayer.
pub fn banish(st: *State, pr: *const Proto, k: usize) void {
    const pp = &pr.player_profiles[k];
    for (pp.exit_dialogs) |d| if (d > 0) {
        st.camp.dialog[@intCast(d)] = 0;
    };
    st.player(k).in_game = 0;
    if (k == 5 or k == 14) st.camp.dialog[646] = 1;
    for (1..max_players) |i| {
        if (i == k) continue;
        const a = pr.player_profiles[i].attitude[k];
        if (a > 0) st.player(i).attitude[1] -= @divTrunc(a, 5);
        if (a < 0) st.player(i).attitude[1] -= @divTrunc(a, 10);
    }
}

/// AstralTurn.
pub fn astralTurn(st: *State, pr: *const Proto, r: *Rand, skip: bool) void {
    const c = st.camp;
    c.dialog[79] = 0;
    const p0 = st.player(0);
    p0.energy += energyIncome(st);
    p0.energy += p0.energy_bonus + p0.energy_penalty;
    if (skip) for (1..max_players) |i| {
        if (st.player(i).first_meet != 0 and rnd(r, 2 + c.difficulty) > 0) addBuild(st, pr, r, i);
    };
    for (pr.campaign_events[1..], 1..) |ev, i| {
        if (ev.attacker > 0 and ev.attacker < c.dialog.len and c.dialog[@intCast(ev.attacker)] > 0) {
            addEvent(st, @intCast(i));
            c.dialog[@intCast(ev.attacker)] = 0;
        }
    }
    if (c.dialog[38] > 1) c.dialog[38] -= 1;
    if (c.dialog[44] > 1) {
        c.dialog[44] -= 1;
    } else if (c.dialog[42] > 0) c.dialog[43] = 1;
    c.current_turn += 1;
}

/// The dcShard constructor; the shard goes first in the list.
fn newShard(st: *State, pr: *const Proto, r: *Rand, terra: i32, bld: i32, lvl: i32) !void {
    var sh: Shard = .{};
    while (true) {
        const taken = for (st.shards.items) |x| {
            if (x.num == sh.num) break true;
        } else false;
        if (!taken) break;
        sh.num += 1;
    }
    st.camp.max_shards += 1;
    sh.level = lvl;
    sh.income = lvl + rnd(r, lvl * 2);
    const suf = rnd(r, @intCast(pr.suffixes.len));
    const pref = rnd(r, @intCast(pr.prefixes.len));
    var nb: [96]u8 = undefined;
    sh.setName(std.fmt.bufPrint(&nb, "{s}{s}", .{ pr.prefixes[@intCast(pref)], pr.suffixes[@intCast(suf)] }) catch "");
    sh.player = -1;
    sh.terrain = terra;
    sh.seed = rnd(r, 10000);
    const m = &sh.map;
    m.type = 2;
    m.water = 10 + rnd(r, 5);
    m.hill = 12 + rnd(r, 10);
    m.forest = 7 + rnd(r, 10);
    m.swamp = 7 + rnd(r, 10);
    m.isles = 5 + rnd(r, 10);
    m.dissolve = 20 + rnd(r, 20);
    m.neutrals = if (lvl > 6) 20 else 1 + (lvl - 1) * 2;
    switch (terra) {
        0 => {
            m.water += 20;
            m.type = 1;
        },
        2 => m.forest += 20,
        3 => m.hill += 20,
        4 => m.swamp += 20,
        5 => {
            m.water += 3;
            m.hill += 3;
            m.forest += 3;
            m.swamp += 3;
        },
        else => {},
    }
    m.players = 2 + @divTrunc(lvl - 1 - @rem(lvl - 1, 2), 2);
    m.x = 6 + @divTrunc(lvl - @rem(lvl, 2), 2) * 2;
    m.y = m.x;
    if (bld == 4) {
        m.players += 1;
        m.x += 2;
        m.y += 2;
        m.neutrals += 1;
    }
    try st.shards.insert(st.gpa, 0, sh);
}

fn overlaps(st: *const State) bool {
    const f = st.shards.items[0];
    for (st.shards.items[1..]) |sh| {
        if (@abs(sh.x - f.x) < 50 and @abs(sh.y - f.y) < 50) return true;
    }
    return false;
}

/// The placement past the owned shards (Own > 0), next to the rulers.
fn placeOuter(st: *State, pr: *const Proto, r: *Rand, own: i32) void {
    var tries: i32 = 0;
    while (true) {
        tries += 1;
        const f = &st.shards.items[0];
        f.y = 768 + (own - 1) * 50 - rnd(r, 50);
        f.x = 20 + rnd(r, 900);
        var need = overlaps(st);
        const add_y = (worldSize(st, pr, 0) - 1) * 50;
        var i: usize = 1;
        while (i < max_players and !need and tries < 1000) : (i += 1) {
            const p = st.player(i);
            if (p.y > add_y - 50 and p.in_game != 0 and p.y < add_y + 100 and p.x - 60 < f.x and p.x + 130 > f.x) need = true;
        }
        if (!need) return;
    }
}

/// MakeShards.
pub fn makeShards(st: *State, pr: *const Proto, r: *Rand, k: usize) !void {
    const max_bld = pr.inner_buildings.len - 1;
    var allow: i32 = 0;
    var own: i32 = 0;
    var build: i32 = 0;
    if (st.shards.items.len > 0) {
        for (1..max_bld + 1) |i| {
            const s = buildingStatus(st, pr, i, k);
            if (s == 1 and isBuildingFree(st, i)) allow += 1;
            if (s == 2) build += 1;
        }
        allow = if (allow < 3) 0 else if (allow < 10) 1 else if (allow < 20) 2 else 3;
        for (st.shards.items) |sh| own += @intFromBool(sh.player == 0);
    } else {
        allow = 5;
        try newShard(st, pr, r, 5, 2, 1);
        const f = &st.shards.items[0];
        f.building[0] = 17;
        f.building[1] = 19;
        f.x = 480;
        f.y = 100;
        f.bonus[0] = .{ 1, 10 };
        f.income = 10;
    }
    if (allow > 0) {
        var lvl: i32 = 1;
        while (lvl < 10) : (lvl += 1) {
            var count: i32 = 0;
            var rl: Roller = .{};
            for (1..max_bld + 1) |i| {
                if (pr.inner_buildings[i].level <= lvl and buildingStatus(st, pr, i, k) == 1 and isBuildingFree(st, i)) {
                    rl.ids[@intCast(count)] = @intCast(i);
                    rl.weights[@intCast(count)] = 10;
                    count += 1;
                }
            }
            while (count > 2 and allow > 0) {
                const bld = @min(rnd(r, 2) + 3, count);
                count -= bld;
                allow -= 1;
                try newShard(st, pr, r, rnd(r, 6), bld, lvl);
                for (0..@intCast(bld)) |i| st.shards.items[0].building[i] = rl.rollExclude(r);
                if (own > 0) {
                    placeOuter(st, pr, r, own);
                } else while (true) {
                    const f = &st.shards.items[0];
                    f.y = 150 + rnd(r, 610);
                    f.x = 100 + rnd(r, 780);
                    var need = f.x > 600 and f.y < 500;
                    if (!need) need = overlaps(st);
                    if (f.y > 568 - 50 and f.y < 568 + 100 and f.x > 400 - 50 and f.x < 400 + 100) need = true;
                    if (!need) break;
                }
            }
        }
    } else if (build >= @as(i32, @intCast(max_bld)) - 3) {
        // The shards after all the buildings.  MSVC evaluates the
        // arguments right to left.
        const n = 1 + rnd(r, 3);
        for (0..@intCast(n)) |_| {
            const bld = 3 + rnd(r, 2);
            try newShard(st, pr, r, rnd(r, 6), bld, 8);
            if (build < max_bld) {
                build = @intCast(max_bld);
                var num: usize = 0;
                for (1..max_bld + 1) |i| {
                    if (buildingStatus(st, pr, i, k) == 1 and isBuildingFree(st, i) and num < 4) {
                        st.shards.items[0].building[num] = @intCast(i);
                        num += 1;
                    }
                }
            }
            placeOuter(st, pr, r, own);
        }
    }
    for (0..st.shards.items.len) |i| if (st.shards.items[i].level > 0) fillBonus(st, pr, r, i);
}

/// dcShard::FillBonus.
fn fillBonus(st: *State, pr: *const Proto, r: *Rand, idx: usize) void {
    const sh = &st.shards.items[idx];
    if (sh.bonus[0][0] != 0) return;
    const lvl = sh.level;
    const li: usize = @intCast(std.math.clamp(lvl - 1, 0, proto_mod.max_shard_level - 1));
    const n: usize = @intCast(@min(1 + @divTrunc(lvl, 8) + rnd(r, 2 + @divTrunc(lvl - 1, 3)), 4));
    var bl: [8]bool = @splat(true);
    var res: [9]bool = @splat(true);
    for (st.shards.items) |x| for (x.bonus) |b| {
        if (b[0] == 7 and b[1] >= 1 and b[1] < 8) bl[@intCast(b[1] - 1)] = false;
        if (b[0] == 8 and b[1] >= 1 and b[1] <= 9) res[@intCast(b[1] - 1)] = false;
    };
    const can_build = std.mem.indexOfScalar(bool, &bl, true) != null;
    var can_res = false;
    if (pr.shard_bonuses.len > 8) for (res, 0..) |ok, i| {
        if (!ok) continue;
        for (pr.shard_bonuses[8].levels[li]) |p| if (p.id == i + 1) {
            can_res = true;
        };
    };
    var rl: Roller = .{};
    var cnt: usize = 0;
    for (pr.shard_bonuses[1..], 1..) |b, i| {
        if (b.levels[li][0].id > 0 and (i != 7 or can_build) and (i != 8 or can_res)) {
            rl.ids[cnt] = @intCast(i);
            rl.weights[cnt] = 5 - b.rarity;
            cnt += 1;
        }
    }
    for (0..n) |i| sh.bonus[i][0] = rl.rollExclude(r);
    for (0..n) |i| {
        const t = sh.bonus[i][0];
        if (t <= 0 or t >= pr.shard_bonuses.len) continue;
        const b = &pr.shard_bonuses[@intCast(t)];
        const lv = b.levels[li];
        if (b.kind == 0) {
            sh.bonus[i][1] = lv[0].id + rnd(r, 1 + lv[0].value - lv[0].id);
        } else {
            var r2: Roller = .{};
            var c2: usize = 0;
            for (lv) |p| {
                if (p.id <= 0) continue;
                if (t == 7 and (p.id > 8 or !bl[@intCast(p.id - 1)])) continue;
                if (t == 8 and (p.id > 9 or !res[@intCast(p.id - 1)])) continue;
                r2.ids[c2] = p.id;
                r2.weights[c2] = p.value;
                c2 += 1;
            }
            sh.bonus[i][1] = r2.roll(r);
        }
    }
}

/// RunCampaignEvent (Campaign.cpp:916) as a runner: `step` applies the
/// effects until a dialog is needed and returns it; `answer` hands the
/// choice back.  The recursion of the original is a stack of frames: most
/// calls replace the frame (they end with return), the question dialogs
/// push one and go on with the transitions of their own event.
pub const Runner = struct {
    frames: [max_depth]Frame = undefined,
    n: usize = 0,
    text_buf: [1000]u8 = undefined,

    const max_depth = 32;

    const Stage = enum { begin, wait, transitions };

    const Frame = struct {
        index: i32,
        player: i32,
        param: i32,
        mult: i32 = 0,
        stage: Stage = .begin,
        question: bool = false,
        q_events: [4]i32 = @splat(0),
    };

    /// A dialog of campaign_dialog.var.  `codes` are Show's A1..A4: 1/0
    /// for an answer, in a question window the question (> 0) or the
    /// dialog's own answer (< 0).
    pub const Request = struct {
        dialog: usize,
        codes: [4]i32,
        question: bool,
        /// sNewText of a normal dialog.
        text: []const u8,
    };

    pub fn busy(self: *const Runner) bool {
        return self.n > 0;
    }

    pub fn abort(self: *Runner) void {
        self.n = 0;
    }

    /// RunCampaignEvent(index, player, param).
    pub fn start(self: *Runner, index: i32, k: i32, param: i32) void {
        self.n = 0;
        self.push(.{ .index = index, .player = k, .param = param });
    }

    fn push(self: *Runner, f: Frame) void {
        if (self.n == max_depth) {
            std.log.warn("campaign event {d}: too deep", .{f.index});
            self.frames[self.n - 1] = f;
            return;
        }
        self.frames[self.n] = f;
        self.n += 1;
    }

    fn top(self: *Runner) *Frame {
        return &self.frames[self.n - 1];
    }

    /// A tail call: the event ends with RunCampaignEvent(...); return.
    fn tail(self: *Runner, index: i32) void {
        const f = self.top();
        f.* = .{ .index = index, .player = f.player, .param = f.param };
    }

    /// `name` is arHumans[0].sName for the dialog texts.
    pub fn step(self: *Runner, st: *State, pr: *const Proto, r: *Rand, name: []const u8) ?Request {
        var guard: usize = 0;
        while (self.n > 0) : (guard += 1) {
            if (guard > 10_000) {
                std.log.warn("campaign event loop", .{});
                self.n = 0;
                return null;
            }
            const f = self.top();
            if (f.index <= 0 or f.index >= pr.campaign_events.len) {
                self.n -= 1;
                continue;
            }
            const ev = &pr.campaign_events[@intCast(f.index)];
            switch (f.stage) {
                .wait => return null,
                .begin => {
                    if (self.effects(st, pr, r, ev)) continue;
                    if (ev.dialog > 0 and ev.dialog < pr.campaign_dialogs.len) {
                        f.stage = .wait;
                        return self.dialog(st, pr, ev, name);
                    }
                    f.stage = .transitions;
                },
                .transitions => if (!self.transitions(st, pr, r, ev)) {
                    self.n -= 1;
                },
            }
        }
        return null;
    }

    /// The individual effects; true when effect 19 replaced the event.
    fn effects(self: *Runner, st: *State, pr: *const Proto, r: *Rand, ev: *const proto_mod.Event) bool {
        const f = self.top();
        const c = st.camp;
        st.player(0).karma += ev.karma;
        f.mult = 0;
        for (ev.effects, 0..) |e, k| {
            switch (e.id) {
                1 => if (State.validPlayer(e.p2)) {
                    const w = e.power +% rnd(r, e.p1);
                    st.player(e.p2).energy += w;
                    if (k == 0) f.mult = w;
                },
                2 => {
                    const who = if (e.p1 == 0) f.player else e.p1;
                    if (State.validPlayer(who) and e.p2 >= 0 and e.p2 < max_players) st.player(who).attitude[@intCast(e.p2)] += e.power;
                },
                3 => if (e.power >= 0 and e.power < c.dialog.len and e.p2 >= 0 and e.p2 < c.dialog.len) {
                    if (e.p2 == 0 or c.dialog[@intCast(e.p2)] > 0) c.dialog[@intCast(e.power)] = e.p1;
                },
                5 => if (State.validPlayer(e.p2)) {
                    const w = e.power +% rnd(r, e.p1);
                    if (w > 0) st.player(e.p2).energy_bonus += w else st.player(e.p2).energy_penalty += w;
                    if (k == 0) f.mult = w;
                },
                6 => f.param = e.power,
                7 => if (State.validPlayer(e.p2)) {
                    st.player(e.p2).karma += e.power +% rnd(r, e.p1);
                },
                12 => if (State.validPlayer(e.power)) {
                    st.player(e.power).treaty = e.p1;
                },
                13 => {
                    var found = false;
                    for (&c.quest) |*q| if (q[0] == e.power) {
                        q[1] = e.p1;
                        found = true;
                    };
                    if (!found) for (&c.quest) |*q| if (q[0] == 0) {
                        q[0] = e.power;
                        q[1] = e.p1;
                        break;
                    };
                },
                14 => st.invasion = e.power != 0,
                15 => if (e.power >= 0 and e.power < c.items.len) {
                    c.items[@intCast(e.power)] += e.p1;
                    if (e.p2 > 0 and e.p2 < c.items.len) c.items[@intCast(e.power)] += c.items[@intCast(e.p2)];
                },
                // The shift of the queue never runs (i = 48; i <= 0).
                17 => c.events[0] = e.power,
                18 => if (e.power > 0 and e.power < max_players) banish(st, pr, @intCast(e.power)),
                19 => if (State.validPlayer(e.p1) and st.player(e.p1).in_game != 0) {
                    self.tail(e.power);
                    return true;
                },
                21 => {
                    st.attack_turn = true;
                    std.log.warn("campaign event {d}: PickBonusScreen(NULL, {d}) is not ported", .{ f.index, -e.power });
                },
                22 => {
                    const w = e.power +% rnd(r, e.p1);
                    st.player(0).energy_lose += w;
                    if (k == 0) f.mult = w;
                },
                23 => if (e.power >= 0 and e.power < c.journal.len) {
                    c.journal[@intCast(e.power)] = e.p1;
                    if (e.p1 > 0) st.journal = true;
                },
                24 => c.time_bonus += e.power,
                25 => st.player(0).map_score += e.power,
                else => {},
            }
        }
        return false;
    }

    fn dialog(self: *Runner, st: *State, pr: *const Proto, ev: *const proto_mod.Event, name: []const u8) Request {
        const f = self.top();
        const d = &pr.campaign_dialogs[@intCast(ev.dialog)];
        var req: Request = .{ .dialog = @intCast(ev.dialog), .codes = @splat(0), .question = false, .text = "" };
        if (ev.dlg_param1 >= 10) {
            req.question = true;
            f.question = true;
            var codes: [4]i32 = @splat(0);
            var evs: [4]i32 = @splat(0);
            const base = f.param * 3;
            var count: i32 = 0;
            for (ev.effects) |e| if (e.id == 8) {
                for (pr.questions, 0..) |q, i| {
                    if (q.quester != e.power) continue;
                    if (q.trigger < 0 or q.trigger >= st.camp.dialog.len or st.camp.dialog[@intCast(q.trigger)] <= 0) continue;
                    if (count >= base and count < base + 4) {
                        codes[@intCast(count - base)] = @intCast(i + 1);
                        evs[@intCast(count - base)] = q.event;
                    }
                    count += 1;
                }
                break;
            };
            if (count < base + 4 and count >= base) {
                codes[@intCast(count - base)] = -2;
                evs[@intCast(count - base)] = 0;
            }
            if (count > base + 4) {
                codes[3] = -1;
                evs[3] = f.index;
                f.param += 1;
            }
            req.codes = codes;
            f.q_events = evs;
            return req;
        }
        f.question = false;
        const args: []const event.Arg = switch (ev.dlg_param1) {
            1 => if (ev.dlg_param2 == 2) &.{ .{ .s = name }, .{ .d = f.mult } } else &.{.{ .s = name }},
            2 => if (ev.dlg_param2 == 1) &.{ .{ .d = f.mult }, .{ .s = name } } else &.{.{ .d = f.mult }},
            else => &.{},
        };
        req.text = event.format(&self.text_buf, d.text, args);
        const c = st.camp;
        for (0..proto_mod.max_answer) |i| {
            if (i >= d.n_answers) continue;
            const a = d.answers[i];
            if (a.len > 1 and a[1] == '#') continue;
            var on = true;
            for (ev.effects) |e| if (e.id == 4 and e.power == i + 1) {
                const flag = if (e.p2 >= 0 and e.p2 < c.dialog.len) c.dialog[@intCast(e.p2)] else 0;
                switch (e.p1) {
                    1 => if (st.player(0).energy < e.p2) {
                        on = false;
                    },
                    2 => if (flag == 0) {
                        on = false;
                    },
                    3 => if (c.items[1] < 1) {
                        on = false;
                    },
                    4 => if (c.items[0] < 2) {
                        on = false;
                    },
                    5 => if (flag != 0) {
                        on = false;
                    },
                    else => {},
                }
            };
            req.codes[i] = @intFromBool(on);
        }
        return req;
    }

    /// The 1-based answer of the dialog `step` returned.
    pub fn answer(self: *Runner, pr: *const Proto, a: usize) void {
        if (self.n == 0) return;
        const f = self.top();
        if (f.stage != .wait) return;
        const k = std.math.clamp(a, 1, 4) - 1;
        if (f.question) {
            const nx = f.q_events[k];
            f.stage = .transitions;
            if (nx > 0) self.push(.{ .index = nx, .player = f.player, .param = f.param });
            return;
        }
        const nx = pr.campaign_events[@intCast(f.index)].answers[k];
        if (nx > 0) self.tail(nx) else f.stage = .transitions;
    }

    /// The transitions after the dialog; false when the event is over.
    fn transitions(self: *Runner, st: *State, pr: *const Proto, r: *Rand, ev: *const proto_mod.Event) bool {
        const c = st.camp;
        for (ev.effects) |e| {
            if (e.id == 9 and State.validPlayer(e.power) and attitude(st, @intCast(e.power)) < e.p1) {
                self.tail(e.p2);
                return true;
            }
            if (e.id == 10 and e.power >= 0 and e.power < c.dialog.len) {
                if (c.dialog[@intCast(e.power)] > 0) {
                    if (e.p1 > 0) {
                        self.tail(e.p1);
                        return true;
                    }
                } else if (e.p2 > 0) {
                    self.tail(e.p2);
                    return true;
                }
            }
            if (e.id == 11) {
                if (e.p1 > 0 and rnd(r, 100) > e.p2) self.tail(e.p1) else self.tail(e.power);
                return true;
            }
            if (e.id == 26) {
                if (st.player(0).map_score >= e.power) {
                    if (e.p1 > 0) {
                        self.tail(e.p1);
                        return true;
                    }
                } else if (e.p2 > 0) {
                    self.tail(e.p2);
                    return true;
                }
            }
            if (e.id == 20) {
                std.log.warn("FinishCampaign({d}) is not ported", .{e.power});
                return false;
            }
        }
        _ = pr;
        return false;
    }
};

/// SaveGame(0) in the astral (MODE 2): the magic, the mode, Campaign, the
/// shards from pFirstShard and Tutorial; unpacked.
pub fn astralBytes(gpa: std.mem.Allocator, st: *const State) ![]u8 {
    var out: std.ArrayList(u8) = .empty;
    errdefer out.deinit(gpa);
    try out.appendSlice(gpa, &save_magic);
    try out.append(gpa, mode_astral);
    try out.appendSlice(gpa, std.mem.asBytes(st.camp));
    const n = @min(@as(usize, @intCast(@max(st.camp.max_shards, 0))), st.shards.items.len);
    for (st.shards.items[0..n]) |*sh| try out.appendSlice(gpa, std.mem.asBytes(sh));
    try out.appendSlice(gpa, std.mem.asBytes(&st.tutorial));
    return out.toOwnedSlice(gpa);
}

/// The campaign blocks of a save: Campaign, iMaxShards shards and Tutorial.
/// LoadGame links each shard before the previous one, so the list comes
/// out reversed.
pub fn readBlocks(st: *State, data: []const u8) !void {
    const cs = @sizeOf(Campaign);
    if (data.len < cs) return error.ShortCampaign;
    @memcpy(std.mem.asBytes(st.camp), data[0..cs]);
    const n: usize = @intCast(@max(st.camp.max_shards, 0));
    const ss = @sizeOf(Shard);
    if (data.len != cs + n * ss + @sizeOf(Tutorial)) return error.BadCampaignSize;
    st.shards.clearRetainingCapacity();
    for (0..n) |i| {
        var sh: Shard = undefined;
        @memcpy(std.mem.asBytes(&sh), data[cs + i * ss ..][0..ss]);
        try st.shards.insert(st.gpa, 0, sh);
    }
    @memcpy(std.mem.asBytes(&st.tutorial), data[cs + n * ss ..][0..@sizeOf(Tutorial)]);
}

/// An unpacked game%d.cmp: its MODE (1 a shard, 2 the astral).
pub fn saveMode(data: []const u8) !u8 {
    if (data.len < 3 or !std.mem.eql(u8, data[0..2], &save_magic)) return error.NotACampaignSave;
    return data[2];
}

/// LoadGame(2) of an astral save, with the fix-up after it.
pub fn loadAstral(st: *State, pr: *const Proto, data: []const u8) !void {
    if (try saveMode(data) != mode_astral) return error.NotAnAstralSave;
    try readBlocks(st, data[3..]);
    st.turn = -1;
    if (st.tutorial.map == 0) for (0..max_players) |i| {
        for (pr.inner_buildings[1..], 1..) |b, j| if (b.level <= 0) {
            st.player(i).bld[j] = 1;
        };
    };
}

const tst = std.testing;
const GameFs = @import("../platform/fs.zig").GameFs;
const packfile = @import("../formats/packfile.zig");
const savefile = @import("../formats/savefile.zig");
const mapfile = @import("../formats/mapfile.zig");

fn readTestFile(gpa: std.mem.Allocator, path: []const u8) ![]u8 {
    const bytes = std.Io.Dir.cwd().readFileAlloc(tst.io, path, gpa, .unlimited) catch return error.SkipZigTest;
    defer gpa.free(bytes);
    return packfile.unpack(gpa, bytes);
}

// testdata/tutorial was saved on the first campaign map: its shards are
// the ones NewCampaign made from the seed of the header.
test "new campaign shards match the original" {
    const gpa = tst.allocator;
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
    const names = try mapfile.provinceNames(gpa, mdata);
    defer gpa.free(names);
    var save = try savefile.parse(gpa, gdata, names.len);
    defer save.deinit(gpa);
    try tst.expectEqual(@as(u8, 1), try saveMode(gdata));

    // The tail: pRitual (count, 0x18 each), then the campaign blocks.
    const tail = save.tail;
    const rituals: usize = @intCast(std.mem.readInt(i32, tail[0..4], .little));
    const ref = try State.create(gpa, &pr);
    defer ref.destroy();
    try readBlocks(ref, tail[4 + rituals * 0x18 ..]);
    try tst.expectEqual(@as(u8, 1), ref.tutorial.map);

    const st = try State.create(gpa, &pr);
    defer st.destroy();
    var r: Rand = .{};
    r.seed(@bitCast(std.mem.readInt(i32, gdata[3..][0..4], .little)));
    try newCampaign(st, &pr, &r);
    try tst.expectEqual(ref.camp.max_shards, st.camp.max_shards);
    for (0..max_players) |i| {
        try tst.expectEqual(ref.player(i).x, st.player(i).x);
        try tst.expectEqual(ref.player(i).y, st.player(i).y);
    }
    try tst.expectEqual(ref.shards.items.len, st.shards.items.len);
    // The file holds the list NewCampaign made; readBlocks reversed it as
    // LoadGame does.
    const n = ref.shards.items.len;
    for (ref.shards.items, 0..) |want, k| {
        const got = st.shards.items[n - 1 - k];
        const fields = [_][2]i32{
            .{ want.num, got.num },         .{ want.x, got.x },               .{ want.y, got.y },
            .{ want.level, got.level },     .{ want.seed, got.seed },         .{ want.income, got.income },
            .{ want.terrain, got.terrain }, .{ want.player, got.player },     .{ want.map.x, got.map.x },
            .{ want.map.players, got.map.players }, .{ want.map.water, got.map.water }, .{ want.map.dissolve, got.map.dissolve },
            .{ want.map.neutrals, got.map.neutrals },
        };
        for (fields, 0..) |fv, j| if (fv[0] != fv[1]) {
            std.debug.print("\nshard {d} field {d}: want {d} got {d}\n", .{ k, j, fv[0], fv[1] });
            return error.TestExpectedEqual;
        };
        try tst.expectEqualSlices(i32, &want.building, &got.building);
        if (same_lang) try tst.expectEqualSlices(u8, std.mem.sliceTo(&want.name, 0), std.mem.sliceTo(&got.name, 0));
        for (want.bonus, got.bonus) |wb, gb| {
            try tst.expectEqual(wb[0], gb[0]);
            if (wb[0] > 0) try tst.expectEqual(wb[1], gb[1]);
        }
    }
}

test "astral save round trip" {
    const gpa = tst.allocator;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    const st = try State.create(gpa, &pr);
    defer st.destroy();
    var r: Rand = .{};
    r.seed(1234);
    try newCampaign(st, &pr, &r);
    const bytes = try astralBytes(gpa, st);
    defer gpa.free(bytes);
    const back = try State.create(gpa, &pr);
    defer back.destroy();
    try loadAstral(back, &pr, bytes);
    try tst.expectEqual(st.shards.items.len, back.shards.items.len);
    // Reversed by the load.
    const n = st.shards.items.len;
    for (st.shards.items, 0..) |sh, i| try tst.expectEqual(sh.num, back.shards.items[n - 1 - i].num);
}
