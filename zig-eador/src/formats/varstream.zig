//! Positional reader for var/*.var and text/*.txt, the same semantics as
//! ReadWord / ReadInt / ReadText / SkipText in Prototype.cpp:669-793.
//! Key names are never compared: the original reads fields strictly in file
//! order, so the order of reads in loaders must follow the C++ code.
//! Works on the imported UTF-8 text (all delimiters are ASCII).
const std = @import("std");

/// Two numbers of a "(a, b; c, d)" list.
pub const Pair = struct { id: i32 = 0, value: i32 = 0 };

pub const Reader = struct {
    src: []const u8,
    pos: usize = 0,
    /// cSymbol: the last character read by `char` or `int` (the character
    /// that ended the number).  Unchanged at the end of the file, as
    /// CurrentFile.get leaves it.
    sym: u8 = 0,

    pub fn init(src: []const u8) Reader {
        return .{ .src = src };
    }

    pub fn eof(self: *const Reader) bool {
        return self.pos >= self.src.len;
    }

    /// CurrentFile.get(ch): one character, null at the end.
    pub fn char(self: *Reader) ?u8 {
        if (self.eof()) return null;
        const c = self.src[self.pos];
        self.pos += 1;
        self.sym = c;
        return c;
    }

    /// ReadWord: text up to ':' or ';' (excluded); the delimiter and the
    /// character after it are consumed.
    pub fn word(self: *Reader) []const u8 {
        const start = self.pos;
        while (self.pos < self.src.len and self.src[self.pos] != ':' and self.src[self.pos] != ';') self.pos += 1;
        const w = self.src[start..self.pos];
        if (!self.eof()) {
            self.pos += 1;
            if (!self.eof()) self.pos += 1;
        }
        return w;
    }

    /// ReadInt: digits and '-' (anywhere, makes the value negative); the
    /// first other character is consumed.
    pub fn int(self: *Reader) i32 {
        var v: i32 = 0;
        var neg = false;
        while (self.char()) |c| {
            switch (c) {
                '-' => neg = true,
                '0'...'9' => v = v *% 10 +% @as(i32, c - '0'),
                else => break,
            }
        }
        return if (neg) -%v else v;
    }

    /// ReadWord into the trash buffer, then ReadInt: "Key: 12".
    pub fn field(self: *Reader) i32 {
        _ = self.word();
        return self.int();
    }

    /// ReadWord twice: "Key: text;" -> "text".
    pub fn fieldText(self: *Reader) []const u8 {
        _ = self.word();
        return self.word();
    }

    /// ReadText: text up to '#' (excluded, consumed).
    pub fn text(self: *Reader) []const u8 {
        const start = self.pos;
        while (self.pos < self.src.len and self.src[self.pos] != '#') self.pos += 1;
        const s = self.src[start..self.pos];
        if (!self.eof()) self.pos += 1;
        return s;
    }

    /// SkipText: everything up to and including '#'.
    pub fn skipText(self: *Reader) void {
        _ = self.text();
    }

    /// SkipText + ReadText: one "#...#" entry of text/*.txt.
    pub fn entry(self: *Reader) []const u8 {
        self.skipText();
        return self.text();
    }

    /// "(a, b, c)" after its key: `CurrentFile.get` + `ReadInt` until the
    /// number ends on ')' (race quests, unit resources, upgrade needs).
    /// Returns the number of values.
    pub fn list(self: *Reader, out: []i32) !usize {
        var n: usize = 0;
        while (true) {
            if (n == out.len) return error.ListTooLong;
            _ = self.char();
            out[n] = self.int();
            n += 1;
            if (self.sym == ')') return n;
            if (self.eof()) return error.UnexpectedEof;
        }
    }

    /// "(a, b; c, d)" after its key: pairs read as get+ReadInt+get+ReadInt
    /// until ')' (unit upgrades and loot).  A pair written without a space
    /// ("82,2") loses its second number: the separator `get` eats the digit
    /// and ReadInt stops on ';' with 0.  This is what the game does with the
    /// GOG unit.var, so it is kept.
    pub fn pairs(self: *Reader, out: []Pair) !usize {
        var n: usize = 0;
        while (true) {
            if (n == out.len) return error.ListTooLong;
            _ = self.char();
            const a = self.int();
            _ = self.char();
            const b = self.int();
            out[n] = .{ .id = a, .value = b };
            n += 1;
            if (self.sym == ')') return n;
            if (self.eof()) return error.UnexpectedEof;
        }
    }
};

const t = std.testing;

test "ReadWord / ReadInt as in Prototype.cpp" {
    var r = Reader.init("Quantity: 558\n/0\nName: Пусто;\n\n/1\nTalent1: 3, -2\n");
    try t.expectEqualStrings("Quantity", r.word());
    try t.expectEqual(@as(i32, 558), r.int());
    try t.expectEqualStrings("Пусто", r.fieldText());
    _ = r.word();
    try t.expectEqual(@as(i32, 3), r.int());
    // CurrentFile.get(cSymbol) skips the space after ','.
    _ = r.char();
    try t.expectEqual(@as(i32, -2), r.int());
    try t.expect(r.eof());
    try t.expectEqualStrings("", r.word());
    try t.expectEqual(@as(i32, 0), r.int());
}

test "lists and pairs stop on ')'" {
    var r = Reader.init("Need: (60, 7)\nLvl 09 loot: (82,2; 94, 5)\nLvl 10 loot: (0, 0)\n");
    _ = r.word();
    var need: [3]i32 = undefined;
    try t.expectEqual(@as(usize, 2), try r.list(&need));
    try t.expectEqualSlices(i32, &.{ 60, 7 }, need[0..2]);
    var p: [5]Pair = undefined;
    _ = r.word();
    try t.expectEqual(@as(usize, 2), try r.pairs(&p));
    // Original behaviour: "82,2" -> (82, 0).
    try t.expectEqual(Pair{ .id = 82, .value = 0 }, p[0]);
    try t.expectEqual(Pair{ .id = 94, .value = 5 }, p[1]);
    _ = r.word();
    try t.expectEqual(@as(usize, 1), try r.pairs(&p));
    try t.expectEqual(Pair{}, p[0]);
    var small: [1]i32 = undefined;
    var r2 = Reader.init("(1, 2)");
    try t.expectError(error.ListTooLong, r2.list(&small));
}

test "ReadText / SkipText" {
    var r = Reader.init("/1 Воин\n#Воины сильны.\nВторая строка#\n\n/2 X\n#Y#");
    try t.expectEqualStrings("Воины сильны.\nВторая строка", r.entry());
    try t.expectEqualStrings("Y", r.entry());
    try t.expectEqualStrings("", r.entry());
}
