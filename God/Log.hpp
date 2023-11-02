#pragma once

#include "BaseInterface.hpp"
#include <string>

class Log : public BaseInterface {
public:
	virtual void log(const std::string& message) = 0;
};
