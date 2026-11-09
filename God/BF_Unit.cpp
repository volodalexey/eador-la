#include "Battlefield.h"
//#include <string.h>

int MinWeight;		//Минимальная длина пути (в единицах скорости)
int MinStamina;		//Необходимая стамина для найденного пути
BITMAP *bmpPathPic;	//Картинка движущегося юнита
int CanPass[3];
bool bGoBerz;		//Флаг берсерка
dcHero * Heroes[2];	//Участвующие в бою герои
clPath *TruePath;	//Указатель на найденный путь
BF_Unit *arNeigh [6];
int iMoves;

//Преобразование координат юнита в координаты графического объекта

int TransX(int x, int y, int w)
{
	if (w>64)
	{
		if (y%2)
		{return 70+x*104-(w-64)/2;}
		else
		{return 18+x*104-(w-64)/2;};
	}
	else
	{
		if (y%2)
		{return 70+x*104;}
		else
		{return 18+x*104;};
	};
};

int TransY(int x, int y, int h)
{
	if (h>80)
	{return 15+y*90-(h-80);}
	else
	{return 15+y*90;};
};

void SimulateMap(int x, int y, int Weight, int Speed, int SpeedLeft, bool Flight)
{
	if (Weight<0)
	{
		Field[x][y].Land=Weight;
		Weight=1;
	}
	else
	{
		if ((x<0)||(y<0)||(x>7)||(y>7))
		{return;};
		if ((!Flight)&&((pFirstBF_Obj+arGrid[x][y].Land)->iMoveCost==0))
		{return;};
		if (Flight)
		{iMoves=1;}
		else
		{iMoves=GetMoveCost(x,y);};
		if (SpeedLeft<iMoves)
		{
			if (Speed>SpeedLeft)
			{
				Weight++;
				SpeedLeft=Speed;
			};
		};
		if ((Weight>Field[x][y].Land)&&(Field[x][y].Land!=0))
		{
			return;
		};
		SpeedLeft-=iMoves;
		Field[x][y].Land=Weight;
		Field[x][y].Pass=1;
		if (SpeedLeft<=0)
		{
			Weight++;
			SpeedLeft=Speed;
		};
	};
	SimulateMap(x-1+y%2,y-1,Weight,Speed,SpeedLeft,Flight);	
	SimulateMap(x+y%2,y-1,Weight,Speed,SpeedLeft,Flight);
	SimulateMap(x-1+y%2,y+1,Weight,Speed,SpeedLeft,Flight);
	SimulateMap(x+y%2,y+1,Weight,Speed,SpeedLeft,Flight);
	SimulateMap(x-1,y,Weight,Speed,SpeedLeft,Flight);
	SimulateMap(x+1,y,Weight,Speed,SpeedLeft,Flight);
};


void SimulatePath (int x, int y, int Weight, clPath *Path, int x1, int y1, int Speed, int SpeedLeft, bool Flight)
{
	if (Weight<0)
	{
		Weight=1;
	}
	else
	{
		if ((x<0)||(y<0)||(x>7)||(y>7))
		{return;};
		if (Field[x][y].Pass<1)
		{return;};
		if (Flight)
		{iMoves=1;}
		else
		{iMoves=GetMoveCost(x,y);};
		if (SpeedLeft<iMoves)
		{
			if (Speed>SpeedLeft)
			{
				Weight++;
				SpeedLeft=Speed;
			};
		};
		SpeedLeft-=iMoves;		
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
		};
		delete pRoad;
		return;
	};
	if (SpeedLeft<=0)
	{
		Weight++;
		SpeedLeft=Speed;
		if (Weight>MinWeight)
		{
			delete pRoad;
			return;
		};
	};
	SimulatePath(x-1+y%2,y-1,Weight,pRoad,x1,y1,Speed,SpeedLeft,Flight);
	SimulatePath(x+y%2,y-1,Weight,pRoad,x1,y1,Speed,SpeedLeft,Flight);
	SimulatePath(x-1+y%2,y+1,Weight,pRoad,x1,y1,Speed,SpeedLeft,Flight);
	SimulatePath(x-1,y,Weight,pRoad,x1,y1,Speed,SpeedLeft,Flight);
	SimulatePath(x+1,y,Weight,pRoad,x1,y1,Speed,SpeedLeft,Flight);	
	delete pRoad;
};


//**************************************************
//***  Поиск пути **********************************
//**************************************************

int GetMoveCost(int x, int y)
{
	int MoveCost=(pFirstBF_Obj+arGrid[x][y].Land)->iMoveCost;			
	if ((CanPass[0]==arGrid[x][y].Land)||(CanPass[1]==arGrid[x][y].Land)||(CanPass[2]==arGrid[x][y].Land))
	{MoveCost=1;};
	return MoveCost;
};

int GetStamCost(int x, int y)
{
	int StamCost=(pFirstBF_Obj+arGrid[x][y].Land)->iStamCost;
	if ((CanPass[0]==arGrid[x][y].Land)||(CanPass[1]==arGrid[x][y].Land)||(CanPass[2]==arGrid[x][y].Land))
	{StamCost=0;};
	return StamCost;
};


//Проверка соседней клетки для создания карты

bool TryXYmap(int x, int y, int Weight, int Stam, int Speed)
{	
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if (arGrid[x][y].Pass)
		{
			if (((Field[x][y].Land==0)&&
			((GetMoveCost(x,y)<=Speed)||((Weight==0)&&(bFirstMove))))||
			((Weight+GetMoveCost(x,y))<Field[x][y].Land))
			{
				if (((pFirstBF_Obj+arGrid[x][y].Land)->iStamCost<=Stam)&&
					((pFirstBF_Obj+arGrid[x][y].Land)->iMoveCost))
				{
					return 1;
				};
			};
		};
	};
	return 0;
};

//Рекурсивная функция для создания карты поиска пути

int StepMap (int x, int y, int Weight, int Speed, int StamCost, int StamLeft)
{
	if (bSimulate)
	{
		SimulateMap(x,y,Weight,Speed,Speed,false);
		return 0;
	};
	if (Weight<0)
	{
		Weight=0;
	}
	else
	{
		Weight+=GetMoveCost(x,y);
		Speed-=GetMoveCost(x,y);
		StamCost+=GetStamCost(x,y);
		Field[x][y].Land=Weight;
		Field[x][y].Pass=1;
		if (Speed<1)
		{
			return 0;
		};

	};

	if (TryXYmap(x-1+y%2,y-1,Weight,StamLeft-StamCost,Speed))
	{
		StepMap(x-1+y%2,y-1,Weight,Speed,StamCost,StamLeft);
	};
	if (TryXYmap(x+y%2,y-1,Weight,StamLeft-StamCost,Speed))
	{
		StepMap(x+y%2,y-1,Weight,Speed,StamCost,StamLeft);
	};
	if (TryXYmap(x-1+y%2,y+1,Weight,StamLeft-StamCost,Speed))
	{
		StepMap(x-1+y%2,y+1,Weight,Speed,StamCost,StamLeft);
	};
	if (TryXYmap(x+y%2,y+1,Weight,StamLeft-StamCost,Speed))
	{
		StepMap(x+y%2,y+1,Weight,Speed,StamCost,StamLeft);
	};
	if (TryXYmap(x-1,y,Weight,StamLeft-StamCost,Speed))
	{
		StepMap(x-1,y,Weight,Speed,StamCost,StamLeft);
	};
	if (TryXYmap(x+1,y,Weight,StamLeft-StamCost,Speed))
	{
		StepMap(x+1,y,Weight,Speed,StamCost,StamLeft);
	};	
	return 0;
};

//Проверка соседней клетки для поиска пути

bool TryXYpath(int x, int y, int Weight, int Stam)
{
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if ((arGrid[x][y].Pass)&&((Field[x][y].Land==0)||
			((Weight+GetMoveCost(x,y))<Field[x][y].Land)))
		{
			if ((GetStamCost(x,y)<=Stam)&&
				((pFirstBF_Obj+arGrid[x][y].Land)->iMoveCost)&&
				(Field[x][y].Pass))
			{
				return 1;
			};
		};
	};
	return 0;
};

//Рекурсивная функция для поиска пути

int StepPath (int x, int y, int Weight, clPath *Path, int x1, int y1, int StamCost, int StamLeft)
{
	if (bSimulate)
	{
	//	SimulatePath(x,y,Weight,Path,x1,y1,10,10,false);
		return 0;
	};
 	Weight+=GetMoveCost(x,y);
	if (Weight>MinWeight)
	{
		return 0;
	};
	StamCost+=GetStamCost(x,y);
	Field[x][y].Land=Weight;	
	clPath *pRoad=new clPath(x, y, Path);
	if ((x==x1)&&(y==y1))
	{
		if (((TruePath)&&(Weight<MinWeight))||(!TruePath)) //Потом вес заменить на стамину
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
			P1=new clPath(TransX(x, y, bmpPathPic->w), TransY(x, y, bmpPathPic->h) ,NULL);
			P2=pRoad->pPrev; 			
			while (P2)
			{
				P3=new clPath(TransX(P2->x, P2->y, bmpPathPic->w), TransY(P2->x, P2->y, bmpPathPic->h), P1);
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
	if (TryXYpath(x-1+y%2,y-1,Weight,StamLeft-StamCost))
	{
		StepPath(x-1+y%2,y-1,Weight,pRoad,x1,y1,StamCost,StamLeft);
	};
	if (TryXYpath(x+y%2,y-1,Weight,StamLeft-StamCost))
	{
		StepPath(x+y%2,y-1,Weight,pRoad,x1,y1,StamCost,StamLeft);
	};
	if (TryXYpath(x-1+y%2,y+1,Weight,StamLeft-StamCost))
	{
		StepPath(x-1+y%2,y+1,Weight,pRoad,x1,y1,StamCost,StamLeft);
	};
	if (TryXYpath(x+y%2,y+1,Weight,StamLeft-StamCost))
	{
		StepPath(x+y%2,y+1,Weight,pRoad,x1,y1,StamCost,StamLeft);
	};
	if (TryXYpath(x-1,y,Weight,StamLeft-StamCost))
	{
		StepPath(x-1,y,Weight,pRoad,x1,y1,StamCost,StamLeft);
	};
	if (TryXYpath(x+1,y,Weight,StamLeft-StamCost))
	{
		StepPath(x+1,y,Weight,pRoad,x1,y1,StamCost,StamLeft);
	};
	delete pRoad;
	return 0;
};



//Проверка соседней клетки для создания карты (для летающих юнитов)

bool FlyXYmap(int x, int y, int Weight, bool Over)
{
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if (((pFirstBF_Obj+arGrid[x][y].Land)->iMoveCost)||Over)
		{
			if ((Field[x][y].Land==0)||((Weight+1)<Field[x][y].Land))
			{
				return 1;			
			};
		};
	};
	return 0;
};

//Рекурсивная функция для создания карты поиска пути для летающих юнитов

int FlyMap (int x, int y, int Weight, int Speed, bool Over)
{
	if (bSimulate)
	{
		SimulateMap(x,y,Weight,Speed,Speed,true);
		return 0;
	};
	if (Weight<0)
	{
		Weight=0;
	}
	else
	{
		Weight+=1;
		Speed-=1;
//		Found++;		
		Field[x][y].Land=Weight;
		Field[x][y].Pass=1;
		if (Speed<1)
		{
			return 0;
		};
	};

	if (FlyXYmap(x-1+y%2,y-1,Weight,Over))
	{
		FlyMap(x-1+y%2,y-1,Weight,Speed, Over);
	};
	if (FlyXYmap(x+y%2,y-1,Weight, Over))
	{
		FlyMap(x+y%2,y-1,Weight,Speed, Over);
	};
	if (FlyXYmap(x-1+y%2,y+1,Weight, Over))
	{
		FlyMap(x-1+y%2,y+1,Weight,Speed, Over);
	};
	if (FlyXYmap(x+y%2,y+1,Weight, Over))
	{
		FlyMap(x+y%2,y+1,Weight,Speed, Over);
	};
	if (FlyXYmap(x-1,y,Weight, Over))
	{
		FlyMap(x-1,y,Weight,Speed, Over);
	};
	if (FlyXYmap(x+1,y,Weight, Over))
	{
		FlyMap(x+1,y,Weight,Speed, Over);
	};	
	return 0;
};

//Проверка соседней клетки для поиска пути (для летающих юнитов)

bool FlyXYpath(int x, int y, int Weight)
{
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if ((Field[x][y].Pass)&&(
			(Field[x][y].Land==0)||((Weight+1)<Field[x][y].Land)))
		{
			return 1;		
		};
	};
	return 0;
};

//Поиск пути для летающих юнитов

int FlyPath (int x, int y, int Weight, clPath *Path, int x1, int y1)
{
	if (bSimulate)
	{
	//	SimulateMap(x,y,Weight,Speed,Speed,false);
		return 0;
	};
 	Weight++;
	if (Weight>MinWeight)
	{
		return 0;
	};	
	Field[x][y].Land=Weight;	
	clPath *pRoad=new clPath(x, y, Path);
	if ((x==x1)&&(y==y1))
	{
		if (((TruePath)&&(Weight<MinWeight))||(!TruePath))
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
			P1=new clPath(TransX(x, y, bmpPathPic->w), TransY(x, y, bmpPathPic->h) ,NULL);
			P2=pRoad->pPrev; 			
			while (P2)
			{
				P3=new clPath(TransX(P2->x, P2->y, bmpPathPic->w), TransY(P2->x, P2->y, bmpPathPic->h), P1);
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
	if (FlyXYpath(x-1+y%2,y-1,Weight))
	{
		FlyPath(x-1+y%2,y-1,Weight,pRoad,x1,y1);
	};
	if (FlyXYpath(x+y%2,y-1,Weight))
	{
		FlyPath(x+y%2,y-1,Weight,pRoad,x1,y1);
	};
	if (FlyXYpath(x-1+y%2,y+1,Weight))
	{
		FlyPath(x-1+y%2,y+1,Weight,pRoad,x1,y1);
	};
	if (FlyXYpath(x+y%2,y+1,Weight))
	{
		FlyPath(x+y%2,y+1,Weight,pRoad,x1,y1);
	};
	if (FlyXYpath(x-1,y,Weight))
	{
		FlyPath(x-1,y,Weight,pRoad,x1,y1);
	};
	if (FlyXYpath(x+1,y,Weight))
	{
		FlyPath(x+1,y,Weight,pRoad,x1,y1);
	};
	delete pRoad;
	return 0;
};

//Проверка соседней клетки для создания карты дальности выстрела

bool ShootXYmap(int x, int y, int Weight)
{
	if ((x>=0)&&(y>=0)&&(x<8)&&(y<8))
	{
		if ((Field[x][y].Shoot<1)||((Field[x][y].Shoot>1)&&(Field[x][y].Land>Weight)))
		{
			return 1;		
		};
	};
	return 0;
};

//Рекурсивная функция для создания карты дальности выстрела

int ShootMap (int x, int y, int Weight, int Range)
{
	if (Weight<0)
	{
		Weight=0;
		Field[x][y].Shoot=1;
	}
	else
	{
		Weight++;
		Range--;
		if (Weight==1)
		{Field[x][y].Shoot=1;}
		else
		{Field[x][y].Shoot=2;};
		Field[x][y].Land=Weight;
		if (Range==0)
		{
			return 0;
		};
	};

	if (ShootXYmap(x-1+y%2,y-1, Weight))
	{
		ShootMap(x-1+y%2,y-1,Weight,Range);
	};
	if (ShootXYmap(x+y%2,y-1, Weight))
	{
		ShootMap(x+y%2,y-1,Weight,Range);
	};
	if (ShootXYmap(x-1+y%2,y+1, Weight))
	{
		ShootMap(x-1+y%2,y+1,Weight,Range);
	};
	if (ShootXYmap(x+y%2,y+1, Weight))
	{
		ShootMap(x+y%2,y+1,Weight,Range);
	};
	if (ShootXYmap(x-1,y, Weight))
	{
		ShootMap(x-1,y,Weight,Range);
	};
	if (ShootXYmap(x+1,y, Weight))
	{
		ShootMap(x+1,y,Weight,Range);
	};	
	return 0;
};

int GetHit(int Attack, int Defence)
{
	if (Defence<0)
	{Defence=0;};
	int Add=1;
	int Max=3;
	if (Attack>4)
	{
		Add=Attack/5;		
		Max=Add*2+1;
	};
	if (!bSimulate)	
	{Attack=Attack+Add-Random(Max);};
	if (Attack<1)
	{
		Attack=1;
	};
	if (Defence>=Attack)
	{
		if (bSimulate)
		{
			if (Defence-Attack<5)
			{
				return 1;
			}
			else
			{
				return 0;
			};
		}
		else
		{
			if ((Defence-Attack<10)&&(Random(20+Attack-Defence)>=10))
			{
				return 1;
			}
			else
			{
				return 0;
			};
		};
	}
	else
	{
		return Attack-Defence;
	};
};

int GetMeleeHit(BF_Unit * Attacker, BF_Unit * Defender, bool bAttacking)
{	
	int Att=Attacker->GetAttack();
	int Def=Defender->GetDefence();
	if (!bAttacking)
	{Att=Attacker->GetCounterAttack();};
	//Сокрушающий удар
	if ((bAttacking)&&(buttCrush.bPress))
	{Att+=Att/2;};
	//Сокрушение зла
	if ((Attacker->CheckSkill(61)>0)&&(Defender->pUnit->pType()->iKarma<0))
	{
		Att+=Attacker->CheckSkill(61)*abs(Defender->pUnit->pType()->iKarma);
	};
	if ((Attacker->CheckSkill(200)>0)&&(Defender->pUnit->pType()->iKarma>0))
	{
		Att+=Attacker->CheckSkill(200)*Defender->pUnit->pType()->iKarma;
	};
	//Магический удар
	if (Attacker->CheckSkill(27))
	{
		Def=Defender->GetResist();
	}
	else
	{
		//Бронебойный удар
		if (Attacker->CheckSkill(39))
		{
			Def/=2;
		};
		//Парирование
		if ((bAttacking)&&(Defender->bParry)&&(Defender->iStamina>0)&&(!Defender->bResting)&&(Defender->CheckSkill(36)==0))
		{
			Def+=Defender->CheckSkill(53);
		};
		//Точный удар
		if (bAttacking)
		{Def-=Attacker->CheckSkill(76);};		
	};
	return GetHit(Att,Def);

};

int GetRangedHit(BF_Unit * Attacker, BF_Unit * Defender)
{	
	int Result;
	//Магический выстрел
	if (Attacker->CheckSkill(28))
	{
		return GetHit(Attacker->GetRangedAttack(),Defender->GetResist()-Attacker->CheckSkill(95));
	}
	else
	{
		//Бронебойный выстрел
		if (Attacker->CheckSkill(17))
		{
			Result=GetHit(Attacker->GetRangedAttack(),Defender->GetRangedDefence()/2-Attacker->CheckSkill(77));
		}
		else
		//Обычный выстрел
		{
			Result=GetHit(Attacker->GetRangedAttack(),Defender->GetRangedDefence()-Attacker->CheckSkill(77));
		};
	};
	//Огненные стрелы
	if (Attacker->CheckSkill(60)>Defender->GetResist())
	{
		Result+=Attacker->CheckSkill(60)-Defender->GetResist();
	};
	return Result;
};

int CalcMeleeHit(BF_Unit * Attacker, BF_Unit * Defender, bool bAttacking)
{	
	int Att=Attacker->GetAttack();
	int Def=Defender->GetDefence();
	if (!bAttacking)
	{Att=Attacker->GetCounterAttack();};
	//Сокрушающий удар
	if ((bAttacking)&&(Attacker->CheckSkill(59)>0)&&(Attacker->iStamina>Attacker->CheckSkill(59)))
	{Att+=Att/2;};
	//Сокрушение зла
	if ((Attacker->CheckSkill(61)>0)&&(Defender->pUnit->GetKarma()<0))
	{
		Att+=Attacker->CheckSkill(61)*abs(Defender->pUnit->GetKarma());
	};
	if ((Attacker->CheckSkill(200)>0)&&(Defender->pUnit->GetKarma()>0))
	{
		Att+=Attacker->CheckSkill(200)*Defender->pUnit->GetKarma();
	};
	//Магический удар
	if (Attacker->CheckSkill(27))
	{
		Def=Defender->GetResist();
	}
	else
	{
		//Бронебойный удар
		if (Attacker->CheckSkill(39))
		{
			Def/=2;
		};
		//Парирование
		if ((bAttacking)&&(Defender->bParry)&&(Defender->iStamina>0)&&(!Defender->bResting))
		{
			Def+=Defender->CheckSkill(53);
		};
		//Точный удар
		if (bAttacking)
		{Def-=Attacker->CheckSkill(76);};		
	};
	if (Def<0)
	{Def=0;};
	int Result=Att-Def;
	if (Result<0)
	{Result=0;};	
	return Result;

};

int CalcRangedHit(BF_Unit * Attacker, BF_Unit * Defender)
{	
	int Result;
	//Магический выстрел
	if (Attacker->CheckSkill(28))
	{
		int Def=Defender->GetResist()-Attacker->CheckSkill(95);
		if (Def<0){Def=0;}
		Result=Attacker->GetRangedAttack()-Def;
	}
	else
	{
		int Def;
		//Бронебойный выстрел
		if (Attacker->CheckSkill(17))
		{
			Def=Defender->GetRangedDefence()/2-Attacker->CheckSkill(77);
		}
		else
		//Обычный выстрел
		{
			Def=Defender->GetRangedDefence()-Attacker->CheckSkill(77);
		};
		if (Def<0)
		{Def=0;};
		Result=Attacker->GetRangedAttack()-Def;
		//Огненные стрелы
		if (Attacker->CheckSkill(60)>Defender->GetResist())
		{
			Result+=Attacker->CheckSkill(60)-Defender->GetResist();
		};
	};	
	return Result;
};

int Vampire;

void HitEffect(BF_Unit * Attacker, BF_Unit * Defender, int Hit, bool bAtt, bool bCanRound)
{
	if (bCanRound)
	{Vampire=0;};
	BF_Effect * Eff;
	//Ядовитый удар
	if ((Attacker->CheckSkill(41))&&(!Defender->CheckSkill(42)))
	{
		Defender->pEffect=new BF_Effect(Defender->pEffect,1,-Attacker->CheckSkill(41),POISON_DURATION);
		Defender->pEffect->bVisible=true;
		Defender->pEffect->pAbility=(pFirstUnitUpg+71);
	};

	//Ядовитая плоть
	if ((!Attacker->CheckSkill(42))&&(Defender->CheckSkill(96)))
	{
		Attacker->pEffect=new BF_Effect(Attacker->pEffect,1,-Defender->CheckSkill(96),POISON_DURATION);
		Attacker->pEffect->bVisible=true;
		Attacker->pEffect->pAbility=(pFirstUnitUpg+71);
	};

	//Оглушающий удар
	if (Attacker->CheckSkill(40))
	{
		Defender->UseStamina(Attacker->CheckSkill(40));
	};

	//Устрашение
	if (Attacker->CheckSkill(50))
	{
		Defender->ChangeMorale(-Attacker->CheckSkill(50));		
	};

	if (Attacker!=Defender)
	{

	//Кража снарядов
	if ((Attacker->CheckSkill(44))&&(Defender->iAmmo>0))
	{
		if (Attacker->CheckSkill(44)>Defender->iAmmo)
		{
			Defender->iAmmo=0;
		}
		else
		{
			Defender->iAmmo-=Attacker->CheckSkill(44);
		};
//		if (Defender->G_Obj->pStars)
//		{
//			delete Defender->G_Obj->pStars;
//		};
		if (bShowAnim)
		{
			clEffQue *Eff;
			if (bAtt)
			{
				Eff=Attacker->G_Obj->AddAttackEnemyEffect(3,0);
			}
			else
			{
				Eff=Defender->G_Obj->AddAttackUnitEffect(3,0);
			};
			Eff->iNum=20;Eff->Param1=20;Eff->x=10;Eff->y=0;Eff->w=44;Eff->h=40;Eff->r=150;Eff->g=200;Eff->b=50;
		};
//		Defender->G_Obj->Make_Stars(Defender->G_Obj,20,20,10,0,44,40,150,200,50);		
	};

	//Повреждение брони
	if (Attacker->CheckSkill(45))
	{
		Defender->pEffect=new BF_Effect(Defender->pEffect,4,-1,Attacker->CheckSkill(45));
		Defender->pEffect->bVisible=true;
		Defender->pEffect->pAbility=(pFirstUnitUpg+75);
		if (Defender->pUnit->pHero)
		{
			if (Defender->pUnit->pHero->DamageArmor(1))
			{
				if (bShowAnim)
				{
					clEffQue *Eff;
					if (bAtt)
					{
						Eff=Attacker->G_Obj->AddAttackEnemyEffect(2,20);
					}
					else
					{
						Eff=Defender->G_Obj->AddAttackUnitEffect(2,20);
					};
					Eff->iNum=372;
					Eff->iColor=makecol(220,150,20);
					Eff->wavSound=wavArmor;
				};
			};
		};
	};

	//Корни
	if ((Attacker->CheckSkill(67))&&(Defender->iLife>Hit)&&(Defender->CheckSkill(203)==0))
	{
		BF_Effect *EF=Defender->pEffect;
		bool bNew=true;
		while (EF)
		{
			if (EF->pAbility==(pFirstUnitUpg+109))
			{
				bNew=false;
				if (Attacker->CheckSkill(67)>EF->iTime)
				{EF->iTime=Attacker->CheckSkill(67);};
			};
			EF=EF->pNext;
		};
		if (bNew)
		{
			Defender->pEffect=new BF_Effect(Defender->pEffect,88,1,Attacker->CheckSkill(67));
			Defender->pEffect->bVisible=true;
			Defender->pEffect->pAbility=(pFirstUnitUpg+109);			
		};
		if (bShowAnim)
		{
			clEffQue *Eff;
			if (bAtt)
			{
				Eff=Attacker->G_Obj->AddAttackEnemyEffect(3,0);
			}
			else
			{
				Eff=Defender->G_Obj->AddAttackUnitEffect(3,0);
			};
			Eff->iNum=20;Eff->Param1=20;Eff->x=10;Eff->y=0;Eff->w=44;Eff->h=40;Eff->r=150;Eff->g=200;Eff->b=50;
			Eff->wavSound=wavRoot;
		};		
	};

	//Повреждение оружия
	if (Attacker->CheckSkill(52))
	{
		Defender->pEffect=new BF_Effect(Defender->pEffect,2,-1,Attacker->CheckSkill(52));
		Defender->pEffect->bVisible=true;
		Defender->pEffect->pAbility=(pFirstUnitUpg+82);
		Defender->pEffect=new BF_Effect(Defender->pEffect,3,-1,Attacker->CheckSkill(52));	
		Defender->pEffect->pAbility=(pFirstUnitUpg+82);
		if (Defender->pUnit->pHero)
		{
			if (Defender->pUnit->pHero->DamageWeapon(2))
			{
				if (bShowAnim)
				{
					clEffQue *Eff;
					if (bAtt)
					{
						Eff=Attacker->G_Obj->AddAttackEnemyEffect(2,20);
					}
					else
					{
						Eff=Defender->G_Obj->AddAttackUnitEffect(2,20);
					};
					Eff->iNum=371;
					Eff->iColor=makecol(220,150,20);
					Eff->wavSound=wavWeapon;
				};
			};
		};
	};

	//Калечащий укус
	if (Attacker->CheckSkill(51))
	{
		bool WillCurse=true;
		Eff=Defender->pEffect;
		while (Eff)
		{
			if (Eff->pAbility==(pFirstUnitUpg+81))
			{
				Eff->iTime=2;				
				WillCurse=false;
			};
			Eff=Eff->pNext;
		};
		if (WillCurse)
		{
			Defender->pEffect=new BF_Effect(Defender->pEffect, 7, -1, 2);
			Defender->pEffect->bVisible=true;
			Defender->pEffect->pAbility=(pFirstUnitUpg+81);			
		};		
	};


	//Проклятие
	if ((Attacker->CheckSkill(43)>Defender->GetResist())&&(!Defender->CheckSkill(60)))
	{
		bool WillCurse=true;
		Eff=Defender->pEffect;
		while (Eff)
		{
			if (Eff->pAbility==(pFirstUnitUpg+73))
			{
				if (Eff->iTime<Attacker->CheckSkill(43)-Defender->GetResist())
				{
					Eff->iTime=Attacker->CheckSkill(43)-Defender->GetResist();
				};
				WillCurse=false;
			};
			Eff=Eff->pNext;
		};
		if (WillCurse)
		{
			Defender->pEffect=new BF_Effect(Defender->pEffect, 2, -1, Attacker->CheckSkill(43)-Defender->GetResist());
			Defender->pEffect->bVisible=true;
			Defender->pEffect->pAbility=(pFirstUnitUpg+73);
			Defender->pEffect=new BF_Effect(Defender->pEffect, 4, -1, Attacker->CheckSkill(43)-Defender->GetResist());
			Defender->pEffect->pAbility=(pFirstUnitUpg+73);
			Defender->pEffect=new BF_Effect(Defender->pEffect, 5, -1, Attacker->CheckSkill(43)-Defender->GetResist());
			Defender->pEffect->pAbility=(pFirstUnitUpg+73);
			Defender->pEffect=new BF_Effect(Defender->pEffect, 6, -1, Attacker->CheckSkill(43)-Defender->GetResist());
			Defender->pEffect->pAbility=(pFirstUnitUpg+73);			
		};
//		if (Defender->G_Obj->pStars)
//		{
//			delete Defender->G_Obj->pStars;
//		};
		if (bShowAnim)
		{
			clEffQue *Eff;
			if (bAtt)
			{
				Eff=Attacker->G_Obj->AddAttackEnemyEffect(3,0);
			}
			else
			{
				Eff=Defender->G_Obj->AddAttackUnitEffect(3,0);
			};
			Eff->iNum=20;Eff->Param1=20;Eff->x=10;Eff->y=0;Eff->w=44;Eff->h=40;Eff->r=150;Eff->g=200;Eff->b=50;
		};
//		Defender->G_Obj->Make_Stars(Defender->G_Obj,20,20,10,0,44,40,150,200,50);
	};

	//Уязвимость
	if ((Attacker->CheckSkill(79)>0)&&((pFirstSpell+21)->arEffect[2][0]>=Defender->GetResist())&&(Defender->CheckSkill(60)==0))
	{		
		Defender->UnderSpell((pFirstSpell+21),Attacker,Attacker->iSide);
	};

	//Болезнь
	if ((Attacker->CheckSkill(80)>0)&&((pFirstSpell+30)->arEffect[2][0]>=Defender->GetResist())&&(Defender->CheckSkill(60)==0)&&
		(Defender->GetUnitKind()!=2)&&(Defender->GetUnitKind()!=4))
	{		
		Defender->UnderSpell((pFirstSpell+30),Attacker,Attacker->iSide);
	};

	//Удар+заклинание
	if (Attacker->CheckSkill(206)>0)
	{
		int Spell=Attacker->CheckSkill(206);
		bool bCast=true;
		if ((pFirstSpell+Spell)->arEffect[2][0]<=Defender->GetResist())
		{bCast=false;}
		for (int i=0; i<MAX_UNIT_KIND; i++)
		{
			if ((pFirstSpell+Spell)->arUnitKind[i]==Defender->GetUnitKind())
			{bCast=false;}
		};
		for (int i=0; i<MAX_ANTI_EFFECT; i++)
		{
			if (((pFirstSpell+Spell)->arAntiEffect[i]>0)&&(Defender->CheckSkill((pFirstSpell+Spell)->arAntiEffect[i])>0))
			{bCast=false;}
		};			
		if (bCast)
		{		
			Defender->UnderSpell((pFirstSpell+Spell),Attacker,Attacker->iSide);
		}
	}

	//Круговая атака
	if ((bCanRound)&&((Attacker->CheckSkill(65))||((Attacker->CheckSkill(66))&&(buttRoundAtt.bPress)&&(bAtt))))
	{
		bool bEffect=false;
		if (Attacker->FindNeighbour(1-Attacker->iSide)>1)
		{
			BF_Unit *arN[6];
			for (int i=0; i<6; i++)
			{
				arN[i]=arNeigh[i];
			};
			for (int i=0; i<6; i++)
			{
				if ((arN[i])&&(arN[i]!=Defender))
				{
					int Hit=GetMeleeHit(Attacker,arN[i],bAtt);
					if (Hit>arN[i]->iLife)
					{
						Hit=arN[i]->iLife;
					};
					HitEffect(Attacker, arN[i], Hit, bAtt, false);
					bool bDead=Hit>=arN[i]->iLife;
					if (bShowAnim)
					{
						clEffQue *Eff;
						bEffect=true;
						if (bAtt)
						{
							Eff=Attacker->G_Obj->AddAttackEnemyEffect(1,0,arN[i]->G_Obj);
						}
						else
						{
							Eff=Defender->G_Obj->AddAttackEnemyEffect(1,0,arN[i]->G_Obj);
						};
						Eff->iColor=makecol(220,50,50);
						Eff->iNum=Hit;
						if (bGoBerz)
						{
							if (bAtt)
							{
								Eff=Attacker->G_Obj->AddAttackEnemyEffect(2,15,arN[i]->G_Obj);
							}
							else
							{
								Eff=Defender->G_Obj->AddAttackEnemyEffect(2,15,arN[i]->G_Obj);
							};
							Eff->iColor=makecol(250,150,20);
							Eff->iNum=3;
							Eff->wavSound=wavBerz;							
						};
						if (bDead)
						{
							if (bAtt)
							{
								Eff=Attacker->G_Obj->AddAttackEnemyEffect(4,0,arN[i]->G_Obj);
							}
							else
							{
								Eff=Defender->G_Obj->AddAttackEnemyEffect(4,0,arN[i]->G_Obj);
							};				
						}
				//		else
				//		{arN[i]->DrawBar();};
					};
					arN[i]->TakeDamage(Hit,1);
					if ((bShowAnim)&&(!bDead))
					{arN[i]->DrawBar();};
				};
			};
		};
		if ((bShowAnim)&&(bEffect)&&(buttRoundAtt.bPress)&&(bAtt))
		{
			clEffQue *Eff;			
			Eff=Attacker->G_Obj->AddAttackUnitEffect(2,0);
			Eff->iColor=makecol(220,220,70);
			Eff->iNum=70;			
			Eff->wavSound=wavRound;
		};
	};

	//Вампиризм
	if ((Attacker->CheckSkill(68))&&(Defender->pUnit->GetUnitKind()!=2)
		&&(Defender->pUnit->GetUnitKind()!=4)&&(Attacker->iLife<Attacker->GetLife()))
	{
		int Drain=Hit*Attacker->CheckSkill(68)/100;
		if (Drain>Attacker->GetLife()-Attacker->iLife)
		{
			Drain=Attacker->GetLife()-Attacker->iLife;
		};
		Attacker->iLife+=Drain;
		Vampire+=Drain;
		if (bShowAnim)
		{
			clEffQue *Eff;
			if (bCanRound)
			{
				if (bAtt)
				{
					Eff=Attacker->G_Obj->AddAttackUnitEffect(1,0);
				}
				else
				{
					Eff=Defender->G_Obj->AddAttackEnemyEffect(1,0);
				};
				Eff->iColor=makecol(20,220,20);
				Eff->iNum=Vampire;
			}
			else
			{
				//*****//
			};
		};

//		Attacker->G_Obj->AnimNumber(Drain,makecol(20,220,20));
	};


	}; //Суицид
};

void ShootEffect(BF_Unit * Attacker, BF_Unit * Defender, int Hit)
{	
	//Ядовитый выстрел
	if ((Attacker->CheckSkill(78))&&(!Defender->CheckSkill(42)))
	{
		Defender->pEffect=new BF_Effect(Defender->pEffect,1,-Attacker->CheckSkill(78),POISON_DURATION);
		Defender->pEffect->bVisible=true;
		Defender->pEffect->pAbility=(pFirstUnitUpg+71);
	};

	//Тяжелые снаряды
	if (Attacker->CheckSkill(69))
	{
		Defender->pEffect=new BF_Effect(Defender->pEffect,5,-1,Attacker->CheckSkill(69));
		Defender->pEffect->bVisible=true;
		Defender->pEffect->pAbility=(pFirstUnitUpg+114);
		if (Defender->pUnit->pHero)
		{
			if ((Defender->pUnit->pHero->DamageArmor(1))&&(bShowAnim))
			{				
				Defender->G_Obj->AnimText(372,makecol(220,150,20));
				PlayUnitSound(wavArmor);
			//	play_sample(wavArmor,iSoundVolume,128,1000,0);
			//	clEffQue *Eff=Attacker->G_Obj->pLink->AddAttackEnemyEffect(2,20);
			//	Eff->iNum=372;
			//	Eff->iColor=makecol(250,150,20);
			};
		};
	};

		//Выстрел+заклинание
	if (Attacker->CheckSkill(207)>0)
	{
		int Spell=Attacker->CheckSkill(207);
		bool bCast=true;
		if ((pFirstSpell+Spell)->arEffect[2][0]<=Defender->GetResist())
		{bCast=false;}
		for (int i=0; i<MAX_UNIT_KIND; i++)
		{
			if ((pFirstSpell+Spell)->arUnitKind[i]==Defender->GetUnitKind())
			{bCast=false;}
		};
		for (int i=0; i<MAX_ANTI_EFFECT; i++)
		{
			if (((pFirstSpell+Spell)->arAntiEffect[i]>0)&&(Defender->CheckSkill((pFirstSpell+Spell)->arAntiEffect[i])>0))
			{bCast=false;}
		};			
		if (bCast)
		{		
			Defender->UnderSpell((pFirstSpell+Spell),Attacker,Attacker->iSide);
		}
	}

};

//***********************************************************
//*****  Методы BF_Effect  **********************************
//***********************************************************

BF_Effect::BF_Effect(BF_Effect *prEffect, int Type, int Power, int Time)
{
	pNext=prEffect;
	pPrev=NULL;
	if (pNext)
	{
		pNext->pPrev=this;
	};
	iType=Type;
	iPower=Power;
	iTime=Time;
	pSpell=NULL;
	pAbility=NULL;
	bVisible=false;
	bDestroyOnHit=false;
};

BF_Effect::~BF_Effect()
{
	if (bGameInProcess)
	{
	if (pPrev)
	{
		pPrev->pNext=pNext;
	};
	if (pNext)
	{
		pNext->pPrev=pPrev;
	};
	};
};


//***********************************************************
//*****  Методы BF_Unit  ************************************
//***********************************************************


BF_Unit::BF_Unit()
{
	iLife=0;
	bInBattle=false;
	pUnit=NULL;
	pHero=NULL;
	objBar=NULL;
	bPacked=false;
};

BF_Unit::~BF_Unit()
{
	if (bGameInProcess)
	{
	BF_Effect *Temp;
	while (pEffect)
	{
		Temp=pEffect->pNext;
		delete pEffect;
		pEffect=Temp;
	};
	};
};

G_Object * BF_Unit::Init(dcUnit *Un, G_Object *Obj, int Sider, int Numm)
{
	pUnit=Un;
	pTrueForm=pUnit;
	iSide=Sider;
	iNumm=Numm;
	iLife=Un->iLife*LifeMod[iSide]/100+Heroes[iSide]->CheckUnitBonus(1)*(100-(Un->GetLife()-Un->iLife)*100/Un->GetLife())/100;	
	x=Un->x;
	y=Un->y;
//	textprintf(screen,Font1,100,100,COLOR_2,"%d, %d",x ,y);
	arGrid[x][y].Pass=0;	
	iMorale=Un->GetMorale();
	iFear=0;
	iStamina=Un->GetStamina()+Heroes[iSide]->CheckUnitBonus(11);
	iSpeed=Un->GetSpeed();
	if (pUnit->CheckAbility(88)>0)
	{iSpeed=0;}
	iAmmo=Un->GetAmmo();
	iMeleeDamageDone=0;
	iRangeDamageDone=0;
	iMeleeDamageTaken=0;
	iRangeDamageTaken=0;
	iSpellDamageTaken=0;
	iPoisonDamageTaken=0;
	iDamageHealed=0;
	iSpellCast=0;
	iEnemyShotted=0;
	iEnemySlayed=0;
	iStaminaUsed=0;
	iSurvived=1;
	bCanAct=true;
	bResting=false;	
	bInBattle=true;
	bSummoned=false;
	bParry=false;
	pEffect=NULL;
	bMarch=true;//CheckSkill(29);	
	bPacked=false;
	if ((iAmmo>1)&&(Heroes[iSide])&&(CheckSkill(24)==0)&&(CheckSkill(28)==0))
	{iAmmo+=Heroes[iSide]->CheckUnitBonus(10);};
	if (CheckSkill(19)==0)
	{
		iMorale+=Un->iMorale/2+Heroes[iSide]->CheckUnitBonus(12)+Heroes[iSide]->GetSquadMorale(Numm);
		if (iSide)
		{iMorale+=ProvinceMoraleBonus;};
	};
	if (iMorale<1)
	{iMorale=1;};
	if (iLife>GetLife())
	{iLife=GetLife();};
	//Врождённая реинкарнация
	if (CheckSkill(97)>0)
	{
		pEffect=new BF_Effect(pEffect,74,1,0);
		pEffect->pSpell=(pFirstSpell+59);
		pEffect->bDestroyOnHit=false;
		pEffect->bVisible=true;
	};
	if (bSimulate)
	{
		G_Obj=objSim;
		objBar=objSim;
	}
	else
	{
		G_Obj = new G_Object(Obj, TransX(x,y,pUnit->pType()->Pic->w), TransY(x,y,pUnit->pType()->Pic->h), 2, pUnit->pType()->Pic);
		G_Obj->Shadow=pUnit->pType()->Shadow;
		G_Obj->FlipShadow=pUnit->pType()->FlipShadow;
		if (objBar)	
		{
			delete objBar;
			objBar=NULL;
		};
		if (Obj)
		{
			objBar=new G_Object(NULL,0,0,3,bmpBar);	
			G_Obj->AddCargo(objBar,(pUnit->pType()->Pic->w-bmpUnitBar->w)/2,pUnit->pType()->Pic->h,3);
			DrawBar();
		};
	};	
	return G_Obj;
};

G_Object * BF_Unit::Summon(int U_Type, int Power, int sx, int sy, G_Object *Obj)
{
	pUnit=new dcUnit(U_Type,GetUnitExp(U_Type,Power));
	pTrueForm=pUnit;
/*	pUnit->iLevel=Power;
	for (int i=0; i<pUnit->iLevel; i++)
	{
		pUnit->arUpg[i]=pUnit->LevelUp(i+1,0);
	};	
*/
//	textprintf_centre_ex(screen,Font1,500,190,makecol(250,250,250),-1,"U_Type: %d",U_Type);
//	readkey();	
	iSide=AA;
	iNumm=0;
	iLife=pUnit->GetLife();//+Heroes[iSide]->CheckUnitBonus(1);
	x=sx;
	y=sy;
	arGrid[x][y].Pass=0;
	iMorale=pUnit->GetMorale();
	iFear=0;
	iStamina=pUnit->GetStamina()+Heroes[iSide]->CheckUnitBonus(11);
	iSpeed=pUnit->GetSpeed();
	if (pUnit->CheckAbility(88)>0)
	{iSpeed=0;}
	iAmmo=pUnit->GetAmmo();
	iMeleeDamageDone=0;
	iRangeDamageDone=0;
	iMeleeDamageTaken=0;
	iRangeDamageTaken=0;
	iSpellDamageTaken=0;
	iPoisonDamageTaken=0;
	iDamageHealed=0;
	iSpellCast=0;
	iEnemyShotted=0;
	iEnemySlayed=0;
	iStaminaUsed=0;
	iSurvived=0;
	bCanAct=true;
	bResting=false;	
	bInBattle=true;
	bSummoned=true;
	bParry=false;
	pEffect=NULL;
	bPacked=false;
	bMarch=true;//CheckSkill(29);	
//	if ((iAmmo>0)&&(Heroes[iSide])&&(CheckSkill(24)==0)&&(CheckSkill(28)==0))
//	{iAmmo+=Heroes[iSide]->CheckUnitBonus(10);};
//	if (CheckSkill(19)==0)
//	{
//		iMorale+=Heroes[iSide]->CheckUnitBonus(12);
//		if (iSide)
//		{iMorale+=ProvinceMoraleBonus;};
//	};
	if (iMorale<1)
	{iMorale=1;};
	if (iLife>GetLife())
	{iLife=GetLife();};
	//Врождённая реинкарнация
	if (CheckSkill(97)>0)
	{
		pEffect=new BF_Effect(pEffect,74,1,0);
		pEffect->pSpell=(pFirstSpell+59);
		pEffect->bDestroyOnHit=false;
		pEffect->bVisible=true;
	};
	if (bSimulate)
	{
		G_Obj=objSim;
		objBar=objSim;
	}
	else
	{
		if (objBar)	
		{
			delete objBar;
			objBar=NULL;
		};
		G_Obj = new G_Object(Obj, TransX(x,y,pUnit->pType()->Pic->w), TransY(x,y,pUnit->pType()->Pic->h), 2, pUnit->pType()->Pic);
		G_Obj->Shadow=pUnit->pType()->Shadow;
		G_Obj->FlipShadow=pUnit->pType()->FlipShadow;
		G_Obj->pRaise=new G_Raise(G_Obj);
		G_Obj->Make_Stars(G_Obj,80,30,10,20,44,40,180,100,250);	
		objBar=new G_Object(NULL,0,0,3,bmpBar);	
		G_Obj->AddCargo(objBar,(pUnit->pType()->Pic->w-bmpUnitBar->w)/2,pUnit->pType()->Pic->h,3);	
		DrawBar();
	};
	return G_Obj;
};

void BF_Unit::DrawBar()
{
	if ((objBar)&&(bInBattle)&&(pUnit)&&(!bSimulate))
	{
	if ((bShowAnim)&&(bInBattle)&&(iLife>0)&&(GetLife()>0)&&(GetMorale()>0)&&(GetStamina()>0))
	{
		blit(bmpUnitBar,bmpBar,0,0,0,0,bmpUnitBar->w,bmpUnitBar->h);
		int Size=30*iLife/GetLife();
		if (Size>30){Size=30;};
		rectfill(bmpBar,15,2,15+Size,3,makecol(200,0,0));
		Size=30*iStamina/GetStamina();
		if (Size>30){Size=30;};
		rectfill(bmpBar,15,7,15+Size,9,makecol(200,200,0));
		Size=30*iMorale/GetMorale();
		if (Size>30){Size=30;};
		rectfill(bmpBar,15,12,15+Size,15,makecol(50,50,250));
		if (bCanAct)
		{textprintf_ex(bmpBar,Font1,3,3,makecol(20,200,20),-1,"%d",iSpeed);}
		else
		{textprintf_ex(bmpBar,Font1,3,3,makecol(200,20,20),-1,"%d",iSpeed);};
		if ((iLife>0)&&((arHumans[arPlayer[ActivePlayer].iColor].bWarBar)||(bAltFlag)))
		{objBar->iLayer=3;}
		else
		{objBar->iLayer=-1;};
	}
	else
	{objBar->iLayer=-1;};
	};
};

void BF_Unit::Clear()
{
	iLife=0;
	pUnit=NULL;
	pHero=NULL;
//	x=-1;
//	y=-1;
	iFear=0;
	iStamina=0;
	iSpeed=0;
	iAmmo=0;
	iMeleeDamageDone=0;
	iRangeDamageDone=0;
	iMeleeDamageTaken=0;
	iRangeDamageTaken=0;
	iSpellDamageTaken=0;
	iDamageHealed=0;
	iSpellCast=0;
	iEnemyShotted=0;
	iEnemySlayed=0;
	iStaminaUsed=0;
	iSurvived=0;
	bCanAct=false;
	bResting=false;	
	bInBattle=false;
	bSummoned=false;
	bParry=false;
//	G_Obj=NULL;
	pEffect=NULL;
	bMarch=false;
	bPacked=false;
};

int BF_Unit::CheckEffect(int Index)
{
	int Result=0;
	if (pEffect)
	{
		bool Search=true;
		BF_Effect *Eff=pEffect;
		while (Search)
		{
			if (Eff->iType==Index)
			{
				Result+=Eff->iPower;
			};
			if (Eff->pNext)
			{
				Eff=Eff->pNext;
			}
			else
			{
				Search=false;
			};
		};
	};
	return Result;
};

int BF_Unit::GetEffectDuration(int Index)
{
	if (pEffect)
	{
		bool Search=true;
		BF_Effect *Eff=pEffect;
		while (Search)
		{
			if (Eff->iType==Index)
			{
				return Eff->iTime;
			};
			if (Eff->pNext)
			{
				Eff=Eff->pNext;
			}
			else
			{
				Search=false;
			};
		};
	};
	return 0;
};

int BF_Unit::CheckSkill(int Index)
{
	int Result=0;
	if ((Index>=32)&&(Index<=34))
	{
		if (CheckSkill(14)>0)
		{return 0;};
	};
	Result+=CheckEffect(Index);
	if ((Result<=0)||(Index!=25))
	{
		Result+=pUnit->CheckAbility(Index);
		Result+=pUnit->CheckUpg(Index);
	};
	if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0)){Result+=Heroes[iSide]->CheckUnitBonus(Index);};
	return Result;
};

bool BF_Unit::CheckCaster()
{
	for (int i=1; i<=iMaxSpell; i++)
	{
		if (CheckSkill(2000+i))
		{
			return true;
		};
	};
	return false;
};

int BF_Unit::FindNeighbour(int Side)
{
	for (int i=0; i<6; i++)
	{
		arNeigh[i]=NULL;
	};
	int Found=0;
	for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
	{
		if ((Squad[Side][k].iLife)&&(Squad[Side][k].x==x-1+y%2)&&(Squad[Side][k].y==y-1))
		{
			arNeigh[Found]=&Squad[Side][k];
			Found++;
		};
		if ((Squad[Side][k].iLife)&&(Squad[Side][k].x==x+y%2)&&(Squad[Side][k].y==y-1))
		{
			arNeigh[Found]=&Squad[Side][k];
			Found++;
		};
		if ((Squad[Side][k].iLife)&&(Squad[Side][k].x==x+1)&&(Squad[Side][k].y==y))
		{
			arNeigh[Found]=&Squad[Side][k];
			Found++;
		};
		if ((Squad[Side][k].iLife)&&(Squad[Side][k].x==x+y%2)&&(Squad[Side][k].y==y+1))
		{
			arNeigh[Found]=&Squad[Side][k];
			Found++;
		};
		if ((Squad[Side][k].iLife)&&(Squad[Side][k].x==x-1+y%2)&&(Squad[Side][k].y==y+1))
		{
			arNeigh[Found]=&Squad[Side][k];
			Found++;
		};
		if ((Squad[Side][k].iLife)&&(Squad[Side][k].x==x-1)&&(Squad[Side][k].y==y))
		{
			arNeigh[Found]=&Squad[Side][k];
			Found++;
		};
	};
	return Found;
};

bool BF_Unit::IsNeighbour(int Rx, int Ry)
{
	
	if ((Rx==x-1+y%2)&&(Ry==y-1))
	{
		return true;		
	};
	if ((Rx==x+y%2)&&(Ry==y-1))
	{
		return true;		
	};
	if ((Rx==x+1)&&(Ry==y))
	{
		return true;
	};
	if ((Rx==x+y%2)&&(Ry==y+1))
	{
		return true;
	};
	if ((Rx==x-1+y%2)&&(Ry==y+1))
	{
		return true;
	};
	if ((Rx==x-1)&&(Ry==y))
	{
		return true;
	};	
	return false;
};


void BF_Unit::UseStamina(unsigned int Num)
{
	if (!CheckSkill(18))
	{
		if (iStamina<Num)
		{
			ChangeMorale(iStamina-Num);
			Num=iStamina;
		};
		iStamina-=Num;
		iStaminaUsed+=Num;
	};
};

void BF_Unit::ChangeMorale(int Num)
{
	if (!CheckSkill(19))
	{
		iMorale+=Num;
		if (iMorale<=0)
		{
			if (iFear>0)
			{
				iFear-=iMorale*10;
			}
			else
			{
				iFear=50-iMorale*10;
			}
			iMorale=0;
		};
	};
};

void BF_Unit::Move(int x1, int y1)
{	
	TruePath=NULL;
	MinWeight=200;
	MinStamina=200;
	bmpPathPic=G_Obj->Pic;
	for (int i=0; i<8; i++){
		for (int j=0; j<8; j++){
			Field[i][j].Land=0;	
		};
	};
	if ((CheckSkill(14))||(CheckSkill(15)))
	{
		FlyPath(x,y,-1,NULL,x1,y1);
	}
	else
	{
		int curStam=iStamina-GetFatigure();
		if (curStam<1)
		{curStam=1;}
		int tmpStep=-GetMoveCost(x,y);
		int tmpStam=-GetStamCost(x,y);
		StepPath(x, y, tmpStep, NULL, x1, y1, tmpStam, curStam);		
	};
	arGrid[x][y].Pass=1;	
	G_Obj->Move(TruePath, 2+GetSpeed()+arHumans[arPlayer[ActivePlayer].iColor].iBattleSpeed, 1, true);
	UseStamina(MinStamina+GetFatigure());
	x=x1;
	y=y1;
	iSpeed-=MinWeight;
	if (iSpeed<0)
	{iSpeed=0;};
	arGrid[x][y].Pass=0;
	if ((iSpeed==0)&&(!FindNeighbour(1-iSide)))
	{
		bCanAct=false;
	};
	DrawBar();
};

void BF_Unit::Death()
{
/*	if (bSimulate)
	{
		G_Obj=objSim;
		objBar=objSim;
	};*/
	
	if (pUnit->pHero)
	{
		pUnit->pHero->DamageArmor(5);
		pUnit->pHero->DamageWeapon(5);
		for (int i=MAX_SQUAD; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if ((Squad[iSide][i].pUnit)&&(Squad[iSide][i].iLife>0))
			{
				Squad[iSide][i].ChangeMorale(-1);
				Squad[iSide][i].DrawBar();
			};
		};
	};	
	
//Изменение боевого духа соседей
	int Nei=FindNeighbour(iSide);
	if (Nei)
	{
		for (int i=0; i<Nei; i++)
		{
			arNeigh[i]->ChangeMorale(-1);
			arNeigh[i]->DrawBar();
		};
	};
	Nei=FindNeighbour(1-iSide);
	if (Nei)
	{
		for (int i=0; i<Nei; i++)
		{
			arNeigh[i]->ChangeMorale(1);
			arNeigh[i]->DrawBar();
		};
	};

//Удаление всех эффектов
	BF_Effect *Temp;
	int iChangeSide=-1;
	bool bReincarnate=false, bDarkPact=false;
	while (pEffect)
	{
		Temp=pEffect->pNext;
		if (pEffect->iType==73)
		{
			iChangeSide=pEffect->iPower;
		};
		if (pEffect->iType==74)
		{
			bReincarnate=true;
		};
		if (pEffect->iType==91)
		{
			bDarkPact=true;
		};
		if (pEffect->iType==90)
		{			
			delete pUnit;
			pUnit=pTrueForm;
			if (iSpeed>GetSpeed())
			{iSpeed=GetSpeed();};
			if (iAmmo>GetAmmo())
			{iAmmo=GetAmmo();};
			G_Obj->Pic=pUnit->pType()->Pic;			
		};
		delete pEffect;
		pEffect=Temp;
	};
	if ((!bReincarnate)&&(!bDarkPact))
	{
		objBar->iLayer=-1;
		arGrid[x][y].Pass=1;
		bCanAct=false;
	};
	if (bReincarnate)
	{
		iLife=GetLife();		
	//	arGrid[x][y].Pass=0;
		iFear=0;	
		pEffect=NULL;
		G_Obj->Pic=pUnit->pType()->Pic;
		G_Obj->iLayer=2;
		G_Obj->bReincarnate=true;
		if (iChangeSide>=0)
		{
			ChangeSide(iChangeSide);
		};
//		>pRaise=new G_Raise(G_Obj);
//		G_Obj->pStars=new G_Stars(G_Obj,80,30,10,20,44,40,250,250,220);
	}
	else
	{
	if (bDarkPact)
	{	
		dcUnit *pDemon;
		switch (pTrueForm->pType()->iLevel)
		{
		case 1:
			pDemon=new dcUnit(21,0);
			break;
		case 2:
			pDemon=new dcUnit(37,0);
			break;
		case 3:
			pDemon=new dcUnit(56,0);
			break;
		case 4:
			pDemon=new dcUnit(65,0);
			break;
		};
		pDemon->x=pTrueForm->x;
		pDemon->y=pTrueForm->y;		
		if (pUnit==pTrueForm)
		{
			if (bSimulate)
			{bPacked=true;}
			else
			{delete pTrueForm;}
			pTrueForm=pDemon;
			pUnit=pDemon;
		}
		else
		{
			if (bSimulate)
			{bPacked=true;}
			else
			{delete pTrueForm;}
			pTrueForm=pDemon;
		};
		if ((!bSummoned)&&(!bSimulate))
		{
			if (iChangeSide>=0)
			{Heroes[1-iSide]->pSquad[iNumm]=pDemon;}
			else
			{Heroes[iSide]->pSquad[iNumm]=pDemon;}
		};
		iLife=GetLife();
		iAmmo=GetAmmo();
		iStamina=GetStamina();
		iMorale=GetMorale();		
	//	arGrid[x][y].Pass=0;
		iFear=0;	
		pEffect=NULL;
		G_Obj->Pic=pUnit->pType()->Pic;
		G_Obj->Shadow=pUnit->pType()->Shadow;
		G_Obj->iLayer=2;
		G_Obj->bReincarnate=true;
		if (iChangeSide>=0)
		{
			ChangeSide(iChangeSide);
		};
//		>pRaise=new G_Raise(G_Obj);
//		G_Obj->pStars=new G_Stars(G_Obj,80,30,10,20,44,40,250,250,220);
	}
	else
	{

	if (bSummoned)
	{
		SummonExp[iSide]+=(pUnit->pType()->iExp+pUnit->pType()->iExp*pUnit->iLevel/10);
		SummonSurv[iSide]+=iSurvived;
		SummonWork[iSide]+=iMeleeDamageDone*2+iRangeDamageDone+iMeleeDamageTaken+iRangeDamageTaken+
		iSpellDamageTaken+iPoisonDamageTaken+iDamageHealed*2+iEnemyShotted*10+iEnemySlayed*20+iStaminaUsed;
		delete pUnit;
		if (!bSimulate)
		{
			G_Obj->bDestroyOnDeath=true;
			G_Obj->RemoveCargo(objBar);
		};
//		bInBattle=false;
		Clear();
		iSurvived=1;	//Отметка для запрета повторного использования в этом бою
	}
	else
	{
		if (iChangeSide>=0)
		{
			ChangeSide(iChangeSide);
		}
		else
		{
			if ((pUnit->GetUnitKind()==2)||(pUnit->GetUnitKind()==4)||((pFirstBF_Obj+arGrid[x][y].Land)->iMoveCost<=0))
			{
				bInBattle=false;
				if (!bSimulate)
				{G_Obj->bInvisOnDeath=true;};
			};
		};
	};
	};
	};

};

void BF_Unit::Panic()
{	
	if ((bSimulate)||(Random(100)>iFear)||(CheckSkill(36)>0)||(CheckSkill(88)>0)||(iStamina<=0))
	{
		G_Obj->AnimText(1, makecol(220,50,20));		
	}
	else
	{
		if (x!=iSide*7)
		{
		//Построение карты поиска пути
		for (int i=0; i<8; i++){
			for (int j=0; j<8; j++)
			{
				Field[i][j].Land=0;
				Field[i][j].Pass=0;
				Field[i][j].Shoot=0;
			};
		};
		bFirstMove=true;
		if (CheckSkill(14)) //Летающий
		{
			FlyMap(x,y,-1,iSpeed,true);
		}
		else
		{
			if (CheckSkill(15)) //Низколетающий
			{
				FlyMap(x,y,-1,iSpeed,false);
			}
			else
			{
				StepMap(x,y,-1,iSpeed,0,iStamina);
			};
		};
		int Found=0;
		int arSteps [2][8];
		for (int i=iSide*7; ((i!=7-iSide*7)&&(Found==0)); i=i+1-iSide*2)
		{
			for (int j=0; j<8; j++)
			{
				if ((Field[i][j].Pass)&&(arGrid[i][j].Pass))
				{
					arSteps[0][Found]=i;
					arSteps[1][Found]=j;
					Found++;
				};
			};
		};
		if (Found)
		{
			Found=Random(Found);
			G_Obj->AnimText(2, makecol(220,50,20));
//			textprintf_ex(screen,Font1,0,605,makecol(220,50,50),-1,"x %d, y %d", arSteps[0][Found], arSteps[1][Found]);
//			readkey();
			Move(arSteps[0][Found],arSteps[1][Found]);
		}
		else
		{
			G_Obj->AnimText(1, makecol(220,50,20));
		};
	}
	else
	{
		G_Obj->AnimText(1, makecol(220,50,20));
		//Убег
	};
	};
	iSpeed=0;
	bCanAct=false;
	DrawBar();
};

bool BF_Unit::TakeDamage (int Hit, int dType)
{
	bGoBerz=false;
	switch (dType)
	{
	case 0:
		iMeleeDamageTaken+=Hit;
		break;
	case 1:
		iRangeDamageTaken+=Hit;
		break;
	case 2:
		iSpellDamageTaken+=Hit;
		break;
	case 3:
		iPoisonDamageTaken+=Hit;
		break;
	};
	BF_Effect *Eff=pEffect;
	while (Eff)
	{
		BF_Effect *Eff1=Eff->pNext;
		if (Eff->bDestroyOnHit)
		{
			if (Eff->pPrev)
			{
				delete Eff;
			}
			else
			{
				pEffect=Eff->pNext;
				delete Eff;							
			};						
		};
		Eff=Eff1;
	};
	iLife-=Hit;
	if (iLife==0)
	{
		Death();
	//	G_Obj->AnimDeath();
		return 1;
	};
	
	if (((Hit>=GetLife()/4)||(Hit>9))&&(dType!=3))
	{
		if ((CheckSkill(25))&&(!CheckEffect(25)))
		{
			if (bShowAnim)
			{
				bGoBerz=true;
			};
	//		G_Obj->AnimText(3, makecol(250,150,20));
			if (iMorale<GetMorale())
			{iMorale=GetMorale();};
			pEffect=new BF_Effect(pEffect, 2, CheckSkill(25), 0);
			pEffect=new BF_Effect(pEffect, 25, CheckSkill(25), 0);
			pEffect->pAbility=(pFirstUnitUpg+55);
			pEffect->bVisible=true;
			pEffect=new BF_Effect(pEffect, 13, 1, 0);
			pEffect=new BF_Effect(pEffect, 19, 1, 0);
		}
		else
		{
			ChangeMorale(-1);
		};
	};
	return 0;
};

void BF_Unit::ChangeSide(int NewNum)
{
	int iNum;
	G_Obj->RemoveCargo(objSelect);
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		if (Squad[iSide][i].pUnit==pUnit)
		{
			iNum=i;
		};
	};
	BITMAP *bmpOld=Squad[1-iSide][NewNum].bmpBar;
	Squad[1-iSide][NewNum]=Squad[iSide][iNum];
	Squad[1-iSide][NewNum].iSide=1-iSide;
	Squad[1-iSide][NewNum].G_Obj->bFlip=!Squad[1-iSide][NewNum].G_Obj->bFlip;
	Squad[iSide][iNum].bmpBar=bmpOld;
	if (Squad[1-iSide][NewNum].iLife>0)
	{Squad[1-iSide][NewNum].DrawBar();};
	Clear();
	G_Obj=NULL;
	objBar=NULL;
};


//Воздействие заклинаний
int BF_Unit::UnderSpell(clSpell *Spell, BF_Unit * Hero, int Caster)
{
	G_Object *objDeath=G_Obj;
	int Hit, Resist, SpellPower, Duration, iType=pUnit->GetUnitType();
	bool bDestroyOnHit=false, bLifeDrain=false, bFriendlyFire=false;
	BF_Effect * Eff, * Eff1;
	Resist=GetResist()-Hero->CheckSkill(908);
	/*if (Hero->pHero)
	{Resist-=Hero->pHero->CheckHeroBonus(908);}	*/
	
	Resist=Resist*Spell->iResistMod/100;

	if (Resist<0){Resist=0;};
	if (Spell->bNegative)
	{			
		G_Obj->Make_Stars(G_Obj,50,40,10,0,44,40,250,150,150);
	}
	else
	{
		G_Obj->Make_Stars(G_Obj,50,40,10,0,44,40,10,200,200);
	};
	for (int i=0; i<MAX_SPELL_EFFECT; i++)
	{		
		SpellPower=Spell->arEffect[1][i];
		if (Spell->arEffect[1][i]>0)
		{
			SpellPower=Spell->arEffect[1][i]+Hero->CheckSkill(905)*Spell->iPowerMod/100;
			/*if (Hero->pHero)			
			{
				SpellPower=Spell->arEffect[1][i]+Hero->pHero->CheckHeroBonus(905)*Spell->iPowerMod/100;			
			}*/
		}
		if (Spell->arEffect[1][i]<0)
		{
			SpellPower=Spell->arEffect[1][i]-Hero->CheckSkill(905)*Spell->iPowerMod/100;
			/*if (Hero->pHero)
			{
				SpellPower=Spell->arEffect[1][i]-Hero->pHero->CheckHeroBonus(905)*Spell->iPowerMod/100;
			}	*/	
		}
		Duration=Spell->arEffect[2][i];
		int Index=Spell->arEffect[0][i];
		if (Duration>0)
		{
			Duration+=Hero->CheckSkill(906)*Spell->iDurationMod/100;
			/*if (Hero->pHero)
			{
				Duration+=Hero->pHero->CheckHeroBonus(906)*Spell->iDurationMod/100;
			};*/
			if (Index==1)
			{
				Index=-1;
			};
			if (Index==11)
			{
				Index=-11;
			};
		};
		switch (Index)
		{
		case 0:
			break;
		case 1:
			if ((Spell->bNegative)&&((iSide!=Caster)||(!bFriendlyFire)))				
			{
				if (Spell->arEffect[2][i]==-1)
				{
			//Повреждение
					Hit=GetHit(SpellPower,Resist*Spell->bResistPower+GetDefence()*Spell->bDefencePower);
					if (Hit>iLife)
					{Hit=iLife;};					
					G_Obj->AnimNumber(Hit,makecol(250,50,180));
					G_Obj->iExplo=18;
					
					if (bLifeDrain)
					{
						int Heal=Hit;
						if (Heal>Hero->GetLife()-Hero->iLife)
						{Heal=Hero->GetLife()-Hero->iLife;};
						if (Heal>0)
						{
							Hero->iLife+=Heal;
							Hero->G_Obj->AnimNumber(Heal,makecol(50,220,50));
						};
					};					
					if (TakeDamage(Hit,2))
					{
					//	play_sample((pFirstUnitType+pUnit->ipType)->wavDeath,iSoundVolume,128,1000,0);
						objDeath->AnimDeath((pFirstUnitType+iType)->wavDeath);
						return 0;
					};
					if (bGoBerz)
					{
						G_Obj->pLink->pEffQue=new clEffQue(2,10,G_Obj);
						G_Obj->pLink->pEffQue->iNum=3;
						G_Obj->pLink->iTimer=15;
						G_Obj->pLink->pEffQue->iColor=makecol(250,150,20);
						G_Obj->pLink->pEffQue->wavSound=wavBerz;						
					};
				};
			}
			else
			{
				if ((Spell->arEffect[2][i]==-1)&&((pUnit->GetUnitKind()==1)||(((pUnit->GetUnitKind()==3)||(pUnit->GetUnitKind()==5))&&(Spell->iLifeCost==0))))
				{
			//Лечение
					Hit=SpellPower;
					if (Hit>GetLife()-iLife)
					{
						Hit=GetLife()-iLife;
					};
					G_Obj->AnimNumber(Hit, makecol(60,230,20));
					iLife+=Hit;
				};
			};
			break;
		case 10:
			if (Spell->bNegative)
			{
				if (Spell->arEffect[2][i]==-1)
				{
			//Сжигание снарядов
					Hit=GetHit(SpellPower,Resist*Spell->bResistPower);
					if (iAmmo<Hit)
					{
						Hit=iAmmo;
					};
					if (Hit>0)
					{
						iAmmo-=Hit;						
						if (Hit>iLife)
						{Hit=iLife;};
						G_Obj->AnimNumber(Hit,makecol(250,50,180));
						if (TakeDamage(Hit,2))
						{
					//		play_sample((pFirstUnitType+pUnit->ipType)->wavDeath,iSoundVolume,128,1000,0);
							objDeath->AnimDeath((pFirstUnitType+iType)->wavDeath);
							return 0;
						};
						if (bGoBerz)
						{
							G_Obj->pLink->pEffQue=new clEffQue(2,10,G_Obj);
							G_Obj->pLink->pEffQue->iNum=3;
							G_Obj->pLink->iTimer=15;
							G_Obj->pLink->pEffQue->iColor=makecol(250,150,20);
							G_Obj->pLink->pEffQue->wavSound=wavBerz;
						};
					};													
				}
				else
				{
			//Постоянный эффект
				};
			}
			else
			{
				if (Spell->arEffect[2][i]==-1)
				{
			//Пополнение снарядов					
					if ((Spell->bRestoreOnly)&&(SpellPower>GetAmmo()-iAmmo))
					{
						SpellPower=GetAmmo()-iAmmo;
						if (SpellPower<0)
						{SpellPower=0;};
					};
					iAmmo+=SpellPower;
					if (SpellPower>0)
					{
						G_Obj->AnimNumber(SpellPower,makecol(250,250,150));									
					};
				}
				else
				{
			//Постоянный эффект
				};
			};
			break;
		case 11:
			if (Spell->bNegative)
			{
				if (Spell->arEffect[2][i]==-1)
				{
			//Понижение энергии
					Hit=SpellPower-Resist*Spell->bResistPower;
					if (Hit>0)
					{UseStamina(Hit);}
					else
					{Hit=0;};
					G_Obj->AnimNumber(Hit,makecol(120,130,30));									
				};				
			}
			else
			{
				if (Spell->arEffect[2][i]==-1)
				{
			//Повышение энергии					
					if ((Spell->bRestoreOnly)&&(SpellPower>GetStamina()-iStamina))
					{
						SpellPower=GetStamina()-iStamina;
						if (SpellPower<0)
						{SpellPower=0;};
					};
					iStamina+=SpellPower;
					if (Spell->arEffect[0][0]==12)
					{G_Obj->AnimNumber(SpellPower,makecol(250,250,150),15);}
					else
					{G_Obj->AnimNumber(SpellPower,makecol(250,250,150));};
				};
			};
			break;
		case 12:
			if (Spell->bNegative)
			{
				if (Spell->arEffect[2][i]==-1)
				{
			//Страх
					Hit=SpellPower-Resist*Spell->bResistPower;
					if (Hit>0)
					{ChangeMorale(-Hit);};
					G_Obj->AnimNumber(Hit,makecol(0,50,180));									
				}
				else
				{
			//Постоянный эффект
				};
			}
			else
			{
				if (Spell->arEffect[2][i]==-1)
				{
			//Воодушевление	
					if (Spell->bRestoreOnly)
					{
						if (SpellPower>GetMorale()-iMorale)
						{SpellPower=GetMorale()-iMorale;};
						if (SpellPower<0)
						{SpellPower=0;};
					};
					ChangeMorale(SpellPower);
					G_Obj->AnimNumber(SpellPower,makecol(60,60,250));									
				}
				else
				{
			//Постоянный эффект
				};

			};
			break;
		case 31:
		//Снятие чар

			SpellPower+=Hero->CheckSkill(908);
			/*if (Hero->pHero)
			{
				SpellPower+=Hero->pHero->CheckHeroBonus(908);
			};*/
			if (Caster==iSide)
			//С дружественного юнита
			{
				Eff=pEffect;				
				bool bDestroy=bDemo;
				while (Eff)
				{
					bDestroy=false;
					if ((Eff->pSpell)&&(Eff->pSpell->bNegative)&&(Eff->iType!=73)&&(Eff->pSpell->arEffect[0][0]!=30))
					{						
						if (Eff->iTime>0)
						{
							Eff->iTime-=SpellPower;
							if (Eff->iTime<=0)
							{
								bDestroy=true;
							};
						};						
					};
					if (bDestroy)
					{
						if (Eff->iType==90)
						{
							iLife=iLife*pTrueForm->GetLife()/pUnit->GetLife();
							if (iLife<=0)
							{iLife=1;};	
							delete pUnit;
							pUnit=pTrueForm;
							if (iLife>GetLife())
							{iLife=GetLife();};
							if (iAmmo>GetAmmo())
							{iAmmo=GetAmmo();};
							if (iSpeed>GetSpeed())
							{iSpeed=GetSpeed();};
							G_Obj->Pic=pUnit->pType()->Pic;
							G_Obj->Shadow=pUnit->pType()->Shadow;
							G_Obj->FlipShadow=pUnit->pType()->FlipShadow;
						};
						Eff1=Eff;
						Eff=Eff->pNext;
						if (Eff1->pPrev)
						{
							delete Eff1;
						}
						else
						{
							pEffect=Eff1->pNext;
							delete Eff1;							
						};							
					}
					else
					{
						Eff=Eff->pNext;
					};
				};
			}
			else
			//С вражеского юнита
			{
				Eff=pEffect;				
				bool bDestroy=bDemo;
				int iChangeSide=-1;
				while (Eff)
				{
					bDestroy=false;
					if ((Eff->pSpell)&&((!Eff->pSpell->bNegative)||(Eff->iType==73))&&(Eff->pSpell->arEffect[0][0]!=30))
					{						
						if (Eff->iTime>0)
						{
							Eff->iTime-=SpellPower;
							if (Eff->iTime<=0)
							{
								bDestroy=true;
								if (Eff->iType==73)
								{
									iChangeSide=Eff->iPower;
								};
							};
						};						
					};
					if (bDestroy)
					{
						if (Eff->iType==90)
						{
							iLife=iLife*pTrueForm->GetLife()/pUnit->GetLife();
							if (iLife<=0)
							{iLife=1;};	
							delete pUnit;
							pUnit=pTrueForm;
							if (iLife>GetLife())
							{iLife=GetLife();};
							if (iAmmo>GetAmmo())
							{iAmmo=GetAmmo();};
							if (iSpeed>GetSpeed())
							{iSpeed=GetSpeed();};
							G_Obj->MoveCargo(objBar,-G_Obj->Pic->w/2+pTrueForm->pType()->Pic->w/2,-G_Obj->Pic->h+pTrueForm->pType()->Pic->h);
							G_Obj->Pic=pUnit->pType()->Pic;
							G_Obj->Shadow=pUnit->pType()->Shadow;
							G_Obj->FlipShadow=pUnit->pType()->FlipShadow;
						};
						Eff1=Eff;
						Eff=Eff->pNext;
						if (Eff1->pPrev)
						{
							delete Eff1;
						}
						else
						{
							pEffect=Eff1->pNext;
							delete Eff1;							
						};
					}
					else
					{
						Eff=Eff->pNext;
					};
				};
			//Повреждения вызванным существам
				if ((bSummoned)&&(SpellPower+Spell->arEffect[1][i]*2>Resist))
				{
					Hit=GetHit(SpellPower+Spell->arEffect[1][i]*2,Resist);
					if (Hit>iLife)
					{Hit=iLife;};
					G_Obj->AnimNumber(Hit,makecol(250,50,180));
					if (TakeDamage(Hit,2))					
					{
				//		play_sample((pFirstUnitType+pUnit->ipType)->wavDeath,iSoundVolume,128,1000,0);
						objDeath->AnimDeath((pFirstUnitType+iType)->wavDeath);
						return 0;
					};
					if (bGoBerz)
					{
						G_Obj->pLink->pEffQue=new clEffQue(2,10,G_Obj);
						G_Obj->pLink->pEffQue->iNum=3;
						G_Obj->pLink->iTimer=15;
						G_Obj->pLink->pEffQue->iColor=makecol(250,150,20);
						G_Obj->pLink->pEffQue->wavSound=wavBerz;
					};
				};
				if (iChangeSide>=0)
				{
					ChangeSide(iChangeSide);
				};
			};
			break;			
		case 47:
		//Паутина
			if ((Resist>3)||(CheckSkill(201)>0))
			{
				G_Obj->AnimText(4, makecol(250,150,20));				
			}
			else
			{
			if (Duration>GetAttack())
				{Duration=3;}
			else
				{Duration=2;};
			{
			bool WillCurse=true;			
			Eff=pEffect;
			while (Eff)
			{
				if ((Eff->pAbility==(pFirstUnitUpg+77))||(Eff->pSpell==Spell))
				{				
					Eff->iTime++;				
					WillCurse=false;
				};
				Eff=Eff->pNext;
			};
			if (WillCurse)
			{
				pEffect=new BF_Effect(pEffect, 36, 1, Duration);
				pEffect->bVisible=true;
				pEffect->pSpell=Spell;
				pEffect=new BF_Effect(pEffect, 4, 1, Duration);			
				pEffect->pSpell=Spell;
				pEffect=new BF_Effect(pEffect, 5, 1, Duration);			
				pEffect->pSpell=Spell;
				pEffect=new BF_Effect(pEffect, 38, 1, Duration);			
				pEffect->pSpell=Spell;
				iSpeed=0;
				bCanAct=false;
				G_Obj->iWeb=1;
			};					
			};
			};
			break;
		case 70:
			bDestroyOnHit=true;
			break;
		case 71:
			bLifeDrain=true;
			break;
		case 72:
			bFriendlyFire=true;
			break;
		case 73:
		//Подчинение
			if (Spell->arEffect[2][i]==-1)
			{
				if (SpellPower>=Resist)
				{
					int iNum=0, iNewNum;
					BF_Effect *Eff=pEffect, *Eff1;					
					while (Eff)
					{
						Eff1=Eff->pNext;
						if (Eff->iType==73)
						{
							if (Eff->pPrev==NULL)
							{
								pEffect=Eff->pNext;
							};
							delete Eff;
						};						
						Eff=Eff1;
					};
					for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
					{
						if (Squad[iSide][i].pUnit==pUnit)
						{
							iNum=i;
						};
					};					
					iNewNum=MAX_SQUAD;
					while (Squad[Caster][iNewNum].pUnit)
					{iNewNum++;};
					if (!CheckSkill(36))
					{
						bCanAct=true;
						iSpeed=GetSpeed();
						if (iStamina<1)
						{iStamina=1;};			
						bMarch=true;			
						if (iMorale<=0)
						{
							ChangeMorale(1);
							iFear=0;
						};
					};
					ChangeSide(iNewNum);
			//		Squad[Caster][iNewNum]=Squad[iSide][iNum];
			//		Squad[Caster][iNewNum].iSide=Caster;
					Squad[Caster][iNewNum].pEffect=new BF_Effect(Squad[Caster][iNewNum].pEffect,73,iNum,1);
					Squad[Caster][iNewNum].pEffect->pSpell=Spell;
					Squad[Caster][iNewNum].pEffect->bVisible=true;
			//		Squad[Caster][iNewNum].G_Obj->bFlip=!Squad[Caster][iNewNum].G_Obj->bFlip;
			//		Clear();
				}
				else
				{
					G_Obj->AnimText(4, makecol(250,150,20));
				};
			}
			else
			{
				Duration-=Resist*Spell->bResistDuration;
				if (Duration>0)
				{
					int iNum=0, iNewNum;
					BF_Effect *Eff=pEffect, *Eff1;					
					while (Eff)
					{
						Eff1=Eff->pNext;
						if (Eff->iType==73)
						{
							if (Eff->pPrev==NULL)
							{
								pEffect=Eff->pNext;
							};
							delete Eff;
						};						
						Eff=Eff1;
					};
					for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
					{
						if (Squad[iSide][i].pUnit==pUnit)
						{
							iNum=i;
						};
					};					
					iNewNum=MAX_SQUAD;
					while (Squad[Caster][iNewNum].pUnit)
					{iNewNum++;};
					if (!CheckSkill(36))
					{
						bCanAct=true;
						iSpeed=GetSpeed();
						if (iStamina<1)
						{iStamina=1;};
						bMarch=true;
						if (iMorale<=0)
						{
							ChangeMorale(1);
							iFear=0;
						};
					};
					ChangeSide(iNewNum);			
					Squad[Caster][iNewNum].pEffect=new BF_Effect(Squad[Caster][iNewNum].pEffect,73,iNum,Duration);
					Squad[Caster][iNewNum].pEffect->pSpell=Spell;
					Squad[Caster][iNewNum].pEffect->bVisible=true;
				}
				else
				{
					G_Obj->AnimText(4, makecol(250,150,20));
				};
			};			
			break;
		case 75:
		//Дополнительный ход
			if (!CheckSkill(36))
			{
				bCanAct=true;
				if (!CheckSkill(88))
				{
					iSpeed=GetSpeed();
					bMarch=true;
				};
				if (iStamina<1)
				{
					iStamina=1;
					bResting=false;
				};
				if (iMorale<=0)
				{
					if (iFear<60)
					{
						if (bSimulate)
						{
							if (iFear<30)
							{
								ChangeMorale(1);
								iFear=0;
							}
							else
							{
								iFear-=10;
								Panic();
							};
						}
						else
						{
						if (Random(100)>iFear)
						{
							ChangeMorale(1);
							iFear=0;
						}
						else
						{
							iFear-=10;
							Panic();
						};
						};
					}
					else
					{
						iFear-=10;
						Panic();
					};
				};
			};
			break;
		case 92:
		//Суицид
			if (Resist*Spell->bResistPower<=SpellPower)
			{
				Hit=GetMeleeHit(this,this,true)*2;			
				if (Hit>iLife)
				{Hit=iLife;};
				Vampire=0;
				HitEffect(this,this,Hit,true,false);
				G_Obj->AnimNumber(Hit,makecol(250,50,180));
				UseStamina(1);					
				if (TakeDamage(Hit,2))
				{
			//		play_sample((pFirstUnitType+pUnit->ipType)->wavDeath,iSoundVolume,128,1000,0);
					objDeath->AnimDeath((pFirstUnitType+iType)->wavDeath);
					return 0;
				}
				else
				{
					if (bGoBerz)
					{
						G_Obj->pLink->pEffQue=new clEffQue(2,10,G_Obj);
						G_Obj->pLink->pEffQue->iNum=3;
						G_Obj->pLink->iTimer=15;
						G_Obj->pLink->pEffQue->iColor=makecol(250,150,20);
						G_Obj->pLink->pEffQue->wavSound=wavBerz;
					};
				};
			}
			else
			{
				G_Obj->AnimText(4, makecol(250,150,20));
			};
			break;
		case 93:
		//Снятие яда
			Eff=pEffect;								
			while (Eff)
			{
				Eff1=Eff->pNext;
				if ((Eff->pAbility)&&((Eff->pAbility->arUpg[0][0]==41)||(Eff->pAbility->arUpg[0][0]==78)))
				{
					if (!Eff->pPrev)
					{pEffect=Eff1;};
					delete Eff;
				};
				Eff=Eff1;		
			};
			break;
		default:
		//Enchant
			if (Index<0)
			{Index=-Index;};
			if ((Spell->bNegative)&&(Caster!=iSide))
			{
				Duration-=Resist*Spell->bResistDuration;
				/*int Sign=1;
				if (SpellPower<0){Sign=-1;};
				if (abs(SpellPower)<(Resist*Spell->bResistPower+GetDefence()*Spell->bDefencePower))
				{
					SpellPower=0;
				}
				else
				{
					SpellPower=abs(SpellPower)-(Resist*Spell->bResistPower+GetDefence()*Spell->bDefencePower);
					SpellPower*=Sign;
				};*/
			};			
			if ((Duration>0)&&(SpellPower!=0))
			{
				if ((Spell==(pFirstSpell+40))&&(G_Obj->iIce==0))
				{G_Obj->iIce=1;};
				if (!Spell->bCumulative)
				{
					BF_Effect *Eff=pEffect;
					bool bFound=false;
					while (Eff)
					{
						if ((Eff->pSpell==Spell)&&(Eff->iType==Spell->arEffect[0][i]))
						{
							bFound=true;
							if((Duration>Eff->iTime)||(abs(SpellPower)>abs(Eff->iPower)))
							{
								Eff->iTime=Duration;
								Eff->iPower=SpellPower;
							};
						};
						if ((Spell->arEffect[0][i]>1000)&&(Eff->iType==90))
						{
							bFound=true;
							if((Duration>Eff->iTime)||(abs(SpellPower)>abs(Eff->iPower)))
							{
								Eff->iTime=Duration;
								dcUnit *NewUnit=new dcUnit();							
								NewUnit->SetUnitType(Spell->arEffect[0][i]-1000);
								iLife=NewUnit->GetLife()*iLife/pUnit->GetLife();
								iSpeed=NewUnit->GetSpeed()*iSpeed/pUnit->GetSpeed();
								if (iLife<=0)
								{iLife=1;};
								delete pUnit;
								pUnit=NewUnit;
								G_Obj->Pic=(pFirstUnitType+Spell->arEffect[0][i]-1000)->Pic;
							};
						};
						Eff=Eff->pNext;
					};
					if (!bFound)
					{						
						pEffect=new BF_Effect(pEffect, Spell->arEffect[0][i],SpellPower,Duration);
						pEffect->pSpell=Spell;
						pEffect->bDestroyOnHit=bDestroyOnHit;

						if (Spell->arEffect[0][i]==7)
						{
							if ((SpellPower>0)&&(bCanAct)&&(CheckSkill(88)==0)&&(CheckSkill(36)==0))
							{
								iSpeed+=SpellPower;
								if (iSpeed>GetSpeed())
								{
									iSpeed=GetSpeed();
								};
							};
							if (SpellPower<0)
							{
								if (iSpeed>0)
								{
									iSpeed+=SpellPower;
									if (iSpeed<1)
									{
										iSpeed=1;
									};								
								};
							};
						};
					
						if (Spell->arEffect[0][i]==36)
						{iSpeed=0;};
						if (Spell->arEffect[0][i]==38)
						{bCanAct=false;};
						//Полиморф
						if (Spell->arEffect[0][i]>1000)
						{
							pEffect->iType=90;
							dcUnit *NewUnit=new dcUnit(Spell->arEffect[0][i]-1000,0);							
							iLife=NewUnit->GetLife()*iLife/pUnit->GetLife();
							int iSpeed=pUnit->GetSpeed();
							if (iSpeed<1)
							{iSpeed=1;}
							iSpeed=NewUnit->GetSpeed()*iSpeed/iSpeed;
							iAmmo=NewUnit->GetAmmo();
							if (iLife<=0)
							{iLife=1;};
							pUnit=NewUnit;
							if (iLife>GetLife())
							{iLife=GetLife();};
							G_Obj->Pic=(pFirstUnitType+Spell->arEffect[0][i]-1000)->Pic;
							G_Obj->Shadow=pUnit->pType()->Shadow;
							G_Obj->FlipShadow=pUnit->pType()->FlipShadow;
							G_Obj->MoveCargo(objBar,G_Obj->Pic->w/2-pTrueForm->pType()->Pic->w/2,G_Obj->Pic->h-pTrueForm->pType()->Pic->h);
						};

						if ((i==0)||((bDestroyOnHit)&&(i==1)))
						{
							pEffect->bVisible=true;
						}
						else
						{
							pEffect->bVisible=false;
						};
					};
				}
				else
				{
					if (Spell->arEffect[0][i]==7)
					{
						if (SpellPower>0)
						{
							iSpeed+=SpellPower;
						};
						if (SpellPower<0)
						{
							if (iSpeed>0)
							{
								iSpeed+=SpellPower;
								if (iSpeed<1)
								{
									iSpeed=1;
								};
							};
						};
					};
					pEffect=new BF_Effect(pEffect, Spell->arEffect[0][i],SpellPower,Duration);
					pEffect->pSpell=Spell;
					pEffect->bDestroyOnHit=bDestroyOnHit;
					if (Spell->arEffect[0][i]==36)
					{iSpeed=0;};
					if (Spell->arEffect[0][i]==38)
					{bCanAct=false;};
					if (i==0)
					{
						pEffect->bVisible=true;
					}
					else
					{
						pEffect->bVisible=false;
					};
				};				
			}
			else
			{
				G_Obj->AnimText(4, makecol(250,150,20));
			}
			break;
		};
	};
	DrawBar();
	return iLife;
};

void BF_Unit::Heal(BF_Unit *Target)
{
	int Heal=CheckSkill(24);
	if (!bSimulate)
	{Heal+=1-Random(3);};
	if (Heal>(Target->GetLife()-Target->iLife))
	{Heal=Target->GetLife()-Target->iLife;};
	iDamageHealed+=Heal;
	Target->iLife+=Heal;
	if (bShowAnim)
	{
		Target->G_Obj->AnimNumber(Heal, makecol(60,230,20));
		PlayUnitSound(wawHeal);
	//	play_sample(wawHeal,iSoundVolume,128,1000,0);
		G_Obj->Make_Stars(G_Obj,20,10,10,0,24,40,250,250,30);
		Target->G_Obj->Make_Stars(Target->G_Obj,50,40,10,0,44,40,60,250,30);
	};
	if (pUnit->pHero)
	{
		if ((pUnit->pHero->DamageWeapon(1))&&(bShowAnim))
		{
			G_Obj->AnimText(371,makecol(220,150,20));
			PlayUnitSound(wavWeapon);
		//	play_sample(wavWeapon,iSoundVolume,128,1000,0);
		};
	};
	UseStamina(1);
	iAmmo--;
	iSpeed=0;
	bCanAct=0;
	DrawBar();
	Target->DrawBar();
};

void BF_Unit::RaiseCorpse(BF_Unit *Enemy)
{
	StupidLogger(objMain,"***RaiseBegin***");
	if ((Enemy->iLife>0)||(Enemy->GetUnitKind()==2)||(Enemy->GetUnitKind()==4)||((pFirstBF_Obj+arGrid[Enemy->x][Enemy->y].Land)->iMoveCost<=0))
	{return;};
	for (int i=MAX_SQUAD; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		if (Squad[iSide][i].pUnit==NULL)
		{
			int Lvl;
			if ((pHero)&&(pHero->ipClass1>0))
			{Lvl=pHero->CheckHeroBonus(907)+pHero->CheckHeroBonus(909);}
			else
			{Lvl=pUnit->iLevel/4;};
			objMain=Squad[iSide][i].Summon(19,Lvl,Enemy->x,Enemy->y,objMain);
			Squad[iSide][i].iSide=iSide;
			if (iSide)
			{
				Squad[iSide][i].G_Obj->bFlip=true;
			}
			else
			{
				Squad[iSide][i].G_Obj->bFlip=false;
			};
			Enemy->bInBattle=false;
			if (bShowAnim)
			{
				Enemy->G_Obj->bInvisOnDeath=true;
			};
		//	Enemy->x=10;
		//	Enemy->y=10;
		//	Enemy->G_Obj->iLayer=-1;
			break;
		};
	};
	StupidLogger(objMain,"***RaiseEnd***");
};

void BF_Unit::Shoot(BF_Unit *Enemy)
{
	StupidLogger(objMain,"***ShootBerin***");
	int Hit;
	int nShots=1;
	if (buttDoubleShot.bPress)
	{
		nShots++;
	};
	int Wait=0;
	if (nShots>iAmmo)
	{nShots=iAmmo;};

	//Окаменение
	if (CheckSkill(46))
	{
	if (bShowAnim)
	{
		PlayUnitSound(pUnit->pType()->wavShoot);
	//	play_sample((pFirstUnitType+pUnit->ipType)->wavShoot,iSoundVolume,128,1000,0);
	};
	if ((CheckSkill(46)>Enemy->GetResist())&&(Enemy->CheckSkill(202)==0))
	{
		bool WillCurse=true;
		BF_Effect * Eff=Enemy->pEffect;
		while (Eff)
		{
			if (Eff->pAbility==(pFirstUnitUpg+76))
			{
				if (Eff->iTime<CheckSkill(46)-Enemy->GetResist())
				{
					Eff->iTime=CheckSkill(46)-Enemy->GetResist();
				};
				WillCurse=false;
			};
			Eff=Eff->pNext;
		};
		if (WillCurse)
		{
			Enemy->pEffect=new BF_Effect(Enemy->pEffect, 36, 1, CheckSkill(46)-Enemy->GetResist());
			Enemy->pEffect->bVisible=true;
			Enemy->pEffect->pAbility=(pFirstUnitUpg+76);
			Enemy->pEffect=new BF_Effect(Enemy->pEffect, 4, 3, CheckSkill(46)-Enemy->GetResist());			
			Enemy->pEffect->pAbility=(pFirstUnitUpg+76);
			Enemy->pEffect=new BF_Effect(Enemy->pEffect, 5, 3, CheckSkill(46)-Enemy->GetResist());			
			Enemy->pEffect->pAbility=(pFirstUnitUpg+76);
			Enemy->pEffect=new BF_Effect(Enemy->pEffect, 38, 1, CheckSkill(46)-Enemy->GetResist());			
			Enemy->pEffect->pAbility=(pFirstUnitUpg+76);
			Enemy->pEffect=new BF_Effect(Enemy->pEffect, 6, 3, CheckSkill(46)-Enemy->GetResist());			
			Enemy->pEffect->pAbility=(pFirstUnitUpg+76);		
			Enemy->iSpeed=0;
			Enemy->bCanAct=false;
		};
//		if (Enemy->G_Obj->pStars)
//		{
//			delete Enemy->G_Obj->pStars;
//		};
		Enemy->G_Obj->Make_Stars(Enemy->G_Obj,30,50,10,0,44,40,200,200,200);
		if (bShowAnim)
		{
			PlayUnitSound(pUnit->pType()->wavShootHit);
		//	play_sample((pFirstUnitType+pUnit->ipType)->wavShootHit,iSoundVolume,128,1000,0);
			if (Enemy->G_Obj->bmpStone==NULL)
			{Enemy->G_Obj->StoneForm();};
		};
	}
	else
	{
		Enemy->G_Obj->AnimText(4, makecol(250,150,20));
	};
//		if (G_Obj->pStars)
//		{
//			delete G_Obj->pStars;
//		};
		G_Obj->Make_Stars(G_Obj,20,20,10,0,44,40,200,200,200);
		iAmmo--;
		nShots=0;
	};

	//Паутина
	if (CheckSkill(47))
	{	
		
		if (bShowAnim)
		{
			PlayUnitSound(pUnit->pType()->wavShoot);
		//	play_sample((pFirstUnitType+pUnit->ipType)->wavShoot,iSoundVolume,128,1000,0);
		};
		if (Enemy->CheckSkill(201)==0)
		{
		int EffTime=2;
		if (CheckSkill(47)>Enemy->GetAttack())
		{EffTime=3;};
		bool WillCurse=true;
		BF_Effect * Eff=Enemy->pEffect;
		while (Eff)
		{
			if ((Eff->pAbility==(pFirstUnitUpg+77))||(Eff->pSpell==(pFirstSpell+18)))
			{				
				Eff->iTime++;				
				WillCurse=false;
			};
			Eff=Eff->pNext;
		};
		if (WillCurse)
		{
			Enemy->pEffect=new BF_Effect(Enemy->pEffect, 36, 1, EffTime);
			Enemy->pEffect->bVisible=true;
			Enemy->pEffect->pAbility=(pFirstUnitUpg+77);
			Enemy->pEffect=new BF_Effect(Enemy->pEffect, 4, 1, EffTime);			
			Enemy->pEffect->pAbility=(pFirstUnitUpg+77);
			Enemy->pEffect=new BF_Effect(Enemy->pEffect, 5, 1, EffTime);			
			Enemy->pEffect->pAbility=(pFirstUnitUpg+77);		
			Enemy->pEffect=new BF_Effect(Enemy->pEffect, 38, 1, EffTime);			
			Enemy->pEffect->pAbility=(pFirstUnitUpg+77);		
			Enemy->iSpeed=0;
			Enemy->bCanAct=false;
			Enemy->G_Obj->iWeb=1;
		};		
//		if (Enemy->G_Obj->pStars)
//		{
//			delete Enemy->G_Obj->pStars;
//		};
		if (bShowAnim)
		{
			PlayUnitSound(pUnit->pType()->wavShootHit);
		//	play_sample((pFirstUnitType+pUnit->ipType)->wavShootHit,iSoundVolume,128,1000,0);
		};
		Enemy->G_Obj->Make_Stars(Enemy->G_Obj,30,50,10,0,44,40,200,200,200);		
//		if (G_Obj->pStars)
//		{
//			delete G_Obj->pStars;
//		};
		G_Obj->Make_Stars(G_Obj,20,20,10,0,44,40,200,200,200);
		}
		else
		{Enemy->G_Obj->AnimText(4, makecol(250,150,20));}
		iAmmo--;
		nShots=0;
	};

	while (nShots)
	{
		Hit=GetRangedHit(this,Enemy);
		/*if (CheckSkill(60)>Enemy->GetResist())
		{
			Hit+=CheckSkill(60)-Enemy->GetResist();
		};*/
	
		if (Enemy->iLife<Hit)
		{
			Hit=Enemy->iLife;
		};
		iRangeDamageDone+=Hit;
		iAmmo--;
		if (pUnit->pHero)
		{
			if ((pUnit->pHero->DamageWeapon(1))&&(bShowAnim))
			{
				G_Obj->AnimText(371,makecol(220,150,20));
				PlayUnitSound(wavWeapon);
			//	play_sample(wavWeapon,iSoundVolume,128,1000,0);
			};
		};
		if (Enemy->pUnit->pHero)
		{
			if ((Enemy->pUnit->pHero->DamageArmor(1))&&(bShowAnim))
			{
				Enemy->G_Obj->AnimText(372,makecol(220,150,20));
				PlayUnitSound(wavArmor);
			//	play_sample(wavArmor,iSoundVolume,128,1000,0);
			//	clEffQue *Eff=G_Obj->pLink->AddAttackEnemyEffect(2,20);
			//	Eff->iNum=372;
			//	Eff->iColor=makecol(250,150,20);				
			};
		};
		int MissSpeed=15+arHumans[arPlayer[ActivePlayer].iColor].iBattleSpeed*3;
		if ((pHero)&&(pHero->ipClass1>0)&&(pHero->arInventory[1][0]>0))
		{
			if ((pFirstItem+pHero->arInventory[1][0])->ipClass==3)
			{G_Obj->AnimShoot(Enemy->G_Obj,Hit,Enemy->iLife-Hit,Wait*10,(pFirstUnitType+41),Enemy->pUnit->pType(),(pFirstUnitType+41)->iMissile,MissSpeed);}
			else
			{G_Obj->AnimShoot(Enemy->G_Obj,Hit,Enemy->iLife-Hit,Wait*10,(pFirstUnitType+43),Enemy->pUnit->pType(),(pFirstUnitType+43)->iMissile,MissSpeed);};
		}
		else
		{
			G_Obj->AnimShoot(Enemy->G_Obj,Hit,Enemy->iLife-Hit,Wait*10,pUnit->pType(),Enemy->pUnit->pType(),pUnit->pType()->iMissile,MissSpeed);
		};
		ShootEffect(this,Enemy,Hit);
		nShots--;
		Wait++;
		int Typo=1;
		if (CheckSkill(28))
		{Typo=2;};
		if (Enemy->TakeDamage(Hit, Typo))
		{
			nShots=0;
			iEnemyShotted++;
			ChangeMorale(1);
			if ((CheckSkill(64)>0)&&(Enemy->pUnit)&&(!Enemy->bSummoned))
			{RaiseCorpse(Enemy);};
		};
		if ((bGoBerz)&&(bShowAnim))
		{
			clEffQue *Eff=G_Obj->pLink->AddAttackEnemyEffect(2,20);
			Eff->iNum=3;
			Eff->iColor=makecol(250,150,20);
			Eff->wavSound=wavBerz;
		};
	};
	if (iSpeed<GetSpeed())
		{UseStamina(2);}
	else
		{UseStamina(1);};	
	if (buttDoubleShot.bPress)
	{
		UseStamina(CheckSkill(20));
	};
	iSpeed=0;
	bCanAct=false;
	DrawBar();
	Enemy->DrawBar();
	StupidLogger(objMain,"***ShootEnd***");
};

int BF_Unit::SoulSteal(BF_Unit *Enemy)
{
	int Heal=0;
	if ((Enemy->pUnit)&&(CheckSkill(57)>0)&&(iLife<GetLife())&&(Enemy->pUnit->GetUnitKind()!=2)&&(Enemy->pUnit->GetUnitKind()!=4))
	{
		Heal=CheckSkill(57);
		if (Heal>GetLife()-iLife)
		{Heal=GetLife()-iLife;};
		iLife+=Heal;		
	};
	return Heal;
};

int BF_Unit::Attack(int x1, int y1, BF_Unit *Enemy)
{
	int Hit, AttBonus;
	bCanAct=false;
	AttBonus=0;
	if ((x1!=x)||(y1!=y))
	{
//атака с разбега	
		if (CheckSkill(37))
		{
			AttBonus=abs(x-Enemy->x)+abs(y-Enemy->y)-2;
			if (AttBonus<0)
			{AttBonus=0;};
	//		G_Obj->AnimNumber(AttBonus,makecol(200,250,30));
		};
		Move(x1,y1);
	};
//первый удар у противника
	if ((Enemy->CheckSkill(16))&&(!CheckSkill(16))&&(!CheckSkill(26))&&(!Enemy->CheckSkill(38))&&(!Enemy->bResting))
	{
		if (Enemy->iStamina>0)
		{
			Hit=GetMeleeHit(Enemy,this,0);
			if (iLife<Hit)
			{
				Hit=iLife;
			};			
			Enemy->iMeleeDamageDone+=Hit;
			Enemy->UseStamina(1);
			Enemy->bCanAct=false;
	/*		if ((Enemy->CheckSkill(68))&&((pFirstUnitType+pUnit->ipType)->iUnitKind!=2)
				&&((pFirstUnitType+pUnit->ipType)->iUnitKind!=4)&&(Enemy->iLife<Enemy->GetLife()))
			{

			};
			*/
			G_Obj->AnimAttack(Enemy->G_Obj,Hit,0,iLife-Hit,Enemy->pUnit->pType(),pUnit->pType());
			HitEffect(Enemy, this, Hit, false,true);
			/*if (pUnit->pHero)
			{
				pUnit->pHero->DamageArmor(1);
			};
			if (Enemy->pUnit->pHero)
			{
				Enemy->pUnit->pHero->DamageWeapon(1);
			};*/
			if (pUnit->pHero)
				{
					if ((pUnit->pHero->DamageArmor(1))&&(bShowAnim))
					{
						clEffQue *Eff;					
						Eff=G_Obj->AddAttackUnitEffect(2,20);
						Eff->iColor=makecol(250,150,20);
						Eff->iNum=372;
						Eff->wavSound=wavArmor;
					};
				};
				if (Enemy->pUnit->pHero)
				{
					if ((Enemy->pUnit->pHero->DamageWeapon(1))&&(bShowAnim))
					{
						clEffQue *Eff;					
						Eff=G_Obj->AddAttackEnemyEffect(2,20);
						Eff->iColor=makecol(250,150,20);
						Eff->iNum=371;
						Eff->wavSound=wavWeapon;
					};
				};
			TakeDamage(Hit, 0);
			if ((bGoBerz)&&(bShowAnim))
			{
				clEffQue *Eff=G_Obj->AddAttackUnitEffect(2,20);
				Eff->iNum=3;
				Eff->iColor=makecol(250,150,20);
				Eff->wavSound=wavBerz;
			};
		}
		if (iLife==0)
		{
			Enemy->ChangeMorale(1);
			Enemy->iEnemySlayed++;
			//Похищение души
			int Heal=Enemy->SoulSteal(this);
			if ((bShowAnim)&&(Heal>0))
			{
				clEffQue *Eff;					
				Eff=G_Obj->AddAttackEnemyEffect(1,0);
				Eff->iColor=makecol(20,220,50);
				Eff->iNum=Heal;
				Eff->wavSound=wavSoul;
			};
			if ((Enemy->CheckSkill(64)>0)&&(pUnit)&&(!bSummoned))
			{Enemy->RaiseCorpse(this);};
			DrawBar();
			Enemy->DrawBar();
			return 0;
		}
		else
		{		
			Hit=GetMeleeHit(this,Enemy,1);
			Hit+=AttBonus;
			Enemy->bParry=false;
			if (Enemy->iLife<Hit)
			{
				Hit=Enemy->iLife;
			};
			//Топчет		
			if ((Enemy->iLife-Hit<CheckSkill(63))&&(Enemy->CheckSkill(14)==0)&&(CheckSkill(88)==0))
			{
				Hit=Enemy->iLife;
				if ((Enemy->CheckSkill(74)==0)&&(Enemy->CheckSkill(91)==0))
				{
					arGrid[x][y].Pass=1;
					x=Enemy->x;
					y=Enemy->y;
				};
			};
			iMeleeDamageDone+=Hit;			
			if (iSpeed<GetSpeed())
			{UseStamina(2);}
			else
			{UseStamina(1);};			
			if (buttRoundAtt.bPress)
			{UseStamina(CheckSkill(66));};
			G_Obj->AnimAttack(Enemy->G_Obj,Hit,1,Enemy->iLife-Hit,pUnit->pType(),Enemy->pUnit->pType());
			HitEffect(this, Enemy, Hit, true, true);
			if (buttCrush.bPress)
			{
				UseStamina(CheckSkill(59));
				if (bShowAnim)
				{
					clEffQue *Eff;					
					Eff=G_Obj->AddAttackUnitEffect(2,0);
					Eff->iColor=makecol(220,220,50);
					Eff->iNum=69;
					Eff->wavSound=wavCrush;
				};
			};
			//Топчет		
			if ((Enemy->iLife==Hit)&&(CheckSkill(63)>0)&&(bShowAnim)&&(Enemy->CheckSkill(74)==0)&&(Enemy->CheckSkill(91)==0)&&(Enemy->CheckSkill(14)==0)&&(CheckSkill(88)==0))
			{			
				clAttackAnim *Att=G_Obj->pAttack;
				while (Att->pNext)
				{
					Att=Att->pNext;
				};
				Att->bTrample=true;
				Att->x2=TransX(x,y,G_Obj->Pic->w);
				Att->y2=TransY(x,y,G_Obj->Pic->h);
			};
			if (pUnit->pHero)
			{
				if ((pUnit->pHero->DamageWeapon(1))&&(bShowAnim))
				{
					clEffQue *Eff;					
					Eff=G_Obj->AddAttackUnitEffect(2,0);
					Eff->iColor=makecol(250,150,20);
					Eff->iNum=371;
					Eff->wavSound=wavWeapon;
				};
			};
			if (Enemy->pUnit->pHero)
			{
				if ((Enemy->pUnit->pHero->DamageArmor(1))&&(bShowAnim))
				{
					clEffQue *Eff;					
					Eff=G_Obj->AddAttackEnemyEffect(2,20);
					Eff->iColor=makecol(250,150,20);
					Eff->iNum=372;
					Eff->wavSound=wavArmor;
				};
			};
			if (Enemy->TakeDamage(Hit,0))
			{			
				iEnemySlayed++;
				ChangeMorale(1);
				arGrid[x][y].Pass=0;
				//Похищение души
				int Heal=SoulSteal(Enemy);					
				if ((bShowAnim)&&(Heal>0))
				{
					clEffQue *Eff;					
					Eff=G_Obj->AddAttackUnitEffect(1,0);
					Eff->iColor=makecol(20,220,50);
					Eff->iNum=Heal;
					Eff->wavSound=wavSoul;
				};
				if ((CheckSkill(64)>0)&&(Enemy->pUnit)&&(!Enemy->bSummoned))
				{RaiseCorpse(Enemy);};
			};
			if ((bGoBerz)&&(bShowAnim))
			{
				clEffQue *Eff=G_Obj->AddAttackEnemyEffect(2,20);
				Eff->iNum=3;
				Eff->iColor=makecol(250,150,20);
				Eff->wavSound=wavBerz;
			};
		};
	}
	else
//первый удар у этого юнита
	{
		Hit=GetMeleeHit(this,Enemy,1);
		Hit+=AttBonus;
		Enemy->bParry=false;
		if (Enemy->iLife<Hit)
		{
			Hit=Enemy->iLife;
		};
		//Топчет
		if ((Enemy->iLife-Hit<CheckSkill(63))&&(Enemy->CheckSkill(14)==0)&&(CheckSkill(88)==0))
		{
			Hit=Enemy->iLife;
			if ((Enemy->CheckSkill(74)==0)&&(Enemy->CheckSkill(91)==0))
			{
				arGrid[x][y].Pass=1;	
				x=Enemy->x;
				y=Enemy->y;
			};
		};
		iMeleeDamageDone+=Hit;		
		if (iSpeed<GetSpeed())
		{UseStamina(2);}
		else
		{UseStamina(1);};
		if (buttRoundAtt.bPress)
		{UseStamina(CheckSkill(66));};
		G_Obj->AnimAttack(Enemy->G_Obj,Hit,1,Enemy->iLife-Hit,pUnit->pType(),Enemy->pUnit->pType());
		HitEffect(this, Enemy, Hit, true, true);
		if (buttCrush.bPress)
		{
			UseStamina(CheckSkill(59));
			if (bShowAnim)
			{
				clEffQue *Eff;					
				Eff=G_Obj->AddAttackUnitEffect(2,0);
				Eff->iColor=makecol(220,220,50);
				Eff->iNum=69;
				Eff->wavSound=wavCrush;
			};
		};
		if ((Enemy->iLife==Hit)&&(CheckSkill(63)>0)&&(bShowAnim)&&(Enemy->CheckSkill(74)==0)&&(Enemy->CheckSkill(91)==0)&&(Enemy->CheckSkill(14)==0)&&(CheckSkill(88)==0))
		{			
			clAttackAnim *Att=G_Obj->pAttack;
			while (Att->pNext)
			{
				Att=Att->pNext;
			};
			Att->bTrample=true;
			Att->x2=TransX(x,y,G_Obj->Pic->w);
			Att->y2=TransY(x,y,G_Obj->Pic->h);
		};
		if (pUnit->pHero)
		{
			if ((pUnit->pHero->DamageWeapon(1))&&(bShowAnim))
			{
				clEffQue *Eff;					
				Eff=G_Obj->AddAttackUnitEffect(2,0);
				Eff->iColor=makecol(250,150,20);
				Eff->iNum=371;
				Eff->wavSound=wavWeapon;
			};
		};
		if (Enemy->pUnit->pHero)
		{
			if ((Enemy->pUnit->pHero->DamageArmor(1))&&(bShowAnim))
			{
				clEffQue *Eff;					
				Eff=G_Obj->AddAttackEnemyEffect(2,20);
				Eff->iColor=makecol(250,150,20);
				Eff->iNum=372;
				Eff->wavSound=wavArmor;
			};
		};

		if (Enemy->TakeDamage(Hit,0))
		{			
			iEnemySlayed++;
			ChangeMorale(1);
			arGrid[x][y].Pass=0;
			//Похищение души
			int Heal=SoulSteal(Enemy);				
			if ((bShowAnim)&&(Heal>0))
			{
				clEffQue *Eff;					
				Eff=G_Obj->AddAttackUnitEffect(1,0);
				Eff->iColor=makecol(20,220,50);
				Eff->iNum=Heal;
				Eff->wavSound=wavSoul;
			};
			if ((CheckSkill(64)>0)&&(Enemy->pUnit)&&(!Enemy->bSummoned))
			{RaiseCorpse(Enemy);};
		}
		else
		{
			if ((bGoBerz)&&(bShowAnim))
			{
				clEffQue *Eff=G_Obj->AddAttackEnemyEffect(2,20);
				Eff->iNum=3;
				Eff->iColor=makecol(250,150,20);
				Eff->wavSound=wavBerz;
			};
			if ((Enemy->iStamina>0)&&(!CheckSkill(26))&&(!Enemy->CheckSkill(38))&&(!Enemy->bResting))
			{			

				Hit=GetMeleeHit(Enemy,this,0);
				if (iLife<Hit)
				{
					Hit=iLife;
				};				
				Enemy->iMeleeDamageDone+=Hit;
				Enemy->UseStamina(1);
				Enemy->bCanAct=false;
				G_Obj->AnimAttack(Enemy->G_Obj,Hit,0,iLife-Hit,Enemy->pUnit->pType(),pUnit->pType());
				HitEffect(Enemy, this, Hit, false, true);
				/*if (pUnit->pHero)
				{
					pUnit->pHero->DamageArmor(1);
				};
				if (Enemy->pUnit->pHero)
				{
					Enemy->pUnit->pHero->DamageWeapon(1);
				};*/

				if (pUnit->pHero)
				{
					if ((pUnit->pHero->DamageArmor(1))&&(bShowAnim))
					{
						clEffQue *Eff;					
						Eff=G_Obj->AddAttackUnitEffect(2,20);
						Eff->iColor=makecol(250,150,20);
						Eff->iNum=372;
						Eff->wavSound=wavArmor;
					};
				};
				if (Enemy->pUnit->pHero)
				{
					if ((Enemy->pUnit->pHero->DamageWeapon(1))&&(bShowAnim))
					{
						clEffQue *Eff;					
						Eff=G_Obj->AddAttackEnemyEffect(2,20);
						Eff->iColor=makecol(250,150,20);
						Eff->iNum=371;
						Eff->wavSound=wavWeapon;
					};
				};


				if (TakeDamage(Hit,0))
				{			
					Enemy->ChangeMorale(1);
					Enemy->iEnemySlayed++;
					//Похищение души
					int Heal=Enemy->SoulSteal(this);						
					if ((bShowAnim)&&(Heal>0))
					{
						clEffQue *Eff;					
						Eff=G_Obj->AddAttackEnemyEffect(1,0);
						Eff->iColor=makecol(20,220,50);
						Eff->iNum=Heal;
						Eff->wavSound=wavSoul;
					};
					if ((Enemy->CheckSkill(64)>0)&&(pUnit)&&(!bSummoned))
					{Enemy->RaiseCorpse(this);};
					DrawBar();
					Enemy->DrawBar();
					return 0;
				};
				if ((bGoBerz)&&(bShowAnim))
				{
					clEffQue *Eff=G_Obj->AddAttackUnitEffect(2,20);
					Eff->iNum=3;
					Eff->iColor=makecol(250,150,20);
					Eff->wavSound=wavBerz;
				};
			};
		};
	};	
	iSpeed=0;
	DrawBar();
	Enemy->DrawBar();
	return 1;
};

void BF_Unit::GoMarch()
{
	UseStamina(CheckSkill(29));
	bMarch=false;
	iSpeed++;
	G_Obj->AnimText(67,COLOR_2);
	if (bShowAnim)
	{
		PlayUnitSound(wavMarch);
	//	play_sample(wavMarch,iSoundVolume,128,1000,0);
	};
	DrawBar();	
};

void BF_Unit::Replace(int x1, int y1)
{
	arGrid[x][y].Pass=1;	
	x=x1;
	y=y1;
	G_Obj->x=TransX(x,y,G_Obj->Pic->w);
	G_Obj->y=TransY(x,y,G_Obj->Pic->h);
	G_Obj->fX=G_Obj->x;
	G_Obj->fY=G_Obj->y;
	clPath *P1, *P2;
	if (G_Obj->Track)
	{
		P1=G_Obj->Track;
		while (P1)
		{
			P2=P1->pPrev;
			delete P1;
			P1=P2;
		};
		G_Obj->Track=NULL;
	};
	arGrid[x][y].Pass=0;
};

//Возвращает изменение атаки в % в зависимости от текущего боевого духа
int GetMoraleBonus(int Morale)
{
	int Result=0;
	if (Morale<6)
	{Result=Morale*10-60;};
	if (Morale>15)
	{
		Morale=Morale-15;
		int Bonus=1;
		while (Morale>=Bonus)
		{
			Result++;
			Morale-=Bonus;
			Bonus++;
		};
		Result*=5;
	};
	return Result;
};

int BF_Unit::GetLife()
{
	int Result=pUnit->GetLife();
	//Result+=CheckEffect(1);
	if ((!bSummoned)&&(Heroes[iSide])){Result+=Heroes[iSide]->CheckUnitBonus(1);};
	Result=Result*LifeMod[iSide]/100;
	if (Result<1)
	{Result=1;};
	return Result;
};

int BF_Unit::GetAttack()
{
	if (CheckSkill(38))
	{return 0;};
	int Result=pUnit->GetAttack();
	Result+=CheckEffect(2);
	if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0)){Result+=Heroes[iSide]->CheckUnitBonus(2);};
	Result*=100;	
	if (((iLife*2<GetLife())&&(!CheckSkill(13))))
	{
		Result-=Result*((GetLife()/2-iLife)*100/GetLife())/100;
	};
	if (iStamina<6)
	{
		Result-=Result*(6-iStamina)/10;
	};	
	Result/=100;
	Result+=Result*GetMoraleBonus(iMorale)/100;
	if (Result<1)
	{
		Result=1;
	};
	return Result;
};

int BF_Unit::GetCounterAttack()
{
	if (CheckSkill(38))
	{return 0;};
	int Result=pUnit->GetCounterAttack();
	Result+=CheckEffect(3);
	if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0)){Result+=Heroes[iSide]->CheckUnitBonus(3);};
	if ((!bSimulate)&&(iLife>0))
	{
	if ((!CheckSkill(14))&&(!bSimulate))
	{
		Result+=(pFirstBF_Obj+arGrid[x][y].Land)->iCounterAttack;
	};
	for (int i=0; i<3; i++)
	{
		if (((arGrid[x][y].Land)==2+i)&&(CheckSkill(32+i)))
		{
			Result+=(CheckSkill(32+i)-CheckSkill(32+i)%8)/8;
		};
	};
	};
	Result*=100;	
	if ((iLife*2<GetLife())&&(!CheckSkill(13)))
	{
		Result-=Result*((GetLife()/2-iLife)*100/GetLife())/100;
	};
	if (iStamina<6)
	{
		Result-=Result*(6-iStamina)/10;
	};	
	Result/=100;
	Result+=Result*GetMoraleBonus(iMorale)/100;
	if (Result<1)
	{
		Result=1;
	};
	return Result;
};

int BF_Unit::GetDefence()
{
	int Result=pUnit->GetDefence();
	Result+=CheckEffect(4);	
	if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0)){Result+=Heroes[iSide]->CheckUnitBonus(4);};
	if ((!bSimulate)&&(iLife>0))
	{
	if ((!CheckSkill(14))&&(!bSimulate))
	{
		Result+=(pFirstBF_Obj+arGrid[x][y].Land)->iDefence;
	};
	for (int i=0; i<3; i++)
	{
		if (((arGrid[x][y].Land)==2+i)&&(CheckSkill(32+i)))
		{
			Result+=(CheckSkill(32+i)-CheckSkill(32+i)%4)/4;
		};
	};
	};
	if (iStamina==0)
	{Result/=2;};
	if (Result<0)
	{Result=0;};
	return Result;
};

int BF_Unit::GetRangedDefence()
{
	int Result=pUnit->GetRangedDefence();
	Result+=CheckEffect(5);	
	if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0)){Result+=Heroes[iSide]->CheckUnitBonus(5);};
	if ((!bSimulate)&&(iLife>0))
	{
	if ((!CheckSkill(14))&&(!bSimulate))
	{
		Result+=(pFirstBF_Obj+arGrid[x][y].Land)->iRangedDefence;
		if ((bCastle)&&(x>5))
		{Result+=3;};
	};
	for (int i=0; i<3; i++)
	{
		if (((arGrid[x][y].Land)==2+i)&&(CheckSkill(32+i)))
		{
			Result+=(CheckSkill(32+i)-CheckSkill(32+i)%8)/8;
		};
	};
	};
	if (iStamina==0)
	{Result/=2;};
	if (Result<0)
	{Result=0;};
	return Result;
};

int BF_Unit::GetResist()
{
	int Result=pUnit->GetResist();
	Result+=CheckEffect(6);
	if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0)){Result+=Heroes[iSide]->CheckUnitBonus(6);};
	if (Result<0)
	{Result=0;};
	return Result;
};

int BF_Unit::GetSpeed()
{
	int Result=pUnit->GetSpeed();
	if (Result==0)
	{return 0;}
	Result+=CheckEffect(7);
	if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0)){Result+=Heroes[iSide]->CheckUnitBonus(7);};
	if ((iStamina<5)&&(Result>1))
	{
		Result--;
	};
	if ((iStamina<3)&&(Result>1))
	{
		Result--;
	};
	if (Result<1)
	{
		Result=1;
	};	
	if (Result<0)
	{Result=0;};
	return Result;
};

int BF_Unit::GetFatigure()
{
	int Result=pUnit->GetSpeed();
	Result+=CheckEffect(7);
	if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0)){Result+=Heroes[iSide]->CheckUnitBonus(7);};
	if ((iStamina<5)&&(Result>1))
	{
		Result--;
	};
	if ((iStamina<3)&&(Result>1))
	{
		Result--;
	};
	if (Result<1)
	{Result=1;}
	else
	{Result=0;};
	
	return Result;
};

int BF_Unit::GetRangedAttack()
{
//	if (CheckSkill(38))
//	{return 0;};
	int Result=pUnit->GetRangedAttack();
	if (Result==0)
	{
		return Result;
	};
	if ((!CheckSkill(14))&&(!bSimulate)&&(iLife>0))
	{		
		if ((bCastle)&&(x>5))
		{Result++;};
	};
	Result+=CheckEffect(8);
	if (((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0))&&(CheckSkill(24)==0)&&(pUnit->GetUnitClass()==2)){Result+=Heroes[iSide]->CheckUnitBonus(8);};
	Result*=100;	
	if ((iLife*2<GetLife())&&(!CheckSkill(13)))
	{
		Result-=Result*((GetLife()/2-iLife)*100/GetLife())/100;
	};
	if (iStamina<6)
	{
		Result-=Result*(6-iStamina)/10;
	};	
	Result/=100;
	Result+=Result*GetMoraleBonus(iMorale)/100;
	if (Result<1)
	{
		Result=1;
	};	
	return Result;
};

int BF_Unit::GetShootingRange()
{
	int Result=pUnit->GetShootingRange();
	if (Result>1)
	{
		if ((!CheckSkill(14))&&(!bSimulate)&&(iLife>0))
		{
			Result+=(pFirstBF_Obj+arGrid[x][y].Land)->iShootingRange;			
			if ((bCastle)&&(x>5))
			{Result++;};	
		};
		Result+=CheckEffect(9);
		if (((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0))&&(CheckSkill(24)==0)&&(pUnit->GetUnitClass()==2)){Result+=Heroes[iSide]->CheckUnitBonus(9);};
	};
	if (Result<0)
	{Result=0;};
	return Result;
};

int BF_Unit::GetAmmo()
{
	if ((pUnit->GetRangedAttack()<1)&&(CheckSkill(24)==0)&&(CheckSkill(46)==0)&&(CheckSkill(47)==0)&&(!CheckCaster()))
	{return 0;};
	int Result=pUnit->GetAmmo();
	if (Result>0)
	{
		Result+=CheckEffect(10);
		if ((Result>1)&&(pUnit->GetUnitClass()==2))
		{
			if (((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0))&&(CheckSkill(24)==0)&&(pUnit->GetRangedAttack()>0)){Result+=Heroes[iSide]->CheckUnitBonus(10);};
		};
	};	
	if (Result<0)
	{Result=0;};
	return Result;
};

int BF_Unit::GetStamina()
{
	int Result=pUnit->GetStamina();
	//Result+=CheckEffect(11);	
	if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0)){Result+=Heroes[iSide]->CheckUnitBonus(11);};
	if (Result<0)
	{Result=0;};
	return Result;
};

int BF_Unit::GetMorale()
{
	int Result=pUnit->GetMorale();
	Result+=CheckEffect(12);
	if ((CheckSkill(19)==0)&&(!bSummoned))
	{
		if (Heroes[iSide])
		{
			if ((Heroes[iSide])&&(Squad[iSide][0].iLife>0))
			{
				Result+=Heroes[iSide]->CheckUnitBonus(12);
			};		
			Result+=Heroes[iSide]->GetSquadMorale(iNumm);		
		};
		if (iSide)
		{Result+=ProvinceMoraleBonus;};
	};
	if (Result<1)
	{Result=1;};
	return Result;
};

int BF_Unit::GetExp()
{
	int Result=pUnit->GetExp();	
	return Result;
};

int BF_Unit::GetRace()
{
	int Result=pUnit->GetRace();	
	return Result;
};

int	BF_Unit::GetUnitKind()
{
	int Result=pUnit->GetUnitKind();	
	return Result;
};

int	BF_Unit::GetUnitClass()
{
	int Result=pUnit->GetUnitClass();	
	return Result;
};

int BF_Unit::GetKarma()
{
	int Result=pUnit->GetKarma();	
	return Result;
};

BF_Unit *pUnitHint=NULL;

void BF_Unit::MakeParamHint(int Param)
{
	if ((HintType!=7)||(HintMember!=Param)||(pUnitHint!=this))
	{
		HintType=7; HintMember=Param; pUnitHint=this;
		rectfill(bmpHint,0,0,MAX_HINT_W,MAX_HINT_H,COLOR_0);
		HintW=bmpGUI_ParamHint->w;
		HintH=62;
		blit(bmpGUI_ParamHint,bmpHint,0,0,0,0,HintW,HintH);
		int Work, Current, Current1;
		switch (Param)
		{
		case 1:	//жизнь		
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+1)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, GetLife());
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, iLife);
			break;
		case 2: //атака
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+2)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, pUnit->GetAttack());
			if (CheckSkill(38))
			{
				if (pUnit->GetAttack()>0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: -%d",(pFirstPhrase+252)->str, pUnit->GetAttack());
					HintH+=20;
				};
			}
			else
			{
				Work=CheckEffect(2);
				Current=pUnit->GetAttack()+Work;
				if (Work>0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+252)->str, Work);
					HintH+=20;
				};
				if (Work<0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+252)->str, Work);
					HintH+=20;
				};
				if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0))
				{
					Work=Heroes[iSide]->CheckUnitBonus(2);
					if (Work>0)
					{
						Current+=Work;
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+11)->str, Work);
						HintH+=20;
					};
				};
				Current*=100;
				Current1=Current;
				if (((iLife*2<GetLife())&&(!CheckSkill(13))))
				{
					Current-=Current*((GetLife()/2-iLife)*100/GetLife())/100;
					Work=Current1/100-Current/100;
					if (Work>0)
					{
						Current1=Current/100;
						Current1*=100;
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: -%d",(pFirstPhrase+498)->str, Work);
						HintH+=20;
					};
				};
				if (iStamina<6)
				{
					Current-=Current*(6-iStamina)/10;
					Work=Current1/100-Current/100;
					if (Work>0)
					{						
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: -%d",(pFirstPhrase+499)->str, Work);
						HintH+=20;
					};
				};
				Current/=100;
				Work=Current*GetMoraleBonus(iMorale)/100;				
				if (Work>0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstAbilityNum+12)->sName, Work);
					HintH+=20;
				};
				if (Work<0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstAbilityNum+12)->sName, Work);
					HintH+=20;
				};
			};
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, GetAttack());
			break;
		case 3: //контратака
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+3)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, pUnit->GetCounterAttack());
			if (CheckSkill(38))
			{
				if (pUnit->GetCounterAttack()>0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: -%d",(pFirstPhrase+252)->str, pUnit->GetCounterAttack());
					HintH+=20;
				};
			}
			else
			{
				Work=CheckEffect(3);
				Current=pUnit->GetCounterAttack()+Work;
				if (Work>0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+252)->str, Work);
					HintH+=20;
				};
				if (Work<0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+252)->str, Work);
					HintH+=20;
				};
				if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0))
				{
					Work=Heroes[iSide]->CheckUnitBonus(3);
					if (Work>0)
					{
						Current+=Work;
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+11)->str, Work);
						HintH+=20;
					};
				};
				Work=0;
				if ((!bSimulate)&&(iLife>0))
				{
					if (!CheckSkill(14))
					{
						Work=(pFirstBF_Obj+arGrid[x][y].Land)->iCounterAttack;
					};
					for (int i=0; i<3; i++)
					{
						if (((arGrid[x][y].Land)==2+i)&&(CheckSkill(32+i)))
						{
							Work+=(CheckSkill(32+i)-CheckSkill(32+i)%8)/8;
						};
					};
				};
				if (Work>0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+500)->str, Work);
					HintH+=20;
				};
				if (Work<0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+500)->str, Work);
					HintH+=20;
				};
				Current*=100;
				Current1=Current;
				if (((iLife*2<GetLife())&&(!CheckSkill(13))))
				{
					Current-=Current*((GetLife()/2-iLife)*100/GetLife())/100;
					Work=Current1/100-Current/100;
					if (Work>0)
					{
						Current1=Current/100;
						Current1*=100;
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: -%d",(pFirstPhrase+498)->str, Work);
						HintH+=20;
					};
				};
				if (iStamina<6)
				{
					Current-=Current*(6-iStamina)/10;
					Work=Current1/100-Current/100;
					if (Work>0)
					{						
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: -%d",(pFirstPhrase+499)->str, Work);
						HintH+=20;
					};
				};
				Current/=100;
				Work=Current*GetMoraleBonus(iMorale)/100;				
				if (Work>0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstAbilityNum+12)->sName, Work);
					HintH+=20;
				};
				if (Work<0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstAbilityNum+12)->sName, Work);
					HintH+=20;
				};
			};
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, GetCounterAttack());
			break;
		case 4: //защита
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+4)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, pUnit->GetDefence());
			Work=CheckEffect(4);
			Current=pUnit->GetDefence()+Work;
			if (Work>0)
			{
				blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
				textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+252)->str, Work);
				HintH+=20;
			};
			if (Work<0)
			{
				blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
				textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+252)->str, Work);
				HintH+=20;
			};
			if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0))
			{
				Work=Heroes[iSide]->CheckUnitBonus(4);
				if (Work>0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+11)->str, Work);
					HintH+=20;
				};
			};
			Work=0;
			if ((!bSimulate)&&(iLife>0))
			{
				if (!CheckSkill(14))
				{
					Work=(pFirstBF_Obj+arGrid[x][y].Land)->iDefence;
				};
				for (int i=0; i<3; i++)
				{
					if (((arGrid[x][y].Land)==2+i)&&(CheckSkill(32+i)))
					{
						Work+=(CheckSkill(32+i)-CheckSkill(32+i)%4)/4;
					};
				};			
				if (Work>0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+500)->str, Work);
					HintH+=20;
				};
				if (Work<0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+500)->str, Work);
					HintH+=20;
				};
			};			
			Current1=Current;
			if (iStamina==0)
			{
				Current/=2;
				Work=Current1-Current;
				if (Work>0)
				{						
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: -%d",(pFirstPhrase+499)->str, Work);
					HintH+=20;
				};
			};
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, GetDefence());
			break;
		case 5: //защита от выстрела
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+5)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, pUnit->GetRangedDefence());
			Work=CheckEffect(5);
			Current=pUnit->GetRangedDefence()+Work;
			if (Work>0)
			{
				blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
				textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+252)->str, Work);
				HintH+=20;
			};
			if (Work<0)
			{
				blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
				textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+252)->str, Work);
				HintH+=20;
			};
			if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0))
			{
				Work=Heroes[iSide]->CheckUnitBonus(5);
				if (Work>0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+11)->str, Work);
					HintH+=20;
				};
			};
			Work=0;
			if ((!bSimulate)&&(iLife>0))
			{
				if (!CheckSkill(14))
				{
					Work=(pFirstBF_Obj+arGrid[x][y].Land)->iRangedDefence;
					if ((bCastle)&&(x>5))
					{Work+=3;};
				};
				for (int i=0; i<3; i++)
				{
					if (((arGrid[x][y].Land)==2+i)&&(CheckSkill(32+i)))
					{
						Work+=(CheckSkill(32+i)-CheckSkill(32+i)%8)/8;
					};
				};			
				if (Work>0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+500)->str, Work);
					HintH+=20;
				};
				if (Work<0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+500)->str, Work);
					HintH+=20;
				};
			};			
			Current1=Current;
			if (iStamina==0)
			{
				Current/=2;
				Work=Current1-Current;
				if (Work>0)
				{						
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: -%d",(pFirstPhrase+499)->str, Work);
					HintH+=20;
				};
			};
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, GetRangedDefence());
			break;
		case 6: //сопротивление
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+6)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, pUnit->GetResist());
			Work=CheckEffect(6);
			Current=pUnit->GetResist()+Work;
			if (Work>0)
			{
				blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
				textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+252)->str, Work);
				HintH+=20;
			};
			if (Work<0)
			{
				blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
				textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+252)->str, Work);
				HintH+=20;
			};
			if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0))
			{
				Work=Heroes[iSide]->CheckUnitBonus(6);
				if (Work>0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+11)->str, Work);
					HintH+=20;
				};
			};			
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, GetResist());
			break;
		case 7: //скорость
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+7)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, pUnit->GetSpeed());
			Work=CheckEffect(7);
			Current=pUnit->GetSpeed()+Work;
			if (Work>0)
			{
				blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
				textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+252)->str, Work);
				HintH+=20;
			};
			if (Work<0)
			{
				blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
				textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+252)->str, Work);
				HintH+=20;
			};			
			if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0))
			{
				Work=Heroes[iSide]->CheckUnitBonus(7);
				if (Work>0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+11)->str, Work);
					HintH+=20;
				};
			};
			Work=0;
			if ((iStamina<5)&&(Current>1))
			{
				Work--;
				Current--;
			};
			if ((iStamina<3)&&(Current>1))
			{
				Work--;
				Current--;
			};
			if (Work<0)
			{						
				blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
				textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+499)->str, Work);
				HintH+=20;
			};	
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, iSpeed);
			break;
		case 8: //Дистанционная атака
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+8)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, pUnit->GetRangedAttack());
			if (pUnit->GetRangedAttack()>0)
			{
			/*if (CheckSkill(38))
			{
				blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
				textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: -%d",(pFirstPhrase+252)->str, pUnit->GetRangedAttack());
				HintH+=20;
			}
			else*/
			{
				Work=CheckEffect(8);
				Current=pUnit->GetRangedAttack()+Work;
				if (Work>0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+252)->str, Work);
					HintH+=20;
				};
				if (Work<0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+252)->str, Work);
					HintH+=20;
				};
				if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0)&&(pUnit->GetUnitClass()==2))
				{
					Work=Heroes[iSide]->CheckUnitBonus(8);
					if (Work>0)
					{
						Current+=Work;
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+11)->str, Work);
						HintH+=20;
					};
				};
				Work=0;
				if ((!bSimulate)&&(iLife>0))
				{
					if (!CheckSkill(14))
					{
						if ((bCastle)&&(x>5))
						{Work++;};						
					};					
				};
				if (Work>0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+500)->str, Work);
					HintH+=20;
				};
				if (Work<0)
				{
					Current+=Work;
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+500)->str, Work);
					HintH+=20;
				};
				Current*=100;
				Current1=Current;
				if (((iLife*2<GetLife())&&(!CheckSkill(13))))
				{
					Current-=Current*((GetLife()/2-iLife)*100/GetLife())/100;
					Work=Current1/100-Current/100;
					if (Work>0)
					{
						Current1=Current/100;
						Current1*=100;
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: -%d",(pFirstPhrase+498)->str, Work);
						HintH+=20;
					};
				};
				if (iStamina<6)
				{
					Current-=Current*(6-iStamina)/10;
					Work=Current1/100-Current/100;
					if (Work>0)
					{						
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: -%d",(pFirstPhrase+499)->str, Work);
						HintH+=20;
					};
				};
				Current/=100;
				Work=Current*GetMoraleBonus(iMorale)/100;				
				if (Work>0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstAbilityNum+12)->sName, Work);
					HintH+=20;
				};
				if (Work<0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstAbilityNum+12)->sName, Work);
					HintH+=20;
				};
			};
			};
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, GetRangedAttack());
			break;
		case 9: //дальность выстрела
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+9)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, pUnit->GetShootingRange());			
			Current=pUnit->GetShootingRange();
			if (Current>1)
			{
				Work=CheckEffect(9);
				Current+=Work;
				if (Work>0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+252)->str, Work);
					HintH+=20;
				};
				if (Work<0)
				{
					blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
					textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+252)->str, Work);
					HintH+=20;
				};
				if ((!bSummoned)&&(Heroes[iSide])&&(Squad[iSide][0].iLife>0)&&(pUnit->GetUnitClass()==2))
				{
					Work=Heroes[iSide]->CheckUnitBonus(9);
					if (Work>0)
					{
						Current+=Work;
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+11)->str, Work);
						HintH+=20;
					};
				};
				Work=0;
				if ((!bSimulate)&&(iLife>0))
				{
					if (!CheckSkill(14))
					{
						Work=(pFirstBF_Obj+arGrid[x][y].Land)->iShootingRange;
						if ((bCastle)&&(x>5))
						{Work++;};
					};							
					if (Work>0)
					{
						Current+=Work;
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(20,220,20),-1,"%s: +%d",(pFirstPhrase+500)->str, Work);
						HintH+=20;
					};
					if (Work<0)
					{
						Current+=Work;
						blit(bmpGUI_ParamHint,bmpHint,0,HintH,0,HintH,HintW,20);
						textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH+3,makecol(220,0,0),-1,"%s: %d",(pFirstPhrase+500)->str, Work);
						HintH+=20;
					};
				};
			};				
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, GetShootingRange());
			break;
		case 10: //снаряды			
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+10)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, GetAmmo());
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, iAmmo);
			break;
		case 11: //выносливость
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+11)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, GetStamina());
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, iStamina);
			break;
		case 12: //боевой дух
			textprintf_centre_ex(bmpHint,Font1,HintW/2,16,COLOR_1,-1,(pFirstAbilityNum+12)->sName);
			textprintf_centre_ex(bmpHint,Font1,HintW/2,45,COLOR_2,-1,"%s: %d",(pFirstPhrase+496)->str, GetMorale());
			blit(bmpGUI_ParamHint,bmpHint,0,bmpGUI_ParamHint->h-29,0,HintH,HintW,29);
			HintH+=29;
			textprintf_centre_ex(bmpHint,Font1,HintW/2,HintH-22,COLOR_2,-1,"%s: %d",(pFirstPhrase+497)->str, iMorale);
			break;
		};
	};
};

//***************************************************
//*** Окно информации о юните ***********************
//***************************************************
void BF_Unit::ShowInfoScreen()
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_UnitInfo->w/2;
	int y=384-bmpGUI_UnitInfo->h/2;	
	const int w=59;
	const int h=31;
	const int h1=24;
	const int dx=60;
	const int dy=56;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);	
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpBack);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_BattleUnitInfo->w,bmpGUI_BattleUnitInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_BattleUnitInfo,bmpWindow,0,0,0,0,bmpGUI_BattleUnitInfo->w,bmpGUI_BattleUnitInfo->h);
	set_trans_blender(140,140,140,140);
	draw_trans_sprite(bmpWindow,pUnit->pType()->Shadow,254-pUnit->pType()->Pic->w/2,100-pUnit->pType()->Pic->h/2);
	draw_sprite(bmpWindow,pUnit->pType()->Pic,254-pUnit->pType()->Pic->w/2,100-pUnit->pType()->Pic->h/2);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+336,y+442,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	UI_Mask arMedalMask[3];
	int Medals=0;

	//Награды
	for (int i=0; i<3; i++)
	{
		if (pUnit->arMedal[i]>0)
		{
			draw_sprite(bmpWindow,(pFirstMedal+pUnit->arMedal[i])->Pic,323,91+i*60);
			WObj=arMedalMask[i].Init(x+317,y+85+i*60,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
			Medals++;
		};
	};

	UI_TextBox TextBox;
	WObj=TextBox.Init(x+407,y+73,303,324,2,COLOR_1,Font2,WObj,542+x,44+y,542+x,410+y);	

	//Параметры
	if (pUnit->pHero)
	{
		draw_sprite(bmpWindow,ClassPic[pUnit->pHero->ipClass1-1],323,91);
		char Txt[2000];
		sprintf(Txt,pUnit->pType()->sText);
		if (pUnit->pHero->iLevel>=10)
		{
			draw_sprite(bmpWindow,ClassPic[3+(pUnit->pHero->ipClass1-1)*4+pUnit->pHero->ipClass2],323,91+60);
			sprintf(Txt,(pFirstMessage+52)->msg,Txt,ClassText[3+(pUnit->pHero->ipClass1-1)*4+pUnit->pHero->ipClass2]);
		};
		if (pUnit->pHero->iLevel>=20)
		{
			draw_sprite(bmpWindow,ClassPic[19+(pUnit->pHero->ipClass1-1)*4+pUnit->pHero->ipClass2],323,91+120);
			sprintf(Txt,(pFirstMessage+52)->msg,Txt,ClassText[19+(pUnit->pHero->ipClass1-1)*4+pUnit->pHero->ipClass2]);
		};
		TextBox.Activate(Txt);
		textprintf_centre_ex(bmpWindow,Font1,370,17,COLOR_1,-1,"%s %s",pUnit->pHero->pstName->str,ClassName[pUnit->pHero->GetClassName()].str);	
//		textprintf_centre_ex(bmpWindow,Font1,133,53,makecol(220,220,0),-1,"%s",(pFirstPhrase+11)->str);
		int Limit=(pUnit->pHero->iExp-pUnit->pHero->ExpForLevel(pUnit->pHero->iLevel))*58/
			(pUnit->pHero->ExpForLevel(pUnit->pHero->iLevel+1)-pUnit->pHero->ExpForLevel(pUnit->pHero->iLevel));
		if (Limit>58)
		{Limit=58;}	
	//	textprintf_centre_ex(bmpWindow,Font1,234,53,makecol(220,220,0),-1,"%s %d",(pFirstPhrase+5)->str,pUnit->pHero->iLevel);
		for (int i=0; i<Limit; i++)
		{
			rectfill(bmpWindow,319+i,65,320+i,83,makecol(40+i*2,10+i/2,50+i*2.5));
		};
		textprintf_centre_ex(bmpWindow,Font1,348,71,COLOR_1,-1,"%d",pUnit->pHero->iLevel);
		textprintf_centre_ex(bmpWindow,Font1,347,70,makecol(220,220,0),-1,"%d",pUnit->pHero->iLevel);
	}
	else
	{
		textprintf_centre_ex(bmpWindow,Font1,370,17,COLOR_1,-1,"%s (%s)",pUnit->pType()->sName,(pFirstPhrase+303+pUnit->GetKarma())->str);		
		int Limit=(pUnit->iExp-pUnit->ExpForLevel(pUnit->iLevel))*58/(pUnit->ExpForLevel(pUnit->iLevel+1)-pUnit->ExpForLevel(pUnit->iLevel));
		if (Limit>58)
		{Limit=58;}
		for (int i=0; i<Limit; i++)
		{
			rectfill(bmpWindow,319+i,65,320+i,83,makecol(40+i*2,10+i/2,50+i*2.5));
		};
		textprintf_centre_ex(bmpWindow,Font1,348,71,COLOR_1,-1,"%d",pUnit->iLevel);
		textprintf_centre_ex(bmpWindow,Font1,347,70,makecol(220,220,0),-1,"%d",pUnit->iLevel);
		TextBox.Activate(pUnit->pType()->sText);
	};

	int Limit=iLife*100/GetLife();
	if (Limit>100)
	{Limit=100;}
	for (int i=0; i<Limit; i++)
	{
		rectfill(bmpWindow,113+i,176,114+i,194,makecol(50+i*2,10+i/2,10+i/2));
	};
	textprintf_centre_ex(bmpWindow,Font1,168,181,COLOR_1,-1,"%d",iLife);
	textprintf_centre_ex(bmpWindow,Font1,167,180,COLOR_2,-1,"%d",iLife);

	Limit=iStamina*100/GetStamina();
	if (Limit>100)
	{Limit=100;}
	for (int i=0; i<Limit; i++)
	{
		rectfill(bmpWindow,52+i,176+h1,53+i,194+h1,makecol(50+i*2,50+i*2,10+i/2));
	};
	textprintf_centre_ex(bmpWindow,Font1,106,181+h1,COLOR_1,-1,"%d",iStamina);
	textprintf_centre_ex(bmpWindow,Font1,105,180+h1,COLOR_2,-1,"%d",iStamina);

	Limit=iMorale*100/GetMorale();
	if (Limit>100)
	{Limit=100;}
	for (int i=0; i<Limit; i++)
	{
		rectfill(bmpWindow,52+i,176+h1*2,53+i,194+h1*2,makecol(10+i/2,10+i/2,50+i*2));
	};
	textprintf_centre_ex(bmpWindow,Font1,106,181+h1*2,COLOR_1,-1,"%d",iMorale);
	textprintf_centre_ex(bmpWindow,Font1,105,180+h1*2,COLOR_2,-1,"%d",iMorale);

	if (GetSpeed()>0)
	{
		Limit=iSpeed*100/GetSpeed();
		if (Limit>100)
		{Limit=100;}
		for (int i=0; i<Limit; i++)
		{
			rectfill(bmpWindow,194+i,176+h1,195+i,194+h1,makecol(5+i*2,i*2,10+i*2));
		};
	};
	textprintf_centre_ex(bmpWindow,Font1,248,181+h1,COLOR_1,-1,"%d",iSpeed);
	textprintf_centre_ex(bmpWindow,Font1,247,180+h1,COLOR_2,-1,"%d",iSpeed);

	if (GetAmmo()>0)
	{
		Limit=iAmmo*100/GetAmmo();
		if (Limit>100)
		{Limit=100;}
		for (int i=0; i<Limit; i++)
		{
			rectfill(bmpWindow,194+i,176+h1*2,195+i,194+h1*2,makecol(10+i/2,50+i*2,10+i/2));
		};
	};
	textprintf_centre_ex(bmpWindow,Font1,248,181+h1*2,COLOR_1,-1,"%d",iAmmo);
	textprintf_centre_ex(bmpWindow,Font1,247,180+h1*2,COLOR_2,-1,"%d",iAmmo);

	textprintf_centre_ex(bmpWindow,Font1,dx,dy,makecol(220,220,0),-1,"%d",GetLife());
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy,makecol(220,220,0),-1,"%d",GetStamina());
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy,makecol(220,220,0),-1,"%d",GetMorale());
	textprintf_centre_ex(bmpWindow,Font1,dx,dy+h,makecol(220,220,0),-1,"%d",GetAttack());
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy+h,makecol(220,220,0),-1,"%d",GetCounterAttack());
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy+h,makecol(220,220,0),-1,"%d",GetSpeed());
	textprintf_centre_ex(bmpWindow,Font1,dx,dy+h*2,makecol(220,220,0),-1,"%d",GetDefence());
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy+h*2,makecol(220,220,0),-1,"%d",GetRangedDefence());
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy+h*2,makecol(220,220,0),-1,"%d",GetResist());
	textprintf_centre_ex(bmpWindow,Font1,dx,dy+h*3,makecol(220,220,0),-1,"%d",GetRangedAttack());
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy+h*3,makecol(220,220,0),-1,"%d",GetShootingRange());
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy+h*3,makecol(220,220,0),-1,"%d",GetAmmo());
	
	UI_Mask1 arParamMask[13];
	int arParam[18];
	arParamMask[1].Init(x+18,y+45,58,30,1);arParam[1]=GetLife();
	arParamMask[2].Init(x+18,y+45+h,58,30,1);arParam[2]=GetAttack();
	arParamMask[3].Init(x+18+w,y+45+h,58,30,1);arParam[3]=GetCounterAttack();
	arParamMask[4].Init(x+18,y+45+h*2,58,30,1);arParam[4]=GetDefence();
	arParamMask[5].Init(x+18+w,y+45+h*2,58,30,1);arParam[5]=GetRangedDefence();
	arParamMask[6].Init(x+18+w*2,y+45+h*2,58,30,1);arParam[6]=GetResist();
	arParamMask[7].Init(x+18+w*2,y+45+h,58,30,1);arParam[7]=GetSpeed();
	arParamMask[8].Init(x+18,y+45+h*3,58,30,1);arParam[8]=GetRangedAttack();
	arParamMask[9].Init(x+18+w,y+45+h*3,58,30,1);arParam[9]=GetShootingRange();
	arParamMask[10].Init(x+18+w*2,y+45+h*3,58,30,1);arParam[10]=iAmmo;
	arParamMask[11].Init(x+18+w,y+45,58,30,1);arParam[11]=GetStamina();
	arParamMask[12].Init(x+18+w*2,y+45,58,30,1);arParam[12]=GetMorale();

	UI_Mask1 arParamMask2[5];
	arParamMask2[0].Init(x+91,y+175,150,20,1);
	arParamMask2[1].Init(x+29,y+200,126,20,1);
	arParamMask2[2].Init(x+29,y+224,126,20,1);
	arParamMask2[3].Init(x+170,y+200,126,20,1);
	arParamMask2[4].Init(x+170,y+224,126,20,1);

	//Свойства юнита
	textprintf_centre_ex(bmpWindow,Font1,169,256,COLOR_1,-1,"%s",(pFirstPhrase+84)->str);	
	int iAbil=0, MaxAbilMask=0;
	int arAbil[50];
	for (int i=0; i<50; i++)
	{arAbil[i]=0;};
	if (bSummoned)
	{		
		arAbil[iAbil]=103;
		draw_sprite(bmpWindow,(pFirstAbilityNum+103)->Pic,17+iAbil*60, 281);
		iAbil++;
		MaxAbilMask++;
	};
	for (int i=13; i<3000; i++)
	{
		if ((CheckSkill(i)>0)&&(iAbil<50))
		{			
			for (int j=1; j<=iMaxAbilityNum; j++)
			{
				if ((pFirstAbilityNum+j)->Num==i)
				{
					arAbil[iAbil]=i;
					if (iAbil<5)
					{
						draw_sprite(bmpWindow,(pFirstAbilityNum+j)->Pic,17+iAbil*60, 281);
						if ((pFirstAbilityNum+j)->bNumeric)
							{
								ShowIconNumber(bmpWindow, 17+iAbil*60, 281, CheckSkill(i));
							//	rectfill(bmpWindow,52+i*59,252,72+iAbil*59,267,COLOR_1);
							//	textprintf_centre(bmpWindow,Font1,59+iAbil*60,323,COLOR_2,"%d",CheckSkill(i));
							};
						MaxAbilMask++;
					};
					iAbil++;
					break;
				};
			};
		};
	};
	iAbil--;
	UI_Mask arAbilMask[5];
	for (int i=0; i<MaxAbilMask; i++)
	{
		WObj=arAbilMask[i].Init(x+11+i*60,y+275,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
	};
	int iAbilLine=0;
	UI_Button1 buttScrollUp1;
	WObj=buttScrollUp1.Init(x+313,y+272,1,2,bmpButton[76],bmpButton[77],bmpButton[78],bmpButton[79],WObj);
	buttScrollUp1.Activate(0);	
	UI_Button1 buttScrollDown1;
	WObj=buttScrollDown1.Init(x+313,y+310,1,2,bmpButton[80],bmpButton[81],bmpButton[82],bmpButton[83],WObj);
	if (iAbil<5)
	{buttScrollDown1.Activate(0);}
	else
	{buttScrollDown1.Activate(1);};

	//Активные эффекты
	textprintf_centre_ex(bmpWindow,Font1,169,354,COLOR_1,-1,"%s",(pFirstPhrase+85)->str);
	int iEff=0, MaxEffMask=0;
	BF_Effect *arEff[50];
	for (int i=0; i<50; i++)
	{arEff[i]=NULL;};
	BF_Effect *Eff=pEffect;
	int Spellz=0;
	while (Eff)
	{
		if (Eff->bVisible)
		{
			arEff[Spellz]=Eff;
			if (Spellz<5)
			{
				if (Eff->pSpell)
				{draw_sprite(bmpWindow,Eff->pSpell->Pic,17+Spellz*60,378);};
				if (Eff->pAbility)
				{draw_sprite(bmpWindow,Eff->pAbility->Pic,17+Spellz*60,378);};
				if (Eff->iTime>0)
				{
					ShowIconNumber(bmpWindow, 17+Spellz*60,378,Eff->iTime);
			//		textprintf_centre(bmpWindow,Font1,59+Spellz*60,420,COLOR_2,"%d",Eff->iTime);
				};
				MaxEffMask++;
			};
			Spellz++;
		};
		Eff=Eff->pNext;
	};
	Spellz--;
	UI_Mask arEffMask[5];
	for (int i=0; i<MaxEffMask; i++)
	{
		WObj=arEffMask[i].Init(x+11+i*60,y+372,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
	};
	int iEffLine=0;
	UI_Button1 buttScrollUp2;
	WObj=buttScrollUp2.Init(x+313,y+361,1,2,bmpButton[76],bmpButton[77],bmpButton[78],bmpButton[79],WObj);
	buttScrollUp2.Activate(0);	
	UI_Button1 buttScrollDown2;
	WObj=buttScrollDown2.Init(x+313,y+398,1,2,bmpButton[80],bmpButton[81],bmpButton[82],bmpButton[83],WObj);
	if (Spellz<5)
	{buttScrollDown2.Activate(0);}
	else
	{buttScrollDown2.Activate(1);};

	
	

	while (!buttExit.Check())
	{
		if (buttScrollUp1.Check())
		{
			iAbilLine--;
			if (iAbilLine==0)
			{buttScrollUp1.Activate(0);};
			buttScrollDown1.Activate(1);
			blit(bmpGUI_BattleUnitInfo,bmpWindow,14,278,14,278,298,62);
			for (int i=0; i<5; i++)
			{
				if (arAbil[i+iAbilLine*5]>0)
				{
					for (int j=1; j<=iMaxAbilityNum; j++)
					{
						if ((pFirstAbilityNum+j)->Num==arAbil[i+iAbilLine*5])
						{
							draw_sprite(bmpWindow,(pFirstAbilityNum+j)->Pic,17+i*60, 281);
							if ((pFirstAbilityNum+j)->bNumeric)
							{
								ShowIconNumber(bmpWindow, 17+i*60, 281, CheckSkill(arAbil[i+iAbilLine*5]));
							//	rectfill(bmpWindow,52+i*59,252,72+i*59,267,COLOR_1);
							//	textprintf_centre(bmpWindow,Font1,59+i*60,323,COLOR_2,"%d",CheckSkill(arAbil[i+iAbilLine*5]));
							};
						};
					};
					arAbilMask[i].Activate(2);
				}
				else
				{
					arAbilMask[i].Activate(-1);
				};
			};
		};
		if (buttScrollDown1.Check())
		{
			iAbilLine++;
			if (iAbilLine>=(iAbil-iAbil%5)/5)
			{buttScrollDown1.Activate(0);};
			buttScrollUp1.Activate(1);
			blit(bmpGUI_BattleUnitInfo,bmpWindow,14,278,14,278,298,62);
			for (int i=0; i<5; i++)
			{
				if (arAbil[i+iAbilLine*5]>0)
				{
					for (int j=1; j<=iMaxAbilityNum; j++)
					{
						if ((pFirstAbilityNum+j)->Num==arAbil[i+iAbilLine*5])
						{
							draw_sprite(bmpWindow,(pFirstAbilityNum+j)->Pic,17+i*60, 281);
							if ((pFirstAbilityNum+j)->bNumeric)
							{
								ShowIconNumber(bmpWindow, 17+i*60, 281, CheckSkill(arAbil[i+iAbilLine*5]));
							//	rectfill(bmpWindow,52+i*59,252,72+i*59,267,COLOR_1);
							//	textprintf_centre(bmpWindow,Font1,59+i*60,323,COLOR_2,"%d",CheckSkill(arAbil[i+iAbilLine*5]));
							};
						};
					};
					arAbilMask[i].Activate(2);
				}
				else
				{
					arAbilMask[i].Activate(-1);
				};
			};
		};
		if (buttScrollUp2.Check())
		{
			iEffLine--;
			if (iEffLine==0)
			{buttScrollUp2.Activate(0);};
			buttScrollDown2.Activate(1);
			blit(bmpGUI_BattleUnitInfo,bmpWindow,14,376,14,376,300,62);
			for (int i=0; i<5; i++)
			{
				if (arEff[i+iEffLine*5])
				{
					if (arEff[i+iEffLine*5]->pSpell)
					{draw_sprite(bmpWindow,arEff[i+iEffLine*5]->pSpell->Pic,17+i*60,378);};
					if (arEff[i+iEffLine*5]->pAbility)
					{draw_sprite(bmpWindow,arEff[i+iEffLine*5]->pAbility->Pic,17+i*60,378);};
					if (arEff[i+iEffLine*5]->iTime>0)
					{
						ShowIconNumber(bmpWindow, 17+i*60, 378, arEff[i+iEffLine*5]->iTime);
					//	textprintf_centre(bmpWindow,Font1,59+i*60,420,COLOR_2,"%d",arEff[i+iEffLine*5]->iTime);
					};
					arEffMask[i].Activate(2);
				}
				else
				{
					arEffMask[i].Activate(-1);
				};
			};
		};
		if (buttScrollDown2.Check())
		{
			iEffLine++;
			if (iEffLine>=Spellz/5)
			{buttScrollDown2.Activate(0);};
			buttScrollUp2.Activate(1);
			blit(bmpGUI_BattleUnitInfo,bmpWindow,14,376,14,376,300,62);
			for (int i=0; i<5; i++)
			{
				if (arEff[i+iEffLine*5])
				{
					if (arEff[i+iEffLine*5]->pSpell)
					{draw_sprite(bmpWindow,arEff[i+iEffLine*5]->pSpell->Pic,17+i*60,378);};
					if (arEff[i+iEffLine*5]->pAbility)
					{draw_sprite(bmpWindow,arEff[i+iEffLine*5]->pAbility->Pic,17+i*60,378);};
					if (arEff[i+iEffLine*5]->iTime>0)
					{
						ShowIconNumber(bmpWindow, 17+i*60, 378, arEff[i+iEffLine*5]->iTime);
					//	textprintf_centre(bmpWindow,Font1,59+i*60,420,COLOR_2,"%d",arEff[i+iEffLine*5]->iTime);
					};
					arEffMask[i].Activate(2);
				}
				else
				{
					arEffMask[i].Activate(-1);
				};
			};
		};
		TextBox.Check();
		bHint=false;
		for (int i=0; i<MaxAbilMask; i++)
		{
			int Check=arAbilMask[i].Check();
			if (Check>1)
			{
				bHint=true;
				MakeAbilityHint(arAbil[i+iAbilLine*5]);
			};
			if ((Check==30)||(Check==40))
			{
				bHint=false;
				WObj=ShowActiveObjects(WObj,6,0);
				WindowAbilityInfo(arAbil[i+iAbilLine*5],CheckSkill(arAbil[i+iAbilLine*5]));
			};
		};
		for (int i=0; i<Medals; i++)
		{
			int Check=arMedalMask[i].Check();
			if ((Check==30)||(Check==40))
			{
				bHint=false;
				WObj=ShowActiveObjects(WObj,6,0);
				WindowMedalInfo(pUnit->arMedal[i]);
			};
		};
		for (int i=0; i<MaxEffMask; i++)
		{
			int Check=arEffMask[i].Check();
			if ((Check==30)||(Check==40))
			{
				bHint=false;
				WObj=ShowActiveObjects(WObj,6,0);
				if ((arEff[i+iEffLine*5]->pAbility)&&((arEff[i+iEffLine*5]->pAbility->arUpg[0][0]==41)||(arEff[i+iEffLine*5]->pAbility->arUpg[0][0]==78)||(arEff[i+iEffLine*5]->pAbility->arUpg[0][0]==25)))
				{WindowEffectInfo(arEff[i+iEffLine*5]->pAbility,arEff[i+iEffLine*5]->pSpell,arEff[i+iEffLine*5]->iTime,arEff[i+iEffLine*5]->iPower);}
				else
				{WindowEffectInfo(arEff[i+iEffLine*5]->pAbility,arEff[i+iEffLine*5]->pSpell,arEff[i+iEffLine*5]->iTime,0);};
			};
		};
		for (int i=1; i<13; i++)
		{
			int Check=arParamMask[i].Check();
			if (Check>1)
			{
				bHint=true;
				MakeAbilityHint(i);
			};
			if ((Check==30)||(Check==40))
			{
				bHint=false;				
				WObj=ShowActiveObjects(WObj,6,0);
				WindowAbilityInfo(i,arParam[i]);
			};
		};

		//Подсказки о параметрах текущего состояния
		//жизнь
		int Check=arParamMask2[0].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(88);
		};
		if ((Check==30)||(Check==40))
		{
			bHint=false;				
			WObj=ShowActiveObjects(WObj,6,0);
			WindowAbilityInfo(1,iLife);
		};

		//выносливость
		Check=arParamMask2[1].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(492);
		};
		if ((Check==30)||(Check==40))
		{
			bHint=false;				
			WObj=ShowActiveObjects(WObj,6,0);
			WindowAbilityInfo(11,iStamina);
		};

		//боевой дух
		Check=arParamMask2[2].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(89);
		};
		if ((Check==30)||(Check==40))
		{
			bHint=false;				
			WObj=ShowActiveObjects(WObj,6,0);
			WindowAbilityInfo(12,iMorale);
		};

		//скорость
		Check=arParamMask2[3].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(493);
		};
		if ((Check==30)||(Check==40))
		{
			bHint=false;				
			WObj=ShowActiveObjects(WObj,6,0);
			WindowAbilityInfo(7,iSpeed);
		};

		//запас выстрелов
		Check=arParamMask2[4].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(494);
		};
		if ((Check==30)||(Check==40))
		{
			bHint=false;				
			WObj=ShowActiveObjects(WObj,6,0);
			WindowAbilityInfo(10,iAmmo);
		};

		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
		}
		else
		{Sleeper(24);};
	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	bHint=false;
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	iAntisleep=iAntisleepDelay;
};

int WarMenu(int AA)
{
	iAntisleep=iAntisleepDelay;
	int Result=0;
	clHumanProfile *PRF=&arHumans[arPlayer[ActivePlayer].iColor];
	int x=512-bmpWarMenu->w/2;
	int y=384-bmpWarMenu->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpWarMenu->w,bmpWarMenu->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpWarMenu,bmpWindow,0,0,0,0,bmpWarMenu->w,bmpWarMenu->h);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+167,y+289,1,2,bmpButton[305],bmpButton[306],bmpButton[307],bmpButton[308],WObj);
	buttExit.iHint=135;
	buttExit.ipHotkey=KEY_ESC;
	UI_Mask arMask[3];
	BITMAP * arMenu[3];
	UI_Mask mskOpt[3];
	BITMAP * arOpt[3];
	BITMAP *Dark=create_bitmap(262,20);
	rectfill(Dark,0,0,262,20,COLOR_1);
	UI_ScrollBar scrSpeed;
	WObj=scrSpeed.Init(x+91,y+252,200,7,2,false,true,WObj);
	scrSpeed.AttachValue(&PRF->iBattleSpeed,0,12);

	G_Object *objBird[3];
	for (int i=0; i<3; i++)
	{
		arMenu[i]=create_bitmap(bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
		WObj=arMask[i].Init(x+57,y+35+i*28,1,bmpGUI_MenuScrN,arMenu[i],WObj,2);
		blit(bmpGUI_MenuScrA,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
		textprintf_centre_ex(bmpWindow,Font1,195,44+i*28,COLOR_1,-1,(pFirstPhrase+255+i)->str);
		textprintf_centre_ex(arMenu[i],Font1,195-59,44-35,COLOR_1,-1,(pFirstPhrase+255+i)->str);		

		arOpt[i]=create_bitmap(262,20);
		WObj=mskOpt[i].Init(x+68,y+154+i*23,1,arOpt[i],arOpt[i],WObj,2);
		draw_sprite(bmpWindow,bmpCheckBox,300,156+i*23);
		blit(bmpWindow,arOpt[i],68,154+i*23,0,0,262,20);
		draw_trans_sprite(arOpt[i],Dark,0,0);
		textprintf_ex(bmpWindow,Font1,72,160+i*23,COLOR_1,-1,(pFirstPhrase+258+i)->str);
		textprintf_ex(arOpt[i],Font1,72-68,160-154,COLOR_2,-1,(pFirstPhrase+258+i)->str);		
		WObj=new G_Object(WObj,x+300,y+152+i*23,2,bmpCheckBird);
		objBird[i]=WObj;
	};
	if (IsPlayed[1-AA])
	{textprintf_centre_ex(bmpWindow,Font1,195,44,makecol(100,100,100),-1,(pFirstPhrase+255)->str);};
	textprintf_centre_ex(bmpWindow,Font1,190,237,COLOR_1,-1,(pFirstPhrase+397)->str);
	if (PRF->bWarGrid)
	{objBird[0]->iLayer=3;}
	else
	{objBird[0]->iLayer=-1;};
	if (PRF->bWarBar)
	{objBird[1]->iLayer=3;}
	else
	{objBird[1]->iLayer=-1;};
	if (PRF->bWarEndTurn)
	{objBird[2]->iLayer=3;}
	else
	{objBird[2]->iLayer=-1;};
	int Check;
	bool bBox=bDemo;
	while ((!buttExit.Check())&&(Result==0))
	{
		for (int i=0; i<3; i++)
		{
			scrSpeed.Check();
			if ((i>0)||(!IsPlayed[1-AA]))
			{
				Check=arMask[i].Check();
				if (Check==30)			
				{
					if (i==2)
					{
						if ((pFirstDialog+1194)->Show(1,1,0,0,NULL,NULL)==1)
						{Result=i+1;};
					}
					else
					{
						SideCanCast[AA]=(pFirstDialog+1460)->Show(1,1,1,0,NULL,NULL)-1;
						Result=i+1;
					};
				};
			};
			Check=mskOpt[i].Check();
			if (Check==30)
			{
				switch (i)
				{
				case 0:
					PRF->bWarGrid=!PRF->bWarGrid;
					bBox=PRF->bWarGrid;
					break;
				case 1:
					PRF->bWarBar=!PRF->bWarBar;
					bBox=PRF->bWarBar;
					break;
				case 2:
					PRF->bWarEndTurn=!PRF->bWarEndTurn;
					bBox=PRF->bWarEndTurn;
					break;
				};
				if (bBox)
				{objBird[i]->iLayer=3;}
				else
				{objBird[i]->iLayer=-1;};
			};
		};
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	PRF->Save();
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	for (int i=0; i<3; i++)
	{
		destroy_bitmap(arMenu[i]);
		destroy_bitmap(arOpt[i]);
	};
	destroy_bitmap(Dark);
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
	return Result;
};