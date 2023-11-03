#pragma once

#include "Log.hpp"
#include <fstream>

class TextFileLog : public Log {
	std::ofstream out;
public:
	explicit TextFileLog(std::ofstream&& out) : out(std::move(out)) {}

	explicit TextFileLog(std::string filename) : TextFileLog(std::ofstream(filename, std::ofstream::out)) {}

	void log(const std::string& message) override {
		out << message << std::endl;
	}
};
