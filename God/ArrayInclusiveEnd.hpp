#pragma once

#include "AsBinary.hpp"
#include <format>

namespace binary {
	template <class It>
	class ArrayInclusiveEnd {
		It begin;
		It end;
	public:
		ArrayInclusiveEnd(It begin, It end) : begin(begin), end(end) {}
		ArrayInclusiveEnd(It begin, int len) : ArrayInclusiveEnd(begin, begin + len) {}

		int len() const { return end - begin; }

		friend std::ostream& operator<<(std::ostream& out, const ArrayInclusiveEnd& arr) {
			out << AsBinary(arr.len());
			for (auto it = arr.begin; it != arr.end; ++it) out << AsBinary(*it);
			return out << AsBinary(*arr.end);
		}
	};

	template <class T>
	class InputArrayInclusiveEnd {
		T *&begin;
		T *end;
	public:
		InputArrayInclusiveEnd(T*&begin) : begin(begin), end(begin) {}

		int len() const { return end - begin; }

		friend std::istream& operator>>(std::istream& in, InputArrayInclusiveEnd& arr) {
			int len = 0;
			in >> InputAsBinary<int>{len};
			arr.begin = new T[len + 1];
			arr.end = arr.begin + len;
			for (int i = 0; i <= len; i++) {
				try {
					in >> InputAsBinary<T>{ arr.begin[i] };
				}
				catch (...) {
					std::throw_with_nested(
						std::runtime_error(
							std::format(
								"Failed while read {} with index {}",
								typeid(T).name(),
								i)));
				}
			}
			return in;
		}

		friend std::istream& operator>>(std::istream& in, InputArrayInclusiveEnd&& arr) {
			return in >> arr;
		}
	};
}