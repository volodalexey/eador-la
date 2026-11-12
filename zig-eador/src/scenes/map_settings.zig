//! Настройки создаваемого мира: WindowMapSettings (Windower.cpp:6435).
//!
//! Тип ландшафта и тип границ (стрелки), десять ползунков 0..4.  «Принять»
//! пишет значения в `map` (локальная копия экрана новой игры) и тип границ
//! в профиль (iMapBorders); «Отмена» ничего не меняет.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const profile = @import("../data/profile.zig");
const MapSliders = @import("../data/newgame.zig").MapSliders;
const button = @import("../ui/button.zig");
const Button = button.Button;
const Mask = @import("../ui/mask.zig").Mask;
const ScrollBar = @import("../ui/scrollbar.zig").ScrollBar;
const Hint = @import("../ui/hint.zig").Hint;

const gui_map_settings = 129;
/// stuff.dat 027Borders0..030Borders3.
const stuff_borders = 27;
/// MAX_MAP_TYPE.
const max_map_type = 2;
const ph_title = 356;
const ph_labels = 357; // 357..368
const ph_amount = 373; // 373..377: мало .. очень много
const ph_neutrals = 378; // 378..382
const ph_type = 382; // + Type: 383 Остров, 384 Обычный
const msg_hints = 95; // 95..106
const dx = 204;
const dy = 74;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };

const Btn = enum { accept, cancel, default, type_l, type_r, border_l, border_r };
const specs: std.enums.EnumArray(Btn, button.Spec) = .init(.{
    .accept = .{ .x = 370, .y = 336, .normal = 305, .active = 306, .pressed = 307, .inactive = 308, .hint = 354 },
    .cancel = .{ .x = 305, .y = 336, .normal = 84, .active = 85, .pressed = 86, .inactive = 86, .hint = 106 },
    .default = .{ .x = 239, .y = 336, .normal = 87, .active = 88, .pressed = 90, .inactive = 89, .hint = 355 },
    .type_l = button.left(34, 70),
    .type_r = button.right(200, 70),
    .border_l = button.left(72, 310),
    .border_r = button.right(153, 310),
});

fn barPos(i: usize) [2]f32 {
    // scrMain[0], [1] in the first column below the type; [2..9] in the
    // other two columns.
    if (i < 2) return .{ 50, 101 + @as(f32, @floatFromInt((i + 1) * dy)) };
    const k = i - 2;
    return .{ 50 + @as(f32, @floatFromInt((1 + k / 4) * dx)), 101 + @as(f32, @floatFromInt((k % 4) * dy)) };
}

pub const MapSettings = struct {
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    stuff: gfx.Sheet,
    x: f32,
    y: f32,
    b: std.enums.EnumArray(Btn, Button),
    bars: [10]ScrollBar,
    masks: [12]Mask,
    map: *profile.Map,
    map_type: i32,
    border: i32,
    old_cursor: u32,
    hint: Hint = .{},

    pub fn create(c: *Ctx, map: *profile.Map) !scene.Scene {
        const self = try c.gpa.create(MapSettings);
        errdefer c.gpa.destroy(self);
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer buttons.deinit();
        var stuff = try gfx.Sheet.load(c.fs, "stuff");
        errdefer stuff.deinit();
        const win = iface.texIndex(gui_map_settings) orelse return error.MissingMapSettings;
        const x: f32 = @floatFromInt(512 - @divTrunc(win.width, 2));
        const y: f32 = @floatFromInt(384 - @divTrunc(win.height, 2));
        self.* = .{
            .iface = iface,
            .buttons = buttons,
            .stuff = stuff,
            .x = x,
            .y = y,
            .b = undefined,
            .bars = undefined,
            .masks = undefined,
            .map = map,
            .map_type = std.math.clamp(map.type, 1, max_map_type),
            .border = std.math.clamp(c.human.get(.map_borders), 0, 3),
            .old_cursor = c.cursor,
        };
        try button.initAll(Btn, &self.buttons, x, y, specs, &self.b);
        errdefer for (&self.b.values) |*k| k.deinit();
        const v = MapSliders.from(map.*).v;
        var done: usize = 0;
        errdefer for (self.bars[0..done]) |*s| s.deinit();
        for (&self.bars, 0..) |*s, i| {
            const p = barPos(i);
            s.* = try ScrollBar.init(&self.buttons, x + p[0], y + p[1], 140, 7, v[i], 0, 4);
            done += 1;
        }
        for (&self.masks, 0..) |*m, i| {
            const h: f32 = if (i == 3) 87 else 55;
            m.* = .rect(x + 42 + @as(f32, @floatFromInt((i / 4) * dx)), y + 47 + @as(f32, @floatFromInt((i % 4) * dy)), 175, h);
        }
        c.setCursor(0);
        return scene.Scene.of(MapSettings, self, true);
    }

    pub fn deinit(self: *MapSettings, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        for (&self.bars) |*s| s.deinit();
        for (&self.b.values) |*k| k.deinit();
        self.stuff.deinit();
        self.buttons.deinit();
        self.iface.deinit();
    }

    pub fn update(self: *MapSettings, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        self.hint.clear();
        for (&self.bars) |*s| if (s.state == .dragging) {
            _ = s.check(f);
            return .none;
        };
        const b = &self.b;
        if (b.getPtr(.cancel).check(f)) return .pop;
        if (b.getPtr(.type_l).check(f)) self.map_type = if (self.map_type > 1) self.map_type - 1 else max_map_type;
        if (b.getPtr(.type_r).check(f)) self.map_type = if (self.map_type < max_map_type) self.map_type + 1 else 1;
        if (b.getPtr(.border_l).check(f)) self.border = if (self.border > 0) self.border - 1 else 3;
        if (b.getPtr(.border_r).check(f)) self.border = if (self.border < 3) self.border + 1 else 0;
        if (b.getPtr(.accept).check(f)) {
            var s: MapSliders = undefined;
            for (&s.v, self.bars) |*v, bar| v.* = bar.value;
            s.apply(self.map);
            self.map.type = self.map_type;
            c.human.set(.map_borders, self.border);
            return .pop;
        }
        if (b.getPtr(.default).check(f)) {
            for (&self.bars, MapSliders.defaults.v) |*bar, v| bar.setValue(v);
            self.map_type = 2;
            self.border = 0;
        }
        for (&self.bars) |*s| _ = s.check(f);
        const p = f.pointer;
        for (&self.masks, 0..) |*m, i| {
            _ = m.check(p);
            if (m.hot()) self.hint.message(c, msg_hints + i, p.pos);
        }
        for (&b.values) |*k| if (k.hint > 0 and k.hot()) self.hint.phrase(c, k.hint, p.pos);
        return .none;
    }

    pub fn draw(self: *MapSettings, c: *Ctx) void {
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 128 });
        const x = self.x;
        const y = self.y;
        gfx.drawTex(self.iface.texIndex(gui_map_settings), x, y);
        const ph = c.proto;
        const f1 = &c.font1;
        f1.drawCentered(ph.phrase(ph_title), x + 332, y + 20, color1);
        for (0..12) |i| f1.drawCentered(ph.phrase(ph_labels + i), x + 127 + @as(f32, @floatFromInt((i / 4) * dx)), y + 55 + @as(f32, @floatFromInt((i % 4) * dy)), color1);
        f1.drawCentered(ph.phrase(@intCast(ph_type + self.map_type)), x + 127, y + 81, color2);
        for (self.bars, 0..) |bar, i| {
            const v: usize = @intCast(bar.value);
            // The value is printed above its bar; arSet[7] (independents)
            // has its own words.
            const word: usize = if (i == 7) ph_neutrals else ph_amount;
            const p = barPos(i);
            f1.drawCentered(ph.phrase(word + v), x + p[0] + 77, y + p[1] - 20, color2);
        }
        gfx.drawTex(self.stuff.texIndex(stuff_borders + @as(usize, @intCast(self.border))), x + 96, y + 300);
        for (&self.b.values) |*k| k.draw(&self.buttons, c.tick);
        for (&self.bars) |*s| s.draw(&self.iface, &self.buttons, c.tick);
        self.hint.draw(c, &self.iface);
    }
};

test "bar positions follow the original layout" {
    try std.testing.expectEqual([2]f32{ 50, 175 }, barPos(0));
    try std.testing.expectEqual([2]f32{ 50, 249 }, barPos(1));
    try std.testing.expectEqual([2]f32{ 254, 101 }, barPos(2));
    try std.testing.expectEqual([2]f32{ 458, 323 }, barPos(9));
}
