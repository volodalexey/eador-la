// raylib window, present() and input (RAYLIB.md, stage 7).
// This file replaces NativeDisplay.cpp (X11/Xinerama → raylib).

#define EADOR_NATIVE_DISPLAY_IMPLEMENTATION
#include "NativeDisplay.h"
#include "NativeViewport.hpp"
#include "allegro_rl.h"
#include "rl_backend.h"

#include <cstdio>
#include <cstring>

// -------------------------------------------------------------------------
// Timer callbacks (registered via install_int)
// -------------------------------------------------------------------------
struct TimerEntry {
    void (*fn)();
    double interval_s;
    double last_fire;
    bool active;
};
static constexpr int MAX_TIMERS = 8;
static TimerEntry g_timers[MAX_TIMERS] = {};
static int g_timer_count = 0;

void rl_install_int(void (*fn)(), int ms) {
    if (!fn || ms <= 0) return;
    for (int i = 0; i < g_timer_count; i++) {
        if (g_timers[i].fn == fn) {
            g_timers[i].interval_s = ms / 1000.0;
            return;
        }
    }
    if (g_timer_count < MAX_TIMERS) {
        g_timers[g_timer_count++] = {fn, ms / 1000.0, 0.0, true};
    }
}

void rl_remove_int(void (*fn)()) {
    if (!fn) return;
    for (int i = 0; i < g_timer_count; i++) {
        if (g_timers[i].fn == fn) g_timers[i].active = false;
    }
}

void rl_pump();

// -------------------------------------------------------------------------
// Global display state
// -------------------------------------------------------------------------
namespace {
    BITMAP *canvas = nullptr;
    NativeViewport viewport{0, 0, 1024, 768};
    double last_draw = 0.0;

    // Raw mouse position in physical screen coordinates
    int raw_mouse_x = 0;
    int raw_mouse_y = 0;

    // Keyboard queue (for eador_readkey / ureadkey)
    static constexpr int KEYQ_SIZE = 32;
    static int key_queue_char[KEYQ_SIZE];
    static int key_queue_scan[KEYQ_SIZE];
    static int key_queue_head = 0;
    static int key_queue_tail = 0;

    inline bool key_queue_empty() { return key_queue_head == key_queue_tail; }
    inline void key_queue_push(int c, int scan) {
        int next = (key_queue_tail + 1) % KEYQ_SIZE;
        if (next != key_queue_head) {
            key_queue_char[key_queue_tail] = c;
            key_queue_scan[key_queue_tail] = scan;
            key_queue_tail = next;
        }
    }
    inline int key_queue_pop_char() {
        int c = key_queue_char[key_queue_head];
        key_queue_head = (key_queue_head + 1) % KEYQ_SIZE;
        return c;
    }
    inline int key_queue_pop_scan() {
        return key_queue_scan[(key_queue_head - 1 + KEYQ_SIZE) % KEYQ_SIZE];
    }

    // -------------------------------------------------------------------------
    // Input update — read raylib state → Allegro globals
    // -------------------------------------------------------------------------
    // Must run right after every poll: raylib resets its key/char queues on the next poll.
    void update_input() {
        if (canvas) {
            int sw, sh;
            rlb_screen_size(&sw, &sh);
            viewport = NativeViewport::fit(sw, sh);
        }

        // Key states
        rlb_keys_down(key);

        // Modifier flags
        key_shifts = 0;
        if (key[KEY_LSHIFT] || key[KEY_RSHIFT]) key_shifts |= 1;
        if (key[KEY_LCONTROL] || key[KEY_RCONTROL]) key_shifts |= 4;
        if (key[KEY_ALT] || key[KEY_ALTGR]) key_shifts |= 8;

        // Mouse
        int buttons, wheel_delta;
        rlb_mouse(&raw_mouse_x, &raw_mouse_y, &buttons, &wheel_delta);
        mouse_b = buttons;
        mouse_z += wheel_delta;

        // Keyboard events: chars and scancodes
        // raylib queues them separately; match them up
        // Strategy: drain chars first, then drain scancodes
        for (;;) {
            int c = rlb_next_char_pressed();
            if (!c) break;
            key_queue_push(c, 0);  // scan filled in later if possible
        }
        for (;;) {
            int scan = rlb_next_key_pressed();
            if (!scan) break;
            // If this key produces a char, attach scan to the last char without a scan
            if (rlb_key_produces_char(scan)) {
                bool found = false;
                for (int i = key_queue_head; i != key_queue_tail; i = (i + 1) % KEYQ_SIZE) {
                    if (key_queue_scan[i] == 0 && key_queue_char[i] > 0) {
                        key_queue_scan[i] = scan;
                        found = true;
                        break;
                    }
                }
                if (!found) key_queue_push(0, scan);
            } else {
                // Non-char key (ESC, arrows, F-keys, etc.)
                key_queue_push(0, scan);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Timer tick
    // -------------------------------------------------------------------------
    // Timer callbacks blit to the screen, which presents and ticks again.
    bool in_timers = false;

    void tick_timers() {
        if (in_timers) return;
        in_timers = true;
        double now = rlb_time();
        for (int i = 0; i < g_timer_count; i++) {
            TimerEntry &e = g_timers[i];
            if (!e.active || !e.fn) continue;
            if (now - e.last_fire >= e.interval_s) {
                e.last_fire = now;
                e.fn();
            }
        }
        in_timers = false;
    }

    // -------------------------------------------------------------------------
    // present() — upload canvas and draw letterboxed
    // -------------------------------------------------------------------------
    void present() {
        if (!canvas || !rlb_is_open()) return;
        rlb_draw((const uint32_t *)canvas->data, 1024, 768,
                 viewport.x, viewport.y, viewport.w, viewport.h);
        last_draw = rlb_time();
        update_input();
        tick_timers();
    }
}  // namespace

// -------------------------------------------------------------------------
// Public eador_* functions
// -------------------------------------------------------------------------

BITMAP *eador_screen() {
    return canvas;
}

int eador_set_gfx_mode(int card, int w, int h, int vw, int vh) {
    (void)vw; (void)vh;

    if (card == GFX_TEXT) {
        rlb_close_window();
        if (canvas) { destroy_bitmap(canvas); canvas = nullptr; }
        return 0;
    }

    bool fullscreen = (card == GFX_AUTODETECT_FULLSCREEN);
    if (!rlb_is_open()) {
        if (!rlb_open_window(fullscreen)) return -1;
    }

    int screen_w, screen_h;
    rlb_screen_size(&screen_w, &screen_h);
    if (fullscreen) {
        int mon_w, mon_h;
        rlb_monitor_size(&mon_w, &mon_h);
        viewport = NativeViewport::fit(mon_w, mon_h);
    } else {
        int win_w = (w > 0) ? w : 1024;
        int win_h = (h > 0) ? h : 768;
        viewport = NativeViewport::fit(win_w, win_h);
    }

    // Create logical 1024×768 canvas
    if (canvas) destroy_bitmap(canvas);
    canvas = create_bitmap(1024, 768);
    if (!canvas) return -1;

    clear_to_color(canvas, makecol(0, 0, 0));

    // Arm timers
    double now = rlb_time();
    for (int i = 0; i < g_timer_count; i++) g_timers[i].last_fire = now;

    return 0;
}

void eador_blit(BITMAP *source, BITMAP *destination,
                int sx, int sy, int dx, int dy, int bw, int bh) {
    blit(source, destination, sx, sy, dx, dy, bw, bh);
    if (destination == canvas) present();
}

int eador_mouse_x() {
    rl_pump();
    return canvas ? viewport.mouseX(raw_mouse_x) : raw_mouse_x;
}

int eador_mouse_y() {
    rl_pump();
    return canvas ? viewport.mouseY(raw_mouse_y) : raw_mouse_y;
}

void eador_position_mouse(int x, int y) {
    if (!rlb_is_open()) return;
    if (canvas) {
        int px = viewport.x + x * viewport.w / 1024;
        int py = viewport.y + y * viewport.h / 768;
        rlb_set_mouse(px, py);
    } else {
        rlb_set_mouse(x, y);
    }
}

int eador_readkey() {
    present();
    while (rlb_is_open() && !rlb_should_close()) {
        if (!key_queue_empty()) {
            int c = key_queue_pop_char();
            int scan = key_queue_pop_scan();
            return (scan << 8) | (c & 0xFF);
        }
        rl_pump();
        rlb_sleep(0.001);
    }
    return 0;
}

void eador_allegro_exit() {
    if (canvas) { destroy_bitmap(canvas); canvas = nullptr; }
    rlb_close_window();
}

// Game strings are CP1251 (-fexec-charset), raylib/GLFW expect UTF-8.
static int cp1251_to_unicode(unsigned char c) {
    static const unsigned short hi[64] = {
        0x0402,0x0403,0x201A,0x0453,0x201E,0x2026,0x2020,0x2021,0x20AC,0x2030,0x0409,0x2039,0x040A,0x040C,0x040B,0x040F,
        0x0452,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,0x003F,0x2122,0x0459,0x203A,0x045A,0x045C,0x045B,0x045F,
        0x00A0,0x040E,0x045E,0x0408,0x00A4,0x0490,0x00A6,0x00A7,0x0401,0x00A9,0x0404,0x00AB,0x00AC,0x00AD,0x00AE,0x0407,
        0x00B0,0x00B1,0x0406,0x0456,0x0491,0x00B5,0x00B6,0x00B7,0x0451,0x2116,0x0454,0x00BB,0x0458,0x0405,0x0455,0x0457,
    };
    if (c < 0x80) return c;
    if (c >= 0xC0) return 0x0410 + (c - 0xC0);
    return hi[c - 0x80];
}

void eador_set_window_title(const char *name) {
    if (!name) return;
    char utf8[512];
    size_t o = 0;
    for (const unsigned char *p = (const unsigned char *)name; *p && o + 4 < sizeof utf8; p++) {
        int u = cp1251_to_unicode(*p);
        if (u < 0x80) {
            utf8[o++] = (char)u;
        } else if (u < 0x800) {
            utf8[o++] = (char)(0xC0 | (u >> 6));
            utf8[o++] = (char)(0x80 | (u & 0x3F));
        } else {
            utf8[o++] = (char)(0xE0 | (u >> 12));
            utf8[o++] = (char)(0x80 | ((u >> 6) & 0x3F));
            utf8[o++] = (char)(0x80 | (u & 0x3F));
        }
    }
    utf8[o] = 0;
    rlb_set_title_utf8(utf8);
}

// -------------------------------------------------------------------------
// Allegro stubs forwarded from allegro_rl.cpp
// -------------------------------------------------------------------------

void rl_install_int_c(void (*fn)(), int ms) { rl_install_int(fn, ms); }

void rl_pump() {
    if (!rlb_is_open()) return;
    // Busy-wait loops call this thousands of times per second: redraw at most ~60 FPS.
    if (canvas && rlb_time() - last_draw >= 0.016) {
        present();
        return;
    }
    rlb_poll();
    update_input();
    tick_timers();
}

bool rl_keypressed() {
    if (!rlb_is_open()) return false;
    rl_pump();
    return !key_queue_empty();
}

void rl_clear_keybuf() {
    key_queue_head = key_queue_tail = 0;
}

int rl_ureadkey(int *scancode) {
    if (!rlb_is_open()) { if (scancode) *scancode = 0; return 0; }
    while (rlb_is_open() && !rlb_should_close()) {
        rl_pump();
        if (!key_queue_empty()) {
            int c = key_queue_pop_char();
            int scan = key_queue_pop_scan();
            if (scancode) *scancode = scan;
            return c;
        }
    }
    if (scancode) *scancode = 0;
    return 0;
}

void rl_rest(int ms) {
    if (!rlb_is_open()) {
        rlb_sleep(ms / 1000.0);
        return;
    }
    double end = rlb_time() + ms / 1000.0;
    while (rlb_time() < end) rl_pump();
}

int rl_get_desktop_resolution(int *w, int *h) {
    rlb_monitor_size(w, h);
    return 0;
}
