#include "../GoD/ArrayInclusiveEnd.hpp"
#include "doctest-configured.h"
#include <sstream>

TEST_SUITE("ArrayInclusiveEnd") {
	char arr[] = { 1, 2, 3, 4, 42 };
	auto count = _countof(arr) - 1;

	TEST_CASE("save array of int") {
		auto box = binary::ArrayInclusiveEnd<char*>(arr, arr + count);
	
		SUBCASE("len should be equal count of source elements minus one") {
			CHECK(box.len() == count);
		}
	}
}