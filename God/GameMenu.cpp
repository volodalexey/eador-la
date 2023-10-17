#include "Mapper.h"

clHumanProfile::clHumanProfile()
{
	iVictory=0;
	iLost=0;			//Поражения
	iHint=0;
	iSingle=0;
	iCampaign=0;
	iBattleSpeed=2;		//Скорость движения в бою
	iMapBorders=0;		//Тип границ генерируемой карты	
	bWarGrid=false;		//Сетка в бою
	bWarBar=false;		//Инфо панели в бою
	bWarEndTurn=false;	//Автопередача хода в бою
	bHerbOnMap=false;	//Выводить гербы на карту	
	bAutosave=true;
	bFinishTutorial=false;
	bDiplomacy=true;	//Дипломатия в одиночной игре
	bFog=true;			//Закрытая карта в одиночной игре
	iEnterData=0;
	iColor1=Random(16); 
	iColor2=Random(16); 
	iSign=Random(iMaxHerbSign); 
	ipAvatar=17; 
	iDifficulty=0;
	bTutorialMode=true;

	//Настройки игры
	iSndVolume=225;				//Громкость эффектов
	iMusVolume=255;				//Громкость музыки
	iScrollSpeed=SCROLL_SPEED;	//Скорость скроллинга
	iAutoLvlUp=0;				//Автоматическиё апгрейды юнитов до указанного ранга
	iAutoCombat=0;				//Всегда автобой
	for (int i=0; i<10; i++)
	{arMess[i]=true;};
	arMap[0]=false;
	arMap[1]=true;
	arMap[2]=true;
	arMap[3]=true;
	arMap[4]=true;
	arMap[5]=true;
	arMap[6]=false;
	arMap[7]=false;
	arMap[8]=false;
	arMap[9]=false;
};

clHumanProfile::~clHumanProfile()
{};


void clHumanProfile::ClearNames()
{
	sprintf(sName,(pFirstPhrase+216)->str);
	sprintf(sLand,(pFirstPhrase+217)->str);
	iVictory=0;
	iLost=0;			//Поражения
	iHint=0;
	iSndVolume=iSoundVolume;//Громкость эффектов
	iMusVolume=iMusicVolume;//Громкость музыки
	iScrollSpeed=SCROLL_SPEED;		//Скорость скроллинга
	iBattleSpeed=2;		//Скорость движения в бою
	iMapBorders=0;		//Тип границ генерируемой карты
	iAutoLvlUp=0;		//Автоматическиё апгрейды юнитов до указанного ранга
	bWarGrid=false;		//Сетка в бою
	bWarBar=false;		//Инфо панели в бою
	bWarEndTurn=false;	//Автопередача хода в бою
	bHerbOnMap=false;	//Выводить гербы на карту
	iAutoCombat=0;	//Всегда автобой
	bAutosave=true;
	bFinishTutorial=false;
	bDiplomacy=true;
	bFog=true;
	iEnterData=0;
	iColor1=Random(16); 
	iColor2=Random(16); 
	iSign=Random(iMaxHerbSign); 
	ipAvatar=17; 
	iDifficulty=0;
	Map.SetDefault();
	for (int i=0; i<10; i++)
	{arMess[i]=true;};
	arMap[0]=false;
	arMap[1]=true;
	arMap[2]=true;
	arMap[3]=true;
	arMap[4]=true;
	arMap[5]=true;
	arMap[6]=false;
	arMap[7]=false;
	arMap[8]=false;
	arMap[9]=false;
};

void clHumanProfile::Save()
{
	char FileName[100];
	if (iFileIndex==-1)
	{
		int Ind=0;
		al_ffblk ProFile;
		while (iFileIndex==-1)
		{
			sprintf(FileName,"Save\\player%d.prf",Ind);
			if (al_findfirst(FileName,&ProFile,FA_ALL)==0)
			{al_findclose(&ProFile);}
			else
			{iFileIndex=Ind;};
			Ind++;
		};
	};
	PACKFILE *WorkFile;	
	sprintf(FileName,"save\\player%d.prf",iFileIndex);
	WorkFile=pack_fopen(FileName,"wp");
	pack_fwrite(this,sizeof *this,WorkFile);
	pack_fclose(WorkFile);
};

void ShowWallpaper()
{
	int RndPic=Random(3);
	switch (RndPic)
	{
	case 0:
		blit(bmpStuff[77],screen1,0,0,0,0,1024,768);
		break;
	case 1:
		blit(bmpGUI_Screen01,screen1,0,0,0,0,1024,768);
		break;
	case 2:
		blit(bmpStuff[80],screen1,0,0,0,0,1024,768);	
		break;
	}
}

void SaveRecords()
{
	PACKFILE *WorkFile;	
	char FileName[100];
	sprintf(FileName,"save\\ruler.boo");
	WorkFile=pack_fopen(FileName,"wp");
	pack_fwrite(arRecords,(sizeof *arRecords)*10,WorkFile);
	pack_fclose(WorkFile);
};

void SaveCampRecords()
{
	PACKFILE *WorkFile;	
	char FileName[100];
	sprintf(FileName,"save\\eador.boo");
	WorkFile=pack_fopen(FileName,"wp");
	pack_fwrite(arCampRecords,(sizeof *arCampRecords)*10,WorkFile);
	pack_fclose(WorkFile);
};

int RunGame(bool bSave)
{
	iGameMode=2;
	if (bSave)
	{
		RunHourGlass(149, HOUR_SHIFT);
		CurGame.bAutoSave=true;
		WriteLog("Сохранение...");	
		CurGame.iActivePlayer=1;
		SaveGame(0);
	};	
	int LastPlayer;
	iActiveTrack=Random(MUZ_MAX_MAIN,1+iSeed/100,1+iSeed%100);
	ChangeMusic();
	bTimeShift=false;
	bExitGame=false;
	int AIS;
	while (!bExitGame)
	{	
		for (int i=1; ((i<=CurGame.iMaxPlayers)&&(!bExitGame)); i++)
		{
			bool bNewTurn=true;
			if (CurGame.iActivePlayer>0)
			{
				i=CurGame.iActivePlayer;
				CurGame.iActivePlayer=0;
				if (bSave)
				{bSave=false;}
				else
				{bNewTurn=false;};
			};
			if ((arPlayer[i].bInGame)&&(arPlayer[i].Control==1))
			{
				if (CurGame.iGameType==3)
				{
					bHourGlass=false;
					int All=0;
					for (int j=1; j<=CurGame.iMaxPlayers; j++)
					{
						if ((arPlayer[j].Control==1)&&(arPlayer[j].bInGame))
						{All++;};
					};
					if (All>1)
					{
						blit(bmpStuff[77],screen1,0,0,0,0,1024,768);
						WindowNextTurn(i);
					};
				};
				if (bNewTurn)
				{BeginMainTurn(i);};
				if (!bExitGame)
				{
					MainScreen(i);
					LastPlayer=i;
					if (((!bExitGame)||(iExitFlag==0))&&(bShowWallpaper))
					{
						ShowWallpaper();
					};
					OldProgress=0;
					NewProgress=0;
				};
			};			
		};
		if (!bExitGame)
		{		
			RunHourGlass(149, HOUR_SHIFT);
			CurGame.bAutoSave=true;
			WriteLog("Сохранение...");
	//		if (CurGame.iGameType==2)
			CurGame.iActivePlayer=LastPlayer;
			SaveGame(4000);
			CurGame.iActivePlayer=0;
			CurGame.iCurrentTurn++;
			bTimeShift=true;
	//		else
	//		{SaveGame(4000);};
			RunHourGlass(148, HOUR_SHIFT);			
		
			AIS=0;
			for (int i=1; i<=CurGame.iMaxPlayers; i++)
			{
				if ((arPlayer[i].bInGame)&&(arPlayer[i].Control!=1))			
				{AIS++;};
			};
			for (int i=1; ((i<=CurGame.iMaxPlayers)&&(!bExitGame)); i++)
			{
				if ((arPlayer[i].bInGame)&&(arPlayer[i].Control!=1))			
				{					
					if (!bExitGame)
					{
						RunHourGlass(148, HOUR_SHIFT);
						MakeTurnAI(i, 15000/AIS);
					};
				};
			};
			EndMainTurn();
			for (int i=1; ((i<=CurGame.iMaxPlayers)&&(!bExitGame)); i++)
			{
				if (arPlayer[i].bInGame)
				{
					EventServer(i);
				};
			};
			AIS=0;
			for (int i=1; i<=CurGame.iMaxPlayers; i++)
			{
				if ((arPlayer[i].bInGame)&&(arPlayer[i].Control!=1))			
				{AIS++;};
			};
			for (int i=1; ((i<=CurGame.iMaxPlayers)&&(!bExitGame)); i++)
			{
				if ((arPlayer[i].bInGame)&&(arPlayer[i].Control!=1))
				{
					BeginMainTurn(i);
					NewProgress+=3000/AIS;
				};				
			};	
		};
		
		if (bExitGame)
		{
			bool bSave=true;
			if (CurGame.iGameType==3)
			{
				for (int i=1; i<=CurGame.iMaxPlayers; i++)
				{if (arPlayer[i].pEvent){bSave=false;};};
			};
			if ((bSave)&&(iExitFlag==0))
			{
				RunHourGlass(149, HOUR_SHIFT);
				CurGame.bAutoSave=false;
				WriteLog("Сохранение...");
		//		if (CurGame.iGameType==2)
				CurGame.iActivePlayer=LastPlayer;
				SaveGame(22000);
		//		else
		//		{SaveGame("autosave.sav",22000);};
			};
			if ((iExitFlag>0)&&(CurGame.iGameType==1))
			{
				char sFile[100];
				sprintf(sFile,"save\\game%d.sav",arHumans[0].iFileIndex);
				delete_file(sFile);
				sprintf(sFile,"save\\map%d.map",arHumans[0].iFileIndex);
				delete_file(sFile);
			};
			if ((iExitFlag>0)&&(CurGame.iGameType==3))
			{
				char sFile[100];
				sprintf(sFile,"save\\game%d.hot",arHumans[0].iFileIndex);
				delete_file(sFile);
				sprintf(sFile,"save\\map%d.hot",arHumans[0].iFileIndex);
				delete_file(sFile);
			};
			UnloadGame();			
			set_mouse_sprite(bmpMousePointer[1]);
			mouse_pointer=1;
		};
		if (bExitGame)
		{
			if (iExitFlag==-1)
			{
				if (bShowWallpaper)
				{ShowWallpaper();}
				bExitGame=false;
				LoadGame(CurGame.iGameType);			
			};
		};
	};
	bHourGlass=false;
	return iExitFlag;
};


void SaveGame(int Progress)
{
	PACKFILE *WorkFile;
	char sFile [150], sBuff [150], sExt[10];
	switch (CurGame.iGameType)
	{
	case 1:
		sprintf(sFile,"save\\game%d.sav",arHumans[0].iFileIndex);
		sprintf(sExt,"bks");
		break;
	case 2:	
		sprintf(sFile,"save\\game%d.cmp",arHumans[0].iFileIndex);
		if (iGameMode==1)
		{sprintf(sExt,"bka");}
		else
		{sprintf(sExt,"bkc");};
		break;
	case 3:
		sprintf(sFile,"save\\game%d.hot",arHumans[0].iFileIndex);
		sprintf(sExt,"bkh");
		break;
	case 4:
		sprintf(sFile,"save\\game%d.lan",arHumans[0].iFileIndex);
		sprintf(sExt,"bkl");
		break;
	case 5:
		sprintf(sFile,"save\\game%d.net",arHumans[0].iFileIndex);
		sprintf(sExt,"bkn");
		break;
	};
	al_ffblk Fil;
	if (al_findfirst(sFile,&Fil,FA_ALL)==0)
	{
		replace_extension(sBuff,sFile,sExt,sizeof sBuff);
		if ((iGameMode!=1)&&(bTimeShift))
		{FileCopy(sFile,sBuff);};
	};
	al_findclose(&Fil);
	
//	sprintf(sFile,"save\\%s",sFileName);
	WorkFile=pack_fopen(sFile,"wp");
	char MODE; //0 - одиночная; 1 - кампания; 2 - кампания, астрал

	MODE=111;
	pack_putc(MODE,WorkFile);

///********* ВЕРСИЯ СОХРАНЕНИЯ *************
	MODE=107;
	pack_putc(MODE,WorkFile);

	if (iGameMode==1)
	{MODE=2;}
	else
	{
		if (CurGame.iGameType==2)
		{MODE=1;}
		else
		{MODE=0;};
	};	
	pack_putc(MODE,WorkFile);
	if (MODE<2)
	{
//Файл карты
//	pack_fwrite(&sMapFile,sizeof sMapFile,WorkFile);
//	NewProgress+=Progress/5;
//Общие данные
	pack_fwrite(&CurGame,sizeof CurGame,WorkFile);
	pack_fwrite(&WorldRules,sizeof WorldRules,WorkFile);
	int PRG=NewProgress;
//Провинции	
	for (int i=1; i<=MapSize; i++)
	{
		NewProgress=PRG+Progress/2*i/MapSize;
		dcProvince *PR=(pFirstProvince+i);
		pack_fwrite(PR,sizeof *PR,WorkFile);		
		if (PR->pGuard)
		{
			pack_putc(1,WorkFile);
			pack_fwrite(PR->pGuard,sizeof *PR->pGuard,WorkFile);
			for (int j=0; j<MAX_SQUAD; j++)
			{
				if (PR->pGuard->pSquad[j])
				{
					pack_putc(1,WorkFile);
					pack_fwrite(PR->pGuard->pSquad[j],sizeof *PR->pGuard->pSquad[j],WorkFile);

				}
				else
				{
					pack_putc(0,WorkFile);
				};
			};
		}
		else
		{
			pack_putc(0,WorkFile);
		};
		if (PR->pGarrison)
		{
			pack_putc(1,WorkFile);
			pack_fwrite(PR->pGarrison,sizeof *PR->pGarrison,WorkFile);
			for (int j=0; j<MAX_SQUAD; j++)
			{
				if (PR->pGarrison->pSquad[j])
				{
					pack_putc(1,WorkFile);
					pack_fwrite(PR->pGarrison->pSquad[j],sizeof *PR->pGarrison->pSquad[j],WorkFile);

				}
				else
				{
					pack_putc(0,WorkFile);
				};
			};
		}
		else
		{
			pack_putc(0,WorkFile);
		};
		for (int k=0; k<MAX_PROVINCE_SITE; k++)
		{
			if (PR->arSiteGuard[k])
			{
				pack_putc(1,WorkFile);
				pack_fwrite(PR->arSiteGuard[k],sizeof *PR->arSiteGuard[k],WorkFile);
				for (int j=0; j<MAX_SQUAD; j++)
				{
					if (PR->arSiteGuard[k]->pSquad[j])
					{
						pack_putc(1,WorkFile);
						pack_fwrite(PR->arSiteGuard[k]->pSquad[j],sizeof *PR->arSiteGuard[k]->pSquad[j],WorkFile);
					}
					else
					{
						pack_putc(0,WorkFile);
					};
				};
			}
			else
			{
				pack_putc(0,WorkFile);
			};
		};		
	};
//Игроки
	pack_fwrite(&arPlayer,sizeof arPlayer,WorkFile);
	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		for (int i=0; i<MAX_HERO; i++)
		{
			if (arPlayer[PL].arHero[i])
			{
				pack_putc(1,WorkFile);
				pack_fwrite(arPlayer[PL].arHero[i],sizeof *arPlayer[PL].arHero[i],WorkFile);
				for (int j=0; j<MAX_SQUAD; j++)
				{
					if (arPlayer[PL].arHero[i]->pSquad[j])
					{
						pack_putc(1,WorkFile);
						pack_fwrite(arPlayer[PL].arHero[i]->pSquad[j],sizeof *arPlayer[PL].arHero[i]->pSquad[j],WorkFile);
					}
					else
					{
						pack_putc(0,WorkFile);
					};
				};
			}
			else
			{
				pack_putc(0,WorkFile);
			};
		};

		pack_fwrite(&iMaxDefender,sizeof iMaxDefender,WorkFile);
		for (int i=0; i<=iMaxDefender; i++)
		{
//			textprintf(screen,Font1,0,40,COLOR_2,"  %d, %d    ",PL,i);
			pack_fwrite((arPlayer[PL].pDefender+i),sizeof *(arPlayer[PL].pDefender+i),WorkFile);
		};

		pack_fwrite(&iMaxOuterBld,sizeof iMaxOuterBld,WorkFile);
		for (int i=0; i<=iMaxOuterBld; i++)
		{
			pack_fwrite((arPlayer[PL].pOuterBld+i),sizeof *(arPlayer[PL].pOuterBld+i),WorkFile);
		};

		pack_fwrite(&iMaxRitual,sizeof iMaxRitual,WorkFile);
		for (int i=0; i<=iMaxRitual; i++)
		{
			pack_fwrite((arPlayer[PL].pRitual+i),sizeof *(arPlayer[PL].pRitual+i),WorkFile);
			pack_fwrite((arPlayer[PL].pRitualCD+i),sizeof *(arPlayer[PL].pRitualCD+i),WorkFile);
		};	

		for (int i=1; i<=MapSize; i++)
		{
			pack_fwrite((arPlayer[PL].pFog+i),sizeof *(arPlayer[PL].pFog+i),WorkFile);
		};

		pack_fwrite(&iMaxEvent,sizeof iMaxEvent,WorkFile);
		for (int i=1; i<=iMaxEvent; i++)
		{
			pack_fwrite((arPlayer[PL].pStory+i),sizeof *(arPlayer[PL].pStory+i),WorkFile);
			pack_fwrite((arPlayer[PL].pPlace+i),sizeof *(arPlayer[PL].pPlace+i),WorkFile);
		};
	};
	NewProgress+=Progress/5;
	pack_fwrite(&iMaxPureItem,sizeof iMaxPureItem,WorkFile);

	//События
	/*for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		int Events=0;
		dcEvent *EV=arPlayer[PL].pEvent;
		while (EV)
		{
			Events++;
			EV=EV->pNext;
		};
		pack_fwrite(&Events,sizeof Events,WorkFile);
		EV=arPlayer[PL].pEvent;
		while (EV)
		{			
			pack_fwrite(EV,sizeof *EV,WorkFile);
			int Wrk=-1;
			for (int i=0; i<MAX_HERO; i++)
			{
				if (arPlayer[EV->Player1].arHero[i]==EV->Hero1)
				{Wrk=i;};
			};
			Wrk=-1;
			pack_fwrite(&Wrk,sizeof Wrk,WorkFile);
			for (int i=0; i<MAX_HERO; i++)
			{
				if (arPlayer[EV->Player2].arHero[i]==EV->Hero2)
				{Wrk=i;};
			};
			pack_fwrite(&Wrk,sizeof Wrk,WorkFile);
			Wrk=EV->Prov1->iNumber;
			pack_fwrite(&Wrk,sizeof Wrk,WorkFile);
			Wrk=EV->Prov2->iNumber;
			pack_fwrite(&Wrk,sizeof Wrk,WorkFile);
			EV=EV->pNext;
		};
	};*/
//Магазины
	pack_fwrite(&iMaxShop,sizeof iMaxShop,WorkFile);
	for (int i=0; i<iMaxShop; i++)
	{
		pack_fwrite((pFirstShop+i),sizeof *(pFirstShop+i),WorkFile);
	};
	NewProgress+=Progress/5;
	int Count=0;
	dcRitual *RR=pRitual;
	while (RR)
	{
		RR=RR->pNext;
		Count++;
	};
	pack_fwrite(&Count,sizeof Count,WorkFile);
	RR=pRitual;
	while (RR)
	{
		pack_fwrite(RR,sizeof *RR,WorkFile);
		RR=RR->pNext;
	};
	NewProgress+=Progress/10;
	};
	if (MODE>0)
	{
	/*	{
		int Max=Campaign.iMaxShards;
		dcShard *SH=pFirstShard;
		while (SH)
		{
			while ((SH->pNext)&&(SH->pNext->iLevel<=0))
			{
				SH->pNext=SH->pNext->pNext;
				Max--;				
			};
			pack_fwrite(SH,sizeof *SH,WorkFile);			
			SH=SH->pNext;
		};
		Campaign.iMaxShards=Max;
		};*/


		//Сохранение данных кампании
		pack_fwrite(&Campaign,sizeof Campaign,WorkFile);
//		pack_fwrite(&arCampPlayer,sizeof arCampPlayer,WorkFile);
		//Осколки миров	
		dcShard *SH=pFirstShard;
		for (int i=0; i<Campaign.iMaxShards; i++)		
		{			
			pack_fwrite(SH,sizeof *SH,WorkFile);
			SH=SH->pNext;
		};
	};
	pack_fwrite(&Tutorial,sizeof Tutorial,WorkFile);

		
//	dcEvent *pEvent;					//Список событий
//	UI_Mask arHeroMask[MAX_HERO];		//Массив масок для фигурок героев	
	pack_fclose(WorkFile);
	if (iGameMode==1){FileCopy(sFile,sBuff);};
	
};

//void ClearGameFiles(char *sFileName)
//{
//	PACKFILE *WorkFile;
//	char sFile [150];
//	sprintf(sFile,"save\\%s",sFileName);
//	WorkFile=pack_fopen(sFile,"rp");
//	char MODE=pack_getc(WorkFile);
//	if (MODE<2)
//	{
////Файл карты
//		pack_fread(&sMapFile,sizeof sMapFile,WorkFile);
//		pack_fclose(WorkFile);
//		delete_file(sMapFile.str);
		


bool LoadGame(int iModd)
{	
	bTimeShift=false;
	NewProgress=0;
	OldProgress=0;
	RunHourGlass(153, 0);
	PACKFILE *WorkFile;
	dcOldHero* WrkHero=new dcOldHero;
	char Version=102;
	char sFile [150];
	switch (iModd)
	{
	case 1:
		sprintf(sFile,"save\\game%d.sav",arHumans[0].iFileIndex);
		break;
	case 2:
		sprintf(sFile,"save\\game%d.cmp",arHumans[0].iFileIndex);
		break;
	case 3:
		sprintf(sFile,"save\\game%d.hot",arHumans[0].iFileIndex);
		break;
	case 4:
		sprintf(sFile,"save\\game%d.lan",arHumans[0].iFileIndex);
		break;
	case 5:
		sprintf(sFile,"save\\game%d.net",arHumans[0].iFileIndex);
		break;
	};
	
//	sprintf(sFile,"save\\%s",sFileName);
	WorkFile=pack_fopen(sFile,"rp");
	char MODE=pack_getc(WorkFile);
	if (MODE==111)
	{
		Version=pack_getc(WorkFile);
		MODE=pack_getc(WorkFile);
	};
//	MODE=pack_getc(WorkFile);	
	if (MODE<2)
	{
//Файл карты
//	pack_fread(&sMapFile,sizeof sMapFile,WorkFile);
//	pack_fclose(WorkFile);
	
//	WorkFile=pack_fopen(sFile,"rp");
//	MODE=pack_getc(WorkFile);
//	pack_fread(&sMapFile,sizeof sMapFile,WorkFile);

//Общие данные
	pack_fread(&CurGame,sizeof CurGame,WorkFile);
	if (Version>=107)
	{pack_fread(&WorldRules,sizeof WorldRules,WorkFile);}
	else
	{WorldRules.iMode=0;}
	LoadMap();
	NewProgress=1000;
//Провинции	
	dcProvince *PR=new dcProvince;
	for (int i=1; i<=MapSize; i++)
	{
		NewProgress=1000+i*1000/MapSize;
		pack_fread(PR,sizeof *PR,WorkFile);	
		PR->sName=(pFirstProvince+PR->iNumber)->sName;		
		PR->Pic=NULL;
		PR->Obj=NULL;		
		if (pack_getc(WorkFile))
		{
			PR->pGuard=new dcHero;
			if (Version==102)
			{
				pack_fread(WrkHero,sizeof *WrkHero,WorkFile);
				PR->pGuard->ConvertOld(*WrkHero);
			}
			else
			{
				pack_fread(PR->pGuard,sizeof *PR->pGuard,WorkFile);
			};
		//	if ((PR->pGuard->ipPlayer>0)&&(PR->pGuard->ipPlayer!=PR->ipPlayer))
		//	{PR->pGuard->ipPlayer=PR->ipPlayer;};
			for (int j=0; j<MAX_SQUAD; j++)
			{
				if (pack_getc(WorkFile))
				{					
					PR->pGuard->pSquad[j]=new dcUnit;
					pack_fread(PR->pGuard->pSquad[j],sizeof *PR->pGuard->pSquad[j],WorkFile);
					PR->pGuard->pSquad[j]->pHero=NULL;
					if ((PR->pGuard->pSquad[j]->GetUnitType()<=0)||(PR->pGuard->pSquad[j]->GetUnitType()>iMaxUnitType))
					{PR->pGuard->pSquad[j]->SetUnitType(1);};
				}
				else
				{
					PR->pGuard->pSquad[j]=NULL;
				};
			};
			/*if (PR->pGuard->pSquad[0]==NULL)
			{
				for (int i=1; i<MAX_SQUAD; i++)
				{
					if (PR->pGuard->pSquad[i])
					{
						PR->pGuard->pSquad[0]=PR->pGuard->pSquad[i];
						PR->pGuard->pSquad[i]=NULL;
						break;
					};
				};
			};*/
		}
		else
		{
			PR->pGuard=NULL;
		};
		
		if (pack_getc(WorkFile))
		{
			PR->pGarrison=new dcHero;
			if (Version==102)
			{
				pack_fread(WrkHero,sizeof *WrkHero,WorkFile);
				PR->pGarrison->ConvertOld(*WrkHero);
			}
			else
			{
				pack_fread(PR->pGarrison,sizeof *PR->pGarrison,WorkFile);
			};
			for (int j=0; j<MAX_SQUAD; j++)
			{
				if (pack_getc(WorkFile))
				{
					PR->pGarrison->pSquad[j]=new dcUnit;
					pack_fread(PR->pGarrison->pSquad[j],sizeof *PR->pGarrison->pSquad[j],WorkFile);
					PR->pGarrison->pSquad[j]->pHero=NULL;
					if ((PR->pGarrison->pSquad[j]->GetUnitType()<=0)||(PR->pGarrison->pSquad[j]->GetUnitType()>iMaxUnitType))
					{
						delete PR->pGarrison->pSquad[j];
						PR->pGarrison->pSquad[j]=NULL;
					};
				}
				else
				{
					PR->pGarrison->pSquad[j]=NULL;
				};
			};
		}
		else
		{
			PR->pGarrison=NULL;
		};
		
		for (int k=0; k<MAX_PROVINCE_SITE; k++)
		{
			if (pack_getc(WorkFile))
			{
				PR->arSiteGuard[k]=new dcHero;
				if (Version==102)
				{
					pack_fread(WrkHero,sizeof *WrkHero,WorkFile);
					PR->arSiteGuard[k]->ConvertOld(*WrkHero);
				}
				else
				{
					pack_fread(PR->arSiteGuard[k],sizeof *PR->arSiteGuard[k],WorkFile);
				};
				for (int j=0; j<MAX_SQUAD; j++)
				{
					if (pack_getc(WorkFile))
					{						
						PR->arSiteGuard[k]->pSquad[j]=new dcUnit;
						pack_fread(PR->arSiteGuard[k]->pSquad[j],sizeof *PR->arSiteGuard[k]->pSquad[j],WorkFile);
						PR->arSiteGuard[k]->pSquad[j]->pHero=NULL;
						if (PR->arSiteGuard[k]->pSquad[j]->GetUnitType()>1000)
						{PR->arSiteGuard[k]->pSquad[j]->SetUnitType(1);};
					}
					else
					{
						PR->arSiteGuard[k]->pSquad[j]=NULL;
					};
				};
			}
			else
			{
				PR->arSiteGuard[k]=NULL;
			};
		};
		*(pFirstProvince+PR->iNumber)=*PR;
	//	if ((i>=93)&&((pFirstProvince+93)->Pic))
	//	{
	//		FileLog("Pic!!!");
	//	};
	};	

//Игроки
	WriteLog("Игроки");
/*	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		arPlayer[PL].Init(true,PL);
		
	};
*/
	for (int PL=CurGame.iMaxPlayers+1; PL<MAX_PLAYERS; PL++)
	{
		arPlayer[PL].bInGame=false;
	};
	pack_fread(&arPlayer,sizeof arPlayer,WorkFile);
	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		arPlayer[PL].pDefender=new int[iMaxDefender+1];
		arPlayer[PL].pOuterBld=new int[iMaxOuterBld+1];	
		arPlayer[PL].pRitual=new int[iMaxRitual+1];
		arPlayer[PL].pRitualCD=new int[iMaxRitual+1];
		arPlayer[PL].pFog=new int [MapSize+1];
		arPlayer[PL].pStory=new int [iMaxEvent+1];
		arPlayer[PL].pPlace=new int [iMaxEvent+1];
		arPlayer[PL].pEvent=NULL;
		for (int i=0; i<MAX_HERO; i++)
		{
			if (pack_getc(WorkFile))
			{
				arPlayer[PL].arHero[i]=new dcHero;
				if (Version==102)
				{
					pack_fread(WrkHero,sizeof *WrkHero,WorkFile);
					arPlayer[PL].arHero[i]->ConvertOld(*WrkHero);
				}
				else
				{
					pack_fread(arPlayer[PL].arHero[i],sizeof *arPlayer[PL].arHero[i],WorkFile);
				};
			//	if (arPlayer[PL].arHero[i]->Pic>0)
			//	{arPlayer[PL].arHero[i]=NULL;}
			//	else
			//	{
				arPlayer[PL].arHero[i]->pstName=(pName[arPlayer[PL].arHero[i]->ipClass1-1]+arPlayer[PL].arHero[i]->ipName);				
//				textprintf(screen, Font1, 0,0,COLOR_2,"%d",arPlayer[PL].arHero[i]->apSkill[0][0]);
//				WriteLog(arPlayer[PL].arHero[i]->pstName->str);
//				readkey();
				if (arPlayer[PL].arHero[i]->ipPortrait<0)
				{arPlayer[PL].arHero[i]->ipPortrait=1;};
				arPlayer[PL].arHero[i]->Pic=bmpPortrait[arPlayer[PL].arHero[i]->ipPortrait];
				arPlayer[PL].arHero[i]->smPic=bmpSmallPort[arPlayer[PL].arHero[i]->ipPortrait];				
				arPlayer[PL].arHero[i]->pProvince=(pFirstProvince+arPlayer[PL].arHero[i]->ipProvince);
				/*if (arPlayer[PL].arHero[i]->pProvince->ipTerrain==0)
				{
					arPlayer[PL].arHero[i]->pProvince=(pFirstProvince+arPlayer[PL].ipProvince);
					arPlayer[PL].arHero[i]->ipProvince=arPlayer[PL].ipProvince;
				};*/


				for (int j=0; j<MAX_SQUAD; j++)
				{
					if (pack_getc(WorkFile))
					{
						arPlayer[PL].arHero[i]->pSquad[j]=new dcUnit;
						pack_fread(arPlayer[PL].arHero[i]->pSquad[j],sizeof *arPlayer[PL].arHero[i]->pSquad[j],WorkFile);
						if (j==0)
						{
							arPlayer[PL].arHero[i]->pSquad[j]->pHero=arPlayer[PL].arHero[i];
						}
						else
						{
							arPlayer[PL].arHero[i]->pSquad[j]->pHero=NULL;
						};
						if ((arPlayer[PL].arHero[i]->pSquad[j]->GetUnitType()<=0)||(arPlayer[PL].arHero[i]->pSquad[j]->GetUnitType()>iMaxUnitType))
						{
							delete arPlayer[PL].arHero[i]->pSquad[j];
							arPlayer[PL].arHero[i]->pSquad[j]=NULL;
						};
					}
					else
					{
						arPlayer[PL].arHero[i]->pSquad[j]=NULL;
					};
				};
				for (int j=1; j<=INVENTORY_SIZE; j++)
				{
					if ((arPlayer[PL].arHero[i]->arInventory[j][0]<0)||(arPlayer[PL].arHero[i]->arInventory[j][0]>iMaxItem))
					{
						arPlayer[PL].arHero[i]->arInventory[j][0]=0;
						arPlayer[PL].arHero[i]->arInventory[j][1]=0;
					};
				};
				if (arPlayer[PL].arHero[i]->iExp<arPlayer[PL].arHero[i]->ExpForLevel(arPlayer[PL].arHero[i]->iLevel))
				{arPlayer[PL].arHero[i]->iExp=arPlayer[PL].arHero[i]->ExpForLevel(arPlayer[PL].arHero[i]->iLevel);}
	//		};
			}
			else
			{
				arPlayer[PL].arHero[i]=NULL;
			};
		};
		NewProgress=2100;

		WriteLog("Защитники");
		int iWrk, iMax=iMaxDefender;
		if (Version>102)
		{
			pack_fread(&iMax,sizeof iMax,WorkFile);
		}
		else
		{iMax=65;}
		for (int i=0; i<=iMax; i++)
		{
			if (i<=iMaxDefender)
			{pack_fread((arPlayer[PL].pDefender+i),sizeof *(arPlayer[PL].pDefender+i),WorkFile);}
			else
			{pack_fread(&iWrk,sizeof iWrk,WorkFile);};
		};
		if (iMax<iMaxDefender)
		{
			for (int i=iMax+1; i<=iMaxDefender; i++)
			{
				*(arPlayer[PL].pDefender+i)=0;
			};
		};
		NewProgress=2200;

		WriteLog("Постройки");
		iMax=iMaxOuterBld;
		if (Version>102)
		{
			pack_fread(&iMax,sizeof iMax,WorkFile);
		}
		else
		{iMax=40;};
		for (int i=0; i<=iMax; i++)
		{
			if (i<=iMaxOuterBld)
			{pack_fread((arPlayer[PL].pOuterBld+i),sizeof *(arPlayer[PL].pOuterBld+i),WorkFile);}
			else
			{pack_fread(&iWrk,sizeof iWrk,WorkFile);};
		};
		if (iMax<iMaxOuterBld)
		{
			for (int i=iMax+1; i<=iMaxOuterBld; i++)
			{
				*(arPlayer[PL].pOuterBld+i)=0;
			};
		};
		
		NewProgress=2300;

		WriteLog("Ритуалы");
		iMax=iMaxRitual;
		if (Version>102)
		{
			pack_fread(&iMax,sizeof iMax,WorkFile);
		}
		else
		{iMax=24;};
		for (int i=0; i<=iMax; i++)
		{
			if (i<=iMaxRitual)
			{
				pack_fread((arPlayer[PL].pRitual+i),sizeof *(arPlayer[PL].pRitual+i),WorkFile);
				pack_fread((arPlayer[PL].pRitualCD+i),sizeof *(arPlayer[PL].pRitualCD+i),WorkFile);
			}
			else
			{
				pack_fread(&iWrk,sizeof iWrk,WorkFile);
				pack_fread(&iWrk,sizeof iWrk,WorkFile);
			};
		};
		if (iMax<iMaxRitual)
		{
			for (int i=iMax+1; i<=iMaxRitual; i++)
			{
				*(arPlayer[PL].pRitual+i)=0;
				*(arPlayer[PL].pRitualCD+i)=0;
			};
		};
		NewProgress=2400;
		//Туман
		for (int i=1; i<=MapSize; i++)
		{			
			pack_fread((arPlayer[PL].pFog+i),sizeof *(arPlayer[PL].pFog+i),WorkFile);
		};
		*(arPlayer[PL].pFog)=0;
		NewProgress=2500;
		//События
		iMax=iMaxEvent;
		if (Version>102)
		{
			pack_fread(&iMax,sizeof iMax,WorkFile);
		}
		else
		{iMax=1030;};
		for (int i=1; i<=iMax; i++)
		{
			if (i<=iMaxEvent)
			{
				pack_fread((arPlayer[PL].pStory+i),sizeof *(arPlayer[PL].pStory+i),WorkFile);
				pack_fread((arPlayer[PL].pPlace+i),sizeof *(arPlayer[PL].pPlace+i),WorkFile);
			}
			else
			{
				pack_fread(&iWrk,sizeof iWrk,WorkFile);
				pack_fread(&iWrk,sizeof iWrk,WorkFile);
			};
		};
		if (iMax<iMaxEvent)
		{
			for (int i=iMax+1; i<=iMaxRitual; i++)
			{
				*(arPlayer[PL].pStory+i)=-1;
				*(arPlayer[PL].pPlace+i)=0;
			};
		};		
		NewProgress=2600;
		
		if (arPlayer[PL].Control==1)
		{
			PACKFILE *PrfFile;
			char stName [150];
			sprintf(stName,"save\\player%d.prf",arPlayer[PL].ipProfile);
			PrfFile=pack_fopen(stName,"rp");
			pack_fread(&arHumans[arPlayer[PL].iColor],sizeof arHumans[arPlayer[PL].iColor],PrfFile);
			pack_fclose(PrfFile);
		};
		arPlayer[PL].MakeFlags();
	
	};

//	arPlayer[1].arTreasury[20][0]=395;
//	arPlayer[1].arTreasury[20][1]=1;

	//Проверка изменения количества предметов
	int OldMaxItem=394;
	if (Version>105)
	{pack_fread(&OldMaxItem,sizeof OldMaxItem,WorkFile);};
	int DI=iMaxPureItem-OldMaxItem, MX=GetMin(iMaxPureItem,OldMaxItem);
	if (OldMaxItem!=iMaxPureItem)
	{
		for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
		{
			if (arPlayer[PL].bInGame)
			{
				for (int i=0; i<TREASURY_SIZE; i++)
				{
					if (arPlayer[PL].arTreasury[i][0]>MX)
					{arPlayer[PL].arTreasury[i][0]+=DI;};
				};
				for (int HR=0; HR<MAX_HERO; HR++)
				{
					if (arPlayer[PL].arHero[HR])
					{
						for (int i=15; i<=INVENTORY_SIZE; i++)
						{
							if (arPlayer[PL].arHero[HR]->arInventory[i][0]>MX)
							{arPlayer[PL].arHero[HR]->arInventory[i][0]+=DI;};
						};
						for (int i=0; i<HERO_QUEST_LIMIT; i++)
						{
							if ((arPlayer[PL].arHero[HR]->arQuest[i].Index>0)&&((pFirstQuest+arPlayer[PL].arHero[HR]->arQuest[i].Index)->iType==7))
							{
								if ((pFirstProvince+arPlayer[PL].arHero[HR]->arQuest[i].ipProvince)->arSite[arPlayer[PL].arHero[HR]->arQuest[i].ipSite][5]>MX)
								{(pFirstProvince+arPlayer[PL].arHero[HR]->arQuest[i].ipProvince)->arSite[arPlayer[PL].arHero[HR]->arQuest[i].ipSite][5]+=DI;};
							};
						};
					};
				};
			};
		};		
		for (int Prov=1; Prov<=MapSize; Prov++)
		{
			PR=(pFirstProvince+Prov);
			for (int i=0; i<MAX_PROVINCE_SITE; i++)
			{
				if ((PR->arSite[i][3]>0)&&(PR->arSite[i][3]<=iMaxQuest)&&((pFirstQuest+PR->arSite[i][3])->iType==7)&&(PR->arSite[i][5]>MX))
				{
					PR->arSite[i][5]+=DI;
				};
			};
		};
	};
	//События
	/*for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{		
		int Events;
		pack_fread(&Events,sizeof Events,WorkFile);
		arPlayer[PL].pEvent=NULL;
		dcEvent *EV;
		for (int i=0; i<Events; i++)
		{
			EV=new dcEvent(0,0,0,0,NULL,NULL,NULL,NULL,NULL);
			pack_fread(EV,sizeof *EV,WorkFile);
			int Wrk;
			pack_fread(&Wrk,sizeof Wrk,WorkFile);
			EV->Hero1=arPlayer[EV->Player1].arHero[Wrk];
			pack_fread(&Wrk,sizeof Wrk,WorkFile);
			EV->Hero2=arPlayer[EV->Player2].arHero[Wrk];
			pack_fread(&Wrk,sizeof Wrk,WorkFile);
			EV->Prov1=(pFirstProvince+Wrk);
			pack_fread(&Wrk,sizeof Wrk,WorkFile);
			EV->Prov2=(pFirstProvince+Wrk);
			EV->pNext=arPlayer[PL].pEvent;
			arPlayer[PL].pEvent=EV;
		};		
	};*/

//Магазины
	pack_fread(&iMaxShop,sizeof iMaxShop,WorkFile);
	pFirstShop=new dcShop[iMaxShop];
	for (int i=0; i<iMaxShop; i++)
	{
		pack_fread((pFirstShop+i),sizeof *(pFirstShop+i),WorkFile);
		if (DI!=0)
		{
			for (int j=0; j<SHOP_SIZE; j++)
			{			
				if ((pFirstShop+i)->Items[j][0]>MX)
				{(pFirstShop+i)->Items[j][0]+=DI;};
			};
		};
	};
	NewProgress=2700;
//Действующие ритуалы
	int Count=0;	
	pack_fread(&Count,sizeof Count,WorkFile);
	pRitual=NULL;
	dcRitual *RR;
	for (int i=0; i<Count; i++)
	{
		RR=new dcRitual();
		pack_fread(RR,sizeof *RR,WorkFile);
		RR->pNext=pRitual;
		pRitual=RR;
	};
	NewProgress=2800;
	FillObstracleMap(19200);
	}
	else
	{
		CurGame.iCurrentTurn=-1;
		CurGame.iGameType=2;
	};

	if (MODE>0)
	{	
		//Загрузка данных кампании
		pack_fread(&Campaign,sizeof Campaign,WorkFile);		

		//Осколки миров		
		dcShard *SH=NULL, *SH1=NULL;
		for (int i=0; i<Campaign.iMaxShards; i++)		
		{
			SH=new dcShard();
			pack_fread(SH,sizeof *SH,WorkFile);
			SH->pNext=SH1;
	//		if (SH->Terrain>=0)
	//		{
			SH->Pic=bmpShard[SH->Terrain];
			SH->Select=bmpShardSelect[SH->Terrain];
	//		}
	//		else
	//		{
	//			SH->Pic=bmpShard[1];
	//			SH->Select=bmpShardSelect[1];
	//		};
			SH1=SH;
		};
		pFirstShard=SH;
		if (Version==104)
		{
			dcShard *SH=pFirstShard;
			while (SH)
			{
				if (SH->iLevel>0)
				{
					SH->FillBonus();			
				};
				SH=SH->pNext;
			};
		};
	};
	pack_fread(&Tutorial,sizeof Tutorial,WorkFile);
	pack_fclose(WorkFile);
	if ((Tutorial.Map)&&(MODE<2))
	{
		stString *STR=new stString;
		sprintf(STR->str,arPlayer[1].GetName());
		arPlayer[1].arHero[0]->pstName=STR;
		arPlayer[1].arHero[0]->Pic=bmpAvatar[17];
		arPlayer[1].arHero[0]->smPic=bmpSmallAvatar[17];
	};
	if ((MODE>0)&&(Version<104))
	{
		Campaign.arJournal[978]=arCampDiff[Campaign.iDifficulty][3];
		if (Tutorial.Bonus==0)
		{Campaign.arJournal[979]=-1;}
		else
		{Campaign.arJournal[979]=Campaign.iDifficulty;}
	};
	if ((!Tutorial.Map)&&(MODE>0))
	{
		for (int i=0; i<MAX_PLAYERS; i++)
		{		
			for (int j=1; j<=iMaxInnerBld; j++)
			{
				if ((pFirstInnerBld+j)->iLevel<=0)
				{Campaign.arCampPlayer[i].arBld[j]=1;};
			};
		};

	};
	if (CurGame.bAutoSave)
	{arPlayer[CurGame.iActivePlayer].iTimeShift++;};
	if (bDemo)
	{
		if (GetWorldSize(0)>=2)
		{
			bHourGlass=false;
			int Answer=(pFirstDialog+1241)->Show(1,1,0,0,NULL,NULL);
			if (Answer==2)
			{OpenWeb();};
			if (MODE<2)
			{UnloadGame();};
			return false;
		};
	};	
	return true;
};

bool IsPlayerPicked (int PL, int Max)
{
	if (PL==0)
	{return false;};
	int Count=0;
	for (int i=1; i<=Max; i++)
	{
		if ((arPlayer[i].Control>1)&&(arPlayer[i].ipProfile==PL))
		{Count++;};
	};
	if (Count>1)
	{return true;}
	else
	{return false;};
};

bool NewGame(int iSeed, int GameType)
{
	BITMAP *bmpScr=create_bitmap(1024,768);		
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpScr);	
	arPlayer[1].Control=1;
	arPlayer[1].ipProfile=iActivePlayer;
	arPlayer[1].iColor=0;
	arPlayer[1].iDifficulty=arHumans[0].iDifficulty;
	clMap Map=arHumans[0].Map;
	int Size=(Map.X-6)/2;


	for (int i=2; i<17; i++)
	{
		arPlayer[i].iDifficulty=arHumans[0].iDifficulty;
		arPlayer[i].Control=2;
		arPlayer[i].ipProfile=0;		
	};
	UI_Button1 buttAccept;
	objGUI=buttAccept.Init(929,698,1,2,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttAccept.Activate(1);
	buttAccept.iHint=226;

	UI_Button1 buttExit;
	objGUI=buttExit.Init(851,698,1,2,bmpButton[309],bmpButton[310],bmpButton[311],bmpButton[311],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=106;


	UI_Button1 buttMapL;
	objGUI=buttMapL.Init(742,37,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	buttMapL.Activate(1);
	
	UI_Button1 buttMapR;
	objGUI=buttMapR.Init(921,37,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
	buttMapR.Activate(1);

	UI_Button1 buttPlL;
	objGUI=buttPlL.Init(92,37,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	buttPlL.Activate(1);
	
	UI_Button1 buttPlR;
	objGUI=buttPlR.Init(272,37,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
	buttPlR.Activate(1);

	UI_Button1 buttRuleL[16], buttRuleR[16], buttDiffL[16], buttDiffR[16];
	UI_Mask mskPlayer[16];
	for (int i=0; i<16; i++)
	{
		objGUI=buttRuleL[i].Init(91+(i%4)*248,97+((i-i%4)/4)*130,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
		objGUI=buttRuleR[i].Init(171+(i%4)*248,97+((i-i%4)/4)*130,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
		objGUI=buttDiffL[i].Init(48+(i%4)*248,169+((i-i%4)/4)*130,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
		objGUI=buttDiffR[i].Init(222+(i%4)*248,169+((i-i%4)/4)*130,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
		mskPlayer[i].Init(181+(i%4)*248,70+((i-i%4)/4)*130,1,bmpStuff[55],bmpEmpty,NULL,-1);
	};

	UI_Mask mskDiplo;
	objGUI=mskDiplo.Init(100-6,661-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);

	UI_Mask mskFog;
	objGUI=mskFog.Init(297-6,661-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);

	UI_Mask mskMap;
	objGUI=mskMap.Init(623-6,661-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);

	int MaxHumans=0, Humans=0, arHumanID[16];
	for (int i=0; i<16; i++)
	{arHumanID[i]=-1;};
	al_ffblk ProFile;
	clHumanProfile FileBase[200];
	PACKFILE *WorkFile;
	char stName[20], FName[100];
	arHumanID[0]=1;
	sprintf(stName,"player%d.prf",arHumans[0].iFileIndex);
	if ((GameType>2)&&(al_findfirst("Save\\player*.prf",&ProFile,FA_ALL)==0))
	{		
		do {			
			bool bThisOne=true;
			for (int i=6; i<11; i++)
			{
				if	(stName[i]!=ProFile.name[i])
				{bThisOne=false;};
			};
			if (bThisOne)
			{arHumanID[0]=MaxHumans;};
		//	if (!bThisOne)
			{
				sprintf(FName,"Save\\%s",ProFile.name);
				WorkFile=pack_fopen(FName,"rp");
				pack_fread(&FileBase[MaxHumans],sizeof FileBase[MaxHumans],WorkFile);
				pack_fclose(WorkFile);				
				MaxHumans++;
			};			
		} while ((al_findnext(&ProFile)==0)&&(MaxHumans<200));
		al_findclose(&ProFile);
	};

	

	bool bRepeat=true, bGenerate=true;
	while (bRepeat)
	{
		blit(bmpGUI_NewGame,bmpScr,0,0,0,0,bmpGUI_NewGame->w,bmpGUI_NewGame->h);
		textprintf_centre_ex(bmpScr,Font1,512,21,COLOR_1,-1,(pFirstPhrase+227)->str);
		textprintf_centre_ex(bmpScr,Font1,193,21,COLOR_1,-1,(pFirstPhrase+229)->str);
		textprintf_centre_ex(bmpScr,Font1,193,48,COLOR_1,-1,"%d",Map.Players);
		textprintf_centre_ex(bmpScr,Font1,842,21,COLOR_1,-1,(pFirstPhrase+228)->str);
		textprintf_centre_ex(bmpScr,Font1,842,48,COLOR_1,-1,arShardSize[Size].sName);
		textprintf_centre_ex(bmpScr,Font1,128,628,COLOR_1,-1,(pFirstPhrase+98)->str);
		textprintf_centre_ex(bmpScr,Font1,330,628,COLOR_1,-1,(pFirstPhrase+389)->str);
		textprintf_centre_ex(bmpScr,Font1,653,628,COLOR_1,-1,(pFirstPhrase+390)->str);

		if (arHumans[0].bDiplomacy)
		{draw_sprite(bmpScr,bmpStuff[0],100,661);};
		if (arHumans[0].bFog)
		{draw_sprite(bmpScr,bmpStuff[31],297,661);};
		int dx, dy;
		if (GameType>2)
		{
			Humans=0;
			for (int i=1; i<Map.Players; i++)
			{
				if ((arPlayer[i+1].Control==1)&&(arHumanID[i]>=0))
				{Humans++;};
			};
		};

		for (int i=0; i<Map.Players; i++)
		{
			dx=(i%4)*248; dy=((i-i%4)/4)*130;
			
			draw_sprite(bmpScr,bmpGUI_NGPlayer,31+dx,81+dy);
			if (GameType>2)
			{
				draw_sprite(bmpScr,bmpStuff[55],181+(i%4)*248,70+((i-i%4)/4)*130);
				if (arPlayer[i+1].Control>1)
				{draw_sprite(bmpScr,bmpStuff[57],232+(i%4)*248,95+((i-i%4)/4)*130);}
				else
				{draw_sprite(bmpScr,bmpStuff[56],232+(i%4)*248,95+((i-i%4)/4)*130);};
			};

			if ((arPlayer[i+1].Control==1)&&(i>0)&&(arHumanID[i]>=0))
			{
				arHumans[i]=FileBase[arHumanID[i]];
				arPlayer[i+1].ipProfile=arHumans[i].iFileIndex;
				arPlayer[i+1].iColor=i;
			};
				
		
			if (arPlayer[i+1].Control>1)
			{
				buttRuleL[i].Activate(1);
				buttRuleR[i].Activate(1);
			}
			else
			{
				if ((GameType>2)&&(Humans<MaxHumans))
				{
					buttRuleL[i].Activate(1);
					buttRuleR[i].Activate(1);
				}
				else
				{
					buttRuleL[i].Activate(-1);
					buttRuleR[i].Activate(-1);				
				};
			};
			
			if (arPlayer[i+1].iDifficulty>0)
			{buttDiffL[i].Activate(1);}
			else
			{buttDiffL[i].Activate(0);};
			if (arPlayer[i+1].iDifficulty<6)
			{buttDiffR[i].Activate(1);}
			else
			{buttDiffR[i].Activate(0);};
			if ((arPlayer[i+1].ipProfile==0)&&(arPlayer[i+1].Control>1))
			{
				textprintf_centre_ex(bmpScr,Font1,140+dx,150+dy,COLOR_1,-1,(pFirstPhrase+230)->str);
				draw_sprite(bmpScr,bmpSmallAvatar[16],114+dx,86+dy);
				draw_sprite(bmpScr,bmpHerbRandom,54+dx,106+dy);
				draw_sprite(bmpScr,bmpHerbRandom,194+dx,106+dy);
			}
			else
			{
				if ((arPlayer[i+1].Control==1)&&(arHumanID[i]<0))
				{
					textprintf_centre_ex(bmpScr,Font1,140+dx,150+dy,COLOR_1,-1,(pFirstPhrase+212)->str);
					draw_sprite(bmpScr,bmpSmallAvatar[16],114+dx,86+dy);
					draw_sprite(bmpScr,bmpHerbRandom,54+dx,106+dy);
					draw_sprite(bmpScr,bmpHerbRandom,194+dx,106+dy);
				};
				if ((arPlayer[i+1].Control>1)||(arHumanID[i]>=0))
				{
					textprintf_centre_ex(bmpScr,Font1,140+dx,150+dy,COLOR_1,-1,arPlayer[i+1].GetName());
					draw_sprite(bmpScr,arPlayer[i+1].GetSmallAvatar(),114+dx,86+dy);
					arPlayer[i+1].ShowHerb(54+dx,106+dy,bmpScr);
					arPlayer[i+1].ShowHerb(194+dx,106+dy,bmpScr);
				};
			};
			textprintf_centre_ex(bmpScr,Font1,140+dx,180+dy,COLOR_1,-1,arDifficulty[arPlayer[i+1].iDifficulty].sName);
			if (Map.Players==2)
			{buttPlL.Activate(0);}
			else
			{buttPlL.Activate(1);};
			if (Map.Players==16)
			{buttPlR.Activate(0);}
			else
			{buttPlR.Activate(1);};
			if (Size==0)
			{buttMapL.Activate(0);}
			else
			{buttMapL.Activate(1);};
			if (Size==MAX_SHARD_SIZE-1)
			{buttMapR.Activate(0);}
			else
			{buttMapR.Activate(1);};
		};
		for (int i=Map.Players; i<16; i++)
		{
			buttRuleL[i].Activate(-1);
			buttRuleR[i].Activate(-1);
			buttDiffL[i].Activate(-1);
			buttDiffR[i].Activate(-1);
		};
		buttRuleL[0].Activate(0);
		buttRuleR[0].Activate(0);

		bool bCycle=true;
		while (bCycle)
		{
			if (buttMapL.Check())
			{
				if (Size>0)
				{
					Size--;
					if (Map.Players>arShardSize[Size].Players)
					{Map.Players=arShardSize[Size].Players;};
				};
				bCycle=false;
			};

			if (buttMapR.Check())
			{
				if (Size<MAX_SHARD_SIZE-1)
				{Size++;};				
				bCycle=false;
			};

			if (buttPlL.Check())
			{
				if (Map.Players>2)
				{Map.Players--;};				
				bCycle=false;
			};

			if (buttPlR.Check())
			{
				if (Map.Players<16)
				{
					Map.Players++;
					if ((arPlayer[Map.Players].Control>1)&&(arPlayer[Map.Players].ipProfile>0)&&(IsPlayerPicked(arPlayer[Map.Players].ipProfile,Map.Players)))
					{arPlayer[Map.Players].ipProfile=0;};
					if ((arPlayer[Map.Players].Control==1)&&(arHumanID[Map.Players-1]>=0))
					{
						bool bSearch=false;
						for (int j=0; j<Map.Players-1; j++)
						{
							if (arHumanID[Map.Players-1]==arHumanID[j])
							{bSearch=true;};
						};
						if (bSearch)
						{arHumanID[Map.Players-1]=-1;};						
					};

					bCycle=false;
					while (Map.Players>arShardSize[Size].Players)
					{Size++;};
				};
			};

			if (mskMap.Check()>10)
			{WindowMapSettings(&Map,&(arHumans[0].iMapBorders));};

			if (mskFog.Check()>10)
			{
				arHumans[0].bFog=!arHumans[0].bFog;
				if (!arHumans[0].bFog)
				{arHumans[0].bDiplomacy=false;};
				bCycle=false;
			};

			if (mskDiplo.Check()>10)
			{
				arHumans[0].bDiplomacy=!arHumans[0].bDiplomacy;
				if (arHumans[0].bDiplomacy)
				{arHumans[0].bFog=true;};
				bCycle=false;
			};

			if (buttExit.Check())
			{
				bCycle=false;
				bRepeat=false;
				bGenerate=false;
			};
			if (buttAccept.Check())
			{
				bCycle=false;
				bRepeat=false;
			};
			for (int i=0; i<Map.Players; i++)
			{
				if ((i>0)&&(GameType>2))
				{
					if (mskPlayer[i].Check()>10)
					{
						arPlayer[i+1].Control=3-arPlayer[i+1].Control;
					//	if (arPlayer[i+1].Control==1)
					//	{
							arHumanID[i]=-1;							
					//	}
					//	else
					//	{
							arPlayer[i+1].ipProfile=0;
					//	};
						bCycle=false;
					};
				};
				if ((i>0)&&(buttRuleL[i].Check()))
				{
					if (arPlayer[i+1].Control>1)
					{
						do 
						{
							if (arPlayer[i+1].ipProfile>0)
							{arPlayer[i+1].ipProfile--;}
							else
							{arPlayer[i+1].ipProfile=16;};						
						}while(IsPlayerPicked(arPlayer[i+1].ipProfile,Map.Players));
					}
					else
					{
						bool bSearch=true;
						while (bSearch)
						{
							if (arHumanID[i]>=0)
							{arHumanID[i]-=1;}
							else
							{arHumanID[i]=MaxHumans-1;};
							bSearch=false;
							if (arHumanID[i]>-1)
							{
								for (int j=0; j<Map.Players; j++)
								{
									if ((i!=j)&&(arHumanID[i]==arHumanID[j]))
									{bSearch=true;};
								};
							};
						};

					};
					bCycle=false;
				};
				if ((i>0)&&(buttRuleR[i].Check()))
				{
					if (arPlayer[i+1].Control>1)
					{
						do 
						{
							if (arPlayer[i+1].ipProfile<16)
							{arPlayer[i+1].ipProfile++;}
							else
							{arPlayer[i+1].ipProfile=0;};
						}while(IsPlayerPicked(arPlayer[i+1].ipProfile,Map.Players));
					}
					else
					{
						bool bSearch=true;
						while (bSearch)
						{
							if (arHumanID[i]<MaxHumans-1)
							{arHumanID[i]+=1;}
							else
							{arHumanID[i]=-1;};
							bSearch=false;
							if (arHumanID[i]>-1)
							{
								for (int j=0; j<Map.Players; j++)
								{
									if ((i!=j)&&(arHumanID[i]==arHumanID[j]))
									{bSearch=true;};
								};
							};
						};
					};
					bCycle=false;
				};
				if (buttDiffL[i].Check())
				{
					if (arPlayer[i+1].iDifficulty>0)
					{
						arPlayer[i+1].iDifficulty--;
						bCycle=false;
					};
				};
				if (buttDiffR[i].Check())
				{
					if (arPlayer[i+1].iDifficulty<6)
					{
						arPlayer[i+1].iDifficulty++;
						bCycle=false;
					};
				};
			};


			if (Timer)
			{
				Timer=false;				
				objGUI=ShowActiveObjects(objGUI,10,true);
			};
			
			bHint=false;
		};
	};
	
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	destroy_bitmap(bmpScr);

	if (bGenerate)
	{
		for (int i=1; i<Map.Players; i++)
		{
			if ((arPlayer[i+1].Control==1)&&(arHumanID[i]<0))
			{
				arHumans[i].ClearNames();
				arHumans[i].iFileIndex=-1;
				arHumans[i].iDifficulty=arPlayer[i+1].iDifficulty;
				WindowProfile(i,false);
				arPlayer[i+1].ipProfile=arHumans[i].iFileIndex;
			};
		};
		Tutorial.Map=false;		
		CurGame.iMaxPlayers=Map.Players;
		CurGame.iGameType=GameType;	
		CurGame.iSeed=iSeed;
		srand(CurGame.iSeed);
		for (int i=1; i<=CurGame.iMaxPlayers; i++)
		{
			CurGame.arPlayerSet[i]=arPlayer[i].ipProfile;
			if (arPlayer[i].Control>1)
			{
				if (arPlayer[i].ipProfile==0)
				{
					do{
						arPlayer[i].ipProfile=1+Random(16);
					}while (IsPlayerPicked(arPlayer[i].ipProfile,Map.Players));
				};
			};			
		};
		Map.X=arShardSize[Size].x;
		Map.Y=arShardSize[Size].y;
		CurGame.bDiplomacy=arHumans[0].bDiplomacy;
		CurGame.bFog=arHumans[0].bFog;
		arHumans[0].Map=Map;
		arHumans[0].Save();
		GenerateMap(Map, 0);
	};

	return bGenerate;
//return false;

};


void UnloadGame()
{
//	bHourGlass=false;
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(20,20,20,20);
	bDarken=true;
	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		arPlayer[PL].Clear();
	};	
	UnloadMap();
	delete [] pFirstShop;
	WorldRules.iMode=0;
	bDarken=false;
	Timer=false;
//	while(!Timer){};
	set_trans_blender(128,128,128,128);
};

bool CheckCmpLoad()
{
	bool Result=true;
	al_ffblk ProFile;
	char FileName[100];	
	sprintf(FileName,"save\\game%d.cmp",arHumans[0].iFileIndex);
	if (al_findfirst(FileName,&ProFile,FA_ALL)==0)
	{al_findclose(&ProFile);}
	else
	{Result=false;};
	return Result;
};

bool CheckGameLoad()
{
	bool Result=true;
	al_ffblk ProFile;
	char FileName[100];
	sprintf(FileName,"save\\map%d.map",arHumans[0].iFileIndex);
	if (al_findfirst(FileName,&ProFile,FA_ALL)==0)
	{al_findclose(&ProFile);}
	else
	{Result=false;};
	sprintf(FileName,"save\\game%d.sav",arHumans[0].iFileIndex);
	if (al_findfirst(FileName,&ProFile,FA_ALL)==0)
	{al_findclose(&ProFile);}
	else
	{Result=false;};
	return Result;
};

bool CheckHotLoad()
{
	bool Result=true;
	al_ffblk ProFile;
	char FileName[100];
	sprintf(FileName,"save\\map%d.hot",arHumans[0].iFileIndex);
	if (al_findfirst(FileName,&ProFile,FA_ALL)==0)
	{al_findclose(&ProFile);}
	else
	{Result=false;};
	sprintf(FileName,"save\\game%d.hot",arHumans[0].iFileIndex);
	if (al_findfirst(FileName,&ProFile,FA_ALL)==0)
	{al_findclose(&ProFile);}
	else
	{Result=false;};
	return Result;
};

bool CheckLANLoad()
{
	bool Result=true;
	al_ffblk ProFile;
	char FileName[100];
	sprintf(FileName,"save\\map%d.lan",arHumans[0].iFileIndex);
	if (al_findfirst(FileName,&ProFile,FA_ALL)==0)
	{al_findclose(&ProFile);}
	else
	{Result=false;};
	sprintf(FileName,"save\\game%d.lan",arHumans[0].iFileIndex);
	if (al_findfirst(FileName,&ProFile,FA_ALL)==0)
	{al_findclose(&ProFile);}
	else
	{Result=false;};
	return Result;
};

bool CheckNetLoad()
{
	bool Result=true;
	al_ffblk ProFile;
	char FileName[100];
	sprintf(FileName,"save\\map%d.net",arHumans[0].iFileIndex);
	if (al_findfirst(FileName,&ProFile,FA_ALL)==0)
	{al_findclose(&ProFile);}
	else
	{Result=false;};
	sprintf(FileName,"save\\game%d.net",arHumans[0].iFileIndex);
	if (al_findfirst(FileName,&ProFile,FA_ALL)==0)
	{al_findclose(&ProFile);}
	else
	{Result=false;};
	return Result;
};

void ScreenMenu()
{	
	bool CmpFile=CheckCmpLoad(), GameFile=CheckGameLoad(), HotFile=CheckHotLoad(), LANFile=CheckLANLoad(), NetFile=CheckNetLoad();
	BITMAP *bmpScr=create_bitmap(1024,768);	
	BITMAP *bmpMovie=create_bitmap(1024,bmpGUI_Eador->h);
	rectfill(bmpMovie,0,0,1024,bmpGUI_Eador->h,COLOR_1);
	G_Object *objGUI=new G_Object(NULL,0,0,2,bmpScr);
	objGUI=new G_Object(objGUI,0,23,0,bmpMovie);
	UI_Mask arMask[7];
	BITMAP * arMenu[7];
	for (int i=0; i<7; i++)
	{
		arMenu[i]=create_bitmap(bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
		objGUI=arMask[i].Init(372,503+i*28,1,bmpGUI_MenuScrN,arMenu[i],objGUI,3);
	};
	objGUI=new G_Object(objGUI,0,23,1,bmpGUI_Eador);
	G_Object * objEA=objGUI;
	objEA->bTrans=true;

	BITMAP *bmpName=create_bitmap(bmpStuff[36]->w,bmpStuff[36]->h);
	rectfill(bmpName,0,0,bmpStuff[36]->w,bmpStuff[36]->h,COLOR_0);
	BITMAP *bmpCreat=create_bitmap(bmpStuff[38]->w,bmpStuff[38]->h);
	rectfill(bmpCreat,0,0,bmpStuff[38]->w,bmpStuff[38]->h,COLOR_0);
	G_Object *objName, *objNameS, *objCreat, *objCreatS;
	objGUI=new G_Object(objGUI,373,10,4,bmpName);
	objName=objGUI;
	objGUI->bTrans=true;
	objGUI->iOpacity=1;
	objGUI=new G_Object(objGUI,373,10,3,bmpStuff[37]);
	objNameS=objGUI;
	objGUI->bTrans=true;
	objGUI->iOpacity=1;
	if (flgEnglish==1)
	{objGUI=new G_Object(objGUI,421+30,129-35,4,bmpCreat);}
	else
	{objGUI=new G_Object(objGUI,421,129,4,bmpCreat);};	
	objCreat=objGUI;
	objGUI->bTrans=true;
	objGUI->iOpacity=1;
	if (flgEnglish==1)
	{objGUI=new G_Object(objGUI,421+6+30,129+6-35,4,bmpStuff[39]);}
	else
	{objGUI=new G_Object(objGUI,421+6,129+6,4,bmpStuff[39]);}
	objCreatS=objGUI;
	objGUI->bTrans=true;
	objGUI->iOpacity=1;
	UI_Mask mskYuPlay;
#ifdef YUPLAY
	if ((bDemo)&&(iYuPlay>0))
	{
		textprintf_centre_ex(bmpYuPlay[13],Font1,81,14,COLOR_1,-1,(pFirstPhrase+524)->str);
		textprintf_centre_ex(bmpYuPlay[14],Font1,81,14,COLOR_1,-1,(pFirstPhrase+524)->str);
		objGUI=new G_Object(objGUI,832,636,3,bmpYuPlay[13]);
		objGUI=mskYuPlay.Init(832,636,1,bmpYuPlay[13],bmpYuPlay[14],objGUI,4);
	};
#endif
	iGameMode=0;
	ChangeMusic();
	
	int Mode=0, Result, Scroll=0, Blend=0, Namer=0, Creator=0, iAnName=15, iAnCrea=14, Tick=0;
	bool bRepeat=true, bAnName=false, bAnCrea=false;
	while (bRepeat)
	{		
		blit(bmpGUI_MainMenu,bmpScr,0,0,0,0,bmpGUI_MainMenu->w,bmpGUI_MainMenu->h);
		textprintf_centre_ex(bmpScr,Font1,512,724,makecol(120,120,120),-1,(pFirstMessage+113)->msg);
		if (bDemo)
		{textprintf_centre_ex(bmpScr,Font1,512,744,makecol(120,120,120),-1,(pFirstMessage+36)->msg,stVersion);}
		else
		{textprintf_centre_ex(bmpScr,Font1,512,744,makecol(120,120,120),-1,(pFirstMessage+37)->msg,stVersion);};
		switch (Mode)
		{
		case 0:
			for (int i=0; i<7; i++)
			{				
				blit(bmpGUI_MenuScrA,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
				draw_sprite(bmpScr,bmpGUI_MenuScrN,372,503+i*28);
				if (i>0)
				{
					if (((bDemo)||(bBetha))&&(i>1)&&(i<5)&&(i!=4))
					{
						blit(bmpGUI_MenuScrN,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
						textprintf_centre_ex(arMenu[i],Font1,bmpGUI_MenuScrA->w/2,9,makecol(100,100,100),-1,(pFirstPhrase+202+i)->str);
						textprintf_centre_ex(bmpScr,Font1,512,503+i*28+9,makecol(100,100,100),-1,(pFirstPhrase+202+i)->str);
					}
					else
					{
			//			arMask[i].Activate(1);
						textprintf_centre_ex(arMenu[i],Font1,bmpGUI_MenuScrA->w/2,9,COLOR_1,-1,(pFirstPhrase+202+i)->str);
						textprintf_centre_ex(bmpScr,Font1,512,503+i*28+9,COLOR_1,-1,(pFirstPhrase+202+i)->str);
					};
				}
				else
				{
			//		arMask[i].Activate(1);
					int Color=COLOR_1;
					if (bDemo)
					{Color=makecol(100,100,100);};
					if (arHumans[0].iFileIndex==-1)
					{
						textprintf_centre_ex(arMenu[i],Font1,bmpGUI_MenuScrA->w/2,9,Color,-1,(pFirstPhrase+212+i)->str);
						textprintf_centre_ex(bmpScr,Font1,512,503+i*28+9,Color,-1,(pFirstPhrase+212+i)->str);
					}
					else
					{
						textprintf_centre_ex(arMenu[i],Font1,bmpGUI_MenuScrA->w/2,9,Color,-1,arHumans[0].sName);
						textprintf_centre_ex(bmpScr,Font1,512,503+i*28+9,Color,-1,arHumans[0].sName);
					};
				};
			};
			break;
		case 1:
			for (int i=0; i<4; i++)
			{
		//		arMask[i].Activate(1);
				if ((i==1)&&(!CmpFile))
				{
					blit(bmpGUI_MenuScrN,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
					draw_sprite(bmpScr,bmpGUI_MenuScrN,372,503+i*28);
					textprintf_centre_ex(arMenu[i],Font1,bmpGUI_MenuScrA->w/2,9,makecol(100,100,100),-1,(pFirstPhrase+209+i)->str);
					textprintf_centre_ex(bmpScr,Font1,512,503+i*28+9,makecol(100,100,100),-1,(pFirstPhrase+209+i)->str);
				}
				else
				{
					blit(bmpGUI_MenuScrA,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
					draw_sprite(bmpScr,bmpGUI_MenuScrN,372,503+i*28);
					if (i<2)
					{
						textprintf_centre_ex(arMenu[i],Font1,bmpGUI_MenuScrA->w/2,9,COLOR_1,-1,(pFirstPhrase+209+i)->str);
						textprintf_centre_ex(bmpScr,Font1,512,503+i*28+9,COLOR_1,-1,(pFirstPhrase+209+i)->str);
					};
				};
				textprintf_centre_ex(arMenu[2],Font1,bmpGUI_MenuScrA->w/2,9,COLOR_1,-1,(pFirstPhrase+482)->str);
				textprintf_centre_ex(bmpScr,Font1,512,503+2*28+9,COLOR_1,-1,(pFirstPhrase+482)->str);
				textprintf_centre_ex(arMenu[3],Font1,bmpGUI_MenuScrA->w/2,9,COLOR_1,-1,(pFirstPhrase+211)->str);
				textprintf_centre_ex(bmpScr,Font1,512,503+3*28+9,COLOR_1,-1,(pFirstPhrase+211)->str);
			};
			break;
		case 2:
			for (int i=0; i<4; i++)
			{
		//		arMask[i].Activate(1);
				if ((i==1)&&(!GameFile))
				{
					blit(bmpGUI_MenuScrN,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
					draw_sprite(bmpScr,bmpGUI_MenuScrN,372,503+i*28);			
					textprintf_centre_ex(arMenu[i],Font1,bmpGUI_MenuScrA->w/2,9,makecol(100,100,100),-1,(pFirstPhrase+209+i)->str);
					textprintf_centre_ex(bmpScr,Font1,512,503+i*28+9,makecol(100,100,100),-1,(pFirstPhrase+209+i)->str);				
				}
				else
				{
					blit(bmpGUI_MenuScrA,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
					draw_sprite(bmpScr,bmpGUI_MenuScrN,372,503+i*28);
					if (i<2)
					{
						textprintf_centre_ex(arMenu[i],Font1,bmpGUI_MenuScrA->w/2,9,COLOR_1,-1,(pFirstPhrase+209+i)->str);
						textprintf_centre_ex(bmpScr,Font1,512,503+i*28+9,COLOR_1,-1,(pFirstPhrase+209+i)->str);
					};
				};
			};
			textprintf_centre_ex(arMenu[2],Font1,bmpGUI_MenuScrA->w/2,9,COLOR_1,-1,(pFirstPhrase+299)->str);
			textprintf_centre_ex(bmpScr,Font1,512,503+2*28+9,COLOR_1,-1,(pFirstPhrase+299)->str);
			textprintf_centre_ex(arMenu[3],Font1,bmpGUI_MenuScrA->w/2,9,COLOR_1,-1,(pFirstPhrase+211)->str);
			textprintf_centre_ex(bmpScr,Font1,512,503+3*28+9,COLOR_1,-1,(pFirstPhrase+211)->str);

		/*	for (int i=4; i<7; i++)
			{
				arMask[i].Activate(0);
			};*/
			break;
		case 3:
			for (int i=0; i<4; i++)
			{
				blit(bmpGUI_MenuScrA,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
				draw_sprite(bmpScr,bmpGUI_MenuScrN,372,503+i*28);
				if (i<3)
				{
					textprintf_centre_ex(arMenu[i],Font1,bmpGUI_MenuScrA->w/2,9,COLOR_1,-1,(pFirstPhrase+445+i)->str);
					textprintf_centre_ex(bmpScr,Font1,512,503+i*28+9,COLOR_1,-1,(pFirstPhrase+445+i)->str);
				};
			};
			textprintf_centre_ex(arMenu[3],Font1,bmpGUI_MenuScrA->w/2,9,COLOR_1,-1,(pFirstPhrase+211)->str);
			textprintf_centre_ex(bmpScr,Font1,512,503+3*28+9,COLOR_1,-1,(pFirstPhrase+211)->str);
			break;
		case 4:
			for (int i=0; i<3; i++)
			{		
				if ((i==1)&&(!HotFile))
				{
					blit(bmpGUI_MenuScrN,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
					draw_sprite(bmpScr,bmpGUI_MenuScrN,372,503+i*28);
					textprintf_centre_ex(arMenu[i],Font1,bmpGUI_MenuScrA->w/2,9,makecol(100,100,100),-1,(pFirstPhrase+209+i)->str);
					textprintf_centre_ex(bmpScr,Font1,512,503+i*28+9,makecol(100,100,100),-1,(pFirstPhrase+209+i)->str);
				}
				else
				{
					blit(bmpGUI_MenuScrA,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
					draw_sprite(bmpScr,bmpGUI_MenuScrN,372,503+i*28);
					textprintf_centre_ex(arMenu[i],Font1,bmpGUI_MenuScrA->w/2,9,COLOR_1,-1,(pFirstPhrase+209+i)->str);
					textprintf_centre_ex(bmpScr,Font1,512,503+i*28+9,COLOR_1,-1,(pFirstPhrase+209+i)->str);
				};
			};
			break;

		};
		bool bCycle=true;
		int Check;
		while (bCycle)		
		{			
			switch (Mode)
			{
			//Основное меню
			case 0:
						
				Check=arMask[1].Check();
				if (Check==30)
				{
					Mode=1;
					bCycle=false;
				};
				Check=arMask[4].Check();
				if (Check==30)
				{
					WindowSettings(0);
				};
			//	if (!bDemo)
				{
				Check=arMask[0].Check();
				if ((bDemo)&&(Check>1))
				{
					MakeStringHint(512);
					bHint=true;
				}
				else
				{
				if (Check==30)
				{
				//	OpenWeb();
					ProfileOperator(0);
					CmpFile=CheckCmpLoad();
					GameFile=CheckGameLoad();
					HotFile=CheckHotLoad();
					LANFile=CheckLANLoad();
					NetFile=CheckNetLoad();
//					WindowProfile(&arHumans[0]);
					bCycle=false;
				};
				};
				if (!bBetha)
				{
				Check=arMask[2].Check();
				if ((bDemo)&&(Check>1))
				{
					MakeStringHint(512);
					bHint=true;
				}
				else
				{
				if (Check==30)
				{
					Mode=2;
					bCycle=false;
				};
				};

				Check=arMask[3].Check();
				if ((bDemo)&&(Check>1))
				{
					MakeStringHint(512);
					bHint=true;
				}
				else
				{
				if (Check==30)
				{
					Mode=3;
					bCycle=false;
				};
				};
				};
				};
				Check=arMask[5].Check();
				if (Check==30)
				{
					srand(iSeed);
					ScreenCredits();
					/*Namer=1;
					objNameS->x=objName->x;
					objNameS->y=objName->y;
					objName->bTrans=true;*/
				};
				Check=arMask[6].Check();
				if (Check==30)
				{
					Result=0;
					bCycle=false;
					bRepeat=0;
				};
				break;
			//Кампания
			case 1:
				Check=arMask[0].Check();
				if (Check==30)
				{
					if ((!CmpFile)||((pFirstDialog+1193)->Show(1,1,0,0,NULL,NULL)==1))
					{
						bCycle=false;
						srand(iSeed);
						NewCampaign();					
						while (Astral()<0)
						{			
							if (bShowWallpaper){ShowWallpaper();}
							bExitGame=false;
							char sFile1[150], sFile2[150];
							sprintf(sFile1,"save\\game%d.bka",arHumans[0].iFileIndex);
							sprintf(sFile2,"save\\game%d.cmp",arHumans[0].iFileIndex);
							FileCopy(sFile1,sFile2);
							LoadGame(CurGame.iGameType);
							int Drain=GetEnergyIncome()/20;
							if ((Drain==0)&&(GetEnergyIncome()>0))
							{Drain=1;};
							Campaign.arCampPlayer[0].iEnergyDrain+=Drain;
							bHourGlass=false;
						};
						Mode=0;
						CmpFile=CheckCmpLoad();
					};
				};
				if (CmpFile)
				{
					Check=arMask[1].Check();
					if (Check==30)
					{						
						if (LoadGame(2))
						{
							/*for (int i=1; i<=MapSize; i++)
							{
								for (int j=0; j<30; j++)
								{
									if ((pFirstProvince+i)->arSite[j][2]>15)
									{(pFirstProvince+i)->arSite[j][2]=0;};
								};
							};
							SaveGame(0);*/
						//	CurGame.iCurrentTurn=-1;
							bHourGlass=false;						
							while (Astral()<0)
							{			
								if (bShowWallpaper){ShowWallpaper();}
								bExitGame=false;
								char sFile1[150], sFile2[150];
								sprintf(sFile1,"save\\game%d.bka",arHumans[0].iFileIndex);
								sprintf(sFile2,"save\\game%d.cmp",arHumans[0].iFileIndex);
								FileCopy(sFile1,sFile2);
								LoadGame(CurGame.iGameType);
								int Drain=GetEnergyIncome()/20;
								if ((Drain==0)&&(GetEnergyIncome()>0))
								{Drain=1;};
								Campaign.arCampPlayer[0].iEnergyDrain+=Drain;
								bHourGlass=false;
							};						
							Mode=0;
							bCycle=false;
							CmpFile=CheckCmpLoad();
						};
					};
				};
				Check=arMask[2].Check();
				if (Check==30)
				{
					/*LoadGame(2);
					iGameMode=1;
					Campaign.arCampPlayer[0].iKarma=80;
					Campaign.arCampPlayer[0].iEnergy=100;
					SaveGame(0);*/
					WindowCampHighScore();
				};

				Check=arMask[3].Check();
				if (Check==30)
				{
					Mode=0;
					bCycle=false;
				};
				break;
			//Одиночная игра
			case 2:
				Check=arMask[0].Check();
				if (Check==30)
				{
					if ((!GameFile)||((pFirstDialog+1193)->Show(1,1,0,0,NULL,NULL)==1))
					{
						bCycle=false;
						if (NewGame(iSeed,1))
						{
							RunGame(true);
							iGameMode=0;
							ChangeMusic();
							Mode=0;
							GameFile=CheckGameLoad();
						};
					};
				};
				if (GameFile)
				{
					Check=arMask[1].Check();
					if (Check==30)
					{						
						LoadGame(1);
						RunGame(false);
						iGameMode=0;
						ChangeMusic();
						Mode=0;
						bCycle=false;
						GameFile=CheckGameLoad();
					};
				};
				Check=arMask[2].Check();
				if (Check==30)
				{WindowHighScore();};
				Check=arMask[3].Check();
				if (Check==30)
				{
					Mode=0;
					bCycle=false;
				};
				break;
			//Выбор сетевой игры
			case 3:
				Check=arMask[0].Check();
				if (Check==30)
				{	
					Mode=4;
					bCycle=false;					
				};
				Check=arMask[1].Check();
				if (Check==30)
				{	
					(pFirstDialog+1242)->Show(1,1,0,0,NULL,NULL);
				};
				Check=arMask[2].Check();
				if (Check==30)
				{	
					(pFirstDialog+1242)->Show(1,1,0,0,NULL,NULL);
				};
				Check=arMask[3].Check();
				if (Check==30)
				{
					Mode=0;
					bCycle=false;
				};
				break;
			//HotSeat
			case 4:
				Check=arMask[0].Check();
				if (Check==30)
				{
					if ((!HotFile)||((pFirstDialog+1193)->Show(1,1,0,0,NULL,NULL)==1))
					{
						bCycle=false;
						if (NewGame(iSeed,3))
						{
							RunGame(true);
							iGameMode=0;
							ChangeMusic();
							Mode=0;
							HotFile=CheckHotLoad();
						};
					};
				};
				if (HotFile)
				{
					Check=arMask[1].Check();
					if (Check==30)
					{						
						LoadGame(3);						
						RunGame(false);
						iGameMode=0;
						ChangeMusic();
						Mode=0;
						bCycle=false;
						HotFile=CheckHotLoad();
					};
				};
				Check=arMask[2].Check();
				if (Check==30)
				{
					Mode=3;
					bCycle=false;
				};
				break;
			};
#ifdef YUPLAY
			if ((bDemo)&&(iYuPlay>0))
			{
				if (mskYuPlay.Check()==30)
				{
					if (InstallYuPlay())
					{
						YuPlayBuyGame(true);
						if (!bDemo)
						{
							Result=0;
							bCycle=false;
							bRepeat=0;
						};
					};
				};
			};
#endif
			iSeed++;
			if (iSeed>10000)
			{
				iSeed=1;
			};
			if (Timer)
			{
				Timer=false;
				Tick++;
				if (Tick==900)
				{Tick=0;};
				
				if ((Blend>100)&&(Namer==0))
				{Namer=1;};

				if (Namer>0)
				{
					if (Namer<50)
					{
						set_trans_blender(Namer*5,Namer*5,Namer*5,0);	
						draw_lit_sprite(bmpName,bmpStuff[36],0,0,250);
						objName->iOpacity=Namer*5;
						objNameS->iOpacity=Namer*3/2;						
						objNameS->x=objName->x+Namer/7;
						objNameS->y=objName->y+Namer/7;						
						Namer+=4;
					}
					else
					{
						if (Namer<100)
						{
							set_trans_blender(250,250,250,0);	
							draw_lit_sprite(bmpName,bmpStuff[36],0,0,250-(Namer-50)*5);
							objName->bTrans=false;
							objNameS->iOpacity=Namer*3/2;
							Namer++;							
						};
					};
					if (Namer==100)
					{
						Namer=101;
						Creator=1;
						draw_sprite(bmpName,bmpStuff[36],0,0);
						draw_sprite(bmpCreat,bmpStuff[38],0,0);
						bAnName=true;
					};
				};
				if ((Creator>0)&&(Creator<50))
				{
						objCreat->iOpacity=Creator*5;
						objCreatS->iOpacity=Creator*3;
					//	objCreatS->x=objCreat->x+Creator/7;
					//	objCreatS->y=objCreat->y+Creator/7;						
						Creator++;
				};
				if (Creator==50)
				{
					Creator=51;
					objCreat->bTrans=false;
				};
				if ((bAnName)&&(flgEnglish!=1))
				{
					if (Tick%2==0)
					{
						if (iAnName>=0)
						{
							draw_sprite(bmpName,bmpAnName[iAnName],0,0);
							iAnName--;
						}
						else
						{
							draw_sprite(bmpName,bmpStuff[36],0,0);
							iAnName=15;
							bAnName=false;
							bAnCrea=true;
							iAnCrea=14;
						};
					};
				}
				else
				{
					if ((Namer==101)&&(Random(400)==0))
					{bAnName=true;};
				};
				if (bAnCrea)
				{
					if (Tick%2==0)
					{
						if (iAnCrea>=0)
						{
							draw_sprite(bmpCreat,bmpAnCrea[iAnCrea],0,0);
							iAnCrea--;
						}
						else
						{
							draw_sprite(bmpCreat,bmpStuff[38],0,0);							
							bAnCrea=false;							
						};
					};
				}
				if (mouse_x<200)
				{
					if (Scroll>0)
					{Scroll--;};
				}
				else
				{
					if (mouse_x>824)
					{
						if (Scroll<bmpGUI_Eador->w-1024)
						{Scroll++;};
					}
					else
					{
						if (Scroll>(bmpGUI_Eador->w-1024)/2)
						{Scroll--;};
						if (Scroll<(bmpGUI_Eador->w-1024)/2)
						{Scroll++;};
					};
				};
				if (Blend<255)
				{
					Blend+=2;
					set_trans_blender(Blend,Blend,Blend,Blend);
				}
				else
				{
					objEA->bTrans=false;					
				};
				objEA->x=-Scroll;
				objGUI=ShowActiveObjects(objGUI,10,true);
				bHint=false;
			}
	//		else
	//		{Sleeper(25);};
		};
	};
	for (int i=0; i<7; i++)
	{		
		destroy_bitmap(arMenu[i]);
	};
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	destroy_bitmap(bmpScr);
	set_trans_blender(128,128,128,128);	
};