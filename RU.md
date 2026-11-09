# Сборка под Linux

## Требования

- `CMake 3.8+`, `Ninja` или `Make`
- Пакеты: `libvorbis-dev`, `libx11-dev`, `libxinerama-dev`, `pkg-config`
- Исходники с субмодулями: `git submodule update --init --recursive`

## Скачать исходники

```sh
git clone --recursive https://github.com/volodalexey/eador-la.git
# или обновить существующий клон
git pull --recurse-submodules
```

## Быстрый старт

```sh
# 1. Сборка (allegro4 собирается из субмодуля автоматически)
make

# 2. Импорт ресурсов из GOG-установщика
make import GOG_EXE=/path/to/setup_eador_genesis_1.05.02.exe
# — или из уже установленной копии
make import GOG_PATH="/path/to/Eador - Genesis"

# 3. Запуск
make run
```

Бинарник и данные оказываются в `out/eador-linux/`.

## Что копирует `make import`

Из GOG-дистрибутива или установщика копируется:

| Откуда             | Куда                    | Описание                          |
|--------------------|-------------------------|-----------------------------------|
| `dat/`             | `out/eador-linux/dat/`  | датафайлы игры (графика, звук…)   |
| `text/`            | `out/eador-linux/text/` | тексты (локаль-зависимые)         |
| `var/`             | `out/eador-linux/var/`  | игровые параметры                 |
| `font01.pcx` и др. | `out/eador-linux/`      | растровые шрифты                  |
| `goggame-*.ico`    | `out/eador-linux/eador.ico` | иконка окна                   |

`save/`, `screens/` и `eador.cfg` при повторном импорте **не трогаются**.  
Английский (`Text/`) и русский (`text/`) установщики обрабатываются одинаково.

## Детали

**Allegro 4.4** собирается из субмодуля `allegro4/` в `out/allegro4/`;
при повторных `make build` пересборки не происходит.

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
make build
ctest --test-dir out/build --output-on-failure
```
