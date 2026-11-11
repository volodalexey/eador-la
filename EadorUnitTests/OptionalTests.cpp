#include <sstream>
#include <string>
#include <vector>

std::ostream& operator<<(std::ostream& out, const std::vector<unsigned char>& bytes);

#include "../God/Optional.hpp"
#include "doctest-configured.h"

std::ostream& operator<<(std::ostream& out, const std::vector<unsigned char>& bytes) {
	for (const auto& el : bytes) out << el;
	return out;
}

// Guard prefix written by OutputOptional<T> before the existence flag.
template <class T>
static std::string optionalPrefix() {
	std::stringstream buf;
	buf << binary::GuardPrefix::named<binary::InputOptional<T>>();
	return buf.str();
}

TEST_SUITE("Optional") {
	TEST_CASE("save exists int") {
		const auto bytes = std::vector<unsigned char>{42, 33, 28, 9};
		const auto prefix = optionalPrefix<decltype(bytes)>();
		std::stringstream buf;
		buf << binary::OutputOptional<decltype(bytes)>{&bytes};
		CAPTURE(buf.str());

		SUBCASE("output should start with guard prefix") {
			CHECK(buf.str().compare(0, prefix.size(), prefix) == 0);
		}

		SUBCASE("bytes count should be equal to prefix plus one byte-flag plus count of bytes") {
			CHECK(buf.str().size() == prefix.size() + 1 + bytes.size());
		}

		SUBCASE("byte-flag after prefix should be equal 1") {
			CHECK(buf.str().at(prefix.size()) == 1);
		}
	}

	TEST_CASE("save nullptr") {
		const auto prefix = optionalPrefix<int>();
		std::stringstream buf;
		buf << binary::OutputOptional<int>{nullptr};
		CAPTURE(buf.str());

		SUBCASE("bytes count should be equal to prefix plus one byte-flag") {
			CHECK(buf.str().size() == prefix.size() + 1);
		}

		SUBCASE("byte-flag after prefix should be equal 0") {
			CHECK(buf.str().at(prefix.size()) == 0);
		}
	}

	TEST_CASE("load int pointer") {
		std::stringstream source{ optionalPrefix<char>() + "\x01\x42" };
		char *destination = nullptr;
		auto box = binary::InputOptional<char>{destination};
		source >> box;

		SUBCASE("pointer should not be nullptr") {
			REQUIRE(destination != nullptr);
		}

		SUBCASE("read int should be equal 0x42") {
			REQUIRE(destination != nullptr);
			CHECK(*destination == 0x42);
		}

		delete destination;
	}

	TEST_CASE("load without guard prefix should throw") {
		std::stringstream source{ "\x01\x42" };
		char *destination = nullptr;
		CHECK_THROWS(source >> binary::InputOptional<char>{destination});
		CHECK(destination == nullptr);
	}
}
