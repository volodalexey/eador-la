# Building on Linux

## Requirements

- `CMake 3.8+`, Ninja or Make
- Packages: `libvorbis-dev`, `libx11-dev`, `libxinerama-dev`, `pkg-config`
- Sources with submodules: `git submodule update --init --recursive`

## Get the sources

```sh
git clone --recursive https://github.com/volodalexey/eador-la.git
# or update an existing clone
git pull --recurse-submodules
```

## Quick start

```sh
# 1. Build (allegro4 is compiled from the submodule automatically)
make

# 2. Import assets from the GOG installer
make import GOG_EXE=/path/to/setup_eador_genesis_1.05.02.exe
# — or from an already installed copy
make import GOG_PATH="/path/to/Eador - Genesis"

# 3. Run
make run
```

The binary and game data end up in `out/eador-linux/`.

## What `make import` copies

From the GOG distribution or installer:

| Source             | Destination                 | Description                       |
|--------------------|-----------------------------|-----------------------------------|
| `dat/`             | `out/eador-linux/dat/`      | game data files (graphics, sound…)|
| `text/`            | `out/eador-linux/text/`     | text strings (locale-dependent)   |
| `var/`             | `out/eador-linux/var/`      | game parameters                   |
| `font01.pcx` etc.  | `out/eador-linux/`          | bitmap fonts                      |
| `goggame-*.ico`    | `out/eador-linux/eador.ico` | window icon                       |

`save/`, `screens/` and `eador.cfg` are **not overwritten** on a repeat import.  
Both the English (`Text/`) and Russian (`text/`) installers are handled identically.

## Details

**Allegro 4.4** is built from the `allegro4/` submodule into `out/allegro4/`;
subsequent `make build` calls skip the rebuild.

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
make build
ctest --test-dir out/build --output-on-failure
```
