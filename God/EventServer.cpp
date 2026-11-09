#include "Mapper.h"

int iEvPlayer, iEvHero;

//Исходя из приоритетов событий и приказов, заносит в iEvHero индекс первого в очереди на обработку героя (игрок-владелец заносится в iEvPlayer). 
//Возвращает индекс приоритета для выбранного героя, 0 если героев нет.
int NextHero()
{
	int Result=0, Init=0, 
		Prior=0, Prior1;	//10 - буран, 9 - одновременная атака осаждающего, 8 - снятие осады, 7 - одновременная атака вражеской провинции, 6 - атака вражеской провинции, 
							//5 - осада, 4 - одновременная атака нейтральных провинций, 3 - атака нейтральных провинций, 1 - нейтралы
	dcHero *HR;
	bool bPickHero;
	for (int Player=1; Player<=CurGame.iMaxPlayers; Player++)
	{
		if (arPlayer[Player].bInGame)
		{
			for (int Hero=0; Hero<MAX_HERO; Hero++)
			{
				if ((arPlayer[Player].arHero[Hero])&&(arPlayer[Player].arHero[Hero]->iDead<0)&&(arPlayer[Player].arHero[Hero]->ipEvent>=0))
				{
					HR=arPlayer[Player].arHero[Hero];
					Prior1=1;
					if (HR->ipEvent==1)
					{
						//Движение
						if ((pFirstProvince+HR->iTarget)->ipPlayer>0)
						{Prior1=8;}
						else
						{Prior1=5;};
						if ((pFirstProvince+HR->iTarget)->ipPlayer==Player)
						{Prior1=9;};
					}
					else
					{
						//Осада
						if (HR->pProvince->ipPlayer!=Player)
						{Prior1=7;};
					};
					//Снежный буран
					if (HR->ipEvent==20)
					{Prior1=10;};

					//Выбор героя исходя из приоритетов и инициативы
					if (Prior1==Prior)
					{
						if (Init<HR->CheckHeroBonus(999))
						{
							Init=HR->CheckHeroBonus(999);
							iEvPlayer=Player;
							iEvHero=Hero;
						}
						else
						{
							if ((Init==HR->CheckHeroBonus(999))&&(HR->iLevel>arPlayer[iEvPlayer].arHero[iEvHero]->iLevel))
							{
								Init=HR->CheckHeroBonus(999);
								iEvPlayer=Player;
								iEvHero=Hero;
							};
						};
					};
					if (Prior1>Prior)
					{
						Init=HR->CheckHeroBonus(999);
						iEvPlayer=Player;
						iEvHero=Hero;
						Prior=Prior1;
					};
				};
			};
		};
	};
	return Prior;
};



void HeroTasks ()
{
	dcHero *HR;
	dcProvince *PR;
	char STR[1000];
	iEvPlayer=0;
	while (NextHero()>0)
	{		
		HR=arPlayer[iEvPlayer].arHero[iEvHero];
		//Герой попал в буран
		if (HR->ipEvent==20)
		{			
			HR->ipEvent=-1;
			if (arPlayer[iEvPlayer].Control==1)
			{
				sprintf(STR,(pFirstDialog+503)->sText,PR->sName,HR->pstName->str);
				ShowEventMessage(iEvPlayer,0,HR->ipProvince,503,1,1,0,0,STR,NULL,true);
			};	
		};
		//Движение героя
		if (HR->ipEvent==1)
		{
			HR->ipEvent=-1;
			PR=(pFirstProvince+HR->iTarget);
			//Герой упёрся в барьер
			if ((PR->ipPlayer!=iEvPlayer)&&(PR->GetRitualBonus(1,12)>0))
			{
				if (arPlayer[iEvPlayer].Control==1)
				{
					sprintf(STR,(pFirstDialog+502)->sText,HR->pstName->str,PR->sName);
					ShowEventMessage(iEvPlayer,0,HR->iTarget,502,1,1,0,0,STR,NULL,true);
				};
				HR->iTarget=0;
			}
			else
			{
				ClearMoveMap();
				MovePlayer=iEvPlayer;
				WalkType=1+HR->CheckSkill(26);			
				WalkSpeed=HR->GetMobility();
				MoveMap(HR->pProvince->iNumber,-1,WalkSpeed,0,0);
				MovePath(HR->pProvince->iNumber,1,NULL,HR->iTarget,WalkSpeed,0,0,0);					
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
					if ((PR->ipPlayer==iEvPlayer)&&(PR->GetSiegeIndex()<0))
					{
						HR->pProvince=PR;
						HR->ipProvince=PR->iNumber;
						if (HR->iTarget==iComes)
						{
							HR->iTask=0;
							HR->iTarget=0;
						};
					}
					else
					{
						int iFrom=0;						
						HR->iTask=0;
						HR->iTarget=0;
						P1=GoPath;
						while (P1)
						{
							if ((P1->y==1)&&((pFirstProvince+P1->x)->ipTerrain>0)&&((pFirstProvince+P1->x)->ipTerrain<7)&&((pFirstProvince+P1->x)->ipPlayer==iEvPlayer)&&(((pFirstProvince+P1->x)->GetSiegeIndex()<0)||((pFirstProvince+P1->x)->GetSiegeIndex()==iEvPlayer)))
							{iFrom=P1->x;};
							P1=P1->pPrev;
						};
						if (iFrom>0)
						{
							HR->pProvince=(pFirstProvince+iFrom);
							HR->ipProvince=iFrom;
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

			};
		};


	};
};