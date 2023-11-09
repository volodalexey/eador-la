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
