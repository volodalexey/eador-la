//! ScreenHeroHero (Screens.cpp:2004): two heroes of a province side by
//! side, each with GUI_InfoBlock, GUI_StatBlock, GUI_ItemBlock and
//! GUI_UnitBlock.  One hand holds an item or a unit of either hero: items
//! move between the dolls and backpacks (game/equip.zig), units between
//! the squads; on a taken place the two change hands when the old one fits
//! the giver.  Either trash throws away what is held (dialogs 1461/1462
//! when arMess[4] is on).  A right click opens WindowItemInfo or
//! WindowUnitInfo, a click on a stat, the level or the life
//! WindowAbilityInfo.  On exit the held thing goes back where it came from.
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
const info_windows = @import("info_windows.zig");
const army_info = @import("army_info.zig");
const Dialog = @import("dialog.zig").Dialog;
const ItemInfo = @import("item_info.zig").ItemInfo;
const hero_mod = @import("../game/hero.zig");
const Hero = hero_mod.Hero;
const Unit = @import("../game/unit.zig").Unit;
const army = @import("../game/army.zig");
const equip = @import("../game/equip.zig");
const movement = @import("../game/movement.zig");
const mapgen = @import("../game/mapgen.zig");
const Shard = mapgen.Shard;

/// interface.dat 2HeroTrade, drawn at x 0 and x 512.
const gui_hero_trade = 2;
/// buttons.dat: exit 226..228, the trash 229..232.
const butt_exit = 226;
const butt_trash = 229;
const ph_exit = 135;
/// dialog.var 1461 "Выгнать юнита?", 1462 "Выбросить предмет?"; asked
/// when arMess[4] is on.
const dlg_delete_unit = 1461;
const dlg_delete_item = 1462;
const msg_delete = 4;

/// SelectedItem/SelectedUnit with SelectedSlot and SelectedHero.
const Held = union(enum) {
    item: struct { item: i32, dur: i32, slot: usize, from: u1 },
    unit: struct { unit: Unit, slot: usize, from: u1 },
};

pub const HeroHero = struct {
    s: *Shard,
    k: [2]usize,
    sh: Sheets,
    hs: hero_ui.HeroSheets,
    unit_shape: rl.Image,
    items: [2]hero_ui.ItemBlock,
    units: [2]army_ui.UnitBlock,
    stats: [2]hero_ui.StatBlock = .{ .{ .x = 8, .y = 278 }, .{ .x = 520, .y = 278 } },
    infos: [2]hero_ui.InfoBlock,
    trash: [2]Button,
    exit: [2]Button,
    held: ?Held = null,
    /// The item card under the pointer and whose it is.
    card: ?struct { item: i32, dur: i32, side: u1 } = null,
    hint: Hint = .{},
    pointer: input.Pointer = .{},
    answer: ?usize = null,
    asking: bool = false,
    old_cursor: u32,

    /// `k1`, `k2` index Shard.heroes (Hero1, Hero2).
    pub fn create(c: *Ctx, s: *Shard, k1: usize, k2: usize, exp_pct: i32) !scene.Scene {
        const self = try c.gpa.create(HeroHero);
        errdefer c.gpa.destroy(self);
        self.* = .{
            .s = s,
            .k = .{ k1, k2 },
            .sh = undefined,
            .hs = undefined,
            .unit_shape = undefined,
            .items = undefined,
            .units = .{ army_ui.UnitBlock.init(59, 411), army_ui.UnitBlock.init(571, 411) },
            .infos = undefined,
            .trash = undefined,
            .exit = undefined,
            .old_cursor = c.cursor,
        };
        for (&self.units) |*u| u.of_hero = true;
        for (&self.infos, 0..) |*b, i| {
            const h = &s.heroes.items[self.k[i]];
            b.* = hero_ui.InfoBlock.init(c, h, @floatFromInt(512 * i), 0, movement.castleParam(s, c.proto, h.player, movement.abil_siege, true), movement.castleParam(s, c.proto, h.player, movement.abil_mobility, true), exp_pct);
        }
        self.sh = try Sheets.load(c);
        errdefer self.sh.deinit();
        self.hs = try hero_ui.HeroSheets.load(c);
        errdefer self.hs.deinit();
        self.unit_shape = self.sh.iface.imageIndex(gui.unit_mask) orelse return error.MissingMask;
        errdefer rl.unloadImage(self.unit_shape);
        self.items[0] = try hero_ui.ItemBlock.init(&self.sh, 207, 14);
        errdefer self.items[0].deinit();
        self.items[1] = try hero_ui.ItemBlock.init(&self.sh, 719, 14);
        errdefer self.items[1].deinit();
        self.trash[0] = try Button.init(&self.sh.buttons, 355, 504, butt_trash, butt_trash + 1, butt_trash + 2);
        errdefer self.trash[0].deinit();
        self.trash[1] = try Button.init(&self.sh.buttons, 867, 504, butt_trash, butt_trash + 1, butt_trash + 2);
        errdefer self.trash[1].deinit();
        for (&self.trash) |*t| {
            t.inactive = butt_trash + 3;
            t.on = false;
            t.sound = sound.dismiss;
        }
        self.trash[0].hotkey = .delete;
        self.exit[0] = try Button.init(&self.sh.buttons, 383, 582, butt_exit, butt_exit + 1, butt_exit + 2);
        errdefer self.exit[0].deinit();
        self.exit[1] = try Button.init(&self.sh.buttons, 895, 582, butt_exit, butt_exit + 1, butt_exit + 2);
        for (&self.exit) |*e| e.hint = ph_exit;
        self.exit[0].hotkey = .escape;
        c.setCursor(0);
        return scene.Scene.of(HeroHero, self, false);
    }

    pub fn deinit(self: *HeroHero, c: *Ctx) void {
        // The end of ScreenHeroHero: what is held goes back to its slot.
        if (self.held) |held| switch (held) {
            .item => |it| self.hero(it.from).inventory[it.slot] = .{ it.item, it.dur },
            .unit => |u| self.hero(u.from).squad[u.slot] = u.unit,
        };
        c.setCursor(self.old_cursor);
        for (&self.exit) |*e| e.deinit();
        for (&self.trash) |*t| t.deinit();
        for (&self.items) |*b| b.deinit();
        rl.unloadImage(self.unit_shape);
        self.hs.deinit();
        self.sh.deinit();
    }

    fn hero(self: *const HeroHero, side: u1) *Hero {
        return &self.s.heroes.items[self.k[side]];
    }

    fn setHeld(self: *HeroHero, h: ?Held) void {
        self.held = h;
        for (&self.trash) |*t| t.on = h != null;
    }

    fn karma(self: *const HeroHero, side: u1) i32 {
        const p = self.hero(side).player;
        return if (p > 0) self.s.rulers[@intCast(p)].karma else 0;
    }

    pub fn update(self: *HeroHero, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        const p = f.pointer;
        self.pointer = p;
        self.hint.clear();
        self.card = null;
        if (self.asking) {
            self.asking = false;
            if (self.answer == 1) self.setHeld(null);
            self.answer = null;
        }
        if (self.exit[0].check(f) or self.exit[1].check(f)) return .pop;

        var action: scene.Action = .none;
        for ([2]u1{ 0, 1 }) |a| {
            if (self.updateItems(c, a, p)) |act| action = act;
            if (self.updateUnits(c, a, p)) |act| action = act;
        }

        if (self.trash[0].check(f) or self.trash[1].check(f)) if (self.held) |held| {
            const dlg: usize = if (held == .unit) dlg_delete_unit else dlg_delete_item;
            if (c.human.message(msg_delete) and dlg < c.proto.dialogs.len) {
                self.asking = true;
                return .{ .push = Dialog.create(c, dlg, .{ true, true, false, false }, &self.answer) catch {
                    self.asking = false;
                    return .none;
                } };
            }
            // dcUnit::Dismiss or the item thrown away.
            self.setHeld(null);
        };

        for ([2]u1{ 0, 1 }) |a| {
            const h = self.hero(a);
            self.infos[a].refresh(c, h);
            if (action == .none and self.infos[a].portraitClick(&self.hs, h, p)) {
                action = .{ .push = army_info.UnitWindow.createHero(c, h, self.infos[a].exp_pct) catch return .none };
            }
            const click = self.infos[a].update(c, h, p, &self.hint) orelse self.stats[a].update(c, h, p, &self.hint);
            if (action == .none) if (click) |ab| {
                action = info_windows.abilityAction(c, ab.num, ab.power);
            };
        }
        for (&self.exit) |*e| if (e.hot()) self.hint.phrase(c, e.hint, p.pos);
        return action;
    }

    /// "Обмен предметами" on the item block of hero `a`.
    fn updateItems(self: *HeroHero, c: *Ctx, a: u1, p: input.Pointer) ?scene.Action {
        const pr = c.proto;
        const h = self.hero(a);
        const inv = self.items[a].check(p);
        if (inv.under) |u| if (u > 0 and h.inventory[u][0] > 0) {
            self.card = .{ .item = h.inventory[u][0], .dur = h.inventory[u][1], .side = a };
        };
        if (inv.click) |k_in| {
            const held_item = if (self.held) |held| switch (held) {
                .item => |it| it,
                .unit => null,
            } else null;
            if (held_item) |it| {
                // "Попытка поместить предмет в...".
                const k = if (k_in == 0) equip.dollTarget(pr, h, it.item) else k_in;
                const eh: equip.Held = .{ .item = it.item, .dur = it.dur, .slot = it.slot, .owner = .hero };
                const done = if (it.from == a) blk: {
                    const tr = &self.s.rulers[@intCast(@max(h.player, 0))].holdings.treasury;
                    break :blk equip.dropOnHero(pr, h, tr, eh, k, null) != null;
                } else equip.dropOnOther(pr, h, self.hero(it.from), eh, k);
                if (done) self.setHeld(null);
            } else if (k_in > 0 and h.inventory[k_in][0] > 0) {
                // "Взятие предмета": a unit in the hand goes back first.
                if (self.held) |held| {
                    const u = held.unit;
                    army.place(self.hero(u.from), pr, u.unit, u.slot);
                }
                self.setHeld(.{ .item = .{ .item = h.inventory[k_in][0], .dur = h.inventory[k_in][1], .slot = k_in, .from = a } });
                h.inventory[k_in] = .{ 0, 0 };
            }
        }
        if (inv.info) |k| if (k > 0 and h.inventory[k][0] > 0) {
            return .{ .push = ItemInfo.create(c, h.inventory[k][0], h.inventory[k][1]) catch return null };
        };
        return null;
    }

    /// "Обмен юнитами" on the unit block of hero `a`.
    fn updateUnits(self: *HeroHero, c: *Ctx, a: u1, p: input.Pointer) ?scene.Action {
        const pr = c.proto;
        const h = self.hero(a);
        if (self.units[a].check(c, h, &self.unit_shape, p, &self.hint)) |slot| {
            if (self.held) |held| switch (held) {
                .unit => |u| self.dropUnit(c, a, slot, u.unit, u.from, u.slot),
                .item => if (h.squad[slot]) |unit| {
                    // "Взятие юнита": an item in the hand goes back first.
                    const it = held.item;
                    self.hero(it.from).inventory[it.slot] = .{ it.item, it.dur };
                    self.setHeld(.{ .unit = .{ .unit = unit, .slot = slot, .from = a } });
                    h.squad[slot] = null;
                },
            } else if (h.squad[slot]) |unit| {
                self.setHeld(.{ .unit = .{ .unit = unit, .slot = slot, .from = a } });
                h.squad[slot] = null;
            }
        }
        if (self.units[a].info) |info| {
            self.units[a].info = null;
            if (info == .unit) if (h.squad[info.unit]) |u| {
                return .{ .push = army_info.UnitWindow.createUnit(c, u, h.squadMoraleOf(pr, info.unit, self.karma(a))) catch return null };
            };
        }
        return null;
    }

    /// The held unit put on slot `slot` of hero `a`: on a taken slot the
    /// unit there goes to the giver's slot if its level fits, else to the
    /// giver's first free slot of its level; without one nothing happens.
    fn dropUnit(self: *HeroHero, c: *Ctx, a: u1, slot: usize, unit: Unit, from: u1, from_slot: usize) void {
        const pr = c.proto;
        if (army.slotLevel(slot) < pr.units[@intCast(unit.type)].level) return;
        const h = self.hero(a);
        const giver = self.hero(from);
        if (h.squad[slot]) |other| {
            const lv = pr.units[@intCast(other.type)].level;
            const free = army.freeSlot(giver, pr, lv);
            if (free == 0) return;
            army.placeUnit(giver, other, if (army.slotLevel(from_slot) < lv) free else from_slot);
        }
        h.squad[slot] = null;
        army.placeUnit(h, unit, slot);
        self.setHeld(null);
    }

    pub fn draw(self: *HeroHero, c: *Ctx) void {
        const pr = c.proto;
        rl.clearBackground(.black);
        gfx.drawTex(self.sh.iface.texIndex(gui_hero_trade), 0, 0);
        gfx.drawTex(self.sh.iface.texIndex(gui_hero_trade), 512, 0);
        const held_item: i32 = if (self.held) |h| switch (h) {
            .item => |it| it.item,
            .unit => 0,
        } else 0;
        const held_unit: ?Unit = if (self.held) |h| switch (h) {
            .unit => |u| u.unit,
            .item => null,
        } else null;
        for ([2]u1{ 0, 1 }) |a| {
            const h = self.hero(a);
            self.infos[a].draw(c, &self.sh, &self.hs, h);
            self.stats[a].draw(c, h);
            self.items[a].draw(c, &self.sh, h, held_item);
            self.units[a].draw(c, &self.sh, h, self.karma(a), held_unit);
        }
        for (&self.trash) |*t| t.draw(&self.sh.buttons, c.tick);
        for (&self.exit) |*e| e.draw(&self.sh.buttons, c.tick);
        if (self.held) |h| switch (h) {
            .item => |it| castle_ui.drawHeld(&self.sh, self.sh.item(pr, it.item), self.pointer),
            .unit => |u| castle_ui.drawHeld(&self.sh, self.sh.unitPic(u.unit.type), self.pointer),
        };
        if (self.card) |cd| {
            castle_ui.itemCard(c, &self.sh, self.pointer.pos, cd.item, cd.dur, self.hero(cd.side), 0, .{ 0, 0 }, .{ 0, 0 });
        } else self.hint.draw(c, &self.sh.iface);
    }
};
