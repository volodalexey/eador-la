//! Предварительный вид созданного осколка, пока не перенесены отрисовка
//! ландшафта (вторая часть GenerateMap) и MainScreen: провинции залиты
//! цветом местности, границы и замки отмечены.  Карту можно двигать
//! перетаскиванием и стрелками; Esc возвращает в меню.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const mapgen = @import("../game/mapgen.zig");
const province = @import("../game/province.zig");
const heraldry = @import("../ui/heraldry.zig");

const border: [4]u8 = .{ 20, 20, 20, 255 };
const name_color: rl.Color = .{ .r = 250, .g = 250, .b = 250, .a = 255 };
const scroll_step = 12;

fn terrainColor(t: i32) [4]u8 {
    return switch (t) {
        province.terrain.water => .{ 34, 66, 130, 255 },
        province.terrain.plain => .{ 120, 150, 70, 255 },
        province.terrain.forest => .{ 40, 100, 45, 255 },
        province.terrain.hill => .{ 150, 125, 80, 255 },
        province.terrain.swamp => .{ 85, 105, 80, 255 },
        province.terrain.mountain => .{ 125, 120, 115, 255 },
        else => .{ 0, 0, 0, 255 },
    };
}

/// The province of every pixel.  Between two border cells of a row
/// GetProvinceIndex gives the same province, so one lookup per gap is
/// enough.
pub fn rasterize(gpa: std.mem.Allocator, s: *const mapgen.Shard) ![]i32 {
    const w: usize = @intCast(s.width());
    const h: usize = @intCast(s.height());
    const out = try gpa.alloc(i32, w * h);
    @memset(out, 0);
    var xs: std.ArrayList(i32) = .empty;
    defer xs.deinit(gpa);
    for (0..@min(h, s.grid.rows())) |y| {
        xs.clearRetainingCapacity();
        var k = s.grid.heads[y];
        while (k >= 0) {
            const c = s.grid.cells.items[@intCast(k)];
            k = c.next;
            try xs.append(gpa, c.x);
        }
        std.mem.sort(i32, xs.items, {}, std.sort.asc(i32));
        const yi: i32 = @intCast(y);
        var i: usize = 0;
        while (i + 1 < xs.items.len) : (i += 1) {
            const a = xs.items[i];
            const b = xs.items[i + 1];
            if (b <= a) continue;
            const p = s.lookup(a + 1, yi);
            if (p == 0) continue;
            var x = @max(a + 1, 0);
            while (x <= b and x < w) : (x += 1) out[y * w + @as(usize, @intCast(x))] = p;
        }
    }
    return out;
}

pub const MapPreview = struct {
    shard: mapgen.Shard,
    tex: rl.Texture2D,
    scroll: rl.Vector2 = .{ .x = 0, .y = 0 },
    last: ?rl.Vector2 = null,
    /// Herb colour of each player's castle.
    colors: [mapgen.max_players + 1]i32 = @splat(0),

    /// Takes ownership of `shard`.  `colors[i]` is player i's first herb
    /// colour.
    pub fn create(c: *Ctx, shard: mapgen.Shard, colors: [mapgen.max_players + 1]i32) !scene.Scene {
        var s = shard;
        errdefer s.deinit();
        const self = try c.gpa.create(MapPreview);
        errdefer c.gpa.destroy(self);
        const w: usize = @intCast(s.width());
        const h: usize = @intCast(s.height());
        const idx = try rasterize(c.gpa, &s);
        defer c.gpa.free(idx);
        const px = try c.gpa.alloc(u8, w * h * 4);
        defer c.gpa.free(px);
        for (0..h) |y| for (0..w) |x| {
            const p = idx[y * w + x];
            var col = terrainColor(if (p > 0) s.terrainOf(p) else -1);
            // A pixel next to another province is a border.
            if (p > 0 and ((x + 1 < w and idx[y * w + x + 1] != p) or (y + 1 < h and idx[(y + 1) * w + x] != p))) col = border;
            px[(y * w + x) * 4 ..][0..4].* = col;
        };
        const tex = try gfx.textureFromRgba(@intCast(w), @intCast(h), px);
        self.* = .{ .shard = s, .tex = tex, .colors = colors };
        // Start at the first player's castle.
        const home = s.provinces[@intCast(s.capitals[1])];
        self.scroll = .{ .x = @floatFromInt(home.x - 512), .y = @floatFromInt(home.y - 384) };
        self.clamp();
        c.setCursor(1);
        return scene.Scene.of(MapPreview, self, false);
    }

    pub fn deinit(self: *MapPreview, _: *Ctx) void {
        rl.unloadTexture(self.tex);
        self.shard.deinit();
    }

    fn clamp(self: *MapPreview) void {
        const mw: f32 = @floatFromInt(@max(self.tex.width - 1024, 0));
        const mh: f32 = @floatFromInt(@max(self.tex.height - 768, 0));
        self.scroll.x = std.math.clamp(self.scroll.x, 0, mw);
        self.scroll.y = std.math.clamp(self.scroll.y, 0, mh);
    }

    pub fn update(self: *MapPreview, _: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        if (f.key(.escape)) return .pop;
        const p = f.pointer;
        if (p.primary.down) {
            if (self.last) |l| {
                self.scroll.x -= p.pos.x - l.x;
                self.scroll.y -= p.pos.y - l.y;
            }
            self.last = p.pos;
        } else self.last = null;
        if (rl.isKeyDown(.left)) self.scroll.x -= scroll_step;
        if (rl.isKeyDown(.right)) self.scroll.x += scroll_step;
        if (rl.isKeyDown(.up)) self.scroll.y -= scroll_step;
        if (rl.isKeyDown(.down)) self.scroll.y += scroll_step;
        self.clamp();
        return .none;
    }

    pub fn draw(self: *MapPreview, c: *Ctx) void {
        rl.clearBackground(.black);
        const o: rl.Vector2 = .{ .x = -@floor(self.scroll.x), .y = -@floor(self.scroll.y) };
        rl.drawTextureV(self.tex, o, .white);
        const s = &self.shard;
        for (s.provinces[1..]) |pv| {
            const x = o.x + @as(f32, @floatFromInt(pv.x));
            const y = o.y + @as(f32, @floatFromInt(pv.y));
            if (x < -200 or x > 1224 or y < -50 or y > 818) continue;
            if (pv.player > 0) {
                const col = heraldry.color(self.colors[@intCast(pv.player)]);
                rl.drawRectangleV(.{ .x = x - 9, .y = y - 9 }, .{ .x = 18, .y = 18 }, .black);
                rl.drawRectangleV(.{ .x = x - 7, .y = y - 7 }, .{ .x = 14, .y = 14 }, col);
                c.font1.drawCentered(pv.name, x, y + 12, name_color);
            } else if (pv.sites[0].index != 0) {
                rl.drawCircleV(.{ .x = x, .y = y }, 4, .{ .r = 230, .g = 200, .b = 40, .a = 255 });
            }
        }
    }
};
