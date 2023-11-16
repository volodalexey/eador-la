#include "CurrentGameState.hpp"
#include "Unit.h"
#include "Mapper.h"
#include "MemSnapshot.hpp"
#include "Player.hpp"
#include "AsBinary.hpp"
#include "Bytes.hpp"
#include "Ritual.hpp"
#include "Rules.hpp"
#include "Game.hpp"
#include "Shop.hpp"
#include "Tutorial.hpp"
#include "Province.hpp"
#include <stdexcept>
#include <format>
#include <iostream>
#include <sstream>
#include <fstream>

#include "Optional.hpp"

namespace binary {
	void writeProvinces(std::ostream& out) {
		for (int i = 1; i <= MapSize; i++) {
			out << AsBinary(pFirstProvince[i].iNumber) << pFirstProvince[i];
		}
	}

	void writePlayers(std::ostream& out) {
		for (int PL = 1; PL <= CurGame.iMaxPlayers; PL++) {
			out << arPlayer[PL];
		}
		out << AsBinary(iMaxPureItem);
	}

	void writeShops(std::ostream& out) {
		out << AsBinary(iMaxShop);
		for (int i = 0; i < iMaxShop; i++) {
			out << pFirstShop[i];
		}
	}

	int count(const dcRitual* begin) {
		int count = 0;
		for (auto it = begin; it; it = it->pNext) count++;
		return count;
	}

	void writeRituals(std::ostream& out) {
		out << AsBinary(count(pRitual));
		for (auto it = pRitual; it; it = it->pNext) out << *it;
	}

	void SaveGame(std::ostream &out) {
		char mode; //0 - одиночная; 1 - кампания; 2 - кампания, астрал
		if (iGameMode == 1)              mode = 2;
		else if (CurGame.iGameType == 2) mode = 1;
		else                             mode = 0;
		if (mode == 2) throw std::runtime_error("Can't save campaign");

		char mark = 111;
		char version = 107;
		out << mark << version << mode << CurGame << WorldRules;

		writeProvinces(out);
		writePlayers(out);
		writeShops(out);
		writeRituals(out);

		out << Tutorial;
	}
}

namespace binary {
	void saveMapBitmaps(PACKFILE* out) {
		PALETTE palette;
		get_palette(palette);
		save_pcx_pf(out, bmpMap, palette);
		for (int i = 1; i <= MapSize; i++) {
			save_pcx_pf(out, pFirstFog[i].Pic, palette);
		}
	}

	void loadMapBitmaps(PACKFILE* out) {
		PALETTE buf;
		bmpMap = load_pcx_pf(out, buf);
		for (int i = 1; i <= MapSize; i++) {
			pFirstFog[i].Pic = load_pcx_pf(out, buf);
		}
	}

	void SaveMap(std::ostream& out) {
		writeBytes(out, MapSize);
		for (int i = 1; i <= MapSize; i++) {
			writeBytes(out, pFirstFog[i]);

			dcProvince& province = pFirstProvince[i];
			writeBytes(out, province);
			writeBytes(out, strlen(province.sName));
			writeBytes(out, province.sName, strlen(province.sName));
		}
		for (int i = 0; i < MAP_SIZE; i++) {
			Map_Border* border = Map[i];
			while (border) {
				writeBytes(out, i);
				writeBytes(out, *border);
				border = border->Next;
			}
		}
		writeBytes(out, -1);
	}

	Map_Border& newMapBorederLeaf(int index) {
		if (not Map[index]) Map[index] = new Map_Border;
		Map_Border* border = Map[index];
		while (border->Next) border = border->Next;
		border->Next = new Map_Border;
		return *border->Next;
	}

	void LoadMap(std::istream &in) {
		readBytes(in, MapSize);

		pFirstProvince = new dcProvince[MapSize + 1];
		pFirstFog = new clFog[MapSize + 1];
		pMoveMap = new int[MapSize + 1];
		pRunMap = new int[MapSize + 1];
		pMobilMap = new int[MapSize + 1];

		for (int i = 1; i <= MapSize; i++) {
			clFog fog = {};
			readBytes(in, fog);
			pFirstFog[fog.Num] = fog;

			dcProvince province = {};
			readBytes(in, province);
			std::size_t nameLen;
			readBytes(in, nameLen);
			province.sName = new char[nameLen + 1];
			readBytes(in, province.sName, nameLen + 1);
			pFirstProvince[province.iNumber] = province;
		}

		for (int i = 0; i < MAP_SIZE; i++) {
			Map[i] = NULL;
		}
		int Index;
		readBytes(in, Index);
		while (Index < 4000) {
			auto &border = newMapBorederLeaf(Index);
			readBytes(in, border);
			readBytes(in, Index);
		};
		pFirstProvince[0].sName = pFirstPhrase[327].str;
	}

	void FillObstracleMap() {
		for (long int i = 1; i <= MapSize; i++) {
			pFirstProvince[i].DrawPic(false);
		}
	}

	void readProvinces(std::istream& in) {
		for (int i = 1; i <= MapSize; i++) {
			int number = 0;
			auto numberBox = InputAsBinary(number);
			in >> numberBox >> pFirstProvince[number];
			pFirstProvince[number].Pic = nullptr;
			pFirstProvince[number].Obj = nullptr;
		}
	}

	void readPlayers(std::istream& in) {
		readBytes(in, arPlayer);
		for (int PL = 1; PL <= CurGame.iMaxPlayers; PL++) {
			in >> arPlayer[PL];
			// TODO: load profiles (old implementation)
			arPlayer[PL].MakeFlags();
		}
	}

	void readShops(std::istream& in) {
		auto box = InputAsBinary(iMaxShop);
		in >> box;
		pFirstShop = new dcShop[iMaxShop];
		for (int i = 0; i < iMaxShop; i++) {
			in >> pFirstShop[i];
		}
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

	void LoadGame(std::istream& in) {
		char mark, version, mode;
		in >> mark >> version >> mode >> CurGame >> WorldRules;

		readProvinces(in);
		readPlayers(in);
		readShops(in);
		readRituals(in);

		// TODO: call FillObstracleMap (old implementation)
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
	// TODO: Implement
}
