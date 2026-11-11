// Allegro 4 compatibility layer header (RAYLIB.md, stages 2-3, 5, 10).
// This header replaces <allegro.h>.
// It does NOT include raylib.h; raylib is confined to allegro_rl.cpp and NativeDisplay_rl.cpp.

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// === Stage 2: Types and structures ===

struct BITMAP {
    int w, h;
    int color_depth;          // always 32 for raylib builds
    // Pixels are uint32_t 0x00RRGGBB, rows top-down (Allegro 32bpp memory bitmap
    // with the X11 TrueColor layout r=16, g=8, b=0).
    uint8_t **line;           // pointers to rows (for ->line[y])
    uint8_t *data;            // contiguous buffer w*h*4
    bool is_screen_target;    // true for canvas and screen
    // Only for offscreen render:
    void *rt;                 // RenderTexture2D*, nullptr if CPU-bitmap
};

// Monochrome glyph: one byte per pixel, non-zero = ink (Allegro FONT_GLYPH equivalent).
typedef struct FONT_GLYPH {
    int w, h;
    uint8_t *dat;
} FONT_GLYPH;

// Mono bitmap font loaded by load_txt_font (both game fonts are mono in Allegro).
struct FONT {
    int height;
    FONT_GLYPH *glyphs[256];  // indexed by CP1251 byte, nullptr if missing
};

typedef struct { uint8_t r, g, b; } RGB;
typedef RGB PALETTE[256];     // used only when save/load PCX map
typedef struct DATAFILE_PROPERTY {
    char *dat;
    int type;
} DATAFILE_PROPERTY;
typedef struct DATAFILE {
    void *dat;                // BITMAP* for "BMP " objects, raw bytes otherwise
    int type;                 // object type id, DAT_END terminates the array
    long size;                // size of the object data
    DATAFILE_PROPERTY *prop;  // not loaded (always NULL)
} DATAFILE;
typedef void AUDIOSTREAM;     // opaque, defined in allegro_rl.cpp
typedef void SAMPLE;
typedef struct { int voice; } AUDIOSTREAM_INFO;

// === Stage 2.4: Constants and macros ===

#define GFX_AUTODETECT_FULLSCREEN  0
#define GFX_AUTODETECT_WINDOWED    1
#define GFX_TEXT                  -1

// Key codes (subset used by the game)
#define KEY_ESC        1
#define KEY_1          2
#define KEY_2          3
#define KEY_3          4
#define KEY_4          5
#define KEY_5          6
#define KEY_6          7
#define KEY_7          8
#define KEY_8          9
#define KEY_9          10
#define KEY_0          11
#define KEY_MINUS      12
#define KEY_EQUALS     13
#define KEY_BACKSPACE  14
#define KEY_TAB        15
#define KEY_Q          16
#define KEY_W          17
#define KEY_E          18
#define KEY_R          19
#define KEY_T          20
#define KEY_Y          21
#define KEY_U          22
#define KEY_I          23
#define KEY_O          24
#define KEY_P          25
#define KEY_OPENBRACE  26
#define KEY_CLOSEBRACE 27
#define KEY_ENTER      28
#define KEY_LCONTROL   29
#define KEY_A          30
#define KEY_S          31
#define KEY_D          32
#define KEY_F          33
#define KEY_G          34
#define KEY_H          35
#define KEY_J          36
#define KEY_K          37
#define KEY_L          38
#define KEY_COLON      39
#define KEY_QUOTE      40
#define KEY_TILDE      41
#define KEY_LSHIFT     42
#define KEY_BACKSLASH  43
#define KEY_Z          44
#define KEY_X          45
#define KEY_C          46
#define KEY_V          47
#define KEY_B          48
#define KEY_N          49
#define KEY_M          50
#define KEY_COMMA      51
#define KEY_STOP       52
#define KEY_SLASH      53
#define KEY_RSHIFT     54
#define KEY_ASTERISK   55
#define KEY_ALT        56
#define KEY_SPACE      57
#define KEY_CAPSLOCK   58
#define KEY_F1         59
#define KEY_F2         60
#define KEY_F3         61
#define KEY_F4         62
#define KEY_F5         63
#define KEY_F6         64
#define KEY_F7         65
#define KEY_F8         66
#define KEY_F9         67
#define KEY_F10        68
#define KEY_NUMLOCK    69
#define KEY_SCRLOCK    70
#define KEY_HOME       71
#define KEY_UP         72
#define KEY_PGUP       73
#define KEY_MINUS_PAD  74
#define KEY_LEFT       75
#define KEY_5_PAD      76
#define KEY_RIGHT      77
#define KEY_PLUS_PAD   78
#define KEY_END        79
#define KEY_DOWN       80
#define KEY_PGDN       81
#define KEY_INSERT     82
#define KEY_DEL        83
#define KEY_F11        87
#define KEY_F12        88
#define KEY_RCONTROL   97
#define KEY_ALTGR      100
#define KEY_ENTER_PAD  108
#define KEY_PAUSE      119

extern uint8_t key[256];      // updated in main loop
extern int     key_shifts;
extern int     mouse_b;
extern int     mouse_z;

// Blender mode (store globally, apply in draw_trans_sprite / draw_lit_sprite)
extern int _blender_r, _blender_g, _blender_b, _blender_a;

// === Stage 4: Graphic primitives ===

// 4.1. Create/destroy bitmap
BITMAP *create_bitmap(int w, int h);
BITMAP *create_bitmap_ex(int depth, int w, int h);
void    destroy_bitmap(BITMAP *bmp);
int     bitmap_color_depth(BITMAP *bmp);

// 4.2. Pixel operations (CPU)
int  getpixel(BITMAP *bmp, int x, int y);
void putpixel(BITMAP *bmp, int x, int y, int color);
int  makecol(int r, int g, int b);
int  getr(int color);
int  getg(int color);
int  getb(int color);

// 4.3. Drawing rectangles
void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void hline(BITMAP *bmp, int x, int y, int x2, int color);
void vline(BITMAP *bmp, int x, int y, int y2, int color);
void clear_bitmap(BITMAP *bmp);
void clear_to_color(BITMAP *bmp, int color);

// 4.4. Blit / sprite / stretch
void blit(BITMAP *src, BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h);
void draw_sprite(BITMAP *dst, BITMAP *src, int x, int y);
void masked_blit(BITMAP *src, BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h);
void stretch_blit(BITMAP *src, BITMAP *dst, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);
void draw_trans_sprite(BITMAP *dst, BITMAP *src, int x, int y);
void draw_lit_sprite(BITMAP *dst, BITMAP *src, int x, int y, int level);
void rotate_sprite(BITMAP *dst, BITMAP *src, int x, int y, int angle);
void rotate_sprite_v_flip(BITMAP *dst, BITMAP *src, int x, int y, int angle);

// 4.5. Fixed-point (for rotate_sprite angles: Allegro fixed = int, 1.0 = 1<<16)
typedef int fixed;
#define itofix(x)   ((x) << 16)
#define fixtoi(x)   ((x) >> 16)
#define fixtorad(x) ((x) * 0.00009587645)
#define radtofix(x) ((x) * 10430.378350)

// 4.6. Load/save images (used by the game for .dat files and PCX saves)
DATAFILE *load_datafile(const char *filename);
void unload_datafile(DATAFILE *dat);
BITMAP *load_bitmap(const char *filename, RGB *palette);
BITMAP *load_pcx(const char *filename, RGB *palette);
int save_bitmap(const char *filename, BITMAP *bmp, const RGB *palette);
int save_bmp(const char *filename, BITMAP *bmp, const RGB *palette);
BITMAP *load_pcx_pf(void *f, RGB *palette);  // f is PACKFILE*
void save_pcx_pf(void *f, BITMAP *bmp, const RGB *palette);
#define DAT_END  -1

// 4.7. PACKFILE (LZSS-compressed file format for saves)
typedef struct PACKFILE_tag PACKFILE;
PACKFILE *pack_fopen(const char *filename, const char *mode);
int pack_fclose(PACKFILE *f);
int pack_fread(void *p, int n, PACKFILE *f);
int pack_fwrite(const void *p, int n, PACKFILE *f);
int pack_getc(PACKFILE *f);
int pack_putc(int c, PACKFILE *f);
int pack_igetw(PACKFILE *f);  // little-endian word
int pack_iputw(int w, PACKFILE *f);
int pack_igetl(PACKFILE *f);  // little-endian long
int pack_iputl(int l, PACKFILE *f);
int pack_feof(PACKFILE *f);

// === Stage 5: Fonts and text ===

FONT *load_txt_font(const char *filename, void *param1, void *param2);
void textprintf_ex(BITMAP *bmp, FONT *fnt, int x, int y, int color, int bg, const char *fmt, ...);
void textprintf(BITMAP *bmp, FONT *fnt, int x, int y, int color, const char *fmt, ...);  // bg=-1
void textprintf_centre_ex(BITMAP *bmp, FONT *fnt, int x, int y, int color, int bg, const char *fmt, ...);
int  text_length(FONT *fnt, const char *str);
int  text_height(FONT *fnt);
extern int allegro_404_char;  // replacement character for missing glyphs (CP1251 '$' = 0x24)

// === Stage 10: Miscellaneous functions ===

void allegro_message(const char *fmt, ...);
int  allegro_init();
void allegro_exit();

void set_trans_blender(int r, int g, int b, int a);
void set_add_blender(int r, int g, int b, int a);
void set_alpha_blender();
void set_write_alpha_blender();  // may be needed by map bitmaps

void show_mouse(BITMAP *bmp);
void scare_mouse();
void unscare_mouse();
void remove_mouse();
extern BITMAP *mouse_sprite;
void set_mouse_sprite(BITMAP *sprite);

void set_keyboard_rate(int delay, int repeat);
void set_uformat(int fmt);
int  keypressed();
void clear_keybuf();
int  poll_keyboard();
int  poll_mouse();

const char *get_config_string(const char *section, const char *key, const char *def);
int         get_config_int(const char *section, const char *key, int def);

void rest(int ms);
int  get_desktop_resolution(int *w, int *h);
void set_display_switch_mode(int mode);
void set_window_title(const char *title);
void vsync();

#define SWITCH_BACKGROUND  1
#define SWITCH_BACKAMNESIA 3

void get_palette(PALETTE p);
void set_palette(const PALETTE p);

int  install_timer();
int  install_mouse();
int  install_keyboard();
int  install_sound(int digi, int midi, char *cfg);
void install_int(void (*fn)(), int ms);
void remove_timer();
void remove_keyboard();

#define DIGI_AUTODETECT  0
#define MIDI_AUTODETECT  0

#define LOCK_FUNCTION(f)
#define LOCK_VARIABLE(v)
#define END_OF_FUNCTION(f)
#define END_OF_MAIN()

void set_color_depth(int depth);

// ureadkey is used in Interface.cpp for name input
int ureadkey(int *scancode);
#define U_ASCII  0

// File operations (used for loading game data and saves)
void delete_file(const char *filename);
char *replace_extension(char *dest, const char *src, const char *ext, int size);

// Directory iteration (used for loading custom maps and saves)
struct al_ffblk {
    int attrib;
    long time;                // modification time (time_t in Allegro)
    long size;
    char name[512];
    void *ff_data;            // search state, owned by al_findfirst/al_findclose
};
int al_findfirst(const char *pattern, struct al_ffblk *info, int attrib);
int al_findnext(struct al_ffblk *info);
void al_findclose(struct al_ffblk *info);
#define FA_RDONLY 1
#define FA_HIDDEN 2
#define FA_DIREC  16
#define FA_ALL    0x7F  // file attribute mask for al_findfirst

// Sound playback (used by Audio.cpp)
SAMPLE *load_sample(const char *filename);
void play_sample(const SAMPLE *spl, int vol, int pan, int freq, int loop);

// Internal (used by allegro_init or via NativeDisplay.cpp in Allegro build)
extern void *system_driver;  // stub in raylib
int _install_allegro_version_check(int system_id, int *errno_ptr, int compat_version);

#ifdef __cplusplus
}
#endif
