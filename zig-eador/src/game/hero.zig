//! dcHero (Unit.h:197, Hero.cpp): a ruler's hero on the strategic map.
//! The saved record (0x3AC bytes) is kept whole so that the fields not
//! ported yet go back to the file unchanged; the ones the map uses are
//! decoded.
//!
//! Record layout (dcOldHero, then dcHero):
//!   0x00 ipName, 0x04 ipPortrait, 0x08 ipPlayer, 0x0C iExp, 0x10 iLevel,
//!   0x14 NiB, 0x18 ipClass1, 0x1C ipClass2, 0x20 iHealth, 0x24 iLeader,
//!   0x28 iMagic, 0x2C iLifeBonus, 0x30 iMoveBonus, 0x34 ipProvince,
//!   0x38 iTask, 0x3C iTarget, 0x40 iAITask, 0x44 iCamper, 0x48 iCampTurn,
//!   0x4C iDead, 0x50..0xA8 pointers (pictures, objects, name, province,
//!   pSquad[16]), 0xA8 arQuest[10] (0x24 each), 0x210 arSpell[22],
//!   0x268 apSkill[11][2], 0x2C0 arInventory[26][2], 0x390 arGarrison[4],
//!   0x3A0 wavDeath, 0x3A4 ipMoveProv, 0x3A8 ipEvent.
const std = @import("std");
const proto_mod = @import("../data/proto.zig");
const Proto = proto_mod.Proto;
const unit_mod = @import("unit.zig");
const Unit = unit_mod.Unit;
const savefile = @import("../formats/savefile.zig");
const Rand = @import("../data/rand.zig").Rand;

pub const record_size = 0x3AC;
pub const max_squad = 16;
/// HERO_SKILL_LIMIT, INVENTORY_SIZE (+1 slot), MAX_HERO.
pub const skill_limit = 11;
pub const inventory_size = 25;
pub const max_hero = 8;
/// Inventory slots 1..15 are worn (CheckHeroBonus).
pub const worn_slots = 15;

/// iTask.
pub const Task = struct {
    pub const none = 0;
    pub const go = 1;
    pub const explore = 2;
    pub const pillage = 3;
    pub const enter = 4;
    pub const attack = 5;
};

/// Hero bonus ids used by the map (ability_num.var).
pub const bonus = struct {
    pub const life = 1;
    pub const speed = 7;
    /// Weapon damage: the second hand counts only with the same weapon type.
    pub const attack = 8;
    pub const ranged = 10;
    pub const mobility = 998;
    pub const initiative = 999;
};

/// apSkill index of Pathfinding (WalkType = 1 + its level).
pub const skill_pathfinding = 26;

pub const Hero = struct {
    /// The player whose list holds the hero and the slot in it.
    owner: i32 = 0,
    slot: u8 = 0,
    rec: [record_size]u8,
    /// pSquad: slot 0 is the hero's own unit.
    squad: [max_squad]?Unit = @splat(null),

    name: i32,
    portrait: i32,
    player: i32,
    exp: i32,
    level: i32,
    class1: i32,
    class2: i32,
    health: i32,
    leader: i32,
    magic: i32,
    life_bonus: i32,
    move_bonus: i32,
    province: i32,
    task: i32,
    target: i32,
    dead: i32,
    event: i32,
    move_prov: i32,
    /// (skill index, level).
    skills: [skill_limit][2]i32,
    /// (item index, durability); slot 0 is unused.
    inventory: [inventory_size + 1][2]i32,

    /// LoadGame: the record and the squad.  Unit types outside unit.var
    /// are dropped as in the original.
    pub fn fromArmy(pr: *const Proto, a: *const savefile.Army) Hero {
        const r = a.rec;
        var h: Hero = .{
            .rec = r[0..record_size].*,
            .name = int(r, 0x00),
            .portrait = int(r, 0x04),
            .player = int(r, 0x08),
            .exp = int(r, 0x0c),
            .level = int(r, 0x10),
            .class1 = int(r, 0x18),
            .class2 = int(r, 0x1c),
            .health = int(r, 0x20),
            .leader = int(r, 0x24),
            .magic = int(r, 0x28),
            .life_bonus = int(r, 0x2c),
            .move_bonus = int(r, 0x30),
            .province = int(r, 0x34),
            .task = int(r, 0x38),
            .target = int(r, 0x3c),
            .dead = int(r, 0x4c),
            .move_prov = int(r, 0x3a4),
            .event = int(r, 0x3a8),
            .skills = undefined,
            .inventory = undefined,
        };
        if (h.portrait < 0) h.portrait = 1;
        for (&h.skills, 0..) |*s, i| s.* = .{ int(r, 0x268 + 8 * i), int(r, 0x26c + 8 * i) };
        for (&h.inventory, 0..) |*s, i| {
            s.* = .{ int(r, 0x2c0 + 8 * i), int(r, 0x2c4 + 8 * i) };
            if (i > 0 and (s[0] < 0 or s[0] >= pr.items.len)) s.* = .{ 0, 0 };
        }
        for (a.list(), a.slots[0..a.n]) |u, slot| {
            const t = int(u, 0);
            if (t <= 0 or t >= pr.units.len) continue;
            h.squad[slot] = Unit.fromBytes(u);
        }
        return h;
    }

    /// arSpell[i] (0x210): the spell remembered in book slot i (1..21).
    pub fn spell(self: *const Hero, i: usize) i32 {
        return std.mem.readInt(i32, self.rec[0x210 + 4 * i ..][0..4], .little);
    }

    pub fn setSpell(self: *Hero, i: usize, v: i32) void {
        setI(&self.rec, 0x210 + 4 * i, v);
    }

    /// arQuest[i] (0xA8 + 0x24 * i).
    pub fn quest(self: *const Hero, i: usize) Quest {
        const o = 0xa8 + quest_size * i;
        var q: Quest = .{
            .index = int(&self.rec, o),
            .province = int(&self.rec, o + 4),
            .site = int(&self.rec, o + 8),
            .time = int(&self.rec, o + 0xc),
            .complete = self.rec[o + 0x10] != 0,
        };
        for (&q.progress, 0..) |*p, k| p.* = int(&self.rec, o + 0x14 + 4 * k);
        return q;
    }

    /// Writes arQuest[i] back; the padding after bComplete stays.
    pub fn setQuest(self: *Hero, i: usize, q: Quest) void {
        const o = 0xa8 + quest_size * i;
        setI(&self.rec, o, q.index);
        setI(&self.rec, o + 4, q.province);
        setI(&self.rec, o + 8, q.site);
        setI(&self.rec, o + 0xc, q.time);
        self.rec[o + 0x10] = @intFromBool(q.complete);
        for (q.progress, 0..) |p, k| setI(&self.rec, o + 0x14 + 4 * k, p);
    }

    pub fn quests(self: *const Hero) [quest_limit]Quest {
        var r: [quest_limit]Quest = undefined;
        for (&r, 0..) |*q, i| q.* = self.quest(i);
        return r;
    }

    /// CanUse (Hero.cpp:773): the class allows the item's type.
    pub fn canUse(self: *const Hero, pr: *const Proto, item: i32) bool {
        if (self.class1 == 0) return true;
        const c1 = self.class1;
        const c2 = self.class2;
        return switch (pr.items[@intCast(item)].class) {
            0 => true,
            1, 6 => c1 < 4 or (c2 > 0 and c2 < 4),
            2, 8 => c1 == 1 or c2 == 1,
            3 => c1 == 2 or c2 == 2,
            4 => c1 == 4 or c2 == 4,
            5 => c1 == 3 or c2 == 3,
            7, 9 => c1 == 1 or c2 == 1 or c1 == 3 or c2 == 3,
            else => false,
        };
    }

    /// The capital's garrison (Mapper.cpp:1620): a dcHero of `owner` in
    /// province `p` with arGarrison[0] = 6 and no units.
    pub fn garrison(pr: *const Proto, owner: i32, p: i32) Hero {
        var rec: [record_size]u8 = @splat(0);
        setI(&rec, 0x08, owner);
        setI(&rec, 0x34, p);
        setI(&rec, 0x4c, -1);
        setI(&rec, 0x390, 6);
        for (1..4) |k| setI(&rec, 0x390 + 4 * k, -1);
        var h = fromArmy(pr, &.{ .rec = &rec });
        h.owner = owner;
        return h;
    }

    /// The record with the decoded fields written back.
    pub fn toRecord(self: *const Hero) [record_size]u8 {
        var b = self.rec;
        const fields = [_]struct { off: usize, v: i32 }{
            .{ .off = 0x00, .v = self.name },     .{ .off = 0x04, .v = self.portrait },
            .{ .off = 0x08, .v = self.player },   .{ .off = 0x0c, .v = self.exp },
            .{ .off = 0x10, .v = self.level },    .{ .off = 0x18, .v = self.class1 },
            .{ .off = 0x1c, .v = self.class2 },   .{ .off = 0x20, .v = self.health },
            .{ .off = 0x24, .v = self.leader },   .{ .off = 0x28, .v = self.magic },
            .{ .off = 0x2c, .v = self.life_bonus }, .{ .off = 0x30, .v = self.move_bonus },
            .{ .off = 0x34, .v = self.province }, .{ .off = 0x38, .v = self.task },
            .{ .off = 0x3c, .v = self.target },   .{ .off = 0x4c, .v = self.dead },
            .{ .off = 0x3a4, .v = self.move_prov }, .{ .off = 0x3a8, .v = self.event },
        };
        for (fields) |f| setI(&b, f.off, f.v);
        for (self.skills, 0..) |s, i| {
            setI(&b, 0x268 + 8 * i, s[0]);
            setI(&b, 0x26c + 8 * i, s[1]);
        }
        for (self.inventory, 0..) |s, i| {
            setI(&b, 0x2c0 + 8 * i, s[0]);
            setI(&b, 0x2c4 + 8 * i, s[1]);
        }
        return b;
    }

    /// The army block of SaveGame: 1, the record, 16 × (present, unit).
    pub fn writeBlock(self: *const Hero, out: *std.ArrayList(u8), gpa: std.mem.Allocator) !void {
        try out.append(gpa, 1);
        try out.appendSlice(gpa, &self.toRecord());
        for (self.squad) |slot| {
            if (slot) |u| {
                try out.append(gpa, 1);
                try out.appendSlice(gpa, &u.toBytes());
            } else try out.append(gpa, 0);
        }
    }

    pub fn alive(self: *const Hero) bool {
        return self.dead < 0;
    }

    /// pstName: names.var list of the base class.
    pub fn nameText(self: *const Hero, pr: *const Proto) []const u8 {
        if (self.class1 < 1 or self.class1 > proto_mod.name_lists) return "";
        const list = pr.hero_names[@intCast(self.class1 - 1)];
        if (self.name < 0 or self.name >= list.len) return "";
        return list[@intCast(self.name)];
    }

    /// GetClassName: base class below level 10, subclass below 20.
    pub fn className(self: *const Hero) usize {
        const c1 = self.class1 - 1;
        const i = if (self.level < 10) c1 else if (self.level < 20) 3 + c1 * 4 + self.class2 else 19 + c1 * 4 + self.class2;
        return @intCast(std.math.clamp(i, 0, proto_mod.max_class - 1));
    }

    /// CheckSkill.
    pub fn checkSkill(self: *const Hero, index: i32) i32 {
        for (self.skills) |s| if (s[0] == index) return s[1];
        return 0;
    }

    /// CheckHeroBonus (Hero.cpp:426): skills, talents, worn items and set
    /// bonuses.  The "full set" flag (arPlayer.bSet) is not set here.
    pub fn heroBonus(self: *const Hero, pr: *const Proto, index: i32) i32 {
        if (index == bonus.initiative and self.class1 == 0) return self.task;
        if (self.class1 == 0) return 0;
        var result: i32 = 0;
        const hp: usize = @intCast(std.math.clamp(self.health, 0, proto_mod.stat_levels));
        switch (index) {
            1 => {
                for (pr.stats[0][0..hp]) |l| result += l[0];
                result += self.life_bonus;
            },
            11 => for (pr.stats[0][0..hp]) |l| {
                result += l[1];
            },
            12 => for (pr.stats[0][0..hp]) |l| {
                result += l[2];
            },
            else => {},
        }
        for (self.skills) |s| {
            if (s[0] <= 0 or s[0] >= pr.skills.len) continue;
            const sk = &pr.skills[@intCast(s[0])];
            const lv: usize = @intCast(std.math.clamp(s[1], 0, proto_mod.skill_levels));
            for (sk.hero[0..lv]) |upg| for (upg) |u| {
                if (u.id == index) result += u.value;
            };
        }
        if (self.class2 > 0) {
            result += talent(pr, self.class1 * 4 - 1 + self.class2, index);
            if (self.level >= 20) result += talent(pr, 15 + self.class1 * 4 + self.class2, index);
        }
        var in_set: [64]i32 = @splat(0);
        for (1..worn_slots + 1) |i| {
            const it = self.inventory[i];
            if (it[0] <= 0 or it[1] <= 0) continue;
            for (pr.item_sets, 0..) |*set, j| {
                if (j == 0 or j >= in_set.len) continue;
                for (set.items[0..set.n_items]) |x| {
                    if (x == it[0]) in_set[j] += 1;
                }
            }
            const item = &pr.items[@intCast(it[0])];
            for (item.effects[0..item.n_effects]) |e| {
                if (e.id != index or e.p2 != 0) continue;
                if ((index == bonus.attack or index == bonus.ranged) and i == 2) {
                    if (pr.items[@intCast(self.inventory[1][0])].class == item.class) result += e.p1;
                } else result += e.p1;
            }
        }
        for (pr.item_sets, 0..) |*set, j| {
            if (j == 0 or j >= in_set.len) continue;
            for (set.bonuses[0..set.n_bonuses]) |*b| {
                if (b.items_needed <= 0 or in_set[j] < b.items_needed) continue;
                for (b.abilities[0..b.n_abilities]) |a| {
                    if (a.id == index and a.p2 == 0) result += a.p1;
                }
            }
        }
        if (index == bonus.attack) {
            const weapon = self.inventory[1];
            const wi = &pr.items[@intCast(weapon[0])];
            if (wi.class == 3) result += self.heroBonus(pr, 993);
            if (wi.class == 4) result += self.heroBonus(pr, 994);
            var mult: i32 = 0;
            for (wi.effects[0..wi.n_effects]) |e| {
                if (e.id == index and e.p2 == 0 and weapon[1] > 0) mult = 1;
            }
            result *= mult;
        }
        return result;
    }

    /// CheckUnitBonus (Hero.cpp:338): what the hero gives every unit of
    /// the squad: squad parts of the skills, talents above 1000, worn items
    /// and set bonuses of area 1.
    pub fn unitBonus(self: *const Hero, pr: *const Proto, index: i32) i32 {
        if (self.class1 == 0) return 0;
        var result: i32 = 0;
        for (self.skills) |s| {
            if (s[0] <= 0 or s[0] >= pr.skills.len) continue;
            const lv: usize = @intCast(std.math.clamp(s[1], 0, proto_mod.skill_levels));
            for (pr.skills[@intCast(s[0])].squad[0..lv]) |upg| for (upg) |u| {
                if (u.id == index) result += u.value;
            };
        }
        if (self.class2 > 0) {
            result += unitTalent(pr, self.class1 * 4 - 1 + self.class2, index);
            if (self.level >= 20) result += unitTalent(pr, 15 + self.class1 * 4 + self.class2, index);
        }
        var in_set: [64]i32 = @splat(0);
        for (1..worn_slots + 1) |i| {
            const it = self.inventory[i];
            if (it[0] <= 0 or it[1] <= 0 or it[0] >= pr.items.len) continue;
            for (pr.item_sets, 0..) |*set, j| {
                if (j == 0 or j >= in_set.len) continue;
                for (set.items[0..set.n_items]) |x| {
                    if (x == it[0]) in_set[j] += 1;
                }
            }
            const item = &pr.items[@intCast(it[0])];
            for (item.effects[0..item.n_effects]) |e| {
                if (e.id == index and e.p2 == 1) result += e.p1;
            }
        }
        for (pr.item_sets, 0..) |*set, j| {
            if (j == 0 or j >= in_set.len) continue;
            for (set.bonuses[0..set.n_bonuses]) |*b| {
                if (b.items_needed <= 0 or in_set[j] < b.items_needed) continue;
                for (b.abilities[0..b.n_abilities]) |a| {
                    if (a.id == index and a.p2 == 1) result += a.p1;
                }
            }
        }
        return result;
    }

    /// GetSquadMorale of this hero's squad (see `squadMorale`).
    pub fn squadMoraleOf(self: *const Hero, pr: *const Proto, num: usize, player_karma: ?i32) i32 {
        return squadMorale(pr, &self.squad, num, player_karma, self);
    }

    /// dcUnit::GetSpeed of squad slot `k`; the hero's own unit adds the
    /// hero bonus (its pHero is set).
    pub fn unitSpeed(self: *const Hero, pr: *const Proto, k: usize) i32 {
        const u = &(self.squad[k] orelse return 0);
        var s = pr.units[@intCast(u.type)].speed + u.checkUpg(pr, bonus.speed) + u.checkAbility(pr, bonus.speed);
        if (k == 0) s += self.heroBonus(pr, bonus.speed);
        return s;
    }

    /// GetMobility (Hero.cpp:1433): the slowest unit, at least 1, plus the
    /// hero bonus, iMoveBonus and the castle (`castle_bonus`,
    /// arPlayer.GetBldParam1(37)).
    pub fn mobility(self: *const Hero, pr: *const Proto, castle_bonus: i32) i32 {
        var result = self.unitSpeed(pr, 0);
        for (1..max_squad) |k| {
            if (self.squad[k] != null) result = @min(result, self.unitSpeed(pr, k));
        }
        result = @max(result, 1);
        result += self.heroBonus(pr, bonus.mobility);
        result += self.move_bonus;
        if (self.player > 0) result += castle_bonus;
        return result;
    }

    /// dcUnit::GetLife .. GetMorale of the hero's own unit (stats 1..12):
    /// the unit type's value, CheckUpg with the hero bonus and CheckAbility.
    pub fn unitStat(self: *const Hero, pr: *const Proto, index: i32) i32 {
        const u = &(self.squad[0] orelse return 0);
        const t = &pr.units[@intCast(u.type)];
        const base = switch (index) {
            1 => t.life,
            2 => t.attack,
            3 => t.counter_attack,
            4 => t.defence,
            5 => t.ranged_defence,
            6 => t.resist,
            7 => t.speed,
            8 => t.ranged_attack,
            9 => t.shooting_range,
            10 => t.ammo,
            11 => t.stamina,
            12 => t.morale,
            else => 0,
        };
        return base + u.checkUpg(pr, index) + u.checkAbility(pr, index) + self.heroBonus(pr, index);
    }

    /// GetSpellSlot (Hero.cpp:593): the book places of circle `level`.
    pub fn spellSlots(self: *const Hero, pr: *const Proto, level: i32) i32 {
        var r = self.heroBonus(pr, 900 + level);
        if (level == 1) r += 3;
        const n: usize = @intCast(std.math.clamp(self.magic, 0, proto_mod.stat_levels));
        for (pr.stats[1][0..n]) |l| r += @as(i32, @intFromBool(l[0] == level)) + @intFromBool(l[1] == level);
        return r;
    }

    /// GetUnitSlot (Hero.cpp:610): the squad places of rank `level`;
    /// arGarrison (0x390) for a garrison.
    pub fn unitSlots(self: *const Hero, pr: *const Proto, level: i32) i32 {
        if (int(&self.rec, 0x390) >= 0) return int(&self.rec, 0x390 + 4 * @as(usize, @intCast(level - 1)));
        var r: i32 = if (level == 1) 3 else 0;
        const n: usize = @intCast(std.math.clamp(self.leader, 0, proto_mod.stat_levels));
        for (pr.stats[2][0..n]) |l| r += @as(i32, @intFromBool(l[0] == level)) + @intFromBool(l[1] == level);
        return r;
    }

    /// GetSkillSlot (Hero.cpp:710).
    pub fn skillSlots(self: *const Hero) i32 {
        var r: i32 = 5;
        for ([2]i32{ 9, 19 }) |lv| {
            if (self.level > lv) r += if (self.class2 == self.class1) 1 else 3;
        }
        return r;
    }

    /// GetSiege (Hero.cpp:1452); `castle_bonus` is GetBldParam1(42).
    pub fn siegePower(self: *const Hero, pr: *const Proto, castle_bonus: i32) i32 {
        var result: i32 = 2;
        if (self.class1 > 0) result += self.health + self.heroBonus(pr, 55);
        for (self.squad, 0..) |slot, i| {
            const u = &(slot orelse continue);
            if (i == 0 and self.class1 != 0) continue;
            result += pr.units[@intCast(u.type)].level + u.checkAbility(pr, 55) + u.checkUpg(pr, 55);
            if (i == 0) result += self.heroBonus(pr, 55);
        }
        if (self.player > 0) result += castle_bonus;
        return @max(result, 1);
    }

    /// The hero's own unit: life and its maximum (GetLife with the bonus).
    pub fn life(self: *const Hero, pr: *const Proto) [2]i32 {
        const u = &(self.squad[0] orelse return .{ 0, 1 });
        return .{ u.life, u.maxLife(pr) + self.heroBonus(pr, bonus.life) };
    }

    /// ExpForLevel (Hero.cpp:755).  `exp_pct` is the difficulty's iExp for
    /// a human, 100 - iDifficulty·5 for a computer.
    pub fn expForLevel(self: *const Hero, pr: *const Proto, level: i32, exp_pct: i32) i32 {
        var result: i32 = 0;
        var adder: i32 = unit_mod.exp_first_level * 2;
        if (self.squad[0]) |u| adder = @divTrunc(adder * pr.units[@intCast(u.type)].exp_mod, 100);
        var i: i32 = 0;
        while (i < level) : (i += 1) {
            result += adder;
            adder += unit_mod.exp_add * (3 + @divTrunc(i + 1, 10) * 3 + @divTrunc(i + 1, 20) * 3);
        }
        return @divTrunc(result * exp_pct, 100);
    }
};

fn talent(pr: *const Proto, class: i32, index: i32) i32 {
    if (class < 0 or class >= pr.classes.len) return 0;
    var r: i32 = 0;
    for (pr.classes[@intCast(class)].talents) |t| {
        if (t.id == index) r += t.value;
    }
    return r;
}

/// arTalent entries of the squad: id 1000 + bonus index.
/// HERO_QUEST_LIMIT, MAX_QUEST_TASK and sizeof(dcQuest).
pub const quest_limit = 10;
pub const quest_tasks = 4;
const quest_size = 0x24;

/// dcQuest: Index, ipProvince, ipSite, iTime, bComplete, arProgress.
pub const Quest = struct {
    index: i32 = 0,
    province: i32 = 0,
    site: i32 = 0,
    time: i32 = 0,
    complete: bool = false,
    progress: [quest_tasks]i32 = @splat(0),
};

pub const Inventory = [inventory_size + 1][2]i32;

/// DropLoot (Hero.cpp:1292): a random inventory slot is emptied; the
/// looter's `skill` rerolls empty slots.  Returns (item, durability).
pub fn dropLoot(r: *Rand, inv: *Inventory, skill: i32) [2]i32 {
    var drop: usize = r.random(inventory_size) + 1;
    var i: i32 = 0;
    while (i < skill) : (i += 1) {
        if (inv[drop][0] == 0 and r.random(100) > 49) drop = r.random(inventory_size) + 1;
    }
    if (inv[drop][0] <= 0) return .{ 0, 0 };
    const out = inv[drop];
    inv[drop] = .{ 0, 0 };
    return out;
}

/// TakeLoot (Hero.cpp:1313): the item goes to its slot if the class may
/// use it and the slot is free, else to the backpack (16..25); false when
/// there is no room.  Contracts, blueprints and ritual scrolls (classes
/// 11..13) count as taken: the player's stocks they add to are not ported.
pub fn takeLoot(pr: *const Proto, hero: *const Hero, inv: *Inventory, item: i32, dur: i32) bool {
    const it = &pr.items[@intCast(item)];
    if (it.class >= 11 and it.class <= 13) return true;
    if (hero.canUse(pr, item)) {
        const slots: []const usize = switch (it.slot) {
            1, 3, 4, 5, 6, 7, 8, 9 => &[_]usize{@intCast(it.slot)},
            2 => if (inv[1][0] == 0 or pr.items[@intCast(inv[1][0])].slot != 100) &[_]usize{2} else &.{},
            10 => &[_]usize{ 10, 11 },
            11 => &[_]usize{ 12, 13, 14, 15 },
            100 => if (inv[2][0] == 0) &[_]usize{1} else &.{},
            else => &.{},
        };
        for (slots) |s| if (inv[s][0] == 0) {
            inv[s] = .{ item, dur };
            return true;
        };
    }
    for (16..inventory_size + 1) |s| if (inv[s][0] == 0) {
        inv[s] = .{ item, dur };
        return true;
    };
    return false;
}

/// Fearless (19) and No discord (98): abilities GetSquadMorale checks.
pub const ability_fearless = 19;
pub const ability_no_discord = 98;

/// CheckAbility + CheckUpg of squad slot `i`; the hero's own unit adds
/// CheckHeroBonus.
fn squadUpg(pr: *const Proto, squad: *const [max_squad]?Unit, i: usize, hero: ?*const Hero, index: i32) i32 {
    const u = &(squad[i] orelse return 0);
    var r = u.checkAbility(pr, index) + u.checkUpg(pr, index);
    if (i == 0) if (hero) |h| {
        r += h.heroBonus(pr, index);
    };
    return r;
}

fn unitKarma(pr: *const Proto, u: Unit) i32 {
    return pr.units[@intCast(u.type)].karma;
}

/// dcHero::GetSquadMorale (Hero.cpp:1496): the morale squad slot `num`
/// gets from the karma of the others (slots 1..15) and, when the squad
/// belongs to a player (ipPlayer > 0), from the player's karma.  A unit
/// without karma takes half the mean of the others'.  `hero` is null for
/// a guard (its CheckHeroBonus and CheckUnitBonus are 0).
pub fn squadMorale(pr: *const Proto, squad: *const [max_squad]?Unit, num: usize, player_karma: ?i32, hero: ?*const Hero) i32 {
    const me = squad[num] orelse return 0;
    if (squadUpg(pr, squad, num, hero, ability_fearless) > 0) return 0;
    var result: i32 = 0;
    const k = unitKarma(pr, me);
    if (k != 0) {
        const sign = std.math.sign(k);
        var anti: i32 = 0;
        var best: i32 = 0;
        var neutral = false;
        for (1..max_squad) |i| {
            if (i == num) continue;
            const o = squad[i] orelse continue;
            const ok = unitKarma(pr, o);
            const ak: i32 = @intCast(@abs(ok));
            if (ok == 0) {
                neutral = true;
            } else if (std.math.sign(ok) != sign) {
                anti += ak;
            } else if (best < ak - 1) best = ak - 1;
        }
        if (anti > 0) {
            result -= @intCast(@abs(k));
            result -= @divTrunc(anti, 3);
        } else {
            if (!neutral) result += 1;
            result += best;
        }
        if (player_karma) |pk| {
            if (pk >= 50 and sign > 0) result += 1;
            if (pk <= -100 and sign < 0) result += 1;
            if (std.math.sign(pk) != sign) {
                const apk: i32 = @intCast(@abs(pk));
                result -= if (apk <= 250) @divTrunc(apk, 50) else 5;
            }
        }
    } else {
        var count: i32 = 0;
        for (1..max_squad) |i| {
            if (i == num) continue;
            const o = squad[i] orelse continue;
            if (unitKarma(pr, o) == 0) continue;
            count += 1;
            result += squadMorale(pr, squad, i, player_karma, hero);
        }
        if (count > 0) result = @divTrunc(result, count * 2);
    }
    const no_discord = squadUpg(pr, squad, num, hero, ability_no_discord) > 0 or
        (if (hero) |h| h.unitBonus(pr, ability_no_discord) > 0 else false);
    if (result < 0 and no_discord) result = 0;
    return result;
}

fn unitTalent(pr: *const Proto, class: i32, index: i32) i32 {
    if (class < 0 or class >= pr.classes.len) return 0;
    var r: i32 = 0;
    for (pr.classes[@intCast(class)].talents) |t| {
        if (t.id > 1000 and @rem(t.id, 1000) == index) r += t.value;
    }
    return r;
}

fn int(b: []const u8, off: usize) i32 {
    return std.mem.readInt(i32, b[off..][0..4], .little);
}

fn setI(b: []u8, off: usize, v: i32) void {
    std.mem.writeInt(i32, b[off..][0..4], v, .little);
}
