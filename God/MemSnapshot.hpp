#pragma once

#include "Snapshot.hpp"

class MemSnapshot : public Snapshot {
	std::string _bytes;
public:
	explicit MemSnapshot(std::string bytes) : _bytes(std::move(bytes)) {}

	std::string bytes() const override { return _bytes; }
};