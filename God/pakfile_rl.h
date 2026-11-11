#pragma once
// PACKFILE and LZSS (RAYLIB.md, stage 9): port of allegro4/src/file.c and lzss.c.

#include <cstddef>
#include <cstdint>
#include <vector>

// Whole-buffer LZSS, byte-compatible with Allegro's lzss_read / lzss_write.
std::vector<uint8_t> rl_lzss_unpack(const uint8_t *src, size_t len, size_t size_hint);
std::vector<uint8_t> rl_lzss_pack(const uint8_t *src, size_t len);
