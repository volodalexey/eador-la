# Сборка под Linux

## Требования

- `CMake 3.22+`, `Ninja` или `Make`
- Пакеты: `zlib1g-dev` (для `innounp`), а также зависимости raylib/GLFW: `libx11-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxi-dev`, `libgl-dev`
- Исходники с субмодулями: `git submodule update --init --recursive`

## Скачать исходники

```sh
git clone --recursive https://github.com/volodalexey/eador-la.git
# или обновить существующий клон
git pull --recurse-submodules
```

## Быстрый старт

```sh
# 1. Сборка (raylib собирается из субмодуля автоматически)
make

# 2. Импорт ресурсов из GOG-установщика
make import GOG_EXE=/path/to/setup_eador_genesis_1.05.02.exe
# — или из уже установленной копии
make import GOG_PATH="/path/to/Eador - Genesis"

# 3. Запуск
make run
```

В `out/` лежат только данные игры, `eador.cfg`, `save/`, `screens/` и бинарники
`eador` и `innounp`. Промежуточные файлы сборки лежат в `build/`.

## Что копирует `make import`

Из GOG-дистрибутива или установщика копируется:

| Откуда             | Куда                    | Описание                          |
|--------------------|-------------------------|-----------------------------------|
| `dat/`             | `out/dat/`  | датафайлы игры (графика, звук…)   |
| `text/`            | `out/text/` | тексты (локаль-зависимые)         |
| `var/`             | `out/var/`  | игровые параметры                 |
| `font01.pcx` и др. | `out/`      | растровые шрифты                  |
| `goggame-*.ico`    | `out/eador.ico` | иконка окна                   |

`save/`, `screens/` и `eador.cfg` при повторном импорте **не трогаются**.  
Английский (`Text/`) и русский (`text/`) установщики обрабатываются одинаково.

## Детали

**raylib** собирается из субмодуля `raylib/` вместе с игрой. Allegro 4 больше
не нужен: его API эмулирует слой `God/allegro_rl.*` (см. `RAYLIB.md`).

**innounp** (каталог `innounp/`) — собственный распаковщик установщиков GOG
(Inno Setup 5.5.7, LZMA1/LZMA2), зависит только от zlib. Используется только
при передаче `GOG_EXE`. Из установленной папки всё копируется напрямую.

**Иконка окна** берётся из `goggame-*.ico`; 32-битные BMP-образы внутри
него передаются в X11 через `_NET_WM_ICON`.

**Заголовок окна** выводится как «Эадор. Сотворение» или «Eador. Genesis»
в зависимости от локали дистрибутива. Передаётся в UTF-8 через `_NET_WM_NAME`,
поэтому корректно показывается в любом современном DE.

**Тесты:**
```sh
make test
```
