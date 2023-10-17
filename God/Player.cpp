#include "Unit.h"

dcShop::dcShop()
{
	for (int i=0; i<SHOP_SIZE; i++)
	{
		Items[i][0]=0;
		Items[i][1]=0;
	};
	for (int i=0; i<MAX_PLAYERS; i++)
	{
		PriceModifier[i]=100;
	};
};

dcShop::~dcShop()
{};

const char* GetAttitudeName(int Att)
{
	for (int i=0; i<=iMaxAttitude; i++)
	{
		if (((pFirstAttitude+i)->Min<=Att)&&((pFirstAttitude+i)->Max>=Att))
		{return (pFirstAttitude+i)->sText;};
	};
	return "";
};

dcPlayer::dcPlayer()
{
	Control=0;
	bBldFilter=false;
	bDefFilter=false;
	bRitFilter=false;
	bEnter=true;
	pEvent=NULL;
	for (int i=0; i<MAX_RECRUIT; i++)
	{
		arRecruit[i]=0;
	};

	for (int i=0; i<LIBRARY_SIZE; i++)
	{
		arLibrary[i]=0;
	};

	for (int i=0; i<TREASURY_SIZE; i++)
	{
		arTreasury[i][0]=0;
		arTreasury[i][1]=0;
	};

	for (int i=0; i<MAX_HERO; i++)
	{
		arHero[i]=NULL;
	};	

	for (int i=1; i<MAX_BLD; i++)
	{
		arBld[i]=false;
	};

	for (int i=0; i<MAX_PLAYERS; i++)
	{
		Politic[i].Clear();
	};
	for (int i=0; i<MAX_GROUP; i++)
	{
		for (int j=0; j<MAX_GROUP_BUILD; j++)
		{
			arGroup[i][j]=0;
		};
	};
	AIUnion[0]=0;
	AIUnion[1]=0;
	arBld[0]=true;
	iMember=0;
	iCondition=15;
};

dcPlayer::~dcPlayer()
{};

void dcPlayer::Init(_Bool InGame, int Color)
{
	for (int i=0; i<MAX_RECRUIT; i++)
	{
		arRecruit[i]=0;
	};

	for (int i=0; i<LIBRARY_SIZE; i++)
	{
		arLibrary[i]=0;
	};

	for (int i=0; i<TREASURY_SIZE; i++)
	{
		arTreasury[i][0]=0;
		arTreasury[i][1]=0;
	};

	for (int i=0; i<MAX_HERO; i++)
	{
		arHero[i]=NULL;
	};	

	for (int i=1; i<MAX_BLD; i++)		
	{
		arBld[i]=false;
	};

	for (int i=0; i<SHOP_SIZE; i++)
	{
		Shop.Items[i][0]=0;
		Shop.Items[i][1]=0;
	};
	for (int i=0; i<MAX_PLAYERS; i++)
	{
		Shop.PriceModifier[i]=100;
	};

	for (int i=0; i<MAX_PLAYERS; i++)
	{
		Politic[i].Clear();
		/*if ((Control!=1)&&(arPlayer[i].bInGame)&&(arPlayer[i].Control==2))
		{
			Politic[i].Attitude=(pFirstPlayerProfile+ipProfile)->arAtt[i];
		};*/
	};

	for (int i=0; i<MAX_GROUP; i++)
	{
		for (int j=0; j<MAX_GROUP_BUILD; j++)
		{
			arGroup[i][j]=0;
		};
	};

	for (int i=0; i<10; i++)		
	{
		arRes[i]=0;
	};

	for (int i=0; i<MAX_CAMPERS; i++)
	{arCamper[i]=0;};

	arBld[0]=true;
	bEnter=true;
	pEvent=NULL;
	iUnion=0;
	bInGame=InGame;
	Num=Color;
//	ipProfile=Color;
	iMercTime=0;
	iPlanned=0;
	iColor=Color-1;
	iWall=100;
	iTower=0;
	iTowerHealth=10;
	iTowerShoot=12;
	iCorrupt=0;
	iCalcCorrupt=0;
	if (Control==1)
	{iKarma=0;}
	else
	{iKarma=(pFirstPlayerProfile+ipProfile)->iKarma;};
	GoldInc=0;
	GemInc=0;
	iGoldAdd=0;
	iGemAdd=0;
	iWin=0;
	iLose=0;
	iQuest=0;
	iDragonHunt=0;
	iExplore=0;
	iPillage=0;
	iTimeShift=0;
	bNoLoss=true;
	bSet=false;
	for (int i=0; i<3; i++)
	{arLoot[i]=0;};
	for (int i=0; i<17; i++)
	{arKills[i]=0;};
	AIUnion[0]=0;
	AIUnion[1]=0;
	iMember=0;
	iCondition=15;
	iCanInnerBld=1;
	iCanOuterBld=1;
	iCanHire=1;
	iCanCast=1;

	pDefender=new int[iMaxDefender+1];
	for (int i=0; i<=iMaxDefender; i++)
	{
		*(pDefender+i)=0;
	};

	pOuterBld=new int[iMaxOuterBld+1];
	for (int i=0; i<=iMaxOuterBld; i++)
	{
		*(pOuterBld+i)=0;
	};

	pRitual=new int[iMaxRitual+1];
	pRitualCD=new int[iMaxRitual+1];
	for (int i=0; i<=iMaxRitual; i++)
	{
		*(pRitual+i)=0;
		*(pRitualCD+i)=0;
	};

	pStory=new int[iMaxEvent+1];
	pPlace=new int[iMaxEvent+1];
	for (int i=0; i<=iMaxEvent; i++)
	{
		*(pStory+i)=-1;
		*(pPlace+i)=0;
	};

	if (InGame)
	{
		MakeFlags();
	};
};

void dcPlayer::Clear()
{
	
//	WriteLog("Защитники");
//	readkey();
	delete [] pDefender;
	
//	WriteLog("Постройки");
//	readkey();
	delete [] pOuterBld;
//	WriteLog("Ритуалы");
//	readkey();
	delete [] pRitual;
	delete [] pRitualCD;
//	WriteLog("Туман");
//	readkey();
	delete [] pFog;
	delete [] pStory;
	delete [] pPlace;

//	WriteLog("Герои");
//	readkey();

	while (arHero[0])
	{delete arHero[0];};


	for (int i=0; i<MAX_HERO; i++)
	{
		if (arHero[i])
		{			
			arHero[i]=NULL;
		};
	};
	while (pEvent)
	{			
		dcEvent *EV=pEvent->pNext;
		delete pEvent;
		pEvent=EV;
	};
	
};

int dcPlayer::GetResource(int Index)
{
	int Result=0;
	for (int i=1; i<=MapSize; i++)
	{
		if (((pFirstProvince+i)->ipPlayer==Num)&&((pFirstProvince+i)->GetResource(1)==Index))
		{
			Result++;
		};
	};
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if ((i!=Num)&&(arPlayer[i].bInGame)&&(Politic[i].Trade.bTrade))
		{
			for (int j=0; j<16; j++)
			{
				if (Politic[i].Trade.Stuff[j]==Index)
				{Result--;};
				if (arPlayer[i].Politic[Num].Trade.Stuff[j]==Index)
				{Result++;};
			};
		};
	};
	return Result;
};

int dcPlayer::GetResGoldPrice(int Index)
{
	int Result=0;
	if (GetResource(Index)==0)
	{
		Result=(pFirstResource+Index)->iGold*CurGame.ResPrices[Index]/100;
		Result-=Result*arRes[Index]/100;
		Result-=Result*GetBldParam1(17)/100;				
	};
	return Result;
};

int dcPlayer::GetResGemPrice(int Index)
{
	int Result=0;
	if (GetResource(Index)==0)
	{
		Result=(pFirstResource+Index)->iGem*CurGame.ResPrices[Index]/100;
		Result-=Result*arRes[Index]/100;
		Result-=Result*GetBldParam1(17)/100;				
	};
	return Result;
};

int dcPlayer::GetResGoldPriceStat(int Index)
{
	int Result=0;
	Result=(pFirstResource+Index)->iGold*CurGame.ResPrices[Index]/100;
	Result-=Result*arRes[Index]/100;
	Result-=Result*GetBldParam1(17)/100;
	return Result;
};

int dcPlayer::GetResGemPriceStat(int Index)
{
	int Result=0;
	Result=(pFirstResource+Index)->iGem*CurGame.ResPrices[Index]/100;
	Result-=Result*arRes[Index]/100;
	Result-=Result*GetBldParam1(17)/100;
	return Result;
};

int dcPlayer::GetItemGoldPrice(int Item, dcShop * Shop)
{
	clItem It=*(pFirstItem+Item);
	int Result=It.iGoldCost;
	for (int i=0; i<MAX_ITEM_RES; i++)
	{
		if (It.arRes[i]>0)
		{
			Result+=GetResGoldPrice(It.arRes[i]);
			/*if (GetResource(It.arRes[i])==0)
			{
				int Bonuz=(pFirstResource+It.arRes[i])->iGold*CurGame.ResPrices[It.arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};
	Result=Result*Shop->PriceModifier[Num]/100;
	return Result;
};

int dcPlayer::GetItemGemPrice(int Item, dcShop * Shop)
{
	clItem It=*(pFirstItem+Item);
	int Result=It.iGemCost;
	for (int i=0; i<MAX_ITEM_RES; i++)
	{
		if (It.arRes[i]>0)
		{
			Result+=GetResGemPrice(It.arRes[i]);
			/*if (GetResource(It.arRes[i])==0)
			{
				int Bonuz=(pFirstResource+It.arRes[i])->iGem*CurGame.ResPrices[It.arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};
	Result=Result*Shop->PriceModifier[Num]/100;
	return Result;
};

int dcPlayer::GetItemGoldRepPrice(int Item, int Dur, dcShop * Shop)
{
	clItem It=*(pFirstItem+Item);
	int Result=0;
	if (Dur<It.iDurability)
	{
		Result=GetItemGoldPrice(Item,Shop)/2;
		Result=Result*(It.iDurability-Dur)/It.iDurability;		
		if (Result<1)
		{Result=1;};
	};	
	return Result;
};

int dcPlayer::GetItemGemRepPrice(int Item, int Dur, dcShop * Shop)
{
	clItem It=*(pFirstItem+Item);
	int Result=0;
	if (Dur<It.iDurability)
	{
		Result=GetItemGemPrice(Item,Shop)/2;
		Result=Result*(It.iDurability-Dur)/It.iDurability;				
	};	
	return Result;
};

int dcPlayer::GetUnitGoldPrice(int Unit, int Prov)
{
	clUnitType * UN=pFirstUnitType+Unit;
	int Result=UN->iGoldPrice;
	Result-=Result*GetBldParam1(18)/100;
	for (int i=0; i<MAX_UNIT_RES; i++)
	{
		if (UN->arRes[i]>0)
		{
			Result+=GetResGoldPrice(UN->arRes[i]);
			/*if (GetResource(UN->arRes[i])==0)
			{
				int Bonuz=(pFirstResource+UN->arRes[i])->iGold*CurGame.ResPrices[UN->arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};
	ClearMoveMap();
	MovePlayer=Num;
	WalkType=1;			
	WalkSpeed=1;
	MoveMap(Prov,-1,WalkSpeed,0,0);
	int iDistance=*(pMoveMap+ipProvince);
	if ((iDistance<0)||(iDistance>11))
	{iDistance=11;};
	iDistance--;
	Result=Result+Result*iDistance/10;
	return Result;
};

int dcPlayer::GetUnitGemPrice(int Unit, int Prov)
{
	clUnitType * UN=pFirstUnitType+Unit;
	int Result=UN->iGemPrice;
	Result-=Result*GetBldParam1(18)/100;
	for (int i=0; i<MAX_UNIT_RES; i++)
	{
		if (UN->arRes[i]>0)
		{
			Result+=GetResGemPrice(UN->arRes[i]);
			/*if (GetResource(UN->arRes[i])==0)
			{
				int Bonuz=(pFirstResource+UN->arRes[i])->iGem*CurGame.ResPrices[UN->arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};
	ClearMoveMap();
	MovePlayer=Num;
	WalkType=1;			
	WalkSpeed=1;
	MoveMap(Prov,-1,WalkSpeed,0,0);
	int iDistance=*(pMoveMap+ipProvince);
	if ((iDistance<0)||(iDistance>11))
	{iDistance=11;};
	iDistance--;
	Result=Result+Result*iDistance/10;
	return Result;
};

int dcPlayer::GetOutBldGoldPrice(int Building)
{
	clOuterBuilding * BLD=pFirstOuterBld+Building;
	int Result=BLD->iGoldCost;
	for (int i=0; i<MAX_BUILDING_RES; i++)
	{
		if (BLD->arRes[i]>0)
		{
			Result+=GetResGoldPrice(BLD->arRes[i]);
			/*if (GetResource(BLD->arRes[i])==0)
			{
				int Bonuz=(pFirstResource+BLD->arRes[i])->iGold*CurGame.ResPrices[BLD->arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};
	return Result;
};

int dcPlayer::GetOutBldGemPrice(int Building)
{
	clOuterBuilding * BLD=pFirstOuterBld+Building;
	int Result=BLD->iGemCost;
	for (int i=0; i<MAX_BUILDING_RES; i++)
	{
		if (BLD->arRes[i]>0)
		{
			Result+=GetResGemPrice(BLD->arRes[i]);
			/*if (GetResource(BLD->arRes[i])==0)
			{
				int Bonuz=(pFirstResource+BLD->arRes[i])->iGem*CurGame.ResPrices[BLD->arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};	
	return Result;
};

int dcPlayer::GetInBldGoldPrice(int Building)
{
	clInnerBuilding * BLD=pFirstInnerBld+Building;
	int Result=BLD->iGoldCost;
	Result-=Result*GetBldParam1(16)/100;
	for (int i=0; i<MAX_BUILDING_RES; i++)
	{
		if (BLD->arRes[i]>0)
		{
			Result+=GetResGoldPrice(BLD->arRes[i]);
			/*if (GetResource(BLD->arRes[i])==0)
			{
				int Bonuz=(pFirstResource+BLD->arRes[i])->iGold*CurGame.ResPrices[BLD->arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};	
	return Result;
};

int dcPlayer::GetInBldGemPrice(int Building)
{
	clInnerBuilding * BLD=pFirstInnerBld+Building;
	int Result=BLD->iGemCost;
	Result-=Result*GetBldParam1(16)/100;
	for (int i=0; i<MAX_BUILDING_RES; i++)
	{
		if (BLD->arRes[i]>0)
		{
			Result+=GetResGemPrice(BLD->arRes[i]);
			/*if (GetResource(BLD->arRes[i])==0)
			{
				int Bonuz=(pFirstResource+BLD->arRes[i])->iGem*CurGame.ResPrices[BLD->arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};	
	return Result;
};

int dcPlayer::GetDefenderGoldPrice(int Defender)
{
	clDefender * DF=pFirstDefender+Defender;
	int Result=DF->iGoldCost;
	for (int i=0; i<MAX_DEFENDER_RES; i++)
	{
		if (DF->arRes[i]>0)
		{
			Result+=GetResGoldPrice(DF->arRes[i]);
			/*if (GetResource(DF->arRes[i])==0)
			{
				int Bonuz=(pFirstResource+DF->arRes[i])->iGold*CurGame.ResPrices[DF->arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};	
	return Result;
};

int dcPlayer::GetDefenderGemPrice(int Defender)
{
	clDefender * DF=pFirstDefender+Defender;
	int Result=DF->iGemCost;
	for (int i=0; i<MAX_DEFENDER_RES; i++)
	{
		if (DF->arRes[i]>0)
		{
			Result+=GetResGemPrice(DF->arRes[i]);
			/*if (GetResource(DF->arRes[i])==0)
			{
				int Bonuz=(pFirstResource+DF->arRes[i])->iGem*CurGame.ResPrices[DF->arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};	
	return Result;
};

int dcPlayer::GetRitualGoldPrice(int Ritual)
{
	clRitual * RIT=pFirstRitual+Ritual;
	int Result=RIT->iGoldCost;
	Result-=Result*GetBldParam1(7)/100;	
	for (int i=0; i<MAX_RITUAL_RES; i++)
	{
		if (RIT->arRes[i]>0)
		{
			Result+=GetResGoldPrice(RIT->arRes[i]);
			/*if (GetResource(RIT->arRes[i])==0)
			{
				int Bonuz=(pFirstResource+RIT->arRes[i])->iGold*CurGame.ResPrices[RIT->arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};
	return Result;
};

int dcPlayer::GetRitualGemPrice(int Ritual)
{
	clRitual * RIT=pFirstRitual+Ritual;
	int Result=RIT->iGemCost;
	Result-=Result*GetBldParam1(7)/100;
	for (int i=0; i<MAX_RITUAL_RES; i++)
	{
		if (RIT->arRes[i]>0)
		{
			Result+=GetResGemPrice(RIT->arRes[i]);
			/*if (GetResource(RIT->arRes[i])==0)
			{
				int Bonuz=(pFirstResource+RIT->arRes[i])->iGem*CurGame.ResPrices[RIT->arRes[i]]/100;
				Bonuz-=Bonuz*GetBldParam1(17)/100;
				Result+=Bonuz;
			};*/
		};
	};	
	return Result;
};

int dcPlayer::GetHeroGoldPrice(int Class)
{	
	int Numb=0, Collegue=0;
	
	for (int i=0; i<MAX_HERO; i++)
	{
		if (arHero[i])
		{
			Numb++;
			if (arHero[i]->ipClass1==Class)
			{
				Collegue++;
			};
		};
	};
	if (Numb>3)
	{
		Numb=3;
	};
	int Result=arHeroPrice[Numb][0];
	for (int i=0; i<Collegue; i++)
	{
		Result*=iHeroPriceMult;
	};
	Result-=Result*GetBldParam1(13)/100;
	return Result;
};

int dcPlayer::GetHeroGemPrice(int Class)
{	
	int Numb=0, Collegue=0;
	
	for (int i=0; i<MAX_HERO; i++)
	{
		if (arHero[i])
		{
			Numb++;
			if (arHero[i]->ipClass1==Class)
			{
				Collegue++;
			};
		};
	};
	if (Numb>3)
	{
		Numb=3;
	};
	int Result=arHeroPrice[Numb][1];
	for (int i=0; i<Collegue; i++)
	{
		Result*=iHeroPriceMult;
	};
	Result-=Result*GetBldParam1(13)/100;
	return Result;
};

int dcPlayer::GetResurrectGoldPrice(dcHero * Hero)
{
	int Result=arHeroRess[0][0]+Hero->iLevel*arHeroRess[1][0];
	Result-=Result*GetBldParam1(7)/100;	
	return Result;
};

int dcPlayer::GetResurrectGemPrice(dcHero * Hero)
{
	int Result=arHeroRess[0][1]+Hero->iLevel*arHeroRess[1][1];
	Result-=Result*GetBldParam1(7)/100;	
	return Result;
};

int dcPlayer::RecruitHero(int Class)
{
	
	for (int i=0; i<MAX_HERO; i++)
	{
		if (!arHero[i])
		{
			arHero[i]=new dcHero(Num,Class);
			return i;			
		};
	};
	return -1;
};

bool dcPlayer::CanBuild(int Building)
{	
//Уже построена
	if (arBld[Building])
	{return false;};
//Необходимые постройки
	for (int j=0; j<MAX_BUILDING_NEED; j++)
	{
		if ((pFirstInnerBld+Building)->arBuildings[j]>MAX_BLD)
		//Отстроенная группа
		{
			int iGroup=0;
			for (int i=1; i<iMaxInnerBld; i++)
			{					
				if ((arBld[i])&&((pFirstInnerBld+Building)->arBuildings[j]%MAX_BLD==(pFirstInnerBld+i)->iGroup))
				{iGroup++;};
			};
			if (iGroup<arBldGroup[(pFirstInnerBld+Building)->arBuildings[j]%MAX_BLD])
			{
				return false;
			};
		}
		else
		//Обычные
		{
			if (!arBld[(pFirstInnerBld+Building)->arBuildings[j]])
			{return false;};
		};
	};
//Постройки из той же группы
	if ((pFirstInnerBld+Building)->iGroup>0)
	{
		int iGroup=0;
		for (int j=1; j<iMaxInnerBld; j++)
		{					
			if ((arBld[j])&&((pFirstInnerBld+Building)->iGroup==(pFirstInnerBld+j)->iGroup))
			{iGroup++;};
		};
		if (iGroup>=arBldGroup[(pFirstInnerBld+Building)->iGroup])
		{return false;};
	};
//Необходим союз с расой
	for (int i=0; i<MAX_BUILDING_ABILITY; i++)
	{
		if ((pFirstInnerBld+Building)->arAbility[i][0]==47)
		{
			if (iUnion!=(pFirstInnerBld+Building)->arAbility[i][1])
			{return false;};
		};
	};
	return true;
};

int dcPlayer::GetBldParam1(int Ability)
{
	int Result=0;
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if (arBld[i])
		{
			for (int j=0; j<MAX_BUILDING_ABILITY; j++)
			{
				if ((pFirstInnerBld+i)->arAbility[j][0]==Ability)
				{
					Result+=(pFirstInnerBld+i)->arAbility[j][1];
				};
			};
		};
	};
	return Result;
};

int dcPlayer::GetBldParam2(int Ability)
{
	int Result=0;
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if (arBld[i])
		{
			for (int j=0; j<MAX_BUILDING_ABILITY; j++)
			{
				if ((pFirstInnerBld+i)->arAbility[j][0]==Ability)
				{
					Result+=(pFirstInnerBld+i)->arAbility[j][2];
				};
			};
		};
	};
	return Result;
};

int dcPlayer::GetBldParam2(int Ability, int Param1)
{
	int Result=0;
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if (arBld[i])
		{
			for (int j=0; j<MAX_BUILDING_ABILITY; j++)
			{
				if (((pFirstInnerBld+i)->arAbility[j][0]==Ability)&&((pFirstInnerBld+i)->arAbility[j][1]==Param1))
				{
					Result+=(pFirstInnerBld+i)->arAbility[j][2];
				};
			};
		};
	};
	return Result;
};

bool dcPlayer::CompareBldParam1(int Ability, int Index)
{	
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if (arBld[i])
		{
			for (int j=0; j<MAX_BUILDING_ABILITY; j++)
			{
				if (((pFirstInnerBld+i)->arAbility[j][0]==Ability)&&((pFirstInnerBld+i)->arAbility[j][1]==Index))
				{
					return true;
				};
			};
		};
	};
	return false;
};

void dcPlayer::Build(int Choosen)
{
	if ((Choosen>iMaxInnerBld)||(Choosen<=0))
	{
		StopLog("Неправильное строительство:",Choosen);
		return;
	};	
	Gold-=GetInBldGoldPrice(Choosen);
	Gem-=GetInBldGemPrice(Choosen);
	iCanInnerBld--;
	arBld[Choosen]=true;
	for (int i=0; i<MAX_BUILDING_RES; i++)
	{
		if (((pFirstInnerBld+Choosen)->arRes[i]>0)&&
			(!GetResource((pFirstInnerBld+Choosen)->arRes[i])))
		{
			CurGame.ResPrices[(pFirstInnerBld+Choosen)->arRes[i]]+=CurGame.ResBuy;
		};
	};
	int Shopper;
	for (int i=0; i<MAX_BUILDING_ABILITY; i++)
	{					
		switch ((pFirstInnerBld+Choosen)->arAbility[i][0])
		{
		case 1:
			if ((pFirstInnerBld+Choosen)->arAbility[i][1]<2)
			{
				Shopper=0;
				while (Shop.Items[Shopper][0]>0)
				{Shopper++;};
				for (int j=1; j<=iMaxItem; j++)
				{
					if ((pFirstItem+j)->ipBuilding==Choosen)
					{
						Shop.Items[Shopper][0]=j;
						if ((pFirstInnerBld+Choosen)->arAbility[i][1]==0)
						{
							Shop.Items[Shopper][1]=0;
						}
						else
						{
							Shop.Items[Shopper][1]=1;
						};
						Shopper++;
					};
				};
			}
			else
			{
				ClearRoll();
				int Count=0;
				for (int j=1; j<=iMaxItem; j++)
				{
					if (((pFirstItem+j)->iShopLevel==(pFirstInnerBld+Choosen)->arAbility[i][2])&&((pFirstItem+j)->ipClass<10)
						&&((pFirstItem+j)->iRarity>1)&&((pFirstItem+j)->iRarity<=(pFirstInnerBld+Choosen)->arAbility[i][2]))
					{
						RollerN[Count]=j;
						RollerP[Count]=7-(pFirstItem+j)->iRarity;
						Count++;
					};
				};
				Shopper=0;
				while (Shop.Items[Shopper][0]>0)
				{Shopper++;};
				for (int j=0; j<7-(pFirstInnerBld+Choosen)->arAbility[i][2]; j++)
				{
					Shop.Items[Shopper+j][0]=Roll(true);
					Shop.Items[Shopper+j][1]=1;
				};
			};
			break;
		case 2:						
			Shopper=0;
			while (arLibrary[Shopper]>0)
			{Shopper++;};
			for (int j=1; j<=iMaxSpell; j++)
			{
				if ((pFirstSpell+j)->ipBuilding==Choosen)
				{
					arLibrary[Shopper]=j;								
					Shopper++;								
				};
			};
			break;
		case 4:
			(pFirstProvince+ipProvince)->pGarrison->arGarrison[(pFirstInnerBld+Choosen)->arAbility[i][1]-1]=(pFirstInnerBld+Choosen)->arAbility[i][2];
			break;
		case 6:
			if (arRecruit[11]==0)
			{
				AddMercenary();
			};
			break;
		case 8:
			Shopper=0;
			if ((pFirstInnerBld+Choosen)->arAbility[i][2]==0)
			{
				while (arRecruit[Shopper]>0)
				{Shopper++;};
			}
			else
			{Shopper=11;};
			arRecruit[Shopper]=(pFirstInnerBld+Choosen)->arAbility[i][1];
			break;					
		case 11:
			if ((pFirstInnerBld+Choosen)->arAbility[i][2]==0)
			{
				*(pOuterBld+(pFirstInnerBld+Choosen)->arAbility[i][1])=-1;
			}
			else
			{
				*(pOuterBld+(pFirstInnerBld+Choosen)->arAbility[i][1])+=(pFirstInnerBld+Choosen)->arAbility[i][2];
			};						
			break;
		case 15:
			if ((pFirstInnerBld+Choosen)->arAbility[i][2]==0)
			{
				*(pDefender+(pFirstInnerBld+Choosen)->arAbility[i][1])=-1;
			}
			else
			{
				*(pDefender+(pFirstInnerBld+Choosen)->arAbility[i][1])+=(pFirstInnerBld+Choosen)->arAbility[i][2];
			};						
			break;
		case 27:
			if ((pFirstInnerBld+Choosen)->arAbility[i][2]==0)
			{
				*(pRitual+(pFirstInnerBld+Choosen)->arAbility[i][1])=-1;
			}
			else
			{
				*(pRitual+(pFirstInnerBld+Choosen)->arAbility[i][1])+=(pFirstInnerBld+Choosen)->arAbility[i][2];
			};						
			break;
		case 30:
			iTowerShoot+=(pFirstInnerBld+Choosen)->arAbility[i][1];
			iTowerHealth+=(pFirstInnerBld+Choosen)->arAbility[i][2];
			break;					
		case 35:
			(pFirstProvince+ipProvince)->iFort+=(pFirstInnerBld+Choosen)->arAbility[i][1];
			iWall+=(pFirstInnerBld+Choosen)->arAbility[i][1];
			break;
		case 36:
			iTower+=(pFirstInnerBld+Choosen)->arAbility[i][1];
			break;
		case 39:
			iCanHire+=(pFirstInnerBld+Choosen)->arAbility[i][1];
			break;
		case 40:
			iCanOuterBld+=(pFirstInnerBld+Choosen)->arAbility[i][1];
			break;
		case 46:
			iKarma+=(pFirstInnerBld+Choosen)->arAbility[i][1];
			break;
		case 48:
			if (Control==1)
			{
				*(pStory+(pFirstInnerBld+Choosen)->arAbility[i][1])=(pFirstInnerBld+Choosen)->arAbility[i][2];
				*(pPlace+(pFirstInnerBld+Choosen)->arAbility[i][1])=ipProvince;
			};
			break;
		case 50:
			(pFirstProvince+ipProvince)->PlaceSite((pFirstInnerBld+Choosen)->arAbility[i][1],(pFirstInnerBld+Choosen)->arAbility[i][2],0,-1,0);			
			break;
		};				
	};
};

void dcPlayer::AddMercenary()
{
	if (GetBldParam1(6)>0)
	{
		ClearRoll();
		for (int i=0; i<GetBldParam1(6); i++)
		{
			if (i<MAX_MERC_LEVEL)
			{
				for (int j=0; j<MAX_MERCENARY; j++)
				{
					RollerN[i*MAX_MERCENARY+j]=arMercenary[i][j][0];
					RollerP[i*MAX_MERCENARY+j]=arMercenary[i][j][1];
				};
			};
		};
		arRecruit[11]=Roll(false);
	};
};

void dcPlayer::SellItem(int Index, int Dur)
{
	clItem *It=(pFirstItem+Index);
	Gold+=It->iGoldCost/4;
	Gem+=It->iGemCost/4;
	if (It->iDurability>0)
	{
		Gold-=(It->iGoldCost*(It->iDurability-Dur)/It->iDurability/8);
		Gem-=(It->iGemCost*(It->iDurability-Dur)/It->iDurability/8);
	};
};

void dcPlayer::CastRitual(int Index, int Prov, int Choosen)
{
//	int Slot=0;
	clRitual *Rit=(pFirstRitual+Index);
	if (*(pRitual+Index)>0)
	{*(pRitual+Index)-=1;};
	if (Rit->iCooldown-GetBldParam1(53)>0)
	{*(pRitualCD+Index)=Rit->iCooldown-GetBldParam1(53);};
	Gold-=GetRitualGoldPrice(Index);
	Gem-=GetRitualGemPrice(Index);
	iKarma+=Rit->iKarma;
	iCanCast--;
	char Mess[200];
	for (int i=0; i<MAX_RITUAL_RES; i++)
	{
		if ((Rit->arRes[i]>0)&&(!GetResource(Rit->arRes[i])))
		{CurGame.ResPrices[Rit->arRes[i]]+=CurGame.ResBuy;};
	};
	dcProvince *PR=(pFirstProvince+Prov);

	if ((PR->ipPlayer!=Num)&&(PR->GetRitualBonus(1,12)>0))
	{
		if (Control==1)
		{(pFirstDialog+1136)->Show(1,1,0,0,NULL,NULL);}
		else
		{
			sprintf(Mess,"Игрок %s попытался провести ритуал %s в провинции %s, но попал в барьер",GetName(),Rit->sName,(pFirstProvince+Prov)->sName);
			FileLog(Mess);
		};
		return;
	};


	if ((Rit->iTime>0)||(Rit->ipEvent>0))
	{new dcRitual(Index,Num,Prov);};
	
	
	int iVar=0;
	
	if (Rit->GetAbility(20)>0)
	{
		if (Choosen<0)
		{
		int Heroes=0, arHR[MAX_HERO];
		for (int i=0; i<MAX_HERO; i++)
		{
			if ((arHero[i])&&(arHero[i]->iDead<0)&&(arHero[i]->ipProvince==Prov))
			{
				arHR[Heroes]=i;
				Heroes++;
			};
		};
		if (Heroes>1)
		{
			Choosen=-1;
			UI_Mask arMask[MAX_HERO];
			int DD=68;
			UI_TextBox txtChoose;
			BITMAP *bmpBack=create_bitmap(1024,768);
			blit(screen1,bmpBack,0,0,0,0,1024,768);
			BITMAP *bmpWork=create_bitmap(bmpGUI_DefChoose->w,bmpGUI_DefChoose->h-DD+DD*Heroes);
			int x=512-bmpWork->w/2, y=384-bmpWork->h/2;
			G_Object *objMain=new G_Object(NULL,x,y,1,bmpWork);
			objMain=new G_Object(objMain,0,0,0,bmpBack);
			objMain=txtChoose.Init(x+25,y+30,254,90,2,COLOR_1,Font2,objMain);
			txtChoose.Activate((pFirstMessage+27)->msg);
			blit(bmpGUI_DefChoose,bmpWork,0,0,0,0,bmpGUI_DefChoose->w,131);
			for (int i=0; i<Heroes; i++)
			{
				blit(bmpGUI_DefChoose,bmpWork,0,131,0,131+i*DD,bmpGUI_DefChoose->w,DD);
				draw_sprite(bmpWork,arHero[arHR[i]]->smPic,20,141+i*DD);
				textprintf_centre_ex(bmpWork,Font1,180,146+i*DD,COLOR_1,-1,arHero[arHR[i]]->pstName->str);				
				objMain=arMask[i].Init(x+14,y+135+i*DD,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objMain,2);
			};			
			blit(bmpGUI_DefChoose,bmpWork,0,bmpGUI_DefChoose->h-17,0,131+Heroes*DD,bmpGUI_DefChoose->w,17);
			set_mouse_sprite(bmpMousePointer[1]);
			while (Choosen<0)
			{
				for (int i=0; i<Heroes; i++)
				{
					if (arMask[i].Check()==30)
					{
						Choosen=arHR[i];
					};
				};
				if (Timer)
				{
					Timer=false;
					objMain=ShowActiveObjects(objMain,3,true);
				};
			};
			G_Object * Temp;
			while (objMain)
			{
				Temp=objMain->pNext;		
				delete objMain;
				objMain=Temp;
			};
			destroy_bitmap(bmpWork);
			destroy_bitmap(bmpBack);
		}
		else
		{Choosen=arHR[0];};
		};
		if (Control!=1)
		{	sprintf(Mess,"Игрок %s провёл ритуал %s в провинции %s, (цель: герой %s)",GetName(),Rit->sName,(pFirstProvince+Prov)->sName,arHero[Choosen]->pstName->str);
			FileLog(Mess);
		};
		for (int i=0; i<MAX_RITUAL_EFFECT; i++)
		{
			if (Rit->arEffect[i][0]==5)
			{arHero[Choosen]->CallEncounter(Rit->arEffect[i][1],NULL,arHero[Choosen]->pProvince,0,0,0,0);};
		};
	}
	else
	{
		if (Control!=1)
		{	sprintf(Mess,"Игрок %s провёл ритуал %s в провинции %s",GetName(),Rit->sName,(pFirstProvince+Prov)->sName);
			FileLog(Mess);
		};
	};

	//Эффекты ритуала
	for (int i=0; i<MAX_RITUAL_EFFECT; i++)
	{
		switch (Rit->arEffect[i][0])
		{
		case 3: //Изменение отношения к правителю
			PR->StaticEvent(Rit->arEffect[i][1],Num);
		//	PR->ChangeAttitude(Num,Rit->arEffect[i][1]);
			break;
		case 4: //Изменение населения (и получение ресурсов)
			if (PR->iPopulation>0)
			{
				int Dead=PR->iPopulation*Rit->arEffect[i][1]/100;
				PR->iPopulation-=Dead;
				if (Rit->arEffect[i][2]==1)
				{
					Gold+=20+Dead*2;
					iVar=20+Dead*2;
				};
				if (Rit->arEffect[i][2]==2)
				{
					Gem+=10+Dead/3;
					iVar=10+Dead/3;
				};
			};
			break;
		//case 5: //Изменение здоровья воинов
		//	if (PR->pGuard)
		//	{PR->pGuard->HealUnits(Rit->arEffect[i][1]);};
		//	if (PR->pGarrison)
		//	{PR->pGarrison->HealUnits(Rit->arEffect[i][1]);};
		//	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
		//	{
		//		if (arPlayer[PL].bInGame)
		//		{
		//			for (int HR=0; HR<MAX_HERO; HR++)					
		//			{
		//				if ((arPlayer[PL].arHero[HR])&&(arPlayer[PL].arHero[HR]->iDead<0)&&(arPlayer[PL].arHero[HR]->ipProvince==Prov))
		//				{arPlayer[PL].arHero[HR]->HealUnits(Rit->arEffect[i][1]);};
		//			};
		//		};
		//	};
		//	break;
		case 7: //Изменение боевого духа воинов
			{
				int Morale=Rit->arEffect[i][1];
				if (Rit->arEffect[i][2]!=0)
				{
					if (PR->ipPlayer==Num)
					{Morale+=Rit->arEffect[i][2];}
					else
					{Morale-=Rit->arEffect[i][2];};
				};
			if (PR->pGuard)
			{PR->pGuard->ChangeUnitsMorale(Morale);};
			if (PR->pGarrison)
			{PR->pGarrison->ChangeUnitsMorale(Morale);};
			for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
			{
				if (arPlayer[PL].bInGame)
				{
					for (int HR=0; HR<MAX_HERO; HR++)					
					{
						if ((arPlayer[PL].arHero[HR])&&(arPlayer[PL].arHero[HR]->iDead<0)&&(arPlayer[PL].arHero[HR]->ipProvince==Prov))
						{arPlayer[PL].arHero[HR]->ChangeUnitsMorale(Morale);};
					};
				};
			};
			};
			break;
		case 9: //Вывод диалога
			if (Rit->arEffect[i][2]==1)
			{
				char Mess[500];
				sprintf(Mess,(pFirstDialog+Rit->arEffect[i][1])->sText,iVar);
				(pFirstDialog+Rit->arEffect[i][1])->Show(true,false,false,false,NULL,Mess);
			}
			else
			{(pFirstDialog+Rit->arEffect[i][1])->Show(true,false,false,false,NULL,NULL);};
			break;
		case 11: //Нанесение вреда укреплениям и уничтожение постройки
			if (PR->iFort>0)
			{PR->iFort-=Rit->arEffect[i][1];};
			if (PR->iFort<0)
			{PR->iFort=0;};
			{
				int Bld=-1;
				for (int j=0; j<MAX_PROVINCE_BLD; j++)
				{
					if ((PR->arBuilding[j]>0)&&((pFirstOuterBld+PR->arBuilding[j])->iWall==0)&&((Bld<0)||(Random(100)>50)))
					{Bld=j;};
				};
				if (Bld>=0)
				{PR->arBuilding[Bld]=0;};
			};
			break;
		case 13: //Повышение недовольства до максимума
			PR->arUnrest[PR->ipPlayer]=500;
			break;
		case 14: //Изменение недовольства
			{
				int Unrest=Rit->arEffect[i][1]+Random(Rit->arEffect[i][2]);
				if (Unrest>0)
				{
					PR->arUnrest[PR->ipPlayer]-=Unrest;
					if (PR->arUnrest[PR->ipPlayer]<0)
					{PR->arUnrest[PR->ipPlayer]=0;};
				}
				else
				{
					PR->arUnrest[PR->ipPlayer]+=abs(Unrest)*100/(100+PR->GetUnrestSlowdown());
					if (PR->arUnrest[PR->ipPlayer]>500)
					{PR->arUnrest[PR->ipPlayer]=500;};
				};
			};
			break;
		};
	};
};

BITMAP* dcPlayer::GetAvatar()
{
	if (Control==2)
	{return bmpAvatar[ipProfile-1];};
	if (Control==1)
	{
		if (CurGame.iGameType==2)
		{
			if (Tutorial.Map)
			{return bmpAvatar[17];}
			else
			{return bmpAvatar[18];};
		}
		else
		{return bmpAvatar[arHumans[iColor].ipAvatar];};
	};
	return bmpAvatar[16];
};

BITMAP* dcPlayer::GetSmallAvatar()
{
	if (Control==2)
	{return bmpSmallAvatar[ipProfile-1];};
	if (Control==1)
	{return bmpSmallAvatar[arHumans[iColor].ipAvatar];};
	return bmpSmallAvatar[16];
};

char* dcPlayer::GetName()
{
	if (Control==2)
	{return (pFirstPlayerProfile+ipProfile)->sName;};
	if (Control==1)
	{return arHumans[iColor].sName;};
	return (pFirstPhrase+323)->str;
};

void dcPlayer::ReturnHero(int NN)
{
	for (int i=0; i<MAX_NEIGHBOUR; i++)
	{
		if ((arHero[NN]->pProvince->arNeighbour[i]>0)&&((pFirstProvince+arHero[NN]->pProvince->arNeighbour[i])->ipPlayer==Num))
		{
			arHero[NN]->ipProvince=arHero[NN]->pProvince->arNeighbour[i];
			arHero[NN]->pProvince=(pFirstProvince+arHero[NN]->pProvince->arNeighbour[i]);
			return;
		};
	};
	arHero[NN]->ipProvince=ipProvince;
	arHero[NN]->pProvince=(pFirstProvince+ipProvince);
	return;
};

void dcPlayer::MessageEntry(int Sender, int Answer)
{
	dcMessage Mess=arPlayer[Sender].Politic[Num].Message;
	//Политический договор
	if (Mess.Treaty>0)
	{
		if (Mess.Treaty<4)
		{
			arPlayer[Sender].Politic[Num].arTalk[Mess.Treaty][1]=Answer;
			arPlayer[Sender].Politic[Num].arTalk[Mess.Treaty][2]=CurGame.iCurrentTurn;
		};
	};
	if ((Mess.Type==2)||(Mess.Type==3))
	{
		if (Mess.Trade.IsFree())
		{
			arPlayer[Sender].Politic[Num].arTalk[6][1]=Answer;
			arPlayer[Sender].Politic[Num].arTalk[6][2]=CurGame.iCurrentTurn;
		}
		else
		{
			if (Mess.Trade1.IsFree())
			{
				arPlayer[Sender].Politic[Num].arTalk[7][1]=Answer;
				arPlayer[Sender].Politic[Num].arTalk[7][2]=CurGame.iCurrentTurn;
			}
			else
			{
				arPlayer[Sender].Politic[Num].arTalk[7-Mess.Type][1]=Answer;
				arPlayer[Sender].Politic[Num].arTalk[7-Mess.Type][2]=CurGame.iCurrentTurn;
			};
		};
	};
};

void dcPlayer::AcceptDeal(int Sender)
{
	MessageEntry(Sender,1);
	dcMessage *Mess=&arPlayer[Sender].Politic[Num].Message;
	//Политический договор
	if (Mess->Treaty>0)
	{
		if (Mess->Treaty==2)
		{
			for (int i=1; i<=CurGame.iMaxPlayers; i++)
			{
				if ((i!=Num)&&(i!=Sender)&&(arPlayer[i].bInGame))
				{
					if ((Politic[i].Treaty==3)&&(arPlayer[i].Politic[Sender].Treaty==1))
					{
						arPlayer[i].Politic[Sender].Treaty=2;
						arPlayer[Sender].Politic[i].Treaty=2;
					};
					if ((arPlayer[Sender].Politic[i].Treaty==3)&&(Politic[i].Treaty==1))
					{
						arPlayer[i].Politic[Num].Treaty=2;
						Politic[i].Treaty=2;
						Politic[i].arTalk[2][2]=CurGame.iCurrentTurn;
					};
				};
			};
		};
		if (Mess->Treaty==4)
		{Mess->Treaty=2;};
		Politic[Sender].Treaty=Mess->Treaty;
		Politic[Sender].arTalk[Mess->Treaty][2]=CurGame.iCurrentTurn;
		Politic[Sender].arTalk[Mess->Treaty][1]=1;
		arPlayer[Sender].Politic[Num].Treaty=Mess->Treaty;
		if (Mess->Treaty==1)
		{DeclareWar(Sender);};
	};
	switch (Mess->Type)
	{
	//Торговый договор
	case 2:
		Politic[Sender].Trade=Mess->Trade1;
		Politic[Sender].Trade.bTrade=true;
		arPlayer[Sender].Politic[Num].Trade=Mess->Trade;
		arPlayer[Sender].Politic[Num].Trade.bTrade=true;
//		arPlayer[Sender].Politic[Num].arTalk[5][2]=CurGame.iCurrentTurn;
		arPlayer[Sender].Politic[Num].arTalk[5][1]=1;
		break;
	//Обмен
	case 3:
		arPlayer[Sender].Gold+=Mess->Trade1.Gold;
		Gold-=Mess->Trade1.Gold;
		arPlayer[Sender].Gold-=Mess->Trade.Gold;
		Gold+=Mess->Trade.Gold;
		arPlayer[Sender].Gem+=Mess->Trade1.Gem;
		Gem-=Mess->Trade1.Gem;
		arPlayer[Sender].Gem-=Mess->Trade.Gem;
		Gem+=Mess->Trade.Gem;
		for (int i=0; i<8; i++)
		{
			if (Mess->Trade.Stuff[i]>0)
			{
				(pFirstProvince+Mess->Trade.Stuff[i])->ipPlayer=Num;
				if ((pFirstProvince+Mess->Trade.Stuff[i])->pGarrison)
				{(pFirstProvince+Mess->Trade.Stuff[i])->pGarrison->ipPlayer=Num;};
				if ((pFirstProvince+Mess->Trade.Stuff[i])->pGuard)
				{(pFirstProvince+Mess->Trade.Stuff[i])->pGuard->ipPlayer=Num;};
			};
			if (Mess->Trade1.Stuff[i]>0)
			{
				(pFirstProvince+Mess->Trade1.Stuff[i])->ipPlayer=Sender;
				if ((pFirstProvince+Mess->Trade1.Stuff[i])->pGarrison)
				{(pFirstProvince+Mess->Trade1.Stuff[i])->pGarrison->ipPlayer=Sender;};
				if ((pFirstProvince+Mess->Trade1.Stuff[i])->pGuard)
				{(pFirstProvince+Mess->Trade1.Stuff[i])->pGuard->ipPlayer=Sender;};
			};
			if (Mess->Trade.Stuff[i+8]>0)
			{
				int Item=-1;
				for (int j=0; j<TREASURY_SIZE; j++)
				{
					if ((arPlayer[Sender].arTreasury[j][0]==Mess->Trade.Stuff[i+8])&&
						(arPlayer[Sender].arTreasury[j][1]==Mess->arDurability[i]))
					{
						Item=j;
					};
				};
				if (Item>=0)
				{
					arPlayer[Sender].arTreasury[Item][0]=0;
					arPlayer[Sender].arTreasury[Item][1]=0;
					Item=-1;
					for (int j=0; ((j<TREASURY_SIZE)&&(Item<0)); j++)
					{
						if (arTreasury[j][0]==0)
						{
							Item=j;
							arTreasury[j][0]=Mess->Trade.Stuff[i+8];
							arTreasury[j][1]=Mess->arDurability[i];
						};
					};
				};				
			};
		};
		break;
	//Отмена торговли
	case 4:
		Politic[Sender].Trade.Clear();
		arPlayer[Sender].Politic[Num].Trade.Clear();
		break;
	//Жребий
	case 7:
		(pFirstDialog+1122)->Show(1,1,0,0,NULL,NULL);
		if (Random(100)<50)
		{
			(pFirstDialog+1124)->Show(1,1,0,0,NULL,NULL);
			Lose(Num);
		}
		else
		{
			(pFirstDialog+1123)->Show(1,1,0,0,NULL,NULL);
			Victory(Num);
		};
		break;
	//Сдаться
	case 8:
		arPlayer[Sender].Politic[Num].Attitude+=10;
		Lose(Num);
		break;
	//Победа
	case 9:
		Victory(Num);
		break;
	};
	if (Politic[Sender].Treaty>1)
	{
		for (int i=0; i<MAX_HERO; i++)
		{
			if ((arHero[i])&&(arHero[i]->iDead<0)&&(arHero[i]->pProvince->ipPlayer==Sender))
			{
				ReturnHero(i);
			};
			if ((arPlayer[Sender].arHero[i])&&(arPlayer[Sender].arHero[i]->iDead<0)&&(arPlayer[Sender].arHero[i]->pProvince->ipPlayer==Num))
			{
				arPlayer[Sender].ReturnHero(i);
			};
		};
	};
	arPlayer[Sender].Politic[Num].Message.Clear();
};

void dcPlayer::CancelDeal(int Sender)
{
	MessageEntry(Sender,0);
	//Объявление войны при отказе в мирном разрешении присвоения осколка с союзником
	if ((arPlayer[Sender].Politic[Num].Message.Type==8)||(arPlayer[Sender].Politic[Num].Message.Type==7))
	{
		DeclareWar(Sender);
	};
	arPlayer[Sender].Politic[Num].Message.Clear();
};

int dcPlayer::GetWars()
{
	int Result=0;
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if ((i!=Num)&&(arPlayer[i].bInGame)&&(Politic[i].Treaty==1))
		{Result++;};
	};
	return Result;
};

int dcPlayer::GetUnionCount()
{
	int Result=0;
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if ((i!=Num)&&(arPlayer[i].bInGame)&&(Politic[i].Treaty==3))
		{Result++;};
	};
	return Result;
};

void dcPlayer::DeclareWar(int Enemy)
{
	Politic[Enemy].Treaty=1;
	Politic[Enemy].Trade.Clear();
	Politic[Enemy].arTalk[1][2]=CurGame.iCurrentTurn;
	arPlayer[Enemy].Politic[Num].Treaty=1;
	arPlayer[Enemy].Politic[Num].Trade.Clear();
};

int dcPlayer::GetAttitude(int Victim)
{
	int Result=Politic[Victim].Attitude;
	if (iKarma>=20)
	{		
		Result+=(arPlayer[Victim].iKarma/10)*(iKarma/10)/5;
	};
	if (iKarma<=-20)
	{		
		Result+=(arPlayer[Victim].iKarma/10)*(iKarma/10)/10;
	};
	if ((abs(arPlayer[Victim].iKarma)<20)&&(abs(iKarma)<20))
	{
		Result+=5-abs(arPlayer[Victim].iKarma-iKarma)/10;
	};	
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if ((i!=Num)&&(i!=Victim)&&(arPlayer[i].bInGame))
		{
			Result+=(arPlayer[i].Politic[Victim].Attitude/10)*(Politic[i].Attitude/50);
			if (arPlayer[i].Politic[Victim].Treaty==1)
			{Result-=Politic[i].Attitude/5;};
			if (arPlayer[i].Politic[Victim].Treaty==3)
			{Result+=Politic[i].Attitude/5;};
		};
	};
	Result+=arPlayer[Victim].GetBldParam1(43);
	return Result;
};

bool dcPlayer::CheckMessage(int Victim)
{
	if (Politic[Victim].Message.Type==5)
	{return true;};
	if (Politic[Victim].Message.Type>0)
	{
		if ((Politic[Victim].Treaty==1)&&(Politic[Victim].Message.Treaty!=2))
		{return false;};		
		if (!Politic[Victim].Message.Trade.IsLegal(Num,Politic[Victim].Message.Type))
		{return false;};
		if (!Politic[Victim].Message.Trade1.IsLegal(Victim,Politic[Victim].Message.Type))
		{return false;};
	};
	return true;
};

int GetCurPlayers()
{
	int Result=0;
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if (arPlayer[i].bInGame)
		{Result++;};
	};
	return Result;
};

void DrawHerb(int Col1, int Col2, int Sign, int x, int y, BITMAP *Target)
{	
	draw_sprite(Target,bmpHerbBorder,x,y);
	set_trans_blender(arHerbColors[Col1][0],arHerbColors[Col1][1],arHerbColors[Col1][2],0);			
	draw_lit_sprite(Target,bmpHerbMain,x,y,255);
	set_trans_blender(arHerbColors[Col2][0],arHerbColors[Col2][1],arHerbColors[Col2][2],0);
	draw_lit_sprite(Target,bmpHerbSign[Sign],x,y,255);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(Target,bmpHerbMain,x,y);
};

void DrawFlag(int Col1, int Col2, int x, int y, BITMAP *Target)
{	
	draw_sprite(Target,bmpFlagMain,x,y);
	set_trans_blender(arHerbColors[Col1][0],arHerbColors[Col1][1],arHerbColors[Col1][2],0);			
	draw_lit_sprite(Target,bmpFlagCol1,x,y,255);
	set_trans_blender(arHerbColors[Col2][0],arHerbColors[Col2][1],arHerbColors[Col2][2],0);
	draw_lit_sprite(Target,bmpFlagCol2,x,y,255);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(Target,bmpFlagMain,x,y);
};

void dcPlayer::ShowHerb(int x, int y, BITMAP *Target)
{
	if (Control==1)
	{
		DrawHerb(arHumans[iColor].iColor1,arHumans[iColor].iColor2,arHumans[iColor].iSign,x,y,Target);
	};
	if (Control==2)
	{
		DrawHerb((pFirstPlayerProfile+ipProfile)->iColor1,(pFirstPlayerProfile+ipProfile)->iColor2,(pFirstPlayerProfile+ipProfile)->iSign,x,y,Target);
	};
	if (Control==3)
	{
		DrawHerb(iColor-1,iColor-1,0,x,y,Target);
	};
};

void dcPlayer::ShowFlag(int x, int y, BITMAP *Target)
{
	if (Control==1)
	{
		DrawFlag(arHumans[iColor].iColor1,arHumans[iColor].iColor2,x,y,Target);
	};
	if (Control==2)
	{
		DrawFlag((pFirstPlayerProfile+ipProfile)->iColor1,(pFirstPlayerProfile+ipProfile)->iColor2,x,y,Target);
	};
	if (Control==3)
	{
		DrawFlag(iColor-1,iColor-1,x,y,Target);
	};
};

void dcPlayer::MakeFlags()
{
//	if (!BF_Cheat)
	{
	ShowHerb(0,0,bmpHerb[iColor]);
	ShowFlag(0,0,bmpFlag[iColor][0]);
	draw_sprite(bmpFlag[iColor][1],bmpTownFlag[0][0],0,0);
	draw_sprite(bmpFlag[iColor][2],bmpTownFlag[1][0],0,0);
	draw_sprite(bmpFlag[iColor][3],bmpTownFlag[2][0],0,0);
	int Col1, Col2, Hrb;
	if (Control==1)
	{
		Col1=arHumans[iColor].iColor1;
		Col2=arHumans[iColor].iColor2;
		Hrb=arHumans[iColor].iSign;
	}
	if (Control==2)
	{
		Col1=(pFirstPlayerProfile+ipProfile)->iColor1;
		Col2=(pFirstPlayerProfile+ipProfile)->iColor2;
		Hrb=(pFirstPlayerProfile+ipProfile)->iSign;
	};
	if (Control==3)
	{
		Col1=iColor-1;
		Col2=iColor-1;
		Hrb=0;
	};
	set_trans_blender(arHerbColors[Col1][0],arHerbColors[Col1][1],arHerbColors[Col1][2],0);			
	draw_lit_sprite(bmpFlag[iColor][1],bmpTownFlag[0][1],0,0,255);
	draw_lit_sprite(bmpFlag[iColor][2],bmpTownFlag[1][1],0,0,255);
	draw_lit_sprite(bmpFlag[iColor][3],bmpTownFlag[2][1],0,0,255);
	set_trans_blender(arHerbColors[Col2][0],arHerbColors[Col2][1],arHerbColors[Col2][2],0);
	draw_lit_sprite(bmpFlag[iColor][1],bmpHerbSignS[Hrb],50,15,255);
	draw_lit_sprite(bmpFlag[iColor][2],bmpTownFlag[1][2],0,0,255);
	draw_lit_sprite(bmpFlag[iColor][3],bmpTownFlag[2][2],0,0,255);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpFlag[iColor][1],bmpTownFlag[0][1],0,0);
	draw_trans_sprite(bmpFlag[iColor][2],bmpTownFlag[1][1],0,0);
	draw_trans_sprite(bmpFlag[iColor][3],bmpTownFlag[2][1],0,0);
	};
};

//Возвращает истину, если этот герой наибольшего уровня у данного игрока
bool dcPlayer::IsBestHero(int Hero)
{
	if (!arHero[Hero])
	{return false;};
	for (int i=0; i<MAX_HERO; i++) 
	{
		if ((i!=Hero)&&(arHero[i])&&(arHero[i]->iLevel>arHero[Hero]->iLevel))
		{return false;};
	};
	return true;
};

void dcPlayer::CheckCampQuest(int ipProv, int ipSite)
{
	for (int i=0; i<MAX_CAMP_QUEST; i++)
	{
	if ((Control==1)&&(CurGame.iGameType==2)&&(Campaign.arQuest[i][0]>0)&&(Campaign.arQuest[i][1]==1))
	{

		if (Campaign.arQuest[i][2]==0)
		{
			if ((pFirstQuest+Campaign.arQuest[i][0])->Target[0]==(pFirstProvince+ipProv)->arSite[ipSite][0])
			{
				if (Random(100)<(pFirstQuest+Campaign.arQuest[i][0])->iPossibility)
				{
					if ((pFirstQuest+Campaign.arQuest[i][0])->ipEncSuccess)
					{
						(pFirstProvince+ipProv)->StaticEvent((pFirstQuest+Campaign.arQuest[i][0])->ipEncSuccess,Num);
					};
					Campaign.arQuest[i][1]=2;
				}
				else
				{
					if ((pFirstQuest+Campaign.arQuest[i][0])->ipEncNotFound)
					{
						(pFirstProvince+ipProv)->StaticEvent((pFirstQuest+Campaign.arQuest[i][0])->ipEncNotFound,Num);						
					};
				};

			};
		}
		else
		{
			if ((Campaign.arQuest[i][2]==ipProv)&&(Campaign.arQuest[i][3]==ipSite))
			{
				if ((pFirstQuest+Campaign.arQuest[i][0])->ipEncSuccess)
				{
					(pFirstProvince+ipProv)->StaticEvent((pFirstQuest+Campaign.arQuest[i][0])->ipEncSuccess,Num);
				};
				Campaign.arQuest[i][1]=2;
			};
		};

	};
	};
};