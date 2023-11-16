#pragma once

#include "AsBinary.hpp"

namespace binary {
	template <class It>
	class ArrayInclusiveEnd {
		It begin;
		It end;
	public:
		ArrayInclusiveEnd(It begin, It end) : begin(begin), end(end) {}

		int len() const { return end - begin + 1; }

		friend std::ostream& operator<<(std::ostream& out, const ArrayInclusiveEnd& arr) {
			out << AsBinary(arr.len());
			for (auto it = arr.begin; it != arr.end; ++it) out << AsBinary(*it);
			return out << *arr.end;
		}
	};
}