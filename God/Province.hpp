#pragma once

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcProvince& province);
	inline std::istream& operator>>(std::istream& in, dcProvince& province);
}

#include "Unit.h"
#include "Bytes.hpp"
#include "Hero.hpp"
#include "ArrayInclusiveEnd.hpp"
#include "GuardPrefix.hpp"
#include <cstring>

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcProvince& province) {
		out << GuardPrefix::named<dcProvince>();
		writeBytes(out, province);

		out << OutputOptional(province.pGuard)
			<< OutputOptional(province.pGarrison)
			<< ArrayInclusiveEnd(province.sName, province.sName + std::strlen(province.sName));

		for (int i = 0; i < MAX_PROVINCE_SITE; i++) {
			out << OutputOptional(province.arSiteGuard[i]);
		}

		return out;
	}

	inline std::istream& operator>>(std::istream& in, dcProvince& province) {
		try {
			in >> GuardPrefix::named<dcProvince>();
			readBytes(in, province);
			province.Pic = nullptr;
			province.Obj = nullptr;

			try {
				in >> InputOptional(province.pGuard);
			}
			catch (...) {
				std::throw_with_nested(std::runtime_error("Failed to read guard of province"));
			}

			try {
				in >> InputOptional(province.pGarrison);
			}
			catch (...) {
				std::throw_with_nested(std::runtime_error("Failed to read garrison of province"));
			}


			in >> InputArrayInclusiveEnd(province.sName);

			for (int i = 0; i < MAX_PROVINCE_SITE; i++) {
				in >> InputOptional(province.arSiteGuard[i]);
			}

			return in;
		}
		catch (...) {
			std::throw_with_nested(std::runtime_error("Failed to read province"));
		}
	}
}
