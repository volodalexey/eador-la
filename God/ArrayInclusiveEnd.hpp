#pragma once

#include "AsBinary.hpp"

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
			return out << *arr.end;
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
			in >> InputAsBinary(len);
			arr.begin = new int[len + 1];
			arr.end = arr.begin + len;
			for (int i = 0; i <= len; i++) {
				auto box = InputAsBinary(arr.begin[i]);
				in >> box;
			}
			return in;
		}
	};
}