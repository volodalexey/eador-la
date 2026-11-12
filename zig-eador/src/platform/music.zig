//! Background music (clAudioTrack over music.dat in Audio.cpp): one Ogg
//! stream from common/music/<name>.ogg, updated every frame instead of the
//! original's timer callback.
//!
//! ChangeMusic (Audio.cpp:163) plays the track of the current game mode
//! (iGameMode): 0 Menu01, 1 Astral01, 2 Main01..Main11 in turn (iActiveTrack),
//! 3 a random War01..War06, 4 Prepare01, 5 Memory01 (castle, credits),
//! 6 x01Victory, 7 x02Lose, 8 x03Gremlin (looped).  Tracks are not looped
//! (PlayMusic(..., Loop = 0), except the gremlin's 1).  AnimHourglass
//! (Audio.cpp:371) runs every 200 ms: a counter starting at 100 counts down
//! and, at 0, restarts the music of the current mode if the track has ended
//! (cycling Main01..Main11 / a new random War track by mode), then waits
//! 20 + Random(100) ticks again.
const std = @import("std");
const rl = @import("raylib");
const GameFs = @import("fs.zig").GameFs;
const Rand = @import("../data/rand.zig").Rand;

const tick_s = 0.2;

const max_main = 11; // MUZ_MAX_MAIN
const max_war = 6; // MUZ_MAX_WAR

/// The iGameMode of ChangeMusic (Audio.cpp:175).
pub const Mode = enum {
    /// 0: Menu01 — the main menu.
    menu,
    /// 1: Astral01 — the astral.
    astral,
    /// 2: Main01..Main11 in turn (iActiveTrack++ wraps at 11).
    map,
    /// 3: a random War01..War06 each change.
    war,
    /// 4: Prepare01 — the battle placement.
    prepare,
    /// 5: Memory01 — the castle, credits, end-of-game screens.
    memory,
    /// 6: x01Victory — the battle's victory jingle, one-shot.
    victory,
    /// 7: x02Lose — the battle's defeat jingle, one-shot.
    lose,
    /// 8: x03Gremlin — looped.
    gremlin,
};

pub const Player = struct {
    gpa: std.mem.Allocator,
    /// raylib streams from this buffer, it must outlive the stream.
    bytes: ?[]u8 = null,
    stream: ?rl.Music = null,
    /// The mode of the track playing now (ChangeMusic's iMusicMode).
    mode: ?Mode = null,
    /// iActiveTrack: the last Main track of the map (1..11, 0 before any).
    active_track: u8 = 0,
    volume: f32 = 1,
    enabled: bool = true,
    acc: f32 = 0,
    /// iPause.
    pause: u32 = 100,

    pub fn init(gpa: std.mem.Allocator) Player {
        return .{ .gpa = gpa };
    }

    pub fn deinit(self: *Player) void {
        self.stop();
    }

    pub fn stop(self: *Player) void {
        if (self.stream) |s| {
            rl.stopMusicStream(s);
            rl.unloadMusicStream(s);
        }
        if (self.bytes) |b| self.gpa.free(b);
        self.stream = null;
        self.bytes = null;
    }

    /// ChangeMusic: switch to the mode's track when the mode changed or
    /// nothing is playing (Audio.cpp:165).
    pub fn setMode(self: *Player, fs: *GameFs, mode: Mode) void {
        const same = if (self.mode) |m| m == mode else false;
        if (same and !self.finished()) return;
        self.mode = mode;
        self.playMode(fs, mode);
    }

    /// The mode's track: the ChangeMusic switch (Audio.cpp:175).  The map
    /// and war tracks are drawn from raylib's own RNG (as in recomp) so the
    /// game's stream is not touched.
    fn playMode(self: *Player, fs: *GameFs, mode: Mode) void {
        var name_buf: [16]u8 = undefined;
        const name: []const u8 = switch (mode) {
            .menu => "Menu01",
            .astral => "Astral01",
            .map => blk: {
                if (self.active_track == 0) self.active_track = @as(u8, @intCast(rl.getRandomValue(0, 10)));
                self.active_track = self.active_track % max_main + 1;
                break :blk std.fmt.bufPrint(&name_buf, "Main{d:0>2}", .{self.active_track}) catch return;
            },
            .war => blk: {
                const n = 1 + @as(u8, @intCast(rl.getRandomValue(0, 5)));
                break :blk std.fmt.bufPrint(&name_buf, "War{d:0>2}", .{n}) catch return;
            },
            .prepare => "Prepare01",
            .memory => "Memory01",
            .victory => "x01Victory",
            .lose => "x02Lose",
            .gremlin => "x03Gremlin",
        };
        self.play(fs, name, mode == .gremlin);
    }

    /// The battle's result jingle: x01Victory/x02Lose plays once, then the
    /// mode returns to the map (Battlefield.cpp:1598: iGameMode 6/7, then 2,
    /// iPause = 30).  The jingle's track ends and the map's music takes over.
    pub fn jingle(self: *Player, fs: *GameFs, win: bool) void {
        self.mode = .map;
        self.pause = 30;
        self.play(fs, if (win) "x01Victory" else "x02Lose", false);
    }

    fn play(self: *Player, fs: *GameFs, name: []const u8, loop: bool) void {
        self.stop();
        if (!self.enabled) return;
        var pb: [128]u8 = undefined;
        const path = std.fmt.bufPrint(&pb, "music/{s}.ogg", .{name}) catch return;
        const bytes = fs.read(path) catch return;
        var s = rl.loadMusicStreamFromMemory(".ogg", bytes) catch {
            self.gpa.free(bytes);
            return;
        };
        s.looping = loop;
        rl.setMusicVolume(s, self.volume);
        rl.playMusicStream(s);
        self.bytes = bytes;
        self.stream = s;
    }

    /// ChangeMusicVolume with iMusicVolume (0..255).
    pub fn setVolume(self: *Player, v: i32) void {
        self.volume = @as(f32, @floatFromInt(std.math.clamp(v, 0, 255))) / 255;
        if (self.stream) |s| rl.setMusicVolume(s, self.volume);
    }

    pub fn update(self: *Player, fs: *GameFs, rand: *Rand, dt: f32) void {
        if (self.stream) |s| rl.updateMusicStream(s);
        // AnimHourglass: the pause only runs when the music is on.
        if (!self.enabled) return;
        self.acc += dt;
        while (self.acc >= tick_s) : (self.acc -= tick_s) {
            self.pause -= 1;
            if (self.pause == 0) {
                // AnimHourglass: restart the mode's music if the track ended.
                if (self.mode) |m| if (self.finished()) self.playMode(fs, m);
                self.pause = 20 + rand.random(100);
            }
        }
    }

    /// The track ended (MainTrack < 0).
    pub fn finished(self: *const Player) bool {
        const s = self.stream orelse return true;
        return !rl.isMusicStreamPlaying(s);
    }
};
