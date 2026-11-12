//! The hero's spell book (arSpell[1..21]) as ScreenHero (Screens.cpp:2449)
//! and ScreenHeroLibrary (Screens.cpp:4054) change it: SpellSlotLvl,
//! GetFreeSpellSlot (Screens.cpp:53, :98), moving and swapping spells,
//! taking one from the library and writing a scroll into the book.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const hero_mod = @import("hero.zig");
const Hero = hero_mod.Hero;

/// SPELLBOOK_SIZE.
pub const size = 21;
/// 1, SpellLvl2..4 (Windower.h) and the end of the book.
pub const lvl = [_]usize{ 1, 9, 15, 19, size + 1 };
/// SelectedSlot of a spell taken from the library (or pushed out of the
/// book by a scroll): it has no place to go back to.
pub const from_library: i32 = -100;

/// SelectedSpell and SelectedSlot.
pub const Held = struct { spell: i32, slot: i32 };

pub const Result = struct {
    /// What is in the hand afterwards.
    held: ?Held,
    /// A spell of the library went into the book (bBuy).
    learnt: bool = false,
};

/// SpellSlotLvl: the highest circle book slot `i` takes.
pub fn slotLevel(i: i32) i32 {
    if (i < lvl[1]) return 1;
    if (i < lvl[2]) return 2;
    if (i < lvl[3]) return 3;
    return 4;
}

pub fn level(pr: *const Proto, sp: i32) i32 {
    if (sp <= 0 or sp >= pr.spells.len) return 0;
    return pr.spells[@intCast(sp)].level;
}

/// Book slot `i` (1..21) is open: GetSpellSlot of its circle covers it.
pub fn open(pr: *const Proto, h: *const Hero, i: usize) bool {
    if (i == 0 or i > size) return false;
    const r: usize = @intCast(slotLevel(@intCast(i)) - 1);
    return i - lvl[r] < @as(usize, @intCast(@max(h.spellSlots(pr, @intCast(r + 1)), 0)));
}

/// GetFreeSpellSlot: the first empty open slot of circle `level` or a
/// higher one (the switch falls through), 0 if there is none.
pub fn freeSlot(pr: *const Proto, h: *const Hero, lv: i32) usize {
    var r: usize = @intCast(std.math.clamp(lv, 1, 4) - 1);
    while (r < 4) : (r += 1) {
        const n: usize = @intCast(@max(h.spellSlots(pr, @intCast(r + 1)), 0));
        for (lvl[r]..@min(lvl[r] + n, size + 1)) |i| if (h.spell(i) == 0) return i;
    }
    return 0;
}

/// A click on book slot `c` with `held` in the hand.  Null: the circle of
/// the slot is too low, nothing happens.  `keep` (Ctrl) keeps a library
/// spell in the hand to put it into another slot.
pub fn drop(pr: *const Proto, h: *Hero, held: Held, c: usize, keep: bool) ?Result {
    if (slotLevel(@intCast(c)) < level(pr, held.spell)) return null;
    const old = h.spell(c);
    if (held.slot < 0) {
        // A spell of the library.
        h.setSpell(c, held.spell);
        if (old > 0) return .{ .held = .{ .spell = old, .slot = from_library }, .learnt = true };
        return .{ .held = if (keep) held else null, .learnt = true };
    }
    if (old == 0) {
        h.setSpell(c, held.spell);
        return .{ .held = null };
    }
    // A taken slot: the old spell goes to the place the held one came
    // from, or to a free slot of its circle if that place is too low;
    // with no free slot the two change hands.
    const old_lv = level(pr, old);
    const free = freeSlot(pr, h, old_lv);
    if (free == 0) {
        h.setSpell(c, held.spell);
        return .{ .held = .{ .spell = old, .slot = held.slot } };
    }
    if (held.slot == 0 or slotLevel(held.slot) < old_lv) h.setSpell(free, old) else h.setSpell(@intCast(held.slot), old);
    h.setSpell(c, held.spell);
    return .{ .held = null };
}

/// The scroll of spell `sp` dropped on book slot `c` (ScreenHeroLibrary,
/// "Свиток с заклинанием"): the spell is written in and the scroll is
/// spent; a spell that was there is in the hand.  Null: the slot's circle
/// is too low.
pub fn writeScroll(pr: *const Proto, h: *Hero, sp: i32, c: usize) ?Result {
    if (slotLevel(@intCast(c)) < level(pr, sp)) return null;
    const old = h.spell(c);
    h.setSpell(c, sp);
    return .{ .held = if (old > 0) .{ .spell = old, .slot = from_library } else null };
}

/// The held spell goes back to its slot if the slot is empty and takes
/// its circle (on leaving the screen or taking something else).
pub fn putBack(pr: *const Proto, h: *Hero, held: Held) void {
    if (held.slot <= 0 or held.slot > size) return;
    const s: usize = @intCast(held.slot);
    if (h.spell(s) == 0 and slotLevel(held.slot) >= level(pr, held.spell)) h.setSpell(s, held.spell);
}

/// The library of the hero's ruler has the spell: throwing it away asks
/// nothing (it can be taken again).
pub fn inLibrary(lib: []const i32, sp: i32) bool {
    return std.mem.indexOfScalar(i32, lib, sp) != null;
}

const tst = std.testing;

test "slot circles" {
    try tst.expectEqual(@as(i32, 1), slotLevel(1));
    try tst.expectEqual(@as(i32, 1), slotLevel(8));
    try tst.expectEqual(@as(i32, 2), slotLevel(9));
    try tst.expectEqual(@as(i32, 3), slotLevel(15));
    try tst.expectEqual(@as(i32, 4), slotLevel(19));
    try tst.expectEqual(@as(i32, 4), slotLevel(21));
    try tst.expectEqual(@as(i32, 1), slotLevel(from_library));
}

test "moving, swapping and learning spells" {
    const gpa = tst.allocator;
    const GameFs = @import("../platform/fs.zig").GameFs;
    var fs = GameFs.open(tst.io, gpa, "zig-out/assets", "ru") catch return error.SkipZigTest;
    defer fs.close();
    var pr = try Proto.load(gpa, &fs);
    defer pr.deinit();
    // Three spells of the first circle.
    var first: [3]i32 = undefined;
    var n: usize = 0;
    for (pr.spells[1..], 1..) |sp, i| if (sp.level <= 1 and n < 3) {
        first[n] = @intCast(i);
        n += 1;
    };
    try tst.expectEqual(@as(usize, 3), n);
    var h = Hero.garrison(&pr, 1, 1);
    try tst.expect(h.spellSlots(&pr, 1) >= 3);
    try tst.expect(open(&pr, &h, 1) and !open(&pr, &h, lvl[1]));

    // A library spell into an empty slot, then with Ctrl into another.
    var r = drop(&pr, &h, .{ .spell = first[0], .slot = from_library }, 1, false).?;
    try tst.expect(r.held == null and r.learnt);
    r = drop(&pr, &h, .{ .spell = first[1], .slot = from_library }, 2, true).?;
    try tst.expectEqual(first[1], r.held.?.spell);
    // A library spell onto a taken slot: the old one is in the hand.
    r = drop(&pr, &h, .{ .spell = first[2], .slot = from_library }, 1, false).?;
    try tst.expectEqual(Held{ .spell = first[0], .slot = from_library }, r.held.?);
    try tst.expectEqual(first[2], h.spell(1));

    // A book spell taken from slot 2 onto slot 1: the other goes to 2.
    h.setSpell(2, 0);
    r = drop(&pr, &h, .{ .spell = first[1], .slot = 2 }, 1, false).?;
    try tst.expect(r.held == null);
    try tst.expectEqual(first[1], h.spell(1));
    try tst.expectEqual(first[2], h.spell(2));

    // A scroll over slot 2: written in, the old spell in the hand.
    r = writeScroll(&pr, &h, first[0], 2).?;
    try tst.expectEqual(first[2], r.held.?.spell);
    try tst.expectEqual(first[0], h.spell(2));

    // putBack only fills an empty slot.
    putBack(&pr, &h, .{ .spell = first[2], .slot = 2 });
    try tst.expectEqual(first[0], h.spell(2));
    putBack(&pr, &h, .{ .spell = first[2], .slot = 3 });
    try tst.expectEqual(first[2], h.spell(3));
    if (h.spellSlots(&pr, 1) == 3) try tst.expectEqual(@as(usize, 0), freeSlot(&pr, &h, 1));
}
