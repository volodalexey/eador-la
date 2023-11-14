#include "CurrentGameState.hpp"
#include "Unit.h"
#include <stdexcept>
#include <format>
#include <iostream>
#include <sstream>
#include <fstream>
#include "MemSnapshot.hpp"
#include "Mapper.h"

namespace {
	std::ostream& operator<<(std::ostream& out, const dcUnit& unit);
	std::ostream& operator<<(std::ostream& out, const dcHero& hero);

	template <class T>
	struct OutputOptional {
		enum Existense : char { EXISTS = 1, NOT_EXISTS = 0 };

		T const* const obj;

		friend std::ostream& operator<<(std::ostream& out, const OutputOptional<T>& x) {
			if (not x.obj) return out << NOT_EXISTS;
			return out << EXISTS << *x.obj;
		}
	};

	struct OutputSquad {
		dcUnit const* const* const units;

		friend std::ostream& operator<<(std::ostream& out, const OutputSquad& squad) {
			auto x = squad.units[1];
			for (int i = 0; i < MAX_SQUAD; i++) out << OutputOptional(squad.units[i]);
			return out;
		}
	};

	template <class T>
	void writeBytes(std::ostream& out, const T& x) {
		out.write(reinterpret_cast<const char*>(&x), sizeof(x));
	}

	void writeBytes(std::ostream& out, const char* x, std::size_t len) {
		out.write(x, len);
	}

	std::ostream& operator<<(std::ostream& out, const dcUnit& unit) {
		writeBytes(out, unit);
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const dcHero& hero) {
		writeBytes(out, hero);
		return out << OutputSquad(hero.pSquad);
	}

	std::ostream& operator<<(std::ostream& out, const dcProvince& province) {
		writeBytes(out, province);
		out << OutputOptional(province.pGuard) << OutputOptional(province.pGarrison);

		for (int i = 0; i < MAX_PROVINCE_SITE; i++) {
			out << OutputOptional(province.arSiteGuard[i]);
		}

		return out;
	}

	std::ostream& operator<<(std::ostream& out, const dcPlayer& player) {
		for (int i = 0; i < MAX_HERO; i++) {
			out << OutputOptional(player.arHero[i]);
		}

		writeBytes(out, iMaxDefender);
		for (int i = 0; i <= iMaxDefender; i++) {
			writeBytes(out, player.pDefender[i]);
		}

		writeBytes(out, iMaxOuterBld);
		for (int i = 0; i <= iMaxOuterBld; i++) {
			writeBytes(out, player.pOuterBld[i]);
		}

		writeBytes(out, iMaxRitual);
		for (int i = 0; i <= iMaxRitual; i++) {
			writeBytes(out, player.pRitual[i]);
			writeBytes(out, player.pRitualCD[i]);
		}

		for (int i = 1; i <= MapSize; i++) {
			writeBytes(out, player.pFog[i]);
		}

		writeBytes(out, iMaxEvent);
		for (int i = 1; i <= iMaxEvent; i++) {
			writeBytes(out, player.pStory[i]);
			writeBytes(out, player.pPlace[i]);
		}

		return out;
	}

	std::ostream& operator<<(std::ostream& out, const dcRitual& ritual) {
		writeBytes(out, ritual);
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const dcGame& game) {
		writeBytes(out, game);
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const dcWorldRules& rules) {
		writeBytes(out, rules);
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const dcShop& shop) {
		writeBytes(out, shop);
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const dcTutorial& tutorial) {
		writeBytes(out, tutorial);
		return out;
	}

	void writeProvinces(std::ostream& out) {
		for (int i = 1; i <= MapSize; i++) {
			out << pFirstProvince[i];
		}
	}

	void writePlayers(std::ostream& out) {
		writeBytes(out, arPlayer);
		for (int PL = 1; PL <= CurGame.iMaxPlayers; PL++) {
			out << arPlayer[PL];
		}
		writeBytes(out, iMaxPureItem);
	}

	void writeShops(std::ostream& out) {
		writeBytes(out, iMaxShop);
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
		writeBytes(out, count(pRitual));
		for (auto it = pRitual; it; it = it->pNext) out << *it;
	}

	void SaveGame(std::ostream &out)
	{
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

namespace {

	template <class T>
	void readBytes(std::istream& in, T& x) {
		in.read(reinterpret_cast<char*>(&x), sizeof(x));
	}

	void readBytes(std::istream& in, char* x, std::size_t len) {
		in.read(x, len);
	}

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

	void SaveMap(std::ostream& out)
	{
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

	std::istream& operator>>(std::istream& in, dcHero& x) {
		readBytes(in, x);
		for (int j = 0; j < MAX_SQUAD; j++)
		{
			if (in.get()) {
				x.pSquad[j] = nullptr;
				continue;
			}

			dcUnit unit = {};
			readBytes(in, unit);
			unit.pHero = nullptr;

			if (not (0 <= unit.GetUnitType() and unit.GetUnitType() < iMaxUnitType)) {
				unit.SetUnitType(1);
			}

			x.pSquad[j] = new dcUnit(unit);
		}
		return in;
	}

	void readProvinces(std::istream& in) {
		for (int i = 1; i <= MapSize; i++) {
			dcProvince PR = {};
			readBytes(in, PR);
			PR.sName = pFirstProvince[PR.iNumber].sName;
			PR.Pic = NULL;
			PR.Obj = NULL;
			if (in.get()) {
				PR.pGuard = new dcHero;
				in >> *PR.pGuard;
			}
			else {
				PR.pGuard = NULL;
			}

			if (in.get()) {
				PR.pGarrison = new dcHero;
				in >> *PR.pGarrison;
			}
			else {
				PR.pGarrison = NULL;
			}

			for (int k = 0; k < MAX_PROVINCE_SITE; k++) {
				if (in.get()) {
					PR.arSiteGuard[k] = new dcHero;
					in >> *PR.arSiteGuard[k];
				}
				else {
					PR.arSiteGuard[k] = NULL;
				}
			}
			pFirstProvince[PR.iNumber] = PR;
		}
	}

	void readPlayers(std::istream& in) {
		//Игроки
		readBytes(in, arPlayer);
		for (int PL = 1; PL <= CurGame.iMaxPlayers; PL++) {
			auto& player = arPlayer[PL];
			
			player.pDefender = new int[iMaxDefender + 1];
			player.pOuterBld = new int[iMaxOuterBld + 1];
			player.pRitual = new int[iMaxRitual + 1];
			player.pRitualCD = new int[iMaxRitual + 1];
			player.pFog = new int[MapSize + 1];
			player.pStory = new int[iMaxEvent + 1];
			player.pPlace = new int[iMaxEvent + 1];
			player.pEvent = nullptr;
			for (int i = 0; i < MAX_HERO; i++) {
				if (in.get()) {
					dcHero hero = {};
					in >> hero;

					for (int j = 1; j <= INVENTORY_SIZE; j++) {
						auto& item = hero.arInventory[j];
						if (not (0 <= item[0] and item[0] <= iMaxItem)) {
							item[0] = 0;
							item[1] = 0;
						}
					}

					if (auto exp = hero.ExpForLevel(hero.iLevel); hero.iExp < exp) {
						hero.iExp = exp;
					}

					hero.pstName = &pName[hero.ipClass1 - 1][hero.ipName];
					if (hero.ipPortrait < 0) hero.ipPortrait = 1;
					hero.Pic = bmpPortrait[hero.ipPortrait];
					hero.smPic = bmpSmallPort[hero.ipPortrait];
					hero.pProvince = &pFirstProvince[hero.ipProvince];

					player.arHero[i] = new dcHero(hero);
					if (auto unit = player.arHero[i]->pSquad[0]; unit)
						unit->pHero = player.arHero[i];
				}
				else {
					player.arHero[i] = nullptr;
				}
			}

			int maxDefenders;
			readBytes(in, maxDefenders);
			for (int i = 0; i <= std::min(maxDefenders, iMaxDefender); i++) {
				readBytes(in, player.pDefender[i]);
			};
			for (int i = 0; i < (iMaxDefender - maxDefenders); i++) {
				int stub;
				readBytes(in, stub);
			}
			for (int i = maxDefenders + 1; i <= iMaxDefender; i++) {
				player.pDefender[i] = {};
			}
			
			int maxOuterBld;
			readBytes(in, maxOuterBld);
			for (int i = 0; i <= std::min(maxOuterBld, iMaxOuterBld); i++) {
				readBytes(in, player.pOuterBld[i]);
			};
			for (int i = 0; i < (iMaxOuterBld - maxOuterBld); i++) {
				int stub;
				readBytes(in, stub);
			}
			for (int i = maxOuterBld + 1; i <= iMaxOuterBld; i++) {
				player.pOuterBld[i] = {};
			}

			int maxRituals;
			readBytes(in, maxRituals);
			for (int i = 0; i <= std::min(maxRituals, iMaxRitual); i++) {
				readBytes(in, player.pRitual[i]);
				readBytes(in, player.pRitualCD[i]);
			}
			for (int i = 0; i < (iMaxRitual - maxRituals); i++) {
				int stub;
				readBytes(in, stub);
				readBytes(in, stub);
			}
			for (int i = maxRituals + 1; i <= iMaxRitual; i++) {
				player.pRitual[i] = 0;
				player.pRitualCD[i] = 0;
			}

			//Туман
			player.pFog[0] = 0;
			for (int i = 1; i <= MapSize; i++) {
				readBytes(in, player.pFog[i]);
			};

			//События
			int maxEvents;
			readBytes(in, maxEvents);
			for (int i = 0; i <= std::min(maxEvents, iMaxEvent); i++) {
				readBytes(in, player.pStory[i]);
				readBytes(in, player.pPlace[i]);
			}
			for (int i = 0; i < (iMaxEvent - maxEvents); i++) {
				int stub;
				readBytes(in, stub);
				readBytes(in, stub);
			}
			for (int i = maxEvents + 1; i <= iMaxEvent; i++) {
				player.pStory[i] = -1;
				player.pPlace[i] = 0;
			}

			if (player.Control == 1)
			{
				PACKFILE* PrfFile;
				char stName[150];
				sprintf(stName, "save\\player%d.prf", player.ipProfile);
				PrfFile = pack_fopen(stName, "rp");
				pack_fread(&arHumans[player.iColor], sizeof arHumans[player.iColor], PrfFile);
				pack_fclose(PrfFile);
			};

			player.MakeFlags();
		}
	}

	void LoadGame(std::istream& in)
	{
		bTimeShift = false;
		dcOldHero* WrkHero = new dcOldHero;
		char sFile[150];

		in.get(); // some marker
		char Version = in.get();
		char MODE = in.get();

		if (MODE < 2)
		{
			//Общие данные
			readBytes(in, CurGame);
			readBytes(in, WorldRules);

			readProvinces(in);
			readPlayers(in);

			//Магазины
			readBytes(in, iMaxShop);
			pFirstShop = new dcShop[iMaxShop];
			for (int i = 0; i < iMaxShop; i++) {
				readBytes(in, pFirstShop[i]);
			}

			//Действующие ритуалы
			int Count;
			readBytes(in, Count);
			pRitual = NULL;
			for (int i = 0; i < Count; i++) {
				dcRitual* RR = new dcRitual();
				readBytes(in, *RR);
				RR->pNext = pRitual;
				pRitual = RR;
			}

			FillObstracleMap();
		}
		else {
			CurGame.iCurrentTurn = -1;
			CurGame.iGameType = 2;
		}
	}
}

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
