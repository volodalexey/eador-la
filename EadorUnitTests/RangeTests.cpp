#include "../God/Range.hpp"
#include "doctest-configured.h"
#include <sstream>

TEST_SUITE("Range") {
	TEST_CASE("output numbers") {
		const int numbers[3] = { 1, 2, 3 };
		std::stringstream buf;
		buf << binary::Range(numbers, numbers + 3);

		SUBCASE("buf should contains 12 bytes") {
			CHECK(buf.str().size() == 12);
		}
		SUBCASE("buf should be equal to concrete byte sequence") {
			CHECK(buf.str() == std::string("\1\0\0\0" "\2\0\0\0" "\3\0\0\0", 3 * sizeof(int)));
		}
	}

	TEST_CASE("input numbers") {
		int numbers[3] = {0};
		std::stringstream buf{std::string("\1\0\0\0" "\2\0\0\0" "\3\0\0\0", 3 * sizeof(int))};
		buf >> binary::Range(numbers, numbers + 3);

		SUBCASE("1st number should be equal 1") {
			CHECK(numbers[0] == 1);
		}
		SUBCASE("2nd number should be equal 2") {
			CHECK(numbers[1] == 2);
		}
		SUBCASE("3th number should be equal 3") {
			CHECK(numbers[2] == 3);
		}
	}
}
