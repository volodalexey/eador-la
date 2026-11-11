# Обёртка над CMake для сборки под Linux.
#
#   make                                                    сборка игры (raylib)
#   make import GOG_EXE=/path/to/setup_eador_genesis_*.exe  импорт данных из установщика GOG
#   make import GOG_PATH="/path/to/Eador - Genesis"          импорт из установленной копии
#   make run                                                запуск
#   make test                                               юнит-тесты
#   make clean
#
# В out/ лежат только данные игры и бинарники eador и innounp,
# промежуточные файлы сборки — в build/. Субмодули не меняются.

OUT     ?= out
BUILD   ?= build
OUT_DIR := $(abspath $(OUT))

.PHONY: all build configure innounp import run test clean

all: build

configure:
	cmake -S . -B $(BUILD)/eador -G Ninja \
		-DCMAKE_BUILD_TYPE=Release \
		-DEADOR_OUT_DIR="$(OUT_DIR)"

build: configure
	cmake --build $(BUILD)/eador

# Свой распаковщик установщиков GOG (Inno Setup 5.5.7), зависит только от zlib.
innounp:
	cmake -S innounp -B $(BUILD)/innounp -G Ninja \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$(OUT_DIR)"
	cmake --build $(BUILD)/innounp

import:
ifdef GOG_EXE
	$(MAKE) innounp
	sh cmake/import-gog.sh --exe "$(GOG_EXE)" "$(OUT_DIR)" "$(OUT_DIR)/innounp"
else ifdef GOG_PATH
	sh cmake/import-gog.sh --game "$(GOG_PATH)" "$(OUT_DIR)"
else
	@echo "Usage: make import GOG_EXE=/path/to/setup_eador_genesis_*.exe"
	@echo "    or make import GOG_PATH=/path/to/installed/Eador"
	@exit 1
endif

run:
	cd "$(OUT_DIR)" && ./eador

test: build
	ctest --test-dir $(BUILD)/eador --output-on-failure

clean:
	rm -rf $(BUILD) "$(OUT_DIR)/eador" "$(OUT_DIR)/innounp"
