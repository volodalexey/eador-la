#pragma once

#include "Unit.h"
#include "ArrayInclusiveEnd.hpp"
#include "Hero.hpp"
#include "Optional.hpp"

namespace binary {
	inline std::ostream& operator<<(std::ostream& out, const dcPlayer& player) {
		writeBytes(out, player);

		for (int i = 0; i < MAX_HERO; i++) {
			out << OutputOptional(player.arHero[i]);
		}

		out << ArrayInclusiveEnd(player.pDefender, iMaxDefender)
			<< ArrayInclusiveEnd(player.pOuterBld, iMaxOuterBld)
			<< ArrayInclusiveEnd(player.pRitual, iMaxRitual)
			<< ArrayInclusiveEnd(player.pRitualCD, iMaxRitual)
			<< ArrayInclusiveEnd(player.pFog, MapSize)
			<< ArrayInclusiveEnd(player.pStory, iMaxEvent)
			<< ArrayInclusiveEnd(player.pPlace, iMaxEvent);

		return out;
	}

	inline std::istream& operator>>(std::istream& in, dcPlayer& player) {
		readBytes(in, player);

		for (int i = 0; i < MAX_HERO; i++) {
			auto box = InputOptional(player.arHero[i]);
			in >> box;
		}

		auto defenders = InputArrayInclusiveEnd(player.pDefender);
		auto outerBuilding = InputArrayInclusiveEnd(player.pOuterBld);
		auto rituals = InputArrayInclusiveEnd(player.pRitual);
		auto ritualsCD = InputArrayInclusiveEnd(player.pRitualCD);
		auto fog = InputArrayInclusiveEnd(player.pFog);
		auto stories = InputArrayInclusiveEnd(player.pStory);
		auto places = InputArrayInclusiveEnd(player.pPlace);

		return in >> defenders >> outerBuilding >> rituals >> ritualsCD >> fog >> stories >> places;
	}
}