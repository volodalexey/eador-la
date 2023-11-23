#include "../GoD/AsBinary.hpp"
#include "doctest-configured.h"
#include <sstream>

TEST_SUITE("AsBinary") {
	TEST_CASE("read char") {
		char ch;
		std::stringstream{ "\x10" } >> binary::InputAsBinary<char>{ch};
		SUBCASE("char should be equal to 0x10") {
			CHECK(ch == 0x10);
		}
	}

	TEST_CASE("read int") {
		int number = 42;
		SUBCASE("number should be equal to 0") {
			std::stringstream{ std::string("\0\0\0\0", 4)} >> binary::InputAsBinary<int>{number};
			CHECK(number == 0);
		}
		SUBCASE("number should be equal to 3") {
			std::stringstream{ std::string("\3\0\0\0", 4) } >> binary::InputAsBinary<int>{number};
			CHECK(number == 3);
		}
	}

	TEST_CASE("copy box") {
		int number = 42;
		auto original = binary::InputAsBinary<int>{ number };
		auto copy = original;
		
		SUBCASE("original and copy should be located at different addresses") {
			CHECK(&original != &copy);
		}
		
		SUBCASE("references inside original and copy should be equal") {
			CHECK(&original.obj == &copy.obj);
		}
	}
}