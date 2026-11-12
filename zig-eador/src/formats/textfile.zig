//! Parser for text/*.txt description files (CP1251):
//!   N. Title
//!   #Body text, may span lines#
const std = @import("std");

pub const Entry = struct {
    index: u32,
    title: []const u8,
    body: []const u8,
};

pub const TextFile = struct {
    arena: std.heap.ArenaAllocator,
    entries: []const Entry,

    pub fn deinit(self: *TextFile) void {
        self.arena.deinit();
    }

    pub fn byIndex(self: *const TextFile, idx: u32) ?Entry {
        for (self.entries) |e| {
            if (e.index == idx) return e;
        }
        return null;
    }
};

pub fn parse(gpa: std.mem.Allocator, text: []const u8) !TextFile {
    var arena = std.heap.ArenaAllocator.init(gpa);
    errdefer arena.deinit();
    const a = arena.allocator();
    var entries: std.ArrayList(Entry) = .empty;

    var i: usize = 0;
    while (i < text.len) {
        // Start of a line: expect "N." or "N " header.
        const line_end = std.mem.indexOfScalarPos(u8, text, i, '\n') orelse text.len;
        const line = std.mem.trim(u8, text[i..line_end], " \t\r");
        var d: usize = 0;
        while (d < line.len and std.ascii.isDigit(line[d])) d += 1;
        if (d == 0 or line.len == d or (line[d] != '.' and line[d] != ' ')) {
            i = line_end + 1;
            continue;
        }
        const idx = std.fmt.parseInt(u32, line[0..d], 10) catch {
            i = line_end + 1;
            continue;
        };
        const title = std.mem.trim(u8, line[d + 1 ..], " \t");
        i = line_end + 1;

        var body: []const u8 = "";
        if (std.mem.indexOfScalarPos(u8, text, i, '#')) |open| {
            if (std.mem.indexOfScalarPos(u8, text, open + 1, '#')) |close| {
                // Normalise CRLF and line breaks inside the body to '\n'.
                var buf: std.ArrayList(u8) = .empty;
                for (text[open + 1 .. close]) |c| {
                    if (c != '\r') try buf.append(a, c);
                }
                body = buf.items;
                i = close + 1;
            }
        }
        try entries.append(a, .{ .index = idx, .title = title, .body = body });
    }
    return .{ .arena = arena, .entries = entries.items };
}
