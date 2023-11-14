#pragma once

#include "Unit.h"
#include "Bytes.hpp"

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcTutorial& tutorial) {
		writeBytes(out, tutorial);
		return out;
	}

	inline std::istream& operator>>(std::istream& in, dcTutorial& tutorial) {
		readBytes(in, tutorial);
		return in;
	}
}
