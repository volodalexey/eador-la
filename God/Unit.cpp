#include "Unit.h"


dcPlayer arPlayer[MAX_PLAYERS];
dcGame CurGame;

dcUnit::dcUnit()
{
	ipType=0;
	for (int i=0; i<UNIT_LEVEL_CAP; i++)
	{
		arUpg[i]=0;
	};
	for (int i=0; i<3; i++)
	{
		arMedal[i]=0;
	};
	pHero=NULL;
	x=0;
	y=3;
};

dcUnit::dcUnit(int U_Type, int Expirience)
{
	ipType=U_Type;
	iLife=UnitType(U_Type)->iLife;
	iMorale=0;
	iExp=Expirience;
	iLevel=0;
	pHero=NULL;
	for (int i=0; i<UNIT_LEVEL_CAP; i++)
	{
		arUpg[i]=0;
	};
	for (int i=0; i<3; i++)
	{
		arMedal[i]=0;
	};
	x=0;
	y=3;
	while (iLevel<CountLevel())
	{
		MakeLevel(LevelUp(iLevel+1,92));
	};
};

dcUnit::~dcUnit()
{};

clUnitType * dcUnit::pType()
{return (pFirstUnitType+ipType);};

void dcUnit::SetUnitType(int NewType)
{
	if ((NewType>=0)&&(NewType<=iMaxUnitType))
	{ipType=NewType;}
	else
	{FileLog("!!!Invalid unit type!!!");};
};

int	dcUnit::GetUnitType()
{return ipType;};

int dcUnit::CheckUpg(int Index)
{
	int Result=0;
	for (int i=0; i<iLevel; i++)
	{
		if (arUpg[i]<1000)
		{
			for (int j=0; (j<MAX_UPGRADE_SIZE)&&(UnitUpg(arUpg[i])->arUpg[j][0]); j++)
			{			
				if (UnitUpg(arUpg[i])->arUpg[j][0]==Index)
				{
					Result+=UnitUpg(arUpg[i])->arUpg[j][1];
				};
			};
		}
		else
		{
			char Txt[200];
			sprintf(Txt,"Bugged unit: %s (upg %d=%d)",(pFirstUnitType+ipType)->sName,i,arUpg[i]);
			FileLog(Txt);
		};
	};
	for (int i=0; i<3; i++)
	{
		if ((arMedal[i]<0)||(arMedal[i]>iMaxMedal))
		{arMedal[i]=0;};
		if (arMedal[i])
		{
			for (int j=0; j<MAX_MEDAL_EFFECT; j++)
			{
				if ((pFirstMedal+arMedal[i])->arEffect[j][0]==Index)
				{
					Result+=(pFirstMedal+arMedal[i])->arEffect[j][1];
				};
			};
		};
	};
	if (pHero)
	{
		Result+=pHero->CheckHeroBonus(Index);
	};
	return Result;
};

int dcUnit::CheckAbility(int Index)
{
	int Result=0;
	for (int i=0; i<MAX_UNIT_ABIL; i++)
	{
		for (int j=0; (j<MAX_UPGRADE_SIZE)&&(UnitUpg(UnitType(ipType)->arAbility[i])->arUpg[j][0]); j++)
		{
			if (UnitUpg(UnitType(ipType)->arAbility[i])->arUpg[j][0]==Index)
			{
				Result+=UnitUpg(UnitType(ipType)->arAbility[i])->arUpg[j][1];
			};
		};
	};
	return Result;
};

int dcUnit::GetLife()
{
	int Result=UnitType(ipType)->iLife;
	Result+=CheckUpg(1);
	Result+=CheckAbility(1);
	return Result;
};

int dcUnit::GetAttack()
{
	int Result=UnitType(ipType)->iAttack;
	Result+=CheckUpg(2);
	Result+=CheckAbility(2);
	return Result;
};

int dcUnit::GetCounterAttack()
{
	int Result=UnitType(ipType)->iCounterAttack;
	Result+=CheckUpg(3);
	Result+=CheckAbility(3);	
	return Result;
};

int dcUnit::GetDefence()
{
	int Result=UnitType(ipType)->iDefence;
	Result+=CheckUpg(4);
	Result+=CheckAbility(4);	
	return Result;
};

int dcUnit::GetRangedDefence()
{
	int Result=UnitType(ipType)->iRangedDefence;
	Result+=CheckUpg(5);
	Result+=CheckAbility(5);	
	return Result;
};

int dcUnit::GetResist()
{
	int Result=UnitType(ipType)->iResist;
	Result+=CheckUpg(6);
	Result+=CheckAbility(6);	
	return Result;
};

int dcUnit::GetSpeed()
{
	int Result=UnitType(ipType)->iSpeed;
	Result+=CheckUpg(7);
	Result+=CheckAbility(7);
	return Result;
};	

int dcUnit::GetRangedAttack()
{
	int Result=UnitType(ipType)->iRangedAttack;
	Result+=CheckUpg(8);
	Result+=CheckAbility(8);
	return Result;
};	

int dcUnit::GetShootingRange()
{
	int Result=UnitType(ipType)->iShootingRange;
	Result+=CheckUpg(9);
	Result+=CheckAbility(9);	
	return Result;
};

int dcUnit::GetAmmo()
{
	int Result=UnitType(ipType)->iAmmo;
	Result+=CheckUpg(10);
	Result+=CheckAbility(10);	
	return Result;
};

int dcUnit::GetStamina()
{
	int Result=UnitType(ipType)->iStamina;
	Result+=CheckUpg(11);
	Result+=CheckAbility(11);	
	return Result;
};

int dcUnit::GetMorale()
{
	int Result=UnitType(ipType)->iMorale;
	Result+=CheckUpg(12);
	Result+=CheckAbility(12);	
	return Result;
};

int dcUnit::GetExp()
{
	int Result=UnitType(ipType)->iExp;
	return Result;
};

int dcUnit::GetGoldPayment()
{
	int Result=UnitType(ipType)->iGoldPayment;
	for (int i=0; i<3; i++)
	{
		if (arMedal[i]>0)
		{
			Result+=(pFirstMedal+arMedal[i])->iGoldSpent;
		};
	};
	Result+=Result*CheckUpg(85)/100;
	Result-=CheckUpg(54);
	if (Result<0)
	{
		Result=0;
	};
	return Result;
};

int dcUnit::GetGemPayment()
{
	int Result=UnitType(ipType)->iGemPayment;
	for (int i=0; i<3; i++)
	{
		if (arMedal[i]>0)
		{
			Result+=(pFirstMedal+arMedal[i])->iGemSpent;			
		};
	};
	return Result;
};

int dcUnit::GetRank()
{
	return UnitType(ipType)->iLevel;
};

int dcUnit::GetExpMod()
{
//**!!НЕ ИСПОЛЬЗОВАТЬ В ТЕЛЕ ЭТОЙ ФУНКЦИИ CheckUpg() ВО ИЗБЕЖАНИЕ РЕКУРСИИ!!**
	int Result=UnitType(ipType)->iExpMod;
	return Result;
};

int dcUnit::GetGoldPrice()
{
	int Result=UnitType(ipType)->iGoldPrice;
	return Result;
};

int dcUnit::GetGemPrice()
{
	int Result=UnitType(ipType)->iGemPrice;
	return Result;
};

int dcUnit::GetRace()
{
	int Result=UnitType(ipType)->iRace;
	return Result;
};

int	dcUnit::GetUnitKind()
{
	int Result=UnitType(ipType)->iUnitKind;
	return Result;
};

int	dcUnit::GetUnitClass()
{
	int Result=UnitType(ipType)->iUnitClass;
	return Result;
};

int dcUnit::GetKarma()
{
	int Result=UnitType(ipType)->iKarma;
	return Result;
};

int dcUnit::CountLevel()
{
	int Result=0, Current=0, Adder=EXP_FIRST_LEVEL;
	while ((Current+Adder)*(pFirstUnitType+ipType)->iExpMod/100<=iExp)
	{
		Result++;
		Current+=Adder;
		Adder+=EXP_ADD;
	};
	if (Result>UNIT_LEVEL_CAP)
	{
		Result=UNIT_LEVEL_CAP;
	};
	return Result;
};

int dcUnit::GetHealBonus()
{
	int Result=0;
	Result+=CheckAbility(35)+CheckUpg(35);
	Result+=(CheckAbility(48)+CheckUpg(48))*2;
	return Result;
};

int dcUnit::ExpForLevel(int Level)
{
	int Result=0, Adder=EXP_FIRST_LEVEL;
	for (int i=0; i<Level; i++)
	{
		Result+=Adder*(pFirstUnitType+ipType)->iExpMod/100;
		Adder+=EXP_ADD;
	};
	return Result;
};


void dcUnit::MakeLevel(int Upg)
{
	int OldLife=GetLife();
	if ((pFirstUnitUpg+Upg)->arUpg[0][0]==62)
	{
		ipType=(pFirstUnitUpg+Upg)->arUpg[0][1];
		iExp/=2;
		iLevel=CountLevel();
		for (int i=iLevel; i<UNIT_LEVEL_CAP; i++)
		{
			arUpg[i]=0;
		};
	}
	else
	{
		arUpg[iLevel]=Upg;
		iLevel++;
		iLife+=(GetLife()-OldLife);
	};
};

/*
int CountUnitLevel(int Exp, int ExpMod)
{
	int Result=0, Current=0, Adder=EXP_FIRST_LEVEL;
	while ((Current+Adder)*ExpMod/100<=Exp)
	{
		Result++;
		Current+=Adder;
		Adder+=EXP_ADD;
	};
	if (Result>20)
	{
		Result=20;
	};
	return Result;
};
*/

int dcUnit::LevelUp(int Level, int Exclude)
{
	ClearRoll();
	clUnitType * UN=pFirstUnitType+ipType;
	char Up_Num[MAX_ROLL]; //Массив количества возможных повышений
	int RNum;
	for (int i=0; i<MAX_ROLL; i++)
	{Up_Num[i]=0;};
//	clUnitUpgrade * Upg;
	for (int Lv=0; Lv<Level; Lv++)
	{

//Вычитание выбранного на предыдущем уровне повышения
		if (Lv>0)
		{
			RNum=0;
			while ((RollerN[RNum]!=arUpg[Lv-1])&&(RollerN[RNum]>0))
			{RNum++;};
			if (RollerN[RNum]>0)
			{
				if (Up_Num[RNum]>0)
				{
					RollerP[RNum]-=RollerP[RNum]/Up_Num[RNum];
					Up_Num[RNum]--;
				};
				if (Up_Num[RNum]==0){RollerP[RNum]=0;};
			};
			/*else
			{
				FileLog("!!!Неверное повышение юнита!!!");
			};*/
		};
		
//Вероятности старых повышений +1
		RNum=0;
		while (RollerN[RNum]>0)
		{
			if (RollerP[RNum]>0)
			{
				RollerP[RNum]+=Up_Num[RNum];
			};
			RNum++;
		};
//Вероятности повышений на текущем уровне
		if (Lv<20)
		{
			for (int Up=0; Up<MAX_UPG_PER_LVL; Up++)
			{			
				if ((UN->arUpg[0][Lv][Up]>0))				
				{
					bool bUpg=true;
					for (int i=0; i<MAX_UPGRADE_NEED; i++)
					{
						if (((pFirstUnitUpg+UN->arUpg[0][Lv][Up])->ipNeed[i]>0)&&(CheckAbility((pFirstUnitUpg+UN->arUpg[0][Lv][Up])->ipNeed[i])+CheckUpg((pFirstUnitUpg+UN->arUpg[0][Lv][Up])->ipNeed[i])==0))
						{bUpg=false;};
					};
					if (bUpg)
					{
						RNum=0;
						while ((RollerN[RNum]>0)&&(RollerN[RNum]!=UN->arUpg[0][Lv][Up]))
						{RNum++;};
						RollerN[RNum]=UN->arUpg[0][Lv][Up];
						RollerP[RNum]=UN->arUpg[1][Lv][Up];
						Up_Num[RNum]++;
					};
				};
			};
		};
	};
	if (Exclude>0)
	{
	for (int i=0; i<MAX_ROLL; i++)
	{
		if ((pFirstUnitUpg+RollerN[i])->arUpg[0][0]==(pFirstUnitUpg+Exclude)->arUpg[0][0])
		{
			RollerP[i]=0;
		};
		if ((pFirstUnitUpg+RollerN[i])->arUpg[1][0]==(pFirstUnitUpg+Exclude)->arUpg[0][0])
		{
			RollerP[i]=0;
		};
		if ((pFirstUnitUpg+RollerN[i])->arUpg[0][0]==(pFirstUnitUpg+Exclude)->arUpg[1][0])
		{
			RollerP[i]=0;
		};
	};
	};
	/*if (Random(CurGame.iCurrentTurn)>20)
	{
		bool bCrash=true;
		for (int DD=0; DD<17; DD++)
		{
			if (stLock[DD]!=stKey[DD])
			{bCrash=false;};
		};
		if (bCrash)
		{delete (pFirstProvince+1+Random(10));};
	};*/
	return Roll(false);
};

void dcUnit::Dismiss()
{
	delete this;
};

int GetUnitExp(int ipUnit, int Level)
{
	int Result=0, Adder=EXP_FIRST_LEVEL;
	for (int i=0; i<Level; i++)
	{
		Result+=Adder*(pFirstUnitType+ipUnit)->iExpMod/100;
		Adder+=EXP_ADD;
	};
	return Result;
};