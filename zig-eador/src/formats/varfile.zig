//! Parser for the game's text tables (var/*.var, CP1251).
//!
//! Layout:
//!   Quantity: N          (optional header)
//!   /0 optional comment
//!   Key: value;          (trailing ';' optional)
//!   ...
//!   /1
//!   ...
//! Values are kept as raw CP1251 bytes; callers convert with cp1251.zig.
const std = @import("std");

pub const Field = struct { key: []const u8, value: []const u8 };

pub const Record = struct {
    index: u32,
    fields: []const Field,

    pub fn get(self: Record, key: []const u8) ?[]const u8 {
        for (self.fields) |f| {
            if (std.ascii.eqlIgnoreCase(f.key, key)) return f.value;
        }
        return null;
    }

    pub fn getInt(self: Record, key: []const u8) ?i32 {
        const v = self.get(key) orelse return null;
        return std.fmt.parseInt(i32, std.mem.trim(u8, v, " ;"), 10) catch null;
    }
};

pub const VarFile = struct {
    arena: std.heap.ArenaAllocator,
    quantity: ?u32,
    records: []const Record,

    pub fn deinit(self: *VarFile) void {
        self.arena.deinit();
    }

    pub fn byIndex(self: *const VarFile, idx: u32) ?Record {
        for (self.records) |r| {
            if (r.index == idx) return r;
        }
        return null;
    }
};

/// Parses `text`. Returned slices point into an arena-owned copy of `text`.
pub fn parse(gpa: std.mem.Allocator, text: []const u8) !VarFile {
    var arena = std.heap.ArenaAllocator.init(gpa);
    errdefer arena.deinit();
    const a = arena.allocator();
    const src = try a.dupe(u8, text);

    var records: std.ArrayList(Record) = .empty;
    var fields: std.ArrayList(Field) = .empty;
    var cur_index: ?u32 = null;
    var quantity: ?u32 = null;

    var it = std.mem.splitScalar(u8, src, '\n');
    while (it.next()) |raw_line| {
        const line = std.mem.trim(u8, raw_line, " \t\r");
        if (line.len == 0 or std.mem.startsWith(u8, line, "//")) continue;

        if (line[0] == '/' and line.len > 1 and std.ascii.isDigit(line[1])) {
            if (cur_index) |idx| {
                try records.append(a, .{ .index = idx, .fields = try fields.toOwnedSlice(a) });
            }
            var end: usize = 1;
            while (end < line.len and std.ascii.isDigit(line[end])) end += 1;
            cur_index = std.fmt.parseInt(u32, line[1..end], 10) catch null;
            continue;
        }

        const colon = std.mem.indexOfScalar(u8, line, ':') orelse continue;
        const key = std.mem.trim(u8, line[0..colon], " \t");
        var value = std.mem.trim(u8, line[colon + 1 ..], " \t");
        if (value.len > 0 and value[value.len - 1] == ';') value = value[0 .. value.len - 1];

        if (cur_index == null) {
            if (std.ascii.eqlIgnoreCase(key, "Quantity")) {
                quantity = std.fmt.parseInt(u32, value, 10) catch null;
            }
            continue;
        }
        try fields.append(a, .{ .key = key, .value = value });
    }
    if (cur_index) |idx| {
        try records.append(a, .{ .index = idx, .fields = try fields.toOwnedSlice(a) });
    }
    return .{ .arena = arena, .quantity = quantity, .records = try records.toOwnedSlice(a) };
}

/// Parses "(a, b; c, d)" style lists into integers, ignoring separators.
pub fn parseIntList(gpa: std.mem.Allocator, value: []const u8) ![]i32 {
    var out: std.ArrayList(i32) = .empty;
    errdefer out.deinit(gpa);
    var i: usize = 0;
    while (i < value.len) {
        const c = value[i];
        if (std.ascii.isDigit(c) or (c == '-' and i + 1 < value.len and std.ascii.isDigit(value[i + 1]))) {
            var j = i + 1;
            while (j < value.len and std.ascii.isDigit(value[j])) j += 1;
            try out.append(gpa, std.fmt.parseInt(i32, value[i..j], 10) catch 0);
            i = j;
        } else i += 1;
    }
    return out.toOwnedSlice(gpa);
}
