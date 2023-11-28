#pragma once

#include "GuardPrefix.hpp"
#include <iostream>
#include <stdexcept>
#include <format>

namespace binary {
	template <class T>
	struct InputOptional;

	template <class T>
	struct OutputOptional {
		T const* const obj;

		friend std::ostream& operator<<(std::ostream& out, const OutputOptional<T>& x) {
			out << GuardPrefix::named<InputOptional<T>>();
			if (not x.obj) return out << (char)0;
			return out << (char)1 << *x.obj;
		}
	};

	template <class T>
	struct InputOptional {
		T*& obj;

		friend std::istream& operator>>(std::istream& in, InputOptional<T>& x) {
			try {
				char existence;
				in >> GuardPrefix::named<InputOptional<T>>() >> existence;
				x.obj = nullptr;
				if (existence) {
					x.obj = new T;
					in >> *x.obj;
				}
				return in;
			}
			catch (...) {
				std::throw_with_nested(
					std::runtime_error(
						std::format(
							"Failed to read optional of {}",
							typeid(T).name())));
			}
		}

		friend std::istream& operator>>(std::istream& in, InputOptional<T>&& x) {
			return in >> x;
		}
	};
}
