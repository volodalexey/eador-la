# Пеписывание ядра (God) "Эадор. Сотворение" на zig

`Raylib` остаётся рендером (`raylib_zig-6.2.0`)

## Импорт (перепаковка и подготовка) ресурсов игры для raylib

```sh
zig build import -Dgog_exe=/path/to/setup_eador_genesis_1.05.02_language_update_(russian)_(21780).exe
```

## Архитектура

```text
zig-eador/
├── build.zig            цели: eador, eador-import, test; шаг import
├── src/
│   ├── main.zig         окно, кадровый цикл, стек сцен
│   ├── platform/
│   │   ├── canvas.zig   виртуальный экран 1024×768 (RenderTexture2D + letterbox)
│   │   ├── input.zig    Pointer + клавиатура (см. ниже)
│   │   ├── gfx.zig      спрайты, атлас, тонирование, затемнение, blend
│   │   ├── font.zig     PCX-шрифты font01/font02, текст в cp1251-глифах
│   │   ├── audio.zig    музыка (stream), звуки юнитов и заклинаний
│   │   └── fs.zig       поиск ресурсов, регистронезависимые пути
│   ├── formats/         парсеры/сериализаторы для файлов игры: datafile, lzss, packfile, varfile, textfile, cp1251, pcx, png
│   ├── data/            прототипы (Prototype.cpp): UnitType, Spell, Item, Race, Site, Ritual, ...
│   ├── game/            состояние партии (Unit.h: Game, Province, Player, Hero, Unit, Shop, Event, Ritual)
│   ├── save/            кодек оригинальных .sav/.map/.cmp/.hot/.prf/.boo
│   ├── ui/              виджеты (Interface.cpp, Windower.cpp, G_Object.cpp)
│   ├── scenes/          intro, menu, new_game, map, castle, hero, shop, battle, campaign, ...
│   ├── logic/           MainProcess, Mapper (генерация), Province, Hero, Player, Event, Simulation
│   ├── battle/          Battlefield, BF_Unit, BattleAI
│   └── ai/              MainAI
└── tools/import.zig     eador-import: God/dat, var, text → zig-out/assets
```

## [План переписывания](./PLAN.md)

Эталон кода C++ исходник `../God/*.cpp`. Если код расходится с данными `GOG`, смотреть `./recomp/` (восстановлен по бинарнику) и делать так, чтобы данные `GOG` читались и работали
Декомпилированный код (russian/english) из оригинальных файлов `Eador.exe` - `./recomp/` - для сверки и точной логики (иногда `Ghidra` может ошибаться, поэтому есть дизассемблерный код внутри)
Стиль - Идиоматичный Zig: состояние в структурах, стек сцен вместо вложенных `while`, индексы вместо указателей
Ресурсы - Импорт `.dat` → `PNG`/`OGG` при сборке (`eador-import`), игра читает готовые файлы
Сохранения Чтение и запись формата GOG 1.05.02 (32-битные дампы), эталоны в `testdata`
Локализации RU и EN импорт из установщика GOG (`-Dgog_exe`) или из установленной игры (`-Dgog_game`, например `wine`)
Эталон данных Файлы GOG 1.05.02 (`.var`, `.txt`, `.dat`, `.sav`|`.cmp`|`.hot`)
Не переносится из C++ - Netpatch, Snapshot, Host, State, cpp-httplib, EadorServer, YuPlay, Keymaster.cpp, реестр, ключи, ShellExecute, EventServer.cpp, Overlord.cpp, остатки Audiere

## Файлы сохранения

- кампания
 -- game0.cmp
 -- game0.bkc
 -- map0.cmp
- одинночная игра
 -- game0.sav
 -- game0.bks
 -- map0.map
- многопользовательская игра
 -- game0.hot
 -- game0.bka
 -- map0.hot

## Проверки

```sh
zig build test
zig build test -Dforce # принудительно запустить заново все тесты с пустым кешем
```

## Скриншоты для тестирования

- Загрузить game0 + map0 + отобразить карту + не двигать курсор + снять скриншот

  ```sh
  EADOR_SCREEN=continue EADOR_SHOT=/home/lex/Pictures/Screenshots/turn45-zig.png EADOR_SET_CURSOR=0 zig buil
  d run
  ```

  - EADOR_SCREEN - какую сцену отобразить
  - EADOR_SHOT - куда сохранить снятый скриншот
  - EADOR_SET_CURSOR - выставить курсор (0 - принудительно отключить выставление)
  - EADOR_FRAMES - подождать указанное количество фреймов
