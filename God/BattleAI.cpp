#include "Battlefield.h"

int iAISacVictim;


//Проверка соседней клетки для создания карты AI
bool TryAIXYmap(int x, int y, int Weight)
{	
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if (arGrid[x][y].Pass)
		{					
			if ((((Field[x][y].Land==0)||(Weight<=Field[x][y].Land))&&(pFirstBF_Obj+arGrid[x][y].Land)->iMoveCost))
			{
				return 1;				
			};
		};
	};
	return 0;
};

//Рекурсивная функция для создания карты поиска пути AI
bool bFirst;
int StepAIMap (int x, int y, int Weight, int Speed, int SpeedLeft, int StamCost, int StamLeft)
{
	if (bSimulate)
	{
		SimulateMap(x,y,Weight,Speed,SpeedLeft,false);
		return 0;
	};
	if (Weight<0)
	{
		Field[x][y].Land=Weight;
		Weight=1;
	}
	else
	{
		if (SpeedLeft<GetMoveCost(x,y))
		{
			if (Speed>SpeedLeft)
			{
				Weight++;
				SpeedLeft=Speed;
			};
		};
		if ((Weight>Field[x][y].Land)&&(Field[x][y].Land!=0))
		{
			return 0;
		};
		SpeedLeft-=GetMoveCost(x,y);
		StamCost+=GetStamCost(x,y);
		Field[x][y].Land=Weight;
		Field[x][y].Pass=1;
		if (SpeedLeft<=0)
		{
			Weight++;
			SpeedLeft=Speed;
		};
	};

	if (TryAIXYmap(x-1+y%2,y-1,Weight))
	{
		StepAIMap(x-1+y%2,y-1,Weight,Speed,SpeedLeft,StamCost,StamLeft);
	};
	if (TryAIXYmap(x+y%2,y-1,Weight))
	{
		StepAIMap(x+y%2,y-1,Weight,Speed,SpeedLeft,StamCost,StamLeft);
	};
	if (TryAIXYmap(x-1+y%2,y+1,Weight))
	{
		StepAIMap(x-1+y%2,y+1,Weight,Speed,SpeedLeft,StamCost,StamLeft);
	};
	if (TryAIXYmap(x+y%2,y+1,Weight))
	{
		StepAIMap(x+y%2,y+1,Weight,Speed,SpeedLeft,StamCost,StamLeft);
	};
	if (TryAIXYmap(x-1,y,Weight))
	{
		StepAIMap(x-1,y,Weight,Speed,SpeedLeft,StamCost,StamLeft);
	};
	if (TryAIXYmap(x+1,y,Weight))
	{
		StepAIMap(x+1,y,Weight,Speed,SpeedLeft,StamCost,StamLeft);
	};	
	return 0;
};

//Проверка соседней клетки для поиска пути AI

bool TryAIXYpath(int x, int y, int Weight)
{
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if ((arGrid[x][y].Pass)&&(Field[x][y].Pass)&&(Weight<=Field[x][y].Land))
		{			
			return 1;
		};
	};
	return 0;
};

//Поиск пути AI (без преобразования координат)

int StepAIPath (int x, int y, int Weight, clPath *Path, int x1, int y1, int Speed, int SpeedLeft, int StamCost, int StamLeft)
{
	if (bSimulate)
	{
		SimulatePath(x,y,Weight,Path,x1,y1,Speed,SpeedLeft,false);
		return 0;
	};
	if (Weight<0)
	{
		Weight=1;
	}
	else
	{
		if (SpeedLeft<GetMoveCost(x,y))
		{
			if (Speed>SpeedLeft)
			{
				Weight++;
				SpeedLeft=Speed;
			};
		};
		SpeedLeft-=GetMoveCost(x,y);
		StamCost+=GetStamCost(x,y);		
	}; 			
	clPath *pRoad=new clPath(x, y, Path);
	if ((x==x1)&&(y==y1))
	{
		if (((TruePath)&&(Weight<MinWeight))||((!TruePath)&&(Weight==MinWeight))) //Потом вес заменить на стамину
		{	
			clPath *P1, *P2, *P3;
			if (TruePath)
			{
				P1=TruePath;
				while (P1)
				{
					P2=P1->pPrev;
					delete P1;
					P1=P2;
				};
			};
			P1=new clPath(x, y, NULL);
			P2=pRoad->pPrev; 			
			while (P2)
			{
				P3=new clPath(P2->x, P2->y, P1);
				P2=P2->pPrev;
				P1=P3;
			};
			TruePath=P1;
			MinWeight=Weight;
			MinStamina=StamCost;
		};
		delete pRoad;
		return 1;
	};
	if (SpeedLeft<=0)
	{
		Weight++;
		SpeedLeft=Speed;
		if (Weight>MinWeight)
		{
			delete pRoad;
			return 0;
		};
	};
	if (TryAIXYpath(x-1+y%2,y-1,Weight))
	{
		StepAIPath(x-1+y%2,y-1,Weight,pRoad,x1,y1,Speed,SpeedLeft,StamCost,StamLeft);
	};
	if (TryAIXYpath(x+y%2,y-1,Weight))
	{
		StepAIPath(x+y%2,y-1,Weight,pRoad,x1,y1,Speed,SpeedLeft,StamCost,StamLeft);
	};
	if (TryAIXYpath(x-1+y%2,y+1,Weight))
	{
		StepAIPath(x-1+y%2,y+1,Weight,pRoad,x1,y1,Speed,SpeedLeft,StamCost,StamLeft);
	};
	if (TryAIXYpath(x+y%2,y+1,Weight))
	{
		StepAIPath(x+y%2,y+1,Weight,pRoad,x1,y1,Speed,SpeedLeft,StamCost,StamLeft);
	};
	if (TryAIXYpath(x-1,y,Weight))
	{
		StepAIPath(x-1,y,Weight,pRoad,x1,y1,Speed,SpeedLeft,StamCost,StamLeft);
	};
	if (TryAIXYpath(x+1,y,Weight))
	{
		StepAIPath(x+1,y,Weight,pRoad,x1,y1,Speed,SpeedLeft,StamCost,StamLeft);
	};
	delete pRoad;
	return 0;
};

bool TryFly[8][8];

//Проверка соседней клетки для создания карты (для летающих юнитов)

int FlyAIMap (int x, int y, int Weight, int Speed, int SpeedLeft, bool Over);

void FlyAIXYmap(int xp, int yp, int x, int y, int Weight, int Speed, int SpeedLeft, bool Over)
{
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if (((pFirstBF_Obj+arGrid[x][y].Land)->iMoveCost)||Over)
		{			
			if ((Field[x][y].Land==0)||(Weight<Field[x][y].Land)||((Weight==Field[x][y].Land)&&(TryFly[x][y])))
			{
				if (arGrid[x][y].Pass)
				{
					if (Speed==SpeedLeft)
					{TryFly[x][y]=false;}else{TryFly[x][y]=true;};
				};
				FlyAIMap (x, y, Weight, Speed, SpeedLeft, Over);
				if ((SpeedLeft<Speed)&&(!arGrid[x][y].Pass)&&(arGrid[xp][yp].Pass)&&(TryFly[x][y]))
				{
					TryFly[x][y]=false;
					FlyAIMap (x, y, Weight+1, Speed, Speed, Over);					
				};
			};
		};
	};
};


//Рекурсивная функция для создания карты поиска пути для летающих юнитов

int FlyAIMap (int x, int y, int Weight, int Speed, int SpeedLeft, bool Over)
{
	if (bSimulate)
	{
		SimulateMap(x,y,Weight,Speed,SpeedLeft,true);
		return 0;
	};
	if (Weight<0)
	{
		Field[x][y].Land=Weight;
		Weight=1;
		TryFly[x][y]=false;
//		SpeedLeft=0;
	}
	else
	{		
		SpeedLeft--;
		if ((arGrid[x][y].Pass)||(!TryFly[x][y]))
		{
			Field[x][y].Land=Weight;
		};		
		Field[x][y].Pass=1;
		if (SpeedLeft<=0)
		{			
			if (!arGrid[x][y].Pass)
			{
				return 0;
			};		
			Weight++;
			SpeedLeft=Speed;
		};		
	};
	FlyAIXYmap(x,y,x-1+y%2,y-1,Weight,Speed,SpeedLeft,Over);	
	FlyAIXYmap(x,y,x+y%2,y-1,Weight,Speed,SpeedLeft,Over);
	FlyAIXYmap(x,y,x-1+y%2,y+1,Weight,Speed,SpeedLeft,Over);	
	FlyAIXYmap(x,y,x+y%2,y+1,Weight,Speed,SpeedLeft,Over);	
	FlyAIXYmap(x,y,x-1,y,Weight,Speed,SpeedLeft,Over);
	FlyAIXYmap(x,y,x+1,y,Weight,Speed,SpeedLeft,Over);
	
	return 0;
};

//Проверка соседней клетки для поиска пути (для летающих юнитов)

int FlyAIPath (int x, int y, int Weight, clPath *Path, int x1, int y1, int Speed, int SpeedLeft);

void FlyAIXYpath(int xp, int yp, int x, int y, int Weight, clPath *Path, int x1, int y1, int Speed, int SpeedLeft)
{
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if ((Field[x][y].Pass)&&((Weight<Field[x][y].Land)||(Field[x][y].Land==0)||((Weight==Field[x][y].Land)&&(TryFly[x][y]))))
		{
			if (arGrid[x][y].Pass)
			{
				if (Speed==SpeedLeft)
				{TryFly[x][y]=false;}else{TryFly[x][y]=true;};
			};
			FlyAIPath (x, y, Weight, Path, x1, y1, Speed, SpeedLeft);
			if ((SpeedLeft<Speed)&&(!arGrid[x][y].Pass)&&(arGrid[xp][yp].Pass)&&(TryFly[x][y]))
			{
				TryFly[x][y]=false;
				FlyAIPath (x, y, Weight+1, Path, x1, y1, Speed, Speed);
			};		
		};
	};	
};

//void FlyAIXYpath(int xp, int yp, int x, int y, int Weight, clPath *Path, int x1, int y1, int Speed, int SpeedLeft)
//{
//	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
//	{
//		if ((Field[x][y].Pass)&&((Weight<=Field[x][y].Land)||(Field[x][y].Land==0)))
//		{			
//			FlyAIPath (x, y, Weight, Path, x1, y1, Speed, SpeedLeft);
//			if ((SpeedLeft<Speed)&&(!arGrid[x][y].Pass)&&(arGrid[xp][yp].Pass))
//			{				
//				FlyAIPath (x, y, Weight+1, Path, x1, y1, Speed, Speed);
//			};		
//		};
//	};	
//};

//Поиск пути для летающих юнитов без преобразования координат

int FlyAIPath (int x, int y, int Weight, clPath *Path, int x1, int y1, int Speed, int SpeedLeft)
{
	if (bSimulate)
	{
		SimulatePath(x,y,Weight,Path,x1,y1,Speed,SpeedLeft,true);
		return 0;
	};
	if (Weight<0)
	{		
		Weight=1;
		TryFly[x][y]=false;
//		SpeedLeft=0;
	}
	else
	{		
		SpeedLeft--;		
		if (SpeedLeft<=0)
		{			
			if (!arGrid[x][y].Pass)
			{
				return 0;
			};
			Weight++;			
			SpeedLeft=Speed;			
		};
		if (Weight>MinWeight+1)
		{
			return 0;
		};
	}; 
	clPath *pRoad=new clPath(x, y, Path);
	if ((x==x1)&&(y==y1))
	{
		if (((TruePath)&&(Weight<MinWeight))||((!TruePath)&&(Weight<=MinWeight+1)))
		{	
			clPath *P1, *P2, *P3;
			if (TruePath)
			{
				P1=TruePath;
				while (P1)
				{
					P2=P1->pPrev;
					delete P1;
					P1=P2;
				};
			};
			P1=new clPath(x, y, NULL);
			P2=pRoad->pPrev; 			
			while (P2)
			{
				P3=new clPath(P2->x, P2->y, P1);
				P2=P2->pPrev;
				P1=P3;
			};
			TruePath=P1;
			MinWeight=Weight;
			MinStamina=0;
		};
		delete pRoad;
		return 1;
	};
	FlyAIXYpath(x,y,x-1+y%2,y-1,Weight,pRoad,x1,y1,Speed,SpeedLeft);
	FlyAIXYpath(x,y,x+y%2,y-1,Weight,pRoad,x1,y1,Speed,SpeedLeft);
	FlyAIXYpath(x,y,x-1+y%2,y+1,Weight,pRoad,x1,y1,Speed,SpeedLeft);
	FlyAIXYpath(x,y,x+y%2,y+1,Weight,pRoad,x1,y1,Speed,SpeedLeft);
	FlyAIXYpath(x,y,x-1,y,Weight,pRoad,x1,y1,Speed,SpeedLeft);
	FlyAIXYpath(x,y,x+1,y,Weight,pRoad,x1,y1,Speed,SpeedLeft);	
	delete pRoad;
	return 0;
};


//Стандартные действия после вызова заклинания AI
void SpellCastAI(clSpell *Spell)
{
	ActiveUnit->G_Obj->AnimText(Spell->sName,makecol(200,60,250));
	ActiveUnit->G_Obj->AnimIcon(Spell->Pic,3+arHumans[arPlayer[ActivePlayer].iColor].iBattleSpeed/2);	
	if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
	{
		arPlayer[BF_Player[AA]].Gem-=Spell->iCost*(100-ActiveUnit->pHero->CheckHeroBonus(983))/100;
		GemSpent[AA]+=Spell->iCost*(100-ActiveUnit->pHero->CheckHeroBonus(983))/100;
		arPlayer[BF_Player[AA]].iKarma+=Spell->iKarma;
		for (int i=0; i<SPELLBOOK_SIZE; i++)
		{
			if (((pFirstSpell+Spells[AA][0][i])==Spell)&&(Spells[AA][1][i]>0))
			{
				Spells[AA][1][i]=0;
				break;
			};
		};
	}
	else
	{
		int Index=0;
		for (int i=0; i<=iMaxSpell; i++)
		{
			if ((pFirstSpell+i)==Spell)
			{
				Index=i;
			};
		};		
		ActiveUnit->iAmmo-=ActiveUnit->CheckSkill(2000+Index);
	};		
	if (ActiveUnit->iLife>0)
	{	
		ActiveUnit->UseStamina(Spell->iStamCost);
		if ((bArchmage)&&(ActiveUnit->pHero)&&(ActiveUnit->iStamina>0))
		{
//			SelectUnitAI(ActiveUnit);		
		}
		else
		{
			ActiveUnit->bCanAct=false;
			ActiveUnit->iSpeed=0;
		};
		if (Spell->iLifeCost>0)
		{	
			int Hit=Spell->iLifeCost;
			if (Hit>ActiveUnit->iLife)
			{
				Hit=ActiveUnit->iLife;
			};
			int iType=ActiveUnit->pUnit->GetUnitType();
			if (ActiveUnit->TakeDamage(Hit,3))
			{ActiveUnit->G_Obj->AnimDeath((pFirstUnitType+iType)->wavDeath);};
//Добавить задержку перед вылетом цифирки
			if (bShowAnim)
			{
				ActiveUnit->G_Obj->pLink->pEffQue=new clEffQue(1,10,ActiveUnit->G_Obj);
				ActiveUnit->G_Obj->pLink->pEffQue->iNum=Hit;
				ActiveUnit->G_Obj->pLink->iTimer=15;
				ActiveUnit->G_Obj->pLink->pEffQue->iColor=makecol(250,50,40);
		//		Squad[AA][0].G_Obj->AnimNumber(Hit, makecol(200,50,40));		
			};
		};
	};
	if ((bArchmage)&&(ActiveUnit->pHero))
	{
		bArchmage=false;
	};	
	if (Spell->bSacrifice)
	{
		if (Squad[AA][iAISacVictim].iLife>0)
		{
			Squad[AA][iAISacVictim].iLife=0;			
			Squad[AA][iAISacVictim].G_Obj->AnimDeath(Squad[AA][iAISacVictim].pUnit->pType()->wavDeath);
			Squad[AA][iAISacVictim].Death();
		};
	};
	ActiveUnit->DrawBar();
};

//Выбор активного юнита АИ
void AI_SelectUnit(BF_Unit *Unit, bool CanRun)
{
	for (int i=0; i<8; i++){
		for (int j=0; j<8; j++)
		{
			Field[i][j].Land=0;
			Field[i][j].Pass=0;
			Field[i][j].Shoot=0;
		};
	};

//Построение карты дальности выстрела

	if ((Unit->GetShootingRange())&&(Unit->iAmmo))
	{
		ShootMap(Unit->x,Unit->y,-1,Unit->GetShootingRange());
		for (int i=0; i<8; i++)
		{
			for (int j=0; j<8; j++)
			{
				Field[i][j].Land=0;
			};
		};
	};

//Построение карты поиска пути
	for (int i=0; i<3; i++)
	{CanPass[i]=0;};

	bFirstMove=(Unit->iSpeed>=Unit->GetSpeed());
	if ((!Unit->bMarch)&&(Unit->CheckSkill(29)>0)&&(Unit->iSpeed<=Unit->GetSpeed()))
	{bFirstMove=false;};

	if (Unit->CheckSkill(32))
	{CanPass[0]=2;};
	if (Unit->CheckSkill(33))
	{CanPass[1]=3;};
	if (Unit->CheckSkill(34))
	{CanPass[2]=4;};
	if (Unit->CheckSkill(29)==0)
	{Unit->bMarch=false;};


	if ((Unit->iSpeed>0)&&(Unit->iStamina>0))
	{
		int Speed=Unit->iSpeed;
		int Stam=Unit->iStamina-Unit->GetFatigure();
		if ((Unit->bMarch)&&(Unit->CheckSkill(29)<Stam)&&(CanRun))
		{
			Speed++;
			Stam-=Unit->CheckSkill(29);
		};
		if (Unit->CheckSkill(14)) //Летающий
		{
			FlyMap(Unit->x,Unit->y,-1,Speed,true);
		}
		else
		{
			if (Unit->CheckSkill(15)) //Низколетающий
			{
				FlyMap(Unit->x,Unit->y,-1,Speed,false);
			}
			else
			{				
				StepMap(Unit->x,Unit->y,-1,Speed,0,Stam);
			};
		};
	};

	if (Unit!=ActiveUnit)
	{
		ActiveUnit=Unit;
	};
};

//Проверка, может ли юнит ударить с указанного тайла при текущей скорости
int AI_InMeleeRange(int x, int y)
{	
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if ((ActiveUnit->x==x)&&(ActiveUnit->y==y))
		{
			return 3;
		};
		if ((arGrid[x][y].Pass)&&(Field[x][y].Pass))
		{
			if ((Field[x][y].Land>ActiveUnit->iSpeed)&&(ActiveUnit->bMarch)&&(!ActiveUnit->IsNeighbour(x,y)))
			{
				if (ActiveUnit->iStamina-ActiveUnit->CheckSkill(29)>=5)
				{return 1;}
				else
				{return 0;};
			}
			else
			{return 2;};			
		};
	};
	return 0;
};

//Проверка, может ли юнит ударить с указанного тайла при максимуме скорости
int AI_InMeleeRangePower(int x, int y)
{	
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if ((ActiveUnit->x==x)&&(ActiveUnit->y==y))
		{
			return 3;
		};
		if ((arGrid[x][y].Pass)&&(Field[x][y].Pass))
		{
			if ((Field[x][y].Land>ActiveUnit->GetSpeed())&&(ActiveUnit->bMarch)&&(!ActiveUnit->IsNeighbour(x,y)))
			{return 1;}
			else
			{return 2;};			
		};
	};
	return 0;
};

int AI_CalcMeleeHit(BF_Unit * Attacker, BF_Unit * Defender)
{	
	if ((Defender->CheckSkill(16))&&(!Defender->bResting)&&(!Defender->CheckSkill(36))&&(Defender->iStamina>0)&&(!Attacker->CheckSkill(26))&&(!Attacker->CheckSkill(16))&&(GetMeleeHit(Defender,Attacker,0)>=Attacker->iLife))
	{
		return -100;
	}
	else
	{
		return (GetMeleeHit(Attacker,Defender,1));
	};
};

int AI_CalcRangedHit(BF_Unit * Attacker, BF_Unit * Defender)
{
	return GetRangedHit(Attacker,Defender);
};

//Возвращает истину, если ActiveUnit тайл x,y нравится больше чем тайл x1,y1
bool AI_BetterPlace(int x, int y, int x1, int y1, int mov, int mov1) 
{
	//Проверка соседей для круговой атаки
	if ((ActiveUnit->CheckSkill(65)>0)||((ActiveUnit->CheckSkill(66)>0)&&(ActiveUnit->iStamina>ActiveUnit->CheckSkill(66))))
	{
		int OldX=ActiveUnit->x;
		int OldY=ActiveUnit->y;
		ActiveUnit->x=x; ActiveUnit->y=y;
		int Neig=ActiveUnit->FindNeighbour(1-ActiveUnit->iSide);
		ActiveUnit->x=x1; ActiveUnit->y=y1;
		int Neig1=ActiveUnit->FindNeighbour(1-ActiveUnit->iSide);
		ActiveUnit->x=OldX; ActiveUnit->y=OldY;
		if (Neig>Neig1)
		{return true;};
	};
	//Проверка знания местности
	if ((ActiveUnit->CheckSkill(32)>3)&&(arGrid[x][y].Land==2)&&(arGrid[x1][y1].Land!=2))
	{return true;};
	if ((ActiveUnit->CheckSkill(33)>3)&&(arGrid[x][y].Land==3)&&(arGrid[x1][y1].Land!=3))
	{return true;};
	if ((ActiveUnit->CheckSkill(34)>3)&&(arGrid[x][y].Land==4)&&(arGrid[x1][y1].Land!=4))
	{return true;};
	//Проверка затрат хода
	if (ActiveUnit->CheckSkill(18)==0)
	{
		if (mov1>mov)
		{return true;};
		if (mov1<mov)
		{return false;};
	};
	//Проверка холмов
	if ((arGrid[x][y].Land==3)&&(arGrid[x1][y1].Land!=3))
	{return true;};
	//Проверка близости к вражеским тылам
	if ((AA==0)&&(x>x1))
	{return true;};
	if ((AA==1)&&(x<x1))
	{return true;};	
	return false;
};

//Массив возможностей юнитов
int arUnitPower[2][MAX_SQUAD+MAX_SUMMONS][7]; //0-лечение, 1-стрельба, 2-рукопашная, 3-под магическим обстрелом, 4-под обстрелом, 5-под магической атакой, 6-под атакой

//Оценка возможностей юнитов на данном ходу
void AI_CalcPower()
{
	int MoveMap [8][8];
	for (int Side=0; Side<2; Side++)
	{
		for (int Unit=0; Unit<MAX_SQUAD+MAX_SUMMONS; Unit++)
		{
			for (int i=0; i<7; i++)
			{
				arUnitPower[Side][Unit][i]=0;
			};
		};
	};
	for (int Side=0; Side<2; Side++)
	{
		for (int Unit=0; Unit<MAX_SQUAD+MAX_SUMMONS; Unit++)
		{
			if ((Squad[Side][Unit].pUnit)&&(Squad[Side][Unit].iLife>0))
			{
				AI_SelectUnit(&Squad[Side][Unit],true);
		if (ActiveUnit->CheckSkill(24))
		{
			AI_SelectUnit(ActiveUnit,true);			
		//Поиск целей с места
			for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
			{
				if ((Squad[Side][i].iLife>0)&&(Field[Squad[Side][i].x][Squad[Side][i].y].Shoot)
					&&(Squad[Side][i].iLife<Squad[Side][i].GetLife())
					&&(Squad[Side][i].GetUnitKind()!=2)&&(Squad[Side][i].GetUnitKind()!=4))
				{
					if (ActiveUnit->iAmmo>0)
					{
						arUnitPower[Side][Unit][0]=2;
					}
					else
					{
						arUnitPower[Side][Unit][0]=1;
					};
				};
			};
		//Карта проходимости
			if ((ActiveUnit->GetSpeed()>1)&&(arUnitPower[Side][Unit][0]==0))
			{
			for (int i=0; i<8; i++){
			for (int j=0; j<8; j++){
				MoveMap[i][j]=0;};};
			for (int i=0; i<8; i++)
			{
				for (int j=0; j<8; j++)
				{
					if ((Field[i][j].Land>0)&&(Field[i][j].Land<=ActiveUnit->GetSpeed())&&(Field[i][j].Pass)&&(arGrid[i][j].Pass))
					{
						if ((Field[i][j].Land==ActiveUnit->GetSpeed())&&(ActiveUnit->bMarch)&&(MoveMap[i][j]<1))
						{
							MoveMap[i][j]=1;
						};
						if (Field[i][j].Land<ActiveUnit->iSpeed)
						{
							MoveMap[i][j]=2;
						};
					};					
				};
			};
			for (int x=0; x<8; x++)
			{
			for (int y=0; y<8; y++)
			{
			if (MoveMap[x][y]>0)
			{
				for (int i=0; i<8; i++){
				for (int j=0; j<8; j++)
				{
					Field[i][j].Land=0;
					Field[i][j].Pass=0;
					Field[i][j].Shoot=0;
				};
				};

				//Влияние холмов на дальность
				int CurrRange=ActiveUnit->GetShootingRange();
				if ((ActiveUnit->GetShootingRange()>1)&&(ActiveUnit->CheckSkill(14)<=0))
				{
					if ((arGrid[ActiveUnit->x][ActiveUnit->y].Land==3)&&(arGrid[x][y].Land!=3))
					{
						CurrRange--;
					};
					if ((arGrid[ActiveUnit->x][ActiveUnit->y].Land!=3)&&(arGrid[x][y].Land==3))
					{
						CurrRange++;
					};
				}

				//Поиск целей с подходом
				ShootMap(x,y,-1,CurrRange);
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{		
					if ((Squad[Side][i].iLife>0)&&(Field[Squad[Side][i].x][Squad[Side][i].y].Shoot)
					&&(Squad[Side][i].iLife<Squad[Side][i].GetLife())
					&&(Squad[Side][i].GetUnitKind()!=2)&&(Squad[Side][i].GetUnitKind()!=4))
					{
						if (ActiveUnit->iAmmo>0)
						{
							arUnitPower[Side][Unit][0]=2;
						}
						else
						{
							arUnitPower[Side][Unit][0]=1;
						};
					};
				};
			};
			};
			};			
			};
		};

//Стрельба
//********
		
		//Поиск целей с места		
		if (ActiveUnit->GetRangedAttack()>0)
		{
			AI_SelectUnit(ActiveUnit,true);
			for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
			{		
				if ((Squad[1-Side][i].pUnit)&&(Squad[1-Side][i].iLife>0)&&(Field[Squad[1-Side][i].x][Squad[1-Side][i].y].Shoot>1))
				{
					if (ActiveUnit->iAmmo>0)
					{
						arUnitPower[Side][Unit][1]=2;
						if (ActiveUnit->CheckSkill(28))
						{
							arUnitPower[1-Side][i][3]=1;
						}
						else
						{
							arUnitPower[1-Side][i][4]=1;
						};
					}
					else
					{
						arUnitPower[Side][Unit][1]=1;
					};
				};
			};
			//Карта проходимости
			if (ActiveUnit->GetSpeed()>1)
			{
			for (int i=0; i<8; i++){
			for (int j=0; j<8; j++){
				MoveMap[i][j]=0;};};
			for (int i=0; i<8; i++)
			{
				for (int j=0; j<8; j++)
				{
					if ((Field[i][j].Land>0)&&(Field[i][j].Land<=ActiveUnit->GetSpeed())&&(Field[i][j].Pass)&&(arGrid[i][j].Pass))
					{
						if ((Field[i][j].Land==ActiveUnit->GetSpeed())&&(ActiveUnit->bMarch)&&(MoveMap[i][j]<1))
						{
							MoveMap[i][j]=1;
						};
						if (Field[i][j].Land<ActiveUnit->GetSpeed())
						{
							MoveMap[i][j]=2;
						};
					};					
				};
			};

			for (int x=0; x<8; x++)
			{
			for (int y=0; y<8; y++)
			{
			if (MoveMap[x][y]>0)
			{
				for (int i=0; i<8; i++){
				for (int j=0; j<8; j++)
				{
					Field[i][j].Land=0;
					Field[i][j].Pass=0;
					Field[i][j].Shoot=0;
				};
				};
				//Влияние холмов на дальность
				int CurrRange=ActiveUnit->GetShootingRange();
				if ((ActiveUnit->GetShootingRange()>1)&&(ActiveUnit->CheckSkill(14)<=0))
				{
					if ((arGrid[ActiveUnit->x][ActiveUnit->y].Land==3)&&(arGrid[x][y].Land!=3))
					{
						CurrRange--;
					};
					if ((arGrid[ActiveUnit->x][ActiveUnit->y].Land!=3)&&(arGrid[x][y].Land==3))
					{
						CurrRange++;
					};
				};
				//Поиск целей с подходом
				ShootMap(x,y,-1,CurrRange);
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{		
					if ((Squad[1-Side][i].pUnit)&&(Squad[1-Side][i].iLife>0)&&(Field[Squad[1-Side][i].x][Squad[1-Side][i].y].Shoot>1))
					{
						if (ActiveUnit->iAmmo>0)
						{
							arUnitPower[Side][Unit][1]=2;
							if (ActiveUnit->CheckSkill(28))
							{
								arUnitPower[1-Side][i][3]=1;
							}
							else
							{
								arUnitPower[1-Side][i][4]=1;
							};
						}
						else
						{
							arUnitPower[Side][Unit][1]=1;
						};
					};
				};
			};
			};
			};			
			};
		};

//Рукопашная
//**********
		AI_SelectUnit(ActiveUnit,true);
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if ((Squad[1-Side][i].pUnit)&&(Squad[1-Side][i].iLife))
			{
				if (AI_InMeleeRangePower(Squad[1-Side][i].x-1+Squad[1-Side][i].y%2,Squad[1-Side][i].y-1)>0)
				{
					arUnitPower[Side][Unit][2]=2;
					if (ActiveUnit->CheckSkill(27))
					{arUnitPower[1-Side][i][5]=1;}
					else
					{arUnitPower[1-Side][i][6]=1;};
				};
				if (AI_InMeleeRangePower(Squad[1-Side][i].x+Squad[1-Side][i].y%2,Squad[1-Side][i].y-1)>0)
				{					
					arUnitPower[Side][Unit][2]=2;
					if (ActiveUnit->CheckSkill(27))
					{arUnitPower[1-Side][i][5]=1;}
					else
					{arUnitPower[1-Side][i][6]=1;};
				};
				if (AI_InMeleeRangePower(Squad[1-Side][i].x+1,Squad[1-Side][i].y)>0)
				{					
					arUnitPower[Side][Unit][2]=2;
					if (ActiveUnit->CheckSkill(27))
					{arUnitPower[1-Side][i][5]=1;}
					else
					{arUnitPower[1-Side][i][6]=1;};
				};
				if (AI_InMeleeRangePower(Squad[1-Side][i].x+Squad[1-Side][i].y%2,Squad[1-Side][i].y+1)>0)
				{					
					arUnitPower[Side][Unit][2]=2;
					if (ActiveUnit->CheckSkill(27))
					{arUnitPower[1-Side][i][5]=1;}
					else
					{arUnitPower[1-Side][i][6]=1;};
				};
				if (AI_InMeleeRangePower(Squad[1-Side][i].x-1+Squad[1-Side][i].y%2,Squad[1-Side][i].y+1)>0)
				{					
					arUnitPower[Side][Unit][2]=2;
					if (ActiveUnit->CheckSkill(27))
					{arUnitPower[1-Side][i][5]=1;}
					else
					{arUnitPower[1-Side][i][6]=1;};
				};
				if (AI_InMeleeRangePower(Squad[1-Side][i].x-1,Squad[1-Side][i].y)>0)
				{					
					arUnitPower[Side][Unit][2]=2;
					if (ActiveUnit->CheckSkill(27))
					{arUnitPower[1-Side][i][5]=1;}
					else
					{arUnitPower[1-Side][i][6]=1;};
				};
			};
		};

			};
		};
	};
};

void AI_SimulatePower()
{
	for (int Side=0; Side<2; Side++)
	{
		for (int Unit=0; Unit<MAX_SQUAD+MAX_SUMMONS; Unit++)
		{
			for (int i=0; i<7; i++)
			{
				arUnitPower[Side][Unit][i]=0;
			};
		};
	};
	for (int Side=0; Side<2; Side++)
	{
		for (int Unit=0; Unit<MAX_SQUAD+MAX_SUMMONS; Unit++)
		{
			if ((Squad[Side][Unit].pUnit)&&(Squad[Side][Unit].iLife>0))
			{
				ActiveUnit=&Squad[Side][Unit];
		if (ActiveUnit->CheckSkill(24))
		{		
			if (ActiveUnit->iAmmo>0)
			{
				arUnitPower[Side][Unit][0]=2;
			}
			else
			{
				arUnitPower[Side][Unit][0]=1;
			};
		};		
	
//Стрельба
//********
		
		//Поиск целей с места		
		if (ActiveUnit->GetRangedAttack()>0)
		{			
			for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
			{		
				if ((Squad[1-Side][i].pUnit)&&(Squad[1-Side][i].iLife>0)&&(abs(Squad[1-Side][i].x-ActiveUnit->x)<ActiveUnit->GetShootingRange()+ActiveUnit->iSpeed))
				{
					if (ActiveUnit->iAmmo>0)
					{
						arUnitPower[Side][Unit][1]=2;
						if (ActiveUnit->CheckSkill(28))
						{
							arUnitPower[1-Side][i][3]=1;
						}
						else
						{
							arUnitPower[1-Side][i][4]=1;
						};
					}
					else
					{
						arUnitPower[Side][Unit][1]=1;
					};
				};
			};
		};

//Рукопашная
//**********
//		AI_SelectUnit(ActiveUnit);
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if ((Squad[1-Side][i].pUnit)&&(Squad[1-Side][i].iLife)&&(abs(Squad[1-Side][i].x-ActiveUnit->x)<=ActiveUnit->iSpeed+1))
			{
				arUnitPower[Side][Unit][2]=2;
				if (ActiveUnit->CheckSkill(27))
				{arUnitPower[1-Side][i][5]=1;}
				else
				{arUnitPower[1-Side][i][6]=1;};
			};
		};

			};
		};
	};
};

int DangerZone[8][8];

//Проверка соседней клетки для создания карты опасности
bool TryXYZone(int x, int y, int Weight)
{	
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if (DangerZone[x][y]>Weight)
		{					
			return 1;	
		};
	};
	return 0;
};

//Создание опасной зоны вокруг указанной точки
int StepZone (int x, int y, int Weight)
{
	DangerZone[x][y]=Weight;
	Weight++;
	if (TryXYZone(x-1+y%2,y-1,Weight))
	{
		StepZone(x-1+y%2,y-1,Weight);
	};
	if (TryXYZone(x+y%2,y-1,Weight))
	{
		StepZone(x+y%2,y-1,Weight);
	};
	if (TryXYZone(x-1+y%2,y+1,Weight))
	{
		StepZone(x-1+y%2,y+1,Weight);
	};
	if (TryXYZone(x+y%2,y+1,Weight))
	{
		StepZone(x+y%2,y+1,Weight);
	};
	if (TryXYZone(x-1,y,Weight))
	{
		StepZone(x-1,y,Weight);
	};
	if (TryXYZone(x+1,y,Weight))
	{
		StepZone(x+1,y,Weight);
	};	
	return 0;
};

//Вычисление зон близости к противнику
void AI_CalcDangerZone()
{
	for (int i=0; i<8; i++){
	for (int j=0; j<8; j++){
		DangerZone[i][j]=100;};};
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		if ((Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0))
		{
			StepZone(Squad[1-AA][i].x,Squad[1-AA][i].y,0);
		};
	};
};


//Вычисление зон близости к противнику
void AI_SimulateDangerZone()
{	
	int Max=0;
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		if ((Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0))
		{
			if (Squad[1-AA][i].x>Max)
			{Max=Squad[1-AA][i].x;};
		};
	};
	for (int i=0; i<=Max; i++){
	for (int j=0; j<8; j++){
		DangerZone[i][j]=0;};};

	for (int i=Max+1; i<8; i++){
	for (int j=0; j<8; j++){
		DangerZone[i][j]=i-Max;};};	
};

//Возвращает ценность юнита для общества
int AI_GetUnitTaste(BF_Unit *Unit)
{
	int Result=Unit->GetAttack()+Unit->GetCounterAttack()/2+Unit->GetRangedAttack()*Unit->iAmmo/4+Unit->CheckSkill(24)*Unit->iAmmo/4;
	Result+=Unit->pUnit->pType()->iLevel*2+Unit->pUnit->iLevel;
	if (Unit->pHero)
	{
		Result+=(Unit->pHero->ipClass1-1)*3+Unit->pHero->iLevel;
	};
	return Result;
};

//Возвращает пользу от перемещения дружественного юнита в указанный тайл
int GetTeleportTaste(BF_Unit *Unit, int x, int y)
{
	int Result=0;	
	if ((arGrid[x][y].Pass)&&((pFirstBF_Obj+arGrid[x][y].Land)->iMoveCost>0))
	{
		int OldX=Unit->x, OldY=Unit->y;
		Unit->x=x; Unit->y=y;
		int Neig=Unit->FindNeighbour(1-AA);
		if (Neig>0)
		{
			if ((Unit->CheckSkill(65)>0)||((Unit->CheckSkill(66)>0)&&(Unit->iStamina>=Unit->CheckSkill(66))))
			{
				for (int i=0; i<Neig; i++)
				{
					int Hit=AI_CalcMeleeHit(Unit,arNeigh[i]);
					if (arNeigh[i]->iLife<=Hit)
					{Hit=5+arNeigh[i]->iLife*2;};
					if (arNeigh[i]->iAmmo<2)
					{Hit/=2;};
					Result+=Hit;					
				};
			}
			else
			{
				int BigHit=0;
				for (int i=0; i<Neig; i++)
				{
					int Hit=AI_CalcMeleeHit(Unit,arNeigh[i]);
					if (arNeigh[i]->iLife<=Hit)
					{Hit=5+arNeigh[i]->iLife*2;};
					if (arNeigh[i]->iAmmo<2)
					{Hit/=2;};
					if (BigHit<Hit)
					BigHit=Hit;					
				};
				Result=BigHit;
			};
//			StopLog("Телепорт:",Result);
		};
		Unit->x=OldX; Unit->y=OldY;
	};	
	return Result;
};

//Возвращает пользу от перемещения дружественного юнита в указанный тайл в режиме симуляции
int GetTeleportTaste(BF_Unit *Unit, int x)
{
	int Result=0;
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		if ((Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0)&&(abs(Squad[1-AA][i].x-x)<2))	
		{
			int Hit=AI_CalcMeleeHit(Unit,&Squad[1-AA][i]);
			if (Squad[1-AA][i].iLife<=Hit)
			{Hit=5+Squad[1-AA][i].iLife*2;};
			if (Squad[1-AA][i].iAmmo<2)
			{Hit/=2;};
			if (Result<Hit)
			Result=Hit;					
		};
	};		
	return Result;
};


int iGainSpellDamage;	//Хранит полученное в функции повреждение, наносимое заклинанием
//Вычисление эффективности заклинания на вражеском юните
int AI_GetEnemySpellTaste(int ipSpell, BF_Unit *Target, int Unit)
{
	clSpell *SP=pFirstSpell+ipSpell;
	iGainSpellDamage=0;
	bool bPosessed=true;
	int AntiSkill=0;
	for (int i=0; i<MAX_ANTI_EFFECT; i++)
	{
		AntiSkill+=SP->arAntiEffect[i]*Target->CheckSkill(SP->arAntiEffect[i]);
	};
	for (int i=0; i<MAX_UNIT_KIND; i++)
	{
		if (SP->arUnitKind[i]==Target->GetUnitKind())
		{AntiSkill=1;};
	};
	if ((AntiSkill)&&(Target->CheckSkill(205)!=ipSpell))
	{return 0;};
	BF_Effect *Eff;
	int Buff=0, Resist=0, SpellPower=0, Duration=0, AttBuff=0, BaseDamage=0, Hit, SelfHeal=0;
	bool bLifeDrain=false;	
	Resist=Target->GetResist()-ActiveUnit->CheckSkill(908);
	if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
	{Resist-=ActiveUnit->pHero->CheckHeroBonus(908);}	
	Resist=Resist*SP->iResistMod/100;
	if (Resist<0){Resist=0;};
	for (int i=0; i<MAX_SPELL_EFFECT; i++)
	{
		if (SP->arEffect[0][i]>0)
		{
			SpellPower=SP->arEffect[1][i];
			if (SP->arEffect[1][i]>0)
			{
				SpellPower=SpellPower+ActiveUnit->CheckSkill(905)*SP->iPowerMod/100;
				if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
				{
					SpellPower=SpellPower+ActiveUnit->pHero->CheckHeroBonus(905)*SP->iPowerMod/100;
				}
			}
			if (SP->arEffect[1][i]<0)
			{
				SpellPower=SpellPower-ActiveUnit->CheckSkill(905)*SP->iPowerMod/100;
				if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
				{
					SpellPower=SpellPower-ActiveUnit->pHero->CheckHeroBonus(905)*SP->iPowerMod/100;
				}
			}
			Duration=SP->arEffect[2][i];
			int Index=SP->arEffect[0][i];
			if (Duration>0)
			{
				Duration+=ActiveUnit->CheckSkill(906)*SP->iDurationMod/100;
				if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
				{
					Duration+=ActiveUnit->pHero->CheckHeroBonus(906)*SP->iDurationMod/100;
				};
				if (Index==1)
				{
					Index=-1;
				};
			};
			switch (Index)
			{
			case 0:	break;
			case 1:									
				if (SP->arEffect[2][i]==-1)
				{
					//Повреждение
					BaseDamage+=SpellPower;
					Hit=GetHit(SpellPower,Resist*SP->bResistPower+Target->GetDefence()*SP->bDefencePower);
					iGainSpellDamage+=Hit;
					if (((arUnitPower[1-AA][Unit][0]==2)||(arUnitPower[1-AA][Unit][1]==2))&&(arUnitPower[1-AA][Unit][3]==0)&&(arUnitPower[1-AA][Unit][4]==0)&&(arUnitPower[1-AA][Unit][5]==0)&&(arUnitPower[1-AA][Unit][6]==0))
					{
						AttBuff+=Target->GetRangedAttack()*Target->iAmmo/4;
						AttBuff+=Target->CheckSkill(24)*Target->iAmmo/4;
					};
					AttBuff+=Target->GetRangedAttack()*Target->iAmmo/5+(Target->GetAttack()+Target->GetDefence()+Target->GetRangedDefence())/2;
					if (bLifeDrain)
					{
						SelfHeal+=Hit;
						if (SelfHeal>ActiveUnit->GetLife()-ActiveUnit->iLife)
						{
							SelfHeal=ActiveUnit->GetLife()-ActiveUnit->iLife;
						};
					};							
				};
				break;
				case 10:
					if (SP->bNegative)
					{
						if (SP->arEffect[2][i]==-1)
						{
					//Сжигание снарядов
							BaseDamage+=SpellPower;
							Hit=GetHit(SpellPower,Resist*SP->bResistPower);
							if (Target->iAmmo<Hit)
							{
								Hit=Target->iAmmo;
							};
							if (Hit>0)
							{
								iGainSpellDamage=Hit;
								Buff+=Hit*(Target->GetRangedAttack()+Target->CheckSkill(24))/2;
								if (((arUnitPower[1-AA][Unit][0]==2)||(arUnitPower[1-AA][Unit][1]==2))&&(arUnitPower[1-AA][Unit][3]==0)&&(arUnitPower[1-AA][Unit][4]==0)&&(arUnitPower[1-AA][Unit][5]==0)&&(arUnitPower[1-AA][Unit][6]==0))
								{
									AttBuff+=Target->GetRangedAttack()*Target->iAmmo/4;
									AttBuff+=Target->CheckSkill(24)*Target->iAmmo/4;
								};
								AttBuff+=Target->GetRangedAttack()*Target->iAmmo/5+(Target->GetAttack()+Target->GetDefence()+Target->GetRangedDefence())/2;
							};													
						}
						else
						{
					//Постоянный эффект
						};
					};			
					break;
					case 11:
					if (!Target->CheckSkill(18))
					{
					if (SP->bNegative)
					{
						if (SP->arEffect[2][i]==-1)
						{
					//Понижение энергии
							Hit+=SpellPower-Resist*SP->bResistPower;
							if (Hit>0)
							{
								if (Target->iStamina<Hit)								
								{
									Hit=Target->iStamina*2;
								};
								if (arUnitPower[1-AA][Unit][0]==2)
								{
									Hit+=Hit*(Target->CheckSkill(24)/7);
								};
								if (arUnitPower[1-AA][Unit][1]==2)
								{
									Hit+=Hit*(Target->GetRangedAttack()/7);
								};
								if (arUnitPower[1-AA][Unit][2]==2)
								{
									Hit+=Hit*(Target->GetAttack()/7);
								};
								Buff+=Hit;
							};							
						}
						else
						{
					//Постоянный эффект
						};
					};
					};
					break;
				case 12:
					if (!Target->CheckSkill(19))
					{
					if (SP->bNegative)
					{
						if (SP->arEffect[2][i]==-1)
						{
					//Страх
							Hit=SpellPower-Resist*SP->bResistPower;
							if ((Hit>0)&&(Target->iMorale>0))
							{
								if (Target->iMorale<Hit)
								{
									Hit=20+Target->iMorale*4;
								}
								else
								{
									if (Target->iMorale<Hit+5)
									{
										Hit=Hit*3/2;
									}
								};
								if (arUnitPower[1-AA][Unit][0]==2)
								{
									Hit+=Hit*(Target->CheckSkill(24)/7);
								};
								if (arUnitPower[1-AA][Unit][1]==2)
								{
									Hit+=Hit*(Target->GetRangedAttack()/7);
								};
								if (arUnitPower[1-AA][Unit][2]==2)
								{
									Hit+=Hit*(Target->GetAttack()/7);
								};
								Buff+=Hit;
							};
						}
						else
						{
					//Постоянный эффект
						};
					};	
					};
					break;
				case 31:
					//Снятие чар
					SpellPower=SP->arEffect[1][i]+ActiveUnit->CheckSkill(908);
					if (ActiveUnit->pHero)
					{
						SpellPower=SpellPower+ActiveUnit->pHero->CheckHeroBonus(908);
					};			
					Eff=Target->pEffect;				
					while (Eff)
					{
						if ((Eff->pSpell)&&(!Eff->pSpell->bNegative))
						{
							if (Eff->iTime>SpellPower)
							{
								Buff+=SpellPower*Eff->iPower;								
							}
							else
							{
								Buff+=Eff->iTime*Eff->iPower;
							};							
						};
						if (Eff->iType==73)
						{
							Buff+=50;
							iGainSpellDamage+=Target->GetAttack()*(Eff->iTime-1);
							bPosessed=false;
						};
						if (Eff->iType==90)
						{
							Buff+=50;
							iGainSpellDamage+=Target->GetAttack()*(Eff->iTime-1);
						};
						Eff=Eff->pNext;
					};
				//Повреждения вызванным существам
					if ((Target->bSummoned)&&(SpellPower>Resist))
					{
						BaseDamage+=SpellPower;
						Hit=GetHit(SpellPower+SP->arEffect[1][i]*2,Resist);						
						iGainSpellDamage+=Hit;
						if (((arUnitPower[1-AA][Unit][0]==2)||(arUnitPower[1-AA][Unit][1]==2))&&(arUnitPower[1-AA][Unit][3]==0)&&(arUnitPower[1-AA][Unit][4]==0)&&(arUnitPower[1-AA][Unit][5]==0)&&(arUnitPower[1-AA][Unit][6]==0))
						{
							AttBuff+=Target->GetRangedAttack()*Target->iAmmo/4;
							AttBuff+=Target->CheckSkill(24)*Target->iAmmo/4;
						};
						AttBuff+=Target->GetRangedAttack()*Target->iAmmo/5+(Target->GetAttack()+Target->GetDefence()+Target->GetRangedDefence())/2;
					};			
					break;
				//Паутина
				case 47:
					if ((Resist<4)&&(!Target->CheckSkill(36)))
					{
						if (Duration>Target->GetAttack())
							{Duration=2;}
						else
							{Duration=1;};
						if ((arUnitPower[1-AA][Unit][5]>0)||(arUnitPower[1-AA][Unit][6]>0))
						{
							Buff+=Target->GetCounterAttack()*Duration;
							if (Target->pHero)
							{Buff+=10;};
						};
					};
					break;				
				case 71:
					bLifeDrain=true;
					break;
//				case 72:
//					bFriendlyFire=true;
//					break;
				case 73:
				//Подчинение
					if (SP->arEffect[2][i]==-1)
					{
						if ((SpellPower>Resist)&&(!Target->CheckSkill(36))&&
							((arUnitPower[1-AA][Unit][0]==2)||(arUnitPower[1-AA][Unit][1]==2)||(arUnitPower[1-AA][Unit][2]==2)))
						{
							iGainSpellDamage+=Target->GetAttack()+Target->GetRangedAttack();
							Buff+=Target->GetAttack()+Target->GetRangedAttack();
						};
					}
					else
					{
						Duration-=Resist*SP->bResistDuration;
						if ((Duration>0)&&(!Target->CheckSkill(36))&&
							((arUnitPower[1-AA][Unit][0]==2)||(arUnitPower[1-AA][Unit][1]==2)||(arUnitPower[1-AA][Unit][2]==2)))
						{
							iGainSpellDamage+=(Target->GetAttack()+Target->GetRangedAttack())*Duration;
							Buff+=Target->GetAttack()+Target->GetRangedAttack()+Target->iLife;
						};
					};						
				break;
				case 92:
				//Суицид
					if (Resist*SP->bResistPower<=SpellPower)
					{
						iGainSpellDamage+=GetMeleeHit(Target,Target,true);
						if (((arUnitPower[1-AA][Unit][0]==2)||(arUnitPower[1-AA][Unit][1]==2))&&(arUnitPower[1-AA][Unit][3]==0)&&(arUnitPower[1-AA][Unit][4]==0)&&(arUnitPower[1-AA][Unit][5]==0)&&(arUnitPower[1-AA][Unit][6]==0))
						{
							AttBuff+=Target->GetRangedAttack()*Target->iAmmo/4;
							AttBuff+=Target->CheckSkill(24)*Target->iAmmo/4;
						};
						AttBuff+=Target->GetRangedAttack()*Target->iAmmo/5+(Target->GetAttack()+Target->GetDefence()+Target->GetRangedDefence())/2;						
					};
			default:
				Duration-=Resist*SP->bResistDuration;
				BF_Effect *Eff=Target->pEffect;										
				while (Eff)
				{
					if (Eff->pSpell==SP)
					{
						Duration=0;
					};
					Eff=Eff->pNext;
				};	
				
				if (Duration>1)
				{					
					if ((Index>1000)&&((Target->pUnit->pType()->iLevel>2)||(Target->pHero)))
					{//Полиморф
					//	Buff+=(Target->iLife-(pFirstUnitType+Index-1000)->iLife)*Duration;
						if (arUnitPower[1-AA][Unit][2]==2)
						{
							Buff+=(Target->GetAttack()-(pFirstUnitType+Index-1000)->iAttack)*Duration;
						};
						Buff+=Target->GetRangedAttack()*Target->iAmmo;
						if ((arUnitPower[AA][Unit][5]>0)||(arUnitPower[1-AA][Unit][6]>0))
						{
							iGainSpellDamage+=Target->iLife;
							Buff+=(Target->iLife-(pFirstUnitType+Index-1000)->iLife)*Duration;
						};
//						textprintf(screen,Font1,0,0,COLOR_2,"Полиморф на %s. Дамаг: %d, Бафф: %d               ",(pFirstUnitType+Target->pUnit->ipType)->sName,iGainSpellDamage,Buff);
//						readkey();
					};
					switch (Index)
					{
					case -1: //Отравление
						Buff-=Duration*SpellPower;
						iGainSpellDamage-=Duration*SpellPower;
						if (((arUnitPower[1-AA][Unit][0]==2)||(arUnitPower[1-AA][Unit][1]==2))&&(arUnitPower[1-AA][Unit][3]==0)&&(arUnitPower[1-AA][Unit][4]==0)&&(arUnitPower[1-AA][Unit][5]==0)&&(arUnitPower[1-AA][Unit][6]==0)&&(SpellPower<0))
						{
							Buff+=Target->GetRangedAttack()*Target->iAmmo/4;
							Buff+=Target->CheckSkill(24)*Target->iAmmo/4;
						};
						break;
					case 2:	//Снижение атаки
						if (arUnitPower[1-AA][Unit][2]==2)
						{
				//			StopLog("Атака:",Unit);
							if ((abs(SpellPower)>Target->GetAttack())&&(SpellPower<0))
							{
								Buff+=(Target->GetAttack()-1)*Duration;
							}
							else
							{
								Buff-=Duration*SpellPower;
							};
						};
						break;
					case 3:	//Снижение контратаки
						if ((arUnitPower[1-AA][Unit][5]>0)||(arUnitPower[1-AA][Unit][6]>0))
						{
				//			StopLog("КонтрАтака:",Unit);
							if ((abs(SpellPower)>Target->GetCounterAttack())&&(SpellPower<0))
							{
								Buff+=(Target->GetCounterAttack()-1)*Duration;
							}
							else
							{
								Buff-=Duration*SpellPower;
							};
						};
						break;
					case 4:	//Снижение защиты
						if (arUnitPower[1-AA][Unit][6]>0)
						{
				//			StopLog("Защита:",Unit);
							if ((abs(SpellPower)>Target->GetDefence())&&(SpellPower<0))
							{
								Buff+=(Target->GetDefence())*Duration;
							}
							else
							{
								Buff-=Duration*SpellPower;
							};
						};
						break;
					case 5:	//Снижение дистанционной защиты
						if (arUnitPower[1-AA][Unit][4]>0)
						{
				//			StopLog("ДистЗащита:",Unit);
							if ((abs(SpellPower)>Target->GetRangedDefence())&&(SpellPower<0))
							{
								Buff+=(Target->GetRangedDefence())*Duration;
							}
							else
							{
								Buff-=Duration*SpellPower;
							};
						};
						break;
					case 6:	//Снижение сопротивления
						if ((arUnitPower[1-AA][Unit][3]>0)||(arUnitPower[1-AA][Unit][5]>0))
						{
				//			StopLog("Сопрот:",Unit);
							if ((abs(SpellPower)>Target->GetResist())&&(SpellPower<0))
							{
								Buff+=(Target->GetResist())*Duration;
							}
							else
							{
								Buff-=Duration*SpellPower;
							};								
						};
						break;
					case 7:	//Замедление
						if ((Target->GetSpeed()>1)&&(arUnitPower[1-AA][Unit][0]==0)&&(arUnitPower[1-AA][Unit][1]==0)&&(arUnitPower[1-AA][Unit][0]==0)&&(arUnitPower[1-AA][Unit][2]==0))
						{
				//			StopLog("Скор:",Unit);
							if ((abs(SpellPower)>Target->GetSpeed())&&(SpellPower<0))
							{
								Buff+=(Target->GetResist()+Target->GetAttack()/5)*Duration;
							}
							else
							{
								Buff-=Duration*(SpellPower-Target->GetAttack()/5);
							};								
						};
						break;
					case 8:	//Снижение дистанционной атаки
						if (arUnitPower[1-AA][Unit][1]==2)
						{
				//			StopLog("ДистАтака:",Unit);
							if ((abs(SpellPower)>Target->GetRangedAttack())&&(SpellPower<0))
							{
								Buff+=(Target->GetRangedAttack()-1)*Duration;
							}
							else
							{
								Buff-=Duration*SpellPower;
							};
						};
						break;
					case 36:
					case 38:	//Запрет атаки или обездвиживание
						if ((arUnitPower[1-AA][Unit][1]==2)||(arUnitPower[1-AA][Unit][2]==2))
						{
				//			StopLog("ДистАтака:",Unit);							
							iGainSpellDamage+=(Target->GetAttack()+Target->GetRangedAttack());
							Buff+=Target->GetLife()*Duration;							
						}
						else
						{
							Buff+=2*Duration;
						};
						break;


					};
				};
				break;
			};
		};
	};
							
	int MyTaste=Buff;
	//Не вредить подчинённым
	if ((iGainSpellDamage>0)&&(bPosessed)&&(Target->GetEffectDuration(73)>0)&&(Target->GetEffectDuration(73)<3))
	{iGainSpellDamage=-iGainSpellDamage;};
	if (iGainSpellDamage>=Target->iLife)
	{
		iGainSpellDamage=Target->iLife;
		if (SelfHeal>Target->iLife)
		{iGainSpellDamage*=2;}
		else
		{iGainSpellDamage+=SelfHeal;};
		MyTaste+=iGainSpellDamage*6;
		MyTaste+=AttBuff;		
	}
	else
	{		
		iGainSpellDamage+=SelfHeal;
		//Снижение эффективности повреждений по спящей цели
		if (Target->GetEffectDuration(70)>1)
		{MyTaste+=iGainSpellDamage/Target->GetEffectDuration(70);}
		else
		{MyTaste+=iGainSpellDamage*3;};
		if ((iGainSpellDamage>5)||(iGainSpellDamage>=BaseDamage/2))
		{MyTaste+=AttBuff;};
	};
	MyTaste+=MyTaste*AI_GetUnitTaste(Target)/50;
	
	//if (ActiveUnit->pHero)
	//{
	//	MyTaste-=SP->iCost;
	//}
	//else
	//{
	//	MyTaste-=ActiveUnit->CheckSkill(2000+Spell)*3;
	//};
	return MyTaste;			
};


//Вычисление эффективности заклинания на дружественном юните
int AI_GetAllySpellTaste(int ipSpell, BF_Unit *Target, int Unit)
{
	clSpell *SP=pFirstSpell+ipSpell;
	iGainSpellDamage=0;
	int AntiSkill=0;
	for (int i=0; i<MAX_ANTI_EFFECT; i++)
	{
		AntiSkill+=SP->arAntiEffect[i]*Target->CheckSkill(SP->arAntiEffect[i]);
	};
	for (int i=0; i<MAX_UNIT_KIND; i++)
	{
		if (SP->arUnitKind[i]==Target->GetUnitKind())
		{AntiSkill=1;};
	};
	if ((AntiSkill)&&(Target->CheckSkill(205)!=ipSpell))
	{return 0;};
	BF_Effect *Eff;
	int Buff=0, Resist=0, SpellPower=0, Duration=0, AttBuff=0, BaseDamage=0, Hit, SelfHeal=0;
	bool bFriendlyFire=false;	
	if (ActiveUnit->pHero)
	{Resist=Target->GetResist()-ActiveUnit->pHero->CheckHeroBonus(908);}
	else
	{Resist=Target->GetResist();};
	if (Resist<0){Resist=0;};
	for (int i=0; i<MAX_SPELL_EFFECT; i++)
	{
		if (SP->arEffect[0][i]>0)
		{
			SpellPower=SP->arEffect[1][i];
			if ((SP->arEffect[1][i]>0)&&(ActiveUnit->pHero))
			{
				SpellPower=SpellPower+ActiveUnit->pHero->CheckHeroBonus(905)*SP->iPowerMod/100;
			};
			if ((SP->arEffect[1][i]<0)&&(ActiveUnit->pHero))
			{
				SpellPower=SpellPower-ActiveUnit->pHero->CheckHeroBonus(905)*SP->iPowerMod/100;
			};
			Duration=SP->arEffect[2][i];
			int Index=SP->arEffect[0][i];
			if (Duration>0)
			{
				if (ActiveUnit->pHero)
				{
					Duration+=ActiveUnit->pHero->CheckHeroBonus(906)*SP->iDurationMod/100;
				};
				if (Index==1)
				{
					Index=-1;
				};
			};
			switch (Index)
			{
			case 0:	break;
			case 1:									
				if (SP->arEffect[2][i]==-1)
				{
					//Повреждение
					if((SP->bNegative)&&(!bFriendlyFire))
					{
						Hit=GetHit(SpellPower,Resist*SP->bResistPower+Target->GetDefence()*SP->bDefencePower);
						iGainSpellDamage-=Hit;					
					}
					else
					//Лечение
					{
						iGainSpellDamage=SpellPower;						
					};
				};
				break;
				case 10:
					if (!SP->bNegative)
					{
						if (SP->arEffect[2][i]==-1)
						{
					//Пополнение снарядов
							if ((arUnitPower[AA][Unit][0]>0)||(arUnitPower[AA][Unit][1]>0))
							{
								if (Target->GetAmmo()-Target->iAmmo>SpellPower)
								{
									Buff+=SpellPower*(Target->GetRangedAttack()+Target->CheckSkill(24));
									if (Target->iAmmo==0)
									{
										iGainSpellDamage+=Target->GetRangedAttack()+Target->CheckSkill(24);
									};
								}
								else
								{
									Buff+=(Target->GetAmmo()-Target->iAmmo)*(Target->GetRangedAttack()+Target->CheckSkill(24));									
									if (Target->iAmmo==0)
									{
										iGainSpellDamage+=Target->GetRangedAttack()+Target->CheckSkill(24);
									};
								};
							};																				
						}
						else
						{
					//Постоянный эффект
						};
					};			
					break;
					case 11:
					if (!Target->CheckSkill(18))
					{
					if (!SP->bNegative)
					{
						if (SP->arEffect[2][i]==-1)
						{
					//Пополнение энергии
							if (Target->GetStamina()-Target->iStamina>SpellPower)
							{
								Hit=SpellPower;
							}
							else
							{
								if (Target->iStamina>=5)
								{Hit=0;}
								else
								{Hit=Target->GetStamina()-Target->iStamina;};
							};
							if (Hit>0)
							{
								Buff+=Hit;
								if (Target->iStamina==0)
								{
									Buff+=Hit*Target->GetAttack();
								}
								else
								{
									if (Target->iStamina<5)
									{
										Buff+=Hit*Target->GetAttack()/5;
										if (Target==ActiveUnit)
										{
											iGainSpellDamage+=Target->GetAttack();
										};
									};
								};
							};
						}
						else
						{
					//Постоянный эффект
						};
					};
					};
					break;
				case 12:
					if (!Target->CheckSkill(19))
					{
					if (!SP->bNegative)
					{
						if (SP->arEffect[2][i]==-1)
						{
					//Воодушевление
							if (SpellPower>Target->GetMorale()-Target->iMorale)
							{SpellPower=Target->GetMorale()-Target->iMorale;};
							if (SpellPower<0)
							{SpellPower=0;};
							Buff+=SpellPower;
							if (Target->iMorale==0)
							{
								iGainSpellDamage+=Target->GetAttack()*SpellPower;
							};
							if (Target->iMorale<5)
							{
								Buff+=SpellPower;
							};								
						}
						else
						{
					//Постоянный эффект
						};
					};	
					};
					break;
				case 31:
					//Снятие чар
					if (ActiveUnit->pHero)
					{
						SpellPower=SP->arEffect[1][i]+ActiveUnit->pHero->CheckHeroBonus(908);
					};			
					Eff=Target->pEffect;						
					while (Eff)
					{
						if ((Eff->pSpell)&&(Eff->pSpell->bNegative))
						{
							if (Eff->iTime>SpellPower)
							{
								Buff+=SpellPower*Eff->iPower;								
							}
							else
							{
								Buff+=Eff->iTime*Eff->iPower;
							};							
						};						
						Eff=Eff->pNext;
					};				
					break;								
				case 72:
					bFriendlyFire=true;
					break;
				case 75:
				//Дополнительный ход
					if ((!Target->CheckSkill(36))&&(Target->iMorale>0)&&(!Target->bCanAct))
					{
						if (arUnitPower[AA][Unit][0]==2)
						{
							iGainSpellDamage+=Target->CheckSkill(24);
						};
						if (arUnitPower[AA][Unit][1]==2)
						{
							iGainSpellDamage+=Target->GetRangedAttack();
						};
						if (arUnitPower[AA][Unit][2]==2)
						{
							iGainSpellDamage+=Target->GetAttack();
						};
					};
				break;
				case 89:
				//Телепорт
					if ((Target->GetAttack()>5)&&(Target->iAmmo==0)&&(arUnitPower[AA][Unit][2]<2)&&(Target->bCanAct))
					{
						int BigHit=0;
						for (int x=0; x<8; x++)
						{
							for (int y=0; y<8; y++)
							{
								int Hit=GetTeleportTaste(Target,x,y);
								if (Hit>BigHit)
								{BigHit=Hit;};
							};
						};						
						iGainSpellDamage=BigHit;
					}
					break;
				case 91:
				//Темный пакт
					if ((Target->iLife<=Target->GetLife()/4)&&((arUnitPower[AA][Unit][5]>0)||(arUnitPower[AA][Unit][6]>0)))
					{
						iGainSpellDamage=5+(Target->pUnit->pType()->iLevel-1)*10;
						Buff+=Target->pUnit->pType()->iLevel*50;
			//			textprintf(screen,Font1,0,0,COLOR_2,"Темный Пакт на %s. Дамаг: %d, Бафф: %d               ",(pFirstUnitType+Target->pUnit->ipType)->sName,iGainSpellDamage,Buff);
			//			readkey();
					};
					break;
			default:
				Duration-=Resist*SP->bResistDuration;
				BF_Effect *Eff=Target->pEffect;										
				while (Eff)
				{
					if (Eff->pSpell==SP)
					{
						Duration=0;
					};
					Eff=Eff->pNext;
				};										
				if (Duration>1)
				{
					if (Index>1000)
					{
						if (!Target->bCanAct)
						{Duration--;};
						iGainSpellDamage=((pFirstUnitType+Index-1000)->iAttack-Target->pUnit->GetAttack());
						if (Target->iAmmo>=2)
						{iGainSpellDamage-=Target->GetRangedAttack()*2;};
						iGainSpellDamage=iGainSpellDamage*Target->GetAttack()/(Target->pUnit->GetAttack()+1)*Duration;
						if (Target->pHero)
						{iGainSpellDamage=iGainSpellDamage*2/3;};
					};
					switch (Index)
					{
					case -1: //Регенерация
						if (Target->iLife<Target->GetLife())
						{
							Buff+=Target->GetRangedAttack()*Target->iAmmo/20;
							Buff+=Target->GetAttack()/2;
							if ((arUnitPower[AA][Unit][3]>0)||(arUnitPower[AA][Unit][4]>0)&&(arUnitPower[AA][Unit][5]>0)||(arUnitPower[AA][Unit][6]>0))
							{
								Buff+=Duration*SpellPower;
							};
						};						
						break;
					case 2:	//Повышение атаки
						if (arUnitPower[AA][Unit][2]==2)
						{
				//			StopLog("Атака:",Unit);
							Buff+=Duration*SpellPower;
						};
						break;
					case 3:	//Повышение контратаки
						if ((arUnitPower[AA][Unit][5]>0)||(arUnitPower[AA][Unit][6]>0))
						{
				//			StopLog("КонтрАтака:",Unit);
							Buff+=Duration*SpellPower;						
						};
						break;
					case 4:	//Повышение защиты
						if (arUnitPower[AA][Unit][6]>0)
						{
				//			StopLog("Защита:",Unit);
							Buff+=Duration*SpellPower;							
						};
						break;
					case 5:	//Повышение дистанционной защиты
						if (arUnitPower[AA][Unit][4]>0)
						{
				//			StopLog("ДистЗащита:",Unit);
							Buff+=Duration*SpellPower;			
						};
						break;
					case 6:	//Повышение сопротивления
						if ((arUnitPower[AA][Unit][3]>0)||(arUnitPower[AA][Unit][5]>0))
						{
				//			StopLog("Сопрот:",Unit);
							Buff+=Duration*SpellPower;														
						};
						break;
					case 7:	//Ускорение
						if ((arUnitPower[AA][Unit][0]==0)&&(arUnitPower[AA][Unit][1]==0)&&(arUnitPower[AA][Unit][0]==0)&&(arUnitPower[AA][Unit][2]==0))
						{
				//			StopLog("Скор:",Unit);
							Buff+=Duration*(SpellPower+Target->GetAttack()/5);
						};
						break;
					case 8:	//Повышение дистанционной атаки
						if (arUnitPower[AA][Unit][1]==2)
						{
				//			StopLog("ДистАтака:",Unit);
							Buff+=Duration*SpellPower;							
						};
						break;
					case 68: //Вампиризм
						if ((Target->iLife<Target->GetLife())&&(arUnitPower[AA][Unit][2]==2))
						{							
							Buff+=Target->GetAttack()*SpellPower/100*Duration;
							iGainSpellDamage+=Target->GetAttack()*SpellPower/100;
						};						
						break;
					};
				};
				break;
			};
		};
	};
							
	int MyTaste=Buff;						
	if (iGainSpellDamage<0)
	{		
		if (abs(iGainSpellDamage)>=Target->iLife)
		{
			iGainSpellDamage=-Target->iLife;		
			MyTaste+=iGainSpellDamage*6;		
		}
		else
		{
			if (Target->CheckEffect(70)>0)
			{
				iGainSpellDamage+=(Target->GetEffectDuration(70)-1);
				MyTaste+=3*(Target->GetEffectDuration(70)-1);
			};
			MyTaste+=iGainSpellDamage*3;			
		};
	}
	else
	{
		if ((SP->arEffect[0][0]==1)&&(iGainSpellDamage>=Target->GetLife()-Target->iLife))
		{
			iGainSpellDamage=Target->GetLife()-Target->iLife;
		};
		MyTaste+=iGainSpellDamage*3;
		MyTaste+=iGainSpellDamage*Target->pUnit->pType()->iLevel/2;
		if (Target->pHero)
		{MyTaste+=iGainSpellDamage;};
	};
	MyTaste+=MyTaste*AI_GetUnitTaste(Target)/50;
	//if (ActiveUnit->pHero)
	//{
	//	MyTaste-=SP->iCost;
	//}
	//else
	//{
	//	MyTaste-=ActiveUnit->CheckSkill(2000+Spell)*3;
	//};
	return MyTaste;
};

//Изменение пользы заклинания в зависимости от цены вызова, поиск жертв
int AI_CalcSpellPenalty(clSpell *SP, int Spell)
{
	int MyTaste=0;
	if (SP->iLifeCost>=ActiveUnit->iLife)
	{
		MyTaste-=(ActiveUnit->iLife*6+40);
	}
	else
	{
		MyTaste-=SP->iLifeCost*3;
	};

	MyTaste-=SP->iStamCost/2;
	
	int Vic1, Vic2;
	if (SP->bSacrifice)
	{
		iAISacVictim=-1;
		for (int i=0; i<MAX_SQUAD; i++)
		{
			if ((Squad[AA][i].pUnit)&&(Squad[AA][i].iLife>0)&&(Squad[AA][i].GetUnitKind()==1))
			{
				if (iAISacVictim<0)
				{
					iAISacVictim=i;
					Vic2=Squad[AA][i].iLife*Squad[AA][i].pUnit->pType()->iLevel+Squad[AA][i].pUnit->iLevel*2;
					if (Squad[AA][i].CheckSkill(73))
					{Vic2=-Vic2;}
				}
				else
				{
					Vic1=Squad[AA][i].iLife*Squad[AA][i].pUnit->pType()->iLevel+Squad[AA][i].pUnit->iLevel*2;
					if (Squad[AA][i].CheckSkill(73))
					{Vic1=-Vic1;}
					if (Vic1<Vic2)
					{
						Vic2=Vic1;
						iAISacVictim=i;
					};
				};
			};
		};
		if (iAISacVictim<0)
		{
			MyTaste-=1000;
		}
		else
		{
			MyTaste-=Vic2;
		};
	};


	if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
	{
		MyTaste-=SP->iCost;
	}
	else
	{
		MyTaste-=ActiveUnit->CheckSkill(2000+Spell)*(ActiveUnit->GetRangedAttack()+ActiveUnit->CheckSkill(24))/2;
	};
	return MyTaste;
};


//Активирует следующего юнита. Активный юнит выбирает и совершает действие. Если CanMove истинно, он может просто двигаться к цели, иначе будет ожидать следующего обращения. Возвращает время задержки на проигрыш анимации
int AI_Main(bool CanMove)
{
	WriteLog("                                             ");
	int arCanMelee[MAX_SQUAD+MAX_SUMMONS];
	int arMeleeCoord[MAX_SQUAD+MAX_SUMMONS][2];
	int arCanRange[MAX_SQUAD+MAX_SUMMONS];
	int arRangeCoord[MAX_SQUAD+MAX_SUMMONS][2];
	int arCanHeal[MAX_SQUAD+MAX_SUMMONS];
	int arHealCoord[MAX_SQUAD+MAX_SUMMONS][2];
	int arCanCast[MAX_SQUAD+MAX_SUMMONS][2];
	int MoveMap [8][8];
	int ActiveNum;
	int Antihit;
	if (FindNextUnit())
	{
		AI_SelectUnit(FindNextUnit(),true);
	};
	if ((ActiveUnit)&&(ActiveUnit->bCanAct)&&((ActiveUnit->iSpeed>0)||(ActiveUnit->CheckSkill(88))))
	{
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if (&Squad[AA][i]==ActiveUnit)
			{ActiveNum=i;};
		};

		//Поиск целей
		//***********

//Магия
//*****		
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			arCanCast[i][0]=0;
		};
	//Составление списка доступных заклинаний
		int arCast[SPELLBOOK_SIZE][2];
		for (int i=0; i<SPELLBOOK_SIZE; i++)
		{
			arCast[i][0]=0;
			arCast[i][1]=0;
		};
		int SpellCount=0;
		if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
		{
			if (SideCanCast[AA]>0)
			{
				for (int i=0; i<SPELLBOOK_SIZE; i++)
				{
					if (Spells[AA][0][i])
					{
						if ((Spells[AA][1][i])&&(arPlayer[BF_Player[AA]].Gem>=(pFirstSpell+Spells[AA][0][i])->iCost*(100-ActiveUnit->pHero->CheckHeroBonus(983))/100))
						{
							bool bNew=true;
							arCast[i][0]=Spells[AA][0][i];
							arCast[i][1]=i;
							SpellCount++;
						};
					};
				};
			}
		}
		else
		{		
			for (int i=1; i<=iMaxSpell; i++)
			{
				if ((ActiveUnit->CheckSkill(2000+i))&&(ActiveUnit->iAmmo>=ActiveUnit->CheckSkill(2000+i)))
				{				
					arCast[SpellCount][0]=i;
					arCast[SpellCount][1]=-1;
					SpellCount++;
				};				
			};
		};
		//Выбор заклинания
		int ChoosenSpell=-1, SpellTaste=0, SpellTarget1=-1, SpellTarget2=-1, SpellHit=0;
		if (SpellCount>0)
		{
//			AI_CalcDangerZone();
			AI_SelectUnit(ActiveUnit,true);
			for (int Spell=0; Spell<SPELLBOOK_SIZE; Spell++)
			{
				int Damage=0, Buff=0, Summon=0, Penalty=0;				
				if (arCast[Spell][0]>0)
				{
					clSpell *SP=(pFirstSpell+arCast[Spell][0]);
					if ((!SP->bSacrifice)||(SideCanCast[AA]>1))
					{
					int SpellPower;
					//Одиночная цель
					if (SP->iTarget==1)
					{
						//Противник
						if (SP->bOnEnemy)
						{
							for (int Uni=0; Uni<MAX_SQUAD+MAX_SUMMONS; Uni++)
							{
							if ((Squad[1-AA][Uni].pUnit)&&(Squad[1-AA][Uni].iLife>0))
							{								
								int MyTaste=AI_GetEnemySpellTaste(arCast[Spell][0],&Squad[1-AA][Uni],Uni);
								MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
								if ((MyTaste>SpellTaste)&&((iGainSpellDamage>0)||(MyTaste>0)))
								{
	//								textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
	//								readkey();
									SpellTaste=MyTaste;
									SpellHit=iGainSpellDamage;
									ChoosenSpell=Spell;
									SpellTarget1=1-AA;
									SpellTarget2=Uni;
								};							
							};
							};
						};
						//Союзник
						if (SP->bOnAlly)
						{
							for (int Uni=0; Uni<MAX_SQUAD+MAX_SUMMONS; Uni++)
							{
							if ((Squad[AA][Uni].pUnit)&&(Squad[AA][Uni].iLife>0))
							{
								int MyTaste=AI_GetAllySpellTaste(arCast[Spell][0],&Squad[AA][Uni],Uni);
								MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
								if ((MyTaste>SpellTaste)&&((iGainSpellDamage>0)||(MyTaste>0)))
								{
			//						textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
			//						readkey();
									SpellTaste=MyTaste;
									SpellHit=iGainSpellDamage;
									ChoosenSpell=Spell;
									SpellTarget1=AA;
									SpellTarget2=Uni;
								};
							};
							};
						};
					};
					//Вызов заклятья без цели (на все поле)
					if ((SP->iTarget==0)&&(SP->iArea==2))
					{
						int MyTaste=0, MyDamage=0;						
						if (SP->bOnAlly)
						{
							for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
							{
								if ((Squad[AA][k].pUnit)&&(Squad[AA][k].iLife>0))
								{									
								//Найден свой подходящий юнит
									iGainSpellDamage=0;
									MyTaste+=AI_GetAllySpellTaste(arCast[Spell][0],&Squad[AA][k],k);
									MyDamage+=iGainSpellDamage;
								};
							};
						};
						if (SP->bOnEnemy)
						{
							for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
							{
								if ((Squad[1-AA][k].pUnit)&&(Squad[1-AA][k].iLife>0))
								{						
								//Найден вражеский подходящий юнит
									iGainSpellDamage=0;
									MyTaste+=AI_GetEnemySpellTaste(arCast[Spell][0],&Squad[1-AA][k],k);
									MyDamage+=iGainSpellDamage;
								};
							};
						};
						iGainSpellDamage=MyDamage;
						MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
	//					textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
	//					readkey();
						if ((MyTaste>SpellTaste)&&((iGainSpellDamage>0)||(MyTaste>0)))
						{
					//		textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
					//		readkey();
							SpellTaste=MyTaste;
							SpellHit=iGainSpellDamage;
							ChoosenSpell=Spell;							
						};
					};
					//На себя
					if ((SP->iTarget==0)&&(SP->iArea==0))
					{
						int MyTaste=AI_GetAllySpellTaste(arCast[Spell][0],ActiveUnit,ActiveNum);
						MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
						if ((MyTaste>SpellTaste)&&((iGainSpellDamage>0)||(MyTaste>0)))
						{
							SpellTaste=MyTaste;
							SpellHit=iGainSpellDamage;
							ChoosenSpell=Spell;
						}
					}
					//Труп юнита
					if ((SP->iTarget==2)||(SP->iTarget==4))
					{
						//Противник
						if (SP->bOnEnemy)
						{
							for (int Uni=0; Uni<MAX_SQUAD+MAX_SUMMONS; Uni++)
							{
								if ((Squad[1-AA][Uni].pUnit)&&(Squad[1-AA][Uni].iLife==0)&&(Squad[1-AA][Uni].bInBattle)&&((SP->iTarget==2)||(arGrid[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y].Pass)))
							{
								Buff=0;
								Damage=0;
								if (SP->arEffect[0][0]>1000)
								{	
									SpellPower=SP->arEffect[1][0];
									if (ActiveUnit->pHero)
									{
										SpellPower+=ActiveUnit->CheckSkill(907)/2+ActiveUnit->CheckSkill(909);
										SpellPower+=ActiveUnit->pHero->CheckHeroBonus(907)/2+ActiveUnit->pHero->CheckHeroBonus(909);
									};
									Buff=((pFirstUnitType+(SP->arEffect[0][0]-1000))->iLevel+1)*10+SpellPower*3-DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]+(pFirstUnitType+(SP->arEffect[0][0]-1000))->iLife;
									if (DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]<2)
									{
										Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2;
									}
									else
									{
										Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2-DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]-1+(pFirstUnitType+(SP->arEffect[0][0]-1000))->iSpeed;
										if (Damage<1)
										{Damage=1;};
									};
								};
								int MyTaste=Buff;								
								MyTaste+=Damage*3;
								MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
								if ((MyTaste>SpellTaste)&&((Damage>0)||(Buff>0)))
								{
		//							textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
		//							readkey();
									SpellTaste=MyTaste;
									SpellHit=Damage;
									ChoosenSpell=Spell;
									SpellTarget1=1-AA;
									SpellTarget2=Uni;
								};
							};
							};

						};
						if (SP->bOnAlly)
						{
							for (int Uni=0; Uni<MAX_SQUAD+MAX_SUMMONS; Uni++)
							{
								if ((Squad[AA][Uni].pUnit)&&(Squad[AA][Uni].iLife==0)&&(Squad[AA][Uni].bInBattle)&&((SP->iTarget==2)||(arGrid[Squad[AA][Uni].x][Squad[AA][Uni].y].Pass)))
							{
								Buff=0;
								Damage=0;
								if (SP->arEffect[0][0]>1000)
								{	
									SpellPower=SP->arEffect[1][0];
									SpellPower+=ActiveUnit->CheckSkill(907)/2+ActiveUnit->CheckSkill(909);
									if (ActiveUnit->pHero)
									{
										SpellPower+=ActiveUnit->pHero->CheckHeroBonus(907)/2+ActiveUnit->pHero->CheckHeroBonus(909);
									};
									Buff=((pFirstUnitType+(SP->arEffect[0][0]-1000))->iLevel+1)*10+SpellPower*3-1-DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]+(pFirstUnitType+(SP->arEffect[0][0]-1000))->iLife;
									if (DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]<2)
									{
										Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2;
									}
									else
									{
										Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2-DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]-1+(pFirstUnitType+(SP->arEffect[0][0]-1000))->iSpeed;
										if (Damage<1)
										{Damage=1;};
									};
								};
//								Воскрешение
								if (SP->arEffect[0][0]==1)
								{
									Buff=(Squad[AA][Uni].pUnit->pType()->iLevel)*15+Squad[AA][Uni].pUnit->iLevel*2;
									Damage=Squad[AA][Uni].pUnit->pType()->iAttack;
								};
								int MyTaste=Buff;								
								MyTaste+=Damage*3;
								MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
								if ((MyTaste>SpellTaste)&&((Damage>0)||(Buff>0)))
								{
			//						textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
			//						readkey();
									SpellTaste=MyTaste;
									SpellHit=Damage;
									ChoosenSpell=Spell;
									SpellTarget1=AA;
									SpellTarget2=Uni;
								};
							};
							};
						};
					};
					//Вызов заклинания в свободном тайле
					if (SP->iTarget==5)
					{
						for (int i=0; i<8; i++){
						for (int j=0; j<8; j++)
						{
							Field[i][j].Land=0;
							Field[i][j].Pass=0;
							Field[i][j].Shoot=0;
						};
						};					
						ShootMap(ActiveUnit->x,ActiveUnit->y,-1,ActiveUnit->CheckSkill(907)/2+1);
						for (int x=0; x<8; x++){
						for (int y=0; y<8; y++)
						{
							if ((Field[x][y].Shoot>0)&&(arGrid[x][y].Pass)&&((pFirstBF_Obj+arGrid[x][y].Land)->iMoveCost>0))
						{							
								if (SP->arEffect[0][0]>1000)
								{
								Buff=0;
								Damage=0;
								SpellPower=SP->arEffect[1][0];
								SpellPower+=ActiveUnit->CheckSkill(907);
								if (ActiveUnit->pHero)
								{
									SpellPower+=ActiveUnit->pHero->CheckHeroBonus(907);
								};
								Buff=((pFirstUnitType+(SP->arEffect[0][0]-1000))->iLevel+1)*10+SpellPower*3-DangerZone[x][y]*2+(pFirstUnitType+(SP->arEffect[0][0]-1000))->iLife;
								if (DangerZone[x][y]<2)
								{
									Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2;
								}
								else
								{
									Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2-DangerZone[x][y]+(pFirstUnitType-1+(SP->arEffect[0][0]-1000))->iSpeed;
									if (Damage<1)
									{Damage=1;};
								};
								int MyTaste=Buff;								
								MyTaste+=Damage*3;
								MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
								if ((MyTaste>SpellTaste)&&((Damage>0)||(Buff>0)))
								{
			//						textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
			//						readkey();
									SpellTaste=MyTaste;
									SpellHit=Damage;
									ChoosenSpell=Spell;
									SpellTarget1=x;
									SpellTarget2=y;
							//		clear_keybuf();
							//		textprintf(screen,Font1,10,10,COLOR_2,"%d",DangerZone[x][y]);
							//		readkey();
								};							
							};
						};
						};
						};
					};
					//Вызов заклинания в тайле
					if (SP->iTarget==3)
					{
					for (int i=0; i<8; i++){
					for (int j=0; j<8; j++)
					{

					int MyTaste=0, MyDamage=0;
					for (int x=0; x<8; x++){
					for (int y=0; y<8; y++)
					{
						Field[x][y].Land=0;
						Field[x][y].Pass=0;
						Field[x][y].Shoot=0;
					};
					};					
					ShootMap(i,j,-1,SP->iRadius);					
					if ((SP->bOnAlly)||(SP->bOnEnemy))
					{
						for (int x=0; x<8; x++)
						{
							for (int y=0; y<8; y++)
							{								
								if ((Field[x][y].Shoot>0)||((x==i)&&(y==j)))
								{
									if (SP->bOnAlly)
									{
										for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
										{
											if ((Squad[AA][k].pUnit)&&(Squad[AA][k].iLife>0)&&(Squad[AA][k].x==x)&&(Squad[AA][k].y==y))
											{												
											//Найден свой подходящий юнит
												iGainSpellDamage=0;
												MyTaste+=AI_GetAllySpellTaste(arCast[Spell][0],&Squad[AA][k],k);												
												MyDamage+=iGainSpellDamage;
											};
										};
									};
									if (SP->bOnEnemy)
									{
										for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
										{
											if ((Squad[1-AA][k].pUnit)&&(Squad[1-AA][k].iLife>0)&&(Squad[1-AA][k].x==x)&&(Squad[1-AA][k].y==y))
											{
											//Найден вражеский подходящий юнит
												iGainSpellDamage=0;
												MyTaste+=AI_GetEnemySpellTaste(arCast[Spell][0],&Squad[1-AA][k],k);
												MyDamage+=iGainSpellDamage;
											};
										};
									};
								};								
							};
						};
					}
					else
					{
						for (int x=0; x<8; x++)
						{
							for (int y=0; y<8; y++)
							{
								if ((Field[x][y].Shoot>0)||((x==i)&&(y==j)))
								{
							//Найден тайл для заклинания
								};
							};
						};
					};
					iGainSpellDamage=MyDamage;
					MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
					if ((MyTaste>SpellTaste)&&((iGainSpellDamage>0)||(MyTaste>0)))
					{
		//				textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
		//				readkey();
						SpellTaste=MyTaste;
						SpellHit=iGainSpellDamage;
						ChoosenSpell=Spell;
						SpellTarget1=i;
						SpellTarget2=j;
					};
					};
					};
					};
					}
				};
			};
			if (SpellHit==0)
			{SpellHit=SpellTaste/10;};
		};

//Лечение
//*******
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			arCanHeal[i]=0;
		};

		if ((ActiveUnit->CheckSkill(24))&&(ActiveUnit->iAmmo>0))
		{
			AI_SelectUnit(ActiveUnit,true);
		//Поиск целей с места
			for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
			{
				if ((Squad[AA][i].iLife>0)&&(Field[Squad[AA][i].x][Squad[AA][i].y].Shoot)
					&&(Squad[AA][i].iLife<Squad[AA][i].GetLife())
					&&(Squad[AA][i].GetUnitKind()!=2)&&(Squad[AA][i].GetUnitKind()!=4))
				{
						arCanHeal[i]=3;
						arHealCoord[i][0]=ActiveUnit->x;
						arHealCoord[i][1]=ActiveUnit->y;						
				};
			};
		//Карта проходимости
			if (ActiveUnit->iSpeed>1)
			{
			for (int i=0; i<8; i++){
			for (int j=0; j<8; j++){
				MoveMap[i][j]=0;};};
			for (int i=0; i<8; i++)
			{
				for (int j=0; j<8; j++)
				{
					if ((Field[i][j].Land>0)&&(Field[i][j].Land<=ActiveUnit->iSpeed)&&(Field[i][j].Pass)&&(arGrid[i][j].Pass))
					{
						if ((Field[i][j].Land==ActiveUnit->iSpeed)&&(ActiveUnit->bMarch)&&(MoveMap[i][j]<1))
						{
							MoveMap[i][j]=1;
						};
						if (Field[i][j].Land<ActiveUnit->iSpeed)
						{
							MoveMap[i][j]=2;
						};
					};					
				};
			};
			for (int x=0; x<8; x++)
			{
			for (int y=0; y<8; y++)
			{
			if (MoveMap[x][y]>0)
			{
				for (int i=0; i<8; i++){
				for (int j=0; j<8; j++)
				{
					Field[i][j].Land=0;
					Field[i][j].Pass=0;
					Field[i][j].Shoot=0;
				};
				};

				//Влияние холмов на дальность
				int CurrRange=ActiveUnit->GetShootingRange();
				if ((ActiveUnit->GetShootingRange()>1)&&(ActiveUnit->CheckSkill(14)<=0))
				{
					if ((arGrid[ActiveUnit->x][ActiveUnit->y].Land==3)&&(arGrid[x][y].Land!=3))
					{
						CurrRange--;
					};
					if ((arGrid[ActiveUnit->x][ActiveUnit->y].Land!=3)&&(arGrid[x][y].Land==3))
					{
						CurrRange++;
					};
				};
				//Поиск целей с подходом
				ShootMap(x,y,-1,CurrRange);
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{		
					if ((arCanHeal[i]<MoveMap[x][y])&&(Squad[AA][i].iLife>0)&&(Field[Squad[AA][i].x][Squad[AA][i].y].Shoot)
					&&(Squad[AA][i].iLife<Squad[AA][i].GetLife())
					&&(Squad[AA][i].GetUnitKind()!=2)&&(Squad[AA][i].GetUnitKind()!=4))
					{
						arCanHeal[i]=MoveMap[x][y];
						arHealCoord[i][0]=x;
						arHealCoord[i][1]=y;
					};
				};
			};
			};
			};			
			};
		};

//Стрельба
//********
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			arCanRange[i]=0;
		};
		//Поиск целей с места		
		if ((ActiveUnit->iAmmo>0)&&(ActiveUnit->GetRangedAttack()>0))
		{
			AI_SelectUnit(ActiveUnit,true);
			for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
			{		
				if ((Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0)&&(Field[Squad[1-AA][i].x][Squad[1-AA][i].y].Shoot>1))
				{
					arCanRange[i]=3;
					arRangeCoord[i][0]=ActiveUnit->x;
					arRangeCoord[i][1]=ActiveUnit->y;
				};
			};
			//Карта проходимости
			if (ActiveUnit->iSpeed>1)
			{
			for (int i=0; i<8; i++){
			for (int j=0; j<8; j++){
				MoveMap[i][j]=0;};};
			for (int i=0; i<8; i++)
			{
				for (int j=0; j<8; j++)
				{
					if ((Field[i][j].Land>0)&&(Field[i][j].Land<=ActiveUnit->iSpeed)&&(Field[i][j].Pass)&&(arGrid[i][j].Pass))
					{
						if ((Field[i][j].Land==ActiveUnit->iSpeed)&&(ActiveUnit->bMarch)&&(MoveMap[i][j]<1))
						{
							MoveMap[i][j]=1;
						};
						if (Field[i][j].Land<ActiveUnit->iSpeed)
						{
							MoveMap[i][j]=2;
						};
					};					
				};
			};

			for (int x=0; x<8; x++)
			{
			for (int y=0; y<8; y++)
			{
			if (MoveMap[x][y]>0)
			{
				for (int i=0; i<8; i++){
				for (int j=0; j<8; j++)
				{
					Field[i][j].Land=0;
					Field[i][j].Pass=0;
					Field[i][j].Shoot=0;
				};
				};
				//Влияние холмов на дальность
				int CurrRange=ActiveUnit->GetShootingRange();
				if ((ActiveUnit->GetShootingRange()>1)&&(ActiveUnit->CheckSkill(14)<=0))
				{
					if ((arGrid[ActiveUnit->x][ActiveUnit->y].Land==3)&&(arGrid[x][y].Land!=3))
					{
						CurrRange--;
					};
					if ((arGrid[ActiveUnit->x][ActiveUnit->y].Land!=3)&&(arGrid[x][y].Land==3))
					{
						CurrRange++;
					};
				}
				//Поиск целей с подходом
				ShootMap(x,y,-1,CurrRange);
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{		
					if ((arCanRange[i]<MoveMap[x][y])&&(Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0)&&(Field[Squad[1-AA][i].x][Squad[1-AA][i].y].Shoot>1))
					{
						arCanRange[i]=MoveMap[x][y];
						arRangeCoord[i][0]=x;
						arRangeCoord[i][1]=y;
					};
				};
			};
			};
			};			
			};
		};
//Рукопашная
//**********
		AI_SelectUnit(ActiveUnit,true);
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
		//Выбор тайла для атаки
			arCanMelee[i]=0;
			if (Squad[1-AA][i].iLife)
			{
				if (AI_InMeleeRange(Squad[1-AA][i].x-1+Squad[1-AA][i].y%2,Squad[1-AA][i].y-1)>0)
				{
					arCanMelee[i]=AI_InMeleeRange(Squad[1-AA][i].x-1+Squad[1-AA][i].y%2,Squad[1-AA][i].y-1);
					arMeleeCoord[i][0]=Squad[1-AA][i].x-1+Squad[1-AA][i].y%2;
					arMeleeCoord[i][1]=Squad[1-AA][i].y-1;
				};
				if (AI_InMeleeRange(Squad[1-AA][i].x+Squad[1-AA][i].y%2,Squad[1-AA][i].y-1)>0)
				{
					if ((arCanMelee[i]==0)||
	(AI_BetterPlace(Squad[1-AA][i].x+Squad[1-AA][i].y%2,Squad[1-AA][i].y-1,arMeleeCoord[i][0],arMeleeCoord[i][1],arCanMelee[i],AI_InMeleeRange(Squad[1-AA][i].x+Squad[1-AA][i].y%2,Squad[1-AA][i].y-1))))
					{
						arCanMelee[i]=AI_InMeleeRange(Squad[1-AA][i].x+Squad[1-AA][i].y%2,Squad[1-AA][i].y-1);
						arMeleeCoord[i][0]=Squad[1-AA][i].x+Squad[1-AA][i].y%2;
						arMeleeCoord[i][1]=Squad[1-AA][i].y-1;
					};
				};
				if (AI_InMeleeRange(Squad[1-AA][i].x+1,Squad[1-AA][i].y)>0)
				{
					if ((arCanMelee[i]==0)||
	(AI_BetterPlace(Squad[1-AA][i].x+1,Squad[1-AA][i].y,arMeleeCoord[i][0],arMeleeCoord[i][1],arCanMelee[i],AI_InMeleeRange(Squad[1-AA][i].x+1,Squad[1-AA][i].y))))
					{
						arCanMelee[i]=AI_InMeleeRange(Squad[1-AA][i].x+1,Squad[1-AA][i].y);
						arMeleeCoord[i][0]=Squad[1-AA][i].x+1;
						arMeleeCoord[i][1]=Squad[1-AA][i].y;
					};
				};
				if (AI_InMeleeRange(Squad[1-AA][i].x+Squad[1-AA][i].y%2,Squad[1-AA][i].y+1)>0)
				{
					if ((arCanMelee[i]==0)||
	(AI_BetterPlace(Squad[1-AA][i].x+Squad[1-AA][i].y%2,Squad[1-AA][i].y+1,arMeleeCoord[i][0],arMeleeCoord[i][1],arCanMelee[i],AI_InMeleeRange(Squad[1-AA][i].x+Squad[1-AA][i].y%2,Squad[1-AA][i].y+1))))
					{
						arCanMelee[i]=AI_InMeleeRange(Squad[1-AA][i].x+Squad[1-AA][i].y%2,Squad[1-AA][i].y+1);
						arMeleeCoord[i][0]=Squad[1-AA][i].x+Squad[1-AA][i].y%2;
						arMeleeCoord[i][1]=Squad[1-AA][i].y+1;
					};
				};
				if (AI_InMeleeRange(Squad[1-AA][i].x-1+Squad[1-AA][i].y%2,Squad[1-AA][i].y+1)>0)
				{
					if ((arCanMelee[i]==0)||
	(AI_BetterPlace(Squad[1-AA][i].x-1+Squad[1-AA][i].y%2,Squad[1-AA][i].y+1,arMeleeCoord[i][0],arMeleeCoord[i][1],arCanMelee[i],AI_InMeleeRange(Squad[1-AA][i].x-1+Squad[1-AA][i].y%2,Squad[1-AA][i].y+1))))
					{
						arCanMelee[i]=AI_InMeleeRange(Squad[1-AA][i].x-1+Squad[1-AA][i].y%2,Squad[1-AA][i].y+1);
						arMeleeCoord[i][0]=Squad[1-AA][i].x-1+Squad[1-AA][i].y%2;
						arMeleeCoord[i][1]=Squad[1-AA][i].y+1;
					};
				};
				if (AI_InMeleeRange(Squad[1-AA][i].x-1,Squad[1-AA][i].y)>0)
				{
					if ((arCanMelee[i]==0)||
	(AI_BetterPlace(Squad[1-AA][i].x-1,Squad[1-AA][i].y,arMeleeCoord[i][0],arMeleeCoord[i][1],arCanMelee[i],AI_InMeleeRange(Squad[1-AA][i].x-1,Squad[1-AA][i].y))))
					{
						arCanMelee[i]=AI_InMeleeRange(Squad[1-AA][i].x-1,Squad[1-AA][i].y);
						arMeleeCoord[i][0]=Squad[1-AA][i].x-1;
						arMeleeCoord[i][1]=Squad[1-AA][i].y;
					};
				};
			};
		};

		int WillStrike=-1,	MeleeHit=0;
		int WillShoot=-1, ShootHit=0;
		int WillHeal=-1, HealHit=0;
		int Taste=-1000, Taste1;
		bool KillShoot=false, KillMelee=false;
		AI_SelectUnit(ActiveUnit,true);

		int EnemyArmy=0, OurArmy=0;
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if ((Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0))
			{
				if ((CalcMeleeHit(ActiveUnit,&Squad[1-AA][i],true)>0)||(CalcRangedHit(ActiveUnit,&Squad[1-AA][i])>0))
				{EnemyArmy++;};
			};
			if ((Squad[AA][i].pUnit)&&(Squad[AA][i].iLife>0))
			{OurArmy++;};
		};

		//Оценка целей
		//************

		//Лечение
		if ((ActiveUnit->CheckSkill(24))&&(ActiveUnit->iAmmo>0))
		{
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if (arCanHeal[i]>0)
			{	
				int Hit=ActiveUnit->CheckSkill(24);
				if (Squad[AA][i].GetLife()-Squad[AA][i].iLife<Hit)
				{Hit=Squad[AA][i].GetLife()-Squad[AA][i].iLife;};
				Taste1=Hit*5+Squad[AA][i].GetDefence()+Squad[AA][i].GetRangedDefence()+Squad[AA][i].GetResist()+(arCanHeal[i]-1)*2;
				if (Squad[AA][i].pUnit->pHero)
				{Taste1+=Hit*(Squad[AA][i].pUnit->pHero->ipClass1/2);};
				if (arCanHeal[i]==1)
				{Taste1/=2;};
				if (Squad[AA][i].CheckSkill(73))
				{Taste1=-2000;};
				if (Taste1>Taste)
				{
					Taste=Taste1;					
					WillHeal=i;
					HealHit=Hit;
				};				
			};
		};
		};
		
		//Выстрел
		if ((ActiveUnit->GetRangedAttack()>0)&&(ActiveUnit->iAmmo>0))
		{
		Taste=-1000;
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if (arCanRange[i]>0)
			{	
				int MyTaste=0, MyHit=0;
				//Паутина
				if (ActiveUnit->CheckSkill(47)>0)
				{
					if (OurArmy>2)
					{
						MyTaste=Squad[1-AA][i].iLife/2;
						MyHit=Squad[1-AA][i].iLife/4;
						if (Squad[1-AA][i].pUnit->pHero)
						{
							MyTaste*=2;
							MyHit*=2;
						};
						if (ActiveUnit->CheckSkill(47)>Squad[1-AA][i].GetAttack())
						{MyTaste*=2;};
						if (Squad[1-AA][i].CheckSkill(36)>0)
						{
							MyTaste/=3;
							MyHit/=3;
						};
						if (Squad[1-AA][i].CheckSkill(201)>0)
						{
							MyTaste=-1000;
							MyHit=0;
						};
					};
				}
				else
				{
					//Окаменение
					if (ActiveUnit->CheckSkill(46)>0)
					{
						if ((Squad[1-AA][i].CheckSkill(36)==0)&&(OurArmy>2)&&(ActiveUnit->CheckSkill(46)>Squad[1-AA][i].GetResist()))
						{
							MyTaste=(ActiveUnit->CheckSkill(46)-Squad[1-AA][i].GetResist()-1)*
								(Squad[1-AA][i].iLife/2+Squad[1-AA][i].GetAttack());
							MyHit=(ActiveUnit->CheckSkill(46)-Squad[1-AA][i].GetResist()-1)*
								(Squad[1-AA][i].iLife/4+Squad[1-AA][i].GetAttack());
							if (Squad[1-AA][i].pUnit->pHero)
							{
								MyTaste*=2;
								MyHit*=2;
							};
						};
						if ((ActiveUnit->CheckSkill(46)<=Squad[1-AA][i].GetResist())||(Squad[1-AA][i].CheckSkill(202)>0))
						{
							MyTaste=-2000;
							MyHit=-3;
						};
					}
					else
					{
						//Обычный выстрел
						MyHit=AI_CalcRangedHit(ActiveUnit,&Squad[1-AA][i]);
						//Двойной выстрел
						if ((MyHit<Squad[1-AA][i].iLife)&&(arCanRange[i]==3)&&(ActiveUnit->CheckSkill(20)>0)&&(ActiveUnit->iStamina>=ActiveUnit->CheckSkill(20)))
						{
							MyHit*=2;
							arCanRange[i]=4;
						};
						//Ядовитый выстрел
						if ((Squad[1-AA][i].CheckSkill(42)==0)&&(MyHit<Squad[1-AA][i].iLife))
						{MyHit+=ActiveUnit->CheckSkill(78)*2;};

						MyTaste=MyHit*2-abs(MyHit-Squad[1-AA][i].iLife-1);
						int Adder=(Squad[1-AA][i].GetAttack()+Squad[1-AA][i].GetCounterAttack())/5;
						Adder+=Squad[1-AA][i].GetRangedAttack()*Squad[1-AA][i].iAmmo/5;						
						Adder-=Squad[1-AA][i].bSummoned*3;
						if (Squad[1-AA][i].pUnit->pHero)
						{
							Adder+=MyHit*(Squad[1-AA][i].pUnit->pHero->ipClass1/2);
						};
						if ((MyHit>=ShootHit/2)||(MyHit>=Squad[1-AA][i].iLife))
						{
							MyTaste+=Adder;
						};
						if ((Adder>0)&&(MyHit>=Squad[1-AA][i].iLife))
						{
							MyTaste+=Adder;
						};
						if (arCanRange[i]==1)
						{MyTaste/=2;};

						//Не бить подчинённых
						if ((Squad[1-AA][i].GetEffectDuration(73)>0)&&(Squad[1-AA][i].GetEffectDuration(73)<3))
						{MyTaste=-2000;};

						if ((MyTaste>0)&&(Squad[1-AA][i].CheckSkill(73)))
						{
							MyTaste/=10;
						};
						if ((MyTaste>0)&&(Squad[1-AA][i].CheckSkill(70)))
						{
							MyTaste/=2;
						};
					};
				};		
				if (((WillShoot<0)&&(EnemyArmy==0))||((MyHit>0)&&(MyTaste>Taste)))
				{
					Taste=MyTaste;
					WillShoot=i;
					ShootHit=MyHit;
					if (MyHit>=Squad[1-AA][i].iLife)
					{KillShoot=true;}
					else
					{KillShoot=false;};
				};				
			};
		};
		};
		if (!ActiveUnit->CheckSkill(38))
		{

		//Рукопашная
		Taste=-1000;
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{ 
			if (arCanMelee[i]>0)
			{
				int Hit=AI_CalcMeleeHit(ActiveUnit,&Squad[1-AA][i]);
				if ((Hit<Squad[1-AA][i].iLife)&&(ActiveUnit->CheckSkill(59))&&(ActiveUnit->iStamina>ActiveUnit->CheckSkill(59))&&(arCanMelee[i]>1))
				{
					buttCrush.bPress=true;
					Hit=AI_CalcMeleeHit(ActiveUnit,&Squad[1-AA][i]);
					buttCrush.bPress=false;
					arCanMelee[i]=5;
				};
				//Проверка вампиризма на ответной атаке
				if (((Hit<Squad[1-AA][i].iLife)||(Squad[1-AA][i].CheckSkill(16)))&&(Squad[1-AA][i].CheckSkill(68)>0)&&(ActiveUnit->CheckSkill(26)==0)&&(ActiveUnit->GetUnitKind()!=2)&&(ActiveUnit->GetUnitKind()!=4)&&(Squad[1-AA][i].iStamina>0)&&(!Squad[1-AA][i].bResting))
				{
					if (Squad[1-AA][i].CheckSkill(27))
					{Antihit=Squad[1-AA][i].GetCounterAttack()-ActiveUnit->GetResist();}
					else
					{Antihit=Squad[1-AA][i].GetCounterAttack()-ActiveUnit->GetDefence();}
					if (Antihit>ActiveUnit->iLife)
					{Antihit=ActiveUnit->iLife;};
					Antihit=Antihit*Squad[1-AA][i].CheckSkill(68)/100;
					if (Antihit>0)
					{Hit-=Antihit;};
				};
				int MyTaste=Hit*2-abs(Hit-Squad[1-AA][i].iLife-1);
				//Не бить подчинённых
				if ((Squad[1-AA][i].GetEffectDuration(73)>0)&&(Squad[1-AA][i].GetEffectDuration(73)<3))
				{MyTaste=-2000;};
				
				if (Hit<Squad[1-AA][i].iLife)
				{
					//Проверка яда
					if (Squad[1-AA][i].CheckSkill(42)==0)
					{MyTaste+=ActiveUnit->CheckSkill(41)*3;}
					//Не бить превращённых в драконов
					if ((Squad[1-AA][i].pUnit->GetUnitType()==60)&&(Squad[1-AA][i].pUnit!=Squad[1-AA][i].pTrueForm)&&(Hit<20))
					{MyTaste=-2000;};
				};
				//Проверка топтания
				if ((Hit<Squad[1-AA][i].iLife)&&(ActiveUnit->CheckSkill(63)>=Squad[1-AA][i].iLife-Hit)&&(Squad[1-AA][i].CheckSkill(14)==0))
				{
					Hit=Squad[1-AA][i].iLife;
					MyTaste=Hit*2;
				};
				//Учёт вытягивания жизни из врагов
				if ((Hit>0)&&(Squad[1-AA][i].GetUnitKind()!=2)&&(Squad[1-AA][i].GetUnitKind()!=4)&&(ActiveUnit->iLife<ActiveUnit->GetLife()))
				{
					if (ActiveUnit->CheckSkill(68)>0)
					{
						MyTaste+=GetMin(Hit*ActiveUnit->CheckSkill(68)/100,ActiveUnit->GetLife()-ActiveUnit->iLife)*2;
					}
					if ((ActiveUnit->CheckSkill(57)>0)&&(Hit>=Squad[1-AA][i].iLife))
					{
						MyTaste+=GetMin(ActiveUnit->CheckSkill(57),ActiveUnit->GetLife()-ActiveUnit->iLife)*2;
					};
				};
				//Круговая атака
				if ((ActiveUnit->CheckSkill(65)>0)||((ActiveUnit->CheckSkill(66)>0)&&(ActiveUnit->iStamina>ActiveUnit->CheckSkill(66))))
				{					
					int OldX=ActiveUnit->x;
					int OldY=ActiveUnit->y;
					ActiveUnit->x=arMeleeCoord[i][0];
					ActiveUnit->y=arMeleeCoord[i][1];
					int Neig=ActiveUnit->FindNeighbour(1-ActiveUnit->iSide);
					if (Neig>1)
					{
						if ((ActiveUnit->CheckSkill(66)>0)&&(ActiveUnit->iStamina>ActiveUnit->CheckSkill(66)))
						{
							arCanMelee[i]+=100;
						};
						for (int j=0; j<Neig; j++)
						{
							if (arNeigh[j]!=&Squad[1-AA][i])
							{
								Hit+=AI_CalcMeleeHit(ActiveUnit,arNeigh[j]);
								MyTaste+=AI_CalcMeleeHit(ActiveUnit,arNeigh[j])*2;
							};
						};
					};					
					ActiveUnit->x=OldX; ActiveUnit->y=OldY;				
				};
				int Adder=Squad[1-AA][i].CheckSkill(36)*5;
				Adder+=Squad[1-AA][i].GetAttack()/2;				
				Adder-=Squad[1-AA][i].bSummoned*3;
				if ((Hit<Squad[1-AA][i].iLife)&&(ActiveUnit->CheckSkill(26)==0)&&(Squad[1-AA][i].iStamina>0)&&(!Squad[1-AA][i].bResting))
				{
					if (Squad[1-AA][i].CheckSkill(27))
					{Antihit=Squad[1-AA][i].GetCounterAttack()-ActiveUnit->GetResist();}
					else
					{Antihit=Squad[1-AA][i].GetCounterAttack()-ActiveUnit->GetDefence();}
					if (Antihit>0)
					{Adder-=GetMin(Antihit,ActiveUnit->iLife);}
				};
				Adder+=Squad[1-AA][i].GetRangedAttack()*Squad[1-AA][i].iAmmo/5;
				if (Squad[1-AA][i].pUnit->pHero)
				{
					Adder+=Hit*(Squad[1-AA][i].pUnit->pHero->ipClass1/2);
				};
				if ((Hit>=MeleeHit/2)||(Hit>=Squad[1-AA][i].iLife))
				{
					MyTaste+=Adder;
				};
				if ((Adder>0)&&(Hit>=Squad[1-AA][i].iLife))
				{
					MyTaste+=Adder;
				};
				if ((MyTaste>0)&&(Squad[1-AA][i].CheckSkill(73)))
				{
					MyTaste/=10;
				};
				if ((MyTaste>0)&&(Squad[1-AA][i].GetEffectDuration(70)>1))
				{
					MyTaste/=Squad[1-AA][i].GetEffectDuration(70);
				};
				if ((arCanMelee[i]==1)&&(MyTaste>0))
				{MyTaste/=2;};
				if (((WillStrike<0)&&(EnemyArmy==0))||((Hit>0)&&(MyTaste>Taste)))
				{
					Taste=MyTaste;
					WillStrike=i;
					MeleeHit=Hit;
					if (Hit>=Squad[1-AA][i].iLife)
					{KillMelee=true;}
					else
					{KillMelee=false;};
				};			
			};
		};
		};
		

		//Выбор действия
		//**************

		//Заклинание
		if ((ChoosenSpell>=0)&&(SpellHit>=ShootHit)&&(SpellHit>=MeleeHit)&&(SpellHit>=HealHit))
		{
			WriteLog("Заклинание");
			IndexAI=-1;
			clSpell *SP=(pFirstSpell+arCast[ChoosenSpell][0]);
			if (bShowAnim)
			{
				PlaySpellSound(SP->wavCast);
			//	play_sample(SP->wavCast,iSoundVolume,128,1000,0);
			};
			switch (SP->iTarget)
			{
			case 1:
				if (SP->arEffect[0][0]==89)
				{
					//Телепорт
					int BigHit=0, spX, spY;
					for (int x=0; x<8; x++)
					{
						for (int y=0; y<8; y++)
						{
							int Hit=GetTeleportTaste(&Squad[SpellTarget1][SpellTarget2],x,y);
							if (Hit>BigHit)
							{
								BigHit=Hit;
								spX=x;
								spY=y;
							};
						};
					};
					Squad[SpellTarget1][SpellTarget2].Replace(spX,spY);
					Squad[SpellTarget1][SpellTarget2].G_Obj->Make_Stars(Squad[SpellTarget1][SpellTarget2].G_Obj,50,40,10,0,44,40,10,200,200);
				}
				else
				{
					Squad[SpellTarget1][SpellTarget2].UnderSpell(SP,ActiveUnit,AA);
				};
				break;
			case 2:
			case 4:
				CorpseSpell(SP,&Squad[SpellTarget1][SpellTarget2]);
				break;
			case 5:
				TileSpell(SP,SpellTarget1,SpellTarget2);
				break;
			case 3:
				for (int x=0; x<8; x++){
				for (int y=0; y<8; y++)
				{
					Field[x][y].Land=0;
					Field[x][y].Pass=0;
					Field[x][y].Shoot=0;
				};
				};					
				ShootMap(SpellTarget1,SpellTarget2,-1,SP->iRadius);					
				if ((SP->bOnAlly)||(SP->bOnEnemy))
				{
					for (int x=0; x<8; x++)
					{
						for (int y=0; y<8; y++)
						{								
							if ((Field[x][y].Shoot>0)||((x==SpellTarget1)&&(y==SpellTarget2)))
							{
								if (SP->bOnAlly)
								{
									for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
									{
										if ((Squad[AA][k].pUnit)&&(Squad[AA][k].iLife>0)&&(Squad[AA][k].x==x)&&(Squad[AA][k].y==y))
										{
											int AntiSkill=0;
											for (int i=0; i<MAX_ANTI_EFFECT; i++)
											{
												AntiSkill+=SP->arAntiEffect[i]*Squad[AA][k].CheckSkill(SP->arAntiEffect[i]);
											};
											for (int i=0; i<MAX_UNIT_KIND; i++)
											{
												if (SP->arUnitKind[i]==Squad[AA][k].GetUnitKind())
												{AntiSkill=1;};
											};
											if ((!AntiSkill)||(Squad[AA][k].CheckSkill(205)==arCast[ChoosenSpell][0]))
											{													
										//Найден свой подходящий юнит
												Squad[AA][k].UnderSpell(SP,ActiveUnit,AA);												
											};
										};
									};
								};
								if (SP->bOnEnemy)
								{
									for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
									{
										if ((Squad[1-AA][k].pUnit)&&(Squad[1-AA][k].iLife>0)&&(Squad[1-AA][k].x==x)&&(Squad[1-AA][k].y==y))
										{
											int AntiSkill=0;
											for (int i=0; i<MAX_ANTI_EFFECT; i++)
											{
												AntiSkill+=SP->arAntiEffect[i]*Squad[1-AA][k].CheckSkill(SP->arAntiEffect[i]);
											};
											for (int i=0; i<MAX_UNIT_KIND; i++)
											{
												if (SP->arUnitKind[i]==Squad[1-AA][k].GetUnitKind())
												{AntiSkill=1;};
											};
											if ((!AntiSkill)||(Squad[1-AA][k].CheckSkill(205)==arCast[ChoosenSpell][0]))
											{
									//Найден вражеский подходящий юнит
												Squad[1-AA][k].UnderSpell(SP,ActiveUnit,AA);
											};
										};
									};
								};
							};								
						};
					};
				}
				else
				{
					for (int x=0; x<8; x++)
					{
						for (int y=0; y<8; y++)
						{
							if ((Field[x][y].Shoot>0)||((x==SpellTarget1)&&(y==SpellTarget2)))
							{
						//Найден тайл для заклинания
							};
						};
					};
				};
				break;
			case 0:	
				if (SP->iArea==2)
				{
					if (SP->bOnAlly)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[AA][k].pUnit)&&(Squad[AA][k].iLife>0))
							{
								int AntiSkill=0;
								for (int i=0; i<MAX_ANTI_EFFECT; i++)
								{
									AntiSkill+=SP->arAntiEffect[i]*Squad[AA][k].CheckSkill(SP->arAntiEffect[i]);
								};
								for (int i=0; i<MAX_UNIT_KIND; i++)
								{
									if (SP->arUnitKind[i]==Squad[AA][k].GetUnitKind())
									{AntiSkill=1;};
								};
								if ((!AntiSkill)||(Squad[AA][k].CheckSkill(205)==arCast[ChoosenSpell][0]))
								{													
							//Найден свой подходящий юнит
									Squad[AA][k].UnderSpell(SP,ActiveUnit,AA);												
								};
							};
						};
					};
					if (SP->bOnEnemy)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[1-AA][k].pUnit)&&(Squad[1-AA][k].iLife>0))
							{
								int AntiSkill=0;
								for (int i=0; i<MAX_ANTI_EFFECT; i++)
								{
									AntiSkill+=SP->arAntiEffect[i]*Squad[1-AA][k].CheckSkill(SP->arAntiEffect[i]);
								};
								for (int i=0; i<MAX_UNIT_KIND; i++)
								{
									if (SP->arUnitKind[i]==Squad[1-AA][k].GetUnitKind())
									{AntiSkill=1;};
								};
								if ((!AntiSkill)||(Squad[1-AA][k].CheckSkill(205)==arCast[ChoosenSpell][0]))
								{
						//Найден вражеский подходящий юнит
									Squad[1-AA][k].UnderSpell(SP,ActiveUnit,AA);
								};
							};
						};
					};
				};
				if (SP->iArea==0)
				{ActiveUnit->UnderSpell(SP,ActiveUnit,AA);}
				break;
			};
			SpellCastAI((pFirstSpell+arCast[ChoosenSpell][0]));
			return 1;
		};

		//Лечение
		if ((WillHeal>=0)&&(HealHit>=ShootHit)&&(HealHit>=MeleeHit))
		{
			IndexAI=-1;
			if ((ActiveUnit->x==arHealCoord[WillHeal][0])&&(ActiveUnit->y==arHealCoord[WillHeal][1]))
			{
				ActiveUnit->Heal(&Squad[AA][WillHeal]);
				return 1;
			}
			else
			{
				if (arCanHeal[WillHeal]==1)
				{
					ActiveUnit->GoMarch();
				};
				ActiveUnit->Move(arHealCoord[WillHeal][0],arHealCoord[WillHeal][1]);
				while (ActiveUnit->G_Obj->Track)
				{
					BF_RefreshScreen();
				};
				ActiveUnit->Heal(&Squad[AA][WillHeal]);
				return 1;
			}
		};
		

		//Стрельба
		if ((WillShoot>=0)&&((ShootHit>=MeleeHit)||(KillShoot)||((ActiveUnit->GetAmmo()>3)&&(ActiveUnit->GetAttack()<ActiveUnit->GetRangedAttack())&&(ShootHit>=MeleeHit/2)))&&((!KillMelee)||(KillShoot)))
		{
			IndexAI=-1;
			if ((ActiveUnit->x==arRangeCoord[WillShoot][0])&&(ActiveUnit->y==arRangeCoord[WillShoot][1]))
			{				
				if (arCanRange[WillShoot]==4)
				{buttDoubleShot.bPress=true;};
				if (ActiveUnit->CheckSkill(47))
				{ActiveUnit->G_Obj->AnimText((pFirstAbilityNum+47)->sName,makecol(250,250,30));};
				if (ActiveUnit->CheckSkill(46))
				{ActiveUnit->G_Obj->AnimText((pFirstAbilityNum+46)->sName,makecol(250,250,30));};
				ActiveUnit->Shoot(&Squad[1-AA][WillShoot]);
				buttDoubleShot.bPress=false;
				return 1;
			}
			else
			{
				if (arCanRange[WillShoot]==1)
				{
					ActiveUnit->GoMarch();
				};
				ActiveUnit->Move(arRangeCoord[WillShoot][0],arRangeCoord[WillShoot][1]);
				while (ActiveUnit->G_Obj->Track)
				{
					BF_RefreshScreen();
				};
				if (ActiveUnit->CheckSkill(47))
				{ActiveUnit->G_Obj->AnimText((pFirstAbilityNum+47)->sName,makecol(250,250,30));};
				if (ActiveUnit->CheckSkill(46))
				{ActiveUnit->G_Obj->AnimText((pFirstAbilityNum+46)->sName,makecol(250,250,30));};
				ActiveUnit->Shoot(&Squad[1-AA][WillShoot]);
				return 1;
			}
		}
		else
		{
			//Рукопашная
			if (WillStrike>=0)
			{
				WriteLog("Рукопашная");
				IndexAI=-1;
//				clear_keybuf();
//				textprintf(screen,Font1,10,10,COLOR_2,"Рукопашная: %d (%d) Выстрел: %d (%d)",MeleeHit, WillStrike, ShootHit, WillStrike);
//				readkey();
				if (arCanMelee[WillStrike]==1)
				{					
					ActiveUnit->GoMarch();
				};
				if (arCanMelee[WillStrike]%100==5)
				{
					buttCrush.bPress=true;
		//			ActiveUnit->G_Obj->AnimText(69,COLOR_2);
				};
				if (arCanMelee[WillStrike]>99)
				{
					buttRoundAtt.bPress=true;
		//			ActiveUnit->G_Obj->AnimText(69,COLOR_2);
				};
				ActiveUnit->Attack(arMeleeCoord[WillStrike][0],arMeleeCoord[WillStrike][1],&Squad[1-AA][WillStrike]);
				buttCrush.bPress=false;
				buttRoundAtt.bPress=false;
				return 2;
			}
		};
			
		//Цель не найдена
		//***************
		if (CanMove)
		{
			
			//Сбор снарядов
			if ((ActiveUnit->CheckSkill(23))&&(ActiveUnit->iAmmo==0))
			{
				ActiveUnit->iSpeed=0;
				ActiveUnit->bCanAct=false;
				int AddAmmo=Random(ActiveUnit->CheckSkill(23)+1);
				ActiveUnit->iAmmo+=AddAmmo;
				ActiveUnit->DrawBar();
				ActiveUnit->G_Obj->AnimText(262,COLOR_3);
				int Count=20;
				while ((Count>0)&&(bShowAnim))
				{
					if (Timer)
					{
						BF_RefreshScreen();
						Count--;
					};
				};
				ActiveUnit->G_Obj->AnimNumber(AddAmmo,makecol(250,250,50));
				if (bShowAnim)
				{
					PlayUnitSound(wavGather);
				//	play_sample(wavGather,iSoundVolume,128,1000,0);
				};
				return 0;
			};
			
			//Медитация
			if ((ActiveUnit->CheckSkill(22))&&(ActiveUnit->iAmmo==0))
			{
				ActiveUnit->iSpeed=0;
				ActiveUnit->bCanAct=false;
				ActiveUnit->bResting=true;
				ActiveUnit->DrawBar();
				return 0;
			};

			//Отдых
			if ((ActiveUnit->iStamina<=ActiveUnit->GetStamina()/2)&&(ActiveUnit->iSpeed==ActiveUnit->GetSpeed())&&(arUnitPower[AA][ActiveNum][3]==0)&&(arUnitPower[AA][ActiveNum][4]==0))
			{
				WriteLog("Отдых");
				ActiveUnit->iSpeed=0;
				ActiveUnit->bCanAct=false;
				ActiveUnit->bResting=true;
				ActiveUnit->DrawBar();
				return 0;
			};

			//Первая помощь
			if ((ActiveUnit->iLife<ActiveUnit->GetLife())&&(ActiveUnit->iSpeed==ActiveUnit->GetSpeed())&&(ActiveUnit->CheckSkill(35))&&(arUnitPower[AA][ActiveNum][3]==0)&&(arUnitPower[AA][ActiveNum][4]==0))
			{
				WriteLog("Первая помощь");
				ActiveUnit->iSpeed=0;
				ActiveUnit->bCanAct=false;
				ActiveUnit->bResting=true;
				ActiveUnit->DrawBar();
				return 0;
			};

			//Трупоедство
			if ((ActiveUnit->CheckSkill(49))&&(ActiveUnit->iLife<=ActiveUnit->GetLife()/2))
			{
				int Corpse=FindCorpse(1-AA,ActiveUnit->x,ActiveUnit->y);
				int CSq=1-AA;
				if (Corpse<0)
				{
					Corpse=FindCorpse(AA,ActiveUnit->x,ActiveUnit->y);
					CSq=AA;
				};
				if (Corpse>=0)
				{					
					Squad[CSq][Corpse].bInBattle=false;
					Squad[CSq][Corpse].x=10;
					Squad[CSq][Corpse].y=10;
					Squad[CSq][Corpse].G_Obj->iLayer=-1;
					Corpse=ActiveUnit->CheckSkill(49)/2+Random(ActiveUnit->CheckSkill(49)/2+1);
					if (Corpse>ActiveUnit->GetLife()-ActiveUnit->iLife)
					{
						Corpse=ActiveUnit->GetLife()-ActiveUnit->iLife;
					};
					ActiveUnit->iLife+=Corpse;
					ActiveUnit->G_Obj->AnimText(68,COLOR_2);
					if (bShowAnim)
					{
						PlayUnitSound(wavEat);
					//	play_sample(wavEat,iSoundVolume,128,1000,0);
					};
					int Count=20;
					while ((Count>0)&&(bShowAnim))
					{
						if (Timer)
						{
							BF_RefreshScreen();
							Count--;
						};
					};
					ActiveUnit->G_Obj->AnimNumber(Corpse,makecol(50,250,0));
					ActiveUnit->iSpeed=0;
					ActiveUnit->bCanAct=false;
					ActiveUnit->DrawBar();
					return 0;
				}
				else
				{
					int CorpseX, CorpseY, CorpseNum, CorpseS, CorpseRun=0;
				//	ActiveUnit->bMarch=false;
					AI_SelectUnit(ActiveUnit,false);
					for (int x=0; x<8; x++)
					{
						for (int y=0; y<8; y++)
						{
							if ((arGrid[x][y].Pass)&&(Field[x][y].Pass))
							{
								if ((CorpseRun==0)||(Field[x][y].Land<ActiveUnit->iSpeed))
								{
									int Corp=FindCorpse(1-AA,x,y);
									if (Corp<0)
									{
										Corp=FindCorpse(AA,x,y);
										if (Corp>=0)
										{
											CorpseS=AA;
										};
									}
									else
									{
										CorpseS=1-AA;
									};
									if (Corp>=0)
									{
										CorpseNum=Corp;
										CorpseX=x;
										CorpseY=y;
										if (Field[x][y].Land<ActiveUnit->iSpeed)
										{CorpseRun=2;}
										else
										{CorpseRun=1;};
									};
								};
							};
						};
					};
					if (CorpseRun>0)
					{
					ActiveUnit->Move(CorpseX,CorpseY);					
					if (CorpseRun>1)
					{
						IndexAI=-1;
						while(ActiveUnit->G_Obj->Track)
						{
							BF_RefreshScreen();
						};
						Squad[CorpseS][CorpseNum].bInBattle=false;
						Squad[CorpseS][CorpseNum].x=10;
						Squad[CorpseS][CorpseNum].y=10;
						Squad[CorpseS][CorpseNum].G_Obj->iLayer=-1;
						CorpseNum=ActiveUnit->CheckSkill(49)/2+Random(ActiveUnit->CheckSkill(49)/2+1);
						if (CorpseNum>ActiveUnit->GetLife()-ActiveUnit->iLife)
						{
							CorpseNum=ActiveUnit->GetLife()-ActiveUnit->iLife;
						};
						ActiveUnit->iLife+=CorpseNum;
						ActiveUnit->G_Obj->AnimText(68,COLOR_2);
						if (bShowAnim)
						{
							PlayUnitSound(wavEat);
						//	play_sample(wavEat,iSoundVolume,128,1000,0);
						};
						int Count=20;
						while ((Count>0)&&(bShowAnim))
						{
							if (Timer)
							{
								BF_RefreshScreen();
								Count--;
							};
						};
						ActiveUnit->G_Obj->AnimNumber(CorpseNum,makecol(50,250,0));
						ActiveUnit->iSpeed=0;
						ActiveUnit->bCanAct=false;
						ActiveUnit->DrawBar();
						return 0;
					}
					else
					{return 1;};
					};
				};
			};

			int GoMap[8][8];
			for (int i=0; i<8; i++){
			for (int j=0; j<8; j++)
			{
				Field[i][j].Land=0;
				Field[i][j].Pass=0;
				Field[i][j].Shoot=0;
				TryFly[i][j]=true;
			};
			};
			

//Построение карты поиска пути
			if (ActiveUnit->iSpeed<1)
			{
				ActiveUnit->bCanAct=false;
				if ((ActiveUnit->iStamina<ActiveUnit->GetStamina())&&(ActiveUnit->iSpeed==ActiveUnit->GetSpeed()))
				{
					ActiveUnit->bResting=true;
					ActiveUnit->DrawBar();
				};
				return 0;
			};
			for (int i=0; i<3; i++)
			{CanPass[i]=0;};

			bFirstMove=(ActiveUnit->iSpeed>=ActiveUnit->GetSpeed());
			if ((!ActiveUnit->bMarch)&&(ActiveUnit->iSpeed<=ActiveUnit->GetSpeed()))
			{bFirstMove=false;};

			if (ActiveUnit->CheckSkill(32))
			{CanPass[0]=2;};

			
			if (ActiveUnit->CheckSkill(33))
			{CanPass[1]=3;};

			
			if (ActiveUnit->CheckSkill(34))
			{CanPass[2]=4;};

			
	//		WriteLog("Строю карту");
			if (ActiveUnit->CheckSkill(14)) //Летающий
			{
				FlyAIMap(ActiveUnit->x,ActiveUnit->y,-1,ActiveUnit->GetSpeed(),ActiveUnit->iSpeed,true);
			}
			else
			{
				if (ActiveUnit->CheckSkill(15)) //Низколетающий
				{
					FlyAIMap(ActiveUnit->x,ActiveUnit->y,-1,ActiveUnit->GetSpeed(),ActiveUnit->iSpeed,false);
				}
				else
				{				
					StepAIMap(ActiveUnit->x,ActiveUnit->y,-1,ActiveUnit->GetSpeed(),ActiveUnit->iSpeed,0,80);
				};
			};
//			WriteLog("Карта создана");
		//Поиск тайла для движения
			for (int i=0; i<8; i++){
			for (int j=0; j<8; j++)
			{
				if ((Field[i][j].Pass)&&(arGrid[i][j].Pass))
				{
					GoMap[i][j]=Field[i][j].Land;
				}
				else
				{
					GoMap[i][j]=0;
				};
			};
			};
			int OldX=ActiveUnit->x, OldY=ActiveUnit->y;
			int Way=1000, x1=8, y1=8, Army=0;
			bool bNeedGo=true;
			for (int i=0; i<MAX_SQUAD; i++)
			{
				if ((Squad[AA][i].pUnit)&&(Squad[AA][i].iLife>0)&&(Squad[AA][i].GetAttack()+Squad[AA][i].GetRangedAttack()*Squad[AA][i].iAmmo>Squad[AA][i].CheckSkill(24)*Squad[AA][i].iAmmo))
				{Army++;};				
			};			
			for (int x=0; x<8; x++)
			{
			for (int y=0; y<8; y++)
			{
				if ((GoMap[x][y]>0)&&(GoMap[x][y]<Way))
				{

					ActiveUnit->x=x;
					ActiveUnit->y=y;					
					if ((ActiveUnit->CheckSkill(24)*ActiveUnit->iAmmo<=0)||(ActiveUnit->GetAttack()+ActiveUnit->GetRangedAttack()*ActiveUnit->iAmmo>ActiveUnit->CheckSkill(24)*ActiveUnit->iAmmo)||(Army<=1))
					{
						//Поиск цели для выстрела
						if (ActiveUnit->GetRangedAttack()*ActiveUnit->iAmmo>0)
						{
							for (int i=0; i<8; i++){
							for (int j=0; j<8; j++)
							{
								Field[i][j].Shoot=0;
								Field[i][j].Land=0;
							};
							};
							ShootMap(x,y,-1,ActiveUnit->GetShootingRange());
							for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
							{
								if ((Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0)&&(Field[Squad[1-AA][i].x][Squad[1-AA][i].y].Shoot==2)&&((EnemyArmy==0)||(CalcRangedHit(ActiveUnit,&Squad[1-AA][i])>0)))
								{
									x1=x;
									y1=y;
									Way=GoMap[x][y];
								};
							};
						};
						//Поиск цели для рукопашной
						int NEI=ActiveUnit->FindNeighbour(1-AA);
						for (int i=0; i<NEI; i++)
						{
							if (arNeigh[i])
							{
								if ((EnemyArmy==0)||(((arNeigh[i]->pUnit->GetUnitType()!=60)||(arNeigh[i]->pUnit==arNeigh[i]->pTrueForm))&&(CalcMeleeHit(ActiveUnit,arNeigh[i],true)>0)))
								{
									x1=x;
									y1=y;
									Way=GoMap[x][y];
								};
							};
						};
					}
					else
					{
						//Поиск цели для лечения
						bNeedGo=false;
						for (int i=0; i<8; i++){
						for (int j=0; j<8; j++)
						{
							Field[i][j].Shoot=0;
							Field[i][j].Land=0;
						};
						};
						ShootMap(x,y,-1,ActiveUnit->GetShootingRange());
						for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
						{
							if ((Squad[AA][i].pUnit)&&(Squad[AA][i].iLife>0)&&(Squad[AA][i].pUnit->GetUnitKind()!=2)&&(Squad[AA][i].pUnit->GetUnitKind()!=4)
								&&(Squad[AA][i].iLife<Squad[AA][i].GetLife())&&(Field[Squad[AA][i].x][Squad[AA][i].y].Shoot>0))
							{
								x1=x;
								y1=y;
								Way=GoMap[x][y];
							};
						};
						/*if (ActiveUnit->FindNeighbour(AA)>0)
						{
							bool WillGo=false;
							for (int i=0; i<ActiveUnit->FindNeighbour(AA); i++)
							{								
								if (arNeigh[i]->iLife<arNeigh[i]->GetLife())
								{
									WillGo=true;
								};
							};
							if (WillGo)
							{
								x1=x;
								y1=y;
								Way=GoMap[x][y];
							};
						};*/
					};
				};
			};
			};
			ActiveUnit->x=OldX;
			ActiveUnit->y=OldY;
			if ((x1==8)&&(bNeedGo))
			{
				int Dist=50, TRG;
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{
					if ((Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0)&&(abs(Squad[1-AA][i].x-ActiveUnit->x)+abs(Squad[1-AA][i].y-ActiveUnit->y)<Dist))
					{
						Dist=abs(Squad[1-AA][i].x-ActiveUnit->x)+abs(Squad[1-AA][i].y-ActiveUnit->y);
						TRG=i;
					};
				};
				if (Dist<50)
				{
					for (int x=0; x<8; x++)
					{
						for (int y=0; y<8; y++)
							{
							if ((GoMap[x][y]>0)&&(abs(Squad[1-AA][TRG].x-x)+abs(Squad[1-AA][TRG].y-y)<Dist))
							{
								Dist=abs(Squad[1-AA][TRG].x-x)+abs(Squad[1-AA][TRG].y-y);
								x1=x;
								y1=y;
							};
						};
					};
				};
			};
			if (x1<8)
		//Движение
			{					
				for (int i=0; i<8; i++){
				for (int j=0; j<8; j++)
				{
					Field[i][j].Land=0;
					Field[i][j].Pass=0;
					Field[i][j].Shoot=0;
					TryFly[i][j]=true;
				};
				};

	//Построение карты поиска пути
				for (int i=0; i<3; i++)
				{CanPass[i]=0;};
				bFirstMove=(ActiveUnit->iSpeed>=ActiveUnit->GetSpeed());
				if ((!ActiveUnit->bMarch)&&(ActiveUnit->iSpeed<=ActiveUnit->GetSpeed()))
				{bFirstMove=false;};
				if (ActiveUnit->CheckSkill(32))
				{CanPass[0]=2;};			
				if (ActiveUnit->CheckSkill(33))
				{CanPass[1]=3;};			
				if (ActiveUnit->CheckSkill(34))
				{CanPass[2]=4;};
			
		//		WriteLog("Строю карту");
				if (ActiveUnit->CheckSkill(14)) //Летающий
				{
					FlyAIMap(ActiveUnit->x,ActiveUnit->y,-1,ActiveUnit->GetSpeed(),ActiveUnit->iSpeed,true);
				}
				else
				{
					if (ActiveUnit->CheckSkill(15)) //Низколетающий
					{
						FlyAIMap(ActiveUnit->x,ActiveUnit->y,-1,ActiveUnit->GetSpeed(),ActiveUnit->iSpeed,false);
					}
					else
						{				
						StepAIMap(ActiveUnit->x,ActiveUnit->y,-1,ActiveUnit->GetSpeed(),ActiveUnit->iSpeed,0,80);
					};
				};
				TruePath=NULL;
				MinWeight=GoMap[x1][y1];
				MinStamina=500;
				bmpPathPic=ActiveUnit->G_Obj->Pic;
			//	ActiveUnit->bMarch=false;
				for (int i=0; i<8; i++){
				for (int j=0; j<8; j++){
					TryFly[i][j]=true;	
				};
				};
				clPath *Step;
				clPath *DeadEnd;		
				if ((ActiveUnit->CheckSkill(14))||(ActiveUnit->CheckSkill(15)))
				{
					FlyAIPath(ActiveUnit->x,ActiveUnit->y,-1,NULL,x1,y1,ActiveUnit->GetSpeed(),ActiveUnit->iSpeed);
					if (TruePath)
					{
					{WriteLog("Путь найден");};
					DeadEnd=TruePath;
					AI_SelectUnit(ActiveUnit,false);
					int GoSpeed=ActiveUnit->iSpeed;
					while (GoSpeed>0)
					{
						Step=TruePath;
						for (int i=0; i<=GoSpeed; i++)
						{
							if ((Step)&&(Step->pPrev))
							{
								Step=Step->pPrev; //вылет!!!!
							};
						};
				//		Step=TruePath;
				//		DeadEnd=TruePath;
				//		AI_SelectUnit(ActiveUnit);
				//		while ((Step)&&(Step->pPrev)&&(Field[Step->pPrev->x][Step->pPrev->y].Pass)&&
				//		(arGrid[Step->pPrev->x][Step->pPrev->y].Pass))
				//		{Step=Step->pPrev;};
						if ((Step)&&(Field[Step->x][Step->y].Pass)&&(arGrid[Step->x][Step->y].Pass))
						{
							IndexAI=-1;
							ActiveUnit->Move(Step->x,Step->y);
							while (DeadEnd)
							{
								Step=DeadEnd;
								DeadEnd=DeadEnd->pPrev;
								delete Step;								
							};					
							return 1;
						};
						GoSpeed--;
					};
					if (IndexAI==ActiveUnit->iNumm)
					{IndexAI=-100;};
					if (IndexAI==-1)
					{IndexAI=ActiveUnit->iNumm;};
					while (DeadEnd)
					{
						Step=DeadEnd;
						DeadEnd=DeadEnd->pPrev;
						delete Step;								
					};
			/*		if (ActiveUnit->bWait)
					{
						ActiveUnit->bCanAct=false;
					}
					else
					{
						ActiveUnit->bWait=true;
					};
					*/}
				}
				else
				{
					StepAIPath(ActiveUnit->x,ActiveUnit->y,-1,NULL,x1,y1,ActiveUnit->GetSpeed(),ActiveUnit->iSpeed,0,80);
					if (TruePath)
					{
						WriteLog("Путь найден");
					};
					Step=TruePath;
					DeadEnd=TruePath;
					AI_SelectUnit(ActiveUnit,false);
					while ((Step->pPrev)&&(Field[Step->pPrev->x][Step->pPrev->y].Pass)&&
					(arGrid[Step->pPrev->x][Step->pPrev->y].Pass))
					{Step=Step->pPrev;};
				
			//	clPath *Step=TruePath;
			//	clPath *DeadEnd=TruePath;;
				
					if ((Step->x!=ActiveUnit->x)||(Step->y!=ActiveUnit->y))
					{
						IndexAI=-1;
						ActiveUnit->Move(Step->x,Step->y);
						while (DeadEnd)
						{
							Step=DeadEnd;
							DeadEnd=DeadEnd->pPrev;
							delete Step;								
						};					
						return 1;
					}
					else
					{
						if (IndexAI==ActiveUnit->iNumm)
						{IndexAI=-100;};
						if (IndexAI==-1)
						{IndexAI=ActiveUnit->iNumm;};
						while (DeadEnd)
						{
							Step=DeadEnd;
							DeadEnd=DeadEnd->pPrev;
							delete Step;								
						};
				/*		if (ActiveUnit->bWait)
						{
							ActiveUnit->bCanAct=false;
						}
						else
						{
							ActiveUnit->bWait=true;
						};
				*/
					};
				};
			}
			else
			{
				//Установка флага выхода из цикла
		//		textprintf(screen,Font1,0,0,COLOR_2,"Нет пути, сторона");
		//		textprintf(screen1,Font1,0,0,COLOR_2,"Нет пути, сторона");
				/*int x1=ActiveUnit->x, y1=ActiveUnit->y, Nei;
				for (int x=0; x<8; x++)
				{
					for (int y=0; y<8; y++)
					{
						ActiveUnit->x=x;
						ActiveUnit->y=y;						
						int Nei=ActiveUnit->FindNeighbour(1-AA);
						if (x>4)
						{
							textprintf(screen,Font1,(x-4)*200,y*20,COLOR_2,"(%d, %d): П: %d С: %d",x,y,GoMap[x][y],Nei);
							textprintf(screen1,Font1,(x-4)*200,y*20,COLOR_2,"(%d, %d): П: %d С: %d",x,y,GoMap[x][y],Nei);
						};
					};
				};
				ActiveUnit->x=x1; ActiveUnit->y=y1;
								
				clear_keybuf();
				readkey();*/
				if (IndexAI==ActiveUnit->iNumm)
				{IndexAI=-100;};
				if (IndexAI==-1)
				{IndexAI=ActiveUnit->iNumm;};
			};
		};
	};	
	return 0;
};








void AI_Simulate()
{	
	int arCanMelee[MAX_SQUAD+MAX_SUMMONS];
	int arMeleeCoord[MAX_SQUAD+MAX_SUMMONS][2];
	int arCanRange[MAX_SQUAD+MAX_SUMMONS];
	int arRangeCoord[MAX_SQUAD+MAX_SUMMONS][2];
	int arCanHeal[MAX_SQUAD+MAX_SUMMONS];
	int arHealCoord[MAX_SQUAD+MAX_SUMMONS][2];
	int arCanCast[MAX_SQUAD+MAX_SUMMONS][2];
//	int MoveMap [8][8];
	int ActiveNum;
	//if (FindNextUnit())
	//{
	//	AI_SelectUnit(FindNextUnit());
	//};
	if ((ActiveUnit)&&(ActiveUnit->bCanAct))
	{
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if (&Squad[AA][i]==ActiveUnit)
			{ActiveNum=i;};
		};

		//Поиск целей
		//***********

//Магия
//*****		
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			arCanCast[i][0]=0;
		};
	//Составление списка доступных заклинаний
		int arCast[SPELLBOOK_SIZE][2];
		for (int i=0; i<SPELLBOOK_SIZE; i++)
		{
			arCast[i][0]=0;
			arCast[i][1]=0;
		};
		int SpellCount=0;
		if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
		{
			for (int i=0; i<SPELLBOOK_SIZE; i++)
			{
				if (Spells[AA][0][i])
				{
					if ((Spells[AA][1][i])&&(arPlayer[BF_Player[AA]].Gem>=(pFirstSpell+Spells[AA][0][i])->iCost*(100-ActiveUnit->pHero->CheckHeroBonus(983))/100))
					{
						bool bNew=true;
						arCast[i][0]=Spells[AA][0][i];
						arCast[i][1]=i;
						SpellCount++;
					};
				};
			};	
		}
		else
		{		
			for (int i=1; i<=iMaxSpell; i++)
			{
				if ((ActiveUnit->CheckSkill(2000+i))&&(ActiveUnit->iAmmo>=ActiveUnit->CheckSkill(2000+i)))
				{				
					arCast[SpellCount][0]=i;
					arCast[SpellCount][1]=-1;
					SpellCount++;
				};				
			};
		};
		//Выбор заклинания
		int ChoosenSpell=-1, SpellTaste=0, SpellTarget1=-1, SpellTarget2=-1, SpellHit=0;
		if (SpellCount>0)
		{
//			AI_CalcDangerZone();
//			AI_SelectUnit(ActiveUnit);
			for (int Spell=0; Spell<SPELLBOOK_SIZE; Spell++)
			{
				int Damage=0, Buff=0, Summon=0, Penalty=0;				
				if (arCast[Spell][0]>0)
				{
					clSpell *SP=(pFirstSpell+arCast[Spell][0]);
					int SpellPower;
					//Одиночная цель
					if (SP->iTarget==1)
					{
						//Противник
						if (SP->bOnEnemy)
						{
							for (int Uni=0; Uni<MAX_SQUAD+MAX_SUMMONS; Uni++)
							{
							if ((Squad[1-AA][Uni].pUnit)&&(Squad[1-AA][Uni].iLife>0))
							{								
								int MyTaste=AI_GetEnemySpellTaste(arCast[Spell][0],&Squad[1-AA][Uni],Uni);
								MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
								if ((MyTaste>SpellTaste)&&((iGainSpellDamage>0)||(MyTaste>0)))
								{
	//								textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
	//								readkey();
									SpellTaste=MyTaste;
									SpellHit=iGainSpellDamage;
									ChoosenSpell=Spell;
									SpellTarget1=1-AA;
									SpellTarget2=Uni;
								};							
							};
							};
						};
						//Союзник
						if (SP->bOnAlly)
						{
							for (int Uni=0; Uni<MAX_SQUAD+MAX_SUMMONS; Uni++)
							{
							if ((Squad[AA][Uni].pUnit)&&(Squad[AA][Uni].iLife>0))
							{
								int MyTaste=AI_GetAllySpellTaste(arCast[Spell][0],&Squad[AA][Uni],Uni);
								MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
								if ((MyTaste>SpellTaste)&&((iGainSpellDamage>0)||(MyTaste>0)))
								{
			//						textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
			//						readkey();
									SpellTaste=MyTaste;
									SpellHit=iGainSpellDamage;
									ChoosenSpell=Spell;
									SpellTarget1=AA;
									SpellTarget2=Uni;
								};
							};
							};
						};
					};
					//Вызов заклятья без цели (на все поле)
					if ((SP->iTarget==0)&&(SP->iArea==2))
					{
						int MyTaste=0, MyDamage=0;
						if (SP->bOnAlly)
						{
							for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
							{
								if ((Squad[AA][k].pUnit)&&(Squad[AA][k].iLife>0))
								{									
								//Найден свой подходящий юнит
									iGainSpellDamage=0;
									MyTaste+=AI_GetAllySpellTaste(arCast[Spell][0],&Squad[AA][k],k);
									MyDamage+=iGainSpellDamage;
								};
							};
						};
						if (SP->bOnEnemy)
						{
							for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
							{
								if ((Squad[1-AA][k].pUnit)&&(Squad[1-AA][k].iLife>0))
								{						
								//Найден вражеский подходящий юнит
									iGainSpellDamage=0;
									MyTaste+=AI_GetEnemySpellTaste(arCast[Spell][0],&Squad[1-AA][k],k);
									MyDamage+=iGainSpellDamage;
								};
							};
						};
						iGainSpellDamage=MyDamage;
						MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
	//					textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
	//					readkey();
						if ((MyTaste>SpellTaste)&&((iGainSpellDamage>0)||(MyTaste>0)))
						{
					//		textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
					//		readkey();
							SpellTaste=MyTaste;
							SpellHit=iGainSpellDamage;
							ChoosenSpell=Spell;							
						};
					};
					//На себя
					if ((SP->iTarget==0)&&(SP->iArea==0))
					{
						int MyTaste=AI_GetAllySpellTaste(arCast[Spell][0],ActiveUnit,ActiveNum);
						MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
						if ((MyTaste>SpellTaste)&&((iGainSpellDamage>0)||(MyTaste>0)))
						{
							SpellTaste=MyTaste;
							SpellHit=iGainSpellDamage;
							ChoosenSpell=Spell;
						}
					}
					//Труп юнита
					if ((SP->iTarget==2)||(SP->iTarget==4))
					{
						//Противник
						if (SP->bOnEnemy)
						{
							for (int Uni=0; Uni<MAX_SQUAD+MAX_SUMMONS; Uni++)
							{
								if ((Squad[1-AA][Uni].pUnit)&&(Squad[1-AA][Uni].iLife==0)&&(Squad[1-AA][Uni].bInBattle)&&((SP->iTarget==2)||(arGrid[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y].Pass)))
							{
								Buff=0;
								Damage=0;
								if (SP->arEffect[0][0]>1000)
								{	
									SpellPower=SP->arEffect[1][0];
									if (ActiveUnit->pHero)
									{
										SpellPower+=ActiveUnit->pHero->CheckHeroBonus(907)+ActiveUnit->pHero->CheckHeroBonus(909);
									};
									Buff=((pFirstUnitType+(SP->arEffect[0][0]-1000))->iLevel+1)*10+SpellPower*3-DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]+(pFirstUnitType+(SP->arEffect[0][0]-1000))->iLife;
									if (DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]<2)
									{
										Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2;
									}
									else
									{
										Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2-DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]-1+(pFirstUnitType+(SP->arEffect[0][0]-1000))->iSpeed;
										if (Damage<1)
										{Damage=1;};
									};
								};
								int MyTaste=Buff;								
								MyTaste+=Damage*3;
								MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
								if ((MyTaste>SpellTaste)&&((Damage>0)||(Buff>0)))
								{
		//							textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
		//							readkey();
									SpellTaste=MyTaste;
									SpellHit=Damage;
									ChoosenSpell=Spell;
									SpellTarget1=1-AA;
									SpellTarget2=Uni;
								};
							};
							};

						};
						if (SP->bOnAlly)
						{
							for (int Uni=0; Uni<MAX_SQUAD+MAX_SUMMONS; Uni++)
							{
								if ((Squad[AA][Uni].pUnit)&&(Squad[AA][Uni].iLife==0)&&(Squad[AA][Uni].bInBattle)&&((SP->iTarget==2)||(arGrid[Squad[AA][Uni].x][Squad[AA][Uni].y].Pass)))
							{
								Buff=0;
								Damage=0;
								if (SP->arEffect[0][0]>1000)
								{	
									SpellPower=SP->arEffect[1][0];
									if (ActiveUnit->pHero)
									{
										SpellPower+=ActiveUnit->pHero->CheckHeroBonus(907)+ActiveUnit->pHero->CheckHeroBonus(909);
									};
									Buff=((pFirstUnitType+(SP->arEffect[0][0]-1000))->iLevel+1)*10+SpellPower*3-1-DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]+(pFirstUnitType+(SP->arEffect[0][0]-1000))->iLife;
									if (DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]<2)
									{
										Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2;
									}
									else
									{
										Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2-DangerZone[Squad[1-AA][Uni].x][Squad[1-AA][Uni].y]-1+(pFirstUnitType+(SP->arEffect[0][0]-1000))->iSpeed;
										if (Damage<1)
										{Damage=1;};
									};
								};
//								Воскрешение
								if (SP->arEffect[0][0]==1)
								{
									Buff=(Squad[AA][Uni].pUnit->pType()->iLevel)*15+Squad[AA][Uni].pUnit->iLevel*2;
									Damage=Squad[AA][Uni].pUnit->pType()->iAttack;
								};
								int MyTaste=Buff;								
								MyTaste+=Damage*3;
								MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
								if ((MyTaste>SpellTaste)&&((Damage>0)||(Buff>0)))
								{
			//						textprintf(screen,Font1,0,0,COLOR_2,"Заклинание %s. Дамаг: %d, Бафф: %d               ",SP->sName,iGainSpellDamage,MyTaste);
			//						readkey();
									SpellTaste=MyTaste;
									SpellHit=Damage;
									ChoosenSpell=Spell;
									SpellTarget1=AA;
									SpellTarget2=Uni;
								};
							};
							};
						};
					};
					//Вызов заклинания в свободном тайле
					if (SP->iTarget==5)
					{
						for (int i=0; i<8; i++){
						for (int j=0; j<8; j++)
						{
							Field[i][j].Land=0;
							Field[i][j].Pass=0;
							Field[i][j].Shoot=0;
						};
						};					
						ShootMap(ActiveUnit->x,ActiveUnit->y,-1,ActiveUnit->CheckSkill(907)/2+1);
						for (int x=0; x<8; x++){
						for (int y=0; y<8; y++)
						{
							if ((Field[x][y].Shoot>0)&&(arGrid[x][y].Pass)&&((pFirstBF_Obj+arGrid[x][y].Land)->iMoveCost>0))
						{							
								if (SP->arEffect[0][0]>1000)
								{
								Buff=0;
								Damage=0;
								SpellPower=SP->arEffect[1][0];
								if (ActiveUnit->pHero)
								{
									SpellPower+=ActiveUnit->pHero->CheckHeroBonus(907);
								};
								Buff=((pFirstUnitType+(SP->arEffect[0][0]-1000))->iLevel+1)*10+SpellPower*3-DangerZone[x][y]*2+(pFirstUnitType+(SP->arEffect[0][0]-1000))->iLife;
								if (DangerZone[x][y]<2)
								{
									Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2;
								}
								else
								{
									Damage=(pFirstUnitType+(SP->arEffect[0][0]-1000))->iAttack+SpellPower/2-DangerZone[x][y]+(pFirstUnitType-1+(SP->arEffect[0][0]-1000))->iSpeed;
									if (Damage<1)
									{Damage=1;};
								};
								int MyTaste=Buff;								
								MyTaste+=Damage*3;
								MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
								if ((MyTaste>SpellTaste)&&((Damage>0)||(Buff>0)))
								{			
									SpellTaste=MyTaste;
									SpellHit=Damage;
									ChoosenSpell=Spell;
									SpellTarget1=x;
									SpellTarget2=y;							
								};							
							};
						};
						};
						};
					};
					//Вызов заклинания в тайле
					if (SP->iTarget==3)
					{
						int Side;
						if (SP->bNegative)
						{Side=1-AA;}
						else
						{Side=AA;};
						int MyTaste=0, MyDamage=0, MaxTargets=1, Targets=0;
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[Side][k].pUnit)&&(Squad[Side][k].iLife>0))
							{Targets++;};
						};
						if (SP->iRadius==1)
						{MaxTargets=(Targets+2)/3;};
						if (SP->iRadius==2)
						{MaxTargets=(Targets+1)/2;};


						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[Side][k].pUnit)&&(Squad[Side][k].iLife>0))
							{
								MyTaste=0, MyDamage=0;								
								if (SP->bNegative)
								{
									iGainSpellDamage=0;
									MyTaste+=AI_GetEnemySpellTaste(arCast[Spell][0],&Squad[Side][k],k);
									MyDamage+=iGainSpellDamage;
									int Targets=1;
									for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
									{
										if ((i!=k)&&(Targets<MaxTargets)&&(Squad[Side][i].pUnit)&&(Squad[Side][i].iLife>0)&&(abs(Squad[Side][i].x-Squad[Side][k].x)<=SP->iRadius))
										{
											iGainSpellDamage=0;
											MyTaste+=AI_GetEnemySpellTaste(arCast[Spell][0],&Squad[Side][i],i);
											MyDamage+=iGainSpellDamage;
											Targets++;
										};
									};
								}
								else
								{
									iGainSpellDamage=0;
									MyTaste+=AI_GetAllySpellTaste(arCast[Spell][0],&Squad[Side][k],k);												
									MyDamage+=iGainSpellDamage;
									int Targets=1;
									for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
									{
										if ((i!=k)&&(Targets<MaxTargets)&&(Squad[Side][i].pUnit)&&(Squad[Side][i].iLife>0)&&(abs(Squad[Side][i].x-Squad[Side][k].x)<=SP->iRadius))
										{
											iGainSpellDamage=0;
											MyTaste+=AI_GetAllySpellTaste(arCast[Spell][0],&Squad[Side][i],i);
											MyDamage+=iGainSpellDamage;
											Targets++;
										};
									};
								};
								iGainSpellDamage=MyDamage;
								MyTaste+=AI_CalcSpellPenalty(SP,arCast[Spell][0]);
								if ((MyTaste>SpellTaste)&&((iGainSpellDamage>0)||(MyTaste>0)))
								{		
									SpellTaste=MyTaste;
									SpellHit=iGainSpellDamage;
									ChoosenSpell=Spell;
									SpellTarget1=Side;
									SpellTarget2=k;
								};	
							};
						};						
					};
				};
			};
			if (SpellHit==0)
			{SpellHit=SpellTaste/10;};
		};

//Лечение
//*******
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			arCanHeal[i]=0;
		};

		if ((ActiveUnit->CheckSkill(24))&&(ActiveUnit->iAmmo>0))
		{			
		//Поиск целей с места
			for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
			{
				if ((Squad[AA][i].iLife>0)&&(abs(Squad[AA][i].x-ActiveUnit->x)<ActiveUnit->GetShootingRange()+ActiveUnit->iSpeed)
					&&(Squad[AA][i].iLife<Squad[AA][i].GetLife())
					&&(Squad[AA][i].GetUnitKind()!=2)&&(Squad[AA][i].GetUnitKind()!=4))
				{
					arCanHeal[i]=3;
					arHealCoord[i][0]=ActiveUnit->x;
					if (abs(Squad[AA][i].x-arHealCoord[i][0])>ActiveUnit->GetShootingRange())
					{
						int Add;
						if (Squad[AA][i].x<ActiveUnit->x)
						{Add=-1;}
						else
						{Add=1;};
						while (abs(Squad[AA][i].x-arHealCoord[i][0])>ActiveUnit->GetShootingRange())
						{arHealCoord[i][0]+=Add;};
						arCanHeal[i]=1;
					};
				};
			};
		};

//Стрельба
//********
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			arCanRange[i]=0;
		};
		//Поиск целей с места		
		if ((ActiveUnit->iAmmo>0)&&(ActiveUnit->GetRangedAttack()>0))
		{			
			for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
			{		
				if ((Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0)&&(abs(Squad[1-AA][i].x-ActiveUnit->x)<ActiveUnit->GetShootingRange()+ActiveUnit->iSpeed))
				{
					arCanRange[i]=3;
					arRangeCoord[i][0]=ActiveUnit->x;
					if (abs(Squad[1-AA][i].x-arRangeCoord[i][0])>ActiveUnit->GetShootingRange())
					{
						int Add;
						if (Squad[1-AA][i].x<ActiveUnit->x)
						{Add=-1;}
						else
						{Add=1;};
						while (abs(Squad[1-AA][i].x-arRangeCoord[i][0])>ActiveUnit->GetShootingRange())
						{arRangeCoord[i][0]+=Add;};
						arCanRange[i]=1;
					};
				};
			};			
		};
//Рукопашная
//**********		
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
		//Выбор тайла для атаки
			arCanMelee[i]=0;
			if (Squad[1-AA][i].bInBattle)
			{
				if ((Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0)&&(abs(Squad[1-AA][i].x-ActiveUnit->x)<=ActiveUnit->iSpeed+1))
				{
					arCanMelee[i]=3;
					arMeleeCoord[i][0]=ActiveUnit->x;
					if (abs(Squad[1-AA][i].x-arMeleeCoord[i][0])>1)
					{
						int Add;
						if (Squad[1-AA][i].x<ActiveUnit->x)
						{Add=-1;}
						else
						{Add=1;};
						while (abs(Squad[1-AA][i].x-arMeleeCoord[i][0])>1)
						{arMeleeCoord[i][0]+=Add;};
						arCanMelee[i]=1;
					};
				};
			};
		};
		int WillStrike=-1,	MeleeHit=0;
		int WillShoot=-1, ShootHit=0;
		int WillHeal=-1, HealHit=0;
		int Taste=-1000, Taste1;
		bool KillShoot=false, KillMelee=false;		

		//Оценка целей
		//************

		//Лечение
		if ((ActiveUnit->CheckSkill(24))&&(ActiveUnit->iAmmo>0))
		{
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if (arCanHeal[i]>0)
			{	
				int Hit=ActiveUnit->CheckSkill(24);
				if (Squad[AA][i].GetLife()-Squad[AA][i].iLife<Hit)
				{Hit=Squad[AA][i].GetLife()-Squad[AA][i].iLife;};
				Taste1=Hit*5+Squad[AA][i].GetDefence()+Squad[AA][i].GetRangedDefence()+Squad[AA][i].GetResist()+(arCanHeal[i]-1)*2;
				if (Squad[AA][i].pUnit->pHero)
				{Taste1+=Hit*(Squad[AA][i].pUnit->pHero->ipClass1/2);};
				if (arCanHeal[i]==1)
				{Taste1/=2;};
				if (Taste1>Taste)
				{
					Taste=Taste1;					
					WillHeal=i;
					HealHit=Hit;
				};				
			};
		};
		};
		
		//Выстрел
		if ((ActiveUnit->GetRangedAttack()>0)&&(ActiveUnit->iAmmo>0))
		{
		Taste=-1000;
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if (arCanRange[i]>0)
			{	
				int MyTaste=0, MyHit=0;				
				if (ActiveUnit->CheckSkill(47)>0)
				{
					MyTaste=Squad[1-AA][i].iLife;
					MyHit=Squad[1-AA][i].iLife/4;
					if (Squad[1-AA][i].pUnit->pHero)
					{
						MyTaste*=2;
						MyHit*=2;
					};
					if (ActiveUnit->CheckSkill(47)>Squad[1-AA][i].GetAttack())
						{MyTaste*=2;};
					if (Squad[1-AA][i].CheckSkill(36)>0)
						{
							MyTaste/=3;
							MyHit/=3;
						};
					if (Squad[1-AA][i].CheckSkill(201)>0)
					{
						MyTaste=-1000;
						MyHit=0;
					}
				}
				else					
				{
					if ((ActiveUnit->CheckSkill(46)>0)&&(!Squad[1-AA][i].CheckSkill(36)))
					{					
						MyTaste=(ActiveUnit->CheckSkill(46)-Squad[1-AA][i].GetResist()-1)*
							(Squad[1-AA][i].iLife+Squad[1-AA][i].GetAttack());
						MyHit=(ActiveUnit->CheckSkill(46)-Squad[1-AA][i].GetResist()-1)*
							(Squad[1-AA][i].iLife/4+Squad[1-AA][i].GetAttack());
						if (Squad[1-AA][i].pUnit->pHero)
						{
							MyTaste*=2;
							MyHit*=2;
						};
						if ((ActiveUnit->CheckSkill(46)<=Squad[1-AA][i].GetResist())||(Squad[1-AA][i].CheckSkill(202)>0))
						{
							MyTaste=-2000;
							MyHit=-3;
						};
					}
					else
					{
						MyHit=AI_CalcRangedHit(ActiveUnit,&Squad[1-AA][i]);
						if ((MyHit<Squad[1-AA][i].iLife)&&(arCanRange[i]==3)&&(ActiveUnit->CheckSkill(20)>0)&&(ActiveUnit->iStamina>=ActiveUnit->CheckSkill(20)))
						{
							MyHit*=2;
							arCanRange[i]=4;
						};
						MyTaste=MyHit*2-abs(MyHit-Squad[1-AA][i].iLife-1);
						if (MyHit<Squad[1-AA][i].iLife)
						{MyTaste+=ActiveUnit->CheckSkill(78)*2;};
						int Adder=(Squad[1-AA][i].GetAttack()+Squad[1-AA][i].GetCounterAttack())/5;
						Adder+=Squad[1-AA][i].GetRangedAttack()*Squad[1-AA][i].iAmmo/5;						
						Adder-=Squad[1-AA][i].bSummoned*3;
						if (Squad[1-AA][i].pUnit->pHero)
						{
							Adder+=MyHit*(Squad[1-AA][i].pUnit->pHero->ipClass1/2);
						};
						if ((MyHit>=ShootHit/2)||(MyHit>=Squad[1-AA][i].iLife))
						{
							MyTaste+=Adder;
						};
						if ((Adder>0)&&(MyHit>=Squad[1-AA][i].iLife))
						{
							MyTaste+=Adder;
						};
						if (arCanRange[i]==1)
						{MyTaste/=2;};
						if ((MyTaste>0)&&(Squad[1-AA][i].CheckSkill(73)))
						{
							MyTaste/=10;
						};
						if ((MyTaste>0)&&(Squad[1-AA][i].CheckSkill(70)))
						{
							MyTaste/=2;
						};
					};
				};		
				if ((MyHit>0)&&(MyTaste>Taste))
				{
					Taste=MyTaste;
					WillShoot=i;
					ShootHit=MyHit;
					if (MyHit>=Squad[1-AA][i].iLife)
					{KillShoot=true;}
					else
					{KillShoot=false;};
				};				
			};
		};
		};
		if (!ActiveUnit->CheckSkill(38))
		{

		//Рукопашная
		Taste=-1000;
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{ 
			if (arCanMelee[i]>0)
			{
				int Hit=AI_CalcMeleeHit(ActiveUnit,&Squad[1-AA][i]);
				if ((Hit<Squad[1-AA][i].iLife)&&(ActiveUnit->CheckSkill(59))&&(ActiveUnit->iStamina>ActiveUnit->CheckSkill(59))&&(arCanMelee[i]>1))
				{
					buttCrush.bPress=true;
					Hit=AI_CalcMeleeHit(ActiveUnit,&Squad[1-AA][i]);
					buttCrush.bPress=false;
					arCanMelee[i]=5;
				};
				int MyTaste=Hit*2-abs(Hit-Squad[1-AA][i].iLife-1);				
				if (Hit<Squad[1-AA][i].iLife)
				{
					MyTaste+=ActiveUnit->CheckSkill(41)*3;
					//Не бить превращённых в драконов
					if ((Squad[1-AA][i].pUnit->GetUnitType()==60)&&(Squad[1-AA][i].pUnit!=Squad[1-AA][i].pTrueForm)&&(Hit<20))
					{MyTaste=-2000;};
				};				
				if ((Hit<Squad[1-AA][i].iLife)&&(ActiveUnit->CheckSkill(63)>=Squad[1-AA][i].iLife-Hit))
				{
					Hit=Squad[1-AA][i].iLife;
					MyTaste=Hit*2;
				};
				//Круговая атака
				if ((ActiveUnit->CheckSkill(65)>0)||((ActiveUnit->CheckSkill(66)>0)&&(ActiveUnit->iStamina>ActiveUnit->CheckSkill(66))))
				{					
					int OldX=ActiveUnit->x;
					int OldY=ActiveUnit->y;
					ActiveUnit->x=arMeleeCoord[i][0];
					ActiveUnit->y=arMeleeCoord[i][1];
					int Neig=ActiveUnit->FindNeighbour(1-ActiveUnit->iSide);
					if (Neig>1)
					{
						if ((ActiveUnit->CheckSkill(66)>0)&&(ActiveUnit->iStamina>ActiveUnit->CheckSkill(66)))
						{
							arCanMelee[i]+=100;
						};
						for (int j=0; j<Neig; j++)
						{
							if (arNeigh[j]!=&Squad[1-AA][i])
							{
								Hit+=AI_CalcMeleeHit(ActiveUnit,arNeigh[j]);
								MyTaste+=AI_CalcMeleeHit(ActiveUnit,arNeigh[j])*2;
							};
						};
					};					
					ActiveUnit->x=OldX; ActiveUnit->y=OldY;				
				};
				int Adder=Squad[1-AA][i].CheckSkill(36)*5;
				Adder+=Squad[1-AA][i].GetAttack()/2;				
				Adder-=Squad[1-AA][i].bSummoned*3;
				if ((Hit<=Squad[1-AA][i].iLife)&&(ActiveUnit->CheckSkill(26)==0)&&(Squad[1-AA][i].iStamina>0)&&(!Squad[1-AA][i].bResting))
				{
					Adder-=Squad[1-AA][i].GetCounterAttack()/2;
				};
				Adder+=Squad[1-AA][i].GetRangedAttack()*Squad[1-AA][i].iAmmo/5;
				if (Squad[1-AA][i].pUnit->pHero)
				{
					Adder+=Hit*(Squad[1-AA][i].pUnit->pHero->ipClass1/2);
				};
				if ((Hit>=MeleeHit/2)||(Hit>=Squad[1-AA][i].iLife))
				{
					MyTaste+=Adder;
				};
				if ((Adder>0)&&(Hit>=Squad[1-AA][i].iLife))
				{
					MyTaste+=Adder;
				};
				if ((MyTaste>0)&&(Squad[1-AA][i].CheckSkill(73)))
				{
					MyTaste/=10;
				};
				if ((MyTaste>0)&&(Squad[1-AA][i].CheckSkill(70)))
				{
					MyTaste/=2;
				};
				if ((arCanMelee[i]==1)&&(MyTaste>0))
				{MyTaste/=2;};
				if ((Hit>0)&&(MyTaste>Taste))
				{
					Taste=MyTaste;
					WillStrike=i;
					MeleeHit=Hit;
					if (Hit>=Squad[1-AA][i].iLife)
					{KillMelee=true;}
					else
					{KillMelee=false;};
				};			
			};
		};
		};

		//Выбор действия
		//**************

		//Заклинание
		if ((ChoosenSpell>=0)&&(SpellHit>=ShootHit)&&(SpellHit>=MeleeHit)&&(SpellHit>=HealHit))
		{
			WriteLog("Заклинание");
			IndexAI=-1;
			clSpell *SP=(pFirstSpell+arCast[ChoosenSpell][0]);
			if (bShowAnim)
			{
				PlaySpellSound(SP->wavCast);
			//	play_sample(SP->wavCast,iSoundVolume,128,1000,0);
			};
			switch (SP->iTarget)
			{
			case 1:
				if (SP->arEffect[0][0]==89)
				{
					//Телепорт
					int BigHit=0, spX;
					for (int x=0; x<8; x++)
					{
						int Hit=GetTeleportTaste(&Squad[SpellTarget1][SpellTarget2],x);
						if (Hit>BigHit)
						{
							BigHit=Hit;
							spX=x;
						};
					};
					Squad[SpellTarget1][SpellTarget2].x=spX;					
				}
				else
				{
					Squad[SpellTarget1][SpellTarget2].UnderSpell(SP,ActiveUnit,AA);
				};
				break;
			case 2:
			case 4:
				CorpseSpell(SP,&Squad[SpellTarget1][SpellTarget2]);
				break;
			case 5:
				TileSpell(SP,SpellTarget1,SpellTarget2);
				break;
			case 3:
				{					
					int MaxTargets=1, Targets=0;
					for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
					{
						if ((Squad[SpellTarget1][k].pUnit)&&(Squad[SpellTarget1][k].iLife>0))
						{Targets++;};
					};
					if (SP->iRadius==1)
					{MaxTargets=(Targets+2)/3;};
					if (SP->iRadius==2)
					{MaxTargets=(Targets+1)/2;};

					Squad[SpellTarget1][SpellTarget2].UnderSpell(SP,ActiveUnit,AA);
					Targets=1;
					for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
					{
						if ((i!=SpellTarget2)&&(Targets<MaxTargets)&&(Squad[SpellTarget1][i].pUnit)&&(Squad[SpellTarget1][i].iLife>0)&&(abs(Squad[SpellTarget1][i].x-Squad[SpellTarget1][SpellTarget2].x)<=SP->iRadius))
						{
							Squad[SpellTarget1][i].UnderSpell(SP,ActiveUnit,AA);
							Targets++;
						};
					};
				};
				break;
			case 0:	
				if (SP->iArea==2)
				{
					if (SP->bOnAlly)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[AA][k].pUnit)&&(Squad[AA][k].iLife>0))
							{
								int AntiSkill=0;
								for (int i=0; i<MAX_ANTI_EFFECT; i++)
								{
									AntiSkill+=SP->arAntiEffect[i]*Squad[AA][k].CheckSkill(SP->arAntiEffect[i]);
								};
								for (int i=0; i<MAX_UNIT_KIND; i++)
								{
									if (SP->arUnitKind[i]==Squad[AA][k].GetUnitKind())
									{AntiSkill=1;};
								};
								if ((!AntiSkill)||(Squad[AA][k].CheckSkill(205)==arCast[ChoosenSpell][0]))
								{													
							//Найден свой подходящий юнит
									Squad[AA][k].UnderSpell(SP,ActiveUnit,AA);												
								};
							};
						};
					};
					if (SP->bOnEnemy)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[1-AA][k].pUnit)&&(Squad[1-AA][k].iLife>0))
							{
								int AntiSkill=0;
								for (int i=0; i<MAX_ANTI_EFFECT; i++)
								{
									AntiSkill+=SP->arAntiEffect[i]*Squad[1-AA][k].CheckSkill(SP->arAntiEffect[i]);
								};
								for (int i=0; i<MAX_UNIT_KIND; i++)
								{
									if (SP->arUnitKind[i]==Squad[1-AA][k].GetUnitKind())
									{AntiSkill=1;};
								};
								if ((!AntiSkill)||(Squad[1-AA][k].CheckSkill(205)==arCast[ChoosenSpell][0]))
								{
						//Найден вражеский подходящий юнит
									Squad[1-AA][k].UnderSpell(SP,ActiveUnit,AA);
								};
							};
						};
					};
				};
				if (SP->iArea==0)
				{ActiveUnit->UnderSpell(SP,ActiveUnit,AA);}
				break;
			};
			if ((pFirstSpell+arCast[ChoosenSpell][0])->bSacrifice)
			{
				int Vic1, Vic2;
				iAISacVictim=-1;
				for (int i=0; i<MAX_SQUAD; i++)
				{
					if ((Squad[AA][i].pUnit)&&(Squad[AA][i].iLife>0)&&(Squad[AA][i].GetUnitKind()!=2)&&(Squad[AA][i].GetUnitKind()!=4)&&(Squad[AA][i].GetUnitKind()!=3))
					{
						if (iAISacVictim<0)
						{
							iAISacVictim=i;
							Vic2=Squad[AA][i].iLife*Squad[AA][i].pUnit->pType()->iLevel+Squad[AA][i].pUnit->iLevel*2;
							if (Squad[AA][i].pHero)
							{Vic2+=30;};
						}
						else
						{
							Vic1=Squad[AA][i].iLife*Squad[AA][i].pUnit->pType()->iLevel+Squad[AA][i].pUnit->iLevel*2;
							if (Squad[AA][i].pHero)
							{Vic1+=30;};
							if (Vic1<Vic2)
							{
								Vic2=Vic1;
								iAISacVictim=i;
							};
						};
					};
				};
			};
			SpellCastAI((pFirstSpell+arCast[ChoosenSpell][0]));
			return;
		};

		//Лечение
		if ((WillHeal>=0)&&(HealHit>=ShootHit)&&(HealHit>=MeleeHit))
		{
			IndexAI=-1;
			ActiveUnit->x=arHealCoord[WillHeal][0];
			ActiveUnit->Heal(&Squad[AA][WillHeal]);
			return;
		};
		

		//Стрельба
		if ((WillShoot>=0)&&((ShootHit>=MeleeHit)||(KillShoot))&&((!KillMelee)||(KillShoot)))
		{
			IndexAI=-1;			
			if (arCanRange[WillShoot]==4)
			{buttDoubleShot.bPress=true;};
			ActiveUnit->x=arRangeCoord[WillShoot][0];
			ActiveUnit->Shoot(&Squad[1-AA][WillShoot]);
			buttDoubleShot.bPress=false;
			return;		
		};
		
		//Рукопашная
		if (WillStrike>=0)
		{
			IndexAI=-1;
			if (arCanMelee[WillStrike]%100==5)
			{
				buttCrush.bPress=true;
			};
			if (arCanMelee[WillStrike]>99)
			{
				buttRoundAtt.bPress=true;		
			};
			ActiveUnit->x=arMeleeCoord[WillStrike][0];
			ActiveUnit->Attack(ActiveUnit->x,ActiveUnit->y,&Squad[1-AA][WillStrike]);
			buttCrush.bPress=false;
			buttRoundAtt.bPress=false;
			return;
		};
			
		//Цель не найдена
		//***************
			
		//Сбор снарядов
		if ((ActiveUnit->CheckSkill(23))&&(ActiveUnit->iAmmo==0))
		{
			ActiveUnit->iSpeed=0;
			ActiveUnit->bCanAct=false;
			int AddAmmo=ActiveUnit->CheckSkill(23)-1;
			ActiveUnit->iAmmo+=AddAmmo;			
			return;
		};
			
		//Медитация
		if ((ActiveUnit->CheckSkill(22))&&(ActiveUnit->iAmmo==0))
		{
			ActiveUnit->iSpeed=0;
			ActiveUnit->bCanAct=false;
			ActiveUnit->bResting=true;
			return;
		};

		//Отдых
		if ((ActiveUnit->iStamina<=ActiveUnit->GetStamina()/2)&&(ActiveUnit->iSpeed==ActiveUnit->GetSpeed())&&(arUnitPower[AA][ActiveNum][3]==0)&&(arUnitPower[AA][ActiveNum][4]==0))
		{			
			ActiveUnit->iSpeed=0;
			ActiveUnit->bCanAct=false;
			ActiveUnit->bResting=true;
		//	ActiveUnit->DrawBar();
			return;
		};

		//Первая помощь
		if ((ActiveUnit->iLife<ActiveUnit->GetLife())&&(ActiveUnit->iSpeed==ActiveUnit->GetSpeed())&&(ActiveUnit->CheckSkill(35))&&(arUnitPower[AA][ActiveNum][3]==0)&&(arUnitPower[AA][ActiveNum][4]==0))
		{
			ActiveUnit->iSpeed=0;
			ActiveUnit->bCanAct=false;
			ActiveUnit->bResting=true;			
			return;
		};

		//Трупоедство
		if ((ActiveUnit->CheckSkill(49))&&(ActiveUnit->iLife<=ActiveUnit->GetLife()/2))
		{
			int Corpse=-1, YY=0;
			while ((Corpse<0)&&(YY<8))
			{
				Corpse=FindCorpse(1-AA,ActiveUnit->x,YY);
				YY++;
			};
			int CSq=1-AA;
			if (Corpse<0)
			{
				YY=0;
				while ((Corpse<0)&&(YY<8))
				{
					Corpse=FindCorpse(AA,ActiveUnit->x,YY);
					YY++;
				};				
				CSq=AA;
			};
			if (Corpse>=0)
			{					
				Squad[CSq][Corpse].bInBattle=false;
				Squad[CSq][Corpse].x=10;
				Squad[CSq][Corpse].y=10;
			//	Squad[CSq][Corpse].G_Obj->iLayer=-1;
				Corpse=ActiveUnit->CheckSkill(49)/2+ActiveUnit->CheckSkill(49)/4;
				if (Corpse>ActiveUnit->GetLife()-ActiveUnit->iLife)
				{
					Corpse=ActiveUnit->GetLife()-ActiveUnit->iLife;
				};
				ActiveUnit->iLife+=Corpse;
				ActiveUnit->iSpeed=0;
				ActiveUnit->bCanAct=false;
			//	ActiveUnit->DrawBar();
				return;
			};
		};

		int Add=0, Dist=10;
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if ((Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0)&&(abs(Squad[1-AA][i].x-ActiveUnit->x)<Dist))
			{
				Dist=abs(Squad[1-AA][i].x-ActiveUnit->x);
				if (Squad[1-AA][i].x>ActiveUnit->x)
				{Add=ActiveUnit->iSpeed;}
				else
				{Add=-ActiveUnit->iSpeed;};
			};
		};
		if (Add!=0)
		{
			if (abs(Add)>Dist)
			{ActiveUnit->x+=Dist*sign(Add);}
			else
			{ActiveUnit->x+=Add;};
			if (ActiveUnit->x<0)
			{ActiveUnit->x=0;};
			if (ActiveUnit->x>7)
			{ActiveUnit->x=7;};
		};
	};
};
