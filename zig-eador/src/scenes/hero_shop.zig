//! ScreenHeroShop (Screens.cpp:4745): the hero, his items, the treasury
//! (GUI_TreasuryBlock at 7, 500) and a shop (GUI_ShopBlock at 519, 13).
//! Items are bought, sold, repaired one by one or all at once
//! (buttRepairAll), moved between the hero and the treasury and thrown
//! away.  Haggling (skill 27) lowers the shop's price for the visit.  A
//! right click on an item opens WindowItemInfo.
//!
//! Not ported: the tutorial, YuPlay, the alchemy window of the resource
//! bar.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const Hint = @import("../ui/hint.zig").Hint;
const Mask = @import("../ui/mask.zig").Mask;
const Button = @import("../ui/button.zig").Button;
const castle_ui = @import("../ui/castle_ui.zig");
const Sheets = castle_ui.Sheets;
const gui = castle_ui.gui;
const info_windows = @import("info_windows.zig");
const hero_ui = @import("../ui/hero_ui.zig");
const Dialog = @import("dialog.zig").Dialog;
const ItemInfo = @import("item_info.zig").ItemInfo;
const mapgen = @import("../game/mapgen.zig");
const Shard = mapgen.Shard;
const player = @import("../game/player.zig");
const site = @import("../game/site.zig");
const hero_mod = @import("../game/hero.zig");
const Hero = hero_mod.Hero;
const equip = @import("../game/equip.zig");
const movement = @import("../game/movement.zig");

/// interface.dat 34ItemTrade.
const gui_item_trade = 34;
/// mouse.dat: the pointer, repair, no repair.
const cur_pointer = 0;
const cur_repair = 17;
const cur_no_repair = 18;
/// buttons.dat: exit, repair all.
const butt_exit = 226;
const butt_repair = 56;
const butt_repair_all = 60;
/// The Haggling skill (apSkill index).
const skill_haggle = 27;
/// dialog.var 1462: "Выбросить предмет?"; arMess[4] asks it.
const dlg_delete_item = 1462;
const msg_delete = 4;

const shop_x = 519;
const shop_y = 13;
const box_x = 7;
const box_y = 500;
const cell = 55;

fn slotXY(x0: f32, y0: f32, i: usize) [2]f32 {
    return .{ x0 + @as(f32, @floatFromInt(i % 9)) * cell, y0 + @as(f32, @floatFromInt(i / 9)) * cell };
}

/// A grid of item slots (the shop or the treasury).
fn Grid(comptime n: usize) type {
    return struct {
        masks: [n]Mask,

        const G = @This();
        const Check = struct { under: ?usize = null, click: ?usize = null, info: ?usize = null };

        fn init(x0: f32, y0: f32) G {
            var g: G = .{ .masks = undefined };
            for (&g.masks, 0..) |*m, i| {
                const at = slotXY(x0 - 6, y0 - 6, i);
                m.* = .{ .x = at[0], .y = at[1] };
            }
            return g;
        }

        fn check(self: *G, shape: *const rl.Image, p: input.Pointer) Check {
            var r: Check = .{};
            for (&self.masks, 0..) |*m, i| {
                m.shape = shape;
                const res = m.check(p);
                if (m.hot()) r.under = i;
                switch (res) {
                    .click => r.click = i,
                    .right_click => r.info = i,
                    .none => {},
                }
            }
            return r;
        }

        fn drawSelect(self: *const G, sh: *Sheets) void {
            for (self.masks) |m| if (m.hot()) gfx.drawTex(sh.iface.texIndex(gui.spell_select), m.x, m.y);
        }
    };
}

pub const HeroShop = struct {
    s: *Shard,
    k: usize,
    shop: *site.Shop,
    old_price: i32,
    sh: Sheets,
    hs: hero_ui.HeroSheets,
    shape: rl.Image,
    shop_grid: Grid(player.shop_size),
    box: Grid(player.treasury_size),
    items: hero_ui.ItemBlock,
    info: hero_ui.InfoBlock,
    stat: hero_ui.StatBlock = .{ .x = 8, .y = 278 },
    res: castle_ui.ResourceBar = .{ .x = 0, .y = 734 },
    repair: castle_ui.Toggle,
    repair_all: Button,
    trash: castle_ui.Trash,
    exit: Button,
    held: ?equip.Held = null,
    pick: castle_ui.Pick = .{},
    card: ?Card = null,
    hint: Hint = .{},
    pointer: input.Pointer = .{},
    /// The answer of the "throw it away?" dialog.
    answer: ?usize = null,
    asking: bool = false,
    old_cursor: u32,

    const Card = struct { item: i32, dur: i32, text: u32, price: player.Price };

    /// `k` indexes Shard.heroes; `shop` is the castle's or a site's.
    pub fn create(c: *Ctx, s: *Shard, k: usize, shop: *site.Shop, exp_pct: i32) !scene.Scene {
        const self = try c.gpa.create(HeroShop);
        errdefer c.gpa.destroy(self);
        const h = &s.heroes.items[k];
        const pl: usize = @intCast(h.player);
        self.* = .{
            .s = s,
            .k = k,
            .shop = shop,
            .old_price = shop.prices[pl],
            .sh = undefined,
            .hs = undefined,
            .shape = undefined,
            .shop_grid = Grid(player.shop_size).init(shop_x, shop_y),
            .box = Grid(player.treasury_size).init(box_x, box_y),
            .items = undefined,
            .info = hero_ui.InfoBlock.init(c, h, 0, 0, movement.castleParam(s, c.proto, h.player, movement.abil_siege, true), movement.castleParam(s, c.proto, h.player, movement.abil_mobility, true), exp_pct),
            .repair = undefined,
            .repair_all = undefined,
            .trash = undefined,
            .exit = undefined,
            .old_cursor = c.cursor,
        };
        self.sh = try Sheets.load(c);
        errdefer self.sh.deinit();
        self.hs = try hero_ui.HeroSheets.load(c);
        errdefer self.hs.deinit();
        self.shape = self.sh.iface.imageIndex(gui.spell_mask) orelse return error.MissingMask;
        errdefer rl.unloadImage(self.shape);
        self.items = try hero_ui.ItemBlock.init(&self.sh, 207, 14);
        errdefer self.items.deinit();
        self.repair = try castle_ui.Toggle.init(&self.sh, shop_x + 13, shop_y + 676, butt_repair);
        errdefer self.repair.deinit();
        self.repair.hint = 144;
        self.repair.hint_down = 145;
        self.repair.hotkey = .r;
        self.repair_all = try Button.init(&self.sh.buttons, shop_x + 78, shop_y + 676, butt_repair_all, butt_repair_all + 1, butt_repair_all + 2);
        errdefer self.repair_all.deinit();
        self.repair_all.inactive = butt_repair_all + 3;
        self.repair_all.hint = 143;
        self.repair_all.hotkey = .t;
        self.repair_all.enabled = h.class1 > 0;
        self.trash = try castle_ui.Trash.init(&self.sh, 32, 420);
        errdefer self.trash.deinit();
        self.trash.sound = sound.dismiss;
        self.exit = try Button.init(&self.sh.buttons, 934, 687, butt_exit, butt_exit + 1, butt_exit + 2);
        self.exit.hotkey = .escape;
        self.exit.hint = 135;
        const haggle = h.checkSkill(skill_haggle);
        if (shop.prices[pl] > 100 and haggle > 0) shop.prices[pl] = @divTrunc(shop.prices[pl] * (10 - haggle), 10);
        c.setCursor(cur_pointer);
        return scene.Scene.of(HeroShop, self, false);
    }

    pub fn deinit(self: *HeroShop, c: *Ctx) void {
        self.putBack();
        self.shop.prices[@intCast(self.hero().player)] = self.old_price;
        player.compact(self.shop);
        c.setCursor(self.old_cursor);
        self.exit.deinit();
        self.trash.deinit();
        self.repair_all.deinit();
        self.repair.deinit();
        self.items.deinit();
        rl.unloadImage(self.shape);
        self.hs.deinit();
        self.sh.deinit();
    }

    fn hero(self: *const HeroShop) *Hero {
        return &self.s.heroes.items[self.k];
    }

    fn market(self: *const HeroShop, c: *const Ctx) player.Market {
        return .{ .s = self.s, .pr = c.proto, .player = self.hero().player };
    }

    fn treasury(self: *const HeroShop) *equip.Treasury {
        return &self.s.rulers[@intCast(self.hero().player)].holdings.treasury;
    }

    /// An item of the hero or the treasury goes back where it was taken.
    fn putBack(self: *HeroShop) void {
        const held = self.held orelse return;
        switch (held.owner) {
            .hero => self.hero().inventory[held.slot] = .{ held.item, held.dur },
            .treasury => self.treasury()[held.slot] = .{ held.item, held.dur },
            .shop => {},
        }
        self.held = null;
    }

    fn drop(self: *HeroShop, c: *Ctx) void {
        self.held = null;
        self.pick = .{};
        self.trash.on = false;
        c.setCursor(cur_pointer);
    }

    fn take(self: *HeroShop, held: equip.Held) void {
        self.held = held;
        self.pick = .{ .flag = if (held.owner == .shop) .buy else .sell, .item = held.item, .dur = held.dur };
        self.trash.on = true;
    }

    fn itemInfo(c: *Ctx, item: i32, dur: i32) scene.Action {
        if (item <= 0) return .none;
        return .{ .push = ItemInfo.create(c, item, dur) catch |e| {
            std.log.err("item window: {t}", .{e});
            return .none;
        } };
    }

    /// After a drop: the hand, the sound of a purchase.
    fn dropped(self: *HeroShop, c: *Ctx, r: equip.Result) void {
        if (r.bought) sound.play(.unit, sound.buy);
        if (r.held) |h| {
            self.held = h;
            self.pick = .{};
            if (h.owner == .shop) self.pick = .{ .flag = .buy, .item = h.item, .dur = h.dur };
        } else self.drop(c);
    }

    fn repairOne(self: *HeroShop, c: *Ctx, it: *[2]i32) void {
        const m = self.market(c);
        const price = m.repairPrice(it[0], it[1], self.shop);
        self.pick = .{ .flag = .repair, .item = it[0], .dur = it[1] };
        if ((price[0] > 0 or price[1] > 0) and m.affords(price)) {
            m.pay(price);
            sound.play(.unit, sound.repair);
            it[1] = c.proto.items[@intCast(it[0])].durability;
            self.pick.dur = it[1];
            c.setCursor(cur_no_repair);
        }
    }

    fn hoverOwned(self: *HeroShop, c: *Ctx, it: [2]i32) void {
        const m = self.market(c);
        const price = if (self.repair.pressed) m.repairPrice(it[0], it[1], self.shop) else player.sellPrice(c.proto, it[0], it[1]);
        self.card = .{ .item = it[0], .dur = it[1], .text = if (self.repair.pressed) 14 else 12, .price = price };
        if (self.held != null) return;
        if (self.repair.pressed) {
            self.pick = .{ .flag = .repair, .item = it[0], .dur = it[1] };
            c.setCursor(if ((price[0] > 0 or price[1] > 0) and m.affords(price)) cur_repair else cur_no_repair);
        } else self.pick = .{ .flag = .sell, .item = it[0], .dur = it[1] };
    }

    pub fn update(self: *HeroShop, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        const pr = c.proto;
        const m = self.market(c);
        const h = self.hero();
        const tr = self.treasury();
        const p = f.pointer;
        self.pointer = p;
        self.hint.clear();
        self.card = null;
        if (self.asking) {
            self.asking = false;
            if (self.answer == 1) self.drop(c);
            self.answer = null;
        }
        if (self.exit.check(f)) return .pop;

        // GUI_ShopBlock::Check.
        _ = self.repair.check(f);
        const all = equip.repairAllPrice(m, h, self.shop);
        self.repair_all.on = all[0] > 0 and all[0] <= m.ruler().gold and (all[1] <= m.ruler().gem or all[1] == 0);
        if (self.repair_all.check(f)) {
            sound.play(.unit, sound.repair_all);
            equip.repairAll(m, pr, h, self.shop);
        }
        if (self.repair.pressed) if (self.held) |held| if (held.owner != .shop) {
            self.putBack();
            self.drop(c);
        };
        const shop = self.shop_grid.check(&self.shape, p);
        var shop_click = shop.click;
        if (shop.under == null and p.primary.released and p.in(515, 9, 500, 666)) shop_click = player.shop_size;
        if (shop.under) |u| if (self.shop.items[u][0] > 0) {
            const it = self.shop.items[u][0];
            self.card = .{ .item = it, .dur = pr.items[@intCast(it)].durability, .text = 13, .price = m.itemPrice(it, self.shop) };
            if (self.held == null) self.pick = .{ .flag = .buy, .item = it, .dur = self.shop.items[u][1] };
        };
        if (shop_click) |i| self.clickShop(c, i);

        // GUI_TreasuryBlock::Check.
        const box = self.box.check(&self.shape, p);
        if (box.under) |u| if (tr[u][0] > 0) self.hoverOwned(c, tr[u]);
        if (box.click) |i| self.clickTreasury(c, i);

        // GUI_ItemBlock::Check.
        const inv = self.items.check(p);
        if (inv.under) |u| if (u > 0 and h.inventory[u][0] > 0) self.hoverOwned(c, h.inventory[u]);
        if (inv.click) |k| self.clickHero(c, k);

        if (self.held == null and shop.under == null and box.under == null and inv.under == null) {
            self.pick = .{};
            c.setCursor(cur_pointer);
        }
        if (self.trash.check(f)) if (self.held) |held| {
            if (held.owner != .shop and c.human.message(msg_delete) and dlg_delete_item < pr.dialogs.len) {
                self.asking = true;
                return .{ .push = Dialog.create(c, dlg_delete_item, .{ true, true, false, false }, &self.answer) catch {
                    self.asking = false;
                    self.drop(c);
                    return .none;
                } };
            }
            self.drop(c);
        };
        self.info.refresh(c, h);
        if (self.info.portraitClick(&self.hs, h, p)) return .{ .push = @import("army_info.zig").UnitWindow.createHero(c, h, self.info.exp_pct) catch return .none };
        const abil_click = self.info.update(c, h, p, &self.hint) orelse self.stat.update(c, h, p, &self.hint);
        if (self.res.update(c, m, p, &self.hint)) |r| return @import("garrison.zig").resourceInfo(c, m, r);
        if (self.repair.mask.hot()) self.hint.phrase(c, self.repair.hintPhrase(), p.pos);
        if (self.repair_all.hot()) self.hint.phrase(c, self.repair_all.hint, p.pos);
        if (self.exit.hot()) self.hint.phrase(c, self.exit.hint, p.pos);

        if (shop.info) |i| if (self.shop.items[i][0] > 0) return itemInfo(c, self.shop.items[i][0], pr.items[@intCast(self.shop.items[i][0])].durability);
        if (box.info) |i| return itemInfo(c, tr[i][0], tr[i][1]);
        if (inv.info) |k| if (k > 0) return itemInfo(c, h.inventory[k][0], h.inventory[k][1]);
        if (abil_click) |a| return info_windows.abilityAction(c, a.num, a.power);
        return .none;
    }

    fn clickShop(self: *HeroShop, c: *Ctx, i: usize) void {
        const m = self.market(c);
        if (self.held) |held| {
            if (held.owner != .shop) {
                // The sale.
                const got = player.sellPrice(c.proto, held.item, held.dur);
                m.ruler().gold += got[0];
                m.ruler().gem += got[1];
                sound.play(.unit, sound.buy);
                self.drop(c);
                return;
            }
            if (i < player.shop_size and self.shop.items[i][0] > 0 and self.shop.items[i][0] != held.item and m.affords(m.itemPrice(self.shop.items[i][0], self.shop))) {
                self.takeShop(c, i);
            } else self.drop(c);
            return;
        }
        if (i < player.shop_size and self.shop.items[i][0] > 0 and m.affords(m.itemPrice(self.shop.items[i][0], self.shop))) self.takeShop(c, i);
    }

    fn takeShop(self: *HeroShop, c: *Ctx, i: usize) void {
        const it = self.shop.items[i][0];
        self.take(.{ .item = it, .dur = c.proto.items[@intCast(it)].durability, .slot = i, .owner = .shop });
    }

    fn buy(self: *HeroShop, c: *Ctx, f_ctrl: bool) equip.Buy {
        return .{ .m = self.market(c), .shop = self.shop, .ctrl = f_ctrl };
    }

    fn clickTreasury(self: *HeroShop, c: *Ctx, i: usize) void {
        const tr = self.treasury();
        if (self.held) |held| {
            const ctrl = rl.isKeyDown(.left_control) or rl.isKeyDown(.right_control);
            if (equip.dropOnTreasury(c.proto, self.hero(), tr, held, i, self.buy(c, ctrl))) |r| self.dropped(c, r);
            return;
        }
        if (tr[i][0] <= 0) return;
        if (self.repair.pressed) return self.repairOne(c, &tr[i]);
        self.take(.{ .item = tr[i][0], .dur = tr[i][1], .slot = i, .owner = .treasury });
        tr[i] = .{ 0, 0 };
    }

    fn clickHero(self: *HeroShop, c: *Ctx, k_in: usize) void {
        const h = self.hero();
        var k = k_in;
        if (self.held) |held| {
            if (k == 0) k = equip.dollTarget(c.proto, h, held.item);
            const ctrl = rl.isKeyDown(.left_control) or rl.isKeyDown(.right_control);
            if (equip.dropOnHero(c.proto, h, self.treasury(), held, k, self.buy(c, ctrl))) |r| self.dropped(c, r);
            return;
        }
        if (k == 0 or h.inventory[k][0] <= 0) return;
        if (self.repair.pressed) return self.repairOne(c, &h.inventory[k]);
        self.take(.{ .item = h.inventory[k][0], .dur = h.inventory[k][1], .slot = k, .owner = .hero });
        h.inventory[k] = .{ 0, 0 };
    }

    pub fn draw(self: *HeroShop, c: *Ctx) void {
        const pr = c.proto;
        const m = self.market(c);
        const h = self.hero();
        rl.clearBackground(.black);
        gfx.drawTex(self.sh.iface.texIndex(gui_item_trade), 0, 0);
        gfx.drawTex(self.sh.iface.texIndex(gui.shop), 512, 0);
        var b: [16]u8 = undefined;
        for (self.shop.items, 0..) |it, i| {
            if (it[0] <= 0) continue;
            const at = slotXY(shop_x, shop_y, i);
            gfx.drawTex(self.sh.item(pr, it[0]), at[0], at[1]);
            if (it[1] > 0) {
                const n = castle_ui.fmtInt(&b, it[1]);
                c.font1.draw(n, at[0] + 38, at[1] + 38, castle_ui.color1);
                c.font1.draw(n, at[0] + 37, at[1] + 37, castle_ui.color2);
            }
            if (!m.affords(m.itemPrice(it[0], self.shop))) gfx.drawTex(self.sh.iface.texIndex(gui.spell_shadow), at[0], at[1]);
        }
        for (self.treasury(), 0..) |it, i| {
            if (it[0] <= 0) continue;
            const at = slotXY(box_x, box_y, i);
            gfx.drawTex(self.sh.item(pr, it[0]), at[0], at[1]);
            if (it[1] == 0) gfx.drawTex(self.sh.iface.texIndex(gui.spell_shadow), at[0], at[1]);
        }
        self.shop_grid.drawSelect(&self.sh);
        self.box.drawSelect(&self.sh);
        castle_ui.shopPlaque(c, &self.sh, m, self.shop, self.pick, if (h.class1 > 0) equip.repairAllPrice(m, h, self.shop) else null, shop_x, shop_y);
        self.items.draw(c, &self.sh, h, if (self.held) |held| held.item else 0);
        self.stat.draw(c, h);
        self.info.draw(c, &self.sh, &self.hs, h);
        self.res.draw(c, &self.sh, m);
        self.repair.draw(&self.sh);
        self.repair_all.draw(&self.sh.buttons, c.tick);
        self.trash.draw(&self.sh);
        self.exit.draw(&self.sh.buttons, c.tick);
        if (self.held) |held| castle_ui.drawHeld(&self.sh, self.sh.item(pr, held.item), self.pointer);
        if (self.card) |cd| {
            const r = m.ruler();
            castle_ui.itemCard(c, &self.sh, self.pointer.pos, cd.item, cd.dur, h, cd.text, cd.price, .{ r.gold, r.gem });
        } else self.hint.draw(c, &self.sh.iface);
    }
};
