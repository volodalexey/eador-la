//! BF_Unit::ShowInfoScreen (BF_Unit.cpp:4682): the battle unit window over
//! the field: the picture, the medals (or the hero's classes), the level
//! and experience bar, life, stamina, morale, speed and ammo bars, the
//! twelve stats, the abilities and the visible effects (five a row, with
//! scroll buttons) and the unit's text with arrows and wheel scrolling.
//!
//! A click on a stat, a bar, an ability, a medal or an effect opens
//! WindowAbilityInfo, WindowMedalInfo or WindowEffectInfo (BF_Unit.cpp:
//! 5090); hovering shows the name (MakeAbilityHint, MakeStringHint).
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Button = @import("../ui/button.zig").Button;
const Hint = @import("../ui/hint.zig").Hint;
const bt = @import("../game/battle.zig");
const unit_mod = @import("../game/unit.zig");
const hero_ui = @import("../ui/hero_ui.zig");
const info_windows = @import("info_windows.zig");
const ScrollBox = @import("window_info.zig").ScrollBox;

/// interface.dat 74BallteUnitInfo, 1SpellSelect.
const gui_window = 74;
const gui_select = 1;
/// buttons.dat: exit, scroll up 76..79, scroll down 80..83.
const butt_exit = 226;
const butt_up = 76;
const butt_down = 80;
/// stuff.dat: the number plates of ShowIconNumber.
const stuff_count1 = 32;
const stuff_count2 = 33;
/// ability_num.var record of a summoned unit.
const ability_summoned = 103;
/// Phrases: the karma names (303 + karma), "Свойства", "Эффекты".
const ph_karma = 303;
const ph_abilities = 84;
const ph_effects = 85;
/// message.txt 52: "%s ... %s", the unit text with a class text.
const msg_class = 52;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };
const yellow: rl.Color = .{ .r = 220, .g = 220, .b = 0, .a = 255 };

const max_icons = 50;

/// An icon of the ability or effect rows.
const Icon = struct {
    sheet: enum { ability, spell, skill, none },
    index: usize = 0,
    /// The number on the plate (0: none).
    number: i32 = 0,
    name: []const u8 = "",
    /// An ability: its ability_num.var number.  An effect: pAbility
    /// (unit_upg.var), pSpell, iTime and iPower.
    num: i32 = 0,
    upgrade: i32 = 0,
    spell: i32 = 0,
    time: i32 = 0,
    power: i32 = 0,
};

/// arParamMask: the stat of each cell of the 3×4 grid, (18 + 59·k,
/// 45 + 31·r), 58×30.
const grid = [4][3]i32{ .{ 1, 11, 12 }, .{ 2, 3, 7 }, .{ 4, 5, 6 }, .{ 8, 9, 10 } };
/// arParamMask2: life, stamina, morale, speed, ammo bars with their
/// phrases and WindowAbilityInfo numbers.
const bars = [5]struct { x: f32, y: f32, w: f32, hint: u32, num: i32 }{
    .{ .x = 91, .y = 175, .w = 150, .hint = 88, .num = 1 },
    .{ .x = 29, .y = 200, .w = 126, .hint = 492, .num = 11 },
    .{ .x = 29, .y = 224, .w = 126, .hint = 89, .num = 12 },
    .{ .x = 170, .y = 200, .w = 126, .hint = 493, .num = 7 },
    .{ .x = 170, .y = 224, .w = 126, .hint = 494, .num = 10 },
};

pub const UnitInfo = struct {
    b: *bt.Battle,
    ref: bt.Ref,
    x: f32,
    y: f32,
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    stuff: gfx.Sheet,
    units: gfx.Sheet,
    shadow: gfx.Sheet,
    ability_pics: gfx.Sheet,
    spell_pics: gfx.Sheet,
    skill_pics: gfx.Sheet,
    medal_pics: gfx.Sheet,
    class_pics: gfx.Sheet,
    exit: Button,
    up: [2]Button,
    down: [2]Button,
    abil: [max_icons]Icon = undefined,
    n_abil: usize = 0,
    eff: [max_icons]Icon = undefined,
    n_eff: usize = 0,
    /// iAbilLine, iEffLine.
    line: [2]usize = .{ 0, 0 },
    /// The text box (the hero's text with its classes).
    text_buf: [2048]u8 = undefined,
    text: []const u8 = "",
    box: ScrollBox,
    /// ExpForLevel's factor for a hero.
    exp_pct: i32,
    hint: Hint = .{},
    hover: ?[2]usize = null,
    old_cursor: u32,

    pub fn create(c: *Ctx, b: *bt.Battle, ref: bt.Ref, exp_pct: i32) !scene.Scene {
        const self = try c.gpa.create(UnitInfo);
        errdefer c.gpa.destroy(self);
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer buttons.deinit();
        var stuff = try gfx.Sheet.load(c.fs, "stuff");
        errdefer stuff.deinit();
        var units = try gfx.Sheet.load(c.fs, "units");
        errdefer units.deinit();
        var shadow = try gfx.Sheet.load(c.fs, "unit_shadow");
        errdefer shadow.deinit();
        var ability_pics = try gfx.Sheet.load(c.fs, "ability");
        errdefer ability_pics.deinit();
        var spell_pics = try gfx.Sheet.load(c.fs, "spells");
        errdefer spell_pics.deinit();
        var skill_pics = try gfx.Sheet.load(c.fs, "skills");
        errdefer skill_pics.deinit();
        var medal_pics = try gfx.Sheet.load(c.fs, "medal");
        errdefer medal_pics.deinit();
        var class_pics = try gfx.Sheet.load(c.fs, "hero_class");
        errdefer class_pics.deinit();
        const wt = iface.texIndex(gui_window) orelse return error.MissingUnitInfo;
        const x: f32 = @floatFromInt(512 - @divTrunc(wt.width, 2));
        const y: f32 = @floatFromInt(384 - @divTrunc(wt.height, 2));
        var exit = try Button.init(&buttons, x + 336, y + 442, butt_exit, butt_exit + 1, butt_exit + 2);
        errdefer exit.deinit();
        exit.hotkey = .escape;
        var up: [2]Button = undefined;
        var down: [2]Button = undefined;
        var n_buttons: usize = 0;
        errdefer for (0..n_buttons) |k| {
            down[k].deinit();
            up[k].deinit();
        };
        for ([2]f32{ 272, 361 }, [2]f32{ 310, 398 }, 0..) |uy, dy, k| {
            up[k] = try Button.init(&buttons, x + 313, y + uy, butt_up, butt_up + 1, butt_up + 2);
            errdefer up[k].deinit();
            up[k].inactive = butt_up + 3;
            down[k] = try Button.init(&buttons, x + 313, y + dy, butt_down, butt_down + 1, butt_down + 2);
            down[k].inactive = butt_down + 3;
            n_buttons += 1;
        }
        self.* = .{
            .b = b,
            .ref = ref,
            .x = x,
            .y = y,
            .iface = iface,
            .buttons = buttons,
            .stuff = stuff,
            .units = units,
            .shadow = shadow,
            .ability_pics = ability_pics,
            .spell_pics = spell_pics,
            .skill_pics = skill_pics,
            .medal_pics = medal_pics,
            .class_pics = class_pics,
            .exit = exit,
            .up = up,
            .down = down,
            .box = undefined,
            .exp_pct = exp_pct,
            .old_cursor = c.cursor,
        };
        self.collect(c);
        self.box = try ScrollBox.init(c, &self.buttons, x + 407, y + 73, 303, 324, .{ x + 542, y + 44 }, .{ x + 542, y + 410 }, self.text);
        c.setCursor(0);
        return scene.Scene.of(UnitInfo, self, true);
    }

    pub fn deinit(self: *UnitInfo, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.box.deinit();
        for (&self.up) |*u| u.deinit();
        for (&self.down) |*d| d.deinit();
        self.exit.deinit();
        for ([_]*gfx.Sheet{ &self.iface, &self.buttons, &self.stuff, &self.units, &self.shadow, &self.ability_pics, &self.spell_pics, &self.skill_pics, &self.medal_pics, &self.class_pics }) |s| s.deinit();
    }

    fn fighter(self: *const UnitInfo) *const bt.Fighter {
        return &self.b.units[self.ref.side][self.ref.slot];
    }

    /// The ability row (CheckSkill of every ability_num.var record from
    /// 13 up) and the visible effects, the text.
    fn collect(self: *UnitInfo, c: *Ctx) void {
        const b = self.b;
        const pr = c.proto;
        const f = self.fighter();
        if (f.summoned) self.addAbil(.{ .sheet = .ability, .index = ability_summoned, .name = pr.abilities[ability_summoned].name, .num = 103 });
        var i: i32 = 13;
        while (i < 3000 and self.n_abil < max_icons) : (i += 1) {
            const v = b.skill(f, i);
            if (v <= 0) continue;
            for (pr.abilities[1..], 1..) |a, j| {
                if (a.num != i) continue;
                self.addAbil(.{ .sheet = .ability, .index = j, .number = if (a.numeric) v else 0, .name = a.name, .num = i });
                break;
            }
        }
        for (f.effectList()) |e| {
            if (!e.visible or self.n_eff == max_icons) continue;
            var icon: Icon = .{ .sheet = .none, .number = if (e.time > 0) e.time else 0, .upgrade = e.ability, .spell = e.spell, .time = e.time, .power = e.power };
            if (e.spell > 0 and e.spell < pr.spells.len) {
                icon.sheet = .spell;
                icon.index = @intCast(e.spell - 1);
                icon.name = pr.spells[@intCast(e.spell)].name;
            } else if (e.ability > 0 and e.ability < pr.unit_upgrades.len) {
                icon.sheet = .skill;
                icon.index = @intCast(e.ability);
                icon.name = pr.unit_upgrades[@intCast(e.ability)].name;
            }
            self.eff[self.n_eff] = icon;
            self.n_eff += 1;
        }
        const t = &pr.units[@intCast(f.unit.type)];
        self.text = t.text;
        if (self.hero()) |h| {
            var cur: []const u8 = t.text;
            var k: usize = 0;
            var bufs: [2][1024]u8 = undefined;
            for ([2]i32{ 10, 20 }, [2]i32{ 3, 19 }) |lvl, base| {
                if (h.level < lvl) continue;
                const ci = base + (h.class1 - 1) * 4 + h.class2;
                if (ci < 0 or ci >= pr.classes.len) continue;
                cur = format2(&bufs[k], message(pr, msg_class), cur, pr.classes[@intCast(ci)].text);
                k = 1 - k;
            }
            const n = @min(cur.len, self.text_buf.len);
            @memcpy(self.text_buf[0..n], cur[0..n]);
            self.text = self.text_buf[0..n];
        }
    }

    fn addAbil(self: *UnitInfo, icon: Icon) void {
        if (self.n_abil == max_icons) return;
        self.abil[self.n_abil] = icon;
        self.n_abil += 1;
    }

    fn hero(self: *const UnitInfo) ?*const @import("../game/hero.zig").Hero {
        const f = self.fighter();
        if (!f.hero_unit) return null;
        return self.b.armies[f.home].hero;
    }

    fn message(pr: anytype, i: usize) []const u8 {
        return if (i < pr.messages.len) pr.messages[i] else "%s %s";
    }

    /// sprintf(out, fmt, a, b) for a format with two "%s".
    fn format2(out: []u8, fmt: []const u8, a: []const u8, b2: []const u8) []const u8 {
        var n: usize = 0;
        var k: usize = 0;
        var arg: usize = 0;
        while (k < fmt.len and n < out.len) {
            if (k + 1 < fmt.len and fmt[k] == '%' and fmt[k + 1] == 's') {
                const s = if (arg == 0) a else if (arg == 1) b2 else "";
                arg += 1;
                const m = @min(s.len, out.len - n);
                @memcpy(out[n .. n + m], s[0..m]);
                n += m;
                k += 2;
                continue;
            }
            out[n] = fmt[k];
            n += 1;
            k += 1;
        }
        return out[0..n];
    }

    fn rows(self: *const UnitInfo, r: usize) []const Icon {
        return if (r == 0) self.abil[0..self.n_abil] else self.eff[0..self.n_eff];
    }

    pub fn update(self: *UnitInfo, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        if (self.exit.check(f)) return .pop;
        self.box.update(f);
        for (0..2) |r| {
            const n = self.rows(r).len;
            self.up[r].on = self.line[r] > 0;
            self.down[r].on = (self.line[r] + 1) * 5 < n;
            if (self.up[r].check(f)) self.line[r] -= 1;
            if (self.down[r].check(f)) self.line[r] += 1;
        }
        self.hover = null;
        const p = f.pointer;
        const click = p.primary.released or p.secondary.released;
        const x = self.x;
        const y = self.y;
        for (0..2) |r| {
            const icons = self.rows(r);
            const iy = y + (if (r == 0) @as(f32, 275) else 372);
            for (0..5) |k| {
                const idx = self.line[r] * 5 + k;
                if (idx >= icons.len) break;
                const ix = x + 11 + 60 * @as(f32, @floatFromInt(k));
                if (!p.in(ix, iy, 64, 64)) continue;
                self.hover = .{ r, k };
                const icon = icons[idx];
                if (r == 0) {
                    hero_ui.abilityHint(c, &self.hint, icon.num, p.pos);
                } else if (icon.name.len > 0) {
                    self.hint.what = .{ .string = icon.name };
                    self.hint.pos = p.pos;
                }
                if (click) return .{ .push = (if (r == 0)
                    info_windows.ability(c, icon.num, self.b.skill(self.fighter(), icon.num))
                else
                    info_windows.effect(c, icon.upgrade, icon.spell, icon.time, icon.power)) catch return .none };
            }
        }
        const fu = self.fighter();
        if (self.hero() == null) for (fu.unit.medals, 0..) |md, i| {
            if (md > 0 and click and p.in(x + 317, y + 85 + 60 * @as(f32, @floatFromInt(i)), 64, 64)) {
                return .{ .push = info_windows.medal(c, md) catch return .none };
            }
        };
        for (grid, 0..) |row, r| for (row, 0..) |id, k| {
            if (!p.in(x + 18 + 59 * @as(f32, @floatFromInt(k)), y + 45 + 31 * @as(f32, @floatFromInt(r)), 58, 30)) continue;
            hero_ui.abilityHint(c, &self.hint, id, p.pos);
            if (click) return info_windows.abilityAction(c, id, self.param(id));
        };
        const now = [5]i32{ fu.life, fu.stamina, fu.morale, fu.speed, fu.ammo };
        for (bars, now) |bar_, v| {
            if (!p.in(x + bar_.x, y + bar_.y, bar_.w, 20)) continue;
            self.hint.phrase(c, bar_.hint, p.pos);
            if (click) return info_windows.abilityAction(c, bar_.num, v);
        }
        return .none;
    }

    /// arParam: GetLife .. GetMorale; the ammo is iAmmo (BF_Unit.cpp:4850).
    fn param(self: *const UnitInfo, id: i32) i32 {
        const b = self.b;
        const f = self.fighter();
        return switch (id) {
            1 => b.maxLife(f),
            2 => b.attack(f),
            3 => b.counterAttack(f),
            4 => b.defence(f),
            5 => b.rangedDefence(f),
            6 => b.resist(f),
            7 => b.maxSpeed(f),
            8 => b.rangedAttack(f),
            9 => b.shootingRange(f),
            10 => f.ammo,
            11 => b.maxStamina(f),
            12 => b.maxMorale(f),
            else => 0,
        };
    }

    fn iconTex(self: *UnitInfo, icon: Icon) ?rl.Texture2D {
        return switch (icon.sheet) {
            .ability => self.ability_pics.texIndex(icon.index),
            .spell => self.spell_pics.texIndex(icon.index),
            .skill => self.skill_pics.texIndex(icon.index),
            .none => null,
        };
    }

    /// ShowIconNumber.
    fn iconNumber(self: *UnitInfo, c: *Ctx, x: f32, y: f32, n: i32) void {
        var buf: [16]u8 = undefined;
        const s = std.fmt.bufPrint(&buf, "{d}", .{n}) catch "";
        if (n < 10) {
            gfx.drawTex(self.stuff.texIndex(stuff_count1), x + 37, y + 32);
            c.font1.draw(s, x + 40, y + 36, color1);
        } else {
            gfx.drawTex(self.stuff.texIndex(stuff_count2), x + 29, y + 33);
            c.font1.drawCentered(s, x + 39, y + 37, color1);
        }
    }

    /// A 1-pixel-column bar of `limit` (0..max) columns from (bx, by).
    fn bar(bx: f32, by: f32, limit: i32, comptime col: fn (i32) rl.Color) void {
        var i: i32 = 0;
        while (i < limit) : (i += 1) {
            rl.drawRectangle(@intFromFloat(bx + @as(f32, @floatFromInt(i))), @intFromFloat(by), 2, 19, col(i));
        }
    }

    fn pct(cur: i32, max: i32, width: i32) i32 {
        if (max <= 0) return 0;
        return std.math.clamp(@divTrunc(cur * width, max), 0, width);
    }

    fn u8c(v: i32) u8 {
        return @intCast(std.math.clamp(v, 0, 255));
    }

    fn shadowed(c: *Ctx, s: []const u8, x: f32, y: f32) void {
        c.font1.drawCentered(s, x + 1, y + 1, color1);
        c.font1.drawCentered(s, x, y, color2);
    }

    pub fn draw(self: *UnitInfo, c: *Ctx) void {
        const b = self.b;
        const pr = c.proto;
        const f = self.fighter();
        const x = self.x;
        const y = self.y;
        var buf: [128]u8 = undefined;
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 255 - 190 });
        gfx.drawTex(self.iface.texIndex(gui_window), x, y);
        if (@import("../data/proto.zig").UnitType.picIndex(@intCast(f.unit.type))) |k| {
            const pic = self.units.texIndex(k);
            const px = x + 254 - gfx.texW(pic) / 2;
            const py = y + 100 - gfx.texH(pic) / 2;
            gfx.drawTex2(self.shadow.texIndex(k), px, py, .{ .r = 255, .g = 255, .b = 255, .a = 140 });
            gfx.drawTex(pic, px, py);
        }
        // The medals, or the classes of a hero.
        for (f.unit.medals, 0..) |m, i| {
            if (m > 0) gfx.drawTex(self.medal_pics.texIndex(@intCast(m - 1)), x + 323, y + 91 + 60 * @as(f32, @floatFromInt(i)));
        }
        const t = &pr.units[@intCast(f.unit.type)];
        if (self.hero()) |h| {
            const c1 = h.class1 - 1;
            gfx.drawTex(self.class_pics.texIndex(@intCast(@max(c1, 0))), x + 323, y + 91);
            if (h.level >= 10) gfx.drawTex(self.class_pics.texIndex(@intCast(@max(3 + c1 * 4 + h.class2, 0))), x + 323, y + 151);
            if (h.level >= 20) gfx.drawTex(self.class_pics.texIndex(@intCast(@max(19 + c1 * 4 + h.class2, 0))), x + 323, y + 211);
            c.font1.drawCentered(std.fmt.bufPrint(&buf, "{s} {s}", .{ h.nameText(pr), pr.classes[h.className()].name }) catch "", x + 370, y + 17, color1);
            const e0 = h.expForLevel(pr, h.level, self.exp_pct);
            const e1 = h.expForLevel(pr, h.level + 1, self.exp_pct);
            bar(x + 319, y + 65, pct(h.exp - e0, e1 - e0, 58), expCol);
            self.level(c, h.level);
        } else {
            const karma: i32 = std.math.clamp(t.karma, -3, 3);
            c.font1.drawCentered(std.fmt.bufPrint(&buf, "{s} ({s})", .{ t.name, pr.phrase(@intCast(ph_karma + karma)) }) catch "", x + 370, y + 17, color1);
            const e0 = unit_mod.expForLevel(pr, f.unit.type, f.unit.level);
            const e1 = unit_mod.expForLevel(pr, f.unit.type, f.unit.level + 1);
            bar(x + 319, y + 65, pct(f.unit.exp - e0, e1 - e0, 58), expCol);
            self.level(c, f.unit.level);
        }
        self.box.draw(c, &self.buttons, self.text);

        // The bars.
        const h1: f32 = 24;
        bar(x + 113, y + 176, pct(f.life, b.maxLife(f), 100), lifeCol);
        shadowed(c, num(&buf, f.life), x + 167, y + 180);
        bar(x + 52, y + 176 + h1, pct(f.stamina, b.maxStamina(f), 100), stamCol);
        shadowed(c, num(&buf, f.stamina), x + 105, y + 180 + h1);
        bar(x + 52, y + 176 + h1 * 2, pct(f.morale, b.maxMorale(f), 100), moraleCol);
        shadowed(c, num(&buf, f.morale), x + 105, y + 180 + h1 * 2);
        if (b.maxSpeed(f) > 0) bar(x + 194, y + 176 + h1, pct(f.speed, b.maxSpeed(f), 100), speedCol);
        shadowed(c, num(&buf, f.speed), x + 247, y + 180 + h1);
        if (b.maxAmmo(f) > 0) bar(x + 194, y + 176 + h1 * 2, pct(f.ammo, b.maxAmmo(f), 100), ammoCol);
        shadowed(c, num(&buf, f.ammo), x + 247, y + 180 + h1 * 2);

        // The stats.
        const stats = [4][3]i32{
            .{ b.maxLife(f), b.maxStamina(f), b.maxMorale(f) },
            .{ b.attack(f), b.counterAttack(f), b.maxSpeed(f) },
            .{ b.defence(f), b.rangedDefence(f), b.resist(f) },
            .{ b.rangedAttack(f), b.shootingRange(f), b.maxAmmo(f) },
        };
        for (stats, 0..) |row, r| for (row, 0..) |v, k| {
            c.font1.drawCentered(num(&buf, v), x + 60 + 59 * @as(f32, @floatFromInt(k)), y + 56 + 31 * @as(f32, @floatFromInt(r)), yellow);
        };

        // The abilities and the effects.
        c.font1.drawCentered(pr.phrase(ph_abilities), x + 169, y + 256, color1);
        c.font1.drawCentered(pr.phrase(ph_effects), x + 169, y + 354, color1);
        for (0..2) |r| {
            const icons = self.rows(r);
            const iy = y + (if (r == 0) @as(f32, 281) else 378);
            for (0..5) |k| {
                const idx = self.line[r] * 5 + k;
                if (idx >= icons.len) break;
                const ix = x + 17 + 60 * @as(f32, @floatFromInt(k));
                gfx.drawTex(self.iconTex(icons[idx]), ix, iy);
                if (icons[idx].number > 0) self.iconNumber(c, ix, iy, icons[idx].number);
                // UI_Mask shows only its Active picture, under the pointer.
                const sel = if (self.hover) |hv| hv[0] == r and hv[1] == k else false;
                if (sel) gfx.drawTex(self.iface.texIndex(gui_select), ix - 6, iy - 6);
            }
            self.up[r].draw(&self.buttons, c.tick);
            self.down[r].draw(&self.buttons, c.tick);
        }
        self.exit.draw(&self.buttons, c.tick);
        self.hint.draw(c, &self.iface);
    }

    fn level(self: *UnitInfo, c: *Ctx, lvl: i32) void {
        var buf: [16]u8 = undefined;
        c.font1.drawCentered(num(&buf, lvl), self.x + 348, self.y + 71, color1);
        c.font1.drawCentered(num(&buf, lvl), self.x + 347, self.y + 70, yellow);
    }

    fn num(buf: []u8, v: i32) []const u8 {
        return std.fmt.bufPrint(buf, "{d}", .{v}) catch "";
    }

    fn expCol(i: i32) rl.Color {
        return .{ .r = u8c(40 + i * 2), .g = u8c(10 + @divTrunc(i, 2)), .b = u8c(50 + @divTrunc(i * 5, 2)), .a = 255 };
    }
    fn lifeCol(i: i32) rl.Color {
        return .{ .r = u8c(50 + i * 2), .g = u8c(10 + @divTrunc(i, 2)), .b = u8c(10 + @divTrunc(i, 2)), .a = 255 };
    }
    fn stamCol(i: i32) rl.Color {
        return .{ .r = u8c(50 + i * 2), .g = u8c(50 + i * 2), .b = u8c(10 + @divTrunc(i, 2)), .a = 255 };
    }
    fn moraleCol(i: i32) rl.Color {
        return .{ .r = u8c(10 + @divTrunc(i, 2)), .g = u8c(10 + @divTrunc(i, 2)), .b = u8c(50 + i * 2), .a = 255 };
    }
    fn speedCol(i: i32) rl.Color {
        return .{ .r = u8c(5 + i * 2), .g = u8c(i * 2), .b = u8c(10 + i * 2), .a = 255 };
    }
    fn ammoCol(i: i32) rl.Color {
        return .{ .r = u8c(10 + @divTrunc(i, 2)), .g = u8c(50 + i * 2), .b = u8c(10 + @divTrunc(i, 2)), .a = 255 };
    }
};

test "format with two strings" {
    var buf: [64]u8 = undefined;
    try std.testing.expectEqualStrings("a, b.", UnitInfo.format2(&buf, "%s, %s.", "a", "b"));
}
