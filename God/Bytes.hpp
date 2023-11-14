#pragma once

#include <iostream>

namespace binary {
	template <class T>
	void writeBytes(std::ostream& out, const T& x) {
		out.write(reinterpret_cast<const char*>(&x), sizeof(x));
	}

	void writeBytes(std::ostream& out, const char* x, std::size_t len) {
		out.write(x, len);
	}

	template <class T>
	void readBytes(std::istream& in, T& x) {
		in.read(reinterpret_cast<char*>(&x), sizeof(x));
	}

	void readBytes(std::istream& in, char* x, std::size_t len) {
		in.read(x, len);
	}
}
