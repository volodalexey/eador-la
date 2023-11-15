#pragma once

#include <iostream>

namespace binary {
	template <class T>
	struct OutputOptional {
		T const* const obj;

		friend std::ostream& operator<<(std::ostream& out, const OutputOptional<T>& x) {
			if (not x.obj) return out << false;
			return out << (char)1 << *x.obj;
		}
	};

	template <class T>
	struct InputOptional {
		T*& obj;

		friend std::istream& operator>>(std::istream& in, InputOptional<T>& x) {
			bool existence;
			in >> existence;
			x.obj = nullptr;
			if (existence) {
				x.obj = new T;
				in >> *x.obj;
			}
			return in;
		}
	};
}
