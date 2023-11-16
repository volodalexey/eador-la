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

	template <class T>
	struct InputAsBinary {
		T& obj;
	};

	template <class T>
	inline std::istream& operator>>(std::istream& in, InputAsBinary<T> ref) {
		return in >> ref.obj;
	}

	inline std::istream& operator>>(std::istream& in, InputAsBinary<int> ref) {
		readBytes(in, ref.obj);
		return in;
	}
}