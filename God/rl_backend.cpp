// raylib backend (RAYLIB.md, stage 7).
// IMPORTANT: this file MUST NOT include allegro_rl.h.
// Allegro KEY_* macros collide with raylib's KeyboardKey enum values.

#include "rl_backend.h"
#include <raylib.h>
#include <cstdlib>
#include <cstring>
#include <time.h>

// -------------------------------------------------------------------------
// Raylib KeyboardKey → Allegro scancode mapping
// (Allegro scancodes are defined in allegro_rl.h; we keep copies here as
//  plain integers so we don't need to include that header.)
// -------------------------------------------------------------------------
// Allegro scancode constants (must stay in sync with allegro_rl.h §2.4)
static constexpr int AL_ESC        = 1;
static constexpr int AL_1          = 2;
static constexpr int AL_2          = 3;
static constexpr int AL_3          = 4;
static constexpr int AL_4          = 5;
static constexpr int AL_5          = 6;
static constexpr int AL_6          = 7;
static constexpr int AL_7          = 8;
static constexpr int AL_8          = 9;
static constexpr int AL_9          = 10;
static constexpr int AL_0          = 11;
static constexpr int AL_MINUS      = 12;
static constexpr int AL_EQUALS     = 13;
static constexpr int AL_BACKSPACE  = 14;
static constexpr int AL_TAB        = 15;
static constexpr int AL_Q          = 16;
static constexpr int AL_W          = 17;
static constexpr int AL_E          = 18;
static constexpr int AL_R          = 19;
static constexpr int AL_T          = 20;
static constexpr int AL_Y          = 21;
static constexpr int AL_U          = 22;
static constexpr int AL_I          = 23;
static constexpr int AL_O          = 24;
static constexpr int AL_P          = 25;
static constexpr int AL_OPENBRACE  = 26;
static constexpr int AL_CLOSEBRACE = 27;
static constexpr int AL_ENTER      = 28;
static constexpr int AL_LCONTROL   = 29;
static constexpr int AL_A          = 30;
static constexpr int AL_S          = 31;
static constexpr int AL_D          = 32;
static constexpr int AL_F          = 33;
static constexpr int AL_G          = 34;
static constexpr int AL_H          = 35;
static constexpr int AL_J          = 36;
static constexpr int AL_K          = 37;
static constexpr int AL_L          = 38;
static constexpr int AL_COLON      = 39;
static constexpr int AL_QUOTE      = 40;
static constexpr int AL_TILDE      = 41;
static constexpr int AL_LSHIFT     = 42;
static constexpr int AL_BACKSLASH  = 43;
static constexpr int AL_Z          = 44;
static constexpr int AL_X          = 45;
static constexpr int AL_C          = 46;
static constexpr int AL_V          = 47;
static constexpr int AL_B          = 48;
static constexpr int AL_N          = 49;
static constexpr int AL_M          = 50;
static constexpr int AL_COMMA      = 51;
static constexpr int AL_STOP       = 52;
static constexpr int AL_SLASH      = 53;
static constexpr int AL_RSHIFT     = 54;
static constexpr int AL_ASTERISK   = 55;
static constexpr int AL_ALT        = 56;
static constexpr int AL_SPACE      = 57;
static constexpr int AL_CAPSLOCK   = 58;
static constexpr int AL_F1         = 59;
static constexpr int AL_F2         = 60;
static constexpr int AL_F3         = 61;
static constexpr int AL_F4         = 62;
static constexpr int AL_F5         = 63;
static constexpr int AL_F6         = 64;
static constexpr int AL_F7         = 65;
static constexpr int AL_F8         = 66;
static constexpr int AL_F9         = 67;
static constexpr int AL_F10        = 68;
static constexpr int AL_NUMLOCK    = 69;
static constexpr int AL_SCRLOCK    = 70;
static constexpr int AL_HOME       = 71;
static constexpr int AL_UP         = 72;
static constexpr int AL_PGUP       = 73;
static constexpr int AL_MINUS_PAD  = 74;
static constexpr int AL_LEFT       = 75;
static constexpr int AL_5_PAD      = 76;
static constexpr int AL_RIGHT      = 77;
static constexpr int AL_PLUS_PAD   = 78;
static constexpr int AL_END        = 79;
static constexpr int AL_DOWN       = 80;
static constexpr int AL_PGDN       = 81;
static constexpr int AL_INSERT     = 82;
static constexpr int AL_DEL        = 83;
static constexpr int AL_F11        = 87;
static constexpr int AL_F12        = 88;
static constexpr int AL_RCONTROL   = 97;
static constexpr int AL_ALTGR      = 100;
static constexpr int AL_ENTER_PAD  = 108;
static constexpr int AL_PAUSE      = 119;

// Flat lookup: raylib key code → Allegro scancode.
// raylib key codes go up to ~348 (KEY_KP_EQUAL); anything unmapped stays 0.
static constexpr int RL_KEY_MAX = 350;
static int g_rl_to_al[RL_KEY_MAX] = {};  // zero-initialised

static void build_keymap() {
    auto set = [](int rl, int al) {
        if (rl >= 0 && rl < RL_KEY_MAX) g_rl_to_al[rl] = al;
    };
    set(KEY_ESCAPE,       AL_ESC);
    set(KEY_ONE,          AL_1);
    set(KEY_TWO,          AL_2);
    set(KEY_THREE,        AL_3);
    set(KEY_FOUR,         AL_4);
    set(KEY_FIVE,         AL_5);
    set(KEY_SIX,          AL_6);
    set(KEY_SEVEN,        AL_7);
    set(KEY_EIGHT,        AL_8);
    set(KEY_NINE,         AL_9);
    set(KEY_ZERO,         AL_0);
    set(KEY_MINUS,        AL_MINUS);
    set(KEY_EQUAL,        AL_EQUALS);
    set(KEY_BACKSPACE,    AL_BACKSPACE);
    set(KEY_TAB,          AL_TAB);
    set(KEY_Q,            AL_Q);
    set(KEY_W,            AL_W);
    set(KEY_E,            AL_E);
    set(KEY_R,            AL_R);
    set(KEY_T,            AL_T);
    set(KEY_Y,            AL_Y);
    set(KEY_U,            AL_U);
    set(KEY_I,            AL_I);
    set(KEY_O,            AL_O);
    set(KEY_P,            AL_P);
    set(KEY_LEFT_BRACKET, AL_OPENBRACE);
    set(KEY_RIGHT_BRACKET,AL_CLOSEBRACE);
    set(KEY_ENTER,        AL_ENTER);
    set(KEY_LEFT_CONTROL, AL_LCONTROL);
    set(KEY_RIGHT_CONTROL,AL_RCONTROL);
    set(KEY_A,            AL_A);
    set(KEY_S,            AL_S);
    set(KEY_D,            AL_D);
    set(KEY_F,            AL_F);
    set(KEY_G,            AL_G);
    set(KEY_H,            AL_H);
    set(KEY_J,            AL_J);
    set(KEY_K,            AL_K);
    set(KEY_L,            AL_L);
    set(KEY_SEMICOLON,    AL_COLON);
    set(KEY_APOSTROPHE,   AL_QUOTE);
    set(KEY_GRAVE,        AL_TILDE);
    set(KEY_LEFT_SHIFT,   AL_LSHIFT);
    set(KEY_RIGHT_SHIFT,  AL_RSHIFT);
    set(KEY_BACKSLASH,    AL_BACKSLASH);
    set(KEY_Z,            AL_Z);
    set(KEY_X,            AL_X);
    set(KEY_C,            AL_C);
    set(KEY_V,            AL_V);
    set(KEY_B,            AL_B);
    set(KEY_N,            AL_N);
    set(KEY_M,            AL_M);
    set(KEY_COMMA,        AL_COMMA);
    set(KEY_PERIOD,       AL_STOP);
    set(KEY_SLASH,        AL_SLASH);
    set(KEY_LEFT_ALT,     AL_ALT);
    set(KEY_RIGHT_ALT,    AL_ALTGR);
    set(KEY_SPACE,        AL_SPACE);
    set(KEY_CAPS_LOCK,    AL_CAPSLOCK);
    set(KEY_F1,           AL_F1);
    set(KEY_F2,           AL_F2);
    set(KEY_F3,           AL_F3);
    set(KEY_F4,           AL_F4);
    set(KEY_F5,           AL_F5);
    set(KEY_F6,           AL_F6);
    set(KEY_F7,           AL_F7);
    set(KEY_F8,           AL_F8);
    set(KEY_F9,           AL_F9);
    set(KEY_F10,          AL_F10);
    set(KEY_F11,          AL_F11);
    set(KEY_F12,          AL_F12);
    set(KEY_NUM_LOCK,     AL_NUMLOCK);
    set(KEY_SCROLL_LOCK,  AL_SCRLOCK);
    set(KEY_HOME,         AL_HOME);
    set(KEY_UP,           AL_UP);
    set(KEY_PAGE_UP,      AL_PGUP);
    set(KEY_KP_SUBTRACT,  AL_MINUS_PAD);
    set(KEY_LEFT,         AL_LEFT);
    set(KEY_KP_5,         AL_5_PAD);
    set(KEY_RIGHT,        AL_RIGHT);
    set(KEY_KP_ADD,       AL_PLUS_PAD);
    set(KEY_END,          AL_END);
    set(KEY_DOWN,         AL_DOWN);
    set(KEY_PAGE_DOWN,    AL_PGDN);
    set(KEY_INSERT,       AL_INSERT);
    set(KEY_DELETE,       AL_DEL);
    set(KEY_KP_ENTER,     AL_ENTER_PAD);
    set(KEY_PAUSE,        AL_PAUSE);
}

// -------------------------------------------------------------------------
// GPU upload texture (1024×768 RGBA8, reused every frame)
// -------------------------------------------------------------------------
static Texture2D g_tex = {0};
static bool g_window_open = false;
// Conversion buffer: 0x00RRGGBB → RGBA8
static unsigned char g_rgba[1024 * 768 * 4];

bool rlb_open_window(bool fullscreen) {
    if (g_window_open) return true;
    build_keymap();
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 768, "Eador");
    if (!IsWindowReady()) return false;
    SetExitKey(0);   // disable ESC-closes-window; game handles exit itself
    HideCursor();    // the game draws mouse_sprite into the frame itself
    SetTargetFPS(0); // uncapped; we drive the loop
    InitAudioDevice();
    if (fullscreen) {
        int mon = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(mon), GetMonitorHeight(mon));
        ToggleFullscreen();
    }
    // Create the upload texture (empty 1024×768 RGBA8)
    Image blank = {};
    blank.data    = g_rgba;
    blank.width   = 1024;
    blank.height  = 768;
    blank.mipmaps = 1;
    blank.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    g_tex = LoadTextureFromImage(blank);
    SetTextureFilter(g_tex, TEXTURE_FILTER_BILINEAR);
    g_window_open = true;
    return true;
}

void rlb_close_window() {
    if (!g_window_open) return;
    if (g_tex.id) UnloadTexture(g_tex);
    g_tex.id = 0;
    CloseAudioDevice();
    CloseWindow();
    g_window_open = false;
}

bool rlb_is_open() { return g_window_open; }

bool rlb_should_close() {
    return g_window_open && WindowShouldClose();
}

void rlb_screen_size(int *w, int *h) {
    if (!g_window_open) { *w = 1024; *h = 768; return; }
    *w = GetScreenWidth();
    *h = GetScreenHeight();
}

void rlb_monitor_size(int *w, int *h) {
    int mon = g_window_open ? GetCurrentMonitor() : 0;
    *w = GetMonitorWidth(mon);
    *h = GetMonitorHeight(mon);
}

void rlb_set_title_utf8(const char *title) {
    if (g_window_open && title) SetWindowTitle(title);
}

double rlb_time() { return GetTime(); }

void rlb_sleep(double seconds) {
    // Can't use WaitTime() here — it blocks without polling.
    // Sleep is only called from rest() with no window, so plain sleep is fine.
    if (seconds > 0) {
        struct timespec ts = { (long)seconds, (long)((seconds - (long)seconds) * 1e9) };
        nanosleep(&ts, nullptr);
    }
}

void rlb_poll() {
    if (!g_window_open) return;
    PollInputEvents();
}

void rlb_draw(const uint32_t *pixels, int w, int h, int vx, int vy, int vw, int vh) {
    if (!g_window_open || !pixels) return;

    // Convert 0x00RRGGBB → RGBA8
    int n = w * h;
    for (int i = 0; i < n; i++) {
        uint32_t c = pixels[i];
        g_rgba[i * 4 + 0] = (c >> 16) & 0xFF;
        g_rgba[i * 4 + 1] = (c >> 8)  & 0xFF;
        g_rgba[i * 4 + 2] = c         & 0xFF;
        g_rgba[i * 4 + 3] = 0xFF;
    }
    UpdateTexture(g_tex, g_rgba);

    // Debug: EADOR_RL_DUMP=/path/frame.png [EADOR_RL_DUMP_AT=seconds] saves one frame.
    static bool dumped = false;
    if (!dumped && w == 1024 && h == 768) {
        const char *path = getenv("EADOR_RL_DUMP");
        const char *at = getenv("EADOR_RL_DUMP_AT");
        double when = at ? atof(at) : 8.0;
        if (path && GetTime() >= when) {
            Image img = {g_rgba, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
            ExportImage(img, path);
            dumped = true;
        }
    }

    BeginDrawing();
    ClearBackground(BLACK);
    Rectangle src = {0, 0, (float)w, (float)h};
    Rectangle dst = {(float)vx, (float)vy, (float)vw, (float)vh};
    DrawTexturePro(g_tex, src, dst, {0, 0}, 0.0f, WHITE);
    EndDrawing();
    // EndDrawing() already calls PollInputEvents() internally
}

// -------------------------------------------------------------------------
// Input
// -------------------------------------------------------------------------

void rlb_keys_down(uint8_t keys[256]) {
    for (int i = 0; i < 256; i++) keys[i] = 0;
    for (int rl = 0; rl < RL_KEY_MAX; rl++) {
        int al = g_rl_to_al[rl];
        if (al > 0 && al < 256 && IsKeyDown((KeyboardKey)rl))
            keys[al] = 1;
    }
}

int rlb_next_key_pressed() {
    int rl = GetKeyPressed();
    if (rl <= 0 || rl >= RL_KEY_MAX) return 0;
    return g_rl_to_al[rl];
}

int rlb_next_char_pressed() {
    return GetCharPressed();
}

bool rlb_key_produces_char(int al_scancode) {
    // Printable key range in Allegro scancodes: letters (30-53) + digits (2-11) + space (57)
    return (al_scancode >= AL_1 && al_scancode <= AL_SLASH) ||
            al_scancode == AL_SPACE ||
            al_scancode == AL_TILDE ||
            al_scancode == AL_BACKSLASH ||
            al_scancode == AL_OPENBRACE ||
            al_scancode == AL_CLOSEBRACE ||
            al_scancode == AL_EQUALS ||
            al_scancode == AL_COLON ||
            al_scancode == AL_QUOTE ||
            al_scancode == AL_COMMA ||
            al_scancode == AL_STOP;
}

void rlb_mouse(int *x, int *y, int *buttons, int *wheel_delta) {
    if (!g_window_open) { *x = 0; *y = 0; *buttons = 0; *wheel_delta = 0; return; }
    Vector2 pos = GetMousePosition();
    *x = (int)pos.x;
    *y = (int)pos.y;
    *buttons = 0;
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))   *buttons |= 1;
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))  *buttons |= 2;
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) *buttons |= 4;
    *wheel_delta = (int)GetMouseWheelMove();
}

void rlb_set_mouse(int x, int y) {
    if (g_window_open) SetMousePosition(x, y);
}
