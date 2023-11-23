#pragma once

#include "AsBinary.hpp"
#include <memory>

namespace binary {
	template <class It>
	class Range {
		It begin;
		It end;
	public:
		Range(It begin, It end) : begin(std::move(begin)), end(std::move(end)) {}

		friend auto& operator<<(std::ostream& out, const Range& range) {
			for (auto it = range.begin; it != range.end; ++it) {
				out << AsBinary(*it);
			}
			return out;
		}

		friend auto& operator>>(std::istream& in, Range& range) {
			for (auto it = range.begin; it != range.end; ++it) {
				in >> InputAsBinary(*it);
			}
			return in;
		}

		friend auto& operator>>(std::istream& in, Range&& range) {
			return in >> range;
		}
	};
}