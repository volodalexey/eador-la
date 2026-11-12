//! The dialogs of dcHero::Death (Hero.cpp:1578) for a human's hero whose
//! own unit fell, and the turn's dialog 1239 (MainProcess.cpp:8867) when
//! a body reaches the capital.  MainScreen drives them one dialog at a
//! time (`step`), as it does with the province events.
//!
//! Death after a won battle (1) or a draw (-2): dialog 30/1091 when the
//! ruler can pay the resurrection (resurrect now, carry home, bury), else
//! 31/1092 (carry home, bury).  After a defeat: dialog 27 (carry home,
//! abandon).  Burying asks dialog 28 first; "no" asks again.
//!
//! Not ported: the tutorial map's own dialogs (campaign dialogs 38..41).
const std = @import("std");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const sound = @import("../platform/sound.zig");
const Dialog = @import("dialog.zig").Dialog;
const event_mod = @import("../game/event.zig");
const player = @import("../game/player.zig");
const Market = player.Market;
const heroes = @import("../game/heroes.zig");
const movement = @import("../game/movement.zig");

const dlg_defeat = 27;
const dlg_bury = 28;
const dlg_win = 30;
const dlg_win_poor = 31;
const dlg_draw = 1091;
const dlg_draw_poor = 1092;
const dlg_arrived = 1239;
/// spell_sound.dat: the resurrection.
const snd_resurrect = 58;

pub const Step = union(enum) {
    push: scene.Scene,
    /// The hero stays in the list (alive or carried home).
    kept,
    /// The hero was buried: indices into Shard.heroes changed.
    buried,
};

pub const Death = struct {
    k: usize,
    win: i32,
    distance: i32,
    stage: enum { ask, confirm } = .ask,
    /// The dialog shown and whether its answers are shifted by one
    /// (31 and 1092 lack "resurrect").
    shown: usize = 0,
    shift: usize = 0,
    answer: ?usize = null,
    buf: [512]u8 = undefined,

    /// The start of Death: the order dropped, the distance measured.  A
    /// computer's hero is settled at once (null).
    pub fn begin(m: Market, w: *movement.Walker, k: usize, win: i32) ?Death {
        const d = heroes.deathDistance(m, w, k);
        if (m.ruler().kind != .human) {
            heroes.computerDeath(m, k, win, d);
            return null;
        }
        return .{ .k = k, .win = win, .distance = d };
    }

    fn open(self: *Death, c: *Ctx, index: usize, enabled: [4]bool, body: ?[]const u8) Step {
        self.shown = index;
        self.answer = null;
        const sc = (if (body) |b| Dialog.createText(c, index, enabled, &self.answer, b) else Dialog.create(c, index, enabled, &self.answer)) catch |e| {
            std.log.err("hero death dialog {d}: {t}", .{ index, e });
            return .kept;
        };
        return .{ .push = sc };
    }

    /// The next dialog, or how it ended.  Called again after each dialog
    /// closes.
    pub fn step(self: *Death, c: *Ctx, m: Market) Step {
        if (self.answer) |a| {
            self.answer = null;
            switch (self.stage) {
                .confirm => {
                    if (a == 1) {
                        heroes.bury(m.s, c.proto, self.k);
                        return .buried;
                    }
                    self.stage = .ask;
                },
                .ask => switch (a + self.shift) {
                    1 => if (self.win == 1 or self.win == -2) {
                        heroes.resurrect(m, self.k, false);
                        return .kept;
                    } else {
                        heroes.deliver(m.s, self.k, self.distance);
                        return .kept;
                    },
                    2 => if (self.win == 1 or self.win == -2) {
                        heroes.deliver(m.s, self.k, self.distance);
                        return .kept;
                    } else {
                        self.stage = .confirm;
                        return self.open(c, dlg_bury, .{ true, true, false, false }, null);
                    },
                    else => {
                        self.stage = .confirm;
                        return self.open(c, dlg_bury, .{ true, true, false, false }, null);
                    },
                },
            }
        }
        const pr = c.proto;
        const h = &m.s.heroes.items[self.k];
        const name = h.nameText(pr);
        if (self.win == 1 or self.win == -2) {
            const p = heroes.resurrectPrice(m, h);
            const rich = heroes.canPay(m, p);
            const index: usize = if (rich) (if (self.win < 0) dlg_draw else dlg_win) else (if (self.win < 0) dlg_draw_poor else dlg_win_poor);
            self.shift = if (rich) 0 else 1;
            const body = event_mod.format(&self.buf, pr.dialogs[index].text, &.{ .{ .s = name }, .{ .d = p[0] }, .{ .d = p[1] }, .{ .d = self.distance } });
            return self.open(c, index, .{ true, true, rich, false }, body);
        }
        self.shift = 0;
        const body = event_mod.format(&self.buf, pr.dialogs[dlg_defeat].text, &.{ .{ .s = name }, .{ .d = self.distance } });
        return self.open(c, dlg_defeat, .{ true, true, false, false }, body);
    }
};

/// Dialog 1239: the body is in the capital.  "Resurrect" is offered when
/// the ruler can pay and the capital is not besieged.
pub const Arrival = struct {
    k: usize,
    answer: ?usize = null,
    asked: bool = false,
    buf: [512]u8 = undefined,

    pub fn step(self: *Arrival, c: *Ctx, m: Market) ?scene.Scene {
        const h = &m.s.heroes.items[self.k];
        const p = heroes.resurrectPrice(m, h);
        if (self.asked) {
            if (self.answer == 1 and heroes.canPay(m, p)) {
                sound.play(.spell, snd_resurrect);
                heroes.resurrect(m, self.k, true);
            }
            return null;
        }
        self.asked = true;
        const pr = c.proto;
        const can = heroes.canPay(m, p) and movement.siege(m.s, m.s.capitals[@intCast(m.player)]) < 0;
        const body = event_mod.format(&self.buf, pr.dialogs[dlg_arrived].text, &.{ .{ .s = h.nameText(pr) }, .{ .d = p[0] }, .{ .d = p[1] } });
        return Dialog.createWith(c, dlg_arrived, .{ can, true, false, false }, &self.answer, .{ .text = body, .portrait = @intCast(@max(h.portrait, 0)) }) catch |e| {
            std.log.err("dialog {d}: {t}", .{ dlg_arrived, e });
            return null;
        };
    }
};
