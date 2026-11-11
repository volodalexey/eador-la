# Building on Linux

## Requirements

- `CMake 3.22+`, Ninja or Make
- Packages: `zlib1g-dev` (for `innounp`), plus raylib/GLFW dependencies: `libx11-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxi-dev`, `libgl-dev`
- Sources with submodules: `git submodule update --init --recursive`

## Get the sources

```sh
git clone --recursive https://github.com/volodalexey/eador-la.git
# or update an existing clone
git pull --recurse-submodules
```

## Quick start

```sh
# 1. Build (raylib is compiled from the submodule automatically)
make

# 2. Import assets from the GOG installer
make import GOG_EXE=/path/to/setup_eador_genesis_1.05.02.exe
# — or from an already installed copy
make import GOG_PATH="/path/to/Eador - Genesis"

# 3. Run
make run
```

`out/` contains only the game data, `eador.cfg`, `save/`, `screens/` and the
`eador` and `innounp` binaries. Intermediate build files go to `build/`.

## What `make import` copies

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

## Details

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
