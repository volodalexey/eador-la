//! The screens shown while a game is created or loaded (RunHourGlass,
//! Audio.cpp:394, see ui/hourglass.zig).
//!
//! Creating: GenerateMap (Mapper.cpp:563) and the save of RunGame(true).
//! ShowWallpaper (GameMenu.cpp:120) picks one of three pictures with
//! Random(3), the next hint of text/hint.txt (arHumans[0].iHint) is drawn
//! in grey font02 over a black strip at 150/255, and the panel says
//! "Сотворение карты..." (phrase 328), then "Сохранение..." (149) while
//! SaveMap and SaveGame write the files.  The bar follows NewProgress of
//! the stages this port makes: the shard 0..2100, the province shapes
//! 10000 + 5000·i/N (the "Fog" step), the borders up to 20000 (the terrain
//! layers are not drawn yet) and DrawPic of every province 20000 +
//! 2000·i/N (FillObstracleMap(2000)).
//!
//! Loading: LoadGame (GameMenu.cpp:657), "Загрузка..." (153) over the
//! screen below.  LoadMap is 0..1000 (here the province shapes), the rest
//! of the file up to 2800, then FillObstracleMap(19200): 2800 + 19200·i/N.
//!
//! The work is split into province steps, done for up to `budget` seconds
//! a frame, so the hourglass turns as in the original's timer.
const std = @import("std");
const rl = @import("raylib");
const scene = @import("../scene.zig");
const Ctx = scene.Ctx;
const input = @import("../platform/input.zig");
const gfx = @import("../platform/gfx.zig");
const Hourglass = @import("../ui/hourglass.zig").Hourglass;
const text = @import("../ui/text.zig");
const ng = @import("../data/newgame.zig");
const profile = @import("../data/profile.zig");
const mapgen = @import("../game/mapgen.zig");
const newmap = @import("../game/newmap.zig");
const newsave = @import("../game/newsave.zig");
const loadgame = @import("../game/loadgame.zig");
const packfile = @import("../formats/packfile.zig");
const main_screen = @import("main_screen.zig");
const MainScreen = main_screen.MainScreen;
const Opening = MainScreen.Opening;
const Stub = @import("stub.zig").Stub;

const ph_create = 328;
const ph_save = 149;
const ph_load = 153;

/// interface.dat bmpGUI_Screen01; stuff.dat 77 and 80 (ShowWallpaper).
const gui_screen01 = 80;
const stuff_screen02 = 77;
const stuff_screen03 = 80;

/// COLOR_2.
const grey: rl.Color = .{ .r = 220, .g = 220, .b = 220, .a = 255 };

/// Seconds of work a frame.
const budget = 1.0 / 60.0;

/// Sheets of the panel.
const Panel = struct {
    iface: gfx.Sheet,
    anim: gfx.Sheet,

    fn load(c: *Ctx) !Panel {
        var iface = try gfx.Sheet.load(c.fs, "interface");
        errdefer iface.deinit();
        return .{ .iface = iface, .anim = try gfx.Sheet.load(c.fs, "anim") };
    }

    fn deinit(self: *Panel) void {
        self.anim.deinit();
        self.iface.deinit();
    }

    fn draw(self: *Panel, c: *Ctx, g: *const Hourglass) void {
        g.draw(&self.iface, &self.anim, &c.font1, c.proto.phrase(g.phrase));
    }
};

/// NewProgress while MainScreen makes its pictures: the shapes from
/// `shapes0` to `shapes1`, the textures at `pics0`, DrawPic up to `pics1`.
const Scale = struct { shapes0: i32, shapes1: i32, pics0: i32, pics1: i32 };

fn openingProgress(op: *const Opening, s: Scale) i32 {
    const n: i32 = @intCast(@max(op.count(), 1));
    const k: i32 = @intCast(op.done());
    return switch (op.phase) {
        .shapes => s.shapes0 + @divTrunc((s.shapes1 - s.shapes0) * k, n),
        .textures => s.shapes1,
        .pics => s.pics0 + @divTrunc((s.pics1 - s.pics0) * k, n),
        .done => s.pics1,
    };
}

/// Runs `work` until it changes the state or the frame's time is up.
fn runSteps(self: anytype, c: *Ctx) !?scene.Action {
    const t0 = rl.getTime();
    while (true) {
        if (try self.work(c)) |a| return a;
        if (!self.drawn) return null;
        if (rl.getTime() - t0 > budget) return null;
    }
}

/// GenerateMap and the first SaveGame of a new game, then MainScreen.
pub const Creating = struct {
    panel: Panel,
    stuff: gfx.Sheet,
    /// ShowWallpaper's Random(3).
    wallpaper: u32,
    /// pFirstHint + arHumans[0].iHint.
    hint: []const u8,
    glass: Hourglass = .{ .phrase = ph_create },
    game: ng.Game,
    players: [ng.max_slots]mapgen.Player,
    lands: [ng.max_slots][96]u8,
    pls: [mapgen.max_players + 1]main_screen.Player,
    state: State = .generate,
    /// The current state has been on the screen.
    drawn: bool = false,
    opening: ?Opening = null,
    /// The game file, unpacked.
    raw_game: ?[]u8 = null,

    const State = enum { generate, pictures, save };

    /// `players[i].land` may point into `lands[i]`.
    pub fn create(c: *Ctx, game: ng.Game, players: []const mapgen.Player, lands: *const [ng.max_slots][96]u8, pls: [mapgen.max_players + 1]main_screen.Player, hint: []const u8) !scene.Scene {
        const self = try c.gpa.create(Creating);
        errdefer c.gpa.destroy(self);
        var panel = try Panel.load(c);
        errdefer panel.deinit();
        self.* = .{
            .panel = panel,
            .stuff = try gfx.Sheet.load(c.fs, "stuff"),
            .wallpaper = c.rand.random(3),
            .hint = hint,
            .game = game,
            .players = undefined,
            .lands = lands.*,
            .pls = pls,
        };
        for (players, 0..) |pl, i| {
            self.players[i] = pl;
            if (pl.land.len > 0) self.players[i].land = self.lands[i][0..pl.land.len];
        }
        // RunGame: the map music starts with the game (GameMenu.cpp:159).
        c.music.setMode(c.fs, .map);
        return scene.Scene.of(Creating, self, false);
    }

    pub fn deinit(self: *Creating, c: *Ctx) void {
        if (self.opening) |*op| op.abort(c);
        if (self.raw_game) |g| c.gpa.free(g);
        self.stuff.deinit();
        self.panel.deinit();
    }

    fn playerList(self: *const Creating) []const mapgen.Player {
        return self.players[0..self.game.players()];
    }

    pub fn update(self: *Creating, c: *Ctx, _: *const input.Frame, dt: f32) scene.Action {
        self.glass.update(dt);
        if (!self.drawn) return .none;
        const a = runSteps(self, c) catch |e| {
            std.log.err("map generation: {t}", .{e});
            return .{ .replace = Stub.create(c, "GenerateMap") catch return .pop };
        };
        return a orelse .none;
    }

    fn work(self: *Creating, c: *Ctx) !?scene.Action {
        switch (self.state) {
            .generate => {
                const g = &self.game;
                var shard = try mapgen.generate(c.gpa, c.proto, &c.rand, .{
                    .seed = g.seed,
                    .map = g.map,
                    .game_type = @backingInt(g.type),
                    .players = self.playerList(),
                    .lake_ships = c.cfg.lake_ships,
                });
                self.glass.progress = 2100;
                {
                    errdefer shard.deinit();
                    // SaveGame(0) of RunGame(true); the file is written
                    // after SaveMap, MainScreen starts from it.
                    const raw = try newsave.gameBytes(c.gpa, c.proto, &shard, .{
                        .seed = g.seed,
                        .game_type = @backingInt(g.type),
                        .players = self.playerList(),
                        .requested = g.player_set[0..g.players()],
                        .diplomacy = g.diplomacy,
                        .fog = g.fog,
                        .price_mod = c.cfg.resource_price_mod,
                    });
                    self.raw_game = raw;
                    try loadgame.attachSaved(&shard, raw);
                }
                self.opening = try MainScreen.begin(c, shard, self.pls, 1, 0, null, .{ .province = shard.capitals[1] });
                self.state = .pictures;
            },
            .pictures => {
                const op = &self.opening.?;
                try op.step(c);
                self.glass.progress = openingProgress(op, .{ .shapes0 = 10000, .shapes1 = 15000, .pics0 = 20000, .pics1 = 22000 });
                if (op.phase == .done) {
                    self.state = .save;
                    self.glass.phrase = ph_save;
                    self.drawn = false;
                }
            },
            .save => {
                try self.save(c);
                var op = self.opening.?;
                self.opening = null;
                const sc = op.finish(c) catch |e| {
                    op.abort(c);
                    return e;
                };
                return .{ .replace = sc };
            },
        }
        return null;
    }

    /// SaveMap and SaveGame (Mapper.cpp:3189, GameMenu.cpp:330):
    /// save/map{N}.map and save/game{N}.sav (.hot for hot seat), N the
    /// active profile's file index.
    fn save(self: *Creating, c: *Ctx) !void {
        const op = &self.opening.?;
        const pics = op.madePictures() orelse return error.NoPictures;
        const ext = switch (self.game.type) {
            .single => "map",
            .hotseat => "hot",
        };
        const gext = switch (self.game.type) {
            .single => "sav",
            .hotseat => "hot",
        };
        const n = c.human.fileIndex();
        const raw_map = try newmap.mapBytes(c.gpa, &op.ms.shard, &pics.map, pics.shapes);
        defer c.gpa.free(raw_map);
        const map = try packfile.packAllegro(c.gpa, raw_map);
        defer c.gpa.free(map);
        const gdata = try packfile.packAllegro(c.gpa, self.raw_game.?);
        defer c.gpa.free(gdata);
        var pb: [64]u8 = undefined;
        try c.fs.write(try std.fmt.bufPrint(&pb, "save/map{d}.{s}", .{ n, ext }), map);
        try c.fs.write(try std.fmt.bufPrint(&pb, "save/game{d}.{s}", .{ n, gext }), gdata);
        if (c.verbose) std.debug.print("saved save/map{d}.{s} ({d} bytes) and save/game{d}.{s} ({d} bytes)\n", .{ n, ext, map.len, n, gext, gdata.len });
    }

    pub fn draw(self: *Creating, c: *Ctx) void {
        rl.clearBackground(.black);
        const wall = switch (self.wallpaper) {
            0 => self.stuff.texIndex(stuff_screen02),
            1 => self.panel.iface.texIndex(gui_screen01),
            else => self.stuff.texIndex(stuff_screen03),
        };
        gfx.drawTex(wall, 0, 0);
        // bmpBar: 888×100 of COLOR_1 at 150/255, the hint box over it.
        rl.drawRectangle(68, 648, 888, 100, .{ .r = 0, .g = 0, .b = 0, .a = 150 });
        var box = text.Box.init(78, 658, 848, 100, &c.font2, grey);
        box.centre = true;
        _ = box.draw(self.hint);
        self.panel.draw(c, &self.glass);
        self.drawn = true;
    }
};

/// A saved game read for MainScreen (CheckGameLoad / LoadGame).
pub const Saved = struct {
    shard: mapgen.Shard,
    players: [mapgen.max_players + 1]main_screen.Player,
    player: usize,
    turn: i32,
    fog: []u8,
    start: main_screen.Start,
    /// LoadMap's map and province shapes, ready for MainScreen.
    pictures: newmap.Pictures,
};

/// save\map%d and save\game%d of the active profile (.hot for hot seat,
/// `game_type` 3).
pub fn readSaved(c: *Ctx, game_type: i32) !Saved {
    const ext = if (game_type == 3) "hot" else "map";
    const gext = if (game_type == 3) "hot" else "sav";
    const n = c.human.fileIndex();
    var pb: [64]u8 = undefined;
    const map_packed = try c.fs.read(try std.fmt.bufPrint(&pb, "save/map{d}.{s}", .{ n, ext }));
    defer c.gpa.free(map_packed);
    const map_data = try packfile.unpack(c.gpa, map_packed);
    defer c.gpa.free(map_data);
    const game_packed = try c.fs.read(try std.fmt.bufPrint(&pb, "save/game{d}.{s}", .{ n, gext }));
    defer c.gpa.free(game_packed);
    const game_data = try packfile.unpack(c.gpa, game_packed);
    defer c.gpa.free(game_data);
    var g = try loadgame.fromBytes(c.gpa, c.proto, map_data, game_data);
    // Herb colours and ExpForLevel's factor of the loaded rulers.
    var pls: [mapgen.max_players + 1]main_screen.Player = @splat(.{});
    for (1..g.n_players + 1) |k| {
        const r = g.rulers[k];
        if (r.kind == .human) {
            const h = if (r.ruler == c.human.fileIndex()) c.human else profile.load(c.fs, r.ruler);
            const exp = c.proto.difficulties[@intCast(std.math.clamp(r.difficulty, 0, @as(i32, @intCast(c.proto.difficulties.len)) - 1))].exp;
            pls[k] = main_screen.Player.ofHuman(if (h) |*hh| hh else null, exp);
        } else pls[k] = main_screen.Player.ofComputer(c.proto, r.ruler, r.difficulty);
    }
    // RunGame: the interrupted player (iActivePlayer), else the first
    // human in turn order.
    const player: usize = blk: {
        const a = g.active_player;
        if (a >= 1 and a <= g.n_players and g.rulers[@intCast(a)].kind == .human) break :blk @intCast(a);
        for (1..g.n_players + 1) |k| if (g.rulers[k].kind == .human) break :blk k;
        break :blk 1;
    };
    const start = g.start(player);
    const turn = g.turn;
    errdefer g.deinit();
    const map = try g.mapBitmap(c.gpa);
    errdefer c.gpa.free(map.px);
    const shapes = try g.fogShapes(c.gpa);
    errdefer newmap.freeShapes(c.gpa, shapes);
    const r = g.release(player);
    return .{
        .shard = r.shard,
        .players = pls,
        .player = player,
        .turn = turn,
        .fog = r.fog,
        .start = start,
        .pictures = .{ .map = map, .shapes = shapes },
    };
}

/// The saved game's MainScreen at once, without the loading screen
/// (EADOR_SCREEN hooks that act on the MainScreen).
pub fn openNow(c: *Ctx, game_type: i32) !scene.Scene {
    var g = try readSaved(c, game_type);
    var op = MainScreen.begin(c, g.shard, g.players, g.player, g.turn, g.fog, g.start) catch |e| {
        g.pictures.deinit(c.gpa);
        return e;
    };
    op.pictures = g.pictures;
    op.phase = .textures;
    return op.run(c);
}

/// LoadGame over the scene below, then MainScreen in its place.
pub const Loading = struct {
    panel: Panel,
    glass: Hourglass = .{ .phrase = ph_load },
    game_type: i32,
    state: State = .read,
    drawn: bool = false,
    opening: ?Opening = null,

    const State = enum { read, pictures };

    pub fn create(c: *Ctx, game_type: i32) !scene.Scene {
        const self = try c.gpa.create(Loading);
        errdefer c.gpa.destroy(self);
        self.* = .{ .panel = try Panel.load(c), .game_type = game_type };
        // RunGame: the map music starts with the game (GameMenu.cpp:159).
        c.music.setMode(c.fs, .map);
        return scene.Scene.of(Loading, self, true);
    }

    pub fn deinit(self: *Loading, c: *Ctx) void {
        if (self.opening) |*op| op.abort(c);
        self.panel.deinit();
    }

    pub fn update(self: *Loading, c: *Ctx, _: *const input.Frame, dt: f32) scene.Action {
        self.glass.update(dt);
        if (!self.drawn) return .none;
        const a = runSteps(self, c) catch |e| {
            std.log.err("load game: {t}", .{e});
            return .pop;
        };
        return a orelse .none;
    }

    fn work(self: *Loading, c: *Ctx) !?scene.Action {
        switch (self.state) {
            .read => {
                const g = try readSaved(c, self.game_type);
                var op = try MainScreen.begin(c, g.shard, g.players, g.player, g.turn, g.fog, g.start);
                op.pictures = g.pictures;
                op.phase = .textures;
                self.opening = op;
                self.state = .pictures;
            },
            .pictures => {
                const op = &self.opening.?;
                while (op.phase != .done) try op.step(c);
                self.glass.progress = openingProgress(op, .{ .shapes0 = 0, .shapes1 = 1000, .pics0 = 2800, .pics1 = 22000 });
                if (op.phase == .done) {
                    var o = self.opening.?;
                    self.opening = null;
                    const sc = o.finish(c) catch |e| {
                        o.abort(c);
                        return e;
                    };
                    return .{ .replace = sc };
                }
            },
        }
        return null;
    }

    pub fn draw(self: *Loading, c: *Ctx) void {
        self.panel.draw(c, &self.glass);
        self.drawn = true;
    }
};
