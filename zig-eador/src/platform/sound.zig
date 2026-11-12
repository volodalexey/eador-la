//! Interface sounds (RU exe): FUN_004fe460(n) plays unit_sound.dat object n,
//! FUN_004fe3b0(n) spell_sound.dat object n (object 0 at half volume).
//! FUN_004a1190 plays a button's sound (+0x2C) when it fires: n > 0 a unit
//! sound, n <= 0 spell sound |n|; buttons are built with 0, the click.
const std = @import("std");
const rl = @import("raylib");
const gamefs = @import("fs.zig");
const GameFs = gamefs.GameFs;

pub const Dat = enum { unit, spell };

/// unit_sound.dat objects used by the garrison (FUN_004c0450, FUN_00445500).
pub const buy = 0x9B;
pub const dismiss = 0x9E;
pub const repair = 0x1C;
pub const repair_all = 0x81;

var fs_ref: ?*GameFs = null;
var cache: std.AutoHashMapUnmanaged(u32, rl.Sound) = .empty;
var missing: std.AutoHashMapUnmanaged(u32, void) = .empty;

/// Called once the audio device is up; sounds load on first use.
pub fn init(fs: *GameFs) void {
    fs_ref = fs;
}

pub fn deinit() void {
    const fs = fs_ref orelse return;
    var it = cache.valueIterator();
    while (it.next()) |s| rl.unloadSound(s.*);
    cache.deinit(fs.gpa);
    missing.deinit(fs.gpa);
    fs_ref = null;
}

fn load(dat: Dat, n: u32) ?rl.Sound {
    const fs = fs_ref orelse return null;
    const key = n * 2 + @intFromEnum(dat);
    if (cache.get(key)) |s| return s;
    if (missing.contains(key)) return null;
    const s = blk: {
        var pb: [128]u8 = undefined;
        const dir = if (dat == .unit) "sound/unit_sound" else "sound/spell_sound";
        const path = gamefs.indexedPath(fs, &pb, dir, n, ".ogg") orelse break :blk null;
        const bytes = fs.read(path) catch break :blk null;
        defer fs.gpa.free(bytes);
        const wave = rl.loadWaveFromMemory(".ogg", bytes) catch break :blk null;
        defer rl.unloadWave(wave);
        break :blk rl.loadSoundFromWave(wave);
    };
    if (s) |x| {
        cache.put(fs.gpa, key, x) catch {};
    } else missing.put(fs.gpa, key, {}) catch {};
    return s;
}

/// iSoundVolume / 255.
var volume: f32 = 225.0 / 255.0;

/// iSoundVolume (0..255).
pub fn setVolume(v: i32) void {
    volume = @as(f32, @floatFromInt(std.math.clamp(v, 0, 255))) / 255;
}

pub fn play(dat: Dat, n: u32) void {
    const s = load(dat, n) orelse return;
    rl.setSoundVolume(s, volume * @as(f32, if (dat == .spell and n == 0) 0.5 else 1));
    rl.playSound(s);
}

/// The button click (spell sound 0 at half volume).
pub fn click() void {
    play(.spell, 0);
}

/// A button's sound field: > 0 unit sound, <= 0 spell sound |n|.
pub fn button(n: i32) void {
    if (n > 0) play(.unit, @intCast(n)) else play(.spell, @intCast(-n));
}
