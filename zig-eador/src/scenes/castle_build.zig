//! ScreenCastleBuild (Castle.cpp:250) and WindowInnBldInfo
//! (Windower.cpp:3897): the 63 building slots of the castle by quarter,
//! the planned building in slot 0, the building card (MakeInnBldHint) on
//! hover and the building window on a click with "Build", "Plan" and the
//! links to the needed buildings and to those it opens.
//!
//! A click on the unit, a spell, item, outer building, defender, ritual or
//! resource opens its window (WindowUnitTypeInfo, WindowSpellInfo, ...);
//! hovering shows the name or the item card.  Not ported: the tutorial
//! hints.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Hint = @import("../ui/hint.zig").Hint;
const Mask = @import("../ui/mask.zig").Mask;
const Button = @import("../ui/button.zig").Button;
const text = @import("../ui/text.zig");
const castle_ui = @import("../ui/castle_ui.zig");
const army_info = @import("army_info.zig");
const CostWindow = @import("info_windows.zig").CostWindow;
const SpellInfo = @import("spell_info.zig").SpellInfo;
const ItemInfo = @import("item_info.zig").ItemInfo;
const Sheets = castle_ui.Sheets;
const gui = castle_ui.gui;
const proto_mod = @import("../data/proto.zig");
const player = @import("../game/player.zig");
const castle = @import("../game/castle.zig");

/// stuff.dat: the lock of a building that cannot be built yet.
const stuff_lock1 = 40;
const stuff_lock2 = 41;
/// unit_sound.dat wavBuild.
const wav_build = 157;
/// set_trans_blender(128) left by MakeCastlePic.
const half: rl.Color = .{ .r = 255, .g = 255, .b = 255, .a = 128 };
const dark: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 128 };
const green: rl.Color = .{ .r = 0, .g = 250, .b = 0, .a = 255 };

const dx = 58;
/// arSlot of ScreenCastleBuild.
const slot_xy = blk: {
    var a: [castle.n_slots][2]f32 = undefined;
    a[0] = .{ 56, 14 };
    a[1] = .{ 401, 14 };
    a[2] = .{ 16, 80 };
    a[3] = .{ 16, 137 };
    const rows = [_]struct { first: usize, n: usize, x: f32, y: f32 }{
        .{ .first = 4, .n = 7, .x = 95, .y = 80 },
        .{ .first = 11, .n = 6, .x = 95, .y = 137 },
        .{ .first = 17, .n = 3, .x = 172, .y = 213 },
        .{ .first = 20, .n = 8, .x = 28, .y = 270 },
        .{ .first = 28, .n = 2, .x = 16, .y = 346 },
        .{ .first = 30, .n = 6, .x = 150, .y = 346 },
        .{ .first = 36, .n = 3, .x = 56, .y = 421 },
        .{ .first = 39, .n = 3, .x = 56, .y = 478 },
        .{ .first = 42, .n = 3, .x = 289, .y = 421 },
        .{ .first = 45, .n = 2, .x = 319, .y = 478 },
        .{ .first = 47, .n = 5, .x = 18, .y = 553 },
        .{ .first = 52, .n = 3, .x = 323, .y = 553 },
        .{ .first = 55, .n = 5, .x = 18, .y = 629 },
        .{ .first = 60, .n = 3, .x = 323, .y = 629 },
    };
    for (rows) |r| for (0..r.n) |i| {
        a[r.first + i] = .{ r.x + dx * @as(f32, @floatFromInt(i)), r.y };
    };
    break :blk a;
};

/// The quarter titles (phrases 232..241).
const titles = [_][2]f32{ .{ 252, 19 }, .{ 252, 64 }, .{ 252, 198 }, .{ 252, 331 }, .{ 134, 406 }, .{ 373, 406 }, .{ 150, 538 }, .{ 408, 538 }, .{ 150, 614 }, .{ 408, 614 } };

/// The building slots of ScreenCastleBuild.
pub const BuildPanel = struct {
    shape: rl.Image,
    masks: [castle.n_slots]Mask = undefined,
    /// arBld: the building of each slot.
    slots: [castle.n_slots]i32 = @splat(0),
    rules: castle.Rules = undefined,
    /// The building whose card is shown, and where.
    card: ?i32 = null,
    card_pos: rl.Vector2 = .{ .x = 0, .y = 0 },

    pub fn init(sh: *Sheets) !BuildPanel {
        var p: BuildPanel = .{ .shape = sh.iface.imageIndex(gui.spell_mask) orelse return error.MissingMask };
        for (&p.masks, slot_xy) |*m, xy| m.* = .{ .x = xy[0] - 6, .y = xy[1] - 6 };
        return p;
    }

    pub fn deinit(self: *BuildPanel) void {
        rl.unloadImage(self.shape);
    }

    /// The top of the screen loop: a planned building that is built, can
    /// be built now or never can is dropped; the slots are laid out.
    pub fn refresh(self: *BuildPanel, m: player.Market) !void {
        self.rules = try castle.Rules.of(m.s, m.pr, @intCast(m.player));
        const h = m.holdings();
        if (h.planned > 0 and (h.planned >= m.pr.inner_buildings.len or self.rules.built(h.planned) or self.rules.canBuild(h.planned) or self.rules.isRestricted(h.planned))) h.planned = 0;
        self.rules.slots(&self.slots);
    }

    /// Returns the building to open WindowInnBldInfo for.
    pub fn update(self: *BuildPanel, m: player.Market, p: input.Pointer) ?i32 {
        self.card = null;
        var open: ?i32 = null;
        for (&self.masks, self.slots, 0..) |*mk, b, i| {
            mk.shape = &self.shape;
            if (i == 1 or (i >= 2 and b <= 0)) {
                mk.state = .visible;
                continue;
            }
            const r = mk.check(p);
            if (i == 0) {
                // The planner: only its card.
                if (mk.hot() and m.holdings().planned > 0) self.showCard(m.holdings().planned, p);
                continue;
            }
            if (r == .click or r == .right_click) {
                open = b;
            } else if (mk.hot()) self.showCard(b, p);
        }
        return open;
    }

    fn showCard(self: *BuildPanel, b: i32, p: input.Pointer) void {
        self.card = b;
        self.card_pos = p.pos;
    }

    pub fn draw(self: *const BuildPanel, c: *Ctx, sh: *Sheets, m: player.Market) void {
        const r = m.ruler();
        const h = m.holdings();
        for (titles, 0..) |t, i| c.font1.drawCentered(c.proto.phrase(232 + i), t[0], t[1], castle_ui.color1);
        if (h.planned > 0) {
            gfx.drawTex(sh.building(h.planned), slot_xy[0][0], slot_xy[0][1]);
            gfx.drawTex(sh.iface.texIndex(gui.planned), slot_xy[0][0], slot_xy[0][1]);
        }
        for (self.slots[2..], 2..) |b, i| {
            const x = slot_xy[i][0];
            const y = slot_xy[i][1];
            gfx.drawTex(sh.building(b), x, y);
            if (b <= 0) continue;
            if (self.rules.built(b)) {
                gfx.drawTex(sh.iface.texIndex(gui.built), x + 26, y);
            } else if (self.rules.canBuild(b)) {
                if (h.planned > 0 and self.rules.isParent(b, h.planned) != 0) gfx.drawTex(sh.iface.texIndex(gui.planned), x, y);
                const price = castle.innerPrice(m, b);
                var shift: f32 = 26;
                if (price[0] > r.gold) {
                    gfx.drawTex(sh.iface.texIndex(gui.no_gold), x + shift, y);
                    shift = 0;
                }
                if (price[1] > r.gem and price[1] > 0) gfx.drawTex(sh.iface.texIndex(gui.no_gem), x + shift, y);
            } else {
                rl.drawRectangle(@intFromFloat(x), @intFromFloat(y), 52, 52, dark);
                gfx.drawTex2(sh.stuff.texIndex(stuff_lock1), x, y, half);
                gfx.drawTex2(sh.stuff.texIndex(stuff_lock2), x, y, half);
            }
        }
        for (self.masks, self.slots, 0..) |mk, b, i| {
            if (mk.hot() and (b > 0 or i == 0)) gfx.drawTex(sh.iface.texIndex(gui.spell_select), mk.x, mk.y);
        }
    }

    pub fn drawOver(self: *const BuildPanel, c: *Ctx, sh: *Sheets, m: player.Market) void {
        if (self.card) |b| castle_ui.buildingCard(c, sh, self.card_pos, m, b);
    }
};

/// What WindowInnBldInfo returned: -1 build, -2 plan, a building to show.
pub const Choice = union(enum) { none, build, plan, open: i32 };

const max_objs = 6;
const max_breed = 25;
const breed_line = 5;

const ObjKind = enum { spell, item, outer, defender, ritual };
const Obj = struct { kind: ObjKind, id: i32 };
const Card = union(enum) { none, building: i32, group: i32, item: i32 };

/// WindowInnBldInfo: the building window over the castle.
pub const BuildInfo = struct {
    sh: *Sheets,
    m: player.Market,
    rules: castle.Rules,
    id: i32,
    result: *Choice,
    x: f32,
    y: f32,
    height: f32,
    w: f32,
    exit: Button,
    build: Button,
    plan: Button,
    left: ?Button = null,
    right: ?Button = null,
    shape: rl.Image,
    unit_shape: rl.Image,
    unit: i32 = 0,
    unit_y: f32 = 0,
    unit_mask: Mask = .{ .x = 0, .y = 0 },
    objs: [max_objs]Obj = undefined,
    n_objs: usize = 0,
    obj_masks: [max_objs]Mask = undefined,
    parents_y: f32 = 0,
    n_parents: usize = 0,
    parent_masks: [castle.max_need]Mask = undefined,
    breed: [max_breed]i32 = @splat(0),
    n_breed: usize = 0,
    breed_y: f32 = 0,
    line: usize = 0,
    breed_masks: [breed_line]Mask = @splat(.{ .x = 0, .y = 0 }),
    res_masks: [castle.max_res]Mask = undefined,
    hint: Hint = .{},
    card: Card = .none,
    card_pos: rl.Vector2 = .{ .x = 0, .y = 0 },
    old_cursor: u32,

    /// `sh` belongs to the castle screen below, which outlives the window.
    pub fn create(c: *Ctx, sh: *Sheets, m: player.Market, id: i32, result: *Choice) !scene.Scene {
        const pr = c.proto;
        const self = try c.gpa.create(BuildInfo);
        errdefer c.gpa.destroy(self);
        const b = &pr.inner_buildings[@intCast(id)];
        const rules = try castle.Rules.of(m.s, pr, @intCast(m.player));
        const bg = sh.iface.texIndex(gui.inn_bld_info) orelse return error.MissingBuildInfo;

        var unit: i32 = 0;
        var objs: [max_objs]Obj = undefined;
        var n_objs: usize = 0;
        for (pr.spells[1..], 1..) |sp, i| if (sp.building == id and n_objs < max_objs) {
            objs[n_objs] = .{ .kind = .spell, .id = @intCast(i) };
            n_objs += 1;
        };
        for (pr.items[1..], 1..) |it, i| if (it.building == id and n_objs < max_objs) {
            objs[n_objs] = .{ .kind = .item, .id = @intCast(i) };
            n_objs += 1;
        };
        for (b.abilities[0..b.n_abilities]) |a| {
            const kind: ?ObjKind = switch (a.id) {
                11 => .outer,
                15 => .defender,
                27 => .ritual,
                else => null,
            };
            if (a.id == 8) unit = a.p1;
            if (kind) |k| if (n_objs < max_objs) {
                objs[n_objs] = .{ .kind = k, .id = a.p1 };
                n_objs += 1;
            };
        }
        var breed: [max_breed]i32 = @splat(0);
        var n_breed: usize = 0;
        for (pr.inner_buildings[1..], 1..) |ib, i| for (ib.needs) |n| {
            if (n == id and rules.noAllyFlag(@intCast(i)) and n_breed < max_breed) {
                breed[n_breed] = @intCast(i);
                n_breed += 1;
            }
        };
        var height: f32 = 390;
        if (unit > 0) height += 90;
        if (n_objs > 0) height += 70;
        if (b.needs[0] > 0) height += 100;
        if (n_breed > 0) height += 100;
        const w: f32 = @floatFromInt(bg.width);
        const x: f32 = 512 - @as(f32, @floatFromInt(@divTrunc(bg.width, 2)));
        const y: f32 = 384 - @floor(height / 2);

        var exit = try Button.init(&sh.buttons, x + 300, y + height - 49, 247, 248, 249);
        errdefer exit.deinit();
        exit.hotkey = .escape;
        exit.hint = 135;
        var build = try Button.init(&sh.buttons, x + 18, y + height - 49, 239, 240, 241);
        errdefer build.deinit();
        build.inactive = 242;
        build.hint = 93;
        build.sound = wav_build;
        var plan = try Button.init(&sh.buttons, x + 159, y + height - 49, 243, 244, 245);
        errdefer plan.deinit();
        plan.inactive = 246;
        plan.hint = 251;
        const shape = sh.iface.imageIndex(gui.spell_mask) orelse return error.MissingMask;
        errdefer rl.unloadImage(shape);
        const unit_shape = sh.iface.imageIndex(gui.unit_mask) orelse return error.MissingMask;
        errdefer rl.unloadImage(unit_shape);

        self.* = .{
            .sh = sh,
            .m = m,
            .rules = rules,
            .id = id,
            .result = result,
            .x = x,
            .y = y,
            .height = height,
            .w = w,
            .exit = exit,
            .build = build,
            .plan = plan,
            .shape = shape,
            .unit_shape = unit_shape,
            .unit = unit,
            .objs = objs,
            .n_objs = n_objs,
            .breed = breed,
            .n_breed = n_breed,
            .old_cursor = c.cursor,
        };
        self.build.on = !rules.built(id) and rules.canBuild(id) and m.holdings().can_inner_bld > 0 and m.affords(castle.innerPrice(m, id));
        self.plan.on = !rules.built(id) and !rules.canBuild(id) and !rules.isRestricted(id);
        self.layout(sh, b);
        if (n_breed > breed_line) {
            self.left = try Button.init(&sh.buttons, x + 15, y + self.breed_y + 15, 236, 237, 238);
            self.left.?.inactive = 288;
            self.left.?.on = false;
            self.right = try Button.init(&sh.buttons, x + 340, y + self.breed_y + 15, 233, 234, 235);
            self.right.?.inactive = 287;
        }
        c.setCursor(0);
        return scene.Scene.of(BuildInfo, self, true);
    }

    fn layout(self: *BuildInfo, sh: *Sheets, b: *const proto_mod.InnerBuilding) void {
        var shift: f32 = 342;
        if (self.unit > 0) {
            const sw = gfx.texW(sh.iface.texIndex(gui.unit_slot));
            self.unit_y = shift;
            self.unit_mask = .{ .x = self.x + self.w / 2 - @floor(sw / 2) - 3, .y = self.y + shift - 6 };
            shift += 90;
        }
        if (self.n_objs > 0) {
            const s0 = self.w / 2 - 30 * @as(f32, @floatFromInt(self.n_objs));
            for (self.obj_masks[0..self.n_objs], 0..) |*mk, i| mk.* = .{ .x = self.x + s0 + 60 * @as(f32, @floatFromInt(i)) - 1, .y = self.y + shift - 1 };
            shift += 70;
        }
        if (b.needs[0] > 0) {
            for (b.needs) |n| self.n_parents += @intFromBool(n > 0);
            shift += 30;
            self.parents_y = shift;
            const s0 = self.w / 2 - 30 * @as(f32, @floatFromInt(self.n_parents));
            for (self.parent_masks[0..self.n_parents], 0..) |*mk, i| mk.* = .{ .x = self.x + s0 + 60 * @as(f32, @floatFromInt(i)) - 1, .y = self.y + shift - 1 };
            shift += 70;
        }
        if (self.n_breed > 0) {
            shift += 30;
            self.breed_y = shift;
        }
        for (&self.res_masks, b.res, 0..) |*mk, r, i| {
            mk.* = if (r > 0) Mask.rect(self.x + 214 + 26 * @as(f32, @floatFromInt(i % 3)), self.y + 57 + 25 * @as(f32, @floatFromInt(i / 3)), 20, 20) else Mask.rect(0, 0, 0, 0);
        }
    }

    fn shown(self: *const BuildInfo) usize {
        return @min(breed_line, self.n_breed - self.line * breed_line);
    }

    fn breedX(self: *const BuildInfo, i: usize) f32 {
        return self.w / 2 - 30 * @as(f32, @floatFromInt(self.shown())) + 60 * @as(f32, @floatFromInt(i));
    }

    pub fn deinit(self: *BuildInfo, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        if (self.left) |*b| b.deinit();
        if (self.right) |*b| b.deinit();
        self.plan.deinit();
        self.build.deinit();
        self.exit.deinit();
        rl.unloadImage(self.unit_shape);
        rl.unloadImage(self.shape);
    }

    fn setCard(self: *BuildInfo, p: input.Pointer, what: Card) void {
        self.card = what;
        self.card_pos = p.pos;
    }

    pub fn update(self: *BuildInfo, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        const pr = c.proto;
        const p = f.pointer;
        self.hint.clear();
        self.card = .none;
        if (self.exit.check(f)) return .pop;
        if (self.unit > 0) {
            self.unit_mask.shape = &self.unit_shape;
            if (self.unit_mask.check(p) != .none) return .{ .push = army_info.UnitWindow.createType(c, self.unit) catch return .none };
        }
        for (self.obj_masks[0..self.n_objs], self.objs[0..self.n_objs]) |*mk, o| {
            mk.shape = &self.shape;
            if (mk.check(p) != .none) {
                const next = switch (o.kind) {
                    .spell => SpellInfo.create(c, o.id),
                    .item => ItemInfo.createFor(c, o.id, pr.items[@intCast(o.id)].durability, self.m),
                    .outer => CostWindow.outer(c, o.id, self.m),
                    .defender => CostWindow.defender(c, o.id, 0, 0, self.m),
                    .ritual => CostWindow.ritual(c, o.id, self.m),
                };
                return .{ .push = next catch return .none };
            }
            if (!mk.hot()) continue;
            if (o.kind == .item) {
                self.setCard(p, .{ .item = o.id });
                continue;
            }
            const name = switch (o.kind) {
                .spell => pr.spells[@intCast(o.id)].name,
                .outer => if (o.id > 0 and o.id < pr.outer_buildings.len) pr.outer_buildings[@intCast(o.id)].name else "",
                .defender => if (o.id > 0 and o.id < pr.defenders.len) pr.defenders[@intCast(o.id)].name else "",
                .ritual => if (o.id > 0 and o.id < pr.rituals.len) pr.rituals[@intCast(o.id)].name else "",
                .item => unreachable,
            };
            self.hint.what = .{ .string = name };
            self.hint.pos = p.pos;
        }
        const b = &pr.inner_buildings[@intCast(self.id)];
        var parent: usize = 0;
        for (b.needs) |n| {
            if (n <= 0) continue;
            const mk = &self.parent_masks[parent];
            parent += 1;
            mk.shape = &self.shape;
            const r = mk.check(p);
            if (n < 1000) {
                if (r == .click or r == .right_click) return self.choose(.{ .open = n });
                if (mk.hot()) self.setCard(p, .{ .building = n });
            } else if (mk.hot()) self.setCard(p, .{ .group = n - 1000 });
        }
        for (self.breed_masks[0..self.shown()], 0..) |*mk, i| {
            mk.x = self.x + self.breedX(i) - 1;
            mk.y = self.y + self.breed_y - 1;
            mk.shape = &self.shape;
            const r = mk.check(p);
            const id = self.breed[self.line * breed_line + i];
            if (r == .click or r == .right_click) return self.choose(.{ .open = id });
            if (mk.hot()) self.setCard(p, .{ .building = id });
        }
        for (&self.res_masks, b.res) |*mk, r| {
            if (r <= 0) continue;
            if (mk.check(p) != .none) return .{ .push = army_info.ResourceWindow.create(c, r, self.m) catch return .none };
            if (mk.hot()) {
                self.hint.what = .{ .string = pr.resources[@intCast(r)].name };
                self.hint.pos = p.pos;
            }
        }
        if (self.left) |*l| if (l.check(f) and self.line > 0) {
            self.line -= 1;
            l.on = self.line > 0;
            self.right.?.on = true;
        };
        if (self.right) |*rb| if (rb.check(f)) {
            self.line += 1;
            rb.on = (self.line + 1) * breed_line < self.n_breed;
            self.left.?.on = true;
        };
        if (self.build.check(f)) return self.choose(.build);
        if (self.plan.check(f)) return self.choose(.plan);
        for ([_]*const Button{ &self.exit, &self.build, &self.plan }) |bt| {
            if (bt.hot()) self.hint.phrase(c, bt.hint, p.pos);
        }
        return .none;
    }

    fn choose(self: *BuildInfo, r: Choice) scene.Action {
        self.result.* = r;
        return .pop;
    }

    pub fn draw(self: *BuildInfo, c: *Ctx) void {
        const pr = c.proto;
        const sh = self.sh;
        const m = self.m;
        const x = self.x;
        const y = self.y;
        const w = self.w;
        const b = &pr.inner_buildings[@intCast(self.id)];
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 255 - 190 });
        if (sh.iface.texIndex(gui.inn_bld_info)) |bg| {
            rl.drawTextureRec(bg, .{ .x = 0, .y = 0, .width = w, .height = self.height - 51 }, .{ .x = x, .y = y }, .white);
            rl.drawTextureRec(bg, .{ .x = 0, .y = 717, .width = w, .height = 51 }, .{ .x = x, .y = y + self.height - 51 }, .white);
        }
        gfx.drawTex(sh.building(self.id), x + 23, y + 54);
        gfx.drawTex(sh.building(self.id), x + 299, y + 54);
        c.font1.drawCentered(b.name, x + 185, y + 24, castle_ui.color1);
        const r = m.ruler();
        const price = castle.innerPrice(m, self.id);
        var buf: [16]u8 = undefined;
        c.font1.drawCentered(castle_ui.fmtInt(&buf, price[0]), x + 150, y + 62, if (r.gold >= price[0]) castle_ui.color2 else castle_ui.short_red);
        c.font1.drawCentered(castle_ui.fmtInt(&buf, price[1]), x + 150, y + 87, if (r.gem >= price[1]) castle_ui.color2 else castle_ui.short_red);
        for (b.res, 0..) |rs, i| {
            if (rs <= 0) continue;
            const rx = x + 214 + 26 * @as(f32, @floatFromInt(i % 3));
            const ry = y + 57 + 25 * @as(f32, @floatFromInt(i / 3));
            gfx.drawTex(sh.res(rs), rx, ry);
            if (m.resource(rs) <= 0) gfx.drawTex(sh.iface.texIndex(gui.no_res), rx, ry);
        }
        self.drawState(c);
        const box = text.Box.init(x + 31, y + 170, 312, 142, &c.font2, castle_ui.color1);
        _ = box.draw(b.text);

        if (self.unit > 0) {
            const slot = sh.iface.texIndex(gui.unit_slot);
            gfx.drawTex(slot, x + w / 2 - @floor(gfx.texW(slot) / 2), y + self.unit_y);
            const pic = sh.unitPic(self.unit);
            gfx.drawTex(pic, x + w / 2 - @floor(gfx.texW(pic) / 2) - 2, y + self.unit_y);
            if (self.unit_mask.hot()) gfx.drawTex(sh.iface.texIndex(gui.unit_select), self.unit_mask.x, self.unit_mask.y);
        }
        for (self.objs[0..self.n_objs], self.obj_masks[0..self.n_objs]) |o, mk| {
            const sx = mk.x + 1;
            const sy = mk.y + 1;
            gfx.drawTex(sh.iface.texIndex(gui.spell_slot), sx, sy);
            const pic = switch (o.kind) {
                .spell => sh.spell(o.id),
                .item => sh.item(pr, o.id),
                .outer => if (o.id > 0) sh.outbuild.texIndex(@intCast(o.id - 1)) else null,
                .defender => if (o.id > 0) sh.defenders.texIndex(@intCast(o.id - 1)) else null,
                .ritual => if (o.id > 0) sh.ritual.texIndex(@intCast(o.id - 1)) else null,
            };
            gfx.drawTex(pic, sx + 5, sy + 5);
            if (mk.hot()) gfx.drawTex(sh.iface.texIndex(gui.spell_select), mk.x, mk.y);
        }
        if (self.n_parents > 0) {
            self.drawString(c, self.parents_y - 30, 242);
            var i: usize = 0;
            for (b.needs) |n| {
                if (n <= 0) continue;
                const mk = self.parent_masks[i];
                i += 1;
                const sx = mk.x + 1;
                const sy = mk.y + 1;
                gfx.drawTex(sh.iface.texIndex(gui.spell_slot), sx, sy);
                if (n < 1000) {
                    gfx.drawTex(sh.building(n), sx + 5, sy + 5);
                    if (!self.rules.built(n)) rl.drawRectangle(@intFromFloat(sx + 5), @intFromFloat(sy + 5), 52, 52, dark);
                } else {
                    gfx.drawTex(sh.iface.texIndex(gui.group_icon), sx + 5, sy + 5);
                    if (!self.rules.groupFull(n - 1000)) rl.drawRectangle(@intFromFloat(sx + 5), @intFromFloat(sy + 5), 52, 52, dark);
                }
                if (mk.hot()) gfx.drawTex(sh.iface.texIndex(gui.spell_select), mk.x, mk.y);
            }
        }
        if (self.n_breed > 0) {
            self.drawString(c, self.breed_y - 30, 243);
            for (0..self.shown()) |i| {
                const sx = x + self.breedX(i);
                const sy = y + self.breed_y;
                gfx.drawTex(sh.iface.texIndex(gui.spell_slot), sx, sy);
                gfx.drawTex(sh.building(self.breed[self.line * breed_line + i]), sx + 5, sy + 5);
                const mk = self.breed_masks[i];
                if (mk.hot()) gfx.drawTex(sh.iface.texIndex(gui.spell_select), mk.x, mk.y);
            }
            if (self.left) |*l| l.draw(&sh.buttons, c.tick);
            if (self.right) |*rb| rb.draw(&sh.buttons, c.tick);
        }
        self.exit.draw(&sh.buttons, c.tick);
        self.build.draw(&sh.buttons, c.tick);
        self.plan.draw(&sh.buttons, c.tick);
        switch (self.card) {
            .none => {},
            .building => |id| castle_ui.buildingCard(c, sh, self.card_pos, m, id),
            .group => |g| castle_ui.groupCard(c, sh, self.card_pos, self.rules, g),
            .item => |id| castle_ui.itemCard(c, sh, self.card_pos, id, pr.items[@intCast(id)].durability, null, 0, .{ 0, 0 }, .{ 0, 0 }),
        }
        self.hint.draw(c, &sh.iface);
    }

    /// 98String with a phrase across the window.
    fn drawString(self: *const BuildInfo, c: *Ctx, at: f32, ph: usize) void {
        const s = self.sh.iface.texIndex(gui.string);
        gfx.drawTex(s, self.x + self.w / 2 - @floor(gfx.texW(s) / 2), self.y + at);
        c.font1.drawCentered(c.proto.phrase(ph), self.x + self.w / 2, self.y + at + 9, castle_ui.color1);
    }

    /// The state line at y 120: built, can be built, no money, built this
    /// turn already, never (restricted), not yet (needs).
    fn drawState(self: *const BuildInfo, c: *Ctx) void {
        const m = self.m;
        const h = m.holdings();
        const r = m.ruler();
        const price = castle.innerPrice(m, self.id);
        const cx = self.x + self.w / 2;
        const cy = self.y + 120;
        const pr = c.proto;
        if (self.rules.built(self.id)) {
            c.font1.drawCentered(pr.phrase(244), cx, cy, castle_ui.color1);
            return;
        }
        var ph: usize = 247;
        var col: rl.Color = castle_ui.short_red;
        if (self.rules.canBuild(self.id)) {
            if (h.can_inner_bld <= 0) {
                ph = 249;
            } else if (r.gold >= price[0] and r.gem >= price[1]) {
                ph = 245;
                col = green;
            } else ph = 246;
        } else if (self.rules.isRestricted(self.id)) ph = 248;
        c.font1.drawCentered(pr.phrase(ph), cx, cy, col);
        c.font1.drawCentered(pr.phrase(ph), cx + 1, cy + 1, castle_ui.color1);
    }
};

test "slot coordinates" {
    try std.testing.expectEqual([2]f32{ 95 + dx * 6, 80 }, slot_xy[10]);
    try std.testing.expectEqual([2]f32{ 150 + dx * 5, 346 }, slot_xy[35]);
    try std.testing.expectEqual([2]f32{ 323 + dx * 2, 629 }, slot_xy[62]);
}
