#pragma once

// Thin raylib backend for NativeDisplay_rl.cpp (RAYLIB.md, stage 7).
// It must not see allegro_rl.h: Allegro KEY_* macros clash with raylib's KeyboardKey enum.
// Keys are reported as Allegro scancodes, pixels are Allegro 32bpp (0x00RRGGBB).

#include <stdint.h>

bool   rlb_open_window(bool fullscreen);
void   rlb_close_window();
bool   rlb_is_open();
bool   rlb_should_close();  // WindowShouldClose()
void   rlb_screen_size(int *w, int *h);
void   rlb_monitor_size(int *w, int *h);
void   rlb_set_title_utf8(const char *title);
double rlb_time();
void   rlb_sleep(double seconds);

// Upload a w*h 0x00RRGGBB image and draw it into the (vx, vy, vw, vh) viewport.
// Also polls input events (EndDrawing does it).
void   rlb_draw(const uint32_t *pixels, int w, int h, int vx, int vy, int vw, int vh);
// Poll input events without drawing.
void   rlb_poll();

// Input state after the last rlb_draw/rlb_poll. Read it right after each call:
// raylib clears its queues on the next poll.
void   rlb_keys_down(uint8_t keys[256]);   // indexed by Allegro scancode
int    rlb_next_key_pressed();             // Allegro scancode, 0 when the queue is empty
int    rlb_next_char_pressed();            // Unicode code point, 0 when the queue is empty
bool   rlb_key_produces_char(int scancode);
void   rlb_mouse(int *x, int *y, int *buttons, int *wheel_delta);
void   rlb_set_mouse(int x, int y);
void   rlb_show_keyboard(bool show);       // show/hide soft keyboard on Android
