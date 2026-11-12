//! ScreenStatistics (Screens.cpp:6127): the ruler's portrait, name, title
//! and counters, the income and expenses of a turn (game/statistics.zig),
//! the resources with their market prices and the other rulers, ten at a
//! time.  "P" opens ScreenProvList; a province picked there closes this
//! screen too and is left in `out` (the map centres on it).
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Button = @import("../ui/button.zig").Button;
const Hint = @import("../ui/hint.zig").Hint;
const heraldry = @import("../ui/heraldry.zig");
const profile = @import("../data/profile.zig");
const mapgen = @import("../game/mapgen.zig");
const Shard = mapgen.Shard;
const stats = @import("../game/statistics.zig");
const player = @import("../game/player.zig");
const ProvList = @import("prov_list.zig").ProvList;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };
const green: rl.Color = .{ .r = 50, .g = 220, .b = 50, .a = 255 };
const plus: rl.Color = .{ .r = 20, .g = 220, .b = 20, .a = 255 };
const red: rl.Color = .{ .r = 220, .g = 20, .b = 20, .a = 255 };

/// interface.dat 66Statistics, 24RulerBar.
const gui_statistics = 66;
const gui_ruler_bar = 24;
/// portraits.dat / smallport.dat: the avatars after MAX_HERO_PIC heroes;
/// avatar 16 is the unknown ruler.
const max_hero_pic = 16;
const avatar_unknown = 16;
/// Opponents shown at once.
const shown = 10;

/// What the screen shows of arPlayer[i]: GetName, GetAvatar and ShowHerb.
pub const Face = struct {
    name: []const u8 = "",
    /// bmpAvatar index.
    avatar: i32 = avatar_unknown,
    color1: i32 = 0,
    color2: i32 = 0,
    sign: i32 = 0,
};

pub const Statistics = struct {
    s: *Shard,
    k: usize,
    turn: i32,
    /// The picked province, 0 for none.
    out: *i32,
    faces: [mapgen.max_players + 1]Face,
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    portraits: gfx.Sheet,
    smallport: gfx.Sheet,
    arms: heraldry.Heraldry,
    exit: Button,
    prov: Button,
    up: Button,
    down: Button,
    report: stats.Report,
    /// iShift: the first player of the right column.
    shift: usize,
    hint: Hint = .{},
    /// The province ScreenProvList returns.
    picked: i32 = 0,
    old_cursor: u32,
    /// "Name Nick".
    title_buf: [128]u8 = undefined,

    pub fn create(c: *Ctx, s: *Shard, k: usize, turn: i32, faces: [mapgen.max_players + 1]Face, out: *i32) !scene.Scene {
        const self = try c.gpa.create(Statistics);
        errdefer c.gpa.destroy(self);
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer buttons.deinit();
        var portraits = try gfx.Sheet.load(c.fs, "portraits");
        errdefer portraits.deinit();
        var smallport = try gfx.Sheet.load(c.fs, "smallport");
        errdefer smallport.deinit();
        var arms = try heraldry.Heraldry.load(c.fs);
        errdefer arms.deinit();
        var exit = try Button.init(&buttons, 918, 682, 226, 227, 228);
        errdefer exit.deinit();
        exit.hotkey = .escape;
        exit.hint = 135;
        var prov = try Button.init(&buttons, 190, 700, 87, 88, 89);
        errdefer prov.deinit();
        prov.inactive = 90;
        prov.hotkey = .p;
        prov.hint = 551;
        var up = try Button.init(&buttons, 766, 675, 76, 77, 78);
        errdefer up.deinit();
        up.inactive = 79;
        var down = try Button.init(&buttons, 847, 675, 80, 81, 82);
        errdefer down.deinit();
        down.inactive = 83;
        self.* = .{
            .s = s,
            .k = k,
            .turn = turn,
            .out = out,
            .faces = faces,
            .iface = iface,
            .buttons = buttons,
            .portraits = portraits,
            .smallport = smallport,
            .arms = arms,
            .exit = exit,
            .prov = prov,
            .up = up,
            .down = down,
            .report = stats.report(s, c.proto, k, c.cfg.max_profit),
            .shift = if (k == 1) 2 else 1,
            .old_cursor = c.cursor,
        };
        self.up.on = false;
        // With fewer than 12 players everyone fits: no scroll buttons.
        self.up.enabled = s.n_players >= 12;
        self.down.enabled = s.n_players >= 12;
        self.down.on = self.overflow();
        c.setCursor(0);
        return scene.Scene.of(Statistics, self, false);
    }

    pub fn deinit(self: *Statistics, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.down.deinit();
        self.up.deinit();
        self.prov.deinit();
        self.exit.deinit();
        self.arms.deinit();
        self.smallport.deinit();
        self.portraits.deinit();
        self.buttons.deinit();
        self.iface.deinit();
    }

    /// The other players from iShift on do not fit into ten rows (the
    /// draw loop's buttScrollDown.Activate(1)).
    fn overflow(self: *const Statistics) bool {
        var n: usize = 0;
        for (self.shift..self.s.n_players + 1) |i| n += @intFromBool(i != self.k);
        return n > shown;
    }

    pub fn update(self: *Statistics, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        if (self.picked > 0) {
            self.out.* = self.picked;
            return .pop;
        }
        if (self.up.check(f)) {
            self.shift -|= 1;
            if (self.shift == self.k and self.shift > 1) self.shift -= 1;
            if (self.shift <= 1 or (self.shift == 2 and self.k == 1)) self.up.on = false;
            self.down.on = true;
        }
        if (self.down.check(f)) {
            self.shift += 1;
            if (self.shift == self.k and self.s.n_players > self.shift + 9) self.shift += 1;
            self.down.on = self.overflow();
            self.up.on = true;
        }
        if (self.prov.check(f)) return .{ .push = ProvList.create(c, self.s, self.k, &self.picked) catch |e| {
            std.log.err("province list: {t}", .{e});
            return .none;
        } };
        if (self.exit.check(f)) return .pop;
        for ([_]*const Button{ &self.exit, &self.prov }) |b| if (b.hot()) self.hint.phrase(c, b.hint, f.pointer.pos);
        return .none;
    }

    fn face(self: *const Statistics, i: usize) Face {
        return self.faces[i];
    }

    fn difficulty(c: *const Ctx, d: i32) []const u8 {
        return c.proto.difficulties[@intCast(std.math.clamp(d, 0, @as(i32, @intCast(c.proto.difficulties.len)) - 1))].name;
    }

    fn line(c: *Ctx, buf: []u8, ph: usize, v: i32, y: f32) void {
        c.font1.drawCentered(std.fmt.bufPrint(buf, "{s}: {d}", .{ c.proto.phrase(ph), v }) catch "", 517, y, color1);
    }

    pub fn draw(self: *Statistics, c: *Ctx) void {
        const pr = c.proto;
        const s = self.s;
        const r = &s.rulers[self.k];
        const me = self.face(self.k);
        rl.clearBackground(.black);
        gfx.drawTex(self.portraits.texIndex(avatarIndex(me.avatar)), 452, 43);
        gfx.drawTex(self.iface.texIndex(gui_statistics), 0, 0);
        self.arms.drawHerb(me.color1, me.color2, me.sign, 419, 54);
        self.arms.drawHerb(me.color1, me.color2, me.sign, 585, 54);
        c.font1.drawCentered(std.fmt.bufPrint(&self.title_buf, "{s} {s}", .{ me.name, nick(pr, r.karma) }) catch "", 517, 23, color1);
        c.font1.drawCentered(pr.phrase(264), 185, 49, color1);
        c.font1.drawCentered(pr.phrase(58), 838, 49, color1);
        c.font1.drawCentered(difficulty(c, r.difficulty), 517, 204, color1);

        var buf: [128]u8 = undefined;
        line(c, &buf, 51, self.turn, 252);
        const ally = r.holdings.union_race;
        const ally_name = if (ally > 0 and ally < pr.races.len) pr.races[@intCast(ally)].name else pr.phrase(40);
        c.font1.drawCentered(std.fmt.bufPrint(&buf, "{s}: {s}", .{ pr.phrase(186), ally_name }) catch "", 517, 272, color1);
        const ch = &r.chronicle;
        line(c, &buf, 267, ch.win, 292);
        line(c, &buf, 268, ch.lose, 312);
        line(c, &buf, 269, stats.provinces(s, self.k), 332);
        line(c, &buf, 270, stats.heroes(s, self.k), 352);
        line(c, &buf, 273, stats.defeated(s, self.k), 372);
        c.font1.drawCentered(pr.phrase(274), 517, 395, color1);
        line(c, &buf, 275, ch.loot[0], 415);
        line(c, &buf, 276, ch.loot[1], 435);
        line(c, &buf, 277, ch.loot[2], 455);
        line(c, &buf, 278, ch.quest, 480);
        line(c, &buf, 396, ch.time_shift, 505);

        self.drawEconomy(c);
        self.drawResources(c);
        c.font1.drawCentered(fmtInt(&buf, r.gold), 470, 730, color2);
        c.font1.drawCentered(fmtInt(&buf, r.gem), 593, 730, color2);
        self.drawRulers(c);

        self.exit.draw(&self.buttons, c.tick);
        self.prov.draw(&self.buttons, c.tick);
        self.up.draw(&self.buttons, c.tick);
        self.down.draw(&self.buttons, c.tick);
        self.hint.draw(c, &self.iface);
    }

    fn drawEconomy(self: *const Statistics, c: *Ctx) void {
        const pr = c.proto;
        const rp = self.report;
        const Row = struct { ph: usize, v: stats.Pair, y: f32, col: rl.Color };
        const list = [_]Row{
            .{ .ph = 55, .v = rp.castle, .y = 108, .col = green },
            .{ .ph = 56, .v = rp.provinces, .y = 132, .col = green },
            .{ .ph = 57, .v = rp.heroes, .y = 156, .col = green },
            .{ .ph = 58, .v = rp.opponents_in, .y = 180, .col = green },
            .{ .ph = 61, .v = rp.income, .y = 204, .col = green },
            .{ .ph = 266, .v = rp.troops, .y = 252, .col = red },
            .{ .ph = 59, .v = rp.guards, .y = 276, .col = red },
            .{ .ph = 60, .v = rp.garrisons, .y = 300, .col = red },
            .{ .ph = 58, .v = rp.opponents_out, .y = 324, .col = red },
            .{ .ph = 263, .v = rp.corruption, .y = 348, .col = red },
            .{ .ph = 61, .v = rp.expenses, .y = 372, .col = red },
        };
        c.font1.drawCentered(pr.phrase(53), 120, 84, color1);
        c.font1.drawCentered(pr.phrase(54), 120, 228, color1);
        c.font1.drawCentered(pr.phrase(62), 120, 396, color1);
        var buf: [16]u8 = undefined;
        for (list) |row| {
            c.font1.drawCentered(pr.phrase(row.ph), 117, row.y, color2);
            c.font1.drawCentered(fmtInt(&buf, row.v.gold), 253, row.y, row.col);
            c.font1.drawCentered(fmtInt(&buf, row.v.gem), 339, row.y, row.col);
        }
        const b = rp.balance();
        for ([2]i32{ b.gold, b.gem }, [2]f32{ 253, 339 }) |v, x| {
            const text = (if (v > 0) std.fmt.bufPrint(&buf, "+{d}", .{v}) else std.fmt.bufPrint(&buf, "{d}", .{v})) catch "";
            c.font1.drawCentered(text, x, 396, if (v > 0) plus else red);
        }
    }

    fn drawResources(self: *const Statistics, c: *Ctx) void {
        const pr = c.proto;
        const m: player.Market = .{ .s = self.s, .pr = pr, .player = @intCast(self.k) };
        c.font1.drawCentered(pr.phrase(63), 117, 452, color1);
        var buf: [16]u8 = undefined;
        for (1..pr.resources.len) |i| {
            const y = 475 + 24 * @as(f32, @floatFromInt(i - 1));
            const ri: i32 = @intCast(i);
            const p = m.resPriceStat(ri);
            c.font1.drawCentered(pr.resources[i].name, 125, y, color2);
            c.font1.drawCentered(fmtInt(&buf, m.resource(ri)), 211, y, color2);
            c.font1.drawCentered(fmtInt(&buf, p[0]), 266, y, color2);
            c.font1.drawCentered(fmtInt(&buf, p[1]), 339, y, color2);
        }
    }

    /// The right column: a stranger (arPlayerSet 0, no contact, in the
    /// game, a computer) is shown as unknown.
    fn drawRulers(self: *Statistics, c: *Ctx) void {
        const pr = c.proto;
        const s = self.s;
        const me = &s.rulers[self.k];
        var row: usize = 0;
        for (self.shift..s.n_players + 1) |i| {
            if (i == self.k) continue;
            if (row >= shown) break;
            const y = 76 + 59 * @as(f32, @floatFromInt(row));
            row += 1;
            gfx.drawTex(self.iface.texIndex(gui_ruler_bar), 655, y);
            const o = &s.rulers[i];
            if (s.player_set[i] == 0 and me.chronicle.treaty[i] == 0 and s.in_game[i] and o.kind != .human) {
                gfx.drawTex(self.smallport.texIndex(avatarIndex(avatar_unknown)), 660, y + 4);
                gfx.drawTex(self.arms.flag.texIndex(heraldry.flag_herb_random), 723, y + 14);
                gfx.drawTex(self.arms.flag.texIndex(heraldry.flag_herb_random), 947, y + 14);
                c.font1.drawCentered(pr.phrase(271), 852, y + 14, color1);
                c.font1.drawCentered(difficulty(c, o.difficulty), 852, y + 34, color1);
                continue;
            }
            const fc = self.face(i);
            gfx.drawTex(self.smallport.texIndex(avatarIndex(fc.avatar)), 660, y + 4);
            self.arms.drawHerb(fc.color1, fc.color2, fc.sign, 723, y + 14);
            self.arms.drawHerb(fc.color1, fc.color2, fc.sign, 947, y + 14);
            c.font1.drawCentered(fc.name, 852, y + 14, color1);
            c.font1.drawCentered(if (s.in_game[i]) difficulty(c, o.difficulty) else pr.phrase(265), 852, y + 34, color1);
        }
    }
};

fn avatarIndex(a: i32) usize {
    return @intCast(std.math.clamp(a, 0, profile.max_avatars - 1) + max_hero_pic);
}

/// GetRulerNick.
pub fn nick(pr: *const @import("../data/proto.zig").Proto, karma: i32) []const u8 {
    for (pr.ruler_nicks) |n| if (n.min <= karma and n.max >= karma) return n.text;
    return "";
}

fn fmtInt(buf: []u8, v: i32) []const u8 {
    return std.fmt.bufPrint(buf, "{d}", .{v}) catch "";
}
