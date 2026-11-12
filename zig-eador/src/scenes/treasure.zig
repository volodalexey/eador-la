//! WindowTreasure (Windower.cpp:3187): the gold, gems and up to five items
//! a site gives, over the darkened screen.  The loot itself is rolled by
//! game/hero_task.zig; the window only shows it and hands the items to
//! WindowItemInfo on a click.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const proto = @import("../data/proto.zig");
const Mask = @import("../ui/mask.zig").Mask;
const Button = @import("../ui/button.zig").Button;
const castle_ui = @import("../ui/castle_ui.zig");
const text = @import("../ui/text.zig");
const ItemInfo = @import("item_info.zig").ItemInfo;

/// interface.dat 57Treasure.
const gui_treasure = 57;
const butt_exit = 226;
/// message 1 ("Сокровища").
const msg_treasure = 1;
const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };

pub const TreasureWindow = struct {
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    sheets: castle_ui.Sheets,
    loot_mask: ?rl.Image,
    gold: i32,
    gem: i32,
    items: [5][2]i32,
    n_items: usize,
    /// site (index into Shard.shops for a shop site) and level.
    site: i32,
    level: i32,
    x: f32,
    y: f32,
    exit: Button,
    masks: [5]Mask = undefined,
    text_buf: [512]u8 = undefined,
    done: *bool,
    old_cursor: u32,

    pub fn create(c: *Ctx, t: anytype, done: *bool) !scene.Scene {
        const self = try c.gpa.create(TreasureWindow);
        errdefer c.gpa.destroy(self);
        self.* = .{
            .iface = undefined,
            .buttons = undefined,
            .sheets = undefined,
            .loot_mask = null,
            .gold = t.gold,
            .gem = t.gem,
            .items = t.items,
            .n_items = t.n_items,
            .site = t.site,
            .level = t.level,
            .x = 0,
            .y = 0,
            .exit = undefined,
            .done = done,
            .old_cursor = c.cursor,
        };
        self.iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer self.iface.deinit();
        self.buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer self.buttons.deinit();
        self.sheets = try castle_ui.Sheets.load(c);
        errdefer self.sheets.deinit();
        self.loot_mask = self.iface.imageIndex(0);
        const wt = self.iface.texIndex(gui_treasure) orelse return error.MissingTreasureWindow;
        self.x = @floatFromInt(512 - @divTrunc(wt.width, 2));
        self.y = @floatFromInt(384 - @divTrunc(wt.height, 2));
        self.exit = try Button.init(&self.buttons, self.x + 425, self.y + 222, butt_exit, butt_exit + 1, butt_exit + 2);
        self.exit.hotkey = .escape;
        for (&self.masks, 0..) |*m, i| {
            const col: f32 = @floatFromInt(i % 3);
            const row: f32 = @floatFromInt(i / 3);
            m.* = .{ .x = self.x + 303 + col * 60, .y = self.y + 158 + row * 60, .w = 58, .h = 58 };
        }
        c.setCursor(0);
        return scene.Scene.of(TreasureWindow, self, true);
    }

    pub fn deinit(self: *TreasureWindow, c: *Ctx) void {
        c.setCursor(self.old_cursor);
        if (self.loot_mask) |img| rl.unloadImage(img);
        self.sheets.deinit();
        self.buttons.deinit();
        self.iface.deinit();
    }

    pub fn update(self: *TreasureWindow, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        if (self.exit.check(f)) {
            self.done.* = true;
            return .pop;
        }
        for (&self.masks, 0..) |*m, i| {
            if (i >= self.n_items or self.items[i][0] <= 0) continue;
            if (self.loot_mask) |*img| m.shape = img;
            const r = m.check(f.pointer);
            if (r != .none) return .{ .push = ItemInfo.create(c, self.items[i][0], self.items[i][1]) catch return .none };
        }
        return .none;
    }

    pub fn draw(self: *TreasureWindow, c: *Ctx) void {
        rl.drawRectangle(0, 0, 1024, 768, .{ .r = 0, .g = 0, .b = 0, .a = 255 - 190 });
        gfx.drawTex(self.iface.texIndex(gui_treasure), self.x, self.y);
        const body = if (msg_treasure < c.proto.messages.len) c.proto.messages[msg_treasure] else "";
        var box = text.Box.init(self.x + 198, self.y + 41, 277, 96, &c.font2, color1);
        _ = box.draw(body);
        var b: [16]u8 = undefined;
        c.font1.drawCentered(std.fmt.bufPrint(&b, "{d}", .{self.gold}) catch "", self.x + 258, self.y + 195, color2);
        c.font1.drawCentered(std.fmt.bufPrint(&b, "{d}", .{self.gem}) catch "", self.x + 258, self.y + 256, color2);
        for (self.items[0..self.n_items], 0..) |it, i| {
            if (it[0] <= 0) continue;
            const col: f32 = @floatFromInt(i % 3);
            const row: f32 = @floatFromInt(i / 3);
            gfx.drawTex(self.sheets.item(c.proto, it[0]), self.x + 309 + col * 60, self.y + 164 + row * 60);
            if (self.masks[i].hot()) gfx.drawTex(self.iface.texIndex(1), self.x + 303 + col * 60, self.y + 158 + row * 60);
        }
        self.exit.draw(&self.buttons, c.tick);
    }
};
