#pragma once

#include "Log.hpp"
#include "Prototype.h"

class GameGlobalLog : public Log {
public:
	void log(const std::string& message) override {
		FileLog(message.data());
	}
};