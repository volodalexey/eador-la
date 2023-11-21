#pragma once

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcProvince& province);
	inline std::istream& operator>>(std::istream& in, dcProvince& province);
}

#include "Unit.h"
#include "Bytes.hpp"
#include "Hero.hpp"
#include "ArrayInclusiveEnd.hpp"

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcProvince& province) {
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
		readBytes(in, province);
		province.Pic = nullptr;
		province.Obj = nullptr;

		in >> InputOptional(province.pGuard)
			>> InputOptional(province.pGarrison)
			>> InputArrayInclusiveEnd(province.sName);

		for (int i = 0; i < MAX_PROVINCE_SITE; i++) {
			in >> InputOptional(province.arSiteGuard[i]);
		}

		return in;
	}
}
