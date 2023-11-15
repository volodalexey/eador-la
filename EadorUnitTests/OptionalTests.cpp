#include <sstream>
#include <vector>

std::ostream& operator<<(std::ostream& out, const std::vector<unsigned char>& bytes);

#include "../GoD/Optional.hpp"
#include "doctest-configured.h"

std::ostream& operator<<(std::ostream& out, const std::vector<unsigned char>& bytes) {
	for (const auto& el : bytes) out << el;
	return out;
}

TEST_SUITE("Optional") {
	TEST_CASE("save exists int") {
		const auto bytes = std::vector<unsigned char>{42, 33, 28, 9};
		std::stringstream buf;
		buf << binary::OutputOptional<decltype(bytes)>{&bytes};
		CAPTURE(buf.str());

		SUBCASE("bytes count should be equal to count of bytes plus one byte-flag") {
			CHECK(buf.str().size() == 1 + bytes.size());
		}

		SUBCASE("first byte should be equal 1") {
			CHECK(buf.str().front() == 1);
		}
	}

	TEST_CASE("save nullptr") {
		std::stringstream buf;
		buf << binary::OutputOptional<int>{nullptr};
		CAPTURE(buf.str());

		SUBCASE("bytes count should be equal to ont byte-flag") {
			CHECK(buf.str().size() == 1);
		}

		SUBCASE("first byte should be equal 0") {
			CHECK(buf.str().front() == 0);
		}
	}
}