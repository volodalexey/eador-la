//! Eador. Genesis: порт на Zig + raylib.
//!
//! usage: eador [assets_dir]   (по умолчанию zig-out/assets или EADOR_ASSETS;
//!                              EADOR_LANG=ru|en выбирает язык,
//!                              EADOR_SCREEN=intro|menu — стартовая сцена)
const std = @import("std");
const rl = @import("raylib");
const GameFs = @import("platform/fs.zig").GameFs;
const gfx = @import("platform/gfx.zig");
const sound = @import("platform/sound.zig");
const music = @import("platform/music.zig");
const canvas = @import("platform/canvas.zig");
const input = @import("platform/input.zig");
const config = @import("data/config.zig");
const Proto = @import("data/proto.zig").Proto;
const profile = @import("data/profile.zig");
const scene = @import("scene.zig");
const Ctx = @import("app.zig").Ctx;
const Intro = @import("scenes/intro.zig").Intro;
const Menu = @import("scenes/menu.zig").Menu;
const Dialog = @import("scenes/dialog.zig").Dialog;
const Credits = @import("scenes/credits.zig").Credits;
const Profile = @import("scenes/profile.zig").Profile;
const Settings = @import("scenes/settings.zig").Settings;
const NewGame = @import("scenes/new_game.zig").NewGame;
const loading = @import("scenes/loading.zig");

const default_dir = "zig-out/assets";

pub fn main(init: std.process.Init) !void {
    const gpa = init.gpa;
    const arena = init.arena.allocator();
    const args = try init.minimal.args.toSlice(arena);
    const dir = if (args.len > 1) args[1] else init.environ_map.get("EADOR_ASSETS") orelse default_dir;

    var fs = GameFs.open(init.io, gpa, dir, init.environ_map.get("EADOR_LANG")) catch |e| {
        std.debug.print(
            "cannot open assets '{s}': {t}\nimport them first: zig build -Dgog_exe=setup_eador_genesis_*.exe  or  -Dgog_game=\"/path/to/Eador - Genesis\"\n",
            .{ dir, e },
        );
        return e;
    };
    defer fs.close();

    const cfg = blk: {
        const text = fs.read("eador.cfg") catch break :blk config.Config{};
        defer gpa.free(text);
        break :blk config.Config.parse(text);
    };

    rl.setTraceLogLevel(.warning);
    rl.setConfigFlags(.{ .window_resizable = true });
    const english = std.mem.eql(u8, fs.lang(), "en");
    rl.initWindow(canvas.width, canvas.height, if (english) "Eador. Genesis" else "Эадор. Сотворение");
    defer rl.closeWindow();
    rl.setWindowMinSize(canvas.width / 2, canvas.height / 2);
    if (fs.read("icon.png")) |bytes| {
        defer gpa.free(bytes);
        if (rl.loadImageFromMemory(".png", bytes)) |img| {
            rl.setWindowIcon(img);
            rl.unloadImage(img);
        } else |_| {}
    } else |_| {}
    var inp = input.Input.init(.mouse);

    var cv = try canvas.Canvas.init();
    defer cv.deinit();
    rl.initAudioDevice();
    defer rl.closeAudioDevice();
    sound.init(&fs);
    defer sound.deinit();
    rl.setTargetFPS(60);
    rl.setExitKey(.null);
    rl.hideCursor();

    var font1 = try gfx.Font.load(&fs, "font01");
    var font2 = try gfx.Font.load(&fs, "font02");
    const mouse = try gfx.Sheet.load(&fs, "mouse");
    const t0 = rl.getTime();
    var proto = Proto.load(gpa, &fs) catch |e| {
        std.debug.print("cannot load prototypes ({s}): {t}\n", .{ fs.lang(), e });
        return e;
    };
    defer proto.deinit();
    const verbose = init.environ_map.get("EADOR_VERBOSE");
    if (verbose != null)
        std.debug.print("prototypes loaded in {d:.0} ms\n", .{(rl.getTime() - t0) * 1000});
    var ctx: Ctx = .{
        .gpa = gpa,
        .fs = &fs,
        .font1 = font1,
        .font2 = font2,
        .mouse = mouse,
        .music = music.Player.init(gpa),
        .cfg = cfg,
        .proto = &proto,
        .human = profile.loadActive(&fs),
    };
    ctx.music.enabled = cfg.sound;
    ctx.verbose = verbose != null;
    // Randomizer(): srand(time % 10000).  EADOR_SEED=N fixes it (screenshots).
    const rnd_seed: u32 = if (init.environ_map.get("EADOR_SEED")) |s|
        std.fmt.parseInt(u32, s, 10) catch 1
    else blk: {
        const now = std.Io.Clock.real.now(init.io);
        break :blk @intCast(@mod(@divTrunc(now.nanoseconds, std.time.ns_per_s), 10000));
    };
    ctx.rand.seed(rnd_seed);
    if (ctx.human.fileIndex() == -1) ctx.human.clearNames(proto.phrase(216), proto.phrase(217), &ctx.rand);
    // Loader(): iMusicVolume and iSoundVolume from the profile.
    sound.setVolume(ctx.human.get(.snd_volume));
    ctx.music.setVolume(ctx.human.get(.mus_volume));
    defer {
        ctx.music.deinit();
        ctx.mouse.deinit();
        font1.deinit(gpa);
        font2.deinit(gpa);
    }

    var stack: scene.Stack = .{};
    defer stack.deinit(&ctx);
    var wantSetCursor: ?u32 = null;
    if (init.environ_map.get("EADOR_SET_CURSOR")) |cur_pos|
        wantSetCursor = try std.fmt.parseInt(u32, cur_pos, 10);
    const wantScreen = init.environ_map.get("EADOR_SCREEN");
    const wantScreenName = wantScreen orelse "";
    const wantMenuScreen = std.mem.eql(u8, wantScreenName, "menu");
    const wantCreditsScreen = std.mem.eql(u8, wantScreenName, "credits");
    const wantProfileScreen = std.mem.eql(u8, wantScreenName, "profile");
    const wantSettingsScreen = std.mem.eql(u8, wantScreenName, "settings");
    const wantNewGameScreen = std.mem.eql(u8, wantScreenName, "newgame");
    const wantHotseatScreen = std.mem.eql(u8, wantScreenName, "hotseat");
    const wantShardScreen = std.mem.eql(u8, wantScreenName, "shard");
    const wantContinueScreen = std.mem.eql(u8, wantScreenName, "continue");
    const wantContinueHotScreen = std.mem.eql(u8, wantScreenName, "continue-hot");
    const wantCastleScreen = std.mem.eql(u8, wantScreenName, "castle");
    const wantGarrisonScreen = std.mem.eql(u8, wantScreenName, "garrison");
    const wantStatsScreen = std.mem.eql(u8, wantScreenName, "stats");
    const wantPlistScreen = std.mem.eql(u8, wantScreenName, "plist");
    const wantHeroScreen = std.mem.eql(u8, wantScreenName, "hero");
    const wantHeroGarScreen = std.mem.eql(u8, wantScreenName, "herogar");
    const wantHeroShopScreen = std.mem.eql(u8, wantScreenName, "heroshop");
    const wantHeroLibScreen = std.mem.eql(u8, wantScreenName, "herolib");
    const wantHeroSwapScreen = std.mem.eql(u8, wantScreenName, "heroswap");
    const skip_intro = !cfg.intro or (wantMenuScreen or wantCreditsScreen or wantProfileScreen or wantSettingsScreen or wantNewGameScreen or wantHotseatScreen or wantShardScreen or wantContinueScreen or wantContinueHotScreen or wantCastleScreen or wantGarrisonScreen or wantStatsScreen or wantPlistScreen or wantHeroGarScreen);
    if (skip_intro) {
        if (wantSetCursor) |cur_pos| ctx.setCursor(cur_pos);
        try stack.push(try Menu.create(&ctx));
    } else try stack.push(try Intro.create(&ctx));
    if (wantCreditsScreen) {
        if (wantSetCursor) |cur_pos| ctx.setCursor(cur_pos);
        try stack.push(try Credits.create(&ctx));
    }
    // EADOR_SCREEN=profile: окно профиля поверх меню.
    var profile_closed = false;
    if (wantProfileScreen) {
        if (wantSetCursor) |cur_pos| ctx.setCursor(cur_pos);
        try stack.push(try Profile.create(&ctx, &profile_closed));
    }
    // EADOR_SCREEN=newgame|hotseat: экран новой игры; shard: сразу
    // «Создать карту» с настройками профиля.
    if (wantNewGameScreen or wantHotseatScreen or wantShardScreen) {
        const ng_scene = try NewGame.create(&ctx, if (wantHotseatScreen) .hotseat else .single);
        try stack.push(ng_scene);
        if (wantShardScreen) @as(*NewGame, @ptrCast(@alignCast(ng_scene.ptr))).accept_now = true;
    }
    // EADOR_SCREEN=continue|continue-hot: «Продолжить» одиночной игры или
    // hot-seat (с экраном загрузки).  С EADOR_BATTLE=1 карта открывается
    // сразу, и начинается бой первого героя с охраной соседней нейтральной
    // провинции.
    if (wantContinueScreen or wantContinueHotScreen) {
        const gt: i32 = if (wantContinueScreen) 1 else 3;
        if (init.environ_map.get("EADOR_BATTLE")) |mode| {
            if (loading.openNow(&ctx, gt)) |sc| {
                try stack.push(sc);
                const ms: *@import("scenes/main_screen.zig").MainScreen = @ptrCast(@alignCast(sc.ptr));
                if (!ms.debugBattle(std.mem.eql(u8, mode, "auto"))) std.log.err("battle: no guarded neighbour", .{});
            } else |e| std.log.err("continue: {t}", .{e});
        } else if (init.environ_map.get("EADOR_EVENT")) |s| {
            // EADOR_EVENT=N: событие N в столице (диалог поверх карты).
            if (loading.openNow(&ctx, gt)) |sc| {
                try stack.push(sc);
                const ms: *@import("scenes/main_screen.zig").MainScreen = @ptrCast(@alignCast(sc.ptr));
                ms.debugEvent(&ctx, std.fmt.parseInt(i32, s, 10) catch 1);
            } else |e| std.log.err("continue: {t}", .{e});
        } else if (init.environ_map.get("EADOR_PANEL")) |mode| {
            // EADOR_PANEL=main|build_list|raze_list|hire_list|guard|site_list|
            // fort|ritual_list|enchant [EADOR_PROV=N]: панель провинции N
            // (по умолчанию столица) на этой странице.
            if (loading.openNow(&ctx, gt)) |sc| {
                try stack.push(sc);
                const ms: *@import("scenes/main_screen.zig").MainScreen = @ptrCast(@alignCast(sc.ptr));
                const prov = if (init.environ_map.get("EADOR_PROV")) |s| std.fmt.parseInt(i32, s, 10) catch 0 else 0;
                ms.debugPanel(&ctx, mode, prov);
            } else |e| std.log.err("continue: {t}", .{e});
        } else switch (Menu.loadContinue(&ctx, gt)) {
            .push => |sc| try stack.push(sc),
            else => std.log.err("continue: no saved game", .{}),
        }
    }
    // EADOR_SCREEN=castle [EADOR_CASTLE_TAB=build|hero|garrison|treasury|
    // library|stats]: замок игрока из «Продолжить»; garrison — гарнизон столицы
    // поверх карты (ScreenGarrison).
    // stats|plist — статистика или список провинций поверх карты.
    // hero|herogar|heroshop|herolib|heroswap — экран первого героя игрока,
    // его гарнизон, магазин, библиотека (ScreenHeroLibrary) или обмен со вторым
    // героем (ScreenHeroHero, второй герой переносится в провинцию первого).
    if (wantCastleScreen or wantGarrisonScreen or wantStatsScreen or wantPlistScreen or wantHeroScreen or wantHeroGarScreen or wantHeroShopScreen or wantHeroLibScreen or wantHeroSwapScreen) {
        switch (if (loading.openNow(&ctx, 1)) |sc| scene.Action{ .push = sc } else |e| blk: {
            std.log.err("castle: {t}", .{e});
            break :blk scene.Action.none;
        }) {
            .push => |sc| {
                try stack.push(sc);
                const ms: *@import("scenes/main_screen.zig").MainScreen = @ptrCast(@alignCast(sc.ptr));
                // EADOR_BUILD=7,3,...: построить здания замка бесплатно.
                if (init.environ_map.get("EADOR_BUILD")) |list| ms.debugBuild(&ctx, list);
                if (wantHeroScreen or wantHeroGarScreen or wantHeroShopScreen or wantHeroLibScreen or wantHeroSwapScreen) {
                    const what: @import("scenes/main_screen.zig").MainScreen.DebugHero = if (wantHeroGarScreen) .garrison else if (wantHeroShopScreen) .shop else if (wantHeroLibScreen) .library else if (wantHeroSwapScreen) .exchange else .hero;
                    try stack.push(try ms.debugHero(&ctx, what));
                    if (init.environ_map.get("EADOR_INFO")) |info| {
                        if (std.mem.eql(u8, info, "hero")) try stack.push(try ms.debugHero(&ctx, .info));
                    }
                } else if (wantGarrisonScreen) {
                    try stack.push(try ms.debugGarrison(&ctx));
                } else if (wantStatsScreen or wantPlistScreen) {
                    try stack.push(try ms.debugStatistics(&ctx, wantPlistScreen));
                } else {
                    const name = init.environ_map.get("EADOR_CASTLE_TAB") orelse "build";
                    const tab = std.meta.stringToEnum(@import("ui/castle_ui.zig").Tab, name) orelse .build;
                    const cs = try ms.debugCastle(&ctx, if (tab == .stats) .build else tab);
                    try stack.push(cs);
                    if (tab == .stats) switch (@as(*@import("scenes/castle.zig").CastleScreen, @ptrCast(@alignCast(cs.ptr))).openStatistics(&ctx)) {
                        .push => |w| try stack.push(w),
                        else => {},
                    };
                    // EADOR_BUILD_INFO=N: окно здания N (WindowInnBldInfo).
                    if (init.environ_map.get("EADOR_BUILD_INFO")) |s| {
                        const id = std.fmt.parseInt(i32, s, 10) catch 0;
                        const scr: *@import("scenes/castle.zig").CastleScreen = @ptrCast(@alignCast(cs.ptr));
                        switch (scr.debugInfo(&ctx, id)) {
                            .push => |w| try stack.push(w),
                            else => {},
                        }
                    }
                }
            },
            else => std.log.err("castle: no saved game", .{}),
        }
    }
    // EADOR_SCREEN=settings: окно настроек поверх меню.
    if (wantSettingsScreen) {
        if (wantSetCursor) |cur_pos| ctx.setCursor(cur_pos);
        try stack.push(try Settings.create(&ctx, null));
    }
    // EADOR_DIALOG=N: dialog.var N поверх стартовой сцены (проверка окон).
    var dialog_result: ?usize = null;
    if (init.environ_map.get("EADOR_DIALOG")) |s| {
        const n = std.fmt.parseInt(usize, s, 10) catch 0;
        try stack.push(try Dialog.create(&ctx, n, .{ true, true, true, true }, &dialog_result));
    }
    // EADOR_MENU=map|war: меню игры (WindowMenu) или меню боя (WarMenu)
    // поверх стартовой сцены (проверка окон).
    var menu_out: @import("scenes/game_menu.zig").Out = .{};
    var war_out: @import("scenes/war_menu.zig").Out = .{};
    if (init.environ_map.get("EADOR_MENU")) |s| {
        if (std.mem.eql(u8, s, "war")) {
            try stack.push(try @import("scenes/war_menu.zig").WarMenu.create(&ctx, false, &war_out));
        } else try stack.push(try @import("scenes/game_menu.zig").GameMenu.create(&ctx, .{ .past = false, .surrender = true }, &menu_out));
    }
    // EADOR_INFO=kind:N[:M]: информационное окно поверх стартовой сцены
    // (ability:N:сила, medal, race, skill:N:уровень, effect:улучшение:заклинание,
    // defender, outer, ritual, item, spell, resource, unit).
    if (init.environ_map.get("EADOR_INFO")) |s| {
        if (!std.mem.eql(u8, s, "hero")) {
            if (debugInfo(&ctx, s)) |sc| try stack.push(sc) else |e| std.log.err("info {s}: {t}", .{ s, e });
        }
    }

    // EADOR_SHOT=file.png [EADOR_FRAMES=N]: отрисовать N кадров (по умолчанию 30),
    // сохранить кадр и выйти.
    const wantScreenshot = init.environ_map.get("EADOR_SHOT");
    if (wantSetCursor) |cur_pos| {
        if (cur_pos == 0) {
            ctx.skipSetCursor = true;
        }
    }
    const shot_frames = if (init.environ_map.get("EADOR_FRAMES")) |s| std.fmt.parseInt(u32, s, 10) catch 30 else 30;
    var frames: u32 = 0;

    while (!rl.windowShouldClose()) {
        if (wantScreenshot) |path| {
            if (frames == shot_frames) {
                try saveShot(arena, &cv, path);
                break;
            }
            frames += 1;
        }
        // Скриншоты воспроизводимы: фиксированный шаг 1/60 с.
        const dt: f32 = if (wantScreenshot != null) 1.0 / 60.0 else rl.getFrameTime();
        ctx.advanceTime(dt);
        ctx.music.update(&fs, &ctx.rand, dt);
        const frame = inp.poll(dt);
        if (!try stack.update(&ctx, &frame, dt)) break;

        gfx.hide_cursor = false;
        cv.begin();
        stack.draw(&ctx);
        cv.end();

        rl.beginDrawing();
        cv.present();
        if (inp.source == .mouse and frame.pointer.hover and !gfx.hide_cursor) drawCursor(&ctx);
        rl.endDrawing();
    }
}

fn debugInfo(ctx: *Ctx, spec: []const u8) !scene.Scene {
    const iw = @import("scenes/info_windows.zig");
    var it = std.mem.splitScalar(u8, spec, ':');
    const kind = it.next() orelse return error.BadInfo;
    const a = std.fmt.parseInt(i32, it.next() orelse "1", 10) catch 1;
    const b = std.fmt.parseInt(i32, it.next() orelse "0", 10) catch 0;
    const eq = struct {
        fn f(x: []const u8, y: []const u8) bool {
            return std.mem.eql(u8, x, y);
        }
    }.f;
    if (eq(kind, "ability")) return iw.ability(ctx, a, b);
    if (eq(kind, "medal")) return iw.medal(ctx, a);
    if (eq(kind, "race")) return iw.race(ctx, a);
    if (eq(kind, "skill")) return iw.skill(ctx, a, if (b > 0) b else 1);
    if (eq(kind, "effect")) return iw.effect(ctx, a, b, 3, 0);
    if (eq(kind, "defender")) return iw.CostWindow.defender(ctx, a, 0, 0, null);
    if (eq(kind, "outer")) return iw.CostWindow.outer(ctx, a, null);
    if (eq(kind, "ritual")) return iw.CostWindow.ritual(ctx, a, null);
    if (eq(kind, "item")) return @import("scenes/item_info.zig").ItemInfo.create(ctx, a, if (a > 0 and a < ctx.proto.items.len) ctx.proto.items[@intCast(a)].durability else 0);
    if (eq(kind, "spell")) return @import("scenes/spell_info.zig").SpellInfo.create(ctx, a);
    if (eq(kind, "resource")) return @import("scenes/army_info.zig").ResourceWindow.create(ctx, a, null);
    if (eq(kind, "unit")) return @import("scenes/army_info.zig").UnitWindow.createType(ctx, a);
    return error.BadInfo;
}

fn saveShot(arena: std.mem.Allocator, cv: *canvas.Canvas, path: []const u8) !void {
    var img = try rl.loadImageFromTexture(cv.target.texture);
    defer rl.unloadImage(img);
    rl.imageFlipVertical(&img);
    if (!rl.exportImage(img, try arena.dupeSentinel(u8, path, 0))) return error.ShotFailed;
}

/// Курсор (set_mouse_sprite) в координатах окна, в масштабе кадра.
fn drawCursor(ctx: *Ctx) void {
    const t = ctx.mouse.texIndex(ctx.cursor) orelse return;
    const vp = canvas.Canvas.viewport();
    const scale = vp.width / canvas.width;
    const p = rl.getMousePosition();
    const w: f32 = @floatFromInt(t.width);
    const h: f32 = @floatFromInt(t.height);
    rl.drawTexturePro(t, .{ .x = 0, .y = 0, .width = w, .height = h }, .{ .x = p.x, .y = p.y, .width = w * scale, .height = h * scale }, .{ .x = 0, .y = 0 }, 0, .white);
}
