//! Профиль игрока: ProfileOperator (Windower.cpp:6096) с окнами
//! WindowProfile (5519) и WindowChoosePlayer (5913).
//!
//! ProfileOperator(0) открывал WindowProfile, пока тот возвращал «к списку»,
//! показывал WindowChoosePlayer и снова WindowProfile.  Здесь это одна сцена
//! с двумя режимами.  Профиль правится на месте (c.human = arHumans[0]) и
//! сохраняется при каждом выходе из окна профиля, как в оригинале.  В конце
//! пишется save/game.con и применяются громкости профиля; `closed`
//! становится true, чтобы меню перечитало сохранения.
//!
//! GOG FUN_0041c020 создаёт те же 12 кнопок, что и C++; удаления профиля в
//! игре нет.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const profile = @import("../data/profile.zig");
const Human = profile.Human;
const button = @import("../ui/button.zig");
const Button = button.Button;
const Mask = @import("../ui/mask.zig").Mask;
const Input = @import("../ui/input.zig").Input;
const Hint = @import("../ui/hint.zig").Hint;
const heraldry = @import("../ui/heraldry.zig");

/// interface.dat.
const gui_profile = 62;
const gui_choose_player = 125;
const gui_ruler_bar = 24;
/// portraits.dat / smallport.dat: avatars follow MAX_HERO_PIC hero pictures.
const max_hero_pic = 16;
/// bmpSmallAvatar[16]: the "new player" row.
const new_player_avatar = 16;

/// phrases.var.
const ph_new_player = 212;
const ph_profile = 348;
const ph_anonymous = 216;
const ph_homeland = 217;
const ph_choose = 444;
const ph_name_hint = 214;
const ph_land_hint = 215;
const ph_diff_hint = 218;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };
/// draw_lit_sprite(.., 100) with set_trans_blender(200, 200, 200, 0).
const row_lit: rl.Color = .{ .r = 200, .g = 200, .b = 200, .a = 100 };

const rows = 6;
const row_h = 59;

const Spec = button.Spec;
const left = button.left;
const right = button.right;

const EditButton = enum { list, exit, herb_l, herb_r, col1_l, col1_r, col2_l, col2_r, diff_l, diff_r, face_l, face_r };
const edit_specs: std.enums.EnumArray(EditButton, Spec) = .init(.{
    .list = .{ .x = 13, .y = 374, .normal = 87, .active = 88, .pressed = 90, .inactive = 89, .hint = 349 },
    .exit = .{ .x = 211, .y = 374, .normal = 305, .active = 306, .pressed = 307, .inactive = 308, .hint = 135 },
    .herb_l = left(89, 141),
    .herb_r = right(172, 141),
    .col1_l = left(11, 141),
    .col1_r = right(65, 141),
    .col2_l = left(192, 141),
    .col2_r = right(246, 141),
    .diff_l = left(49, 181),
    .diff_r = right(206, 181),
    .face_l = left(44, 275),
    .face_r = right(214, 275),
});

const ListButton = enum { exit, up, down };
const list_specs: std.enums.EnumArray(ListButton, Spec) = .init(.{
    .exit = .{ .x = 157, .y = 451, .normal = 84, .active = 85, .pressed = 86, .inactive = 86, .hint = 106 },
    .up = .{ .x = 73, .y = 451, .normal = 76, .active = 77, .pressed = 78, .inactive = 79 },
    .down = .{ .x = 241, .y = 451, .normal = 80, .active = 81, .pressed = 82, .inactive = 83 },
});

/// The arrows wrap around (iColor1, iSign, ipAvatar).
fn prev(v: i32, n: i32) i32 {
    return if (v > 0) v - 1 else n - 1;
}

fn next(v: i32, n: i32) i32 {
    return if (v < n - 1) v + 1 else 0;
}

fn avatarIndex(avatar: i32) usize {
    return @intCast(std.math.clamp(avatar, 0, profile.max_avatars - 1) + max_hero_pic);
}

pub fn difficultyName(c: *Ctx, d: i32) []const u8 {
    return c.proto.difficulties[@intCast(std.math.clamp(d, 0, profile.max_difficulty))].name;
}

const Mode = enum { edit, list };

pub const Profile = struct {
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    portraits: gfx.Sheet,
    smallport: gfx.Sheet,
    arms: heraldry.Heraldry,
    ruler_shape: rl.Image,
    closed: *bool,
    /// The profile being edited: c.human (ProfileOperator) or a hot-seat
    /// player's (WindowProfile(i, false)).
    h: *Human,
    old_cursor: u32,
    hint: Hint = .{},
    mode: Mode = .edit,
    /// bList of WindowProfile: the list and difficulty buttons work.
    allow_list: bool = true,

    // WindowProfile.
    ex: f32,
    ey: f32,
    eb: std.enums.EnumArray(EditButton, Button),
    name_mask: Mask,
    land_mask: Mask,
    diff_mask: Mask,
    name_in: Input,
    land_in: Input,
    /// bSave, bName, bLand.
    save: bool = false,
    name_fresh: bool = true,
    land_fresh: bool = true,

    // WindowChoosePlayer.
    lx: f32,
    ly: f32,
    lb: std.enums.EnumArray(ListButton, Button),
    row_masks: [rows + 1]Mask,
    slots: [profile.max_slots]i32 = undefined,
    n_slots: usize = 0,
    /// Shift: the list moves by whole pages of six.
    page: usize = 0,
    shown: [rows]Human = undefined,
    n_shown: usize = 0,

    /// ProfileOperator(0): the profile in use, with the list.
    pub fn create(c: *Ctx, closed: *bool) !scene.Scene {
        return createFor(c, &c.human, true, closed);
    }

    /// WindowProfile(i, bList): `h` must outlive the scene.  Without the
    /// list the window only edits and saves `h`.
    pub fn createFor(c: *Ctx, h: *Human, allow_list: bool, closed: *bool) !scene.Scene {
        const self = try c.gpa.create(Profile);
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
        const shape = iface.imageIndex(gui_ruler_bar) orelse return error.MissingRulerBar;
        errdefer rl.unloadImage(shape);
        const pw = iface.texIndex(gui_profile) orelse return error.MissingProfileWindow;
        const cw = iface.texIndex(gui_choose_player) orelse return error.MissingChoosePlayer;
        const ex: f32 = @floatFromInt(512 - @divTrunc(pw.width, 2));
        const ey: f32 = @floatFromInt(384 - @divTrunc(pw.height, 2));
        const lx: f32 = @floatFromInt(512 - @divTrunc(cw.width, 2));
        const ly: f32 = @floatFromInt(384 - @divTrunc(cw.height, 2));
        self.* = .{
            .iface = iface,
            .buttons = buttons,
            .portraits = portraits,
            .smallport = smallport,
            .arms = arms,
            .ruler_shape = shape,
            .closed = closed,
            .h = h,
            .allow_list = allow_list,
            .old_cursor = c.cursor,
            .ex = ex,
            .ey = ey,
            .eb = undefined,
            .name_mask = .rect(ex + 37, ey + 52, 202, 29),
            .land_mask = .rect(ex + 37, ey + 90, 202, 29),
            .diff_mask = .rect(ex + 68, ey + 184, 140, 25),
            .name_in = .{ .x = ex + 50, .y = ey + 56, .max_len = 19, .color = color2, .centre = true },
            .land_in = .{ .x = ex + 50, .y = ey + 94, .max_len = 19, .color = color2, .centre = true },
            .lx = lx,
            .ly = ly,
            .lb = undefined,
            .row_masks = undefined,
        };
        try button.initAll(EditButton, &self.buttons, ex, ey, edit_specs, &self.eb);
        errdefer for (&self.eb.values) |*b| b.deinit();
        try button.initAll(ListButton, &self.buttons, lx, ly, list_specs, &self.lb);
        self.eb.getPtr(.exit).hotkey = .escape;
        self.lb.getPtr(.exit).hotkey = .escape;
        for (&self.row_masks, 0..) |*m, i| m.* = .{
            .x = lx + 20,
            .y = ly + 37 + @as(f32, @floatFromInt(i * row_h)),
            .shape = &self.ruler_shape,
        };
        self.enterEdit();
        c.setCursor(0);
        return scene.Scene.of(Profile, self, true);
    }

    pub fn deinit(self: *Profile, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        for (&self.eb.values) |*b| b.deinit();
        for (&self.lb.values) |*b| b.deinit();
        rl.unloadImage(self.ruler_shape);
        self.arms.deinit();
        self.smallport.deinit();
        self.portraits.deinit();
        self.buttons.deinit();
        self.iface.deinit();
    }

    /// The start of WindowProfile.
    fn enterEdit(self: *Profile) void {
        self.mode = .edit;
        self.save = false;
        self.name_fresh = true;
        self.land_fresh = true;
        var buf: [96]u8 = undefined;
        self.name_in.set(self.h.name(&buf));
        self.name_in.active = false;
        self.land_in.set(self.h.land(&buf));
        self.land_in.active = false;
        self.eb.getPtr(.list).on = self.allow_list;
        self.updateDifficulty();
    }

    fn updateDifficulty(self: *Profile) void {
        const d = self.h.get(.difficulty);
        self.eb.getPtr(.diff_l).on = self.allow_list and d > 0;
        self.eb.getPtr(.diff_r).on = self.allow_list and d < profile.max_difficulty;
    }

    /// The start of WindowChoosePlayer.
    fn enterList(self: *Profile, c: *Ctx) void {
        self.mode = .list;
        self.n_slots = profile.listSlots(c.fs, &self.slots).len;
        self.page = 0;
        self.loadPage(c);
    }

    fn loadPage(self: *Profile, c: *Ctx) void {
        const first = self.page * rows;
        self.n_shown = 0;
        for (self.slots[first..@min(first + rows, self.n_slots)]) |slot| {
            self.shown[self.n_shown] = profile.load(c.fs, slot) orelse blk: {
                var h = Human.empty();
                h.set(.file_index, slot);
                break :blk h;
            };
            self.n_shown += 1;
        }
        // The scroll buttons appear only with more than six profiles.
        const many = self.n_slots > rows;
        self.lb.getPtr(.up).enabled = many;
        self.lb.getPtr(.down).enabled = many;
        self.lb.getPtr(.up).on = self.page > 0;
        self.lb.getPtr(.down).on = self.n_slots > first + rows;
    }

    pub fn update(self: *Profile, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        return switch (self.mode) {
            .edit => self.updateEdit(c, f),
            .list => self.updateList(c, f),
        };
    }

    fn updateEdit(self: *Profile, c: *Ctx, f: *const input.Frame) scene.Action {
        // bInput: no hotkeys while a field takes the keyboard.
        const bf = if (self.name_in.active or self.land_in.active) f.withoutKeys() else f.*;
        const h = self.h;
        const b = &self.eb;
        if (b.getPtr(.exit).check(&bf)) return self.closeEdit(c, false);
        if (self.allow_list and b.getPtr(.list).check(&bf)) return self.closeEdit(c, true);

        var clicked = false;
        const Step = struct { button: EditButton, field: profile.Field, n: i32, up: bool };
        const steps = [_]Step{
            .{ .button = .col1_l, .field = .color1, .n = profile.max_colors, .up = false },
            .{ .button = .col1_r, .field = .color1, .n = profile.max_colors, .up = true },
            .{ .button = .col2_l, .field = .color2, .n = profile.max_colors, .up = false },
            .{ .button = .col2_r, .field = .color2, .n = profile.max_colors, .up = true },
            .{ .button = .herb_l, .field = .sign, .n = profile.max_signs, .up = false },
            .{ .button = .herb_r, .field = .sign, .n = profile.max_signs, .up = true },
            .{ .button = .face_l, .field = .avatar, .n = profile.max_avatars, .up = false },
            .{ .button = .face_r, .field = .avatar, .n = profile.max_avatars, .up = true },
        };
        for (steps) |s| if (b.getPtr(s.button).check(&bf)) {
            const v = h.get(s.field);
            h.set(s.field, if (s.up) next(v, s.n) else prev(v, s.n));
            clicked = true;
        };
        if (self.allow_list) {
            const d = h.get(.difficulty);
            if (b.getPtr(.diff_l).check(&bf) and d > 0) {
                h.set(.difficulty, d - 1);
                clicked = true;
            }
            if (b.getPtr(.diff_r).check(&bf) and h.get(.difficulty) < profile.max_difficulty) {
                h.set(.difficulty, h.get(.difficulty) + 1);
                clicked = true;
            }
            self.updateDifficulty();
        }

        const p = f.pointer;
        if (self.name_mask.check(p) == .click) {
            if (self.name_fresh) self.name_in.clear();
            self.name_fresh = false;
            self.land_in.active = false;
            self.name_in.active = true;
            self.save = true;
        } else if (self.name_mask.hot()) self.hint.phrase(c, ph_name_hint, p.pos);
        if (self.land_mask.check(p) == .click) {
            if (self.land_fresh) self.land_in.clear();
            self.land_fresh = false;
            self.name_in.active = false;
            self.land_in.active = true;
            self.save = true;
        } else if (self.land_mask.hot()) self.hint.phrase(c, ph_land_hint, p.pos);
        _ = self.diff_mask.check(p);
        if (self.diff_mask.hot()) self.hint.phrase(c, ph_diff_hint, p.pos);
        _ = self.name_in.check(f);
        _ = self.land_in.check(f);

        // bClick: the arms changed; the fields stop taking the keyboard.
        if (clicked) {
            self.name_in.active = false;
            self.land_in.active = false;
            self.save = true;
        }
        for (&b.values) |*btn| if (btn.hint > 0 and btn.hot()) self.hint.phrase(c, btn.hint, p.pos);
        return .none;
    }

    /// The end of WindowProfile: the names are taken only after an edit.
    fn closeEdit(self: *Profile, c: *Ctx, to_list: bool) scene.Action {
        const h = self.h;
        if (self.save) {
            h.setName(self.name_in.text());
            h.setLand(self.land_in.text());
            h.set(.enter_data, 3);
        }
        h.save(c.fs) catch |e| std.log.err("cannot save the profile: {t}", .{e});
        if (!self.allow_list) {
            self.closed.* = true;
            return .pop;
        }
        if (to_list) {
            self.enterList(c);
            return .none;
        }
        // The end of ProfileOperator.
        profile.saveActiveIndex(c.fs, h.fileIndex()) catch |e| std.log.err("cannot write save/game.con: {t}", .{e});
        sound.setVolume(h.get(.snd_volume));
        c.music.setVolume(h.get(.mus_volume));
        self.closed.* = true;
        return .pop;
    }

    fn updateList(self: *Profile, c: *Ctx, f: *const input.Frame) scene.Action {
        const p = f.pointer;
        if (self.lb.getPtr(.exit).check(f)) {
            self.enterEdit();
            return .none;
        }
        for (self.row_masks[0 .. self.n_shown + 1], 0..) |*m, i| {
            if (m.check(p) != .click) continue;
            if (i == 0) {
                self.h.set(.file_index, -1);
                self.h.clearNames(c.proto.phrase(ph_anonymous), c.proto.phrase(ph_homeland), &c.rand);
            } else self.h.* = self.shown[i - 1];
            self.enterEdit();
            return .none;
        }
        if (self.lb.getPtr(.up).check(f)) {
            self.page -= 1;
            self.loadPage(c);
        }
        if (self.lb.getPtr(.down).check(f)) {
            self.page += 1;
            self.loadPage(c);
        }
        if (self.lb.getPtr(.exit).hot()) self.hint.phrase(c, self.lb.getPtr(.exit).hint, p.pos);
        return .none;
    }

    pub fn draw(self: *Profile, c: *Ctx) void {
        // draw_trans_sprite of the screen at 128 over black.
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 128 });
        switch (self.mode) {
            .edit => self.drawEdit(c),
            .list => self.drawList(c),
        }
        self.hint.draw(c, &self.iface);
    }

    fn drawEdit(self: *Profile, c: *Ctx) void {
        const x = self.ex;
        const y = self.ey;
        const win = self.iface.texIndex(gui_profile);
        gfx.drawTex(win, x, y);
        const h = self.h;
        c.font1.drawCentered(c.proto.phrase(if (h.fileIndex() == -1) ph_new_player else ph_profile), x + 141, y + 25, color1);
        c.font1.drawCentered(difficultyName(c, h.get(.difficulty)), x + 140, y + 191, color1);
        gfx.drawTex(self.portraits.texIndex(avatarIndex(h.get(.avatar))), x + 73, y + 215);
        const c1 = h.get(.color1);
        const c2 = h.get(.color2);
        const sign = h.get(.sign);
        self.arms.drawHerb(c1, c2, sign, x + 124, y + 140);
        self.arms.drawHerb(c1, c2, sign, x + 22, y + 220);
        self.arms.drawHerb(c1, c2, sign, x + 217, y + 220);
        // The banners are drawn on the window bitmap and cut at its edges.
        const clip: rl.Rectangle = .{ .x = x, .y = y, .width = gfx.texW(win), .height = gfx.texH(win) };
        self.arms.drawTownFlag(c1, c2, sign, x + 219 - 50 + 15, y + 6, clip);
        self.arms.drawTownFlag(c1, c2, sign, x + 17 - 50 + 3, y + 6, clip);
        rl.drawRectangleV(.{ .x = x + 38, .y = y + 147 }, .{ .x = 20, .y = 20 }, heraldry.color(c1));
        rl.drawRectangleV(.{ .x = x + 219, .y = y + 147 }, .{ .x = 20, .y = 20 }, heraldry.color(c2));
        self.name_in.draw(&c.font1, &self.iface);
        self.land_in.draw(&c.font1, &self.iface);
        for (&self.eb.values) |*b| b.draw(&self.buttons, c.tick);
    }

    fn drawList(self: *Profile, c: *Ctx) void {
        const x = self.lx;
        const y = self.ly;
        gfx.drawTex(self.iface.texIndex(gui_choose_player), x, y);
        c.font1.drawCentered(c.proto.phrase(ph_choose), x + 190, y + 19, color1);
        const ruler = self.iface.texIndex(gui_ruler_bar);
        gfx.drawTex(ruler, x + 20, y + 37);
        gfx.drawTex(self.smallport.texIndex(new_player_avatar + max_hero_pic), x + 25, y + 41);
        const random = self.arms.flag.texIndex(heraldry.flag_herb_random);
        gfx.drawTex(random, x + 88, y + 51);
        gfx.drawTex(random, x + 312, y + 51);
        c.font1.drawCentered(c.proto.phrase(ph_new_player), x + 217, y + 61, color1);
        var nb: [96]u8 = undefined;
        for (self.shown[0..self.n_shown], 1..) |*h, i| {
            const dy: f32 = @floatFromInt(i * row_h);
            gfx.drawTex(ruler, x + 20, y + 37 + dy);
            gfx.drawTex(self.smallport.texIndex(avatarIndex(h.get(.avatar))), x + 25, y + 41 + dy);
            self.arms.drawHuman(h, x + 88, y + 51 + dy);
            self.arms.drawHuman(h, x + 312, y + 51 + dy);
            c.font1.drawCentered(h.name(&nb), x + 217, y + 51 + dy, color1);
            c.font1.drawCentered(difficultyName(c, h.get(.difficulty)), x + 217, y + 71 + dy, color1);
        }
        for (self.row_masks[0 .. self.n_shown + 1]) |m| if (m.hot())
            rl.drawRectangleV(.{ .x = m.x, .y = m.y }, .{ .x = gfx.texW(ruler), .y = gfx.texH(ruler) }, row_lit);
        for (&self.lb.values) |*b| b.draw(&self.buttons, c.tick);
    }
};

test "arrows wrap around" {
    try std.testing.expectEqual(@as(i32, 15), prev(0, 16));
    try std.testing.expectEqual(@as(i32, 0), next(15, 16));
    try std.testing.expectEqual(@as(i32, 5), next(4, 16));
    try std.testing.expectEqual(@as(usize, 16 + 20), avatarIndex(40));
}
