#include "Windower.h"

BITMAP *bmpRed, *bmpBlack;

GUI_ProvBar::GUI_ProvBar()
{
	bCheckable=false;
	Pic=create_bitmap(bmpStuff[68]->w,bmpStuff[68]->h);	
};

GUI_ProvBar::~GUI_ProvBar()
{
	destroy_bitmap(Pic);
};

G_Object * GUI_ProvBar::Init(int st_y, int st_Layer, dcProvince *Prov, bool Buttons, G_Object * st_Obj)
{
	x=0;
	Mode=0;
	Shift=0;
	y=st_y;
	Layer=st_Layer;
	PR=Prov;
	Obj=st_Obj;
	bCheckable=Buttons;
	if (Buttons)
	{
		mskTerra.Init(x+24,y+32,211,24,1);
		mskTerra.iHint=527;
		mskGold.Init(x+250,y+6,88,23,1);
		mskGold.iHint=405;
		mskGem.Init(x+250,y+33,88,23,1);
		mskGem.iHint=406;
		mskLevel.Init(x+457,y+4,148,25,1);
		mskLevel.iHint=403;
		mskMood.Init(x+457,y+32,148,25,1);
		mskMood.iHint=401;
		mskHome.Init(x+610,y+3,170,55,1);
		mskHome.iHint=528;
		Obj=mskRes.Init(x+342-6,y-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,Layer+1);
		Obj=mskRace.Init(x+401-6,y-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,Layer+1);
		Obj=mskGuard.Init(x+786-6,y-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,Layer+1);
		Obj=mskFort.Init(x+846-6,y-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,Layer+1);
		Obj=mskSite.Init(x+906-6,y-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,Layer+1);
		Obj=buttLeft.Init(x+240,y+17,1,Layer+1,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],Obj);		
		Obj=buttRight.Init(x+942,y+17,1,Layer+1,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],Obj);
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			Obj=mskBuild[i].Init(x+612-6+i*57,y-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,Layer+1);
		};
		for (int i=0; i<MAX_SITE; i++)
		{
			Obj=mskS[i].Init(x+260+i*57-6,y-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,Layer+1);
		};
		Obj=buttSelect.Init(x+964,y+3,1,Layer+1,bmpButton[322],bmpButton[323],bmpButton[324],bmpButton[324],Obj);
		buttSelect.Activate(1);
		buttSelect.iHint=550;
	};
	Obj=new G_Object(Obj,x,y,Layer,Pic);
	return Obj;
};

void GUI_ProvBar::Show()
{
	char Txt[200];
	//Обычная инфа
	if (Mode==0)
	{
	if (bCheckable)
	{
		buttLeft.Activate(-1);
		buttRight.Activate(-1);
	};
	blit(bmpStuff[68],Pic,0,0,0,0,bmpStuff[68]->w,bmpStuff[68]->h);
	textprintf_centre_ex(Pic,Font1,129,12,COLOR_1,-1,PR->sName);
	sprintf(Txt,"%s %d~",(pFirstTerrain+PR->ipTerrain)->sName,PR->iExplore);
	if (PR->iExplore>=ProvLevel[PR->GetLevel()].iExplored)
	{textprintf_centre_ex(Pic,Font1,129,41,COLOR_1,-1,Txt);}
	else
	{textprintf_centre_ex(Pic,Font1,129,41,makecol(220,20,20),-1,Txt);}	
	draw_sprite(Pic,(pFirstRace+PR->ipRace)->Pic,401,5);			
	if (PR->GetResource(1)>0)
	{
		draw_sprite(Pic,(pFirstResource+PR->GetResource(1))->BigPic,342,5);
	}
	else
	{
		if (PR->GetResource(0)>0)
		{
			draw_sprite(Pic,(pFirstResource+PR->GetResource(0))->BigPic,342,5);
			draw_trans_sprite(Pic,bmpRed,342,5);
		}
		else
		{
			if ((PR->GetResource(-1)>0)&&(arPlayer[PR->ipPlayer].CompareBldParam1(26,PR->ipTerrain)))
			{				
				draw_sprite(Pic,(pFirstResource+PR->GetResource(-1))->BigPic,342,5);
				draw_trans_sprite(Pic,bmpBlack,342,5);
				draw_trans_sprite(Pic,bmpStuff[40],342,5);
				draw_trans_sprite(Pic,bmpStuff[41],342,5);				
			}
			else
			{
				draw_trans_sprite(Pic,bmpStuff[13],342,5);
			};
		};
	};	
	int Count=0, Bld;
	if (arPlayer[PR->ipPlayer].ipProvince==PR->iNumber)
	{
		draw_sprite(Pic,bmpStuff[69],610,3);
		draw_sprite(Pic,(pFirstInnerBld+149)->Pic,846,5);
		ShowIconPercent(Pic,845,5,100*PR->iFort/arPlayer[PR->ipPlayer].iWall);
	}
	else
	{
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			if (PR->arBuilding[i]>0)
			{
				draw_sprite(Pic,(pFirstOuterBld+PR->arBuilding[i])->Pic,612+Count*57,5);
				Count++;				
				if ((pFirstOuterBld+PR->arBuilding[i])->iWall>0)
				{
					draw_sprite(Pic,(pFirstOuterBld+PR->arBuilding[i])->Pic,846,5);
					ShowIconPercent(Pic,845,5,100*PR->iFort/(pFirstOuterBld+PR->arBuilding[i])->iWall);					
				};
			//	mskBuild[i].Replace(x+612-6+Count*57,y-1);
			};
		};
	};
	if (PR->iDefender>0)
	{
		draw_sprite(Pic,(pFirstDefender+PR->iDefender)->Pic,786,5);
		int MaxLife=0, CurLife=0;
		for (int i=0; i<MAX_SQUAD; i++)
		{
			if (PR->pGuard->pSquad[i])
			{
				MaxLife+=PR->pGuard->pSquad[i]->GetLife();
				CurLife+=PR->pGuard->pSquad[i]->iLife;
			};
		};
		ShowIconPercent(Pic,786,5,100*CurLife/MaxLife);
	};

	Count=0; Bld=0;
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if ((PR->arSite[i][0]>0)&&(PR->arSite[i][1]<=PR->iExplore))
		{							
			Bld=PR->arSite[i][0];
		//	FileLog(PR->sName);
		//	FileLog((pFirstSite+Bld)->sName);
			Count++;
		};
	};
	if (Bld>0)
	{
		draw_sprite(Pic,(pFirstSite+Bld)->Pic,906,5);
		ShowIconNumber(Pic,906,5,Count);
	};

	if (PR->GetGoldIncome()>0)
	{
		textprintf_centre_ex(Pic,Font1,298,12,COLOR_2,-1,"+%d",PR->GetGoldIncome());
	}
	else
	{
		textprintf_centre_ex(Pic,Font1,298,12,COLOR_2,-1,"%d",PR->GetGoldIncome());
	};
	if (PR->GetGemIncome()>0)
	{
		textprintf_centre_ex(Pic,Font1,298,39,COLOR_2,-1,"+%d",PR->GetGemIncome());
	}
	else
	{
		textprintf_centre_ex(Pic,Font1,298,39,COLOR_2,-1,"%d",PR->GetGemIncome());
	};
			
			
	if (PR->GetLevel()<MAX_PROVINCE_LEVEL)
	{
		if (PR->iPopulation<=0)
		{
			for (int i=0; i<(10-(-PR->iPopulation)%10)*14; i++)
			{
				rectfill(Pic,459+i,5,459+i,27,makecol(5+i/5,80+i,10+i/10));
			};
		}
		else
		{
			for (int i=0; i<145*(PR->iPopulation-ProvLevel[PR->GetLevel()-1].iPopulation)/(ProvLevel[PR->GetLevel()].iPopulation-ProvLevel[PR->GetLevel()-1].iPopulation); i++)
			{
				rectfill(Pic,459+i,5,459+i,27,makecol(5+i/5,80+i,10+i/10));
			};
		};
	};
	textprintf_centre_ex(Pic,Font1,533,13,COLOR_1,-1,ProvLevel[PR->GetLevel()].sName);
	textprintf_centre_ex(Pic,Font1,532,12,COLOR_2,-1,ProvLevel[PR->GetLevel()].sName);

	for (int i=0; i<145*PR->arUnrest[PR->ipPlayer]/MAX_UNREST; i++)
	{
		rectfill(Pic,459+i,33,459+i,54,makecol(110+i,20+i/7,20+i/7));
	};
	int Mood=PR->GetMood();
	if (Mood<-5)
	{Mood=-5;};
	if (Mood>3)
	{Mood=3;};
	if (PR->iPopulation>0)
	{
		textprintf_centre_ex(Pic,Font1,533,40,COLOR_1,-1,(pFirstPhrase+Mood+80)->str);
		if (Mood>=0)
		{textprintf_centre_ex(Pic,Font1,532,39,makecol(200-Mood*20,200+Mood*15,20+Mood*40),-1,(pFirstPhrase+Mood+80)->str);}
		else
		{textprintf_centre_ex(Pic,Font1,532,39,makecol(200-Mood*10,200+Mood*40,20+Mood*4),-1,(pFirstPhrase+Mood+80)->str);};
	}
	else
	{
		textprintf_centre_ex(Pic,Font1,533,40,COLOR_1,-1,(pFirstPhrase+231)->str);
		textprintf_centre_ex(Pic,Font1,532,39,COLOR_2,-1,(pFirstPhrase+231)->str);
	};
	};


	//Список сайтов
	if (Mode==1)
	{
	blit(bmpStuff[71],Pic,0,0,0,0,bmpStuff[68]->w,bmpStuff[68]->h);
	textprintf_centre_ex(Pic,Font1,129,12,COLOR_1,-1,PR->sName);
	sprintf(Txt,"%s %d~",(pFirstTerrain+PR->ipTerrain)->sName,PR->iExplore);
	if (PR->iExplore>=ProvLevel[PR->GetLevel()].iExplored)
	{textprintf_centre_ex(Pic,Font1,129,41,COLOR_1,-1,Txt);}
	else
	{textprintf_centre_ex(Pic,Font1,129,41,makecol(220,20,20),-1,Txt);}
	SiteLimit=0; Sites=0;
	for (int i=0; i<MAX_PROVINCE_SITE; i++)
	{
		if ((PR->arSite[i][0]>0)&&(PR->arSite[i][1]<=PR->iExplore))
		{
			if ((SiteLimit<MAX_SITE)&&(Sites>=Shift*MAX_SITE))
			{
				arSite[SiteLimit]=i;
				draw_sprite(Pic,(pFirstSite+PR->arSite[i][0])->Pic,260+SiteLimit*57,5);
				SiteLimit++;
			};			
			Sites++;
		};
	};
	if (bCheckable)
	{
		if (Sites>MAX_SITE)
		{
			if (Shift>0)
			{buttLeft.Activate(1);}
			else
			{buttLeft.Activate(0);};
			if (Sites>((Shift+1)*MAX_SITE))
			{buttRight.Activate(1);}
			else
			{buttRight.Activate(0);};	
		}
		else
		{
			buttLeft.Activate(-1);
			buttRight.Activate(-1);
		};
	};
	};
	
	
};

void GUI_ProvBar::Activate(dcProvince *Prov)
{
	PR=Prov;
	Obj->iLayer=Layer;
	Show();
	if (bCheckable)
	{buttSelect.Activate(1);};
	Shift=0;
};

void GUI_ProvBar::Hide()
{	
	Obj->iLayer=-1;
	if (bCheckable)
	{
		buttSelect.Activate(-1);
		buttLeft.Activate(-1);
		buttRight.Activate(-1);
		mskRes.G_Obj->iLayer=-1;
		mskRace.G_Obj->iLayer=-1;
		mskGuard.G_Obj->iLayer=-1;
		mskFort.G_Obj->iLayer=-1;
		mskSite.G_Obj->iLayer=-1;
		buttLeft.G_Obj->iLayer=-1;
		buttRight.G_Obj->iLayer=-1;
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			mskBuild[i].G_Obj->iLayer=-1;
		};
		for (int i=0; i<MAX_SITE; i++)
		{
			mskS[i].G_Obj->iLayer=-1;
		};
	};
};

int GUI_ProvBar::Check(G_Object * objMain, int MaxLayer)
{
	if ((Layer<0)||(!bCheckable))
	{return 0;};

	char Txt[200];

	int Check=mskTerra.Check();
	if ((Check==30)||(Check==40))
	{
		objMain=ShowActiveObjects(objMain,MaxLayer,false);
		WindowInfo((pFirstPhrase+400)->str,(pFirstMessage+30)->msg,bmpStuff[8]);
	};

	if (Mode==0)
	{
	Check=mskMood.Check();
	if ((Check==30)||(Check==40))
	{
		objMain=ShowActiveObjects(objMain,MaxLayer,false);
		WindowInfo((pFirstPhrase+401)->str,(pFirstMessage+31)->msg,bmpStuff[51]);
	};

	Check=mskMood.Check();
				
	Check=mskLevel.Check();
	if ((Check==30)||(Check==40))
	{
		objMain=ShowActiveObjects(objMain,MaxLayer,false);
		WindowInfo((pFirstPhrase+403)->str,(pFirstMessage+32)->msg,bmpStuff[50]);
	};
	mskGold.Check();
	mskGem.Check();

	Check=mskRace.Check();
	if (Check==2)
	{
		MakeStringHint((pFirstRace+PR->ipRace)->sName);
		bHint=true;
	};
	if ((Check==30)||(Check==40))
	{
		objMain=ShowActiveObjects(objMain,MaxLayer,false);
		WindowRaceInfo(PR->ipRace);		
	};

	Check=mskRes.Check();
	if (Check==2)
	{
		if (PR->GetResource(1)>0)
		{
			MakeStringHint((pFirstResource+PR->GetResource(1))->sName);
		}
		else
		{
			if (PR->GetResource(0)>0)
			{
				sprintf(Txt,"%s (%s)",(pFirstResource+PR->GetResource(0))->sName,(pFirstPhrase+530)->str);
				MakeStringHint(Txt);
			}
			else
			{
				if ((PR->GetResource(-1)>0)&&(arPlayer[PR->ipPlayer].CompareBldParam1(26,PR->ipTerrain)))
				{				
					sprintf(Txt,"%s (%s)",(pFirstResource+PR->GetResource(-1))->sName,(pFirstPhrase+529)->str);
					MakeStringHint(Txt);
					}
				else
				{
					MakeStringHint(26);
				};
			};
		};
		bHint=true;
	};
	if ((Check==30)||(Check==40))
	{
		if ((PR->GetResource(0)==0)&&((PR->GetResource(-1)==0)||(!arPlayer[PR->ipPlayer].CompareBldParam1(26,PR->ipTerrain))))
		{
			objMain=ShowActiveObjects(objMain,MaxLayer,false);
			WindowInfo((pFirstPhrase+26)->str,(pFirstMessage+34)->msg,bmpStuff[13]);
		}
		else
		{
			objMain=ShowActiveObjects(objMain,MaxLayer,false);
			WindowResourceInfo(PR->GetResource(-1));
		};
	};

	if (arPlayer[PR->ipPlayer].ipProvince==PR->iNumber)
	{
		mskHome.Check();
	}
	else
	{
		for (int i=0; i<MAX_PROVINCE_BLD; i++)
		{
			Check=mskBuild[i].Check();
			int Bld=0, Count=-1;
			for (int j=0; j<MAX_PROVINCE_BLD; j++)
			{
				if (PR->arBuilding[j]>0)
				{Count++;}
				if (i==Count)
				{
					Bld=PR->arBuilding[j];
					break;
				}
			}

			if (Check==2)
			{
				if (Bld>0)
				{MakeStringHint((pFirstOuterBld+Bld)->sName);}
				else
				{MakeStringHint(531);};
				bHint=true;
			};
			if (((Check==30)||(Check==40))&&(Bld>0))
			{
				objMain=ShowActiveObjects(objMain,MaxLayer,false);
				WindowOuterBldInfo(Bld);
			};
		};
	};

	Check=mskGuard.Check();

	if (Check==2)
	{
		if (PR->iDefender>0)
		{MakeStringHint((pFirstDefender+PR->iDefender)->sName);}
		else
		{MakeStringHint(532);};
		bHint=true;
	};

	if (((Check==30)||(Check==40))&&(PR->iDefender>0))
	{
		if ((pFirstDefender+PR->iDefender)->iGoldPayment<0)
		{WindowDefenderInfo(PR->iDefender, PR->pGuard->GetDefenderGoldPayment(), PR->pGuard->GetDefenderGemPayment());}
		else
		{WindowDefenderInfo(PR->iDefender, 0, 0);}
		
	};

	if (mskSite.Check()>1)
	{
		MakeStringHint(533);
		bHint=true;
	};

	if (mskFort.Check()>1)
	{
		bool bFort=false;
		if (arPlayer[PR->ipPlayer].ipProvince==PR->iNumber)
		{bFort=true;}
		else
		{
			for (int i=0; i<MAX_PROVINCE_BLD; i++)
			{
				if ((PR->arBuilding[i]>0)&&((pFirstOuterBld+PR->arBuilding[i])->iWall>0))
				{bFort=true;}				
			}
		}
		if (bFort)
		{MakeStringHint(117);}
		else
		{MakeStringHint(38);}
		bHint=true;
	};
	};
	if (Mode==1)
	{
		for (int i=0; i<SiteLimit; i++)
		{
			Check=mskS[i].Check();
			if (Check>1)
			{
				MakeSiteHint(PR,arSite[i]);
				bHint=true;
			};
		};
		if (Sites>MAX_SITE)
		{
			if (buttLeft.Check())
			{
				Shift--;
				Show();
			};
			if (buttRight.Check())
			{
				Shift++;
				Show();
			};			
		};
	};
	int Result=0;
	if (buttSelect.Check())
	{Result=1;};
	return Result;
}

int ScreenProvList(int Player)
{
	int Result=-1;
	const int MAX_PROV=10;
	const int MAX_MODE=10;
	const int MAX_MAP=400;
	iAntisleep=iAntisleepDelay;
	BITMAP *bmpStat=create_bitmap(1024,768);	
	draw_sprite(bmpStat,bmpStuff[70],0,0);
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpStat);

	draw_sprite(bmpStat,bmpStuff[73],11,715);
	draw_sprite(bmpStat,bmpStuff[73],568,715);
	draw_sprite(bmpStat,bmpStuff[73],693,715);
	draw_sprite(bmpStat,bmpStuff[73],818,715);
	draw_sprite(bmpStat,bmpStuff[73],943,715);
	
	draw_sprite(bmpStat,bmpStuff[74],959,715);
	UI_Button1 buttExit;
	objGUI=buttExit.Init(959,718,1,2,bmpButton[256],bmpButton[257],bmpButton[258],bmpButton[258],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;
	UI_Button1 buttMode[MAX_MODE];
	for (int i=0; i<MAX_MODE; i++)
	{
		draw_sprite(bmpStat,bmpStuff[74],27+i*54,715);
		objGUI=buttMode[i].Init(29+i*54,718,1,2,bmpButton[345+i*3],bmpButton[346+i*3],bmpButton[347+i*3],bmpButton[347+i*3],objGUI);
		buttMode[i].ipHotkey=KEY_1+i;
		buttMode[i].iHint=534+i;
	};
	buttMode[0].Activate(0);
	buttMode[9].ipHotkey=KEY_0;

	draw_sprite(bmpStat,bmpStuff[74],584,715);
	UI_Button1 buttSort1;
	objGUI=buttSort1.Init(584+2,718,1,2,bmpButton[325],bmpButton[326],bmpButton[327],bmpButton[327],objGUI);
	buttSort1.Activate(0);
	buttSort1.iHint=544;

	draw_sprite(bmpStat,bmpStuff[74],584+54,715);
	UI_Button1 buttSort2;
	objGUI=buttSort2.Init(584+2+54,718,1,2,bmpButton[328],bmpButton[329],bmpButton[330],bmpButton[330],objGUI);
	buttSort2.iHint=545;

	draw_sprite(bmpStat,bmpStuff[74],709,715);
	UI_Button1 buttType1;
	objGUI=buttType1.Init(709+2,718,1,2,bmpButton[331],bmpButton[332],bmpButton[333],bmpButton[333],objGUI);
	buttType1.Activate(0);
	buttType1.iHint=546;

	draw_sprite(bmpStat,bmpStuff[74],709+54,715);
	UI_Button1 buttType2;
	objGUI=buttType2.Init(709+2+54,718,1,2,bmpButton[334],bmpButton[335],bmpButton[336],bmpButton[336],objGUI);
	buttType2.iHint=547;

	draw_sprite(bmpStat,bmpStuff[74],834,715);
	UI_Button1 buttPgUp;
	objGUI=buttPgUp.Init(834+2,718,1,2,bmpButton[337],bmpButton[338],bmpButton[339],bmpButton[340],objGUI);
	buttPgUp.ipHotkey=KEY_PGUP;
	buttPgUp.iHint=548;
	
	draw_sprite(bmpStat,bmpStuff[74],834+54,715);
	UI_Button1 buttPgDn;
	objGUI=buttPgDn.Init(834+2+54,718,1,2,bmpButton[341],bmpButton[342],bmpButton[343],bmpButton[344],objGUI);
	buttPgDn.ipHotkey=KEY_PGDN;
	buttPgDn.iHint=549;
	


	/*UI_Button1 buttSite;
	objGUI=buttSite.Init(600,715,1,2,bmpButton[87],bmpButton[88],bmpButton[90],bmpButton[90],objGUI);
	buttSite.ipHotkey=KEY_S;*/
	
	GUI_ProvBar Bars[MAX_PROV];
	bool bRedraw=true, bArrange=true, bOrder=true;
	for (int i=0; i<MAX_PROV; i++)
	{
		objGUI=Bars[i].Init(17+i*70,2,pFirstProvince,true,objGUI);		
	};
	int ProvLimit, Shift=0, MaxProv=0, Mode=0;
	int Plist[MAX_MAP][2], Tlist[MAX_MAP][MAX_MODE];
	for (int i=0; i<MAX_MAP; i++)
	{
		Plist[i][0]=0;		
	};
	ClearMoveMap();
	MovePlayer=0;
	WalkType=10;
	WalkSpeed=1;
	MoveMap(arPlayer[Player].ipProvince,-1,WalkSpeed,0,0);
	for (int i=1; i<=MapSize; i++)
	{
		if ((pFirstProvince+i)->ipPlayer==Player)
		{
			Plist[MaxProv][0]=i;
			Tlist[MaxProv][0]=100-*(pMoveMap+i);
			if (i==arPlayer[Player].ipProvince)
			{Tlist[MaxProv][0]=101;};
			if (*(pMoveMap+i)<0)
			{Tlist[MaxProv][0]=-100;};
			Tlist[MaxProv][1]=(pFirstProvince+i)->GetGoldIncome();
			Tlist[MaxProv][2]=(pFirstProvince+i)->GetGemIncome();
			Tlist[MaxProv][3]=(pFirstProvince+i)->iPopulation;
			Tlist[MaxProv][4]=(pFirstProvince+i)->GetMood();
			Tlist[MaxProv][5]=(pFirstProvince+i)->arUnrest[(pFirstProvince+i)->ipPlayer];
			Tlist[MaxProv][6]=(pFirstProvince+i)->iExplore;
			Tlist[MaxProv][7]=(pFirstProvince+i)->ipRace;
			Tlist[MaxProv][8]=0;
			for (int j=0; j<MAX_PROVINCE_SITE; j++)
			{
				if (((pFirstProvince+i)->arSite[j][0]>0)&&((pFirstProvince+i)->arSite[j][1]<=(pFirstProvince+i)->iExplore))
				{Tlist[MaxProv][8]++;};
			};
			Tlist[MaxProv][9]=0;
			for (int j=0; j<MAX_PROVINCE_BLD; j++)
			{
				if ((pFirstProvince+i)->arBuilding[j]>0)
				{Tlist[MaxProv][9]++;};
			};
			MaxProv++;
		};
	};
	

	int Scroller=mouse_z;

	while (Result<0)
	{
		if (bArrange)
		{
			for (int i=0; i<400; i++)
			{Plist[i][1]=0;};
			int Max=0, Min;
			for (int i=1; i<=MapSize; i++)
			{
				if ((pFirstProvince+i)->ipPlayer==Player)
				{			
					Plist[Max][0]=i;
					Max++;
				};
			};
			int Choosen;
			for (int i=0; i<MaxProv; i++)
			{
				Max=-32000;
				Min=32000;
				for (int j=0; j<MaxProv; j++)
				{
					if (Plist[j][0]>0)
					{
						if (bOrder)
						{
							if (Tlist[j][Mode]>Max)
							{
								Max=Tlist[j][Mode];
								Choosen=j;
							};
						}
						else
						{
							if (Tlist[j][Mode]<Min)
							{
								Min=Tlist[j][Mode];
								Choosen=j;
							};
						};
					};
				};
				Plist[i][1]=Plist[Choosen][0];
				Plist[Choosen][0]=0;
			};
			bRedraw=true;
			bArrange=false;
		};
		if (bRedraw)
		{
			for (int i=0; i<MAX_PROV; i++)
			{Bars[i].Hide();};
			ProvLimit=0;
			int Delta=Shift;
			for (int i=Shift; ((i<MaxProv)&&(ProvLimit<MAX_PROV)); i++)
			{				
				Bars[ProvLimit].Activate((pFirstProvince+Plist[i][1]));
				ProvLimit++;
			};
			if (Shift+MAX_PROV<MaxProv)
			{buttPgDn.Activate(1);}
			else
			{buttPgDn.Activate(0);};
			if (Shift>0)
			{buttPgUp.Activate(1);}
			else
			{buttPgUp.Activate(0);};
			bRedraw=false;
		};
		for (int i=0; i<ProvLimit; i++)
		{
			if (Bars[i].Check(objGUI,4))
			{
				Result=Bars[i].PR->iNumber;
			};
		};
		for (int i=0; i<MAX_MODE; i++)
		{
			if ((buttMode[i].Check())&&(i!=Mode))
			{
				buttMode[Mode].Activate(1);
				Mode=i;
				buttMode[Mode].Activate(0);
				Shift=0;
				bArrange=true;
			};
		};

		if (buttType1.Check())
		{
			for (int i=0; i<MAX_PROV; i++)
			{				
				Bars[i].Mode=1-Bars[i].Mode;				
			};
			bRedraw=true;
			buttType1.Activate(0);
			buttType2.Activate(1);
		};
		if (buttType2.Check())
		{
			for (int i=0; i<MAX_PROV; i++)
			{				
				Bars[i].Mode=1-Bars[i].Mode;				
			};
			bRedraw=true;
			buttType1.Activate(1);
			buttType2.Activate(0);
		};

		if (buttSort1.Check())
		{
			bOrder=!bOrder;
			Shift=0;
			bArrange=true;
			buttSort1.Activate(0);
			buttSort2.Activate(1);
		};
		if (buttSort2.Check())
		{
			bOrder=!bOrder;
			Shift=0;
			bArrange=true;
			buttSort1.Activate(1);
			buttSort2.Activate(0);
		};
		if (buttPgDn.Check())
		{
			Shift+=MAX_PROV;
		//	if (Shift+MAX_PROV>MaxProv)
		//	{Shift=MaxProv-MAX_PROV;};
			bRedraw=true;
		};
		if (buttPgUp.Check())
		{
			Shift-=MAX_PROV;
			if (Shift<0)
			{Shift=0;};
			bRedraw=true;
		};

		if (mouse_z<Scroller)
		{
			Scroller=mouse_z;
			if (Shift+MAX_PROV<MaxProv)
			{
				Shift++;
				bRedraw=true;
			};
		};
		if (mouse_z>Scroller)
		{
			Scroller=mouse_z;
			if (Shift>0)
			{
				Shift--;
				bRedraw=true;
			};
		};
		if (buttExit.Check())
		{Result=0;};
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,4,true);
		}
		else
		{Sleeper(24);};
		bHint=false;		
	};
	destroy_bitmap(bmpStat);	
	iAntisleep=iAntisleepDelay;
	return Result;
};