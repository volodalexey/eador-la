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
			auto box = InputSquad(hero.pSquad);
			return in >> box;
		}
		catch (...) {
			std::throw_with_nested(std::runtime_error("Failed to read Hero"));
		}
	}
}
