//! ScreenHero (Screens.cpp:2449): the hero's portrait and stats
//! (GUI_InfoBlock, GUI_StatBlock), the doll and backpack (GUI_ItemBlock),
//! the squad (GUI_UnitBlock), the spell book (GUI_SpellBlock) and the
//! skills (GUI_SkillBlock).  Items are put on, taken off, moved and thrown
//! away; spells are moved between the book's slots (game/spellbook.zig)
//! and thrown away (dialog 1463 unless the library has the spell).  A
//! right click on an item opens WindowItemInfo, on a spell WindowSpellInfo;
//! a click on the class opens WindowInfo with its description, on a skill
//! WindowSkillInfo, on a stat, the level or the life WindowAbilityInfo.
//! Units are moved between the squad's slots and dismissed (dialog 1461);
//! a right click on one opens WindowUnitInfo (its pHero is NULL outside
//! slot 0, so only the squad's morale comes from the hero).
//! GUI_QuestBlock lists the quests; a click shows the journal entry
//! (dialog 494) and may abandon the quest (dialog 495).
//! Opened from the map, buttDismiss buries the hero after dialog 28.
//!
//! Not ported: the Ally unit of the call (WindowTakeLoot), the encounter
//! of a quest found done (CheckQuestDone), YuPlay.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const sound = @import("../platform/sound.zig");
const Hint = @import("../ui/hint.zig").Hint;
const Mask = @import("../ui/mask.zig").Mask;
const Button = @import("../ui/button.zig").Button;
const castle_ui = @import("../ui/castle_ui.zig");
const Sheets = castle_ui.Sheets;
const gui = castle_ui.gui;
const info_windows = @import("info_windows.zig");
const army_info = @import("army_info.zig");
const army_ui = @import("../ui/army_ui.zig");
const army = @import("../game/army.zig");
const quest = @import("../game/quest.zig");
const event_mod = @import("../game/event.zig");
const Unit = @import("../game/unit.zig").Unit;
const hero_ui = @import("../ui/hero_ui.zig");
const Dialog = @import("dialog.zig").Dialog;
const ItemInfo = @import("item_info.zig").ItemInfo;
const SpellInfo = @import("spell_info.zig").SpellInfo;
const WindowInfo = @import("window_info.zig").WindowInfo;
const formatStr = @import("army_info.zig").formatStr;
const spellbook = @import("../game/spellbook.zig");
const mapgen = @import("../game/mapgen.zig");
const Shard = mapgen.Shard;
const hero_mod = @import("../game/hero.zig");
const Hero = hero_mod.Hero;
const equip = @import("../game/equip.zig");
const movement = @import("../game/movement.zig");
const heroes = @import("../game/heroes.zig");

/// interface.dat 3HeroScreen, 25SpellClosed.
const gui_hero_screen = 3;
const gui_spell_closed = 25;
/// buttons.dat: exit, the trash (229..232).
const butt_exit = 226;
const butt_trash = 229;
/// dialog.var 1462 "Выбросить предмет?", 1463 the spell; asked when
/// arMess[4] is on.
const dlg_delete_item = 1462;
const dlg_delete_unit = 1461;
/// dialog.var 494 the quest's journal entry, 495 "abandon the quest?".
const dlg_quest = 494;
const dlg_abandon = 495;
/// Phrases: 65 "Задания", 73 "Выполнено" / the progress, 74 "Не выполнено".
const ph_quests = 65;
const ph_done = 73;
const ph_not_done = 74;
const quest_done_color: rl.Color = .{ .r = 0, .g = 120, .b = 0, .a = 255 };
const dlg_delete_spell = 1463;
/// buttDismiss (buttons.dat 166..169, hint 142) and dialog 28.
const butt_dismiss = 166;
const ph_dismiss = 142;
const dlg_bury = 28;
const msg_delete = 4;

pub const HeroScreen = struct {
    s: *Shard,
    k: usize,
    sh: Sheets,
    hs: hero_ui.HeroSheets,
    items: hero_ui.ItemBlock,
    info: hero_ui.InfoBlock,
    stat: hero_ui.StatBlock = .{ .x = 8, .y = 278 },
    trash: Button,
    exit: Button,
    unit_shape: rl.Image,
    spell_shape: rl.Image,
    book: hero_ui.SpellBlock,
    units: army_ui.UnitBlock = army_ui.UnitBlock.init(unit_x, unit_y),
    quests: [hero_mod.quest_limit]Mask = undefined,
    /// The quest of the open dialog 494 / 495.
    quest_ask: ?struct { i: usize, confirm: bool } = null,
    quest_buf: [hero_mod.quest_limit][2][160]u8 = undefined,
    quest_text: [hero_mod.quest_limit][2][]const u8 = @splat(.{ "", "" }),
    skills: [14]Mask = undefined,
    held: ?equip.Held = null,
    /// SelectedSpell and SelectedSlot.
    spell: ?spellbook.Held = null,
    /// SelectedUnit and SelectedSlot (0 after a swap with no free slot).
    unit: ?struct { unit: Unit, slot: usize } = null,
    card: ?struct { item: i32, dur: i32 } = null,
    hint: Hint = .{},
    hint_buf: [128]u8 = undefined,
    pointer: input.Pointer = .{},
    answer: ?usize = null,
    asking: bool = false,
    /// buttDismiss (Activate(-1) unless CanDelete) and its dialog 28.
    dismiss: Button,
    dismissing: bool = false,
    /// The hero was dismissed: `k` is no longer his.
    gone: bool = false,
    old_cursor: u32,

    const unit_x = 59;
    const unit_y = 411;
    const quest_x = 664;
    const quest_y = 16;
    const quest_w = 340;
    const quest_h = 39;
    const spell_x = 536;
    const spell_y = 520;
    const skill_x = 218;
    const skill_y = 16;

    /// `can_delete`: CanDelete, the dismiss button (only from the map).
    pub fn create(c: *Ctx, s: *Shard, k: usize, exp_pct: i32, can_delete: bool) !scene.Scene {
        const self = try c.gpa.create(HeroScreen);
        errdefer c.gpa.destroy(self);
        const h = &s.heroes.items[k];
        self.* = .{
            .s = s,
            .k = k,
            .sh = undefined,
            .hs = undefined,
            .items = undefined,
            .info = hero_ui.InfoBlock.init(c, h, 0, 0, movement.castleParam(s, c.proto, h.player, movement.abil_siege, true), movement.castleParam(s, c.proto, h.player, movement.abil_mobility, true), exp_pct),
            .trash = undefined,
            .exit = undefined,
            .unit_shape = undefined,
            .spell_shape = undefined,
            .book = undefined,
            .dismiss = undefined,
            .old_cursor = c.cursor,
        };
        self.sh = try Sheets.load(c);
        errdefer self.sh.deinit();
        self.hs = try hero_ui.HeroSheets.load(c);
        errdefer self.hs.deinit();
        self.items = try hero_ui.ItemBlock.init(&self.sh, 356, 13);
        errdefer self.items.deinit();
        self.unit_shape = self.sh.iface.imageIndex(gui.unit_mask) orelse return error.MissingMask;
        errdefer rl.unloadImage(self.unit_shape);
        self.spell_shape = self.sh.iface.imageIndex(gui.spell_mask) orelse return error.MissingMask;
        errdefer rl.unloadImage(self.spell_shape);
        self.book = try hero_ui.SpellBlock.init(&self.sh, spell_x, spell_y);
        errdefer self.book.deinit();
        self.trash = try Button.init(&self.sh.buttons, 319, 510, butt_trash, butt_trash + 1, butt_trash + 2);
        errdefer self.trash.deinit();
        self.trash.inactive = butt_trash + 3;
        self.trash.on = false;
        self.trash.hotkey = .delete;
        self.trash.sound = sound.dismiss;
        self.exit = try Button.init(&self.sh.buttons, 920, 623, butt_exit, butt_exit + 1, butt_exit + 2);
        self.exit.hotkey = .escape;
        self.exit.hint = 135;
        errdefer self.exit.deinit();
        self.dismiss = try Button.init(&self.sh.buttons, 246, 444, butt_dismiss, butt_dismiss + 1, butt_dismiss + 2);
        self.dismiss.inactive = butt_dismiss + 3;
        self.dismiss.hint = ph_dismiss;
        self.dismiss.enabled = can_delete;
        self.units.of_hero = true;
        for (&self.quests, 0..) |*m, i| m.* = Mask.rect(quest_x + 1, quest_y + 41 + quest_h * @as(f32, @floatFromInt(i)), 338, 36);
        self.showQuests(c);
        for (&self.skills, 0..) |*m, i| m.* = .{ .x = skill_x - 6 + skillPos(i)[0], .y = skill_y - 6 + skillPos(i)[1] };
        c.setCursor(0);
        return scene.Scene.of(HeroScreen, self, false);
    }

    pub fn deinit(self: *HeroScreen, c: *Ctx) void {
        if (!self.gone) {
            if (self.held) |held| if (held.slot > 0) {
                self.hero().inventory[held.slot] = .{ held.item, held.dur };
            };
            if (self.spell) |sp| spellbook.putBack(c.proto, self.hero(), sp);
            if (self.unit) |u| if (u.slot > 0) army.place(self.hero(), c.proto, u.unit, u.slot);
        }
        c.setCursor(self.old_cursor);
        self.dismiss.deinit();
        self.book.deinit();
        self.exit.deinit();
        self.trash.deinit();
        rl.unloadImage(self.spell_shape);
        rl.unloadImage(self.unit_shape);
        self.items.deinit();
        self.hs.deinit();
        self.sh.deinit();
    }

    fn hero(self: *const HeroScreen) *Hero {
        return &self.s.heroes.items[self.k];
    }

    fn skillPos(i: usize) [2]f32 {
        return .{ @as(f32, @floatFromInt(i % 2)) * 60, @as(f32, @floatFromInt(i / 2)) * 60 };
    }

    fn karma(self: *const HeroScreen) i32 {
        const p = self.hero().player;
        return if (p > 0) self.s.rulers[@intCast(p)].karma else 0;
    }

    fn updateTrash(self: *HeroScreen) void {
        self.trash.on = self.held != null or self.spell != null or self.unit != null;
    }

    /// What is in the hand goes back before something else is taken.
    fn putBackHeld(self: *HeroScreen, c: *Ctx, keep: enum { item, spell, unit }) void {
        const h = self.hero();
        if (keep != .item) if (self.held) |held| {
            if (held.slot > 0) h.inventory[held.slot] = .{ held.item, held.dur };
            self.held = null;
        };
        if (keep != .spell) if (self.spell) |sp| {
            spellbook.putBack(c.proto, h, sp);
            self.spell = null;
        };
        if (keep != .unit) if (self.unit) |u| {
            army.place(h, c.proto, u.unit, u.slot);
            self.unit = null;
        };
    }

    fn strHint(self: *HeroScreen, s: []const u8, at: rl.Vector2) void {
        self.hint.what = .{ .string = s };
        self.hint.pos = at;
    }

    pub fn update(self: *HeroScreen, c: *Ctx, f: *const input.Frame, _: f32) scene.Action {
        const pr = c.proto;
        const h = self.hero();
        const p = f.pointer;
        self.pointer = p;
        self.hint.clear();
        self.card = null;
        if (self.asking) {
            self.asking = false;
            if (self.answer == 1) {
                if (self.unit != null) self.unit = null else if (self.held != null) self.held = null else self.spell = null;
            }
            self.answer = null;
            self.updateTrash();
        }
        if (self.dismissing) {
            self.dismissing = false;
            const yes = self.answer == 1;
            self.answer = null;
            if (yes) {
                // delete Hero: what is in the hand goes with him.
                heroes.bury(self.s, pr, self.k);
                self.gone = true;
                return .pop;
            }
        }
        if (self.quest_ask) |qa| {
            const yes = self.answer == 1;
            self.answer = null;
            self.quest_ask = null;
            if (yes and !qa.confirm and dlg_abandon < pr.dialogs.len) {
                self.quest_ask = .{ .i = qa.i, .confirm = true };
                return .{ .push = Dialog.create(c, dlg_abandon, .{ true, true, false, false }, &self.answer) catch {
                    self.quest_ask = null;
                    return .none;
                } };
            }
            if (yes and qa.confirm) {
                quest.abandon(pr, self.s.provinces, h, qa.i);
                self.showQuests(c);
            }
        }
        if (self.exit.check(f)) return .pop;
        if (self.dismiss.check(f)) {
            self.dismissing = true;
            return .{ .push = Dialog.create(c, dlg_bury, .{ true, true, false, false }, &self.answer) catch {
                self.dismissing = false;
                return .none;
            } };
        }

        const inv = self.items.check(p);
        if (inv.under) |u| if (u > 0 and h.inventory[u][0] > 0) {
            self.card = .{ .item = h.inventory[u][0], .dur = h.inventory[u][1] };
        };
        if (inv.click) |k_in| {
            if (self.held) |held| {
                const k = if (k_in == 0) equip.dollTarget(pr, h, held.item) else k_in;
                self.drop(c, held, k);
            } else if (k_in > 0 and h.inventory[k_in][0] > 0) {
                // "Взятие предмета": a spell or unit in the hand goes back first.
                self.putBackHeld(c, .item);
                self.held = .{ .item = h.inventory[k_in][0], .dur = h.inventory[k_in][1], .slot = k_in, .owner = .hero };
                h.inventory[k_in] = .{ 0, 0 };
                self.trash.on = true;
            }
        }

        // "Манипулирование юнитами".
        if (self.units.check(c, h, &self.unit_shape, p, &self.hint)) |slot| self.clickUnit(c, slot);
        var unit_info: ?usize = null;
        if (self.units.info) |info| {
            self.units.info = null;
            if (info == .unit) unit_info = info.unit;
        }
        // "Манипулирование заклинаниями".
        const bk = self.book.check(c, h, p, &self.hint);
        if (bk.click) |i| self.clickBook(c, i);
        const n_skills: usize = @intCast(@min(h.skillSlots(), 13));
        for (self.skills[0 .. n_skills + 1], 0..) |*m, i| {
            if (i > 0 and h.skills[i - 1][0] <= 0) continue;
            m.shape = &self.spell_shape;
            const res = m.check(p);
            if (res != .none) return self.skillInfo(c, i);
            if (!m.hot()) continue;
            if (i == 0) {
                self.strHint(pr.classes[h.className()].name, p.pos);
            } else {
                const s = h.skills[i - 1];
                if (s[0] < pr.skills.len) self.strHint(std.fmt.bufPrint(&self.hint_buf, "{s} {d}", .{ pr.skills[@intCast(s[0])].name, s[1] }) catch "", p.pos);
            }
        }

        if (self.trash.check(f)) if (self.unit != null) {
            if (c.human.message(msg_delete) and dlg_delete_unit < pr.dialogs.len) {
                self.asking = true;
                return .{ .push = Dialog.create(c, dlg_delete_unit, .{ true, true, false, false }, &self.answer) catch {
                    self.asking = false;
                    return .none;
                } };
            }
            // dcUnit::Dismiss.
            self.unit = null;
            self.trash.on = false;
        } else if (self.spell) |sp| {
            const lib = &self.s.rulers[@intCast(h.player)].holdings.library;
            if (c.human.message(msg_delete) and !spellbook.inLibrary(lib, sp.spell) and dlg_delete_spell < pr.dialogs.len) {
                self.asking = true;
                return .{ .push = Dialog.create(c, dlg_delete_spell, .{ true, true, false, false }, &self.answer) catch {
                    self.asking = false;
                    return .none;
                } };
            }
            self.spell = null;
            self.trash.on = false;
        } else if (self.held != null) {
            if (c.human.message(msg_delete) and dlg_delete_item < pr.dialogs.len) {
                self.asking = true;
                return .{ .push = Dialog.create(c, dlg_delete_item, .{ true, true, false, false }, &self.answer) catch {
                    self.asking = false;
                    return .none;
                } };
            }
            self.held = null;
            self.trash.on = false;
        };
        for (&self.quests, 0..) |*m, i| {
            if (h.quest(i).index <= 0) {
                m.state = .visible;
                continue;
            }
            if (m.check(p) == .click) return self.questEntry(c, i);
        }
        self.info.refresh(c, h);
        if (self.info.portraitClick(&self.hs, h, p)) return .{ .push = army_info.UnitWindow.createHero(c, h, self.info.exp_pct) catch return .none };
        const abil_click = self.info.update(c, h, p, &self.hint) orelse self.stat.update(c, h, p, &self.hint);
        if (self.exit.hot()) self.hint.phrase(c, self.exit.hint, p.pos);
        if (self.dismiss.hot()) self.hint.phrase(c, self.dismiss.hint, p.pos);
        if (inv.info) |k| if (k > 0 and h.inventory[k][0] > 0) {
            return .{ .push = ItemInfo.create(c, h.inventory[k][0], h.inventory[k][1]) catch return .none };
        };
        if (bk.info) |sp| return .{ .push = SpellInfo.create(c, sp) catch return .none };
        if (unit_info) |i| if (h.squad[i]) |u| {
            return .{ .push = army_info.UnitWindow.createUnit(c, u, h.squadMoraleOf(pr, i, self.karma())) catch return .none };
        };
        if (abil_click) |a| return info_windows.abilityAction(c, a.num, a.power);
        return .none;
    }

    /// GUI_SkillBlock::Check (Screens.cpp:909): slot 0 is the class
    /// (WindowInfo with ClassDesc; a warrior's text names the ability of
    /// his unit's first upgrade, arUpg[0] - 30), the others the skills.
    fn skillInfo(self: *HeroScreen, c: *Ctx, i: usize) scene.Action {
        const pr = c.proto;
        const h = self.hero();
        if (i > 0) {
            const s = h.skills[i - 1];
            return .{ .push = info_windows.skill(c, s[0], s[1]) catch return .none };
        }
        const cl = &pr.classes[h.className()];
        var buf: [1024]u8 = undefined;
        var body: []const u8 = cl.desc;
        if (h.class1 == 2) if (h.squad[0]) |u| {
            const j = u.upgrades[0] - 30;
            const name = if (j >= 0 and j < pr.abilities.len) pr.abilities[@intCast(j)].name else "";
            body = formatStr(&buf, cl.desc, name);
        };
        return .{ .push = WindowInfo.createFrom(c, cl.name, body, "hero_class", h.className()) catch return .none };
    }

    /// A click on book slot `i`: the spell in the hand goes there, or the
    /// spell of the slot is taken (an item in the hand goes back first).
    fn clickBook(self: *HeroScreen, c: *Ctx, i: usize) void {
        const h = self.hero();
        if (self.spell) |sp| {
            if (spellbook.drop(c.proto, h, sp, i, false)) |r| self.spell = r.held;
        } else if (h.spell(i) > 0) {
            self.putBackHeld(c, .spell);
            self.spell = .{ .spell = h.spell(i), .slot = @intCast(i) };
            h.setSpell(i, 0);
        }
        self.updateTrash();
    }

    /// GUI_UnitBlock's click on open slot `i`: the unit in the hand goes
    /// there, or the slot's unit is taken (an item or spell in the hand
    /// goes back first).
    fn clickUnit(self: *HeroScreen, c: *Ctx, i: usize) void {
        const pr = c.proto;
        const h = self.hero();
        if (self.unit) |sel| {
            if (army.slotLevel(i) < pr.units[@intCast(sel.unit.type)].level) return;
            if (h.squad[i]) |other| {
                const lv = pr.units[@intCast(other.type)].level;
                const free = army.freeSlot(h, pr, lv);
                if (free == 0) {
                    // No room for the unit there: it goes to the hand.
                    h.squad[i] = null;
                    army.placeUnit(h, sel.unit, i);
                    self.unit = .{ .unit = other, .slot = 0 };
                    return;
                }
                army.placeUnit(h, other, if (sel.slot == 0 or army.slotLevel(sel.slot) < lv) free else sel.slot);
            }
            h.squad[i] = null;
            army.placeUnit(h, sel.unit, i);
            self.unit = null;
        } else if (h.squad[i]) |u| {
            self.putBackHeld(c, .unit);
            self.unit = .{ .unit = u, .slot = i };
            h.squad[i] = null;
        }
        self.updateTrash();
    }

    /// GUI_QuestBlock::Show: CheckQuestDone and the two lines of each quest.
    fn showQuests(self: *HeroScreen, c: *Ctx) void {
        const pr = c.proto;
        const h = self.hero();
        for (0..hero_mod.quest_limit) |i| {
            self.quest_text[i] = .{ "", "" };
            if (h.quest(i).index <= 0) continue;
            quest.checkDone(pr, self.s.provinces, h, i);
            const q = h.quest(i);
            if (q.index >= pr.quests.len) continue;
            const qp = &pr.quests[@intCast(q.index)];
            const st = quest.site(self.s.provinces, q);
            const b = &self.quest_buf[i];
            const qty = qp.tasks[0].quantity;
            if (qp.kind == quest.kind_kill_site) {
                const unit_name = if (st) |x| unitName(pr, x.level) else "";
                const need = if (st) |x| x.data else 0;
                self.quest_text[i][0] = std.fmt.bufPrint(&b[0], "{s}: {s}", .{ qp.name, unit_name }) catch "";
                self.quest_text[i][1] = std.fmt.bufPrint(&b[1], "{s}: {d}/{d}", .{ pr.phrase(ph_done), @min(q.progress[0], need), need }) catch "";
                continue;
            }
            const item_name = if (st) |x| itemName(pr, x.data) else "";
            self.quest_text[i][0] = if (qp.kind == quest.kind_bring_item)
                event_mod.format(&b[0], qp.name, &.{.{ .s = item_name }})
            else
                event_mod.format(&b[0], qp.name, &.{});
            self.quest_text[i][1] = if (qty > 1)
                std.fmt.bufPrint(&b[1], "{s}: {d}/{d}", .{ pr.phrase(ph_done), q.progress[0], qty }) catch ""
            else
                pr.phrase(if (q.complete) ph_done else ph_not_done);
        }
    }

    fn unitName(pr: anytype, t: i32) []const u8 {
        return if (t > 0 and t < pr.units.len) pr.units[@intCast(t)].name else "";
    }

    fn itemName(pr: anytype, it: i32) []const u8 {
        return if (it > 0 and it < pr.items.len) pr.items[@intCast(it)].name else "";
    }

    /// GUI_QuestBlock::Check: dialog 494 with the journal entry.
    fn questEntry(self: *HeroScreen, c: *Ctx, i: usize) scene.Action {
        const pr = c.proto;
        const q = self.hero().quest(i);
        if (q.index <= 0 or q.index >= pr.quests.len or dlg_quest >= pr.dialogs.len) return .none;
        const qp = &pr.quests[@intCast(q.index)];
        const st = quest.site(self.s.provinces, q);
        const prov = if (q.province > 0 and q.province < self.s.provinces.len) self.s.provinces[@intCast(q.province)].name else "";
        var buf: [1024]u8 = undefined;
        const body = switch (qp.kind) {
            quest.kind_bring_item => event_mod.format(&buf, qp.text, &.{ .{ .s = prov }, .{ .s = if (st) |x| itemName(pr, x.data) else "" } }),
            quest.kind_kill_site => event_mod.format(&buf, qp.text, &.{ .{ .s = if (st) |x| unitName(pr, x.level) else "" }, .{ .d = if (st) |x| x.data else 0 } }),
            else => event_mod.format(&buf, qp.text, &.{.{ .s = prov }}),
        };
        self.quest_ask = .{ .i = i, .confirm = false };
        return .{ .push = Dialog.createText(c, dlg_quest, .{ true, true, false, false }, &self.answer, body) catch {
            self.quest_ask = null;
            return .none;
        } };
    }

    /// The held item put on place `k`: on a taken place the two change
    /// hands when the old one may go back (ScreenHero, "Занятый слот").
    fn drop(self: *HeroScreen, c: *Ctx, held: equip.Held, k: usize) void {
        const h = self.hero();
        const tr = &self.s.rulers[@intCast(h.player)].holdings.treasury;
        if (equip.dropOnHero(c.proto, h, tr, held, k, null)) |r| {
            self.held = r.held;
            self.trash.on = self.held != null;
        }
    }

    pub fn draw(self: *HeroScreen, c: *Ctx) void {
        const pr = c.proto;
        const h = self.hero();
        rl.clearBackground(.black);
        gfx.drawTex(self.sh.iface.texIndex(gui_hero_screen), 0, 0);
        self.info.draw(c, &self.sh, &self.hs, h);
        self.stat.draw(c, h);
        self.items.draw(c, &self.sh, h, if (self.held) |held| held.item else 0);
        self.units.draw(c, &self.sh, h, self.karma(), if (self.unit) |u| u.unit else null);
        self.drawQuests(c);
        self.book.draw(c, &self.sh, h, if (self.spell) |sp| sp.spell else 0);
        gfx.drawTex(self.hs.classes.texIndex(h.className()), skill_x, skill_y);
        const n_skills: usize = @intCast(@min(h.skillSlots(), 13));
        for (1..14) |i| {
            const at = skillPos(i);
            if (i > n_skills) {
                gfx.drawTex(self.sh.iface.texIndex(gui_spell_closed), skill_x + at[0], skill_y + at[1]);
                continue;
            }
            const s = h.skills[i - 1];
            if (s[0] > 0) gfx.drawTex(self.hs.skills.texIndex(@intCast((s[0] - 1) * 5 + s[1])), skill_x + at[0], skill_y + at[1]);
        }
        for (self.skills) |m| if (m.hot()) gfx.drawTex(self.sh.iface.texIndex(gui.spell_select), m.x, m.y);
        self.trash.draw(&self.sh.buttons, c.tick);
        self.exit.draw(&self.sh.buttons, c.tick);
        self.dismiss.draw(&self.sh.buttons, c.tick);
        if (self.held) |held| castle_ui.drawHeld(&self.sh, self.sh.item(pr, held.item), self.pointer);
        if (self.spell) |sp| castle_ui.drawHeld(&self.sh, self.sh.spell(sp.spell), self.pointer);
        if (self.unit) |u| castle_ui.drawHeld(&self.sh, self.sh.unitPic(u.unit.type), self.pointer);
        if (self.card) |cd| {
            castle_ui.itemCard(c, &self.sh, self.pointer.pos, cd.item, cd.dur, h, 0, .{ 0, 0 }, .{ 0, 0 });
        } else self.hint.draw(c, &self.sh.iface);
    }

    fn drawQuests(self: *const HeroScreen, c: *Ctx) void {
        const h = self.hero();
        var b: [64]u8 = undefined;
        const cx = quest_x + quest_w / 2;
        c.font1.drawCentered(std.fmt.bufPrint(&b, "{s}:", .{c.proto.phrase(ph_quests)}) catch "", cx, quest_y + 12, castle_ui.color1);
        for (0..hero_mod.quest_limit) |i| {
            const q = h.quest(i);
            if (q.index <= 0) continue;
            const m = self.quests[i];
            if (m.hot()) rl.drawRectangle(@intFromFloat(m.x), @intFromFloat(m.y), 338, 36, .{ .r = 200, .g = 200, .b = 200, .a = 128 });
            const col = if (q.complete) quest_done_color else castle_ui.color1;
            const y = quest_y + 45 + quest_h * @as(f32, @floatFromInt(i));
            c.font1.drawCentered(self.quest_text[i][0], cx, y, col);
            c.font1.drawCentered(self.quest_text[i][1], cx, y + 18, col);
        }
    }
};
