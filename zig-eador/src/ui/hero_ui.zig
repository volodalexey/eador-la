//! Blocks of the hero screens (Screens.cpp): GUI_InfoBlock (:1076),
//! GUI_StatBlock (:733) and GUI_ItemBlock (:208), shared by ScreenHero
//! and ScreenHeroShop.
//!
//! A click on a stat, the level or the life returns the
//! WindowAbilityInfo to open (`AbilityClick`); `portraitClick` requests
//! WindowUnitInfo of the hero's own unit.
const std = @import("std");
const rl = @import("raylib");
const Ctx = @import("../app.zig").Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Hint = @import("hint.zig").Hint;
const Mask = @import("mask.zig").Mask;
const castle_ui = @import("castle_ui.zig");
const Sheets = castle_ui.Sheets;
const gui = castle_ui.gui;
const proto_mod = @import("../data/proto.zig");
const hero_mod = @import("../game/hero.zig");
const Hero = hero_mod.Hero;
const equip = @import("../game/equip.zig");

const color1 = castle_ui.color1;
const color2 = castle_ui.color2;

/// interface.dat 83HeroPort, 117SpellGlow, 25SpellClosed.
const gui_hero_port = 83;
const gui_spell_glow = 117;
const gui_spell_closed = 25;

const spellbook = @import("../game/spellbook.zig");

/// GUI_SpellBlock (Screens.cpp:577): the hero's book, circle 1 at the
/// bottom row, circle 4 at the top; closed slots show 25SpellClosed, open
/// ones glow when the spell in the hand fits them.
pub const SpellBlock = struct {
    x: f32,
    y: f32,
    shape: rl.Image,
    masks: [spellbook.size + 1]Mask = undefined,

    const w = 60;
    const h = 60;

    /// The picture of slot `i` relative to the block.
    pub fn pos(i: usize) [2]f32 {
        const r: usize = @intCast(spellbook.slotLevel(@intCast(i)) - 1);
        return .{ @as(f32, @floatFromInt(i - spellbook.lvl[r])) * w, @as(f32, @floatFromInt(3 - r)) * h };
    }

    pub fn init(sh: *Sheets, x: f32, y: f32) !SpellBlock {
        var b: SpellBlock = .{ .x = x, .y = y, .shape = sh.iface.imageIndex(gui.spell_mask) orelse return error.MissingMask };
        for (b.masks[1..], 1..) |*m, i| {
            const at = pos(i);
            m.* = .{ .x = x + at[0] - 6, .y = y + at[1] - 6 };
        }
        b.masks[0] = .{ .x = -1000, .y = -1000 };
        return b;
    }

    pub fn deinit(self: *SpellBlock) void {
        rl.unloadImage(self.shape);
    }

    pub const Check = struct { click: ?usize = null, info: ?i32 = null };

    /// GUI_SpellBlock::Check: the hint of the spell under the pointer, the
    /// clicked open slot, the spell of a right click (WindowSpellInfo).
    pub fn check(self: *SpellBlock, c: *Ctx, hero: *const Hero, p: input.Pointer, hint: *Hint) Check {
        var r: Check = .{};
        for (self.masks[1..], 1..) |*m, i| {
            if (!spellbook.open(c.proto, hero, i)) {
                m.state = .visible;
                continue;
            }
            m.shape = &self.shape;
            const res = m.check(p);
            const sp = hero.spell(i);
            const known = sp > 0 and sp < c.proto.spells.len;
            if (m.hot() and known) {
                hint.what = .{ .string = c.proto.spells[@intCast(sp)].name };
                hint.pos = p.pos;
            }
            switch (res) {
                .click => r.click = i,
                .right_click => if (known) {
                    r.info = sp;
                },
                .none => {},
            }
        }
        return r;
    }

    /// `held`: the spell in the hand (or of the scroll), 0 for none.
    pub fn draw(self: *const SpellBlock, c: *Ctx, sh: *Sheets, hero: *const Hero, held: i32) void {
        const lv = spellbook.level(c.proto, held);
        for (self.masks[1..], 1..) |m, i| {
            const at = pos(i);
            const x = self.x + at[0];
            const y = self.y + at[1];
            if (!spellbook.open(c.proto, hero, i)) {
                gfx.drawTex(sh.iface.texIndex(gui_spell_closed), x, y);
                continue;
            }
            gfx.drawTex(sh.spell(hero.spell(i)), x, y);
            if (held > 0 and lv <= spellbook.slotLevel(@intCast(i))) gfx.drawTex2(sh.iface.texIndex(gui_spell_glow), m.x, m.y, .{ .r = 255, .g = 255, .b = 255, .a = 200 });
            if (m.hot()) gfx.drawTex(sh.iface.texIndex(gui.spell_select), m.x, m.y);
        }
    }
};

/// WindowAbilityInfo(num, power) of a click on a field.
pub const AbilityClick = struct { num: i32, power: i32 };

fn released(p: input.Pointer) bool {
    return p.primary.released or p.secondary.released;
}

/// MakeAbilityHint: the ability_num.var name of `num`.
pub fn abilityHint(c: *Ctx, hint: *Hint, num: i32, at: rl.Vector2) void {
    for (c.proto.abilities[1..]) |a| if (a.num == num) {
        hint.what = .{ .string = a.name };
        hint.pos = at;
        return;
    };
}

/// The pictures the hero screens need besides the castle sheets.
pub const HeroSheets = struct {
    portraits: gfx.Sheet,
    classes: gfx.Sheet,
    skills: gfx.Sheet,

    pub fn load(c: *Ctx) !HeroSheets {
        var p = try gfx.Sheet.load(c.fs, "portraits");
        errdefer p.deinit();
        var cl = try gfx.Sheet.load(c.fs, "hero_class");
        errdefer cl.deinit();
        const sk = try gfx.Sheet.load(c.fs, "heroskills");
        return .{ .portraits = p, .classes = cl, .skills = sk };
    }

    pub fn deinit(self: *HeroSheets) void {
        self.skills.deinit();
        self.classes.deinit();
        self.portraits.deinit();
    }
};

/// GUI_InfoBlock: the portrait, name, class, leadership/magic/health,
/// initiative/siege/mobility and the life and experience bars.  It also
/// keeps the life share while the maximum changes with the items (Show).
pub const InfoBlock = struct {
    x: f32,
    y: f32,
    health: f32,
    old_life: i32,
    old_max: i32,
    /// GetSiege and GetMobility need the castle (GetBldParam1 42 and 37).
    siege_bonus: i32,
    mobility_bonus: i32,
    exp_pct: i32,

    const w = 60;

    pub fn init(c: *Ctx, h: *const Hero, x: f32, y: f32, siege_bonus: i32, mobility_bonus: i32, exp_pct: i32) InfoBlock {
        const l = h.life(c.proto);
        return .{
            .x = x,
            .y = y,
            .health = @as(f32, @floatFromInt(l[0])) / @as(f32, @floatFromInt(@max(l[1], 1))),
            .old_life = l[0],
            .old_max = l[1],
            .siege_bonus = siege_bonus,
            .mobility_bonus = mobility_bonus,
            .exp_pct = exp_pct,
        };
    }

    /// The start of Show: the hero's life follows a new maximum.
    pub fn refresh(self: *const InfoBlock, c: *Ctx, h: *Hero) void {
        if (!h.alive()) return;
        const u = &(h.squad[0] orelse return);
        const max = h.life(c.proto)[1];
        if (max == self.old_max) {
            u.life = self.old_life;
        } else {
            u.life = @intFromFloat(@as(f32, @floatFromInt(max)) * self.health);
            if (u.life <= 0) u.life = 1;
        }
    }

    pub fn update(self: *const InfoBlock, c: *Ctx, h: *const Hero, p: input.Pointer, hint: *Hint) ?AbilityClick {
        const pr = c.proto;
        const params = [_]i32{ 116, 117, 118, hero_mod.bonus.initiative, 55, hero_mod.bonus.mobility };
        const vals = [_]i32{ h.leader, h.magic, h.health, h.heroBonus(pr, hero_mod.bonus.initiative), h.siegePower(pr, self.siege_bonus), h.mobility(pr, self.mobility_bonus) };
        var r: ?AbilityClick = null;
        for (params, vals, 0..) |pm, v, i| {
            const fi: f32 = @floatFromInt(i);
            if (p.in(20 + self.x + @mod(fi, 3) * w, 211 + self.y + @floor(fi / 3) * 30, w - 1, 29)) {
                abilityHint(c, hint, pm, p.pos);
                if (released(p)) r = .{ .num = pm, .power = v };
            }
        }
        if (p.in(self.x + 23, self.y + 12, 18, 150)) {
            abilityHint(c, hint, 119, p.pos);
            if (released(p)) r = .{ .num = 119, .power = h.level };
        }
        if (p.in(self.x + 175, self.y + 12, 18, 150)) {
            abilityHint(c, hint, 1, p.pos);
            if (released(p)) r = .{ .num = 1, .power = h.life(pr)[0] };
        }
        return r;
    }

    /// GUI_InfoBlock::mskHero: either button opens the hero's unit window.
    pub fn portraitClick(self: *const InfoBlock, hs: *HeroSheets, h: *const Hero, p: input.Pointer) bool {
        const pic = hs.portraits.texIndex(@intCast(@max(h.portrait, 0)));
        return released(p) and p.in(self.x + 38, self.y + 10, gfx.texW(pic), gfx.texH(pic));
    }

    pub fn draw(self: *const InfoBlock, c: *Ctx, sh: *Sheets, hs: *HeroSheets, h: *const Hero) void {
        const pr = c.proto;
        const x = self.x;
        const y = self.y;
        gfx.drawTex(hs.portraits.texIndex(@intCast(@max(h.portrait, 0))), x + 43, y + 12);
        gfx.drawTex(sh.iface.texIndex(gui_hero_port), x + 42, y + 116);
        c.font1.drawCentered(h.nameText(pr), x + 109, y + 169, color1);
        c.font1.drawCentered(pr.classes[h.className()].name, x + 109, y + 193, color1);
        var b: [16]u8 = undefined;
        const vals = [_]i32{ h.leader, h.magic, h.health, h.heroBonus(pr, hero_mod.bonus.initiative), h.siegePower(pr, self.siege_bonus), h.mobility(pr, self.mobility_bonus) };
        for (vals, 0..) |v, i| {
            const fi: f32 = @floatFromInt(i);
            c.font1.drawCentered(castle_ui.fmtInt(&b, v), x + 63 + @mod(fi, 3) * w, y + 222 + @floor(fi / 3) * 30, color2);
        }
        const l = h.life(pr);
        const lbar: i32 = @intFromFloat(@ceil(@as(f32, @floatFromInt(l[0])) / @as(f32, @floatFromInt(@max(l[1], 1))) * 100));
        var i: i32 = 0;
        while (i < @min(lbar, 100)) : (i += 1) {
            rl.drawRectangle(@intFromFloat(x + 179), @intFromFloat(y + 111 - @as(f32, @floatFromInt(i))), 10, 1, .{ .r = @intCast(100 + i), .g = @intCast(@divTrunc(i, 2)), .b = @intCast(@divTrunc(i, 2)), .a = 255 });
        }
        const e0 = h.expForLevel(pr, h.level, self.exp_pct);
        const e1 = h.expForLevel(pr, h.level + 1, self.exp_pct);
        const ebar = std.math.clamp(@divTrunc(100 * (h.exp - e0), @max(e1 - e0, 1)), 0, 100);
        i = 0;
        while (i < ebar) : (i += 1) {
            rl.drawRectangle(@intFromFloat(x + 28), @intFromFloat(y + 111 - @as(f32, @floatFromInt(i))), 10, 1, .{ .r = @intCast(100 + i), .g = @intCast(@divTrunc(i, 2)), .b = @intCast(120 + i), .a = 255 });
        }
        c.font1.drawCentered(castle_ui.fmtInt(&b, h.level), x + 35, y + 146, color2);
        c.font1.drawCentered(castle_ui.fmtInt(&b, l[0]), x + 180, y + 146, color2);
    }
};

/// GUI_StatBlock: the twelve stats of the hero's unit.
pub const StatBlock = struct {
    x: f32,
    y: f32,

    const dx = 53;
    const dy = 10;
    const w = 60;
    const h = 31;
    /// Life, stamina, morale; attack, counterattack, speed; defence,
    /// ranged defence, resistance; ranged attack, range, ammo.
    const params = [12]i32{ 1, 11, 12, 2, 3, 7, 4, 5, 6, 8, 9, 10 };

    pub fn update(self: *const StatBlock, c: *Ctx, hero: *const Hero, p: input.Pointer, hint: *Hint) ?AbilityClick {
        var r: ?AbilityClick = null;
        for (params, 0..) |pm, i| {
            const fi: f32 = @floatFromInt(i);
            if (p.in(10 + self.x + @mod(fi, 3) * w, self.y + @floor(fi / 3) * h, w - 1, h - 1)) {
                abilityHint(c, hint, pm, p.pos);
                if (released(p)) r = .{ .num = pm, .power = value(c, hero, pm) };
            }
        }
        return r;
    }

    /// GetLife .. GetAmmo of the hero's unit, as drawn.
    fn value(c: *Ctx, hero: *const Hero, pm: i32) i32 {
        return if (pm == 1) hero.life(c.proto)[1] else hero.unitStat(c.proto, pm);
    }

    pub fn draw(self: *const StatBlock, c: *Ctx, hero: *const Hero) void {
        var b: [16]u8 = undefined;
        for (params, 0..) |pm, i| {
            const fi: f32 = @floatFromInt(i);
            const v = value(c, hero, pm);
            c.font1.drawCentered(castle_ui.fmtInt(&b, v), self.x + dx + @mod(fi, 3) * w, self.y + dy + @floor(fi / 3) * h, color2);
        }
    }
};

/// GUI_ItemBlock: the doll (places 1..15), the backpack (16..25) and the
/// doll's picture as place 0 (a click there puts the item where it fits).
pub const ItemBlock = struct {
    x: f32,
    y: f32,
    shape: rl.Image,
    masks: [hero_mod.inventory_size + 1]Mask = undefined,

    const w = 60;
    const h = 59;

    /// The picture of place `k` relative to the block.
    pub fn pos(k: usize) [2]f32 {
        return switch (k) {
            1 => .{ 0, 0 },
            2 => .{ w * 4, 0 },
            3 => .{ 0, h * 2 },
            4 => .{ 0, h },
            5 => .{ 0, h * 3 },
            6 => .{ w * 4, h * 2 },
            7 => .{ 0, h * 5 },
            8 => .{ w * 4, h * 3 },
            9 => .{ w * 4, h },
            10 => .{ 0, h * 4 },
            11 => .{ w * 4, h * 4 },
            12...15 => .{ w * @as(f32, @floatFromInt(k - 11)), h * 5 },
            16...20 => .{ w * @as(f32, @floatFromInt(k - 16)), 359 },
            21...25 => .{ w * @as(f32, @floatFromInt(k - 21)), 419 },
            else => .{ 0, 0 },
        };
    }

    pub fn init(sh: *Sheets, x: f32, y: f32) !ItemBlock {
        var b: ItemBlock = .{ .x = x, .y = y, .shape = sh.iface.imageIndex(gui.spell_mask) orelse return error.MissingMask };
        b.masks[0] = Mask.rect(x + 59, y, 176, 279);
        for (b.masks[1..], 1..) |*m, k| {
            const at = pos(k);
            m.* = .{ .x = x + at[0] - 6, .y = y + at[1] - 6 };
        }
        return b;
    }

    pub fn deinit(self: *ItemBlock) void {
        rl.unloadImage(self.shape);
    }

    pub const Check = struct { under: ?usize = null, click: ?usize = null, info: ?usize = null };

    pub fn check(self: *ItemBlock, p: input.Pointer) Check {
        var r: Check = .{};
        for (&self.masks, 0..) |*m, k| {
            if (k > 0) m.shape = &self.shape;
            const res = m.check(p);
            if (m.hot()) r.under = k;
            switch (res) {
                .click => r.click = k,
                .right_click => r.info = k,
                .none => {},
            }
        }
        return r;
    }

    pub fn draw(self: *const ItemBlock, c: *Ctx, sh: *Sheets, hero: *const Hero, held: i32) void {
        for (1..hero_mod.inventory_size + 1) |k| {
            const it = hero.inventory[k];
            const at = pos(k);
            if (it[0] > 0) {
                gfx.drawTex(sh.item(c.proto, it[0]), self.x + at[0], self.y + at[1]);
                if (it[1] == 0) gfx.drawTex(sh.iface.texIndex(gui.spell_shadow), self.x + at[0], self.y + at[1]);
            }
        }
        for (self.masks[1..], 1..) |m, k| {
            if (k <= hero_mod.worn_slots and equip.glows(c.proto, hero, held, k)) gfx.drawTex2(sh.iface.texIndex(gui_spell_glow), m.x, m.y, .{ .r = 255, .g = 255, .b = 255, .a = 200 });
            if (m.hot()) gfx.drawTex(sh.iface.texIndex(gui.spell_select), m.x, m.y);
        }
    }
};
