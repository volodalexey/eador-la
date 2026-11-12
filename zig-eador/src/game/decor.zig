//! dcProvince::DrawPic (Province.cpp:3587, GOG FUN_00457760): the picture
//! of a province's town and outer buildings with the layer-2 nature
//! objects (trees) and landmarks that stand in front of them.  The other
//! nature is baked into the map picture; this one covers the town.
//! Every choice is Random(Max, P1, P2, P3, P4) of the game seed and the
//! map point, so the picture is the same on every load.  Ported from
//! recomp orig/decor.zig.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const mapgen = @import("mapgen.zig");
const province = @import("province.zig");
const terrain = province.terrain;
const keyed4 = @import("../data/rand.zig").keyed4;

/// RGBA8, top row first; alpha 0 is transparent.
pub const Img = struct { w: u32, h: u32, px: []const u8 };

/// A picture placed in the 280×240 scratch bitmap (the province centre at
/// X = 140, Y = 120).
pub const Sprite = struct { img: Img, x: i32, y: i32 };

pub const Composite = struct {
    w: u32,
    h: u32,
    /// RGBA8, owned by the caller's allocator.
    px: []u8,
    /// PicX, PicY.
    map_x: i32,
    map_y: i32,
};

const half_w = 140;
const half_h = 120;
const scratch_w = 2 * half_w;
const scratch_h = 2 * half_h;
/// arBorder[..][150].
const prof_w = 150;
const n_sprites = province.max_bld + 1;

/// GetProvinceIndex over the map picture, computed once; points outside
/// it are looked up directly.
pub const Ids = struct {
    w: u32,
    h: u32,
    ids: []u16,

    pub fn init(gpa: std.mem.Allocator, s: *const mapgen.Shard) !Ids {
        const w: u32 = @intCast(s.width());
        const h: u32 = @intCast(s.height());
        const ids = try gpa.alloc(u16, @as(usize, w) * h);
        for (0..h) |y| for (0..w) |x| {
            ids[y * w + x] = @intCast(std.math.clamp(s.lookup(@intCast(x), @intCast(y)), 0, 0xffff));
        };
        return .{ .w = w, .h = h, .ids = ids };
    }

    pub fn deinit(self: *Ids, gpa: std.mem.Allocator) void {
        gpa.free(self.ids);
    }

    pub fn at(self: *const Ids, s: *const mapgen.Shard, x: i32, y: i32) i32 {
        if (x >= 0 and y >= 0 and x < self.w and y < self.h) return self.ids[@as(usize, @intCast(y)) * self.w + @as(usize, @intCast(x))];
        return s.lookup(x, y);
    }
};

/// What DrawPic reads besides the province.
pub const World = struct {
    s: *const mapgen.Shard,
    pr: *const Proto,
    ids: *const Ids,
    /// CurGame.iSeed.
    seed: u32,
    /// The global iSeed of the picture variant (the menu counter; the
    /// recomp calls it the frame counter).  Not saved.
    variant: i32 = 0,

    fn terrainAt(self: *const World, p: i32) i32 {
        if (p <= 0 or p >= self.s.provinces.len) return -1;
        return self.s.provinces[@intCast(p)].terrain;
    }
};

fn opaquePx(img: Img, x: usize, y: usize) bool {
    return img.px[(y * img.w + x) * 4 + 3] != 0;
}

fn blit(dst: []u8, img: Img, x: i32, y: i32) void {
    for (0..img.h) |sy| {
        const dy = y + @as(i32, @intCast(sy));
        if (dy < 0 or dy >= scratch_h) continue;
        for (0..img.w) |sx| {
            const dx = x + @as(i32, @intCast(sx));
            if (dx < 0 or dx >= scratch_w) continue;
            const o = (sy * img.w + sx) * 4;
            if (img.px[o + 3] == 0) continue;
            const d = (@as(usize, @intCast(dy)) * scratch_w + @as(usize, @intCast(dx))) * 4;
            @memcpy(dst[d..][0..4], img.px[o..][0..4]);
        }
    }
}

/// Roll(false, iSeed, P2, P3, P4) over `ids` with `weights`.
fn roll(seed: u32, ids: []const u16, weights: []const i32, p2: i32, p3: i32, p4: i32) ?u16 {
    var sum: i64 = 0;
    for (weights) |w| sum += w;
    if (sum <= 0 or ids.len == 0) return null;
    const r: i64 = keyed4(seed, @intCast(sum), @bitCast(seed), p2, p3, p4);
    var acc: i64 = 0;
    var i: usize = 0;
    while (true) {
        if (i >= weights.len) return ids[ids.len - 1];
        acc += weights[i];
        i += 1;
        if (acc >= r + 1) break;
    }
    i -= 1;
    while (i < weights.len and weights[i] == 0) i += 1;
    return if (i < ids.len) ids[i] else null;
}

/// DrawPic of province `n` whose town and buildings are `sprites` (null
/// slots are empty).  `nature` gives nature.dat object k as `get(k) ?Img`.
/// Null for water and mountains.
pub fn compose(gpa: std.mem.Allocator, w: *const World, nature: anytype, n: i32, sprites: [n_sprites]?Sprite) !?Composite {
    const pv = &w.s.provinces[@intCast(n)];
    if (pv.terrain == terrain.water or pv.terrain == terrain.mountain) return null;
    var any = false;
    for (sprites) |sp| any = any or sp != null;
    const seed = w.seed;
    const pr = w.pr;

    const scratch = try gpa.alloc(u8, scratch_w * scratch_h * 4);
    defer gpa.free(scratch);
    @memset(scratch, 0);

    // arBorder (the lowest opaque row per column), arDown, arDown1,
    // arLeft, arRight, arUp.
    // arBorder[A][x] is read past its row in the original: one flat array.
    var border: [n_sprites * prof_w]i32 = @splat(0);
    var down: [n_sprites]i32 = @splat(-1);
    var down1: [n_sprites]i32 = @splat(1000);
    var left: [n_sprites]i32 = @splat(-1);
    var right: [n_sprites]i32 = @splat(-1);
    var up: [n_sprites]i32 = @splat(-1);
    for (sprites, 0..) |sp, a| {
        const s = sp orelse continue;
        for (0..s.img.w) |x| {
            var v: i32 = 0;
            var y = s.img.h;
            while (y > 0) {
                y -= 1;
                if (!opaquePx(s.img, x, y)) continue;
                v = @as(i32, @intCast(y)) + s.y;
                if (left[a] < 0) left[a] = @as(i32, @intCast(x)) + s.x;
                right[a] = @as(i32, @intCast(x)) + s.x;
                break;
            }
            if (x < prof_w) border[a * prof_w + x] = v;
            down[a] = @max(down[a], v);
            if (v > 0 and v < down1[a]) down1[a] = v;
        }
        const hw = @divTrunc(@as(i32, @intCast(s.img.w)), 2);
        const hh = @divTrunc(@as(i32, @intCast(s.img.h)), 2);
        if (down[a] < 0) down[a] = s.y + hh;
        if (down1[a] == 1000) down1[a] = s.y + hh;
        if (left[a] < 0) left[a] = s.x + hw;
        if (right[a] < 0) right[a] = s.x + hw;
        rows: for (0..s.img.h) |y| for (0..s.img.w) |x| {
            if (opaquePx(s.img, x, y)) {
                up[a] = @as(i32, @intCast(y)) + s.y;
                break :rows;
            }
        };
        if (up[a] < 0) up[a] = s.y + hh;
    }

    var cand: [256]u16 = undefined;
    var weights: [256]i32 = undefined;
    var j: i32 = 0;
    while (j < scratch_h + 40) : (j += 1) {
        var need_check = false;
        for (sprites, 0..) |sp, a| {
            const s = sp orelse continue;
            if (j == down1[a]) blit(scratch, s.img, s.x, s.y);
            if (j > down1[a]) need_check = true;
        }
        if (!need_check) continue;
        const my = j + pv.y - half_h;
        var i: i32 = -30;
        while (i < scratch_w + 30) : (i += 1) {
            const mx = i + pv.x - half_w;
            const q = w.ids.at(w.s, mx, my);
            if (q <= 0 or q >= w.s.provinces.len) continue;
            const pq = &w.s.provinces[@intCast(q)];
            // The landmark of layer 2 at its point instead of a tree.
            const lm = pq.buildings[province.max_bld];
            if (lm > 0 and lm < pr.landmarks.len and pq.bld_place[province.max_bld][0] == mx and pq.bld_place[province.max_bld][1] == my) {
                const l = &pr.landmarks[@intCast(lm)];
                if (l.layer == 2) {
                    if (nature.get(l.pic)) |img| blit(scratch, img, i - l.dx, j - l.dy);
                    continue;
                }
            }
            const t = pq.terrain;
            if (t == terrain.hill or t < 0 or t >= proto_mod.land_types) continue;
            const density = pr.terr_layer[@intCast(t)][2];
            if (density <= 0) continue;
            const at_centre = mx == pq.x and ((my == pq.y and t == terrain.hill) or (my == pq.bld_place[0][1] and t == terrain.mountain));
            if (!(keyed4(seed, 20000, mx, my, @bitCast(seed), 2) < density or at_centre)) continue;

            var nc: usize = 0;
            for (pr.map_objects[1..], 1..) |*o, z| {
                if (o.layer != 2 or o.landmark or o.land[@intCast(t)] <= 0) continue;
                var ok = true;
                // In every fourth forest (by seed) one kind of tree.
                if (t == terrain.forest and o.kind > 0 and (seed +% @as(u32, @bitCast(q))) % 4 == 0 and
                    @as(i32, @intCast((seed / 7 +% @as(u32, @bitCast(q))) % 2)) + 1 != o.kind) ok = false;
                const yy = my - @divTrunc(o.dy, 2);
                for ([3]i32{ mx - @divTrunc(o.dx, 2), mx + @divTrunc(o.dx, 2), mx }) |xx| {
                    const r = w.ids.at(w.s, xx, yy);
                    const rt = w.terrainAt(r);
                    if (r == 0 or rt < 0 or rt >= proto_mod.land_types or o.land[@intCast(rt)] == 0) ok = false;
                }
                if ((ok or at_centre) and nc < cand.len) {
                    cand[nc] = @intCast(z);
                    weights[nc] = o.land[@intCast(t)];
                    nc += 1;
                }
            }
            if (nc == 0) continue;
            const k = roll(seed, cand[0..nc], weights[0..nc], my, mx, 2) orelse continue;
            const o = &pr.map_objects[k];
            if (o.n_pics == 0) continue;
            const v = keyed4(seed, @intCast(o.n_pics), my, w.variant, 2, mx);
            if (v >= o.n_pics) continue;
            const img = nature.get(o.pics[v]) orelse continue;
            const sw: i32 = @intCast(img.w);
            const xp = i - @divTrunc(sw, 2);
            const yp = j - @as(i32, @intCast(img.h));
            const shift: i32 = if (t == terrain.forest) 15 else 3;

            // In front of a building whose base is above this row and
            // not behind any. If there are no buildings, draw always.
            var draw = !any;
            var forbid = false;
            for (sprites, 0..) |sp, a| {
                const s = sp orelse continue;
                if (!(j > down1[a] and xp + sw > left[a] and xp < right[a])) continue;
                const col = i - s.x;
                if (i > left[a] and i < right[a] + @as(i32, @intCast(s.img.w))) {
                    const fi = @as(i32, @intCast(a * prof_w)) + col;
                    const bv = if (fi >= 0 and fi < border.len) border[@intCast(fi)] else 0;
                    if (bv > 0) {
                        if (col >= 0 and col < prof_w) {
                            if (j > bv + shift) draw = true;
                            if (j < bv + shift) forbid = true;
                        }
                        continue;
                    }
                }
                const c1 = xp - s.x;
                if (c1 >= 0 and c1 < prof_w and xp > left[a] and xp < right[a]) {
                    const b = border[a * prof_w + @as(usize, @intCast(c1))];
                    if (b > 0 and j > b + shift) draw = true;
                }
                const c2 = xp + sw - s.x;
                if (c2 >= 0 and c2 < prof_w and xp + sw > left[a] and xp + sw < right[a]) {
                    const b = border[a * prof_w + @as(usize, @intCast(c2))];
                    if (b > 0 and j > b + shift) draw = true;
                }
            }
            if (draw and !forbid) blit(scratch, img, xp, yp);
        }
    }

    // Only each building's own box is taken from the scratch bitmap.
    // If there are no buildings, take the whole scratch for trees.
    var x1: i32 = if (any) 1000 else 0;
    var y1: i32 = if (any) 1000 else 0;
    var x2: i32 = if (any) -1 else scratch_w;
    var y2: i32 = if (any) -1 else scratch_h;
    for (sprites, 0..) |sp, a| {
        if (sp == null) continue;
        x1 = @min(x1, left[a]);
        x2 = @max(x2, right[a]);
        y1 = @min(y1, up[a]);
        y2 = @max(y2, down[a]);
    }
    if (x2 <= x1 or y2 <= y1) return null;
    const ow: u32 = @intCast(x2 - x1);
    const oh: u32 = @intCast(y2 - y1);
    const out = try gpa.alloc(u8, @as(usize, ow) * oh * 4);
    @memset(out, 0);
    if (any) {
        for (sprites, 0..) |sp, a| {
            if (sp == null) continue;
            var y = @max(up[a], 0);
            while (y < @min(down[a], scratch_h)) : (y += 1) {
                var x = @max(left[a], 0);
                while (x < @min(right[a], scratch_w)) : (x += 1) {
                    const s = (@as(usize, @intCast(y)) * scratch_w + @as(usize, @intCast(x))) * 4;
                    const d = (@as(usize, @intCast(y - y1)) * ow + @as(usize, @intCast(x - x1))) * 4;
                    @memcpy(out[d..][0..4], scratch[s..][0..4]);
                }
            }
        }
    } else {
        // No buildings: copy the whole scratch with trees
        for (0..scratch_h) |y| {
            for (0..scratch_w) |x| {
                const s = (y * scratch_w + x) * 4;
                if (scratch[s + 3] == 0) continue; // skip transparent pixels
                @memcpy(out[s..][0..4], scratch[s..][0..4]);
            }
        }
    }
    return .{ .w = ow, .h = oh, .px = out, .map_x = pv.x - half_w + x1, .map_y = pv.y - half_h + y1 };
}

/// Where DrawPic puts a picture of `w`×`h` centred on map point (x, y)
/// of province `pv`.
pub fn placeAt(pv: *const province.Province, w: u32, h: u32, x: i32, y: i32) [2]i32 {
    return .{ x - @divTrunc(@as(i32, @intCast(w)), 2) - pv.x + half_w, y - @divTrunc(@as(i32, @intCast(h)), 2) - pv.y + half_h };
}

const tst = std.testing;

test "roll picks by weight and skips zero weights" {
    const ids = [_]u16{ 4, 7, 9 };
    for (0..50) |k| {
        const r = roll(8097, &ids, &[_]i32{ 0, 5, 5 }, @intCast(k), 3, 2).?;
        try tst.expect(r == 7 or r == 9);
    }
    try tst.expectEqual(@as(?u16, null), roll(8097, &ids, &[_]i32{ 0, 0, 0 }, 1, 2, 3));
}
