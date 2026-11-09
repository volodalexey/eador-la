// Распаковщик установщиков GOG Eador: Genesis (Inno Setup 5.5.7 unicode,
// один .exe, без шифрования). Порт src/formats/inno.zig из zig-eador.
#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace innounp {

struct Summary {
	size_t files = 0;
	uint64_t bytes = 0;
	size_t skipped_filtered = 0;
};

// Вызывается после записи каждой порции данных: сколько байт записано из общего числа.
using Progress = std::function<void(uint64_t done, uint64_t total)>;

// Распаковывает все файлы установщика в out_dir (каталоги создаются,
// существующие файлы перезаписываются). Ошибки — std::runtime_error.
Summary extract(const std::string &exe_path, const std::string &out_dir,
                const Progress &progress = {});

} // namespace innounp
