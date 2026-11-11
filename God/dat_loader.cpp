// Allegro 4 .dat datafile loader (RAYLIB.md, stage 6).
// Port of allegro4/src/datafile.c (load_file_object, read_bitmap), file.c
// (packed file and chunk headers) and lzss.c (decompression).
//
// Format:
//   file   := "slh!" lzss(stream) | "slh." stream      (no password)
//   stream := "ALL." object_list
//   object_list := count(mlong) { "prop" type(mlong) size(mlong) bytes | type(mlong) chunk }
//   chunk  := filesize(mlong) datasize(mlong) payload[filesize]
//             datasize < 0: payload is LZSS, unpacking to -datasize bytes
//   "FILE" payload := object_list          (nested datafile, no magic)
//   "BMP " payload := bits(mshort) w(mshort) h(mshort) pixels
//   anything else  := raw bytes (Ogg data, etc.)

#include "dat_loader.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// Allegro's default_palette (allegro4/src/allegro.c). Without a palette in the
// datafile, Allegro expands 8bpp objects through _current_palette, which
// set_gfx_mode sets to default_palette. get_palette() returns it too.
const uint8_t rl_default_palette[256][3] = {
{ 0, 0, 0}, { 0, 0,42}, { 0,42, 0}, { 0,42,42}, {42, 0, 0}, {42, 0,42}, {42,21, 0}, {42,42,42},
{21,21,21}, {21,21,63}, {21,63,21}, {21,63,63}, {63,21,21}, {63,21,63}, {63,63,21}, {63,63,63},
{ 0, 0, 0}, { 5, 5, 5}, { 8, 8, 8}, {11,11,11}, {14,14,14}, {17,17,17}, {20,20,20}, {24,24,24},
{28,28,28}, {32,32,32}, {36,36,36}, {40,40,40}, {45,45,45}, {50,50,50}, {56,56,56}, {63,63,63},
{ 0, 0,63}, {16, 0,63}, {31, 0,63}, {47, 0,63}, {63, 0,63}, {63, 0,47}, {63, 0,31}, {63, 0,16},
{63, 0, 0}, {63,16, 0}, {63,31, 0}, {63,47, 0}, {63,63, 0}, {47,63, 0}, {31,63, 0}, {16,63, 0},
{ 0,63, 0}, { 0,63,16}, { 0,63,31}, { 0,63,47}, { 0,63,63}, { 0,47,63}, { 0,31,63}, { 0,16,63},
{31,31,63}, {39,31,63}, {47,31,63}, {55,31,63}, {63,31,63}, {63,31,55}, {63,31,47}, {63,31,39},
{63,31,31}, {63,39,31}, {63,47,31}, {63,55,31}, {63,63,31}, {55,63,31}, {47,63,31}, {39,63,31},
{31,63,31}, {31,63,39}, {31,63,47}, {31,63,55}, {31,63,63}, {31,55,63}, {31,47,63}, {31,39,63},
{45,45,63}, {49,45,63}, {54,45,63}, {58,45,63}, {63,45,63}, {63,45,58}, {63,45,54}, {63,45,49},
{63,45,45}, {63,49,45}, {63,54,45}, {63,58,45}, {63,63,45}, {58,63,45}, {54,63,45}, {49,63,45},
{45,63,45}, {45,63,49}, {45,63,54}, {45,63,58}, {45,63,63}, {45,58,63}, {45,54,63}, {45,49,63},
{ 0, 0,28}, { 7, 0,28}, {14, 0,28}, {21, 0,28}, {28, 0,28}, {28, 0,21}, {28, 0,14}, {28, 0, 7},
{28, 0, 0}, {28, 7, 0}, {28,14, 0}, {28,21, 0}, {28,28, 0}, {21,28, 0}, {14,28, 0}, { 7,28, 0},
{ 0,28, 0}, { 0,28, 7}, { 0,28,14}, { 0,28,21}, { 0,28,28}, { 0,21,28}, { 0,14,28}, { 0, 7,28},
{14,14,28}, {17,14,28}, {21,14,28}, {24,14,28}, {28,14,28}, {28,14,24}, {28,14,21}, {28,14,17},
{28,14,14}, {28,17,14}, {28,21,14}, {28,24,14}, {28,28,14}, {24,28,14}, {21,28,14}, {17,28,14},
{14,28,14}, {14,28,17}, {14,28,21}, {14,28,24}, {14,28,28}, {14,24,28}, {14,21,28}, {14,17,28},
{20,20,28}, {22,20,28}, {24,20,28}, {26,20,28}, {28,20,28}, {28,20,26}, {28,20,24}, {28,20,22},
{28,20,20}, {28,22,20}, {28,24,20}, {28,26,20}, {28,28,20}, {26,28,20}, {24,28,20}, {22,28,20},
{20,28,20}, {20,28,22}, {20,28,24}, {20,28,26}, {20,28,28}, {20,26,28}, {20,24,28}, {20,22,28},
{ 0, 0,16}, { 4, 0,16}, { 8, 0,16}, {12, 0,16}, {16, 0,16}, {16, 0,12}, {16, 0, 8}, {16, 0, 4},
{16, 0, 0}, {16, 4, 0}, {16, 8, 0}, {16,12, 0}, {16,16, 0}, {12,16, 0}, { 8,16, 0}, { 4,16, 0},
{ 0,16, 0}, { 0,16, 4}, { 0,16, 8}, { 0,16,12}, { 0,16,16}, { 0,12,16}, { 0, 8,16}, { 0, 4,16},
{ 8, 8,16}, {10, 8,16}, {12, 8,16}, {14, 8,16}, {16, 8,16}, {16, 8,14}, {16, 8,12}, {16, 8,10},
{16, 8, 8}, {16,10, 8}, {16,12, 8}, {16,14, 8}, {16,16, 8}, {14,16, 8}, {12,16, 8}, {10,16, 8},
{ 8,16, 8}, { 8,16,10}, { 8,16,12}, { 8,16,14}, { 8,16,16}, { 8,14,16}, { 8,12,16}, { 8,10,16},
{11,11,16}, {12,11,16}, {13,11,16}, {15,11,16}, {16,11,16}, {16,11,15}, {16,11,13}, {16,11,12},
{16,11,11}, {16,12,11}, {16,13,11}, {16,15,11}, {16,16,11}, {15,16,11}, {13,16,11}, {12,16,11},
{11,16,11}, {11,16,12}, {11,16,13}, {11,16,15}, {11,16,16}, {11,15,16}, {11,13,16}, {11,12,16},
{ 0, 0, 0}, { 0, 0, 0}, { 0, 0, 0}, { 0, 0, 0}, { 0, 0, 0}, { 0, 0, 0}, { 0, 0, 0}, {63,63,63},
};

namespace {

constexpr uint32_t id4(char a, char b, char c, char d) {
    return ((uint32_t)(uint8_t)a << 24) | ((uint32_t)(uint8_t)b << 16) | ((uint32_t)(uint8_t)c << 8) | (uint8_t)d;
}

constexpr uint32_t F_PACK_MAGIC = 0x736C6821u;   // "slh!"
constexpr uint32_t F_NOPACK_MAGIC = 0x736C682Eu; // "slh."
constexpr uint32_t DAT_MAGIC = id4('A', 'L', 'L', '.');
constexpr uint32_t DAT_FILE = id4('F', 'I', 'L', 'E');
constexpr uint32_t DAT_BITMAP = id4('B', 'M', 'P', ' ');
constexpr uint32_t DAT_PROPERTY = id4('p', 'r', 'o', 'p');

// allegro4/src/graphics.c
const int rgb_scale_5[32] = {
    0,   8,   16,  24,  33,  41,  49,  57,  66,  74,  82,  90,  99,  107, 115, 123,
    132, 140, 148, 156, 165, 173, 181, 189, 198, 206, 214, 222, 231, 239, 247, 255};
const int rgb_scale_6[64] = {
    0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,
    65,  69,  73,  77,  81,  85,  89,  93,  97,  101, 105, 109, 113, 117, 121, 125,
    130, 134, 138, 142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190,
    195, 199, 203, 207, 211, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255};

// LZSS decompression of a whole buffer (allegro4/src/lzss.c, lzss_read).
std::vector<uint8_t> lzss_unpack(const uint8_t *src, size_t len, size_t size_hint) {
    constexpr int N = 4096, F = 18, THRESHOLD = 2;
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

// Bounded big-endian reader over a byte buffer.
struct Reader {
    const uint8_t *p;
    size_t len;
    size_t pos = 0;
    bool fail = false;

    bool has(size_t n) {
        if (pos + n > len) { fail = true; return false; }
        return true;
    }
    int getc() { return has(1) ? p[pos++] : 0; }
    int mgetw() { if (!has(2)) return 0; int v = (p[pos] << 8) | p[pos + 1]; pos += 2; return v; }
    int igetw() { if (!has(2)) return 0; int v = p[pos] | (p[pos + 1] << 8); pos += 2; return v; }
    uint32_t mgetl() {
        if (!has(4)) return 0;
        uint32_t v = ((uint32_t)p[pos] << 24) | ((uint32_t)p[pos + 1] << 16) | ((uint32_t)p[pos + 2] << 8) | p[pos + 3];
        pos += 4;
        return v;
    }
};

inline uint32_t rgb32(int r, int g, int b) {
    return ((uint32_t)(r & 0xFF) << 16) | ((uint32_t)(g & 0xFF) << 8) | (uint32_t)(b & 0xFF);
}

// datafile.c read_bitmap(), converted to the game's 32bpp layout.
BITMAP *read_bitmap(Reader &rd) {
    int bits = (int16_t)rd.mgetw();
    bool rgba = bits < 0;
    if (rgba) bits = -bits;
    int w = rd.mgetw();
    int h = rd.mgetw();
    if (rd.fail || w <= 0 || h <= 0) return nullptr;

    size_t bpp = bits == 8 ? 1 : bits == 15 || bits == 16 ? 2 : bits == 24 ? (rgba ? 4 : 3) : bits == 32 ? (rgba ? 4 : 3) : 0;
    if (!bpp || !rd.has((size_t)w * h * bpp)) return nullptr;

    if (bits == 8) {
        // blit_from_256 via _palette_expansion_table (no COLORCONV_KEEP_TRANS by default).
        BITMAP *bmp = create_bitmap(w, h);
        for (int y = 0; y < h; y++) {
            uint32_t *row = (uint32_t *)bmp->line[y];
            for (int x = 0; x < w; x++) {
                int idx = rd.getc();
                const uint8_t *pal = rl_default_palette[idx];
                int r = rgb_scale_6[pal[0]];
                int g = rgb_scale_6[pal[1]];
                int b = rgb_scale_6[pal[2]];
                row[x] = rgb32(r, g, b);
            }
        }
        return bmp;
    }

    BITMAP *bmp = create_bitmap(w, h);
    for (int y = 0; y < h; y++) {
        uint32_t *row = (uint32_t *)bmp->line[y];
        for (int x = 0; x < w; x++) {
            int r, g, b;
            if (bits == 15 || bits == 16) {
                int c = rd.igetw();  // both stored as 5.6.5
                r = rgb_scale_5[(c >> 11) & 0x1F];
                g = rgb_scale_6[(c >> 5) & 0x3F];
                b = rgb_scale_5[c & 0x1F];
            } else {
                r = rd.getc();
                g = rd.getc();
                b = rd.getc();
                if (rgba) rd.getc();  // alpha is dropped when converting to the 32bpp screen format
            }
            row[x] = rgb32(r, g, b);
        }
    }
    return bmp;
}

void free_object(DATAFILE &obj) {
    if (!obj.dat) return;
    if ((uint32_t)obj.type == DAT_BITMAP) destroy_bitmap((BITMAP *)obj.dat);
    else if ((uint32_t)obj.type == DAT_FILE) unload_datafile((DATAFILE *)obj.dat);
    else free(obj.dat);
    obj.dat = nullptr;
}

DATAFILE *load_file_object(Reader &rd);

// One object chunk (file.c pack_fopen_chunk + datafile.c load_object).
bool load_object(DATAFILE &obj, Reader &rd, uint32_t type) {
    int32_t filesize = (int32_t)rd.mgetl();
    int32_t datasize = (int32_t)rd.mgetl();
    if (rd.fail || filesize < 0 || !rd.has((size_t)filesize)) return false;

    const uint8_t *payload = rd.p + rd.pos;
    rd.pos += filesize;

    std::vector<uint8_t> unpacked;
    if (datasize < 0) {
        unpacked = lzss_unpack(payload, (size_t)filesize, (size_t)-datasize);
        if (unpacked.size() < (size_t)-datasize) return false;
        unpacked.resize((size_t)-datasize);
        payload = unpacked.data();
        datasize = -datasize;
    } else if (datasize > filesize) {
        return false;
    }

    Reader sub{payload, (size_t)datasize};
    if (type == DAT_FILE) {
        obj.dat = load_file_object(sub);
    } else if (type == DAT_BITMAP) {
        obj.dat = read_bitmap(sub);
    } else {
        // Binary data: keep the bytes as they are (Ogg streams for Audio.cpp).
        void *buf = malloc(datasize > 0 ? (size_t)datasize : 1);
        if (buf && datasize > 0) memcpy(buf, payload, (size_t)datasize);
        obj.dat = buf;
    }
    if (!obj.dat) return false;
    obj.type = (int)type;
    obj.size = datasize;
    obj.prop = nullptr;
    return true;
}

DATAFILE *load_file_object(Reader &rd) {
    int32_t count = (int32_t)rd.mgetl();
    if (rd.fail || count < 0 || (size_t)count > rd.len) return nullptr;

    DATAFILE *dat = (DATAFILE *)calloc((size_t)count + 1, sizeof(DATAFILE));
    int c = 0;
    bool failed = false;
    while (c < count) {
        uint32_t type = rd.mgetl();
        if (rd.fail) { failed = true; break; }
        if (type == DAT_PROPERTY) {
            // Properties (object names etc.) are not used by the game: skip them.
            rd.mgetl();
            int32_t size = (int32_t)rd.mgetl();
            if (size < 0 || !rd.has((size_t)size)) { failed = true; break; }
            rd.pos += size;
            continue;
        }
        if (!load_object(dat[c], rd, type)) { failed = true; break; }
        c++;
    }
    dat[c].type = DAT_END;
    dat[c].dat = nullptr;
    if (failed) {
        unload_datafile(dat);
        return nullptr;
    }
    return dat;
}

}  // namespace

DATAFILE *load_datafile(const char *filename) {
    if (!filename) return nullptr;
    FILE *fp = fopen(filename, "rb");
    if (!fp) return nullptr;
    std::vector<uint8_t> raw;
    uint8_t chunk[65536];
    size_t n;
    while ((n = fread(chunk, 1, sizeof chunk, fp)) > 0) raw.insert(raw.end(), chunk, chunk + n);
    fclose(fp);
    if (raw.size() < 4) return nullptr;

    uint32_t magic = ((uint32_t)raw[0] << 24) | ((uint32_t)raw[1] << 16) | ((uint32_t)raw[2] << 8) | raw[3];
    std::vector<uint8_t> stream;
    if (magic == F_PACK_MAGIC) stream = lzss_unpack(raw.data() + 4, raw.size() - 4, 0);
    else if (magic == F_NOPACK_MAGIC) stream.assign(raw.begin() + 4, raw.end());
    else return nullptr;  // password-protected or not a datafile

    Reader rd{stream.data(), stream.size()};
    if (rd.mgetl() != DAT_MAGIC) return nullptr;
    return load_file_object(rd);
}

void unload_datafile(DATAFILE *dat) {
    if (!dat) return;
    for (int i = 0; dat[i].type != DAT_END; i++) free_object(dat[i]);
    free(dat);
}
