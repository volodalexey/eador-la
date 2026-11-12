# Eador: Genesis — linux|android port

## Building on Linux

### Requirements

- `CMake 3.22+`, Ninja or Make
- Packages: `zlib1g-dev` (for `innounp`), plus raylib/GLFW dependencies: `libx11-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxi-dev`, `libgl-dev`
- Sources with submodules: `git submodule update --init --recursive`

### Get the sources

```sh
git clone --recursive https://github.com/volodalexey/eador-la.git
# or update an existing clone
git pull --recurse-submodules
```

### Quick start

```sh
# 1. Build (raylib is compiled from the submodule automatically)
make

# 2. Import assets from the GOG installer
make import GOG_EXE=/path/to/setup_eador_genesis_1.05.02_language_update_(21780).exe
# — or from an already installed copy
make import GOG_PATH="/path/to/Eador - Genesis"

# 3. Run
make run
```

`out/` contains only the game data, `eador.cfg`, `save/`, `screens/` and the
`eador` and `innounp` binaries. Intermediate build files go to `build/`.

### What `make import` copies

From the GOG distribution or installer:

| Source             | Destination                 | Description                       |
|--------------------|-----------------------------|-----------------------------------|
| `dat/`             | `out/dat/`      | game data files (graphics, sound…)|
| `text/`            | `out/text/`     | text strings (locale-dependent)   |
| `var/`             | `out/var/`      | game parameters                   |
| `font01.pcx` etc.  | `out/`          | bitmap fonts                      |
| `goggame-*.ico`    | `out/eador.ico` | window icon                       |

`save/`, `screens/` and `eador.cfg` are **not overwritten** on a repeat import.  
Both the English (`Text/`) and Russian (`text/`) installers are handled identically.

### Details

**raylib** is built from the `raylib/` submodule together with the game.
Allegro 4 is no longer needed: the `God/allegro_rl.*` layer emulates its API (see `RAYLIB.md`).

**innounp** (the `innounp/` directory) is a built-in extractor for the GOG
installers (Inno Setup 5.5.7, LZMA1/LZMA2) that depends only on zlib. It is used
only when passing `GOG_EXE`. Assets from an installed folder are copied directly.

**Window icon** comes from `goggame-*.ico`; the 32-bit BMP images inside it
are set via the X11 `_NET_WM_ICON` property.

**Window title** is shown as «Эадор. Сотворение» or «Eador. Genesis»
depending on the distribution locale. It is sent as UTF-8 via `_NET_WM_NAME`,
so any modern desktop environment renders it correctly.

**Unit tests:**

```sh
make test
```

## Building for Android

### Requirements

- JDK 17 (e.g., `openjdk-17-jdk`)
- Android SDK packages (X11/GL dev packages from the Linux section are not needed):

  ```sh
  sdkmanager "platforms;android-35" "build-tools;35.0.0" \
             "ndk;29.0.14206865" "cmake;3.22.1"
  ```

- `ANDROID_HOME` environment variable or `android/local.properties` with `sdk.dir=/path/to/sdk`

The project uses `Gradle` wrapper 8.13, which is downloaded automatically
on the first build.

### Quick start

```sh
# 1. Build APK (all ABIs: arm64-v8a, armeabi-v7a, x86, x86_64)
cd android
./gradlew assembleRelease

# The APK is at: app/build/outputs/apk/release/eador_reforged.apk
```

### Installing and importing game data

The APK contains only the game code. Game data must be imported on first launch:

1. Install the APK: `adb install app/build/outputs/apk/release/eador_reforged.apk`
2. Launch the app. You'll see two buttons:
   - **Import from GOG_EXE file**: select the GOG installer `.exe` (requires ~600 MB free space during extraction)
   - **Import from GOG_PATH folder**: select the root folder of an already installed copy
3. After successful import, the game starts automatically on subsequent launches.

Game data is stored in `/sdcard/Android/data/com.eador.reforged/files/`.

### Manual data import

You can push data via `adb` instead of using the in-app importer:

```sh
# Example: push from a Linux installation's out/ directory
D=/sdcard/Android/data/com.eador.reforged/files
adb shell mkdir -p "$D"
adb push out/dat out/text out/var out/font01.pcx out/font01.txt \
         out/font02.pcx out/font02.txt "$D/"
```

### Details

**Platform:** raylib with `PLATFORM=Android` (GLES2). Touch input is mapped to mouse:

- Single tap = left click
- Long press (≥500 ms) = right click
- Two-finger vertical swipe = mouse wheel

**Soft keyboard:** Opens automatically when a text field is active (hero name, land name).
Enter finishes input and hides it; the system Back button hides only the keyboard, the field stays active.

**ABIs:** The APK includes native libraries for all four ABIs (arm64-v8a, armeabi-v7a, x86, x86_64).
