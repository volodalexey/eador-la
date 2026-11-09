#include "../God/Unit.h"
#include <iosfwd>
#include "../God/Province.hpp"
#include "doctest-configured.h"
#include <sstream>

TEST_SUITE("Province") {
	TEST_CASE("serialized province should be equal to source province") {
		using namespace binary;

		auto source = dcProvince{};
		source.arSite[1][0] = 42;
		source.pGuard = nullptr;
		source.pGarrison = nullptr;
		source.sName = strdup("province");
		auto copy = dcProvince{};

		std::stringstream buf;
		buf << source;
		buf >> copy;

		CHECK(source.arSite[1][0] == copy.arSite[1][0]);
	}
}
