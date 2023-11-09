#include "CurrentGameState.hpp"
#include "Unit.h"
#include <stdexcept>
#include <format>
#include <iostream>
#include <sstream>
#include <fstream>
#include "MemSnapshot.hpp"

namespace {

	#define BOOGY_WRITE(x) out.write(reinterpret_cast<const char*>(x), sizeof(*(x)))

	const char EXISTS = 1;
	const char NOT_EXISTS = 0;

	std::ostream& operator<<(std::ostream& out, const dcHero& hero) {
		out.write(reinterpret_cast<const char*>(&hero), sizeof(hero));
		for (int i = 0; i < MAX_SQUAD; i++)
		{
			if (hero.pSquad[i])
			{
				out << EXISTS;
				out.write(reinterpret_cast<char*>(hero.pSquad[i]), sizeof(*hero.pSquad[i]));
			}
			else
			{
				out << NOT_EXISTS;
			};
		};
		return out;
	}

	void writeProvinces(std::ostream& out) {
		for (int i = 1; i <= MapSize; i++)
		{
			dcProvince* PR = (pFirstProvince + i);
			out.write(reinterpret_cast<char*>(PR), sizeof(*PR));
			if (PR->pGuard) out << EXISTS << *PR->pGuard;
			else out << NOT_EXISTS;
			if (PR->pGarrison) out << EXISTS << *PR->pGarrison;
			else out << NOT_EXISTS;

			for (int k = 0; k < MAX_PROVINCE_SITE; k++)
			{
				if (PR->arSiteGuard[k])
				{
					out << EXISTS;
					out.write(reinterpret_cast<char*>(PR->arSiteGuard[k]), sizeof(*PR->arSiteGuard[k]));
					for (int j = 0; j < MAX_SQUAD; j++)
					{
						if (PR->arSiteGuard[k]->pSquad[j])
						{
							out << EXISTS;
							out.write(reinterpret_cast<char*>(PR->arSiteGuard[k]->pSquad[j]), sizeof(*PR->arSiteGuard[k]->pSquad[j]));
						}
						else
						{
							out << NOT_EXISTS;
						};
					};
				}
				else
				{
					out << NOT_EXISTS;
				};
			};
		};
	}

	void writePlayers(std::ostream& out) {
		out.write(reinterpret_cast<char*>(&arPlayer), sizeof(arPlayer));
		for (int PL = 1; PL <= CurGame.iMaxPlayers; PL++)
		{
			for (int i = 0; i < MAX_HERO; i++)
			{
				if (arPlayer[PL].arHero[i]) out << EXISTS << *arPlayer[PL].arHero[i];
				else out << NOT_EXISTS;
			};

			BOOGY_WRITE(&iMaxDefender);
			out.write(reinterpret_cast<const char*>(arPlayer[PL].pDefender), sizeof(*arPlayer[PL].pDefender) * (iMaxDefender + 1));

			BOOGY_WRITE(&iMaxOuterBld);
			out.write(reinterpret_cast<const char*>(arPlayer[PL].pOuterBld), sizeof(*arPlayer[PL].pOuterBld) * (iMaxOuterBld + 1));

			BOOGY_WRITE(&iMaxRitual);
			for (int i = 0; i <= iMaxRitual; i++)
			{
				out.write(reinterpret_cast<const char*>(&arPlayer[PL].pRitual[i]), sizeof(*arPlayer[PL].pRitual));
				out.write(reinterpret_cast<const char*>(&arPlayer[PL].pRitualCD[i]), sizeof(*arPlayer[PL].pRitualCD));
			};
			
			out.write(reinterpret_cast<const char*>(&arPlayer[PL].pFog[1]), sizeof(*arPlayer[PL].pFog) * MapSize);

			BOOGY_WRITE(&iMaxEvent);
			for (int i = 1; i <= iMaxEvent; i++)
			{
				out.write(reinterpret_cast<const char*>(&arPlayer[PL].pStory[i]), sizeof(*arPlayer[PL].pStory));
				out.write(reinterpret_cast<const char*>(&arPlayer[PL].pPlace[i]), sizeof(*arPlayer[PL].pPlace));
			};
		};
		BOOGY_WRITE(&iMaxPureItem);
	}

	void writeRituals(std::ostream& out) {
		int Count = 0;
		dcRitual* RR = pRitual;
		while (RR)
		{
			RR = RR->pNext;
			Count++;
		};
		BOOGY_WRITE(&Count);
		RR = pRitual;
		while (RR)
		{
			out.write(reinterpret_cast<char*>(RR), sizeof(*RR));
			RR = RR->pNext;
		};
	}

	void writeWorldShards(std::ostream& out) {
		dcShard* SH = pFirstShard;
		for (int i = 0; i < Campaign.iMaxShards; i++)
		{
			out.write(reinterpret_cast<char*>(SH), sizeof(*SH));
			SH = SH->pNext;
		};
	}

	void SaveGame(std::ostream &out)
	{
		out << (char)111 << (char)107; // что-то непонятное и потом версия

		char MODE; //0 - одиночная; 1 - кампания; 2 - кампания, астрал
		if (iGameMode == 1)              MODE = 2;
		else if (CurGame.iGameType == 2) MODE = 1;
		else                             MODE = 0;
		out << MODE;

		if (MODE < 2)
		{
			//Общие данные
			out.write(reinterpret_cast<char*>(&CurGame), sizeof(CurGame));
			out.write(reinterpret_cast<char*>(&WorldRules), sizeof(WorldRules));

			writeProvinces(out);
			writePlayers(out);

			//Магазины
			BOOGY_WRITE(&iMaxShop);
			out.write(reinterpret_cast<char*>(pFirstShop), sizeof(*pFirstShop) * iMaxShop);

			writeRituals(out);
		};
		if (MODE > 0)
		{
			//Сохранение данных кампании
			out.write(reinterpret_cast<char*>(&Campaign), sizeof(Campaign));
			writeWorldShards(out);
		};
		out.write(reinterpret_cast<char*>(&Tutorial), sizeof(Tutorial));
	}
}

namespace {
	void LoadMap() {}

	void FillObstracleMap(int) {}

	void UnloadGame() {}

	template <class T>
	void readBytes(std::istream& in, T& x) {
		in.read(reinterpret_cast<char*>(&x), sizeof(x));
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

	bool LoadGame(std::istream& in)
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
			LoadMap();

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

			FillObstracleMap(19200);
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
