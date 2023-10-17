#include "Mapper.h"

bool bWorldRulesTest=false;

BITMAP *bmpShard[MAX_SHARD_PIC];	//Изображения осколков
BITMAP *bmpShardSelect[MAX_SHARD_PIC];
BITMAP *bmpMainShard;
BITMAP *bmpMainShardSelect;
BITMAP *bmpAstral;
BITMAP *bmpLowPanel;
BITMAP *bmpShardBonus[MAX_SHARD_BONUS_PIC];
dcShard * pFirstShard=NULL;
//dcCampPlayer arCampPlayer[MAX_PLAYERS];
dcCampaign Campaign;
dcTutorial Tutorial;
const int DIALOG_ALLOWED=6;
bool bInvasion;	//Флаг вторжения
bool bCycle, bTurn, bJournal, bShowRecords;
//int WorldSize;

bool PickBonusScreen(dcShard *Shard, int Mode);

//Возвращает размер мира указанного игрока
int GetWorldSize(int Player)	
{
	int Result=Campaign.arCampPlayer[Player].iSizeBonus;
	if (Player==0)
	{
		dcShard *SH=pFirstShard;
		while (SH)
		{
			if (SH->ipPlayer==0)
			{Result++;};
			SH=SH->pNext;
		};
	}
	else
	{
		int BLD=0;
		for (int i=1; i<=iMaxInnerBld; i++)
		{
			if ((Campaign.arCampPlayer[Player].arBld[i])&&((pFirstInnerBld+i)->iLevel>0))
			{BLD++;};
		};
		Result=BLD/3-Result;
	};
	return Result;
};

//Возвращает 0 для недоступной постройки, 1 для доступной (или расположенной на нейтральном осколке) и 2 для уже имеющейся у игрока
int GetBuildingStatus(int Build, int Player)
{
	if ((pFirstInnerBld+Build)->iLevel<1)
	{return 2;};
	if (Campaign.arCampPlayer[Player].arBld[Build]>0)
	{return 2;};
	dcShard *SH=pFirstShard;
	while (SH)
	{
		if (SH->ipPlayer==Player)
		{
			for (int i=0; i<MAX_SHARD_BUILD; i++)
			{
				if (SH->arBuilding[i]==Build)
				{return 2;};
			};
		};
		SH=SH->pNext;
	};
	for (int i=0; i<MAX_BUILDING_NEED; i++)
	{
		if (((pFirstInnerBld+Build)->arBuildings[i]>0)&&((pFirstInnerBld+Build)->arBuildings[i]<1000))
		{			
			if (GetBuildingStatus((pFirstInnerBld+Build)->arBuildings[i],Player)<2)
			{return 0;};
		};
		if ((pFirstInnerBld+Build)->arBuildings[i]>1000)
		{
			int iSum=0;
			for (int BL=1; BL<=iMaxInnerBld; BL++)
			{
				if ((pFirstInnerBld+BL)->iGroup==(pFirstInnerBld+Build)->arBuildings[i]-1000)
				{
					bool bAdd=false;
					if ((pFirstInnerBld+BL)->iLevel<1)
					{bAdd=true;};
					if (Campaign.arCampPlayer[Player].arBld[BL]>0)
					{bAdd=true;};
					dcShard *SH=pFirstShard;
					while (SH)
					{
						if (SH->ipPlayer==Player)
						{
							for (int k=0; k<MAX_SHARD_BUILD; k++)
							{
								if (SH->arBuilding[k]==BL)
								{bAdd=true;};
							};
						};
						SH=SH->pNext;
					};
					if (bAdd)
					{iSum++;};
				};				
			};
			if (iSum<arBldGroup[(pFirstInnerBld+Build)->arBuildings[i]-1000])
			{return 0;};
		};
	};
	return 1;
};

int IsBuildingAllowed(int Build, int Player)
{
	if ((pFirstInnerBld+Build)->iLevel<1)
	{return 2;};
	if (Campaign.arCampPlayer[Player].arBld[Build]>0)
	{return 2;};	
	for (int i=0; i<MAX_BUILDING_NEED; i++)
	{
		if (((pFirstInnerBld+Build)->arBuildings[i]>0)&&((pFirstInnerBld+Build)->arBuildings[i]<1000))
		{			
			if (IsBuildingAllowed((pFirstInnerBld+Build)->arBuildings[i],Player)<2)
			{return 0;};
		};
	};
	return 1;
};

bool IsBuildingFree(int Build)
{
	dcShard *SH=pFirstShard;
	while (SH)
	{
		if (SH->ipPlayer<1)
		{
			for (int i=0; i<MAX_SHARD_BUILD; i++)
			{
				if (SH->arBuilding[i]==Build)
				{return false;};
			};
		};
		SH=SH->pNext;
	};
	return true;
};


dcCampaign::dcCampaign()
{
	iCurrentTurn=0;
	iMaxShards=0;
	iTimeBonus=0;
	iDifficulty=0;
	iReserv=0;
	for (int i=0; i<1000; i++)
	{
		arJournal[i]=0;		
	};
	arJournal[980]=-1;
	arJournal[979]=-1;
	arJournal[978]=arCampDiff[0][3];
	for (int i=0; i<2000; i++)
	{
		arDialog[i]=0;
	};
	for (int i=0; i<20; i++)
	{
		arItems[i]=0;
	};
	for (int i=0; i<=DIALOG_ALLOWED; i++)
	{
		arDialog[i]=1;
	};
	for (int i=0; i<50; i++)
	{Events[i]=0;};
};

dcCampaign::~dcCampaign()
{};

void dcCampaign::Clear()
{
	iCurrentTurn=0;
	iMaxShards=0;
	iDifficulty=0;
	iTimeBonus=0;
	iReserv=0;
	iDifficulty=0;
	pFirstShard=NULL;
	for (int i=0; i<1000; i++)
	{
		arJournal[i]=0;		
	};
	arJournal[980]=-1;
	arJournal[979]=-1;
	arJournal[978]=arCampDiff[0][3];
	for (int i=0; i<2000; i++)
	{
		arDialog[i]=0;
	};
	for (int i=0; i<=DIALOG_ALLOWED; i++)
	{
		arDialog[i]=1;
	};
	for (int i=0; i<20; i++)
	{
		arItems[i]=0;
	};
	for (int i=0; i<MAX_CAMP_QUEST; i++)
	{
		for (int j=0; j<4; j++)
		{arQuest[i][j]=0;};
	};	
};


dcShard * dcCampaign::GetActiveShard()
{
	dcShard *SH=pFirstShard;
	while (SH)
	{
		if (SH->Num==iActiveShard)
		{return SH;};
		SH=SH->pNext;
	};
	return NULL;
};


void dcCampPlayer::Clear()
{
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if ((pFirstInnerBld+i)->iLevel>0)
		{arBld[i]=0;}
		else
		{arBld[i]=1;};
	};
	bFirstMeet=false;
	bShardMeet=false;
	bInGame=true;
	iTreaty=0;
	iTreaty1=0;	
	iEnergyBonus=0;
	iEnergyPenalty=0;
	iEnergyLose=0;
	iEnergyDrain=0;
	iSizeBonus=0;
	iScore=0;
	iMapScore=0;
	iQuestScore=0;
	iDipScore=0;
	iKillScore=0;
	ipProfile=0;
};


void dcCampPlayer::AddBuild()
{
	int MinLvl=20;
	if (ipProfile<16)
	{
		for (int i=0; i<MAX_PLAYER_BLD; i++)
		{
			if (((pFirstPlayerProfile+ipProfile)->arBuild[i]>0)&&(arBld[(pFirstPlayerProfile+ipProfile)->arBuild[i]]==0)&&((pFirstInnerBld+(pFirstPlayerProfile+ipProfile)->arBuild[i])->iLevel<MinLvl))
			{MinLvl=(pFirstInnerBld+(pFirstPlayerProfile+ipProfile)->arBuild[i])->iLevel;};
		};
	};
	ClearRoll();
	int Count=0;
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if (IsBuildingAllowed(i,ipProfile)==1)
		{
			RollerN[Count]=i;
			RollerP[Count]=10;
			if (ipProfile<16)
			{
				for (int j=0; j<MAX_PLAYER_BLD; j++)
				{
					if ((pFirstPlayerProfile+ipProfile)->arBuild[j]>0)
					{
						if ((pFirstPlayerProfile+ipProfile)->arBuild[j]==i)
						{RollerP[Count]+=100;}
						else
						{
							if (((pFirstInnerBld+(pFirstPlayerProfile+ipProfile)->arBuild[j])->iLevel==MinLvl)&&(IsParent(i,(pFirstPlayerProfile+ipProfile)->arBuild[j],-1)))
							{
								RollerP[Count]+=20;
							};
						};
					};
				};
				if ((pFirstPlayerProfile+ipProfile)->iKarma!=0)
				{
					for (int j=0; j<MAX_BUILDING_ABILITY; j++)
					{
						if ((pFirstInnerBld+i)->arAbility[j][0]==46)
						{
							if (sign((pFirstPlayerProfile+ipProfile)->iKarma)==sign((pFirstInnerBld+i)->arAbility[j][1]))
							{RollerP[Count]*=2;}
							else
							{RollerP[Count]/=3;};
						};
					};
				};
			};			
			Count++;
		};
	};
	if (Count>0)
	{
		int Pick=Roll(false);
		arBld[Pick]=1;
		FileLog((pFirstInnerBld+Pick)->sName);
	};
};

int dcCampPlayer::GetAttitude()
{
	int Result=arAttitude[0];
	if (iKarma>=20)
	{		
		Result+=(Campaign.arCampPlayer[0].iKarma/10)*(iKarma/10)/5;
	};
	if (iKarma<=-20)
	{		
		Result+=(Campaign.arCampPlayer[0].iKarma/10)*(iKarma/10)/10;
	};
	if ((abs(Campaign.arCampPlayer[0].iKarma)<20)&&(abs(iKarma)<20))
	{
		Result+=5-abs(Campaign.arCampPlayer[0].iKarma-iKarma)/10;
	};	
	for (int i=1; i<MAX_PLAYERS; i++)
	{
		if ((i!=ipProfile)&&(Campaign.arCampPlayer[i].bInGame))
		{
			Result+=(Campaign.arCampPlayer[i].arAttitude[0]/10)*(arAttitude[i]/50);
		};
	};
	return Result;
};


dcTutorial::dcTutorial()
{
	Mode=false;
	Map=false;
};

void dcTutorial::Init()
{
	Mode=true;
	Map=true;
	Move=1;
	Trade=0;
	Hire=0;
	Library=0;
	Repair=0;
	Build=0;
	Mercenary=0;
	Garrison=0;
	Scroll=0;
	Egg=0;
	Castles=0;
	RitualHero=0;
	RitualProv=0;
	RitualCast=0;
	Death=0;
	Hero=0;
	Population=0;
	Resource=0;
	Groups=0;
	SpellRank=0;
	UnitRank=0;
	Task=0;
	PrvBuild=0;
	PrvGuard=0;
	Turn=0;
	Battle=1;
	Site=-1;
	Stamina=1;	
	Ranged=1;
	Heal=1;
	Morale=1;
	Ammo=1;
	Orc=1;
	Fortify=0;
	Magic=1;
	Bonus=0;
	Alchemy=0;
	Corrupt=0;
	Quest=0;
	CastleDefense=0;
	Diplomacy=0;
	NoDip=0;
	DipTrade=0;
	Exchange=0;
	War=0;
	Peace=0;
	Union=0;
	Broken=1;
	SquadMorale=0;
	LostShard=0;
	DefendWorld=0;
	DefendCastle=0;
	UnitPrice=0;
	Add1=0;
	Add2=0;
	buttEnd=0;
	buttCastle=0; 
	buttStat=0;
	buttDiplo=0;
	buttNext=1;
	buttBuild=0;
	buttGuard=0;
	buttEnter=0;
	buttRitual=0;
	buttShop=0;
	buttLibrary=0;
	buttGarrison=0;
	buttPillage=0;
	buttAttack=0;
	buttExplore=0;
	buttRitHero=0;	
};

dcShard::dcShard()
{};

dcShard::dcShard(dcShard * Next, int Terra, int Bld, int Lvl)
{
	Num=0;
	dcShard *SH;
	bool bTake=true;
	while (bTake)
	{
		bTake=false;
		SH=pFirstShard;
		while (SH)
		{
			if (SH->Num==Num)
			{
				bTake=true;
				SH=NULL;
			}
			else
			{SH=SH->pNext;};
		};
		if (bTake)
		{Num++;};
	};	
	Campaign.iMaxShards++;
	iLevel=Lvl;
	Income=Lvl+Random(Lvl*2);	
	Income=Income*arCampDiff[Campaign.iDifficulty][4]/100;
	int Suf=Random(iMaxSuffix);
	int Pref=Random(iMaxPrefix);
	sprintf(sName,"%s%s",(pFirstPrefix+Pref)->str,(pFirstSuffix+Suf)->str);
	pNext=Next;
	ipPlayer=-1;
	Terrain=Terra;
	iSeed=Random(10000);
	for (int i=0; i<MAX_SHARD_BUILD; i++)
	{arBuilding[i]=0;};
	for (int i=0; i<MAX_SHARD_BONUS; i++)
	{arBonus[i][0]=0;};
	for (int i=0; i<MAX_SHARD_EVENT; i++)
	{arEvent[i]=0;};
	Pic=bmpShard[Terra];
	Select=bmpShardSelect[Terra];
	Map.Type=2;
	Map.Water=10+Random(5);
	Map.Hill=12+Random(10);
	Map.Forest=7+Random(10);
	Map.Swamp=7+Random(10);
	Map.Isles=5+Random(10);
	Map.Dissolve=20+Random(20);
	Map.Neutrals=1+(Lvl-1)*2;
	if (Lvl>6)
	{Map.Neutrals=20;};
	switch(Terrain)
	{
	case 0:
		Map.Water+=20;
		Map.Type=1;
		break;		
	case 2:
		Map.Forest+=20;
		break;
	case 3:
		Map.Hill+=20;
		break;
	case 4:
		Map.Swamp+=20;
		break;
	case 5:
		Map.Water+=3;
		Map.Hill+=3;
		Map.Forest+=3;
		Map.Swamp+=3;
		break;
	};
//	Map.Players=Bld-1;
//	Map.X=6+(Bld-3)*2;
//	Map.Y=6+(Bld-3)*2;	
	Map.Players=2+(Lvl-1-(Lvl-1)%2)/2;
	Map.X=6+((Lvl-Lvl%2)/2)*2;
	Map.Y=6+((Lvl-Lvl%2)/2)*2;
	if (Bld==4)
	{
		Map.Players++;
		Map.X+=2;
		Map.Y+=2;
		Map.Neutrals++;
	};	
};

dcShard::~dcShard()
{};

//Заполнение массива бонусов новых осколков
void dcShard::FillBonus()
{
	if (arBonus[0][0]==0)
	{
		int Bonus=1+(iLevel)/8+Random(2+(iLevel-1)/3);
		if (Bonus>4)
		{Bonus=4;};
		ClearRoll();
		int Count=0;
		bool arBl[8], CanBuild=false;
		bool arRes[9], CanRes=false;
		for (int i=0; i<8; i++)
		{arBl[i]=true;};
		for (int i=0; i<9; i++)
		{arRes[i]=true;};
		dcShard *SH=pFirstShard;
		while (SH)
		{
			for (int i=0; i<MAX_SHARD_BONUS; i++)
			{
				if ((SH->arBonus[i][0]==7)&&(SH->arBonus[i][1]<8))
				{arBl[SH->arBonus[i][1]-1]=false;};
				if (SH->arBonus[i][0]==8)
				{arRes[SH->arBonus[i][1]-1]=false;};
			};
			SH=SH->pNext;			
		};
		for (int i=0; i<8; i++)
		{
			if (arBl[i]){CanBuild=true;};
		};
		for (int i=0; i<9; i++)
		{
			if (arRes[i])
			{
				for (int j=0; j<MAX_SHARD_BONUS_TYPE; j++)
				{
					if ((pFirstShardBonus+8)->arLevel[iLevel-1][j][0]==i+1)
					{CanRes=true;};
				};
			};
		};
		for (int i=1; i<=iMaxShardBonus; i++)
		{
			if (((pFirstShardBonus+i)->arLevel[iLevel-1][0][0]>0)&&((i!=7)||(CanBuild))&&((i!=8)||(CanRes)))
			{
				RollerN[Count]=i;
				RollerP[Count]=5-(pFirstShardBonus+i)->iRarity;
				Count++;
			};
		};		
		for (int i=0; i<Bonus; i++)
		{
			arBonus[i][0]=Roll(true);
		};

		for (int i=0; i<Bonus; i++)
		{
			if ((pFirstShardBonus+arBonus[i][0])->iType==0)
			{
				arBonus[i][1]=(pFirstShardBonus+arBonus[i][0])->arLevel[iLevel-1][0][0]+Random(1+(pFirstShardBonus+arBonus[i][0])->arLevel[iLevel-1][0][1]-(pFirstShardBonus+arBonus[i][0])->arLevel[iLevel-1][0][0]);
			}
			else
			{
				ClearRoll();
				Count=0;
				for (int j=0; j<MAX_SHARD_BONUS_TYPE; j++)
				{
					if (((pFirstShardBonus+arBonus[i][0])->arLevel[iLevel-1][j][0]>0)
						&&((arBonus[i][0]!=7)||(arBl[(pFirstShardBonus+arBonus[i][0])->arLevel[iLevel-1][j][0]-1]))
						&&((arBonus[i][0]!=8)||(arRes[(pFirstShardBonus+arBonus[i][0])->arLevel[iLevel-1][j][0]-1])))
					{
						RollerN[Count]=(pFirstShardBonus+arBonus[i][0])->arLevel[iLevel-1][j][0];
						RollerP[Count]=(pFirstShardBonus+arBonus[i][0])->arLevel[iLevel-1][j][1];
						Count++;
					};
				};
				arBonus[i][1]=Roll(false);
			};		
		};
	};
};


//int GetAttitude(int Player)
//{
//	int Result=Campaign.arCampPlayer[Player].arAttitude[0];
//	if ((pFirstPlayerProfile+Player)->iKarma!=0)
//	{
//		int Mult=1+abs((pFirstPlayerProfile+Player)->iKarma/50);
//		Result+=sign((pFirstPlayerProfile+Player)->iKarma)*sign(Campaign.arCampPlayer[0].iKarma)*abs(Campaign.arCampPlayer[0].iKarma)*Mult/10;
//	}
//	else
//	{
//		Result-=abs(Campaign.arCampPlayer[0].iKarma/50);
//	};
//	return Result;
//};

//Возвращает доход энергии с мира игрока
int GetEnergyIncome()
{
	int Result=0;
	dcShard *SH=pFirstShard;	
	while (SH)
	{
		if (SH->ipPlayer==0)
		{
			Result+=SH->Income;			
		};
		SH=SH->pNext;		
	};
//	Result=Result*arCampDiff[Campaign.iDifficulty][4]/100;
	Result-=Result*Campaign.arCampPlayer[0].iEnergyLose/100;
	Result-=Campaign.arCampPlayer[0].iEnergyDrain;
	if (Result<0)
	{Result=0;};
	return Result;
};

void AddEvent(int Index)
{
	for (int i=0; i<50; i++)
	{
		if (Campaign.Events[i]==Index)
		{return;};
	};
	for (int i=0; i<50; i++)
	{
		if (Campaign.Events[i]==0)
		{
			Campaign.Events[i]=Index;
			return;
		};
	};
};

bool NoEvents()
{
	for (int i=0; i<50; i++)
	{
		if (Campaign.Events[i]>0)
		{			
			return false;
		};
	};
	return true;
};

void BanishPlayer(int Player)
{
	for (int k=0; k<MAX_PLAYER_DIALOG; k++)
	{
		if ((pFirstPlayerProfile+Player)->arExitDlg[k]>0)
		{Campaign.arDialog[(pFirstPlayerProfile+Player)->arExitDlg[k]]=0;};
	};
	Campaign.arCampPlayer[Player].bInGame=false;

	//Сплетня о Дариоле
	if ((Player==5)||(Player==14))
	{Campaign.arDialog[646]=1;};
	for (int i=1; i<MAX_PLAYERS; i++)
	{
		if ((i!=Player)&&((pFirstPlayerProfile+i)->arAtt[Player]>0))
		{Campaign.arCampPlayer[i].arAttitude[1]-=(pFirstPlayerProfile+i)->arAtt[Player]/5;};
		if ((i!=Player)&&((pFirstPlayerProfile+i)->arAtt[Player]<0))
		{Campaign.arCampPlayer[i].arAttitude[1]-=(pFirstPlayerProfile+i)->arAtt[Player]/10;};
	};
};

bool bFinish;


void FinishCampaign(int Index)
{
	//0 - сдался, 1 - поражение при защите своего мира, 2 - подчинился другому правителю, 3 - Уль-Даган разрушил мир, 4 - стал слугой Белеза, 5 - уничтожил всех Разрушителем
	//6 - подчинил всех Разрушителем, 7 - собрал все Ключи Истины, 8 - победил всех оппонентов, 9 - уничтожил всё живое, 10 - собрал Оберег, 11 - вторжение демонов
	if (bFinish)
	{return;};
	bFinish=true;
	if (iGameMode!=5)
	{
		iGameMode=5;
		ChangeMusic();
	};
	bCycle=false;
	BITMAP *bmpScr=create_bitmap(1024,768);
	draw_sprite(bmpScr,bmpAvatar[18],448,43);
	draw_sprite(bmpScr,bmpGUI_EndCamp,0,0);
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpScr);	
	UI_Button1 buttExit;
	objGUI=buttExit.Init(483,261,1,2,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;

	if (Campaign.arJournal[979]<0)
	{Campaign.iDifficulty=0;}
	else
	{Campaign.iDifficulty=Campaign.arJournal[979];};

	UI_TextBox txtLegend;
	objGUI=txtLegend.Init(70,82,310,195,2,COLOR_1,Font2,objGUI,201,51,201,290);
	if (Index<3)
	{
		char Txt[2000], Txt1[2000];
		int Memory=0;
		if (Campaign.arCampPlayer[0].iScore>1000)
		{Memory=1;};
		if (Campaign.arCampPlayer[0].iScore>3000)
		{Memory=2;};
		if (Campaign.arCampPlayer[0].iScore>10000)
		{Memory=3;};
		sprintf(Txt,(pFirstMessage+55)->msg,(pFirstMessage+75+Index)->msg,(pFirstMessage+88+Memory)->msg);
		sprintf(Txt1,Txt,arHumans[0].sName);		
		txtLegend.Activate(Txt1);
	}
	else
	{txtLegend.Activate((pFirstMessage+75+Index)->msg);};

	textprintf_centre_ex(bmpScr,Font1,512,23,COLOR_1,-1,"%s %s",arHumans[0].sName,GetRulerNick(Campaign.arCampPlayer[0].iKarma));
	textprintf_centre_ex(bmpScr,Font1,512,204,COLOR_1,-1,arDifficulty[Campaign.iDifficulty].sName);	
	textprintf_ex(bmpScr,Font1,650,83,COLOR_1,-1,"%s: %d",(pFirstPhrase+442)->str,GetWorldSize(0));
	textprintf_ex(bmpScr,Font1,650,103,COLOR_1,-1,"%s: %d",(pFirstPhrase+273)->str,Campaign.arCampPlayer[0].iKillScore);
	textprintf_ex(bmpScr,Font1,650,123,COLOR_1,-1,"%s: %d",(pFirstPhrase+441)->str,Campaign.arCampPlayer[0].iMapScore);
	int Score=(Campaign.arCampPlayer[0].iScore+Campaign.arCampPlayer[0].iKillScore*50)/4*(1+Campaign.iDifficulty)+Campaign.arCampPlayer[0].iMapScore/(10+Campaign.iCurrentTurn/10);
	switch (Index)
	{
	case 0:
	case 1:
		Score/=5;
		break;
	case 2:
		Score/=4;
		break;
	case 3:
	case 11:
		Score/=3;
		break;
	case 4:
	case 9:
		Score/=2;
		break;
	case 7:
		Score+=400;
		break;
	case 10:
		Score+=150;
		break;
	};
	textprintf_centre_ex(bmpScr,Font1,512,235,COLOR_1,-1,"%s: %d",(pFirstPhrase+483)->str,Score);
	for (int i=0; i<10; i++)
	{
		if (arCampRecords[i].Score<Score)
		{
			for (int j=9; j>i; j--)
			{
				arCampRecords[j]=arCampRecords[j-1];
			};
			arCampRecords[i].Score=Score;
			arCampRecords[i].Col1=arHumans[0].iColor1;
			arCampRecords[i].Col2=arHumans[0].iColor2;
			arCampRecords[i].Sign=arHumans[0].iSign;
			arCampRecords[i].ipPortrait=Index;
			sprintf(arCampRecords[i].sName,"%s %s %s",GetRulerTitle(Score/5),arHumans[0].sName,GetRulerNick(Campaign.arCampPlayer[0].iKarma));
			SaveCampRecords();
			bShowRecords=true;
			break;
		};
	};
	while (!buttExit.Check())
	{
		txtLegend.Check();
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,5,true);			
		};
		bHint=false;
	};

	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};

	dcShard *SH, *SH1;
	SH=pFirstShard;
	while (SH)
	{
		SH1=SH->pNext;
		delete SH;
		SH=SH1;
	};
	pFirstShard=NULL;
	char sFile[100];
	sprintf(sFile,"save\\game%d.cmp",arHumans[0].iFileIndex);
	delete_file(sFile);
	sprintf(sFile,"save\\map%d.cmp",arHumans[0].iFileIndex);
	delete_file(sFile);
};

//Проверка поражения и победы в кампании. Возвращает индекс проигрыша.
void IsEndGame()
{
	if (bFinish)
	{return;};
	bool bWin=true;
	for (int i=1; i<17; i++)
	{
		if (Campaign.arCampPlayer[i].bInGame)
		{bWin=false;};
	};
	if (bWin)
	{		
		FinishCampaign(8);
		return;
	};

	if (Campaign.arItems[3]>=1000)
	{
		(pFirstCampDialog+1277)->Show(1,0,0,0,NULL,NULL);
		FinishCampaign(10);
		return;
	};

	if ((bCycle)&&(Campaign.iCurrentTurn>=Campaign.arJournal[978]+Campaign.iTimeBonus))
	{
		if (Campaign.arDialog[670]>0)
		{(pFirstCampDialog+1278)->Show(1,0,0,0,NULL,NULL);}
		else
		{(pFirstCampDialog+1230)->Show(1,0,0,0,NULL,NULL);};
		FinishCampaign(11);
		return;
	};
	dcShard *SH=pFirstShard;
	while (SH)
	{
		if (SH->ipPlayer==-1)
		{return;};
		SH=SH->pNext;		
	};
	if (Campaign.arDialog[90]==0)
	{
		(pFirstCampDialog+1220)->Show(1,0,0,0,NULL,NULL);
		FinishCampaign(1);
	}
	else
	{
		int AddY=(GetWorldSize(0)-1)*50;
		for (int i=1; i<17; i++)
		{
			if ((Campaign.arCampPlayer[i].bInGame)&&(AddY>=Campaign.arCampPlayer[i].Y))
			{return;}
		};
		if (Campaign.arCampPlayer[9].bFirstMeet)
		{
			char Txt[1000];
			int Winner=9;
			if (!Campaign.arCampPlayer[9].bInGame)
			{
				Winner=1;
				while(!Campaign.arCampPlayer[Winner].bInGame)
				{Winner++;};
			};
			sprintf(Txt,(pFirstCampDialog+1219)->sText,(pFirstPlayerProfile+Winner)->sName);
			int Answer=(pFirstCampDialog+1219)->Show(1,1,0,0,NULL,Txt);
			if (Answer==1)
			{
				FinishCampaign(2);
			}
			else
			{
				(pFirstCampDialog+1221)->Show(1,0,0,0,NULL,NULL);
				FinishCampaign(1);
			};
		}
		else
		{
			(pFirstCampDialog+1220)->Show(1,0,0,0,NULL,NULL);
			FinishCampaign(1);
		};
	};
};

//void WinCampaign(int Index)
//{};

//****Вызов событий в кампании
void RunCampaignEvent(int Index, int Player, int Param)
{	
	clEvent *EV=(pFirstCampEvent+Index);
	char Mess[1000];
	
//Изменение кармы
	Campaign.arCampPlayer[0].iKarma+=EV->iKarma;	

//Воздействие эффектов
	long int Mult=0, iWork;
	//Индивидуальные эффекты
	for (int Eff=0; Eff<MAX_EVENT_EFFECT; Eff++)
	{
		switch (EV->arEffect[Eff][0])
		{
		case 1: //Получение энергии
			iWork=EV->arEffect[Eff][1]+Random(EV->arEffect[Eff][2]);
			Campaign.arCampPlayer[EV->arEffect[Eff][3]].iEnergy+=iWork;
			if (Eff==0)
			{Mult=iWork;};
			blit(bmpGUI_AstralLow,bmpLowPanel,0,0,0,0,bmpGUI_AstralLow->w,bmpGUI_AstralLow->h);
			textprintf_ex(bmpLowPanel,Font1,135,79,COLOR_2,-1,"%d",Campaign.arCampPlayer[0].iEnergy);
			break;
		case 2: //Изменение отношений
			if (EV->arEffect[Eff][2]==0)
			{Campaign.arCampPlayer[Player].arAttitude[EV->arEffect[Eff][3]]+=EV->arEffect[Eff][1];}
			else
			{Campaign.arCampPlayer[EV->arEffect[Eff][2]].arAttitude[EV->arEffect[Eff][3]]+=EV->arEffect[Eff][1];};
			break;
		case 3: //Изменение флага состояния
			if ((EV->arEffect[Eff][3]==0)||(Campaign.arDialog[EV->arEffect[Eff][3]]>0))
			{Campaign.arDialog[EV->arEffect[Eff][1]]=EV->arEffect[Eff][2];};
			break;
		case 5: //Изменение дохода энергии
			iWork=EV->arEffect[Eff][1]+Random(EV->arEffect[Eff][2]);
			if (iWork>0)
			{Campaign.arCampPlayer[EV->arEffect[Eff][3]].iEnergyBonus+=iWork;}
			else
			{Campaign.arCampPlayer[EV->arEffect[Eff][3]].iEnergyPenalty+=iWork;}
			if (Eff==0)
			{Mult=iWork;};
			break;
		case 6: //Изменение внутреннего параметра
			Param=EV->arEffect[Eff][1];
			break;
		case 7: //Изменение кармы
			Campaign.arCampPlayer[EV->arEffect[Eff][3]].iKarma+=EV->arEffect[Eff][1]+Random(EV->arEffect[Eff][2]);
			break;
		case 12: //Изменение Treaty
			Campaign.arCampPlayer[EV->arEffect[Eff][1]].iTreaty=EV->arEffect[Eff][2];
			break;
		case 13:
			iWork=0;
			for (int i=0; i<MAX_CAMP_QUEST; i++)
			{
				if (Campaign.arQuest[i][0]==EV->arEffect[Eff][1])
				{
					Campaign.arQuest[i][1]=EV->arEffect[Eff][2];
					iWork=1;
				};
			};
			if (iWork==0)
			{
				for (int i=0; i<MAX_CAMP_QUEST; i++)
				{
					if (Campaign.arQuest[i][0]==0)
					{
						Campaign.arQuest[i][0]=EV->arEffect[Eff][1];
						Campaign.arQuest[i][1]=EV->arEffect[Eff][2];
						break;
					};
			}	;
			};
			break;
		case 14:
			bInvasion=EV->arEffect[Eff][1];
			break;
		case 15:
			Campaign.arItems[EV->arEffect[Eff][1]]+=EV->arEffect[Eff][2];
			if (EV->arEffect[Eff][3]>0)
			{Campaign.arItems[EV->arEffect[Eff][1]]+=Campaign.arItems[EV->arEffect[Eff][3]];};
			break;
		case 17:
			if (Campaign.Events[0]>0)
			{
				for (int i=48; i<=0; i--)
				{Campaign.Events[i+1]=Campaign.Events[i];};
			};
			Campaign.Events[0]=EV->arEffect[Eff][1];
			break;
		//Уничтожить игрока
		case 18:
			BanishPlayer(EV->arEffect[Eff][1]);
			break;
		case 19:
		//Переход к событию, если владыка жив
			if (Campaign.arCampPlayer[EV->arEffect[Eff][2]].bInGame)
			{
				RunCampaignEvent(EV->arEffect[Eff][1], Player, Param);
				return;
			};
			break;
		case 21:
		//Нападение на игрока
			bTurn=true;
			PickBonusScreen(NULL,-EV->arEffect[Eff][1]);
			break;
		case 22: 
		//Понижение дохода энергии в %
			iWork=EV->arEffect[Eff][1]+Random(EV->arEffect[Eff][2]);
			Campaign.arCampPlayer[0].iEnergyLose+=iWork;			
			if (Eff==0)
			{Mult=iWork;};
			break;
		case 23:
		//Запись в журнале			
			Campaign.arJournal[EV->arEffect[Eff][1]]=EV->arEffect[Eff][2];
			if (EV->arEffect[Eff][2]>0)
			{bJournal=true;};
			break;
		case 24:
		//Дополнение времени
			Campaign.iTimeBonus+=EV->arEffect[Eff][1];			
			break;
		case 25:
		//Изменение славы
			Campaign.arCampPlayer[0].iMapScore+=EV->arEffect[Eff][1];			
			break;
		};
	};
	//Обычные эффекты	

	if (EV->ipDialog)
	{
//Вызов диалога
		if (EV->iDlgParam1>=10)
		{
			int arAnswer[4], arEvent[4];
			for (int i=0; i<4; i++)
			{arAnswer[i]=0;};
			int Count=0;
			for (int Eff=0; Eff<MAX_EVENT_EFFECT; Eff++)
			{
				if (EV->arEffect[Eff][0]==8)
				{
					/*if (Campaign.arDialog[EV->arEffect[Eff][2]]>0)
					{
						if ((Count>=Param*3)&&(Count<Param*3+4))
						{
							arAnswer[Count-Param*3]=EV->arEffect[Eff][1];
							arEvent[Count-Param*3]=EV->arEffect[Eff][3];
						};
						Count++;
					};*/
					for (int i=0; i<=iMaxQuestion; i++)
					{
						if (((pFirstQuestion+i)->iQuester==EV->arEffect[Eff][1])&&(Campaign.arDialog[(pFirstQuestion+i)->iTrigger]>0))
						{
							if ((Count>=Param*3)&&(Count<Param*3+4))
							{
								arAnswer[Count-Param*3]=i+1;
								arEvent[Count-Param*3]=(pFirstQuestion+i)->iEvent;
							};
							Count++;
						};
					};
					break;
				};
			};			
			if (Count<Param*3+4)
			{
				arAnswer[Count-Param*3]=-2;
				arEvent[Count-Param*3]=0;				
			};
			if (Count>Param*3+4)
			{
				arAnswer[3]=-1;
				arEvent[3]=Index;
				Param++;
			};
			DialogDX=0;
			int	Answer=(pFirstCampDialog+EV->ipDialog)->Show(arAnswer[0],arAnswer[1],arAnswer[2],arAnswer[3],NULL,NULL)-1;	
			if (arEvent[Answer]>0)
			{RunCampaignEvent(arEvent[Answer],Player,Param);};
		}
		else
		{
		if (EV->iDlgParam1==0)
		{sprintf(Mess,(pFirstCampDialog+EV->ipDialog)->sText);}
		if (EV->iDlgParam1==1)
		{
			if (EV->iDlgParam2==2)
			{sprintf(Mess,(pFirstCampDialog+EV->ipDialog)->sText,arHumans[0].sName,Mult);}
			else
				{sprintf(Mess,(pFirstCampDialog+EV->ipDialog)->sText,arHumans[0].sName);}
		};
		if (EV->iDlgParam1==2)
		{
			if (EV->iDlgParam2==1)
				{sprintf(Mess,(pFirstCampDialog+EV->ipDialog)->sText,Mult,arHumans[0].sName);}
			else
				{sprintf(Mess,(pFirstCampDialog+EV->ipDialog)->sText,Mult);}
		};
		int An[MAX_ANSWER];
		for (int i=0; i<MAX_ANSWER; i++)
		{
			bool bWork;
			An[i]=0;
			if ((pFirstCampDialog+EV->ipDialog)->arAnswer[i][1]!='#')			
			{
				An[i]=1;
				for (int j=0; j<MAX_EVENT_EFFECT;j++)
				{
					if ((EV->arEffect[j][0]==4)&&(EV->arEffect[j][1]==i+1))
					{							
						switch (EV->arEffect[j][2])
						{
						case 1:
							if (Campaign.arCampPlayer[0].iEnergy<EV->arEffect[j][3])
							{An[i]=0;};
							break;
						case 2:
							if (Campaign.arDialog[EV->arEffect[j][3]]==0)
							{An[i]=0;};
							break;
						case 3:
							if (Campaign.arItems[1]<1)
							{An[i]=0;};
							break;
						case 4:
							if (Campaign.arItems[0]<2)
							{An[i]=0;};
							break;
						case 5:
							if (Campaign.arDialog[EV->arEffect[j][3]]!=0)
							{An[i]=0;};
							break;
						};							
					};	
				};				
			};
		};
		
		DialogDX=0;
		if ((Campaign.arItems[0]>=2)&&(EV->CalcEffect(16,0,0)>0))
		{iHeroAdvice=2;};
		int	Answer=(pFirstCampDialog+EV->ipDialog)->Show(An[0],An[1],An[2],An[3],NULL,Mess)-1;
		if (EV->arEvent[Answer]>0)
		{
			RunCampaignEvent(EV->arEvent[Answer],Player,Param);	
			return;
		};
		};
	};
	
//Вызов следующего события
	//Переход по отношениям
	for (int Eff=0; Eff<MAX_EVENT_EFFECT; Eff++)
	{
		if  (EV->arEffect[Eff][0]==9)
		{
			if (Campaign.arCampPlayer[EV->arEffect[Eff][1]].GetAttitude()<EV->arEffect[Eff][2])
			{
				RunCampaignEvent(EV->arEffect[Eff][3],Player,Param);
				return;				
			};			
		};

	//Переход по триггерам	
		if  (EV->arEffect[Eff][0]==10)
		{
			if (Campaign.arDialog[EV->arEffect[Eff][1]]>0)
			{
				if (EV->arEffect[Eff][2]>0)
				{
					RunCampaignEvent(EV->arEffect[Eff][2],Player,Param);
					return;
				};
			}
			else
			{
				if (EV->arEffect[Eff][3]>0)
				{
					RunCampaignEvent(EV->arEffect[Eff][3],Player,Param);
					return;
				};
			};
		};

	//Случайный или безусловный переход	
		if  (EV->arEffect[Eff][0]==11)
		{
			if ((EV->arEffect[Eff][2]>0)&&(Random(100)>EV->arEffect[Eff][3]))
			{RunCampaignEvent(EV->arEffect[Eff][2],Player,Param);}
			else
			{RunCampaignEvent(EV->arEffect[Eff][1],Player,Param);};
			return;
		};

	//Переход по славе	
		if  (EV->arEffect[Eff][0]==26)
		{
			if (Campaign.arCampPlayer[0].iMapScore>=EV->arEffect[Eff][1])
			{
				if (EV->arEffect[Eff][2]>0)
				{
					RunCampaignEvent(EV->arEffect[Eff][2],Player,Param);
					return;
				};
			}
			else
			{
				if (EV->arEffect[Eff][3]>0)
				{
					RunCampaignEvent(EV->arEffect[Eff][3],Player,Param);
					return;
				};
			};
		};
	//Завершение кампании
		if  (EV->arEffect[Eff][0]==20)
		{		
			FinishCampaign(EV->arEffect[Eff][1]);
			return;
		};
	};
};


void MakeShards(int Player)
{
	int Allow=0, Own=0, Build=0, AddWorldRules=0;
	if (pFirstShard)
	{
		for (int i=1; i<=iMaxInnerBld; i++)
		{
			if ((GetBuildingStatus(i,Player)==1)&&(IsBuildingFree(i)))
			{Allow++;};
			if (GetBuildingStatus(i,Player)==2)
			{Build++;};
		};
		if (Allow<3)
		{Allow=0;}
		else
		{
		if (Allow<10)
		{Allow=1;}
		else
		{
			if (Allow<20)
			{Allow=2;}
			else
			{Allow=3;};
		};
		};
		dcShard *SH=pFirstShard;
		while (SH)
		{
			if (SH->ipPlayer==0)
			{Own++;};
			SH=SH->pNext;
		};
		
	}
	else
	{
		Allow=5;
		pFirstShard=new dcShard(pFirstShard,5,2,1);			
		pFirstShard->arBuilding[0]=17;
		pFirstShard->arBuilding[1]=19;
		pFirstShard->x=480;
		pFirstShard->y=100;
		pFirstShard->arBonus[0][0]=1;
		pFirstShard->arBonus[0][1]=10;
		pFirstShard->Income=10;
		if ((bWorldRulesTest)&&(bUseWorldRules))
		{AddWorldRules=iMaxWorldRules;};
	};
	if (Allow>0)
	{
	for (int Lvl=1; Lvl<10; Lvl++)
	{
		int Count=0;
		ClearRoll();
		for (int i=1; i<=iMaxInnerBld; i++)
		{
			if (((pFirstInnerBld+i)->iLevel<=Lvl)&&(GetBuildingStatus(i,Player)==1)&&(IsBuildingFree(i)))
			{
				RollerN[Count]=i;
				RollerP[Count]=10;
				Count++;
			};			
		};
		int Try;
		while ((Count>2)&&(Allow>0))
		{
			int Bld=Random(2)+3;
			if (Bld>Count)
			{Bld=Count;};
			Count-=Bld;
			Allow--;
			pFirstShard=new dcShard(pFirstShard,Random(6),Bld,Lvl);
			if (AddWorldRules>0)
			{
				pFirstShard->arEvent[0]=AddWorldRules;
				sprintf(pFirstShard->sName,(pFirstWorldRules+AddWorldRules)->sName);
				AddWorldRules--;
			};
			for (int i=0; i<Bld; i++)
			{
				pFirstShard->arBuilding[i]=Roll(true);
			};
			bool bNeedPlace=true;
			Try=0;
			while (bNeedPlace)
			{
				if (Own>0)
				{
					Try++;
					pFirstShard->y=768+(Own-1)*50-Random(50);
					pFirstShard->x=20+Random(900);
					dcShard *SH=pFirstShard->pNext;
					bNeedPlace=false;
					while ((SH)&&(!bNeedPlace))
					{
						if ((abs(SH->x-pFirstShard->x)<50)&&(abs(SH->y-pFirstShard->y)<50))
						{bNeedPlace=true;};
						SH=SH->pNext;
					};
					int AddY=(GetWorldSize(0)-1)*50;
					for (int i=1; ((i<MAX_PLAYERS)&&(!bNeedPlace)&&(Try<1000)); i++)
					{
						if ((Campaign.arCampPlayer[i].Y>AddY-50)&&(Campaign.arCampPlayer[i].bInGame)&&(Campaign.arCampPlayer[i].Y<AddY+100)
							&&(Campaign.arCampPlayer[i].X-60<pFirstShard->x)&&(Campaign.arCampPlayer[i].X+130>pFirstShard->x))
						{bNeedPlace=true;};
			
					};
				}
				else
				{
					pFirstShard->y=150+Random(610);
					pFirstShard->x=100+Random(780);
					dcShard *SH=pFirstShard->pNext;
					bNeedPlace=false;
					if ((pFirstShard->x>600)&&(pFirstShard->y<500))
					{bNeedPlace=true;};
					while ((SH)&&(!bNeedPlace))
					{
						if ((abs(SH->x-pFirstShard->x)<50)&&(abs(SH->y-pFirstShard->y)<50))
						{bNeedPlace=true;};
						SH=SH->pNext;
					};
					if ((pFirstShard->y>568-50)&&(pFirstShard->y<568+100)&&(pFirstShard->x>400-50)&&(pFirstShard->x<400+100))
					{bNeedPlace=true;};
				};
			};
		};
	};
	}
	else
	{	
		//Генерация осколков после получения всех зданий
		if (Build>=iMaxInnerBld-3)
		{
			Allow=1+Random(3);
			for (int All=0; All<Allow; All++)
			{
				pFirstShard=new dcShard(pFirstShard,Random(6),3+Random(2),8);
				if (Build<iMaxInnerBld)
				{
					Build=iMaxInnerBld;
					int Num=0;
					for (int i=1; i<=iMaxInnerBld; i++)
					{
						if ((GetBuildingStatus(i,Player)==1)&&(IsBuildingFree(i))&&(Num<4))
						{
							pFirstShard->arBuilding[Num]=i;
							Num++;						
						};			
					};
				};
				bool bNeedPlace=true;
				int Try=0;
				while (bNeedPlace)
				{				
					Try++;
					pFirstShard->y=768+(Own-1)*50-Random(50);
					pFirstShard->x=20+Random(900);
					dcShard *SH=pFirstShard->pNext;
					bNeedPlace=false;
					while ((SH)&&(!bNeedPlace))
					{
						if ((abs(SH->x-pFirstShard->x)<50)&&(abs(SH->y-pFirstShard->y)<50))
						{bNeedPlace=true;};
						SH=SH->pNext;
					};
					int AddY=(GetWorldSize(0)-1)*50;
					for (int i=1; ((i<MAX_PLAYERS)&&(!bNeedPlace)&&(Try<1000)); i++)
					{
						if ((Campaign.arCampPlayer[i].Y>AddY-50)&&(Campaign.arCampPlayer[i].bInGame)&&(Campaign.arCampPlayer[i].Y<AddY+100)
							&&(Campaign.arCampPlayer[i].X-60<pFirstShard->x)&&(Campaign.arCampPlayer[i].X+130>pFirstShard->x))
						{bNeedPlace=true;};			
					};
				};
			};
		};
	};
	dcShard *SH=pFirstShard;
	while (SH)
	{
		if (SH->iLevel>0)
		{
			SH->FillBonus();			
		};
		SH=SH->pNext;
	};
};

void NewCampaign()
{
	Campaign.Clear();
	int arX[3];
	arX[0]=2; arX[1]=-1; arX[2]=-1;
	for (int i=0; i<MAX_PLAYERS; i++)
	{
		Campaign.arCampPlayer[i].Clear();
		Campaign.arCampPlayer[i].iEnergy=100;
		/*Campaign.arCampPlayer[i].iEnergyBonus=0;
		Campaign.arCampPlayer[i].iEnergyPenalty=0;
		Campaign.arCampPlayer[i].iEnergyLose=0;*/
		Campaign.arCampPlayer[i].ipProfile=i;
		Campaign.arCampPlayer[i].iKarma=(pFirstPlayerProfile+i)->iKarma;
		Campaign.arCampPlayer[i].X=0;
		while (Campaign.arCampPlayer[i].X==0)
		{
			int NewX=Random(5);
			for (int k=0; k<3; k++)
			{
				if (NewX==arX[k])
				{NewX=-1;};
			};
			if (NewX>=0)
			{
				Campaign.arCampPlayer[i].X=NewX*200+20;
				arX[2]=arX[1];
				arX[1]=NewX;
			};
		};
		Campaign.arCampPlayer[i].Y=((pFirstPlayerProfile+i)->Y-1)*50;
		Campaign.arCampPlayer[i].iScore=0;
		for (int j=0; j<MAX_PLAYERS; j++)
		{Campaign.arCampPlayer[i].arAttitude[j]=(pFirstPlayerProfile+i)->arAtt[j];};
		Campaign.arCampPlayer[i].bFirstMeet=false;
		Campaign.arCampPlayer[i].bInGame=true;
	};
	Campaign.arCampPlayer[0].bFirstMeet=true;
	Campaign.arCampPlayer[0].X=512;
	Campaign.arCampPlayer[0].Y=50;
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if ((pFirstInnerBld+i)->iLevel<=0)
		{Campaign.arCampPlayer[0].arBld[i]=1;};
	};
	MakeShards(0);
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if ((pFirstInnerBld+i)->iLevel<0)
		{Campaign.arCampPlayer[0].arBld[i]=0;};
	};
	Tutorial.Init();
	CurGame.iMaxPlayers=2;
	CurGame.iGameType=2;	
	CurGame.iSeed=iSeed;
	arPlayer[1].Control=1;
	arPlayer[1].ipProfile=iActivePlayer;
	arPlayer[1].iColor=0;
	arPlayer[1].iDifficulty=0;
	arPlayer[2].Control=3;
	arPlayer[2].ipProfile=1;
	arPlayer[2].iDifficulty=0;
	clMap nMap;
	nMap.X=6; nMap.Y=6; nMap.Neutrals=1;
	CurGame.bDiplomacy=false;
	CurGame.bFog=true;
	GenerateMap(nMap, 0);
	bHourGlass=false;
	blit(bmpGUI_Screen01,screen1,0,0,0,0,1024,768);
	blit(bmpGUI_Screen01,screen,0,0,0,0,1024,768);
//	rectfill(screen,0,0,1024,768,COLOR_1);
//	rectfill(screen1,0,0,1024,768,COLOR_1);	
	Tutorial.Mode=arHumans[0].arMess[0];
	if (arHumans[0].iEnterData==0)
	{
		if (!WindowEnter(arHumans[0].sName,330,17))
		{sprintf(arHumans[0].sName,(pFirstPhrase+216)->str);};
		arHumans[0].iEnterData=1;
//		arHumans[0].iFileIndex=0;
		arHumans[0].iColor1=2;
		arHumans[0].iColor2=4;
		arHumans[0].iSign=7;
		arHumans[0].ipAvatar=17;
		arHumans[0].Save();
		iActivePlayer=arHumans[0].iFileIndex;
		PACKFILE *WorkFile;
		WorkFile=pack_fopen("save\\game.con","w");
		pack_fwrite(&iActivePlayer,sizeof iActivePlayer,WorkFile);
		pack_fclose(WorkFile);
		arPlayer[1].MakeFlags();
	};
	char Txt[500];
	sprintf(Txt,(pFirstCampDialog+1)->sText,(pFirstProvince+26)->sName);	
	int Class=(pFirstCampDialog+1)->Show(1,1,1,1,NULL,Txt);
	arPlayer[1].arHero[0]=new dcHero(1,Class);
	arPlayer[1].arHero[0]->iHealth+=4;
	arPlayer[1].arHero[0]->pSquad[0]->iLife=arPlayer[1].arHero[0]->pSquad[0]->GetLife();
	for (int i=1; i<MAX_SQUAD; i++)
	{
		if (arPlayer[1].arHero[0]->pSquad[i])
		{
			delete arPlayer[1].arHero[0]->pSquad[i];
			arPlayer[1].arHero[0]->pSquad[i]=NULL;
		};
	};
	for (int i=0; i<16; i++)
	{arPlayer[1].arHero[0]->arInventory[i][0]=0;};
	for (int i=0; i<SPELLBOOK_SIZE; i++)
	{arPlayer[1].arHero[0]->arSpell[i]=0;};

	arPlayer[1].Gold=0;
	arPlayer[1].Gem=0;

	arPlayer[1].arHero[0]->Pic=bmpAvatar[17];
	arPlayer[1].arHero[0]->smPic=bmpSmallAvatar[17];	
	arPlayer[1].arHero[0]->ipProvince=26;
	arPlayer[1].arHero[0]->pProvince=(pFirstProvince+26);
	switch (arPlayer[1].arHero[0]->ipClass1)
	{
	case 1:
		arPlayer[1].arHero[0]->apSkill[0][0]=8;
		break;
	case 2:
		arPlayer[1].arHero[0]->apSkill[0][0]=22;
		break;
	case 3:
		arPlayer[1].arHero[0]->apSkill[0][0]=2;
		break;
	case 4:
		arPlayer[1].arHero[0]->apSkill[0][0]=15;
		break;
	};
	arPlayer[1].Shop.Items[0][0]=1;
	arPlayer[1].Shop.Items[1][0]=5;
	arPlayer[1].Shop.Items[2][0]=9;
	arPlayer[1].Shop.Items[3][0]=13;
	arPlayer[1].Shop.Items[4][0]=17;
	arPlayer[1].Shop.Items[5][0]=29;
	arPlayer[1].Shop.Items[6][0]=37;
	arPlayer[1].Shop.Items[7][0]=41;
	arPlayer[1].Shop.Items[8][0]=42;
	arPlayer[1].Shop.Items[9][0]=46;
	arPlayer[1].Shop.Items[10][0]=51;
	arPlayer[1].Shop.Items[11][0]=55;
	arPlayer[1].Shop.Items[12][0]=63;
	arPlayer[1].Shop.Items[13][0]=59;

	arPlayer[1].arRecruit[0]=1;
	arPlayer[1].arRecruit[1]=2;

	arPlayer[1].arLibrary[0]=4;
	arPlayer[1].arLibrary[1]=12;
	arPlayer[1].arLibrary[2]=14;

	(pFirstProvince+13)->PlaceDefender(1);
	(pFirstProvince+14)->PlaceDefender(1);
	(pFirstProvince+8)->PlaceDefender(1);

	stString *STR=new stString;
	sprintf(STR->str,arPlayer[1].GetName());
	arPlayer[1].arHero[0]->pstName=STR;	
	(pFirstCampDialog+2)->Show(1,0,0,0,NULL,NULL);
	sprintf(Txt,(pFirstCampDialog+3)->sText,(pFirstProvince+27)->sName);
	int Answer;
	if (arHumans[0].bFinishTutorial)
	{Answer=(pFirstCampDialog+3)->Show(1,1,1,1,NULL,Txt);}
	else
	{Answer=(pFirstCampDialog+3)->Show(1,1,1,0,NULL,Txt);};
	if (Answer==4)
	{
		RunCampaignEvent(119,0,0);
		if (Campaign.arDialog[124]>0)
		{
			CurGame.iCurrentTurn=-1;
			UnloadGame();
		};
	}
	else
	{
		if (Answer==1)
		{
			Answer=(pFirstCampDialog+4)->Show(1,1,1,0,NULL,NULL);
		};
		if (Answer!=3)
		{
			Answer=(pFirstCampDialog+5)->Show(1,1,1,0,NULL,NULL);
			if (Answer==1)
			{
				(pFirstCampDialog+6)->Show(1,0,0,0,NULL,NULL);
			};
		};
	};

//	CurGame.iCurrentTurn=-1;
};

int GetRulerDiff()
{
	int Result=arCampDiff[Campaign.iDifficulty][0]+Campaign.iCurrentTurn/arCampDiff[Campaign.iDifficulty][2];
	if (Result>arCampDiff[Campaign.iDifficulty][1])
	{Result=arCampDiff[Campaign.iDifficulty][1];};
	return Result+1;
};

int GetLocalDiff(int Level)
{
	return arCampDiff[Campaign.iDifficulty][0]+Level/(9/(arCampDiff[Campaign.iDifficulty][1]-arCampDiff[Campaign.iDifficulty][0]));		
};

void MakeLocalLord(int Player)
{
	arPlayer[Player].Control=3;
	arPlayer[Player].ipProfile=16+Player;
//	arPlayer[Player].iDifficulty=arPlayer[1].iDifficulty;
	arPlayer[Player].iColor=Player-2;
	Campaign.arCampPlayer[16+Player].Clear();
	Campaign.arCampPlayer[16+Player].ipProfile=16+Player;
	/*for (int i=1; i<=iMaxInnerBld; i++)
	{
		if ((pFirstInnerBld+i)->iLevel>0)
		{Campaign.arCampPlayer[16+Player].arBld[i]=0;}
		else
		{Campaign.arCampPlayer[16+Player].arBld[i]=1;};
	};*/
	int BB=Campaign.iCurrentTurn/2+Campaign.iCurrentTurn/2*Campaign.iDifficulty/4+Random(2+Campaign.iDifficulty);
	while (BB>0)
	{
		Campaign.arCampPlayer[16+Player].AddBuild();
		/*ClearRoll();
		int Count=0;
		for (int i=1; i<=iMaxInnerBld; i++)
		{
			if (IsBuildingAllowed(i,16+Player)==1)
			{
				RollerN[Count]=i;
				RollerP[Count]=10;
				Count++;
			};
		};
		if (Count>0)
		{
			Campaign.arCampPlayer[16+Player].arBld[Roll(false)]=1;
		};*/
		BB--;
	};
};

void AttackPlayer(int Attacker, int Defender)
{	
	clMap Mapz;	
	Mapz.SetDefault();
	Mapz.Difficulty=Campaign.iDifficulty;
	Mapz.Players=2;	
	Mapz.X=8;
	Mapz.Y=8;
	CurGame.bDiplomacy=false;

	if (Attacker==0)
	{
		Campaign.bDefender=false;
		Campaign.iActiveShard=-Defender;
		CurGame.iSeed=Defender*100+GetWorldSize(Defender);
		arPlayer[2].iDifficulty=GetRulerDiff()+1;
		//arPlayer[2].iDifficulty=Campaign.iDifficulty+2+GetWorldSize(Defender)/(20-Campaign.iDifficulty*2);
		arPlayer[2].ipProfile=Defender;
		Campaign.arCampPlayer[Defender].arAttitude[0]-=40;
		if ((Defender==3)&&(Campaign.arDialog[103]>0))
		{
			Campaign.arDialog[103]=0;
			Campaign.arCampPlayer[3].iTreaty=0;
			Campaign.arCampPlayer[Defender].arAttitude[0]-=80;
		};
		Mapz.Neutrals=1+GetWorldSize(Defender)/2;
		CurGame.bFog=true;
	}
	else
	{
		Campaign.bDefender=true;
		Campaign.iActiveShard=-Attacker;
		CurGame.iSeed=pFirstShard->iSeed+GetWorldSize(Defender);
		//arPlayer[2].iDifficulty=Campaign.iDifficulty+Campaign.iCurrentTurn/(20-Campaign.iDifficulty*2);
		arPlayer[2].iDifficulty=GetRulerDiff()+1;
		arPlayer[2].ipProfile=Attacker;
		Mapz.Neutrals=1+GetWorldSize(Defender)/2;
		CurGame.bFog=false;
	};
	CurGame.iMaxPlayers=2;
	CurGame.iGameType=2;	
	arPlayer[1].Control=1;
	arPlayer[1].ipProfile=iActivePlayer;
	arPlayer[1].iColor=0;
	arPlayer[1].iDifficulty=Campaign.iDifficulty;
	arPlayer[2].Control=2;
	CurGame.arPlayerSet[2]=arPlayer[2].ipProfile;
	if (arPlayer[2].iDifficulty<3)
	{arPlayer[2].iDifficulty=3;};
	if (arPlayer[2].iDifficulty>6)
	{arPlayer[2].iDifficulty=6;};
	GenerateMap(Mapz, 0);
	arPlayer[1].iKarma=Campaign.arCampPlayer[0].iKarma;
	if (Campaign.arDialog[40]>0)
	{arPlayer[1].iUnion=5;};
	//Получение зданий не воюющими властителями
	for (int i=1; i<MAX_PLAYERS; i++)
	{
		if ((Campaign.arCampPlayer[i].bInGame)&&(Campaign.arCampPlayer[i].bFirstMeet)&&(i!=abs(Attacker))&&(i!=abs(Defender)))
		{
			if (Random(2+Campaign.iDifficulty)>0)
			{Campaign.arCampPlayer[i].AddBuild();};
		};
	};
};

void AttackShard(dcShard *SH)
{
//	int Result=0;
	//Таймеры квестовых событий	
	if (Campaign.arDialog[50]>0)
	{
		if (Campaign.arDialog[50]==1)
		{
			(pFirstQuest+32)->iGiver=-GetWorldSize(0);
			Campaign.arDialog[50]=0;
		}
		else
		{Campaign.arDialog[50]--;}
	};
	if (Campaign.arDialog[32]>0)
	{
		if (Campaign.arDialog[32]==1)
		{
			(pFirstQuest+33)->iGiver=-GetWorldSize(0);
			Campaign.arDialog[32]=0;
		}
		else
		{Campaign.arDialog[32]--;}
	};
	if (Campaign.arDialog[33]>0)
	{
		if (Campaign.arDialog[33]==1)
		{
			(pFirstQuest+34)->iGiver=-GetWorldSize(0);
			Campaign.arDialog[33]=0;
		}
		else
		{Campaign.arDialog[33]--;}
	};
	if (Campaign.arDialog[92]>1)
	{Campaign.arDialog[92]--;};
//Инициализация квестов
	for (int Q=1; Q<=iMaxQuest; Q++)
	{
		if (((pFirstQuest+Q)->iGiver<0)&&(abs((pFirstQuest+Q)->iGiver)<=GetWorldSize(0)))
		{
			int iPlace=-1;
			for (int i=0; i<MAX_CAMP_QUEST; i++)
			{
				if (Campaign.arQuest[i][0]==0)
				{
					iPlace=i;
					break;
				};
				if (Campaign.arQuest[i][0]==Q)
				{
					iPlace=i;
					break;
				};
			};
			if ((iPlace>=0)&&(Campaign.arQuest[iPlace][0]==0))
			{
				Campaign.arQuest[iPlace][0]=Q;
				Campaign.arQuest[iPlace][1]=0;
				if ((pFirstQuest+Q)->ipEncInit>0)
				{RunCampaignEvent((pFirstQuest+Q)->ipEncInit,0,0);};
			};
		};
	};
	
	CurGame.iMaxPlayers=SH->Map.Players;
	CurGame.iGameType=2;	
	CurGame.iSeed=SH->iSeed;
	arPlayer[1].Control=1;
	arPlayer[1].ipProfile=iActivePlayer;
	arPlayer[1].iColor=0;
	arPlayer[1].iDifficulty=Campaign.iDifficulty;
	//arPlayer[1].iDifficulty=(SH->iLevel+arHumans[0].iDifficulty-2)/2;
	//if (arPlayer[1].iDifficulty<0)
	//{arPlayer[1].iDifficulty=0;};
//	if (arPlayer[1].iDifficulty>3)
//	{arPlayer[1].iDifficulty=3;};
	ClearRoll();
	int Count=0;
	for (int i=1; i<MAX_PLAYERS; i++)
	{
		if ((Campaign.arCampPlayer[i].bFirstMeet)&&(Campaign.arCampPlayer[i].bInGame)&&((i!=10)||(Campaign.arDialog[670]==0)))
		{
			RollerN[Count]=i;
			RollerP[Count]=10;
			Count++;
		};
	};
	for (int i=2; i<=SH->Map.Players; i++)
	{
		CurGame.arPlayerSet[i]=0;
		if ((Count>0)&&(((SH->Map.Players>2)&&(i==2))||(Random(100)<50+Count*5)))
		{
			arPlayer[i].Control=2;
			arPlayer[i].ipProfile=Roll(true);			
		//	arPlayer[i].iDifficulty=1+Campaign.iCurrentTurn/(16-Campaign.iDifficulty*2);
			arPlayer[i].iDifficulty=GetRulerDiff();
			if (arPlayer[i].iDifficulty<1)
			{arPlayer[i].iDifficulty=1;};
			if (arPlayer[i].iDifficulty>6)
			{arPlayer[i].iDifficulty=6;};
			Count--;
		}
		else
		{
			arPlayer[i].Control=3;			
		};
	};

	int arBLD[16], NeedBld=0;
//Получение зданий не воюющими властителями
	for (int i=0; i<Count+SH->Map.Players; i++)
	{
		if ((RollerN[i]>0)&&(RollerP[i]>0))
		{
			if (Random(2+Campaign.iDifficulty)>0)
			{
				arBLD[NeedBld]=RollerN[i];
				NeedBld++;
			};
		};
	};
	for (int i=0; i<NeedBld; i++)
	{Campaign.arCampPlayer[arBLD[i]].AddBuild();};

//Создание местных лордов
	for (int i=2; i<=SH->Map.Players; i++)
	{
		if (arPlayer[i].Control==3)
		{
			MakeLocalLord(i);
			arPlayer[i].iDifficulty=GetLocalDiff(SH->iLevel-1);
		//	arPlayer[i].iDifficulty=(arPlayer[1].iDifficulty+1)*(SH->iLevel-1)/5;
			if (arPlayer[i].iDifficulty<0)
			{arPlayer[i].iDifficulty=0;};
			if (arPlayer[i].iDifficulty>5)
			{arPlayer[i].iDifficulty=5;};
		};
	};
//	SH->Map.Neutrals=NeutralPower;
	CurGame.bDiplomacy=true;
	CurGame.bFog=true;
	Campaign.iActiveShard=SH->Num;	
	GenerateMap(SH->Map,SH->arEvent[0]);
	if (Campaign.arDialog[40]>0)
	{arPlayer[1].iUnion=5;};
	arPlayer[1].iKarma=Campaign.arCampPlayer[0].iKarma;
	for (int i=2; i<=CurGame.iMaxPlayers; i++)
	{
		if (arPlayer[i].Control==2)
		{
			arPlayer[i].Politic[1].Attitude=Campaign.arCampPlayer[arPlayer[i].ipProfile].arAttitude[0];
		};
	};	
//	Result=RunGame(true);	
//	return Result;
};

int GetUnitEnergy(int ipUnit)
{
	int	Result=((pFirstUnitType+ipUnit)->iGoldPrice+(pFirstUnitType+ipUnit)->iGemPrice*5/2)*2;
	for (int i=0; i<MAX_UNIT_RES; i++)
	{
		if ((pFirstUnitType+ipUnit)->arRes[i])
		{
			Result+=((pFirstResource+(pFirstUnitType+ipUnit)->arRes[i])->iGold+(pFirstResource+(pFirstUnitType+ipUnit)->arRes[i])->iGem*5/2)/2;
		};
	};
	if (ipUnit>12)
	{Result+=Result/2;};
	return Result;
};

int GetItemEnergy(int ipItem)
{
	int	Result=((pFirstItem+ipItem)->iGoldCost+(pFirstItem+ipItem)->iGemCost*5/2)*2+((pFirstItem+ipItem)->iShopLevel-1)*10;
	for (int i=0; i<MAX_ITEM_RES; i++)
	{
		if ((pFirstItem+ipItem)->arRes[i])
		{
			Result+=((pFirstResource+(pFirstItem+ipItem)->arRes[i])->iGold+(pFirstResource+(pFirstItem+ipItem)->arRes[i])->iGem*5/2)/3;
		};
	};	
	return Result;
};

int GetBuildEnergy(int ipBuild)
{
	int	Result=((pFirstInnerBld+ipBuild)->iGoldCost+(pFirstInnerBld+ipBuild)->iGemCost*5/2)*2;
	for (int i=0; i<MAX_BUILDING_RES; i++)
	{
		if ((pFirstInnerBld+ipBuild)->arRes[i])
		{
			Result+=((pFirstResource+(pFirstInnerBld+ipBuild)->arRes[i])->iGold+(pFirstResource+(pFirstInnerBld+ipBuild)->arRes[i])->iGem*5/2);
		};
	};	
	return Result;
};

int GetDefenderEnergy(int ipDefender)
{
	int	Result=(pFirstDefender+ipDefender)->iGoldCost+(pFirstDefender+ipDefender)->iGemCost*5/2+10;
	for (int i=0; i<MAX_DEFENDER_RES; i++)
	{
		if ((pFirstDefender+ipDefender)->arRes[i])
		{
			Result+=((pFirstResource+(pFirstDefender+ipDefender)->arRes[i])->iGold+(pFirstResource+(pFirstDefender+ipDefender)->arRes[i])->iGem*5/2)/3;
		};
	};	
	return Result;
};

int GetResourceEnergy(int ipResource)
{
//	int	Result=((pFirstResource+ipResource)->iGold+(pFirstResource+ipResource)->iGem*5/2);
	int	Result=20+(pFirstResource+ipResource)->iRarity*5;
	return Result;
};


//******************************************************************
//******** Экран выбора бонусов для вторжения **********************
//******************************************************************

bool PickBonusScreen(dcShard *Shard, int Mode)
{	
	iAntisleep=iAntisleepDelay;
	bool Result=true;
	CurrentPlayer=0;
	const int MAX=300;
	BITMAP *bmpScr=create_bitmap(1024,768);	
	draw_sprite(bmpScr,bmpGUI_PickBonus,0,0);
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpScr);	
	UI_Button1 buttExit;
	objGUI=buttExit.Init(934,699,1,2,bmpButton[309],bmpButton[310],bmpButton[311],bmpButton[311],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;
	UI_Button1 buttAttack;
	objGUI=buttAttack.Init(854,699,1,2,bmpButton[36],bmpButton[37],bmpButton[254],bmpButton[286],objGUI);
	buttAttack.Activate(1);
//	buttAttack.ipHotkey=KEY_ESC;
	buttAttack.iHint=200;

	int MaxRes[4], MaxUnit=0, MaxItem=0, MaxBuild=0, MaxStuff=0;	
	int arRes[4], arUnit[MAX][2], arItem[MAX][2], arBuild[MAX][2], arStuff[MAX][3];
	int EnergySpent=0;
	UI_Input Inp[4];
	UI_Button1 buttRes[4][2];
	UI_Mask mskUnit[13], mskItem[13], mskBuild[13], mskStuff[13];
	UI_Mask1 mskRes[4];	
	UI_ScrollBar scrRes[4];	
	for (int i=0; i<4; i++)
	{
		MaxRes[i]=0;
		arRes[i]=0;
		objGUI=Inp[i].Init(140+i*247,85,1,6,2,COLOR_2,Font1,objGUI);		
		Inp[i].bCenter=true;
		objGUI=buttRes[i][0].Init(105+i*247,83,0,-2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
		objGUI=buttRes[i][1].Init(216+i*247,83,1,-2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
		mskRes[i].Init(127+i*247,85,90,22,1);
		objGUI=scrRes[i].Init(124+i*247,107,80,7,2,false,true,objGUI);
	//	scrRes[i].buttAdd
	};
	UI_Button1 buttUnitL, buttUnitR;
	objGUI=buttUnitL.Init(33,231,-1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	objGUI=buttUnitR.Init(970,231,-1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);

	UI_Button1 buttItemL, buttItemR;
	objGUI=buttItemL.Init(33,370,-1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	objGUI=buttItemR.Init(970,370,-1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);

	UI_Button1 buttBuildL, buttBuildR;
	objGUI=buttBuildL.Init(33,500,-1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	objGUI=buttBuildR.Init(970,500,-1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);

	UI_Button1 buttStuffL, buttStuffR;
	objGUI=buttStuffL.Init(33,630,-1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	objGUI=buttStuffR.Init(970,630,-1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);

	for (int i=0; i<13; i++)
	{
		objGUI=mskUnit[i].Init(51+i*70,188,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,objGUI,2);
		objGUI=mskItem[i].Init(59+i*70,342,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);
		objGUI=mskBuild[i].Init(59+i*70,472,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);
		objGUI=mskStuff[i].Init(59+i*70,602,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);
	};

	objGUI=new G_Object(objGUI,20,20,-1,bmpGUI_Hint);
	G_Object *objTutorial=objGUI;
	UI_Mask mskTutorial;
	objGUI=mskTutorial.Init(20,20,1,bmpGUI_Hint,bmpEmpty,objGUI,-1);
	UI_TextBox txtTutorial;
	objGUI=txtTutorial.Init(40,50,297,200,4,COLOR_1,Font2,objGUI);
	txtTutorial.bShadow=false;
	txtTutorial.Hide();
	bool bTutHint=false;
	if ((Tutorial.Mode)&&(Tutorial.Bonus==1))
	{
	//	Tutorial.Bonus=1;
		txtTutorial.Activate((pFirstTutText+87)->msg);
		objTutorial->iLayer=3;
		bTutHint=true;
		ScrollSound();
	};

	dcShard *SH=pFirstShard;
	while (SH)
	{
		if (SH->ipPlayer==0)
		{
			for (int i=0; i<MAX_SHARD_BONUS; i++)
			{
				switch (SH->arBonus[i][0])
				{
				case 0:
					break;
				case 1:
				case 2:
				case 3:
				case 4:
					MaxRes[SH->arBonus[i][0]-1]+=SH->arBonus[i][1];
					break;
				case 5:
					arItem[MaxItem][0]=SH->arBonus[i][1];
					arItem[MaxItem][1]=0;
					MaxItem++;
					break;
				case 6:
					arUnit[MaxUnit][0]=SH->arBonus[i][1];
					arUnit[MaxUnit][1]=0;
					MaxUnit++;
					break;
				case 7:
					arBuild[MaxBuild][0]=SH->arBonus[i][1];
					arBuild[MaxBuild][1]=0;
					MaxBuild++;
					break;
				default:					
					arStuff[MaxStuff][0]=SH->arBonus[i][1];
					arStuff[MaxStuff][1]=0;
					arStuff[MaxStuff][2]=SH->arBonus[i][0];
					MaxStuff++;
					break;
				};
			};
		};
		SH=SH->pNext;		
	};

//Упорядочиваем списки
	int U; bool bChange;
	for (int i=0; i<MaxUnit-1; i++)
	{
		for (int j=i; j<MaxUnit; j++)
		{
			bChange=false;
			if (GetUnitEnergy(arUnit[i][0])>GetUnitEnergy(arUnit[j][0]))
			{bChange=true;};
			if ((GetUnitEnergy(arUnit[i][0])==GetUnitEnergy(arUnit[j][0]))&&(arUnit[i][0]>arUnit[j][0]))
			{bChange=true;};
			if (bChange)
			{
				U=arUnit[i][0];
				arUnit[i][0]=arUnit[j][0];
				arUnit[j][0]=U;
			};
		};
	};

	for (int i=0; i<MaxItem-1; i++)
	{
		for (int j=i; j<MaxItem; j++)
		{
			bChange=false;
			if (GetItemEnergy(arItem[i][0])>GetItemEnergy(arItem[j][0]))
			{bChange=true;};
			if ((GetItemEnergy(arItem[i][0])==GetItemEnergy(arItem[j][0]))&&(arItem[i][0]>arItem[j][0]))
			{bChange=true;};
			if (bChange)
			{
				U=arItem[i][0];
				arItem[i][0]=arItem[j][0];
				arItem[j][0]=U;
			};
		};
	};

	for (int i=0; i<MaxBuild-1; i++)
	{
		for (int j=i; j<MaxBuild; j++)
		{
			bChange=false;
			if (GetBuildEnergy(arBuild[i][0])>GetBuildEnergy(arBuild[j][0]))
			{bChange=true;};
			if ((GetBuildEnergy(arBuild[i][0])==GetBuildEnergy(arBuild[j][0]))&&(arBuild[i][0]>arBuild[j][0]))
			{bChange=true;};
			if (bChange)
			{
				U=arBuild[i][0];
				arBuild[i][0]=arBuild[j][0];
				arBuild[j][0]=U;
			};
		};
	};

	for (int i=0; i<MaxStuff-1; i++)
	{
		for (int j=i; j<MaxStuff; j++)
		{
			bChange=false;
			if (arStuff[i][2]>arStuff[j][2])
			{bChange=true;};
			if (arStuff[i][2]==arStuff[j][2])
			{
				switch (arStuff[i][2])
				{
				case 8:
					if (arStuff[i][0]>arStuff[j][0])
					{bChange=true;};
					break;
				case 9:
					if (GetDefenderEnergy(arStuff[i][0])>GetDefenderEnergy(arStuff[j][0]))
					{bChange=true;};
					break;
				};
			};
			if (bChange)
			{
				U=arStuff[i][0];
				arStuff[i][0]=arStuff[j][0];
				arStuff[j][0]=U;
				U=arStuff[i][2];
				arStuff[i][2]=arStuff[j][2];
				arStuff[j][2]=U;
			};
		};
	};

	if (Mode==0)
	{textprintf_centre_ex(bmpScr,Font1,512,20,COLOR_1,-1,"%s %s",(pFirstPhrase+340)->str,Shard->sName);}
	else
	{textprintf_centre_ex(bmpScr,Font1,512,20,COLOR_1,-1,(pFirstPhrase+440)->str,(pFirstPlayerProfile+Mode)->sName);};
	textprintf_centre_ex(bmpScr,Font1,512,173,COLOR_1,-1,(pFirstShardBonus+6)->sName);
	textprintf_centre_ex(bmpScr,Font1,512,318,COLOR_1,-1,(pFirstShardBonus+5)->sName);
	textprintf_centre_ex(bmpScr,Font1,512,449,COLOR_1,-1,(pFirstShardBonus+7)->sName);
	textprintf_centre_ex(bmpScr,Font1,512,578,COLOR_1,-1,(pFirstPhrase+341)->str);
	textprintf_centre_ex(bmpScr,Font1,223,706,COLOR_1,-1,(pFirstPhrase+342)->str);
	textprintf_centre_ex(bmpScr,Font1,642,706,COLOR_1,-1,(pFirstPhrase+343)->str);
	textprintf_centre_ex(bmpScr,Font1,223,733,COLOR_2,-1,"%d",Campaign.arCampPlayer[0].iEnergy);
	textprintf_centre_ex(bmpScr,Font1,642,733,COLOR_2,-1,"%d",EnergySpent);


	for (int i=0; i<4; i++)
	{
		textprintf_centre_ex(bmpScr,Font1,142+i*247,64,COLOR_1,-1,(pFirstShardBonus+i+1)->sName);
		if (arRes[i]>0)
		{buttRes[i][0].Activate(1);}
		else
		{buttRes[i][0].Activate(0);};
		if (arRes[i]<MaxRes[i])
		{buttRes[i][1].Activate(1);}
		else
		{buttRes[i][1].Activate(0);};
		Inp[i].EnterInt(arRes[i]);
		Inp[i].Activate(0);
		Inp[i].Show();
		scrRes[i].AttachValue(&arRes[i],0,MaxRes[i]);
		scrRes[i].Show();
		textprintf_centre_ex(bmpScr,Font1,173+i*247,126,COLOR_2,-1,"%d",arRes[i]*(pFirstShardBonus+1+i)->iPrice);
	};


	bool bRepeat=true, bChangePrice=false, bUnitShow=true, bItemShow=true, bBuildShow=true, bStuffShow=true, bAttack=true; 
	int iClick=-2, UnitShift=0, UnitMask=0, ItemShift=0, ItemMask=0, BuildShift=0, BuildMask=0, StuffShift=0, StuffMask=0, Check;
	if (Mode<0)
	{bRepeat=false;};
	
	while (bRepeat)
	{
		if (bTutHint)
		{
			if (Tutorial.Mode)
			{Check=mskTutorial.Check();}
			else
			{Check=30;};
			if ((Check==30)||(Check==40))
			{
				if (Tutorial.Bonus==1)
				{
					Tutorial.Bonus=2;
					txtTutorial.Activate((pFirstTutText+88)->msg);					
				}
				else
				{
					objTutorial->iLayer=-1;
					txtTutorial.Hide();
					bTutHint=false;
				};
			};
		};
//Полоска юнитов	
		if (bUnitShow)
		{
			blit(bmpGUI_PickBonus,bmpScr,54,191,54,191,915,110);
			UnitMask=0;
			for (int i=UnitShift*13; i<MaxUnit; i++)
			{
				if (i<(UnitShift+1)*13)
				{
					draw_sprite(bmpScr,bmpGUI_PickUnit,57+(i-UnitShift*13)*70,194);
					draw_sprite(bmpScr,(pFirstUnitType+arUnit[i][0])->Icon,59+(i-UnitShift*13)*70,196);
					if (arUnit[i][1]>0)
					{
						draw_sprite(bmpScr,bmpGUI_Built,98+(i-UnitShift*13)*70,196);
						textprintf_centre_ex(bmpScr,Font1,100+(i-UnitShift*13)*70,285,COLOR_2,-1,"%d",GetUnitEnergy(arUnit[i][0]));
					}
					else
					{textprintf_centre_ex(bmpScr,Font1,100+(i-UnitShift*13)*70,285,makecol(130,130,130),-1,"%d",GetUnitEnergy(arUnit[i][0]));};
					UnitMask++;
				};				
			};
			if (MaxUnit>13)
			{
				if (UnitShift>0)
				{buttUnitL.Activate(1);}
				else
				{buttUnitL.Activate(0);};
				if ((UnitShift+1)*13<=MaxUnit)
				{buttUnitR.Activate(1);}
				else
				{buttUnitR.Activate(0);};
			};

			bUnitShow=false;
		};

		if (buttUnitL.Check())
		{
			UnitShift--;
			bUnitShow=true;
		};
		if (buttUnitR.Check())
		{
			UnitShift++;
			bUnitShow=true;
		};

//Полоска предметов
		if (bItemShow)
		{
			blit(bmpGUI_PickBonus,bmpScr,54,338,54,338,915,95);
			ItemMask=0;
			for (int i=ItemShift*13; i<MaxItem; i++)
			{
				if (i<(ItemShift+1)*13)
				{
					draw_sprite(bmpScr,bmpGUI_PickSpell,57+(i-ItemShift*13)*70,340);
					draw_sprite(bmpScr,(pFirstItem+arItem[i][0])->Pic,65+(i-ItemShift*13)*70,348);
					if (arItem[i][1]>0)
					{
						draw_sprite(bmpScr,bmpGUI_Built,95+(i-ItemShift*13)*70,352);
						textprintf_centre_ex(bmpScr,Font1,102+(i-ItemShift*13)*70,415,COLOR_2,-1,"%d",GetItemEnergy(arItem[i][0]));
					}
					else
					{textprintf_centre_ex(bmpScr,Font1,102+(i-ItemShift*13)*70,415,makecol(130,130,130),-1,"%d",GetItemEnergy(arItem[i][0]));};
					ItemMask++;
				};				
			};
			if (MaxItem>13)
			{
				if (ItemShift>0)
				{buttItemL.Activate(1);}
				else
				{buttItemL.Activate(0);};
				if ((ItemShift+1)*13<=MaxItem)
				{buttItemR.Activate(1);}
				else
				{buttItemR.Activate(0);};
			};

			bItemShow=false;
		};

		if (buttItemL.Check())
		{
			ItemShift--;
			bItemShow=true;
		};
		if (buttItemR.Check())
		{
			ItemShift++;
			bItemShow=true;
		};

//Полоска зданий
		if (bBuildShow)
		{
			blit(bmpGUI_PickBonus,bmpScr,54,468,54,468,915,95);
			BuildMask=0;
			for (int i=BuildShift*13; i<MaxBuild; i++)
			{
				if (i<(BuildShift+1)*13)
				{
					draw_sprite(bmpScr,bmpGUI_PickSpell,57+(i-BuildShift*13)*70,470);
					draw_sprite(bmpScr,(pFirstInnerBld+arBuild[i][0])->Pic,65+(i-BuildShift*13)*70,478);
					if (arBuild[i][1]>0)
					{
						draw_sprite(bmpScr,bmpGUI_Built,95+(i-BuildShift*13)*70,482);
						textprintf_centre_ex(bmpScr,Font1,102+(i-BuildShift*13)*70,545,COLOR_2,-1,"%d",GetBuildEnergy(arBuild[i][0]));
					}
					else
					{textprintf_centre_ex(bmpScr,Font1,102+(i-BuildShift*13)*70,545,makecol(130,130,130),-1,"%d",GetBuildEnergy(arBuild[i][0]));};
					BuildMask++;
				};				
			};
			if (MaxBuild>13)
			{
				if (BuildShift>0)
				{buttBuildL.Activate(1);}
				else
				{buttBuildL.Activate(0);};
				if ((BuildShift+1)*13<=MaxBuild)
				{buttBuildR.Activate(1);}
				else
				{buttBuildR.Activate(0);};
			};

			bBuildShow=false;
		};

		if (buttBuildL.Check())
		{
			BuildShift--;
			bBuildShow=true;
		};
		if (buttBuildR.Check())
		{
			BuildShift++;
			bBuildShow=true;
		};

//Полоска спец бонусов
		if (bStuffShow)
		{
			blit(bmpGUI_PickBonus,bmpScr,54,598,54,598,915,95);
			StuffMask=0;
			for (int i=StuffShift*13; i<MaxStuff; i++)
			{
				if (i<(StuffShift+1)*13)
				{
					draw_sprite(bmpScr,bmpGUI_PickSpell,57+(i-StuffShift*13)*70,600);
					switch (arStuff[i][2])
					{
					case 8:
						draw_sprite(bmpScr,(pFirstResource+arStuff[i][0])->BigPic,65+(i-StuffShift*13)*70,608);
						if (arStuff[i][1]>0)
						{
							draw_sprite(bmpScr,bmpGUI_Built,95+(i-StuffShift*13)*70,612);
							textprintf_centre_ex(bmpScr,Font1,102+(i-StuffShift*13)*70,675,COLOR_2,-1,"%d",GetResourceEnergy(arStuff[i][0]));
						}
						else
						{textprintf_centre_ex(bmpScr,Font1,102+(i-StuffShift*13)*70,675,makecol(130,130,130),-1,"%d",GetResourceEnergy(arStuff[i][0]));};						
						break;
					case 9:
						draw_sprite(bmpScr,(pFirstDefender+arStuff[i][0])->Pic,65+(i-StuffShift*13)*70,608);
						if (arStuff[i][1]>0)
						{
							draw_sprite(bmpScr,bmpGUI_Built,95+(i-StuffShift*13)*70,612);
							textprintf_centre_ex(bmpScr,Font1,102+(i-StuffShift*13)*70,675,COLOR_2,-1,"%d",GetDefenderEnergy(arStuff[i][0]));
						}
						else
						{textprintf_centre_ex(bmpScr,Font1,102+(i-StuffShift*13)*70,675,makecol(130,130,130),-1,"%d",GetDefenderEnergy(arStuff[i][0]));};						
						break;
					};					
					
					StuffMask++;
				};				
			};
			if (MaxStuff>13)
			{
				if (StuffShift>0)
				{buttStuffL.Activate(1);}
				else
				{buttStuffL.Activate(0);};
				if ((StuffShift+1)*13<=MaxStuff)
				{buttStuffR.Activate(1);}
				else
				{buttStuffR.Activate(0);};
			};

			bStuffShow=false;
		};

		if (buttStuffL.Check())
		{
			StuffShift--;
			bStuffShow=true;
		};
		if (buttStuffR.Check())
		{
			StuffShift++;
			bStuffShow=true;
		};

//Маски
		for (int i=0; i<UnitMask; i++)
		{
			Check=mskUnit[i].Check();
			if (Check==30)
			{
				int Picked=0;
				if (arUnit[i+UnitShift*13][1]==0)
				{
					for (int j=0; j<MaxUnit; j++)
					{
						if (arUnit[j][1]>0)
						{Picked++;};
					};					
				};
				if (Picked<6)
				{				
					arUnit[i+UnitShift*13][1]=1-arUnit[i+UnitShift*13][1];
					bChangePrice=true;
					bUnitShow=true;
				}
				else
				{
					(pFirstDialog+1094)->Show(1,1,0,0,NULL,NULL);
				};
			};
			if (Check==40)
			{
				WindowUnitTypeInfo(arUnit[i+UnitShift*13][0]);
			};
			if (Check>1)
			{
				MakeStringHint((pFirstUnitType+arUnit[i+UnitShift*13][0])->sName);
				bHint=true;
			};
		};

		for (int i=0; i<ItemMask; i++)
		{
			Check=mskItem[i].Check();
			if (Check==30)
			{					
				arItem[i+ItemShift*13][1]=1-arItem[i+ItemShift*13][1];
				bChangePrice=true;
				bItemShow=true;				
			};
			if (Check==40)
			{
				WindowItemInfo(arItem[i+ItemShift*13][0],(pFirstItem+arItem[i+ItemShift*13][0])->iDurability);
			};
			if (Check>1)
			{
				MakeItemHint(arItem[i+ItemShift*13][0],(pFirstItem+arItem[i+ItemShift*13][0])->iDurability,NULL,0,0,0);
				bHint=true;
			};
		};

		for (int i=0; i<BuildMask; i++)
		{
			Check=mskBuild[i].Check();
			if (Check==30)
			{					
				arBuild[i+BuildShift*13][1]=1-arBuild[i+BuildShift*13][1];
				bChangePrice=true;
				bBuildShow=true;				
			};			
			if (Check>1)
			{
				MakeInnBldHint(arBuild[i+BuildShift*13][0],-1);
				bHint=true;
			};
		};

		for (int i=0; i<StuffMask; i++)
		{
			Check=mskStuff[i].Check();
			if (Check==30)
			{					
				arStuff[i+StuffShift*13][1]=1-arStuff[i+StuffShift*13][1];
				bChangePrice=true;
				bStuffShow=true;				
			};
			if (Check==40)
			{
				switch (arStuff[i+StuffShift*13][2])
				{
				case 8:
					WindowResourceInfo(arStuff[i+StuffShift*13][0]);
					break;
				case 9:
					WindowDefenderInfo(arStuff[i+StuffShift*13][0], 0, 0);
					break;
				};
			};
			if (Check>1)
			{
				char Txt[200];
				switch (arStuff[i+StuffShift*13][2])
				{
				case 8:
					sprintf(Txt,"%s %s (10~)",(pFirstShardBonus+arStuff[i+StuffShift*13][2])->sName,(pFirstResource+arStuff[i+StuffShift*13][0])->sName);					
					break;
				case 9:
					sprintf(Txt,"%s %s",(pFirstShardBonus+arStuff[i+StuffShift*13][2])->sName,(pFirstDefender+arStuff[i+StuffShift*13][0])->sName);					
					break;
				};
				MakeStringHint(Txt);
				bHint=true;
			};
		};		

// Окна ввода
		for (int i=0; i<4; i++)
		{
			int Check=scrRes[i].Check();
			if (Check>2)
			{
				Inp[i].EnterInt(arRes[i]);
				Inp[i].Show();
				blit(bmpGUI_PickBonus,bmpScr,138+i*247,120,138+i*247,120,88,24);
				textprintf_centre_ex(bmpScr,Font1,173+i*247,126,COLOR_2,-1,"%d",arRes[i]*(pFirstShardBonus+1+i)->iPrice);
				iClick=-1;
				bChangePrice=true;
			};
			/*if (buttRes[i][0].Check())
			{
				arRes[i]--;				
				buttRes[i][1].Activate(1);
				if (arRes[i]==0)
				{buttRes[i][0].Activate(0);};
				Inp[i].EnterInt(arRes[i]);
				Inp[i].Show();
				blit(bmpGUI_PickBonus,bmpScr,138+i*247,120,138+i*247,120,88,24);
				textprintf_centre_ex(bmpScr,Font1,173+i*247,126,COLOR_2,-1,"%d",arRes[i]*(pFirstShardBonus+1+i)->iPrice);
				iClick=-1;
				bChangePrice=true;
				
			};
			if (buttRes[i][1].Check())
			{
				arRes[i]++;				
				buttRes[i][0].Activate(1);
				if (arRes[i]==MaxRes[i])
				{buttRes[i][1].Activate(0);};
				Inp[i].EnterInt(arRes[i]);
				Inp[i].Show();
				blit(bmpGUI_PickBonus,bmpScr,138+i*247,120,138+i*247,120,88,24);
				textprintf_centre_ex(bmpScr,Font1,173+i*247,126,COLOR_2,-1,"%d",arRes[i]*(pFirstShardBonus+1+i)->iPrice);
				blit(bmpGUI_PickBonus,bmpScr,536,730,536,730,260,26);
				textprintf_centre_ex(bmpScr,Font1,642,733,COLOR_2,-1,"%d",EnergySpent);
				iClick=-1;
				bChangePrice=true;
			};*/
			if (Check!=3)
			{
				if (mskRes[i].Check()==30)
				{
					iClick=i;				
				};
			};
			if (Inp[i].Check(true)==2)
			{
				iClick=-1;
			};
		};

//Ввод значений
		if (iClick>-2)
		{
			for (int i=0; i<4; i++)
			{
			//	int Old=arRes[i];
				arRes[i]=Inp[i].GetZyfer();
				if (arRes[i]>MaxRes[i])
				{
					arRes[i]=MaxRes[i];
					Inp[i].EnterInt(arRes[i]);
				};
				if (arRes[i]<0)
				{
					arRes[i]=MaxRes[i];
					Inp[i].EnterInt(arRes[i]);
				};
				Inp[i].Show();				
				blit(bmpGUI_PickBonus,bmpScr,138+i*247,120,138+i*247,120,88,24);
				textprintf_centre_ex(bmpScr,Font1,173+i*247,126,COLOR_2,-1,"%d",arRes[i]*(pFirstShardBonus+1+i)->iPrice);
				if (arRes[i]>0)
				{buttRes[i][0].Activate(1);}
				else
				{buttRes[i][0].Activate(0);};
				if (arRes[i]<MaxRes[i])
				{buttRes[i][1].Activate(1);}
				else
				{buttRes[i][1].Activate(0);};
				bChangePrice=true;
				Inp[i].Activate(0);
				scrRes[i].Show();
			};
			if (iClick>=0)
			{
				Inp[iClick].Activate(1);
			};
			iClick=-2;
		};

//Вычисление цены
		if (bChangePrice)
		{
			EnergySpent=0;
			for (int i=0; i<4; i++)
			{
				EnergySpent+=arRes[i]*(pFirstShardBonus+i+1)->iPrice;
			};
			for (int i=0; i<MaxUnit; i++)
			{
				if (arUnit[i][1]>0)
				{EnergySpent+=GetUnitEnergy(arUnit[i][0]);};
			};
			for (int i=0; i<MaxItem; i++)
			{
				if (arItem[i][1]>0)
				{EnergySpent+=GetItemEnergy(arItem[i][0]);};
			};
			for (int i=0; i<MaxBuild; i++)
			{
				if (arBuild[i][1]>0)
				{EnergySpent+=GetBuildEnergy(arBuild[i][0]);};
			};
			for (int i=0; i<MaxStuff; i++)
			{
				if (arStuff[i][1]>0)
				{
					switch (arStuff[i][2])
					{
					case 8:
						EnergySpent+=GetResourceEnergy(arStuff[i][0]);
						break;
					case 9:
						EnergySpent+=GetDefenderEnergy(arStuff[i][0]);
						break;
					};
				};
			};
			blit(bmpGUI_PickBonus,bmpScr,536,730,536,730,260,26);
			if (EnergySpent>Campaign.arCampPlayer[0].iEnergy)
			{
				textprintf_centre_ex(bmpScr,Font1,642,733,makecol(255,20,20),-1,"%d",EnergySpent);
				buttAttack.Activate(0);
			}
			else
			{
				textprintf_centre_ex(bmpScr,Font1,642,733,COLOR_2,-1,"%d",EnergySpent);
				buttAttack.Activate(1);
			};
			bChangePrice=false;
		};

		if (buttAttack.Check())
		{
			bRepeat=false;
		};

		if (buttExit.Check())
		{
			bRepeat=false;
			bAttack=false;
		};

		if (!Tutorial.Mode)
		{objTutorial->iLayer=-1;}
		

		if ((Timer)&&(CurGame.iCurrentTurn<0))
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,5,true);			
		};
		bHint=false;
	};

	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};	
	destroy_bitmap(bmpScr);
	//Вторжение
	if (bAttack)
	{
		if (Mode<0)
		{
			//В Мир игрока
			AttackPlayer(abs(Mode),0);
			arPlayer[1].Gold+=MaxRes[0];
			arPlayer[1].Gem+=MaxRes[1];
			arPlayer[1].iGoldAdd=MaxRes[2];
			arPlayer[1].iGemAdd=MaxRes[3];
			int Count=1;
			for (int i=0; i<MaxUnit; i++)
			{
				if (i>=MaxUnit-6)
				{
					dcUnit *pUnit=new dcUnit(arUnit[i][0],0);
					(pFirstProvince+arPlayer[1].ipProvince)->pGarrison->PlaceUnit(pUnit,Count);
					Count++;
				};
			};
			Count=0;
			for (int i=0; i<MaxItem; i++)
			{
				if (i>=MaxItem-36)
				{
					arPlayer[1].arTreasury[Count][0]=arItem[i][0];
					arPlayer[1].arTreasury[Count][1]=(pFirstItem+arItem[i][0])->iDurability;
					Count++;
				};
			};
			int Gold=arPlayer[1].Gold, Gem=arPlayer[1].Gem;
			for (int i=0; i<MaxBuild; i++)
			{
				if ((arBuild[i][0]>0)&&((pFirstInnerBld+arBuild[i][0])->iGroup<=0))
				{
					arPlayer[1].Build(arBuild[i][0]);					
				};
			};
			arPlayer[1].Gold=Gold; arPlayer[1].Gem=Gem;
			for (int i=0; i<MaxStuff; i++)
			{
				
				switch (arStuff[i][2])
				{
				case 8:
					arPlayer[1].arRes[arStuff[i][0]]=10;
					break;
				case 9:
					*(arPlayer[1].pDefender+arStuff[i][0])+=1;
					break;
				};				
			};
		}
		else
		{
			//На нейтральный осколок или в мир врага
			SaveGame(0);
			Campaign.arCampPlayer[0].iEnergy-=EnergySpent;
			if (Mode==0)
			{AttackShard(Shard);}
			else
			{AttackPlayer(0,Mode);};
			arPlayer[1].Gold+=arRes[0];
			arPlayer[1].Gem+=arRes[1];
			arPlayer[1].iGoldAdd=arRes[2];
			arPlayer[1].iGemAdd=arRes[3];
			int Count=1;
			for (int i=0; i<MaxUnit; i++)
			{
				if (arUnit[i][1]>0)
				{
					dcUnit *pUnit=new dcUnit(arUnit[i][0],0);
					(pFirstProvince+arPlayer[1].ipProvince)->pGarrison->PlaceUnit(pUnit,Count);
					Count++;
				};
			};
			Count=0;
			for (int i=0; i<MaxItem; i++)
			{
				if (arItem[i][1]>0)
				{
					arPlayer[1].arTreasury[Count][0]=arItem[i][0];
					arPlayer[1].arTreasury[Count][1]=(pFirstItem+arItem[i][0])->iDurability;
					Count++;
				};
			};
			int Gold=arPlayer[1].Gold, Gem=arPlayer[1].Gem;
			for (int i=0; i<MaxBuild; i++)
			{
				if (arBuild[i][1]>0)
				{
					arPlayer[1].Build(arBuild[i][0]);
				//	arPlayer[1].arBld[arBuild[i][0]]=true;					
				};
			};
			arPlayer[1].Gold=Gold; arPlayer[1].Gem=Gem;
			for (int i=0; i<MaxStuff; i++)
			{
				if (arStuff[i][1]>0)
				{
					switch (arStuff[i][2])
					{
					case 8:
						arPlayer[1].arRes[arStuff[i][0]]=10;
						break;
					case 9:
						*(arPlayer[1].pDefender+arStuff[i][0])+=1;
						break;
					};					
				};
			};
		};
	};
	iAntisleep=iAntisleepDelay;
	return Result;
};

//Поиск оппонента и атака мира игрока
void WorldAttack()
{
	ClearRoll();
	int Count=0;
	for (int i=1; i<17; i++)
	{
		if ((Campaign.arCampPlayer[i].bInGame)&&(Campaign.arCampPlayer[i].bFirstMeet)&&((pFirstPlayerProfile+i)->iAggro/5-Campaign.arCampPlayer[i].GetAttitude()>25-Campaign.iCurrentTurn/5))
		{
			RollerN[Count]=i;
			RollerP[Count]=(pFirstPlayerProfile+i)->iAggro/5-Campaign.arCampPlayer[i].GetAttitude()+Campaign.iCurrentTurn/5-25;
			Count++;
		};
	};
	if (Count>0)
	{
		int Att=Roll(false);
		AddEvent((pFirstPlayerProfile+Att)->ipEventAttack);
	};
};


void AstralTurn(bool bSkip)
{
//	dcShard *SH=pFirstShard;
	Campaign.arDialog[79]=0;
	int Income=0;
	//while (SH)
	//{
	//	if (SH->ipPlayer==0)
	//	{
	//		Income+=SH->Income;			
	//	};
	//	SH=SH->pNext;		
	//};
	Income=GetEnergyIncome();//*Campaign.arCampPlayer[0].iEnergyLose/100;
//	if (Income<0)
//	{Income=0;};
	Campaign.arCampPlayer[0].iEnergy+=Income;
	Campaign.arCampPlayer[0].iEnergy+=(Campaign.arCampPlayer[0].iEnergyBonus+Campaign.arCampPlayer[0].iEnergyPenalty);
	if (bSkip)
	{
		for (int i=1; i<MAX_PLAYERS; i++)
		{
			if (Campaign.arCampPlayer[i].bFirstMeet)
			{
				if (Random(2+Campaign.iDifficulty)>0)
				{Campaign.arCampPlayer[i].AddBuild();};				
			};
		};
	};

	for (int i=1; i<=iMaxCampEvent; i++)
	{
		if (((pFirstCampEvent+i)->ipAttacker>0)&&(Campaign.arDialog[(pFirstCampEvent+i)->ipAttacker]>0))
		{
			AddEvent(i);
			Campaign.arDialog[(pFirstCampEvent+i)->ipAttacker]=0;
		};
	};
	if (Campaign.arDialog[38]>1)
	{Campaign.arDialog[38]--;};
	//Триггеры рекламы
	if (Campaign.arDialog[44]>1)
	{Campaign.arDialog[44]--;}
	else
	{
		if (Campaign.arDialog[42]>0)
		{Campaign.arDialog[43]=1;};
	};	
	Campaign.iCurrentTurn++;	
};


//***********************************************************************************************************
//************ ОСНОВНОЙ ПРОЦЕСС АСТРАЛА *********************************************************************
//***********************************************************************************************************

int Astral()
{
//	Campaign.arCampPlayer[0].iKarma=80;
	iAntisleep=iAntisleepDelay;	
	int Result=0;
	bFinish=false;
	bTurn=false;
	bJournal=false;
	bShowRecords=false;
	bool bLostShard=false, bLoseEnergy=false;
	BITMAP* bmpJournal=create_bitmap(300,12);
	rectfill(bmpJournal,0,0,300,12,COLOR_0);
	textprintf_ex(bmpJournal,Font1,1,1,COLOR_1,-1,(pFirstPhrase+480)->str);
	textprintf_ex(bmpJournal,Font1,0,0,COLOR_2,-1,(pFirstPhrase+480)->str);
	int MY=0, AddY=-50, Glowing=50, GlowAdd=5, MaxBld=0, MaxBonus=0;
	int BarX=696, BarY=349;	
	if (CurGame.iCurrentTurn<0)
	{
		iGameMode=1;	
		ChangeMusic();
	};
	int LowY=768-bmpGUI_AstralLow->h;
	bCycle=true;
	bmpLowPanel=create_bitmap(bmpGUI_AstralLow->w,bmpGUI_AstralLow->h);	
	BITMAP *bmpInfoBar=create_bitmap(bmpGUI_ShardBar->w,bmpGUI_ShardBar->h);
	rectfill(bmpInfoBar,0,0,bmpGUI_ShardBar->w,bmpGUI_ShardBar->h,COLOR_0);
	blit(bmpGUI_ShardBar,bmpInfoBar,0,0,0,0,bmpGUI_ShardBar->w,bmpGUI_ShardBar->h);
	Campaign.arCampPlayer[0].ShardPic=create_bitmap(bmpMainShard->w,bmpMainShard->h);
	blit(bmpMainShard,Campaign.arCampPlayer[0].ShardPic,0,0,0,0,bmpMainShard->w,bmpMainShard->h);
	DrawHerb(arHumans[0].iColor1,arHumans[0].iColor2,arHumans[0].iSign,47,53,Campaign.arCampPlayer[0].ShardPic);
	set_trans_blender(200,200,200,0);
	Campaign.arCampPlayer[0].ShardMask=create_bitmap(bmpMainShard->w,bmpMainShard->h);		
	rectfill(Campaign.arCampPlayer[0].ShardMask,0,0,bmpMainShard->w,bmpMainShard->h,COLOR_0);
	draw_lit_sprite(Campaign.arCampPlayer[0].ShardMask,Campaign.arCampPlayer[0].ShardPic,0,0,100);
	for (int i=1; i<MAX_PLAYERS; i++)
	{
		Campaign.arCampPlayer[i].ShardPic=create_bitmap(bmpMainShard->w,bmpMainShard->h);
		blit(bmpMainShard,Campaign.arCampPlayer[i].ShardPic,0,0,0,0,bmpMainShard->w,bmpMainShard->h);
		DrawHerb((pFirstPlayerProfile+i)->iColor1,(pFirstPlayerProfile+i)->iColor2,(pFirstPlayerProfile+i)->iSign,47,53,Campaign.arCampPlayer[i].ShardPic);
		Campaign.arCampPlayer[i].ShardMask=create_bitmap(bmpMainShard->w,bmpMainShard->h);		
		rectfill(Campaign.arCampPlayer[i].ShardMask,0,0,bmpMainShard->w,bmpMainShard->h,COLOR_0);
		draw_lit_sprite(Campaign.arCampPlayer[i].ShardMask,Campaign.arCampPlayer[i].ShardPic,0,0,100);
	};
	set_trans_blender(128,128,128,128);
	int iMainBar=0, iRuler=0;
	dcShard *Select=NULL;

//****
	while (bCycle)
	{
	blit(bmpGUI_AstralLow,bmpLowPanel,0,0,0,0,bmpGUI_AstralLow->w,bmpGUI_AstralLow->h);
	textprintf_ex(bmpLowPanel,Font1,135,79,COLOR_2,-1,"%d",Campaign.arCampPlayer[0].iEnergy);
	G_Object *objMain=new G_Object(NULL,0,0,0,bmpAstral);
	objMain=new G_Object(objMain,0,768-bmpGUI_AstralLow->h,5,bmpLowPanel);
	objMain=new G_Object(objMain,BarX,BarY,5,bmpInfoBar);
	objMain=new G_Object(objMain,350,740,-1,bmpJournal);
	G_Object * objJournal=objMain;
	objJournal->iOpacity=1;
	objJournal->bTrans=true;
	UI_Mask mskBar[2];
	objMain=mskBar[0].Init(0,768-bmpGUI_AstralLow->h,1,bmpLowPanel,bmpEmpty,objMain,1);
	objMain=mskBar[1].Init(BarX,BarY,1,bmpInfoBar,bmpEmpty,objMain,1);
	UI_Mask mskBuild[MAX_SHARD_BUILD];
	UI_Mask mskBonus[MAX_SHARD_BUILD];
	UI_Mask1 mskCheat;
	mskCheat.Init(930,LowY+63,19,44,1);
	for (int i=0; i<MAX_SHARD_BUILD; i++)
	{
		objMain=mskBuild[i].Init(BarX+41+i*60-6,BarY+171-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objMain,5);
		objMain=mskBonus[i].Init(BarX+41+i*60-6,BarY+230-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objMain,5);
	};
	UI_Mask1 mskEnergy, mskIncome, mskShardSize, mskShardType;
	mskEnergy.Init(86,LowY+72,150,27,1);
	mskEnergy.iHint=467;
	mskIncome.Init(BarX+51,BarY+134,215,35,1);
	mskIncome.iHint=488;
	mskShardSize.Init(BarX+67,BarY+71,190,20,1);
	mskShardSize.iHint=489;
	mskShardType.Init(BarX+67,BarY+99,190,20,1);
	mskShardType.iHint=490;
	dcShard *SH=pFirstShard;	
	int WorldSize=GetWorldSize(0);
	AddY=(WorldSize-1)*50;

//Активизация вопросов Зарру в зависимости от размера мира
	switch (WorldSize)
	{
	case 2:
		if (Campaign.arDialog[15]==0)
		{
			Campaign.arDialog[13]=1;
			Campaign.arDialog[15]=1;
		};
		break;
	case 3:
		if (Campaign.arDialog[12]==0)
		{
			Campaign.arDialog[11]=1;
			Campaign.arDialog[12]=1;
		};
		break;
	case 5:
		if (Campaign.arDialog[247]==0)
		{
			Campaign.arDialog[236]=1;
			Campaign.arDialog[246]=1;
			Campaign.arDialog[247]=1;
		};
		break;
	case 6:
		if (Campaign.arDialog[250]==0)
		{
			Campaign.arDialog[249]=1;
			Campaign.arDialog[250]=1;
		};		
		break;
	case 8:
		if (Campaign.arDialog[261]==0)
		{
			Campaign.arDialog[260]=1;
			Campaign.arDialog[261]=1;
		};
		break;
	case 11:
		if (Campaign.arDialog[288]==0)
		{
			Campaign.arDialog[287]=1;
			Campaign.arDialog[288]=1;
		};
		break;
	case 15:
		if (Campaign.arDialog[206]==0)
		{
			Campaign.arDialog[205]=1;
			Campaign.arDialog[206]=1;
		};
		break;
	case 21:
		if (Campaign.arDialog[242]==0)
		{
			Campaign.arDialog[241]=1;
			Campaign.arDialog[242]=1;
			Campaign.arDialog[89]=1;
		};
		break;
	};



	SH=pFirstShard;
	while (SH)
	{
		if (SH->ipPlayer==-1)
		{
			objMain=new G_Object(objMain,0,0,2,SH->Pic);
			objMain->PlaceOnMap(SH->x,768+AddY-SH->y);
			objMain=SH->Mask.Init(0, 0, 1, SH->Pic, SH->Select, SH->Select, objMain, 3);		
			SH->Mask.G_Obj->PlaceOnMap(SH->x,768+AddY-SH->y);
		};
		SH=SH->pNext;
	};
	UI_Mask mskPlayer[MAX_PLAYERS];
	objMain=new G_Object(objMain,0,0,2,Campaign.arCampPlayer[0].ShardPic);
	objMain->PlaceOnMap(400,768-200);
	objMain=mskPlayer[0].Init(0,0,1,Campaign.arCampPlayer[0].ShardPic,Campaign.arCampPlayer[0].ShardMask,objMain,3);
	objMain->PlaceOnMap(400,768-200);
//Запланированные события
	for (int i=1; i<=iMaxCampEvent; i++)
	{
		if (((pFirstCampEvent+i)->iPossibility>0)&&((pFirstCampEvent+i)->iPossibility<=WorldSize)&&(Campaign.arDialog[i]==0))
		{
			AddEvent(i);
		};
	};
	if ((Campaign.arDialog[92]==1)&&(Campaign.arDialog[78]<1))
	{AddEvent(1188);};

//Появление новых владык
	for (int i=1; i<MAX_PLAYERS; i++)
	{
		if (((Campaign.arCampPlayer[i].Y<=AddY)||(Campaign.arCampPlayer[i].bFirstMeet))&&(Campaign.arCampPlayer[i].bInGame))
		{
			if (!Campaign.arCampPlayer[i].bFirstMeet)
			{
				FileLog((pFirstPlayerProfile+i)->sName);
				Campaign.arCampPlayer[i].Clear();
				Campaign.arCampPlayer[i].ipProfile=i;
				int Add=(Campaign.iCurrentTurn-WorldSize)*5/(10-Campaign.iDifficulty);
				for (int j=0; j<WorldSize*3+Add; j++)
				{Campaign.arCampPlayer[i].AddBuild();};
				Campaign.arCampPlayer[i].iSizeBonus=-Add/3;
				Campaign.arCampPlayer[i].bFirstMeet=true;
				AddEvent((pFirstPlayerProfile+i)->ipEventFirstMeet);
				for (int k=0; k<MAX_PLAYER_DIALOG; k++)
				{
					if ((pFirstPlayerProfile+i)->arEnterDlg[k]>0)
					{Campaign.arDialog[(pFirstPlayerProfile+i)->arEnterDlg[k]]=1;};
				};
			};
			objMain=new G_Object(objMain,0,0,2,Campaign.arCampPlayer[i].ShardPic);
		//	Campaign.arCampPlayer[i].Y=GetWorldSize(i)*50;
			objMain->PlaceOnMap(Campaign.arCampPlayer[i].X,AddY-Campaign.arCampPlayer[i].Y);
			objMain=mskPlayer[i].Init(0,0,1,Campaign.arCampPlayer[i].ShardPic,Campaign.arCampPlayer[i].ShardMask,objMain,3);
			objMain->PlaceOnMap(Campaign.arCampPlayer[i].X,AddY-Campaign.arCampPlayer[i].Y);
		};
	};

	UI_Button1 buttMenu;
	objMain=buttMenu.Init(8,LowY+62,1,6,bmpButton[104],bmpButton[105],bmpButton[106],bmpButton[106],objMain);
	buttMenu.Activate(1);
	buttMenu.iHint=96;
	buttMenu.ipHotkey=KEY_ESC;

	UI_Button1 buttZarr;
	objMain=buttZarr.Init(805,LowY+65,1,6,bmpButton[283],bmpButton[284],bmpButton[285],bmpButton[285],objMain);
	buttZarr.iHint=329;

	UI_Button1 buttStat;
	objMain=buttStat.Init(865,LowY+65,1,6,bmpButton[98],bmpButton[99],bmpButton[100],bmpButton[100],objMain);
	buttStat.Activate(1);
	buttStat.iHint=99;

	UI_Button1 buttEnd;
	objMain=buttEnd.Init(954,LowY+62,1,6,bmpButton[101],bmpButton[102],bmpButton[103],bmpButton[103],objMain);
	buttEnd.Activate(1);
	buttEnd.iHint=100;
	buttEnd.ipHotkey=KEY_E;

	UI_Button1 buttAttack;
	objMain=buttAttack.Init(BarX+100,BarY+287,-1,6,bmpButton[279],bmpButton[280],bmpButton[281],bmpButton[282],objMain);
	if (Select)
	{buttAttack.Activate(1);}
	else
	{MaxBld=0;MaxBonus=0;};
	buttAttack.iHint=200;

	UI_Button1 buttDialog;	
	objMain=buttDialog.Init(BarX+100,BarY+287,-1,6,bmpButton[279],bmpButton[280],bmpButton[281],bmpButton[282],objMain);
	if (iRuler>0)
	{buttDialog.Activate(1);};
	buttDialog.iHint=350;

	objMain=new G_Object(objMain,20,20,-1,bmpGUI_Hint);
	G_Object *objTutorial=objMain;
	UI_Mask mskTutorial;
	objMain=mskTutorial.Init(20,20,1,bmpGUI_Hint,bmpEmpty,objMain,-1);
	UI_TextBox txtTutorial;
	objMain=txtTutorial.Init(40,50,297,200,7,COLOR_1,Font2,objMain);
	txtTutorial.bShadow=false;
	txtTutorial.Hide();
	bool bTutHint=false;

	int Check;
	bool bRedraw=false;	

//***
	while ((!bRedraw)&&(bCycle))
	{	
		if (buttMenu.Check())
		{
			bExitGame=false;
			WindowMenu(0);
			if (bExitGame)
			{
				if (pFirstShard)
				{SaveGame(0);};
				bCycle=false;
			};
		};
		if (bTutHint)
		{
			int Check;
			if (Tutorial.Mode)
			{Check=mskTutorial.Check();}
			else
			{Check=30;};
			if ((Check==30)||(Check==40))
			{
				bTutHint=false;
				txtTutorial.Hide();
				objTutorial->iLayer=-1;
			};
		};	
	
		if ((CurGame.iCurrentTurn>=0)||(Campaign.arDialog[124]>0))
		{			
			if (CurGame.iCurrentTurn>=0)
			{
				iGameMode=2;
				RunGame(bTurn);
			}
			else
			{iExitFlag=4;};			

			if (iExitFlag<=0)
			{
				bCycle=false;
				dcShard* SH1;
				SH=pFirstShard;
				while (SH)
				{
					SH1=SH->pNext;
					delete SH;
					SH=SH1;
				};
				pFirstShard=NULL;
				Result=iExitFlag;
			}
			else
			{
			iGameMode=1;				
			ChangeMusic();
			
			if (Campaign.iActiveShard<0)
			{
			//После атаки мира
				if (Campaign.bDefender)
				{
					//Игрок защищался

					if (iExitFlag==1)
					{	
					//Поражение
						FinishCampaign(1);
						bCycle=false;
						bRedraw=true;
					};


					if (iExitFlag==2)
					{	
					//Победа
						Campaign.arCampPlayer[0].iSizeBonus++;
						MakeShards(0);
						AstralTurn(false);
						Select=NULL;
						bRedraw=true;
						iMainBar=0;				
						SaveGame(0);
					};
				}
				else
				{
					//Игрок атаковал
					if (iExitFlag==1)
					{	
					//Поражение
						bLoseEnergy=true;
						AstralTurn(false);
					};

					if (iExitFlag==2)
					{	
					//Победа
						/*for (int k=0; k<MAX_PLAYER_DIALOG; k++)
						{
							if ((pFirstPlayerProfile+abs(Campaign.iActiveShard))->arExitDlg[k]>0)
							{Campaign.arDialog[(pFirstPlayerProfile+abs(Campaign.iActiveShard))->arExitDlg[k]]=0;};
						};
						Campaign.arCampPlayer[abs(Campaign.iActiveShard)].bInGame=false;*/
						BanishPlayer(abs(Campaign.iActiveShard));
						if ((pFirstPlayerProfile+abs(Campaign.iActiveShard))->ipEventDefeat>0)
						{AddEvent((pFirstPlayerProfile+abs(Campaign.iActiveShard))->ipEventDefeat);};
						Campaign.arCampPlayer[0].iKillScore++;
					//	Campaign.arCampPlayer[0].iScore+=100;
						MakeShards(0);
						AstralTurn(false);
					};

					Select=NULL;
					bRedraw=true;
					iMainBar=0;				
					SaveGame(0);
				};
			}
			else
			{
			//После атаки осколка
				if (iExitFlag==1)
				{			
				//Поражение
					bLostShard=true;
					dcShard* SH1;
					SH=pFirstShard;
					if (SH->Num==Campaign.iActiveShard)
					{
						SH1=SH;
						pFirstShard=pFirstShard->pNext;
						delete SH1;
						Campaign.iMaxShards--;
					}
					else
					{
						while (Campaign.GetActiveShard())
						{
							if (SH->pNext->Num==Campaign.iActiveShard)
							{
								SH1=SH->pNext;
								SH->pNext=SH1->pNext;
								delete SH1;
								Campaign.iMaxShards--;
							}
							else
							{
								SH=SH->pNext;
							};
						};
					};					
					AstralTurn(false);					
				};

				if (iExitFlag==2)
				{	
				//Победа
					Campaign.GetActiveShard()->ipPlayer=0;
					/*SH=pFirstShard;
					while (SH)
					{
						if (SH->Num==Campaign.iActiveShard)
						{SH->ipPlayer=0;};
						SH=SH->pNext;
					};*/
					MakeShards(0);
					AstralTurn(false);
				};

				if (iExitFlag==4)
				{
				//Пройдена первая миссия
					if (Campaign.arDialog[125]>0)
					{AddEvent(129);}
					else
					{
						if (Campaign.arDialog[124]>0)
						{
							Campaign.arDialog[124]=0;
							AddEvent(124);
						}
						else
						{AddEvent(1);};
					};
					Tutorial.Map=false;
					arHumans[0].bFinishTutorial=true;
				//	arHumans[0].ipAvatar=18;
					arHumans[0].Save();
					for (int i=1; i<=iMaxInnerBld; i++)
					{
						if ((pFirstInnerBld+i)->iLevel<0)
						{Campaign.arCampPlayer[0].arBld[i]=1;};
					};
					SH=pFirstShard;
					while (SH)
					{
						if (SH->arBuilding[0]==17)
						{SH->ipPlayer=0;};
						SH=SH->pNext;
					};				
					blit(bmpMainShard,Campaign.arCampPlayer[0].ShardPic,0,0,0,0,bmpMainShard->w,bmpMainShard->h);
					DrawHerb(arHumans[0].iColor1,arHumans[0].iColor2,arHumans[0].iSign,47,53,Campaign.arCampPlayer[0].ShardPic);
					set_trans_blender(200,200,200,0);				
					rectfill(Campaign.arCampPlayer[0].ShardMask,0,0,bmpMainShard->w,bmpMainShard->h,COLOR_0);
					draw_lit_sprite(Campaign.arCampPlayer[0].ShardMask,Campaign.arCampPlayer[0].ShardPic,0,0,100);
					set_trans_blender(128,128,128,128);
					Tutorial.Move=50;					
				};
				Select=NULL;
				bRedraw=true;
				iMainBar=0;				
				SaveGame(0);
			};
			CurGame.iCurrentTurn=-1;
			};
		};
		if (!bRedraw)
		{

		if (bJournal)
		{
			bJournal=false;
			objJournal->iOpacity=255;
			objJournal->iLayer=6;
			buttStat.bBlink=true;
		};

		if (
		//	(BF_Cheat)&&
			(mskCheat.Check()==30))
		{
			if (key[KEY_0])
			{Campaign.iDifficulty=0;};
			if (key[KEY_1])
			{Campaign.arDialog[665]=1;};
			if (key[KEY_2])
			{Campaign.arDialog[667]=1;};
			if (key[KEY_3])
			{Campaign.arDialog[669]=1;};
			if (key[KEY_4])
			{Campaign.arItems[3]+=520;};
			if (key[KEY_5])
			{Campaign.iDifficulty=5;};
			if (key[KEY_6])
			{Campaign.iDifficulty=6;};
			if (key[KEY_A])
			{
				Campaign.arCampPlayer[0].iEnergy+=1000;
				bRedraw=true;
			};
	//		if (key[KEY_E])
	//		{RunCampaignEvent(775,0,0);};
			if (key[KEY_W])
			{
				int ST=0;
				for (int i=1; i<17; i++)
				{
					if ((Campaign.arCampPlayer[i].bInGame)&&(Campaign.arCampPlayer[i].bFirstMeet))
					{
						textprintf(screen,Font2, 10, 10+ST*20, COLOR_2, "%s: %d",(pFirstPlayerProfile+i)->sName,GetWorldSize(i)+Campaign.arCampPlayer[i].iSizeBonus);
						ST++;
					};
				};
				while((key[KEY_W]))
				{};
				clear_keybuf();
				readkey();
			};

		};

		if (buttStat.Check())
		{
			ScreenCampStat();
			buttStat.bBlink=false;
		};

		if (buttZarr.Check())
		{
			if (Campaign.arDialog[256])
			{//Триггер с вопросом о демонах
				if ((Campaign.arDialog[257])||(Campaign.arDialog[258]))
				{
					Campaign.arDialog[256]=0;
					Campaign.arDialog[255]=1;
				};
			};
			iGameMode=8;				
			ChangeMusic();
			buttZarr.bBlink=false;
			if (Campaign.arDialog[79]>0)
			{RunCampaignEvent(871,0,0);}
			else
			{
				if (Campaign.arDialog[78]>0)
				{RunCampaignEvent(860,0,0);}
				else
				{
					if (Campaign.arDialog[236]>0)
					{RunCampaignEvent(236,0,0);}
					else
					{
						if (Campaign.arDialog[241]>0)
						{RunCampaignEvent(241,0,0);}
						else				
						{RunCampaignEvent(12,0,0);};
					};
				};
			};
			iGameMode=1;				
			ChangeMusic();
			bRedraw=true;
		};
		
		if (buttEnd.Check())
		{
			AstralTurn(true);
			bRedraw=true;
		};
		SH=pFirstShard;
		mskBar[0].Check();
		mskBar[1].Check();
		if ((mskBar[0].flag<2)&&(mskBar[1].flag<2))
		{			
			bool MainCheck=true;
			while (SH)
			{
				if (SH->ipPlayer==-1)
				{
					Check=SH->Mask.Check(0,MY);
					if ((SH!=Select)&&(SH->Mask.bGlow))
					{SH->Mask.bGlow=false;};
					if (Check>1)
					{
						MainCheck=false;
						for (int i=0; i<MAX_PLAYERS; i++)
						{
							if ((Campaign.arCampPlayer[i].bInGame)&&(Campaign.arCampPlayer[i].bFirstMeet))
							{
								mskPlayer[i].flag=1;
								mskPlayer[i].G_Obj->iLayer=-1;
							};
						};
						if (SH->arEvent[0]>0)
						{
							MakeTextHint(SH->arEvent[0],(pFirstWorldRules+SH->arEvent[0])->sText);
							bHint=true;
						};
					};
					if ((Check==30)||(Check==40))
					{
						MaxBld=0; 
						MaxBonus=0;
						blit(bmpGUI_ShardBar,bmpInfoBar,0,0,0,0,bmpGUI_ShardBar->w,bmpGUI_ShardBar->h);
						textprintf_centre_ex(bmpInfoBar,Font1,161,44,COLOR_1,-1,SH->sName);						
						textprintf_centre_ex(bmpInfoBar,Font1,161,74,COLOR_1,-1,arShardSize[(SH->Map.X-6)/2].sName);
						textprintf_centre_ex(bmpInfoBar,Font1,161,104,COLOR_1,-1,(pFirstPhrase+SH->Terrain+334)->str);
						textprintf_centre_ex(bmpInfoBar,Font1,161,145,COLOR_2,-1,"+%d",SH->Income);
						for (int i=0; i<MAX_SHARD_BUILD; i++)
						{
							if (SH->arBuilding[i]>0)
							{
								draw_sprite(bmpInfoBar,(pFirstInnerBld+SH->arBuilding[i])->Pic,41+i*60,171);
								MaxBld++;
							};
						};
						for (int i=0; i<MAX_SHARD_BUILD; i++)
						{
							if (SH->arBonus[i][0]>0)
							{
								switch (SH->arBonus[i][0])
								{
								case 1:
								case 2:
								case 3:
								case 4:
									draw_sprite(bmpInfoBar,bmpShardBonus[SH->arBonus[i][0]-1],41+i*60,230);
									break;
								case 6:
									draw_sprite(bmpInfoBar,bmpShardBonus[4],41+i*60,230);
									break;
								case 5:
									draw_sprite(bmpInfoBar,(pFirstItem+SH->arBonus[i][1])->Pic,41+i*60,230);
									break;
								case 7:
									draw_sprite(bmpInfoBar,(pFirstInnerBld+SH->arBonus[i][1])->Pic,41+i*60,230);
									break;
								case 8:
									if (SH->arBonus[i][1]>0)
									{draw_sprite(bmpInfoBar,(pFirstResource+SH->arBonus[i][1])->BigPic,41+i*60,230);};
									break;
								case 9:
									draw_sprite(bmpInfoBar,(pFirstDefender+SH->arBonus[i][1])->Pic,41+i*60,230);
									break;
								};								
								MaxBonus++;
							};
						};
						buttAttack.Activate(1);
						buttDialog.Activate(-1);
						iRuler=-1;
						Select=SH;
						if (Tutorial.Move==51)
						{
							Tutorial.Move=52;
							txtTutorial.Activate((pFirstTutText+71)->msg);
							objTutorial->iLayer=6;
							bTutHint=true;
							ScrollSound();
							buttAttack.bBlink=true;
						};
					};
				};
				SH=SH->pNext;
			};
			if (MainCheck)
			{
				for (int i=0; i<MAX_PLAYERS; i++)
				{
					if ((Campaign.arCampPlayer[i].bInGame)&&(Campaign.arCampPlayer[i].bFirstMeet))
					{
						Check=mskPlayer[i].Check(0,MY);
						if ((Check==30)||(Check==40))
						{iMainBar=i;};
					};
				};
			};
		};
		Check=mskEnergy.Check();
		if ((Check==30)||(Check==40))
		{WindowInfo((pFirstPhrase+467)->str,(pFirstMessage+92)->msg,bmpStuff[65]);};
		if (iMainBar>=0)
		{
			MaxBld=0; MaxBonus=0;
			buttAttack.Activate(-1);
			blit(bmpGUI_MainShardBar,bmpInfoBar,0,0,0,0,bmpGUI_MainShardBar->w,bmpGUI_MainShardBar->h);
			if (iMainBar==0)
			{				
				draw_sprite(bmpInfoBar,bmpAvatar[18],94,65);
				DrawHerb(arHumans[0].iColor1,arHumans[0].iColor2,arHumans[0].iSign,57,85,bmpInfoBar);
				DrawHerb(arHumans[0].iColor1,arHumans[0].iColor2,arHumans[0].iSign,230,85,bmpInfoBar);
				textprintf_centre_ex(bmpInfoBar,Font1,160,43,COLOR_1,-1,arHumans[0].sName);
				textprintf_centre_ex(bmpInfoBar,Font1,160,231,COLOR_1,-1,arDifficulty[Campaign.iDifficulty].sName);
				textprintf_centre_ex(bmpInfoBar,Font1,160,263,COLOR_1,-1,GetRulerNick(Campaign.arCampPlayer[0].iKarma));
				buttDialog.Activate(-1);
			}
			else
			{
				draw_sprite(bmpInfoBar,bmpAvatar[iMainBar-1],94,65);
				DrawHerb((pFirstPlayerProfile+iMainBar)->iColor1,(pFirstPlayerProfile+iMainBar)->iColor2,(pFirstPlayerProfile+iMainBar)->iSign,57,85,bmpInfoBar);
				DrawHerb((pFirstPlayerProfile+iMainBar)->iColor1,(pFirstPlayerProfile+iMainBar)->iColor2,(pFirstPlayerProfile+iMainBar)->iSign,230,85,bmpInfoBar);
				textprintf_centre_ex(bmpInfoBar,Font1,160,43,COLOR_1,-1,(pFirstPlayerProfile+iMainBar)->sName);
				textprintf_centre_ex(bmpInfoBar,Font1,160,231,COLOR_1,-1,GetAttitudeName(Campaign.arCampPlayer[iMainBar].GetAttitude()));
				buttDialog.Activate(1);
			};
			iRuler=iMainBar;
			Select=NULL;
			iMainBar=-1;
		};
		if (iRuler>0)
		{
			if (buttDialog.Check())
			{
				if (key[KEY_PLUS_PAD])
				{Campaign.arCampPlayer[iRuler].arAttitude[0]+=10;}
				else
				{
				if (key[KEY_MINUS_PAD])
				{Campaign.arCampPlayer[iRuler].arAttitude[0]-=10;}
				else
				{
					bInvasion=false;
					RunCampaignEvent((pFirstPlayerProfile+iRuler)->ipEventMeet,iRuler,0);
					if (bInvasion)
					{
						if (PickBonusScreen(NULL, iRuler))
						{bTurn=true;};
					};

				};
				};
			};
		};
		if (Select)
		{
			Check=mskIncome.Check();
			if ((Check==30)||(Check==40))
			{WindowInfo((pFirstPhrase+467)->str,(pFirstMessage+92)->msg,bmpStuff[65]);};
			Check=mskShardSize.Check();
			if ((Check==30)||(Check==40))
			{WindowInfo((pFirstPhrase+489)->str,(pFirstMessage+93)->msg,bmpStuff[58]);};
			Check=mskShardType.Check();
			if ((Check==30)||(Check==40))
			{WindowInfo((pFirstPhrase+490)->str,(pFirstMessage+94)->msg,bmpStuff[31]);};
			for (int i=0; i<MaxBld; i++)
			{
				Check=mskBuild[i].Check();
				if (Check>1)
				{
					MakeInnBldHint(Select->arBuilding[i],-1);
					bHint=true;
				};
			};
			for (int i=0; i<MaxBonus; i++)
			{
				Check=mskBonus[i].Check();
				if (Check>1)
				{
					char Txt[200];
					switch(Select->arBonus[i][0])
					{
					case 1:
					case 2:
					case 3:
					case 4:
						sprintf(Txt,"%s %d",(pFirstShardBonus+Select->arBonus[i][0])->sName,Select->arBonus[i][1]);
						break;
					case 5:
						sprintf(Txt,"%s %s",(pFirstShardBonus+Select->arBonus[i][0])->sName,(pFirstItem+Select->arBonus[i][1])->sName);
						break;
					case 6:
						sprintf(Txt,"%s %s",(pFirstShardBonus+Select->arBonus[i][0])->sName,(pFirstUnitType+Select->arBonus[i][1])->sName);
						break;
					case 7:
						sprintf(Txt,"%s %s",(pFirstShardBonus+Select->arBonus[i][0])->sName,(pFirstInnerBld+Select->arBonus[i][1])->sName);
						break;
					case 8:
						sprintf(Txt,"%s %s (10~)",(pFirstShardBonus+Select->arBonus[i][0])->sName,(pFirstResource+Select->arBonus[i][1])->sName);
						break;
					case 9:
						sprintf(Txt,"%s %s",(pFirstShardBonus+Select->arBonus[i][0])->sName,(pFirstDefender+Select->arBonus[i][1])->sName);
						break;
					};
					MakeStringHint(Txt);
					bHint=true;
				};
			};
			if (buttAttack.Check())
			{
				if ((bDemo)&&(GetWorldSize(0)>=2))
				{					
					bCycle=false;
					int Answer=(pFirstCampDialog+177)->Show(1,1,1,0,NULL,NULL);
					if (Answer==3){OpenWeb();};
				}
				else
				{
				
				if ((key[KEY_V])&&(BF_Cheat))
				{					
					Select->ipPlayer=0;
					MakeShards(0);
					Select=NULL;
					bRedraw=true;
					iMainBar=0;					
				}
				else
				{
					int NeutralPower=1+WorldSize/5;
					if (arHumans[0].iEnterData==2)
					{
						if (!WindowEnter(arHumans[0].sLand,331,-1))
						{
							sprintf(arHumans[0].sLand,(pFirstPhrase+217)->str);
						};
						arHumans[0].ipAvatar=18;
						arHumans[0].iEnterData=3;
						arHumans[0].Save();						
					};
					if (Tutorial.Bonus==0)
					{
						Tutorial.Bonus=1;
						Campaign.iDifficulty=DifficultyWindow();
						Campaign.arJournal[979]=Campaign.iDifficulty;
						Campaign.arJournal[978]=arCampDiff[Campaign.iDifficulty][3];

						AttackShard(Select);
						bTurn=true;
					}
					else
					{
						if (PickBonusScreen(Select, 0))
						{bTurn=true;};
					};				
				};
				};
			};
		};
		if (!Tutorial.Mode)
		{objTutorial->iLayer=-1;};
		
		if ((Timer)&&(CurGame.iCurrentTurn<0))
		{
			if (Select)
			{
				Select->Mask.bGlow=true;
				Select->Mask.Glow_Obj->iOpacity=Glowing;
				Glowing+=GlowAdd;
				if ((Glowing>150)||(Glowing<50))
				{GlowAdd=-GlowAdd;};
			};
			if (((mouse_y==0)||(key[KEY_UP]))&&(MY>0))
			{
				MY-=arHumans[0].iScrollSpeed;
				if (MY<0)
				{MY=0;}
			};			
			if (((mouse_y==767)||(key[KEY_DOWN]))&&(MY<AddY))
			{
				MY+=arHumans[0].iScrollSpeed;
				if (MY>AddY)
				{
					MY=AddY;
				};
			};
			if (objJournal->iLayer==6)
			{
				objJournal->iOpacity--;
				if (objJournal->iOpacity==1)
				{objJournal->iLayer=-1;};
			};
			
			/*if (iIncTimer>0)
			{iIncTimer--;}
			else
			{
				if (iIncomeY>0)
				{
					iIncomeY--;
					objIncome->y=768-iIncomeY;
				};
			};
			if (((mouse_x==0)||(key[KEY_LEFT]))&&(MX>0))
			{
				MX-=SCROLL_SPEED;
				if (MX<0)
				{MX=0;};
			};
			if (((mouse_y==0)||(key[KEY_UP]))&&(MY>0))
			{
				MY-=SCROLL_SPEED;
				if (MY<0)
				{MY=0;}
			};
			if (((mouse_x==1023)||(key[KEY_RIGHT]))&&(MX<bmpMap->w-724))
			{
				MX+=SCROLL_SPEED;
				if (MX>bmpMap->w-724)
				{
					MX=bmpMap->w-724;
				};
			};
			if (((mouse_y==767)||(key[KEY_DOWN]))&&(MY<bmpMap->h-741))
			{
				MY+=SCROLL_SPEED;
				if (MY>bmpMap->h-741)
				{
					MY=bmpMap->h-741;
				};
			};*/
			Timer=false;
			objMain=ShowActiveObjects(objMain,10,true,0,MY);
			for (int i=0; i<50; i++)
			{
				if (Campaign.Events[i]>0)
				{
					RunCampaignEvent(Campaign.Events[i],0,0);
					Campaign.Events[i]=0;
					bRedraw=true;
					break;
				};
			};

			if (bLoseEnergy)
			{
				bLoseEnergy=false;
				int Lose=1+GetEnergyIncome()/30+Random(GetEnergyIncome()/12);
				Campaign.arCampPlayer[0].iEnergyDrain+=Lose;
				SaveGame(0);
				char Txt[1000];
				sprintf(Txt,(pFirstCampDialog+1310)->sText,Lose);
				(pFirstCampDialog+1310)->Show(1,1,1,0,NULL,Txt);
			};

			IsEndGame();
			
			//Проверка состояния квестов
			if (bCycle)
			{
				//Вести о Гарросе
				if (Campaign.arDialog[38]==1)
				{AddEvent(1451);};
				//Собраны все части разрушителя для Уль-Дагана
				if (Campaign.arItems[2]==6)
				{AddEvent(972);};
				//Собраны все части разрушителя
				if ((Campaign.arDialog[34]==0)&&(Campaign.arItems[1]==6))
				{AddEvent(1296);};
				if (Campaign.arDialog[31]>0)
				{
					Campaign.arDialog[31]=0;
					AddEvent(1226);
				};

				//***Сообщения Зарра***
				if (Campaign.arDialog[78]==0)
				{
					//Предупреждение об атаке демонов
					if ((Campaign.arDialog[98]==0)&&(Campaign.iCurrentTurn>=Campaign.arJournal[978]-10+Campaign.iTimeBonus))
					{AddEvent(1302);};
					//Реклама
					if ((Campaign.arDialog[41]==0)&&(Campaign.arCampPlayer[0].iMapScore>2000))
					{AddEvent(1452);};
					//Реклама (возвращение)
					if (Campaign.arDialog[44]==1)
					{AddEvent(1467);};
					if ((!Campaign.arCampPlayer[10].bInGame)&&(Campaign.arCampPlayer[16].bInGame)&&(Campaign.iCurrentTurn>49))
					{AddEvent(1019);};
				};

				for (int i=0; i<MAX_CAMP_QUEST; i++)
				{
					if ((Campaign.arQuest[i][0]>0)&&(Campaign.arQuest[i][1]==2)&&((pFirstQuest+Campaign.arQuest[i][0])->ipEncDone>0))
					{
						//RunCampaignEvent((pFirstQuest+Campaign.arQuest[i][0])->ipEncDone,0,0);
						AddEvent((pFirstQuest+Campaign.arQuest[i][0])->ipEncDone);
						Campaign.arQuest[i][1]=4;
						break;
					};
					if ((Campaign.arQuest[i][0]>0)&&(Campaign.arQuest[i][1]<2)&&((pFirstQuest+Campaign.arQuest[i][0])->Param[0]>0))
					{
						Campaign.arQuest[i][1]=3;
					};
				};
				//Атака лАншара
				if ((Campaign.arCampPlayer[7].bInGame)&&(Campaign.arDialog[359]==0)&&(Campaign.arDialog[350]==0)&&(GetWorldSize(0)==24))
				{AddEvent(1310);};

				//Финал лАншара
				if ((Campaign.arCampPlayer[7].bInGame)&&(Campaign.arDialog[360]==0)&&(Campaign.arDialog[350]>0)&&(GetWorldSize(0)>=46))
				{AddEvent(1313);};

				//Финал Белеза
				if ((Campaign.arCampPlayer[3].bInGame)&&(Campaign.arDialog[103]>0)&&(Campaign.arItems[4]>6))
				{AddEvent(1316);};
			};
			//Проверка атаки мира
			if ((Campaign.iReserv<GetWorldSize(0))&&(bCycle)&&(!bRedraw)&&(NoEvents()))
			{
				if ((Campaign.arDialog[73]==0)&&(GetWorldSize(0)==12))
				{
					Campaign.arDialog[73]=1;
					RunCampaignEvent(349,0,0);
					PickBonusScreen(NULL, -11);
					bTurn=true;
					Campaign.iReserv=GetWorldSize(0);
				};
				switch (GetWorldSize(0))
				{
				case 29:
					if ((Campaign.arCampPlayer[16].bInGame)&&(Campaign.arDialog[574]>0))
					{
						Campaign.arDialog[574]=0;
						RunCampaignEvent((pFirstPlayerProfile+16)->ipEventAttack,0,0);
					}
					else
					{WorldAttack();};
					Campaign.iReserv=GetWorldSize(0);
					break;
				case 39:
					if ((Campaign.arCampPlayer[1].bInGame)&&(Campaign.arDialog[702]>0))
					{						
						RunCampaignEvent((pFirstPlayerProfile+1)->ipEventAttack,0,0);
					}
					else
					{WorldAttack();};
					Campaign.iReserv=GetWorldSize(0);
					break;
				case 48:
				case 57:
				case 65:
				case 72:
				case 78:
				case 83:
				case 87:
				case 91:
				case 95:
				case 99:
				case 103:
					WorldAttack();
					Campaign.iReserv=GetWorldSize(0);
					break;
				};
			};

			if ((Tutorial.Mode)&&(bCycle)&&(!bRedraw)&&(NoEvents()))
			{
				switch(Tutorial.Move)
				{
				case 50:
					Tutorial.Move=51;
					txtTutorial.Activate((pFirstTutText+70)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
					break;	
				};
				if ((Tutorial.Move==52)&&(WorldSize==2))
				{
					Tutorial.Move=53;				
					txtTutorial.Activate((pFirstTutText+92)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
				};
				if ((Tutorial.Bonus==2)&&(WorldSize==3))
				{
					Tutorial.Bonus=3;				
					txtTutorial.Activate((pFirstTutText+89)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
				};
				if ((WorldSize==6)&&(Tutorial.Move==53))
				{
					Tutorial.Move=54;
					txtTutorial.Activate((pFirstTutText+91)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
					buttZarr.bBlink=true;
				};
				if ((WorldSize==14)&&(Tutorial.Move==54))
				{
					Tutorial.Move=55;
					txtTutorial.Activate((pFirstTutText+113)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
					buttStat.bBlink=true;
				};
				if ((bLostShard)&&(Tutorial.LostShard==0))
				{
					Tutorial.LostShard=1;
					bLostShard=false;
					txtTutorial.Activate((pFirstTutText+117)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();					
				};
			};
			if ((WorldSize==15)&&(Campaign.arDialog[90]==0)&&(bCycle)&&(!bRedraw)&&(NoEvents()))
			{
				Campaign.arDialog[90]=1;
				Campaign.arDialog[260]=0;
				txtTutorial.Activate((pFirstTutText+90)->msg);
				objTutorial->iLayer=6;
				bTutHint=true;
				ScrollSound();
			};
			bHint=false;
		};
		};

	};//bRedraw
	G_Object * Temp;
	while (objMain)
	{
		Temp=objMain->pNext;		
		delete objMain;
		objMain=Temp;
	};
	};//bCycle
	destroy_bitmap(bmpJournal);
	destroy_bitmap(bmpInfoBar);
	destroy_bitmap(bmpLowPanel);
//	destroy_bitmap(Campaign.arCampPlayer[0].ShardPic);
	dcShard *SH=pFirstShard;
	while (pFirstShard)
	{
		SH=SH->pNext;
		delete pFirstShard;
		pFirstShard=SH;
	};
	for (int i=0; i<MAX_PLAYERS; i++)
	{
		destroy_bitmap(Campaign.arCampPlayer[i].ShardPic);		
		destroy_bitmap(Campaign.arCampPlayer[i].ShardMask);		
	};
	if (bShowRecords)
	{WindowCampHighScore();};
	iGameMode=0;
	ChangeMusic();
	iAntisleep=iAntisleepDelay;
	return Result;
	
//	if (bmpIncome)
//	{destroy_bitmap(bmpIncome);};
};