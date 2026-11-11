# План: сборка Eador. Genesis под Android

Документ для отдельного контекста. Всё, что помечено «проверено», уже проверено
на этой машине; остальное — решения, которые нужно реализовать.

## Исходное состояние

Репозиторий `/mnt/ssd2/projects/temp/eador-la`:

- `God/` — игра (C++20, Allegro 4.4). Исходники в UTF-8, на Linux строки
  компилируются в CP1251 флагом `-fexec-charset=CP1251` (`God/CMakeLists.txt`).
  Код игры собирается в статическую `eador_core` с `NO_EADOR_MAIN`, точка входа
  `int EadorMain(int argc, char **argv)` (`God/EadorMain.cpp`).
- `God/NativeDisplay.cpp/.h` — логический экран 1024×768 поверх Allegro
  (`canvas`, `present()`, пересчёт координат мыши через `NativeViewport.hpp`).
  Использует X11, Xinerama, `_xwin` и iconv — на Android не собирается.
- `God/alogg_impl.cpp` — Ogg Vorbis через libvorbisfile
  (`ov_open_callbacks`, `ov_read`, `ov_pcm_seek`, `ov_pcm_tell`, `ov_pcm_total`,
  `ov_info`, `ov_clear`) поверх `play_audio_stream` Allegro.
- `allegro4/` — субмодуль Allegro 4.4 (liballeg/allegro4). **Не править.**
- `innounp/` — свой распаковщик установщиков GOG (Inno Setup 5.5.7, LZMA1/LZMA2,
  zlib, SHA-1). API: `innounp::extract(exe_path, out_dir, progress)`
  (`innounp/innounp.h`). Сверен побайтно с innoextract на обоих установщиках
  (проверено). Субмодуль innoextract удалён.
- `cmake/import-gog.sh` — логика импорта для Linux (образец для Android).
- `God/Eador.cfg` — шаблон конфига (`Windowed: 0` — полноэкранный).

Окружение (проверено):

- JDK 17 (`/usr/lib/jvm/java-17-openjdk`) и 25; системный Gradle 9.8.1.
- В кэше Gradle: AGP 7.3.1, 8.9.1, 8.11.1, 8.13.2, 9.4.1; дистрибутивы Gradle
  7.4, 8.13, 9.4.1 (`~/.gradle/wrapper/dists`).
- Android SDK `/opt/android-sdk`: platforms 30/33/35/36, build-tools 30.0.3,
  35.0.0, 35.0.1, 36.1.0, cmake 3.22.1 и 3.30.3.
- NDK 29.0.14206865 (clang 21): есть `libaaudio.so`, `libandroid.so`,
  `liblog.so`, `libz.so`, `sources/android/native_app_glue`.
- Установщики GOG: `/mnt/ssd2/projects/temp/eador/installer/setup_eador_genesis_1.05.02_language_update_(21780).exe`
  (en) и `..._(russian)_(21780).exe` (ru).

## Требования пользователя

- Бэкенд: свой Android-драйвер для Allegro 4 (образец — порт PSP, `allegro4/src/psp/`).
- Минимально возможные JDK и Gradle.
- Все четыре ABI: arm64-v8a, armeabi-v7a, x86, x86_64.
- APK только с кодом. Если данных игры нет — экран с двумя кнопками на языке
  системы (ru/en, остальные языки — en), сверху вниз:
  - «Import game files from GOG_EXE file» / «Импортировать файлы игры из файла GOG_EXE»;
  - «Import game files from GOG_PATH folder» / «Импортировать файлы игры из папки GOG_PATH».
- Если данные есть — кнопки не показываются, игра запускается сразу.
- Иконка по умолчанию, имя файла `eador_reforged.apk`.

## Решения

| Вопрос | Решение | Почему |
|---|---|---|
| JDK | 17 | Минимум для AGP 8.x, установлен |
| Gradle / AGP | Gradle wrapper 8.13 + AGP 8.13.2 | Минимальная пара для NDK 29 и compileSdk 35, оба в кэше (сборка без сети) |
| Язык UI | Java, без Kotlin и androidx | Не нужен плагин Kotlin и внешние зависимости |
| minSdk | 26 | AAudio доступен с API 26; без фолбэка на OpenSL ES |
| compileSdk / targetSdk | 35 | Есть platform 35 и build-tools 35.0.0 |
| Платформа Allegro | `ALLEGRO_UNIX` без X11, свои `alplatf.h`/`alunixac.h` | Штатный путь `alconfig.h` → `alucfg.h` → `aintunix.h`, субмодуль не меняется |
| CP1251 | Перекодировать `God/*` и `include/*` в каталог сборки | NDK clang не знает `-fexec-charset=CP1251` (проверено) |
| Потоки | Игра в отдельном `std::thread`, поток `android_main` обрабатывает события | Игра — блокирующий цикл; ввод и окно должны обслуживаться независимо |
| Данные | `getExternalFilesDir(null)` | Можно положить данные вручную через `adb push` |

Альтернатива для «ещё меньше Gradle»: AGP 7.3.1 + Gradle 7.4 (тоже в кэше), но
AGP 7.3 официально не поддерживает compileSdk 35 и новые NDK. Не использовать
без явного запроса.

## Проверенные факты, на которые опирается план

1. NDK clang: `-fexec-charset=CP1251` → `error: invalid value 'CP1251'`.
2. Файл, перекодированный `iconv -f UTF-8 -t CP1251`, компилируется с
   `-Wno-invalid-source-encoding`, байты литералов сохраняются:
   «Сохранение» → `d1 ee f5 f0 e0 ed e5 ed e8 e5` в `.rodata`.
3. Все `God/*.cpp`, `God/*.h`, `God/*.hpp` перекодируются в CP1251 без ошибок.
4. Не-ASCII есть в 29 файлах `God/*.cpp` **и в заголовках**: `God/Battlefield.h`,
   `Globals.h`, `Interface.h`, `Mapper.h`, `Prototype.h`, `Unit.h`, `Windower.h`,
   `include/alogg.h`, `include/aloggdll.h`. Перекодировать нужно и заголовки,
   а include-пути направить на перекодированные копии.
5. Раскладка распакованного установщика (`innounp`): `dat/`, `var/`, шрифты
   `font0{1,2}.{pcx,txt}`, `goggame-1207659147.ico` лежат **в корне**.
   Тексты: `Text/` в английском, `text/` в русском. В `app/` только иконка,
   `webcache.zip` и пустые каталоги. Игра ищет `text/` в нижнем регистре.
6. В `allegro4/src/unix/` системно-зависимы (не собирать): `alsa5.c`, `alsa9.c`,
   `alsamidi.c`, `arts.c`, `jack.c`, `sgial.c`, `uesd.c`, `uoss.c`,
   `uossmidi.c`, `umodules.c`, `usigalrm.c`, `ustimer.c`. `uptimer.c`
   включает `sys/io.h` только под `ALLEGRO_LINUX_VGA` — безопасен.
7. `usystem.c` определяет `_system_driver_list` (конфликт с нашим списком)
   и `_unix_bg_man`, `_unix_find_resource`, `_unix_read_os_type`,
   `_unix_yield_timeslice`, `_unix_get_executable_name`, `_unix_get_page_size`.
   Его не собирать, нужные функции реализовать у себя.
8. `utimer.c` содержит пустой `_timer_driver_list` под `#ifndef ALLEGRO_MACOSX`
   — конфликт с нашим списком таймеров. Либо не собирать `utimer.c` и
   реализовать `_unix_rest` у себя (рекомендуется, ~30 строк), либо не объявлять
   свой `_timer_driver_list` и отдавать таймер через `system_driver->timer_drivers`.
9. В `God/` нет других X11/iconv-зависимостей, кроме `NativeDisplay.cpp`.
   `Yuplay.cpp` под `#ifdef YUPLAY` — пустой. `GlobalCrutches.cpp` тянет
   `cpp-httplib` (`RemoteHost.hpp`) — header-only, должен собраться под Android;
   проверить на шаге 6.
10. `CurrentGameState.cpp` использует `<format>` — в libc++ NDK 29 есть.

## Структура новых файлов

```text
android/
├── settings.gradle.kts
├── build.gradle.kts                # плагин AGP 8.13.2 (apply false)
├── gradle.properties               # android.useAndroidX=false
├── gradle/wrapper/                 # wrapper 8.13 (gradle wrapper --gradle-version 8.13)
├── gradlew, gradlew.bat
└── app/
    ├── build.gradle.kts
    └── src/main/
        ├── AndroidManifest.xml
        ├── assets/eador.cfg              # копия God/Eador.cfg (задача Gradle)
        ├── java/com/eador/reforged/
        │   ├── LauncherActivity.java     # проверка данных, кнопки импорта
        │   └── GameActivity.java         # extends android.app.NativeActivity
        ├── res/values/strings.xml        # en
        ├── res/values-ru/strings.xml     # ru
        ├── res/layout/activity_launcher.xml
        └── cpp/
            ├── CMakeLists.txt            # верхний CMake
            ├── allegro/                  # конфиги платформы для Allegro
            │   ├── allegro/platform/alplatf.h
            │   └── allegro/platform/alunixac.h
            ├── adriver/                  # Android-драйверы Allegro 4
            │   ├── aandroid.h            # ID драйверов, общий API с main
            │   ├── adrvlist.c
            │   ├── asystem.c
            │   ├── agfx.c
            │   ├── akey.c
            │   ├── amouse.c
            │   └── asound.c
            ├── vorbis.cmake              # libogg + libvorbis(file) из исходников
            ├── cp1251.cmake              # перекодировка God/ и include/
            ├── android_main.cpp          # native_app_glue, поток игры, очереди ввода
            └── jni_import.cpp            # JNI для LauncherActivity (innounp, копирование)
God/NativeDisplayAndroid.cpp              # замена NativeDisplay.cpp без X11
```

## Этапы

### 1. Gradle-скелет

- `gradle wrapper --gradle-version 8.13` в `android/` (дистрибутив в кэше).
- `app/build.gradle.kts`: `namespace = "com.eador.reforged"`, `compileSdk = 35`,
  `minSdk = 26`, `targetSdk = 35`, `ndkVersion = "29.0.14206865"`,
  `abiFilters` — четыре ABI, `externalNativeBuild.cmake.path = "src/main/cpp/CMakeLists.txt"`,
  `version = "3.22.1"`, аргументы `-DANDROID_STL=c++_static`.
- Имя APK: в `androidComponents.onVariants` или через
  `applicationVariants.all { outputs.all { outputFileName = "eador_reforged.apk" } }`
  (в AGP 8 работает через `BaseVariantOutputImpl`). Для release без ключа
  подписывать debug-ключом (`signingConfig = signingConfigs["debug"]`), иначе
  APK не установится.
- Задача, копирующая `../God/Eador.cfg` в `assets/eador.cfg` перед `preBuild`.
- Проверка: `./gradlew assembleRelease` с пустым `android_main` даёт APK.

### 2. Vorbis

- Исходники libogg 1.3.5 и libvorbis 1.3.7 (xiph). Скачать один раз в
  `android/app/src/main/cpp/third_party/` скриптом или `FetchContent`
  (URL: `https://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz`,
  `https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.gz`).
  Проверить доступность сети; при `FetchContent` — `URL_HASH SHA256=...`.
- libogg требует сгенерированный `ogg/config_types.h` — написать вручную
  (`int16_t`, `int32_t`, `int64_t`, `uint*` из `<stdint.h>`).
- Статические `ogg`, `vorbis` (lib/*.c без `psytune.c`, `tone.c`, `barkmel.c`),
  `vorbisfile`.

### 3. Allegro 4 под Android

Источники для `add_library(allegro STATIC ...)`:

- `ALLEGRO_SRC_FILES` и `ALLEGRO_SRC_C_FILES` — взять списки из
  `allegro4/cmake/FileList.cmake` через `include()`; пути относительны корню
  субмодуля.
- Из `src/unix/`: `uthreads.c`, `uptimer.c`, `ufile.c`, `udrvlist.c`.
  Не брать `usystem.c`, `utimer.c`, `umain.c` и всё из факта 6.
- `adriver/*.c`.

Определения компиляции: `ALLEGRO_SRC`, `ALLEGRO_LIB_BUILD`, `ALLEGRO_STATICLINK`,
`ALLEGRO_NO_ASM`. Include: сначала `cpp/allegro/` (наши сгенерированные
заголовки), затем `allegro4/include`.

`alplatf.h`:

```c
#define ALLEGRO_UNIX
#define ALLEGRO_NO_ASM
#define ALLEGRO_USE_C
```

`alunixac.h` (по `alunixac.h.cmake`): `ALLEGRO_COLOR8/16/24/32`,
`ALLEGRO_LITTLE_ENDIAN`, `ALLEGRO_HAVE_LIBPTHREAD`, `ALLEGRO_HAVE_DIRENT_H`,
`ALLEGRO_HAVE_INTTYPES_H`, `ALLEGRO_HAVE_STDINT_H`, `ALLEGRO_HAVE_SYS_STAT_H`,
`ALLEGRO_HAVE_SYS_TIME_H`, `ALLEGRO_HAVE_MEMCMP`, `ALLEGRO_HAVE_MKSTEMP`,
`ALLEGRO_HAVE_MMAP`, `ALLEGRO_HAVE_MPROTECT`, `ALLEGRO_HAVE_POSIX_MONOTONIC_CLOCK`,
`ALLEGRO_HAVE_SCHED_YIELD`, `ALLEGRO_HAVE_SYSCONF`, `#define RETSIGTYPE void`.
**Не** определять `ALLEGRO_WITH_XWINDOWS`, `ALLEGRO_WITH_MODULES`,
`ALLEGRO_LINUX`, `ALLEGRO_WITH_*DIGI`, `ALLEGRO_HAVE_SYS_IO_H`.

Драйверы (ID — через `AL_ID`, в `aandroid.h`):

- `adrvlist.c` (образец `src/psp/pdrivers.c`): `_system_driver_list`
  (`system_android`, затем `system_none`), `_keyboard_driver_list`,
  `_mouse_driver_list`, `_gfx_driver_list`, `_digi_driver_list`,
  `_timer_driver_list` (`TIMERDRV_UNIX_PTHREADS`, `&timerdrv_unix_pthreads`),
  `BEGIN_MIDI_DRIVER_LIST MIDI_DRIVER_DIGMID END_MIDI_DRIVER_LIST`,
  `BEGIN_JOYSTICK_DRIVER_LIST END_JOYSTICK_DRIVER_LIST`.
- `asystem.c` (образцы `src/psp/psystem.c`, `src/unix/usystem.c`):
  `SYSTEM_DRIVER system_android`. `init`: `_unix_bg_man = &_bg_man_pthreads`,
  `_unix_bg_man->init()`, `_unix_driver_lists_init()`, `os_type = OSTYPE_UNIX`.
  `exit`: обратное. `message` → `__android_log_print`. Мьютексы —
  `_unix_create_mutex` и др. из `uthreads.c`. `get_desktop_resolution` —
  размер `ANativeWindow`. `desktop_color_depth` → 32. `gfx_drivers` и
  `digi_drivers` → `_unix_gfx_driver_list`, `_unix_digi_driver_list`.
  Здесь же определить `_unix_bg_man`, `_unix_find_resource` (искать в рабочем
  каталоге), `_unix_read_os_type`, `_unix_yield_timeslice` (`sched_yield`),
  `_unix_get_executable_name` (`<datadir>/eador`), `_unix_get_page_size`
  (`sysconf(_SC_PAGESIZE)`), `_unix_rest` (`nanosleep`, с колбэком — цикл
  как в `utimer.c`), `__crt0_argc`, `__crt0_argv`.
- `agfx.c` (образец `src/psp/pgfx.c`): `GFX_DRIVER gfx_android`, `windowed = FALSE`.
  `init(w, h, …, depth)`: только 32 bpp; память `w*h*4`, `_make_bitmap`.
  Экспортировать `void android_gfx_present(BITMAP *src)`: `ANativeWindow_lock`,
  масштабирование 4:3 с чёрными полосами (ближайший сосед), перестановка
  каналов при необходимости (формат окна `WINDOW_FORMAT_RGBX_8888`,
  `ANativeWindow_setBuffersGeometry`), `unlockAndPost`. Если окна нет
  (свёрнуто) — пропустить кадр. Хранить `viewport` для пересчёта касаний.
  Проверить порядок каналов: Allegro 32 bpp по умолчанию `_rgb_r_shift_32 = 16`
  (BGRA в памяти), окну нужен RGBA — либо выставить сдвиги в `init`
  (`_rgb_r_shift_32 = 0; _rgb_g_shift_32 = 8; _rgb_b_shift_32 = 16`) до загрузки
  датафайлов, либо переставлять байты в `present`.
- `akey.c` (образец `src/psp/pkey.c`): `poll` читает очередь из `android_main`
  и вызывает `_handle_key_press(ascii, scancode)` / `_handle_key_release`.
  Back → `KEY_ESC`; аппаратная клавиатура: `AKEYCODE_*` → `KEY_*`
  (буквы, цифры, Enter, Space, стрелки, Ctrl, Shift, F1–F12).
- `amouse.c` (образец `src/psp/pmouse.c`): `timer_poll` читает очередь касаний:
  `_mouse_x/_mouse_y` в физических координатах окна (пересчёт в 1024×768 делает
  `NativeDisplayAndroid`), `_mouse_b`: касание = 1, долгое нажатие (≥500 мс без
  сдвига > 16 px) = 2, вертикальный свайп двумя пальцами → `_mouse_z ±1`.
  `init` возвращает 2 (кнопки).
- `asound.c` (образец `src/psp/psound.c`): AAudio, 44100 Гц, стерео,
  `AAUDIO_FORMAT_PCM_I16`, колбэк данных вызывает
  `_mix_some_samples((uintptr_t)buf, 0, TRUE)` порциями `SAMPLES_PER_BUFFER`;
  `_mixer_init(SAMPLES_PER_BUFFER * 2, 44100, TRUE, TRUE, &voices)`.
  Если AAudio не открылся — `detect` возвращает FALSE (игра идёт без звука).

Проверка: `liballegro.a` собирается для arm64-v8a без неразрешённых символов
при линковке с пустой программой, вызывающей `allegro_init()`.

### 4. CP1251

`cp1251.cmake`: для каждого файла из `God/` (`*.cpp`, `*.h`, `*.hpp`) и
`include/` (`alogg.h`, `aloggdll.h`, `cpp-httplib/…` не трогать) —
`add_custom_command(OUTPUT …/cp1251/God/<file> COMMAND iconv -f UTF-8 -t CP1251 …)`.
Include-пути игры: `…/cp1251/God`, `…/cp1251/include`, затем `include/`
(для `cpp-httplib`). Флаги: `-Wno-invalid-source-encoding`, без
`-fexec-charset`/`-finput-charset`. iconv есть на хосте (проверено).

### 5. NativeDisplayAndroid.cpp

Тот же интерфейс, что в `NativeDisplay.h` (`eador_screen`, `eador_set_gfx_mode`,
`eador_blit`, `eador_mouse_x/y`, `eador_position_mouse`, `eador_readkey`,
`eador_allegro_exit`, `eador_set_window_title`), с
`#define EADOR_NATIVE_DISPLAY_IMPLEMENTATION` как в оригинале:

- `eador_set_gfx_mode`: для `GFX_TEXT` — `set_gfx_mode` как есть; иначе
  `set_gfx_mode(card, 1024, 768, 0, 0)`, затем `canvas` = `screen`
  (драйвер уже даёт 1024×768) или отдельный memory bitmap.
- `present()` → `android_gfx_present(canvas)` под `recursive_mutex`.
- `eador_blit`: как в Linux-версии (blit + present при выводе на canvas).
- Координаты мыши — через `NativeViewport` из размеров окна.
- `eador_set_window_title` — no-op.

В CMake Android-сборки вместо `NativeDisplay.cpp` подставить этот файл.
`God/CMakeLists.txt` не менять; Android-CMake перечисляет исходники сам
(список из `God/CMakeLists.txt` минус `NativeDisplay.cpp`).

### 6. Нативная часть приложения

`libeador.so` = `eador_core`-исходники (CP1251) + `EadorMain.cpp`
(перекодировать тоже) + `NativeDisplayAndroid.cpp` + `alogg_impl.cpp` +
`android_main.cpp` + `jni_import.cpp` + `android_native_app_glue.c` +
`allegro`, `vorbisfile`, `vorbis`, `ogg`, `innounp`, `log`, `android`,
`aaudio`, `z`, `m`. Флаг `-u ANativeActivity_onCreate`.
`innounp` подключить через `add_subdirectory(../../../../innounp …)` с
`-DINNOUNP_TOOL=OFF`; `find_package(ZLIB)` в NDK находит `libz` (проверить,
иначе `target_link_libraries(innounp PUBLIC z)` через опцию).

`android_main.cpp`:

- `android_main(android_app *app)`: обрабатывает `APP_CMD_INIT_WINDOW` /
  `TERM_WINDOW` (передаёт `ANativeWindow*` в `agfx.c` под мьютексом),
  `onInputEvent` складывает касания и клавиши в очереди для `akey.c`/`amouse.c`.
- При первом `INIT_WINDOW`: `chdir(<externalFilesDir>)` (путь из
  `app->activity->externalDataPath`), создать `save/`, `screens/`, скопировать
  `eador.cfg` из assets (`AAssetManager`), если его нет, затем запустить
  `std::thread([]{ EadorMain(1, argv); })`.
- Когда `EadorMain` возвращается — `ANativeActivity_finish`.
- Back не отдавать системе (возвращать 1 из обработчика ввода).

### 7. Java-часть

`AndroidManifest.xml`: `LauncherActivity` — `MAIN/LAUNCHER`; `GameActivity`
(`android.app.NativeActivity`-наследник, `meta-data android.app.lib_name = eador`,
`screenOrientation="sensorLandscape"`, `configChanges` с
`orientation|screenSize|screenLayout|keyboardHidden|keyboard|navigation`,
тема `@android:style/Theme.NoTitleBar.Fullscreen`). `android:hasCode="true"`.
Иконка — `@android:drawable/sym_def_app_icon` или `android:icon` не задавать.

`LauncherActivity`:

- `onCreate`: `System.loadLibrary("eador")`; если `nativeHasGameData(dir)`
  (`dat/Interface.dat`, `var/`, `text/`, `font01.txt`) — `startActivity(GameActivity)`
  и `finish()`.
- Иначе — layout с двумя кнопками и `TextView` статуса/прогресса. Тексты из
  `strings.xml` (ru в `values-ru`).
- Кнопка 1: `ACTION_OPEN_DOCUMENT`, `type = "*/*"` (у .exe нет надёжного
  MIME). Результат: скопировать URI в `getCacheDir()/setup.exe` через
  `ContentResolver.openInputStream` в фоновом потоке, затем
  `nativeImportExe(cachePath, filesDir)`.
- Кнопка 2: `ACTION_OPEN_DOCUMENT_TREE`. Результат: рекурсивно скопировать
  через `DocumentsContract` (`buildChildDocumentsUriUsingTree`, query
  `COLUMN_DOCUMENT_ID`, `COLUMN_DISPLAY_NAME`, `COLUMN_MIME_TYPE`) только
  `dat/`, `text/`/`Text/`, `var/`, `font0{1,2}.{pcx,txt}`, `goggame-*.ico`
  (сравнение имён без учёта регистра) в `getCacheDir()/gog_game`, затем
  `nativeImportFolder(cacheDir, filesDir)`.
- Прогресс из JNI — через вызов Java-метода (`onProgress(long done, long total)`).
- После успешного импорта — удалить временные файлы и запустить `GameActivity`.
  Ошибку показать в статусе (`import_failed`).

`jni_import.cpp`: порт `cmake/import-gog.sh` на C++ (`std::filesystem`):
`--exe` = `innounp::extract` во временный каталог + копирование; `--game` —
только копирование. Каталоги искать в корне без учёта регистра, `Text/` →
`text/`, проверка `dat/Interface.dat`, создать `save/`, `screens/`, не трогать
существующие `save/`, `screens/`, `eador.cfg`. Нужно ~600 МБ свободного места
во время импорта из .exe (установщик + распаковка) — проверять и сообщать.

### 8. Порядок работы и проверки

1. Gradle-скелет с пустым `android_main` → APK собирается.
2. Vorbis для arm64-v8a.
3. Allegro + драйверы-заглушки → линкуется.
4. Графика: `android_main` заливает экран цветом через `agfx.c` → видно на
   устройстве/эмуляторе.
5. CP1251 + игра без звука → доходит до главного меню (данные положить через
   `adb push out/eador-linux/{dat,text,var,font*} /sdcard/Android/data/com.eador.reforged/files/`).
6. Касания → меню кликается; «Одиночная игра» запускается (это путь, где на Linux
   был segfault с гербом — фикс в `God/Eador.cpp` уже есть).
7. Звук.
8. Экран импорта и JNI, оба режима, ru/en.
9. Все четыре ABI, `eador_reforged.apk`.

Эмулятор: в SDK нет `emulator`/system images (проверить `sdkmanager --list_installed`);
для x86_64 образ скачивается отдельно. Устройство — через `adb`.

## Открытые вопросы (решить по ходу)

- Ввод имени игрока (`WindowEnter`, кампания) без аппаратной клавиатуры:
  показывать системную клавиатуру (`ANativeActivity_showSoftInput`) при
  ожидании `readkey`, либо Java-диалог. Сделать после основного запуска.
- Ctrl-модификаторы (покупка нескольких, `key[KEY_LCONTROL]`) на сенсоре —
  возможно, экранная кнопка-модификатор. Не входит в первый этап.
- Производительность масштабирования 1024×768 → экран на слабых armeabi-v7a;
  при необходимости — `ANativeWindow_setBuffersGeometry(window, 1024, 768, …)`
  и аппаратное масштабирование композитором вместо CPU.
