//! Allegro 4 packfile LZSS decoder (4096-byte ring, 18-byte max match, threshold 2).
//! Ring starts zero-filled with write position N-F, same as Allegro's lzss_read.
const std = @import("std");

const N = 4096;
const F = 18;
const THRESHOLD = 2;

/// Decode `src`. `limit` stops after that many output bytes (per-object streams).
///
/// interface.dat is one 48 MB stream that must be unpacked before the loading
/// panel can be drawn, so the inner loop writes into a preallocated slice and
/// checks bounds once per token instead of once per byte.
pub fn decode(gpa: std.mem.Allocator, src: []const u8, size_hint: usize, limit: ?usize) ![]u8 {
    @setRuntimeSafety(false);
    var ring: [N]u8 = @splat(0);
    var r: usize = N - F;
    const max = limit orelse std.math.maxInt(usize);

    var out = try gpa.alloc(u8, @max(@min(size_hint, max), F));
    errdefer gpa.free(out);
    var n: usize = 0;

    var i: usize = 0;
    var flags: u32 = 0;
    while (i < src.len and n < max) {
        flags >>= 1;
        if (flags & 0x100 == 0) {
            flags = @as(u32, src[i]) | 0xFF00;
            i += 1;
            if (i >= src.len) break;
        }
        // A token yields at most F bytes; grow before writing it.
        if (n + F > out.len) {
            const grown = @max(out.len * 2, n + F);
            out = try gpa.realloc(out, if (limit != null) @min(grown, @max(max, n + F)) else grown);
        }
        if (flags & 1 != 0) {
            const c = src[i];
            i += 1;
            out[n] = c;
            n += 1;
            ring[r] = c;
            r = (r + 1) & (N - 1);
        } else {
            if (i + 1 >= src.len) break;
            const pos = @as(usize, src[i]) | (@as(usize, src[i + 1] & 0xF0) << 4);
            const len = @as(usize, src[i + 1] & 0x0F) + THRESHOLD + 1;
            i += 2;
            for (0..len) |k| {
                const c = ring[(pos + k) & (N - 1)];
                out[n + k] = c;
                ring[r] = c;
                r = (r + 1) & (N - 1);
            }
            n += len;
        }
    }
    if (n > max) n = max;
    return gpa.realloc(out, n);
}
