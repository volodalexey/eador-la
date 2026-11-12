//! GUI_UnitBlock (Screens.cpp:424): the squad slots of a garrison by rank,
//! and GUI_RecruitBlock (Screens.cpp:1330): the twelve units for hire with
//! their prices and resources.  A right click asks for an info window
//! (`info`): WindowUnitInfo on a squad unit, WindowUnitTypeInfo on a unit
//! for hire, WindowResourceInfo on a resource of a card (any click).
const std = @import("std");
const rl = @import("raylib");
const Ctx = @import("../app.zig").Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Mask = @import("mask.zig").Mask;
const Hint = @import("hint.zig").Hint;
const castle_ui = @import("castle_ui.zig");
const Sheets = castle_ui.Sheets;
const gui = castle_ui.gui;
const hero_mod = @import("../game/hero.zig");
const Hero = hero_mod.Hero;
const Unit = @import("../game/unit.zig").Unit;
const army = @import("../game/army.zig");
const player = @import("../game/player.zig");
const proto_mod = @import("../data/proto.zig");

/// Phrases: 10 "уровень", 333 "наёмник".
const ph_level = 10;
const ph_merc = 333;

/// The window a block asks for (scenes/army_info.zig).
pub const Info = union(enum) { unit: usize, unit_type: i32, resource: i32 };

pub const UnitBlock = struct {
    x: f32,
    y: f32,
    masks: [hero_mod.max_squad]Mask = undefined,
    /// Check == 40 on a unit: its window.
    info: ?Info = null,
    /// A hero's squad: GetSquadMorale with his bonuses.
    of_hero: bool = false,
    buf: [96]u8 = undefined,

    const w = 72;
    const h = 88;
    const d = -8;

    pub fn init(x: f32, y: f32) UnitBlock {
        var b: UnitBlock = .{ .x = x, .y = y };
        for (b.masks[1..], 1..) |*m, i| {
            const at = slotPos(i);
            m.* = .{ .x = x + d + at[0], .y = y + d + at[1] };
        }
        return b;
    }

    /// The picture of slot i (1..15) relative to the block: rank 4 on
    /// top, rank 1 at the bottom.
    fn slotPos(i: usize) [2]f32 {
        const lvl: usize = @intCast(army.slotLevel(i));
        const col: f32 = @floatFromInt(i - army.rank_first[lvl]);
        return .{ col * w, @as(f32, @floatFromInt(4 - lvl)) * h };
    }

    fn open(g: *const Hero, pr: *const @import("../data/proto.zig").Proto, i: usize) bool {
        return i < army.openSlots(g, pr, @intCast(army.slotLevel(i)))[1];
    }

    /// Check: the clicked open slot; the hint names the unit under the
    /// pointer.
    pub fn check(self: *UnitBlock, c: *Ctx, g: *const Hero, shape: *const rl.Image, p: input.Pointer, hint: *Hint) ?usize {
        var click: ?usize = null;
        self.info = null;
        for (self.masks[1..], 1..) |*m, i| {
            if (!open(g, c.proto, i)) {
                m.state = .visible;
                continue;
            }
            m.shape = shape;
            const r = m.check(p);
            if (m.hot()) if (g.squad[i]) |u| {
                const t = &c.proto.units[@intCast(u.type)];
                hint.what = .{ .string = std.fmt.bufPrint(&self.buf, "{s} ({s} {d})", .{ t.name, c.proto.phrase(ph_level), t.level }) catch "" };
                hint.pos = p.pos;
            };
            if (r == .click and click == null) click = i;
            if (r == .right_click and g.squad[i] != null) self.info = .{ .unit = i };
        }
        return click;
    }

    /// Show, and the masks: the select frame under the pointer, the glow
    /// where the held unit fits.
    pub fn draw(self: *const UnitBlock, c: *Ctx, sh: *Sheets, g: *const Hero, karma: i32, held: ?Unit) void {
        for (1..hero_mod.max_squad) |i| {
            const at = slotPos(i);
            const x = self.x + at[0];
            const y = self.y + at[1];
            if (!open(g, c.proto, i)) {
                gfx.drawTex(sh.iface.texIndex(gui.unit_closed), x, y);
                continue;
            }
            if (g.squad[i]) |*u| castle_ui.drawUnit(c, sh, u, x, y, hero_mod.squadMorale(c.proto, &g.squad, i, karma, if (self.of_hero) g else null));
        }
        for (self.masks[1..], 1..) |m, i| {
            if (!open(g, c.proto, i)) continue;
            if (m.hot()) {
                gfx.drawTex(sh.iface.texIndex(gui.unit_select), m.x, m.y);
            } else if (held) |u| if (c.proto.units[@intCast(u.type)].level <= army.slotLevel(i)) {
                gfx.drawTex(sh.iface.texIndex(gui.unit_glow), m.x, m.y);
            };
        }
    }
};

pub const RecruitBlock = struct {
    x: f32,
    y: f32,
    /// arRecr and arCost of the last recalc.
    recruit: [player.max_recruit]i32 = @splat(0),
    cost: [player.max_recruit]player.Price = @splat(.{ 0, 0 }),
    masks: [player.max_recruit]Mask = undefined,
    res_masks: [player.max_recruit][proto_mod.max_unit_res]Mask = undefined,
    info: ?Info = null,
    buf: [128]u8 = undefined,

    const w = 81;
    const rw = 22;
    const h = 179;
    const d = -8;

    pub const Pick = struct { unit: i32, merc: bool };

    pub fn init(x: f32, y: f32) RecruitBlock {
        var b: RecruitBlock = .{ .x = x, .y = y };
        for (&b.masks, 0..) |*m, i| {
            const at = picPos(i);
            m.* = .{ .x = x + d + at[0], .y = y + d + at[1] };
            for (&b.res_masks[i], 0..) |*rm, j| rm.* = Mask.rect(x + d + at[0] - 21 + 26 + @as(f32, @floatFromInt(j)) * rw, y + d + at[1] - 16 + 166, 21, 21);
        }
        return b;
    }

    /// The unit picture of slot i: 0..5 the bottom row, 6..11 the top.
    fn picPos(i: usize) [2]f32 {
        const col: f32 = @floatFromInt(i % 6);
        return .{ col * w + 21, if (i < 6) h + 16 else 16 };
    }

    /// Recalc and the prices of Show (the mercenary at double price).
    pub fn recalc(self: *RecruitBlock, m: player.Market, distance: i32) void {
        self.recruit = m.holdings().recruit;
        for (self.recruit, &self.cost, 0..) |t, *cost, i| {
            if (t <= 0 or t >= m.pr.units.len) {
                cost.* = .{ 0, 0 };
                continue;
            }
            cost.* = m.unitPrice(t, distance);
            if (i == player.merc_slot) cost.* = .{ cost[0] * 2, cost[1] * 2 };
        }
    }

    fn valid(self: *const RecruitBlock, c: *const Ctx, i: usize) bool {
        return self.recruit[i] > 0 and self.recruit[i] < c.proto.units.len;
    }

    /// Check: a click on an affordable unit.
    pub fn check(self: *RecruitBlock, c: *Ctx, m: player.Market, shape: *const rl.Image, p: input.Pointer, hint: *Hint) ?Pick {
        const pr = c.proto;
        var pick: ?Pick = null;
        self.info = null;
        for (&self.masks, 0..) |*mk, i| {
            if (!self.valid(c, i)) {
                mk.state = .visible;
                continue;
            }
            mk.shape = shape;
            const r = mk.check(p);
            const t = &pr.units[@intCast(self.recruit[i])];
            if (mk.hot()) {
                hint.what = .{ .string = (if (i == player.merc_slot)
                    std.fmt.bufPrint(&self.buf, "{s} ({s}, {s} {d})", .{ t.name, pr.phrase(ph_merc), pr.phrase(ph_level), t.level })
                else
                    std.fmt.bufPrint(&self.buf, "{s} ({s} {d})", .{ t.name, pr.phrase(ph_level), t.level })) catch "" };
                hint.pos = p.pos;
            }
            if (r == .click and pick == null and m.affords(self.cost[i])) pick = .{ .unit = self.recruit[i], .merc = i == player.merc_slot };
            if (r == .right_click) self.info = .{ .unit_type = self.recruit[i] };
            for (t.res, &self.res_masks[i]) |res, *rm| {
                if (res <= 0) continue;
                const rr = rm.check(p);
                if (rm.hot()) {
                    hint.what = .{ .string = pr.resources[@intCast(res)].name };
                    hint.pos = p.pos;
                }
                if (rr != .none) self.info = .{ .resource = res };
            }
        }
        return pick;
    }

    pub fn draw(self: *const RecruitBlock, c: *Ctx, sh: *Sheets, m: player.Market) void {
        const pr = c.proto;
        const r = m.ruler();
        const yellow = castle_ui.yellow;
        const red: rl.Color = .{ .r = 220, .g = 20, .b = 0, .a = 255 };
        var b: [16]u8 = undefined;
        for (0..player.max_recruit) |i| {
            const at = picPos(i);
            const col: f32 = @floatFromInt(i % 6);
            const row: f32 = if (i < 6) h else 0;
            if (!self.valid(c, i)) {
                gfx.drawTex(sh.iface.texIndex(gui.recruit_closed), self.x + 17 + col * w, self.y + 12 + row);
                continue;
            }
            const t = &pr.units[@intCast(self.recruit[i])];
            gfx.drawTex(sh.unitPic(self.recruit[i]), self.x + at[0], self.y + at[1]);
            for (t.res, 0..) |res, j| {
                if (res <= 0) continue;
                const rx = self.x + col * w + 20 + @as(f32, @floatFromInt(j)) * rw;
                const ry = self.y + row + 160;
                gfx.drawTex(sh.res(res), rx, ry);
                if (m.resource(res) == 0) gfx.drawTex(sh.iface.texIndex(gui.no_res), rx, ry);
            }
            const cost = self.cost[i];
            c.font1.drawCentered(castle_ui.fmtInt(&b, cost[0]), self.x + col * w + 60, self.y + row + 109, if (r.gold >= cost[0]) yellow else red);
            const gem_ok = r.gem >= cost[1] or (i < 6 and cost[1] == 0);
            c.font1.drawCentered(castle_ui.fmtInt(&b, cost[1]), self.x + col * w + 60, self.y + row + 138, if (gem_ok) yellow else red);
        }
        for (self.masks, 0..) |mk, i| {
            if (self.valid(c, i) and mk.hot()) gfx.drawTex(sh.iface.texIndex(gui.unit_select), mk.x, mk.y);
        }
    }
};
