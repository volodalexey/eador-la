#pragma once

#include "Unit.h"
#include "Bytes.hpp"

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcUnit& unit) {
		writeBytes(out, unit);
		return out;
	}

	inline std::istream& operator>>(std::istream& in, dcUnit& unit) {
		readBytes(in, unit);
		return in;
	}
}
