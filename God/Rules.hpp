#pragma once

#include "Unit.h"
#include "Bytes.hpp"

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcWorldRules& rules) {
		writeBytes(out, rules);
		return out;
	}

	inline std::istream& operator>>(std::istream& in, dcWorldRules& rules) {
		readBytes(in, rules);
		return in;
	}
}
