#pragma once

#include <allegro.h>

BITMAP *eador_screen();
int eador_set_gfx_mode(int card, int w, int h, int vw, int vh);
void eador_blit(BITMAP *source, BITMAP *destination,
                int sx, int sy, int dx, int dy, int w, int h);
int eador_mouse_x();
int eador_mouse_y();
void eador_position_mouse(int x, int y);
int eador_readkey();
void eador_allegro_exit();
void eador_set_window_title(const char *name);

// Only the native client uses a logical screen; Allegro itself and the audio
// adapter continue to see the original API and physical screen.
#ifndef EADOR_NATIVE_DISPLAY_IMPLEMENTATION
#define screen eador_screen()
#define set_gfx_mode eador_set_gfx_mode
#define blit eador_blit
#define mouse_x eador_mouse_x()
#define mouse_y eador_mouse_y()
#define position_mouse eador_position_mouse
#define readkey eador_readkey
#define allegro_exit eador_allegro_exit
#define set_window_title eador_set_window_title
#endif
