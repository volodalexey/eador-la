#ifndef USED_MAPPER
#define USED_MAPPER

#include "Windower.h"

struct Map_Border
{
	int x;
	int Prov[3];
	Map_Border * Next;
};

class clDeep
{
public:
	int x, y, Deep;
	clDeep *pNext;
	clDeep(int sx, int sy, int sDeep, clDeep *Next);
	~clDeep();
};

extern Map_Border *Map[MAP_SIZE];


void GenerateMap(clMap Map, int WorldRul);
//Type 1 - окружена€ водой суша, 2 - случайные мор€ и озера, Isles - процент озер/островов, Dissolve - чем выше, тем меньшими кусочками будут создаватьс€ участки территории
//¬ода указываетс€ в процениах от общей площади, холмы, леса и болота - в процентах от площади суши
//NeutralPower - ограничение силы нейтралов
void MainScreen(int Player);
void EndMainTurn();
void BeginMainTurn(int Player);
void MakeTurnAI(int Player, int Progress);
int GetDiploAnswerAI(int Player, int Writer);
void SendGreetingsAI(int Player, int Victim);
void SendAnswerAI(int Player, int Victim);
void SaveMap();
void LoadMap();	//«агрузить карту
void UnloadMap();
void SaveGame(int Progress);
bool LoadGame(int iModd); //1 - обычна€, 2 - кампани€, 3 - HotSeat, 4 - LAN, 5 - Inet
bool NewGame(int iSeed, int GameType);
void NewCampaign();
void UnloadGame();
int Astral();
void FillObstracleMap(int Progress);
int RunGame(bool bTurner);
void ShowWallpaper();

#endif