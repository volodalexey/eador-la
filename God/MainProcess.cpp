#include "Mapper.h"

const int BAR_SIZE=6; //Количество позиций в окне строительства
int arBuild[BAR_SIZE];
int arBuildNum[BAR_SIZE];
int CastleProv;

BITMAP *bmpHeroDoll[4];
BITMAP *bmpHeroDollS[4];
BITMAP *bmpHeroDollShadow[4];
BITMAP *bmpFlag[MAX_PLAYERS][4];
BITMAP *bmpHerb[MAX_PLAYERS];
BITMAP *bmpHeroSelect;
BITMAP *bmpRebelDoll;
BITMAP *bmpOuterBld[MAX_OUTBLD_PIC];
BITMAP *bmpPath[4], *bmpQuest[2];
BITMAP *bmpUnhappy[3], *bmpUnrest, *bmpDefender;
BITMAP *bmpBorder;

bool bFogCheat=false;
int MovePlayer, WalkType, WalkSpeed, MinLength, PathKicker;
clPath *GoPath, *P1, *P2, *P3;
UI_TextBox txtError;	//Сообщение о невозможности строительства/найма


int MX,MY;

void CentreMap(int x, int y)
{
	MX=x-512;
	MY=y-384;
	if (MX<0)
	{MX=0;};
	if (MY<0)
	{MY=0;};
	if (MX>bmpMap->w-724)
	{MX=bmpMap->w-724;};
	if (MY>bmpMap->h-741)
	{MY=bmpMap->h-741;};
};

void ClearMoveMap()
{
	for (int i=1; i<=MapSize; i++)
	{
		*(pMoveMap+i)=-1;
		*(pRunMap+i)=-1;
		*(pMobilMap+i)=-100;
	};
};


//Возвращает затраты хода на прохождение провинции текущим героем
int GetMoveCost(int Prov)
{	
	int Result=(pFirstTerrain+(pFirstProvince+Prov)->ipTerrain)->iBF_Pass;
	if (Result<=1)
	{return 1;};
	if ((pFirstProvince+Prov)->ipTerrain<=WalkType)
	{return 1;};	
	if ((pFirstProvince+Prov)->GetStable()>0)
	{return 1;};	
	return Result;
};

dcProvince *PP;
bool bSwim; int Add; 
bool WillGo;

//Заполняет массив pMoveMap количеством ходов, необходимым для достижения провинции текущим героем
int MoveMap (int Prov, int Weight, int Moves, int Port, int addMoves)
{
	bool FirstMove=false;
	if (Weight>MAX_PATH)
	{return 0;};

//Первый ход	
	if (Weight<0)
	{
		bSwim=false;
		FirstMove=true;
		Weight=1;
		*(pMoveMap+Prov)=Weight;

//Конюшня
		if (WalkType<10)
		{
			Moves+=(pFirstProvince+Prov)->GetStable();
		};
		addMoves=0;
//Осада
		if ((MovePlayer>0)&&((pFirstProvince+Prov)->ipPlayer!=MovePlayer))
		{Moves=1;};
		*(pMobilMap+Prov)=Moves;
	}
	else
	{
		if (((pFirstProvince+Prov)->ipTerrain==0)&&(Port>0))
		{
			*(pMobilMap+Prov)=Port;
			Port--;
			bSwim=true;
		}
		else
		{			
			bSwim=false;
			if (Moves<=0)
			{				
				Weight++;
				Moves=WalkSpeed-GetMoveCost(Prov)+addMoves;
				*(pMobilMap+Prov)=Moves;
				addMoves=0;
			}
			else
			{
				*(pMobilMap+Prov)=Moves;
				addMoves=0;				
				Moves-=GetMoveCost(Prov);
//Конюшня
				if ((Moves<=0)&&(WalkType<10))
				{
					addMoves=(pFirstProvince+Prov)->GetStable();
				};
			};
		};
		if (*(pMoveMap+Prov)==Weight)
		{
			if (bSwim)
			{
				if (*(pRunMap+Prov)<Port)
				{*(pRunMap+Prov)=Port;}
				else
				{return 0;};
			}
			else
			{
				if (*(pRunMap+Prov)<Moves)
				{*(pRunMap+Prov)=Moves;}
				else
				{return 0;};
			};
		};
		if ((*(pMoveMap+Prov)<0)||(*(pMoveMap+Prov)>Weight))
		{
			*(pMoveMap+Prov)=Weight;
			if (bSwim)
			{*(pRunMap+Prov)=Port;}
			else
			{*(pRunMap+Prov)=Moves;};
		};
		

		if ((MovePlayer>0)&&((pFirstProvince+Prov)->ipTerrain>0)&&((pFirstProvince+Prov)->ipTerrain<7)&&(((pFirstProvince+Prov)->ipPlayer!=MovePlayer)||((pFirstProvince+Prov)->GetSiegeIndex()>=0)))
		{
			return 0;
		};
	};
	
	if ((pFirstProvince+Prov)->ipTerrain>0)
	{
		Port=(pFirstProvince+Prov)->GetPort();
		if ((WalkType==10)&&(Port<Moves))
		{
			Port=0;
		};		
	};
	

	for (int i=0; i<MAX_NEIGHBOUR; i++)
	{
		if ((pFirstProvince+Prov)->arNeighbour[i]>0)
		{			
			PP=(pFirstProvince+(pFirstProvince+Prov)->arNeighbour[i]);
			WillGo=true;
			//море
			if ((PP->ipTerrain==0)&&(Port<1)&&(WalkType<10))
			{WillGo=false;};
			//горы
			if ((PP->ipTerrain==7)&&((WalkType<10)||(Moves<2)))
			{WillGo=false;};
			//уже достигнута за более короткий путь
			Add=Weight;
			if ((Moves<=0)||((Port>0)&&(PP->ipTerrain==0)&&((pFirstProvince+Prov)->ipTerrain>0))&&(Moves<WalkSpeed))
			{Add++;};
			if (((*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])>=0))&&
				(*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])<Add))
			{WillGo=false;};
			if ((Port>0)&&(PP->ipTerrain==0))
			{Add=Port;} else {Add=Moves;}
			if (
				(*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])<=Weight)&&
			//	((*(pMobilMap+(pFirstProvince+Prov)->arNeighbour[i])>=0))&&
				(*(pMobilMap+(pFirstProvince+Prov)->arNeighbour[i])>=Add))
			{WillGo=false;};
			//запрет атаки из осаждённой провинции
			if ((WillGo)&&(MovePlayer>0)&&((pFirstProvince+Prov)->ipPlayer!=MovePlayer)&&(PP->ipPlayer!=MovePlayer)&&(!Tutorial.Map)&&((pFirstProvince+Prov)->ipTerrain>0)&&((pFirstProvince+Prov)->ipTerrain!=7))
			{
				WillGo=false;
				//разрешить атаку, если по соседству есть своя провинция
				/*for (int j=0; j<MAX_NEIGHBOUR; j++)
				{
					if ((PP->arNeighbour[j]>0)&&((pFirstProvince+PP->arNeighbour[j])->ipPlayer==MovePlayer))
					{WillGo=true;};
				};*/
			};
			/*if ((WillGo)&&(*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])==Add))
			{
				if (PR->ipTerrain==0)
				{
					if (*(pRunMap+(pFirstProvince+Prov)->arNeighbour[i])>=Port-1)
					{WillGo=false;}
				}
				else
				{

				};

			};*/
			if (WillGo)
			{			
				if ((Port>0)&&(PP->ipTerrain==0)&&((pFirstProvince+Prov)->ipTerrain>0))
				{
					if (!FirstMove)
					{
						MoveMap(PP->iNumber,Weight+1,1,Port,0);
					}
					else
					{
						MoveMap(PP->iNumber,Weight,1,Port,0);
					};
				}
				else
				{
					MoveMap(PP->iNumber,Weight,Moves,Port,addMoves);			
				};
			};				
		};
	};	
	return 0;
};


//На основании массива pMoveMap ищет оптимальный путь из провинции Prov в провинцию Prov1

int LastMoves, Checker;
int MovePath (int Prov, int Weight, clPath *Path, int Prov1, int Moves, int Port, int Length, int addMoves)
{ 	
	if ((GoPath)&&(Length>=MinLength))
	{return 0;};
	if (Path)
	{
		if (((pFirstProvince+Prov)->ipTerrain==0)&&(Port>0))
		{		
			Port--;	
		}
		else
		{
			if (Moves<=0)
			{
				Weight++;
				Moves=WalkSpeed-GetMoveCost(Prov)+addMoves;
				addMoves=0;
			}
			else
			{
				addMoves=0;
				Moves-=GetMoveCost(Prov);
//Конюшня
				if ((Moves<=0)&&(WalkType<10))
				{
					addMoves=(pFirstProvince+Prov)->GetStable();
				};
			};
		};
		if (Weight>*(pMoveMap+Prov1))
		{
			return 0;
		};
	}
	else
	{
		LastMoves=0;
		//Конюшня
		if (WalkType<10)
		{
			Moves+=(pFirstProvince+Prov)->GetStable();
		};
		//Осада
		addMoves=0;
		if ((MovePlayer>0)&&((pFirstProvince+Prov)->ipPlayer!=MovePlayer))
		{Moves=1;};
	};

	clPath *pRoad=new clPath(Prov, Weight, Path);
	Length++;
	if (Prov==Prov1)
	{
		if (GoPath)
		{
			if ((Length<MinLength)||((Length==MinLength)&&(Moves>LastMoves)))
			{
				P1=GoPath;
				while (P1)
				{
					P1=GoPath->pPrev;
					delete GoPath;
					GoPath=P1;
				};
			}
			else
			{
				delete pRoad;
				return 0;
			}
		}
//		textprintf(screen,Font1,0,0,COLOR_2,"%d",Weight);
//		readkey();
		P1=new clPath(Prov, Weight ,NULL);
		P2=pRoad->pPrev; 			
		while (P2)
		{
			P3=new clPath(P2->x, P2->y, P1);
			P2=P2->pPrev;
			P1=P3;
		};
		GoPath=P1;					
		MinLength=Length;
		LastMoves=Moves;
		delete pRoad;
		return 1;
	};

	if ((Path)&&(MovePlayer>0)&&((pFirstProvince+Prov)->ipTerrain>0)&&((pFirstProvince+Prov)->ipTerrain<7)&&(((pFirstProvince+Prov)->ipPlayer!=MovePlayer)||((pFirstProvince+Prov)->GetSiegeIndex()>=0)))
	{
		delete pRoad;
		return 0;
	};
	
	
	if ((pFirstProvince+Prov)->ipTerrain>0)
	{
		Port=(pFirstProvince+Prov)->GetPort();
		if ((WalkType==10)&&(Port<Moves))
		{
			Port=0;
		};		
	};

	for (int i=0; i<MAX_NEIGHBOUR; i++)
	{
		if ((pFirstProvince+Prov)->arNeighbour[i]>0)
		{
			PP=(pFirstProvince+(pFirstProvince+Prov)->arNeighbour[i]);
			bool WillGo=true;
			//море
			if ((PP->ipTerrain==0)&&(Port<1)&&(WalkType<10))
			{WillGo=false;};
			//горы
			if ((PP->ipTerrain==7)&&((WalkType<10)||(Moves<2)))
			{WillGo=false;};
			//уже достигнута за более короткий путь
			Add=Weight;
		//	if ((Moves<=0)||((Port>0)&&(PP->ipTerrain==0)&&((pFirstProvince+Prov)->ipTerrain>0))&&(Moves<WalkSpeed))
		//	{Add++;};

			if (PathKicker==0)
			{

			if (((*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])>=0))&&
				(*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])<GetMax(Add-PathKicker,0)))
			{WillGo=false;};

			if ((Port>0)&&(PP->ipTerrain==0))
			{Add=Port;} else {Add=Moves;}

			if ((Weight==*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i]))&&
				(*(pMobilMap+(pFirstProvince+Prov)->arNeighbour[i])>Add)&&
				(PathKicker==0))
				{WillGo=false;};
			}
			else
			{
				Checker=0;
				if (((*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])>=0))&&
					(*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])<Add)&&
					(Moves<WalkSpeed))
				{WillGo=false;};

				if (((*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])>=0))&&
					(*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i])<Add))
				{Checker=1;};

				if ((Port>0)&&(PP->ipTerrain==0))
				{Add=Port;} else {Add=Moves;}

				if ((Weight>=*(pMoveMap+(pFirstProvince+Prov)->arNeighbour[i]))&&
					(*(pMobilMap+(pFirstProvince+Prov)->arNeighbour[i])>Add)&&
					(Checker>0))
				{WillGo=false;};
			}

			//запрет атаки из осаждённой провинции
			if ((WillGo)&&(MovePlayer>0)&&((pFirstProvince+Prov)->ipPlayer!=MovePlayer)&&(PP->ipPlayer!=MovePlayer)&&(!Tutorial.Map)&&((pFirstProvince+Prov)->ipTerrain>0)&&((pFirstProvince+Prov)->ipTerrain!=7))
			{
				WillGo=false;
				//разрешить атаку, если по соседству есть своя провинция
				/*for (int j=0; j<MAX_NEIGHBOUR; j++)
				{
					if ((PP->arNeighbour[j]>0)&&((pFirstProvince+PP->arNeighbour[j])->ipPlayer==MovePlayer))
					{WillGo=true;};
				};*/
			};

			if (WillGo)
			{			
				if ((Port>0)&&(PP->ipTerrain==0)&&((pFirstProvince+Prov)->ipTerrain>0))
				{
					if (Path)
					{
						MovePath(PP->iNumber,Weight+1,pRoad,Prov1,1,Port,Length,0);
					}
					else
					{
						MovePath(PP->iNumber,Weight,pRoad,Prov1,1,Port,Length,0);
					};
				}
				else
				{
					MovePath(PP->iNumber,Weight,pRoad,Prov1,Moves,Port,Length,addMoves);			
				};			
			};				
		};
	};		

	delete pRoad;
	return 0;
};


//Открывает туман войны вокруг провинции с портом
int Swim(int Player, int Prov, int Port)
{
	if (Port<0)
	{return 0;};
	for (int i=0; i<MAX_NEIGHBOUR; i++)
	{
		if ((pFirstProvince+Prov)->arNeighbour[i]>0)
		{
			*(arPlayer[Player].pFog+(pFirstProvince+Prov)->arNeighbour[i])=0;
			if ((pFirstProvince+(pFirstProvince+Prov)->arNeighbour[i])->ipTerrain==0)
			{
				Swim(Player, (pFirstProvince+Prov)->arNeighbour[i], Port-1);
			};
		};
	};
	return 0;
};

//Вычисляет туман войны
void MakeFog(int Player)
{
	bool bView=bDemo;
	if (!CurGame.bFog)
	{
		for (int i=1; i<=MapSize; i++)
		{
			*(arPlayer[Player].pFog+i)=0;			
		};
		return;
	};
	if (!bFogCheat)
	{
	for (int i=1; i<=MapSize; i++)
	{
		if (*(arPlayer[Player].pFog+i)<2)
		{
			*(arPlayer[Player].pFog+i)=1;
		};
	};
	};
	//Вокруг провинций
	for (int i=1; i<=MapSize; i++)
	{
		if ((pFirstProvince+i)->ipPlayer==Player)
		{
			*(arPlayer[Player].pFog+i)=0;
			if ((pFirstProvince+i)->GetPort()>0)
			{
				Swim(Player,i,(pFirstProvince+i)->GetPort());
			};
			bView=false;
			if ((pFirstProvince+i)->iDefender>0)
			{
				for (int k=0; k<MAX_DEFENDER_ABILITY; k++)
				{
					if ((pFirstDefender+(pFirstProvince+i)->iDefender)->arAbility[k][0]==5)
					{bView=true;};
				};
			};
			if ((pFirstProvince+i)->CheckBldAbil(11)>0)
			{bView=true;};
			for (int j=0; j<MAX_NEIGHBOUR; j++)
			{
				if ((pFirstProvince+i)->arNeighbour[j]>0)
				{
					*(arPlayer[Player].pFog+(pFirstProvince+i)->arNeighbour[j])=0;					
					if (bView)
					{
						for (int l=0; l<MAX_NEIGHBOUR; l++)
						{
							if ((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->arNeighbour[l]>0)
							{
								*(arPlayer[Player].pFog+(pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->arNeighbour[l])=0;
							};
						};
					};
				};
			};
		};
	};
	//Вокруг героев
	for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0))
		{
			*(arPlayer[Player].pFog+arPlayer[Player].arHero[i]->ipProvince)=0;
			for (int j=0; j<MAX_NEIGHBOUR; j++)
			{
				if ((pFirstProvince+arPlayer[Player].arHero[i]->ipProvince)->arNeighbour[j]>0)
				{
					*(arPlayer[Player].pFog+(pFirstProvince+arPlayer[Player].arHero[i]->ipProvince)->arNeighbour[j])=0;
				};
			};			
		};
	};
};

//Формирует и выводит на экран список построек для строительства
int BuildList(int Player, int Province, int Shift, BITMAP * Target)
{
	int MaxBld=0;
	if (arPlayer[Player].ipProvince==Province)
	{
		txtError.Activate((pFirstMessage+5)->msg);
		return 0;
	};

	if (arPlayer[Player].iCanOuterBld<=0)
	{
		txtError.Activate((pFirstMessage+2)->msg);
		return 0;
	};

	if ((pFirstProvince+Province)->GetSiegeIndex()>=0)
	{
		txtError.Activate((pFirstMessage+18)->msg);
		return 0;
	};
	
	for (int i=1; i<=iMaxOuterBld; i++)
	{
		if ((*(arPlayer[Player].pOuterBld+i)!=0)&&((!arPlayer[Player].bBldFilter)||((pFirstProvince+Province)->CanBuild(i))))
		{
			if ((MaxBld>=Shift)&&(MaxBld<Shift+BAR_SIZE))
			{
				arBuild[MaxBld-Shift]=i;
			};
			MaxBld++;		
		};
	};
	int Limit=MaxBld;
	if (Limit>BAR_SIZE)
	{Limit=BAR_SIZE;};
	for (int i=0; i<Limit; i++)
	{
		draw_sprite(Target,bmpGUI_BuildPanel,50,39+i*59);
		draw_sprite(Target,(pFirstOuterBld+arBuild[i])->Pic,52,41+i*59);
		if (*(arPlayer[Player].pOuterBld+arBuild[i])>0)
		{
			ShowIconNumber(Target, 52, 41+i*59, *(arPlayer[Player].pOuterBld+arBuild[i]));
		//	textprintf_centre_ex(Target,Font1,95,83+i*59,COLOR_1,-1,"%d",*(arPlayer[Player].pOuterBld+arBuild[i]));
		//	textprintf_centre_ex(Target,Font1,94,82+i*59,COLOR_2,-1,"%d",*(arPlayer[Player].pOuterBld+arBuild[i]));
		};
		if ((pFirstProvince+Province)->CanBuild(arBuild[i]))
		{
			textprintf_centre_ex(Target,Font1,193,48+i*59,COLOR_1,-1,"%s",(pFirstOuterBld+arBuild[i])->sName);
		}
		else
		{
			textprintf_centre_ex(Target,Font1,193,48+i*59,makecol(220,20,20),-1,"%s",(pFirstOuterBld+arBuild[i])->sName);
		};
		if (arPlayer[Player].GetOutBldGoldPrice(arBuild[i])<=arPlayer[Player].Gold)
		{
			textprintf_centre_ex(Target,Font1,151,70+i*59,COLOR_2,-1,"%d",arPlayer[Player].GetOutBldGoldPrice(arBuild[i]));
		}
		else
		{
			textprintf_centre_ex(Target,Font1,151,70+i*59,makecol(220,20,20),-1,"%d",arPlayer[Player].GetOutBldGoldPrice(arBuild[i]));
		};
		if ((arPlayer[Player].GetOutBldGemPrice(arBuild[i])<=arPlayer[Player].Gem)||(arPlayer[Player].GetOutBldGemPrice(arBuild[i])==0))
		{
			textprintf_centre_ex(Target,Font1,233,70+i*59,COLOR_2,-1,"%d",arPlayer[Player].GetOutBldGemPrice(arBuild[i]));
		}
		else
		{
			textprintf_centre_ex(Target,Font1,233,70+i*59,makecol(220,20,20),-1,"%d",arPlayer[Player].GetOutBldGemPrice(arBuild[i]));
		};

	};
	if (MaxBld==0)
	{
		bool bSpace=false;
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			if ((pFirstProvince+Province)->arBuilding[i]==0)
			{bSpace=true;};
		};
		if (bSpace)
		{txtError.Activate((pFirstMessage+3)->msg);}
		else
		{txtError.Activate((pFirstMessage+4)->msg);};
	};
	return MaxBld;
};

//Формирует и выводит на экран список построек для сноса
int RazeList(int Province, int Shift, BITMAP * Target)
{
	int MaxBld=0;
	for (int i=0; i<MAX_PROVINCE_BLD; i++)
	{
		if ((pFirstProvince+Province)->arBuilding[i]>0)
		{	
			if ((MaxBld>=Shift)&&(MaxBld<Shift+BAR_SIZE))
			{
				arBuild[MaxBld-Shift]=(pFirstProvince+Province)->arBuilding[i];
			};
			MaxBld++;			
		};
	};
	int Limit=MaxBld;
	if (Limit>BAR_SIZE)
	{Limit=BAR_SIZE;};
	for (int i=0; i<Limit; i++)
	{
		draw_sprite(Target,bmpGUI_RazePanel,50,39+i*59);
		draw_sprite(Target,(pFirstOuterBld+arBuild[i])->Pic,52,41+i*59);
		textprintf_centre_ex(Target,Font1,193,48+i*59,COLOR_1,-1,"%s",(pFirstOuterBld+arBuild[i])->sName);
	};
	return MaxBld;
};

//Формирует и выводит на экран список стражей для найма
int HireList(int Player, int Province, int Shift, BITMAP * Target)
{
	if (arPlayer[Player].iCanHire<=0)
	{
		txtError.Activate((pFirstMessage+7)->msg);
		return 0;
	};
	if ((pFirstProvince+Province)->GetSiegeIndex()>=0)
	{
		txtError.Activate((pFirstMessage+19)->msg);
		return 0;
	};
	if ((pFirstProvince+Province)->iDefender>0)
	{
		if ((pFirstDefender+(pFirstProvince+Province)->iDefender)->bNoDismiss)
		{
			txtError.Activate((pFirstMessage+26)->msg);
			return 0;
		};
	};
	int MaxDef=0;
	for (int i=1; i<=iMaxDefender; i++)
	{
		if ((*(arPlayer[Player].pDefender+i)!=0)&&((!arPlayer[Player].bDefFilter)||((pFirstProvince+Province)->CanHire(i))))
		{
			if ((MaxDef>=Shift)&&(MaxDef<Shift+BAR_SIZE))
			{
				arBuild[MaxDef-Shift]=i;
			};
			MaxDef++;		
		};
	};
	int Limit=MaxDef;
	if (Limit>BAR_SIZE)
	{Limit=BAR_SIZE;};
	for (int i=0; i<Limit; i++)
	{
		draw_sprite(Target,bmpGUI_BuildPanel,50,39+i*59);
		draw_sprite(Target,(pFirstDefender+arBuild[i])->Pic,52,41+i*59);
		if (*(arPlayer[Player].pDefender+arBuild[i])>0)
		{
			ShowIconNumber(Target, 52, 41+i*59, *(arPlayer[Player].pDefender+arBuild[i]));
		//	textprintf_centre_ex(Target,Font1,95,83+i*59,COLOR_1,-1,"%d",*(arPlayer[Player].pDefender+arBuild[i]));
		//	textprintf_centre_ex(Target,Font1,94,82+i*59,COLOR_2,-1,"%d",*(arPlayer[Player].pDefender+arBuild[i]));
		};
		if ((pFirstProvince+Province)->CanHire(arBuild[i]))
		{
			textprintf_centre_ex(Target,Font1,190,48+i*59,COLOR_1,-1,"%s",(pFirstDefender+arBuild[i])->sName);
		}
		else
		{
			textprintf_centre_ex(Target,Font1,190,48+i*59,makecol(220,20,20),-1,"%s",(pFirstDefender+arBuild[i])->sName);
		};
		if (arPlayer[Player].GetDefenderGoldPrice(arBuild[i])<=arPlayer[Player].Gold)
		{
			textprintf_centre_ex(Target,Font1,151,70+i*59,COLOR_2,-1,"%d",arPlayer[Player].GetDefenderGoldPrice(arBuild[i]));
		}
		else
		{
			textprintf_centre_ex(Target,Font1,151,70+i*59,makecol(220,20,20),-1,"%d",arPlayer[Player].GetDefenderGoldPrice(arBuild[i]));
		};
		if ((arPlayer[Player].GetDefenderGemPrice(arBuild[i])<=arPlayer[Player].Gem)||(arPlayer[Player].GetDefenderGemPrice(arBuild[i])==0))
		{
			textprintf_centre_ex(Target,Font1,233,70+i*59,COLOR_2,-1,"%d",arPlayer[Player].GetDefenderGemPrice(arBuild[i]));
		}
		else
		{
			textprintf_centre_ex(Target,Font1,233,70+i*59,makecol(220,20,20),-1,"%d",arPlayer[Player].GetDefenderGemPrice(arBuild[i]));
		};

	};
	if (MaxDef==0)
	{
		txtError.Activate((pFirstMessage+6)->msg);
	};
	return MaxDef;
};

//Формирует и выводит на экран список особых мест
int SiteList(int Province, int Shift, BITMAP * Target, bool CanEnter, dcHero *Hero)
{
	int MaxBld=0;
	if ((Tutorial.Map)&&(Tutorial.Site>=0))
	{
		MaxBld=1;
		arBuild[0]=(pFirstProvince+Province)->arSite[Tutorial.Site][0];
		arBuildNum[0]=Tutorial.Site;
	}
	else
	{
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if (((pFirstProvince+Province)->arSite[i][0]>0)&&((pFirstProvince+Province)->arSite[i][1]<=(pFirstProvince+Province)->iExplore)&&
			((!CanEnter)||((pFirstProvince+Province)->arSiteGuard[i])||((pFirstSite+(pFirstProvince+Province)->arSite[i][0])->bCanEnter)))
		{	
			if ((MaxBld>=Shift)&&(MaxBld<Shift+BAR_SIZE))
			{
				arBuild[MaxBld-Shift]=(pFirstProvince+Province)->arSite[i][0];
				arBuildNum[MaxBld-Shift]=i;
			};
			MaxBld++;			
		};
	};
	};
	int Limit=MaxBld;
	if (Limit>BAR_SIZE)
	{Limit=BAR_SIZE;};
	for (int i=0; i<Limit; i++)
	{
		draw_sprite(Target,bmpGUI_RazePanel,50,39+i*59);
		draw_sprite(Target,(pFirstSite+arBuild[i])->Pic,52,41+i*59);
		textprintf_centre_ex(Target,Font1,183,48+i*59,COLOR_1,-1,"%s",(pFirstSite+arBuild[i])->sName);
		if ((pFirstProvince+Province)->arSiteGuard[arBuildNum[i]])
		{
			textprintf_centre_ex(Target,Font1,193,70+i*59,makecol(220,20,20),-1,"(%s)",(pFirstGuardType+(pFirstProvince+Province)->arSite[arBuildNum[i]][4])->sName);
		};
		if (Hero)
		{
			for (int j=0; j<HERO_QUEST_LIMIT; j++)
			{
				if ((Hero->arQuest[j].Index>0)&&(Hero->arQuest[j].ipProvince==Province)&&(Hero->arQuest[j].ipSite==arBuildNum[i]))
				{
					Hero->CheckQuestDone(j);
					if (Hero->arQuest[j].bComplete)
					{draw_sprite(Target,bmpQuest[1],46,46+i*59);}
					else
					{draw_sprite(Target,bmpQuest[0],46,46+i*59);};
				};
			};
		};
	};
	return MaxBld;
};

//Формирует и выводит на экран список ритуалов
int RitualList(int Player, int Province, int Shift, BITMAP * Target)
{
	int MaxRit=0;
	if (arPlayer[Player].iCanCast<=0)
	{
		txtError.Activate((pFirstMessage+9)->msg);
		return 0;
	};
	for (int i=1; i<=iMaxRitual; i++)
	{
		if ((*(arPlayer[Player].pRitual+i)!=0)&&((!arPlayer[Player].bRitFilter)||((pFirstProvince+Province)->CanCast(i,Player))))
		{
			if ((MaxRit>=Shift)&&(MaxRit<Shift+BAR_SIZE))
			{
				arBuild[MaxRit-Shift]=i;
			};
			MaxRit++;		
		};
	};
	int Limit=MaxRit;
	if (Limit>BAR_SIZE)
	{Limit=BAR_SIZE;};	
	for (int i=0; i<Limit; i++)
	{
		draw_sprite(Target,bmpGUI_BuildPanel,50,39+i*59);
		draw_sprite(Target,(pFirstRitual+arBuild[i])->Pic,52,41+i*59);
		if (*(arPlayer[Player].pRitual+arBuild[i])>0)
		{
			ShowIconNumber(Target, 52, 41+i*59, *(arPlayer[Player].pRitual+arBuild[i]));			
		//	textprintf_centre_ex(Target,Font1,95,83+i*59,COLOR_1,-1,"%d",*(arPlayer[Player].pRitual+arBuild[i]));
		//	textprintf_centre_ex(Target,Font1,94,82+i*59,COLOR_2,-1,"%d",*(arPlayer[Player].pRitual+arBuild[i]));
		};
		if (*(arPlayer[Player].pRitualCD+arBuild[i])>0)
		{
			draw_sprite(Target,bmpStuff[34],53,42+i*59);
			textprintf_centre_ex(Target,Font1,74,47+i*59,COLOR_1,-1,"%d",*(arPlayer[Player].pRitualCD+arBuild[i]));
		};
		if (((pFirstProvince+Province)->CanCast(arBuild[i],Player))&&(*(arPlayer[Player].pRitualCD+arBuild[i])==0))
		{
			textprintf_centre_ex(Target,Font1,193,48+i*59,COLOR_1,-1,"%s",(pFirstRitual+arBuild[i])->sName);
		}
		else
		{
			textprintf_centre_ex(Target,Font1,193,48+i*59,makecol(220,20,20),-1,"%s",(pFirstRitual+arBuild[i])->sName);
		};
		if ((arPlayer[Player].GetRitualGoldPrice(arBuild[i])<=arPlayer[Player].Gold)||(arPlayer[Player].GetRitualGoldPrice(arBuild[i])==0))
		{
			textprintf_centre_ex(Target,Font1,151,70+i*59,COLOR_2,-1,"%d",arPlayer[Player].GetRitualGoldPrice(arBuild[i]));
		}
		else
		{
			textprintf_centre_ex(Target,Font1,151,70+i*59,makecol(220,20,20),-1,"%d",arPlayer[Player].GetRitualGoldPrice(arBuild[i]));
		};
		if ((arPlayer[Player].GetRitualGemPrice(arBuild[i])<=arPlayer[Player].Gem)||(arPlayer[Player].GetRitualGemPrice(arBuild[i])==0))
		{
			textprintf_centre_ex(Target,Font1,233,70+i*59,COLOR_2,-1,"%d",arPlayer[Player].GetRitualGemPrice(arBuild[i]));
		}
		else
		{
			textprintf_centre_ex(Target,Font1,233,70+i*59,makecol(220,20,20),-1,"%d",arPlayer[Player].GetRitualGemPrice(arBuild[i]));
		};

	};
	if (MaxRit==0)
	{
		txtError.Activate((pFirstMessage+8)->msg);
	};
	return MaxRit;
};

//Формирует и выводит на экран список ритуалов для героев
int HeroRitualList(int Player, int Hero, int Shift, BITMAP * Target)
{
	int MaxRit=0;
	if (arPlayer[Player].iCanCast<=0)
	{
		txtError.Activate((pFirstMessage+9)->msg);
		return 0;
	};
	for (int i=1; i<=iMaxRitual; i++)
	{
		if ((*(arPlayer[Player].pRitual+i)!=0)&&((pFirstRitual+i)->GetAbility(20)))
		{
			if ((MaxRit>=Shift)&&(MaxRit<Shift+BAR_SIZE))
			{
				arBuild[MaxRit-Shift]=i;
			};
			MaxRit++;		
		};
	};
	int Limit=MaxRit;
	if (Limit>BAR_SIZE)
	{Limit=BAR_SIZE;};	
	for (int i=0; i<Limit; i++)
	{
		draw_sprite(Target,bmpGUI_BuildPanel,50,39+i*59);
		draw_sprite(Target,(pFirstRitual+arBuild[i])->Pic,52,41+i*59);
		if (*(arPlayer[Player].pRitual+arBuild[i])>0)
		{
			ShowIconNumber(Target, 52, 41+i*59, *(arPlayer[Player].pRitual+arBuild[i]));			
		//	textprintf_centre_ex(Target,Font1,95,83+i*59,COLOR_1,-1,"%d",*(arPlayer[Player].pRitual+arBuild[i]));
		//	textprintf_centre_ex(Target,Font1,94,82+i*59,COLOR_2,-1,"%d",*(arPlayer[Player].pRitual+arBuild[i]));
		};
		if (*(arPlayer[Player].pRitualCD+arBuild[i])>0)
		{
			draw_sprite(Target,bmpStuff[34],53,42+i*59);
			textprintf_centre_ex(Target,Font1,74,47+i*59,COLOR_1,-1,"%d",*(arPlayer[Player].pRitualCD+arBuild[i]));
		};
		if ((arPlayer[Player].arHero[Hero]->pProvince->CanCast(arBuild[i],Player))&&(*(arPlayer[Player].pRitualCD+arBuild[i])==0))
		{
			textprintf_centre_ex(Target,Font1,193,48+i*59,COLOR_1,-1,"%s",(pFirstRitual+arBuild[i])->sName);
		}
		else
		{
			textprintf_centre_ex(Target,Font1,193,48+i*59,makecol(220,20,20),-1,"%s",(pFirstRitual+arBuild[i])->sName);
		};
		if ((arPlayer[Player].GetRitualGoldPrice(arBuild[i])<=arPlayer[Player].Gold)||(arPlayer[Player].GetRitualGoldPrice(arBuild[i])==0))
		{
			textprintf_centre_ex(Target,Font1,151,70+i*59,COLOR_2,-1,"%d",arPlayer[Player].GetRitualGoldPrice(arBuild[i]));
		}
		else
		{
			textprintf_centre_ex(Target,Font1,151,70+i*59,makecol(220,20,20),-1,"%d",arPlayer[Player].GetRitualGoldPrice(arBuild[i]));
		};
		if ((arPlayer[Player].GetRitualGemPrice(arBuild[i])<=arPlayer[Player].Gem)||(arPlayer[Player].GetRitualGemPrice(arBuild[i])==0))
		{
			textprintf_centre_ex(Target,Font1,233,70+i*59,COLOR_2,-1,"%d",arPlayer[Player].GetRitualGemPrice(arBuild[i]));
		}
		else
		{
			textprintf_centre_ex(Target,Font1,233,70+i*59,makecol(220,20,20),-1,"%d",arPlayer[Player].GetRitualGemPrice(arBuild[i]));
		};

	};
	if (MaxRit==0)
	{
		txtError.Activate((pFirstMessage+28)->msg);
	};
	return MaxRit;
};

//Формирует и выводит на экран список действующих ритуалов
int EnchantList(int Player, int Province, int Shift, BITMAP * Target)
{
	int MaxRit=0;
	int arTime[6];
	dcRitual *RR=pRitual;
	while (RR)
	{		
		if ((RR->ipProvince==Province)&&(RR->ipPlayer==Player)&&(RR->Timer>0))
		{		
			if ((MaxRit>=Shift)&&(MaxRit<Shift+BAR_SIZE))
			{
				arBuild[MaxRit-Shift]=RR->Index;
				arTime[MaxRit-Shift]=RR->Timer;
			};
			MaxRit++;
		};
		RR=RR->pNext;		
	};
	int Limit=MaxRit;
	if (Limit>BAR_SIZE)
	{Limit=BAR_SIZE;};
	for (int i=0; i<Limit; i++)
	{
		draw_sprite(Target,bmpGUI_RazePanel,50,39+i*59);		
		draw_sprite(Target,(pFirstRitual+arBuild[i])->Pic,52,41+i*59);
		textprintf_centre_ex(Target,Font1,193,48+i*59,COLOR_1,-1,"%s",(pFirstRitual+arBuild[i])->sName);
		if ((pFirstRitual+arBuild[i])->iTime>0)
		{
			draw_sprite(Target,bmpStuff[35],50+95,39+24+i*59);
			textprintf_centre_ex(Target,Font1,50+139,39+31+i*59,COLOR_2,-1,"%d",arTime[i]);
		};
	};
	return MaxRit;
};

//Формирует и выводит на экран список героев
int HeroList(int Player, int Hero, int Shift, BITMAP * Target, bool Exclude)
{
	int MaxHero=0;
	int PrN=arPlayer[Player].arHero[Hero]->pProvince->iNumber;
	for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[Player].arHero[i])&&((arPlayer[Player].arHero[i])->iDead<0)&&(arPlayer[Player].arHero[i]->pProvince->iNumber==PrN)&&((!Exclude)||(i!=Hero)))
		{	
			if ((MaxHero>=Shift)&&(MaxHero<Shift+BAR_SIZE))
			{
				arBuild[MaxHero-Shift]=i;
				arBuildNum[MaxHero-Shift]=i;
			};
			MaxHero++;			
		};
	};
	int Limit=MaxHero;
	if (Limit>BAR_SIZE)
	{Limit=BAR_SIZE;};
	for (int i=0; i<Limit; i++)
	{
		draw_sprite(Target,bmpGUI_RazePanel,50,39+i*59);
		draw_sprite(Target,arPlayer[Player].arHero[arBuild[i]]->smPic,52,41+i*59);
		textprintf_centre_ex(Target,Font1,193,48+i*59,COLOR_1,-1,"%s",arPlayer[Player].arHero[arBuild[i]]->pstName->str);
	};
	return MaxHero;
};

//******Создание графических объектов для визуализации карты**********
G_Object * ShowMap(int Player, G_Object *objMain, int *Prov, G_Object *objEmpty)
{
	for (int i=1; i<=MapSize; i++)
	{
		if (*(arPlayer[Player].pFog+i)>0)
		{
			objMain=new G_Object(objMain,0,0,1,(pFirstFog+i)->Pic);
			objMain->PlaceOnMap((pFirstFog+i)->x,(pFirstFog+i)->y);
			if (*(arPlayer[Player].pFog+i)==1)
			{
				objMain->bTrans=true;
				objMain->iLayer=4;
			};
		};
	};
	for (int y=0; y<bmpMap->h; y++)	
	{
		Map_Border *MP=Map[y];
		while (MP)
		{
			if ((MP->Prov[0]>0)&&(MP->Prov[1]>0)
			//	&&(Random(10,MP->x,y,CurGame.iSeed)>8)
				)
			{
				if (((*(arPlayer[Player].pFog+MP->Prov[0])==2)&&(*(arPlayer[Player].pFog+MP->Prov[1])==0))||
					((*(arPlayer[Player].pFog+MP->Prov[0])==0)&&(*(arPlayer[Player].pFog+MP->Prov[1])==2)))
				{
					objMain=new G_Object(objMain,0,0,1,bmpMapDark);
					objMain->PlaceOnMap(MP->x-15,y-15);					
					objMain->bTrans=true;					
				};				
			};
			MP=MP->Next;
		};
	};
	G_Object *Tmp=objMain, *Tmp1, *Tmp2;
	while (Tmp)
	{
		if (Tmp->Pic==bmpMapDark)
		{
			Tmp1=Tmp->pNext;
			while (Tmp1)
			{
				if ((Tmp1->Pic==bmpMapDark)&&(abs(Tmp1->MapX-Tmp->MapX)<10)&&(abs(Tmp1->MapY-Tmp->MapY)<10))
				{
					Tmp2=Tmp1;
					Tmp1=Tmp1->pNext;
					delete Tmp2;
				}
				else
				{Tmp1=Tmp1->pNext;};
			};
		};
		Tmp=Tmp->pNext;
	};
	for (int i=1; i<=MapSize; i++)
	{
		*(Prov+i)=0;
	};	

	for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0))
		{
			*(Prov+arPlayer[Player].arHero[i]->pProvince->iNumber)+=1;
		};
	};

	
	for (int i=MapSize; i>0; i--)
	{
		if (*(arPlayer[Player].pFog+i)<2)
		{
		dcProvince *PR=(pFirstProvince+i);

		WriteLog("Ресурсы");
		if (PR->GetResource(1)>0)
		{
			objMain=new G_Object(objMain,0,0,2,(pFirstResource+PR->GetResource(1))->MedPic);
			objMain->PlaceOnMap(
				(pFirstProvince+i)->x+30-(pFirstResource+PR->GetResource(1))->MedPic->w/2,
				(pFirstProvince+i)->y-(pFirstResource+PR->GetResource(1))->MedPic->h/2);
			objMain=new G_Object(objMain,0,0,2,bmpResBorder);
			objMain->PlaceOnMap(
				(pFirstProvince+i)->x+30-(pFirstResource+PR->GetResource(1))->MedPic->w/2-2,
				(pFirstProvince+i)->y-(pFirstResource+PR->GetResource(1))->MedPic->h/2-2);
		}
		else
		{
		if (PR->GetResource(0)>0)
		{
			objMain=new G_Object(objMain,0,0,2,bmpResGuardBorder);
			objMain->bTrans=true;
			objMain->PlaceOnMap(
				(pFirstProvince+i)->x+30-(pFirstResource+PR->GetResource(0))->MedPic->w/2-2,
				(pFirstProvince+i)->y-(pFirstResource+PR->GetResource(0))->MedPic->h/2-2);			
			objMain=new G_Object(objMain,0,0,2,(pFirstResource+PR->GetResource(0))->MedPic);
			objMain->PlaceOnMap(
				(pFirstProvince+i)->x+30-(pFirstResource+PR->GetResource(0))->MedPic->w/2,
				(pFirstProvince+i)->y-(pFirstResource+PR->GetResource(0))->MedPic->h/2);
			objMain=new G_Object(objMain,0,0,2,bmpResGuardBorder);
			objMain->PlaceOnMap(
				(pFirstProvince+i)->x+30-(pFirstResource+PR->GetResource(0))->MedPic->w/2-2,
				(pFirstProvince+i)->y-(pFirstResource+PR->GetResource(0))->MedPic->h/2-2);			
		}
		else
		{
			if ((PR->GetResource(-1)>0)&&(arPlayer[Player].CompareBldParam1(26,PR->ipTerrain)))
			{
				objMain=new G_Object(objMain,0,0,2,bmpResHidden);
				objMain->PlaceOnMap(
					(pFirstProvince+i)->x+30-bmpResHidden->w/2,
					(pFirstProvince+i)->y-bmpResHidden->h/2);
				objMain=new G_Object(objMain,0,0,2,bmpResBorder);
				objMain->PlaceOnMap(
					(pFirstProvince+i)->x+30-bmpResHidden->w/2-2,
					(pFirstProvince+i)->y-bmpResHidden->h/2-2);
			};
		};
		};

		WriteLog("Гербы");
		if ((PR->Pic)&&(PR->ipTerrain!=WATER_ID)&&(PR->ipTerrain!=MOUNTAIN_ID))
		{
			objMain=new G_Object(objMain,0,0,1,PR->Pic);
			objMain->PlaceOnMap(PR->PicX,PR->PicY);
			PR->Obj=objMain;

			if (PR->GetLevel()>2)
			{
				objMain=new G_Object(objMain,0,0,1,bmpTownShadow[PR->ipRace-1][PR->GetLevel()-3]);
				objMain->PlaceOnMap(PR->x-bmpTown[0][0]->w/2,PR->y-bmpTown[0][0]->h/2);
				objMain->bTrans=true;
			};
		};
		
		if (PR->ipPlayer>0)
		{
			//Герб
			if (arHumans[arPlayer[Player].iColor].arMap[0])
			{
				objMain=new G_Object(objMain,0,0,2,bmpHerb[arPlayer[PR->ipPlayer].iColor]);
				objMain->PlaceOnMap(PR->x-bmpHerb[arPlayer[PR->ipPlayer].iColor]->w/2,PR->y-bmpHerb[arPlayer[PR->ipPlayer].iColor]->h/2);
			};
			objMain=new G_Object(objMain,0,0,2,bmpFlag[arPlayer[PR->ipPlayer].iColor][1]);
			objMain->PlaceOnMap(PR->x-bmpTown[0][0]->w/2,PR->y-bmpTown[0][0]->h/2);
			if (PR->GetLevel()>5)
			{
				objMain=new G_Object(objMain,0,0,2,bmpFlag[arPlayer[PR->ipPlayer].iColor][2]);
				objMain->PlaceOnMap(PR->x-bmpTown[0][0]->w/2,PR->y-bmpTown[0][0]->h/2);
			};
			if (PR->GetLevel()>8)
			{
				objMain=new G_Object(objMain,0,0,2,bmpFlag[arPlayer[PR->ipPlayer].iColor][3]);
				objMain->PlaceOnMap(PR->x-bmpTown[0][0]->w/2,PR->y-bmpTown[0][0]->h/2);
			};			
		};
		int iCoord=0;
		if ((pFirstProvince+i)->ipPlayer==Player)
		{
			if (((pFirstProvince+i)->iDefender>0)&&(arHumans[arPlayer[Player].iColor].arMap[3]))
			{
				objMain=new G_Object(objMain,0,0,2,bmpDefender);
				objMain->PlaceOnMap(PR->x-21+(iCoord%4)*15,PR->y+20+(iCoord/4)*16);
				iCoord++;
			};
			if ((pFirstProvince+i)->GetMood()<0)
			{
				if (arHumans[arPlayer[Player].iColor].arMap[1])
				{
				switch ((pFirstProvince+i)->GetMood())
				{
				case -1:
					objMain=new G_Object(objMain,0,0,2,bmpUnhappy[0]);
					break;
				case -2:
					objMain=new G_Object(objMain,0,0,2,bmpUnhappy[1]);
					break;
				default:
					objMain=new G_Object(objMain,0,0,2,bmpUnhappy[2]);
					break;
				};				
				objMain->PlaceOnMap(PR->x-21+(iCoord%4)*15,PR->y+20+(iCoord/4)*16);
				iCoord++;
				};
				if (((pFirstProvince+i)->arUnrest[Player]>300)&&(arHumans[arPlayer[Player].iColor].arMap[2]))
				{
					objMain=new G_Object(objMain,0,0,2,bmpUnrest);
					objMain->PlaceOnMap(PR->x-21+(iCoord%4)*15,PR->y+20+(iCoord/4)*16);
					iCoord++;
				};
			};
			if ((arHumans[arPlayer[Player].iColor].arMap[4])&&((pFirstProvince+i)->iExplore<ProvLevel[(pFirstProvince+i)->GetLevel()].iExplored))
			{
				objMain=new G_Object(objMain,0,0,2,bmpNoRoom);
				objMain->PlaceOnMap(PR->x-21+(iCoord%4)*15,PR->y+20+(iCoord/4)*16);
				iCoord++;
			};
			bool bArena=false, bShop=false, bLair=false, bTemple=false;
			for (int k=0; k<MAX_PROVINCE_SITE; k++)
			{
				if (PR->arSite[k][1]<=PR->iExplore)
				{
					switch (PR->arSite[k][0])
					{
					case 41:
						if (arHumans[arPlayer[Player].iColor].arMap[6])
						{bArena=true;};
						break;
					case 35:
						if (arHumans[arPlayer[Player].iColor].arMap[8])
						{bLair=true;};
						//No break!
					case 32:
					case 33:
					case 34:
				//	case 35:
					case 36:
					case 37:
					case 38:
					case 39:
					case 40:
						if (arHumans[arPlayer[Player].iColor].arMap[7])
						{bShop=true;};
						break;						
					case 43:
					case 44:
					case 45:
						if (arHumans[arPlayer[Player].iColor].arMap[8])
							{bLair=true;};
					break;
					case 27:
						if (arHumans[arPlayer[Player].iColor].arMap[9])
						{bTemple=true;};
						break;
						};
				};
			};
			if (bArena)
			{
				objMain=new G_Object(objMain,0,0,2,bmpArena);
				objMain->PlaceOnMap(PR->x-21+(iCoord%4)*15,PR->y+20+(iCoord/4)*16);
				iCoord++;
			};
			if (bShop)
			{
				objMain=new G_Object(objMain,0,0,2,bmpShop);
				objMain->PlaceOnMap(PR->x-21+(iCoord%4)*15,PR->y+20+(iCoord/4)*16);
				iCoord++;
			};
			if (bLair)
			{
				objMain=new G_Object(objMain,0,0,2,bmpLair);
				objMain->PlaceOnMap(PR->x-21+(iCoord%4)*15,PR->y+20+(iCoord/4)*16);
				iCoord++;
			};
			if (bTemple)
			{
				objMain=new G_Object(objMain,0,0,2,bmpTemple);
				objMain->PlaceOnMap(PR->x-21+(iCoord%4)*15,PR->y+20+(iCoord/4)*16);
				iCoord++;
			};			
		};
	
		WriteLog("Здания");

		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			if (PR->arBuilding[i]>0)
			{
				objMain=new G_Object(objMain,0,0,1,(pFirstOuterBld+PR->arBuilding[i])->bmpShadow);	
				objMain->PlaceOnMap(PR->arBldPlace[i][0]-(pFirstOuterBld+PR->arBuilding[i])->bmpBld->w/2,PR->arBldPlace[i][1]-(pFirstOuterBld+PR->arBuilding[i])->bmpBld->h/2);
				objMain->bTrans=true;
			};
		};
	
		for (int j=1; j<=CurGame.iMaxPlayers; j++)
		{
			if ((arPlayer[j].bInGame)&&(PR->iNumber==arPlayer[j].ipProvince))
			{
			
				objMain=new G_Object(objMain,0,0,1,(pFirstOuterBld+23)->bmpShadow);
				objMain->PlaceOnMap(PR->arBldPlace[0][0]-(pFirstOuterBld+23)->bmpBld->w/2,PR->arBldPlace[0][1]-(pFirstOuterBld+23)->bmpBld->h/2);	
				objMain->bTrans=true;
				objMain=new G_Object(objMain,0,0,2,bmpFlag[arPlayer[PR->ipPlayer].iColor][2]);
				objMain->PlaceOnMap(PR->arBldPlace[0][0]-(pFirstOuterBld+23)->bmpBld->w/2-17,PR->arBldPlace[0][1]-(pFirstOuterBld+23)->bmpBld->h/2-27);
				objMain=new G_Object(objMain,0,0,2,bmpFlag[arPlayer[PR->ipPlayer].iColor][2]);
				objMain->PlaceOnMap(PR->arBldPlace[0][0]-(pFirstOuterBld+23)->bmpBld->w/2-2,PR->arBldPlace[0][1]-(pFirstOuterBld+23)->bmpBld->h/2-12);
				objMain=new G_Object(objMain,0,0,2,bmpFlag[arPlayer[PR->ipPlayer].iColor][2]);
				objMain->PlaceOnMap(PR->arBldPlace[0][0]-(pFirstOuterBld+23)->bmpBld->w/2-48,PR->arBldPlace[0][1]-(pFirstOuterBld+23)->bmpBld->h/2-22);
			};
		};
	
		if (PR->GetSiegeIndex()==0)
		{
			objMain=new G_Object(objMain,0,0,3,bmpRebelDoll);
			objMain->PlaceOnMap(
			PR->x-bmpRebelDoll->w/2,
			PR->y-bmpRebelDoll->h);			
		};
		};
	};
	
	WriteLog("Герои");

	for (int j=1; j<=CurGame.iMaxPlayers; j++)
	{

	for (int i=1; i<=MapSize; i++)
	{
		*(Prov+i)=0;
	};	

	for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[j].arHero[i])&&(arPlayer[j].arHero[i]->iDead<0))
		{
			*(Prov+arPlayer[j].arHero[i]->pProvince->iNumber)+=1;
		};
	};
	for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[j].arHero[i])&&(arPlayer[j].arHero[i]->iDead<0)&&(*(arPlayer[Player].pFog+arPlayer[j].arHero[i]->ipProvince)==0))
		{
			if (*(Prov+arPlayer[j].arHero[i]->pProvince->iNumber)>0)
			{
			objMain=new G_Object(objMain,0,0,3,bmpHeroDoll[arPlayer[j].arHero[i]->ipClass1-1]);
			objMain->Shadow=bmpHeroDollShadow[arPlayer[j].arHero[i]->ipClass1-1];
			objMain->PlaceOnMap(
			arPlayer[j].arHero[i]->pProvince->x-bmpHeroDoll[arPlayer[j].arHero[i]->ipClass1-1]->w/2,
			arPlayer[j].arHero[i]->pProvince->y-bmpHeroDoll[arPlayer[j].arHero[i]->ipClass1-1]->h);
			arPlayer[j].arHero[i]->ObjN=objMain;
			if (Player==j)
			{
			objMain=arPlayer[j].arHeroMask[i].Init(0,0,1,bmpHeroDoll[arPlayer[j].arHero[i]->ipClass1-1],
				bmpHeroDollS[arPlayer[j].arHero[i]->ipClass1-1],objMain,4);
			objMain->PlaceOnMap(
			arPlayer[j].arHero[i]->pProvince->x-bmpHeroDoll[arPlayer[j].arHero[i]->ipClass1-1]->w/2,
			arPlayer[j].arHero[i]->pProvince->y-bmpHeroDoll[arPlayer[j].arHero[i]->ipClass1-1]->h);
			arPlayer[j].arHero[i]->ObjA=objMain;
			};
			objMain=new G_Object(objMain,0,0,2,bmpFlag[arPlayer[j].iColor][0]);
			objMain->PlaceOnMap(
			arPlayer[j].arHero[i]->pProvince->x-bmpFlag[arPlayer[j].iColor][0]->w/2-3,
			arPlayer[j].arHero[i]->pProvince->y-bmpFlag[arPlayer[j].iColor][0]->h);
			if (*(Prov+arPlayer[j].arHero[i]->pProvince->iNumber)>1)
			{
				objMain=new G_Object(objMain,0,0,2,bmpFlag[arPlayer[j].iColor][0]);
				objMain->PlaceOnMap(
				arPlayer[j].arHero[i]->pProvince->x-bmpFlag[arPlayer[j].iColor][0]->w/2+1,
				arPlayer[j].arHero[i]->pProvince->y-bmpFlag[arPlayer[j].iColor][0]->h-5);
				*(Prov+arPlayer[j].arHero[i]->pProvince->iNumber)=-1;
			};
			}
			else
			{
				if (j==Player)
				{
					arPlayer[j].arHeroMask[i].Init(0,0,0,bmpGUI_Dead,bmpGUI_Dead,objEmpty,-1);
				};
			};
		};
	};
	};
	for (int i=1; i<=MapSize; i++)
	{
		*(Prov+i)=0;
	};	

	for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0))
		{
			*(Prov+arPlayer[Player].arHero[i]->pProvince->iNumber)+=1;
		};
	};
	return objMain;

};

void MainScreen(int Player)
{
	iAntisleep=iAntisleepDelay;
	bool bEnterPressed=false, bKeyGPressed=false;
	bool bIdle;
	CurrentPlayer=Player;
	int Condition=0,	//Сосотояние инфо-окна 1 - выбрана провинция, 2 - меню строительства
		Member=arPlayer[Player].ipProvince,	//Индекс выбранного элемента
		Limit=0,		//Максимальное количество объектов в списке выбора
		Shift=0,		//Смещение в списке выбора
		Choosen=0;		//Выбранный из списка элемент
	bool bTutFree=true;	//Флаг использования подсказки
	if (arPlayer[Player].iCondition<15)
	{
		if ((pFirstProvince+arPlayer[Player].iMember)->ipPlayer==Player)
		{
			Condition=0;
			Member=arPlayer[Player].iMember;
		};
	};
	if (arPlayer[Player].iCondition>15)
	{
		if ((arPlayer[Player].arHero[arPlayer[Player].iMember])&&(arPlayer[Player].arHero[arPlayer[Player].iMember]->iDead<0))
		{
			Condition=20;
			Member=arPlayer[Player].iMember;
		};
	};	
	if ((Tutorial.Map)&&(arPlayer[Player].arHero[0]->iDead<0)&&(Tutorial.Move<20))
	{
		Member=arPlayer[Player].arHero[0]->ipProvince;
		Condition=0;
	};
	
	if (Condition==0)
	{CentreMap((pFirstProvince+Member)->x, (pFirstProvince+Member)->y);}
	else
	{CentreMap(arPlayer[Player].arHero[Member]->pProvince->x, arPlayer[Player].arHero[Member]->pProvince->y);};
	


	char * InfoString;
	const int BarX=707;//750;
	const int BarY=243;//280;
	const int LowY=768-bmpGUI_LowPanel->h;//723;

	/*int Iteration=0;
	char stLog[150];
	bool bLogger=true;*/


	bool bTutHint=false;
	G_Object objEmpty(NULL,0,0,-1,bmpGUI_Dead);
	BITMAP *bmpIncome=NULL;
	BITMAP * bmpInfoBar=create_bitmap(bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
	blit(bmpGUI_ProvinceBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
	BITMAP * bmpLowPanel=create_bitmap(bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);
	blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);
	set_mouse_sprite(bmpMousePointer[0]);
	bool Redraw=true;
	bool bFirst=true;
	int iRitualer=0;	//Номер провинции для анимации ритуала
	GoPath=NULL;
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if (arPlayer[i].iWall==-1000)
		{
			(pFirstProvince+arPlayer[i].ipProvince)->DrawPic(false);
			arPlayer[i].iWall=0;
		};
	};
	while (Redraw)
	{
	MakeFog(Player);
	G_Object *objMain=new G_Object(NULL,0,LowY,4,bmpLowPanel);
//	G_Object *objLow=objMain;
	objMain=new G_Object(objMain,BarX,BarY,5,bmpInfoBar);
	objMain=new G_Object(objMain,0,0,-1,bmpInfoBar);
	G_Object * objShadow=objMain;
	objShadow->bTrans=true;
	objShadow->iOpacity=50;

	UI_Button1 buttMenu;
	objMain=buttMenu.Init(8,LowY+62,1,6,bmpButton[104],bmpButton[105],bmpButton[106],bmpButton[106],objMain);
	buttMenu.Activate(1);
	buttMenu.iHint=96;
	buttMenu.ipHotkey=KEY_ESC;
	UI_Button1 buttNextHero;
	objMain=buttNextHero.Init(685+1,LowY+65,1,6,bmpButton[178],bmpButton[179],bmpButton[180],bmpButton[181],objMain);
	bool bNext=false;
	for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0)&&((Condition<17)||(Member!=i)))
		{bNext=true;};
	};
	if (bNext)
	{buttNextHero.Activate(1);}
	else
	{buttNextHero.Activate(0);};
	buttNextHero.ipHotkey=KEY_N;
	buttNextHero.ipHotkey2=KEY_SPACE;
	buttNextHero.iHint=151;
	UI_Button1 buttCastle;
	if (arPlayer[Player].iCanInnerBld>0)
	{objMain=buttCastle.Init(745+1,LowY+65,1,6,bmpButton[92],bmpButton[93],bmpButton[94],bmpButton[94],objMain);}
	else
	{objMain=buttCastle.Init(745+1,LowY+65,1,6,bmpButton[319],bmpButton[320],bmpButton[321],bmpButton[321],objMain);};
	buttCastle.Activate(1);
	buttCastle.iHint=97;
	buttCastle.ipHotkey=KEY_C;
	UI_Button1 buttDipl;	
	objMain=buttDipl.Init(805+1,LowY+65,1,6,bmpButton[95],bmpButton[96],bmpButton[97],bmpButton[255],objMain);
	buttDipl.Activate(0);
	if (CurGame.bDiplomacy)
	{
		for (int i=1; i<=CurGame.iMaxPlayers; i++)
		{
			if ((Player!=i)&&(arPlayer[Player].Politic[i].Treaty>0)&&(arPlayer[i].bInGame)&&(arPlayer[i].Control<3))
			{buttDipl.Activate(1);};
		};
	};
	buttDipl.iHint=98;
	UI_Button1 buttStat;
	objMain=buttStat.Init(865+1,LowY+65,1,6,bmpButton[98],bmpButton[99],bmpButton[100],bmpButton[100],objMain);
	buttStat.Activate(1);
	buttStat.iHint=99;
	UI_Button1 buttEnd;
	objMain=buttEnd.Init(954,LowY+62,1,6,bmpButton[101],bmpButton[102],bmpButton[103],bmpButton[103],objMain);
	buttEnd.Activate(1);
	buttEnd.iHint=100;
	buttEnd.ipHotkey=KEY_E;
	
	UI_Button1 buttGuard;
	if (arPlayer[Player].iCanHire>0)
	{objMain=buttGuard.Init(BarX+40,BarY+396,1,6,bmpButton[64],bmpButton[65],bmpButton[66],bmpButton[111],objMain);}
	else
	{objMain=buttGuard.Init(BarX+40,BarY+396,1,6,bmpButton[375],bmpButton[376],bmpButton[377],bmpButton[111],objMain);}
	buttGuard.Activate(1);
	buttGuard.iHint=92;
	buttGuard.ipHotkey=KEY_Q;
	UI_Button1 buttBuild;
	if (arPlayer[Player].iCanOuterBld>0)
	{objMain=buttBuild.Init(BarX+40+60,BarY+396,1,6,bmpButton[67],bmpButton[68],bmpButton[69],bmpButton[91],objMain);}
	else
	{objMain=buttBuild.Init(BarX+40+60,BarY+396,1,6,bmpButton[378],bmpButton[379],bmpButton[380],bmpButton[91],objMain);}
	buttBuild.Activate(1);
	buttBuild.iHint=93;
	buttBuild.ipHotkey=KEY_W;
	UI_Button1 buttGarrison;
	objMain=buttGarrison.Init(BarX+40+60*2,BarY+396,1,6,bmpButton[70],bmpButton[71],bmpButton[72],bmpButton[157],objMain);
	buttGarrison.Activate(0);
	buttGarrison.iHint=94;
	buttGarrison.ipHotkey=KEY_D;
	UI_Button1 buttRitual;
	objMain=buttRitual.Init(BarX+40+60*3,BarY+396,1,6,bmpButton[73],bmpButton[74],bmpButton[75],bmpButton[120],objMain);
	buttRitual.Activate(1);
	buttRitual.iHint=95;
	buttRitual.ipHotkey=KEY_R;

	UI_Button1 buttExplore;
	objMain=buttExplore.Init(BarX+40,BarY+354,1,6,bmpButton[125],bmpButton[126],bmpButton[127],bmpButton[128],objMain);
	buttExplore.Activate(-1);
	buttExplore.iHint=555;
	buttExplore.ipHotkey=KEY_Q;
	UI_Button1 buttPillage;
	objMain=buttPillage.Init(BarX+40+60*2,BarY+354,1,6,bmpButton[129],bmpButton[130],bmpButton[131],bmpButton[132],objMain);
	buttPillage.Activate(-1);
	buttPillage.iHint=556;
	buttPillage.ipHotkey=KEY_T;
	UI_Button1 buttEnter;	
	objMain=buttEnter.Init(BarX+40+60,BarY+354,1,6,bmpButton[133],bmpButton[134],bmpButton[135],bmpButton[136],objMain);
	buttEnter.Activate(-1);
	buttEnter.iHint=557;
	buttEnter.ipHotkey=KEY_W;
	UI_Button1 buttAttack;	
	objMain=buttAttack.Init(BarX+40+60*2,BarY+354,1,6,bmpButton[137],bmpButton[138],bmpButton[139],bmpButton[140],objMain);
	buttAttack.Activate(-1);
	buttAttack.iHint=558;
	buttAttack.ipHotkey=KEY_T;
	UI_Button1 buttHeroRitual;
	objMain=buttHeroRitual.Init(BarX+40+60*3,BarY+354,1,6,bmpButton[73],bmpButton[74],bmpButton[75],bmpButton[120],objMain);
	buttHeroRitual.Activate(-1);
	buttHeroRitual.iHint=95;
	buttHeroRitual.ipHotkey=KEY_R;
	UI_Button1 buttSkip;
	objMain=buttSkip.Init(BarX+40,BarY+354,1,6,bmpButton[84],bmpButton[85],bmpButton[86],bmpButton[86],objMain);
	buttSkip.Activate(-1);
	buttSkip.iHint=101;
	buttSkip.ipHotkey=KEY_Q;
	
	UI_Button1 buttShop;
	objMain=buttShop.Init(BarX+40,BarY+396,1,6,bmpButton[141],bmpButton[142],bmpButton[143],bmpButton[144],objMain);
	buttShop.Activate(-1);
	buttShop.iHint=102;
	buttShop.ipHotkey=KEY_A;
	UI_Button1 buttLibrary;
	objMain=buttLibrary.Init(BarX+40+60,BarY+396,1,6,bmpButton[145],bmpButton[146],bmpButton[147],bmpButton[148],objMain);
	buttLibrary.Activate(-1);
	buttLibrary.iHint=103;
	buttLibrary.ipHotkey=KEY_S;
	UI_Button1 buttHeroGarr;
	objMain=buttHeroGarr.Init(BarX+40+60*2,BarY+396,1,6,bmpButton[149],bmpButton[150],bmpButton[151],bmpButton[152],objMain);
	buttHeroGarr.Activate(-1);
	buttHeroGarr.iHint=104;
	buttHeroGarr.ipHotkey=KEY_D;
	UI_Button1 buttExchange;
	objMain=buttExchange.Init(BarX+40+60*3,BarY+396,1,6,bmpButton[153],bmpButton[154],bmpButton[155],bmpButton[156],objMain);
	buttExchange.Activate(-1);
	buttExchange.iHint=105;
	buttExchange.ipHotkey=KEY_F;
	
	UI_Button1 buttScrollUp;
	objMain=buttScrollUp.Init(BarX+40,BarY+396,1,6,bmpButton[76],bmpButton[77],bmpButton[78],bmpButton[79],objMain);
	buttScrollUp.Activate(-1);
	buttScrollUp.ipHotkey=-1;
	UI_Button1 buttCancel;
	objMain=buttCancel.Init(BarX+40+60,BarY+396,1,6,bmpButton[84],bmpButton[85],bmpButton[86],bmpButton[86],objMain);
	buttCancel.Activate(-1);
	buttCancel.ipHotkey=KEY_ESC;
	buttCancel.iHint=106;
	UI_Button1 buttScrollDown;
	objMain=buttScrollDown.Init(BarX+40+60*2,BarY+396,1,6,bmpButton[80],bmpButton[81],bmpButton[82],bmpButton[83],objMain);
	buttScrollDown.Activate(-1);
	buttScrollDown.ipHotkey=-2;
	UI_Button2 buttBldFilter;
	objMain=buttBldFilter.Init(BarX+40+60*3,BarY+396,1,6,bmpButton[87],bmpButton[88],bmpButton[89],bmpButton[90],bmpButton[90],objMain);
	buttBldFilter.Activate(-1,arPlayer[Player].bBldFilter);
	buttBldFilter.iHint=107;
	buttBldFilter.iHint1=108;
	UI_Button2 buttDefFilter;
	objMain=buttDefFilter.Init(BarX+40+60*3,BarY+396,1,6,bmpButton[112],bmpButton[113],bmpButton[114],bmpButton[115],bmpButton[115],objMain);
	buttDefFilter.Activate(-1,arPlayer[Player].bDefFilter);
	buttDefFilter.iHint=109;
	buttDefFilter.iHint1=110;
	UI_Button2 buttRitFilter;
	objMain=buttRitFilter.Init(BarX+40+60*3,BarY+396,1,6,bmpButton[121],bmpButton[122],bmpButton[123],bmpButton[124],bmpButton[124],objMain);
	buttRitFilter.Activate(-1,arPlayer[Player].bRitFilter);
	buttRitFilter.iHint=111;
	buttRitFilter.iHint1=112;

	UI_Button1 buttBuild2;
	objMain=buttBuild2.Init(BarX+40+28,BarY+396,1,6,bmpButton[67],bmpButton[68],bmpButton[69],bmpButton[91],objMain);
	buttBuild2.Activate(-1);
	buttBuild2.iHint=93;
	buttBuild2.wavSound=wavBuild;
	UI_Button1 buttCancel2;
	objMain=buttCancel2.Init(BarX+40+60*2+30,BarY+396,1,6,bmpButton[84],bmpButton[85],bmpButton[86],bmpButton[86],objMain);
	buttCancel2.Activate(-1);
	buttCancel2.ipHotkey=KEY_ESC;
	buttCancel2.iHint=106;
	UI_Button1 buttRaze;
	objMain=buttRaze.Init(BarX+40+30,BarY+396,1,6,bmpButton[107],bmpButton[108],bmpButton[109],bmpButton[110],objMain);
	buttRaze.Activate(-1);
	buttRaze.iHint=113;
	UI_Button1 buttRitual2;
	objMain=buttRitual2.Init(BarX+40+30,BarY+396,1,6,bmpButton[73],bmpButton[74],bmpButton[75],bmpButton[120],objMain);
	buttRitual2.Activate(-1);
	buttRitual2.iHint=95;
	
	
	UI_Button1 buttGuard2;
	objMain=buttGuard2.Init(BarX+40+30,BarY+396,1,6,bmpButton[64],bmpButton[65],bmpButton[66],bmpButton[111],objMain);
	buttGuard2.Activate(-1);
	buttGuard2.iHint=92;
//	buttGuard2.wavSound=wavCoins;
	UI_Button1 buttDismiss;
	objMain=buttDismiss.Init(BarX+40+30,BarY+396,1,6,bmpButton[116],bmpButton[117],bmpButton[118],bmpButton[119],objMain);
	buttDismiss.Activate(-1);
	buttDismiss.iHint=114;

	UI_Button1 hotHeroes[8];
	for (int i=0; i<8; i++)
	{
		objMain=hotHeroes[i].Init(1025,0,1,-1,bmpButton[64],bmpButton[65],bmpButton[66],bmpButton[111],objMain);
		hotHeroes[i].ipHotkey=KEY_1+i;
		hotHeroes[i].Activate(1);
	};

	UI_Mask mskRace;
	objMain=mskRace.Init(BarX+46,BarY+203,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objMain,7);

	UI_Mask mskInfoBar;
	objMain=mskInfoBar.Init(BarX,BarY,1,bmpInfoBar,bmpGUI_Dead,objMain,-1);
	UI_Mask mskLowPanel;
	objMain=mskLowPanel.Init(0,LowY,1,bmpLowPanel,bmpGUI_Dead,objMain,-1);

	UI_Mask mskHeroPic;
	objMain=mskHeroPic.Init(BarX+127,BarY+33,0,bmpGUI_SpellMask,bmpGUI_SpellSelect,objMain,7);
	mskHeroPic.iHint=146;

	UI_Mask mskBuild[BAR_SIZE];
	for (int i=0; i<BAR_SIZE; i++)
	{		
		objMain=mskBuild[i].Init(BarX+48,BarY+37+i*59,0,bmpGUI_BuildMask,bmpGUI_BuildSelect,objMain,7);
	};
	UI_Mask mskMain[4];
	for (int i=0; i<4; i++)
	{		
		objMain=mskMain[i].Init(BarX+34+i*60,BarY+302,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objMain,7);
		mskMain[i].iHint=115+i;
	};

	//Маски для параметров героя
	UI_Mask1 mskParam[8];
	mskParam[0].Init(BarX+72,BarY+41,56,54,0);	//Опыт	
	mskParam[1].Init(BarX+190,BarY+41,56,54,0);	//Жизнь
	mskParam[2].Init(BarX+69,BarY+157,58,28,0);	//Командование
	mskParam[3].Init(BarX+127,BarY+157,58,28,0);//Магия
	mskParam[4].Init(BarX+186,BarY+157,58,28,0);//Здоровье
	mskParam[5].Init(BarX+69,BarY+188,58,28,0);	//Инициатива
	mskParam[6].Init(BarX+127,BarY+188,58,28,0);//Осада
	mskParam[7].Init(BarX+186,BarY+188,58,28,0);//Мобильность

	UI_Mask1 mskRes[11];
	for (int i=0; i<6; i++)
	{mskRes[i].Init(BarX+110+i*26,BarY+97,20,20,1);};
	mskRes[6].Init(BarX+110,BarY+64,72,24,1);
	mskRes[7].Init(BarX+183,BarY+64,72,24,1);
	mskRes[8].Init(BarX+51,BarY+132,55,24,1);
	mskRes[9].Init(BarX+110,BarY+132,72,24,1);
	mskRes[10].Init(BarX+183,BarY+132,72,24,1);

	UI_Mask1 mskCD;
	mskCD.Init(BarX+188,BarY+96,77,23,1);
	mskCD.iHint=394;


	UI_Mask1 mskCheat;
	mskCheat.Init(926,729,25,35,1);

	UI_Mask1 mskAlchemy;
	mskAlchemy.Init(72,727,265,40,1);
	mskAlchemy.iHint=352;

	UI_TextBox txtDs;
	objMain=txtDs.Init(BarX+59,BarY+163,195,184,7,COLOR_1,Font2,objMain,BarX+145,BarY+137,BarX+145,BarY+359);
	txtDs.bShadow=false;
	txtDs.Hide();

	UI_TextBox txtDs1;
	objMain=txtDs1.Init(BarX+59,BarY+196,195,159,7,COLOR_1,Font2,objMain,BarX+145,BarY+172,BarX+145,BarY+362);
	txtDs1.bShadow=false;
	txtDs1.Hide();

	UI_TextBox txtDs2;
	objMain=txtDs2.Init(BarX+59,BarY+128,195,138,7,COLOR_1,Font2,objMain,BarX+145,BarY+104,BarX+145,BarY+274);
	txtDs2.bShadow=false;
	txtDs2.Hide();

	objMain=new G_Object(objMain,20,20,-1,bmpGUI_Hint);
	G_Object *objTutorial=objMain;
	UI_Mask mskTutorial;
	objMain=mskTutorial.Init(20,20,1,bmpGUI_Hint,bmpEmpty,objMain,-1);
	UI_TextBox txtTutorial;
	objMain=txtTutorial.Init(40,50,297,200,7,COLOR_1,Font2,objMain);
	txtTutorial.bShadow=false;
	txtTutorial.Hide();

	objMain=txtError.Init(BarX+56,BarY+44,207,340,7,COLOR_2,Font2,objMain);
	txtError.Hide();

	G_Object *objMapSelect;
	objMain=new G_Object(objMain,0,0,-1,bmpMapSelect);
	objMapSelect=objMain;
	objMapSelect->bTrans=true;
	objMapSelect->iOpacity=50;
	objMapSelect->iDelay=4;




	G_Object *objBroken, *objAnvil1, *objAnvil2, *objEnchant;
	objMain=new G_Object(objMain,827+26,224+16,-1,bmpGUI_Anvil1);
	objAnvil1=objMain;
	objMain=new G_Object(objMain,827+26,224+16,-1,bmpGUI_Anvil2);
	objAnvil2=objMain;
	objMain=new G_Object(objMain,827+27,224+15,-1,bmpStuff[42]);
	objEnchant=objMain;
	objMain=new G_Object(objMain,827,224,-1,bmpGUI_Broken);
	objBroken=objMain;
	objAnvil2->bTrans=true;
	objAnvil2->iOpacity=0;
	int Anvil=0, AddAnvil=10;
	UI_Mask mskBroken;
	mskBroken.Init(827,224,1,bmpGUI_Broken,bmpGUI_Broken,NULL,1);

	UI_Mask1 mskExplore, mskMood, mskResource, mskRace1, mskLevel, mskGoldInc, mskGemInc, mskTerra;
	mskTerra.Init(780,310,190,22,1);
	mskTerra.iHint=437;
	mskExplore.Init(760,340,210,22,1);
	mskExplore.iHint=400;
	mskMood.Init(818,481,146,24,1);
	mskMood.iHint=401;
	mskResource.Init(763,411,200,22,1);
	mskResource.iHint=399;
	mskRace1.Init(818,451,146,22,1);
	mskRace.iHint=402;
	mskRace1.iHint=402;
	mskLevel.Init(761,511,200,24,1);
	mskLevel.iHint=403;
	mskGoldInc.Init(760,376,103,34,1);
	mskGoldInc.iHint=405;
	mskGemInc.Init(865,376,103,34,1);
	mskGemInc.iHint=406;

	
	G_Object *objIncome=NULL;
	int iIncomeY=0, iIncTimer=0;
	/*if ((CurGame.iCurrentTurn==0)&&(Iteration<1000)&&(Iteration%10==0))
	{
		sprintf(stLog,"Step: 0, Inc: %d, bFirst: %d",iIncomeY,bFirst);
		FileLog(stLog);
	};*/

	//Анимация дохода
	if ((bFirst)&&(CurGame.iCurrentTurn>0))
	{
		iIncomeY=46;
		iIncTimer=100;
		bShowAnim=true;
		bFirst=false;
		bmpIncome=create_bitmap(352,iIncomeY);
		blit(bmpGUI_LowPanel,bmpIncome,321,60,0,0,352,iIncomeY);
		objMain=new G_Object(objMain,321,768-iIncomeY,7,bmpIncome);
		objIncome=objMain;
		textprintf_ex(bmpIncome,Font1,30,19,COLOR_2,-1,"%s: %d",(pFirstPhrase+51)->str,CurGame.iCurrentTurn);
		if (arPlayer[Player].GoldInc>0)
		{textprintf_ex(bmpIncome,Font1,150,19,makecol(50,250,50),-1,"+%d",arPlayer[Player].GoldInc);}
		else
		{textprintf_ex(bmpIncome,Font1,150,19,makecol(250,50,50),-1,"%d",arPlayer[Player].GoldInc);};
		int Add=24, Tst=abs(arPlayer[Player].GoldInc/10);
		while (Tst>0)
		{
			Tst/=10;
			Add+=9;
		};		
		draw_sprite(bmpIncome,bmpGUI_Gold,150+Add,15);

		if (arPlayer[Player].GemInc>0)
		{textprintf_ex(bmpIncome,Font1,220+Add,19,makecol(50,250,50),-1,"+%d",arPlayer[Player].GemInc);}
		else
		{textprintf_ex(bmpIncome,Font1,220+Add,19,makecol(250,50,50),-1,"%d",arPlayer[Player].GemInc);};
		Add+=24, Tst=abs(arPlayer[Player].GemInc/10);
		while (Tst>0)
		{
			Tst/=10;
			Add+=9;
		};
		draw_sprite(bmpIncome,bmpGUI_Crystal,220+Add,13);		
	}
	else
	{bGlobalError=true;};

	/*if ((CurGame.iCurrentTurn==0)&&(Iteration<1000)&&(Iteration%10==0))
	{
		sprintf(stLog,"Step: 1, Inc: %d, bFirst: %d",iIncomeY,bFirst);
		FileLog(stLog);
	};*/




	objMain=new G_Object(objMain,0,0,-1,bmpHeroSelect);
	G_Object * objHeroSelect=objMain;
	G_Object * objHeroQuest[HERO_QUEST_LIMIT];
	for (int i=0; i<HERO_QUEST_LIMIT; i++)
	{
		objHeroQuest[i]=new G_Object(objMain,0,0,-1,bmpQuest[0]);
		objMain=objHeroQuest[i];
	};

	int *Prov=new int [MapSize+1];

	objMain=ShowMap(Player,objMain,Prov,&objEmpty);

	/*if ((CurGame.iCurrentTurn==0)&&(Iteration<1000)&&(Iteration%10==0))
	{
		sprintf(stLog,"Step: 2, Inc: %d, bFirst: %d",iIncomeY,bFirst);
		FileLog(stLog);
	};*/

	if (iRitualer)
	{
		objMain=new G_Object(objMain,0,0,3,bmpEmpty);
		objMain->PlaceOnMap((pFirstProvince+iRitualer)->x,(pFirstProvince+iRitualer)->y);
//		int dx=(pFirstProvince+iRitualer)->x-(pFirstProvince+iRitualer)->Obj->MapX-20;
//		int dy=(pFirstProvince+iRitualer)->y-(pFirstProvince+iRitualer)->Obj->MapY-20;
		objMain->Make_Stars(objMain,80,30,-20,-20,40,40,180,100,250);
		iRitualer=0;
	};

	/*if ((CurGame.iCurrentTurn==0)&&(Iteration<1000)&&(Iteration%10==0))
	{
		sprintf(stLog,"Step: 3, Inc: %d, bFirst: %d",iIncomeY,bFirst);
		FileLog(stLog);
	};*/

	
	/*for (int i=0; i<MAX_HERO; i++)
	{
		if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0))
		{
			if (*(Prov+arPlayer[Player].arHero[i]->pProvince->iNumber)>0)
			{
			objMain=new G_Object(objMain,0,0,3,bmpHeroDoll[arPlayer[Player].arHero[i]->ipClass1-1]);
			objMain->PlaceOnMap(
			arPlayer[Player].arHero[i]->pProvince->x-bmpHeroDoll[arPlayer[Player].arHero[i]->ipClass1-1]->w/2,
			arPlayer[Player].arHero[i]->pProvince->y-bmpHeroDoll[arPlayer[Player].arHero[i]->ipClass1-1]->h);
			arPlayer[Player].arHero[i]->ObjN=objMain;
			objMain=arPlayer[Player].arHeroMask[i].Init(0,0,1,bmpHeroDoll[arPlayer[Player].arHero[i]->ipClass1-1],
				bmpHeroDollS[arPlayer[Player].arHero[i]->ipClass1-1],objMain,4);
			objMain->PlaceOnMap(
			arPlayer[Player].arHero[i]->pProvince->x-bmpHeroDoll[arPlayer[Player].arHero[i]->ipClass1-1]->w/2,
			arPlayer[Player].arHero[i]->pProvince->y-bmpHeroDoll[arPlayer[Player].arHero[i]->ipClass1-1]->h);
			arPlayer[Player].arHero[i]->ObjA=objMain;
			objMain=new G_Object(objMain,0,0,2,bmpFlag[arPlayer[Player].iColor]);
			objMain->PlaceOnMap(
			arPlayer[Player].arHero[i]->pProvince->x-bmpFlag[arPlayer[Player].iColor]->w/2-3,
			arPlayer[Player].arHero[i]->pProvince->y-bmpFlag[arPlayer[Player].iColor]->h);
			if (*(Prov+arPlayer[Player].arHero[i]->pProvince->iNumber)>1)
			{
				objMain=new G_Object(objMain,0,0,2,bmpFlag[arPlayer[Player].iColor]);
				objMain->PlaceOnMap(
				arPlayer[Player].arHero[i]->pProvince->x-bmpFlag[arPlayer[Player].iColor]->w/2+1,
				arPlayer[Player].arHero[i]->pProvince->y-bmpFlag[arPlayer[Player].iColor]->h-5);
				*(Prov+arPlayer[Player].arHero[i]->pProvince->iNumber)=-1;
			};
			}
			else
			{
				arPlayer[Player].arHeroMask[i].Init(0,0,0,bmpGUI_Dead,bmpGUI_Dead,&objEmpty,-1);
			};
		};
	};*/
	Redraw=false;
	bool bEndTurn=false, bTutor=true;
	bHourGlass=false;
	int Check;

	while ((!bEndTurn)&&(!Redraw)&&(!bExitGame))
	{
		if ((CurGame.iCurrentTurn==0)&&(WorldRules.iMode>0)&&((pFirstWorldRules+WorldRules.iMode)->ipStartEvent>0)&&(WorldRules.arTrigger[0]==0))
		{
			(pFirstProvince+arPlayer[Player].ipProvince)->StaticEvent((pFirstWorldRules+WorldRules.iMode)->ipStartEvent,Player);
			WorldRules.arTrigger[0]=1;
		};
		try {
		if (bTutor)
		{

		//****Обучающая миссия****

		//Смерть героя в первой миссии
		if ((Tutorial.Map)&&(Tutorial.Death==1)&&(Tutorial.buttCastle==0))
		{
			Tutorial.Death=0;
			bTutHint=true;
			ScrollSound();
			txtTutorial.Activate((pFirstTutText+54)->msg);
			objTutorial->iLayer=6;
			bTutFree=false;
		};

		//Выделение героя
		if ((Tutorial.Map)&&(Tutorial.Move==1))
		{
			Tutorial.Move=2;
			bTutHint=true;
			ScrollSound();
			txtTutorial.Activate((pFirstTutText+1)->msg);
			objTutorial->iLayer=6;
			buttNextHero.bBlink=true;
		};

		//Лавка1
		if ((Tutorial.Map)&&(Tutorial.Move==4)&&(arPlayer[Player].arHero[0]->ipProvince==27)&&(iIncTimer<100))
		{
			Tutorial.Move=5;
			(pFirstCampDialog+6+arPlayer[Player].arHero[0]->ipClass1)->Show(1,0,0,0,NULL,NULL);			
			Tutorial.buttEnd=0;
			switch (arPlayer[Player].arHero[0]->ipClass1)
			{
			case 1:
			case 2:
				Tutorial.Trade=1;
				Tutorial.buttShop=1;
				buttShop.bBlink=true;
				txtTutorial.Activate((pFirstTutText+4)->msg);
				arPlayer[1].Gold+=200;
				break;
			case 3:
				Tutorial.Hire=1;
				Tutorial.buttGarrison=1;
				buttHeroGarr.bBlink=true;
				txtTutorial.Activate((pFirstTutText+14)->msg);
				arPlayer[1].Gold+=200;
				break;
			case 4:
				Tutorial.Library=1;
				Tutorial.buttLibrary=1;
				buttLibrary.bBlink=true;
				txtTutorial.Activate((pFirstTutText+20)->msg);
				break;
			};
		//	if (Tutorial.Mode)
			{
				bTutHint=true;
				ScrollSound();
				objTutorial->iLayer=6;
				buttNextHero.bBlink=true;
			};
		};

		//Лавка 2
		if ((Tutorial.Map)&&(Tutorial.Move==8)&&(arPlayer[Player].arHero[0]->ipProvince==27)&&(iIncTimer<100)&&(Tutorial.Site==0)&&((pFirstProvince+27)->arSite[0][0]==0))
		{
			Tutorial.Move=9;			
			Tutorial.buttEnd=0;
			Tutorial.buttEnter=0;
			switch (arPlayer[Player].arHero[0]->ipClass1)
			{
			case 1:
			case 2:
				(pFirstCampDialog+17)->Show(1,0,0,0,NULL,NULL);
				Tutorial.Hire=1;
				Tutorial.buttGarrison=1;
				buttHeroGarr.bBlink=true;
				txtTutorial.Activate((pFirstTutText+14)->msg);
				arPlayer[1].Gold+=200;
				break;				
			case 3:
				(pFirstCampDialog+19)->Show(1,0,0,0,NULL,NULL);
				Tutorial.Trade=1;
				Tutorial.buttShop=1;
				buttShop.bBlink=true;
				txtTutorial.Activate((pFirstTutText+4)->msg);
				arPlayer[1].Gold+=200;
				break;				
			case 4:
				(pFirstCampDialog+18)->Show(1,0,0,0,NULL,NULL);
				Tutorial.Hire=1;
				Tutorial.buttGarrison=1;
				buttHeroGarr.bBlink=true;
				txtTutorial.Activate((pFirstTutText+14)->msg);
				arPlayer[1].Gold+=200;
				break;
			};
	//		if (Tutorial.Mode)
			{
				bTutHint=true;
				ScrollSound();
				objTutorial->iLayer=6;
				buttNextHero.bBlink=true;
			};
		};

		//Орки
		if ((Tutorial.Map)&&(Tutorial.Move==10))
		{
			Tutorial.Move=11;
			objTutorial->iLayer=-1;
			txtTutorial.Hide();
			objMain=ShowActiveObjects(objMain,10,true,MX,MY);
			int Answer=(pFirstCampDialog+20)->Show(1,1,0,0,NULL,NULL);
			if (Answer==2)
			{
				char Txt[500];
				sprintf(Txt,(pFirstCampDialog+21)->sText,(pFirstProvince+21)->sName);
				(pFirstCampDialog+21)->Show(1,1,0,0,NULL,Txt);
			};
	//		if (Tutorial.Mode)
			{
				bTutHint=true;
				ScrollSound();
				txtTutorial.Activate((pFirstTutText+25)->msg);
				objTutorial->iLayer=6;
				buttEnter.bBlink=true;
			};
			Tutorial.buttEnter=1;
			buttEnter.Activate(1);			
			(pFirstProvince+27)->iExplore=2;
			Tutorial.Site=1;
		};

		//Лавка 3
		if ((Tutorial.Map)&&(Tutorial.Move==12)&&(arPlayer[Player].arHero[0]->ipProvince==27)&&(iIncTimer<100)&&(Tutorial.Site==1)&&((pFirstProvince+27)->arSite[1][0]==0))
		{
			Tutorial.Move=13;			
			Tutorial.buttEnd=0;
			Tutorial.buttEnter=0;
			char Txt[500];				
			switch (arPlayer[Player].arHero[0]->ipClass1)
			{
			case 1:
			case 2:
			case 3:
				sprintf(Txt,(pFirstCampDialog+22)->sText,arPlayer[1].GetName());
				(pFirstCampDialog+22)->Show(1,0,0,0,NULL,Txt);
				Tutorial.Library=1;
				Tutorial.buttLibrary=1;
				buttLibrary.bBlink=true;
				txtTutorial.Activate((pFirstTutText+20)->msg);
				arPlayer[1].Gem+=50;
				break;
			case 4:
				sprintf(Txt,(pFirstCampDialog+23)->sText,arPlayer[1].GetName());
				(pFirstCampDialog+23)->Show(1,0,0,0,NULL,Txt);
				Tutorial.Trade=1;
				Tutorial.buttShop=1;
				buttShop.bBlink=true;
				txtTutorial.Activate((pFirstTutText+4)->msg);
				arPlayer[1].Gold+=200;
				break;
			};
	//		if (Tutorial.Mode)
			{
				bTutHint=true;
				ScrollSound();
				objTutorial->iLayer=6;
				buttNextHero.bBlink=true;
			};
		};

		//Нежить
		if ((Tutorial.Map)&&(Tutorial.Move==14))
		{
			Tutorial.Move=15;
			objTutorial->iLayer=-1;
			txtTutorial.Hide();
			objMain=ShowActiveObjects(objMain,10,true,MX,MY);
			int Answer=(pFirstCampDialog+24)->Show(1,1,0,0,NULL,NULL);
			if (Answer==2)
			{
				Answer=(pFirstCampDialog+25)->Show(1,1,0,0,NULL,NULL);
				if (Answer==2)
				{
					Answer=(pFirstCampDialog+26)->Show(1,1,1,0,NULL,NULL);
					if (Answer==3)
					{
						Answer=(pFirstCampDialog+27)->Show(1,1,1,0,NULL,NULL);
						if (Answer==3)
						{
							Answer=(pFirstCampDialog+28)->Show(1,1,0,0,NULL,NULL);
							if (Answer==2)
							{
								Answer=(pFirstCampDialog+29)->Show(1,1,0,0,NULL,NULL);
								if (Answer==1)
								{
									Answer=(pFirstCampDialog+30)->Show(1,1,0,0,NULL,NULL);
									if (Answer==1)
									{
										(pFirstCampDialog+31)->Show(1,0,0,0,NULL,NULL);
									};
								};
							};
						};
					};
				};
			};
	//		if (Tutorial.Mode)
			{
				txtTutorial.Activate((pFirstTutText+27)->msg);
				objTutorial->iLayer=6;
				bTutHint=true;
				ScrollSound();
				buttEnter.bBlink=true;
			};
			Tutorial.buttEnter=1;
			buttEnter.Activate(1);			
			(pFirstProvince+27)->iExplore=3;
			Tutorial.Site=2;
		};

		//Разбойники
		if ((Tutorial.Map)&&(Tutorial.Move==16)&&((pFirstProvince+27)->arSite[2][0]==0))
		{
			Tutorial.Move=17;
			objTutorial->iLayer=-1;
			txtTutorial.Hide();
			objMain=ShowActiveObjects(objMain,10,true,MX,MY);
			char Txt[500];
			sprintf(Txt, (pFirstCampDialog+32)->sText, (pFirstProvince+21)->sName);
			int Answer=(pFirstCampDialog+32)->Show(1,1,0,0,NULL,Txt);
			if (Answer==2)
			{
				sprintf(Txt,(pFirstCampDialog+33)->sText,(pFirstProvince+21)->sName);
				(pFirstCampDialog+33)->Show(1,0,0,0,NULL,Txt);
			};
	//		if (Tutorial.Mode)
			{
				txtTutorial.Activate((pFirstTutText+45)->msg);
				objTutorial->iLayer=6;
				bTutHint=true;
				ScrollSound();
				buttNextHero.bBlink=true;
				buttShop.bBlink=true;
			};
			Tutorial.Site=-1;
			Tutorial.Repair=1;
			Tutorial.buttEnter=0;
		};

		//Захвачена провинция
		if ((Tutorial.Map)&&(Tutorial.Move<=18)&&(arPlayer[1].arHero[0]->ipProvince==21))
		{
			Tutorial.Move=19;
			objTutorial->iLayer=-1;
			txtTutorial.Hide();
			objMain=ShowActiveObjects(objMain,10,true,MX,MY);
			char Txt[500];
			sprintf(Txt, (pFirstCampDialog+34)->sText, arPlayer[1].GetName());
			(pFirstCampDialog+34)->Show(1,0,0,0,NULL,Txt);
			if (arHumans[0].iEnterData==1)
			{
				WindowHerb(&arHumans[0]);
				arHumans[0].iEnterData=2;
				arHumans[0].Save();
				arPlayer[1].MakeFlags();
			};
		//	if (Tutorial.Mode)
			{
				txtTutorial.Activate((pFirstTutText+50)->msg);
				objTutorial->iLayer=6;
				bTutHint=true;
				ScrollSound();
				buttNextHero.bBlink=true;
			};
			MX+=300;
		};

		//Первый вход в замок
		if ((Tutorial.Map)&&(Tutorial.Move==19)&&(arPlayer[1].arHero[0]->ipProvince==23))
		{
			Tutorial.Move=20;
			objTutorial->iLayer=-1;
			txtTutorial.Hide();
			(pFirstProvince+23)->ipPlayer=1;
			(pFirstProvince+23)->iFort=arPlayer[1].iWall;
			objMain=ShowActiveObjects(objMain,10,true,MX,MY);
			char Txt[500];
			sprintf(Txt, (pFirstCampDialog+35)->sText, arPlayer[1].GetName());
			(pFirstCampDialog+35)->Show(1,0,0,0,NULL,Txt);
			int Answer=(pFirstCampDialog+36)->Show(1,1,0,0,NULL,NULL);
			if (Answer==2)
			{(pFirstCampDialog+37)->Show(1,0,0,0,NULL,NULL);};
			arPlayer[1].Gold+=500;
			arPlayer[1].Gem+=100;			
			buttCastle.Activate(1);
	//		if (Tutorial.Mode)
			{
				buttCastle.bBlink=true;
				bTutHint=true;
				ScrollSound();
				txtTutorial.Activate((pFirstTutText+51)->msg);
				objTutorial->iLayer=6;
			};
			Tutorial.buttCastle=1;
			for (int i=0; i<14; i++)
			{arPlayer[1].Shop.Items[i][0]=0;};	

			arPlayer[1].arRecruit[0]=0;
			arPlayer[1].arRecruit[1]=0;

			arPlayer[1].arLibrary[0]=0;
			arPlayer[1].arLibrary[1]=0;
			arPlayer[1].arLibrary[2]=0;	
		};		

		//Выполнено задание по отстройке
		if ((Tutorial.Map)&&(Tutorial.Task==0)&&(arPlayer[1].arBld[2])&&(arPlayer[1].arBld[7])&&(arPlayer[1].arBld[36]))
		{
			bTutFree=false;
			Tutorial.Task=1;
			objTutorial->iLayer=-1;
			txtTutorial.Hide();
			objMain=ShowActiveObjects(objMain,10,true,MX,MY);
			(pFirstCampDialog+42)->Show(1,0,0,0,NULL,NULL);
			char Txt[500];
			sprintf(Txt, (pFirstCampDialog+43)->sText, (pFirstProvince+7)->sName);
			int Answer=(pFirstCampDialog+43)->Show(1,1,1,0,NULL,Txt);
			if (Answer==2)
			{(pFirstCampDialog+44)->Show(1,1,0,0,NULL,NULL);};
			if (Answer==3)
			{(pFirstCampDialog+45)->Show(1,0,0,0,NULL,NULL);};
			if ((pFirstProvince+10)->ipPlayer==1)
			{
				sprintf(Txt, (pFirstCampDialog+46)->sText, (pFirstProvince+10)->sName);
				int Answer=(pFirstCampDialog+46)->Show(1,0,0,0,NULL,Txt);
			};
			(pFirstProvince+10)->ConstructBuilding(10,true, false);
			Redraw=true;
		};

		//Первый выход из замка
		if ((Tutorial.Map)&&(Tutorial.Build==2))
		{
			Tutorial.Build=3;
			bTutHint=true;
			ScrollSound();
			txtTutorial.Activate((pFirstTutText+56)->msg);
			objTutorial->iLayer=6;			
		};

		//Найм стражей 1
		if ((bTutFree)&&(Tutorial.Map)&&(Tutorial.Turn>2)&&(arPlayer[1].arBld[7])&&(Tutorial.PrvGuard==0)&&(!Redraw))
		{
			bTutFree=false;
			Tutorial.PrvGuard=1;
			txtTutorial.Activate((pFirstTutText+63)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			Tutorial.buttGuard=1;
			buttGuard.bBlink=true;
			if (Condition<15)
			{Condition=0;};
		};

		//Найм стражей 2
		if ((Tutorial.Map)&&(Tutorial.PrvGuard==2))
		{
			Tutorial.PrvGuard=3;
			txtTutorial.Activate((pFirstTutText+64)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			bTutFree=false;
		};

		//Постройки 1
		if ((bTutFree)&&(Tutorial.Map)&&(Tutorial.Turn>2)&&(arPlayer[1].arBld[36])&&(Tutorial.PrvBuild==0)&&(!Redraw))
		{
			Tutorial.PrvBuild=1;
			txtTutorial.Activate((pFirstTutText+61)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			Tutorial.buttBuild=1;
			buttBuild.bBlink=true;
			if (Condition<15)
			{Condition=0;};
			bTutFree=false;
		};

		//Постройки 2
		if ((Tutorial.Map)&&(Tutorial.PrvBuild==2))
		{
			Tutorial.PrvBuild=3;
			txtTutorial.Activate((pFirstTutText+62)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			bTutFree=false;
		};
		
		//Ритуал над героем
		if ((bTutFree)&&(Tutorial.Turn>2)&&((arPlayer[1].arBld[20])||(arPlayer[1].arBld[21]))&&(Tutorial.RitualHero==0)&&(!Redraw))
		{
			Tutorial.RitualHero=1;
			txtTutorial.Activate((pFirstTutText+96)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			Tutorial.buttRitHero=1;
			buttHeroRitual.bBlink=true;
			bTutFree=false;
		};

		//Статистика
		if ((bTutFree)&&(Tutorial.Map)&&(Tutorial.Turn>5)&&(Tutorial.Move==23))
		{
			Tutorial.Move=24;
			txtTutorial.Activate((pFirstTutText+65)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			Tutorial.buttStat=1;
			buttStat.Activate(1);
			buttStat.bBlink=true;
			bTutFree=false;
		};

		//Особые места
		if ((Tutorial.Map)&&(Tutorial.Move==21))
		{
			Tutorial.Move=22;			
			txtTutorial.Activate((pFirstTutText+59)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			Tutorial.buttEnter=1;
			Tutorial.buttAttack=1;
			bTutFree=false;
		};

		//Исследования
		if ((bTutFree)&&(Tutorial.Map)&&(Tutorial.Move==22)&&(Tutorial.Turn>=2))
		{
			Tutorial.Move=23;			
			txtTutorial.Activate((pFirstTutText+60)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			Tutorial.buttExplore=1;
			bTutFree=false;
		};

		//Напоминание о задании
		if ((bTutFree)&&(Tutorial.Map)&&(Tutorial.Task==0)&&(Tutorial.Turn>0)&&(Tutorial.Turn%10==0))
		{
			Tutorial.Turn++;
			txtTutorial.Activate((pFirstTutText+66)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			bTutFree=false;
		};

		//Родовые замки
		if ((bTutFree)&&(Tutorial.Map)&&((pFirstProvince+8)->ipPlayer==1)&&(Tutorial.Castles==0))
		{
			Tutorial.Castles=1;
			txtTutorial.Activate((pFirstTutText+69)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			bTutFree=false;
		};

		//Победа в обучающей миссии
		if ((Tutorial.Map)&&(!arPlayer[2].bInGame))
		{
			objMain=ShowActiveObjects(objMain,10,true,MX,MY);
			int Answer=(pFirstCampDialog+47)->Show(1,1,0,0,NULL,NULL);
			if (Answer==1)
			{(pFirstCampDialog+48)->Show(1,0,0,0,NULL,NULL);};
			Answer=(pFirstCampDialog+49)->Show(1,1,0,0,NULL,NULL);
			if (Answer==1)
			{(pFirstCampDialog+50)->Show(1,0,0,0,NULL,NULL);}
			else
			{(pFirstCampDialog+51)->Show(1,0,0,0,NULL,NULL);};
			(pFirstCampDialog+52)->Show(1,0,0,0,NULL,NULL);
			bExitGame=true;
			iExitFlag=4;
		};

		//****Подсказки в процессе игры****

		//Найм героя
		if ((!Tutorial.Map)&&(CurGame.iGameType==2)&&(Tutorial.Hero==0))
		{
			Tutorial.Hero=1;
			txtTutorial.Activate((pFirstTutText+73)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			buttCastle.bBlink=true;
		};

		//Защита мира
		if ((!Tutorial.Map)&&(CurGame.iGameType==2)&&(Campaign.iActiveShard<0)&&(Tutorial.DefendWorld==0))
		{
			Tutorial.DefendWorld=1;
			txtTutorial.Activate((pFirstTutText+118)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();			
		};

		//Герой нанят
		if ((!Tutorial.Map)&&(CurGame.iGameType==2)&&(Tutorial.Hero==3))
		{
			bool bDead=false;
			for (int i=0; i<MAX_HERO; i++)
			{
				if (arPlayer[Player].arHero[i])
				{bDead=true;};
			};
			if (bDead)
			{
				Tutorial.Hero=4;
				txtTutorial.Activate((pFirstTutText+76)->msg);
				objTutorial->iLayer=6;
				bTutHint=true;
				ScrollSound();
			};
		};

		//Гибель героя
		if ((!Tutorial.Map)&&(CurGame.iGameType==2)&&((Tutorial.Hero==4)||(Tutorial.Hero==6)))
		{
			bool bDead=false;
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead>0))
				{bDead=true;};
			};
			if (bDead)
			{
				if (Tutorial.Hero==4)
				{
					Tutorial.Hero=5;
					txtTutorial.Activate((pFirstTutText+77)->msg);
				}
				else
				{
					Tutorial.Hero=7;
					txtTutorial.Activate((pFirstTutText+111)->msg);
				};
				objTutorial->iLayer=6;
				bTutHint=true;
				ScrollSound();
				bTutFree=false;
			};			
		};

		//Дипломатия
		if ((Tutorial.Mode)&&(bTutFree)&&(CurGame.bDiplomacy)&&(Tutorial.Diplomacy==0))
		{
			bool bDip=false;
			for (int i=1; i<=CurGame.iMaxPlayers; i++)
			{
				if ((i!=Player)&&(arPlayer[i].Control==2)&&(arPlayer[i].Politic[Player].Treaty>0))
				{
					bDip=true;
					break;					
				};							
			};
			if (bDip)
			{
				if (GetCurPlayers()<=2)
				{
					if (Tutorial.NoDip==0)
					{
						Tutorial.NoDip=1;							
						txtTutorial.Activate((pFirstTutText+100)->msg);
						objTutorial->iLayer=6;
						bTutHint=true;
						ScrollSound();
					};
				}
				else
				{
					Tutorial.Diplomacy=1;
					buttDipl.bBlink=true;
					txtTutorial.Activate((pFirstTutText+99)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
				};
			};	
		};

		//Защита замка
		if ((!Tutorial.Map)&&(CurGame.iGameType==2)&&(Tutorial.DefendCastle==0)&&(bTutFree))
		{
			bool bDead=false;
			for (int i=1; i<=MapSize; i++)
			{
				if (((pFirstProvince+i)->ipPlayer==Player)&&((pFirstProvince+i)->pGarrison))
				{
					bool bEmpty=true;
					for (int j=1; j<MAX_SQUAD; j++)
					{
						if ((pFirstProvince+i)->pGarrison->pSquad[j])
						{bEmpty=false;};
					};
					if (bEmpty)
					{
						for (int j=0; j<MAX_NEIGHBOUR; j++)
						{
							if (((pFirstProvince+i)->arNeighbour[j]>0)&&((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipPlayer!=0)&&
								((pFirstProvince+(pFirstProvince+i)->arNeighbour[j])->ipPlayer!=Player))
							{bDead=true;};
						};
					};
				};
			};
			if (bDead)
			{
				Tutorial.DefendCastle=1;
				txtTutorial.Activate((pFirstTutText+116)->msg);
				objTutorial->iLayer=6;
				bTutHint=true;
				ScrollSound();
				bTutFree=false;
			};
		};

		//Воскрешение героя
		if ((!Tutorial.Map)&&(CurGame.iGameType==2)&&(Tutorial.Hero==5)&&(bTutFree))
		{
			bool bDead=false;
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead==0))
				{bDead=true;};				
			};
			if (bDead)
			{
				Tutorial.Hero=6;
				txtTutorial.Activate((pFirstTutText+78)->msg);
				objTutorial->iLayer=6;
				bTutHint=true;
				ScrollSound();
				bTutFree=false;
			};
		};
		

		
		//Дальние земли
		if ((bTutFree)&&(!Tutorial.Map)&&(CurGame.iGameType==2)&&(Tutorial.Population==0)&&(CurGame.iCurrentTurn>=2))
		{
			Tutorial.Population=1;
			txtTutorial.Activate((pFirstTutText+82)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			bTutFree=false;
		};

		//Население
		if ((bTutFree)&&(!Tutorial.Map)&&(CurGame.iGameType==2)&&(Tutorial.Population==1)&&(CurGame.iCurrentTurn>=3))
		{
			Tutorial.Population=2;
			txtTutorial.Activate((pFirstTutText+80)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			bTutFree=false;
		};

		//Население 2
		if ((bTutFree)&&(!Tutorial.Map)&&(CurGame.iGameType==2)&&(Tutorial.Population==2)&&(CurGame.iCurrentTurn>=4))
		{
			Tutorial.Population=3;
			txtTutorial.Activate((pFirstTutText+81)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
			bTutFree=false;
		};

		//Ресурс
		if ((bTutFree)&&(!Tutorial.Map)&&(CurGame.iGameType==2)&&(Tutorial.Resource==0))
		{
			int Res=0;
			for (int i=1; i<10; i++)
			{
				if (arPlayer[Player].GetResource(i)>0)
				{Res=i;};
			};
			if (Res>0)
			{
				Tutorial.Resource=1;
				char Txt[500];
				sprintf(Txt, (pFirstTutText+79)->msg, (pFirstResource+Res)->sName);
				txtTutorial.Activate(Txt);
				objTutorial->iLayer=6;
				bTutHint=true;
				ScrollSound();
				bTutFree=false;
			};
		};

		//Ритуал в провинции
		if ((bTutFree)&&(Tutorial.RitualProv==0)&&(!Redraw))
		{
			for (int i=1; i<=iMaxRitual; i++)
			{
				if (((pFirstRitual+i)->GetAbility(20)==0)&&(*(arPlayer[Player].pRitual+i)!=0))
				{
					Tutorial.RitualProv=1;
					txtTutorial.Activate((pFirstTutText+98)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
					buttRitual.bBlink=true;
					bTutFree=false;
					break;
				};
			};			
		};

		//Коррупция
		if ((bTutFree)&&(Tutorial.Corrupt==0)&&(!Redraw)&&(arPlayer[Player].iCalcCorrupt>5))
		{
			Tutorial.Corrupt=1;
			txtTutorial.Activate((pFirstTutText+112)->msg);
			objTutorial->iLayer=6;
			bTutHint=true;
			ScrollSound();
		};		
		};//bTutor
		bHint=false;
		/*if ((CurGame.iCurrentTurn==0)&&(Iteration<1000)&&(Iteration%10==0)&&(bLogger))
		{
			sprintf(stLog,"Step: 4, Inc: %d, bFirst: %d",iIncomeY,bFirst);
			FileLog(stLog);
		};*/
		switch (Condition)
//Проверка состояния инфоокна
		{
		case 1:				
	//Выбрана провинция
			/*if (((pFirstProvince+Member)->ipPlayer!=Player)&&(key[KEY_Q]))
			{
				(pFirstProvince+Member)->ipPlayer=Player;
				delete (pFirstProvince+Member)->pGuard;
				(pFirstProvince+Member)->pGuard=NULL;
				Redraw=true;
			};*/
			if (buttGuard.Check())
			{
				Condition=6;
				Shift=0;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=HireList(Player,Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				for (int i=0; i<4; i++)
				{mskMain[i].Activate(-1);};
				buttGuard.Activate(-1);
				buttBuild.Activate(-1);
				buttGarrison.Activate(-1);
				buttRitual.Activate(-1);
				buttScrollUp.Activate(0);
				buttCancel.Activate(1);
				if (Limit>BAR_SIZE)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};
				buttDefFilter.Activate(1,arPlayer[Player].bDefFilter);				
			};
		/*	if (buttGuard.flag>1)
			{
				bHint=true;
				MakeStringHint(92);
			};*/
				
			if (buttGarrison.Check())
			{
				if ((pFirstProvince+Member)->pGarrison)
				{
					CentreMap((pFirstProvince+Member)->x+300, (pFirstProvince+Member)->y);
				//	objLow->iLayer=-1;
					rectfill(screen1,0,0,512,768,COLOR_1);
					objMain=ShowActiveObjects(objMain,5,false,MX,MY);
				//	objLow->iLayer=4;
					if ((pFirstProvince+Member)->GetSiegeIndex()<0)
					{ScreenGarrison((pFirstProvince+Member)->pGarrison,true);}
					else
					{ScreenGarrison((pFirstProvince+Member)->pGarrison,false);};
				};
			};
			if (buttRitual.Check())
			{
				buttRitual.bBlink=false;
				Condition=12;
				Shift=0;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=RitualList(Player,Member,Shift,bmpInfoBar);
				if ((Limit>0)&&(Tutorial.RitualCast==0))
				{
					Tutorial.RitualCast=1;
					bTutHint=true;
					ScrollSound();
					txtTutorial.Activate((pFirstTutText+97)->msg);
					objTutorial->iLayer=6;
				};
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				for (int i=0; i<4; i++)
				{mskMain[i].Activate(-1);};
				buttGuard.Activate(-1);
				buttBuild.Activate(-1);
				buttGarrison.Activate(-1);
				buttRitual.Activate(-1);
				buttScrollUp.Activate(0);
				buttCancel.Activate(1);
				if (Limit>BAR_SIZE)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};
				buttRitFilter.Activate(1,arPlayer[Player].bRitFilter);
				
			};
	//Маски боковой панели
			Check=mskRace.Check();
			if ((Check==30)||(Check==40))
			{
				objMain=ShowActiveObjects(objMain,10,false,MX,MY);
				WindowRaceInfo((pFirstProvince+Member)->ipRace);
			};
			Check=mskRace1.Check();
			if ((Check==30)||(Check==40))
			{
				objMain=ShowActiveObjects(objMain,10,false,MX,MY);
				WindowRaceInfo((pFirstProvince+Member)->ipRace);
			};
			if (*(arPlayer[Player].pFog+Member)<1)
			{
				if ((pFirstProvince+Member)->ipPlayer==0)
				{
					mskExplore.iHint=404;
					Check=mskExplore.Check();
					if ((Check==30)||(Check==40))
					{
						objMain=ShowActiveObjects(objMain,10,false,MX,MY);
						WindowInfo((pFirstPhrase+404)->str,(pFirstMessage+33)->msg,bmpStuff[52]);
					};
				}
				else
				{
					mskExplore.iHint=400;
					Check=mskExplore.Check();
					if ((Check==30)||(Check==40))
					{
						objMain=ShowActiveObjects(objMain,10,false,MX,MY);
						WindowInfo((pFirstPhrase+400)->str,(pFirstMessage+30)->msg,bmpStuff[8]);
					};
				};
				Check=mskMood.Check();
				if ((Check==30)||(Check==40))
				{
					objMain=ShowActiveObjects(objMain,10,false,MX,MY);
					WindowInfo((pFirstPhrase+401)->str,(pFirstMessage+31)->msg,bmpStuff[51]);
				};
				Check=mskLevel.Check();
				if ((Check==30)||(Check==40))
				{
					objMain=ShowActiveObjects(objMain,10,false,MX,MY);
					WindowInfo((pFirstPhrase+403)->str,(pFirstMessage+32)->msg,bmpStuff[50]);
				};
				mskGoldInc.Check();
				mskGemInc.Check();
			};
			Check=mskResource.Check();
			if ((Check==30)||(Check==40))
			{
				if ((pFirstProvince+Member)->GetResource(0)==0)
				{
					objMain=ShowActiveObjects(objMain,10,false,MX,MY);
					WindowInfo((pFirstPhrase+26)->str,(pFirstMessage+34)->msg,bmpStuff[13]);
				}
				else
				{
					objMain=ShowActiveObjects(objMain,10,false,MX,MY);
					WindowResourceInfo((pFirstProvince+Member)->GetResource(0));
				};
			};
			Check=mskTerra.Check();
			if ((Check==30)||(Check==40))
			{
				objMain=ShowActiveObjects(objMain,10,false,MX,MY);
				WindowInfo((pFirstTerrain+(pFirstProvince+Member)->ipTerrain)->sName,(pFirstMessage+43+(pFirstProvince+Member)->ipTerrain)->msg,bmpStuff[42+(pFirstProvince+Member)->ipTerrain]);				
			};
	
//Просмотр стража
			if (mskMain[0].Check()==30)
			{
				Condition=8;
				Choosen=(pFirstProvince+Member)->iDefender;
				if (Choosen>0)
				{
					blit(bmpGUI_Defender,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
					draw_sprite(bmpInfoBar,(pFirstDefender+Choosen)->Pic,52,65);
					textprintf_centre_ex(bmpInfoBar,Font1,160,44,COLOR_1,-1,"%s",(pFirstDefender+Choosen)->sName);
					textprintf_centre_ex(bmpInfoBar,Font1,149,71,COLOR_2,-1,"%d",arPlayer[Player].GetDefenderGoldPrice(Choosen));
					textprintf_centre_ex(bmpInfoBar,Font1,227,71,COLOR_2,-1,"%d",arPlayer[Player].GetDefenderGemPrice(Choosen));
					textprintf_centre_ex(bmpInfoBar,Font1,87,138,COLOR_2,-1,"%d",(pFirstProvince+Member)->pGuard->iTask);
					if ((pFirstDefender+Choosen)->iGoldPayment<0)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,150,138,COLOR_2,-1,"%d",(pFirstProvince+Member)->pGuard->GetDefenderGoldPayment());
						textprintf_centre_ex(bmpInfoBar,Font1,227,138,COLOR_2,-1,"%d",(pFirstProvince+Member)->pGuard->GetDefenderGemPayment());
					}
					else
					{
						textprintf_centre_ex(bmpInfoBar,Font1,150,138,COLOR_2,-1,"%d",(pFirstDefender+Choosen)->iGoldPayment);
						textprintf_centre_ex(bmpInfoBar,Font1,227,138,COLOR_2,-1,"%d",(pFirstDefender+Choosen)->iGemPayment);
					};
					txtDs1.Activate((pFirstDefender+Choosen)->sText);
					for (int j=0; j<MAX_DEFENDER_RES; j++)
					{
						if ((pFirstDefender+Choosen)->arRes[j]>0)
						{
							draw_sprite(bmpInfoBar,(pFirstResource+(pFirstDefender+Choosen)->arRes[j])->Pic,110+j*26,97);
							if (arPlayer[Player].GetResource((pFirstDefender+Choosen)->arRes[j])==0)
							{
								draw_sprite(bmpInfoBar,bmpGUI_NoRes,110+j*26,97);
							};
						};
					};
					if ((pFirstDefender+Choosen)->bNoDismiss)
					{buttDismiss.Activate(0);}
					else
					{buttDismiss.Activate(1);};
				}
				else
				{
					blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
					buttDismiss.Activate(0);
				};
				buttCancel2.Activate(1);					
				for (int i=0; i<4; i++)
				{mskMain[i].Activate(-1);};
				buttGuard.Activate(-1);
				buttBuild.Activate(-1);
				buttGarrison.Activate(-1);
				buttRitual.Activate(-1);				
			};
//Просмотр построенных зданий
			if (mskMain[1].Check()==30)
			{
				Condition=4;
				Shift=0;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=RazeList(Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				for (int i=0; i<4; i++)
				{mskMain[i].Activate(-1);};
				buttGuard.Activate(-1);
				buttBuild.Activate(-1);
				buttGarrison.Activate(-1);
				buttRitual.Activate(-1);
				buttScrollUp.Activate(0);
				buttCancel.Activate(1);
				if (Limit>BAR_SIZE)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};				
			};
//Просмотр действующих ритуалов
			if ((objEnchant->iLayer>0)&&(mskBroken.Check()==30))
			{
				Condition=14;
				mskBroken.iHint=0;
				Shift=0;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=EnchantList(Player,Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				for (int i=0; i<4; i++)
				{mskMain[i].Activate(-1);};
				buttGuard.Activate(-1);
				buttBuild.Activate(-1);
				buttGarrison.Activate(-1);
				buttRitual.Activate(-1);
				buttScrollUp.Activate(0);
				buttCancel.Activate(1);
				if (Limit>BAR_SIZE)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};				
			};
//Просмотр состояния укреплений
			if (mskMain[2].Check()==30)
			{
				Condition=11;
				blit(bmpGUI_Fort,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				textprintf_centre_ex(bmpInfoBar,Font1,184,49,COLOR_1,-1,"%s",(pFirstPhrase+35)->str);
				Choosen=0;
				int Towerz;
				for (int i=0; i<MAX_PROVINCE_BLD; i++)
				{
					if (((pFirstProvince+Member)->arBuilding[i]>0)&&(pFirstOuterBld+(pFirstProvince+Member)->arBuilding[i])->iWall>0)
					{
						Choosen=(pFirstProvince+Member)->arBuilding[i];
					};
				};
				int HomeCastle=0;
				for (int i=1; i<=CurGame.iMaxPlayers; i++)
				{
					if ((arPlayer[i].bInGame)&&(arPlayer[i].ipProvince==(pFirstProvince+Member)->iNumber))
					{HomeCastle=i;};					
				};
				if ((Choosen>0)||(HomeCastle>0))
				{
					int Fort;					
					if (HomeCastle>0)
					{
						Fort=100*(pFirstProvince+Member)->iFort/arPlayer[HomeCastle].iWall;
						Towerz=arPlayer[HomeCastle].iTower;
						draw_sprite(bmpInfoBar,(pFirstInnerBld+149)->Pic,52,41);
					}
					else
					{
						Fort=100*(pFirstProvince+Member)->iFort/(pFirstOuterBld+Choosen)->iWall;
						Towerz=(pFirstOuterBld+Choosen)->iTowerType;
						draw_sprite(bmpInfoBar,(pFirstOuterBld+Choosen)->Pic,52,41);
					};
					textprintf_centre_ex(bmpInfoBar,Font1,156,133,COLOR_1,-1,"%s",(pFirstPhrase+36)->str);
					if (Fort==100)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,150,COLOR_1,-1,"%s",(pFirstPhrase+27)->str);
					}
					else
					{
					if (Fort>75)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,150,COLOR_1,-1,"%s",(pFirstPhrase+28)->str);
					}
					else
					{
					if (Fort>50)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,150,COLOR_1,-1,"%s",(pFirstPhrase+29)->str);
					}						
					else
					{
					if (Fort>40)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,150,COLOR_1,-1,"%s",(pFirstPhrase+30)->str);
					}
					else
					{
					if (Fort>30)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,150,COLOR_1,-1,"%s",(pFirstPhrase+31)->str);
					}
					else
					{
					if (Fort>20)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,150,COLOR_1,-1,"%s",(pFirstPhrase+32)->str);
					}
						else
					{
					if (Fort>0)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,150,COLOR_1,-1,"%s",(pFirstPhrase+33)->str);
					}
					else
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,150,COLOR_1,-1,"%s",(pFirstPhrase+34)->str);
					};};};};};};};
					if (Towerz>=1)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,173,COLOR_1,-1,"%s 1",(pFirstPhrase+37)->str);
						if (Fort>44)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,190,COLOR_1,-1,"%s",(pFirstPhrase+27)->str);
						}
						else
						{
						if (Fort>41)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,190,COLOR_1,-1,"%s",(pFirstPhrase+28)->str);
						}
						else
						{
						if (Fort>38)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,190,COLOR_1,-1,"%s",(pFirstPhrase+29)->str);
						}
						else
						{
						if (Fort>35)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,190,COLOR_1,-1,"%s",(pFirstPhrase+32)->str);
						}
						else
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,190,COLOR_1,-1,"%s",(pFirstPhrase+33)->str);
						};};};};
					};

					if (Towerz>=2)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,213,COLOR_1,-1,"%s 2",(pFirstPhrase+37)->str);
						if (Fort>34)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,230,COLOR_1,-1,"%s",(pFirstPhrase+27)->str);
						}
						else
						{
						if (Fort>31)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,230,COLOR_1,-1,"%s",(pFirstPhrase+28)->str);
						}
						else
						{
						if (Fort>28)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,230,COLOR_1,-1,"%s",(pFirstPhrase+29)->str);
						}
						else
						{
						if (Fort>25)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,230,COLOR_1,-1,"%s",(pFirstPhrase+32)->str);
						}
						else
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,230,COLOR_1,-1,"%s",(pFirstPhrase+33)->str);
						};};};};
					};

					if (Towerz>=3)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,253,COLOR_1,-1,"%s 3",(pFirstPhrase+37)->str);
						if (Fort>19)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,270,COLOR_1,-1,"%s",(pFirstPhrase+27)->str);
						}
						else
						{
						if (Fort>16)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,270,COLOR_1,-1,"%s",(pFirstPhrase+28)->str);
						}
						else
						{
						if (Fort>13)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,270,COLOR_1,-1,"%s",(pFirstPhrase+29)->str);
						}
						else
						{
						if (Fort>10)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,270,COLOR_1,-1,"%s",(pFirstPhrase+32)->str);
						}
						else
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,270,COLOR_1,-1,"%s",(pFirstPhrase+33)->str);
						};};};};
					};

					if (Towerz>=4)
					{
						textprintf_centre_ex(bmpInfoBar,Font1,156,293,COLOR_1,-1,"%s 4",(pFirstPhrase+37)->str);
						if (Fort>9)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,310,COLOR_1,-1,"%s",(pFirstPhrase+27)->str);
						}
						else
						{
						if (Fort>6)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,310,COLOR_1,-1,"%s",(pFirstPhrase+28)->str);
						}
						else
						{
						if (Fort>3)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,310,COLOR_1,-1,"%s",(pFirstPhrase+29)->str);
						}
						else
						{
						if (Fort>0)
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,310,COLOR_1,-1,"%s",(pFirstPhrase+32)->str);
						}
						else
						{
							textprintf_centre_ex(bmpInfoBar,Font1,156,310,COLOR_1,-1,"%s",(pFirstPhrase+33)->str);
						};};};};
					};
				}
				else
				{
					textprintf_centre_ex(bmpInfoBar,Font1,156,135,COLOR_1,-1,"%s",(pFirstPhrase+38)->str);				
				};
				buttCancel2.Activate(1);					
				for (int i=0; i<4; i++)
				{mskMain[i].Activate(-1);};
				buttGuard.Activate(-1);
				buttBuild.Activate(-1);
				buttGarrison.Activate(-1);
				buttRitual.Activate(-1);				
			};
//Просмотр особых мест
			if (mskMain[3].Check()==30)
			{
				Condition=9;
				Shift=0;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=SiteList(Member,Shift,bmpInfoBar,false,NULL);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				for (int i=0; i<4; i++)
				{mskMain[i].Activate(-1);};
				buttGuard.Activate(-1);
				buttBuild.Activate(-1);
				buttGarrison.Activate(-1);
				buttRitual.Activate(-1);
				buttScrollUp.Activate(0);
				buttCancel.Activate(1);
				if (Limit>BAR_SIZE)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};
			};

			if (buttBuild.Check())
			{
				if (arPlayer[Player].ipProvince==(pFirstProvince+Member)->iNumber)
				{
					if ((pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()<0)
					{				
						int Pick=CastleMain(Player);
						if (Pick>0)
						{
							Member=Pick-1;
							Condition=20;
						};						
						Redraw=true;
					}
					else
					{
						(pFirstDialog+388)->Show(true,true,false,false,NULL,NULL);
					};
				}
				else
				{
					Condition=2;
					Shift=0;
					blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
					Limit=BuildList(Player,Member,Shift,bmpInfoBar);
					for (int i=0; i<BAR_SIZE; i++)
					{
						if (i<Limit)
						{mskBuild[i].Activate(7);}
						else
						{mskBuild[i].Activate(-1);};
					};
					for (int i=0; i<4; i++)
					{mskMain[i].Activate(-1);};
					buttGuard.Activate(-1);
					buttBuild.Activate(-1);
					buttGarrison.Activate(-1);
					buttRitual.Activate(-1);
					buttScrollUp.Activate(0);
					buttCancel.Activate(1);
					if (Limit>BAR_SIZE)
					{buttScrollDown.Activate(1);}
					else
					{buttScrollDown.Activate(0);};
					buttBldFilter.Activate(1,arPlayer[Player].bBldFilter);				
				};
			};	


			break;
		case 2:
	//Выбрано строительство в провинции
			for (int i=0; i<BAR_SIZE; i++)
			{
				if (i<Limit)
				{
					Check=mskBuild[i].Check();
					if (Check==40)
					{WindowOuterBldInfo(arBuild[i]);};
					if (Check==30)
					{
						Condition=3;
						blit(bmpGUI_Building,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
						Choosen=arBuild[i];
						draw_sprite(bmpInfoBar,(pFirstOuterBld+Choosen)->Pic,52,65);
						textprintf_centre_ex(bmpInfoBar,Font1,156,44,COLOR_1,-1,"%s",(pFirstOuterBld+Choosen)->sName);
						textprintf_centre_ex(bmpInfoBar,Font1,147,71,COLOR_2,-1,"%d",arPlayer[Player].GetOutBldGoldPrice(Choosen));
						textprintf_centre_ex(bmpInfoBar,Font1,227,71,COLOR_2,-1,"%d",arPlayer[Player].GetOutBldGemPrice(Choosen));
						txtDs.Activate((pFirstOuterBld+Choosen)->sText);
						for (int j=0; j<MAX_BUILDING_RES; j++)
						{
							if ((pFirstOuterBld+Choosen)->arRes[j]>0)
							{
								draw_sprite(bmpInfoBar,(pFirstResource+(pFirstOuterBld+Choosen)->arRes[j])->Pic,110+j*26,97);
								if (arPlayer[Player].GetResource((pFirstOuterBld+Choosen)->arRes[j])==0)
								{
									draw_sprite(bmpInfoBar,bmpGUI_NoRes,110+j*26,97);
								};
							};
						};
						if (((pFirstProvince+Member)->CanBuild(Choosen))&&
							(arPlayer[Player].GetOutBldGoldPrice(Choosen)<=arPlayer[Player].Gold)&&
						((arPlayer[Player].GetOutBldGemPrice(Choosen)<=arPlayer[Player].Gem)||((arPlayer[Player].GetOutBldGemPrice(Choosen)==0))))
						{buttBuild2.Activate(1);}
						else
						{buttBuild2.Activate(0);};
						buttCancel2.Activate(1);					
						buttScrollUp.Activate(-1);
						buttCancel.Activate(-1);
						buttScrollDown.Activate(-1);
						buttBldFilter.Activate(-1,arPlayer[Player].bBldFilter);
						for (int i=0; i<BAR_SIZE; i++)
						{mskBuild[i].Activate(-1);};
						};
				};
			};
			if (buttScrollUp.Check())
			{
				Shift--;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=BuildList(Player,Member,Shift,bmpInfoBar);
				if (Shift==0)
				{
					buttScrollUp.Activate(0);
				};
				buttScrollDown.Activate(1);
			};
			if (buttCancel.Check())
			{
				Condition=0;
			};
			if (buttScrollDown.Check())
			{					
				Shift++;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=BuildList(Player,Member,Shift,bmpInfoBar);
				if (Shift>=Limit-BAR_SIZE)
				{
					buttScrollDown.Activate(0);
				};
				buttScrollUp.Activate(1);					
			};
			buttBldFilter.Check();
			if (buttBldFilter.bPress!=arPlayer[Player].bBldFilter)
			{
				arPlayer[Player].bBldFilter=buttBldFilter.bPress;
				Shift=0;
				txtError.Hide();
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=BuildList(Player,Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				buttScrollUp.Activate(0);					
				if (Limit>BAR_SIZE)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};
			};				
			break;
		case 3:
	//Выбрана постройка из списка строительства
			if (buttBuild2.Check())
			{	
				(pFirstProvince+Member)->ConstructBuilding(Choosen,true, true);
				if ((Tutorial.Map)&&(Tutorial.PrvBuild==1))
				{
					Tutorial.PrvBuild=2;
					buttBuild.bBlink=false;
				};
				Condition=0;
				Redraw=true;
			};
			if (buttCancel2.Check())
			{
				Condition=2;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				txtDs.Hide();
				Limit=BuildList(Player,Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};					
				buttBuild2.Activate(-1);
				buttCancel2.Activate(-1);
				if (Shift==0)
				{buttScrollUp.Activate(0);}
				else
				{buttScrollUp.Activate(1);};
				buttCancel.Activate(1);
				if (Shift>=Limit-BAR_SIZE)
				{buttScrollDown.Activate(0);}
				else
				{buttScrollDown.Activate(1);}					
				buttBldFilter.Activate(1,arPlayer[Player].bBldFilter);
			};
			for (int i=0; i<MAX_BUILDING_RES; i++)
			{
				if ((pFirstOuterBld+Choosen)->arRes[i]>0)
				{
					Check=mskRes[i].Check();
					if (Check>1)
					{
						if ((Check==30)||(Check==40))
						{
							WindowResourceInfo((pFirstOuterBld+Choosen)->arRes[i]);
						}
						else
						{
							bHint=true;
							MakeStringHint((pFirstResource+(pFirstOuterBld+Choosen)->arRes[i])->sName);
						};
					};
				};
			};
			Check=mskRes[6].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(310);
			};
			Check=mskRes[7].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(311);
			};
			txtDs.Check();
			break;
		case 4:
	//Выбран список построенных зданий
			for (int i=0; i<BAR_SIZE; i++)
			{
				if (i<Limit)
				{
					Check=mskBuild[i].Check();
					if (Check==40)
					{WindowOuterBldInfo(arBuild[i]);};
					if (Check==30)
					{
						Condition=5;
						blit(bmpGUI_Building,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
						Choosen=arBuild[i];
						draw_sprite(bmpInfoBar,(pFirstOuterBld+Choosen)->Pic,52,65);
						textprintf_centre_ex(bmpInfoBar,Font1,156,44,COLOR_1,-1,"%s",(pFirstOuterBld+Choosen)->sName);
						textprintf_centre_ex(bmpInfoBar,Font1,150,71,COLOR_2,-1,"%d",arPlayer[Player].GetOutBldGoldPrice(Choosen));
						textprintf_centre_ex(bmpInfoBar,Font1,227,71,COLOR_2,-1,"%d",arPlayer[Player].GetOutBldGemPrice(Choosen));
						txtDs.Activate((pFirstOuterBld+Choosen)->sText);
						for (int j=0; j<MAX_BUILDING_RES; j++)
						{
							if ((pFirstOuterBld+Choosen)->arRes[j]>0)
							{
								draw_sprite(bmpInfoBar,(pFirstResource+(pFirstOuterBld+Choosen)->arRes[j])->Pic,110+j*26,97);
								if (arPlayer[Player].GetResource((pFirstOuterBld+Choosen)->arRes[j])==0)
								{
									draw_sprite(bmpInfoBar,bmpGUI_NoRes,110+j*26,97);
								};
							};
						};
						bool bRaze=true;
						for (int i=0; i<MAX_BUILDING_ABILITY; i++)
						{
							if ((pFirstOuterBld+Choosen)->arAbility[i][0]==1)
							{
								bRaze=false;
							};
						};
						if ((Tutorial.Map)&&(Choosen==10))
						{bRaze=false;};
						if (bRaze)
						{buttRaze.Activate(1);}
						else
						{buttRaze.Activate(0);};
						buttCancel2.Activate(1);					
						buttScrollUp.Activate(-1);
						buttCancel.Activate(-1);
						buttScrollDown.Activate(-1);						
						for (int i=0; i<BAR_SIZE; i++)
						{mskBuild[i].Activate(-1);};
						};
				};
			};
			if (buttScrollUp.Check())
			{
				Shift--;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=RazeList(Member,Shift,bmpInfoBar);
				if (Shift==0)
				{
					buttScrollUp.Activate(0);
				};
				buttScrollDown.Activate(1);
			};
			if (buttCancel.Check())
			{
				Condition=0;
			};
			if (buttScrollDown.Check())
			{					
				Shift++;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=RazeList(Member,Shift,bmpInfoBar);
				if (Shift>=Limit-BAR_SIZE)
				{
					buttScrollDown.Activate(0);
				};
				buttScrollUp.Activate(1);					
			};			
			break;
		case 5:
	//Выбрана постройка из списка сноса
			if (buttRaze.Check())
			{
				if ((pFirstProvince+Member)->GetSiegeIndex()<0)
				{
					if ((!arHumans[arPlayer[Player].iColor].arMess[6])||((pFirstDialog+1)->Show(true,true,false,false,NULL,NULL)==1))
					{
						int Slot=0;
						while ((pFirstProvince+Member)->arBuilding[Slot]!=Choosen)
						{
							Slot++;
						};
						(pFirstProvince+Member)->RazeBuilding(Slot, true);				
						Condition=0;
						Redraw=true;
					};
				}
				else
				{(pFirstDialog+1243)->Show(true,true,false,false,NULL,NULL);};
			};
			if (buttCancel2.Check())
			{
				Condition=4;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				txtDs.Hide();
				Limit=RazeList(Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};					
				buttRaze.Activate(-1);
				buttCancel2.Activate(-1);
				if (Shift==0)
				{buttScrollUp.Activate(0);}
				else
				{buttScrollUp.Activate(1);};
				buttCancel.Activate(1);
				if (Shift>=Limit-BAR_SIZE)
				{buttScrollDown.Activate(0);}
				else
				{buttScrollDown.Activate(1);}									
			};
			txtDs.Check();
			break;
		case 6:
	//Выбран найм стража в провинции
			for (int i=0; i<BAR_SIZE; i++)
			{
				if (i<Limit)
				{
					Check=mskBuild[i].Check();
					if (Check==40)
					{WindowDefenderInfo(arBuild[i], 0, 0);};
					if (Check==30)
					{
						Condition=7;
						blit(bmpGUI_Defender,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
						Choosen=arBuild[i];
						draw_sprite(bmpInfoBar,(pFirstDefender+Choosen)->Pic,52,65);
						textprintf_centre_ex(bmpInfoBar,Font1,156,44,COLOR_1,-1,"%s",(pFirstDefender+Choosen)->sName);
						textprintf_centre_ex(bmpInfoBar,Font1,150,71,COLOR_2,-1,"%d",arPlayer[Player].GetDefenderGoldPrice(Choosen));
						textprintf_centre_ex(bmpInfoBar,Font1,227,71,COLOR_2,-1,"%d",arPlayer[Player].GetDefenderGemPrice(Choosen));
						textprintf_centre_ex(bmpInfoBar,Font1,91,138,COLOR_2,-1,"%d",(pFirstDefender+Choosen)->iInitiative);
						textprintf_centre_ex(bmpInfoBar,Font1,150,138,COLOR_2,-1,"%d",(pFirstDefender+Choosen)->iGoldPayment);
						textprintf_centre_ex(bmpInfoBar,Font1,227,138,COLOR_2,-1,"%d",(pFirstDefender+Choosen)->iGemPayment);

						txtDs1.Activate((pFirstDefender+Choosen)->sText);
						for (int j=0; j<MAX_DEFENDER_RES; j++)
						{
							if ((pFirstDefender+Choosen)->arRes[j]>0)
							{
								draw_sprite(bmpInfoBar,(pFirstResource+(pFirstDefender+Choosen)->arRes[j])->Pic,110+j*26,97);
								if (arPlayer[Player].GetResource((pFirstDefender+Choosen)->arRes[j])==0)
								{
									draw_sprite(bmpInfoBar,bmpGUI_NoRes,110+j*26,97);
								};
							};
						};
						if (((pFirstProvince+Member)->CanHire(Choosen))&&
							(arPlayer[Player].GetDefenderGoldPrice(Choosen)<=arPlayer[Player].Gold)&&
							((arPlayer[Player].GetDefenderGemPrice(Choosen)<=arPlayer[Player].Gem)||(arPlayer[Player].GetDefenderGemPrice(Choosen)==0)))
						{buttGuard2.Activate(1);}
						else
						{buttGuard2.Activate(0);};
						buttCancel2.Activate(1);					
						buttScrollUp.Activate(-1);
						buttCancel.Activate(-1);
						buttScrollDown.Activate(-1);
						buttDefFilter.Activate(-1,arPlayer[Player].bDefFilter);
						for (int i=0; i<BAR_SIZE; i++)
						{mskBuild[i].Activate(-1);};
						};
				};
			};
			if (buttScrollUp.Check())
			{
				Shift--;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=HireList(Player,Member,Shift,bmpInfoBar);
				if (Shift==0)
				{
					buttScrollUp.Activate(0);
				};
				buttScrollDown.Activate(1);
			};
			if (buttCancel.Check())
			{
				Condition=0;
			};
			if (buttScrollDown.Check())
			{					
				Shift++;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=HireList(Player,Member,Shift,bmpInfoBar);
				if (Shift>=Limit-BAR_SIZE)
				{
					buttScrollDown.Activate(0);
				};
				buttScrollUp.Activate(1);					
			};
			buttDefFilter.Check();
			if (buttDefFilter.bPress!=arPlayer[Player].bDefFilter)
			{
				arPlayer[Player].bDefFilter=buttDefFilter.bPress;
				Shift=0;
				txtError.Hide();
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=HireList(Player,Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				buttScrollUp.Activate(0);					
				if (Limit>BAR_SIZE)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};
			};				
			break;
		case 7:
	//Выбран страж из списка найма
			if (buttGuard2.Check())
			{
				bool bHire=true;
				if (((pFirstProvince+Member)->iDefender>0)&&(arHumans[arPlayer[Player].iColor].arMess[5]))
				{
					if ((pFirstDialog+1165)->Show(1,1,0,0,NULL,NULL)==2)
					{bHire=false;};
				};
				if (bHire)
				{
				if ((pFirstProvince+Member)->iDefender>0)
				{					
					arPlayer[Player].iKarma-=(pFirstDefender+(pFirstProvince+Member)->iDefender)->iKarma;
				};
				arPlayer[Player].iKarma+=(pFirstDefender+Choosen)->iKarma;				
				arPlayer[Player].iCanHire--;
				(pFirstProvince+Member)->PlaceDefender(Choosen);
				if (*(arPlayer[Player].pDefender+Choosen)>0)
				{
					*(arPlayer[Player].pDefender+Choosen)=*(arPlayer[Player].pDefender+Choosen)-1;
				};
				CoinSound();
				//play_sample(wavCoins,iSoundVolume/2,128,1000,0);
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
				if ((Tutorial.Map)&&(Tutorial.PrvGuard==1))
				{
					Tutorial.PrvGuard=2;					
					buttGuard.bBlink=false;
				};
	
				Condition=0;
				Redraw=true;
				};
			};
			if (buttCancel2.Check())
			{
				Condition=6;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				txtDs1.Hide();
				Limit=HireList(Player,Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};					
				buttGuard2.Activate(-1);
				buttCancel2.Activate(-1);
				if (Shift==0)
				{buttScrollUp.Activate(0);}
				else
				{buttScrollUp.Activate(1);};
				buttCancel.Activate(1);
				if (Shift>=Limit-BAR_SIZE)
				{buttScrollDown.Activate(0);}
				else
				{buttScrollDown.Activate(1);}					
				buttDefFilter.Activate(1,arPlayer[Player].bDefFilter);
			};
			for (int i=0; i<MAX_DEFENDER_RES; i++)
			{
				if ((pFirstDefender+Choosen)->arRes[i]>0)
				{
					Check=mskRes[i].Check();
					if (Check>1)
					{
						if ((Check==30)||(Check==40))
						{
							WindowResourceInfo((pFirstDefender+Choosen)->arRes[i]);
						}
						else
						{
							bHint=true;
							MakeStringHint((pFirstResource+(pFirstDefender+Choosen)->arRes[i])->sName);
						};
					};
				};
			};
			Check=mskRes[6].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(310);
			};
			Check=mskRes[7].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(311);
			};
			Check=mskRes[8].Check();
			if (Check>1)
			{
				if ((Check==30)||(Check==40))
				{
					WindowAbilityInfo(999,(pFirstDefender+Choosen)->iInitiative);
				}
				else
				{
					bHint=true;
					MakeAbilityHint(999);
				};
			};
			Check=mskRes[9].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(90);
			};
			Check=mskRes[10].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(91);
			};
			txtDs1.Check();
			break;
		case 8:
	//Выбран страж в провинции
			if (buttDismiss.Check())
			{	
				if ((!arHumans[arPlayer[Player].iColor].arMess[5])||((pFirstDialog+2)->Show(true,true,false,false,NULL,NULL)==1))
				{
					arPlayer[Player].iKarma-=(pFirstDefender+(pFirstProvince+Member)->iDefender)->iKarma;
					(pFirstProvince+Member)->DismissDefender();				
					Condition=0;
					Redraw=true;
				};
			};
			if (buttCancel2.Check())
			{
				Condition=0;
			};
			txtDs1.Check();
			break;
		case 9:
	//Выбран список особых мест
			for (int i=0; i<BAR_SIZE; i++)
			{
				if (i<Limit)
				{
					if (mskBuild[i].Check()==30)
					{
						Condition=10;
						blit(bmpGUI_Site,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
						Choosen=arBuild[i];
						draw_sprite(bmpInfoBar,(pFirstSite+Choosen)->Pic,52,41);
						textprintf_centre_ex(bmpInfoBar,Font1,182,48,COLOR_1,-1,"%s",(pFirstSite+Choosen)->sName);
						if ((pFirstProvince+Member)->arSiteGuard[arBuildNum[i]])
						{
							txtDs2.Activate((pFirstSite+Choosen)->sText);
							textprintf_centre_ex(bmpInfoBar,Font1,156,315,makecol(250,30,30),-1,"%s:",(pFirstPhrase+25)->str);
							textprintf_centre_ex(bmpInfoBar,Font1,156,340,makecol(250,30,30),-1,"%s",(pFirstGuardType+(pFirstProvince+Member)->arSite[arBuildNum[i]][4])->sName);
						}
						else
						{txtDs2.Activate((pFirstSite+Choosen)->sFreeText);};
						buttCancel2.Activate(1);					
						buttScrollUp.Activate(-1);
						buttCancel.Activate(-1);
						buttScrollDown.Activate(-1);						
						for (int i=0; i<BAR_SIZE; i++)
						{mskBuild[i].Activate(-1);};						
					};						
				};
			};
			if (buttScrollUp.Check())
			{
				Shift--;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=SiteList(Member,Shift,bmpInfoBar,false,NULL);
				if (Shift==0)
				{
					buttScrollUp.Activate(0);
				};
				buttScrollDown.Activate(1);
			};
			if (buttCancel.Check())
			{
				Condition=0;
			};
			if (buttScrollDown.Check())
			{					
				Shift++;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=SiteList(Member,Shift,bmpInfoBar,false,NULL);
				if (Shift>=Limit-BAR_SIZE)
				{
					buttScrollDown.Activate(0);
				};
				buttScrollUp.Activate(1);					
			};			
			break;
		case 10:
	//Выбрано особое место
			if (buttCancel2.Check())
			{
				Condition=9;				
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=SiteList(Member,Shift,bmpInfoBar,false,NULL);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				txtDs2.Hide();
				for (int i=0; i<4; i++)
				{mskMain[i].Activate(-1);};
				buttCancel2.Activate(-1);			
				if (Shift>0)
				{buttScrollUp.Activate(1);}
				else
				{buttScrollUp.Activate(0);};
				buttCancel.Activate(1);
				if (Shift>=Limit-BAR_SIZE)
				{buttScrollDown.Activate(0);}
				else
				{buttScrollDown.Activate(1);};
			};
			txtDs2.Check();
			break;
	//Информация об укреплениях
		case 11:
			if (buttCancel2.Check())
			{
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Condition=0;
			};
		case 12:
	//Выбран список ритуалов
			for (int i=0; i<BAR_SIZE; i++)
			{
				if (i<Limit)
				{
					Check=mskBuild[i].Check();
					if (Check==40)
					{WindowRitualInfo(arBuild[i]);};
					if (Check==30)
					{
						Condition=13;
						blit(bmpGUI_Building,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
						Choosen=arBuild[i];
						draw_sprite(bmpInfoBar,(pFirstRitual+Choosen)->Pic,52,65);
						textprintf_centre_ex(bmpInfoBar,Font1,156,44,COLOR_1,-1,"%s",(pFirstRitual+Choosen)->sName);
						textprintf_centre_ex(bmpInfoBar,Font1,150,71,COLOR_2,-1,"%d",arPlayer[Player].GetRitualGoldPrice(Choosen));
						textprintf_centre_ex(bmpInfoBar,Font1,227,71,COLOR_2,-1,"%d",arPlayer[Player].GetRitualGemPrice(Choosen));
						draw_sprite(bmpInfoBar,bmpStuff[35],186,95);
						textprintf_centre_ex(bmpInfoBar,Font1,232,102,COLOR_2,-1,"%d",(pFirstRitual+Choosen)->iCooldown-arPlayer[Player].GetBldParam1(53));
						txtDs.Activate((pFirstRitual+Choosen)->sText);
						for (int j=0; j<MAX_RITUAL_RES; j++)
						{
							if ((pFirstRitual+Choosen)->arRes[j]>0)
							{
								draw_sprite(bmpInfoBar,(pFirstResource+(pFirstRitual+Choosen)->arRes[j])->Pic,110+j*26,97);
								if (arPlayer[Player].GetResource((pFirstRitual+Choosen)->arRes[j])==0)
								{
									draw_sprite(bmpInfoBar,bmpGUI_NoRes,110+j*26,97);
								};
							};
						};
						if (((pFirstProvince+Member)->CanCast(Choosen,Player))&&
							(*(arPlayer[Player].pRitualCD+Choosen)==0)&&
							((arPlayer[Player].GetRitualGoldPrice(Choosen)<=arPlayer[Player].Gold)||((arPlayer[Player].GetRitualGoldPrice(Choosen)==0)))&&
							((arPlayer[Player].GetRitualGemPrice(Choosen)<=arPlayer[Player].Gem)||((arPlayer[Player].GetRitualGemPrice(Choosen)==0))))
						{buttRitual2.Activate(1);}
						else
						{buttRitual2.Activate(0);};
						buttCancel2.Activate(1);					
						buttScrollUp.Activate(-1);
						buttCancel.Activate(-1);
						buttScrollDown.Activate(-1);
						buttRitFilter.Activate(-1,arPlayer[Player].bRitFilter);
						for (int i=0; i<BAR_SIZE; i++)
						{mskBuild[i].Activate(-1);};
						};
				};
			};
			if (buttScrollUp.Check())
			{
				Shift--;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=RitualList(Player,Member,Shift,bmpInfoBar);
				if (Shift==0)
				{
					buttScrollUp.Activate(0);
				};
				buttScrollDown.Activate(1);
			};
			if (buttCancel.Check())
			{
				Condition=0;
			};
			if (buttScrollDown.Check())
			{					
				Shift++;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=RitualList(Player,Member,Shift,bmpInfoBar);
				if (Shift>=Limit-BAR_SIZE)
				{
					buttScrollDown.Activate(0);
				};
				buttScrollUp.Activate(1);					
			};
			buttRitFilter.Check();
			if (buttRitFilter.bPress!=arPlayer[Player].bRitFilter)
			{
				arPlayer[Player].bRitFilter=buttRitFilter.bPress;
				Shift=0;
				txtError.Hide();
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=RitualList(Player,Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				buttScrollUp.Activate(0);					
				if (Limit>BAR_SIZE)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};
			};				
			break;
		case 13:
	//Выбран ритуал
			if (buttRitual2.Check())
			{
				arPlayer[Player].CastRitual(Choosen,Member,-1);
				bShowAnim=true;
				iRitualer=Member;				
				Redraw=true;
				Condition=0;
				PlaySpellSound((pFirstRitual+Choosen)->Sound);
			//	play_sample((pFirstRitual+Choosen)->Sound,iSoundVolume,128,1000,0);
			};
			if (buttCancel2.Check())
			{
				Condition=12;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				txtDs.Hide();
				Limit=RitualList(Player,Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};					
				buttRitual2.Activate(-1);
				buttCancel2.Activate(-1);
				if (Shift==0)
				{buttScrollUp.Activate(0);}
				else
				{buttScrollUp.Activate(1);};
				buttCancel.Activate(1);
				if (Shift>=Limit-BAR_SIZE)
				{buttScrollDown.Activate(0);}
				else
				{buttScrollDown.Activate(1);}					
				buttRitFilter.Activate(1,arPlayer[Player].bRitFilter);
			};
			for (int i=0; i<MAX_RITUAL_RES; i++)
			{
				if ((pFirstRitual+Choosen)->arRes[i]>0)
				{
					Check=mskRes[i].Check();
					if (Check>1)
					{
						if ((Check==30)||(Check==40))
						{
							WindowResourceInfo((pFirstRitual+Choosen)->arRes[i]);
						}
						else
						{
							bHint=true;
							MakeStringHint((pFirstResource+(pFirstRitual+Choosen)->arRes[i])->sName);
						};
					};
				};
			};
			Check=mskRes[6].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(310);
			};
			Check=mskRes[7].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(311);
			};
			mskCD.Check();
			txtDs.Check();
			break;
		case 14:
	//Выбран список действующих ритуалов
			for (int i=0; i<BAR_SIZE; i++)
			{
				if (i<Limit)
				{
					Check=mskBuild[i].Check();
					if ((Check==40)||(Check==30))
					{WindowRitualInfo(arBuild[i]);};
				};
			};
			if (buttScrollUp.Check())
			{
				Shift--;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=EnchantList(Player,Member,Shift,bmpInfoBar);
				if (Shift==0)
				{
					buttScrollUp.Activate(0);
				};
				buttScrollDown.Activate(1);
			};
			if (buttCancel.Check())
			{
				Condition=0;
			};
			if (buttScrollDown.Check())
			{					
				Shift++;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=EnchantList(Player,Member,Shift,bmpInfoBar);
				if (Shift>=Limit-BAR_SIZE)
				{
					buttScrollDown.Activate(0);
				};
				buttScrollUp.Activate(1);					
			};
			break;
		case 15:
	//Выбран список героев для активизации
			for (int i=0; i<BAR_SIZE; i++)
			{
				if (i<Limit)
				{
					if (mskBuild[i].Check()==30)
					{
						Condition=20;
						arPlayer[Player].arHero[Member]->ObjN->Pic=bmpHeroDoll[arPlayer[Player].arHero[arBuild[i]]->ipClass1-1];
						arPlayer[Player].arHero[Member]->ObjA->Pic=bmpHeroDollS[arPlayer[Player].arHero[arBuild[i]]->ipClass1-1];
						arPlayer[Player].arHeroMask[Member].bmpMask=bmpHeroDoll[arPlayer[Player].arHero[arBuild[i]]->ipClass1-1];
						Member=arBuild[i];						
					};						
				};
			};
			if (buttScrollUp.Check())
			{
				Shift--;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=HeroList(Player,Member,Shift,bmpInfoBar,false);
				if (Shift==0)
				{
					buttScrollUp.Activate(0);
				};
				buttScrollDown.Activate(1);
			};			
			if (buttScrollDown.Check())
			{					
				Shift++;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=HeroList(Player,Member,Shift,bmpInfoBar,false);
				if (Shift>=Limit-BAR_SIZE)
				{
					buttScrollDown.Activate(0);
				};
				buttScrollUp.Activate(1);					
			};			
			break;
		case 16:
	//Выбран список героев для обмена
			for (int i=0; i<BAR_SIZE; i++)
			{
				if (i<Limit)
				{
					if (mskBuild[i].Check()==30)
					{
						ScreenHeroHero(arPlayer[Player].arHero[Member],arPlayer[Player].arHero[arBuild[i]]);						
						Condition=20;						
					};						
				};
			};
			if (buttScrollUp.Check())
			{
				Shift--;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=HeroList(Player,Member,Shift,bmpInfoBar,true);
				if (Shift==0)
				{
					buttScrollUp.Activate(0);
				};
				buttScrollDown.Activate(1);
			};			
			if (buttScrollDown.Check())
			{					
				Shift++;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=HeroList(Player,Member,Shift,bmpInfoBar,true);
				if (Shift>=Limit-BAR_SIZE)
				{
					buttScrollDown.Activate(0);
				};
				buttScrollUp.Activate(1);					
			};			
			break;
		case 17:
	//Выбран список особых мест для входа
			for (int i=0; i<BAR_SIZE; i++)
			{
				if (i<Limit)
				{
					if (mskBuild[i].Check()==30)
					{
						if ((Tutorial.Map)&&(Tutorial.Move==7))
						{					
							Tutorial.Move=8;
						//	if (Tutorial.Mode)
							objTutorial->iLayer=6;
							txtTutorial.Activate((pFirstTutText+13)->msg);
							bTutHint=true;
							ScrollSound();
							buttEnd.Activate(1);
							buttEnd.bBlink=true;
							buttEnter.bBlink=false;
							Tutorial.buttEnd=1;
						};
						if ((Tutorial.Map)&&(Tutorial.Move==11))
						{					
							Tutorial.Move=12;					
							objTutorial->iLayer=6;
							txtTutorial.Activate((pFirstTutText+26)->msg);
							bTutHint=true;
							ScrollSound();
							buttEnd.Activate(1);
							buttEnd.bBlink=true;
							buttEnter.bBlink=false;
							Tutorial.buttEnd=1;
						};
						if ((Tutorial.Map)&&(Tutorial.Move==15))
						{					
							Tutorial.Move=16;							
							buttEnd.Activate(1);
							buttEnd.bBlink=true;
							buttEnter.bBlink=false;
							Tutorial.buttEnd=1;
						};
						arPlayer[Player].arHero[Member]->iTask=4;
						arPlayer[Player].arHero[Member]->iTarget=arBuildNum[i];
						Condition=20;						
					};						
				};
			};
			if (buttScrollUp.Check())
			{
				Shift--;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=SiteList(arPlayer[Player].arHero[Member]->pProvince->iNumber,Shift,bmpInfoBar,true,arPlayer[Player].arHero[Member]);
				if (Shift==0)
				{
					buttScrollUp.Activate(0);
				};
				buttScrollDown.Activate(1);
			};
			if (buttCancel.Check())
			{
				Condition=20;				
			};
			if (buttScrollDown.Check())
			{					
				Shift++;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=SiteList(arPlayer[Player].arHero[Member]->pProvince->iNumber,Shift,bmpInfoBar,true,arPlayer[Player].arHero[Member]);
				if (Shift>=Limit-BAR_SIZE)
				{
					buttScrollDown.Activate(0);
				};
				buttScrollUp.Activate(1);					
			};			
			break;
		case 18:
	//Выбран список ритуалов для героя
			for (int i=0; i<BAR_SIZE; i++)
			{
				if (i<Limit)
				{
					Check=mskBuild[i].Check();
					if (Check==40)
					{WindowRitualInfo(arBuild[i]);};
					if (Check==30)
					{
						Condition=19;
						blit(bmpGUI_Building,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);						
						Choosen=arBuild[i];
						draw_sprite(bmpInfoBar,(pFirstRitual+Choosen)->Pic,52,65);						
						textprintf_centre_ex(bmpInfoBar,Font1,156,44,COLOR_1,-1,"%s",(pFirstRitual+Choosen)->sName);
						textprintf_centre_ex(bmpInfoBar,Font1,150,71,COLOR_2,-1,"%d",arPlayer[Player].GetRitualGoldPrice(Choosen));
						textprintf_centre_ex(bmpInfoBar,Font1,227,71,COLOR_2,-1,"%d",arPlayer[Player].GetRitualGemPrice(Choosen));
						draw_sprite(bmpInfoBar,bmpStuff[35],186,95);
						textprintf_centre_ex(bmpInfoBar,Font1,232,102,COLOR_2,-1,"%d",(pFirstRitual+Choosen)->iCooldown-arPlayer[Player].GetBldParam1(53));
						txtDs.Activate((pFirstRitual+Choosen)->sText);
						for (int j=0; j<MAX_RITUAL_RES; j++)
						{
							if ((pFirstRitual+Choosen)->arRes[j]>0)
							{
								draw_sprite(bmpInfoBar,(pFirstResource+(pFirstRitual+Choosen)->arRes[j])->Pic,110+j*26,97);
								if (arPlayer[Player].GetResource((pFirstRitual+Choosen)->arRes[j])==0)
								{
									draw_sprite(bmpInfoBar,bmpGUI_NoRes,110+j*26,97);
								};
							};
						};
						if ((arPlayer[Player].arHero[Member]->pProvince->CanCast(Choosen,Player))&&
							(*(arPlayer[Player].pRitualCD+Choosen)==0)&&
							((arPlayer[Player].GetRitualGoldPrice(Choosen)<=arPlayer[Player].Gold)||((arPlayer[Player].GetRitualGoldPrice(Choosen)==0)))&&
							((arPlayer[Player].GetRitualGemPrice(Choosen)<=arPlayer[Player].Gem)||((arPlayer[Player].GetRitualGemPrice(Choosen)==0))))
						{buttRitual2.Activate(1);}
						else
						{buttRitual2.Activate(0);};
						buttCancel2.Activate(1);					
						buttScrollUp.Activate(-1);
						buttCancel.Activate(-1);
						buttScrollDown.Activate(-1);
						buttRitFilter.Activate(-1,arPlayer[Player].bRitFilter);
						for (int i=0; i<BAR_SIZE; i++)
						{mskBuild[i].Activate(-1);};
						};
				};
			};
			if (buttScrollUp.Check())
			{
				Shift--;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=HeroRitualList(Player,Member,Shift,bmpInfoBar);
				if (Shift==0)
				{
					buttScrollUp.Activate(0);
				};
				buttScrollDown.Activate(1);
			};
			if (buttCancel.Check())
			{
				Condition=20;
			};
			if (buttScrollDown.Check())
			{					
				Shift++;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=HeroRitualList(Player,Member,Shift,bmpInfoBar);
				if (Shift>=Limit-BAR_SIZE)
				{
					buttScrollDown.Activate(0);
				};
				buttScrollUp.Activate(1);					
			};
			/*buttRitFilter.Check();
			if (buttRitFilter.bPress!=arPlayer[Player].bRitFilter)
			{
				arPlayer[Player].bRitFilter=buttRitFilter.bPress;
				Shift=0;
				txtError.Hide();
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=RitualList(Player,Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				buttScrollUp.Activate(0);					
				if (Limit>BAR_SIZE)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};
			};*/				
			break;
		case 19:
	//Выбран ритуал для героя
			if (buttRitual2.Check())
			{
				int OldProv=arPlayer[Player].arHero[Member]->ipProvince;
				arPlayer[Player].CastRitual(Choosen,arPlayer[Player].arHero[Member]->ipProvince,Member);
				bShowAnim=true;
				iRitualer=arPlayer[Player].arHero[Member]->ipProvince;
				if (arPlayer[Player].arHero[Member]->ipProvince!=OldProv)
				{CentreMap(arPlayer[Player].arHero[Member]->pProvince->x,arPlayer[Player].arHero[Member]->pProvince->y);}
				Redraw=true;
				Condition=20;
				PlaySpellSound((pFirstRitual+Choosen)->Sound);
				//play_sample((pFirstRitual+Choosen)->Sound,iSoundVolume,128,1000,0);
			};
			if (buttCancel2.Check())
			{
				Condition=18;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				txtDs.Hide();
				Limit=HeroRitualList(Player,Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};					
				buttRitual2.Activate(-1);
				buttCancel2.Activate(-1);
				if (Shift==0)
				{buttScrollUp.Activate(0);}
				else
				{buttScrollUp.Activate(1);};
				buttCancel.Activate(1);
				if (Shift>=Limit-BAR_SIZE)
				{buttScrollDown.Activate(0);}
				else
				{buttScrollDown.Activate(1);}					
//				buttRitFilter.Activate(1,arPlayer[Player].bRitFilter);
			};
			for (int i=0; i<MAX_RITUAL_RES; i++)
			{
				if ((pFirstRitual+Choosen)->arRes[i]>0)
				{
					Check=mskRes[i].Check();
					if (Check>1)
					{
						if ((Check==30)||(Check==40))
						{
							WindowResourceInfo((pFirstRitual+Choosen)->arRes[i]);
						}
						else
						{
							bHint=true;
							MakeStringHint((pFirstResource+(pFirstRitual+Choosen)->arRes[i])->sName);
						};
					};
				};
			};
			Check=mskRes[6].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(310);
			};
			Check=mskRes[7].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(311);
			};
			mskCD.Check();
			txtDs.Check();
			break;
		case 21:
	//Выбран герой
			if (buttExplore.Check())
			{
				arPlayer[Player].arHero[Member]->iTask=2;
				Condition=20;
			};
			if (buttPillage.Check())
			{
				arPlayer[Player].arHero[Member]->iTask=3;
				Condition=20;
			};
			if (buttEnter.Check())
			{
				if ((Tutorial.Map)&&(Tutorial.Move==6))
				{					
					Tutorial.Move=7;					
					objTutorial->iLayer=6;
					txtTutorial.Activate((pFirstTutText+12)->msg);
					bTutHint=true;
					ScrollSound();
				};				
				Condition=17;
				Shift=0;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=SiteList(arPlayer[Player].arHero[Member]->pProvince->iNumber,Shift,bmpInfoBar,true,arPlayer[Player].arHero[Member]);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				buttExplore.Activate(-1);
				buttPillage.Activate(-1);
				buttEnter.Activate(-1);
				buttAttack.Activate(-1);
				buttShop.Activate(-1);
				buttHeroRitual.Activate(-1);
				buttLibrary.Activate(-1);
				buttHeroGarr.Activate(-1);
				buttExchange.Activate(-1);
				buttSkip.Activate(-1);
				mskHeroPic.Activate(-1);				
				buttScrollUp.Activate(0);
				buttCancel.Activate(1);
				if (Limit>BAR_SIZE)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};
			};
			if (buttAttack.Check())
			{
				arPlayer[Player].arHero[Member]->iTask=5;
				Condition=20;
			};
			if (buttSkip.Check())
			{
				arPlayer[Player].arHero[Member]->iTask=0;
				Condition=20;
			};
			if (buttShop.Check())
			{
				ScreenHeroShop(arPlayer[Player].arHero[Member],&arPlayer[Player].Shop);
				Condition=20;
				if ((Tutorial.Map)&&(Tutorial.Move==5))
				{
					buttShop.bBlink=false;
					Tutorial.Move=6;
					objTutorial->iLayer=-1;
					txtTutorial.Hide();
					objMain=ShowActiveObjects(objMain,10,true,MX,MY);
					int Answer=(pFirstCampDialog+11)->Show(1,1,1,1,NULL,NULL);
					if (Answer==2)
					{(pFirstCampDialog+12)->Show(1,1,0,0,NULL,NULL);};
					if (Answer==3)
					{(pFirstCampDialog+13)->Show(1,0,0,0,NULL,NULL);};
					txtTutorial.Activate((pFirstTutText+11)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
					buttEnter.Activate(1);
					buttEnter.bBlink=true;
					Tutorial.buttEnter=1;
					Tutorial.Site=0;
				};
				if ((Tutorial.Map)&&(Tutorial.Move==9)&&(arPlayer[Player].arHero[0]->ipClass1==3))
				{
					buttShop.bBlink=false;
					Tutorial.Move=10;
				};
				if ((Tutorial.Map)&&(Tutorial.Move==13)&&(arPlayer[Player].arHero[0]->ipClass1==4))
				{
					buttShop.bBlink=false;
					Tutorial.Move=14;
				};
				if ((Tutorial.Map)&&(Tutorial.Move==17))
				{
					buttShop.bBlink=false;
					Tutorial.Move=18;					
					txtTutorial.Activate((pFirstTutText+49)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
				};
			};
			if (buttLibrary.Check())
			{
				ScreenHeroLibrary(arPlayer[Player].arHero[Member]);
				if ((Tutorial.Map)&&(Tutorial.Move==5))
				{
					buttLibrary.bBlink=false;
					Tutorial.Move=6;
					objTutorial->iLayer=-1;
					txtTutorial.Hide();
					objMain=ShowActiveObjects(objMain,10,true,MX,MY);
					char Text[500];
					sprintf (Text,(pFirstCampDialog+16)->sText,arPlayer[1].GetName());
					int Answer=(pFirstCampDialog+16)->Show(1,1,1,1,NULL,Text);
					if (Answer==2)
					{(pFirstCampDialog+12)->Show(1,1,0,0,NULL,NULL);};
					if (Answer==3)
					{(pFirstCampDialog+13)->Show(1,0,0,0,NULL,NULL);};
					txtTutorial.Activate((pFirstTutText+11)->msg);
					arPlayer[1].Gem+=50;
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
					buttEnter.Activate(1);
					buttEnter.bBlink=true;
					Tutorial.buttEnter=1;
					Tutorial.Site=0;
				};
				if ((Tutorial.Map)&&(Tutorial.Move==13)&&(arPlayer[Player].arHero[0]->ipClass1!=4))
				{
					buttLibrary.bBlink=false;
					Tutorial.Move=14;
				};
			};
			if (buttHeroGarr.Check())
			{
				ScreenHeroGarrison(arPlayer[Player].arHero[Member],arPlayer[Player].arHero[Member]->pProvince->pGarrison,false);
				Condition=20;
				if ((Tutorial.Map)&&(Tutorial.Move==5))
				{
					buttHeroGarr.bBlink=false;
					Tutorial.Move=6;
					objTutorial->iLayer=-1;
					txtTutorial.Hide();
					objMain=ShowActiveObjects(objMain,10,true,MX,MY);
					char Text[500];
					sprintf (Text,(pFirstCampDialog+14)->sText,arPlayer[1].GetName());
					int Answer=(pFirstCampDialog+14)->Show(1,1,1,1,NULL,Text);
					if (Answer==2)
					{(pFirstCampDialog+12)->Show(1,1,0,0,NULL,NULL);};
					if (Answer==3)
					{(pFirstCampDialog+15)->Show(1,0,0,0,NULL,NULL);};
					txtTutorial.Activate((pFirstTutText+11)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
					buttEnter.Activate(1);
					buttEnter.bBlink=true;
					Tutorial.buttEnter=1;
					Tutorial.Site=0;
				};
				if ((Tutorial.Map)&&(Tutorial.Move==9)&&(arPlayer[Player].arHero[0]->ipClass1!=3))
				{
					buttHeroGarr.bBlink=false;
					Tutorial.Move=10;
				};
			};
			if (buttExchange.Check())
			{
				int MaxHero=0, Finder;
				for (int i=0; i<MAX_HERO; i++)
				{
					if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->pProvince->iNumber==arPlayer[Player].arHero[Member]->pProvince->iNumber)&&(arPlayer[Player].arHero[i]->iDead<0))
					{
						MaxHero++;
						if (i!=Member)
						{
							Finder=i;
						};
					};
				};
				if (MaxHero>2)
				{
					Condition=16;
					Shift=0;					
					blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
					Limit=HeroList(Player,Member,Shift,bmpInfoBar,true);
					for (int i=0; i<BAR_SIZE; i++)
					{
						if (i<Limit)
						{mskBuild[i].Activate(7);}
						else
						{mskBuild[i].Activate(-1);};
					};
					buttExplore.Activate(-1);
					buttPillage.Activate(-1);
					buttEnter.Activate(-1);
					buttAttack.Activate(-1);
					buttShop.Activate(-1);
					buttHeroRitual.Activate(-1);
					buttLibrary.Activate(-1);
					buttHeroGarr.Activate(-1);
					buttExchange.Activate(-1);
					buttSkip.Activate(-1);
					mskHeroPic.Activate(-1);
					buttScrollUp.Activate(0);	
					if (Limit>BAR_SIZE)
					{buttScrollDown.Activate(1);}
					else
					{buttScrollDown.Activate(0);};	
				}
				else
				{
					ScreenHeroHero(arPlayer[Player].arHero[Member],arPlayer[Player].arHero[Finder]);
					Condition=20;
				};
			};

			if (buttHeroRitual.Check())
			{
				buttHeroRitual.bBlink=false;
				Condition=18;
				Shift=0;
				blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
				Limit=HeroRitualList(Player,Member,Shift,bmpInfoBar);
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				for (int i=0; i<BAR_SIZE; i++)
				{
					if (i<Limit)
					{mskBuild[i].Activate(7);}
					else
					{mskBuild[i].Activate(-1);};
				};
				if ((Limit>0)&&(Tutorial.RitualCast==0))
				{
					Tutorial.RitualCast=1;
					bTutHint=true;
					ScrollSound();
					txtTutorial.Activate((pFirstTutText+97)->msg);
					objTutorial->iLayer=6;
				};
				buttExplore.Activate(-1);
				buttPillage.Activate(-1);
				buttEnter.Activate(-1);
				buttAttack.Activate(-1);
				buttShop.Activate(-1);
				buttHeroRitual.Activate(-1);
				buttLibrary.Activate(-1);
				buttHeroGarr.Activate(-1);
				buttExchange.Activate(-1);
				buttSkip.Activate(-1);
				mskHeroPic.Activate(-1);
				buttScrollUp.Activate(0);
				buttCancel.Activate(1);				
			//	buttRitFilter.Activate(1,arPlayer[Player].bRitFilter);
				if (Limit>BAR_SIZE)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};			
			};

			if (key[KEY_G])
			{bKeyGPressed=true;};


			if ((mskHeroPic.Check()==30)||((bKeyGPressed)&&(!key[KEY_G])))
			{
				bKeyGPressed=false;
				if (ScreenHero(arPlayer[Player].arHero[Member],NULL,true))
				{Condition=20;}
				else
				{
					Condition=0;
					Member=arPlayer[Player].ipProvince;
					Redraw=true;
				};
				if ((bTutHint)&&(Tutorial.Move==15))
				{
					bTutHint=false;
					objTutorial->iLayer=-1;
					txtTutorial.Hide();
				};
			};
			{
				int Check=mskParam[0].Check();
				if (Check>1)
				{
					bHint=true;
					MakeAbilityHint(119);
				};
				if ((Check==30)||(Check==40))
				{
					bHint=false;
					WindowAbilityInfo(119,arPlayer[Player].arHero[Member]->iLevel);
				};

				Check=mskParam[1].Check();
				if (Check>1)
				{
					bHint=true;
					MakeAbilityHint(1);
				};
				if ((Check==30)||(Check==40))
				{
					bHint=false;
					WindowAbilityInfo(1,arPlayer[Player].arHero[Member]->pSquad[0]->iLife);
				};

				Check=mskParam[2].Check();
				if (Check>1)
				{
					bHint=true;
					MakeAbilityHint(116);
				};
				if ((Check==30)||(Check==40))
				{
					bHint=false;
					WindowAbilityInfo(116,arPlayer[Player].arHero[Member]->iLeader);
				};

				Check=mskParam[3].Check();
				if (Check>1)
				{
					bHint=true;
					MakeAbilityHint(117);
				};
				if ((Check==30)||(Check==40))
				{
					bHint=false;
					WindowAbilityInfo(117,arPlayer[Player].arHero[Member]->iMagic);
				};

				Check=mskParam[4].Check();
				if (Check>1)
				{
					bHint=true;
					MakeAbilityHint(118);
				};
				if ((Check==30)||(Check==40))
				{
					bHint=false;
					WindowAbilityInfo(118,arPlayer[Player].arHero[Member]->iHealth);
				};

				Check=mskParam[5].Check();
				if (Check>1)
				{
					bHint=true;
					MakeAbilityHint(999);
				};
				if ((Check==30)||(Check==40))
				{
					bHint=false;
					WindowAbilityInfo(999,arPlayer[Player].arHero[Member]->CheckHeroBonus(999));
				};

				Check=mskParam[6].Check();
				if (Check>1)
				{
					bHint=true;
					MakeAbilityHint(55);
				};
				if ((Check==30)||(Check==40))
				{
					bHint=false;
					WindowAbilityInfo(55,arPlayer[Player].arHero[Member]->GetSiege());
				};

				Check=mskParam[7].Check();
				if (Check>1)
				{
					bHint=true;
					MakeAbilityHint(998);
				};
				if ((Check==30)||(Check==40))
				{
					bHint=false;
					WindowAbilityInfo(998,arPlayer[Player].arHero[Member]->GetMobility());
				};				
			};
			break;

		};	
		
			
		InfoString="";
		objShadow->iLayer=-1;

		/*if ((CurGame.iCurrentTurn==0)&&(Iteration<1000)&&(Iteration%10==0)&&(bLogger))
		{
			sprintf(stLog,"Step: 5, Inc: %d, bFirst: %d",iIncomeY,bFirst);
			FileLog(stLog);
		};*/
		
//Проверка объекта под курсором мыши
		if ((!Redraw)&&((objBroken->iLayer<0)||(mskBroken.Check()<2))&&(mskInfoBar.Check()<2)&&(mskLowPanel.Check()<2)&&((!bTutHint)||(mskTutorial.flag<2)))
		{
			bool PrCheck=true, PrGo=true;
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0))
				{
					int Check=arPlayer[Player].arHeroMask[i].Check(MX,MY);
					if (Check>1)
					{
						if (*(Prov+arPlayer[Player].arHero[i]->pProvince->iNumber)==1)
						{InfoString=arPlayer[Player].arHero[i]->pstName->str;}
						else
						{InfoString=(pFirstPhrase+152)->str;};
						PrCheck=false;						
					};
					if (Check==30)
					{
						PrCheck=false;
						PrGo=false;
						Member=i;
						if (*(Prov+arPlayer[Player].arHero[i]->pProvince->iNumber)==1)
						{							
							Condition=20;							
						}
						else
//Отображение списка героев для выбора активного
						{
							Condition=15;
							if (GoPath)
							{
								P1=GoPath;
								while (P1)
								{
									P1=GoPath->pPrev;
									delete GoPath;
									GoPath=P1;
								};
								GoPath=NULL;
								G_Object *TMP=objMain;
								while (objMain->bTempo)
								{
									objMain=objMain->pNext;
								};
								while (TMP)
								{
									G_Object *TMP1=TMP->pNext;
									if (TMP->bTempo)
									{						
										delete TMP;						
									};
									TMP=TMP1;
								};			
							};
							Shift=0;
							objEnchant->iLayer=-1;
							objBroken->iLayer=-1;
							objAnvil1->iLayer=-1;
							objAnvil2->iLayer=-1;
							objHeroSelect->iLayer=1;
							objHeroSelect->PlaceOnMap(
								arPlayer[Player].arHero[Member]->pProvince->x-objHeroSelect->Pic->w/2,
								arPlayer[Player].arHero[Member]->pProvince->y-objHeroSelect->Pic->h/2-2);
							blit(bmpGUI_BuildBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
							Limit=HeroList(Player,Member,Shift,bmpInfoBar,false);
							for (int i=0; i<BAR_SIZE; i++)
							{
								if (i<Limit)
								{mskBuild[i].Activate(7);}
								else
								{mskBuild[i].Activate(-1);};
							};
							for (int i=0; i<4; i++)
							{mskMain[i].Activate(-1);};
							buttGuard.Activate(-1);
							buttBuild.Activate(-1);
							buttGarrison.Activate(-1);
							buttRitual.Activate(-1);
							for (int i=0; i<4; i++)
							{mskMain[i].Activate(-1);};
							buttBuild2.Activate(-1);
							buttRitual2.Activate(-1);
							buttCancel2.Activate(-1);
							buttCancel.Activate(-1);
							buttRaze.Activate(-1);
							buttDismiss.Activate(-1);
							buttGuard2.Activate(-1);
							buttBldFilter.Activate(-1,arPlayer[Player].bBldFilter);
							buttDefFilter.Activate(-1,arPlayer[Player].bDefFilter);
							buttRitFilter.Activate(-1,arPlayer[Player].bRitFilter);
							txtDs.Hide();
							txtDs1.Hide();
							txtDs2.Hide();
							txtError.Hide();
							buttExplore.Activate(-1);
							buttPillage.Activate(-1);
							buttEnter.Activate(-1);
							buttAttack.Activate(-1);
							buttShop.Activate(-1);
							buttHeroRitual.Activate(-1);
							buttLibrary.Activate(-1);
							buttHeroGarr.Activate(-1);
							buttExchange.Activate(-1);
							mskHeroPic.Activate(-1);
							buttSkip.Activate(-1);
							buttScrollUp.Activate(0);
							mskRace.Activate(-1);
							objMapSelect->iLayer=-1;
							bool bNext=false;
							for (int i=0; i<MAX_HERO; i++)
							{
								if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0)&&((Condition<17)||(Member!=i)))
								{bNext=true;};
							};
							if (bNext)
							{buttNextHero.Activate(1);}
							else
							{buttNextHero.Activate(0);};
							if (Limit>BAR_SIZE)
							{buttScrollDown.Activate(1);}
							else
							{buttScrollDown.Activate(0);};
							for (int i=0; i<8; i++)
							{mskParam[i].Activate(-1);};
							for (int i=0; i<HERO_QUEST_LIMIT; i++)
							{objHeroQuest[i]->iLayer=-1;};
						};
					};
				};
			};
			int Result=GetProvinceIndex(MX+mouse_x,MY+mouse_y);
			dcProvince * PR=(pFirstProvince+Result);

			if (*(arPlayer[Player].pFog+Result)<2)
			{
			if ((PrCheck)&&(mouse_b & 1))
			{			
				if ((Result>0)&&(PR->ipTerrain>0)&&(PR->ipTerrain<7))
				{
					Member=Result;				
					Condition=0;
				};
			};

			if ((PrGo)&&(mouse_b & 2)&&(Condition>15))
			{			
				if ((Result>0)&&((!Tutorial.Map)||(Tutorial.buttCastle>0)||
					((arPlayer[1].arHero[0]->ipProvince==26)&&(PR->iNumber==27))||
					((arPlayer[1].arHero[0]->ipProvince==27)&&(PR->iNumber==21)&&(Tutorial.Site<0)&&(Tutorial.Move>15))||
					((arPlayer[1].arHero[0]->ipProvince==21)&&(PR->iNumber==23)) ))
				{
					bool bMove=true;					
					if (PR->ipTerrain==WATER_ID)
					{
						objMain=ShowActiveObjects(objMain,10,false,MX,MY);
						WindowInfo(PR->sName,(pFirstMessage+49)->msg,bmpStuff[53]);
						bMove=false;
					};
					if (PR->ipTerrain==MOUNTAIN_ID)
					{
						objMain=ShowActiveObjects(objMain,10,false,MX,MY);
						WindowInfo(PR->sName,(pFirstMessage+48)->msg,bmpStuff[47]);
						bMove=false;
					};
					if ((bMove)&&(PR->ipPlayer!=Player)&&(arPlayer[Player].arHero[Member]->pProvince->ipPlayer!=Player)&&(!Tutorial.Map))
					{
						for (int i=0; i<MAX_NEIGHBOUR; i++)
						{
							if (PR->arNeighbour[i]==arPlayer[Player].arHero[Member]->ipProvince)
							{
								bMove=false;
								(pFirstDialog+1244)->Show(1,1,0,0,NULL,NULL);
							};
						};
					};
					if ((bMove)&&(*(pMoveMap+Result)<=0))
					{
						objMain=ShowActiveObjects(objMain,10,false,MX,MY);
						(pFirstDialog+1166)->Show(1,1,0,0,NULL,NULL);
						bMove=false;
					};
					if (bMove)
					{
						if (arPlayer[Player].arHero[Member]->pProvince->iNumber==Result)
						{
							arPlayer[Player].arHero[Member]->iTask=0;
						}
						else
						{
							arPlayer[Player].arHero[Member]->iTask=1;
								arPlayer[Player].arHero[Member]->iTarget=Result;
							if ((Tutorial.Map)&&(Tutorial.Move==3))
							{
								Tutorial.Move=4;
								bTutHint=true;
								ScrollSound();
								txtTutorial.Activate((pFirstTutText+3)->msg);
								objTutorial->iLayer=6;
								Tutorial.buttEnd=true;
								buttEnd.Activate(1);
								buttEnd.bBlink=true;
							};
						};
						Condition=20;
					};
				};
			};	
					
			if (PrCheck)
			{
				InfoString=PR->sName;

				//char TT[200];
				//if (PR->iNumber>0)
				//{
				//	sprintf(TT,"%s: %d, %d",PR->sName, *(pMoveMap+PR->iNumber), *(pMobilMap+PR->iNumber));
				//	InfoString=TT;
				//}

				if (PR->iNumber>0)
				{
					objShadow->iLayer=4;
					objShadow->Pic=(pFirstFog+PR->iNumber)->Pic;
					objShadow->PlaceOnMap((pFirstFog+PR->iNumber)->x,(pFirstFog+PR->iNumber)->y);
				}
				else
				{objShadow->iLayer=-1;};
			};		
			}
			else
			{
				InfoString=(pFirstPhrase+46)->str;
			};
		};


//Выделение активной провинции
		if ((Condition==0)&&(!Redraw))
		{
			if (GoPath)
			{
				P1=GoPath;
				while (P1)
				{
					P1=GoPath->pPrev;
					delete GoPath;
					GoPath=P1;
				};
				GoPath=NULL;
				G_Object *TMP=objMain;
				while (objMain->bTempo)
				{
					objMain=objMain->pNext;
				};
				while (TMP)
				{
					G_Object *TMP1=TMP->pNext;
					if (TMP->bTempo)
					{						
						delete TMP;						
					};
					TMP=TMP1;
				};			
			};
			dcProvince * PR=(pFirstProvince+Member);
			if (PR->ipPlayer==Player)
			{
				buttGuard.Activate(1);
				buttBuild.Activate(1);
				buttGarrison.Activate(0);
				for (int i=0; i<4; i++)
				{mskMain[i].Activate(7);};
			}
			else
			{
				buttGuard.Activate(-1);
				buttBuild.Activate(-1);
				buttGarrison.Activate(-1);
				for (int i=0; i<4; i++)
				{mskMain[i].Activate(-1);};
				for (int i=0; i<MAX_HERO; i++)
				{
					if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->pProvince==PR))
					{mskMain[2].Activate(7);};
				};
			};			
			objEnchant->iLayer=-1;
			objBroken->iLayer=-1;
			objAnvil1->iLayer=-1;
			objAnvil2->iLayer=-1;
			dcRitual *RR=pRitual;
			while ((objEnchant->iLayer<0)&&(RR))
			{		
				if ((RR->ipProvince==Member)&&(RR->ipPlayer==Player)&&(RR->Timer>0))
				{
					objEnchant->iLayer=7;
					objBroken->iLayer=7;
					mskBroken.iHint=398;
				};
				RR=RR->pNext;		
			};
			
			if (*(arPlayer[Player].pFog+Member)==0)
			{buttRitual.Activate(1);}
			else
			{buttRitual.Activate(0);};
			buttExplore.Activate(-1);
			buttPillage.Activate(-1);
			buttEnter.Activate(-1);
			buttAttack.Activate(-1);
			buttShop.Activate(-1);
			buttHeroRitual.Activate(-1);
			buttLibrary.Activate(-1);
			buttHeroGarr.Activate(-1);
			buttExchange.Activate(-1);
			buttSkip.Activate(-1);			
			buttBuild2.Activate(-1);
			buttRitual2.Activate(-1);
			buttCancel2.Activate(-1);
			buttRaze.Activate(-1);
			buttDismiss.Activate(-1);
			buttGuard2.Activate(-1);
			buttScrollUp.Activate(-1);
			buttCancel.Activate(-1);
			buttScrollDown.Activate(-1);			
			buttBldFilter.Activate(-1,arPlayer[Player].bBldFilter);
			buttDefFilter.Activate(-1,arPlayer[Player].bDefFilter);
			buttRitFilter.Activate(-1,arPlayer[Player].bRitFilter);
			txtDs.Hide();
			txtDs1.Hide();
			txtDs2.Hide();
			txtError.Hide();
			mskRace.Activate(1);
			for (int i=0; i<BAR_SIZE; i++)
			{mskBuild[i].Activate(-1);};
			for (int i=0; i<8; i++)
			{mskParam[i].Activate(-1);};
			mskHeroPic.Activate(-1);
			objHeroSelect->iLayer=-1;
			for (int i=0; i<HERO_QUEST_LIMIT; i++)
			{objHeroQuest[i]->iLayer=-1;};
			Condition=1;
			bool bNext=false;
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0)&&((Condition<17)||(Member!=i)))
				{bNext=true;};
			};
			if (bNext)
			{buttNextHero.Activate(1);}
			else
			{buttNextHero.Activate(0);};
			rectfill(bmpMapSelect,0,0,bmpMapSelect->w,bmpMapSelect->h,COLOR_0);
			set_trans_blender(200,200,200,0);
			draw_lit_sprite(bmpMapSelect,(pFirstFog+PR->iNumber)->Pic,0,0,200);
			objMapSelect->iLayer=4;
			objMapSelect->PlaceOnMap((pFirstFog+PR->iNumber)->x,(pFirstFog+PR->iNumber)->y);
			blit(bmpGUI_ProvinceBar,bmpInfoBar,0,0,0,0,bmpGUI_ProvinceBar->w,bmpGUI_ProvinceBar->h);
			set_trans_blender(128,128,128,128);
		//Общедоступная информация
			if (PR->ipPlayer>0)
			{
				draw_sprite(bmpInfoBar,bmpHerb[arPlayer[PR->ipPlayer].iColor],8,60);
				draw_sprite(bmpInfoBar,bmpHerb[arPlayer[PR->ipPlayer].iColor],273,60);
			};
			textprintf_centre_ex(bmpInfoBar,Font1,156,43,COLOR_1,-1,"%s",PR->sName);
			textprintf_centre_ex(bmpInfoBar,Font1,156,73,COLOR_1,-1,"%s",(pFirstTerrain+PR->ipTerrain)->sName);
			draw_sprite(bmpInfoBar,(pFirstRace+PR->ipRace)->Pic,52,209);
			textprintf_centre_ex(bmpInfoBar,Font1,185,215,COLOR_1,-1,"%s",(pFirstRace+PR->ipRace)->sName);
			if (PR->GetResource(1)>0)
			{
				draw_sprite(bmpInfoBar,(pFirstResource+PR->GetResource(1))->Pic,57,169);
				textprintf_centre_ex(bmpInfoBar,Font1,168,176,COLOR_1,-1,"%s",(pFirstResource+PR->GetResource(1))->sName);
			}
			else
			{
				if (PR->GetResource(0)>0)
				{
					draw_sprite(bmpInfoBar,(pFirstResource+PR->GetResource(0))->Pic,57,169);
					textprintf_centre_ex(bmpInfoBar,Font1,168,176,makecol(220,20,20),-1,"%s",(pFirstResource+PR->GetResource(0))->sName);
				}
				else
				{
					if ((PR->GetResource(-1)>0)&&(arPlayer[Player].CompareBldParam1(26,PR->ipTerrain)))
					{
						draw_sprite(bmpInfoBar,(pFirstResource+PR->GetResource(-1))->Pic,57,169);
						textprintf_centre_ex(bmpInfoBar,Font1,168,176,makecol(90,90,90),-1,"%s",(pFirstResource+PR->GetResource(-1))->sName);
					}
					else
					{
						textprintf_centre_ex(bmpInfoBar,Font1,168,176,COLOR_1,-1,"%s",(pFirstPhrase+26)->str);
					};
				};
			};

		//Скрываемая туманом
			if (*(arPlayer[Player].pFog+Member)==0)
			{
			if (PR->ipPlayer==0)
			{
				textprintf_centre_ex(bmpInfoBar,Font1,156,103,COLOR_1,-1,"%s",(pFirstProvinceType+PR->ipType)->sName);
			}
			else
			{
				if (PR->iExplore>=ProvLevel[PR->GetLevel()].iExplored)
				{
					textprintf_centre_ex(bmpInfoBar,Font1,152,103,COLOR_1,-1,"%s %d~",(pFirstPhrase+24)->str,PR->iExplore);
				}
				else
				{
					textprintf_centre_ex(bmpInfoBar,Font1,152,103,makecol(220,20,20),-1,"%s %d~",(pFirstPhrase+24)->str,PR->iExplore);
				};
			};
			int Count=0, Bld=0;	
			bool bNeedTell=true;
			int HomeCastle=0;
			for (int i=1; i<=CurGame.iMaxPlayers; i++)
			{
				if ((arPlayer[i].bInGame)&&(arPlayer[i].ipProvince==(pFirstProvince+Member)->iNumber))
				{HomeCastle=i;};
			};
			if (HomeCastle>0)
			{
				if (Player==HomeCastle)
				{
					buttGarrison.Activate(1);
				};
				textprintf_centre_ex(bmpInfoBar,Font1,184,374,COLOR_2,-1,"%d~",
					100*PR->iFort/arPlayer[HomeCastle].iWall);
				draw_sprite(bmpInfoBar,(pFirstInnerBld+149)->Pic,160,308);
				bNeedTell=false;
			};
			for (int i=0; i<MAX_PROVINCE_BLD; i++)
			{
				if (PR->arBuilding[i]>0)
				{
					Count++;
					if ((Bld==0)||(((pFirstOuterBld+PR->arBuilding[i])->iGoldCost>(pFirstOuterBld+Bld)->iGoldCost)&&((pFirstOuterBld+PR->arBuilding[i])->iWall==0)))
					{
						Bld=PR->arBuilding[i];
					};
					if ((pFirstOuterBld+PR->arBuilding[i])->iWall>0)
					{
						bNeedTell=false;
						draw_sprite(bmpInfoBar,(pFirstOuterBld+PR->arBuilding[i])->Pic,160,308);
						textprintf_centre_ex(bmpInfoBar,Font1,184,374,COLOR_2,-1,"%d~",
							100*PR->iFort/(pFirstOuterBld+PR->arBuilding[i])->iWall);
					};
					for (int j=0; j<MAX_BUILDING_ABILITY; j++)
					{
						if ((pFirstOuterBld+PR->arBuilding[i])->arAbility[j][0]==4)
						{
							if (PR->ipPlayer==Player)
							{
								buttGarrison.Activate(1);
							};
						};
					};
				};
			};
			if (bNeedTell)
			{
				textprintf_centre_ex(bmpInfoBar,Font1,186,374,COLOR_2,-1,"-");
			};
			textprintf_centre_ex(bmpInfoBar,Font1,126,374,COLOR_2,-1,"%d",Count);
			if (Bld>0)
			{
				draw_sprite(bmpInfoBar,(pFirstOuterBld+Bld)->Pic,100,308);
			};

			if ((PR->iDefender>0)&&(PR->ipPlayer>0))
			{
				draw_sprite(bmpInfoBar,(pFirstDefender+PR->iDefender)->Pic,40,308);
				int MaxLife=0, CurLife=0;
				for (int i=0; i<MAX_SQUAD; i++)
				{
					if (PR->pGuard->pSquad[i])
					{
					//	if (PR->pGuard->pSquad[i]->ipType>100)
					//	{PR->pGuard->pSquad[i]->ipType=11;};
						MaxLife+=PR->pGuard->pSquad[i]->GetLife();
						CurLife+=PR->pGuard->pSquad[i]->iLife;
					};
				};
				textprintf_centre_ex(bmpInfoBar,Font1,64,374,COLOR_2,-1,"%d~",100*CurLife/MaxLife);
			}
			else
			{
				textprintf_centre_ex(bmpInfoBar,Font1,66,374,COLOR_2,-1,"-");
			};

			Count=0; Bld=0; bNeedTell=true;
			for (int i=0; i<MAX_PROVINCE_SITE; i++)
			{
				if ((PR->arSite[i][0]>0)&&(PR->arSite[i][1]<=PR->iExplore))
				{							
					Bld=PR->arSite[i][0];
					Count++;
				};
			};
			textprintf_centre_ex(bmpInfoBar,Font1,246,374,COLOR_2,-1,"%d",Count);
			if (Bld>0)
			{
				draw_sprite(bmpInfoBar,(pFirstSite+Bld)->Pic,220,308);
			};

			if (PR->GetGoldIncome()>0)
			{
				textprintf_centre_ex(bmpInfoBar,Font1,115,144,COLOR_2,-1,"+%d",PR->GetGoldIncome());
			}
			else
			{
				textprintf_centre_ex(bmpInfoBar,Font1,115,144,COLOR_2,-1,"%d",PR->GetGoldIncome());
			};
			if (PR->GetGemIncome()>0)
			{
				textprintf_centre_ex(bmpInfoBar,Font1,230,144,COLOR_2,-1,"+%d",PR->GetGemIncome());
			}
			else
			{
				textprintf_centre_ex(bmpInfoBar,Font1,230,144,COLOR_2,-1,"%d",PR->GetGemIncome());
			};
			
			
			if (PR->GetLevel()<MAX_PROVINCE_LEVEL)
			{
				if (PR->iPopulation<=0)
				{
					for (int i=0; i<(10-(-PR->iPopulation)%10)*20; i++)
					{
						rectfill(bmpInfoBar,56+i,268,56+i,290,makecol(i/5,50+i,i/10));
					};
				}
				else
				{
					for (int i=0; i<200*(PR->iPopulation-ProvLevel[PR->GetLevel()-1].iPopulation)/(ProvLevel[PR->GetLevel()].iPopulation-ProvLevel[PR->GetLevel()-1].iPopulation); i++)
					{
						rectfill(bmpInfoBar,56+i,268,57+i,290,makecol(i/5,50+i,i/10));
					};
				};
			};
			textprintf_centre_ex(bmpInfoBar,Font1,158,276,COLOR_1,-1,"%s",ProvLevel[PR->GetLevel()].sName);
			textprintf_centre_ex(bmpInfoBar,Font1,157,275,COLOR_2,-1,"%s",ProvLevel[PR->GetLevel()].sName);

			for (int i=0; i<145*PR->arUnrest[PR->ipPlayer]/MAX_UNREST; i++)
			{
				rectfill(bmpInfoBar,110+i,238,110+i,260,makecol(110+i,20+i/7,20+i/7));
			};
			int Mood=PR->GetMood();
			if (Mood<-5)
			{Mood=-5;};
			if (Mood>3)
			{Mood=3;};
			if (PR->iPopulation>0)
			{
				textprintf_centre_ex(bmpInfoBar,Font1,188,245,COLOR_1,-1,"%s",(pFirstPhrase+Mood+80)->str);
				if (Mood>=0)
				{textprintf_centre_ex(bmpInfoBar,Font1,187,244,makecol(200-Mood*20,200+Mood*15,20+Mood*40),-1,"%s",(pFirstPhrase+Mood+80)->str);}
				else
				{textprintf_centre_ex(bmpInfoBar,Font1,187,244,makecol(200-Mood*10,200+Mood*40,20+Mood*4),-1,"%s",(pFirstPhrase+Mood+80)->str);};
			}
			else
			{
				textprintf_centre_ex(bmpInfoBar,Font1,188,245,COLOR_1,-1,"%s",(pFirstPhrase+231)->str);
				textprintf_centre_ex(bmpInfoBar,Font1,187,244,COLOR_2,-1,"%s",(pFirstPhrase+231)->str);
			};
		};
		};

// Выделение активного героя
		if ((Condition==20)&&(!Redraw))
		{
			if ((Tutorial.Map)&&(Tutorial.Move==2))
			{
				Tutorial.Move=3;
				bTutHint=true;
				ScrollSound();
				txtTutorial.Activate((pFirstTutText+2)->msg);
				objTutorial->iLayer=6;
				buttNextHero.bBlink=false;
			};
			Condition=21;
			if (GoPath)
			{
				P1=GoPath;
				while (P1)
				{
					P1=GoPath->pPrev;
					delete GoPath;
					GoPath=P1;
				};
				GoPath=NULL;
				G_Object *TMP=objMain;
				while (objMain->bTempo)
				{
					objMain=objMain->pNext;
				};
				while (TMP)
				{
					G_Object *TMP1=TMP->pNext;
					if (TMP->bTempo)
					{						
						delete TMP;						
					};
					TMP=TMP1;
				};			
			};			
			objEnchant->iLayer=-1;
			objBroken->iLayer=-1;
			objAnvil1->iLayer=-1;
			objAnvil2->iLayer=-1;
			int Broken=0;
			for (int i=1; i<16; i++)
			{
				if ((arPlayer[Player].arHero[Member]->arInventory[i][0]>0)&&((pFirstItem+arPlayer[Player].arHero[Member]->arInventory[i][0])->iDurability>1))
				{
					if (arPlayer[Player].arHero[Member]->arInventory[i][1]==0)
					{
						Broken=2;
						mskBroken.iHint=370;
						break;
					};
					if (arPlayer[Player].arHero[Member]->arInventory[i][1]<=(pFirstItem+arPlayer[Player].arHero[Member]->arInventory[i][0])->iDurability/5)
					{
						Broken=1;
						mskBroken.iHint=369;
					};
				};
			};
			if (Broken>0)
			{
				objBroken->iLayer=7;				
				if (Broken==1)
				{objAnvil1->iLayer=7;}
				else
				{objAnvil2->iLayer=7;};
			};
			
			buttGuard.Activate(-1);
			buttBuild.Activate(-1);
			buttGarrison.Activate(-1);
			buttRitual.Activate(-1);
			for (int i=0; i<4; i++)
			{mskMain[i].Activate(-1);};
			buttBuild2.Activate(-1);
			buttRitual2.Activate(-1);
			buttCancel2.Activate(-1);
			buttRaze.Activate(-1);
			buttDismiss.Activate(-1);
			buttGuard2.Activate(-1);
			buttScrollUp.Activate(-1);
			buttCancel.Activate(-1);
			buttScrollDown.Activate(-1);
			buttBldFilter.Activate(-1,arPlayer[Player].bBldFilter);
			buttDefFilter.Activate(-1,arPlayer[Player].bDefFilter);
			buttRitFilter.Activate(-1,arPlayer[Player].bRitFilter);			
			txtDs.Hide();
			txtDs1.Hide();
			txtDs2.Hide();
			txtError.Hide();
			mskRace.Activate(-1);
			objMapSelect->iLayer=-1;
			bool bNext=false;
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0)&&((Condition<17)||(Member!=i)))
				{bNext=true;};
			};
			if (bNext)
			{buttNextHero.Activate(1);}
			else
			{buttNextHero.Activate(0);};
			for (int i=0; i<HERO_QUEST_LIMIT; i++)
			{objHeroQuest[i]->iLayer=-1;};
			for (int i=0; i<BAR_SIZE; i++)
			{mskBuild[i].Activate(-1);};
			for (int i=0; i<8; i++)
			{mskParam[i].Activate(1);};
			dcProvince *PR=arPlayer[Player].arHero[Member]->pProvince;
			ClearMoveMap();
		
			MovePlayer=Player;
			WalkType=1+arPlayer[Player].arHero[Member]->CheckSkill(26);
			WalkSpeed=arPlayer[Player].arHero[Member]->GetMobility();
		//		+arPlayer[Player].arHero[Member]->pProvince->CheckBldAbil(2);
			MoveMap(PR->iNumber,-1,WalkSpeed,0,0);
			objHeroSelect->iLayer=1;
			objHeroSelect->PlaceOnMap(
				arPlayer[Player].arHero[Member]->pProvince->x-objHeroSelect->Pic->w/2,
				arPlayer[Player].arHero[Member]->pProvince->y-objHeroSelect->Pic->h/2-2);
			if (arHumans[arPlayer[Player].iColor].arMap[5])
			{
				for (int i=0; i<HERO_QUEST_LIMIT; i++)
				{
					if (arPlayer[Player].arHero[Member]->arQuest[i].Index>0)
					{
						arPlayer[Player].arHero[Member]->CheckQuestDone(i);
						if (arPlayer[Player].arHero[Member]->arQuest[i].bComplete)
						{objHeroQuest[i]->Pic=bmpQuest[1];}
							else
						{objHeroQuest[i]->Pic=bmpQuest[0];};
						objHeroQuest[i]->PlaceOnMap(
							(pFirstProvince+arPlayer[Player].arHero[Member]->arQuest[i].ipProvince)->x-25,
							(pFirstProvince+arPlayer[Player].arHero[Member]->arQuest[i].ipProvince)->y-25);
						objHeroQuest[i]->iLayer=4;
					};
				};
			};
			blit(bmpGUI_HeroBar,bmpInfoBar,0,0,0,0,bmpGUI_HeroBar->w,bmpGUI_HeroBar->h);			
			draw_sprite(bmpInfoBar,arPlayer[Player].arHero[Member]->smPic,133,39);
			draw_sprite(bmpInfoBar,bmpHerb[arPlayer[Player].iColor],8,60);
			draw_sprite(bmpInfoBar,bmpHerb[arPlayer[Player].iColor],273,60);
			textprintf_centre_ex(bmpInfoBar,Font1,160,103,COLOR_1,-1,"%s",arPlayer[Player].arHero[Member]->pstName->str);
			textprintf_centre_ex(bmpInfoBar,Font1,160,130,COLOR_1,-1,"%s",ClassName[arPlayer[Player].arHero[Member]->GetClassName()].str);
			textprintf_centre_ex(bmpInfoBar,Font1,108,165,COLOR_2,-1,"%d",arPlayer[Player].arHero[Member]->iLeader);
			textprintf_centre_ex(bmpInfoBar,Font1,170,165,COLOR_2,-1,"%d",arPlayer[Player].arHero[Member]->iMagic);
			textprintf_centre_ex(bmpInfoBar,Font1,229,165,COLOR_2,-1,"%d",arPlayer[Player].arHero[Member]->iHealth);
			textprintf_centre_ex(bmpInfoBar,Font1,108,199,COLOR_2,-1,"%d",arPlayer[Player].arHero[Member]->CheckHeroBonus(999));			
			textprintf_centre_ex(bmpInfoBar,Font1,170,199,COLOR_2,-1,"%d",arPlayer[Player].arHero[Member]->GetSiege());
			if (PR->ipPlayer==Player)
			{
				if (PR->GetStable()>0)
				{
					textprintf_centre_ex(bmpInfoBar,Font1,229,199,COLOR_2,-1,"%d+%d",arPlayer[Player].arHero[Member]->GetMobility(),PR->GetStable());
				}
				else
				{
					textprintf_centre_ex(bmpInfoBar,Font1,229,199,COLOR_2,-1,"%d",arPlayer[Player].arHero[Member]->GetMobility());
				}
			}
			else
			{textprintf_centre_ex(bmpInfoBar,Font1,229,199,COLOR_2,-1,"1");};
			textprintf_centre_ex(bmpInfoBar,Font1,158,248,COLOR_1,-1,"%s:",(pFirstPhrase+39)->str);
			int Expobar=50*arPlayer[Player].arHero[Member]->pSquad[0]->iLife/arPlayer[Player].arHero[Member]->pSquad[0]->GetLife();
			if (Expobar>50)
			{Expobar=50;};
			for (int i=0; i<Expobar; i++)
			{
				rectfill(bmpInfoBar,195+i,69,195+i,91,makecol(100+i*3,20+i*5/20,20+i*5/50));
			};
			textprintf_centre_ex(bmpInfoBar,Font1,221,77,COLOR_1,-1,"%d",arPlayer[Player].arHero[Member]->pSquad[0]->iLife);
			textprintf_centre_ex(bmpInfoBar,Font1,220,76,COLOR_2,-1,"%d",arPlayer[Player].arHero[Member]->pSquad[0]->iLife);

			Expobar=50*(arPlayer[Player].arHero[Member]->iExp-arPlayer[Player].arHero[Member]->ExpForLevel(arPlayer[Player].arHero[Member]->iLevel))/
				(arPlayer[Player].arHero[Member]->ExpForLevel(arPlayer[Player].arHero[Member]->iLevel+1)-arPlayer[Player].arHero[Member]->ExpForLevel(arPlayer[Player].arHero[Member]->iLevel));
			if (Expobar>50)
			{Expobar=50;};

			for (int i=0; i<Expobar; i++)
			{
				rectfill(bmpInfoBar,73+i,69,73+i,91,makecol(80+i*27/10,10+i*5/20,100+i*3));
			};
			textprintf_centre_ex(bmpInfoBar,Font1,99,77,COLOR_1,-1,"%d",arPlayer[Player].arHero[Member]->iLevel);
			textprintf_centre_ex(bmpInfoBar,Font1,98,76,COLOR_2,-1,"%d",arPlayer[Player].arHero[Member]->iLevel);


			if (arPlayer[Player].arHero[Member]->iTask>0)
			{
				buttExplore.Activate(-1);
				buttPillage.Activate(-1);
				buttEnter.Activate(-1);
				buttAttack.Activate(-1);
				buttSkip.Activate(1);
				switch (arPlayer[Player].arHero[Member]->iTask)
				{
		//Поиск пути если указана провинция для движения
				case 1:
					PathKicker=0;
					MovePath(PR->iNumber,1,NULL,arPlayer[Player].arHero[Member]->iTarget,WalkSpeed,0,0,0);
					if (GoPath==NULL)
					{
						PathKicker=1;
						MovePath(PR->iNumber,1,NULL,arPlayer[Player].arHero[Member]->iTarget,WalkSpeed,0,0,0);
					};
					P1=GoPath;
					while (P1)
					{
						if (P1->pPrev)
						{
							if (P1->y==1)
							{objMain=new G_Object(objMain,0,0,3,bmpPath[0]);}
							else
							{objMain=new G_Object(objMain,0,0,3,bmpPath[1]);};
							objMain->bTempo=true;
							objMain->PlaceOnMap(
								(pFirstProvince+P1->x)->x-bmpPath[0]->w/2,
								(pFirstProvince+P1->x)->y-bmpPath[0]->h/2);
							if (P1->pPrev->y==1)
							{objMain=new G_Object(objMain,0,0,3,bmpPath[0]);}
							else
							{objMain=new G_Object(objMain,0,0,3,bmpPath[1]);};
							objMain->bTempo=true;
							objMain->PlaceOnMap(
								((pFirstProvince+P1->pPrev->x)->x+(pFirstProvince+P1->x)->x)/2-bmpPath[0]->w/2,
								((pFirstProvince+P1->pPrev->x)->y+(pFirstProvince+P1->x)->y)/2-bmpPath[0]->h/2);
						}
						else
						{
							if (P1->y==1)
							{objMain=new G_Object(objMain,0,0,3,bmpPath[2]);}
							else
							{objMain=new G_Object(objMain,0,0,3,bmpPath[3]);};
							objMain->bTempo=true;
							objMain->PlaceOnMap(
								(pFirstProvince+P1->x)->x-bmpPath[2]->w/2,
								(pFirstProvince+P1->x)->y-bmpPath[2]->h/2);
						};				
						P1=P1->pPrev;				
					};					
					textprintf_centre_ex(bmpInfoBar,Font1,150,275,COLOR_1,-1,"%s",(pFirstPhrase+41)->str);
					textprintf_centre_ex(bmpInfoBar,Font1,150,295,COLOR_1,-1,"%s",(pFirstProvince+arPlayer[Player].arHero[Member]->iTarget)->sName);
					break;
				case 2:
					textprintf_centre_ex(bmpInfoBar,Font1,150,275,COLOR_1,-1,"%s",(pFirstPhrase+42)->str);
					break;
				case 3:
					textprintf_centre_ex(bmpInfoBar,Font1,150,275,COLOR_1,-1,"%s",(pFirstPhrase+43)->str);
					break;
				case 4:
					textprintf_centre_ex(bmpInfoBar,Font1,150,275,COLOR_1,-1,"%s",(pFirstPhrase+44)->str);
					textprintf_centre_ex(bmpInfoBar,Font1,150,295,COLOR_1,-1,"%s",(pFirstSite+arPlayer[Player].arHero[Member]->pProvince->arSite[arPlayer[Player].arHero[Member]->iTarget][0])->sName);
					break;
				case 5:
					textprintf_centre_ex(bmpInfoBar,Font1,150,275,COLOR_1,-1,"%s",(pFirstPhrase+45)->str);
					break;
				};
			}
			else
			{
				if ((PR->ipPlayer==Player)||((Tutorial.Map)&&(Tutorial.buttCastle==0)))
				{
				if (PR->iExplore<100)
				{buttExplore.Activate(1);}
				else
				{buttExplore.Activate(0);};
				if (PR->iPopulation>0)
				{buttPillage.Activate(1);}
				else
				{buttPillage.Activate(0);}
				buttEnter.Activate(0);
				for (int i=0; i<MAX_PROVINCE_SITE; i++)
				{
					if ((PR->arSite[i][0]>0)&&((PR->arSiteGuard[i])||((pFirstSite+PR->arSite[i][0])->bCanEnter))&&(PR->arSite[i][1]<=PR->iExplore))
					{buttEnter.Activate(1);};
				};
				if ((PR->iDefender>0)&&((pFirstDefender+PR->iDefender)->bNoDismiss))
				{
					buttAttack.Activate(1);
					buttPillage.Activate(-1);
				}
				else
				{buttAttack.Activate(-1);};				
				buttSkip.Activate(-1);
				textprintf_centre_ex(bmpInfoBar,Font1,150,275,COLOR_1,-1,"%s",(pFirstPhrase+40)->str);
				}
				else
				{
					buttExplore.Activate(0);
					buttPillage.Activate(0);
					buttEnter.Activate(0);
					buttAttack.Activate(0);
					buttSkip.Activate(-1);
					textprintf_centre_ex(bmpInfoBar,Font1,150,275,COLOR_1,-1,"%s",(pFirstPhrase+71)->str);
				};
			};
			
			buttHeroRitual.Activate(0);
			buttShop.Activate(0);
			buttLibrary.Activate(0);
			buttHeroGarr.Activate(0);
			if (*(Prov+arPlayer[Player].arHero[Member]->pProvince->iNumber)!=1)
			{
				buttExchange.Activate(1);
			}
			else
			{
				buttExchange.Activate(0);
			};
			if (PR->ipPlayer==Player)
			{
			buttHeroRitual.Activate(1);
			if (arPlayer[Player].ipProvince==PR->iNumber)
			{
				buttHeroGarr.Activate(1);
			//	if (arPlayer[Player].Shop.Items[0][0]>0)
				{buttShop.Activate(1);};
				if (arPlayer[Player].arLibrary[0]>0)
				{buttLibrary.Activate(1);};
			};			
			for (int i=0; i<MAX_PROVINCE_BLD; i++)
			{
				if (PR->arBuilding[i]>0)
				{
					for (int j=0; j<MAX_BUILDING_ABILITY; j++)
					{
						if ((pFirstOuterBld+PR->arBuilding[i])->arAbility[j][0]==4)
						{
							buttHeroGarr.Activate(1);
						};
					};
					if ((pFirstOuterBld+PR->arBuilding[i])->bShop)
					{
						buttShop.Activate(1);
					};
					if ((pFirstOuterBld+PR->arBuilding[i])->bLibrary)
					{
						buttLibrary.Activate(1);
					};

				};
			};
			};
			if ((Tutorial.Map)&&(Tutorial.Trade>0)&&(arPlayer[Player].arHero[Member]->ipProvince==27))
			{buttShop.Activate(1);};
			if ((Tutorial.Map)&&(Tutorial.Hire>0)&&(arPlayer[Player].arHero[Member]->ipProvince==27))
			{buttHeroGarr.Activate(1);};
			if ((Tutorial.Map)&&(Tutorial.Library>0)&&(arPlayer[Player].arHero[Member]->ipProvince==27))
			{buttLibrary.Activate(1);};
			if ((Tutorial.Map)&&(Tutorial.Site>=0)&&(arPlayer[Player].arHero[Member]->ipProvince==27))
			{buttEnter.Activate(1);};
			mskHeroPic.Activate(7);
			

		};



		blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);
		textprintf_centre_ex(bmpLowPanel,Font1,500,79,COLOR_2,-1,"%s",InfoString);
		textprintf_centre_ex(bmpLowPanel,Font1,156,79,COLOR_2,-1,"%d",arPlayer[Player].Gold);
		textprintf_centre_ex(bmpLowPanel,Font1,280,79,COLOR_2,-1,"%d",arPlayer[Player].Gem);

		for (int j=0; j<8; j++)
		{
			int Pick=-1;
			if ((arPlayer[Player].arHero[j])&&(arPlayer[Player].arHero[j]->iDead<0)&&(hotHeroes[j].Check()))
			{
				Pick=j;
			};
			if (Pick>=0)
			{
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0)&&
					(arPlayer[Player].arHero[i]->ObjN)&&(arPlayer[Player].arHero[i]->ObjA)&&
					(arPlayer[Player].arHero[i]->pProvince==arPlayer[Player].arHero[Pick]->pProvince)&&
					(arPlayer[Player].arHeroMask[i].flag>0))
				{
					arPlayer[Player].arHeroMask[i].bmpMask=bmpHeroDoll[arPlayer[Player].arHero[Pick]->ipClass1-1];
					arPlayer[Player].arHero[i]->ObjN->Pic=bmpHeroDoll[arPlayer[Player].arHero[Pick]->ipClass1-1];
					arPlayer[Player].arHero[i]->ObjA->Pic=bmpHeroDollS[arPlayer[Player].arHero[Pick]->ipClass1-1];
				};
			};			
			Member=Pick;
			Condition=20;
			CentreMap(arPlayer[Player].arHero[Member]->pProvince->x,arPlayer[Player].arHero[Member]->pProvince->y);
			};
		};

		
		if (buttNextHero.Check())
		{
			int Pick=-1;
			if (Condition>17)
			{
				for (int i=Member+1; (i<MAX_HERO)&&(Pick<0); i++)
				{
					if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0))
					{
						Pick=i;
						break;
					};
				};
			};
			if (Pick<0)
			{
				for (int i=0; (i<MAX_HERO)&&(Pick<0); i++)
				{
					if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0))
					{
						Pick=i;
						break;
					};
				};
			};			
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0)&&
					(arPlayer[Player].arHero[i]->ObjN)&&(arPlayer[Player].arHero[i]->ObjA)&&
					(arPlayer[Player].arHero[i]->pProvince==arPlayer[Player].arHero[Pick]->pProvince)&&
					(arPlayer[Player].arHeroMask[i].flag>0))
				{
					arPlayer[Player].arHeroMask[i].bmpMask=bmpHeroDoll[arPlayer[Player].arHero[Pick]->ipClass1-1];
					arPlayer[Player].arHero[i]->ObjN->Pic=bmpHeroDoll[arPlayer[Player].arHero[Pick]->ipClass1-1];
					arPlayer[Player].arHero[i]->ObjA->Pic=bmpHeroDollS[arPlayer[Player].arHero[Pick]->ipClass1-1];
				};
			};			
			Member=Pick;
			Condition=20;
			CentreMap(arPlayer[Player].arHero[Member]->pProvince->x,arPlayer[Player].arHero[Member]->pProvince->y);
		};

		/*if ((CurGame.iCurrentTurn==0)&&(Iteration<1000)&&(Iteration%10==0)&&(bLogger))
		{
			sprintf(stLog,"Step: 6, Inc: %d, bFirst: %d",iIncomeY,bFirst);
			FileLog(stLog);
		};*/

		if (buttCastle.Check())
		{
			if ((pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()<0)
			{
				CastleProv=0;
				int Pick=CastleMain(Player);
				if (Pick>0)
				{
					Member=Pick-1;
					Condition=20;
					CentreMap((pFirstProvince+arPlayer[Player].ipProvince)->x,(pFirstProvince+arPlayer[Player].ipProvince)->y);
				}
				else
				{
					if (CastleProv>0)
					{
						Member=CastleProv;
						Condition=0;
						CentreMap((pFirstProvince+Member)->x,(pFirstProvince+Member)->y);
					}
					else
					{
						if (Condition<15)
						{Condition=0;};
						if (Condition>15)
						{Condition=20;};
						if (Condition==15)
						{
							Condition=0;
							Member=arPlayer[Player].ipProvince;
						};
					};
				};
				Redraw=true;
			}
			else
			{
				(pFirstDialog+388)->Show(true,true,false,false,NULL,NULL);
			};
		};

		if (buttDipl.Check())
		{
			if (buttDipl.bBlink)
			{
				txtTutorial.Hide();
				objTutorial->iLayer=-1;
				bTutHint=false;
				buttDipl.bBlink=false;
			};
			ScreenDiplomacy(Player);
		};

		if (buttStat.Check())
		{			
			int NProv=ScreenStatistics(Player);
			buttStat.bBlink=false;
			if (NProv>0)
			{
				Member=NProv;
				Condition=0;
				CentreMap((pFirstProvince+Member)->x,(pFirstProvince+Member)->y);
			};
		};
		if (((Condition>1)&&(Condition<15))||((Condition>16)&&(Condition<20)))
		{buttMenu.ipHotkey=-100;}
		else
		{buttMenu.ipHotkey=KEY_ESC;};
		if (buttMenu.Check())
		{
			objMain=ShowActiveObjects(objMain,10,false,MX,MY);
			bool bSett=WindowMenu(Player);
			if (bExitGame)
			{objMain=ShowActiveObjects(objMain,10,true,MX,MY);}
			else
			{
				if (bSett)
				{
					if (Condition<15)
					{Condition=0;};
					if (Condition>15)
					{Condition=20;};
					if (Condition==15)
					{
						Condition=0;
						Member=arPlayer[Player].ipProvince;
					};
					Redraw=true;
				};
			};
			if (!arPlayer[Player].bInGame)
			{bEndTurn=true;};
		//	bExitGame=true;	
		//	iExitFlag=0;
		};

		if ((key[KEY_ENTER])||(key[KEY_ENTER_PAD]))
		{bEnterPressed=true;}
		else
		{
			if (bEnterPressed)
			{
				bEnterPressed=false;
				if (Condition<16)
				{CentreMap((pFirstProvince+Member)->x, (pFirstProvince+Member)->y);}
				else
				{CentreMap(arPlayer[Player].arHero[Member]->pProvince->x, arPlayer[Player].arHero[Member]->pProvince->y);};
			}
		}

		if (buttEnd.Check())
		{
			int Pick;
			bEndTurn=true;
			if (arHumans[arPlayer[Player].iColor].arMess[2])
			{
				for (int i=0; i<MAX_HERO; i++)
				{
					if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0)&&(arPlayer[Player].arHero[i]->iTask==0)&&(arPlayer[Player].arHero[i]->pProvince->ipPlayer==Player))
					{
						bEndTurn=false;
						Pick=i;
						break;
					};
				};
				if (!bEndTurn)
				{
					if  ((pFirstDialog+390)->Show(true,true,false,false,NULL,NULL)==1)
					{bEndTurn=true;}
					else
					{
						for (int i=0; i<MAX_HERO; i++)
						{
							if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead<0)&&
								(arPlayer[Player].arHero[i]->ObjN)&&(arPlayer[Player].arHero[i]->ObjA)&&
								(arPlayer[Player].arHero[i]->pProvince==arPlayer[Player].arHero[Pick]->pProvince)&&
								(arPlayer[Player].arHeroMask[i].flag>0))
							{
								arPlayer[Player].arHeroMask[i].bmpMask=bmpHeroDoll[arPlayer[Player].arHero[Pick]->ipClass1-1];
								arPlayer[Player].arHero[i]->ObjN->Pic=bmpHeroDoll[arPlayer[Player].arHero[Pick]->ipClass1-1];
								arPlayer[Player].arHero[i]->ObjA->Pic=bmpHeroDollS[arPlayer[Player].arHero[Pick]->ipClass1-1];
							};
						};			
						Member=Pick;
						Condition=20;
						CentreMap(arPlayer[Player].arHero[Member]->pProvince->x,arPlayer[Player].arHero[Member]->pProvince->y);
					};
				};
			};
		};

		if (arPlayer[Player].GetBldParam1(34)>0)
		{
			Check=mskAlchemy.Check();
			if ((Check==30)||(Check==40))
			{
				objMain=ShowActiveObjects(objMain,10,false,MX,MY);
				WindowAlchemy(Player);
			};
		};

		if (key[KEY_P])
		{
			int NewProv=ScreenProvList(Player);
			if (NewProv>0)
			{
				Member=NewProv;
				Condition=0;
				CentreMap((pFirstProvince+Member)->x,(pFirstProvince+Member)->y);
			};
		};

		
		Check=mskCheat.Check();
		if (((Check==30)||(Check==40))&&(key[KEY_C])&&(key[KEY_H])&&(key[KEY_E]))
		{BF_Cheat=true;}
		else
		{
		if ((BF_Cheat)&&((Check==30)||(Check==40)))
		{
			poll_keyboard();
			if (key[KEY_X])
			{arPlayer[1].iTimeShift--;};
   			if (key[KEY_SPACE])
			{
				arPlayer[Player].Gold+=50000;
				arPlayer[Player].Gem+=5000;					
				arPlayer[Player].iCanInnerBld=1000;
				arPlayer[Player].iCanOuterBld=1000;
				arPlayer[Player].iCanCast=1000;
				arPlayer[Player].iCanHire=1000;
				for (int i=1; i<=MapSize; i++)
				{
					*(arPlayer[Player].pFog+i)=0;
				};
				bFogCheat=true;
				Redraw=true;
			};
			

			
			if ((key[KEY_W])&&(key[KEY_Q]))
			{
				for (int i=0; i<=iMaxInnerBld; i++)
				{
					Campaign.arCampPlayer[0].arBld[i]=1;
				};
			};

			if (key[KEY_K])
			{
				arPlayer[Player].iKarma+=20;
			};

			if ((key[KEY_E])&&(Condition==21))
			{
	//			arPlayer[Player].arHero[Member]->arInventory[10][0]=374;
	//			arPlayer[Player].arHero[Member]->arInventory[10][1]=1;					
				arPlayer[Player].arHero[Member]->iExp+=1000;
				while (arPlayer[Player].arHero[Member]->iLevel<arPlayer[Player].arHero[Member]->CountLevel())
				{
					WindowHeroLvlUp(arPlayer[Player].arHero[Member]);
				};
			};

			if ((key[KEY_I])&&(Condition==21))
			{	
				arPlayer[Player].arHero[Member]->arInventory[22][0]=392;
				arPlayer[Player].arHero[Member]->arInventory[22][1]=1;
			};

			if (key[KEY_L])
			{
				bLog=!bLog;
			};

			if ((key[KEY_LCONTROL])||(key[KEY_RCONTROL]))
			{
				if (key[KEY_0])
				{arPlayer[Player].iUnion=0;};
				if (key[KEY_1])
				{arPlayer[Player].iUnion=2;};
				if (key[KEY_2])
				{arPlayer[Player].iUnion=3;};
				if (key[KEY_3])
				{arPlayer[Player].iUnion=4;};
				if (key[KEY_4])
				{arPlayer[Player].iUnion=5;};
				if (key[KEY_5])
				{arPlayer[Player].iUnion=6;};
				if (key[KEY_6])
				{arPlayer[Player].iUnion=7;};
				if (key[KEY_7])
				{arPlayer[Player].iUnion=8;};
			};

			if (key[KEY_1])
			{
				for (int j=0; j<22; j++)
				{
					arPlayer[Player].arLibrary[j]=j;
				};
			};

			if (key[KEY_2])
			{
				for (int j=0; j<18; j++)
				{
					arPlayer[Player].arLibrary[j]=22+j;
				};
			};

			if (key[KEY_3])
			{
				for (int j=0; j<18; j++)
				{
					arPlayer[Player].arLibrary[j]=40+j;
				};
			};

			if (key[KEY_4])
			{
				for (int j=0; j<18; j++)
				{
					arPlayer[Player].arLibrary[j]=58+j;
				};
			};

			if (key[KEY_F1])
			{
				if (arPlayer[1].Control==1)
				{arPlayer[1].Control=2;}
				else
				{arPlayer[1].Control=1;};
			};

			if (key[KEY_F2])
			{
				if (arPlayer[2].Control==1)
				{arPlayer[2].Control=2;}
				else
				{arPlayer[2].Control=1;};
			};

			if (key[KEY_F3])
			{
				if (arPlayer[3].Control==1)
				{arPlayer[3].Control=2;}
				else
				{arPlayer[3].Control=1;};
			};

			if (key[KEY_F4])
			{
				if (arPlayer[4].Control==1)
				{arPlayer[4].Control=2;}
				else
				{arPlayer[4].Control=1;};
			};

			if (key[KEY_F5])
			{
				if (arPlayer[5].Control==1)
				{arPlayer[5].Control=2;}
				else
				{arPlayer[5].Control=1;};
			};

			if (key[KEY_F6])
			{
				if (arPlayer[6].Control==1)
				{arPlayer[6].Control=2;}
				else
				{arPlayer[6].Control=1;};
			};

			if (key[KEY_F7])
			{
				if (arPlayer[7].Control==1)
				{arPlayer[7].Control=2;}
				else
				{arPlayer[7].Control=1;};
			};

			if (key[KEY_F8])
			{
				if (arPlayer[8].Control==1)
				{arPlayer[8].Control=2;}
				else
				{arPlayer[8].Control=1;};
			};

			if (key[KEY_R])
			{
				for (int i=1; i<=iMaxInnerBld; i++)
				{
					arPlayer[Player].arBld[i]=false;					
				};
				for (int j=0; j<LIBRARY_SIZE; j++)
				{
					arPlayer[Player].arLibrary[j]=0;
				};
				for (int j=0; j<MAX_RECRUIT; j++)
				{
					arPlayer[Player].arRecruit[j]=0;
				};
				for (int i=0; i<MAX_GROUP; i++)
				{
					for (int j=0; j<MAX_GROUP_BUILD; j++)
					{
						arPlayer[Player].arGroup[i][j]=0;
					};
				};
				for (int i=0; i<SHOP_SIZE; i++)
				{
					arPlayer[Player].Shop.Items[i][0]=0;
					arPlayer[Player].Shop.Items[i][1]=0;
				};
				arPlayer[Player].bEnter=true;
			};

			if ((key[KEY_U])&&(Condition==1))
			{		
				ClearRoll();
				int Count=0;				
				for (int j=1; j<=iMaxEvent; j++)
				{
					int Poss=(pFirstProvince+Member)->GetEventPoss(j);
					if (*(arPlayer[Player].pStory+j)>=0)
					{Poss=0;};
					if (Poss>0)
					{
						RollerN[Count]=j;
						RollerP[Count]=Poss;
						Count++;
					};					
				};			
				if (Count>0)
				{
					int EVE=Roll(false);
					(pFirstProvince+Member)->StaticEvent(EVE,Player);
				};
				Condition=0;			
			};

			if (key[KEY_B])
			{
				for (int i=1; i<=iMaxInnerBld; i++)
				{
					if ((pFirstInnerBld+i)->iGroup==0)
					{
						arPlayer[Player].arBld[i]=true;
					};
				};
			};
			if (key[KEY_A])
			{
				for (int i=1; i<=iMaxRitual; i++)
				{
					*(arPlayer[Player].pRitual+i)+=Random(3)+1;					
				};
				for (int i=1; i<=iMaxOuterBld; i++)
				{
					*(arPlayer[Player].pOuterBld+i)+=Random(3)+1;					
				};
			};
			if (key[KEY_G])
			{
				for (int i=1; i<=iMaxDefender; i++)
				{
					*(arPlayer[Player].pDefender+i)+=Random(3)+1;					
				};
			};

			if (key[KEY_V])
			{
				iExitFlag=2;
				if (Tutorial.Map)
				{iExitFlag=4;};
				bExitGame=true;
				if (CurGame.iGameType==2)
				{
					if (!Tutorial.Map)
					{WindowCampaignVictory();};
				}
				else
				{ScreenEndGame(Player,true,true,true);};
			};
			if (key[KEY_Z])
			{
				iExitFlag=5;				
				bExitGame=true;				
			};
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
				objTutorial->iLayer=-1;
				txtTutorial.Hide();
				bTutFree=true;
				if (Tutorial.Build==3)
				{
					Tutorial.Build=4;					
					bTutHint=true;
					ScrollSound();
					objTutorial->iLayer=6;
					txtTutorial.Activate((pFirstTutText+57)->msg);
					bTutFree=false;
				};
				if (Tutorial.Move==22)
				{
					Tutorial.Move=23;			
					txtTutorial.Activate((pFirstTutText+60)->msg);
					objTutorial->iLayer=6;
					bTutHint=true;
					ScrollSound();
					bTutFree=false;
					Tutorial.buttExplore=1;
					if (Condition==21)
					{Condition=20;};
				};
				if (Tutorial.Population==2)
				{
					Tutorial.Population=3;					
					bTutHint=true;
					ScrollSound();
					bTutFree=false;
					objTutorial->iLayer=6;
					txtTutorial.Activate((pFirstTutText+81)->msg);
				};
			};
		};

		if (Tutorial.Map)
		{
			if (Tutorial.buttEnd==0)
			{buttEnd.Activate(-1);};
			if (Tutorial.buttCastle==0)
			{
				buttCastle.Activate(-1);
				buttGarrison.Activate(-1);
			}; 
			if (Tutorial.buttStat==0)
			{buttStat.Activate(-1);};
			if (Tutorial.buttDiplo==0)
			{buttDipl.Activate(-1);};
			if (Tutorial.buttNext==0)
			{buttNextHero.Activate(-1);};
			if (Tutorial.buttBuild==0)
			{buttBuild.Activate(-1);};
			if (Tutorial.buttGuard==0)
			{buttGuard.Activate(-1);};
			if (Tutorial.buttEnter==0)
			{buttEnter.Activate(-1);};
			if (Tutorial.buttRitual==0)
			{buttRitual.Activate(-1);};
			if (Tutorial.buttRitHero==0)
			{buttHeroRitual.Activate(-1);};
			if (Tutorial.buttShop==0)
			{buttShop.Activate(-1);};
			if (Tutorial.buttLibrary==0)
			{buttLibrary.Activate(-1);};
			if (Tutorial.buttGarrison==0)
			{buttGarrison.Activate(-1);};
			if (Tutorial.buttPillage==0)
			{buttPillage.Activate(-1);};
			if (Tutorial.buttAttack==0)
			{buttAttack.Activate(-1);};
			if (Tutorial.buttExplore==0)
			{buttExplore.Activate(-1);};
		};

		
		

		if (!Tutorial.Mode)
		{objTutorial->iLayer=-1;};
//		bLogger=false;
		if (Timer)
		{
			/*if ((CurGame.iCurrentTurn==0)&&(Iteration<1000)&&(Iteration%10==0))
			{
				sprintf(stLog,"Step: 7, Inc: %d, bFirst: %d",iIncomeY,bFirst);
				FileLog(stLog);
			};*/
			try{
			bIdle=true;
			bTutor=true;
			if (objIncome)
			{
				if (iIncTimer>0)
				{iIncTimer--;}
				else
				{
					if (iIncomeY>0)
					{
						iIncomeY--;
						objIncome->y=768-iIncomeY;
						bIdle=false;
					};
				};
			};
			poll_keyboard();
			/*if ((CurGame.iCurrentTurn==0)&&(Iteration<1000)&&(Iteration%10==0))
			{
				sprintf(stLog,"Step: 8, Inc: %d, bFirst: %d",iIncomeY,bFirst);
				FileLog(stLog);
			};*/			
			if (((mouse_x==0)||(key[KEY_LEFT]))&&(MX>0))
			{
				MX-=arHumans[arPlayer[Player].iColor].iScrollSpeed;				
				if (MX<0)
				{MX=0;}
				else
				{bIdle=false;};
			};
			if (((mouse_y==0)||(key[KEY_UP]))&&(MY>0))
			{
				MY-=arHumans[arPlayer[Player].iColor].iScrollSpeed;				
				if (MY<0)
				{MY=0;}
				else
				{bIdle=false;};
			};
			if (((mouse_x==1023)||(key[KEY_RIGHT]))&&(MX<bmpMap->w-724))
			{
				MX+=arHumans[arPlayer[Player].iColor].iScrollSpeed;				
				if (MX>bmpMap->w-724)
				{MX=bmpMap->w-724;}
				else
				{bIdle=false;};
			};
			if (((mouse_y==767)||(key[KEY_DOWN]))&&(MY<bmpMap->h-741))
			{
				MY+=arHumans[arPlayer[Player].iColor].iScrollSpeed;
				if (MY>bmpMap->h-741)
				{MY=bmpMap->h-741;}
				else
				{bIdle=false;};
			};
			if (objAnvil2->iLayer>0)
			{
			//	bIdle=false;
				Anvil+=AddAnvil;
				if (Anvil<=0)
				{
					AddAnvil=10;
					Anvil=1;
				};
				if (Anvil>=300)
				{AddAnvil=-10;};
				objAnvil2->iOpacity=Anvil;
				if (objAnvil2->iOpacity>255)
				{objAnvil2->iOpacity=255;};
			};

			if (objMapSelect->iLayer>0)
			{
				objMapSelect->iOpacity+=objMapSelect->iDelay;
				if ((objMapSelect->iOpacity<=10)||(objMapSelect->iOpacity>=80))
				{objMapSelect->iDelay=-objMapSelect->iDelay;};
			};
			/*if ((CurGame.iCurrentTurn==0)&&(Iteration<1000)&&(Iteration%10==0))
			{
				sprintf(stLog,"Step: 9, Inc: %d, bFirst: %d",iIncomeY,bFirst);
				FileLog(stLog);
			};	*/
		//	objHeroSelect->Pic=NULL;
			Timer=false;
			try
			{objMain=ShowActiveObjects(objMain,10,true,MX,MY);}
			catch(...)
			{
			//	if (bGlobalError)
				{
					FileLog("Internal error1!");
					allegro_message("Internal error1!");					
					bGlobalError=false;
				};
			};
			/*if ((CurGame.iCurrentTurn==0)&&(Iteration<1000)&&(Iteration%10==0))
			{
				sprintf(stLog,"Step: 10, Inc: %d, bFirst: %d",iIncomeY,bFirst);
				FileLog(stLog);
			};*/
//			Iteration++;
//			bLogger=true;
			}
			catch(...)
			{
				FileLog("Internal error2!");
				allegro_message("Internal error2!");				
				bGlobalError=false;
			};
		}
		else
		{
			if ((bIdle)&&(!bAnimation))
			{Sleeper(24);}
		};
		}
		catch(...)
		{
			FileLog("MainProcess Error");
			if (bGlobalError)
			{
				allegro_message("MainProcess Error!");
				bGlobalError=false;
			};
		};
	};
	if (GoPath)
	{
		P1=GoPath;
		while (P1)
		{
			P1=GoPath->pPrev;
			delete GoPath;
			GoPath=P1;
		};
		GoPath=NULL;
	};

	G_Object * Temp;
	while (objMain)
	{
		Temp=objMain->pNext;		
		delete objMain;
		objMain=Temp;
	};
	arPlayer[Player].iCondition=Condition;
	arPlayer[Player].iMember=Member;
	delete [] Prov;	

	}; //Redraw
	if ((Tutorial.Build==3)||(Tutorial.Build==4))
	{
		Tutorial.Move=21;
		Tutorial.Build=5;
	};
	if ((Tutorial.Map)&&(Tutorial.Build>0))
	{Tutorial.Turn++;};
	destroy_bitmap(bmpInfoBar);
	destroy_bitmap(bmpLowPanel);
	if (bmpIncome)
	{destroy_bitmap(bmpIncome);};
	bGlobalError=false;	
	iAntisleep=iAntisleepDelay;

//	WriteKeys();
};

void EndMainTurn()
{
	srand(CurGame.iSeed+CurGame.iCurrentTurn);

//	StopLog("Нейтральный игрок",0);

	dcRitual *RR=pRitual;
	while (RR)
	{		
		if (RR->Timer<=1)
		{
			if (RR==pRitual)
			{
				pRitual=RR->pNext;
				delete RR;
				RR=pRitual;
			}
			else
			{
				dcRitual *RR1=pRitual;
				while (RR1->pNext!=RR)
				{RR1=RR1->pNext;};
				RR1->pNext=RR->pNext;
				delete RR;
				RR=RR1->pNext;
			};
		}
		else
		{
			RR->Timer--;
			RR=RR->pNext;
		};
	};
	for (int i=1; i<=MapSize; i++)
	{
		dcProvince *PR=(pFirstProvince+i);
//		FileLog(PR->sName);
		PR->RepairWalls();
		if (PR->ipPlayer==0)
		{
			dcRitual *RR=pRitual;
			while (RR)
			{
				if ((RR->ipProvince==i)&&(RR->iEvent>0))
				{
					RR->iEvent=0;
					if ((pFirstRitual+RR->Index)->iTime==0)
					{RR->Timer=0;};
					PR->StaticEvent((pFirstRitual+RR->Index)->ipEvent,0);
				};
				RR=RR->pNext;
			};
		};
		for (int j=1; j<MAX_PLAYERS; j++)
		{
			if (PR->arAttitude[j][0]!=0)
			{				
				PR->arAttitude[j][1]--;
				if (PR->arAttitude[j][1]==0)
				{
					PR->arAttitude[j][0]-=PR->arAttitude[j][0]/abs(PR->arAttitude[j][0]);
					PR->arAttitude[j][1]=ATTITUDE_TIME;
				};
			};
		};
		for (int j=0; j<MAX_PROVINCE_SITE; j++)
		{
			if (PR->arSiteGuard[j])
			{
				PR->arSiteGuard[j]->RestoreUnits(true,0,true);				
			};
		};
	//	FileLog("Guard");
		if ((PR->ipPlayer==0)&&(PR->pGuard))
		{
			PR->pGuard->RestoreUnits(true,0,true);
		};
	};
	

//	StopLog("Остальные игроки",0);
	for (int Player=1; Player<=CurGame.iMaxPlayers; Player++)
	{
		if (arPlayer[Player].bInGame)
		{
		
		//Появление наёмников
		arPlayer[Player].iMercTime++;
		if (arPlayer[Player].iMercTime==MERCENARY_TIME)
		{
			arPlayer[Player].AddMercenary();
			arPlayer[Player].iMercTime=0;
		}
		else
		{
			if ((arPlayer[Player].arRecruit[11]==0)&&(arPlayer[Player].GetBldParam1(24)>0)&&(arPlayer[Player].iMercTime>MERCENARY_TIME/2))
			{
				arPlayer[Player].AddMercenary();
				arPlayer[Player].iMercTime=0;
			};
		};

		//Изменение отношений в зависимости от текущих договоров
		if (CurGame.iCurrentTurn%10==0)
		{
			for (int j=1; j<=CurGame.iMaxPlayers; j++)
			{
				if ((Player!=j)&&(arPlayer[j].bInGame))
				{
					if (arPlayer[Player].Politic[j].Treaty==1)
					{arPlayer[Player].Politic[j].Attitude--;};
					if (arPlayer[Player].Politic[j].Treaty==3)
					{arPlayer[Player].Politic[j].Attitude++;};
					if (arPlayer[Player].Politic[j].Trade.bTrade)
					{arPlayer[Player].Politic[j].Attitude++;};	
				};
			};
		};

		//Уменьшение задержек ритуалов
		for (int i=0; i<=iMaxRitual; i++)
		{
			if (*(arPlayer[Player].pRitualCD+i)>0)
			{*(arPlayer[Player].pRitualCD+i)-=1;};
		};

		for (int iHero=0; iHero<MAX_HERO; iHero++)
		{	
			dcHero * Hero=arPlayer[Player].arHero[iHero];
			if ((Hero)&&(Hero->iDead<0))
			{
				int iComes=0, iExplore=0, iBonus=0, iExpl, Site;
				dcEvent *pEvent;
				bool bSearch=true;
				//Проверка снежного бурана
				bool bBlizz=false;
				if (Hero->iTask!=0)
				{
					dcRitual *RR=pRitual;					
					while (RR)
					{
						if ((RR->ipProvince==Hero->pProvince->iNumber)&&(RR->iEvent>0))
						{
							for (int i=0; i<MAX_RITUAL_EFFECT; i++)
							{
								if ((pFirstRitual+RR->Index)->arEffect[i][0]==8)
								{bBlizz=true;};
							};
						};
						RR=RR->pNext;
					};
				};
				if (bBlizz)
				{
					pEvent=new dcEvent(20,&arPlayer[Player].pEvent,503,Player,0,arPlayer[Player].arHero[iHero],Hero->pProvince->pGuard,arPlayer[Player].arHero[iHero]->pProvince,Hero->pProvince,arPlayer[Player].pEvent);					
					sprintf(pEvent->sMessage,(pFirstDialog+503)->sText,Hero->pProvince->sName,arPlayer[Player].arHero[iHero]->pstName->str);
				}
				else
				{
				switch (Hero->iTask)
				{
			//Движение героя
				case 1:			
					ClearMoveMap();
					MovePlayer=Player;
					WalkType=1+arPlayer[Player].arHero[iHero]->CheckSkill(26);			
					WalkSpeed=arPlayer[Player].arHero[iHero]->GetMobility();
					MoveMap(arPlayer[Player].arHero[iHero]->pProvince->iNumber,-1,WalkSpeed,0,0);
					PathKicker=0;
					MovePath(arPlayer[Player].arHero[iHero]->pProvince->iNumber,1,NULL,arPlayer[Player].arHero[iHero]->iTarget,WalkSpeed,0,0,0);
					if (GoPath==NULL)
					{
						PathKicker=1;
						MovePath(arPlayer[Player].arHero[iHero]->pProvince->iNumber,1,NULL,arPlayer[Player].arHero[iHero]->iTarget,WalkSpeed,0,0,0);
					};
					if (GoPath)
					{
						P1=GoPath;
						while (P1)
						{
							if ((P1->y==1)&&((pFirstProvince+P1->x)->ipTerrain>0)&&((pFirstProvince+P1->x)->ipTerrain<7))
							{iComes=P1->x;};
							P1=P1->pPrev;
						};						
						dcProvince *PR=(pFirstProvince+iComes);
						if ((PR->ipPlayer==Player)&&(PR->GetSiegeIndex()<0))
						{
							arPlayer[Player].arHero[iHero]->pProvince=PR;
							arPlayer[Player].arHero[iHero]->ipProvince=PR->iNumber;
							if (arPlayer[Player].arHero[iHero]->iTarget==iComes)
							{
								arPlayer[Player].arHero[iHero]->iTask=0;
								arPlayer[Player].arHero[iHero]->iTarget=0;
							};
						}
						else
						{
							int iFrom=0;						
							arPlayer[Player].arHero[iHero]->iTask=0;
							arPlayer[Player].arHero[iHero]->iTarget=0;
							P1=GoPath;
							while (P1)
							{
								if ((P1->y==1)&&((pFirstProvince+P1->x)->ipTerrain>0)&&((pFirstProvince+P1->x)->ipTerrain<7)&&((pFirstProvince+P1->x)->ipPlayer==Player)&&(((pFirstProvince+P1->x)->GetSiegeIndex()<0)||((pFirstProvince+P1->x)->GetSiegeIndex()==Player)))
								{iFrom=P1->x;};
								P1=P1->pPrev;
							};
							if (iFrom>0)
							{
								arPlayer[Player].arHero[iHero]->pProvince=(pFirstProvince+iFrom);
								arPlayer[Player].arHero[iHero]->ipProvince=iFrom;
							};
							pEvent=new dcEvent(1,&arPlayer[Player].pEvent,0,Player,PR->ipPlayer,arPlayer[Player].arHero[iHero],NULL,arPlayer[Player].arHero[iHero]->pProvince,PR,arPlayer[Player].pEvent);
						};
					};
					if (GoPath)
					{
						P1=GoPath;
						while (P1)
						{
							P1=GoPath->pPrev;
							delete GoPath;
							GoPath=P1;
						};
						GoPath=NULL;						
					};
					break;
//Исследование провинции
				case 2:
					if (Hero->pProvince->iExplore<100)
					{
					if (Hero->iLevel<UNIT_LEVEL_CAP)
					{Hero->iExp+=EXPLORE_EXP+Hero->iLevel/2;}
					long int Expl=(15+Hero->CheckHeroBonus(996))*(100+arPlayer[Player].GetBldParam1(25))/100;
					iExplore=1+Expl*20/(20+Hero->pProvince->iExplore)*(pFirstTerrain+Hero->pProvince->ipTerrain)->iExploreMod/100;
					for (int i=1; i<=iMaxInnerBld; i++)
					{
						if (arPlayer[Player].arBld[i])
						{
							for (int j=0; j<MAX_BUILDING_ABILITY; j++)
							{
								if ((pFirstInnerBld+i)->arAbility[j][0]==25)
								{
									iBonus+=(pFirstInnerBld+i)->arAbility[j][1];
								};
							};
						};
					};
					iExplore+=iExplore*iBonus/100;
					iExpl=iExplore;
					if (iExpl+Hero->pProvince->iExplore>100)
					{iExpl=100-Hero->pProvince->iExplore;};
					while ((bSearch)&&(iExplore>0))
					{						
						Hero->pProvince->iExplore++;
						arPlayer[Player].iExplore++;
						for (int i=0; i<MAX_PROVINCE_SITE; i++)
						{
							if (Hero->pProvince->arSite[i][1]==Hero->pProvince->iExplore)
							{
								bSearch=false;
								Site=Hero->pProvince->arSite[i][0];
								//Найден охраняемый сайт
								if (Hero->pProvince->arSiteGuard[i])
								{
									dcEvent *pEvent;
									if ((pFirstSite+Site)->arAbility[0][0]==21)
									{
										pEvent=new dcEvent(3,&arPlayer[Player].pEvent,(pFirstSite+Site)->arAbility[0][1],Player,0,Hero,NULL,Hero->pProvince,NULL,arPlayer[Player].pEvent);									
										sprintf(pEvent->sMessage,(pFirstDialog+(pFirstSite+Site)->arAbility[0][1])->sText,Hero->pProvince->sName,Hero->pstName->str,(pFirstSite+Site)->sName);										
									}
									else
									{
										pEvent=new dcEvent(3,&arPlayer[Player].pEvent,22,Player,0,Hero,NULL,Hero->pProvince,NULL,arPlayer[Player].pEvent);									
										sprintf(pEvent->sMessage,(pFirstDialog+22)->sText,Hero->pProvince->sName,Hero->pstName->str,(pFirstSite+Site)->sName);										
									};
									dcEvent *pTreeEvent=new dcEvent((pFirstSite+Site)->ipEvent,NULL,(pFirstSite+Site)->ipDialog,Player,0,Hero,Hero->pProvince->arSiteGuard[i],Hero->pProvince,NULL,NULL);
									pEvent->pTree=pTreeEvent;
									pTreeEvent->iParam1=i;									
									char txtSquad [500];
									int MaxGuards=Hero->pProvince->arSiteGuard[i]->MakeDescription(txtSquad);									
									sprintf(pTreeEvent->sMessage,(pFirstDialog+(pFirstSite+Site)->ipDialog)->sText,Hero->pstName->str,MaxGuards,txtSquad);
								}
								else
								//Сайт без охраны
								{
									if (((pFirstSite+Site)->iGuarded>0)||((pFirstSite+Site)->bCanEnter))
									{
										dcEvent *pEvent=new dcEvent(3,&arPlayer[Player].pEvent,22,Player,0,Hero,NULL,Hero->pProvince,NULL,arPlayer[Player].pEvent);
										//arPlayer[Player].pEvent=pEvent;
										sprintf(pEvent->sMessage,(pFirstDialog+22)->sText,Hero->pProvince->sName,Hero->pstName->str,(pFirstSite+Site)->sName);
										dcEvent *pTreeEvent;
											if ((pFirstSite+Site)->bCanEnter)
											{pTreeEvent=new dcEvent(2,NULL,(pFirstSite+Site)->ipDialog2,Player,0,Hero,NULL,Hero->pProvince,NULL,NULL);}
											else
											{pTreeEvent=new dcEvent(4,NULL,(pFirstSite+Site)->ipDialog2,Player,0,Hero,NULL,Hero->pProvince,NULL,NULL);};
										pEvent->pTree=pTreeEvent;
										pTreeEvent->iParam1=i;
										sprintf(pTreeEvent->sMessage,(pFirstDialog+(pFirstSite+Site)->ipDialog2)->sText,Hero->pstName->str);
									}
									else
									{
								//Никогда не охраняемый сайт
										pEvent=new dcEvent((pFirstSite+Site)->ipEvent,&arPlayer[Player].pEvent,(pFirstSite+Site)->ipDialog,Player,0,Hero,NULL,Hero->pProvince,NULL,arPlayer[Player].pEvent);										
									//	arPlayer[Player].pEvent=pEvent;
										pEvent->iParam1=i;
									};
								};
							};
						};
						iExplore--;
					};
					if (bSearch)
					{					
						if (Hero->pProvince->iExplore<100)
						{
							pEvent=new dcEvent(3,&arPlayer[Player].pEvent,21,Player,0,Hero,NULL,Hero->pProvince,NULL,arPlayer[Player].pEvent);
							sprintf(pEvent->sMessage,(pFirstDialog+21)->sText,Hero->pstName->str,iExpl,Hero->pProvince->sName);
						}
						else
						{
							pEvent=new dcEvent(3,&arPlayer[Player].pEvent,26,Player,0,Hero,NULL,Hero->pProvince,NULL,arPlayer[Player].pEvent);
							sprintf(pEvent->sMessage,(pFirstDialog+26)->sText,Hero->pstName->str,Hero->pProvince->sName);
						};
					//	arPlayer[Player].pEvent=pEvent;
						pEvent->pTree=NULL;
					};
					};
					if (Hero->pProvince->iExplore>=100)
					{
						Hero->pProvince->iExplore=100;
						Hero->iTask=0;
					};
					break;
//Разграбление провинции
				case 3:
					if (Hero->pProvince->iPopulation>0)
					{
						int iPower=Hero->iHealth+Hero->CheckHeroBonus(50);
						for (int i=1; i<MAX_SQUAD; i++)
						{
							if (Hero->pSquad[i])
							{
								iPower+=Hero->pSquad[i]->pType()->iLevel;
								if (Hero->pSquad[i]->CheckAbility(58)+Hero->pSquad[i]->CheckUpg(58)>0)
								{
									iPower++;
								};
								if (Hero->pSquad[i]->CheckAbility(50)+Hero->pSquad[i]->CheckUpg(50)>0)
								{
									iPower++;
								};
							};
						};
						int iKilled=5+iPower+Hero->pProvince->iPopulation*(iPower+1)/100;
						if (iKilled>Hero->pProvince->iPopulation)
						{
							iKilled=Hero->pProvince->iPopulation;
						};
						Hero->pProvince->DrawPic(false);
						int iLoot=(iKilled+Random(iKilled))/7+20+Random(50);
						iLoot+=iLoot*Hero->CheckHeroBonus(982)/100;
						iLoot+=iLoot*arPlayer[Player].GetBldParam1(41)/100;
						arPlayer[Player].iKarma-=5;
						arPlayer[Player].Gold+=iLoot;
						arPlayer[Player].iPillage+=iLoot;
						Hero->pProvince->iPopulation-=iKilled;
						Hero->iExp+=GetMin(1+iKilled/40,10+Hero->iLevel);
						if (Hero->pProvince->arAttitude[Player][0]>-10)
						{Hero->pProvince->ChangeAttitude(Player,-GetMin(3,1+iKilled/200));};
						if (Hero->pProvince->arAttitude[Player][0]<-10)
						{
							Hero->pProvince->arAttitude[Player][0]=-10;
						};						
						if (Hero->pProvince->iPopulation>0)
						{
							pEvent=new dcEvent(6,&arPlayer[Player].pEvent,63,Player,0,Hero,NULL,Hero->pProvince,NULL,arPlayer[Player].pEvent);
							sprintf(pEvent->sMessage,(pFirstDialog+63)->sText,Hero->pstName->str,Hero->pProvince->sName,iLoot);
						}
						else
						{
							pEvent=new dcEvent(7,&arPlayer[Player].pEvent,64,Player,0,Hero,NULL,Hero->pProvince,NULL,arPlayer[Player].pEvent);
							sprintf(pEvent->sMessage,(pFirstDialog+64)->sText,Hero->pProvince->sName,iLoot,Hero->pstName->str);
						};
						pEvent->iParam1=iLoot;
					//	arPlayer[Player].pEvent=pEvent;
					}
					else
					{
						Hero->iTask=0;
					};										
					break;
//Вход в особое место
				case 4:
					Site=Hero->pProvince->arSite[Hero->iTarget][0];
				//	textprintf(screen,Font1,100,100,COLOR_2,"Сайт: %d (%s), Герой: %s, Провинция: %s",Site, (pFirstSite+Hero->pProvince->arSite[Site][0])->sName, Hero->pstName->str, Hero->pProvince->s);
					Hero->iTask=0;
					pEvent=new dcEvent((pFirstSite+Site)->ipEvent,&arPlayer[Player].pEvent,(pFirstSite+Site)->ipDialog,Player,0,Hero,Hero->pProvince->arSiteGuard[Hero->iTarget],Hero->pProvince,NULL,arPlayer[Player].pEvent);
				//	arPlayer[Player].pEvent=pEvent;
					pEvent->iParam1=Hero->iTarget;					
					if (Hero->pProvince->arSiteGuard[Hero->iTarget])
					{
				//		textprintf(screen,Font1,100,120,COLOR_2,"Охрана есть");
						char txtSquad [500];
						int MaxGuards=Hero->pProvince->arSiteGuard[Hero->iTarget]->MakeDescription(txtSquad);
						sprintf(pEvent->sMessage,(pFirstDialog+(pFirstSite+Site)->ipDialog)->sText,Hero->pstName->str,MaxGuards,txtSquad);
					};
					break;

//Атаковать стража
				case 5:				
					Hero->iTask=0;
					pEvent=new dcEvent(5,&arPlayer[Player].pEvent,32,Player,0,Hero,Hero->pProvince->pGuard,Hero->pProvince,NULL,arPlayer[Player].pEvent);
				//	arPlayer[Player].pEvent=pEvent;
				//	Hero->pProvince->pGuard->ipPlayer=0;
					sprintf(pEvent->sMessage,(pFirstDialog+32)->sText,Hero->pstName->str,Hero->pProvince->sName);
					break;
//Осада
				default:
					if ((Hero->pProvince->ipPlayer!=Player)&&(Hero->pProvince->pGarrison)&&((Tutorial.Map==false)||(Tutorial.buttCastle>0)))
					{	
						bool bGuard=false;
						for (int i=0; i<MAX_SQUAD; i++)
						{
							if (Hero->pProvince->pGarrison->pSquad[i])
							{bGuard=true;};
						};
						if (bGuard)
						{
						int Time=0, SiegeForce=0;
						for (int HR=0; HR<MAX_HERO; HR++)
						{
							if ((arPlayer[Player].arHero[HR])&&(arPlayer[Player].arHero[HR]->iDead<0)&&(arPlayer[Player].arHero[HR]->ipProvince==Hero->ipProvince))
							{SiegeForce+=arPlayer[Player].arHero[HR]->GetSiege();};
						};
						Hero->pProvince->iFort-=Hero->GetSiege();
						int Fort;
						if (arPlayer[Hero->pProvince->ipPlayer].ipProvince==Hero->pProvince->iNumber)
						{
							Fort=100*Hero->pProvince->iFort/arPlayer[Hero->pProvince->ipPlayer].iWall;
							if (Fort>50)
							{
								Time=(Hero->pProvince->iFort-arPlayer[Hero->pProvince->ipPlayer].iWall/2)/SiegeForce;
								if ((Hero->pProvince->iFort-arPlayer[Hero->pProvince->ipPlayer].iWall/2)%SiegeForce>0)
								{Time++;};
							};
						}
						else
						{
							int BLD=-1;
							for (int i=0; i<MAX_PROVINCE_BLD; i++)
							{
								if ((Hero->pProvince->arBuilding[i]>0)&&(pFirstOuterBld+Hero->pProvince->arBuilding[i])->iWall>0)
								{
									BLD=Hero->pProvince->arBuilding[i];
								};
							};
							if (BLD>=0)
							{
								Fort=100*Hero->pProvince->iFort/(pFirstOuterBld+BLD)->iWall;
								Time=(Hero->pProvince->iFort-(pFirstOuterBld+BLD)->iWall/2)/SiegeForce;
								if ((Hero->pProvince->iFort-(pFirstOuterBld+BLD)->iWall/2)%SiegeForce>0)
								{Time++;};
							}
							else
							{
								StopLog("НЕ НАЙДЕНЫ УКРЕПЛЕНИЯ В ПРОВИНЦИИ ",Hero->ipProvince);
								Fort=0;
							};
						};
						if (Fort<=0)
						{
							Hero->pProvince->iFort=0;
							if ((arPlayer[Player].Control!=1)&&(arPlayer[Hero->pProvince->ipPlayer].Control==1))
							{pEvent=new dcEvent(11,&arPlayer[Hero->pProvince->ipPlayer].pEvent,378,Player,Hero->pProvince->ipPlayer,Hero,Hero->pProvince->pGarrison,Hero->pProvince,NULL,arPlayer[Hero->pProvince->ipPlayer].pEvent);}
							else
							{pEvent=new dcEvent(11,&arPlayer[Player].pEvent,378,Player,Hero->pProvince->ipPlayer,Hero,Hero->pProvince->pGarrison,Hero->pProvince,NULL,arPlayer[Player].pEvent);};
							sprintf(pEvent->sMessage,(pFirstDialog+378)->sText,Hero->pstName->str,Hero->pProvince->sName);
						}
						else
						{
							if (Fort<=50)
							{
								if ((arPlayer[Player].Control!=1)&&(arPlayer[Hero->pProvince->ipPlayer].Control==1))
								{pEvent=new dcEvent(12,&arPlayer[Hero->pProvince->ipPlayer].pEvent,377,Player,Hero->pProvince->ipPlayer,Hero,Hero->pProvince->pGarrison,Hero->pProvince,NULL,arPlayer[Hero->pProvince->ipPlayer].pEvent);}
								else
								{pEvent=new dcEvent(12,&arPlayer[Player].pEvent,377,Player,Hero->pProvince->ipPlayer,Hero,Hero->pProvince->pGarrison,Hero->pProvince,NULL,arPlayer[Player].pEvent);};
								sprintf(pEvent->sMessage,(pFirstDialog+377)->sText,Hero->pstName->str,Hero->pProvince->sName);
							}
							else
							{
								if (arPlayer[Player].Control==1)
								{
									dcEvent *EV=arPlayer[Player].pEvent;
									pEvent=NULL;
									while (EV)
									{
										if ((EV->Index==13)&&(EV->Prov1==Hero->pProvince))
										{pEvent=EV;};
										EV=EV->pNext;
									};
									if (!pEvent)
									{pEvent=new dcEvent(13,&arPlayer[Player].pEvent,376,Player,Hero->pProvince->ipPlayer,Hero,Hero->pProvince->pGarrison,Hero->pProvince,NULL,arPlayer[Player].pEvent);};
									sprintf(pEvent->sMessage,(pFirstDialog+376)->sText,Hero->pstName->str,Hero->pProvince->sName,Time);
								}
								if (arPlayer[Hero->pProvince->ipPlayer].Control==1)
								{
									dcEvent *EV=arPlayer[Hero->pProvince->ipPlayer].pEvent;
									pEvent=NULL;
									while (EV)
									{
										if ((EV->Index==13)&&(EV->Prov1==Hero->pProvince))
										{pEvent=EV;};
										EV=EV->pNext;
									};
									if (!pEvent)
									{pEvent=new dcEvent(13,&arPlayer[Hero->pProvince->ipPlayer].pEvent,379,Player,Hero->pProvince->ipPlayer,Hero,Hero->pProvince->pGarrison,Hero->pProvince,NULL,arPlayer[Hero->pProvince->ipPlayer].pEvent);};
									sprintf(pEvent->sMessage,(pFirstDialog+379)->sText,Hero->pProvince->sName,Time);
								};
							};
						};
						}
						else
						{
							//Сдача крепости без гарнизона
							dcEvent *EV=arPlayer[Player].pEvent;
							bool bNeed=true;
							while (EV)
							{
								if ((EV->Index==17)&&(EV->Prov1==Hero->pProvince))
								{bNeed=false;};
								EV=EV->pNext;
							};
							if (bNeed)
							{
								pEvent=new dcEvent(17,&arPlayer[Player].pEvent,824,Player,Hero->pProvince->ipPlayer,Hero,Hero->pProvince->pGarrison,Hero->pProvince,NULL,arPlayer[Player].pEvent);
								sprintf(pEvent->sMessage,(pFirstDialog+824)->sText,Hero->pstName,Hero->pProvince->sName);
							};
							if (arPlayer[Hero->pProvince->ipPlayer].Control==1)							
							{
								EV=arPlayer[Hero->pProvince->ipPlayer].pEvent;
								bNeed=true;
								while (EV)
								{
									if ((EV->Index==18)&&(EV->Prov1==Hero->pProvince))
									{bNeed=false;};
									EV=EV->pNext;
								};
								if (bNeed)
								{
									pEvent=new dcEvent(18,&arPlayer[Hero->pProvince->ipPlayer].pEvent,825,Player,Hero->pProvince->ipPlayer,Hero,Hero->pProvince->pGarrison,Hero->pProvince,NULL,arPlayer[Hero->pProvince->ipPlayer].pEvent);
									sprintf(pEvent->sMessage,(pFirstDialog+825)->sText,Hero->pProvince->sName,arPlayer[Player].GetName());
								};
							};
						};
					//	arPlayer[Player].pEvent=pEvent;
					};
					break;
				};//switch
				};//bBlizz
				Hero->iMoveBonus=0;
			};
		};
		};
	};
	WriteLog("Ход прошел");
	FileLog(" ");
	char sTurn[100];
	sprintf(sTurn,"**** ХОД: %d ****",CurGame.iCurrentTurn);
	FileLog(sTurn);
};


//***************************************************************************
//**** Обработка команд героям и нейтральных провинций **********************
//***************************************************************************
void EndMainTurnNew()
{
	srand(CurGame.iSeed+CurGame.iCurrentTurn);

//Длительность ритуалов
	dcRitual *RR=pRitual;
	while (RR)
	{		
		if (RR->Timer<=1)
		{
			if (RR==pRitual)
			{
				pRitual=RR->pNext;
				delete RR;
				RR=pRitual;
			}
			else
			{
				dcRitual *RR1=pRitual;
				while (RR1->pNext!=RR)
				{RR1=RR1->pNext;};
				RR1->pNext=RR->pNext;
				delete RR;
				RR=RR1->pNext;
			};
		}
		else
		{
			RR->Timer--;
			RR=RR->pNext;
		};
	};

	for (int i=1; i<=MapSize; i++)
	{
		dcProvince *PR=(pFirstProvince+i);
		PR->RepairWalls();
		//Проведение ритуалов в нейтральных провинциях
		if (PR->ipPlayer==0)
		{
			dcRitual *RR=pRitual;
			while (RR)
			{
				if ((RR->ipProvince==i)&&(RR->iEvent>0))
				{
					RR->iEvent=0;
					PR->StaticEvent((pFirstRitual+RR->Index)->ipEvent,0);
					if ((pFirstRitual+RR->Index)->iTime==0)
					{RR->Timer=0;};
				};
				RR=RR->pNext;
			};
		};
		//Изменение отношений местных жителей
		for (int j=1; j<MAX_PLAYERS; j++)
		{
			if (PR->arAttitude[j][0]!=0)
			{				
				PR->arAttitude[j][1]--;
				if (PR->arAttitude[j][1]==0)
				{
					PR->arAttitude[j][0]-=PR->arAttitude[j][0]/abs(PR->arAttitude[j][0]);
					PR->arAttitude[j][1]=ATTITUDE_TIME;
				};
			};
		};
		//Восстановление стражей
		for (int j=0; j<MAX_PROVINCE_SITE; j++)
		{
			if (PR->arSiteGuard[j])
			{
				PR->arSiteGuard[j]->RestoreUnits(true,0,true);				
			};
		};
	
		if ((PR->ipPlayer==0)&&(PR->pGuard))
		{
			PR->pGuard->RestoreUnits(true,0,true);
		};
	};
	

//	StopLog("Остальные игроки",0);
	for (int Player=1; Player<=CurGame.iMaxPlayers; Player++)
	{
		if (arPlayer[Player].bInGame)
		{
		
		//Появление наёмников
		arPlayer[Player].iMercTime++;
		if (arPlayer[Player].iMercTime==MERCENARY_TIME)
		{
			arPlayer[Player].AddMercenary();
			arPlayer[Player].iMercTime=0;
		}
		else
		{
			if ((arPlayer[Player].arRecruit[11]==0)&&(arPlayer[Player].GetBldParam1(24)>0)&&(arPlayer[Player].iMercTime>MERCENARY_TIME/2))
			{
				arPlayer[Player].AddMercenary();
				arPlayer[Player].iMercTime=0;
			};
		};

		//Изменение отношений в зависимости от текущих договоров
		if (CurGame.iCurrentTurn%10==0)
		{
			for (int j=1; j<=CurGame.iMaxPlayers; j++)
			{
				if ((Player!=j)&&(arPlayer[j].bInGame))
				{
					if (arPlayer[Player].Politic[j].Treaty==1)
					{arPlayer[Player].Politic[j].Attitude--;};
					if (arPlayer[Player].Politic[j].Treaty==3)
					{arPlayer[Player].Politic[j].Attitude++;};
					if (arPlayer[Player].Politic[j].Trade.bTrade)
					{arPlayer[Player].Politic[j].Attitude++;};	
				};
			};
		};

		//Уменьшение задержек ритуалов
		for (int i=0; i<=iMaxRitual; i++)
		{
			if (*(arPlayer[Player].pRitualCD+i)>0)
			{*(arPlayer[Player].pRitualCD+i)-=1;};
		};

		//****** Обработка приказов героям ******

		for (int iHero=0; iHero<MAX_HERO; iHero++)
		{	
			dcHero * Hero=arPlayer[Player].arHero[iHero];
			if ((Hero)&&(Hero->iDead<0))
			{
				int iComes=0, iExplore=0, iBonus=0, iExpl, Site;
				dcEvent *pEvent;
				bool bSearch=true;
				//Проверка снежного бурана
				bool bBlizz=false;
				if (Hero->iTask!=0)
				{
					dcRitual *RR=pRitual;					
					while (RR)
					{
						if ((RR->ipProvince==Hero->pProvince->iNumber)&&(RR->iEvent>0))
						{
							for (int i=0; i<MAX_RITUAL_EFFECT; i++)
							{
								if ((pFirstRitual+RR->Index)->arEffect[i][0]==8)
								{bBlizz=true;};
							};
						};
						RR=RR->pNext;
					};
				};
				if (bBlizz)
				{Hero->ipEvent=20;}
				else
				{
				//Движение героя
				if (Hero->iTask==1)
				{
					ClearMoveMap();
					MovePlayer=Player;
					WalkType=1+arPlayer[Player].arHero[iHero]->CheckSkill(26);			
					WalkSpeed=arPlayer[Player].arHero[iHero]->GetMobility();
					MoveMap(arPlayer[Player].arHero[iHero]->pProvince->iNumber,-1,WalkSpeed,0,0);
					PathKicker=0;
					MovePath(arPlayer[Player].arHero[iHero]->pProvince->iNumber,1,NULL,arPlayer[Player].arHero[iHero]->iTarget,WalkSpeed,0,0,0);
					if (GoPath==NULL)
					{
						PathKicker=1;
						MovePath(arPlayer[Player].arHero[iHero]->pProvince->iNumber,1,NULL,arPlayer[Player].arHero[iHero]->iTarget,WalkSpeed,0,0,0);
					};
					if (GoPath)
					{
						P1=GoPath;
						while (P1)
						{
							if ((P1->y==1)&&((pFirstProvince+P1->x)->ipTerrain>0)&&((pFirstProvince+P1->x)->ipTerrain<7))
							{iComes=P1->x;};
							P1=P1->pPrev;
						};						
						dcProvince *PR=(pFirstProvince+iComes);
						if (((PR->ipPlayer==Player)&&(PR->GetSiegeIndex()<0))||(PR->GetSiegeIndex()==Player))
						{
							Hero->ipEvent=0;
							arPlayer[Player].arHero[iHero]->pProvince=PR;
							arPlayer[Player].arHero[iHero]->ipProvince=PR->iNumber;
							if (arPlayer[Player].arHero[iHero]->iTarget==iComes)
							{
								arPlayer[Player].arHero[iHero]->iTask=0;
								arPlayer[Player].arHero[iHero]->iTarget=0;
							};
						}
						else
						{
							int iFrom=0;
							Hero->ipMoveProv=Hero->ipProvince;
							arPlayer[Player].arHero[iHero]->iTask=0;
							P1=GoPath;
							while (P1)
							{
								if ((P1->y==1)&&((pFirstProvince+P1->x)->ipTerrain>0)&&((pFirstProvince+P1->x)->ipTerrain<7)&&((pFirstProvince+P1->x)->ipPlayer==Player)&&(((pFirstProvince+P1->x)->GetSiegeIndex()<0)||((pFirstProvince+P1->x)->GetSiegeIndex()==Player)))
								{iFrom=P1->x;};
								P1=P1->pPrev;
							};
							if (iFrom>0)
							{
								Hero->ipProvince=iFrom;
								Hero->pProvince=(pFirstProvince+iFrom);
							};
							Hero->ipEvent=1;
						};
					};
					if (GoPath)
					{
						P1=GoPath;
						while (P1)
						{
							P1=GoPath->pPrev;
							delete GoPath;
							GoPath=P1;
						};
						GoPath=NULL;						
					};
				};
				};
				Hero->iMoveBonus=0;
			};
		};
		};
	};
	WriteLog("Ход прошел");
	FileLog(" ");
	char sTurn[100];
	sprintf(sTurn,"**** ХОД: %d ****",CurGame.iCurrentTurn);
	FileLog(sTurn);
};

void Lose(int Player)
{	
	bHourGlass=false;
	if (CurGame.iGameType>2)
	{
		ScreenEndGame(Player,false,true,false);
		CheckPlayerCondition(Player,true);
		iGameMode=2;
		ChangeMusic();
	};
	if (CurGame.iGameType==2)
	{
		iExitFlag=1;
		bExitGame=true;
		ScreenEndGame(Player,false,true,false);
		if (Campaign.GetActiveShard())
		{
			int PL=0, Land=0;
			for (int i=1; i<=CurGame.iMaxPlayers; i++)
			{
				if ((arPlayer[i].bInGame)&&(arPlayer[i].Control>1))
				{
					int Land1=0;
					for (int j=1; j<=MapSize; j++)
					{
						if ((pFirstProvince+j)->ipPlayer==i)
						{Land1++;};
					};
					if (Land1>Land)
					{
						Land=Land1;
						PL=i;
					};
				};
			};
			if ((PL>0)&&(arPlayer[PL].Control==2))
			{
				for (int i=0; i<MAX_SHARD_BUILD; i++)
				{
					if (Campaign.GetActiveShard()->arBuilding[i]>0)
					{Campaign.arCampPlayer[arPlayer[PL].ipProfile].arBld[Campaign.GetActiveShard()->arBuilding[i]]=1;};
				};
			};
		};
	};
	if (CurGame.iGameType==1)
	{
		iExitFlag=1;
		bExitGame=true;
		if (ScreenEndGame(Player,false,true,true))
		{WindowHighScore();};
	};
};

void Victory(int Player)
{
	bHourGlass=false;
	if (CurGame.iGameType>2)
	{		
		ScreenEndGame(Player,true,true,false);
	};
	if (CurGame.iGameType==2)
	{
		if (!Tutorial.Map)
		{WindowCampaignVictory();};
	};
	if (CurGame.iGameType==1)
	{
		(pFirstDialog+375)->Show(true,true,false,false,NULL,NULL);
		if (ScreenEndGame(Player,true,true,true))
		{WindowHighScore();};
	};
	iExitFlag=2;
	bExitGame=true;	
};

//Проверяет условия победы или поражения, возвращает истину, если для игрока игра окончена
int CheckPlayerCondition(int Player, bool bLose)
{
	if (!arPlayer[Player].bInGame)
	{return true;};
	if ((arPlayer[Player].Control==1)&&(Tutorial.Map))
	{return false;};
	bool Result=false;
	if ((bLose)||((pFirstProvince+arPlayer[Player].ipProvince)->ipPlayer!=Player))
	{
		if (arPlayer[Player].bInGame)
		{
			//Расcылка сообщений по сети
		};
		arPlayer[Player].bInGame=false;
		Result=true;
		if ((!Tutorial.Map)&&(!bLose))
		{
		bHourGlass=false;
		OldProgress=0;
		if (arPlayer[Player].Control==1)
		{
			if (CurGame.iGameType>2)
			{(pFirstDialog+374)->Show(true,true,false,false,NULL,NULL);}
			else
			{
				int Answer=(pFirstDialog+374)->Show(true,true,true,false,NULL,NULL);
				if (Answer==3)
				{
//					Exit=1;
					bExitGame=true;	
					iExitFlag=-1;
					return -1;
				};
			};

		}
		else
		{
			if (arPlayer[Player].Control==2)
			{
				char sTxt[500];
				sprintf(sTxt,(pFirstDialog+389)->sText,arPlayer[Player].GetName());
				(pFirstDialog+389)->Show(true,true,false,false,arPlayer[Player].GetAvatar(),sTxt);
			}
			else
			{(pFirstDialog+1192)->Show(true,false,false,false,NULL,NULL);};
		};		
		int Count=0;
		for (int i=1; i<=CurGame.iMaxPlayers; i++)
		{
			if ((arPlayer[i].bInGame)&&(arPlayer[i].Control==1))
			{Count++;};
		};
		if (Count==0)
		{
			Lose(Player);
		};
		};
		//Очистка дипломатии
		for (int i=1; i<=CurGame.iMaxPlayers; i++)
		{
			arPlayer[Player].Politic[i].Trade.Clear();
			arPlayer[i].Politic[Player].Trade.Clear();
			arPlayer[Player].Politic[i].Message.Clear();
			arPlayer[i].Politic[Player].Message.Clear();
		};
		//Преобразование родовой провинции в нейтральную
		if ((pFirstProvince+arPlayer[Player].ipProvince)->pGarrison->arGarrison[3]>0)		
		{(pFirstProvince+arPlayer[Player].ipProvince)->arBuilding[0]=22;}
		else
		{
			if ((pFirstProvince+arPlayer[Player].ipProvince)->pGarrison->arGarrison[2]>0)
			{(pFirstProvince+arPlayer[Player].ipProvince)->arBuilding[0]=21;}
			else
			{
				if ((pFirstProvince+arPlayer[Player].ipProvince)->pGarrison->arGarrison[1]>0)
				{(pFirstProvince+arPlayer[Player].ipProvince)->arBuilding[0]=20;}
				else
				{
			//		if (*(arPlayer[Player].pOuterBld+20)<0)
			//		{(pFirstProvince+arPlayer[Player].ipProvince)->arBuilding[0]=20;}
			//		else
					{(pFirstProvince+arPlayer[Player].ipProvince)->arBuilding[0]=19;};					
				};
			};
		};
		if (*(arPlayer[Player].pOuterBld+7)<0)
		{(pFirstProvince+arPlayer[Player].ipProvince)->arBuilding[1]=7;}
		else
		{
			if (*(arPlayer[Player].pOuterBld+6)<0)
			{(pFirstProvince+arPlayer[Player].ipProvince)->arBuilding[1]=6;}
			else
			{(pFirstProvince+arPlayer[Player].ipProvince)->arBuilding[1]=5;};
				
		};
		if (*(arPlayer[Player].pOuterBld+18)<0)
		{(pFirstProvince+arPlayer[Player].ipProvince)->arBuilding[2]=18;}
		else
		{(pFirstProvince+arPlayer[Player].ipProvince)->arBuilding[2]=17;};
		for (int i=0; i<MAX_PROVINCE_SITE; i++)
		{
			if ((pFirstProvince+arPlayer[Player].ipProvince)->arSite[i][0]==48)
			{(pFirstProvince+arPlayer[Player].ipProvince)->arSite[i][0]=0;};
		};
		arPlayer[Player].iWall=-1000;

		//Преобразование остальных провинций в нейтральные
		for (int i=1; i<=MapSize; i++)
		{
			if ((pFirstProvince+i)->ipPlayer==Player)
			{
			if ((pFirstProvince+i)->GetSiegeIndex()<1)
			{
				(pFirstProvince+i)->MakeNeutral();
			}
			else
			{
				//Осаждённые
				dcProvince *PR=(pFirstProvince+i);
				int NewOwner=PR->GetSiegeIndex();
				PR->ipPlayer=NewOwner;
				if (PR->pGarrison)
				{
					PR->pGarrison->ipPlayer=NewOwner;
					for (int i=0; i<MAX_SQUAD; i++)
					{
						if (PR->pGarrison->pSquad[i])
						{
							PR->pGarrison->pSquad[i]->Dismiss();
							PR->pGarrison->pSquad[i]=NULL;
						};
					};
				};


			};
			};
		};
		while (arPlayer[Player].arHero[0])
		{delete arPlayer[Player].arHero[0];};		
	}
	else
	{
		int Count=0;
		for (int i=1; i<=CurGame.iMaxPlayers; i++)
		{
			if (arPlayer[i].bInGame)
			{Count++;};
		};
		if (Count==1)
		{
			Result=true;
			if (arPlayer[Player].Control==1)
			{
				Victory(Player);
			};
		};
	};
	return Result;
};

G_Object *objEvent;

void RunEvents(int Player, int Index, G_Object *objMain)
{
	dcEvent *EV=arPlayer[Player].pEvent, *EV1;
	int dx;
	while ((!bExitGame)&&(EV))
	{
		if (EV->Index==Index)
		{
			if (arPlayer[Player].Control==1)
			{
				if (EV->Prov2)
				{
					dx=200;
					if (EV->Prov2->x+400>bmpMap->w)
					{dx=-200;};
					CentreMap(EV->Prov2->x+dx,EV->Prov2->y);
					objEvent->PlaceOnMap(EV->Prov2->x,EV->Prov2->y);
					objEvent->iLayer=8;
				}
				else
				{
					dx=200;
					if (EV->Prov1->x+400>bmpMap->w)
					{dx=-200;};
					CentreMap(EV->Prov1->x+dx,EV->Prov1->y);
					objEvent->PlaceOnMap(EV->Prov1->x,EV->Prov1->y);
					objEvent->iLayer=8;
				};
				bHourGlass=false;
				objMain=ShowActiveObjects(objMain,10,false,MX,MY);
			};			
			EV->Call(dx);			
			EV1=EV->pNext;
			delete EV;
			EV=EV1;
		}
		else
		{
			EV=EV->pNext;
		};
	};
};

void Exiter(G_Object *objMain, int *Prov, BITMAP * bmpLowPanel)
{
	delete [] Prov;
	G_Object * Temp;
	while (objMain)
	{
		Temp=objMain->pNext;		
		delete objMain;
		objMain=Temp;
	};
	destroy_bitmap(bmpLowPanel);
};

//Создать карту и вывести сообщение о происшествии в провинции ipProv игроку Player. Mode=0 - обычная карта, Mode=1 - только провинция и окрестности
int ShowEventMessage(int Player, int Mode, int ipProv, int Dialog, int An1, int An2, int An3, int An4, char *Mess, BITMAP *Pic, bool HotSeat)
{
	int Answer=1;
	if (arPlayer[Player].Control==1)
	{
	dcProvince *PR=(pFirstProvince+ipProv);
	int *Prov=new int [MapSize+1];
	const int LowY=768-bmpGUI_LowPanel->h;
	G_Object objEmpty(NULL,0,0,-1,bmpEmpty);	
	G_Object *objMain=NULL;
	BITMAP * bmpLowPanel=create_bitmap(bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);
	int dx;

	if (Mode==0)
	{
		MakeFog(Player);
		CurrentPlayer=Player;
		objMain=new G_Object(NULL,0,LowY,5,bmpLowPanel);
		objMain=new G_Object(objMain,0,0,-1,bmpQuest[0]);
		objEvent=objMain;
		objMain=ShowMap(Player,objMain,Prov,&objEmpty);
		blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
		textprintf_centre_ex(bmpLowPanel,Font1,156,79,COLOR_2,-1,"%d",arPlayer[Player].Gold);
		textprintf_centre_ex(bmpLowPanel,Font1,280,79,COLOR_2,-1,"%d",arPlayer[Player].Gem);
		if (PR->x+400>bmpMap->w)
		{dx=-200;};
		CentreMap(PR->x+dx,PR->y);
		objEvent->PlaceOnMap(PR->x,PR->y);
		objEvent->iLayer=8;
		objMain=ShowActiveObjects(objMain,10,false,MX,MY);
	}

	if (Mode==1)
	{
		if ((!CurGame.bFog)||(bFogCheat))
		{
			for (int i=1; i<=MapSize; i++)
			{
				*(arPlayer[0].pFog+i)=0;			
			};
		}
		else
		{	
			for (int i=1; i<=MapSize; i++)
			{		
				*(arPlayer[0].pFog+i)=2;
			};
			*(arPlayer[0].pFog+ipProv)=0;
			for (int i=0; i<MAX_NEIGHBOUR; i++)
			{
				if (PR->arNeighbour[i]>0)
				{
					*(arPlayer[0].pFog+PR->arNeighbour[i])=0;
					for (int j=0; j<MAX_NEIGHBOUR; j++)
					{
						if (((pFirstProvince+PR->arNeighbour[i])->arNeighbour[j]>0)&&(*(arPlayer[0].pFog+(pFirstProvince+PR->arNeighbour[i])->arNeighbour[j])==2))
						{
							*(arPlayer[0].pFog+(pFirstProvince+PR->arNeighbour[i])->arNeighbour[j])=1;
						};
					};
				};
			};
		};
		CurrentPlayer=0;
		objMain=new G_Object(NULL,0,0,-1,bmpQuest[0]);
		objEvent=objMain;
		objMain=ShowMap(Player,objMain,Prov,&objEmpty);		
		if (PR->x+400>bmpMap->w)
		{dx=-200;};
		CentreMap(PR->x+dx,PR->y);
		objEvent->PlaceOnMap(PR->x,PR->y);
		objEvent->iLayer=8;
		objMain=ShowActiveObjects(objMain,10,false,MX,MY);
	};

	
	if ((HotSeat)&&(CurGame.iGameType==3))
	{
		iMessageFor=Player;
	};
	DialogDX=dx;
	Answer=(pFirstDialog+Dialog)->Show(An1,An2,An3,An4,Pic,Mess);		
	Exiter(objMain, Prov, bmpLowPanel);
	}
	return Answer;
};

void BeginMainTurn(int Player)
{
	GUI_ProvBar ProvBar;
	if (CurGame.iGameType<3)
	{arPlayer[1].bInGame=true;};
//	FileLog("MainTurn");
	
//Обработка событий
	if (CheckPlayerCondition(Player,false))
	{return;};
	int *Prov=new int [MapSize+1];
	const int LowY=768-bmpGUI_LowPanel->h;
	G_Object objEmpty(NULL,0,0,-1,bmpEmpty);	
	G_Object *objMain=NULL;
	BITMAP * bmpLowPanel=create_bitmap(bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);	
	MakeFog(Player);		
	CentreMap((pFirstProvince+arPlayer[Player].ipProvince)->x, (pFirstProvince+arPlayer[Player].ipProvince)->y);
	int dx;
	if (arPlayer[Player].Control==1)
	{
		iAntisleep=iAntisleepDelay;
		if (Tutorial.Map)
		{arPlayer[Player].iKarma=0;};
		
		CurrentPlayer=Player;
		objMain=new G_Object(NULL,0,LowY,5,bmpLowPanel);
		objMain=new G_Object(objMain,0,0,-1,bmpQuest[0]);
		objEvent=objMain;
		objMain=ShowMap(Player,objMain,Prov,&objEmpty);
		blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
		textprintf_centre_ex(bmpLowPanel,Font1,156,79,COLOR_2,-1,"%d",arPlayer[Player].Gold);
		textprintf_centre_ex(bmpLowPanel,Font1,280,79,COLOR_2,-1,"%d",arPlayer[Player].Gem);
		objMain=ProvBar.Init(664,5,pFirstProvince,false,objMain);
		ProvBar.Hide();
//		rectfill(screen1,0,0,1024,768,COLOR_1);
		bHourGlass=false;
		objMain=ShowActiveObjects(objMain,10,false,MX,MY);		
	}
	else
	{RunHourGlass(148, HOUR_SHIFT);};
	
	//Ритуальные события
	dcRitual *RR=pRitual;
	while (RR)
	{
		if (((pFirstProvince+RR->ipProvince)->ipPlayer==Player)&&(RR->iEvent>0))
		{
			RR->iEvent=0;
			dcProvince *PR=(pFirstProvince+RR->ipProvince);
			if (arPlayer[Player].Control==1)
			{				
				blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
				textprintf_centre_ex(bmpLowPanel,Font1,148,78,COLOR_2,-1,"%d",arPlayer[Player].Gold);
				textprintf_centre_ex(bmpLowPanel,Font1,276,78,COLOR_2,-1,"%d",arPlayer[Player].Gem);
				ProvBar.Activate(PR);
				dx=200;
				if (PR->x+400>bmpMap->w)
				{dx=-200;};
				CentreMap(PR->x+dx,PR->y);
				objEvent->PlaceOnMap(PR->x,PR->y);
				objEvent->iLayer=8;
				objMain=ShowActiveObjects(objMain,10,false,MX,MY);
				ProvBar.Hide();
				objEvent->iLayer=-1;
				EventDX=dx;
			};
			PR->StaticEvent((pFirstRitual+RR->Index)->ipEvent,Player);
			if ((pFirstRitual+RR->Index)->iTime==0)
			{RR->Timer=0;};			
		};
		RR=RR->pNext;
	};	
	RunEvents(Player,15,objMain);
	RunEvents(Player,1,objMain);
	RunEvents(Player,10,objMain);
	while ((!bExitGame)&&(arPlayer[Player].pEvent))
	{	
		dx=200;		
		if (arPlayer[Player].Control==1)
		{
			blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
			textprintf_centre_ex(bmpLowPanel,Font1,156,79,COLOR_2,-1,"%d",arPlayer[Player].Gold);
			textprintf_centre_ex(bmpLowPanel,Font1,280,79,COLOR_2,-1,"%d",arPlayer[Player].Gem);
			if (arPlayer[Player].pEvent->Prov2)
			{
				if (arPlayer[Player].pEvent->Prov2->x+400>bmpMap->w)
				{dx=-200;};
				CentreMap(arPlayer[Player].pEvent->Prov2->x+dx,arPlayer[Player].pEvent->Prov2->y);
				objEvent->PlaceOnMap(arPlayer[Player].pEvent->Prov2->x,arPlayer[Player].pEvent->Prov2->y);
				objEvent->iLayer=8;
			}
			else
			{
				if (arPlayer[Player].pEvent->Prov1->x+400>bmpMap->w)
				{dx=-200;};
				CentreMap(arPlayer[Player].pEvent->Prov1->x+dx,arPlayer[Player].pEvent->Prov1->y);
				objEvent->PlaceOnMap(arPlayer[Player].pEvent->Prov1->x,arPlayer[Player].pEvent->Prov1->y);
				objEvent->iLayer=8;
			};
			objMain=ShowActiveObjects(objMain,10,false,MX,MY);
		};		
		int Result=arPlayer[Player].pEvent->Call(dx);		
//		objEvent->iLayer=-1;
		if (bExitGame)
		{
			Exiter(objMain, Prov, bmpLowPanel);
			return;
		};
		if ((Result==555)&&(CheckPlayerCondition(Player,false)))
		{
			Exiter(objMain, Prov, bmpLowPanel);
			return;
		};
		if (arPlayer[Player].pEvent->Index==16)
		{
			if (Result==1)
			{
				dcEvent* EVCombat=EnterEnemyLand(arPlayer[Player].pEvent->Player1,arPlayer[Player].pEvent->Prov2->ipPlayer,arPlayer[Player].pEvent->Hero1,NULL,arPlayer[Player].pEvent->Prov1,arPlayer[Player].pEvent->Prov2);
				if (EVCombat)
				{
					EVCombat->Call(dx);
					delete EVCombat;
				};
			};
			if (Result==0)
			{
				dcEvent* EVCombat=EnterEnemyLand(arPlayer[Player].pEvent->Player2,arPlayer[Player].pEvent->Prov2->ipPlayer,arPlayer[Player].pEvent->Hero2,NULL,arPlayer[Player].pEvent->Prov1,arPlayer[Player].pEvent->Prov2);
				if (EVCombat)
				{
			//		RunHourGlass(148);
					EVCombat->Call(0);
					if (arPlayer[Player].Control==1)
					{bHourGlass=false;};
					delete EVCombat;
				};
			};
		};
		if (CheckPlayerCondition(Player,false))
		{
			Exiter(objMain, Prov, bmpLowPanel);
			return;
		};
		dcEvent *EV=arPlayer[Player].pEvent->pNext;
		delete arPlayer[Player].pEvent;
		arPlayer[Player].pEvent=EV;
	};
	if (CheckPlayerCondition(Player,false))
	{
		Exiter(objMain, Prov, bmpLowPanel);
		return;
	};
	WriteLog("События произошли");
	if (bExitGame)
	{
		Exiter(objMain, Prov, bmpLowPanel);
		return;
	};

	//Дипломатическая переписка
	MakeFog(Player);
	//Проверка почты союзников (для игрока-человека)
	if (arPlayer[Player].Control==1)
	{
		for (int i=1; i<=CurGame.iMaxPlayers; i++)
		{		
			if ((arPlayer[i].bInGame)&&(i!=Player)&&(arPlayer[i].Politic[Player].Treaty==3))
			{
				for (int j=1; j<=CurGame.iMaxPlayers; j++)
				{		
					if ((arPlayer[j].bInGame)&&(j!=i)&&(j!=Player)&&(arPlayer[i].Politic[j].Message.Type==1)&&(arPlayer[i].Politic[j].Message.Treaty==2))
					{
						char Mess[500];
						sprintf(Mess, (pFirstDialog+1125)->sText, arPlayer[i].GetName(), arPlayer[j].GetName());
						int Ans=(pFirstDialog+1125)->Show(1,1,0,0,arPlayer[j].GetAvatar(),Mess);
						if (Ans==1)
						{
							if (arPlayer[j].Control!=1)
							{
								arPlayer[Player].Politic[j].Treaty=2;
								arPlayer[j].Politic[Player].Treaty=2;
								arPlayer[i].Politic[j].Treaty=2;
								arPlayer[j].Politic[i].Treaty=2;
								arPlayer[Player].Politic[j].arTalk[2][2]=CurGame.iCurrentTurn;
								arPlayer[i].Politic[j].arTalk[2][2]=CurGame.iCurrentTurn;
								arPlayer[i].Politic[j].Message.Clear();
							}
							else
							{
								//Мир с игроком
							};
						};
					};
				};
			};
		};
	};
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		//Первая встреча
		if ((arPlayer[i].bInGame)&&(i!=Player)&&(arPlayer[i].Politic[Player].Treaty==0))
		{
			bool bMeet=false;
			for (int j=1; j<=MapSize; j++)
			{
				if (((pFirstProvince+j)->ipPlayer==Player)&&(*(arPlayer[i].pFog+j)<2))
				{bMeet=true;};
				if (((pFirstProvince+j)->ipPlayer==i)&&(*(arPlayer[Player].pFog+j)<2))
				{bMeet=true;};
			};
			if (bMeet)
			{
				if ((arPlayer[Player].Control==3)||(arPlayer[i].Control==3)||(!CurGame.bDiplomacy))
				{
					if (arPlayer[i].Control!=1)
					{
						arPlayer[Player].Politic[i].Treaty=1;
						arPlayer[i].Politic[Player].Treaty=1;
						if ((arPlayer[Player].Control==1)&&(arPlayer[i].Control==3)&&(CurGame.bFog))
						{
							(pFirstDialog+1204)->Show(1,1,0,0,NULL,NULL);
						};
						if ((arPlayer[Player].Control==1)&&(arPlayer[i].Control==2)&&(CurGame.bFog))
						{							
							char sTXT[200];
							sprintf(sTXT,(pFirstDialog+1205)->sText,arPlayer[i].GetName());
							(pFirstDialog+1205)->Show(1,1,0,0,NULL,sTXT);
						};
					};
				}
				else
				{
					arPlayer[Player].Politic[i].Treaty=2;
					arPlayer[i].Politic[Player].Treaty=2;
					if ((arPlayer[Player].Control==1)&&(arPlayer[i].Control!=1)&&(CurGame.bFog)&&(!Tutorial.Map))
					{
						SendGreetingsAI(i,Player);
						WindowMessage(Player, i);
					};
					if ((arPlayer[Player].Control!=1)&&(arPlayer[i].Control==1)&&(CurGame.bFog)&&(!Tutorial.Map))
					{
						SendGreetingsAI(Player,i);
					};					
				};
			};
		};
		//Получение ответа на сообщение
		if ((arPlayer[i].bInGame)&&(i!=Player)&&(arPlayer[i].Politic[Player].Answer!=0))
		{
			if (arPlayer[Player].Control==1)
			{
				SendAnswerAI(i,Player);
				if (arPlayer[i].Politic[Player].Answer<1000)
				{WindowMessage(0, i);}
				else
				{WindowMessage(0, arPlayer[i].Politic[Player].Answer-1000);};
			};
			arPlayer[i].Politic[Player].Answer=0;
		};
		//Сообщение при невыполнимом предложении
		if ((arPlayer[i].bInGame)&&(arPlayer[Player].Control==1)&&(!arPlayer[Player].CheckMessage(i)))
		//	((arPlayer[Player].Politic[i].Message.Type==2)||(arPlayer[Player].Politic[i].Message.Type==3)))
		{
	//		if ((!arPlayer[Player].Politic[i].Message.Trade.IsLegal(Player,arPlayer[Player].Politic[i].Message.Type))||(!arPlayer[Player].Politic[i].Message.Trade1.IsLegal(i,arPlayer[Player].Politic[i].Message.Type)))
	//		{
				char Txt[500];
				sprintf(Txt,(pFirstDialog+827)->sText,arPlayer[i].GetName());
				(pFirstDialog+827)->Show(true,false,false,false,NULL,Txt);
				arPlayer[i].CancelDeal(Player);
	//		};
		};
		//Получение сообщения
		if ((arPlayer[i].bInGame)&&(i!=Player)&&(arPlayer[i].Politic[Player].Message.Type>0))
		{
			if (arPlayer[i].CheckMessage(Player))
		//	if ((arPlayer[i].Politic[Player].Message.Trade.IsLegal(i,arPlayer[i].Politic[Player].Message.Type))&&(arPlayer[i].Politic[Player].Message.Trade1.IsLegal(Player,arPlayer[i].Politic[Player].Message.Type)))
			{
				if (arPlayer[Player].Control==1)
				{
					WindowMessage(Player, i);
					if (bExitGame)
					{
						Exiter(objMain, Prov, bmpLowPanel);
						return;
					};
				}
				else
				{
					if (arPlayer[i].Control==1)
					{
						if (arPlayer[Player].Politic[i].Answer==0)
						{
							arPlayer[Player].Politic[i].Answer=GetDiploAnswerAI(Player, i);
						};
					}
					else
					{

					};
				};
			}
			else
			{
				if (arPlayer[i].Control!=1)
				{arPlayer[Player].CancelDeal(i);};
			};
		};
	};

	//Объявление войны врагам союзников 
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if ((i!=Player)&&(arPlayer[i].bInGame))
		{
			if (arPlayer[Player].Politic[i].Treaty==3)
			{
				for (int j=1; j<=CurGame.iMaxPlayers; j++)
				{
					if ((j!=Player)&&(j!=i)&&(arPlayer[j].bInGame)&&(arPlayer[Player].Politic[j].Treaty==2)&&(arPlayer[i].Politic[j].Treaty==1))
					{
						if ((arPlayer[Player].Control==1)||(arPlayer[j].Control!=1))
						{
							arPlayer[Player].DeclareWar(j);
							if (arPlayer[Player].Control==1)
							{
								bHourGlass=false;
								OldProgress=0;
								char Mess[500];
								sprintf(Mess,(pFirstDialog+829)->sText,arPlayer[i].GetName(),arPlayer[j].GetName());
								(pFirstDialog+829)->Show(1,1,0,0,NULL,Mess);
							};
						};
					};
				};
			};
			//(и наоборот для человека)
			if ((arPlayer[Player].Control==1)&&(arPlayer[Player].Politic[i].Treaty==1))
			{
				for (int j=1; j<=CurGame.iMaxPlayers; j++)
				{
					if ((j!=Player)&&(j!=i)&&(arPlayer[j].bInGame)&&(arPlayer[Player].Politic[j].Treaty==2)&&(arPlayer[i].Politic[j].Treaty==3))
					{
						arPlayer[j].DeclareWar(Player);						
						char Mess[500];
						sprintf(Mess,(pFirstDialog+830)->sText,arPlayer[j].GetName(),arPlayer[i].GetName());
						(pFirstDialog+830)->Show(1,1,0,0,NULL,Mess);
					};
				};
			};
		};
	};
	
	//Возврат героев, осаждающих дружественные замки
	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		if (arPlayer[PL].bInGame)
		{	
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[PL].arHero[i])&&(arPlayer[PL].arHero[i]->iDead<0)&&(arPlayer[PL].arHero[i]->pProvince->ipPlayer!=PL)&&(arPlayer[PL].Politic[arPlayer[PL].arHero[i]->pProvince->ipPlayer].Treaty>1))
				{
					arPlayer[PL].ReturnHero(i);
				};
			};
		};
	};

	//Проверка контакта с другим правителем

	/*for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if ((arPlayer[i].bInGame)&&(i!=Player)&&(arPlayer[i].Politic[Player].Treaty==0))
		{
			bool bMeet=false;
			for (int j=1; j<=MapSize; j++)
			{
				if (((pFirstProvince+j)->ipPlayer==Player)&&(*(arPlayer[i].pFog+j)<2))
				{bMeet=true;};
				if (((pFirstProvince+j)->ipPlayer==i)&&(*(arPlayer[Player].pFog+j)<2))
				{bMeet=true;};
			};
			if (bMeet)
			{
				arPlayer[Player].Politic[i].Treaty=2;
				arPlayer[i].Politic[Player].Treaty=2;
				if ((arPlayer[Player].Control==1)&&(arPlayer[i].Control!=1))
				{
					SendGreetingsAI(i,Player);
					WindowMessage(Player, i);
				};
				if ((arPlayer[Player].Control!=1)&&(arPlayer[i].Control==1))
				{
					SendGreetingsAI(Player,i);
				};

			};
		};
	};*/
	
	
	if (arPlayer[Player].Control==1)
	{
	//Обязательные события (по правилам мира)
		if (WorldRules.iMode>0)		
		{
			if (((pFirstWorldRules+WorldRules.iMode)->ipSpecialEvent>0)&&((pFirstWorldRules+WorldRules.iMode)->iSpecialEventTurn==CurGame.iCurrentTurn))
			{
				int Target=arPlayer[Player].ipProvince;
				blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
				textprintf_centre_ex(bmpLowPanel,Font1,156,79,COLOR_2,-1,"%d",arPlayer[Player].Gold);
				textprintf_centre_ex(bmpLowPanel,Font1,280,79,COLOR_2,-1,"%d",arPlayer[Player].Gem);
				dx=200;
				if ((pFirstProvince+Target)->x+400>bmpMap->w)
				{dx=-200;};
				CentreMap((pFirstProvince+Target)->x+dx,(pFirstProvince+Target)->y);
				objEvent->PlaceOnMap((pFirstProvince+Target)->x,(pFirstProvince+Target)->y);
				objEvent->iLayer=8;
				ProvBar.Activate(pFirstProvince+Target);
				objMain=ShowActiveObjects(objMain,10,false,MX,MY);
				ProvBar.Hide();
				objEvent->iLayer=-1;
				EventDX=dx;
				(pFirstProvince+Target)->StaticEvent((pFirstWorldRules+WorldRules.iMode)->ipSpecialEvent,Player);
				if (bExitGame)
				{
					Exiter(objMain, Prov, bmpLowPanel);
					return;
				};
			};
		};	
	//Обязательные события (по таймеру)
	for (int i=1; i<=iMaxEvent; i++)
	{
		if (*(arPlayer[Player].pStory+i)>0)
		{
			*(arPlayer[Player].pStory+i)-=1;
			int Target=0;
			if (*(arPlayer[Player].pStory+i)==0)
			{
				if (*(arPlayer[Player].pPlace+i)>0)
				{
					if ((pFirstProvince+*(arPlayer[Player].pPlace+i))->ipPlayer==Player)
					{
						Target=*(arPlayer[Player].pPlace+i);						
					};
				}
				else
				{
					ClearRoll();
					int Count=0;
					for (int Prov=1; Prov<MapSize; Prov++)
					{
						if ((pFirstProvince+Prov)->ipPlayer==Player)
						{
							RollerN[Count]=Prov;
							RollerP[Count]=10;
							Count++;
						};
					};
					if (Count>0)
					{
						Target=Roll(false);
					};
				};
				*(arPlayer[Player].pPlace+i)=0;
				if (Target>0)
				{
					blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
					textprintf_centre_ex(bmpLowPanel,Font1,156,79,COLOR_2,-1,"%d",arPlayer[Player].Gold);
					textprintf_centre_ex(bmpLowPanel,Font1,280,79,COLOR_2,-1,"%d",arPlayer[Player].Gem);
					dx=200;
					if ((pFirstProvince+Target)->x+400>bmpMap->w)
					{dx=-200;};
					CentreMap((pFirstProvince+Target)->x+dx,(pFirstProvince+Target)->y);
					objEvent->PlaceOnMap((pFirstProvince+Target)->x,(pFirstProvince+Target)->y);
					objEvent->iLayer=8;
					ProvBar.Activate(pFirstProvince+Target);
					objMain=ShowActiveObjects(objMain,10,false,MX,MY);
					ProvBar.Hide();
					objEvent->iLayer=-1;
					EventDX=dx;
					(pFirstProvince+Target)->StaticEvent(i,Player);
					if (bExitGame)
					{
						Exiter(objMain, Prov, bmpLowPanel);
						return;
					};
				};
			};
		};
	};
//Активизация квеста (кампания)
	if ((CurGame.iGameType==2)&&(Campaign.iActiveShard>=0))
	{
		for (int i=0; i<MAX_CAMP_QUEST; i++)
		{
			if ((Campaign.arQuest[i][0]>0)&&(Campaign.arQuest[i][1]==0)&&((pFirstQuest+Campaign.arQuest[i][0])->iTime<=CurGame.iCurrentTurn))
			{
				Campaign.arQuest[i][1]=1;
				if ((pFirstQuest+Campaign.arQuest[i][0])->ipEncWait)
				{
					blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
					textprintf_centre_ex(bmpLowPanel,Font1,148,78,COLOR_2,-1,"%d",arPlayer[Player].Gold);
					textprintf_centre_ex(bmpLowPanel,Font1,276,78,COLOR_2,-1,"%d",arPlayer[Player].Gem);
					dx=200;
					if ((pFirstProvince+arPlayer[Player].ipProvince)->x+400>bmpMap->w)
					{dx=-200;};
					CentreMap((pFirstProvince+arPlayer[Player].ipProvince)->x+dx,(pFirstProvince+arPlayer[Player].ipProvince)->y);
					objEvent->PlaceOnMap((pFirstProvince+arPlayer[Player].ipProvince)->x,(pFirstProvince+arPlayer[Player].ipProvince)->y);
					objEvent->iLayer=8;
					ProvBar.Activate((pFirstProvince+arPlayer[Player].ipProvince));
					objMain=ShowActiveObjects(objMain,10,false,MX,MY);
					ProvBar.Hide();
					objEvent->iLayer=-1;
					EventDX=dx;
					(pFirstProvince+arPlayer[Player].ipProvince)->StaticEvent((pFirstQuest+Campaign.arQuest[i][0])->ipEncWait,Player);
					if (bExitGame)
					{
						Exiter(objMain, Prov, bmpLowPanel);
						return;
					};
				};
			};
		};
//Обязательные события (кампания)
		for (int EV=1; EV<=iMaxEvent; EV++)
		{
			clEvent *pEV=(pFirstEvent+EV);
			if (pEV->iPossibility==0)
			{
			for (int i=0; i<MAX_EVENT_EFFECT; i++)
			{
				if ((pEV->arEffect[i][0]==8)&&(pEV->arEffect[i][1]==26)&&(Campaign.arDialog[pEV->arEffect[i][2]])&&(pEV->arEffect[i][3]==CurGame.iCurrentTurn))
				{
					blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
					textprintf_centre_ex(bmpLowPanel,Font1,148,78,COLOR_2,-1,"%d",arPlayer[Player].Gold);
					textprintf_centre_ex(bmpLowPanel,Font1,276,78,COLOR_2,-1,"%d",arPlayer[Player].Gem);
					dx=200;
					if ((pFirstProvince+arPlayer[Player].ipProvince)->x+400>bmpMap->w)
					{dx=-200;};
					CentreMap((pFirstProvince+arPlayer[Player].ipProvince)->x+dx,(pFirstProvince+arPlayer[Player].ipProvince)->y);
					objEvent->PlaceOnMap((pFirstProvince+arPlayer[Player].ipProvince)->x,(pFirstProvince+arPlayer[Player].ipProvince)->y);
					objEvent->iLayer=8;
					ProvBar.Activate((pFirstProvince+arPlayer[Player].ipProvince));
					objMain=ShowActiveObjects(objMain,10,false,MX,MY);
					ProvBar.Hide();
					objEvent->iLayer=-1;
					EventDX=dx;
					(pFirstProvince+arPlayer[Player].ipProvince)->StaticEvent(EV,Player);
					if (bExitGame)
					{
						Exiter(objMain, Prov, bmpLowPanel);
						return;
					};
				};
			};
			};
		};
	};
//События при атаке оппонентов
	if ((CurGame.iGameType==2)&&(Campaign.iActiveShard<0))
	{
		dcHero *pGarrison=(pFirstProvince+arPlayer[Player].ipProvince)->pGarrison;
		int Event=0;
		switch (abs(Campaign.iActiveShard))
		{
		case 1:
			//Вианта против Адрагерона
			if ((Campaign.arDialog[322]>0)&&(Campaign.arCampPlayer[12].bInGame))
			{
				if ((Campaign.arDialog[322]==3)&&(pGarrison->GetFreeUnitSlotNum(2)>=2)&&(CurGame.iCurrentTurn>20+Random(10)+Random(10)))
				{
					Event=992;
					Campaign.arDialog[322]=4;
				};
				if ((Campaign.arDialog[322]==2)&&(pGarrison->GetFreeUnitSlotNum(1)>=3)&&(CurGame.iCurrentTurn>2+Random(3)+Random(3)))
				{
					Event=991;
					Campaign.arDialog[322]=3;
				};
				if ((Campaign.arDialog[322]==1)&&(CurGame.iCurrentTurn==2))
				{
					Event=990;
					Campaign.arDialog[322]=2;
				};
			};
			break;
		case 7:
			//Вианта против Ланшара
			if ((Campaign.arDialog[310]>0)&&(Campaign.arCampPlayer[12].bInGame))
			{
				if ((Campaign.arDialog[310]==3)&&(pGarrison->GetFreeUnitSlotNum(2)>=2)&&(CurGame.iCurrentTurn>20+Random(10)+Random(10)))
				{
					Event=992;
					Campaign.arDialog[310]=4;
				};
				if ((Campaign.arDialog[310]==2)&&(pGarrison->GetFreeUnitSlotNum(1)>=3)&&(CurGame.iCurrentTurn>2+Random(3)+Random(3)))
				{
					Event=991;
					Campaign.arDialog[310]=3;
				};
				if ((Campaign.arDialog[310]==1)&&(CurGame.iCurrentTurn==2))
				{
					Event=990;
					Campaign.arDialog[310]=2;
				};
			};
			break;
		case 14:
			//Белез против Стинхрии
			if ((Campaign.arDialog[111]>0)&&(Campaign.arCampPlayer[3].bInGame))
			{
				int Slots=pGarrison->GetFreeUnitSlotNum(2);
				if ((Campaign.arDialog[111]==3)&&(pGarrison->GetFreeUnitSlotNum(2)>=2)&&(CurGame.iCurrentTurn>20+Random(10)+Random(10)))
				{
					Event=995;
					Campaign.arDialog[111]=4;
				};
				if ((Campaign.arDialog[111]==2)&&(arPlayer[Player].arBld[3])&&(arPlayer[Player].arBld[6]))
				{
					Event=994;
					Campaign.arDialog[111]=3;
					int Gold=arPlayer[Player].Gold, Gem=arPlayer[Player].Gem;
					arPlayer[Player].Build(22);
					arPlayer[Player].Gold=Gold; arPlayer[Player].Gem=Gem;
				};
				if ((Campaign.arDialog[111]==1)&&(CurGame.iCurrentTurn==2))
				{
					Event=993;
					Campaign.arDialog[111]=2;
				};
			};
			break;
		};
		if (Event>0)
		{
			blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
			textprintf_centre_ex(bmpLowPanel,Font1,148,78,COLOR_2,-1,"%d",arPlayer[Player].Gold);
			textprintf_centre_ex(bmpLowPanel,Font1,276,78,COLOR_2,-1,"%d",arPlayer[Player].Gem);
			dx=200;
			if ((pFirstProvince+arPlayer[Player].ipProvince)->x+400>bmpMap->w)
			{dx=-200;};
			CentreMap((pFirstProvince+arPlayer[Player].ipProvince)->x+dx,(pFirstProvince+arPlayer[Player].ipProvince)->y);
			objEvent->PlaceOnMap((pFirstProvince+arPlayer[Player].ipProvince)->x,(pFirstProvince+arPlayer[Player].ipProvince)->y);
			objEvent->iLayer=8;
			ProvBar.Activate((pFirstProvince+arPlayer[Player].ipProvince));
			objMain=ShowActiveObjects(objMain,10,false,MX,MY);
			ProvBar.Hide();
			objEvent->iLayer=-1;
			EventDX=dx;
			(pFirstProvince+arPlayer[Player].ipProvince)->StaticEvent(Event,Player);
			if (bExitGame)
			{
				Exiter(objMain, Prov, bmpLowPanel);
				return;
			};
		};
	};
	
	};

//Доход и лечение войска
	int GoldInc=0, GoldSpent=0, ProvNum=0;
	int GemInc=0, GemSpent=0;	
	arPlayer[Player].iCanInnerBld=0;
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if ((arPlayer[Player].CanBuild(i))&&(!IsRestricted(i, Player)))
		{
			arPlayer[Player].iCanInnerBld=1;
			break;
		}
	};

	arPlayer[Player].iCanOuterBld=1;
	arPlayer[Player].iCanHire=1;
	arPlayer[Player].iCanCast=1;
//Провинции
	for (int i=1; i<=MapSize; i++)
	{
		if ((pFirstProvince+i)->ipPlayer==Player)
		{
		if ((pFirstProvince+i)->iPopulation>0)
		{
			ProvNum+=(pFirstProvince+i)->GetCorruption();
		};
//		char txt[200];
//		sprintf(txt,"Провинция: %s",(pFirstProvince+i)->sName);
//		FileLog(txt);
//		FileLog("Доход...      ");
		int DefFourage=0;
		if ((pFirstProvince+i)->GetSiegeIndex()<0)
		{
		GoldInc+=(pFirstProvince+i)->GetGoldIncome();
		GemInc+=(pFirstProvince+i)->GetGemIncome();
		//Прирост населения
		int OldLvl=(pFirstProvince+i)->GetLevel();
		if (OldLvl<MAX_PROVINCE_LEVEL)
		{			
			if (((pFirstProvince+i)->iExplore>=ProvLevel[(pFirstProvince+i)->GetLevel()].iExplored)||((pFirstProvince+i)->GetGrow()<0))
			{
				if (((pFirstProvince+i)->GetGrow()<0)&&((pFirstProvince+i)->iPopulation+(pFirstProvince+i)->GetGrow()<=0))
				{
					//На грани вымирания
				}
				else
				{
					(pFirstProvince+i)->iPopulation+=(pFirstProvince+i)->GetGrow();
					(pFirstProvince+i)->bWarning=true;
				};

			}
			else
			{
				if (((pFirstProvince+i)->bWarning)&&(arPlayer[Player].Control==1)&&(arHumans[arPlayer[Player].iColor].arMess[1]))
				{
					(pFirstProvince+i)->bWarning=false;
					if (!Tutorial.Map)
					{
						blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
						textprintf_centre_ex(bmpLowPanel,Font1,156,79,COLOR_2,-1,"%d",arPlayer[Player].Gold);
						textprintf_centre_ex(bmpLowPanel,Font1,280,79,COLOR_2,-1,"%d",arPlayer[Player].Gem);
						dx=200;
						if ((pFirstProvince+i)->x+400>bmpMap->w)
						{dx=-200;};
						CentreMap((pFirstProvince+i)->x+dx,(pFirstProvince+i)->y);
						objEvent->PlaceOnMap((pFirstProvince+i)->x,(pFirstProvince+i)->y);
						objEvent->iLayer=8;
						ProvBar.Activate((pFirstProvince+i));
						objMain=ShowActiveObjects(objMain,10,false,MX,MY);
						ProvBar.Hide();
						objEvent->iLayer=-1;
						char Message[500];
						sprintf(Message,(pFirstDialog+25)->sText,(pFirstProvince+i)->sName);
						DialogDX=dx;
						(pFirstDialog+25)->Show(true,true,false,false,NULL,Message);
					};
				};
			};
			if (OldLvl!=(pFirstProvince+i)->GetLevel())
			{
				if (arPlayer[Player].Control==1)
				{(pFirstProvince+i)->DrawPic(true);}
				else
				{(pFirstProvince+i)->DrawPic(false);};
			};
		};		
		//Защитник
	//	FileLog("Защитники...");		
		if ((pFirstProvince+i)->iDefender>0)
		{
			if (arPlayer[Player].Control==1)
			{
				if ((pFirstDefender+(pFirstProvince+i)->iDefender)->iGoldPayment<0)
				{
					GoldSpent+=(pFirstProvince+i)->pGuard->GetDefenderGoldPayment();
					GemSpent+=(pFirstProvince+i)->pGuard->GetDefenderGemPayment();
				}
				else
				{
					GoldSpent+=(pFirstDefender+(pFirstProvince+i)->iDefender)->iGoldPayment;
					GemSpent+=(pFirstDefender+(pFirstProvince+i)->iDefender)->iGemPayment;
				};
			};
			for (int AB=0; AB<MAX_DEFENDER_ABILITY; AB++)
			{
				if ((pFirstDefender+(pFirstProvince+i)->iDefender)->arAbility[AB][0]==4)
				{
					DefFourage=(pFirstDefender+(pFirstProvince+i)->iDefender)->arAbility[AB][1];
				};
			};
			(pFirstProvince+i)->pGuard->RestoreUnits(true,(pFirstProvince+i)->GetHeal(),true);

		};
		}
		else
		{
			if ((pFirstProvince+i)->GetSiegeIndex()==0)
			{
				dcProvince *PR=(pFirstProvince+i);
				int Time=0;
				PR->iFort-=PR->pGuard->GetSiege();
				int Fort;
				if (arPlayer[Player].ipProvince==PR->iNumber)
				{
					Fort=100*PR->iFort/arPlayer[Player].iWall;
					if (Fort>50)
					{
						Time=(PR->iFort-arPlayer[Player].iWall/2)/PR->pGuard->GetSiege();
						if ((PR->iFort-arPlayer[Player].iWall/2)%PR->pGuard->GetSiege()>0)
						{Time++;};
					};
				}
				else
				{
					int BLD=-1;
					for (int i=0; i<MAX_PROVINCE_BLD; i++)
					{
						if ((PR->arBuilding[i]>0)&&(pFirstOuterBld+PR->arBuilding[i])->iWall>0)
						{
							BLD=PR->arBuilding[i];
						};
					};
					if (BLD>=0)
					{
						Fort=100*PR->iFort/(pFirstOuterBld+BLD)->iWall;
						Time=(PR->iFort-(pFirstOuterBld+BLD)->iWall/2)/PR->pGuard->GetSiege();
						if ((PR->iFort-(pFirstOuterBld+BLD)->iWall/2)%PR->pGuard->GetSiege()>0)
						{Time++;};
					}
					else
					{
						StopLog("НЕ НАЙДЕНЫ УКРЕПЛЕНИЯ В ПРОВИНЦИИ ",PR->iNumber);
						Fort=0;
					};
				};
				if (Fort<=50)
				{
					if (arPlayer[Player].Control==1)
					{						
						char sMessage[1000];
						sprintf(sMessage,(pFirstDialog+381)->sText,PR->sName);
						(pFirstDialog+381)->Show(true,true,false,false,NULL,sMessage);
						if (InitBattle(PR,NULL,PR->pGuard,PR->pGarrison,-1,true)==1)
						{
							PR->Conquer(0,NULL);
							if (PR->ipEvent>0)
							{
								PR->StaticEvent(PR->ipEvent,Player);
							}
							else
							{
								sprintf(sMessage,(pFirstDialog+382)->sText,PR->sName);
								(pFirstDialog+382)->Show(true,true,false,false,NULL,sMessage);
							};
							PR->ipEvent=0;
						}
						else
						{
							bool bDead=true;
							for (int SQ=0; SQ<MAX_SQUAD; SQ++)
							{
								if ((PR->pGuard->pSquad[SQ])&&(PR->pGuard->pSquad[SQ]->iLife>0))
								{bDead=false;};
							};
							if (bDead)
							{
								delete PR->pGuard;
								PR->pGuard=NULL;
							};
						};
					}
					else
					{
						if ((!PR->pGarrison)||(InitBattle(PR,NULL,PR->pGuard,PR->pGarrison,-1,false)==1))
						{
							PR->Conquer(0,NULL);
							if (bExitGame)
							{
								Exiter(objMain, Prov, bmpLowPanel);
								return;
							};
						}
						else
						{
							delete PR->pGuard;
							PR->pGuard=NULL;
						};
					};
				}
				else
				{
					if (arPlayer[Player].Control==1)
					{
						char sMessage[1000];						
						sprintf(sMessage,(pFirstDialog+379)->sText,PR->sName,Time);
						(pFirstDialog+379)->Show(true,true,false,false,NULL,sMessage);
					};
				};						
			};
		};
		//Гарнизон
	//	FileLog("Гарнизоны...");
		if ((pFirstProvince+i)->pGarrison)
		{
			int Payment=0;
			for (int k=0; k<MAX_SQUAD; k++)
			{
				if ((pFirstProvince+i)->pGarrison->pSquad[k])
				{				
					Payment+=(pFirstProvince+i)->pGarrison->pSquad[k]->GetGoldPayment();
					GemSpent+=(pFirstProvince+i)->pGarrison->pSquad[k]->GetGemPayment();
				};
			};
			(pFirstProvince+i)->pGarrison->RestoreUnits(true,(pFirstProvince+i)->GetHeal()+10,false);
			Payment-=Payment*(DefFourage+arPlayer[Player].GetBldParam1(28))/100;
			if (Payment>0)
			{GoldSpent+=Payment;};
		};
//		FileLog("События...   ");
			int RND;
			if (bStaticEvent)
			{RND=Random(1000, CurGame.iCurrentTurn, i);}
			else
			{RND=Random(1000);}
			if ((CurGame.iCurrentTurn>2)&&(!Tutorial.Map)&&(RND<EVENT_POSSIBILITY))
			{
			//Случайное событие
				if (arPlayer[Player].Control==1)
				{
					ClearRoll();
					int Count=0;				
					for (int j=1; j<=iMaxEvent; j++)
					{
						int Poss=(pFirstProvince+i)->GetEventPoss(j);
						if (*(arPlayer[Player].pStory+j)>=0)
						{Poss=0;};
						if (Poss>0)
						{
							RollerN[Count]=j;
							RollerP[Count]=Poss;
							Count++;
						};					
					};
					if (Count>0)
					{
						int Event;
						if (bStaticEvent)
						{Event=Roll(false,i,CurGame.iCurrentTurn);}
						else
						{Event=Roll(false);}
						StopLog("Событие: ",Event);
					//	rectfill(screen1,0,0,1024,768,COLOR_1);
						blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
						textprintf_centre_ex(bmpLowPanel,Font1,148,78,COLOR_2,-1,"%d",arPlayer[Player].Gold);
						textprintf_centre_ex(bmpLowPanel,Font1,276,78,COLOR_2,-1,"%d",arPlayer[Player].Gem);
						dx=200;
						if ((pFirstProvince+i)->x+400>bmpMap->w)
						{dx=-200;};
						CentreMap((pFirstProvince+i)->x+dx,(pFirstProvince+i)->y);
						objEvent->PlaceOnMap((pFirstProvince+i)->x,(pFirstProvince+i)->y);
						objEvent->iLayer=8;
						ProvBar.Activate((pFirstProvince+i));
						objMain=ShowActiveObjects(objMain,10,false,MX,MY);
						ProvBar.Hide();
						objEvent->iLayer=-1;
						EventDX=dx;
						(pFirstProvince+i)->StaticEvent(Event,Player);
						if (bExitGame)
						{
							Exiter(objMain, Prov, bmpLowPanel);
							return;
						};
					};
				}
				else
				{
					if ((Random(100)-(pFirstPlayerProfile+arPlayer[Player].ipProfile)->iKarma/10+(pFirstProvince+i)->GetMood()*10)>50)
					{
					if ((pFirstPlayerProfile+arPlayer[Player].ipProfile)->iKarma==0)
					{
				//		arPlayer[Player].iKarma=Random(11)-5;
						arPlayer[Player].Gold+=Random(100);
						arPlayer[Player].Gem+=Random(20);
					};
					if ((pFirstPlayerProfile+arPlayer[Player].ipProfile)->iKarma>0)
					{arPlayer[Player].Gold+=Random(200);};
					if ((pFirstPlayerProfile+arPlayer[Player].ipProfile)->iKarma<0)
					{arPlayer[Player].Gem+=Random(40);};
					arPlayer[Player].Gold+=Random(200);
					arPlayer[Player].Gem+=Random(40);
					if (arPlayer[Player].ipProvince!=i)
					{(pFirstProvince+i)->ChangeAttitude(Player,-1);}
					}
					else
					{(pFirstProvince+i)->ChangeAttitude(Player,1);};
				};
			}
			else
			{
		
			WriteLog("Недовольство...");
		//Изменение шкалы недовольства
			if ((pFirstProvince+i)->GetSiegeIndex()<0)
			{
			if ((pFirstProvince+i)->GetMood()>=0)
			{
				if	(((pFirstProvince+i)->arUnrest[Player]>=0)&&((pFirstProvince+i)->iPopulation>0))
				{
					(pFirstProvince+i)->arUnrest[Player]-=(1+(pFirstProvince+i)->GetMood());
					if ((pFirstProvince+i)->arUnrest[Player]<0)
					{(pFirstProvince+i)->arUnrest[Player]=0;};
				};
			}
			else
			{
				if ((pFirstProvince+i)->iPopulation>0)
				{
		//			if ((pFirstProvince+i)->arAttitude[1][0]<-50)
		//			{(pFirstProvince+i)->arAttitude[1][0]=0;};
					int Unrest=(pFirstProvince+i)->GetMood()*(pFirstProvince+i)->GetLevel()*(pFirstRace+(pFirstProvince+i)->ipRace)->iUnrest/50+(pFirstProvince+i)->GetUnrestSlowdown();
					if (Unrest>=0)
					{
						Unrest=-1;
					};
					(pFirstProvince+i)->arUnrest[Player]-=Unrest;
					if ((pFirstProvince+i)->arUnrest[Player]>=MAX_UNREST)
					{
						if (arPlayer[Player].Control==1)
						{
							blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
							textprintf_centre_ex(bmpLowPanel,Font1,156,79,COLOR_2,-1,"%d",arPlayer[Player].Gold);
							textprintf_centre_ex(bmpLowPanel,Font1,280,79,COLOR_2,-1,"%d",arPlayer[Player].Gem);
							dx=200;
							if ((pFirstProvince+i)->x+400>bmpMap->w)
							{dx=-200;};
							CentreMap((pFirstProvince+i)->x+dx,(pFirstProvince+i)->y);
							objEvent->PlaceOnMap((pFirstProvince+i)->x,(pFirstProvince+i)->y);
							objEvent->iLayer=8;
							ProvBar.Activate((pFirstProvince+i));
							objMain=ShowActiveObjects(objMain,10,false,MX,MY);
							ProvBar.Hide();
							objEvent->iLayer=-1;
						};
						Rebel(pFirstProvince+i,dx);
						if (bExitGame)
						{
							Exiter(objMain, Prov, bmpLowPanel);
							return;
						};
						(pFirstProvince+i)->iPopulation-=(pFirstProvince+i)->iPopulation*(20+Random(15))/100;
						(pFirstProvince+i)->arUnrest[Player]=200+Random(100);
					};
				};
			};
			};
			};
		};
	};
		
	int Perc=0;
	WriteLog("Родовой замок...");
//Родовой замок
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if (arPlayer[Player].arBld[i])
		{
			for (int j=0; j<MAX_BUILDING_ABILITY; j++)
			{
				if ((pFirstInnerBld+i)->arAbility[j][0]==5)
				{
					GoldInc+=(pFirstInnerBld+i)->arAbility[j][1];
					GemInc+=(pFirstInnerBld+i)->arAbility[j][2];
				};
				if ((pFirstInnerBld+i)->arAbility[j][0]==9)
				{
					GoldInc+=arPlayer[Player].GetResource((pFirstInnerBld+i)->arAbility[j][1])*(pFirstInnerBld+i)->arAbility[j][2];
				};
				if ((pFirstInnerBld+i)->arAbility[j][0]==22)
				{
					Perc+=(pFirstInnerBld+i)->arAbility[j][1];
				};
				if ((pFirstInnerBld+i)->arAbility[j][0]==39)
				{
					arPlayer[Player].iCanHire+=(pFirstInnerBld+i)->arAbility[j][1];
				};
				if ((pFirstInnerBld+i)->arAbility[j][0]==40)
				{
					arPlayer[Player].iCanOuterBld+=(pFirstInnerBld+i)->arAbility[j][1];
				};
				if ((pFirstInnerBld+i)->arAbility[j][0]==54)
				{
					arPlayer[Player].iCanCast+=(pFirstInnerBld+i)->arAbility[j][1];
				};
			};
		};		
	};
	bool bHidden=bDemo;
	int Profit=arPlayer[Player].Gold*arPlayer[Player].GetBldParam1(32)/100;
	if (Profit>MAX_PROFIT)
	{Profit=MAX_PROFIT;};
	GoldInc+=Profit;
	GoldInc+=arPlayer[Player].iGoldAdd;
	GemInc+=arPlayer[Player].iGemAdd;
//	GoldInc+=arPlayer[Player].Gold*arPlayer[Player].GetBldParam1(32)/100;
//Оппоненты
	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		if ((PL!=Player)&&(arPlayer[PL].bInGame))
		{
			if (arPlayer[Player].Politic[PL].Trade.bTrade)
			{
				GoldInc-=arPlayer[Player].Politic[PL].Trade.Gold;
				GoldInc+=arPlayer[PL].Politic[Player].Trade.Gold;
				GemInc-=arPlayer[Player].Politic[PL].Trade.Gem;
				GemInc+=arPlayer[PL].Politic[Player].Trade.Gem;
			};
			for (int i=1; i<=iMaxInnerBld; i++)
			{
				if (arPlayer[PL].arBld[i])
				{
					for (int j=0; j<MAX_BUILDING_ABILITY; j++)
					{
						if ((pFirstInnerBld+i)->arAbility[j][0]==29)
						{
							GoldInc+=(pFirstInnerBld+i)->arAbility[j][1];
						};
					};
				};
			};			
		};
	};
//Герои
	WriteLog("Герои...                                 ");
	for (int Hero=0; Hero<MAX_HERO; Hero++)
	{
		if (arPlayer[Player].arHero[Hero])
		{
			if (arPlayer[Player].arHero[Hero]->iDead>0)
			{
				arPlayer[Player].arHero[Hero]->iDead--;
				if ((arPlayer[Player].Control==1)&&(arPlayer[Player].arHero[Hero]->iDead==0))
				{
					int Gold=arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[Hero]), Gem=arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[Hero]);
				//	sprintf((pFirstDialog+1239)->arAnswer[0],(pFirstPhrase+501)->str,Gold,Gem);
					char TXT[1000];
					sprintf(TXT,(pFirstDialog+1239)->sText,arPlayer[Player].arHero[Hero]->pstName->str,Gold,Gem);
					int Answer=0;
					if ((arPlayer[Player].Gold>=Gold)&&(arPlayer[Player].Gem>=Gem)&&((pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()<0))
					{Answer=(pFirstDialog+1239)->Show(1,1,0,0,arPlayer[Player].arHero[Hero]->Pic,TXT);}
					else
					{(pFirstDialog+1239)->Show(0,1,0,0,arPlayer[Player].arHero[Hero]->Pic,TXT);};
					if (Answer==1)
					{
						PlaySpellSound(58);		
						arPlayer[Player].Gold-=Gold;
						arPlayer[Player].Gem-=Gem;
						arPlayer[Player].arHero[Hero]->iDead=-1;
						arPlayer[Player].arHero[Hero]->ipProvince=arPlayer[Player].ipProvince;
						arPlayer[Player].arHero[Hero]->pProvince=(pFirstProvince+arPlayer[Player].ipProvince);
						arPlayer[Player].arHero[Hero]->pSquad[0]->iLife=arPlayer[Player].arHero[Hero]->pSquad[0]->GetLife()/2+1;
					};
				};
			}
			else
			{
				//Отмена исследования полностью исследованных провинций
				if ((arPlayer[Player].arHero[Hero]->iTask==2)&&(arPlayer[Player].arHero[Hero]->pProvince->iExplore>=100))
				{arPlayer[Player].arHero[Hero]->iTask=0;};
			};
			int Payment=0;
			for (int k=0; k<MAX_SQUAD; k++)
			{				
				if (arPlayer[Player].arHero[Hero]->pSquad[k])
				{
					Payment+=arPlayer[Player].arHero[Hero]->pSquad[k]->GetGoldPayment();
					GemSpent+=arPlayer[Player].arHero[Hero]->pSquad[k]->GetGemPayment();
					/*int HealBonus=0;
					if (k>0)
					{HealBonus=arPlayer[Player].arHero[Hero]->pSquad[k]->GetHealBonus();}
					else
					{HealBonus=arPlayer[Player].arHero[Hero]->iHealth+arPlayer[Player].arHero[Hero]->CheckHeroBonus(35)+arPlayer[Player].arHero[Hero]->CheckHeroBonus(48)*2;};
					if (arPlayer[Player].arHero[Hero]->pProvince->ipPlayer==Player)
					{
						if ((arPlayer[Player].arHero[Hero]->pSquad[k]->iLife<arPlayer[Player].arHero[Hero]->pSquad[k]->GetLife())&&(arPlayer[Player].arHero[Hero]->pSquad[k]->iLife>0))
						{
							int LifeAdd=arPlayer[Player].arHero[Hero]->pSquad[k]->GetLife()*(10+arPlayer[Player].arHero[Hero]->pProvince->GetHeal())/100;							
							if (LifeAdd<1)
							{LifeAdd=1;};
							LifeAdd+=HealBonus;
							arPlayer[Player].arHero[Hero]->pSquad[k]->iLife+=LifeAdd;
							if (arPlayer[Player].arHero[Hero]->pSquad[k]->iLife>arPlayer[Player].arHero[Hero]->pSquad[k]->GetLife())
							{
								arPlayer[Player].arHero[Hero]->pSquad[k]->iLife=arPlayer[Player].arHero[Hero]->pSquad[k]->GetLife();					
							};
						};
						if (arPlayer[Player].arHero[Hero]->pSquad[k]->iMorale<0)
						{
							arPlayer[Player].arHero[Hero]->pSquad[k]->iMorale++;
						};
						if (arPlayer[Player].arHero[Hero]->pSquad[k]->iMorale>0)
						{
							arPlayer[Player].arHero[Hero]->pSquad[k]->iMorale--;
						};
					}
					else
					{
						if ((arPlayer[Player].arHero[Hero]->pSquad[k]->iLife<arPlayer[Player].arHero[Hero]->pSquad[k]->GetLife())&&(arPlayer[Player].arHero[Hero]->pSquad[k]->iLife>0))
						{
							arPlayer[Player].arHero[Hero]->pSquad[k]->iLife+=HealBonus;
							if (arPlayer[Player].arHero[Hero]->pSquad[k]->iLife>arPlayer[Player].arHero[Hero]->pSquad[k]->GetLife())
							{
								arPlayer[Player].arHero[Hero]->pSquad[k]->iLife=arPlayer[Player].arHero[Hero]->pSquad[k]->GetLife();					
							};							
						};
						if (arPlayer[Player].arHero[Hero]->pSquad[k]->iMorale>-15)
						{
							arPlayer[Player].arHero[Hero]->pSquad[k]->iMorale--;
						};
					};*/
				};
			};
			if (arPlayer[Player].arHero[Hero]->pProvince->ipPlayer==Player)
			{arPlayer[Player].arHero[Hero]->RestoreUnits(true,arPlayer[Player].arHero[Hero]->pProvince->GetHeal(),false);}
			else
			{arPlayer[Player].arHero[Hero]->RestoreUnits(false,arPlayer[Player].arHero[Hero]->pProvince->GetHeal(),false);};
			if (arPlayer[Player].arHero[Hero]->iDead<0)
			{
				Payment-=Payment*arPlayer[Player].arHero[Hero]->CheckHeroBonus(997)/100;
				GoldInc+=arPlayer[Player].arHero[Hero]->CheckHeroBonus(81);
				GemInc+=arPlayer[Player].arHero[Hero]->CheckHeroBonus(82);
			//	arPlayer[Player].arHero[Hero]->CheckQuestsFail();
			//	void dcHero::CheckQuestsFail()
				dcHero *HR=arPlayer[Player].arHero[Hero];
				for (int i=0; i<HERO_QUEST_LIMIT; i++)
				{
					if (HR->arQuest[i].Index>0)
					{
						if (HR->arQuest[i].ipSite>=0)
						{
							if ((pFirstProvince+HR->arQuest[i].ipProvince)->arSite[HR->arQuest[i].ipSite][0]==0)
							{
								if (arPlayer[Player].Control==1)
								{
									blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
									textprintf_centre_ex(bmpLowPanel,Font1,156,79,COLOR_2,-1,"%d",arPlayer[Player].Gold);
									textprintf_centre_ex(bmpLowPanel,Font1,280,79,COLOR_2,-1,"%d",arPlayer[Player].Gem);
									dx=200;
									if ((pFirstProvince+HR->arQuest[i].ipProvince)->x+400>bmpMap->w)
									{dx=-200;};
									CentreMap((pFirstProvince+HR->arQuest[i].ipProvince)->x+dx,(pFirstProvince+HR->arQuest[i].ipProvince)->y);
									objEvent->PlaceOnMap((pFirstProvince+HR->arQuest[i].ipProvince)->x,(pFirstProvince+HR->arQuest[i].ipProvince)->y);
									objEvent->iLayer=8;
									objMain=ShowActiveObjects(objMain,10,false,MX,MY);
									objEvent->iLayer=-1;
									EventDX=dx;
								};
								HR->CallEncounter((pFirstQuest+HR->arQuest[i].Index)->ipEncFailGiver,NULL,(pFirstProvince+HR->arQuest[i].ipProvince), HR->arQuest[i].ipSite, HR->arQuest[i].Index, 0, 0);
								HR->AbandonQuest(i);
								i--;
							};
						}
						else
						{
							if ((pFirstProvince+HR->arQuest[i].ipProvince)->ipPlayer!=0)
							{
								if (arPlayer[Player].Control==1)
								{
									blit(bmpGUI_LowPanel,bmpLowPanel,0,0,0,0,bmpGUI_LowPanel->w,bmpGUI_LowPanel->h);		
									textprintf_centre_ex(bmpLowPanel,Font1,156,79,COLOR_2,-1,"%d",arPlayer[Player].Gold);
									textprintf_centre_ex(bmpLowPanel,Font1,280,79,COLOR_2,-1,"%d",arPlayer[Player].Gem);
									dx=200;
									if ((pFirstProvince+HR->arQuest[i].ipProvince)->x+400>bmpMap->w)
									{dx=-200;};
									CentreMap((pFirstProvince+HR->arQuest[i].ipProvince)->x+dx,(pFirstProvince+HR->arQuest[i].ipProvince)->y);
									objEvent->PlaceOnMap((pFirstProvince+HR->arQuest[i].ipProvince)->x,(pFirstProvince+HR->arQuest[i].ipProvince)->y);
									objEvent->iLayer=8;
									objMain=ShowActiveObjects(objMain,10,false,MX,MY);
									objEvent->iLayer=-1;
									EventDX=dx;
								};
								HR->CallEncounter((pFirstQuest+HR->arQuest[i].Index)->ipEncFailGiver,NULL,(pFirstProvince+HR->arQuest[i].ipProvince), 0, HR->arQuest[i].Index, 0, 0);
								HR->AbandonQuest(i);
								i--;
							};
						};
					};
				};
			};
			GoldSpent+=Payment;
		};
	};

	GoldInc+=GoldInc*Perc/100;


	//Коррупция
	int Corrupt=(arPlayer[Player].iCorrupt+ProvNum)/10-arDifficulty[arPlayer[Player].iDifficulty].iCorruptStart;
	if ((arPlayer[Player].Control==1)&&(Corrupt>0))
	{
		if (Corrupt>arDifficulty[arPlayer[Player].iDifficulty].iCorruptCap)
		{Corrupt=arDifficulty[arPlayer[Player].iDifficulty].iCorruptCap;};
		Corrupt=Corrupt*(100+arPlayer[Player].GetBldParam1(51))/100;
		arPlayer[Player].iCalcCorrupt=Corrupt;
		GoldSpent+=GoldInc*Corrupt/100;
		GemSpent+=GemInc*Corrupt/100;
	};

	if (arPlayer[Player].Control==1)
	{
	for (int i=1; i<=iMaxResource; i++)
	{
		while(arPlayer[Player].GetResource(i)<0)
		{
			for (int j=1; j<=CurGame.iMaxPlayers; j++)
			{
				if ((j!=Player)&&(arPlayer[j].bInGame)&&(arPlayer[Player].Politic[j].Trade.bTrade))
				{
					bool bCancel=false;
					for (int k=0; k<16; k++)
					{
						if (arPlayer[Player].Politic[j].Trade.Stuff[k]==i)
						{bCancel=true;};
					};
					if (bCancel)
					{
						char Txt[500];
						sprintf(Txt,(pFirstDialog+828)->sText,arPlayer[j].GetName());
						(pFirstDialog+828)->Show(true,false,false,false,NULL,Txt);
						arPlayer[Player].Politic[j].Trade.Clear();
						arPlayer[j].Politic[Player].Trade.Clear();
					};
				};
			};
		};
	};

	if ((arPlayer[Player].Gold<0)&&(GoldInc-GoldSpent<0))
	{
		for (int j=1; ((j<=CurGame.iMaxPlayers)&&(GoldInc-GoldSpent<0)); j++)
		{
			if ((j!=Player)&&(arPlayer[j].bInGame)&&(arPlayer[Player].Politic[j].Trade.Gold>0))
			{					
				char Txt[500];
				sprintf(Txt,(pFirstDialog+828)->sText,arPlayer[j].GetName());
				(pFirstDialog+828)->Show(true,false,false,false,NULL,Txt);
				arPlayer[Player].Politic[j].Trade.Clear();
				arPlayer[j].Politic[Player].Trade.Clear();
			};
		};
	};
	if ((arPlayer[Player].Gem<0)&&(GemInc-GemSpent<0))
	{
		for (int j=1; ((j<=CurGame.iMaxPlayers)&&(GemInc-GemSpent<0)); j++)
		{
			if ((j!=Player)&&(arPlayer[j].bInGame)&&(arPlayer[Player].Politic[j].Trade.Gem>0))
			{					
				char Txt[500];
				sprintf(Txt,(pFirstDialog+828)->sText,arPlayer[j].GetName());
				(pFirstDialog+828)->Show(true,false,false,false,NULL,Txt);
				arPlayer[Player].Politic[j].Trade.Clear();
				arPlayer[j].Politic[Player].Trade.Clear();
			};
		};
	};
	};



	/*if (Random(CurGame.iCurrentTurn)>10)
	{
		if (Pirate1+Pirate2+Pirate3+Pirate4>0)
		{
			FileLog((pFirstProvince+13)->sName);
			delete (pFirstProvince+13);
		};
		if (bHidden)
		{
			if (CurGame.iGameType!=2)
			{
				FileLog((pFirstPlayerProfile+6)->sName);
				delete arPlayer[1].arHero[0];
			};
			if (GetWorldSize(0)>1)
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
	arPlayer[Player].Gold+=GoldInc-GoldSpent;
	arPlayer[Player].GoldInc=GoldInc-GoldSpent;
	arPlayer[Player].Gem+=GemInc-GemSpent;
	arPlayer[Player].GemInc=GemInc-GemSpent;
	
	WriteLog("Ход начался");
	if (arPlayer[Player].Control!=1)
	{
		arPlayer[Player].Gold+=5+arPlayer[Player].iDifficulty*8;
		arPlayer[Player].Gem+=CurGame.iCurrentTurn/20+arPlayer[Player].iDifficulty*2;
		arPlayer[Player].GoldInc+=5+arPlayer[Player].iDifficulty*8;
		arPlayer[Player].GemInc+=CurGame.iCurrentTurn/20+arPlayer[Player].iDifficulty*2;
	};
	Exiter(objMain, Prov, bmpLowPanel);
	iAntisleep=iAntisleepDelay;
	
	//MakeTurnAI(Player);
};

