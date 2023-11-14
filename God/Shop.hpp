#pragma once

#include "Unit.h"
#include "Bytes.hpp"

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcShop& shop) {
		writeBytes(out, shop);
		return out;
	}

	inline std::istream& operator>>(std::istream& in, dcShop& shop) {
		readBytes(in, shop);
		return in;
	}
}
