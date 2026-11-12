//! The garrison screens: HomeGarrison (Castle.cpp:1813), the garrison tab
//! of the castle, and ScreenGarrison (Screens.cpp:5813), the garrison of a
//! province opened from the map.  Both share ArmyPanel: hire from the
//! recruit block (Ctrl keeps the unit for another purchase, the mercenary
//! costs double), move units between the squad slots, dismiss with the
//! trash button; no hiring while the province is besieged (dialog 387).
//! ScreenGarrison also refunds a dismissed unit when the castle has
//! ability 24 (dialog 1090).  A right click opens WindowUnitInfo on a
//! unit and WindowUnitTypeInfo on a unit for hire, a click on a resource
//! WindowResourceInfo (scenes/army_info.zig).
//!
//! HomeGarrison keeps the bought unit itself selected after a Ctrl
//! purchase (the same dcUnit stands in the slot and under the mouse);
//! here both screens take a new unit as ScreenGarrison does.
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
const Dialog = @import("dialog.zig").Dialog;
const army_info = @import("army_info.zig");
const hero_mod = @import("../game/hero.zig");
const mapgen = @import("../game/mapgen.zig");
const Shard = mapgen.Shard;
const Hero = @import("../game/hero.zig").Hero;
const unit_mod = @import("../game/unit.zig");
const Unit = unit_mod.Unit;
const army = @import("../game/army.zig");
const player = @import("../game/player.zig");
const movement = @import("../game/movement.zig");

/// dialog.var: 387 no hiring during a siege, 1090 the refund.
const dlg_siege = 387;
const dlg_refund = 1090;
/// arMess[3]: report the refund.
const msg_refund = 3;
/// arMedal[0] = 6 marks a mercenary.
const medal_merc = 6;

pub const ArmyPanel = struct {
    s: *Shard,
    player: i32,
    /// Index into Shard.garrisons and the garrison's province.
    garrison: usize,
    province: i32,
    /// MoveMap turns from the province to the capital (GetUnitGoldPrice).
    distance: i32,
    can_hire: bool,
    /// ScreenGarrison: the refund of ability 24.
    refund: bool,
    units: army_ui.UnitBlock,
    recruit: army_ui.RecruitBlock,
    trash: castle_ui.Trash,
    unit_shape: rl.Image,
    /// SelectedUnit, SelectedSlot (-100: bought, not placed yet), bMerc.
    held: ?Unit = null,
    held_slot: i32 = 0,
    merc: bool = false,
    dialog_result: ?usize = null,
    text: [512]u8 = undefined,

    pub const Layout = struct { recruit: [2]f32, units: [2]f32, trash: [2]f32 };

    pub fn init(c: *Ctx, sh: *Sheets, s: *Shard, w: *movement.Walker, k: usize, g: usize, lay: Layout, refund: bool) !ArmyPanel {
        const gar = &s.garrisons.items[g];
        const m: player.Market = .{ .s = s, .pr = c.proto, .player = @intCast(k) };
        var self: ArmyPanel = .{
            .s = s,
            .player = @intCast(k),
            .garrison = g,
            .province = gar.province,
            .distance = m.unitDistance(w, gar.province),
            .can_hire = movement.siege(s, gar.province) < 0,
            .refund = refund,
            .units = army_ui.UnitBlock.init(lay.units[0], lay.units[1]),
            .recruit = army_ui.RecruitBlock.init(lay.recruit[0], lay.recruit[1]),
            .trash = try castle_ui.Trash.init(sh, lay.trash[0], lay.trash[1]),
            .unit_shape = undefined,
        };
        errdefer self.trash.deinit();
        if (refund) self.trash.sound = sound.dismiss;
        self.unit_shape = sh.iface.imageIndex(gui.unit_mask) orelse return error.MissingMask;
        self.recruit.recalc(m, self.distance);
        return self;
    }

    pub fn deinit(self: *ArmyPanel) void {
        rl.unloadImage(self.unit_shape);
        self.trash.deinit();
    }

    fn hero(self: *ArmyPanel) *Hero {
        return &self.s.garrisons.items[self.garrison];
    }

    pub fn market(self: *const ArmyPanel, c: *const Ctx) player.Market {
        return .{ .s = self.s, .pr = c.proto, .player = self.player };
    }

    /// The end of the screen: a unit taken from a slot goes back there.
    pub fn close(self: *ArmyPanel) void {
        if (self.held) |u| if (self.held_slot >= 0) {
            self.hero().squad[@intCast(self.held_slot)] = u;
        };
        self.held = null;
    }

    fn drop(self: *ArmyPanel) void {
        self.held = null;
        self.held_slot = 0;
        self.trash.on = false;
    }

    fn newUnit(self: *ArmyPanel, c: *Ctx, t: i32, merc: bool) Unit {
        const m = self.market(c);
        if (merc) {
            const most = unit_mod.expForLevel(c.proto, t, 3 * m.bldParam1(player.abil.merc_level));
            var u = Unit.init(c.proto, &c.rand, t, @intCast(c.rand.random(@intCast(@max(most, 0)))));
            u.medals[0] = medal_merc;
            return u;
        }
        return Unit.init(c.proto, &c.rand, t, m.bldParam1(player.abil.recruit_exp));
    }

    pub fn update(self: *ArmyPanel, c: *Ctx, f: *const input.Frame, hint: *Hint) scene.Action {
        const m = self.market(c);
        const g = self.hero();
        if (self.recruit.check(c, m, &self.unit_shape, f.pointer, hint)) |pick| {
            if (self.held) |u| if (self.held_slot >= 0) army.placeUnit(g, u, @intCast(self.held_slot));
            self.merc = pick.merc;
            self.held = self.newUnit(c, pick.unit, pick.merc);
            self.held_slot = -100;
            self.trash.on = true;
        }
        var action: scene.Action = .none;
        if (self.units.check(c, g, &self.unit_shape, f.pointer, hint)) |slot| {
            if (self.held) |u| {
                const level = c.proto.units[@intCast(u.type)].level;
                if (self.held_slot < 0) {
                    if (!self.can_hire) {
                        self.drop();
                        if (Dialog.create(c, dlg_siege, .{ true, true, false, false }, &self.dialog_result)) |d| {
                            action = .{ .push = d };
                        } else |_| {}
                    } else if (g.squad[slot] == null and army.slotLevel(slot) >= level) self.buy(c, f, u, slot);
                } else if (army.slotLevel(slot) >= level) self.move(c, u, slot);
            } else if (g.squad[slot]) |u| {
                self.held = u;
                self.held_slot = @intCast(slot);
                g.squad[slot] = null;
                self.trash.on = true;
            }
        }
        if (self.trash.check(f)) {
            if (self.held) |u| if (self.held_slot >= 0) {
                if (self.dismiss(c, u)) |a| action = a;
            };
            self.drop();
        }
        if (action == .none) action = self.openInfo(c);
        return action;
    }

    /// The window a right click (or a resource click) asked for.
    fn openInfo(self: *ArmyPanel, c: *Ctx) scene.Action {
        const info = self.units.info orelse self.recruit.info orelse return .none;
        self.units.info = null;
        self.recruit.info = null;
        const m = self.market(c);
        const sc = switch (info) {
            .unit => |i| blk: {
                const g = self.hero();
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

    /// The purchase of the held unit into the empty slot.
    fn buy(self: *ArmyPanel, c: *Ctx, f: *const input.Frame, u: Unit, slot: usize) void {
        const m = self.market(c);
        sound.play(.unit, sound.buy);
        army.placeUnit(self.hero(), u, slot);
        var price = m.unitPrice(u.type, self.distance);
        if (self.merc) {
            price = .{ price[0] * 2, price[1] * 2 };
            m.holdings().recruit[player.merc_slot] = 0;
        }
        m.pay(price);
        m.raiseResPrices(&c.proto.units[@intCast(u.type)].res);
        self.recruit.recalc(m, self.distance);
        const again = !self.merc and f.ctrl and m.affords(m.unitPrice(u.type, self.distance));
        if (again) {
            self.held = self.newUnit(c, u.type, false);
        } else self.drop();
    }

    /// A unit taken from slot held_slot goes to `slot`; the one standing
    /// there takes the freed slot, or a free slot of its rank.
    fn move(self: *ArmyPanel, c: *Ctx, u: Unit, slot: usize) void {
        const g = self.hero();
        if (g.squad[slot]) |other| {
            const level = c.proto.units[@intCast(other.type)].level;
            const free = army.freeSlot(g, c.proto, level);
            if (free == 0) return;
            const from: usize = @intCast(self.held_slot);
            army.placeUnit(g, other, if (army.slotLevel(from) < level) free else from);
            g.squad[slot] = null;
        }
        army.placeUnit(g, u, slot);
        self.drop();
    }

    /// Dismiss; ScreenGarrison pays (GoldPrice + 2·GemPrice)·(level+1)/10
    /// back when the castle has ability 24, except for a mercenary.
    fn dismiss(self: *ArmyPanel, c: *Ctx, u: Unit) ?scene.Action {
        if (!self.refund) return null;
        const m = self.market(c);
        if (m.bldParam1(player.abil.dismiss_refund) <= 0 or u.medals[0] == medal_merc) return null;
        const t = &c.proto.units[@intCast(u.type)];
        const price = @max(@divTrunc((t.gold_price + t.gem_price * 2) * (u.level + 1), 10), 1);
        m.ruler().gold += price;
        if (!c.human.message(msg_refund) or dlg_refund >= c.proto.dialogs.len) return null;
        const body = formatInt(&self.text, c.proto.dialogs[dlg_refund].text, price);
        return .{ .push = Dialog.createText(c, dlg_refund, .{ true, true, false, false }, &self.dialog_result, body) catch return null };
    }

    pub fn draw(self: *const ArmyPanel, c: *Ctx, sh: *Sheets) void {
        const m = self.market(c);
        const g = &self.s.garrisons.items[self.garrison];
        self.recruit.draw(c, sh, m);
        self.units.draw(c, sh, g, m.ruler().karma, self.held);
        self.trash.draw(sh);
    }

    /// The mouse sprite: the held unit's picture.
    pub fn drawHeld(self: *const ArmyPanel, sh: *Sheets, p: input.Pointer) void {
        const u = self.held orelse return;
        castle_ui.drawHeld(sh, sh.unitPic(u.type), p);
    }
};

/// WindowResourceInfo of resource `r` for the bar's player.
pub fn resourceInfo(c: *Ctx, m: player.Market, r: i32) scene.Action {
    return .{ .push = army_info.ResourceWindow.create(c, r, m) catch |e| {
        std.log.err("resource window: {t}", .{e});
        return .none;
    } };
}

/// sprintf(text, n) for the one "%d" of a dialog text.
pub fn formatInt(buf: []u8, text: []const u8, n: i32) []const u8 {
    var w: std.Io.Writer = .fixed(buf);
    if (std.mem.indexOf(u8, text, "%d")) |i| {
        w.print("{s}{d}{s}", .{ text[0..i], n, text[i + 2 ..] }) catch {};
    } else w.writeAll(text) catch {};
    return w.buffered();
}

/// ScreenGarrison: the garrison of a province over the left half of the
/// map (the scene below), 28GarrisonScreen on the right.
pub const ProvinceGarrison = struct {
    sh: Sheets,
    walker: movement.Walker,
    panel: ArmyPanel,
    res: castle_ui.ResourceBar = .{ .x = 512, .y = 734 },
    exit: Button,
    hint: Hint = .{},
    old_cursor: u32,
    pointer: input.Pointer = .{},

    const layout: ArmyPanel.Layout = .{ .recruit = .{ 512, 0 }, .units = .{ 571, 373 }, .trash = .{ 867, 460 } };

    /// `g` is the garrison (Shard.garrisons index) of player `k`.
    pub fn create(c: *Ctx, s: *Shard, k: usize, g: usize) !scene.Scene {
        const self = try c.gpa.create(ProvinceGarrison);
        errdefer c.gpa.destroy(self);
        self.* = .{ .sh = undefined, .walker = undefined, .panel = undefined, .exit = undefined, .old_cursor = c.cursor };
        self.sh = try Sheets.load(c);
        errdefer self.sh.deinit();
        self.walker = try movement.Walker.init(c.gpa, s, c.proto, c.cfg.max_path);
        errdefer self.walker.deinit(c.gpa);
        self.panel = try ArmyPanel.init(c, &self.sh, s, &self.walker, k, g, layout, true);
        errdefer self.panel.deinit();
        self.exit = try Button.init(&self.sh.buttons, 895, 540, 226, 227, 228);
        self.exit.hint = 135;
        self.exit.hotkey = .escape;
        c.setCursor(0);
        return scene.Scene.of(ProvinceGarrison, self, true);
    }

    pub fn deinit(self: *ProvinceGarrison, c: *Ctx) void {
        self.panel.close();
        c.setCursor(self.old_cursor);
        self.exit.deinit();
        self.panel.deinit();
        self.walker.deinit(c.gpa);
        self.sh.deinit();
    }

    pub fn update(self: *ProvinceGarrison, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        self.pointer = f.pointer;
        if (self.exit.check(f)) return .pop;
        if (self.exit.hot()) self.hint.phrase(c, self.exit.hint, f.pointer.pos);
        const a = self.panel.update(c, f, &self.hint);
        if (self.res.update(c, self.panel.market(c), f.pointer, &self.hint)) |r| if (a == .none) return resourceInfo(c, self.panel.market(c), r);
        return a;
    }

    pub fn draw(self: *ProvinceGarrison, c: *Ctx) void {
        gfx.drawTex(self.sh.iface.texIndex(gui.castle), 0, 0);
        gfx.drawTex(self.sh.iface.texIndex(gui.garrison), 512, 0);
        self.panel.draw(c, &self.sh);
        self.res.draw(c, &self.sh, self.panel.market(c));
        self.exit.draw(&self.sh.buttons, c.tick);
        self.panel.drawHeld(&self.sh, self.pointer);
        self.hint.draw(c, &self.sh.iface);
    }
};

test "dialog text with the refund" {
    var b: [64]u8 = undefined;
    try std.testing.expectEqualStrings("Вернули 12 золота.", formatInt(&b, "Вернули %d золота.", 12));
    try std.testing.expectEqualStrings("Без числа", formatInt(&b, "Без числа", 3));
}
