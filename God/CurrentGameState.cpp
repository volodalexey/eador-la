#include "CurrentGameState.hpp"
#include "Unit.h"
#include "Mapper.h"
#include <stdexcept>
#include <format>
#include <iostream>
#include <sstream>
#include <fstream>

namespace binary {
	std::ostream& operator<<(std::ostream& out, const dcPlayer& player);
	std::istream& operator>>(std::istream& in, dcPlayer& player);
}

#include "MemSnapshot.hpp"
#include "Fog.hpp"
#include "Shop.hpp"
#include "Province.hpp"
#include "Player.hpp"
#include "Bytes.hpp"
#include "Ritual.hpp"
#include "Rules.hpp"
#include "Game.hpp"
#include "Range.hpp"
#include "AsBinary.hpp"
#include "Optional.hpp"

namespace binary {
	int count(const dcRitual* begin) {
		int count = 0;
		for (auto it = begin; it; it = it->pNext) count++;
		return count;
	}

	void writeRituals(std::ostream& out) {
		out << AsBinary(count(pRitual));
		for (auto it = pRitual; it; it = it->pNext) out << *it;
	}

	void readRituals(std::istream& in) {
		int count;
		auto box = InputAsBinary(count);
		in >> box;
		pRitual = nullptr;
		for (int i = 0; i < count; i++) {
			dcRitual* ritual = new dcRitual();
			in >> *ritual;
			ritual->pNext = pRitual;
			pRitual = ritual;
		}
	}
}

namespace binary {
	void SaveGame(std::ostream& out) {
		char mode; //0 - одиночная; 1 - кампания; 2 - кампания, астрал
		if (iGameMode == 1)              mode = 2;
		else if (CurGame.iGameType == 2) mode = 1;
		else                             mode = 0;
		if (mode == 2) throw std::runtime_error("Can't save campaign");

		char mark = 111;
		char version = 107;
		out << mark
			<< version
			<< mode
			<< CurGame
			<< WorldRules
			<< AsBinary(iMaxShop)
			<< AsBinary(iMaxPureItem)
			<< ArrayInclusiveEnd(pFirstProvince, pFirstProvince + MapSize)
			<< ArrayInclusiveEnd(pFirstShop, pFirstShop + iMaxShop - 1)
			<< Range(&arPlayer[1], &arPlayer[CurGame.iMaxPlayers]);

		writeRituals(out);
	}

	void LoadGame(std::istream& in) {
		char mark, version, mode;
		in >> mark
			>> version
			>> mode
			>> CurGame
			>> WorldRules
			>> InputAsBinary(iMaxShop)
			>> InputAsBinary(iMaxPureItem)
			>> InputArrayInclusiveEnd(pFirstProvince)
			>> InputArrayInclusiveEnd(pFirstShop)
			>> Range(&arPlayer[1], &arPlayer[CurGame.iMaxPlayers]);

		readRituals(in);

		// TODO: call FillObstracleMap (old implementation)
	}
}

namespace binary {
	Map_Border& newMapBorderLeaf(int index) {
		if (not Map[index]) Map[index] = new Map_Border{ .Next = nullptr };
		Map_Border* border = Map[index];
		while (border->Next) border = border->Next;
		border->Next = new Map_Border;
		return *border->Next;
	}

	void SaveMap(std::ostream& out) {
		out << AsBinary(MapSize)
			<< ArrayInclusiveEnd(pFirstFog, pFirstFog + MapSize)
			<< ArrayInclusiveEnd(pFirstProvince, pFirstProvince + MapSize);

		for (int i = 0; i < MAP_SIZE; i++) {
			Map_Border* border = Map[i];
			while (border) {
				out << AsBinary(i);
				writeBytes(out, *border);
				border = border->Next;
			}
		}
	}

	void LoadMap(std::istream& in) {
		in >> InputAsBinary(MapSize)
			>> InputArrayInclusiveEnd(pFirstFog)
			>> InputArrayInclusiveEnd(pFirstProvince);

		pMoveMap = new int[MapSize + 1];
		pRunMap = new int[MapSize + 1];
		pMobilMap = new int[MapSize + 1];

		for (int i = 0; i < MAP_SIZE; i++) {
			Map[i] = nullptr;
		}
		int index = 0;
		in >> InputAsBinary(index);
		while (index < 4000) {
			auto& border = newMapBorderLeaf(index);
			readBytes(in, border);
			in >> InputAsBinary(index);
		};
	}

	void FillObstracleMap() {
		for (long int i = 1; i <= MapSize; i++) {
			pFirstProvince[i].DrawPic(false);
		}
	}
}

using namespace binary;

std::shared_ptr<Snapshot> CurrentGameState::snapshot() const
{
	std::stringstream ss(std::ios::out | std::ios::binary);
	SaveGame(ss);
	return std::make_shared<MemSnapshot>(ss.str());
}

void CurrentGameState::applay(std::shared_ptr<Snapshot> snapshot)
{
	std::stringstream buf{ snapshot->bytes() };
	LoadGame(buf);
}
