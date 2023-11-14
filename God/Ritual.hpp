#pragma once

#include "Unit.h"
#include "Bytes.hpp"

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcRitual& ritual) {
		writeBytes(out, ritual);
		return out;
	}

	inline std::istream& operator>>(std::istream& in, dcRitual& ritual) {
		readBytes(in, ritual);
		return in;
	}
}
