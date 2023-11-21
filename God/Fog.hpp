#pragma once

#include "Prototype.h"
#include "Bytes.hpp"

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const clFog& fog) {
		writeBytes(out, fog);
		return out;
	}

	inline std::istream& operator>>(std::istream& in, clFog& fog) {
		readBytes(in, fog);
		return in;
	}
}
