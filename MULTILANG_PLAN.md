# План реализации многоязычной поддержки (по примеру zig-eador)

## Цель

Портировать логику импорта и загрузки файлов из zig-eador для поддержки нескольких языков в одной установке.

## Текущее состояние

- ✅ Игра работает с русской ИЛИ английской версией (одна за раз)
- ✅ `make import` копирует `dat/`, `text/`, `var/` "как есть"
- ✅ Файлы загружаются напрямую: `load_datafile("dat/Interface.dat")`, `OpenFile("text/hint.txt")`

## Целевая структура (zig-eador)

```text
out/
├── assets/
│   ├── common/           # Общие ресурсы
│   │   ├── dat/          # Оригинальные .dat (без изменений)
│   │   ├── fonts/        # fontNN.fnt + .png (BMFont формат)
│   │   ├── sprites/      # Распакованные спрайты
│   │   │   ├── interface/NNN_Name.png + index.txt
│   │   │   ├── buttons/NNN_Name.png + index.txt
│   │   │   └── ...
│   │   ├── music/        # Main01.ogg, War01.ogg, ...
│   │   ├── sound/        # unit_sound/NNN_Name.ogg + index.txt
│   │   ├── eador.cfg     # UTF-8, без \r
│   │   └── icon.png      # Иконка окна
│   ├── ru/               # Русская локализация
│   │   ├── manifest.txt  # Language: ru, Format: 2
│   │   ├── var/          # UTF-8, без \r
│   │   ├── text/         # UTF-8, без \r
│   │   ├── save/         # Сейвы для русской версии
│   │   ├── sprites/      # Локализованные спрайты (кнопки, меню)
│   │   │   ├── buttons/229_ButtDel.png
│   │   │   ├── interface/008_MainMenu.png
│   │   │   └── stuff/036_RacePlaque1.png
│   │   └── dat/          # Локализованные .dat (buttons, interface, stuff)
│   └── en/               # Английская локализация (та же структура)
├── eador                 # Бинарник
└── save/ -> assets/ru/save/  # Симлинк на активные сейвы
```

## Задачи

### Фаза 1: Импортер (C++ или shell)

**Выбор подхода:**

- [ ] **Вариант A (рекомендуется)**: Использовать готовый `zig-eador/src/import.zig`
  - Скомпилировать `eador-import` из zig-eador
  - Вызывать из Makefile: `./zig-out/bin/eador-import --game <src> out/assets`
  - **Плюсы**: Готовое решение, проверенное, поддержка PNG/PCX/шрифтов
  - **Минусы**: Зависимость от Zig

- [ ] **Вариант B**: Портировать на C++ постепенно
  - Начать с простого: копировать dat/text/var в assets/<lang>
  - Добавить конвертацию CP1251→UTF-8 (простая таблица)
  - Добавить распаковку спрайтов/звуков позже
  - **Плюсы**: Полный контроль, без зависимостей
  - **Минусы**: Много работы (~2000 строк кода)

**Подзадачи для варианта B:**

1. [ ] `import/cp1251.cpp` — CP1251↔UTF-8 конверсия
2. [ ] `import/eador_import.cpp` — основная логика
   - [ ] Определение языка из `goggame-*.info`
   - [ ] Создание структуры `assets/<lang>/` и `assets/common/`
   - [ ] Копирование `dat/` в `common/dat/`
   - [ ] Конвертация `text/*.txt`, `var/*.var` (CP1251→UTF-8, удаление `\r`)
   - [ ] Копирование `save/` в `<lang>/save/`
   - [ ] Экспорт `manifest.txt`
3. [ ] `import/dat_export.cpp` — распаковка спрайтов/звуков (raylib)
4. [ ] `import/font_export.cpp` — PCX шрифты → BMFont
5. [ ] `import/CMakeLists.txt` — сборка `eador-import`

### Фаза 2: Обновление загрузки файлов

**Цель**: Игра должна читать файлы из `assets/<lang>/` и `assets/common/`

1. [ ] `God/asset_loader.h/cpp` — обёртка над файловой системой

   ```cpp
   // Загружает файл, сначала ищет в <lang>/, потом в common/
   std::vector<uint8_t> load_asset(const char* path);
   // "text/hint.txt" → "assets/ru/text/hint.txt" или "assets/en/text/hint.txt"
   
   // Для сейвов: только в <lang>/save/, не fallback
   bool save_exists(const char* name);
   void save_write(const char* name, const void* data, size_t len);
   ```

2. [ ] Обновить все вызовы загрузки:
   - [ ] `load_datafile("dat/Interface.dat")` → `load_asset_datafile("dat/interface.dat")`
   - [ ] `OpenFile("text/hint.txt")` → открыть через `asset_loader`
   - [ ] `pack_fopen("save/game0.sav")` → `pack_fopen_asset("save/game0.sav")`
   - [ ] ~150-200 мест в коде

3. [ ] Обновить звуки/музыку:
   - [ ] `Audio.cpp`: загрузка из `common/music/`, `common/sound/`
   - [ ] Использовать `index.txt` для поиска объектов по имени

4. [ ] Обновить шрифты:
   - [ ] `load_txt_font("font01.txt")` → загрузка BMFont из `common/fonts/font01.fnt`

### Фаза 3: Переключение языков

1. [ ] Определение активного языка:

   ```cpp
   // God/language.cpp
   std::string current_language() {
       const char* env = getenv("EADOR_LANG");
       if (env) return env;  // Принудительный выбор
       // Автоопределение: ru если есть assets/ru/manifest.txt, иначе en
       if (file_exists("assets/ru/manifest.txt")) return "ru";
       return "en";
   }
   ```

2. [ ] Обновить Makefile:

   ```makefile
   run:
       cd $(OUT_DIR) && EADOR_LANG=$(or $(EADOR_LANG),$(shell ./detect_lang.sh)) ./eador
   ```

3. [ ] Отдельные сейвы: `assets/ru/save/` vs `assets/en/save/`

### Фаза 4: Обновление документации

1. [ ] `README.md`, `RU.md`, `EN.md`:
   - Описать новую структуру `assets/`
   - Примеры: `make run EADOR_LANG=ru`
   - Объяснить импорт обеих версий

2. [ ] `RAYLIB.md`: обновить пути к файлам

## Оценка трудозатрат

- **Фаза 1 (вариант A)**: 2-4 часа (интеграция zig-импортера)
- **Фаза 1 (вариант B)**: 20-30 часов (полный C++ импортер)
- **Фаза 2**: 10-15 часов (обновление ~200 мест в коде)
- **Фаза 3**: 2-3 часа (переключение языков)
- **Фаза 4**: 1-2 часа (документация)

**Итого**: 15-24 часа (вариант A) или 33-50 часов (вариант B)

## Рекомендуемый план действий

### Быстрое решение (минимальное)

Если нужна только поддержка `EADOR_LANG` без изменения структуры файлов:

1. Оставить `make import` как есть (копирует в `out/`)
2. Добавить `God/language.cpp` с `detect_language()` и `text_prefix()`
3. Заменить `"text/"` на `text_prefix().c_str()` в ~20 местах
4. Обновить Makefile для `EADOR_LANG=ru/en`
5. **Время**: 3-5 часов

### Полное решение (как zig-eador)

1. Скомпилировать `zig-eador/src/import.zig` (или портировать на C++)
2. Обновить `make import` для вызова `eador-import`
3. Реализовать `asset_loader.cpp` для чтения из `assets/<lang>/`
4. Обновить все вызовы загрузки файлов
5. **Время**: 15-50 часов (зависит от варианта импортера)

## Следующий шаг

Выберите подход и начните с фазы 1 или быстрого решения.
