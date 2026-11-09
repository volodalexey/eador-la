#include "Mapper.h"

//const int BAR_SIZE=5; //Количество позиций в окне строительства
//int arBuild[BAR_SIZE];
//int arBuildNum[BAR_SIZE];
dcWorldRules WorldRules(0);
Map_Border *Map[MAP_SIZE];
BITMAP *bmpMap=NULL;
dcProvince * pFirstProvince;
int MapSize;
//BITMAP *bmpHeroDoll[4];
//BITMAP *bmpHeroDollS[4];
//BITMAP *bmpFlag[16];
//BITMAP *bmpHerb[16];
//BITMAP *bmpHeroSelect;
//BITMAP *bmpOuterBld[MAX_OUTBLD_PIC];
//BITMAP *bmpPath[4];
//int MovePlayer, WalkType, WalkSpeed, MinLength;
int *pMoveMap;
int *pRunMap;
int *pMobilMap;
//clPath *GoPath, *P1, *P2, *P3;

clMap::clMap()
{
	Players=4;
	Difficulty=0;
	X=10;
	Y=10;
	SetDefault();
};

clMap::~clMap()
{};

void clMap::SetDefault()
{	
	Type=2;
	Water=25;
	Hill=25;
	Forest=20;
	Swamp=20;
	Isles=10;
	Dissolve=30;
	Neutrals=20;
	Resource=2;
	Site=2;
	Population=2;
};

bool WillGo1;
dcProvince *PR;

//class clLandscape
//{
//public:
//	int ipIndex,
//		iLayer,
//		x,y;
//	clLandscape * pNext;
//	clLandscape(int Index, int Layer, int sx, int sy, clLandscape * Next);
//};

//clLandscape::clLandscape(int Index, int Layer, int sx, int sy, clLandscape *Next)
//{
//	ipIndex=Index;
//	iLayer=Layer;
//	x=sx;
//	y=sy;
//	pNext=Next;
//};

int MakeMap (int Prov, int Weight)
{	
	Weight++;
	if ((*(pMoveMap+Prov)<0)||(*(pMoveMap+Prov)>Weight))
	{*(pMoveMap+Prov)=Weight;};
	
	for (int i=0; i<MAX_NEIGHBOUR; i++)
	{
		if ((pFirstProvince+Prov)->arNeighbour[i]>0)
		{			
			PR=(pFirstProvince+(pFirstProvince+Prov)->arNeighbour[i]);
			WillGo1=true;
			//море
			if ((PR->ipTerrain==0)||(PR->ipTerrain==7))
			{WillGo1=false;};			
			//уже достигнута за более короткий путь
			if (((*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])>=0))&&
				(*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])<=Weight+1))
			{WillGo1=false;};
			if (WillGo1)
			{			
				MakeMap(PR->iNumber,Weight);				
			};				
		};
	};	
	return 0;
};

int iFloodType, iFloodDeep, iFloodMax, iFloodSuit;
int Flooder(int Province, int Poss, int Step)
{
	dcProvince * PR=(pFirstProvince+Province);
	if (PR->ipTerrain==iFloodSuit)
	{
		PR->ipTerrain=iFloodType;
		Step++;
	};
	if (Step>=iFloodMax)
	{return Step;};

	for (int i=0; i<MAX_NEIGHBOUR; i++)
	{
		if ((PR->arNeighbour[i]>0)&&(Random(iFloodDeep)<Poss)&&((pFirstProvince+PR->arNeighbour[i])->ipTerrain==iFloodSuit))
		{
			Step=Flooder(PR->arNeighbour[i],Poss-1,Step);
			if (Step>=iFloodMax)
			{return Step;};
		};
	};
	return Step;
};

//Возвращает номер провинции, в которой находится указанная точка
int GetProvinceIndex(int x, int y)
{
	if ((y<0)||(y>MAP_SIZE)||(x<0))
	{return 0;};
	int Left=-5000, Right=5000;
	int LNode=1, RNode=1;
	Map_Border *BrLeft=NULL, *BrRight=NULL, *Cr;
	Cr=Map[y];
	while (Cr)
	{	
		if (x-Cr->x>0)
		{
			if (x-Cr->x<x-Left)
			{
				if ((BrLeft)&&(BrLeft->Prov[2]>0)&&(abs(Left-Cr->x)==LNode))
				{
					LNode++;
				}
				else
				{
					Left=Cr->x;
					BrLeft=Cr;
					LNode=1;
				};
			};
		}
		else
		{
			if (Cr->x-x<Right-x)
			{
				if ((BrRight)&&(BrRight->Prov[2]>0)&&(abs(Right-Cr->x)==RNode))
				{
					RNode++;
				}
				else
				{
					Right=Cr->x;
					BrRight=Cr;
					RNode=1;
				};
			};
		};	
		Cr=Cr->Next;
	};
	int Result=0;
//	rectfill(screen,1000,0,1010,10,makecol(0,0,0));
	if ((BrRight)&&(BrLeft))
	{
//		rectfill(screen,1000,0,1010,10,makecol(200,0,0));
		for (int i=0; i<3; i++)
		{
			for (int j=0; j<3; j++)
			{
				if ((BrRight->Prov[i]==BrLeft->Prov[j])&&(BrRight->Prov[i]>0))
				{Result=BrRight->Prov[i];};
			};
		};
	};

	if ((Result<0)||(Result>CurGame.Map.X*CurGame.Map.Y))
	{
		Result=0;
		FileLog("!!Wrong province index!!");
	}

	/*if (key[KEY_SPACE])
	{
		rectfill(screen,0,720,1000,740,COLOR_1);
		textprintf_ex(screen,Font1,100,720,COLOR_2,-1,"X: %d, Y: %d, Соседи: (%d, %d, %d) (%d, %d, %d)",x,y,
			BrLeft->Prov[0],BrLeft->Prov[1],BrLeft->Prov[2],
			BrRight->Prov[0],BrRight->Prov[1],BrRight->Prov[2]);

	};*/

	return Result;
};

BITMAP *MakeObstracles(BITMAP * Pic, int x, int y)
{
	BITMAP *bmpNew=create_bitmap(Pic->w,Pic->h);
	blit(Pic,bmpNew,0,0,0,0,Pic->w,Pic->h);
	int Arr[500];
	bool bEmpty=true;
	for (int i=0; i<Pic->w; i++)
	{
		Arr[i]=0;
		for (int j=Pic->h-1; j>=0; j--)
		{
			if (getpixel(Pic,i,j)!=makecol(255,0,255))
			{
				Arr[i]=j;
				break;
			};
		};
	};	
	for (int j=y; j<y+Pic->h+40; j++)
	{
		for (int i=x-30; i<x+Pic->w+30; i++)
		{
			int PR=GetProvinceIndex(i,j), PR1;			
			int Poss=0;
			if ((PR>0)&&((pFirstProvince+PR)->ipTerrain!=3)&&(arTerrLayer[(pFirstProvince+PR)->ipTerrain][2]>0)&&((Random(20000,i,j,CurGame.iSeed,2)<arTerrLayer[(pFirstProvince+PR)->ipTerrain][2])||((x==(pFirstProvince+PR)->x)&&(y==(pFirstProvince+PR)->y)&&(((pFirstProvince+PR)->ipTerrain==HILL_ID)||((pFirstProvince+PR)->ipTerrain==MOUNTAIN_ID)))))
			{				
				ClearRoll();
				int Counter=0;
				for (int z=1; z<=iMaxMapObject; z++)
				{
					if (((pFirstMapObject+z)->iLayer==2)&&(!(pFirstMapObject+z)->bLandmark)&&((pFirstMapObject+z)->arLand[(pFirstProvince+PR)->ipTerrain]>0))
					{
						bool bDraw=true;
						if (((pFirstProvince+PR)->ipTerrain==2)&&((pFirstMapObject+z)->iType>0)&&((CurGame.iSeed+PR)%4==0))
						{
							if ((CurGame.iSeed/7+PR)%2+1!=(pFirstMapObject+z)->iType)
							{bDraw=false;};
						};
						if (PR==11)
						{
							int HHHH=0;
						};
						PR1=GetProvinceIndex(i-(pFirstMapObject+z)->dx/2,j-(pFirstMapObject+z)->dy/2);
						if ((PR1==0)||((pFirstMapObject+z)->arLand[(pFirstProvince+PR1)->ipTerrain]==0))
						{bDraw=false;};
						PR1=GetProvinceIndex(i+(pFirstMapObject+z)->dx/2,j-(pFirstMapObject+z)->dy/2);
						if ((PR1==0)||((pFirstMapObject+z)->arLand[(pFirstProvince+PR1)->ipTerrain]==0))
						{bDraw=false;};
						PR1=GetProvinceIndex(i,j-(pFirstMapObject+z)->dy/2);
						if ((PR1==0)||((pFirstMapObject+z)->arLand[(pFirstProvince+PR1)->ipTerrain]==0))
						{bDraw=false;};
						if (bDraw)
						{
							RollerN[Counter]=z;
							RollerP[Counter]=(pFirstMapObject+z)->arLand[(pFirstProvince+PR)->ipTerrain];
							Counter++;							
						};
					};					
				};
	//			textprintf (screen,Font1,700,20,COLOR_2,"Спрайт: %d, земля: %d",Counter,(pFirstProvince+PR)->ipTerrain);
				if (Counter>0)
				{
					int iObj=Roll(false,CurGame.iSeed,j,i,2);
					int iPic=0;
					while ((iPic<MAX_MAPOBJ_PIC)&&((pFirstMapObject+iObj)->arPic[iPic])){iPic++;};
					iPic=Random(iPic,j,iSeed,2,i);									
					int XP=i-(pFirstMapObject+iObj)->arPic[iPic]->w/2, YP=j-(pFirstMapObject+iObj)->arPic[iPic]->h, Add=(pFirstMapObject+iObj)->arPic[iPic]->w/2, Shift=3;
					if ((pFirstProvince+GetProvinceIndex(x+Pic->w/2,y+Pic->h/2))->ipTerrain==2)
					{Shift=12;};
					if ((XP+Add*2>x)&&(XP<x+Pic->w)&&(YP+(pFirstMapObject+iObj)->arPic[iPic]->h>y))
					{					
						bool bDraw=false;
						if ((i-x>0)&&(i-x<Pic->w)&&(Arr[i-x]>0))
						{
							if (j-y>Arr[i-x]+Shift)							
							{bDraw=true;};
						}
						else
						{
							if ((i-x-Add>0)&&(i-x-Add<Pic->w)&&(Arr[i-x-Add]>0)&&(j-y>Arr[i-x-Add]+Shift))
							{bDraw=true;};
							if ((i-x+Add>0)&&(i-x+Add<Pic->w)&&(Arr[i-x+Add]>0)&&(j-y>Arr[i-x+Add]+Shift))
							{bDraw=true;};
						};
						if (bDraw)
						{
							draw_sprite(bmpNew,(pFirstMapObject+iObj)->arPic[iPic],XP-x,YP-y);
							bEmpty=false;
						};
					};
					
				};
			};
		};
	};
	if (bEmpty)
	{
		destroy_bitmap(bmpNew);
		return NULL;
	};
	return bmpNew;
};

void FillObstracleMap(int Progress)
{
	dcProvince * Prov=pFirstProvince;
	int PRG=NewProgress;
	for (long int i=1; i<=MapSize; i++)
	{
		Prov++;
	//	char Txt[50];
	//	sprintf(Txt,"Province %d:",i);
	//	FileLog(Txt);
		Prov->DrawPic(false);
		NewProgress=PRG+i*Progress/MapSize;		
	};
	
};

//Создает границу между точками  (x1, y1) и (x2, y2)
void DrawLine(int x1, int y1, int x2, int y2, int Pr1, int Pr2, int Pr3)
{	
	float dx, dy;
	if (abs(x1-x2)>abs(y1-y2))
	{
		dx=(x2-x1)/abs(x2-x1);
		dy=(y2-y1);
		dy/=abs(x2-x1);	
		float ry=y1;
		int y=y1;
		for (int x=x1; x!=x2; x+=dx)
		{
			Map_Border *Point=Map[y];
			while ((Point)&&(Point->x!=x))
			{Point=Point->Next;};
			if (!Point)
			{
				Point=new Map_Border;
				Point->Next=Map[y];
				Point->x=x;
				Map[y]=Point;
				Point->Prov[2]=0;
				Point->Prov[0]=Pr1;
				Point->Prov[1]=Pr2;			
				if (x==x1)
				{Point->Prov[2]=Pr3;};
			}
			else
			{
				bool bAdd=true;
				for (int i=0; i<3; i++)
				{
					if (Point->Prov[i]==Pr1)
					{bAdd=false;};
				};
				if (bAdd)
				{Point->Prov[2]=Pr1;};
				bAdd=true;
				for (int i=0; i<3; i++)
				{
					if (Point->Prov[i]==Pr2)
					{bAdd=false;};
				};
				if (bAdd)
				{Point->Prov[2]=Pr2;};
				if (x==x1)
				{
					bAdd=true;
					for (int i=0; i<3; i++)
					{
						if (Point->Prov[i]==Pr3)
						{bAdd=false;};
					};
					if (bAdd)
					{Point->Prov[2]=Pr3;};
				};
			};
		
						
			ry+=dy;
			y=ry;
		};
	}
	else
	{
		dx=(x2-x1);
		dx/=abs(y2-y1);
		dy=(y2-y1)/abs(y2-y1);	
		float rx=x1;
		int x=x1;
		for (int y=y1; y!=y2; y+=dy)
		{
			Map_Border *Point=Map[y];
			while ((Point)&&(Point->x!=x))
			{Point=Point->Next;};
			if (!Point)
			{
				Point=new Map_Border;
				Point->Next=Map[y];
				Point->x=x;
				Map[y]=Point;
				Point->Prov[2]=0;
				Point->Prov[0]=Pr1;
				Point->Prov[1]=Pr2;	
				if (y==y1)
				{Point->Prov[2]=Pr3;};
			}
			else
			{
				bool bAdd=true;
				for (int i=0; i<3; i++)
				{
					if (Point->Prov[i]==Pr1)
					{bAdd=false;};
				};
				if (bAdd)
				{Point->Prov[2]=Pr1;};
				bAdd=true;
				for (int i=0; i<3; i++)
				{
					if (Point->Prov[i]==Pr2)
					{bAdd=false;};
				};
				if (bAdd)
				{Point->Prov[2]=Pr2;};
				if (y==y1)
				{
					bAdd=true;
					for (int i=0; i<3; i++)
					{
						if (Point->Prov[i]==Pr3)
						{bAdd=false;};
					};
					if (bAdd)
					{Point->Prov[2]=Pr3;};
				};
			};	
			rx+=dx;
			x=rx;
		};
	};
};

clDeep::clDeep(int sx, int sy, int sDeep, clDeep *Next)
{
	x=sx; y=sy; Deep=sDeep; pNext=Next;
};

clDeep::~clDeep()
{};

//Добавляет новую точку в массив глубин, возвращает истину при успехе
clDeep * AddDeep(int x, int y, int Deep, clDeep *pDeep)
{
	bool Result=false;
	int PR=GetProvinceIndex(x,y);
	if ((PR>0)&&(pFirstProvince+PR)->ipTerrain==WATER_ID)
	{Result=true;};	
	clDeep *DP=pDeep;
	while ((DP)&&(Result))
	{
		if ((DP->x==x)&&(DP->y==y))
		{Result=false;};
		DP=DP->pNext;
	};
	if (Result)
	{pDeep=new clDeep(x,y,Deep,pDeep);}
	else
	{pDeep=NULL;};
	return pDeep;
};

bool GetDeep(int x, int y)
{
	if ((x<0)||(y<0)||(x>=bmpMap->w)||(y>=bmpMap->h))
	{return false;};
		
	int PR=(GetProvinceIndex(x,y));
	if (PR==0)
	{return false;};
	if ((pFirstProvince+PR)->ipTerrain==0)
	{return false;}
	else
	{return true;};
};

void MakeCastleName(int ipProvince, bool * pNames)
{
	ClearRoll();
	int Count=0;
	for (int i=1; i<=iMaxProvinceName; i++)
	{			
		if ((*(pNames+i))&&((pFirstProvinceName+i)->CheckType(7))&&((pFirstProvinceName+i)->CheckTerrain((pFirstProvince+ipProvince)->ipTerrain)))
		{
			bool bName=false;
			for (int j=0; j<MAX_NAME_TYPE; j++)
			{
				if ((pFirstProvinceName+i)->arType[j]==7)
				{bName=true;};
			};
			if (bName)
			{
				RollerN[Count]=i;
				RollerP[Count]=(pFirstProvinceName+i)->iRarity;
				Count++;
			};
		};
	};
	int iName=Roll(false);
	(pFirstProvince+ipProvince)->sName=new char [sizeof (pFirstProvinceName+iName)->sName];
	for (int i=0; i<sizeof (pFirstProvinceName+iName)->sName; i++)
	{
		*((pFirstProvince+ipProvince)->sName+i)=(pFirstProvinceName+iName)->sName[i];
	};
	*(pNames+iName)=false;
};


void GenerateMap (clMap mpMap, int WorldRul)
{
	FileLog("Generation started");
	WorldRules.Init(WorldRul);

//	CurGame.iSeed=3290; mpMap.X=6; mpMap.Y=6; mpMap.Difficulty=0; mpMap.Dissolve=33; mpMap.Forest=10; mpMap.Hill=18; mpMap.Isles=12; mpMap.Neutrals=1; mpMap.Players=2; mpMap.Population=2; mpMap.Resource=2; mpMap.Site=2; mpMap.Swamp=30; mpMap.Type=2; mpMap.Water=11;
//	CurGame.iSeed=1380; mpMap.X=8; mpMap.Y=8; mpMap.Difficulty=0; mpMap.Dissolve=39; mpMap.Forest=15; mpMap.Hill=23; mpMap.Isles=11; mpMap.Neutrals=2; mpMap.Players=3; mpMap.Population=2; mpMap.Resource=2; mpMap.Site=2; mpMap.Swamp=10; mpMap.Type=2; mpMap.Water=14;
//	CurGame.iSeed=2025;
	if (CurGame.iSeed==0)
	{CurGame.iSeed=111;}
	
	char Txt[200];
	sprintf(Txt,"Seed: %d, X: %d, Y: %d Type: %d, Water: %d, Hill: %d, Forest: %d, Swamp: %d, Isles: %d, Dissolve: %d.",CurGame.iSeed, mpMap.X, mpMap.Y, mpMap.Type, mpMap.Water, mpMap.Hill, mpMap.Forest, mpMap.Swamp, mpMap.Isles, mpMap.Dissolve);
	FileLog(Txt);
	sprintf(Txt,"Profile: %d, TutorialMap: %d, GameMode: %d, Players: %d.",arHumans[0].iFileIndex, Tutorial.Map, CurGame.iGameType, CurGame.iMaxPlayers);
	FileLog(Txt);
	clPlayerProfile * Prf=NULL;
	if ((CurGame.iGameType==2)&&(Campaign.iActiveShard<0)&&(!Campaign.bDefender))
	{Prf=(pFirstPlayerProfile+abs(Campaign.iActiveShard));};
	if (Prf)
	{
		switch (Prf->iLandType)
		{
		case 2:
			mpMap.Forest+=Prf->iLandPoss;
			if (mpMap.Forest+mpMap.Hill+mpMap.Swamp>90)
			{mpMap.Forest=90-(mpMap.Hill+mpMap.Swamp);};
			break;
		case 3:
			mpMap.Hill+=Prf->iLandPoss;
			if (mpMap.Forest+mpMap.Hill+mpMap.Swamp>90)
			{mpMap.Hill=90-(mpMap.Forest+mpMap.Swamp);};
			break;
		case 4:
			mpMap.Swamp+=Prf->iLandPoss;
			if (mpMap.Forest+mpMap.Hill+mpMap.Swamp>90)
			{mpMap.Swamp=90-(mpMap.Hill+mpMap.Forest);};
			break;
		};
	};

	OldProgress=0;
	NewProgress=0;
	BITMAP *bmpBar=create_bitmap(888,100);
	rectfill(bmpBar,0,0,888,100,COLOR_1);
	UI_TextBox txtHint;
	txtHint.Init(0,0,848,100,1,COLOR_2,Font2,NULL);
	txtHint.bCentre=true;
	txtHint.Activate((pFirstHint+arHumans[0].iHint)->msg);
	arHumans[0].iHint++;
	if (arHumans[0].iHint>iMaxHint)
	{arHumans[0].iHint=0;};
	arHumans[0].Save();
	set_trans_blender(150,150,150,150);
	ShowWallpaper();
	draw_trans_sprite(screen1,bmpBar,68,648);	
	draw_sprite(screen1,txtHint.Pic,78,658);	
	set_trans_blender(128,128,128,128);
	RunHourGlass(328, 0);
	delete txtHint.Obj;
	destroy_bitmap(bmpBar);
	for (int i=0; i<10; i++)
	{
		CurGame.ResPrices[i]=100;
	};
	CurGame.ResBuy=RES_BUY_CHANGE-CurGame.iMaxPlayers/5;
	CurGame.iCurrentTurn=0;
	CurGame.iActivePlayer=0;
	CurGame.Map=mpMap;
	srand(CurGame.iSeed);

	//Расовые квесты
	for (int i=0; i<=iMaxRace; i++)
	{
		int Count=0;
		CurGame.arRaceQuest[i]=1;
		ClearRoll();
		for (int j=0; j<MAX_RACE_QUEST; j++)
		{
			if ((pFirstRace+i)->arQuest[j]>0)
			{
				RollerN[Count]=(pFirstRace+i)->arQuest[j];
				RollerP[Count]=(pFirstQuest+(pFirstRace+i)->arQuest[j])->iPossibility;
				Count++;
			};
		};
		if (Count>0)
		{CurGame.arRaceQuest[i]=Roll(false);};
	};
	/*CurGame.arRaceQuest[3]=2;
	CurGame.arRaceQuest[4]=6;
	CurGame.arRaceQuest[5]=3;
	CurGame.arRaceQuest[6]=5;
	CurGame.arRaceQuest[7]=4;
	CurGame.arRaceQuest[8]=51;*/
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if (arPlayer[i].Control>1)
		{			
			arPlayer[i].Gem=20+arPlayer[i].iDifficulty*5;
			arPlayer[i].Gold=500+arPlayer[i].iDifficulty*50;
		}
		else
		{
			arPlayer[i].Gold=arDifficulty[arPlayer[i].iDifficulty].iGold;
			arPlayer[i].Gem=arDifficulty[arPlayer[i].iDifficulty].iGem;

			/*switch (arPlayer[i].iDifficulty)
			{
			case 0:
				arPlayer[i].Gem=100;
				arPlayer[i].Gold=1000;
				break;
			case 1:
				arPlayer[i].Gem=50;
				arPlayer[i].Gold=850;
				break;
			case 2:
				arPlayer[i].Gem=30;
				arPlayer[i].Gold=600;
				break;
			default:
				arPlayer[i].Gem=20;
				arPlayer[i].Gold=500;
				break;
			};*/
		};
		arPlayer[i].Init(true,i);
	};
	for (int i=CurGame.iMaxPlayers+1; i<MAX_PLAYERS; i++)
	{
		arPlayer[i].bInGame=false;
	};
	//Установка взаимоотношений между ИИ
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if (arPlayer[i].Control==2)
		{
			for (int j=1; j<=CurGame.iMaxPlayers; j++)
			{
				if ((i!=j)&&(arPlayer[j].Control==2))
				{
					arPlayer[i].Politic[j].Attitude=(pFirstPlayerProfile+arPlayer[i].ipProfile)->arAtt[arPlayer[j].ipProfile];
				};
			};
		};
	};
	arPlayer[0].Control=2;
	arPlayer[0].Gem=10000;
	arPlayer[0].iDifficulty=0;
	pRitual=NULL;

	int Row1[100][5], Row2[100][5];
	mpMap.Y=mpMap.Y-mpMap.Y%2;
	MapSize=mpMap.X*mpMap.Y;
	pFirstProvince=new dcProvince[MapSize+1];
	pMoveMap=new int [MapSize+1];
	pRunMap=new int [MapSize+1];
	pMobilMap=new int [MapSize+1];
	for (int i=0; i<MAP_SIZE; i++)
	{
		Map[i]=NULL;
	};


	//Создание границ
//	rectfill(screen,0,0,1024,768,makecol(20,50,150));
//	int Txt=1;
//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Номер карты: %d",CurGame.iSeed);
//	Txt++;
//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Создание границ...");
//	Txt++;

	int y=Map_h/6, RX=Map_w/3;
	for (int i=0; i<mpMap.X; i++)
	{
		Row1[i][0]=i*Map_w+Random(RX)+Map_w/6+Map_w/3;
		Row1[i][1]=y+Random(Map_h/3-2)-Map_h/6+1;
		Row1[i][2]=0;
		Row1[i][3]=0;
		Row1[i][4]=i+1;
	};

	for (int Line=0; Line<mpMap.Y/2; Line++)
	{
		y+=Map_h/3;
		for (int i=0; i<=mpMap.X; i++)
		{			
			Row2[i][0]=i*Map_w+Random(RX);
			Row2[i][1]=y+Random(Map_h/3-2)-Map_h/6+1;
			if (i==0)
			{Row2[i][2]=0;}
			else
			{Row2[i][2]=Row1[i-1][4];};
			if (i==mpMap.X)
			{
				Row2[i][3]=0;
				Row2[i][4]=Row1[i-1][3];
			}
			else
			{
				Row2[i][3]=Row1[i][4];
				Row2[i][4]=Row1[i][2];
			};
		};
		for (int i=0; i<mpMap.X; i++)
		{			
			DrawLine(Row1[i][0],Row1[i][1],Row2[i][0],Row2[i][1],Row1[i][2],Row1[i][4],Row1[i][3]);
			DrawLine(Row1[i][0],Row1[i][1],Row2[i+1][0],Row2[i+1][1],Row1[i][3],Row1[i][4],Row1[i][2]);			
		};
		if (Line>0)
		{
			DrawLine(Row1[mpMap.X][0],Row1[mpMap.X][1],Row2[mpMap.X][0],Row2[mpMap.X][1],Row1[mpMap.X][2],Row1[mpMap.X][4],Row1[mpMap.X][3]);
		};

		y+=Map_h/3;
		for (int i=0; i<=mpMap.X; i++)
		{
			Row1[i][0]=i*Map_w+Random(RX);
			Row1[i][1]=y+Random(Map_h/3-2)-Map_h/6+1;
			Row1[i][2]=Row2[i][2];
			Row1[i][3]=Row2[i][3];
			if (i==0)
			{Row1[i][4]=0;}
			else
			{Row1[i][4]=Line*2*mpMap.X+mpMap.X+i;};
			DrawLine(Row2[i][0],Row2[i][1],Row1[i][0],Row1[i][1],Row2[i][2],Row2[i][3],Row2[i][4]);
		};

		y+=Map_h/3;
		for (int i=0; i<=mpMap.X; i++)
		{			
			Row2[i][0]=i*Map_w+Random(RX)+Map_w/6+Map_w/3;
			Row2[i][1]=y+Random(Map_h/3-2)-Map_h/6+1;
			Row2[i][2]=Row1[i][4];
			if (i==mpMap.X)
			{
				Row2[i][3]=0;
				Row2[i][4]=0;
			}
			else
			{
				Row2[i][3]=Row1[i+1][4];
				Row2[i][4]=Row1[i][3];
			};
		};
		for (int i=1; i<=mpMap.X; i++)
		{			
			DrawLine(Row1[i][0],Row1[i][1],Row2[i-1][0],Row2[i-1][1],Row1[i][2],Row1[i][4],Row1[i][3]);
			DrawLine(Row1[i][0],Row1[i][1],Row2[i][0],Row2[i][1],Row1[i][3],Row1[i][4],Row1[i][2]);			
		};
		DrawLine(Row1[0][0],Row1[0][1],Row2[0][0],Row2[0][1],Row1[0][3],Row1[0][4],Row1[0][3]);

		y+=Map_h/3;
		for (int i=0; i<=mpMap.X; i++)
		{
			Row1[i][0]=i*Map_w+Random(RX)+Map_w/6+Map_w/3;
			Row1[i][1]=y+Random(Map_h/3-2)-Map_h/6+1;
			Row1[i][2]=Row2[i][2];
			Row1[i][3]=Row2[i][3];
			if (i==mpMap.X)
			{Row1[i][4]=0;}
			else
			{Row1[i][4]=(Line+1)*mpMap.X*2+i+1;};
			DrawLine(Row2[i][0],Row2[i][1],Row1[i][0],Row1[i][1],Row2[i][2],Row2[i][3],Row2[i][4]);
		};
	};
	y+=Map_h/3;
	for (int i=0; i<=mpMap.X; i++)
	{			
		Row2[i][0]=i*Map_w+Random(RX);
		Row2[i][1]=y+Random(Map_h/3-2)-Map_h/6+1;
		if (i==0)
		{Row2[i][2]=0;}
		else
		{Row2[i][2]=Row1[i-1][4];};
		if (i==mpMap.X)
		{
			Row2[i][3]=0;
			Row2[i][4]=Row1[i-1][2];
		}
		else
		{
			Row2[i][3]=Row1[i][4];
			Row2[i][4]=Row1[i][3];
		};
	};
	for (int i=1; i<mpMap.X; i++)
	{			
		DrawLine(Row1[i][0],Row1[i][1],Row2[i][0],Row2[i][1],Row1[i][2],0,Row1[i][3]);
		DrawLine(Row1[i][0],Row1[i][1],Row2[i+1][0],Row2[i+1][1],Row1[i][3],0,Row1[i][2]);			
	};
	DrawLine(Row1[0][0],Row1[0][1],Row2[1][0],Row2[1][1],Row1[0][3],0,Row1[0][2]);
	DrawLine(Row1[mpMap.X][0],Row1[mpMap.X][1],Row2[mpMap.X][0],Row2[mpMap.X][1],Row1[mpMap.X][2],0,Row1[mpMap.X][3]);

	NewProgress=100;

	

	//Вычисление соседей, размеров и центральных точек
//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Вычисление соседей, размеров и центральных точек...");
//	Txt++;
	long FullSize=0;
//	objMain=new G_Object(objMain,0,0,0,bmpGUI_Dead);	
	for (int j=0; j<MAX_NEIGHBOUR; j++)
	{
		pFirstProvince->arNeighbour[j]=0;
	};
	dcProvince * Prov=pFirstProvince;
	for (int i=1; i<=MapSize; i++)
	{
		Prov++;
		Prov->iNumber=i;
//		Prov->TownPic1=create_bitmap(60,46);
//		rectfill(Prov->TownPic1,0,0,60,46,COLOR_0);
//		Prov->TownPic2=create_bitmap(100,48);
//		rectfill(Prov->TownPic2,0,0,100,48,COLOR_0);
		for (int j=0; j<MAX_NEIGHBOUR; j++)
		{
			Prov->arNeighbour[j]=0;
		};
		Prov->ipPlayer=0;
		Prov->iSize=0;
		int Neigh=0;
		for (int y=0; y<(mpMap.Y+1)*Map_h;y++)
		{
			Map_Border * Cr=Map[y];
			int x1=-1, x2=-1;
			while (Cr)
			{
				if ((Cr->Prov[0]==i)||(Cr->Prov[1]==i)||(Cr->Prov[2]==i))
				{
					if (x1>=0)
					{x2=Cr->x;}
					else
					{x1=Cr->x;};
					for (int j=0; j<3; j++)
					{
						if ((Cr->Prov[j]!=i)&&(Cr->Prov[j]>0))
						{
							bool bAdd=true;
							for (int k=0; k<Neigh; k++)
							{
								if (Prov->arNeighbour[k]==Cr->Prov[j])
								{bAdd=false;};
							};
							if (bAdd)
							{
								Prov->arNeighbour[Neigh]=Cr->Prov[j];
								Neigh++;
							};
						};
					};
				};				
				Cr=Cr->Next;
			};
			if (x1>=0)
			{
				if (x2>=0)
				{Prov->iSize+=abs(x2-x1);}
				else
				{Prov->iSize++;};
			};
		};
		FullSize+=Prov->iSize;
		int Size=0;
		for (int y=0; y<(mpMap.Y+1)*Map_h;y++)
		{
			Map_Border * Cr=Map[y];
			int x1=-1, x2=-1;
			while (Cr)
			{
				if ((Cr->Prov[0]==i)||(Cr->Prov[1]==i)||(Cr->Prov[2]==i))
				{
					if (x1>=0)
					{x2=Cr->x;}
					else
					{x1=Cr->x;};					
				};				
				Cr=Cr->Next;
			};
			if (x1>=0)
			{
				if (x2>=0)
				{Size+=abs(x2-x1);}
				else
				{Size++;};
			};
			if (Size>=Prov->iSize/2)
			{
				Prov->y=y;
				Prov->x=(x1+x2)/2;
				break;
			};
		};
	//Поиск оптимальных мест для расположения построек
		int arPlace[10][3]=
		{	
			{-50,25,0},
			{-70,-5,0},
			{-50,-35,0},
			{50,25,0},
			{70,-5,0},
			{50,-35,0},
			{-25,-55,0},
			{35,-55,0},
			{-30,50,0},
			{30,50,0}
		};
		int PP;
		for (int k=0; k<10; k++)
		{
			if (GetProvinceIndex(Prov->x+arPlace[k][0],Prov->y+arPlace[k][1])==i)
			{arPlace[k][2]+=15;};
			for (int Go=10; Go<40; Go+=10)
			{
				PP=GetProvinceIndex(Prov->x+arPlace[k][0]+Go,Prov->y+arPlace[k][1]);
				if (PP==i)
				{arPlace[k][2]+=30/Go;}
				else
				{
					if ((PP==0)||((pFirstProvince+PP)->ipTerrain==0))
					{arPlace[k][2]-=30/Go;};
				};
				PP=GetProvinceIndex(Prov->x+arPlace[k][0]+Go,Prov->y+arPlace[k][1]-Go);
				if (PP==i)
				{arPlace[k][2]+=30/Go;}
				else
				{
					if ((PP==0)||((pFirstProvince+PP)->ipTerrain==0))
					{arPlace[k][2]-=30/Go;};
				};
				PP=GetProvinceIndex(Prov->x+arPlace[k][0]+Go,Prov->y+arPlace[k][1]+Go);
				if (PP==i)
				{arPlace[k][2]+=30/Go;}
				else
				{
					if ((PP==0)||((pFirstProvince+PP)->ipTerrain==0))
					{arPlace[k][2]-=30/Go;};
				};
				PP=GetProvinceIndex(Prov->x+arPlace[k][0],Prov->y+arPlace[k][1]-Go);
				if (PP==i)
				{arPlace[k][2]+=30/Go;}
				else
				{
					if ((PP==0)||((pFirstProvince+PP)->ipTerrain==0))
					{arPlace[k][2]-=30/Go;};
				};
				PP=GetProvinceIndex(Prov->x+arPlace[k][0],Prov->y+arPlace[k][1]+Go);
				if (PP==i)
				{arPlace[k][2]+=30/Go;}
				else
				{
					if ((PP==0)||((pFirstProvince+PP)->ipTerrain==0))
					{arPlace[k][2]-=30/Go;};
				};
				PP=GetProvinceIndex(Prov->x+arPlace[k][0]-Go,Prov->y+arPlace[k][1]-Go);
				if (PP==i)
				{arPlace[k][2]+=30/Go;}
				else
				{
					if ((PP==0)||((pFirstProvince+PP)->ipTerrain==0))
					{arPlace[k][2]-=30/Go;};
				};
				PP=GetProvinceIndex(Prov->x+arPlace[k][0]-Go,Prov->y+arPlace[k][1]);
				if (PP==i)
				{arPlace[k][2]+=30/Go;}
				else
				{
					if ((PP==0)||((pFirstProvince+PP)->ipTerrain==0))
					{arPlace[k][2]-=30/Go;};
				};
				PP=GetProvinceIndex(Prov->x+arPlace[k][0]-Go,Prov->y+arPlace[k][1]+Go);
				if (PP==i)
				{arPlace[k][2]+=30/Go;}
				else
				{
					if ((PP==0)||((pFirstProvince+PP)->ipTerrain==0))
					{arPlace[k][2]-=30/Go;};
				};
			};
		};
		for (int BL=0; BL<=MAX_PROVINCE_BLD; BL++)
		{
			int MAX=-100, Choose=0;
			for (int k=0; k<10; k++)
			{
				if (arPlace[k][2]>MAX)
				{
					MAX=arPlace[k][2];
					Choose=k;
				};
			};
			Prov->arBldPlace[BL][0]=Prov->x+arPlace[Choose][0];
			Prov->arBldPlace[BL][1]=Prov->y+arPlace[Choose][1];
			arPlace[Choose][2]=-1000;
		};
	};
	NewProgress=200;

	long MedSize=FullSize/MapSize;
	bool *pNames=new bool [iMaxProvinceName+1];
	int Counter=0;
	for (int i=0; i<=iMaxProvinceName; i++)
	{
		*(pNames+i)=true;
	};
	iMaxShop=0;
	pFirstShop=NULL;

	if (Tutorial.Map)
	{
	//Карта для обучающей миссии
		for (int j=1; j<=MapSize; j++)
		{	
			*(pMoveMap+j)=2;
		};

		//Замок противника
		(pFirstProvince+7)->ipTerrain=1;
		(pFirstProvince+7)->ipType=7;
		(pFirstProvince+7)->iExplore=50;
		(pFirstProvince+7)->ipPlayer=2;
		(pFirstProvince+7)->ipRace=1;
		(pFirstProvince+7)->iPopulation=10;
		(pFirstProvince+7)->pGarrison=new dcHero();
		(pFirstProvince+7)->pGarrison->ipPlayer=2;
		(pFirstProvince+7)->pGarrison->arGarrison[0]=6;
		(pFirstProvince+7)->pGarrison->ipProvince=7;
		(pFirstProvince+7)->pGarrison->pProvince=(pFirstProvince+7);
		arPlayer[2].iWall=50;
		(pFirstProvince+7)->iFort=arPlayer[2].iWall;
		(pFirstProvince+7)->iGemInc=5;
		(pFirstProvince+7)->iGoldInc=20;
		arPlayer[2].pFog=new int [MapSize+1];
		for (int j=1; j<=MapSize; j++)
		{
			*(arPlayer[2].pFog+j)=2;
		};		
		arPlayer[2].ipProvince=7;
		(pFirstProvince+7)->PlaceDefender(6);
		arPlayer[2].Build(39);
		for (int i=0; i<6; i++)
		{(pFirstProvince+7)->pGarrison->pSquad[i]=new dcUnit(4,30);};
		MakeCastleName(7,pNames);

		//Замок игрока
		(pFirstProvince+23)->ipTerrain=1;
		(pFirstProvince+23)->ipType=7;
		(pFirstProvince+23)->iExplore=50;
		(pFirstProvince+23)->ipPlayer=0;
		(pFirstProvince+23)->ipRace=1;
		(pFirstProvince+23)->iPopulation=10;
		(pFirstProvince+23)->pGarrison=new dcHero();
		(pFirstProvince+23)->pGarrison->ipPlayer=1;
		(pFirstProvince+23)->pGarrison->arGarrison[0]=6;
		(pFirstProvince+23)->pGarrison->ipProvince=23;
		(pFirstProvince+23)->pGarrison->pProvince=(pFirstProvince+23);
		(pFirstProvince+23)->iFort=arPlayer[1].iWall;
		(pFirstProvince+23)->iGemInc=5;
		(pFirstProvince+23)->iGoldInc=20;		
		arPlayer[1].pFog=new int [MapSize+1];
		*(arPlayer[1].pFog)=0;
		for (int j=1; j<=MapSize; j++)
		{
			*(arPlayer[1].pFog+j)=2;
		};		
		arPlayer[1].ipProvince=23;
		if (arHumans[0].iEnterData==3)
		{
			(pFirstProvince+23)->sName=new char [sizeof arHumans[arPlayer[1].iColor].sLand];
			for (int j=0; j<sizeof (pFirstPlayerProfile+arPlayer[1].ipProfile)->sLand; j++)
			{
				*((pFirstProvince+23)->sName+j)=arHumans[arPlayer[1].iColor].sLand[j];
			};
		}
		else
		{
			MakeCastleName(23,pNames);
		};
		
		//Остальные провинции
		(pFirstProvince+1)->ipTerrain=MOUNTAIN_ID;
		(pFirstProvince+2)->ipTerrain=MOUNTAIN_ID;
		(pFirstProvince+3)->ipTerrain=WATER_ID;
		(pFirstProvince+4)->ipTerrain=MOUNTAIN_ID;
		(pFirstProvince+5)->ipTerrain=FOREST_ID;
		(pFirstProvince+6)->ipTerrain=FOREST_ID;

		(pFirstProvince+8)->ipTerrain=HILL_ID;
		(pFirstProvince+9)->ipTerrain=WATER_ID;
		(pFirstProvince+10)->ipTerrain=FOREST_ID;
		(pFirstProvince+11)->ipTerrain=PLAIN_ID;
		(pFirstProvince+12)->ipTerrain=PLAIN_ID;

		(pFirstProvince+13)->ipTerrain=HILL_ID;
		(pFirstProvince+14)->ipTerrain=HILL_ID;
		(pFirstProvince+15)->ipTerrain=MOUNTAIN_ID;
		(pFirstProvince+16)->ipTerrain=MOUNTAIN_ID;
		(pFirstProvince+17)->ipTerrain=WATER_ID;
		(pFirstProvince+18)->ipTerrain=PLAIN_ID;

		(pFirstProvince+19)->ipTerrain=MOUNTAIN_ID;
		(pFirstProvince+20)->ipTerrain=MOUNTAIN_ID;
		(pFirstProvince+21)->ipTerrain=FOREST_ID;
		(pFirstProvince+22)->ipTerrain=WATER_ID;
		(pFirstProvince+24)->ipTerrain=FOREST_ID;

		(pFirstProvince+25)->ipTerrain=MOUNTAIN_ID;
		(pFirstProvince+26)->ipTerrain=PLAIN_ID;
		(pFirstProvince+27)->ipTerrain=PLAIN_ID;
		(pFirstProvince+28)->ipTerrain=WATER_ID;
		(pFirstProvince+29)->ipTerrain=WATER_ID;
		(pFirstProvince+30)->ipTerrain=SWAMP_ID;

		(pFirstProvince+31)->ipTerrain=WATER_ID;
		(pFirstProvince+32)->ipTerrain=WATER_ID;
		(pFirstProvince+33)->ipTerrain=WATER_ID;
		(pFirstProvince+34)->ipTerrain=WATER_ID;
		(pFirstProvince+35)->ipTerrain=SWAMP_ID;
//		(pFirstProvince+35)->arSite[0][0]=4;
//		(pFirstProvince+35)->arSite[0][1]=1;
		(pFirstProvince+36)->ipTerrain=FOREST_ID;

		(pFirstProvince+13)->ipPlayer=2;
		(pFirstProvince+14)->ipPlayer=2;
		(pFirstProvince+8)->ipPlayer=2;
		(pFirstProvince+8)->arBuilding[0]=10;

		for (int PP=26; PP<28; PP++)
		{
		dcProvince *PR=(pFirstProvince+PP);
		PR->ipType=1;
		PR->ipRace=1;
		PR->iPopulation=300;		
		PR->iExplore=1;		
		
//Название
		ClearRoll();
		int Count=0;
		for (int i=1; i<=iMaxProvinceName; i++)
		{			
			if ((*(pNames+i))&&((pFirstProvinceName+i)->CheckType(PR->ipType))&&((pFirstProvinceName+i)->CheckTerrain(PR->ipTerrain)))
			{
				bool bName=false;
				for (int j=0; j<MAX_NAME_TYPE; j++)
				{
					if ((pFirstProvinceName+i)->arType[j]==1)
					{bName=true;};
				};
				if ((bName)&&(((pFirstProvinceName+i)->iCoast==0)||(((pFirstProvinceName+i)->iCoast==1)&&(PR->Coastal==1))))
				{
				RollerN[Count]=i;
				RollerP[Count]=(pFirstProvinceName+i)->iRarity;
				Count++;
				};
			};
		};
		int iName=Roll(false);
		PR->sName=new char [sizeof (pFirstProvinceName+iName)->sName];
		for (int i=0; i<sizeof (pFirstProvinceName+iName)->sName; i++)
		{
			*(PR->sName+i)=(pFirstProvinceName+iName)->sName[i];
		};
		*(pNames+iName)=false;		

//Доход
		PR->iGoldInc=(pFirstTerrain+PR->ipTerrain)->iGoldInc+(pFirstProvinceType+PR->ipType)->iGold;
		PR->iGoldInc=PR->iGoldInc*PR->iSize/MedSize;
		PR->iGoldInc+=2-Random(5);
		if (PR->iGoldInc<=0)
		{PR->iGoldInc=1;};

		PR->iGemInc=(pFirstTerrain+PR->ipTerrain)->iGemInc+(pFirstProvinceType+PR->ipType)->iGem;
		PR->iGemInc=PR->iGemInc*PR->iSize/MedSize;
		PR->iGemInc-=Random(4);
		if (PR->iGemInc<0)
		{PR->iGemInc=0;};		
		};
		PR=(pFirstProvince+27);
		PR->pGarrison=new dcHero();
		PR->pGarrison->ipPlayer=1;
		PR->pGarrison->arGarrison[0]=0;
		PR->pGarrison->ipProvince=27;
		PR->pGarrison->pProvince=PR;
		PR->iPopulation=10000;		
		PR->arBuilding[0]=7;
		PR->arBuilding[1]=18;
		PR->arBuilding[2]=19;
		PR->PlaceSite(11,1,0,0,0);
		PR->arSite[0][4]=4;
		PR->PlaceSite(12,1,0,0,0);
		PR->arSite[1][4]=6;
		PR->PlaceSite(10,1,0,0,0);
		PR->arSite[2][4]=5;
		PR->iFort=100;
		for (int k=0; k<3; k++)
		{
			PR->arSiteGuard[k]->iTask=0;
			for (int i=0; i<MAX_SQUAD; i++)
			{	
				if (PR->arSiteGuard[k]->pSquad[i])
				{
					delete PR->arSiteGuard[k]->pSquad[i];
					PR->arSiteGuard[k]->pSquad[i]=NULL;
					PR->arSite[k][1]=k+1;
				};
			};
		};
		for (int i=0; i<3; i++)
		{
			PR->arSiteGuard[0]->pSquad[i]=new dcUnit(13,0);
		};
		for (int i=0; i<3; i++)
		{
			PR->arSiteGuard[1]->pSquad[i]=new dcUnit(14,0);
		};
		for (int i=0; i<2; i++)
		{
			PR->arSiteGuard[2]->pSquad[i]=new dcUnit(19,0);
			PR->arSiteGuard[2]->pSquad[i+2]=new dcUnit(20,0);
		};

		PR=(pFirstProvince+21);
		PR->ipType=2;
		PR->ipRace=1;
		PR->iPopulation=100;		
		PR->iExplore=30;		
		
//Название
		ClearRoll();
		int Count=0;
		for (int i=1; i<=iMaxProvinceName; i++)
		{			
			if ((*(pNames+i))&&((pFirstProvinceName+i)->CheckType(PR->ipType))&&((pFirstProvinceName+i)->CheckTerrain(PR->ipTerrain)))
			{
				bool bName=false;
				for (int j=0; j<MAX_NAME_TYPE; j++)
				{
					if ((pFirstProvinceName+i)->arType[j]==2)
					{bName=true;};
				};
				if ((bName)&&(((pFirstProvinceName+i)->iCoast==0)||(((pFirstProvinceName+i)->iCoast==1)&&(PR->Coastal==1))))
				{
				RollerN[Count]=i;
				RollerP[Count]=(pFirstProvinceName+i)->iRarity;
				Count++;
				};
			};
		};
		int iName=Roll(false);
		PR->sName=new char [sizeof (pFirstProvinceName+iName)->sName];
		for (int i=0; i<sizeof (pFirstProvinceName+iName)->sName; i++)
		{
			*(PR->sName+i)=(pFirstProvinceName+iName)->sName[i];
		};
		*(pNames+iName)=false;		

//Доход
		PR->iGoldInc=8;
		PR->iGemInc=0;
		PR->PlaceGuard(5,0);
		PR->arBuilding[0]=10;
	}
	else
	{

	//Генерация ландшафта
//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Генерация ландшафта...");
//	Txt++;

//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Вода и суша...");
//	Txt++;

	for (int i=1; i<=MapSize; i++)
	{
		(pFirstProvince+i)->ipTerrain=1;
	};
	mpMap.Water=MapSize*mpMap.Water/100;
	int WaterPlaced=0;
	switch (mpMap.Type)
	{
	case 1:
		for (int i=1; i<=mpMap.X; i++)
		{
			(pFirstProvince+i)->ipTerrain=0;
			(pFirstProvince+1+MapSize-i)->ipTerrain=0;
		};
		for (int i=0; i<mpMap.Y; i++)
		{
			(pFirstProvince+1+i*mpMap.X)->ipTerrain=0;
			(pFirstProvince+(i+1)*mpMap.X)->ipTerrain=0;
		};
		WaterPlaced=mpMap.X*2+(mpMap.Y-1)*2;
		iFloodSuit=1;
		iFloodType=0;
		iFloodDeep=mpMap.Water/10+5;
		while (WaterPlaced<mpMap.Water)
		{
			int Try=Random(MapSize)+1;
			while ((pFirstProvince+Try)->ipTerrain>0)
			{Try=Random(MapSize)+1;};
			iFloodMax=mpMap.Water-WaterPlaced;
			WaterPlaced+=Flooder(Try, iFloodDeep*15/mpMap.Dissolve, 0);
		};

		break;
	case 2:
		iFloodSuit=1;
		iFloodType=0;
		iFloodDeep=mpMap.Water/10+5;
		while (WaterPlaced<mpMap.Water)
		{			
			iFloodMax=mpMap.Water-WaterPlaced;
			WaterPlaced+=Flooder(Random(MapSize)+1, iFloodDeep*15/mpMap.Dissolve, 0);
		};
		break;

	};
	mpMap.Isles=mpMap.Isles*mpMap.Water/100;
	while (mpMap.Isles>0)
	{
		int Try=Random(MapSize)+1;
		while ((pFirstProvince+Try)->ipTerrain>0)
		{Try=Random(MapSize)+1;};
		(pFirstProvince+Try)->ipTerrain=1;
		Try=Random(MapSize)+1;
		while ((pFirstProvince+Try)->ipTerrain==0)
		{Try=Random(MapSize)+1;};
		(pFirstProvince+Try)->ipTerrain=0;
		mpMap.Isles--;
	};

//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Горы и холмы...");
//	Txt++;
	mpMap.Hill=(MapSize-WaterPlaced)*mpMap.Hill/100;
	int Mountain=mpMap.Hill/5;
	mpMap.Hill-=Mountain;	
	int HillPlaced=0, MountainPlaced=0;
	iFloodSuit=1;
	while ((HillPlaced<mpMap.Hill)||(MountainPlaced<Mountain))
	{
		
		iFloodType=MOUNTAIN_ID;
		iFloodDeep=6+Mountain/10;
		iFloodMax=Mountain-MountainPlaced;
		int Try=Random(MapSize)+1;
		while ((pFirstProvince+Try)->ipTerrain!=1)
		{Try=Random(MapSize)+1;};
		if (MountainPlaced<Mountain)
		{
			MountainPlaced+=Flooder(Try,1,0);
		};
		if (HillPlaced<mpMap.Hill)
		{
			iFloodType=3;
			iFloodDeep=mpMap.Hill/10+6;
			iFloodMax=mpMap.Hill-HillPlaced;
			HillPlaced+=Flooder(Try,iFloodDeep*10/mpMap.Dissolve,0);
		};
	};


//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Леса...");
//	Txt++;
	mpMap.Forest=(MapSize-WaterPlaced)*mpMap.Forest/100;
	if (mpMap.Forest+HillPlaced+WaterPlaced>MapSize)
	{
		mpMap.Forest=0;
	};
	int ForestPlaced=0;
	iFloodSuit=1;
	iFloodType=2;
	iFloodDeep=mpMap.Forest/10+6;
	while (ForestPlaced<mpMap.Forest)
	{		
		iFloodMax=mpMap.Forest-ForestPlaced;
		int Try=Random(MapSize)+1;
		while ((pFirstProvince+Try)->ipTerrain!=1)
		{Try=Random(MapSize)+1;};		
		ForestPlaced+=Flooder(Try,iFloodDeep*10/mpMap.Dissolve,0);
	};


//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Болота...");
//	Txt++;

	mpMap.Swamp=(MapSize-WaterPlaced)*mpMap.Swamp/100;
	if (mpMap.Swamp+ForestPlaced+HillPlaced+WaterPlaced>MapSize)
	{
		mpMap.Swamp=(MapSize-(ForestPlaced+HillPlaced+WaterPlaced))/2;		
	};
	int SwampPlaced=0;
	iFloodSuit=1;
	iFloodType=4;
	iFloodDeep=mpMap.Swamp/10+6;

	while (SwampPlaced<mpMap.Swamp)
	{		
		iFloodMax=mpMap.Swamp-SwampPlaced;
		int Try=Random(MapSize)+1;
		while ((pFirstProvince+Try)->ipTerrain!=1)
		{Try=Random(MapSize)+1;};		
		SwampPlaced+=Flooder(Try,iFloodDeep*10/mpMap.Dissolve,0);
	};
	NewProgress=300;

//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Расстановка родовых замков...");
//	Txt++;

	int *MapCastle=new int [MapSize+1];
	for (int i=1; i<=MapSize; i++)
	{
		*(MapCastle+i)=0;
	};


	int MinNeigh=6;
	int MinDist=(mpMap.X+mpMap.Y)/2+1-CurGame.iMaxPlayers/2;
	int Castles=0;
	int arCastle[MAX_PLAYERS+1];
//	MovePlayer=0; WalkType=0; WalkSpeed=1;
	bool bCycle=true, bWork;
	while (bCycle)
	{
		bool bRepeat=true;
		int iRepeater=0;
		for (int i=1; i<=MapSize; i++)
		{
			(pFirstProvince+i)->bWarning=true;
		};
		while (bRepeat)
		{

		bWork=true;
		int Places=0;
		Castles=0;
		for (int i=1; i<=MAX_PLAYERS; i++)
		{arCastle[i]=0;};

		for (int i=1; i<=MapSize; i++)
		{
			(pFirstProvince+i)->iFort=0;
			if (((pFirstProvince+i)->ipTerrain>0)&&((pFirstProvince+i)->ipTerrain<MOUNTAIN_ID))
			{
				int NEI=0;
				bool BigLand=false;
				for (int j=0; j<MAX_NEIGHBOUR; j++)
				{
					if (((pFirstProvince+i)->arNeighbour[j]>0)&&
						((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipTerrain>0)&&
						((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipTerrain<MOUNTAIN_ID))
					{
						int NEI1=0;
						for (int k=0; k<MAX_NEIGHBOUR; k++)
						{
							if (((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->arNeighbour[k]>0)&&
								((pFirstProvince+(pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->arNeighbour[k])->ipTerrain>0)&&
								((pFirstProvince+(pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->arNeighbour[k])->ipTerrain<MOUNTAIN_ID))
							{
								NEI1++;								
							};
						};
						if (NEI1>3)
						{BigLand=true;};
						NEI++;
					};
				};
				if ((NEI>=MinNeigh)&&(BigLand))
				{
					(pFirstProvince+i)->iFort=1;
					Places++;
				};
			};
		};		
		if (Places>=CurGame.iMaxPlayers)
		{
//			textprintf(screen,Font1,0,700,COLOR_2,"Дистанция: %d, Соседи: %d   ",MinDist, MinNeigh);
//			rectfill(screen,0,725,200,728,COLOR_1);
			int Try=Random(MapSize)+1;
			while (((pFirstProvince+Try)->iFort!=1)||(!(pFirstProvince+Try)->bWarning))
			{Try=Random(MapSize)+1;};
			arCastle[1]=Try;
			(pFirstProvince+Try)->iFort=2;
			(pFirstProvince+Try)->bWarning=false;
			arCastle[1]=Try;

	//		(pFirstProvince+Try)->ipPlayer=1;
	//		arPlayer[1].ipProvince=Try;
			Castles++;
			for (int i=1; i<=MapSize; i++)
			{(pFirstProvince+i)->Work=1;};
			for (int PL=2; PL<=CurGame.iMaxPlayers; PL++)
			{
			//	PROCESSBAR(PL,35);
				if (bWork)
				{
//					rectfill(screen,PL*10,725,PL*10+3,728,COLOR_2);
					ClearMoveMap();
					MakeMap(arCastle[PL-1],0);
					for (int j=1; j<=MapSize; j++)
					{
						if ((pFirstProvince+j)->iFort==1)
						{
							if (*(pMoveMap+j)<0)
							{
								if ((MinNeigh>3)&&(CurGame.iGameType!=2))
								{(pFirstProvince+j)->Work*=(mpMap.X+mpMap.Y)/2;}
								else
								{
									(pFirstProvince+j)->Work=0;
									(pFirstProvince+j)->iFort=0;
								};
							}
							else
							{
								if (*(pMoveMap+j)<MinDist)
								{
									(pFirstProvince+j)->Work=0;
									(pFirstProvince+j)->iFort=0;
								}
								else
								{								
									(pFirstProvince+j)->Work*=*((pMoveMap+j)-1);
								};
							};							
						};
					};				
					int Found=0, MaxDist=0;
					for (int i=1; i<=MapSize; i++)
					{
						if (((pFirstProvince+i)->iFort==1)&&((pFirstProvince+i)->Work>MaxDist))
						{
							Found=i;
							MaxDist=(pFirstProvince+i)->Work;
						};					
					};
					if (Found>0)
					{
						(pFirstProvince+Found)->iFort=2;
						arCastle[PL]=Found;
						Castles++;
					}
					else
					{
						bWork=false;
					};

				};
			};
			if (Castles>=CurGame.iMaxPlayers)
			{
				bCycle=false;
				bRepeat=false;
			};
			if (Places<iRepeater+3)
			{
				bRepeat=false;
			};
			iRepeater++;
		}
		else
		{bRepeat=false;};
		};
		if ((MinNeigh>4)||((MinNeigh>3)&&(MinDist<6)))
		{MinNeigh--;}
		else
		{
			MinNeigh=6;
			MinDist--;
			if (NewProgress<1000)
			{NewProgress++;};
		};		
	};
	for (int i=1; i<=MapSize; i++)
	{
		(pFirstProvince+i)->bWarning=true;
		(pFirstProvince+i)->iFort=0;
	};
	ClearMoveMap();
	int ForestCastle=0, HillCastle=0, SwampCastle=0, PlainCastle=0;
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		arPlayer[i].pFog=new int [MapSize+1];
		*(arPlayer[i].pFog)=0;
		for (int j=1; j<=MapSize; j++)
		{
			*(arPlayer[i].pFog+j)=2;
		};
		*(arPlayer[i].pFog+arCastle[i])=0;
		arPlayer[i].ipProvince=arCastle[i];
		MakeMap(arCastle[i],0);		
		(pFirstProvince+arCastle[i])->ipPlayer=i;
		(pFirstProvince+arCastle[i])->ipType=7;
		(pFirstProvince+arCastle[i])->iExplore=50;
		if (arPlayer[i].Control==1)
		{

	//!!Тест квестов!!
			/*for (int Quest=54; Quest<57; Quest++)
			{
				(pFirstProvince+arCastle[i])->PlaceSite((pFirstQuest+Quest)->iGiver,1,Quest,0,0);
			};*/
	//***


			(pFirstProvince+arCastle[i])->sName=new char [sizeof arHumans[arPlayer[i].iColor].sLand];
			for (int j=0; j<sizeof arHumans[arPlayer[i].iColor].sLand; j++)
			{
				*((pFirstProvince+arCastle[i])->sName+j)=arHumans[arPlayer[i].iColor].sLand[j];
			};
		}
		else
		{
			if (arPlayer[i].Control==2)
			{
				(pFirstProvince+arCastle[i])->sName=new char [sizeof (pFirstPlayerProfile+arPlayer[i].ipProfile)->sLand];
				for (int j=0; j<sizeof (pFirstPlayerProfile+arPlayer[i].ipProfile)->sLand; j++)
				{
					*((pFirstProvince+arCastle[i])->sName+j)=(pFirstPlayerProfile+arPlayer[i].ipProfile)->sLand[j];
				};
				switch (arPlayer[i].ipProfile)
				{
				case 5:
					(pFirstProvince+arCastle[i])->ipTerrain=2;
					break;
				case 9:
					(pFirstProvince+arCastle[i])->PlaceSite(35,3,0,-1,0);
					break;
				case 12:
					(pFirstProvince+arCastle[i])->ipTerrain=2;
					(pFirstProvince+arCastle[i])->PlaceSite(43,3,0,-1,0);					
					break;
				case 13:
					(pFirstProvince+arCastle[i])->ipTerrain=4;
					(pFirstProvince+arCastle[i])->PlaceSite(45,0,0,-1,0);
					break;
				case 14:
					(pFirstProvince+arCastle[i])->ipTerrain=3;
					(pFirstProvince+arCastle[i])->PlaceSite(44,3,0,-1,0);
					break;
				};
			}
			else
			{MakeCastleName(arCastle[i],pNames);};
		};
		
		(pFirstProvince+arCastle[i])->ipRace=1;
		(pFirstProvince+arCastle[i])->iPopulation=10;
		(pFirstProvince+arCastle[i])->pGarrison=new dcHero();
		(pFirstProvince+arCastle[i])->pGarrison->ipPlayer=i;
		(pFirstProvince+arCastle[i])->pGarrison->arGarrison[0]=6;
		(pFirstProvince+arCastle[i])->pGarrison->ipProvince=arCastle[i];
		(pFirstProvince+arCastle[i])->pGarrison->pProvince=(pFirstProvince+arCastle[i]);

		(pFirstProvince+arCastle[i])->iFort=arPlayer[i].iWall;
		
		switch ((pFirstProvince+arCastle[i])->ipTerrain)
		{
			case 2:
				(pFirstProvince+arCastle[i])->iGoldInc=10;
				(pFirstProvince+arCastle[i])->iGemInc=5;
				(pFirstProvince+arCastle[i])->iExplore=45;
				ForestCastle++;
				break;
			case 3:	
				(pFirstProvince+arCastle[i])->iGoldInc=12;
				(pFirstProvince+arCastle[i])->iGemInc=4;
				(pFirstProvince+arCastle[i])->iExplore=50;
				HillCastle++;
				break;
			case 4:
				(pFirstProvince+arCastle[i])->iGoldInc=8;
				(pFirstProvince+arCastle[i])->iGemInc=6;
				(pFirstProvince+arCastle[i])->iExplore=40;
				SwampCastle++;
				break;
			default:
				(pFirstProvince+arCastle[i])->iGoldInc=15;
				(pFirstProvince+arCastle[i])->iGemInc=3;
				(pFirstProvince+arCastle[i])->iExplore=60;
				PlainCastle++;
				break;
		};
		if (arPlayer[i].Control==1)
		{
			(pFirstProvince+arCastle[i])->iGoldInc+=arDifficulty[arPlayer[i].iDifficulty].iGoldAdd;
			(pFirstProvince+arCastle[i])->iGemInc+=arDifficulty[arPlayer[i].iDifficulty].iGemAdd;
			(pFirstProvince+arCastle[i])->iPopulation+=arDifficulty[arPlayer[i].iDifficulty].iPopulation;


//			(pFirstProvince+arCastle[i])->PlaceSite((pFirstQuest+50)->iGiver,3,50,0,0);



			//switch (arPlayer[i].iDifficulty)
			//{
			//case 0:
			//	(pFirstProvince+arCastle[i])->iGoldInc+=15;
			//	(pFirstProvince+arCastle[i])->iGemInc=+5;
			//	(pFirstProvince+arCastle[i])->iPopulation+=1000;
			//	break;
			//case 1:
			//	(pFirstProvince+arCastle[i])->iGoldInc+=10;
			//	(pFirstProvince+arCastle[i])->iGemInc+=3;
			//	(pFirstProvince+arCastle[i])->iPopulation+=500;

			//	break;
			//case 2:
			//	(pFirstProvince+arCastle[i])->iGoldInc+=5;
			//	(pFirstProvince+arCastle[i])->iGemInc+=1;
			//	(pFirstProvince+arCastle[i])->iPopulation+=100;
			//	break;
			//};
		};
		int NEI=0;
		for (int j=0; j<MAX_NEIGHBOUR; j++)
		{
			if ((pFirstProvince+arCastle[i])->arNeighbour[j]>0)
			{
				*(arPlayer[i].pFog+(pFirstProvince+arCastle[i])->arNeighbour[j])=0;

			};
			if (((pFirstProvince+arCastle[i])->arNeighbour[j]>0)&&
				((pFirstProvince+(pFirstProvince+arCastle[i])->arNeighbour[j])->ipTerrain>0)&&
				((pFirstProvince+(pFirstProvince+arCastle[i])->arNeighbour[j])->ipTerrain<MOUNTAIN_ID))
			{
				NEI++;
			};
		};
		if (NEI<3)
		{
			for (int j=0; j<MAX_NEIGHBOUR; j++)
			{
				if ((NEI<3)&&((pFirstProvince+arCastle[i])->arNeighbour[j]>0)&&
				((pFirstProvince+(pFirstProvince+arCastle[i])->arNeighbour[j])->ipTerrain==0))
				{
					(pFirstProvince+(pFirstProvince+arCastle[i])->arNeighbour[j])->ipTerrain=1;
					*(pMoveMap+(pFirstProvince+arCastle[i])->arNeighbour[j])=2;
					NEI++;
				};
				if ((NEI<3)&&((pFirstProvince+arCastle[i])->arNeighbour[j]>0)&&
				((pFirstProvince+(pFirstProvince+arCastle[i])->arNeighbour[j])->ipTerrain==MOUNTAIN_ID))
				{
					(pFirstProvince+(pFirstProvince+arCastle[i])->arNeighbour[j])->ipTerrain=3;
					*(pMoveMap+(pFirstProvince+arCastle[i])->arNeighbour[j])=2;
					NEI++;
				};				
			};			
		};
	};
	for (int j=1; j<=MapSize; j++)
	{
		if (*(pMoveMap+j)<0)
		{
			*(pMoveMap+j)=8;
		}
	};

//Расстановка ресурсов
	int arRes[20];
	for (int i=1; i<=iMaxResource; i++)
	{
		arRes[i]=0;
	};
	//По 2 простых ресурса рядом с каждым замком
	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		int arNeigh[100];
		bool arTerr[5][2];
		for (int i=1; i<5; i++)
		{
			arTerr[i][0]=false;
			arTerr[i][1]=false;
		};
		int NEI=0;
		for (int j=0; j<MAX_NEIGHBOUR; j++)
		{
			if (((pFirstProvince+arCastle[PL])->arNeighbour[j]>0)&&
				((pFirstProvince+(pFirstProvince+arCastle[PL])->arNeighbour[j])->ipTerrain>0)&&
				((pFirstProvince+(pFirstProvince+arCastle[PL])->arNeighbour[j])->ipTerrain<MOUNTAIN_ID))
			{
				arNeigh[NEI]=(pFirstProvince+arCastle[PL])->arNeighbour[j];
				arTerr[(pFirstProvince+(pFirstProvince+arCastle[PL])->arNeighbour[j])->ipTerrain][0]=true;
				NEI++;
			};
		};
		int NEI1=NEI;
		for (int i=0; i<NEI; i++)
		{
			for (int j=0; j<MAX_NEIGHBOUR; j++)
			{
				if (((pFirstProvince+arNeigh[i])->arNeighbour[j]>0)&&
				((pFirstProvince+arNeigh[i])->arNeighbour[j]!=arCastle[PL])&&
				((pFirstProvince+(pFirstProvince+arNeigh[i])->arNeighbour[j])->arSite[0][0]==0)&&
				((pFirstProvince+(pFirstProvince+arNeigh[i])->arNeighbour[j])->ipTerrain>0)&&
				((pFirstProvince+(pFirstProvince+arNeigh[i])->arNeighbour[j])->ipTerrain<MOUNTAIN_ID))
				{
					bool bAdd=true;
					for (int k=0; k<NEI1; k++)
					{
						if ((pFirstProvince+arNeigh[i])->arNeighbour[j]==arNeigh[k])
						{bAdd=false;};
					};
					if (bAdd)
					{
						arNeigh[NEI1]=(pFirstProvince+arNeigh[i])->arNeighbour[j];
						arTerr[(pFirstProvince+(pFirstProvince+arNeigh[i])->arNeighbour[j])->ipTerrain][1]=true;
						NEI1++;
					};
				};
			};
		};
		int Count=0, Land=0;
		for (int i=1; i<5; i++)
		{
			if ((arTerr[i][0])||(arTerr[i][1]))
			{
				Count++;
				Land=i;
			};
		};
		if (Count<2)
		{
			int Morph=Random(4)+1;
			while (Morph==Land)
			{
				Morph=Random(4)+1;
			};
			Land=Random(NEI1-1);
			(pFirstProvince+arNeigh[Land])->ipTerrain=Morph;
			if (Land>=NEI)
			{arTerr[Morph][1]=true;}
			else
			{arTerr[Morph][0]=true;};
		};
		Count=0, Land=0;
		for (int i=1; i<5; i++)
		{
			if (arTerr[i][1])
			{
				Count++;
				Land=i;
			};
		};
		if (Count>1)
		{
			Count=0;
			Land=0;
			while (Count<2)
			{
				int Try=Random(4)+1;
				if ((arTerr[Try][1])&&(Try!=Land))
				{
					Count++;
					Land=Try;
					Try=NEI+Random(NEI1-NEI);
					while (((pFirstProvince+arNeigh[Try])->ipTerrain!=Land))
					{
						Try=NEI+Random(NEI1-NEI);
					};					
					switch (Land)
					{
						case 1:
							(pFirstProvince+arNeigh[Try])->arSite[0][0]=3;
							arRes[3]++;
							break;
						case 2:
							(pFirstProvince+arNeigh[Try])->arSite[0][0]=2;
							arRes[2]++;
							break;
						case 3:
							(pFirstProvince+arNeigh[Try])->arSite[0][0]=1;
							arRes[1]++;
							break;
						case 4:
							(pFirstProvince+arNeigh[Try])->arSite[0][0]=4;
							arRes[4]++;
							break;
					};
					(pFirstProvince+arNeigh[Try])->arSite[0][1]=1;
					(pFirstProvince+arNeigh[Try])->arSite[0][4]=0;
					(pFirstProvince+arNeigh[Try])->arSiteGuard[0]=NULL;
				};
			};
		}
		else
		{
			if (Land>0)
			{
			int Try=NEI+Random(NEI1-NEI);
			while (((pFirstProvince+arNeigh[Try])->ipTerrain!=Land))
			{
				//Зациклило
				Try=NEI+Random(NEI1-NEI);
			};					
			switch (Land)
			{
				case 1:
					(pFirstProvince+arNeigh[Try])->arSite[0][0]=3;
					arRes[3]++;
					break;
				case 2:
					(pFirstProvince+arNeigh[Try])->arSite[0][0]=2;
					arRes[2]++;
					break;
				case 3:
					(pFirstProvince+arNeigh[Try])->arSite[0][0]=1;
					arRes[1]++;
					break;
				case 4:
					(pFirstProvince+arNeigh[Try])->arSite[0][0]=4;
					arRes[4]++;
					break;
			};
			(pFirstProvince+arNeigh[Try])->arSite[0][1]=1;
			(pFirstProvince+arNeigh[Try])->arSite[0][4]=0;
			(pFirstProvince+arNeigh[Try])->arSite[0][2]=0;
			(pFirstProvince+arNeigh[Try])->arSite[0][3]=0;
			(pFirstProvince+arNeigh[Try])->arSite[0][5]=0;
			(pFirstProvince+arNeigh[Try])->arSiteGuard[0]=NULL;
			Try=Random(NEI);
			while (((pFirstProvince+arNeigh[Try])->ipTerrain==Land)||(!arTerr[(pFirstProvince+arNeigh[Try])->ipTerrain][0]))
			{
				Try=Random(NEI);
			};
			if (*(pMoveMap+arNeigh[Try])==2)
			{
				*(pMoveMap+arNeigh[Try])+=1;
			};
			switch ((pFirstProvince+arNeigh[Try])->ipTerrain)
			{
				case 1:
					(pFirstProvince+arNeigh[Try])->arSite[0][0]=3;
					arRes[3]++;
					break;
				case 2:
					(pFirstProvince+arNeigh[Try])->arSite[0][0]=2;
					arRes[2]++;
					break;
				case 3:
					(pFirstProvince+arNeigh[Try])->arSite[0][0]=1;
					arRes[1]++;
					break;
				case 4:
					(pFirstProvince+arNeigh[Try])->arSite[0][0]=4;
					arRes[4]++;
					break;
			};
			(pFirstProvince+arNeigh[Try])->arSite[0][1]=1;
			(pFirstProvince+arNeigh[Try])->arSite[0][4]=0;
			(pFirstProvince+arNeigh[Try])->arSite[0][2]=0;
			(pFirstProvince+arNeigh[Try])->arSite[0][3]=0;
			(pFirstProvince+arNeigh[Try])->arSite[0][5]=0;
			(pFirstProvince+arNeigh[Try])->arSiteGuard[0]=NULL;
		};	
		};
	};
	//Остальные ресурсы
	for (int i=1; i<=iMaxResource; i++)
	{
		int MaxDrop=1+Random(2);
		int Land=0;
		int FreeLand;
		switch (i)
		{
		case 1:
		case 6:
		case 7:
			MaxDrop+=HillPlaced/7/((pFirstResource+i)->iRarity+1);
			Land=3;
			FreeLand=HillPlaced-arRes[1]-arRes[6]-arRes[7]-HillCastle;
			break;
		case 2:		
		case 8:
			MaxDrop+=ForestPlaced/7/((pFirstResource+i)->iRarity+1);
			Land=2;
			FreeLand=ForestPlaced-arRes[2]-arRes[8]-ForestCastle;
			break;
		case 4:
		case 5:
		case 9:		
			MaxDrop+=SwampPlaced/7/((pFirstResource+i)->iRarity+1);
			Land=4;
			FreeLand=SwampPlaced-arRes[4]-arRes[5]-arRes[9]-SwampCastle;
			break;
		case 3:		
			MaxDrop+=(MapSize-HillPlaced-ForestPlaced-SwampPlaced-MountainPlaced-WaterPlaced)/15;
			Land=1;
			FreeLand=MapSize-HillPlaced-ForestPlaced-SwampPlaced-WaterPlaced-MountainPlaced-arRes[3]-PlainCastle;
			break;			
		};
		MaxDrop+=mpMap.Resource/3;
		MaxDrop+=MaxDrop*(mpMap.Resource-2)/5;
		while ((arRes[i]<MaxDrop)&&(FreeLand>2))
		{
			int Try=Random(MapSize)+1;
			while (((pFirstProvince+Try)->ipTerrain!=Land)||((pFirstProvince+Try)->arSite[0][0]>0)||((pFirstProvince+Try)->ipType>0))
			{
				Try=Random(MapSize)+1;
			};		
			(pFirstProvince+Try)->arSite[0][0]=i;			
			(pFirstProvince+Try)->arSite[0][2]=0;
			(pFirstProvince+Try)->arSite[0][3]=0;
			(pFirstProvince+Try)->arSite[0][5]=0;
			int Calc=Random(20)+40+(pFirstResource+i)->iRarity*30-*(pMoveMap+Try)*10;
			if (Calc>100)
			{Calc=100;};
			if (Calc<0)
			{Calc=0;};
			(pFirstProvince+Try)->arSite[0][1]=Calc;
			if ((pFirstResource+i)->iRarity*2+3>*(pMoveMap+Try))
			{
				ClearRoll();
				int Counter=0;
				for (int j=0; j<MAX_SITE_GUARD; j++)
				{
					if ((pFirstSite+i)->arGuard[j][0]>0)
					{
						RollerN[Counter]=(pFirstSite+i)->arGuard[j][0];
						RollerP[Counter]=(pFirstSite+i)->arGuard[j][1];
						Counter++;
					};
				};
				(pFirstProvince+Try)->arSite[0][4]=Roll(false);
				int GrdLvl=(pFirstResource+i)->iRarity;//*2+2-*(pMoveMap+Try);
				if (GrdLvl>3)
				{GrdLvl=3;};
				if (GrdLvl<0)
				{GrdLvl=0;};
				(pFirstProvince+Try)->PlaceSiteGuard((pFirstGuardType+(pFirstProvince+Try)->arSite[0][4])->arGuard[GrdLvl],0,(pFirstGuardType+(pFirstProvince+Try)->arSite[0][4])->iKarma);
			}
			else
			{
				(pFirstProvince+Try)->arSite[0][4]=0;
				(pFirstProvince+Try)->arSiteGuard[0]=NULL;
			};
			arRes[i]++;
			FreeLand--;
		};
	};
	};
	NewProgress=1000;

	int *QuestList= new int [iMaxQuest+1];
	for (int i=0; i<=iMaxQuest; i++)
	{
		if (((pFirstQuest+i)->iGiver>0)&&((pFirstQuest+i)->iType!=7)&&((pFirstQuest+i)->iType!=8))
		{*(QuestList+i)=1;}
		else
		{*(QuestList+i)=0;};
	};
	

//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Заселение провинций...");
//	Txt++;
//	pFirstProvince->sName="Великое Ничто";
//	pFirstProvince->sName=new char[sizeof (pFirstPhrase+327)->str];
	pFirstProvince->sName=(pFirstPhrase+327)->str;
	for (int i=1; i<=MapSize; i++)
	{
		dcProvince * PR=(pFirstProvince+i);
		if (PR->ipTerrain==0)
		{
			bool Lake=true;
			for (int j=0; j<MAX_NEIGHBOUR; j++)
			{
				if ((PR->arNeighbour[j]>0)&&((pFirstProvince+PR->arNeighbour[j])->ipTerrain==0))
				{Lake=false;};
			};			
			if (Lake)
			{
				PR->sName=new char[sizeof (pFirstPhrase+325)->str];
				PR->Coastal=bLakeShip;
				sprintf(PR->sName,(pFirstPhrase+325)->str);
			}
			else
			{
				PR->sName=new char[sizeof (pFirstPhrase+324)->str];
				PR->Coastal=true;
				sprintf(PR->sName,(pFirstPhrase+324)->str);			
			};
		};
		if (PR->ipTerrain==7)
		{
			PR->sName=new char[sizeof (pFirstPhrase+326)->str];
			sprintf(PR->sName,(pFirstPhrase+326)->str);
		};
	};

	for (int i=1; i<=MapSize; i++)
	{
		NewProgress=1000+i*1000/MapSize;
		dcProvince * PR=(pFirstProvince+i);
		if ((PR->ipTerrain>0)&&(PR->ipTerrain<7))
		{
		int Level=*(pMoveMap+i);
//Прибрежная
		PR->Coastal=false;
		for (int i=0; i<MAX_NEIGHBOUR; i++)
		{
			if ((PR->arNeighbour[i]>0)&&((pFirstProvince+PR->arNeighbour[i])->ipTerrain==0)&&((pFirstProvince+PR->arNeighbour[i])->Coastal))
			{
				PR->Coastal=true;
			};
		};
//Тип провинции
		if (PR->ipType==0)
		{
//		textprintf(screen,Font1,0,700,COLOR_2,"Тип            ");
		ClearRoll();
		Counter=0;
		for (int j=1; j<=iMaxProvinceType; j++)
		{			
			if (((pFirstProvinceType+j)->arTerrain[PR->ipTerrain]>0)&&((pFirstProvinceType+j)->iDifficult<Level-1))
			{
				RollerN[Counter]=j;
				RollerP[Counter]=(pFirstProvinceType+j)->arTerrain[PR->ipTerrain];
				if ((Prf)&&(Prf->iProvType==j))
				{RollerP[Counter]+=Prf->iProvPoss;};
				if (WorldRules.iMode>0)
				{
					for (int k=0; k<MAX_WORLD_PROVTYPES; k++)
					{
						if ((pFirstWorldRules+WorldRules.iMode)->arProvTypes[k][0]==j)
						{RollerP[Counter]+=RollerP[Counter]*(pFirstWorldRules+WorldRules.iMode)->arProvTypes[k][1]/100;}
					}
				}
				Counter++;				
			};
			PR->ipType=Roll(false);
		//	PR->ipType=Roll(false,PR->iNumber,CurGame.iSeed);
		};
		int Level1=Level-(pFirstProvinceType+PR->ipType)->iDifficult;
		if (Level1>3)
		{Level1=3;};
		if (Level1<0)
		{Level1=0;};
		PR->ipRace=(pFirstProvinceType+PR->ipType)->ipRace;
		PR->iPopulation=(pFirstProvinceType+PR->ipType)->iPopulation+Level1*(pFirstProvinceType+PR->ipType)->iLevelPop;
		if (PR->iPopulation>0)
		{
			PR->iPopulation=PR->iPopulation*(pFirstTerrain+PR->ipTerrain)->iPopulationMod/100;
			PR->iPopulation+=Random(PR->iPopulation/5);
			PR->iPopulation+=PR->iPopulation*(mpMap.Population-2)/5;
		};
		PR->iExplore=(pFirstProvinceType+PR->ipType)->iExplored+Level1*(pFirstProvinceType+PR->ipType)->iLevelExplored+Random(5);
		PR->iExplore=PR->iExplore*(pFirstTerrain+PR->ipTerrain)->iExploreMod/100;
		PR->iExplore+=PR->iExplore*(mpMap.Population-2)/5;
//		PR->iPopulation=PR->iPopulation*(pFirstTerrain+PR->ipTerrain)->iPopulationMod/100;
//Название
//		textprintf(screen,Font1,0,700,COLOR_2,"Название        ");
		ClearRoll();
		int Count=0; bool Isle=true;
		for (int i=0; i<MAX_NEIGHBOUR; i++)
		{
			if ((PR->arNeighbour[i]>0)&&((pFirstProvince+PR->arNeighbour[i])->ipTerrain!=0))
			{Isle=false;};
		};
		if (Isle)
		{
			for (int i=1; i<=iMaxProvinceName; i++)
			{			
				if ((*(pNames+i))&&((pFirstProvinceName+i)->CheckType(PR->ipType))&&((pFirstProvinceName+i)->CheckTerrain(PR->ipTerrain))&&((pFirstProvinceName+i)->iCoast==2))
				{				
					RollerN[Count]=i;
					RollerP[Count]=(pFirstProvinceName+i)->iRarity;
					Count++;
				};
			};
		};
		if (Count==0)
		{		
			for (int i=1; i<=iMaxProvinceName; i++)
			{			
				if ((*(pNames+i))&&((pFirstProvinceName+i)->CheckType(PR->ipType))&&((pFirstProvinceName+i)->CheckTerrain(PR->ipTerrain)))
				{
					if (((pFirstProvinceName+i)->iCoast==0)||(((pFirstProvinceName+i)->iCoast==1)&&(PR->Coastal==1)))
					{
					RollerN[Count]=i;
					RollerP[Count]=(pFirstProvinceName+i)->iRarity;
					Count++;
					};
				};
			};
		};
		if (Count>0)
		{
		int iName=Roll(false);
//		int iName=Roll(false,PR->iNumber,CurGame.iSeed);
		PR->sName=new char [sizeof (pFirstProvinceName+iName)->sName];
		for (int i=0; i<sizeof (pFirstProvinceName+iName)->sName; i++)
		{
			*(PR->sName+i)=(pFirstProvinceName+iName)->sName[i];
		};
//		PR->sName=(pFirstProvinceName+iName)->sName;
		*(pNames+iName)=false;
		}
		else
		{
			int Pref=Random(iMaxPrefix), Suf=Random(iMaxSuffix);
			char sTmp [20];
			sprintf(sTmp,"%s%s",(pFirstPrefix+Pref)->str,(pFirstSuffix+Suf)->str);
			PR->sName=new char [sizeof (sTmp)];
			for (int i=0; i<sizeof (sTmp); i++)
			{
				*(PR->sName+i)=sTmp[i];
			};
		};

//Охранник
//		textprintf(screen,Font1,0,700,COLOR_2,"Охранник  %d",PR->ipType);
		int Grd=0;
		ClearRoll();
		while ((pFirstProvinceType+PR->ipType)->arGuard[Grd][0]>0)
		{
			RollerN[Grd]=Grd+1;
			RollerP[Grd]=(pFirstProvinceType+PR->ipType)->arGuard[Grd][1];
			Grd++;
		};
		Grd=Roll(false)-1;
		PR->iDefender=Grd;

		int GrdLvl=0;
		int GrdPower;
		int Typoz=(pFirstProvinceType+PR->ipType)->arGuard[Grd][0];
		if (Level<0)
		{GrdPower=15;}
		else
		{GrdPower=1+(Level-2)*3+Random(Level-2)-Random(Level-2);};
		if (GrdPower>mpMap.Neutrals)
		{GrdPower=mpMap.Neutrals;};

		for (int k=1; k<4; k++)
		{
			if (abs((pFirstGuard+(pFirstGuardType+Typoz)->arGuard[GrdLvl])->Power-GrdPower)>=abs((pFirstGuard+(pFirstGuardType+Typoz)->arGuard[k])->Power-GrdPower))
			{GrdLvl=k;};
		};
		if (GrdLvl<0)
		{GrdLvl=0;};
		if (GrdLvl>3)
		{GrdLvl=3;};		
		PR->PlaceGuard((pFirstGuardType+(pFirstProvinceType+PR->ipType)->arGuard[Grd][0])->arGuard[GrdLvl],(pFirstGuardType+(pFirstProvinceType+PR->ipType)->arGuard[Grd][0])->iKarma);

//Доход
//		textprintf(screen,Font1,0,700,COLOR_2,"Доход         ");
		PR->iGoldInc=(pFirstTerrain+PR->ipTerrain)->iGoldInc+(pFirstProvinceType+PR->ipType)->iGold;
		PR->iGoldInc=PR->iGoldInc*PR->iSize/MedSize;
		PR->iGoldInc+=2-Random(5);
		if (PR->iGoldInc<=0)
		{PR->iGoldInc=1;};

		PR->iGemInc=(pFirstTerrain+PR->ipTerrain)->iGemInc+(pFirstProvinceType+PR->ipType)->iGem;
		PR->iGemInc=PR->iGemInc*PR->iSize/MedSize;
		PR->iGemInc-=Random(4);
		if (PR->iGemInc<0)
		{PR->iGemInc=0;};
		};

//Cайты
//		textprintf(screen,Font1,0,700,COLOR_2,"Сайты          ");

	//Квестовые
		int Possib=100-(iMaxQuest-13)*150/(CurGame.Map.X*CurGame.Map.Y);
		if (Possib>98){Possib=98;};
		if (Possib<50){Possib=50;};
		if (Random(100,CurGame.iSeed,PR->iNumber)>Possib)
		{
			int Count=0;
			ClearRoll();
			for (int i=1; i<=iMaxQuest; i++)
			{
				if (*(QuestList+i)==1)
				{
					RollerN[Count]=i;
					RollerP[Count]=(pFirstQuest+i)->iPossibility;
					Count++;
				};
			};
			if (Count>0)
			{
				int Quest=Roll(false);
//				int Quest=Roll(false,CurGame.iSeed,PR->iNumber,15);
				/*if (Quest>38)
				{
					FileLog("***");
					FileLog((pFirstQuest+Quest)->sName);
					FileLog(PR->sName);
				};*/
				*(QuestList+Quest)=0;
				PR->PlaceSite((pFirstQuest+Quest)->iGiver,Level-2,Quest,0,0);				
			};	
		};

	//Обычные
		int Sites=(pFirstProvinceType+PR->ipType)->iSites+Random(11)-5;
		Sites+=Sites*(mpMap.Site-1)/10;
		if (Sites>GetMin(MAX_PROVINCE_SITE-3,99-PR->iExplore))
		{Sites=GetMin(MAX_PROVINCE_SITE-3,99-PR->iExplore);};
		/*if (Sites>(99-PR->iExplore))
		{
			PR->iExplore=99-Sites;
		};*/
		for (int i=1; i<=iMaxSite; i++)
		{
			for (int j=0; j<MAX_SITE_ABILITY; j++)
			{
				if (((pFirstSite+i)->arAbility[j][0]==5)&&((pFirstSite+i)->arAbility[j][1]==PR->ipType))
				{
					PR->PlaceSite(i,Level-2,0,0,0);
				};
			};
		};
		int Start=0;
		while (PR->arSite[Start][0]>0)
		{Start++;};
		for (int i=0; i<Sites; i++)
		{
			ClearRoll();
			Counter=0;
			for (int j=1; j<=iMaxSite; j++)
			{
				if (((pFirstSite+j)->arTerrain[PR->ipTerrain-1])&&((pFirstSite+j)->iPlace>0))
				{
					RollerN[Counter]=j;
					RollerP[Counter]=(pFirstSite+j)->arTerrain[PR->ipTerrain-1];
					if ((RollerP[Counter]>0)&&(WorldRules.iMode>0))
					{
						for (int k=0; k<MAX_WORLD_SITES; k++)
						{
							if ((pFirstWorldRules+WorldRules.iMode)->arSites[k][0]==j)
							{RollerP[Counter]+=RollerP[Counter]*(pFirstWorldRules+WorldRules.iMode)->arSites[k][1]/100;}
						}
					}
					Counter++;
				};
			};
			PR->PlaceSite(Roll(false),Level-2,0,0,0);
//			PR->PlaceSite(Roll(false,PR->iNumber,i),Level-2,0);
		};
		};
	};

	//Квесты в кампании
	if (CurGame.iGameType==2)
	{
		ClearMoveMap();
		MakeMap(arPlayer[1].ipProvince,0);
		for (int Q=0; Q<MAX_CAMP_QUEST; Q++)
		{
			if ((Campaign.arQuest[Q][0]>0)&&(Campaign.arQuest[Q][1]<2)&&((pFirstQuest+Campaign.arQuest[Q][0])->Place[0]!=0))
			{
				clQuest *pQuest=(pFirstQuest+Campaign.arQuest[Q][0]);
				int Dist=100, Choosen=0;
				for (int Prov=1; Prov<=MapSize; Prov++)
				{
					dcProvince *PR=(pFirstProvince+Prov);
					if ((PR->ipTerrain!=WATER_ID)&&(PR->ipTerrain!=MOUNTAIN_ID)&&(PR->ipType!=7)&&(*(pMoveMap+Prov)>0)&&(PR->arSite[MAX_PROVINCE_SITE-1][0]==0))
					{
						if ((pQuest->Place[0]<0)||(PR->ipTerrain==pQuest->Place[0]))
						{
							if ((abs(*(pMoveMap+Prov)-pQuest->Quantity[0])<Dist)||((abs(*(pMoveMap+Prov)-pQuest->Quantity[0])==Dist)&&(Random(10)<5)))
							{
								Dist=abs(*(pMoveMap+Prov)-pQuest->Quantity[0]);
								Choosen=Prov;
							};
						};
					};
				};
				if (Choosen>0)
				{
					dcProvince *PR=(pFirstProvince+Choosen);
					delete [] PR->sName;
					PR->sName=new char [sizeof pQuest->sName];
					for (int i=0; i<sizeof pQuest->sName; i++)
					{
						*(PR->sName+i)=pQuest->sName[i];
					};
					for (int i=0; i<MAX_PROVINCE_SITE; i++)
					{
						if (PR->arSite[i][0]==0)
						{
							PR->arSite[i][0]=pQuest->Target[0];
							PR->PlaceSiteGuard((pFirstGuardType+pQuest->Index[0])->arGuard[pQuest->iExp],i,0);
							PR->arSite[i][1]=0;
							PR->arSite[i][4]=pQuest->Index[0];
							Campaign.arQuest[Q][3]=i;
							break;
						};
					};
					Campaign.arQuest[Q][2]=Choosen;
				}
				else
				{
					Campaign.arQuest[Q][1]=-1;
				};
			};
		};
	};

	delete [] pNames;
	delete [] QuestList;

//	//Отрисовка ландшафта
//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Отрисовка ландшафта...");
//	Txt++;
	FileLog("Landscape..  ");
	if (mpMap.Y*(Map_h+Map_h/3)/2+Map_h>=750)
	{
		bmpMap=create_bitmap(mpMap.X*Map_w+Map_w*7/6,mpMap.Y*(Map_h+Map_h/3)/2+Map_h);
		rectfill(bmpMap,0,0,mpMap.X*Map_w+Map_w*7/6,mpMap.Y*(Map_h+Map_h/3)/2+Map_h,COLOR_1);
	}
	else
	{
		bmpMap=create_bitmap(mpMap.X*Map_w+Map_w*7/6,750);
		rectfill(bmpMap,0,0,mpMap.X*Map_w+Map_w*7/6,750,COLOR_1);
	};

	pFirstFog=new clFog [MapSize+1];
	for (int i=1; i<=MapSize; i++)
	{
		(pFirstFog+i)->x=mpMap.X*Map_w+Map_w*7/6;
		(pFirstFog+i)->y=mpMap.Y*(Map_h+Map_h/3)/2+Map_h;
		(pFirstFog+i)->x1=-10;
		(pFirstFog+i)->y1=-10;
		(pFirstFog+i)->Num=i;

	};
	
	for (int x=0; x<mpMap.X*Map_w+Map_w*7/6; x+=bmpMapGrass->w)
	{
//		PROCESSBAR(x,35);//grey
		for (int y=0; y<mpMap.Y*(Map_h+Map_h/3)/2+Map_h; y+=bmpMapGrass->h)
		{
			blit(bmpMapGrass,bmpMap,0,0,x,y,bmpMapGrass->w,bmpMapGrass->h);
		};
	};
	NewProgress=2100;
//	set_trans_blender	(158,158,158,158);
	for (int y=0; y<mpMap.Y*(Map_h+Map_h/3)/2+Map_h; y++)
	{
		for (int x=0; x<mpMap.X*Map_w+Map_w*7/6; x++)
		{
//		PROCESSBAR(x,35);//grey
		
			int PR=GetProvinceIndex(x,y);
			if (PR>0)
			{
				if ((pFirstFog+PR)->x>x)
				{(pFirstFog+PR)->x=x;};
				if ((pFirstFog+PR)->y>y)
				{(pFirstFog+PR)->y=y;};
				if ((pFirstFog+PR)->x1<x)
				{(pFirstFog+PR)->x1=x;};
				if ((pFirstFog+PR)->y1<y)
				{(pFirstFog+PR)->y1=y;};		
		//		int Deep, Coord, ClR, ClG, ClB;				
				/*switch ((pFirstProvince+PR)->ipTerrain)
				{
				case WATER_ID:
					if (Random(100)>95)
					{draw_trans_sprite(bmpMap,bmpMapSand,x-15,y-15);};
					break;
				case 4:
					if (Random(1000)>993)
					{draw_trans_sprite(bmpMap,bmpMapSwamp,x-15,y-15);};
					break;	
				};*/
			};
		};
	};
	for (int Layer=0; Layer<MAX_TERR_LAYER; Layer++)
	{
		int PR;
		for (int i=1; i<=iMaxMapObject; i++)
		{			
			if (((pFirstMapObject+i)->iLayer==Layer)&&((pFirstMapObject+i)->iType==-1))
			{
				int MaxSprite=0, Trans;
				Trans=(pFirstMapObject+i)->iSite*255/100;
				set_trans_blender(Trans,Trans,Trans,Trans);
				for (int j=0; j<MAX_MAPOBJ_PIC; j++)
				{
					if ((pFirstMapObject+i)->arPic[j])
					{MaxSprite++;};
				};
				for (y=0; y<mpMap.Y*(Map_h+Map_h/3)/2+Map_h; y++)
				{
					for (int x=0; x<mpMap.X*Map_w+Map_w*7/6; x++)
					{
						PR=GetProvinceIndex(x,y);
						if ((PR>0)&&((pFirstMapObject+i)->arLand[(pFirstProvince+PR)->ipTerrain]>0)&&(Random(20000)<(pFirstMapObject+i)->arLand[(pFirstProvince+PR)->ipTerrain]))
						{
							if ((pFirstProvince+PR)->ipTerrain==0)
							{
								//Светотень на песке
								int PR1=GetProvinceIndex(x+6,y-6);
								if ((PR1>0)&&((pFirstProvince+PR1)->ipTerrain!=0))
								{draw_trans_sprite(bmpMap,(pFirstMapObject+i)->arPic[1],x-(pFirstMapObject+i)->dx/2,y-(pFirstMapObject+i)->dy/2);}
								else
								{
									PR1=GetProvinceIndex(x-6,y+6);
									if ((PR1>0)&&((pFirstProvince+PR1)->ipTerrain!=0))
									{draw_trans_sprite(bmpMap,(pFirstMapObject+i)->arPic[2],x-(pFirstMapObject+i)->dx/2,y-(pFirstMapObject+i)->dy/2);}
									else
									{draw_trans_sprite(bmpMap,(pFirstMapObject+i)->arPic[0],x-(pFirstMapObject+i)->dx/2,y-(pFirstMapObject+i)->dy/2);};
								};
							}
							else
							{draw_trans_sprite(bmpMap,(pFirstMapObject+i)->arPic[Random(MaxSprite)],x-(pFirstMapObject+i)->dx/2,y-(pFirstMapObject+i)->dy/2);};
						};
					};
				};
				if (NewProgress<3000)
				{NewProgress+=100;};
			};
		};
	};
	//Создание карты глубин
	/*textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Создание карты глубин...");
	Txt++;
	clDeep *pDeep=NULL;
	int iDeep=0;
	bool bNeedWater=true;
	for (int y=0; y<mpMap.Y*(Map_h+Map_h/3)/2+Map_h; y++)
	{
		Map_Border *MP=Map[y];
		while (MP)
		{
			if ((MP->Prov[0]>0)&&(MP->Prov[1]>0))
			{
				if ((((pFirstProvince+MP->Prov[0])->ipTerrain==WATER_ID)&&((pFirstProvince+MP->Prov[1])->ipTerrain!=WATER_ID))||
					(((pFirstProvince+MP->Prov[0])->ipTerrain!=WATER_ID)&&((pFirstProvince+MP->Prov[1])->ipTerrain==WATER_ID)))
				{
					pDeep=new clDeep(MP->x,y,iDeep,pDeep);
				};
			};
			MP=MP->Next;
		};
	};
	if (pDeep)
	{
		clDeep *DP=pDeep, *NewDeep;
		while (bNeedWater)
		{
			bNeedWater=false;
			DP=pDeep;
			while (DP)
			{
				if (DP->Deep==iDeep)
				{
					NewDeep=AddDeep(DP->x-1,DP->y-1,iDeep+1,pDeep);
					if (NewDeep){pDeep=NewDeep;bNeedWater=true;};
					NewDeep=AddDeep(DP->x-1,DP->y,iDeep+1,pDeep);
					if (NewDeep){pDeep=NewDeep;bNeedWater=true;};
					NewDeep=AddDeep(DP->x-1,DP->y+1,iDeep+1,pDeep);
					if (NewDeep){pDeep=NewDeep;bNeedWater=true;};
					NewDeep=AddDeep(DP->x,DP->y-1,iDeep+1,pDeep);
					if (NewDeep){pDeep=NewDeep;bNeedWater=true;};
					NewDeep=AddDeep(DP->x,DP->y,iDeep+1,pDeep);
					if (NewDeep){pDeep=NewDeep;bNeedWater=true;};
					NewDeep=AddDeep(DP->x,DP->y+1,iDeep+1,pDeep);
					if (NewDeep){pDeep=NewDeep;bNeedWater=true;};
					NewDeep=AddDeep(DP->x+1,DP->y-1,iDeep+1,pDeep);
					if (NewDeep){pDeep=NewDeep;bNeedWater=true;};
					NewDeep=AddDeep(DP->x+1,DP->y,iDeep+1,pDeep);
					if (NewDeep){pDeep=NewDeep;bNeedWater=true;};
					NewDeep=AddDeep(DP->x+1,DP->y+1,iDeep+1,pDeep);
					if (NewDeep){pDeep=NewDeep;bNeedWater=true;};
				};
				DP=DP->pNext;			
			};
			iDeep++;
		};*/
		//Отрисовка воды

//		textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Отрисовка воды...");
//		Txt++;
		FileLog("Water..  ");
		int Deep, Coord;
		set_trans_blender(40,40,40,40);
		for (int x=0; x<mpMap.X*Map_w+Map_w*7/6; x++)
		{
			NewProgress=3000+x*(7000/(mpMap.X*Map_w+Map_w*7/6));
//			PROCESSBAR(x,35);//grey			
			for (int y=0; y<mpMap.Y*(Map_h+Map_h/3)/2+Map_h; y++)
			{
				int PR=GetProvinceIndex(x,y);
				if ((PR>0)&&((pFirstProvince+PR)->ipTerrain==0))
				{
					Deep=100;
					Coord=1;					
					while ((Deep==100)&&(Coord<100))
					{
						if ((GetDeep(x+Coord, y))||(GetDeep(x-Coord, y))||(GetDeep(x, y+Coord))||(GetDeep(x, y-Coord))
							||(GetDeep(x-Coord, y+Coord))||(GetDeep(x-Coord, y-Coord))||(GetDeep(x+Coord, y-Coord))||(GetDeep(x+Coord, y+Coord)))
						{
							Deep=Coord;
						};
						Coord+=5;
					};
					if (Deep>Random(1000))
					{
						draw_trans_sprite(bmpMap,bmpMapWater[Random(5)],x-15, y-15);
					};
				};
			};
		};

		
		/*for (int i=iDeep; i>5; i--)
		{
			DP=pDeep;
			while (DP)
			{
				if ((DP->Deep==i)&&(Random(100)<i))
				{
					draw_trans_sprite(bmpMap,bmpMapWater,DP->x-15, DP->y-15);
				};
				DP=DP->pNext;
			};
		};
		set_trans_blender(128,128,128,128);*/
	/*	DP=pDeep;
		while (DP)
		{
			pDeep=pDeep->pNext;
			delete DP;
			DP=pDeep;
		};
	};*/


	//Отрисовка тумана войны
//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Отрисовка тумана войны...");
//	Txt++;
	FileLog("Fog..  ");
	for (int i=1; i<=MapSize; i++)
	{
		NewProgress=10000+5000*i/MapSize;
	//	PROCESSBAR(i,3);
		(pFirstFog+i)->Pic=create_bitmap(3+(pFirstFog+i)->x1-(pFirstFog+i)->x,3+(pFirstFog+i)->y1-(pFirstFog+i)->y);
		rectfill((pFirstFog+i)->Pic,0,0,(pFirstFog+i)->Pic->w,(pFirstFog+i)->Pic->h,COLOR_0);
		for (int x=(pFirstFog+i)->x-2; x<(pFirstFog+i)->x1+1; x++)
		{
			for (int y=(pFirstFog+i)->y-2; y<(pFirstFog+i)->y1+1; y++)
			{
				if ((x>=0)&&(y>=0)&&(x<bmpMap->w)&&(y<bmpMap->h)&&(
					(GetProvinceIndex(x,y)==i)||(GetProvinceIndex(x-1,y)==i)||(GetProvinceIndex(x,y-1)==i)||
					(GetProvinceIndex(x+1,y)==i)||(GetProvinceIndex(x,y+1)==i)||(GetProvinceIndex(x+1,y+1)==i)||
					(GetProvinceIndex(x+1,y+2)==i)||(GetProvinceIndex(x+2,y+1)==i)||(GetProvinceIndex(x+2,y+2)==i)||
					(GetProvinceIndex(x,y+2)==i)||(GetProvinceIndex(x+2,y)==i)||(getpixel(bmpMap,x,y)==COLOR_1)))
				{
					putpixel((pFirstFog+i)->Pic,1+x-(pFirstFog+i)->x,1+y-(pFirstFog+i)->y,COLOR_1);
				};
			};
		};
	};
	
//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Отрисовка спрайтов...");
//	Txt++;

	FileLog("Sprites..  ");

	bool bPick; int iPoss, Count;
	for (int LN=1; LN<=iMaxLandmark; LN++)
	{
		for (int Terr=0; Terr<MAX_TERRAIN+2; Terr++)
		{
			Count=0;
			ClearRoll();
			for (int i=1; i<=MapSize; i++)
			{
				if ((((pFirstProvince+i)->ipTerrain==(pFirstLandmark+LN)->arLand[Terr])||((Terr==0)&&((pFirstLandmark+LN)->arLand[Terr]<0)))
					&&((pFirstProvince+i)->arBuilding[MAX_PROVINCE_BLD]==0))
				{
					bPick=false;
					if ((pFirstLandmark+LN)->arType[0]==0)
					{bPick=true;}
					else
					{
						for (int j=0; j<MAX_MAPOBJ_TYPE; j++)
						{
							if ((pFirstLandmark+LN)->arType[j]==(pFirstProvince+i)->ipType)
							{bPick=true;};
						};
					};
					if (bPick)
					{
						if ((pFirstLandmark+LN)->iSite==0)
						{iPoss=10;}
						else
						{
							iPoss=0;
							for (int j=0; j<MAX_PROVINCE_SITE; j++)
							{
								if ((pFirstProvince+i)->arSite[j][0]==(pFirstLandmark+LN)->iSite)
								{iPoss++;};
							};
						};
						if (iPoss>0)
						{
							RollerN[Count]=i;
							RollerP[Count]=iPoss;
							Count++;
						};
						};
				};
			};
			if (Count>0)
			{
				int PR=Roll(false);
				(pFirstProvince+PR)->arBuilding[MAX_PROVINCE_BLD]=LN;
				if ((pFirstProvince+PR)->ipTerrain==0)
				{
					(pFirstProvince+PR)->arBldPlace[MAX_PROVINCE_BLD][0]=(pFirstProvince+PR)->x;
					(pFirstProvince+PR)->arBldPlace[MAX_PROVINCE_BLD][1]=(pFirstProvince+PR)->y;
				};
			};			
		};
	};
	NewProgress=15100;
	int PR1; 
	bool bDraw;
	set_trans_blender(128,128,128,128);	
	for (int Layer=0; Layer<MAX_TERR_LAYER; Layer++)
	{
	for (int y=0; y<mpMap.Y*(Map_h+Map_h/3)/2+Map_h; y++)	
	{
//		textprintf (screen,Font1,700,0,COLOR_2,"Layer: %d  Y: %d    ",Layer,y);
		NewProgress=15100+Layer*2900/MAX_TERR_LAYER+y*(2900/MAX_TERR_LAYER)/(mpMap.Y*(Map_h+Map_h/3)/2+Map_h);
		for (int x=0; x<mpMap.X*Map_w+Map_w*7/6; x++)		
		{	
			bool NeedRoll=true;
			int PR=GetProvinceIndex(x,y);
			if (((pFirstProvince+PR)->arBuilding[MAX_PROVINCE_BLD]>0)&&((pFirstProvince+PR)->arBldPlace[MAX_PROVINCE_BLD][0]==x)&&((pFirstProvince+PR)->arBldPlace[MAX_PROVINCE_BLD][1]==y))
			{
				int LN=(pFirstProvince+PR)->arBuilding[MAX_PROVINCE_BLD];
				if ((pFirstLandmark+LN)->iLayer==Layer)
				{
					NeedRoll=false;
					if ((pFirstLandmark+LN)->Shadow)
					{
						draw_trans_sprite(bmpMap,(pFirstLandmark+LN)->Shadow,x-(pFirstLandmark+LN)->dx,y-(pFirstLandmark+LN)->dy);
					};
					draw_sprite(bmpMap,(pFirstLandmark+LN)->Pic,x-(pFirstLandmark+LN)->dx,y-(pFirstLandmark+LN)->dy);
				};
			};
			if (NeedRoll)
			{			
			int Poss=0;
			if ((PR>0)&&(arTerrLayer[(pFirstProvince+PR)->ipTerrain][Layer]>0)&&((Random(20000,x,y,CurGame.iSeed,Layer)<arTerrLayer[(pFirstProvince+PR)->ipTerrain][Layer])||((x==(pFirstProvince+PR)->x)&&(y==(pFirstProvince+PR)->y)&&(((pFirstProvince+PR)->ipTerrain==HILL_ID)||((pFirstProvince+PR)->ipTerrain==MOUNTAIN_ID)))
				||(((pFirstProvince+PR)->ipTerrain==MOUNTAIN_ID)&&(y>(pFirstProvince+PR)->y)&&(x==(pFirstProvince+PR)->x)&&(y==(pFirstProvince+PR)->arBldPlace[0][1]))))
			{				
				ClearRoll();
				int Counter=0;
				for (int i=1; i<=iMaxMapObject; i++)
				{
					if (((pFirstMapObject+i)->iType>=0)&&((pFirstMapObject+i)->iLayer==Layer)&&(!(pFirstMapObject+i)->bLandmark)&&((pFirstMapObject+i)->arLand[(pFirstProvince+PR)->ipTerrain]>0))
					{
						bDraw=true;
						if (((pFirstProvince+PR)->ipTerrain==2)&&((pFirstMapObject+i)->iType>0)&&((CurGame.iSeed+PR)%4==0))
						{
							if ((CurGame.iSeed/7+PR)%2+1!=(pFirstMapObject+i)->iType)
							{bDraw=false;};
						};
						PR1=GetProvinceIndex(x-(pFirstMapObject+i)->dx/2,y-(pFirstMapObject+i)->dy/2);
						if ((PR1==0)||((pFirstMapObject+i)->arLand[(pFirstProvince+PR1)->ipTerrain]==0))
						{bDraw=false;};
						PR1=GetProvinceIndex(x+(pFirstMapObject+i)->dx/2,y-(pFirstMapObject+i)->dy/2);
						if ((PR1==0)||((pFirstMapObject+i)->arLand[(pFirstProvince+PR1)->ipTerrain]==0))
						{bDraw=false;};
						PR1=GetProvinceIndex(x,y-(pFirstMapObject+i)->dy/2);
						if ((PR1==0)||((pFirstMapObject+i)->arLand[(pFirstProvince+PR1)->ipTerrain]==0))
						{bDraw=false;};
						if ((x==(pFirstProvince+PR)->x)&&(y==(pFirstProvince+PR)->y)&&((pFirstProvince+PR)->ipTerrain==MOUNTAIN_ID))
						{
							for (int dy=40; dy>=0; dy-=5)
							{
								if (PR==GetProvinceIndex(x,y+dy))
								{
									(pFirstProvince+PR)->arBldPlace[0][1]=y+dy;
									break;
								};
							};
						};
						if ((bDraw)||((x==(pFirstProvince+PR)->x)&&(y==(pFirstProvince+PR)->y)&&((pFirstProvince+PR)->ipTerrain==HILL_ID))||
							((x==(pFirstProvince+PR)->x)&&(y==(pFirstProvince+PR)->arBldPlace[0][1])&&((pFirstProvince+PR)->ipTerrain==MOUNTAIN_ID)))
						{
							RollerN[Counter]=i;
							RollerP[Counter]=(pFirstMapObject+i)->arLand[(pFirstProvince+PR)->ipTerrain];
							Counter++;
						};
					};					
				};
	//			textprintf (screen,Font1,700,20,COLOR_2,"Спрайт: %d, земля: %d",Counter,(pFirstProvince+PR)->ipTerrain);
				if (Counter>0)
				{
					int iObj=Roll(false,CurGame.iSeed,y,x,Layer);
					int iPic=0;
					while ((iPic<MAX_MAPOBJ_PIC)&&((pFirstMapObject+iObj)->arPic[iPic])){iPic++;};
					iPic=Random(iPic,y,iSeed,Layer,x);
					for (int i=1; i<=iMaxMapObject; i++)
					{
						if ((pFirstMapObject+i)->iType-1000==iObj)
						{
							if ((pFirstMapObject+i)->iLayer==0)
							{
								draw_sprite(bmpMap,(pFirstMapObject+i)->arPic[0],x-(pFirstMapObject+iObj)->arPic[iPic]->w/2,y-(pFirstMapObject+iObj)->arPic[iPic]->h);
							}
							else
							{
								draw_trans_sprite(bmpMap,(pFirstMapObject+i)->arPic[0],x-(pFirstMapObject+iObj)->arPic[iPic]->w/2,y-(pFirstMapObject+iObj)->arPic[iPic]->h);
							};
						};
					};
				//	dcProvince *PPR=pFirstProvince;
					int XP=x-(pFirstMapObject+iObj)->arPic[iPic]->w/2, YP=y-(pFirstMapObject+iObj)->arPic[iPic]->h;
					/*if ((Layer==2)&&((pFirstProvince+PR1)->ipTerrain!=3)&&((pFirstProvince+PR1)->ipTerrain!=8))
					{
						for (int Prr=1; Prr<=MapSize; Prr++)
						{
							PPR++;
							if ((PPR->y-20<YP)&&(PPR->y+26>YP)&&(PPR->x-30-(pFirstMapObject+iObj)->arPic[iPic]->w<XP)&&(PPR->x+30>XP))
							{
								draw_sprite(PPR->TownPic1,(pFirstMapObject+iObj)->arPic[iPic],XP-PPR->x+30,YP-PPR->y+20);
							};
							if ((PPR->y-3<YP)&&(PPR->y+45>YP)&&(PPR->x-50-(pFirstMapObject+iObj)->arPic[iPic]->w<XP)&&(PPR->x+50>XP))
							{
								draw_sprite(PPR->TownPic2,(pFirstMapObject+iObj)->arPic[iPic],XP-PPR->x+50,YP-PPR->y+3);
							};
						};
					};*/
					draw_sprite(bmpMap,(pFirstMapObject+iObj)->arPic[iPic],XP,YP);
				};
			};
		};
		};
	};
	};
	/*while (pLandObj)
	{
		clLandscape *Temp=pLandObj;
		pLandObj=pLandObj->pNext;
		delete Temp;
	};*/

//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Отрисовка границ...");
//	Txt++;
//	objMain=new G_Object(objMain,0,0,0,bmpGUI_Dead);
	FileLog("Borders..  ");
	Prov=pFirstProvince;
	BITMAP *RedDot=create_bitmap(1,1);
	putpixel(RedDot,0,0,makecol(200,50,50));
	BITMAP *BlackDot=create_bitmap(1,1);
	putpixel(BlackDot,0,0,makecol(0,0,0));
	for (int i=0; i<bmpMap->h; i++)
	{
		Map_Border *Temp=Map[i];
		while (Temp)
		{
			if ((Temp->Prov[0]!=0)&&(Temp->Prov[1]!=0))
			{
				switch (arHumans[0].iMapBorders)
				{
				case 1:
					putpixel(bmpMap,Temp->x,i,makecol(200,50,50));
					break;
				case 2:
					if (((Temp->x%3==0)||(i%3==0))&&((Temp->x+y)%4==0))
					{draw_sprite(bmpMap,bmpBorder,Temp->x-3,i-2);};
					break;
				case 3:
					break;
				default:
					draw_trans_sprite(bmpMap,RedDot,Temp->x,i);
					draw_trans_sprite(bmpMap,BlackDot,Temp->x+1,i-1);
					break;
				};

				/*if (((Temp->x%3==0)||(i%3==0))&&((Temp->x+y)%4==0))
				{draw_sprite(bmpMap,bmpBorder,Temp->x-3,i-2);};*/
			};
			Temp=Temp->Next;
		};
	};
	//for (int i=0; i<bmpMap->h; i++)
 //   {
 //        int col= makecol(200,50,50); //grey
 //        Map_Border *Temp=Map[i];
 //        while (Temp)
 //        {
 //             if ((Temp->Prov[0]!=0)&&(Temp->Prov[1]!=0))
 //             {
	//			   if( ((Temp->x+i)/3)% 8 < 5 ){ rectfill(bmpMap, Temp->x, i, Temp->x+1, i+1, col ); };
 //                  //if( i%8 < 5 ){ rectfill(bmpMap, Temp->x, i, Temp->x+1, i+1, col ); } // grey - ВАНИАНТ 1 - тот что прислал
 //                  //if( ((Temp->x+i)/3)%7 < 4 ){ rectfill(bmpMap, Temp->x, i, Temp->x+1, i+1, col ); } //grey- ВАНИАНТ 2 - меньший шаг на 1рх
 //             };
 //             Temp=Temp->Next;
 //        };
 //   };

//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Отрисовка НИЧТО...");
//	Txt++;
	FileLog("Nothing..  ");
	set_trans_blender(40,40,40,40);

	for (int x=0; x<mpMap.X*Map_w+Map_w*7/6; x++)
	{
		NewProgress=18000+x*2000/(mpMap.X*Map_w+Map_w*7/6);
		int Deep, Coord;
		for (y=0; y<mpMap.Y*(Map_h+Map_h/3)/2+Map_h; y++)
		{
			int PR=GetProvinceIndex(x,y);
			if (PR==0)
			{
				if (Random(100)>90)
				{
					draw_trans_sprite(bmpMap,bmpMapDark,x-15, y-15);				
				};
			};
		};
	};
	set_trans_blender(128,128,128,128);
	


	
	

	//////////////////////////////////////////////////////////////////////////////////////
	//BLOCK#01
	//grey WORK
	//const int  MAP_BORDER_WIDTH = 20;	//max удаление от реальной границы при апроксимации

	// сглаживание границ ландшафта
/*	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Сглаживание границ ландшафта...");
	Txt++;
	objMain=new G_Object(objMain,0,0,0,bmpGUI_Dead);

	const int  MAP_MIXED_DEPTH	= 9;//20;//10; //7;// max глубина смешивания текстур между провинциями с разн.типом ландшафта
if(MAP_MIXED_DEPTH > 0){
	//const int  MAP_COAST_RANGE	=  5;		// отступ береговой линии от границы текстур (наружу; -вглубь воды)
	const int  MAP_COAST_WIDTH	=  5;//9;//11;//		// ширина береговой линии между водой и землей (для песка)

	const int  MAP_MIXED_STEP	=  1;//1; // шаг при смешивании текстур
	const int  MAP_MIXED_LENGTH =  1;//2; // длина участка при смешивании текстур

	const int  MAP_MIX_MTR_LINES   =  5;	// кол-во кривых смешивания
	const int  MAP_MIX_MTR_PIXELS  = 10;	// кол-во точек в каждой кривой смешивания
	const int  MAP_LN = MAP_MIX_MTR_LINES;
	const int  MAP_PN = MAP_MIX_MTR_PIXELS;
	int MAP_MIX_MATRIX[MAP_LN][MAP_PN] = {	1, 1, 2, 2, 5, 3, 3, 4, 4, 7,
											5, 5, 6, 6, 9, 9, 7, 7, 8, 8,

											1, 2, 3, 6, 9, 8, 4, 2 ,3, 1,

											3, 2, 5, 4, 6, 8, 9, 8, 6, 2,
											1, 3, 5, 5, 4, 2, 4, 5, 4, 2	}; // y= [(PR1+PR2)%LN][f(x)] 


	int mtrMax = -1;
	for(int i=0; i < MAP_MIX_MTR_LINES; i++) { //find maximum value
		for(int j=0; j < MAP_MIX_MTR_PIXELS; j++) {
			if(mtrMax < MAP_MIX_MATRIX[i][j]) mtrMax=MAP_MIX_MATRIX[i][j];
		}
	}
	float mtrDiv = float(MAP_MIXED_DEPTH)/float(mtrMax!=0?mtrMax:1);	if(mtrDiv<0) mtrDiv=-mtrDiv;
	//float mtrDiv = float(mtrMax)/float(MAP_MIXED_DEPTH);	if(mtrDiv<0) mtrDiv=-mtrDiv;
	char a_strTemp[256]; sprintf(a_strTemp,"mtrMax= %d  _DEPTH= %d  Div= %d  =%d.%d",mtrMax,MAP_MIXED_DEPTH, int(mtrDiv*1000),int(mtrDiv),int(mtrDiv*1000)%1000 ); textprintf_ex(screen,Font1,30,(Txt++)*20,COLOR_2,-1, a_strTemp);
	for(int i=0; i < MAP_MIX_MTR_LINES &&mtrDiv > 0; i++){ //растянуть/сжать линии MIX_MATRIX- f(MAP_MIXED_DEPTH)
		for(int j=0; j < MAP_MIX_MTR_PIXELS; j++){
			//{char a_strTemp[256]; 			int go1= 10*(MAP_MIX_MATRIX[i][j]/mtrDiv);			sprintf(a_strTemp,"X1= %d.%d", go1/10, go1%10 );									textprintf_ex(screen,Font1,30,(Txt++)*20,COLOR_2,-1, a_strTemp);			sprintf(a_strTemp,"      X2= %d  ", ROUND( float(MAP_MIX_MATRIX[i][j])/mtrDiv ) );	textprintf_ex(screen,Font1,30,(Txt++)*20,COLOR_2,-1, a_strTemp);}
			//MAP_MIX_MATRIX[i][j] = ROUND( float(MAP_MIX_MATRIX[i][j])/mtrDiv ); 
			MAP_MIX_MATRIX[i][j] = ROUND( float(MAP_MIX_MATRIX[i][j]) * mtrDiv ); 
		}
	}

	int preTER = -1, prePR = -1;	int tnn = 301, stn = 12;
	for(int x=0; x < mpMap.X*Map_w+Map_w*7/6; x++) { //проход по карте по Х 
		PROCESSBAR(x,50);//if(x%100 == 99) {textprintf_centre_ex(screen,Font1,tnn+=stn,180,COLOR_2,-1,".");}
		for(int y=0; y < mpMap.Y*(Map_h+Map_h/3)/2+Map_h; y++ ) //y+=2 ) //y++) // y+=3
		{
			int PR=GetProvinceIndex(x,y);
			if(PR > 0) 
			{
				if(preTER < 0 || y == 0) { 
					preTER = (pFirstProvince+PR)->ipTerrain;
					prePR = PR;
				} 
				else {
					if(preTER != (pFirstProvince+PR)->ipTerrain) {  
						#define MAX_MAP_MIXED_ZONE	1000
						int line_mix[MAX_MAP_MIXED_ZONE],lmi= 0, m3i= (x +PR+prePR)%(MAP_LN*MAP_PN);  //,m3ii= m3i/10, m3ij= m3i%10;
						int mix_wid= MAP_MIX_MATRIX[m3i/10][m3i%10];//= MAP_MIX_MATRIX[(preTER+(pFirstProvince+PR)->ipTerrain)%MAP_LN][x%MAP_PN];//	//int mix_wid = MAP_MIXED_DEPTH; // = MAP_MIX_MATRIX[(PR+prePR)%MAP_LN][x%MAP_PN]; //= *(MAP_MIX_MATRIX+x%(MAP_LN*MAP_PN))

						//захват линии смешивания
						for(int yy=y-mix_wid; yy < y+mix_wid+1 ; yy++){
							if(yy>0 && GetProvinceIndex(x,yy)>0)
								line_mix[lmi++]= getpixel(bmpMap,x,yy);
						}
						lmi--;

						int board_w=0;
						//смешивание текстур
						if(preTER==WATER_ID || (pFirstProvince+PR)->ipTerrain==WATER_ID) //берег/вода
						{
							int beach_wid= 2*mix_wid+MAP_COAST_WIDTH; //NEW!!!
							for(int i=mix_wid; i < mix_wid+beach_wid && i+mix_wid+beach_wid<MAX_MAP_MIXED_ZONE; i++)
							{
								line_mix[i+mix_wid+beach_wid] = line_mix[i];
								line_mix[i] = GetCoastPixel();
							}
							lmi+= beach_wid;
							board_w = beach_wid/2;
							if(lmi>=MAX_MAP_MIXED_ZONE) lmi= MAX_MAP_MIXED_ZONE; 
							
							//смешивание текстур
							Mix_Array(line_mix, 0,		  lmi/3,	MAP_MIXED_LENGTH,MAP_MIXED_STEP); //water-sand
							//Mix_Array(line_mix, lmi/3,  2*lmi/3,	MAP_MIXED_LENGTH,MAP_MIXED_STEP);
							Mix_Array(line_mix, 2*lmi/3,    lmi,	MAP_MIXED_LENGTH,MAP_MIXED_STEP); //sand-earth
						} else {
							Mix_Array(line_mix,0,lmi,MAP_MIXED_LENGTH,MAP_MIXED_STEP);//смешивание текстур
							//int Mix_Array(int *arr /line/, int nn /lmi/, int pos1 /i/, int len /LEN/, int step=1 /STEP/, int rnd=0 /random(rnd)/)
							
						}
						
						mix_wid+= board_w;
						for(int yy=y+mix_wid+1; yy > y-mix_wid &&lmi>=0; yy--) {//отрисовка смешаной линии
							if(line_mix[lmi]>=0 && yy>0 && GetProvinceIndex(x,yy)>0)
								putpixel(bmpMap,x,yy,line_mix[lmi--]);
						}
						preTER = (pFirstProvince+PR)->ipTerrain; 
						prePR = PR;

					}
				}
			} else { 
				preTER = -1; }
		}
	}

	// FOR y(x) ---------------------------------------------------------------------
	preTER = -1; prePR = -1;
	for(int y=0; y < mpMap.Y*(Map_h+Map_h/3)/2+Map_h ; y++ ) { //проход по карте по Y 
		PROCESSBAR( y+1,50);//if(y%100 == 99) {textprintf_centre_ex(screen,Font1,tnn+=stn,180,COLOR_2,-1,".");}
		for(int x=0; x < mpMap.X*Map_w+Map_w*7/6; x++)
		{
			int PR=GetProvinceIndex(x,y);
			if(PR > 0) {
				if(preTER < 0 || x == 0) { 
					preTER = (pFirstProvince+PR)->ipTerrain;
					prePR = PR;
				} else {
					if(preTER != (pFirstProvince+PR)->ipTerrain) {
						int line_mix[MAX_MAP_MIXED_ZONE],lmi= 0, m3i= (y +PR+prePR)%(MAP_LN*MAP_PN);
						int mix_wid= MAP_MIX_MATRIX[m3i/10][m3i%10];
						
						for(int xx=x-mix_wid; xx < x+mix_wid+1 ; xx++){//захват линии смешивания
							if(xx>0 && GetProvinceIndex(xx,y)>0)
								line_mix[lmi++]= getpixel(bmpMap,xx,y);
						}
						lmi--;
						int board_w=0;
						if(preTER==WATER_ID || (pFirstProvince+PR)->ipTerrain==WATER_ID) { //берег/вода
							int beach_wid= 2*mix_wid+MAP_COAST_WIDTH;
							for(int i=mix_wid; i < mix_wid+beach_wid && i+mix_wid+beach_wid<MAX_MAP_MIXED_ZONE; i++){
								line_mix[i+mix_wid+beach_wid] = line_mix[i];
								line_mix[i] = GetCoastPixel();
							}
							board_w = beach_wid/2;
							lmi+= beach_wid;
							if(lmi>=MAX_MAP_MIXED_ZONE) lmi= MAX_MAP_MIXED_ZONE;							
							//смешивание текстур
							Mix_Array(line_mix, 0,		  lmi/3,	MAP_MIXED_LENGTH,MAP_MIXED_STEP); // water-sand
							Mix_Array(line_mix, 2*lmi/3,    lmi,	MAP_MIXED_LENGTH,MAP_MIXED_STEP); // sand-earth
						} else {
							Mix_Array(line_mix,0,lmi,MAP_MIXED_LENGTH,MAP_MIXED_STEP);//смешивание текстур
						}
						
						mix_wid+= board_w;
						for(int xx=x+mix_wid+1; xx > x-mix_wid &&lmi>=0; xx--) {//отрисовка смешаной линии
							if(line_mix[lmi]>=0 && xx>0 && GetProvinceIndex(xx,y)>0)
								putpixel(bmpMap,xx,y,line_mix[lmi--]);
						}
						preTER = (pFirstProvince+PR)->ipTerrain; 
						prePR = PR;
					}
				}
			} else { 
				preTER = -1; }
		}
	}

*/

//WAIT1();

//	// FOR y(x)	//BLOCK-#02
//	for(int y=0; y < mpMap.Y*(Map_h+Map_h/3)/2+Map_h &&false; y++ ) { //проход по карте по Y 
//		PROCESSBAR( y+1,20);//if(y%100 == 99) {textprintf_centre_ex(screen,Font1,tnn+=stn,180,COLOR_2,-1,".");}
//		for(int x=0; x < mpMap.X*Map_w+Map_w*7/6; x++)
//		{
//			int PR=GetProvinceIndex(x,y);
//			if(PR > 0) {
//				if(preTER < 0 || x == 0) { 
//					preTER = (pFirstProvince+PR)->ipTerrain;
//					prePR = PR;
//				} else {
//					if(preTER != (pFirstProvince+PR)->ipTerrain) {  
//						int line_mix[100], lmi= 0, m3i= (y +PR+prePR)%(MAP_LN*MAP_PN), m3ii = m3i/10, m3ij = m3i%10;
//						int mix_wid = MAP_MIX_MATRIX[m3i/10][m3i%10];
//						//захват линии смешивания
//						for(int xx=x-mix_wid; xx < x+mix_wid+1; xx++) { 
//							if(xx>0 && GetProvinceIndex(xx,y)>0)
//								line_mix[lmi++]= getpixel(bmpMap,xx,y);
//						}
//						lmi--;
//						//смешивание текстур
//						for(int xx=x-mix_wid; xx < x+mix_wid+1; xx+=(MAP_MIXED_LENGTH+MAP_MIXED_STEP)) {
//							for(int lmj=0; lmj < MAP_MIXED_LENGTH ; lmj++) {
//								if(lmi >= 0 && line_mix[lmi]>=0 && xx+lmj>0 && GetProvinceIndex(xx+lmj,y)>0) {
//									putpixel(bmpMap,xx+lmj,y,line_mix[lmi]);
//								}
//								lmi--;
//							}
//							lmi-= MAP_MIXED_STEP;
//						}
//						preTER = (pFirstProvince+PR)->ipTerrain; 
//						prePR = PR;
//					}
//				}
//			} else { 
//				preTER = -1; }
//		}
//	}
//
//WAIT1();
//} //if(MAP_MIXED_DEPTH > 0){

	//grey WORK
	//ENDOFBLOCK#01
	//////////////////////////////////////////////////////////////////////////////////////


	FileLog("FillObstr..  ");
	FillObstracleMap(2000);
	
	FileLog("Prf..  ");
	arHumans[0].Save();
	
	
//	char sFileName[20];
//	if (CurGame.iGameType==1)
//	{sprintf(sFileName,"map%d.map",arHumans[0].iFileIndex);}
//	else
//	{sprintf(sFileName,"map%d.cmp",arHumans[0].iFileIndex);};
//	sprintf(sFileName,"Map%d.map",CurGame.iSeed);

//	textprintf_ex(screen,Font1,30,Txt*20,COLOR_2,-1,"Сохранение карты в %s ...",sFileName);
//	Txt++;
	FileLog("Save Map...  ");
	RunHourGlass(149, 0);
	SaveMap();
	FileLog("GenerationFinished.  ");	
};



void SaveMap()
{
	PACKFILE *WorkFile;
	PALETTE Palet;
	get_palette(Palet);

//	sprintf(sMapFile.str,sFileName);
//	char sFile [50];
//	sprintf(sFile,"save/%s",sFileName);
	char sFileName[100];
	switch (CurGame.iGameType)
	{
	case 1:
		sprintf(sFileName,"save/map%d.map",arHumans[0].iFileIndex);
		break;
	case 2:	
		sprintf(sFileName,"save/map%d.cmp",arHumans[0].iFileIndex);
		break;
	case 3:
		sprintf(sFileName,"save/map%d.hot",arHumans[0].iFileIndex);
		break;
	case 4:
		sprintf(sFileName,"save/map%d.lan",arHumans[0].iFileIndex);
		break;
	case 5:
		sprintf(sFileName,"save/map%d.net",arHumans[0].iFileIndex);
		break;
	};
	WorkFile=pack_fopen(sFileName,"wp");
	save_pcx_pf(WorkFile,bmpMap,Palet);
	pack_iputw(0,WorkFile);
	pack_iputw(MapSize,WorkFile);
	for (int i=1; i<=MapSize; i++)
	{
		pack_fwrite((pFirstFog+i),sizeof *(pFirstFog+i),WorkFile);
		save_pcx_pf(WorkFile,(pFirstFog+i)->Pic,Palet);
		dcProvince *PR=(pFirstProvince+i);
		pack_fwrite(PR,sizeof *PR,WorkFile);
		char *CH=PR->sName;
		while (*CH!='\0')
		{			
			pack_putc(*CH,WorkFile);
			CH++;
		};
		pack_putc('\0',WorkFile);

		/*if (PR->pGuard)
		{
			pack_putc(1,WorkFile);
			pack_fwrite(PR->pGuard,sizeof *PR->pGuard,WorkFile);
			for (int j=0; j<MAX_SQUAD; j++)
			{
				if (PR->pGuard->pSquad[j])
				{
					pack_putc(1,WorkFile);
					pack_fwrite(PR->pGuard->pSquad[j],sizeof *PR->pGuard->pSquad[j],WorkFile);

				}
				else
				{
					pack_putc(0,WorkFile);
				};
			};
		}
		else
		{
			pack_putc(0,WorkFile);
		};

		for (int k=0; k<MAX_PROVINCE_SITE; k++)
		{
			if (PR->arSiteGuard[k])
			{
				pack_putc(1,WorkFile);
				pack_fwrite(PR->arSiteGuard[k],sizeof *PR->arSiteGuard[k],WorkFile);
				for (int j=0; j<MAX_SQUAD; j++)
				{
					if (PR->arSiteGuard[k]->pSquad[j])
					{
						pack_putc(1,WorkFile);
						pack_fwrite(PR->arSiteGuard[k]->pSquad[j],sizeof *PR->arSiteGuard[k]->pSquad[j],WorkFile);
					}
					else
					{
						pack_putc(0,WorkFile);
					};
				};
			}
			else
			{
				pack_putc(0,WorkFile);
			};
		};*/
	};
	for (int i=0; i<MAP_SIZE; i++)
	{
//		rectfill(screen,i%1000,(i/1000)*5,i%1000+1,(i/1000)*5+3,COLOR_2);
		Map_Border *BR=Map[i];
		while (BR)
		{
			pack_iputw(i,WorkFile);
			pack_fwrite(BR,sizeof *BR,WorkFile);
			BR=BR->Next;
		};
	};
	pack_iputw(-1,WorkFile);

	pack_fclose(WorkFile);
};

void LoadMap()
{
	PACKFILE *WorkFile;
	PALETTE Palet;		
//	char sFile [50];
//	sprintf(sMapFile.str,sFileName);
//	sprintf(sFile,"maps/%s",sFileName);
	char sFileName[100];
	switch (CurGame.iGameType)
	{
	case 1:
		sprintf(sFileName,"save/map%d.map",arHumans[0].iFileIndex);
		break;
	case 2:	
		sprintf(sFileName,"save/map%d.cmp",arHumans[0].iFileIndex);
		break;
	case 3:
		sprintf(sFileName,"save/map%d.hot",arHumans[0].iFileIndex);
		break;
	case 4:
		sprintf(sFileName,"save/map%d.lan",arHumans[0].iFileIndex);
		break;
	case 5:
		sprintf(sFileName,"save/map%d.net",arHumans[0].iFileIndex);
		break;
	};
	WorkFile=pack_fopen(sFileName,"rp");
	bmpMap=load_pcx_pf(WorkFile,Palet);
	int Version=102;
	MapSize=pack_igetw(WorkFile);
	if (MapSize==0)
	{
		MapSize=pack_igetw(WorkFile);
		Version=103;
	};
	pFirstProvince=new dcProvince[MapSize+1];
	pFirstFog=new clFog[MapSize+1];
	pMoveMap=new int [MapSize+1];
	pRunMap=new int [MapSize+1];
	pMobilMap=new int [MapSize+1];
	clFog *FG=new clFog;
	dcProvince *PR=new dcProvince;
	dcOldHero *TH=new dcOldHero;
	dcUnit *TU=new dcUnit;	
	

	for (int i=1; i<=MapSize; i++)
	{
		pack_fread(FG,sizeof *FG,WorkFile);
		FG->Pic=load_pcx_pf(WorkFile,Palet);		
		*(pFirstFog+FG->Num)=*FG;		
		pack_fread(PR,sizeof *PR,WorkFile);
		char STR[200];
		int Count=0;
		STR[Count]=pack_getc(WorkFile);
		while (STR[Count]!='\0')
		{
			Count++;
			STR[Count]=pack_getc(WorkFile);
		};
		PR->sName=new char [Count+1];
		for (int i=0; i<=Count; i++)
		{
			*(PR->sName+i)=STR[i];
		};
		if (Version==102)
		{
		if (pack_getc(WorkFile))
		{
			
			pack_fread(TH,sizeof *TH,WorkFile);			
			for (int j=0; j<MAX_SQUAD; j++)
			{
				if (pack_getc(WorkFile))
				{					
					pack_fread(TU,sizeof *TU,WorkFile);
				}
				else
				{
		//			PR->pGuard->pSquad[j]=NULL;
				};
			};
		}
		else
		{
	//		PR->pGuard=NULL;
		};
		for (int k=0; k<MAX_PROVINCE_SITE; k++)
		{
			if (pack_getc(WorkFile))
			{
				pack_fread(TH,sizeof *TH,WorkFile);				
				for (int j=0; j<MAX_SQUAD; j++)
				{
					if (pack_getc(WorkFile))
					{						
						pack_fread(TU,sizeof *TU,WorkFile);
					}
					else
					{
			//			PR->arSiteGuard[k]->pSquad[j]=NULL;
					};
				};
			}
			else
			{
	//			PR->arSiteGuard[k]=NULL;
			};
		};
		};
		*(pFirstProvince+PR->iNumber)=*PR;
	};
	for (int i=0; i<MAP_SIZE; i++)
	{
		Map[i]=NULL;
	};
	Map_Border *BR=new Map_Border;
	int Index=pack_igetw(WorkFile);
	while (Index<4000)
	{
		pack_fread(BR,sizeof *BR,WorkFile);
		if (Map[Index])
		{
			Map_Border *TMP=Map[Index];
			while (TMP->Next)
			{
				TMP=TMP->Next;
			};
			TMP->Next=new Map_Border;
			*(TMP->Next)=*BR;
			TMP->Next->Next=NULL;
		}
		else
		{
			Map[Index]=new Map_Border;
			*Map[Index]=*BR;
			Map[Index]->Next=NULL;
		};
		Index=pack_igetw(WorkFile);		
	};
	pFirstProvince->sName=(pFirstPhrase+327)->str;
	pack_fclose(WorkFile);	
//	delete PR;
//	delete FG;
//	delete BR;
	TH->ipPlayer=0;
	for (int i=0; i<MAX_SQUAD; i++)
	{
		TH->pSquad[i]=NULL;
	};
	delete TH;
	delete TU;
};

void UnloadMap()
{
//	WriteLog("Графика");
	FileLog("Unloading map...");
	destroy_bitmap(bmpMap);
	bmpMap=NULL;
	for (int i=1; i<=MapSize; i++)
	{
		destroy_bitmap((pFirstFog+i)->Pic);		
	};
//	WriteLog("Туман карта");
//	readkey();
	delete [] pFirstFog;
//	WriteLog("Провинции");
//	readkey();
	pFirstProvince->pGarrison=NULL;
	pFirstProvince->pGuard=NULL;
	pFirstProvince->sName=new char[2];
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		pFirstProvince->arSiteGuard[i]=NULL;
	};
	delete [] pFirstProvince;

//	WriteLog("Карта движения");
//	readkey();
	delete [] pMoveMap;
	delete [] pRunMap;
	delete [] pMobilMap;
//	WriteLog("Границы");
//	readkey();
	Map_Border *BR, *BR1;
	for (int i=0; i<MAP_SIZE; i++)
	{
		BR=Map[i];
		while (BR)
		{
			BR1=BR->Next;
			delete BR;
			BR=BR1;
		};
	};	
	FileLog("Done!");
};