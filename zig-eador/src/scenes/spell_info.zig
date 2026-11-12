//! WindowSpellInfo (Windower.cpp:1637): the spell window over the darkened
//! screen: the spell picture left and right, the name, the mana and
//! stamina costs and the description in a UI_TextBox with scroll arrows.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Button = @import("../ui/button.zig").Button;
const text = @import("../ui/text.zig");
const castle_ui = @import("../ui/castle_ui.zig");
const scrollSize = @import("window_info.zig").scrollSize;

/// interface.dat 27SpellInfo; buttons.dat: exit 226..228, the text box
/// arrows 48..51 (up) and 52..55 (down).
const gui_spell_info = 27;
const butt_exit = 226;
const butt_up = 48;
const butt_down = 52;

/// TextBox.Init(x+31, y+112, 342, 193).
const box_dx = 31;
const box_dy = 112;
const box_w = 342;
const box_h = 193;

pub const SpellInfo = struct {
    spell: i32,
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    spells: gfx.Sheet,
    x: f32,
    y: f32,
    exit: Button,
    up: Button,
    down: Button,
    /// CurrentLine and ScrollSize.
    line: i32 = 0,
    scroll: i32 = 0,
    old_cursor: u32,

    pub fn create(c: *Ctx, spell: i32) !scene.Scene {
        if (spell <= 0 or spell >= c.proto.spells.len) return error.NoSpell;
        const self = try c.gpa.create(SpellInfo);
        errdefer c.gpa.destroy(self);
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        var buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer buttons.deinit();
        var spells = try gfx.Sheet.load(c.fs, "spells");
        errdefer spells.deinit();
        const wt = iface.texIndex(gui_spell_info) orelse return error.MissingSpellInfo;
        const x: f32 = @floatFromInt(512 - @divTrunc(wt.width, 2));
        const y: f32 = @floatFromInt(384 - @divTrunc(wt.height, 2));
        var exit = try Button.init(&buttons, x + 168, y + 342, butt_exit, butt_exit + 1, butt_exit + 2);
        errdefer exit.deinit();
        exit.hotkey = .escape;
        var up = try Button.init(&buttons, x + 185, y + 86, butt_up, butt_up + 1, butt_up + 2);
        errdefer up.deinit();
        up.inactive = butt_up + 3;
        var down = try Button.init(&buttons, x + 185, y + 313, butt_down, butt_down + 1, butt_down + 2);
        errdefer down.deinit();
        down.inactive = butt_down + 3;
        var buf: [text.max_lines][]const u8 = undefined;
        const n = text.wrap(&c.font2, c.proto.spells[@intCast(spell)].text, box_w, &buf).len;
        const scroll = scrollSize(n, lineStep(c), text.boxHeight(box_h));
        up.on = scroll > 0;
        down.on = scroll > 0;
        self.* = .{
            .spell = spell,
            .iface = iface,
            .buttons = buttons,
            .spells = spells,
            .x = x,
            .y = y,
            .exit = exit,
            .up = up,
            .down = down,
            .scroll = scroll,
            .old_cursor = c.cursor,
        };
        c.setCursor(0);
        return scene.Scene.of(SpellInfo, self, true);
    }

    fn lineStep(c: *Ctx) i32 {
        return @intFromFloat(c.font2.lineHeight() + 2);
    }

    pub fn deinit(self: *SpellInfo, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        self.down.deinit();
        self.up.deinit();
        self.exit.deinit();
        self.spells.deinit();
        self.buttons.deinit();
        self.iface.deinit();
    }

    pub fn update(self: *SpellInfo, _: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        if (self.exit.check(f)) return .pop;
        if (self.up.check(f) and self.line > 0) self.line -= 1;
        if (self.down.check(f) and self.line < self.scroll) self.line += 1;
        const p = f.pointer;
        const bx = self.x + box_dx;
        const by = self.y + box_dy;
        const bh: f32 = @floatFromInt(text.boxHeight(box_h));
        if (self.scroll > 0 and p.hover and p.pos.x >= bx - 5 and p.pos.x < bx + box_w + 5 and p.pos.y > by - 10 and p.pos.y < by + bh + 10) {
            if (p.wheel > 0 and self.line > 0) self.line -= 1;
            if (p.wheel < 0 and self.line < self.scroll) self.line += 1;
        }
        return .none;
    }

    pub fn draw(self: *SpellInfo, c: *Ctx) void {
        const sp = &c.proto.spells[@intCast(self.spell)];
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 255 - 190 });
        gfx.drawTex(self.iface.texIndex(gui_spell_info), self.x, self.y);
        const pic = self.spells.texIndex(@intCast(self.spell - 1));
        gfx.drawTex(pic, self.x + 21, self.y + 20);
        gfx.drawTex(pic, self.x + 328, self.y + 20);
        c.font1.drawCentered(sp.name, self.x + 200, self.y + 21, castle_ui.color1);
        var b: [16]u8 = undefined;
        c.font1.drawCentered(castle_ui.fmtInt(&b, sp.cost), self.x + 148, self.y + 56, castle_ui.color2);
        c.font1.drawCentered(castle_ui.fmtInt(&b, sp.stamina_cost), self.x + 260, self.y + 56, castle_ui.color2);
        const bx = self.x + box_dx;
        const by = self.y + box_dy;
        const bh = text.boxHeight(box_h);
        const step: f32 = @floatFromInt(lineStep(c));
        var buf: [text.max_lines][]const u8 = undefined;
        const lines = text.wrap(&c.font2, sp.text, box_w, &buf);
        rl.beginScissorMode(@intFromFloat(bx), @intFromFloat(by), box_w, bh);
        for (lines, 0..) |line, k| {
            const row = @as(f32, @floatFromInt(@as(i32, @intCast(k)) - self.line)) * step;
            if (row + step <= 0) continue;
            if (row >= @as(f32, @floatFromInt(bh))) break;
            c.font2.draw(line, bx, by + row, castle_ui.color1);
        }
        rl.endScissorMode();
        self.up.draw(&self.buttons, c.tick);
        self.down.draw(&self.buttons, c.tick);
        self.exit.draw(&self.buttons, c.tick);
    }
};
