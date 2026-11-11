# Миграция Eador. Genesis с Allegro 4 на raylib

Репозиторий: `/mnt/ssd2/projects/temp/eador-la`  
Raylib в субмодуле: `raylib/` (зафиксирован на теге `6.2`).

---

## Состояние кода

| Компонент | Что сейчас | Суть проблемы при замене |
|---|---|---|
| Графика | Allegro 4 `BITMAP*`, `blit`, `draw_sprite`, `draw_trans_sprite`, `draw_lit_sprite`, `rotate_sprite`, `stretch_blit`, `textprintf*`, `rectfill`, `getpixel/putpixel` | Нужна эмуляция `BITMAP` поверх `Image`/`RenderTexture2D` |
| Слой отображения | `NativeDisplay.h` перекрывает `screen`, `blit`, `mouse_x/y`, `readkey`, `set_gfx_mode` макросами | Единственная точка замены — уже существует |
| Шрифты | `Font1`/`Font2` — Allegro-шрифты из `font01.pcx` + `font01.txt` (рисованный bitmap-шрифт) | Загрузить один раз как `Image`, нарезать глифы |
| Таймер | `install_int(ItIsTime, 20ms)`, `install_int(AnimHourglass, 200ms)`, `install_int(MouseTime, 5ms)` — прерывания | Перенести в главный цикл raylib по deltaTime |
| Ввод | `mouse_x/y/b/z`, `key[]`, `keypressed()`, `readkey()`, `ureadkey()`, `poll_keyboard/mouse()` | Переписать через raylib Input |
| Звук (Ogg) | `alogg_impl.cpp` — собственный alogg поверх `play_audio_stream` / `get_audio_stream_buffer` Allegro | Переписать на `AudioStream` raylib |
| Данные | Allegro `.dat`-файлы (`load_datafile`) со спрайтами PCX/BMP и Ogg | Написать парсер `.dat`, или конвертировать ресурсы |
| Сохранения | `pack_fopen` / `pack_fwrite` / `load_pcx_pf` — Allegro LZSS + PCX | Заменить на `fopen` + zlib/miniz (PCX разобрать вручную) |
| `COLOR_0` | `makecol(255,0,255)` — цвет прозрачности (magenta chroma-key) | Учесть при загрузке спрайтов из `.dat` |

Всего файлов игры: `God/` — 29 `.cpp`, ~84 заголовка.  
Код игры не знает о платформе: все платформозависимые вызовы идут через `NativeDisplay.h` (макросы) и `Globals.h` (включает `allegro.h`). Это главный рычаг замены.

---

## Стратегия

Менять код самой игры нельзя — он большой и рабочий. Вместо этого:

1. Написать **тонкий слой совместимости** `allegro_rl.h`/`allegro_rl.cpp`, который реализует ровно тот подмножество Allegro 4 API, что реально использует игра, поверх raylib.
2. Заменить `allegro.h` на `allegro_rl.h` в точке включения (`Globals.h`).
3. Заменить `NativeDisplay.cpp` на `NativeDisplay_rl.cpp` (вместо X11/Xinerama — raylib Window).
4. Заменить `alogg_impl.cpp` на версию поверх `AudioStream` raylib.
5. Убрать `allegro4/` из сборки, добавить `raylib/` через `add_subdirectory`.

Код игры остаётся нетронутым.

---

## Этапы

### Этап 1. Инфраструктура сборки

**Цель:** заменить `allegro4/` на `raylib/` в CMake, проект должен компилироваться (с заглушками).

1.1. Добавить в корневой `CMakeLists.txt` блок для raylib-сборки рядом с `ALLEGRO4_PREFIX`:

```cmake
option(USE_RAYLIB "Build with raylib instead of Allegro 4" OFF)

if(USE_RAYLIB)
  set(RAYLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  add_subdirectory(raylib)
  add_subdirectory("God")  # God/CMakeLists.txt для raylib
endif()
```

1.2. Создать `God/CMakeLists_rl.txt` (или добавить ветку `if(USE_RAYLIB)` в существующий `God/CMakeLists.txt`):
- Убрать `pkg_check_modules(VORBIS …)`, `X11`, `XINERAMA`.
- Убрать `NativeDisplay.cpp` из `EADOR_SOURCES`.
- Добавить `NativeDisplay_rl.cpp`, `alogg_impl_rl.cpp`, `allegro_rl.cpp`.
- `target_link_libraries(eador_core PUBLIC raylib)`.
- Убрать `-fexec-charset=CP1251` (raylib не нужен CP1251, см. этап 3).

1.3. Добавить цель в `Makefile`:

```make
raylib:
    cmake -S . -B out/build-rl -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DUSE_RAYLIB=ON
    cmake --build out/build-rl
```

**Проверка:** `cmake --build out/build-rl` доходит до линковки (пусть с неразрешёнными символами — их закроют следующие этапы).

---

### Этап 2. Слой совместимости `allegro_rl` — типы и структуры

**Цель:** объявить все типы Allegro, которые использует игра, так чтобы `Globals.h` и весь `God/` компилировались без `allegro.h`.

Создать `God/allegro_rl.h`. Файл **не** включает `allegro.h`.

#### 2.1. BITMAP

```cpp
struct BITMAP {
    int w, h;
    int color_depth;        // всегда 32 для raylib-сборки
    // Пиксели в RGBA8, строки снизу вверх (как Allegro)
    uint8_t **line;         // указатели на строки (для ->line[y])
    uint8_t *data;          // непрерывный буфер w*h*4
    bool is_screen_target;  // true для canvas и экрана
    // Только для offscreen render:
    RenderTexture2D *rt;    // nullptr если CPU-bitmap
};
```

Allegro хранит пиксели в `bmp->line[y]`, игра использует именно такой доступ в `G_Object.cpp` (многочисленные `getpixel`/`putpixel`). Слой должен поддерживать оба способа.

#### 2.2. FONT

```cpp
struct FONT {
    // bitmap-шрифт: набор глифов из PCX
    struct Glyph { int x, y, w, h; };  // в atlas-текстуре
    Texture2D atlas;
    Glyph glyphs[256];
    int height;
};
```

#### 2.3. Остальные типы

```cpp
typedef struct { uint8_t r, g, b; } RGB;
typedef RGB PALETTE[256];           // используется только при save/load PCX карты
typedef struct { void *dat; int size; int type; } DATAFILE_ENTRY;
typedef DATAFILE_ENTRY DATAFILE;
typedef void AUDIOSTREAM;           // непрозрачный, определяется в allegro_rl.cpp
typedef void SAMPLE;
typedef struct { int voice; } AUDIOSTREAM_INFO;
#define ALOGG_OK 0
#define ALOGG_POLL_NOTPLAYING -1
#define ALOGG_POLL_PLAYJUSTFINISHED 1
struct ALOGG_OGG;                   // определяется в alogg_impl_rl.cpp
```

#### 2.4. Константы и макросы Allegro

```cpp
#define GFX_AUTODETECT_FULLSCREEN  0
#define GFX_AUTODETECT_WINDOWED    1
#define GFX_TEXT                  -1
#define KEY_ESC    1
#define KEY_F1    59
// ... все KEY_* которые реально проверяет игра (key[KEY_F1] и др.)

extern uint8_t key[256];     // обновляется в главном цикле
extern int     key_shifts;
extern int     mouse_b;
extern int     mouse_z;

// Blender mode (хранить глобально, применять в draw_trans_sprite / draw_lit_sprite)
extern int _blender_r, _blender_g, _blender_b, _blender_a;
```

**Проверка:** `God/*.cpp` компилируются с `-include allegro_rl.h` вместо `allegro.h` (без линковки).

---

### Этап 3. Кодировка строк (CP1251)

Игра скомпилирована с `-fexec-charset=CP1251 -finput-charset=UTF-8`, строковые литералы в коде — кириллица в CP1251. Шрифты (`font01.pcx`) ожидают CP1251.

При сборке с raylib можно оставить тот же флаг, если компилятор его поддерживает (GCC — да). **Не менять кодировку файлов игры.**

Добавить в CMake для raylib-таргета:

```cmake
target_compile_options(eador_core PUBLIC
  -fexec-charset=CP1251
  -finput-charset=UTF-8
)
```

Raylib никак не затрагивается этим флагом — он компилируется отдельно.

**Проверка:** строковые литералы в `.rodata` содержат CP1251-байты (проверить `xxd` на откомпилированном объектнике).

---

### Этап 4. Слой совместимости — графические примитивы

Реализовать в `God/allegro_rl.cpp`. Все функции работают с `BITMAP*`, не вызывают BeginDrawing/EndDrawing.

#### 4.1. Создание/уничтожение bitmap

```cpp
BITMAP *create_bitmap(int w, int h);           // CPU-bitmap, пиксели в data
BITMAP *create_bitmap_ex(int depth, int w, int h);
void    destroy_bitmap(BITMAP *bmp);
int     bitmap_color_depth(BITMAP *bmp);       // всегда 32
```

#### 4.2. Пиксельные операции (CPU)

```cpp
int  getpixel(BITMAP *bmp, int x, int y);      // возвращает 0xAARRGGBB
void putpixel(BITMAP *bmp, int x, int y, int color);
int  makecol(int r, int g, int b);             // → 0xFF000000 | r<<16 | g<<8 | b
int  getr(int color); int getg(int color); int getb(int color);
```

#### 4.3. Рисование прямоугольников

```cpp
void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void hline(BITMAP *bmp, int x, int y, int x2, int color);
void vline(BITMAP *bmp, int x, int y, int y2, int color);
void clear_bitmap(BITMAP *bmp);
void clear_to_color(BITMAP *bmp, int color);
```

#### 4.4. Blit / sprite / stretch

```cpp
// Прямое копирование прямоугольника
void blit(BITMAP *src, BITMAP *dst,
          int sx, int sy, int dx, int dy, int w, int h);

// Blit с учётом COLOR_0 (magenta = прозрачность)
void draw_sprite(BITMAP *dst, BITMAP *src, int x, int y);
void masked_blit(BITMAP *src, BITMAP *dst,
                 int sx, int sy, int dx, int dy, int w, int h);

// Blit с масштабированием
void stretch_blit(BITMAP *src, BITMAP *dst,
                  int sx, int sy, int sw, int sh,
                  int dx, int dy, int dw, int dh);

// draw_trans_sprite: наложить src на dst с alpha = _blender_a / 255
// (игра всегда вызывает set_trans_blender(128,128,128,128) → alpha 50%)
void draw_trans_sprite(BITMAP *dst, BITMAP *src, int x, int y);

// draw_lit_sprite: colorize src цветом (_blender_r/g/b) с интенсивностью level/255
void draw_lit_sprite(BITMAP *dst, BITMAP *src, int x, int y, int level);

// rotate_sprite: угол в fixed (Allegro fixed = int, 1.0 = 1<<16)
void rotate_sprite(BITMAP *dst, BITMAP *src, int x, int y, int angle);
```

#### 4.5. Перенос bitmap ↔ Texture2D (для показа на экране)

Внутренняя функция, вызываемая только из `NativeDisplay_rl.cpp`:

```cpp
Texture2D rl_bitmap_to_texture(BITMAP *bmp);  // CPU-bitmap → GPU текстура
void rl_texture_to_bitmap(Texture2D tex, BITMAP *dst); // GPU → CPU (для getpixel на canvas)
```

**Проверка:** `create_bitmap` / `draw_sprite` / `blit` / `draw_trans_sprite` работают корректно в unit-тесте без окна raylib (pure CPU, нет вызовов raylib).

---

### Этап 5. Слой совместимости — шрифты и текст

#### 5.1. Загрузка шрифта

Формат: `.txt`-файл с метаданными (ширина глифов) + `.pcx`-файл с атласом.

```cpp
FONT *load_txt_font(const char *filename, void *param1, void *param2);
```

Алгоритм:
1. Прочитать `font01.txt` — описание символов (начало, конец диапазона, ширина).
2. Загрузить `font01.pcx` как `Image` через `LoadImage`.
3. Нарезать глифы, сохранить в `FONT::glyphs[]`.
4. Загрузить `Image` как `FONT::atlas` через `LoadTextureFromImage`.

Альтернатива: конвертировать шрифт один раз в `.png`-атлас + JSON офсеты — тогда не нужен парсер `.pcx`.

#### 5.2. Вывод текста

```cpp
// Аналоги allegro textprintf*
void textprintf_ex(BITMAP *bmp, FONT *fnt, int x, int y,
                   int color, int bg, const char *fmt, ...);
void textprintf_centre_ex(BITMAP *bmp, FONT *fnt, int x, int y,
                          int color, int bg, const char *fmt, ...);
int  text_length(FONT *fnt, const char *str);
int  text_height(FONT *fnt);
```

Реализация: форматировать в `char buf[]`, затем нарисовать глиф за глифом из атласа прямо в `BITMAP::data`.

**Важно:** строки в CP1251 — индекс глифа = байт. Таблица символов в шрифте совпадает с CP1251.

**Проверка:** текст рисуется корректно на CPU-bitmap, кириллица отображается.

---

### Этап 6. Загрузка Allegro `.dat`-файлов

`.dat` — собственный контейнер Allegro. Содержит объекты типов `BMP`, `PCX`, `RLE_SPRITE`, `SAMPLE`, `MIDI`, `FLIC`, произвольные (`USER`). Формат:

- Заголовок: магия `ALLEG_D`, версия.
- Список объектов: тип (4 байта), имя (8 байт), свойства, размер, данные.
- Данные могут быть сжаты LZSS.

Нужно реализовать `load_datafile(const char *filename)` без Allegro.

#### 6.1. Парсер `.dat`

```cpp
DATAFILE *load_datafile(const char *filename);
DATAFILE *load_datafile_object(const char *filename, const char *objectname);
void      unload_datafile(DATAFILE *dat);
```

Источники для реализации:
- `allegro4/src/datafile.c` — читать, не линковать.
- `allegro4/include/allegro/datafile.h` — структуры.

Особенности:
- Объекты типа `BMP`/`PCX` надо декодировать в `BITMAP*` (свой PCX-декодер ~100 строк, или взять из `allegro4/src/pcx.c`).
- `COLOR_0 = makecol(255,0,255)` — magenta становится прозрачностью: при загрузке PCX/BMP устанавливать alpha = 0 для этого цвета.
- Объекты типа `USER` (Ogg, сырые данные) — возвращать `dat`/`size` как есть.
- LZSS-распаковка: взять `allegro4/src/lzss.c` и `allegro4/src/pakfile.c` (они portable C, без Allegro-зависимостей).

#### 6.2. Шрифты из `.dat`

Игра загружает шрифты отдельно через `load_txt_font`, а не из `.dat`. Так что `.dat`-парсер шрифты можно не поддерживать.

**Проверка:** `load_datafile("dat/Interface.dat")` возвращает корректный массив, первые несколько `BITMAP*` содержат правильные размеры.

---

### Этап 7. `NativeDisplay_rl.cpp` — окно и главный цикл

Заменяет `NativeDisplay.cpp` (X11/Xinerama → raylib Window).

#### 7.1. Инициализация

```cpp
int eador_set_gfx_mode(int card, int w, int h, int vw, int vh) {
    if (card == GFX_TEXT) { CloseWindow(); return 0; }
    // 1024×768 логический canvas
    canvas = create_bitmap(1024, 768);
    // окно в нужном режиме
    if (card == GFX_AUTODETECT_FULLSCREEN)
        SetConfigFlags(FLAG_FULLSCREEN_MODE);
    else
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 768, "Эадор. Сотворение");
    SetTargetFPS(0);  // игра сама управляет скоростью через Timer
    // viewport как в NativeViewport.hpp
    viewport = NativeViewport::fit(GetScreenWidth(), GetScreenHeight());
    return 0;
}
```

#### 7.2. `present()` — вывод canvas на экран

Вызывается из `eador_blit` (когда destination == canvas) и из `eador_readkey`.

```cpp
void present() {
    // CPU-bitmap → Texture2D → DrawTexturePro с letterbox
    Texture2D tex = rl_bitmap_to_texture(canvas);
    BeginDrawing();
    ClearBackground(BLACK);
    Rectangle src = {0, 0, 1024, 768};
    Rectangle dst = {(float)viewport.x, (float)viewport.y,
                     (float)viewport.w, (float)viewport.h};
    DrawTexturePro(tex, src, dst, {0,0}, 0, WHITE);
    EndDrawing();
    UnloadTexture(tex);
}
```

Вместо создания/уничтожения текстуры каждый кадр — держать `RenderTexture2D canvas_rt` и обновлять через `UpdateTexture`. Выбрать по результатам профилирования.

#### 7.3. Главный цикл и таймеры

Allegro регистрирует три прерывания:

| Callback | Интервал | Что делает |
|---|---|---|
| `ItIsTime` | 20 мс (50 Гц) | Опрос клавиатуры, затемнение, blit экрана, poll звука |
| `AnimHourglass` | 200 мс | Анимация песочных часов, переключение музыки |
| `MouseTime` | 5 мс | Опрос мыши (определяется в `Interface.cpp`) |

В raylib нет прерываний. Перенести вызовы в начало `present()`:

```cpp
static double lastItIsTime = 0, lastAnimHourglass = 0, lastMouseTime = 0;
double now = GetTime();
if (now - lastItIsTime  >= 0.020) { ItIsTime();      lastItIsTime  = now; }
if (now - lastAnimHourglass >= 0.200) { AnimHourglass(); lastAnimHourglass = now; }
if (now - lastMouseTime >= 0.005) { MouseTime();    lastMouseTime  = now; }
```

`present()` вызывается из `eador_blit` — т.е. из основного игрового цикла. Если на каком-то экране долго нет blit, таймеры не тикают. Добавить вызов `present()` из `eador_readkey` и из `rest()`.

#### 7.4. Ввод

```cpp
void eador_update_input() {
    // Вызывать в начале present() или по запросу poll_keyboard/poll_mouse
    for (int i = 0; i < 256; i++)
        key[i] = IsKeyDown(rl_scancode_to_key(i)) ? 1 : 0;
    // mouse
    Vector2 pos = GetMousePosition();
    _raw_mouse_x = (int)pos.x; _raw_mouse_y = (int)pos.y;
    mouse_b  = (IsMouseButtonDown(0) ? 1 : 0) | (IsMouseButtonDown(1) ? 2 : 0);
    mouse_z += (int)GetMouseWheelMove();  // аккумулятор
}

int eador_mouse_x() { return canvas ? viewport.mouseX(_raw_mouse_x) : _raw_mouse_x; }
int eador_mouse_y() { return canvas ? viewport.mouseY(_raw_mouse_y) : _raw_mouse_y; }

int eador_readkey() {
    present();  // показать то, что нарисовано
    while (!WindowShouldClose()) {
        eador_update_input();
        int k = GetCharPressed();
        if (k) return k;  // scancode в старшем байте не нужен игре (alogg)
        // опрос таймеров
        present();
    }
    return 0;
}
```

Таблица `rl_scancode_to_key[]`: `KEY_A`..`KEY_Z`, `KEY_0`..`KEY_9`, `KEY_ESC`, `KEY_ENTER`, `KEY_SPACE`, `KEY_F1`..`KEY_F12`, стрелки, `KEY_LCONTROL`, `KEY_LSHIFT` и т.д. — около 60 кодов, которые реально проверяет игра.

`ureadkey(&scan)` — используется в `Interface.cpp` для ввода имени. Вернуть `GetCharPressed()`, скан = 0.

#### 7.5. Скриншоты

```cpp
// Игра: if (key[KEY_F1]) { bScrShot = true; }
// В G_Object.cpp: TakeScreenShot("screens/scr%03d.pcx")
// Заменить на:
void SaveBitmapPcx(BITMAP *bmp, const char *filename);
// Реализация: конвертировать bmp->data в Image raylib → ExportImage
```

**Проверка:** окно открывается, logo.png рисуется, мышь и клавиатура работают. Игра доходит до заставки.

#### 7.6. Как сделано (статус: готово, кроме скриншотов — они ждут `save_bmp` из этапа 9)

- `raylib.h` и `allegro_rl.h` нельзя включать в один файл: макросы `KEY_*` Allegro ломают enum `KeyboardKey`. Поэтому raylib изолирован в `rl_backend.cpp`/`rl_backend.h` (окно, загрузка кадра, ввод в сканкодах Allegro), а `NativeDisplay_rl.cpp` работает только через этот интерфейс.
- Таймеры: `install_int` регистрирует колбэки в таблице, `tick_timers()` вызывает их по интервалам (учитываются `iRefreshDelay`/`iMouseDelay` из конфига). Защита от рекурсии: колбэки сами делают blit на экран.
- Игра крутит busy-wait циклы (`if (Timer)`, `while (key[KEY_ESC])`, опрос `mouse_x` в меню). Поэтому `rl_pump()` (события + таймеры + кадр не чаще ~60 FPS) вызывается из `poll_keyboard`, `poll_mouse`, `keypressed`, `rest`, `readkey`, `ureadkey` и из `eador_mouse_x/y`.
- Канвас `0x00RRGGBB` перед `UpdateTexture` конвертируется в RGBA8. ESC не закрывает окно (`SetExitKey(0)`). Заголовок окна переводится из CP1251 в UTF-8.
- `ureadkey` возвращает Unicode-код (`GetCharPressed`), как ждёт `UI_Input::Check` (кириллица > 1000).
- Отладка: `EADOR_RL_DUMP=/путь/кадр.png EADOR_RL_DUMP_AT=секунды` сохраняет один кадр canvas.
- Попутно в `dat_loader.cpp` поддержаны 8bpp-объекты (есть в `Site.dat`): они раскрываются через `default_palette` Allegro, как это делает `blit_from_256`.

---

### Этап 8. Звук — `alogg_impl_rl.cpp`

Заменить `alogg_impl.cpp`. Оставить тот же публичный API (`alogg_create_ogg_from_buffer`, `alogg_play_ex_ogg`, `alogg_poll_ogg` и т.д.), но реализовать через raylib `AudioStream`.

#### 8.1. ALOGG_OGG поверх AudioStream raylib

```cpp
struct ALOGG_OGG {
    // libvorbisfile (как сейчас)
    OggVorbis_File vf;
    alogg_membuf   mem;
    unsigned char *data;
    long           data_len;

    // raylib
    AudioStream    stream;   // InitAudioStream(freq, 16, channels)
    int            freq, stereo, volume, pan, speed, loop;
    int            playing, finished;
    ogg_int64_t    total_samples;
    int            sample_len;
};
```

Алгоритм `alogg_poll_ogg`:

```cpp
if (!IsAudioStreamProcessed(ogg->stream)) return ALOGG_OK;
// декодировать следующий буфер libvorbisfile
// XOR ^ 0x8000 как сейчас (Allegro unsigned PCM → убрать при переходе на raylib,
// raylib ожидает signed PCM)
UpdateAudioStream(ogg->stream, buf, frames);
```

Инициализировать raylib audio:

```cpp
// В eador_set_gfx_mode или в allegro_init:
InitAudioDevice();
// Для каждого ALOGG_OGG::stream:
ogg->stream = InitAudioStream(ogg->freq, 16, ogg->stereo ? 2 : 1);
PlayAudioStream(ogg->stream);
```

Громкость/панорама: `SetAudioStreamVolume(stream, vol/255.0f)`. Панорама в raylib не поддерживается нативно — игнорировать (pan=128 — центр, все вызовы с pan=128).

#### 8.2. `play_audio_stream` / `get_audio_stream_buffer` / `free_audio_stream_buffer`

Эти функции Allegro используются только внутри `alogg_impl.cpp`. После замены на `alogg_impl_rl.cpp` — не нужны. Оставить пустые заглушки в `allegro_rl.h`.

#### 8.3. Инициализация звука из `Eador.cpp`

```cpp
install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, Garb);
// → InitAudioDevice() (уже вызван в eador_set_gfx_mode)
// Заглушка: return 0
```

**Проверка:** музыка играет на главном экране (треки из `dat/music.dat`).

#### 8.4. Как сделано (статус: готово)

- `alogg_impl_rl.cpp` повторяет логику `alogg_impl.cpp`, но пишет PCM в `AudioStream`. Размер подбуфера равен `buffer_len` (`iOggBuffer` из конфига) через `SetAudioStreamBufferSizeDefault`: `UpdateAudioStream` не принимает больше кадров, чем помещается в подбуфер.
- `^ 0x8000` убран: raylib принимает signed 16-bit PCM в том виде, в каком его отдаёт `ov_read`.
- Громкость `vol/255`, панорама `(pan-128)/128`, скорость `speed/1000` переходят в pitch.
- `raylib.h` подключается до `alogg.h`: тот включает `allegro_rl.h` с макросами `KEY_*`.
- Проверено записью с монитора звуковой карты (`parec`): до запуска игры тишина, на главном меню есть сигнал.

---

### Этап 9. Сохранения — `PACKFILE` и PCX карты

Игра использует Allegro `PACKFILE` для сохранения/загрузки (`save/*.sav`, `var/*.dat`).

#### 9.1. PACKFILE

Allegro PACKFILE — LZSS-сжатый файл. Игра пишет/читает сырые C-структуры через `pack_fwrite`/`pack_fread`.

Вариант А (рекомендуется): **порт `pakfile.c` из Allegro 4** — взять `allegro4/src/pakfile.c` и `allegro4/src/lzss.c`, скомпилировать с `allegro_rl.cpp`, убрав зависимости от остальных модулей Allegro (~500 строк портируемого C).

Вариант Б: заменить PACKFILE на `fopen`/`fwrite` + zlib deflate. Сохранения получат другой формат — сломает совместимость с существующими сохранениями.

**Выбрать Вариант А.** Он сохраняет обратную совместимость.

#### 9.2. PCX в сохранениях карты

`save_pcx_pf` / `load_pcx_pf` — сохранение/загрузка `BITMAP` в PCX прямо в PACKFILE.

Реализация: PCX формат простой (RLE, 1 плоскость, 24bpp для 32-bit bitmap). Написать `save_pcx_pf` и `load_pcx_pf` (~150 строк).

Альтернатива: конвертировать карту в PNG (ExportImage raylib) — но сломает старые сохранения.

**Проверка:** сохранение и загрузка игры работают, карта восстанавливается.

#### 9.3. Как сделано (статус: готово)

- `God/pakfile_rl.cpp`: порт `file.c` (упакованные файлы `slh!`/`slh.`), `lzss.c` (сжатие и распаковка), `pcx.c` (`load_pcx_pf`/`save_pcx_pf`, 8 и 24 бита) и `save_bmp` из `bmp.c` для скриншотов по F1. Файл читается целиком при открытии и пишется целиком при закрытии: игра работает с сохранениями только последовательно.
- `get_palette` возвращает `default_palette` Allegro (таблица `rl_default_palette` в `dat_loader.cpp`).
- `load_datafile` использует ту же LZSS-распаковку.
- Проверено на существующих сохранениях из `out/eador-linux/save` (`game0.sav`, `game0.bkc`, `game0.cmp`, `map0.map`, `map0.cmp`, `player0.prf`): распаковка и повторное сжатие дают файлы, байт в байт совпадающие с оригиналами Allegro. PCX карты из `map0.map` (1954×1340, 24 бита) после чтения и записи тоже совпадает байт в байт.

---

### Этап 10. `allegro_message` и мелкие функции

Оставшиеся Allegro-функции, используемые в игре:

```cpp
void allegro_message(const char *fmt, ...) {
    // Показать текстовое сообщение в окне raylib, дождаться нажатия клавиши
    // Используется только при ошибках (Randomizer overflow, отсутствие GFX)
}

int  allegro_init()         { InitAudioDevice(); return 0; }
void allegro_exit()         { CloseAudioDevice(); CloseWindow(); }

// Blender (используется для draw_trans_sprite / draw_lit_sprite)
void set_trans_blender(int r, int g, int b, int a);
void set_add_blender(int r, int g, int b, int a);
void set_alpha_blender();

// Курсор мыши
void show_mouse(BITMAP *bmp); // bmp=NULL → HideCursor(), иначе программный курсор
void scare_mouse();
void unscare_mouse();

// Клавиатура
void set_keyboard_rate(int delay, int repeat);
void set_uformat(int fmt);  // заглушка
int  keypressed()  { return GetKeyPressed() != 0; }
void clear_keybuf()         { while (GetKeyPressed()); }
int  poll_keyboard()        { eador_update_input(); return 0; }
int  poll_mouse()           { eador_update_input(); return 0; }

// get/set_config — загрузка Eador.cfg
const char *get_config_string(const char *section, const char *key, const char *def);
int         get_config_int(const char *section, const char *key, int def);
// реализовать простой INI-парсер (~80 строк)

// Прочее
void rest(int ms) { WaitTime(ms / 1000.0); }
int  get_desktop_resolution(int *w, int *h) {
    *w = GetScreenWidth(); *h = GetScreenHeight(); return 0;
}
void set_display_switch_mode(int mode) {}  // заглушка
void set_window_title(const char *title) { SetWindowTitle(title); }
void vsync() { /* EndDrawing вызывается из present() */ }

// PALETTE (только для PCX save/load карты — передаётся в функции, но игнорируется)
void get_palette(PALETTE p)  {}  // заглушка
void set_palette(const PALETTE p) {}  // заглушка

// Timer / keyboard install
int  install_timer()   { return 0; }
int  install_mouse()   { return 0; }
int  install_keyboard(){ return 0; }
int  install_sound(int digi, int midi, char *cfg) { return 0; }
void install_int(void (*fn)(), int ms) {}  // таймеры перенесены в present()
void remove_timer() {}
#define LOCK_FUNCTION(f)
#define LOCK_VARIABLE(v)
#define END_OF_FUNCTION(f)
```

**`set_color_depth()`**: игра пробует разные глубины цвета. При raylib — всегда 32bpp, заглушка.

#### 10.1. Как сделано (статус: готово)

- `al_findfirst`/`al_findnext`/`al_findclose`: порт `allegro4/src/unix/ufile.c` (маски `*` и `?`, `*.*` → `*`, путь без маски проверяется через `stat`). В `al_ffblk` добавлено поле `ff_data`, `time` и `size` теперь `long`.
- `replace_extension`: порт из `file.c`, возвращает `char *`, как в Allegro.
- `allegro_message` печатает в stderr, как Allegro на Unix без GUI.
- `get_config_*` возвращают значение по умолчанию: игра читает `eador.cfg` сама (`LoadConfig`).
- `load_sample`/`play_sample` остались заглушками: в игре они встречаются только в закомментированном коде, все звуки идут через alogg.

---

### Этап 11. Программный курсор мыши

Игра рисует курсор сама через `bmpMousePointer[]` и вызывает `show_mouse(screen)` / `show_mouse(NULL)`.

```cpp
// В NativeDisplay_rl.cpp:
static BITMAP *g_cursor_bmp = nullptr;

void show_mouse(BITMAP *bmp) {
    if (bmp == nullptr) { HideCursor(); g_cursor_bmp = nullptr; }
    else { HideCursor(); g_cursor_bmp = bmp; }  // рисовать в present()
}

// В present(), перед EndDrawing:
if (g_cursor_bmp) {
    // нарисовать g_cursor_bmp в позиции (eador_mouse_x(), eador_mouse_y())
    rl_draw_bitmap_at(g_cursor_bmp, eador_mouse_x(), eador_mouse_y());
}
```

---

#### 11.1. Как сделано (статус: готово)

План выше не понадобился. Игра сама рисует `mouse_sprite` в `screen1` (`G_Object.cpp`, `draw_sprite(screen1, mouse_sprite, mouse_x, mouse_y)`), а `show_mouse` есть только в закомментированном коде. Поэтому системный курсор скрывается (`HideCursor` в `rl_backend.cpp`), а `show_mouse` ничего не делает.

---

### Этап 12. Финальная сборка и проверки

#### 12.0. Результаты (2026-11-11)

| Проверка | Результат |
|---|---|
| Сборка `make raylib` | собирается без ошибок и предупреждений |
| Старая сборка `out/build` (Allegro) | собирается, ветка `USE_RAYLIB=OFF` не сломана |
| Внешние символы `libeador_core.a` + `libalogg_impl.a` | только libc/libstdc++/libvorbis, символов Allegro нет; `ldd Eador` не показывает liballeg и Xinerama (libX11 нужен GLFW) |
| `EadorUnitTests` | 9 из 12; 3 падающих теста в `OptionalTests.cpp` падают так же в Allegro-сборке, к миграции не относятся |
| 1. Заставка и главное меню | отрисовываются (проверено дампом кадра `EADOR_RL_DUMP`) |
| 6. Музыка | играет в меню (проверено записью `parec`) |
| 7. Формат сохранений | LZSS и PCX совпадают байт в байт с файлами Allegro |
| 2–5, 8–10 | требуют ручной проверки: в окружении нет средств эмуляции ввода (xdotool) |

12.1. Убедиться, что `eador_core` линкуется без символов Allegro 4:

```sh
nm out/build-rl/God/libeador_core.a | grep ' U ' | grep -v '__'
```

12.2. Прогнать `EadorUnitTests` (не используют графику, должны работать без изменений).

12.3. Функциональные проверки по порядку:

| № | Проверка |
|---|---|
| 1 | Окно открывается, заставка отрисовывается |
| 2 | Главное меню кликается мышью |
| 3 | Новая игра → карта генерируется, прогресс-бар работает |
| 4 | Провинции рисуются, герой двигается по карте |
| 5 | Бой запускается и завершается |
| 6 | Звуки и музыка воспроизводятся |
| 7 | Сохранение/загрузка игры |
| 8 | Скриншот по F1 сохраняется в `screens/` |
| 9 | Полноэкранный режим, letterbox 4:3 |
| 10 | Выход из игры через меню |

---

## Итоговая структура новых файлов

```
God/
├── allegro_rl.h          // замена allegro.h: все типы и API Allegro 4
├── allegro_rl.cpp        // реализация (графические примитивы CPU)
├── dat_loader.cpp        // load_datafile / unload_datafile / PCX-декодер
├── dat_loader.h
├── pakfile_rl.cpp        // PACKFILE, LZSS, PCX, save_bmp: порт file.c, lzss.c, pcx.c, bmp.c
├── pakfile_rl.h
├── NativeDisplay_rl.cpp  // замена NativeDisplay.cpp: present(), таймеры, очередь клавиш
├── rl_backend.cpp        // единственное место с raylib.h: окно, текстура, ввод
├── rl_backend.h
├── alogg_impl_rl.cpp     // замена alogg_impl.cpp: AudioStream raylib
└── Globals.h             // без изменений — добавить #ifdef USE_RAYLIB
```

Изменения в существующих файлах:

```
God/Globals.h             // строка 19: #ifdef USE_RAYLIB → #include "allegro_rl.h"
                          //            #else             → #include <allegro.h>
God/CMakeLists.txt        // добавить ветку USE_RAYLIB
CMakeLists.txt            // добавить USE_RAYLIB + add_subdirectory(raylib)
Makefile                  // добавить цель raylib
```

Код игры (`Audio.cpp`, `Battlefield.cpp`, ...) — **не менять**.

---

## Зависимости и предположения

- libvorbisfile не нужен: `alogg_impl_rl.cpp` проигрывает Ogg через `Music` raylib (`LoadMusicStreamFromMemory`, внутри stb_vorbis). Размер подбуфера берётся из `iOggBuffer` через `SetAudioStreamBufferSizeDefault` перед загрузкой.
- Allegro `.dat` содержит BMP/PCX объекты с флагом цветности. При глубине 8bpp в `.dat` — конвертировать в 32bpp при загрузке.
- Игра хранит в `int` значения цветов (makecol) — при 32bpp это `0x00RRGGBB` (без alpha). Альфа = 255, если не задана.
