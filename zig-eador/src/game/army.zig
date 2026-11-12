//! Squad slots of a garrison (a dcHero without class): SlotLvl and
//! GetFreeUnitSlot (Screens.cpp:41, :65), dcHero::GetUnitSlot (Hero.cpp:610)
//! and dcHero::PlaceUnit (Hero.cpp:813).  Slot 0 is the hero's own unit;
//! rank 1 is slots 1..6, rank 2 7..10, rank 3 11..13, rank 4 14..15.
const std = @import("std");
const hero_mod = @import("hero.zig");
const Hero = hero_mod.Hero;
const Unit = @import("unit.zig").Unit;

pub const max_squad = hero_mod.max_squad;
/// SlotLvl2..SlotLvl4 (Windower.h).
pub const rank_first = [5]usize{ 0, 1, 7, 11, 14 };
/// The first slot after each rank.
pub const rank_end = [5]usize{ 0, 7, 11, 14, 16 };

/// SlotLvl: the highest unit level slot `slot` takes.
pub fn slotLevel(slot: usize) i32 {
    if (slot < rank_first[2]) return 1;
    if (slot < rank_first[3]) return 2;
    if (slot < rank_first[4]) return 3;
    return 4;
}

/// arGarrison[level-1] (record +0x390); a garrison keeps it >= 0.
pub fn unitSlots(h: *const Hero, level: usize) i32 {
    return std.mem.readInt(i32, h.rec[0x390 + 4 * (level - 1) ..][0..4], .little);
}

/// The open slots of a rank: rank_first .. rank_first + GetUnitSlot.  A
/// hero's ranks come from his leadership (a garrison's from arGarrison).
pub fn openSlots(h: *const Hero, pr: *const @import("../data/proto.zig").Proto, level: usize) [2]usize {
    const first = rank_first[level];
    const n: usize = @intCast(std.math.clamp(h.unitSlots(pr, @intCast(level)), 0, @as(i32, @intCast(rank_end[level] - first))));
    return .{ first, first + n };
}

/// GetFreeUnitSlot: the first empty open slot of rank `level` or above
/// (the switch falls through), 0 for none.
pub fn freeSlot(h: *const Hero, pr: *const @import("../data/proto.zig").Proto, level: i32) usize {
    var l: usize = @intCast(std.math.clamp(level, 1, 4));
    while (l <= 4) : (l += 1) {
        const r = openSlots(h, pr, l);
        for (r[0]..r[1]) |i| if (h.squad[i] == null) return i;
    }
    return 0;
}

/// PlaceUnit for a garrison: a unit standing on the battle place of
/// another moves to the first free place of the 2×8 field.
pub fn placeUnit(h: *Hero, unit: Unit, slot: usize) void {
    var u = unit;
    const taken = struct {
        fn f(sq: *const [max_squad]?Unit, x: i32, y: i32) bool {
            for (sq) |o| if (o) |v| if (v.x == x and v.y == y) return true;
            return false;
        }
    }.f;
    if (taken(&h.squad, u.x, u.y)) {
        var x: i32 = 0;
        place: while (x < 2) : (x += 1) {
            var y: i32 = 0;
            while (y < 8) : (y += 1) {
                if (taken(&h.squad, x, y)) continue;
                u.x = x;
                u.y = y;
                break :place;
            }
        } else return;
    }
    h.squad[slot] = u;
}

/// dcHero::PlaceUnit: slot 0 of a hero is the first free slot for the
/// unit's level (none: the unit is not placed).
pub fn place(h: *Hero, pr: *const @import("../data/proto.zig").Proto, unit: Unit, slot: usize) void {
    var s = slot;
    if (h.class1 > 0 and s == 0) {
        s = freeSlot(h, pr, pr.units[@intCast(unit.type)].level);
        if (s == 0) return;
    }
    placeUnit(h, unit, s);
}

test "garrison slots" {
    var h: Hero = undefined;
    h.rec = @splat(0);
    h.squad = @splat(null);
    std.mem.writeInt(i32, h.rec[0x390..][0..4], 2, .little);
    std.mem.writeInt(i32, h.rec[0x394..][0..4], -1, .little);
    std.mem.writeInt(i32, h.rec[0x398..][0..4], 1, .little);
    try std.testing.expectEqual([2]usize{ 1, 3 }, openSlots(&h, undefined, 1));
    try std.testing.expectEqual([2]usize{ 7, 7 }, openSlots(&h, undefined, 2));
    try std.testing.expectEqual(@as(usize, 1), freeSlot(&h, undefined, 1));
    const u: Unit = .{ .type = 1, .life = 10, .exp = 0 };
    placeUnit(&h, u, 1);
    placeUnit(&h, u, 2);
    // Both stood on (0, 3): the second moves to (0, 0).
    try std.testing.expectEqual(@as(i32, 0), h.squad[2].?.y);
    // Rank 1 is full, rank 2 has no slots: the free slot is in rank 3.
    try std.testing.expectEqual(@as(usize, 11), freeSlot(&h, undefined, 1));
    try std.testing.expectEqual(@as(usize, 0), freeSlot(&h, undefined, 4));
    try std.testing.expectEqual(@as(i32, 3), slotLevel(13));
}
