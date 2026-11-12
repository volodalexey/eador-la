//! eador.cfg (LoadConfig, Prototype.cpp:947).  Values are positional:
//! ReadWord skips text up to ':' or ';' and the next character, ReadInt reads
//! an optional '-' and digits.  Key names are never compared.
//!
//! GOG 1.05.02 has 24 fields; the 1.05.1 C++ code reads 26 (iMaxHerbSign
//! and iMaxAvatarPic after "Max illustration").  The GOG binary reads the
//! file's 24 (FUN_00468fb0 in recomp), so the layout here follows GOG and
//! the two counts come from the datafiles (see Config.herb_signs).
const std = @import("std");
const Reader = @import("../formats/varstream.zig").Reader;

pub const Config = struct {
    sound: bool = true,
    windowed: bool = true,
    run_background: bool = true,
    /// ms, at least 5.
    refresh_delay: i32 = 40,
    /// ms, at least 2.
    mouse_delay: i32 = 25,
    keyboard_delay: i32 = 500,
    keyboard_repeat: i32 = 500,
    /// At least 100.
    ogg_buffer: i32 = 20000,
    /// 16, 24 or 32.
    color_depth: i32 = 32,
    /// Below 10 means 0.
    sleep_time: i32 = 50,
    /// At most 10.
    antiblink_delay: i32 = 0,
    /// Read and dropped (YuPlay is not ported).
    yuplay: i32 = 0,
    intro: bool = true,
    wallpaper: bool = true,
    static_loot: bool = false,
    static_event: bool = false,
    /// RES_BUY_CHANGE, MAX_PROFIT, MERCENARY_TIME, EVENT_POSSIBILITY,
    /// MAX_BATTLE_DURATION.
    resource_price_mod: i32 = 5,
    max_profit: i32 = 500,
    mercenary_time: i32 = 7,
    event_possibility: i32 = 30,
    max_battle_duration: i32 = 40,
    /// MAX_ILLUSTRATION.
    max_illustration: i32 = 107,
    /// MAX_PATH.
    max_path: i32 = 20,
    lake_ships: bool = false,

    /// A missing or short file keeps the defaults for the missing fields,
    /// like the original (it stops reading at the end of the file).
    pub fn parse(text: []const u8) Config {
        var c: Config = .{};
        var r = Reader.init(text);
        const b = struct {
            fn f(r_: *Reader, dst: *bool) void {
                if (r_.eof()) return;
                dst.* = r_.field() != 0;
            }
        }.f;
        const n = struct {
            fn f(r_: *Reader, dst: *i32) void {
                if (r_.eof()) return;
                dst.* = r_.field();
            }
        }.f;
        b(&r, &c.sound);
        b(&r, &c.windowed);
        b(&r, &c.run_background);
        n(&r, &c.refresh_delay);
        n(&r, &c.mouse_delay);
        n(&r, &c.keyboard_delay);
        n(&r, &c.keyboard_repeat);
        n(&r, &c.ogg_buffer);
        n(&r, &c.color_depth);
        n(&r, &c.sleep_time);
        n(&r, &c.antiblink_delay);
        n(&r, &c.yuplay);
        b(&r, &c.intro);
        b(&r, &c.wallpaper);
        b(&r, &c.static_loot);
        b(&r, &c.static_event);
        n(&r, &c.resource_price_mod);
        n(&r, &c.max_profit);
        n(&r, &c.mercenary_time);
        n(&r, &c.event_possibility);
        n(&r, &c.max_battle_duration);
        n(&r, &c.max_illustration);
        n(&r, &c.max_path);
        b(&r, &c.lake_ships);

        c.refresh_delay = @max(c.refresh_delay, 5);
        c.mouse_delay = @max(c.mouse_delay, 2);
        c.ogg_buffer = @max(c.ogg_buffer, 100);
        if (c.color_depth != 24 and c.color_depth != 32) c.color_depth = 16;
        if (c.sleep_time < 10) c.sleep_time = 0;
        c.antiblink_delay = @min(c.antiblink_delay, 10);
        return c;
    }
};

const t = std.testing;

test "GOG eador.cfg" {
    const text =
        \\***MAIN SETTINGS***
        \\Sound (0 - off/1 - on): 1
        \\Windowed (0 - off/1 - on): 0
        \\Run background (0 - off/1 - on): 1
        \\Refresh delay (default 40): 3
        \\Poll mouse delay (default 25): 25
        \\Keyboard delay (default 500): 500
        \\Keyboard repeat (default 500): 500
        \\Sound buffer (default 20000): 20000
        \\Color depth (16/24/32, default 32): 8
        \\Sleep time (default 50): 5
        \\Antiblink Delay (default 1): 30
        \\YuPlay (0 - off/1 - on): 0
        \\Intro (0 - off/1 - on): 0
        \\Wallpaper (0 - off/1 - on): 1
        \\
        \\***GAMEPLAY PARAMS***
        \\Static loot (0 - off/1 - on): 0
        \\Static event (0 - off/1 - on): 0
        \\Resource price modifier (default 5): 5
        \\Max percentage profit (default 500): 500
        \\Mercenary change time (default 7): 7
        \\Event possibility (default 30): 30
        \\Max battle duration (default 40): 35
        \\
        \\***ADDITIONAL SETTINGS***
        \\Max illustration (default 107): 107
        \\Max path (default 20): 20
        \\Lake ships (0 - off/1 - on, default 0): 1
    ;
    const c = Config.parse(text);
    try t.expect(c.sound and !c.windowed and !c.intro and c.lake_ships);
    try t.expectEqual(@as(i32, 5), c.refresh_delay);
    try t.expectEqual(@as(i32, 16), c.color_depth);
    try t.expectEqual(@as(i32, 0), c.sleep_time);
    try t.expectEqual(@as(i32, 10), c.antiblink_delay);
    try t.expectEqual(@as(i32, 35), c.max_battle_duration);
    try t.expectEqual(@as(i32, 107), c.max_illustration);
    try t.expectEqual(@as(i32, 20), c.max_path);
}

test "missing fields keep defaults" {
    const c = Config.parse("Sound: 0\n");
    try t.expect(!c.sound and c.intro);
    try t.expectEqual(@as(i32, 40), c.refresh_delay);
}

test "imported eador.cfg (skipped without assets)" {
    const text = std.Io.Dir.cwd().readFileAlloc(t.io, "zig-out/assets/common/eador.cfg", t.allocator, .limited(1 << 16)) catch return error.SkipZigTest;
    defer t.allocator.free(text);
    const c = Config.parse(text);
    try t.expectEqual(@as(i32, 40), c.refresh_delay);
    try t.expectEqual(@as(i32, 40), c.max_battle_duration);
    try t.expectEqual(@as(i32, 107), c.max_illustration);
    try t.expectEqual(@as(i32, 20), c.max_path);
    try t.expect(!c.lake_ships);
}
