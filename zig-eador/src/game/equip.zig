//! Moving items between the hero's doll and backpack, the treasury and a
//! shop: the item part of ScreenHero (Screens.cpp:2449) and ScreenHeroShop
//! (Screens.cpp:4745), with SlotNum and GetFreeSlot (Screens.cpp:10, :29).
//!
//! Where the original moves an item to GetFreeSlot and the backpack is
//! full (slot 0, which no screen shows), the item stays where it is.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const hero_mod = @import("hero.zig");
const Hero = hero_mod.Hero;
const player = @import("player.zig");
const site = @import("site.zig");

/// ItemOwner: 1 the hero, 2 the treasury, 3 the shop.
pub const Owner = enum { hero, treasury, shop };

/// SelectedItem, SelectedItemDur, SelectedSlot and ItemOwner.
pub const Held = struct { item: i32, dur: i32, slot: usize, owner: Owner };

pub const Treasury = [player.treasury_size][2]i32;

/// Item slot 100: a two-handed weapon (slots 1 and 2).
pub const two_hand = 100;
/// The first backpack slot.
pub const bag_first = 16;

fn slotOf(pr: *const Proto, item: i32) i32 {
    if (item <= 0 or item >= pr.items.len) return 0;
    return pr.items[@intCast(item)].slot;
}

/// SlotNum: the item slot of doll place `k` (11 the second ring, 12..15
/// the amulets).
pub fn slotNum(k: usize) i32 {
    return switch (k) {
        11 => 10,
        12...15 => 11,
        else => @intCast(k),
    };
}

/// GetFreeSlot: the first empty backpack slot other than `exclude`, 0 if
/// the backpack is full.
pub fn freeSlot(h: *const Hero, exclude: usize) usize {
    for (bag_first..hero_mod.inventory_size + 1) |i| {
        if (h.inventory[i][0] == 0 and i != exclude) return i;
    }
    return 0;
}

/// The glow of doll place `k` (1..15) for the item in the hand.
pub fn glows(pr: *const Proto, h: *const Hero, item: i32, k: usize) bool {
    if (item <= 0 or !h.canUse(pr, item)) return false;
    const s = slotOf(pr, item);
    return slotNum(k) == s or (s == two_hand and slotNum(k) == 1);
}

/// A click on the doll (place 0): where the item in the hand goes.
pub fn dollTarget(pr: *const Proto, h: *const Hero, item: i32) usize {
    const s = slotOf(pr, item);
    const inv = &h.inventory;
    if (h.canUse(pr, item) and s > 0) {
        return switch (s) {
            10 => if (inv[11][0] == 0 and inv[10][0] != 0) 11 else 10,
            11 => for (12..16) |i| {
                if (inv[i][0] == 0) break i;
            } else 12,
            two_hand => 1,
            1...9 => @intCast(s),
            else => bag_first,
        };
    }
    const f = freeSlot(h, 0);
    return if (f == 0) bag_first else f;
}

/// The held item may go to place `k`.
pub fn accepts(pr: *const Proto, h: *const Hero, held: Held, k: usize) bool {
    const inv = &h.inventory;
    const s = slotOf(pr, held.item);
    const use = h.canUse(pr, held.item);
    const sn = slotNum(k);
    const a = sn == s and use and (sn != 2 or inv[1][0] == 0 or slotOf(pr, inv[1][0]) != two_hand or freeSlot(h, 0) != 0);
    const b = s == two_hand and sn == 1 and use and (inv[k][0] == 0 or inv[2][0] == 0 or freeSlot(h, held.slot) != 0);
    return a or b or k > 15;
}

fn toBag(h: *Hero, from: usize, exclude: usize) void {
    const f = freeSlot(h, exclude);
    if (f == 0) return;
    h.inventory[f] = h.inventory[from];
    h.inventory[from] = .{ 0, 0 };
}

/// The purchase of a shop item (CoinSound is the caller's).
pub const Buy = struct {
    m: player.Market,
    shop: *site.Shop,
    /// key[KEY_LCONTROL] || key[KEY_RCONTROL]: keep buying the same lot.
    ctrl: bool = false,

    /// Pays for `held` and takes one from the lot; true when it is sold out.
    pub fn purchase(self: Buy, pr: *const Proto, held: Held) bool {
        self.m.pay(self.m.itemPrice(held.item, self.shop));
        self.m.raiseResPrices(&pr.items[@intCast(held.item)].res);
        const sl = &self.shop.items[held.slot];
        if (sl[1] > 0) {
            sl[1] -= 1;
            if (sl[1] == 0) {
                sl[0] = 0;
                return true;
            }
        }
        return false;
    }

    /// bSkip of a purchase: the hand empties unless Ctrl is held, the lot
    /// is left and the next one is affordable.
    fn keep(self: Buy, held: Held, sold_out: bool) bool {
        return self.ctrl and !sold_out and self.m.affords(self.m.itemPrice(held.item, self.shop));
    }
};

pub const Result = struct {
    /// What is in the hand afterwards.
    held: ?Held,
    bought: bool = false,
};

/// The held item put on place `k` of the hero (1..25, after
/// `dollTarget`).  Null: nothing happens.
pub fn dropOnHero(pr: *const Proto, h: *Hero, treasury: *Treasury, held: Held, k: usize, buy: ?Buy) ?Result {
    if (k == 0 or k > hero_mod.inventory_size) return null;
    if (!accepts(pr, h, held, k)) return null;
    const inv = &h.inventory;
    const sel = held.slot;
    if (inv[k][0] > 0) {
        const t = inv[k];
        const ts = slotOf(pr, t[0]);
        const sn = slotNum(sel);
        const ok = (sn == ts and h.canUse(pr, t[0])) or freeSlot(h, 0) != 0 or
            (ts == two_hand and sn == 1 and (inv[2][0] == 0 or freeSlot(h, sel) != 0)) or
            sel > 15 or held.owner != .hero;
        if (!ok) return null;
        switch (held.owner) {
            .hero => {
                // The item the hero gives up goes to the backpack if it
                // does not fit the place the held one came from.
                const f = freeSlot(h, 0);
                if ((!h.canUse(pr, t[0]) or sn != ts) and sel < bag_first and f != 0) {
                    inv[f] = t;
                } else inv[sel] = t;
                if (ts == two_hand and inv[2][0] > 0 and h.canUse(pr, t[0]) and sel == 1) toBag(h, 2, sel);
            },
            .treasury => treasury[sel] = t,
            .shop => {},
        }
        inv[k] = .{ held.item, held.dur };
        if (k == 1 and slotOf(pr, held.item) == two_hand and inv[2][0] > 0) toBag(h, 2, sel);
        if (k == 2 and inv[1][0] > 0 and slotOf(pr, inv[1][0]) == two_hand) toBag(h, 1, 0);
        if (held.owner == .shop) {
            _ = buy.?.purchase(pr, held);
            // The replaced item is in the hand, from the first empty
            // backpack slot or else the treasury.
            for (bag_first..26) |i| if (inv[i][0] == 0) {
                return .{ .held = .{ .item = t[0], .dur = t[1], .slot = i, .owner = .hero }, .bought = true };
            };
            var slot: usize = player.treasury_size - 1;
            for (treasury, 0..) |it, i| if (it[0] == 0) {
                slot = i;
                break;
            };
            return .{ .held = .{ .item = t[0], .dur = t[1], .slot = slot, .owner = .treasury }, .bought = true };
        }
        return .{ .held = null };
    }
    if (slotOf(pr, held.item) == two_hand and inv[2][0] > 0 and k == 1 and freeSlot(h, 0) == 0) return null;
    var keep = false;
    var bought = false;
    if (held.owner == .shop) if (buy) |b| {
        bought = true;
        keep = b.keep(held, b.purchase(pr, held));
    };
    inv[k] = .{ held.item, held.dur };
    if (slotOf(pr, held.item) == two_hand and inv[2][0] > 0 and k == 1) toBag(h, 2, 0);
    if (k == 2 and inv[1][0] > 0 and slotOf(pr, inv[1][0]) == two_hand) toBag(h, 1, 0);
    return .{ .held = if (keep) held else null, .bought = bought };
}

/// ScreenHeroHero (Screens.cpp:2004): the item `held` taken from `giver`
/// put on place `k` of another hero `to` (1..25, after `dollTarget`).  On
/// a taken place the item there goes to the giver: to the place the held
/// one came from, or to his backpack when he cannot wear it there.  False:
/// nothing happens.  For the same hero use `dropOnHero`.
pub fn dropOnOther(pr: *const Proto, to: *Hero, giver: *Hero, held: Held, k: usize) bool {
    if (k == 0 or k > hero_mod.inventory_size) return false;
    const inv = &to.inventory;
    const s = slotOf(pr, held.item);
    const use = to.canUse(pr, held.item);
    const sn = slotNum(k);
    const a = sn == s and use and (sn != 2 or inv[1][0] == 0 or slotOf(pr, inv[1][0]) != two_hand or freeSlot(to, 0) != 0);
    const b = s == two_hand and sn == 1 and use and (inv[k][0] == 0 or inv[2][0] == 0 or freeSlot(to, 0) != 0);
    if (!(a or b or k > 15)) return false;
    const sel = held.slot;
    if (inv[k][0] > 0) {
        const t = inv[k];
        const ts = slotOf(pr, t[0]);
        const ssn = slotNum(sel);
        const gi = &giver.inventory;
        const ok = (ssn == ts and giver.canUse(pr, t[0])) or freeSlot(giver, 0) != 0 or
            (ts == two_hand and ssn == 1 and (gi[2][0] == 0 or freeSlot(giver, sel) != 0)) or sel > 15;
        if (!ok) return false;
        const f = freeSlot(giver, 0);
        if ((!giver.canUse(pr, t[0]) or ssn != ts) and sel < bag_first and f != 0) {
            gi[f] = t;
        } else gi[sel] = t;
        if (ts == two_hand and gi[2][0] > 0 and giver.canUse(pr, t[0]) and sel == 1) toBag(giver, 2, sel);
    } else if (s == two_hand and inv[2][0] > 0 and k == 1 and freeSlot(to, 0) == 0) return false;
    inv[k] = .{ held.item, held.dur };
    if (k == 1 and s == two_hand and inv[2][0] > 0) toBag(to, 2, 0);
    if (k == 2 and inv[1][0] > 0 and slotOf(pr, inv[1][0]) == two_hand) toBag(to, 1, 0);
    return true;
}

/// The held item put on treasury slot `k`.  Null: nothing happens.
pub fn dropOnTreasury(pr: *const Proto, h: ?*Hero, treasury: *Treasury, held: Held, k: usize, buy: ?Buy) ?Result {
    if (treasury[k][0] > 0) {
        const t = treasury[k];
        switch (held.owner) {
            .hero => {
                const hr = h orelse return null;
                const inv = &hr.inventory;
                const sel = held.slot;
                const ts = slotOf(pr, t[0]);
                const sn = slotNum(sel);
                const ok = (sn == ts and hr.canUse(pr, t[0])) or freeSlot(hr, 0) != 0 or
                    (ts == two_hand and sn == 1 and (inv[2][0] == 0 or freeSlot(hr, sel) != 0)) or sel > 15;
                if (!ok) return null;
                const f = freeSlot(hr, 0);
                if ((!hr.canUse(pr, t[0]) or sn != ts) and sel < bag_first and f != 0) {
                    inv[f] = t;
                } else inv[sel] = t;
                if (ts == two_hand and inv[2][0] > 0 and hr.canUse(pr, t[0]) and sel == 1) toBag(hr, 2, sel);
            },
            .treasury => treasury[held.slot] = t,
            .shop => return null,
        }
        treasury[k] = .{ held.item, held.dur };
        return .{ .held = null };
    }
    var keep = false;
    var bought = false;
    if (held.owner == .shop) if (buy) |b| {
        bought = true;
        keep = b.keep(held, b.purchase(pr, held));
    };
    treasury[k] = .{ held.item, held.dur };
    return .{ .held = if (keep) held else null, .bought = bought };
}

/// The total repair of the hero's items (buttRepairAll): the items worn
/// below their durability.
pub fn repairAllPrice(m: player.Market, h: *const Hero, shop: *const site.Shop) player.Price {
    var out: player.Price = .{ 0, 0 };
    for (h.inventory[1..]) |it| {
        if (it[0] <= 0) continue;
        const p = m.repairPrice(it[0], it[1], shop);
        if (p[0] == 0) continue;
        out[0] += p[0];
        out[1] += p[1];
    }
    return out;
}

pub fn repairAll(m: player.Market, pr: *const Proto, h: *Hero, shop: *const site.Shop) void {
    for (h.inventory[1..]) |*it| {
        if (it[0] <= 0) continue;
        const p = m.repairPrice(it[0], it[1], shop);
        if (p[0] == 0) continue;
        m.pay(p);
        it[1] = pr.items[@intCast(it[0])].durability;
    }
}

test "doll places" {
    try std.testing.expectEqual(@as(i32, 10), slotNum(11));
    try std.testing.expectEqual(@as(i32, 11), slotNum(14));
    try std.testing.expectEqual(@as(i32, 3), slotNum(3));
    try std.testing.expectEqual(@as(i32, 20), slotNum(20));
}
