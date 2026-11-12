//! Windows-1251 <-> Unicode. All game texts (.var/.txt) are stored in CP1251.
const std = @import("std");

/// Code points for bytes 0x80..0xBF. 0xC0..0xFF map linearly to U+0410..U+044F.
const high: [64]u16 = .{
    0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021, 0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
    0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0098, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
    0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7, 0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
    0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7, 0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
};

pub fn toCodepoint(b: u8) u21 {
    if (b < 0x80) return b;
    if (b >= 0xC0) return 0x0410 + @as(u21, b - 0xC0);
    return high[b - 0x80];
}

pub fn fromCodepoint(cp: u21) ?u8 {
    if (cp < 0x80) return @intCast(cp);
    if (cp >= 0x0410 and cp <= 0x044F) return @intCast(cp - 0x0410 + 0xC0);
    for (high, 0..) |h, i| {
        if (h == cp) return @intCast(0x80 + i);
    }
    return null;
}

pub fn toUtf8Alloc(gpa: std.mem.Allocator, src: []const u8) ![]u8 {
    var out: std.ArrayList(u8) = .empty;
    errdefer out.deinit(gpa);
    try out.ensureTotalCapacity(gpa, src.len * 2);
    var buf: [4]u8 = undefined;
    for (src) |b| {
        if (b < 0x80) {
            try out.append(gpa, b);
        } else {
            const n = std.unicode.utf8Encode(toCodepoint(b), &buf) catch unreachable;
            try out.appendSlice(gpa, buf[0..n]);
        }
    }
    return out.toOwnedSlice(gpa);
}

/// UTF-8 to CP1251; characters without a CP1251 byte become '?'.
pub fn fromUtf8Alloc(gpa: std.mem.Allocator, src: []const u8) ![]u8 {
    var out: std.ArrayList(u8) = .empty;
    errdefer out.deinit(gpa);
    try out.ensureTotalCapacity(gpa, src.len);
    var i: usize = 0;
    while (i < src.len) try out.append(gpa, fromCodepoint(nextCodepoint(src, &i)) orelse '?');
    return out.toOwnedSlice(gpa);
}

/// Minimal UTF-8 decoder; invalid sequences yield U+FFFD and advance one byte.
pub fn nextCodepoint(s: []const u8, i: *usize) u21 {
    const b = s[i.*];
    const len: usize = if (b < 0x80) 1 else if (b >> 5 == 0b110) 2 else if (b >> 4 == 0b1110) 3 else if (b >> 3 == 0b11110) 4 else 0;
    if (len == 0 or i.* + len > s.len) {
        i.* += 1;
        return 0xFFFD;
    }
    const cp = std.unicode.utf8Decode(s[i.*..][0..len]) catch {
        i.* += 1;
        return 0xFFFD;
    };
    i.* += len;
    return cp;
}
