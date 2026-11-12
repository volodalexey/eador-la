//! «Создатели» (ScreenCredits, Screens.cpp:9861).
//!
//! Layers of the original: 0 the astral picture at x = 253, 1 floating
//! pictures (spells, items, castle buildings, abilities, outer buildings,
//! units) drifting right, 2 the A140Credits frame and the exit button,
//! 3 the credits text scrolling up in a 512×740 window at y = 14.
//! The text is built once from credits.var; one timer tick moves it 1 px.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const proto = @import("../data/proto.zig");
const Button = @import("../ui/button.zig").Button;

const gui_credits = 140;
/// buttons.dat: exit N / A / P.
const butt_exit = 226;
const view_h = 740;
const view_y = 14;
const rows = 6;
const slots = 20;

const title_empty: rl.Color = .{ .r = 250, .g = 210, .b = 20, .a = 255 };
const title: rl.Color = .{ .r = 220, .g = 220, .b = 20, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };

/// Sheets of the floating pictures.
const Kind = enum { spells, items, innerbld, ability, heroskills, outbuild, units };

const Floater = struct {
    kind: Kind = .spells,
    index: usize = 0,
    /// 0: free (bmpEmpty), > 1024: gone.
    x: i32 = 0,
    y: i32 = 0,
    w: i32 = 0,
    h: i32 = 0,
    opacity: u8 = 128,
};

pub const Credits = struct {
    iface: gfx.Sheet,
    astral: gfx.Sheet,
    buttons: gfx.Sheet,
    pics: [@typeInfo(Kind).@"enum".field_names.len]gfx.Sheet,
    exit: Button,
    floaters: [rows][slots]Floater = @splat(@splat(.{})),
    tick_s: f32,
    acc: f32 = 0,
    tick: u32 = 0,
    tick1: u32 = 0,
    /// Scroller: top of the text inside the window.
    scroller: i32 = view_h - 1,

    pub fn create(c: *Ctx) !scene.Scene {
        const self = try c.gpa.create(Credits);
        errdefer c.gpa.destroy(self);
        self.* = .{
            .iface = try gfx.Sheet.load(c.fs, "interface"),
            .astral = undefined,
            .buttons = undefined,
            .pics = undefined,
            .exit = undefined,
            .tick_s = @as(f32, @floatFromInt(c.cfg.refresh_delay)) / 1000,
        };
        errdefer self.iface.deinit();
        self.astral = try gfx.Sheet.load(c.fs, "astral");
        errdefer self.astral.deinit();
        self.buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer self.buttons.deinit();
        var loaded: usize = 0;
        errdefer for (self.pics[0..loaded]) |*s| s.deinit();
        inline for (@typeInfo(Kind).@"enum".field_names, 0..) |fname, i| {
            self.pics[i] = try gfx.Sheet.load(c.fs, fname);
            loaded += 1;
        }
        const ew: f32 = if (self.buttons.texIndex(butt_exit)) |t| @floatFromInt(t.width) else 64;
        const eh: f32 = if (self.buttons.texIndex(butt_exit)) |t| @floatFromInt(t.height) else 58;
        self.exit = try Button.init(&self.buttons, 1024 - ew, 768 - eh, butt_exit, butt_exit + 1, butt_exit + 2);
        self.exit.hotkey = .escape;
        // ScreenCredits: iGameMode 5 (Memory01), Screens.cpp:9866.
        c.music.setMode(c.fs, .memory);
        return scene.Scene.of(Credits, self, false);
    }

    pub fn deinit(self: *Credits, c: *Ctx) void {
        self.exit.deinit();
        for (&self.pics) |*s| s.deinit();
        self.buttons.deinit();
        self.astral.deinit();
        self.iface.deinit();
        // Back to the main menu (iGameMode 0).
        c.music.setMode(c.fs, .menu);
    }

    fn sheet(self: *Credits, k: Kind) *gfx.Sheet {
        return &self.pics[@backingInt(k)];
    }

    pub fn update(self: *Credits, c: *Ctx, f: *const input.Frame, dt: f32) scene.Action {
        if (self.exit.check(f)) return .pop;
        self.acc += dt;
        while (self.acc >= self.tick_s) : (self.acc -= self.tick_s) self.step(c, f.pointer);
        return .none;
    }

    fn step(self: *Credits, c: *Ctx, p: input.Pointer) void {
        self.scroller -= 1;
        if (self.scroller < -1400) self.scroller = view_h;
        self.tick += 1;
        self.tick1 += 1;
        for (&self.floaters) |*row| for (row) |*o| {
            if (o.x <= 0) continue;
            o.x += 1;
            const v = 200 - @divTrunc(@as(i32, @intCast(@abs(768 - o.x))), 2);
            o.opacity = @intCast(std.math.clamp(v, 1, 255));
            const px: i32 = @intFromFloat(p.pos.x);
            const py: i32 = @intFromFloat(p.pos.y);
            if (p.hover and px > o.x and py > o.y and px < o.x + o.w and py < o.y + o.h) o.opacity = 250;
        };
        if (self.tick1 == 100) {
            self.tick1 = 0;
            const n: u32 = @intCast(c.proto.units.len - 1);
            self.spawn(5, .units, proto.UnitType.picIndex(1 + c.rand.random(n)), 550);
        }
        if (self.tick == 60) {
            const pr = c.proto;
            self.spawn(0, .spells, (1 + c.rand.random(@intCast(pr.spells.len - 1))) - 1, 50);
            const item = pr.items[1 + c.rand.random(@intCast(pr.max_pure_item))];
            self.spawn(1, .items, if (item.pic) |pic| pic.index else null, 150);
            self.spawn(2, .innerbld, 1 + c.rand.random(@intCast(pr.inner_buildings.len - 1)), 250);
            const num = 1 + c.rand.random(@intCast(pr.abilities.len - 1));
            if (pr.abilities[num].num > 1000)
                self.spawn(3, .heroskills, 1 + c.rand.random(140), 350)
            else
                self.spawn(3, .ability, num, 350);
            self.spawn(4, .outbuild, proto.OuterBuilding.iconIndex(1 + c.rand.random(@intCast(pr.outer_buildings.len - 1))), 450);
            self.tick = 0;
        }
    }

    /// The first free slot of the row gets the picture at x = 512 - w.
    fn spawn(self: *Credits, row: usize, kind: Kind, index: ?usize, y: i32) void {
        const i = index orelse return;
        const t = self.sheet(kind).texIndex(i) orelse return;
        for (&self.floaters[row]) |*o| {
            if (o.x != 0 and o.x <= 1024) continue;
            o.* = .{ .kind = kind, .index = i, .x = 512 - t.width, .y = y, .w = t.width, .h = t.height };
            return;
        }
    }

    pub fn draw(self: *Credits, c: *Ctx) void {
        rl.clearBackground(.black);
        gfx.drawTex(self.astral.texIndex(0), 253, 0);
        for (&self.floaters) |*row| for (row) |o| {
            if (o.x <= 0) continue;
            const t = self.sheet(o.kind).texIndex(o.index) orelse continue;
            rl.drawTextureV(t, .{ .x = @floatFromInt(o.x), .y = @floatFromInt(o.y) }, .{ .r = 255, .g = 255, .b = 255, .a = o.opacity });
        };
        gfx.drawTex(self.iface.texIndex(gui_credits), 0, 0);
        self.exit.draw(&self.buttons, c.tick);
        rl.beginScissorMode(0, view_y, 512, view_h);
        drawText(c, @floatFromInt(view_y + self.scroller));
        rl.endScissorMode();
    }

    /// The bmpCredits layout (Screens.cpp:9894): names in two columns.
    fn drawText(c: *Ctx, top: f32) void {
        var dy: f32 = top;
        for (c.proto.credits) |cr| {
            if (cr.names.len == 0) {
                c.font1.drawCentered(cr.title, 256, dy, title_empty);
            } else {
                c.font1.drawCentered(cr.title, 256, dy, title);
                dy += 18;
                var i: usize = 0;
                while (i + 1 < cr.names.len) : (i += 2) {
                    c.font2.drawCentered(cr.names[i], 512 / 4 + 10, dy, color2);
                    c.font2.drawCentered(cr.names[i + 1], 512 * 3 / 4 - 10, dy, color2);
                    dy += 15;
                }
                if (cr.names.len % 2 > 0) {
                    c.font2.drawCentered(cr.names[cr.names.len - 1], 256, dy, color2);
                    dy += 16;
                }
            }
            dy += 22;
        }
    }
};
