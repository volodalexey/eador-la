//! Scene stack.  Each modal loop of the original (while (...) { if (Timer)
//! ... readkey() ... }) is a scene: `update` handles one frame of input and
//! time, `draw` paints the whole frame.  Only the top scene updates; scenes
//! below it are drawn first when the top one is an overlay.
const std = @import("std");
const input = @import("platform/input.zig");

pub const Ctx = @import("app.zig").Ctx;

pub const Action = union(enum) {
    none,
    /// Close this scene.
    pop,
    /// Open a scene above this one; it takes ownership of the pointer.
    push: Scene,
    /// Close this scene and open another in its place.
    replace: Scene,
    quit,
};

pub const Scene = struct {
    ptr: *anyopaque,
    vtable: *const VTable,
    /// Draw the scene below first (dialogs, hints).
    overlay: bool = false,

    pub const VTable = struct {
        update: *const fn (*anyopaque, *Ctx, *const input.Frame, f32) Action,
        draw: *const fn (*anyopaque, *Ctx) void,
        deinit: *const fn (*anyopaque, *Ctx) void,
    };

    /// Wraps a heap-allocated `*T` with update/draw/deinit methods.
    pub fn of(comptime T: type, p: *T, overlay: bool) Scene {
        const gen = struct {
            fn update(o: *anyopaque, c: *Ctx, f: *const input.Frame, dt: f32) Action {
                return T.update(@ptrCast(@alignCast(o)), c, f, dt);
            }
            fn draw(o: *anyopaque, c: *Ctx) void {
                T.draw(@ptrCast(@alignCast(o)), c);
            }
            fn deinit(o: *anyopaque, c: *Ctx) void {
                const self: *T = @ptrCast(@alignCast(o));
                if (@hasDecl(T, "deinit")) self.deinit(c);
                c.gpa.destroy(self);
            }
            const vt: VTable = .{ .update = update, .draw = draw, .deinit = deinit };
        };
        return .{ .ptr = p, .vtable = &gen.vt, .overlay = overlay };
    }
};

pub const max_depth = 16;

pub const Stack = struct {
    items: [max_depth]Scene = undefined,
    len: usize = 0,

    pub fn push(self: *Stack, s: Scene) !void {
        if (self.len == max_depth) return error.SceneStackFull;
        self.items[self.len] = s;
        self.len += 1;
    }

    pub fn top(self: *Stack) ?Scene {
        return if (self.len == 0) null else self.items[self.len - 1];
    }

    fn popOne(self: *Stack, c: *Ctx) void {
        self.len -= 1;
        const s = self.items[self.len];
        s.vtable.deinit(s.ptr, c);
    }

    /// Returns false when the game should exit.
    pub fn update(self: *Stack, c: *Ctx, f: *const input.Frame, dt: f32) !bool {
        const s = self.top() orelse return false;
        switch (s.vtable.update(s.ptr, c, f, dt)) {
            .none => {},
            .pop => self.popOne(c),
            .push => |n| try self.push(n),
            .replace => |n| {
                self.popOne(c);
                try self.push(n);
            },
            .quit => return false,
        }
        return self.len > 0;
    }

    pub fn draw(self: *Stack, c: *Ctx) void {
        if (self.len == 0) return;
        var first = self.len - 1;
        while (first > 0 and self.items[first].overlay) first -= 1;
        for (self.items[first..self.len]) |s| s.vtable.draw(s.ptr, c);
    }

    pub fn deinit(self: *Stack, c: *Ctx) void {
        while (self.len > 0) self.popOne(c);
    }
};
