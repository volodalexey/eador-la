//! Blocks shared by the castle and garrison screens: GUI_ResourceBlock
//! (Screens.cpp:1238), DrawUnit (Screens.cpp:131), MakeItemHint
//! (Windower.cpp:2755) and the tab buttons of the castle screens
//! (Castle.cpp: ScreenTreasury, HomeGarrison, HomeLibrary).
//!
//! Not ported: the alchemy mask of the resource bar (WindowAlchemy) and
//! the info windows a right click opens (WindowResourceInfo, ...).
const std = @import("std");
const rl = @import("raylib");
const Ctx = @import("../app.zig").Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const proto_mod = @import("../data/proto.zig");
const Hint = @import("hint.zig").Hint;
const Button = @import("button.zig").Button;
const Mask = @import("mask.zig").Mask;
const sound = @import("../platform/sound.zig");
const hero_mod = @import("../game/hero.zig");
const unit_mod = @import("../game/unit.zig");
const player = @import("../game/player.zig");
const castle_mod = @import("../game/castle.zig");
const text_mod = @import("text.zig");

pub const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
pub const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };
pub const yellow: rl.Color = .{ .r = 220, .g = 220, .b = 0, .a = 255 };

/// interface.dat objects.
pub const gui = struct {
    pub const spell_mask = 0;
    pub const spell_select = 1;
    pub const spell_shadow = 4;
    pub const unit_mask = 5;
    pub const unit_select = 6;
    pub const unit_closed = 7;
    pub const trash_norm = 8;
    pub const trash_act = 9;
    pub const pointer = 14;
    pub const exp_low = 16;
    pub const health_low = 18;
    pub const garrison = 28;
    pub const recruit_closed = 29;
    pub const no_res = 30;
    pub const shop = 33;
    pub const dead = 38;
    pub const castle = 50;
    pub const item_hint = 104;
    pub const treasury = 110;
    pub const home_garrison = 111;
    pub const home_barracks = 112;
    pub const home_library = 113;
    pub const build_bar = 41;
    pub const build_screen = 90;
    pub const built = 91;
    pub const no_gold = 92;
    pub const no_gem = 93;
    pub const inn_bld_hint = 94;
    pub const inn_bld_info = 95;
    pub const unit_slot = 96;
    pub const spell_slot = 97;
    pub const string = 98;
    pub const group_icon = 99;
    pub const group_hint = 100;
    pub const planned = 101;
    pub const unit_glow = 118;
};

/// buttons.dat: the trash button's pressed and inactive pictures.
pub const butt_trash_pressed = 231;
pub const butt_trash_inactive = 232;

/// The datafiles of the castle screens.
pub const Sheets = struct {
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    items: gfx.Sheet,
    defenders: gfx.Sheet,
    outbuild: gfx.Sheet,
    ritual: gfx.Sheet,
    units: gfx.Sheet,
    icons: gfx.Sheet,
    resource: gfx.Sheet,
    medals: gfx.Sheet,
    spells: gfx.Sheet,
    castle: gfx.Sheet,
    innerbld: gfx.Sheet,
    stuff: gfx.Sheet,

    const list = .{
        .{ "iface", "interface" },  .{ "buttons", "buttons" },   .{ "items", "items" },
        .{ "defenders", "defenders" }, .{ "outbuild", "outbuild" }, .{ "ritual", "ritual" },
        .{ "units", "units" },      .{ "icons", "unit_icons" },  .{ "resource", "resource" },
        .{ "medals", "medal_small" }, .{ "spells", "spells" },   .{ "castle", "castle" },
        .{ "innerbld", "innerbld" }, .{ "stuff", "stuff" },
    };

    pub fn load(c: *Ctx) !Sheets {
        var s: Sheets = undefined;
        var done: usize = 0;
        errdefer inline for (list, 0..) |e, i| {
            if (i < done) @field(s, e[0]).deinit();
        };
        inline for (list) |e| {
            @field(s, e[0]) = try gfx.Sheet.load(c.fs, e[1]);
            done += 1;
        }
        return s;
    }

    pub fn deinit(self: *Sheets) void {
        inline for (list) |e| @field(self, e[0]).deinit();
    }

    /// clItem::Pic.
    pub fn item(self: *Sheets, pr: *const proto_mod.Proto, id: i32) ?rl.Texture2D {
        if (id <= 0 or id >= pr.items.len) return null;
        const p = pr.items[@intCast(id)].pic orelse return null;
        const sh = switch (p.sheet) {
            .items => &self.items,
            .defenders => &self.defenders,
            .outbuild => &self.outbuild,
            .ritual => &self.ritual,
        };
        return sh.texIndex(p.index);
    }

    /// clResource::Pic.
    pub fn res(self: *Sheets, r: i32) ?rl.Texture2D {
        return if (r > 0) self.resource.texIndex(@intCast(r - 1)) else null;
    }

    pub fn spell(self: *Sheets, s: i32) ?rl.Texture2D {
        return if (s > 0) self.spells.texIndex(@intCast(s - 1)) else null;
    }

    pub fn unitPic(self: *Sheets, t: i32) ?rl.Texture2D {
        return if (t > 0) self.units.texIndex(@intCast(t - 1)) else null;
    }

    pub fn unitIcon(self: *Sheets, t: i32) ?rl.Texture2D {
        return if (t > 0) self.icons.texIndex(@intCast(t - 1)) else null;
    }

    /// clInnerBuilding::Pic: innerbld.dat object = building id (0 empty).
    pub fn building(self: *Sheets, b: i32) ?rl.Texture2D {
        return if (b >= 0) self.innerbld.texIndex(@intCast(b)) else null;
    }
};

pub fn fmtInt(buf: []u8, v: i32) []const u8 {
    return std.fmt.bufPrint(buf, "{d}", .{v}) catch "";
}

/// GUI_ResourceBlock at (x, y): gold, gems and resources 1..9.
pub const ResourceBar = struct {
    x: f32,
    y: f32,
    buf: [96]u8 = undefined,

    const w = 26;

    fn count(c: *const Ctx) usize {
        return @min(9, c.proto.resources.len -| 1);
    }

    /// The hint "name (count)" of the resource under the pointer; a click
    /// (left or right) returns the resource for WindowResourceInfo.
    pub fn update(self: *ResourceBar, c: *Ctx, m: player.Market, p: input.Pointer, hint: *Hint) ?i32 {
        for (1..count(c) + 1) |i| {
            const rx = self.x + 255 + w * @as(f32, @floatFromInt(i - 1));
            if (!p.in(rx, self.y, 21, 21)) continue;
            const r: i32 = @intCast(i);
            if (p.primary.released or p.secondary.released) return r;
            const t = std.fmt.bufPrint(&self.buf, "{s} ({d})", .{ c.proto.resources[i].name, m.resource(r) }) catch return null;
            hint.what = .{ .string = t };
            hint.pos = p.pos;
        }
        return null;
    }

    pub fn draw(self: *const ResourceBar, c: *Ctx, sh: *Sheets, m: player.Market) void {
        var b: [16]u8 = undefined;
        const r = m.ruler();
        c.font1.drawCentered(fmtInt(&b, r.gold), self.x + 86, self.y + 5, yellow);
        c.font1.drawCentered(fmtInt(&b, r.gem), self.x + 210, self.y + 5, yellow);
        for (1..count(c) + 1) |i| {
            const rx = self.x + 255 + w * @as(f32, @floatFromInt(i - 1));
            const ri: i32 = @intCast(i);
            if (m.resource(ri) > 0) {
                gfx.drawTex(sh.res(ri), rx, self.y);
            } else {
                rl.drawRectangle(@intFromFloat(rx), @intFromFloat(self.y), 21, 21, color1);
                gfx.drawTex2(sh.res(ri), rx, self.y, .{ .r = 255, .g = 255, .b = 255, .a = 100 });
            }
        }
    }
};

/// The part of a bar picture from row `top` down (blit of the low rows).
fn barFrom(t: ?rl.Texture2D, x: f32, y: f32, top: i32) void {
    const tx = t orelse return;
    const t0 = std.math.clamp(top, 0, tx.height);
    if (t0 >= tx.height) return;
    const ft: f32 = @floatFromInt(t0);
    rl.drawTextureRec(tx, .{ .x = 0, .y = ft, .width = @floatFromInt(tx.width), .height = @as(f32, @floatFromInt(tx.height)) - ft }, .{ .x = x, .y = y + ft }, .white);
}

/// DrawUnit for a unit without a hero: icon, morale (null: -1000, none),
/// health and experience bars, the dead mark, level and medals.
pub fn drawUnit(c: *Ctx, sh: *Sheets, u: *const unit_mod.Unit, x: f32, y: f32, morale: ?i32) void {
    const pr = c.proto;
    gfx.drawTex(sh.unitIcon(u.type), x, y);
    var b: [16]u8 = undefined;
    if (morale) |mo| if (mo != 0) {
        const s = (if (mo > 0) std.fmt.bufPrint(&b, "+{d}", .{mo}) else std.fmt.bufPrint(&b, "{d}", .{mo})) catch "";
        c.font1.draw(s, x + 23, y + 3, color1);
        c.font1.draw(s, x + 22, y + 2, if (mo > 0) rl.Color{ .r = 50, .g = 220, .b = 50, .a = 255 } else rl.Color{ .r = 220, .g = 50, .b = 50, .a = 255 });
    };
    const health = sh.iface.texIndex(gui.health_low);
    const hh: i32 = if (health) |t| t.height else 0;
    const max = @max(u.maxLife(pr), 1);
    barFrom(health, x + 59, y + 2, @divTrunc(hh * (max - u.life), max));
    if (u.life == 0) gfx.drawTex2(sh.iface.texIndex(gui.dead), x, y, .{ .r = 255, .g = 255, .b = 255, .a = 80 });
    const exp = sh.iface.texIndex(gui.exp_low);
    const eh: i32 = if (exp) |t| t.height else 0;
    const e0 = unit_mod.expForLevel(pr, u.type, u.level);
    const e1 = unit_mod.expForLevel(pr, u.type, u.level + 1);
    barFrom(exp, x + 1, y + 2, if (e1 > e0) @divTrunc(eh * (e1 - u.exp), e1 - e0) else eh);
    const lv = fmtInt(&b, u.level);
    c.font1.drawCentered(lv, x + 23, y + 70, color1);
    c.font1.drawCentered(lv, x + 22, y + 69, .{ .r = 200, .g = 100, .b = 255, .a = 255 });
    for (u.medals, 0..) |md, j| {
        if (md > 0) gfx.drawTex(sh.medals.texIndex(@intCast(md - 1)), 34 + x + 10 * @as(f32, @floatFromInt(j)), y + 67);
    }
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

/// An effect line of the item card: the ability_num.var name with its
/// power, "%" and " (squad)".
fn effectText(pr: *const proto_mod.Proto, buf: []u8, e: proto_mod.Ability3) struct { text: []const u8, effect: i32 } {
    var w: std.Io.Writer = .fixed(buf);
    var effect: i32 = 1;
    var numeric = false;
    var percent = false;
    for (pr.abilities[1..]) |a| if (a.num == e.id) {
        w.end = 0;
        w.writeAll(a.name) catch {};
        effect = a.effect;
        numeric = a.numeric;
        percent = a.percent;
    };
    if (numeric) {
        (if (e.p1 > 0) w.print(" +{d}", .{e.p1}) else w.print(" {d}", .{e.p1})) catch {};
        if (percent) w.writeAll("%") catch {};
    }
    if (e.p2 > 0) w.print(" ({s})", .{pr.phrase(17)}) catch {};
    return .{ .text = w.buffered(), .effect = effect };
}

/// MakeItemHint: the item card beside the pointer.  `price_text` > 0 is
/// the phrase over the price plaque (12 sell, 13 buy, 14 repair).
pub fn itemCard(c: *Ctx, sh: *Sheets, at: rl.Vector2, id: i32, dur: i32, hero: ?*const hero_mod.Hero, price_text: u32, price: player.Price, treasury: player.Price) void {
    const pr = c.proto;
    if (id <= 0 or id >= pr.items.len) return;
    const it = &pr.items[@intCast(id)];
    const bg = sh.iface.texIndex(gui.item_hint) orelse return;
    var h: i32 = 136;
    if (price_text > 0) h += 54;
    for (it.effects[0..it.n_effects]) |e| h += @as(i32, @intFromBool(e.id > 0)) * 20;
    var set: ?*const proto_mod.ItemSet = null;
    for (pr.item_sets[1..]) |*s| {
        if (std.mem.indexOfScalar(i32, s.items[0..s.n_items], id) == null) continue;
        set = s;
        h += 60 + @as(i32, @intCast(s.n_items)) * 20;
        for (s.bonuses[0..s.n_bonuses]) |bn| for (bn.abilities[0..bn.n_abilities]) |a| {
            h += @as(i32, @intFromBool(a.id > 0)) * 20;
        };
    }
    h = @min(h, 700);
    const hf: f32 = @floatFromInt(h);
    const w: f32 = @floatFromInt(bg.width);
    const bh: f32 = @floatFromInt(bg.height);
    const pos = (Hint{ .pos = at }).place(c, w, hf);
    const x = pos.x;
    const y = pos.y;
    var b: [128]u8 = undefined;
    if (price_text > 0) {
        rl.drawTextureRec(bg, .{ .x = 0, .y = 0, .width = w, .height = hf - 74 }, pos, .white);
        rl.drawTextureRec(bg, .{ .x = 0, .y = bh - 74, .width = w, .height = 74 }, .{ .x = x, .y = y + hf - 74 }, .white);
        c.font1.drawCentered(pr.phrase(price_text), x + 150, y + hf - 52, color1);
        const check = price_text == 13 or price_text == 14;
        const red: rl.Color = .{ .r = 250, .g = 30, .b = 30, .a = 255 };
        c.font1.drawCentered(fmtInt(&b, price[0]), x + 97, y + hf - 25, if (check and treasury[0] < price[0]) red else color2);
        c.font1.drawCentered(fmtInt(&b, price[1]), x + 207, y + hf - 25, if (check and treasury[1] < price[1]) red else color2);
    } else {
        rl.drawTextureRec(bg, .{ .x = 0, .y = 0, .width = w, .height = hf - 20 }, pos, .white);
        rl.drawTextureRec(bg, .{ .x = 0, .y = 696, .width = w, .height = 11 }, .{ .x = x, .y = y + hf - 20 }, .white);
        rl.drawTextureRec(bg, .{ .x = 0, .y = bh - 9, .width = w, .height = 9 }, .{ .x = x, .y = y + hf - 9 }, .white);
    }
    c.font1.drawCentered(it.name, x + 151, y + 20, color1);
    c.font1.drawCentered(it.name, x + 150, y + 19, rarityColor(it.rarity));
    const cant = if (hero) |hr| it.class < 10 and !hr.canUse(pr, id) else false;
    const cls = if (it.class >= 0 and it.class < pr.item_types.len) pr.item_types[@intCast(it.class)] else "";
    c.font1.drawCentered(cls, x + 150, y + 50, if (cant) rl.Color{ .r = 250, .g = 30, .b = 30, .a = 255 } else color2);
    if (it.class > 10) return;
    const slot: usize = @intCast(std.math.clamp(if (it.slot == 100) 12 else it.slot, 0, @as(i32, @intCast(pr.item_slots.len)) - 1));
    c.font1.drawCentered(pr.item_slots[slot], x + 150, y + 74, color2);
    c.font1.drawCentered(std.fmt.bufPrint(&b, "{s}: {d}/{d}", .{ pr.phrase(16), dur, it.durability }) catch "", x + 150, y + 98, color2);
    var sy = y + 128;
    for (it.effects[0..it.n_effects]) |e| {
        if (e.id <= 0) continue;
        const t = effectText(pr, &b, e);
        const col: rl.Color = switch (t.effect) {
            -1 => .{ .r = 220, .g = 0, .b = 0, .a = 255 },
            0 => .{ .r = 220, .g = 220, .b = 0, .a = 255 },
            else => if (e.p1 < 0) rl.Color{ .r = 220, .g = 0, .b = 0, .a = 255 } else rl.Color{ .r = 50, .g = 220, .b = 50, .a = 255 },
        };
        c.font1.drawCentered(t.text, x + 150, sy, col);
        sy += 20;
    }
    sy += 10;
    const s = set orelse return;
    c.font1.drawCentered(s.name, x + 150, sy, color2);
    sy += 20;
    const dim: rl.Color = .{ .r = 100, .g = 100, .b = 100, .a = 255 };
    const lit: rl.Color = .{ .r = 220, .g = 220, .b = 50, .a = 255 };
    var worn: i32 = 0;
    for (s.items[0..s.n_items]) |si| {
        if (si <= 0 or si >= pr.items.len) continue;
        var on = false;
        if (hero) |hr| for (hr.inventory[1..16]) |inv| {
            if (inv[0] == si) on = true;
        };
        worn += @intFromBool(on);
        c.font1.drawCentered(pr.items[@intCast(si)].name, x + 150, sy, if (on) lit else dim);
        sy += 20;
    }
    sy += 10;
    c.font1.drawCentered(pr.phrase(72), x + 150, sy, color2);
    sy += 20;
    for (s.bonuses[0..s.n_bonuses]) |bn| for (bn.abilities[0..bn.n_abilities]) |a| {
        if (a.id <= 0) continue;
        var eb: [96]u8 = undefined;
        const t = effectText(pr, &eb, a);
        c.font1.drawCentered(std.fmt.bufPrint(&b, "({d}) {s}", .{ bn.items_needed, t.text }) catch "", x + 150, sy, if (worn < bn.items_needed) dim else lit);
        sy += 20;
    };
}

/// PickFlag, PickItem, PickDur of GUI_ShopBlock: what its plaque shows.
pub const Pick = struct { flag: enum { none, sell, buy, repair } = .none, item: i32 = 0, dur: i32 = 0 };

/// The plaque of GUI_ShopBlock::Show at the shop's corner (x, y): the
/// sale, purchase or repair price of `pick`, with the resources of a
/// purchase or repair (crossed out where missing).  `repair_all` is the
/// price of buttRepairAll, shown for a hero.
pub fn shopPlaque(c: *Ctx, sh: *Sheets, m: player.Market, shop: *const @import("../game/site.zig").Shop, pick: Pick, repair_all: ?player.Price, x: f32, y: f32) void {
    const pr = c.proto;
    const r = m.ruler();
    const red: rl.Color = .{ .r = 220, .g = 50, .b = 20, .a = 255 };
    var b: [16]u8 = undefined;
    if (repair_all) |p| {
        const short: rl.Color = .{ .r = 220, .g = 40, .b = 20, .a = 255 };
        c.font1.drawCentered(fmtInt(&b, p[0]), x + 180, y + 686, if (p[0] > r.gold) short else color2);
        c.font1.drawCentered(fmtInt(&b, p[1]), x + 180, y + 715, if (p[1] > r.gem and p[1] > 0) short else color2);
    }
    const it = pick.item;
    if (it <= 0 or it >= pr.items.len) return;
    const res = struct {
        fn f(cc: *Ctx, s: *Sheets, mm: player.Market, item: i32, bx: f32, by: f32) void {
            for (cc.proto.items[@intCast(item)].res, 0..) |rs, i| {
                if (rs <= 0) continue;
                const rx = bx + 323 + 26 * @as(f32, @floatFromInt(i % 3));
                const ry = by + 694 + 25 * @as(f32, @floatFromInt(i / 3));
                gfx.drawTex(s.res(rs), rx, ry);
                if (mm.resource(rs) == 0) gfx.drawTex(s.iface.texIndex(gui.no_res), rx, ry);
            }
        }
    }.f;
    switch (pick.flag) {
        .none => {},
        .sell => {
            const p = player.sellPrice(pr, it, pick.dur);
            c.font1.drawCentered(pr.phrase(12), x + 313, y + 673, color1);
            c.font1.drawCentered(fmtInt(&b, p[0]), x + 280, y + 700, color2);
            c.font1.drawCentered(fmtInt(&b, p[1]), x + 280, y + 724, color2);
        },
        .buy => {
            const p = m.itemPrice(it, shop);
            c.font1.drawCentered(pr.phrase(13), x + 313, y + 673, color1);
            c.font1.drawCentered(fmtInt(&b, p[0]), x + 280, y + 700, if (p[0] > r.gold) red else color2);
            c.font1.drawCentered(fmtInt(&b, p[1]), x + 280, y + 724, if (p[1] > r.gem) red else color2);
            res(c, sh, m, it, x, y);
        },
        .repair => {
            const p = m.repairPrice(it, pick.dur, shop);
            if (p[0] + p[1] <= 0) {
                c.font1.drawCentered(pr.phrase(15), x + 313, y + 673, color1);
                return;
            }
            c.font1.drawCentered(pr.phrase(14), x + 313, y + 673, color1);
            c.font1.drawCentered(fmtInt(&b, p[0]), x + 280, y + 700, if (p[0] > r.gold) red else color2);
            c.font1.drawCentered(fmtInt(&b, p[1]), x + 280, y + 724, if (p[1] > r.gem) red else color2);
            res(c, sh, m, it, x, y);
        },
    }
}

/// makecol(250, 0, 0) of the building windows.
pub const short_red: rl.Color = .{ .r = 250, .g = 0, .b = 0, .a = 255 };

/// MakeInnBldHint (Windower.cpp:2994): the building card beside the
/// pointer (94InnBldHint): both pictures, the name, the price (red where
/// the treasury is short), the resources (crossed out where missing) and
/// the description.
pub fn buildingCard(c: *Ctx, sh: *Sheets, at: rl.Vector2, m: player.Market, id: i32) void {
    const pr = c.proto;
    if (id <= 0 or id >= pr.inner_buildings.len) return;
    const b = &pr.inner_buildings[@intCast(id)];
    const bg = sh.iface.texIndex(gui.inn_bld_hint) orelse return;
    const pos = (Hint{ .pos = at }).place(c, @floatFromInt(bg.width), @floatFromInt(bg.height));
    const x = pos.x;
    const y = pos.y;
    gfx.drawTex(bg, x, y);
    gfx.drawTex(sh.building(id), x + 14, y + 44);
    gfx.drawTex(sh.building(id), x + 290, y + 44);
    c.font1.drawCentered(b.name, x + 176, y + 15, color1);
    const price = castle_mod.innerPrice(m, id);
    const r = m.ruler();
    var buf: [16]u8 = undefined;
    c.font1.drawCentered(fmtInt(&buf, price[0]), x + 141, y + 52, if (r.gold >= price[0]) color2 else short_red);
    c.font1.drawCentered(fmtInt(&buf, price[1]), x + 141, y + 77, if (r.gem >= price[1] or price[1] == 0) color2 else short_red);
    for (b.res, 0..) |rs, i| {
        if (rs <= 0) continue;
        const rx = x + 205 + 26 * @as(f32, @floatFromInt(i % 3));
        const ry = y + 47 + 25 * @as(f32, @floatFromInt(i / 3));
        gfx.drawTex(sh.res(rs), rx, ry);
        if (m.resource(rs) <= 0) gfx.drawTex(sh.iface.texIndex(gui.no_res), rx, ry);
    }
    const box = text_mod.Box.init(x + 22, y + 130, 312, 142, &c.font2, color1);
    _ = box.draw(b.text);
}

/// MakeGroupHint (Windower.cpp:3138): the group card (A00GroupHint) with
/// its limit, name and the count built.
pub fn groupCard(c: *Ctx, sh: *Sheets, at: rl.Vector2, rules: castle_mod.Rules, g: i32) void {
    const pr = c.proto;
    if (g < 0 or g >= pr.build_groups.len) return;
    const bg = sh.iface.texIndex(gui.group_hint) orelse return;
    const w: f32 = @floatFromInt(bg.width);
    const pos = (Hint{ .pos = at }).place(c, w, @floatFromInt(bg.height));
    gfx.drawTex(bg, pos.x, pos.y);
    var buf: [128]u8 = undefined;
    c.font1.drawCentered(formatInt(&buf, pr.phrase(250), rules.groupMax(g)), pos.x + w / 2, pos.y + 17, color1);
    c.font1.drawCentered(pr.build_groups[@intCast(g)].name, pos.x + w / 2, pos.y + 37, color1);
    const n = rules.groupCount(g, pr.inner_buildings.len -| 1);
    const t = std.fmt.bufPrint(&buf, "({s}: {d})", .{ pr.phrase(244), n }) catch "";
    c.font1.drawCentered(t, pos.x + w / 2, pos.y + 67, color1);
}

/// sprintf(text, n) for the one "%d" of a phrase.
pub fn formatInt(buf: []u8, s: []const u8, n: i32) []const u8 {
    var w: std.Io.Writer = .fixed(buf);
    if (std.mem.indexOf(u8, s, "%d")) |i| {
        w.print("{s}{d}{s}", .{ s[0..i], n, s[i + 2 ..] }) catch {};
    } else w.writeAll(s) catch {};
    return w.buffered();
}

/// The trash button (bmpGUI_ButtTrashNorm/Act, buttons.dat 231/232) with
/// the Delete hotkey; Activate(0) while nothing is held.
pub const Trash = struct {
    mask: Mask,
    shape: rl.Image,
    on: bool = false,
    /// wavSound: the click, or unit sound 158 (wavDelete).
    sound: i32 = 0,

    pub fn init(sh: *Sheets, x: f32, y: f32) !Trash {
        return .{ .mask = .{ .x = x, .y = y }, .shape = sh.iface.imageIndex(gui.trash_norm) orelse return error.MissingButton };
    }

    pub fn deinit(self: *Trash) void {
        rl.unloadImage(self.shape);
    }

    pub fn check(self: *Trash, f: *const input.Frame) bool {
        if (!self.on) {
            self.mask.state = .visible;
            return false;
        }
        self.mask.shape = &self.shape;
        const hit = f.key(.delete) or self.mask.check(f.pointer) == .click;
        if (hit) sound.button(self.sound);
        return hit;
    }

    pub fn draw(self: *const Trash, sh: *Sheets) void {
        const t = if (!self.on) sh.buttons.texIndex(butt_trash_inactive) else switch (self.mask.state) {
            .visible => sh.iface.texIndex(gui.trash_norm),
            .hot, .right_pressed => sh.iface.texIndex(gui.trash_act),
            .pressed => sh.buttons.texIndex(butt_trash_pressed),
        };
        gfx.drawTex(t, self.mask.x, self.mask.y);
    }
};

/// UI_Button2 (Interface.cpp:543): a button that stays pressed until the
/// next click; iHint while up, iHint1 while down.
pub const Toggle = struct {
    mask: Mask,
    shape: rl.Image,
    first: usize,
    pressed: bool = false,
    hotkey: ?rl.KeyboardKey = null,
    hint: u32 = 0,
    hint_down: u32 = 0,

    /// buttons.dat first (normal), +1 active, +2 pressed.
    pub fn init(sh: *Sheets, x: f32, y: f32, first: usize) !Toggle {
        return .{ .mask = .{ .x = x, .y = y }, .shape = sh.buttons.imageIndex(first) orelse return error.MissingButton, .first = first };
    }

    pub fn deinit(self: *Toggle) void {
        rl.unloadImage(self.shape);
    }

    pub fn check(self: *Toggle, f: *const input.Frame) bool {
        self.mask.shape = &self.shape;
        const hit = (self.hotkey != null and f.key(self.hotkey.?)) or self.mask.check(f.pointer) == .click;
        if (hit) {
            self.pressed = !self.pressed;
            sound.click();
        }
        return hit;
    }

    pub fn hintPhrase(self: *const Toggle) u32 {
        return if (self.pressed) self.hint_down else self.hint;
    }

    pub fn draw(self: *const Toggle, sh: *Sheets) void {
        const i = if (self.pressed) self.first + 2 else if (self.mask.hot()) self.first + 1 else self.first;
        gfx.drawTex(sh.buttons.texIndex(i), self.mask.x, self.mask.y);
    }
};

/// The mouse sprite of a picked item or unit: the picture with
/// bmpGUI_Pointer over it.  The pointer sprite of the frame is hidden.
pub fn drawHeld(sh: *Sheets, pic: ?rl.Texture2D, p: input.Pointer) void {
    if (pic == null or !p.hover) return;
    gfx.hide_cursor = true;
    gfx.drawTex(pic, @floor(p.pos.x), @floor(p.pos.y));
    gfx.drawTex(sh.iface.texIndex(gui.pointer), @floor(p.pos.x), @floor(p.pos.y));
}

/// The buttons of every castle screen: the six tabs and the exit.
pub const Tab = enum { build, hero, garrison, treasury, library, stats };

pub const TabBar = struct {
    tabs: std.enums.EnumArray(Tab, Button),
    exit: Button,

    pub const Click = union(enum) { none, exit, tab: Tab };

    const Spec = struct { x: f32, first: usize, hint: u32, key: ?rl.KeyboardKey };
    const specs = std.enums.EnumArray(Tab, Spec).init(.{
        .build = .{ .x = 61, .first = 267, .hint = 141, .key = .c },
        .hero = .{ .x = 121, .first = 250, .hint = 136, .key = .f },
        .garrison = .{ .x = 181, .first = 263, .hint = 94, .key = .d },
        .treasury = .{ .x = 241, .first = 259, .hint = 102, .key = .a },
        .library = .{ .x = 301, .first = 271, .hint = 318, .key = .s },
        .stats = .{ .x = 361, .first = 275, .hint = 99, .key = null },
    });
    const y = 686;

    /// The tab of `current` is Activate(-2): pressed, no clicks.
    pub fn init(sheet: *gfx.Sheet, current: Tab) !TabBar {
        var self: TabBar = undefined;
        var done: usize = 0;
        errdefer for (self.tabs.values[0..done]) |*b| b.deinit();
        for (&self.tabs.values, specs.values, 0..) |*b, s, i| {
            b.* = try Button.init(sheet, s.x, y, s.first, s.first + 1, s.first + 2);
            b.inactive = s.first + 3;
            b.hint = s.hint;
            b.hotkey = s.key;
            if (@as(Tab, @enumFromInt(i)) == current) {
                b.on = false;
                b.inactive = s.first + 2;
            }
            done += 1;
        }
        self.exit = try Button.init(sheet, 451, y, 256, 257, 258);
        self.exit.hint = 135;
        self.exit.hotkey = .escape;
        return self;
    }

    pub fn deinit(self: *TabBar) void {
        for (&self.tabs.values) |*b| b.deinit();
        self.exit.deinit();
    }

    pub fn check(self: *TabBar, f: *const input.Frame) Click {
        var r: Click = .none;
        if (self.exit.check(f)) r = .exit;
        for (&self.tabs.values, 0..) |*b, i| {
            if (b.check(f) and r == .none) r = .{ .tab = @enumFromInt(i) };
        }
        return r;
    }

    pub fn hint(self: *const TabBar, c: *Ctx, h: *Hint, at: rl.Vector2) void {
        if (self.exit.hot()) h.phrase(c, self.exit.hint, at);
        for (&self.tabs.values) |*b| if (b.hot()) h.phrase(c, b.hint, at);
    }

    pub fn draw(self: *const TabBar, sheet: *gfx.Sheet, tick: u16) void {
        for (&self.tabs.values) |*b| b.draw(sheet, tick);
        self.exit.draw(sheet, tick);
    }
};
