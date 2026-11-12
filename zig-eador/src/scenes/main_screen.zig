//! MainScreen (MainProcess.cpp:1376): the strategic map of the player whose
//! turn it is.  First slice of the port:
//!   the map picture (newmap: the first layer and the borders),
//!   the fog (ShowMap, MainProcess.cpp:996): unknown provinces black, seen
//!   ones at half opacity, dark blots where unknown meets explored land,
//!   scrolling with CentreMap's limits (MX 0..W-724, MY 0..H-741),
//!   the province under the pointer and the selected one (Condition 1),
//!   the low panel (gold, gems, the name under the pointer), the province
//!   panel (40InfoBar) and the "Menu" and "End turn" buttons.
//! Heroes (ShowMap, MainProcess.cpp:1300): dolls with their shadow and
//! banner, a click selects one (Condition 20/21) and opens the hero panel
//! (51HeroBar), the secondary button gives a "go" order (MainProcess.cpp:
//! 4959) and the route is drawn from MovePath; "Next hero" cycles them.
//! "End turn" moves the heroes with an order (EndMainTurn, without the
//! battles) and redoes the fog.
//! A click on the doll of several heroes opens their list in the panel
//! (Condition 15, HeroList); an order to water or mountains shows
//! WindowInfo.
//! "Menu" opens WindowMenu (game_menu.zig).
//! Not yet: the rest of EndMainTurn.
//!
//! Scrolling: the original scrolls when the mouse touches the screen edge
//! or with the arrow keys; dragging with the primary button also scrolls,
//! for touch screens.  The secondary button clears the selection.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const png = @import("../formats/png.zig");
const mapgen = @import("../game/mapgen.zig");
const newmap = @import("../game/newmap.zig");
const province = @import("../game/province.zig");
const heraldry = @import("../ui/heraldry.zig");
const profile_mod = @import("../data/profile.zig");
const hero_mod = @import("../game/hero.zig");
const movement = @import("../game/movement.zig");
const economy = @import("../game/economy.zig");
const event_mod = @import("../game/event.zig");
const newsave = @import("../game/newsave.zig");
const decor = @import("../game/decor.zig");
const castle = @import("../game/castle.zig");
const castle_ui = @import("../ui/castle_ui.zig");
const CastleScreen = @import("castle.zig").CastleScreen;
const ProvinceGarrison = @import("garrison.zig").ProvinceGarrison;
const HeroShop = @import("hero_shop.zig").HeroShop;
const HeroScreen = @import("hero_screen.zig").HeroScreen;
const HeroLibrary = @import("hero_library.zig").HeroLibrary;
const HeroGarrison = @import("hero_garrison.zig").HeroGarrison;
const HeroHero = @import("hero_hero.zig").HeroHero;
const TreasureWindow = @import("treasure.zig").TreasureWindow;
const hero_task = @import("../game/hero_task.zig");
const ritual = @import("../game/ritual.zig");
const proto_mod = @import("../data/proto.zig");
const sound = @import("../platform/sound.zig");
const packfile = @import("../formats/packfile.zig");
const Dialog = @import("dialog.zig").Dialog;
const WindowInfo = @import("window_info.zig").WindowInfo;
const info_windows = @import("info_windows.zig");
const ResourceWindow = @import("army_info.zig").ResourceWindow;
const statistics_scene = @import("statistics.zig");
const ProvList = @import("prov_list.zig").ProvList;
const ProvPanel = @import("prov_panel.zig").ProvPanel;
const battle_scene = @import("battle.zig");
const hero_death = @import("hero_death.zig");
const game_menu = @import("game_menu.zig");
const Loading = @import("loading.zig").Loading;
const BattleScene = battle_scene.BattleScene;
const Button = @import("../ui/button.zig").Button;
const Mask = @import("../ui/mask.zig").Mask;
const Hint = @import("../ui/hint.zig").Hint;

/// LowY: the top of 39LowPanel (1024×106).
const low_y = 768 - 106;
/// BarX, BarY: 40InfoBar.
const bar_x = 707;
const bar_y = 243;
const bar_w = 312;
const bar_h = 470;
/// CentreMap: the view is 724×741 for the limits (the panels cover the rest).
const view_w = 724;
const view_h = 741;
/// The map is blitted 1024×723 (ShowActiveObjects).
const map_rows = 723;

const gui_low_panel = 39;
const gui_hero_bar = 51;
/// HeroList: 41BuildBar, the row mask 44 and its highlight 43, the row
/// frame 46RazePanel; BAR_SIZE rows 59 px apart.
const gui_build_bar = 41;
const gui_build_select = 43;
const gui_build_mask = 44;
/// bmpGUI_SpellMask, bmpGUI_SpellSelect.
const gui_spell_mask = 0;
const gui_spell_select = 1;
const gui_raze_panel = 46;
const bar_size = 6;
/// buttScrollUp 76..79, buttScrollDown 80..83.
const butt_scroll_up = 76;
const butt_scroll_down = 80;
/// The most heroes listed.
const max_list = 64;
/// WindowInfo of an order to water (message 49, stuff 53) or mountains
/// (message 48, stuff 47).
const msg_water = 49;
const msg_mountain = 48;
const stuff_water = 53;
const stuff_mountain = 47;
/// mapobj.dat: bmpHeroDoll 0..3, bmpHeroDollShadow 4..7, bmpHeroDollS
/// 20..23, bmpHeroSelect 40, bmpPath 41..44.
const obj_doll: usize = 0;
const obj_doll_shadow: usize = 4;
const obj_doll_active: usize = 20;
const obj_select: usize = 40;
const obj_path: usize = 41;
/// bmpQuest[0]: objEvent, the mark of the province an event happens in.
const obj_event: usize = 55;
/// buttNextHero 178..181.
const butt_next = 178;
const hint_next = 151;
/// Phrases: 39 "Приказ", 41..45 the orders, 152 "Несколько героев".
const ph_order = 39;
const ph_task_first = 41;
const ph_heroes = 152;
/// dialog.var: 1166 the hero cannot get there, 1244 no attack from a
/// foreign province.
const dlg_unreachable = 1166;
const dlg_no_attack = 1244;
const nature_dark = 36;
const n_races = 8;
/// bmpTown[race][level - 3]; bmpTownShadow follows the nine towns.
const town_shadow = 9;
/// The castle outer building (pFirstOuterBld + 23) of a ruler's home.
const bld_castle = 23;
/// bmpTown[0][0] is 120×120.
const town_box = 120;
/// buttMenu 104..106, buttEnd 101..103.
const butt_menu = 104;
const butt_end = 101;
const hint_menu = 96;
const hint_end = 100;
/// buttCastle 92..94 (319..321 when no castle building is left this
/// turn).
const butt_castle = 92;
const butt_castle_done = 319;
const hint_castle = 97;
/// buttStat (MainProcess.cpp:1498): buttons.dat 98..100, phrase 99.
const butt_stat = 98;
const hint_stat = 99;
/// buttShop 141..144 on the hero panel (phrase 102, key A) and mskHeroPic
/// over the portrait (phrase 146, key G).
const butt_shop = 141;
const hint_shop = 102;
/// buttLibrary 145..148 on the hero panel (phrase 103, key S).
const butt_library = 145;
const hint_library = 103;
/// The hero panel's orders: explore 125..128, enter 133..136, pillage
/// 129..132, attack 137..140, ritual 73..75 (120), skip 84..86.
const butt_explore = 125;
const butt_enter = 133;
const butt_pillage = 129;
const butt_attack = 137;
const butt_hero_ritual = 73;
const butt_hero_ritual_off = 120;
const butt_skip = 84;
const butt_cancel = 84;
const hint_explore = 555;
const hint_pillage = 556;
const hint_enter = 557;
const hint_attack = 558;
const hint_hero_ritual = 95;
const hint_skip = 101;
const hint_cancel = 106;
/// Phrases: 40 "Нет", 71 "Осада".
const ph_no_order = 40;
const ph_siege = 71;
/// interface.dat 42BuildPanel, 45Building.
const gui_build_panel = 42;
const gui_building = 45;
const hint_hero_pic = 146;
/// buttExchange 153..156 (phrase 105, key F).
const butt_exchange = 153;
const hint_exchange = 105;
/// dialog.var 388: the castle is besieged.
const dlg_castle_siege = 388;
/// The outer building ability that keeps a garrison.
const ab_outer_garrison = 4;
/// Phrase 51 "Ход".
const ph_turn = 51;

const color1: rl.Color = .{ .r = 0, .g = 0, .b = 0, .a = 255 };
const color2: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };
/// A drag shorter than this is a click.
const drag_slop = 6;

/// What the screen needs of arPlayer[i].
pub const Player = struct {
    /// Herb colours and sign (ShowHerb: the profile of a human, players.var
    /// of a computer).
    color1: i32 = 0,
    color2: i32 = 0,
    sign: i32 = 0,
    /// ExpForLevel's factor: the difficulty's iExp for a human,
    /// 100 - iDifficulty·5 for a computer.
    exp_pct: i32 = 100,
    /// GetName and GetAvatar: the profile's name and avatar of a human,
    /// players.var of a computer (avatar ipProfile-1).
    name_buf: [64]u8 = undefined,
    name_len: usize = 0,
    avatar: i32 = 16,

    pub fn name(self: *const Player) []const u8 {
        return self.name_buf[0..self.name_len];
    }

    pub fn setName(self: *Player, s: []const u8) void {
        self.name_len = @min(s.len, self.name_buf.len);
        @memcpy(self.name_buf[0..self.name_len], s[0..self.name_len]);
    }

    /// A human with profile `h` (null when the file is missing).
    pub fn ofHuman(h: ?*const profile_mod.Human, exp_pct: i32) Player {
        var p: Player = .{ .exp_pct = exp_pct };
        const hh = h orelse return p;
        p.color1 = hh.get(.color1);
        p.color2 = hh.get(.color2);
        p.sign = hh.get(.sign);
        p.avatar = hh.get(.avatar);
        var buf: [64]u8 = undefined;
        p.setName(hh.name(&buf));
        return p;
    }

    /// A computer ruler: players.var record `ruler`.
    pub fn ofComputer(pr: *const @import("../data/proto.zig").Proto, ruler: i32, difficulty: i32) Player {
        const ri: usize = @intCast(std.math.clamp(ruler, 0, @as(i32, @intCast(pr.player_profiles.len)) - 1));
        const pp = &pr.player_profiles[ri];
        var p: Player = .{ .color1 = pp.color1, .color2 = pp.color2, .sign = pp.sign, .exp_pct = 100 - difficulty * 5, .avatar = @as(i32, @intCast(ri)) - 1 };
        p.setName(pp.name);
        return p;
    }
};

/// Where MainScreen opens: a province or the hero in a slot of the
/// player's list (iCondition > 15).
pub const Start = @import("../game/loadgame.zig").Start;

/// The right page of the hero panel: Condition 16 (the heroes to exchange
/// with), 17 (the sites to enter) and 18/19 (the hero's rituals).
pub const HeroMode = enum { main, sites, rituals, ritual, exchange };

/// Condition 15: the heroes of a province to pick from.  `member` is the
/// clicked one (Member), `shift` the first row shown.
const HeroList = struct { prov: i32, member: usize, shift: usize = 0 };

/// pFirstFog: a province's box and its picture (black on transparent).
const Fog = struct { x: f32, y: f32, tex: ?rl.Texture2D };

/// dcProvince::Pic at PicX, PicY (DrawPic) and the level it was drawn for.
const Pic = struct { x: f32 = 0, y: f32 = 0, tex: ?rl.Texture2D = null, level: i32 = -1 };

/// A sheet's objects as CPU images for DrawPic, loaded once.
const Images = struct {
    sheet: *gfx.Sheet,
    map: std.AutoHashMapUnmanaged(u32, rl.Image) = .empty,

    pub fn get(self: *Images, k: i32) ?decor.Img {
        if (k < 0) return null;
        const key: u32 = @intCast(k);
        const img = self.map.get(key) orelse blk: {
            const loaded = self.sheet.imageIndex(key) orelse return null;
            self.map.put(self.sheet.gpa, key, loaded) catch {
                rl.unloadImage(loaded);
                return null;
            };
            break :blk loaded;
        };
        const n: usize = @intCast(img.width * img.height * 4);
        const px: [*]const u8 = @ptrCast(img.data);
        return .{ .w = @intCast(img.width), .h = @intCast(img.height), .px = px[0..n] };
    }

    fn deinit(self: *Images) void {
        var it = self.map.valueIterator();
        while (it.next()) |img| rl.unloadImage(img.*);
        self.map.deinit(self.sheet.gpa);
    }
};

/// mapobj.dat: bmpResBorder, bmpResGuardBorder, bmpResHidden.
const obj_res_border = 58;
const obj_res_guard_border = 59;
const obj_res_hidden = 60;
/// resource.dat: Pic, BigPic, then MedPic (iMaxResource each).
const res_med_pic = 2;
/// CompareBldParam1(26, terrain): a castle building that finds the hidden
/// resources of a terrain.
const ab_hidden_resource = 26;
/// Profile map icon 0: the coat of arms over the owned provinces.
const icon_herb = 0;

pub const MainScreen = struct {
    shard: mapgen.Shard,
    players: [mapgen.max_players + 1]Player,
    /// CurrentPlayer.
    player: usize = 1,
    /// CurGame.iCurrentTurn.
    turn: i32 = 0,
    map_tex: rl.Texture2D,
    fogs: []Fog,
    /// arPlayer[Player].pFog: 0 explored, 1 seen, 2 unknown.
    fog: []u8,
    /// bmpMapDark positions on the edge of the unknown land.
    edge: std.ArrayList(rl.Vector2) = .empty,
    iface: gfx.Sheet,
    buttons: gfx.Sheet,
    nature: gfx.Sheet,
    /// town/town1..8: bmpTown (0..8) and bmpTownShadow (9..17) per race.
    towns: [n_races]?gfx.Sheet = @splat(null),
    /// outbld: building (i-1)*2 and its shadow (i-1)*2+1 of outer building i.
    outbld: gfx.Sheet,
    arms: heraldry.Heraldry,
    menu: Button,
    end: Button,
    castle_button: Button,
    stat_button: Button,
    /// The province panel (Conditions 1..14).
    panel: ProvPanel,
    shop_button: Button,
    library_button: Button,
    explore_button: Button,
    enter_button: Button,
    pillage_button: Button,
    attack_button: Button,
    ritual_button: Button,
    hero_garr_button: Button,
    exchange_button: Button,
    skip_button: Button,
    cancel_button: Button,
    ritual_cast: Button,
    ritual_cancel: Button,
    /// The hero panel's second page.
    hero_mode: HeroMode = .main,
    site_shift: usize = 0,
    exchange_shift: usize = 0,
    /// ScreenHero or ScreenHeroHero closed: the squad and the mobility may
    /// have changed (Condition 20 again).
    refresh_hero: bool = false,
    ritual_shift: usize = 0,
    ritual_choice: i32 = 0,
    /// The answer of a hero task dialog.
    hero_answer: ?usize = null,
    /// The orders of the turn (EndMainTurn) and their dialogs.
    hero_tasks: ?hero_task.Runner = null,
    treasure_done: bool = false,
    /// site.dat and ritual.dat pictures of the lists.
    sites_sheet: gfx.Sheet,
    rituals_sheet: gfx.Sheet,
    /// mskHeroPic.
    hero_pic: Mask = .{ .x = bar_x + 127, .y = bar_y + 33 },
    hero_pic_shape: ?rl.Image = null,
    hint: Hint = .{},
    /// MX, MY.
    mx: f32 = 0,
    my: f32 = 0,
    /// ProvLevel (province_level.var).
    levels: []const @import("../data/proto.zig").ProvinceLevel = &.{},
    /// Member while Condition is 1; 0 for none.
    selected: i32 = 0,
    hover: i32 = 0,
    press: ?rl.Vector2 = null,
    dragging: bool = false,
    /// Condition 20/21: Member is the hero (index into shard.heroes).
    hero: ?usize = null,
    /// The hero under the pointer (its active doll is shown).
    hover_hero: ?usize = null,
    mapobj: gfx.Sheet,
    smallport: gfx.Sheet,
    /// bmpHeroDoll as images for the hit test of arHeroMask.
    dolls: [4]?rl.Image = @splat(null),
    next: Button,
    /// pMoveMap of the selected hero and GoPath of its order.
    walker: movement.Walker,
    /// The answer of an order dialog (ignored).
    dialog_result: ?usize = null,
    /// InfoString of a hero under the pointer.
    hover_name: ?[]const u8 = null,
    /// bTimeShift: a turn has ended in this session.
    time_shift: bool = false,
    /// resource.dat (MedPic on the map).
    resource: gfx.Sheet,
    /// dcProvince::Pic of every province (index = number).
    pics: []Pic = &.{},
    /// GetProvinceIndex over the map picture for DrawPic.
    ids: decor.Ids = .{ .w = 0, .h = 0, .ids = &.{} },
    /// CurGame.iSeed.
    seed: u32 = 111,
    /// Condition 15.
    list: ?HeroList = null,
    scroll_up: Button,
    scroll_down: Button,
    /// bmpGUI_BuildMask for mskBuild.
    build_mask: ?rl.Image = null,
    rows: [bar_size]Mask = undefined,
    /// A hero of the player stopped before a guarded neutral province at
    /// the end of the turn: its battle opens next frame (the event 1 of
    /// BeginMainTurn, Event.cpp:1417).
    pending_battle: ?usize = null,
    /// Set by the battle screen when it closes.
    battle_done: bool = false,
    /// The attacker of the battle and InitBattle's result.
    battle_hero: usize = 0,
    battle_result: i32 = 0,
    /// dcHero::Death of a human's hero after the battle.
    death: ?hero_death.Death = null,
    /// Dialog 1239 for the bodies that reached the capital this turn
    /// (owner, slot), then the one shown.
    arrivals: [mapgen.max_players * hero_mod.max_hero][2]i32 = undefined,
    n_arrivals: usize = 0,
    arrival: ?hero_death.Arrival = null,
    /// CastleMain's hero: the slot of the hero hired or resurrected.
    castle_hero: ?u8 = null,
    /// Shard.heroes.len when last seen: hiring and burying shift indices.
    known_heroes: usize = 0,
    /// WindowMenu is open; what it chose.
    menu_open: bool = false,
    menu_out: game_menu.Out = .{},
    /// The computer plays the player's side too (EADOR_BATTLE=auto).
    auto_battle: bool = false,
    /// The province ScreenStatistics / ScreenProvList / CastleMain
    /// returned (CastleProv, NProv), 0 for none.
    picked: i32 = 0,
    /// The events of the turn change (BeginMainTurn): their dialogs are
    /// shown one by one, then the runner goes.
    events: ?event_mod.Runner = null,
    /// The answer of an event dialog.
    event_answer: ?usize = null,
    /// objEvent (bmpQuest[0]) over the province of the event shown.
    event_prov: i32 = 0,

    /// A new game: player 1, turn 0, the new game's fog.  Takes ownership
    /// of `shard`.
    pub fn create(c: *Ctx, shard: mapgen.Shard, players: [mapgen.max_players + 1]Player) !scene.Scene {
        return open(c, shard, players, 1, 0, null, .{ .province = shard.capitals[1] });
    }

    /// MainScreen(player): takes ownership of `shard` and `fog` (pFog by
    /// province, null for a new game's) and centres on `start`.
    pub fn open(c: *Ctx, shard: mapgen.Shard, players: [mapgen.max_players + 1]Player, player: usize, turn: i32, fog: ?[]u8, start: Start) !scene.Scene {
        var op = try begin(c, shard, players, player, turn, fog, start);
        return op.run(c);
    }

    /// The first part of `open`; the pictures are made by Opening.step.
    pub fn begin(c: *Ctx, shard: mapgen.Shard, players: [mapgen.max_players + 1]Player, player: usize, turn: i32, fog: ?[]u8, start: Start) !Opening {
        var s = shard;
        errdefer s.deinit();
        errdefer if (fog) |f| c.gpa.free(f);
        if (fog) |f| if (f.len != s.provinces.len) return error.BadFog;
        const self = try c.gpa.create(MainScreen);
        errdefer c.gpa.destroy(self);
        self.* = .{
            .shard = s,
            .players = players,
            .player = player,
            .turn = turn,
            .levels = c.proto.province_levels,
            .map_tex = std.mem.zeroes(rl.Texture2D),
            .fogs = &.{},
            .fog = &.{},
            .iface = try gfx.Sheet.load(c.fs, "interface"),
            .buttons = undefined,
            .nature = undefined,
            .outbld = undefined,
            .arms = undefined,
            .menu = undefined,
            .end = undefined,
            .castle_button = undefined,
            .stat_button = undefined,
            .panel = undefined,
            .shop_button = undefined,
            .library_button = undefined,
            .explore_button = undefined,
            .enter_button = undefined,
            .pillage_button = undefined,
            .attack_button = undefined,
            .ritual_button = undefined,
            .hero_garr_button = undefined,
            .exchange_button = undefined,
            .skip_button = undefined,
            .cancel_button = undefined,
            .ritual_cast = undefined,
            .ritual_cancel = undefined,
            .sites_sheet = undefined,
            .rituals_sheet = undefined,
            .next = undefined,
            .scroll_up = undefined,
            .scroll_down = undefined,
            .mapobj = undefined,
            .smallport = undefined,
            .walker = undefined,
            .resource = undefined,
        };
        errdefer self.iface.deinit();
        self.known_heroes = self.shard.heroes.items.len;
        self.walker = try movement.Walker.init(c.gpa, &self.shard, c.proto, c.cfg.max_path);
        errdefer self.walker.deinit(c.gpa);
        self.mapobj = try gfx.Sheet.load(c.fs, "mapobj");
        errdefer self.mapobj.deinit();
        self.smallport = try gfx.Sheet.load(c.fs, "smallport");
        errdefer self.smallport.deinit();
        for (&self.dolls, 0..) |*d, k| d.* = self.mapobj.imageIndex(obj_doll + k);
        errdefer for (self.dolls) |d| if (d) |img| rl.unloadImage(img);
        self.buttons = try gfx.Sheet.load(c.fs, "buttons");
        errdefer self.buttons.deinit();
        self.nature = try gfx.Sheet.load(c.fs, "nature");
        errdefer self.nature.deinit();
        self.outbld = try gfx.Sheet.load(c.fs, "outbld");
        errdefer self.outbld.deinit();
        self.arms = try heraldry.Heraldry.load(c.fs);
        errdefer self.arms.deinit();
        for (&self.towns, 1..) |*t, r| {
            var pb: [32]u8 = undefined;
            t.* = gfx.Sheet.load(c.fs, std.fmt.bufPrint(&pb, "town/town{d}", .{r}) catch unreachable) catch null;
        }
        errdefer for (&self.towns) |*t| if (t.*) |*x| x.deinit();
        self.menu = try Button.init(&self.buttons, 8, low_y + 62, butt_menu, butt_menu + 1, butt_menu + 2);
        errdefer self.menu.deinit();
        self.menu.hotkey = .escape;
        self.menu.hint = hint_menu;
        self.end = try Button.init(&self.buttons, 954, low_y + 62, butt_end, butt_end + 1, butt_end + 2);
        errdefer self.end.deinit();
        self.end.hotkey = .e;
        self.end.hint = hint_end;
        self.next = try Button.init(&self.buttons, 686, low_y + 65, butt_next, butt_next + 1, butt_next + 2);
        errdefer self.next.deinit();
        self.next.inactive = butt_next + 3;
        self.next.hotkey = .n;
        self.next.hint = hint_next;
        const cb: usize = if (self.shard.rulers[player].holdings.can_inner_bld > 0) butt_castle else butt_castle_done;
        self.castle_button = try Button.init(&self.buttons, 745 + 1, low_y + 65, cb, cb + 1, cb + 2);
        errdefer self.castle_button.deinit();
        self.castle_button.hotkey = .c;
        self.castle_button.hint = hint_castle;
        self.stat_button = try Button.init(&self.buttons, 865 + 1, low_y + 65, butt_stat, butt_stat + 1, butt_stat + 2);
        errdefer self.stat_button.deinit();
        self.stat_button.hint = hint_stat;
        try self.panel.init(c, &self.iface, &self.buttons);
        errdefer self.panel.deinit();
        self.shop_button = try Button.init(&self.buttons, bar_x + 40, bar_y + 396, butt_shop, butt_shop + 1, butt_shop + 2);
        errdefer self.shop_button.deinit();
        self.shop_button.inactive = butt_shop + 3;
        self.shop_button.hotkey = .a;
        self.shop_button.hint = hint_shop;
        self.library_button = try Button.init(&self.buttons, bar_x + 40 + 60, bar_y + 396, butt_library, butt_library + 1, butt_library + 2);
        errdefer self.library_button.deinit();
        self.library_button.inactive = butt_library + 3;
        self.library_button.hotkey = .s;
        self.library_button.hint = hint_library;
        self.explore_button = try Button.init(&self.buttons, bar_x + 40, bar_y + 354, butt_explore, butt_explore + 1, butt_explore + 2);
        errdefer self.explore_button.deinit();
        self.explore_button.inactive = butt_explore + 3;
        self.explore_button.hotkey = .q;
        self.explore_button.hint = hint_explore;
        self.enter_button = try Button.init(&self.buttons, bar_x + 40 + 60, bar_y + 354, butt_enter, butt_enter + 1, butt_enter + 2);
        errdefer self.enter_button.deinit();
        self.enter_button.inactive = butt_enter + 3;
        self.enter_button.hotkey = .w;
        self.enter_button.hint = hint_enter;
        self.pillage_button = try Button.init(&self.buttons, bar_x + 40 + 60 * 2, bar_y + 354, butt_pillage, butt_pillage + 1, butt_pillage + 2);
        errdefer self.pillage_button.deinit();
        self.pillage_button.inactive = butt_pillage + 3;
        self.pillage_button.hotkey = .t;
        self.pillage_button.hint = hint_pillage;
        self.attack_button = try Button.init(&self.buttons, bar_x + 40 + 60 * 2, bar_y + 354, butt_attack, butt_attack + 1, butt_attack + 2);
        errdefer self.attack_button.deinit();
        self.attack_button.inactive = butt_attack + 3;
        self.attack_button.hotkey = .t;
        self.attack_button.hint = hint_attack;
        self.ritual_button = try Button.init(&self.buttons, bar_x + 40 + 60 * 3, bar_y + 354, butt_hero_ritual, butt_hero_ritual + 1, butt_hero_ritual + 2);
        errdefer self.ritual_button.deinit();
        self.ritual_button.inactive = butt_hero_ritual_off;
        self.ritual_button.hotkey = .r;
        self.ritual_button.hint = hint_hero_ritual;
        self.hero_garr_button = try Button.init(&self.buttons, bar_x + 40 + 60 * 2, bar_y + 396, 149, 150, 151);
        errdefer self.hero_garr_button.deinit();
        self.hero_garr_button.inactive = 152;
        self.hero_garr_button.hotkey = .d;
        self.hero_garr_button.hint = 104;
        self.exchange_button = try Button.init(&self.buttons, bar_x + 40 + 60 * 3, bar_y + 396, butt_exchange, butt_exchange + 1, butt_exchange + 2);
        errdefer self.exchange_button.deinit();
        self.exchange_button.inactive = butt_exchange + 3;
        self.exchange_button.hotkey = .f;
        self.exchange_button.hint = hint_exchange;
        self.skip_button = try Button.init(&self.buttons, bar_x + 40, bar_y + 354, butt_skip, butt_skip + 1, butt_skip + 2);
        errdefer self.skip_button.deinit();
        self.skip_button.hotkey = .q;
        self.skip_button.hint = hint_skip;
        self.cancel_button = try Button.init(&self.buttons, bar_x + 40 + 60, bar_y + 396, butt_cancel, butt_cancel + 1, butt_cancel + 2);
        errdefer self.cancel_button.deinit();
        self.cancel_button.hotkey = .escape;
        self.cancel_button.hint = hint_cancel;
        self.ritual_cast = try Button.init(&self.buttons, bar_x + 40 + 30, bar_y + 396, butt_hero_ritual, butt_hero_ritual + 1, butt_hero_ritual + 2);
        errdefer self.ritual_cast.deinit();
        self.ritual_cast.inactive = butt_hero_ritual_off;
        self.ritual_cast.hint = hint_hero_ritual;
        self.ritual_cancel = try Button.init(&self.buttons, bar_x + 40 + 60 * 2 + 30, bar_y + 396, butt_cancel, butt_cancel + 1, butt_cancel + 2);
        errdefer self.ritual_cancel.deinit();
        self.ritual_cancel.hotkey = .escape;
        self.ritual_cancel.hint = hint_cancel;
        self.sites_sheet = try gfx.Sheet.load(c.fs, "site");
        errdefer self.sites_sheet.deinit();
        self.rituals_sheet = try gfx.Sheet.load(c.fs, "ritual");
        self.hero_pic_shape = self.iface.imageIndex(gui_spell_mask);
        errdefer if (self.hero_pic_shape) |img| rl.unloadImage(img);
        self.scroll_up = try Button.init(&self.buttons, bar_x + 40, bar_y + 396, butt_scroll_up, butt_scroll_up + 1, butt_scroll_up + 2);
        errdefer self.scroll_up.deinit();
        self.scroll_up.inactive = butt_scroll_up + 3;
        self.scroll_down = try Button.init(&self.buttons, bar_x + 40 + 60 * 2, bar_y + 396, butt_scroll_down, butt_scroll_down + 1, butt_scroll_down + 2);
        errdefer self.scroll_down.deinit();
        self.scroll_down.inactive = butt_scroll_down + 3;
        self.build_mask = self.iface.imageIndex(gui_build_mask);
        errdefer if (self.build_mask) |img| rl.unloadImage(img);
        for (&self.rows, 0..) |*m, i| {
            m.* = .{ .x = bar_x + 48, .y = bar_y + 37 + @as(f32, @floatFromInt(i * 59)) };
            if (self.build_mask) |*img| m.shape = img;
        }
        self.resource = try gfx.Sheet.load(c.fs, "resource");
        return .{ .ms = self, .fog = fog, .start = start };
    }

    /// MainScreen being opened under a loading screen: the province shapes
    /// (LoadMap / the "Fog" step of GenerateMap), the map textures, then
    /// DrawPic of every province (FillObstracleMap).  Each `step` does one
    /// province, so the screen can redraw between them.
    pub const Opening = struct {
        ms: *MainScreen,
        /// Owned until `finish`.
        fog: ?[]u8,
        start: Start,
        phase: Phase = .shapes,
        pictures: ?newmap.Pictures = null,
        painter: ?Painter = null,
        /// The next province of the phase (1-based).
        next: usize = 1,

        pub const Phase = enum { shapes, textures, pics, done };

        /// The provinces done in the current phase.
        pub fn done(self: *const Opening) usize {
            return self.next - 1;
        }

        pub fn count(self: *const Opening) usize {
            return self.ms.shard.count();
        }

        /// The map picture with its borders and the province shapes once
        /// they are all made (SaveMap writes them); kept until `finish`.
        pub fn madePictures(self: *const Opening) ?*const newmap.Pictures {
            if (self.phase == .shapes) return null;
            return if (self.pictures) |*p| p else null;
        }

        pub fn step(self: *Opening, c: *Ctx) !void {
            const ms = self.ms;
            switch (self.phase) {
                .shapes => {
                    const p = if (self.pictures) |*x| x else {
                        self.pictures = try ms.startPictures(c);
                        return;
                    };
                    const all = try p.step(c.gpa, &ms.shard);
                    self.next = p.next;
                    if (all) {
                        newmap.borders(&p.map, &ms.shard, c.human.get(.map_borders));
                        self.phase = .textures;
                    }
                },
                .textures => {
                    const p = if (self.pictures) |*x| x else return error.NoPictures;
                    try ms.makeTextures(c, p);
                    // CurGame.iSeed for DrawPic: a new game's file is
                    // attached before the shard comes here.
                    ms.seed = newsave.seedOf(&ms.shard);
                    self.painter = Painter.init(ms);
                    self.phase = .pics;
                    self.next = 1;
                },
                .pics => {
                    const n = ms.shard.count();
                    if (self.next <= n) {
                        self.painter.?.paint(ms, c, self.next);
                        self.next += 1;
                    }
                    if (self.next > n) {
                        self.painter.?.deinit();
                        self.painter = null;
                        self.phase = .done;
                    }
                },
                .done => {},
            }
        }

        /// All the steps at once, then `finish`.
        pub fn run(self: *Opening, c: *Ctx) !scene.Scene {
            errdefer self.abort(c);
            while (self.phase != .done) try self.step(c);
            return self.finish(c);
        }

        /// The fog and the start position; the Opening is spent.  On an
        /// error the caller still calls `abort`.
        pub fn finish(self: *Opening, c: *Ctx) !scene.Scene {
            const ms = self.ms;
            if (self.pictures) |*p| p.deinit(c.gpa);
            self.pictures = null;
            if (self.fog) |f| {
                ms.fog = f;
                self.fog = null;
                try ms.makeFog(c);
            } else try ms.initFog(c);
            // c.setCursor(1); Do we need to set initial cursor position ? How to avoid conflicts with EADOR_SET_CURSOR ?
            const n: i32 = @intCast(ms.shard.count());
            var at: i32 = 1;
            switch (self.start) {
                .province => |p| at = if (p >= 1 and p <= n) p else 1,
                .hero => |slot| if (ms.heroInSlot(slot)) |k| {
                    ms.selectHero(k);
                    at = ms.shard.heroes.items[k].province;
                } else {
                    at = ms.shard.capitals[ms.player];
                },
            }
            const pv = ms.shard.provinces[@intCast(std.math.clamp(at, 1, n))];
            ms.centre(pv.x, pv.y);
            ms.updateNext();
            // RunGame: the map plays the Main tracks (iGameMode 2,
            // GameMenu.cpp:159, ChangeMusic at 170).
            c.music.setMode(c.fs, .map);
            return scene.Scene.of(MainScreen, ms, false);
        }

        /// Frees everything, the shard included.
        pub fn abort(self: *Opening, c: *Ctx) void {
            if (self.painter) |*p| p.deinit();
            if (self.pictures) |*p| p.deinit(c.gpa);
            if (self.fog) |f| c.gpa.free(f);
            self.ms.deinit(c);
            c.gpa.destroy(self.ms);
        }
    };

    pub fn deinit(self: *MainScreen, c: *Ctx) void {
        if (self.events) |*ev| ev.deinit();
        if (self.build_mask) |img| rl.unloadImage(img);
        self.scroll_down.deinit();
        self.scroll_up.deinit();
        self.next.deinit();
        if (self.hero_pic_shape) |img| rl.unloadImage(img);
        self.library_button.deinit();
        self.shop_button.deinit();
        self.rituals_sheet.deinit();
        self.sites_sheet.deinit();
        self.ritual_cancel.deinit();
        self.ritual_cast.deinit();
        self.cancel_button.deinit();
        self.skip_button.deinit();
        self.exchange_button.deinit();
        self.hero_garr_button.deinit();
        self.ritual_button.deinit();
        self.attack_button.deinit();
        self.pillage_button.deinit();
        self.enter_button.deinit();
        self.explore_button.deinit();
        if (self.hero_tasks) |*t| t.deinit();
        self.panel.deinit();
        self.stat_button.deinit();
        self.castle_button.deinit();
        for (self.dolls) |d| if (d) |img| rl.unloadImage(img);
        self.smallport.deinit();
        self.mapobj.deinit();
        self.walker.deinit(c.gpa);
        self.edge.deinit(c.gpa);
        c.gpa.free(self.fog);
        self.freePics(c);
        self.ids.deinit(c.gpa);
        self.resource.deinit();
        self.freePictures(c);
        self.end.deinit();
        self.menu.deinit();
        for (&self.towns) |*t| if (t.*) |*x| x.deinit();
        self.arms.deinit();
        self.outbld.deinit();
        self.nature.deinit();
        self.buttons.deinit();
        self.iface.deinit();
        self.shard.deinit();
    }

    /// The first layer of the map picture and the province boxes, as
    /// GenerateMap draws them.
    fn startPictures(self: *MainScreen, c: *Ctx) !newmap.Pictures {
        const tile_png = c.fs.read("sprites/nature/000_000Grass.png") catch null;
        defer if (tile_png) |b| c.gpa.free(b);
        const tile = if (tile_png) |b| png.decode(c.gpa, b) catch null else null;
        defer if (tile) |t| c.gpa.free(t.rgba);
        return newmap.Pictures.init(c.gpa, &self.shard, if (tile) |t| .{ .w = t.w, .h = t.h, .rgba = t.rgba } else null);
    }

    /// The map and fog textures from the finished pictures, and what
    /// DrawPic needs.
    fn makeTextures(self: *MainScreen, c: *Ctx, p: *const newmap.Pictures) !void {
        const s = &self.shard;
        self.map_tex = try texture(c.gpa, &p.map, null);
        self.fogs = try c.gpa.alloc(Fog, s.provinces.len);
        @memset(self.fogs, .{ .x = 0, .y = 0, .tex = null });
        for (p.shapes[1..], 1..) |*sh, i| {
            self.fogs[i] = .{
                .x = @floatFromInt(sh.x0 - 1),
                .y = @floatFromInt(sh.y0 - 1),
                .tex = texture(c.gpa, &sh.bmp, newmap.magenta) catch null,
            };
        }
        self.ids = try decor.Ids.init(c.gpa, s);
        self.pics = try c.gpa.alloc(Pic, s.provinces.len);
        @memset(self.pics, .{});
    }

    fn freePictures(self: *MainScreen, c: *Ctx) void {
        for (self.fogs) |f| if (f.tex) |t| rl.unloadTexture(t);
        c.gpa.free(self.fogs);
        self.fogs = &.{};
        rl.unloadTexture(self.map_tex);
    }

    /// The bitmap as a texture; `key` (Allegro's mask colour) is transparent.
    fn texture(gpa: std.mem.Allocator, b: *const newmap.Bitmap, key: ?u32) !rl.Texture2D {
        const px = try gpa.alloc(u8, b.px.len * 4);
        defer gpa.free(px);
        for (b.px, 0..) |v, i| {
            const a: u8 = if (key != null and v == key.?) 0 else 255;
            px[i * 4 ..][0..4].* = .{ @truncate(v >> 16), @truncate(v >> 8), @truncate(v), a };
        }
        return gfx.textureFromRgba(b.w, b.h, px);
    }

    /// The new game's pFog (0 for the capital and its neighbours), then
    /// MakeFog: everything known becomes "seen", the own provinces and
    /// their neighbours explored.
    fn initFog(self: *MainScreen, c: *Ctx) !void {
        const s = &self.shard;
        self.fog = try c.gpa.alloc(u8, s.provinces.len);
        @memset(self.fog, 2);
        const home: usize = @intCast(s.capitals[self.player]);
        self.fog[home] = 0;
        for (s.provinces[home].neighbourList()) |nb| self.fog[@intCast(nb)] = 0;
        try self.makeFog(c);
    }

    fn makeFog(self: *MainScreen, c: *Ctx) !void {
        const s = &self.shard;
        for (self.fog[1..]) |*f| if (f.* < 2) {
            f.* = 1;
        };
        for (s.provinces[1..], 1..) |*pv, i| {
            if (pv.player != @as(i32, @intCast(self.player))) continue;
            self.fog[i] = 0;
            for (pv.neighbourList()) |nb| self.fog[@intCast(nb)] = 0;
        }
        // Around the heroes.
        for (s.heroes.items) |*h| {
            if (h.owner != @as(i32, @intCast(self.player)) or !h.alive()) continue;
            self.fog[@intCast(h.province)] = 0;
            for (s.provinces[@intCast(h.province)].neighbourList()) |nb| self.fog[@intCast(nb)] = 0;
        }
        // Dark blots on the cells between unknown and explored land; one
        // per 10×10 neighbourhood.
        self.edge.clearRetainingCapacity();
        for (0..s.grid.rows()) |y| {
            var k = s.grid.heads[y];
            while (k >= 0) {
                const cell = s.grid.cells.items[@intCast(k)];
                k = cell.next;
                if (cell.a <= 0 or cell.b <= 0) continue;
                const fa = self.fog[@intCast(cell.a)];
                const fb = self.fog[@intCast(cell.b)];
                if (!((fa == 2 and fb == 0) or (fa == 0 and fb == 2))) continue;
                const v: rl.Vector2 = .{ .x = @floatFromInt(cell.x), .y = @floatFromInt(y) };
                const near = for (self.edge.items) |o| {
                    if (@abs(o.x - v.x) < 10 and @abs(o.y - v.y) < 10) break true;
                } else false;
                if (!near) try self.edge.append(c.gpa, v);
            }
        }
    }

    /// CentreMap.
    fn centre(self: *MainScreen, x: i32, y: i32) void {
        self.mx = @floatFromInt(x - 512);
        self.my = @floatFromInt(y - 384);
        self.clamp();
    }

    fn clamp(self: *MainScreen) void {
        const mw: f32 = @floatFromInt(@max(self.map_tex.width - view_w, 0));
        const mh: f32 = @floatFromInt(@max(self.map_tex.height - view_h, 0));
        self.mx = std.math.clamp(self.mx, 0, mw);
        self.my = std.math.clamp(self.my, 0, mh);
    }

    /// The pointer is over the map, not over a panel.
    fn overMap(self: *const MainScreen, p: input.Pointer) bool {
        if (p.pos.y >= low_y) return false;
        if ((self.selected > 0 or self.hero != null or self.list != null) and p.in(bar_x, bar_y, bar_w, bar_h)) return false;
        if (self.selected > 0 and self.hero == null and self.list == null and self.panel.over(p)) return false;
        return true;
    }

    fn provinceAt(self: *const MainScreen, p: input.Pointer) i32 {
        if (!p.hover or !self.overMap(p)) return 0;
        return self.shard.lookup(@intFromFloat(self.mx + p.pos.x), @intFromFloat(self.my + p.pos.y));
    }

    pub fn update(self: *MainScreen, c: *Ctx, f: *const input.Frame, dt: f32) scene.Action {
        self.hint.clear();
        const p = f.pointer;
        if (self.battle_done) {
            self.battle_done = false;
            const k = self.battle_hero;
            if (k < self.shard.heroes.items.len) if (self.shard.heroes.items[k].squad[0]) |u| if (u.life == 0) {
                self.death = hero_death.Death.begin(self.market(c, self.shard.heroes.items[k].owner), &self.walker, k, self.battle_result);
            };
            // The battle of an order of the hero (game/hero_task.zig).
            if (self.hero_tasks) |*t| t.battleDone(self.battle_result);
            self.makeFog(c) catch {};
            self.drawPics(c);
            self.afterHeroes();
        }
        if (self.pumpHeroes(c)) |a| return a;
        if (self.castle_hero) |slot| {
            self.castle_hero = null;
            self.checkHeroes();
            if (self.heroInSlot(slot)) |k| {
                self.selectHero(k);
                const cap = &self.shard.provinces[@intCast(self.shard.capitals[self.player])];
                self.centre(cap.x, cap.y);
            }
        }
        self.checkHeroes();
        if (self.refresh_hero) {
            self.refresh_hero = false;
            self.refreshRoute();
        }
        if (self.pending_battle) |k| {
            self.pending_battle = null;
            const target = self.shard.heroes.items[k].target;
            self.battle_hero = k;
            self.battle_result = 0;
            const where: battle_scene.Where = .{ .shard = &self.shard, .hero = k, .target = target, .done = &self.battle_done, .result = &self.battle_result };
            return .{ .push = BattleScene.create(c, where, self.auto_battle) catch |e| {
                std.log.err("battle: {t}", .{e});
                return .none;
            } };
        }
        if (self.pumpHeroTasks(c)) |a| return a;
        if (self.pumpEvents(c)) |a| return a;
        if (self.menu_open) {
            self.menu_open = false;
            if (self.afterMenu(c)) |a| return a;
        }
        const panel_page = self.selected > 0 and self.hero == null and self.list == null and self.panel.mode != .main;
        if (self.hero_mode == .main and !panel_page and self.menu.check(f)) {
            const gt = newsave.gameType(&self.shard);
            const tutorial = event_mod.tutorialMap(&self.shard);
            self.menu_open = true;
            return .{ .push = game_menu.GameMenu.create(c, .{
                .past = self.time_shift and gt == 1 and !tutorial,
                .surrender = gt == 1 and !tutorial,
            }, &self.menu_out) catch |e| {
                std.log.err("game menu: {t}", .{e});
                self.menu_open = false;
                return .none;
            } };
        }
        if (self.end.check(f)) self.endTurn(c);
        if (self.next.check(f) or f.key(.space)) self.nextHero();
        if (self.picked > 0) self.pickProvince(self.picked);
        if (self.castle_button.check(f)) return self.openCastle(c);
        if (self.stat_button.check(f)) return self.openStatistics(c);
        if (f.key(.p)) return self.openProvList(c);
        self.updateShopButton(c);
        if (self.shop_button.check(f)) return self.openHeroShop(c);
        self.updateLibraryButton(c);
        if (self.library_button.check(f)) return self.openHeroLibrary(c);
        if (self.hero != null) {
            switch (self.hero_mode) {
                .main => if (self.updateHeroPanel(c, f)) |a| return a,
                .sites => if (self.updateSiteList(c, f)) |a| return a,
                .rituals, .ritual => if (self.updateRitualPage(c, f)) |a| return a,
                .exchange => if (self.updateExchangeList(c, f)) |a| return a,
            }
        } else self.hero_pic.state = .visible;
        if (self.hero_mode == .main) for ([_]*Button{ &self.menu, &self.end, &self.next, &self.castle_button, &self.stat_button, &self.shop_button, &self.library_button, &self.explore_button, &self.enter_button, &self.pillage_button, &self.attack_button, &self.ritual_button, &self.skip_button, &self.hero_garr_button, &self.exchange_button }) |b| if (b.hot()) self.hint.phrase(c, b.hint, p.pos);
        if (self.list != null) self.updateList(f);
        if (self.hero == null and self.list == null and self.selected > 0) {
            if (self.updatePanel(c, f)) |a| return a;
        }

        // Edge and arrow scrolling: iScrollSpeed every refresh tick.
        const speed: f32 = @as(f32, @floatFromInt(c.human.get(.scroll_speed))) * 1000 / @as(f32, @floatFromInt(c.cfg.refresh_delay)) * dt;
        if (!c.skipSetCursor) {
            if ((p.hover and p.pos.x <= 0) or rl.isKeyDown(.left)) self.mx -= speed;
            if ((p.hover and p.pos.x >= 1023) or rl.isKeyDown(.right)) self.mx += speed;
            if ((p.hover and p.pos.y <= 0) or rl.isKeyDown(.up)) self.my -= speed;
            if ((p.hover and p.pos.y >= 767) or rl.isKeyDown(.down)) self.my += speed;
        }

        if (p.primary.pressed and self.overMap(p)) {
            self.press = p.pos;
            self.dragging = false;
        }
        if (self.press) |start| {
            if (p.primary.down) {
                if (!self.dragging and (@abs(p.pos.x - start.x) > drag_slop or @abs(p.pos.y - start.y) > drag_slop)) self.dragging = true;
                if (self.dragging) {
                    self.mx -= p.pos.x - start.x;
                    self.my -= p.pos.y - start.y;
                    self.press = p.pos;
                }
            } else {
                if (!self.dragging and p.primary.released) {
                    if (self.heroAt(p)) |k| {
                        self.clickHero(k);
                    } else {
                        const at = self.provinceAt(p);
                        if (at > 0 and self.fog[@intCast(at)] < 2 and province.terrain.land(self.shard.provinces[@intCast(at)].terrain)) {
                            self.selected = at;
                            self.panel.reset();
                            self.hero = null;
                            self.list = null;
                        }
                    }
                }
                self.press = null;
            }
        }
        var action: scene.Action = .none;
        if (p.secondary.released and self.overMap(p)) {
            if (self.hero != null) {
                // Condition 16 → 20 with the order.
                if (self.hero_mode == .exchange) self.hero_mode = .main;
                action = self.order(c, p);
            } else {
                self.selected = 0;
                self.list = null;
            }
        }
        self.clamp();
        self.hover = self.provinceAt(p);
        self.hover_hero = self.heroAt(p);
        if (self.hover_hero) |k| {
            const h = &self.shard.heroes.items[k];
            self.hover_name = if (self.heroesIn(h.province) == 1) h.nameText(c.proto) else c.proto.phrase(ph_heroes);
        } else self.hover_name = null;
        return action;
    }

    pub fn draw(self: *MainScreen, c: *Ctx) void {
        var list_buf: [max_list]usize = undefined;
        rl.clearBackground(.black);
        const ox = -@floor(self.mx);
        const oy = -@floor(self.my);
        rl.beginScissorMode(0, 0, 1024, map_rows);
        rl.drawTextureV(self.map_tex, .{ .x = ox, .y = oy }, .white);
        // ShowMap layers: towns and the unknown land (1), flags (2), the
        // seen land at half opacity (4).
        self.drawTowns(ox, oy);
        self.drawFog(ox, oy, 2);
        if (self.hero) |k| centredAbove(self.mapobj.texIndex(obj_select), self.shard.provinces[@intCast(self.shard.heroes.items[k].province)], ox, oy, 2);
        if (self.list) |l| centredAbove(self.mapobj.texIndex(obj_select), self.shard.provinces[@intCast(l.prov)], ox, oy, 2);
        // The hero banners were created after the provinces' layer-2
        // objects, so they are drawn first.
        self.drawHeroes(ox, oy, .banners);
        self.drawFlags(c, ox, oy);
        self.drawHeroes(ox, oy, .dolls);
        self.drawPath(ox, oy);
        // objEvent, layer 8: PlaceOnMap(x, y) puts its corner there.
        if (self.event_prov > 0) {
            const pv = &self.shard.provinces[@intCast(self.event_prov)];
            gfx.drawTex(self.mapobj.texIndex(obj_event), ox + @as(f32, @floatFromInt(pv.x)), oy + @as(f32, @floatFromInt(pv.y)));
        }
        self.drawFog(ox, oy, 1);
        if (self.selected > 0) self.shade(self.selected, ox, oy, .{ .r = 157, .g = 157, .b = 157, .a = 60 });
        if (self.hover > 0 and self.hover != self.selected) self.shade(self.hover, ox, oy, .{ .r = 0, .g = 0, .b = 0, .a = 51 });
        rl.endScissorMode();

        gfx.drawTex(self.iface.texIndex(gui_low_panel), 0, low_y);
        const name = self.hover_name orelse if (self.hover > 0 and self.fog[@intCast(self.hover)] < 2) self.shard.provinces[@intCast(self.hover)].name else "";
        c.font1.drawCentered(name, 500, low_y + 79, color2);
        var b: [32]u8 = undefined;
        const pl = self.shard.rulers[self.player];
        c.font1.drawCentered(std.fmt.bufPrint(&b, "{d}", .{pl.gold}) catch "", 156, low_y + 79, color2);
        c.font1.drawCentered(std.fmt.bufPrint(&b, "{d}", .{pl.gem}) catch "", 280, low_y + 79, color2);
        c.font1.draw(std.fmt.bufPrint(&b, "{s}: {d}", .{ c.proto.phrase(ph_turn), self.turn }) catch "", 360, low_y + 79, color2);
        self.menu.draw(&self.buttons, c.tick);
        self.end.draw(&self.buttons, c.tick);
        self.next.draw(&self.buttons, c.tick);
        self.castle_button.draw(&self.buttons, c.tick);
        self.stat_button.draw(&self.buttons, c.tick);
        if (self.hero) |k| switch (self.hero_mode) {
            .main => {
                self.drawHeroBar(c, k);
                self.shop_button.draw(&self.buttons, c.tick);
                self.library_button.draw(&self.buttons, c.tick);
                self.explore_button.draw(&self.buttons, c.tick);
                self.enter_button.draw(&self.buttons, c.tick);
                if (self.attack_button.enabled) self.attack_button.draw(&self.buttons, c.tick) else self.pillage_button.draw(&self.buttons, c.tick);
                self.ritual_button.draw(&self.buttons, c.tick);
                self.skip_button.draw(&self.buttons, c.tick);
                self.hero_garr_button.draw(&self.buttons, c.tick);
                self.exchange_button.draw(&self.buttons, c.tick);
                if (self.hero_pic.hot()) gfx.drawTex(self.iface.texIndex(gui_spell_select), self.hero_pic.x, self.hero_pic.y);
            },
            .sites => self.drawSiteList(c),
            .rituals => self.drawRitualList(c),
            .ritual => self.drawRitualDetail(c),
            .exchange => self.drawHeroRows(c, self.exchangeRows(&list_buf)),
        } else if (self.list) |l| self.drawHeroList(c, l) else if (self.selected > 0) {
            self.panel.draw(c, self.panelView(), self.selected);
        }
        self.hint.draw(c, &self.iface);
    }

    /// A province picture in one colour (draw_lit_sprite of pFirstFog.Pic).
    fn shade(self: *const MainScreen, i: i32, ox: f32, oy: f32, col: rl.Color) void {
        const f = self.fogs[@intCast(i)];
        const t = f.tex orelse return;
        rl.drawTextureV(t, .{ .x = ox + f.x, .y = oy + f.y }, col);
    }

    /// The fog pictures of the provinces in state `v` (2 unknown, black;
    /// 1 seen, half black); the dark blots go with the unknown land.
    fn drawFog(self: *MainScreen, ox: f32, oy: f32, v: u8) void {
        for (self.fog[1..], 1..) |f, i| {
            if (f != v) continue;
            self.shade(@intCast(i), ox, oy, .{ .r = 0, .g = 0, .b = 0, .a = if (v == 1) 128 else 255 });
        }
        if (v != 2) return;
        const dark = self.nature.texIndex(nature_dark) orelse return;
        for (self.edge.items) |e| rl.drawTextureV(dark, .{ .x = ox + e.x - 15, .y = oy + e.y - 15 }, .{ .r = 255, .g = 255, .b = 255, .a = 128 });
    }

    /// A picture centred on (x, y) of the map.
    fn centred(t: ?rl.Texture2D, x: i32, y: i32, ox: f32, oy: f32, col: rl.Color) void {
        const tx = t orelse return;
        rl.drawTextureV(tx, .{ .x = ox + @as(f32, @floatFromInt(x - @divTrunc(tx.width, 2))), .y = oy + @as(f32, @floatFromInt(y - @divTrunc(tx.height, 2))) }, col);
    }

    fn onScreen(pv: *const province.Province, ox: f32, oy: f32) bool {
        const x = ox + @as(f32, @floatFromInt(pv.x));
        const y = oy + @as(f32, @floatFromInt(pv.y));
        return x > -200 and x < 1224 and y > -200 and y < map_rows + 200;
    }

    /// arPlayer[j].ipProvince of a player in the game.
    fn isHome(self: *const MainScreen, pv: *const province.Province) bool {
        for (1..self.shard.n_players + 1) |k| {
            if (self.shard.in_game[k] and self.shard.capitals[k] == pv.number) return true;
        }
        return false;
    }

    /// The building pictures DrawPic composes: the town of level 3 and up
    /// first, then the castle at the first place of a home province, else
    /// the outer buildings at their places.
    fn picSprites(self: *MainScreen, pv: *const province.Province, towns: []?Images, outbld: *Images) [province.max_bld + 1]?decor.Sprite {
        var sp: [province.max_bld + 1]?decor.Sprite = @splat(null);
        const lvl = self.levelOf(pv.population);
        if (lvl > 2 and pv.race >= 1 and pv.race <= n_races) if (towns[@intCast(pv.race - 1)]) |*t| {
            if (t.get(lvl - 3)) |img| {
                const at = decor.placeAt(pv, img.w, img.h, pv.x, pv.y);
                sp[0] = .{ .img = img, .x = at[0], .y = at[1] };
            }
        };
        if (self.isHome(pv)) {
            if (outbld.get((bld_castle - 1) * 2)) |img| {
                const at = decor.placeAt(pv, img.w, img.h, pv.bld_place[0][0], pv.bld_place[0][1]);
                sp[1] = .{ .img = img, .x = at[0], .y = at[1] };
            }
        } else for (pv.buildings[0..province.max_bld], 0..) |b, k| {
            if (b <= 0) continue;
            const img = outbld.get((b - 1) * 2) orelse continue;
            const at = decor.placeAt(pv, img.w, img.h, pv.bld_place[k][0], pv.bld_place[k][1]);
            sp[k + 1] = .{ .img = img, .x = at[0], .y = at[1] };
        }
        return sp;
    }

    /// DrawPic for the provinces whose level changed since their picture
    /// was made (all of them at first).
    fn drawPics(self: *MainScreen, c: *Ctx) void {
        var p = Painter.init(self);
        defer p.deinit();
        for (1..self.shard.provinces.len) |i| p.paint(self, c, i);
    }

    /// The sheets' images DrawPic reads, loaded once for a pass.
    const Painter = struct {
        nature: Images,
        outbld: Images,
        towns: [n_races]?Images = @splat(null),

        fn init(ms: *MainScreen) Painter {
            var p: Painter = .{ .nature = .{ .sheet = &ms.nature }, .outbld = .{ .sheet = &ms.outbld } };
            for (&p.towns, &ms.towns) |*t, *sh| if (sh.*) |*x| {
                t.* = .{ .sheet = x };
            };
            return p;
        }

        fn deinit(self: *Painter) void {
            for (&self.towns) |*t| if (t.*) |*x| x.deinit();
            self.outbld.deinit();
            self.nature.deinit();
        }

        /// DrawPic of province `i` when its level changed.
        fn paint(self: *Painter, ms: *MainScreen, c: *Ctx, i: usize) void {
            const pv = &ms.shard.provinces[i];
            const lvl = ms.levelOf(pv.population);
            const pic = &ms.pics[i];
            if (pic.level == lvl) return;
            if (pic.tex) |t| rl.unloadTexture(t);
            pic.* = .{ .level = lvl };
            const w: decor.World = .{ .s = &ms.shard, .pr = c.proto, .ids = &ms.ids, .seed = ms.seed, .variant = @intCast(c.seed) };
            const sp = ms.picSprites(pv, &self.towns, &self.outbld);
            const comp = (decor.compose(c.gpa, &w, &self.nature, @intCast(i), sp) catch null) orelse return;
            defer c.gpa.free(comp.px);
            pic.tex = gfx.textureFromRgba(comp.w, comp.h, comp.px) catch null;
            pic.x = @floatFromInt(comp.map_x);
            pic.y = @floatFromInt(comp.map_y);
        }
    };

    fn freePics(self: *MainScreen, c: *Ctx) void {
        for (self.pics) |p| if (p.tex) |t| rl.unloadTexture(t);
        c.gpa.free(self.pics);
        self.pics = &.{};
    }

    /// ShowMap's layer 1 per province (province 1 first): the castle and
    /// building shadows and the town shadow at 50%, then Pic.
    fn drawTowns(self: *MainScreen, ox: f32, oy: f32) void {
        const half: rl.Color = .{ .r = 255, .g = 255, .b = 255, .a = 128 };
        for (self.shard.provinces[1..], 1..) |*pv, i| {
            if (self.fog[i] == 2 or !onScreen(pv, ox, oy)) continue;
            if (pv.terrain == province.terrain.water or pv.terrain == province.terrain.mountain) continue;
            if (self.isHome(pv)) centred(self.outbld.texIndex((bld_castle - 1) * 2 + 1), pv.bld_place[0][0], pv.bld_place[0][1], ox, oy, half);
            for (pv.buildings[0..province.max_bld], 0..) |b, k| {
                if (b > 0) centred(self.outbld.texIndex(@intCast((b - 1) * 2 + 1)), pv.bld_place[k][0], pv.bld_place[k][1], ox, oy, half);
            }
            const lvl = self.levelOf(pv.population);
            if (lvl > 2 and pv.race >= 1 and pv.race <= n_races) if (self.towns[@intCast(pv.race - 1)]) |*t| {
                centred(t.texIndex(town_shadow + @as(usize, @intCast(lvl - 3))), pv.x, pv.y, ox, oy, half);
            };
            const p = self.pics[i];
            if (p.tex) |t| rl.drawTextureV(t, .{ .x = ox + p.x, .y = oy + p.y }, .white);
        }
    }

    /// ShowMap's layer 2 per province: the castle banners of a home, the
    /// town banners (more for big towns), the coat of arms (profile map
    /// icon 0) and the resource mark.
    fn drawFlags(self: *MainScreen, c: *Ctx, ox: f32, oy: f32) void {
        const eco: economy.Economy = .{ .s = &self.shard, .pr = c.proto };
        const castle_half = town_box / 2;
        for (self.shard.provinces[1..], 1..) |*pv, i| {
            if (self.fog[i] == 2 or !onScreen(pv, ox, oy)) continue;
            if (pv.player > 0) {
                const pl = self.players[@intCast(pv.player)];
                if (self.isHome(pv)) for ([3][2]i32{ .{ -17, -27 }, .{ -2, -12 }, .{ -48, -22 } }) |d| {
                    const fx = pv.bld_place[0][0] - castle_half + d[0];
                    const fy = pv.bld_place[0][1] - castle_half + d[1];
                    self.arms.drawCityFlag(1, pl.color1, pl.color2, pl.sign, ox + @as(f32, @floatFromInt(fx)), oy + @as(f32, @floatFromInt(fy)));
                };
                const x = ox + @as(f32, @floatFromInt(pv.x - town_box / 2));
                const y = oy + @as(f32, @floatFromInt(pv.y - town_box / 2));
                const lvl = self.levelOf(pv.population);
                var k: usize = 3;
                while (k > 0) {
                    k -= 1;
                    if ((k == 1 and lvl <= 5) or (k == 2 and lvl <= 8)) continue;
                    self.arms.drawCityFlag(k, pl.color1, pl.color2, pl.sign, x, y);
                }
                if (c.human.mapIcon(icon_herb)) self.arms.drawHerb(pl.color1, pl.color2, pl.sign, ox + @as(f32, @floatFromInt(pv.x - heraldry.herb_w / 2)), oy + @as(f32, @floatFromInt(pv.y - heraldry.herb_h / 2)));
            }
            self.drawResource(&eco, pv, ox, oy);
        }
    }

    /// The resource mark right of the centre: a found and free one in
    /// bmpResBorder, a guarded one in bmpResGuardBorder (under the
    /// picture, then again over it at 50%), a hidden one as bmpResHidden
    /// when the castle knows the terrain's resources.  Objects of a layer
    /// are drawn in the reverse order of their creation.
    fn drawResource(self: *MainScreen, eco: *const economy.Economy, pv: *const province.Province, ox: f32, oy: f32) void {
        const half: rl.Color = .{ .r = 255, .g = 255, .b = 255, .a = 128 };
        const cx = pv.x + 30;
        const free = eco.resource(pv, 1);
        const guarded = if (free > 0) 0 else eco.resource(pv, 0);
        const r = if (free > 0) free else guarded;
        if (r > 0) {
            const pic = self.resource.texIndex(@intCast(r - 1 + res_med_pic * (@as(i32, @intCast(eco.pr.resources.len)) - 1))) orelse return;
            const bx = ox + @as(f32, @floatFromInt(cx - @divTrunc(pic.width, 2) - 2));
            const by = oy + @as(f32, @floatFromInt(pv.y - @divTrunc(pic.height, 2) - 2));
            const border = self.mapobj.texIndex(if (free > 0) obj_res_border else obj_res_guard_border);
            gfx.drawTex(border, bx, by);
            gfx.drawTex(pic, bx + 2, by + 2);
            if (free == 0) gfx.drawTex2(border, bx, by, half);
            return;
        }
        if (eco.resource(pv, -1) <= 0 or !eco.compareBldParam1(@intCast(self.player), ab_hidden_resource, pv.terrain)) return;
        const pic = self.mapobj.texIndex(obj_res_hidden) orelse return;
        const bx = ox + @as(f32, @floatFromInt(cx - @divTrunc(pic.width, 2) - 2));
        const by = oy + @as(f32, @floatFromInt(pv.y - @divTrunc(pic.height, 2) - 2));
        gfx.drawTex(self.mapobj.texIndex(obj_res_border), bx, by);
        gfx.drawTex(pic, bx + 2, by + 2);
    }

    fn heroInSlot(self: *const MainScreen, slot: u8) ?usize {
        for (self.shard.heroes.items, 0..) |*h, k| {
            if (h.owner == @as(i32, @intCast(self.player)) and h.slot == slot and h.alive()) return k;
        }
        return null;
    }

    /// Prov[]: the player's living heroes in province `p`.
    fn heroesIn(self: *const MainScreen, p: i32) usize {
        var n: usize = 0;
        for (self.shard.heroes.items) |*h| {
            if (h.owner == @as(i32, @intCast(self.player)) and h.alive() and h.province == p) n += 1;
        }
        return n;
    }

    /// The doll's top-left corner on the map.
    fn dollPos(self: *const MainScreen, h: *const hero_mod.Hero) ?rl.Vector2 {
        const img = self.dolls[@intCast(std.math.clamp(h.class1 - 1, 0, 3))] orelse return null;
        const pv = &self.shard.provinces[@intCast(h.province)];
        return .{ .x = @floatFromInt(pv.x - @divTrunc(img.width, 2)), .y = @floatFromInt(pv.y - img.height) };
    }

    /// arHeroMask.Check: the player's hero whose doll is under the pointer.
    fn heroAt(self: *const MainScreen, p: input.Pointer) ?usize {
        if (!p.hover or !self.overMap(p)) return null;
        for (self.shard.heroes.items, 0..) |*h, k| {
            if (h.owner != @as(i32, @intCast(self.player)) or !h.alive()) continue;
            const img = self.dolls[@intCast(std.math.clamp(h.class1 - 1, 0, 3))] orelse continue;
            const at = self.dollPos(h) orelse continue;
            const lx: i32 = @intFromFloat(@floor(self.mx + p.pos.x - at.x));
            const ly: i32 = @intFromFloat(@floor(self.my + p.pos.y - at.y));
            if (lx < 0 or ly < 0 or lx >= img.width or ly >= img.height) continue;
            if (rl.getImageColor(img, lx, ly).a > 0) return k;
        }
        return null;
    }

    /// A click on a doll (MainProcess.cpp:4831): the hero (Condition 20),
    /// or the list of the province's heroes (Condition 15).
    fn clickHero(self: *MainScreen, k: usize) void {
        self.hero_mode = .main;
        const prov = self.shard.heroes.items[k].province;
        const n = self.heroesIn(prov);
        if (n <= 1) return self.selectHero(k);
        self.list = .{ .prov = prov, .member = k };
        self.hero = null;
        self.selected = 0;
        self.walker.path.clearRetainingCapacity();
        self.scroll_up.on = false;
        self.scroll_down.on = n > bar_size;
        self.updateNext();
    }

    /// HeroList: the player's living heroes in `prov` by slot, without
    /// `exclude` (Exclude, the exchange list).
    fn listHeroes(self: *const MainScreen, prov: i32, exclude: ?usize, out: *[max_list]usize) []usize {
        const items = self.shard.heroes.items;
        var n: usize = 0;
        for (items, 0..) |*h, k| {
            if (h.owner != @as(i32, @intCast(self.player)) or !h.alive() or h.province != prov or n == out.len) continue;
            if (exclude != null and exclude.? == k) continue;
            var j = n;
            while (j > 0 and items[out[j - 1]].slot > h.slot) : (j -= 1) out[j] = out[j - 1];
            out[j] = k;
            n += 1;
        }
        return out[0..n];
    }

    /// The rows of the list shown from `shift` on.
    fn shownHeroes(self: *const MainScreen, l: HeroList, out: *[max_list]usize) []usize {
        const all = self.listHeroes(l.prov, null, out);
        const rest = all[@min(l.shift, all.len)..];
        return rest[0..@min(rest.len, bar_size)];
    }

    /// Condition 15 (MainProcess.cpp:4088): a row picks the hero, the
    /// arrows scroll the list.
    fn updateList(self: *MainScreen, f: *const input.Frame) void {
        const l = self.list.?;
        var buf: [max_list]usize = undefined;
        const shown = self.shownHeroes(l, &buf);
        for (&self.rows, 0..) |*m, i| {
            if (i >= shown.len) {
                m.state = .visible;
                continue;
            }
            if (m.check(f.pointer) == .click) return self.selectHero(shown[i]);
        }
        // ipHotkey -1/-2: the wheel steps the list while the arrow is active.
        const wheel_up = f.pointer.wheel > 0 and self.scroll_up.on;
        const wheel_down = f.pointer.wheel < 0 and self.scroll_down.on;
        if ((self.scroll_up.check(f) or wheel_up) and l.shift > 0) {
            self.list.?.shift -= 1;
            if (self.list.?.shift == 0) self.scroll_up.on = false;
            self.scroll_down.on = true;
        }
        if (self.scroll_down.check(f) or wheel_down) {
            self.list.?.shift += 1;
            const total = self.listHeroes(l.prov, null, &buf).len;
            if (self.list.?.shift + bar_size >= total) self.scroll_down.on = false;
            self.scroll_up.on = true;
        }
    }

    /// HeroList's panel (MainProcess.cpp:967): a frame, the small portrait
    /// and the name per row, the highlight of the row under the pointer.
    fn drawHeroList(self: *MainScreen, c: *Ctx, l: HeroList) void {
        var buf: [max_list]usize = undefined;
        self.drawHeroRows(c, self.shownHeroes(l, &buf));
    }

    fn drawHeroRows(self: *MainScreen, c: *Ctx, shown: []const usize) void {
        const bx: f32 = bar_x;
        const by: f32 = bar_y;
        gfx.drawTex(self.iface.texIndex(gui_build_bar), bx, by);
        for (shown, 0..) |k, i| {
            const h = &self.shard.heroes.items[k];
            const dy: f32 = @floatFromInt(i * 59);
            gfx.drawTex(self.iface.texIndex(gui_raze_panel), bx + 50, by + 39 + dy);
            gfx.drawTex(self.smallport.texIndex(@intCast(@max(h.portrait, 0))), bx + 52, by + 41 + dy);
            c.font1.drawCentered(h.nameText(c.proto), bx + 193, by + 48 + dy, color1);
            if (self.rows[i].hot()) gfx.drawTex(self.iface.texIndex(gui_build_select), self.rows[i].x, self.rows[i].y);
        }
        self.scroll_up.draw(&self.buttons, c.tick);
        self.scroll_down.draw(&self.buttons, c.tick);
    }

    /// EADOR_BATTLE: the first hero of the player attacks the guard of a
    /// neighbouring neutral province (to look at the battle screen).
    pub fn debugBattle(self: *MainScreen, auto: bool) bool {
        self.auto_battle = auto;
        for (self.shard.heroes.items, 0..) |*h, k| {
            if (h.owner != @as(i32, @intCast(self.player)) or !h.alive()) continue;
            for (self.shard.provinces[@intCast(h.province)].neighbourList()) |nb| {
                const pv = &self.shard.provinces[@intCast(nb)];
                if (pv.player != 0 or pv.guard < 0 or !province.terrain.land(pv.terrain)) continue;
                h.target = nb;
                self.pending_battle = k;
                return true;
            }
        }
        return false;
    }

    /// The events of the turn change: the answer of the last dialog goes
    /// back to the runner, and the next dialog opens over the map centred
    /// on its province (CentreMap(x + dx, y), objEvent, DialogDX).  When
    /// the runner is done, the changed provinces are redrawn.
    fn pumpEvents(self: *MainScreen, c: *Ctx) ?scene.Action {
        const ev = if (self.events) |*x| x else return null;
        if (self.event_answer) |a| {
            self.event_answer = null;
            ev.answered(a -| 1);
        }
        if (ev.next()) |sh| {
            self.event_prov = sh.prov;
            self.centre(self.shard.provinces[@intCast(sh.prov)].x + sh.dx, self.shard.provinces[@intCast(sh.prov)].y);
            const sc = Dialog.createWith(c, sh.dialog, sh.enabled, &self.event_answer, .{ .text = sh.text(), .dx = sh.dx, .treasure = sh.treasure }) catch |e| {
                std.log.err("event dialog {d}: {t}", .{ sh.dialog, e });
                // Skip it as if the first open answer were taken.
                self.event_answer = (std.mem.indexOfScalar(bool, &sh.enabled, true) orelse 0) + 1;
                return null;
            };
            return .{ .push = sc };
        }
        self.event_prov = 0;
        self.eventsDone(c);
        return null;
    }

    /// The end of the events: the pictures of the provinces they changed
    /// (DrawPic) and the map state that depends on them.
    fn eventsDone(self: *MainScreen, c: *Ctx) void {
        var ev = self.events orelse return;
        self.events = null;
        defer ev.deinit();
        for (ev.touched.items) |p| self.pics[@intCast(p)].level = -1;
        self.drawPics(c);
        self.makeFog(c) catch {};
        if (self.hero) |k| if (!self.shard.heroes.items[k].alive()) {
            self.hero = null;
        };
        self.refreshRoute();
        self.updateNext();
    }

    /// EADOR_EVENT=N: StaticEvent(N) in the selected province or the
    /// capital, as the "U" key of the original's debug build.
    pub fn debugEvent(self: *MainScreen, c: *Ctx, n: i32) void {
        const prov = if (self.selected > 0) self.selected else self.shard.capitals[self.player];
        var ev = self.newRunner(c);
        ev.add(self.player, prov, n) catch return;
        self.events = ev;
    }

    fn newRunner(self: *MainScreen, c: *Ctx) event_mod.Runner {
        return .{ .gpa = c.gpa, .pr = c.proto, .s = &self.shard, .r = &c.rand, .turn = self.turn, .possibility = c.cfg.event_possibility, .static = c.cfg.static_event };
    }

    /// Condition 20 → 21: the move map and the route of the hero's order.
    fn selectHero(self: *MainScreen, k: usize) void {
        self.hero_mode = .main;
        self.list = null;
        self.hero = k;
        self.selected = 0;
        self.refreshRoute();
        self.updateNext();
    }

    fn refreshRoute(self: *MainScreen) void {
        const k = self.hero orelse return;
        const h = &self.shard.heroes.items[k];
        self.walker.forHero(h);
        self.walker.path.clearRetainingCapacity();
        if (h.task == hero_mod.Task.go) self.walker.route(self.shard.gpa, h.province, h.target) catch {};
    }

    /// buttNextHero is active while another living hero exists.
    fn updateNext(self: *MainScreen) void {
        var any = false;
        for (self.shard.heroes.items, 0..) |*h, k| {
            if (h.owner == @as(i32, @intCast(self.player)) and h.alive() and (self.hero == null or self.hero.? != k)) any = true;
        }
        self.next.on = any;
    }

    /// buttNextHero (MainProcess.cpp:5775): the next living hero by slot.
    fn nextHero(self: *MainScreen) void {
        if (!self.next.on) return;
        const pl: i32 = @intCast(self.player);
        const cur_slot: i32 = if (self.hero) |k| self.shard.heroes.items[k].slot else if (self.list) |l| self.shard.heroes.items[l.member].slot else -1;
        var pick: ?usize = null;
        var best: i32 = 99;
        var first: ?usize = null;
        var first_slot: i32 = 99;
        for (self.shard.heroes.items, 0..) |*h, k| {
            if (h.owner != pl or !h.alive()) continue;
            const sl: i32 = h.slot;
            if (sl > cur_slot and sl < best) {
                best = sl;
                pick = k;
            }
            if (sl < first_slot) {
                first_slot = sl;
                first = k;
            }
        }
        const k = pick orelse first orelse return;
        self.selectHero(k);
        const pv = &self.shard.provinces[@intCast(self.shard.heroes.items[k].province)];
        self.centre(pv.x, pv.y);
    }

    /// The secondary button with a hero selected (MainProcess.cpp:4959):
    /// a "go" order to the province under the pointer.
    fn order(self: *MainScreen, c: *Ctx, p: input.Pointer) scene.Action {
        const k = self.hero orelse return .none;
        const target = self.provinceAt(p);
        if (target <= 0 or self.fog[@intCast(target)] == 2) return .none;
        const h = &self.shard.heroes.items[k];
        const pv = &self.shard.provinces[@intCast(target)];
        if (pv.terrain == province.terrain.water) return info(c, pv.name, msg_water, stuff_water);
        if (pv.terrain == province.terrain.mountain) return info(c, pv.name, msg_mountain, stuff_mountain);
        const pl: i32 = @intCast(self.player);
        if (pv.player != pl and self.shard.provinces[@intCast(h.province)].player != pl) {
            if (std.mem.indexOfScalar(i32, pv.neighbourList(), h.province) != null) return self.dialog(c, dlg_no_attack);
        }
        if (self.walker.move[@intCast(target)] <= 0) return self.dialog(c, dlg_unreachable);
        if (h.province == target) {
            h.task = hero_mod.Task.none;
        } else {
            h.task = hero_mod.Task.go;
            h.target = target;
        }
        self.refreshRoute();
        return .none;
    }

    /// WindowMenu's answer, as MainProcess.cpp:5894 and RunGame
    /// (GameMenu.cpp:282) take it.
    fn afterMenu(self: *MainScreen, c: *Ctx) ?scene.Action {
        const gt = newsave.gameType(&self.shard);
        switch (self.menu_out.result) {
            .none => {
                // bSett: Condition 0, the hero list goes back to the capital.
                if (self.menu_out.redraw) {
                    if (self.list != null) {
                        self.list = null;
                        self.selected = self.shard.capitals[self.player];
                    } else if (self.hero == null) self.selected = 0;
                }
                return null;
            },
            // iExitFlag 0: SaveGame(22000) without bAutoSave.
            .exit => {
                self.saveGame(c, false) catch |e| std.log.err("save game: {t}", .{e});
                c.saves_changed = true;
                return .pop;
            },
            // iExitFlag -1: LoadGame of the turn's autosave.
            .past => return .{ .replace = Loading.create(c, gt) catch |e| {
                std.log.err("loading screen: {t}", .{e});
                return .pop;
            } },
            // Lose of a single game: iExitFlag 1, the save is deleted.
            // ScreenEndGame and WindowHighScore are not ported.
            .surrender => {
                const n = c.human.fileIndex();
                var pb: [64]u8 = undefined;
                if (std.fmt.bufPrint(&pb, "save/game{d}.sav", .{n})) |f| c.fs.delete(f) catch {} else |_| {}
                if (std.fmt.bufPrint(&pb, "save/map{d}.map", .{n})) |f| c.fs.delete(f) catch {} else |_| {}
                c.saves_changed = true;
                return .pop;
            },
        }
    }

    /// RunGame's SaveGame(4000) before the turn changes (GameMenu.cpp:227):
    /// bAutoSave set, iActivePlayer the player who ended the turn.
    fn autoSave(self: *MainScreen, c: *Ctx) !void {
        return self.saveGame(c, true);
    }

    /// SaveGame (GameMenu.cpp:335).  From the second save of the session
    /// on (bTimeShift) the old file is copied to .bks / .bkh first.
    fn saveGame(self: *MainScreen, c: *Ctx, auto: bool) !void {
        const gt = newsave.gameType(&self.shard);
        // SaveGame: .bkc is the backup of a shard battle (iGameMode 2);
        // the astral map (.bka) is not ported.
        const ext: []const u8, const bak: []const u8 = switch (gt) {
            1 => .{ "sav", "bks" },
            2 => .{ "cmp", "bkc" },
            3 => .{ "hot", "bkh" },
            else => return error.Unsupported,
        };
        const raw = try newsave.progressBytes(c.gpa, c.proto, &self.shard, .{
            .turn = self.turn,
            .active_player = @intCast(self.player),
            .auto_save = auto,
            .player = self.player,
            .fog = self.fog,
        });
        defer c.gpa.free(raw);
        const data = try packfile.packAllegro(c.gpa, raw);
        defer c.gpa.free(data);
        const n = c.human.fileIndex();
        var pb: [64]u8 = undefined;
        var qb: [64]u8 = undefined;
        const file = try std.fmt.bufPrint(&pb, "save/game{d}.{s}", .{ n, ext });
        if (self.time_shift and c.fs.exists(file)) {
            const old = try c.fs.read(file);
            defer c.gpa.free(old);
            try c.fs.write(try std.fmt.bufPrint(&qb, "save/game{d}.{s}", .{ n, bak }), old);
        }
        try c.fs.write(file, data);
    }

    fn castleExtra(self: *MainScreen) @import("castle.zig").Extra {
        return .{ .turn = self.turn, .faces = self.faces(), .out = &self.picked, .hero = &self.castle_hero };
    }

    fn market(self: *MainScreen, c: *Ctx, owner: i32) @import("../game/player.zig").Market {
        return .{ .s = &self.shard, .pr = c.proto, .player = owner };
    }

    /// A hero was hired or buried: the indices into Shard.heroes moved,
    /// so the selected hero and the list are dropped.
    fn checkHeroes(self: *MainScreen) void {
        const n = self.shard.heroes.items.len;
        if (n == self.known_heroes) return;
        self.known_heroes = n;
        self.hero = null;
        self.hover_hero = null;
        self.list = null;
        self.walker.path.clearRetainingCapacity();
        self.updateNext();
    }

    /// The map state after heroes died, moved or came back.
    fn afterHeroes(self: *MainScreen) void {
        self.checkHeroes();
        if (self.hero) |k| if (!self.shard.heroes.items[k].alive()) {
            self.hero = null;
        };
        self.refreshRoute();
        self.updateNext();
    }

    /// The dialogs of a hero's death, then those of the bodies that
    /// reached the capital (one at a time over the map).
    fn pumpHeroes(self: *MainScreen, c: *Ctx) ?scene.Action {
        if (self.death) |*d| {
            const m = self.market(c, self.shard.heroes.items[d.k].owner);
            switch (d.step(c, m)) {
                .push => |sc| return .{ .push = sc },
                .kept, .buried => {
                    self.death = null;
                    self.afterHeroes();
                },
            }
        }
        while (true) {
            if (self.arrival) |*a| {
                const m = self.market(c, self.shard.heroes.items[a.k].owner);
                if (a.step(c, m)) |sc| return .{ .push = sc };
                self.arrival = null;
                self.afterHeroes();
            }
            if (self.n_arrivals == 0) return null;
            self.n_arrivals -= 1;
            const want = self.arrivals[self.n_arrivals];
            for (self.shard.heroes.items, 0..) |*h, k| {
                if (h.owner == want[0] and h.slot == want[1] and h.dead == 0) {
                    self.arrival = .{ .k = k };
                    break;
                }
            }
        }
    }

    /// Member = the province, Condition 0, CentreMap.
    fn pickProvince(self: *MainScreen, prov: i32) void {
        self.picked = 0;
        self.hero_mode = .main;
        if (prov <= 0 or prov >= self.shard.provinces.len) return;
        self.selected = prov;
        self.hero = null;
        self.list = null;
        const pv = &self.shard.provinces[@intCast(prov)];
        self.centre(pv.x, pv.y);
    }

    /// GetName, GetAvatar and ShowHerb of every player for the statistics.
    pub fn faces(self: *const MainScreen) [mapgen.max_players + 1]statistics_scene.Face {
        var out: [mapgen.max_players + 1]statistics_scene.Face = @splat(.{});
        for (&out, &self.players, 0..) |*o, *p, k| {
            o.* = .{ .name = p.name(), .avatar = p.avatar, .color1 = p.color1, .color2 = p.color2, .sign = p.sign };
            // Control 3 (a local lord): DrawHerb(iColor-1, iColor-1, 0).
            if (k <= self.shard.n_players and self.shard.rulers[k].kind == .lord) {
                const col = self.shard.rulers[k].color - 1;
                o.* = .{ .name = "", .avatar = 16, .color1 = col, .color2 = col, .sign = 0 };
            }
        }
        return out;
    }

    /// buttStat (MainProcess.cpp:5876): ScreenStatistics.
    fn openStatistics(self: *MainScreen, c: *Ctx) scene.Action {
        self.picked = 0;
        return .{ .push = statistics_scene.Statistics.create(c, &self.shard, self.player, self.turn, self.faces(), &self.picked) catch |e| {
            std.log.err("statistics: {t}", .{e});
            return .none;
        } };
    }

    /// key[KEY_P] (MainProcess.cpp:5984): ScreenProvList.
    fn openProvList(self: *MainScreen, c: *Ctx) scene.Action {
        self.picked = 0;
        return .{ .push = ProvList.create(c, &self.shard, self.player, &self.picked) catch |e| {
            std.log.err("province list: {t}", .{e});
            return .none;
        } };
    }

    /// EADOR_SCREEN=stats|plist.
    pub fn debugStatistics(self: *MainScreen, c: *Ctx, list: bool) !scene.Scene {
        self.picked = 0;
        if (list) return ProvList.create(c, &self.shard, self.player, &self.picked);
        return statistics_scene.Statistics.create(c, &self.shard, self.player, self.turn, self.faces(), &self.picked);
    }

    /// buttCastle (MainProcess.cpp:5823): CastleMain, or dialog 388 while
    /// the capital is besieged.  A province picked in the statistics
    /// (CastleProv) is selected on return.  The hero tab is not ported.
    fn openCastle(self: *MainScreen, c: *Ctx) scene.Action {
        const cap = self.shard.capitals[self.player];
        if (cap < 1) return .none;
        if (movement.siege(&self.shard, cap) >= 0) return self.dialog(c, dlg_castle_siege);
        if (self.list != null) {
            self.list = null;
            self.selected = cap;
        }
        self.picked = 0;
        return .{ .push = CastleScreen.create(c, &self.shard, self.player, .build, self.castleExtra()) catch |e| {
            std.log.err("castle: {t}", .{e});
            return .none;
        } };
    }

    fn hasOuterAbility(pr: *const @import("../data/proto.zig").Proto, pv: *const province.Province, id: i32) bool {
        for (pv.buildings[0..province.max_bld]) |bld| {
            if (bld <= 0 or bld >= pr.outer_buildings.len) continue;
            const ob = &pr.outer_buildings[@intCast(bld)];
            for (ob.abilities[0..ob.n_abilities]) |a| if (a.id == id) return true;
        }
        return false;
    }

    /// buttShop of the hero panel (MainProcess.cpp:5682): shown in the
    /// player's provinces, active in the capital or where an outer
    /// building has a shop.  It opens the castle shop.
    fn updateShopButton(self: *MainScreen, c: *Ctx) void {
        const b = &self.shop_button;
        b.enabled = false;
        if (self.hero_mode != .main) return;
        const k = self.hero orelse return;
        const h = &self.shard.heroes.items[k];
        if (h.owner != @as(i32, @intCast(self.player))) return;
        b.enabled = true;
        b.on = false;
        const pv = &self.shard.provinces[@intCast(h.province)];
        if (pv.player != @as(i32, @intCast(self.player))) return;
        if (self.shard.capitals[self.player] == pv.number) b.on = true;
        for (pv.buildings[0..province.max_bld]) |bld| {
            if (bld > 0 and bld < c.proto.outer_buildings.len and c.proto.outer_buildings[@intCast(bld)].shop) b.on = true;
        }
    }

    /// buttLibrary of the hero panel (MainProcess.cpp:5683): shown in the
    /// player's provinces, active in the capital when the library has a
    /// spell or where an outer building has bLibrary.
    fn updateLibraryButton(self: *MainScreen, c: *Ctx) void {
        const b = &self.library_button;
        b.enabled = false;
        if (self.hero_mode != .main) return;
        const k = self.hero orelse return;
        const h = &self.shard.heroes.items[k];
        if (h.owner != @as(i32, @intCast(self.player))) return;
        b.enabled = true;
        b.on = false;
        const pv = &self.shard.provinces[@intCast(h.province)];
        if (pv.player != @as(i32, @intCast(self.player))) return;
        if (self.shard.capitals[self.player] == pv.number and self.shard.rulers[self.player].holdings.library[0] > 0) b.on = true;
        for (pv.buildings[0..province.max_bld]) |bld| {
            if (bld > 0 and bld < c.proto.outer_buildings.len and c.proto.outer_buildings[@intCast(bld)].library) b.on = true;
        }
    }

    fn openHeroLibrary(self: *MainScreen, c: *Ctx) scene.Action {
        const k = self.hero orelse return .none;
        return .{ .push = HeroLibrary.create(c, &self.shard, k, self.players[self.player].exp_pct) catch |e| {
            std.log.err("hero library: {t}", .{e});
            return .none;
        } };
    }

    fn openHeroShop(self: *MainScreen, c: *Ctx) scene.Action {
        const k = self.hero orelse return .none;
        const shop = &self.shard.rulers[self.player].holdings.shop;
        return .{ .push = HeroShop.create(c, &self.shard, k, shop, self.players[self.player].exp_pct) catch |e| {
            std.log.err("hero shop: {t}", .{e});
            return .none;
        } };
    }

    /// mskHeroPic or G: ScreenHero.
    fn openHeroScreen(self: *MainScreen, c: *Ctx) scene.Action {
        const k = self.hero orelse return .none;
        self.refresh_hero = true;
        return .{ .push = HeroScreen.create(c, &self.shard, k, self.players[self.player].exp_pct, true) catch |e| {
            std.log.err("hero screen: {t}", .{e});
            return .none;
        } };
    }

    pub const DebugHero = enum { hero, garrison, shop, library, exchange, info };

    /// EADOR_SCREEN=hero|herogar|heroshop|herolib|heroswap: the first hero of
    /// the player; herogar is ScreenHeroGarrison with his province's garrison;
    /// heroswap is ScreenHeroHero with his second hero, who is moved to the
    /// first one's province.
    pub fn debugHero(self: *MainScreen, c: *Ctx, what: DebugHero) !scene.Scene {
        const k = for (self.shard.heroes.items, 0..) |h, i| {
            if (h.owner == @as(i32, @intCast(self.player)) and h.alive() and h.squad[0] != null) break i;
        } else return error.NoHero;
        self.selectHero(k);
        const exp = self.players[self.player].exp_pct;
        return switch (what) {
            .shop => HeroShop.create(c, &self.shard, k, &self.shard.rulers[self.player].holdings.shop, exp),
            .library => HeroLibrary.create(c, &self.shard, k, exp),
            .hero => HeroScreen.create(c, &self.shard, k, exp, true),
            .garrison => blk: {
                const g = self.shard.provinces[@intCast(self.shard.heroes.items[k].province)].garrison;
                if (g < 0) return error.NoGarrison;
                break :blk HeroGarrison.create(c, &self.shard, k, @intCast(g), exp);
            },
            .info => @import("army_info.zig").UnitWindow.createHero(c, &self.shard.heroes.items[k], exp),
            .exchange => blk: {
                const other = for (self.shard.heroes.items, 0..) |h, i| {
                    if (i != k and h.owner == @as(i32, @intCast(self.player)) and h.alive() and h.squad[0] != null) break i;
                } else return error.NoSecondHero;
                self.shard.heroes.items[other].province = self.shard.heroes.items[k].province;
                break :blk HeroHero.create(c, &self.shard, k, other, exp);
            },
        };
    }

    /// buttGarrison: ScreenGarrison over the map, centred 300 px right of
    /// the province (hire only without a siege, decided by ArmyPanel).
    fn openGarrison(self: *MainScreen, c: *Ctx) scene.Action {
        const pv = &self.shard.provinces[@intCast(self.selected)];
        if (pv.garrison < 0) return .none;
        self.centre(pv.x + 300, pv.y);
        return .{ .push = ProvinceGarrison.create(c, &self.shard, self.player, @intCast(pv.garrison)) catch |e| {
            std.log.err("garrison: {t}", .{e});
            return .none;
        } };
    }

    /// EADOR_SCREEN=castle: the castle of the player on tab `tab`.
    pub fn debugCastle(self: *MainScreen, c: *Ctx, tab: castle_ui.Tab) !scene.Scene {
        return CastleScreen.create(c, &self.shard, self.player, tab, self.castleExtra());
    }

    /// EADOR_BUILD=7,3,...: dcPlayer::Build of the listed buildings, free.
    pub fn debugBuild(self: *MainScreen, c: *Ctx, list: []const u8) void {
        var it = std.mem.tokenizeScalar(u8, list, ',');
        while (it.next()) |t| {
            const id = std.fmt.parseInt(i32, t, 10) catch continue;
            const r = &self.shard.rulers[self.player];
            const gold = r.gold;
            const gem = r.gem;
            castle.build(&self.shard, c.proto, &c.rand, self.player, id) catch |e| std.log.err("build {d}: {t}", .{ id, e });
            r.gold = gold;
            r.gem = gem;
        }
    }

    /// EADOR_SCREEN=garrison: the capital's garrison as from the map.
    pub fn debugGarrison(self: *MainScreen, c: *Ctx) !scene.Scene {
        const cap = self.shard.capitals[self.player];
        const g = self.shard.provinces[@intCast(cap)].garrison;
        if (g < 0) return error.NoGarrison;
        self.selected = cap;
        const pv = &self.shard.provinces[@intCast(cap)];
        self.centre(pv.x + 300, pv.y);
        return ProvinceGarrison.create(c, &self.shard, self.player, @intCast(g));
    }

    /// EADOR_PANEL: province `prov` (0: the capital) on page `mode`.
    pub fn debugPanel(self: *MainScreen, c: *Ctx, mode: []const u8, prov: i32) void {
        _ = c;
        const p = if (prov > 0 and prov < self.shard.provinces.len) prov else self.shard.capitals[self.player];
        self.pickProvince(p);
        self.panel.reset();
        self.panel.prov = p;
        self.panel.mode = std.meta.stringToEnum(@import("prov_panel.zig").Mode, mode) orelse .main;
    }

    fn panelView(self: *MainScreen) @import("prov_panel.zig").View {
        return .{ .s = &self.shard, .player = self.player, .fog = self.fog, .players = &self.players, .arms = &self.arms, .iface = &self.iface, .buttons = &self.buttons, .hint = &self.hint };
    }

    /// The province panel and what its buttons ask of the map.
    fn updatePanel(self: *MainScreen, c: *Ctx, f: *const input.Frame) ?scene.Action {
        switch (self.panel.update(c, self.panelView(), self.selected, f)) {
            .none => return null,
            .action => |a| return a,
            .castle => return self.openCastle(c),
            .garrison => return self.openGarrison(c),
            .cast => |idx| return self.castRitualAt(c, idx, self.selected, null),
            .changed => {
                self.pics[@intCast(self.selected)].level = -1;
                self.drawPics(c);
                return null;
            },
        }
    }

    /// WindowInfo(title, pFirstMessage[msg], bmpStuff[pic]).
    fn info(c: *Ctx, title: []const u8, msg: usize, pic: usize) scene.Action {
        const body = if (msg < c.proto.messages.len) c.proto.messages[msg] else "";
        return .{ .push = WindowInfo.create(c, title, body, pic) catch return .none };
    }

    fn dialog(self: *MainScreen, c: *Ctx, index: usize) scene.Action {
        return .{ .push = Dialog.create(c, index, .{ true, false, false, false }, &self.dialog_result) catch return .none };
    }

    /// "End turn": the heroes' orders and the attitude timers
    /// (EndMainTurn), the next turn, then BeginMainTurn's income, growth,
    /// upkeep and healing for every player in the game, and MakeFog.
    /// The province events are queued (event.zig) and their dialogs shown
    /// by pumpEvents after the turn change, so their effects come after
    /// the income of the turn.  The computer players' moves are not ported.
    fn endTurn(self: *MainScreen, c: *Ctx) void {
        self.autoSave(c) catch |e| std.log.err("autosave: {t}", .{e});
        self.time_shift = true;
        self.list = null;
        for (0..self.shard.heroes.items.len) |k| {
            const out = movement.endTurnMove(c.gpa, &self.walker, &self.shard, k) catch continue;
            const h = &self.shard.heroes.items[k];
            if (out != .attack or self.pending_battle != null or h.owner != @as(i32, @intCast(self.player))) continue;
            const t = &self.shard.provinces[@intCast(h.target)];
            if (t.player == 0 and t.guard >= 0) self.pending_battle = k;
        }
        // The orders of the heroes (EndMainTurn): dialogs shown after the
        // turn change; the ritual cooldowns go down every turn.
        ritual.turnCooldowns(&self.shard, c.proto);
        self.queueHeroTasks(c);
        // BeginMainTurn takes a turn off iDead: the bodies arriving now get
        // dialog 1239 for a human (in reverse, as the queue pops).
        self.n_arrivals = 0;
        var it = std.mem.reverseIterator(self.shard.heroes.items);
        while (it.nextPtr()) |h| {
            if (h.dead != 1 or h.owner < 1 or self.shard.rulers[@intCast(h.owner)].kind != .human) continue;
            if (!self.shard.in_game[@intCast(h.owner)] or self.n_arrivals == self.arrivals.len) continue;
            self.arrivals[self.n_arrivals] = .{ h.owner, h.slot };
            self.n_arrivals += 1;
        }
        var eco: economy.Economy = .{ .s = &self.shard, .pr = c.proto, .max_profit = c.cfg.max_profit };
        eco.endTurn();
        castle.mercenaryTurn(&self.shard, c.proto, &c.rand, c.cfg.mercenary_time);
        self.turn += 1;
        if (self.events) |*old| old.deinit();
        self.events = self.newRunner(c);
        const ev = &self.events.?;
        eco.events = ev;
        for (1..self.shard.n_players + 1) |k| {
            if (!self.shard.in_game[k]) continue;
            // BeginMainTurn: the timed events of a human (pStory), then one
            // castle building a turn if anything can be built.
            if (self.shard.rulers[k].kind == .human) ev.storyTurn(k) catch {};
            castle.resetTurn(&self.shard, c.proto, k) catch {};
            eco.beginTurn(k, self.turn);
        }
        self.makeFog(c) catch {};
        // DrawPic of the towns that changed level.
        self.drawPics(c);
        if (self.hero) |k| {
            if (!self.shard.heroes.items[k].alive()) self.hero = null;
        }
        self.refreshRoute();
        self.updateNext();
    }

    /// ShowMap's heroes: for each player, a doll with its shadow and the
    /// banner in each explored province holding his living heroes, a
    /// second banner for several of them.  The doll under the pointer is
    /// the active one (bmpHeroDollS).
    fn drawHeroes(self: *MainScreen, ox: f32, oy: f32, what: enum { banners, dolls }) void {
        const half: rl.Color = .{ .r = 255, .g = 255, .b = 255, .a = 128 };
        const heroes = self.shard.heroes.items;
        const pass: usize = if (what == .banners) 0 else 1;
        for (heroes, 0..) |*h, k| {
            if (!h.alive() or self.fog[@intCast(h.province)] != 0) continue;
            const first = for (heroes[0..k]) |*o| {
                if (o.owner == h.owner and o.alive() and o.province == h.province) break false;
            } else true;
            if (!first) continue;
            const pv = &self.shard.provinces[@intCast(h.province)];
            const cls: usize = @intCast(std.math.clamp(h.class1 - 1, 0, 3));
            if (pass == 0) {
                const pl = self.players[@intCast(std.math.clamp(h.owner, 0, mapgen.max_players))];
                const fx = ox + @as(f32, @floatFromInt(pv.x - heraldry.flag_w / 2 - 3));
                const fy = oy + @as(f32, @floatFromInt(pv.y - heraldry.flag_h));
                self.arms.drawFlag(pl.color1, pl.color2, fx, fy);
                var n: usize = 0;
                for (heroes) |*o| n += @intFromBool(o.owner == h.owner and o.alive() and o.province == h.province);
                if (n > 1) self.arms.drawFlag(pl.color1, pl.color2, fx + 4, fy - 5);
                continue;
            }
            const at = self.dollPos(h) orelse continue;
            gfx.drawTex2(self.mapobj.texIndex(obj_doll_shadow + cls), ox + at.x, oy + at.y, half);
            const active = self.hover_hero != null and heroes[self.hover_hero.?].province == h.province and h.owner == @as(i32, @intCast(self.player));
            gfx.drawTex(self.mapobj.texIndex((if (active) obj_doll_active else obj_doll) + cls), ox + at.x, oy + at.y);
        }
    }

    /// The route of the selected hero (MainProcess.cpp:5584): a dot on
    /// each province and between them, green on the first turn and red
    /// later, and the target mark.
    fn drawPath(self: *MainScreen, ox: f32, oy: f32) void {
        if (self.hero == null) return;
        const steps = self.walker.path.items;
        for (steps, 0..) |st, i| {
            const pv = &self.shard.provinces[@intCast(st.province)];
            if (i + 1 < steps.len) {
                const nx = &self.shard.provinces[@intCast(steps[i + 1].province)];
                centredAt(self.mapobj.texIndex(obj_path + @as(usize, @intFromBool(st.turn != 1))), pv.x, pv.y, ox, oy);
                centredAt(self.mapobj.texIndex(obj_path + @as(usize, @intFromBool(steps[i + 1].turn != 1))), @divTrunc(pv.x + nx.x, 2), @divTrunc(pv.y + nx.y, 2), ox, oy);
            } else {
                centredAt(self.mapobj.texIndex(obj_path + 2 + @as(usize, @intFromBool(st.turn != 1))), pv.x, pv.y, ox, oy);
            }
        }
    }

    /// PlaceOnMap(x - w/2, y - h/2 - dy).
    fn centredAbove(t: ?rl.Texture2D, pv: province.Province, ox: f32, oy: f32, dy: i32) void {
        const tx = t orelse return;
        gfx.drawTex(tx, ox + @as(f32, @floatFromInt(pv.x - @divTrunc(tx.width, 2))), oy + @as(f32, @floatFromInt(pv.y - @divTrunc(tx.height, 2) - dy)));
    }

    /// PlaceOnMap(x - bmpPath[0]->w/2, y - h/2): the original centres all
    /// four path pictures by the first one's size only for the dots; the
    /// target is centred by its own.
    fn centredAt(t: ?rl.Texture2D, x: i32, y: i32, ox: f32, oy: f32) void {
        const tx = t orelse return;
        gfx.drawTex(tx, ox + @as(f32, @floatFromInt(x - @divTrunc(tx.width, 2))), oy + @as(f32, @floatFromInt(y - @divTrunc(tx.height, 2))));
    }

    /// The hero panel (Condition 21, MainProcess.cpp:5517): portrait,
    /// arms, name, class, leadership/magic/health, initiative/siege/
    /// mobility, life and experience bars and the order.
    fn drawHeroBar(self: *MainScreen, c: *Ctx, k: usize) void {
        const pr = c.proto;
        const h = &self.shard.heroes.items[k];
        const bx: f32 = bar_x;
        const by: f32 = bar_y;
        gfx.drawTex(self.iface.texIndex(gui_hero_bar), bx, by);
        gfx.drawTex(self.smallport.texIndex(@intCast(@max(h.portrait, 0))), bx + 133, by + 39);
        const pl = self.players[@intCast(std.math.clamp(h.owner, 0, mapgen.max_players))];
        self.arms.drawHerb(pl.color1, pl.color2, pl.sign, bx + 8, by + 60);
        self.arms.drawHerb(pl.color1, pl.color2, pl.sign, bx + 273, by + 60);
        c.font1.drawCentered(h.nameText(pr), bx + 160, by + 103, color1);
        c.font1.drawCentered(pr.classes[h.className()].name, bx + 160, by + 130, color1);
        var b: [64]u8 = undefined;
        const num = struct {
            fn f(buf: []u8, v: i32) []const u8 {
                return std.fmt.bufPrint(buf, "{d}", .{v}) catch "";
            }
        }.f;
        c.font1.drawCentered(num(&b, h.leader), bx + 108, by + 165, color2);
        c.font1.drawCentered(num(&b, h.magic), bx + 170, by + 165, color2);
        c.font1.drawCentered(num(&b, h.health), bx + 229, by + 165, color2);
        c.font1.drawCentered(num(&b, h.heroBonus(pr, hero_mod.bonus.initiative)), bx + 108, by + 199, color2);
        c.font1.drawCentered(num(&b, h.siegePower(pr, movement.castleParam(&self.shard, pr, h.player, movement.abil_siege, true))), bx + 170, by + 199, color2);
        const here = &self.shard.provinces[@intCast(h.province)];
        const mob = h.mobility(pr, movement.castleParam(&self.shard, pr, h.player, movement.abil_mobility, true));
        const mob_text = if (here.player != @as(i32, @intCast(self.player)))
            "1"
        else if (movement.stable(&self.shard, pr, h.province) > 0)
            std.fmt.bufPrint(&b, "{d}+{d}", .{ mob, movement.stable(&self.shard, pr, h.province) }) catch ""
        else
            num(&b, mob);
        c.font1.drawCentered(mob_text, bx + 229, by + 199, color2);
        c.font1.drawCentered(std.fmt.bufPrint(&b, "{s}:", .{pr.phrase(ph_order)}) catch "", bx + 158, by + 248, color1);
        // Life bar (+195) and experience bar (+73), 50 pixels each.
        const life = h.life(pr);
        const lbar = @min(@divTrunc(50 * life[0], @max(life[1], 1)), 50);
        var i: i32 = 0;
        while (i < lbar) : (i += 1) {
            rl.drawRectangle(@intFromFloat(bx + 195 + @as(f32, @floatFromInt(i))), @intFromFloat(by + 69), 1, 23, .{ .r = @intCast(100 + i * 3), .g = @intCast(20 + @divTrunc(i * 5, 20)), .b = @intCast(20 + @divTrunc(i * 5, 50)), .a = 255 });
        }
        c.font1.drawCentered(num(&b, life[0]), bx + 221, by + 77, color1);
        c.font1.drawCentered(num(&b, life[0]), bx + 220, by + 76, color2);
        const e0 = h.expForLevel(pr, h.level, pl.exp_pct);
        const e1 = h.expForLevel(pr, h.level + 1, pl.exp_pct);
        const ebar = std.math.clamp(@divTrunc(50 * (h.exp - e0), @max(e1 - e0, 1)), 0, 50);
        i = 0;
        while (i < ebar) : (i += 1) {
            rl.drawRectangle(@intFromFloat(bx + 73 + @as(f32, @floatFromInt(i))), @intFromFloat(by + 69), 1, 23, .{ .r = @intCast(80 + @divTrunc(i * 27, 10)), .g = @intCast(10 + @divTrunc(i * 5, 20)), .b = @intCast(100 + i * 3), .a = 255 });
        }
        c.font1.drawCentered(num(&b, h.level), bx + 99, by + 77, color1);
        c.font1.drawCentered(num(&b, h.level), bx + 98, by + 76, color2);
        if (h.task >= hero_mod.Task.go and h.task <= hero_mod.Task.attack) {
            c.font1.drawCentered(pr.phrase(@intCast(ph_task_first + h.task - 1)), bx + 150, by + 275, color1);
        } else if (here.player == @as(i32, @intCast(self.player))) {
            c.font1.drawCentered(pr.phrase(ph_no_order), bx + 150, by + 275, color1);
        } else {
            c.font1.drawCentered(pr.phrase(ph_siege), bx + 150, by + 275, color1);
        }
        if (h.task == hero_mod.Task.go and h.target > 0 and h.target < self.shard.provinces.len) {
            c.font1.drawCentered(self.shard.provinces[@intCast(h.target)].name, bx + 150, by + 295, color1);
        } else if (h.task == hero_mod.Task.enter and h.target >= 0 and h.target < province.max_site) {
            const site = here.sites[@intCast(h.target)].index;
            if (site > 0 and site < pr.sites.len) c.font1.drawCentered(pr.sites[@intCast(site)].name, bx + 150, by + 295, color1);
        }
    }

    /// GetLevel.
    fn levelOf(self: *const MainScreen, pop: i32) i32 {
        const lv = self.levels;
        for (1..lv.len - 1) |i| if (pop <= lv[i].max_population) return @intCast(i);
        return @intCast(lv.len - 1);
    }

    // -- the hero panel's orders (Condition 21) ---------------------------

    /// The sites the hero may enter: explored, with a guard or bCanEnter.
    fn siteSlots(pv: *const province.Province, pr: *const @import("../data/proto.zig").Proto, out: *[province.max_site]usize) []usize {
        var n: usize = 0;
        for (pv.sites, 0..) |st, i| {
            if (st.index <= 0 or st.index >= pr.sites.len or st.pos > pv.explore) continue;
            if (pv.site_guards[i] >= 0) {
                out[n] = i;
                n += 1;
                continue;
            }
            const site = &pr.sites[@intCast(st.index)];
            if (!site.can_enter) continue;
            out[n] = i;
            n += 1;
        }
        return out[0..n];
    }

    fn hasSites(self: *const MainScreen, c: *Ctx, pv: *const province.Province) bool {
        _ = self;
        var buf: [province.max_site]usize = undefined;
        return siteSlots(pv, c.proto, &buf).len > 0;
    }

    /// Condition 21: the buttons of the selected hero.
    fn updateHeroPanel(self: *MainScreen, c: *Ctx, f: *const input.Frame) ?scene.Action {
        const k = self.hero orelse return null;
        const h = &self.shard.heroes.items[k];
        const pr = c.proto;
        const pv = &self.shard.provinces[@intCast(h.province)];
        const own = pv.player == @as(i32, @intCast(self.player));
        const has_task = h.task >= hero_mod.Task.go and h.task <= hero_mod.Task.attack;
        const orders = own and !has_task;
        const no_dismiss = pv.defender > 0 and pv.defender < pr.defenders.len and pr.defenders[@intCast(pv.defender)].no_dismiss;

        self.explore_button.enabled = orders;
        self.explore_button.on = pv.explore < 100;
        self.enter_button.enabled = orders;
        self.enter_button.on = self.hasSites(c, pv);
        self.pillage_button.enabled = orders and !no_dismiss;
        self.pillage_button.on = pv.population > 0;
        self.attack_button.enabled = orders and no_dismiss;
        self.attack_button.on = no_dismiss;
        self.skip_button.enabled = own and has_task;
        self.skip_button.on = true;
        self.ritual_button.enabled = own;
        self.ritual_button.on = true;
        self.hero_garr_button.enabled = own;
        self.hero_garr_button.on = pv.garrison >= 0 and (self.shard.capitals[self.player] == pv.number or hasOuterAbility(pr, pv, ab_outer_garrison));
        self.shop_button.enabled = own;
        self.library_button.enabled = own;
        // Active while the province holds another hero (Prov[] != 1).
        self.exchange_button.on = self.heroesIn(h.province) > 1;

        if (self.explore_button.check(f)) {
            h.task = hero_mod.Task.explore;
            h.target = 0;
            self.refreshRoute();
            return null;
        }
        if (self.enter_button.check(f)) {
            self.hero_mode = .sites;
            self.site_shift = 0;
            self.scroll_up.on = false;
            return null;
        }
        if (self.pillage_button.check(f)) {
            h.task = hero_mod.Task.pillage;
            h.target = 0;
            self.refreshRoute();
            return null;
        }
        if (self.attack_button.check(f)) {
            h.task = hero_mod.Task.attack;
            h.target = 0;
            self.refreshRoute();
            return null;
        }
        if (self.ritual_button.check(f)) {
            self.hero_mode = .rituals;
            self.ritual_shift = 0;
            self.scroll_up.on = false;
            return null;
        }
        if (self.skip_button.check(f)) {
            h.task = hero_mod.Task.none;
            h.target = 0;
            self.refreshRoute();
            return null;
        }
        if (self.hero_garr_button.check(f)) return self.openHeroGarrison(c);
        if (self.exchange_button.check(f)) {
            var buf: [max_list]usize = undefined;
            const others = self.listHeroes(h.province, k, &buf);
            if (others.len == 1) return self.openExchange(c, others[0]);
            if (others.len > 1) {
                self.hero_mode = .exchange;
                self.exchange_shift = 0;
            }
            return null;
        }

        if (self.hero_pic_shape) |*img| self.hero_pic.shape = img;
        if (self.hero_pic.check(f.pointer) == .click or f.key(.g)) return self.openHeroScreen(c);
        if (self.hero_pic.hot()) self.hint.phrase(c, hint_hero_pic, f.pointer.pos);
        return null;
    }

    /// Condition 17: the list of sites to enter.
    fn updateSiteList(self: *MainScreen, c: *Ctx, f: *const input.Frame) ?scene.Action {
        const k = self.hero orelse return null;
        const h = &self.shard.heroes.items[k];
        const pv = &self.shard.provinces[@intCast(h.province)];
        var buf: [province.max_site]usize = undefined;
        const all = siteSlots(pv, c.proto, &buf);
        if (self.cancel_button.check(f)) {
            self.hero_mode = .main;
            return null;
        }
        if (self.site_shift > all.len) self.site_shift = all.len;
        const shown = all[@min(self.site_shift, all.len)..];
        for (&self.rows, 0..) |*m, i| {
            if (i >= shown.len) {
                m.state = .visible;
                continue;
            }
            m.shape = if (self.build_mask) |*img| img else null;
            if (m.check(f.pointer) == .click) {
                h.task = hero_mod.Task.enter;
                h.target = @intCast(shown[i]);
                self.hero_mode = .main;
                self.refreshRoute();
                return null;
            }
        }
        if ((self.scroll_up.check(f)) and self.site_shift > 0) {
            self.site_shift -= 1;
            self.scroll_up.on = self.site_shift > 0;
            self.scroll_down.on = true;
        }
        if (self.scroll_down.check(f)) {
            self.site_shift += 1;
            if (self.site_shift + bar_size >= all.len) self.scroll_down.on = false;
            self.scroll_up.on = true;
        }
        self.scroll_up.enabled = true;
        self.scroll_down.enabled = true;
        self.scroll_up.on = self.site_shift > 0;
        self.scroll_down.on = self.site_shift + bar_size < all.len;
        return null;
    }

    /// Conditions 18 and 19: the hero's rituals and the chosen one.
    fn updateRitualPage(self: *MainScreen, c: *Ctx, f: *const input.Frame) ?scene.Action {
        const k = self.hero orelse return null;
        const h = &self.shard.heroes.items[k];
        const pr = c.proto;
        const m = self.market(c, @intCast(self.player));
        if (self.hero_mode == .ritual) {
            const idx = self.ritual_choice;
            const cost = ritual.price(m, idx);
            const cd = ritual.cooldown(&self.shard, pr, self.player, idx);
            const can = ritual.canCast(&self.shard, pr, h.province, idx, @intCast(self.player));
            self.ritual_cast.on = can and cd == 0 and m.affords(cost) and m.holdings().can_cast > 0;
            if (self.ritual_cast.check(f)) return self.castRitual(c, idx);
            if (self.ritual_cancel.check(f)) {
                self.hero_mode = .rituals;
                return null;
            }
            return null;
        }
        if (self.cancel_button.check(f)) {
            self.hero_mode = .main;
            return null;
        }
        var buf: [64]i32 = undefined;
        const all = ritual.heroRituals(&self.shard, pr, self.player, &buf);
        if (self.ritual_shift > all.len) self.ritual_shift = all.len;
        const shown = all[@min(self.ritual_shift, all.len)..];
        for (&self.rows, 0..) |*mk, i| {
            if (i >= shown.len) {
                mk.state = .visible;
                continue;
            }
            mk.shape = if (self.build_mask) |*img| img else null;
            const r = mk.check(f.pointer);
            if (r == .click) {
                self.ritual_choice = shown[i];
                self.hero_mode = .ritual;
                return null;
            }
            if (r == .right_click) {
                return .{ .push = info_windows.CostWindow.ritual(c, shown[i], m) catch return .none };
            }
        }
        if ((self.scroll_up.check(f)) and self.ritual_shift > 0) self.ritual_shift -= 1;
        if (self.scroll_down.check(f)) self.ritual_shift += 1;
        self.scroll_up.enabled = true;
        self.scroll_down.enabled = true;
        self.scroll_up.on = self.ritual_shift > 0;
        self.scroll_down.on = self.ritual_shift + bar_size < all.len;
        return null;
    }

    /// ButtRitual2 of the hero panel: CastRitual of the chosen ritual.
    fn castRitual(self: *MainScreen, c: *Ctx, idx: i32) scene.Action {
        const k = self.hero orelse return .none;
        return self.castRitualAt(c, idx, self.shard.heroes.items[k].province, k);
    }

    /// CastRitual(idx, prov, hero), the sound, then the event and dialog
    /// of the ritual.
    fn castRitualAt(self: *MainScreen, c: *Ctx, idx: i32, prov: i32, hero: ?usize) scene.Action {
        const pr = c.proto;
        const m = self.market(c, @intCast(self.player));
        const res = ritual.cast(m, idx, prov, hero, &c.rand) catch |e| {
            std.log.err("ritual {d}: {t}", .{ idx, e });
            return .none;
        };
        const snd = pr.rituals[@intCast(idx)].sound;
        if (snd >= 0) sound.play(.spell, @intCast(snd)) else sound.play(.unit, @intCast(-snd));
        self.hero_mode = .main;
        self.refreshRoute();
        self.pics[@intCast(prov)].level = -1;
        self.drawPics(c);
        if (res.event) |ev| {
            if (self.events == null) self.events = self.newRunner(c);
            if (self.events) |*r| r.add(self.player, prov, ev) catch {};
        }
        if (res.dialog) |d| {
            if (d > 0 and d < pr.dialogs.len) {
                var buf: [1024]u8 = undefined;
                const body = event_mod.format(&buf, pr.dialogs[@intCast(d)].text, &.{.{ .d = res.dialog_value }});
                return .{ .push = Dialog.createText(c, d, .{ true, false, false, false }, &self.dialog_result, body) catch return .none };
            }
        }
        return .none;
    }

    /// Condition 16: the other heroes of the province; the arrows scroll.
    fn exchangeRows(self: *const MainScreen, out: *[max_list]usize) []usize {
        const k = self.hero orelse return out[0..0];
        const all = self.listHeroes(self.shard.heroes.items[k].province, k, out);
        const rest = all[@min(self.exchange_shift, all.len)..];
        return rest[0..@min(rest.len, bar_size)];
    }

    /// Condition 16 (MainProcess.cpp:4127): a row opens ScreenHeroHero.
    fn updateExchangeList(self: *MainScreen, c: *Ctx, f: *const input.Frame) ?scene.Action {
        const k = self.hero orelse return null;
        var buf: [max_list]usize = undefined;
        const shown = self.exchangeRows(&buf);
        for (&self.rows, 0..) |*m, i| {
            if (i >= shown.len) {
                m.state = .visible;
                continue;
            }
            m.shape = if (self.build_mask) |*img| img else null;
            if (m.check(f.pointer) == .click) {
                self.hero_mode = .main;
                return self.openExchange(c, shown[i]);
            }
        }
        var all_buf: [max_list]usize = undefined;
        const total = self.listHeroes(self.shard.heroes.items[k].province, k, &all_buf).len;
        const wheel_up = f.pointer.wheel > 0 and self.scroll_up.on;
        const wheel_down = f.pointer.wheel < 0 and self.scroll_down.on;
        if ((self.scroll_up.check(f) or wheel_up) and self.exchange_shift > 0) self.exchange_shift -= 1;
        if ((self.scroll_down.check(f) or wheel_down) and self.exchange_shift + bar_size < total) self.exchange_shift += 1;
        self.scroll_up.enabled = true;
        self.scroll_down.enabled = true;
        self.scroll_up.on = self.exchange_shift > 0;
        self.scroll_down.on = self.exchange_shift + bar_size < total;
        return null;
    }

    /// ScreenHeroHero(Member, other), then Condition 20.
    fn openExchange(self: *MainScreen, c: *Ctx, other: usize) scene.Action {
        const k = self.hero orelse return .none;
        self.refresh_hero = true;
        return .{ .push = HeroHero.create(c, &self.shard, k, other, self.players[self.player].exp_pct) catch |e| {
            std.log.err("hero exchange: {t}", .{e});
            return .none;
        } };
    }

    /// buttHeroGarr: ScreenHeroGarrison of the hero and his province.
    fn openHeroGarrison(self: *MainScreen, c: *Ctx) scene.Action {
        const k = self.hero orelse return .none;
        const h = &self.shard.heroes.items[k];
        const pv = &self.shard.provinces[@intCast(h.province)];
        if (pv.garrison < 0) return .none;
        return .{ .push = HeroGarrison.create(c, &self.shard, k, @intCast(pv.garrison), self.players[self.player].exp_pct) catch |e| {
            std.log.err("hero garrison: {t}", .{e});
            return .none;
        } };
    }

    // -- the orders of the turn -------------------------------------------

    /// Queues the heroes' orders (EndMainTurn) of the human player.
    fn queueHeroTasks(self: *MainScreen, c: *Ctx) void {
        if (self.hero_tasks) |*old| old.deinit();
        var run = hero_task.Runner.init(c.gpa, c.proto, &self.shard, &c.rand);
        const d = &c.proto.difficulties[@intCast(std.math.clamp(self.shard.rulers[self.player].difficulty, 0, @import("../data/proto.zig").max_difficulty - 1))];
        if (self.shard.rulers[self.player].kind == .human) run.loot_pct = d.loot;
        run.static_loot = c.cfg.static_loot;
        for (0..self.shard.heroes.items.len) |k| {
            const h = &self.shard.heroes.items[k];
            if (h.owner < 1 or !self.shard.in_game[@intCast(h.owner)]) continue;
            if (self.shard.rulers[@intCast(h.owner)].kind != .human) continue;
            run.add(k) catch {};
        }
        self.hero_tasks = run;
    }

    /// The next dialog / battle / treasure of the heroes' orders.
    fn pumpHeroTasks(self: *MainScreen, c: *Ctx) ?scene.Action {
        const t = if (self.hero_tasks) |*x| x else return null;
        if (self.hero_answer) |a| {
            self.hero_answer = null;
            t.answered(a -| 1);
        }
        if (self.treasure_done) {
            self.treasure_done = false;
            t.treasureDone();
        }
        if (t.next()) |req| switch (req) {
            .dialog => |sh| {
                self.event_prov = sh.prov;
                const pv = &self.shard.provinces[@intCast(sh.prov)];
                self.centre(pv.x + sh.dx, pv.y);
                return .{ .push = Dialog.createWith(c, sh.dialog, sh.enabled, &self.hero_answer, .{ .text = sh.text(), .dx = sh.dx, .treasure = sh.treasure }) catch |e| {
                    std.log.err("hero task dialog {d}: {t}", .{ sh.dialog, e });
                    self.hero_answer = (std.mem.indexOfScalar(bool, &sh.enabled, true) orelse 0) + 1;
                    return null;
                } };
            },
            .battle => |b| {
                const kk = for (self.shard.heroes.items, 0..) |*hh, j| {
                    if (hh.owner == @as(i32, @intCast(b.owner)) and hh.slot == b.slot and hh.alive()) break j;
                } else {
                    t.battleDone(0);
                    return null;
                };
                self.battle_hero = kk;
                self.battle_result = 0;
                self.battle_done = false;
                const where: battle_scene.Where = .{
                    .shard = &self.shard,
                    .hero = kk,
                    .target = b.prov,
                    .done = &self.battle_done,
                    .result = &self.battle_result,
                    .site = b.site,
                    .own = b.own,
                };
                return .{ .push = BattleScene.create(c, where, false) catch |e| {
                    std.log.err("battle: {t}", .{e});
                    t.battleDone(0);
                    return null;
                } };
            },
            .treasure => |tr| {
                self.treasure_done = false;
                return .{ .push = TreasureWindow.create(c, tr, &self.treasure_done) catch |e| {
                    std.log.err("treasure window: {t}", .{e});
                    self.treasure_done = true;
                    return null;
                } };
            },
        };
        if (t.idle()) {
            for (t.touched.items) |p| self.pics[@intCast(p)].level = -1;
            t.deinit();
            self.hero_tasks = null;
            self.event_prov = 0;
            self.drawPics(c);
            self.makeFog(c) catch {};
            self.afterHeroes();
        }
        return null;
    }

    // -- drawing ----------------------------------------------------------

    fn drawSiteList(self: *MainScreen, c: *Ctx) void {
        const k = self.hero orelse return;
        const h = &self.shard.heroes.items[k];
        const pr = c.proto;
        const pv = &self.shard.provinces[@intCast(h.province)];
        const bx: f32 = bar_x;
        const by: f32 = bar_y;
        gfx.drawTex(self.iface.texIndex(gui_build_bar), bx, by);
        var buf: [province.max_site]usize = undefined;
        const all = siteSlots(pv, pr, &buf);
        const shown = all[@min(self.site_shift, all.len)..];
        for (shown, 0..) |slot, i| {
            if (i >= bar_size) break;
            const dy: f32 = @floatFromInt(i * 59);
            const st = &pv.sites[slot];
            const site = &pr.sites[@intCast(st.index)];
            gfx.drawTex(self.iface.texIndex(gui_raze_panel), bx + 50, by + 39 + dy);
            if (st.index > 0) gfx.drawTex(self.sites_sheet.texIndex(@intCast(st.index - 1)), bx + 52, by + 41 + dy);
            c.font1.drawCentered(site.name, bx + 183, by + 48 + dy, color1);
            if (pv.site_guards[slot] >= 0 and st.guard >= 0 and st.guard < pr.guard_types.len) {
                var gb: [64]u8 = undefined;
                c.font1.drawCentered(std.fmt.bufPrint(&gb, "({s})", .{pr.guard_types[@intCast(st.guard)].name}) catch "", bx + 193, by + 70 + dy, .{ .r = 220, .g = 20, .b = 20, .a = 255 });
            }
            if (self.rows[i].hot()) gfx.drawTex(self.iface.texIndex(gui_build_select), self.rows[i].x, self.rows[i].y);
        }
        self.scroll_up.draw(&self.buttons, c.tick);
        self.scroll_down.draw(&self.buttons, c.tick);
        self.cancel_button.draw(&self.buttons, c.tick);
    }

    fn drawRitualList(self: *MainScreen, c: *Ctx) void {
        const k = self.hero orelse return;
        const h = &self.shard.heroes.items[k];
        const pr = c.proto;
        const m = self.market(c, @intCast(self.player));
        const bx: f32 = bar_x;
        const by: f32 = bar_y;
        gfx.drawTex(self.iface.texIndex(gui_build_bar), bx, by);
        var buf: [64]i32 = undefined;
        const all = ritual.heroRituals(&self.shard, pr, self.player, &buf);
        const shown = all[@min(self.ritual_shift, all.len)..];
        var nb: [32]u8 = undefined;
        for (shown, 0..) |idx, i| {
            if (i >= bar_size) break;
            const dy: f32 = @floatFromInt(i * 59);
            const r = &pr.rituals[@intCast(idx)];
            gfx.drawTex(self.iface.texIndex(gui_build_panel), bx + 50, by + 39 + dy);
            if (proto_mod.Ritual.picIndex(@intCast(idx))) |p| gfx.drawTex(self.rituals_sheet.texIndex(p), bx + 52, by + 41 + dy);
            const stock = ritual.stock(&self.shard, pr, self.player, idx);
            if (stock > 0) c.font1.draw(std.fmt.bufPrint(&nb, "{d}", .{stock}) catch "", bx + 56, by + 40 + dy, color2);
            const cd = ritual.cooldown(&self.shard, pr, self.player, idx);
            if (cd > 0) c.font1.draw(std.fmt.bufPrint(&nb, "{d}", .{cd}) catch "", bx + 58, by + 60 + dy, color2);
            const can = ritual.canCast(&self.shard, pr, h.province, idx, @intCast(self.player));
            c.font1.drawCentered(r.name, bx + 183, by + 48 + dy, if (can and cd == 0) color1 else .{ .r = 220, .g = 20, .b = 20, .a = 255 });
            const cost = ritual.price(m, idx);
            const gold_ok = cost[0] <= m.ruler().gold or cost[0] == 0;
            const gem_ok = cost[1] <= m.ruler().gem or cost[1] == 0;
            c.font1.drawCentered(std.fmt.bufPrint(&nb, "{d}", .{cost[0]}) catch "", bx + 151, by + 70 + dy, if (gold_ok) color2 else .{ .r = 220, .g = 20, .b = 20, .a = 255 });
            c.font1.drawCentered(std.fmt.bufPrint(&nb, "{d}", .{cost[1]}) catch "", bx + 233, by + 70 + dy, if (gem_ok) color2 else .{ .r = 220, .g = 20, .b = 20, .a = 255 });
            if (self.rows[i].hot()) gfx.drawTex(self.iface.texIndex(gui_build_select), self.rows[i].x, self.rows[i].y);
        }
        self.scroll_up.draw(&self.buttons, c.tick);
        self.scroll_down.draw(&self.buttons, c.tick);
        self.cancel_button.draw(&self.buttons, c.tick);
    }

    fn drawRitualDetail(self: *MainScreen, c: *Ctx) void {
        const k = self.hero orelse return;
        const h = &self.shard.heroes.items[k];
        const pr = c.proto;
        const m = self.market(c, @intCast(self.player));
        const idx = self.ritual_choice;
        if (idx <= 0 or idx >= pr.rituals.len) return;
        const r = &pr.rituals[@intCast(idx)];
        const bx: f32 = bar_x;
        const by: f32 = bar_y;
        gfx.drawTex(self.iface.texIndex(gui_building), bx, by);
        if (proto_mod.Ritual.picIndex(@intCast(idx))) |p| gfx.drawTex(self.rituals_sheet.texIndex(p), bx + 52, by + 65);
        c.font1.drawCentered(r.name, bx + 156, by + 44, color1);
        var nb: [32]u8 = undefined;
        const cost = ritual.price(m, idx);
        c.font1.drawCentered(std.fmt.bufPrint(&nb, "{d}", .{cost[0]}) catch "", bx + 150, by + 71, color2);
        c.font1.drawCentered(std.fmt.bufPrint(&nb, "{d}", .{cost[1]}) catch "", bx + 227, by + 71, color2);
        const cd = ritual.cooldown(&self.shard, pr, self.player, idx);
        c.font1.drawCentered(std.fmt.bufPrint(&nb, "{d}", .{@max(r.cooldown - m.bldParam1(53), 0)}) catch "", bx + 232, by + 102, color2);
        _ = h;
        _ = cd;
        _ = c.font2.drawWrappedClip(r.text, bx + 59, by + 163, 195, by + 163 + 184, color1);
        self.ritual_cast.draw(&self.buttons, c.tick);
        self.ritual_cancel.draw(&self.buttons, c.tick);
    }
};
