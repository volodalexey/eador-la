//! Заставка (Eador.cpp:893-1010, SNOWBALL включён).
//!
//! Оригинал крутил `while (!keypressed() && Count < 85) if (Timer) {...}`,
//! таймер ItIsTime срабатывал раз в iRefreshDelay мс.  Здесь каждый такой
//! тик — вызов `step`.  Затемнение сделано, как в оригинале: полупрозрачный
//! чёрный прямоугольник рисуется поверх кадра каждый тик, а кадр между
//! тиками не очищается (как screen1).
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const menu = @import("menu.zig");

/// stuff.dat: логотипы издателей (bmpStuff[78+Logoz]) и «Bliss» (bmpStuff[67]).
const stuff_logo0 = 78;
const stuff_bliss = 67;
const logo_count = 2;
const art_frames = 14;
/// bmpBR: чёрный прямоугольник 600×400.
const br_w = 600;
const br_h = 400;

pub const Intro = struct {
    art: gfx.Sheet,
    stuff: gfx.Sheet,
    tick_s: f32,
    acc: f32 = 0,
    pending: u32 = 0,
    tick: i32 = 0,
    count: i32 = 0,
    logo: u32 = 0,
    english: bool,
    cleared: bool = false,
    played_71: bool = false,

    pub fn create(c: *Ctx) !scene.Scene {
        const self = try c.gpa.create(Intro);
        errdefer c.gpa.destroy(self);
        var art = try gfx.Sheet.load(c.fs, "art");
        errdefer art.deinit();
        const stuff = try gfx.Sheet.load(c.fs, "stuff");
        const delay = c.cfg.refresh_delay;
        self.* = .{
            .art = art,
            .stuff = stuff,
            .tick_s = @as(f32, @floatFromInt(delay)) / 1000,
            .english = std.mem.eql(u8, c.fs.lang(), "en"),
        };
        return scene.Scene.of(Intro, self, false);
    }

    pub fn deinit(self: *Intro, _: *Ctx) void {
        self.art.deinit();
        self.stuff.deinit();
    }

    pub fn update(self: *Intro, c: *Ctx, f: *const input.Frame, dt: f32) scene.Action {
        if (f.any() or self.count >= 85) {
            c.setCursor(1);
            return .{ .replace = menu.Menu.create(c) catch return .quit };
        }
        self.acc += dt;
        while (self.acc >= self.tick_s) : (self.acc -= self.tick_s) self.pending += 1;
        return .none;
    }

    pub fn draw(self: *Intro, _: *Ctx) void {
        if (!self.cleared) {
            rl.clearBackground(.black);
            self.cleared = true;
        }
        // Тики, накопленные за кадр; `count` может закончить заставку раньше.
        while (self.pending > 0 and self.count < 85) : (self.pending -= 1) self.step();
        self.pending = 0;
    }

    fn centered(t: ?rl.Texture2D) void {
        const tx = t orelse return;
        gfx.drawTex(tx, @floatFromInt(512 - @divTrunc(tx.width, 2)), @floatFromInt(384 - @divTrunc(tx.height, 2)));
    }

    /// draw_trans_sprite(screen1, bmpBR, ...) при set_trans_blender(a, ...).
    fn fade(a: u8) void {
        rl.drawRectangle(512 - br_w / 2, 384 - br_h / 2, br_w, br_h, .{ .r = 0, .g = 0, .b = 0, .a = a });
    }

    /// draw_sprite(screen1, bmpBR, 512-bmpArt[0]->w/2, 384-bmpArt[0]->h/2):
    /// оригинал ставит прямоугольник по размеру первого кадра арта.
    fn blackAtArt(self: *Intro) void {
        const w: i32 = if (self.art.texIndex(0)) |t| t.width else br_w;
        const h: i32 = if (self.art.texIndex(0)) |t| t.height else br_h;
        rl.drawRectangle(512 - @divTrunc(w, 2), 384 - @divTrunc(h, 2), br_w, br_h, .black);
    }

    fn artFrame(self: *Intro, i: usize) void {
        const t = self.art.texIndex(i) orelse return;
        const a0 = self.art.texIndex(0) orelse t;
        gfx.drawTex(t, @floatFromInt(512 - @divTrunc(a0.width, 2)), @floatFromInt(384 - @divTrunc(a0.height, 2)));
    }

    fn step(self: *Intro) void {
        self.tick += 1;
        if (self.logo < logo_count) {
            const c0 = @divTrunc(self.tick, 3);
            if (c0 == 0) centered(self.stuff.texIndex(stuff_logo0 + self.logo));
            if (c0 >= 20) fade(30);
            if (c0 > 30) {
                self.blackAtArt();
                self.tick = 0;
                self.logo += 1;
            }
            return;
        }

        self.count = @divTrunc(self.tick, 3);
        const n = self.count;
        if (n == 0) {
            if (self.english) {
                self.tick = 90;
            } else {
                self.artFrame(0);
                // Оригинал играл звук на каждом тике с Count==0 (дважды
                // наложением); здесь один раз.
                if (!self.played_71) sound.play(.unit, 71);
                self.played_71 = true;
            }
        } else if (n < art_frames) self.artFrame(@intCast(n));

        // Блендер, выставленный в конце прошлого тика: 50 (30 только после n == 50).
        if (n >= 30 and n < 40) fade(50);
        if (n == 40) {
            self.blackAtArt();
            sound.play(.spell, 46);
        }
        if (n >= 40 and n < 50) {
            centered(self.stuff.texIndex(stuff_bliss));
            fade(@intCast(250 - (n - 40) * 25));
        }
        if (n == 50) centered(self.stuff.texIndex(stuff_bliss));
        if (n >= 70) fade(50);
    }
};
