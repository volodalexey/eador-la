//! The info windows of the garrison screens (Windower.cpp):
//! WindowUnitTypeInfo (:1306) for a unit for hire, WindowUnitInfo (:916)
//! for a squad unit or the hero himself, and WindowResourceInfo (:1549).
//! Each is drawn over the darkened screen and closed by its button or Esc.
//!
//! The stat and ability icons show their names (MakeAbilityHint), the
//! fields their phrases; the rank opens WindowInfo (phrase 495, tutorial 86)
//! and a resource its window.  A click on a stat, an ability, the life,
//! morale or level field opens WindowAbilityInfo, on a medal
//! WindowMedalInfo.  The description scrolls with its arrows or the wheel.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Button = @import("../ui/button.zig").Button;
const Hint = @import("../ui/hint.zig").Hint;
const Mask = @import("../ui/mask.zig").Mask;
const castle_ui = @import("../ui/castle_ui.zig");
const hero_ui = @import("../ui/hero_ui.zig");
const proto_mod = @import("../data/proto.zig");
const unit_mod = @import("../game/unit.zig");
const Unit = unit_mod.Unit;
const Hero = @import("../game/hero.zig").Hero;
const player = @import("../game/player.zig");
const WindowInfo = @import("window_info.zig").WindowInfo;
const ScrollBox = @import("window_info.zig").ScrollBox;
const info_windows = @import("info_windows.zig");

/// interface.dat 17ResourceInfo, 26UnitInfo, 65UnitTypeInfo, 1 select.
const gui_resource_info = 17;
const gui_unit_info = 26;
const gui_unit_type_info = 65;
const gui_select = 1;
/// buttons.dat: exit, the ability scroll 76..79 / 80..83.
const butt_exit = 226;
const butt_up = 76;
const butt_down = 80;
/// stuff.dat: the number plates of ShowIconNumber, the rank picture.
const stuff_count1 = 32;
const stuff_count2 = 33;
const stuff_rank = 66;
/// Phrases: "уровень", karma names 303 + karma, the rank, the resource
/// fields 309..311.
const ph_level = 10;
const ph_karma = 303;
const ph_rank = 495;
const ph_res_field = 309;
/// tutorial.txt 86: the rank text; message.txt 24: the resource text.
const tut_rank = 86;
const msg_resource = 24;
/// unit_upg.var effects of GetGoldPayment: + percent, - gold.
const upg_pay_perc = 85;
const upg_pay_less = 54;

const color1 = castle_ui.color1;
const color2 = castle_ui.color2;
const yellow = castle_ui.yellow;
const dark: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 - 190 };

const max_abil = 50;

/// The stat grid: stat id by (row, column), rows 31 and columns 59 apart.
const grid = [4][3]i32{ .{ 1, 11, 12 }, .{ 2, 3, 7 }, .{ 4, 5, 6 }, .{ 8, 9, 10 } };
const cell_w = 59;
const cell_h = 31;

/// A field with a phrase hint (UI_Mask1 with iHint).
const Field = struct { x: f32, y: f32, w: f32, h: f32, hint: u32 };

const type_fields = [_]Field{
    .{ .x = 33, .y = 207, .w = 88, .h = 23, .hint = 307 },
    .{ .x = 126, .y = 207, .w = 88, .h = 23, .hint = 308 },
    .{ .x = 22, .y = 175, .w = 80, .h = 25, .hint = 90 },
    .{ .x = 104, .y = 175, .w = 80, .h = 25, .hint = 91 },
};
const unit_fields = [_]Field{
    .{ .x = 26, .y = 38, .w = 128, .h = 23, .hint = 88 },
    .{ .x = 174, .y = 38, .w = 128, .h = 23, .hint = 89 },
    .{ .x = 22, .y = 203, .w = 80, .h = 25, .hint = 90 },
    .{ .x = 104, .y = 203, .w = 80, .h = 25, .hint = 91 },
    .{ .x = 318, .y = 43, .w = 61, .h = 44, .hint = 5 },
};

/// A unit ability with a picture: ability_num.var record and its value.
const Abil = struct { num: i32, index: usize, value: i32 };

/// WindowUnitTypeInfo (`unit` null) or WindowUnitInfo.
pub const UnitWindow = struct {
    type: i32,
    unit: ?Unit,
    hero: ?*const Hero,
    exp_pct: i32,
    /// GetSquadMorale of the unit's slot.
    squad_morale: i32,
    sh: castle_ui.Sheets,
    shadow: gfx.Sheet,
    ability_pics: gfx.Sheet,
    medal_pics: gfx.Sheet,
    class_pics: ?gfx.Sheet,
    text_buf: [4096]u8 = undefined,
    text: []const u8 = "",
    box: ScrollBox,
    x: f32,
    y: f32,
    exit: Button,
    up: Button,
    down: Button,
    abil: [max_abil]Abil = undefined,
    n_abil: usize = 0,
    /// iAbilLine: the shown page of ten.
    page: usize = 0,
    hover_abil: ?usize = null,
    res_masks: [proto_mod.max_unit_res]Mask = undefined,
    hint: Hint = .{},
    old_cursor: u32,

    /// WindowUnitTypeInfo(t).
    pub fn createType(c: *Ctx, t: i32) !scene.Scene {
        return create(c, t, null, 0, null, 100);
    }

    /// WindowUnitInfo(u, morale) for a unit without a hero.
    pub fn createUnit(c: *Ctx, u: Unit, morale: i32) !scene.Scene {
        return create(c, u.type, u, morale, null, 100);
    }

    /// GUI_InfoBlock calls WindowUnitInfo(pHero->pSquad[0], 0).
    pub fn createHero(c: *Ctx, h: *const Hero, exp_pct: i32) !scene.Scene {
        const u = h.squad[0] orelse return error.NoUnit;
        return create(c, u.type, u, 0, h, exp_pct);
    }

    fn create(c: *Ctx, t: i32, u: ?Unit, morale: i32, hero: ?*const Hero, exp_pct: i32) !scene.Scene {
        if (t <= 0 or t >= c.proto.units.len) return error.NoUnit;
        const self = try c.gpa.create(UnitWindow);
        errdefer c.gpa.destroy(self);
        var sh = try castle_ui.Sheets.load(c);
        errdefer sh.deinit();
        var shadow = try gfx.Sheet.load(c.fs, "unit_shadow");
        errdefer shadow.deinit();
        var ability_pics = try gfx.Sheet.load(c.fs, "ability");
        errdefer ability_pics.deinit();
        var medal_pics = try gfx.Sheet.load(c.fs, "medal");
        errdefer medal_pics.deinit();
        var class_pics: ?gfx.Sheet = if (hero != null) try gfx.Sheet.load(c.fs, "hero_class") else null;
        errdefer if (class_pics) |*pics| pics.deinit();
        const wt = sh.iface.texIndex(if (u == null) gui_unit_type_info else gui_unit_info) orelse return error.MissingUnitInfo;
        const x: f32 = @floatFromInt(512 - @divTrunc(wt.width, 2));
        const y: f32 = @floatFromInt(384 - @divTrunc(wt.height, 2));
        var exit = try Button.init(&sh.buttons, x + (if (u == null) @as(f32, 284) else 336), y + 366, butt_exit, butt_exit + 1, butt_exit + 2);
        errdefer exit.deinit();
        exit.hotkey = .escape;
        var up = try Button.init(&sh.buttons, x + 313, y + 272, butt_up, butt_up + 1, butt_up + 2);
        errdefer up.deinit();
        up.inactive = butt_up + 3;
        var down = try Button.init(&sh.buttons, x + 313, y + 310, butt_down, butt_down + 1, butt_down + 2);
        errdefer down.deinit();
        down.inactive = butt_down + 3;
        // WindowUnitTypeInfo has no ability scroll buttons.
        up.enabled = u != null;
        down.enabled = u != null;
        self.* = .{
            .type = t,
            .unit = u,
            .hero = hero,
            .exp_pct = exp_pct,
            .squad_morale = morale,
            .sh = sh,
            .shadow = shadow,
            .ability_pics = ability_pics,
            .medal_pics = medal_pics,
            .class_pics = class_pics,
            .box = undefined,
            .x = x,
            .y = y,
            .exit = exit,
            .up = up,
            .down = down,
            .old_cursor = c.cursor,
        };
        for (&self.res_masks, 0..) |*m, i| m.* = Mask.rect(x + 224 + 26 * @as(f32, @floatFromInt(i)), y + 207, 20, 20);
        self.collect(c);
        self.text = self.proto(c).text;
        if (hero) |h| {
            var bufs: [2][4096]u8 = undefined;
            var k: usize = 0;
            for ([2]i32{ 10, 20 }, [2]i32{ 3, 19 }) |level, base| {
                if (h.level < level) continue;
                const ci = base + (h.class1 - 1) * 4 + h.class2;
                if (ci < 0 or ci >= c.proto.classes.len or c.proto.messages.len <= 52) continue;
                self.text = @import("../game/event.zig").format(&bufs[k], c.proto.messages[52], &.{ .{ .s = self.text }, .{ .s = c.proto.classes[@intCast(ci)].text } });
                k = 1 - k;
            }
            const n = @min(self.text.len, self.text_buf.len);
            @memcpy(self.text_buf[0..n], self.text[0..n]);
            self.text = self.text_buf[0..n];
        }
        const text_x: f32 = if (u == null) 342 else 403;
        const arrow_x: f32 = if (u == null) 481 else 542;
        self.box = try ScrollBox.init(c, &self.sh.buttons, x + text_x, y + 74, 300, 250, .{ x + arrow_x, y + 44 }, .{ x + arrow_x, y + 334 }, self.text);
        c.setCursor(0);
        return scene.Scene.of(UnitWindow, self, true);
    }

    pub fn deinit(self: *UnitWindow, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.box.deinit();
        self.down.deinit();
        self.up.deinit();
        self.exit.deinit();
        if (self.class_pics) |*pics| pics.deinit();
        self.medal_pics.deinit();
        self.ability_pics.deinit();
        self.shadow.deinit();
        self.sh.deinit();
    }

    fn proto(self: *const UnitWindow, c: *const Ctx) *const proto_mod.UnitType {
        return &c.proto.units[@intCast(self.type)];
    }

    /// The type's abilities, and for a unit its upgrades and medals
    /// (CheckAbility + CheckUpg; no hero bonus outside the hero's slot).
    fn skill(self: *const UnitWindow, c: *const Ctx, i: i32) i32 {
        if (self.unit) |*u| return u.checkAbility(c.proto, i) + self.upgrade(c, i);
        const bare: Unit = .{ .type = self.type, .life = 0, .exp = 0 };
        return bare.checkAbility(c.proto, i);
    }

    fn upgrade(self: *const UnitWindow, c: *const Ctx, i: i32) i32 {
        const u = self.unit orelse return 0;
        return u.checkUpg(c.proto, i) + if (self.hero) |h| h.heroBonus(c.proto, i) else @as(i32, 0);
    }

    /// arParam: GetLife .. GetMorale, or the type's values.
    fn stat(self: *const UnitWindow, c: *const Ctx, i: i32) i32 {
        const t = self.proto(c);
        const base = switch (i) {
            1 => t.life,
            2 => t.attack,
            3 => t.counter_attack,
            4 => t.defence,
            5 => t.ranged_defence,
            6 => t.resist,
            7 => t.speed,
            8 => t.ranged_attack,
            9 => t.shooting_range,
            10 => t.ammo,
            11 => t.stamina,
            12 => t.morale,
            else => 0,
        };
        if (self.unit == null) return base;
        return base + self.skill(c, i);
    }

    /// The abilities from 13 up that have an ability_num.var record; the
    /// type window keeps ten.
    fn collect(self: *UnitWindow, c: *Ctx) void {
        const limit: usize = if (self.unit == null) 10 else max_abil;
        var i: i32 = 13;
        while (i < 3000 and self.n_abil < limit) : (i += 1) {
            const v = self.skill(c, i);
            if (v <= 0) continue;
            for (c.proto.abilities[1..], 1..) |a, j| {
                if (a.num != i) continue;
                self.abil[self.n_abil] = .{ .num = i, .index = j, .value = if (a.numeric) v else 0 };
                self.n_abil += 1;
                break;
            }
        }
    }

    /// The abilities of the shown page.
    fn shown(self: *const UnitWindow) []const Abil {
        const first = @min(self.page * 10, self.n_abil);
        return self.abil[first..@min(first + 10, self.n_abil)];
    }

    /// GetGoldPayment / GetGemPayment.
    fn payment(self: *const UnitWindow, c: *const Ctx) [2]i32 {
        const t = self.proto(c);
        const u = self.unit orelse return .{ t.gold_payment, t.gem_payment };
        var gold = t.gold_payment;
        var gem = t.gem_payment;
        for (u.medals) |m| {
            if (m <= 0 or m >= c.proto.medals.len) continue;
            gold += c.proto.medals[@intCast(m)].gold_spent;
            gem += c.proto.medals[@intCast(m)].gem_spent;
        }
        gold += @divTrunc(gold * self.upgrade(c, upg_pay_perc), 100);
        gold -= self.upgrade(c, upg_pay_less);
        return .{ @max(gold, 0), gem };
    }

    /// GetMorale + iMorale / 2 + the squad morale.
    fn totalMorale(self: *const UnitWindow, c: *const Ctx) i32 {
        const u = self.unit orelse return 0;
        return self.stat(c, 12) + @divTrunc(u.morale, 2) + self.squad_morale;
    }

    pub fn update(self: *UnitWindow, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        if (self.exit.check(f)) return .pop;
        self.box.update(f);
        if (self.unit != null) {
            // iAbil - 1 after the count: the last page is (n-1) / 10.
            const last = if (self.n_abil > 0) (self.n_abil - 1) / 10 else 0;
            self.up.on = self.page > 0;
            self.down.on = self.page < last;
            if (self.up.check(f)) self.page -= 1;
            if (self.down.check(f)) self.page += 1;
        }
        const p = f.pointer;
        const x = self.x;
        const y = self.y;
        const top: f32 = if (self.unit == null) 43 else 67;
        // arParamMask: WindowAbilityInfo(i, arParam[i]).
        for (grid, 0..) |row, r| for (row, 0..) |id, k| {
            if (p.in(x + 18 + cell_w * @as(f32, @floatFromInt(k)), y + top + cell_h * @as(f32, @floatFromInt(r)), 58, 30)) {
                hero_ui.abilityHint(c, &self.hint, id, p.pos);
                if (released(p)) return info_windows.abilityAction(c, id, self.stat(c, id));
            }
        };
        const fields: []const Field = if (self.unit == null) &type_fields else &unit_fields;
        for (fields) |fd| if (p.in(x + fd.x, y + fd.y, fd.w, fd.h)) self.hint.phrase(c, fd.hint, p.pos);
        // arSpecMask of WindowUnitInfo: life, morale and the level.
        if (self.unit) |u| if (released(p)) {
            if (p.in(x + 26, y + 38, 128, 23)) return info_windows.abilityAction(c, 1, u.life);
            if (p.in(x + 174, y + 38, 128, 23)) return info_windows.abilityAction(c, 12, self.totalMorale(c));
            if (p.in(x + 318, y + 43, 61, 44)) return info_windows.abilityAction(c, 119, if (self.hero) |h| h.level else u.level);
            for (u.medals, 0..) |md, i| {
                if (md > 0 and p.in(x + 317, y + 85 + 60 * @as(f32, @floatFromInt(i)), 64, 64)) {
                    return .{ .push = info_windows.medal(c, md) catch return .none };
                }
            }
        };
        self.hover_abil = null;
        for (self.shown(), 0..) |a, k| {
            if (p.in(x + 11 + 60 * @as(f32, @floatFromInt(k % 5)), y + 234 + 60 * @as(f32, @floatFromInt(k / 5)), 64, 64)) {
                self.hover_abil = k;
                hero_ui.abilityHint(c, &self.hint, a.num, p.pos);
                if (released(p)) return info_windows.abilityAction(c, a.num, self.skill(c, a.num));
            }
        }
        // The rank (WindowUnitInfo: only for a unit without a hero).
        const rank_x: f32 = if (self.unit == null) 197 else 195;
        const rank_y: f32 = if (self.unit == null) 175 else 203;
        var rank = Mask.rect(x + rank_x, y + rank_y, 120, 20);
        if (self.hero == null and rank.over(p)) {
            self.hint.phrase(c, ph_rank, p.pos);
            if (p.primary.released or p.secondary.released) {
                const body = if (tut_rank < c.proto.tutorial.len) c.proto.tutorial[tut_rank] else "";
                return .{ .push = WindowInfo.create(c, c.proto.phrase(ph_rank), body, stuff_rank) catch return .none };
            }
        }
        if (self.unit == null) {
            const t = self.proto(c);
            for (t.res, &self.res_masks) |r, *m| {
                if (r <= 0 or r >= c.proto.resources.len) continue;
                const res = m.check(p);
                if (m.hot()) {
                    self.hint.what = .{ .string = c.proto.resources[@intCast(r)].name };
                    self.hint.pos = p.pos;
                }
                if (res != .none) return .{ .push = ResourceWindow.create(c, r, null) catch return .none };
            }
        }
        return .none;
    }

    fn drawNumber(self: *UnitWindow, c: *Ctx, x: f32, y: f32, n: i32) void {
        var b: [16]u8 = undefined;
        const s = castle_ui.fmtInt(&b, n);
        if (n < 10) {
            gfx.drawTex(self.sh.stuff.texIndex(stuff_count1), x + 37, y + 32);
            c.font1.draw(s, x + 40, y + 36, color1);
        } else {
            gfx.drawTex(self.sh.stuff.texIndex(stuff_count2), x + 29, y + 33);
            c.font1.drawCentered(s, x + 39, y + 37, color1);
        }
    }

    pub fn draw(self: *UnitWindow, c: *Ctx) void {
        const pr = c.proto;
        const t = self.proto(c);
        const x = self.x;
        const y = self.y;
        const is_type = self.unit == null;
        var b: [160]u8 = undefined;
        rl.drawRectangle(0, 0, 1024, 768, dark);
        gfx.drawTex(self.sh.iface.texIndex(if (is_type) gui_unit_type_info else gui_unit_info), x, y);
        if (proto_mod.UnitType.picIndex(@intCast(self.type))) |k| {
            const pic = self.sh.units.texIndex(k);
            const px = x + 255 - @floor(gfx.texW(pic) / 2);
            const py = y + (if (is_type) @as(f32, 105) else 132) - @floor(gfx.texH(pic) / 2);
            gfx.drawTex2(self.shadow.texIndex(k), px, py, .{ .r = 255, .g = 255, .b = 255, .a = 140 });
            gfx.drawTex(pic, px, py);
        }
        const karma: i32 = std.math.clamp(t.karma, -3, 3);
        const name = if (self.hero) |h|
            std.fmt.bufPrint(&b, "{s} {s}", .{ h.nameText(pr), pr.classes[h.className()].name }) catch ""
        else
            std.fmt.bufPrint(&b, "{s} ({s})", .{ t.name, pr.phrase(@intCast(ph_karma + karma)) }) catch "";
        c.font1.drawCentered(name, x + (if (is_type) @as(f32, 325) else 370), y + 17, color1);
        const lvl = if (self.hero != null) pr.phrase(11) else std.fmt.bufPrint(&b, "{s} {d}", .{ pr.phrase(ph_level), t.level }) catch "";
        c.font1.drawCentered(lvl, x + 254, y + (if (is_type) @as(f32, 185) else 208), color1);

        const top: f32 = if (is_type) 55 else 79;
        for (grid, 0..) |row, r| for (row, 0..) |id, k| {
            c.font1.drawCentered(castle_ui.fmtInt(&b, self.stat(c, id)), x + 60 + cell_w * @as(f32, @floatFromInt(k)), y + top + cell_h * @as(f32, @floatFromInt(r)), color2);
        };
        const pay = self.payment(c);
        if (is_type) {
            c.font1.drawCentered(castle_ui.fmtInt(&b, pay[0]), x + 64, y + 182, color2);
            c.font1.drawCentered(castle_ui.fmtInt(&b, pay[1]), x + 152, y + 182, color2);
            c.font1.drawCentered(castle_ui.fmtInt(&b, t.gold_price), x + 82, y + 213, color2);
            c.font1.drawCentered(castle_ui.fmtInt(&b, t.gem_price), x + 177, y + 213, color2);
            for (t.res, 0..) |r, i| {
                if (r > 0) gfx.drawTex(self.sh.res(r), x + 224 + 26 * @as(f32, @floatFromInt(i)), y + 207);
            }
        } else {
            const u = self.unit.?;
            c.font1.drawCentered(castle_ui.fmtInt(&b, pay[0]), x + 64, y + 210, color2);
            c.font1.drawCentered(castle_ui.fmtInt(&b, pay[1]), x + 150, y + 210, color2);
            for (u.medals, 0..) |m, i| {
                if (m > 0) gfx.drawTex(self.medal_pics.texIndex(@intCast(m - 1)), x + 323, y + 91 + 60 * @as(f32, @floatFromInt(i)));
            }
            // The experience, life and morale bars, 2 px a step.
            const level = if (self.hero) |h| h.level else u.level;
            if (self.hero) |h| {
                const e0 = h.expForLevel(pr, level, self.exp_pct);
                const e1 = h.expForLevel(pr, level + 1, self.exp_pct);
                bar(x + 319, y + 66, 18, share(h.exp - e0, e1 - e0, 58), heroExpCol);
                const pics = &self.class_pics.?;
                const c1 = h.class1 - 1;
                gfx.drawTex(pics.texIndex(@intCast(@max(c1, 0))), x + 323, y + 91);
                if (level >= 10) gfx.drawTex(pics.texIndex(@intCast(@max(3 + c1 * 4 + h.class2, 0))), x + 323, y + 151);
                if (level >= 20) gfx.drawTex(pics.texIndex(@intCast(@max(19 + c1 * 4 + h.class2, 0))), x + 323, y + 211);
            } else {
                const e0 = unit_mod.expForLevel(pr, u.type, level);
                const e1 = unit_mod.expForLevel(pr, u.type, level + 1);
                bar(x + 319, y + 66, 18, share(u.exp - e0, e1 - e0, 58), expCol);
            }
            c.font1.drawCentered(castle_ui.fmtInt(&b, level), x + 348, y + 71, color1);
            c.font1.drawCentered(castle_ui.fmtInt(&b, level), x + 347, y + 70, yellow);
            bar(x + 50, y + 40, 19, share(u.life, self.stat(c, 1), 100), lifeCol);
            shadowed(c, castle_ui.fmtInt(&b, u.life), x + 101, y + 45);
            const mo = self.totalMorale(c);
            if (self.stat(c, 12) > 0) bar(x + 204, y + 40, 19, share(mo, self.stat(c, 12), 100), moraleCol);
            shadowed(c, castle_ui.fmtInt(&b, mo), x + 257, y + 45);
        }
        self.box.draw(c, &self.sh.buttons, self.text);

        for (self.shown(), 0..) |a, k| {
            const ix = x + 17 + 60 * @as(f32, @floatFromInt(k % 5));
            const iy = y + 240 + 60 * @as(f32, @floatFromInt(k / 5));
            gfx.drawTex(self.ability_pics.texIndex(a.index), ix, iy);
            if (a.value > 0) self.drawNumber(c, ix, iy, a.value);
            if (self.hover_abil == k) gfx.drawTex(self.sh.iface.texIndex(gui_select), ix - 6, iy - 6);
        }
        self.up.draw(&self.sh.buttons, c.tick);
        self.down.draw(&self.sh.buttons, c.tick);
        self.exit.draw(&self.sh.buttons, c.tick);
        self.hint.draw(c, &self.sh.iface);
    }
};

/// WindowResourceInfo: with a market the player's count and surcharges
/// (GetResGoldPrice), without (CurrentPlayer 0) the base prices.
pub const ResourceWindow = struct {
    res: i32,
    market: ?player.Market,
    sh: castle_ui.Sheets,
    x: f32,
    y: f32,
    exit: Button,
    text_buf: [2048]u8 = undefined,
    text: []const u8 = "",
    hint: Hint = .{},
    old_cursor: u32,

    pub fn create(c: *Ctx, r: i32, m: ?player.Market) !scene.Scene {
        if (r <= 0 or r >= c.proto.resources.len) return error.NoResource;
        const self = try c.gpa.create(ResourceWindow);
        errdefer c.gpa.destroy(self);
        var sh = try castle_ui.Sheets.load(c);
        errdefer sh.deinit();
        const wt = sh.iface.texIndex(gui_resource_info) orelse return error.MissingResourceInfo;
        const x: f32 = @floatFromInt(512 - @divTrunc(wt.width, 2));
        const y: f32 = @floatFromInt(384 - @divTrunc(wt.height, 2));
        var exit = try Button.init(&sh.buttons, x + 168, y + 542, butt_exit, butt_exit + 1, butt_exit + 2);
        errdefer exit.deinit();
        exit.hotkey = .escape;
        self.* = .{ .res = r, .market = m, .sh = sh, .x = x, .y = y, .exit = exit, .old_cursor = c.cursor };
        const fmt = if (msg_resource < c.proto.messages.len) c.proto.messages[msg_resource] else "%s";
        self.text = formatStr(&self.text_buf, fmt, c.proto.resources[@intCast(r)].text);
        c.setCursor(0);
        return scene.Scene.of(ResourceWindow, self, true);
    }

    pub fn deinit(self: *ResourceWindow, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.exit.deinit();
        self.sh.deinit();
    }

    pub fn update(self: *ResourceWindow, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        if (self.exit.check(f)) return .pop;
        const p = f.pointer;
        const fields = [3][3]f32{ .{ 84, 42, 0 }, .{ 133, 87, 1 }, .{ 228, 42, 2 } };
        for (fields) |fd| {
            if (p.in(self.x + fd[0], self.y + 49, fd[1], 23)) self.hint.phrase(c, ph_res_field + @as(u32, @intFromFloat(fd[2])), p.pos);
        }
        return .none;
    }

    pub fn draw(self: *ResourceWindow, c: *Ctx) void {
        const x = self.x;
        const y = self.y;
        const res = &c.proto.resources[@intCast(self.res)];
        var b: [16]u8 = undefined;
        rl.drawRectangle(0, 0, 1024, 768, dark);
        gfx.drawTex(self.sh.iface.texIndex(gui_resource_info), x, y);
        // resource.dat: the small pictures, then the big ones (BigPic).
        const big = self.sh.resource.texIndex(@intCast(self.res - 1 + @as(i32, @intCast(c.proto.resources.len - 1))));
        gfx.drawTex(big, x + 21, y + 20);
        gfx.drawTex(big, x + 328, y + 20);
        c.font1.drawCentered(res.name, x + 200, y + 22, color1);
        if (self.market) |m| {
            const price = m.resPrice(self.res);
            c.font1.drawCentered(castle_ui.fmtInt(&b, m.resource(self.res)), x + 105, y + 56, color2);
            c.font1.drawCentered(castle_ui.fmtInt(&b, price[0]), x + 180, y + 56, color2);
            c.font1.drawCentered(castle_ui.fmtInt(&b, price[1]), x + 274, y + 56, color2);
        } else {
            c.font1.drawCentered(castle_ui.fmtInt(&b, res.gold), x + 180, y + 56, color2);
            c.font1.drawCentered(castle_ui.fmtInt(&b, res.gem), x + 274, y + 56, color2);
        }
        _ = c.font2.drawWrappedClip(self.text, x + 31, y + 112, 342, y + 112 + 393, color1);
        self.exit.draw(&self.sh.buttons, c.tick);
        self.hint.draw(c, &self.sh.iface);
    }
};

/// Check == 30 or 40: a button released over the mask.
fn released(p: input.Pointer) bool {
    return p.primary.released or p.secondary.released;
}

/// sprintf(out, fmt, s) for the one "%s" of a text.
pub fn formatStr(out: []u8, fmt: []const u8, s: []const u8) []const u8 {
    var w: std.Io.Writer = .fixed(out);
    if (std.mem.indexOf(u8, fmt, "%s")) |i| {
        w.print("{s}{s}{s}", .{ fmt[0..i], s, fmt[i + 2 ..] }) catch {};
    } else w.writeAll(fmt) catch {};
    return w.buffered();
}

/// `cur`·`width`/`max`, at most `width` (the original's Limit).
fn share(cur: i32, max: i32, width: i32) i32 {
    if (max <= 0) return 0;
    return std.math.clamp(@divTrunc(cur * width, max), 0, width);
}

/// rectfill(x+i, y, x+i+1, y+h-1) for i < limit.
fn bar(x: f32, y: f32, h: i32, limit: i32, comptime col: fn (i32) rl.Color) void {
    var i: i32 = 0;
    while (i < limit) : (i += 1) rl.drawRectangle(@intFromFloat(x + @as(f32, @floatFromInt(i))), @intFromFloat(y), 2, h, col(i));
}

fn shadowed(c: *Ctx, s: []const u8, x: f32, y: f32) void {
    c.font1.drawCentered(s, x + 1, y + 1, color1);
    c.font1.drawCentered(s, x, y, color2);
}

fn u8c(v: i32) u8 {
    return @intCast(std.math.clamp(v, 0, 255));
}

fn heroExpCol(i: i32) rl.Color {
    return .{ .r = u8c(60 + i * 2), .g = u8c(20 + @divTrunc(i, 2)), .b = u8c(70 + @divTrunc(i * 5, 2)), .a = 255 };
}

fn expCol(i: i32) rl.Color {
    return .{ .r = u8c(40 + i * 2), .g = u8c(10 + @divTrunc(i, 2)), .b = u8c(50 + @divTrunc(i * 5, 2)), .a = 255 };
}
fn lifeCol(i: i32) rl.Color {
    return .{ .r = u8c(50 + i * 2), .g = u8c(10 + @divTrunc(i, 2)), .b = u8c(10 + @divTrunc(i, 2)), .a = 255 };
}
fn moraleCol(i: i32) rl.Color {
    return .{ .r = u8c(10 + @divTrunc(i, 2)), .g = u8c(10 + @divTrunc(i, 2)), .b = u8c(50 + i * 2), .a = 255 };
}

test "resource text with one string" {
    var b: [64]u8 = undefined;
    try std.testing.expectEqualStrings("Ресурс: железо.", formatStr(&b, "Ресурс: %s.", "железо"));
    try std.testing.expectEqualStrings("без", formatStr(&b, "без", "x"));
}

test "bar share" {
    try std.testing.expectEqual(@as(i32, 50), share(5, 10, 100));
    try std.testing.expectEqual(@as(i32, 58), share(200, 10, 58));
    try std.testing.expectEqual(@as(i32, 0), share(3, 0, 58));
}
