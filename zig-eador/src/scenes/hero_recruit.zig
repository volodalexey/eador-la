//! ScreenHeroRecruit (Castle.cpp:1077): the hero tab of the castle.  The
//! four classes for hire with their prices (a click opens
//! HeroRecruitWindow, Castle.cpp:856, unless the ruler has 8 heroes,
//! dialog 1077, or 4 of the class, dialog 1089) and the fallen heroes
//! (iDead >= 0): the turns left while the body is on its way (a click
//! shows dialog 29), else the resurrection price (a click opens
//! ResurrectWindow, Castle.cpp:949: resurrect, bury after dialog 28,
//! cancel).  A hired or resurrected hero closes the castle and is
//! selected on the map.
//!
//! Not ported: the tutorial hints, the exchange button of ResurrectWindow
//! (ScreenHeroGarrison), shown inactive.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const Hint = @import("../ui/hint.zig").Hint;
const Mask = @import("../ui/mask.zig").Mask;
const Button = @import("../ui/button.zig").Button;
const text = @import("../ui/text.zig");
const castle_ui = @import("../ui/castle_ui.zig");
const Sheets = castle_ui.Sheets;
const Dialog = @import("dialog.zig").Dialog;
const event_mod = @import("../game/event.zig");
const player = @import("../game/player.zig");
const Market = player.Market;
const Price = player.Price;
const heroes = @import("../game/heroes.zig");
const hero_mod = @import("../game/hero.zig");

/// interface.dat: 58HeroRecruit, 59HeroRecruitWindow, 60HeroResurrectWindow,
/// 61HeroDead, 71StrHint, 105..108 HeroSelect, 109DeadSelect.
const gui_recruit = 58;
const gui_recruit_window = 59;
const gui_resurrect_window = 60;
const gui_dead = 61;
const gui_str_hint = 71;
const gui_select = 105;
const gui_dead_select = 109;
/// buttons.dat first pictures.
const butt_recruit = 158;
const butt_resurrect = 162;
const butt_exchange = 149;
const butt_bury = 166;
const butt_cancel = 84;
/// dialog.var.
const dlg_bury = 28;
const dlg_on_way = 29;
const dlg_max_heroes = 1077;
const dlg_max_class = 1089;
/// phrases.var: the titles, "Ходов", the button hints.
const ph_hire = 47;
const ph_dead = 48;
const ph_turns = 313;
const ph_recruit = 137;
const ph_resurrect = 138;
const ph_bury = 139;
const ph_exchange = 140;
const ph_cancel = 106;
/// spell_sound.dat: the resurrection.
const snd_resurrect = 58;

const dx = 122;
const color1 = castle_ui.color1;
const color2 = castle_ui.color2;
const short: rl.Color = .{ .r = 220, .g = 20, .b = 20, .a = 255 };

fn priceColor(have: i32, need: i32) rl.Color {
    return if (have >= need) color2 else short;
}

fn darken() void {
    rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 255 - 190 });
}

pub const HeroTab = struct {
    class_shapes: [4]?rl.Image = @splat(null),
    classes: [4]Mask = undefined,
    corpses: [hero_mod.max_hero]Mask = undefined,
    /// arCorpses: the Shard.heroes index of each corpse shown.
    dead: [hero_mod.max_hero]usize = undefined,
    n_dead: usize = 0,
    smallport: gfx.Sheet,
    /// The answers of the windows and dialogs opened from the tab.
    recruit_result: ?i32 = null,
    resurrect_result: ?i32 = null,
    dialog_result: ?usize = null,
    buf: [256]u8 = undefined,

    /// What the castle does after a frame of the tab.
    pub const Out = union(enum) {
        none,
        push: scene.Scene,
        /// A hero was hired or resurrected: the castle closes and the map
        /// selects the hero in this slot.
        hero: u8,
    };

    pub fn init(c: *Ctx, sh: *Sheets) !HeroTab {
        var t: HeroTab = .{ .smallport = try gfx.Sheet.load(c.fs, "smallport") };
        for (&t.class_shapes, &t.classes, 0..) |*img, *m, i| {
            img.* = sh.iface.imageIndex(gui_select + i);
            m.* = .{ .x = @floatFromInt(31 + i * dx), .y = 68 };
        }
        const dead = sh.iface.texIndex(gui_dead);
        for (&t.corpses, 0..) |*m, i| {
            m.* = Mask.rect(@floatFromInt(14 + (i % 2) * 243), @floatFromInt(254 + (i / 2) * 105), gfx.texW(dead), gfx.texH(dead));
        }
        return t;
    }

    pub fn deinit(self: *HeroTab) void {
        for (self.class_shapes) |img| if (img) |x| rl.unloadImage(x);
        self.smallport.deinit();
    }

    /// arCorpses: the owner's heroes with iDead >= 0, by slot.
    fn collect(self: *HeroTab, m: Market) void {
        self.n_dead = 0;
        for (m.s.heroes.items, 0..) |*h, k| {
            if (h.owner != m.player or h.dead < 0 or self.n_dead == self.dead.len) continue;
            self.dead[self.n_dead] = k;
            self.n_dead += 1;
        }
    }

    fn dialog(self: *HeroTab, c: *Ctx, index: usize, enabled: [4]bool, body: ?[]const u8) Out {
        self.dialog_result = null;
        const sc = (if (body) |b| Dialog.createText(c, index, enabled, &self.dialog_result, b) else Dialog.create(c, index, enabled, &self.dialog_result)) catch |e| {
            std.log.err("dialog {d}: {t}", .{ index, e });
            return .none;
        };
        return .{ .push = sc };
    }

    pub fn update(self: *HeroTab, c: *Ctx, m: Market, p: input.Pointer, hint: *Hint) Out {
        _ = hint;
        if (self.recruit_result) |r| {
            self.recruit_result = null;
            if (r > 0) return .{ .hero = @intCast(r - 1) };
        }
        if (self.resurrect_result) |r| {
            self.resurrect_result = null;
            if (r > 0) return .{ .hero = @intCast(r - 1) };
        }
        self.dialog_result = null;
        self.collect(m);
        for (self.corpses[0..self.n_dead], self.dead[0..self.n_dead]) |*mk, k| {
            if (mk.check(p) != .click) continue;
            const h = &m.s.heroes.items[k];
            if (h.dead == 0) {
                self.resurrect_result = null;
                return .{ .push = ResurrectWindow.create(c, m, k, &self.resurrect_result) catch |e| {
                    std.log.err("resurrect window: {t}", .{e});
                    return .none;
                } };
            }
            const fmt = c.proto.dialogs[dlg_on_way].text;
            return self.dialog(c, dlg_on_way, .{ true, false, false, false }, event_mod.format(&self.buf, fmt, &.{.{ .d = h.dead }}));
        }
        for (&self.classes, &self.class_shapes, 0..) |*mk, *img, i| {
            if (img.*) |*x| mk.shape = x else continue;
            if (mk.check(p) != .click) continue;
            const owner: usize = @intCast(m.player);
            const class: i32 = @intCast(i + 1);
            if (heroes.count(m.s, owner) >= heroes.max_hero) return self.dialog(c, dlg_max_heroes, .{ true, true, false, false }, null);
            if (heroes.classCount(m.s, owner, class) >= heroes.max_class_heroes) return self.dialog(c, dlg_max_class, .{ true, true, false, false }, null);
            self.recruit_result = null;
            return .{ .push = RecruitWindow.create(c, m, class, &self.recruit_result) catch |e| {
                std.log.err("recruit window: {t}", .{e});
                return .none;
            } };
        }
        return .none;
    }

    pub fn draw(self: *HeroTab, c: *Ctx, sh: *Sheets, m: Market) void {
        const pr = c.proto;
        const r = m.ruler();
        gfx.drawTex(sh.iface.texIndex(gui_recruit), 0, 0);
        c.font1.drawCentered(pr.phrase(ph_hire), 260, 18, color1);
        c.font1.drawCentered(pr.phrase(ph_dead), 260, 232, color1);
        var b: [64]u8 = undefined;
        for (0..4) |i| {
            const fi: f32 = @floatFromInt(i * dx);
            if (self.classes[i].hot()) gfx.drawTex(sh.iface.texIndex(gui_select + i), self.classes[i].x, self.classes[i].y);
            c.font1.drawCentered(pr.classes[i].name, 72 + fi, 52, color1);
            const pc = heroes.price(m, @intCast(i + 1));
            c.font1.drawCentered(castle_ui.fmtInt(&b, pc[0]), 76 + fi, 167, priceColor(r.gold, pc[0]));
            c.font1.drawCentered(castle_ui.fmtInt(&b, pc[1]), 76 + fi, 193, priceColor(r.gem, pc[1]));
        }
        self.collect(m);
        const plaque = sh.iface.texIndex(gui_str_hint);
        for (self.dead[0..self.n_dead], 0..) |k, n| {
            const h = &m.s.heroes.items[k];
            const x: f32 = @floatFromInt(14 + (n % 2) * 243);
            const y: f32 = @floatFromInt(254 + (n / 2) * 105);
            gfx.drawTex(sh.iface.texIndex(gui_dead), x, y);
            gfx.drawTex(self.smallport.texIndex(@intCast(@max(h.portrait, 0))), x + 11, y + 9);
            c.font1.drawCentered(h.nameText(pr), x + 154, y + 15, color1);
            c.font1.drawCentered(pr.classes[h.className()].name, x + 154, y + 43, color1);
            if (h.dead > 0) {
                // bmpTime: the first 151 columns of 71StrHint and its last 23.
                if (plaque) |t| {
                    const ph: f32 = @floatFromInt(t.height);
                    rl.drawTextureRec(t, .{ .x = 0, .y = 0, .width = 151, .height = ph }, .{ .x = x + 63, .y = y + 65 }, .white);
                    rl.drawTextureRec(t, .{ .x = @as(f32, @floatFromInt(t.width)) - 23, .y = 0, .width = 23, .height = ph }, .{ .x = x + 63 + 151, .y = y + 65 }, .white);
                }
                c.font1.drawCentered(std.fmt.bufPrint(&b, "{s}:{d}", .{ pr.phrase(ph_turns), h.dead }) catch "", x + 154, y + 75, color1);
            } else {
                const pc = heroes.resurrectPrice(m, h);
                c.font1.drawCentered(castle_ui.fmtInt(&b, pc[0]), x + 114, y + 76, priceColor(r.gold, pc[0]));
                c.font1.drawCentered(castle_ui.fmtInt(&b, pc[1]), x + 201, y + 76, priceColor(r.gem, pc[1]));
            }
            c.font1.drawCentered(castle_ui.fmtInt(&b, h.level), x + 42, y + 76, color2);
            if (self.corpses[n].hot()) gfx.drawTex(sh.iface.texIndex(gui_dead_select), self.corpses[n].x, self.corpses[n].y);
        }
    }
};

/// HeroRecruitWindow: the class's unit picture, name, price and text;
/// "Нанять" pays and hires (CoinSound).  `out` gets the hero's slot + 1,
/// or 0 on cancel.
pub const RecruitWindow = struct {
    m: Market,
    class: i32,
    out: *?i32,
    sh: Sheets,
    recruit: Button,
    cancel: Button,
    x: f32,
    y: f32,
    hint: Hint = .{},
    old_cursor: u32,

    pub fn create(c: *Ctx, m: Market, class: i32, out: *?i32) !scene.Scene {
        const self = try c.gpa.create(RecruitWindow);
        errdefer c.gpa.destroy(self);
        self.* = .{ .m = m, .class = class, .out = out, .sh = undefined, .recruit = undefined, .cancel = undefined, .x = 0, .y = 0, .old_cursor = c.cursor };
        self.sh = try Sheets.load(c);
        errdefer self.sh.deinit();
        const win = self.sh.iface.texIndex(gui_recruit_window) orelse return error.MissingWindow;
        self.x = @floatFromInt(512 - @divTrunc(win.width, 2));
        self.y = @floatFromInt(384 - @divTrunc(win.height, 2));
        self.recruit = try Button.init(&self.sh.buttons, self.x + 160, self.y + 78, butt_recruit, butt_recruit + 1, butt_recruit + 2);
        errdefer self.recruit.deinit();
        self.recruit.inactive = butt_recruit + 3;
        self.recruit.hint = ph_recruit;
        self.recruit.on = heroes.canPay(m, heroes.price(m, class));
        self.cancel = try Button.init(&self.sh.buttons, self.x + 279, self.y + 78, butt_cancel, butt_cancel + 1, butt_cancel + 2);
        self.cancel.hotkey = .escape;
        self.cancel.hint = ph_cancel;
        c.setCursor(0);
        return scene.Scene.of(RecruitWindow, self, true);
    }

    pub fn deinit(self: *RecruitWindow, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.cancel.deinit();
        self.recruit.deinit();
        self.sh.deinit();
    }

    pub fn update(self: *RecruitWindow, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        if (self.cancel.check(f)) {
            self.out.* = 0;
            return .pop;
        }
        if (self.recruit.check(f)) {
            sound.play(.unit, sound.buy);
            const slot = heroes.hire(self.m, &c.rand, self.class) catch |e| {
                std.log.err("hire: {t}", .{e});
                self.out.* = 0;
                return .pop;
            };
            self.out.* = @as(i32, slot) + 1;
            return .pop;
        }
        for ([_]*const Button{ &self.recruit, &self.cancel }) |b| {
            if (b.hot()) self.hint.phrase(c, b.hint, f.pointer.pos);
        }
        return .none;
    }

    pub fn draw(self: *RecruitWindow, c: *Ctx) void {
        const pr = c.proto;
        const i: usize = @intCast(self.class - 1);
        darken();
        const x = self.x;
        const y = self.y;
        gfx.drawTex(self.sh.iface.texIndex(gui_recruit_window), x, y);
        const pic = self.sh.unitPic(self.class + 39);
        gfx.drawTex(pic, x + 53 - @floor(gfx.texW(pic) / 2), y + 67 - @floor(gfx.texH(pic) / 2));
        c.font1.drawCentered(pr.classes[i].name, x + 211, y + 25, color1);
        const pc = heroes.price(self.m, self.class);
        const r = self.m.ruler();
        var b: [16]u8 = undefined;
        c.font1.drawCentered(castle_ui.fmtInt(&b, pc[0]), x + 148, y + 55, priceColor(r.gold, pc[0]));
        c.font1.drawCentered(castle_ui.fmtInt(&b, pc[1]), x + 231, y + 55, priceColor(r.gem, pc[1]));
        const box = text.Box.init(x + 32, y + 147, 287, 191, &c.font2, color1);
        _ = box.draw(pr.classes[i].text);
        self.recruit.draw(&self.sh.buttons, c.tick);
        self.cancel.draw(&self.sh.buttons, c.tick);
        self.hint.draw(c, &self.sh.iface);
    }
};

/// ResurrectWindow: the hero's portrait, name, class, level and the
/// resurrection price.  `out` gets the hero's slot + 1 when he is
/// resurrected in the capital, -1 when buried, 0 on cancel.
pub const ResurrectWindow = struct {
    m: Market,
    k: usize,
    out: *?i32,
    sh: Sheets,
    portraits: gfx.Sheet,
    resurrect: Button,
    exchange: Button,
    bury: Button,
    cancel: Button,
    x: f32,
    y: f32,
    answer: ?usize = null,
    asking: bool = false,
    hint: Hint = .{},
    old_cursor: u32,

    pub fn create(c: *Ctx, m: Market, k: usize, out: *?i32) !scene.Scene {
        const self = try c.gpa.create(ResurrectWindow);
        errdefer c.gpa.destroy(self);
        self.* = .{ .m = m, .k = k, .out = out, .sh = undefined, .portraits = undefined, .resurrect = undefined, .exchange = undefined, .bury = undefined, .cancel = undefined, .x = 0, .y = 0, .old_cursor = c.cursor };
        self.sh = try Sheets.load(c);
        errdefer self.sh.deinit();
        self.portraits = try gfx.Sheet.load(c.fs, "portraits");
        errdefer self.portraits.deinit();
        const win = self.sh.iface.texIndex(gui_resurrect_window) orelse return error.MissingWindow;
        self.x = @floatFromInt(512 - @divTrunc(win.width, 2));
        self.y = @floatFromInt(384 - @divTrunc(win.height, 2));
        const Spec = struct { b: *Button, dx: f32, first: usize, hint: u32 };
        const specs = [_]Spec{
            .{ .b = &self.resurrect, .dx = 152, .first = butt_resurrect, .hint = ph_resurrect },
            .{ .b = &self.exchange, .dx = 212, .first = butt_exchange, .hint = ph_exchange },
            .{ .b = &self.bury, .dx = 272, .first = butt_bury, .hint = ph_bury },
        };
        var done: usize = 0;
        errdefer for (specs[0..done]) |s| s.b.deinit();
        for (specs) |s| {
            s.b.* = try Button.init(&self.sh.buttons, self.x + s.dx, self.y + 130, s.first, s.first + 1, s.first + 2);
            s.b.inactive = s.first + 3;
            s.b.hint = s.hint;
            done += 1;
        }
        self.cancel = try Button.init(&self.sh.buttons, self.x + 332, self.y + 130, butt_cancel, butt_cancel + 1, butt_cancel + 2);
        self.cancel.hotkey = .escape;
        self.cancel.hint = ph_cancel;
        // ScreenHeroGarrison is not ported.
        self.exchange.on = false;
        self.resurrect.on = heroes.canPay(m, heroes.resurrectPrice(m, self.hero()));
        c.setCursor(0);
        return scene.Scene.of(ResurrectWindow, self, true);
    }

    pub fn deinit(self: *ResurrectWindow, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.cancel.deinit();
        self.bury.deinit();
        self.exchange.deinit();
        self.resurrect.deinit();
        self.portraits.deinit();
        self.sh.deinit();
    }

    fn hero(self: *const ResurrectWindow) *hero_mod.Hero {
        return &self.m.s.heroes.items[self.k];
    }

    pub fn update(self: *ResurrectWindow, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        if (self.asking) {
            self.asking = false;
            const yes = self.answer == 1;
            self.answer = null;
            if (yes) {
                heroes.bury(self.m.s, c.proto, self.k);
                self.out.* = -1;
                return .pop;
            }
        }
        if (self.cancel.check(f)) {
            self.out.* = 0;
            return .pop;
        }
        if (self.bury.check(f)) {
            self.asking = true;
            return .{ .push = Dialog.create(c, dlg_bury, .{ true, true, false, false }, &self.answer) catch {
                self.asking = false;
                return .none;
            } };
        }
        if (self.resurrect.check(f)) {
            sound.play(.spell, snd_resurrect);
            heroes.resurrect(self.m, self.k, true);
            self.out.* = @as(i32, self.hero().slot) + 1;
            return .pop;
        }
        for ([_]*const Button{ &self.resurrect, &self.bury, &self.cancel }) |b| {
            if (b.hot()) self.hint.phrase(c, b.hint, f.pointer.pos);
        }
        return .none;
    }

    pub fn draw(self: *ResurrectWindow, c: *Ctx) void {
        const pr = c.proto;
        const h = self.hero();
        darken();
        const x = self.x;
        const y = self.y;
        gfx.drawTex(self.sh.iface.texIndex(gui_resurrect_window), x, y);
        gfx.drawTex(self.portraits.texIndex(@intCast(@max(h.portrait, 0))), x + 16, y + 17);
        c.font1.drawCentered(h.nameText(pr), x + 277, y + 23, color1);
        c.font1.drawCentered(pr.classes[h.className()].name, x + 277, y + 50, color1);
        var b: [16]u8 = undefined;
        c.font1.drawCentered(castle_ui.fmtInt(&b, h.level), x + 210, y + 82, color2);
        const pc = heroes.resurrectPrice(self.m, h);
        const r = self.m.ruler();
        c.font1.drawCentered(castle_ui.fmtInt(&b, pc[0]), x + 324, y + 80, priceColor(r.gold, pc[0]));
        c.font1.drawCentered(castle_ui.fmtInt(&b, pc[1]), x + 324, y + 106, priceColor(r.gem, pc[1]));
        self.resurrect.draw(&self.sh.buttons, c.tick);
        self.exchange.draw(&self.sh.buttons, c.tick);
        self.bury.draw(&self.sh.buttons, c.tick);
        self.cancel.draw(&self.sh.buttons, c.tick);
        self.hint.draw(c, &self.sh.iface);
    }
};
