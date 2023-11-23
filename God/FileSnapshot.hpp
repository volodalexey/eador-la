#pragma once

#include "Snapshot.hpp"
#include <filesystem>
#include <iterator>
#include <fstream>

class FileSnapshot : public Snapshot {
	std::filesystem::path path;
public:
	explicit FileSnapshot(std::filesystem::path path) : path(std::move(path)) {}

	std::string bytes() const override {
		std::ifstream file(path, std::ios::binary);
		return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
	}
};