#define EADOR_NATIVE_DISPLAY_IMPLEMENTATION
#include "NativeDisplay.h"
#include "NativeViewport.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iconv.h>
#include <mutex>
#include <vector>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xinerama.h>
#include <xalleg.h>

namespace {
BITMAP *canvas = nullptr;
NativeViewport viewport{0, 0, 1024, 768};
std::recursive_mutex displayMutex;

struct MonitorGeometry {
    int x, y, width, height;
};

bool current_monitor_geometry(MonitorGeometry *geometry) {
    Display *display = XOpenDisplay(nullptr);
    if (!display)
        return false;

    Window root = DefaultRootWindow(display);
    Window root_return;
    Window child_return;
    int root_x, root_y, window_x, window_y;
    unsigned int mask;
    bool result = false;

    if (XQueryPointer(display, root, &root_return, &child_return,
                      &root_x, &root_y, &window_x, &window_y, &mask)) {
        int monitor_count = 0;
        XineramaScreenInfo *monitors = XineramaQueryScreens(display, &monitor_count);
        for (int i = 0; monitors && i < monitor_count; ++i) {
            const auto &monitor = monitors[i];
            if (root_x >= monitor.x_org && root_x < monitor.x_org + monitor.width &&
                root_y >= monitor.y_org && root_y < monitor.y_org + monitor.height) {
                geometry->x = monitor.x_org;
                geometry->y = monitor.y_org;
                geometry->width = monitor.width;
                geometry->height = monitor.height;
                result = true;
                break;
            }
        }
        if (monitors)
            XFree(monitors);
    }

    XCloseDisplay(display);
    return result;
}

void move_fullscreen_window(const MonitorGeometry &monitor) {
    if (!_xwin.display || !_xwin.fs_window)
        return;

    XMoveResizeWindow(_xwin.display, _xwin.fs_window,
                      monitor.x, monitor.y, monitor.width, monitor.height);
    XRaiseWindow(_xwin.display, _xwin.fs_window);
    XFlush(_xwin.display);
}

// Иконка окна из eador.ico (копируется из goggame-*.ico при импорте).
// Берутся все 32-битные BMP-образы; PNG-образы пропускаются.
// Формат _NET_WM_ICON: подряд [ширина, высота, ARGB...] для каждого размера.
std::vector<unsigned long> load_ico_for_net_wm(const char *path) {
    std::vector<unsigned long> result;
    FILE *f = fopen(path, "rb");
    if (!f)
        return result;
    std::vector<unsigned char> data;
    unsigned char buf[65536];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
        data.insert(data.end(), buf, buf + n);
    fclose(f);

    auto u16 = [&](size_t p) { return p + 2 <= data.size() ? data[p] | data[p + 1] << 8 : 0; };
    auto u32 = [&](size_t p) {
        return p + 4 <= data.size()
            ? uint32_t(data[p]) | uint32_t(data[p + 1]) << 8 |
              uint32_t(data[p + 2]) << 16 | uint32_t(data[p + 3]) << 24
            : 0u;
    };
    if (data.size() < 6 || u16(0) != 0 || u16(2) != 1)
        return result;

    const int count = u16(4);
    for (int i = 0; i < count; ++i) {
        const size_t entry = 6 + 16 * size_t(i);
        const size_t size = u32(entry + 8);
        const size_t offset = u32(entry + 12);
        if (offset + 40 > data.size() || offset + size > data.size())
            continue;
        const size_t header = u32(offset);
        const int w = int(int32_t(u32(offset + 4)));
        const int h = int(int32_t(u32(offset + 8))) / 2;  // XOR + AND маски
        const int bpp = u16(offset + 14);
        if (header < 40 || bpp != 32 || w <= 0 || h <= 0 ||
            offset + header + size_t(w) * h * 4 > data.size())
            continue;
        result.push_back(w);
        result.push_back(h);
        for (int y = 0; y < h; ++y) {
            // BMP хранит строки снизу вверх, пиксели в порядке BGRA.
            const unsigned char *row = &data[offset + header + size_t(h - 1 - y) * w * 4];
            for (int x = 0; x < w; ++x) {
                const unsigned char *p = row + x * 4;
                result.push_back(
                    (unsigned long)(p[3]) << 24 | (unsigned long)(p[2]) << 16 |
                    (unsigned long)(p[1]) << 8 | p[0]);
            }
        }
    }
    return result;
}

void set_window_icon() {
    if (!_xwin.display || _xwin.wm_window == None)
        return;
    const std::vector<unsigned long> icon = load_ico_for_net_wm("eador.ico");
    if (icon.empty())
        return;
    XLOCK();
    XChangeProperty(_xwin.display, _xwin.wm_window,
                    XInternAtom(_xwin.display, "_NET_WM_ICON", False), XA_CARDINAL, 32,
                    PropModeReplace, reinterpret_cast<const unsigned char *>(icon.data()),
                    int(icon.size()));
    XFlush(_xwin.display);
    XUNLOCK();
}

void present() {
    if (!canvas || !screen)
        return;
    stretch_blit(canvas, screen, 0, 0, 1024, 768,
                 viewport.x, viewport.y, viewport.w, viewport.h);
}
}

BITMAP *eador_screen() {
    return canvas ? canvas : screen;
}

int eador_set_gfx_mode(int card, int w, int h, int vw, int vh) {
    std::lock_guard<std::recursive_mutex> lock(displayMutex);
    if (canvas) {
        destroy_bitmap(canvas);
        canvas = nullptr;
    }
    if (card != GFX_AUTODETECT_FULLSCREEN) {
        const int result = set_gfx_mode(card, w, h, vw, vh);
        if (result == 0)
            set_window_icon();
        return result;
    }

    MonitorGeometry monitor{0, 0, 0, 0};
    if (!current_monitor_geometry(&monitor)) {
        if (get_desktop_resolution(&monitor.width, &monitor.height) != 0)
            return -1;
    }
    int desktopW = monitor.width;
    int desktopH = monitor.height;
    if (desktopW < 4 || desktopH < 3)
        return -1;

    // Keep the desktop mode instead of switching the monitor to 1024x768,
    // where the monitor/driver could stretch a 4:3 frame to a widescreen panel.
    if (set_gfx_mode(card, desktopW, desktopH, 0, 0) != 0)
        return -1;

    move_fullscreen_window(monitor);
    set_window_icon();

    canvas = create_bitmap_ex(bitmap_color_depth(screen), 1024, 768);
    if (!canvas) {
        set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
        return -1;
    }
    viewport = NativeViewport::fit(desktopW, desktopH);
    clear_to_color(canvas, makecol(0, 0, 0));
    clear_to_color(screen, makecol(0, 0, 0));
    return 0;
}

void eador_blit(BITMAP *source, BITMAP *destination,
                int sx, int sy, int dx, int dy, int w, int h) {
    std::lock_guard<std::recursive_mutex> lock(displayMutex);
    blit(source, destination, sx, sy, dx, dy, w, h);
    if (canvas && destination == canvas)
        present();
}

int eador_mouse_x() {
    return canvas ? viewport.mouseX(mouse_x) : mouse_x;
}

int eador_mouse_y() {
    return canvas ? viewport.mouseY(mouse_y) : mouse_y;
}

void eador_position_mouse(int x, int y) {
    if (canvas)
        position_mouse(viewport.x + x * viewport.w / 1024,
                       viewport.y + y * viewport.h / 768);
    else
        position_mouse(x, y);
}

int eador_readkey() {
    // Error/loading screens sometimes draw directly on the logical screen
    // and then wait for a key, without a screen blit in between.
    {
        std::lock_guard<std::recursive_mutex> lock(displayMutex);
        present();
    }
    return readkey();
}

// Строки игры собраны в CP1251 (-fexec-charset), а Allegro передаёт заголовок
// в XStoreName как есть, и оконный менеджер показывает его как Latin-1.
// Дополнительно пишем _NET_WM_NAME в UTF-8.
void eador_set_window_title(const char *name) {
    set_window_title(name);
    if (!name || !_xwin.display || _xwin.wm_window == None)
        return;

    char utf8[512];
    iconv_t cd = iconv_open("UTF-8", "CP1251");
    if (cd == (iconv_t)-1)
        return;
    char *in = const_cast<char *>(name);
    size_t inLeft = strlen(name);
    char *out = utf8;
    size_t outLeft = sizeof(utf8) - 1;
    const size_t rc = iconv(cd, &in, &inLeft, &out, &outLeft);
    iconv_close(cd);
    if (rc == (size_t)-1)
        return;
    *out = '\0';

    XLOCK();
    const Atom utf8String = XInternAtom(_xwin.display, "UTF8_STRING", False);
    const auto *bytes = reinterpret_cast<const unsigned char *>(utf8);
    const int length = int(out - utf8);
    XChangeProperty(_xwin.display, _xwin.wm_window,
                    XInternAtom(_xwin.display, "_NET_WM_NAME", False), utf8String, 8,
                    PropModeReplace, bytes, length);
    XChangeProperty(_xwin.display, _xwin.wm_window,
                    XInternAtom(_xwin.display, "_NET_WM_ICON_NAME", False), utf8String, 8,
                    PropModeReplace, bytes, length);
    XFlush(_xwin.display);
    XUNLOCK();
}

void eador_allegro_exit() {
    // Stop callbacks before freeing the canvas that they can draw into.
    remove_timer();
    std::lock_guard<std::recursive_mutex> lock(displayMutex);
    if (canvas) {
        destroy_bitmap(canvas);
        canvas = nullptr;
    }
    allegro_exit();
}
