//! The game's random numbers: MSVC rand() (the original is built with
//! MSVC 2008) and Random(Max) from Randomizer.cpp.
const std = @import("std");

pub const Rand = struct {
    state: u32 = 1,

    /// srand().
    pub fn seed(self: *Rand, s: u32) void {
        self.state = s;
    }

    /// rand(): 0..32767.
    pub fn rand(self: *Rand) u32 {
        self.state = self.state *% 214013 +% 2531011;
        return (self.state >> 16) & 0x7fff;
    }

    /// Random(Max) (Randomizer.cpp:26).  L_INT is a 32-bit unsigned long,
    /// so only 0 gives 0; above 30000 extra rand() digits are appended.
    pub fn random(self: *Rand, max: u32) u32 {
        if (max == 0) return 0;
        var result = self.rand();
        var add = max;
        while (add > 30000) {
            add /= 10;
            result = result *% 10 +% self.rand() % 10;
        }
        return result % max;
    }
};

/// fistp to 64 bits: out of range and NaN give the x87 "indefinite" value.
fn fistp(x: f64) u64 {
    if (!(@abs(x) < 9.2e18)) return 0x8000000000000000;
    const i: i64 = @intFromFloat(x);
    return @bitCast(i);
}

/// Random(Max, Param1, Param2) (Randomizer.cpp:42, GOG FUN_00454bb0): a
/// number derived from the seed and the parameters, no rand() is used.
/// Conversions truncate (the code sets RC = 11).  The product wraps and is
/// loaded as signed, so a negative one makes log() NaN and the result 0.
pub fn keyed(seed: u32, max: u32, p1: i32, p2: i32) u32 {
    var temp: f64 = @floatFromInt(seed);
    while (temp > 10) temp /= 10;
    const v = p1 *% 100 +% p2 +% 1;
    const t = @log(temp * @as(f64, @floatFromInt(v))) * 100000;
    const whole: u32 = @truncate(fistp(t));
    const frac = t - @as(f64, @floatFromInt(whole));
    return @truncate(fistp(@as(f64, @floatFromInt(max)) * frac));
}

/// Random(Max, Param1, Param2, Param3, Param4) (Randomizer.cpp:74, GOG
/// FUN_00454ad0): frac(log(v·(P1·100 + P2 + 1) + P4) / (P3 + 100) · 10⁷)
/// · Max, v the seed's leading digits.  The x87 computes it in 80 bits.
pub fn keyed4(seed: u32, max: u32, p1: i32, p2: i32, p3: i32, p4: i32) u32 {
    var v: f80 = @floatFromInt(seed);
    while (v > 10.0) v /= 10.0;
    const t: i32 = p1 *% 100 +% p2 +% 1;
    const x: f80 = v * @as(f80, @floatFromInt(t)) + @as(f80, @floatFromInt(p4));
    if (!(x > 0)) return 0;
    const f: f80 = @log(x) / @as(f80, @floatFromInt(p3 +% 100)) * 10000000.0;
    if (!(@abs(f) < 9.2e18)) return 0;
    const whole: u32 = @truncate(@as(u64, @bitCast(@as(i64, @intFromFloat(f)))));
    const p: f80 = @as(f80, @floatFromInt(max)) * (f - @as(f80, @floatFromInt(whole)));
    if (!(@abs(p) < 9.2e18)) return 0;
    return @truncate(@as(u64, @bitCast(@as(i64, @intFromFloat(p)))));
}

/// Random(Max, Param1, Param2, Param3) (Randomizer.cpp:57): the whole seed
/// (no division to one digit), /99, and the absolute value of the log.
/// MSVC's long double is a double.
pub fn keyed3(seed: u32, max: u32, p1: i32, p2: i32, p3: i32) u32 {
    const t: i32 = p1 *% 100 +% p2 +% 1;
    const x: f64 = @as(f64, @floatFromInt(seed)) * @as(f64, @floatFromInt(t)) / 99.0;
    if (!(x > 0)) return 0;
    var f: f64 = @log(x) / @as(f64, @floatFromInt(p3 +% 100)) * 10000000.0;
    if (f < 0) f = -f;
    const whole: u32 = @truncate(fistp(f));
    return @truncate(fistp(@as(f64, @floatFromInt(max)) * (f - @as(f64, @floatFromInt(whole)))));
}

test "keyed4 matches the recomp values for seed 355" {
    try std.testing.expectEqual(@as(u32, 9688), keyed4(355, 20000, 500, 300, 355, 2));
    try std.testing.expectEqual(@as(u32, 1303), keyed4(355, 20000, 1724, 497, 355, 2));
    try std.testing.expectEqual(@as(u32, 612), keyed4(355, 20000, 1953, 1339, 355, 2));
    try std.testing.expectEqual(@as(u32, 5), keyed4(355, 57, 355, 812, 1500, 2));
    try std.testing.expectEqual(@as(u32, 4), keyed4(355, 5, 812, 4321, 2, 1500));
    // log of a non-positive value: the x87 "indefinite" path gives 0.
    try std.testing.expectEqual(@as(u32, 0), keyed4(355, 20000, -120, 40, 355, 2));
}

test "keyed stays in range and gives 0 on an overflowed product" {
    for (1..200) |n| try std.testing.expect(keyed(8097, 100, 8097, @intCast(n)) < 100);
    try std.testing.expectEqual(@as(u32, 0), keyed(30_000_000, 100, 30_000_000, 1));
}

test "MSVC rand sequence" {
    var r: Rand = .{};
    try std.testing.expectEqual(@as(u32, 41), r.rand());
    try std.testing.expectEqual(@as(u32, 18467), r.rand());
    try std.testing.expectEqual(@as(u32, 6334), r.rand());
    r.seed(1);
    try std.testing.expectEqual(@as(u32, 41 % 16), r.random(16));
    try std.testing.expectEqual(@as(u32, 0), r.random(0));
}
