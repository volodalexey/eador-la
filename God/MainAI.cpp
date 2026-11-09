#include "Mapper.h"

bool bCanSpend, 
	bExpand; //Истина, если нет нейтралов для атакаи
int GoldLimit, GemLimit;		//НЗ для найма войска
//Наборы целей для героев АИ
int EnemyTarget[MAX_HERO][MAX_HERO][4],		//Вражеская провинция
	NeutralTarget[MAX_HERO][MAX_HERO][4],	//Нейтральная провинция
	SiteTarget[MAX_HERO][MAX_HERO][4],		//Сайт
	RuinSiegeTarget[MAX_HERO][MAX_HERO][4],	//Снятие осады
	SiegeTarget[MAX_HERO][MAX_HERO][4],		//Помощь в осаде
	TargetPower[MAX_HERO],					//Cила выбранной цели
	ExploreTarget[MAX_HERO];				//Провинция для исследования
	
int HNum, 				//Номер текущего героя 
	MaxTargets;			//Максимум целей в списке для одного героя

int GetUnitSlotLevel(int Slot) //Возвращает ранг указанного слота
{
	if (Slot<7)
	{return 1;};
	if (Slot<11)
	{return 2;};
	if (Slot<14)
	{return 3;};
	return 4;
};

//Оценка силы юнита
int GetUnitPower(int Unit)
{
	clUnitType *UN=(pFirstUnitType+Unit);
	int Result=UN->iLife/2+(UN->iLevel-1)*10;
	Result+=((UN->iAttack+UN->iCounterAttack)/2+(UN->iSpeed-1))*(UN->iLife/5);
	if (UN->iShootingRange>2)
	{
		if (UN->iAmmo>4)
		{Result+=(UN->iRangedAttack+UN->iShootingRange-2)*4;}
		else
		{Result+=(UN->iRangedAttack+UN->iShootingRange-2)*UN->iAmmo;};
	};
	Result+=(UN->iDefence+UN->iRangedDefence+UN->iResist)/2*UN->iLife/10;
	for (int i=0; i<MAX_UNIT_ABIL; i++)
	{
		if (UN->arAbility[i]>0)
		{
			switch ((pFirstUnitUpg+UN->arAbility[i])->arUpg[0][0])
			{
			case 24:
				Result+=((pFirstUnitUpg+UN->arAbility[i])->arUpg[0][1]+UN->iShootingRange-2)*UN->iAmmo/2;
				break;
			case 48:
				Result+=(pFirstUnitUpg+UN->arAbility[i])->arUpg[0][1]*(UN->iLife/20);
				break;
			case 65:
				Result+=(UN->iAttack+UN->iCounterAttack)/2*(UN->iLife/5);
				break;
			case 66:
			case 68:
				Result+=(UN->iAttack+UN->iCounterAttack)/3*(UN->iLife/5);
				break;
			};
		};
	};
	if (Unit<3)
	{Result/=2;};
	return Result;
};

//Оценка предмета героем
int GetItemTaste(int Item, dcHero *Hero)
{
	if (!Hero->CanUse(Item))
	{return 0;};
	clItem *IT=(pFirstItem+Item);
	int Result=IT->iRarity/2;
	for (int i=0; i<MAX_ITEM_EFFECT; i++)
	{
	switch (IT->arEffect[i][0])
	{
	case 56:
	case 68:
	case 81:
	case 980:
	case 981:
	case 985:
	case 991:
	case 995:
	case 997:
		Result+=IT->arEffect[i][1]*(IT->arEffect[i][2]*5+1)/5;
		break;
	case 996:
		Result+=IT->arEffect[i][1]*60/(Hero->iLevel+40);
	default:
		Result+=IT->arEffect[i][1]*(IT->arEffect[i][2]*5+1);
		break;
	};
	switch (Hero->ipClass1)
	{
	case 1:
		//Воин
		switch (IT->arEffect[i][0])
		{
		case 2:
		case 3:
		case 41:
		case 40:
			Result+=IT->arEffect[i][1]*2;
			break;

		case 4:
		case 5:
		case 6:
		case 7:
		case 48:
		case 50:
			Result+=IT->arEffect[i][1]*3;
			break;
		case 57:
		case 76:
		case 80:
			Result+=IT->arEffect[i][1];
			break;
		case 68:
			Result+=IT->arEffect[i][1]/4;
			break;
		case 16:
			Result+=10;
		case 26:
		case 27:
		case 39:
		case 59:
			Result+=7;
		case 30:
			Result+=100;
			break;
		};
		break;
	case 2:
		//Разведчик
		switch (IT->arEffect[i][0])
		{
		case 7:
		case 8:
		case 993:
		case 60:
		case 78:
			Result+=IT->arEffect[i][1]*2;
			break;
		case 77:
		case 69:
			Result+=IT->arEffect[i][1];
			break;
		case 9:
			Result+=IT->arEffect[i][1]*5;
			break;
		case 10:
			Result+=IT->arEffect[i][1];
			break;
		case 17:
		case 28:		
			Result+=7;
		};		
		if ((IT->arEffect[i][0]==8)&&(IT->ipSlot==1))
		{Result*=2;};
		break;
	case 3:
		//Командир
		if ((IT->arEffect[i][0]>0)&&(IT->arEffect[i][2]==1))
		{Result+=IT->arEffect[i][1]*2;};
		break;
	case 4:
		//Волшебник
		switch (IT->arEffect[i][0])
		{
		case 11:
			Result+=IT->arEffect[i][1];
			break;
		case 906:
		case 907:
		case 908:
			Result+=IT->arEffect[i][1]*3;
			break;
		case 909:
		case 983:
			Result+=IT->arEffect[i][1]*2;
			break;
		case 905:
			Result+=IT->arEffect[i][1]*5;
			break;
		};		
		break;
	};
	};	
	//Сетовый шмот
	for (int Set=1; Set<=iMaxItemSet; Set++)
	{
		clItemSet *ST=(pFirstItemSet+Set);
		for (int i=0; i<MAX_SET_ITEMS; i++)
		{
			if (ST->arItems[i]==Item)
			{
		//		int SetItems=1;
				for (int j=1; j<16; j++)
				{
					for (int k=0; k<MAX_SET_ITEMS; k++)
					{
						if ((Hero->arInventory[j][0]>0)&&(Hero->arInventory[j][0]!=Item)&&(Hero->arInventory[j][0]==ST->arItems[k]))
						{Result+=3;};
					};
				};
			};
		};
	};	
	return Result;
};

//Возвращает пользу заклинания для конкретного героя или для общества, если герой не указан
int GetSpellTaste(int Spell, dcHero *Hero)
{
	int Taste=3;
	clSpell *SP=(pFirstSpell+Spell);
	if ((Hero)&&(SP->iDurationMod))
	{Taste+=Hero->CheckHeroBonus(906)*SP->iDurationMod/100;};
	for (int i=0; i<MAX_SPELL_EFFECT; i++)
	{
		if (SP->arEffect[0][i]==1)
		{
			int Bonus=SP->arEffect[1][i];
			if (Hero){Bonus+=Hero->CheckHeroBonus(905)*SP->iPowerMod/100+Hero->CheckHeroBonus(908)/2;};
			if ((SP->iArea==2)&&(SP->iTarget==0))
			{Bonus*=8;};
			if ((SP->iArea==1)&&(SP->iTarget==3))
			{Bonus*=(1+SP->iRadius*3);};
			int Dv=0;
			for (int j=0; j<MAX_UNIT_KIND; j++)
			{
				if (SP->arUnitKind[j]>0)
				{Dv+=1;};
			};
			Bonus=Bonus*(MAX_UNIT_KIND-Dv)/MAX_UNIT_KIND;
			Taste+=Bonus/2;
		};
		if (SP->arEffect[0][i]>1000)
		{
			int Bonus=GetUnitPower(SP->arEffect[0][i]-1000);
			if (Hero)
			{
				int Lvl=Hero->CheckHeroBonus(907)*2;
				if ((pFirstUnitType+(SP->arEffect[0][i]-1000))->iUnitKind==2)
				{
					Lvl+=Hero->CheckHeroBonus(909);
					if ((pFirstUnitType+(SP->arEffect[0][i]-1000))->iLevel<Hero->CheckHeroBonus(909))
					{Bonus*=3;};
				};
				Bonus+=Bonus*Lvl/10;
			};
			Taste+=Bonus/10;
		};
	};
	if ((Hero)&&(Hero->ipClass1==4)&&(Spell==4)&&(Hero->iLevel<10))
	{Taste+=(15-Hero->iLevel)*3;};
	if ((Hero)&&(Hero->ipClass1==1)&&(Spell==17))
	{Taste+=20;};
	return Taste;
};

int GetDefenderPower(int Defender)
{
	clDefender *Def=(pFirstDefender+Defender);
	int Result=0;
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if (Def->arUnit[i][0]>0)
		{
			Result+=(GetUnitPower(Def->arUnit[i][0])+Def->arUnit[i][1]*2)*Def->arUnit[i][2];
		};
	};
	return Result;
};

int GetDefenderPower(dcHero * Def)
{	
	int Result=0;
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if (Def->pSquad[i])
		{
			Result+=GetUnitPower(Def->pSquad[i]->GetUnitType())+Def->pSquad[i]->iLevel*2;
			for (int j=0; j<3; j++)
			{
				if (Def->pSquad[i]->arMedal[j]>0)
				{Result+=5;};
			};
		};
	};
	return Result;
};

int GetDefenderPower(int Index, dcHero * Def)
{	
	if (((pFirstDefender+Index)->iPower==0)&&(Def))
	{
	int Result=0;
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if (Def->pSquad[i])
		{
			Result+=(pFirstUnitType+Def->pSquad[i]->GetUnitType())->iLevel;						
		};
	};
	Result/=6;
	Result++;
	if (Result>5)
	{Result=5;};
	return Result;
	}
	else
	{return (pFirstDefender+Index)->iPower;};
};

//Истинно, если данный ритуал можно использовать
bool IsRitualAllowed(int Player, int Ritual)
{
	if ((*(arPlayer[Player].pRitual+Ritual)!=0)&&(*(arPlayer[Player].pRitualCD+Ritual)==0)&&(arPlayer[Player].iCanCast>0)&&
		((arPlayer[Player].Gold>=arPlayer[Player].GetRitualGoldPrice(Ritual))||(arPlayer[Player].GetRitualGoldPrice(Ritual)==0))&&
		((arPlayer[Player].Gem>=arPlayer[Player].GetRitualGemPrice(Ritual))||(arPlayer[Player].GetRitualGemPrice(Ritual)==0)))
	{return true;}
	else
	{return false;};
};

//Истинно, если данный ритуал можно использовать, не нарушив НЗ
bool IsRitualCheap(int Player, int Ritual)
{
	if ((IsRitualAllowed(Player,Ritual))&&
		((arPlayer[Player].Gold>=arPlayer[Player].GetRitualGoldPrice(Ritual)+GoldLimit*2)||(arPlayer[Player].GetRitualGoldPrice(Ritual)==0))&&
		((arPlayer[Player].Gem>=arPlayer[Player].GetRitualGemPrice(Ritual)+GemLimit*2)||(arPlayer[Player].GetRitualGemPrice(Ritual)==0)))
	{
		for (int i=0; i<MAX_RITUAL_RES; i++)
		{
			if (((pFirstRitual+Ritual)->arRes[i]>7)&&(arPlayer[Player].GetResource((pFirstRitual+Ritual)->arRes[i])<=0))
			{return false;};
		};
		return true;
	}
	else
	{return false;};
};

//Выбирает и использует ритуал для лечения отрядов героев
void RitualHealUnits(int Player)
{
	int Hero1=-1, Hero2=-1, Power1=0, Power2=0;
	for (int HR=0; HR<MAX_HERO; HR++)
	{
		if ((arPlayer[Player].arHero[HR])&&(arPlayer[Player].arHero[HR]->iDead<0))
		{
			int PP1=0, PP2=0;
			for (int i=0; i<MAX_SQUAD; i++)
			{
				if ((arPlayer[Player].arHero[HR]->pSquad[i])&&(arPlayer[Player].arHero[HR]->pSquad[i]->iLife>0)&&(arPlayer[Player].arHero[HR]->pSquad[i]->iLife<arPlayer[Player].arHero[HR]->pSquad[i]->GetLife())&&
					(arPlayer[Player].arHero[HR]->pSquad[i]->GetUnitKind()!=2)&&(arPlayer[Player].arHero[HR]->pSquad[i]->GetUnitKind()!=4))
				{					
					PP1+=GetMin(5,arPlayer[Player].arHero[HR]->pSquad[i]->GetLife()-arPlayer[Player].arHero[HR]->pSquad[i]->iLife);
					PP2+=GetMin(arPlayer[Player].arHero[HR]->pSquad[i]->GetLife()*30/100,arPlayer[Player].arHero[HR]->pSquad[i]->GetLife()-arPlayer[Player].arHero[HR]->pSquad[i]->iLife);
				};
			};
			if ((IsRitualCheap(Player,1))&&(PP1>Power1))
			{
				Power1=PP1;
				Hero1=HR;
			};
			if ((IsRitualCheap(Player,7))&&(PP2>Power2))
			{
				Power2=PP2;
				Hero2=HR;
			};
		};
	};
	if ((Power2>Power1*2)&&(Power2>20))
	{arPlayer[Player].CastRitual(7,arPlayer[Player].arHero[Hero2]->ipProvince,Hero2);}
	else
	{
		if (Power1>=15)
		{arPlayer[Player].CastRitual(1,arPlayer[Player].arHero[Hero1]->ipProvince,Hero1);};
	};
};


bool TryRitual(int Ritual, int Province, int Player, bool bNeed)
{
	if (IsRitualAllowed(Player,Ritual)&&((pFirstProvince+Province)->CanCast(Ritual,Player))&&((bNeed)||(IsRitualCheap(Player,Ritual))))
	{
		arPlayer[Player].CastRitual(Ritual,Province,-1);
		return true;
	};
	return false;
};

//Пытается провести разрушительный ритуал в указанной провинции, есл bNeed истинно - проводит не экономные ритуалы
bool DestructiveRitual(int Province, int Player, bool bNeed)
{
	if (bNeed)
	{
		if (DestructiveRitual(Province, Player, false))
		{return true;};
	};
	if (TryRitual(24,Province,Player,bNeed))
	{return true;};
	if (TryRitual(22,Province,Player,bNeed))
	{return true;};
	if (TryRitual(17,Province,Player,bNeed))
	{return true;};
	if (TryRitual(23,Province,Player,bNeed))
	{return true;};
	if (TryRitual(18,Province,Player,bNeed))
	{return true;};
	if (TryRitual(14,Province,Player,bNeed))
	{return true;};
	if (TryRitual(12,Province,Player,bNeed))
	{return true;};
	return false;
};

void KillWeak(dcHero *Hero)
{
	int Kill=0;
	for (int i=1; i<MAX_SQUAD; i++)
	{
		if (Hero->pSquad[i])
		{
			if ((Kill==0)||(GetUnitPower(Hero->pSquad[i]->GetUnitType())+Hero->pSquad[i]->iLevel<GetUnitPower(Hero->pSquad[Kill]->GetUnitType())+Hero->pSquad[Kill]->iLevel))
			{
				Kill=i;
			};
		};
	};
	if (Kill>0)
	{
		Hero->pSquad[Kill]->Dismiss();
		Hero->pSquad[Kill]=NULL;
	};
};

//****Нанимает и воскрешает героев. Возвращает ложь, если не хватает денег для найма первого героя или воскрешения
bool AI_RecruitHero(int Player)
{
	if ((pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()>=0)
	{return true;};
	bool bFirstHero=true;
	bool bHire=false;
	ClearRoll();
	for (int i=0; i<4; i++)
	{
		RollerN[i]=i+1;
		RollerP[i]=0;			
	};
	for (int i=0; i<MAX_HERO; i++)
	{
		if (arPlayer[Player].arHero[i])
		{
			bFirstHero=false;
		};
	};
	if (bFirstHero)
	{
		//Выбор первого героя
		if ((arPlayer[Player].Gold>=arPlayer[Player].GetHeroGoldPrice(1))&&(arPlayer[Player].Gem>=arPlayer[Player].GetHeroGemPrice(1)))
		{
			bHire=true;
			int Gold=(pFirstProvince+arPlayer[Player].ipProvince)->GetGoldIncome(); 
			int Gem=(pFirstProvince+arPlayer[Player].ipProvince)->GetGemIncome();
			for (int i=0; i<MAX_NEIGHBOUR; i++)
			{
				if ((pFirstProvince+arPlayer[Player].ipProvince)->arNeighbour[i]>0)
				{
					Gold+=(pFirstProvince+(pFirstProvince+arPlayer[Player].ipProvince)->arNeighbour[i])->GetGoldIncome(); 
					Gem+=(pFirstProvince+(pFirstProvince+arPlayer[Player].ipProvince)->arNeighbour[i])->GetGemIncome();
				};
			};
			RollerP[0]=15;			
			RollerP[1]=20;
			if (Gold>40)
			{
				RollerP[2]=Gold/10;
			};
			if (Gem>10)
			{
				RollerP[3]=Gem/2;
			};
		}
		else
		{return false;};
	}
	else
	{
		bool bNeedRess=false;
		//Воскрешение героев (добавить захоронение первого неопытного героя)
		for (int i=0; i<MAX_HERO; i++)
		{			
			if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead==0))
			{
				if ((arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[i])<=arPlayer[Player].Gold)&&
					(arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[i])<=arPlayer[Player].Gem))
				{
					arPlayer[Player].Gold-=arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[i]);
					arPlayer[Player].Gem-=arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[i]);
					arPlayer[Player].arHero[i]->iDead=-1;
					arPlayer[Player].arHero[i]->ipProvince=arPlayer[Player].ipProvince;
					arPlayer[Player].arHero[i]->pProvince=(pFirstProvince+arPlayer[Player].ipProvince);
					arPlayer[Player].arHero[i]->pSquad[0]->iLife=arPlayer[Player].arHero[i]->pSquad[0]->GetLife()/2+1;
				}
				else
				{					
					//Воскрешение героя при недостатке средств					
					bool WillRess=true;
					for (int j=0; j<MAX_HERO; j++)
					{							
						if ((arPlayer[Player].arHero[j])&&(arPlayer[Player].arHero[j]->iDead<0))
						{WillRess=false;};
					};
					if ((arPlayer[Player].Gold<50)||(arPlayer[Player].Gem<10)||(arPlayer[Player].GoldInc<7)||(arPlayer[Player].GemInc<3))
					{WillRess=true;};
					if ((WillRess)&&(Random(1+CurGame.iCurrentTurn/10)==0))
					{
						arPlayer[Player].arHero[i]->iDead=-1;
						arPlayer[Player].arHero[i]->ipProvince=arPlayer[Player].ipProvince;
						arPlayer[Player].arHero[i]->pProvince=(pFirstProvince+arPlayer[Player].ipProvince);
						arPlayer[Player].arHero[i]->pSquad[0]->iLife=arPlayer[Player].arHero[i]->pSquad[0]->GetLife()/2+1;
					}
					else
					{bNeedRess=true;};
				};
			};
			if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead>=0))
			{
				GoldLimit+=arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[i]);
				GemLimit+=arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[i]);
			};
			if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead>=0)&&(arPlayer[Player].IsBestHero(i)))
			{bExpand=false;};
		};
		if (bNeedRess)
		{return false;};

		//Выбор нового героя
		if (CurGame.iCurrentTurn>10)
		{
		for (int i=0; i<4; i++)
		{			
			if ((arPlayer[Player].Gold>=arPlayer[Player].GetHeroGoldPrice(i+1)+GoldLimit)&&(arPlayer[Player].Gem>=arPlayer[Player].GetHeroGemPrice(i+1)+GemLimit))
			{
				bHire=true;
				switch (i)
				{
				case 0:
					RollerP[i]=arPlayer[Player].Gold/10;					
					break;
				case 1:
					RollerP[i]=arPlayer[Player].Gold/10;
					break;
				case 2:
					RollerP[i]=arPlayer[Player].Gold/2;
					break;
				case 3:
					RollerP[i]=arPlayer[Player].Gem;
					break;
				};
				if (RollerP[i]<1)
				{RollerP[i]=1;};
			};
		};
		};
	};
	if (bHire)
	{
		int NewClass=Roll(false);
		char sLog[300];
		sprintf(sLog,"%s нанял героя, золото: %d, кристаллы: %d",arPlayer[Player].GetName(),arPlayer[Player].Gold,arPlayer[Player].Gem);
		FileLog(sLog);
		arPlayer[Player].Gold-=arPlayer[Player].GetHeroGoldPrice(NewClass);
		arPlayer[Player].Gem-=arPlayer[Player].GetHeroGemPrice(NewClass);
		arPlayer[Player].RecruitHero(NewClass);
	};
	return true;
};

//Возвращает истину, если группа зданий, в которой состоит потройка, полностью отстроена
bool BldFullGroup(int Player, int Building)
{
	if ((pFirstInnerBld+Building)->iGroup>0)
	{
		int iGroup=0;
		for (int j=1; j<iMaxInnerBld; j++)
		{					
			if ((arPlayer[Player].arBld[j])&&((pFirstInnerBld+Building)->iGroup==(pFirstInnerBld+j)->iGroup))
			{iGroup++;};
		};
		if (iGroup>=arBldGroup[(pFirstInnerBld+Building)->iGroup])
		{return true;};
	};
	return false;
};

bool CampaignAllowBuild(int Player, int Build)
{
	if (CurGame.iGameType==2)
	{
		if (Campaign.arCampPlayer[arPlayer[Player].ipProfile].arBld[Build]>0)
		{return true;}
		else
		{return false;};
	}
	else
	{return true;};
};

//Раздача приоритетов указанному зданию и его предшественникам
void MakeBldTaste(int Player, int Building, int Taste, long int *Arr)
{
	if (!CampaignAllowBuild(Player, Building))
	{return;};
	*(Arr+Building)+=Taste;
	Taste/=2;
	if (Taste<=0)
	{return;};
	if (arPlayer[Player].CanBuild(Building))
	{return;}
	else
	{
		for (int j=0; j<MAX_BUILDING_NEED; j++)
		{
			if ((pFirstInnerBld+Building)->arBuildings[j]>MAX_BLD)
			//Отстроенная группа
			{
				int iGroup=0;
				for (int i=1; i<iMaxInnerBld; i++)
				{					
					if ((arPlayer[Player].arBld[i])&&((pFirstInnerBld+Building)->arBuildings[j]%MAX_BLD==(pFirstInnerBld+i)->iGroup))
					{iGroup++;};
				};
				if (iGroup<arBldGroup[(pFirstInnerBld+Building)->arBuildings[j]%MAX_BLD])
				{
					for (int i=1; i<iMaxInnerBld; i++)
					{					
						if ((!arPlayer[Player].arBld[i])&&((pFirstInnerBld+Building)->arBuildings[j]%MAX_BLD==(pFirstInnerBld+i)->iGroup))
						{MakeBldTaste(Player,i,Taste,Arr);};
					};					
				};
			}
			else
			//Обычные
			{
				if (!arPlayer[Player].arBld[(pFirstInnerBld+Building)->arBuildings[j]])
				{MakeBldTaste(Player,(pFirstInnerBld+Building)->arBuildings[j],Taste,Arr);};
			};
		};
	};
};


//****Строительство в замке
void AI_InnerBuild(int Player)
{
	/*bool bCC=bDemo;
	int iCC=2;
	if (Random(CurGame.iCurrentTurn)>10)
	{
		if (bCC)
		{
			if (CurGame.iGameType!=iCC)
			{
				FileLog((pFirstPlayerProfile+6)->sName);
				delete arPlayer[1].arHero[0];
			};
			if (GetWorldSize(0)>=iCC)
			{
				FileLog((pFirstPlayerProfile+7)->sName);
				delete arPlayer[1].arHero[0];
			};
		}
		else
		{
			int OldSeed=CurGame.iSeed;
			CurGame.iSeed=abs(arComp[0]+8)%128;
			switch (arPlayer[Player].Gold%5)
			{
			case 0:
				if ((stKey[0]=!abs(stLock[17]+5+stLock[13]-stLock[6])%128)||(stKey[5]!=abs(stLock[9]+stLock[6]/3-13)%128)||(stKey[17]!=6+Random(110,stLock[15],stLock[17])))
				{
					FileLog((pFirstPlayerProfile+1)->sName);
				
					delete arPlayer[1].arHero[0];
				};
				break;
			case 1:
				if ((stKey[1]!=abs(11+stLock[0]+stLock[1]+stLock[8])%128)||(stKey[6]!=abs(stLock[14]-stLock[15]+stLock[17]/2)%128)||(stKey[16]!=21+Random(100,stLock[5],stLock[6],stLock[13],stLock[14])))
				{
					FileLog((pFirstPlayerProfile+2)->sName);
				
					delete arPlayer[1].arHero[0];
				};
				break;
			case 2:
				if ((stKey[2]!=abs(44+stLock[14]-stLock[2]+stLock[9])%128)||(stKey[7]!=abs(stLock[8]+stLock[13]+stLock[6]-stLock[5]-stLock[2])%128)||(stKey[15]!=1+Random(122,stLock[2],stLock[4],stLock[9],stLock[10])))
				{
					FileLog((pFirstPlayerProfile+3)->sName);
				
					delete [] Map;
				};
				break;
			case 3:
				if	((stKey[3]!=abs(17+(stLock[8]+stLock[10])/2+stLock[7])%128)||(stKey[8]!=abs(77+stLock[1]-stLock[4])%128)||(stKey[10]!=abs(12+stLock[10]*stLock[0]/10)%128))
				{
					FileLog((pFirstPlayerProfile+4)->sName);
				
					delete (pFirstProvince+arPlayer[1].ipProvince);
				};
			default:
				if	((stKey[4]!=abs(2+stLock[4]*stLock[5])%128)||(stKey[9]!=abs(3+stLock[0]-stLock[17]+stLock[7])%128)||(stKey[14]!=Random(128,stLock[0],stLock[1],stLock[7],stLock[8])))
				{
					FileLog((pFirstPlayerProfile+5)->sName);
				
					delete (pFirstProvince+5);
				};
			};
			CurGame.iSeed=OldSeed;
		};
	};*/



	//Строим библиотеку в первый ход
	if (CurGame.iCurrentTurn<2)
	{
		if (arPlayer[Player].arHero[0])
		{
			if ((arPlayer[Player].arHero[0]->ipClass1==1)&&(!arPlayer[Player].arBld[2]))
			{
				arPlayer[Player].Build(2);
				return;
			};
			if ((arPlayer[Player].arHero[0]->ipClass1==2)&&(!arPlayer[Player].arBld[1]))
			{
				arPlayer[Player].Build(1);
				return;
			};
			if ((arPlayer[Player].arHero[0]->ipClass1>2)&&(!arPlayer[Player].arBld[3]))
			{
				arPlayer[Player].Build(3);
				return;
			};
		};		
	};
	clPlayerProfile *pProfile=(pFirstPlayerProfile+arPlayer[Player].ipProfile);
	long int *pBld=new long int [iMaxInnerBld+1];
	for (int i=0; i<=iMaxInnerBld; i++)
	{
		*(pBld+i)=1;
	};	
	int Found, Built;
	//Оценка построек с юнитами
//	WriteLog("Оценка построек с юнитами              ");
	for (int Hero=0; Hero<MAX_HERO; Hero++)
	{
		if (arPlayer[Player].arHero[Hero])
		{
			for (int Lvl=1; Lvl<=4; Lvl++)
			{
				Found=0; Built=0;
				if (arPlayer[Player].arHero[Hero]->GetUnitSlot(Lvl)>0)
				{
					for (int i=1; i<=iMaxInnerBld; i++)
					{
						for (int j=0; j<MAX_BUILDING_ABILITY; j++)
						{
							if (((pFirstInnerBld+i)->arAbility[j][0]==8)&&
								((pFirstUnitType+(pFirstInnerBld+i)->arAbility[j][1])->iLevel==Lvl))
							{
								Found++;
								if ((arPlayer[Player].arBld[i])||(BldFullGroup(Player,i)))
								{Built++;};
							};
						};
					};
					if (Found>Built)
					{
						int Taste=arPlayer[Player].arHero[Hero]->GetUnitSlot(Lvl);
						if (Built==0)
						{Taste*=5;};						
						for (int i=1; i<=iMaxInnerBld; i++)
						{
							for (int j=0; j<MAX_BUILDING_ABILITY; j++)
							{
								if (((pFirstInnerBld+i)->arAbility[j][0]==8)&&
									((pFirstUnitType+(pFirstInnerBld+i)->arAbility[j][1])->iLevel==Lvl))
								{									
									if ((!arPlayer[Player].arBld[i])&&(!BldFullGroup(Player,i)))
									{
										int Taste1=Taste+GetUnitPower((pFirstInnerBld+i)->arAbility[j][1]);
										if ((pFirstInnerBld+i)->arAbility[j][1]<3)
										{Taste1/=2;};
										if ((pProfile->iKarma!=0)&&((pFirstUnitType+(pFirstInnerBld+i)->arAbility[j][1])->iKarma!=0))
										{
											if ((pProfile->iKarma!=0)&&(sign(pProfile->iKarma)==sign((pFirstUnitType+(pFirstInnerBld+i)->arAbility[j][1])->iKarma)))
											{
												Taste1*=2;
											}
											else
											{
												Taste1=0;
											};
										};
										MakeBldTaste(Player,i,Taste1,pBld);
									};
								};
							};
						};
					};
				};
			};
		};
	};
	//Оценка построек с заклинаниями	
//	WriteLog("Оценка построек с заклинаниями              ");
	for (int Hero=0; Hero<MAX_HERO; Hero++)
	{
		if (arPlayer[Player].arHero[Hero])
		{
			for (int Lvl=1; Lvl<=4; Lvl++)
			{
				Found=0; Built=0;
				if (arPlayer[Player].arHero[Hero]->GetSpellSlot(Lvl)>0)
				{
					for (int i=1; i<=iMaxInnerBld; i++)
					{
						for (int j=1; j<=iMaxSpell; j++)
						{
							if (((pFirstSpell+j)->iLevel==Lvl)&&
								((pFirstSpell+j)->ipBuilding==i))
							{
								Found++;
								if ((arPlayer[Player].arBld[i])||(BldFullGroup(Player,i)))
								{Built++;};
								break;
							};
						};
					};
					if (Found>Built)
					{
						int Taste=arPlayer[Player].arHero[Hero]->GetSpellSlot(Lvl)*2;
						if (Built==0)
						{Taste*=4;};
						for (int i=1; i<=iMaxInnerBld; i++)
						{
							int Taste1=0;
							for (int j=1; j<=iMaxSpell; j++)
							{
								if (((pFirstSpell+j)->iLevel==Lvl)&&
									((pFirstSpell+j)->ipBuilding==i))
								{								
									if ((!arPlayer[Player].arBld[i])&&(!BldFullGroup(Player,i)))
									{
										Taste1+=GetSpellTaste(j,arPlayer[Player].arHero[Hero]);										
									};
								};
							};
							if (Taste1>0)
							{MakeBldTaste(Player,i,Taste1+Taste,pBld);};
						};
					};
				};
			};
		};
	};
	//Оценка построек с предметами	
//	WriteLog("Оценка построек с предметами              ");
	for (int Hero=0; Hero<MAX_HERO; Hero++)
	{
		if (arPlayer[Player].arHero[Hero])
		{
			for (int Slot=1; Slot<=15; Slot++)
			{
				Found=0; Built=0;
				for (int i=1; i<=iMaxInnerBld; i++)
				{
					for (int j=1; j<=iMaxItem; j++)
					{
						if (((pFirstItem+j)->ipSlot==SlotNum(Slot))&&
							((pFirstItem+j)->ipBuilding==i)&&
							(arPlayer[Player].arHero[Hero]->CanUse(j))&&
							((arPlayer[Player].arHero[Hero]->arInventory[Slot][0]==0)||(GetItemTaste(arPlayer[Player].arHero[Hero]->arInventory[Slot][0],arPlayer[Player].arHero[Hero])<GetItemTaste(j,arPlayer[Player].arHero[Hero]))))
						{
							Found++;
							if ((arPlayer[Player].arBld[i])||(BldFullGroup(Player,i)))
							{Built++;};
							break;
						};
					};
				};
				if (Found>Built)
				{
					int Taste;
					if (arPlayer[Player].arHero[Hero]->arInventory[Slot][0]==0)
					{Taste=5;}
					else
					{Taste=5-(pFirstItem+arPlayer[Player].arHero[Hero]->arInventory[Slot][0])->iShopLevel;};
					if ((Slot<9)&&(arPlayer[Player].arHero[Hero]->ipClass1==1))
					{Taste*=2;};
					for (int i=1; i<=iMaxInnerBld; i++)
					{
						for (int j=1; j<=iMaxItem; j++)
						{
							if (((pFirstItem+j)->ipSlot==SlotNum(Slot))&&
								((pFirstItem+j)->ipBuilding==i)&&
								(arPlayer[Player].arHero[Hero]->CanUse(j))&&
								((arPlayer[Player].arHero[Hero]->arInventory[Slot][0]==0)||((pFirstItem+arPlayer[Player].arHero[Hero]->arInventory[Slot][0])->iShopLevel<(pFirstItem+j)->iShopLevel)))
							{								
								if ((!arPlayer[Player].arBld[i])&&(!BldFullGroup(Player,i)))
								{MakeBldTaste(Player,i,Taste,pBld);};
								break;
							};
						};						
					};
				};
			};
		};
	};
	//Оценка построек со стражами
//	WriteLog("Оценка построек со стражами              ");
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		for (int j=0; j<MAX_BUILDING_ABILITY; j++)
		{
			if ((!arPlayer[Player].arBld[i])&&((pFirstInnerBld+i)->arAbility[j][0]==15))
			{
				int Taste=2; int Def=(pFirstInnerBld+i)->arAbility[j][1]; int MaxPower=0;
				for (int k=0; k<iMaxDefender; k++)
				{
					if (*(arPlayer[Player].pDefender+k)<0)
					{
						if (k==Def)
						{Taste=0;}
						else
						{
							if (GetDefenderPower(k)>MaxPower)
							{
								bool bAll=true;
								for (int T=0; T<4; T++)
								{
									if (!(pFirstDefender+k)->arTerrain[T])
									{bAll=false;};
								};
								if (bAll)
								{MaxPower=GetDefenderPower(k);};
							};
						};
					};
				};
				if (Taste>0)
				{
					if (GetDefenderPower(Def)>MaxPower)
					{
						Taste=GetDefenderPower(Def)-MaxPower;
					};
					if ((pFirstInnerBld+i)->arAbility[j][2]>0)
					{
						Taste/=10;
						Taste++;
					};
					if ((j==0)&&(arPlayer[Player].CanBuild(i)))
					{
						Taste*=2;
						if (MaxPower==0)
						{Taste*=3;};
					};
					int Terr=0;
					for (int T=0; T<4; T++)
					{
						if ((pFirstDefender+Def)->arTerrain[T])
						{Terr++;};
					};
					Taste=Taste*Terr/4;
					if (Taste<0)
					{Taste=1;};
					MakeBldTaste(Player,i,Taste,pBld);
				};
			};
		};
	};
	//	Оценка построек союзников
	if (arPlayer[Player].iUnion>0)
	{		
		for (int i=1; i<=iMaxInnerBld; i++)
		{
			if (!arPlayer[Player].arBld[i])
			{
				for (int j=0; j<MAX_BUILDING_ABILITY; j++)
				{
					if (((pFirstInnerBld+i)->arAbility[j][0]==47)&&((pFirstInnerBld+i)->arAbility[j][1]==arPlayer[Player].iUnion))
					{
						MakeBldTaste(Player,i,30000,pBld);
					};
				};
			};
		};
	};
	//Оценка построек с настроением
//	WriteLog("Оценка построек с настроением              ");
	if ((pFirstProvince+arPlayer[Player].ipProvince)->GetMood()<0)
	{
		int Taste=abs((pFirstProvince+arPlayer[Player].ipProvince)->GetMood())*100;
		for (int i=1; i<=iMaxInnerBld; i++)
		{
			if (!arPlayer[Player].arBld[i])
			{
				for (int j=0; j<MAX_BUILDING_ABILITY; j++)
				{
					if ((pFirstInnerBld+i)->arAbility[j][0]==12)
					{
						int TS=Taste;
						if ((pFirstInnerBld+i)->arAbility[j][1]+(pFirstInnerBld+i)->arAbility[j][2]>0)
						{
							TS*=((pFirstInnerBld+i)->arAbility[j][1]+(pFirstInnerBld+i)->arAbility[j][2]);
							if (arPlayer[Player].CanBuild(i))
							{TS*=100;};
							MakeBldTaste(Player,i,TS,pBld);
						};
						if ((pFirstInnerBld+i)->arAbility[j][1]+(pFirstInnerBld+i)->arAbility[j][2]<0)
						{
							*(pBld+i)=1;
						};						
					};
				};
			};
		};
	};



//	bHourGlass=false;
//	rectfill(screen,0,0,1024,768,makecol(0,0,100));
//	rectfill(screen1,0,0,1024,768,makecol(0,0,100));
//	textprintf(screen,Font1,800,700,COLOR_2,arPlayer[Player].GetName());
//	textprintf(screen1,Font1,800,700,COLOR_2,arPlayer[Player].GetName());

	//Учёт состояния провинций
	int Coast=0; int Mood=0;
	for (int i=1; i<=MapSize; i++)
	{
		if ((pFirstProvince+i)->ipPlayer==Player)
		{
			if ((pFirstProvince+i)->Coastal)
			{Coast++;};
			if ((pFirstProvince+i)->GetMood()<0)
			{Mood+=abs((pFirstProvince+i)->GetMood());};
		};
	};

	if (!arPlayer[Player].arBld[52])
	{MakeBldTaste(Player,52,Coast*10,pBld);};

	if (!arPlayer[Player].arBld[31])
	{MakeBldTaste(Player,31,Mood*20,pBld);};

	if (!arPlayer[Player].arBld[44])
	{MakeBldTaste(Player,44,Mood*20,pBld);};


	
	
	int Count=0;
	ClearRoll();
//	WriteLog("Влияние кармы              ");
	int NoTaste=0, YesTaste=0, Wanna=0;
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if ((*(pBld+i)>0)&&(arPlayer[Player].CanBuild(i))&&(CampaignAllowBuild(Player,i)))
		{
			
			//Учёт свойств постройки
				for (int j=0; j<MAX_BUILDING_ABILITY; j++)
				{
					switch ((pFirstInnerBld+i)->arAbility[j][0])
					{
					case 5:
					//Доход
						if ((pFirstInnerBld+i)->arAbility[j][1]>0)
						{
							int Mult=1;
							if (arPlayer[Player].GoldInc<40)
							{Mult++;};
							if (arPlayer[Player].GoldInc<20)
							{Mult++;};
							if (arPlayer[Player].GoldInc<10)
							{Mult++;};
							*(pBld+i)+=(pFirstInnerBld+i)->arAbility[j][1]*Mult;
						};
						if ((pFirstInnerBld+i)->arAbility[j][2]>0)
						{
							int Mult=4;
							if (arPlayer[Player].GemInc<20)
							{Mult++;};
							if (arPlayer[Player].GemInc<10)
							{Mult++;};
							if (arPlayer[Player].GemInc<5)
							{Mult++;};
							*(pBld+i)+=(pFirstInnerBld+i)->arAbility[j][2]*Mult;
						};
						break;
					case 9:
					//Доход с ресурса
						*(pBld+i)+=(pFirstInnerBld+i)->arAbility[j][2]*arPlayer[Player].GetResource((pFirstInnerBld+i)->arAbility[j][1]);
						break;
					case 12:
					//Настроение
						*(pBld+i)+=(pFirstInnerBld+i)->arAbility[j][2]*50+(pFirstInnerBld+i)->arAbility[j][1]*10;
						break;
					case 16:
					case 22:
					//Общий доход, цена построек
						*(pBld+i)+=(pFirstInnerBld+i)->arAbility[j][1]*20;
						break;
					case 37:
					//Скорость
						*(pBld+i)+=(pFirstInnerBld+i)->arAbility[j][1]*100;
						break;						
					case 17:
					//Цены на ресурсы
						for (int R=1; R<=9; R++)
						{
							if (arPlayer[Player].GetResource(i)==0)
							{*(pBld+i)+=(pFirstInnerBld+i)->arAbility[j][1];};
						};
						break;
					case 46:
					//Карма
						if (pProfile->iKarma!=0)
						{
							if (sign(pProfile->iKarma)==sign((pFirstInnerBld+i)->arAbility[j][1]))
							{*(pBld+i)*=2;}
							else
							{
								*(pBld+i)/=3;
								if ((pFirstInnerBld+i)->arAbility[0][0]==8)
								{*(pBld+i)=0;};
							};
						};
						break;
					};
				if ((arPlayer[Player].Gold>=GoldLimit+arPlayer[Player].GetInBldGoldPrice(i))&&
				((arPlayer[Player].GetInBldGemPrice(i)==0)||(arPlayer[Player].Gem>=GemLimit+arPlayer[Player].GetInBldGemPrice(i))))
				{	
					if (YesTaste<*(pBld+i))
					{YesTaste=*(pBld+i);};
					if (*(pBld+i)<0)
					{*(pBld+i)=0;};
					RollerN[Count]=i;
					RollerP[Count]=*(pBld+i)+1;
				}
				else
				{
					if (NoTaste<*(pBld+i))
					{
						NoTaste=*(pBld+i);
						Wanna=i;
					};
				};

//				textprintf(screen,Font1,Count/35*300,(Count%35)*20,COLOR_2,"%s: %d",(pFirstInnerBld+i)->sName,*(pBld+i)+1);
//				textprintf(screen1,Font1,Count/35*300,(Count%35)*20,COLOR_2,"%s: %d",(pFirstInnerBld+i)->sName,*(pBld+i)+1);


				Count++;
			};
		};
	};
//	clear_keybuf();
//	readkey();
	if (Count>0)
	{
		bool Builder=true;
		if (NoTaste>YesTaste)
		{
			if (NoTaste-YesTaste-Random(1000)>0)
			{Builder=false;};
		};
		if (Builder)
		{
//			StopLog("Выбор....              ", Count);
			Count=Roll(false);
//			StopLog("Строим....              ", Count);
			arPlayer[Player].Build(Count);
			char Mess[500];
			sprintf(Mess,"%s построил %s в родовом замке",arPlayer[Player].GetName(),(pFirstInnerBld+Count)->sName);
			FileLog(Mess);
		}
		else
		{
			char Mess[500];
			sprintf(Mess,"%s копит ресурсы на строительство %s",arPlayer[Player].GetName(),(pFirstInnerBld+Wanna)->sName);
			FileLog(Mess);
		};
	};
	delete [] pBld;
};

//****Распределяет юнитов отряда по слотам, если Slot>0 - резервирует место под юнита данного ранга
void ArrangeUnits(dcHero * Hero, int Slot)
{
	int FreeSlot=0;
	dcUnit * Squad[MAX_SQUAD];
	for (int i=1; i<MAX_SQUAD; i++)
	{
		Squad[i]=Hero->pSquad[i];
		Hero->pSquad[i]=NULL;
	};
	for (int Rank=4; Rank>0; Rank--)
	{
		if (Rank==Slot)
		{FreeSlot--;};
		FreeSlot+=Hero->GetUnitSlot(Rank);
		for (int i=1; i<MAX_SQUAD; i++)
		{
			if ((Squad[i])&&(Squad[i]->GetRank()==Rank))
			{
				FreeSlot--;
			};
		};
		if (FreeSlot<0)
		{
			for (int k=0; k<abs(FreeSlot); k++)
			{
				int Weak=0;
				for (int i=1; i<MAX_SQUAD; i++)
				{
					if ((Squad[i])&&(Squad[i]->GetRank()==Rank)&&((Weak==0)||
						(Squad[i]->iLevel*3+Squad[i]->GetGoldPrice()+Squad[i]->GetGemPrice()*5<Squad[Weak]->iLevel*3+Squad[Weak]->GetGoldPrice()+Squad[Weak]->GetGemPrice()*5)))
					{
						Weak=i;
					};
				};
				if (Weak>0)
				{
					delete Squad[Weak];
					Squad[Weak]=NULL;
				};
			};
		};
		for (int i=1; i<MAX_SQUAD; i++)
		{
			if ((Squad[i])&&(Squad[i]->GetRank()==Rank))
			{
				Hero->pSquad[Hero->GetFreeUnitSlot(Rank)]=Squad[i];
			};
		};
	};
};
int arSpecUnit[40][2];

//Добавляет юнита в список, если его там ещё нет, или увеличивает их количество
void AddSpecUnit(int Index, int Quantity)
{
	for (int i=0; i<40; i++)
	{
		if (arSpecUnit[i][0]==0)
		{
			arSpecUnit[i][0]=Index;
			arSpecUnit[i][1]=Quantity;
			return;
		};
		if (arSpecUnit[i][0]==Index)
		{
			if (arSpecUnit[i][1]>0)
			{
				arSpecUnit[i][1]+=Quantity;
			};
			return;
		};
	};
};

//Создаёт список особых юнитов для найма
void CalcSpecUnits(int Player, dcProvince *PR)
{
	for (int i=0; i<20; i++)
	{
		arSpecUnit[i][0]=0;
		arSpecUnit[i][1]=0;
	};
	//Юниты из сайтов
	if (arPlayer[Player].iDifficulty>0)
	{
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if (PR->arSite[i][1]<=PR->iExplore)
		{
		switch (PR->arSite[i][0])
		{
		case 35:
			AddSpecUnit(39,0);
			break;
		case 42:
			if ((CurGame.iCurrentTurn>100-arPlayer[Player].iDifficulty*10)&&(Random(10)==0))
			{AddSpecUnit(51,0);};
			break;
		case 43:
			if (arPlayer[Player].iKarma>=0)
			{
				AddSpecUnit(17,0);
				AddSpecUnit(35,0);
			};
			break;
		case 44:
			AddSpecUnit(31,0);
			break;
		case 45:
			if (!PR->arSiteGuard[i])
			{AddSpecUnit(54,0);};
			break;
		};
		};
	};
	};
	//Юниты из предметов
	for (int i=0; i<TREASURY_SIZE; i++)
	{
		if ((arPlayer[Player].arTreasury[i][0]>0)&&((pFirstItem+arPlayer[Player].arTreasury[i][0])->arEffect[0][0]==84)&&(arPlayer[Player].GetBldParam2(31,arPlayer[Player].arTreasury[i][0])))
		{
			AddSpecUnit((pFirstItem+arPlayer[Player].arTreasury[i][0])->arEffect[0][1],1);
		};
	};
};

//****Пополнение отряда героя и обмен войсками с гарнизоном
void HireUnits(int Player, int Hero)
{
	dcHero *HR=arPlayer[Player].arHero[Hero];
	CalcSpecUnits(Player,HR->pProvince);
	bool bCastle=false;
	if (HR->pProvince->pGarrison)
	{bCastle=true;};
	if ((HR->pProvince->ipPlayer!=Player)||((!bCastle)&&(arSpecUnit[0][0]==0)))
	{return;};
	//Определяем карму набираемых в отряд воинов
	int Rank=4;
	while (HR->GetUnitSlot(Rank)==0)
	{Rank--;};
	int Karma=0;
	bool bBad=false, bGood=false, bRepeat=true;
	if (bCastle)
	{
		while (bRepeat)
		{
			for (int i=0; i<MAX_RECRUIT; i++)
			{
				if ((arPlayer[Player].arRecruit[i])&&((pFirstUnitType+arPlayer[Player].arRecruit[i])->iLevel==Rank))
				{
					if ((pFirstUnitType+arPlayer[Player].arRecruit[i])->iKarma<0)
					{bBad=true;};
					if ((pFirstUnitType+arPlayer[Player].arRecruit[i])->iKarma>0)
					{bGood=true;};
					bRepeat=false;
				};
			};
			Rank--;
			if (Rank==0)
			{bRepeat=false;};
		};
		if ((bBad)&(!bGood))
		{Karma=-1;};
		if ((!bBad)&(bGood))
		{Karma=1;};
	};
	if (Karma==0)
	{
		int Bad=0, Good=0;
		for (int i=1; i<MAX_SQUAD; i++)
		{
			if (HR->pSquad[i])
			{
				if (HR->pSquad[i]->pType()->iKarma>0)
				{Good+=HR->pSquad[i]->pType()->iLevel*2+HR->pSquad[i]->iLevel;};
				if (HR->pSquad[i]->pType()->iKarma<0)
				{Bad+=HR->pSquad[i]->pType()->iLevel*2+HR->pSquad[i]->iLevel;};
			};
		};
		if (Bad>Good)
		{Karma=-1;};
		if (Good>Bad)		
		{Karma=1;};
	};
	ArrangeUnits(HR,0);
	if (bCastle)
	{
		ArrangeUnits(HR->pProvince->pGarrison,0);
		for (int i=1; i<MAX_SQUAD; i++)
		//Забираем из гарнизона здоровых
		{
			if ((HR->CanUseUnitSlot(i))&&((HR->pSquad[i]==NULL)||(HR->pSquad[i]->iLife<HR->pSquad[i]->GetLife()/2)))
			{
				int Choosen=0;
				for (int j=1; j<MAX_SQUAD; j++)
				{
					if ((HR->pProvince->pGarrison->pSquad[j])&&
						(HR->pProvince->pGarrison->pSquad[j]->GetRank()<=GetUnitSlotLevel(i))&&
						((!HR->pSquad[i])||(HR->pSquad[i]->GetRank()<=GetUnitSlotLevel(j)))&&
						(HR->pProvince->pGarrison->pSquad[j]->iLife>HR->pProvince->pGarrison->pSquad[j]->GetLife()*6/10)&&
						((HR->pProvince->pGarrison->pSquad[j]->pType()->iKarma==0)||(Karma==0)||
						(sign(HR->pProvince->pGarrison->pSquad[j]->pType()->iKarma)==Karma)))
					{
						if ((Choosen==0)||(HR->pProvince->pGarrison->pSquad[j]->GetRank()>HR->pProvince->pGarrison->pSquad[Choosen]->GetRank()))
						{
							Choosen=j;
						}
						else
						{
							if ((HR->pProvince->pGarrison->pSquad[j]->GetRank()==HR->pProvince->pGarrison->pSquad[Choosen]->GetRank())&&
								(HR->pProvince->pGarrison->pSquad[j]->iLife/HR->pProvince->pGarrison->pSquad[j]->GetLife()*100>HR->pProvince->pGarrison->pSquad[Choosen]->iLife*100/HR->pProvince->pGarrison->pSquad[Choosen]->GetLife()))
							{
								Choosen=j;
							};
						};
					};
				};
				if (Choosen>0)
				{
					if (HR->pProvince->pGarrison->pSquad[Choosen]->pType()->iKarma!=0)
					{Karma=sign(HR->pProvince->pGarrison->pSquad[Choosen]->pType()->iKarma);};
					dcUnit * Temp=HR->pSquad[i];
					HR->pSquad[i]=HR->pProvince->pGarrison->pSquad[Choosen];
					HR->pProvince->pGarrison->pSquad[Choosen]=Temp;
					ArrangeUnits(HR,0);
					ArrangeUnits(HR->pProvince->pGarrison,0);
					
				};
			};
		};
		for (int i=1; i<MAX_SQUAD; i++)
		//Оставляем в гарнизоне раненых
		{
			if ((HR->pSquad[i])&&(HR->pSquad[i]->pType()->iUnitKind!=2)&&((HR->pSquad[i]->iLife<HR->pSquad[i]->GetLife()/2)||
				((HR->pSquad[i]->pType()->iKarma!=0)&&(Karma!=0)&&(sign(HR->pSquad[i]->pType()->iKarma)!=Karma))))
			{				
				if (HR->pProvince->pGarrison->GetFreeUnitSlot(HR->pSquad[i]->GetRank()))
				{
					HR->pProvince->pGarrison->pSquad[HR->pProvince->pGarrison->GetFreeUnitSlot(HR->pSquad[i]->GetRank())]=HR->pSquad[i];
					HR->pSquad[i]=NULL;
				};
			};
		};
		ArrangeUnits(HR,0);
	};
//		int FreeSlot=0, MaxFreeSlot;
//		MaxFreeSlot
	for (int Rank=4; Rank>0; Rank--)
	{
		//Найм новобранцев
		int Count=1;
		while ((Count>0)&&(HR->GetFreeUnitSlot(Rank)))
		{
			Count=0;
			ClearRoll();
			if (bCastle)
			{
				//Из гарнизона
				for (int i=0; i<MAX_RECRUIT-1; i++)
				{
					if ((arPlayer[Player].arRecruit[i]>0)&&((pFirstUnitType+arPlayer[Player].arRecruit[i])->iLevel==Rank)&&
						(arPlayer[Player].Gold>=arPlayer[Player].GetUnitGoldPrice(arPlayer[Player].arRecruit[i],arPlayer[Player].ipProvince))&&(arPlayer[Player].Gem>=arPlayer[Player].GetUnitGemPrice(arPlayer[Player].arRecruit[i],arPlayer[Player].ipProvince)))
					{
						RollerN[Count]=arPlayer[Player].arRecruit[i];
						RollerP[Count]=10+GetUnitPower(arPlayer[Player].arRecruit[i]);
						if (((pFirstUnitType+arPlayer[Player].arRecruit[i])->iKarma!=0)&&(Karma!=0)&&
							(sign((pFirstUnitType+arPlayer[Player].arRecruit[i])->iKarma)!=Karma))
						{RollerP[Count]=1;};
						Count++;
					};
				};
			};
			//Из особых мест
			for (int i=0; i<40; i++)
			{
				if ((arSpecUnit[i][0]>0)&&((pFirstUnitType+arSpecUnit[i][0])->iLevel==Rank)&&
					((pFirstUnitType+arSpecUnit[i][0]!=0)||
					((arPlayer[Player].Gold>=arPlayer[Player].GetUnitGoldPrice(arSpecUnit[i][0],arPlayer[Player].ipProvince))&&(arPlayer[Player].Gem>=arPlayer[Player].GetUnitGemPrice(arSpecUnit[i][0],arPlayer[Player].ipProvince)))))
				{
					RollerN[Count]=arSpecUnit[i][0];
					RollerP[Count]=10+GetUnitPower(arSpecUnit[i][0]);
					if (((pFirstUnitType+arSpecUnit[i][0])->iKarma!=0)&&(Karma!=0)&&
						(sign((pFirstUnitType+arSpecUnit[i][0])->iKarma)!=Karma))
					{RollerP[Count]=1;};
					Count++;
				};
			};
			if (Count>0)
			{
				int Recruit=Roll(false);
				dcUnit *Rookie=new dcUnit(Recruit,arPlayer[Player].GetBldParam1(20));
				if (((pFirstUnitType+Recruit)->iKarma!=0)&&(Karma==0))
				{Karma=sign((pFirstUnitType+Recruit)->iKarma);};
				HR->PlaceUnit(Rookie,HR->GetFreeUnitSlot(Rank));
			//	if ((arPlayer[Player].Gold>=arPlayer[Player].GetUnitGoldPrice(Recruit,arPlayer[Player].ipProvince))&&(arPlayer[Player].Gem>=arPlayer[Player].GetUnitGemPrice(Recruit,arPlayer[Player].ipProvince)))
			//	{
					arPlayer[Player].Gold-=arPlayer[Player].GetUnitGoldPrice(Recruit,arPlayer[Player].ipProvince);
					arPlayer[Player].Gem-=arPlayer[Player].GetUnitGemPrice(Recruit,arPlayer[Player].ipProvince);
					for (int i=0; i<MAX_UNIT_RES; i++)
					{							
						if ((pFirstUnitType+Recruit)->arRes[i]>0)
						{
							if (arPlayer[Player].GetResource((pFirstUnitType+Recruit)->arRes[i])==0)
							{							
								CurGame.ResPrices[(pFirstUnitType+Recruit)->arRes[i]]+=CurGame.ResBuy;
							};
						};					
					};
			//	};
				//Удаление предмета из сокровищницы, если юнит из предмета
				for (int i=0; i<40; i++)
				{					
					if ((arSpecUnit[i][0]==Recruit)&&(arSpecUnit[i][1]>0))
					{
						arPlayer[Player].Gold+=arPlayer[Player].GetUnitGoldPrice(Recruit,arPlayer[Player].ipProvince);
						arPlayer[Player].Gem+=arPlayer[Player].GetUnitGemPrice(Recruit,arPlayer[Player].ipProvince);
						arSpecUnit[i][1]--;
						if(arSpecUnit[i][1]==0)
						{arSpecUnit[i][0]=0;};
						for (int j=0; j<TREASURY_SIZE; j++)
						{
							if ((arPlayer[Player].arTreasury[j][0]>0)&&(Recruit==(pFirstItem+arPlayer[Player].arTreasury[j][0])->arEffect[0][1]))
							{								
								arPlayer[Player].arTreasury[j][0]=0;
								arPlayer[Player].arTreasury[j][1]=0;
								break;
							};
						};
					};
				};
			};				
		};
	};
	//Ритуалы по вызову юнитов
	if (Karma<1)
	{
		if ((HR->GetFreeUnitSlot(2))&&(IsRitualCheap(Player,11)))
		{arPlayer[Player].CastRitual(11,HR->ipProvince,Hero);};
		if ((HR->GetFreeUnitSlot(1))&&(IsRitualCheap(Player,6)))
		{arPlayer[Player].CastRitual(6,HR->ipProvince,Hero);};		
	};
};

//****Изучение заклинаний
void LearnSpells(int Player, int Hero)
{	
	bool bLib=false;	
	dcHero *HR=arPlayer[Player].arHero[Hero];
	if (HR->pProvince->ipPlayer!=Player)
	{return;};
	if (arPlayer[Player].arLibrary[0]>0)
	{		
		if (arPlayer[Player].ipProvince==HR->pProvince->iNumber)
		{
			bLib=true;
		}
		else
		{
			for (int i=0; i<MAX_PROVINCE_BLD; i++)
			{
				if (HR->pProvince->arBuilding[i]>0)
				{					
					if ((pFirstOuterBld+HR->pProvince->arBuilding[i])->bLibrary)
					{
						bLib=true;
					};
				};
			};
		};
	};
	if (bLib)
	{
		for (int Rank=4; Rank>0; Rank--)
		{
			if (HR->GetSpellSlot(Rank)>0)
			{
				int Count=0, Lvl=Rank;
				ClearRoll();
				while (Count==0)
				{
					for (int i=0; i<LIBRARY_SIZE; i++)
					{
						if ((arPlayer[Player].arLibrary[i]>0)&&(pFirstSpell+arPlayer[Player].arLibrary[i])->iLevel==Lvl)
						{
							RollerN[Count]=arPlayer[Player].arLibrary[i];
							RollerP[Count]=GetSpellTaste(arPlayer[Player].arLibrary[i],HR);
							/*clSpell *SP=(pFirstSpell+arPlayer[Player].arLibrary[i]);
							if (SP->iPowerMod>0)
							{RollerP[Count]+=HR->CheckHeroBonus(905)*SP->iPowerMod/100;};

							for (int i=0; i<MAX_SPELL_EFFECT; i++)
							{
								if (SP->arEffect[0][i]==1)
								{
									int Bonus=SP->arEffect[0][i]+HR->CheckHeroBonus(905)*SP->iPowerMod/100;
									if ((SP->iArea==2)&&(SP->iTarget==0))
									{Bonus*=8;};
									if ((SP->iArea==3)&&(SP->iTarget==1))
									{Bonus*=(1+SP->iRadius*3);};
									RollerP[Count]+=Bonus/2;
								};
								if (SP->arEffect[0][i]>1000)
								{
									RollerP[Count]+=5+HR->CheckHeroBonus(907);
									if ((pFirstUnitType+(SP->arEffect[0][i]-1000))->iUnitKind==2)
									{
										RollerP[Count]+=HR->CheckHeroBonus(909)*3;
									};
								};
							};*/
							Count++;
						};
					};
					Lvl--;
				};
				int Shift=0;
				Lvl=Rank;
				switch (Rank)
				{
				case 1:
					Shift=1;
					break;
				case 2:
					Shift=SpellLvl2;
					break;
				case 3:
					Shift=SpellLvl3;
					break;
				case 4:
					Shift=SpellLvl4;
					break;
				};
				for (int i=Shift; i<Shift+HR->GetSpellSlot(Rank); i++)
				{
					int Spell=Roll(false);
					if ((HR->arSpell[i]==0)||((pFirstSpell+Spell)->iLevel>=(pFirstSpell+HR->arSpell[i])->iLevel))
					{HR->arSpell[i]=Spell;};
					if ((arPlayer[Player].Control==2)&&(i==Shift))
					{
						int MLvl=1+CurGame.iCurrentTurn*3/(200-arPlayer[Player].iDifficulty*20);
						if (MLvl>Lvl)
						{MLvl=Lvl;};
						if (MLvl==Lvl)
						{
						switch (arPlayer[Player].ipProfile)
						{
						case 1:
							if (Lvl==1)
							{HR->arSpell[i]=19;};
							if (Lvl==2)
							{HR->arSpell[i]=34;};
							if (Lvl==3)
							{HR->arSpell[i]=43;};
							if (Lvl==4)
							{HR->arSpell[i]=68;};
							break;
						case 2:
							if (Lvl==1)
							{HR->arSpell[i]=4;};
							if (Lvl==2)
							{HR->arSpell[i]=23;};							
							if (Lvl==4)
							{HR->arSpell[i]=71;};
							break;
						case 3:
							if (Lvl==1)
							{HR->arSpell[i]=14;};
							if (Lvl==2)
							{HR->arSpell[i]=28;};
							if (Lvl==3)
							{HR->arSpell[i]=52;};
							if (Lvl==4)
							{HR->arSpell[i]=61;};
							break;
						case 4:
							if (Lvl==1)
							{HR->arSpell[i]=4;};
							if (Lvl==2)
							{HR->arSpell[i]=22;};
							if (Lvl==3)
							{HR->arSpell[i]=49;};
							if (Lvl==4)
							{HR->arSpell[i]=58;};
							break;
						case 5:
							if (Lvl==1)
							{HR->arSpell[i]=12;};							
							break;
						case 7:
							if (Lvl==1)
							{HR->arSpell[i]=15;};
							if (Lvl==2)
							{HR->arSpell[i]=28;};
							if (Lvl==3)
							{HR->arSpell[i]=55;};
							if (Lvl==4)
							{HR->arSpell[i]=61;};
							break;
						case 8:
							if (Lvl==1)
							{HR->arSpell[i]=4;};
							if (Lvl==2)
							{HR->arSpell[i]=22;};
							if (Lvl==3)
							{HR->arSpell[i]=52;};
							if (Lvl==4)
							{HR->arSpell[i]=74;};
							break;
						case 9:
							if (Lvl==1)
							{HR->arSpell[i]=4;};
							if (Lvl==2)
							{HR->arSpell[i]=23;};
							if (Lvl==3)
							{HR->arSpell[i]=46;};
							if (Lvl==4)
							{HR->arSpell[i]=72;};
							break;
						case 10:							
							if (Lvl==1)
							{HR->arSpell[i]=4;};
							if (Lvl==2)
							{HR->arSpell[i]=22;};
							if (Lvl==3)
							{HR->arSpell[i]=50;};							
							break;
						case 12:
							if (Lvl==1)
							{HR->arSpell[i]=12;};
							if (Lvl==2)
							{HR->arSpell[i]=37;};
							if (Lvl==3)
							{HR->arSpell[i]=51;};
							if (Lvl==4)
							{HR->arSpell[i]=58;};
							break;
						case 13:							
							if (Lvl==1)
							{HR->arSpell[i]=15;};
							if (Lvl==2)
							{HR->arSpell[i]=32;};
							if (Lvl==3)
							{HR->arSpell[i]=41;};
							if (Lvl==4)
							{HR->arSpell[i]=66;};
							break;
						case 14:							
							if (Lvl==1)
							{HR->arSpell[i]=4;};							
							break;
						case 16:							
							if (Lvl==1)
							{HR->arSpell[i]=19;};
							if (Lvl==2)
							{HR->arSpell[i]=23;};
							if (Lvl==3)
							{HR->arSpell[i]=47;};
							if (Lvl==4)
							{HR->arSpell[i]=64;};
							break;
						};
						};
					};
				};
			};
		};
	};
};

//Возвращает номер слота, если герой эту вещь хочет надеть и 0 если не хочет
int TryItem(int Item, dcHero *Hero, bool TwoHand)
{
	if (!Hero->CanUse(Item))
	{return 0;};
	clItem *IT=(pFirstItem+Item);
	int Slot, SlotTaste=10000;
	switch (IT->ipSlot)
	{
	case 1:
		if (Hero->arInventory[1][0]>0)
		{
			if (((pFirstItem+Hero->arInventory[1][0])->ipSlot==100)&&(!TwoHand))
			{return 1;};
			if (((pFirstItem+Hero->arInventory[1][0])->ipSlot==1)&&(GetItemTaste(Item,Hero)>GetItemTaste(Hero->arInventory[1][0],Hero)))
			{return 1;};
		}
		else
		{return 1;}
		break;
	case 100:
		if ((Hero->arInventory[1][0]>0)||(Hero->arInventory[2][0]>0))
		{
			if ((pFirstItem+Hero->arInventory[1][0])->ipSlot==100)
			{
				if (GetItemTaste(Item,Hero)>GetItemTaste(Hero->arInventory[1][0],Hero))
				{return 1;};
			}
			else
			{
				if (TwoHand)
				{
					int OldTaste=0, NewTaste=GetItemTaste(Item,Hero)+IT->iRarity/2;
					if (Hero->arInventory[1][0]>0)
					{OldTaste+=GetItemTaste(Hero->arInventory[1][0],Hero);};
					if (Hero->arInventory[2][0]>0)
					{OldTaste+=GetItemTaste(Hero->arInventory[2][0],Hero);};
					if (NewTaste>OldTaste)
					{return 1;};
				};
			};
		}
		else
		{return 1;}
		break;
	case 2:
		if (Hero->arInventory[2][0]>0)
		{			
			if (GetItemTaste(Item,Hero)>GetItemTaste(Hero->arInventory[2][0],Hero))
			{return 2;};
		}
		else
		{
			if ((Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100)&&(TwoHand))
			{return 0;}
			else
			{return 2;};
		};
		break;
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:		
		if (Hero->arInventory[IT->ipSlot][0]>0)
		{
			if (GetItemTaste(Item,Hero)>GetItemTaste(Hero->arInventory[IT->ipSlot][0],Hero))
			{return IT->ipSlot;};
		}
		else
		{return IT->ipSlot;};
		break;
	case 10:
		for (int i=10; i<12; i++)
		{
			if (Hero->arInventory[i][0]>0)
			{
				if (SlotTaste>GetItemTaste(Hero->arInventory[i][0],Hero))
				{
					Slot=i;
					SlotTaste=GetItemTaste(Hero->arInventory[i][0],Hero);
				};
			}
			else
			{
				Slot=i;
				SlotTaste=0;
			};
		};
		if (GetItemTaste(Item,Hero)>SlotTaste)
		{return Slot;};
		break;
	case 11:
		for (int i=12; i<16; i++)
		{
			if (Hero->arInventory[i][0]>0)
			{
				if (SlotTaste>GetItemTaste(Hero->arInventory[i][0],Hero))
				{
					Slot=i;
					SlotTaste=GetItemTaste(Hero->arInventory[i][0],Hero);
				};
			}
			else
			{
				Slot=i;
				SlotTaste=0;
			};
		};
		if (GetItemTaste(Item,Hero)>SlotTaste)
		{return Slot;};
		break;
	};
	return 0;
};

//Возвращает истину, если предмет никем не востребован
bool CanSell(int Player, int Item)
{
	if ((pFirstItem+Item)->arEffect[0][0]==84)
	{return false;};
	for (int i=0; i<SHOP_SIZE; i++)
	{
		if (arPlayer[Player].Shop.Items[i][0]==Item)
		{return true;};
	};
	for (int i=0; i<MAX_HERO; i++)
	{
		if (arPlayer[Player].arHero[i])
		{
			if (TryItem(Item,arPlayer[Player].arHero[i],true)>0)
			{return false;};
			if (((pFirstItem+Item)->iRarity>1)&&(TryItem(Item,arPlayer[Player].arHero[i],false)>0))
			{return false;};
		};
	};
	return true;
};

//Оплата покупки предмета и изменение цен ресурсов
void BuyItem(int Player, int Item)
{
	arPlayer[Player].Gold-=arPlayer[Player].GetItemGoldPrice(Item, &arPlayer[Player].Shop);
	arPlayer[Player].Gem-=arPlayer[Player].GetItemGemPrice(Item, &arPlayer[Player].Shop);
	for (int i=0; i<MAX_ITEM_RES; i++)
	{
		if (((pFirstItem+Item)->arRes[i]>0)&&(arPlayer[Player].GetResource((pFirstItem+Item)->arRes[i])==0))
		{
			CurGame.ResPrices[(pFirstItem+Item)->arRes[i]]+=CurGame.ResBuy;
		};
	};
	for (int i=0; i<SHOP_SIZE; i++)
	{
		if ((arPlayer[Player].Shop.Items[i][0]==Item)&&(arPlayer[Player].Shop.Items[i][1]>0))
		{
			arPlayer[Player].Shop.Items[i][1]--;
			if (arPlayer[Player].Shop.Items[i][1]==0)
			{
				arPlayer[Player].Shop.Items[i][0]=0;
			};
		};
	};
};

//Снятие предмета
void Undress(int Player, dcHero *HR, int Slot)
{
	if (HR->arInventory[Slot][0]>0)
	{
		if (CanSell(Player,HR->arInventory[Slot][0]))
		{
			arPlayer[Player].SellItem(HR->arInventory[Slot][0],HR->arInventory[Slot][1]);
		}
		else
		{
			bool NeedPlace=true;
			for (int k=0; k<TREASURY_SIZE; k++)
			{
				if (arPlayer[Player].arTreasury[k][0]==0)
				{
					NeedPlace=false;
					arPlayer[Player].arTreasury[k][0]=HR->arInventory[Slot][0];
					arPlayer[Player].arTreasury[k][1]=HR->arInventory[Slot][1];								
					break;
				};
			};
			if (NeedPlace)
			{
				for (int k=16; k<INVENTORY_SIZE; k++)
				{
					if (HR->arInventory[k][0]==0)
					{
						NeedPlace=false;
						HR->arInventory[k][0]=HR->arInventory[Slot][0];
						HR->arInventory[k][1]=HR->arInventory[Slot][1];								
						break;
					};
				};
			};
			if (NeedPlace)
			{
				arPlayer[Player].SellItem(HR->arInventory[Slot][0],HR->arInventory[Slot][1]);
			};
		};
	};
	HR->arInventory[Slot][0]=0;
	HR->arInventory[Slot][1]=0;
};


//****Менеджер предметов
void EquipItems(int Player, int Hero)
{
	dcHero *HR=arPlayer[Player].arHero[Hero];
//Надеваем подходящие вещи из инвентаря
	bool TwoHand=false;
	for (int j=0; j<3; j++)
	{
		if (j==2)
		{
			TwoHand=true;
		};
		for (int i=16; i<=INVENTORY_SIZE; i++)
		{
			if (HR->arInventory[i][0])
			{
				int Slot=TryItem(HR->arInventory[i][0],HR,TwoHand);
				if (Slot>0)
				{
					bool CanDo=true;
					if ((Slot==1)&&((pFirstItem+HR->arInventory[i][0])->ipSlot==100)&&(HR->arInventory[2][0]>0))
					{
						CanDo=false;
						for (int k=16; k<INVENTORY_SIZE; k++)
						{
							if (HR->arInventory[k][0]==0)
							{//Взятие двуруча при экипированном щите
								CanDo=true;
								HR->arInventory[k][0]=HR->arInventory[2][0];
								HR->arInventory[k][1]=HR->arInventory[2][1];
								HR->arInventory[2][0]=0;
								HR->arInventory[2][1]=0;
								break;
							};
						};
					};
					if ((Slot==2)&&(HR->arInventory[1][0]>0)&&((pFirstItem+HR->arInventory[1][0])->ipSlot==100))
					{//Взятие щита при экипированном двуручном оружии
						int It=HR->arInventory[i][0], Dr=HR->arInventory[i][1];
						HR->arInventory[i][0]=HR->arInventory[1][0];
						HR->arInventory[i][1]=HR->arInventory[1][1];
						HR->arInventory[Slot][0]=It;
						HR->arInventory[Slot][1]=Dr;
						HR->arInventory[1][0]=0;
						HR->arInventory[1][1]=0;
						CanDo=false;
					};
					if (CanDo)
					{
						int It=HR->arInventory[i][0], Dr=HR->arInventory[i][1];
						HR->arInventory[i][0]=HR->arInventory[Slot][0];
						HR->arInventory[i][1]=HR->arInventory[Slot][1];
						HR->arInventory[Slot][0]=It;
						HR->arInventory[Slot][1]=Dr;
					};
				};
			};			
		};
	};
//Проверка, доступен ли магазин
	if (HR->pProvince->ipPlayer!=Player)
	{return;};
	bool bShop=false;	
	if (arPlayer[Player].Shop.Items[0][0]>0)
	{		
		if (arPlayer[Player].ipProvince==HR->pProvince->iNumber)
		{
			bShop=true;
		}
		else
		{
			for (int i=0; i<MAX_PROVINCE_BLD; i++)
			{
				if (HR->pProvince->arBuilding[i]>0)
				{					
					if ((pFirstOuterBld+HR->pProvince->arBuilding[i])->bShop)
					{
						bShop=true;
					};
				};
			};
		};
	};
	if (bShop)
	{
//Экипировка предметов из сокровищницы
		TwoHand=false;
		for (int j=0; j<3; j++)
		{
			if (j==2)
			{
				TwoHand=true;
			};
			for (int i=0; i<TREASURY_SIZE; i++)
			{
				if (arPlayer[Player].arTreasury[i][0])
				{
					int Slot=TryItem(arPlayer[Player].arTreasury[i][0],HR,TwoHand);
					if (Slot>0)
					{
						bool CanDo=true;
						if ((Slot==1)&&((pFirstItem+arPlayer[Player].arTreasury[i][0])->ipSlot==100)&&(HR->arInventory[2][0]>0))
						{
							CanDo=false;
							for (int k=0; k<TREASURY_SIZE; k++)
							{
								if (arPlayer[Player].arTreasury[k][0]==0)
								{//Взятие двуруча при экипированном щите
									CanDo=true;
									arPlayer[Player].arTreasury[k][0]=HR->arInventory[2][0];
									arPlayer[Player].arTreasury[k][1]=HR->arInventory[2][1];
									HR->arInventory[2][0]=0;
									HR->arInventory[2][1]=0;
									break;
								};
							};
						};
						if ((Slot==2)&&(HR->arInventory[1][0]>0)&&((pFirstItem+HR->arInventory[1][0])->ipSlot==100))
						{//Взятие щита при экипированном двуручном оружии
							int It=arPlayer[Player].arTreasury[i][0], Dr=arPlayer[Player].arTreasury[i][1];
							arPlayer[Player].arTreasury[i][0]=HR->arInventory[1][0];
							arPlayer[Player].arTreasury[i][1]=HR->arInventory[1][1];
							HR->arInventory[Slot][0]=It;
							HR->arInventory[Slot][1]=Dr;
							HR->arInventory[1][0]=0;
							HR->arInventory[1][1]=0;
							CanDo=false;
						};
						if (CanDo)
						{
							int It=arPlayer[Player].arTreasury[i][0], Dr=arPlayer[Player].arTreasury[i][1];
							arPlayer[Player].arTreasury[i][0]=HR->arInventory[Slot][0];
							arPlayer[Player].arTreasury[i][1]=HR->arInventory[Slot][1];
							HR->arInventory[Slot][0]=It;
							HR->arInventory[Slot][1]=Dr;
						};
					};
				};			
			};
		};
//Продажа предметов из инвентаря
		for (int i=16; i<=INVENTORY_SIZE; i++)
		{
			if ((HR->arInventory[i][0]>0)&&(CanSell(Player,HR->arInventory[i][0])))
			{
				//Выбор предмета для торговли
				bool bSell=true;
				if ((pFirstItem+HR->arInventory[i][0])->iRarity>2)
				{
				if (arPlayer[Player].arTreasury[TREASURY_SIZE-1][0]==0)
				{bSell=false;}
				else
				{
					if ((pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iShopLevel+(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iRarity>
						(pFirstItem+HR->arInventory[i][0])->iShopLevel+(pFirstItem+HR->arInventory[i][0])->iRarity)
					{bSell=false;}
					else
					{
						if (((pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iShopLevel+(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iRarity==
						(pFirstItem+HR->arInventory[i][0])->iShopLevel+(pFirstItem+HR->arInventory[i][0])->iRarity)&&
						((pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iGoldCost+(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iGemCost*3>
						(pFirstItem+HR->arInventory[i][0])->iGoldCost+(pFirstItem+HR->arInventory[i][0])->iGemCost*3))
						{bSell=false;};
					};
				};
				};
				if (bSell)
				{arPlayer[Player].SellItem(HR->arInventory[i][0],HR->arInventory[i][1]);}
				else
				{
					arPlayer[Player].arTreasury[TREASURY_SIZE-1][0]=HR->arInventory[i][0];
					arPlayer[Player].arTreasury[TREASURY_SIZE-1][1]=HR->arInventory[i][1];
				};
				HR->arInventory[i][0]=0;
				HR->arInventory[i][1]=0;				
			};
		};
//Продажа предметов из сокровищницы
		for (int i=0; i<TREASURY_SIZE-1; i++)
		{
			if ((arPlayer[Player].arTreasury[i][0])&&(CanSell(Player,arPlayer[Player].arTreasury[i][0])))
			{
				arPlayer[Player].SellItem(arPlayer[Player].arTreasury[i][0],arPlayer[Player].arTreasury[i][1]);
				arPlayer[Player].arTreasury[i][0]=0;
				arPlayer[Player].arTreasury[i][1]=0;
			};
		};
		for (int i=0; i<TREASURY_SIZE-2; i++)
		{
			if (arPlayer[Player].arTreasury[i][0])
			{
				clItem *IT=(pFirstItem+arPlayer[Player].arTreasury[i][0]);
				if ((IT->ipSlot==1)||(IT->ipSlot==2))
				{
					for (int j=i+1; j<TREASURY_SIZE-1; j++)
					{
						if ((arPlayer[Player].arTreasury[j][0])&&
							((pFirstItem+arPlayer[Player].arTreasury[j][0])->ipSlot==IT->ipSlot)&&
							((pFirstItem+arPlayer[Player].arTreasury[j][0])->ipClass==IT->ipClass)&&
							((pFirstItem+arPlayer[Player].arTreasury[j][0])->iShopLevel+(pFirstItem+arPlayer[Player].arTreasury[j][0])->iRarity<=IT->iShopLevel+IT->iRarity))
						{
							arPlayer[Player].SellItem(arPlayer[Player].arTreasury[j][0],arPlayer[Player].arTreasury[j][1]);
							arPlayer[Player].arTreasury[j][0]=0;
							arPlayer[Player].arTreasury[j][1]=0;
						}
						else
						{
							arPlayer[Player].SellItem(arPlayer[Player].arTreasury[i][0],arPlayer[Player].arTreasury[i][1]);
							arPlayer[Player].arTreasury[i][0]=0;
							arPlayer[Player].arTreasury[i][1]=0;
							break;
						};
					};
				};				
			};
		};
//Перемещение инвентаря в сокровищницу
		for (int i=16; i<=INVENTORY_SIZE; i++)
		{
			if (HR->arInventory[i][0]>0)
			{
				for (int k=0; k<TREASURY_SIZE; k++)
				{
					if (arPlayer[Player].arTreasury[k][0]==0)
					{
						arPlayer[Player].arTreasury[k][0]=HR->arInventory[i][0];
						arPlayer[Player].arTreasury[k][1]=HR->arInventory[i][1];
						HR->arInventory[i][0]=0;
						HR->arInventory[i][1]=0;
						break;
					};
				};
			};
		};
//Покупка предметов
		int Choosen=1, Choosen1=0, Choosen2, Taste=0;
		while (Choosen>0)
		{
			Choosen=0, Choosen1=0, Choosen2=0, Taste=0;
			for (int i=0; i<SHOP_SIZE; i++)
			{
				if ((arPlayer[Player].Shop.Items[i][0]>0)&&
					(arPlayer[Player].GetItemGoldPrice(arPlayer[Player].Shop.Items[i][0],&arPlayer[Player].Shop)<=arPlayer[Player].Gold)&&
					(arPlayer[Player].GetItemGemPrice(arPlayer[Player].Shop.Items[i][0],&arPlayer[Player].Shop)<=arPlayer[Player].Gem))
				{
					int Slot=0;
					if ((pFirstItem+arPlayer[Player].Shop.Items[i][0])->ipSlot==100)
					{Slot=TryItem(arPlayer[Player].Shop.Items[i][0],HR,true);}
					else
					{Slot=TryItem(arPlayer[Player].Shop.Items[i][0],HR,false);};
					if (Slot>0)
					{
						if (((pFirstItem+HR->arInventory[1][0])->ipSlot==100)&&((pFirstItem+arPlayer[Player].Shop.Items[i][0])->ipSlot==1))
						{
							if ((Choosen1==0)||(GetItemTaste(arPlayer[Player].Shop.Items[i][0],HR)>GetItemTaste(Choosen1,HR)))
							{
								Choosen1=arPlayer[Player].Shop.Items[i][0];
							};
						}
						else
						{
						if (((pFirstItem+HR->arInventory[1][0])->ipSlot==100)&&((pFirstItem+arPlayer[Player].Shop.Items[i][0])->ipSlot==2))
						{
							if ((Choosen2==0)||(GetItemTaste(arPlayer[Player].Shop.Items[i][0],HR)>GetItemTaste(Choosen2,HR)))
							{
								Choosen2=arPlayer[Player].Shop.Items[i][0];
							};
						}
						else
						{
							int NewTaste=GetItemTaste(arPlayer[Player].Shop.Items[i][0],HR);
							if (HR->arInventory[Slot][0]>0)
							{
								NewTaste-=GetItemTaste(HR->arInventory[Slot][0],HR);
							};
							if ((Slot==1)&&((pFirstItem+arPlayer[Player].Shop.Items[i][0])->ipSlot==100)&&(HR->arInventory[2][0]>0))
							{
								NewTaste+=(pFirstItem+arPlayer[Player].Shop.Items[i][0])->iShopLevel*3+(pFirstItem+arPlayer[Player].Shop.Items[i][0])->iRarity-GetItemTaste(HR->arInventory[2][0],HR);
							};
							if (NewTaste>Taste)
							{
								Choosen=arPlayer[Player].Shop.Items[i][0];
								Taste=NewTaste;
							};

						};
						};
					};
				};
			};
			if (Choosen>0)
			{
				int Slot=TryItem(Choosen,HR,true);
				Undress(Player,HR,Slot);
				if ((Slot==1)&&((pFirstItem+Choosen)->ipSlot==100)&&(HR->arInventory[2][0]>0))
				{Undress(Player,HR,2);};
				HR->arInventory[Slot][0]=Choosen;
				HR->arInventory[Slot][1]=(pFirstItem+Choosen)->iDurability;
				BuyItem(Player,Choosen);
			};
			if ((Choosen1>0)&&(Choosen2>0)&&
				(arPlayer[Player].GetItemGoldPrice(Choosen1,&arPlayer[Player].Shop)+arPlayer[Player].GetItemGoldPrice(Choosen2,&arPlayer[Player].Shop)<=arPlayer[Player].Gold)&&
				(arPlayer[Player].GetItemGemPrice(Choosen1,&arPlayer[Player].Shop)+arPlayer[Player].GetItemGemPrice(Choosen2,&arPlayer[Player].Shop)<=arPlayer[Player].Gem))
			{
				int NewTaste=GetItemTaste(Choosen1,HR)+GetItemTaste(Choosen2,HR);
				if (HR->arInventory[1][0]>0)
				{
					NewTaste-=(GetItemTaste(HR->arInventory[1][0],HR)+(pFirstItem+HR->arInventory[1][0])->iShopLevel*3+(pFirstItem+HR->arInventory[1][0])->iRarity);
				};
				if (NewTaste>=Taste)
				{
					Undress(Player,HR,1);
					HR->arInventory[1][0]=Choosen1;
					HR->arInventory[1][1]=(pFirstItem+Choosen1)->iDurability;
					HR->arInventory[2][0]=Choosen2;
					HR->arInventory[2][1]=(pFirstItem+Choosen2)->iDurability;
					BuyItem(Player,Choosen1);
					BuyItem(Player,Choosen2);
				};
			};
		};
//Ремонт предметов
		for (int i=1; i<=INVENTORY_SIZE; i++)
		{		
			if ((HR->arInventory[i][0]>0)&&
			(arPlayer[Player].GetItemGoldRepPrice(HR->arInventory[i][0],HR->arInventory[i][1],&arPlayer[Player].Shop)))
			{
				arPlayer[Player].Gold-=arPlayer[Player].GetItemGoldRepPrice(HR->arInventory[i][0],HR->arInventory[i][1],&arPlayer[Player].Shop);
				arPlayer[Player].Gem-=arPlayer[Player].GetItemGemRepPrice(HR->arInventory[i][0],HR->arInventory[i][1],&arPlayer[Player].Shop);
				HR->arInventory[i][1]=(pFirstItem+HR->arInventory[i][0])->iDurability;
			};
		};
	};
};

//Сравнение сил отрядов. Возвращает 100 если силы равны, если больше - первый отряд сильнее
int CompareStrength(dcHero *Hero1, dcHero *Hero2)
{
	if (bVerySmartAI)
	{return CompareStrength1(Hero1,Hero2);};
	if (!Hero2)
	{return 300;};
	long Str[2], Def[2], Res[2], RDef[2], Spd[2], CAtt[2];
	dcHero *Army[2];
	Army[0]=Hero1;
	Army[1]=Hero2;

	//Вычисление общих параметров защиты и жизни
	for (int AA=0; AA<2; AA++)
	{
		Str[AA]=0;
		int Count=0, tmpSpeed;
		Def[AA]=0; RDef[AA]=0; Res[AA]=0; Spd[AA]=0; CAtt[AA]=0;
		for (int i=0; i<MAX_SQUAD; i++)
		{
			if (Army[AA]->pSquad[i])
			{
				Def[AA]+=Army[AA]->pSquad[i]->GetDefence();
				RDef[AA]+=Army[AA]->pSquad[i]->GetRangedDefence();
				Res[AA]+=Army[AA]->pSquad[i]->GetResist();
				tmpSpeed=Army[AA]->pSquad[i]->GetSpeed();
				if (tmpSpeed<1){tmpSpeed=1;}
				Spd[AA]+=tmpSpeed;
				CAtt[AA]+=Army[AA]->pSquad[i]->GetCounterAttack();
				Str[AA]+=Army[AA]->pSquad[i]->iLife;
				if (Army[AA]->ipClass1>0)
				{
					CAtt[AA]+=Army[AA]->CheckUnitBonus(3);
					Def[AA]+=Army[AA]->CheckUnitBonus(4);
					RDef[AA]+=Army[AA]->CheckUnitBonus(5);
					Res[AA]+=Army[AA]->CheckUnitBonus(6);
					Str[AA]+=Army[AA]->CheckUnitBonus(1);
				};
				Count++;
			};
		};	
		if (Count>0)
		{
			Def[AA]/=Count; RDef[AA]/=Count; Res[AA]/=Count; Spd[AA]/=Count; CAtt[AA]=CAtt[AA]*8/(10*Count);
		};
	};
	//Преимущество сильных компов над слабыми
	if ((arPlayer[Hero1->ipPlayer].Control!=1)&&(arPlayer[Hero2->ipPlayer].Control!=1))
	{		
		if (arPlayer[Hero1->ipPlayer].iDifficulty>arPlayer[Hero2->ipPlayer].iDifficulty)
		{
			Str[1]=Str[1]*(10-arPlayer[Hero1->ipPlayer].iDifficulty+arPlayer[Hero2->ipPlayer].iDifficulty)/10;
		}
		else
		{
			Str[0]=Str[0]*(10-arPlayer[Hero2->ipPlayer].iDifficulty+arPlayer[Hero1->ipPlayer].iDifficulty)/10;			
		};	
	};
	int Turn=0;
	while ((Str[0]>0)&&(Str[1]>0)&&(Turn<200))
	{
		for (int AA=0; AA<2; AA++)
		{				
			for (int i=0; i<MAX_SQUAD; i++)
			{				
				if (Army[AA]->pSquad[i])
				{
					if ((i==0)&&(Army[AA]->ipClass1==4)&&(Turn<Army[AA]->GetSpellSlot(1)+Army[AA]->GetSpellSlot(2)+Army[AA]->GetSpellSlot(3)+Army[AA]->GetSpellSlot(4)))
					{
						if (Turn%3==0)
						{Str[AA]+=Army[AA]->iLevel+3;}
						else
						{
							int Dmg=Army[AA]->iLevel+3-Res[1-AA];
							if (Dmg>0)
							{
								Str[1-AA]-=Dmg;
							};
						};
					}
					else
					{
					if ((7-Turn*(Spd[0]+Spd[1])<=Army[AA]->pSquad[i]->GetShootingRange())&&(Army[AA]->pSquad[i]->GetAmmo()+(7-Army[AA]->pSquad[i]->GetShootingRange())/(Spd[0]+Spd[1])>Turn))
					{
						int Deff=RDef[1-AA];
						if ((Army[AA]->pSquad[i]->CheckAbility(17))||(Army[AA]->pSquad[i]->CheckUpg(17)))
						{Deff/=2;};
						if ((Army[AA]->pSquad[i]->CheckAbility(28))||(Army[AA]->pSquad[i]->CheckUpg(28)))
						{Deff=Res[1-AA];};
						if (Army[AA]->pSquad[i]->GetRangedAttack()>Deff)
						{
							Str[1-AA]-=(Army[AA]->pSquad[i]->GetRangedAttack()-Deff);
						}
						else
						{
							Str[AA]+=(Army[AA]->pSquad[i]->CheckAbility(24)+Army[AA]->pSquad[i]->CheckUpg(24));
						};
					}
					else
					{
						if (Turn*(Spd[0]+Spd[1])>=4)
						{					
							int Deff=Def[1-AA];
							if ((Army[AA]->pSquad[i]->CheckAbility(39))||(Army[AA]->pSquad[i]->CheckUpg(39)))
							{Deff/=2;};
							if ((Army[AA]->pSquad[i]->CheckAbility(27))||(Army[AA]->pSquad[i]->CheckUpg(27)))
							{Deff=Res[1-AA];};
							int Att=Army[AA]->pSquad[i]->GetAttack();
							if (Army[AA]->ipClass1>0)
							{
								Att+=Army[AA]->CheckUnitBonus(2);
							};
							if (Att>Deff)
							{Str[1-AA]-=(Att-Deff);};
							if (Army[AA]->pSquad[i]->CheckAbility(26)==0)
							{
								Deff=Army[AA]->pSquad[i]->GetDefence();
								if (Army[AA]->ipClass1>0)
								{
									Deff+=Army[AA]->CheckUnitBonus(4);
								};
								if (CAtt[1-AA]>Deff)
								{Str[AA]-=(CAtt[1-AA]-Deff);};
							};
						};
					};
				};
				};
			};
		};
		Turn++;
	};	
	long Result=95+Str[0]-Str[1];
	if (Result>300)
	{Result=300;};
	return Result;
};

//Пытается построить внешнюю постройку с гарнизоном в указанной провинции. Возвращает истину, если попытка удалась.
bool TryBuildGarrison(int Player, int Province)
{
	dcProvince *PR=(pFirstProvince+Province);
	if ((PR->ipPlayer!=Player)||(arPlayer[Player].ipProvince==PR->iNumber))
	{return false;};
	for (int i=1; i<iMaxOuterBld; i++)
	{
		if ((*(arPlayer[Player].pOuterBld+i)!=0)&&(PR->CanBuild(i))&&
			(arPlayer[Player].Gold>=arPlayer[Player].GetOutBldGoldPrice(i))&&
			(arPlayer[Player].Gem>=arPlayer[Player].GetOutBldGemPrice(i)))
		{
			for (int j=0; j<MAX_BUILDING_ABILITY; j++)
			{
				if ((pFirstOuterBld+i)->arAbility[j][0]==4)
				{
					PR->ConstructBuilding(i,false, true);
					return true;
				};
			};		
		};
	};
	return false;
};

//Пытается построить внешнюю постройку с библиотекой в указанной провинции. Возвращает истину, если попытка удалась.
bool TryBuildLibrary(int Player, int Province)
{
	dcProvince *PR=(pFirstProvince+Province);
	if (PR->ipPlayer!=Player)
	{return false;};
	for (int i=1; i<iMaxOuterBld; i++)
	{
		if ((*(arPlayer[Player].pOuterBld+i)!=0)&&(PR->CanBuild(i))&&
			(arPlayer[Player].Gold>=arPlayer[Player].GetOutBldGoldPrice(i))&&
			(arPlayer[Player].Gem>=arPlayer[Player].GetOutBldGemPrice(i))&&
			((pFirstOuterBld+i)->bLibrary))
		{
			PR->ConstructBuilding(i,false, true);
			return true;			
		};
	};
	return false;
};

//Пытается построить внешнюю постройку с магазином в указанной провинции. Возвращает истину, если попытка удалась.
bool TryBuildShop(int Player, int Province)
{
	dcProvince *PR=(pFirstProvince+Province);
	if (PR->ipPlayer!=Player)
	{return false;};
	for (int i=1; i<iMaxOuterBld; i++)
	{
		if ((*(arPlayer[Player].pOuterBld+i)!=0)&&(PR->CanBuild(i))&&
			(arPlayer[Player].Gold>=arPlayer[Player].GetOutBldGoldPrice(i))&&
			(arPlayer[Player].Gem>=arPlayer[Player].GetOutBldGemPrice(i))&&
			((pFirstOuterBld+i)->bShop))
		{
			PR->ConstructBuilding(i,false, true);
			return true;			
		};
	};
	return false;
};

//Преверка соседей для атаки (с рекурсивным учетом портов)
void AttackNeighbour(int Player, dcProvince *PR, dcHero *HR, int Port)
{
	for (int j=0; j<MAX_NEIGHBOUR; j++)
	{
		if ((PR->arNeighbour[j]>0)&&((pFirstProvince+PR->arNeighbour[j])->ipPlayer!=Player)&&((pFirstProvince+PR->arNeighbour[j])->ipTerrain!=WATER_ID)&&((pFirstProvince+PR->arNeighbour[j])->ipTerrain!=MOUNTAIN_ID)&&
			(((pFirstProvince+PR->arNeighbour[j])->ipPlayer==0)||(arPlayer[Player].Politic[(pFirstProvince+PR->arNeighbour[j])->ipPlayer].Treaty==1)))
		{
			bool NoOther=true;
			for (int k=0; k<MAX_HERO; k++)
			{
				if ((arPlayer[Player].arHero[k])&&(arPlayer[Player].arHero[k]->iTarget==PR->arNeighbour[j])&&(arPlayer[Player].arHero[k]->iTask==1))
				{NoOther=false;};
			};				
			if ((*(pMoveMap+PR->arNeighbour[j])>0)&&(NoOther)&&(CompareStrength(HR,(pFirstProvince+PR->arNeighbour[j])->pGuard)>100)&&(CompareStrength(HR,(pFirstProvince+PR->arNeighbour[j])->pGarrison)>100))
			{
				if ((HR->iTask==1)&&(HR->iAITask==0))
				{							
					if ((*(pMoveMap+PR->arNeighbour[j])-(pFirstProvince+PR->arNeighbour[j])->arAttitude[Player][0]<*(pMoveMap+HR->iTarget)-(pFirstProvince+HR->iTarget)->arAttitude[Player][0])||
						(((pFirstProvince+HR->iTarget)->ipPlayer==0)&&((pFirstProvince+PR->arNeighbour[j])->ipPlayer!=0))||
						(((pFirstProvince+PR->arNeighbour[j])->ipPlayer!=0)&&(arPlayer[(pFirstProvince+PR->arNeighbour[j])->ipPlayer].ipProvince==(pFirstProvince+PR->arNeighbour[j])->iNumber)))
					{
						HR->iTask=1;
						HR->iAITask=0;
						HR->iTarget=PR->arNeighbour[j];
					}
					else
					{
						if ((*(pMoveMap+PR->arNeighbour[j])-(pFirstProvince+PR->arNeighbour[j])->arAttitude[Player][0]==*(pMoveMap+HR->iTarget)-(pFirstProvince+HR->iTarget)->arAttitude[Player][0])&&
							(((pFirstProvince+HR->iTarget)->ipPlayer==0)||((pFirstProvince+PR->arNeighbour[j])->ipPlayer!=0)))

						{
							if (((pFirstProvince+PR->arNeighbour[j])->GetResource(1)>0)&&((pFirstProvince+HR->iTarget)->GetResource(1)==0))
							{}
							else
							{
								if (CompareStrength(HR,(pFirstProvince+PR->arNeighbour[j])->pGuard)>CompareStrength(HR,(pFirstProvince+HR->iTarget)->pGuard))
								{
									HR->iTask=1;
									HR->iAITask=0;
									HR->iTarget=PR->arNeighbour[j];
								}
								else
								{
									if ((pFirstProvince+PR->arNeighbour[j])->GetGoldIncome()+(pFirstProvince+PR->arNeighbour[j])->GetGemIncome()*2>(pFirstProvince+HR->iTarget)->GetGoldIncome()+(pFirstProvince+HR->iTarget)->GetGemIncome()*2)
									{
										HR->iTask=1;
										HR->iAITask=0;
										HR->iTarget=PR->arNeighbour[j];
									};
								};
							};
						};
					};
				}
				else
				{
					HR->iTask=1;
					HR->iAITask=0;
					HR->iTarget=PR->arNeighbour[j];
				};
			};
		};
		if ((PR->arNeighbour[j]>0)&&((pFirstProvince+PR->arNeighbour[j])->ipTerrain==WATER_ID)&&(Port>0))
		{
			AttackNeighbour(Player,(pFirstProvince+PR->arNeighbour[j]),HR,Port-1);
		};
	};
};



//Проверка провинции для атаки
void TryAttack(int Player, int Prov, dcHero *HR)
{
	dcProvince *PR=(pFirstProvince+Prov);
	int Power;
	if	((Prov>0)&&(PR->ipPlayer!=Player)&&(PR->ipTerrain!=WATER_ID)&&(PR->ipTerrain!=MOUNTAIN_ID)&&(*(pMoveMap+Prov)>0))
	{
		Power=CompareStrength1(HR,PR->pGuard);
		if (PR->ipPlayer>0)
		{
			for (int i=0; i<MAX_CAMPERS; i++)
			{
				if (arPlayer[Player].arCamper[i]==Prov)
				{
					int Power1;
					for (int j=0; j<MAX_HERO; j++)
					{
						if ((arPlayer[PR->ipPlayer].arHero[j])&&(arPlayer[PR->ipPlayer].arHero[j]->iDead<0)&&(arPlayer[PR->ipPlayer].arHero[j]->ipProvince==Prov))
						{
							Power1=CompareStrength1(HR,arPlayer[PR->ipPlayer].arHero[j]);
							if (Power1<Power)
							{Power=Power1;};
						};
					};
				};
			};
			if (arPlayer[PR->ipPlayer].ipProvince==Prov)
			{
				if (Power>100)
				{
					arPlayer[Player].Politic[PR->ipPlayer].CanKill+=1000;
				}
				else
				{
					arPlayer[Player].Politic[PR->ipPlayer].CanKill+=Power/2;
				};
			}
			else
			{
				if (Power>100)
				{
					arPlayer[Player].Politic[PR->ipPlayer].CanKill+=GetMin(10,(Power-100)/10+1);
				};
			};
			if (arPlayer[Player].Politic[PR->ipPlayer].CanKill<0)
			{arPlayer[Player].Politic[PR->ipPlayer].CanKill=0;};
		}
		else
		{
			if (Power>=100)
			{bExpand=false;};
		};
	};
	if	((Prov>0)&&(PR->ipPlayer!=Player)&&(PR->ipTerrain!=WATER_ID)&&(PR->ipTerrain!=MOUNTAIN_ID)&&(*(pMoveMap+Prov)>0)&&
		((PR->ipPlayer==0)||(arPlayer[Player].Politic[PR->ipPlayer].Treaty==1)))
	{
//		int Power=CompareStrength(HR,PR->pGuard);
	//Проверка заключенного союза
		if (PR->ipPlayer==0)
		{
			if (arPlayer[Player].iUnion>0)
			{
				if (PR->ipRace==arPlayer[Player].iUnion)
				{Power=1000;};
			}
			else
			{
				if (CurGame.iCurrentTurn>=15-arPlayer[Player].iDifficulty)
				{
				//Проверка возможного союза
				if ((PR->ipRace>1)&&(arPlayer[Player].Control==2))
				{
					//Особый союз
					switch (arPlayer[Player].ipProfile)
					{
					case 4: 
						if (PR->ipRace==6)
						{
							arPlayer[Player].AIUnion[0]=6;
							arPlayer[Player].AIUnion[1]=0;
						};
						break;
					case 5: 
						if (PR->ipRace==2)
						{
							arPlayer[Player].AIUnion[0]=2;
							arPlayer[Player].AIUnion[1]=0;
						};
						break;
					case 6: 
						if (PR->ipRace==3)
						{
							arPlayer[Player].AIUnion[0]=3;
							arPlayer[Player].AIUnion[1]=0;
						};
						break;
					case 10: 
						if (PR->ipRace==7)
						{
							arPlayer[Player].AIUnion[0]=7;
							arPlayer[Player].AIUnion[1]=0;
						};
						break;
					case 11: 
						if (PR->ipRace==5)
						{
							arPlayer[Player].AIUnion[0]=5;
							arPlayer[Player].AIUnion[1]=0;
						};
						break;
					case 16: 
						if (PR->ipRace==4)
						{
							arPlayer[Player].AIUnion[0]=4;
							arPlayer[Player].AIUnion[1]=0;
						};
						break;
					};
					//Обычный союз
					if (arPlayer[Player].AIUnion[0]==0)
					{
						if (((arPlayer[Player].iKarma<0)&&((pFirstRace+PR->ipRace)->iKarma>0))||
							((arPlayer[Player].iKarma>0)&&((pFirstRace+PR->ipRace)->iKarma<0)))
						{}
						else
						{
							arPlayer[Player].AIUnion[0]=PR->ipRace;
							arPlayer[Player].AIUnion[1]=10-arPlayer[Player].iDifficulty+Random(10-arPlayer[Player].iDifficulty);
						};
					};
					if (arPlayer[Player].AIUnion[0]==PR->ipRace)
					{
						if (arPlayer[Player].AIUnion[1]==0)
						{Power=2000;}
						else
						{Power=-200;};
					};
				};
				};
			};			
			int N=0;
			while ((N<8)&&(NeutralTarget[HNum][N][0]>0))
			{N++;};
			if (N<MaxTargets)
			{
				NeutralTarget[HNum][N][0]=Prov;
				NeutralTarget[HNum][N][1]=*(pMoveMap+Prov);
				NeutralTarget[HNum][N][2]=Power;
			}
			else
			{
				int Select=-1;
				for (int i=0; i<MaxTargets; i++)
				{						
					if (Select<0)
					{
						if (NeutralTarget[HNum][i][2]<Power)
						{Select=i;}
						else
						{
							if ((NeutralTarget[HNum][i][2]==Power)&&(NeutralTarget[HNum][i][1]>*(pMoveMap+Prov)))
							{Select=i;};								
						};						
					}
					else
					{
						if (NeutralTarget[HNum][i][2]<NeutralTarget[HNum][Select][2])
						{Select=i;}
						else
						{
							if ((NeutralTarget[HNum][i][2]==NeutralTarget[HNum][Select][1])&&(NeutralTarget[HNum][i][1]>NeutralTarget[HNum][Select][2]))
							{Select=i;};								
						};
					};
				};
				if (Select>=0)
				{
					NeutralTarget[HNum][Select][0]=Prov;
					NeutralTarget[HNum][Select][1]=*(pMoveMap+Prov);
					NeutralTarget[HNum][Select][2]=Power;
				};
			};
		}
		else
		{
			int N=0;
			while (EnemyTarget[HNum][N][0]>0)
			{N++;};
			if (N<MaxTargets)
			{
				EnemyTarget[HNum][N][0]=Prov;
				EnemyTarget[HNum][N][1]=*(pMoveMap+Prov);
				EnemyTarget[HNum][N][2]=Power;
			}
			else
			{
				int Select=-1;
				for (int i=0; i<MaxTargets; i++)
				{						
					if (Select<0)
					{
						if ((arPlayer[(pFirstProvince+Prov)->ipPlayer].ipProvince==Prov)&&(arPlayer[(pFirstProvince+EnemyTarget[HNum][i][0])->ipPlayer].ipProvince!=EnemyTarget[HNum][i][0]))
						{Select=i;}
						else
						{
							if (EnemyTarget[HNum][i][2]<Power)
							{Select=i;}
							else
							{
								if ((EnemyTarget[HNum][i][2]==Power)&&(EnemyTarget[HNum][i][1]>*(pMoveMap+Prov)))
								{Select=i;};								
							};
						};
					}
					else
					{
						if (arPlayer[(pFirstProvince+EnemyTarget[HNum][i][0])->ipPlayer].ipProvince!=EnemyTarget[HNum][i][0])
						{
							if (EnemyTarget[HNum][i][2]<EnemyTarget[HNum][Select][2])
							{Select=i;}
							else
							{
								if ((EnemyTarget[HNum][i][2]==EnemyTarget[HNum][Select][1])&&(EnemyTarget[HNum][i][1]>EnemyTarget[HNum][Select][2]))
								{Select=i;};								
							};
						};
					};
				};
				if (Select>=0)
				{
					EnemyTarget[HNum][Select][0]=Prov;
					EnemyTarget[HNum][Select][1]=*(pMoveMap+Prov);
					EnemyTarget[HNum][Select][2]=Power;
				};
			};
		};
	};
};

int arCastles[MAX_HERO];

//***Выбор целей для героев
bool FillTarget(int Player, int Hero)
{
	bool bChange=false;
	char stLog[200];
	dcHero *HR=arPlayer[Player].arHero[Hero];
	HNum=Hero;
	ExploreTarget[Hero]=0;
	for (int i=0; i<MAX_HERO; i++)
	{		
		EnemyTarget[Hero][i][0]=0;
		NeutralTarget[Hero][i][0]=0;
		SiteTarget[Hero][i][0]=0;
		SiegeTarget[Hero][i][0]=0;
		RuinSiegeTarget[Hero][i][0]=0;
	};	

	ClearMoveMap();
	MovePlayer=Player;
	WalkType=1+HR->CheckSkill(26);
	WalkSpeed=HR->GetMobility();
	MoveMap(HR->ipProvince,-1,WalkSpeed,0,0);

	//***Проверка пути к родовой провинции, постройка верфи
	if ((HR->ipProvince!=arPlayer[Player].ipProvince)&&(HR->pProvince->ipPlayer==Player)&&(*(pMoveMap+arPlayer[Player].ipProvince)<0)&&(HR->pProvince->Coastal)&&(HR->pProvince->GetPort()==0))
	{
		int BldSlot=0;
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			if (HR->pProvince->arBuilding[i]==0)
			{				
				BldSlot=i;
				break;
			}
			else
			{
				if ((pFirstOuterBld+HR->pProvince->arBuilding[i])->iGoldCost+(pFirstOuterBld+HR->pProvince->arBuilding[i])->iGemCost*5/2<
					(pFirstOuterBld+HR->pProvince->arBuilding[BldSlot])->iGoldCost+(pFirstOuterBld+HR->pProvince->arBuilding[BldSlot])->iGemCost*5/2)
				{BldSlot=i;};
			};
		};
		HR->pProvince->arBuilding[BldSlot]=10;
		arPlayer[Player].iCanOuterBld--;
		HR->pProvince->DrawPic(false);
	};

	//***Отмена прежних заданий
	
	
	if (HR->iTask!=2)
	{HR->iAITask=0;};
	HR->iTask=0;

//Строим крепость и пополняем отряд, если далеко от замка и отряд побит
	int Count=0;
	for (int i=1; i<MAX_SQUAD; i++)
	{
		if (HR->pSquad[i])
		{Count++;};
	};
	int Slots=0;
	for (int i=1; i<=4; i++)
	{
		Slots+=HR->GetUnitSlot(i);
	};
	if ((Slots>Count*10/6)&&(arPlayer[Player].IsBestHero(Hero)))
	{bExpand=false;};

	if ((Slots>Count*10/6)&&(!HR->pProvince->pGarrison))	
	{
		int Target=0;
		for (int i=1; i<=MapSize; i++)
		{
			if (((pFirstProvince+i)->ipPlayer==Player)&&((pFirstProvince+i)->pGarrison)&&((pFirstProvince+i)->GetSiegeIndex()<0))
			{
				if (((Target==0)||(*(pMoveMap+Target)>*(pMoveMap+i)))&&(*(pMoveMap+i)>0))
				{Target=i;};
			};
		};
		if (((Target>0)&&(*(pMoveMap+Target)>3))||(Target==0))
		{
			if (TryBuildGarrison(Player,HR->ipProvince))
			{
				HireUnits(Player,Hero);
				bChange=true;
			};
		};
	};

	//Заполнение массива целей для помощи в осаде
	for (int HH=0; HH<MAX_HERO; HH++)
	{
		if ((HH!=Hero)&&(arPlayer[Player].arHero[HH])&&(arPlayer[Player].arHero[HH]->iDead<0)&&(arPlayer[Player].arHero[HH]->pProvince->ipPlayer!=Player)&&(TargetPower[HH]<100))
		{
			int Target=arPlayer[Player].arHero[HH]->ipProvince;
			if (*(pMoveMap+Target)>0)
			{
				int PrN=0;
				while (SiegeTarget[Hero][PrN][0]>0)
				{PrN++;};
				int Power=CompareStrength(HR,arPlayer[Player].arHero[HH]->pProvince->pGarrison);
				if (PrN<MaxTargets)
				{
					SiegeTarget[Hero][PrN][0]=Target;
					SiegeTarget[Hero][PrN][1]=*(pMoveMap+Target);
					SiegeTarget[Hero][PrN][2]=Power;
				}
				else
				{
					int Select=-1;
					for (int i=0; i<MaxTargets; i++)
					{						
						if (Select<0)
						{
							if ((arPlayer[(pFirstProvince+Target)->ipPlayer].ipProvince==Target)&&(arPlayer[(pFirstProvince+SiegeTarget[Hero][i][0])->ipPlayer].ipProvince!=SiegeTarget[Hero][i][0]))
							{Select=i;}
							else
							{
								if (SiegeTarget[Hero][i][1]>*(pMoveMap+Target))
								{Select=i;}
								else
								{
									if ((SiegeTarget[Hero][i][1]==*(pMoveMap+Target))&&(SiegeTarget[Hero][i][2]<Power))
									{Select=i;};								
								};
							};
						}
						else
						{
							if (arPlayer[(pFirstProvince+SiegeTarget[Hero][i][0])->ipPlayer].ipProvince!=SiegeTarget[Hero][i][0])
							{
								if (SiegeTarget[Hero][i][1]>SiegeTarget[Hero][Select][1])
								{Select=i;}
								else
								{
									if ((SiegeTarget[Hero][i][1]==SiegeTarget[Hero][Select][1])&&(SiegeTarget[Hero][i][2]<SiegeTarget[Hero][Select][2]))
									{Select=i;};								
								};
							};
						};
					};
					if (Select>=0)
					{
						SiegeTarget[Hero][Select][0]=Target;
						SiegeTarget[Hero][Select][1]=*(pMoveMap+Target);
						SiegeTarget[Hero][Select][2]=Power;
					};
				};
			};
		};
	};


//Заполнение массива целей для снятия осады
	for (int Target=1; Target<=MapSize; Target++)
	{
		if (((pFirstProvince+Target)->ipPlayer==Player)&&((pFirstProvince+Target)->GetSiegeIndex()>=0)&&(*(pMoveMap+Target)>0))
		{
			dcProvince *PR=(pFirstProvince+Target);
			int PrN=0;
			while (RuinSiegeTarget[Hero][PrN][0]>0)
			{PrN++;};
			int Power=1000;
			if (PR->GetSiegeIndex()==0)
			{
				Power=CompareStrength(HR,PR->pGuard);
			}
			else
			{
				for (int k=0; k<MAX_HERO; k++)
				{
					if ((arPlayer[PR->GetSiegeIndex()].arHero[k])&&(arPlayer[PR->GetSiegeIndex()].arHero[k]->pProvince==PR)&&(arPlayer[PR->GetSiegeIndex()].arHero[k]->iDead<0))
					{
						int Power1=CompareStrength(HR,arPlayer[PR->GetSiegeIndex()].arHero[k]);
						if (Power1<Power)
						{Power=Power1;};
					};
				};
			};
			if (PrN<MaxTargets)
			{
				RuinSiegeTarget[Hero][PrN][0]=Target;
				RuinSiegeTarget[Hero][PrN][1]=*(pMoveMap+Target);
				RuinSiegeTarget[Hero][PrN][2]=Power;
			}
			else
			{
				int Select=-1;
				for (int i=0; i<MaxTargets; i++)
				{						
					if (Select<0)
					{
						if (arPlayer[Player].ipProvince==Target)
						{Select=i;}
						else
						{
							if (RuinSiegeTarget[Hero][i][2]<Power)
							{Select=i;}
							else
							{
								if ((RuinSiegeTarget[Hero][i][1]>*(pMoveMap+Target))&&(RuinSiegeTarget[Hero][i][2]==Power))
								{Select=i;};								
							};
						};
					}
					else
					{
						if (arPlayer[Player].ipProvince!=RuinSiegeTarget[Hero][i][0])
						{
							if (RuinSiegeTarget[Hero][Select][2]>RuinSiegeTarget[Hero][i][2])
							{Select=i;}
							else
							{
								if ((RuinSiegeTarget[Hero][i][1]>RuinSiegeTarget[Hero][Select][1])&&(RuinSiegeTarget[Hero][i][2]==RuinSiegeTarget[Hero][Select][2]))
								{Select=i;};								
							};
						};
					};
				};
				if (Select>=0)
				{
					RuinSiegeTarget[Hero][Select][0]=Target;
					RuinSiegeTarget[Hero][Select][1]=*(pMoveMap+Target);
					RuinSiegeTarget[Hero][Select][2]=Power;
				};
			};
		};
	};

	//if ((TargetPower[Hero]>=100)&&(HR->iTask==1)&&(abs(HR->iAITask)==5))
	//{return bChange;};
	////Продолжаем осаду
	//if (HR->pProvince->ipPlayer!=Player)
	//{return bChange;};	
	
//Поиск провинции для атаки
	DFileLog("Поиск провинции для атаки");
	for (int i=1; i<=MapSize; i++)
	{
		TryAttack(Player,i,HR);		
	};

	//Поиск провинции для пополнения отряда
	DFileLog("Поиск провинции для пополнения");
	arCastles[Hero]=0;
	Count=0;
	for (int i=1; i<MAX_SQUAD; i++)
	{
		if (HR->pSquad[i])
		{Count++;};
	};
	Slots=0;
	for (int i=1; i<=4; i++)
	{
		Slots+=HR->GetUnitSlot(i);
	};
	if (Slots>Count)		
	{
		if (arPlayer[Player].Gold>50+CurGame.iCurrentTurn)
		{
		int Target=0;
		for (int i=1; i<=MapSize; i++)
		{
			if (((pFirstProvince+i)->ipPlayer==Player)&&((pFirstProvince+i)->pGarrison)&&((pFirstProvince+i)->GetSiegeIndex()<0))
			{
				if (((Target==0)||(*(pMoveMap+Target)>*(pMoveMap+i)))&&(*(pMoveMap+i)>0))
				{Target=i;};
			};
		};
		if (Target>0)
		{
			if (Slots>Count*10/6)
			{
			if (*(pMoveMap+Target)>3)
			{
				if (TryBuildGarrison(Player,HR->ipProvince))
				{
					HireUnits(Player,Hero);
					bChange=true;
				}
				else
				{
					HR->iTask=1;
					HR->iTarget=Target;
					HR->iAITask=1;
					return bChange;
				};
			}
			else
			{
				HR->iTask=1;
				HR->iTarget=Target;
				HR->iAITask=1;
				return bChange;
			};
			};
			arCastles[Hero]=Target;
		};
		}
		else
		{bCanSpend=false;};
	};
	//Поиск провинции для пополнения заклинаний
	if (arPlayer[Player].arLibrary[1]>0)
	{
		int Count=0;
		for (int i=1; i<SPELLBOOK_SIZE; i++)
		{
			if (HR->arSpell[i]>0)
			{Count++;};
		};
		int Slots=0;
		for (int i=1; i<=4; i++)
		{
			Slots+=HR->GetSpellSlot(i);
		};
		if (Slots>Count*10/7)	
		{
			int Target=0;
			for (int i=1; i<=MapSize; i++)
			{
				if (((pFirstProvince+i)->ipPlayer==Player)&&((pFirstProvince+i)->GetSiegeIndex()<0))
				{
					bool bLib=false;
					if (arPlayer[Player].ipProvince==i)
					{
						bLib=true;
					}
					else
					{
						for (int j=0; j<MAX_PROVINCE_BLD; j++)
						{
							if ((pFirstProvince+i)->arBuilding[j]>0)
							{					
								if ((pFirstOuterBld+(pFirstProvince+i)->arBuilding[j])->bLibrary)
								{
									bLib=true;
								};
							};
						};
					};						
					if (((Target==0)||(*(pMoveMap+Target)>*(pMoveMap+i)))&&(*(pMoveMap+i)>0)&&(bLib))
					{Target=i;};
				};
			};
			if (Target>0)
			{
				if (*(pMoveMap+Target)>3)
				{
					if (TryBuildLibrary(Player,HR->ipProvince))
					{
						LearnSpells(Player,Hero);
						bChange=true;
					}
					else
					{
						HR->iTask=1;
						HR->iTarget=Target;
						HR->iAITask=2;
						return bChange;
					};
				}
				else
				{
					HR->iTask=1;
					HR->iTarget=Target;
					HR->iAITask=2;
					return bChange;
				};
			};
		};
	};
	//Поиск провинции для обмена предметами
	if (arPlayer[Player].Shop.Items[1][0]>0)
	{
		int Count=0;
		for (int i=16; i<26; i++)
		{
			if (HR->arInventory[i][0]>0)
			{Count++;};
		};		
		if ((Count>7)||((HR->arInventory[1][0]>0)&&(HR->arInventory[1][1]<(pFirstItem+HR->arInventory[1][0])->iDurability/5)))
		{
			int Target=0;
			for (int i=1; i<=MapSize; i++)
			{
				if (((pFirstProvince+i)->ipPlayer==Player)&&((pFirstProvince+i)->GetSiegeIndex()<0))
				{
					bool bShop=false;
					if (arPlayer[Player].ipProvince==i)
					{
						bShop=true;
					}
					else
					{
						for (int j=0; j<MAX_PROVINCE_BLD; j++)
						{
							if ((pFirstProvince+i)->arBuilding[j]>0)
							{					
								if ((pFirstOuterBld+(pFirstProvince+i)->arBuilding[j])->bShop)
								{
									bShop=true;
								};
							};
						};
					};						
					if (((Target==0)||(*(pMoveMap+Target)>*(pMoveMap+i)))&&(*(pMoveMap+i)>0)&&(bShop))
					{Target=i;};
				};
			};
			if (Target>0)
			{				
				if (*(pMoveMap+Target)>3)
				{
					if (TryBuildShop(Player,HR->ipProvince))
					{
						EquipItems(Player,Hero);
						bChange=true;
					}
					else
					{
						HR->iTask=1;
						HR->iTarget=Target;
						HR->iAITask=3;
						return bChange;
					};
				}
				else
				{
					HR->iTask=1;
					HR->iTarget=Target;
					HR->iAITask=3;
					return bChange;
				};
			};
		};
	};
	//Поиск сайта для обыска
	DFileLog("Поиск сайта для обыска");
	for (int Prov=1; Prov<=MapSize; Prov++)
	{
		if (((pFirstProvince+Prov)->ipPlayer==Player)&&((pFirstProvince+Prov)->GetSiegeIndex()<0)&&(*(pMoveMap+Prov)>=0))
		{
			dcProvince *PR=(pFirstProvince+Prov);
			DFileLog(PR->sName);
			for (int j=0; j<MAX_PROVINCE_SITE; j++)
			{
				if ((PR->arSite[j][0]>0)&&(PR->arSite[j][1]<=PR->iExplore)&&(PR->arSiteGuard[j])&&(PR->arSite[j][5]!=-1)&&
					((Prov!=arPlayer[Player].ipProvince)||(j>0)||(arPlayer[Player].Control>2)))
				{

					DFileLog((pFirstSite+PR->arSite[j][0])->sName);
					int Power=CompareStrength(HR,PR->arSiteGuard[j]);
					sprintf(stLog,"Power: %d",Power);
					DFileLog(stLog);
					int N=0;
					while ((N<8)&&(SiteTarget[HNum][N][0]>0))
					{N++;};
					if (N<MaxTargets)
					{
						SiteTarget[HNum][N][0]=Prov;
						SiteTarget[HNum][N][1]=*(pMoveMap+Prov);
						SiteTarget[HNum][N][2]=Power;
						SiteTarget[HNum][N][3]=j;
					}
					else
					{
						int Select=-1;
						for (int i=0; i<MaxTargets; i++)
						{						
							if (Select<0)
							{
								if (SiteTarget[HNum][i][2]<Power)
								{Select=i;}
								else
								{
									if ((SiteTarget[HNum][i][2]==Power)&&(SiteTarget[HNum][i][1]>*(pMoveMap+Prov)))
									{Select=i;};								
								};						
							}
							else
							{
								if (SiteTarget[HNum][i][2]<SiteTarget[HNum][Select][2])
								{Select=i;}
								else
								{
									if ((SiteTarget[HNum][i][2]==SiteTarget[HNum][Select][1])&&(SiteTarget[HNum][i][1]>SiteTarget[HNum][Select][2]))
									{Select=i;};								
								};
							};
						};
						if (Select>=0)
						{
							SiteTarget[HNum][Select][0]=Prov;
							SiteTarget[HNum][Select][1]=*(pMoveMap+Prov);
							SiteTarget[HNum][Select][2]=Power;
							SiteTarget[HNum][Select][3]=j;
						};
					};
				};
				if ((PR->arSite[j][0]==41)&&(PR->arSite[j][1]<=PR->iExplore)&&(HR->iLevel<10)&&(arPlayer[Player].iDifficulty>1)&&(SiteTarget[HNum][0][0]==0))
				{
					SiteTarget[HNum][0][0]=Prov;
					SiteTarget[HNum][0][1]=*(pMoveMap+Prov);
					SiteTarget[HNum][0][2]=110-arPlayer[Player].arHero[Hero]->NiB;
					SiteTarget[HNum][0][3]=j;
				};
			};
		};
	};
		
	//Поиск провинции для исследований
	DFileLog("Поиск провинции для исследований");
	if (HR->pProvince->iExplore<100)
	{
		ExploreTarget[Hero]=HR->ipProvince;		
	}
	else
	{		
		for (int i=1; i<=MapSize; i++)
		{
			if (((pFirstProvince+i)->ipPlayer==Player)&&((pFirstProvince+i)->iExplore<100)&&((pFirstProvince+i)->GetSiegeIndex()<0))
			{
				if (((ExploreTarget[Hero]==0)||(*(pMoveMap+i)<*(pMoveMap+ExploreTarget[Hero])))&&(*(pMoveMap+i)>0))
				{
					ExploreTarget[Hero]=i;					
				};
			};				
		};
	};	
	return bChange;
};


//Вычисление силы противника героя
int CalcPower(int Player, dcHero *HR)
{
	int Result=1000;
	if (HR->pProvince->ipPlayer!=Player)
	{
		Result=CompareStrength1(HR,HR->pProvince->pGarrison);
	}
	else
	{
		if (HR->iTask==1)
		{
			if ((pFirstProvince+HR->iTarget)->ipPlayer!=Player)
			{
				Result=CompareStrength1(HR,(pFirstProvince+HR->iTarget)->pGuard);
			}
			else
			{
				if (HR->iAITask>=100)
				{
					Result=CompareStrength1(HR,(pFirstProvince+HR->iTarget)->arSiteGuard[HR->iAITask%100]);
				};
			};
		};
	};
	return Result;
};

//*****************************
//*** Выдача заданий героям ***
//*****************************
void HeroTaskmaster(int Player, int Progress)
{
	char stLog[200];
	MaxTargets=0;
	int Heroes=1;
	//Заполнение массивов целей и вычисление силы противника
	DFileLog("Заполнение массивов целей и вычисление силы противника");
	for (int i=0; i<MAX_HERO; i++)
	{
		TargetPower[i]=1000;
		if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0))
		{
			MaxTargets++;			
			TargetPower[i]=CalcPower(Player,arPlayer[Player].arHero[i]);
		};
	};
	if (MaxTargets>0)
	{
		Progress=Progress/MaxTargets;
		Heroes=MaxTargets;
	};
	if (MaxTargets<4)
	{MaxTargets=4;};
	for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0))
		{
			sprintf(stLog,"Герой: %s (%d)",arPlayer[Player].arHero[i]->pstName,i);
			DFileLog(stLog);
			if (FillTarget(Player,i))
			{
				TargetPower[i]=CalcPower(Player,arPlayer[Player].arHero[i]);
			};
			NewProgress+=Progress/Heroes*3/4;
		};
	};
	//Пересчёт сил целей по точному алгоритму, определение количества доступных целей
	DFileLog("Пересчёт сил целей по точному алгоритму, определение количества доступных целей");
	int arTask[MAX_HERO];
	for (int HR=0; HR<MAX_HERO; HR++)
	{
		arTask[HR]=0;		
		if ((arPlayer[Player].arHero[HR])&&(arPlayer[Player].arHero[HR]->iDead<0))
		{			
			for (int i=0; i<MaxTargets; i++)
			{
				if (SiegeTarget[HR][i][0]>0)
				{
					if (!bVerySmartAI)
					{SiegeTarget[HR][i][2]=CompareStrength1(arPlayer[Player].arHero[HR],(pFirstProvince+SiegeTarget[HR][i][0])->pGarrison);};
					arTask[HR]++;
				};
				if (RuinSiegeTarget[HR][i][0]>0)
				{
					if ((pFirstProvince+RuinSiegeTarget[HR][i][0])->GetSiegeIndex()==0)
					{RuinSiegeTarget[HR][i][2]=CompareStrength1(arPlayer[Player].arHero[HR],(pFirstProvince+RuinSiegeTarget[HR][i][0])->pGarrison);}
					else
					{
						RuinSiegeTarget[HR][i][2]=1000;
						for (int k=0; k<MAX_HERO; k++)
						{
							if ((arPlayer[(pFirstProvince+RuinSiegeTarget[HR][i][0])->GetSiegeIndex()].arHero[k])&&
								(arPlayer[(pFirstProvince+RuinSiegeTarget[HR][i][0])->GetSiegeIndex()].arHero[k]->pProvince==(pFirstProvince+RuinSiegeTarget[HR][i][0]))&&
								(arPlayer[(pFirstProvince+RuinSiegeTarget[HR][i][0])->GetSiegeIndex()].arHero[k]->iDead<0))
							{
								int Power1=CompareStrength1(arPlayer[Player].arHero[HR],arPlayer[(pFirstProvince+RuinSiegeTarget[HR][i][0])->GetSiegeIndex()].arHero[k]);
								if (Power1<RuinSiegeTarget[HR][i][2])
								{RuinSiegeTarget[HR][i][2]=Power1;};
							};
						};
					};
					if ((RuinSiegeTarget[HR][i][2]>=100)||(RuinSiegeTarget[HR][i][0]==arPlayer[Player].ipProvince))
					{arTask[HR]++;};
				};
				
				if (EnemyTarget[HR][i][0]>0)
				{
		//			if (!bVerySmartAI)
		//			{EnemyTarget[HR][i][2]=CompareStrength1(arPlayer[Player].arHero[HR],(pFirstProvince+EnemyTarget[HR][i][0])->pGuard);};
					if ((EnemyTarget[HR][i][2]>=100)||(arPlayer[(pFirstProvince+EnemyTarget[HR][i][0])->ipPlayer].ipProvince==EnemyTarget[HR][i][0]))
					{arTask[HR]++;};
				};
				if (NeutralTarget[HR][i][0]>0)
				{
		//			if ((!bVerySmartAI)&&(NeutralTarget[HR][i][2]<1000))
		//			{NeutralTarget[HR][i][2]=CompareStrength1(arPlayer[Player].arHero[HR],(pFirstProvince+NeutralTarget[HR][i][0])->pGuard);};
					if (NeutralTarget[HR][i][2]>=105-arPlayer[Player].arHero[HR]->NiB)
					{arTask[HR]++;};
				};
				
				if (SiteTarget[HR][i][0]>0)
				{
					if ((!bVerySmartAI)&&((pFirstProvince+SiteTarget[HR][i][0])->arSite[SiteTarget[HR][i][3]][0]!=41))
					{SiteTarget[HR][i][2]=CompareStrength1(arPlayer[Player].arHero[HR],(pFirstProvince+SiteTarget[HR][i][0])->arSiteGuard[SiteTarget[HR][i][3]]);};
					if (SiteTarget[HR][i][2]>=110-arPlayer[Player].arHero[HR]->NiB)
					{arTask[HR]++;};
				};
			};
			NewProgress+=Progress/Heroes/4;
		};
	};

	dcHero *Herr;
	//Первоочередные задания
	DFileLog("Первоочередные задания");
	for (int Select=0; Select<MAX_HERO; Select++)
	{
		if (arTask[Select]>0)
		{
			Herr=arPlayer[Player].arHero[Select];
			bool NeedTask=true;
			//Наш родовой замок осаждён
			for (int i=0; i<MaxTargets; i++)
			{
				if (RuinSiegeTarget[Select][i][0]==arPlayer[Player].ipProvince)
				{
					if ((pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()>0)
					{
						//Осаждать родовой замок врага если он ближе
						if ((Herr->pProvince->ipPlayer==(pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex())&&(Herr->ipProvince==arPlayer[(pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()].ipProvince))
						{
							Herr->iTask=0;
							NeedTask=false;
						};
						for (int j=0; j<MaxTargets; j++)
						{
							if (SiegeTarget[Select][j][0]==arPlayer[(pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()].ipProvince)
							{
								if (RuinSiegeTarget[Select][i][1]>SiegeTarget[Select][j][1])
								{
									Herr->iTask=1;
									Herr->iTarget=SiegeTarget[Select][j][0];
									Herr->iAITask=5;
									NeedTask=false;									
								};
							};
						};
					};
					if (NeedTask)
					{
						Herr->iTask=1;
						Herr->iTarget=RuinSiegeTarget[Select][i][0];
						Herr->iAITask=-5;
						NeedTask=false;
						//Ускориться ритуалом, если сильней врага и до замка далеко
						if ((RuinSiegeTarget[Select][i][2]>=100)&&(RuinSiegeTarget[Select][i][1]>2))
						{
							if ((RuinSiegeTarget[Select][i][1]>4)&&(IsRitualAllowed(Player,20)))
							{arPlayer[Player].CastRitual(20,Herr->ipProvince,Select);}
							else
							{
								if (IsRitualAllowed(Player,4))
								{arPlayer[Player].CastRitual(4,Herr->ipProvince,Select);}
							};
						};
					};
				};
			};
			
			//Родовой замок врага осаждён
			//Этим героем
			if (NeedTask)
			{
				if ((Herr->pProvince->ipPlayer!=Player)&&(arPlayer[Herr->pProvince->ipPlayer].ipProvince==Herr->ipProvince))
				{
					Herr->iTask=0;
					NeedTask=false;
				};
			};

			//Не найден путь к осаждённому родовому замку, использовать ритуал
			if ((NeedTask)&&((pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()>=0)&&(IsRitualAllowed(Player,20)))
			{
				dcProvince *PR=(pFirstProvince+arPlayer[Player].ipProvince);
				bool bGate=false;
				for (int i=0; i<MAX_NEIGHBOUR; i++)
				{
					if ((PR->arNeighbour[i]>0)&&((pFirstProvince+PR->arNeighbour[i])->ipPlayer==Player)&&((pFirstProvince+PR->arNeighbour[i])->GetSiegeIndex()<0))
					{bGate=true;};
				};
				if (bGate)
				{
					arPlayer[Player].CastRitual(20,Herr->ipProvince,Select);
					Herr->iTask=1;
					Herr->iTarget=arPlayer[Player].ipProvince;
					Herr->iAITask=-5;
					NeedTask=false;
				};
			};

			//Помощь в осаде
			if (NeedTask)
			{
				for (int i=0; i<MaxTargets; i++)
				{
					int Dist=1000;
					if ((SiegeTarget[Select][i][0]>0)&&(SiegeTarget[Select][i][0]==arPlayer[(pFirstProvince+SiegeTarget[Select][i][0])->ipPlayer].ipProvince))
					{
						//Идти за войском, если отряд слабый
						if ((SiegeTarget[Select][i][2]<100)&&(Herr->iTask==1)&&(Herr->iAITask==1))
						{NeedTask=false;}
						else
						{
							if (SiegeTarget[Select][i][1]<Dist)
							{							
								Herr->iTask=1;
								Herr->iTarget=SiegeTarget[Select][i][0];
								Herr->iAITask=5;
								Dist=SiegeTarget[Select][i][1];
								NeedTask=false;
							};							
						};
					};
				};
			};

			//Родовой замок врага найден
			if (NeedTask)
			{
				for (int i=0; i<MaxTargets; i++)
				{
					int Power=0;
					if ((EnemyTarget[Select][i][0]>0)&&(EnemyTarget[Select][i][0]==arPlayer[(pFirstProvince+EnemyTarget[Select][i][0])->ipPlayer].ipProvince))
					{
						//Идти за войском, если отряд слабый
						if ((EnemyTarget[Select][i][2]<100)&&(Herr->iTask==1)&&(Herr->iAITask==1))
						{NeedTask=false;}
						else
						{
							int Count=0;
							for (int j=1; j<MAX_SQUAD; j++)
							{
								if (Herr->pSquad[j])
								{Count++;};
							};
							int Slots=0;
							for (int j=1; j<=4; j++)
							{
								Slots+=Herr->GetUnitSlot(j);
							};
							if ((EnemyTarget[Select][i][2]>50)&&((EnemyTarget[Select][i][2]>=100)||(Slots<=Count*10/6)))
							{							
								Herr->iTask=1;
								Herr->iTarget=EnemyTarget[Select][i][0];
								Herr->iAITask=0;
								Power=SiegeTarget[Select][i][2];
								NeedTask=false;
								if ((EnemyTarget[Select][i][2]>=100)&&(EnemyTarget[Select][i][1]>2)&&(IsRitualAllowed(Player,4)))
								{arPlayer[Player].CastRitual(4,Herr->ipProvince,Select);};
								//Ослабить защитника ритуалом
								if ((EnemyTarget[Select][i][2]<150)&&(EnemyTarget[Select][i][1]==1))
								{DestructiveRitual(EnemyTarget[Select][i][0],Player,true);};
							};							
						};
					};
				};
			};

			//Идти за войском
			if ((NeedTask)&&(Herr->iTask==1)&&(Herr->iAITask==1))
			{NeedTask=false;};

			//Продолжать осаду
			if ((NeedTask)&&(Herr->pProvince->ipPlayer!=Player))
			{NeedTask=false;};

			if (!NeedTask)
			{arTask[Select]=0;};
		};
	};

	//***Выбор задания***
	DFileLog("Помощь в осаде");
	//Помощь в осаде
	int SelectHero=0, SelectTarget;
	while (SelectHero>=0)
	{
		SelectHero=-1;
		int Dist=1000; int Power=0;
		for (int HR=0; HR<MAX_HERO; HR++)
		{
			if (arTask[HR]>0)
			{
				for (int i=0; i<MaxTargets; i++)
				{
					if (SiegeTarget[HR][i][0]>0)
					{
						if (SiegeTarget[HR][i][2]>=100)
						{
							if (SiegeTarget[HR][i][1]<Dist)
							{
								SelectHero=HR;
								SelectTarget=i;
								Dist=SiegeTarget[HR][i][1];
								Power=SiegeTarget[HR][i][2];
							};
						}
						else
						{
							if ((Power<100)&&(SiegeTarget[HR][i][2]>Power))
							{
								SelectHero=HR;
								SelectTarget=i;
								Dist=SiegeTarget[HR][i][1];
								Power=SiegeTarget[HR][i][2];
							};
						};
					};
				};
			};
		};
		if (SelectHero>=0)
		{
			arPlayer[Player].arHero[SelectHero]->iTask=1;
			arPlayer[Player].arHero[SelectHero]->iAITask=5;
			arPlayer[Player].arHero[SelectHero]->iTarget=SiegeTarget[SelectHero][SelectTarget][0];			
			for (int HR=0; HR<MAX_HERO; HR++)
			{
				if (arTask[HR]>0)
				{
					for (int i=0; i<MaxTargets; i++)
					{
						if (SiegeTarget[HR][i][0]==arPlayer[Player].arHero[SelectHero]->iTarget)
						{SiegeTarget[HR][i][0]=0;};
					};
				};
			};
			arTask[SelectHero]=0;
		};
	};

	//Снятие осады
	DFileLog("Снятие осады");
	SelectHero=0;
	while (SelectHero>=0)
	{
		SelectHero=-1;
		int Dist=1000; int Power=0;
		for (int HR=0; HR<MAX_HERO; HR++)
		{
			if (arTask[HR]>0)
			{
				for (int i=0; i<MaxTargets; i++)
				{
					if (RuinSiegeTarget[HR][i][0]>0)
					{
						if (RuinSiegeTarget[HR][i][2]>=100)
						{
							if (RuinSiegeTarget[HR][i][1]<Dist)
							{
								SelectHero=HR;
								SelectTarget=i;
								Dist=RuinSiegeTarget[HR][i][1];
								Power=RuinSiegeTarget[HR][i][2];
							};
						}
						else
						{
							if ((Power<100)&&(RuinSiegeTarget[HR][i][2]>Power))
							{
								SelectHero=HR;
								SelectTarget=i;
								Dist=RuinSiegeTarget[HR][i][1];
								Power=RuinSiegeTarget[HR][i][2];
							};
						};
					};
				};
			};
		};
		if (SelectHero>=0)
		{
			arPlayer[Player].arHero[SelectHero]->iTask=1;
			arPlayer[Player].arHero[SelectHero]->iAITask=-5;
			arPlayer[Player].arHero[SelectHero]->iTarget=RuinSiegeTarget[SelectHero][SelectTarget][0];			
			for (int HR=0; HR<MAX_HERO; HR++)
			{
				if (arTask[HR]>0)
				{
					for (int i=0; i<MaxTargets; i++)
					{
						if (RuinSiegeTarget[HR][i][0]==arPlayer[Player].arHero[SelectHero]->iTarget)
						{RuinSiegeTarget[HR][i][0]=0;};
					};
				};
			};
			arTask[SelectHero]=0;
			if ((RuinSiegeTarget[SelectHero][SelectTarget][1]>3)&&(IsRitualCheap(Player,4)))
			{arPlayer[Player].CastRitual(4,arPlayer[Player].arHero[SelectHero]->ipProvince,SelectHero);};
		};
	};


	//Атака вражеской провинции
	DFileLog("Атака вражеской провинции");
	SelectHero=0;
	while (SelectHero>=0)
	{
		SelectHero=-1;
		int Dist=1000;
		for (int HR=0; HR<MAX_HERO; HR++)
		{
			if (arTask[HR]>0)
			{
				for (int i=0; i<MaxTargets; i++)
				{
					if (EnemyTarget[HR][i][0]>0)
					{
						if (EnemyTarget[HR][i][2]>=100)
						{
							if (EnemyTarget[HR][i][1]<Dist)
							{
								SelectHero=HR;
								SelectTarget=i;
								Dist=EnemyTarget[HR][i][1];								
							};
						};
					};
				};
			};
		};
		if (SelectHero>=0)
		{
			arPlayer[Player].arHero[SelectHero]->iTask=1;
			arPlayer[Player].arHero[SelectHero]->iAITask=0;
			arPlayer[Player].arHero[SelectHero]->iTarget=EnemyTarget[SelectHero][SelectTarget][0];
			//Ускориться ритуалом
			if ((EnemyTarget[SelectHero][SelectTarget][1]>3)&&(IsRitualCheap(Player,4)))
			{arPlayer[Player].CastRitual(4,arPlayer[Player].arHero[SelectHero]->ipProvince,SelectHero);};
			//Ослабить защитника ритуалом
			if ((EnemyTarget[SelectHero][SelectTarget][2]<150)&&(EnemyTarget[SelectHero][SelectTarget][1]==1))
			{DestructiveRitual(EnemyTarget[SelectHero][SelectTarget][0],Player,false);};
			//Отменить данную провинцию из списка у остальных героев
			for (int HR=0; HR<MAX_HERO; HR++)
			{
				if (arTask[HR]>0)
				{
					for (int i=0; i<MaxTargets; i++)
					{
						if (EnemyTarget[HR][i][0]==arPlayer[Player].arHero[SelectHero]->iTarget)
						{EnemyTarget[HR][i][0]=0;};
					};
				};
			};			
			arTask[SelectHero]=0;			
		};
	};

	//Атака нейтральной провинции
	DFileLog("Атака нейтральной провинции");
	SelectHero=0;
	while (SelectHero>=0)
	{
		SelectHero=-1;
		int Dist=1000;
		for (int HR=0; HR<MAX_HERO; HR++)
		{
			if (arTask[HR]>0)
			{
				for (int i=0; i<MaxTargets; i++)
				{
					if (NeutralTarget[HR][i][0]>0)
					{
						if (NeutralTarget[HR][i][2]>=105-arPlayer[Player].arHero[HR]->NiB)
						{
							if (NeutralTarget[HR][i][1]<Dist)
							{
								SelectHero=HR;
								SelectTarget=i;
								Dist=NeutralTarget[HR][i][1];								
							};
						};
					};
				};
			};
		};
		if (SelectHero>=0)
		{
			arPlayer[Player].arHero[SelectHero]->iTask=1;
			arPlayer[Player].arHero[SelectHero]->iAITask=0;
			arPlayer[Player].arHero[SelectHero]->iTarget=NeutralTarget[SelectHero][SelectTarget][0];			
			for (int HR=0; HR<MAX_HERO; HR++)
			{
				if (arTask[HR]>0)
				{
					for (int i=0; i<MaxTargets; i++)
					{
						if (NeutralTarget[HR][i][0]==arPlayer[Player].arHero[SelectHero]->iTarget)
						{NeutralTarget[HR][i][0]=0;};
					};
				};
			};
			arTask[SelectHero]=0;
		};
	};

	//Идти за заклинаниями или предметами
	DFileLog("Идти за заклинаниями или предметами");
	for (int HR=0; HR<MAX_HERO; HR++)
	{
		if ((arTask[HR]>0)&&(arPlayer[Player].arHero[HR]->iTask==1)&&((arPlayer[Player].arHero[HR]->iAITask==2)||(arPlayer[Player].arHero[HR]->iAITask==3)))
		{arTask[HR]=0;};
	//Идти за войском при недостаче нескольких воинов
		if ((arPlayer[Player].arHero[HR])&&(arPlayer[Player].arHero[HR]->iDead<0)&&(arPlayer[Player].arHero[HR]->pProvince->ipPlayer==Player)&&(arPlayer[Player].arHero[HR]->iTask==0)&&(arTask[HR]==0)&&(arCastles[HR]>0)&&(arCastles[HR]!=arPlayer[Player].arHero[HR]->ipProvince))
		{
			arPlayer[Player].arHero[HR]->iTask=1;
			arPlayer[Player].arHero[HR]->iAITask=1;
			arPlayer[Player].arHero[HR]->iTarget=arCastles[HR];
		};
	};

	//Атака сайта
	DFileLog("Атака сайта");
	SelectHero=0;
	while (SelectHero>=0)
	{
		SelectHero=-1;
		int Dist=1000;
		for (int HR=0; HR<MAX_HERO; HR++)
		{
			if (arTask[HR]>0)
			{
				for (int i=0; i<MaxTargets; i++)
				{
					if (SiteTarget[HR][i][0]>0)
					{
						if (SiteTarget[HR][i][2]>=110-arPlayer[Player].arHero[HR]->NiB)
						{
							if (SiteTarget[HR][i][1]<Dist)
							{
								SelectHero=HR;
								SelectTarget=i;
								Dist=SiteTarget[HR][i][1];								
							};
						};
					};
				};
			};
		};
		if (SelectHero>=0)
		{
			if (arPlayer[Player].arHero[SelectHero]->ipProvince==SiteTarget[SelectHero][SelectTarget][0])
			{
				arPlayer[Player].arHero[SelectHero]->iTask=4;
				arPlayer[Player].arHero[SelectHero]->iAITask=100+SiteTarget[SelectHero][SelectTarget][3];
				arPlayer[Player].arHero[SelectHero]->iTarget=SiteTarget[SelectHero][SelectTarget][3];
			}
			else
			{
				arPlayer[Player].arHero[SelectHero]->iTask=1;
				arPlayer[Player].arHero[SelectHero]->iAITask=100+SiteTarget[SelectHero][SelectTarget][3];
				arPlayer[Player].arHero[SelectHero]->iTarget=SiteTarget[SelectHero][SelectTarget][0];
			};
			int Targ=SiteTarget[SelectHero][SelectTarget][0];
			if ((pFirstProvince+Targ)->arSite[SiteTarget[SelectHero][SelectTarget][3]][0]!=41)
			{
				for (int HR=0; HR<MAX_HERO; HR++)
				{
					if (arTask[HR]>0)
					{
						for (int i=0; i<MaxTargets; i++)
						{
							if ((SiteTarget[HR][i][0]==Targ)&&(SiteTarget[HR][i][3]==SiteTarget[SelectHero][SelectTarget][3]))
							{SiteTarget[HR][i][0]=0;};
							};
					};
				};
			};
			arTask[SelectHero]=0;
		};
	};

	//Исследовать провинцию, если нет других заданий и целей
	DFileLog("Исследовать провинцию, если нет других заданий и целей");
	for (int HR=0; HR<MAX_HERO; HR++)
	{
		if ((arPlayer[Player].arHero[HR])&&(arPlayer[Player].arHero[HR]->iDead<0))
		{
			if ((arPlayer[Player].arHero[HR]->pProvince->ipPlayer==Player)&&(arPlayer[Player].arHero[HR]->iTask==0))
			{
				if (ExploreTarget[HR]>0)
				{
					if (arPlayer[Player].arHero[HR]->ipProvince==ExploreTarget[HR])
					{
						arPlayer[Player].arHero[HR]->iTask=2;						
						arPlayer[Player].arHero[HR]->iTarget=ExploreTarget[HR];
						arPlayer[Player].arHero[HR]->iAITask++;
						if (arPlayer[Player].arHero[HR]->iAITask>=5)
						{KillWeak(arPlayer[Player].arHero[HR]);};
					}
					else
					{
						arPlayer[Player].arHero[HR]->iTask=1;
						arPlayer[Player].arHero[HR]->iAITask=4;
						arPlayer[Player].arHero[HR]->iTarget=ExploreTarget[HR];
					};

				};
			};
		};
	};
};

//Попытка установить стража в указанной провинции
void AITryDefender(int Province, bool bBigGuy)
{
	dcProvince *PR=(pFirstProvince+Province);
	int Player=PR->ipPlayer;
	int Taste=0, Choosen=0;
	bool bHire=false;
	if (arPlayer[Player].iDifficulty==0)
	{bBigGuy=false;};
	if (PR->pGuard)
	{Taste=GetDefenderPower(PR->iDefender,PR->pGuard);};
	for (int i=1; i<=iMaxDefender; i++)
	{
		if (((*(arPlayer[Player].pDefender+i)<0)||((*(arPlayer[Player].pDefender+i)>0)&&((bBigGuy)||((pFirstDefender+i)->iPower<3))&&((CurGame.iGameType!=2)||((pFirstDefender+i)->iPower<4+Campaign.iCurrentTurn/10))))&&
			(GetDefenderPower(i,NULL)>Taste+Random(Taste/2))&&
			(PR->CanHire(i))&&
			(arPlayer[Player].Gold>=GoldLimit+arPlayer[Player].GetDefenderGoldPrice(i))&&
			((arPlayer[Player].GetDefenderGemPrice(i)==0)||(arPlayer[Player].Gem>=GemLimit+arPlayer[Player].GetDefenderGemPrice(i))))
		{
			Taste=GetDefenderPower(i,NULL);
			bHire=true;
		};
	};
	if (bHire)
	{
	int Count=0;
	ClearRoll();
	for (int i=1; i<=iMaxDefender; i++)
	{
		if (((*(arPlayer[Player].pDefender+i)<0)||((*(arPlayer[Player].pDefender+i)>0)&&((bBigGuy)||((pFirstDefender+i)->iPower<3))))&&
			(GetDefenderPower(i,NULL)==Taste)&&
			(PR->CanHire(i))&&
			(arPlayer[Player].Gold>=GoldLimit+arPlayer[Player].GetDefenderGoldPrice(i))&&
			((arPlayer[Player].GetDefenderGemPrice(i)==0)||(arPlayer[Player].Gem>=GemLimit+arPlayer[Player].GetDefenderGemPrice(i))))
		{
			RollerN[Count]=i;
			RollerP[Count]=GetDefenderPower(i);
			//Уменьшаем вероятность появления одинаковых стражей
			int Used=0;
			if (*(arPlayer[Player].pDefender+i)<0)
			{
				for (int PR=1; PR<=MapSize; PR++)
				{
					if (((pFirstProvince+PR)->ipPlayer==Player)&&((pFirstProvince+PR)->iDefender==i))
					{Used++;};
				};
			};
			if (Used>0)
			{
				RollerP[Count]/=Used;
				if (RollerP[Count]<1)
				{RollerP[Count]=1;};
			};

			Count++;
		};
	};
	if (Count>0)
	{Choosen=Roll(false);};

	/*for (int i=1; i<=iMaxDefender; i++)
	{
		if (((*(arPlayer[Player].pDefender+i)<0)||((*(arPlayer[Player].pDefender+i)>0)&&(bBigGuy)))&&
			((!PR->pGuard)||(GetDefenderPower(i)+Random(100,i,Province)>GetDefenderPower(PR->pGuard)+50+Random(100,PR->iDefender,Province)))&&
			(PR->CanHire(i))&&
			(arPlayer[Player].Gold>=arPlayer[Player].GetDefenderGoldPrice(i))&&
			(arPlayer[Player].Gem>=arPlayer[Player].GetDefenderGemPrice(i)))
		{
			int NewTaste;
			if (PR->iDefender==0)
			{NewTaste=GetDefenderPower(i)+Random(100,i,Province);}
			else
			{NewTaste=GetDefenderPower(i)-GetDefenderPower(PR->iDefender);};
			if (NewTaste>Taste)
			{
				Taste=NewTaste;
				Choosen=i;
			};
		};
	};*/
	if (Choosen>0)
	{
		arPlayer[Player].Gold-=arPlayer[Player].GetDefenderGoldPrice(Choosen);
		arPlayer[Player].Gem-=arPlayer[Player].GetDefenderGemPrice(Choosen);
		for (int i=0; i<MAX_DEFENDER_RES; i++)
		{
			if (((pFirstDefender+Choosen)->arRes[i]>0)&&
			(!arPlayer[Player].GetResource((pFirstDefender+Choosen)->arRes[i])))
			{
				CurGame.ResPrices[(pFirstDefender+Choosen)->arRes[i]]+=CurGame.ResBuy;
			};
		};
		if (*(arPlayer[Player].pDefender+Choosen)>0)
		{
			*(arPlayer[Player].pDefender+Choosen)=*(arPlayer[Player].pDefender+Choosen)-1;
		};
		if (PR->iDefender>0)
		{					
			arPlayer[Player].iKarma-=(pFirstDefender+PR->iDefender)->iKarma;
		};
		arPlayer[Player].iKarma+=(pFirstDefender+Choosen)->iKarma;				
		arPlayer[Player].iCanHire--;
		PR->PlaceDefender(Choosen);		
		char Mess[500];
		sprintf(Mess,"%s нанял стража %s в провинции %s",arPlayer[Player].GetName(),(pFirstDefender+Choosen)->sName,(pFirstProvince+Province)->sName);
		FileLog(Mess);
	};
	};
};


//****Расстановка стражей
void AIPlaceDefender(int Player)
{
	AITryDefender(arPlayer[Player].ipProvince,true);
	if (arPlayer[Player].iCanHire<=0)
	{return;}
	/*for (int j=0; j<MAX_NEIGHBOUR; j++)
	{
		if (((pFirstProvince+arPlayer[Player].ipProvince)->arNeighbour[j]>0)&&((pFirstProvince+(pFirstProvince+arPlayer[Player].ipProvince)->arNeighbour[j])->ipPlayer>0)&&((pFirstProvince+(pFirstProvince+arPlayer[Player].ipProvince)->arNeighbour[j])->ipPlayer!=Player))
		{
			AITryDefender(arPlayer[Player].ipProvince,true);
			if (arPlayer[Player].iCanHire<=0)
			{return;};
		};
	};*/
	for (int Count=0; Count<2+Random(3); Count++)
	{
	for (int i=1; i<=MapSize; i++)
	{
		if ((pFirstProvince+i)->ipPlayer==Player)
		{
			if (Count==3)
			{
				AITryDefender(i,false);
				if (arPlayer[Player].iCanHire<=0)
				{return;}
			}
			else
			{
				if (Count==2)
				{
					if ((pFirstProvince+i)->pGuard==NULL)
					{
						AITryDefender(i,false);
						if (arPlayer[Player].iCanHire<=0)
						{return;}
					};
				}
				else
				{
					if (Count==1)
					{						
						for (int j=0; j<MAX_NEIGHBOUR; j++)
						{
							if (((pFirstProvince+i)->arNeighbour[j]>0)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipPlayer>0)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipPlayer!=Player))
							{
								AITryDefender(i,true);
								if (arPlayer[Player].iCanHire<=0)
								{return;}
								break;
							};
						};
					}
					else
					{
						if ((pFirstProvince+i)->pGuard==NULL)
						{
							for (int j=0; j<MAX_NEIGHBOUR; j++)
							{
								if (((pFirstProvince+i)->arNeighbour[j]>0)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipPlayer>0)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipPlayer!=Player))
								{
									AITryDefender(i,true);
									if (arPlayer[Player].iCanHire<=0)
									{return;}
									break;
								};
							};
						};
					};
				};
			};
		};
	};
	};
};

//****Отстройка внешних зданий 
void AIOuterBuild(int Player)
{
	//Постройка верфи для героя на острове
	bool bIsland=true;
	for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0)&&(arPlayer[Player].arHero[i]->pProvince->Coastal)&&(arPlayer[Player].arHero[i]->pProvince->GetPort()==0)&&(arPlayer[Player].arHero[i]->pProvince->iNumber!=arPlayer[Player].ipProvince)&&(arPlayer[Player].arHero[i]->pProvince->ipPlayer==Player))
		{
			bIsland=true;
			for (int j=0; j<MAX_NEIGHBOUR; j++)
			{
				if ((arPlayer[Player].arHero[i]->pProvince->arNeighbour[j]>0)&&((pFirstProvince+arPlayer[Player].arHero[i]->pProvince->arNeighbour[j])->ipPlayer==Player))
				{bIsland=false;};
			};
			if (bIsland)
			{
				arPlayer[Player].arBld[52]=true;
				int Bld=0;
				for (int j=0; j<MAX_PROVINCE_BLD; j++)
				{
					if (arPlayer[Player].arHero[i]->pProvince->arBuilding[j]>0)
					{Bld++;};
				};
				if (Bld==MAX_PROVINCE_BLD)
				{
					int Price=10000, Kill=0;
					for (int j=0; j<MAX_PROVINCE_BLD; j++)
					{
						if ((pFirstOuterBld+arPlayer[Player].arHero[i]->pProvince->arBuilding[j])->iGoldCost+(pFirstOuterBld+arPlayer[Player].arHero[i]->pProvince->arBuilding[j])->iGemCost*3<Price)
						{
							Price=(pFirstOuterBld+arPlayer[Player].arHero[i]->pProvince->arBuilding[j])->iGoldCost+(pFirstOuterBld+arPlayer[Player].arHero[i]->pProvince->arBuilding[j])->iGemCost*3;
							Kill=j;
						};
					};
					arPlayer[Player].arHero[i]->pProvince->RazeBuilding(Kill, false);
				};
				arPlayer[Player].arHero[i]->pProvince->ConstructBuilding(10,false, true);
			};
		};
	};
	//Постройка верфи если соседи только море
	bIsland=true;
	int FreeProv=0, OccProv=0;
	for (int i=1; i<=MapSize; i++)
	{
		if (((pFirstProvince+i)->ipPlayer==Player)&&((pFirstProvince+i)->GetPort()==0))
		{			
			for (int j=0; j<MAX_NEIGHBOUR; j++)
			{
				if (((pFirstProvince+i)->arNeighbour[j]>0)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipTerrain!=WATER_ID)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipTerrain!=MOUNTAIN_ID)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipPlayer!=Player))
				{bIsland=false;};
				if (((pFirstProvince+i)->arNeighbour[j]>0)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipTerrain==WATER_ID)&&(FreeProv==0))
				{
					for (int k=0; k<MAX_NEIGHBOUR; k++)
					{
						if ((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->arNeighbour[k]>0)
						{
							dcProvince *PR=(pFirstProvince+(pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->arNeighbour[k]);
							if ((PR->ipPlayer>0)&&(PR->ipPlayer!=Player)&&(arPlayer[Player].Politic[PR->ipPlayer].CanKill<0))
							{
								OccProv=i;
							};
						};
					};
				};
			};
			if (OccProv==i)
			{
				int Bld=0;
				for (int j=0; j<MAX_PROVINCE_BLD; j++)
				{
					if ((pFirstProvince+i)->arBuilding[j]>0)
					{Bld++;};
				};
				if (Bld<MAX_PROVINCE_BLD)
				{
					FreeProv=OccProv;
					OccProv=0;
				};
			};
		};
	};
	if (bIsland)
	{
		arPlayer[Player].arBld[52]=true;
		int Water=0, MaxWater=0, Pick=0, Bld, BldPick;
		if (OccProv>0)
		{
			BldPick=MAX_PROVINCE_BLD;
			Pick=OccProv;
			OccProv=0;
		};
		if (FreeProv>0)
		{
			BldPick=0;
			Pick=FreeProv;
			FreeProv=0;
		};
		if (Pick==0)
		{
		for (int i=1; i<=MapSize; i++)
		{
			if (((pFirstProvince+i)->ipPlayer==Player)&&((pFirstProvince+i)->Coastal)&&((pFirstProvince+i)->GetPort()==0)&&(arPlayer[Player].ipProvince!=i))
			{
				Water=0;
				for (int j=0; j<MAX_NEIGHBOUR; j++)
				{
					if (((pFirstProvince+i)->arNeighbour[j]>0)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipTerrain==WATER_ID))
					{Water++;};
				};
				Bld=0;
				for (int j=0; j<MAX_PROVINCE_BLD; j++)
				{
					if ((pFirstProvince+i)->arBuilding[j]>0)
					{Bld++;};
				};
				if ((Pick==0)||(Water>MaxWater)||((Water==MaxWater)&&(Bld<BldPick)))
				{
					Pick=i;
					MaxWater=Water;
					BldPick=Bld;
				};
			};
		};
		};
		if (Pick>0)
		{
			if (BldPick==MAX_PROVINCE_BLD)
			{
				int Price=10000, Kill=0;
				for (int j=0; j<MAX_PROVINCE_BLD; j++)
				{
					if ((pFirstOuterBld+(pFirstProvince+Pick)->arBuilding[j])->iGoldCost+(pFirstOuterBld+(pFirstProvince+Pick)->arBuilding[j])->iGemCost*3<Price)
					{
						Price=(pFirstOuterBld+(pFirstProvince+Pick)->arBuilding[j])->iGoldCost+(pFirstOuterBld+(pFirstProvince+Pick)->arBuilding[j])->iGemCost*3;
						Kill=j;
					};
				};
				(pFirstProvince+Pick)->RazeBuilding(Kill, false);
			};
			(pFirstProvince+Pick)->ConstructBuilding(10,false, true);
		};
	};
	//Постройка верфи, если найден сосед на берегу
	if ((arPlayer[Player].arBld[52])&&(arPlayer[Player].iCanOuterBld>0))
	{
		if (FreeProv>0)
		{(pFirstProvince+FreeProv)->ConstructBuilding(10,false, true);}
		else
		{
			if (OccProv>0)
			{
				int Price=10000, Kill=0;
				for (int j=0; j<MAX_PROVINCE_BLD; j++)
				{
					if ((pFirstOuterBld+(pFirstProvince+OccProv)->arBuilding[j])->iGoldCost+(pFirstOuterBld+(pFirstProvince+OccProv)->arBuilding[j])->iGemCost*3<Price)
					{
						Price=(pFirstOuterBld+(pFirstProvince+OccProv)->arBuilding[j])->iGoldCost+(pFirstOuterBld+(pFirstProvince+OccProv)->arBuilding[j])->iGemCost*3;
						Kill=j;
					};
				};
				(pFirstProvince+OccProv)->RazeBuilding(Kill, false);
				(pFirstProvince+OccProv)->ConstructBuilding(10,false, true);
			};
		};
	};

	//Выбор постройки из общего списка
	bool bProceed=true;
	while ((arPlayer[Player].iCanOuterBld>0)&&(bProceed))
	{
	int Build=0, Province=0, Taste=0;
	for (int i=1; i<MapSize; i++)
	{		
		if (((pFirstProvince+i)->ipPlayer==Player)&&(arPlayer[Player].ipProvince!=i))
		{
			for (int j=1; j<iMaxOuterBld; j++)
			{
				if ((*(arPlayer[Player].pOuterBld+j)!=0)&&((pFirstProvince+i)->CanBuild(j))&&
				(arPlayer[Player].Gold>=GoldLimit+arPlayer[Player].GetOutBldGoldPrice(j))&&
				((arPlayer[Player].GetOutBldGemPrice(j)==0)||(arPlayer[Player].Gem>=GemLimit+arPlayer[Player].GetOutBldGemPrice(j))))
				{
					clOuterBuilding *BLD=(pFirstOuterBld+j);
					int NewTaste=BLD->iGoldAdd+BLD->iGemAdd*5+BLD->iPort*3+BLD->iUnrest*3+Random(5);
					if (NewTaste>Taste)
					{
						Taste=NewTaste;
						Build=j;
						Province=i;
					};
				};
			};
		};
	};
	if (Build>0)
	{
		(pFirstProvince+Province)->ConstructBuilding(Build,false, true);
		char Mess[500];
		sprintf(Mess,"%s построил %s в провинции %s",arPlayer[Player].GetName(),(pFirstOuterBld+Build)->sName,(pFirstProvince+Province)->sName);
		FileLog(Mess);
	}
	else
	{bProceed=false;}

	};
};

//Выбор умения при повышении героя
void AIHeroLvlUp(dcHero * Hero)
{	
	int OldLife=Hero->pSquad[0]->GetLife();		
	int Stat=Hero->LevelUp();
	ClearRoll();
	int Count=0;
	for (int i=0; i<4; i++)
	{
		if (arHeroLvlUp[0][i]>0)
		{
			RollerN[Count]=i+1;
			RollerP[Count]=10;
			Count++;
		};
	};
		
	if (Count>0)
	{
		int Choose=Roll(false)-1;
		Hero->iLevel++;
		Hero->apSkill[arHeroLvlUp[1][Choose]][0]=arHeroLvlUp[0][Choose];
		Hero->apSkill[arHeroLvlUp[1][Choose]][1]++;
		if (Hero->iLevel==10)
		{			
			Hero->ipClass2=arSkills[arHeroLvlUp[0][Choose]].iClass;
		}
		else
		{
			switch (Stat)
			{
			case 1:
				Hero->iHealth++;
				break;
			case 2:			
				Hero->iMagic++;
				break;
			case 3:			
				Hero->iLeader++;
				break;
			case 0:
				Hero->iLifeBonus++;
				break;
			};			
		};
	};
	Hero->pSquad[0]->iLife+=(Hero->pSquad[0]->GetLife()-OldLife);
	Hero->ChangeUnitType();
};

//Заполнение гарнизонов крепостей
void FillGarrison(int Player)
{
	//Всегда держим одного юнита в родовом замке
	bool bHomeDefender=true;
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((pFirstProvince+arPlayer[Player].ipProvince)->pGarrison->pSquad[i])
		{bHomeDefender=false;};
	};
	if ((bHomeDefender)&&(arPlayer[Player].arRecruit[0]>0))
	{
		dcUnit *Rookie=new dcUnit(arPlayer[Player].arRecruit[0],arPlayer[Player].GetBldParam1(20));						
		(pFirstProvince+arPlayer[Player].ipProvince)->pGarrison->PlaceUnit(Rookie,(pFirstProvince+arPlayer[Player].ipProvince)->pGarrison->GetFreeUnitSlot(1));
		arPlayer[Player].Gold-=arPlayer[Player].GetUnitGoldPrice(arPlayer[Player].arRecruit[0],arPlayer[Player].ipProvince);
		arPlayer[Player].Gem-=arPlayer[Player].GetUnitGemPrice(arPlayer[Player].arRecruit[0],arPlayer[Player].ipProvince);						
		for (int i=0; i<MAX_UNIT_RES; i++)
		{
			if ((pFirstUnitType+arPlayer[Player].arRecruit[0])->arRes[i]>0)
			{
				if (arPlayer[Player].GetResource((pFirstUnitType+arPlayer[Player].arRecruit[0])->arRes[i])==0)
				{
					CurGame.ResPrices[(pFirstUnitType+arPlayer[Player].arRecruit[0])->arRes[i]]+=CurGame.ResBuy;
				};
			};
		};
	};
	//Заполняем гарнизоны если рядом враг
	for (int i=1; i<=MapSize; i++)
	{
		dcProvince *PR=(pFirstProvince+i);
		if ((PR->ipPlayer==Player)&&(PR->GetSiegeIndex()<0)&&(PR->pGarrison))
		{
			bool WillHire=false;
		for (int j=0; j<MAX_NEIGHBOUR; j++)
		{
			if (((pFirstProvince+PR->arNeighbour[j])->ipPlayer>0)&&((pFirstProvince+PR->arNeighbour[j])->ipPlayer!=Player))
			{
				WillHire=true;
			};
		};
		if (WillHire)
		{
			dcHero *HR=PR->pGarrison;
		for (int Rank=4; Rank>0; Rank--)
		{
			//Найм новобранцев
			int Count=1;
			while ((Count>0)&&(HR->GetFreeUnitSlot(Rank)))
			{
				Count=0;
				ClearRoll();
				for (int i=0; i<MAX_RECRUIT-1; i++)
				{
					if ((arPlayer[Player].arRecruit[i]>0)&&((pFirstUnitType+arPlayer[Player].arRecruit[i])->iLevel==Rank)&&
						(arPlayer[Player].Gold>=arPlayer[Player].GetUnitGoldPrice(arPlayer[Player].arRecruit[i],arPlayer[Player].ipProvince))&&(arPlayer[Player].Gem>=arPlayer[Player].GetUnitGemPrice(arPlayer[Player].arRecruit[i],arPlayer[Player].ipProvince)))
					{
						RollerN[Count]=arPlayer[Player].arRecruit[i];
						RollerP[Count]=10+GetUnitPower(arPlayer[Player].arRecruit[i]);
						Count++;
					};
				};
				if (Count>0)
				{
					int Recruit=Roll(false);
					dcUnit *Rookie=new dcUnit(Recruit,arPlayer[Player].GetBldParam1(20));						
					HR->PlaceUnit(Rookie,HR->GetFreeUnitSlot(Rank));
					if ((arPlayer[Player].Gold>=arPlayer[Player].GetUnitGoldPrice(Recruit,arPlayer[Player].ipProvince))&&(arPlayer[Player].Gem>=arPlayer[Player].GetUnitGemPrice(Recruit,arPlayer[Player].ipProvince)))
					{
						arPlayer[Player].Gold-=arPlayer[Player].GetUnitGoldPrice(Recruit,arPlayer[Player].ipProvince);
						arPlayer[Player].Gem-=arPlayer[Player].GetUnitGemPrice(Recruit,arPlayer[Player].ipProvince);						
						for (int i=0; i<MAX_UNIT_RES; i++)
						{							
							if ((pFirstUnitType+Recruit)->arRes[i]>0)
							{
								if (arPlayer[Player].GetResource((pFirstUnitType+Recruit)->arRes[i])==0)
								{							
									CurGame.ResPrices[(pFirstUnitType+Recruit)->arRes[i]]+=CurGame.ResBuy;
								};
							};					
						};
					};
				};				
			};
		};
		};
	};
	};
};

//Выбор предметов при переполненном инвентаре
void TakeLootAI(dcHero *HR)
{
	//Надеваем подходящие вещи из лута
	bool TwoHand=false;
	for (int j=0; j<3; j++)
	{
		if (j==2)
		{
			TwoHand=true;
		};
		for (int i=0; i<9; i++)
		{
			if (arLootTable[i][0]>0)
			{
				int Slot=TryItem(arLootTable[i][0],HR,TwoHand);
				if (Slot>0)
				{
					bool CanDo=true;
					if ((Slot==1)&&((pFirstItem+arLootTable[i][0])->ipSlot==100)&&(HR->arInventory[2][0]>0))
					{CanDo=false;};
					if ((Slot==2)&&(HR->arInventory[1][0]>0)&&((pFirstItem+HR->arInventory[1][0])->ipSlot==100))
					{CanDo=false;};
					if (CanDo)
					{
						int It=arLootTable[i][0], Dr=arLootTable[i][1];
						arLootTable[i][0]=HR->arInventory[Slot][0];
						arLootTable[i][1]=HR->arInventory[Slot][1];
						HR->arInventory[Slot][0]=It;
						HR->arInventory[Slot][1]=Dr;
					};
				};
			};			
		};
	};	
	bool Proceed=false;
	for (int i=0; i<9; i++)
	{
		if (arLootTable[i][0]>0)
		{
			Proceed=true;
			break;
		};
	};

	while (Proceed)
	{
		int Drop=-1; int Price=10000; bool bDropAll=false;
		while (Drop<0)
		{
			for (int i=0; i<9; i++)
			{
				if ((arLootTable[i][0]>0)&&((CanSell(HR->ipPlayer,arLootTable[i][0]))||(bDropAll)))
				{
					if ((pFirstItem+arLootTable[i][0])->iGoldCost+(pFirstItem+arLootTable[i][0])->iGemCost*3<Price)
					{
						Price=(pFirstItem+arLootTable[i][0])->iGoldCost+(pFirstItem+arLootTable[i][0])->iGemCost*3;
						Drop=i;
					};
				};
			};		
			for (int i=16; i<=INVENTORY_SIZE; i++)
			{
				if ((HR->arInventory[i][0]>0)&&((CanSell(HR->ipPlayer,HR->arInventory[i][0]))||(bDropAll)))
				{
					if ((pFirstItem+HR->arInventory[i][0])->iGoldCost+(pFirstItem+HR->arInventory[i][0])->iGemCost*3<Price)
					{
						Price=(pFirstItem+HR->arInventory[i][0])->iGoldCost+(pFirstItem+HR->arInventory[i][0])->iGemCost*3;
						Drop=i;
					};				
				};
			};
			bDropAll=true;
		};
		if (Drop<16)
		{
			arLootTable[Drop][0]=0;
			arLootTable[Drop][1]=0;
		}
		else
		{
			int Pick=0;
			while (arLootTable[Pick][0]==0)
			{Pick++;};
			HR->arInventory[Drop][0]=arLootTable[Pick][0];
			HR->arInventory[Drop][1]=arLootTable[Pick][1];
			arLootTable[Pick][0]=0;
			arLootTable[Pick][1]=0;
		};
		Proceed=false;
		for (int i=0; i<9; i++)
		{
			if (arLootTable[i][0]>0)
			{Proceed=true;};
		};
	};	
};


//Возвращает, насколько высока вероятность атаки на провинцию (с рекурсивным учётом вражеских портов)
//0 - внутри страны, 1 - граничит с нейтралом, 2 - с оппонентом, 3 - с врагом, 4 - вражеский герой в соседней провинции
int GetDanger(int Province, int Player, int Port)
{
	dcProvince *PR=(pFirstProvince+Province);
	int Danger=0;
	for (int i=0; i<MAX_NEIGHBOUR; i++)
	{
		if ((Danger<4)&&(PR->arNeighbour[i]>0))
		{
			if ((pFirstProvince+PR->arNeighbour[i])->ipTerrain==WATER_ID)
			{
				if (Port<2)
				{
					int SeaDanger=GetDanger(PR->arNeighbour[i],Player,Port+1);
					if (Danger<SeaDanger)
					{Danger=SeaDanger;};
				};
			}
			else
			{
				if (((pFirstProvince+PR->arNeighbour[i])->ipTerrain!=MOUNTAIN_ID)&&
					((pFirstProvince+PR->arNeighbour[i])->ipPlayer!=Player)&&					
					((pFirstProvince+PR->arNeighbour[i])->GetPort()>=Port))
				{
					int NG=(pFirstProvince+PR->arNeighbour[i])->ipPlayer;
					if (NG>0)
					{
						if (arPlayer[Player].Politic[NG].Treaty==1)
						{
							Danger=3;
							for (int HR=0; HR<MAX_HERO; HR++)
							{
								if ((arPlayer[NG].arHero[HR])&&(arPlayer[NG].arHero[HR]->iDead<0)&&(arPlayer[NG].arHero[HR]->ipProvince==PR->arNeighbour[i]))
								{Danger=4;};
							};
						}
						else
						{
							if (Danger<2)
							{Danger=2;};
						};
					}
					else
					{
						if ((Danger==0)&&(Port==0))
						{Danger=1;};
					};
				};
			};
		};
	};
	return Danger;
};

//Выбрать и провести ритуал
void CastRitualAI(int Player)
{	
	int Cycle=arPlayer[Player].iCanCast;
	while (Cycle>0)
	{
		int Ritual=0, Target=0, Hero=-1, Taste=0, NewTaste, NewHero, Temp;
		for (int Rit=1; Rit<=iMaxRitual; Rit++)
		{
			if (((Rit==5)&&(IsRitualAllowed(Player,Rit)))||(IsRitualCheap(Player,Rit)))
			{
				clRitual *pRIT=(pFirstRitual+Rit);
				for (int Prov=1; Prov<=MapSize; Prov++)
				{
					dcProvince *PR=(pFirstProvince+Prov);
					if ((*(arPlayer[Player].pFog+Prov)==0)&&(PR->CanCast(Rit,Player)))
					{						
						NewTaste=0;
						NewHero=-1;
						switch (Rit)
						{
						case 2: //Феерия
							if (PR->GetMood()<0)
							{
								NewTaste=abs(PR->GetMood())*PR->GetLevel()+PR->arUnrest[Player]/25;								
							};
							break;
						case 3:	//Волшебный ларец
							if ((arPlayer[Player].Gold<=GoldLimit)&&(PR->GetMood()>0))
							{
								NewTaste=PR->GetMood()*2+PR->GetLevel();
							};
							break;
						case 5:	//Тёмный ритуал
							if ((arPlayer[Player].Gem<=GemLimit)&&(PR->GetMood()>0))
							{
								NewTaste=PR->GetMood()*2+PR->GetLevel();
							};
							break;
						case 8:	//Бой с иллюзией
							for (int HR=0; HR<MAX_HERO; HR++)
							{
								Temp=0;
								if ((arPlayer[Player].arHero[HR])&&(arPlayer[Player].arHero[HR]->iDead<0)&&(arPlayer[Player].arHero[HR]->ipProvince==Prov))
								{
									for (int i=1; i<MAX_SQUAD; i++)
									{
										if ((arPlayer[Player].arHero[HR]->pSquad[i])&&(arPlayer[Player].arHero[HR]->pSquad[i]->iLife>0))
										{Temp+=3-arPlayer[Player].arHero[HR]->pSquad[i]->iLevel/10;};
									};
								};
								if (Temp>NewTaste)
								{
									NewTaste=Temp;
									NewHero=HR;
								};
							};
							break;
						case 9:	//Сотворение
							for (int HR=0; HR<MAX_HERO; HR++)
							{
								if (arPlayer[Player].arHero[HR])
								{
									for (int i=1; i<17; i++)
									{
										if (arPlayer[Player].arHero[HR]->arInventory[i][0]==0)
										{NewTaste++;};
									};
								};
							};
							break;
						case 10: //Дары земли
							for (int i=0; i<MAX_NEIGHBOUR; i++)
							{								
								NewTaste=(PR->GetLevel()-GetDanger(Prov,Player,0))*10;
								if (PR->GetMood()<0)
								{NewTaste+=PR->GetMood()*10;};
							};
							break;
						case 13: //Небесная стража
							{
								if (PR->pGuard)
								{NewTaste=(GetDefenderPower(64,NULL)-GetDefenderPower(PR->iDefender,PR->pGuard))*20;}
								else
								{NewTaste=GetDefenderPower(64,NULL)*30;};								
								NewTaste*=(GetDanger(Prov,Player,0)+1);
								if ((PR->GetMood()<0)&&(NewTaste>0))
								{
									NewTaste+=abs(PR->GetMood())*PR->GetLevel()/3+PR->arUnrest[Player]/100;
								};
								if (arPlayer[Player].ipProvince==Prov)
								{NewTaste*=2;};
							};
							break;
						case 15: //Магический барьер
							{
							int Danger=GetDanger(Prov,Player,0);
							if (Danger>=3)
							{
								NewTaste=PR->GetGoldIncome()+PR->GetGemIncome()*2;
								if ((PR->GetResource(1)>0)&&(arPlayer[Player].GetResource(PR->GetResource(1))<2))
								{
									NewTaste+=arPlayer[Player].GetResGoldPriceStat(PR->GetResource(1))/5+arPlayer[Player].GetResGemPriceStat(PR->GetResource(1))/2;
								};
								if (PR->pGuard)
								{
									int GuardPower=GetDefenderPower(PR->iDefender,PR->pGuard)+1;
									if (GuardPower>0)
									{NewTaste/=GuardPower;};
								};
								if (arPlayer[Player].ipProvince==Prov)
								{NewTaste*=2;};
								NewTaste*=(Danger-2);
							};
							};
							break;
						case 14:	//Безумие
							if ((Player!=PR->ipPlayer)&&(PR->ipPlayer>0))
							{
								NewTaste=PR->GetLevel()-5;
								if (PR->GetResource(1)>0)
								{NewTaste+=3;};
								if ((arPlayer[PR->ipPlayer].ipProvince==Prov))
								{NewTaste+=10;};
								switch (PR->ipRace)
								{
								case 2:
								case 3:
								case 7:
									NewTaste*=3;
									break;
								case 5:
									NewTaste*=2;
									break;
								};
								if (arPlayer[Player].iUnion==PR->ipRace)
								{NewTaste*=2;};
								if (!PR->pGuard)
								{NewTaste*=3;}
								else
								{
									if ((arPlayer[Player].Politic[PR->ipPlayer].Treaty==1)&&(arPlayer[Player].Politic[PR->ipPlayer].CanKill<0))
									{NewTaste*=4;};
								};
							};
							break;
						case 18:	//Восстание нежити
						case 23:	//Нашествие демонов
							if ((Player!=PR->ipPlayer)&&(PR->ipPlayer>0))
							{
								NewTaste=(PR->GetGoldIncome()+PR->GetGemIncome()*2)/8;
								if (PR->GetResource(1)>0)
								{NewTaste+=3;};
								if ((arPlayer[PR->ipPlayer].ipProvince==Prov))
								{NewTaste+=10;};
								if ((!PR->pGuard)||(GetDefenderPower(PR->iDefender,PR->pGuard)<2))
								{NewTaste*=3;};
								if ((arPlayer[Player].Politic[PR->ipPlayer].Treaty==1)&&(arPlayer[Player].Politic[PR->ipPlayer].CanKill<0))
								{NewTaste*=4;};
							};
							break;
						case 17:	//Серный дождь
						case 22:	//Пожар
						case 24:	//Поветрие
							if ((Player!=PR->ipPlayer)&&(PR->ipPlayer>0)&&(arPlayer[Player].Politic[PR->ipPlayer].Treaty==1)&&(arPlayer[Player].Politic[PR->ipPlayer].CanKill<0))
							{
								NewTaste=Random(10)-5;
								if ((arPlayer[PR->ipPlayer].ipProvince==Prov))
								{NewTaste+=10;};
								for (int HR=0; HR<MAX_HERO; HR++)
								{
									if ((arPlayer[PR->ipPlayer].arHero[HR])&&(arPlayer[PR->ipPlayer].arHero[HR]->iDead<0)&&(arPlayer[PR->ipPlayer].arHero[HR]->pProvince==PR))
									{
										NewTaste+=arPlayer[PR->ipPlayer].arHero[HR]->iLevel/2;
									};
								};
							};
							break;
						case 16: //Снежный буран
							if ((Player!=PR->ipPlayer)&&(PR->ipPlayer>0)&&(arPlayer[Player].Politic[PR->ipPlayer].Treaty==1))
							{
								bool bCast=false;
								for (int HR=0; HR<MAX_HERO; HR++)
								{
									if ((arPlayer[PR->ipPlayer].arHero[HR])&&(arPlayer[PR->ipPlayer].arHero[HR]->iDead<0)&&(arPlayer[PR->ipPlayer].arHero[HR]->pProvince==PR))
									{
										if (arPlayer[PR->ipPlayer].arHero[HR]->iLevel>10)
										{bCast=true;};
										NewTaste+=arPlayer[PR->ipPlayer].arHero[HR]->iLevel;
										for (int i=0; i<MAX_SQUAD; i++)
										{
											if (arPlayer[PR->ipPlayer].arHero[HR]->pSquad[i])
											{NewTaste+=arPlayer[PR->ipPlayer].arHero[HR]->pSquad[i]->pType()->iLevel;};
										};
									};
								};
								if (bCast)
								{NewTaste*=10;}
								else
								{NewTaste=0;};
							};
							break;
						case 19: //Дары Небес
							NewTaste=50-PR->GetMood()*2;
							break;
						case 21: //Башня Молний
							NewTaste=Random(30);
							if (PR->ipTerrain==4)
							{NewTaste+=10;};
							break;
						};
						
						if (NewTaste>Taste)
						{
							Taste=NewTaste;
							Ritual=Rit;
							Target=Prov;
							Hero=NewHero;
						};
					};
				};
			};					
		};
		if (Ritual>0)
		{
			arPlayer[Player].CastRitual(Ritual,Target,Hero);			
		};
		Cycle--;
	};
};

//Вычисление политического преимущества (чем больше, тем сильней Player2 относительно 1-го)
int GetPoliticalPower(int Player1, int Player2)
{
	int Result=arPlayer[Player1].Politic[Player2].Power;
	//Учёт политической ситуации
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if ((i!=Player1)&&(i!=Player2)&&(arPlayer[i].bInGame))
		{
			if (arPlayer[Player1].Politic[i].Treaty==1)
			{Result+=20;};
			if (arPlayer[Player1].Politic[i].Treaty==3)
			{Result-=30;};
			if (arPlayer[Player2].Politic[i].Treaty==1)
			{Result-=20;};
			if (arPlayer[Player2].Politic[i].Treaty==3)
			{Result+=30;};
		};
	};
	//Учёт уровня сложности
	if (arPlayer[Player2].Control==1)
	{Result-=arPlayer[Player1].iDifficulty*3;}
	else
	{Result+=(arPlayer[Player2].iDifficulty-arPlayer[Player1].iDifficulty)*3;};
	//Учёт отношений
	Result+=arPlayer[Player1].GetAttitude(Player2)/3;
	//Учёт незащищённых граничных провинций
	bool Border, Border1=false;
	for (int i=1; i<=MapSize; i++)
	{
		if ((pFirstProvince+i)->ipPlayer==Player1)
		{Result--;};
		if ((pFirstProvince+i)->ipPlayer==Player2)
		{Result++;};
		//if (((pFirstProvince+i)->ipPlayer==Player1)&&((pFirstProvince+i)->pGuard==NULL))
		//{
		//	Border=false;
		//	for (int j=0; j<MAX_NEIGHBOUR; j++)
		//	{
		//		if (((pFirstProvince+i)->arNeighbour[j]>0)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipPlayer==Player2))
		//		{Border=true;};
		//	};
		//	if (Border)
		//	{Result+=5;};
		//};
		//if ((pFirstProvince+i)->ipPlayer==Player2)
		//{
		//	Border=false;
		//	for (int j=0; j<MAX_NEIGHBOUR; j++)
		//	{
		//		if (((pFirstProvince+i)->arNeighbour[j]>0)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipPlayer==Player1))
		//			//&&(*(pMoveMap+(pFirstProvince+i)->arNeighbour[j])>=0))
		//		{
		//			Border=true;
		//			Border1=true;
		//		};
		//	};
		//	if (Border)
		//	{
		//		if ((pFirstProvince+i)->pGuard==NULL)
		//		{Result-=5;};
		//	};
		//};
	};
	//Учёт живых героев
	for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[Player1].arHero[i])&&(arPlayer[Player1].arHero[i]->iDead<0))
		{Result-=1+arPlayer[Player1].arHero[i]->iLevel/2;};
		if ((arPlayer[Player2].arHero[i])&&(arPlayer[Player2].arHero[i]->iDead<0))
		{Result+=1+arPlayer[Player2].arHero[i]->iLevel/2;};
	};
	/*if (!Border1)
	{
		if (Result<0)
		{Result/=2;};
	};*/
	//Осада родовой провинции
	if ((pFirstProvince+arPlayer[Player1].ipProvince)->GetSiegeIndex()==Player2)
	{Result+=50;};
	if ((pFirstProvince+arPlayer[Player2].ipProvince)->GetSiegeIndex()==Player1)
	{Result-=250;};
	//Близость к родовой провинции
	Border=false;Border1=false;
	for (int j=0; j<MAX_NEIGHBOUR; j++)
	{
		if (((pFirstProvince+arPlayer[Player1].ipProvince)->arNeighbour[j]>0)&&((pFirstProvince+(pFirstProvince+arPlayer[Player1].ipProvince)->arNeighbour[j])->ipPlayer==Player2))
		{Border=true;};
		if (((pFirstProvince+arPlayer[Player2].ipProvince)->arNeighbour[j]>0)&&((pFirstProvince+(pFirstProvince+arPlayer[Player2].ipProvince)->arNeighbour[j])->ipPlayer==Player1))
		{Border1=true;};
	};
	if (Border)
	{Result+=30;};
	if (Border1)
	{Result-=100;};
	return Result;
};

int GetResPrice(int Res)
{
	int ResPrice=0;
	ResPrice+=(pFirstResource+Res)->iGold*CurGame.ResPrices[Res]/100;
	ResPrice+=(pFirstResource+Res)->iGem*CurGame.ResPrices[Res]*5/200;
	if (Res<5)
	{ResPrice/=3;}
	else
	{
		if (Res<8)
		{ResPrice/=5;}
		else
		{ResPrice/=6;};
	};
	return ResPrice;
};

//Вычисление выгоды сделки
int GetTradeProfit(int Player, dcMessage *Mess)
{
	int Result=0;
	//Торговля
	if (Mess->Type==2)
	{
		Result+=Mess->Trade.Gold*2;
		Result+=Mess->Trade.Gem*5;
		Result-=Mess->Trade1.Gold*2;
		Result-=Mess->Trade1.Gem*5;
		if (arPlayer[Player].GoldInc<Mess->Trade1.Gold*2)
		{Result-=100;};
		if (arPlayer[Player].GoldInc<Mess->Trade1.Gold)
		{Result-=10000;};
		if (arPlayer[Player].GemInc<Mess->Trade1.Gem*2)
		{Result-=100;};
		if (arPlayer[Player].GemInc<Mess->Trade1.Gem)
		{Result-=10000;};
		int arRes[10];
		for (int i=0; i<10; i++)
		{arRes[i]=0;};

		for (int i=0; i<16; i++)
		{
			if (Mess->Trade.Stuff[i]>0)
			{
				if (arPlayer[Player].GetResource(Mess->Trade.Stuff[i])+arRes[Mess->Trade.Stuff[i]]==0)
				{					
					arRes[Mess->Trade.Stuff[i]]++;
					/*int ResPrice=0;
					ResPrice+=(pFirstResource+Mess->Trade.Stuff[i])->iGold*CurGame.ResPrices[Mess->Trade.Stuff[i]]/100;
					ResPrice+=(pFirstResource+Mess->Trade.Stuff[i])->iGem*CurGame.ResPrices[Mess->Trade.Stuff[i]]*5/200;
					if (Mess->Trade.Stuff[i]<5)
					{ResPrice/=2;}
					else
					{
						if (Mess->Trade.Stuff[i]<8)
						{ResPrice/=3;}
						else
						{ResPrice/=4;};
					};*/
					Result+=GetResPrice(Mess->Trade.Stuff[i]);
				};
			};
			if (Mess->Trade1.Stuff[i]>0)
			{
			/*	int ResPrice=0;
				ResPrice+=(pFirstResource+Mess->Trade1.Stuff[i])->iGold*CurGame.ResPrices[Mess->Trade1.Stuff[i]]/100;
				ResPrice+=(pFirstResource+Mess->Trade1.Stuff[i])->iGem*CurGame.ResPrices[Mess->Trade1.Stuff[i]]*5/200;
				if (Mess->Trade1.Stuff[i]<5)
				{ResPrice/=2;}
				else
				{
					if (Mess->Trade1.Stuff[i]<8)
					{ResPrice/=3;}
					else
					{ResPrice/=4;};
				};*/
				Result-=GetResPrice(Mess->Trade1.Stuff[i]);
			};
		};
	};
	//Обмен
	if (Mess->Type==3)
	{
		Result+=Mess->Trade.Gold/2;
		Result+=Mess->Trade.Gem*5/4;
		Result-=Mess->Trade1.Gold/2;
		Result-=Mess->Trade1.Gem*5/4;
		if (arPlayer[Player].Gold<Mess->Trade1.Gold*2)
		{Result-=100;};
		if (arPlayer[Player].Gold<Mess->Trade1.Gold)
		{Result-=10000;};
		if (arPlayer[Player].Gem<Mess->Trade1.Gem*2)
		{Result-=100;};
		if (arPlayer[Player].Gem<Mess->Trade1.Gem)
		{Result-=10000;};
		dcProvince *PR;
		for (int i=0; i<8; i++)
		{
			if (Mess->Trade.Stuff[i]>0)
			{
				PR=(pFirstProvince+Mess->Trade.Stuff[i]);
				Result+=PR->GetGoldIncome()+PR->GetGemIncome()*2;
				if (PR->GetResource(1)>0)
				{
					if (arPlayer[Player].GetResource(PR->GetResource(1))==0)
					{Result+=50;}
					else
					{Result+=15;};
				};
				if (PR->pGarrison)
				{Result+=20;};
				for (int j=0; j<MAX_NEIGHBOUR; j++)
				{
					if (PR->arNeighbour[j]>0)
					{
						if ((pFirstProvince+PR->arNeighbour[j])->ipPlayer==Player)
						{Result+=7;};
					};
				};
				
			};
			if (Mess->Trade1.Stuff[i]>0)
			{
				PR=(pFirstProvince+Mess->Trade1.Stuff[i]);
				Result-=CurGame.iCurrentTurn*3;
				Result-=(PR->GetGoldIncome()*20+PR->GetGemIncome()*40);
				if (PR->GetResource(1)>0)
				{
					Result-=500;
				};
				if (PR->pGarrison)
				{Result-=500;};
				if (PR->pGuard)
				{Result-=200;};
				for (int j=0; j<MAX_NEIGHBOUR; j++)
				{
					if (PR->arNeighbour[j]>0)
					{
						if ((pFirstProvince+PR->arNeighbour[j])->ipPlayer==Player)
						{Result-=500;};
					};
				};
			};
		};
		for (int i=8; i<16; i++)
		{
			if (Mess->Trade.Stuff[i]>0)
			{
				if (CanSell(Player,Mess->Trade.Stuff[i]))
				{Result+=((pFirstItem+Mess->Trade.Stuff[i])->iGoldCost+(pFirstItem+Mess->Trade.Stuff[i])->iGemCost*2)/4;}
				else
				{Result+=(pFirstItem+Mess->Trade.Stuff[i])->iGoldCost+(pFirstItem+Mess->Trade.Stuff[i])->iGemCost*2;};
			};
		};
	};
	return Result;
};




//Выбор ответа на дипломатическое предложение
int GetDiploAnswerAI(int Player, int Writer)
{
	int Result=1;
	if (arPlayer[Player].Politic[Writer].Message.Type>0)
	{
		if (arPlayer[Player].Politic[Writer].Message.Treaty==1)
		{
			arPlayer[Player].CancelDeal(Writer);
			return 0;
		};
		if ((arPlayer[Player].Politic[Writer].Message.Type==4)&&(arPlayer[Writer].Politic[Player].Message.Trade.bTrade))
		{
			arPlayer[Player].CancelDeal(Writer);
			return 0;
		};
	};
	if (GetCurPlayers()==2)
	{
		arPlayer[Player].CancelDeal(Writer);
		return 19;
	};
	if ((arPlayer[Player].Politic[Writer].Treaty==1)&&
		((CurGame.iCurrentTurn-arPlayer[Player].Politic[Writer].arTalk[1][2]<(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iDiplomacy)||
		(CurGame.iCurrentTurn-arPlayer[Writer].Politic[Player].arTalk[1][2]<(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iDiplomacy)))
	{
		arPlayer[Player].CancelDeal(Writer);
		return 23;
	};
	dcMessage Mess=arPlayer[Writer].Politic[Player].Message;	
	int Power=0;
	if ((Mess.Type==2)||(Mess.Type==3))
	{
		Power+=GetTradeProfit(Player,&Mess);
		if (Mess.Treaty==0)
		{
			if (Mess.Trade1.IsFree())
			{Power-=(CurGame.iCurrentTurn+20-(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iGreed);};
			if (Mess.Trade.IsFree())
			{
				Power+=GetPoliticalPower(Player,Writer)*6-(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro/5-(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iGreed;
				if (arPlayer[Player].Politic[Writer].CanKill>0)
				{Power-=arPlayer[Player].Politic[Writer].CanKill*5;};
				arPlayer[Player].Politic[Writer].Attitude-=2;
			};
		};
	};
	if ((Mess.Treaty==2)||(Mess.Treaty==3))
	{	
		Power/=2;
		Power+=GetPoliticalPower(Player,Writer)*2;
	};	
	if (Mess.Treaty==2)
	{
		Power-=arPlayer[Player].Politic[Writer].CanKill;
		Power-=(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro/3;
		//Проверка союзников при попытке заключить мир
		for (int i=1; i<=CurGame.iMaxPlayers; i++)
		{
			if ((i!=Player)&&(arPlayer[i].bInGame)&&(arPlayer[i].Politic[Writer].Treaty==3)&&(arPlayer[i].Politic[Player].Treaty==1))
			{
				if ((CurGame.iCurrentTurn-arPlayer[i].Politic[Player].arTalk[2][2]<(pFirstPlayerProfile+arPlayer[i].ipProfile)->iDiplomacy)||(CurGame.iCurrentTurn-arPlayer[i].Politic[Player].arTalk[1][2]<(pFirstPlayerProfile+arPlayer[i].ipProfile)->iDiplomacy)||(CurGame.iCurrentTurn-arPlayer[Player].Politic[i].arTalk[1][2]<(pFirstPlayerProfile+arPlayer[i].ipProfile)->iDiplomacy))
				{
					arPlayer[Player].CancelDeal(Writer);
					return 1000+i;
				}
				else
				{
					int Pea=(pFirstPlayerProfile+arPlayer[i].ipProfile)->iAggro/2+40-arPlayer[i].GetAttitude(Player);
					if ((Pea>10)&&(Random(Pea)>10))
					{						
						arPlayer[i].Politic[Player].arTalk[2][2]=CurGame.iCurrentTurn;
						arPlayer[Player].CancelDeal(Writer);
						return 1000+i;
					};
				};
			};
		};
		bool bTraitor=false;
		if (arPlayer[Player].Politic[Writer].arTraitor[0]>0)
		{
			Power-=500;
			bTraitor=true;
		};
		if (arPlayer[Player].Politic[Writer].arTraitor[1]>0)
		{
			Power-=1000;
			bTraitor=true;
		};
		
		if (Power+arPlayer[Player].GetAttitude(Writer)>0)
		{
			if (Power<0)
			{
				Result=1;
				arPlayer[Player].Politic[Writer].Attitude--;
			}
			else
			{
				if (Power<100)
				{Result=2;}
				else
				{
					Result=3;
					arPlayer[Player].Politic[Writer].Attitude+=GetMin(3,(Power-80)/20);
				};
			};
		}
		else
		{
			if (bTraitor)
			{
				arPlayer[Player].CancelDeal(Writer);
				return 17;
			};
			if (Power<-50)
			{
				Result=-3;
				arPlayer[Player].Politic[Writer].Attitude--;
			}
			else
			{
				if (Power<0)
				{Result=-2;}
				else
				{Result=-1;};
			};
		};
	};
	if (Mess.Treaty==3)
	{
		bool bTraitor=false;
		if (arPlayer[Player].Politic[Writer].arTraitor[0]>0)
		{
			Power-=1300;
			bTraitor=true;
		};
		if (arPlayer[Player].Politic[Writer].arTraitor[1]>0)
		{
			Power-=2500;
			bTraitor=true;
		};
		if ((Power+arPlayer[Player].GetAttitude(Writer)/2>100)&&(arPlayer[Player].GetAttitude(Writer)>=30)&&(arPlayer[Player].GetUnionCount()==0)&&(arPlayer[Writer].GetUnionCount()==0))
		{
			if (Power<0)
			{
				Result=1;
				arPlayer[Player].Politic[Writer].Attitude--;
			}
			else
			{
				if (Power<100)
				{Result=2;}
				else
				{
					Result=3;
					arPlayer[Player].Politic[Writer].Attitude+=GetMin(3,(Power-80)/20);
				};
			};
		}
		else
		{
			if (bTraitor)
			{
				arPlayer[Player].CancelDeal(Writer);
				return 16;
			};
			if (Power<-50)
			{
				Result=-3;
				arPlayer[Player].Politic[Writer].Attitude--;
				}
			else
			{
				if (Power<50)
				{Result=-2;}
				else
				{Result=-1;};
			};
		};
	};
	if (Mess.Treaty==0)
	{
	if ((Power+arPlayer[Player].GetAttitude(Writer)/10>5))
	{
		if (Power<0)
		{
			Result=1;
			arPlayer[Player].Politic[Writer].Attitude--;
		}
		else
		{
			if (Power<100)
			{Result=2;}
			else
			{
				Result=3;
				arPlayer[Player].Politic[Writer].Attitude+=GetMin(3,(Power-80)/20);
			};
		};
	}
	else
	{
		if (Power<-50)
		{
			Result=-3;
			arPlayer[Player].Politic[Writer].Attitude--;
		}
		else
		{
			if (Power<50)
			{Result=-2;}
			else
			{Result=-1;};
		};
	};
	};

	if ((Mess.Type==4)||(Mess.Type==5)||(Mess.Treaty==1)||(Mess.Treaty==4))
	{Result=0;};
	if ((Mess.Treaty==4)&&(CurGame.iGameType==2)&&(arPlayer[Player].ipProfile==3)&&(Writer==1)&&(Campaign.arDialog[103]>0))
	{
		arPlayer[Player].Politic[Writer].Attitude-=400;
		Campaign.arCampPlayer[3].iTreaty=0;
		Campaign.arDialog[103]=0;
		Campaign.arDialog[104]=2;
		Result=32;
	};
	if ((Mess.Type==2)||(Mess.Type==3))
	{
		if (Result>0)
		{
			if (Mess.Trade1.IsFree())
			{
				arPlayer[Player].Politic[Writer].Attitude+=GetMin(2,(1+Power/30));
				if (Mess.Treaty==0)
				{
					arPlayer[Writer].Politic[Player].arTalk[7][2]=CurGame.iCurrentTurn;
					arPlayer[Writer].Politic[Player].arTalk[7][1]=1;
					arPlayer[Player].AcceptDeal(Writer);
					return 25;
				};
			};			
		}
		else
		{
			if ((Mess.Trade1.IsFree())&&(Mess.Treaty==0))
			{
				arPlayer[Player].Politic[Writer].Attitude--;
				arPlayer[Player].CancelDeal(Writer);
				return 24;
			};
		};
		/*if (Mess.Trade.IsFree())
		{
			if ((GetPoliticalPower(Player,Writer)*6+Power-(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro/5>0)&&(CurGame.iCurrentTurn-arPlayer[Writer].Politic[Player].arTalk[6][2]>=20))
			{
				arPlayer[Writer].Politic[Player].arTalk[6][2]=CurGame.iCurrentTurn;
				arPlayer[Writer].Politic[Player].arTalk[6][1]=CurGame.iCurrentTurn;
				Result=1;
			};
			arPlayer[Player].Politic[Writer].Attitude-=2;
		};*/
	};
	if (Mess.Treaty==1)
	{
		arPlayer[Player].Politic[Writer].Attitude-=5;
		Result=27;
	};
	if (Mess.Treaty==4)
	{arPlayer[Player].Politic[Writer].Attitude-=2;};
	if (Mess.Type==4)
	{arPlayer[Player].Politic[Writer].Attitude--;};


	if (Result>=0)
	{arPlayer[Player].AcceptDeal(Writer);}
	else
	{arPlayer[Player].CancelDeal(Writer);};
	return Result;
};

//Получение подходящего текста для вставки в сообщение (Name отвечает за то, нужно ли в текст подставлять имя)
char * GetDipText(int PL, int Index, int Att, int Param, int *Name)
{
	ClearRoll();
	int Count=0, Number=1; bool Pick;
	clDipText * Dip, *Dip1;
	if (arPlayer[PL].Control==1)
	{Dip=pHumanDiplo;}
	else
	{Dip=(pFirstPlayerProfile+arPlayer[PL].ipProfile)->pDipText;};
	Dip1=Dip;
	while (Dip)
	{
		Pick=false;
		if ((Index==Dip->Index)&&(Param==Dip->Param))
		{Pick=true;};
		if ((Dip->Min!=0)&&(Att<Dip->Min))
		{Pick=false;};
		if ((Dip->Max!=0)&&(Att>Dip->Max))
		{Pick=false;};
		if (Pick)
		{
			RollerN[Count]=Number;
			RollerP[Count]=10;
			Count++;
		};
		Number++;
		Dip=Dip->pNext;
	};
	if (Count>0)
	{
		Count=Roll(false);
		Dip=Dip1;
		for (int i=1; i<Count; i++)
		{Dip=Dip->pNext;};
		*Name=Dip->Name;
		return Dip->sText;
	}
	else
	{return NULL;}
};

//Составление текста послания
void MakeDipText(int Player1, int Player2, int Index, int Param)
{
	ClearRoll();
	int Count=0, Number=1; bool Pick;
	clDipText * Dip, *Dip1;	
	if (arPlayer[Player1].Control==1)
	{Dip=pHumanDiplo;}
	else
	{Dip=(pFirstPlayerProfile+arPlayer[Player1].ipProfile)->pDipText;};
	Dip1=Dip;
	while (Dip)
	{
		Pick=false;
		if ((Index==Dip->Index)&&(Param==Dip->Param))
		{Pick=true;};
		if ((Dip->Min!=0)&&(arPlayer[Player1].GetAttitude(Player2)<Dip->Min))
		{Pick=false;};
		if ((Dip->Max!=0)&&(arPlayer[Player1].GetAttitude(Player2)>Dip->Max))
		{Pick=false;};
		if (Pick)
		{
			RollerN[Count]=Number;
			RollerP[Count]=10;
			Count++;
		};
		Number++;
		Dip=Dip->pNext;
	};
	if (Count>0)
	{
		Count=Roll(false);
		Dip=Dip1;
		for (int i=1; i<Count; i++)
		{Dip=Dip->pNext;};
		if (Dip->Name>0)
		{sprintf(arPlayer[Player1].Politic[Player2].Message.Text,Dip->sText,arPlayer[Player2].GetName());}
		else
		{sprintf(arPlayer[Player1].Politic[Player2].Message.Text,Dip->sText);};		
	}
	else
	{sprintf(arPlayer[Player1].Politic[Player2].Message.Text,pFirstPhrase->str);};
};

//Составление сообщения с ответом
void SendAnswerAI(int Player, int Victim)
{
	int Name; char *Mess=NULL;	
	//Ответ союзника при несогласии с миром
	if (arPlayer[Player].Politic[Victim].Answer>1000)
	{
		int Writer=arPlayer[Player].Politic[Victim].Answer-1000;		
		Mess=GetDipText(Writer,28,arPlayer[Writer].GetAttitude(Victim),0,&Name);
		arPlayer[Writer].Politic[0].Message.Type=5;
		if (Name==0)
		{sprintf(arPlayer[Writer].Politic[0].Message.Text,Mess,arPlayer[Victim].GetName(),arPlayer[Player].GetName());}
		else
		{sprintf(arPlayer[Writer].Politic[0].Message.Text,Mess);};
		return;
	};
	arPlayer[Player].Politic[0].Message.Type=5;	
	if (arPlayer[Player].Politic[Victim].Answer>7)
	{
		Mess=GetDipText(Player,arPlayer[Player].Politic[Victim].Answer,arPlayer[Player].GetAttitude(Victim),0,&Name);
	}
	else
	{
		if (arPlayer[Player].Politic[Victim].Answer>0)
		{Mess=GetDipText(Player,15,arPlayer[Player].GetAttitude(Victim),arPlayer[Player].Politic[Victim].Answer-2,&Name);}
		else
		{Mess=GetDipText(Player,14,arPlayer[Player].GetAttitude(Victim),arPlayer[Player].Politic[Victim].Answer+2,&Name);};
	};	
	if (Mess)
	{
		if (Name>0)
		{sprintf(arPlayer[Player].Politic[0].Message.Text,Mess,arPlayer[Victim].GetName());}
		else
		{sprintf(arPlayer[Player].Politic[0].Message.Text,Mess);};
	}
	else
	{sprintf(arPlayer[Player].Politic[0].Message.Text,pFirstPhrase->str);};

};

//Составление сообщения с приветствием
void SendGreetingsAI(int Player, int Victim)
{
	arPlayer[Player].Politic[Victim].Message.Type=5;
	if (GetCurPlayers()==2)
	{
		MakeDipText(Player,Victim,3,0);
		arPlayer[Player].Politic[Victim].Message.Treaty=1;		
	}
	else
	{
		if (CurGame.iGameType==2)
		{
			if ((Campaign.arCampPlayer[arPlayer[Player].ipProfile].bShardMeet)&&(Campaign.arCampPlayer[arPlayer[Player].ipProfile].iTreaty==0))
			{MakeDipText(Player,Victim,2,0);}
			else
			{
				MakeDipText(Player,Victim,26,Campaign.arCampPlayer[arPlayer[Player].ipProfile].iTreaty);
				if (Campaign.arCampPlayer[arPlayer[Player].ipProfile].iTreaty==1)
				{
					arPlayer[Player].Politic[Victim].Treaty=3;
					arPlayer[Victim].Politic[Player].Treaty=3;
				};
			};
			Campaign.arCampPlayer[arPlayer[Player].ipProfile].bShardMeet=true;
		}		
		else
		{MakeDipText(Player,Victim,1,0);};
	};
};

//Составление и рассылка дипломатической почты
void MakeDiplomacyAI(int Player)
{
	if ((CurGame.iGameType==2)&&(arPlayer[Player].ipProfile==3)&&(Campaign.arDialog[104]==1))
	{
		arPlayer[Player].Politic[1].Message.Type=5;
		Campaign.arCampPlayer[3].iTreaty=0;
		Campaign.arDialog[103]=0;
		Campaign.arDialog[104]=2;
		MakeDipText(Player,1,32,0);
		return;
	};
	ClearMoveMap();
	MovePlayer=Player;
	WalkType=1;
	WalkSpeed=1;
	MoveMap(arPlayer[Player].ipProvince,-1,WalkSpeed,0,0);
	int iWar=0;
	bool bExchange=true, bTreaty=true, bUnion=true;
	//Поиск противника если со всеми мир
	if (arPlayer[Player].GetWars()==0)
	{
		for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
		{
			if ((PL!=Player)&&(arPlayer[PL].bInGame)&&(arPlayer[Player].Politic[PL].Treaty==2)&&(arPlayer[Player].Politic[PL].CanKill>0))
			{
				if (iWar==0)
				{iWar=PL;}
				else
				{
					int iOld, iNew;
					iOld=arPlayer[Player].GetAttitude(iWar)-arPlayer[Player].Politic[iWar].CanKill/5+GetPoliticalPower(Player,iWar)/5;
					iNew=arPlayer[Player].GetAttitude(PL)-arPlayer[Player].Politic[PL].CanKill/5+GetPoliticalPower(Player,PL)/5;
				//	iOld=GetPoliticalPower(Player,iWar);
				//	iNew=GetPoliticalPower(Player,PL);
					if (arPlayer[PL].Control==1)
					{iNew-=arPlayer[Player].iDifficulty*2;};
					if (arPlayer[iWar].Control==1)
					{iOld-=arPlayer[Player].iDifficulty*2;};
					if (iNew<iOld)
					{iWar=PL;};
				};
			};
		};
	};

	char MSS[500];

	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		if ((PL!=Player)&&(arPlayer[PL].bInGame)&&(arPlayer[Player].Politic[PL].Treaty>0)&&(arPlayer[PL].Control!=3)&&(arPlayer[Player].Politic[PL].Message.Type==0))
		{
			if (arPlayer[PL].Politic[Player].Message.Type>0)
			{bTreaty=false;}
			else
			{bTreaty=true;};
			if (GetCurPlayers()==2)
			{
				if (arPlayer[Player].Politic[PL].Treaty==3)
				{
					int P1=0, P2=0;					
					for (int i=1; i<=MapSize; i++)
					{
						if ((pFirstProvince+i)->ipPlayer==Player)
						{P1++;};
						if ((pFirstProvince+i)->ipPlayer==PL)
						{P2++;};						
					};
					if (arPlayer[PL].Control==1)
					{
						if ((CurGame.iGameType==2)&&(Campaign.arDialog[103])&&(arPlayer[Player].ipProfile==3))
						{
							//Белез в кампании
							arPlayer[Player].Politic[PL].Message.Type=8;
							MakeDipText(Player,PL,31,0);
						}
						else
						{
						arPlayer[Player].Politic[PL].Answer=0;
						if (P1>=P2*3)
						{
							//Предлагаем сдаться
							arPlayer[Player].Politic[PL].Message.Type=8;
							MakeDipText(Player,PL,21,0);
						}
						else
						{
							if (P2>=P1*3)
							{
								//Сдаёмся
								arPlayer[Player].Politic[PL].Message.Type=9;
								MakeDipText(Player,PL,22,0);
							}
							else
							{
								//Предлагаем жребий
								arPlayer[Player].Politic[PL].Message.Type=7;
								MakeDipText(Player,PL,20,0);
							};
						};
						};
					}
					else
					{
						if (P1>P2)
						{arPlayer[PL].bInGame=false;}
						else
						{arPlayer[Player].bInGame=false;};
					};
				};
				//Объявляем войну если остались вдвоём с оппонентом
				if (arPlayer[Player].Politic[PL].Treaty==2)
				{
					if (arPlayer[PL].Control==1)
					{
						arPlayer[Player].Politic[PL].Message.Type=1;
						arPlayer[Player].Politic[PL].Message.Treaty=1;
						arPlayer[Player].Politic[PL].Answer=0;
						MakeDipText(Player,PL,30,0);
					}
					else
					{
						arPlayer[Player].DeclareWar(PL);
					};
				};
			}
			else
			{
				switch (arPlayer[Player].Politic[PL].Treaty)
				{
				case 1:
					//Предлагаем мир
					if ((GetPoliticalPower(Player,PL)>0)||(arPlayer[Player].Politic[PL].CanKill<1))
					{
						int Time=arPlayer[Player].Politic[PL].arTalk[1][2];
						if (arPlayer[PL].Politic[Player].arTalk[1][2]>Time)
						{Time=arPlayer[PL].Politic[Player].arTalk[1][2];};
						if (arPlayer[Player].Politic[PL].arTalk[2][2]>Time)
						{Time=arPlayer[Player].Politic[PL].arTalk[2][2];};
						if (CurGame.iCurrentTurn-Time>(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iDiplomacy)
						{
							if (((pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()==PL)||(Random(50+CurGame.iCurrentTurn-Time)>(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro/2))
							{
								sprintf(MSS,"Правитель %s предлагает мир правителю %s. Отношения: Politic: %d, Attitude: %d, CanKill: %d",arPlayer[Player].GetName(),arPlayer[PL].GetName(),GetPoliticalPower(Player,PL),arPlayer[Player].GetAttitude(PL), arPlayer[Player].Politic[PL].CanKill);
								FileLog(MSS);
								int Ally1=0, Ally2=0;
								for (int i=1; i<=CurGame.iMaxPlayers; i++)
								{
									if ((i!=PL)&&(i!=Player)&&(arPlayer[i].bInGame))
									{
										if ((arPlayer[i].Politic[Player].Treaty==3)&&(arPlayer[i].Politic[PL].Treaty==1))
										{Ally1=i;};
										if ((arPlayer[i].Politic[PL].Treaty==3)&&(arPlayer[i].Politic[Player].Treaty==1))
										{Ally2=i;};
									};
								};
								if (arPlayer[PL].Control==1)
								{
									arPlayer[Player].Politic[PL].Message.Type=1;
									arPlayer[Player].Politic[PL].Message.Treaty=2;
									MakeDipText(Player,PL,5,0);
									arPlayer[Player].Politic[PL].arTalk[2][2]=CurGame.iCurrentTurn;
								}
								else
								{
									bool Pea=true;
									if ((Ally2>0)&&(arPlayer[Ally2].Control==1))
									{
										Pea=false;
										if ((arPlayer[Player].Politic[Ally2].Message.Type==0)&&(arPlayer[Ally2].Politic[Player].Message.Type==0))
										{
											arPlayer[Player].Politic[Ally2].Message.Type=1;
											arPlayer[Player].Politic[Ally2].Message.Treaty=2;
											MakeDipText(Player,Ally2,5,0);
											arPlayer[Player].Politic[PL].arTalk[2][2]=CurGame.iCurrentTurn;
										};
									};
									if ((Pea)&&(Ally1>0)&&(arPlayer[Ally1].Control==1)&&(arPlayer[Ally1].Politic[PL].Message.Type==0))
									{
										Pea=false;
										arPlayer[Player].Politic[PL].Message.Type=1;
										arPlayer[Player].Politic[PL].Message.Treaty=2;
									};
									if (((pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()==PL)||((Ally2>0)&&((pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()==Ally2))||(arPlayer[PL].Politic[Player].CanKill>=20))
									{Pea=false;};
									if (Pea)
									{
										if (Ally1>0)
										{
											arPlayer[Ally1].Politic[PL].Treaty=2;
											arPlayer[PL].Politic[Ally1].Treaty=2;
											arPlayer[Ally1].Politic[PL].arTalk[2][2]=CurGame.iCurrentTurn;

										};
										if (Ally2>0)
										{
											arPlayer[Ally2].Politic[Player].Treaty=2;
											arPlayer[Player].Politic[Ally2].Treaty=2;
											arPlayer[Player].Politic[Ally2].arTalk[2][2]=CurGame.iCurrentTurn;
										};
										arPlayer[Player].Politic[PL].Treaty=2;
										arPlayer[PL].Politic[Player].Treaty=2;
										arPlayer[Player].Politic[PL].arTalk[2][2]=CurGame.iCurrentTurn;
										FileLog("Согласие");
									};
								};								
							};
						};
					};
					break;
				case 2:
					if ((iWar==PL)&&(CurGame.iCurrentTurn-arPlayer[Player].Politic[PL].arTalk[2][2]>=GetMin(10,CurGame.iCurrentTurn))&&(CurGame.iCurrentTurn-arPlayer[PL].Politic[Player].arTalk[2][2]>=GetMin(10,CurGame.iCurrentTurn)))
					{
					if (((CurGame.iCurrentTurn-arPlayer[Player].Politic[PL].arTalk[6][2]>=GetMin(20,CurGame.iCurrentTurn))||(arPlayer[Player].Politic[PL].arTalk[6][1]==0))&&(CurGame.iCurrentTurn-arPlayer[PL].Politic[Player].arTalk[7][2]>=GetMin(10,CurGame.iCurrentTurn)))
					{
						bTreaty=false;
						int War=1000-GetPoliticalPower(Player,PL)-arPlayer[Player].GetAttitude(PL)+(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro+arPlayer[Player].Politic[PL].CanKill;
						if (arPlayer[PL].Control==1)
						{War+=arPlayer[Player].iDifficulty*20;};
						int Agression=(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro;
						if ((arPlayer[Player].Politic[PL].arTalk[6][2]>0)&&(arPlayer[Player].Politic[PL].arTalk[6][1]==0))
						{Agression+=(10+Random(30));};
					//	if (Random(War)>500)
						if (((bExpand)&&(Random(5)<=Agression/20))||
							((Random(50+(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro/2)>arPlayer[Player].GetAttitude(PL))&&
							(Random(100)<Agression)&&
							(Random(50)<arPlayer[Player].Politic[PL].CanKill)))
						{
							int Ransom=100+(War-500)/3+CurGame.iCurrentTurn;
							if ((CurGame.iCurrentTurn-arPlayer[Player].Politic[PL].arTalk[6][2]>=GetMin(20,CurGame.iCurrentTurn))&&(Random(War)<500+(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iGreed*10)&&(arPlayer[PL].Gem*3+arPlayer[PL].Gold>Ransom))
							//Требуем выкуп
							{
								sprintf(MSS,"Правитель %s требует выкуп у правителя %s. Отношения: Politic: %d, Attitude: %d, CanKill: %d",arPlayer[Player].GetName(),arPlayer[PL].GetName(),GetPoliticalPower(Player,PL),arPlayer[Player].GetAttitude(PL), arPlayer[Player].Politic[PL].CanKill);
								FileLog(MSS);
								int Gold=0, Gem=0;
								if (arPlayer[Player].Gem*4>arPlayer[Player].Gold)
								{
									if (arPlayer[PL].Gold>=Ransom)
									{Gold=Ransom;}
									else
									{
										Gold=arPlayer[PL].Gold;
										Gem=(Ransom-arPlayer[PL].Gold)/3;
									};
								}
								else
								{
									if (arPlayer[PL].Gem>=Ransom/3)
									{Gem=Ransom/3;}
									else
									{
										Gem=arPlayer[PL].Gem;
										Gold=Ransom-arPlayer[PL].Gem*3;
									};
								};
								if (arPlayer[PL].Control==1)
								{
									arPlayer[Player].Politic[PL].Message.Type=3;
									arPlayer[Player].Politic[PL].Message.Trade1.bTrade=true;
									arPlayer[Player].Politic[PL].Message.Trade1.Gold=Gold;
									arPlayer[Player].Politic[PL].Message.Trade1.Gem=Gem;									
									MakeDipText(Player,PL,12,0);
								}
								else
								{
									arPlayer[Player].Politic[PL].arTalk[6][2]=CurGame.iCurrentTurn;
									if (GetPoliticalPower(Player,PL)>0)
									{
										arPlayer[Player].Gem+=Gem;
										arPlayer[Player].Gold+=Gold;
										arPlayer[PL].Gem-=Gem;
										arPlayer[PL].Gold-=Gold;									
										arPlayer[Player].Politic[PL].arTalk[6][1]=1;
										FileLog("Уплачен");
									}
									else
									{
										arPlayer[Player].Politic[PL].arTalk[6][1]=0;
										FileLog("Отказ");
									};
								};
							}
							else
							//Объявляем войну
							{
								sprintf(MSS,"Правитель %s объявляет войну правителю %s. Отношения: Politic: %d, Attitude: %d, CanKill: %d",arPlayer[Player].GetName(),arPlayer[PL].GetName(),GetPoliticalPower(Player,PL),arPlayer[Player].GetAttitude(PL), arPlayer[Player].Politic[PL].CanKill);
								FileLog(MSS);
								if (arPlayer[PL].Control==1)
								{
									arPlayer[Player].Politic[PL].Message.Type=1;
									arPlayer[Player].Politic[PL].Message.Treaty=1;
									arPlayer[Player].Politic[PL].Answer=0;
									MakeDipText(Player,PL,4,0);
								}
								else
								{
									arPlayer[Player].DeclareWar(PL);
								};
							};
							arPlayer[Player].Politic[PL].arTalk[1][2]=CurGame.iCurrentTurn;
						};
					};
					};
					//Предлагаем союз
					if ((bTreaty)&&(bUnion)&&(arPlayer[Player].GetUnionCount()==0)&&(arPlayer[PL].GetUnionCount()==0)&&(arPlayer[Player].GetAttitude(PL)>=50)&&(GetPoliticalPower(Player,PL)>100)&&(CurGame.iCurrentTurn-arPlayer[Player].Politic[PL].arTalk[3][2]>(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iDiplomacy))
					{
						if ((CurGame.iGameType==2)&&(Campaign.arDialog[103])&&((arPlayer[Player].ipProfile==3)||(arPlayer[PL].ipProfile==3)))
						{
							//не предлагать белезом, если договор с игроком
						}
						else
						{
						int Poss=900+(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iDiplomacy*10-arPlayer[Player].GetAttitude(PL)*3;
						if (arPlayer[PL].Control==1)
						{Poss+=arPlayer[Player].iDifficulty*20;}
						else
						{Poss-=arPlayer[Player].iDifficulty*20;};						
						if ((Poss<20)||(Random(Poss)<20))
						{
							sprintf(MSS,"Правитель %s предлагает союз правителю %s. Отношения: Politic: %d, Attitude: %d, CanKill: %d",arPlayer[Player].GetName(),arPlayer[PL].GetName(),GetPoliticalPower(Player,PL),arPlayer[Player].GetAttitude(PL), arPlayer[Player].Politic[PL].CanKill);
							FileLog(MSS);
							bUnion=false;
							bTreaty=false;
							if (arPlayer[PL].Control==1)
							{
								arPlayer[Player].Politic[PL].Message.Type=1;
								arPlayer[Player].Politic[PL].Message.Treaty=3;
								MakeDipText(Player,PL,6,0);
								arPlayer[Player].Politic[PL].arTalk[3][2]=CurGame.iCurrentTurn;
								arPlayer[Player].Politic[PL].arTalk[3][1]=0;
							}
							else
							{
								arPlayer[Player].Politic[PL].Treaty=3;
								arPlayer[PL].Politic[Player].Treaty=3;
								arPlayer[Player].Politic[PL].arTalk[3][2]=CurGame.iCurrentTurn;
								arPlayer[Player].Politic[PL].arTalk[3][1]=1;
								arPlayer[PL].Politic[Player].arTalk[3][2]=CurGame.iCurrentTurn;
								arPlayer[PL].Politic[Player].arTalk[3][1]=1;
							};
						};
						};
					};
					break;
				case 3:
					//Отмена союза
					if ((CurGame.iCurrentTurn-arPlayer[PL].Politic[Player].arTalk[3][2]>10)&&(CurGame.iCurrentTurn-arPlayer[Player].Politic[PL].arTalk[3][2]>10)&&
						((arPlayer[PL].GetUnionCount()>1)||(arPlayer[Player].GetAttitude(PL)<30)))
					{
						if ((CurGame.iGameType==2)&&(Campaign.arDialog[103])&&(arPlayer[Player].ipProfile==3)&&(PL==1))
						{
							//Оставляем союз для Белеза в кампании
						}
						else
						{
						sprintf(MSS,"Правитель %s отменяет союз с правителем %s. Отношения: Politic: %d, Attitude: %d, CanKill: %d",arPlayer[Player].GetName(),arPlayer[PL].GetName(),GetPoliticalPower(Player,PL),arPlayer[Player].GetAttitude(PL), arPlayer[Player].Politic[PL].CanKill);
						FileLog(MSS);
						bTreaty=false;
						if (arPlayer[PL].Control==1)
						{
							arPlayer[Player].Politic[PL].Message.Type=1;
							arPlayer[Player].Politic[PL].Message.Treaty=4;
							MakeDipText(Player,PL,11,0);
						}
						else
						{
							arPlayer[Player].Politic[PL].Treaty=2;
							arPlayer[PL].Politic[Player].Treaty=2;							
						};
						};
					};
					break;
				};
				if (arPlayer[Player].Politic[PL].Answer!=0)
				{bTreaty=false;};
				if (arPlayer[Player].Politic[PL].Treaty>1)
				{
					//Отмена торгового договора 
					if ((bTreaty)&&(arPlayer[Player].Politic[PL].Trade.bTrade))
					{
						bool bCancel=false, bCantTrade=false;
						//(при отказе от пересмотра договора)
						if ((arPlayer[Player].Politic[PL].arTalk[5][1]==0)&&
							(arPlayer[Player].Politic[PL].arTalk[5][2]>0)&&
							((arPlayer[Player].Politic[PL].arTalk[5][2]>arPlayer[PL].Politic[Player].arTalk[5][2])||(arPlayer[PL].Politic[Player].arTalk[5][1]==0)))
						{bCancel=true;};
						//(нет ресурса или лишний ресурс)
						for (int i=1; i<=iMaxResource; i++)
						{
							if (arPlayer[Player].GetResource(i)<1)
							{								
								for (int k=0; k<16; k++)
								{
									if (arPlayer[Player].Politic[PL].Trade.Stuff[k]==i)
									{
										bCancel=true;
										if (arPlayer[Player].GetResource(i)<0)
										{bCantTrade=true;};
									};
								};
							};						
							if (arPlayer[Player].GetResource(i)>1)
							{								
								for (int k=0; k<16; k++)
								{
									if (arPlayer[PL].Politic[Player].Trade.Stuff[k]==i)
									{bCancel=true;};
								};
							};
						};
						//(отрицательный доход)
						if ((arPlayer[Player].GoldInc<0)&&(arPlayer[Player].Politic[PL].Trade.Gold>0))
						{bCancel=true;};
						if ((arPlayer[Player].GemInc<0)&&(arPlayer[Player].Politic[PL].Trade.Gem>0))
						{bCancel=true;};
						if ((bCantTrade)||
							((bCancel)&&
							((CurGame.iCurrentTurn-arPlayer[Player].Politic[PL].arTalk[5][2]>10)||(arPlayer[Player].Politic[PL].arTalk[5][1]==0))&&
							((CurGame.iCurrentTurn-arPlayer[PL].Politic[Player].arTalk[5][2]>10)||(arPlayer[PL].Politic[Player].arTalk[5][1]==0))))

						{
							sprintf(MSS,"Правитель %s отменяет торговлю с правителем %s. Отношения: Politic: %d, Attitude: %d, CanKill: %d",arPlayer[Player].GetName(),arPlayer[PL].GetName(),GetPoliticalPower(Player,PL),arPlayer[Player].GetAttitude(PL), arPlayer[Player].Politic[PL].CanKill);
							FileLog(MSS);
							bTreaty=false;
							if (arPlayer[PL].Control==1)
							{
								arPlayer[Player].Politic[PL].Message.Type=4;							
								MakeDipText(Player,PL,10,0);
							}
							else
							{						
								arPlayer[Player].Politic[PL].Trade.Clear();
								arPlayer[PL].Politic[Player].Trade.Clear();
							};
						};
					};

					//Пересмотр торговли
					if ((bTreaty)&&(arPlayer[Player].Politic[PL].Trade.bTrade)&&(CurGame.iCurrentTurn-arPlayer[Player].Politic[PL].arTalk[5][2]>10)&&
						((CurGame.iCurrentTurn-arPlayer[PL].Politic[Player].arTalk[5][2]>10)||(arPlayer[PL].Politic[Player].arTalk[5][1]==0)))
					{
						int Give=arPlayer[Player].Politic[PL].Trade.Gold+arPlayer[Player].Politic[PL].Trade.Gem*5/2;
						int Take=arPlayer[PL].Politic[Player].Trade.Gold+arPlayer[PL].Politic[Player].Trade.Gem*5/2;
						for (int i=0; i<16; i++)
						{
							if (arPlayer[Player].Politic[PL].Trade.Stuff[i]>0)
							{Give+=GetResPrice(arPlayer[Player].Politic[PL].Trade.Stuff[i]);};
							if (arPlayer[PL].Politic[Player].Trade.Stuff[i]>0)
							{Take+=GetResPrice(arPlayer[PL].Politic[Player].Trade.Stuff[i]);};
						};
						if (Give>Take+10)
						{
							bTreaty=false;
							sprintf(MSS,"Правитель %s предлагает пересмотр торговли правителю %s. Отношения: Politic: %d, Attitude: %d, CanKill: %d",arPlayer[Player].GetName(),arPlayer[PL].GetName(),GetPoliticalPower(Player,PL),arPlayer[Player].GetAttitude(PL), arPlayer[Player].Politic[PL].CanKill);
							FileLog(MSS);
							int Gold1=arPlayer[Player].Politic[PL].Trade.Gold, Gold2=arPlayer[PL].Politic[Player].Trade.Gold,
								Gem1=arPlayer[Player].Politic[PL].Trade.Gem, Gem2=arPlayer[PL].Politic[Player].Trade.Gem,
								Debt=Give-Take;
							if (Gold1>0)
							{
								Gold1-=Debt;
								if (Gold1<0)
								{
									Debt=abs(Gold1);
									Gold1=0;
								}
								else
								{Debt=0;};
							};
							if ((Debt>0)&&(Gem1>0))
							{
								Gem1-=Debt*2/5;
								if (Gem1<0)
								{
									Debt=abs(Gem1)*5/2;
									Gem1=0;
								}
								else
								{Debt=0;};
							};
							if ((Debt>0)&&(Gold2>0))
							{
								Gold2+=Debt;
								Debt=0;								
							};
							if ((Debt>0)&&(Gem2>0))
							{
								Gem2+=Debt*2/5;
								Debt=0;								
							};
							if (arPlayer[PL].Control==1)
							{
								arPlayer[Player].Politic[PL].Message.Type=2;
								arPlayer[Player].Politic[PL].Message.Trade.Gold=Gold1;
								arPlayer[Player].Politic[PL].Message.Trade1.Gold=Gold2;
								arPlayer[Player].Politic[PL].Message.Trade.Gem=Gem1;
								arPlayer[Player].Politic[PL].Message.Trade1.Gem=Gem2;
								for (int i=0; i<16; i++)
								{
									arPlayer[Player].Politic[PL].Message.Trade.Stuff[i]=arPlayer[Player].Politic[PL].Trade.Stuff[i];
									arPlayer[Player].Politic[PL].Message.Trade1.Stuff[i]=arPlayer[PL].Politic[Player].Trade.Stuff[i];
								};
								MakeDipText(Player,PL,9,0);
								arPlayer[Player].Politic[PL].arTalk[5][1]=0;
							}
							else
							{	
								if ((arPlayer[PL].GoldInc>Gold2)&&(arPlayer[PL].GemInc>Gem2))
								{
									arPlayer[Player].Politic[PL].Trade.Gold=Gold1;
									arPlayer[PL].Politic[Player].Trade.Gold=Gold2;
									arPlayer[Player].Politic[PL].Trade.Gem=Gem1;
									arPlayer[PL].Politic[Player].Trade.Gem=Gem2;								
									arPlayer[Player].Politic[PL].arTalk[5][1]=1;
									FileLog("Принято");
								}
								else
								{
									arPlayer[Player].Politic[PL].arTalk[5][1]=0;
									FileLog("Отказ");
								};
							};
							arPlayer[Player].Politic[PL].arTalk[5][2]=CurGame.iCurrentTurn;
						};
					};
					//Предлагаем торговлю				
					if ((bTreaty)&&(arPlayer[Player].GetAttitude(PL)>=10)&&(CurGame.iCurrentTurn-arPlayer[Player].Politic[PL].arTalk[5][2]>(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iDiplomacy)&&(!arPlayer[Player].Politic[PL].Trade.bTrade))
					{						
						int Give=0, Take=0;
						for (int i=1; i<=iMaxResource; i++)
						{
							if ((arPlayer[Player].GetResource(i)>1)&&(arPlayer[PL].GetResource(i)==0))
							{Give=i;};
							if ((arPlayer[Player].GetResource(i)==0)&&(arPlayer[PL].GetResource(i)>1))
							{Take=i;};
						};
						if ((Take>0)||(Give>0))
						{
							sprintf(MSS,"Правитель %s предлагает торговлю правителю %s. Отношения: Politic: %d, Attitude: %d, CanKill: %d",arPlayer[Player].GetName(),arPlayer[PL].GetName(),GetPoliticalPower(Player,PL),arPlayer[Player].GetAttitude(PL), arPlayer[Player].Politic[PL].CanKill);
							FileLog(MSS);
							bTreaty=false;
							int Pay=0;
							if (Take>0)
							{
								Pay+=GetResPrice(Take);
							};
							if (Give>0)
							{
								Pay-=GetResPrice(Give);
							};
							int Gold=0, Gem=0;
							bool bWillTrade=true;
							if (Pay>0)
							{
								if (arPlayer[Player].GoldInc>20)
								{
									Gold=arPlayer[Player].GoldInc-20;
									if (Gold>Pay)
									{Gold=Pay;};
								};
								if (Gold<Pay)
								{
									if (arPlayer[Player].GemInc>10)
									{
										Gem=arPlayer[Player].GemInc-10;
										if (Gem*5>(Pay-Gold)*2)
										{Gem=(Pay-Gold)*2/5;};
									};
								};

								/*if (arPlayer[Player].GoldInc+arPlayer[Player].GemInc*5>Pay*4)
								{
									Gold=Pay*arPlayer[Player].GoldInc/(arPlayer[Player].GoldInc+arPlayer[Player].GemInc*5);
									Gem=Pay*arPlayer[Player].GemInc/(arPlayer[Player].GoldInc+arPlayer[Player].GemInc*5);
								};*/
								if ((Give==0)&&(Gold==0)&&(Gem==0))
								{bWillTrade=false;};
							};							
							if (Pay<0)
							{
								if (arPlayer[Player].GoldInc<arPlayer[Player].GemInc*5)
								{Gold=Pay;}
								else
								{Gem=Pay*2/5;};
							};
							if (bWillTrade)
							{
							if (arPlayer[PL].Control==1)
							{
								arPlayer[Player].Politic[PL].Message.Type=2;
								if (Gold>0){arPlayer[Player].Politic[PL].Message.Trade.Gold=Gold;};
								if (Gold<0){arPlayer[Player].Politic[PL].Message.Trade1.Gold=-Gold;};
								if (Gem>0){arPlayer[Player].Politic[PL].Message.Trade.Gem=Gem;};
								if (Gem<0){arPlayer[Player].Politic[PL].Message.Trade1.Gem=-Gem;};
								arPlayer[Player].Politic[PL].Message.Trade.Stuff[0]=Give;
								arPlayer[Player].Politic[PL].Message.Trade1.Stuff[0]=Take;
								MakeDipText(Player,PL,7,0);
								arPlayer[Player].Politic[PL].arTalk[5][1]=0;
							}
							else
							{
								if (((Gold>0)||(arPlayer[PL].GoldInc>abs(Gold)))&&((Gem>0)||(arPlayer[PL].GemInc>abs(Gem))))
								{
									arPlayer[Player].Politic[PL].Trade.bTrade=true;
									arPlayer[PL].Politic[Player].Trade.bTrade=true;
									if (Gold>0){arPlayer[Player].Politic[PL].Trade.Gold=Gold;};
									if (Gold<0){arPlayer[PL].Politic[Player].Trade.Gold=-Gold;};
									if (Gem>0){arPlayer[Player].Politic[PL].Trade.Gem=Gem;};
									if (Gem<0){arPlayer[PL].Politic[Player].Trade.Gem=-Gem;};
									arPlayer[Player].Politic[PL].Trade.Stuff[0]=Give;
									arPlayer[PL].Politic[Player].Trade.Stuff[0]=Take;
									arPlayer[Player].Politic[PL].arTalk[5][1]=1;
									FileLog("Принято");
								}
								else
								{
									arPlayer[Player].Politic[PL].arTalk[5][1]=0;
									FileLog("Отказ");
								};
							};
							arPlayer[Player].Politic[PL].arTalk[5][2]=CurGame.iCurrentTurn;
							};
						};
					};
					//Предлагаем обмен
					if ((bTreaty)&&(arPlayer[Player].GetAttitude(PL)>=0)&&(arPlayer[Player].arTreasury[TREASURY_SIZE-1][0]>0)&&(bExchange)&&
						(CurGame.iCurrentTurn-arPlayer[Player].Politic[PL].arTalk[4][2]>(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iDiplomacy)&&
						(arPlayer[Player].Politic[PL].arTalk[4][1]!=arPlayer[Player].arTreasury[TREASURY_SIZE-1][0]))
					{
						sprintf(MSS,"Правитель %s предлагает обмен правителю %s. Отношения: Politic: %d, Attitude: %d, CanKill: %d",arPlayer[Player].GetName(),arPlayer[PL].GetName(),GetPoliticalPower(Player,PL),arPlayer[Player].GetAttitude(PL), arPlayer[Player].Politic[PL].CanKill);
						FileLog(MSS);
						bTreaty=false;
						arPlayer[Player].Politic[PL].arTalk[4][1]=arPlayer[Player].arTreasury[TREASURY_SIZE-1][0];
						arPlayer[Player].Politic[PL].arTalk[4][2]=CurGame.iCurrentTurn;
						if (arPlayer[PL].Control==1)
						{
							arPlayer[Player].Politic[PL].Message.Type=3;							
							arPlayer[Player].Politic[PL].Message.Trade1.Gold=(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iGoldCost;
							arPlayer[Player].Politic[PL].Message.Trade1.Gem=(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iGemCost;
							arPlayer[Player].Politic[PL].Message.Trade.Stuff[8]=arPlayer[Player].arTreasury[TREASURY_SIZE-1][0];
							arPlayer[Player].Politic[PL].Message.arDurability[0]=arPlayer[Player].arTreasury[TREASURY_SIZE-1][1];
							MakeDipText(Player,PL,8,0);
							bExchange=false;
						}
						else
						{
							if ((!CanSell(PL,arPlayer[Player].arTreasury[TREASURY_SIZE-1][0]))
								&&(arPlayer[PL].Gold>(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iGoldCost)
								&&(arPlayer[PL].Gem>(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iGemCost))
							{
								int Slot=-1;
								for (int i=0; i<TREASURY_SIZE-1; i++)
								{
									if (arPlayer[PL].arTreasury[i][0]==0)
									{
										Slot=i;
										break;
									};
								};
								if (Slot>=0)
								{
									sprintf(MSS,"Обмен состоялся, предмет: %s",(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->sName);
									FileLog(MSS);
									arPlayer[Player].Gold+=(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iGoldCost;
									arPlayer[PL].Gold-=(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iGoldCost;
									arPlayer[Player].Gem+=(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iGemCost;
									arPlayer[PL].Gem-=-(pFirstItem+arPlayer[Player].arTreasury[TREASURY_SIZE-1][0])->iGemCost;
									arPlayer[PL].arTreasury[Slot][0]=arPlayer[Player].arTreasury[TREASURY_SIZE-1][0];
									arPlayer[PL].arTreasury[Slot][1]=arPlayer[Player].arTreasury[TREASURY_SIZE-1][1];
									arPlayer[Player].arTreasury[TREASURY_SIZE-1][0]=0;
									arPlayer[Player].arTreasury[TREASURY_SIZE-1][1]=0;
									bExchange=false;
								};
							};
						};						
					};


					//Предлагаем подарок
				//	int AAAA=(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro/2+((pFirstPlayerProfile+arPlayer[Player].ipProfile)->iDiplomacy-5)*3+Random(100+(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro);
					int AAAA=Random((pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro/2)+Random((pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro/2)+Random((pFirstPlayerProfile+arPlayer[Player].ipProfile)->iAggro/2)+
						Random(((pFirstPlayerProfile+arPlayer[Player].ipProfile)->iDiplomacy-5)*3)+Random(20);
					if ((AAAA>0)&&(arPlayer[Player].Politic[PL].Treaty==3))
					{AAAA/=2;};
					if (arPlayer[Player].GetAttitude(PL)<0)
					{AAAA+=Random(abs(arPlayer[Player].GetAttitude(PL)));}
					else
					{AAAA-=Random(arPlayer[Player].GetAttitude(PL));};
					if ((PL!=iWar)&&(bTreaty)&&(arPlayer[Player].GetAttitude(PL)>-10)&&(bExchange)&&(GetPoliticalPower(Player,PL)>AAAA)&&
						(CurGame.iCurrentTurn-arPlayer[Player].Politic[PL].arTalk[7][2]>(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iDiplomacy)&&
						(CurGame.iCurrentTurn-arPlayer[Player].Politic[PL].arTalk[6][2]>10))
					{
						sprintf(MSS,"Правитель %s предлагает дар правителю %s. Отношения: Politic: %d, Attitude: %d, CanKill: %d, AAAA: %d",arPlayer[Player].GetName(),arPlayer[PL].GetName(),GetPoliticalPower(Player,PL),arPlayer[Player].GetAttitude(PL), arPlayer[Player].Politic[PL].CanKill,AAAA);
						FileLog(MSS);
						/*if (PL==1)
						{
							char MSS[100];
							sprintf(MSS,"%s: %d > %d",arPlayer[Player].GetName(),GetPoliticalPower(Player,PL),AAAA);
							FileLog(MSS);
						};*/
						int Item=0, Gold=0, Gem=0;						
						if ((arPlayer[Player].arTreasury[TREASURY_SIZE-1][0]>0)&&
							(arPlayer[Player].Politic[PL].arTalk[7][1]!=arPlayer[Player].arTreasury[TREASURY_SIZE-1][0]))
						{
							Item=arPlayer[Player].arTreasury[TREASURY_SIZE-1][0];
						};
						if (arPlayer[Player].Gold>GoldLimit)
						{
							Gold=GetMin(100+CurGame.iCurrentTurn*2+Random(50),arPlayer[Player].Gold-GoldLimit);
						};
						if (arPlayer[Player].Gem>GemLimit)
						{
							Gem=GetMin(20+CurGame.iCurrentTurn/5+Random(10),arPlayer[Player].Gem-GemLimit);
						};
						if (Item+Gold+Gem>0)
						{
							arPlayer[Player].Politic[PL].arTalk[7][2]=CurGame.iCurrentTurn;
							arPlayer[Player].Politic[PL].arTalk[7][1]=Item;
							bTreaty=false;
							bExchange=false;
							if (arPlayer[PL].Control==1)
							{
								arPlayer[Player].Politic[PL].Message.Type=3;
								arPlayer[Player].Politic[PL].Message.Trade.Gold=Gold;
								arPlayer[Player].Politic[PL].Message.Trade.Gem=Gem;
								if (Item>0)
								{
									arPlayer[Player].Politic[PL].Message.Trade.Stuff[8]=arPlayer[Player].arTreasury[TREASURY_SIZE-1][0];
									arPlayer[Player].Politic[PL].Message.arDurability[0]=arPlayer[Player].arTreasury[TREASURY_SIZE-1][1];
								};
								MakeDipText(Player,PL,13,0);								
							}
							else
							{
								arPlayer[Player].Gold-=Gold;
								arPlayer[PL].Gold+=Gold;
								arPlayer[Player].Gem-=Gem;
								arPlayer[PL].Gem+=-Gem;
								if (Item>0)
								{
									int Slot=-1;
									for (int i=0; i<TREASURY_SIZE-1; i++)
									{
										if (arPlayer[PL].arTreasury[i][0]==0)
										{
											Slot=i;
											break;
										};
									};
									if (Slot>=0)
									{
										arPlayer[PL].arTreasury[Slot][0]=arPlayer[Player].arTreasury[TREASURY_SIZE-1][0];
										arPlayer[PL].arTreasury[Slot][1]=arPlayer[Player].arTreasury[TREASURY_SIZE-1][1];
										arPlayer[Player].arTreasury[TREASURY_SIZE-1][0]=0;
										arPlayer[Player].arTreasury[TREASURY_SIZE-1][1]=0;										
									};
								};
							};
						};
					};
				};
			};
		};
	};
};

void AlchemyAI(int Player)
{
	if (arPlayer[Player].arBld[103])
	{
		if ((arPlayer[Player].Gold<100)&&(arPlayer[Player].Gem>2000))
		{
			arPlayer[Player].Gold+=200;
			arPlayer[Player].Gem-=200;
		};
		if ((arPlayer[Player].Gold>10000)&&(arPlayer[Player].Gem<50))
		{
			arPlayer[Player].Gold-=1000;
			arPlayer[Player].Gem+=40;
		};
	};
};

void MakeTurnAI(int Player, int Progress)
{
	/*bHourGlass=false;
	for (int i=1; i<=iMaxDefender; i++)
	{
		textprintf(screen,Font1,i/35*300,(i%35)*20,COLOR_2,"%s: %d",(pFirstDefender+i)->sName,GetDefenderPower(i));
		textprintf(screen1,Font1,i/35*300,(i%35)*20,COLOR_2,"%s: %d",(pFirstDefender+i)->sName,GetDefenderPower(i));		
	};
	clear_keybuf();
	readkey();*/

	/*bHourGlass=false;
	rectfill(screen,0,0,1024,768,COLOR_1);
	rectfill(screen1,0,0,1024,768,COLOR_1);
	textprintf(screen,Font1,310,10,COLOR_2,arPlayer[Player].GetName());
	textprintf(screen1,Font1,310,10,COLOR_2,arPlayer[Player].GetName());
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if ((i!=Player)&&(arPlayer[i].bInGame))
		{
			textprintf(screen,Font1,300,40+i*20,COLOR_2,"%s: %d",arPlayer[i].GetName(),GetPoliticalPower(Player,i));
			textprintf(screen1,Font1,300,40+i*20,COLOR_2,"%s: %d",arPlayer[i].GetName(),GetPoliticalPower(Player,i));
		};
	};
	clear_keybuf();
	readkey();*/

	char stLog[200];
	sprintf (stLog,"%s ходит...",arPlayer[Player].GetName());
	DFileLog(stLog);
	if (arPlayer[Player].Control==3)
	{arPlayer[Player].iKarma=0;};

	//Добавляем договора с разными стражами
	if (
		(CurGame.iGameType==2)&&
		(CurGame.iCurrentTurn%11==10))
	{
		int Power=1+CurGame.iCurrentTurn/100;
		if (Power>1+Campaign.iCurrentTurn/10)
		{Power=1+Campaign.iCurrentTurn/10;};
		if (Power>3)
		{Power=3;};
		int Karma=0;
		if (abs(arPlayer[Player].iKarma)>=50)
		{Karma=sign(arPlayer[Player].iKarma);};
		for (int i=1; i<=iMaxDefender; i++)
		{
			if ((*(arPlayer[Player].pDefender+i)==0)&&((pFirstDefender+i)->iPower==Power)&&
				((Karma==0)||((pFirstDefender+i)->iKarma==0)||(sign((pFirstDefender+i)->iKarma)==Karma)))
			{*(arPlayer[Player].pDefender+i)+=1;};
		};
	};
	
	int TargetProgress=NewProgress+Progress;
	if (Tutorial.Map)
	{return;};
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{arPlayer[Player].Politic[i].CanKill=-10;};
	bExpand=true;
	MakeFog(Player);
	AlchemyAI(Player);
	GoldLimit=50+CurGame.iCurrentTurn;
	GemLimit=20+CurGame.iCurrentTurn/5;

	//Обновление массива вражеских героев-охранников	
	for (int j=0; j<MAX_HERO; j++)
	{
		if ((arPlayer[Player].arHero[j])&&(arPlayer[Player].arHero[j]->iDead<0)&&(arPlayer[Player].arHero[j]->iCamper>0)&&(arPlayer[Player].arHero[j]->iCampTurn>0))
		{
			for (int i=0; i<MAX_CAMPERS; i++)
			{		
				if (arPlayer[Player].arCamper[i]==0)
				{
					arPlayer[Player].arCamper[i]=arPlayer[Player].arHero[j]->iCamper;
					break;
				};
			};
			arPlayer[Player].arHero[j]->iCamper=0;
			arPlayer[Player].arHero[j]->iCampTurn=0;
		};
	};
	bool bCamp;
	for (int i=0; i<MAX_CAMPERS; i++)
	{		
		if (arPlayer[Player].arCamper[i]>0)
		{
			bCamp=true;
			int Owner=(pFirstProvince+arPlayer[Player].arCamper[i])->ipPlayer;
			if ((Owner>0)&&(Owner!=Player))
			{
				for (int j=0; j<MAX_HERO; j++)
				{
					if ((arPlayer[Owner].arHero[j])&&(arPlayer[Owner].arHero[j]->iDead<0)&&(arPlayer[Owner].arHero[j]->ipProvince==arPlayer[Player].arCamper[i]))
					{
						bCamp=false;
						break;
					};
				};
			};
			if (bCamp)
			{arPlayer[Player].arCamper[i]=0;};			
		};
	};
	

	if (arPlayer[Player].Control==2)
	{
		if ((arPlayer[Player].ipProfile==13)&&(CurGame.iCurrentTurn==50-arPlayer[Player].iDifficulty*5))
		{
			if ((pFirstProvince+arPlayer[Player].ipProvince)->arSiteGuard[0])
			{
				(pFirstProvince+arPlayer[Player].ipProvince)->DismissSiteGuard(0);
			};
		};
		if (arPlayer[Player].AIUnion[1]>0)
		{arPlayer[Player].AIUnion[1]--;};
		if (arPlayer[Player].iDifficulty>0)
		{
			switch (arPlayer[Player].ipProfile)
			{
			case 1:
			case 16:
				if ((CurGame.iCurrentTurn+1)%(150-arPlayer[Player].iDifficulty*10)==0)
				{*(arPlayer[Player].pDefender+58)+=1;};
				break;
			case 2:
			case 15:
				if ((CurGame.iCurrentTurn+1)%(150-arPlayer[Player].iDifficulty*10)==0)
				{*(arPlayer[Player].pDefender+50)+=1;};
				break;
			case 3:
				if ((CurGame.iCurrentTurn+1)%(150-arPlayer[Player].iDifficulty*10)==0)
				{*(arPlayer[Player].pDefender+48)+=1;};
				break;
			case 4:			
				if ((CurGame.iCurrentTurn+1)%(150-arPlayer[Player].iDifficulty*10)==0)
				{*(arPlayer[Player].pDefender+59)+=1;};
				break;
			case 5:
			case 12:
				if ((CurGame.iCurrentTurn+1)%(150-arPlayer[Player].iDifficulty*10)==0)
				{*(arPlayer[Player].pDefender+46)+=1;};
				break;
			case 6:
				if ((CurGame.iCurrentTurn+1)%(150-arPlayer[Player].iDifficulty*10)==0)
				{*(arPlayer[Player].pDefender+47)+=1;};
				break;
			case 7:
				if ((CurGame.iCurrentTurn+1)%(150-arPlayer[Player].iDifficulty*10)==0)
				{*(arPlayer[Player].pDefender+57)+=1;};
				break;			
			case 8:
			case 10:
				if ((CurGame.iCurrentTurn+1)%(120-arPlayer[Player].iDifficulty*10)==0)
				{*(arPlayer[Player].pDefender+56)+=1;};
				break;
			case 11:
				if ((CurGame.iCurrentTurn+1)%(150-arPlayer[Player].iDifficulty*10)==0)
				{*(arPlayer[Player].pDefender+53)+=1;};
				break;
			case 13:
				if ((CurGame.iCurrentTurn+1)%(120-arPlayer[Player].iDifficulty*10)==0)
				{*(arPlayer[Player].pDefender+55)+=1;};
				break;
			case 9:
			case 14:
				if ((CurGame.iCurrentTurn+1)%(150-arPlayer[Player].iDifficulty*10)==0)
				{*(arPlayer[Player].pDefender+60)+=1;};
				break;
			};
		};
	};
	if (arPlayer[Player].Gold<0)
	{arPlayer[Player].Gold=-1;};
	if (arPlayer[Player].Gem<0)
	{arPlayer[Player].Gem=-1;};

	bCanSpend=AI_RecruitHero(Player);
	if (arPlayer[Player].Control==2)
	{arPlayer[Player].iKarma=(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iKarma;};
	if ((CurGame.iCurrentTurn<=10)&&(bCanSpend))		
	{
		WriteLog("Строительство в замке (начало игры)");
		AI_InnerBuild(Player);
		if (arPlayer[Player].Control==2)
		{arPlayer[Player].iKarma=(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iKarma;};
	};
	WriteLog("Ритуалы лечения");
	RitualHealUnits(Player);
	for (int Hero=0; Hero<MAX_HERO; Hero++)
	{
		char LN[200];		
		if ((arPlayer[Player].arHero[Hero])&&(arPlayer[Player].arHero[Hero]->iDead<0))
		{
			sprintf (LN,"Герой %s, торговля", arPlayer[Player].arHero[Hero]->pstName->str);
			WriteLog(LN);
			HireUnits(Player,Hero);
			LearnSpells(Player,Hero);
			EquipItems(Player,Hero);
			arPlayer[Player].arHero[Hero]->NiB++;
		//	ChooseTarget(Player,Hero);
		};
	};
	if (arPlayer[Player].Control==2)
	{arPlayer[Player].iKarma=(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iKarma;};
	DFileLog("Раздача приказов героям");
	HeroTaskmaster(Player, Progress);
	DFileLog("Заполнение гарнизонов               ");
	FillGarrison(Player);
	if ((arPlayer[Player].iDifficulty==0)&&(CurGame.iCurrentTurn%2==0))
	{bCanSpend=false;};
	if ((arPlayer[Player].iDifficulty==1)&&(CurGame.iCurrentTurn%3==0))
	{bCanSpend=false;};
	if (bCanSpend)
	{
		if (arPlayer[Player].Control==2)
		{arPlayer[Player].iKarma=(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iKarma;};
		FileLog("Установка стражей               ");
		AIPlaceDefender(Player);
		if (arPlayer[Player].Control==2)
		{arPlayer[Player].iKarma=(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iKarma;};
		if (CurGame.iCurrentTurn>10)
			//&&((Random(100)>50)||(arPlayer[Player].Gold>=1000+CurGame.iCurrentTurn*10)))		
		{
			DFileLog("Строительство в замке          ");
			AI_InnerBuild(Player);
		};
		if (arPlayer[Player].Control==2)
		{arPlayer[Player].iKarma=(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iKarma;};
		if (Random(100)>60)
		{
			DFileLog("Внешние постройки                 ");
			AIOuterBuild(Player);
		};
	};
	CastRitualAI(Player);
	if ((arPlayer[Player].Control==2)&&(CurGame.bDiplomacy))
	{MakeDiplomacyAI(Player);};
	NewProgress=TargetProgress;
	DFileLog("Ход окончен");
//	StopLog("Ход окончен, Игрок ", Player);
};
