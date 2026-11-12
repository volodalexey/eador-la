//! G_Object::StoneForm/Stoning and G_Stars, advanced by the battle tick.
const std = @import("std");
const rl = @import("raylib");

pub const Stone = struct {
    image: rl.Image,
    texture: rl.Texture2D,
    left: u8 = 20,

    /// Takes ownership of the RGBA8 image, including on failure.
    pub fn init(image: rl.Image) ?Stone {
        const texture = rl.loadTextureFromImage(image) catch {
            rl.unloadImage(image);
            return null;
        };
        return .{ .image = image, .texture = texture };
    }

    pub fn deinit(self: *Stone) void {
        rl.unloadTexture(self.texture);
        rl.unloadImage(self.image);
    }

    pub fn tick(self: *Stone) void {
        if (self.left == 0) return;
        self.left -= 1;
        const pixels: [*]u8 = @ptrCast(self.image.data);
        stoning(pixels[0..@intCast(self.image.width * self.image.height * 4)]);
        rl.updateTexture(self.texture, pixels);
    }
};

/// Each channel approaches the pixel's mean by five, for twenty ticks.
pub fn stoning(pixels: []u8) void {
    var i: usize = 0;
    while (i + 4 <= pixels.len) : (i += 4) {
        if (pixels[i + 3] == 0) continue;
        const mean = @divTrunc(@as(i32, pixels[i]) + pixels[i + 1] + pixels[i + 2], 3);
        var channels: [3]i32 = undefined;
        var near = true;
        for (&channels, pixels[i..][0..3]) |*out, in| {
            out.* = @as(i32, in) + 5 * @as(i32, std.math.sign(mean - in));
            if (@abs(mean - out.*) > 5) near = false;
        }
        for (channels, 0..) |value, k| pixels[i + k] = @intCast(if (near) mean else std.math.clamp(value, 0, 255));
    }
}

const Star = struct { life: i32 = 0, x: i32 = 0, y: i32 = 0, dx: i32 = 0 };

/// G_Stars of Make_Stars: the grey attack stars and the coloured ones of
/// a summon (180, 100, 250).
pub const Stars = struct {
    timer: u8,
    count: u8,
    /// Offset of the box the particles are born in, from the unit picture.
    ox: i32 = 10,
    oy: i32 = 0,
    color: [3]u8 = .{ 200, 200, 200 },
    particles: [50]Star = @splat(.{}),

    pub fn tick(self: *Stars, random: std.Random) bool {
        for (self.particles[0..self.count]) |*p| {
            if (p.life == 0) continue;
            p.life -= 1;
            p.x += p.dx;
            p.y += 1;
            if (random.uintLessThan(u8, 10) == 9) p.dx = @as(i32, random.uintLessThan(u8, 5)) - 2;
        }
        if (self.timer > 0) {
            self.timer -= 1;
            for (self.particles[0..self.count]) |*p| {
                if (p.life != 0) continue;
                p.* = .{ .life = 25, .x = random.uintLessThan(u8, 44), .y = random.uintLessThan(u8, 40), .dx = @as(i32, random.uintLessThan(u8, 5)) - 2 };
                break;
            }
        }
        for (self.particles[0..self.count]) |p| if (p.life > 0) return false;
        return self.timer == 0;
    }

    fn mix(base: u8, level: i32) u8 {
        return @intCast(std.math.clamp(base + @divTrunc((255 - @as(i32, base)) * level, 5), 0, 255));
    }

    fn dim(base: u8, by: i32) u8 {
        return @intCast(std.math.clamp(@divTrunc(@as(i32, base), by), 0, 255));
    }

    /// One pixel of G_Stars::Run: `kind` 0 is the dim point, 1 the point
    /// brightening towards white, 2 a point of the 7×7 sprite.
    fn pixel(self: *const Stars, x: i32, y: i32, kind: u2, level: i32) void {
        const c = self.color;
        const col: rl.Color = switch (kind) {
            0 => .{ .r = dim(c[0], level), .g = dim(c[1], level), .b = dim(c[2], level), .a = 255 },
            1 => .{ .r = mix(c[0], level), .g = mix(c[1], level), .b = mix(c[2], level), .a = 255 },
            else => blk: {
                const part: i32 = switch (level) {
                    0 => 1,
                    1 => 2,
                    else => 3,
                };
                break :blk .{ .r = dim(if (level == 0) 250 else c[0], part), .g = dim(if (level == 0) 250 else c[1], part), .b = dim(if (level == 0) 250 else c[2], part), .a = 255 };
            },
        };
        rl.drawPixel(x, y, col);
    }

    /// The 7×7 star of G_Stars, including its growth from a single pixel
    /// and its fading cross. (200, 200, 200) is the grey of the petrify and
    /// web attacks; a summon passes (180, 100, 250).
    pub fn draw(self: *const Stars, x: f32, y: f32) void {
        for (self.particles[0..self.count]) |p| {
            if (p.life == 0) continue;
            const px = @as(i32, @intFromFloat(x)) + self.ox + p.x;
            const py = @as(i32, @intFromFloat(y)) + self.oy + p.y;
            const t = p.life;
            if (t > 20) {
                self.pixel(px, py, 0, t - 20);
            } else if (t > 15) {
                const k = 21 - t;
                self.pixel(px, py, 1, k);
                const level = 6 - k;
                self.pixel(px - 1, py, 0, level);
                self.pixel(px + 1, py, 0, level);
                self.pixel(px, py - 1, 0, level);
                self.pixel(px, py + 1, 0, level);
            } else if (t > 10) {
                for (0..7) |iy| for (0..7) |ix| {
                    const dx = @as(i32, @intCast(ix)) - 3;
                    const dy = @as(i32, @intCast(iy)) - 3;
                    const ax = @abs(dx);
                    const ay = @abs(dy);
                    if ((ax == 3 and ay != 0) or (ay == 3 and ax != 0)) continue;
                    const level: i32 = if (ax == 0 and ay == 0) 0 else if (ax + ay == 1 or ax == 1 and ay == 1) 1 else 2;
                    self.pixel(px + dx, py + dy, 2, level);
                };
            } else if (t > 5) {
                const k = t - 5;
                self.pixel(px, py, 1, k);
                const level = 11 - t;
                self.pixel(px - 1, py, 0, level);
                self.pixel(px + 1, py, 0, level);
                self.pixel(px, py - 1, 0, level);
                self.pixel(px, py + 1, 0, level);
            } else {
                self.pixel(px, py, 0, 6 - t);
            }
        }
    }
};

/// G_Raise: the picture is drawn one line higher each tick until it stands.
pub const Raise = struct {
    image: rl.Image,
    texture: rl.Texture2D,
    /// The row of the picture currently at the top of the sprite.
    line: i32,

    /// Takes ownership of the RGBA8 image, including on failure.
    pub fn init(image: rl.Image) ?Raise {
        const texture = rl.loadTextureFromImage(image) catch {
            rl.unloadImage(image);
            return null;
        };
        return .{ .image = image, .texture = texture, .line = image.height };
    }

    pub fn deinit(self: *Raise) void {
        rl.unloadTexture(self.texture);
        rl.unloadImage(self.image);
    }

    /// One line up; true when the whole picture is shown.
    pub fn tick(self: *Raise) bool {
        if (self.line > 0) self.line -= 1;
        return self.line == 0;
    }

    pub fn draw(self: *const Raise, x: f32, y: f32, flip: bool) void {
        const w = f32i(self.texture.width);
        const h = f32i(self.texture.height);
        const shown = h - f32i(self.line);
        if (shown <= 0) return;
        // A negative width mirrors left to right, as drawTextureRec does
        // for the standing unit (rotate_sprite_v_flip of G_Raise).
        rl.drawTextureRec(self.texture, .{
            .x = 0,
            .y = f32i(self.line),
            .width = if (flip) -w else w,
            .height = shown,
        }, .{ .x = x, .y = y + f32i(self.line) }, .white);
    }
};

fn f32i(v: i32) f32 {
    return @floatFromInt(v);
}
