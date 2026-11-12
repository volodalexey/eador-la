//! String tables of the form "Quantity: N" + "/i\nName: text;" records:
//! phrases.var (LoadPhrases), prefix.var, suffix.var, names.var,
//! unit_class.var, unit_type.var, item_type.var (Loader, Prototype.cpp).
const std = @import("std");
const Reader = @import("../formats/varstream.zig").Reader;

/// Whether the file holds Quantity or Quantity+1 records: the original reads
/// `for (i=0; i<=iMax...)` for phrases and type lists, `i<iMax` for
/// prefix/suffix/names.
pub const Count = enum { quantity, quantity_plus_one };

/// Reads the Quantity header; the result is the record count.
pub fn quantity(r: *Reader, count: Count) !usize {
    const q = r.field();
    if (q < 0) return error.BadQuantity;
    return @as(usize, @intCast(q)) + @intFromBool(count == .quantity_plus_one);
}

/// `n` "Key: text;" records.  The slices point into the reader's source.
pub fn names(a: std.mem.Allocator, r: *Reader, n: usize) ![][]const u8 {
    const items = try a.alloc([]const u8, n);
    for (items) |*it| it.* = r.fieldText();
    return items;
}

/// A whole "Quantity: N" table.
pub fn table(a: std.mem.Allocator, src: []const u8, count: Count) ![][]const u8 {
    var r = Reader.init(src);
    return names(a, &r, try quantity(&r, count));
}

/// pFirstPhrase + i; an index out of range gives "".
pub fn get(items: []const []const u8, i: usize) []const u8 {
    return if (i < items.len) items[i] else "";
}

test "phrases.var: Quantity+1 records" {
    var arena: std.heap.ArenaAllocator = .init(std.testing.allocator);
    defer arena.deinit();
    const tab = try table(arena.allocator(), "Quantity: 2\n/0\nName: Пусто;\n\n/1\nName: Берсерк!;\n\n/2\nName: Третья;\n", .quantity_plus_one);
    try std.testing.expectEqual(@as(usize, 3), tab.len);
    try std.testing.expectEqualStrings("Пусто", get(tab, 0));
    try std.testing.expectEqualStrings("Берсерк!", get(tab, 1));
    try std.testing.expectEqualStrings("Третья", get(tab, 2));
    try std.testing.expectEqualStrings("", get(tab, 3));
}

test "prefix.var: Quantity records" {
    var arena: std.heap.ArenaAllocator = .init(std.testing.allocator);
    defer arena.deinit();
    const tab = try table(arena.allocator(), "Quantity: 1\n\n/0\nName: Злой;\n", .quantity);
    try std.testing.expectEqual(@as(usize, 1), tab.len);
    try std.testing.expectEqualStrings("Злой", get(tab, 0));
}
