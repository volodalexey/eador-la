//! Access to the imported assets (output of eador-import):
//!
//!   <root>/<lang>/...   language files (var, text, save, localised sprites)
//!   <root>/common/...   files shared by every language
//!
//! A relative path is looked up in the current language first, then in
//! common/.  save/ is the exception: each language is its own installation,
//! so saves are read and written only in <root>/<lang>/save/.  Paths are
//! lower-case with '/' separators.
const std = @import("std");
const Io = std.Io;

pub const max_langs = 8;

pub const GameFs = struct {
    io: Io,
    gpa: std.mem.Allocator,
    root: Io.Dir,
    common: Io.Dir,
    lang_dir: Io.Dir,
    /// Imported languages (directories with a manifest.txt), sorted.
    langs: [max_langs][8]u8 = undefined,
    lang_lens: [max_langs]u8 = undefined,
    n_langs: usize = 0,
    current: usize = 0,

    /// `prefer` selects the language; null or unknown takes "ru" if present,
    /// otherwise the first one.
    pub fn open(io: Io, gpa: std.mem.Allocator, path: []const u8, prefer: ?[]const u8) !GameFs {
        var root = try Io.Dir.cwd().openDir(io, path, .{ .iterate = true });
        errdefer root.close(io);
        const common = root.openDir(io, "common", .{ .iterate = true }) catch return error.NotImported;
        var fs: GameFs = .{ .io = io, .gpa = gpa, .root = root, .common = common, .lang_dir = undefined };
        var it = root.iterate();
        while (try it.next(io)) |e| {
            if (e.kind != .directory or e.name.len > 8 or fs.n_langs == max_langs) continue;
            var d = root.openDir(io, e.name, .{}) catch continue;
            defer d.close(io);
            _ = d.statFile(io, "manifest.txt", .{}) catch continue;
            @memcpy(fs.langs[fs.n_langs][0..e.name.len], e.name);
            fs.lang_lens[fs.n_langs] = @intCast(e.name.len);
            fs.n_langs += 1;
        }
        if (fs.n_langs == 0) return error.NotImported;
        fs.sortLangs();
        fs.current = fs.find(prefer orelse "ru") orelse fs.find("ru") orelse 0;
        fs.lang_dir = try root.openDir(io, fs.lang(), .{ .iterate = true });
        return fs;
    }

    fn sortLangs(self: *GameFs) void {
        var i: usize = 1;
        while (i < self.n_langs) : (i += 1) {
            var j = i;
            while (j > 0 and std.mem.lessThan(u8, self.langAt(j), self.langAt(j - 1))) : (j -= 1) {
                std.mem.swap([8]u8, &self.langs[j], &self.langs[j - 1]);
                std.mem.swap(u8, &self.lang_lens[j], &self.lang_lens[j - 1]);
            }
        }
    }

    fn langAt(self: *const GameFs, i: usize) []const u8 {
        return self.langs[i][0..self.lang_lens[i]];
    }

    fn find(self: *const GameFs, code: []const u8) ?usize {
        for (0..self.n_langs) |i| if (std.mem.eql(u8, self.langAt(i), code)) return i;
        return null;
    }

    /// Current language code ("ru", "en", ...).
    pub fn lang(self: *const GameFs) []const u8 {
        return self.langAt(self.current);
    }

    /// Switches to the next imported language (wraps around).
    pub fn nextLang(self: *GameFs) !void {
        const next = (self.current + 1) % self.n_langs;
        const d = try self.root.openDir(self.io, self.langAt(next), .{ .iterate = true });
        self.lang_dir.close(self.io);
        self.lang_dir = d;
        self.current = next;
    }

    pub fn close(self: *GameFs) void {
        self.lang_dir.close(self.io);
        self.common.close(self.io);
        self.root.close(self.io);
    }

    /// Saves (save/...) belong to one language only and never fall back to
    /// common/: each language is a separate installation of the game.
    fn isSave(rel: []const u8) bool {
        return std.mem.startsWith(u8, rel, "save/");
    }

    /// Caller owns the returned bytes.
    pub fn read(self: *GameFs, rel: []const u8) ![]u8 {
        return self.lang_dir.readFileAlloc(self.io, rel, self.gpa, .unlimited) catch |e| switch (e) {
            error.FileNotFound => if (isSave(rel)) e else self.common.readFileAlloc(self.io, rel, self.gpa, .unlimited),
            else => e,
        };
    }

    /// File size in bytes, 0 if it does not exist.
    pub fn size(self: *GameFs, rel: []const u8) u64 {
        const st = self.lang_dir.statFile(self.io, rel, .{}) catch {
            if (isSave(rel)) return 0;
            return (self.common.statFile(self.io, rel, .{}) catch return 0).size;
        };
        return st.size;
    }

    /// The file exists, even if empty (al_findfirst).
    pub fn exists(self: *GameFs, rel: []const u8) bool {
        _ = self.lang_dir.statFile(self.io, rel, .{}) catch {
            if (isSave(rel)) return false;
            _ = self.common.statFile(self.io, rel, .{}) catch return false;
            return true;
        };
        return true;
    }

    /// Writes a file of the current language (save/...), creating its
    /// directory.
    pub fn write(self: *GameFs, rel: []const u8, data: []const u8) !void {
        if (std.fs.path.dirname(rel)) |dir| try self.lang_dir.createDirPath(self.io, dir);
        try self.lang_dir.writeFile(self.io, .{ .sub_path = rel, .data = data });
    }

    /// Deletes a file of the current language (save/...); a missing file is
    /// not an error.
    pub fn delete(self: *GameFs, rel: []const u8) !void {
        self.lang_dir.deleteFile(self.io, rel) catch |e| switch (e) {
            error.FileNotFound => {},
            else => return e,
        };
    }

    /// Sprite sheets ("interface", "town/town1", ...): every directory under
    /// sprites/ with an index.txt, from both roots, sorted, without duplicates.
    pub fn listSheets(self: *GameFs, arena: std.mem.Allocator) ![]const []const u8 {
        var out: std.ArrayList([]const u8) = .empty;
        for ([_]Io.Dir{ self.common, self.lang_dir }) |base| {
            var dir = base.openDir(self.io, "sprites", .{ .iterate = true }) catch continue;
            defer dir.close(self.io);
            var w = try dir.walk(arena);
            defer w.deinit();
            while (try w.next(self.io)) |e| {
                if (e.kind != .file or !std.mem.eql(u8, e.basename, "index.txt")) continue;
                const name = std.fs.path.dirname(e.path) orelse continue;
                const seen = for (out.items) |s| {
                    if (std.mem.eql(u8, s, name)) break true;
                } else false;
                if (!seen) try out.append(arena, try arena.dupe(u8, name));
            }
        }
        std.mem.sort([]const u8, out.items, {}, struct {
            fn lt(_: void, a: []const u8, b: []const u8) bool {
                return std.mem.lessThan(u8, a, b);
            }
        }.lt);
        return out.items;
    }
};

/// One line of a sprites/<sheet>/index.txt or sound/<dat>/index.txt:
/// "<object index>\t<name>[\t<w>\t<h>]".
pub const IndexEntry = struct { index: u32, name: []const u8, w: u32 = 0, h: u32 = 0 };

/// Parses an index file; names point into `text`.
pub fn parseIndex(gpa: std.mem.Allocator, text: []const u8) ![]IndexEntry {
    var list: std.ArrayList(IndexEntry) = .empty;
    errdefer list.deinit(gpa);
    var lines = std.mem.tokenizeScalar(u8, text, '\n');
    while (lines.next()) |l| {
        var f = std.mem.splitScalar(u8, l, '\t');
        const idx = std.fmt.parseInt(u32, f.next() orelse continue, 10) catch continue;
        const name = f.next() orelse continue;
        var e: IndexEntry = .{ .index = idx, .name = name };
        if (f.next()) |w| e.w = std.fmt.parseInt(u32, w, 10) catch 0;
        if (f.next()) |h| e.h = std.fmt.parseInt(u32, h, 10) catch 0;
        try list.append(gpa, e);
    }
    return list.toOwnedSlice(gpa);
}

/// "sound/spell_sound" + 46 -> "sound/spell_sound/046_S046.ogg" using the
/// directory's index.txt.  Null when the object is not listed.
pub fn indexedPath(fs: *GameFs, buf: []u8, dir: []const u8, index: u32, ext: []const u8) ?[]const u8 {
    var pb: [256]u8 = undefined;
    const text = fs.read(std.fmt.bufPrint(&pb, "{s}/index.txt", .{dir}) catch return null) catch return null;
    defer fs.gpa.free(text);
    const entries = parseIndex(fs.gpa, text) catch return null;
    defer fs.gpa.free(entries);
    for (entries) |e| {
        if (e.index == index) return std.fmt.bufPrint(buf, "{s}/{d:0>3}_{s}{s}", .{ dir, index, e.name, ext }) catch null;
    }
    return null;
}

test "saves are per language and never come from common/" {
    const io = std.testing.io;
    const gpa = std.testing.allocator;
    var tmp = std.testing.tmpDir(.{});
    defer tmp.cleanup();
    for ([_][]const u8{ "ru", "en" }) |l| {
        try tmp.dir.createDirPath(io, l);
        var pb: [32]u8 = undefined;
        try tmp.dir.writeFile(io, .{ .sub_path = try std.fmt.bufPrint(&pb, "{s}/manifest.txt", .{l}), .data = "" });
    }
    try tmp.dir.createDirPath(io, "common/save");
    try tmp.dir.writeFile(io, .{ .sub_path = "common/save/player0.prf", .data = "common" });
    try tmp.dir.writeFile(io, .{ .sub_path = "common/shared.txt", .data = "shared" });

    var rb: [128]u8 = undefined;
    const root = try std.fmt.bufPrint(&rb, "{s}/{s}", .{ std.testing.TmpDir.parent_dir_path, &tmp.sub_path });
    var fs = try GameFs.open(io, gpa, root, "ru");
    defer fs.close();

    try std.testing.expectError(error.FileNotFound, fs.read("save/player0.prf"));
    try std.testing.expectEqual(@as(u64, 0), fs.size("save/player0.prf"));
    const shared = try fs.read("shared.txt");
    defer gpa.free(shared);
    try std.testing.expectEqualStrings("shared", shared);

    try fs.write("save/player0.prf", "ru");
    const ru = try fs.read("save/player0.prf");
    defer gpa.free(ru);
    try std.testing.expectEqualStrings("ru", ru);

    try fs.nextLang();
    try std.testing.expectEqualStrings("en", fs.lang());
    try std.testing.expectError(error.FileNotFound, fs.read("save/player0.prf"));
    try std.testing.expectEqual(@as(u64, 0), fs.size("save/player0.prf"));

    // Deleting in en leaves ru's profile; deleting a missing file is fine.
    try fs.delete("save/player0.prf");
    try fs.nextLang();
    try std.testing.expectEqual(@as(u64, 2), fs.size("save/player0.prf"));
    try fs.delete("save/player0.prf");
    try std.testing.expectEqual(@as(u64, 0), fs.size("save/player0.prf"));
}

test "index lines" {
    const gpa = std.testing.allocator;
    const e = try parseIndex(gpa, "0\t00ItemMask\t58\t58\n46\tS046\n");
    defer gpa.free(e);
    try std.testing.expectEqual(@as(usize, 2), e.len);
    try std.testing.expectEqualStrings("00ItemMask", e[0].name);
    try std.testing.expectEqual(@as(u32, 58), e[0].h);
    try std.testing.expectEqual(@as(u32, 46), e[1].index);
}
