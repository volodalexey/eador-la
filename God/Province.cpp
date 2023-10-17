#include "Unit.h"
int iMaxShop;
dcShop *pFirstShop;

//Заполняет таблицу вероятностей для магазина типа iShop предметами уровня не выше iLevel (Если bEqual истинно, то только указанного уровня)
//iShop: 1, 2, 9 Оружие и щиты, 3 Луки и стрелы, 4 Жезлы и сферы, 5 Знамёна, 6 кожа и пояса, 7 Кольчуги, 8 Латы, 10 Свитки, -1 Одежда, -2 Украшения
int ShopRoller(int iShop, int iLevel, int iRare, bool bEqual)
{
	int CN=0;
	bool bAdd;
	ClearRoll();
	for (int k1=1; k1<=iMaxItem; k1++)
	{
		bAdd=false;
		switch (iShop)
		{						
		case 3:
		case 4:
		case 5:
		case 7:
		case 8:
		case 10:
		case 11:
		case 12:
		case 13:
			if (((pFirstItem+k1)->ipClass==iShop)&&((pFirstItem+k1)->iShopLevel<=iLevel))
			{bAdd=true;};
			break;
		case 6:
			if ((((pFirstItem+k1)->ipClass==iShop)||((pFirstItem+k1)->ipSlot==5))&&((pFirstItem+k1)->iShopLevel<=iLevel))
			{bAdd=true;};
			break;
		case 1:
		case 2:
		case 9:
			if ((((pFirstItem+k1)->ipClass==1)||((pFirstItem+k1)->ipClass==2)||((pFirstItem+k1)->ipClass==9))&&((pFirstItem+k1)->iShopLevel<=iLevel))
			{bAdd=true;};			
			break;
		case -1:
			if (((pFirstItem+k1)->ipClass==0)&&(((pFirstItem+k1)->ipSlot==3)||((pFirstItem+k1)->ipSlot==4)||((pFirstItem+k1)->ipSlot==6)||((pFirstItem+k1)->ipSlot==7)||((pFirstItem+k1)->ipSlot==8)||(((pFirstItem+k1)->ipSlot==10)&&((pFirstItem+k1)->iDurability>1)))&&((pFirstItem+k1)->iShopLevel<=iLevel))
			{bAdd=true;};			
			break;
		case -2:
			if (((pFirstItem+k1)->ipClass==0)&&(((pFirstItem+k1)->ipSlot==9)||(((pFirstItem+k1)->ipSlot==10)&&((pFirstItem+k1)->iDurability==1))||((pFirstItem+k1)->ipSlot==11))&&((pFirstItem+k1)->iShopLevel<=iLevel))
			{bAdd=true;};
			break;					
		};
		if (((pFirstItem+k1)->iRarity>=iRare)&&(bAdd)&&(((pFirstItem+k1)->iShopLevel==iLevel)||(!bEqual)))
		{
			RollerN[CN]=k1;
			RollerP[CN]=8-(pFirstItem+k1)->iRarity;
			CN++;
		};
	};
	return CN;
}
dcProvince::dcProvince()
{
	pGuard=NULL;
	pGarrison=NULL;
	for (int i=0; i<=MAX_PROVINCE_BLD; i++)
	{
		arBuilding[i]=0;
	};


	Pic=NULL;
	TownPic=NULL;
	Obj=NULL;
	for (int i=0; i<MAX_PROVINCE_BLD; i++)
	{
		BuildPic[i]=NULL;
	};
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		for (int j=0; j<6; j++)
		{arSite[i][j]=0;};
		arSiteGuard[i]=NULL;
	};
	for (int i=0; i<MAX_PLAYERS; i++)
	{
		arAttitude[i][0]=0;
		arUnrest[i]=0;
	};
	for (int i=0; i<MAX_NEIGHBOUR; i++)
	{
		arNeighbour[i]=0;
	};
	iPopulation=0;
	iDefender=0;
	iFort=0;
	ipType=0;
	ipEvent=0;
	bWarning=true;
	Work=0;
	ipPlayer=0;
	iConquered=0;
	iSize=100;
	ipRace=1;
	iExplore=1;
	PicX=0; PicY=0;

};

dcProvince::~dcProvince()
{
	if (bGameInProcess)
	{
	if (pGuard)
	{
		delete pGuard;
	};
	if (pGarrison)
	{
		delete pGarrison;
	};
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if (arSiteGuard[i])
		{
			delete arSiteGuard[i];
		};
	};
	
	if (Pic)
	{destroy_bitmap(Pic);}
	
	delete [] sName;
	};
};

void dcProvince::PlaceGuard(int Guardian, int Karma)
{
	clGuard * Grd;
	Grd=(pFirstGuard+Guardian);
	if (pGuard)
	{DismissDefender();};
	pGuard = new dcHero;
	pGuard->iExp=Guardian;			//Сохраняем тип охранника для выдачи трофеев
	pGuard->ipName=Karma;
	pGuard->iTask=Grd->Quick+Random(Grd->QuickAdd+1); //Инициатива
	for (int i=0; i<Grd->Num; i++)
	{

		int UType=0;
		int Count=Grd->arUnit[UType][2];
		while (i>=Count)
		{
			UType++;
			Count+=Grd->arUnit[UType][2];
		};
		int Lvl=Grd->arUnit[UType][1]+Random(6)-3;
		if (Lvl<0)
		{Lvl=0;};
		if (Lvl>UNIT_LEVEL_CAP)
		{Lvl=UNIT_LEVEL_CAP;}
		pGuard->pSquad[i]=new dcUnit(Grd->arUnit[UType][0],GetUnitExp(Grd->arUnit[UType][0],Lvl));
		pGuard->pSquad[i]->x=(i-(i%8))/8;
		pGuard->pSquad[i]->y=i%8;
	};

	int Add=Random(Grd->NumAdd+1);
	for (int i=Grd->Num; i<Grd->Num+Add; i++)
	{
		ClearRoll();
		for (int j=0; j<MAX_GUARD_UNIT; j++)
		{
			if (Grd->arUnit[j][0]>0)
			{
				RollerN[j]=j+1;
				RollerP[j]=Grd->arUnit[j][3];
			};
		};
		int UType=Roll(false)-1;
		int Lvl=Grd->arUnit[UType][1]+Random(6)-3;
		if (Lvl<0)
		{Lvl=0;};
		if (Lvl>UNIT_LEVEL_CAP)
		{Lvl=UNIT_LEVEL_CAP;}
		pGuard->pSquad[i]=new dcUnit(Grd->arUnit[UType][0],GetUnitExp(Grd->arUnit[UType][0],Lvl));
//		pGuard->pSquad[i]->iExp=pGuard->pSquad[i]->ExpForLevel(Lvl);
//		pGuard->pSquad[i]->iLevel=0;
/*		while (pGuard->pSquad[i]->iLevel<pGuard->pSquad[i]->CountLevel())
		{
			pGuard->pSquad[i]->MakeLevel(pGuard->pSquad[i]->LevelUp(pGuard->pSquad[i]->iLevel+1,92));
		};
*/
		pGuard->pSquad[i]->x=(i-(i%8))/8;
		pGuard->pSquad[i]->y=i%8;
	};	
};

void dcProvince::DismissDefender()
{
	/*
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if (pGuard->pSquad[i])
		{
			delete pGuard->pSquad[i];
		};
	};
	*/
	if (pGuard)
	{delete pGuard;};
	pGuard=NULL;
	iDefender=0;
};

void dcProvince::PlaceDefender(int Defender)
{
	if (iDefender>0)
	{
		DismissDefender();
	};
	clDefender * Def=(pFirstDefender+Defender);
	pGuard = new dcHero;
	pGuard->ipPlayer=ipPlayer;
	iDefender=Defender;
	pGuard->iTask=Def->iInitiative;
	pGuard->iExp=-Defender;			//Сохраняем тип стража для выдачи трофеев (отрицательный для стражей, положительный для охранников)
	for (int i=0; i<MAX_GUARD_UNIT; i++)
	{
		int UType=0;		
		int Count=Def->arUnit[UType][2];
		while ((i>=Count)&&(Def->arUnit[UType][0]>0)&&(UType<MAX_GUARD_UNIT-1))
		{
			UType++;
			Count+=Def->arUnit[UType][2];
		};
		if (Def->arUnit[UType][0]>0)
		{
			int Lvl=Def->arUnit[UType][1]+Random(6)-3;
			if (Lvl<0)
			{Lvl=0;};
			if (Lvl>UNIT_LEVEL_CAP)
			{Lvl=UNIT_LEVEL_CAP;}
			int Expoz=GetUnitExp(Def->arUnit[UType][0],Lvl);
			if (ipPlayer>0)
			{
				Expoz+=arPlayer[ipPlayer].GetBldParam1(20);
			};
			pGuard->pSquad[i]=new dcUnit(Def->arUnit[UType][0],Expoz);
			pGuard->pSquad[i]->x=(i-(i%8))/8;
			pGuard->pSquad[i]->y=i%8;
		};
	};
};

void dcProvince::DismissSiteGuard(int Site)
{
	delete arSiteGuard[Site];
	arSiteGuard[Site]=NULL;
	arSite[Site][4]=0;
};

void dcProvince::PlaceSiteGuard(int Guardian, int Site, int Karma)
{
	clGuard * Grd;
	Grd=(pFirstGuard+Guardian);
	if (arSiteGuard[Site])
	{DismissSiteGuard(Site);};
	arSiteGuard[Site] = new dcHero;
	arSiteGuard[Site]->iExp=Guardian;			//Сохраняем тип стража для выдачи трофеев
//	arSiteGuard[Site]->ipName=Karma;
	arSiteGuard[Site]->iTask=Grd->Quick+Random(Grd->QuickAdd+1);
	for (int i=0; i<Grd->Num; i++)
	{

		int UType=0;
		int Count=Grd->arUnit[UType][2];
		while (i>=Count)
		{
			UType++;
			Count+=Grd->arUnit[UType][2];
		};
		int Lvl=Grd->arUnit[UType][1]+Random(6)-3;
		if (Lvl<0)
		{Lvl=0;};
		if (Lvl>UNIT_LEVEL_CAP)
		{Lvl=UNIT_LEVEL_CAP;}
		arSiteGuard[Site]->pSquad[i]=new dcUnit(Grd->arUnit[UType][0],GetUnitExp(Grd->arUnit[UType][0],Lvl));
	//	arSiteGuard[Site]->pSquad[i]->iExp=arSiteGuard[Site]->pSquad[i]->ExpForLevel(Lvl);
	//	arSiteGuard[Site]->pSquad[i]->iLevel=0;
		arSiteGuard[Site]->pSquad[i]->x=(i-(i%8))/8;
		arSiteGuard[Site]->pSquad[i]->y=i%8;

	/*	while (arSiteGuard[Site]->pSquad[i]->iLevel<arSiteGuard[Site]->pSquad[i]->CountLevel())
		{
			arSiteGuard[Site]->pSquad[i]->MakeLevel(arSiteGuard[Site]->pSquad[i]->LevelUp(arSiteGuard[Site]->pSquad[i]->iLevel+1,92));
		};
*/
	};

	int Add=Random(Grd->NumAdd+1);
	for (int i=Grd->Num; i<Grd->Num+Add; i++)
	{
		ClearRoll();
		for (int j=0; j<MAX_GUARD_UNIT; j++)
		{
			if (Grd->arUnit[j][0]>0)
			{
				RollerN[j]=j+1;
				RollerP[j]=Grd->arUnit[j][3];
			};
		};
		int UType=Roll(false)-1;
		int Lvl=Grd->arUnit[UType][1]+Random(6)-3;
		if (Lvl<0)
		{Lvl=0;};
		if (Lvl>UNIT_LEVEL_CAP)
		{Lvl=UNIT_LEVEL_CAP;}
		arSiteGuard[Site]->pSquad[i]=new dcUnit(Grd->arUnit[UType][0],GetUnitExp(Grd->arUnit[UType][0],Lvl));
//		arSiteGuard[Site]->pSquad[i]->iExp=arSiteGuard[Site]->pSquad[i]->ExpForLevel(Lvl);
//		arSiteGuard[Site]->pSquad[i]->iLevel=0;
/*		while (arSiteGuard[Site]->pSquad[i]->iLevel<arSiteGuard[Site]->pSquad[i]->CountLevel())
		{
			arSiteGuard[Site]->pSquad[i]->MakeLevel(arSiteGuard[Site]->pSquad[i]->LevelUp(arSiteGuard[Site]->pSquad[i]->iLevel+1,92));
		};
*/
		arSiteGuard[Site]->pSquad[i]->x=(i-(i%8))/8;
		arSiteGuard[Site]->pSquad[i]->y=i%8;
	};	
};

int dcProvince::GetLevel()
{
	for (int i=1; i<MAX_PROVINCE_LEVEL; i++)
	{
		if (iPopulation<=ProvLevel[i].iPopulation)
		{
			return i;
		};
	};
	return MAX_PROVINCE_LEVEL;
};

int dcProvince::GetResource(int Status)
{
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if (arSite[i][0]>0)
		{
			for (int j=0; j<MAX_SITE_ABILITY; j++)
			{
				if ((pFirstSite+arSite[i][0])->arAbility[j][0]==1)
				{
					if (Status<0)
					{
						return (pFirstSite+arSite[i][0])->arAbility[j][1];
					}
					else
					{
						if ((arSite[i][1]<=iExplore)&&((Status==0)||(arSite[i][4]==0)))
						{
							return (pFirstSite+arSite[i][0])->arAbility[j][1];
						};
					};
				};
			};
		};
	};
	return 0;
};

int dcProvince::GetGoldIncome()
{
	if ((ipTerrain==WATER_ID)||(ipTerrain==MOUNTAIN_ID))
	{return 0;};
	int Result=iGoldInc;
	if (Result<=0)
	{Result=1;};
//Уровень провинции
	Result+=Result*ProvLevel[GetLevel()].iIncome/100;
	if (Result==0)
	{return 0;};
	Result=Result*(pFirstRace+ipRace)->iGold/100;
//Настроение жителей
	Result+=Result*GetMood()/20;
//Исследованная территория
	Result-=Result*(100-iExplore)/500;
	if (iExplore==100)
	{
		Result+=2;
	};
//Родовая провинция
	if (arPlayer[ipPlayer].ipProvince==iNumber)
	{
		/*for (int i=0; i<iMaxInnerBld; i++)
		{
			if (arPlayer[ipPlayer].arBld[i])
			{
				for (int j=0; j<MAX_BUILDING_ABILITY; j++)
				{					
					if ((pFirstInnerBld+i)->arAbility[j][0]==5)
					{
						Result+=(pFirstInnerBld+i)->arAbility[j][1];
					};
				};
			};
		};*/
	}
	else
	{
//Постройки в провинции
	for (int i=0; i<MAX_PROVINCE_BLD; i++)
	{
		if (arBuilding[i]>0)
		{
			Result+=(pFirstOuterBld+arBuilding[i])->iGoldAdd;
			for (int j=0; j<MAX_BUILDING_ABILITY; j++)
			{
				if (((pFirstOuterBld+arBuilding[i])->arAbility[j][0]==6)&&(arPlayer[ipPlayer].arBld[(pFirstOuterBld+arBuilding[i])->arAbility[j][2]]))
				{
					Result+=(pFirstOuterBld+arBuilding[i])->arAbility[j][1];
				};
			};
			for (int j=0; j<MAX_RESOURCE_BONUS; j++)
			{
				if (((pFirstOuterBld+arBuilding[i])->arResBonus[j][0]>0)&&((pFirstOuterBld+arBuilding[i])->arResBonus[j][0]==GetResource(1)))
				{
					Result+=(pFirstOuterBld+arBuilding[i])->arResBonus[j][1];
				};
			};
		};
	};
	};

//Постройки в замке
	for (int i=0; i<iMaxInnerBld; i++)
	{
		if (arPlayer[ipPlayer].arBld[i])
		{
			for (int j=0; j<MAX_BUILDING_ABILITY; j++)
			{
				if (((pFirstInnerBld+i)->arAbility[j][0]==14)&&(
					((pFirstInnerBld+i)->arAbility[j][1]==0)||
					((pFirstInnerBld+i)->arAbility[j][1]==ipTerrain)||
					(((pFirstInnerBld+i)->arAbility[j][1]==8)&&(Coastal))))
				{
					Result+=(pFirstInnerBld+i)->arAbility[j][2];
				};				
			};
		};
	};
	Result+=arPlayer[ipPlayer].GetBldParam2(49,ipRace);

//Сайты
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if ((arSite[i][0]>0)&&
			(((arSite[i][4]>0)&&((pFirstSite+arSite[i][0])->bWorkGuard))||
			((arSite[i][4]==0)&&((pFirstSite+arSite[i][0])->bWorkFree)))&&
			((arSite[i][1]<=iExplore)||((pFirstSite+arSite[i][0])->bWorkHide)))
		{
			for (int j=0; j<MAX_SITE_ABILITY; j++)
			{
				if ((pFirstSite+arSite[i][0])->arAbility[j][0]==2)
				{
					Result+=(pFirstSite+arSite[i][0])->arAbility[j][1];
				};
			};
		};
	};
	Result+=GetRitualBonus(1,6);

//Постройки в провинции (доход в %)
	for (int i=0; i<MAX_PROVINCE_BLD; i++)
	{
		if (arBuilding[i]>0)
		{
			Result+=Result*(pFirstOuterBld+arBuilding[i])->iGoldPerc/100;
		};
	};
//Страж
	if (iDefender>0)
	{
		for (int i=0; i<MAX_DEFENDER_ABILITY; i++)
		{
			if (((pFirstDefender+iDefender)->arAbility[i][0]==7)&&((pFirstDefender+iDefender)->arAbility[i][1]==GetResource(1)))
			{
				Result+=(pFirstDefender+iDefender)->arAbility[i][2];
			};
			if ((pFirstDefender+iDefender)->arAbility[i][0]==1)
			{
				Result+=Result*(pFirstDefender+iDefender)->arAbility[i][1]/100;
			};
		};
	};
	Result+=Result*GetSiteParam1(6)/100;
	Result-=Result*GetUnitSkill(58)/100;

	if (Result<0)
	{
		Result=0;
	};

	return Result;
};

int dcProvince::GetGemIncome()
{
	if ((ipTerrain==WATER_ID)||(ipTerrain==MOUNTAIN_ID))
	{return 0;};
	if (ipRace<0)
	{
		return 0;
	};
	int Result=iGemInc;	
	int Perc=0;
//Уровень провинции
	Result+=Result*ProvLevel[GetLevel()].iIncome/100;
	Result=Result*(pFirstRace+ipRace)->iGem/100;
	Result+=Result*GetMood()/20;	
//Исследованная территория
	Result-=Result*(100-iExplore)/500;
//Настроение жителей
	Result+=Result*GetMood()/20;

	if (arPlayer[ipPlayer].ipProvince==iNumber)
	{
		/*for (int i=0; i<iMaxInnerBld; i++)
		{
			if (arPlayer[ipPlayer].arBld[i])
			{
				for (int j=0; j<MAX_BUILDING_ABILITY; j++)
				{					
					if ((pFirstInnerBld+i)->arAbility[j][0]==5)
					{
						Result+=(pFirstInnerBld+i)->arAbility[j][2];
					};
				};
			};
		};*/
	}
	else
	{
//Постройки в провинции
	for (int i=0; i<MAX_PROVINCE_BLD; i++)
	{
		if (arBuilding[i]>0)
		{
			Result+=(pFirstOuterBld+arBuilding[i])->iGemAdd;
			Perc+=(pFirstOuterBld+arBuilding[i])->iGemPerc;
		};
	};
	};
	Result+=arPlayer[ipPlayer].GetBldParam2(55,ipRace);
//Сайты
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if ((arSite[i][0]>0)&&
			((arSite[i][4]==0)||((pFirstSite+arSite[i][0])->bWorkGuard))&&
			((arSite[i][4]>0)||((pFirstSite+arSite[i][0])->bWorkFree))&&
			((arSite[i][1]<=iExplore)||((pFirstSite+arSite[i][0])->bWorkHide)))
		{
			for (int j=0; j<MAX_SITE_ABILITY; j++)
			{
				if ((pFirstSite+arSite[i][0])->arAbility[j][0]==2)
				{
					Result+=(pFirstSite+arSite[i][0])->arAbility[j][2];
				};
			};
		};
	};
	Result+=GetRitualBonus(2,6);
//Страж
	if (iDefender>0)
	{
		for (int i=0; i<MAX_DEFENDER_ABILITY; i++)
		{			
			if ((pFirstDefender+iDefender)->arAbility[i][0]==1)
			{
				Result+=Result*(pFirstDefender+iDefender)->arAbility[i][2]/100;
			};
		};
	};
	Result+=Result*Perc/100;


	return Result;
};

int dcProvince::GetPort()
{
	int Result=0;
	if (arPlayer[ipPlayer].ipProvince==iNumber)
	{
		for (int i=1; i<iMaxInnerBld; i++)
		{
			if (arPlayer[ipPlayer].arBld[i])
			{
				for (int j=0; j<MAX_BUILDING_ABILITY; j++)
				{
					if ((pFirstInnerBld+i)->arAbility[j][0]==44)
					{
						Result+=(pFirstInnerBld+i)->arAbility[j][1];
					};
				};
			};
		}
	}
	else
	{
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			if ((arBuilding[i]>0)&&((pFirstOuterBld+arBuilding[i])->iPort>Result))
			{
				Result=(pFirstOuterBld+arBuilding[i])->iPort;
			};
		};
	};
	return Result;
};

int dcProvince::GetStable()
{
	int Result=0;
	if (arPlayer[ipPlayer].ipProvince==iNumber)
	{
		for (int i=1; i<iMaxInnerBld; i++)
		{
			if (arPlayer[ipPlayer].arBld[i])
			{
				for (int j=0; j<MAX_BUILDING_ABILITY; j++)
				{
					if ((pFirstInnerBld+i)->arAbility[j][0]==10)
					{
						Result+=(pFirstInnerBld+i)->arAbility[j][1];
					};
				};
			};
		}
	}
	else
	{
		Result=CheckBldAbil(2);		
	};
	return Result;
};

int dcProvince::GetHeal()
{
	int Result=0;
	if (arPlayer[ipPlayer].ipProvince==iNumber)
	{
		for (int i=1; i<iMaxInnerBld; i++)
		{
			if (arPlayer[ipPlayer].arBld[i])
			{
				for (int j=0; j<MAX_BUILDING_ABILITY; j++)
				{
					if ((pFirstInnerBld+i)->arAbility[j][0]==21)
					{
						Result+=(pFirstInnerBld+i)->arAbility[j][1];
					};
				};
			};
		}
	}
	else
	{
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			if (arBuilding[i]>0)
			{
				Result+=(pFirstOuterBld+arBuilding[i])->iHeal;
			};
		};
	};
	if (iDefender)
	{
		for (int i=0; i<MAX_DEFENDER_ABILITY; i++)
		{
			if ((pFirstDefender+iDefender)->arAbility[i][0]==6)
			{
				Result+=(pFirstDefender+iDefender)->arAbility[i][1];
			};
		};
	};
	return Result;
};


int dcProvince::GetGrow()
{
	if (iPopulation<=0)
	{return 1;};
	int Result=10+iPopulation/50;
	Result=Result*(pFirstRace+ipRace)->iGrow/100;
	if (ipTerrain!=(pFirstRace+ipRace)->iTerra)
	{Result=Result*(pFirstTerrain+ipTerrain)->iPopulationMod/100;};
	Result+=abs(Result)*GetMood()/10;
	int Add=0;
	if (arPlayer[ipPlayer].ipProvince==iNumber)
	{
		for (int i=1; i<iMaxInnerBld; i++)
		{
			if (arPlayer[ipPlayer].arBld[i])
			{
				for (int j=0; j<MAX_BUILDING_ABILITY; j++)
				{
					if ((pFirstInnerBld+i)->arAbility[j][0]==3)
					{
						Add+=(pFirstInnerBld+i)->arAbility[j][1];
					};
				};
			};
		}
	}
	else
	{
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			if (arBuilding[i]>0)
			{
				Add+=(pFirstOuterBld+arBuilding[i])->iGrow;
			};
		};
	};
	for (int i=1; i<iMaxInnerBld; i++)
	{
		if (arPlayer[ipPlayer].arBld[i])
		{
			for (int j=0; j<MAX_BUILDING_ABILITY; j++)
			{
				if ((pFirstInnerBld+i)->arAbility[j][0]==3)
				{
					Add+=(pFirstInnerBld+i)->arAbility[j][2];
				};
			};
		};
	};
	if (iDefender)
	{
		for (int i=0; i<MAX_DEFENDER_ABILITY; i++)
		{
			if ((pFirstDefender+iDefender)->arAbility[i][0]==9)
			{
				Add+=(pFirstDefender+iDefender)->arAbility[i][1];
			};
		};
	};
	Add+=GetSiteParam1(7);
	Add+=GetRitualBonus(1,2);
	Result+=Add;
	if (Result>0)
	{Result+=Result*Add/100;};
	return Result;
};

int dcProvince::GetMood()
{
	if ((iPopulation<=0)||(ipPlayer==0))
	{return 0;};
	int Result=arAttitude[ipPlayer][0];
	if (arPlayer[ipPlayer].ipProvince==iNumber)
	{
		Result+=arPlayer[ipPlayer].GetBldParam1(12);
	}	
	else
	{
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			if (arBuilding[i]>0)
			{
				Result+=(pFirstOuterBld+arBuilding[i])->iUnrest;
				for (int j=0; j<MAX_BUILDING_ABILITY; j++)
				{
					if (((pFirstOuterBld+arBuilding[i])->arAbility[j][0]==3)&&(arPlayer[ipPlayer].arBld[(pFirstOuterBld+arBuilding[i])->arAbility[j][2]]))
					{
						Result+=(pFirstOuterBld+arBuilding[i])->arAbility[j][1];
					};
				};
			};
		};
	};
	Result+=arPlayer[ipPlayer].GetBldParam2(12);	
	if (iDefender)
	{
		for (int i=0; i<MAX_DEFENDER_ABILITY; i++)
		{
			if ((pFirstDefender+iDefender)->arAbility[i][0]==2)
			{
				Result+=(pFirstDefender+iDefender)->arAbility[i][1];
			};
		};
	};
	if ((abs(arPlayer[ipPlayer].iKarma)>=50)&&((pFirstRace+ipRace)->iKarma!=0))
	{
		Result+=sign(arPlayer[ipPlayer].iKarma)*sign((pFirstRace+ipRace)->iKarma);
	};
	Result+=GetRitualBonus(1,1);
	Result+=GetSiteParam1(19);
	dcRitual *RR=pRitual;
	while (RR)
	{
		if (RR->ipProvince==iNumber)
		{
			for (int i=0; i<MAX_RITUAL_EFFECT; i++)
			{
				if ((pFirstRitual+RR->Index)->arEffect[i][0]==1)
				{
					if (ipPlayer==RR->ipPlayer)
					{Result+=(pFirstRitual+RR->Index)->arEffect[i][2];}
					else
					{Result-=(pFirstRitual+RR->Index)->arEffect[i][2];};
				};
			};
		};
		RR=RR->pNext;
	};
	return Result;
};

int dcProvince::GetUnrestSlowdown()
{
	if ((iPopulation<=0)||(ipPlayer==0))
	{return 0;};
	int Result=0;
	if (arPlayer[ipPlayer].ipProvince==iNumber)
	{
		Result+=arPlayer[ipPlayer].GetBldParam1(23);
	}	
	else
	{
		Result+=CheckBldAbil(5);		
	};
	Result+=arPlayer[ipPlayer].GetBldParam2(23);
	if (iDefender)
	{
		Result++;
		for (int i=0; i<MAX_DEFENDER_ABILITY; i++)
		{
			if ((pFirstDefender+iDefender)->arAbility[i][0]==3)
			{
				Result+=(pFirstDefender+iDefender)->arAbility[i][1];
			};
		};
	};
	return Result;
};

int dcProvince::GetWall()
{	
	if (arPlayer[ipPlayer].ipProvince==iNumber)
	{return arPlayer[ipPlayer].iWall;};
	if ((arPlayer[1].ipProvince==iNumber)&&(Tutorial.Map))
	{return arPlayer[1].iWall;};
	int iWall=0;
	for (int i=0; i<MAX_PROVINCE_BLD; i++)
	{
		if (arBuilding[i]>0)
		{iWall+=(pFirstOuterBld+arBuilding[i])->iWall;};
	};
	return iWall;	
};

bool dcProvince::CanBuild(int Building)
{		
	if (GetSiegeIndex()>=0)
	{return false;};
	clOuterBuilding * BL=pFirstOuterBld+Building;
	if (BL->arTerrain[ipTerrain-1]==0)
	{return false;};
	if ((BL->iCoastal==1)&&(!Coastal))
	{return false;};
	if ((BL->iCoastal==2)&&(Coastal))
	{return false;};
	int Bld=0;
	bool Out=true;
	for (int i=0; i<MAX_PROVINCE_BLD; i++)
	{
		if (arBuilding[i]==Building)
		{return false;};
		clOuterBuilding * BLD=pFirstOuterBld+arBuilding[i];
		while (BLD->ipUpgrade>0)
		{
			if (Building==BLD->ipUpgrade)
			{return false;};
			BLD=pFirstOuterBld+BLD->ipUpgrade;
		};
		if (arBuilding[i]>0)
		{
			if (BL->ipUpgrade==arBuilding[i])
			{
				Out=false;
			};
			Bld++;
		}
	};
	if ((Bld==MAX_PROVINCE_BLD)&&(BL->ipUpgrade==0))
	{return false;};
	if ((BL->ipUpgrade>0)&&(Out))
	{return false;};

	return true;
};

bool dcProvince::CanHire(int Defender)
{		
	if (GetSiegeIndex()>=0)
	{return false;};
	clDefender * DF=pFirstDefender+Defender;
	if (DF->arTerrain[ipTerrain-1]==0)
	{return false;};
	if ((iDefender>0)&&((pFirstDefender+iDefender)->bNoDismiss))
	{return false;};
	return true;
};

bool dcProvince::CanCast(int Ritual, int Player)
{		
	clRitual * RIT=pFirstRitual+Ritual;
	if ((!(RIT->bOnEnemy))&&(Player!=ipPlayer))
	{return false;};
	if ((!(RIT->bOnAlly))&&(Player==ipPlayer)&&(GetSiegeIndex()<0))
	{return false;};
	if ((RIT->GetAbility(18))&&(arPlayer[ipPlayer].ipProvince==iNumber))
	{return false;};
	if ((RIT->GetAbility(19))&&(GetSiegeIndex()>=0))
	{return false;};
	if (RIT->GetAbility(20))
	{
		bool NoHero=true;
		if (RIT->bOnAlly)
		{
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0)&&(arPlayer[Player].arHero[i]->pProvince==this))
				{NoHero=false;};
			};
		}
		else
		{
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[ipPlayer].arHero[i])&&(arPlayer[ipPlayer].arHero[i]->iDead<0)&&(arPlayer[ipPlayer].arHero[i]->pProvince==this))
				{NoHero=false;};
			};
		};
		if (NoHero)
		{return false;};
	};
	if (RIT->GetAbility(22))
	{
		if (arPlayer[ipPlayer].ipProvince==iNumber)
		{return false;};
		int BL=0;
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			if (arBuilding[i])
			{BL++;};
		};
		if (BL==MAX_PROVINCE_BLD)
		{return false;};
	};
	if (RIT->GetAbility(23)>0)
	{		
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			if (arBuilding[i]==RIT->GetAbility(23))
			{return false;};
		};
	};
	if ((RIT->GetAbility(24))&&(iPopulation<=0))
	{return false;};	
	if ((RIT->GetAbility(25))&&(iDefender>0)&&((pFirstDefender+iDefender)->bNoDismiss))
	{return false;};
	/*dcRitual *RR=pRitual;
	while (RR)
	{
		if (RR->ipProvince==iNumber)
		{
			if (RR->Index==Ritual)
			{return false;};
			if (RR->ipPlayer!=Player)
			{
				for (int i=0 ;i<MAX_RITUAL_EFFECT; i++)
				{
					if ((pFirstRitual+RR->Index)->arEffect[i][0]==12)
					{return false;};
				};
			};
		};
		RR=RR->pNext;
	};*/
	return true;
};

int dcProvince::CheckBldAbil(int Index)
{
	int Result=0;
	for (int i=0; i<MAX_PROVINCE_BLD; i++)
	{
		if (arBuilding[i]>0)
		{
			for (int j=0; j<MAX_BUILDING_ABILITY; j++)
			{
				if ((pFirstOuterBld+arBuilding[i])->arAbility[j][0]==Index)
				{
					Result+=(pFirstOuterBld+arBuilding[i])->arAbility[j][1];
				};
			};
		};
	};
	return Result;
};

int dcProvince::GetSiteParam1(int Index)
{
	int Result=0;
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if ((arSite[i][0]>0)&&
			((arSite[i][4]==0)||((pFirstSite+arSite[i][0])->bWorkGuard))&&
			((arSite[i][4]>0)||((pFirstSite+arSite[i][0])->bWorkFree))&&
			((arSite[i][1]<=iExplore)||((pFirstSite+arSite[i][0])->bWorkHide)))
		{
			for (int j=0; j<MAX_SITE_ABILITY; j++)
			{
				if ((pFirstSite+arSite[i][0])->arAbility[j][0]==Index)
				{
					Result+=(pFirstSite+arSite[i][0])->arAbility[j][1];
				};
			};
		};
	};
	return Result;
};

int dcProvince::GetSiteParam2(int Index)
{
	int Result=0;
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if ((arSite[i][0]>0)&&
			((arSite[i][4]==0)||((pFirstSite+arSite[i][0])->bWorkGuard))&&
			((arSite[i][4]>0)||((pFirstSite+arSite[i][0])->bWorkFree))&&
			((arSite[i][1]<=iExplore)||((pFirstSite+arSite[i][0])->bWorkHide)))
		{
			for (int j=0; j<MAX_SITE_ABILITY; j++)
			{
				if ((pFirstSite+arSite[i][0])->arAbility[j][0]==Index)
				{
					Result+=(pFirstSite+arSite[i][0])->arAbility[j][2];
				};
			};
		};
	};
	return Result;
};

int dcProvince::GetSiteParam2(int Index, int Param1)
{
	int Result=0;
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if ((arSite[i][0]>0)&&
			((arSite[i][4]==0)||((pFirstSite+arSite[i][0])->bWorkGuard))&&
			((arSite[i][4]>0)||((pFirstSite+arSite[i][0])->bWorkFree))&&
			((arSite[i][1]<=iExplore)||((pFirstSite+arSite[i][0])->bWorkHide)))
		{
			for (int j=0; j<MAX_SITE_ABILITY; j++)
			{
				if (((pFirstSite+arSite[i][0])->arAbility[j][0]==Index)&&((pFirstSite+arSite[i][0])->arAbility[j][1]==Param1))
				{
					Result+=(pFirstSite+arSite[i][0])->arAbility[j][2];
				};
			};
		};
	};
	return Result;
};

int dcProvince::GetUnitSkill(int Index)
{
	int Result=0;
	if (pGarrison)
	{
		for (int i=0; i<MAX_SQUAD; i++)
		{
			if (pGarrison->pSquad[i])
			{
				Result+=pGarrison->pSquad[i]->CheckAbility(Index)+pGarrison->pSquad[i]->CheckUpg(Index);
			};
		};
	};
	for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[ipPlayer].arHero[i])&&(arPlayer[ipPlayer].arHero[i]->pProvince==this)&&(arPlayer[ipPlayer].arHero[i]->iDead<=0))
		{			
			for (int j=0; j<MAX_SQUAD; j++)
			{
				if (arPlayer[ipPlayer].arHero[i]->pSquad[j])
				{
					Result+=arPlayer[ipPlayer].arHero[i]->pSquad[j]->CheckAbility(Index)+arPlayer[ipPlayer].arHero[i]->pSquad[j]->CheckUpg(Index);
				};
			};
		};
	};
	return Result;
};

int dcProvince::GetEventPoss(int ipEvent)
{
	bool bExit;
	if (GetSiegeIndex()>=0)
	{return 0;};
	//Отмена атаки, если вокруг провинции барьер
	if (((pFirstEvent+ipEvent)->ipAttacker>0)&&(GetRitualBonus(1,12)>0))
	{return 0;};
	//Проверка условий, необходимых для события
	for (int i=0; i<MAX_EVENT_EFFECT; i++)
	{
		if ((pFirstEvent+ipEvent)->arEffect[i][0]==8)
		{
			int Count;
			switch ((pFirstEvent+ipEvent)->arEffect[i][1])
			{
			case 1:
				if (GetLevel()<(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 2:
				if (ipTerrain!=(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 3:
				if (arPlayer[ipPlayer].Gold<(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 4:
				if (GetFreeSpace(true)==0)
				{return 0;};
				break;
			case 5:
				if (GetMood()>(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 6:
				if (GetMood()<(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 7:
				if (GetLevel()>(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 8:
				Count=0;
				for (int i=0; i<TREASURY_SIZE; i++)
				{
					if (arPlayer[ipPlayer].arTreasury[i][0]==0)
					{Count++;};
				};
				if (Count<(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 9:
				if (arPlayer[ipPlayer].iKarma>(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 10:
				if (arPlayer[ipPlayer].iKarma<(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 11:
				if (arPlayer[ipPlayer].ipProvince==iNumber)
				{return 0;};
				break;
			case 12:
				if (!Coastal)
				{return 0;};
				break;
			case 13:
				if (ipRace!=(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 14:
				bExit=true;
				for (int j=0; j<MAX_HERO; j++)
				{
					if ((arPlayer[ipPlayer].arHero[j])&&(arPlayer[ipPlayer].arHero[j]->pProvince==this)&&(arPlayer[ipPlayer].arHero[j]->iDead<0))
					{bExit=false;};
				};
				if (((pFirstEvent+ipEvent)->arEffect[i][2]>0)&&(pGuard)&&(pGuard->ipPlayer==ipPlayer))
				{bExit=false;};
				if (bExit)
				{return 0;};
				break;
			case 15:
				if (arPlayer[ipPlayer].ipProvince!=iNumber)
				{return 0;};
				break;
			case 16:
				if (GetResource(-1)>0)
				{return 0;};
				break;
			case 17:
				bExit=true;
				for (int k=0; k<MAX_PROVINCE_SITE; k++)
				{
					if (arSite[k][0]==0)
					{bExit=false;};
				};
				if (!bExit)
				{
					if (GetFreeSpace((pFirstEvent+ipEvent)->arEffect[i][2])==0)
					{bExit=true;};
				};
				if (bExit)
				{return 0;};
				break;
			case 18:
				if (pGuard)
				{
					if (((pFirstEvent+ipEvent)->arEffect[i][2]==0)||((pFirstEvent+ipEvent)->arEffect[i][2]!=iDefender))
					{return 0;};					
				}
				else
				{
					if ((pFirstEvent+ipEvent)->arEffect[i][2]>0)
					{return 0;};
				};
				break;
			case 19:				
				bExit=true;
				for (int k=0; k<MAX_PROVINCE_BLD; k++)
				{
					if (arBuilding[k]==(pFirstEvent+ipEvent)->arEffect[i][2])
					{bExit=false;};
				};
				if (bExit)
				{return 0;};
				break;
			case 20:
				if (arPlayer[ipPlayer].ipProvince==iNumber)
				{return 0;};
				bExit=true;
				for (int k=0; k<MAX_PROVINCE_BLD; k++)
				{
					if (arBuilding[k]==0)
					{bExit=false;};
				};
				if (bExit)
				{return 0;};
				break;
			case 21:				
				bExit=false;
				for (int k=0; k<MAX_PROVINCE_BLD; k++)
				{
					if (arBuilding[k]==(pFirstEvent+ipEvent)->arEffect[i][2])
					{bExit=true;};
				};
				if (bExit)
				{return 0;};
				break;
			case 22:
				if (arPlayer[ipPlayer].iCalcCorrupt<(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 23:
				if (arPlayer[ipPlayer].Gem<(pFirstEvent+ipEvent)->arEffect[i][2])
				{return 0;};
				break;
			case 24:
				if (arPlayer[ipPlayer].GetResource((pFirstEvent+ipEvent)->arEffect[i][2])==0)
				{return 0;};
				break;
			case 25:
				if (arPlayer[ipPlayer].GetResource((pFirstEvent+ipEvent)->arEffect[i][2])>0)
				{return 0;};
				break;
			case 26:
				if ((CurGame.iGameType!=2)||(Campaign.iActiveShard<0)||(Campaign.arDialog[(pFirstEvent+ipEvent)->arEffect[i][2]]==0)||((pFirstEvent+ipEvent)->arEffect[i][3]>CurGame.iCurrentTurn))
				{return 0;};
				break;
			case 27:				
				for (int j=0; j<MAX_HERO; j++)
				{
					if ((arPlayer[ipPlayer].arHero[j])&&(arPlayer[ipPlayer].arHero[j]->pProvince==this)&&(arPlayer[ipPlayer].arHero[j]->iDead<0))
					{return 0;;};
				};
				break;
			case 28:
				for (int j=0; j<MAX_PROVINCE_SITE; j++)
				{
					if (arSite[j][0]==(pFirstEvent+ipEvent)->arEffect[i][2])
					{return 0;};
				};
				break;
			case 29:
				bExit=true;
				for (int j=0; j<TREASURY_SIZE; j++)
				{
					if (arPlayer[ipPlayer].arTreasury[j][0]==(pFirstEvent+ipEvent)->arEffect[i][2])
					{
						bExit=false;
						break;
					};					
				};
				if (bExit)
				{return 0;};
				break;
			};
		};
	};
	//Глобальная вероятность
	int Result=(pFirstEvent+ipEvent)->iPossibility;
	//Инициируемые соседями
	for (int j=0; j<MAX_NEIGHBOUR; j++)
	{
		if ((arNeighbour[j]>0)&&((pFirstProvince+arNeighbour[j])->ipPlayer==0))
		{
			for (int k=0; k<MAX_PROVINCE_EVENT; k++)
			{
				if ((pFirstProvinceType+(pFirstProvince+arNeighbour[j])->ipType)->arEvent[k][0]==ipEvent)
				{
					Result+=(pFirstProvinceType+(pFirstProvince+arNeighbour[j])->ipType)->arEvent[k][1];
				};
			};
		};
	};
	//Инициируемые сайтами
	Result+=GetSiteParam2(4,ipEvent);

	//Изменение вероятности в зависимости от правил мира
	if (WorldRules.iMode>0)
	{
		for (int k=0; k<MAX_WORLD_EVENTS; k++)
		{
			if ((pFirstWorldRules+WorldRules.iMode)->arRandomEvents[k][0]==ipEvent)
			{Result+=(pFirstWorldRules+WorldRules.iMode)->arRandomEvents[k][1];}
		}
	}

	if (Result<=0)
	{return 0;};
	//Изменение вероятности в зависимости от построек
	if (arPlayer[ipPlayer].ipProvince==iNumber)
	{
		Result+=Result*arPlayer[ipPlayer].GetBldParam2(45,ipEvent)/100;
	}	
	else
	{
		for (int i=1; i<MAX_PROVINCE_BLD; i++)
		{
			if (arBuilding[i]>0)
			{
				for (int j=0; j<MAX_BUILDING_ABILITY; j++)
				{
					if (((pFirstOuterBld+arBuilding[i])->arAbility[j][0]==7)&&((pFirstOuterBld+arBuilding[i])->arAbility[j][1]==ipEvent))
					{
						Result+=Result*(pFirstOuterBld+arBuilding[i])->arAbility[j][2]/100;
					};
				};
			};
		};
	};	
	if (sign(arPlayer[ipPlayer].iKarma+arPlayer[ipPlayer].GetBldParam1(38))==sign((pFirstEvent+ipEvent)->iType))
	{
		Result+=Result*abs(arPlayer[ipPlayer].iKarma+arPlayer[ipPlayer].GetBldParam1(38))/100;
	};
	if (Result<1)
	{Result=1;};
	return Result;
};


void dcProvince::ChangeAttitude(int Player, int Change)
{
	if (Change!=0)
	{
		if (arAttitude[Player][0]==0)
		{
			arAttitude[Player][1]=ATTITUDE_TIME;
		};
		arAttitude[Player][0]+=Change;
	};
};

bool bBury;


int dcProvince::AttackDefender(dcHero *Attacker, int DX, int DefCount)
{
	int Result=1, iScGold=0, iBrGold=0, Answer, Count=0;
	bool bAn[2];
	char sText[500], sArmy[500];
	int GuardsNum=pGuard->MakeDescription(sArmy);
	int Power=CompareStrength1(Attacker,pGuard);	
	BITMAP *Pic=bmpIllustration[(pFirstDefender+iDefender)->ipPicture];
	sprintf(sText,(pFirstDialog+369)->sText,Attacker->pstName->str,sName,arPlayer[ipPlayer].GetName(),GuardsNum,sArmy);
	DialogDX=DX;
	if (arPlayer[ipPlayer].Control==1)
	{iMessageFor=Attacker->ipPlayer;}
	else
	{
		iDialogButt=1;
		pAdvice=Attacker;
		if (arDifficulty[arPlayer[Attacker->ipPlayer].iDifficulty].bShowAdvice)
		{
			iHeroAdvice=1;
			iAdvice=Power;			
		};
	};
	if ((Power>180+Random(10))&&((pFirstDefender+iDefender)->CheckAbility(11)==0)&&(arPlayer[ipPlayer].ipProvince!=iNumber))
	{
		//Отступление
		sprintf(sText,(pFirstDialog+1240)->sText,Attacker->pstName->str,sName,arPlayer[ipPlayer].GetName(),GuardsNum,sArmy);
		Answer=(pFirstDialog+1240)->Show(1,1,0,0,Pic,sText)-1;
	}
	else
	{
		bAn[0]=0; bAn[1]=0;
		
		//Разведка
		if (Attacker->CheckSkill(24)>0)
		{
			iScouting=SCOUTING_PRICE;
			if (Attacker->CheckSkill(24)>=5)
			{iScouting/=2;};
			iScGold=iScouting;
			if (iScouting<=arPlayer[Attacker->ipPlayer].Gold)
			{bAn[0]=1;};
		};

		//Подкуп
		if ((DefCount<2)&&(!pGarrison)&&((pFirstDefender+iDefender)->CheckAbility(12)==0)&&(Power>160+Random(10)-Attacker->CheckSkill(27)*2))
		{
			iBribe=(pFirstDefender+iDefender)->iGoldCost*4+(pFirstDefender+iDefender)->iGemCost*10;
			if (iBribe<=0)
			{iBribe=pGuard->GetDefenderGoldPayment()*25+pGuard->GetDefenderGemPayment()*50;};
			iBribe+=50;
			iBribe=(iBribe*10-iBribe*Attacker->CheckSkill(27))/10;
			iBrGold=iBribe;
			if (iBribe<=arPlayer[Attacker->ipPlayer].Gold)
			{
				if (iScouting==0)
				{bAn[0]=1;}
				else
				{bAn[1]=1;};
			};
		};
		iDialogButt=1;
		pAdvice=Attacker;
		Answer=(pFirstDialog+369)->Show(true,true,bAn[0],bAn[1],Pic,sText);
		if ((Answer==3)&&(iScGold>0))
		{			
			arPlayer[Attacker->ipPlayer].Gold-=iScGold;
			pGuard->MakeScoutingText();
			iBribe=iBrGold;
			if (iBribe>arPlayer[Attacker->ipPlayer].Gold)
			{bAn[1]=0;};
			iDialogButt=1;
			pAdvice=Attacker;
			Answer=(pFirstDialog+369)->Show(true,true,bAn[1],false,Pic,stScouting);
		};
	};		
				
	switch (Answer)
	{
	case 0:
		DismissDefender();
		break;
	case 1:
		Result=InitBattle(Attacker->pProvince,this,Attacker,pGuard,0,true);
		if (abs(Result)==1)
		{
			DismissDefender();
		};
		if (Attacker->pSquad[0]->iLife==0)
		{
			bBury=Attacker->Death(Result);
			if ((DefCount>1)&&((bBury)||(Attacker->pSquad[0]->iLife==0)))
			{Result=0;};
		};
		break;
	case 2:		
		if (arPlayer[ipPlayer].Control==1)
		{
			DialogDX=DX;
			iMessageFor=ipPlayer;
			(pFirstDialog+373)->Show(true,true,false,false,NULL,NULL);
		};
		Result=0;
		break;
	case 3:
	case 4:
		arPlayer[Attacker->ipPlayer].Gold-=iBrGold;
		pGuard->ipPlayer=Attacker->ipPlayer;
		pGuard->ChangeMorale(-7,0);
		break;
	};
	return Result;
};

int dcProvince::AttackSiege(dcHero *Attacker, int DX)
{
	int Result=1, iScGold=0;
	int DefCount=1;
	int arDef[9];
	bool bHourz=bHourGlass;
	while ((Result>0)&&(GetSiegeIndex()>=0))
	{
		int DefPlayer=GetSiegeIndex();
		if (arPlayer[DefPlayer].Control==1)
		{
		//Осаждает игрок
			char sText[500], sArmy[500];
			int GuardsNum=Attacker->MakeDescription(sArmy);
			sprintf(sText,(pFirstDialog+1464)->sText,sName,GuardsNum,sArmy);
			DialogDX=DX;
			iMessageFor=DefPlayer;
			bMessageFor=true;
			(pFirstDialog+1464)->Show(true,true,false,false,NULL,sText);
			bMessageFor=false;

			while ((DefCount>0)&&(Result==1))
			{
		//Выбор защитника
				DefCount=0;
				for (int i=0; i<MAX_HERO; i++)
				{
					if ((arPlayer[DefPlayer].arHero[i])&&(arPlayer[DefPlayer].arHero[i]->pProvince==this)&&(arPlayer[DefPlayer].arHero[i]->iDead<0))
					{			
						arDef[DefCount]=i;
						DefCount++;
					};
				};	
				int Select=-1;
				if (DefCount<=0)
				{return Result;};
				if (DefCount>1)
				{
					UI_Mask arMask[9];
					int DD=68;
					UI_TextBox txtChoose;
					BITMAP *bmpBack=create_bitmap(1024,768);			
					blit(screen1,bmpBack,0,0,0,0,1024,768);
					BITMAP *bmpWork=create_bitmap(bmpGUI_DefChoose->w,bmpGUI_DefChoose->h-DD+DD*DefCount);			
					int x=512-bmpGUI_DefChoose->w/2, y=384-bmpGUI_DefChoose->h/2;			
					G_Object *objMain=new G_Object(NULL,x,y,1,bmpWork);			
					objMain=new G_Object(objMain,0,0,0,bmpBack);
					objMain=txtChoose.Init(x+25,y+30,254,90,2,COLOR_1,Font2,objMain);			
					txtChoose.Activate((pFirstMessage+16)->msg);
					blit(bmpGUI_DefChoose,bmpWork,0,0,0,0,bmpGUI_DefChoose->w,131);			
					for (int i=0; i<DefCount; i++)
					{
						blit(bmpGUI_DefChoose,bmpWork,0,131,0,131+i*DD,bmpGUI_DefChoose->w,DD);			
						draw_sprite(bmpWork,arPlayer[DefPlayer].arHero[arDef[i]]->smPic,20,141+i*DD);
						textprintf_centre_ex(bmpWork,Font1,180,146+i*DD,COLOR_1,-1,arPlayer[DefPlayer].arHero[arDef[i]]->pstName->str);
						objMain=arMask[i].Init(x+14,y+135+i*DD,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objMain,2);				
					};			
					blit(bmpGUI_DefChoose,bmpWork,0,bmpGUI_DefChoose->h-17,0,131+DefCount*DD,bmpGUI_DefChoose->w,17);			
					set_mouse_sprite(bmpMousePointer[1]);
					while (Select<0)
					{				
						for (int i=0; i<DefCount; i++)
						{
							if (arMask[i].Check()==30)
							{
								Select=i;
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
				{
					Select=0;
				};
				if (arPlayer[Attacker->ipPlayer].Control==1)
				{
					char sText[500], sArmy[500];
					int GuardsNum=arPlayer[DefPlayer].arHero[arDef[Select]]->MakeDescription(sArmy);
					sprintf(sText,(pFirstDialog+385)->sText,Attacker->pstName->str,sName,arPlayer[DefPlayer].GetName(),arPlayer[DefPlayer].arHero[arDef[Select]]->pstName->str,GuardsNum,sArmy);
					DialogDX=DX;
					iMessageFor=Attacker->ipPlayer;
					int Answer;
					if (Attacker->CheckSkill(24)>0)
					{
						iScouting=SCOUTING_PRICE;
						if (Attacker->CheckSkill(24)>=5)
						{iScouting/=2;};
						iScGold=iScouting;
						if (iScouting<=arPlayer[Attacker->ipPlayer].Gold)
						{Answer=(pFirstDialog+385)->Show(true,true,true,false,NULL,sText);}
						else
						{Answer=(pFirstDialog+385)->Show(true,true,false,false,NULL,sText);};
						if (Answer==3)
						{
							arPlayer[Attacker->ipPlayer].Gold-=iScGold;
							arPlayer[DefPlayer].arHero[arDef[Select]]->MakeScoutingText();
							Answer=(pFirstDialog+385)->Show(true,true,false,false,NULL,stScouting);
						};
					}
					else
					{Answer=(pFirstDialog+385)->Show(true,true,false,false,NULL,sText);};
			
					if (Answer==1)
					{
						Result=InitBattle(Attacker->pProvince,this,Attacker,arPlayer[DefPlayer].arHero[arDef[Select]],0,true);
						if (arPlayer[DefPlayer].arHero[arDef[Select]]->pSquad[0]->iLife==0)
						{
							iMessageFor=DefPlayer;
							if (Result==0)
							{arPlayer[DefPlayer].arHero[arDef[Select]]->Death(1);}
							else
							{
								if (Result==1)
								{arPlayer[DefPlayer].arHero[arDef[Select]]->Death(0);}
								else
								{arPlayer[DefPlayer].arHero[arDef[Select]]->Death(Result);};
							};
						};
						if (Attacker->pSquad[0]->iLife==0)
						{
							iMessageFor=Attacker->ipPlayer;
							bBury=Attacker->Death(Result);
							if ((DefCount>1)&&((bBury)||(Attacker->pSquad[0]->iLife==0)))
							{Result=0;};
						};
					}
					else
					{
						DialogDX=DX;
						iMessageFor=DefPlayer;
						(pFirstDialog+373)->Show(true,true,false,false,NULL,NULL);
						Result=0;
					};
				}
				else
				{
					if (Attacker->ipPlayer==0)
					{
						Result=InitBattle(this,NULL,Attacker,arPlayer[DefPlayer].arHero[arDef[Select]],0,true);
					}
					else
					{
						if (CompareStrength1(Attacker,arPlayer[DefPlayer].arHero[arDef[Select]])>100)
						{
							Result=InitBattle(Attacker->pProvince,this,Attacker,arPlayer[DefPlayer].arHero[arDef[Select]],0,true);
							Attacker->NiB=0;
							if (Attacker->pSquad[0]->iLife==0)
							{
								bBury=Attacker->Death(Result);
								if ((DefCount>1)&&((bBury)||(Attacker->pSquad[0]->iLife==0)))
								{Result=0;};
							};
						}
						else
						{
							DialogDX=DX;
							(pFirstDialog+373)->Show(true,true,false,false,NULL,NULL);
							Result=0;
							if (Attacker->iCamper==iNumber)
							{Attacker->iCampTurn++;}
							else
							{
								Attacker->iCamper=iNumber;
								Attacker->iCampTurn=0;
							};
						};					
					};
					if (arPlayer[DefPlayer].arHero[arDef[Select]]->pSquad[0]->iLife==0)
					{
						if (Result==0)
						{arPlayer[DefPlayer].arHero[arDef[Select]]->Death(1);}
						else
						{
							if (Result==1)
							{arPlayer[DefPlayer].arHero[arDef[Select]]->Death(0);}
							else
							{arPlayer[DefPlayer].arHero[arDef[Select]]->Death(Result);}
						};
					};
				};
			};
		}
		else
		{
		//Осаждат ИИ
			bool bFight=true;
			while ((DefCount>0)&&(Result==1))
			{
		//Выбор защитника
				DefCount=0;
				int Select=-100;
				dcHero *Defender=NULL;
				if (DefPlayer==0)
				{
					if (pGuard)
					{
						DefCount=1;
						Defender=pGuard;
					};
				}
				else
				{
					for (int i=0; i<MAX_HERO; i++)
					{
						if ((arPlayer[DefPlayer].arHero[i])&&(arPlayer[DefPlayer].arHero[i]->pProvince==this)&&(arPlayer[DefPlayer].arHero[i]->iDead<0))
						{
							DefCount++;
						};
					};
					if (Select==-100)
					{
						for (int i=0; i<MAX_HERO; i++)
						{
							if ((arPlayer[DefPlayer].arHero[i])&&(arPlayer[DefPlayer].arHero[i]->pProvince==this)&&(arPlayer[DefPlayer].arHero[i]->iDead<0))
							{
								if ((Select==-100)||(CompareStrength(arPlayer[DefPlayer].arHero[i],Attacker)>CompareStrength(arPlayer[DefPlayer].arHero[Select],Attacker)))
								{
									Select=i;
									Defender=arPlayer[DefPlayer].arHero[i];
								};
							};
						};
					};
				}
				if ((Defender)&&(DefCount>0))
				{
			//Атака нейтралов
				if (Attacker->ipPlayer==0)
				{
					Result=InitBattle(this,NULL,Attacker,Defender,0,false);
					if (DefPlayer>0)
					{
						if ((Select>-1)&&(Defender->pSquad[0]->iLife==0))
						{
							if (Result==0)
							{arPlayer[DefPlayer].arHero[Select]->Death(1);}
							else
							{
								if (Result==1)
								{arPlayer[DefPlayer].arHero[Select]->Death(0);}
								else
								{arPlayer[DefPlayer].arHero[Select]->Death(Result);};
							};
						};
					}
					else
					{
						if (Result==1)
						{DismissDefender();};
					};
				}				
				else
				{
					if (arPlayer[Attacker->ipPlayer].Control==1)
		//Атака игрока
					{
						char sText[500], sArmy[500];
						int GuardsNum=Defender->MakeDescription(sArmy);
						if (DefPlayer==0)
						{sprintf(sText,(pFirstDialog+386)->sText,Attacker->pstName->str,sName,GuardsNum,sArmy);}
						else
						{sprintf(sText,(pFirstDialog+385)->sText,Attacker->pstName->str,sName,arPlayer[DefPlayer].GetName(),Defender->pstName->str,GuardsNum,sArmy);}
						DialogDX=DX;
						bMessageFor=true;
						iMessageFor=Attacker->ipPlayer;
						iDialogButt=1;
						pAdvice=Attacker;
							int Answer;
							if (Attacker->CheckSkill(24)>0)
							{
								iScouting=SCOUTING_PRICE;
								if (Attacker->CheckSkill(24)>=5)
								{iScouting/=2;};
								iScGold=iScouting;								
								if (iScouting<=arPlayer[Attacker->ipPlayer].Gold)
								{Answer=(pFirstDialog+385)->Show(true,true,true,false,NULL,sText);}
								else
								{Answer=(pFirstDialog+385)->Show(true,true,false,false,NULL,sText);};
								if (Answer==3)
								{
									iDialogButt=1;
									pAdvice=Attacker;
									arPlayer[Attacker->ipPlayer].Gold-=iScGold;
									Defender->MakeScoutingText();
									Answer=(pFirstDialog+385)->Show(true,true,false,false,NULL,stScouting);
								};
							}
							else
							{Answer=(pFirstDialog+385)->Show(true,true,false,false,NULL,sText);};
							bMessageFor=false;
							if (Answer==1)
							{
								Result=InitBattle(Attacker->pProvince,this,Attacker,Defender,0,true);
								if (DefPlayer>0)
								{
									if (Defender->pSquad[0]->iLife==0)
									{
										if (Result==0)
										{arPlayer[DefPlayer].arHero[Select]->Death(1);}
										else
										{
											if (Result==1)
											{arPlayer[DefPlayer].arHero[Select]->Death(0);}
											else
											{arPlayer[DefPlayer].arHero[Select]->Death(Result);}
										};
									};
								}
								else
								{
									if (Result==1)
									{DismissDefender();}
								}
								if (Attacker->pSquad[0]->iLife==0)
								{
									bBury=Attacker->Death(Result);
									if ((DefCount>1)&&((bBury)||(Attacker->pSquad[0]->iLife==0)))
									{Result=0;};
								};
							}
							else
							{Result=0;};
						}
					
					else
		//Атака AI
					{
						if (CompareStrength1(Attacker,Defender)>100)
						{
							Result=InitBattle(Attacker->pProvince,this,Attacker,Defender,0,false);
							Attacker->NiB=0;
							if (DefPlayer>0)
							{
								if ((Defender->pSquad[0]->iLife==0)&&(Select>-1))
								{
									if (Result==0)
									{arPlayer[DefPlayer].arHero[Select]->Death(1);}
									else
									{
										if (Result==1)
										{arPlayer[DefPlayer].arHero[Select]->Death(0);}
										else
										{arPlayer[DefPlayer].arHero[Select]->Death(Result);};
									};
								};
							}
							else
							{
								if (Result==1)
								{DismissDefender();}
							}
							if (Attacker->pSquad[0]->iLife==0)
							{
								bBury=Attacker->Death(Result);
								if ((DefCount>1)&&((bBury)||(Attacker->pSquad[0]->iLife==0)))
								{Result=0;};
							};
						}
						else
						{
							Result=0;
							if (Select>0)
							{
								if (Attacker->iCamper==iNumber)
								{Attacker->iCampTurn++;}
								else
								{
									Attacker->iCamper=iNumber;
									Attacker->iCampTurn=0;
								};
							};
						};
					};
				};
			};
		};
	};
	}
	if (!bHourz)
	{bHourGlass=false;};
	return Result;
};

int dcProvince::UnderAttack(dcHero *Attacker, int DX)
{
	int Result=1, iScGold=0;
	int DefCount=1;
	int arDef[9];
	bool bCrazyAttack=false;
	if (ipPlayer==0)
	{
		Result=InitBattle(this,NULL,Attacker,pGuard,0,false);
		if (Result==1)
		{
			DismissDefender();
		};
		return Result;
	};
	if (GetSiegeIndex()>=0)
	{
		Result=AttackSiege(Attacker,DX);
		if (Result!=1)
		{return Result;}
	}
	//Ожидание помощи при атаке родовой провинции слабым отрядом
	if ((Attacker->ipPlayer>0)&&(ipPlayer>0)&&(arPlayer[Attacker->ipPlayer].Control>1)&&(iNumber==arPlayer[ipPlayer].ipProvince))
	{
		int PL=Attacker->ipPlayer, HNum=0;		
		for (int i=0; i<MAX_HERO; i++)
		{
			if ((arPlayer[PL].arHero[i])&&(arPlayer[PL].arHero[i]->iDead<0))
			{
				HNum++;				
			};
			if (HNum==1)
			{bCrazyAttack=true;};
			if (arPlayer[ipPlayer].pEvent)
			{
				dcEvent *EV=arPlayer[ipPlayer].pEvent->pNext;
				while ((!bCrazyAttack)&&(EV))
				{
					if ((EV->Index==10)&&(EV->Prov2==this))
					{bCrazyAttack=true;};
					EV=EV->pNext;
				};
			};
		};
	};
	if (arPlayer[ipPlayer].Control==1)
//Нападение на провинцию игрока
	{
	//Сообщение при атаке противника
	if (Attacker->ipPlayer!=0)
	{
		char sText[500], sArmy[500];
		int GuardsNum=Attacker->MakeDescription(sArmy);		
		sprintf(sText,(pFirstDialog+368)->sText,sName,arPlayer[Attacker->ipPlayer].GetName(),Attacker->pstName->str,GuardsNum,sArmy);
		DialogDX=DX;
		iMessageFor=ipPlayer;
		(pFirstDialog+368)->Show(true,true,false,false,NULL,sText);
	};

	while ((DefCount>0)&&(Result==1))
	{
//Выбор защитника
		DefCount=0;
		if ((iDefender>0)&&(pGuard->ipPlayer!=Attacker->ipPlayer))
		{
			arDef[DefCount]=-1;
			DefCount++;
		};
		
		for (int i=0; i<MAX_HERO; i++)
		{
			if ((arPlayer[ipPlayer].arHero[i])&&(arPlayer[ipPlayer].arHero[i]->pProvince==this)&&(arPlayer[ipPlayer].arHero[i]->iDead<0))
			{			
				arDef[DefCount]=i;
				DefCount++;
			};
		};
	//	char TXT[100];
	//	sprintf(TXT,"DefCount: %d, Def0: %d, Def1: %d  ipPlayer",DefCount,arDef[0],arDef[1]);
	//	StopLog(TXT,ipPlayer);
		int Select=-1;
		if (DefCount>1)
		{
			UI_Mask arMask[9];
			int DD=68;
			UI_TextBox txtChoose;
			BITMAP *bmpBack=create_bitmap(1024,768);			
			blit(screen1,bmpBack,0,0,0,0,1024,768);
			BITMAP *bmpWork=create_bitmap(bmpGUI_DefChoose->w,bmpGUI_DefChoose->h-DD+DD*DefCount);			
			int x=512-bmpGUI_DefChoose->w/2, y=384-bmpGUI_DefChoose->h/2;			
			G_Object *objMain=new G_Object(NULL,x,y,1,bmpWork);			
			objMain=new G_Object(objMain,0,0,0,bmpBack);
			objMain=txtChoose.Init(x+25,y+30,254,90,2,COLOR_1,Font2,objMain);			
			txtChoose.Activate((pFirstMessage+16)->msg);
			blit(bmpGUI_DefChoose,bmpWork,0,0,0,0,bmpGUI_DefChoose->w,131);			
			for (int i=0; i<DefCount; i++)
			{
				blit(bmpGUI_DefChoose,bmpWork,0,131,0,131+i*DD,bmpGUI_DefChoose->w,DD);
			//	draw_sprite(bmpWork,bmpGUI_RazePanel,20,141+i*DD);
				if (arDef[i]<0)
				{
					draw_sprite(bmpWork,(pFirstDefender+iDefender)->Pic,20,141+i*DD);
					textprintf_centre_ex(bmpWork,Font1,180,146+i*DD,COLOR_1,-1,(pFirstDefender+iDefender)->sName);					
				}
				else
				{
					draw_sprite(bmpWork,arPlayer[ipPlayer].arHero[arDef[i]]->smPic,20,141+i*DD);
					textprintf_centre_ex(bmpWork,Font1,180,146+i*DD,COLOR_1,-1,arPlayer[ipPlayer].arHero[arDef[i]]->pstName->str);					
				};				
				objMain=arMask[i].Init(x+14,y+135+i*DD,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objMain,2);				
			};			
			blit(bmpGUI_DefChoose,bmpWork,0,bmpGUI_DefChoose->h-17,0,131+DefCount*DD,bmpGUI_DefChoose->w,17);			
			set_mouse_sprite(bmpMousePointer[1]);
			while (Select<0)
			{				
				for (int i=0; i<DefCount; i++)
				{
					if (arMask[i].Check()==30)
					{
						Select=i;
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
			blit(bmpBack,screen1,0,0,0,0,1024,768);
			destroy_bitmap(bmpWork);
			destroy_bitmap(bmpBack);
		}
		else
		{
			Select=0;
		};
		if (DefCount>0)
		{
			if (arDef[Select]<0)
	//Выбран стражник
			{
				if (arPlayer[Attacker->ipPlayer].Control==1)
				{
					Result=AttackDefender(Attacker, DX, DefCount);
				}
				else
				{
					if (Attacker->ipPlayer==0)
					{
						Result=InitBattle(this,NULL,Attacker,pGuard,0,false);
						if (abs(Result)==1)
						{
							DismissDefender();
						};
					}
					else
					{
						if ((bCrazyAttack)||(CompareStrength1(Attacker,pGuard)>100))
						{
							Result=InitBattle(Attacker->pProvince,this,Attacker,pGuard,0,false);
							Attacker->NiB=0;
							if (abs(Result)==1)
							{
								DismissDefender();
							};
							if (Attacker->pSquad[0]->iLife==0)
							{
								bBury=Attacker->Death(Result);
								if ((DefCount>1)&&((bBury)||(Attacker->pSquad[0]->iLife==0)))
								{
									Result=0;
								};
							};
						}
						else
						{
							DialogDX=DX;
							(pFirstDialog+373)->Show(true,true,false,false,NULL,NULL);
							Result=0;
						};
					};
				};
			}
			else
	//Выбран герой
			{
				if (arPlayer[Attacker->ipPlayer].Control==1)
				{
					char sText[500], sArmy[500];
					int GuardsNum=arPlayer[ipPlayer].arHero[arDef[Select]]->MakeDescription(sArmy);
					sprintf(sText,(pFirstDialog+370)->sText,Attacker->pstName->str,sName,arPlayer[ipPlayer].GetName(),arPlayer[ipPlayer].arHero[arDef[Select]]->pstName->str,GuardsNum,sArmy);
					DialogDX=DX;
					iMessageFor=Attacker->ipPlayer;
					int Answer;
					iDialogButt=1;
					pAdvice=Attacker;
					if (Attacker->CheckSkill(24)>0)
					{
						iScouting=SCOUTING_PRICE;
						if (Attacker->CheckSkill(24)>=5)
						{iScouting/=2;};
						iScGold=iScouting;
						if (iScouting<=arPlayer[Attacker->ipPlayer].Gold)
						{Answer=(pFirstDialog+370)->Show(true,true,true,false,NULL,sText);}
						else
						{Answer=(pFirstDialog+370)->Show(true,true,false,false,NULL,sText);};
						if (Answer==3)
						{
							iDialogButt=1;
							pAdvice=Attacker;
							arPlayer[Attacker->ipPlayer].Gold-=iScGold;
							arPlayer[ipPlayer].arHero[arDef[Select]]->MakeScoutingText();
							Answer=(pFirstDialog+370)->Show(true,true,false,false,NULL,stScouting);
						};
					}
					else
					{Answer=(pFirstDialog+370)->Show(true,true,false,false,NULL,sText);};
			//		int Answer=(pFirstDialog+370)->Show(true,true,false,false,NULL,sText);
					if (Answer==1)
					{
						Result=InitBattle(Attacker->pProvince,this,Attacker,arPlayer[ipPlayer].arHero[arDef[Select]],0,true);
						if (arPlayer[ipPlayer].arHero[arDef[Select]]->pSquad[0]->iLife==0)
						{
							iMessageFor=ipPlayer;
							if (Result==0)
							{arPlayer[ipPlayer].arHero[arDef[Select]]->Death(1);}
							else
							{
								if (Result==1)
								{arPlayer[ipPlayer].arHero[arDef[Select]]->Death(0);}
								else
								{arPlayer[ipPlayer].arHero[arDef[Select]]->Death(Result);};
							};
						};
						if (Attacker->pSquad[0]->iLife==0)
						{
							iMessageFor=Attacker->ipPlayer;
							bBury=Attacker->Death(Result);
							if ((DefCount>1)&&((bBury)||(Attacker->pSquad[0]->iLife==0)))
							{Result=0;};
						};
					}
					else
					{
						DialogDX=DX;
						iMessageFor=ipPlayer;
						(pFirstDialog+373)->Show(true,true,false,false,NULL,NULL);
						Result=0;
					};
				}
				else
				{
					if (Attacker->ipPlayer==0)
					{
						Result=InitBattle(this,NULL,Attacker,arPlayer[ipPlayer].arHero[arDef[Select]],0,true);
					}
					else
					{
						if ((bCrazyAttack)||(CompareStrength1(Attacker,arPlayer[ipPlayer].arHero[arDef[Select]])>100))
						{
							Result=InitBattle(Attacker->pProvince,this,Attacker,arPlayer[ipPlayer].arHero[arDef[Select]],0,true);
							Attacker->NiB=0;
							if (Attacker->pSquad[0]->iLife==0)
							{
								bBury=Attacker->Death(Result);
								if ((DefCount>1)&&((bBury)||(Attacker->pSquad[0]->iLife==0)))
								{Result=0;};
							};
						}
						else
						{
							DialogDX=DX;
							(pFirstDialog+373)->Show(true,true,false,false,NULL,NULL);
							Result=0;
							if (Attacker->iCamper==iNumber)
							{Attacker->iCampTurn++;}
							else
							{
								Attacker->iCamper=iNumber;
								Attacker->iCampTurn=0;
							};
						};					
					};
					if (arPlayer[ipPlayer].arHero[arDef[Select]]->pSquad[0]->iLife==0)
					{
						if (Result==0)
						{arPlayer[ipPlayer].arHero[arDef[Select]]->Death(1);}
						else
						{
							if (Result==1)
							{arPlayer[ipPlayer].arHero[arDef[Select]]->Death(0);}
							else
							{arPlayer[ipPlayer].arHero[arDef[Select]]->Death(Result);}
						};
					};
				};
			};
		};
	};
	}
	else
//Нападение на провинцию AI
	{
//		bool bFight=true;
		while ((DefCount>0)&&(Result==1))
		{
	//Выбор защитника
			DefCount=0;
			int Select=-100;
			dcHero *Defender=NULL;
			if ((iDefender>0)&&(pGuard->ipPlayer!=Attacker->ipPlayer))
			{
				WriteLog("Защитник...");
				DefCount++;
				if (CompareStrength(pGuard,Attacker)>100)
				{
					Select=-1;
					Defender=pGuard;
				};
			};
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[ipPlayer].arHero[i])&&(arPlayer[ipPlayer].arHero[i]->pProvince==this)&&(arPlayer[ipPlayer].arHero[i]->iDead<0))
				{
					DefCount++;
				};
			};
			if (Select==-100)
			{
				for (int i=0; i<MAX_HERO; i++)
				{
					if ((arPlayer[ipPlayer].arHero[i])&&(arPlayer[ipPlayer].arHero[i]->pProvince==this)&&(arPlayer[ipPlayer].arHero[i]->iDead<0))
					{
						WriteLog("Герой...");
				//		DefCount++;
						if ((Select==-100)||(CompareStrength(arPlayer[ipPlayer].arHero[i],Attacker)>CompareStrength(arPlayer[ipPlayer].arHero[Select],Attacker)))
						{
							Select=i;
							Defender=arPlayer[ipPlayer].arHero[i];
						};
					};
				};
			};
			WriteLog("Или Защитник?...");
			if ((Select!=-1)&&(iDefender>0)&&((Select==-100)||(CompareStrength(arPlayer[ipPlayer].arHero[Select],Attacker)<100)))
			{
				Select=-1;
				Defender=pGuard;
			};
			if ((Defender)&&(DefCount>0))
			{
		//Атака нейтралов
				if (Attacker->ipPlayer==0)
				{
					Result=InitBattle(this,NULL,Attacker,Defender,0,false);
					if ((abs(Result)==1)&&(Select==-1))
					{
						DismissDefender();
					}
					else
					{
						/*if (!Defender->pSquad[0])
						{
							for (int i=1; i<MAX_SQUAD; i++)
							{
								if (Defender->pSquad[i])
								{
									Defender->pSquad[0]=Defender->pSquad[i];
									Defender->pSquad[i]=NULL;
									break;
								};
							};
						};*/
						if ((Select>-1)&&(Defender->pSquad[0]->iLife==0))
						{
							if (Result==0)
							{arPlayer[ipPlayer].arHero[Select]->Death(1);}
							else
							{
								if (Result==1)
								{arPlayer[ipPlayer].arHero[Select]->Death(0);}
								else
								{arPlayer[ipPlayer].arHero[Select]->Death(Result);};
							};

						};
					};
				}
				else
				{
					if (arPlayer[Attacker->ipPlayer].Control==1)
		//Атака игрока
					{
						if (Select==-1)
						{
							Result=AttackDefender(Attacker,DX,DefCount);
							/*char sText[500], sArmy[500];
							int GuardsNum=Defender->MakeDescription(sArmy);
							sprintf(sText,(pFirstDialog+369)->sText,Attacker->pstName->str,sName,arPlayer[ipPlayer].GetName(),GuardsNum,sArmy);
							DialogDX=DX;							
							if (arPlayer[Attacker->ipPlayer].iDifficulty<2)
							{
								iHeroAdvice=1;
								iAdvice=CompareStrength1(Attacker,Defender);
								pAdvice=Attacker;
							};
							int Answer;
							if (Attacker->CheckSkill(24)>0)
							{
								iScouting=SCOUTING_PRICE;
								if (Attacker->CheckSkill(24)>=5)
								{iScouting/=2;};
								iScGold=iScouting;
								if (iScouting<=arPlayer[Attacker->ipPlayer].Gold)
								{Answer=(pFirstDialog+369)->Show(true,true,true,false,NULL,sText);}
								else
								{Answer=(pFirstDialog+369)->Show(true,true,false,false,NULL,sText);};
								if (Answer==3)
								{
									arPlayer[Attacker->ipPlayer].Gold-=iScGold;
									Defender->MakeScoutingText();
									Answer=(pFirstDialog+369)->Show(true,true,false,false,NULL,stScouting);
								};
							}
							else
							{Answer=(pFirstDialog+369)->Show(true,true,false,false,NULL,sText);};
					
							if (Answer==1)
							{
								Result=InitBattle(Attacker->pProvince,this,Attacker,Defender,0,true);
								if (abs(Result)==1)
								{
									DismissDefender();
								};
								if (Attacker->pSquad[0]->iLife==0)
								{
									bBury=Attacker->Death(Result);
									if ((DefCount>1)&&((bBury)||(Attacker->pSquad[0]->iLife==0)))
									{Result=0;};
								};
							}
							else
							{Result=0;}*/
						}
						else
						{
							char sText[500], sArmy[500];
							int GuardsNum=Defender->MakeDescription(sArmy);
							sprintf(sText,(pFirstDialog+370)->sText,Attacker->pstName->str,sName,arPlayer[ipPlayer].GetName(),Defender->pstName->str,GuardsNum,sArmy);
							DialogDX=DX;
							int Answer;
							iDialogButt=1;
							pAdvice=Attacker;
							if (Attacker->CheckSkill(24)>0)
							{
								iScouting=SCOUTING_PRICE;
								if (Attacker->CheckSkill(24)>=5)
								{iScouting/=2;};
								iScGold=iScouting;								
								if (iScouting<=arPlayer[Attacker->ipPlayer].Gold)
								{Answer=(pFirstDialog+370)->Show(true,true,true,false,NULL,sText);}
								else
								{Answer=(pFirstDialog+370)->Show(true,true,false,false,NULL,sText);};
								if (Answer==3)
								{
									iDialogButt=1;
									pAdvice=Attacker;
									arPlayer[Attacker->ipPlayer].Gold-=iScGold;
									Defender->MakeScoutingText();
									Answer=(pFirstDialog+370)->Show(true,true,false,false,NULL,stScouting);
								};
							}
							else
							{Answer=(pFirstDialog+370)->Show(true,true,false,false,NULL,sText);};
					//		int Answer=(pFirstDialog+370)->Show(true,true,false,false,NULL,sText);
							if (Answer==1)
							{
								Result=InitBattle(Attacker->pProvince,this,Attacker,Defender,0,true);
								if (Defender->pSquad[0]->iLife==0)
								{
									if (Result==0)
									{arPlayer[ipPlayer].arHero[Select]->Death(1);}
									else
									{
										if (Result==1)
										{arPlayer[ipPlayer].arHero[Select]->Death(0);}
										else
										{arPlayer[ipPlayer].arHero[Select]->Death(Result);}
									};
								};
								if (Attacker->pSquad[0]->iLife==0)
								{
									bBury=Attacker->Death(Result);
									if ((DefCount>1)&&((bBury)||(Attacker->pSquad[0]->iLife==0)))
									{Result=0;};
								};
							}
							else
							{Result=0;};
						};
					}
					else
		//Атака AI
					{
						if ((bCrazyAttack)||(CompareStrength1(Attacker,Defender)>100))
						{
							Result=InitBattle(Attacker->pProvince,this,Attacker,Defender,0,false);
							Attacker->NiB=0;
							if ((abs(Result)==1)&&(Select==-1))
							{
								DismissDefender();
							}
							else
							{
								if ((Defender->pSquad[0]->iLife==0)&&(Select>-1))
								{
									if (Result==0)
									{arPlayer[ipPlayer].arHero[Select]->Death(1);}
									else
									{
										if (Result==1)
										{arPlayer[ipPlayer].arHero[Select]->Death(0);}
										else
										{arPlayer[ipPlayer].arHero[Select]->Death(Result);};
									};
								};
							};
							if (Attacker->pSquad[0]->iLife==0)
							{
								bBury=Attacker->Death(Result);
								if ((DefCount>1)&&((bBury)||(Attacker->pSquad[0]->iLife==0)))
								{Result=0;};
							};
						}
						else
						{
							Result=0;
							if (Select>0)
							{
								if (Attacker->iCamper==iNumber)
								{Attacker->iCampTurn++;}
								else
								{
									Attacker->iCamper=iNumber;
									Attacker->iCampTurn=0;
								};
							};
						};
					};
				};
			};
		};
	};
	return Result;
};

void dcProvince::StaticEvent(int Index, int Player)
{
	//textprintf(screen,Font1,100,100,COLOR_2,"Событие: %d",Index);
	//textprintf(screen1,Font1,100,100,COLOR_2,"Событие: %d",Index);
	//clear_keybuf();
	//readkey();
	clEvent *EV=(pFirstEvent+Index);
	bool bRedraw=true, bGuardGold=false;
	if (arPlayer[Player].Control!=1)
	{bRedraw=false;};
	char Mess[1000];
	int AttLevel=-1;
	for (int i=0; i<6; i++)
	{
		Treasurez[i][0]=0;
		Treasurez[i][1]=0;
	};
	int Lootz=1;
	
//Изменение кармы
	arPlayer[ipPlayer].iKarma+=EV->iKarma;
	arPlayer[ipPlayer].iKarma+=EV->CalcEffect(7,1,arPlayer[ipPlayer].iKarma);

//Воздействие эффектов
	long int Mult=0, Count, iWork;
	//Индивидуальные эффекты
	for (int Eff=0; Eff<MAX_EVENT_EFFECT; Eff++)
	{
		switch (EV->arEffect[Eff][0])
		{		
		case 9:
			PlaceSite(EV->arEffect[Eff][1],CurGame.iCurrentTurn/20,0,EV->arEffect[Eff][2],EV->arEffect[Eff][3]);
			break;
		case 12:
			if (*(arPlayer[ipPlayer].pOuterBld+EV->arEffect[Eff][1])>=0)
			{
				*(arPlayer[ipPlayer].pOuterBld+EV->arEffect[Eff][1])+=EV->arEffect[Eff][2];
				if (Lootz<6)
				{
					for (int k=1; k<iMaxItem; k++)
					{
						if (((pFirstItem+k)->arEffect[0][0]==87)&&((pFirstItem+k)->arEffect[0][1]==EV->arEffect[Eff][1]))
						Treasurez[Lootz][0]=k;
						Treasurez[Lootz][1]=1;
					};
					Lootz++;
				};
			}
			break;
		case 13:
			for (int i=0; i<EV->arEffect[Eff][2]; i++)
			{
				if (Lootz<6)
				{
					Treasurez[Lootz][0]=iMaxPureItem+EV->arEffect[Eff][1];
					Treasurez[Lootz][1]=1;
					Lootz++;
				};
				Count=0;			
				while ((arPlayer[ipPlayer].arTreasury[Count][0]>0)&&(Count<=TREASURY_SIZE))
				{Count++;};
				if (Count<TREASURY_SIZE)
				{
					arPlayer[ipPlayer].arTreasury[Count][0]=iMaxPureItem+EV->arEffect[Eff][1];
					arPlayer[ipPlayer].arTreasury[Count][1]=1;
				};
			};
			break;
		case 14:
			Count=0;
			ClearRoll();
			for (int i=1; i<=iMaxPureItem; i++)
			{
				if (((pFirstItem+i)->iShopLevel==EV->arEffect[Eff][1])&&((pFirstItem+i)->iRarity>=EV->arEffect[Eff][2]))
				{
					RollerN[Count]=i;
					RollerP[Count]=7-(pFirstItem+i)->iRarity;
					Count++;
				};
			};
			iWork=Roll(false);
			if (Lootz<6)
			{
				Treasurez[Lootz][0]=iWork;
				Treasurez[Lootz][1]=(pFirstItem+iWork)->iDurability;
				Lootz++;
			};
			Count=0;
			while ((arPlayer[ipPlayer].arTreasury[Count][0]>0)&&(Count<=TREASURY_SIZE))
			{Count++;};
			if (Count<TREASURY_SIZE)
			{
				arPlayer[ipPlayer].arTreasury[Count][0]=iWork;
				arPlayer[ipPlayer].arTreasury[Count][1]=(pFirstItem+iWork)->iDurability;
			};
			break;
		case 15:
			AttLevel=EV->arEffect[Eff][1];
			break;
		case 16:
			for (int i=0; i<EV->arEffect[Eff][2]; i++)
			{
				Count=0;			
				while ((arPlayer[ipPlayer].arTreasury[Count][0]>0)&&(Count<=TREASURY_SIZE))
				{Count++;};
				if (Count<TREASURY_SIZE)
				{
					arPlayer[ipPlayer].arTreasury[Count][0]=EV->arEffect[Eff][1];
					arPlayer[ipPlayer].arTreasury[Count][1]=(pFirstItem+EV->arEffect[Eff][1])->iDurability*EV->arEffect[Eff][3]/100;
					if (Lootz<6)
					{
						Treasurez[Lootz][0]=EV->arEffect[Eff][1];
						Treasurez[Lootz][1]=(pFirstItem+EV->arEffect[Eff][1])->iDurability*EV->arEffect[Eff][3]/100;
						Lootz++;
					};
				};
			};
			break;
		case 17:
			*(arPlayer[Player].pStory+EV->arEffect[Eff][1])=EV->arEffect[Eff][2]+Random(EV->arEffect[Eff][3]);
			break;
		case 18:
			switch (EV->arEffect[Eff][2])
			{
			case 0:
				*(arPlayer[Player].pPlace+EV->arEffect[Eff][1])=0;
				break;
			case 1:
				*(arPlayer[Player].pPlace+EV->arEffect[Eff][1])=iNumber;
				break;
			case 2:
				*(arPlayer[Player].pPlace+EV->arEffect[Eff][1])=arPlayer[Player].ipProvince;
				break;
			};
			break;
		case 19:
			if (EV->arEffect[Eff][1]>0)
			{
				PlaceDefender(EV->arEffect[Eff][1]);
			}
			else
			{
				if (pGuard)
				{
					DismissDefender();
				};
			};
			break;
		case 20:
			for (int i=0; i<MAX_PROVINCE_BLD; i++)
			{
				if (arBuilding[i]==EV->arEffect[Eff][1])
				{RazeBuilding(i,true);};
			};
			break;
		case 21:
			ConstructBuilding(EV->arEffect[Eff][1],bRedraw, false);
			break;
		case 24:
			iWork=-1;
			for (int i=0; i<MAX_PROVINCE_SITE; i++)
			{
				if ((arSite[i][0]==EV->arEffect[Eff][1])&&(arSite[i][1]<=iExplore))
				{iWork=i;};
			};
			if (iWork>=0)
			{
				arSite[iWork][0]=0;
				if (arSiteGuard[iWork])
				{DismissSiteGuard(iWork);};
			}
			else
			{
				for (int i=0; i<MAX_PROVINCE_SITE; i++)
				{
					if (arSite[i][0]==EV->arEffect[Eff][1])
					{iWork=i;};
				};
				if (iWork>=0)
				{
					arSite[iWork][0]=0;
					if (arSiteGuard[iWork])
					{DismissSiteGuard(iWork);};
				};
			};
			break;
		case 25:
			iWork=EV->arEffect[Eff][1];
			for (int i=MAX_PROVINCE_BLD-1; i>=0; i--)
			{
				if ((arBuilding[i])&&(iWork>0)&&((pFirstOuterBld+arBuilding[i])->iWall<=0))
				{
					RazeBuilding(i,bRedraw);
					iWork--;
				};
			};
			break;
		case 26:
			for (int i=1; i<=MapSize; i++)
			{
				if ((pFirstProvince+i)->ipRace==EV->arEffect[Eff][1])
				{
					(pFirstProvince+i)->ChangeAttitude(ipPlayer,EV->arEffect[Eff][2]);
				};
			};
			break;
		case 27:
			iWork=EV->arEffect[Eff][2]+Random(EV->arEffect[Eff][3]);
			CurGame.ResPrices[EV->arEffect[Eff][1]]+=iWork;
			if (Eff==0)
			{Mult=abs(iWork);};
			break;
		case 29:
			if (pGuard)
			{pGuard->ChangeLife(EV->arEffect[Eff][1],EV->arEffect[Eff][2],EV->arEffect[Eff][3]);};
			if (pGarrison)
			{pGarrison->ChangeLife(EV->arEffect[Eff][1],EV->arEffect[Eff][2],EV->arEffect[Eff][3]);};
			for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
			{
				if (arPlayer[PL].bInGame)
				{
					for (int HR=0; HR<MAX_HERO; HR++)
					{
						if ((arPlayer[PL].arHero[HR])&&(arPlayer[PL].arHero[HR]->iDead<0)&&(arPlayer[PL].arHero[HR]->pProvince==this))
						{
							arPlayer[PL].arHero[HR]->ChangeLife(EV->arEffect[Eff][1],EV->arEffect[Eff][2],EV->arEffect[Eff][3]);
						};
					};
				};
			};
			break;
		case 30:
			if (pGarrison)
			{pGarrison->ChangeMorale(EV->arEffect[Eff][1]+Random(EV->arEffect[Eff][2]),EV->arEffect[Eff][3]);};
			if (pGuard)
			{pGuard->ChangeMorale(EV->arEffect[Eff][1]+Random(EV->arEffect[Eff][2]),EV->arEffect[Eff][3]);};
			for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
			{
				if (arPlayer[PL].bInGame)
				{
					for (int HR=0; HR<MAX_HERO; HR++)
					{
						if ((arPlayer[PL].arHero[HR])&&(arPlayer[PL].arHero[HR]->iDead<0)&&(arPlayer[PL].arHero[HR]->pProvince==this))
						{
							arPlayer[PL].arHero[HR]->ChangeMorale(EV->arEffect[Eff][1]+Random(EV->arEffect[Eff][2]),EV->arEffect[Eff][3]);
						};
					};
				};
			};
			break;
		case 31:
			Rebel(this,EventDX);
			break;
		case 32:
			iFort+=EV->arEffect[Eff][1]+Random(EV->arEffect[Eff][2]);
			if (iFort<0)
			{iFort=0;};
			if (iFort>GetWall())
			{iFort=GetWall();};
			break;
		case 33:
			Campaign.arDialog[EV->arEffect[Eff][1]]=EV->arEffect[Eff][2];
			break;
		case 34:
			Campaign.arCampPlayer[0].iEnergy+=EV->arEffect[Eff][1]+Random(EV->arEffect[Eff][2]);
			break;
		case 35:
			if (CurGame.iGameType==2)
			{
				int Count=0, Build=0;
				ClearRoll();
				for (int i=1; i<=iMaxInnerBld; i++)
				{
					if ((GetBuildingStatus(i,0)==1)&&(IsBuildingFree(i)))
					{
						RollerN[Count]=i;
						RollerP[Count]=10;
						Count++;
					};			
				};
				if (Count>0)
				{Build=Roll(false);}
				else
				{
					for (int i=1; i<=iMaxInnerBld; i++)
					{						
						if (GetBuildingStatus(i,0)==1)
						{
							bool bAdd=true;
							if (Campaign.GetActiveShard())
							{
								bAdd=true;
								for (int k=0; k<MAX_SHARD_BUILD; k++)
								{
									if (Campaign.GetActiveShard()->arBuilding[k]==i)
									{bAdd=false;}
								};
							};
							if (bAdd)
							{
								RollerN[Count]=i;
								RollerP[Count]=10;
								Count++;
							};
						};			
					};
					if (Count>0)
					{Build=Roll(false);} ;
				};
				if (Build>0)
				{
					Treasurez[Lootz][0]=-Build;					
					Lootz++;
					Campaign.arCampPlayer[0].arBld[Build]=1;
				};
			};
			break;
		case 36:
			if (pGarrison)
			{
				for (int i=0; i<EV->arEffect[Eff][2]; i++)
				{
					int Slot=pGarrison->GetFreeUnitSlot((pFirstUnitType+EV->arEffect[Eff][1])->iLevel);
					if (Slot>0)
					{
						dcUnit *pUnit=new dcUnit(EV->arEffect[Eff][1],Random(EV->arEffect[Eff][3]+1));
						pGarrison->PlaceUnit(pUnit,Slot);
					};
				};
			};
			break;
		case 37:
			if (EV->arEffect[Eff][1]>0)
			{
				RunCampaignEvent(EV->arEffect[Eff][1], 0, 0);
				return;
			};
			break;
		case 38:
			bGuardGold=true;
			break;
		case 39:
			DismissDefender();
			MakeNeutral();
			break;
		case 40:		
			ShopRoller(EV->arEffect[Eff][1],EV->arEffect[Eff][2],EV->arEffect[Eff][3],true);
			iWork=Roll(false);
			if (Lootz<6)
			{
				Treasurez[Lootz][0]=iWork;
				Treasurez[Lootz][1]=(pFirstItem+iWork)->iDurability;
				Lootz++;
			};
			switch ((pFirstItem+iWork)->ipClass)
			{
			case 11:
				if (*(arPlayer[Player].pDefender+(pFirstItem+iWork)->arEffect[0][1])>=0)
				{*(arPlayer[Player].pDefender+(pFirstItem+iWork)->arEffect[0][1])+=1;};
				break;
			case 12:
				if (*(arPlayer[Player].pOuterBld+(pFirstItem+iWork)->arEffect[0][1])>=0)
				{*(arPlayer[Player].pOuterBld+(pFirstItem+iWork)->arEffect[0][1])+=1;};
				break;
			case 13:
				if (*(arPlayer[Player].pRitual+(pFirstItem+iWork)->arEffect[0][1])>=0)
				{*(arPlayer[Player].pRitual+(pFirstItem+iWork)->arEffect[0][1])+=1;};
				break;
			default:			
				Count=0;
				while ((arPlayer[Player].arTreasury[Count][0]>0)&&(Count<=TREASURY_SIZE))
				{Count++;};
				if (Count<TREASURY_SIZE)
				{
					arPlayer[Player].arTreasury[Count][0]=iWork;
					arPlayer[Player].arTreasury[Count][1]=(pFirstItem+iWork)->iDurability;
				};
				break;
			}
			break;
		case 41:
			for (int i=0; i<TREASURY_SIZE; i++)
			{
				if (arPlayer[Player].arTreasury[i][0]==EV->arEffect[Eff][1])
				{
					arPlayer[Player].arTreasury[i][0]=0;
					arPlayer[Player].arTreasury[i][1]=0;
					break;
				};
			};
			break;
		case 42:
			if (*(arPlayer[ipPlayer].pDefender+EV->arEffect[Eff][1])>=0)
			{
				*(arPlayer[ipPlayer].pDefender+EV->arEffect[Eff][1])+=EV->arEffect[Eff][2];
				if (Lootz<6)
				{
					for (int k=1; k<iMaxItem; k++)
					{
						if (((pFirstItem+k)->arEffect[0][0]==86)&&((pFirstItem+k)->arEffect[0][1]==EV->arEffect[Eff][1]))
						Treasurez[Lootz][0]=k;
						Treasurez[Lootz][1]=1;
					};
					Lootz++;
				};
			}
			break;
		case 43:
			WorldRules.arTrigger[EV->arEffect[Eff][1]]=EV->arEffect[Eff][2];
			break;
		case 44:
			switch (EV->arEffect[Eff][1])
			{
			case 24:
				if (arPlayer[Player].GetResource(EV->arEffect[Eff][2])>0)
				{
					StaticEvent(EV->arEffect[Eff][3],Player);
					return;
				}
				break;
			}
			break;
		case 45:
			iWork=PlaceSite(EV->arEffect[Eff][1],CurGame.iCurrentTurn/20,0,-1,0);
			if (iWork>=0)
			{				
				arSite[iWork][5]=-1;
				PlaceSiteGuard((pFirstGuardType+EV->arEffect[Eff][2])->arGuard[EV->arEffect[Eff][3]],iWork,(pFirstGuardType+EV->arEffect[Eff][2])->iKarma);
				arSite[iWork][4]=EV->arEffect[Eff][2];
			}
			break;
		case 46:
			Victory(Player);
			return;
			break;

		};
	};
	//Обычные эффекты
	iWork=EV->CalcEffect(1,GetLevel(),arPlayer[ipPlayer].Gold);
	if ((iWork<0)&&(bGuardGold))
	{iWork-=iWork*arPlayer[ipPlayer].GetBldParam1(33)/10;};
//	StopLog("Золото:", iWork);
	arPlayer[ipPlayer].Gold+=iWork;
	Treasurez[0][0]=iWork;
	if (EV->arEffect[0][0]==1)
	{
		Mult=abs(iWork);
		/*if (Pirate1+Pirate2+Pirate3+Pirate4>0)
		{
			FileLog(sName);
			delete (pFirstProvince+12);
		};*/
	};
	ChangeAttitude(ipPlayer,EV->CalcEffect(2,GetLevel(),arAttitude[ipPlayer][0]));
	iWork=GetLevel();
	iPopulation+=iPopulation*EV->CalcEffect(3,GetLevel(),iPopulation)/100;
	if (iWork!=GetLevel())
	{DrawPic(bRedraw);};
	int Inc1=GetGoldIncome();
	iGoldInc+=EV->CalcEffect(5,GetLevel(),iGoldInc);
	if (iGoldInc<0)
	{iGoldInc=0;};
	int Inc2=GetGoldIncome();
	if (EV->arEffect[0][0]==5)
	{Mult=Inc2-Inc1;};
	iWork=EV->CalcEffect(6,GetLevel(),arPlayer[ipPlayer].Gem);
	arPlayer[ipPlayer].Gem+=iWork;
	Treasurez[0][1]=iWork;
	if (EV->arEffect[0][0]==6)
	{Mult=abs(iWork);};
	arUnrest[ipPlayer]+=EV->CalcEffect(10,GetLevel(),arUnrest[ipPlayer]);
	if (arUnrest[ipPlayer]<0)
	{arUnrest[ipPlayer]=0;};
	if (arUnrest[ipPlayer]>MAX_UNREST)
	{arUnrest[ipPlayer]=MAX_UNREST;};
	arPlayer[Player].iCorrupt+=EV->CalcEffect(22,GetLevel(),arPlayer[Player].iCorrupt);
	Inc1=GetGemIncome();
	iGemInc+=EV->CalcEffect(23,GetLevel(),iGemInc);
	if (iGemInc<0)
	{iGemInc=0;};
	Inc2=GetGemIncome();
	if (EV->arEffect[0][0]==23)
	{Mult=Inc2-Inc1;};

	if (EV->ipDialog)
	{
//Вызов диалога
		Mult=abs(Mult);
		if (Mult==0)
		{Mult=1;};
		if (EV->iDlgParam1==0)
		{sprintf(Mess,(pFirstDialog+EV->ipDialog)->sText);}
		if (EV->iDlgParam1==1)
		{
			if (EV->iDlgParam2==2)
				{sprintf(Mess,(pFirstDialog+EV->ipDialog)->sText,sName,Mult);}
			else
				{sprintf(Mess,(pFirstDialog+EV->ipDialog)->sText,sName);}
		};
		if (EV->iDlgParam1==2)
		{
			if (EV->iDlgParam2==1)
				{sprintf(Mess,(pFirstDialog+EV->ipDialog)->sText,Mult,sName);}
			else
				{sprintf(Mess,(pFirstDialog+EV->ipDialog)->sText,Mult);}
		};
		bool An[MAX_ANSWER];
		for (int i=0; i<MAX_ANSWER; i++)
		{
			bool bWork;
			An[i]=false;
			if ((pFirstDialog+EV->ipDialog)->arAnswer[i][1]!='#')			
			{
				An[i]=true;
				for (int j=0; j<MAX_EVENT_EFFECT;j++)
				{
					if ((EV->arEffect[j][0]==4)&&(EV->arEffect[j][1]==i+1))
					{							
						switch (EV->arEffect[j][2])
						{
						case 1:
							if (arPlayer[ipPlayer].Gold<EV->arEffect[j][3])
							{An[i]=false;};
							break;
						case 2:
							if (arPlayer[ipPlayer].Gem<EV->arEffect[j][3])
							{An[i]=false;};
							break;
						case 3:
							bWork=true;
							for (int j=0; j<MAX_HERO; j++)
							{
								if ((arPlayer[ipPlayer].arHero[j])&&(arPlayer[ipPlayer].arHero[j]->pProvince==this)&&(arPlayer[ipPlayer].arHero[j]->iDead<0))
								{bWork=false;};
							};
							if (iDefender>0)
							{bWork=false;};
							if (bWork)
							{An[i]=false;};
							break;
						};							
					};	
				};
				/*switch (EV->GetParam1(4,i+1))
				{
				case 0:
					An[i]=true;
					break;
				case 1:
					if (arPlayer[ipPlayer].Gold>=EV->GetParam2(4,i+1))
					{An[i]=true;};
					break;
				case 2:
					if (arPlayer[ipPlayer].Gem>=EV->GetParam2(4,i+1))
					{An[i]=true;};
					break;
				case 3:
					for (int j=0; j<MAX_HERO; j++)
					{
						if ((arPlayer[ipPlayer].arHero[j])&&(arPlayer[ipPlayer].arHero[j]->pProvince==this))
						{An[i]=true;};
					};
					if (iDefender>0)
					{An[i]=true;};
					break;
				};		*/			
			};
		};
		int Answer;
		if (arPlayer[Player].Control==1)
		{
			DialogDX=EventDX;
			Answer=(pFirstDialog+EV->ipDialog)->Show(An[0],An[1],An[2],An[3],NULL,Mess)-1;
		}
		else
//Выбор ответа для ИИ
		{
			switch (Index)
			{
			case 925:	//Дары небес
				Answer=2;
				if (GetMood()<-2)
				{Answer=3;};
				if (arPlayer[Player].Gold<200+CurGame.iCurrentTurn)
				{Answer=1;};
				if (arPlayer[Player].Gem<50+CurGame.iCurrentTurn/3)
				{Answer=0;};
				break;
			default:
				Answer=0;
				break;
			};
		};
		if (EV->arEvent[Answer]>0)
		{
			StaticEvent(EV->arEvent[Answer],Player);
			return;
		};
	};

	if (EV->ipAttacker>0)
	{
//Вызов сражения
		dcProvince Fake;
		Fake.sName=new char [3];		
		int Lvl=Random(CurGame.iCurrentTurn)/50;
		if (AttLevel>=0)
		{Lvl=AttLevel;};
		if (Lvl>3)
		{Lvl=3;};
		if (Lvl<0)
		{Lvl=0;};
		while ((Lvl>0)&&((pFirstGuard+(pFirstGuardType+EV->ipAttacker)->arGuard[Lvl])->Power>CurGame.Map.Neutrals))
		{Lvl--;};
		Fake.PlaceGuard((pFirstGuardType+EV->ipAttacker)->arGuard[Lvl],(pFirstGuardType+EV->ipAttacker)->iKarma);
		Fake.pGuard->ipPlayer=0;
		int Battle=UnderAttack(Fake.pGuard,200);
		switch (Battle)
		{
		case -1:
			if (EV->ipEventDraw>0)
			{StaticEvent(EV->ipEventDraw,Player);};
			break;
		case 1:
			if (EV->ipProvType>0)
			{
				iDefender=0;								
				pGuard=Fake.pGuard;
				ipType=EV->ipProvType;
				Fake.pGuard=NULL;
				bool bTakeIt=true;
				if (pGarrison)
				{
					for (int i=0; i<MAX_SQUAD; i++)
					{
						if (pGarrison->pSquad[i])
						{bTakeIt=false;};
					};
				};
				if (bTakeIt)
				{
					Conquer(0,NULL);
					if (EV->ipEventLose>0)
					{StaticEvent(EV->ipEventLose,Player);};
				}
				else
				{
					ipEvent=EV->ipEventLose;
				};				
			}
			else
			{
				if (EV->ipEventLose>0)
				{StaticEvent(EV->ipEventLose,Player);};
			};
			break;
		case 0:
			if (EV->ipEventWin>0)
			{StaticEvent(EV->ipEventWin,Player);};
		};
	};
//Вызов следующего события
	for (int Eff=0; Eff<MAX_EVENT_EFFECT; Eff++)
	{
		if  (EV->arEffect[Eff][0]==11)
		{
			if ((EV->arEffect[Eff][2]!=0)&&(Random(100)>EV->arEffect[Eff][3]))
			{
				if (EV->arEffect[Eff][2]>0)
				{
					StaticEvent(EV->arEffect[Eff][2],Player);
					return;
				};
			}
			else
			{
				StaticEvent(EV->arEffect[Eff][1],Player);
				return;
			};
		};
	};
};

int dcProvince::GetFreeSpace(int Hidden)
{
	if (Hidden>0)
	{
		bool Nothing=true;
		for (int i=iExplore+1; i<100; i++)
		{
			bool Free=true;
			for (int j=0; j<MAX_PROVINCE_SITE; j++)
			{
				if ((arSite[j][0]>0)&&(arSite[j][1]==i))
				{Free=false;};
			};
			if (Free)
			{
				Nothing=false;
			};
		};
		if (Nothing)
		{return 0;};
	};
	if (Hidden<0)
	{
		bool Nothing=true;
		for (int i=1; i<=iExplore; i++)
		{
			bool Free=true;
			for (int j=0; j<MAX_PROVINCE_SITE; j++)
			{
				if ((arSite[j][0]>0)&&(arSite[j][1]==i))
				{Free=false;};
			};
			if (Free)
			{
				Nothing=false;
			};
		};
		if (Nothing)
		{return 0;};
	};
	bool bPlacing=true;
	int Result;
	while (bPlacing)
	{
		Result=Random(99)+1;
		if (Result<50)
		{Result=Random(99)+1;};
		if ((Hidden==0)||((Hidden>0)&&(Result>iExplore))||((Hidden<0)&&(Result<=iExplore)))
		{
			bPlacing=false;
			for (int j=0; j<MAX_PROVINCE_SITE; j++)
			{
				if ((arSite[j][0]>0)&&(arSite[j][1]==Result))
				{
					bPlacing=true;
				};
			};
		};
	};
	return Result;
};

int dcProvince::PlaceSite(int Index, int Level, int Quest, int Hidden, int Guard)
{
	if (Level<0)
	{Level=0;};
	//Поиск места в провинции
	int Place=0;
	while ((arSite[Place][0]>0)&&(Place<MAX_PROVINCE_SITE))
	{Place++;};
	if (Place>=MAX_PROVINCE_SITE)
	{return -1;};
	clSite *pSite=(pFirstSite+Index);
	if (Hidden==0)
	{Hidden=pSite->iPlace-1;};
	int Scale=GetFreeSpace(Hidden);
	if (Scale==0)
	{return -1;};
	arSite[Place][1]=Scale;
	
	arSite[Place][0]=Index;
	arSite[Place][2]=Level*2+Random(2);
	if (arSite[Place][1]>60+Random(40))
	{
		arSite[Place][2]+=Random((arSite[Place][1]-60)/11);
	};
	arSite[Place][2]+=pSite->iTreasure;
	if (arSite[Place][2]>10)
	{arSite[Place][2]=10;};
	arSite[Place][3]=Quest;
	arSite[Place][4]=0;
	arSite[Place][5]=0;	
	//Установка охраны
	if (((Guard>0)&&(pSite->iGuarded>0))||((Random(100)<pSite->iGuarded)&&(Guard>=0)))
	{
		ClearRoll();
		int Counter=0;
		for (int j=0; j<MAX_SITE_GUARD; j++)
		{
			if (pSite->arGuard[j][0]>0)
			{
				RollerN[Counter]=pSite->arGuard[j][0];
				RollerP[Counter]=pSite->arGuard[j][1];
				Counter++;
			};
		};
		arSite[Place][4]=Roll(false);
		int GrdLvl=-1, GrdPower, Grd=arSite[Place][4];
		for (int i=0; i<MAX_SITE_ABILITY; i++)
		{
			if (pSite->arAbility[i][0]==8)
			{GrdLvl=pSite->arAbility[i][1];};
		};
		if (GrdLvl>=0)
		{
			if (GrdLvl>3){GrdLvl=3;};
		}
		else
		{
			GrdLvl=0;		
			GrdPower=1+Level*3+Random(Level+1)-Random(Level+1);
			if (arSite[Place][1]>60+Random(30))
			{
				GrdPower+=Random(Level+4);
			};
			if (GrdPower>CurGame.Map.Neutrals)
			{GrdPower=CurGame.Map.Neutrals;};

			for (int k=1; k<4; k++)
			{
				if (abs((pFirstGuard+(pFirstGuardType+Grd)->arGuard[GrdLvl])->Power-GrdPower)>abs((pFirstGuard+(pFirstGuardType+Grd)->arGuard[k])->Power-GrdPower))
				{GrdLvl=k;};
			};
		}
		GrdPower=(pFirstGuard+(pFirstGuardType+Grd)->arGuard[GrdLvl])->Power;				
		arSite[Place][2]=GrdPower+pSite->iTreasure;
//		textprintf(screen,Font1,10,500,COLOR_2,"Охранник:%d Уровень:%d (%d)       ",arSite[Place][4],GrdLvl,(pFirstGuardType+arSite[Place][4])->arGuard[GrdLvl]);
		PlaceSiteGuard((pFirstGuardType+arSite[Place][4])->arGuard[GrdLvl],Place,(pFirstGuardType+arSite[Place][4])->iKarma);
		if (abs(arSite[Place][2])>100)
		{
			FileLog("Site Loot!!!");
		};
	};
	//Установка рандомных квестов
	if (Quest==0)
	{
		for (int i=0; i<MAX_SITE_ABILITY; i++)
		{
			if ((pSite->arAbility[i][0]==15)&&(Random(100)<pSite->arAbility[i][2]))
			{
				Quest=pSite->arAbility[i][1];
			};
		};
		if (Quest>0)
		{
			arSite[Place][3]=Quest;
			clQuest *pQuest=(pFirstQuest+Quest);
			if (pQuest->iType==7)
			{
				if (pQuest->Index[0]==1)
				{
					arSite[Place][5]=iMaxPureItem+3+Random(iMaxSpell-3);
				};
			};			
		}
		else
		{
			for (int i=1; i<=iMaxQuest; i++)
			{
				if (((pFirstQuest+i)->iGiver==Index)&&((pFirstQuest+i)->iType==8))
				{arSite[Place][3]=9999;};
			};
		};
	};
	//Заполнение магазина
	int iShop=-999, iLevel, Items=8, Prices=300;
	for (int k=0; k<MAX_SITE_ABILITY; k++)
	{
		if (pSite->arAbility[k][0]==13)
		{
			iShop=pSite->arAbility[k][1];
			iLevel=pSite->arAbility[k][2];
			if (Random(10)==0)
			{
				iLevel++;
				if (Random(10)==0)
				{iLevel++;};
			};
		};
		if (pSite->arAbility[k][0]==14)
		{
			Items=pSite->arAbility[k][1];
			Prices=pSite->arAbility[k][2];
		};
	};
	if (iShop!=-999)
	{
		arSite[Place][2]=iMaxShop;
		dcShop *pNewShop=new dcShop[iMaxShop+1], *Shop;
		if (pFirstShop)
		{
			for (int i=0; i<iMaxShop; i++)
			{
				*(pNewShop+i)=*(pFirstShop+i);
			};
			delete [] pFirstShop;
		};
		pFirstShop=pNewShop;
		Shop=(pFirstShop+iMaxShop);
		iMaxShop++;
		ShopRoller(iShop,iLevel,0,false);
		Shop->Type=iShop;
		for (int k1=0; k1<Items; k1++)
		{
			Shop->Items[k1][0]=Roll(true);			
			if ((pFirstItem+Shop->Items[k1][0])->iRarity==1)
			{Shop->Items[k1][1]=0;}
			else
			{Shop->Items[k1][1]=1+Random(6-(pFirstItem+Shop->Items[k1][0])->iRarity);};
		};
		for (int k1=0; k1<MAX_PLAYERS; k1++)
		{
			Shop->PriceModifier[k1]=Prices;
		};				
	};
	if (abs(arSite[Place][2])>100)
	{
		FileLog("Site Loot!!!");
	};
	return Place;
};

void dcProvince::ConstructBuilding(int Build, bool Redraw, bool Pay)
{
	int Slot=0;
	clOuterBuilding *BLD=(pFirstOuterBld+Build);	
	while ((Slot<MAX_PROVINCE_BLD)&&(arBuilding[Slot]!=BLD->ipUpgrade))
	{
		Slot++;
	};
	if (Slot>=MAX_PROVINCE_BLD)
	{
		if ((Pay)||(BLD->ipUpgrade==0))
		{return;};
		Slot=0;
		while ((Slot<MAX_PROVINCE_BLD)&&(arBuilding[Slot]>0))
		{
			Slot++;
		};
		if (Slot>=MAX_PROVINCE_BLD)
		{return;};
	};
	if (BLD->iWall>0)
	{
		if (arBuilding[Slot]>0)
		{
			iFort+=(BLD->iWall-(pFirstOuterBld+arBuilding[Slot])->iWall);
		}
		else
		{
			iFort=BLD->iWall;
		};
	};
	arBuilding[Slot]=Build;
	arPlayer[ipPlayer].iKarma+=BLD->iKarma;
	bool bGarrison=false;
	for (int i=0; i<MAX_BUILDING_ABILITY; i++)
	{
		if (BLD->arAbility[i][0]==4)
		{
			bGarrison=true;
		};
		if (BLD->arAbility[i][0]==12)
		{
			PlaceSite(BLD->arAbility[i][1],BLD->arAbility[i][2],0,0,0);
		};
	};
	if (bGarrison)
	{
		if (!(pGarrison))
		{
			pGarrison=new dcHero;
			pGarrison->ipPlayer=ipPlayer;
			pGarrison->ipProvince=iNumber;
			pGarrison->pProvince=this;
		};
		for (int i=0; i<MAX_BUILDING_ABILITY; i++)
		{
			if (BLD->arAbility[i][0]==4)
			{
				pGarrison->arGarrison[BLD->arAbility[i][1]-1]=BLD->arAbility[i][2];						
			};
		};
	};
	if (Pay)
	{
		arPlayer[ipPlayer].iCanOuterBld--;
		if (*(arPlayer[ipPlayer].pOuterBld+Build)>0)
		{
			*(arPlayer[ipPlayer].pOuterBld+Build)=*(arPlayer[ipPlayer].pOuterBld+Build)-1;
		};	
		arPlayer[ipPlayer].Gold-=arPlayer[ipPlayer].GetOutBldGoldPrice(Build);
		arPlayer[ipPlayer].Gem-=arPlayer[ipPlayer].GetOutBldGemPrice(Build);
		for (int i=0; i<MAX_BUILDING_RES; i++)
		{
			if ((BLD->arRes[i]>0)&&
			(!arPlayer[ipPlayer].GetResource(BLD->arRes[i])))
			{
				CurGame.ResPrices[BLD->arRes[i]]+=CurGame.ResBuy;
			};
		};
	};
//	if (BuildPic[Slot])
//	{destroy_bitmap(BuildPic[Slot]);};
	//BuildPic[Slot]=MakeObstracles((pFirstOuterBld+Build)->bmpBld,x-100-(Slot%2)*20,y-85+(2-Slot)*30);
	DrawPic(Redraw);
};

int dcProvince::GetSiegeIndex()
{
	if ((pGuard)&&(pGuard->ipPlayer!=ipPlayer))
	{return 0;};
	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		if ((PL!=ipPlayer)&&(arPlayer[PL].bInGame))
		{
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[PL].arHero[i])&&(arPlayer[PL].arHero[i]->ipProvince==iNumber)&&(arPlayer[PL].arHero[i]->iDead<0))
				{return PL;};
			};
		};
	};
	return -1;
};

void dcProvince::RepairWalls()
{
	if ((pGarrison)&&(GetSiegeIndex()<0))
	{
		int Repair=10, Wall;
		if ((iDefender>0)&&(ipPlayer>0))
		{
			for (int i=0; i<MAX_DEFENDER_ABILITY; i++)
			{
				if ((pFirstDefender+i)->arAbility[i][0]==8)
				{
					Repair+=(pFirstDefender+i)->arAbility[i][1];
				};
			};
		};
		if ((ipPlayer==0)||(arPlayer[ipPlayer].ipProvince!=iNumber))
		{
			for (int i=0; i<MAX_PROVINCE_BLD; i++)
			{
				if ((arBuilding[i])&&(pFirstOuterBld+arBuilding[i])->iWall>0)
				{Wall=(pFirstOuterBld+arBuilding[i])->iWall;};
			}
		}
		else
		{
			Wall=arPlayer[ipPlayer].iWall;			
		};
		if (iFort<Wall)
		{iFort+=Wall*Repair/100;};
		if (iFort>Wall)
		{iFort=Wall;};
	};
};

void dcProvince::Conquer(int Player, dcHero *HR)
{
	int Loser=ipPlayer;
	if (ipPlayer==0)
	{
		ChangeAttitude(Player,(pFirstProvinceType+ipType)->iConqUnrest);
		arPlayer[Player].iKarma+=(pFirstProvinceType+ipType)->iConqKarma;
	};
	if (arPlayer[ipPlayer].ipProvince==iNumber)
	{arPlayer[Player].arKills[ipPlayer]=1;};
	ipPlayer=Player;
	if (pGarrison)
	{
		pGarrison->ipPlayer=Player;
	};
	if (HR)
	{
		HR->ipProvince=iNumber;
		HR->pProvince=this;
	};
	if (Loser>0)
	{
		CheckPlayerCondition(Loser, false);
	};
};

int dcProvince::GetRitualBonus(int Param, int Index)
{
	int Result=0;
	dcRitual *RR=pRitual;
	while (RR)
	{
		if (RR->ipProvince==iNumber)
		{
			for (int i=0; i<MAX_RITUAL_EFFECT; i++)
			{
				if ((pFirstRitual+RR->Index)->arEffect[i][0]==Index)
				{Result+=(pFirstRitual+RR->Index)->arEffect[i][Param];};
			};
		};
		RR=RR->pNext;
	};
	return Result;
};

void dcProvince::RazeBuilding(int Slot, bool Redraw)
{
	bool bGarrison=false; int Choosen=arBuilding[Slot];
	if (Choosen>0)
	{
		for (int i=0; i<MAX_BUILDING_ABILITY; i++)
		{
			if ((pFirstOuterBld+Choosen)->arAbility[i][0]==4)
			{
				bGarrison=true;
			};
		};
		if (bGarrison)
		{
//			for (int i=0; i<MAX_SQUAD; i++)
//			{
//				if (pGarrison->pSquad[i])
//				{
//					delete pGarrison->pSquad[i];
//				};
//			};
			delete pGarrison;
			pGarrison=NULL;
		};
		arBuilding[Slot]=0;
		arPlayer[ipPlayer].iKarma-=(pFirstOuterBld+Choosen)->iKarma;
		DrawPic(Redraw);
	};
};

void dcProvince::DrawPic(bool bRedraw)
{
//	char Txt[200];
	const int X=140, Y=120;
	int Px=x, Py=y;
	bool bEmpty=true;	
	BITMAP *bmpOld=Pic;
	BITMAP *Temp=create_bitmap(X*2,Y*2);
	rectfill(Temp,0,0,X*2,Y*2,COLOR_0);
	int arInsert[MAX_PROVINCE_BLD+1][2],
		arLeft[MAX_PROVINCE_BLD+1],
		arRight[MAX_PROVINCE_BLD+1],
		arDown[MAX_PROVINCE_BLD+1],
		arDown1[MAX_PROVINCE_BLD+1],
		arUp[MAX_PROVINCE_BLD+1],
		arBorder[MAX_PROVINCE_BLD+1][150];
	BITMAP *arPic[MAX_PROVINCE_BLD+1];
	if ((ipTerrain!=WATER_ID)&&(ipTerrain!=MOUNTAIN_ID))
	{
	if (GetLevel()>2)
	{
		arPic[0]=bmpTown[ipRace-1][GetLevel()-3];
		arInsert[0][0]=X-arPic[0]->w/2;
		arInsert[0][1]=Y-arPic[0]->h/2;		
	}
	else
	{arPic[0]=NULL;};
	bool bHome=false;
	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		if ((arPlayer[PL].bInGame)&&(arPlayer[PL].ipProvince==iNumber))
		{bHome=true;};
	};
	if (bHome)
	{
		arPic[1]=(pFirstOuterBld+23)->bmpBld;
		arInsert[1][0]=arBldPlace[0][0]-arPic[1]->w/2-Px+X;
		arInsert[1][1]=arBldPlace[0][1]-arPic[1]->h/2-Py+Y;
		arPic[2]=NULL;
		arPic[3]=NULL;
	}
	else
	{
		for (int BL=0; BL<MAX_PROVINCE_BLD; BL++)
		{
			if (arBuilding[BL]>0)
			{
				arPic[BL+1]=(pFirstOuterBld+arBuilding[BL])->bmpBld;
				arInsert[BL+1][0]=arBldPlace[BL][0]-arPic[BL+1]->w/2-Px+X;
				arInsert[BL+1][1]=arBldPlace[BL][1]-arPic[BL+1]->h/2-Py+Y;
			}
			else
			{arPic[BL+1]=NULL;};
		};
	};
//	FileLog("CalcPoints...");

	for (int A=0; A<MAX_PROVINCE_BLD+1; A++)
	{
		if (arPic[A])
		{
			bEmpty=false;
			arDown[A]=-1; arLeft[A]=-1; arRight[A]=-1;
			arDown1[A]=1000;
			for (int x=0; x<arPic[A]->w; x++)
			{
				arBorder[A][x]=0;
				for (int y=arPic[A]->h-1; y>=0; y--)
				{
					if (getpixel(arPic[A],x,y)!=COLOR_0)
					{
						arBorder[A][x]=y+arInsert[A][1];
						if (arLeft[A]<0)
						{arLeft[A]=x+arInsert[A][0];};
						arRight[A]=x+arInsert[A][0];
						break;
					};
				};
				if (arBorder[A][x]>arDown[A])
				{arDown[A]=arBorder[A][x];};
				if ((arBorder[A][x]<arDown1[A])&&(arBorder[A][x]>0))
				{arDown1[A]=arBorder[A][x];};
			};
			if (arDown[A]<0)
			{arDown[A]=arInsert[A][1]+arPic[A]->h/2;};
			if (arDown1[A]==1000)
			{arDown1[A]=arInsert[A][1]+arPic[A]->h/2;};

		
			if (arLeft[A]<0)
			{arLeft[A]=arInsert[A][0]+arPic[A]->w/2;};

			
			
			if (arRight[A]<0)
			{arRight[A]=arInsert[A][0]+arPic[A]->w/2;};

			arUp[A]=-1;
			for (int y=0; ((y<arPic[A]->h)&&(arUp[A]<0)); y++)
			{
				for (int x=0; x<arPic[A]->w; x++)
				{
					if (getpixel(arPic[A],x,y)!=COLOR_0)
					{
						arUp[A]=y+arInsert[A][1];
						break;
					};
				};
			};
			if (arUp[A]<0)
			{arUp[A]=arInsert[A][1]+arPic[A]->h/2;};
		};
	};

	for (int j=0; j<Temp->h+40; j++)
	{
		bool NeedCheck=false;
		for (int k=0; k<MAX_PROVINCE_BLD+1; k++)
		{
			if (arPic[k])
			{
				if (j==arDown1[k])
				{
					draw_sprite(Temp,arPic[k],arInsert[k][0],arInsert[k][1]);					
					bEmpty=false;
				};
				if (j>arDown1[k])
				{NeedCheck=true;};				
			};
		};
		if (NeedCheck)
		{
		for (int i=-30; i<Temp->w+30; i++)
		{			
			int PR=GetProvinceIndex(i+Px-X,j+Py-Y), PR1;			
			int Poss=0;	bool NeedRoll=true;		
			if (((pFirstProvince+PR)->arBuilding[MAX_PROVINCE_BLD]>0)&&((pFirstProvince+PR)->arBldPlace[MAX_PROVINCE_BLD][0]==i+Px-X)&&((pFirstProvince+PR)->arBldPlace[MAX_PROVINCE_BLD][1]==j+Py-Y))
			{
				int LN=(pFirstProvince+PR)->arBuilding[MAX_PROVINCE_BLD];
				if ((pFirstLandmark+LN)->iLayer==2)
				{
					NeedRoll=false;					
					draw_sprite(Temp,(pFirstLandmark+LN)->Pic,i-(pFirstLandmark+LN)->dx,j-(pFirstLandmark+LN)->dy);
				};
			};
			if ((NeedRoll)&&(PR>0)&&((pFirstProvince+PR)->ipTerrain!=3)&&(arTerrLayer[(pFirstProvince+PR)->ipTerrain][2]>0)&&((Random(20000,i+Px-X,j+Py-Y,CurGame.iSeed,2)<arTerrLayer[(pFirstProvince+PR)->ipTerrain][2])||
				((i+Px-X==(pFirstProvince+PR)->x)&&(j+Py-Y==(pFirstProvince+PR)->y)&&(((pFirstProvince+PR)->ipTerrain==HILL_ID))||
				((i+Px-X==(pFirstProvince+PR)->x)&&(j+Py-Y==(pFirstProvince+PR)->arBldPlace[0][1])&&((pFirstProvince+PR)->ipTerrain==MOUNTAIN_ID)))))
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
						PR1=GetProvinceIndex(i+Px-X-(pFirstMapObject+z)->dx/2,j+Py-Y-(pFirstMapObject+z)->dy/2);
						if ((PR1==0)||((pFirstMapObject+z)->arLand[(pFirstProvince+PR1)->ipTerrain]==0))
						{bDraw=false;};
						PR1=GetProvinceIndex(i+Px-X+(pFirstMapObject+z)->dx/2,j+Py-Y-(pFirstMapObject+z)->dy/2);
						if ((PR1==0)||((pFirstMapObject+z)->arLand[(pFirstProvince+PR1)->ipTerrain]==0))
						{bDraw=false;};
						PR1=GetProvinceIndex(i+Px-X,j+Py-Y-(pFirstMapObject+z)->dy/2);
						if ((PR1==0)||((pFirstMapObject+z)->arLand[(pFirstProvince+PR1)->ipTerrain]==0))
						{bDraw=false;};
						if ((bDraw)||((i+Px-X==(pFirstProvince+PR)->x)&&(j+Py-Y==(pFirstProvince+PR)->y)&&((pFirstProvince+PR)->ipTerrain==HILL_ID))
							||((i+Px-X==(pFirstProvince+PR)->x)&&(j+Py-Y==(pFirstProvince+PR)->arBldPlace[0][1])&&((pFirstProvince+PR)->ipTerrain==MOUNTAIN_ID)))
						{
							RollerN[Counter]=z;
							RollerP[Counter]=(pFirstMapObject+z)->arLand[(pFirstProvince+PR)->ipTerrain];
							Counter++;							
						};
					};					
				};
	
				if (Counter>0)
				{
					int iObj=Roll(false,CurGame.iSeed,j+Py-Y,i+Px-X,2);
					int iPic=0;
					while ((iPic<MAX_MAPOBJ_PIC)&&((pFirstMapObject+iObj)->arPic[iPic])){iPic++;};
					iPic=Random(iPic,j+Py-Y,iSeed,2,i+Px-X);									
					int XP=i-(pFirstMapObject+iObj)->arPic[iPic]->w/2, YP=j-(pFirstMapObject+iObj)->arPic[iPic]->h, Add=(pFirstMapObject+iObj)->arPic[iPic]->w, Shift=3;
					if ((pFirstProvince+PR)->ipTerrain==2)
					{Shift=15;};
					bool bDraw=false, bForbid=false;
					for (int A=0; A<MAX_PROVINCE_BLD+1; A++)
					{
						if ((arPic[A])&&(j>arDown1[A]))
						{
							if ((XP+Add>arLeft[A])&&(XP<arRight[A])&&(YP+(pFirstMapObject+iObj)->arPic[iPic]->h>0))
							{
						
								if ((i>arLeft[A])&&(i<arRight[A]+arPic[A]->w)&&(arBorder[A][i-arInsert[A][0]]>0))
								{
									if ((i-arInsert[A][0]>=0)&&(i-arInsert[A][0]<150))
									{
							//		sprintf(Txt,"Index: %d",i-arInsert[A][0]);
							//		FileLog(Txt);
									if (j>arBorder[A][i-arInsert[A][0]]+Shift)							
									{bDraw=true;};
									if (j<arBorder[A][i-arInsert[A][0]]+Shift)
									{bForbid=true;};
									};
								}
								else
								{
							//		sprintf(Txt,"Index: %d, %d",XP-arInsert[A][0],XP+Add-arInsert[A][0]);
							//		FileLog(Txt);
									if ((XP-arInsert[A][0]>=0)&&(XP-arInsert[A][0]<150))
									{
									if ((XP>arLeft[A])&&(XP<arRight[A])&&(arBorder[A][XP-arInsert[A][0]]>0)&&(j>arBorder[A][XP-arInsert[A][0]]+Shift))									
									{bDraw=true;};
									};
									if ((XP-arInsert[A][0]+Add>=0)&&(XP-arInsert[A][0]+Add<150))
									{
									if ((XP+Add>arLeft[A])&&(XP+Add<arRight[A])&&(arBorder[A][XP+Add-arInsert[A][0]]>0)&&(j>arBorder[A][XP+Add-arInsert[A][0]]+Shift))									
									{bDraw=true;};
									};
								};
							};							
						};
					};
					if ((bDraw)&&(!bForbid))
					{
						draw_sprite(Temp,(pFirstMapObject+iObj)->arPic[iPic],XP,YP);
						bEmpty=false;
					};
				};
			};
		};
	};
	};
	//bEmpty=true;
	//for (int A=0; A<MAX_PROVINCE_BLD+1; A++)
	//{
	//	if (arPic[A])
	//	{bEmpty=false;};
	//};
	}; //WATER/MOUNT
	if (bEmpty)
	{
		Pic=create_bitmap(1,1);
		putpixel(Pic,0,0,COLOR_0);
		PicX=Px;
		PicY=Py;
//		FileLog("Empty");

	}
	else
	{
		int X1=1000, Y1=1000, X2=-1, Y2=-1;
		for (int A=0; A<MAX_PROVINCE_BLD+1; A++)
		{
			if (arPic[A])
			{
				if (arLeft[A]<X1)
				{X1=arLeft[A];};
				if (arRight[A]>X2)
				{X2=arRight[A];};
				if (arUp[A]<Y1)
				{Y1=arUp[A];};
				if (arDown[A]>Y2)
				{Y2=arDown[A];};
			};
		};
	//	sprintf(Txt,"X1: %d, X2: %d, Y1: %d, Y2: %d", X1, X2, Y1, Y2);
	//	FileLog(Txt);
		Pic=create_bitmap(X2-X1,Y2-Y1);
		PicX=Px-X+X1;
		PicY=Py-Y+Y1;
		rectfill(Pic,0,0,X2-X1,Y2-Y1,COLOR_0);
		for (int A=0; A<MAX_PROVINCE_BLD+1; A++)
		{
			if (arPic[A])
			{
				blit(Temp,Pic,arLeft[A],arUp[A],arLeft[A]-X1,arUp[A]-Y1,arRight[A]-arLeft[A],arDown[A]-arUp[A]);
			};
		};
	};
	if ((Obj)&&(bRedraw))
	{
		Obj->Pic=Pic;
		Obj->PlaceOnMap(PicX,PicY);
	};
	destroy_bitmap(Temp);
	if (bmpOld)
	{
		destroy_bitmap(bmpOld);
	};	
};

int dcProvince::GetCorruption()
{
	return ((pFirstRace+ipRace)->iCorrupt+GetSiteParam1(20))*(100+CheckBldAbil(9))/100;
};

void dcProvince::MakeNeutral()
{
	int NewType=(pFirstRace+ipRace)->ipProvType;				
	/*switch (ipRace)
	{
	case 1:
		NewType=1;
		break;
	case 2:
		NewType=9;
		break;
	case 3:
		NewType=10;
		break;
	case 4:
		NewType=4;
		break;
	case 5:
		NewType=6;
		break;
	case 6:
		NewType=8;
		break;
	case 7:
		NewType=18;
		break;
	};*/
	if (GetLevel()==1)
	{NewType=15;};
	if (GetLevel()==2)
	{NewType=5;};
	ipPlayer=0;
	ipType=21;
	iDefender=0;
	if (!pGuard)
	{
		ipType=NewType;
		int Adder=(GetLevel()-3)/2;	
		if (Adder<0)
		{Adder=0;};
		if (Adder>3)
		{Adder=3;};
		int Grd=(pFirstProvinceType+NewType)->arGuard[0][0];
		PlaceGuard((pFirstGuardType+Grd)->arGuard[Adder],(pFirstGuardType+Grd)->iKarma);
	}
	else
	{pGuard->ipPlayer=0;};
	if (pGarrison)
	{
		pGarrison->ipPlayer=0;
		for (int i=0; i<MAX_SQUAD; i++)
		{
			if (pGarrison->pSquad[i])
			{
				pGarrison->pSquad[i]->Dismiss();
				pGarrison->pSquad[i]=NULL;
			};
		};
	};
};

void FillMonsters (dcProvince *PR)
{
	if ((PR->ipTerrain==WATER_ID)||(PR->ipTerrain==MOUNTAIN_ID))
	{return;}
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if ((PR->arSite[i][0]>0)&&(PR->arSiteGuard[i]))
		{
			for (int j=0; j<MAX_SQUAD; j++)
			{
				if ((PR->arSiteGuard[i]->pSquad[j])&&(PR->arSiteGuard[i]->pSquad[j]->iLife>0))
				{RollerP[PR->arSiteGuard[i]->pSquad[j]->GetUnitType()]+=1;}
			}
		}
	}
}


int dcProvince::GetQuestUnit(int Rank)
{
	ClearRoll();
	RollerN[0]=1;
	for (int i=1; i<=iMaxUnitType; i++)
	{RollerN[i]=i;};
	FillMonsters(this);
	for (int i=0; i<MAX_NEIGHBOUR; i++)
	{
		if (arNeighbour[i]>0)
		FillMonsters((pFirstProvince+arNeighbour[i]));
	};
	int Choose=0;
	for (int i=1; i<=iMaxUnitType; i++)
	{
		if ((RollerP[i]>RollerP[Choose])&&((pFirstUnitType+i)->iLevel==Rank))
		{
			Choose=i;
		};
	};
	return Choose;
};