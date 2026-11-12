//! CastleMain (Castle.cpp:2227): the castle screens of a player switched
//! by the tab buttons.  Ported: ScreenCastleBuild (castle_build.zig),
//! HomeGarrison (garrison.zig ArmyPanel), ScreenTreasury (Castle.cpp:1385),
//! HomeLibrary (Castle.cpp:2129), ScreenHeroRecruit (hero_recruit.zig; a
//! hired or resurrected hero closes the castle) and ScreenStatistics
//! (statistics.zig, over the current tab; a province picked there closes
//! the castle).  The right half shows
//! MakeCastlePic; after a building the old picture fades into the new one
//! (AnimOpa).
//!
//! A right click opens WindowItemInfo in the treasury, WindowSpellInfo in
//! the library and WindowResourceInfo on the resource bar; those windows
//! lead on to their own (info_windows.zig).
//!
//! Not ported: the tutorial, the alchemy window of the resource bar.
//! The castle plays Memory01 (mode 5) like the GOG binary (recomp's
//! FUN_004fe540); the map music resumes when it closes.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const Hint = @import("../ui/hint.zig").Hint;
const Mask = @import("../ui/mask.zig").Mask;
const castle_ui = @import("../ui/castle_ui.zig");
const Sheets = castle_ui.Sheets;
const gui = castle_ui.gui;
pub const Tab = castle_ui.Tab;
const ArmyPanel = @import("garrison.zig").ArmyPanel;
const mapgen = @import("../game/mapgen.zig");
const Shard = mapgen.Shard;
const player = @import("../game/player.zig");
const castle_pic = @import("../game/castle_pic.zig");
const castle = @import("../game/castle.zig");
const movement = @import("../game/movement.zig");
const castle_build = @import("castle_build.zig");
const Statistics = @import("statistics.zig").Statistics;
const Face = @import("statistics.zig").Face;
const ItemInfo = @import("item_info.zig").ItemInfo;
const SpellInfo = @import("spell_info.zig").SpellInfo;
const HeroTab = @import("hero_recruit.zig").HeroTab;

/// What the statistics tab needs of the map: CurGame.iCurrentTurn, the
/// rulers' names and arms, and where CastleProv goes.
pub const Extra = struct {
    turn: i32 = 0,
    faces: [mapgen.max_players + 1]Face = @splat(.{}),
    out: ?*i32 = null,
    /// CastleMain's -Result: the slot of the hero hired or resurrected.
    hero: ?*?u8 = null,
};

/// mouse.dat: the pointer, repair, no repair.
const cur_pointer = 0;
const cur_repair = 17;
const cur_no_repair = 18;

/// WindowSpellInfo over the current screen.
pub fn spellInfo(c: *Ctx, sp: i32) scene.Action {
    return .{ .push = SpellInfo.create(c, sp) catch |e| {
        std.log.err("spell window: {t}", .{e});
        return .none;
    } };
}

pub const CastleScreen = struct {
    s: *Shard,
    k: usize,
    tab: Tab,
    sh: Sheets,
    bar: castle_ui.TabBar,
    res: castle_ui.ResourceBar = .{ .x = 0, .y = 734 },
    hint: Hint = .{},
    parts: [512]castle_pic.Part = undefined,
    n_parts: usize = 0,
    /// bmpOldCastle and AnimOpa (200 down to 0 by 5 a 20 ms tick).
    old_parts: [512]castle_pic.Part = undefined,
    n_old: usize = 0,
    fade: f32 = 0,
    build: ?castle_build.BuildPanel = null,
    /// The result of the open WindowInnBldInfo.
    choice: castle_build.Choice = .none,
    /// Show of the loop: the building of the open window.
    shown: i32 = 0,
    walker: movement.Walker,
    garrison: ?ArmyPanel = null,
    treasury: ?Treasury = null,
    heroes: ?HeroTab = null,
    library: Library = .{},
    old_cursor: u32,
    pointer: input.Pointer = .{},
    extra: Extra,
    /// The province ScreenStatistics returned (CastleProv).
    picked: i32 = 0,

    pub fn create(c: *Ctx, s: *Shard, k: usize, tab: Tab, extra: Extra) !scene.Scene {
        const self = try c.gpa.create(CastleScreen);
        errdefer c.gpa.destroy(self);
        self.* = .{ .s = s, .k = k, .tab = tab, .sh = undefined, .bar = undefined, .walker = undefined, .old_cursor = c.cursor, .extra = extra };
        self.sh = try Sheets.load(c);
        errdefer self.sh.deinit();
        self.walker = try movement.Walker.init(c.gpa, s, c.proto, c.cfg.max_path);
        errdefer self.walker.deinit(c.gpa);
        self.bar = try castle_ui.TabBar.init(&self.sh.buttons, tab);
        errdefer self.bar.deinit();
        try self.enter(c, tab);
        c.setCursor(cur_pointer);
        // The castle plays Memory01 (FUN_004fe540 mode 5, recomp analysis).
        c.music.setMode(c.fs, .memory);
        return scene.Scene.of(CastleScreen, self, false);
    }

    pub fn deinit(self: *CastleScreen, c: *Ctx) void {
        self.leave();
        self.library.deinit();
        c.setCursor(self.old_cursor);
        self.bar.deinit();
        self.walker.deinit(c.gpa);
        self.sh.deinit();
        // Back to the map: the Main tracks resume.
        c.music.setMode(c.fs, .map);
    }

    fn market(self: *const CastleScreen, c: *const Ctx) player.Market {
        return .{ .s = self.s, .pr = c.proto, .player = @intCast(self.k) };
    }

    fn leave(self: *CastleScreen) void {
        if (self.garrison) |*g| {
            g.close();
            g.deinit();
            self.garrison = null;
        }
        if (self.treasury) |*t| {
            t.finish(&self.s.rulers[self.k].holdings);
            t.deinit();
            self.treasury = null;
        }
        if (self.heroes) |*t| {
            t.deinit();
            self.heroes = null;
        }
        if (self.build) |*b| {
            b.deinit();
            self.build = null;
        }
        self.fade = 0;
    }

    /// MakeCastlePic into `parts`, the current picture kept as the old one.
    fn recompose(self: *CastleScreen, c: *Ctx) !void {
        @memcpy(self.old_parts[0..self.n_parts], self.parts[0..self.n_parts]);
        self.n_old = self.n_parts;
        const built = try castle.buildings(self.s, self.k);
        self.n_parts = castle_pic.compose(c.proto, built, &self.s.rulers[self.k].holdings, &self.parts).len;
        self.fade = 200;
    }

    fn enter(self: *CastleScreen, c: *Ctx, tab: Tab) !void {
        self.tab = tab;
        const h = &self.s.rulers[self.k].holdings;
        const built = try castle.buildings(self.s, self.k);
        self.n_parts = castle_pic.compose(c.proto, built, h, &self.parts).len;
        // ScreenCastleBuild: the first visit after a building fades out
        // the parts shown only before it (bEnter).
        if (tab == .build and h.enter) {
            h.enter = false;
            try self.recompose(c);
        }
        switch (tab) {
            .build => {
                self.build = try castle_build.BuildPanel.init(&self.sh);
                try self.build.?.refresh(self.market(c));
            },
            .garrison => {
                const cap = self.s.capitals[self.k];
                const g = if (cap > 0) self.s.provinces[@intCast(cap)].garrison else -1;
                if (g >= 0) self.garrison = try ArmyPanel.init(c, &self.sh, self.s, &self.walker, self.k, @intCast(g), .{ .recruit = .{ 512, 392 }, .units = .{ 59, 339 }, .trash = .{ 340, 430 } }, false);
            },
            .treasury => self.treasury = try Treasury.init(&self.sh),
            .hero => self.heroes = try HeroTab.init(c, &self.sh),
            .library => {},
            else => {},
        }
    }

    fn switchTo(self: *CastleScreen, c: *Ctx, tab: Tab) void {
        self.leave();
        self.bar.deinit();
        self.bar = castle_ui.TabBar.init(&self.sh.buttons, tab) catch |e| {
            std.log.err("castle tabs: {t}", .{e});
            return;
        };
        self.enter(c, tab) catch |e| std.log.err("castle tab: {t}", .{e});
        c.setCursor(cur_pointer);
    }

    /// The answer of WindowInnBldInfo: build, plan or open another one.
    fn takeChoice(self: *CastleScreen, c: *Ctx, shown: i32) scene.Action {
        const choice = self.choice;
        self.choice = .none;
        const m = self.market(c);
        switch (choice) {
            .none => return .none,
            .build => {
                castle.build(self.s, c.proto, &c.rand, self.k, shown) catch |e| std.log.err("build {d}: {t}", .{ shown, e });
                self.recompose(c) catch |e| std.log.err("castle pic: {t}", .{e});
            },
            .plan => m.holdings().planned = shown,
            .open => |id| return self.openInfo(c, id),
        }
        if (self.build) |*b| b.refresh(m) catch {};
        return .none;
    }

    /// Result 6 of a tab: ScreenStatistics over the castle, then the same
    /// tab again unless a province was picked.
    pub fn openStatistics(self: *CastleScreen, c: *Ctx) scene.Action {
        self.picked = 0;
        return .{ .push = Statistics.create(c, self.s, self.k, self.extra.turn, self.extra.faces, &self.picked) catch |e| {
            std.log.err("statistics: {t}", .{e});
            return .none;
        } };
    }

    /// EADOR_BUILD_INFO: the window of building `id`.
    pub fn debugInfo(self: *CastleScreen, c: *Ctx, id: i32) scene.Action {
        return self.openInfo(c, id);
    }

    fn openInfo(self: *CastleScreen, c: *Ctx, id: i32) scene.Action {
        if (id <= 0 or id >= c.proto.inner_buildings.len) return .none;
        self.shown = id;
        self.choice = .none;
        return .{ .push = castle_build.BuildInfo.create(c, &self.sh, self.market(c), id, &self.choice) catch |e| {
            std.log.err("building window: {t}", .{e});
            self.shown = 0;
            return .none;
        } };
    }

    pub fn update(self: *CastleScreen, c: *Ctx, f: *const input.Frame, dt: f32) scene.Action {
        self.hint.clear();
        if (self.picked > 0) {
            if (self.extra.out) |o| o.* = self.picked;
            return .pop;
        }
        self.pointer = f.pointer;
        if (self.fade > 0) self.fade = @max(0, self.fade - 250 * dt);
        if (self.shown > 0) {
            const shown = self.shown;
            self.shown = 0;
            const a = self.takeChoice(c, shown);
            if (a != .none) return a;
        }
        const m = self.market(c);
        var action: scene.Action = .none;
        switch (self.tab) {
            .build => if (self.build) |*b| {
                if (b.update(m, f.pointer)) |id| action = self.openInfo(c, id);
            },
            .garrison => if (self.garrison) |*g| {
                action = g.update(c, f, &self.hint);
            },
            .treasury => if (self.treasury) |*t| {
                action = t.update(c, m, f);
            },
            .hero => if (self.heroes) |*t| switch (t.update(c, m, f.pointer, &self.hint)) {
                .none => {},
                .push => |sc| action = .{ .push = sc },
                .hero => |slot| {
                    if (self.extra.hero) |o| o.* = slot;
                    return .pop;
                },
            },
            .library => switch (self.library.update(c, &m.holdings().library, f.pointer, &self.sh, &self.hint)) {
                .info => |sp| action = spellInfo(c, sp),
                else => {},
            },
            else => {},
        }
        if (self.res.update(c, m, f.pointer, &self.hint)) |r| if (action == .none) {
            action = @import("garrison.zig").resourceInfo(c, m, r);
        };
        switch (self.bar.check(f)) {
            .none => {},
            .exit => return .pop,
            .tab => |t| if (t == .stats) {
                return self.openStatistics(c);
            } else self.switchTo(c, t),
        }
        self.bar.hint(c, &self.hint, f.pointer.pos);
        return action;
    }

    fn drawParts(self: *CastleScreen, parts: []const castle_pic.Part, k: f32) void {
        for (parts) |p| {
            const a: u8 = @intFromFloat(@as(f32, @floatFromInt(p.alpha)) * k);
            gfx.drawTex2(self.sh.castle.texIndex(p.index), 512 + @as(f32, @floatFromInt(p.x)), @floatFromInt(p.y), .{ .r = 255, .g = 255, .b = 255, .a = a });
        }
    }

    fn drawCastle(self: *CastleScreen) void {
        rl.beginScissorMode(512, 0, 512, 768);
        if (self.fade > 0) {
            self.drawParts(self.old_parts[0..self.n_old], 1);
            self.drawParts(self.parts[0..self.n_parts], (255 - self.fade) / 255);
        } else self.drawParts(self.parts[0..self.n_parts], 1);
        rl.endScissorMode();
    }

    pub fn draw(self: *CastleScreen, c: *Ctx) void {
        rl.clearBackground(.black);
        const m = self.market(c);
        switch (self.tab) {
            .garrison => {
                self.drawCastle();
                gfx.drawTex(self.sh.iface.texIndex(gui.home_barracks), 512, 0);
                gfx.drawTex(self.sh.iface.texIndex(gui.home_garrison), 0, 0);
                if (self.garrison) |*g| g.draw(c, &self.sh);
            },
            .treasury => if (self.treasury) |*t| t.draw(c, &self.sh, m),
            .library => {
                self.drawCastle();
                gfx.drawTex(self.sh.iface.texIndex(gui.home_library), 0, 0);
                self.library.draw(c, &self.sh, &m.holdings().library);
            },
            .build => {
                self.drawCastle();
                gfx.drawTex(self.sh.iface.texIndex(gui.build_screen), 0, 0);
                if (self.build) |*b| b.draw(c, &self.sh, m);
            },
            .hero => {
                self.drawCastle();
                if (self.heroes) |*t| t.draw(c, &self.sh, m);
            },
            .stats => {
                self.drawCastle();
                gfx.drawTex(self.sh.iface.texIndex(gui.build_screen), 0, 0);
            },
        }
        self.res.draw(c, &self.sh, m);
        self.bar.draw(&self.sh.buttons, c.tick);
        if (self.garrison) |*g| g.drawHeld(&self.sh, self.pointer);
        if (self.treasury) |*t| t.drawOver(c, &self.sh, m, self.pointer);
        if (self.build) |*b| b.drawOver(c, &self.sh, m);
        self.hint.draw(c, &self.sh.iface);
    }
};

/// ScreenTreasury: the castle shop (GUI_ShopBlock at 519, 13) and the
/// treasury (GUI_TreasuryBlock at 7, 467).
const Treasury = struct {
    shape: rl.Image,
    shop_masks: [player.shop_size]Mask = undefined,
    masks: [player.treasury_size]Mask = undefined,
    repair: castle_ui.Toggle,
    trash: castle_ui.Trash,
    /// SelectedItem, SelectedItemDur, SelectedSlot, ItemOwner (2 the
    /// treasury, 3 the shop).
    held: ?Held = null,
    /// PickFlag, PickItem, PickDur of the shop's info plaque.
    pick: Pick = .{},
    /// The item card under the pointer (MakeItemHint).
    card: ?Card = null,

    const Held = struct { item: i32, dur: i32, slot: usize, owner: enum { treasury, shop } };
    const Pick = castle_ui.Pick;
    const Card = struct { pos: rl.Vector2, item: i32, dur: i32, text: u32, price: player.Price };

    const shop_x = 519;
    const shop_y = 13;
    const box_x = 7;
    const box_y = 467;
    const cell = 55;
    const d = -6;

    fn slotXY(x0: f32, y0: f32, i: usize) [2]f32 {
        return .{ x0 + @as(f32, @floatFromInt(i % 9)) * cell, y0 + @as(f32, @floatFromInt(i / 9)) * cell };
    }

    fn init(sh: *Sheets) !Treasury {
        var t: Treasury = .{
            .shape = sh.iface.imageIndex(gui.spell_mask) orelse return error.MissingMask,
            .repair = undefined,
            .trash = undefined,
        };
        errdefer rl.unloadImage(t.shape);
        t.repair = try castle_ui.Toggle.init(sh, shop_x + 13, shop_y + 676, 56);
        errdefer t.repair.deinit();
        t.repair.hint = 144;
        t.repair.hint_down = 145;
        t.repair.hotkey = .r;
        t.trash = try castle_ui.Trash.init(sh, 512 + 94, 689);
        for (&t.shop_masks, 0..) |*m, i| {
            const at = slotXY(shop_x + d, shop_y + d, i);
            m.* = .{ .x = at[0], .y = at[1] };
        }
        for (&t.masks, 0..) |*m, i| {
            const at = slotXY(box_x + d, box_y + d, i);
            m.* = .{ .x = at[0], .y = at[1] };
        }
        return t;
    }

    fn deinit(self: *Treasury) void {
        self.trash.deinit();
        self.repair.deinit();
        rl.unloadImage(self.shape);
    }

    /// Leaving the screen: an item taken from the treasury goes back; the
    /// shop moves its items up.
    fn finish(self: *Treasury, h: *player.Holdings) void {
        if (self.held) |held| if (held.owner == .treasury) {
            h.treasury[held.slot] = .{ held.item, held.dur };
        };
        self.held = null;
        h.compactShop();
    }

    fn drop(self: *Treasury, c: *Ctx) void {
        self.held = null;
        self.pick = .{};
        self.trash.on = false;
        c.setCursor(cur_pointer);
    }

    fn checkMasks(self: *Treasury, masks: []Mask, p: input.Pointer) struct { under: ?usize, click: ?usize, info: ?usize } {
        var under: ?usize = null;
        var click: ?usize = null;
        var info: ?usize = null;
        for (masks, 0..) |*m, i| {
            m.shape = &self.shape;
            const r = m.check(p);
            if (m.hot()) under = i;
            if (r == .click) click = i;
            if (r == .right_click) info = i;
        }
        return .{ .under = under, .click = click, .info = info };
    }

    /// The right click of a slot: WindowItemInfo.
    fn itemInfo(c: *Ctx, it: [2]i32) scene.Action {
        if (it[0] <= 0) return .none;
        return .{ .push = ItemInfo.create(c, it[0], it[1]) catch |e| {
            std.log.err("item window: {t}", .{e});
            return .none;
        } };
    }

    fn update(self: *Treasury, c: *Ctx, m: player.Market, f: *const input.Frame) scene.Action {
        const pr = c.proto;
        const h = m.holdings();
        const p = f.pointer;
        self.card = null;
        _ = self.repair.check(f);
        // Repair mode puts an item taken from the treasury back.
        if (self.repair.pressed) if (self.held) |held| if (held.owner == .treasury) {
            h.treasury[held.slot] = .{ held.item, held.dur };
            self.drop(c);
        };

        const shop = self.checkMasks(&self.shop_masks, p);
        var shop_click: ?usize = shop.click;
        // mskSell: a click in the shop outside the slots.
        if (shop.under == null and p.primary.released and p.in(515, 9, 500, 666)) shop_click = player.shop_size;
        if (shop.under) |u| if (h.shop.items[u][0] > 0) {
            const it = h.shop.items[u][0];
            self.card = .{ .pos = p.pos, .item = it, .dur = pr.items[@intCast(it)].durability, .text = 13, .price = m.itemPrice(it, &h.shop) };
            if (self.held == null) self.pick = .{ .flag = .buy, .item = it, .dur = h.shop.items[u][1] };
        };
        if (shop_click) |i| self.clickShop(c, m, i);

        const box = self.checkMasks(&self.masks, p);
        if (box.under) |u| if (h.treasury[u][0] > 0) {
            const it = h.treasury[u];
            const price = if (self.repair.pressed) m.repairPrice(it[0], it[1], &h.shop) else player.sellPrice(pr, it[0], it[1]);
            self.card = .{ .pos = p.pos, .item = it[0], .dur = it[1], .text = if (self.repair.pressed) 14 else 12, .price = price };
            if (self.held == null) {
                if (self.repair.pressed) {
                    self.pick = .{ .flag = .repair, .item = it[0], .dur = it[1] };
                    c.setCursor(if ((price[0] > 0 or price[1] > 0) and m.affords(price)) cur_repair else cur_no_repair);
                } else self.pick = .{ .flag = .sell, .item = it[0], .dur = it[1] };
            }
        };
        if (box.click) |i| self.clickTreasury(c, m, i);
        if (self.held == null and shop.under == null and box.under == null) {
            self.pick = .{};
            c.setCursor(cur_pointer);
        }
        if (self.trash.check(f)) self.drop(c);
        if (shop.info) |i| return itemInfo(c, .{ h.shop.items[i][0], if (h.shop.items[i][0] > 0) pr.items[@intCast(h.shop.items[i][0])].durability else 0 });
        if (box.info) |i| return itemInfo(c, h.treasury[i]);
        return .none;
    }

    fn clickShop(self: *Treasury, c: *Ctx, m: player.Market, i: usize) void {
        const h = m.holdings();
        if (self.held) |held| {
            if (held.owner == .treasury) {
                // The sale of an item of the treasury.
                const got = player.sellPrice(c.proto, held.item, held.dur);
                m.ruler().gold += got[0];
                m.ruler().gem += got[1];
                sound.play(.unit, sound.buy);
                self.drop(c);
                return;
            }
            if (i < player.shop_size and h.shop.items[i][0] > 0 and h.shop.items[i][0] != held.item and m.affords(m.itemPrice(h.shop.items[i][0], &h.shop))) {
                self.takeFromShop(c, i, h.shop.items[i][0]);
            } else self.drop(c);
            return;
        }
        if (i >= player.shop_size) return;
        const it = h.shop.items[i][0];
        if (it > 0 and m.affords(m.itemPrice(it, &h.shop))) self.takeFromShop(c, i, it);
    }

    fn takeFromShop(self: *Treasury, c: *Ctx, slot: usize, item: i32) void {
        const dur = c.proto.items[@intCast(item)].durability;
        self.held = .{ .item = item, .dur = dur, .slot = slot, .owner = .shop };
        self.pick = .{ .flag = .buy, .item = item, .dur = dur };
        self.trash.on = true;
    }

    fn clickTreasury(self: *Treasury, c: *Ctx, m: player.Market, i: usize) void {
        const pr = c.proto;
        const h = m.holdings();
        if (self.held) |held| {
            if (h.treasury[i][0] > 0) {
                if (held.owner != .treasury) return;
                h.treasury[held.slot] = h.treasury[i];
                h.treasury[i] = .{ held.item, held.dur };
                self.drop(c);
                return;
            }
            if (held.owner == .shop) {
                // The purchase.
                sound.play(.unit, sound.buy);
                m.pay(m.itemPrice(held.item, &h.shop));
                m.raiseResPrices(&pr.items[@intCast(held.item)].res);
                const sl = &h.shop.items[held.slot];
                if (sl[1] > 0) {
                    sl[1] -= 1;
                    if (sl[1] == 0) sl[0] = 0;
                }
            }
            h.treasury[i] = .{ held.item, held.dur };
            self.drop(c);
            return;
        }
        const it = h.treasury[i];
        if (it[0] <= 0) return;
        if (self.repair.pressed) {
            const price = m.repairPrice(it[0], it[1], &h.shop);
            self.pick = .{ .flag = .repair, .item = it[0], .dur = it[1] };
            if ((price[0] > 0 or price[1] > 0) and m.affords(price)) {
                m.pay(price);
                sound.play(.unit, sound.repair);
                h.treasury[i][1] = pr.items[@intCast(it[0])].durability;
                self.pick.dur = h.treasury[i][1];
                c.setCursor(cur_no_repair);
            }
            return;
        }
        self.held = .{ .item = it[0], .dur = it[1], .slot = i, .owner = .treasury };
        h.treasury[i] = .{ 0, 0 };
        self.pick = .{ .flag = .sell, .item = it[0], .dur = it[1] };
        self.trash.on = true;
    }

    fn draw(self: *const Treasury, c: *Ctx, sh: *Sheets, m: player.Market) void {
        const pr = c.proto;
        const h = m.holdings();
        gfx.drawTex(sh.iface.texIndex(gui.treasury), 0, 0);
        gfx.drawTex(sh.iface.texIndex(gui.shop), 512, 0);
        var b: [16]u8 = undefined;
        for (h.shop.items, 0..) |it, i| {
            if (it[0] <= 0) continue;
            const at = slotXY(shop_x, shop_y, i);
            gfx.drawTex(sh.item(pr, it[0]), at[0], at[1]);
            if (it[1] > 0) {
                const n = castle_ui.fmtInt(&b, it[1]);
                c.font1.draw(n, at[0] + 38, at[1] + 38, castle_ui.color1);
                c.font1.draw(n, at[0] + 37, at[1] + 37, castle_ui.color2);
            }
            if (!m.affords(m.itemPrice(it[0], &h.shop))) gfx.drawTex(sh.iface.texIndex(gui.spell_shadow), at[0], at[1]);
        }
        for (h.treasury, 0..) |it, i| {
            if (it[0] <= 0) continue;
            const at = slotXY(box_x, box_y, i);
            gfx.drawTex(sh.item(pr, it[0]), at[0], at[1]);
            if (it[1] == 0) gfx.drawTex(sh.iface.texIndex(gui.spell_shadow), at[0], at[1]);
        }
        for (self.shop_masks) |mk| if (mk.hot()) gfx.drawTex(sh.iface.texIndex(gui.spell_select), mk.x, mk.y);
        for (self.masks) |mk| if (mk.hot()) gfx.drawTex(sh.iface.texIndex(gui.spell_select), mk.x, mk.y);
        castle_ui.shopPlaque(c, sh, m, &h.shop, self.pick, null, shop_x, shop_y);
        self.repair.draw(sh);
        self.trash.draw(sh);
    }

    /// The held item at the pointer and the item card.
    fn drawOver(self: *const Treasury, c: *Ctx, sh: *Sheets, m: player.Market, p: input.Pointer) void {
        if (self.held) |held| castle_ui.drawHeld(sh, sh.item(c.proto, held.item), p);
        if (self.card) |cd| {
            const r = m.ruler();
            castle_ui.itemCard(c, sh, cd.pos, cd.item, cd.dur, null, cd.text, cd.price, .{ r.gold, r.gem });
        }
    }
};

/// GUI_LibraryBlock (Screens.cpp:1565): the spells of the library by
/// circle, the name of the one under the pointer.  HomeLibrary puts it at
/// (80, 101), ScreenHeroLibrary at (592, 101).
pub const Library = struct {
    x0: f32 = 80,
    y0: f32 = 101,
    masks: [player.library_size]Mask = @splat(.{ .x = 0, .y = 0 }),
    shape: ?rl.Image = null,

    const dx = 90;
    const dy = 30;
    const w = 60;
    /// L0..L5: the rows from the first circle up.
    const rows = [_]struct { first: usize, n: usize, x: f32, y: f32 }{
        .{ .first = 0, .n = 3, .x = dx, .y = 493 },
        .{ .first = 3, .n = 6, .x = 0, .y = 423 },
        .{ .first = 9, .n = 6, .x = 0, .y = 363 },
        .{ .first = 15, .n = 6, .x = 0, .y = 262 },
        .{ .first = 21, .n = 3, .x = dx, .y = 202 },
        .{ .first = 24, .n = 6, .x = 0, .y = 101 },
        .{ .first = 30, .n = 3, .x = dx, .y = 0 },
    };

    /// The spell picture of slot i.
    fn pos(self: *const Library, i: usize) [2]f32 {
        for (rows) |r| if (i >= r.first and i < r.first + r.n) {
            return .{ self.x0 + r.x + @as(f32, @floatFromInt(i - r.first)) * w, self.y0 + r.y + dy };
        };
        return .{ 0, 0 };
    }

    /// GUI_LibraryBlock::Check: the hint of the spell under the pointer;
    /// a left click returns `.take`, a right click `.info` (WindowSpellInfo).
    pub const Pick = union(enum) { none, take: i32, info: i32 };

    pub fn update(self: *Library, c: *Ctx, lib: *const [player.library_size]i32, p: input.Pointer, sh: *Sheets, hint: *Hint) Pick {
        if (self.shape == null) self.shape = sh.iface.imageIndex(gui.spell_mask);
        var r: Pick = .none;
        for (&self.masks, lib, 0..) |*mk, sp, i| {
            if (sp <= 0 or sp >= c.proto.spells.len) {
                mk.state = .visible;
                continue;
            }
            const at = self.pos(i);
            mk.x = at[0] - 6;
            mk.y = at[1] - 6;
            if (self.shape) |*s| mk.shape = s;
            switch (mk.check(p)) {
                .click => r = .{ .take = sp },
                .right_click => r = .{ .info = sp },
                .none => {},
            }
            if (mk.hot()) {
                hint.what = .{ .string = c.proto.spells[@intCast(sp)].name };
                hint.pos = p.pos;
            }
        }
        return r;
    }

    pub fn draw(self: *const Library, c: *Ctx, sh: *Sheets, lib: *const [player.library_size]i32) void {
        for (lib, 0..) |sp, i| {
            if (sp <= 0 or sp >= c.proto.spells.len) continue;
            const at = self.pos(i);
            gfx.drawTex(sh.spell(sp), at[0], at[1]);
            if (self.masks[i].hot()) gfx.drawTex(sh.iface.texIndex(gui.spell_select), at[0] - 6, at[1] - 6);
        }
        const titles = [_][2]f32{ .{ 314, 370 }, .{ 315, 210 }, .{ 316, 109 }, .{ 317, 8 } };
        for (titles) |t| c.font1.drawCentered(c.proto.phrase(@intFromFloat(t[0])), self.x0 + 177, self.y0 + t[1], castle_ui.color1);
    }

    pub fn deinit(self: *Library) void {
        if (self.shape) |s| rl.unloadImage(s);
        self.shape = null;
    }
};
