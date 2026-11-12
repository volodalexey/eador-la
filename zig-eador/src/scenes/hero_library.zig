//! ScreenHeroLibrary (Screens.cpp:4054): the hero learns spells from his
//! ruler's library (GUI_LibraryBlock at 592, 101) into the book
//! (GUI_SpellBlock at 34, 520).  A library spell is taken by a click and
//! put into a slot of a high enough circle; with Ctrl it stays in the hand
//! for another slot.  Book spells are moved and swapped as on ScreenHero.
//! A scroll (item class 10) taken from the backpack is written into the
//! book by a click on a slot and is spent; a spell that was there goes to
//! the hand.  The trash throws away the item or spell in the hand (dialogs
//! 1462, 1463; no question for a spell the library has).  A right click
//! opens WindowSpellInfo or WindowItemInfo.
//!
//! Not ported: the tutorial hints (Tutorial.Library, SpellRank), YuPlay.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const Hint = @import("../ui/hint.zig").Hint;
const Button = @import("../ui/button.zig").Button;
const castle_ui = @import("../ui/castle_ui.zig");
const Sheets = castle_ui.Sheets;
const info_windows = @import("info_windows.zig");
const hero_ui = @import("../ui/hero_ui.zig");
const Dialog = @import("dialog.zig").Dialog;
const ItemInfo = @import("item_info.zig").ItemInfo;
const SpellInfo = @import("spell_info.zig").SpellInfo;
const Library = @import("castle.zig").Library;
const mapgen = @import("../game/mapgen.zig");
const Shard = mapgen.Shard;
const player = @import("../game/player.zig");
const proto_mod = @import("../data/proto.zig");
const hero_mod = @import("../game/hero.zig");
const Hero = hero_mod.Hero;
const equip = @import("../game/equip.zig");
const spellbook = @import("../game/spellbook.zig");
const movement = @import("../game/movement.zig");

/// interface.dat 32SpellTrade (left), 31Library (right).
const gui_spell_trade = 32;
const gui_library = 31;
/// buttons.dat: exit 226..228, the trash 229..232.
const butt_exit = 226;
const butt_trash = 229;
const dlg_delete_item = 1462;
const dlg_delete_spell = 1463;
const msg_delete = 4;

pub const HeroLibrary = struct {
    s: *Shard,
    k: usize,
    sh: Sheets,
    hs: hero_ui.HeroSheets,
    items: hero_ui.ItemBlock,
    info: hero_ui.InfoBlock,
    stat: hero_ui.StatBlock = .{ .x = 8, .y = 278 },
    book: hero_ui.SpellBlock,
    library: Library = .{ .x0 = 592, .y0 = 101 },
    trash: Button,
    exit1: Button,
    exit2: Button,
    /// SelectedItem with its slot; iScroll is the spell of a held scroll.
    held: ?equip.Held = null,
    /// SelectedSpell and SelectedSlot (from_library: from the library).
    spell: ?spellbook.Held = null,
    card: ?struct { item: i32, dur: i32 } = null,
    hint: Hint = .{},
    pointer: input.Pointer = .{},
    answer: ?usize = null,
    asking: bool = false,
    old_cursor: u32,

    pub fn create(c: *Ctx, s: *Shard, k: usize, exp_pct: i32) !scene.Scene {
        const self = try c.gpa.create(HeroLibrary);
        errdefer c.gpa.destroy(self);
        const h = &s.heroes.items[k];
        self.* = .{
            .s = s,
            .k = k,
            .sh = undefined,
            .hs = undefined,
            .items = undefined,
            .info = hero_ui.InfoBlock.init(c, h, 0, 0, movement.castleParam(s, c.proto, h.player, movement.abil_siege, true), movement.castleParam(s, c.proto, h.player, movement.abil_mobility, true), exp_pct),
            .book = undefined,
            .trash = undefined,
            .exit1 = undefined,
            .exit2 = undefined,
            .old_cursor = c.cursor,
        };
        self.sh = try Sheets.load(c);
        errdefer self.sh.deinit();
        self.hs = try hero_ui.HeroSheets.load(c);
        errdefer self.hs.deinit();
        self.items = try hero_ui.ItemBlock.init(&self.sh, 207, 14);
        errdefer self.items.deinit();
        self.book = try hero_ui.SpellBlock.init(&self.sh, 34, 520);
        errdefer self.book.deinit();
        self.trash = try Button.init(&self.sh.buttons, 314, 534, butt_trash, butt_trash + 1, butt_trash + 2);
        errdefer self.trash.deinit();
        self.trash.inactive = butt_trash + 3;
        self.trash.on = false;
        self.trash.hotkey = .delete;
        self.trash.sound = sound.dismiss;
        self.exit1 = try Button.init(&self.sh.buttons, 412, 634, butt_exit, butt_exit + 1, butt_exit + 2);
        errdefer self.exit1.deinit();
        self.exit1.hotkey = .escape;
        self.exit1.hint = 135;
        self.exit2 = try Button.init(&self.sh.buttons, 512 + 224, 689, butt_exit, butt_exit + 1, butt_exit + 2);
        self.exit2.hint = 135;
        c.setCursor(0);
        return scene.Scene.of(HeroLibrary, self, false);
    }

    pub fn deinit(self: *HeroLibrary, c: *Ctx) void {
        const h = self.hero();
        if (self.held) |held| h.inventory[held.slot] = .{ held.item, held.dur };
        if (self.spell) |sp| spellbook.putBack(c.proto, h, sp);
        c.setCursor(self.old_cursor);
        self.exit2.deinit();
        self.exit1.deinit();
        self.trash.deinit();
        self.library.deinit();
        self.book.deinit();
        self.items.deinit();
        self.hs.deinit();
        self.sh.deinit();
    }

    fn hero(self: *const HeroLibrary) *Hero {
        return &self.s.heroes.items[self.k];
    }

    fn lib(self: *const HeroLibrary) *const [player.library_size]i32 {
        return &self.s.rulers[@intCast(self.hero().player)].holdings.library;
    }

    /// iScroll: the spell of the scroll in the hand, 0 for none.
    fn scroll(self: *const HeroLibrary, pr: *const proto_mod.Proto) i32 {
        const held = self.held orelse return 0;
        if (held.item <= 0 or held.item >= pr.items.len) return 0;
        const it = &pr.items[@intCast(held.item)];
        return if (it.class == proto_mod.item_class_scroll) it.effects[0].p1 else 0;
    }

    /// The item in the hand goes back where it was taken.
    fn putItemBack(self: *HeroLibrary) void {
        if (self.held) |held| self.hero().inventory[held.slot] = .{ held.item, held.dur };
        self.held = null;
    }

    /// A spell in the hand goes back to an empty book slot it came from.
    fn putSpellBack(self: *HeroLibrary, c: *Ctx) void {
        if (self.spell) |sp| spellbook.putBack(c.proto, self.hero(), sp);
        self.spell = null;
    }

    fn refreshTrash(self: *HeroLibrary) void {
        self.trash.on = self.held != null or self.spell != null;
    }

    pub fn update(self: *HeroLibrary, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        const pr = c.proto;
        const h = self.hero();
        const p = f.pointer;
        self.pointer = p;
        self.hint.clear();
        self.card = null;
        if (self.asking) {
            self.asking = false;
            if (self.answer == 1) {
                if (self.held != null) {
                    self.held = null;
                    self.spell = null;
                } else self.spell = null;
            }
            self.answer = null;
            self.refreshTrash();
        }
        if (self.exit1.check(f) or self.exit2.check(f)) return .pop;

        // "Взятие заклинания в библиотеке".
        var info_spell: ?i32 = null;
        switch (self.library.update(c, self.lib(), p, &self.sh, &self.hint)) {
            .take => |sp| {
                self.putItemBack();
                self.putSpellBack(c);
                self.spell = .{ .spell = sp, .slot = spellbook.from_library };
                self.refreshTrash();
            },
            .info => |sp| info_spell = sp,
            .none => {},
        }

        // "Манипулирование заклинаниями".
        const bk = self.book.check(c, h, p, &self.hint);
        if (bk.click) |i| self.clickBook(c, i);
        if (bk.info) |sp| info_spell = sp;

        // "Манипулирование предметами".
        const inv = self.items.check(p);
        if (inv.under) |u| if (u > 0 and h.inventory[u][0] > 0) {
            self.card = .{ .item = h.inventory[u][0], .dur = h.inventory[u][1] };
        };
        if (inv.click) |k_in| {
            if (self.held) |held| {
                const k = if (k_in == 0) equip.dollTarget(pr, h, held.item) else k_in;
                const tr = &self.s.rulers[@intCast(h.player)].holdings.treasury;
                if (equip.dropOnHero(pr, h, tr, held, k, null)) |r| self.held = r.held;
            } else if (k_in > 0 and h.inventory[k_in][0] > 0) {
                self.putSpellBack(c);
                self.held = .{ .item = h.inventory[k_in][0], .dur = h.inventory[k_in][1], .slot = k_in, .owner = .hero };
                h.inventory[k_in] = .{ 0, 0 };
            }
            self.refreshTrash();
        }

        if (self.trash.check(f)) {
            if (self.held != null) {
                if (c.human.message(msg_delete) and dlg_delete_item < pr.dialogs.len) return self.ask(c, dlg_delete_item);
                self.held = null;
                self.spell = null;
            } else if (self.spell) |sp| {
                if (c.human.message(msg_delete) and !spellbook.inLibrary(self.lib(), sp.spell) and dlg_delete_spell < pr.dialogs.len) return self.ask(c, dlg_delete_spell);
                self.spell = null;
            }
            self.refreshTrash();
        }

        self.info.refresh(c, h);
        if (self.info.portraitClick(&self.hs, h, p)) return .{ .push = @import("army_info.zig").UnitWindow.createHero(c, h, self.info.exp_pct) catch return .none };
        const abil_click = self.info.update(c, h, p, &self.hint) orelse self.stat.update(c, h, p, &self.hint);
        if (self.exit1.hot()) self.hint.phrase(c, self.exit1.hint, p.pos);
        if (self.exit2.hot()) self.hint.phrase(c, self.exit2.hint, p.pos);
        if (info_spell) |sp| return .{ .push = SpellInfo.create(c, sp) catch return .none };
        if (inv.info) |k| if (k > 0 and h.inventory[k][0] > 0) {
            return .{ .push = ItemInfo.create(c, h.inventory[k][0], h.inventory[k][1]) catch return .none };
        };
        if (abil_click) |a| return info_windows.abilityAction(c, a.num, a.power);
        return .none;
    }

    fn ask(self: *HeroLibrary, c: *Ctx, dlg: usize) scene.Action {
        self.asking = true;
        return .{ .push = Dialog.create(c, dlg, .{ true, true, false, false }, &self.answer) catch {
            self.asking = false;
            return .none;
        } };
    }

    /// A click on book slot `i`: a scroll is written in, a spell in the
    /// hand is put there (Ctrl keeps a library spell), or the slot's spell
    /// is taken.
    fn clickBook(self: *HeroLibrary, c: *Ctx, i: usize) void {
        const pr = c.proto;
        const h = self.hero();
        const sc = self.scroll(pr);
        if (sc > 0) {
            if (spellbook.writeScroll(pr, h, sc, i)) |r| {
                self.held = null;
                self.spell = r.held;
            }
        } else if (self.spell) |sp| {
            const ctrl = rl.isKeyDown(.left_control) or rl.isKeyDown(.right_control);
            if (spellbook.drop(pr, h, sp, i, ctrl)) |r| self.spell = r.held;
        } else if (h.spell(i) > 0) {
            self.putItemBack();
            self.spell = .{ .spell = h.spell(i), .slot = @intCast(i) };
            h.setSpell(i, 0);
        }
        self.refreshTrash();
    }

    pub fn draw(self: *HeroLibrary, c: *Ctx) void {
        const pr = c.proto;
        const h = self.hero();
        rl.clearBackground(.black);
        gfx.drawTex(self.sh.iface.texIndex(gui_spell_trade), 0, 0);
        gfx.drawTex(self.sh.iface.texIndex(gui_library), 512, 0);
        self.library.draw(c, &self.sh, self.lib());
        self.items.draw(c, &self.sh, h, if (self.held) |held| held.item else 0);
        self.stat.draw(c, h);
        self.info.draw(c, &self.sh, &self.hs, h);
        const sc = self.scroll(pr);
        self.book.draw(c, &self.sh, h, if (sc > 0) sc else if (self.spell) |sp| sp.spell else 0);
        self.trash.draw(&self.sh.buttons, c.tick);
        self.exit1.draw(&self.sh.buttons, c.tick);
        self.exit2.draw(&self.sh.buttons, c.tick);
        if (self.held) |held| {
            castle_ui.drawHeld(&self.sh, self.sh.item(pr, held.item), self.pointer);
            // The scroll shows its spell at (10, 10) under the pointer.
            if (sc > 0 and self.pointer.hover) {
                gfx.drawTex(self.sh.spell(sc), @floor(self.pointer.pos.x) + 10, @floor(self.pointer.pos.y) + 10);
                gfx.drawTex(self.sh.iface.texIndex(castle_ui.gui.pointer), @floor(self.pointer.pos.x), @floor(self.pointer.pos.y));
            }
        }
        if (self.spell) |sp| castle_ui.drawHeld(&self.sh, self.sh.spell(sp.spell), self.pointer);
        if (self.card) |cd| {
            castle_ui.itemCard(c, &self.sh, self.pointer.pos, cd.item, cd.dur, h, 0, .{ 0, 0 }, .{ 0, 0 });
        } else self.hint.draw(c, &self.sh.iface);
    }
};
