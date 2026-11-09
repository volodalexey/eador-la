# Обёртка над CMake для сборки под Linux.
#
#   make                                                  сборка allegro4 и игры
#   make import GOG_EXE=/path/to/setup_eador_genesis_*.exe  импорт из установщика GOG
#   make import GOG_PATH="/path/to/Eador - Genesis"        импорт из установленной копии
#   make run                                              запуск
#   make clean
#
# Все промежуточные файлы лежат в out/, субмодули не меняются.

OUT         ?= out
BUILD_DIR   ?= $(OUT)/build
INSTALL_DIR ?= $(OUT)/eador-linux
ALLEGRO4    ?= $(abspath $(OUT)/allegro4)
INNOUNP     ?= $(abspath $(OUT)/innounp-build/innounp)

.PHONY: all build configure allegro4 innounp import run clean

all: build

allegro4:
	@if [ ! -f $(ALLEGRO4)/lib/liballeg.so ]; then \
		cmake -S allegro4 -B $(OUT)/allegro4-build -G Ninja \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
			-DCMAKE_INSTALL_PREFIX=$(ALLEGRO4) \
			-DWANT_EXAMPLES=OFF -DWANT_TESTS=OFF -DWANT_DOCS=OFF \
			-DCMAKE_C_FLAGS='-fcommon -std=gnu11 -Wno-error=incompatible-pointer-types -Wno-error=int-conversion -Wno-error=implicit-function-declaration' && \
		cmake --build $(OUT)/allegro4-build && \
		cmake --install $(OUT)/allegro4-build; \
	fi

configure: allegro4
	cmake -S . -B $(BUILD_DIR) -G Ninja \
		-DCMAKE_BUILD_TYPE=Release \
		-DALLEGRO4_PREFIX=$(ALLEGRO4)

build: configure
	cmake --build $(BUILD_DIR)
	cmake --build $(BUILD_DIR) --target prepare-native

# Свой распаковщик установщиков GOG (Inno Setup 5.5.7), зависит только от zlib.
innounp:
	cmake -S innounp -B $(OUT)/innounp-build -G Ninja -DCMAKE_BUILD_TYPE=Release
	cmake --build $(OUT)/innounp-build

import:
ifdef GOG_EXE
	$(MAKE) innounp
	sh cmake/import-gog.sh --exe "$(GOG_EXE)" $(INSTALL_DIR) $(INNOUNP)
else ifdef GOG_PATH
	sh cmake/import-gog.sh --game "$(GOG_PATH)" $(INSTALL_DIR)
else
	@echo "Usage: make import GOG_EXE=/path/to/setup_eador_genesis_*.exe"
	@echo "    or make import GOG_PATH=/path/to/installed/Eador"
	@exit 1
endif

run:
	cd $(INSTALL_DIR) && LD_LIBRARY_PATH=$(ALLEGRO4)/lib ALLEGRO_MODULES=$(ALLEGRO4)/lib/allegro/4.4.4 ./Eador

clean:
	rm -rf $(BUILD_DIR) $(INSTALL_DIR)/Eador
