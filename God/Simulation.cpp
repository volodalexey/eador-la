#include "Battlefield.h"
//int arSimCrystal[2];

int CompareStrength1(dcHero *Hero1, dcHero *Hero2)
{
	if (((!bVerySmartAI))&&(!bSmartAI)&&(iHeroAdvice==0))
	{return CompareStrength(Hero1, Hero2);};
	if (!Hero1)
	{return 0;};
	if (!Hero2)
	{return 200;};
	int Result=100, iTurn=0;
	bSimulate=true;
	objMain=objSim;
	Heroes[0]=Hero1;
	Heroes[1]=Hero2;
	BF_Player[0]=Hero1->ipPlayer;
	BF_Player[1]=Hero2->ipPlayer;
	for (int i=0; i<2; i++)
	{GemSpent[i]=0;};
	//arSimCrystal[0]=arPlayer[BF_Player[0]].Gem;
	//arSimCrystal[1]=arPlayer[BF_Player[1]].Gem;
	bShowBattle=false;
	bShowAnim=false;
	for (int i=0; i<8; i++)
	{
		for (int j=0; j<8; j++)
		{
			arGrid[i][j].Land=1;
			arGrid[i][j].Pass=1;			
		};
	};
	int TowerShoot=0;
	ProvinceMoraleBonus=0;	
	/*int arMagTower[3];
	for (int i=0; i<3; i++)
	{arMagTower[i]=0;};
	int MagTower=0;
	if (Site<1)
	{
		dcProvince *PR;
		if (Prov2)
		{PR=Prov2;}
		else
		{PR=Prov1;};
		int PL=0;
		for (int i=1; i<=CurGame.iMaxPlayers; i++)
		{
			if (arPlayer[i].ipProvince==PR->iNumber)
			{PL=i;};
		};
		if (PL>0)
		{
			ProvinceMoraleBonus=arPlayer[PL].GetBldParam1(19);
			for (int i=1; i<=iMaxInnerBld; i++)
			{
				if (arPlayer[PL].arBld[i])
				{
					for (int j=0; j<MAX_BUILDING_ABILITY; j++)
					{
						if ((pFirstInnerBld+i)->arAbility[j][0]==52)
						{
							arMagTower[MagTower]=(pFirstInnerBld+i)->arAbility[j][2];							
							MagTower++;
						};
					};
				};
			};
		}
		else
		{
			for (int i=0; i<MAX_PROVINCE_BLD; i++)
			{
				if (PR->arBuilding[i]>0)
				{
					for (int j=0; j<MAX_BUILDING_ABILITY; j++)
					{
						if ((pFirstOuterBld+PR->arBuilding[i])->arAbility[j][0]==10)
						{
							ProvinceMoraleBonus+=(pFirstOuterBld+PR->arBuilding[i])->arAbility[j][1];
						};
					};
					if (((pFirstOuterBld+PR->arBuilding[i])->iTowerShoot>0)&&((pFirstOuterBld+PR->arBuilding[i])->iWall==0))
					{
						arMagTower[MagTower]=(pFirstOuterBld+PR->arBuilding[i])->iTowerShoot;
						MagTower++;
					};
				};
			};
		};
	};*/
	LifeMod[0]=100;
	LifeMod[1]=100;
	for (int i=0; i<2; i++)
	{
		//AI
		if ((arPlayer[BF_Player[0]].Control!=1)&&(arPlayer[BF_Player[1]].Control!=1))
		{
			if (arPlayer[BF_Player[i]].iDifficulty<arPlayer[BF_Player[1-i]].iDifficulty)
			{
				LifeMod[i]=(10+arPlayer[BF_Player[i]].iDifficulty-arPlayer[BF_Player[1-i]].iDifficulty)*10;
			};		
		};	
		//Humanz
		if ((arPlayer[BF_Player[1-i]].Control==1)&&(BF_Player[i]==0))
		{
			LifeMod[i]=arDifficulty[arPlayer[BF_Player[1-i]].iDifficulty].iNeutralLife;		
		};	
	};
	G_Object TTT(NULL,0,0,0,NULL);
	G_Object *ObjTemp=&TTT;
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		for (int j=0; j<2; j++)
		{
			Squad[j][i].Clear();			
			if ((i<MAX_SQUAD)&&(Heroes[j]->pSquad[i]))
			{
				ObjTemp=Squad[j][i].Init(Heroes[j]->pSquad[i],ObjTemp,j,i);
				if ((Squad[j][i].y<0)||(Squad[j][i].y>7))
				{Squad[j][i].y=3;};
				if ((Squad[j][i].x<0)||(Squad[j][i].x>1))
				{Squad[j][i].x=0;};
				if (j==1)
				{Squad[j][i].x=7-Squad[j][i].x;};
				if (i==0)
				{
					Squad[j][i].pHero=Heroes[j];
				};			
			};
		};		
	};
	for (int Sd=0; Sd<2; Sd++)
	{
		for (int i=0; i<SPELLBOOK_SIZE; i++)
		{
			Spells[Sd][0][i]=0;
			Spells[Sd][1][i]=0;
		};
		int Nmbr=0;
		for (int i=1; i<=SPELLBOOK_SIZE; i++)
		{
			if (Heroes[Sd]->arSpell[i])
			{
				Spells[Sd][0][Nmbr]=Heroes[Sd]->arSpell[i];
				Spells[Sd][1][Nmbr]=1;
				Nmbr++;
			};
		};
	};
	if (Hero1->CheckHeroBonus(999)<Hero2->CheckHeroBonus(999))
	{AA=1;}
	else
	{AA=0;};
	AA=1-AA;	
	while((Loss(AA)>=0)&&(Loss(1-AA)>=0)&&(iTurn/2<=MAX_BATTLE_DURATION))
	{
		AA=1-AA;	
		if (Heroes[AA]->CheckHeroBonus(984))
		{bArchmage=true;}
		else
		{bArchmage=false;}
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			int iChangeSide=-1;
			if ((Squad[AA][i].iLife>0)&&(Squad[AA][i].bInBattle))
			{			
				Squad[AA][i].iSurvived++;	
				if (Squad[AA][i].CheckSkill(53)>0)
				{
					Squad[AA][i].bParry=true;
				};
				int Hit=0;
				//Отдых
				if ((Squad[AA][i].bResting)||((Squad[AA][i].bCanAct)&&(Squad[AA][i].iSpeed==Squad[AA][i].GetSpeed())))
				{
					Squad[AA][i].iStamina+=(2+Squad[AA][i].CheckSkill(21));
					if (Squad[AA][i].iStamina>Squad[AA][i].GetStamina())
					{Squad[AA][i].iStamina=Squad[AA][i].GetStamina();};
					Squad[AA][i].iAmmo+=Squad[AA][i].CheckSkill(22);
					if (Squad[AA][i].iAmmo>Squad[AA][i].GetAmmo())
					{Squad[AA][i].iAmmo=Squad[AA][i].GetAmmo();};
					Hit+=Squad[AA][i].CheckSkill(35);
					Squad[AA][i].bResting=false;
				};
				BF_Effect * Eff=Squad[AA][i].pEffect;
				BF_Effect * Eff1;
				//Таймеры эффектов
				while (Eff)
				{	
					if (Eff->iType==1)
					{
						Hit+=Eff->iPower;
					};				
					Eff1=Eff->pNext;
					if (Eff->iTime>0)
					{
						Eff->iTime--;
						if (Eff->iTime==0)
						{
							if (Eff->iType==73)
							{
								iChangeSide=Eff->iPower;
							};
							if (Eff->iType==90)
							{	
								Squad[AA][i].iLife=Squad[AA][i].iLife*Squad[AA][i].pTrueForm->GetLife()/Squad[AA][i].pUnit->GetLife();
								if (Squad[AA][i].iLife<=0)
								{Squad[AA][i].iLife=1;};
								delete Squad[AA][i].pUnit;
								Squad[AA][i].pUnit=Squad[AA][i].pTrueForm;
								if (Squad[AA][i].iLife>Squad[AA][i].GetLife())
								{Squad[AA][i].iLife=Squad[AA][i].GetLife();};
								if (Squad[AA][i].iAmmo>Squad[AA][i].GetAmmo())
								{Squad[AA][i].iAmmo=Squad[AA][i].GetAmmo();};
								if (Squad[AA][i].iSpeed>Squad[AA][i].GetSpeed())
								{Squad[AA][i].iSpeed=Squad[AA][i].GetSpeed();};								
							};
							if (Eff->pPrev)
							{
								delete Eff;
							}
							else
							{
								Squad[AA][i].pEffect=Eff->pNext;
								delete Eff;							
							};
						};
					};
					Eff=Eff1;				
				};

				Hit+=Squad[AA][i].CheckSkill(48);

				//Яд/регенерация
				if (Hit+Squad[AA][i].iLife<0)
				{
					Hit=-Squad[AA][i].iLife;
				};
				if ((Hit+Squad[AA][i].iLife>Squad[AA][i].GetLife())&&(Hit>0))
				{
					Hit=Squad[AA][i].GetLife()-Squad[AA][i].iLife;
				};
				if (Hit<0)
				{					
					Squad[AA][i].TakeDamage(abs(Hit),3);					
				};
				if (Hit>0)
				{
					Squad[AA][i].iLife+=Hit;				
				};
				if ((Squad[AA][i].iLife>0)&&(!Squad[AA][i].CheckSkill(36)))
				{
					Squad[AA][i].bCanAct=true;
					if (!Squad[AA][i].CheckSkill(88))
					{		
						Squad[AA][i].iSpeed=Squad[AA][i].GetSpeed();							
						Squad[AA][i].bMarch=true;	
					}
					else
					{Squad[AA][i].bMarch=false;};
					if (Squad[AA][i].iStamina<1)
					{
						Squad[AA][i].bCanAct=false;
						Squad[AA][i].bResting=true;
						Squad[AA][i].iSpeed=0;
					};
					if (Squad[AA][i].iMorale<=0)
					{
						if (Squad[AA][i].iFear<60)
						{
							if (bSimulate)
							{
								if (Squad[AA][i].iFear<30)
								{
									Squad[AA][i].ChangeMorale(1);
									Squad[AA][i].iFear=0;
								}
								else
								{
									Squad[AA][i].iFear-=10;
									Squad[AA][i].Panic();
								};
							}
							else
							{
								if (Random(100)>Squad[AA][i].iFear)
								{
									Squad[AA][i].ChangeMorale(1);
									Squad[AA][i].iFear=0;
								}
								else
								{
									Squad[AA][i].iFear-=10;
									Squad[AA][i].Panic();
								};
							};
						}
						else
						{
							Squad[AA][i].iFear-=10;
							Squad[AA][i].Panic();
						};				
					};
				};
	
				if (iChangeSide>=0)
				{
					Squad[AA][i].ChangeSide(iChangeSide);	
				};			
			};		
		};
	
		iTurn++;
		ActiveUnit=NULL;
		AI_SimulateDangerZone();
		AI_SimulatePower();		
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if ((Squad[AA][i].pUnit)&&(Squad[AA][i].iLife>0)&&(Squad[AA][i].bCanAct)&&(Squad[AA][i].bInBattle))
			{
				ActiveUnit=&Squad[AA][i];
				AI_Simulate();
			};
		};

	};	
	if (bAIBattle)
	{
		if ((Loss(0)<0)&&(Loss(1)>=0))
		{		
			EndBattle(1, 0);
			Result=0;		
		};

		if ((Loss(0)>=0)&&(Loss(1)<0))
		{
			EndBattle(0, 0);
			Result=1;		
		};

		if ((Loss(0)<0)&&(Loss(1)<0))
		{
			EndBattle(-1, 0);
			Result=-1;		
		};

		if ((Loss(0)>=0)&&(Loss(1)>=0))
		{
			EndBattle(-1, 0);
			Result=-2;		
		};
	}
	else
	{	
	for (int i=0; i<2; i++)
	{arPlayer[BF_Player[i]].Gem+=GemSpent[i];};
	int Power[2], Power1[2];
	for (int i=0; i<2; i++)
	{
		Power[i]=1;
		Power1[i]=1;
		for (int j=0; j<MAX_SQUAD; j++)
		{
			if (Heroes[i]->pSquad[j])
			{
				int Price=0;
				if (Heroes[i]->pSquad[j]->pHero)
				{Price=10;}
				else
				{Price=Heroes[i]->pSquad[j]->pType()->iLevel;};
				if (Heroes[i]->pSquad[j]->iLife>0)
				{Power[i]+=Price;};
				if (Squad[i][j].iLife>0)
				{Power1[i]+=Price;};
			};
		};
	};
	Result=100+100*Power1[0]/Power[0]-100*Power1[1]/Power[1];
	};
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		for (int j=0; j<2; j++)
		{
			if (Squad[j][i].G_Obj==objSim)
			{
				Squad[j][i].G_Obj=NULL;
			};

			if (Squad[j][i].objBar==objSim)
			{
				Squad[j][i].objBar=NULL;
			};
			if (Squad[j][i].bPacked)
			{
				Squad[j][i].bPacked=false;
				delete Squad[j][i].pTrueForm;
			};
		};		
	};
	bSimulate=false;
	objMain=NULL;
	return Result;
};