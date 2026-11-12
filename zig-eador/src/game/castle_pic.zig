//! MakeCastlePic (Castle.cpp:112): the castle picture of a player, the
//! castle.var parts in layer 0..2, then Yplace order.  A part shows when
//! one of its Buildings is built (0: always, < 0: before the first visit,
//! bEnter false) and none of its AntiBuildings is.  A part of a building
//! group stands at the group place (group_coord.var) of the last upgrade
//! of its first building; a building without a place takes the first free
//! one, which is written to arGroup as in the original.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const player = @import("player.zig");

/// One sprite: castle.dat object `index` at (x, y) with `alpha` 0..255.
pub const Part = struct { index: usize, x: i32, y: i32, alpha: u8 };

/// The parts to draw, in order; `out` holds them.
pub fn compose(pr: *const Proto, built: []const bool, h: *player.Holdings, out: []Part) []Part {
    const n = @min(pr.castle_parts.len, 1024);
    var place: [1024]i32 = undefined;
    for (pr.castle_parts[0..n], 0..) |*cp, i| {
        place[i] = if (cp.group > 0) (groupPlace(pr, h, cp, false) orelse [2]i32{ -100, -100 })[1] else cp.y_place;
    }
    var k: usize = 0;
    for (0..3) |layer| {
        var y: i32 = 0;
        while (y < 768) : (y += 1) {
            for (pr.castle_parts[0..n], 0..) |*cp, i| {
                if (cp.layer != @as(i32, @intCast(layer)) or place[i] != y) continue;
                if (!shown(cp, built, h.enter)) continue;
                var at: [2]i32 = .{ cp.x, cp.y };
                if (cp.group > 0) {
                    const g = groupPlace(pr, h, cp, true) orelse [2]i32{ -100, -100 };
                    at = .{ g[0] - cp.x, g[1] - cp.y };
                }
                if (k == out.len) return out;
                const alpha: u8 = if (cp.opacity > 0) @intCast(std.math.clamp(@divTrunc(255 * cp.opacity, 100), 0, 255)) else 255;
                out[k] = .{ .index = i, .x = at[0], .y = at[1], .alpha = alpha };
                k += 1;
            }
        }
    }
    return out[0..k];
}

fn isBuilt(built: []const bool, b: i32) bool {
    if (b == 0) return true;
    return b > 0 and b < built.len and built[@intCast(b)];
}

fn shown(cp: *const proto_mod.CastlePart, built: []const bool, enter: bool) bool {
    var bld = cp.builds[0] == 0;
    for (cp.builds, cp.anti_builds) |b, a| {
        if (a > 0 and isBuilt(built, a)) return false;
        if (b > 0 and isBuilt(built, b)) bld = true;
        if (!enter and b < 0) bld = true;
    }
    return bld;
}

/// The group place of the part's building after its upgrades, or the
/// first free place (taken into arGroup when `take`); null when the group
/// is full.
fn groupPlace(pr: *const Proto, h: *player.Holdings, cp: *const proto_mod.CastlePart, take: bool) ?[2]i32 {
    const g: usize = @intCast(cp.group);
    var b = cp.builds[0];
    var guard: usize = 0;
    while (b > 0 and b < pr.inner_buildings.len and pr.inner_buildings[@intCast(b)].upgrade > 0 and guard < 64) : (guard += 1) {
        b = pr.inner_buildings[@intCast(b)].upgrade;
    }
    var free: ?usize = null;
    var found: ?usize = null;
    for (h.group[g], 0..) |v, j| {
        if (free == null and v == 0) free = j;
        if (v == b) found = j;
    }
    const j = found orelse blk: {
        const f = free orelse return null;
        if (take) h.group[g][f] = b;
        break :blk f;
    };
    const c = pr.group_coords[g][j];
    return .{ c.id, c.value };
}

test "castle picture of a new game" {
    const GameFs = @import("../platform/fs.zig").GameFs;
    const gpa = std.testing.allocator;
    var fs = GameFs.open(std.testing.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    var built: [proto_mod.max_bld]bool = @splat(false);
    built[0] = true;
    var h: player.Holdings = .{};
    var buf: [512]Part = undefined;
    const parts = compose(&pr, &built, &h, &buf);
    // The background (part 0, Buildings 0) is drawn.
    var bg = false;
    for (parts) |p| bg = bg or p.index == 0;
    try std.testing.expect(bg);
    // Before the first visit the parts with Buildings (-1) show too.
    h.enter = false;
    const first = compose(&pr, &built, &h, &buf).len;
    h.enter = true;
    try std.testing.expect(first >= compose(&pr, &built, &h, &buf).len);
}
