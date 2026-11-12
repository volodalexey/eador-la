//! WindowItemInfo (Windower.cpp:2401): the item window over the darkened
//! screen: the picture, name, type, slot, durability, price and resources,
//! one row per set the item is in and per effect (and the spell, guard,
//! building or ritual of a scroll, contract or blueprint).
//!
//! A click on a row opens its window: WindowAbilityInfo for an effect,
//! WindowSpellInfo, WindowDefenderInfo, WindowOuterBldInfo or
//! WindowRitualInfo for what a scroll, contract or blueprint gives (a set
//! opens nothing, as in the original); a click on a resource opens
//! WindowResourceInfo.  The rows of a contract, blueprint and ritual show
//! the item's own picture, which is the defender's, building's or
//! ritual's (Prototype.cpp:3375).
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Button = @import("../ui/button.zig").Button;
const Hint = @import("../ui/hint.zig").Hint;
const castle_ui = @import("../ui/castle_ui.zig");
const proto_mod = @import("../data/proto.zig");
const Mask = @import("../ui/mask.zig").Mask;
const SpellInfo = @import("spell_info.zig").SpellInfo;
const info_windows = @import("info_windows.zig");
const CostWindow = info_windows.CostWindow;
const ResourceWindow = @import("army_info.zig").ResourceWindow;
const player = @import("../game/player.zig");

/// interface.dat 35ItemInfo, 72ItemEffect, 78ItemSet.
const gui_item_info = 35;
const gui_item_effect = 72;
const gui_item_set = 78;
const butt_exit = 226;
/// Phrases: "Прочность", "(отряд)", the gold and gem hints.
const ph_durability = 16;
const ph_squad = 17;
const ph_gold = 307;
const ph_gem = 308;

const color1 = castle_ui.color1;
const color2 = castle_ui.color2;

/// arAbil: an effect of the item, a set, or the thing a scroll gives.
const Row = union(enum) { effect: usize, set: usize, spell, defender, building, ritual };

pub const ItemInfo = struct {
    item: i32,
    dur: i32,
    /// CurrentPlayer for WindowResourceInfo; null shows the base prices.
    market: ?player.Market = null,
    res_masks: [proto_mod.max_item_res]Mask = undefined,
    sh: castle_ui.Sheets,
    abilities: gfx.Sheet,
    exit: Button,
    rows: [proto_mod.max_item_effect * 2 + 16]Row = undefined,
    n_rows: usize = 0,
    /// arMask of the rows (x+11, y+122+i*h, bmpGUI_SpellMask).
    row_masks: [proto_mod.max_item_effect * 2 + 16]Mask = undefined,
    shape: rl.Image,
    x: f32 = 0,
    y: f32 = 0,
    h: f32 = 0,
    hint: Hint = .{},
    old_cursor: u32,

    pub fn create(c: *Ctx, item: i32, dur: i32) !scene.Scene {
        return createFor(c, item, dur, null);
    }

    /// `m`: the player whose resources and prices WindowResourceInfo shows.
    pub fn createFor(c: *Ctx, item: i32, dur: i32, m: ?player.Market) !scene.Scene {
        if (item <= 0 or item >= c.proto.items.len) return error.NoItem;
        const self = try c.gpa.create(ItemInfo);
        errdefer c.gpa.destroy(self);
        self.* = .{ .item = item, .dur = dur, .market = m, .sh = undefined, .abilities = undefined, .exit = undefined, .shape = undefined, .old_cursor = c.cursor };
        self.sh = try castle_ui.Sheets.load(c);
        errdefer self.sh.deinit();
        self.shape = self.sh.iface.imageIndex(castle_ui.gui.spell_mask) orelse return error.MissingMask;
        errdefer rl.unloadImage(self.shape);
        self.abilities = try gfx.Sheet.load(c.fs, "ability");
        errdefer self.abilities.deinit();
        self.collect(c);
        const win = self.sh.iface.texIndex(gui_item_info) orelse return error.MissingItemInfo;
        const eff_h = gfx.texH(self.sh.iface.texIndex(gui_item_effect));
        self.h = 190 + eff_h * @as(f32, @floatFromInt(self.n_rows));
        self.x = @floor(512 - @as(f32, @floatFromInt(win.width)) / 2);
        self.y = @floor(384 - self.h / 2);
        for (self.row_masks[0..self.n_rows], 0..) |*mk, i| mk.* = .{ .x = self.x + 11, .y = self.y + 122 + eff_h * @as(f32, @floatFromInt(i)) };
        for (&self.res_masks, 0..) |*mk, i| {
            const fi: f32 = @floatFromInt(i);
            mk.* = Mask.rect(self.x + 352 + 26 * @mod(fi, 3), self.y + 69 + 25 * @floor(fi / 3), 20, 20);
        }
        self.exit = try Button.init(&self.sh.buttons, self.x + 193, self.y + self.h - 58, butt_exit, butt_exit + 1, butt_exit + 2);
        self.exit.hotkey = .escape;
        c.setCursor(0);
        return scene.Scene.of(ItemInfo, self, true);
    }

    pub fn deinit(self: *ItemInfo, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.exit.deinit();
        rl.unloadImage(self.shape);
        self.abilities.deinit();
        self.sh.deinit();
    }

    fn add(self: *ItemInfo, r: Row) void {
        if (self.n_rows == self.rows.len) return;
        self.rows[self.n_rows] = r;
        self.n_rows += 1;
    }

    /// The sets first, then the effects (Count, arAbil).
    fn collect(self: *ItemInfo, c: *Ctx) void {
        const pr = c.proto;
        for (pr.item_sets[1..], 1..) |*s, i| {
            for (s.items[0..s.n_items]) |x| if (x == self.item) self.add(.{ .set = i });
        }
        const it = &pr.items[@intCast(self.item)];
        for (it.effects[0..it.n_effects], 0..) |e, i| {
            if (e.id > 0) self.add(.{ .effect = i });
            switch (e.id) {
                83 => self.add(.spell),
                86 => self.add(.defender),
                87 => self.add(.building),
                94 => self.add(.ritual),
                else => {},
            }
        }
    }

    pub fn update(self: *ItemInfo, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        if (self.exit.check(f)) return .pop;
        const p = f.pointer;
        const it = &c.proto.items[@intCast(self.item)];
        const p1 = it.effects[0].p1;
        for (self.row_masks[0..self.n_rows], self.rows[0..self.n_rows]) |*m, row| {
            m.shape = &self.shape;
            if (m.check(p) == .none) continue;
            const next: anyerror!scene.Scene = switch (row) {
                .effect => |i| info_windows.ability(c, it.effects[i].id, it.effects[i].p1),
                .spell => SpellInfo.create(c, p1),
                .defender => CostWindow.defender(c, p1, 0, 0, self.market),
                .building => CostWindow.outer(c, p1, self.market),
                .ritual => CostWindow.ritual(c, p1, self.market),
                // "Set Info": nothing in the original.
                .set => continue,
            };
            return .{ .push = next catch return .none };
        }
        for (it.res, &self.res_masks) |r, *m| {
            if (r <= 0 or r >= c.proto.resources.len) continue;
            const res = m.check(p);
            if (m.hot()) {
                self.hint.what = .{ .string = c.proto.resources[@intCast(r)].name };
                self.hint.pos = p.pos;
            }
            if (res != .none) return .{ .push = ResourceWindow.create(c, r, self.market) catch return .none };
        }
        if (p.in(self.x + 343, self.y + 18, 88, 23)) self.hint.phrase(c, ph_gold, p.pos);
        if (p.in(self.x + 343, self.y + 41, 88, 23)) self.hint.phrase(c, ph_gem, p.pos);
        return .none;
    }

    fn rarityColor(r: i32) rl.Color {
        const v: [3]u8 = switch (r) {
            5 => .{ 180, 70, 250 },
            4 => .{ 130, 130, 255 },
            3 => .{ 50, 220, 220 },
            2 => .{ 50, 250, 50 },
            1 => .{ 220, 220, 50 },
            7 => .{ 240, 240, 240 },
            6 => .{ 250, 50, 50 },
            else => .{ 220, 220, 220 },
        };
        return .{ .r = v[0], .g = v[1], .b = v[2], .a = 255 };
    }

    pub fn draw(self: *ItemInfo, c: *Ctx) void {
        const pr = c.proto;
        const it = &pr.items[@intCast(self.item)];
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 128 });
        const win = self.sh.iface.texIndex(gui_item_info) orelse return;
        const eff = self.sh.iface.texIndex(gui_item_effect);
        const eff_h = gfx.texH(eff);
        const ww: f32 = @floatFromInt(win.width);
        const wh: f32 = @floatFromInt(win.height);
        const x = self.x;
        const y = self.y;
        const nr: f32 = @floatFromInt(self.n_rows);
        rl.drawTextureRec(win, .{ .x = 0, .y = 0, .width = ww, .height = @min(120 + eff_h * nr, wh) }, .{ .x = x, .y = y }, .white);
        rl.drawTextureRec(win, .{ .x = 0, .y = wh - 75, .width = ww, .height = 75 }, .{ .x = x, .y = y + self.h - 75 }, .white);
        gfx.drawTex(self.sh.item(pr, self.item), x + 20, y + 23);
        c.font1.drawCentered(it.name, x + 211, y + 22, color1);
        c.font1.drawCentered(it.name, x + 210, y + 21, rarityColor(it.rarity));
        const cls = if (it.class >= 0 and it.class < pr.item_types.len) pr.item_types[@intCast(it.class)] else "";
        c.font1.drawCentered(cls, x + 210, y + 50, color2);
        const slot: usize = @intCast(std.math.clamp(if (it.slot == 100) 12 else it.slot, 0, @as(i32, @intCast(pr.item_slots.len)) - 1));
        if (pr.item_slots.len > 0) c.font1.drawCentered(pr.item_slots[slot], x + 210, y + 74, color2);
        var b: [128]u8 = undefined;
        c.font1.drawCentered(std.fmt.bufPrint(&b, "{s}: {d}/{d}", .{ pr.phrase(ph_durability), self.dur, it.durability }) catch "", x + 210, y + 98, color2);
        c.font1.drawCentered(castle_ui.fmtInt(&b, it.gold_cost), x + 390, y + 23, color2);
        c.font1.drawCentered(castle_ui.fmtInt(&b, it.gem_cost), x + 390, y + 48, color2);
        for (it.res, 0..) |r, i| {
            if (r <= 0) continue;
            const fi: f32 = @floatFromInt(i);
            gfx.drawTex(self.sh.res(r), x + 352 + 26 * @mod(fi, 3), y + 69 + 25 * @floor(fi / 3));
        }
        const p1 = it.effects[0].p1;
        for (self.rows[0..self.n_rows], 0..) |row, k| {
            const ry = y + 121 + eff_h * @as(f32, @floatFromInt(k));
            gfx.drawTex(eff, x + 11, ry);
            // bmpEff: black at 140/255 over the text part of the row.
            rl.drawRectangle(@intFromFloat(x + 76), @intFromFloat(ry + 3), 363, 61, .{ .r = 0, .g = 0, .b = 0, .a = 140 });
            const ix = x + 17;
            const iy = ry + 7;
            const ty = ry + 29;
            switch (row) {
                .spell => if (p1 > 0 and p1 < pr.spells.len) {
                    gfx.drawTex(self.sh.spell(p1), ix, iy);
                    c.font1.drawCentered(pr.spells[@intCast(p1)].name, x + 250, ty, color2);
                },
                .defender => if (p1 > 0 and p1 < pr.defenders.len) {
                    gfx.drawTex(self.sh.item(pr, self.item), ix, iy);
                    c.font1.drawCentered(pr.defenders[@intCast(p1)].name, x + 250, ty, color2);
                },
                .building => if (p1 > 0 and p1 < pr.outer_buildings.len) {
                    gfx.drawTex(self.sh.item(pr, self.item), ix, iy);
                    c.font1.drawCentered(pr.outer_buildings[@intCast(p1)].name, x + 250, ty, color2);
                },
                .ritual => if (p1 > 0 and p1 < pr.rituals.len) {
                    gfx.drawTex(self.sh.item(pr, self.item), ix, iy);
                    c.font1.drawCentered(pr.rituals[@intCast(p1)].name, x + 250, ty, color2);
                },
                .set => |s| {
                    gfx.drawTex(self.sh.iface.texIndex(gui_item_set), ix, iy);
                    c.font1.drawCentered(pr.item_sets[s].name, x + 250, ty, color2);
                },
                .effect => |i| self.drawEffect(c, it.effects[i], ix, iy, x + 250, ty),
            }
        }
        self.exit.draw(&self.sh.buttons, c.tick);
        self.hint.draw(c, &self.sh.iface);
    }

    /// An effect row: the ability_num.var picture and name with its power
    /// ("(n)" for a neutral one), "%" and "(отряд)".
    fn drawEffect(self: *ItemInfo, c: *Ctx, e: proto_mod.Ability3, ix: f32, iy: f32, tx: f32, ty: f32) void {
        const pr = c.proto;
        var buf: [128]u8 = undefined;
        var w: std.Io.Writer = .fixed(&buf);
        var effect: i32 = 1;
        var numeric = false;
        var percent = false;
        for (pr.abilities[1..], 1..) |a, j| if (a.num == e.id) {
            w.end = 0;
            w.writeAll(a.name) catch {};
            effect = a.effect;
            numeric = a.numeric;
            percent = a.percent;
            gfx.drawTex(self.abilities.texIndex(j), ix, iy);
        };
        if (numeric) {
            (if (e.p1 > 0) (if (effect == 0) w.print(" ({d})", .{e.p1}) else w.print(" +{d}", .{e.p1})) else w.print(" {d}", .{e.p1})) catch {};
            if (percent) w.writeAll("%") catch {};
        }
        if (e.p2 > 0) w.print(" ({s})", .{pr.phrase(ph_squad)}) catch {};
        const col: rl.Color = switch (effect) {
            -1 => .{ .r = 220, .g = 0, .b = 0, .a = 255 },
            0 => .{ .r = 220, .g = 220, .b = 0, .a = 255 },
            else => if (e.p1 < 0) rl.Color{ .r = 220, .g = 0, .b = 0, .a = 255 } else rl.Color{ .r = 50, .g = 220, .b = 50, .a = 255 },
        };
        c.font1.drawCentered(w.buffered(), tx, ty, col);
    }
};
