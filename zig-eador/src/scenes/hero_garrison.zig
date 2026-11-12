//! ScreenHeroGarrison (Screens.cpp:3225): the hero of the map and the
//! garrison of his province side by side, with the units for hire of his
//! castle.  Units are hired into an empty slot of the hero or of the
//! garrison, moved between the two, and dismissed (with the refund of
//! castle ability 24).  Items are put on, taken off, moved and thrown away
//! (dialog 1462).  A right click opens WindowItemInfo, WindowUnitInfo /
//! WindowUnitTypeInfo, a click on a resource WindowResourceInfo.
//!
//! Not ported: item-summoned units, the tutorial and the event's local
//! recruit list (effect 29).
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const Hint = @import("../ui/hint.zig").Hint;
const Button = @import("../ui/button.zig").Button;
const castle_ui = @import("../ui/castle_ui.zig");
const Sheets = castle_ui.Sheets;
const gui = castle_ui.gui;
const army_ui = @import("../ui/army_ui.zig");
const hero_ui = @import("../ui/hero_ui.zig");
const army_info = @import("army_info.zig");
const Dialog = @import("dialog.zig").Dialog;
const ItemInfo = @import("item_info.zig").ItemInfo;
const equip = @import("../game/equip.zig");
const hero_mod = @import("../game/hero.zig");
const Hero = hero_mod.Hero;
const Unit = @import("../game/unit.zig").Unit;
const unit_mod = @import("../game/unit.zig");
const army = @import("../game/army.zig");
const player = @import("../game/player.zig");
const movement = @import("../game/movement.zig");
const mapgen = @import("../game/mapgen.zig");
const Shard = mapgen.Shard;

/// interface.dat 2HeroTrade (left) and 28GarrisonScreen (right).
const gui_hero_trade = 2;
/// buttons.dat: exit, the trash.
const butt_exit = 226;
const butt_trash = 229;
/// dialog.var 1462 "Выбросить предмет?", asked when arMess[4] is on.
const dlg_delete_item = 1462;
const msg_delete = 4;

const Held = struct {
    unit: Unit,
    /// 0 the hero, 1 the garrison; -100 bought, not placed yet.
    from: i32,
    slot: i32,
    merc: bool = false,
};

pub const HeroGarrison = struct {
    s: *Shard,
    k: usize,
    g: usize,
    sh: Sheets,
    hs: hero_ui.HeroSheets,
    items: hero_ui.ItemBlock,
    walker: movement.Walker,
    distance: i32,
    unit_shape: rl.Image,
    hero_units: army_ui.UnitBlock = undefined,
    garr_units: army_ui.UnitBlock = undefined,
    recruit: army_ui.RecruitBlock = undefined,
    res: castle_ui.ResourceBar = .{ .x = 512, .y = 734 },
    info: hero_ui.InfoBlock,
    stat: hero_ui.StatBlock = .{ .x = 8, .y = 278 },
    trash1: castle_ui.Trash,
    trash2: castle_ui.Trash,
    exit1: Button,
    exit2: Button,
    held: ?Held = null,
    item: ?equip.Held = null,
    card: ?struct { item: i32, dur: i32 } = null,
    hint: Hint = .{},
    pointer: input.Pointer = .{},
    answer: ?usize = null,
    asking: bool = false,
    old_cursor: u32,

    /// `k` indexes Shard.heroes, `g` Shard.garrisons (the province's).
    pub fn create(c: *Ctx, s: *Shard, k: usize, g: usize, exp_pct: i32) !scene.Scene {
        const self = try c.gpa.create(HeroGarrison);
        errdefer c.gpa.destroy(self);
        const h = &s.heroes.items[k];
        self.* = .{
            .s = s,
            .k = k,
            .g = g,
            .sh = undefined,
            .hs = undefined,
            .items = undefined,
            .walker = undefined,
            .distance = 0,
            .unit_shape = undefined,
            .info = undefined,
            .trash1 = undefined,
            .trash2 = undefined,
            .exit1 = undefined,
            .exit2 = undefined,
            .old_cursor = c.cursor,
        };
        self.sh = try Sheets.load(c);
        errdefer self.sh.deinit();
        self.hs = try hero_ui.HeroSheets.load(c);
        errdefer self.hs.deinit();
        self.items = try hero_ui.ItemBlock.init(&self.sh, 207, 14);
        errdefer self.items.deinit();
        self.walker = try movement.Walker.init(c.gpa, s, c.proto, c.cfg.max_path);
        errdefer self.walker.deinit(c.gpa);
        self.unit_shape = self.sh.iface.imageIndex(gui.unit_mask) orelse return error.MissingMask;
        errdefer rl.unloadImage(self.unit_shape);
        self.hero_units = army_ui.UnitBlock.init(59, 411);
        self.hero_units.of_hero = true;
        self.garr_units = army_ui.UnitBlock.init(571, 373);
        self.recruit = army_ui.RecruitBlock.init(512, 0);
        const m = self.market(c);
        self.distance = m.unitDistance(&self.walker, h.province);
        self.recruit.recalc(m, self.distance);
        self.info = hero_ui.InfoBlock.init(c, h, 0, 0, movement.castleParam(s, c.proto, h.player, movement.abil_siege, true), movement.castleParam(s, c.proto, h.player, movement.abil_mobility, true), exp_pct);
        self.trash1 = try castle_ui.Trash.init(&self.sh, 355, 504);
        errdefer self.trash1.deinit();
        self.trash1.sound = sound.dismiss;
        self.trash2 = try castle_ui.Trash.init(&self.sh, 867, 460);
        errdefer self.trash2.deinit();
        self.trash2.sound = sound.dismiss;
        self.exit1 = try Button.init(&self.sh.buttons, 383, 582, butt_exit, butt_exit + 1, butt_exit + 2);
        errdefer self.exit1.deinit();
        self.exit1.hotkey = .escape;
        self.exit1.hint = 135;
        self.exit2 = try Button.init(&self.sh.buttons, 895, 540, butt_exit, butt_exit + 1, butt_exit + 2);
        errdefer self.exit2.deinit();
        self.exit2.hint = 135;
        c.setCursor(0);
        return scene.Scene.of(HeroGarrison, self, false);
    }

    pub fn deinit(self: *HeroGarrison, c: *Ctx) void {
        self.putBack();
        c.setCursor(self.old_cursor);
        self.exit2.deinit();
        self.exit1.deinit();
        self.trash2.deinit();
        self.trash1.deinit();
        rl.unloadImage(self.unit_shape);
        self.walker.deinit(c.gpa);
        self.items.deinit();
        self.hs.deinit();
        self.sh.deinit();
    }

    fn hero(self: *HeroGarrison) *Hero {
        return &self.s.heroes.items[self.k];
    }

    fn garrison(self: *HeroGarrison) *Hero {
        return &self.s.garrisons.items[self.g];
    }

    fn market(self: *HeroGarrison, c: *const Ctx) player.Market {
        return .{ .s = self.s, .pr = c.proto, .player = self.hero().player };
    }

    fn armyOf(self: *HeroGarrison, which: i32) *Hero {
        return if (which == 0) self.hero() else self.garrison();
    }

    /// What is in the hand goes back where it was taken.  A bought unit
    /// that was not placed disappears, as at the end of ScreenHeroGarrison.
    fn putBack(self: *HeroGarrison) void {
        if (self.held) |held| {
            if (held.from >= 0 and held.slot >= 0) self.armyOf(held.from).squad[@intCast(held.slot)] = held.unit;
            self.held = null;
        }
        if (self.item) |held| self.hero().inventory[held.slot] = .{ held.item, held.dur };
        self.item = null;
        self.refreshTrash();
    }

    fn refreshTrash(self: *HeroGarrison) void {
        const on = self.held != null or self.item != null;
        self.trash1.on = on;
        self.trash2.on = on;
    }

    fn drop(self: *HeroGarrison) void {
        self.held = null;
        self.item = null;
        self.refreshTrash();
    }

    fn newUnit(self: *HeroGarrison, c: *Ctx, t: i32, merc: bool) Unit {
        const m = self.market(c);
        if (merc) {
            const most = unit_mod.expForLevel(c.proto, t, 3 * m.bldParam1(player.abil.merc_level));
            var u = Unit.init(c.proto, &c.rand, t, @intCast(c.rand.random(@intCast(@max(most, 0)))));
            u.medals[0] = 6;
            return u;
        }
        return Unit.init(c.proto, &c.rand, t, m.bldParam1(player.abil.recruit_exp));
    }

    fn levelFits(c: *const Ctx, u: Unit, slot: usize) bool {
        return c.proto.units[@intCast(u.type)].level <= army.slotLevel(slot);
    }

    fn slotOpen(self: *HeroGarrison, c: *const Ctx, which: i32, slot: usize) bool {
        return slot < army.openSlots(self.armyOf(which), c.proto, @intCast(army.slotLevel(slot)))[1];
    }

    pub fn update(self: *HeroGarrison, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        self.pointer = f.pointer;
        self.card = null;
        if (self.asking) {
            self.asking = false;
            if (self.answer == 1) self.drop();
            self.answer = null;
        }
        if (self.exit1.check(f) or self.exit2.check(f)) return .pop;
        if (self.exit1.hot()) self.hint.phrase(c, self.exit1.hint, f.pointer.pos);
        if (self.exit2.hot()) self.hint.phrase(c, self.exit2.hint, f.pointer.pos);
        var action: scene.Action = .none;
        const m = self.market(c);

        // GUI_RecruitBlock: a unit for hire.
        self.recruit.recalc(m, self.distance);
        if (self.recruit.check(c, m, &self.unit_shape, f.pointer, &self.hint)) |pick| {
            self.putBack();
            self.held = .{ .unit = self.newUnit(c, pick.unit, pick.merc), .from = -100, .slot = 0, .merc = pick.merc };
            self.refreshTrash();
        }

        if (self.updateItems(c, f.pointer)) |a| action = a;

        // GUI_UnitBlock of the hero and of the garrison.
        if (self.hero_units.check(c, self.hero(), &self.unit_shape, f.pointer, &self.hint)) |slot| {
            if (self.clickUnit(c, f, 0, slot)) |a| action = a;
        }
        if (action == .none) if (self.garr_units.check(c, self.garrison(), &self.unit_shape, f.pointer, &self.hint)) |slot| {
            if (self.clickUnit(c, f, 1, slot)) |a| action = a;
        };

        // The trash: an item (dialog 1462) or a dismissed unit, with the
        // refund of ability 24.
        if (action == .none and (self.trash1.check(f) or self.trash2.check(f))) {
            if (self.item != null) {
                if (c.human.message(msg_delete) and dlg_delete_item < c.proto.dialogs.len) {
                    self.asking = true;
                    return .{ .push = Dialog.create(c, dlg_delete_item, .{ true, true, false, false }, &self.answer) catch {
                        self.asking = false;
                        return .none;
                    } };
                }
                self.drop();
            } else if (self.held) |held| {
                if (held.from >= 0) self.dismiss(c, held.unit);
                self.drop();
            }
        }

        self.info.refresh(c, self.hero());
        if (action == .none and self.info.portraitClick(&self.hs, self.hero(), f.pointer)) {
            action = .{ .push = army_info.UnitWindow.createHero(c, self.hero(), self.info.exp_pct) catch return .none };
        }
        const abil_click = self.info.update(c, self.hero(), f.pointer, &self.hint) orelse self.stat.update(c, self.hero(), f.pointer, &self.hint);
        if (action == .none) if (self.res.update(c, m, f.pointer, &self.hint)) |r| {
            action = @import("garrison.zig").resourceInfo(c, m, r);
        };
        if (action == .none) action = self.openInfo(c);
        if (action == .none) {
            if (abil_click) |a| action = @import("info_windows.zig").abilityAction(c, a.num, a.power);
        }
        if (action == .none and self.held == null and self.item == null) self.drop();
        return action;
    }

    /// GUI_ItemBlock: move an item between the doll and the backpack.  A
    /// click on the doll chooses its fitting place; a right click opens its
    /// WindowItemInfo.
    fn updateItems(self: *HeroGarrison, c: *Ctx, p: input.Pointer) ?scene.Action {
        const pr = c.proto;
        const h = self.hero();
        const inv = self.items.check(p);
        if (inv.under) |u| if (u > 0 and h.inventory[u][0] > 0) {
            self.card = .{ .item = h.inventory[u][0], .dur = h.inventory[u][1] };
        };
        if (inv.click) |k_in| {
            if (self.item) |held| {
                const k = if (k_in == 0) equip.dollTarget(pr, h, held.item) else k_in;
                const tr = &self.s.rulers[@intCast(h.player)].holdings.treasury;
                if (equip.dropOnHero(pr, h, tr, held, k, null)) |r| self.item = r.held;
                self.refreshTrash();
            } else if (k_in > 0 and h.inventory[k_in][0] > 0) {
                // Taking an item first puts back a unit already in hand.
                self.putBack();
                self.item = .{ .item = h.inventory[k_in][0], .dur = h.inventory[k_in][1], .slot = k_in, .owner = .hero };
                h.inventory[k_in] = .{ 0, 0 };
                self.refreshTrash();
            }
        }
        if (inv.info) |k| if (k > 0 and h.inventory[k][0] > 0) {
            return .{ .push = ItemInfo.create(c, h.inventory[k][0], h.inventory[k][1]) catch return null };
        };
        return null;
    }

    /// A click on a unit slot of the hero (0) or the garrison (1).
    fn clickUnit(self: *HeroGarrison, c: *Ctx, f: *const input.Frame, which: i32, slot: usize) ?scene.Action {
        if (!self.slotOpen(c, which, slot)) return null;
        const sq = &self.armyOf(which).squad;
        if (self.held) |held| {
            if (sq[slot]) |other| {
                // Занятый слот: the other unit goes to a free slot.
                const other_level = c.proto.units[@intCast(other.type)].level;
                if (held.from < 0) return null;
                const from_slot: usize = @intCast(held.slot);
                if (held.from == which and army.slotLevel(from_slot) >= other_level) {
                    self.armyOf(which).squad[from_slot] = null;
                    army.placeUnit(self.armyOf(which), other, from_slot);
                } else {
                    const free = army.freeSlot(self.armyOf(held.from), c.proto, other_level);
                    if (free == 0) return null;
                    army.placeUnit(self.armyOf(held.from), other, free);
                }
                sq[slot] = null;
            } else if (!levelFits(c, held.unit, slot)) return null;
            if (held.from < 0) {
                // Купленный юнит: pay for it.
                const price = self.priceOf(c, held.unit.type, held.merc);
                if (!self.market(c).affords(price)) return null;
                army.placeUnit(self.armyOf(which), held.unit, slot);
                self.pay(c, held, price);
                const again = !held.merc and f.ctrl and self.market(c).affords(price);
                if (again) {
                    self.held = .{ .unit = self.newUnit(c, held.unit.type, false), .from = -100, .slot = 0 };
                } else self.drop();
            } else {
                army.placeUnit(self.armyOf(which), held.unit, slot);
                self.drop();
            }
            return null;
        }
        if (sq[slot]) |u| {
            // Taking a unit first puts back an item already in hand.
            if (self.item != null) self.putBack();
            self.held = .{ .unit = u, .from = which, .slot = @intCast(slot) };
            sq[slot] = null;
            self.refreshTrash();
        }
        return null;
    }

    fn priceOf(self: *HeroGarrison, c: *const Ctx, t: i32, merc: bool) player.Price {
        const m = self.market(c);
        var p = m.unitPrice(t, self.distance);
        if (merc) p = .{ p[0] * 2, p[1] * 2 };
        return p;
    }

    fn pay(self: *HeroGarrison, c: *Ctx, held: Held, price: player.Price) void {
        const m = self.market(c);
        sound.play(.unit, sound.buy);
        m.pay(price);
        if (held.merc) m.holdings().recruit[player.merc_slot] = 0;
        m.raiseResPrices(&c.proto.units[@intCast(held.unit.type)].res);
    }

    /// Dismiss a unit from a slot: ability 24 pays part of its price back.
    fn dismiss(self: *HeroGarrison, c: *Ctx, u: Unit) void {
        const m = self.market(c);
        if (m.bldParam1(player.abil.dismiss_refund) <= 0 or u.medals[0] == 6) return;
        const t = &c.proto.units[@intCast(u.type)];
        const price = @max(@divTrunc((t.gold_price + t.gem_price * 2) * (u.level + 1), 10), 1);
        m.ruler().gold += price;
    }

    /// The window a right click asked for.
    fn openInfo(self: *HeroGarrison, c: *Ctx) scene.Action {
        const m = self.market(c);
        if (self.hero_units.info) |info| {
            self.hero_units.info = null;
            self.garr_units.info = null;
            self.recruit.info = null;
            return self.showInfo(c, m, 0, info);
        }
        if (self.garr_units.info) |info| {
            self.hero_units.info = null;
            self.garr_units.info = null;
            self.recruit.info = null;
            return self.showInfo(c, m, 1, info);
        }
        const info = self.recruit.info orelse return .none;
        self.recruit.info = null;
        return self.showInfo(c, m, -1, info);
    }

    fn showInfo(self: *HeroGarrison, c: *Ctx, m: player.Market, which: i32, info: army_ui.Info) scene.Action {
        const sc = switch (info) {
            .unit => |i| blk: {
                const g = self.armyOf(which);
                const u = g.squad[i] orelse return .none;
                break :blk army_info.UnitWindow.createUnit(c, u, hero_mod.squadMorale(c.proto, &g.squad, i, m.ruler().karma, null));
            },
            .unit_type => |t| army_info.UnitWindow.createType(c, t),
            .resource => |r| army_info.ResourceWindow.create(c, r, m),
        } catch |e| {
            std.log.err("info window: {t}", .{e});
            return .none;
        };
        return .{ .push = sc };
    }

    pub fn draw(self: *HeroGarrison, c: *Ctx) void {
        const m = self.market(c);
        rl.clearBackground(.black);
        gfx.drawTex(self.sh.iface.texIndex(gui_hero_trade), 0, 0);
        gfx.drawTex(self.sh.iface.texIndex(gui.garrison), 512, 0);
        const h = self.hero();
        const g = self.garrison();
        const held_unit: ?Unit = if (self.held) |x| x.unit else null;
        self.hero_units.draw(c, &self.sh, h, m.ruler().karma, held_unit);
        self.garr_units.draw(c, &self.sh, g, m.ruler().karma, held_unit);
        self.recruit.draw(c, &self.sh, m);
        self.res.draw(c, &self.sh, m);
        self.items.draw(c, &self.sh, h, if (self.item) |held| held.item else 0);
        self.stat.draw(c, h);
        self.info.draw(c, &self.sh, &self.hs, h);
        self.trash1.draw(&self.sh);
        self.trash2.draw(&self.sh);
        self.exit1.draw(&self.sh.buttons, c.tick);
        self.exit2.draw(&self.sh.buttons, c.tick);
        if (self.held) |held| castle_ui.drawHeld(&self.sh, self.sh.unitPic(held.unit.type), self.pointer);
        if (self.item) |held| castle_ui.drawHeld(&self.sh, self.sh.item(c.proto, held.item), self.pointer);
        if (self.card) |cd| {
            castle_ui.itemCard(c, &self.sh, self.pointer.pos, cd.item, cd.dur, h, 0, .{ 0, 0 }, .{ 0, 0 });
        } else self.hint.draw(c, &self.sh.iface);
    }
};
