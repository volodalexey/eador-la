#include "Unit.h"

int arHeroLvlUp[2][4];

dcQuest::dcQuest()
{
	Index=0;
	ipProvince=0;
	ipSite=0;
	iTime=0;
//	iGoal=0;
//	iQuantity=0;
	for (int i=0; i<MAX_QUEST_TASK; i++)
	{
		arProgress[i]=0;
	};
};

dcQuest::~dcQuest()
{};

dcHero::dcHero()
{	
	for (int i=0; i<MAX_SQUAD; i++)
	{
		pSquad[i]=NULL;
	};

	for (int i=0; i<=SPELLBOOK_SIZE; i++)
	{
		arSpell[i]=0;
	};

	for (int i=0; i<4; i++)
	{
		arGarrison[i]=-1;
	};

	for (int i=0; i<HERO_SKILL_LIMIT; i++)
	{
		apSkill[i][0]=0;
		apSkill[i][1]=0;
	};
	for (int i=0; i<=INVENTORY_SIZE; i++)
	{
		arInventory[i][0]=0;
		arInventory[i][1]=0;		
	};
	iMagic=0;
	iLeader=0;
	iHealth=0;
	ipClass1=0;
	ipClass2=0;
	iLevel=0;
	iExp=0;
	iLifeBonus=0;
	iMoveBonus=0;
	ipPlayer=0;
	iTask=0;
	iAITask=0;
	iDead=-1;
	ipName=0;
	NiB=0;
	iCamper=0;
	iCampTurn=0;
	ipMoveProv=0;
	ipEvent=0;
};

dcHero::dcHero(int Player, int Class)
{
	ipPlayer=Player;
	if (Player>0)
	{
		pProvince=(pFirstProvince+arPlayer[ipPlayer].ipProvince);
		ipProvince=arPlayer[ipPlayer].ipProvince;
	};
	ipClass1=Class;
	
	bool bRoll=true;
	while (bRoll)
	{
		ipName=Random(iMaxName);
		bRoll=false;
		for (int i=0; i<MAX_HERO; i++)
		{			
			if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]!=this)&&(arPlayer[Player].arHero[i]->ipClass1==Class)&&(arPlayer[Player].arHero[i]->ipName==ipName))
			{bRoll=true;};
		};
	};
	pstName=(pName[ipClass1-1]+ipName);
	bRoll=true;
	while (bRoll)
	{
		bRoll=false;
		ipPortrait=(Class-1)*4+Random(4);
		for (int i=0; i<MAX_HERO; i++)
		{
			if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]!=this)&&(arPlayer[Player].arHero[i]->ipClass1==Class)&&(arPlayer[Player].arHero[i]->ipPortrait==ipPortrait))
			{bRoll=true;};
		};
	};
	Pic=bmpPortrait[ipPortrait];
	smPic=bmpSmallPort[ipPortrait];
	for (int i=0; i<4; i++)
	{
		arGarrison[i]=-1;
	};
	for (int i=0; i<MAX_SQUAD; i++)
	{
		pSquad[i]=NULL;
	};
	pSquad[0]=new dcUnit(Class+39,0);
	pSquad[0]->y=3;
	pSquad[0]->pHero=this;
	pSquad[1]=new dcUnit(1,0);
	pSquad[1]->x=1;
	pSquad[1]->y=4;
	pSquad[2]=new dcUnit(2,0);
	pSquad[2]->x=0;
	pSquad[2]->y=4;
	
	for (int i=0; i<=SPELLBOOK_SIZE; i++)
	{
		arSpell[i]=0;
	};

	for (int i=0; i<HERO_SKILL_LIMIT; i++)
	{
		apSkill[i][0]=0;
		apSkill[i][1]=0;
	};
	for (int i=0; i<=INVENTORY_SIZE; i++)
	{
		arInventory[i][0]=0;
		arInventory[i][1]=0;		
	};
	iMagic=0;
	iLeader=0;
	iHealth=0;
	ipClass2=0;
	iLevel=0;
	iExp=0;
	iLifeBonus=0;
	iMoveBonus=0;
	iTask=0;
	iAITask=0;
	iDead=-1;
	NiB=0;
	iCamper=0;
	iCampTurn=0;
	ipMoveProv=0;
	ipEvent=0;
	switch (Class)
	{
	case 1:
		iHealth=1;
		pSquad[0]->x=1;	
		apSkill[0][0]=8+Random(7);
		apSkill[0][1]=1;
		arInventory[1][0]=9+Random(3)*4;
		arInventory[1][1]=(pFirstItem+arInventory[1][0])->iDurability;
		arInventory[2][0]=41+Random(2);
		arInventory[2][1]=(pFirstItem+arInventory[2][0])->iDurability;
//		arInventory[4][0]=55;
//		arInventory[4][1]=(pFirstItem+arInventory[4][0])->iDurability;
		break;
	case 2:		
		iHealth=1;
		pSquad[0]->x=0;
		pSquad[0]->arUpg[0]=62+Random(3);
		pSquad[0]->iLevel++;
		apSkill[0][0]=22+Random(7);
		apSkill[0][1]=1;
		arInventory[1][0]=29;
		arInventory[1][1]=(pFirstItem+arInventory[1][0])->iDurability;
		arInventory[2][0]=46;
		arInventory[2][1]=(pFirstItem+arInventory[2][0])->iDurability;
//		arInventory[7][0]=17;
//		arInventory[7][1]=(pFirstItem+arInventory[7][0])->iDurability;
		break;
	case 3:		
		iLeader=1;
		pSquad[0]->x=1;	
		apSkill[0][0]=1+Random(7);
		apSkill[0][1]=1;
		switch (apSkill[0][0])
		{
		case 1:
			pSquad[3]=new dcUnit(3,0);
			pSquad[3]->x=1;
			pSquad[3]->y=2;
			arInventory[3][0]=51;
			arInventory[3][1]=(pFirstItem+arInventory[3][0])->iDurability;
			break;
		case 2:
			pSquad[3]=new dcUnit(1,0);
			pSquad[3]->x=1;
			pSquad[3]->y=2;
			pSquad[4]=new dcUnit(1,0);
			pSquad[4]->x=1;
			pSquad[4]->y=5;
			break;
		case 5:
			pSquad[3]=new dcUnit(2,0);
			pSquad[3]->x=0;
			pSquad[3]->y=2;
			break;
		default:
			pSquad[3]=new dcUnit(3,0);
			pSquad[3]->x=1;
			pSquad[3]->y=2;
			break;
		};
		break;
	case 4:	
//		arInventory[18][0]=400;
//		arInventory[18][1]=1;
		iMagic=1;
		pSquad[0]->x=0;	
		apSkill[0][0]=15+Random(7);
		apSkill[0][1]=1;
		arSpell[1]=1;
		arSpell[2]=2+Random(2);
		arSpell[3]=4;
		arSpell[4]=4+Random(10);
		switch (apSkill[0][0])
		{
		case 15:
			arInventory[1][0]=5;
			arInventory[1][1]=(pFirstItem+arInventory[1][0])->iDurability;		
			break;
		case 16:
			arSpell[2]=11;
			break;
		case 17:
			arSpell[3]=19;
			arInventory[3][0]=63;
			arInventory[3][1]=(pFirstItem+arInventory[3][0])->iDurability;
			break;	
		case 18:
			arSpell[2]=13;
			break;
		case 19:
			arSpell[5]=Random(3)+1;
			break;
		case 20:
			arInventory[1][0]=37;
			arInventory[1][1]=(pFirstItem+arInventory[1][0])->iDurability;
			break;
		case 21:
			arSpell[3]=14;
			arInventory[1][0]=1;
			arInventory[1][1]=(pFirstItem+arInventory[1][0])->iDurability;
			break;
		};
		break;
	};
	pSquad[0]->iLife+=CheckHeroBonus(1);
};

dcHero::~dcHero()
{
	if (bGameInProcess)
	{
	WriteLog("Запущен деструктор героя");
	if ((ipPlayer>0)&&(ipClass1>0))
	{
		while (arQuest[0].Index>0)
		{AbandonQuest(0);};
		int Found=MAX_HERO;
		for (int i=0; i<MAX_HERO; i++)
		{
			if (arPlayer[ipPlayer].arHero[i]==this)
			{
				WriteLog("Герой найден в списке");
				Found=i;
			};
		};
		for (int i=Found; i<MAX_HERO-1; i++)
		{
			arPlayer[ipPlayer].arHero[i]=arPlayer[ipPlayer].arHero[i+1];			
		};
		arPlayer[ipPlayer].arHero[MAX_HERO-1]=NULL;
	};
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if (pSquad[i])
		{
			delete pSquad[i];
		};
	};
	WriteLog("Герой удален");
	};
};

void dcHero::ConvertOld(dcOldHero OldHero)
{
	ipName=OldHero.ipName;
	ipPortrait=OldHero.ipPortrait;
	ipPlayer=OldHero.ipPlayer;
	iExp=OldHero.iExp;
	iLevel=OldHero.iLevel;
	NiB=OldHero.NiB;
	ipClass1=OldHero.ipClass1;
	ipClass2=OldHero.ipClass2;
	iHealth=OldHero.iHealth;
	iLeader=OldHero.iLeader;
	iMagic=OldHero.iMagic;
	iLifeBonus=OldHero.iLifeBonus;
	iMoveBonus=OldHero.iMoveBonus;
	ipProvince=OldHero.ipProvince;
	iTask=OldHero.iTask;
	iTarget=OldHero.iTarget;
	iAITask=OldHero.iAITask;
	iCamper=OldHero.iCamper;
	iCampTurn=OldHero.iCampTurn;
	iDead=OldHero.iDead;
	wavDeath=OldHero.wavDeath;
	for (int i=0; i<HERO_QUEST_LIMIT; i++)
	{arQuest[i]=OldHero.arQuest[i];};
	for (int i=0; i<SPELLBOOK_SIZE+1; i++)
	{arSpell[i]=OldHero.arSpell[i];};
	for (int i=0; i<HERO_SKILL_LIMIT; i++)
	{
		apSkill[i][0]=OldHero.apSkill[i][0];
		apSkill[i][1]=OldHero.apSkill[i][1];
	};
	for (int i=0; i<INVENTORY_SIZE+1; i++)
	{
		arInventory[i][0]=OldHero.arInventory[i][0];
		arInventory[i][1]=OldHero.arInventory[i][1];
	};
	for (int i=0; i<4; i++)
	{arGarrison[i]=OldHero.arGarrison[i];};
};

int dcHero::CheckUnitBonus(int Index)
{
	if (ipClass1==0)
	{return 0;};
	int Result=0;
	for (int i=0; i<HERO_SKILL_LIMIT; i++)
	{
		if (apSkill[i][0]>0)
		{
			for (int j=0; j<apSkill[i][1]; j++)
			{
				for (int k=0; k<MAX_SKILL_UPG; k++)
				{
					if (arSkills[apSkill[i][0]].arSquadUpg[j][k][0]==Index)
					{
						Result+=arSkills[apSkill[i][0]].arSquadUpg[j][k][1];
					};
				};
			};
		};
	};
	if (ipClass2>0)
	{
		for (int i=0; i<MAX_TALENT; i++)
		{
			if ((arTalent[ipClass1*4-1+ipClass2][i][0]>1000)&&(arTalent[ipClass1*4-1+ipClass2][i][0]%1000==Index))
			{
				Result+=arTalent[ipClass1*4-1+ipClass2][i][1];
			};
		};
		if (iLevel>=20)
		{
			for (int i=0; i<MAX_TALENT; i++)
			{
				if ((arTalent[15+ipClass1*4+ipClass2][i][0]>1000)&&(arTalent[15+ipClass1*4+ipClass2][i][0]%1000==Index))
				{
					Result+=arTalent[15+ipClass1*4+ipClass2][i][1];
				};
			};
		};
	};
	for (int i=1; i<=iMaxItemSet; i++)
	{
		*(pWorkSet+i)=0;
	};
	for (int i=1; i<16; i++)
	{
		if ((arInventory[i][0]>0)&&(arInventory[i][1]>0))
		{
			for (int j=1; j<=iMaxItemSet; j++)
			{
				for (int k=0; k<MAX_SET_ITEMS; k++)
				{
					if ((pFirstItemSet+j)->arItems[k]==arInventory[i][0])
					{
						*(pWorkSet+j)+=1;
					};
				};
			};
	
		for (int j=0; j<MAX_ITEM_EFFECT; j++)
		{
			if (((pFirstItem+arInventory[i][0])->arEffect[j][0]==Index)
				&&((pFirstItem+arInventory[i][0])->arEffect[j][2]==1)&&(arInventory[i][1]>0))
			{Result+=(pFirstItem+arInventory[i][0])->arEffect[j][1];};
		};
	};
	};
	for (int i=1; i<=iMaxItemSet; i++)
	{
		for (int j=0; j<MAX_SET_BONUS; j++)
		{
			if (((pFirstItemSet+i)->arBonusQ[j]>0)&&(*(pWorkSet+i)>=(pFirstItemSet+i)->arBonusQ[j]))
			{
				for (int k=0; k<MAX_SET_ABILITY; k++)
				{
					if (((pFirstItemSet+i)->arBonus[j][k][0]==Index)&&((pFirstItemSet+i)->arBonus[j][k][2]==1))
					{
						Result+=(pFirstItemSet+i)->arBonus[j][k][1];
					};
				};
			};
		};
	};

	return Result;
};

int dcHero::CheckHeroBonus(int Index)
{
	int Result=0;
/*	if (Index==11)
	{
		textprintf_ex(screen,Font1,300,300,makecol(220,220,20),-1,"Stam: %d ",arStat[0][0][1]);
		readkey();
	}
*/
	if ((Index==999)&&(ipClass1==0))
	{
		return iTask;
	};
	if (ipClass1==0)
	{return 0;};

	if (Index==1)
	{
		for (int i=0; i<iHealth; i++)
		{
			Result+=arStat[0][i][0];
		};
		Result+=iLifeBonus;
	};
	if (Index==11)
	{
		for (int i=0; i<iHealth; i++)
		{
			Result+=arStat[0][i][1];
		};
	};
	if (Index==12)
	{
		for (int i=0; i<iHealth; i++)
		{
			Result+=arStat[0][i][2];
		};
	};
	for (int i=0; i<HERO_SKILL_LIMIT; i++)
	{
//		rectfill(screen,0,0,200,20,makecol(0,0,0));
//		textprintf_ex(screen,Font1,5,3,makecol(200,200,200),-1,"Skill %d",i);
		if (apSkill[i][0]>0)
		{
			for (int j=0; j<apSkill[i][1]; j++)
			{
				for (int k=0; k<MAX_SKILL_UPG; k++)
				{
					if (arSkills[apSkill[i][0]].arHeroUpg[j][k][0]==Index)
					{
						Result+=arSkills[apSkill[i][0]].arHeroUpg[j][k][1];
					};
				};
			};
		};
	};
	if (ipClass2>0)
	{
		for (int i=0; i<MAX_TALENT; i++)
		{
			if (arTalent[ipClass1*4-1+ipClass2][i][0]==Index)
			{
				Result+=arTalent[ipClass1*4-1+ipClass2][i][1];
			};
		};
		if (iLevel>=20)
		{
			for (int i=0; i<MAX_TALENT; i++)
			{
				if (arTalent[15+ipClass1*4+ipClass2][i][0]==Index)
				{
					Result+=arTalent[15+ipClass1*4+ipClass2][i][1];
				};
			};
		};
	};
	for (int i=1; i<=iMaxItemSet; i++)
	{
		*(pWorkSet+i)=0;
	};
	for (int i=1; i<16; i++)
	{
		if ((arInventory[i][0]>0)&&(arInventory[i][1]>0))
		{
			for (int j=1; j<=iMaxItemSet; j++)
			{
				for (int k=0; k<MAX_SET_ITEMS; k++)
				{
					if ((pFirstItemSet+j)->arItems[k]==arInventory[i][0])
					{
						*(pWorkSet+j)+=1;
					};
				};
			};
	//		textprintf(screen,Font1,0,20,COLOR_2,"Hero class: %d, Slot: %d, Item: %d      ",ipClass1, i, arInventory[i][0]);

			for (int j=0; j<MAX_ITEM_EFFECT; j++)
			{
				if (((pFirstItem+arInventory[i][0])->arEffect[j][0]==Index)
					&&((pFirstItem+arInventory[i][0])->arEffect[j][2]==0)&&((arInventory[i][1]>0)||((pFirstItem+arInventory[i][0])->iDurability==0)))
				{
					if (((Index==8)||(Index==10))&&(i==2))
					{
						if ((pFirstItem+arInventory[1][0])->ipClass==(pFirstItem+arInventory[i][0])->ipClass)
						{
							Result+=(pFirstItem+arInventory[i][0])->arEffect[j][1];
						};
					}
					else
					{
						Result+=(pFirstItem+arInventory[i][0])->arEffect[j][1];
					};
				};
			};
		};
	};
	for (int i=1; i<=iMaxItemSet; i++)
	{
		if (((pFirstItemSet+i)->iSize>3)&&((pFirstItemSet+i)->iSize<=*(pWorkSet+i)))
		{arPlayer[ipPlayer].bSet=true;};
		for (int j=0; j<MAX_SET_BONUS; j++)
		{
			if (((pFirstItemSet+i)->arBonusQ[j]>0)&&(*(pWorkSet+i)>=(pFirstItemSet+i)->arBonusQ[j]))
			{
				for (int k=0; k<MAX_SET_ABILITY; k++)
				{
					if (((pFirstItemSet+i)->arBonus[j][k][0]==Index)&&((pFirstItemSet+i)->arBonus[j][k][2]==0))
					{
						Result+=(pFirstItemSet+i)->arBonus[j][k][1];
					};
				};
			};
		};
	};
	if (Index==8)
	{
		if ((pFirstItem+arInventory[1][0])->ipClass==3)
		{Result+=CheckHeroBonus(993);};
		if ((pFirstItem+arInventory[1][0])->ipClass==4)
		{Result+=CheckHeroBonus(994);};
		int Mult=0;
		for (int j=0; j<MAX_ITEM_EFFECT; j++)
		{
			if (((pFirstItem+arInventory[1][0])->arEffect[j][0]==Index)
				&&((pFirstItem+arInventory[1][0])->arEffect[j][2]==0)&&(arInventory[1][1]>0))
			{Mult=1;};
		};
		Result*=Mult;		
	};	

	return Result;
};


int dcHero::CheckSkill(int Index)
{	
	for (int i=0; i<HERO_SKILL_LIMIT; i++)
	{
		if (apSkill[i][0]==Index)
		{
			return apSkill[i][1];
		};
	}
	return 0;
};


int dcHero::GetSpellSlot(int Level)
{
	int Result=CheckHeroBonus(900+Level);
	if (Level==1)
	{
		Result+=3;
	};
	for (int i=0; i<iMagic; i++)
	{
		if (arStat[1][i][0]==Level)
		{Result++;};
		if (arStat[1][i][1]==Level)
		{Result++;};
	};
	return Result;
};

int dcHero::GetUnitSlot(int Level)
{
	int Result=0;
	if (arGarrison[0]>=0)
	{
		return arGarrison[Level-1];
	}
	else
	{
		if (Level==1)
		{
			Result=3;
		};
		for (int i=0; i<iLeader; i++)
		{
			if (arStat[2][i][0]==Level)
			{Result++;};
			if (arStat[2][i][1]==Level)
			{Result++;};
		};
	};
	return Result;
};

int dcHero::GetFreeUnitSlotNum(int Level)
{
	int Result=0;
	int SlotLvl[4];
	SlotLvl[0]=1;
	SlotLvl[1]=7;
	SlotLvl[2]=11;
	SlotLvl[3]=14;
	for (int i=Level-1; i<4; i++)
	{
		for (int j=SlotLvl[i]; j<SlotLvl[i]+GetUnitSlot(i+1); j++)
		{
			if (!pSquad[j])
			{
				Result++;
			};
		};
	};
//	return 0;
	return Result;
};


int dcHero::GetFreeUnitSlot(int Level)
{
	int SlotLvl[4];
	SlotLvl[0]=1;
	SlotLvl[1]=7;
	SlotLvl[2]=11;
	SlotLvl[3]=14;
	for (int i=Level-1; i<4; i++)
	{
		for (int j=SlotLvl[i]; j<SlotLvl[i]+GetUnitSlot(i+1); j++)
		{
			if (!pSquad[j])
			{
				return j;
			};
		};
	};
	return 0;
};

bool dcHero::CanUseUnitSlot (int Slot)
{
	int Rank=GetUnitSlotLevel(Slot);
	switch (Rank)
	{
	case 1:
		if (Slot<=GetUnitSlot(Rank))
		{return true;}
		else
		{return false;};
		break;
	case 2:
		if (Slot<=6+GetUnitSlot(Rank))
		{return true;}
		else
		{return false;};
		break;
	case 3:
		if (Slot<=10+GetUnitSlot(Rank))
		{return true;}
		else
		{return false;};
		break;
	case 4:
		if (Slot<=13+GetUnitSlot(Rank))
		{return true;}
		else
		{return false;};
		break;
	};
	return false;
};

int dcHero::GetSkillSlot()
{
	int Result=5;
	if (iLevel>9)
	{
		if (ipClass2==ipClass1)
		{Result++;}
		else
		{Result+=3;};
	};
	if (iLevel>19)
	{
		if (ipClass2==ipClass1)
		{Result++;}
		else
		{Result+=3;};
	};
	return Result;
};

int dcHero::CountLevel()
{
	int Result=0, Current=0, Adder=EXP_FIRST_LEVEL*2;
	if (pSquad[0])
	{Adder=Adder*pSquad[0]->GetExpMod()/100;};
	int Mod;
	if (arPlayer[ipPlayer].Control==1)
	{Mod=arDifficulty[arPlayer[ipPlayer].iDifficulty].iExp;}
	else
	{Mod=100-arPlayer[ipPlayer].iDifficulty*5;};

	while ((Current+Adder)*Mod/100<=iExp)
	{
		Result++;
		Current+=Adder;
		Adder+=EXP_ADD*(3+(Result/10)*3+(Result/20)*3);
	};
	if (Result>30)
	{
		Result=30;
		iExp=ExpForLevel(31);
	};
	return Result;
};

int dcHero::ExpForLevel(int Level)
{
	int Result=0, Adder=EXP_FIRST_LEVEL*2;
	if (pSquad[0])
	{Adder=Adder*pSquad[0]->GetExpMod()/100;};
	for (int i=0; i<Level; i++)
	{
		Result+=Adder;
		Adder+=EXP_ADD*(3+((i+1)/10)*3+((i+1)/20)*3);
	};
	if (arPlayer[ipPlayer].Control==1)
	{Result=Result*arDifficulty[arPlayer[ipPlayer].iDifficulty].iExp/100;}
	else
	{Result=Result*(100-arPlayer[ipPlayer].iDifficulty*5)/100;};
	return Result;
};


bool dcHero::CanUse(int Item)
{
	if (ipClass1==0)
	{return true;};
	switch ((pFirstItem+Item)->ipClass)
	{
	case 0:
		return true;
	case 1:
	case 6:
		return ((ipClass1<4)||((ipClass2>0)&&(ipClass2<4)));
	case 2:
	case 8:
		return ((ipClass1==1)||(ipClass2==1));
	case 3:
		return ((ipClass1==2)||(ipClass2==2));
	case 4:
		return ((ipClass1==4)||(ipClass2==4));
	case 5:
		return ((ipClass1==3)||(ipClass2==3));
	case 7:
	case 9:
		return ((ipClass1==1)||(ipClass2==1)||(ipClass1==3)||(ipClass2==3));
	};
	return 0;
};

int dcHero::GetClassName()
{
	if (iLevel<10)
	{
		return (ipClass1-1);
	};
	if (iLevel<20)
	{
		return (3+(ipClass1-1)*4+ipClass2);
	};
	return (19+(ipClass1-1)*4+ipClass2);
};

void dcHero::PlaceUnit(dcUnit * Unit, int Slot)
{
	if ((ipClass1>0)&&(Slot==0))
	{
		Slot=GetFreeUnitSlot(Unit->GetRank());
		if (Slot==0)
		{return;};
	};
	bool Will=false;
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((pSquad[i])&&(pSquad[i]->x==Unit->x)&&(pSquad[i]->y==Unit->y))
		{Will=true;};
	};
	if (Will)
	{
		for (int x=0; x<2; x++)
		{
			for (int y=0; y<8; y++)
			{
				bool Coord=true;
				for (int i=0; i<MAX_SQUAD; i++)
				{
					if ((pSquad[i])&&(pSquad[i]->x==x)&&(pSquad[i]->y==y))
					{
						Coord=false;
					};
				};
				if (Coord)
				{
					Unit->x=x;
					Unit->y=y;
					pSquad[Slot]=Unit;
					return;
				};
			};
		};
	}
	else
	{
		pSquad[Slot]=Unit;
		return;
	};
};

int dcHero::LevelUp()
{
	int SkillLimit=3;
	int SlotLimit=5;
	int SkillAllowed=7;
	int MaxSkills[HERO_SKILL_LIMIT];
	int Maxes=0;
	int UsedSkills[HERO_SKILL_LIMIT];
	int Uses=0;
	int Result=0;
	for (int i=0; i<4; i++)
	{
		arHeroLvlUp[0][i]=0;
		arHeroLvlUp[1][i]=0;
	};
	if (iLevel==9)
	{
		SlotLimit++;
//Переход на 10-й уровень
		for (int i=0; i<SlotLimit; i++)
		{
			if (apSkill[i][1]==SkillLimit)
			{
				MaxSkills[Maxes]=apSkill[i][0];
				Maxes++;
			};
		};

		/*if (Pirate1+Pirate3+Pirate4+Pirate2>0)
		{
			FileLog(pstName->str);
			delete (pFirstProvince+12);
		};*/

//Выбор умения из доступных старому классу
		ClearRoll();
		int R=0;
		for (int i=1; i<=MAX_HERO_SKILL; i++)
		{
			if (arSkills[i].iClass==ipClass1)
			{
				RollerN[R]=i;
				RollerP[R]=10;
				for (int j=0; j<Maxes; j++)
				{
					if (MaxSkills[j]==i)
					{
						RollerP[R]=0;
					};
				};
				if (RollerP[R])
				{R++;};
			};
		};
		arHeroLvlUp[0][0]=Roll(false);

//Выбор по одному умению из новых классов
		Maxes=1;
		for (int j=1; j<5; j++)
		{
			if (ipClass1!=j)
			{
				R=0;
				ClearRoll();
				for (int i=1; i<=MAX_HERO_SKILL; i++)
				{
					if (arSkills[i].iClass==j)
					{
						RollerN[R]=i;
						RollerP[R]=10;
						R++;
					};
				};
				arHeroLvlUp[0][Maxes]=Roll(false);
				Maxes++;
			};
		};
	}
	else
	{	
//Переход на обычный уровень
//Определение количества доступных слотов и уровней умений
		if (iLevel>9)
		{
			if (ipClass1==ipClass2)
			{SlotLimit++;}
			else
			{
				SlotLimit+=3;
				SkillAllowed=14;
			};			
			if (iLevel<20)
			{
				ClearRoll();			
				RollerN[0]=1;
				RollerP[0]=arStatProgress[0][(ipClass1-1)*4+ipClass2+3]+arStatProgress[0][ipClass1-1]-iHealth;
				if (RollerP[0]<0)
				{RollerP[0]=0;};
				RollerN[1]=2;
				RollerP[1]=arStatProgress[1][(ipClass1-1)*4+ipClass2+3]+arStatProgress[1][ipClass1-1]-iMagic;
				RollerN[2]=3;
				RollerP[2]=arStatProgress[2][(ipClass1-1)*4+ipClass2+3]+arStatProgress[2][ipClass1-1]-iLeader;
				Result=Roll(false);
			};
			
		}
		else
		{
			if ((iLeader==0)&&(iLevel>=2)&&(ipClass1>1))
			{
				Result=3;
			}
			else
			{
				ClearRoll();			
				RollerN[0]=1;
				RollerP[0]=arStatProgress[0][ipClass1-1]-iHealth;
				if (RollerP[0]<0)
				{RollerP[0]=0;};
				RollerN[1]=2;
				RollerP[1]=arStatProgress[1][ipClass1-1]-iMagic;
				RollerN[2]=3;
				RollerP[2]=arStatProgress[2][ipClass1-1]-iLeader;
				Result=Roll(false);
			};
		};

		if (iLevel>19)
		{
			if (ipClass1==ipClass2)
			{SlotLimit++;}
			else
			{SlotLimit+=3;};
		};
		
		if (ipClass1==ipClass2)
		{
			SkillLimit=5;
		};

//Составление списка изученных умений
		for (int i=0; i<SlotLimit; i++)
		{
			if (apSkill[i][0]>0)
			{
				if (apSkill[i][1]==SkillLimit)
				{
					MaxSkills[Maxes]=apSkill[i][0];
					Maxes++;
				}
				else
				{
					UsedSkills[Uses]=apSkill[i][0];
					Uses++;
				};
			};
		};


		if (Uses>0)
//Выбор из уже изученных умений
		{
			ClearRoll();
			for (int i=0; i<Uses; i++)
			{
				RollerN[i]=UsedSkills[i];
				RollerP[i]=10;
			};
			arHeroLvlUp[0][0]=Roll(true);

			if (Maxes+Uses<SlotLimit)
//Выбор из еще неизученных умений
			{
				int R=0;
				ClearRoll();
				for (int i=1; i<=MAX_HERO_SKILL; i++)
				{
					if ((arSkills[i].iClass==ipClass1)||(arSkills[i].iClass==ipClass2))
					{
						RollerN[R]=i;
						RollerP[R]=10;
						for (int j=0; j<Maxes; j++)
						{
							if (MaxSkills[j]==i)
							{
								RollerP[R]=0;
							};
						};
						for (int j=0; j<Uses; j++)
						{
							if (UsedSkills[j]==i)
							{
								RollerP[R]=0;
							};
						};
						if (RollerP[R])
						{R++;};
					};
				};
				arHeroLvlUp[0][1]=Roll(false);

				if (SkillAllowed-Maxes>2)
				{
				int R=0;
				ClearRoll();
				for (int i=1; i<=MAX_HERO_SKILL; i++)
				{
					if (((arSkills[i].iClass==ipClass1)||(arSkills[i].iClass==ipClass2))&&
						(i!=arHeroLvlUp[0][0])&&(i!=arHeroLvlUp[0][1]))
					{
						RollerN[R]=i;
						RollerP[R]=10;
						for (int j=0; j<Maxes; j++)
						{
							if (MaxSkills[j]==i)
							{
								RollerP[R]=0;
							};
						};						
						if (RollerP[R])
						{R++;};
					};
				};
				arHeroLvlUp[0][2]=Roll(false);
				};
			}
			else
//Выбор из уже изученных умений если нет свободных слотов
			{
				if (Uses>1)
				{
					arHeroLvlUp[0][1]=Roll(true);
				};
				if (Uses>2)
				{
					arHeroLvlUp[0][2]=Roll(true);
				};
			};
		}
		else
//Выбор из новых умений если нет подходящих изученных
		{
			int R=0;
			ClearRoll();
			for (int i=1; i<=MAX_HERO_SKILL; i++)
			{
				if ((arSkills[i].iClass==ipClass1)||(arSkills[i].iClass==ipClass2))					
				{
					RollerN[R]=i;
					RollerP[R]=10;
					for (int j=0; j<Maxes; j++)
					{
						if (MaxSkills[j]==i)
						{
							RollerP[R]=0;
						};
					};						
					if (RollerP[R])
					{R++;};
				};
			};
			arHeroLvlUp[0][0]=Roll(true);
			if (SkillAllowed-Maxes>1)
			{arHeroLvlUp[0][1]=Roll(true);};
			if (SkillAllowed-Maxes>2)
			{arHeroLvlUp[0][2]=Roll(true);};
		};				
	};
//Определение слотов для выбранных скилов
	for (int i=0; i<4; i++)
	{		
		if (arHeroLvlUp[0][i]>0)
		{
			int R=-1;
			for (int j=0; j<SlotLimit; j++)
			{
				if (apSkill[j][0]==arHeroLvlUp[0][i])
				{
					R=j;
				};
				if ((apSkill[j][0]==0)&&(R<0))
				{
					R=j;
				};
			};
			arHeroLvlUp[1][i]=R;
		};
	};
	return Result;
};

void dcHero::ChangeUnitType()
{
	if (iLevel>19)
	{
		for (int i=0; i<MAX_TALENT; i++)
		{
			if (arTalent[15+ipClass1*4+ipClass2][i][0]==960)
			{
				pSquad[0]->SetUnitType(arTalent[15+ipClass1*4+ipClass2][i][1]);
			}
		}
		return;
	}
	if (iLevel>9)
	{
		for (int i=0; i<MAX_TALENT; i++)
		{
			if (arTalent[ipClass1*4-1+ipClass2][i][0]==960)
			{
				pSquad[0]->SetUnitType(arTalent[ipClass1*4-1+ipClass2][i][1]);
			}
		}
		return;
	}
}

bool dcHero::DamageWeapon(int Dmg)
{
	bool Result=false;
	if (!bSimulate)
	{
	if (arInventory[1][0]>0)
	{
		for (int i=0; i<Dmg; i++)
		{
			if ((Random(100)>=CheckHeroBonus(995))&&(arInventory[1][1]>0))
			{
				arInventory[1][1]--;				
				if (WorldRules.iMode==1)
				{arInventory[1][1]-=(pFirstWorldRules+1)->iParam1;}
				if (arInventory[1][1]<=0)
				{
					Result=true;
					arInventory[1][1]=0;
				}
			};
			if ((Random(100)>=CheckHeroBonus(995))&&(arInventory[2][0]>0)&&(arInventory[2][1]>0)&&((pFirstItem+arInventory[2][0])->ipClass==3)&&((pFirstItem+arInventory[1][0])->ipClass==3))
			{
				arInventory[2][1]--;
				if (WorldRules.iMode==1)
				{arInventory[2][1]-=(pFirstWorldRules+1)->iParam1;}
				if (arInventory[2][1]<=0)
				{
					Result=true;
					arInventory[2][1]=0;
				}				
			};
		};
	};
	};
	return Result;
};

bool dcHero::DamageArmor(int Dmg)
{	
	bool Result=false;
	if (!bSimulate)
	{
	for (int j=0; j<Dmg; j++)
	{		
		int Count=0;
		ClearRoll();
		if ((arInventory[2][0]>0)&&(arInventory[2][1]>0)&&((pFirstItem+arInventory[2][0])->ipClass!=3))
		{
			RollerN[Count]=2;
			RollerP[Count]=50;
			Count++;
		};
		if ((arInventory[3][0]>0)&&(arInventory[3][1]>0))
		{
			RollerN[Count]=3;
			RollerP[Count]=50;
			Count++;
		};
		if ((arInventory[4][0]>0)&&(arInventory[4][1]>0))
		{
			RollerN[Count]=4;
			RollerP[Count]=30;
			Count++;
		};
		if ((arInventory[5][0]>0)&&(arInventory[5][1]>0))
		{
			RollerN[Count]=5;
			RollerP[Count]=10;
			Count++;
		};
		if ((arInventory[6][0]>0)&&(arInventory[6][1]>0))
		{
			RollerN[Count]=6;
			RollerP[Count]=20;
			Count++;
		};
		if ((arInventory[7][0]>0)&&(arInventory[7][1]>0))
		{
			RollerN[Count]=7;
			RollerP[Count]=30;
			Count++;
		};
		if ((arInventory[8][0]>0)&&(arInventory[8][1]>0))
		{
			RollerN[Count]=8;
			RollerP[Count]=30;
			Count++;
		};
		if ((arInventory[10][0]>0)&&(arInventory[10][1]>0)&&((pFirstItem+arInventory[10][0])->iDurability>1))
		{
			RollerN[Count]=10;
			RollerP[Count]=10;
			Count++;
		};
		if ((arInventory[11][0]>0)&&(arInventory[11][1]>0)&&((pFirstItem+arInventory[11][0])->iDurability>1))
		{
			RollerN[Count]=11;
			RollerP[Count]=10;
			Count++;
		};
		if ((Count>0)&&(Random(100)>=CheckHeroBonus(995)))
		{
			int DMG=Roll(false);		
			arInventory[DMG][1]--;
			if (WorldRules.iMode==1)
			{arInventory[DMG][1]-=(pFirstWorldRules+1)->iParam1;}
			if (arInventory[DMG][1]<=0)
			{
				Result=true;
				arInventory[DMG][1]=0;
			}
		};
	};
	};
	return Result;
};

int dcHero::DropLoot(int * Dur, int Skill)
{
	int Result=0;
	int Drop=Random(INVENTORY_SIZE)+1;
	for (int i=0; i<Skill; i++)
	{
		if ((arInventory[Drop][0]==0)&&(Random(100)>49))
		{
			Drop=Random(INVENTORY_SIZE)+1;
		};
	};
	if (arInventory[Drop][0]>0)
	{
		Result=arInventory[Drop][0];
		*Dur=arInventory[Drop][1];
		arInventory[Drop][0]=0;
		arInventory[Drop][1]=0;
	};
	return Result;
};

bool dcHero::TakeLoot(int Item, int Dur)
{
//	StopLog("Шмотка:",Item);
	if ((pFirstItem+Item)->ipClass==11)
	{
		if (ipPlayer>0)
		{
			if (*(arPlayer[ipPlayer].pDefender+(pFirstItem+Item)->arEffect[0][1])>=0)
			{*(arPlayer[ipPlayer].pDefender+(pFirstItem+Item)->arEffect[0][1])+=1;};
		};
		return 0;
	};
	if ((pFirstItem+Item)->ipClass==12)
	{
		if (ipPlayer>0)
		{
			if (*(arPlayer[ipPlayer].pOuterBld+(pFirstItem+Item)->arEffect[0][1])>=0)
			{*(arPlayer[ipPlayer].pOuterBld+(pFirstItem+Item)->arEffect[0][1])+=1;};
		};
		return 0;
	};
	if ((pFirstItem+Item)->ipClass==13)
	{
		if (ipPlayer>0)
		{
			if (*(arPlayer[ipPlayer].pRitual+(pFirstItem+Item)->arEffect[0][1])>=0)
			{*(arPlayer[ipPlayer].pRitual+(pFirstItem+Item)->arEffect[0][1])+=1;};
		};
		return 0;
	};
	if (CanUse(Item))
	{
		switch ((pFirstItem+Item)->ipSlot)
		{
		case 1:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			if (arInventory[(pFirstItem+Item)->ipSlot][0]==0)
			{
				arInventory[(pFirstItem+Item)->ipSlot][0]=Item;
				arInventory[(pFirstItem+Item)->ipSlot][1]=Dur;
				return 0;
			};
			break;
		case 2:
			if ((arInventory[2][0]==0)&&((arInventory[1][0]==0)||((pFirstItem+arInventory[1][0])->ipSlot!=100)))
			{
				arInventory[(pFirstItem+Item)->ipSlot][0]=Item;
				arInventory[(pFirstItem+Item)->ipSlot][1]=Dur;
				return 0;
			};
			break;

		case 10:
			if (arInventory[10][0]==0)
			{
				arInventory[10][0]=Item;
				arInventory[10][1]=Dur;
				return 0;
			};
			if (arInventory[11][0]==0)
			{
				arInventory[11][0]=Item;
				arInventory[11][1]=Dur;
				return 0;
			};
			break;
		case 11:
			if (arInventory[12][0]==0)
			{
				arInventory[12][0]=Item;
				arInventory[12][1]=Dur;
				return 0;
			};
			if (arInventory[13][0]==0)
			{
				arInventory[13][0]=Item;
				arInventory[13][1]=Dur;
				return 0;
			};
			if (arInventory[14][0]==0)
			{
				arInventory[14][0]=Item;
				arInventory[14][1]=Dur;
				return 0;
			};
			if (arInventory[15][0]==0)
			{
				arInventory[15][0]=Item;
				arInventory[15][1]=Dur;
				return 0;
			};
			break;
		case 100:
			if ((arInventory[1][0]==0)&&(arInventory[2][0]==0))
			{
				arInventory[1][0]=Item;
				arInventory[1][1]=Dur;
				return 0;
			};
			break;			
		};
	};
	for (int i=16; i<26; i++)
	{
		if (arInventory[i][0]==0)
		{
			arInventory[i][0]=Item;
			arInventory[i][1]=Dur;
			return 0;
		};
	};
	return 1;
};

int dcHero::GetMobility()
{
	int Result=pSquad[0]->GetSpeed();	
	for (int i=1; i<MAX_SQUAD; i++)
	{
		if ((pSquad[i])&&(pSquad[i]->GetSpeed()<Result))
		{
			Result=pSquad[i]->GetSpeed();
		};
	};
	if (Result<1)
	{Result=1;}	
	Result+=CheckHeroBonus(998);
	Result+=iMoveBonus;
	if (ipPlayer>0)
	{Result+=arPlayer[ipPlayer].GetBldParam1(37);};
	return Result;
};

int dcHero::GetSiege()
{
	int Result=2;
	if (ipClass1>0)
	{
		Result+=iHealth;
		Result+=CheckHeroBonus(55);
	};	
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((pSquad[i])&&((i>0)||(ipClass1==0)))
		{
			Result+=pSquad[i]->pType()->iLevel+pSquad[i]->CheckAbility(55)+pSquad[i]->CheckUpg(55);
		};
	};
	if (ipPlayer>0)
	{Result+=arPlayer[ipPlayer].GetBldParam1(42);};
	if (Result<1)
	{Result=1;}
	return Result;
};

int dcHero::GetSquadHealBonus()
{
	int Result=0;	
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((pSquad[i])&&(pSquad[i]->iLife>0))
		{
			if ((i==0)&&(ipClass1>0))
			{
				if (CheckHeroBonus(24)>0)
				{Result++;};
			}
			else
			{
				if ((pSquad[i]->CheckAbility(24)+pSquad[i]->CheckUpg(24))>0)
				{Result++;};
			};			
		};
	};	
	return Result;
};

int dcHero::GetSquadMorale(int Num)
{
	int Result=0;	
	if (pSquad[Num])
	{
		if (pSquad[Num]->CheckAbility(19)+pSquad[Num]->CheckUpg(19)>0)
		{return 0;};
		if (pSquad[Num]->GetKarma()!=0)
		{
			int Sign=pSquad[Num]->GetKarma()/abs(pSquad[Num]->GetKarma());		
			int Anti=0, Bonus=0;
			bool bNeutral=false;
			for (int i=1; i<MAX_SQUAD; i++)
			{
				if ((i!=Num)&&(pSquad[i]))
				{
					if (pSquad[i]->GetKarma()!=0)
					{
						if (pSquad[i]->GetKarma()/abs(pSquad[i]->GetKarma())!=Sign)
						{Anti+=abs(pSquad[i]->GetKarma());}
						else
						{
							if (Bonus<abs(pSquad[i]->GetKarma())-1)
							{
								Bonus=abs(pSquad[i]->GetKarma())-1;
							};
						};
					}
					else
					{bNeutral=true;};
				};
			};
			if (Anti>0)
			{
				Result-=abs(pSquad[Num]->GetKarma());
				Result-=Anti/3;
			}
			else
			{
				if (!bNeutral)
				{Result++;};
				Result+=Bonus;
			};
			if (ipPlayer>0)
			{
				if ((arPlayer[ipPlayer].iKarma>=50)&&(Sign>0))
				{Result++;};
				if ((arPlayer[ipPlayer].iKarma<=-100)&&(Sign<0))
				{Result++;};
				if (sign(arPlayer[ipPlayer].iKarma)!=Sign)
				{
					if (abs(arPlayer[ipPlayer].iKarma)<=250)
					{
						Result-=abs(arPlayer[ipPlayer].iKarma)/50;
					}
					else
					{Result-=5;};
				};
			};
		}
		else
		{
			int Count=0;
			for (int i=1; i<MAX_SQUAD; i++)
			{
				if ((i!=Num)&&(pSquad[i])&&(pSquad[i]->GetKarma()!=0))
				{
					Count++;
					Result+=GetSquadMorale(i);
				};
			};
			if (Count>0)
			{
				Result=Result/(Count*2);
			};			
		};
		if ((Result<0)&&((pSquad[Num]->CheckAbility(98)+pSquad[Num]->CheckUpg(98)>0)||(CheckUnitBonus(98)>0)))
		{Result=0;};
	};
	return Result;
};

bool dcHero::Death(int Win)
{
	if ((Tutorial.Map)&&(arPlayer[ipPlayer].Control==1))
	{
		(pFirstCampDialog+38)->Show(1,0,0,0,NULL,NULL);
		int Answer=(pFirstCampDialog+39)->Show(1,1,1,0,NULL,NULL);
		if (Answer==1)
		{(pFirstCampDialog+40)->Show(1,0,0,0,NULL,NULL);};
		if (Answer==3)
		{(pFirstCampDialog+41)->Show(1,0,0,0,NULL,NULL);};
		if (Win==0)
		{Tutorial.Death=1;};
		pSquad[0]->iLife=pSquad[0]->GetLife();
	}
	else
	{
	iTask=0;
	ClearMoveMap();
	MovePlayer=ipPlayer;
	WalkType=1;			
	WalkSpeed=1;
	MoveMap(pProvince->iNumber,-1,WalkSpeed,0,0);
	int iDistance=*(pMoveMap+arPlayer[ipPlayer].ipProvince);
	if ((iDistance<0)||(iDistance>MAX_BODY_DISTANCE))
	{iDistance=MAX_BODY_DISTANCE;};
	if (iDistance<2)
	{iDistance=2;};
	int Answer;
	char sMessage[500];
	bool Cycle=true;
	if ((Win==1)||(Win==-2))
	{
		int Dlg;
		while (Cycle)
		{
			if ((arPlayer[ipPlayer].Gold>=arPlayer[ipPlayer].GetResurrectGoldPrice(this))&&
				(arPlayer[ipPlayer].Gem>=arPlayer[ipPlayer].GetResurrectGemPrice(this)))
			{
				if (arPlayer[ipPlayer].Control==1)
				{
					Dlg=30;
					if (Win<0)
					{Dlg=1091;};
					sprintf(sMessage,(pFirstDialog+Dlg)->sText,pstName->str,arPlayer[ipPlayer].GetResurrectGoldPrice(this),arPlayer[ipPlayer].GetResurrectGemPrice(this),iDistance);
					Answer=(pFirstDialog+Dlg)->Show(true,true,true,false,NULL,sMessage);
				}
				else
				{Answer=1;};
			}
			else
			{
				if (arPlayer[ipPlayer].Control==1)
				{
					Dlg=31;
					if (Win<0)
					{Dlg=1092;};
					sprintf(sMessage,(pFirstDialog+Dlg)->sText,pstName->str,arPlayer[ipPlayer].GetResurrectGoldPrice(this),arPlayer[ipPlayer].GetResurrectGemPrice(this),iDistance);
					Answer=(pFirstDialog+Dlg)->Show(true,true,false,false,NULL,sMessage)+1;
				}
				else
				{Answer=2;};
			};
			switch (Answer)
			{
			case 1:
				arPlayer[ipPlayer].Gold-=arPlayer[ipPlayer].GetResurrectGoldPrice(this);
				arPlayer[ipPlayer].Gem-=arPlayer[ipPlayer].GetResurrectGemPrice(this);
				iDead=-1;
				pSquad[0]->iLife=pSquad[0]->GetLife()/2+1;
				Cycle=false;
				break;
			case 2:
				iDead=iDistance+1;
				pProvince=(pFirstProvince+arPlayer[ipPlayer].ipProvince);
				ipProvince=arPlayer[ipPlayer].ipProvince;
				Cycle=false;
				break;
			case 3:					
				if ((pFirstDialog+28)->Show(true,true,false,false,NULL,NULL)==1)
				{
					Cycle=false;
					delete this;
					return true;
				};
				break;
			};
		};
	}
	else
	{
		if (arPlayer[ipPlayer].Control==1)
		{
		while (Cycle)
		{
			sprintf(sMessage,(pFirstDialog+27)->sText,pstName->str,iDistance);
			if ((pFirstDialog+27)->Show(true,true,false,false,NULL,sMessage)==1)
			{
				Cycle=false;
				iDead=iDistance+1;
				pProvince=(pFirstProvince+arPlayer[ipPlayer].ipProvince);
				ipProvince=arPlayer[ipPlayer].ipProvince;
			}
			else
			{
				if ((pFirstDialog+28)->Show(true,true,false,false,NULL,NULL)==1)
				{
					Cycle=false;
					delete this;
					return true;
				};
			};
		};
		}
		else
		{
			int Count=0;
			for (int i=0; i<MAX_HERO; i++)
			{if (arPlayer[ipPlayer].arHero[i]){Count++;};};
			if ((iExp>0)||(Count>1))
			{
				iDead=iDistance+1;
				pProvince=(pFirstProvince+arPlayer[ipPlayer].ipProvince);
				ipProvince=arPlayer[ipPlayer].ipProvince;
			}
			else
			{
				delete this;
				return true;
			};
		}
	};
	}; //Tutorial
	return false;
};

void dcHero::ChangeLife(int Power, int Type, int Targets)
{
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((pSquad[i])&&(pSquad[i]->iLife>0)&&
		((Targets==0)||((pSquad[i]->GetUnitKind()!=2)&&(pSquad[i]->GetUnitKind()!=4))))
		{
			int Hit;
			if (Type==0)
			{Hit=Power;}
			else
			{
				if (Power>0)
				{Hit=Power*pSquad[i]->GetLife()/100+1;}
				else
				{Hit=Power*pSquad[i]->iLife/100-1;};
			};
			if (Hit<0)
			{
				Hit+=pSquad[i]->GetResist();
				if (Hit>0)
				{Hit=0;};
			};
			pSquad[i]->iLife+=Hit;
			if (pSquad[i]->iLife<=0)
			{pSquad[i]->iLife=1;};
			if (pSquad[i]->iLife>pSquad[i]->GetLife())
			{pSquad[i]->iLife=pSquad[i]->GetLife();};
		};
	};
};

void dcHero::ChangeMorale(int Power, int dKarma)
{
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((pSquad[i])&&(pSquad[i]->iLife>0)&&(pSquad[i]->CheckAbility(19)==0))
		{
			if (dKarma==0)
			{
				pSquad[i]->iMorale+=Power*2;
			}
			else
			{
				if (pSquad[i]->GetKarma()!=0)
				{
					if(sign(dKarma)==sign(pSquad[i]->GetKarma()))
					{pSquad[i]->iMorale+=Power*2;}
					else
					{pSquad[i]->iMorale-=Power*2;};
				};
			};
		};
	};
};


void dcHero::Encounter(int Index, dcHero *NPC, dcProvince *Prov, int Site, int Quest, int Param1, int Param2)
{
	clEncounter *EN=(pFirstEncounter+Index);
//	StopLog("Квест сайта:",Prov->arSite[Site][3]);
	for (int i=0; i<6; i++)
	{
		Treasurez[i][0]=0;
		Treasurez[i][1]=0;
	};
	for (int i=0; i<MAX_RECRUIT; i++)
	{
		Recruitz[i]=0;		
	};
	int Lootz=1;
	char Mess[1000];
	int Power=1;
	int QNum=0;
	switch (EN->GetPower(3))
	{
	case 1:
		Power=Prov->GetLevel();
		break;
	case 2:
		Power=Prov->arSite[Site][2];
		break;
	case 3:
		Power=(pFirstItem+Prov->arSite[Site][5])->iShopLevel;
		break;
	case 4:
		Power=EN->GetParam1(3,4);
		break;	
	};
//Изменение кармы
	arPlayer[ipPlayer].iKarma+=EN->iKarma;
	arPlayer[ipPlayer].iKarma+=EN->CalcEffect(7,Power,Param1,Param2,this);

//Воздействие эффектов
	int Mult=0, Count, iWork, iWork2;
	//Индивидуальные эффекты
	for (int Eff=0; Eff<MAX_EVENT_EFFECT; Eff++)
	{
		switch (EN->arEffect[Eff][0])
		{		
//		case 9:
//			PlaceSite(EV->arEffect[Eff][1],CurGame.iCurrentTurn/20);
//			break;
		case 10:	//Установить значение флага переговоров
			flgEncResult=EN->arEffect[Eff][1];
			break;
		case 12:	//Дать игроку чертеж постройки
			if (*(arPlayer[ipPlayer].pOuterBld+EN->arEffect[Eff][1])>=0)
			{
				*(arPlayer[ipPlayer].pOuterBld+EN->arEffect[Eff][1])+=EN->arEffect[Eff][2];
			};
			break;
		case 13:	//Дать герою свиток с заклятьем
			for (int i=0; i<EN->arEffect[Eff][2]; i++)
			{
				if (TakeLoot(iMaxPureItem+EN->arEffect[Eff][1],1))
				{							
					iWork2=0;
					while ((iWork2<9)&&(arLootTable[iWork2][0]>0))
					{
						iWork2++;
					};
					if (iWork2<9)
					{
						arLootTable[iWork2][0]=iMaxPureItem+EN->arEffect[Eff][1];
						arLootTable[iWork2][1]=1;
					};			
				};
			};
			break;
		case 14:	//Дать герою случайный предмет
			Count=0;
			ClearRoll();
			for (int i=1; i<=iMaxItem; i++)
			{
				if (((pFirstItem+i)->iShopLevel==EN->arEffect[Eff][1])&&((pFirstItem+i)->iRarity>=EN->arEffect[Eff][2]))
				{
					RollerN[Count]=i;
					RollerP[Count]=7-(pFirstItem+i)->iRarity;
					Count++;
				};
			};
			if ((bStaticLoot)&&(Site>0))
			{
				if ((pFirstSite+Prov->arSite[Site][0])->bSelfDestroy)
				{iWork=Roll(false,Site,ipProvince);}
				else
				{iWork=Roll(false,Site,CurGame.iCurrentTurn,ipProvince);}
			}
			else
			{iWork=Roll(false);}
			switch ((pFirstItem+iWork)->iRarity)
			{
			case 4:
				arPlayer[ipPlayer].arLoot[0]++;
				break;
			case 5:
				arPlayer[ipPlayer].arLoot[1]++;
				break;
			case 6:
				arPlayer[ipPlayer].arLoot[2]++;
				break;
			};
			if (Lootz<6)
			{
				Treasurez[Lootz][0]=iWork;
				Treasurez[Lootz][1]=(pFirstItem+iWork)->iDurability;
				Lootz++;
			};
			if (TakeLoot(iWork,(pFirstItem+iWork)->iDurability))
			{
				iWork2=0;
				while ((iWork2<9)&&(arLootTable[iWork2][0]>0))
				{
					iWork2++;
				};
				if (iWork2<9)
				{
					arLootTable[iWork2][0]=iWork;
					arLootTable[iWork2][1]=(pFirstItem+iWork)->iDurability;
				};
			};
			break;
		case 15: //Переходы к альтернативным приключениям
			switch (EN->arEffect[Eff][1])
			{
			case 1: //Армия NPC больше армии героя
				iWork=iHealth+iMagic+CheckSkill(27)*3;
				for (int i=1; i<MAX_SQUAD; i++)
				{
					if (pSquad[i])
					{iWork+=pSquad[i]->pType()->iLevel;};
				};
				iWork2=0;
				if (NPC->ipClass1>0)
				{
					iWork2=NPC->iHealth*2+NPC->iMagic*2;
				};
				for (int i=1; i<MAX_SQUAD; i++)
				{
					if (NPC->pSquad[i])
					{iWork2+=NPC->pSquad[i]->pType()->iLevel;};
				};
				if (iWork2*3/2>=iWork)
				{
					Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
					return;
				};
				break;
			case 2:	//Недостанточная дипломатия героя
				if (CheckSkill(27)<EN->arEffect[Eff][2])
				{
					Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
					return;
				};
				break;
			case 3:	//Карма игрока ниже требуемой
				if (arPlayer[ipPlayer].iKarma<EN->arEffect[Eff][2])
				{
					Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
					return;
				};
				break;
			case 4:	//Карма игрока выше требуемой
				if (arPlayer[ipPlayer].iKarma>EN->arEffect[Eff][2])
				{
					Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
					return;
				};
				break;
			case 5:	//Случайное число больше указанного
				if (Random(100)>EN->arEffect[Eff][2])
				{
					Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
					return;
				};
				break;
			case 6:	//Отношение к игроку в провинции ниже требуемого
				if ((Prov)&&(Prov->arAttitude[ipPlayer][0]<EN->arEffect[Eff][2]))
				{
					Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
					return;
				};
				break;
			case 7: //Проверка состояния квеста
					//данной расы
				if (EN->arEffect[Eff][2]==0)//получен этим героем
				{
					for (int i=0; i<HERO_QUEST_LIMIT; i++)
					{
						if ((arQuest[i].Index==CurGame.arRaceQuest[Prov->ipRace])&&(arQuest[i].ipProvince==Prov->iNumber))
						{
							Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,CurGame.arRaceQuest[Prov->ipRace],Param1,Param2);
							return;
						};
					};
				};
				if (EN->arEffect[Eff][2]==1)//получен другим героем
				{
					for (int HR=0; HR<MAX_HERO; HR++)
					{
						if ((arPlayer[ipPlayer].arHero[HR])&&(arPlayer[ipPlayer].arHero[HR]!=this))
						{
							for (int i=0; i<HERO_QUEST_LIMIT; i++)
							{
								if ((arPlayer[ipPlayer].arHero[HR]->arQuest[i].Index==CurGame.arRaceQuest[Prov->ipRace])
									&&(arPlayer[ipPlayer].arHero[HR]->arQuest[i].ipProvince==Prov->iNumber))
								{
									Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,CurGame.arRaceQuest[Prov->ipRace],Param1,Param2);
									return;
								};
							};
						};
					};
				};
				if (EN->arEffect[Eff][2]==2)//получен в другой провинции
				{
					for (int HR=0; HR<MAX_HERO; HR++)
					{
						if (arPlayer[ipPlayer].arHero[HR])
						{
							for (int i=0; i<HERO_QUEST_LIMIT; i++)
							{
								if ((arPlayer[ipPlayer].arHero[HR]->arQuest[i].Index==CurGame.arRaceQuest[Prov->ipRace])
									&&(arPlayer[ipPlayer].arHero[HR]->arQuest[i].ipProvince!=Prov->iNumber))
								{
									Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,CurGame.arRaceQuest[Prov->ipRace],Param1,Param2);
									return;
								};
							};
						};
					};
				};

				//текущего
				if (EN->arEffect[Eff][2]==4)//Получен, но не выполнен
				{
					for (int i=0; i<HERO_QUEST_LIMIT; i++)
					{
						if ((Quest>0)&&(arQuest[i].Index==Quest)&&(arQuest[i].ipProvince==Prov->iNumber)&&(arQuest[i].ipSite==Site))
						{
							CheckQuestDone(i);
							if (!arQuest[i].bComplete)
							{
								Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
								return;
							};
						};
					};
				};

				//данного сайта
				if (EN->arEffect[Eff][2]==5)//получен этим героем
				{
					if (Prov->arSite[Site][3]<0)
					{
						for (int i=0; i<HERO_QUEST_LIMIT; i++)
						{
							if (arQuest[i].Index==abs(Prov->arSite[Site][3]))
							{
								Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,CurGame.arRaceQuest[Prov->ipRace],Param1,Param2);
								return;
							};
						};
					};
				};

				if (EN->arEffect[Eff][2]==6)//недоступен
				{
					if (Prov->arSite[Site][3]<=0)
					{
						Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,CurGame.arRaceQuest[Prov->ipRace],Param1,Param2);
						return;						
					};
				};


				break;
			case 8:	//Проверка заключенных союзов
				if (EN->arEffect[Eff][2]==0)//Заключен союз с этой расой
				{
					if (arPlayer[ipPlayer].iUnion==Prov->ipRace)
					{
						Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
						return;
					};
				};
				if (EN->arEffect[Eff][2]==1)//Заключен союз с другой расой
				{
					if ((arPlayer[ipPlayer].iUnion>0)&&(arPlayer[ipPlayer].iUnion!=Prov->ipRace))
					{
						Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
						return;
					};
				};
				break;
			/*case 9:
				if ((pFirstUnitType+Prov->arSite[Site][2])->iLevel==EN->arEffect[Eff][2])
				{
					Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
					return;
				};
				break;*/
			case 10: //Случайное число с учётом дипломатии
				if (Random(100-CheckSkill(27)*10)>EN->arEffect[Eff][2])
				{
					Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
					return;
				};
				break;
			case 11: //Проверка отсутствия постройки
				for (int i=0; i<MAX_PROVINCE_BLD; i++)
				{
					if (Prov->arBuilding[i]==EN->arEffect[Eff][2])
					{
						Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
						return;
					};
				};
				break;
			case 12: //Проверка свободного места для постройки
				{
					bool Go=true;
					for (int i=0; i<MAX_PROVINCE_BLD; i++)
					{
						if (Prov->arBuilding[i]==0)
						{
							Go=false;						
						};						
					};
					if (Go)
					{
						Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
						return;
					};
				};
				break;
			case 13: //Проверка триггера кампании
				if ((CurGame.iGameType!=2)||(Campaign.arDialog[EN->arEffect[Eff][2]]==0))
				{
					Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
					return;
				};				
				break;
			case 14: //Проверка таймера события
				if (*(arPlayer[ipPlayer].pStory+EN->arEffect[Eff][2])<=0)
				{
					Encounter(EN->arEffect[Eff][3],NPC,Prov,Site,Quest,Param1,Param2);
					return;
				};				
				break;
			};
			break;
		case 17: //Активация квеста
			switch (EN->arEffect[Eff][1])
			{					
			case 1:
				Quest=CurGame.arRaceQuest[Prov->ipRace];
				break;
			case 2:
				if (Prov->arSite[Site][3]==9999)
				{
		//Генерация случайного квеста
					int Count=0;
					ClearRoll();
					for (int i=1; i<=iMaxQuest; i++)
					{
						if (((pFirstQuest+i)->iGiver==Prov->arSite[Site][0])&&(Power==(pFirstQuest+i)->Index[0]))
						{
							RollerN[Count]=i;
							RollerP[Count]=(pFirstQuest+i)->iPossibility;
							Count++;
						};
					};
					if (Count>0)
					{
						Quest=Roll(false);
						Prov->arSite[Site][3]=Quest;
						if ((pFirstQuest+Quest)->iType==8)
						{
							iWork=Prov->GetQuestUnit((pFirstQuest+Quest)->Target[0]);
							if (iWork>0)
							{
								Prov->arSite[Site][2]=iWork;
								Prov->arSite[Site][5]=(pFirstQuest+Quest)->Quantity[0]+Random((pFirstQuest+Quest)->Param[0]);
							}
							else
							{
								Count=0;
								ClearRoll();
								for (int i=1; i<=iMaxUnitType; i++)
								{
									if (((pFirstUnitType+i)->iLevel==(pFirstQuest+Quest)->Target[0])&&((i<40)||(i>43)))
									{
										RollerN[Count]=i;
										RollerP[Count]=10;
										Count++;
									};
								};
								if (Count>0)
								{
									Prov->arSite[Site][2]=Roll(false);
									Prov->arSite[Site][5]=(pFirstQuest+Quest)->Quantity[0]+Random((pFirstQuest+Quest)->Param[0]);
								}
							}
						};
					};
				}
				else
				{Quest=abs(Prov->arSite[Site][3]);};
				break;
			};
			switch (EN->arEffect[Eff][2])
			{
			case 0:
				Encounter((pFirstQuest+Quest)->ipEncInit,NPC,Prov,Site,Quest,Param1,Param2);
				return;
				break;
			case 1:
				Encounter((pFirstQuest+Quest)->ipEncWait,NPC,Prov,Site,Quest,Param1,Param2);
				return;
				break;
			case 2:
				Encounter((pFirstQuest+Quest)->ipEncSuccess,NPC,Prov,Site,Quest,Param1,Param2);
				return;
				break;
			};
			break;
		case 18:	//Изменение состояния квеста
			if (EN->arEffect[Eff][1]==0) //Получение квеста
			{
				int Q=0;
				while ((Q<HERO_QUEST_LIMIT)&&(arQuest[Q].Index>0))
				{
					Q++;
				};
				if (Q>=HERO_QUEST_LIMIT)
				{
					Encounter(80,NPC,Prov,Site,Quest,Param1,Param2);
					return;
				}
				else
				{
					arQuest[Q].Index=Quest;
					arQuest[Q].bComplete=false;
					arQuest[Q].ipProvince=Prov->iNumber;
					if ((pFirstQuest+Quest)->iGiver>0)
					{arQuest[Q].ipSite=Site;}
					else
					{arQuest[Q].ipSite=-1;};
					arQuest[Q].iTime=0;
					if ((pFirstQuest+Quest)->iGiver>0)
					{Prov->arSite[Site][3]*=-1;};
					for (int i=0; i<MAX_QUEST_TASK; i++)
					{
						arQuest[Q].arProgress[i]=0;
					};
				};
			};
			if (EN->arEffect[Eff][1]==1) //Выполнение квеста
			{
				int Q=0;
				if ((pFirstQuest+Quest)->iType>=7)
				{
					while ((Q<HERO_QUEST_LIMIT)&&((arQuest[Q].Index!=Quest)||(arQuest[Q].ipProvince!=Prov->iNumber)||(arQuest[Q].ipSite!=Site)))
					{Q++;};
				}
				else
				{
					while ((Q<HERO_QUEST_LIMIT)&&(arQuest[Q].Index!=Quest))
					{Q++;};
				};
				if (Q==HERO_QUEST_LIMIT)
				{
					StopLog("Не найден квест для сдачи! Индекс:",Quest);
					return;
				};
				if ((pFirstQuest+Quest)->iType==7)
				{
					for (int i=1; i<=INVENTORY_SIZE; i++)
					{
						if (arInventory[i][0]==Prov->arSite[Site][5])
						{
							arInventory[i][0]=0;
							arInventory[i][1]=0;
							break;
						};
					};
					iExp+=(pFirstQuest+Quest)->iExp*(pFirstItem+Prov->arSite[Site][5])->iShopLevel;
				}
				else
				{
					if ((pFirstQuest+Quest)->iType==9)
					{
						for (int i=1; i<=INVENTORY_SIZE; i++)
						{
							if (arInventory[i][0]==(pFirstQuest+Quest)->Param[0])
							{
								arInventory[i][0]=0;
								arInventory[i][1]=0;
								break;
							};
						};
					}
					if ((pFirstQuest+Quest)->iType==8)
					{Prov->arSite[Site][2]=0;}
					iExp+=(pFirstQuest+Quest)->iExp;
				};
				if ((pFirstQuest+Quest)->iGiver>0)
				{
					Prov->arSite[Site][3]=0;					
					for (int i=0; i<MAX_SITE_ABILITY; i++)
					{
						if ((pFirstSite+Prov->arSite[Site][0])->arAbility[i][0]==18)
						{Prov->arSite[Site][3]=9999;};
					};
					
				};
				bQuestComplete=true;
				arPlayer[ipPlayer].iQuest++;
				for (int i=Q; i<HERO_QUEST_LIMIT-1; i++)
				{arQuest[i]=arQuest[i+1];};
				arQuest[HERO_QUEST_LIMIT-1].Index=0;
		//		if (Q<HERO_QUEST_LIMIT)
		//		{
		//			arQuest[Q].Index=0;					
		//		};
			};
			if (EN->arEffect[Eff][1]==2) //Отказ от квеста
			{
				
				int Q=0;				
				if ((pFirstQuest+Quest)->iType>=7)
				{
					while ((Q<HERO_QUEST_LIMIT)&&((arQuest[Q].Index!=Quest)||(arQuest[Q].ipProvince!=Prov->iNumber)||(arQuest[Q].ipSite!=Site)))
					{Q++;};
				}
				else
				{
					while ((Q<HERO_QUEST_LIMIT)&&(arQuest[Q].Index!=Quest))
					{Q++;};
				};
				if (Q<HERO_QUEST_LIMIT)
				{
					QNum=arQuest[Q].ipProvince;
	//				AbandonQuest(Q);
				};				
			};
			break;
		case 19: //Заключить союз с расой, населяющей провинцию
			arPlayer[ipPlayer].iUnion=Prov->ipRace;
			break;
		case 20:
			Prov->ChangeAttitude(ipPlayer,EN->arEffect[Eff][1]);
			break;
		case 21: //Получение особого предмета
			if (TakeLoot(EN->arEffect[Eff][1],(pFirstItem+EN->arEffect[Eff][1])->iDurability))
			{				
				iWork2=0;
				while ((iWork2<9)&&(arLootTable[iWork2][0]>0))
				{
					iWork2++;
				};
				if (iWork2<9)
				{
					arLootTable[iWork2][0]=EN->arEffect[Eff][1];
					arLootTable[iWork2][1]=(pFirstItem+EN->arEffect[Eff][1])->iDurability;
				};
			};
			if (Lootz<6)
			{
				Treasurez[Lootz][0]=EN->arEffect[Eff][1];
				Treasurez[Lootz][1]=(pFirstItem+EN->arEffect[Eff][1])->iDurability;
				Lootz++;
			};
			break;
		case 22://Дать игроку договор со стражем
			if (*(arPlayer[ipPlayer].pDefender+EN->arEffect[Eff][1])>=0)
			{
				*(arPlayer[ipPlayer].pDefender+EN->arEffect[Eff][1])+=EN->arEffect[Eff][2];
			};
			break;
		case 23://Исцелить героя и воинов
			for (int i=0; i<MAX_SQUAD; i++)
			{
				if ((pSquad[i])&&(pSquad[i]->iLife>0))
				{pSquad[i]->iLife=pSquad[i]->GetLife();};
			};
			break;
		case 24://Войти в магазин
			if (EN->arEffect[Eff][1]==0)
			{
				ScreenHeroShop(this,(pFirstShop+Prov->arSite[Site][2]));
			}
			else
			{
				dcShop *Shop=new dcShop();
				Shop->PriceModifier[ipPlayer]=300;
				int Stuff=8+Random(5);
				ClearRoll();
				int Count=0;
				for (int i=1; i<=iMaxPureItem; i++)
				{
					if ((pFirstItem+i)->iRarity>1)
					{
						RollerN[Count]=i;
						RollerP[Count]=8-(pFirstItem+i)->iRarity;
						Count++;
					};
				};
				for (int i=0; i<Stuff; i++)
				{
					Shop->Items[i][0]=Roll(true);
					Shop->Items[i][1]=1+Random(3);
				};
				ScreenHeroShop(this,Shop);
				delete Shop;
			};
			break;
		case 25:			
			iExp+=EN->arEffect[Eff][1]+Random(EN->arEffect[Eff][2]);
			if (EN->arEffect[Eff][3]>0)
			{
				for (int i=1; i<MAX_SQUAD; i++)
				{
					if ((pSquad[i])&&(pSquad[i]->iLife>0))
					{
						pSquad[i]->iExp+=EN->arEffect[Eff][1]+Random(EN->arEffect[Eff][2]);
					};
				};
			};
		//	bQuestComplete=true;
			break;
		case 26://Изменить боевой дух
			ChangeMorale(EN->arEffect[Eff][1]+Random(EN->arEffect[Eff][2]),EN->arEffect[Eff][3]);			
			break;
		case 28://Уничтожение текущего сайта
			Prov->arSite[Site][0]=EN->arEffect[Eff][1];
			break;
		case 29://Найм юнитов
			for (int i=0; i<MAX_RECRUIT; i++)
			{
				if (Recruitz[i]==0)
				{
					Recruitz[i]=EN->arEffect[Eff][1];
					break;
				};
			};
			if (EN->arEffect[Eff][2]>0)
			{
				dcHero *pGarr=new dcHero();
				for (int i=0; i<4; i++)
				{
					pGarr->arGarrison[i]=0;
				};
				pGarr->ipProvince=Prov->iNumber;
				ScreenHeroGarrison(this,pGarr,true);
				delete pGarr;
			};			
			break;
		case 31: //Присоединение юнита
			{
				dcUnit *UN=new dcUnit(EN->arEffect[Eff][1],GetUnitExp(EN->arEffect[Eff][1],EN->arEffect[Eff][2]+Random(EN->arEffect[Eff][3])));
				int Slot=GetFreeUnitSlot((pFirstUnitType+EN->arEffect[Eff][1])->iLevel);
				if (Slot>0)
				{PlaceUnit(UN,Slot);}
				else
				{ScreenHero(this,UN,false);};
			};
			break;
		case 32: //Возвести постройку
			if (arPlayer[ipPlayer].Control==1)
			{Prov->ConstructBuilding(EN->arEffect[Eff][1],true,false);}
			else
			{Prov->ConstructBuilding(EN->arEffect[Eff][1],false,false);};
			break;
		case 33: //Изменение здоровья
			ChangeLife(EN->arEffect[Eff][1],EN->arEffect[Eff][2],EN->arEffect[Eff][3]);			
			break;
		case 34: //Добавить мобильность
			iMoveBonus+=EN->arEffect[Eff][1];
			break;
		case 35: //Переместить в родовую провинцию
			{
				dcProvince *PR=(pFirstProvince+arPlayer[ipPlayer].ipProvince);
				bool Fizzle=true;
				if (PR->GetSiegeIndex()<0)
				{
					ipProvince=arPlayer[ipPlayer].ipProvince;
					pProvince=PR;
					Fizzle=false;
				}
				else
				{
					for (int i=0; i<MAX_NEIGHBOUR; i++)
					{
						if ((PR->arNeighbour[i]>0)&&((pFirstProvince+PR->arNeighbour[i])->ipPlayer==ipPlayer)&&((pFirstProvince+PR->arNeighbour[i])->GetSiegeIndex()<0))
						{
							ipProvince=PR->arNeighbour[i];
							pProvince=(pFirstProvince+ipProvince);
							Fizzle=false;
							break;
						};
					};
				};
				if ((Fizzle)&&(arPlayer[ipPlayer].Control==1))
				{
					(pFirstDialog+1184)->Show(1,1,0,0,NULL,NULL);
				}
				else
				{
					iTask=0;
					iTarget=0;
				}
			};			
			break;
		};
	};
	//Обычные эффекты
	iWork=EN->CalcEffect(1,Power,Param1,Param2,this);
	arPlayer[ipPlayer].Gold+=iWork;
	Treasurez[0][0]=iWork;
	if (EN->arEffect[0][0]==1)
	{Mult=abs(iWork);};

	if (EN->GetPower(2)!=0)
	{
		Param1=EN->CalcEffect(2,Power,Param1,Param2,this);
		if (EN->arEffect[0][0]==2)
		{Mult=abs(Param1);};		
	};
//	StopLog("Параметр:",Param1);

	if (EN->GetPower(5)!=0)
	{
		Param2=EN->CalcEffect(5,Power,Param1,Param2,this);
		if (EN->arEffect[0][0]==5)
		{Mult=abs(Param2);};
	};
	
	iWork=EN->CalcEffect(6,Power,Param1,Param2,this);
	arPlayer[ipPlayer].Gem+=iWork;
	Treasurez[0][1]=iWork;
	if (EN->arEffect[0][0]==6)
	{Mult=abs(iWork);};

	if (pSquad[0]->iLife>1)
	{
		pSquad[0]->iLife-=pSquad[0]->GetLife()*EN->CalcEffect(16,Power,Param1,Param2,this)/100;
		if (pSquad[0]->iLife<1)
		{pSquad[0]->iLife=1;}
	};

	if (EN->ipDialog)
	{
//Вызов диалога
		int Answer;
		if (arPlayer[ipPlayer].Control==1)
		{		
		if (EN->iDlgParam1==0)
		{sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText);};		
		
		if (QNum==0)
		{
			if (Quest>0)
			{
				for (int i=0; i<HERO_QUEST_LIMIT; i++)
				{
					if (arQuest[i].Index==Quest)
					{QNum=i;};
				};
			}
			else
			{
				for (int i=0; i<HERO_QUEST_LIMIT; i++)
				{
					if (arQuest[i].Index==CurGame.arRaceQuest[Prov->ipRace])
					{QNum=i;};
				};
			};
			QNum=arQuest[QNum].ipProvince;
		};
		if (EN->CalcEffect(30,1,Param1,Param2,this)>0)
		{QNum=Prov->iNumber;};
		if (EN->iDlgParam1==1)
		{
			switch (EN->iDlgParam2)
			{				
				case 2:
					sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText,pstName->str,Mult);
					break;
				case 3:
					sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText,pstName->str,(pFirstProvince+QNum)->sName);
					break;
				default:
					sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText,pstName->str);
					break;				
			};
		};
		if (EN->iDlgParam1==2)
		{
			switch (EN->iDlgParam2)
			{
			case 1:
				sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText,Mult,pstName->str);
				break;
			case 3:
				sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText,Mult,(pFirstProvince+QNum)->sName);
				break;
			default:
				sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText,Mult);
				break;
			};
		};
		if (EN->iDlgParam1==3)
		{
			switch (EN->iDlgParam2)
			{
			case 1:
				sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText,(pFirstProvince+QNum)->sName,pstName->str);
				break;
			case 2:
				sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText,(pFirstProvince+QNum)->sName,Mult);
				break;
			default:
				sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText,(pFirstProvince+QNum)->sName);
				break;
			};
		};
		if (EN->iDlgParam1==4)
		{
			sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText,(pFirstItem+Prov->arSite[Site][5])->sName);			
		};
		if (EN->iDlgParam1==5)
		{sprintf(Mess,(pFirstDialog+EN->ipDialog)->sText,(pFirstUnitType+Prov->arSite[Site][2])->sName,Prov->arSite[Site][5]);};

		bool An[MAX_ANSWER];
		for (int i=0; i<MAX_ANSWER; i++)
		{			
			if ((pFirstDialog+EN->ipDialog)->arAnswer[i][1]!='#')			
			{
				An[i]=true;
		//		if (EN->GetParam1(4,i+1)==0)
		//		{An[i]=true;};
				for (int j=0; j<MAX_EVENT_EFFECT; j++)
				{
				if ((EN->arEffect[j][0]==4)&&(EN->arEffect[j][1]==i+1))
				{
				switch (EN->arEffect[j][2])
				{				
				case 1:
					if (arPlayer[ipPlayer].Gold<EN->arEffect[j][3])
					{An[i]=false;};
					break;
				case 2:
					if (arPlayer[ipPlayer].Gem<EN->arEffect[j][3])
					{An[i]=false;};
					break;
				case 3:
					if (arPlayer[ipPlayer].Gold<abs(Param1))
					{An[i]=false;};
					break;
				case 4:
					if (arPlayer[ipPlayer].Gem<abs(Param1))
					{An[i]=false;};
					break;
				case 5:
					if (arPlayer[ipPlayer].Gold<abs(Param2))
					{An[i]=false;};
					break;
				case 6:
					if (arPlayer[ipPlayer].Gem<abs(Param2))
					{An[i]=false;};
					break;
				case 7:
					if (CheckSkill(27)<EN->arEffect[j][3])
					{An[i]=false;};
					break;
				case 8:
					if (!arPlayer[ipPlayer].arBld[EN->arEffect[j][3]])
					{An[i]=false;};
					break;
				case 9:
					if (GetUnitSlot(EN->arEffect[j][3])==0)
					{An[i]=false;};
					break;
				case 10:
					if ((Site>0)&&(Prov->arSite[Site][2]>0)&&((pFirstUnitType+Prov->arSite[Site][2])->iLevel!=EN->arEffect[j][3]))
					{An[i]=false;};
					break;
				};
				};
				};
			}
			else
			{An[i]=false;};
		};
		DialogDX=EventDX;
		Answer=(pFirstDialog+EN->ipDialog)->Show(An[0],An[1],An[2],An[3],NULL,Mess)-1;
		}
		else
		{
			//Выбор ответа для ИИ
			Answer=0;
		};
		if (EN->arEncounter[Answer]>0)
		{
			Encounter(EN->arEncounter[Answer],NPC,Prov,Site,Quest,Param1,Param2);
			return;
		};
	};

	if (EN->ipAttacker>0)
	{
//Вызов сражения
		dcProvince Fake;
		Fake.sName=new char [3];		
		int Lvl=EN->GetPower(27);
		if (Lvl>0)
		{Lvl--;}
		else
		{
			Lvl=Power-1;
			if (EN->GetParam1(3,1)+EN->GetParam1(3,2)+EN->GetParam1(3,3)==0)
			{Lvl+=Random(CurGame.iCurrentTurn)/50;};
		};		
		if (Lvl>3)
		{Lvl=3;};
		if (Lvl<0)
		{Lvl=0;};
		while ((Lvl>0)&&((pFirstGuard+(pFirstGuardType+EN->ipAttacker)->arGuard[Lvl])->Power>CurGame.Map.Neutrals))
		{Lvl--;};
		Fake.PlaceGuard((pFirstGuardType+EN->ipAttacker)->arGuard[Lvl],(pFirstGuardType+EN->ipAttacker)->iKarma);
		int Battle;
		if (NPC)
		{Battle=InitBattle(Prov,NULL,this,NPC,Site,true);}
		else
		{Battle=InitBattle(Prov,NULL,this,Fake.pGuard,Site,true);};
		switch (Battle)
		{
		case -1:
			if (pSquad[0]->iLife==0)
			{Death(Battle);};
			if (EN->ipEncDraw>0)
			{Encounter(EN->ipEncDraw,NPC,Prov,Site,Quest,Param1,Param2);};
			break;
		case 0:
			if (pSquad[0]->iLife==0)
			{Death(0);};			
			if (EN->ipEncLose>0)
			{Encounter(EN->ipEncLose,NPC,Prov,Site,Quest,Param1,Param2);};
			break;
		case 1:
			if (pSquad[0]->iLife==0)
			{Death(1);};
			if (EN->ipEncWin>0)
			{Encounter(EN->ipEncWin,NPC,Prov,Site,Quest,Param1,Param2);};
		};
	};
//Вызов следующего события

	for (int Eff=0; Eff<MAX_EVENT_EFFECT; Eff++)
	{
		if (EN->arEffect[Eff][0]==11)
		{
			if ((EN->arEffect[Eff][2]>0)&&(Random(100)>EN->arEffect[Eff][3]))
			{Encounter(EN->arEffect[Eff][2],NPC,Prov,Site,Quest,Param1,Param2);}
			else
			{Encounter(EN->arEffect[Eff][1],NPC,Prov,Site,Quest,Param1,Param2);};
		};
		if (EN->arEffect[Eff][0]==8)
		{
			Prov->StaticEvent(EN->arEffect[Eff][1],ipPlayer);
		};
	};
};

void dcHero::CheckQuestDone(int Quest)
{
	clQuest *pQ=(pFirstQuest+arQuest[Quest].Index);
	if ((!arQuest[Quest].bComplete)&&(pQ->iType<7))
	{	
	bool Done=true;
	for (int i=0; i<MAX_QUEST_TASK; i++)
	{
		if ((pQ->Index[i]>0)&&(arQuest[Quest].arProgress[i]<pQ->Quantity[i]))
		{
			Done=false;
		};
	};
	if (Done)
	{
		arQuest[Quest].bComplete=true;
		if (pQ->ipEncDone)
		{
			CallEncounter(pQ->ipEncDone,NULL,pProvince,-1,arQuest[Quest].Index,0,0);
		};
	};
	};
	if (pQ->iType==7)
	{
		arQuest[Quest].bComplete=false;
		for (int i=1; i<=INVENTORY_SIZE; i++)
		{
			if (arInventory[i][0]==(pFirstProvince+arQuest[Quest].ipProvince)->arSite[arQuest[Quest].ipSite][5])
			{arQuest[Quest].bComplete=true;};
		};
	};
	if (pQ->iType==9)
	{
		arQuest[Quest].bComplete=false;
		for (int i=1; i<=INVENTORY_SIZE; i++)
		{
			if (arInventory[i][0]==pQ->Param[0])
			{arQuest[Quest].bComplete=true;};
		};
	};
	if ((pQ->iType==8)&&(!arQuest[Quest].bComplete))
	{
		if ((pFirstProvince+arQuest[Quest].ipProvince)->arSite[arQuest[Quest].ipSite][5]<=arQuest[Quest].arProgress[0])
		{
			arQuest[Quest].bComplete=true;
			if (pQ->ipEncDone)
			{
				CallEncounter(pQ->ipEncDone,NULL,pProvince,-1,arQuest[Quest].Index,0,0);
			};
		};
	};
};

//void dcHero::CheckQuestsFail()
//{
//	for (int i=0; i<HERO_QUEST_LIMIT; i++)
//	{
//		if (arQuest[i].Index>0)
//		{
//			if (arQuest[i].ipSite>=0)
//			{
//				if ((pFirstProvince+arQuest[i].ipProvince)->arSite[arQuest[i].ipSite][0]==0)
//				{
//					CallEncounter((pFirstQuest+arQuest[i].Index)->ipEncFailGiver,NULL,(pFirstProvince+arQuest[i].ipProvince), arQuest[i].ipSite, arQuest[i].Index, 0, 0);
//					AbandonQuest(i);
//					i--;
//				};
//			}
//			else
//			{
//				if ((pFirstProvince+arQuest[i].ipProvince)->ipPlayer!=0)
//				{
//					CallEncounter((pFirstQuest+arQuest[i].Index)->ipEncFailGiver,NULL,(pFirstProvince+arQuest[i].ipProvince), 0, arQuest[i].Index, 0, 0);
//					AbandonQuest(i);
//					i--;
//				};
//			};
//		};
//	};
//};

void dcHero::AbandonQuest(int Quest)
{

	if ((pFirstQuest+arQuest[Quest].Index)->iGiver>0)
	{(pFirstProvince+arQuest[Quest].ipProvince)->arSite[arQuest[Quest].ipSite][3]*=-1;};
	for (int i=Quest; i<HERO_QUEST_LIMIT-1; i++)
	{arQuest[i]=arQuest[i+1];};
	arQuest[HERO_QUEST_LIMIT-1].Index=0;
};

void dcHero::CallEncounter(int Index, dcHero *NPC, dcProvince *Prov, int Site, int Quest, int Param1, int Param2)
{
	int Expoz=iExp;
	bQuestComplete=false;
	for (int i=0; i<9; i++)
	{
		arLootTable[i][0]=0;
	};
	Encounter(Index, NPC, Prov, Site, Quest, Param1, Param2);
	if (Expoz<iExp)
	{
		while (iLevel<CountLevel())
		{
			if (arPlayer[ipPlayer].Control==1)
			{WindowHeroLvlUp(this);}
			else
			{AIHeroLvlUp(this);};
		};
		for (int i=1; i<MAX_SQUAD; i++)
		{
			if ((pSquad[i])&&(pSquad[i]->iLife>0))
			{
				while (pSquad[i]->CountLevel()>pSquad[i]->iLevel)
				{
					if (arPlayer[ipPlayer].Control==1)
					{WindowUnitLvlUp(pSquad[i],ipPlayer);}
					else
					{pSquad[i]->MakeLevel(pSquad[i]->LevelUp(pSquad[i]->iLevel+1,0));};
				};
			};
		};
	};
	if (arLootTable[0][0]>0)
	{
		WindowTakeLoot(this);
	};

};

int dcHero::MakeDescription(char *txtSquad)
{	
	int Guards[MAX_SQUAD];
	for (int i=0; i<MAX_SQUAD; i++)
	{Guards[i]=0;};
	int MaxGuards=0, GuardTypes=0;								
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((pSquad[i])&&(pSquad[i]->iLife>0))
		{
			MaxGuards++;
			bool bAdd=true;
			if (!pSquad[i]->pHero)
			{
				for (int j=0; j<GuardTypes; j++)
				{
					if (Guards[j]==pSquad[i]->GetUnitType())
					{bAdd=false;};
				};
				if (bAdd)
				{
					Guards[GuardTypes]=pSquad[i]->GetUnitType();
					GuardTypes++;
				};
			};
		};
	};
	int Starter=0;
	if ((pSquad[0])&&(pSquad[0]->iLife>0)&&(pSquad[0]->pHero))
	{
		sprintf(txtSquad,"%s",ClassName[GetClassName()].str);	
	}
	else
	{
		sprintf(txtSquad,"%s",(pFirstUnitType+Guards[0])->sName);
		Starter=1;
	};
	
	for (int i=Starter; i<GuardTypes; i++)
	{
		sprintf(txtSquad,"%s, %s",txtSquad,(pFirstUnitType+Guards[i])->sName);
	};
	return MaxGuards;
};

void dcHero::HealUnits(int HealRate)
{
	if (HealRate>0)
	{
		for (int i=0; i<MAX_SQUAD; i++)
		{
			if ((pSquad[i])&&(pSquad[i]->iLife<pSquad[i]->GetLife()))
			{
				int Heal=pSquad[i]->GetLife()*HealRate/100;
				if (Heal<1)
				{Heal=1;};
				pSquad[i]->iLife+=Heal;
				if (pSquad[i]->iLife>pSquad[i]->GetLife())
				{pSquad[i]->iLife=pSquad[i]->GetLife();};
			};
		};
	}
	else
	{
		for (int i=0; i<MAX_SQUAD; i++)
		{
			if ((pSquad[i])&&(pSquad[i]->iLife>0))
			{
				int Heal=(pSquad[i]->iLife*abs(HealRate))/100;
				if (Heal<1)
				{Heal=1;};				
				pSquad[i]->iLife-=Heal;
				if (pSquad[i]->iLife<=0)
				{pSquad[i]->iLife=1;};
			};
		};
	};
};

void dcHero::ChangeUnitsMorale(int Morale)
{
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((pSquad[i])&&(pSquad[i]->iLife>0))
		{
			pSquad[i]->iMorale+=Morale;
			if (pSquad[i]->iMorale<-20)
			{pSquad[i]->iMorale=-20;};
			if (pSquad[i]->iMorale>20)
			{pSquad[i]->iMorale=20;};			
		};
	};
};

int PickItem(int Slot, int Level, int Type)
{
	int Result=0, Count=0;
	ClearRoll();
	for (int i=1; i<=iMaxItem; i++)
	{
		clItem *IT=(pFirstItem+i);
		if ((IT->ipSlot==Slot)&&(IT->iShopLevel==Level)&&(IT->ipClass==Type))
		{
			RollerN[Count]=i;
			RollerP[Count]=10;
			Count++;
		};
	};
	if (Count>0)
	{Result=Roll(false);};
	return Result;
};

dcHero* GetOrca()
{
	dcHero *Oppo=new dcHero(0,4);	
	Oppo->iExp=Oppo->ExpForLevel(30);
	while (Oppo->iLevel<Oppo->CountLevel())
	{
		AIHeroLvlUp(Oppo);

		if (Oppo->iLevel==10)
		{
			Oppo->ipClass2=1;
			int Last=0;
			for (int i=0; i<HERO_SKILL_LIMIT; i++)
			{
				if (Oppo->apSkill[i][0]>0)
				{Last=i;};
			};
			Oppo->apSkill[Last][0]=9;
		};
	};
	Oppo->ChangeUnitType();
	Oppo->apSkill[0][0]=19;
	Oppo->apSkill[0][1]=3;
	Oppo->apSkill[1][0]=15;
	Oppo->apSkill[1][1]=3;
	Oppo->apSkill[2][0]=17;
	Oppo->apSkill[2][1]=3;
	Oppo->apSkill[3][0]=18;
	Oppo->apSkill[3][1]=3;
	Oppo->apSkill[4][0]=9;
	Oppo->apSkill[4][1]=3;
	Oppo->apSkill[5][0]=10;
	Oppo->apSkill[5][1]=3;
	Oppo->apSkill[6][0]=12;
	Oppo->apSkill[6][1]=3;
	Oppo->apSkill[7][0]=20;
	Oppo->apSkill[7][1]=3;
	Oppo->apSkill[8][0]=8;
	Oppo->apSkill[8][1]=3;
	Oppo->apSkill[9][0]=13;
	Oppo->apSkill[9][1]=3;
	Oppo->apSkill[10][0]=0;
	Oppo->apSkill[10][1]=0;


	Oppo->arInventory[1][0]=174;
	Oppo->arInventory[1][1]=100;
	Oppo->arInventory[2][0]=227;
	Oppo->arInventory[2][1]=100;
	Oppo->arInventory[3][0]=256;
	Oppo->arInventory[3][1]=100;
	Oppo->arInventory[4][0]=298;
	Oppo->arInventory[4][1]=100;
	Oppo->arInventory[5][0]=351;
	Oppo->arInventory[5][1]=100;
	Oppo->arInventory[6][0]=362;
	Oppo->arInventory[6][1]=100;
	Oppo->arInventory[7][0]=278;
	Oppo->arInventory[7][1]=100;
	Oppo->arInventory[8][0]=319;
	Oppo->arInventory[8][1]=100;
	Oppo->arInventory[9][0]=373;
	Oppo->arInventory[9][1]=100;
	Oppo->arInventory[10][0]=344;
	Oppo->arInventory[10][1]=100;
	Oppo->arInventory[11][0]=335;
	Oppo->arInventory[11][1]=100;
	Oppo->arInventory[12][0]=390;
	Oppo->arInventory[12][1]=100;
	Oppo->arInventory[13][0]=390;
	Oppo->arInventory[13][1]=100;
	Oppo->arInventory[14][0]=390;
	Oppo->arInventory[14][1]=100;
	Oppo->arInventory[15][0]=385;
	Oppo->arInventory[15][1]=100;

	Oppo->arSpell[0]=4;
	Oppo->arSpell[1]=4;
	Oppo->arSpell[2]=4;
	Oppo->arSpell[3]=4;
	Oppo->arSpell[4]=4;
	Oppo->arSpell[5]=15;
	Oppo->arSpell[6]=15;
	Oppo->arSpell[7]=15;

	Oppo->arSpell[8]=22;
	Oppo->arSpell[9]=22;
	Oppo->arSpell[10]=23;
	Oppo->arSpell[11]=23;
	Oppo->arSpell[12]=32;
	Oppo->arSpell[13]=32;

	Oppo->arSpell[14]=57;
	Oppo->arSpell[15]=40;
	Oppo->arSpell[16]=76;

	Oppo->arSpell[17]=64;
	Oppo->arSpell[18]=65;


	for (int i=1; i<MAX_SQUAD; i++)
	{
		if (Oppo->pSquad[i])
		{
			delete Oppo->pSquad[i];
			Oppo->pSquad[i]=NULL;
		};
	};
	Oppo->pSquad[0]->iLife=Oppo->pSquad[0]->GetLife();
	Oppo->pSquad[0]->iMorale=20;
	return Oppo;
};

dcHero* GenerateFighter(int NewLevel, int League)
{
	dcHero *Oppo=new dcHero(0,Random(4)+1);	
	Oppo->iExp=Oppo->ExpForLevel(NewLevel);
	int Karma=Random(3)-1;
	while (Oppo->iLevel<Oppo->CountLevel())
	{
		AIHeroLvlUp(Oppo);
	};
	int Armor=0, Weapon=4;
	if ((Oppo->ipClass1==2)||(Oppo->ipClass2==2)){Armor=6;};
	if ((Oppo->ipClass1==3)||(Oppo->ipClass2==3)){Armor=7;};
	if ((Oppo->ipClass1==1)||(Oppo->ipClass2==1)){Armor=8;};
	if ((Oppo->ipClass1==3)||(Oppo->ipClass2==3)){Weapon=1;};
	if ((Oppo->ipClass1==1)||(Oppo->ipClass2==1)){Weapon=2;};
	if ((Oppo->ipClass1==2)||(Oppo->ipClass2==2)){Weapon=3;};
	bool bTH=false;
	if ((Weapon==2)&&(Random(10)>4))
	{
		bTH=true;
		Oppo->arInventory[1][0]=PickItem(100,1+NewLevel/6,Weapon);
	}
	else
	{
		Oppo->arInventory[1][0]=PickItem(1,1+NewLevel/6,Weapon);			
	};
	Oppo->arInventory[1][1]=100;
	if (!bTH)
	{
		if (Weapon<3)
		{Oppo->arInventory[2][0]=PickItem(2,1+NewLevel/6,9);}
		else
		{Oppo->arInventory[2][0]=PickItem(2,1+NewLevel/6,Weapon);};		
		Oppo->arInventory[2][1]=100;
	};
	Oppo->arInventory[3][0]=PickItem(3,1+NewLevel/6,Armor);
	Oppo->arInventory[3][1]=100;
	Oppo->arInventory[4][0]=PickItem(4,1+NewLevel/6,Armor);
	Oppo->arInventory[4][1]=100;
	if (League>1)
	{
		Oppo->arInventory[7][0]=PickItem(7,1+NewLevel/6,Armor);
		Oppo->arInventory[7][1]=100;
		Oppo->arInventory[8][0]=PickItem(8,1+NewLevel/6,Armor);
		Oppo->arInventory[8][1]=100;
		Oppo->arInventory[5][0]=PickItem(5,1+NewLevel/6,0);
		Oppo->arInventory[5][1]=100;
		Oppo->arInventory[6][0]=PickItem(6,1+NewLevel/6,0);
		Oppo->arInventory[6][1]=100;
	};
	if (League>2)
	{
		Oppo->arInventory[9][0]=PickItem(9,1+NewLevel/6,0);
		Oppo->arInventory[9][1]=100;
		Oppo->arInventory[10][0]=PickItem(10,1+NewLevel/6,Armor);
		Oppo->arInventory[10][1]=100;
		Oppo->arInventory[11][0]=PickItem(10,1+NewLevel/6,Armor);
		Oppo->arInventory[11][1]=100;
		Oppo->arInventory[12][0]=PickItem(11,1+NewLevel/6,0);
		Oppo->arInventory[12][1]=100;
		Oppo->arInventory[13][0]=PickItem(11,1+NewLevel/6,0);
		Oppo->arInventory[13][1]=100;
		Oppo->arInventory[14][0]=PickItem(11,1+NewLevel/6,0);
		Oppo->arInventory[14][1]=100;
		Oppo->arInventory[15][0]=PickItem(11,1+NewLevel/6,0);
		Oppo->arInventory[15][1]=100;
	};
	Oppo->pSquad[0]->iLife=Oppo->pSquad[0]->GetLife();
	Oppo->pSquad[0]->iMorale=(League-1)*4+Random(League*6);
	for (int i=1; i<MAX_SQUAD; i++)
	{
		if (Oppo->pSquad[i])
		{
			delete Oppo->pSquad[i];
			Oppo->pSquad[i]=NULL;
		};
	};

	int Spellz=1;
	for (int Rank=4; Rank>0; Rank--)
	{
		int Slot=Oppo->GetFreeUnitSlot(Rank);
		int CurrentRank=Rank;
		if ((League==1)&&(CurrentRank>1+NewLevel/5))
		{CurrentRank=1+NewLevel/5;};
		if ((League==2)&&(CurrentRank>1+NewLevel/3))
		{CurrentRank=1+NewLevel/3;};
		while (Slot>0)
		{
			int Count=0;
			ClearRoll();
			for (int i=1; i<=iMaxUnitType; i++)
			{
				if ((i!=40)&&(i!=41)&&(i!=42)&&(i!=43)&&(i!=60))
				{
					bool bAllow=true;
					if ((League==1)&&
					(i==15)||(i==16)||(i==55)||(i==66))
					{bAllow=false;};
					if ((League<3)&&(i==62))
					{bAllow=false;};					
					clUnitType *UT=(pFirstUnitType+i);
					if (UT->iUnitKind==5)
					{bAllow=false;};
					if ((bAllow)&&(UT->iLevel==CurrentRank)&&((UT->iKarma==0)||(sign(UT->iKarma)==Karma)))
					{
						RollerN[Count]=i;
						RollerP[Count]=10;
						Count++;
					};
				};
			};
			int Lvl=NewLevel-8+League*2+Random(9);
			if (Lvl<0){Lvl=0;};
			if(Lvl>30){Lvl=30;};
			int Unit=Roll(false);
			dcUnit *pUnit=new dcUnit(Unit,GetUnitExp(Unit,Lvl));
			Oppo->PlaceUnit(pUnit,Slot);
			Oppo->pSquad[Slot]->iMorale=(League-1)*4+Random(League*3);
			Slot=Oppo->GetFreeUnitSlot(Rank);
		};
		int Count=0;
		ClearRoll();
		for (int i=1; i<=iMaxSpell; i++)
		{
			if ((pFirstSpell+i)->iLevel==CurrentRank)
			{
				RollerN[Count]=i;
				RollerP[Count]=10;
				Count++;			
			};
		};		
		for (int i=0; i<Oppo->GetSpellSlot(Rank); i++)
		{
			Oppo->arSpell[Spellz]=Roll(false);
			Spellz++;
		};
	};
	return Oppo;
};

void dcHero::Arena()
{
	char Text[1000];
	sprintf (Text,(pFirstDialog+843)->sText,pstName->str);
	int Answer=(pFirstDialog+843)->Show(1,1,1,0,NULL,Text);
	if (Answer==1)
	{
		sprintf (Text,(pFirstDialog+844)->sText,pstName->str);
		int Answer=(pFirstDialog+844)->Show(1,1,1,1,NULL,Text);
		int NewLevel=iLevel;
		switch (Answer)
		{
		case 1:
			NewLevel-=(Random(iLevel/5)+iLevel/5);
			break;
		case 2:
			NewLevel+=2-Random(3);
			break;
		case 3:
			NewLevel+=3+Random(3);
			break;
		case 4:
			return;
			break;
		};
		if (NewLevel>30)
		{NewLevel=30;};
		dcHero *Oppo;
		if ((Answer==3)&&(arPlayer[ipPlayer].arCamper[0]==9))
		{
			Oppo=GetOrca();
			sprintf (Text,(pFirstDialog+1455)->sText,pstName->str);
			(pFirstDialog+1455)->Show(1,1,1,0,NULL,Text);

			//ScreenHero(Oppo,NULL,false);
		}
		else
		{
			Oppo=GenerateFighter(NewLevel, Answer);		
			sprintf (Text,(pFirstDialog+845)->sText,pstName->str,ClassName[Oppo->GetClassName()].str);
			(pFirstDialog+845)->Show(1,1,1,0,NULL,Text);
		};		
		bArena=true;
		int Win=InitBattle(pProvince,NULL,this,Oppo,Random(20),true);
		if (Win==1)
		{
			if (Answer==3)
			{arPlayer[ipPlayer].arCamper[0]++;};
			if ((Answer==3)&&(arPlayer[ipPlayer].arCamper[0]==10))
			{
				CallEncounter(680,NULL,pProvince,0,0,0,0);
			}
			else
			{
				int Gold=50+Random(50)+Answer*20+Random(Answer*20)+NewLevel*(20+Answer*7);
				Gold+=Gold*CheckHeroBonus(985)/100;
				Gold=arDifficulty[arPlayer[ipPlayer].iDifficulty].iLoot*Gold/100;
				sprintf (Text,(pFirstDialog+846)->sText,pstName->str,Gold);
				(pFirstDialog+846)->Show(1,1,1,0,NULL,Text);
				arPlayer[ipPlayer].Gold+=Gold;
			};
		};
		if (pSquad[0]->iLife==0)
		{
			Death(Win);
		};
		if (Oppo)
		{delete Oppo;};
		bArena=false;
	};
	if (Answer==2)
	{
		if (Random(20,ipProvince,CurGame.iSeed,CurGame.iCurrentTurn)>9)
		{
			int Lvl=10+Random(21), League=1+Random(3);
			bool bFight=true; int Bet=0, Pay=0;
			dcHero *PL1=GenerateFighter(Lvl,League);
			dcHero *PL2=GenerateFighter(Lvl,League);
			sprintf (Text,(pFirstDialog+847)->sText,ClassName[PL1->GetClassName()].str,ClassName[PL2->GetClassName()].str);
			int Answer=(pFirstDialog+847)->Show(1,1,1,1,NULL,Text);
			if (Answer<4)
			{
				if (Answer<3)
				{
					Bet=Answer;
					bool A1=bDemo, A2, A3;
					A1=(arPlayer[ipPlayer].Gold>=100);
					A2=(arPlayer[ipPlayer].Gold>=200);
					A3=(arPlayer[ipPlayer].Gold>=300);			
					int Answer=(pFirstDialog+848)->Show(A1,A2,A3,1,NULL,NULL);
					if (Answer<4)
					{
						arPlayer[ipPlayer].Gold-=Answer*100;
						Pay=Answer*200;
					}
					else
					{bFight=false;};
				};
				if (bFight)
				{
					int Win=InitBattle(pProvince,NULL,PL1,PL2,Random(20),true);
					if (((Win==1)&&(Bet==1))||((Win==0)&&(Bet==2)))
					{
						sprintf (Text,(pFirstDialog+850)->sText,Pay);
						(pFirstDialog+850)->Show(1,1,0,0,NULL,Text);
						arPlayer[ipPlayer].Gold+=Pay;
					}
					else
					{
						if (Bet>0)
						{(pFirstDialog+851)->Show(1,1,0,0,NULL,NULL);};
					};
				};
			};
			if (PL1){delete PL1;};
			if (PL2){delete PL2;};
		}
		else
		{(pFirstDialog+849)->Show(1,1,0,0,NULL,NULL);};
	};
};

//Возвращает плату в золоте отряду-охраннику с текущим составом юнитов
int dcHero::GetDefenderGoldPayment()
{
	int Result=0;
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if (pSquad[i])
		{Result+=pSquad[i]->pType()->iGoldPayment;};
	};
	return 1+Result/20;
};

//Возвращает плату в кристаллах отряду-охраннику с текущим составом юнитов
int dcHero::GetDefenderGemPayment()
{
	int Result=0;
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if (pSquad[i])
		{Result+=pSquad[i]->pType()->iGemPayment;};
	};
	return Result/20;
};

void dcHero::RestoreUnits(bool bHomeLand, int ProvBonus, bool bRaise)
{
	int HealBonus, SquadHeal=GetSquadHealBonus(), iDead=0;	
	if ((bRaise)&&(CurGame.iCurrentTurn%GUARD_RESS_TIME==0))
	{
		ClearRoll();
	};
	for (int k=0; k<MAX_SQUAD; k++)
	{
	if (pSquad[k])
	{
		if (pSquad[k]->iLife>0)
		{
			//Восстановление нежити
			if ((!bRaise)&&(pSquad[k]->GetUnitKind()==2))
			{				
				if (ipClass1>0)
				{
					pSquad[k]->iLife+=pSquad[k]->GetLife()*CheckSkill(21)/10;
					if (pSquad[k]->iLife>pSquad[k]->GetLife())
					{pSquad[k]->iLife=pSquad[k]->GetLife();};
				};
			}
			else
			{
			if ((k>0)||(ipClass1==0))
			{HealBonus=pSquad[k]->GetHealBonus();}
			else
			{HealBonus=iHealth+CheckHeroBonus(35)+CheckHeroBonus(48)*2;};
			if (bHomeLand)
			{
				if (pSquad[k]->iLife<pSquad[k]->GetLife())
				{
					int LifeAdd=pSquad[k]->GetLife()*(10+ProvBonus)/100;
					if (LifeAdd<1)
					{LifeAdd=1;};
					LifeAdd+=HealBonus;
					if ((pSquad[k]->GetUnitKind()!=2)&&(pSquad[k]->GetUnitKind()!=4))
					{LifeAdd+=SquadHeal;};
					pSquad[k]->iLife+=LifeAdd;
					if (pSquad[k]->iLife>pSquad[k]->GetLife())
					{pSquad[k]->iLife=pSquad[k]->GetLife();};
				};
				if (pSquad[k]->iMorale<0)
				{pSquad[k]->iMorale++;};
				if (pSquad[k]->iMorale>0)
				{pSquad[k]->iMorale--;};
			}
			else
			{
				if (pSquad[k]->iLife<pSquad[k]->GetLife())
				{
					int LifeAdd=0;					
					LifeAdd+=HealBonus;
					if ((pSquad[k]->GetUnitKind()!=2)&&(pSquad[k]->GetUnitKind()!=4))
					{LifeAdd+=SquadHeal;};
					pSquad[k]->iLife+=LifeAdd;
					if (pSquad[k]->iLife>pSquad[k]->GetLife())
					{pSquad[k]->iLife=pSquad[k]->GetLife();};
				};
				if (pSquad[k]->iMorale>-15)
				{pSquad[k]->iMorale--;};
			};
			};
		}
		else
		{
			if ((bRaise)&&(CurGame.iCurrentTurn%GUARD_RESS_TIME==0))
			{
				RollerN[iDead]=k+1;
				RollerP[iDead]=5-pSquad[k]->pType()->iLevel;
				if (RollerP[iDead]<1){RollerP[iDead]=1;}
				iDead++;
			};
		};
	};
	};
	if (iDead>0)
	{
		int Lucky=Roll(false)-1;
		pSquad[Lucky]->iLife=pSquad[Lucky]->GetLife()/2+1;
	};
};

void dcHero::MakeScoutingText()
{
	sprintf(stScouting,(pFirstMessage+54)->msg);
	int En[MAX_SQUAD][4];
	for (int i=0; i<MAX_SQUAD; i++)
	{En[i][0]=0;};
	bool bAdd=bDemo;
	int Count=0;
	for (int i=0; i<MAX_SQUAD; i++)
	{		
		if ((pSquad[i])&&(pSquad[i]->iLife>0))
		{
			bAdd=true;
			for (int j=0; j<Count; j++)
			{
				if (pSquad[i]->GetUnitType()==En[j][0])
				{
					bAdd=false;
					En[j][1]++;
					if (pSquad[i]->iLevel<En[j][2])
					{En[j][2]=pSquad[i]->iLevel;};
					if (pSquad[i]->iLevel>En[j][3])
					{En[j][3]=pSquad[i]->iLevel;};
					break;
				};
			};
			if (bAdd)
			{
				En[Count][0]=pSquad[i]->GetUnitType();
				En[Count][1]=1;
				En[Count][2]=pSquad[i]->iLevel;
				En[Count][3]=pSquad[i]->iLevel;
				Count++;
			};
		};
	};
	char STR[200];
	for (int i=0; i<Count; i++)
	{
		if ((i==0)&&(ipClass1>0)&&(pSquad[0])&&(pSquad[0]->iLife>0))
		{
			sprintf(STR,"%s (%s %d)",ClassName[GetClassName()].str,(pFirstPhrase+454)->str,iLevel);
		}
		else
		{
			if ((En[i][1]==1)||(En[i][2]==En[i][3]))
			{sprintf(STR,"%s: %d (%s %d)",(pFirstUnitType+En[i][0])->sName,En[i][1],(pFirstPhrase+454)->str,En[i][2]);}
			else
			{sprintf(STR,"%s: %d (%s %d - %d)",(pFirstUnitType+En[i][0])->sName,En[i][1],(pFirstPhrase+454)->str,En[i][2],En[i][3]);};
		};
		sprintf(stScouting,(pFirstMessage+55)->msg,stScouting,STR);
	};
};