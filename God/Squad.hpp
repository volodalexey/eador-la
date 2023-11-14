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
		dcUnit** units;

		friend std::istream& operator>>(std::istream& in, InputSquad& squad) {
			auto x = squad.units[1];
			for (int i = 0; i < MAX_SQUAD; i++) {
				auto box = InputOptional(squad.units[i]);
				in >> box;
			}
			return in;
		}
	};
}
