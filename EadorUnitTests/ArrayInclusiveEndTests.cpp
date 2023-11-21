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

	struct custom {
		int a, b;

		auto operator<=>(const custom&) const = default;

		friend std::ostream& operator<<(std::ostream& out, const custom& x) {
			return out << binary::AsBinary<int>{x.a} << binary::AsBinary<int>{x.b};
		}

		friend std::istream& operator>>(std::istream& in, custom& x) {
			return in >> binary::InputAsBinary<int>{x.a} >> binary::InputAsBinary<int>{x.b};
		}
	};

	TEST_CASE("read array of structs") {
		std::stringstream buf;
		buf << binary::AsBinary<int>{2} << custom{ 1, 2 } << custom{ 3, 4 } << custom{ 5, 6 };

		custom* array = nullptr;
		auto box = binary::InputArrayInclusiveEnd<custom>{ array };
		buf >> box;

		SUBCASE("array should not be nullptr") {
			REQUIRE(array != nullptr);
		}

		SUBCASE("len should be equal to 2") {
			CHECK(box.len() == 2);
		}

		SUBCASE("each element should be equal source value") {
			CHECK(array[0] == custom{1, 2});
			CHECK(array[1] == custom{3, 4});
			CHECK(array[2] == custom{5, 6});
		}
	}
}