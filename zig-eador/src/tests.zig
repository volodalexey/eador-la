const std = @import("std");
const cp1251 = @import("formats/cp1251.zig");
const lzss = @import("formats/lzss.zig");
const datafile = @import("formats/datafile.zig");
const varfile = @import("formats/varfile.zig");
const pcxfont = @import("formats/pcxfont.zig");

test {
    _ = @import("formats/png.zig");
    _ = @import("formats/inno.zig");
    _ = @import("formats/packfile.zig");
    _ = @import("formats/peicon.zig");
    _ = @import("formats/textfile.zig");
    _ = @import("data/config.zig");
    _ = @import("data/strings.zig");
    _ = @import("data/proto.zig");
    _ = @import("data/profile.zig");
    _ = @import("data/rand.zig");
    _ = @import("data/newgame.zig");
    _ = @import("game/mapgen.zig");
    _ = @import("game/campaign.zig");
    _ = @import("game/unit.zig");
    _ = @import("game/guard.zig");
    _ = @import("game/site.zig");
    _ = @import("game/newsave.zig");
    _ = @import("game/newmap.zig");
    _ = @import("game/loadgame.zig");
    _ = @import("game/hero.zig");
    _ = @import("game/heroes.zig");
    _ = @import("game/movement.zig");
    _ = @import("game/economy.zig");
    _ = @import("game/event.zig");
    _ = @import("game/statistics.zig");
    _ = @import("game/player.zig");
    _ = @import("game/equip.zig");
    _ = @import("game/spellbook.zig");
    _ = @import("game/castle_pic.zig");
    _ = @import("game/castle.zig");
    _ = @import("game/army.zig");
    _ = @import("game/decor.zig");
    _ = @import("game/ritual.zig");
    _ = @import("game/prov_order.zig");
    _ = @import("game/hero_task.zig");
    _ = @import("scenes/treasure.zig");
    _ = @import("scenes/hero_garrison.zig");
    _ = @import("ui/mask.zig");
    _ = @import("ui/text.zig");
    _ = @import("ui/hourglass.zig");
    _ = @import("scenes/dialog.zig");
    _ = @import("scenes/window_info.zig");
    _ = @import("scenes/info_windows.zig");
    inline for (.{ @import("scenes/window_info.zig").WindowInfo, @import("scenes/window_info.zig").ScrollBox, @import("scenes/info_windows.zig").CostWindow }) |T| std.testing.refAllDecls(T);
    std.testing.refAllDecls(@import("scenes/info_windows.zig"));
    _ = @import("scenes/unit_info.zig");
    _ = @import("game/battle.zig"); // battle tests
    _ = @import("game/battle_sim.zig");
    _ = @import("scenes/battle.zig");
    _ = @import("ui/button.zig");
    _ = @import("ui/scrollbar.zig");
    _ = @import("ui/input.zig");
    _ = @import("ui/hint.zig");
    _ = @import("ui/castle_ui.zig");
    inline for (.{ @import("ui/castle_ui.zig"), @import("ui/castle_ui.zig").Sheets, @import("ui/castle_ui.zig").ResourceBar, @import("ui/castle_ui.zig").TabBar, @import("game/player.zig").Market, @import("game/player.zig").Holdings }) |T| std.testing.refAllDecls(T);
    _ = @import("ui/heraldry.zig");
    _ = @import("scenes/credits.zig");
    _ = @import("scenes/profile.zig");
    _ = @import("scenes/settings.zig");
    _ = @import("scenes/map_settings.zig");
    _ = @import("scenes/game_menu.zig");
    _ = @import("scenes/war_menu.zig");
    _ = @import("scenes/new_game.zig");
    _ = @import("scenes/map_preview.zig");
    _ = @import("scenes/main_screen.zig");
    _ = @import("scenes/castle.zig");
    _ = @import("scenes/statistics.zig");
    _ = @import("scenes/prov_list.zig");
    _ = @import("scenes/castle_build.zig");
    _ = @import("scenes/garrison.zig");
    _ = @import("ui/army_ui.zig");
    _ = @import("scenes/army_info.zig");
    inline for (.{ @import("scenes/army_info.zig").UnitWindow, @import("scenes/army_info.zig").ResourceWindow }) |T| std.testing.refAllDecls(T);
    inline for (.{ @import("scenes/castle.zig").CastleScreen, @import("scenes/castle_build.zig").BuildPanel, @import("scenes/castle_build.zig").BuildInfo, @import("game/castle.zig").Rules, @import("scenes/garrison.zig").ArmyPanel, @import("scenes/garrison.zig").ProvinceGarrison, @import("ui/army_ui.zig").UnitBlock, @import("ui/army_ui.zig").RecruitBlock, @import("ui/castle_ui.zig").Trash, @import("ui/castle_ui.zig").Toggle }) |T| std.testing.refAllDecls(T);
    inline for (.{ @import("scenes/hero_recruit.zig").HeroTab, @import("scenes/hero_recruit.zig").RecruitWindow, @import("scenes/hero_recruit.zig").ResurrectWindow, @import("scenes/hero_death.zig").Death, @import("scenes/hero_death.zig").Arrival, @import("scenes/hero_screen.zig").HeroScreen, @import("scenes/hero_hero.zig").HeroHero, @import("game/quest.zig") }) |T| std.testing.refAllDecls(T);
    _ = @import("formats/varstream.zig");
    _ = @import("platform/fs.zig");
    _ = @import("platform/gfx.zig");
    _ = @import("platform/canvas.zig");
    _ = @import("platform/input.zig");
    _ = @import("platform/mouse_glfw.zig");
}

const t = std.testing;

test "cp1251 roundtrip" {
    try t.expectEqual(@as(u21, 0x0410), cp1251.toCodepoint(0xC0));
    try t.expectEqual(@as(u21, 0x044F), cp1251.toCodepoint(0xFF));
    try t.expectEqual(@as(u21, 0x0401), cp1251.toCodepoint(0xA8));
    for (0..256) |b| {
        const cp = cp1251.toCodepoint(@intCast(b));
        try t.expectEqual(@as(?u8, @intCast(b)), cp1251.fromCodepoint(cp));
    }
    const s = try cp1251.toUtf8Alloc(t.allocator, "\xcf\xf3\xf1\xf2\xee");
    defer t.allocator.free(s);
    try t.expectEqualStrings("Пусто", s);
}

test "profile pack roundtrip preserves the 212-byte layout" {
    const pk = @import("formats/packfile.zig");
    var raw: [212]u8 = @splat(0);
    @memcpy(raw[0..4], "123\x00");
    @memcpy(raw[0x15..0x1e], "ANONYMOUS");
    std.mem.writeInt(i32, raw[0x30..0x34], 0, .little);
    std.mem.writeInt(i32, raw[0x34..0x38], 2, .little);
    std.mem.writeInt(i32, raw[0x38..0x3c], 4, .little);
    std.mem.writeInt(i32, raw[0x3c..0x40], 7, .little);
    std.mem.writeInt(i32, raw[0x40..0x44], 17, .little);
    const encoded = try pk.pack(t.allocator, &raw);
    defer t.allocator.free(encoded);
    try t.expectEqualSlices(u8, "slh!", encoded[0..4]);
    const unpacked = try pk.unpack(t.allocator, encoded);
    defer t.allocator.free(unpacked);
    try t.expectEqualSlices(u8, &raw, unpacked);
}

test "lzss literal and back-reference" {
    // flags 0xFF = 8 literals
    const lit = [_]u8{ 0xFF, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
    const a = try lzss.decode(t.allocator, &lit, 8, null);
    defer t.allocator.free(a);
    try t.expectEqualStrings("abcdefgh", a);

    // 2 literals then a match copying from ring pos N-F (=4078) length 3+2+1-... :
    // flags: bit0=1 'x', bit1=1 'y', bit2=0 match(pos=4078,len=4)
    const pos: u16 = 4078;
    const len_code: u8 = 4 - 3;
    const src = [_]u8{ 0b011, 'x', 'y', @truncate(pos), @as(u8, @truncate(pos >> 4)) & 0xF0 | len_code };
    const b = try lzss.decode(t.allocator, &src, 8, null);
    defer t.allocator.free(b);
    try t.expectEqualStrings("xyxyxy", b);
}

test "datafile uncompressed BMP object" {
    var buf: std.ArrayList(u8) = .empty;
    defer buf.deinit(t.allocator);
    const A = t.allocator;
    try buf.appendSlice(A, "slh.ALL.");
    try buf.appendSlice(A, &std.mem.toBytes(std.mem.nativeToBig(i32, 1)));
    try buf.appendSlice(A, "propNAME");
    try buf.appendSlice(A, &std.mem.toBytes(std.mem.nativeToBig(i32, 3)));
    try buf.appendSlice(A, "Foo");
    try buf.appendSlice(A, "BMP ");
    const body = [_]u8{ 0, 24, 0, 2, 0, 1, 255, 0, 255, 1, 2, 3 };
    try buf.appendSlice(A, &std.mem.toBytes(std.mem.nativeToBig(i32, body.len)));
    try buf.appendSlice(A, &std.mem.toBytes(std.mem.nativeToBig(i32, body.len)));
    try buf.appendSlice(A, &body);

    var df = try datafile.Datafile.parse(A, buf.items);
    defer df.deinit();
    const o = df.find("foo").?;
    try t.expect(o.isBitmap());
    const img = try datafile.decodeBitmap(A, o.data);
    defer A.free(img.pixels);
    try t.expectEqual(@as(u32, 2), img.w);
    try t.expectEqualSlices(u8, &.{ 255, 0, 255, 0, 1, 2, 3, 255 }, img.pixels);
}

test "varfile records" {
    const src =
        "Quantity: 2\r\n\r\n/0\r\nName: A;\r\nLevel: 0\r\n\r\n/1 comment\r\nName: B;\r\nLevel: 7\r\nCoord: (1, 2; -3, 4)\r\n";
    var vf = try varfile.parse(t.allocator, src);
    defer vf.deinit();
    try t.expectEqual(@as(?u32, 2), vf.quantity);
    try t.expectEqual(@as(usize, 2), vf.records.len);
    const r = vf.byIndex(1).?;
    try t.expectEqualStrings("B", r.get("name").?);
    try t.expectEqual(@as(?i32, 7), r.getInt("Level"));
    const l = try varfile.parseIntList(t.allocator, r.get("Coord").?);
    defer t.allocator.free(l);
    try t.expectEqualSlices(i32, &.{ 1, 2, -3, 4 }, l);
}

test "textfile entries" {
    const textfile = @import("formats/textfile.zig");
    const src = "0. Empty\r\n#None#\r\n\r\n2. Second\r\n#Line one\r\nline two#\r\n";
    var tf = try textfile.parse(t.allocator, src);
    defer tf.deinit();
    try t.expectEqual(@as(usize, 2), tf.entries.len);
    const e = tf.byIndex(2).?;
    try t.expectEqualStrings("Second", e.title);
    try t.expectEqualStrings("Line one\nline two", e.body);
}

test "import: text conversion, paths, language" {
    const imp = @import("import.zig");
    const u = try imp.convertText(t.allocator, "Name: \xcf\xf3\xf1\xf2\xee;\r\n");
    defer t.allocator.free(u);
    try t.expectEqualStrings("Name: Пусто;\n", u);

    var b: [64]u8 = undefined;
    try t.expectEqualStrings("text/diplomacy/player0.txt", try imp.lowerPath(&b, "Text\\Diplomacy\\Player0.txt"));
    try t.expectEqual(imp.Kind.text, imp.classify("text/unit.txt"));
    try t.expectEqual(imp.Kind.dat, imp.classify("dat/town/town1.dat"));
    try t.expectEqual(imp.Kind.font_desc, imp.classify("font01.txt"));
    try t.expectEqual(imp.Kind.skip, imp.classify("eula.txt"));
    try t.expectEqual(imp.Kind.save, imp.classify("save/map0.map"));
    try t.expectEqual(imp.Kind.save, imp.classify("save/game0.sav"));
    try t.expectEqual(imp.Kind.skip, imp.classify("save/game0.bks"));
    try t.expectEqual(imp.Kind.save, imp.classify("save/player0.prf"));
    try t.expectEqual(imp.Kind.exe, imp.classify("eador.exe"));
    try t.expectEqual(imp.Kind.skip, imp.classify("unins000.exe"));
    try t.expectEqual(imp.Kind.cfg, imp.classify("eador.cfg"));
    try t.expectEqual(imp.Kind.cfg_fallback, imp.classify("__support/app/eador.cfg"));
    try t.expectEqualStrings("town/town1", imp.datBase("dat/town/town1.dat"));

    try t.expectEqualStrings("en", imp.languageFromGogInfo("{ \"language\": \"English\", }").?);
    try t.expectEqualStrings("ru", imp.languageFromGogInfo("\"language\" : \"Russian\"").?);
    try t.expect(imp.languageFromGogInfo("{ \"name\": \"Eador\" }") == null);

    // Only the objects that differ between releases are per-language.
    try t.expect(imp.isLangDat("stuff"));
    try t.expect(imp.isLangObject("stuff", 79));
    try t.expect(!imp.isLangObject("stuff", 78));
    try t.expect(!imp.isLangDat("castle"));
}

test "font range" {
    const r = try pcxfont.parseRange("font01.pcx 0x20 0xFF\r\n");
    try t.expectEqual(@as(u8, 0x20), r.first);
    try t.expectEqual(@as(u8, 0xFF), r.last);
}
