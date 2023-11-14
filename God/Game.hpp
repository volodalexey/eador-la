#pragma once

#include "Unit.h"
#include "Bytes.hpp"

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcGame& game) {
		writeBytes(out, game);
		return out;
	}

	inline std::istream& operator>>(std::istream& in, dcGame& game) {
		readBytes(in, game);
		return in;
	}
}
