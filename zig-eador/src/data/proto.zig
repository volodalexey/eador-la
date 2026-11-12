//! Prototypes: the tables Loader() and LoadPhrases() read at start-up
//! (Prototype.cpp:1025-...).  Loaders follow the C++ read order; where the
//! GOG 1.05.02 files differ from the 1.05.1 code, the file layout wins and
//! the difference is noted at the loader.
//!
//! All text lives in one arena owned by `Proto`.  Pictures are not loaded
//! here: the matching datafile object has the same index (ClassPic[i] is
//! hero_class.dat object i, pFirstAbilityNum[i].Pic is ability.dat object i)
//! and is drawn through gfx.Sheet.
const std = @import("std");
const GameFs = @import("../platform/fs.zig").GameFs;
const varstream = @import("../formats/varstream.zig");
const Reader = varstream.Reader;
const Pair = varstream.Pair;
const strings = @import("strings.zig");

/// MAX_CLASS_NAME: base classes 1..4 (records 0..3), subclasses
/// ipClass1*4-1+ipClass2 (4..19), level-20 classes 15+ipClass1*4+ipClass2
/// (20..35).
pub const max_class = 36;
/// MAX_TALENT is 4 in the C++ code, GOG hero_class.var has Talent1..3.
pub const max_talent = 3;
/// MAX_ITEM_SLOT_NAME.
pub const max_item_slot = 13;
/// names.var: four lists of hero names (pName[0..3]).
pub const name_lists = 4;

pub const Talent = struct { id: i32 = 0, value: i32 = 0 };

pub const HeroClass = struct {
    name: []const u8,
    talents: [max_talent]Talent,
    /// text/hero_class.txt (ClassText).
    text: []const u8 = "",
    /// text/hero_class_d.txt (ClassDesc), may contain "%s".
    desc: []const u8 = "",
};

/// stAbilityNum: a hero/unit ability with a number.
pub const Ability = struct {
    name: []const u8,
    text: []const u8 = "",
    num: i32,
    /// -1 negative, 0 neutral, 1 positive.
    effect: i32,
    numeric: bool,
    percent: bool,
};

/// stEffect: special effect of a spell or a unit ability.
pub const Effect = struct {
    name: []const u8,
    text: []const u8 = "",
    ability: i32,
    spell: i32,
    numeric: bool,
};

/// clTerrainType.
pub const Terrain = struct {
    name: []const u8,
    /// 0 water, 1 land, 2 mountain.
    passability: i32,
    gold: i32,
    gem: i32,
    population_mod: i32,
    explore_mod: i32,
    bf_pass: i32,
    bf_impass: i32,
};

/// clBF_Object: battlefield obstacle.
pub const BfObject = struct {
    name: []const u8,
    /// 0 impassable.
    move_cost: i32,
    stamina_cost: i32,
    counter_attack: i32,
    defence: i32,
    ranged_defence: i32,
    shooting_range: i32,
};

/// clResource.
pub const Resource = struct {
    name: []const u8,
    text: []const u8 = "",
    /// 0 common, 1 ordinary, 2 rare.
    rarity: i32,
    gold: i32,
    gem: i32,
};

/// MAX_TERRAIN, MAX_RACE_QUEST, MAX_RACE.
pub const max_terrain = 6;
pub const max_race_quest = 10;
pub const max_race = 30;

/// clRace.
pub const Race = struct {
    name: []const u8,
    text: []const u8 = "",
    prov_type: i32,
    gold: i32,
    gem: i32,
    karma: i32,
    corrupt: i32,
    grow: i32,
    /// Home terrain: no population growth penalty (Province.cpp:691).
    terra: i32,
    unrest: i32,
    /// Quest indices for an alliance; unused slots are 0.
    quests: [max_race_quest]i32 = @splat(0),
    n_quests: usize = 0,
    /// Plains..Tundra in file order.  The C++ loader writes them to
    /// arTerrain1[2..7] (one past the end); the array is never read.
    terrain: [max_terrain]i32 = @splat(0),
};

/// MAX_UPGRADE_NEED, MAX_UPGRADE_SIZE.
pub const max_upgrade_need = 3;
pub const max_upgrade_size = 5;

/// clUnitUpgrade: a level-up upgrade or a unit ability (unit_upg.var).
pub const UnitUpgrade = struct {
    name: []const u8,
    text: []const u8 = "",
    only_once: bool,
    /// Required upgrades; unused slots are 0.
    need: [max_upgrade_need]i32 = @splat(0),
    /// (upgrade type, quantity).
    upg: [max_upgrade_size]Pair = @splat(.{}),
    n_upg: usize = 0,
};

/// MAX_UNIT_RES, MAX_UNIT_ABIL, MAX_UPG_PER_LVL, MAX_LOOT_PER_LVL.
pub const max_unit_res = 3;
pub const max_unit_abil = 10;
pub const max_upg_per_lvl = 6;
pub const max_loot_per_lvl = 5;
pub const unit_levels = 20;

/// clUnitType (unit.var).  Record 0 is "Пусто"; record i > 0 draws
/// units.dat / unit_shadow.dat / unit_shadowf.dat / unit_icons.dat object
/// i-1 (Prototype.cpp:1875).
pub const UnitType = struct {
    name: []const u8,
    text: []const u8 = "",
    level: i32,
    life: i32,
    attack: i32,
    counter_attack: i32,
    defence: i32,
    ranged_defence: i32,
    resist: i32,
    speed: i32,
    ranged_attack: i32,
    shooting_range: i32,
    ammo: i32,
    stamina: i32,
    morale: i32,
    exp: i32,
    exp_mod: i32,
    gold_price: i32,
    gem_price: i32,
    gold_payment: i32,
    gem_payment: i32,
    race: i32,
    unit_kind: i32,
    unit_class: i32,
    karma: i32,
    /// unit_sound.dat objects.
    sound_hit: i32,
    sound_shoot: i32,
    sound_shoot_hit: i32,
    sound_death: i32,
    missile: i32,
    /// Required resources; unused slots are 0.
    res: [max_unit_res]i32 = @splat(0),
    /// Starting abilities (unit_upg.var indices); unused slots are 0.
    abilities: [max_unit_abil]i32 = @splat(0),
    /// Per level: (upgrade, weight) candidates; arUpg in the C++ code.
    upgrades: [unit_levels][max_upg_per_lvl]Pair = @splat(@splat(.{})),
    /// Per level: (item, weight); arLoot.
    loot: [unit_levels][max_loot_per_lvl]Pair = @splat(@splat(.{})),

    /// Datafile object index of the picture, shadow and icon (null for 0).
    pub fn picIndex(index: usize) ?usize {
        return if (index == 0) null else index - 1;
    }
};

/// MAX_UNIT_KIND, MAX_ANTI_EFFECT, MAX_SPELL_EFFECT.
pub const max_unit_kind = 5;
pub const max_anti_effect = 10;
pub const max_spell_effect = 20;

pub const SpellEffect = struct {
    id: i32 = 0,
    power: i32 = 0,
    /// -1 instant, 0 permanent.
    duration: i32 = 0,
};

/// clSpell (spell.var).  Picture: spells.dat object with the same index.
pub const Spell = struct {
    name: []const u8,
    text: []const u8 = "",
    cost: i32,
    life_cost: i32,
    stamina_cost: i32,
    item_level: i32,
    level: i32,
    power_mod: i32,
    duration_mod: i32,
    /// 0 none, 1 live unit, 2 corpse, 3 tile, 4 corpse on a free tile,
    /// 5 tile without units.
    target: i32,
    /// 0 caster, 1 target, 2 whole battlefield.
    area: i32,
    radius: i32,
    karma: i32,
    building: i32,
    negative: bool,
    on_enemy: bool,
    on_ally: bool,
    sacrifice: bool,
    /// ResistPower / ResistDuration > 0 set the flag and iResistMod
    /// (the later field wins, as in the C++ code).
    resist_power: bool,
    resist_duration: bool,
    resist_mod: i32 = 0,
    defence_power: bool,
    restore_only: bool,
    cumulative: bool,
    /// spell_sound.dat object.
    sound: i32,
    /// Unit kinds the spell does not affect; unused slots are 0.
    immune_kinds: [max_unit_kind]i32 = @splat(0),
    anti_effects: [max_anti_effect]i32 = @splat(0),
    effects: [max_spell_effect]SpellEffect = @splat(.{}),
    n_effects: usize = 0,
};

/// stat.var: arStat[stat][level] = three values per level of a hero stat
/// (0 Health: Life, Stamina, Morale; 1 Magic, 2 Leader: slot counts).
pub const stat_count = 3;
pub const stat_levels = 10;
pub const StatTable = [stat_count][stat_levels][3]i32;
/// stat_progress.var: arStatProgress[stat][class], 20 class records.
pub const progress_classes = 20;
pub const StatProgress = [stat_count][progress_classes]i32;

/// MAX_HERO_SKILL (records 0..28), MAX_SKILL_UPG, five skill levels.
pub const max_hero_skill = 28;
pub const max_skill_upg = 3;
pub const skill_levels = 5;

/// clSkill (skill.var).  Picture: skills.dat object with the same index.
pub const Skill = struct {
    name: []const u8,
    /// text/skill.txt (arSkillText).
    text: []const u8 = "",
    /// Base class the skill belongs to.
    class: i32,
    /// Per level: (bonus id, value) for the hero and for the squad.
    hero: [skill_levels][max_skill_upg]Pair = @splat(@splat(.{})),
    squad: [skill_levels][max_skill_upg]Pair = @splat(@splat(.{})),
};

/// MAX_GUARD_UNIT, MAX_GUARD_LOOT, MAX_GUARD_LEVEL.
pub const max_guard_unit = 16;
pub const max_guard_loot = 10;
pub const max_guard_level = 4;

pub const GuardUnit = struct {
    unit: i32 = 0,
    level: i32 = 0,
    min_count: i32 = 0,
    possibility: i32 = 0,
};

/// clGuard (guard.var): a concrete guard squad.
pub const Guard = struct {
    /// "Quantity: num/add": minimum warriors and random extra.
    num: i32,
    num_add: i32,
    /// "Initiative: quick/add".
    quick: i32,
    quick_add: i32,
    /// Loot chance, percent.
    loot_poss: i32 = 0,
    loot_num: i32 = 0,
    power: i32 = 0,
    units: [max_guard_unit]GuardUnit = @splat(.{}),
    n_units: usize = 0,
    /// (item, possibility).
    loot: [max_guard_loot]Pair = @splat(.{}),
    n_loot: usize = 0,
};

/// clGuardType (guard_type.var).
pub const GuardType = struct {
    name: []const u8,
    level: i32,
    /// Subtracted from the karma of the player who kills the guard.
    karma: i32,
    /// guard.var index per guard level.
    guards: [max_guard_level]i32,
};

/// MAX_PROVINCE_GUARD, MAX_PROVINCE_EVENT, MAX_PROVINCE_LEVEL, MAX_NAME_TYPE.
pub const max_province_guard = 4;
pub const max_province_event = 10;
pub const max_province_level = 11;
pub const max_name_type = 5;

pub const ProvinceGuard = struct {
    guard_type: i32 = 0,
    possibility: i32 = 0,
    /// Dialog shown on meeting, encounter for negotiations.
    dialog: i32 = 0,
    encounter: i32 = 0,
};

/// clProvinceType (province_type.var).
pub const ProvinceType = struct {
    name: []const u8,
    race: i32,
    /// 0 any, 1 coastal only, 2 inland only, 3 on a river.
    coast: i32,
    sites: i32,
    gold: i32,
    gem: i32,
    difficult: i32,
    population: i32,
    level_pop: i32,
    dipl_unrest: i32,
    conq_unrest: i32,
    dipl_karma: i32,
    conq_karma: i32,
    explored: i32,
    level_explored: i32,
    /// Terrain_Possibility as written in the file: Plains..Tundra
    /// (terrain 1..6).  Use `terrainWeight`.
    terrain_file: [max_terrain]i32 = @splat(0),
    guards: [max_province_guard]ProvinceGuard = @splat(.{}),
    n_guards: usize = 0,
    /// (event, possibility).
    events: [max_province_event]Pair = @splat(.{}),
    n_events: usize = 0,

    /// arTerrain[t] as the game sees it.  arTerrain has MAX_TERRAIN = 6
    /// slots but the loader writes indices 1..6, so Tundra (6) lands in
    /// arGuard[0][0] and the guard list read afterwards overwrites it: the
    /// tundra weight is the type of the first guard (FUN_0045ef60, words
    /// 0x15..0x1A, arGuard at 0x1A).  Index 0 is never written (0).
    pub fn terrainWeight(self: *const ProvinceType, terrain: usize) i32 {
        return switch (terrain) {
            1...5 => self.terrain_file[terrain - 1],
            6 => self.guards[0].guard_type,
            else => 0,
        };
    }
};

/// clProvinceLevel (province_level.var).
pub const ProvinceLevel = struct {
    name: []const u8,
    max_population: i32,
    /// Income change, percent.
    income: i32,
    /// Explored land needed.
    explored: i32,
};

/// clProvinceName (province_name.var).
pub const ProvinceName = struct {
    name: []const u8,
    rarity: i32,
    coast: i32,
    /// Allowed terrains (0 empty, 8 water); unused slots are 0.
    terrains: [max_terrain + 1]i32 = @splat(0),
    n_terrains: usize = 0,
    types: [max_name_type]i32 = @splat(0),
    n_types: usize = 0,

    /// CheckType: true when no type is listed or `t` is among them.
    pub fn checkType(self: *const ProvinceName, v: i32) bool {
        var result = true;
        for (self.types) |x| if (x > 0) {
            if (x == v) return true;
            result = false;
        };
        return result;
    }

    /// CheckTerrain: true when no terrain is listed or `t` is among them.
    pub fn checkTerrain(self: *const ProvinceName, v: i32) bool {
        var result = true;
        for (self.terrains) |x| if (x > 0) {
            if (x == v) return true;
            result = false;
        };
        return result;
    }
};

/// MAX_BUILDING_RES, MAX_RESOURCE_BONUS, MAX_BUILDING_ABILITY.
pub const max_building_res = 6;
pub const max_resource_bonus = 3;
pub const max_building_ability = 7;

/// (index, value, extra value) of a building or site ability.
pub const Ability3 = struct { id: i32 = 0, p1: i32 = 0, p2: i32 = 0 };

/// clOuterBuilding (outer_build.var).  Record i > 0 draws outbuild.dat
/// object i-1 (icon), outbld.dat (i-1)*2 (building) and (i-1)*2+1 (shadow)
/// (Prototype.cpp:2797).
pub const OuterBuilding = struct {
    name: []const u8,
    text: []const u8 = "",
    /// Previous building for upgrades.
    upgrade: i32,
    /// Castle building that allows it.
    building: i32,
    gold_cost: i32,
    gem_cost: i32,
    gold_add: i32,
    gem_add: i32,
    gold_perc: i32,
    gem_perc: i32,
    /// "Mood" in the file (iUnrest).
    unrest: i32,
    attitude: i32,
    grow: i32,
    karma: i32,
    heal: i32,
    /// 0 any, 1 coastal only, 2 inland only, 3 on a river.
    coastal: i32,
    /// Port: maximum sailing range.
    port: i32,
    wall: i32,
    tower_health: i32,
    tower_shoot: i32,
    /// 1..4 simple (tower count), 10 magic (shoots every 2 turns).
    tower_type: i32,
    library: bool,
    shop: bool,
    /// Allowed terrains Plain..Tundra (terrain 1..6).
    terrain: [max_terrain]bool = @splat(false),
    res: [max_building_res]i32 = @splat(0),
    /// (resource, income modifier).
    res_bonus: [max_resource_bonus]Pair = @splat(.{}),
    n_res_bonus: usize = 0,
    abilities: [max_building_ability]Ability3 = @splat(.{}),
    n_abilities: usize = 0,

    pub fn iconIndex(index: usize) ?usize {
        return if (index == 0) null else index - 1;
    }
    pub fn buildingIndex(index: usize) ?usize {
        return if (index == 0) null else (index - 1) * 2;
    }
    pub fn shadowIndex(index: usize) ?usize {
        return if (index == 0) null else (index - 1) * 2 + 1;
    }
};

/// MAX_DEFENDER_RES, MAX_DEFENDER_ABILITY.
pub const max_defender_res = 4;
pub const max_defender_ability = 6;

pub const DefenderUnit = struct { unit: i32 = 0, level: i32 = 0, count: i32 = 0 };

/// clDefender (defender.var): a hireable province guard.  Record i > 0
/// draws defenders.dat object i-1 (Prototype.cpp:2965).
pub const Defender = struct {
    name: []const u8,
    text: []const u8 = "",
    /// illustration.dat object ("Illustration").
    picture: i32,
    /// Strength, used by the AI.
    power: i32,
    gold_cost: i32,
    gem_cost: i32,
    gold_payment: i32,
    gem_payment: i32,
    /// Castle building that allows hiring.
    building: i32,
    initiative: i32,
    karma: i32,
    loot_poss: i32,
    loot_num: i32,
    no_dismiss: bool,
    /// Allowed terrains Plain..Tundra; MainAI indexes it as arTerrain[T].
    terrain: [max_terrain]bool = @splat(false),
    res: [max_defender_res]i32 = @splat(0),
    units: [max_guard_unit]DefenderUnit = @splat(.{}),
    n_units: usize = 0,
    /// (item, possibility).
    loot: [max_guard_loot]Pair = @splat(.{}),
    n_loot: usize = 0,
    abilities: [max_defender_ability]Ability3 = @splat(.{}),
    n_abilities: usize = 0,

    pub fn picIndex(index: usize) ?usize {
        return if (index == 0) null else index - 1;
    }
};

/// MAX_SITE_GUARD, MAX_SITE_ABILITY.
pub const max_site_guard = 30;
pub const max_site_ability = 7;

/// clSite (site.var): a special place in a province.  Record i > 0 draws
/// site.dat object i-1 (Eador.cpp:677); GOG site.dat has 64 objects for 65
/// sites, the last one has no picture.
pub const Site = struct {
    name: []const u8,
    /// text/site.txt and text/site1.txt (the site without guards).
    text: []const u8 = "",
    free_text: []const u8 = "",
    min_level: i32,
    max_level: i32,
    /// Chance that the site is guarded, percent.
    guarded: i32,
    /// 0 not generated, 1 generated, 2 only on unexplored land.
    place: i32,
    event: i32,
    dialog: i32,
    dialog2: i32,
    /// Treasure level offset.
    treasure: i32,
    /// 0 any, 1 coastal only, 2 inland only, 3 on a river.
    coastal: i32,
    work_hide: bool,
    work_guard: bool,
    work_free: bool,
    can_enter: bool,
    self_destroy: bool,
    /// Generation weight per terrain Plain..Tundra; Mapper indexes it as
    /// arTerrain[ipTerrain-1].
    terrain: [max_terrain]i32 = @splat(0),
    /// (guard type, possibility).
    guards: [max_site_guard]Pair = @splat(.{}),
    n_guards: usize = 0,
    abilities: [max_site_ability]Ability3 = @splat(.{}),
    n_abilities: usize = 0,

    pub fn picIndex(index: usize) ?usize {
        return if (index == 0) null else index - 1;
    }
};

/// MAX_RITUAL_RES, MAX_RITUAL_EFFECT.
pub const max_ritual_res = 3;
pub const max_ritual_effect = 5;

/// clRitual (ritual.var).  Record i > 0 draws ritual.dat object i-1
/// (Prototype.cpp:3119).
pub const Ritual = struct {
    name: []const u8,
    text: []const u8 = "",
    gold_cost: i32,
    gem_cost: i32,
    /// Sound: < 0 unit_sound.dat |n|, otherwise spell_sound.dat n (the
    /// commented-out code in Prototype.cpp:3137).
    sound: i32,
    karma: i32,
    /// Can be cast on an own / an enemy province.
    on_ally: bool,
    on_enemy: bool,
    time: i32,
    cooldown: i32,
    /// Event for the province owner on the next turn.
    event: i32,
    item_level: i32,
    item_rarity: i32,
    res: [max_ritual_res]i32 = @splat(0),
    effects: [max_ritual_effect]Ability3 = @splat(.{}),
    n_effects: usize = 0,

    pub fn picIndex(index: usize) ?usize {
        return if (index == 0) null else index - 1;
    }
};

/// MAX_ITEM_EFFECT, MAX_ITEM_RES.
pub const max_item_effect = 6;
pub const max_item_res = 4;

/// Item classes of the generated items (Prototype.cpp:3311-3428).
pub const item_class_scroll = 10;
pub const item_class_contract = 11;
pub const item_class_blueprint = 12;
pub const item_class_ritual = 13;

/// Where an item picture lives: sheet + object index.
pub const Pic = struct {
    pub const Sheet = enum { items, defenders, outbuild, ritual };
    sheet: Sheet,
    index: usize,
};

/// clItem.  Records 0..iMaxPureItem come from item.var, then one scroll per
/// spell, a contract per defender with ability 10, a blueprint per outer
/// building with ability 8 and a scroll per ritual (in this order: item
/// indices in saves depend on it).
pub const Item = struct {
    name: []const u8 = "",
    /// Weapon, armour, cloak, ... (item_slot.var).
    slot: i32 = 0,
    /// Item type the hero class must allow (item_type.var).
    class: i32 = 0,
    durability: i32 = 0,
    gold_cost: i32 = 0,
    gem_cost: i32 = 0,
    shop_level: i32 = 0,
    rarity: i32 = 0,
    building: i32 = 0,
    pic: ?Pic = null,
    /// (index, power, area: 0 hero, 1 squad).
    effects: [max_item_effect]Ability3 = @splat(.{}),
    n_effects: usize = 0,
    res: [max_item_res]i32 = @splat(0),
};

/// MAX_ANSWER, MAX_HERO_PIC.
pub const max_answer = 4;
pub const max_hero_pic = 16;

/// Picture of a dialog: an illustration or a ruler avatar.
pub const DialogPic = union(enum) {
    /// illustration.dat object (bmpIllustration).
    illustration: usize,
    /// Avatar n: portraits.dat / smallport.dat object n + MAX_HERO_PIC.
    avatar: usize,

    pub fn portraitIndex(n: usize) usize {
        return n + max_hero_pic;
    }
};

/// clDialog (dialog.var, campaign_dialog.var).
pub const Dialog = struct {
    text: []const u8,
    /// Window type.
    window: i32,
    pic: DialogPic,
    answers: [max_answer][]const u8 = @splat(""),
    n_answers: usize = 0,
};

/// clJournal (text/journal.txt).
pub const JournalEntry = struct { name: []const u8, text: []const u8 };

/// clQuestion (question.var).
pub const Question = struct {
    text: []const u8,
    quester: i32,
    /// Flag that makes the question visible.
    trigger: i32,
    /// Event the question leads to.
    event: i32,
};

/// MAX_BUILDING_NEED, MAX_BLD, MAX_BLD_GROUP, MAX_MERC_LEVEL, MAX_MERCENARY.
pub const max_building_need = 5;
pub const max_bld = 1000;
pub const max_bld_group = 30;
pub const max_merc_level = 5;
pub const max_mercenary = 11;

/// clInnerBuilding (inner_build.var): a castle building.  Record i draws
/// innerbld.dat object i (Prototype.cpp:3710; object 0 is "000Empty").
pub const InnerBuilding = struct {
    name: []const u8,
    text: []const u8 = "",
    gold_cost: i32,
    gem_cost: i32,
    /// build_group.var group.
    group: i32,
    /// Slot on the building screen.
    slot: i32,
    /// Building it replaces.
    upgrade: i32,
    /// Invisible until the conditions are met.
    hidden: i32,
    /// Shard level it appears on (0 available at once).
    level: i32,
    /// Required buildings; unused slots are 0.
    needs: [max_building_need]i32 = @splat(0),
    res: [max_building_res]i32 = @splat(0),
    abilities: [max_building_ability]Ability3 = @splat(.{}),
    n_abilities: usize = 0,
};

/// build_group.var: arBldGroup (maximum buildings of the group) and GroupName.
pub const BuildGroup = struct { max: i32, name: []const u8 };

/// MAX_PLAYERS, MAX_PLAYER_BLD, MAX_PLAYER_DIALOG, MAX_DIFFICULTY.
pub const max_players = 17;
pub const max_player_bld = 20;
pub const max_player_dialog = 50;
pub const max_difficulty = 7;

/// clDipText: one diplomatic message of text/diplomacy/playerN.txt.
pub const DipText = struct {
    text: []const u8,
    /// Message type.
    index: i32,
    /// Allowed relation range.
    min: i32,
    max: i32,
    /// Uses the ruler's name.
    name: bool,
    /// Selection parameter (advantage, profit, ...).
    param: i32,
};

/// clPlayerProfile (players.var): a computer ruler.  Record i > 0 has its
/// diplomacy in text/diplomacy/player<i>.txt; player0.txt is the human
/// side (pHumanDiplo).
pub const PlayerProfile = struct {
    name: []const u8,
    /// Home province name.
    land: []const u8,
    karma: i32,
    strategy: i32,
    magic: i32,
    targeting: i32,
    diplomacy: i32,
    aggro: i32,
    greed: i32,
    loyal: i32,
    random: i32,
    /// Coat of arms sign ("Herb" in the file).
    sign: i32,
    color1: i32,
    color2: i32,
    /// Position of the ruler's world in the campaign.
    y: i32,
    event_first_meet: i32,
    event_meet: i32,
    event_defeat: i32,
    event_attack: i32,
    land_type: i32 = 0,
    land_poss: i32 = 0,
    prov_type: i32 = 0,
    prov_poss: i32 = 0,
    /// Attitude to the other computer rulers; [0] is always 0.
    attitude: [max_players]i32 = @splat(0),
    buildings: [max_player_bld]i32 = @splat(0),
    enter_dialogs: [max_player_dialog]i32 = @splat(0),
    n_enter_dialogs: usize = 0,
    exit_dialogs: [max_player_dialog]i32 = @splat(0),
    n_exit_dialogs: usize = 0,
    /// Messages in the original list order (reverse file order: the C++
    /// loader prepends each one).
    dip_texts: []const DipText = &.{},
};

/// MAX_SET_ITEMS, MAX_SET_BONUS, MAX_SET_ABILITY, MAX_MEDAL_EFFECT.
pub const max_set_items = 10;
pub const max_set_bonus = 5;
pub const max_set_ability = 5;
pub const max_medal_effect = 10;

pub const SetBonus = struct {
    /// Items of the set needed for the bonus (arBonusQ).
    items_needed: i32 = 0,
    /// (index, power, area).
    abilities: [max_set_ability]Ability3 = @splat(.{}),
    n_abilities: usize = 0,
};

/// clItemSet (item_set.var).
pub const ItemSet = struct {
    name: []const u8,
    items: [max_set_items]i32 = @splat(0),
    n_items: usize = 0,
    bonuses: [max_set_bonus]SetBonus = @splat(.{}),
    n_bonuses: usize = 0,
};

/// clMedal (medal.var).  Record i > 0 draws medal.dat / medal_small.dat
/// object i-1.
pub const Medal = struct {
    name: []const u8,
    text: []const u8 = "",
    /// Upkeep increase in gold / gems.
    gold_spent: i32,
    gem_spent: i32,
    rarity: i32,
    /// (index, power).
    effects: [max_medal_effect]Pair = @splat(.{}),
    n_effects: usize = 0,

    pub fn picIndex(index: usize) ?usize {
        return if (index == 0) null else index - 1;
    }
};

/// MAX_EVENT_EFFECT.
pub const max_event_effect = 10;

/// (index, power, param1, param2) of an event or encounter effect.
pub const Effect4 = struct { id: i32 = 0, power: i32 = 0, p1: i32 = 0, p2: i32 = 0 };

/// clEvent (event.var, camp_event.var): a province event.
pub const Event = struct {
    /// Good or bad.
    kind: i32,
    possibility: i32,
    dialog: i32,
    /// Dialog parameters: 0 none, 1 province name, 2 value of effect 1.
    dlg_param1: i32,
    dlg_param2: i32,
    /// Attacking squad type.
    attacker: i32,
    event_win: i32,
    event_lose: i32,
    event_draw: i32,
    /// Province type set when neutrals capture the province.
    prov_type: i32,
    karma: i32,
    /// Next event per chosen answer.
    answers: [max_answer]i32 = @splat(0),
    n_answers: usize = 0,
    effects: [max_event_effect]Effect4 = @splat(.{}),
    n_effects: usize = 0,
};

/// clEncounter (encounter.var): a hero adventure.
pub const Encounter = struct {
    kind: i32,
    possibility: i32,
    dialog: i32,
    /// Dialog parameters: 0 none, 1 hero name, 2 value of effect 1.
    dlg_param1: i32,
    dlg_param2: i32,
    attacker: i32,
    enc_win: i32,
    enc_lose: i32,
    enc_draw: i32,
    karma: i32,
    /// Next encounter per chosen answer.
    answers: [max_answer]i32 = @splat(0),
    n_answers: usize = 0,
    effects: [max_event_effect]Effect4 = @splat(.{}),
    n_effects: usize = 0,
};

/// MAX_QUEST_TASK, MAX_QUEST_ABILITY.
pub const max_quest_task = 4;
pub const max_quest_ability = 4;

pub const QuestTask = struct {
    index: i32 = 0,
    param: i32 = 0,
    quantity: i32 = 0,
    target: i32 = 0,
    place: i32 = 0,
};

/// clQuest (quest.var + text/quest.txt).
pub const Quest = struct {
    name: []const u8,
    /// Journal entry.
    text: []const u8 = "",
    /// 0 race, site type for a site, negative for the campaign.
    giver: i32,
    /// Task type 1..8 (Prototype.h:878).
    kind: i32,
    possibility: i32,
    enc_init: i32,
    enc_wait: i32,
    enc_found: i32,
    enc_not_found: i32,
    enc_done: i32,
    enc_success: i32,
    enc_fail_giver: i32,
    enc_fail_target: i32,
    enc_fail_time: i32,
    /// Turns to complete.
    time: i32,
    exp: i32,
    tasks: [max_quest_task]QuestTask = @splat(.{}),
    n_tasks: usize = 0,
    abilities: [max_quest_ability]Effect4 = @splat(.{}),
    n_abilities: usize = 0,
};

/// MAX_MAPOBJ_TYPE, MAX_MAPOBJ_PIC, MAX_GROUP, MAX_CASTLE_BLD, MAX_TERR_LAYER.
pub const max_mapobj_type = 5;
pub const max_mapobj_pic = 20;
pub const max_group = 7;
pub const max_castle_bld = 20;
pub const max_terr_layer = 4;
/// Land types of map objects: Sea, Plain..Tundra, Mountain.
pub const land_types = max_terrain + 2;

/// nature.dat objects used directly (Prototype.cpp:4519).
pub const nature_grass = 0;
pub const nature_swamp = 1;
pub const nature_sand = 34;
pub const nature_dark = 36;
pub const nature_water_first = 253;
pub const nature_water_count = 5;

/// clMapObject (mapobject.var): decoration of the strategic map.
pub const MapObject = struct {
    kind: i32,
    layer: i32,
    /// Offset from the top-left corner to the insertion point.
    dx: i32,
    dy: i32,
    /// Site needed in the province.
    site: i32,
    /// Weight per land type Sea, Plain..Tundra, Mountain.
    land: [land_types]i32 = @splat(0),
    prov_types: [max_mapobj_type]i32 = @splat(0),
    landmark: bool = false,
    /// nature.dat objects; 0 leaves the slot empty, as in the C++ code.
    pics: [max_mapobj_pic]i32 = @splat(0),
    n_pics: usize = 0,
};

/// clLandmark (landmark.var).
pub const Landmark = struct {
    kind: i32,
    layer: i32,
    dx: i32,
    dy: i32,
    land: [land_types]i32 = @splat(0),
    prov_types: [max_mapobj_type]i32 = @splat(0),
    site: i32 = 0,
    /// nature.dat objects; shadow 0 means none.
    pic: i32 = 0,
    shadow: i32 = 0,
};

/// clCastle (castle.var): one picture of the castle screen.  Record i draws
/// castle.dat object i.
pub const CastlePart = struct {
    layer: i32,
    opacity: i32,
    x: i32,
    y: i32,
    group: i32,
    y_place: i32,
    /// Buildings that show the picture (0 always, -1 on the first visit).
    builds: [max_castle_bld]i32 = @splat(0),
    n_builds: usize = 0,
    /// Buildings that hide it.
    anti_builds: [max_castle_bld]i32 = @splat(0),
    n_anti_builds: usize = 0,
};

/// MAX_SHARD_SIZE, MAX_SHARD_LEVEL, MAX_SHARD_BONUS_TYPE, MAX_GROUP_BUILD.
pub const max_shard_size = 8;
pub const max_shard_level = 8;
pub const max_shard_bonus_type = 20;
pub const max_group_build = 4;

/// stShardSize (shard.var).
pub const ShardSize = struct { name: []const u8, x: i32, y: i32, players: i32 };

/// clHeroAdvice: a text for a value range (hero_advice, ruler_nick,
/// ruler_title, attitude).
pub const RangeText = struct { min: i32, max: i32, text: []const u8 };

/// clCredits (credits.var).
pub const Credits = struct { title: []const u8, names: []const []const u8 };

/// clShardBonus (shard_bonus.var).  Record i > 0 draws shardbonus.dat
/// object i-1.
pub const ShardBonus = struct {
    name: []const u8,
    /// 0 number, 1 item, 2 unit, 3 building, 4 resource.
    kind: i32,
    rarity: i32,
    /// Price in energy.
    price: i32,
    /// Per shard level: (min, max) for numbers, (object, rarity) otherwise.
    levels: [max_shard_level][max_shard_bonus_type]Pair = @splat(@splat(.{})),
    n_per_level: [max_shard_level]usize = @splat(0),
};

/// clDifficulty (difficulty.var).
pub const Difficulty = struct {
    name: []const u8,
    gold: i32,
    gem: i32,
    gold_add: i32,
    gem_add: i32,
    population: i32,
    exp: i32,
    loot: i32,
    neutral_life: i32,
    corrupt_start: i32,
    corrupt_cap: i32,
    show_advice: bool,
    score: i32,
};

pub const Proto = struct {
    arena: std.heap.ArenaAllocator,

    shard_sizes: [max_shard_size]ShardSize = undefined,
    /// group_coord.var: arGroupCoord[group][building] = (x, y).
    group_coords: [max_group][max_group_build]Pair = @splat(@splat(.{})),
    hero_advice: []RangeText = &.{},
    ruler_nicks: []RangeText = &.{},
    ruler_titles: []RangeText = &.{},
    attitudes: []RangeText = &.{},
    credits: []Credits = &.{},
    shard_bonuses: []ShardBonus = &.{},
    difficulties: [max_difficulty]Difficulty = undefined,
    map_objects: []MapObject = &.{},
    landmarks: []Landmark = &.{},
    castle_parts: []CastlePart = &.{},
    /// terr_layer.var: arTerrLayer[land type][layer].
    terr_layer: [land_types][max_terr_layer]i32 = @splat(@splat(0)),
    encounters: []Encounter = &.{},
    quests: []Quest = &.{},
    events: []Event = &.{},
    campaign_events: []Event = &.{},
    item_sets: []ItemSet = &.{},
    medals: []Medal = &.{},
    player_profiles: []PlayerProfile = &.{},
    /// text/diplomacy/player0.txt (pHumanDiplo), same order rule.
    human_dip_texts: []const DipText = &.{},
    /// text/message.txt, text/tutorial.txt, text/hint.txt.
    messages: []const []const u8 = &.{},
    tutorial: []const []const u8 = &.{},
    hints: []const []const u8 = &.{},
    /// heroprice.var: iHeroPriceMult, arHeroPrice[4] (gold, gems),
    /// arHeroRess[2] (resurrection base and per-level add).
    hero_price_mult: i32 = 0,
    hero_price: [4]Pair = @splat(.{}),
    hero_resurrect: [2]Pair = @splat(.{}),
    /// campdiff.var: arCampDiff[difficulty][0..4].
    camp_diff: [max_difficulty][5]i32 = @splat(@splat(0)),
    inner_buildings: []InnerBuilding = &.{},
    build_groups: []BuildGroup = &.{},
    /// mercenary.var: arMercenary[level][slot] = (unit, possibility).
    mercenaries: [max_merc_level][max_mercenary]Pair = @splat(@splat(.{})),
    dialogs: []Dialog = &.{},
    campaign_dialogs: []Dialog = &.{},
    journal: []JournalEntry = &.{},
    questions: []Question = &.{},
    /// All items, pure ones first (see Item).
    items: []Item = &.{},
    /// iMaxPureItem: the last item.var record.
    max_pure_item: usize = 0,
    rituals: []Ritual = &.{},
    sites: []Site = &.{},
    defenders: []Defender = &.{},
    /// iMaxDefItem: defender abilities with index 10.
    max_def_item: i32 = 0,
    outer_buildings: []OuterBuilding = &.{},
    /// iMaxBldItem: abilities with index 8 over all outer buildings.
    max_bld_item: i32 = 0,
    guards: []Guard = &.{},
    guard_types: []GuardType = &.{},
    province_types: []ProvinceType = &.{},
    province_levels: []ProvinceLevel = &.{},
    province_names: []ProvinceName = &.{},
    stats: StatTable = std.mem.zeroes(StatTable),
    stat_progress: StatProgress = std.mem.zeroes(StatProgress),
    skills: []Skill = &.{},
    spells: []Spell = &.{},
    unit_upgrades: []UnitUpgrade = &.{},
    units: []UnitType = &.{},

    terrains: []Terrain = &.{},
    bf_objects: []BfObject = &.{},
    resources: []Resource = &.{},
    races: []Race = &.{},

    phrases: []const []const u8 = &.{},
    classes: []HeroClass = &.{},
    hero_names: [name_lists][]const []const u8 = @splat(&.{}),
    prefixes: []const []const u8 = &.{},
    suffixes: []const []const u8 = &.{},
    item_slots: []const []const u8 = &.{},
    unit_classes: []const []const u8 = &.{},
    unit_kinds: []const []const u8 = &.{},
    item_types: []const []const u8 = &.{},
    abilities: []Ability = &.{},
    effects: []Effect = &.{},

    pub fn load(gpa: std.mem.Allocator, fs: *GameFs) !Proto {
        var p: Proto = .{ .arena = .init(gpa) };
        errdefer p.arena.deinit();
        const a = p.arena.allocator();
        var l: Loader = .{ .a = a, .fs = fs };

        p.phrases = try strings.table(a, try l.read("var/phrases.var"), .quantity_plus_one);
        try p.loadClasses(&l);
        try p.loadHeroNames(&l);
        p.prefixes = try strings.table(a, try l.read("var/prefix.var"), .quantity);
        p.suffixes = try strings.table(a, try l.read("var/suffix.var"), .quantity);
        try p.loadClassTexts(&l);
        {
            var r = Reader.init(try l.read("var/item_slot.var"));
            p.item_slots = try strings.names(a, &r, max_item_slot);
        }
        p.unit_classes = try strings.table(a, try l.read("var/unit_class.var"), .quantity_plus_one);
        p.unit_kinds = try strings.table(a, try l.read("var/unit_type.var"), .quantity_plus_one);
        p.item_types = try strings.table(a, try l.read("var/item_type.var"), .quantity_plus_one);
        try p.loadAbilities(&l);
        try p.loadEffects(&l);
        try p.loadTerrains(&l);
        try p.loadBfObjects(&l);
        try p.loadResources(&l);
        try p.loadRaces(&l);
        try p.loadUnitUpgrades(&l);
        try p.loadUnits(&l);
        try p.loadSpells(&l);
        try p.loadStats(&l);
        try p.loadSkills(&l);
        try p.loadGuards(&l);
        try p.loadGuardTypes(&l);
        try p.loadProvinceTypes(&l);
        try p.loadProvinceLevels(&l);
        try p.loadProvinceNames(&l);
        try p.loadOuterBuildings(&l);
        try p.loadDefenders(&l);
        try p.loadSites(&l);
        try p.loadRituals(&l);
        try p.loadItems(&l);
        p.dialogs = try loadDialogs(&l, "var/dialog.var", false);
        p.campaign_dialogs = try loadDialogs(&l, "var/campaign_dialog.var", true);
        try p.loadJournal(&l);
        try p.loadQuestions(&l);
        try p.loadInnerBuildings(&l);
        try p.loadBuildGroups(&l);
        try p.loadMercenaries(&l);
        try p.loadPlayerProfiles(&l);
        try p.loadDiplomacy(&l);
        p.messages = try loadTexts(&l, "text/message.txt");
        p.tutorial = try loadTexts(&l, "text/tutorial.txt");
        p.hints = try loadTexts(&l, "text/hint.txt");
        try p.loadHeroPrices(&l);
        try p.loadCampDiff(&l);
        try p.loadItemSets(&l);
        try p.loadMedals(&l);
        p.events = try loadEvents(&l, "var/event.var");
        p.campaign_events = try loadEvents(&l, "var/camp_event.var");
        try p.loadEncounters(&l);
        try p.loadQuests(&l);
        try p.loadMapObjects(&l);
        try p.loadLandmarks(&l);
        try p.loadCastle(&l);
        try p.loadTerrLayer(&l);
        try p.loadShardSizes(&l);
        try p.loadGroupCoords(&l);
        p.hero_advice = try loadRangeTexts(&l, "var/hero_advice.var");
        p.ruler_nicks = try loadRangeTexts(&l, "var/ruler_nick.var");
        p.ruler_titles = try loadRangeTexts(&l, "var/ruler_title.var");
        p.attitudes = try loadRangeTexts(&l, "var/attitude.var");
        try p.loadCredits(&l);
        try p.loadShardBonuses(&l);
        try p.loadDifficulties(&l);
        // world_rules.var: read only with bUseWorldRules (false in
        // Eador.cpp:15); GOG has no such file.
        return p;
    }

    pub fn deinit(self: *Proto) void {
        self.arena.deinit();
    }

    pub fn phrase(self: *const Proto, i: usize) []const u8 {
        return strings.get(self.phrases, i);
    }

    /// hero_class.var (Prototype.cpp:1096).
    fn loadClasses(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/hero_class.var"));
        self.classes = try l.a.alloc(HeroClass, max_class);
        for (self.classes) |*c| {
            c.* = .{ .name = r.fieldText(), .talents = undefined };
            for (&c.talents) |*tal| {
                _ = r.word();
                tal.id = r.int();
                _ = r.char();
                tal.value = r.int();
            }
        }
    }

    /// names.var (Prototype.cpp:1120): Quantity, then four lists of that size.
    fn loadHeroNames(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/names.var"));
        const n = try strings.quantity(&r, .quantity);
        for (&self.hero_names) |*list| list.* = try strings.names(l.a, &r, n);
    }

    /// text/hero_class.txt, text/hero_class_d.txt (Prototype.cpp:1199).
    fn loadClassTexts(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("text/hero_class.txt"));
        for (self.classes) |*c| c.text = r.entry();
        r = Reader.init(try l.read("text/hero_class_d.txt"));
        for (self.classes) |*c| c.desc = r.entry();
    }

    /// ability_num.var + text/ability.txt (Prototype.cpp:1311).
    fn loadAbilities(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/ability_num.var"));
        self.abilities = try l.a.alloc(Ability, try strings.quantity(&r, .quantity_plus_one));
        for (self.abilities) |*ab| {
            const name = r.fieldText();
            const num = r.field();
            const numeric = r.field();
            const effect = r.field();
            const percent = r.field();
            ab.* = .{ .name = name, .num = num, .numeric = numeric != 0, .effect = effect, .percent = percent != 0 };
        }
        r = Reader.init(try l.read("text/ability.txt"));
        for (self.abilities) |*ab| ab.text = r.entry();
    }

    /// effects.var + text/effects.txt (Prototype.cpp:1348).
    fn loadEffects(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/effects.var"));
        self.effects = try l.a.alloc(Effect, try strings.quantity(&r, .quantity_plus_one));
        for (self.effects) |*e| {
            const name = r.fieldText();
            const ability = r.field();
            const spell = r.field();
            const numeric = r.field();
            e.* = .{ .name = name, .ability = ability, .spell = spell, .numeric = numeric != 0 };
        }
        r = Reader.init(try l.read("text/effects.txt"));
        for (self.effects) |*e| e.text = r.entry();
    }

    /// terrain.var (Prototype.cpp:1535).
    fn loadTerrains(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/terrain.var"));
        self.terrains = try l.a.alloc(Terrain, try strings.quantity(&r, .quantity_plus_one));
        for (self.terrains) |*x| {
            const name = r.fieldText();
            const pass = r.field();
            const gold = r.field();
            const gem = r.field();
            const pop = r.field();
            const explore = r.field();
            const bf_pass = r.field();
            const bf_impass = r.field();
            x.* = .{ .name = name, .passability = pass, .gold = gold, .gem = gem, .population_mod = pop, .explore_mod = explore, .bf_pass = bf_pass, .bf_impass = bf_impass };
        }
    }

    /// bf_object.var (Prototype.cpp:1579).
    fn loadBfObjects(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/bf_object.var"));
        self.bf_objects = try l.a.alloc(BfObject, try strings.quantity(&r, .quantity_plus_one));
        for (self.bf_objects) |*x| {
            const name = r.fieldText();
            const move = r.field();
            const stam = r.field();
            const counter = r.field();
            const def = r.field();
            const rdef = r.field();
            const range = r.field();
            x.* = .{ .name = name, .move_cost = move, .stamina_cost = stam, .counter_attack = counter, .defence = def, .ranged_defence = rdef, .shooting_range = range };
        }
    }

    /// resource.var + text/resource.txt (Prototype.cpp:1623).
    fn loadResources(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/resource.var"));
        self.resources = try l.a.alloc(Resource, try strings.quantity(&r, .quantity_plus_one));
        for (self.resources) |*x| {
            const name = r.fieldText();
            const rarity = r.field();
            const gold = r.field();
            const gem = r.field();
            x.* = .{ .name = name, .rarity = rarity, .gold = gold, .gem = gem };
        }
        r = Reader.init(try l.read("text/resource.txt"));
        for (self.resources) |*x| x.text = r.entry();
    }

    /// race.var + text/races.txt (Prototype.cpp:1665).
    fn loadRaces(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/race.var"));
        const n = try strings.quantity(&r, .quantity_plus_one);
        if (n > max_race + 1) return error.TooManyRaces;
        self.races = try l.a.alloc(Race, n);
        for (self.races) |*x| {
            const name = r.fieldText();
            const prov = r.field();
            const gold = r.field();
            const gem = r.field();
            const karma = r.field();
            const corrupt = r.field();
            const grow = r.field();
            const terra = r.field();
            const unrest = r.field();
            x.* = .{ .name = name, .prov_type = prov, .gold = gold, .gem = gem, .karma = karma, .corrupt = corrupt, .grow = grow, .terra = terra, .unrest = unrest };
            _ = r.word();
            x.n_quests = try r.list(&x.quests);
            for (&x.terrain) |*v| v.* = r.field();
        }
        r = Reader.init(try l.read("text/races.txt"));
        for (self.races) |*x| x.text = r.entry();
    }

    /// unit_upg.var + text/upgrade.txt (Prototype.cpp:1744).
    fn loadUnitUpgrades(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/unit_upg.var"));
        self.unit_upgrades = try l.a.alloc(UnitUpgrade, try strings.quantity(&r, .quantity_plus_one));
        for (self.unit_upgrades) |*x| {
            const name = r.fieldText();
            const once = r.field();
            x.* = .{ .name = name, .only_once = once != 0 };
            _ = r.word();
            _ = try r.list(&x.need);
            // "Upg Type: a\nQuantity: b" pairs until a quantity ends on ';'.
            while (true) {
                if (x.n_upg == max_upgrade_size) return error.TooManyUpgrades;
                const kind = r.field();
                const qty = r.field();
                x.upg[x.n_upg] = .{ .id = kind, .value = qty };
                x.n_upg += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }
        r = Reader.init(try l.read("text/upgrade.txt"));
        for (self.unit_upgrades) |*x| x.text = r.entry();
    }

    /// unit.var + text/unit.txt (Prototype.cpp:1867).  71 fields per record.
    fn loadUnits(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/unit.var"));
        self.units = try l.a.alloc(UnitType, try strings.quantity(&r, .quantity_plus_one));
        for (self.units) |*x| {
            x.* = .{
                .name = r.fieldText(),
                .level = r.field(),
                .life = r.field(),
                .attack = r.field(),
                .counter_attack = r.field(),
                .defence = r.field(),
                .ranged_defence = r.field(),
                .resist = r.field(),
                .speed = r.field(),
                .ranged_attack = r.field(),
                .shooting_range = r.field(),
                .ammo = r.field(),
                .stamina = r.field(),
                .morale = r.field(),
                .exp = r.field(),
                .exp_mod = r.field(),
                .gold_price = r.field(),
                .gem_price = r.field(),
                .gold_payment = r.field(),
                .gem_payment = r.field(),
                .race = r.field(),
                .unit_kind = r.field(),
                .unit_class = r.field(),
                .karma = r.field(),
                .sound_hit = r.field(),
                .sound_shoot = r.field(),
                .sound_shoot_hit = r.field(),
                .sound_death = r.field(),
                .missile = r.field(),
            };
            _ = r.word();
            _ = try r.list(&x.res);
            // "Abilityes:" then "Key: n" entries until one ends on ';'.
            _ = r.word();
            var n: usize = 0;
            while (true) {
                if (n == max_unit_abil) return error.TooManyAbilities;
                x.abilities[n] = r.field();
                n += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
            for (&x.upgrades) |*lvl| {
                _ = r.word();
                _ = try r.pairs(lvl);
            }
            for (&x.loot) |*lvl| {
                _ = r.word();
                _ = try r.pairs(lvl);
            }
        }
        r = Reader.init(try l.read("text/unit.txt"));
        for (self.units) |*x| x.text = r.entry();
    }

    /// spell.var + text/spell.txt (Prototype.cpp:2062).
    fn loadSpells(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/spell.var"));
        self.spells = try l.a.alloc(Spell, try strings.quantity(&r, .quantity_plus_one));
        for (self.spells) |*x| {
            x.* = .{
                .name = r.fieldText(),
                .cost = r.field(),
                .life_cost = r.field(),
                .stamina_cost = r.field(),
                .item_level = r.field(),
                .level = r.field(),
                .power_mod = r.field(),
                .duration_mod = r.field(),
                .target = r.field(),
                .area = r.field(),
                .radius = r.field(),
                .karma = r.field(),
                .building = r.field(),
                .negative = r.field() != 0,
                .on_enemy = r.field() != 0,
                .on_ally = r.field() != 0,
                .sacrifice = r.field() != 0,
                .resist_power = false,
                .resist_duration = false,
                .defence_power = false,
                .restore_only = false,
                .cumulative = false,
                .sound = 0,
            };
            const rp = r.field();
            if (rp > 0) {
                x.resist_power = true;
                x.resist_mod = rp;
            }
            const rd = r.field();
            if (rd > 0) {
                x.resist_duration = true;
                x.resist_mod = rd;
            }
            x.defence_power = r.field() != 0;
            x.restore_only = r.field() != 0;
            x.cumulative = r.field() != 0;
            x.sound = r.field();
            _ = r.word();
            _ = try r.list(&x.immune_kinds);
            _ = r.word();
            _ = try r.list(&x.anti_effects);
            // "Effects:" then (id, power, duration) triples until a duration
            // ends on ';'.
            _ = r.word();
            while (true) {
                if (x.n_effects == max_spell_effect) return error.TooManySpellEffects;
                const id = r.field();
                const power = r.field();
                const duration = r.field();
                x.effects[x.n_effects] = .{ .id = id, .power = power, .duration = duration };
                x.n_effects += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }
        r = Reader.init(try l.read("text/spell.txt"));
        for (self.spells) |*x| x.text = r.entry();
    }

    /// stat.var, stat_progress.var (Prototype.cpp:2227).
    fn loadStats(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/stat.var"));
        for (&self.stats) |*stat| for (stat) |*lvl| {
            _ = r.word();
            lvl[0] = r.int();
            _ = r.char();
            lvl[1] = r.int();
            _ = r.char();
            lvl[2] = r.int();
        };
        r = Reader.init(try l.read("var/stat_progress.var"));
        for (0..progress_classes) |c| for (0..stat_count) |s| {
            self.stat_progress[s][c] = r.field();
        };
    }

    /// skill.var + text/skill.txt (Prototype.cpp:2257).
    fn loadSkills(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/skill.var"));
        self.skills = try l.a.alloc(Skill, max_hero_skill + 1);
        for (self.skills) |*x| {
            x.* = .{ .name = r.fieldText(), .class = r.field() };
            for ([_]*[skill_levels][max_skill_upg]Pair{ &x.hero, &x.squad }) |part| {
                _ = r.word(); // "Hero Upgrades:" / "Squad Upgrades:"
                for (part) |*lvl| {
                    _ = r.word();
                    _ = try r.pairs(lvl);
                }
            }
        }
        r = Reader.init(try l.read("text/skill.txt"));
        for (self.skills) |*x| x.text = r.entry();
    }

    /// guard.var (Prototype.cpp:2320).
    fn loadGuards(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/guard.var"));
        self.guards = try l.a.alloc(Guard, try strings.quantity(&r, .quantity_plus_one));
        for (self.guards) |*x| {
            // "Quantity: 4/2": ReadInt stops on '/' and eats it.
            _ = r.word();
            const num = r.int();
            const num_add = r.int();
            _ = r.word();
            const quick = r.int();
            const quick_add = r.int();
            x.* = .{ .num = num, .num_add = num_add, .quick = quick, .quick_add = quick_add };
            // "UnitN: a, b, c, d" until one ends on ';'.
            while (true) {
                if (x.n_units == max_guard_unit) return error.TooManyGuardUnits;
                _ = r.word();
                const unit = r.int();
                _ = r.char();
                const level = r.int();
                _ = r.char();
                const min = r.int();
                _ = r.char();
                const poss = r.int();
                x.units[x.n_units] = .{ .unit = unit, .level = level, .min_count = min, .possibility = poss };
                x.n_units += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
            x.loot_poss = r.field();
            x.loot_num = r.field();
            x.power = r.field();
            while (true) {
                if (x.n_loot == max_guard_loot) return error.TooManyGuardLoot;
                _ = r.word();
                const item = r.int();
                _ = r.char();
                const poss = r.int();
                x.loot[x.n_loot] = .{ .id = item, .value = poss };
                x.n_loot += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }
    }

    /// guard_type.var (Prototype.cpp:2388).
    fn loadGuardTypes(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/guard_type.var"));
        self.guard_types = try l.a.alloc(GuardType, try strings.quantity(&r, .quantity_plus_one));
        for (self.guard_types) |*x| {
            const name = r.fieldText();
            const level = r.field();
            const karma = r.field();
            var g: [max_guard_level]i32 = undefined;
            for (&g) |*v| v.* = r.field();
            x.* = .{ .name = name, .level = level, .karma = karma, .guards = g };
        }
    }

    /// province_type.var (Prototype.cpp:2414).
    fn loadProvinceTypes(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/province_type.var"));
        self.province_types = try l.a.alloc(ProvinceType, try strings.quantity(&r, .quantity_plus_one));
        for (self.province_types) |*x| {
            x.* = .{
                .name = r.fieldText(),
                .race = r.field(),
                .coast = r.field(),
                .sites = r.field(),
                .gold = r.field(),
                .gem = r.field(),
                .difficult = r.field(),
                .population = r.field(),
                .level_pop = r.field(),
                .dipl_unrest = r.field(),
                .conq_unrest = r.field(),
                .dipl_karma = r.field(),
                .conq_karma = r.field(),
                .explored = r.field(),
                .level_explored = r.field(),
            };
            // The "Terrain_Possibility" line has no ':' and is swallowed by
            // the next ReadWord together with "Plains".
            for (&x.terrain_file) |*v| v.* = r.field();
            // "Guards:" then (Guard, Possibility, Dialog, Encounter) until ';'.
            _ = r.word();
            while (true) {
                if (x.n_guards == max_province_guard) return error.TooManyProvinceGuards;
                const gt = r.field();
                const poss = r.field();
                const dialog = r.field();
                const enc = r.field();
                x.guards[x.n_guards] = .{ .guard_type = gt, .possibility = poss, .dialog = dialog, .encounter = enc };
                x.n_guards += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
            _ = r.word();
            while (true) {
                if (x.n_events == max_province_event) return error.TooManyProvinceEvents;
                const ev = r.field();
                const poss = r.field();
                x.events[x.n_events] = .{ .id = ev, .value = poss };
                x.n_events += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }
    }

    /// province_level.var (Prototype.cpp:2542): MAX_PROVINCE_LEVEL+1
    /// records, no Quantity header.
    fn loadProvinceLevels(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/province_level.var"));
        self.province_levels = try l.a.alloc(ProvinceLevel, max_province_level + 1);
        for (self.province_levels) |*x| {
            const name = r.fieldText();
            const pop = r.field();
            const income = r.field();
            const explored = r.field();
            x.* = .{ .name = name, .max_population = pop, .income = income, .explored = explored };
        }
    }

    /// province_name.var (Prototype.cpp:2560).
    fn loadProvinceNames(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/province_name.var"));
        self.province_names = try l.a.alloc(ProvinceName, try strings.quantity(&r, .quantity_plus_one));
        for (self.province_names) |*x| {
            const name = r.fieldText();
            const rarity = r.field();
            const coast = r.field();
            x.* = .{ .name = name, .rarity = rarity, .coast = coast };
            _ = r.word();
            x.n_terrains = try r.list(&x.terrains);
            _ = r.word();
            x.n_types = try r.list(&x.types);
        }
    }

    /// outer_build.var + text/outer_build.txt (Prototype.cpp:2608).
    fn loadOuterBuildings(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/outer_build.var"));
        self.outer_buildings = try l.a.alloc(OuterBuilding, try strings.quantity(&r, .quantity_plus_one));
        self.max_bld_item = 0;
        for (self.outer_buildings) |*x| {
            x.* = .{
                .name = r.fieldText(),
                .upgrade = r.field(),
                .building = r.field(),
                .gold_cost = r.field(),
                .gem_cost = r.field(),
                .gold_add = r.field(),
                .gem_add = r.field(),
                .gold_perc = r.field(),
                .gem_perc = r.field(),
                .unrest = r.field(),
                .attitude = r.field(),
                .grow = r.field(),
                .karma = r.field(),
                .heal = r.field(),
                .coastal = r.field(),
                .port = r.field(),
                .wall = r.field(),
                .tower_health = r.field(),
                .tower_shoot = r.field(),
                .tower_type = r.field(),
                .library = r.field() != 0,
                .shop = r.field() != 0,
            };
            _ = r.word(); // "Terrain:"
            for (&x.terrain) |*v| v.* = r.field() != 0;
            _ = r.word(); // "Resource: (..)"
            _ = try r.list(&x.res);
            // "ResourceBonus:" then (Resource, Bonus) until ';'.
            _ = r.word();
            while (true) {
                if (x.n_res_bonus == max_resource_bonus) return error.TooManyResourceBonuses;
                const res = r.field();
                const bonus = r.field();
                x.res_bonus[x.n_res_bonus] = .{ .id = res, .value = bonus };
                x.n_res_bonus += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
            // "Ability:" then (Ability, Param1, Param2) until ';'.
            _ = r.word();
            while (true) {
                if (x.n_abilities == max_building_ability) return error.TooManyBuildingAbilities;
                const id = r.field();
                if (id == 8) self.max_bld_item += 1;
                const p1 = r.field();
                const p2 = r.field();
                x.abilities[x.n_abilities] = .{ .id = id, .p1 = p1, .p2 = p2 };
                x.n_abilities += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }
        r = Reader.init(try l.read("text/outer_build.txt"));
        for (self.outer_buildings) |*x| x.text = r.entry();
    }

    /// defender.var + text/defender.txt (Prototype.cpp:2806).
    fn loadDefenders(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/defender.var"));
        self.defenders = try l.a.alloc(Defender, try strings.quantity(&r, .quantity_plus_one));
        self.max_def_item = 0;
        for (self.defenders) |*x| {
            x.* = .{
                .name = r.fieldText(),
                .picture = r.field(),
                .power = r.field(),
                .gold_cost = r.field(),
                .gem_cost = r.field(),
                .gold_payment = r.field(),
                .gem_payment = r.field(),
                .building = r.field(),
                .initiative = r.field(),
                .karma = r.field(),
                .loot_poss = r.field(),
                .loot_num = r.field(),
                .no_dismiss = r.field() != 0,
            };
            _ = r.word(); // "Terrain:"
            for (&x.terrain) |*v| v.* = r.field() != 0;
            _ = r.word(); // "Resource: (..)"
            _ = try r.list(&x.res);
            // "Units:" then "UnitN: unit, level, count" until ';'.
            _ = r.word();
            while (true) {
                if (x.n_units == max_guard_unit) return error.TooManyDefenderUnits;
                _ = r.word();
                const unit = r.int();
                _ = r.char();
                const level = r.int();
                _ = r.char();
                const count = r.int();
                x.units[x.n_units] = .{ .unit = unit, .level = level, .count = count };
                x.n_units += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
            // "Loot:" then "ItemN: item, possibility" until ';'.
            _ = r.word();
            while (true) {
                if (x.n_loot == max_guard_loot) return error.TooManyDefenderLoot;
                _ = r.word();
                const item = r.int();
                _ = r.char();
                const poss = r.int();
                x.loot[x.n_loot] = .{ .id = item, .value = poss };
                x.n_loot += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
            // "Ability:" then (Ability, Param1, Param2) until ';'.
            _ = r.word();
            while (true) {
                if (x.n_abilities == max_defender_ability) return error.TooManyDefenderAbilities;
                const id = r.field();
                if (id == 10) self.max_def_item += 1;
                const p1 = r.field();
                const p2 = r.field();
                x.abilities[x.n_abilities] = .{ .id = id, .p1 = p1, .p2 = p2 };
                x.n_abilities += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }
        r = Reader.init(try l.read("text/defender.txt"));
        for (self.defenders) |*x| x.text = r.entry();
    }

    /// site.var + text/site.txt + text/site1.txt (Prototype.cpp:2971).
    fn loadSites(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/site.var"));
        self.sites = try l.a.alloc(Site, try strings.quantity(&r, .quantity_plus_one));
        for (self.sites) |*x| {
            x.* = .{
                .name = r.fieldText(),
                .min_level = r.field(),
                .max_level = r.field(),
                .guarded = r.field(),
                .place = r.field(),
                .event = r.field(),
                .dialog = r.field(),
                .dialog2 = r.field(),
                .treasure = r.field(),
                .coastal = r.field(),
                .work_hide = r.field() != 0,
                .work_guard = r.field() != 0,
                .work_free = r.field() != 0,
                .can_enter = r.field() != 0,
                .self_destroy = r.field() != 0,
            };
            _ = r.word(); // "Terrain:"
            for (&x.terrain) |*v| v.* = r.field();
            // "Guards:" then (Guard, Poss) until ';'.
            _ = r.word();
            while (true) {
                if (x.n_guards == max_site_guard) return error.TooManySiteGuards;
                const g = r.field();
                const poss = r.field();
                x.guards[x.n_guards] = .{ .id = g, .value = poss };
                x.n_guards += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
            // "Ability:" then (Ability, Param1, Param2) until ';'.
            _ = r.word();
            while (true) {
                if (x.n_abilities == max_site_ability) return error.TooManySiteAbilities;
                const id = r.field();
                const p1 = r.field();
                const p2 = r.field();
                x.abilities[x.n_abilities] = .{ .id = id, .p1 = p1, .p2 = p2 };
                x.n_abilities += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }
        r = Reader.init(try l.read("text/site.txt"));
        for (self.sites) |*x| x.text = r.entry();
        r = Reader.init(try l.read("text/site1.txt"));
        for (self.sites) |*x| x.free_text = r.entry();
    }

    /// ritual.var + text/ritual.txt (Prototype.cpp:3111).
    fn loadRituals(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/ritual.var"));
        self.rituals = try l.a.alloc(Ritual, try strings.quantity(&r, .quantity_plus_one));
        for (self.rituals) |*x| {
            x.* = .{
                .name = r.fieldText(),
                .gold_cost = r.field(),
                .gem_cost = r.field(),
                .sound = r.field(),
                .karma = r.field(),
                .on_ally = r.field() != 0,
                .on_enemy = r.field() != 0,
                .time = r.field(),
                .cooldown = r.field(),
                .event = r.field(),
                .item_level = r.field(),
                .item_rarity = r.field(),
            };
            _ = r.word(); // "Resource: (..)"
            _ = try r.list(&x.res);
            // "Effects:" then (Effect, Param1, Param2) until ';'.
            _ = r.word();
            while (true) {
                if (x.n_effects == max_ritual_effect) return error.TooManyRitualEffects;
                const id = r.field();
                const p1 = r.field();
                const p2 = r.field();
                x.effects[x.n_effects] = .{ .id = id, .p1 = p1, .p2 = p2 };
                x.n_effects += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }
        r = Reader.init(try l.read("text/ritual.txt"));
        for (self.rituals) |*x| x.text = r.entry();
    }

    /// item.var plus the generated scrolls, contracts and blueprints
    /// (Prototype.cpp:3228-3428).  Needs spells, defenders, outer buildings
    /// and rituals.
    fn loadItems(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/item.var"));
        const pure = try strings.quantity(&r, .quantity_plus_one);
        self.max_pure_item = pure - 1;
        // iMaxItem = iMaxPureItem + iMaxSpell + iMaxDefItem + iMaxBldItem + iMaxRitual.
        const total = pure + (self.spells.len - 1) + @as(usize, @intCast(self.max_def_item)) +
            @as(usize, @intCast(self.max_bld_item)) + (self.rituals.len - 1);
        self.items = try l.a.alloc(Item, total);
        for (self.items) |*x| x.* = .{};

        for (self.items[0..pure]) |*x| {
            x.name = r.fieldText();
            x.slot = r.field();
            x.class = r.field();
            x.durability = r.field();
            x.gold_cost = r.field();
            x.gem_cost = r.field();
            x.shop_level = r.field();
            x.rarity = r.field();
            x.building = r.field();
            x.pic = .{ .sheet = .items, .index = @intCast(@max(r.field(), 0)) };
            _ = r.word(); // "Resourse: (..)"
            _ = try r.list(&x.res);
            // "Effects:" then (Effect, Power, Area) until ';'.
            _ = r.word();
            while (true) {
                if (x.n_effects == max_item_effect) return error.TooManyItemEffects;
                const id = r.field();
                const power = r.field();
                const area = r.field();
                x.effects[x.n_effects] = .{ .id = id, .p1 = power, .p2 = area };
                x.n_effects += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }

        var k = pure;
        const quoted = struct {
            fn f(a: std.mem.Allocator, name: []const u8) ![]const u8 {
                return std.fmt.allocPrint(a, "\"{s}\"", .{name});
            }
        }.f;

        // Spell scrolls.
        for (self.spells[1..], 1..) |sp, i| {
            const x = &self.items[k];
            k += 1;
            var gold: i32 = 10;
            var lvl: i32 = 1;
            while (lvl <= sp.level) : (lvl += 1) gold *= 3;
            gold += sp.item_level * 20;
            x.* = .{
                .name = try quoted(l.a, sp.name),
                .slot = 0,
                .class = item_class_scroll,
                .durability = 1,
                .gold_cost = gold,
                .gem_cost = @divTrunc(gold, 5),
                .shop_level = sp.item_level,
                .rarity = @divTrunc(sp.level, 2) + 1,
                .building = 0,
            };
            switch (sp.level) {
                0, 1 => x.pic = .{ .sheet = .items, .index = 75 },
                2 => {
                    x.pic = .{ .sheet = .items, .index = 76 };
                    x.res[0] = 4;
                },
                3 => {
                    x.pic = .{ .sheet = .items, .index = 74 };
                    x.res[0] = 5;
                },
                4 => {
                    x.pic = .{ .sheet = .items, .index = 77 };
                    x.res[0] = if (i > 60 and i < 70) 9 else 8;
                },
                else => {},
            }
            x.effects[0] = .{ .id = 83, .p1 = @intCast(i), .p2 = 0 };
            x.n_effects = 1;
        }

        // Guard contracts: defenders with ability 10 (the last one wins).
        for (self.defenders[1..], 1..) |d, i| {
            var abl: ?Ability3 = null;
            for (d.abilities) |a| if (a.id == 10) {
                abl = a;
            };
            const a = abl orelse continue;
            if (k == self.items.len) return error.TooManyGeneratedItems;
            self.items[k] = .{
                .name = try quoted(l.a, d.name),
                .class = item_class_contract,
                .durability = 1,
                .shop_level = a.p1,
                .rarity = a.p2,
                .pic = if (Defender.picIndex(i)) |p| .{ .sheet = .defenders, .index = p } else null,
            };
            self.items[k].effects[0] = .{ .id = 86, .p1 = @intCast(i), .p2 = 0 };
            self.items[k].n_effects = 1;
            k += 1;
        }

        // Blueprints: outer buildings with ability 8.
        for (self.outer_buildings[1..], 1..) |b, i| {
            var abl: ?Ability3 = null;
            for (b.abilities) |a| if (a.id == 8) {
                abl = a;
            };
            const a = abl orelse continue;
            if (k == self.items.len) return error.TooManyGeneratedItems;
            self.items[k] = .{
                .name = try quoted(l.a, b.name),
                .class = item_class_blueprint,
                .durability = 1,
                .shop_level = a.p1,
                .rarity = a.p2,
                .pic = if (OuterBuilding.iconIndex(i)) |p| .{ .sheet = .outbuild, .index = p } else null,
            };
            self.items[k].effects[0] = .{ .id = 87, .p1 = @intCast(i), .p2 = 0 };
            self.items[k].n_effects = 1;
            k += 1;
        }

        // Ritual scrolls.
        for (self.rituals[1..], 1..) |rt, i| {
            if (k == self.items.len) return error.TooManyGeneratedItems;
            self.items[k] = .{
                .name = try quoted(l.a, rt.name),
                .class = item_class_ritual,
                .durability = 1,
                .shop_level = rt.item_level,
                .rarity = rt.item_rarity,
                .pic = if (Ritual.picIndex(i)) |p| .{ .sheet = .ritual, .index = p } else null,
            };
            self.items[k].effects[0] = .{ .id = 94, .p1 = @intCast(i), .p2 = 0 };
            self.items[k].n_effects = 1;
            k += 1;
        }
        // The C++ array is sized by ability counts, filled per building; a
        // building with two such abilities would leave a gap.
        if (k != self.items.len) return error.GeneratedItemCountMismatch;
    }

    /// dialog.var / campaign_dialog.var (Prototype.cpp:3487, 3526).  Window
    /// 3 (and 5 in the campaign) shows an avatar, the others an illustration.
    fn loadDialogs(l: *Loader, rel: []const u8, campaign: bool) ![]Dialog {
        var r = Reader.init(try l.read(rel));
        const out = try l.a.alloc(Dialog, try strings.quantity(&r, .quantity_plus_one));
        for (out) |*x| {
            const window = r.field();
            const pic: usize = @intCast(@max(r.field(), 0));
            const avatar = window == 3 or (campaign and window == 5);
            r.skipText();
            x.* = .{
                .window = window,
                .pic = if (avatar) .{ .avatar = pic } else .{ .illustration = pic },
                .text = r.text(),
            };
            // "AnswerN: text;" entries; after each one CurrentFile.get reads
            // one character, '#' ends the list ("...;#####").
            while (true) {
                if (x.n_answers == max_answer) return error.TooManyAnswers;
                x.answers[x.n_answers] = r.fieldText();
                x.n_answers += 1;
                if ((r.char() orelse return error.UnexpectedEof) == '#') break;
            }
        }
        return out;
    }

    /// text/journal.txt (Prototype.cpp:3565).
    fn loadJournal(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("text/journal.txt"));
        self.journal = try l.a.alloc(JournalEntry, try strings.quantity(&r, .quantity_plus_one));
        for (self.journal) |*x| {
            const name = r.fieldText();
            x.* = .{ .name = name, .text = r.entry() };
        }
    }

    /// question.var (Prototype.cpp:3584).
    fn loadQuestions(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/question.var"));
        self.questions = try l.a.alloc(Question, try strings.quantity(&r, .quantity_plus_one));
        for (self.questions) |*x| {
            const quester = r.field();
            const trigger = r.field();
            const event = r.field();
            x.* = .{ .quester = quester, .trigger = trigger, .event = event, .text = r.entry() };
        }
    }

    /// inner_build.var + text/inner_build.txt (Prototype.cpp:3617).
    fn loadInnerBuildings(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/inner_build.var"));
        const n = try strings.quantity(&r, .quantity_plus_one);
        if (n > max_bld + 1) return error.TooManyInnerBuildings;
        self.inner_buildings = try l.a.alloc(InnerBuilding, n);
        for (self.inner_buildings) |*x| {
            x.* = .{
                .name = r.fieldText(),
                .gold_cost = r.field(),
                .gem_cost = r.field(),
                .group = r.field(),
                .slot = r.field(),
                .upgrade = r.field(),
                .hidden = r.field(),
                .level = r.field(),
            };
            _ = r.word(); // "Buildings: (..)"
            _ = try r.list(&x.needs);
            _ = r.word(); // "Resource: (..)"
            _ = try r.list(&x.res);
            // "*Ability*:" then (Ability, Param1, Param2) until ';'.
            _ = r.word();
            while (true) {
                if (x.n_abilities == max_building_ability) return error.TooManyBuildingAbilities;
                const id = r.field();
                const p1 = r.field();
                const p2 = r.field();
                x.abilities[x.n_abilities] = .{ .id = id, .p1 = p1, .p2 = p2 };
                x.n_abilities += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }
        r = Reader.init(try l.read("text/inner_build.txt"));
        for (self.inner_buildings) |*x| x.text = r.entry();
    }

    /// build_group.var (Prototype.cpp:3726).
    fn loadBuildGroups(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/build_group.var"));
        const n = try strings.quantity(&r, .quantity_plus_one);
        if (n > max_bld_group) return error.TooManyBuildGroups;
        self.build_groups = try l.a.alloc(BuildGroup, n);
        for (self.build_groups) |*x| {
            const max = r.field();
            x.* = .{ .max = max, .name = r.fieldText() };
        }
    }

    /// mercenary.var (Prototype.cpp:3740): "LvlN: unit, poss; ..." with
    /// MAX_MERCENARY pairs per level.
    fn loadMercenaries(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/mercenary.var"));
        for (&self.mercenaries) |*lvl| {
            _ = r.word();
            for (lvl) |*m| {
                const unit = r.int();
                _ = r.char();
                const poss = r.int();
                _ = r.char();
                m.* = .{ .id = unit, .value = poss };
            }
        }
    }

    /// players.var (Prototype.cpp:3757).
    fn loadPlayerProfiles(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/players.var"));
        self.player_profiles = try l.a.alloc(PlayerProfile, try strings.quantity(&r, .quantity_plus_one));
        for (self.player_profiles) |*x| {
            x.* = .{
                .name = r.fieldText(),
                .land = r.fieldText(),
                .karma = r.field(),
                .strategy = r.field(),
                .magic = r.field(),
                .targeting = r.field(),
                .diplomacy = r.field(),
                .aggro = r.field(),
                .greed = r.field(),
                .loyal = r.field(),
                .random = r.field(),
                .sign = r.field(),
                .color1 = r.field(),
                .color2 = r.field(),
                .y = r.field(),
                .event_first_meet = r.field(),
                .event_meet = r.field(),
                .event_defeat = r.field(),
                .event_attack = r.field(),
            };
            // "LandType: a, b" / "ProvType: a, b".
            x.land_type = r.field();
            _ = r.char();
            x.land_poss = r.int();
            x.prov_type = r.field();
            _ = r.char();
            x.prov_poss = r.int();
            // The "*Attitude*" line has no ':' and goes with the first name.
            for (x.attitude[1..]) |*v| v.* = r.field();
            _ = r.word();
            _ = try r.list(&x.buildings);
            _ = r.word();
            x.n_enter_dialogs = try r.list(&x.enter_dialogs);
            _ = r.word();
            x.n_exit_dialogs = try r.list(&x.exit_dialogs);
        }
    }

    /// text/diplomacy/player<i>.txt for every profile (Prototype.cpp:3864).
    fn loadDiplomacy(self: *Proto, l: *Loader) !void {
        for (0..self.player_profiles.len) |i| {
            var pb: [64]u8 = undefined;
            const path = try std.fmt.bufPrint(&pb, "text/diplomacy/player{d}.txt", .{i});
            var r = Reader.init(try l.read(path));
            var list: std.ArrayList(DipText) = .empty;
            // `while (cSymbol != '&')` with cSymbol = ' ' at the start.
            r.sym = ' ';
            while (r.sym != '&') {
                if (r.eof()) return error.UnexpectedEof;
                const index = r.field();
                const min = r.field();
                const max = r.field();
                const name = r.field();
                const param = r.field();
                try list.append(l.a, .{ .index = index, .min = min, .max = max, .name = name != 0, .param = param, .text = r.entry() });
                // Three ReadInt calls step over "\n\n/" or stop on '&'.
                _ = r.int();
                _ = r.int();
                _ = r.int();
            }
            std.mem.reverse(DipText, list.items);
            if (i == 0) self.human_dip_texts = list.items else self.player_profiles[i].dip_texts = list.items;
        }
    }

    /// "Quantity: N" + N+1 "#...#" entries (message, tutorial, hint).
    fn loadTexts(l: *Loader, rel: []const u8) ![]const []const u8 {
        var r = Reader.init(try l.read(rel));
        const out = try l.a.alloc([]const u8, try strings.quantity(&r, .quantity_plus_one));
        for (out) |*x| x.* = r.entry();
        return out;
    }

    /// heroprice.var (Prototype.cpp:3948).
    fn loadHeroPrices(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/heroprice.var"));
        self.hero_price_mult = r.field();
        for ([_][]Pair{ &self.hero_price, &self.hero_resurrect }) |arr| for (arr) |*x| {
            const gold = r.field();
            _ = r.char();
            x.* = .{ .id = gold, .value = r.int() };
        };
    }

    /// campdiff.var (Prototype.cpp:3971).  The code reads five numbers per
    /// line and GOG lines have four: the last `get` eats the first letter
    /// of the next key and the fifth value is 0, as in the game.
    fn loadCampDiff(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/campdiff.var"));
        for (&self.camp_diff) |*row| {
            _ = r.word();
            for (row) |*v| {
                v.* = r.int();
                _ = r.char();
            }
        }
    }

    /// item_set.var (Prototype.cpp:3986).
    fn loadItemSets(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/item_set.var"));
        self.item_sets = try l.a.alloc(ItemSet, try strings.quantity(&r, .quantity_plus_one));
        for (self.item_sets) |*x| {
            x.* = .{ .name = r.fieldText() };
            const size = r.field();
            if (size < 0 or size > max_set_items) return error.TooManySetItems;
            x.n_items = @intCast(size);
            // "Items: a, b, c": ReadInt + get per item; the last get eats
            // the first letter of "Bonus".
            _ = r.word();
            for (x.items[0..x.n_items]) |*v| {
                v.* = r.int();
                _ = r.char();
            }
            const nb = r.field();
            if (nb < 0 or nb > max_set_bonus) return error.TooManySetBonuses;
            x.n_bonuses = @intCast(nb);
            for (x.bonuses[0..x.n_bonuses]) |*b| {
                b.items_needed = r.field();
                while (true) {
                    if (b.n_abilities == max_set_ability) return error.TooManySetAbilities;
                    const id = r.field();
                    const power = r.field();
                    const area = r.field();
                    b.abilities[b.n_abilities] = .{ .id = id, .p1 = power, .p2 = area };
                    b.n_abilities += 1;
                    if (r.sym == ';') break;
                    if (r.eof()) return error.UnexpectedEof;
                }
            }
        }
    }

    /// medal.var + text/medal.txt (Prototype.cpp:4044).
    fn loadMedals(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/medal.var"));
        self.medals = try l.a.alloc(Medal, try strings.quantity(&r, .quantity_plus_one));
        for (self.medals) |*x| {
            const name = r.fieldText();
            const gold = r.field();
            const gem = r.field();
            const rarity = r.field();
            x.* = .{ .name = name, .gold_spent = gold, .gem_spent = gem, .rarity = rarity };
            _ = r.word(); // "Effects:"
            while (true) {
                if (x.n_effects == max_medal_effect) return error.TooManyMedalEffects;
                const id = r.field();
                const power = r.field();
                x.effects[x.n_effects] = .{ .id = id, .value = power };
                x.n_effects += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }
        r = Reader.init(try l.read("text/medal.txt"));
        for (self.medals) |*x| x.text = r.entry();
    }

    /// event.var / camp_event.var (Prototype.cpp:4090, 4186).
    fn loadEvents(l: *Loader, rel: []const u8) ![]Event {
        var r = Reader.init(try l.read(rel));
        const out = try l.a.alloc(Event, try strings.quantity(&r, .quantity_plus_one));
        for (out) |*x| {
            x.* = .{
                .kind = r.field(),
                .possibility = r.field(),
                .dialog = r.field(),
                .dlg_param1 = r.field(),
                .dlg_param2 = r.field(),
                .attacker = r.field(),
                .event_win = r.field(),
                .event_lose = r.field(),
                .event_draw = r.field(),
                .prov_type = r.field(),
                .karma = r.field(),
            };
            // "*Answers*:" then "AnswerN: event" until ';'.
            _ = r.word();
            while (true) {
                if (x.n_answers == max_answer) return error.TooManyEventAnswers;
                x.answers[x.n_answers] = r.field();
                x.n_answers += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
            // "*Effects*:" then (index, power, param1, param2) until ';'.
            _ = r.word();
            while (true) {
                if (x.n_effects == max_event_effect) return error.TooManyEventEffects;
                const id = r.field();
                const power = r.field();
                const p1 = r.field();
                const p2 = r.field();
                x.effects[x.n_effects] = .{ .id = id, .power = power, .p1 = p1, .p2 = p2 };
                x.n_effects += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
        }
        return out;
    }

    /// (index, power, param1, param2) records after a block key, until ';'.
    fn effects4(r: *Reader, out: []Effect4) !usize {
        _ = r.word();
        var n: usize = 0;
        while (true) {
            if (n == out.len) return error.TooManyEffects;
            const id = r.field();
            const power = r.field();
            const p1 = r.field();
            const p2 = r.field();
            out[n] = .{ .id = id, .power = power, .p1 = p1, .p2 = p2 };
            n += 1;
            if (r.sym == ';') return n;
            if (r.eof()) return error.UnexpectedEof;
        }
    }

    /// encounter.var (Prototype.cpp:4275).
    fn loadEncounters(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/encounter.var"));
        self.encounters = try l.a.alloc(Encounter, try strings.quantity(&r, .quantity_plus_one));
        for (self.encounters) |*x| {
            x.* = .{
                .kind = r.field(),
                .possibility = r.field(),
                .dialog = r.field(),
                .dlg_param1 = r.field(),
                .dlg_param2 = r.field(),
                .attacker = r.field(),
                .enc_win = r.field(),
                .enc_lose = r.field(),
                .enc_draw = r.field(),
                .karma = r.field(),
            };
            _ = r.word(); // "*Answers*:"
            while (true) {
                if (x.n_answers == max_answer) return error.TooManyEncounterAnswers;
                x.answers[x.n_answers] = r.field();
                x.n_answers += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
            x.n_effects = try effects4(&r, &x.effects);
        }
    }

    /// quest.var + text/quest.txt (Prototype.cpp:4363).
    fn loadQuests(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/quest.var"));
        self.quests = try l.a.alloc(Quest, try strings.quantity(&r, .quantity_plus_one));
        for (self.quests) |*x| {
            x.* = .{
                .name = r.fieldText(),
                .giver = r.field(),
                .kind = r.field(),
                .possibility = r.field(),
                .enc_init = r.field(),
                .enc_wait = r.field(),
                .enc_found = r.field(),
                .enc_not_found = r.field(),
                .enc_done = r.field(),
                .enc_success = r.field(),
                .enc_fail_giver = r.field(),
                .enc_fail_target = r.field(),
                .enc_fail_time = r.field(),
                .time = r.field(),
                .exp = r.field(),
            };
            // "*Objectives*:" then (Index, Param, Quantity, Target, Place) until ';'.
            _ = r.word();
            while (true) {
                if (x.n_tasks == max_quest_task) return error.TooManyQuestTasks;
                const index = r.field();
                const param = r.field();
                const qty = r.field();
                const target = r.field();
                const place = r.field();
                x.tasks[x.n_tasks] = .{ .index = index, .param = param, .quantity = qty, .target = target, .place = place };
                x.n_tasks += 1;
                if (r.sym == ';') break;
                if (r.eof()) return error.UnexpectedEof;
            }
            x.n_abilities = try effects4(&r, &x.abilities);
        }
        r = Reader.init(try l.read("text/quest.txt"));
        for (self.quests) |*x| x.text = r.entry();
    }

    /// mapobject.var (Prototype.cpp:4526).
    fn loadMapObjects(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/mapobject.var"));
        self.map_objects = try l.a.alloc(MapObject, try strings.quantity(&r, .quantity_plus_one));
        for (self.map_objects) |*x| {
            x.* = .{
                .kind = r.field(),
                .layer = r.field(),
                .dx = r.field(),
                .dy = r.field(),
                .site = r.field(),
            };
            // The "*Land_Types*:" key goes with the first ReadWord.
            _ = r.word();
            for (&x.land) |*v| v.* = r.field();
            _ = r.word();
            _ = try r.list(&x.prov_types);
            x.landmark = r.field() != 0;
            _ = r.word();
            x.n_pics = try r.list(&x.pics);
        }
    }

    /// landmark.var (Prototype.cpp:4606).
    fn loadLandmarks(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/landmark.var"));
        self.landmarks = try l.a.alloc(Landmark, try strings.quantity(&r, .quantity_plus_one));
        for (self.landmarks) |*x| {
            x.* = .{ .kind = r.field(), .layer = r.field(), .dx = r.field(), .dy = r.field() };
            _ = r.word();
            _ = try r.list(&x.land);
            _ = r.word();
            _ = try r.list(&x.prov_types);
            x.site = r.field();
            x.pic = r.field();
            x.shadow = r.field();
        }
    }

    /// castle.var (Prototype.cpp:4678).
    fn loadCastle(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/castle.var"));
        self.castle_parts = try l.a.alloc(CastlePart, try strings.quantity(&r, .quantity_plus_one));
        for (self.castle_parts) |*x| {
            x.* = .{
                .layer = r.field(),
                .opacity = r.field(),
                .x = r.field(),
                .y = r.field(),
                .group = r.field(),
                .y_place = r.field(),
            };
            if (x.group < 0 or x.group >= max_group) return error.BadCastleGroup;
            _ = r.word();
            x.n_builds = try r.list(&x.builds);
            _ = r.word();
            x.n_anti_builds = try r.list(&x.anti_builds);
        }
    }

    /// shard.var (Prototype.cpp:4766): MAX_SHARD_SIZE records, no Quantity.
    fn loadShardSizes(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/shard.var"));
        for (&self.shard_sizes) |*x| {
            const name = r.fieldText();
            const sx = r.field();
            const sy = r.field();
            x.* = .{ .name = name, .x = sx, .y = sy, .players = r.field() };
        }
    }

    /// group_coord.var (Prototype.cpp:4789).
    fn loadGroupCoords(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/group_coord.var"));
        for (&self.group_coords) |*g| {
            _ = r.word();
            _ = try r.pairs(g);
        }
    }

    /// Min / Max / Text records (Prototype.cpp:4813-4878).
    fn loadRangeTexts(l: *Loader, rel: []const u8) ![]RangeText {
        var r = Reader.init(try l.read(rel));
        const out = try l.a.alloc(RangeText, try strings.quantity(&r, .quantity_plus_one));
        for (out) |*x| {
            const min = r.field();
            const max = r.field();
            x.* = .{ .min = min, .max = max, .text = r.fieldText() };
        }
        return out;
    }

    /// credits.var (Prototype.cpp:4881): Quantity records (not +1).
    fn loadCredits(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/credits.var"));
        self.credits = try l.a.alloc(Credits, try strings.quantity(&r, .quantity));
        for (self.credits) |*x| {
            const title = r.fieldText();
            const n = r.field();
            if (n < 0 or n > 20) return error.TooManyCreditNames;
            x.* = .{ .title = title, .names = try strings.names(l.a, &r, @intCast(n)) };
        }
    }

    /// shard_bonus.var (Prototype.cpp:4902): "LevelN: (a, b)(c, d);".
    fn loadShardBonuses(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/shard_bonus.var"));
        self.shard_bonuses = try l.a.alloc(ShardBonus, try strings.quantity(&r, .quantity_plus_one));
        for (self.shard_bonuses) |*x| {
            const name = r.fieldText();
            const kind = r.field();
            const rarity = r.field();
            x.* = .{ .name = name, .kind = kind, .rarity = rarity, .price = r.field() };
            for (&x.levels, &x.n_per_level) |*lvl, *n| {
                _ = r.word();
                _ = r.char(); // '('
                while (r.sym != ';') {
                    if (n.* == max_shard_bonus_type) return error.TooManyShardBonuses;
                    const a = r.int();
                    _ = r.char();
                    const b = r.int();
                    _ = r.char(); // '(' of the next pair or ';'
                    lvl[n.*] = .{ .id = a, .value = b };
                    n.* += 1;
                    if (r.eof()) return error.UnexpectedEof;
                }
            }
        }
    }

    /// difficulty.var (Prototype.cpp:4941).
    fn loadDifficulties(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/difficulty.var"));
        for (&self.difficulties) |*x| {
            x.* = .{
                .name = r.fieldText(),
                .gold = r.field(),
                .gem = r.field(),
                .gold_add = r.field(),
                .gem_add = r.field(),
                .population = r.field(),
                .exp = r.field(),
                .loot = r.field(),
                .neutral_life = r.field(),
                .corrupt_start = r.field(),
                .corrupt_cap = r.field(),
                .show_advice = r.field() != 0,
                .score = r.field(),
            };
        }
    }

    /// terr_layer.var (Prototype.cpp:4752): four "n, " values per land type.
    fn loadTerrLayer(self: *Proto, l: *Loader) !void {
        var r = Reader.init(try l.read("var/terr_layer.var"));
        for (&self.terr_layer) |*row| {
            _ = r.word();
            for (row) |*v| {
                v.* = r.int();
                _ = r.char();
            }
        }
    }
};

/// File access for loaders: the bytes stay in the arena.
const Loader = struct {
    a: std.mem.Allocator,
    fs: *GameFs,

    fn read(self: *Loader, rel: []const u8) ![]const u8 {
        const bytes = self.fs.read(rel) catch |e| {
            std.log.err("cannot read {s}: {t}", .{ rel, e });
            return e;
        };
        defer self.fs.gpa.free(bytes);
        return self.a.dupe(u8, bytes);
    }
};

const t = std.testing;

fn openAssets(lang: []const u8) !GameFs {
    return GameFs.open(t.io, t.allocator, "zig-out/assets", lang) catch error.SkipZigTest;
}

test "prototypes from the imported GOG files (skipped without assets)" {
    for ([_][]const u8{ "ru", "en" }) |lang| {
        var fs = try openAssets(lang);
        defer fs.close();
        if (!std.mem.eql(u8, lang, fs.lang())) continue; // language not imported
        var p = try Proto.load(t.allocator, &fs);
        defer p.deinit();

        try t.expectEqual(@as(usize, 559), p.phrases.len);
        try t.expectEqual(@as(usize, max_class), p.classes.len);
        try t.expectEqualStrings(if (lang[0] == 'r') "Воин" else "Warrior", p.classes[0].name);
        // The last class must not pick up the tail of the file.
        for (p.classes) |c| try t.expect(std.mem.indexOfScalar(u8, c.name, '\n') == null);
        try t.expect(p.classes[0].text.len > 0 and p.classes[35].desc.len > 0);

        for (p.hero_names) |list| try t.expectEqual(@as(usize, 11), list.len);
        try t.expectEqual(@as(usize, max_item_slot), p.item_slots.len);
        try t.expectEqual(@as(usize, 137), p.abilities.len);
        try t.expectEqual(@as(usize, 48), p.effects.len);
        for (p.abilities, 0..) |ab, i| {
            try t.expect(ab.effect >= -1 and ab.effect <= 1);
            // text/ability.txt: "##" at 0, 31 and 62 in both releases.
            const empty = i == 0 or i == 31 or i == 62;
            try t.expectEqual(empty, ab.text.len == 0);
        }
        for (p.effects[1..]) |e| try t.expect(e.text.len > 0);

        try t.expectEqual(@as(usize, 7), p.terrains.len);
        try t.expectEqualStrings(if (lang[0] == 'r') "Тундра" else "Tundra", p.terrains[6].name);
        try t.expectEqual(@as(usize, 12), p.bf_objects.len);
        try t.expectEqual(@as(usize, 10), p.resources.len);
        for (p.resources[1..]) |x| try t.expect(x.text.len > 0 and x.rarity >= 0 and x.rarity <= 2);
        try t.expectEqual(@as(usize, 9), p.races.len);
        // race.var: Quests (0) (0) (1) (2) (6) (3) (5) (4) (51).
        const quests = [_]i32{ 0, 0, 1, 2, 6, 3, 5, 4, 51 };
        for (p.races, quests) |x, q| {
            try t.expectEqual(@as(usize, 1), x.n_quests);
            try t.expectEqual(q, x.quests[0]);
            try t.expect(std.mem.indexOfScalar(u8, x.name, '\n') == null);
        }
        for (p.races[1..]) |x| try t.expect(x.text.len > 0);

        try t.expectEqual(@as(usize, 153), p.unit_upgrades.len);
        // /40 "Броня +1": two upgrades (4, 1) and (5, 1).
        const upg40 = p.unit_upgrades[40];
        try t.expectEqual(@as(usize, 2), upg40.n_upg);
        try t.expectEqual(Pair{ .id = 4, .value = 1 }, upg40.upg[0]);
        try t.expectEqual(Pair{ .id = 5, .value = 1 }, upg40.upg[1]);
        try t.expectEqual(Pair{ .id = 1, .value = 1 }, p.unit_upgrades[1].upg[0]);

        try t.expectEqual(@as(usize, 72), p.units.len);
        const unit1 = p.units[1];
        try t.expectEqualStrings(if (lang[0] == 'r') "Ополченец" else "Militiaman", unit1.name);
        try t.expectEqual(@as(i32, 17), unit1.life);
        try t.expectEqual(@as(i32, 9), unit1.morale);
        try t.expectEqual(@as(i32, 29), unit1.sound_hit);
        try t.expectEqual(@as(i32, 30), unit1.sound_death);
        try t.expectEqual(Pair{ .id = 1, .value = 4 }, unit1.upgrades[0][0]);
        try t.expectEqual(Pair{ .id = 7, .value = 1 }, unit1.upgrades[0][3]);
        try t.expectEqual(Pair{ .id = 3, .value = 9 }, unit1.upgrades[19][0]);
        // "Lvl 09 loot: (82,2; 94, 5)": the original reads (82, 0).
        try t.expectEqual(Pair{ .id = 82, .value = 0 }, unit1.loot[8][0]);
        try t.expectEqual(Pair{ .id = 94, .value = 5 }, unit1.loot[8][1]);
        try t.expectEqual(Pair{ .id = 377, .value = 1 }, unit1.loot[12][0]);
        try t.expectEqual(@as(i32, 53), p.units[2].abilities[0]);
        for (p.units[1..]) |x| {
            try t.expect(x.level >= 1 and x.level <= 5);
            try t.expect(x.life > 0);
            try t.expect(x.text.len > 0);
        }

        try t.expectEqual(@as(usize, 78), p.spells.len);
        // /1 «Магическая искра»: StamCost 1, PowerMod 100, ResistPower 100,
        // AntiEffect (30), one effect Damage 1 / Power 4 / Duration -1.
        const s1 = p.spells[1];
        try t.expectEqual(@as(i32, 1), s1.stamina_cost);
        try t.expectEqual(@as(i32, 100), s1.power_mod);
        try t.expect(s1.resist_power and !s1.resist_duration);
        try t.expectEqual(@as(i32, 100), s1.resist_mod);
        try t.expectEqual(@as(i32, 30), s1.anti_effects[0]);
        try t.expectEqual(@as(usize, 1), s1.n_effects);
        try t.expectEqual(SpellEffect{ .id = 1, .power = 4, .duration = -1 }, s1.effects[0]);
        var max_eff: usize = 0;
        for (p.spells[1..]) |x| {
            try t.expect(x.n_effects >= 1 and x.text.len > 0);
            max_eff = @max(max_eff, x.n_effects);
        }
        try t.expectEqual(@as(usize, 8), max_eff);

        // stat.var: Health Lvl1 = 3, 1, 1; Health Lvl10 = 7, 4, 2;
        // Magic Lvl8 = 2, 2, 0.
        try t.expectEqual([3]i32{ 3, 1, 1 }, p.stats[0][0]);
        try t.expectEqual([3]i32{ 7, 4, 2 }, p.stats[0][9]);
        try t.expectEqual([3]i32{ 2, 2, 0 }, p.stats[1][7]);
        // stat_progress.var: /1 Warrior 6, 1, 3; /20 Mage 2, 4, 4.
        try t.expectEqual(@as(i32, 6), p.stat_progress[0][0]);
        try t.expectEqual(@as(i32, 3), p.stat_progress[2][0]);
        try t.expectEqual(@as(i32, 4), p.stat_progress[1][19]);
        try t.expectEqual(@as(i32, 4), p.stat_progress[2][19]);

        try t.expectEqual(@as(usize, 29), p.skills.len);
        // /1 «Обеспечение»: Class 3, Hero Lvl1 (997, 10; 55, 5), Lvl4 (997, 10; 55, 10).
        const sk1 = p.skills[1];
        try t.expectEqual(@as(i32, 3), sk1.class);
        try t.expectEqual(Pair{ .id = 997, .value = 10 }, sk1.hero[0][0]);
        try t.expectEqual(Pair{ .id = 55, .value = 5 }, sk1.hero[0][1]);
        try t.expectEqual(Pair{ .id = 55, .value = 10 }, sk1.hero[3][1]);
        try t.expectEqual(Pair{}, sk1.squad[0][0]);
        for (p.skills[1..]) |x| {
            try t.expect(x.class >= 1 and x.class <= 4);
            try t.expect(x.text.len > 0);
            try t.expect(std.mem.indexOfScalar(u8, x.name, '\n') == null);
        }

        try t.expectEqual(@as(usize, 169), p.guards.len);
        // /0: Quantity 6/1, Unit4 = 4, 5, 1, 1; Item3 = 3, 1.
        const g0 = p.guards[0];
        try t.expectEqual(@as(i32, 6), g0.num);
        try t.expectEqual(@as(i32, 1), g0.num_add);
        try t.expectEqual(@as(usize, 4), g0.n_units);
        try t.expectEqual(GuardUnit{ .unit = 4, .level = 5, .min_count = 1, .possibility = 1 }, g0.units[3]);
        try t.expectEqual(@as(usize, 3), g0.n_loot);
        try t.expectEqual(Pair{ .id = 3, .value = 1 }, g0.loot[2]);
        // /1: Quantity 4/2, LootPoss 10, MaxLoot 1, Power 1, Item2 = 17, 1.
        const g1 = p.guards[1];
        try t.expectEqual(@as(i32, 4), g1.num);
        try t.expectEqual(@as(i32, 2), g1.num_add);
        try t.expectEqual(@as(i32, 10), g1.loot_poss);
        try t.expectEqual(@as(i32, 1), g1.power);
        try t.expectEqual(Pair{ .id = 17, .value = 1 }, g1.loot[1]);
        var max_units: usize = 0;
        for (p.guards) |x| max_units = @max(max_units, x.n_units);
        try t.expectEqual(@as(usize, 11), max_units);

        try t.expectEqual(@as(usize, 52), p.guard_types.len);
        try t.expectEqual([max_guard_level]i32{ 1, 2, 3, 4 }, p.guard_types[1].guards);

        try t.expectEqual(@as(usize, 25), p.province_types.len);
        // /1: Population 300, ConqKarma -1, Plains 100 .. Desert 20,
        // Guard 1 / 1 / 3 / 1, Event 426 / 20.
        const pt1 = p.province_types[1];
        try t.expectEqual(@as(i32, 300), pt1.population);
        try t.expectEqual(@as(i32, -1), pt1.conq_karma);
        try t.expectEqual(@as(i32, 10), pt1.level_explored);
        try t.expectEqual(@as(i32, 100), pt1.terrainWeight(1));
        try t.expectEqual(@as(i32, 20), pt1.terrainWeight(5));
        try t.expectEqual(@as(i32, 20), pt1.terrain_file[5]);
        // Tundra: the first guard type, not the file's 20 (see terrainWeight).
        try t.expectEqual(@as(i32, 1), pt1.terrainWeight(6));
        try t.expectEqual(ProvinceGuard{ .guard_type = 1, .possibility = 1, .dialog = 3, .encounter = 1 }, pt1.guards[0]);
        try t.expectEqual(Pair{ .id = 426, .value = 20 }, pt1.events[0]);
        for (p.province_types) |x| try t.expect(x.n_guards >= 1 and x.n_events >= 1);

        try t.expectEqual(@as(usize, max_province_level + 1), p.province_levels.len);
        try t.expectEqual(@as(i32, -100), p.province_levels[1].income);
        try t.expect(std.mem.indexOfScalar(u8, p.province_levels[max_province_level].name, '\n') == null);

        try t.expectEqual(@as(usize, 523), p.province_names.len);
        const pn1 = p.province_names[1];
        try t.expectEqual(@as(i32, 1), pn1.rarity);
        try t.expectEqual(@as(usize, 1), pn1.n_terrains);
        try t.expectEqual(@as(i32, 1), pn1.terrains[0]);
        for (p.province_names) |x| {
            try t.expect(x.n_terrains >= 1 and x.n_types >= 1);
            try t.expect(std.mem.indexOfScalar(u8, x.name, '\n') == null);
        }

        try t.expectEqual(@as(usize, 41), p.outer_buildings.len);
        // /1 «Лесопилка»: Building 27, GoldCost 60, GoldAdd 5, Forest only,
        // Resource (2), ResourceBonus 2 / 2, Ability 0 / 0 / 0.
        const ob1 = p.outer_buildings[1];
        try t.expectEqual(@as(i32, 27), ob1.building);
        try t.expectEqual(@as(i32, 60), ob1.gold_cost);
        try t.expectEqual(@as(i32, 5), ob1.gold_add);
        try t.expectEqual([max_terrain]bool{ false, true, false, false, false, false }, ob1.terrain);
        try t.expectEqual(@as(i32, 2), ob1.res[0]);
        try t.expectEqual(@as(usize, 1), ob1.n_res_bonus);
        try t.expectEqual(Pair{ .id = 2, .value = 2 }, ob1.res_bonus[0]);
        try t.expectEqual(@as(usize, 1), ob1.n_abilities);
        try t.expectEqual(@as(?usize, 0), OuterBuilding.buildingIndex(1));
        try t.expectEqual(@as(?usize, 79), OuterBuilding.shadowIndex(40));
        try t.expectEqual(@as(i32, 11), p.max_bld_item);

        try t.expectEqual(@as(usize, 72), p.defenders.len);
        // /1: Illustration 28, Power 1, GoldCost 30, GoldPayment 3, LootPoss 5,
        // all terrains, Unit1 = 1, 5, 6; Unit3 = 3, 3, 2; Item2 = 17, 1.
        const d1 = p.defenders[1];
        try t.expectEqual(@as(i32, 28), d1.picture);
        try t.expectEqual(@as(i32, 1), d1.power);
        try t.expectEqual(@as(i32, 30), d1.gold_cost);
        try t.expectEqual(@as(i32, 3), d1.gold_payment);
        try t.expectEqual(@as(i32, 5), d1.loot_poss);
        try t.expectEqual([max_terrain]bool{ true, true, true, true, true, true }, d1.terrain);
        try t.expectEqual(@as(usize, 3), d1.n_units);
        try t.expectEqual(DefenderUnit{ .unit = 1, .level = 5, .count = 6 }, d1.units[0]);
        try t.expectEqual(DefenderUnit{ .unit = 3, .level = 3, .count = 2 }, d1.units[2]);
        try t.expectEqual(Pair{ .id = 17, .value = 1 }, d1.loot[1]);
        try t.expectEqual(@as(usize, 1), d1.n_abilities);
        try t.expectEqual(@as(i32, 22), p.max_def_item);

        try t.expectEqual(@as(usize, 66), p.sites.len);
        // /1: MinLevel 1, MaxLevel 3, Guarded 40, Event 2, Dialog 36/37,
        // Treasure -20, WorkFree 1, Hill only, Guard 6 / 1,
        // Ability (1, 1, 0) and (2, 1, 0).
        const s1site = p.sites[1];
        try t.expectEqual(@as(i32, 3), s1site.max_level);
        try t.expectEqual(@as(i32, 40), s1site.guarded);
        try t.expectEqual(@as(i32, 37), s1site.dialog2);
        try t.expectEqual(@as(i32, -20), s1site.treasure);
        try t.expect(s1site.work_free and !s1site.work_hide and !s1site.can_enter);
        try t.expectEqual([max_terrain]i32{ 0, 0, 1, 0, 0, 0 }, s1site.terrain);
        try t.expectEqual(@as(usize, 1), s1site.n_guards);
        try t.expectEqual(Pair{ .id = 6, .value = 1 }, s1site.guards[0]);
        try t.expectEqual(@as(usize, 2), s1site.n_abilities);
        try t.expectEqual(Ability3{ .id = 2, .p1 = 1, .p2 = 0 }, s1site.abilities[1]);
        var max_sg: usize = 0;
        var max_sa: usize = 0;
        for (p.sites[1..]) |x| {
            try t.expect(x.text.len > 0);
            try t.expect(std.mem.indexOfScalar(u8, x.name, '\n') == null);
            max_sg = @max(max_sg, x.n_guards);
            max_sa = @max(max_sa, x.n_abilities);
        }
        try t.expectEqual(@as(usize, 14), max_sg);
        try t.expectEqual(@as(usize, 7), max_sa);

        try t.expectEqual(@as(usize, 25), p.rituals.len);
        // /1: GemCost 15, Sound 35, OnAlly 1, Cooldown 4, ItemLevel 1,
        // effects (20, 1, 0) and (5, 502, 0).
        const r1 = p.rituals[1];
        try t.expectEqual(@as(i32, 15), r1.gem_cost);
        try t.expectEqual(@as(i32, 35), r1.sound);
        try t.expect(r1.on_ally and !r1.on_enemy);
        try t.expectEqual(@as(i32, 4), r1.cooldown);
        try t.expectEqual(@as(usize, 2), r1.n_effects);
        try t.expectEqual(Ability3{ .id = 5, .p1 = 502, .p2 = 0 }, r1.effects[1]);
        for (p.rituals[1..]) |x| {
            try t.expect(x.n_effects >= 1 and x.text.len > 0);
            try t.expect(std.mem.indexOfScalar(u8, x.name, '\n') == null);
        }

        // 407 item.var records + 77 spell scrolls + 22 contracts
        // + 11 blueprints + 24 ritual scrolls.
        try t.expectEqual(@as(usize, 406), p.max_pure_item);
        try t.expectEqual(@as(usize, 407 + 77 + 22 + 11 + 24), p.items.len);
        // /1 Нож: Slot 1, Durability 20, GoldCost 5, Building 2, Pic 317,
        // Resourse (1), effects (2,1,0) (3,1,0) (76,1,0).
        const it1 = p.items[1];
        try t.expectEqual(@as(i32, 1), it1.slot);
        try t.expectEqual(@as(i32, 20), it1.durability);
        try t.expectEqual(@as(i32, 5), it1.gold_cost);
        try t.expectEqual(@as(i32, 2), it1.building);
        try t.expectEqual(Pic{ .sheet = .items, .index = 317 }, it1.pic.?);
        try t.expectEqual(@as(i32, 1), it1.res[0]);
        try t.expectEqual(@as(usize, 3), it1.n_effects);
        try t.expectEqual(Ability3{ .id = 76, .p1 = 1, .p2 = 0 }, it1.effects[2]);
        var max_ie: usize = 0;
        for (p.items[0..407]) |x| max_ie = @max(max_ie, x.n_effects);
        try t.expectEqual(@as(usize, max_item_effect), max_ie);

        // First spell scroll: spell 1 (level 0, ItemLevel 0).
        const sc = p.items[407];
        try t.expectEqual(@as(i32, item_class_scroll), sc.class);
        try t.expectEqual(@as(i32, 10), sc.gold_cost);
        try t.expectEqual(@as(i32, 2), sc.gem_cost);
        try t.expectEqual(@as(i32, 1), sc.rarity);
        try t.expectEqual(Pic{ .sheet = .items, .index = 75 }, sc.pic.?);
        try t.expectEqual(Ability3{ .id = 83, .p1 = 1, .p2 = 0 }, sc.effects[0]);
        try t.expect(std.mem.startsWith(u8, sc.name, "\"") and std.mem.endsWith(u8, sc.name, "\""));
        try t.expectEqualStrings(p.spells[1].name, sc.name[1 .. sc.name.len - 1]);
        const classes = [_]struct { first: usize, n: usize, class: i32, effect: i32 }{
            .{ .first = 407, .n = 77, .class = item_class_scroll, .effect = 83 },
            .{ .first = 484, .n = 22, .class = item_class_contract, .effect = 86 },
            .{ .first = 506, .n = 11, .class = item_class_blueprint, .effect = 87 },
            .{ .first = 517, .n = 24, .class = item_class_ritual, .effect = 94 },
        };
        for (classes) |c| for (p.items[c.first..][0..c.n]) |x| {
            try t.expectEqual(c.class, x.class);
            try t.expectEqual(c.effect, x.effects[0].id);
            try t.expect(x.pic != null);
        };

        // Answer counts and window types: checked against a step-by-step
        // simulation of the C++ reader on both GOG releases.
        for ([_]struct { d: []const Dialog, n: usize, answers: [4]usize }{
            .{ .d = p.dialogs, .n = 1530, .answers = .{ 80, 916, 353, 181 } },
            .{ .d = p.campaign_dialogs, .n = 1347, .answers = .{ 529, 508, 235, 75 } },
        }) |c| {
            try t.expectEqual(c.n, c.d.len);
            var count: [4]usize = @splat(0);
            for (c.d) |x| {
                count[x.n_answers - 1] += 1;
                for (x.answers[0..x.n_answers]) |a|
                    try t.expect(std.mem.indexOfAny(u8, a, "\n#") == null);
            }
            try t.expectEqual(c.answers, count);
        }
        // /3 «Вольное поселение»: window 2, illustration 28, three answers
        // (attack, retreat, negotiate).
        const d3 = p.dialogs[3];
        try t.expectEqual(@as(i32, 2), d3.window);
        try t.expectEqual(DialogPic{ .illustration = 28 }, d3.pic);
        try t.expectEqual(@as(usize, 3), d3.n_answers);
        try t.expectEqualStrings(if (lang[0] == 'r') "Атаковать!" else "Attack!", d3.answers[0]);
        try t.expect(std.mem.indexOf(u8, d3.text, "%s") != null);
        var avatars: usize = 0;
        for (p.campaign_dialogs) |x| if (x.pic == .avatar) {
            avatars += 1;
        };
        try t.expectEqual(@as(usize, 1249 + 15), avatars);

        try t.expectEqual(@as(usize, 24), p.journal.len);
        for (p.journal) |x| try t.expect(x.name.len > 0 and x.text.len > 0);
        try t.expectEqual(@as(usize, 290), p.questions.len);
        // /1: Quester 0, Trigger 5, Event 54.
        try t.expectEqual(Question{ .quester = 0, .trigger = 5, .event = 54, .text = p.questions[1].text }, p.questions[1]);
        for (p.questions) |x| try t.expect(x.text.len > 0);

        try t.expectEqual(@as(usize, 212), p.inner_buildings.len);
        // /1 «Мастерские»: GoldCost 50, Slot 36, Resource (2), Ability (1, 0, 0).
        const ib1 = p.inner_buildings[1];
        try t.expectEqual(@as(i32, 50), ib1.gold_cost);
        try t.expectEqual(@as(i32, 36), ib1.slot);
        try t.expectEqual(@as(i32, 2), ib1.res[0]);
        try t.expectEqual(@as(usize, 1), ib1.n_abilities);
        try t.expectEqual(Ability3{ .id = 1, .p1 = 0, .p2 = 0 }, ib1.abilities[0]);
        var max_iba: usize = 0;
        for (p.inner_buildings[1..]) |x| {
            try t.expect(x.text.len > 0);
            try t.expect(std.mem.indexOfScalar(u8, x.name, '\n') == null);
            max_iba = @max(max_iba, x.n_abilities);
        }
        try t.expectEqual(@as(usize, max_building_ability), max_iba);

        try t.expectEqual(@as(usize, 14), p.build_groups.len);
        try t.expectEqual(@as(i32, 4), p.build_groups[1].max);
        try t.expectEqual(@as(i32, 2), p.build_groups[13].max);
        for (p.build_groups) |x| try t.expect(x.name.len > 0 and std.mem.indexOfScalar(u8, x.name, '\n') == null);

        // mercenary.var: Lvl1 starts with (13, 10), Lvl5 ends with (67, 5).
        try t.expectEqual(Pair{ .id = 13, .value = 10 }, p.mercenaries[0][0]);
        try t.expectEqual(Pair{ .id = 8, .value = 10 }, p.mercenaries[0][10]);
        try t.expectEqual(Pair{ .id = 15, .value = 5 }, p.mercenaries[1][1]);
        try t.expectEqual(Pair{ .id = 67, .value = 5 }, p.mercenaries[4][10]);

        try t.expectEqual(@as(usize, 17), p.player_profiles.len);
        // /1 Адрагерон: Karma -100, Aggro 80, Herb 11, Color2 3, Y 38,
        // EventAttack 1065, ProvType 15, 100, attitudes Doricus 50 ..
        // Ul'Dagan 50, 12 enter and 11 exit dialogs.
        const pp1 = p.player_profiles[1];
        try t.expectEqual(@as(i32, -100), pp1.karma);
        try t.expectEqual(@as(i32, 80), pp1.aggro);
        try t.expectEqual(@as(i32, 11), pp1.sign);
        try t.expectEqual(@as(i32, 3), pp1.color2);
        try t.expectEqual(@as(i32, 38), pp1.y);
        try t.expectEqual(@as(i32, 1065), pp1.event_attack);
        try t.expectEqual(@as(i32, 15), pp1.prov_type);
        try t.expectEqual(@as(i32, 100), pp1.prov_poss);
        try t.expectEqual(@as(i32, 0), pp1.attitude[0]);
        try t.expectEqual(@as(i32, 50), pp1.attitude[2]);
        try t.expectEqual(@as(i32, -100), pp1.attitude[4]);
        try t.expectEqual(@as(i32, 50), pp1.attitude[16]);
        try t.expectEqual(@as(usize, 12), pp1.n_enter_dialogs);
        try t.expectEqual(@as(i32, 702), pp1.enter_dialogs[11]);
        try t.expectEqual(@as(usize, 11), pp1.n_exit_dialogs);

        // Message counts per player file (Python simulation of the C++ loop).
        const dips = [_]usize{ 33, 35, 35, 38, 35, 35, 35, 35, 33, 35, 35, 35, 35, 35, 35, 33, 35 };
        try t.expectEqual(dips[0], p.human_dip_texts.len);
        for (p.player_profiles[1..], dips[1..]) |x, n| {
            try t.expectEqual(n, x.dip_texts.len);
            for (x.dip_texts) |d| try t.expect(d.text.len > 0);
        }
        // Reverse file order: the first message of player1.txt is last.
        const last = pp1.dip_texts[pp1.dip_texts.len - 1];
        try t.expectEqual(DipText{ .index = 1, .min = 0, .max = 0, .name = true, .param = 0, .text = last.text }, last);

        try t.expectEqual(@as(usize, 120), p.messages.len);
        try t.expectEqual(@as(usize, 119), p.tutorial.len);
        try t.expectEqual(@as(usize, 44), p.hints.len);

        try t.expectEqual(@as(i32, 10), p.hero_price_mult);
        try t.expectEqual(Pair{ .id = 5000, .value = 500 }, p.hero_price[3]);
        try t.expectEqual(Pair{ .id = 30, .value = 7 }, p.hero_resurrect[1]);
        try t.expectEqual([5]i32{ 0, 2, 12, 90, 0 }, p.camp_diff[0]);
        try t.expectEqual([5]i32{ 1, 4, 6, 55, 0 }, p.camp_diff[6]);

        try t.expectEqual(@as(usize, 18), p.item_sets.len);
        // /1: items 88, 101, 123; one bonus for 3 items: (12, 2, 1).
        const set1 = p.item_sets[1];
        try t.expectEqual(@as(usize, 3), set1.n_items);
        try t.expectEqualSlices(i32, &.{ 88, 101, 123 }, set1.items[0..3]);
        try t.expectEqual(@as(usize, 1), set1.n_bonuses);
        try t.expectEqual(@as(i32, 3), set1.bonuses[0].items_needed);
        try t.expectEqual(Ability3{ .id = 12, .p1 = 2, .p2 = 1 }, set1.bonuses[0].abilities[0]);
        var set_abil: [4]usize = @splat(0);
        for (p.item_sets[1..]) |x| for (x.bonuses[0..x.n_bonuses]) |b| {
            set_abil[b.n_abilities - 1] += 1;
        };
        // Bonus blocks by ability count (awk over item_set.var, minus /0).
        try t.expectEqual([4]usize{ 13, 15, 3, 1 }, set_abil);

        try t.expectEqual(@as(usize, 16), p.medals.len);
        // /1: GoldSpent 3, effects (12, 4) and (2, 1).
        const m1 = p.medals[1];
        try t.expectEqual(@as(i32, 3), m1.gold_spent);
        try t.expectEqual(@as(usize, 2), m1.n_effects);
        try t.expectEqual(Pair{ .id = 12, .value = 4 }, m1.effects[0]);
        try t.expectEqual(Pair{ .id = 2, .value = 1 }, m1.effects[1]);
        for (p.medals[1..]) |x| try t.expect(x.text.len > 0 and x.n_effects >= 1);

        // Totals of "AnswerN:" and "Param2:" lines in the GOG files.
        for ([_]struct { e: []const Event, n: usize, answers: usize, effects: usize }{
            .{ .e = p.events, .n = 1264, .answers = 2557, .effects = 2543 },
            .{ .e = p.campaign_events, .n = 1478, .answers = 2662, .effects = 1955 },
        }) |c| {
            try t.expectEqual(c.n, c.e.len);
            var na: usize = 0;
            var ne: usize = 0;
            for (c.e) |x| {
                na += x.n_answers;
                ne += x.n_effects;
            }
            try t.expectEqual(c.answers, na);
            try t.expectEqual(c.effects, ne);
        }
        // event /1: Type -1, Possibility 20, Dialog 66, DlgParam1 1,
        // answers 2, 1021, 4; effects (4, 1, 1, 100) and (8, 1, 5, 0).
        const ev1 = p.events[1];
        try t.expectEqual(@as(i32, -1), ev1.kind);
        try t.expectEqual(@as(i32, 20), ev1.possibility);
        try t.expectEqual(@as(i32, 66), ev1.dialog);
        try t.expectEqual(@as(usize, 3), ev1.n_answers);
        try t.expectEqualSlices(i32, &.{ 2, 1021, 4 }, ev1.answers[0..3]);
        try t.expectEqual(@as(usize, 2), ev1.n_effects);
        try t.expectEqual(Effect4{ .id = 4, .power = 1, .p1 = 1, .p2 = 100 }, ev1.effects[0]);
        try t.expectEqual(Effect4{ .id = 8, .power = 1, .p1 = 5, .p2 = 0 }, ev1.effects[1]);
        try t.expectEqual(@as(i32, 53), p.campaign_events[1].dialog);

        try t.expectEqual(@as(usize, 729), p.encounters.len);
        var enc_a: usize = 0;
        var enc_e: usize = 0;
        for (p.encounters) |x| {
            enc_a += x.n_answers;
            enc_e += x.n_effects;
        }
        try t.expectEqual(@as(usize, 1384), enc_a);
        try t.expectEqual(@as(usize, 1134), enc_e);
        // /1: Dialog 103, DlgParam1 2, answers 2..5, 7 effects, the last
        // (15, 6, -1, 32).
        const en1 = p.encounters[1];
        try t.expectEqual(@as(i32, 103), en1.dialog);
        try t.expectEqual(@as(i32, 2), en1.dlg_param1);
        try t.expectEqualSlices(i32, &.{ 2, 3, 4, 5 }, en1.answers[0..en1.n_answers]);
        try t.expectEqual(@as(usize, 7), en1.n_effects);
        try t.expectEqual(Effect4{ .id = 2, .power = -60, .p1 = 0, .p2 = 1 }, en1.effects[0]);
        try t.expectEqual(Effect4{ .id = 15, .power = 6, .p1 = -1, .p2 = 32 }, en1.effects[6]);

        try t.expectEqual(@as(usize, 57), p.quests.len);
        // /1: Type 1, Possibility 10, EncInit 64, EncFailGiver 79, Exp 200,
        // objective (19, 50, 1, 0, 0).
        const q1 = p.quests[1];
        try t.expectEqual(@as(i32, 1), q1.kind);
        try t.expectEqual(@as(i32, 10), q1.possibility);
        try t.expectEqual(@as(i32, 64), q1.enc_init);
        try t.expectEqual(@as(i32, 79), q1.enc_fail_giver);
        try t.expectEqual(@as(i32, 200), q1.exp);
        try t.expectEqual(QuestTask{ .index = 19, .param = 50, .quantity = 1 }, q1.tasks[0]);
        for (p.quests) |x| {
            try t.expectEqual(@as(usize, 1), x.n_tasks);
            try t.expectEqual(@as(usize, 1), x.n_abilities);
            try t.expect(std.mem.indexOfScalar(u8, x.name, '\n') == null);
        }
        for (p.quests[1..]) |x| try t.expect(x.text.len > 0);

        try t.expectEqual(@as(usize, 195), p.map_objects.len);
        // /1: Type 2, Layer 2, dx 50, dy 52, land 0, 5, 15, -1, -1, 0, 0, 0,
        // pictures 52..56.
        const mo1 = p.map_objects[1];
        try t.expectEqual(@as(i32, 2), mo1.kind);
        try t.expectEqual(@as(i32, 52), mo1.dy);
        try t.expectEqual([land_types]i32{ 0, 5, 15, -1, -1, 0, 0, 0 }, mo1.land);
        try t.expectEqual(@as(usize, 5), mo1.n_pics);
        try t.expectEqualSlices(i32, &.{ 52, 53, 54, 55, 56 }, mo1.pics[0..5]);
        var max_pics: usize = 0;
        for (p.map_objects) |x| max_pics = @max(max_pics, x.n_pics);
        try t.expectEqual(@as(usize, 15), max_pics);

        try t.expectEqual(@as(usize, 25), p.landmarks.len);
        // /1: Layer 1, dx 27, dy 13, Terrain (1), ProvType (5), Pic 28.
        const lm1 = p.landmarks[1];
        try t.expectEqual(@as(i32, 27), lm1.dx);
        try t.expectEqual(@as(i32, 1), lm1.land[0]);
        try t.expectEqual(@as(i32, 5), lm1.prov_types[0]);
        try t.expectEqual(@as(i32, 28), lm1.pic);
        try t.expectEqual(@as(i32, 0), lm1.shadow);

        try t.expectEqual(@as(usize, 232), p.castle_parts.len);
        // /1: Layer 2, X 199, Y 88, Yplace 147, Buildings (-1), Antibuildings (104).
        const cp1 = p.castle_parts[1];
        try t.expectEqual(@as(i32, 199), cp1.x);
        try t.expectEqual(@as(i32, 147), cp1.y_place);
        try t.expectEqual(@as(i32, -1), cp1.builds[0]);
        try t.expectEqual(@as(i32, 104), cp1.anti_builds[0]);
        var max_builds: usize = 0;
        for (p.castle_parts) |x| max_builds = @max(max_builds, x.n_builds);
        try t.expectEqual(@as(usize, 12), max_builds);

        // terr_layer.var: Forest 30, 20, 60, 0; Mountain 10, 6, 8, 0.
        try t.expectEqual([max_terr_layer]i32{ 0, 0, 1, 0 }, p.terr_layer[0]);
        try t.expectEqual([max_terr_layer]i32{ 30, 20, 60, 0 }, p.terr_layer[2]);
        try t.expectEqual([max_terr_layer]i32{ 10, 6, 8, 0 }, p.terr_layer[7]);

        // shard.var: /0 6x6 for 2 players, /1 8x8 for 3.
        try t.expectEqual(@as(i32, 6), p.shard_sizes[0].x);
        try t.expectEqual(@as(i32, 2), p.shard_sizes[0].players);
        try t.expectEqual(@as(i32, 8), p.shard_sizes[1].y);
        for (p.shard_sizes) |x| try t.expect(x.name.len > 0 and x.x > 0);
        // group_coord.var: /1 (298, 368; ...; 476, 355), /4 (189, 312).
        try t.expectEqual(Pair{ .id = 298, .value = 368 }, p.group_coords[1][0]);
        try t.expectEqual(Pair{ .id = 476, .value = 355 }, p.group_coords[1][3]);
        try t.expectEqual(Pair{ .id = 189, .value = 312 }, p.group_coords[4][0]);
        try t.expectEqual(Pair{}, p.group_coords[4][1]);

        try t.expectEqual(@as(usize, 24), p.hero_advice.len);
        try t.expectEqual(@as(usize, 17), p.ruler_nicks.len);
        try t.expectEqual(@as(i32, -30000), p.ruler_nicks[0].min);
        try t.expectEqual(@as(i32, -200), p.ruler_nicks[0].max);
        try t.expectEqual(@as(usize, 15), p.ruler_titles.len);
        try t.expectEqual(@as(usize, 11), p.attitudes.len);
        for ([_][]const RangeText{ p.hero_advice, p.ruler_nicks, p.ruler_titles, p.attitudes }) |tab|
            for (tab) |x| try t.expect(x.text.len > 0 and std.mem.indexOfScalar(u8, x.text, '\n') == null);

        try t.expectEqual(@as(usize, 18), p.credits.len);
        try t.expectEqual(@as(usize, 0), p.credits[0].names.len);
        try t.expectEqual(@as(usize, 11), p.credits[3].names.len);
        try t.expectEqualStrings("Andrew Stranger", p.credits[3].names[1]);
        try t.expectEqualStrings("Powered by Allegro", p.credits[17].title);

        try t.expectEqual(@as(usize, 10), p.shard_bonuses.len);
        try t.expectEqual(Pair{ .id = 10, .value = 20 }, p.shard_bonuses[0].levels[0][0]);
        try t.expectEqual(Pair{ .id = 80, .value = 100 }, p.shard_bonuses[0].levels[7][0]);
        // /9 «Договор со стражем»: Level2 has six pairs, Level7 ends (17, 5).
        const sb9 = p.shard_bonuses[9];
        try t.expectEqual(@as(i32, 4), sb9.kind);
        try t.expectEqual(@as(usize, 3), sb9.n_per_level[0]);
        try t.expectEqual(@as(usize, 6), sb9.n_per_level[1]);
        try t.expectEqual(Pair{ .id = 17, .value = 5 }, sb9.levels[6][4]);
        try t.expectEqual(@as(usize, 2), sb9.n_per_level[7]);

        // difficulty.var /0: 1000, 100, 15, 5, 1100, 80, 110, 55, 18, 60, 1, 25.
        const d0 = p.difficulties[0];
        try t.expectEqual(@as(i32, 1000), d0.gold);
        try t.expectEqual(@as(i32, 1100), d0.population);
        try t.expectEqual(@as(i32, 55), d0.neutral_life);
        try t.expectEqual(@as(i32, 60), d0.corrupt_cap);
        try t.expect(d0.show_advice);
        try t.expectEqual(@as(i32, 25), d0.score);
        try t.expectEqual(@as(i32, 850), p.difficulties[1].gold);
        for (p.difficulties) |x| try t.expect(x.name.len > 0);
        for (p.defenders[1..]) |x| {
            try t.expect(x.n_units >= 1 and x.n_loot >= 1 and x.text.len > 0);
            try t.expect(std.mem.indexOfScalar(u8, x.name, '\n') == null);
        }
        for (p.outer_buildings[1..]) |x| {
            try t.expect(x.text.len > 0);
            try t.expect(std.mem.indexOfScalar(u8, x.name, '\n') == null);
        }
    }
}

test "race quest list with several entries" {
    var r = Reader.init("Quests: (3, 12, 7)\nPlains: 5\n");
    _ = r.word();
    var q: [max_race_quest]i32 = undefined;
    const n = try r.list(&q);
    try t.expectEqualSlices(i32, &.{ 3, 12, 7 }, q[0..n]);
    try t.expectEqual(@as(i32, 5), r.field());
}
