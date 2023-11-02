#pragma once

#include "BaseInterface.hpp"
#include <string>

class Snapshot : public BaseInterface {
public:
	virtual std::string bytes() const = 0;
};