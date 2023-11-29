#pragma once

namespace binary {
	std::ostream& operator<<(std::ostream& out, const dcHero& hero);
	std::istream& operator>>(std::istream& in, dcHero& hero);
}

#include "Squad.hpp"
#include "Bytes.hpp"
#include "GuardPrefix.hpp"

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcHero& hero) {
		out << GuardPrefix::named<dcHero>();
		writeBytes(out, hero);
		return out << OutputSquad(hero.pSquad);
	}

	inline std::istream& operator>>(std::istream& in, dcHero& hero) {
		try {
			in >> GuardPrefix::named<dcHero>();
			readBytes(in, hero);

			hero.pstName = &pName[std::max(0, hero.ipClass1 - 1)][std::max(0, hero.ipName)];
			hero.Pic = bmpPortrait[std::max(0, hero.ipPortrait)];
			hero.smPic = bmpSmallPort[std::max(0, hero.ipPortrait)];
			hero.pProvince = &pFirstProvince[std::max(0, hero.ipProvince)];

			return in >> InputSquad(hero);
		}
		catch (...) {
			std::throw_with_nested(std::runtime_error("Failed to read Hero"));
		}
	}
}
