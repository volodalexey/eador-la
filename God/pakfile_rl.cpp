// PACKFILE, LZSS and PCX/BMP images (RAYLIB.md, stage 9).
// Port of allegro4/src/file.c (packed files), lzss.c, pcx.c and bmp.c (save_bmp).
//
// Packed file ("rp"/"wp"):  "slh!" + LZSS stream, or "slh." + raw bytes.
// Plain file ("r"/"w"):      raw bytes.
// Files are read whole on open and written whole on close; the game only reads
// and writes sequentially, so this matches Allegro's buffered behaviour.

#include "allegro_rl.h"
#include "dat_loader.h"
#include "pakfile_rl.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

constexpr int N = 4096;       // ring buffer size
constexpr int F = 18;         // upper limit for match length
constexpr int THRESHOLD = 2;  // encode as pair if match is longer than this

constexpr uint32_t F_PACK_MAGIC = 0x736C6821u;    // "slh!"
constexpr uint32_t F_NOPACK_MAGIC = 0x736C682Eu;  // "slh."

// lzss.c, compression state and binary search trees.
struct LzssPacker {
    int match_position = 0;
    int match_length = 0;
    int lson[N + 1];
    int rson[N + 257];
    int dad[N + 1];
    uint8_t text_buf[N + F - 1];

    void inittree() {
        for (int i = N + 1; i <= N + 256; i++) rson[i] = N;
        for (int i = 0; i < N; i++) dad[i] = N;
    }

    void insertnode(int r) {
        int i, p, cmp = 1;
        uint8_t *key = &text_buf[r];
        p = N + 1 + key[0];
        rson[r] = lson[r] = N;
        match_length = 0;

        for (;;) {
            if (cmp >= 0) {
                if (rson[p] != N) p = rson[p];
                else { rson[p] = r; dad[r] = p; return; }
            } else {
                if (lson[p] != N) p = lson[p];
                else { lson[p] = r; dad[r] = p; return; }
            }
            for (i = 1; i < F; i++)
                if ((cmp = key[i] - text_buf[p + i]) != 0) break;
            if (i > match_length) {
                match_position = p;
                if ((match_length = i) >= F) break;
            }
        }

        dad[r] = dad[p];
        lson[r] = lson[p];
        rson[r] = rson[p];
        dad[lson[p]] = r;
        dad[rson[p]] = r;
        if (rson[dad[p]] == p) rson[dad[p]] = r;
        else lson[dad[p]] = r;
        dad[p] = N;
    }

    void deletenode(int p) {
        int q;
        if (dad[p] == N) return;
        if (rson[p] == N) q = lson[p];
        else if (lson[p] == N) q = rson[p];
        else {
            q = lson[p];
            if (rson[q] != N) {
                do { q = rson[q]; } while (rson[q] != N);
                rson[dad[q]] = lson[q];
                dad[lson[q]] = dad[q];
                lson[q] = lson[p];
                dad[lson[p]] = q;
            }
            rson[q] = rson[p];
            dad[rson[p]] = q;
        }
        dad[q] = dad[p];
        if (rson[dad[p]] == p) rson[dad[p]] = q;
        else lson[dad[p]] = q;
        dad[p] = N;
    }
};

}  // namespace

// lzss_write() with last = TRUE, over the whole buffer.
std::vector<uint8_t> rl_lzss_pack(const uint8_t *buf, size_t size) {
    std::vector<uint8_t> out;
    out.reserve(size / 2 + 64);
    LzssPacker *d = new LzssPacker;
    memset(d->text_buf, 0, sizeof d->text_buf);

    uint8_t code_buf[17];
    code_buf[0] = 0;
    int code_buf_ptr = 1;
    uint8_t mask = 1;
    int s = 0, r = N - F, len, i;
    size_t pos = 0;
    d->inittree();

    for (len = 0; len < F && pos < size; len++) d->text_buf[r + len] = buf[pos++];

    if (len > 0) {
        for (i = 1; i <= F; i++) d->insertnode(r - i);
        d->insertnode(r);

        do {
            if (d->match_length > len) d->match_length = len;
            if (d->match_length <= THRESHOLD) {
                d->match_length = 1;
                code_buf[0] |= mask;
                code_buf[code_buf_ptr++] = d->text_buf[r];
            } else {
                code_buf[code_buf_ptr++] = (uint8_t)d->match_position;
                code_buf[code_buf_ptr++] = (uint8_t)(((d->match_position >> 4) & 0xF0) |
                                                     (d->match_length - (THRESHOLD + 1)));
            }
            mask = (uint8_t)(mask << 1);
            if (mask == 0) {
                out.insert(out.end(), code_buf, code_buf + code_buf_ptr);
                code_buf[0] = 0;
                code_buf_ptr = mask = 1;
            }

            int last_match_length = d->match_length;
            for (i = 0; i < last_match_length && pos < size; i++) {
                uint8_t c = buf[pos++];
                d->deletenode(s);
                d->text_buf[s] = c;
                if (s < F - 1) d->text_buf[s + N] = c;
                s = (s + 1) & (N - 1);
                r = (r + 1) & (N - 1);
                d->insertnode(r);
            }
            while (i++ < last_match_length) {
                d->deletenode(s);
                s = (s + 1) & (N - 1);
                r = (r + 1) & (N - 1);
                if (--len) d->insertnode(r);
            }
        } while (len > 0);

        if (code_buf_ptr > 1) out.insert(out.end(), code_buf, code_buf + code_buf_ptr);
    }
    delete d;
    return out;
}

// lzss_read() until EOF.
std::vector<uint8_t> rl_lzss_unpack(const uint8_t *src, size_t len, size_t size_hint) {
    uint8_t text_buf[N + F - 1];
    memset(text_buf, 0, sizeof text_buf);

    std::vector<uint8_t> out;
    out.reserve(size_hint ? size_hint : len * 3);
    size_t pos = 0;
    int r = N - F;
    unsigned int flags = 0;

    for (;;) {
        if (((flags >>= 1) & 256) == 0) {
            if (pos >= len) break;
            flags = src[pos++] | 0xFF00;  // high byte counts eight
        }
        if (flags & 1) {
            if (pos >= len) break;
            uint8_t c = src[pos++];
            text_buf[r++] = c;
            r &= (N - 1);
            out.push_back(c);
        } else {
            if (pos + 1 >= len) break;
            int i = src[pos++];
            int j = src[pos++];
            i |= ((j & 0xF0) << 4);
            j = (j & 0x0F) + THRESHOLD;
            for (int k = 0; k <= j; k++) {
                uint8_t c = text_buf[(i + k) & (N - 1)];
                text_buf[r++] = c;
                r &= (N - 1);
                out.push_back(c);
            }
        }
    }
    return out;
}

// === PACKFILE ===

struct PACKFILE_tag {
    FILE *fp = nullptr;  // write mode only
    bool write = false;
    bool pack = false;
    std::vector<uint8_t> data;
    size_t pos = 0;
};

static bool read_whole_file(const char *filename, std::vector<uint8_t> &out) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return false;
    uint8_t chunk[65536];
    size_t n;
    while ((n = fread(chunk, 1, sizeof chunk, fp)) > 0) out.insert(out.end(), chunk, chunk + n);
    fclose(fp);
    return true;
}

PACKFILE *pack_fopen(const char *filename, const char *mode) {
    if (!filename || !mode) return nullptr;
    bool write = false, pack = false;
    for (const char *m = mode; *m; m++) {
        if (*m == 'w' || *m == 'W') write = true;
        else if (*m == 'r' || *m == 'R') write = false;
        else if (*m == 'p' || *m == 'P') pack = true;
    }

    PACKFILE *f = new PACKFILE;
    f->write = write;
    f->pack = pack;

    if (write) {
        f->fp = fopen(filename, "wb");
        if (!f->fp) { delete f; return nullptr; }
        return f;
    }

    std::vector<uint8_t> raw;
    if (!read_whole_file(filename, raw)) { delete f; return nullptr; }
    if (!pack) {
        f->data.swap(raw);
        return f;
    }

    // Packed read: the magic number decides between LZSS and raw data.
    if (raw.size() < 4) { delete f; return nullptr; }
    uint32_t magic = ((uint32_t)raw[0] << 24) | ((uint32_t)raw[1] << 16) | ((uint32_t)raw[2] << 8) | raw[3];
    if (magic == F_PACK_MAGIC) {
        f->data = rl_lzss_unpack(raw.data() + 4, raw.size() - 4, 0);
    } else if (magic == F_NOPACK_MAGIC) {
        f->data.assign(raw.begin() + 4, raw.end());
    } else {
        delete f;  // Allegro fails with EDOM here
        return nullptr;
    }
    return f;
}

int pack_fclose(PACKFILE *f) {
    if (!f) return 0;
    int ret = 0;
    if (f->write) {
        if (f->pack) {
            const uint8_t magic[4] = {0x73, 0x6C, 0x68, 0x21};  // "slh!"
            std::vector<uint8_t> packed = rl_lzss_pack(f->data.data(), f->data.size());
            if (fwrite(magic, 1, 4, f->fp) != 4) ret = EOF;
            if (!packed.empty() && fwrite(packed.data(), 1, packed.size(), f->fp) != packed.size()) ret = EOF;
        } else if (!f->data.empty() && fwrite(f->data.data(), 1, f->data.size(), f->fp) != f->data.size()) {
            ret = EOF;
        }
        if (fclose(f->fp) != 0) ret = EOF;
    }
    delete f;
    return ret;
}

int pack_feof(PACKFILE *f) {
    return !f || f->write || f->pos >= f->data.size();
}

int pack_getc(PACKFILE *f) {
    if (!f || f->write || f->pos >= f->data.size()) return EOF;
    return f->data[f->pos++];
}

int pack_putc(int c, PACKFILE *f) {
    if (!f || !f->write) return EOF;
    f->data.push_back((uint8_t)c);
    return c & 0xFF;
}

int pack_fread(void *p, int n, PACKFILE *f) {
    if (!f || f->write || !p || n <= 0) return 0;
    size_t avail = f->data.size() - f->pos;
    size_t got = (size_t)n < avail ? (size_t)n : avail;
    memcpy(p, f->data.data() + f->pos, got);
    f->pos += got;
    return (int)got;
}

int pack_fwrite(const void *p, int n, PACKFILE *f) {
    if (!f || !f->write || !p || n <= 0) return 0;
    const uint8_t *b = (const uint8_t *)p;
    f->data.insert(f->data.end(), b, b + n);
    return n;
}

int pack_igetw(PACKFILE *f) {
    int b1, b2;
    if ((b1 = pack_getc(f)) != EOF)
        if ((b2 = pack_getc(f)) != EOF)
            return (b2 << 8) | b1;
    return EOF;
}

int pack_igetl(PACKFILE *f) {
    int b1, b2, b3, b4;
    if ((b1 = pack_getc(f)) != EOF)
        if ((b2 = pack_getc(f)) != EOF)
            if ((b3 = pack_getc(f)) != EOF)
                if ((b4 = pack_getc(f)) != EOF)
                    return (int)(((uint32_t)b4 << 24) | ((uint32_t)b3 << 16) | ((uint32_t)b2 << 8) | (uint32_t)b1);
    return EOF;
}

int pack_iputw(int w, PACKFILE *f) {
    if (pack_putc(w & 0xFF, f) == EOF) return EOF;
    if (pack_putc((w >> 8) & 0xFF, f) == EOF) return EOF;
    return w;
}

int pack_iputl(int l, PACKFILE *f) {
    for (int i = 0; i < 4; i++)
        if (pack_putc((l >> (i * 8)) & 0xFF, f) == EOF) return EOF;
    return l;
}

// === PCX (pcx.c) ===

static inline int scale6(int v) {
    // _rgb_scale_6: 0..63 → 0..255
    static const int t[64] = {
        0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,
        65,  69,  73,  77,  81,  85,  89,  93,  97,  101, 105, 109, 113, 117, 121, 125,
        130, 134, 138, 142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190,
        195, 199, 203, 207, 211, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255};
    return t[v & 63];
}

// 8-plane PCX stays an 8bpp bitmap (fonts need the palette indices);
// 24-bit PCX (map saves) becomes the game's 32bpp 0x00RRGGBB.
BITMAP *load_pcx_pf(void *vf, RGB *pal) {
    PACKFILE *f = (PACKFILE *)vf;
    if (!f) return nullptr;
    PALETTE tmppal;
    if (!pal) pal = tmppal;

    pack_getc(f);  // manufacturer
    pack_getc(f);  // version
    pack_getc(f);  // encoding
    if (pack_getc(f) != 8) return nullptr;  // bits per plane

    int width = -(pack_igetw(f));
    int height = -(pack_igetw(f));
    width += pack_igetw(f) + 1;
    height += pack_igetw(f) + 1;
    pack_igetl(f);  // DPI

    for (int c = 0; c < 16; c++) {
        pal[c].r = pack_getc(f) / 4;
        pal[c].g = pack_getc(f) / 4;
        pal[c].b = pack_getc(f) / 4;
    }
    pack_getc(f);

    int bpp = pack_getc(f) * 8;
    if (bpp != 8 && bpp != 24) return nullptr;
    int bytes_per_line = pack_igetw(f);
    for (int c = 0; c < 60; c++) pack_getc(f);
    if (width <= 0 || height <= 0) return nullptr;

    BITMAP *b = create_bitmap_ex(bpp == 8 ? 8 : 32, width, height);
    if (!b) return nullptr;

    for (int y = 0; y < height; y++) {
        int x = 0, xx = 0;
        int shift = 16;  // R plane first, then G, then B
        uint32_t *row32 = (uint32_t *)b->line[y];
        while (x < bytes_per_line * bpp / 8) {
            if (pack_feof(f)) { destroy_bitmap(b); return nullptr; }
            int ch = pack_getc(f);
            int c = 1;
            if ((ch & 0xC0) == 0xC0) {
                c = ch & 0x3F;
                if (pack_feof(f)) { destroy_bitmap(b); return nullptr; }
                ch = pack_getc(f);
            }
            if (bpp == 8) {
                while (c--) {
                    if (x < b->w) b->line[y][x] = (uint8_t)ch;
                    x++;
                }
            } else {
                while (c--) {
                    if (xx < b->w) row32[xx] |= (uint32_t)(ch & 0xFF) << shift;
                    x++;
                    if (x == bytes_per_line) { xx = 0; shift = 8; }
                    else if (x == bytes_per_line * 2) { xx = 0; shift = 0; }
                    else xx++;
                }
            }
        }
    }

    if (bpp == 8) {  // 256-colour palette after marker 12
        int c;
        while ((c = pack_getc(f)) != EOF) {
            if (c == 12) {
                for (int i = 0; i < 256; i++) {
                    pal[i].r = pack_getc(f) / 4;
                    pal[i].g = pack_getc(f) / 4;
                    pal[i].b = pack_getc(f) / 4;
                }
                break;
            }
        }
    } else {
        // Allegro builds a fake 3-3-2 palette when it wants one; the game ignores it.
        for (int i = 0; i < 256; i++) {
            pal[i].r = ((i >> 5) & 7) * 63 / 7;
            pal[i].g = ((i >> 2) & 7) * 63 / 7;
            pal[i].b = (i & 3) * 63 / 3;
        }
    }
    return b;
}

BITMAP *load_pcx(const char *filename, RGB *palette) {
    PACKFILE *f = pack_fopen(filename, "r");
    if (!f) return nullptr;
    BITMAP *bmp = load_pcx_pf(f, palette);
    pack_fclose(f);
    return bmp;
}

void save_pcx_pf(void *vf, BITMAP *bmp, const RGB *pal) {
    PACKFILE *f = (PACKFILE *)vf;
    if (!f || !bmp) return;
    PALETTE tmppal;
    if (!pal) { get_palette(tmppal); pal = tmppal; }

    int depth = bitmap_color_depth(bmp);
    int planes = depth == 8 ? 1 : 3;

    pack_putc(10, f);  // manufacturer
    pack_putc(5, f);   // version
    pack_putc(1, f);   // RLE
    pack_putc(8, f);   // bits per pixel
    pack_iputw(0, f);
    pack_iputw(0, f);
    pack_iputw(bmp->w - 1, f);
    pack_iputw(bmp->h - 1, f);
    pack_iputw(320, f);
    pack_iputw(200, f);
    for (int c = 0; c < 16; c++) {
        pack_putc(scale6(pal[c].r), f);
        pack_putc(scale6(pal[c].g), f);
        pack_putc(scale6(pal[c].b), f);
    }
    pack_putc(0, f);
    pack_putc(planes, f);
    pack_iputw(bmp->w, f);
    pack_iputw(1, f);
    pack_iputw(bmp->w, f);
    pack_iputw(bmp->h, f);
    for (int c = 0; c < 54; c++) pack_putc(0, f);

    for (int y = 0; y < bmp->h; y++) {
        int runcount = 0;
        int runchar = 0;
        for (int x = 0; x < bmp->w * planes; x++) {
            int ch;
            if (depth == 8) {
                ch = getpixel(bmp, x, y) & 0xFF;
            } else if (x < bmp->w) {
                ch = getr(getpixel(bmp, x, y));
            } else if (x < bmp->w * 2) {
                ch = getg(getpixel(bmp, x - bmp->w, y));
            } else {
                ch = getb(getpixel(bmp, x - bmp->w * 2, y));
            }
            if (runcount == 0) {
                runcount = 1;
                runchar = ch;
            } else if (ch != runchar || runcount >= 0x3F) {
                if (runcount > 1 || (runchar & 0xC0) == 0xC0) pack_putc(0xC0 | runcount, f);
                pack_putc(runchar, f);
                runcount = 1;
                runchar = ch;
            } else {
                runcount++;
            }
        }
        if (runcount > 1 || (runchar & 0xC0) == 0xC0) pack_putc(0xC0 | runcount, f);
        pack_putc(runchar, f);
    }

    if (depth == 8) {
        pack_putc(12, f);
        for (int c = 0; c < 256; c++) {
            pack_putc(scale6(pal[c].r), f);
            pack_putc(scale6(pal[c].g), f);
            pack_putc(scale6(pal[c].b), f);
        }
    }
}

// === BMP (bmp.c save_bmp_pf), used for F1 screenshots ===

int save_bmp(const char *filename, BITMAP *bmp, const RGB *pal) {
    (void)pal;  // 32bpp bitmaps are saved as 24-bit BMP, no palette
    if (!filename || !bmp) return -1;
    PACKFILE *f = pack_fopen(filename, "w");
    if (!f) return -1;

    int filler = 3 - ((bmp->w * 3 - 1) & 3);
    int biSizeImage = (bmp->w * 3 + filler) * bmp->h;
    int bfSize = 54 + biSizeImage;

    pack_iputw(0x4D42, f);  // "BM"
    pack_iputl(bfSize, f);
    pack_iputw(0, f);
    pack_iputw(0, f);
    pack_iputl(54, f);
    pack_iputl(40, f);
    pack_iputl(bmp->w, f);
    pack_iputl(bmp->h, f);
    pack_iputw(1, f);
    pack_iputw(24, f);
    pack_iputl(0, f);
    pack_iputl(biSizeImage, f);
    pack_iputl(0xB12, f);
    pack_iputl(0xB12, f);
    pack_iputl(0, f);
    pack_iputl(0, f);

    for (int i = bmp->h - 1; i >= 0; i--) {
        for (int j = 0; j < bmp->w; j++) {
            int c = getpixel(bmp, j, i);
            pack_putc(getb(c), f);
            pack_putc(getg(c), f);
            pack_putc(getr(c), f);
        }
        for (int j = 0; j < filler; j++) pack_putc(0, f);
    }
    return pack_fclose(f) == 0 ? 0 : -1;
}

int save_bitmap(const char *filename, BITMAP *bmp, const RGB *pal) {
    return save_bmp(filename, bmp, pal);
}

void get_palette(PALETTE p) {
    for (int i = 0; i < 256; i++) {
        p[i].r = rl_default_palette[i][0];
        p[i].g = rl_default_palette[i][1];
        p[i].b = rl_default_palette[i][2];
    }
}
