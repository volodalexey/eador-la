//! The province panel of the map (40InfoBar), MainProcess.cpp:2680-4087
//! and the Condition 0 → 1 setup at :5052.  Mode `main` is Condition 1:
//! the arms of the owner, name, terrain, type or exploration, resource,
//! income, race, population and unrest bars, mood, level and the bottom
//! row (defender with its life, outer buildings, fort, sites).  The other
//! modes are the pages behind its buttons and masks:
//!   build_list / build (2, 3)   buttBuild: BuildList, ConstructBuilding;
//!   raze_list / raze (4, 5)     mskMain[1]: RazeList, RazeBuilding;
//!   hire_list / hire (6, 7)     buttGuard: HireList, the hire;
//!   guard (8)                   mskMain[0]: the defender, buttDismiss;
//!   site_list / site (9, 10)    mskMain[3]: SiteList and the description;
//!   fort (11)                   mskMain[2]: the walls and towers;
//!   ritual_list / ritual (12, 13) buttRitual: RitualList, CastRitual;
//!   enchant (14)                mskBroken: EnchantList.
//! The tutorial hints and blinks of these pages are not ported.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const mapgen = @import("../game/mapgen.zig");
const province = @import("../game/province.zig");
const Province = province.Province;
const economy = @import("../game/economy.zig");
const player_mod = @import("../game/player.zig");
const ritual = @import("../game/ritual.zig");
const order = @import("../game/prov_order.zig");
const statistics = @import("../game/statistics.zig");
const movement = @import("../game/movement.zig");
const event_mod = @import("../game/event.zig");
const proto_mod = @import("../data/proto.zig");
const heraldry = @import("../ui/heraldry.zig");
const Button = @import("../ui/button.zig").Button;
const Mask = @import("../ui/mask.zig").Mask;
const Hint = @import("../ui/hint.zig").Hint;
const hero_ui = @import("../ui/hero_ui.zig");
const wi = @import("window_info.zig");
const WindowInfo = wi.WindowInfo;
const info_windows = @import("info_windows.zig");
const CostWindow = info_windows.CostWindow;
const ResourceWindow = @import("army_info.zig").ResourceWindow;
const Dialog = @import("dialog.zig").Dialog;
const Player = @import("main_screen.zig").Player;

/// BarX, BarY.
const bar_x = 707;
const bar_y = 243;
const bar_size = order.bar_size;

/// interface.dat.
const gui = struct {
    const spell_mask = 0;
    const spell_select = 1;
    const no_res = 30;
    const province_bar = 40;
    const build_bar = 41;
    const build_panel = 42;
    const build_select = 43;
    const build_mask = 44;
    const building = 45;
    const raze_panel = 46;
    const defender = 47;
    const site = 48;
    const fort = 49;
    const broken = 130;
};
/// stuff.dat.
const st = struct {
    const count1 = 32;
    const count2 = 33;
    const cooldown = 34;
    const timer = 35;
    const enchant = 42;
};
/// innerbld.dat 149: the castle of a home province.
const pic_castle = 149;
/// mskBroken and objEnchant (827, 224).
const broken_x = 827;
const broken_y = 224;

/// WindowInfo of the panel's fields (message, stuff.dat picture).
const msg_terrain = 43;
const stuff_terrain = 42;
const msg_no_res = 34;
const stuff_no_res = 13;
/// CompareBldParam1(26, terrain): the castle finds the terrain's hidden
/// resources.
const ab_hidden_resource = 26;
const ab_outer_garrison = 4;
/// Phrases.
const ph = struct {
    const explored = 24;
    const guard = 25;
    const no_res = 26;
    const fort_title = 35;
    const walls = 36;
    const tower = 37;
    const no_fort = 38;
    const mood = 80;
    const empty = 231;
};
/// dialog.var: raze (1), dismiss (2), hire over a defender (1165), raze
/// under siege (1243).
const dlg_raze = 1;
const dlg_dismiss = 2;
const dlg_rehire = 1165;
const dlg_raze_siege = 1243;
/// arMess: the confirmations of defenders (5) and razing (6).
const mess_defender = 5;
const mess_raze = 6;
/// unit_sound.dat: wavCoins, wavBuild.
const snd_coins = 155;
const snd_build = 157;
/// MakeAbilityHint(999): the defender's initiative.
const ab_initiative = 999;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };
const red: rl.Color = .{ .r = 220, .g = 20, .b = 20, .a = 255 };
const grey: rl.Color = .{ .r = 90, .g = 90, .b = 90, .a = 255 };

pub const Mode = enum { main, build_list, build, raze_list, raze, hire_list, hire, guard, site_list, site, fort, ritual_list, ritual, enchant };

/// What the map must do after the panel's update.
pub const Out = union(enum) {
    none,
    action: scene.Action,
    /// buttBuild in the capital: CastleMain.
    castle,
    /// buttGarrison: ScreenGarrison.
    garrison,
    /// buttRitual2: CastRitual(ritual, Member, -1).
    cast: i32,
    /// The province changed (DrawPic).
    changed,
};

/// What the panel reads of the map.
pub const View = struct {
    s: *mapgen.Shard,
    player: usize,
    /// pFog of the player.
    fog: []const u8,
    players: []const Player,
    arms: *heraldry.Heraldry,
    iface: *gfx.Sheet,
    buttons: *gfx.Sheet,
    hint: *Hint,
};

/// UI_Button2: a button that stays pressed (the list filters).
const Toggle = struct {
    b: Button,
    pressed_active: usize,
    press: bool = false,
    /// iHint1: the hint while pressed.
    hint1: u32 = 0,

    fn check(self: *Toggle, f: *const input.Frame) bool {
        if (!self.b.check(f)) return false;
        self.press = !self.press;
        return true;
    }

    fn draw(self: *const Toggle, sheet: *gfx.Sheet) void {
        if (!self.b.enabled) return;
        const i = if (!self.b.on) self.b.inactive orelse self.b.normal else switch (self.b.mask.state) {
            .visible => if (self.press) self.b.pressed else self.b.normal,
            else => if (self.press) self.pressed_active else self.b.active,
        };
        gfx.drawTex(sheet.texIndex(i), self.b.mask.x, self.b.mask.y);
    }

    fn hint(self: *const Toggle) u32 {
        return if (self.press) self.hint1 else self.b.hint;
    }
};

const Ask = enum { raze, hire, dismiss };

/// The text boxes of the pages: txtDs (buildings, rituals), txtDs1
/// (defenders), txtDs2 (sites).
const TextBox = enum { ds, ds1, ds2 };

const Sheets = struct {
    outbuild: gfx.Sheet,
    defenders: gfx.Sheet,
    innerbld: gfx.Sheet,
    races: gfx.Sheet,
    stuff: gfx.Sheet,
    site: gfx.Sheet,
    ritual: gfx.Sheet,
    resource: gfx.Sheet,

    const names = @typeInfo(Sheets).@"struct".field_names;

    fn load(c: *Ctx) !Sheets {
        var out: Sheets = undefined;
        var done: usize = 0;
        errdefer inline for (names, 0..) |n, i| {
            if (i < done) @field(out, n).deinit();
        };
        inline for (names) |n| {
            @field(out, n) = try gfx.Sheet.load(c.fs, n);
            done += 1;
        }
        return out;
    }

    fn deinit(self: *Sheets) void {
        inline for (names) |n| @field(self, n).deinit();
    }
};

const Buttons = struct {
    guard: Button,
    build: Button,
    garrison: Button,
    ritual: Button,
    scroll_up: Button,
    cancel: Button,
    scroll_down: Button,
    build2: Button,
    cancel2: Button,
    raze: Button,
    ritual2: Button,
    guard2: Button,
    dismiss: Button,
};

const Spec = struct { x: f32, n: [4]usize, hint: u32 = 0, key: ?rl.KeyboardKey = null };

/// MainProcess.cpp:1508-1645: x from BarX, all at BarY + 396; normal,
/// active, pressed and inactive pictures of buttons.dat.
const specs = std.enums.EnumFieldStruct(std.meta.FieldEnum(Buttons), Spec, null){
    .guard = .{ .x = 40, .n = .{ 64, 65, 66, 111 }, .hint = 92, .key = .q },
    .build = .{ .x = 100, .n = .{ 67, 68, 69, 91 }, .hint = 93, .key = .w },
    .garrison = .{ .x = 160, .n = .{ 70, 71, 72, 157 }, .hint = 94, .key = .d },
    .ritual = .{ .x = 220, .n = .{ 73, 74, 75, 120 }, .hint = 95, .key = .r },
    .scroll_up = .{ .x = 40, .n = .{ 76, 77, 78, 79 } },
    .cancel = .{ .x = 100, .n = .{ 84, 85, 86, 86 }, .hint = 106, .key = .escape },
    .scroll_down = .{ .x = 160, .n = .{ 80, 81, 82, 83 } },
    .build2 = .{ .x = 68, .n = .{ 67, 68, 69, 91 }, .hint = 93 },
    .cancel2 = .{ .x = 190, .n = .{ 84, 85, 86, 86 }, .hint = 106, .key = .escape },
    .raze = .{ .x = 70, .n = .{ 107, 108, 109, 110 }, .hint = 113 },
    .ritual2 = .{ .x = 70, .n = .{ 73, 74, 75, 120 }, .hint = 95 },
    .guard2 = .{ .x = 70, .n = .{ 64, 65, 66, 111 }, .hint = 92 },
    .dismiss = .{ .x = 70, .n = .{ 116, 117, 118, 119 }, .hint = 114 },
};
/// buttGuard and buttBuild when nothing is left to hire or build this
/// turn.
const butt_guard_done = 375;
const butt_build_done = 378;

/// buttBldFilter, buttDefFilter, buttRitFilter: normal, active, pressed,
/// inactive (= pressed-active), hints.
const filter_specs = [3]struct { n: [4]usize, hint: [2]u32 }{
    .{ .n = .{ 87, 88, 89, 90 }, .hint = .{ 107, 108 } },
    .{ .n = .{ 112, 113, 114, 115 }, .hint = .{ 109, 110 } },
    .{ .n = .{ 121, 122, 123, 124 }, .hint = .{ 111, 112 } },
};

pub const ProvPanel = struct {
    mode: Mode = .main,
    /// Member: the province shown.
    prov: i32 = 0,
    /// Shift of the list.
    shift: usize = 0,
    /// Choosen: the building, defender, site or ritual of a page.
    choice: i32 = 0,
    /// arBuildNum of the chosen site.
    site_slot: usize = 0,
    list: order.List = .{},
    box: ?wi.ScrollBox = null,
    body: []const u8 = "",
    ask: ?Ask = null,
    answer: ?usize = null,
    /// objEnchant is shown (an active ritual of the player here).
    enchanted: bool = false,
    sh: Sheets,
    b: Buttons,
    filters: [3]Toggle,
    build_mask: ?rl.Image = null,
    spell_mask: ?rl.Image = null,
    broken_img: ?rl.Image = null,
    rows: [bar_size]Mask = undefined,
    /// mskMain: defender, buildings, fort, sites.
    main: [4]Mask = undefined,
    race: Mask = .{ .x = bar_x + 46, .y = bar_y + 203 },
    broken: Mask = .{ .x = broken_x, .y = broken_y },
    /// mskRes: the resources (0..5), the prices (6, 7), the initiative and
    /// the upkeep (8..10).
    res: [11]Mask = undefined,
    cd: Mask = Mask.rect(bar_x + 188, bar_y + 96, 77, 23),

    /// In place: the buttons keep pointers into the panel.
    pub fn init(self: *ProvPanel, c: *Ctx, iface: *gfx.Sheet, buttons: *gfx.Sheet) !void {
        self.* = .{ .sh = undefined, .b = undefined, .filters = undefined };
        self.sh = try Sheets.load(c);
        errdefer self.sh.deinit();
        const fields = @typeInfo(Buttons).@"struct".field_names;
        var done: usize = 0;
        errdefer inline for (fields, 0..) |n, i| {
            if (i < done) @field(self.b, n).deinit();
        };
        inline for (fields) |n| {
            const s: Spec = @field(specs, n);
            var b = try Button.init(buttons, bar_x + s.x, bar_y + 396, s.n[0], s.n[1], s.n[2]);
            b.inactive = s.n[3];
            b.hint = s.hint;
            b.hotkey = s.key;
            @field(self.b, n) = b;
            done += 1;
        }
        self.b.build2.sound = snd_build;
        var nf: usize = 0;
        errdefer for (self.filters[0..nf]) |*t| t.b.deinit();
        for (&self.filters, filter_specs) |*t, s| {
            var b = try Button.init(buttons, bar_x + 220, bar_y + 396, s.n[0], s.n[1], s.n[2]);
            b.inactive = s.n[3];
            b.hint = s.hint[0];
            t.* = .{ .b = b, .pressed_active = s.n[3], .hint1 = s.hint[1] };
            nf += 1;
        }
        for (&self.rows, 0..) |*m, i| m.* = .{ .x = bar_x + 48, .y = bar_y + 37 + @as(f32, @floatFromInt(i * 59)) };
        for (&self.main, 0..) |*m, i| m.* = .{ .x = bar_x + 34 + @as(f32, @floatFromInt(i * 60)), .y = bar_y + 302 };
        for (self.res[0..6], 0..) |*m, i| m.* = Mask.rect(bar_x + 110 + @as(f32, @floatFromInt(i * 26)), bar_y + 97, 20, 20);
        self.res[6] = Mask.rect(bar_x + 110, bar_y + 64, 72, 24);
        self.res[7] = Mask.rect(bar_x + 183, bar_y + 64, 72, 24);
        self.res[8] = Mask.rect(bar_x + 51, bar_y + 132, 55, 24);
        self.res[9] = Mask.rect(bar_x + 110, bar_y + 132, 72, 24);
        self.res[10] = Mask.rect(bar_x + 183, bar_y + 132, 72, 24);
        self.build_mask = iface.imageIndex(gui.build_mask);
        self.spell_mask = iface.imageIndex(gui.spell_mask);
        self.broken_img = iface.imageIndex(gui.broken);
    }

    pub fn deinit(self: *ProvPanel) void {
        self.closeText();
        for ([_]?rl.Image{ self.build_mask, self.spell_mask, self.broken_img }) |img| if (img) |x| rl.unloadImage(x);
        for (&self.filters) |*t| t.b.deinit();
        inline for (@typeInfo(Buttons).@"struct".field_names) |n| @field(self.b, n).deinit();
        self.sh.deinit();
    }

    fn closeText(self: *ProvPanel) void {
        if (self.box) |*t| t.deinit();
        self.box = null;
        self.body = "";
    }

    fn openText(self: *ProvPanel, c: *Ctx, v: View, which: TextBox, body: []const u8) void {
        self.closeText();
        const g: struct { y: f32, h: i32, up: f32, down: f32 } = switch (which) {
            .ds => .{ .y = 163, .h = 184, .up = 137, .down = 359 },
            .ds1 => .{ .y = 196, .h = 159, .up = 172, .down = 362 },
            .ds2 => .{ .y = 128, .h = 138, .up = 104, .down = 274 },
        };
        self.box = wi.ScrollBox.init(c, v.buttons, bar_x + 59, bar_y + g.y, 195, g.h, .{ bar_x + 145, bar_y + g.up }, .{ bar_x + 145, bar_y + g.down }, body) catch null;
        self.body = body;
    }

    /// Condition 0: the main page of the province.
    pub fn reset(self: *ProvPanel) void {
        self.closeText();
        self.mode = .main;
        self.shift = 0;
        self.ask = null;
        self.answer = null;
    }

    fn setMode(self: *ProvPanel, mode: Mode) void {
        self.closeText();
        self.mode = mode;
    }

    /// A list page from its first row.
    fn openList(self: *ProvPanel, mode: Mode) void {
        self.setMode(mode);
        self.shift = 0;
    }

    /// mskBroken is over the pointer (it lies above the panel).
    pub fn over(self: *const ProvPanel, p: input.Pointer) bool {
        if (!self.enchanted) return false;
        var m = self.broken;
        if (self.broken_img) |*img| m.shape = img;
        return m.over(p);
    }

    fn market(v: View, c: *Ctx) player_mod.Market {
        return .{ .s = v.s, .pr = c.proto, .player = @intCast(v.player) };
    }

    fn pv(self: *const ProvPanel, v: View) *Province {
        return &v.s.provinces[@intCast(self.prov)];
    }

    fn runner(v: View, c: *Ctx) event_mod.Runner {
        return .{ .gpa = c.gpa, .pr = c.proto, .s = v.s, .r = &c.rand };
    }

    pub fn update(self: *ProvPanel, c: *Ctx, v: View, prov: i32, f: *const input.Frame) Out {
        if (prov != self.prov) {
            self.reset();
            self.prov = prov;
        }
        self.enchanted = order.enchanted(v.s, prov, @intCast(v.player));
        if (self.ask) |a| {
            self.ask = null;
            const ans = self.answer;
            self.answer = null;
            return self.answered(c, v, a, ans);
        }
        if (self.box) |*t| t.update(f);
        return switch (self.mode) {
            .main => self.updateMain(c, v, f),
            .build_list, .raze_list, .hire_list, .site_list, .ritual_list, .enchant => self.updateList(c, v, f),
            .build => self.updateBuild(c, v, f),
            .raze => self.updateRaze(c, v, f),
            .hire => self.updateHire(c, v, f),
            .guard => self.updateGuard(c, v, f),
            .site => self.updateBack(f, .site_list),
            .fort => self.updateBack(f, .main),
            .ritual => self.updateRitual(c, v, f),
        };
    }

    // -- Condition 1 ------------------------------------------------------

    fn ownHero(v: View, prov: i32) bool {
        for (v.s.heroes.items) |*h| {
            if (h.owner == @as(i32, @intCast(v.player)) and h.alive() and h.province == prov) return true;
        }
        return false;
    }

    fn hasOuterAbility(pr: *const proto_mod.Proto, p: *const Province, id: i32) bool {
        for (p.buildings[0..province.max_bld]) |bld| {
            if (bld <= 0 or bld >= pr.outer_buildings.len) continue;
            const ob = &pr.outer_buildings[@intCast(bld)];
            for (ob.abilities[0..ob.n_abilities]) |a| if (a.id == id) return true;
        }
        return false;
    }

    /// The buttons of Condition 1 (MainProcess.cpp:5080, 5117, 5236).
    fn mainButtons(self: *ProvPanel, c: *Ctx, v: View) void {
        const p = self.pv(v);
        const own = p.player == @as(i32, @intCast(v.player));
        const known = v.fog[@intCast(self.prov)] == 0;
        const h = &v.s.rulers[v.player].holdings;
        self.b.guard.enabled = own;
        self.b.build.enabled = own;
        self.b.garrison.enabled = own;
        const g: usize = if (h.can_hire > 0) 64 else butt_guard_done;
        self.b.guard.normal = g;
        self.b.guard.active = g + 1;
        self.b.guard.pressed = g + 2;
        const bb: usize = if (h.can_outer_bld > 0) 67 else butt_build_done;
        self.b.build.normal = bb;
        self.b.build.active = bb + 1;
        self.b.build.pressed = bb + 2;
        const home = order.homeCastle(v.s, self.prov);
        self.b.garrison.on = known and own and (home == v.player or hasOuterAbility(c.proto, p, ab_outer_garrison));
        self.b.ritual.enabled = true;
        self.b.ritual.on = known;
    }

    fn updateMain(self: *ProvPanel, c: *Ctx, v: View, f: *const input.Frame) Out {
        const p = f.pointer;
        self.mainButtons(c, v);
        if (self.b.guard.check(f)) {
            self.openList(.hire_list);
            return .none;
        }
        if (self.b.build.check(f)) {
            if (v.s.capitals[v.player] == self.prov) return .castle;
            self.openList(.build_list);
            return .none;
        }
        if (self.b.garrison.check(f)) return .garrison;
        if (self.b.ritual.check(f)) {
            self.openList(.ritual_list);
            return .none;
        }
        const own = self.pv(v).player == @as(i32, @intCast(v.player));
        const hero = ownHero(v, self.prov);
        for (&self.main, 0..) |*m, i| {
            if (!own and !(i == 2 and hero)) {
                m.state = .visible;
                continue;
            }
            if (self.spell_mask) |*img| m.shape = img;
            const r = m.check(p);
            if (m.hot()) v.hint.phrase(c, @intCast(115 + i), p.pos);
            if (r != .click) continue;
            switch (i) {
                0 => self.openGuard(c, v),
                1 => self.openList(.raze_list),
                2 => self.setMode(.fort),
                else => self.openList(.site_list),
            }
            return .none;
        }
        if (self.enchanted) {
            if (self.broken_img) |*img| self.broken.shape = img;
            const r = self.broken.check(p);
            if (self.broken.hot()) v.hint.phrase(c, 398, p.pos);
            if (r == .click) {
                self.openList(.enchant);
                return .none;
            }
        }
        for ([_]*const Button{ &self.b.guard, &self.b.build, &self.b.garrison, &self.b.ritual }) |b| {
            if (b.hot()) v.hint.phrase(c, b.hint, p.pos);
        }
        if (self.updateFields(c, v, p)) |a| return .{ .action = a };
        return .none;
    }

    /// The masks of the main page (MainProcess.cpp:1763, 2775): the hints,
    /// and on a click WindowRaceInfo, WindowInfo of the terrain,
    /// exploration, mood and level, WindowResourceInfo (or WindowInfo when
    /// there is no resource).
    fn updateFields(self: *ProvPanel, c: *Ctx, v: View, p: input.Pointer) ?scene.Action {
        const pr = c.proto;
        const pvv = self.pv(v);
        const click = p.primary.released or p.secondary.released;
        const Field = struct { x: f32, y: f32, w: f32, h: f32, hint: u32 };
        const terra: Field = .{ .x = 780, .y = 310, .w = 190, .h = 22, .hint = 437 };
        const resource: Field = .{ .x = 763, .y = 411, .w = 200, .h = 22, .hint = 399 };
        const race1: Field = .{ .x = 818, .y = 451, .w = 146, .h = 22, .hint = 402 };
        const known = v.fog[@intCast(self.prov)] < 1;
        const neutral = pvv.player == 0;
        const explore: Field = .{ .x = 760, .y = 340, .w = 210, .h = 22, .hint = if (neutral) 404 else 400 };
        const mood: Field = .{ .x = 818, .y = 481, .w = 146, .h = 24, .hint = 401 };
        const level: Field = .{ .x = 761, .y = 511, .w = 200, .h = 24, .hint = 403 };
        const gold: Field = .{ .x = 760, .y = 376, .w = 103, .h = 34, .hint = 405 };
        const gem: Field = .{ .x = 865, .y = 376, .w = 103, .h = 34, .hint = 406 };
        const in = struct {
            fn f(pt: input.Pointer, fd: Field) bool {
                return pt.in(fd.x, fd.y, fd.w, fd.h);
            }
        }.f;
        for ([_]Field{ terra, resource, race1 }) |fd| if (in(p, fd)) v.hint.phrase(c, fd.hint, p.pos);
        if (known) for ([_]Field{ explore, mood, level, gold, gem }) |fd| {
            if (in(p, fd)) v.hint.phrase(c, fd.hint, p.pos);
        };
        if (self.spell_mask) |*img| self.race.shape = img;
        const race_click = self.race.check(p) != .none;
        if (self.race.hot()) v.hint.phrase(c, 402, p.pos);
        if (race_click or (click and in(p, race1))) return .{ .push = info_windows.race(c, pvv.race) catch return .none };
        if (!click) return null;
        if (in(p, terra) and pvv.terrain >= 0 and pvv.terrain < pr.terrains.len) {
            const t: usize = @intCast(pvv.terrain);
            return info(c, pr.terrains[t].name, msg_terrain + t, stuff_terrain + t);
        }
        if (in(p, resource)) {
            const eco: economy.Economy = .{ .s = v.s, .pr = pr };
            const r = eco.resource(pvv, 0);
            if (r <= 0 or r >= pr.resources.len) return info(c, pr.phrase(ph.no_res), msg_no_res, stuff_no_res);
            return .{ .push = ResourceWindow.create(c, r, market(v, c)) catch return .none };
        }
        if (!known) return null;
        if (in(p, explore)) return if (neutral) info(c, pr.phrase(404), 33, 52) else info(c, pr.phrase(400), 30, 8);
        if (in(p, mood)) return info(c, pr.phrase(401), 31, 51);
        if (in(p, level)) return info(c, pr.phrase(403), 32, 50);
        return null;
    }

    fn info(c: *Ctx, title: []const u8, msg: usize, stuff: usize) scene.Action {
        const body = if (msg < c.proto.messages.len) c.proto.messages[msg] else "";
        return .{ .push = WindowInfo.create(c, title, body, stuff) catch return .none };
    }

    /// mskMain[0] (Condition 8): the defender of the province.
    fn openGuard(self: *ProvPanel, c: *Ctx, v: View) void {
        self.setMode(.guard);
        const d = self.pv(v).defender;
        self.choice = d;
        if (d > 0 and d < c.proto.defenders.len) self.openText(c, v, .ds1, c.proto.defenders[@intCast(d)].text);
    }

    // -- the lists --------------------------------------------------------

    fn makeList(self: *const ProvPanel, c: *Ctx, v: View) order.List {
        const m = market(v, c);
        return switch (self.mode) {
            .build_list => order.buildList(m, self.prov, self.shift),
            .raze_list => order.razeList(self.pv(v), self.shift),
            .hire_list => order.hireList(m, self.prov, self.shift),
            .site_list => order.siteList(c.proto, self.pv(v), self.shift),
            .ritual_list => order.ritualList(m, self.prov, self.shift),
            .enchant => order.enchantList(v.s, self.prov, @intCast(v.player), self.shift),
            else => .{},
        };
    }

    fn filter(self: *ProvPanel) ?*Toggle {
        return switch (self.mode) {
            .build_list => &self.filters[0],
            .hire_list => &self.filters[1],
            .ritual_list => &self.filters[2],
            else => null,
        };
    }

    fn filterFlag(self: *const ProvPanel, v: View) ?*bool {
        const h = &v.s.rulers[v.player].holdings;
        return switch (self.mode) {
            .build_list => &h.bld_filter,
            .hire_list => &h.def_filter,
            .ritual_list => &h.rit_filter,
            else => null,
        };
    }

    fn updateList(self: *ProvPanel, c: *Ctx, v: View, f: *const input.Frame) Out {
        const pr = c.proto;
        const p = f.pointer;
        self.list = self.makeList(c, v);
        if (self.shift > 0 and self.shift >= self.list.total) {
            self.shift = 0;
            self.list = self.makeList(c, v);
        }
        const shown = self.list.shown();
        for (&self.rows, 0..) |*m, i| {
            if (i >= shown.len) {
                m.state = .visible;
                continue;
            }
            if (self.build_mask) |*img| m.shape = img;
            const r = m.check(p);
            if (r == .none) continue;
            const id = shown[i];
            const m_ = market(v, c);
            switch (self.mode) {
                .build_list, .raze_list => if (r == .right_click) {
                    return .{ .action = .{ .push = CostWindow.outer(c, id, m_) catch return .none } };
                } else {
                    self.choice = id;
                    self.setMode(if (self.mode == .build_list) .build else .raze);
                    self.openText(c, v, .ds, pr.outer_buildings[@intCast(id)].text);
                },
                .hire_list => if (r == .right_click) {
                    return .{ .action = .{ .push = CostWindow.defender(c, id, 0, 0, m_) catch return .none } };
                } else {
                    self.choice = id;
                    self.setMode(.hire);
                    self.openText(c, v, .ds1, pr.defenders[@intCast(id)].text);
                },
                .ritual_list => if (r == .right_click) {
                    return .{ .action = .{ .push = CostWindow.ritual(c, id, m_) catch return .none } };
                } else {
                    self.choice = id;
                    self.setMode(.ritual);
                    self.openText(c, v, .ds, pr.rituals[@intCast(id)].text);
                },
                .site_list => if (r == .click) {
                    self.choice = id;
                    self.site_slot = self.list.slots[i];
                    const site = &pr.sites[@intCast(id)];
                    const guarded = self.pv(v).site_guards[self.site_slot] >= 0;
                    self.setMode(.site);
                    self.openText(c, v, .ds2, if (guarded) site.text else site.free_text);
                },
                .enchant => return .{ .action = .{ .push = CostWindow.ritual(c, id, m_) catch return .none } },
                else => {},
            }
            return .none;
        }
        // buttScrollUp / buttScrollDown, the wheel as ipHotkey -1 / -2.
        self.b.scroll_up.on = self.shift > 0;
        self.b.scroll_down.on = self.shift + bar_size < self.list.total;
        if ((self.b.scroll_up.check(f) or (p.wheel > 0 and self.b.scroll_up.on)) and self.shift > 0) self.shift -= 1;
        if ((self.b.scroll_down.check(f) or (p.wheel < 0 and self.b.scroll_down.on)) and self.shift + bar_size < self.list.total) self.shift += 1;
        if (self.b.cancel.check(f)) {
            self.reset();
            return .none;
        }
        if (self.filter()) |t| {
            const flag = self.filterFlag(v).?;
            t.press = flag.*;
            t.b.on = true;
            if (t.check(f)) {
                flag.* = t.press;
                self.shift = 0;
            }
            if (t.b.hot()) v.hint.phrase(c, t.hint(), p.pos);
        }
        if (self.b.cancel.hot()) v.hint.phrase(c, self.b.cancel.hint, p.pos);
        return .none;
    }

    // -- the pages --------------------------------------------------------

    /// mskRes over `res`: the name, on a click WindowResourceInfo; the
    /// hints of the prices (310, 311).
    fn updateRes(self: *ProvPanel, c: *Ctx, v: View, p: input.Pointer, res: []const i32) ?Out {
        for (res, 0..) |r, i| {
            if (r <= 0 or r >= c.proto.resources.len) continue;
            const k = self.res[i].check(p);
            if (self.res[i].hot()) {
                v.hint.what = .{ .string = c.proto.resources[@intCast(r)].name };
                v.hint.pos = p.pos;
            }
            if (k != .none) return .{ .action = .{ .push = ResourceWindow.create(c, r, market(v, c)) catch return .none } };
        }
        if (self.res[6].over(p)) v.hint.phrase(c, 310, p.pos);
        if (self.res[7].over(p)) v.hint.phrase(c, 311, p.pos);
        return null;
    }

    /// buttCancel2 of a page: back to `to`.
    fn cancel2(self: *ProvPanel, c: *Ctx, v: View, f: *const input.Frame, to: Mode) bool {
        if (self.b.cancel2.hot()) v.hint.phrase(c, self.b.cancel2.hint, f.pointer.pos);
        if (!self.b.cancel2.check(f)) return false;
        if (to == .main) self.reset() else self.setMode(to);
        return true;
    }

    fn updateBack(self: *ProvPanel, f: *const input.Frame, to: Mode) Out {
        if (self.b.cancel2.check(f)) {
            if (to == .main) self.reset() else self.setMode(to);
        }
        return .none;
    }

    /// Condition 3: buttBuild2 builds.
    fn updateBuild(self: *ProvPanel, c: *Ctx, v: View, f: *const input.Frame) Out {
        const m = market(v, c);
        const id = self.choice;
        self.b.build2.on = order.canAffordBuild(m, self.prov, id);
        if (self.b.build2.hot()) v.hint.phrase(c, self.b.build2.hint, f.pointer.pos);
        if (self.b.build2.check(f)) {
            var run = runner(v, c);
            defer run.deinit();
            order.construct(m, &run, self.prov, id) catch |e| std.log.err("build {d}: {t}", .{ id, e });
            self.reset();
            return .changed;
        }
        if (self.cancel2(c, v, f, .build_list)) return .none;
        return self.updateRes(c, v, f.pointer, &c.proto.outer_buildings[@intCast(id)].res) orelse .none;
    }

    /// Condition 5: buttRaze, under siege dialog 1243, with arMess[6]
    /// dialog 1.
    fn updateRaze(self: *ProvPanel, c: *Ctx, v: View, f: *const input.Frame) Out {
        self.b.raze.on = order.canRaze(c.proto, self.choice);
        if (self.b.raze.hot()) v.hint.phrase(c, self.b.raze.hint, f.pointer.pos);
        if (self.b.raze.check(f)) {
            if (movement.siege(v.s, self.prov) >= 0) return self.dialog(c, v, dlg_raze_siege, null);
            if (c.human.message(mess_raze)) return self.dialog(c, v, dlg_raze, .raze);
            return self.doRaze(c, v);
        }
        if (self.cancel2(c, v, f, .raze_list)) return .none;
        return .none;
    }

    fn doRaze(self: *ProvPanel, c: *Ctx, v: View) Out {
        var run = runner(v, c);
        defer run.deinit();
        order.raze(&run, self.pv(v), self.choice);
        self.reset();
        return .changed;
    }

    /// Condition 7: buttGuard2 hires, over a defender with arMess[5]
    /// after dialog 1165.
    fn updateHire(self: *ProvPanel, c: *Ctx, v: View, f: *const input.Frame) Out {
        const p = f.pointer;
        const m = market(v, c);
        const id = self.choice;
        self.b.guard2.on = order.canAffordHire(m, self.prov, id);
        if (self.b.guard2.hot()) v.hint.phrase(c, self.b.guard2.hint, p.pos);
        if (self.b.guard2.check(f)) {
            if (self.pv(v).defender > 0 and c.human.message(mess_defender)) return self.dialog(c, v, dlg_rehire, .hire);
            return self.doHire(c, v);
        }
        if (self.cancel2(c, v, f, .hire_list)) return .none;
        const d = &c.proto.defenders[@intCast(id)];
        if (self.updateRes(c, v, p, &d.res)) |o| return o;
        return self.updateUpkeep(c, v, p, d.initiative);
    }

    /// mskRes[8..10]: the initiative (WindowAbilityInfo(999)) and the
    /// upkeep hints.
    fn updateUpkeep(self: *ProvPanel, c: *Ctx, v: View, p: input.Pointer, initiative: i32) Out {
        const k = self.res[8].check(p);
        if (self.res[8].hot()) hero_ui.abilityHint(c, v.hint, ab_initiative, p.pos);
        if (k != .none) return .{ .action = info_windows.abilityAction(c, ab_initiative, initiative) };
        if (self.res[9].over(p)) v.hint.phrase(c, 90, p.pos);
        if (self.res[10].over(p)) v.hint.phrase(c, 91, p.pos);
        return .none;
    }

    fn doHire(self: *ProvPanel, c: *Ctx, v: View) Out {
        var run = runner(v, c);
        defer run.deinit();
        order.hire(market(v, c), &run, self.prov, self.choice) catch |e| std.log.err("hire {d}: {t}", .{ self.choice, e });
        sound.play(.unit, snd_coins);
        self.reset();
        return .changed;
    }

    /// Condition 8: buttDismiss, with arMess[5] after dialog 2.
    fn updateGuard(self: *ProvPanel, c: *Ctx, v: View, f: *const input.Frame) Out {
        const p = self.pv(v);
        const pr = c.proto;
        const has = p.defender > 0 and p.defender < pr.defenders.len;
        self.b.dismiss.on = has and !pr.defenders[@intCast(p.defender)].no_dismiss;
        if (self.b.dismiss.hot()) v.hint.phrase(c, self.b.dismiss.hint, f.pointer.pos);
        if (self.b.dismiss.check(f)) {
            if (c.human.message(mess_defender)) return self.dialog(c, v, dlg_dismiss, .dismiss);
            return self.doDismiss(c, v);
        }
        _ = self.cancel2(c, v, f, .main);
        return .none;
    }

    fn doDismiss(self: *ProvPanel, c: *Ctx, v: View) Out {
        order.dismiss(market(v, c), self.pv(v));
        self.reset();
        return .changed;
    }

    /// Condition 13: buttRitual2 casts.
    fn updateRitual(self: *ProvPanel, c: *Ctx, v: View, f: *const input.Frame) Out {
        const pr = c.proto;
        const m = market(v, c);
        const id = self.choice;
        const cost = ritual.price(m, id);
        const r = m.ruler();
        self.b.ritual2.on = ritual.canCast(v.s, pr, self.prov, id, m.player) and
            ritual.cooldown(v.s, pr, v.player, id) == 0 and
            (cost[0] <= r.gold or cost[0] == 0) and (cost[1] <= r.gem or cost[1] == 0);
        if (self.b.ritual2.hot()) v.hint.phrase(c, self.b.ritual2.hint, f.pointer.pos);
        if (self.b.ritual2.check(f)) {
            self.reset();
            return .{ .cast = id };
        }
        if (self.cancel2(c, v, f, .ritual_list)) return .none;
        if (self.cd.over(f.pointer)) v.hint.phrase(c, 394, f.pointer.pos);
        return self.updateRes(c, v, f.pointer, &pr.rituals[@intCast(id)].res) orelse .none;
    }

    fn dialog(self: *ProvPanel, c: *Ctx, v: View, index: usize, ask: ?Ask) Out {
        if (index >= c.proto.dialogs.len) return if (ask) |a| self.answered(c, v, a, 1) else .none;
        self.ask = ask;
        self.answer = null;
        return .{ .action = .{ .push = Dialog.create(c, index, .{ true, true, false, false }, &self.answer) catch {
            self.ask = null;
            return .none;
        } } };
    }

    /// The answer of a confirmation: 1 (yes) razes and dismisses; the hire
    /// is cancelled only by answer 2.
    fn answered(self: *ProvPanel, c: *Ctx, v: View, a: Ask, ans: ?usize) Out {
        return switch (a) {
            .raze => if (ans == 1) self.doRaze(c, v) else .none,
            .dismiss => if (ans == 1) self.doDismiss(c, v) else .none,
            .hire => if (ans != null and ans != 2) self.doHire(c, v) else .none,
        };
    }

    // -- drawing ----------------------------------------------------------

    pub fn draw(self: *ProvPanel, c: *Ctx, v: View, prov: i32) void {
        if (prov != self.prov) {
            self.reset();
            self.prov = prov;
            self.enchanted = order.enchanted(v.s, prov, @intCast(v.player));
            self.mainButtons(c, v);
        }
        switch (self.mode) {
            .main => self.drawMain(c, v),
            .build_list, .raze_list, .hire_list, .site_list, .ritual_list, .enchant => self.drawList(c, v),
            .build, .raze => self.drawBuilding(c, v),
            .hire => self.drawHire(c, v),
            .guard => self.drawGuard(c, v),
            .site => self.drawSite(c, v),
            .fort => self.drawFort(c, v),
            .ritual => self.drawRitual(c, v),
        }
        if (self.box) |*t| t.draw(c, v.buttons, self.body);
    }

    fn num(buf: []u8, n: i32) []const u8 {
        return std.fmt.bufPrint(buf, "{d}", .{n}) catch "";
    }

    fn signed(buf: []u8, n: i32) []const u8 {
        return (if (n > 0) std.fmt.bufPrint(buf, "+{d}", .{n}) else std.fmt.bufPrint(buf, "{d}", .{n})) catch "";
    }

    /// "%d~": the font draws "~" as the percent sign.
    fn percent(buf: []u8, n: i32) []const u8 {
        return std.fmt.bufPrint(buf, "{d}~", .{n}) catch "";
    }

    fn rgb(r: i32, g: i32, b: i32) rl.Color {
        return .{ .r = @truncate(@as(u32, @bitCast(r))), .g = @truncate(@as(u32, @bitCast(g))), .b = @truncate(@as(u32, @bitCast(b))), .a = 255 };
    }

    fn at(x: f32, y: f32) [2]f32 {
        return .{ bar_x + x, bar_y + y };
    }

    fn text(c: *Ctx, s: []const u8, x: f32, y: f32, col: rl.Color) void {
        c.font1.drawCentered(s, bar_x + x, bar_y + y, col);
    }

    fn pic(t: ?rl.Texture2D, x: f32, y: f32) void {
        gfx.drawTex(t, bar_x + x, bar_y + y);
    }

    fn outerPic(self: *ProvPanel, b: i32) ?rl.Texture2D {
        return if (b > 0) self.sh.outbuild.texIndex(@intCast(b - 1)) else null;
    }

    fn defenderPic(self: *ProvPanel, d: i32) ?rl.Texture2D {
        return if (d > 0) self.sh.defenders.texIndex(@intCast(d - 1)) else null;
    }

    fn ritualPic(self: *ProvPanel, r: i32) ?rl.Texture2D {
        return if (r > 0) self.sh.ritual.texIndex(@intCast(r - 1)) else null;
    }

    fn sitePic(self: *ProvPanel, s: i32) ?rl.Texture2D {
        return if (s > 0) self.sh.site.texIndex(@intCast(s - 1)) else null;
    }

    fn resPic(self: *ProvPanel, r: i32) ?rl.Texture2D {
        return if (r > 0) self.sh.resource.texIndex(@intCast(r - 1)) else null;
    }

    /// ShowIconNumber at the corner of a 64×64 picture.
    fn iconNumber(self: *ProvPanel, c: *Ctx, n: i32, x: f32, y: f32) void {
        var buf: [16]u8 = undefined;
        const s = num(&buf, n);
        if (n < 10) {
            gfx.drawTex(self.sh.stuff.texIndex(st.count1), x + 37, y + 32);
            c.font1.draw(s, x + 40, y + 36, color1);
        } else {
            gfx.drawTex(self.sh.stuff.texIndex(st.count2), x + 29, y + 33);
            c.font1.drawCentered(s, x + 39, y + 37, color1);
        }
    }

    fn glow(v: View, m: *const Mask, sel: usize) void {
        if (m.hot()) gfx.drawTex(v.iface.texIndex(sel), m.x, m.y);
    }

    /// Condition 0 → 1 (MainProcess.cpp:5172).
    fn drawMain(self: *ProvPanel, c: *Ctx, v: View) void {
        const pr = c.proto;
        const p = self.pv(v);
        const eco: economy.Economy = .{ .s = v.s, .pr = pr };
        var b: [96]u8 = undefined;
        pic(v.iface.texIndex(gui.province_bar), 0, 0);
        if (p.player > 0 and p.player < v.players.len) {
            const pl = v.players[@intCast(p.player)];
            v.arms.drawHerb(pl.color1, pl.color2, pl.sign, bar_x + 8, bar_y + 60);
            v.arms.drawHerb(pl.color1, pl.color2, pl.sign, bar_x + 273, bar_y + 60);
        }
        text(c, p.name, 156, 43, color1);
        if (p.terrain >= 0 and p.terrain < pr.terrains.len) text(c, pr.terrains[@intCast(p.terrain)].name, 156, 73, color1);
        if (p.race > 0 and p.race < pr.races.len) {
            pic(self.sh.races.texIndex(@intCast(p.race - 1)), 52, 209);
            text(c, pr.races[@intCast(p.race)].name, 185, 215, color1);
        }
        glow(v, &self.race, gui.spell_select);
        // The resource: found, still guarded (red), hidden but known to
        // the castle (grey), or none.
        const free = eco.resource(p, 1);
        const found = eco.resource(p, 0);
        const hidden = eco.resource(p, -1);
        if (free > 0) {
            pic(self.resPic(free), 57, 169);
            text(c, pr.resources[@intCast(free)].name, 168, 176, color1);
        } else if (found > 0) {
            pic(self.resPic(found), 57, 169);
            text(c, pr.resources[@intCast(found)].name, 168, 176, red);
        } else if (hidden > 0 and eco.compareBldParam1(@intCast(v.player), ab_hidden_resource, p.terrain)) {
            pic(self.resPic(hidden), 57, 169);
            text(c, pr.resources[@intCast(hidden)].name, 168, 176, grey);
        } else text(c, pr.phrase(ph.no_res), 168, 176, color1);

        if (v.fog[@intCast(self.prov)] == 0) {
            const lv = eco.level(p.population);
            if (p.player == 0) {
                if (p.type >= 0 and p.type < pr.province_types.len) text(c, pr.province_types[@intCast(p.type)].name, 156, 103, color1);
            } else {
                const s = std.fmt.bufPrint(&b, "{s} {d}~", .{ pr.phrase(ph.explored), p.explore }) catch "";
                text(c, s, 152, 103, if (p.explore >= pr.province_levels[@intCast(lv)].explored) color1 else red);
            }
            // The bottom row: defender, buildings, fort, sites.
            if (order.fort(v.s, pr, p)) |fo| {
                pic(if (fo.castle) self.sh.innerbld.texIndex(pic_castle) else self.outerPic(fo.building), 160, 308);
                text(c, percent(&b, fo.percent), 184, 374, color2);
            } else text(c, "-", 186, 374, color2);
            var count: i32 = 0;
            var best: i32 = 0;
            for (p.buildings[0..province.max_bld]) |bl| {
                if (bl <= 0 or bl >= pr.outer_buildings.len) continue;
                count += 1;
                const ob = &pr.outer_buildings[@intCast(bl)];
                if (best == 0 or (ob.gold_cost > pr.outer_buildings[@intCast(best)].gold_cost and ob.wall == 0)) best = bl;
            }
            text(c, num(&b, count), 126, 374, color2);
            if (best > 0) pic(self.outerPic(best), 100, 308);
            if (p.defender > 0 and p.player > 0) {
                pic(self.defenderPic(p.defender), 40, 308);
                text(c, percent(&b, order.guardLife(v.s, pr, p)), 64, 374, color2);
            } else text(c, "-", 66, 374, color2);
            var sites: i32 = 0;
            var last: i32 = 0;
            for (p.sites) |s| {
                if (s.index > 0 and s.pos <= p.explore) {
                    last = s.index;
                    sites += 1;
                }
            }
            text(c, num(&b, sites), 246, 374, color2);
            if (last > 0) pic(self.sitePic(last), 220, 308);

            text(c, signed(&b, eco.goldIncome(p)), 115, 144, color2);
            text(c, signed(&b, eco.gemIncome(p)), 230, 144, color2);

            // The population bar: 200 pixels to the next level.
            const levels = pr.province_levels;
            if (lv < proto_mod.max_province_level) {
                const w: i32 = if (p.population <= 0)
                    (10 - @rem(-p.population, 10)) * 20
                else blk: {
                    const lo = levels[@intCast(lv - 1)].max_population;
                    const hi = levels[@intCast(lv)].max_population;
                    break :blk if (hi > lo) @divTrunc(200 * (p.population - lo), hi - lo) else 0;
                };
                var i: i32 = 0;
                while (i < w) : (i += 1) rl.drawRectangle(bar_x + 56 + i, bar_y + 268, 2, 23, rgb(@divTrunc(i, 5), 50 + i, @divTrunc(i, 10)));
            }
            text(c, levels[@intCast(lv)].name, 158, 276, color1);
            text(c, levels[@intCast(lv)].name, 157, 275, color2);

            const unrest = p.unrest[@intCast(std.math.clamp(p.player, 0, province.max_players - 1))];
            var i: i32 = 0;
            while (i < @divTrunc(145 * unrest, 500)) : (i += 1) rl.drawRectangle(bar_x + 110 + i, bar_y + 238, 1, 23, rgb(110 + i, 20 + @divTrunc(i, 7), 20 + @divTrunc(i, 7)));
            if (p.population > 0) {
                const md = std.math.clamp(eco.mood(p), -5, 3);
                const s = pr.phrase(@intCast(ph.mood + md));
                text(c, s, 188, 245, color1);
                text(c, s, 187, 244, if (md >= 0) rgb(200 - md * 20, 200 + md * 15, 20 + md * 40) else rgb(200 - md * 10, 200 + md * 40, 20 + md * 4));
            } else {
                text(c, pr.phrase(ph.empty), 188, 245, color1);
                text(c, pr.phrase(ph.empty), 187, 244, color2);
            }
        }
        for (&self.main) |*m| glow(v, m, gui.spell_select);
        if (self.enchanted) {
            gfx.drawTex(v.iface.texIndex(gui.broken), broken_x, broken_y);
            gfx.drawTex(self.sh.stuff.texIndex(st.enchant), broken_x + 27, broken_y + 15);
        }
        for ([_]*const Button{ &self.b.guard, &self.b.build, &self.b.garrison, &self.b.ritual }) |bt| bt.draw(v.buttons, c.tick);
    }

    fn drawError(c: *Ctx, msg: usize) void {
        if (msg >= c.proto.messages.len) return;
        _ = c.font2.drawWrappedClip(c.proto.messages[msg], bar_x + 56, bar_y + 44, 207, bar_y + 44 + 340, color2);
    }

    /// Price (gold, gems) at (gx, y) and (233, y), red when it is too high.
    fn price(c: *Ctx, m: player_mod.Market, cost: player_mod.Price, gold_zero_ok: bool, gx: f32, y: f32) void {
        var b: [32]u8 = undefined;
        const r = m.ruler();
        const gold_ok = cost[0] <= r.gold or (gold_zero_ok and cost[0] == 0);
        const gem_ok = cost[1] <= r.gem or cost[1] == 0;
        text(c, num(&b, cost[0]), gx, y, if (gold_ok) color2 else red);
        text(c, num(&b, cost[1]), 233, y, if (gem_ok) color2 else red);
    }

    /// BuildList, RazeList, HireList, SiteList, RitualList, EnchantList.
    fn drawList(self: *ProvPanel, c: *Ctx, v: View) void {
        const pr = c.proto;
        const m = market(v, c);
        const p = self.pv(v);
        var b: [64]u8 = undefined;
        pic(v.iface.texIndex(gui.build_bar), 0, 0);
        if (self.list.err) |e| drawError(c, e);
        const raze_frame = switch (self.mode) {
            .raze_list, .site_list, .enchant => true,
            else => false,
        };
        for (self.list.shown(), 0..) |id, i| {
            const dy: f32 = @floatFromInt(i * 59);
            pic(v.iface.texIndex(if (raze_frame) gui.raze_panel else gui.build_panel), 50, 39 + dy);
            switch (self.mode) {
                .build_list => {
                    pic(self.outerPic(id), 52, 41 + dy);
                    const stock = ritual.listStock(v.s, pr, v.player, .outer, id);
                    if (stock > 0) self.iconNumber(c, stock, bar_x + 52, bar_y + 41 + dy);
                    text(c, pr.outer_buildings[@intCast(id)].name, 193, 48 + dy, if (order.canBuild(v.s, pr, p, id)) color1 else red);
                    price(c, m, order.outerPrice(m, id), false, 151, 70 + dy);
                },
                .raze_list => {
                    pic(self.outerPic(id), 52, 41 + dy);
                    text(c, pr.outer_buildings[@intCast(id)].name, 193, 48 + dy, color1);
                },
                .hire_list => {
                    pic(self.defenderPic(id), 52, 41 + dy);
                    const stock = ritual.listStock(v.s, pr, v.player, .defender, id);
                    if (stock > 0) self.iconNumber(c, stock, bar_x + 52, bar_y + 41 + dy);
                    text(c, pr.defenders[@intCast(id)].name, 190, 48 + dy, if (order.canHire(v.s, pr, p, id)) color1 else red);
                    price(c, m, order.defenderPrice(m, id), false, 151, 70 + dy);
                },
                .site_list => {
                    pic(self.sitePic(id), 52, 41 + dy);
                    text(c, pr.sites[@intCast(id)].name, 183, 48 + dy, color1);
                    const slot = self.list.slots[i];
                    const g = p.sites[slot].guard;
                    if (p.site_guards[slot] >= 0 and g >= 0 and g < pr.guard_types.len) {
                        text(c, std.fmt.bufPrint(&b, "({s})", .{pr.guard_types[@intCast(g)].name}) catch "", 193, 70 + dy, red);
                    }
                },
                .ritual_list => {
                    pic(self.ritualPic(id), 52, 41 + dy);
                    const stock = ritual.stock(v.s, pr, v.player, id);
                    if (stock > 0) self.iconNumber(c, stock, bar_x + 52, bar_y + 41 + dy);
                    const cd = ritual.cooldown(v.s, pr, v.player, id);
                    if (cd > 0) {
                        pic(self.sh.stuff.texIndex(st.cooldown), 53, 42 + dy);
                        text(c, num(&b, cd), 74, 47 + dy, color1);
                    }
                    const can = ritual.canCast(v.s, pr, self.prov, id, m.player) and cd == 0;
                    text(c, pr.rituals[@intCast(id)].name, 193, 48 + dy, if (can) color1 else red);
                    price(c, m, ritual.price(m, id), true, 151, 70 + dy);
                },
                .enchant => {
                    pic(self.ritualPic(id), 52, 41 + dy);
                    text(c, pr.rituals[@intCast(id)].name, 193, 48 + dy, color1);
                    if (pr.rituals[@intCast(id)].time > 0) {
                        pic(self.sh.stuff.texIndex(st.timer), 145, 63 + dy);
                        text(c, num(&b, self.list.timers[i]), 189, 70 + dy, color2);
                    }
                },
                else => {},
            }
            glow(v, &self.rows[i], gui.build_select);
        }
        self.b.scroll_up.draw(v.buttons, c.tick);
        self.b.cancel.draw(v.buttons, c.tick);
        self.b.scroll_down.draw(v.buttons, c.tick);
        if (self.filter()) |t| t.draw(v.buttons);
    }

    /// The resources of a price at (110 + j·26, 97), crossed when the
    /// player does not have them.
    fn drawRes(self: *ProvPanel, c: *Ctx, v: View, res: []const i32) void {
        const m = market(v, c);
        for (res, 0..) |r, j| {
            if (r <= 0) continue;
            const x = 110 + @as(f32, @floatFromInt(j * 26));
            pic(self.resPic(r), x, 97);
            if (m.resource(r) == 0) pic(v.iface.texIndex(gui.no_res), x, 97);
        }
    }

    /// Conditions 3 and 5.
    fn drawBuilding(self: *ProvPanel, c: *Ctx, v: View) void {
        const pr = c.proto;
        const m = market(v, c);
        const id = self.choice;
        const ob = &pr.outer_buildings[@intCast(id)];
        var b: [32]u8 = undefined;
        pic(v.iface.texIndex(gui.building), 0, 0);
        pic(self.outerPic(id), 52, 65);
        text(c, ob.name, 156, 44, color1);
        const cost = order.outerPrice(m, id);
        text(c, num(&b, cost[0]), if (self.mode == .build) 147 else 150, 71, color2);
        text(c, num(&b, cost[1]), 227, 71, color2);
        self.drawRes(c, v, &ob.res);
        if (self.mode == .build) self.b.build2.draw(v.buttons, c.tick) else self.b.raze.draw(v.buttons, c.tick);
        self.b.cancel2.draw(v.buttons, c.tick);
    }

    /// Condition 7.
    fn drawHire(self: *ProvPanel, c: *Ctx, v: View) void {
        const pr = c.proto;
        const m = market(v, c);
        const id = self.choice;
        const d = &pr.defenders[@intCast(id)];
        var b: [32]u8 = undefined;
        pic(v.iface.texIndex(gui.defender), 0, 0);
        pic(self.defenderPic(id), 52, 65);
        text(c, d.name, 156, 44, color1);
        const cost = order.defenderPrice(m, id);
        text(c, num(&b, cost[0]), 150, 71, color2);
        text(c, num(&b, cost[1]), 227, 71, color2);
        text(c, num(&b, d.initiative), 91, 138, color2);
        text(c, num(&b, d.gold_payment), 150, 138, color2);
        text(c, num(&b, d.gem_payment), 227, 138, color2);
        self.drawRes(c, v, &d.res);
        self.b.guard2.draw(v.buttons, c.tick);
        self.b.cancel2.draw(v.buttons, c.tick);
    }

    /// Condition 8: the defender of the province, its initiative and
    /// upkeep (the guard's own when iGoldPayment < 0).
    fn drawGuard(self: *ProvPanel, c: *Ctx, v: View) void {
        const pr = c.proto;
        const m = market(v, c);
        const p = self.pv(v);
        const id = p.defender;
        var b: [32]u8 = undefined;
        if (id > 0 and id < pr.defenders.len) {
            const d = &pr.defenders[@intCast(id)];
            pic(v.iface.texIndex(gui.defender), 0, 0);
            pic(self.defenderPic(id), 52, 65);
            text(c, d.name, 160, 44, color1);
            const cost = order.defenderPrice(m, id);
            text(c, num(&b, cost[0]), 149, 71, color2);
            text(c, num(&b, cost[1]), 227, 71, color2);
            const init_ = if (p.guard >= 0) v.s.guards.items[@intCast(p.guard)].initiative else d.initiative;
            text(c, num(&b, init_), 87, 138, color2);
            const pay: statistics.Pair = if (d.gold_payment < 0) statistics.defenderPayment(v.s, pr, p.guard) else .{ .gold = d.gold_payment, .gem = d.gem_payment };
            text(c, num(&b, pay.gold), 150, 138, color2);
            text(c, num(&b, pay.gem), 227, 138, color2);
            self.drawRes(c, v, &d.res);
        } else pic(v.iface.texIndex(gui.build_bar), 0, 0);
        self.b.dismiss.draw(v.buttons, c.tick);
        self.b.cancel2.draw(v.buttons, c.tick);
    }

    /// Condition 10.
    fn drawSite(self: *ProvPanel, c: *Ctx, v: View) void {
        const pr = c.proto;
        const p = self.pv(v);
        const id = self.choice;
        pic(v.iface.texIndex(gui.site), 0, 0);
        pic(self.sitePic(id), 52, 41);
        text(c, pr.sites[@intCast(id)].name, 182, 48, color1);
        const slot = self.site_slot;
        if (p.site_guards[slot] >= 0) {
            var b: [64]u8 = undefined;
            const col: rl.Color = .{ .r = 250, .g = 30, .b = 30, .a = 255 };
            text(c, std.fmt.bufPrint(&b, "{s}:", .{pr.phrase(ph.guard)}) catch "", 156, 315, col);
            const g = p.sites[slot].guard;
            if (g >= 0 and g < pr.guard_types.len) text(c, pr.guard_types[@intCast(g)].name, 156, 340, col);
        }
        self.b.cancel2.draw(v.buttons, c.tick);
    }

    /// The state of the walls in percent (phrases 27..34).
    fn wallWord(pct: i32) usize {
        if (pct == 100) return 27;
        for ([_]i32{ 75, 50, 40, 30, 20, 0 }, 28..) |t, w| if (pct > t) return w;
        return 34;
    }

    /// The state of tower `n` (1..4): phrases 27, 28, 29, 32, 33.
    fn towerWord(pct: i32, n: usize) usize {
        const lim = [4][4]i32{ .{ 44, 41, 38, 35 }, .{ 34, 31, 28, 25 }, .{ 19, 16, 13, 10 }, .{ 9, 6, 3, 0 } };
        for (lim[n - 1], [_]usize{ 27, 28, 29, 32 }) |t, w| if (pct > t) return w;
        return 33;
    }

    /// Condition 11 (MainProcess.cpp:2954).
    fn drawFort(self: *ProvPanel, c: *Ctx, v: View) void {
        const pr = c.proto;
        pic(v.iface.texIndex(gui.fort), 0, 0);
        text(c, pr.phrase(ph.fort_title), 184, 49, color1);
        if (order.fort(v.s, pr, self.pv(v))) |fo| {
            pic(if (fo.castle) self.sh.innerbld.texIndex(pic_castle) else self.outerPic(fo.building), 52, 41);
            text(c, pr.phrase(ph.walls), 156, 133, color1);
            text(c, pr.phrase(wallWord(fo.percent)), 156, 150, color1);
            var b: [64]u8 = undefined;
            var n: usize = 1;
            while (n <= 4 and fo.towers >= n) : (n += 1) {
                const y: f32 = 173 + 40 * @as(f32, @floatFromInt(n - 1));
                text(c, std.fmt.bufPrint(&b, "{s} {d}", .{ pr.phrase(ph.tower), n }) catch "", 156, y, color1);
                text(c, pr.phrase(towerWord(fo.percent, n)), 156, y + 17, color1);
            }
        } else text(c, pr.phrase(ph.no_fort), 156, 135, color1);
        self.b.cancel2.draw(v.buttons, c.tick);
    }

    /// Condition 13.
    fn drawRitual(self: *ProvPanel, c: *Ctx, v: View) void {
        const pr = c.proto;
        const m = market(v, c);
        const id = self.choice;
        const r = &pr.rituals[@intCast(id)];
        var b: [32]u8 = undefined;
        pic(v.iface.texIndex(gui.building), 0, 0);
        pic(self.ritualPic(id), 52, 65);
        text(c, r.name, 156, 44, color1);
        const cost = ritual.price(m, id);
        text(c, num(&b, cost[0]), 150, 71, color2);
        text(c, num(&b, cost[1]), 227, 71, color2);
        pic(self.sh.stuff.texIndex(st.timer), 186, 95);
        text(c, num(&b, r.cooldown - m.bldParam1(53)), 232, 102, color2);
        self.drawRes(c, v, &r.res);
        self.b.ritual2.draw(v.buttons, c.tick);
        self.b.cancel2.draw(v.buttons, c.tick);
    }
};
