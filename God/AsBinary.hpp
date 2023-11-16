#pragma once

#include "Bytes.hpp"

namespace binary {
	template <class T>
	struct AsBinary {
		const T& obj;
	};

	template <class T>
	inline std::ostream& operator<<(std::ostream& out, AsBinary<T> ref) {
		return out << ref.obj;
	}

	inline std::ostream& operator<<(std::ostream& out, AsBinary<int> ref) {
		writeBytes(out, ref.obj);
		return out;
	}
}