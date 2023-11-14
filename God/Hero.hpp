#pragma once

namespace binary {
	std::ostream& operator<<(std::ostream& out, const dcHero& hero);
	std::istream& operator>>(std::istream& in, dcHero& hero);
}

#include "Squad.hpp"
#include "Bytes.hpp"

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcHero& hero) {
		writeBytes(out, hero);
		return out << OutputSquad(hero.pSquad);
	}

	inline std::istream& operator>>(std::istream& in, dcHero& hero) {
		readBytes(in, hero);
		auto box = InputSquad(hero.pSquad);
		return in >> box;
	}
}
