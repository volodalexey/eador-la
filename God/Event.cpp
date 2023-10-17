#include "Windower.h"

bool IsHeroInGame(dcHero * Hero)
{
	for (int Player=1; Player<=CurGame.iMaxPlayers; Player++)
	{
		if (arPlayer[Player].bInGame)
		{
			for (int i=0; i<MAX_HERO; i++)
			{
				if (arPlayer[Player].arHero[i]==Hero)
				{return true;};
			};

		};
	};
	return false;
};

dcPolitics::dcPolitics()
{Clear();};

void dcPolitics::Clear()
{
	Treaty=0;
	Pass=0;
	Attitude=0;
	Answer=0;
	CanKill=0;
	Power=0;
	Trade.Clear();
	Trade.Gold=0;
	Message.Clear();
	for (int j=0; j<4; j++)
	{
		arTraitor[j]=0;
	};
	for (int j=0; j<8; j++)
	{
		arTalk[j][0]=0;
		arTalk[j][1]=0;
	};
};

dcTrade::dcTrade()
{Clear();};

void dcTrade::Clear()
{
	bTrade=false;
	Gold=0;
	Gem=0;
	for (int i=0; i<16; i++)
	{Stuff[i]=0;};
};

bool dcTrade::IsLegal(int Player, int Type)
{	
	if (Type==3)
	{
		if ((Gold>0)&&(arPlayer[Player].Gold<Gold))
		{return false;};
		if ((Gem>0)&&(arPlayer[Player].Gem<Gem))
		{return false;};
		int arItem[8];
		for (int i=0; i<8; i++)
		{arItem[i]=-1;};
		for (int i=0; i<8; i++)
		{
			if ((Stuff[i]>0)&&((pFirstProvince+Stuff[i])->ipPlayer!=Player))
			{return false;};
			if (Stuff[i+8]>0)
			{
				bool bFound=false;
				for (int j=0; ((j<TREASURY_SIZE)&&(!bFound)); j++)
				{
					if (arPlayer[Player].arTreasury[j][0]==Stuff[i+8])
					{
						bFound=true;
						for (int k=0; k<i; k++)
						{
							if (arItem[k]==j)
							{bFound=false;};
						};
						if (bFound)
						{arItem[i]=j;};
					};
				};
				if (!bFound)
				{return false;};
			};
		};
	};
	if (Type==2)
	{
		int arRes[10];
		for (int i=0; i<10; i++)
		{arRes[i]=0;};
		for (int i=0; i<16; i++)
		{
			if (Stuff[i]>0)
			{
				if (arPlayer[Player].GetResource(Stuff[i])>arRes[Stuff[i]])
				{
					arRes[Stuff[i]]++;
				}
				else
				{
					return false;
				};
			};
		};
	};
	return true;
};

bool dcTrade::IsFree()
{
	if (Gold>0)
	{return false;};
	if (Gem>0)
	{return false;};
	for (int i=0; i<16; i++)
	{
		if (Stuff[i]>0)
		{return false;};
	};
	return true;
};

dcMessage::dcMessage()
{Clear();};

void dcMessage::Clear()
{
	Type=0;
	Treaty=0;
	Trade.Clear();
	Trade1.Clear();	
	for (int i=0; i<8; i++)
	{arDurability[i]=0;};
	bDeal=false;
	sprintf(Text,"");
};

dcEvent::dcEvent(int Indx, dcEvent** Own, int Dialog, int Play1, int Play2, dcHero *Her1, dcHero *Her2, dcProvince *Pr1, dcProvince *Pr2, dcEvent *Next)
{
	Index=Indx;
	Owner=Own;
	ipDialog=Dialog;
	Player1=Play1;
	Player2=Play2;
	Hero1=Her1;
	Hero2=Her2;
	Prov1=Pr1;
	Prov2=Pr2;
	pNext=Next;
	if (pNext)
	{pNext->pPrev=this;};
	pPrev=NULL;
	pTree=NULL;
	if (Own)
	{*Owner=this;};
//	if ((Own>0)&&(!arPlayer[Own].pEvent))		
//	{arPlayer[Own].pEvent=this;};
};

dcEvent::~dcEvent()
{
	if (bGameInProcess)
	{
	if (pPrev)
	{pPrev->pNext=pNext;};
	if (pNext)
	{pNext->pPrev=pPrev;};
	if ((Owner)&&(*Owner==this))
	{*Owner=pNext;};
	};
//	if ((Owner>0)&&(arPlayer[Owner].pEvent==this))	
//	{arPlayer[Owner].pEvent=pNext;};
};


void dcEvent::ChangeOwner(dcEvent** NewOwn)
{
	if (pPrev)
	{pPrev->pNext=pNext;};
	if (pNext)
	{pNext->pPrev=pPrev;};
	if ((Owner)&&(*Owner==this))
	{*Owner=pNext;};
	pNext=*NewOwn;
	pPrev=NULL;
	Owner=NewOwn;
	*Owner=this;
	if (pNext)
	{pNext->pPrev=this;};
};

void dcEvent::Exclude()
{
	if (pPrev)
	{pPrev->pNext=pNext;};
	if (pNext)
	{pNext->pPrev=pPrev;};
	if ((Owner)&&(*Owner==this))
	{*Owner=pNext;};
	Owner=NULL;
	pPrev=NULL;
	pNext=NULL;
};

dcEvent* EnterEnemyLand(int Player1, int Player2, dcHero *Hero1, dcHero *Hero2, dcProvince *Prov1, dcProvince *Prov2)
{
	dcEvent *EV;
	if (Prov2->ipPlayer==0)
	//Генерация события при атаке героем нейтральной провинции
	{
		EV=new dcEvent(1,&arPlayer[Player1].pEvent,(pFirstProvinceType+Prov2->ipType)->arGuard[Prov2->iDefender][2],Player1,0,Hero1,Prov2->pGuard,Prov1,Prov2,arPlayer[Player1].pEvent);
		return EV;
	//	arPlayer[Player1].pEvent=EV;
	}
	else
	{
		//Проверка наличия осаждающего отряда в провинции
		int Siege=Prov2->GetSiegeIndex();
		if (Siege>=0)
		{
			if (Siege==Player1)
			{
				Hero1->ipProvince=Prov2->iNumber;
				Hero1->pProvince=Prov2;
			}
			else
			{
				/*if (((arPlayer[Player1].Control==1)&&((arPlayer[Siege].Control==1)||(arPlayer[Player2].Control==1)))||
					((arPlayer[Siege].Control==1)&&(arPlayer[Player2].Control==1)&&(arPlayer[Siege].Num>arPlayer[Player2].Num)))*/
				if ((arPlayer[Player1].Control==1)&&(arPlayer[Siege].Control==1))
				{											
					EV=new dcEvent(15,NULL,0,Player1,Siege,Hero1,NULL,Prov1,Prov2,NULL);					
					bHourGlass=false;
					bMessageFor=true;
					int Attack=EV->Call(0);
					bMessageFor=false;
					delete EV;
					RunHourGlass(148, HOUR_SHIFT);
				//	if (Attack==1)
				//	{EnterEnemyLand(Player1, Player2, Hero1, Hero2, Prov1, Prov2);};
				}
				else
				{
					if (arPlayer[Siege].Control==1)
					{
						EV=new dcEvent(15,&arPlayer[Siege].pEvent,0,Player1,Siege,Hero1,NULL,Prov1,Prov2,arPlayer[Siege].pEvent);
						return EV;
					//	arPlayer[Siege].pEvent=EV;
					}
					else
					{
						EV=new dcEvent(15,&arPlayer[Player1].pEvent,0,Player1,Siege,Hero1,NULL,Prov1,Prov2,arPlayer[Player1].pEvent);
						return EV;
			//			arPlayer[Player1].pEvent=EV;
					};
				};
			};
		}
		else
		{
			if ((arPlayer[Player1].Control==1)&&(arPlayer[Player2].Control==1))
			{
				EV=new dcEvent(10,NULL,0,Player1,Player2,Hero1,NULL,Prov1,Prov2,NULL);
				bHourGlass=false;
				bMessageFor=true;
				EV->Call(0);
				bMessageFor=false;
				RunHourGlass(148, HOUR_SHIFT);
				delete EV;
			}
			else
			{								
				if (arPlayer[Player2].Control==1)
				{
					EV=new dcEvent(10,&arPlayer[Player2].pEvent,0,Player1,Player2,Hero1,NULL,Prov1,Prov2,arPlayer[Player2].pEvent);
					return EV;
				//	arPlayer[Player2].pEvent=EV;
				}
				else
				{
					EV=new dcEvent(10,&arPlayer[Player1].pEvent,0,Player1,Player2,Hero1,NULL,Prov1,Prov2,arPlayer[Player1].pEvent);
					return EV;
			//		arPlayer[Player1].pEvent=EV;					
				};
			};
		};
	};
	return NULL;
};

void EventServer(int Player)
{
	dcEvent *EV=arPlayer[Player].pEvent, *EV1;
	while (EV)
	{		
		if (EV->Index==1)
		{
			dcProvince *PR=EV->Prov2;
			dcHero *pHero=EV->Hero1;			
			//Генерация события при попытке войти в заблокированную провинцию
			/*dcRitual *RR=pRitual;
			bool bBlocked=false;
			while (RR)
			{
				if ((RR->ipProvince==PR->iNumber)&&(RR->ipPlayer!=Player))
				{
					for (int i=0; i<MAX_RITUAL_EFFECT; i++)
					{
						if ((pFirstRitual+RR->Index)->arEffect[i][0]==12)
						{bBlocked=true;};
					};
				};
				RR=RR->pNext;
			};
			if (bBlocked)*/
			if ((PR->ipPlayer!=Player)&&(PR->GetRitualBonus(1,12)>0))
			{			
				EV->Index=20;
				EV->ipDialog=502,					
				sprintf(EV->sMessage,(pFirstDialog+502)->sText,pHero->pstName->str,PR->sName);
				EV=EV->pNext;
			}
			else
			{			
			//Проверка конфликта при одновременной атаке провинции разными игроками
				dcEvent *EVBattle=NULL;
				//Составление списка событий атаки данной провинции другими игроками
				for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
				{
					if (PL!=Player)
					{							
						dcEvent *EVtmp=arPlayer[PL].pEvent;//, *EVtmp1;
						while (EVtmp)
						{	
							if ((EVtmp->Index==1)&&(EVtmp->Prov2==PR))
							{
								EV1=EVtmp->pNext;
								EVtmp->ChangeOwner(&EVBattle);
								EVtmp=EV1;
							}
							else
							{
								EVtmp=EVtmp->pNext;
							};
						};
					};
				};
				if (EVBattle)
				{
					/*clear_keybuf();
					textprintf(screen,Font1,0,0,COLOR_2,"Найден оппонент");
					readkey();*/
					//Добавление в список событий текущего игрока						
					dcEvent *EVtmp=arPlayer[Player].pEvent;
					while (EVtmp)
					{	
						if ((EVtmp->Index==1)&&(EVtmp->Prov2==PR))
						{
							if (EVtmp==EV)
							{EV=EVtmp->pNext;};
							EV1=EVtmp->pNext;
							EVtmp->ChangeOwner(&EVBattle);
							EVtmp=EV1;
						}
						else
						{
							EVtmp=EVtmp->pNext;
						};
					};						
					//Отсев слабейших героев АИ и удаление их из общего списка						
					dcEvent *EVStrong=NULL;
					EVtmp=EVBattle;
					while (EVtmp)
					{
						if (arPlayer[EVtmp->Player1].Control!=1)
						{
							EV1=EVtmp->pNext;
							EVtmp->Exclude();								
							if (!EVStrong)
							{EVStrong=EVtmp;}
							else
							{
								if (CompareStrength(EVtmp->Hero1,EVStrong->Hero1)>100)
								{
									delete EVStrong;										
									EVStrong=EVtmp;										
								};
							};
							EVtmp=EV1;
						}
						else
						{
						//	EVtmp1=EVtmp;
							EVtmp=EVtmp->pNext;
						};									
					};
					//Спарринг героев живых игроков
					/*textprintf(screen,Font1,0,0,COLOR_2,"Поиск соперников");
					readkey();*/
					int arPL[MAX_PLAYERS];
					int PLCount=10;						
					dcEvent *EVHero1=NULL, *EVHero2=NULL;
					while (PLCount>1)
					{
						PLCount=0;
						for (int i=0; i<MAX_PLAYERS; i++)
						{arPL[i]=0;};
						EVtmp=EVBattle;
						while (EVtmp)
						{
							if ((arPL[EVtmp->Player1]==0)&&(arPlayer[EVtmp->Player1].Control==1))
							{
								arPL[EVtmp->Player1]++;
								PLCount++;
							};
							EVtmp=EVtmp->pNext;
						};
						if ((EVHero1)&&(arPL[EVHero1->Player1]==0))
						{PLCount++;};
						if (PLCount>0)
						{
							//Поиск пары с наименьшей инициативой
							EVtmp=EVBattle;
							while (EVtmp)
							{
								if (!EVHero1)
								{EVHero1=EVtmp;}
								else
								{
									if (!EVHero2)
									{
										if (EVtmp->Player1!=EVHero1->Player1)
										{EVHero2=EVtmp;};
									}
									else
									{
										if (EVtmp->Player1==EVHero2->Player1)
										{
											if (EVtmp->Hero1->CheckHeroBonus(999)<EVHero2->Hero1->CheckHeroBonus(999))
											{EVHero2=EVtmp;};
										}
										else
										{
											if (EVtmp->Player1==EVHero1->Player1)
											{
												if (EVtmp->Hero1->CheckHeroBonus(999)<EVHero1->Hero1->CheckHeroBonus(999))
												{EVHero1=EVtmp;};
											}
											else
											{
												if (EVHero1->Hero1->CheckHeroBonus(999)<EVHero2->Hero1->CheckHeroBonus(999))
												{
													if (EVtmp->Hero1->CheckHeroBonus(999)<EVHero2->Hero1->CheckHeroBonus(999))
													{EVHero2=EVtmp;};
												}
												else
												{
													if (EVtmp->Hero1->CheckHeroBonus(999)<EVHero1->Hero1->CheckHeroBonus(999))
													{EVHero1=EVtmp;};
												};
											};
										};
									};
								};									
								EVtmp=EVtmp->pNext;									
							};							
							//Исключение сражающихся из списка
							EVtmp=EVBattle;
							while (EVtmp)
							{
								if ((EVtmp==EVHero1)||(EVtmp==EVHero2))
								{
									EV1=EVtmp->pNext;
									EVtmp->Exclude();
									EVtmp=EV1;
								}
								else
								{
									EVtmp=EVtmp->pNext;
								};
							};
							if (PLCount>1)
							{
								if (EVStrong)
								{
									if ((EVStrong->Hero1->CheckHeroBonus(999)<EVHero1->Hero1->CheckHeroBonus(999))||(EVStrong->Hero1->CheckHeroBonus(999)<EVHero2->Hero1->CheckHeroBonus(999)))
									{
										if ((CompareStrength(EVStrong->Hero1,EVHero1->Hero1)<100)||(CompareStrength(EVStrong->Hero1,EVHero2->Hero1)<100))
										{
											delete EVStrong;
											EVStrong=NULL;
										};
									};
								};
								/*textprintf(screen,Font1,0,0,COLOR_2,"Вызов события");
								readkey();*/
								dcEvent *EVCombat=new dcEvent(16,NULL,832,EVHero1->Player1,EVHero2->Player1,EVHero1->Hero1,EVHero2->Hero1,EVHero1->Prov2,EVHero2->Hero1->pProvince,NULL);
								EVCombat->iParam1=EVHero1->Prov1->iNumber+EVHero2->Prov1->iNumber;
								bHourGlass=false;
								bMessageFor=true;
								int Winner=EVCombat->Call(0);
								bMessageFor=false;
								RunHourGlass(148, HOUR_SHIFT);
								switch (Winner)
								{
								case 0:
									delete EVHero1;
									EVHero1=EVHero2;
									EVHero2=NULL;
									break;
								case 1:
									delete EVHero2;									
									EVHero2=NULL;
									break;
								case -1:
									delete EVHero1;
									delete EVHero2;
									EVHero1=NULL;
									EVHero2=NULL;
									break;
								};
								delete EVCombat;
							};
						};
					};
				/*	clear_keybuf();
					textprintf(screen,Font1,0,0,COLOR_2,"Stopped...");
					readkey();*/
					//Создание события для победителя
					if (EVHero1)
					{
						while (EVHero1)
						{
							if (EVStrong)
							{
								if (arPlayer[EVHero1->Player2].Control==1)
								{
									dcEvent *EVCombat=new dcEvent(16,NULL,832,EVHero1->Player1,EVStrong->Player1,EVHero1->Hero1,EVStrong->Hero1,EVHero1->Prov2,EVStrong->Prov2,NULL);
									bHourGlass=false;
									bMessageFor=true;
									int Winner=EVCombat->Call(0);
									bMessageFor=false;
									RunHourGlass(148, HOUR_SHIFT);
									switch (Winner)
									{
									case 0:										
										EnterEnemyLand(EVStrong->Player1,EVStrong->Prov2->ipPlayer,EVStrong->Hero1,EVStrong->Hero2,EVStrong->Prov1,EVStrong->Prov2);
										break;
									case 1:
										EnterEnemyLand(EVHero1->Player1,EVHero1->Prov2->ipPlayer,EVHero1->Hero1,EVHero1->Hero2,EVHero1->Prov1,EVHero1->Prov2);
										break;									
									};									
									delete EVCombat;
								}
								else
								{
									dcEvent *EVCombat=new dcEvent(16,&arPlayer[EVHero1->Player1].pEvent,832,EVHero1->Player1,EVStrong->Player1,EVHero1->Hero1,EVStrong->Hero1,EVHero1->Prov2,EVStrong->Prov2,arPlayer[EVHero1->Player1].pEvent);
									EVCombat->iParam1=EVHero1->Prov1->iNumber+EVStrong->Prov1->iNumber;
								};
								delete EVStrong;
							}
							else
							{
								EnterEnemyLand(EVHero1->Player1,EVHero1->Prov2->ipPlayer,EVHero1->Hero1,EVHero1->Hero2,EVHero1->Prov1,EVHero1->Prov2);
							};
							delete EVHero1;
							EVHero1=EVBattle;
						};
					}
					else
					{
						if (EVStrong)
						{
							EnterEnemyLand(EVStrong->Player1,EVStrong->Player2,EVStrong->Hero1,EVStrong->Hero2,EVStrong->Prov1,EVStrong->Prov2);
							delete EVStrong;
						};
					};
				}
				else
				{
					dcEvent *EVtmp=EV;
					EV=EV->pNext;
					EnterEnemyLand(EVtmp->Player1,EVtmp->Player2,EVtmp->Hero1,EVtmp->Hero2,EVtmp->Prov1,EVtmp->Prov2);
					delete EVtmp;
				};
			};
		}
		else
		{
			//Штурм игроком игрока
			if  ((arPlayer[EV->Player1].Control==1)&&(arPlayer[EV->Player2].Control==1)&&((EV->Index==11)||(EV->Index==12)))
			{
				bHourGlass=false;
				bMessageFor=true;
				EV->Call(0);
				bMessageFor=false;
				RunHourGlass(148, HOUR_SHIFT);
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
};

int dcEvent::Call(int DX)
//Возможные типы событий, в зависимости от Index:
// 1 - вход в нейтральную провинцию
// 2 - вход в охраняемый сайт
// 3 - исследование провинции
// 4 - вход в сайт без охраны
// 5 - атака стражника своей провинции
// 6 - грабёж обычный
// 7 - грабёж финальный
//10 - атака чужой провинции
//11 - осада, укрепления разрушены
//12 - осада, есть брешь
//13 - осада, нет бреши
//14 - отака осаждающего отряда нейтралов
//15 - атака осаждающего отряда другого игрока
//16 - атака одной провинции двумя героями
//17 - захват крепости без боя при уходе гарнизона
//18 - сдача крепости без боя при уходе гарнизона
//20 - выдача диалога
{
	if (!arPlayer[Player1].bInGame)
	{return 0;};
	int iScGold;
	if (Tutorial.Map)
	{
		if ((Index==1)&&(Tutorial.buttCastle==0)&&(Prov2->iNumber!=21))
		{			
			Hero1->pProvince=Prov2;
			Hero1->ipProvince=Prov2->iNumber;
			return 0;
		};
	};

	if (Index==15)
	{
		if (Prov2->GetSiegeIndex()==0)
		{Index=14;};
		if (Prov2->GetSiegeIndex()<0)
		{Index=10;};
	};
	StopLog("Индекс: ",Index);
	if ((Index==1)||(Index==10))
	{
		if (Prov2->ipPlayer==0)
		{
			if (Index==10)
			{ipDialog=(pFirstProvinceType+Prov2->ipType)->arGuard[Prov2->iDefender][2];};
			Hero2=Prov2->pGuard;
			Index=1;
		}
		else
		{Index=10;};
		if ((Prov2->ipPlayer==Player1)||(Prov2->GetSiegeIndex()==Player1))
		{
			Hero1->pProvince=Prov2;
			Hero1->ipProvince=Prov2->iNumber;
			return 1;
		};
	};	
	char txtLog[200];
	int Result=0;
	if ((IsHeroInGame(Hero1))&&(Hero1->iDead<0))
	{
	//Атака чужой провинции
	if (Index==10)
	{
		if (Prov2->ipPlayer==Player1)
		{
			if ((IsHeroInGame(Hero1))&&(Hero1->iDead<0))
			{
				Hero1->pProvince=Prov2;
				Hero1->ipProvince=Prov2->iNumber;
			};
		}
		else
		{
		sprintf(txtLog,"Герой %s игрока %d атакует провинцию %s          ",Hero1->pstName->str,Player1,Prov2->sName);
		DFileLog(txtLog);
		bool Fight=true;
		if (arPlayer[Player1].Control==1)
		{
			if (arPlayer[Player1].Politic[Player2].Treaty==2)
			{
				/*arPlayer[Player2].Politic[0].Message.Clear();
				arPlayer[Player2].Politic[0].Message.Type=6;
				sprintf(arPlayer[Player2].Politic[0].Message.Text,(pFirstMessage+20)->msg,arPlayer[Player2].GetName());*/
				iMessageFor=Player1;
				char DTxt[200];
				sprintf(DTxt,(pFirstDialog+1195)->sText,arPlayer[Player2].GetName());
				if ((pFirstDialog+1195)->Show(1,1,0,0,arPlayer[Player2].GetAvatar(),DTxt)==2)
			//	if (WindowMessage(0,Player2)==1)
				{Fight=false;}
				else
				{
					arPlayer[Player1].DeclareWar(Player2);					
					arPlayer[Player2].Politic[Player1].Attitude-=30;
					arPlayer[Player2].Politic[Player1].arTraitor[0]++;
					arPlayer[Player2].Politic[Player1].Answer=29;
					for (int i=1; i<=CurGame.iMaxPlayers; i++)
					{
						if ((i!=Player1)&&(i!=Player2)&&(arPlayer[i].bInGame))
						{
							arPlayer[i].Politic[Player1].Attitude-=3;
						};
					};
				};
			};
			if (arPlayer[Player1].Politic[Player2].Treaty==3)
			{
				/*arPlayer[Player2].Politic[0].Message.Clear();
				arPlayer[Player2].Politic[0].Message.Type=6;				
				sprintf(arPlayer[Player2].Politic[0].Message.Text,(pFirstMessage+21)->msg,arPlayer[Player2].GetName());
				if (WindowMessage(0,Player2)==1)*/
				iMessageFor=Player1;
				char DTxt[200];
				sprintf(DTxt,(pFirstDialog+1196)->sText,arPlayer[Player2].GetName());
				if ((pFirstDialog+1196)->Show(1,1,0,0,arPlayer[Player2].GetAvatar(),DTxt)==2)
				{Fight=false;}
				else
				{
					if ((CurGame.iGameType==2)&&(arPlayer[Player2].ipProfile==3)&&(Campaign.arDialog[103]>0))
					{
						Campaign.arCampPlayer[3].iTreaty=0;
						Campaign.arDialog[103]=0;
						Campaign.arDialog[104]=2;	
						arPlayer[Player2].Politic[Player1].Attitude-=450;
					};
					arPlayer[Player1].DeclareWar(Player2);
					arPlayer[Player2].Politic[Player1].Attitude-=60;
					arPlayer[Player2].Politic[Player1].arTraitor[1]++;
					arPlayer[Player2].Politic[Player1].Answer=29;
					for (int i=1; i<=CurGame.iMaxPlayers; i++)
					{
						if ((i!=Player1)&&(i!=Player2)&&(arPlayer[i].bInGame))
						{
							arPlayer[i].Politic[Player1].Attitude-=6;
						};
					};
				};
			};
			if (Fight)
			{
			if (!Prov2->pGuard)
			{
				Fight=false;
				for (int i=0; i<MAX_HERO; i++)
				{
					if ((arPlayer[Player2].arHero[i])&&(arPlayer[Player2].arHero[i]->pProvince==Prov2))
					{Fight=true;};
				};
			};
			if (!Fight)
			{
				sprintf(sMessage,(pFirstDialog+371)->sText,Hero1->pstName,Prov2->sName,arPlayer[Prov2->ipPlayer].GetName());
				DialogDX=DX;
				iMessageFor=Player1;
				/*if (Hero1->CheckSkill(24)>0)
				{
					iScouting=SCOUTING_PRICE;
					if (Hero1->CheckSkill(24)>=5)
					{iScouting/=2;};
				};*/
				int Answer=(pFirstDialog+371)->Show(true,true,false,false,NULL,sMessage);
				/*if (Answer==3)
				{
					Hero2->MakeScoutingText();
					Answer=(pFirstDialog+371)->Show(true,true,false,false,NULL,stScouting);
				};*/
				if (Answer==1)
				{					
					bool bCastle=false;
					if (Prov2->pGarrison)
					{
						for (int i=0; i<MAX_SQUAD; i++)
						{
							if (Prov2->pGarrison->pSquad[i])
							{bCastle=true;};
						};
					};
					if (bCastle)
					{
						sprintf(sMessage,(pFirstDialog+372)->sText,Prov2->sName);
						DialogDX=DX;
						int Answer=(pFirstDialog+372)->Show(true,true,false,false,NULL,sMessage);
						if (Answer==1)
						{							
							Hero1->pProvince=Prov2;
							Hero1->ipProvince=Prov2->iNumber;
						};
					}
					else
					{
						Prov2->Conquer(Player1,Hero1);						
					};
				};
			};
		};
		};
		if (Fight)
		{
			if (Prov2->UnderAttack(Hero1,DX)==1)
			{
				bool bCastle=false;
				if (Prov2->pGarrison)
				{
					for (int i=0; i<MAX_SQUAD; i++)
					{
						if (Prov2->pGarrison->pSquad[i])
						{bCastle=true;};
					};
				};
				if (bCastle)
				{
					if (Hero1->iDead<0)
					{
						if (arPlayer[Player1].Control==1)
						{
							sprintf(sMessage,(pFirstDialog+372)->sText,Prov2->sName);
							DialogDX=DX;
							iMessageFor=Player1;
							int Answer=(pFirstDialog+372)->Show(true,true,false,false,NULL,sMessage);
							if (Answer==1)
							{							
								Hero1->pProvince=Prov2;
								Hero1->ipProvince=Prov2->iNumber;
							};
						}
						else
						{											
							Hero1->pProvince=Prov2;
							Hero1->ipProvince=Prov2->iNumber;
							if (arPlayer[Player2].Control==1)
							{
								iMessageFor=Player2;
								sprintf(sMessage,(pFirstDialog+1238)->sText,Prov2->sName);
								DialogDX=DX;
								(pFirstDialog+1238)->Show(true,true,false,false,NULL,sMessage);
							};
						};
					};
				}
				else
				{
					Prov2->Conquer(Player1,Hero1);					
					if (arPlayer[Player2].Control==1)
					{
						iMessageFor=Player2;
						sprintf(sMessage,(pFirstDialog+382)->sText,Prov2->sName);
						DialogDX=DX;
						(pFirstDialog+382)->Show(true,true,false,false,NULL,sMessage);
					};
				};
			};

		};
		};
	};	

	//Одновременная атака одной провинции двумя игорками
	if (Index==16)
	{
		Result=100;
		if ((IsHeroInGame(Hero1))&&(Hero1->iDead<0)&&(IsHeroInGame(Hero2))&&(Hero2->iDead<0))
		{
			if (arPlayer[Player1].Control==1)
			{
				char sArmy[500];
				int GuardsNum=Hero2->MakeDescription(sArmy);
				sprintf(sMessage,(pFirstDialog+832)->sText,Hero1->pstName->str,Prov1->sName,arPlayer[Player2].GetName(),Hero2->pstName->str,GuardsNum,sArmy);
				DialogDX=DX;
				iMessageFor=Player1;
				int Answer;
				if (Hero1->CheckSkill(24)>0)
				{
					iScouting=SCOUTING_PRICE;
					if (Hero1->CheckSkill(24)>=5)
					{iScouting/=2;};
					iScGold=iScouting;
					if (iScouting<=arPlayer[Player1].Gold)
					{Answer=(pFirstDialog+832)->Show(true,true,true,false,NULL,sMessage);}
					else
					{Answer=(pFirstDialog+832)->Show(true,true,false,false,NULL,sMessage);};
					if (Answer==3)
					{
						arPlayer[Player1].Gold-=iScGold;
						Hero2->MakeScoutingText();
						Answer=(pFirstDialog+832)->Show(true,true,false,false,NULL,stScouting);
					};
				}
				else
				{Answer=(pFirstDialog+832)->Show(true,true,false,false,NULL,sMessage);};
				if (Answer==2)
				{Result=0;};
			}
			else
			{
				if (CompareStrength1(Hero1,Hero2)<100)
				{Result=0;};
			};
			if (Result==100)
			{
				if (arPlayer[Player2].Control==1)
				{
					char sArmy[500];
					int GuardsNum=Hero1->MakeDescription(sArmy);
					sprintf(sMessage,(pFirstDialog+832)->sText,Hero2->pstName->str,Prov1->sName,arPlayer[Player1].GetName(),Hero1->pstName->str,GuardsNum,sArmy);
					DialogDX=DX;
					iMessageFor=Player2;
					int Answer;
					if (Hero2->CheckSkill(24)>0)
					{
						iScouting=SCOUTING_PRICE;
						if (Hero2->CheckSkill(24)>=5)
						{iScouting/=2;};
						iScGold=iScouting;
						if (iScouting<=arPlayer[Player2].Gold)
						{Answer=(pFirstDialog+832)->Show(true,true,true,false,NULL,sMessage);}
						else
						{Answer=(pFirstDialog+832)->Show(true,true,false,false,NULL,sMessage);};
						if (Answer==3)
						{
							arPlayer[Player2].Gold-=iScGold;
							Hero1->MakeScoutingText();
							Answer=(pFirstDialog+832)->Show(true,true,false,false,NULL,stScouting);
						};
					}
					else
					{Answer=(pFirstDialog+832)->Show(true,true,false,false,NULL,sMessage);};
					if (Answer==2)					
					{Result=1;};
				}
				else
				{
					if (CompareStrength1(Hero2,Hero1)<100)
					{
						Result=1;
						DialogDX=DX;
						(pFirstDialog+833)->Show(true,true,false,false,NULL,NULL);
					};
				};
			};
			if (Result==100)
			{
				Result=InitBattle(Prov1,NULL,Hero1,Hero2,0,true);
				if (Result==1)
				{
					Hero2->Death(0);
					if (Hero1->pSquad[0]->iLife==0)
					{
						Hero1->Death(1);
					};
				};
				if (Result==0)
				{
					Hero1->Death(0);
					if (Hero2->pSquad[0]->iLife==0)
					{
						Hero2->Death(1);
					};
				};
				if (Result<0)
				{
					if (Hero1->pSquad[0]->iLife==0)
					{
						Hero1->Death(Result);
					};
					if (Hero2->pSquad[0]->iLife==0)
					{
						Hero2->Death(Result);
					};
				};
			
			};
		}
		else
		{
			if ((IsHeroInGame(Hero1))&&(Hero1->iDead<0))
			{Result=1;};
			if ((IsHeroInGame(Hero2))&&(Hero2->iDead<0))
			{Result=0;};
			if (Result==100)
			{Result=-1;};
		};
	};

//Атака осаждающего отряда
	if (Index==15)
	{
		Player2=Prov2->GetSiegeIndex();
		if (Player1==Player2)
		{
			Hero1->pProvince=Prov2;
			Hero1->ipProvince=Prov2->iNumber;
		}
		else
		{
		bool bCycle=true;
		while (bCycle)
		{
			if (arPlayer[Player2].Control==1)
			//Игрока
			{
				char sArmy[500];
				int iAtt=Hero1->MakeDescription(sArmy);
				sprintf(sMessage,(pFirstDialog+384)->sText,Prov2->sName,Hero1->pstName->str,arPlayer[Player1].GetName(),iAtt,sArmy);
				DialogDX=DX;
				iMessageFor=Player2;
				(pFirstDialog+384)->Show(true,true,false,false,NULL,sMessage);
				int Count=0, iHero=0;
				for (int i=0; i<MAX_HERO; i++)
				{
					if ((arPlayer[Player2].arHero[i])&&(arPlayer[Player2].arHero[i]->pProvince==Prov2)&&(arPlayer[Player2].arHero[i]->iDead<0))
					{
						Count++;
						iHero=i;
					};
				};
				if (Count>1)
				{					
					int DefCount=0;
					int arDef[9];		
					for (int i=0; i<MAX_HERO; i++)
					{
						if ((arPlayer[Player2].arHero[i])&&(arPlayer[Player2].arHero[i]->pProvince==Prov2)&&(arPlayer[Player2].arHero[i]->iDead<0))
						{			
							arDef[DefCount]=i;
							DefCount++;
						};
					};
					int Select=-1;		
				/*	UI_Mask arMask[9];
					UI_TextBox txtChoose;
					BITMAP *bmpBack=create_bitmap(1024,768);
					blit(screen1,bmpBack,0,0,0,0,1024,768);
					BITMAP *bmpWork=create_bitmap(bmpGUI_DefChoose->w,145+70*DefCount);
					int x=512-bmpWork->w/2, y=384-bmpWork->h/2;
					G_Object *objMain=new G_Object(NULL,x,y,1,bmpWork);
					objMain=new G_Object(objMain,0,0,0,bmpBack);
					objMain=txtChoose.Init(x+9,y+9,235,90,2,COLOR_2,Font1,objMain);
					txtChoose.Activate((pFirstMessage+16)->msg);
					blit(bmpGUI_DefChoose,bmpWork,0,0,0,0,bmpGUI_DefChoose->w,130);
					for (int i=0; i<DefCount; i++)
					{
						blit(bmpGUI_DefChoose,bmpWork,0,130,0,130+i*70,bmpGUI_DefChoose->w,70);
						draw_sprite(bmpWork,bmpGUI_RazePanel,10,135+i*70);
						draw_sprite(bmpWork,arPlayer[Player2].arHero[arDef[i]]->smPic,18,142+i*70);
						textprintf_centre_ex(bmpWork,Font1,157,147+i*70,COLOR_2,-1,arPlayer[Player2].arHero[arDef[i]]->pstName->str);
						objMain=arMask[i].Init(x+6,y+128+i*70,1,bmpGUI_BuildMask,bmpGUI_BuildSelect,objMain,2);
					};
					blit(bmpGUI_DefChoose,bmpWork,0,360,0,130+DefCount*70,bmpGUI_DefChoose->w,15);
					set_mouse_sprite(bmpMousePointer[1]);*/


			UI_Mask arMask[9];
			int DD=68;
			UI_TextBox txtChoose;
			BITMAP *bmpBack=create_bitmap(1024,768);
			blit(screen1,bmpBack,0,0,0,0,1024,768);
			BITMAP *bmpWork=create_bitmap(bmpGUI_DefChoose->w,bmpGUI_DefChoose->h-DD+DD*DefCount);
			int x=512-bmpWork->w/2, y=384-bmpWork->h/2;
			G_Object *objMain=new G_Object(NULL,x,y,1,bmpWork);
			objMain=new G_Object(objMain,0,0,0,bmpBack);
			objMain=txtChoose.Init(x+25,y+30,254,90,2,COLOR_1,Font2,objMain);
			txtChoose.Activate((pFirstMessage+16)->msg);
			blit(bmpGUI_DefChoose,bmpWork,0,0,0,0,bmpGUI_DefChoose->w,131);
			for (int i=0; i<DefCount; i++)
			{
				blit(bmpGUI_DefChoose,bmpWork,0,131,0,131+i*DD,bmpGUI_DefChoose->w,DD);
				draw_sprite(bmpWork,arPlayer[Player2].arHero[arDef[i]]->smPic,20,141+i*DD);
				textprintf_centre_ex(bmpWork,Font1,180,146+i*DD,COLOR_1,-1,arPlayer[Player2].arHero[arDef[i]]->pstName->str);				
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
					iHero=arDef[Select];
				};
				Hero2=arPlayer[Player2].arHero[iHero];
				if (arPlayer[Player1].Control==1)
				{
					char sArmy[500];
					int iAtt=Hero2->MakeDescription(sArmy);
					sprintf(sMessage,(pFirstDialog+385)->sText,Hero1->pstName->str,Prov2->sName,arPlayer[Player2].GetName(),Hero2->pstName->str,iAtt,sArmy);
					DialogDX=DX;
					pAdvice=Hero1;
					iDialogButt=1;
					int Answer;
					if (Hero1->CheckSkill(24)>0)
					{
						iScouting=SCOUTING_PRICE;
						if (Hero1->CheckSkill(24)>=5)
						{iScouting/=2;};
						iScGold=iScouting;
						if (iScouting<=arPlayer[Player1].Gold)
						{Answer=(pFirstDialog+385)->Show(true,true,true,false,NULL,sMessage);}
						else
						{Answer=(pFirstDialog+385)->Show(true,true,false,false,NULL,sMessage);};
						if (Answer==3)
						{
							pAdvice=Hero1;
							iDialogButt=1;
							arPlayer[Player1].Gold-=iScGold;
							Hero2->MakeScoutingText();
							Answer=(pFirstDialog+385)->Show(true,true,false,false,NULL,stScouting);
						};
					}
					else
					{Answer=(pFirstDialog+385)->Show(true,true,false,false,NULL,sMessage);};					
					if (Answer==1)
					{
						int Win=InitBattle(Prov1,Prov2,Hero1,Hero2,0,true);
						if ((Win<1)||(Count<2))
						{bCycle=false;};
						if ((Win==1)&&(Count<2))
						{
							Hero1->pProvince=Prov2;
							Hero1->ipProvince=Prov2->iNumber;
						};
						if (Hero1->pSquad[0]->iLife==0)
						{
							bCycle=false;
							Hero1->Death(Win);
						};
						if (Hero2->pSquad[0]->iLife==0)
						{							
							if (Win==0)
							{Hero2->Death(1);}
							else
							{
								if (Win==1)
								{Hero2->Death(0);}
								else
								{Hero2->Death(Win);};
							};
						};
					}
					else
					{bCycle=false;};
				}
				else
				{
					if ((CompareStrength1(Hero1,Hero2)>100)||(arPlayer[Player1].ipProvince==Prov2->iNumber))
					{
						int Win=InitBattle(Prov1,Prov2,Hero1,Hero2,0,true);
						if ((Win<1)||(Count<2))
						{bCycle=false;};
						if ((Win==1)&&(Count<2))
						{
							Hero1->pProvince=Prov2;
							Hero1->ipProvince=Prov2->iNumber;
						};
						if (Hero1->pSquad[0]->iLife==0)
						{
							bCycle=false;
							Hero1->Death(Win);
						};
						if (Hero2->pSquad[0]->iLife==0)
						{							
							if (Win==0)
							{Hero2->Death(1);}
							else
							{
								if (Win==1)
								{Hero2->Death(0);}
								else
								{Hero2->Death(Win);};
							};
						};
					}
					else
					{
						DialogDX=DX;
						(pFirstDialog+373)->Show(true,true,false,false,NULL,NULL);
						bCycle=false;
					};
				};				
			}
			else
			//AI
			{
				int Count=0, iHero=-1;
				for (int i=0; i<MAX_HERO; i++)
				{
					int Power=0;
					if ((arPlayer[Player2].arHero[i])&&(arPlayer[Player2].arHero[i]->pProvince==Prov2)&&(arPlayer[Player2].arHero[i]->iDead<0))
					{
						Count++;
						if (iHero<0)
						{
							iHero=i;
							Power=CompareStrength1(arPlayer[Player2].arHero[i],Hero1);
						}
						else
						{
							int Power1=CompareStrength1(arPlayer[Player2].arHero[i],Hero1);
							if (Power1>Power)
							{
								iHero=i;
								Power=Power1;
							};
						};
					};
				};
				Hero2=arPlayer[Player2].arHero[iHero];
				if (arPlayer[Player1].Control==1)
				{
					char sArmy[500];
					int iAtt=Hero2->MakeDescription(sArmy);
					sprintf(sMessage,(pFirstDialog+385)->sText,Hero1->pstName->str,Prov2->sName,arPlayer[Player2].GetName(),Hero2->pstName->str,iAtt,sArmy);
					DialogDX=DX;
					pAdvice=Hero1;
					iDialogButt=1;
					int Answer=(pFirstDialog+385)->Show(true,true,false,false,NULL,sMessage);
					if (Answer==1)
					{
						int Win=InitBattle(Prov1,Prov2,Hero1,Hero2,0,true);
						if ((Win<1)||(Count<2))
						{bCycle=false;};
						if ((Win==1)&&(Count<2))
						{
							Hero1->pProvince=Prov2;
							Hero1->ipProvince=Prov2->iNumber;
						};
						if (Hero1->pSquad[0]->iLife==0)
						{
							bCycle=false;
							Hero1->Death(Win);
						};
						if (Hero2->pSquad[0]->iLife==0)
						{							
							if (Win==0)
							{Hero2->Death(1);}
							else
							{
								if (Win==1)
								{Hero2->Death(0);}
								else
								{Hero2->Death(Win);};
							};
						};
					}
					else
					{bCycle=false;};
				}
				else
				{
					if ((CompareStrength1(Hero1,Hero2)>100)||(arPlayer[Player1].ipProvince==Prov2->iNumber))
					{
						int Win=InitBattle(Prov1,Prov2,Hero1,Hero2,0,false);
						Hero1->NiB=0;
						if ((Win<1)||(Count<2))
						{bCycle=false;};
						if ((Win==1)&&(Count<2))
						{
							Hero1->pProvince=Prov2;
							Hero1->ipProvince=Prov2->iNumber;
						};
						if (Hero1->pSquad[0]->iLife==0)
						{
							bCycle=false;							
							Hero1->Death(Win);							
						};
						if (Hero2->pSquad[0]->iLife==0)
						{							
							if (Win==0)
							{Hero2->Death(1);}
							else
							{	
								if (Win==1)
								{Hero2->Death(0);}
								else
								{Hero2->Death(Win);};
							};
						};
					}
					else
					{						
						bCycle=false;
					};
				};
			};
		};
		}
	};
	if (Index==18)
	{
		DialogDX=DX;
		(pFirstDialog+ipDialog)->Show(true,true,false,false,NULL,sMessage);
		Prov1->ipPlayer=Player1;
	};
	if (arPlayer[Player1].Control==1)
	{
	bool bAnswer[MAX_ANSWER];
	for (int i=0; i<MAX_ANSWER; i++)
	{
		if ((pFirstDialog+ipDialog)->arAnswer[i][1]=='#')
		{bAnswer[i]=false;}
		else
		{bAnswer[i]=true;};
	};	
	if (Index==1)
	{
		//Атака нейтральной провинции
		if (Prov2->ipPlayer==0)
		{
		DialogDX=DX;
		pAdvice=Hero1;
		iDialogButt=1;
		if (arDifficulty[arPlayer[Player1].iDifficulty].bShowAdvice)
		{
			iHeroAdvice=1;
			iAdvice=CompareStrength1(Hero1,Hero2);			
		};
		if (Prov2->iDefender>3)
		ipDialog=(pFirstProvinceType+Prov2->ipType)->arGuard[Prov2->iDefender][2];
		char txtSquad [500];
		int MaxGuards=Prov2->pGuard->MakeDescription(txtSquad);										
		sprintf(sMessage,(pFirstDialog+ipDialog)->sText,Hero1->pstName->str,Prov2->sName,MaxGuards,txtSquad);
		int Answer;
	//	StopLog("Оценка сил:",CompareStrength(Hero1,Hero2));
		if ((arPlayer[Player1].iUnion==Prov2->ipRace)&&((pFirstProvinceType+Prov2->ipType)->ipRace==Prov2->ipRace))
		{Answer=3;}
		else
		{
		DialogDX=DX;
		if ((Tutorial.Map)&&((Prov2->iNumber==21)||(Prov2->ipRace>1)))
		{bAnswer[2]=false;};
		if (Hero1->CheckSkill(24)>0)
		{
			iScouting=SCOUTING_PRICE;
			if (Hero1->CheckSkill(24)>=5)
			{iScouting/=2;};
			iScGold=iScouting;
			if (iScouting<=arPlayer[Player1].Gold)
			{
				if ((pFirstDialog+ipDialog)->arAnswer[2][1]=='#')
				{bAnswer[2]=true;}
				else
				{
					if ((pFirstDialog+ipDialog)->arAnswer[3][1]=='#')
					{bAnswer[3]=true;}
				};
			};
		};				
		Answer=(pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage);
		if ((pFirstDialog+ipDialog)->arAnswer[Answer-1][1]=='#')
		{
			pAdvice=Hero1;
			iDialogButt=1;
			bAnswer[Answer-1]=false;
			arPlayer[Player1].Gold-=iScGold;
			Hero2->MakeScoutingText();
			Answer=(pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,stScouting);
		};
		};
		
		switch (Answer)
		{
		case 1:
			flgEncResult=-1;
			break;
		case 2:
			flgEncResult=0;
			break;
		case 3:
			flgEncResult=0;
			if ((pFirstProvinceType+Prov2->ipType)->arGuard[Prov2->iDefender][3]>0)
			{
				EventDX=DX;
				Hero1->CallEncounter((pFirstProvinceType+Prov2->ipType)->arGuard[Prov2->iDefender][3],Hero2,Prov2,-1,0,0,0);
			};
			break;
		};
		if (flgEncResult<0)
		{
			int Win=InitBattle(Prov1,Prov2,Hero1,Hero2,0,true);
			if (Win==1)
			{
				Prov2->Conquer(Player1,Hero1);
				Prov2->DismissDefender();				
			};
			if (Hero1->pSquad[0]->iLife==0)
			{
				Hero1->Death(Win);
			};
		};
		if (flgEncResult>0)
		{
			Prov2->ipPlayer=Player1;
			if (flgEncResult==9999)
			{
				Prov2->DismissDefender();
			}
			else
			{
				//Преобразование охранника в стража
				Prov2->iDefender=flgEncResult;
				Prov2->pGuard->ipPlayer=Player1;
			};
			Prov2->ChangeAttitude(Player1,(pFirstProvinceType+Prov2->ipType)->iDiplUnrest);
			Hero1->pProvince=Prov2;
			Hero1->ipProvince=Prov2->iNumber;
			arPlayer[Player1].iKarma+=(pFirstProvinceType+Prov2->ipType)->iDiplKarma;
		};
		
		}
		else
		{
			Hero1->pProvince=Prov2;
			Hero1->ipProvince=Prov2->iNumber;
		};
	};	
	if (Index==2)
	{
		//Сражение с охраной сайта
	//	StopLog("Оценка сил:",CompareStrength1(Hero1,Hero2));
		if (Prov1->arSiteGuard[iParam1])
		{
		DialogDX=DX;
		pAdvice=Hero1;
		iDialogButt=1;
		if (arDifficulty[arPlayer[Player1].iDifficulty].bShowAdvice)
		{
			iHeroAdvice=1;
			iAdvice=CompareStrength1(Hero1,Hero2);			
		};
		if (Hero1->CheckSkill(24)>0)
		{
			iScouting=SCOUTING_PRICE;
			if (Hero1->CheckSkill(24)>=5)
			{iScouting/=2;};
			iScGold=iScouting;
			if (iScouting<=arPlayer[Player1].Gold)
			{
				if ((pFirstDialog+ipDialog)->arAnswer[2][1]=='#')
				{bAnswer[2]=true;}
				else
				{
					if ((pFirstDialog+ipDialog)->arAnswer[3][1]=='#')
					{bAnswer[3]=true;}
				};
			};
		};				
		int Answer=(pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage);
		if ((pFirstDialog+ipDialog)->arAnswer[Answer-1][1]=='#')
		{
			pAdvice=Hero1;
			iDialogButt=1;
			bAnswer[Answer-1]=false;
			arPlayer[Player1].Gold-=iScGold;
			Hero2->MakeScoutingText();
			Answer=(pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,stScouting);
		};
		switch (Answer)
		{
		case 1:
			flgEncResult=-1;			
			break;
		case 2:
			flgEncResult=0;
			break;
		case 3:
			flgEncResult=0;
			EventDX=DX;
			Hero1->CallEncounter((pFirstSite+Prov1->arSite[iParam1][0])->ipDialog2,Prov1->arSiteGuard[iParam1],Prov1,iParam1,0,0,0);
			break;
		};
		if (flgEncResult<0)
		{
			int Win=InitBattle(Prov1,Prov2,Hero1,Hero2,iParam1,true);
			if (Win==1)
			{
				if ((Prov1->arSite[iParam1][4]==23)||(Prov1->arSite[iParam1][4]==41)||(Prov1->arSite[iParam1][4]==46))
				{arPlayer[Player1].iDragonHunt++;};
				if ((WorldRules.iMode==2)&&(Prov1->arSite[iParam1][0]==26)&&(arPlayer[Player1].ipProvince==Prov1->iNumber))
				{Prov1->StaticEvent(1267,Player1);}
				if ((WorldRules.iMode>0)&&(Prov1->arSite[iParam1][5]==-1)&&((pFirstWorldRules+WorldRules.iMode)->ipSiteEvent>0))
				{Prov1->StaticEvent((pFirstWorldRules+WorldRules.iMode)->ipSiteEvent,Player1);}
				if (bExitGame)
				{return 0;}
				//Обыск алтаря смерти в кампании
				if ((CurGame.iGameType==2)&&(Prov1->arSite[iParam1][0]==26)&&(Hero1->pSquad[0]->iLife>0))
				{
					int Choise=0;
					if ((Campaign.iActiveShard==-7)&&(Campaign.arDialog[573]>0))
					{
						if (Campaign.arDialog[30]>0)
						{
							Choise=(pFirstDialog+1218)->Show(1,1,1,0,NULL,NULL);
						}
						else
						{
							Choise=(pFirstDialog+1217)->Show(1,1,0,0,NULL,NULL);
							if (Choise>1)
							{Choise=0;}
						};
					}
					else
					{
						if (Campaign.arDialog[30]>0)
						{
							Choise=(pFirstDialog+1216)->Show(1,1,0,0,NULL,NULL);
							Choise++;
						};
					};
					//Поиск амфоры
					if (Choise==1)
					{
						Hero1->pSquad[0]->iLife=0;
						if (Random(100,CurGame.iSeed,Prov1->iNumber,iParam1)>60)
						{
							(pFirstDialog+1222)->Show(1,0,0,0,NULL,NULL);
							arPlayer[2].bInGame=false;
							arPlayer[1].arKills[2]=1;
							return 555;
						}
						else
						{(pFirstDialog+1223)->Show(1,1,0,0,NULL,NULL);};
					};
					//Поиск Ключа
					if (Choise==2)
					{
						(pFirstDialog+1219)->Show(1,0,0,0,NULL,NULL);
						Campaign.arDialog[30]=0;
						Hero1->pSquad[0]->iLife=0;
						if (Campaign.arDialog[358]>0)
						{
							(pFirstDialog+1221)->Show(1,1,0,0,NULL,NULL);
							Campaign.arJournal[11]=0;
							Campaign.arJournal[12]=0;
							Campaign.arJournal[13]=1;

						}
						else
						{
							(pFirstDialog+1220)->Show(1,1,0,0,NULL,NULL);
							Campaign.arDialog[31]=1;
						};
					};
				};


				Prov1->DismissSiteGuard(iParam1);
				bool bShop=false;
				for (int i=0; i<MAX_SITE_ABILITY; i++)
				{
					if ((pFirstSite+Prov1->arSite[iParam1][0])->arAbility[i][0]==13)
					{bShop=true;}
				}
				if ((Prov1->arSite[iParam1][2]>0)||(bShop))
				{
					WindowTreasure(Prov1->arSite[iParam1][2],Player1,Hero1,Prov1->arSite[iParam1][0], iParam1);
				};				
				//Проверки квестов
		//		bQuestComplete=false;
				arPlayer[Player1].CheckCampQuest(Prov1->iNumber,iParam1);
				for (int i=0; i<HERO_QUEST_LIMIT; i++)
				{
					if ((pFirstQuest+Hero1->arQuest[i].Index)->iType==1) //Обыск сайта
					{
						for (int j=0; j<MAX_QUEST_TASK; j++)
						{
							if (((pFirstQuest+Hero1->arQuest[i].Index)->Index[j]==Prov1->arSite[iParam1][0])&&(!Hero1->arQuest[i].bComplete))
							{
								if	((pFirstQuest+Hero1->arQuest[i].Index)->Param[j]>Random(100))
								{
									Hero1->arQuest[i].arProgress[j]++;
									if ((pFirstQuest+Hero1->arQuest[i].Index)->ipEncFound>0)
									{
										Hero1->CallEncounter((pFirstQuest+Hero1->arQuest[i].Index)->ipEncFound,NULL,Prov1,iParam1,Hero1->arQuest[i].Index,0,0);
									};
									Hero1->CheckQuestDone(i);
								}
								else
								{
									if ((pFirstQuest+Hero1->arQuest[i].Index)->ipEncNotFound>0)
									{
										Hero1->CallEncounter((pFirstQuest+Hero1->arQuest[i].Index)->ipEncNotFound,NULL,Prov1,iParam1,Hero1->arQuest[i].Index,0,0);
									};
								};
							};
						};
					};
				};
			/*	if (bQuestComplete)
				{
					while (Hero1->iLevel<Hero1->CountLevel())
					{
						WindowHeroLvlUp(Hero1);
					};
				};*/
				for (int i=0; i<MAX_SITE_ABILITY; i++)
				{
					if ((pFirstSite+Prov1->arSite[iParam1][0])->arAbility[i][0]==17)
					{
						Hero1->CallEncounter((pFirstSite+Prov1->arSite[iParam1][0])->arAbility[i][1],NULL,Prov1,iParam1,0,0,0);
					};
				};
				if ((pFirstSite+Prov1->arSite[iParam1][0])->bSelfDestroy)
				{
					Prov1->arSite[iParam1][0]=0;
				};
			};
			if (Hero1->pSquad[0]->iLife==0)
			{
				Hero1->Death(Win);
			};
		};
		
		if (((Prov1->arSite[iParam1][0]>0)&&(pFirstSite+Prov1->arSite[iParam1][0])->arAbility[0][0]==21))
		{
			if (Prov1->arSiteGuard[iParam1])
			{Prov1->DismissSiteGuard(iParam1);};
			Prov1->arSite[iParam1][0]=0;
		};
		
		}
		else
	//Никогда не охраняемый сайт
		{
			int Site=Prov1->arSite[iParam1][0];
			if ((Site>0)&&((pFirstSite+Site)->iGuarded==0))
			{
				int Work=(pFirstSite+Site)->arAbility[0][1]+(pFirstSite+Site)->arAbility[0][2]*Prov1->arSite[iParam1][2];
				switch ((pFirstSite+Site)->arAbility[0][0])
				{
				case 9:					
					sprintf(sMessage,(pFirstDialog+(pFirstSite+Site)->ipDialog)->sText,Prov1->sName,Hero1->pstName->str,Work);
					DialogDX=DX;
					if ((pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage)==2)
					{Hero1->iTask=0;};
					Hero1->iExp+=Work;
					while (Hero1->CountLevel()>Hero1->iLevel)
					{
						WindowHeroLvlUp(Hero1);
					};
					break;
				case 10:
					sprintf(sMessage,(pFirstDialog+(pFirstSite+Site)->ipDialog)->sText,Prov1->sName,Hero1->pstName->str,Work);
					DialogDX=DX;
					if ((pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage)==2)
					{Hero1->iTask=0;};
					arPlayer[Player1].Gold+=Work;					
					break;
				case 11:
					sprintf(sMessage,(pFirstDialog+(pFirstSite+Site)->ipDialog)->sText,Prov1->sName,Hero1->pstName->str,Work);
					DialogDX=DX;
					if ((pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage)==2)
					{Hero1->iTask=0;};
					arPlayer[Player1].Gem+=Work;					
					break;
				case 12:
					sprintf(sMessage,(pFirstDialog+(pFirstSite+Site)->ipDialog)->sText,Prov1->sName,Hero1->pstName->str);
					DialogDX=DX;
					if ((pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage)==2)
					{Hero1->iTask=0;};
					if (Prov1->arSite[iParam1][2]>1)
					{WindowTreasure(Prov1->arSite[iParam1][2],Player1,Hero1,Prov1->arSite[iParam1][0], iParam1);}
					else
					{WindowTreasure(2,Player1,Hero1,Prov1->arSite[iParam1][0], iParam1);};
					break;
				case 16:
					DialogDX=DX;
					EventDX=DX;
					Hero1->Arena();
					break;
				default:
					EventDX=DX;
					Hero1->CallEncounter((pFirstSite+Site)->ipDialog2,NULL,Prov1,iParam1,0,0,0);
					break;
				};				
				if ((pFirstSite+Prov1->arSite[iParam1][0])->bSelfDestroy)
				{
					Prov1->arSite[iParam1][0]=0;
				};
			}
			else
			{
				Hero1->CallEncounter((pFirstSite+Prov1->arSite[iParam1][0])->ipDialog2,NULL,Prov1,iParam1,0,0,0);
			};
		};
	};
	
	if (Index==3)
	{
		//Исследование провинции
		DialogDX=DX;
		switch ((pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage))
		{			
		case 1:
			if (pTree)
			{
				pTree->Call(DX);				
				delete pTree;
			}
			else
			{
				while (Hero1->iLevel<Hero1->CountLevel())
				{WindowHeroLvlUp(Hero1);}				
			};

			break;
		case 2:
			while (Hero1->iLevel<Hero1->CountLevel())
			{WindowHeroLvlUp(Hero1);}
			if (pTree)
			{
				if ((pFirstSite+pTree->Prov1->arSite[pTree->iParam1][0])->arAbility[0][0]==21)
				{
					pTree->Prov1->DismissSiteGuard(pTree->iParam1);
					pTree->Prov1->arSite[pTree->iParam1][0]=0;
				};
				delete pTree;
			}
			else
			{
				Hero1->iTask=0;
			};
			break;
		case 3:
			Hero1->iTask=0;
			while (Hero1->iLevel<Hero1->CountLevel())
			{WindowHeroLvlUp(Hero1);}
			if ((pFirstSite+pTree->Prov1->arSite[pTree->iParam1][0])->arAbility[0][0]==21)
			{
				pTree->Prov1->DismissSiteGuard(pTree->iParam1);
				pTree->Prov1->arSite[pTree->iParam1][0]=0;
			};
			delete pTree;
			break;
		};		
	};
	if (Index==4)
	{
		//Найден сайт без охраны
		DialogDX=DX;
		if ((pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage)==2)
		{
			Hero1->iTask=0;
		};
	};
	if (Index==5)
	{
		//Атака стражника своей провинции
		if (Prov1->pGuard)
		{
			Prov1->pGuard->ipPlayer=0;
			DialogDX=DX;
			pAdvice=Hero1;
			iDialogButt=1;
			(pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage);
			int Win=InitBattle(Prov1,NULL,Hero1,Hero2,0,true);
			if (Hero1->pSquad[0]->iLife==0)
			{
				Hero1->Death(Win);
			};
			if (abs(Win)==1)
			{				
				Prov1->DismissDefender();				
			}
			else
			{
			//Изменить тип в зависимости от стража
				if (Tutorial.Map)
				{
					Hero1->pProvince=(pFirstProvince+arPlayer[Player1].ipProvince);
					Hero1->ipProvince=arPlayer[Player1].ipProvince;
				};
				dcHero *Attacker=Prov1->pGuard;
				Prov1->pGuard=NULL;
				Prov1->iDefender=0;
				if (Prov1->UnderAttack(Attacker,DX)==1)
				{
					Prov1->pGuard=Attacker;
					Prov1->pGuard->iExp=1;				
					Prov1->ipType=21;
					bool bTakeIt=true;
					if (Prov1->pGarrison)
					{
						for (int i=0; i<MAX_SQUAD; i++)
						{
							if (Prov1->pGarrison->pSquad[i])
							{bTakeIt=false;};
						};
					};
					if (bTakeIt)
					{
						Prov1->Conquer(0,NULL);						
					};
				}
				else
				{
					Prov1->pGuard=Attacker;
					Prov1->DismissDefender();
				};				
				/*Prov1->pGuard->iExp=1;
				Prov1->iDefender=0;
				Prov1->ipType=21;
				bool bTakeIt=true;
				if (Prov1->pGarrison)
				{
					for (int i=0; i<MAX_SQUAD; i++)
					{
						if (Prov1->pGarrison->pSquad[i])
						{bTakeIt=false;};
					};
				};
				if (bTakeIt)
				{
					Prov1->Conquer(0,NULL);
				};*/
			};			
		};
	};
//Грабеж провинции обычный
	if (Index==6)
	{
		DialogDX=DX;
		if ((pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage)==2)
		{
			Hero1->iTask=0;
		};
		while (Hero1->iLevel<Hero1->CountLevel())
		{WindowHeroLvlUp(Hero1);}
	};
//Грабеж провинции финальный
	if (Index==7)
	{
		Hero1->iTask=0;
		if (arPlayer[Player1].Gem<15)
		{
			bAnswer[1]=false;
		};
		DialogDX=DX;
		switch ((pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage))
		{
		case 1:
			Prov1->iPopulation=-9;
			break;
		case 2:
			arPlayer[Player1].Gem-=15;
			Prov1->iPopulation=-19;
			break;
		};
		while (Hero1->iLevel<Hero1->CountLevel())
		{WindowHeroLvlUp(Hero1);}
	};
	if ((Index==6)||(Index==7))
	{
		//Проверка квестов грабежа
	//	bQuestComplete=false;
		for (int i=0; i<HERO_QUEST_LIMIT; i++)
		{
			if (((pFirstQuest+Hero1->arQuest[i].Index)->iType==6)&&(!Hero1->arQuest[i].bComplete))
			{
				Hero1->arQuest[i].arProgress[0]+=iParam1;
				Hero1->CheckQuestDone(i);
				if ((!Hero1->arQuest[i].bComplete)&&((pFirstQuest+Hero1->arQuest[i].Index)->ipEncFound>0))
				{
					Hero1->CallEncounter((pFirstQuest+Hero1->arQuest[i].Index)->ipEncFound,NULL,Prov1,iParam1,Hero1->arQuest[i].Index,0,0);
				};
			};
			
		/*	if (bQuestComplete)
			{
				while (Hero1->iLevel<Hero1->CountLevel())
				{
					WindowHeroLvlUp(Hero1);
				};
			}*/
		};
	};
	if (Index==8)
	{};
	if ((Index==11)&&(Prov1->ipPlayer==Player2))
	{
		DialogDX=DX;
		(pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage);
		int Win=InitBattle(Prov1,NULL,Hero1,Hero2,-1,true);
		if (Win==1)
		{
			Prov1->Conquer(Player1,Hero1);			
		}
		if (Hero1->pSquad[0]->iLife==0)
		{
			Hero1->Death(Win);
		};
	};
	if ((Index==12)&&(Prov1->ipPlayer==Player2))
	{
		DialogDX=DX;
		int Answer=(pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage);
		if (Answer==2)
		{
			int Win=InitBattle(Prov1,NULL,Hero1,Hero2,-1,true);
			if (Win==1)
			{
				Prov1->Conquer(Player1,Hero1);				
			};
			if (Hero1->pSquad[0]->iLife==0)
			{
				Hero1->Death(Win);
			};
//			StopLog("Шаг ",1);
		};
//		StopLog("Шаг ",2);
	};
	if ((Index==13)&&(Prov1->ipPlayer==Player2))
	{
		DialogDX=DX;
		iMessageFor=Player1;
		(pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage);		
	};
	if (Index==17)
	{
		DialogDX=DX;
		iMessageFor=Player1;
		(pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage);
		Prov1->Conquer(Player1,Hero1);
	};
//	StopLog("Шаг ",3);
	if ((Index==14)&&(Prov2->GetSiegeIndex()==0))
	{
		Hero2=Prov2->pGuard;
		char sArmy[500];
		int iAtt=Hero2->MakeDescription(sArmy);
		sprintf(sMessage,(pFirstDialog+386)->sText,Hero1->pstName->str,Prov2->sName,iAtt,sArmy);
		DialogDX=DX;
		iMessageFor=Player1;
		int Answer=(pFirstDialog+386)->Show(true,true,false,false,NULL,sMessage);
		if (Answer==1)
		{
			int Win=InitBattle(Prov1,Prov2,Hero1,Hero2,0,true);
			if (Win==1)
			{
				Hero1->pProvince=Prov2;
				Hero1->ipProvince=Prov2->iNumber;				
			};
			if (Win!=0)
			{
				bool bDead=true;
				for (int SQ=0; SQ<MAX_SQUAD; SQ++)
				{
					if ((Hero2->pSquad[SQ])&&(Hero2->pSquad[SQ]->iLife>0))
					{bDead=false;};
				};
				if (bDead)
				{
					Prov2->DismissDefender();
					Prov2->ipEvent=0;
				};
			};
			if (Hero1->pSquad[0]->iLife==0)
			{				
				Hero1->Death(Win);
			};
		};		
	};
//Выдача диалога
	if (Index==20)
	{
		DialogDX=DX;
		(pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage);
	};
	}	
	else
//*****Обработка событий компьютерным игроком
	{		
	if (Index==1)
	{
		sprintf(txtLog,"Герой %s игрока %s атакует провинцию %s",Hero1->pstName->str,arPlayer[Player1].GetName(),Prov2->sName);
		WriteLog(txtLog);
		FileLog(txtLog);
		//Атака нейтральной провинции
		if (Prov2->ipPlayer!=Player1)
		{
		if ((arPlayer[Player1].iUnion==0)&&(arPlayer[Player1].AIUnion[0]==Prov2->ipRace)&&(arPlayer[Player1].AIUnion[1]==0))
		{arPlayer[Player1].iUnion=Prov2->ipRace;};
		if ((arPlayer[Player1].iUnion==Prov2->ipRace)||
			((Prov2->ipRace==1)&&(3+Hero1->CheckSkill(27)*3>Random(100))))
		{
			switch (Prov2->ipType)
			{
			case 1:
				flgEncResult=30;
				break;
			case 2:
				flgEncResult=29;
				break;
			case 4:
				flgEncResult=43;
				break;
			case 6:
				flgEncResult=42;
				break;
			case 8:
				flgEncResult=45;
				break;
			case 9:
				flgEncResult=37;
				break;
			case 10:
				flgEncResult=38;
				break;
			case 11:
				flgEncResult=31;
				break;
			case 16:
				if (arPlayer[Player1].iKarma>0)
				{flgEncResult=34;}
				else
				{flgEncResult=-1;};
				break;
			case 17:
				flgEncResult=35;
				break;
			case 18:
				flgEncResult=41;
				break;
			case 20:
				flgEncResult=55;
				break;
			case 24:
				flgEncResult=68;
				break;
			default:
				flgEncResult=-1;
				break;
			};
		}
		else
		{flgEncResult=-1;};	
		if (flgEncResult<0)
		{
	//		bHourGlass=false;
		//	char Txt[500];
		//	sprintf(Txt, "Оценка сил: %d",CompareStrength1(Hero1,Hero2));
		//	FileLog(Txt);
			int Win=InitBattle(Prov1,Prov2,Hero1,Hero2,0,false);
			Hero1->NiB=0;
			if (Win==1)
			{
				FileLog("Победа");
				Prov2->Conquer(Player1,Hero1);
				Prov2->DismissDefender();				
			};
			if (Hero1->pSquad[0]->iLife==0)
			{
				Hero1->Death(Win);
			};
		};		
		if (flgEncResult>0)
		{
			FileLog("Присоединение");
			Prov2->ipPlayer=Player1;
			if (flgEncResult==9999)
			{
				Prov2->DismissDefender();
			}
			else
			{
				//Преобразование охранника в стража
				Prov2->iDefender=flgEncResult;
				Prov2->pGuard->ipPlayer=Player1;
			};
			Prov2->ChangeAttitude(Player1,(pFirstProvinceType+Prov2->ipType)->iDiplUnrest);
			Hero1->pProvince=Prov2;
			Hero1->ipProvince=Prov2->iNumber;
			arPlayer[Player1].iKarma+=(pFirstProvinceType+Prov2->ipType)->iDiplKarma;
		};
		}
		else
		{
			Hero1->pProvince=Prov2;
			Hero1->ipProvince=Prov2->iNumber;
		};
	};
	if (Index==2)
	{
		sprintf(txtLog,"Герой %s игрока %s входит в сайт %s в провинции %s          ",Hero1->pstName->str,arPlayer[Player1].GetName(),(pFirstSite+Prov1->arSite[iParam1][0])->sName,Prov1->sName);
		WriteLog(txtLog);
		FileLog(txtLog);
		//Сражение с охраной сайта
		if (Prov1->arSiteGuard[iParam1])
		{
			if (CompareStrength1(Hero1,Hero2)>=110-Hero1->NiB)
			{
			int Win=InitBattle(Prov1,Prov2,Hero1,Hero2,iParam1,false);
			Hero1->NiB=0;
			if (Win==1)
			{
				FileLog("Победа");
				Prov1->DismissSiteGuard(iParam1);				
				if (Prov1->arSite[iParam1][2]>0)
				{					
					WindowTreasure(Prov1->arSite[iParam1][2],Player1,Hero1,Prov1->arSite[iParam1][0],iParam1);
				};						
				if (((pFirstSite+Prov1->arSite[iParam1][0])->bSelfDestroy)||(Prov1->arSite[iParam1][0]==50))
				{
					Prov1->arSite[iParam1][0]=0;
				};
			}
			else
			{
				FileLog("Поражение");				
			};
			if (Hero1->pSquad[0]->iLife==0)
			{				
				Hero1->Death(Win);
			};		
			};			
			if (((Prov1->arSite[iParam1][0]>0)&&(pFirstSite+Prov1->arSite[iParam1][0])->arAbility[0][0]==21))
			{
				if (Prov1->arSiteGuard[iParam1])
				{Prov1->DismissSiteGuard(iParam1);};
				Prov1->arSite[iParam1][0]=0;
			};
		}
		else
		{
	//Никогда не охраняемый сайт
			int Site=Prov1->arSite[iParam1][0];
			if ((Site>0)&&((pFirstSite+Site)->iGuarded==0))
			{
				int Work=(pFirstSite+Site)->arAbility[0][1]+(pFirstSite+Site)->arAbility[0][2]*Prov1->arSite[iParam1][2];
				switch ((pFirstSite+Site)->arAbility[0][0])
				{
				case 9:					
					Hero1->iExp+=Work;
					while (Hero1->CountLevel()>Hero1->iLevel)
					{
						AIHeroLvlUp(Hero1);						
					};
					break;
				case 10:					
					arPlayer[Player1].Gold+=Work;					
					break;
				case 11:					
					arPlayer[Player1].Gem+=Work;					
					break;
				case 12:					
					if (Prov1->arSite[iParam1][2]>1)
					{WindowTreasure(Prov1->arSite[iParam1][2],Player1,Hero1,Prov1->arSite[iParam1][0],iParam1);}
					else
					{WindowTreasure(2,Player1,Hero1,Prov1->arSite[iParam1][0],iParam1);};
					break;
				case 16:
					Hero1->iExp+=(20+Hero1->iLevel*5);
					while (Hero1->CountLevel()>Hero1->iLevel)
					{
						AIHeroLvlUp(Hero1);						
					};
					break;
				};				
				if (((pFirstSite+Prov1->arSite[iParam1][0])->bSelfDestroy)||(Prov1->arSite[iParam1][0]==52))
				{
					Prov1->arSite[iParam1][0]=0;
				};
			};
		};
	};	
	if (Index==3)
	{
		//Исследование провинции
		sprintf(txtLog,"Герой %s игрока %s исследует провинцию %s          ",Hero1->pstName->str,arPlayer[Player1].GetName(),Prov1->sName);
		WriteLog(txtLog);
		FileLog(txtLog);
		while (Hero1->iLevel<Hero1->CountLevel())
		{AIHeroLvlUp(Hero1);}
		if (pTree)
		{
			pTree->Call(DX);
			delete pTree;
		};
	};
	if (Index==4)
	{
		//Найден сайт без охраны		
	};
	if (Index==5)
	{
		//Атака стражника своей провинции
		if (Prov1->pGuard)
		{	
			Prov1->pGuard->ipPlayer=0;
			int Win=InitBattle(Prov1,NULL,Hero1,Hero2,0,false);
			Hero1->NiB=0;
			if (abs(Win)==1)
			{				
				Prov1->DismissDefender();
			}
			else
			{
				dcHero *Attacker=Prov1->pGuard;
				Prov1->pGuard=NULL;
				Prov1->iDefender=0;
				if (Prov1->UnderAttack(Attacker,DX)==1)
				{
					Prov1->pGuard=Attacker;
					Prov1->pGuard->iExp=1;				
					Prov1->ipType=21;
					bool bTakeIt=true;
					if (Prov1->pGarrison)
					{
						for (int i=0; i<MAX_SQUAD; i++)
						{
							if (Prov1->pGarrison->pSquad[i])
							{bTakeIt=false;};
						};
					};
					if (bTakeIt)
					{
						Prov1->Conquer(0,NULL);						
					};
				}
				else
				{
					Prov1->pGuard=Attacker;
					Prov1->DismissDefender();
				};
			};
			if (Hero1->pSquad[0]->iLife==0)
			{
				Hero1->Death(Win);
			};
		};
	};
//Грабеж провинции обычный
	if (Index==6)
	{
			
	};
//Грабеж провинции финальный
	if (Index==7)
	{
		Hero1->iTask=0;
		/*if (arPlayer[Player1].Gem<15)
		{
			bAnswer[1]=false;
		};
		switch ((pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage))
		{
		case 1:
			Prov1->iPopulation=-9;
			break;
		case 2:
			arPlayer[Player1].Gem-=15;
			Prov1->iPopulation=-19;
			break;
		};*/
	};
	if ((Index==6)||(Index==7))
	{
		//Проверка квестов грабежа
		/*for (int i=0; i<HERO_QUEST_LIMIT; i++)
		{
			if (((pFirstQuest+Hero1->arQuest[i].Index)->iType==6)&&(!Hero1->arQuest[i].bComplete))
			{
				Hero1->arQuest[i].arProgress[0]+=iParam1;
				Hero1->CheckQuestDone(i);
			};
			if ((!Hero1->arQuest[i].bComplete)&&((pFirstQuest+Hero1->arQuest[i].Index)->ipEncFound>0))
			{
				Hero1->CallEncounter((pFirstQuest+Hero1->arQuest[i].Index)->ipEncFound,NULL,Prov1,iParam1,Hero1->arQuest[i].Index,0,0);
			};
	
		};*/
	};
	if ((Index==11)&&(Prov1->ipPlayer==Player2))
	{
		int iBattle;
		if (arPlayer[Player2].Control==1)
		{		
			sprintf(sMessage,(pFirstDialog+381)->sText,Prov1->sName);
			DialogDX=DX;
			(pFirstDialog+381)->Show(true,true,false,false,NULL,sMessage);
			iBattle=InitBattle(Prov1,NULL,Hero1,Hero2,-1,true);
		}
		else
		{iBattle=InitBattle(Prov1,NULL,Hero1,Hero2,-1,false);};
		Hero1->NiB=0;
		if (iBattle==1)
		{
			if (arPlayer[Player2].Control==1)
			{
				sprintf(sMessage,(pFirstDialog+382)->sText,Prov1->sName);
				DialogDX=DX;
				(pFirstDialog+382)->Show(true,true,false,false,NULL,sMessage);
			};
			Prov1->Conquer(Player1,Hero1);			
		};		
		if (Hero1->pSquad[0]->iLife==0)
		{
			Hero1->Death(iBattle);
		};
	};
	if ((Index==12)&&(Prov1->ipPlayer==Player2))
	{		
		if ((CompareStrength1(Hero1,Hero2)>100)||(Prov1->iFort=0))
		{
			sprintf(txtLog,"Герой %s игрока %s штурмует замок в провинции %s          ",Hero1->pstName->str,arPlayer[Player1].GetName(),Prov1->sName);
			WriteLog(txtLog);
			FileLog(txtLog);
			int iBattle;
			if (arPlayer[Player2].Control==1)
			{		
				sprintf(sMessage,(pFirstDialog+381)->sText,Prov1->sName);
				DialogDX=DX;
				(pFirstDialog+381)->Show(true,true,false,false,NULL,sMessage);
				iBattle=InitBattle(Prov1,NULL,Hero1,Hero2,-1,true);
			}
			else
			{iBattle=InitBattle(Prov1,NULL,Hero1,Hero2,-1,false);};
			Hero1->NiB=0;
			if (iBattle==1)
			{
				FileLog("Победа");
				if (arPlayer[Player2].Control==1)
				{
					sprintf(sMessage,(pFirstDialog+382)->sText,Prov1->sName);
					DialogDX=DX;
					(pFirstDialog+382)->Show(true,true,false,false,NULL,sMessage);
				};
				Prov1->Conquer(Player1,Hero1);				
			}
			else
			{
				FileLog("Поражение");
			};			
			if (Hero1->pSquad[0]->iLife==0)
			{
				Hero1->Death(iBattle);
			};
		}
		else
		{
			sprintf(txtLog,"Герой %s игрока %s осаждает замок в провинции %s          ",Hero1->pstName->str,arPlayer[Player1].GetName(),Prov1->sName);
			WriteLog(txtLog);
			FileLog(txtLog);
			if (arPlayer[Player2].Control==1)
			{
				sprintf(sMessage,(pFirstDialog+380)->sText,Prov1->sName);
				DialogDX=DX;
				(pFirstDialog+380)->Show(true,true,false,false,NULL,sMessage);
			};
		};		
	};
	if ((Index==13)&&(arPlayer[Player2].Control==1)&&(Prov1->ipPlayer==Player2))
	{
		DialogDX=DX;
		(pFirstDialog+ipDialog)->Show(true,true,false,false,NULL,sMessage);		
	};
	if (Index==17)
	{
//		(pFirstDialog+ipDialog)->Show(bAnswer[0],bAnswer[1],bAnswer[2],bAnswer[3],NULL,sMessage);
		Prov1->Conquer(Player1,Hero1);
	};
	if ((Index==14)&&(Prov2->GetSiegeIndex()==0))
	{
		Hero2=Prov2->pGuard;
		if ((CompareStrength1(Hero1,Hero2)>100)||(arPlayer[Player1].ipProvince==Prov2->iNumber))
		{
			int Win=InitBattle(Prov1,Prov2,Hero1,Hero2,0,false);
			Hero1->NiB=0;
			if (Win==1)
			{
				Prov2->DismissDefender();
				Hero1->pProvince=Prov2;
				Hero1->ipProvince=Prov2->iNumber;
			};
			if (Hero1->pSquad[0]->iLife==0)
			{				
				Hero1->Death(Win);				
			};			
		};
	};
	};
	}
	else
	{Result=0;};
	EventDX=0;
	DFileLog("EndEvent");
	return Result;
};

void Rebel(dcProvince *PR, int DX)
{
	int Result=500;
	int Grd;
	dcProvince Fake;
	Fake.sName=new char [3];
	char Mess[500];
	if (arPlayer[PR->ipPlayer].Control==1)
	{
		sprintf(Mess,(pFirstDialog+62)->sText,PR->sName);
		DialogDX=DX;
		(pFirstDialog+62)->Show(true,true,false,false,NULL,Mess);
	}
	else
	{
		sprintf(Mess,"Восстание в провинции %s игрока %s",PR->sName,arPlayer[PR->ipPlayer].GetName());
		WriteLog(Mess);
		FileLog(Mess);
	};

	int Adder=(PR->GetLevel()-3)/2;
	int NewType=(pFirstRace+PR->ipRace)->ipProvType;
	if (Adder<0)
	{Adder=0;};
	if (Adder>3)
	{Adder=3;};
	/*switch (PR->ipRace)
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
	Grd=(pFirstProvinceType+NewType)->arGuard[0][0];
	while ((Adder>0)&&((pFirstGuard+(pFirstGuardType+Grd)->arGuard[Adder])->Power>CurGame.Map.Neutrals))
	{Adder--;};
	Fake.PlaceGuard((pFirstGuardType+Grd)->arGuard[Adder],(pFirstGuardType+Grd)->iKarma);


	clear_keybuf();
	WriteLog("Отряд создан");
	
	Result=PR->UnderAttack(Fake.pGuard,DX);
	if (Result==1)
	{			
		bool bTakeIt=true;
		if (PR->pGarrison)
		{
			for (int i=0; i<MAX_SQUAD; i++)
			{
				if (PR->pGarrison->pSquad[i])
				{bTakeIt=false;};
			};
		};
		if (bTakeIt)
		{			
			if (arPlayer[PR->ipPlayer].Control==1)
			{
				sprintf(Mess,(pFirstDialog+65)->sText,PR->sName,(pFirstGuardType+Grd)->sName);
				DialogDX=DX;
				(pFirstDialog+65)->Show(true,true,false,false,NULL,Mess);
			};
			PR->Conquer(0,NULL);
		};
		PR->ipType=NewType;
		PR->pGuard=Fake.pGuard;
		PR->iDefender=0;
		Fake.pGuard=NULL;
	};
	WriteLog("Конец");
//	return Result;
};

long int clEvent::CalcEffect(int Index, int Mult, long int Value)
{
	long int Result=0;
	for (int i=0; i<MAX_EVENT_EFFECT; i++)
	{
		if (arEffect[i][0]==Index)
		{
			switch (arEffect[i][3])
			{
			case 1:
				Result+=arEffect[i][1]*Mult+Random(arEffect[i][2]*Mult);
				break;
			case 2:
				{
			//		StopLog("Было золота:", Value);
					long int Perc=(arEffect[i][1]+Random(arEffect[i][2]));
			//		StopLog("Множитель:        ",Perc);
					Result=Result+Value*Perc/100;
			//		StopLog("Результат:", Result);
				};
				break;
			default:			
				Result+=arEffect[i][1]+Random(arEffect[i][2]);
				break;
			};
		};
	};
	return Result;
};

int clEvent::GetParam1(int Index, int Power)
{
	int Result=0;
	for (int i=0; i<MAX_EVENT_EFFECT; i++)
	{
		if ((arEffect[i][0]==Index)&&(arEffect[i][1]==Power))
		{
			
			Result+=arEffect[i][2];
		};		
	};
	return Result;
};

int clEvent::GetParam2(int Index, int Power)
{
	int Result=0;
	for (int i=0; i<MAX_EVENT_EFFECT; i++)
	{
		if ((arEffect[i][0]==Index)&&(arEffect[i][1]==Power))
		{
			
			Result+=arEffect[i][3];
		};		
	};
	return Result;
};

int clEncounter::CalcEffect(int Index, int Mult, int Param1, int Param2, dcHero *Hero)
{
	int Result=0, Work; 
	float M1, M2;
	for (int i=0; i<MAX_EVENT_EFFECT; i++)
	{
		if (arEffect[i][0]==Index)
		{
			switch (arEffect[i][3])
			{
			case 0:
				Result+=arEffect[i][1]+Random(arEffect[i][2]);
				break;
			case 1:
				Result+=arEffect[i][1]*Mult+Random(arEffect[i][2]*Mult);
				break;
			case 2:
				Result+=Param1;
				break;
			case 3:
				Result+=Param2;
				break;
			case 4:
				Work=arEffect[i][1]+Random(arEffect[i][2]);
				Work+=Work*Hero->CheckHeroBonus(985)/100;
				Result+=Work;
				break;
			case 5:
				M1=0; M2=0;
				switch (arEffect[i][1])
				{
				case 1:
					M1=Mult;
					break;
				case 2:
					M1=Param1;
					break;
				case 3:
					M1=Param2;
					break;
				case 4:
					M1=1+Hero->CheckHeroBonus(985)/100;
					break;
				};
				switch (arEffect[i][2])
				{
				case 1:
					M2=Mult;
					break;
				case 2:
					M2=Param1;
					break;
				case 3:
					M2=Param2;
					break;
				case 4:
					M2=1+Hero->CheckHeroBonus(985)/100;
					break;
				};
				Result+=M1*M2;
				break;
			case 6:
				Result-=Param1;
				break;
			case 7:
				Result-=Param2;
				break;
			};
		};
	};
	return Result;
};

int clEncounter::GetPower(int Index)
{
	int Result=0;
	for (int i=0; i<MAX_EVENT_EFFECT; i++)
	{
		if (arEffect[i][0]==Index)
		{			
			Result+=arEffect[i][1];
		};		
	};
	return Result;
};

int clEncounter::GetParam1(int Index, int Power)
{
	int Result=0;
	for (int i=0; i<MAX_EVENT_EFFECT; i++)
	{
		if ((arEffect[i][0]==Index)&&(arEffect[i][1]==Power))
		{
			
			Result+=arEffect[i][2];
		};		
	};
	return Result;
};

int clEncounter::GetParam2(int Index, int Power)
{
	int Result=0;
	for (int i=0; i<MAX_EVENT_EFFECT; i++)
	{
		if ((arEffect[i][0]==Index)&&(arEffect[i][1]==Power))
		{
			
			Result+=arEffect[i][3];
		};		
	};
	return Result;
};

dcRitual *pRitual;

dcRitual::dcRitual()
{};

dcRitual::dcRitual(int Indx, int Player, int Prov)
{
	pNext=pRitual;
	pRitual=this;
	Index=Indx;
	ipPlayer=Player;
	ipProvince=Prov;
	Timer=(pFirstRitual+Index)->iTime;
	if (Timer<=0)
	{Timer=2;};
	if ((pFirstRitual+Index)->ipEvent>0)
	{iEvent=1;}
	else
	{iEvent=0;};
};

dcRitual::~dcRitual()
{};