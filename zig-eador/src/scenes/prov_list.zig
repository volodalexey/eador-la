//! ScreenProvList (ProvList.cpp:514) with GUI_ProvBar (:5): the player's
//! provinces, ten bars a page, sorted by one of ten keys (1..0) in either
//! order, shown as statistics or as the special places.  The bar's select
//! button closes the screen with the province (the map centres on it).
//!
//! A click on the resource, an outer building or the guard opens
//! WindowResourceInfo, WindowOuterBldInfo or WindowDefenderInfo (with the
//! guard's upkeep when the type has none of its own, ProvList.cpp:448).
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Button = @import("../ui/button.zig").Button;
const Mask = @import("../ui/mask.zig").Mask;
const Hint = @import("../ui/hint.zig").Hint;
const WindowInfo = @import("window_info.zig").WindowInfo;
const info_windows = @import("info_windows.zig");
const CostWindow = info_windows.CostWindow;
const ResourceWindow = @import("army_info.zig").ResourceWindow;
const statistics = @import("../game/statistics.zig");
const player = @import("../game/player.zig");
const mapgen = @import("../game/mapgen.zig");
const Shard = mapgen.Shard;
const province = @import("../game/province.zig");
const Province = province.Province;
const economy = @import("../game/economy.zig");
const movement = @import("../game/movement.zig");
const proto_mod = @import("../data/proto.zig");

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };
const red: rl.Color = .{ .r = 220, .g = 20, .b = 20, .a = 255 };
const half: rl.Color = .{ .r = 255, .g = 255, .b = 255, .a = 128 };

/// MAX_PROV, MAX_MODE, GUI_ProvBar::MAX_SITE, MAX_PROVINCE_BLD.
const rows = 10;
const modes = 10;
const max_site = 12;
const max_bld = province.max_bld;
const bar_h = 70;
const bar_y0 = 17;

/// stuff.dat objects.
const st = struct {
    const terra = 8;
    const res_none = 13;
    const count1 = 32;
    const count2 = 33;
    const dark1 = 40;
    const dark2 = 41;
    const level = 50;
    const mood = 51;
    const prov_bar = 68;
    const home = 69;
    const screen = 70;
    const sites_bar = 71;
    const count3 = 72;
    const between = 73;
    const under = 74;
    const site_hint = 76;
};
/// interface.dat: bmpGUI_SpellMask and bmpGUI_SpellSelect.
const gui_spell_mask = 0;
const gui_spell_select = 1;
/// innerbld.dat 149: the castle shown for the home province.
const bld_castle = 149;
/// CompareBldParam1(26, terrain): the player sees the terrain's hidden
/// resources.
const ab_hidden_resource = 26;
/// MAX_UNREST.
const max_unrest = 500;

const Spec = struct { x: f32, y: f32, n: usize, inactive: usize, hint: u32 = 0, key: ?rl.KeyboardKey = null };

const mode_keys = [modes]rl.KeyboardKey{ .one, .two, .three, .four, .five, .six, .seven, .eight, .nine, .zero };

/// The buttons, in the order of `ProvList.buttonList`.
const n_buttons = 17 + 3 * rows;

fn specs() [n_buttons]Spec {
    var out: [n_buttons]Spec = undefined;
    out[0] = .{ .x = 959, .y = 718, .n = 256, .inactive = 258, .hint = 135, .key = .escape };
    for (0..modes) |i| {
        const n = 345 + 3 * i;
        out[1 + i] = .{ .x = 29 + 54 * @as(f32, @floatFromInt(i)), .y = 718, .n = n, .inactive = n + 2, .hint = @intCast(534 + i), .key = mode_keys[i] };
    }
    out[11] = .{ .x = 586, .y = 718, .n = 325, .inactive = 327, .hint = 544 };
    out[12] = .{ .x = 640, .y = 718, .n = 328, .inactive = 330, .hint = 545 };
    out[13] = .{ .x = 711, .y = 718, .n = 331, .inactive = 333, .hint = 546 };
    out[14] = .{ .x = 765, .y = 718, .n = 334, .inactive = 336, .hint = 547 };
    out[15] = .{ .x = 836, .y = 718, .n = 337, .inactive = 340, .hint = 548, .key = .page_up };
    out[16] = .{ .x = 890, .y = 718, .n = 341, .inactive = 344, .hint = 549, .key = .page_down };
    for (0..rows) |i| {
        const y = barY(i);
        out[17 + 3 * i] = .{ .x = 240, .y = y + 17, .n = 236, .inactive = 288 };
        out[18 + 3 * i] = .{ .x = 942, .y = y + 17, .n = 233, .inactive = 287 };
        out[19 + 3 * i] = .{ .x = 964, .y = y + 3, .n = 322, .inactive = 324, .hint = 550 };
    }
    return out;
}

fn barY(i: usize) f32 {
    return bar_y0 + bar_h * @as(f32, @floatFromInt(i));
}

/// GUI_ProvBar's masks; the buttons are in ProvList.buttonList.
const Bar = struct {
    /// The province shown, 0 for a hidden bar.
    prov: i32 = 0,
    /// Shift: the page of the special places.
    shift: usize = 0,
    terra: Mask,
    gold: Mask,
    gem: Mask,
    level: Mask,
    mood: Mask,
    home: Mask,
    res: Mask,
    race: Mask,
    guard: Mask,
    fort: Mask,
    site: Mask,
    build: [max_bld]Mask,
    sites: [max_site]Mask,

    fn init(y: f32) Bar {
        var b: Bar = .{
            .terra = .rect(24, y + 32, 211, 24),
            .gold = .rect(250, y + 6, 88, 23),
            .gem = .rect(250, y + 33, 88, 23),
            .level = .rect(457, y + 4, 148, 25),
            .mood = .rect(457, y + 32, 148, 25),
            .home = .rect(610, y + 3, 170, 55),
            .res = .{ .x = 342 - 6, .y = y - 1 },
            .race = .{ .x = 401 - 6, .y = y - 1 },
            .guard = .{ .x = 786 - 6, .y = y - 1 },
            .fort = .{ .x = 846 - 6, .y = y - 1 },
            .site = .{ .x = 906 - 6, .y = y - 1 },
            .build = undefined,
            .sites = undefined,
        };
        for (&b.build, 0..) |*m, i| m.* = .{ .x = 612 - 6 + 57 * @as(f32, @floatFromInt(i)), .y = y - 1 };
        for (&b.sites, 0..) |*m, i| m.* = .{ .x = 260 + 57 * @as(f32, @floatFromInt(i)) - 6, .y = y - 1 };
        return b;
    }
};

/// The sort keys of a province (Tlist).
const Entry = struct { prov: i32, keys: [modes]i32 };

/// MakeSiteHint: the card of site slot `slot` of province `prov`.
const SiteCard = struct { prov: i32, slot: usize };

const sheet_names = .{
    .{ "iface", "interface" }, .{ "buttons", "buttons" }, .{ "stuff", "stuff" },
    .{ "races", "races" },     .{ "resource", "resource" }, .{ "site", "site" },
    .{ "outbuild", "outbuild" }, .{ "defenders", "defenders" }, .{ "innerbld", "innerbld" },
};

pub const ProvList = struct {
    s: *Shard,
    k: usize,
    /// Result: the picked province (left as it is on exit).
    out: *i32,
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    stuff: gfx.Sheet,
    races: gfx.Sheet,
    resource: gfx.Sheet,
    site: gfx.Sheet,
    outbuild: gfx.Sheet,
    defenders: gfx.Sheet,
    innerbld: gfx.Sheet,
    /// bmpGUI_SpellMask for the picture masks.
    shape: rl.Image,
    exit: Button,
    mode_b: [modes]Button,
    sort1: Button,
    sort2: Button,
    type1: Button,
    type2: Button,
    pg_up: Button,
    pg_dn: Button,
    lefts: [rows]Button,
    rights: [rows]Button,
    selects: [rows]Button,
    bars: [rows]Bar,
    entries: []Entry,
    /// Plist[i][1]: the provinces in the shown order.
    order: []i32,
    mode: usize = 0,
    /// bOrder: the largest first.
    desc: bool = true,
    /// Bars[i].Mode: the special places.
    sites_view: bool = false,
    shift: usize = 0,
    hint: Hint = .{},
    card: ?SiteCard = null,
    old_cursor: u32,
    /// The text of the resource hint.
    res_buf: [128]u8 = undefined,

    pub fn create(c: *Ctx, s: *Shard, k: usize, out: *i32) !scene.Scene {
        const self = try c.gpa.create(ProvList);
        errdefer c.gpa.destroy(self);
        self.* = undefined;
        self.s = s;
        self.k = k;
        self.out = out;
        self.mode = 0;
        self.desc = true;
        self.sites_view = false;
        self.shift = 0;
        self.hint = .{};
        self.card = null;
        self.old_cursor = c.cursor;
        var loaded: usize = 0;
        errdefer inline for (sheet_names, 0..) |e, i| {
            if (i < loaded) @field(self, e[0]).deinit();
        };
        inline for (sheet_names) |e| {
            @field(self, e[0]) = try gfx.Sheet.load(c.fs, e[1]);
            loaded += 1;
        }
        self.shape = self.iface.imageIndex(gui_spell_mask) orelse return error.MissingMask;
        errdefer rl.unloadImage(self.shape);
        const sp = specs();
        const list = self.buttonList();
        var made: usize = 0;
        errdefer for (list[0..made]) |b| b.deinit();
        for (list, sp) |b, x| {
            b.* = try Button.init(&self.buttons, x.x, x.y, x.n, x.n + 1, x.n + 2);
            b.inactive = x.inactive;
            b.hint = x.hint;
            b.hotkey = x.key;
            made += 1;
        }
        self.mode_b[0].on = false;
        self.sort1.on = false;
        self.type1.on = false;
        for (&self.bars, 0..) |*b, i| b.* = .init(barY(i));
        try self.collect(c);
        errdefer {
            c.gpa.free(self.entries);
            c.gpa.free(self.order);
        }
        self.arrange();
        c.setCursor(0);
        return scene.Scene.of(ProvList, self, false);
    }

    pub fn deinit(self: *ProvList, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        c.gpa.free(self.entries);
        c.gpa.free(self.order);
        for (self.buttonList()) |b| b.deinit();
        rl.unloadImage(self.shape);
        inline for (sheet_names) |e| @field(self, e[0]).deinit();
    }

    fn buttonList(self: *ProvList) [n_buttons]*Button {
        var out: [n_buttons]*Button = undefined;
        out[0] = &self.exit;
        for (&self.mode_b, 0..) |*b, i| out[1 + i] = b;
        out[11] = &self.sort1;
        out[12] = &self.sort2;
        out[13] = &self.type1;
        out[14] = &self.type2;
        out[15] = &self.pg_up;
        out[16] = &self.pg_dn;
        for (0..rows) |i| {
            out[17 + 3 * i] = &self.lefts[i];
            out[18 + 3 * i] = &self.rights[i];
            out[19 + 3 * i] = &self.selects[i];
        }
        return out;
    }

    /// CurrentPlayer for WindowResourceInfo.
    fn market(self: *const ProvList, c: *const Ctx) player.Market {
        return .{ .s = self.s, .pr = c.proto, .player = @intCast(self.k) };
    }

    fn eco(self: *const ProvList, c: *const Ctx) economy.Economy {
        return .{ .s = self.s, .pr = c.proto, .max_profit = c.cfg.max_profit };
    }

    /// Plist and Tlist: the provinces of the player and their keys.  Key 0
    /// is 100 - the turns from the capital (MoveMap with MovePlayer 0,
    /// WalkType 10, WalkSpeed 1), 101 for the capital, -100 out of reach.
    fn collect(self: *ProvList, c: *Ctx) !void {
        const s = self.s;
        const pl: i32 = @intCast(self.k);
        var w = try movement.Walker.init(c.gpa, s, c.proto, c.cfg.max_path);
        defer w.deinit(c.gpa);
        w.clear();
        w.player = 0;
        w.walk_type = 10;
        w.walk_speed = 1;
        const home = s.capitals[self.k];
        if (home > 0) w.moveMap(home, -1, 1, 0, 0);
        var n: usize = 0;
        for (s.provinces[1..]) |*pv| n += @intFromBool(pv.player == pl);
        self.entries = try c.gpa.alloc(Entry, n);
        errdefer c.gpa.free(self.entries);
        self.order = try c.gpa.alloc(i32, n);
        const e = self.eco(c);
        var j: usize = 0;
        for (s.provinces[1..], 1..) |*pv, i| {
            if (pv.player != pl) continue;
            const mv = w.move[i];
            var sites: i32 = 0;
            for (pv.sites) |site| sites += @intFromBool(site.index > 0 and site.pos <= pv.explore);
            var blds: i32 = 0;
            for (pv.buildings[0..max_bld]) |b| blds += @intFromBool(b > 0);
            self.entries[j] = .{ .prov = pv.number, .keys = .{
                if (mv < 0) -100 else if (pv.number == home) 101 else 100 - mv,
                e.goldIncome(pv),
                e.gemIncome(pv),
                pv.population,
                e.mood(pv),
                pv.unrest[@intCast(std.math.clamp(pv.player, 0, province.max_players - 1))],
                pv.explore,
                pv.race,
                sites,
                blds,
            } };
            j += 1;
        }
    }

    /// bArrange: the first largest (or smallest) left each pass, so equal
    /// keys keep the province order.
    fn arrange(self: *ProvList) void {
        const idx = self.order;
        for (self.entries, 0..) |en, i| idx[i] = en.prov;
        const Cmp = struct {
            list: *const ProvList,
            fn key(ctx: @This(), prov: i32) i32 {
                for (ctx.list.entries) |en| if (en.prov == prov) return en.keys[ctx.list.mode];
                return 0;
            }
            fn before(ctx: @This(), a: i32, b: i32) bool {
                return if (ctx.list.desc) key(ctx, a) > key(ctx, b) else key(ctx, a) < key(ctx, b);
            }
        };
        std.sort.insertion(i32, idx, Cmp{ .list = self }, Cmp.before);
        self.redraw();
    }

    /// bRedraw: the bars of the page (Activate resets their site page) and
    /// the page buttons.
    fn redraw(self: *ProvList) void {
        for (&self.bars, 0..) |*b, i| {
            b.prov = if (self.shift + i < self.order.len) self.order[self.shift + i] else 0;
            b.shift = 0;
        }
        self.pg_dn.on = self.shift + rows < self.order.len;
        self.pg_up.on = self.shift > 0;
    }

    pub fn update(self: *ProvList, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        self.card = null;
        const p = f.pointer;
        for (&self.bars, 0..) |*b, i| {
            if (b.prov <= 0) {
                self.lefts[i].enabled = false;
                self.rights[i].enabled = false;
                self.selects[i].enabled = false;
                continue;
            }
            switch (self.checkBar(c, b, i, f)) {
                .none => {},
                .action => |a| return a,
                .pick => {
                    self.out.* = b.prov;
                    return .pop;
                },
            }
        }
        for (&self.mode_b, 0..) |*b, i| {
            if (!b.check(f) or i == self.mode) continue;
            self.mode_b[self.mode].on = true;
            self.mode = i;
            b.on = false;
            self.shift = 0;
            self.arrange();
        }
        if (self.type1.check(f) or self.type2.check(f)) {
            self.sites_view = !self.sites_view;
            self.type1.on = self.sites_view;
            self.type2.on = !self.sites_view;
            self.redraw();
        }
        if (self.sort1.check(f) or self.sort2.check(f)) {
            self.desc = !self.desc;
            self.sort1.on = !self.desc;
            self.sort2.on = self.desc;
            self.shift = 0;
            self.arrange();
        }
        if (self.pg_dn.check(f)) {
            self.shift += rows;
            self.redraw();
        }
        if (self.pg_up.check(f)) {
            self.shift -|= rows;
            self.redraw();
        }
        if (p.wheel < 0 and self.shift + rows < self.order.len) {
            self.shift += 1;
            self.redraw();
        }
        if (p.wheel > 0 and self.shift > 0) {
            self.shift -= 1;
            self.redraw();
        }
        if (self.exit.check(f)) return .pop;
        for (self.buttonList()) |b| if (b.hot() and b.hint > 0) self.hint.phrase(c, b.hint, p.pos);
        return .none;
    }

    const BarResult = union(enum) { none, pick, action: scene.Action };

    fn stringHint(self: *ProvList, s: []const u8, p: input.Pointer) void {
        self.hint.what = .{ .string = s };
        self.hint.pos = p.pos;
    }

    fn info(c: *Ctx, title: u32, msg: usize, pic: usize) scene.Action {
        const body = if (msg < c.proto.messages.len) c.proto.messages[msg] else "";
        return .{ .push = WindowInfo.create(c, c.proto.phrase(title), body, pic) catch return .none };
    }

    fn shapeCheck(self: *ProvList, m: *Mask, p: input.Pointer) @import("../ui/mask.zig").Result {
        m.shape = &self.shape;
        return m.check(p);
    }

    /// GUI_ProvBar::Check.
    fn checkBar(self: *ProvList, c: *Ctx, b: *Bar, i: usize, f: *const input.Frame) BarResult {
        const pr = c.proto;
        const p = f.pointer;
        const pv = &self.s.provinces[@intCast(b.prov)];
        if (b.terra.check(p) != .none) return .{ .action = info(c, 400, 30, st.terra) };
        if (b.terra.hot()) self.hint.phrase(c, 527, p.pos);
        var result: BarResult = .none;
        if (!self.sites_view) {
            self.lefts[i].enabled = false;
            self.rights[i].enabled = false;
            if (b.mood.check(p) != .none) result = .{ .action = info(c, 401, 31, st.mood) };
            if (b.mood.hot()) self.hint.phrase(c, 401, p.pos);
            if (b.level.check(p) != .none) result = .{ .action = info(c, 403, 32, st.level) };
            if (b.level.hot()) self.hint.phrase(c, 403, p.pos);
            _ = b.gold.check(p);
            if (b.gold.hot()) self.hint.phrase(c, 405, p.pos);
            _ = b.gem.check(p);
            if (b.gem.hot()) self.hint.phrase(c, 406, p.pos);

            const race = std.math.clamp(pv.race, 0, @as(i32, @intCast(pr.races.len)) - 1);
            if (self.shapeCheck(&b.race, p) != .none) {
                const rc = &pr.races[@intCast(race)];
                result = .{ .action = .{ .push = WindowInfo.createFrom(c, rc.name, rc.text, "races", @intCast(@max(race - 1, 0))) catch return .none } };
            }
            if (b.race.hot()) self.stringHint(pr.races[@intCast(race)].name, p);

            const res = self.resourceState(c, pv);
            const r_check = self.shapeCheck(&b.res, p);
            if (b.res.hot()) {
                const buf = &self.res_buf;
                self.stringHint(switch (res.kind) {
                    .free => pr.resources[@intCast(res.id)].name,
                    .guarded => std.fmt.bufPrint(buf, "{s} ({s})", .{ pr.resources[@intCast(res.id)].name, pr.phrase(530) }) catch "",
                    .hidden => std.fmt.bufPrint(buf, "{s} ({s})", .{ pr.resources[@intCast(res.id)].name, pr.phrase(529) }) catch "",
                    .none => pr.phrase(26),
                }, p);
            }
            if (r_check != .none) result = if (res.kind == .none)
                .{ .action = info(c, 26, 34, st.res_none) }
            else
                .{ .action = .{ .push = ResourceWindow.create(c, res.id, self.market(c)) catch return .none } };

            const home = self.s.capitals[@intCast(pv.player)] == pv.number;
            if (home) {
                _ = b.home.check(p);
                if (b.home.hot()) self.hint.phrase(c, 528, p.pos);
            } else for (&b.build, 0..) |*m, k| {
                const bc = self.shapeCheck(m, p);
                const bld = nthBuilding(pv, k);
                if (bc != .none and bld > 0 and bld < pr.outer_buildings.len) result = .{ .action = .{ .push = CostWindow.outer(c, bld, self.market(c)) catch return .none } };
                if (!m.hot()) continue;
                if (bld > 0 and bld < pr.outer_buildings.len) self.stringHint(pr.outer_buildings[@intCast(bld)].name, p) else self.hint.phrase(c, 531, p.pos);
            }

            if (self.shapeCheck(&b.guard, p) != .none and pv.defender > 0 and pv.defender < pr.defenders.len) {
                const pay: statistics.Pair = if (pr.defenders[@intCast(pv.defender)].gold_payment < 0) statistics.defenderPayment(self.s, pr, pv.guard) else .{};
                result = .{ .action = .{ .push = CostWindow.defender(c, pv.defender, pay.gold, pay.gem, self.market(c)) catch return .none } };
            }
            if (b.guard.hot()) {
                if (pv.defender > 0 and pv.defender < pr.defenders.len) self.stringHint(pr.defenders[@intCast(pv.defender)].name, p) else self.hint.phrase(c, 532, p.pos);
            }
            _ = self.shapeCheck(&b.site, p);
            if (b.site.hot()) self.hint.phrase(c, 533, p.pos);
            _ = self.shapeCheck(&b.fort, p);
            if (b.fort.hot()) self.hint.phrase(c, if (home or walled(pr, pv)) 117 else 38, p.pos);
        } else {
            const sites = siteSlots(pv, b.shift);
            for (b.sites[0..sites.limit], sites.slots[0..sites.limit]) |*m, slot| {
                _ = self.shapeCheck(m, p);
                if (m.hot()) self.card = .{ .prov = b.prov, .slot = slot };
            }
            const more = sites.total > max_site;
            self.lefts[i].enabled = more;
            self.rights[i].enabled = more;
            self.lefts[i].on = b.shift > 0;
            self.rights[i].on = sites.total > (b.shift + 1) * max_site;
            if (more) {
                if (self.lefts[i].check(f)) b.shift -|= 1;
                if (self.rights[i].check(f)) b.shift += 1;
            }
        }
        self.selects[i].enabled = true;
        if (self.selects[i].check(f)) return .pick;
        return result;
    }

    const ResState = struct { kind: enum { none, free, guarded, hidden } = .none, id: i32 = 0 };

    /// GetResource(1), (0) and (-1) with CompareBldParam1(26, terrain).
    fn resourceState(self: *const ProvList, c: *const Ctx, pv: *const Province) ResState {
        const e = self.eco(c);
        const n: i32 = @intCast(c.proto.resources.len);
        const ok = struct {
            fn f(r: i32, max: i32) bool {
                return r > 0 and r < max;
            }
        }.f;
        const r1 = e.resource(pv, 1);
        if (ok(r1, n)) return .{ .kind = .free, .id = r1 };
        const r0 = e.resource(pv, 0);
        if (ok(r0, n)) return .{ .kind = .guarded, .id = r0 };
        const rh = e.resource(pv, -1);
        if (ok(rh, n) and e.compareBldParam1(pv.player, ab_hidden_resource, pv.terrain)) return .{ .kind = .hidden, .id = rh };
        return .{};
    }

    pub fn draw(self: *ProvList, c: *Ctx) void {
        rl.clearBackground(.black);
        gfx.drawTex(self.stuff.texIndex(st.screen), 0, 0);
        for ([_]f32{ 11, 568, 693, 818, 943 }) |x| gfx.drawTex(self.stuff.texIndex(st.between), x, 715);
        gfx.drawTex(self.stuff.texIndex(st.under), 959, 715);
        for (0..modes) |i| gfx.drawTex(self.stuff.texIndex(st.under), 27 + 54 * @as(f32, @floatFromInt(i)), 715);
        for ([_]f32{ 584, 638, 709, 763, 834, 888 }) |x| gfx.drawTex(self.stuff.texIndex(st.under), x, 715);
        for (&self.bars) |*b| {
            if (b.prov <= 0) continue;
            if (self.sites_view) self.drawSites(c, b) else self.drawStats(c, b);
        }
        for (self.buttonList()) |b| b.draw(&self.buttons, c.tick);
        if (self.card) |cd| self.drawCard(c, cd) else self.hint.draw(c, &self.iface);
    }

    fn glow(self: *ProvList, m: *const Mask) void {
        if (m.hot()) gfx.drawTex(self.iface.texIndex(gui_spell_select), m.x, m.y);
    }

    /// The name and the terrain with the explored share (red below the
    /// level's need).
    fn drawHeader(self: *ProvList, c: *Ctx, pv: *const Province, y: f32) void {
        const pr = c.proto;
        c.font1.drawCentered(pv.name, 129, y + 12, color1);
        var buf: [96]u8 = undefined;
        const t = if (pv.terrain >= 0 and pv.terrain < pr.terrains.len) pr.terrains[@intCast(pv.terrain)].name else "";
        const lv = self.eco(c).level(pv.population);
        const need = pr.province_levels[@intCast(lv)].explored;
        c.font1.drawCentered(std.fmt.bufPrint(&buf, "{s} {d}~", .{ t, pv.explore }) catch "", 129, y + 41, if (pv.explore >= need) color1 else red);
    }

    /// GUI_ProvBar::Show, Mode 0.
    fn drawStats(self: *ProvList, c: *Ctx, b: *Bar) void {
        const pr = c.proto;
        const pv = &self.s.provinces[@intCast(b.prov)];
        const y = b.res.y + 1;
        const e = self.eco(c);
        gfx.drawTex(self.stuff.texIndex(st.prov_bar), 0, y);
        self.drawHeader(c, pv, y);
        if (pv.race > 0) gfx.drawTex(self.races.texIndex(@intCast(pv.race - 1)), 401, y + 5);

        const res = self.resourceState(c, pv);
        const big: usize = pr.resources.len -| 1;
        const res_pic = if (res.id > 0) self.resource.texIndex(big + @as(usize, @intCast(res.id)) - 1) else null;
        switch (res.kind) {
            .free => gfx.drawTex(res_pic, 342, y + 5),
            .guarded => {
                gfx.drawTex(res_pic, 342, y + 5);
                rl.drawRectangle(342, @intFromFloat(y + 5), 52, 52, .{ .r = 255, .g = 0, .b = 0, .a = 128 });
            },
            .hidden => {
                gfx.drawTex(res_pic, 342, y + 5);
                rl.drawRectangle(342, @intFromFloat(y + 5), 52, 52, .{ .r = 0, .g = 0, .b = 0, .a = 128 });
                gfx.drawTex2(self.stuff.texIndex(st.dark1), 342, y + 5, half);
                gfx.drawTex2(self.stuff.texIndex(st.dark2), 342, y + 5, half);
            },
            .none => gfx.drawTex2(self.stuff.texIndex(st.res_none), 342, y + 5, half),
        }

        const owner: usize = @intCast(std.math.clamp(pv.player, 0, mapgen.max_players));
        var buf: [32]u8 = undefined;
        if (self.s.capitals[owner] == pv.number) {
            gfx.drawTex(self.stuff.texIndex(st.home), 610, y + 3);
            gfx.drawTex(self.innerbld.texIndex(bld_castle), 846, y + 5);
            const wall = self.s.rulers[owner].wall;
            if (wall != 0) self.iconString(c, percent(&buf, @divTrunc(100 * pv.fort, wall)), 845, y + 5);
        } else {
            var count: f32 = 0;
            for (pv.buildings[0..max_bld]) |bld| {
                if (bld <= 0 or bld >= pr.outer_buildings.len) continue;
                const pic = self.outbuild.texIndex(@intCast(bld - 1));
                gfx.drawTex(pic, 612 + count * 57, y + 5);
                count += 1;
                const wall = pr.outer_buildings[@intCast(bld)].wall;
                if (wall > 0) {
                    gfx.drawTex(pic, 846, y + 5);
                    self.iconString(c, percent(&buf, @divTrunc(100 * pv.fort, wall)), 845, y + 5);
                }
            }
        }
        if (pv.defender > 0) {
            gfx.drawTex(self.defenders.texIndex(@intCast(pv.defender - 1)), 786, y + 5);
            var max: i32 = 0;
            var cur: i32 = 0;
            if (pv.guard >= 0) for (self.s.guards.items[@intCast(pv.guard)].squad) |slot| {
                const u = slot orelse continue;
                max += u.maxLife(pr);
                cur += u.life;
            };
            if (max > 0) self.iconString(c, percent(&buf, @divTrunc(100 * cur, max)), 786, y + 5);
        }
        var n_sites: i32 = 0;
        var last: i32 = 0;
        for (pv.sites) |site| if (site.index > 0 and site.pos <= pv.explore) {
            last = site.index;
            n_sites += 1;
        };
        if (last > 0) {
            gfx.drawTex(self.site.texIndex(@intCast(last - 1)), 906, y + 5);
            self.iconNumber(c, n_sites, 906, y + 5);
        }

        c.font1.drawCentered(signed(&buf, e.goldIncome(pv)), 298, y + 12, color2);
        c.font1.drawCentered(signed(&buf, e.gemIncome(pv)), 298, y + 39, color2);

        const lv = e.level(pv.population);
        const levels = pr.province_levels;
        if (lv < proto_mod.max_province_level) {
            const w: i32 = if (pv.population <= 0)
                (10 - @rem(-pv.population, 10)) * 14
            else blk: {
                const lo = levels[@intCast(lv - 1)].max_population;
                const hi = levels[@intCast(lv)].max_population;
                break :blk if (hi > lo) @divTrunc(145 * (pv.population - lo), hi - lo) else 0;
            };
            var i: i32 = 0;
            while (i < w) : (i += 1) rl.drawRectangle(459 + i, @intFromFloat(y + 5), 1, 23, rgb(5 + @divTrunc(i, 5), 80 + i, 10 + @divTrunc(i, 10)));
        }
        c.font1.drawCentered(levels[@intCast(lv)].name, 533, y + 13, color1);
        c.font1.drawCentered(levels[@intCast(lv)].name, 532, y + 12, color2);

        const unrest = pv.unrest[@intCast(std.math.clamp(pv.player, 0, province.max_players - 1))];
        var i: i32 = 0;
        while (i < @divTrunc(145 * unrest, max_unrest)) : (i += 1) rl.drawRectangle(459 + i, @intFromFloat(y + 33), 1, 22, rgb(110 + i, 20 + @divTrunc(i, 7), 20 + @divTrunc(i, 7)));
        if (pv.population > 0) {
            const m = std.math.clamp(e.mood(pv), -5, 3);
            const text = pr.phrase(@intCast(80 + m));
            c.font1.drawCentered(text, 533, y + 40, color1);
            const col = if (m >= 0) rgb(200 - m * 20, 200 + m * 15, 20 + m * 40) else rgb(200 - m * 10, 200 + m * 40, 20 + m * 4);
            c.font1.drawCentered(text, 532, y + 39, col);
        } else {
            c.font1.drawCentered(pr.phrase(231), 533, y + 40, color1);
            c.font1.drawCentered(pr.phrase(231), 532, y + 39, color2);
        }

        for ([_]*const Mask{ &b.res, &b.race, &b.guard, &b.fort, &b.site }) |m| self.glow(m);
        if (self.s.capitals[owner] != pv.number) for (&b.build) |*m| self.glow(m);
    }

    /// GUI_ProvBar::Show, Mode 1: the explored special places, twelve a
    /// page.
    fn drawSites(self: *ProvList, c: *Ctx, b: *Bar) void {
        const pv = &self.s.provinces[@intCast(b.prov)];
        const y = b.res.y + 1;
        gfx.drawTex(self.stuff.texIndex(st.sites_bar), 0, y);
        self.drawHeader(c, pv, y);
        const sites = siteSlots(pv, b.shift);
        for (sites.slots[0..sites.limit], 0..) |slot, n| {
            gfx.drawTex(self.site.texIndex(@intCast(pv.sites[slot].index - 1)), 260 + 57 * @as(f32, @floatFromInt(n)), y + 5);
        }
        for (b.sites[0..sites.limit]) |*m| self.glow(m);
    }

    /// MakeSiteHint: the place's name, then its guard type or "no guard".
    fn drawCard(self: *ProvList, c: *Ctx, cd: SiteCard) void {
        const pr = c.proto;
        const bg = self.stuff.texIndex(st.site_hint) orelse return;
        const pos = self.hint.place(c, @floatFromInt(bg.width), @floatFromInt(bg.height));
        gfx.drawTex(bg, pos.x, pos.y);
        const pv = &self.s.provinces[@intCast(cd.prov)];
        const site = pv.sites[cd.slot];
        if (site.index > 0 and site.index < pr.sites.len) c.font1.drawCentered(pr.sites[@intCast(site.index)].name, pos.x + 143, pos.y + 14, color1);
        if (pv.site_guards[cd.slot] >= 0) {
            var buf: [64]u8 = undefined;
            c.font1.drawCentered(std.fmt.bufPrint(&buf, "{s}:", .{pr.phrase(25)}) catch "", pos.x + 143, pos.y + 39, color2);
            const g = site.guard;
            if (g >= 0 and g < pr.guard_types.len) c.font1.drawCentered(pr.guard_types[@intCast(g)].name, pos.x + 143, pos.y + 64, color2);
        } else c.font1.drawCentered(pr.phrase(532), pos.x + 146, pos.y + 39, color2);
    }

    /// ShowIconString: the 072Count3 plate under a picture at (x, y).
    fn iconString(self: *ProvList, c: *Ctx, s: []const u8, x: f32, y: f32) void {
        const t = self.stuff.texIndex(st.count3) orelse return;
        const len = @round(c.font1.measure(s));
        const h: f32 = @floatFromInt(t.height);
        if (len > 40) {
            gfx.drawTex(t, x + 7, y + 32);
            c.font1.drawCentered(s, x + 32, y + 36, color1);
            return;
        }
        rl.drawTextureRec(t, .{ .x = 0, .y = 0, .width = len + 2, .height = h }, .{ .x = x + 7 + (40 - len), .y = y + 32 }, .white);
        rl.drawTextureRec(t, .{ .x = @as(f32, @floatFromInt(t.width)) - 2, .y = 0, .width = 2, .height = h }, .{ .x = x + 49, .y = y + 32 }, .white);
        c.font1.drawCentered(s, x + 49 - @floor(len / 2), y + 36, color1);
    }

    /// ShowIconNumber.
    fn iconNumber(self: *ProvList, c: *Ctx, n: i32, x: f32, y: f32) void {
        var buf: [16]u8 = undefined;
        const s = std.fmt.bufPrint(&buf, "{d}", .{n}) catch return;
        if (n < 10) {
            gfx.drawTex(self.stuff.texIndex(st.count1), x + 37, y + 32);
            c.font1.draw(s, x + 40, y + 36, color1);
        } else {
            gfx.drawTex(self.stuff.texIndex(st.count2), x + 29, y + 33);
            c.font1.drawCentered(s, x + 39, y + 37, color1);
        }
    }
};

/// The explored sites from page `shift` (arSite, SiteLimit, Sites).
fn siteSlots(pv: *const Province, shift: usize) struct { slots: [max_site]usize, limit: usize, total: usize } {
    var out: [max_site]usize = undefined;
    var limit: usize = 0;
    var total: usize = 0;
    for (pv.sites, 0..) |site, k| {
        if (site.index <= 0 or site.pos > pv.explore) continue;
        if (limit < max_site and total >= shift * max_site) {
            out[limit] = k;
            limit += 1;
        }
        total += 1;
    }
    return .{ .slots = out, .limit = limit, .total = total };
}

/// The `n`-th outer building of the province, 0 past the last.
fn nthBuilding(pv: *const Province, n: usize) i32 {
    var count: usize = 0;
    for (pv.buildings[0..max_bld]) |b| {
        if (b <= 0) continue;
        if (count == n) return b;
        count += 1;
    }
    return 0;
}

fn walled(pr: *const proto_mod.Proto, pv: *const Province) bool {
    for (pv.buildings[0..max_bld]) |b| {
        if (b > 0 and b < pr.outer_buildings.len and pr.outer_buildings[@intCast(b)].wall > 0) return true;
    }
    return false;
}

/// makecol: Allegro keeps the low 8 bits of each component.
fn rgb(r: i32, g: i32, b: i32) rl.Color {
    return .{ .r = @truncate(@as(u32, @bitCast(r))), .g = @truncate(@as(u32, @bitCast(g))), .b = @truncate(@as(u32, @bitCast(b))), .a = 255 };
}

fn signed(buf: []u8, v: i32) []const u8 {
    return (if (v > 0) std.fmt.bufPrint(buf, "+{d}", .{v}) else std.fmt.bufPrint(buf, "{d}", .{v})) catch "";
}

/// "%d~": the font draws "~" as the percent sign.
fn percent(buf: []u8, v: i32) []const u8 {
    return std.fmt.bufPrint(buf, "{d}~", .{v}) catch "";
}

test "sites by page" {
    var pv: Province = .{ .explore = 50 };
    for (0..14) |k| pv.sites[k] = .{ .index = @intCast(k + 1), .pos = 10 };
    pv.sites[3].pos = 60;
    const a = siteSlots(&pv, 0);
    try std.testing.expectEqual(@as(usize, 13), a.total);
    try std.testing.expectEqual(@as(usize, 12), a.limit);
    try std.testing.expectEqual(@as(usize, 4), a.slots[3]);
    const b = siteSlots(&pv, 1);
    try std.testing.expectEqual(@as(usize, 1), b.limit);
    try std.testing.expectEqual(@as(usize, 13), b.slots[0]);
}

test "makecol keeps the low bits" {
    try std.testing.expectEqual(@as(u8, 4), rgb(260, 0, 0).r);
}
