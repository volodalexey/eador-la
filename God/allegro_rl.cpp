// Allegro 4 compatibility layer on top of raylib (RAYLIB.md, stages 4-5, 10).
// This file implements the subset of Allegro 4 API used by the game.
// raylib is confined to this file and NativeDisplay_rl.cpp.

#include "allegro_rl.h"
#include "dat_loader.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// === Global variables ===

uint8_t key[256] = {0};
int key_shifts = 0;
int mouse_b = 0;
int mouse_z = 0;
BITMAP *mouse_sprite = nullptr;
int _blender_r = 255, _blender_g = 255, _blender_b = 255, _blender_a = 255;
uint32_t _blender_col_32 = 0;  // makecol(r,g,b) from set_trans_blender, tint for draw_lit_sprite
int allegro_404_char = '$';
void *system_driver = nullptr;

// === Stage 4.1: Create/destroy bitmap (stubs) ===

BITMAP *create_bitmap(int w, int h) {
    return create_bitmap_ex(32, w, h);
}

BITMAP *create_bitmap_ex(int depth, int w, int h) {
    BITMAP *bmp = (BITMAP *)malloc(sizeof(BITMAP));
    bmp->w = w;
    bmp->h = h;
    bmp->color_depth = depth;
    int bytes_per_pixel = (depth == 8) ? 1 : 4;
    bmp->data = (uint8_t *)calloc(w * h * bytes_per_pixel, 1);
    bmp->line = (uint8_t **)malloc(h * sizeof(uint8_t *));
    for (int y = 0; y < h; y++) {
        bmp->line[y] = bmp->data + y * w * bytes_per_pixel;
    }
    bmp->is_screen_target = false;
    bmp->rt = nullptr;
    return bmp;
}

void destroy_bitmap(BITMAP *bmp) {
    if (!bmp) return;
    free(bmp->line);
    free(bmp->data);
    free(bmp);
}

int bitmap_color_depth(BITMAP *bmp) {
    return bmp ? bmp->color_depth : 0;
}

// === Stage 4.2: Pixel operations (stubs) ===

// Pixels are uint32_t 0x00RRGGBB, as Allegro 32bpp memory bitmaps.
// 8bpp bitmaps (palette indices) exist only inside the layer: PCX fonts and images.
int getpixel(BITMAP *bmp, int x, int y) {
    if (!bmp || x < 0 || y < 0 || x >= bmp->w || y >= bmp->h) return -1;
    if (bmp->color_depth == 8) return bmp->line[y][x];
    return (int)((uint32_t *)bmp->line[y])[x];
}

void putpixel(BITMAP *bmp, int x, int y, int color) {
    if (!bmp || x < 0 || y < 0 || x >= bmp->w || y >= bmp->h) return;
    if (bmp->color_depth == 8) { bmp->line[y][x] = (uint8_t)color; return; }
    ((uint32_t *)bmp->line[y])[x] = (uint32_t)color;
}

int makecol(int r, int g, int b) {
    // Allegro 32bpp: 0x00RRGGBB (no alpha). Must match getpixel comparisons.
    return (r << 16) | (g << 8) | b;
}

int getr(int color) { return (color >> 16) & 0xFF; }
int getg(int color) { return (color >> 8) & 0xFF; }
int getb(int color) { return color & 0xFF; }

// === Stage 4.3: Drawing rectangles ===

void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
    if (!bmp) return;
    // Swap coords if reversed (Allegro behavior)
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    // Clip to bitmap bounds
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= bmp->w) x2 = bmp->w - 1;
    if (y2 >= bmp->h) y2 = bmp->h - 1;
    if (x1 > x2 || y1 > y2) return;
    
    uint32_t c = color;  // 0x00RRGGBB
    for (int y = y1; y <= y2; y++) {
        uint32_t *row = (uint32_t *)(bmp->line[y]);
        for (int x = x1; x <= x2; x++) {
            row[x] = c;
        }
    }
}

void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
    hline(bmp, x1, y1, x2, color);
    hline(bmp, x1, y2, x2, color);
    vline(bmp, x1, y1, y2, color);
    vline(bmp, x2, y1, y2, color);
}

void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
    // Bresenham's line algorithm (simplified, no clipping optimization)
    if (!bmp) return;
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;
    
    for (;;) {
        putpixel(bmp, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void hline(BITMAP *bmp, int x, int y, int x2, int color) {
    if (!bmp || y < 0 || y >= bmp->h) return;
    if (x > x2) { int t = x; x = x2; x2 = t; }
    if (x < 0) x = 0;
    if (x2 >= bmp->w) x2 = bmp->w - 1;
    if (x > x2) return;
    
    uint32_t *row = (uint32_t *)(bmp->line[y]);
    for (int i = x; i <= x2; i++) {
        row[i] = color;
    }
}

void vline(BITMAP *bmp, int x, int y, int y2, int color) {
    if (!bmp || x < 0 || x >= bmp->w) return;
    if (y > y2) { int t = y; y = y2; y2 = t; }
    if (y < 0) y = 0;
    if (y2 >= bmp->h) y2 = bmp->h - 1;
    if (y > y2) return;
    
    for (int i = y; i <= y2; i++) {
        uint32_t *row = (uint32_t *)(bmp->line[i]);
        row[x] = color;
    }
}

void clear_bitmap(BITMAP *bmp) {
    clear_to_color(bmp, 0);
}

void clear_to_color(BITMAP *bmp, int color) {
    if (!bmp) return;
    uint32_t c = color;
    for (int y = 0; y < bmp->h; y++) {
        uint32_t *row = (uint32_t *)(bmp->line[y]);
        for (int x = 0; x < bmp->w; x++) {
            row[x] = c;
        }
    }
}

// === Stage 4.4: Blit / sprite / stretch ===

void blit(BITMAP *src, BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h) {
    if (!src || !dst) return;
    
    // Clip to source bounds
    if (sx < 0) { w += sx; dx -= sx; sx = 0; }
    if (sy < 0) { h += sy; dy -= sy; sy = 0; }
    if (sx + w > src->w) w = src->w - sx;
    if (sy + h > src->h) h = src->h - sy;
    
    // Clip to destination bounds
    if (dx < 0) { w += dx; sx -= dx; dx = 0; }
    if (dy < 0) { h += dy; sy -= dy; dy = 0; }
    if (dx + w > dst->w) w = dst->w - dx;
    if (dy + h > dst->h) h = dst->h - dy;
    
    if (w <= 0 || h <= 0) return;
    
    // Copy scanlines
    for (int y = 0; y < h; y++) {
        uint32_t *s = (uint32_t *)(src->line[sy + y]) + sx;
        uint32_t *d = (uint32_t *)(dst->line[dy + y]) + dx;
        memcpy(d, s, w * 4);
    }
}

void draw_sprite(BITMAP *dst, BITMAP *src, int x, int y) {
    if (!src || !dst) return;
    
    int sx = 0, sy = 0;
    int w = src->w, h = src->h;
    
    // Clip to destination
    if (x < 0) { sx = -x; w += x; x = 0; }
    if (y < 0) { sy = -y; h += y; y = 0; }
    if (x + w > dst->w) w = dst->w - x;
    if (y + h > dst->h) h = dst->h - y;
    
    if (w <= 0 || h <= 0) return;
    
    uint32_t mask = makecol(255, 0, 255);  // COLOR_0: magenta transparency
    
    for (int j = 0; j < h; j++) {
        uint32_t *s = (uint32_t *)(src->line[sy + j]) + sx;
        uint32_t *d = (uint32_t *)(dst->line[y + j]) + x;
        for (int i = 0; i < w; i++) {
            if (s[i] != mask) {
                d[i] = s[i];
            }
        }
    }
}

void masked_blit(BITMAP *src, BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h) {
    if (!src || !dst) return;
    
    // Clip to source bounds
    if (sx < 0) { w += sx; dx -= sx; sx = 0; }
    if (sy < 0) { h += sy; dy -= sy; sy = 0; }
    if (sx + w > src->w) w = src->w - sx;
    if (sy + h > src->h) h = src->h - sy;
    
    // Clip to destination bounds
    if (dx < 0) { w += dx; sx -= dx; dx = 0; }
    if (dy < 0) { h += dy; sy -= dy; dy = 0; }
    if (dx + w > dst->w) w = dst->w - dx;
    if (dy + h > dst->h) h = dst->h - dy;
    
    if (w <= 0 || h <= 0) return;
    
    uint32_t mask = makecol(255, 0, 255);
    
    for (int y = 0; y < h; y++) {
        uint32_t *s = (uint32_t *)(src->line[sy + y]) + sx;
        uint32_t *d = (uint32_t *)(dst->line[dy + y]) + dx;
        for (int x = 0; x < w; x++) {
            if (s[x] != mask) {
                d[x] = s[x];
            }
        }
    }
}

void stretch_blit(BITMAP *src, BITMAP *dst, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh) {
    if (!src || !dst || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0) return;
    
    // Simple nearest-neighbor scaling
    for (int j = 0; j < dh; j++) {
        int src_y = sy + (j * sh) / dh;
        if (src_y < 0 || src_y >= src->h) continue;
        if (dy + j < 0 || dy + j >= dst->h) continue;
        
        uint32_t *s = (uint32_t *)(src->line[src_y]);
        uint32_t *d = (uint32_t *)(dst->line[dy + j]);
        
        for (int i = 0; i < dw; i++) {
            int src_x = sx + (i * sw) / dw;
            if (src_x < 0 || src_x >= src->w) continue;
            if (dx + i < 0 || dx + i >= dst->w) continue;
            
            d[dx + i] = s[src_x];
        }
    }
}

// Allegro _blender_trans24 formula (used for 32bpp too).
// Match Allegro's unsigned long arithmetic for bit-exact blending.
static inline uint32_t blend_trans(uint32_t x, uint32_t y, int n) {
    // x: source pixel, y: dest pixel, n: alpha (0-255)
    if (n) n++;
    unsigned long xl = x, yl = y;
    unsigned long res = ((xl & 0xFF00FF) - (yl & 0xFF00FF)) * n / 256 + yl;
    yl &= 0xFF00;
    xl &= 0xFF00;
    unsigned long g = (xl - yl) * n / 256 + yl;
    res &= 0xFF00FF;
    g &= 0xFF00;
    return (uint32_t)(res | g);
}

void draw_trans_sprite(BITMAP *dst, BITMAP *src, int x, int y) {
    if (!src || !dst) return;
    
    int sx = 0, sy = 0;
    int w = src->w, h = src->h;
    
    // Clip to destination
    if (x < 0) { sx = -x; w += x; x = 0; }
    if (y < 0) { sy = -y; h += y; y = 0; }
    if (x + w > dst->w) w = dst->w - x;
    if (y + h > dst->h) h = dst->h - y;
    
    if (w <= 0 || h <= 0) return;
    
    uint32_t mask = makecol(255, 0, 255);
    int alpha = _blender_a;
    
    for (int j = 0; j < h; j++) {
        uint32_t *s = (uint32_t *)(src->line[sy + j]) + sx;
        uint32_t *d = (uint32_t *)(dst->line[y + j]) + x;
        for (int i = 0; i < w; i++) {
            if (s[i] != mask) {
                d[i] = blend_trans(s[i], d[i], alpha);
            }
        }
    }
}

void draw_lit_sprite(BITMAP *dst, BITMAP *src, int x, int y, int level) {
    if (!src || !dst) return;
    
    int sx = 0, sy = 0;
    int w = src->w, h = src->h;
    
    // Clip to destination
    if (x < 0) { sx = -x; w += x; x = 0; }
    if (y < 0) { sy = -y; h += y; y = 0; }
    if (x + w > dst->w) w = dst->w - x;
    if (y + h > dst->h) h = dst->h - y;
    
    if (w <= 0 || h <= 0) return;
    
    uint32_t mask = makecol(255, 0, 255);
    uint32_t tint_color = _blender_col_32;  // from set_trans_blender(r,g,b,0)
    
    for (int j = 0; j < h; j++) {
        uint32_t *s = (uint32_t *)(src->line[sy + j]) + sx;
        uint32_t *d = (uint32_t *)(dst->line[y + j]) + x;
        for (int i = 0; i < w; i++) {
            if (s[i] != mask) {
                // DLS_BLEND: blender(tint_color, src_pixel, level)
                d[i] = blend_trans(tint_color, s[i], level);
            }
        }
    }
}

// Rotation about the sprite centre, as Allegro's rotate_sprite / rotate_sprite_v_flip
// (pivot_scaled_sprite_flip with scale 1.0). Angle: fixed 16.16, 256.0 = full circle.
// The source x axis maps to (cos, sin) on screen and the y axis to (-sin, cos);
// v_flip mirrors the sprite vertically before rotating.
static void rotate_sprite_impl(BITMAP *dst, BITMAP *src, int x, int y, int angle, bool v_flip) {
    if (!src || !dst || src->w <= 0 || src->h <= 0) return;

    angle &= 0xFFFFFF;
    if (angle >= 0x800000) angle -= 0x1000000;
    double rad = angle * (M_PI / (double)0x800000);
    // Round to 16.16 like Allegro, so 90/180/270 degrees are exact.
    double c = std::lround(std::cos(rad) * 65536.0) / 65536.0;
    double s = std::lround(std::sin(rad) * 65536.0) / 65536.0;

    double w = src->w, h = src->h;
    double cx = w / 2.0, cy = h / 2.0;
    double px0 = x + cx, py0 = y + cy;  // pivot position on the destination

    // Bounding box of the rotated sprite.
    double ex = std::fabs(cx * c) + std::fabs(cy * s);
    double ey = std::fabs(cx * s) + std::fabs(cy * c);
    int x1 = (int)std::floor(px0 - ex), x2 = (int)std::ceil(px0 + ex);
    int y1 = (int)std::floor(py0 - ey), y2 = (int)std::ceil(py0 + ey);
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > dst->w) x2 = dst->w;
    if (y2 > dst->h) y2 = dst->h;

    uint32_t mask = makecol(255, 0, 255);

    for (int dy = y1; dy < y2; dy++) {
        uint32_t *d = (uint32_t *)(dst->line[dy]);
        double py = dy + 0.5 - py0;
        for (int dx = x1; dx < x2; dx++) {
            double px = dx + 0.5 - px0;
            // Inverse rotation: destination offset -> source offset.
            double u = px * c + py * s + cx;
            double v = -px * s + py * c + cy;
            if (v_flip) v = h - v;
            int su = (int)std::floor(u), sv = (int)std::floor(v);
            if (su < 0 || sv < 0 || su >= src->w || sv >= src->h) continue;
            uint32_t pix = ((uint32_t *)(src->line[sv]))[su];
            if (pix != mask) d[dx] = pix;
        }
    }
}

void rotate_sprite(BITMAP *dst, BITMAP *src, int x, int y, int angle) {
    rotate_sprite_impl(dst, src, x, y, angle, false);
}

void rotate_sprite_v_flip(BITMAP *dst, BITMAP *src, int x, int y, int angle) {
    rotate_sprite_impl(dst, src, x, y, angle, true);
}

// === Stage 5: Fonts and text ===
// Port of Allegro's bitmap font import (fontbmp.c, fonttxt.c) and mono text
// rendering (font.c, glyph.c). Both game fonts load as mono fonts in Allegro.

static FONT_GLYPH *new_glyph(int w, int h) {
    FONT_GLYPH *g = (FONT_GLYPH *)malloc(sizeof(FONT_GLYPH));
    g->w = w;
    g->h = h;
    g->dat = (uint8_t *)calloc(w * h > 0 ? w * h : 1, 1);
    return g;
}

static void free_glyph(FONT_GLYPH *g) {
    if (!g) return;
    free(g->dat);
    free(g);
}

static FONT_GLYPH *clone_glyph(const FONT_GLYPH *g) {
    FONT_GLYPH *c = new_glyph(g->w, g->h);
    memcpy(c->dat, g->dat, g->w * g->h);
    return c;
}

// Glyphs are boxed by a frame of colour 255 (8bpp) or yellow (truecolor).
static void font_find_character(BITMAP *bmp, int *x, int *y, int *w, int *h) {
    int c = (bmp->color_depth == 8) ? 255 : makecol(255, 255, 0);

    // top left corner of the character
    while ((getpixel(bmp, *x, *y) != c) || (getpixel(bmp, *x + 1, *y) != c) ||
           (getpixel(bmp, *x, *y + 1) != c) || (getpixel(bmp, *x + 1, *y + 1) == c)) {
        (*x)++;
        if (*x >= bmp->w) {
            *x = 0;
            (*y)++;
            if (*y >= bmp->h) { *w = 0; *h = 0; return; }
        }
    }
    // right edge
    *w = 0;
    while ((getpixel(bmp, *x + *w + 1, *y) == c) && (getpixel(bmp, *x + *w + 1, *y + 1) != c) &&
           (*x + *w + 1 <= bmp->w))
        (*w)++;
    // bottom edge
    *h = 0;
    while ((getpixel(bmp, *x, *y + *h + 1) == c) && (getpixel(bmp, *x + 1, *y + *h + 1) != c) &&
           (*y + *h + 1 <= bmp->h))
        (*h)++;
}

// Glyphs cut from one image file, starting at character ' ' (grab_font_from_bitmap).
struct GrabbedFont {
    int begin = ' ';
    int count = 0;
    int height = 0;
    FONT_GLYPH **glyphs = nullptr;
};

static void free_grabbed(GrabbedFont &gf) {
    for (int i = 0; i < gf.count; i++) free_glyph(gf.glyphs[i]);
    free(gf.glyphs);
    gf = GrabbedFont();
}

static bool grab_font_from_bitmap(BITMAP *bmp, GrabbedFont &gf) {
    int x = 0, y = 0, w = 0, h = 0;
    int num = 0;
    for (;;) {  // bitmap_font_count
        font_find_character(bmp, &x, &y, &w, &h);
        if (w <= 0 || h <= 0) break;
        num++;
        x += w;
    }
    if (num <= 0) return false;

    // Allegro would make a colour font from a multicoloured image; the game fonts
    // are single-coloured, so every image is imported as mono (non-zero = ink).
    gf.count = num;
    gf.glyphs = (FONT_GLYPH **)calloc(num, sizeof(FONT_GLYPH *));
    int ix = 0, iy = 0;
    w = h = 1;
    for (int i = 0; i < num; i++) {  // import_bitmap_font_mono
        if (w > 0 && h > 0) font_find_character(bmp, &ix, &iy, &w, &h);
        if (w <= 0 || h <= 0) {
            gf.glyphs[i] = new_glyph(8, 8);
            continue;
        }
        FONT_GLYPH *g = new_glyph(w, h);
        for (int j = 0; j < h; j++)
            for (int k = 0; k < w; k++)
                if (getpixel(bmp, ix + k + 1, iy + j + 1)) g->dat[j * w + k] = 1;
        gf.glyphs[i] = g;
        ix += w;
    }
    gf.height = gf.glyphs[0]->h;
    return true;
}

static bool file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

FONT *load_txt_font(const char *filename, void *param1, void *param2) {
    (void)param1; (void)param2;
    FILE *fp = fopen(filename, "r");
    if (!fp) return nullptr;

    FONT *f = (FONT *)calloc(1, sizeof(FONT));
    GrabbedFont src;
    bool have_src = false, ok = true, any = false;
    int glyph_pos = 32;
    char buf[1024];

    while (ok && fgets(buf, sizeof buf, fp)) {
        buf[strcspn(buf, "\r\n")] = 0;
        char *font_str = strtok(buf, " \t");
        if (!font_str) continue;  // tolerate blank lines
        char *start_str = strtok(nullptr, " \t");
        char *end_str = start_str ? strtok(nullptr, " \t") : nullptr;
        if (!start_str) { ok = false; break; }
        if (font_str[0] == '-') font_str[0] = 0;

        int begin = (int)strtol(start_str, nullptr, 0);
        int end = end_str ? (int)strtol(end_str, nullptr, 0) : -1;
        if (begin <= 0 || (end > 0 && end < begin)) { ok = false; break; }

        // Load the image this range comes from (relative to the .txt if needed).
        if (font_str[0]) {
            if (have_src) { free_grabbed(src); have_src = false; }
            char path[1024];
            snprintf(path, sizeof path, "%s", font_str);
            if (!file_exists(path) && font_str[0] != '/') {
                const char *slash = strrchr(filename, '/');
                if (slash) snprintf(path, sizeof path, "%.*s/%s", (int)(slash - filename), filename, font_str);
            }
            BITMAP *img = load_bitmap(path, nullptr);
            if (img) {
                have_src = grab_font_from_bitmap(img, src);
                destroy_bitmap(img);
            }
            if (have_src) glyph_pos = src.begin;
        }
        if (!have_src) { ok = false; break; }

        if (end == -1) end = begin + (src.begin + src.count - 1) - glyph_pos;

        // Transpose glyphs [glyph_pos, glyph_pos + end - begin] to [begin, end].
        for (int i = 0; i <= end - begin; i++) {
            int s = glyph_pos + i - src.begin;
            int d = begin + i;
            if (s < 0 || s >= src.count || d < 0 || d > 255) continue;
            free_glyph(f->glyphs[d]);
            f->glyphs[d] = clone_glyph(src.glyphs[s]);
            any = true;
        }
        if (src.height > f->height) f->height = src.height;
        glyph_pos += (end - begin) + 1;
    }
    fclose(fp);
    if (have_src) free_grabbed(src);

    if (!ok || !any) {
        for (int i = 0; i < 256; i++) free_glyph(f->glyphs[i]);
        free(f);
        return nullptr;
    }
    return f;
}

static FONT_GLYPH *find_glyph(const FONT *f, int ch) {
    if (ch >= 0 && ch < 256 && f->glyphs[ch]) return f->glyphs[ch];
    if (ch != allegro_404_char) return find_glyph(f, allegro_404_char);
    return nullptr;
}

static void draw_glyph(BITMAP *bmp, const FONT_GLYPH *g, int x, int y, int color, int bg) {
    for (int j = 0; j < g->h; j++) {
        int py = y + j;
        if (py < 0 || py >= bmp->h) continue;
        for (int i = 0; i < g->w; i++) {
            if (g->dat[j * g->w + i]) putpixel(bmp, x + i, py, color);
            else if (bg >= 0) putpixel(bmp, x + i, py, bg);
        }
    }
}

// Strings are CP1251 (set_uformat(U_ASCII)): one byte per character.
static void render_text(BITMAP *bmp, const FONT *f, const char *str, int x, int y, int color, int bg) {
    if (!bmp || !f || !str) return;
    for (const unsigned char *p = (const unsigned char *)str; *p; p++) {
        FONT_GLYPH *g = find_glyph(f, *p);
        if (!g) continue;
        draw_glyph(bmp, g, x, y + (f->height - g->h) / 2, color, bg);
        x += g->w;
    }
}

int text_length(FONT *fnt, const char *str) {
    if (!fnt || !str) return 0;
    int w = 0;
    for (const unsigned char *p = (const unsigned char *)str; *p; p++) {
        FONT_GLYPH *g = find_glyph(fnt, *p);
        if (g) w += g->w;
    }
    return w;
}

int text_height(FONT *fnt) {
    return fnt ? fnt->height : 0;
}

void textprintf_ex(BITMAP *bmp, FONT *fnt, int x, int y, int color, int bg, const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    render_text(bmp, fnt, buf, x, y, color, bg);
}

// Old API: background is _textmode, which defaults to 0 (opaque black).
void textprintf(BITMAP *bmp, FONT *fnt, int x, int y, int color, const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    render_text(bmp, fnt, buf, x, y, color, 0);
}

void textprintf_centre_ex(BITMAP *bmp, FONT *fnt, int x, int y, int color, int bg, const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    render_text(bmp, fnt, buf, x - text_length(fnt, buf) / 2, y, color, bg);
}

// === Stage 6/9: Load/save images ===

BITMAP *load_bitmap(const char *filename, RGB *palette) {
    // For game assets, all bitmaps are PCX
    return load_pcx(filename, palette);
}

// load_pcx / save_pcx_pf / save_bmp / PACKFILE: pakfile_rl.cpp (stage 9).

// === Stage 10: Miscellaneous (stubs) ===

// Allegro on Unix prints the message to stderr when no GUI helper is available.
void allegro_message(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

// The window and audio device are opened by set_gfx_mode (NativeDisplay_rl.cpp).
int allegro_init() {
    return 0;
}

// Game code calls eador_allegro_exit through the NativeDisplay.h macro.
void allegro_exit() {
}

void set_trans_blender(int r, int g, int b, int a) {
    _blender_r = r; _blender_g = g; _blender_b = b; _blender_a = a;
    _blender_col_32 = makecol(r, g, b);
}

void set_add_blender(int r, int g, int b, int a) {
    _blender_r = r; _blender_g = g; _blender_b = b; _blender_a = a;
    _blender_col_32 = makecol(r, g, b);
}

void set_alpha_blender() {
    _blender_r = _blender_g = _blender_b = _blender_a = 255;
}

void set_write_alpha_blender() {
    _blender_r = _blender_g = _blender_b = _blender_a = 255;
}

// Forward declarations of functions implemented in NativeDisplay_rl.cpp
extern void rl_pump();
extern bool rl_keypressed();
extern void rl_clear_keybuf();
extern int  rl_ureadkey(int *scancode);
extern void rl_rest(int ms);
extern int  rl_get_desktop_resolution(int *w, int *h);
extern void rl_install_int_c(void (*fn)(), int ms);

// The game draws mouse_sprite into screen1 itself (G_Object.cpp), the OS
// cursor is hidden in rl_backend.cpp. show_mouse is only in commented-out code.
void show_mouse(BITMAP *bmp) { (void)bmp; }

void scare_mouse() {}
void unscare_mouse() {}
void remove_mouse() {}

void set_mouse_sprite(BITMAP *sprite) {
    mouse_sprite = sprite;
}

void set_keyboard_rate(int delay, int repeat) {}
void set_uformat(int fmt) {}
int keypressed() { return rl_keypressed() ? 1 : 0; }
void clear_keybuf() { rl_clear_keybuf(); }
int poll_keyboard() { rl_pump(); return 0; }
int poll_mouse() { rl_pump(); return 0; }

// The game reads eador.cfg itself (LoadConfig), Allegro's config API is unused.
const char *get_config_string(const char *section, const char *key, const char *def) {
    (void)section; (void)key;
    return def;
}

int get_config_int(const char *section, const char *key, int def) {
    (void)section; (void)key;
    return def;
}

void rest(int ms) {
    rl_rest(ms);
}

int get_desktop_resolution(int *w, int *h) {
    return rl_get_desktop_resolution(w, h);
}

void set_display_switch_mode(int mode) {}
void set_window_title(const char *title) {}  // handled via NativeDisplay.h macro → eador_set_window_title
void vsync() {}

void set_palette(const PALETTE p) {}

int install_timer() { return 0; }
int install_mouse() { return 0; }
int install_keyboard() { return 0; }
int install_sound(int digi, int midi, char *cfg) { return 0; }
void install_int(void (*fn)(), int ms) { rl_install_int_c(fn, ms); }
void remove_timer() {}
void remove_keyboard() {}

void set_color_depth(int depth) {}

int ureadkey(int *scancode) {
    return rl_ureadkey(scancode);
}

void delete_file(const char *filename) {
    remove(filename);
}

// file.c replace_extension
char *replace_extension(char *dest, const char *src, const char *ext, int size) {
    if (!dest || !src || !ext || size <= 0) return dest;
    int end = (int)strlen(src), pos = end;
    while (pos > 0) {
        char c = src[pos - 1];
        if (c == '.' || c == '/') break;
        pos--;
    }
    if (pos > 0 && src[pos - 1] == '.') end = pos - 1;
    char tmp[1024];
    snprintf(tmp, sizeof tmp, "%.*s.%s", end, src, ext);
    snprintf(dest, size, "%s", tmp);
    return dest;
}

// unix/ufile.c al_findfirst / al_findnext / al_findclose
struct FfData {
    DIR *dir = nullptr;
    std::string dirname;   // with trailing '/'
    std::string pattern;
    int attrib = 0;
};

// ff_match: '*' matches any run, '?' any one character, case-sensitive.
static bool ff_match(const char *s, const char *p) {
    if (!*p) return !*s;
    if (*p == '*') {
        while (*p == '*') p++;
        if (!*p) return true;
        for (; *s; s++)
            if (ff_match(s, p)) return true;
        return ff_match(s, p);
    }
    if (!*s) return false;
    if (*p == '?' || *p == *s) return ff_match(s + 1, p + 1);
    return false;
}

static int ff_get_attrib(const char *name, const struct stat *st) {
    int attrib = 0;
    if (access(name, W_OK) != 0) attrib |= FA_RDONLY;
    if (S_ISDIR(st->st_mode)) attrib |= FA_DIREC;
    const char *base = strrchr(name, '/');
    base = base ? base + 1 : name;
    if (base[0] == '.' && (base[1] != '.' || base[2] != '\0')) attrib |= FA_HIDDEN;
    return attrib;
}

static void ff_fill(struct al_ffblk *info, const char *name, const struct stat *st, int attrib) {
    info->attrib = attrib;
    info->time = (long)st->st_mtime;
    info->size = (long)st->st_size;
    snprintf(info->name, sizeof info->name, "%s", name);
}

int al_findfirst(const char *pattern, struct al_ffblk *info, int attrib) {
    if (!pattern || !info) return -1;
    info->ff_data = nullptr;
    struct stat st;

    if (!strpbrk(pattern, "?*")) {
        if (stat(pattern, &st) != 0) return -1;
        int actual = ff_get_attrib(pattern, &st);
        if (actual & ~attrib) return -1;
        const char *base = strrchr(pattern, '/');
        ff_fill(info, base ? base + 1 : pattern, &st, actual);
        return 0;
    }

    FfData *ff = new FfData;
    ff->attrib = attrib;
    const char *slash = strrchr(pattern, '/');
    if (slash) {
        ff->dirname.assign(pattern, slash - pattern + 1);
        ff->pattern = slash + 1;
    } else {
        ff->dirname = "./";
        ff->pattern = pattern;
    }
    if (ff->pattern == "*.*") ff->pattern = "*";
    ff->dir = opendir(ff->dirname.c_str());
    if (!ff->dir) { delete ff; return -1; }
    info->ff_data = ff;
    if (al_findnext(info) != 0) { al_findclose(info); return -1; }
    return 0;
}

int al_findnext(struct al_ffblk *info) {
    FfData *ff = info ? (FfData *)info->ff_data : nullptr;
    if (!ff || !ff->dir) return -1;
    for (;;) {
        struct dirent *e = readdir(ff->dir);
        if (!e) return -1;
        if (!ff_match(e->d_name, ff->pattern.c_str())) continue;
        std::string path = ff->dirname + e->d_name;
        struct stat st;
        if (stat(path.c_str(), &st) != 0) continue;
        int attrib = ff_get_attrib(path.c_str(), &st);
        if (attrib & ~ff->attrib) continue;
        ff_fill(info, e->d_name, &st, attrib);
        return 0;
    }
}

void al_findclose(struct al_ffblk *info) {
    FfData *ff = info ? (FfData *)info->ff_data : nullptr;
    if (!ff) return;
    if (ff->dir) closedir(ff->dir);
    delete ff;
    info->ff_data = nullptr;
}

// Samples are only referenced in commented-out code; all sounds go through alogg.
SAMPLE *load_sample(const char *filename) {
    (void)filename;
    return nullptr;
}

void play_sample(const SAMPLE *spl, int vol, int pan, int freq, int loop) {
    (void)spl; (void)vol; (void)pan; (void)freq; (void)loop;
}

int _install_allegro_version_check(int system_id, int *errno_ptr, int compat_version) {
    // Internal Allegro init, not needed for raylib
    return 0;
}
