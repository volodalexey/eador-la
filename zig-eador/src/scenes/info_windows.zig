//! The info windows of Windower.cpp that drill down from other windows:
//!
//! - WindowAbilityInfo (:1751), WindowMedalInfo (:1815), WindowRaceInfo
//!   (:2255), WindowEffectInfo (:2308) and WindowSkillInfo (:1695): a
//!   WindowInfo layout with the record's picture, name and text;
//! - WindowDefenderInfo (:1868), WindowOuterBldInfo (:1990) and
//!   WindowRitualInfo (:2091): the price, the upkeep or cooldown and the
//!   resources; a click on a resource opens WindowResourceInfo.
//!
//! WindowResourceInfo shows the current player's count and prices
//! (CurrentPlayer: the `market` passed in), or the base prices without one.
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
const proto_mod = @import("../data/proto.zig");
const wi = @import("window_info.zig");
const WindowInfo = wi.WindowInfo;
const ResourceWindow = @import("army_info.zig").ResourceWindow;
const player = @import("../game/player.zig");

/// Phrases: "Уровень", "Длительность", "постоянно".
const ph_level = 5;
const ph_duration = 86;
const ph_forever = 87;

/// unit_upg.var types whose effect window shows the effect's own power
/// (poisons 41 and 78, 25).
const own_power = [_]i32{ 41, 78, 25 };

const color1 = castle_ui.color1;
const color2 = castle_ui.color2;

/// WindowAbilityInfo(num, power): the ability_num.var record `num`; a
/// numeric or percent text gets the power for its "%d".
pub fn ability(c: *Ctx, num: i32, power: i32) !scene.Scene {
    const pr = c.proto;
    var found: ?usize = null;
    for (pr.abilities, 0..) |a, j| if (a.num == num) {
        found = j;
    };
    const j = found orelse return error.NoAbility;
    const a = &pr.abilities[j];
    var buf: [4096]u8 = undefined;
    const body = if (a.numeric or a.percent) castle_ui.formatInt(&buf, a.text, power) else a.text;
    return WindowInfo.open(c, wi.ability_layout, a.name, "", body, "ability", j);
}

/// The action of a click that opens WindowAbilityInfo.
pub fn abilityAction(c: *Ctx, num: i32, power: i32) scene.Action {
    return .{ .push = ability(c, num, power) catch return .none };
}

/// WindowMedalInfo: medal.dat object m-1.
pub fn medal(c: *Ctx, m: i32) !scene.Scene {
    const pr = c.proto;
    if (m <= 0 or m >= pr.medals.len) return error.NoMedal;
    const md = &pr.medals[@intCast(m)];
    return WindowInfo.open(c, wi.ability_layout, md.name, "", md.text, "medal", @intCast(m - 1));
}

/// WindowRaceInfo: races.dat object race-1.
pub fn race(c: *Ctx, r: i32) !scene.Scene {
    const pr = c.proto;
    if (r < 0 or r >= pr.races.len) return error.NoRace;
    const rc = &pr.races[@intCast(r)];
    return WindowInfo.open(c, wi.ability_layout, rc.name, "", rc.text, "races", @intCast(@max(r - 1, 0)));
}

/// WindowEffectInfo for a battle effect: `upgrade` is pAbility (a
/// unit_upg.var record, skills.dat picture), else `spell` is pSpell.  The
/// effects.var record is the last one of that ability type or spell.  A
/// numeric text gets |power| for the poisons (the callers' rule in
/// BF_Unit.cpp:5121), else the upgrade's first value.
pub fn effect(c: *Ctx, upgrade: i32, spell: i32, time: i32, power: i32) !scene.Scene {
    const pr = c.proto;
    var num: usize = 0;
    var sheet: []const u8 = "spells";
    var pic: usize = 0;
    var upg_value: i32 = 0;
    var pw: i32 = 0;
    if (upgrade > 0 and upgrade < pr.unit_upgrades.len) {
        const u = &pr.unit_upgrades[@intCast(upgrade)];
        sheet = "skills";
        pic = @intCast(upgrade);
        upg_value = u.upg[0].value;
        if (std.mem.indexOfScalar(i32, &own_power, u.upg[0].id) != null) pw = power;
        for (pr.effects[1..], 1..) |e, i| if (e.ability == u.upg[0].id) {
            num = i;
        };
    } else if (spell > 0 and spell < pr.spells.len) {
        pic = @intCast(spell - 1);
        for (pr.effects[1..], 1..) |e, i| if (e.spell == spell) {
            num = i;
        };
    } else return error.NoEffect;
    if (num >= pr.effects.len) return error.NoEffect;
    const e = &pr.effects[num];
    var sub_buf: [128]u8 = undefined;
    const sub = if (time > 0)
        std.fmt.bufPrint(&sub_buf, "{s}: {d}", .{ pr.phrase(ph_duration), time }) catch ""
    else
        std.fmt.bufPrint(&sub_buf, "{s}: {s}", .{ pr.phrase(ph_duration), pr.phrase(ph_forever) }) catch "";
    var buf: [4096]u8 = undefined;
    const body = if (e.numeric) castle_ui.formatInt(&buf, e.text, if (pw != 0) (if (pw < 0) -pw else pw) else upg_value) else e.text;
    return WindowInfo.open(c, wi.effect_layout, e.name, sub, body, sheet, pic);
}

/// WindowSkillInfo(skill, level): heroskills.dat object (skill-1)*5+level.
pub fn skill(c: *Ctx, s: i32, level: i32) !scene.Scene {
    const pr = c.proto;
    if (s <= 0 or s >= pr.skills.len) return error.NoSkill;
    const sk = &pr.skills[@intCast(s)];
    var sub_buf: [128]u8 = undefined;
    const sub = std.fmt.bufPrint(&sub_buf, "{s}: {d}", .{ pr.phrase(ph_level), level }) catch "";
    return WindowInfo.open(c, wi.skill_layout, sk.name, sub, sk.text, "heroskills", @intCast(@max((s - 1) * 5 + level, 0)));
}

/// The window of a defender, an outer building or a ritual.
pub const Kind = enum { defender, outer, ritual };

/// interface.dat 19DefenderInfo and 56OuterBuildInfo (also the ritual's).
const gui_defender_info = 19;
const gui_outer_info = 56;
/// stuff.dat 35: the cooldown plate of the ritual.
const stuff_cooldown = 35;
const butt_exit = 226;

const max_res = 6;

/// A field with a phrase hint (UI_Mask1 + MakeStringHint).
const Field = struct { x: f32, y: f32, w: f32, h: f32, hint: u32 };
const cost_fields = [_]Field{
    .{ .x = 27, .y = 83, .w = 89, .h = 23, .hint = 307 },
    .{ .x = 121, .y = 83, .w = 89, .h = 23, .hint = 308 },
};
const pay_fields = [_]Field{
    .{ .x = 110, .y = 47, .w = 89, .h = 23, .hint = 90 },
    .{ .x = 206, .y = 47, .w = 89, .h = 23, .hint = 91 },
};
const cooldown_field: Field = .{ .x = 303, .y = 83, .w = 75, .h = 23, .hint = 394 };

pub const CostWindow = struct {
    kind: Kind,
    id: i32,
    name: []const u8,
    body: []const u8,
    gold_cost: i32,
    gem_cost: i32,
    /// The defender's upkeep.
    gold_pay: i32 = 0,
    gem_pay: i32 = 0,
    cooldown: i32 = 0,
    res: [max_res]i32 = @splat(0),
    /// CurrentPlayer for WindowResourceInfo.
    market: ?player.Market,
    iface: gfx.Sheet,
    resource: gfx.Sheet,
    buttons: gfx.Sheet,
    pics: gfx.Sheet,
    stuff: gfx.Sheet,
    x: f32,
    y: f32,
    exit: Button,
    box: wi.ScrollBox,
    res_masks: [max_res]Mask = undefined,
    hint: Hint = .{},
    old_cursor: u32,

    /// WindowDefenderInfo(id, gold, gem): 0, 0 shows the type's upkeep.
    pub fn defender(c: *Ctx, id: i32, gold_pay: i32, gem_pay: i32, m: ?player.Market) !scene.Scene {
        return create(c, .defender, id, gold_pay, gem_pay, m);
    }

    pub fn outer(c: *Ctx, id: i32, m: ?player.Market) !scene.Scene {
        return create(c, .outer, id, 0, 0, m);
    }

    pub fn ritual(c: *Ctx, id: i32, m: ?player.Market) !scene.Scene {
        return create(c, .ritual, id, 0, 0, m);
    }

    pub fn create(c: *Ctx, kind: Kind, id: i32, gold_pay: i32, gem_pay: i32, m: ?player.Market) !scene.Scene {
        const pr = c.proto;
        const self = try c.gpa.create(CostWindow);
        errdefer c.gpa.destroy(self);
        self.* = .{
            .kind = kind,
            .id = id,
            .name = "",
            .body = "",
            .gold_cost = 0,
            .gem_cost = 0,
            .market = m,
            .iface = undefined,
            .resource = undefined,
            .buttons = undefined,
            .pics = undefined,
            .stuff = undefined,
            .x = 0,
            .y = 0,
            .exit = undefined,
            .box = undefined,
            .old_cursor = c.cursor,
        };
        var sheet: []const u8 = undefined;
        switch (kind) {
            .defender => {
                if (id <= 0 or id >= pr.defenders.len) return error.NoDefender;
                const d = &pr.defenders[@intCast(id)];
                self.fill(d.name, d.text, d.gold_cost, d.gem_cost, &d.res);
                self.gold_pay = gold_pay;
                self.gem_pay = gem_pay;
                if (gold_pay + gem_pay == 0) {
                    self.gold_pay = d.gold_payment;
                    self.gem_pay = d.gem_payment;
                }
                sheet = "defenders";
            },
            .outer => {
                if (id <= 0 or id >= pr.outer_buildings.len) return error.NoBuilding;
                const b = &pr.outer_buildings[@intCast(id)];
                self.fill(b.name, b.text, b.gold_cost, b.gem_cost, &b.res);
                sheet = "outbuild";
            },
            .ritual => {
                if (id <= 0 or id >= pr.rituals.len) return error.NoRitual;
                const r = &pr.rituals[@intCast(id)];
                self.fill(r.name, r.text, r.gold_cost, r.gem_cost, &r.res);
                self.cooldown = r.cooldown;
                sheet = "ritual";
            },
        }
        self.iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer self.iface.deinit();
        self.buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer self.buttons.deinit();
        self.pics = try gfx.Sheet.load(c.fs, sheet);
        errdefer self.pics.deinit();
        self.stuff = try gfx.Sheet.load(c.fs, "stuff");
        errdefer self.stuff.deinit();
        self.resource = try gfx.Sheet.load(c.fs, "resource");
        errdefer self.resource.deinit();
        const wt = self.iface.texIndex(self.gui()) orelse return error.MissingInfoWindow;
        const x: f32 = @floatFromInt(512 - @divTrunc(wt.width, 2));
        const y: f32 = @floatFromInt(384 - @divTrunc(wt.height, 2));
        self.x = x;
        self.y = y;
        self.exit = try Button.init(&self.buttons, x + 168, y + 372, butt_exit, butt_exit + 1, butt_exit + 2);
        errdefer self.exit.deinit();
        self.exit.hotkey = .escape;
        self.box = try wi.ScrollBox.init(c, &self.buttons, x + 34, y + 140, 340, 195, .{ x + 185, y + 115 }, .{ x + 185, y + 342 }, self.body);
        for (&self.res_masks, 0..) |*mk, i| mk.* = Mask.rect(x + 220 + 26 * @as(f32, @floatFromInt(i)), y + 83, 20, 20);
        c.setCursor(0);
        return scene.Scene.of(CostWindow, self, true);
    }

    fn fill(self: *CostWindow, name: []const u8, body: []const u8, gold: i32, gem: i32, res: []const i32) void {
        self.name = name;
        self.body = body;
        self.gold_cost = gold;
        self.gem_cost = gem;
        for (res[0..@min(res.len, max_res)], 0..) |r, i| self.res[i] = r;
    }

    fn gui(self: *const CostWindow) usize {
        return if (self.kind == .defender) gui_defender_info else gui_outer_info;
    }

    pub fn deinit(self: *CostWindow, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.box.deinit();
        self.exit.deinit();
        self.resource.deinit();
        self.stuff.deinit();
        self.pics.deinit();
        self.buttons.deinit();
        self.iface.deinit();
    }

    fn fieldHint(self: *CostWindow, c: *Ctx, p: input.Pointer, fd: Field) void {
        if (p.in(self.x + fd.x, self.y + fd.y, fd.w, fd.h)) self.hint.phrase(c, fd.hint, p.pos);
    }

    pub fn update(self: *CostWindow, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        const pr = c.proto;
        self.hint.clear();
        if (self.exit.check(f)) return .pop;
        self.box.update(f);
        const p = f.pointer;
        for (self.res, &self.res_masks) |r, *m| {
            if (r <= 0 or r >= pr.resources.len) continue;
            const res = m.check(p);
            if (m.hot()) {
                self.hint.what = .{ .string = pr.resources[@intCast(r)].name };
                self.hint.pos = p.pos;
            }
            if (res != .none) return .{ .push = ResourceWindow.create(c, r, self.market) catch return .none };
        }
        for (cost_fields) |fd| self.fieldHint(c, p, fd);
        if (self.kind == .defender) for (pay_fields) |fd| self.fieldHint(c, p, fd);
        if (self.kind == .ritual) self.fieldHint(c, p, cooldown_field);
        return .none;
    }

    pub fn draw(self: *CostWindow, c: *Ctx) void {
        const x = self.x;
        const y = self.y;
        var b: [16]u8 = undefined;
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 255 - 190 });
        gfx.drawTex(self.iface.texIndex(self.gui()), x, y);
        if (self.kind == .ritual) gfx.drawTex(self.stuff.texIndex(stuff_cooldown), x + 298, y + 81);
        const pic = self.pics.texIndex(@intCast(self.id - 1));
        gfx.drawTex(pic, x + 21, y + 20);
        gfx.drawTex(pic, x + 328, y + 20);
        c.font1.drawCentered(self.name, x + 200, y + 22, color1);
        if (self.kind == .defender) {
            c.font1.drawCentered(castle_ui.fmtInt(&b, self.gold_pay), x + 157, y + 54, color2);
            c.font1.drawCentered(castle_ui.fmtInt(&b, self.gem_pay), x + 253, y + 54, color2);
        }
        c.font1.drawCentered(castle_ui.fmtInt(&b, self.gold_cost), x + 74, y + 89, color2);
        c.font1.drawCentered(castle_ui.fmtInt(&b, self.gem_cost), x + 169, y + 89, color2);
        if (self.kind == .ritual) c.font1.drawCentered(castle_ui.fmtInt(&b, self.cooldown), x + 343, y + 89, color2);
        for (self.res, 0..) |r, i| {
            if (r <= 0 or r >= c.proto.resources.len) continue;
            gfx.drawTex(self.resource.texIndex(@intCast(r - 1)), x + 220 + 26 * @as(f32, @floatFromInt(i)), y + 83);
        }
        self.box.draw(c, &self.buttons, self.body);
        self.exit.draw(&self.buttons, c.tick);
        self.hint.draw(c, &self.iface);
    }
};
