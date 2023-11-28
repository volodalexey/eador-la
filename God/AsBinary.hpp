#pragma once

#include "Bytes.hpp"
#include <concepts>

namespace binary {
	template <class T>
	struct AsBinary {
		const T& obj;
	};

	template <class T>
	inline std::ostream& operator<<(std::ostream& out, AsBinary<T> ref) {
		return ::binary::operator<<(out, ref.obj);
	}

	template <class T> requires std::integral<T> or std::floating_point<T>
	inline std::ostream& operator<<(std::ostream& out, AsBinary<T> ref) {
		writeBytes(out, ref.obj);
		return out;
	}

	template <class T>
	struct InputAsBinary {
		T& obj;
	};

	template <class T>
	inline std::istream& operator>>(std::istream& in, InputAsBinary<T> ref) {
		return ::binary::operator>>(in, ref.obj);
	}

	template <class T> requires std::integral<T> or std::floating_point<T>
	inline std::istream& operator>>(std::istream& in, InputAsBinary<T> ref) {
		readBytes(in, ref.obj);
		return in;
	}
}