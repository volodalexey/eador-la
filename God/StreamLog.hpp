#pragma once

#include "Log.hpp"

class StreamLog : public Log {
	std::ostream& out;
public:
	explicit StreamLog(std::ostream& out) : out(out) {}

	void log(const std::string& message) override {
		out << message;
	}
};