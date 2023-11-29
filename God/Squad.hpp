#pragma once

#include "Unit.hpp"
#include "Optional.hpp"
#include <iostream>

namespace binary {
	struct OutputSquad {
		dcUnit const* const* const units;

		friend std::ostream& operator<<(std::ostream& out, const OutputSquad& squad) {
			auto x = squad.units[1];
			for (int i = 0; i < MAX_SQUAD; i++) out << OutputOptional(squad.units[i]);
			return out;
		}
	};

	struct InputSquad {
		dcHero& hero;

		friend std::istream& operator>>(std::istream& in, InputSquad& squad) {
			for (int i = 0; i < MAX_SQUAD; i++) {
				auto& unit = squad.hero.pSquad[i];
				in >> InputOptional(unit);
				if (unit) unit->pHero = &squad.hero;
			}
			return in;
		}

		friend std::istream& operator>>(std::istream& in, InputSquad&& squad) {
			return in >> squad;
		}
	};
}
