//! eador-import: converts a GOG copy of Eador: Genesis into the files the
//! engine reads directly.
//!
//!   eador-import --exe  <GOG installer .exe> <assets_dir>
//!   eador-import --game <installed GOG game dir> <assets_dir>
//!
//! The language comes from "language" in goggame-*.info; without it the
//! import fails.  Output (all paths lower-case, '/' separators):
//!
//!   <lang>/manifest.txt               Language / Format
//!   <lang>/var/*.var, <lang>/text/**  CP1251 -> UTF-8, '\r' removed
//!   <lang>/save/*.sav, *.map, *.prf   copied (packed saves, player profile)
//!   common/eador.cfg                  CP1251 -> UTF-8
//!   common/fonts/fontNN.fnt + .png    BMFont text + RGBA atlas (raylib LoadFont)
//!   common/sprites/<dat>/NNN_<name>.png, index.txt   one PNG per bitmap object
//!   common/music/<name>.ogg           music.dat
//!   common/sound/<dat>/NNN_<name>.ogg, index.txt     other Ogg objects
//!   common/dat/**.dat                 originals, for tools/*.py only
//!   common/icon.png                   Eador.exe's icon (the window icon)
//!
//! Language-dependent artwork (see `lang_objects`) and its .dat go to <lang>/
//! instead of common/.  An existing file with the same size is left alone, so
//! re-importing the same or another language does not rewrite common/.
const std = @import("std");
const Io = std.Io;
const cp1251 = @import("formats/cp1251.zig");
const pcxfont = @import("formats/pcxfont.zig");
const datafile = @import("formats/datafile.zig");
const png = @import("formats/png.zig");
const inno = @import("formats/inno.zig");
const peicon = @import("formats/peicon.zig");

pub const format_version = 2;

/// Objects that differ between the Russian and English releases (found by
/// comparing both): delete/trash buttons, the main menu frame, the race
/// creation plaques and the "a game by" screen.  Everything else in the
/// datafiles is byte-identical.
pub const lang_objects = [_]struct { dat: []const u8, objects: []const u16 }{
    .{ .dat = "buttons", .objects = &.{ 229, 230, 231, 232 } },
    .{ .dat = "interface", .objects = &.{ 8, 9, 10, 133 } },
    .{ .dat = "stuff", .objects = &.{ 36, 37, 38, 39, 79 } },
};

pub fn isLangDat(base: []const u8) bool {
    for (lang_objects) |e| if (std.mem.eql(u8, e.dat, base)) return true;
    return false;
}

pub fn isLangObject(base: []const u8, index: usize) bool {
    for (lang_objects) |e| {
        if (!std.mem.eql(u8, e.dat, base)) continue;
        for (e.objects) |o| if (o == index) return true;
    }
    return false;
}

/// CP1251 -> UTF-8, dropping '\r'.
pub fn convertText(gpa: std.mem.Allocator, src: []const u8) ![]u8 {
    var out: std.ArrayList(u8) = .empty;
    errdefer out.deinit(gpa);
    try out.ensureTotalCapacity(gpa, src.len + src.len / 2);
    var buf: [4]u8 = undefined;
    for (src) |b| {
        if (b == '\r') continue;
        const n = std.unicode.utf8Encode(cp1251.toCodepoint(b), &buf) catch unreachable;
        try out.appendSlice(gpa, buf[0..n]);
    }
    return out.toOwnedSlice(gpa);
}

/// ASCII-lowercase and normalise separators to '/'.
pub fn lowerPath(buf: []u8, path: []const u8) ![]const u8 {
    if (path.len > buf.len) return error.NameTooLong;
    for (path, 0..) |c, i| buf[i] = if (c == '\\') '/' else std.ascii.toLower(c);
    return buf[0..path.len];
}

pub const Kind = enum { text, dat, save, font_desc, cfg, cfg_fallback, exe, skip };

/// `rel` must already be lower-cased.
pub fn classify(rel: []const u8) Kind {
    const root_file = std.mem.indexOfScalar(u8, rel, '/') == null;
    if (std.mem.startsWith(u8, rel, "var/") and std.mem.endsWith(u8, rel, ".var")) return .text;
    if (std.mem.startsWith(u8, rel, "text/") and std.mem.endsWith(u8, rel, ".txt")) return .text;
    if (std.mem.startsWith(u8, rel, "dat/") and std.mem.endsWith(u8, rel, ".dat")) return .dat;
    if (root_file and std.mem.eql(u8, rel, "eador.cfg")) return .cfg;
    if (root_file and std.mem.eql(u8, rel, "eador.exe")) return .exe;
    // The installer keeps the default config here; the game copies it to
    // the root on first start.
    if (std.mem.eql(u8, rel, "__support/app/eador.cfg")) return .cfg_fallback;
    if (std.mem.startsWith(u8, rel, "save/") and
        (std.mem.endsWith(u8, rel, ".map") or std.mem.endsWith(u8, rel, ".sav") or
            std.mem.endsWith(u8, rel, ".prf"))) return .save;
    if (root_file and std.mem.startsWith(u8, rel, "font") and std.mem.endsWith(u8, rel, ".txt")) return .font_desc;
    return .skip;
}

/// Language code from GOG's goggame-*.info ("language": "English").
pub fn languageFromGogInfo(info: []const u8) ?[]const u8 {
    const key = "\"language\"";
    const k = std.mem.indexOf(u8, info, key) orelse return null;
    const q1 = std.mem.indexOfScalarPos(u8, info, k + key.len, '"') orelse return null;
    const q2 = std.mem.indexOfScalarPos(u8, info, q1 + 1, '"') orelse return null;
    const name = info[q1 + 1 .. q2];
    const table = [_][2][]const u8{
        .{ "English", "en" }, .{ "Russian", "ru" }, .{ "German", "de" },
        .{ "Polish", "pl" },  .{ "French", "fr" },  .{ "Czech", "cs" },
    };
    for (table) |e| if (std.ascii.eqlIgnoreCase(e[0], name)) return e[1];
    return null;
}

/// "dat/town/Town1.dat" (lower-cased) -> "town/town1".
pub fn datBase(rel: []const u8) []const u8 {
    return rel["dat/".len .. rel.len - ".dat".len];
}

/// One output root (`<assets>/<lang>` or `<assets>/common`).
const Out = struct {
    io: Io,
    dir: Io.Dir,
    written: usize = 0,
    skipped: usize = 0,

    /// Writes `data` unless a file of the same size already exists.
    fn put(self: *Out, rel: []const u8, data: []const u8) !void {
        if (self.dir.statFile(self.io, rel, .{})) |st| {
            if (st.size == data.len) {
                self.skipped += 1;
                return;
            }
        } else |_| {}
        if (std.fs.path.dirname(rel)) |d| try self.dir.createDirPath(self.io, d);
        try self.dir.writeFile(self.io, .{ .sub_path = rel, .data = data });
        self.written += 1;
    }
};

/// Eador.exe's icon as common/icon.png (what Wine shows for the game).
fn importIcon(gpa: std.mem.Allocator, exe: []const u8, common: *Out) !void {
    var png_data: ?[]const u8 = null;
    if (try peicon.extract(gpa, exe, &png_data)) |img| {
        defer gpa.free(img.rgba);
        const enc = try png.encodeRgba(gpa, img.w, img.h, img.rgba);
        defer gpa.free(enc);
        try common.put("icon.png", enc);
    } else if (png_data) |d| try common.put("icon.png", d);
}

/// Writes every object of one datafile as a separate PNG / Ogg file.
fn importDat(gpa: std.mem.Allocator, rel: []const u8, bytes: []const u8, common: *Out, lang: *Out) !void {
    const base = datBase(rel);
    try (if (isLangDat(base)) lang else common).put(rel, bytes);

    var df = try datafile.Datafile.parse(gpa, bytes);
    defer df.deinit();
    var sprites: std.ArrayList(u8) = .empty;
    defer sprites.deinit(gpa);
    var sounds: std.ArrayList(u8) = .empty;
    defer sounds.deinit(gpa);
    var pb: [256]u8 = undefined;
    for (df.objects, 0..) |o, i| {
        if (o.isBitmap()) {
            const img = datafile.decodeBitmap(gpa, o.data) catch continue;
            defer gpa.free(img.pixels);
            const bytes_png = try png.encodeRgba(gpa, img.w, img.h, img.pixels);
            defer gpa.free(bytes_png);
            const path = try std.fmt.bufPrint(&pb, "sprites/{s}/{d:0>3}_{s}.png", .{ base, i, o.name });
            try (if (isLangObject(base, i)) lang else common).put(path, bytes_png);
            try sprites.print(gpa, "{d}\t{s}\t{d}\t{d}\n", .{ i, o.name, img.w, img.h });
        } else if (o.isOgg()) {
            const path = if (std.mem.eql(u8, base, "music"))
                try std.fmt.bufPrint(&pb, "music/{s}.ogg", .{o.name})
            else
                try std.fmt.bufPrint(&pb, "sound/{s}/{d:0>3}_{s}.ogg", .{ base, i, o.name });
            try common.put(path, o.data);
            try sounds.print(gpa, "{d}\t{s}\n", .{ i, o.name });
        }
    }
    // The index keeps object positions: screens address sprites by index
    // (innerbld.dat: index = building id) as well as by name.
    if (sprites.items.len > 0) {
        const path = try std.fmt.bufPrint(&pb, "sprites/{s}/index.txt", .{base});
        try common.put(path, sprites.items);
    }
    if (sounds.items.len > 0 and !std.mem.eql(u8, base, "music")) {
        const path = try std.fmt.bufPrint(&pb, "sound/{s}/index.txt", .{base});
        try common.put(path, sounds.items);
    }
}

/// fontNN.txt ("font01.pcx 0x20 0xFF", CP1251 codes) + the PCX grid ->
/// fonts/fontNN.png (white ink, transparent background) and an AngelCode
/// BMFont text file with Unicode code points.
fn importFont(gpa: std.mem.Allocator, io: Io, dir: Io.Dir, desc: []const u8, rel: []const u8, common: *Out) !void {
    const range = try pcxfont.parseRange(desc);
    var it = std.mem.tokenizeAny(u8, desc, " \t\r\n");
    const pcx_name = it.next() orelse return error.BadFontDesc;
    const pcx = dir.readFileAlloc(io, pcx_name, gpa, .unlimited) catch |e| blk: {
        var lb: [64]u8 = undefined;
        const lower = lowerPath(&lb, pcx_name) catch return e;
        break :blk try dir.readFileAlloc(io, lower, gpa, .unlimited);
    };
    defer gpa.free(pcx);
    const count = @as(usize, range.last) - range.first + 1;
    var f = try pcxfont.load(gpa, pcx, count);
    defer f.deinit(gpa);

    const name = rel[0 .. rel.len - ".txt".len];
    var pb: [64]u8 = undefined;
    const atlas = try png.encodeRgba(gpa, f.width, f.height, f.pixels);
    defer gpa.free(atlas);
    try common.put(try std.fmt.bufPrint(&pb, "fonts/{s}.png", .{name}), atlas);

    var fnt: std.ArrayList(u8) = .empty;
    defer fnt.deinit(gpa);
    try fnt.print(gpa, "info face=\"{s}\" size={d} bold=0 italic=0 charset=\"\" unicode=1 stretchH=100 smooth=0 aa=1 padding=0,0,0,0 spacing=0,0\n", .{ name, f.line_h });
    try fnt.print(gpa, "common lineHeight={d} base={d} scaleW={d} scaleH={d} pages=1 packed=0\n", .{ f.line_h, f.line_h, f.width, f.height });
    try fnt.print(gpa, "page id=0 file=\"{s}.png\"\n", .{name});
    try fnt.print(gpa, "chars count={d}\n", .{f.glyphs.len});
    for (f.glyphs, 0..) |g, k| {
        const cp = cp1251.toCodepoint(@intCast(range.first + k));
        // Allegro glyphs are the whole grid cell: the text advances by the
        // cell width (font01: 9 px for every character, as on screenshots).
        try fnt.print(gpa, "char id={d} x={d} y={d} width={d} height={d} xoffset=0 yoffset=0 xadvance={d} page=0 chnl=15\n", .{ cp, g.x, g.y, g.w, g.h, g.cell_w });
    }
    try common.put(try std.fmt.bufPrint(&pb, "fonts/{s}.fnt", .{name}), fnt.items);
}

fn fail(comptime fmt: []const u8, args: anytype) noreturn {
    std.debug.print("eador-import: " ++ fmt ++ "\n", args);
    std.process.exit(1);
}

fn detectLanguage(io: Io, gpa: std.mem.Allocator, game: Io.Dir) !?[]const u8 {
    var it = game.iterate();
    while (try it.next(io)) |e| {
        if (e.kind != .file or !std.ascii.startsWithIgnoreCase(e.name, "goggame-") or
            !std.ascii.endsWithIgnoreCase(e.name, ".info")) continue;
        const info = try game.readFileAlloc(io, e.name, gpa, .limited(1 << 20));
        defer gpa.free(info);
        if (languageFromGogInfo(info)) |l| return l;
    }
    return null;
}

const tmp_dir = ".gog_extract";

pub fn main(init: std.process.Init) !void {
    const gpa = init.gpa;
    const io = init.io;
    const args = try init.minimal.args.toSlice(init.arena.allocator());
    const usage = "usage: eador-import (--exe <installer.exe> | --game <game_dir>) <assets_dir>";
    if (args.len != 4) fail(usage, .{});
    const from_exe = std.mem.eql(u8, args[1], "--exe");
    if (!from_exe and !std.mem.eql(u8, args[1], "--game")) fail(usage, .{});
    const cwd = Io.Dir.cwd();

    var assets = try cwd.createDirPathOpen(io, args[3], .{});
    defer assets.close(io);

    // An installer is unpacked into assets/.gog_extract first and then
    // imported like an installed copy.
    if (from_exe) {
        const exe = cwd.readFileAlloc(io, args[2], gpa, .unlimited) catch |e|
            fail("cannot read installer '{s}': {t}", .{ args[2], e });
        defer gpa.free(exe);
        assets.deleteTree(io, tmp_dir) catch {};
        var t = try assets.createDirPathOpen(io, tmp_dir, .{});
        defer t.close(io);
        _ = inno.extract(gpa, io, exe, t) catch |e| fail("cannot unpack '{s}': {t}", .{ args[2], e });
    }
    defer if (from_exe) assets.deleteTree(io, tmp_dir) catch {};
    var game = (if (from_exe) assets.openDir(io, tmp_dir, .{ .iterate = true }) else cwd.openDir(io, args[2], .{ .iterate = true })) catch |e|
        fail("cannot open game dir '{s}': {t}", .{ args[2], e });
    defer game.close(io);

    const lang = (try detectLanguage(io, gpa, game)) orelse
        fail("'{s}': no \"language\" in goggame-*.info, cannot tell the game language", .{args[2]});

    var lang_dir = try assets.createDirPathOpen(io, lang, .{});
    defer lang_dir.close(io);
    var common_dir = try assets.createDirPathOpen(io, "common", .{});
    defer common_dir.close(io);
    var lo: Out = .{ .io = io, .dir = lang_dir };
    var co: Out = .{ .io = io, .dir = common_dir };

    var have_interface = false;
    var have_cfg = false;
    var cfg_fallback: ?[]u8 = null;
    defer if (cfg_fallback) |c| gpa.free(c);

    var walker = try game.walk(gpa);
    defer walker.deinit();
    while (try walker.next(io)) |e| {
        if (e.kind != .file) continue;
        var lb: [512]u8 = undefined;
        const rel = lowerPath(&lb, e.path) catch continue;
        const kind = classify(rel);
        if (kind == .skip) continue;
        const src = try e.dir.readFileAlloc(io, e.basename, gpa, .unlimited);
        defer gpa.free(src);
        switch (kind) {
            .skip => unreachable,
            .dat => {
                importDat(gpa, rel, src, &co, &lo) catch |err| fail("{s}: {t}", .{ rel, err });
                if (std.mem.eql(u8, rel, "dat/interface.dat")) have_interface = true;
            },
            .text, .cfg => {
                const utf8 = try convertText(gpa, src);
                defer gpa.free(utf8);
                if (kind == .cfg) {
                    try co.put(rel, utf8);
                    have_cfg = true;
                } else try lo.put(rel, utf8);
            },
            .cfg_fallback => cfg_fallback = try convertText(gpa, src),
            .save => try lo.put(rel, src),
            .font_desc => importFont(gpa, io, e.dir, src, rel, &co) catch |err| fail("{s}: {t}", .{ rel, err }),
            // The icon is optional: a damaged exe only loses it.
            .exe => importIcon(gpa, src, &co) catch {},
        }
    }
    if (!have_interface) fail("'{s}' is not an Eador: Genesis copy (dat/interface.dat not found)", .{args[2]});
    if (!have_cfg) if (cfg_fallback) |c| try co.put("eador.cfg", c);

    var mb: [128]u8 = undefined;
    try lo.put("manifest.txt", try std.fmt.bufPrint(&mb, "Language: {s}\nFormat: {d}\n", .{ lang, format_version }));
    // Silent by default: the build runner treats output from this step as a failure.
    if (init.environ_map.get("EADOR_IMPORT_VERBOSE") != null) {
        std.debug.print("eador-import: '{s}': {d} written, {d} unchanged in {s}/; {d} written, {d} unchanged in common/\n", .{ lang, lo.written, lo.skipped, lang, co.written, co.skipped });
    }
}
