#include <sstream>
#include <vector>

std::ostream& operator<<(std::ostream& out, const std::vector<unsigned char>& bytes);

#include "../God/Optional.hpp"
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

	TEST_CASE("load int pointer") {
		std::stringstream source{ "\x01\x42" };
		char *destination = nullptr;
		auto box = binary::InputOptional<char>{destination};
		source >> box;

		SUBCASE("pointer should not be nullptr") {
			REQUIRE(destination != nullptr);
		}

		SUBCASE("read int should be equal 0x42") {
			CHECK(*destination == 0x42);
		}
	}
}