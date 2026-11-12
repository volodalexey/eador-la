//! Shared state handed to every scene.
const std = @import("std");
const rl = @import("raylib");
const GameFs = @import("platform/fs.zig").GameFs;
const gfx = @import("platform/gfx.zig");
const music = @import("platform/music.zig");
const Config = @import("data/config.zig").Config;
const Proto = @import("data/proto.zig").Proto;
const Rand = @import("data/rand.zig").Rand;
const Human = @import("data/profile.zig").Human;

/// The version string of the GOG executable (stVersion).
pub const version = "1.05.2";

pub const Ctx = struct {
    gpa: std.mem.Allocator,
    fs: *GameFs,
    /// font01 (Font1 in the original) and font02 (Font2).
    font1: gfx.Font,
    font2: gfx.Font,
    /// mouse.dat; `cursor` is the object drawn at the pointer (set_mouse_sprite).
    mouse: gfx.Sheet,
    cursor: u32 = 0,
    music: music.Player,
    /// eador.cfg.
    cfg: Config,
    /// Prototypes of the current language (LoadPhrases + Loader).
    proto: *Proto,
    /// arHumans[0]: the active player's profile.
    human: Human,
    /// rand() state, seeded like Randomizer() (time % 10000).
    rand: Rand = .{},
    /// iSeed: counts up while the main menu runs and seeds new maps.
    seed: u32 = 1,
    /// A game screen wrote or deleted save files: the main menu checks
    /// them again (CheckGameLoad after RunGame).
    saves_changed: bool = false,
    /// EADOR_VERBOSE: debug output.
    verbose: bool = false,
    skipSetCursor: bool = false,
    /// Seconds since start.
    time: f64 = 0,
    /// AnimHourglass: shared UI blink phase, one tick every 200 ms, 0..1000.
    tick: u16 = 0,
    tick_elapsed: f64 = 0,

    pub fn advanceTime(self: *Ctx, dt: f32) void {
        self.time += dt;
        advanceTick(&self.tick, &self.tick_elapsed, dt);
    }

    pub fn setCursor(self: *Ctx, index: u32) void {
        self.cursor = index;
    }

    pub fn english(self: *const Ctx) bool {
        return std.mem.eql(u8, self.fs.lang(), "en");
    }
};

fn advanceTick(tick: *u16, elapsed: *f64, dt: f32) void {
    elapsed.* += dt;
    while (elapsed.* >= 0.2) {
        elapsed.* -= 0.2;
        tick.* = if (tick.* == 1000) 0 else tick.* + 1;
    }
}

test "UI tick accumulates frame time, catches up and wraps after 1000" {
    var tick: u16 = 0;
    var elapsed: f64 = 0;
    advanceTick(&tick, &elapsed, 0.125);
    try std.testing.expectEqual(@as(u16, 0), tick);
    advanceTick(&tick, &elapsed, 0.125);
    try std.testing.expectEqual(@as(u16, 1), tick);
    try std.testing.expectApproxEqAbs(@as(f64, 0.05), elapsed, 0.000001);
    advanceTick(&tick, &elapsed, 0.625);
    try std.testing.expectEqual(@as(u16, 4), tick);
    try std.testing.expectApproxEqAbs(@as(f64, 0.075), elapsed, 0.000001);

    tick = 999;
    elapsed = 0;
    advanceTick(&tick, &elapsed, 0.25);
    try std.testing.expectEqual(@as(u16, 1000), tick);
    advanceTick(&tick, &elapsed, 0.25);
    try std.testing.expectEqual(@as(u16, 0), tick);
    try std.testing.expectApproxEqAbs(@as(f64, 0.1), elapsed, 0.000001);
}

test "UI tick uses elapsed time rather than frame count" {
    var tick: u16 = 0;
    var elapsed: f64 = 0;
    for (0..60) |_| advanceTick(&tick, &elapsed, 1.0 / 60.0);
    try std.testing.expectEqual(@as(u16, 5), tick);
}
