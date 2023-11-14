#pragma once

#include "Unit.h"
#include "Bytes.hpp"
#include "Hero.hpp"

namespace binary {
	// TODO: save and load province name 

	inline std::ostream& operator<<(std::ostream& out, const dcProvince& province) {
		writeBytes(out, province);
		out << OutputOptional(province.pGuard) << OutputOptional(province.pGarrison);

		for (int i = 0; i < MAX_PROVINCE_SITE; i++) {
			out << OutputOptional(province.arSiteGuard[i]);
		}

		return out;
	}

	inline std::istream& operator>>(std::istream& in, dcProvince& province) {
		readBytes(in, province);
		auto guardBox = InputOptional(province.pGuard);
		auto garrisonBox = InputOptional(province.pGarrison);
		in >> guardBox >> garrisonBox;

		for (int i = 0; i < MAX_PROVINCE_SITE; i++) {
			auto box = InputOptional(province.arSiteGuard[i]);
			in >> box;
		}

		return in;
	}
}
