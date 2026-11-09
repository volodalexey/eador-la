#include "Windower.h"
#include "string.h"

int DialogDX=0, DialogDY=0, EventDX=0;
int iScouting=0, iBribe=0;
char stScouting[2000];

//***************************************************
//*** Вывод окна диалога ****************************
//***************************************************
int clDialog::Show(int A1, int A2, int A3, int A4, BITMAP *Picture, char *sNewText)
{
	iAntisleep=iAntisleepDelay;
	bool bIdle;
	int iWindPic=iWindow-1;
	if (iWindow==5)
	{iWindPic=2;};
	
	int Result=0;
	int x=512-bmpDialogWindow[iWindPic]->w/2+DialogDX;
	int y=384-bmpDialogWindow[iWindPic]->h/2+DialogDY;	
//	const int w=60;
//	const int h=31;
//	const int dx=143;
//	const int dy=97;
	int bAnswer[MAX_ANSWER];
	bAnswer[0]=A1;
	bAnswer[1]=A2;
	bAnswer[2]=A3;
	bAnswer[3]=A4;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);	
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(190,190,190,190);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	set_trans_blender(128,128,128,128);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpDialogWindow[iWindPic]->w,bmpDialogWindow[iWindPic]->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	UI_Button1 buttHero;
	if (iDialogButt>0)
	{
		WObj=new G_Object(WObj,x+bmpWindow->w/2-bmpStuff[75]->w/2,y+bmpWindow->h-3,2,bmpStuff[75]);
		WObj=buttHero.Init(x+bmpWindow->w/2-bmpStuff[75]->w/2+79,y+bmpWindow->h+1,1,3,bmpButton[250],bmpButton[251],bmpButton[252],bmpButton[253],WObj);
		buttHero.iHint=146;
		if (arPlayer[pAdvice->ipPlayer].AIUnion[0]==0)
		{buttHero.bBlink=true;}
	};
	blit(bmpDialogWindow[iWindPic],bmpWindow,0,0,0,0,bmpDialogWindow[iWindPic]->w,bmpDialogWindow[iWindPic]->h);
	UI_TextBox txtMess, txtAnswer[MAX_ANSWER], txtAdvice;
	BITMAP *bmpAnswer[MAX_ANSWER];
	UI_Mask mskAnswer[MAX_ANSWER];
	BITMAP *bmpAdvice;
	UI_Mask mskLoot[5];
	int AdvTrans=0;
	if ((bMessageFor)&&(CurGame.iGameType==3))
	{
		iHeroAdvice=3;		
	};	
	if (iHeroAdvice>0)
	{
		bmpAdvice=create_bitmap(bmpGUI_HeroAdvice->w,bmpGUI_HeroAdvice->h);
		blit(bmpGUI_HeroAdvice,bmpAdvice,0,0,0,0,bmpGUI_HeroAdvice->w,bmpGUI_HeroAdvice->h);
		WObj=new G_Object(WObj,x+84,3,1,bmpAdvice);
		WObj->bTrans=true;
		WObj->iOpacity=1;
	};
	G_Object *objAdvice=WObj;
	if (iHeroAdvice==1)
	{		
		ClearRoll();
		int Count=0;
		for (int i=0; i<=iMaxHeroAdvice; i++)
		{
			if (((pFirstHeroAdvice+i)->Min<=iAdvice)&&((pFirstHeroAdvice+i)->Max>=iAdvice))
			{
				RollerN[Count]=i+1;
				RollerP[Count]=10;
				Count++;
			};
		};
		iAdvice=Roll(false)-1;
		WObj=txtAdvice.Init(x+84+149,3+46,200,118,-1,COLOR_1,Font2,WObj);
		txtAdvice.Activate((pFirstHeroAdvice+iAdvice)->sText);
		draw_sprite(bmpAdvice,txtAdvice.Pic,149,46);
		draw_sprite(bmpAdvice,pAdvice->Pic,3,28);
		textprintf_centre_ex(bmpAdvice,Font1,182,10,COLOR_1,-1,pAdvice->pstName->str);
		AdvTrans=1;
	};
	if (iHeroAdvice==2)
	{		
		WObj=txtAdvice.Init(x+84+149,3+46,200,118,-1,COLOR_1,Font2,WObj);
		txtAdvice.Activate((pFirstMessage+51)->msg);
		draw_sprite(bmpAdvice,txtAdvice.Pic,149,46);
		draw_sprite(bmpAdvice,bmpAvatar[18],3,28);
		textprintf_centre_ex(bmpAdvice,Font1,182,10,COLOR_1,-1,(pFirstPhrase+443)->str);
		AdvTrans=1;	
	};
	if (iHeroAdvice==3)
	{		
		WObj=txtAdvice.Init(x+84+149,3+46,200,118,-1,COLOR_1,Font2,WObj);
		char MSS[200];
		sprintf(MSS,(pFirstPhrase+449)->str,arPlayer[iMessageFor].GetName());
		txtAdvice.Activate(MSS);
		draw_sprite(bmpAdvice,txtAdvice.Pic,149,46);
		draw_sprite(bmpAdvice,arPlayer[iMessageFor].GetAvatar(),3,28);
//		textprintf_centre_ex(bmpAdvice,Font1,182,10,COLOR_1,-1,(pFirstPhrase+443)->str);
		AdvTrans=51;	
	};
	int AY;
	char ANS[61];
	/*draw_sprite(bmpWindow,(pFirstUnitType+Unit->ipType)->Pic,15,13);
	UI_Button1 * buttExit=new UI_Button1();
	WObj=buttExit->Init(x+321,y+484,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit->Activate(1);
	buttExit->ipHotkey=KEY_ESC;*/
	int dy=5;
	switch (iWindow)
	{
	case 1:
		WObj=txtMess.Init(x+26, y+37, 247, 96, 2, COLOR_1, Font2, WObj);
		if (sNewText)
		{txtMess.Activate(sNewText);}
		else
		{txtMess.Activate(sText);};
		for (int i=0; i<MAX_ANSWER; i++)
		{
			if (bAnswer[i])
			{
				textprintf_centre_ex(bmpWindow,Font1,150,166+i*25,COLOR_2,-1,arAnswer[i]);
				bmpAnswer[i]=create_bitmap(266,20);
				rectfill(bmpAnswer[i],0,0,266,20,makecol(30,30,30));
				textprintf_centre_ex(bmpAnswer[i],Font1,133,4,makecol(250,250,30),-1,arAnswer[i]);
				WObj=mskAnswer[i].Init(x+18,y+162+i*25,1,bmpAnswer[i],bmpAnswer[i],WObj,2);
			};
		};
		break;
	case 3:
	case 5:
		if (Picture)
		{
			draw_sprite(bmpWindow,Picture,12,19);
		}
		else
		{
			if (Pic)
			{
				draw_sprite(bmpWindow,Pic,12,19);
			};

		};
		WObj=txtMess.Init(x+151, y+23, 330, 250, 2, COLOR_2, Font2, WObj);
		if (sNewText)
		{txtMess.Activate(sNewText);}
		else
		{txtMess.Activate(sText);};
		AY=298;
		for (int i=0; i<MAX_ANSWER; i++)
		{
			if (bAnswer[i]!=0)
			{
				WObj=txtAnswer[i].Init(x+17, y+AY, 467, 100, 2, makecol(250,250,30), Font1, WObj);				
				txtAnswer[i].bCentre=true;
				if (iWindow==3)
				{txtAnswer[i].Activate(arAnswer[i]);}
				else
				{
					if (bAnswer[i]<0)
					{txtAnswer[i].Activate(arAnswer[abs(bAnswer[i])-1]);}
					else
					{txtAnswer[i].Activate((pFirstQuestion+bAnswer[i]-1)->sText);};
				};
				/*if (txtAnswer[i].Lines==1)
				{
					bmpAnswer[i]=create_bitmap(467,20);
					rectfill(bmpAnswer[i],0,0,467,20,makecol(30,30,30));
					draw_sprite(bmpAnswer[i],txtAnswer[i].Pic,0,4);
					WObj=mskAnswer[i].Init(x+17,y+AY-4,1,bmpAnswer[i],bmpAnswer[i],WObj,3);
					AY+=25;
				}
				else*/
				{
					int FontSize=text_height(Font1)+2;
					bmpAnswer[i]=create_bitmap(470,txtAnswer[i].Lines*FontSize+5);
					rectfill(bmpAnswer[i],0,0,470,txtAnswer[i].Lines*FontSize+5,makecol(30,30,30));
					draw_sprite(bmpAnswer[i],txtAnswer[i].Pic,2,4);
					WObj=mskAnswer[i].Init(x+15,y+AY-4,1,bmpAnswer[i],bmpAnswer[i],WObj,3);
					AY+=5+txtAnswer[i].Lines*FontSize+5;
				};
				txtAnswer[i].Color=COLOR_2;
				if (iWindow==3)
				{txtAnswer[i].Activate(arAnswer[i]);}
				else
				{
					if (bAnswer[i]<0)
					{txtAnswer[i].Activate(arAnswer[abs(bAnswer[i])-1]);}
					else
					{txtAnswer[i].Activate((pFirstQuestion+bAnswer[i]-1)->sText);};
				};
			};
		};
		if (AY<464)
		{
			blit(bmpDialogWindow[iWindPic],bmpWindow,0,464,0,AY,bmpDialogWindow[iWindPic]->w,16);
			rectfill(bmpWindow,0,AY+16,bmpDialogWindow[iWindPic]->w,bmpDialogWindow[iWindPic]->h,COLOR_0);
		};
		break;
	case 2:
	case 4:
		if (iWindow==4)
		{
			dy+=66;
			textprintf_centre_ex(bmpWindow,Font1,100,304,COLOR_2,-1,"%d",Treasurez[0][0]);
			textprintf_centre_ex(bmpWindow,Font1,100,329,COLOR_2,-1,"%d",Treasurez[0][1]);
			for (int i=1; i<6; i++)
			{
				if (Treasurez[i][0]>0)
				{
					draw_sprite(bmpWindow,(pFirstItem+Treasurez[i][0])->Pic,187+(i-1)*60,296);
					WObj=mskLoot[i-1].Init(x+181+(i-1)*60,y+290,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
				};
				if (Treasurez[i][0]<0)
				{
					draw_sprite(bmpWindow,(pFirstInnerBld+abs(Treasurez[i][0]))->Pic,187+(i-1)*60,296);
					WObj=mskLoot[i-1].Init(x+181+(i-1)*60,y+290,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
				};
			};

		};
		if (Picture)
		{
			draw_sprite(bmpWindow,Picture,16,20);
		}
		else
		{
			if (Pic)
			{
				draw_sprite(bmpWindow,Pic,16,20);
			};

		};
//		draw_sprite(bmpWindow,Pic,16,20);

		WObj=txtMess.Init(x+192, y+24, 286, 254, 2, COLOR_2, Font2, WObj);
		if (sNewText)
		{txtMess.Activate(sNewText);}
		else
		{txtMess.Activate(sText);};
		for (int i=0; i<MAX_ANSWER; i++)
		{
			int CH=0, CR=0, GL=0, Add=0;
			if ((arAnswer[i][1]!='#')||(iScouting>0)||(iBribe>0))
			{
				if (arAnswer[i][1]=='#')
				{
					if (iScouting>0)
					{
						sprintf(ANS,(pFirstMessage+53)->msg,iScouting);
						iScouting=0;
					}
					else
					{
						sprintf(ANS,(pFirstPhrase+502)->str,iBribe);
						iBribe=0;
					};
				}
				else
				{sprintf(ANS,arAnswer[i]);};
				while ((CH<61)&&(ANS[CH]!='\0'))
				{			
					if (ANS[CH]=='$')
					{
					//	strncpy(txSample,ANS,CH);
						ANS[CH]=' ';
						if (GL==0)
						{
							GL=CH*9-9+Add;
						//	Add=9;
						};
					};
					if (ANS[CH]=='&')
					{
					//	strncpy(txSample,ANS,CH);
						ANS[CH]=' ';
						if (CR==0)
						{
							CR=CH*9-9+Add;
						//	Add=9;
						};
					};
					CH++;
				};		
			};
			if (bAnswer[i]!=0)
			{
			//	textprintf_centre_ex(bmpWindow,Font1,251,288+i*25,COLOR_2,-1,ANS);
				textprintf_ex(bmpWindow,Font1,251-CH*9/2,288+i*25+dy,COLOR_2,-1,ANS);
				if (GL>0)
				{draw_sprite(bmpWindow,bmpGUI_Gold,251-CH*9/2+GL,284+i*25+dy);};
			//	{draw_sprite(bmpWindow,bmpGUI_Gold,251-text_length(Font1,ANS)/2+GL,284+i*25);};
				if (CR>0)
			//	{draw_sprite(bmpWindow,bmpGUI_Crystal,251-text_length(Font1,ANS)/2+CR,284+i*25);};
				{draw_sprite(bmpWindow,bmpGUI_Crystal,251-CH*9/2+CR,282+i*25+dy);};
				bmpAnswer[i]=create_bitmap(470,20);
				rectfill(bmpAnswer[i],0,0,470,20,makecol(30,30,30));
				textprintf_ex(bmpAnswer[i],Font1,235-CH*9/2,4,makecol(250,250,30),-1,ANS);
			//	textprintf_centre_ex(bmpAnswer[i],Font1,235,4,makecol(250,250,230),-1,ANS);
				if (GL>0)
				{draw_sprite(bmpAnswer[i],bmpGUI_Gold,235-CH*9/2+GL,0);};
			//	{draw_sprite(bmpAnswer[i],bmpGUI_Gold,235-text_length(Font1,ANS)/2+GL,0);};
				if (CR>0)
				{draw_sprite(bmpAnswer[i],bmpGUI_Crystal,235-CH*9/2+CR,-2);};
			//	{draw_sprite(bmpAnswer[i],bmpGUI_Crystal,235-text_length(Font1,ANS)/2+CR,0);};
				WObj=mskAnswer[i].Init(x+16,y+284+i*25+dy,1,bmpAnswer[i],bmpAnswer[i],WObj,2);
			}
			else
			{
				if (arAnswer[i][1]!='#')
				{
					textprintf_ex(bmpWindow,Font1,251-CH*9/2,288+i*25+dy,makecol(150,150,150),-1,ANS);
					if (GL>0)
					{draw_trans_sprite(bmpWindow,bmpGUI_Gold,251-CH*9/2+GL,284+i*25+dy);};
					if (CR>0)
					{draw_trans_sprite(bmpWindow,bmpGUI_Crystal,251-CH*9/2+CR,282+i*25+dy);};
				};
			};
		};
		break;
	};
	bHourGlass=false;
	while (Result==0)
	{
		for (int i=0; i<MAX_ANSWER; i++)
		{
			if (bAnswer[i])
			{
				if (mskAnswer[i].Check()==30)
				{
					Result=i+1;
				};
			};
		};
		if (iWindow==4)
		{
			for (int i=1; i<6; i++)
			{
				if (Treasurez[i][0]!=0)
				{
					int Check=mskLoot[i-1].Check();
					{
						if (Check>1)
						{
							if (Treasurez[i][0]>0)
							{
								if ((Check==30)||(Check==40))
								{
									bHint=false;
									WObj=ShowActiveObjects(WObj,6,1);
									WindowItemInfo(Treasurez[i][0],Treasurez[i][1]);
								}
								else
								{
									MakeItemHint(Treasurez[i][0],Treasurez[i][1],NULL,0,0,0);
									bHint=true;
								};
							}
							else
							{
								if ((Check==30)||(Check==40))
								{
									bHint=false;
									WObj=ShowActiveObjects(WObj,6,1);
									WindowInfo((pFirstPhrase+438)->str,(pFirstMessage+50)->msg,(pFirstInnerBld+abs(Treasurez[i][0]))->Pic);
								}
								else
								{
									MakeInnBldHint(abs(Treasurez[i][0]),0);
									bHint=true;
								};
							};
						};
					};
				};
			};
		};
		if (iDialogButt>0)
		{
			if (buttHero.Check())
			{
				int iTemp=iDialogButt;
				dcHero *pTemp=pAdvice;
				iDialogButt=0;
				ScreenHero(pAdvice,NULL,false);
				pAdvice=pTemp;
				iDialogButt=iTemp;
				buttHero.bBlink=false;
				arPlayer[pAdvice->ipPlayer].AIUnion[0]=1;
		//		if (AdvTrans>0)
		//		{set_trans_blender(AdvTrans,AdvTrans,AdvTrans,AdvTrans);};
			};
		};
		if (Timer)
		{
			Timer=false;
			bIdle=true;
			if ((AdvTrans>0)&&(AdvTrans<255))
			{
				//set_trans_blender(AdvTrans,AdvTrans,AdvTrans,AdvTrans);
				objAdvice->iOpacity=AdvTrans;
				AdvTrans+=5;
				bIdle=false;
			};
			WObj=ShowActiveObjects(WObj,6,1);
			bHint=false;
		}
		else
		{
			if (bIdle)
			{Sleeper(24);};
		};
		bHint=false;
	};

	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};	
	for (int i=0; i<MAX_ANSWER; i++)
	{
		if (bAnswer[i])
		{
			destroy_bitmap(bmpAnswer[i]);
		};
	};	
	blit(bmpBack,screen1,0,0,0,0,1024,768);
//	blit(bmpBack,screen,0,0,0,0,1024,768);	
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	DialogDX=0; DialogDY=0;
	if (iHeroAdvice>0)
	{
		destroy_bitmap(bmpAdvice);
		iHeroAdvice=0;
	//	set_trans_blender(128,128,128,128);
	};
	iDialogButt=0;
	iAntisleep=iAntisleepDelay;
	return Result;
};

//***************************************************
//*** Окно повышения юнита **************************
//***************************************************
void WindowUnitLvlUp(dcUnit * Unit, int Player)
{
	iAntisleep=iAntisleepDelay;
	if (arHumans[arPlayer[Player].iColor].iAutoLvlUp>=Unit->pType()->iLevel)
	{
		Unit->MakeLevel(Unit->LevelUp(Unit->iLevel+1, 0));
		return;
	};
	const int x=512-bmpGUI_UnitLvlUp->w/2;
	const int y=384-bmpGUI_UnitLvlUp->h/2;	
	int Upg[2];
	UI_Mask Mask[2];
	UI_Mask mskUnit;
	UI_TextBox txtLevel, txtDesc;

	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_UnitLvlUp->w,bmpGUI_UnitLvlUp->h);
	blit(bmpGUI_UnitLvlUp,bmpWindow,0,0,0,0,bmpGUI_UnitLvlUp->w,bmpGUI_UnitLvlUp->h);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpWindow,Unit->pType()->Shadow,37,70);
	draw_sprite(bmpWindow,Unit->pType()->Pic,37,70);
	Upg[0]=Unit->LevelUp(Unit->iLevel+1, 0);
	Upg[1]=Unit->LevelUp(Unit->iLevel+1, Upg[0]);
	draw_sprite(bmpWindow,(pFirstUnitUpg+Upg[0])->Pic,117,223);
	draw_sprite(bmpWindow,(pFirstUnitUpg+Upg[1])->Pic,256,223);
	textprintf_centre_ex(bmpWindow,Font1,142,24,COLOR_1,-1,"%s",Unit->pType()->sName);
	textprintf_centre_ex(bmpWindow,Font1,333,24,COLOR_1,-1,"%s:%d",(pFirstPhrase+5)->str,Unit->iLevel+1);
	textprintf_centre_ex(bmpWindow,Font1,217,191,COLOR_1,-1,"%s",(pFirstMessage+12)->msg);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	WObj=Mask[0].Init(x+111,y+217,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);
	WObj=Mask[1].Init(x+250,y+217,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);
	WObj=mskUnit.Init(x+29,y+62,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,WObj,3);
	WObj=txtLevel.Init(x+147,y+73,240,85,3,COLOR_1,Font2,WObj);
	txtLevel.Activate((pFirstMessage+14)->msg);
	WObj=txtDesc.Init(x+49,y+320,335,90,3,COLOR_1,Font2,WObj);
	txtDesc.Hide();	
	set_mouse_sprite(bmpMousePointer[1]);
	int Choose=-1;
	int Under=0;
	bool bUnder;
	while (Choose<0)
	{
		bUnder=true;
		switch (Mask[0].Check())
		{
		case 30:		
			Choose=0;
		case 2:
		case 3:
			bUnder=false;
			if (Under!=1)
			{
				Under=1;
			//	rectfill(bmpWindow,36,296,394,405,makecol(56,56,54));
				blit(bmpGUI_UnitLvlUp,bmpWindow,36,296,36,296,360,100);
				textprintf_centre_ex(bmpWindow,Font1,217,300,COLOR_1,-1,"%s",(pFirstUnitUpg+Upg[0])->sName);
				txtDesc.Activate((pFirstUnitUpg+Upg[0])->sText);
			};
		};		
		switch (Mask[1].Check())
		{
		case 30:		
			Choose=1;
		case 2:
		case 3:
			bUnder=false;
			if (Under!=2)
			{
				Under=2;
			//	rectfill(bmpWindow,36,296,394,405,makecol(56,56,54));
				blit(bmpGUI_UnitLvlUp,bmpWindow,36,296,36,296,360,100);
				textprintf_centre_ex(bmpWindow,Font1,217,300,COLOR_1,-1,"%s",(pFirstUnitUpg+Upg[1])->sName);
				txtDesc.Activate((pFirstUnitUpg+Upg[1])->sText);
			};			
		};
		if ((bUnder)&&(Under>0))
		{
			Under=0;
			blit(bmpGUI_UnitLvlUp,bmpWindow,36,296,36,296,360,100);
			txtDesc.Hide();
		//	rectfill(bmpWindow,36,296,394,405,makecol(56,56,54));
		};

		if (mskUnit.Check()>=30)
		{
			WindowUnitInfo(Unit, 0);
		};
		
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,4,1);
		}
		else
		{Sleeper(24);};
	};
	Unit->MakeLevel(Upg[Choose]);
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,screen1,0,0,0,0,1024,768);
	blit(bmpBack,screen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно награждения юнита ************************
//***************************************************
void WindowUnitReward(dcUnit * Unit, int Reward)
{
	iAntisleep=iAntisleepDelay;
	const int x=512-bmpGUI_UnitReward->w/2;
	const int y=384-bmpGUI_UnitReward->h/2;		
	UI_Mask mskUnit;
	UI_TextBox txtLevel, txtReward;
	UI_Button1 buttReward, buttCancel;
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_UnitReward->w,bmpGUI_UnitReward->h);
	blit(bmpGUI_UnitReward,bmpWindow,0,0,0,0,bmpGUI_UnitReward->w,bmpGUI_UnitReward->h);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpWindow,Unit->pType()->Shadow,37,70);	
	draw_sprite(bmpWindow,Unit->pType()->Pic,37,70);	
	draw_sprite(bmpWindow,(pFirstMedal+Reward)->Pic,189,221);	
	textprintf_centre_ex(bmpWindow,Font1,142,24,COLOR_1,-1,"%s",Unit->pType()->sName);
	textprintf_centre_ex(bmpWindow,Font1,333,24,COLOR_1,-1,"%s: %d",(pFirstPhrase+5)->str,Unit->iLevel);
	textprintf_centre_ex(bmpWindow,Font1,217,191,COLOR_1,-1,"%s",(pFirstMedal+Reward)->sName);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);	
	WObj=buttReward.Init(x+32,y+420,1,3,bmpButton[170],bmpButton[171],bmpButton[172],bmpButton[172],WObj);
	WObj=buttCancel.Init(x+347,y+420,1,3,bmpButton[84],bmpButton[85],bmpButton[86],bmpButton[86],WObj);
	buttReward.iHint=147;
	buttCancel.iHint=106;
	WObj=mskUnit.Init(x+29,y+62,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,WObj,3);
	WObj=txtLevel.Init(x+147,y+73,240,85,3,COLOR_1,Font2,WObj);
	txtLevel.Activate((pFirstMessage+15)->msg);
	WObj=txtReward.Init(x+49,y+310,335,84,3,COLOR_1,Font2,WObj);
	txtReward.Activate((pFirstMedal+Reward)->sText);
	set_mouse_sprite(bmpMousePointer[1]);
	int Choose=-1;
	while (Choose<0)
	{
		bHint=false;
		if (buttReward.Check())
		{
			Choose=0;
			for (int i=0; i<3; i++)
			{
				if (Unit->arMedal[i]==0)
				{
					Unit->arMedal[i]=Reward;
					break;
				};
			};
		};

		if (buttCancel.Check())
		{Choose=0;};

		if (mskUnit.Check()>=30)
		{
			WindowUnitInfo(Unit, 0);
		};
		
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,4,1);
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
	blit(bmpBack,screen1,0,0,0,0,1024,768);
	blit(bmpBack,screen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	iAntisleep=iAntisleepDelay;
};


//***************************************************
//*** Окно повышения героя **************************
//***************************************************
void WindowHeroLvlUp(dcHero * Hero)
{	
//	int Upg[4];
	iAntisleep=iAntisleepDelay;
	UI_Mask Mask[4];
	UI_Mask1 mskHero;	
	BITMAP *bmpBack=create_bitmap(1024,768);
	BITMAP *bmpWindow;
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	int OldLife=Hero->pSquad[0]->GetLife();
	char Text1[1000];
	UI_TextBox txtLevel, txtSkill, txtClass;	

	if (Hero->iLevel==9)
	{
		int W=bmpGUI_HeroLvl10->w, H=bmpGUI_HeroLvl10->h;
		bmpWindow=create_bitmap(W,H);
		int x=512-W/2, y=384-H/2;
		WObj=new G_Object(WObj,x,y,1,bmpWindow);
		blit(bmpGUI_HeroLvl10,bmpWindow,0,0,0,0,W,H);		
		draw_sprite(bmpWindow,Hero->Pic,285,37);
		mskHero.Init(x+285,y+37,132,150,1);
		WObj=txtLevel.Init(x+113,y+230,480,62,2,COLOR_1,Font2,WObj);
		WObj=txtSkill.Init(x+36,y+420,370,240,2,COLOR_1,Font2,WObj);
		txtSkill.Hide();
		WObj=txtClass.Init(x+443,y+500,228,161,2,COLOR_1,Font2,WObj);
		txtClass.Hide();
		sprintf(Text1,(pFirstMessage+11)->msg,ClassName[Hero->ipClass1-1].str,Hero->pstName->str);
		txtLevel.Activate(Text1);
		textprintf_centre_ex(bmpWindow,Font1,354,297,COLOR_1,-1,"%s",(pFirstMessage+12)->msg);

		Hero->LevelUp();
		
		int Masks=0;
		if (arHeroLvlUp[0][0]>0)
		{
			draw_sprite(bmpWindow,bmpHeroSkill[1+(arHeroLvlUp[0][0]-1)*5+Hero->apSkill[arHeroLvlUp[1][0]][1]],184,322);
			WObj=Mask[0].Init(x+178,y+316,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);
			Masks++;
		};

		if (arHeroLvlUp[0][1]>0)
		{
			draw_sprite(bmpWindow,bmpHeroSkill[1+(arHeroLvlUp[0][1]-1)*5+Hero->apSkill[arHeroLvlUp[1][1]][1]],279,322);
			WObj=Mask[1].Init(x+273,y+316,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);
			Masks++;
		};

		if (arHeroLvlUp[0][2]>0)
		{
			draw_sprite(bmpWindow,bmpHeroSkill[1+(arHeroLvlUp[0][2]-1)*5+Hero->apSkill[arHeroLvlUp[1][2]][1]],374,322);
			WObj=Mask[2].Init(x+368,y+316,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);
			Masks++;
		};

		if (arHeroLvlUp[0][3]>0)
		{
			draw_sprite(bmpWindow,bmpHeroSkill[1+(arHeroLvlUp[0][3]-1)*5+Hero->apSkill[arHeroLvlUp[1][3]][1]],469,322);
			WObj=Mask[3].Init(x+463,y+316,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);
			Masks++;
		};
		
		set_mouse_sprite(bmpMousePointer[1]);
		int Choose=-1;
		int Under=-1, Under1=-1;;
		while (Choose<0)
		{
			Under=-1;
			for (int i=0; i<Masks; i++)
			{			
				switch (Mask[i].Check())
				{
				case 30:		
					Choose=i;
				case 2:
				case 3:
					Under=i;					
				};		
			};
			int Check=mskHero.Check();
			if ((Check==30)||(Check==40))
			{WindowUnitInfo(Hero->pSquad[0],0);};
			if (Timer)
			{				
				if (Under!=Under1)
				{
					txtSkill.Hide();
					txtClass.Hide();
					blit(bmpGUI_HeroLvl10,bmpWindow,27,386,27,386,656,83);
			//		blit(bmpGUI_HeroLvl10,bmpWindow,30,412,30,412,360,20);					
					if (Under>=0)
					{
						textprintf_centre_ex(bmpWindow,Font1,216,389,COLOR_1,-1,"%s %d",arSkills[arHeroLvlUp[0][Under]].sName,Hero->apSkill[arHeroLvlUp[1][Under]][1]+1);
						textprintf_centre_ex(bmpWindow,Font1,557,389,COLOR_1,-1,(pFirstPhrase+453)->str);
						textprintf_centre_ex(bmpWindow,Font1,582,433,COLOR_1,-1,"%s",ClassName[3+(Hero->ipClass1-1)*4+arSkills[arHeroLvlUp[0][Under]].iClass].str);
						draw_sprite(bmpWindow,ClassPic[3+(Hero->ipClass1-1)*4+arSkills[arHeroLvlUp[0][Under]].iClass],433,412);
						txtSkill.Activate(arSkillText[arHeroLvlUp[0][Under]].msg);
						txtClass.Activate(ClassText[3+(Hero->ipClass1-1)*4+arSkills[arHeroLvlUp[0][Under]].iClass].msg);
					};
					Under1=Under;
				};
				Timer=false;
				WObj=ShowActiveObjects(WObj,4,1);
			}
			else
			{Sleeper(24);};
		};
		Hero->iLevel++;
		Hero->apSkill[arHeroLvlUp[1][Choose]][0]=arHeroLvlUp[0][Choose];
		Hero->apSkill[arHeroLvlUp[1][Choose]][1]++;
		Hero->ipClass2=arSkills[arHeroLvlUp[0][Choose]].iClass;
	}
	else
	{
		int W=bmpGUI_HeroLvlUp->w, H=bmpGUI_HeroLvlUp->h;
		bmpWindow=create_bitmap(W,H);
		int x=512-W/2, y=384-H/2;
		WObj=new G_Object(WObj,x,y,1,bmpWindow);
		blit(bmpGUI_HeroLvlUp,bmpWindow,0,0,0,0,W,H);
		draw_sprite(bmpWindow,Hero->Pic,21,17);
		mskHero.Init(x+20,y+17,132,150,1);
		WObj=txtLevel.Init(x+176,y+40,215,99,2,COLOR_1,Font2,WObj,x+271,y+16,x+271,y+153);
		WObj=txtSkill.Init(x+33,y+387,370,232,2,COLOR_1,Font2,WObj);
		txtSkill.Hide();		
		if (Hero->iLevel==19)
		{
			sprintf(Text1,ClassText[Hero->GetClassName()+16].msg);
		}
		else
		{
			sprintf(Text1,(pFirstMessage+10)->msg,Hero->pstName->str,ClassName[Hero->GetClassName()].str,Hero->iLevel+1);
		};
		txtLevel.Activate(Text1);
		textprintf_centre_ex(bmpWindow,Font1,215,246,COLOR_1,-1,"%s",(pFirstMessage+12)->msg);
		textprintf_centre_ex(bmpWindow,Font1,215,187,COLOR_1,-1,"%s",(pFirstMessage+13)->msg);
				
		int Stat=Hero->LevelUp();
		switch (Stat)
		{
		case 1:
			draw_sprite(bmpWindow,(pFirstAbilityNum+118)->Pic,27,178);
			draw_sprite(bmpWindow,(pFirstAbilityNum+118)->Pic,352,178);
			textprintf_centre_ex(bmpWindow,Font1,215,214,COLOR_1,-1,"%s +1",(pFirstPhrase+6)->str);
			Hero->iHealth++;
			break;
		case 2:			
			draw_sprite(bmpWindow,(pFirstAbilityNum+117)->Pic,27,178);
			draw_sprite(bmpWindow,(pFirstAbilityNum+117)->Pic,352,178);
			textprintf_centre_ex(bmpWindow,Font1,215,214,COLOR_1,-1,"%s +1",(pFirstPhrase+7)->str);
			Hero->iMagic++;
			break;
		case 3:			
			draw_sprite(bmpWindow,(pFirstAbilityNum+116)->Pic,27,178);
			draw_sprite(bmpWindow,(pFirstAbilityNum+116)->Pic,352,178);
			textprintf_centre_ex(bmpWindow,Font1,215,214,COLOR_1,-1,"%s +1",(pFirstPhrase+8)->str);
			Hero->iLeader++;
			break;
		case 0:			
			draw_sprite(bmpWindow,(pFirstAbilityNum+1)->Pic,27,178);
			draw_sprite(bmpWindow,(pFirstAbilityNum+1)->Pic,352,178);
			textprintf_centre_ex(bmpWindow,Font1,215,214,COLOR_1,-1,"%s +1",(pFirstPhrase+9)->str);
			Hero->iLifeBonus++;
			break;
		};
		int Masks=0;
		if (arHeroLvlUp[0][0]>0)
		{
			draw_sprite(bmpWindow,bmpHeroSkill[1+(arHeroLvlUp[0][0]-1)*5+Hero->apSkill[arHeroLvlUp[1][0]][1]],93,271);
			WObj=Mask[0].Init(x+87,y+265,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);
			Masks++;
		};

		if (arHeroLvlUp[0][1]>0)
		{
			draw_sprite(bmpWindow,bmpHeroSkill[1+(arHeroLvlUp[0][1]-1)*5+Hero->apSkill[arHeroLvlUp[1][1]][1]],188,271);
			WObj=Mask[1].Init(x+182,y+265,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);
			Masks++;
		};

		if (arHeroLvlUp[0][2]>0)
		{
			draw_sprite(bmpWindow,bmpHeroSkill[1+(arHeroLvlUp[0][2]-1)*5+Hero->apSkill[arHeroLvlUp[1][2]][1]],283,271);
			WObj=Mask[2].Init(x+277,y+265,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);
			Masks++;
		};
		
		set_mouse_sprite(bmpMousePointer[0]);
		int Choose=-1;
		int Under=-1, Under1=-1;;
		while (Choose<0)
		{
			Under=-1;
			for (int i=0; i<Masks; i++)
			{			
				switch (Mask[i].Check())
				{
				case 30:		
					Choose=i;
				case 2:
				case 3:
					Under=i;					
				};		
			};
			txtLevel.Check();
			int Check=mskHero.Check();
			if ((Check==30)||(Check==40))
			{WindowUnitInfo(Hero->pSquad[0],0);};
			if (Timer)
			{				
				if (Under!=Under1)
				{
					txtSkill.Hide();
					blit(bmpGUI_HeroLvlUp,bmpWindow,33,367,33,367,300,20);
					if (Under>=0)
					{
						textprintf_centre_ex(bmpWindow,Font1,215,367,COLOR_1,-1,"%s %d",arSkills[arHeroLvlUp[0][Under]].sName,Hero->apSkill[arHeroLvlUp[1][Under]][1]+1);
						txtSkill.Activate(arSkillText[arHeroLvlUp[0][Under]].msg);
					};
					Under1=Under;
				};
				Timer=false;
				WObj=ShowActiveObjects(WObj,4,1);
			}
			else
			{Sleeper(24);};
		};
		Hero->iLevel++;
		Hero->apSkill[arHeroLvlUp[1][Choose]][0]=arHeroLvlUp[0][Choose];
		Hero->apSkill[arHeroLvlUp[1][Choose]][1]++;
	};
	Hero->pSquad[0]->iLife+=(Hero->pSquad[0]->GetLife()-OldLife);
	Hero->ChangeUnitType();
	
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,screen1,0,0,0,0,1024,768);
	blit(bmpBack,screen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	iAntisleep=iAntisleepDelay;
};


//***************************************************
//*** Окно информации о юните ***********************
//***************************************************
void WindowUnitInfo(dcUnit * Unit, int Morale)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_UnitInfo->w/2;
	int y=384-bmpGUI_UnitInfo->h/2;	
	const int w=59;
	const int h=31;
	const int dx=60;
	const int dy=79;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);	
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_UnitInfo->w,bmpGUI_UnitInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_UnitInfo,bmpWindow,0,0,0,0,bmpGUI_UnitInfo->w,bmpGUI_UnitInfo->h);
	set_trans_blender(140,140,140,140);
	draw_trans_sprite(bmpWindow,Unit->pType()->Shadow,255-Unit->pType()->Pic->w/2,132-Unit->pType()->Pic->h/2);
	draw_sprite(bmpWindow,Unit->pType()->Pic,255-Unit->pType()->Pic->w/2,132-Unit->pType()->Pic->h/2);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+336,y+366,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;

	UI_Mask arMedalMask[3];
	int Medals=0;

	//Награды
	for (int i=0; i<3; i++)
	{
		if (Unit->arMedal[i]>0)
		{
			draw_sprite(bmpWindow,(pFirstMedal+Unit->arMedal[i])->Pic,323,91+i*60);
			WObj=arMedalMask[i].Init(x+317,y+85+i*60,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
			Medals++;
		};
	};
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+403,y+74,300,250,2,COLOR_1,Font2,WObj,x+542,y+44,x+542,y+334);
	
	if (Unit->pHero)
	{		
		textprintf_centre_ex(bmpWindow,Font1,370,17,COLOR_1,-1,"%s %s",Unit->pHero->pstName->str,ClassName[Unit->pHero->GetClassName()].str);
		textprintf_centre_ex(bmpWindow,Font1,254,208,COLOR_1,-1,"%s",(pFirstPhrase+11)->str);
		int Limit=(Unit->pHero->iExp-Unit->pHero->ExpForLevel(Unit->pHero->iLevel))*58/
			(Unit->pHero->ExpForLevel(Unit->pHero->iLevel+1)-Unit->pHero->ExpForLevel(Unit->pHero->iLevel));
		if (Limit>58)
		{Limit=58;}
		for (int i=0; i<Limit; i++)
		{
			rectfill(bmpWindow,319+i,66,320+i,83,makecol(60+i*2,20+i/2,70+i*2.5));
		};
		textprintf_centre_ex(bmpWindow,Font1,348,71,COLOR_1,-1,"%d",Unit->pHero->iLevel);
		textprintf_centre_ex(bmpWindow,Font1,347,70,makecol(220,220,0),-1,"%d",Unit->pHero->iLevel);
		draw_sprite(bmpWindow,ClassPic[Unit->pHero->ipClass1-1],323,91);
		char Txt[2000];
		sprintf(Txt,Unit->pType()->sText);
		if (Unit->pHero->iLevel>=10)
		{
			draw_sprite(bmpWindow,ClassPic[3+(Unit->pHero->ipClass1-1)*4+Unit->pHero->ipClass2],323,91+60);
			sprintf(Txt,(pFirstMessage+52)->msg,Txt,ClassText[3+(Unit->pHero->ipClass1-1)*4+Unit->pHero->ipClass2]);
		};
		if (Unit->pHero->iLevel>=20)
		{
			draw_sprite(bmpWindow,ClassPic[19+(Unit->pHero->ipClass1-1)*4+Unit->pHero->ipClass2],323,91+120);
			sprintf(Txt,(pFirstMessage+52)->msg,Txt,ClassText[19+(Unit->pHero->ipClass1-1)*4+Unit->pHero->ipClass2]);
		};
		TextBox.Activate(Txt);
		
	}
	else
	{
		TextBox.Activate(Unit->pType()->sText);
		textprintf_centre_ex(bmpWindow,Font1,370,17,COLOR_1,-1,"%s (%s)",Unit->pType()->sName,(pFirstPhrase+303+Unit->pType()->iKarma)->str);
		/*if ((pFirstUnitType+Unit->ipType)->iKarma>0)
		{
			textprintf_centre_ex(bmpWindow,Font1,193,20,makecol(220-(pFirstUnitType+Unit->ipType)->iKarma*60,220,40),-1,"%s",(pFirstUnitType+Unit->ipType)->sName);
		}
		else
		{
			textprintf_centre_ex(bmpWindow,Font1,193,20,makecol(220,220+(pFirstUnitType+Unit->ipType)->iKarma*60,40),-1,"%s",(pFirstUnitType+Unit->ipType)->sName);
		};*/
		textprintf_centre_ex(bmpWindow,Font1,254,208,COLOR_1,-1,"%s %d",(pFirstPhrase+10)->str,Unit->pType()->iLevel);
		int Limit=(Unit->iExp-Unit->ExpForLevel(Unit->iLevel))*58/(Unit->ExpForLevel(Unit->iLevel+1)-Unit->ExpForLevel(Unit->iLevel));
		if (Limit>58)
		{Limit=58;}
		for (int i=0; i<Limit; i++)
		{
			rectfill(bmpWindow,319+i,66,320+i,83,makecol(40+i*2,10+i/2,50+i*2.5));
		};
		textprintf_centre_ex(bmpWindow,Font1,348,71,COLOR_1,-1,"%d",Unit->iLevel);
		textprintf_centre_ex(bmpWindow,Font1,347,70,makecol(220,220,0),-1,"%d",Unit->iLevel);		
	};

	UI_Mask1 arParamMask[13];
	int arParam[13];
	arParamMask[1].Init(x+18,y+67,58,30,1);arParam[1]=Unit->GetLife();
	arParamMask[2].Init(x+18,y+67+h,58,30,1);arParam[2]=Unit->GetAttack();
	arParamMask[3].Init(x+18+w,y+67+h,58,30,1);arParam[3]=Unit->GetCounterAttack();
	arParamMask[4].Init(x+18,y+67+h*2,58,30,1);arParam[4]=Unit->GetDefence();
	arParamMask[5].Init(x+18+w,y+67+h*2,58,30,1);arParam[5]=Unit->GetRangedDefence();
	arParamMask[6].Init(x+18+w*2,y+67+h*2,58,30,1);arParam[6]=Unit->GetResist();
	arParamMask[7].Init(x+18+w*2,y+67+h,58,30,1);arParam[7]=Unit->GetSpeed();
	arParamMask[8].Init(x+18,y+67+h*3,58,30,1);arParam[8]=Unit->GetRangedAttack();
	arParamMask[9].Init(x+18+w,y+67+h*3,58,30,1);arParam[9]=Unit->GetShootingRange();
	arParamMask[10].Init(x+18+w*2,y+67+h*3,58,30,1);arParam[10]=Unit->GetAmmo();
	arParamMask[11].Init(x+18+w,y+67,58,30,1);arParam[11]=Unit->GetStamina();
	arParamMask[12].Init(x+18+w*2,y+67,58,30,1);arParam[12]=Unit->GetMorale();

	UI_Mask1 arSpecMask[5];	
	arSpecMask[0].Init(x+26,y+38,128,23,1);
	arSpecMask[0].iHint=88;
	arSpecMask[1].Init(x+174,y+38,128,23,1);
	arSpecMask[1].iHint=89;
	arSpecMask[2].Init(x+22,y+203,80,25,1);
	arSpecMask[2].iHint=90;
	arSpecMask[3].Init(x+104,y+203,80,25,1);
	arSpecMask[3].iHint=91;
	arSpecMask[4].Init(x+318,y+43,61,44,1);
	arSpecMask[4].iHint=5;

	UI_Mask1 mskRank;
	mskRank.Init(x+195,y+203,120,20,1);
	mskRank.iHint=495;

	textprintf_centre_ex(bmpWindow,Font1,dx,dy,COLOR_2,-1,"%d",Unit->GetLife());
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy,COLOR_2,-1,"%d",Unit->GetStamina());
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy,COLOR_2,-1,"%d",Unit->GetMorale());
	textprintf_centre_ex(bmpWindow,Font1,dx,dy+h,COLOR_2,-1,"%d",Unit->GetAttack());
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy+h,COLOR_2,-1,"%d",Unit->GetCounterAttack());
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy+h,COLOR_2,-1,"%d",Unit->GetSpeed());
	textprintf_centre_ex(bmpWindow,Font1,dx,dy+h*2,COLOR_2,-1,"%d",Unit->GetDefence());
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy+h*2,COLOR_2,-1,"%d",Unit->GetRangedDefence());
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy+h*2,COLOR_2,-1,"%d",Unit->GetResist());
	textprintf_centre_ex(bmpWindow,Font1,dx,dy+h*3,COLOR_2,-1,"%d",Unit->GetRangedAttack());
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy+h*3,COLOR_2,-1,"%d",Unit->GetShootingRange());
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy+h*3,COLOR_2,-1,"%d",Unit->GetAmmo());

	int Limit=(Unit->iLife)*100/Unit->GetLife();
	if (Limit>100)
	{Limit=100;}
	for (int i=0; i<Limit; i++)
	{
		rectfill(bmpWindow,50+i,40,51+i,58,makecol(50+i*2,10+i/2,10+i/2));
	};
	textprintf_centre_ex(bmpWindow,Font1,102,46,COLOR_1,-1,"%d",Unit->iLife);
	textprintf_centre_ex(bmpWindow,Font1,101,45,COLOR_2,-1,"%d",Unit->iLife);

	if (Unit->GetMorale()>0)
	{
		 Limit=(Unit->GetMorale()+Unit->iMorale/2+Morale)*100/Unit->GetMorale();
		if (Limit>100)
		{Limit=100;}
		for (int i=0; i<Limit; i++)
		{
			rectfill(bmpWindow,204+i,40,205+i,58,makecol(10+i/2,10+i/2,50+i*2));
		};
	};
	textprintf_centre_ex(bmpWindow,Font1,258,46,COLOR_1,-1,"%d",Unit->GetMorale()+Unit->iMorale/2+Morale);
	textprintf_centre_ex(bmpWindow,Font1,257,45,COLOR_2,-1,"%d",Unit->GetMorale()+Unit->iMorale/2+Morale);
	textprintf_centre_ex(bmpWindow,Font1,64,210,COLOR_2,-1,"%d",Unit->GetGoldPayment());
	textprintf_centre_ex(bmpWindow,Font1,150,210,COLOR_2,-1,"%d",Unit->GetGemPayment());

	
	int iAbil=0, MaxAbilMask=0;
	int arAbil[2][50];
	for (int i=0; i<50; i++)
	{arAbil[0][i]=0;};
	for (int i=13; i<3000; i++)
	{
		int Skill=Unit->CheckAbility(i)+Unit->CheckUpg(i);		
		if ((iAbil<50)&&(Skill>0))
		{			
			for (int j=1; j<=iMaxAbilityNum; j++)
			{
				if ((pFirstAbilityNum+j)->Num==i)
				{
					arAbil[0][iAbil]=i;
					arAbil[1][iAbil]=Skill;
					if (iAbil<10)
					{
						draw_sprite(bmpWindow,(pFirstAbilityNum+j)->Pic,17+iAbil%5*60, 240+(iAbil-iAbil%5)/5*60);
						if ((pFirstAbilityNum+j)->bNumeric)
						{
							ShowIconNumber(bmpWindow, 17+iAbil%5*60, 240+(iAbil-iAbil%5)/5*60,Skill);
							//textprintf_centre(bmpWindow,Font1,66+iAbil%5*60,287+(iAbil-iAbil%5)/5*60,COLOR_1,"%d",Skill);
							//textprintf_centre(bmpWindow,Font1,65+iAbil%5*60,286+(iAbil-iAbil%5)/5*60,COLOR_2,"%d",Skill);
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
	UI_Mask arAbilMask[10];
	for (int i=0; i<MaxAbilMask; i++)
	{
		WObj=arAbilMask[i].Init(x+11+i%5*60, y+234+(i-i%5)/5*60,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
	};
	int iAbilLine=0;
	UI_Button1 buttScrollUp1;
	WObj=buttScrollUp1.Init(x+313,y+272,1,2,bmpButton[76],bmpButton[77],bmpButton[78],bmpButton[79],WObj);
	buttScrollUp1.Activate(0);	
	UI_Button1 buttScrollDown1;
	WObj=buttScrollDown1.Init(x+313,y+310,1,2,bmpButton[80],bmpButton[81],bmpButton[82],bmpButton[83],WObj);
	if (iAbil<10)
	{buttScrollDown1.Activate(0);}
	else
	{buttScrollDown1.Activate(1);};

		

	while (!buttExit.Check())
	{
		if (buttScrollUp1.Check())
		{
			iAbilLine--;
			if (iAbilLine==0)
			{buttScrollUp1.Activate(0);};
			buttScrollDown1.Activate(1);
			blit(bmpGUI_UnitInfo,bmpWindow,15,237,15,237,310,125);
			for (int i=0; i<10; i++)
			{
				if (arAbil[0][i+iAbilLine*10]>0)
				{
					for (int j=1; j<=iMaxAbilityNum; j++)
					{
						if ((pFirstAbilityNum+j)->Num==arAbil[0][i+iAbilLine*10])
						{
							draw_sprite(bmpWindow,(pFirstAbilityNum+j)->Pic,17+i%5*60, 240+(i-i%5)/5*60);
							if ((pFirstAbilityNum+j)->bNumeric)
							{
								ShowIconNumber(bmpWindow, 17+i%5*60, 240+(i-i%5)/5*60,arAbil[1][i+iAbilLine*10]);
							//	textprintf_centre(bmpWindow,Font1,66+i%5*60,287+(i-i%5)/5*60,COLOR_1,"%d",arAbil[1][i+iAbilLine*10]);
							//	textprintf_centre(bmpWindow,Font1,65+i%5*60,286+(i-i%5)/5*60,COLOR_2,"%d",arAbil[1][i+iAbilLine*10]);
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
			if (iAbilLine>=(iAbil-iAbil%10)/10)
			{buttScrollDown1.Activate(0);};
			buttScrollUp1.Activate(1);
			blit(bmpGUI_UnitInfo,bmpWindow,15,237,15,237,310,125);
			for (int i=0; i<10; i++)
			{
				if (arAbil[0][i+iAbilLine*10]>0)
				{
					for (int j=1; j<=iMaxAbilityNum; j++)
					{
						if ((pFirstAbilityNum+j)->Num==arAbil[0][i+iAbilLine*10])
						{
							draw_sprite(bmpWindow,(pFirstAbilityNum+j)->Pic,17+i%5*60, 240+(i-i%5)/5*60);
							if ((pFirstAbilityNum+j)->bNumeric)
							{
								ShowIconNumber(bmpWindow, 17+i%5*60, 240+(i-i%5)/5*60,arAbil[1][i+iAbilLine*10]);
						//		textprintf_centre(bmpWindow,Font1,66+i%5*60,287+(i-i%5)/5*60,COLOR_1,"%d",arAbil[1][i+iAbilLine*10]);
						//		textprintf_centre(bmpWindow,Font1,65+i%5*60,286+(i-i%5)/5*60,COLOR_2,"%d",arAbil[1][i+iAbilLine*10]);
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
		TextBox.Check();
		bHint=false;
		for (int i=0; i<MaxAbilMask; i++)
		{
			int Check=arAbilMask[i].Check();
			if (Check>1)
			{
				bHint=true;
				MakeAbilityHint(arAbil[0][i+iAbilLine*10]);
			};
			if ((Check==30)||(Check==40))
			{
				bHint=false;				
				WObj=ShowActiveObjects(WObj,6,0);
				WindowAbilityInfo(arAbil[0][i+iAbilLine*10],arAbil[1][i+iAbilLine*10]);
			};
		};
		for (int i=0; i<Medals; i++)
		{
			int Check=arMedalMask[i].Check();
			if ((Check==30)||(Check==40))
			{
				bHint=false;
				WObj=ShowActiveObjects(WObj,6,0);
				WindowMedalInfo(Unit->arMedal[i]);}
			;
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

		for (int i=0; i<5; i++)
		{
			int Check=arSpecMask[i].Check();		
			if ((Check==30)||(Check==40))
			{
				bHint=false;				
				WObj=ShowActiveObjects(WObj,6,0);
				switch (i)
				{
				case 0:
					WindowAbilityInfo(1,Unit->iLife);
					break;
				case 1:
					WindowAbilityInfo(12,Unit->GetMorale()+Unit->iMorale/2+Morale);
					break;
				case 4:
					if (Unit->pHero)
					{WindowAbilityInfo(119,Unit->pHero->iLevel);}
					else
					{WindowAbilityInfo(119,Unit->iLevel);};
					break;
				};
		//		WindowAbilityInfo(i,arParam[i]);
			};
		};

		if (!Unit->pHero)
		{
			int Check=mskRank.Check();
			if ((Check==30)||(Check==40))
			{
				bHint=false;				
				WObj=ShowActiveObjects(WObj,6,0);
				WindowInfo((pFirstPhrase+495)->str,(pFirstTutText+86)->msg,bmpStuff[66]);
			};
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно информации о типе юнита ******************
//***************************************************
void WindowUnitTypeInfo(int Unit)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_UnitTypeInfo->w/2;
	int y=384-bmpGUI_UnitTypeInfo->h/2;	
	const int w=59;
	const int h=31;
	const int dx=60;
	const int dy=55;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);	
	BITMAP *bmpBack=create_bitmap(1024,768);	
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_UnitTypeInfo->w,bmpGUI_UnitTypeInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_UnitTypeInfo,bmpWindow,0,0,0,0,bmpGUI_UnitTypeInfo->w,bmpGUI_UnitTypeInfo->h);
	set_trans_blender(140,140,140,140);
	draw_trans_sprite(bmpWindow,(pFirstUnitType+Unit)->Shadow,255-(pFirstUnitType+Unit)->Pic->w/2,105-(pFirstUnitType+Unit)->Pic->h/2);
	draw_sprite(bmpWindow,(pFirstUnitType+Unit)->Pic,255-(pFirstUnitType+Unit)->Pic->w/2,105-(pFirstUnitType+Unit)->Pic->h/2);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+284,y+366,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	char Log[20];

	
	textprintf_centre_ex(bmpWindow,Font1,325,17,COLOR_1,-1,"%s (%s)",(pFirstUnitType+Unit)->sName,(pFirstPhrase+303+(pFirstUnitType+Unit)->iKarma)->str);	
	textprintf_centre_ex(bmpWindow,Font1,254,185,COLOR_1,-1,"%s %d",(pFirstPhrase+10)->str,(pFirstUnitType+Unit)->iLevel);
		

	UI_Mask1 arParamMask[13];
	int arParam[13];
	arParamMask[1].Init(x+18,y+43,58,30,1);arParam[1]=(pFirstUnitType+Unit)->iLife;
	arParamMask[2].Init(x+18,y+43+h,58,30,1);arParam[2]=(pFirstUnitType+Unit)->iAttack;
	arParamMask[3].Init(x+18+w,y+43+h,58,30,1);arParam[3]=(pFirstUnitType+Unit)->iCounterAttack;
	arParamMask[4].Init(x+18,y+43+h*2,58,30,1);arParam[4]=(pFirstUnitType+Unit)->iDefence;
	arParamMask[5].Init(x+18+w,y+43+h*2,58,30,1);arParam[5]=(pFirstUnitType+Unit)->iRangedDefence;
	arParamMask[6].Init(x+18+w*2,y+43+h*2,58,30,1);arParam[6]=(pFirstUnitType+Unit)->iResist;
	arParamMask[7].Init(x+18+w*2,y+43+h,58,30,1);arParam[7]=(pFirstUnitType+Unit)->iSpeed;
	arParamMask[8].Init(x+18,y+43+h*3,58,30,1);arParam[8]=(pFirstUnitType+Unit)->iRangedAttack;
	arParamMask[9].Init(x+18+w,y+43+h*3,58,30,1);arParam[9]=(pFirstUnitType+Unit)->iShootingRange;
	arParamMask[10].Init(x+18+w*2,y+43+h*3,58,30,1);arParam[10]=(pFirstUnitType+Unit)->iAmmo;
	arParamMask[11].Init(x+18+w,y+43,58,30,1);arParam[11]=(pFirstUnitType+Unit)->iStamina;
	arParamMask[12].Init(x+18+w*2,y+43,58,30,1);arParam[12]=(pFirstUnitType+Unit)->iMorale;

	UI_Mask1 arSpecMask[4];	
	arSpecMask[0].Init(x+33,y+207,88,23,1);
	arSpecMask[0].iHint=307;
	arSpecMask[1].Init(x+126,y+207,88,23,1);
	arSpecMask[1].iHint=308;
	arSpecMask[2].Init(x+22,y+175,80,25,1);
	arSpecMask[2].iHint=90;
	arSpecMask[3].Init(x+104,y+175,80,25,1);
	arSpecMask[3].iHint=91;	

	UI_Mask1 mskRank;
	mskRank.Init(x+197,y+175,120,20,1);
	mskRank.iHint=495;

	textprintf_centre_ex(bmpWindow,Font1,dx,dy,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iLife);
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iStamina);
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iMorale);
	textprintf_centre_ex(bmpWindow,Font1,dx,dy+h,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iAttack);
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy+h,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iCounterAttack);
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy+h,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iSpeed);
	textprintf_centre_ex(bmpWindow,Font1,dx,dy+h*2,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iDefence);
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy+h*2,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iRangedDefence);
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy+h*2,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iResist);
	textprintf_centre_ex(bmpWindow,Font1,dx,dy+h*3,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iRangedAttack);
	textprintf_centre_ex(bmpWindow,Font1,dx+w,dy+h*3,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iShootingRange);
	textprintf_centre_ex(bmpWindow,Font1,dx+w*2,dy+h*3,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iAmmo);

	textprintf_centre_ex(bmpWindow,Font1,64,182,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iGoldPayment);
	textprintf_centre_ex(bmpWindow,Font1,152,182,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iGemPayment);
	textprintf_centre_ex(bmpWindow,Font1,82,213,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iGoldPrice);
	textprintf_centre_ex(bmpWindow,Font1,177,213,COLOR_2,-1,"%d",(pFirstUnitType+Unit)->iGemPrice);
	UI_Mask1 mskRes[MAX_UNIT_RES];
	for (int i=0; i<MAX_UNIT_RES; i++)
	{
		if ((pFirstUnitType+Unit)->arRes[i]>0)
		{
			draw_sprite(bmpWindow,(pFirstResource+(pFirstUnitType+Unit)->arRes[i])->Pic,224+i*26,207);
			mskRes[i].Init(x+224+i*26,y+207,20,20,1);
		};
	};

	

 	int iAbil=0, MaxAbilMask=0;
	int arAbil[2][50];
	for (int i=0; i<50; i++)
	{arAbil[0][i]=0;};
	for (int i=13; i<3000; i++)
	{
		int Skill=0;
		for (int AB=0; AB<MAX_UNIT_ABIL; AB++)
		{
			for (int UP=0; (UP<MAX_UPGRADE_SIZE)&&(UnitUpg(UnitType(Unit)->arAbility[AB])->arUpg[UP][0]); UP++)
			{
				if (UnitUpg(UnitType(Unit)->arAbility[AB])->arUpg[UP][0]==i)
				{
					Skill+=UnitUpg(UnitType(Unit)->arAbility[AB])->arUpg[UP][1];
				};
			};
		};		
		if ((iAbil<10)&&(Skill>0))
		{			
			for (int j=1; j<=iMaxAbilityNum; j++)
			{
				if ((pFirstAbilityNum+j)->Num==i)
				{
					arAbil[0][iAbil]=i;
					arAbil[1][iAbil]=Skill;
					if (iAbil<10)
					{
						draw_sprite(bmpWindow,(pFirstAbilityNum+j)->Pic,17+iAbil%5*60, 240+(iAbil-iAbil%5)/5*60);
						if ((pFirstAbilityNum+j)->bNumeric)
						{
							ShowIconNumber(bmpWindow, 17+iAbil%5*60, 240+(iAbil-iAbil%5)/5*60,Skill);
						//	textprintf_centre(bmpWindow,Font1,66+iAbil%5*60,287+(iAbil-iAbil%5)/5*60,COLOR_1,"%d",Skill);
						//	textprintf_centre(bmpWindow,Font1,65+iAbil%5*60,286+(iAbil-iAbil%5)/5*60,COLOR_2,"%d",Skill);
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
	UI_Mask arAbilMask[10];
	for (int i=0; i<MaxAbilMask; i++)
	{
		WObj=arAbilMask[i].Init(x+11+i%5*60, y+234+(i-i%5)/5*60,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
	};		

	UI_TextBox TextBox;
	WObj=TextBox.Init(x+342,y+74,300,250,2,COLOR_1,Font2,WObj,x+481,y+44,x+481,y+334);
	TextBox.Activate((pFirstUnitType+Unit)->sText);	

	while (!buttExit.Check())
	{		
		TextBox.Check();
		bHint=false;
		for (int i=0; i<MaxAbilMask; i++)
		{
			int Check=arAbilMask[i].Check();
			if (Check>1)
			{
				bHint=true;
				MakeAbilityHint(arAbil[0][i]);
			};
			if ((Check==30)||(Check==40))
			{
				bHint=false;				
				WObj=ShowActiveObjects(WObj,6,0);
				WindowAbilityInfo(arAbil[0][i],arAbil[1][i]);
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

		for (int i=0; i<4; i++)
		{
			int Check=arSpecMask[i].Check();		
			if ((Check==30)||(Check==40))
			{
				bHint=false;				
			};
		};
		for (int i=0; i<MAX_UNIT_RES; i++)
		{
			if ((pFirstUnitType+Unit)->arRes[i]>0)
			{
				int Check=mskRes[i].Check();
				if (Check>1)
				{
					if ((Check==30)||(Check==40))
					{
						bHint=false;
						WObj=ShowActiveObjects(WObj,6,0);
						WindowResourceInfo((pFirstUnitType+Unit)->arRes[i]);
					}
					else
					{
						bHint=true;
						MakeStringHint((pFirstResource+(pFirstUnitType+Unit)->arRes[i])->sName);
					};
				};
			};
		};
		int Check=mskRank.Check();
		if ((Check==30)||(Check==40))
		{
			bHint=false;				
			WObj=ShowActiveObjects(WObj,6,0);
			WindowInfo((pFirstPhrase+495)->str,(pFirstTutText+86)->msg,bmpStuff[66]);
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно информации о ресурсе *********************
//***************************************************
void WindowResourceInfo (int Res)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_ResourceInfo->w/2;
	int y=384-bmpGUI_ResourceInfo->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_ResourceInfo->w,bmpGUI_ResourceInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_ResourceInfo,bmpWindow,0,0,0,0,bmpGUI_ResourceInfo->w,bmpGUI_ResourceInfo->h);

	draw_sprite(bmpWindow,(pFirstResource+Res)->BigPic,21,20);
	draw_sprite(bmpWindow,(pFirstResource+Res)->BigPic,328,20);
	UI_Button1 * buttExit=new UI_Button1();
	WObj=buttExit->Init(x+168,y+542,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit->Activate(1);
	buttExit->ipHotkey=KEY_ESC;
	UI_Mask1 mskRes[3];
	textprintf_centre_ex(bmpWindow,Font1,200,22,COLOR_1,-1,"%s",(pFirstResource+Res)->sName);
	if (CurrentPlayer==0)
	{
		textprintf_centre_ex(bmpWindow,Font1,180,56,COLOR_2,-1,"%d",(pFirstResource+Res)->iGold);
		textprintf_centre_ex(bmpWindow,Font1,274,56,COLOR_2,-1,"%d",(pFirstResource+Res)->iGem);
	}
	else
	{
	textprintf_centre_ex(bmpWindow,Font1,105,56,COLOR_2,-1,"%d",arPlayer[CurrentPlayer].GetResource(Res));
//	int Price=(pFirstResource+Res)->iGold*CurGame.ResPrices[Res]/100;
//	Price-=Price*arPlayer[CurrentPlayer].GetBldParam1(17)/100;
	textprintf_centre_ex(bmpWindow,Font1,180,56,COLOR_2,-1,"%d",arPlayer[CurrentPlayer].GetResGoldPrice(Res));
//	Price=(pFirstResource+Res)->iGem*CurGame.ResPrices[Res]/100;
//	Price-=Price*arPlayer[CurrentPlayer].GetBldParam1(17)/100;
	textprintf_centre_ex(bmpWindow,Font1,274,56,COLOR_2,-1,"%d",arPlayer[CurrentPlayer].GetResGemPrice(Res));
	};
	mskRes[0].Init(x+84,y+49,42,23,1);
	mskRes[1].Init(x+133,y+49,87,23,1);
	mskRes[2].Init(x+228,y+49,42,23,1);
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+31,y+112,342,393,2,COLOR_1,Font2,WObj,185+x,86+y,185+x,513+y);
	char TXT[2000];
	sprintf(TXT,(pFirstMessage+24)->msg,(pFirstResource+Res)->sText);
	TextBox.Activate(TXT);

	while (!buttExit->Check())
	{
		TextBox.Check();
		for (int i=0; i<3; i++)
		{
			int Check=mskRes[i].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(i+309);
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
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	delete buttExit;
	set_mouse_sprite(OldMouse);
	iAntisleep=iAntisleepDelay;
};


//***************************************************
//*** Окно информации о заклинании ******************
//***************************************************
void WindowSpellInfo (int Spell)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_SpellInfo->w/2;
	int y=384-bmpGUI_SpellInfo->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_SpellInfo->w,bmpGUI_SpellInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_SpellInfo,bmpWindow,0,0,0,0,bmpGUI_SpellInfo->w,bmpGUI_SpellInfo->h);

	draw_sprite(bmpWindow,(pFirstSpell+Spell)->Pic,21,20);
	draw_sprite(bmpWindow,(pFirstSpell+Spell)->Pic,328,20);
	UI_Button1 * buttExit=new UI_Button1();
	WObj=buttExit->Init(x+168,y+342,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit->Activate(1);
	buttExit->ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,200,21,COLOR_1,-1,"%s",(pFirstSpell+Spell)->sName);
	textprintf_centre_ex(bmpWindow,Font1,148,56,COLOR_2,-1,"%d",(pFirstSpell+Spell)->iCost);
	textprintf_centre_ex(bmpWindow,Font1,260,56,COLOR_2,-1,"%d",(pFirstSpell+Spell)->iStamCost);
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+31,y+112,342,193,2,COLOR_1,Font2,WObj,185+x,86+y,185+x,313+y);
	TextBox.Activate((pFirstSpell+Spell)->sText);

	while (!buttExit->Check())
	{
		TextBox.Check();
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	delete buttExit;
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно информации об умении героя ***************
//***************************************************
void WindowSkillInfo (int Skill, int Level)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_SkillInfo->w/2;
	int y=384-bmpGUI_SkillInfo->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_SkillInfo->w,bmpGUI_SkillInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_SkillInfo,bmpWindow,0,0,0,0,bmpGUI_SkillInfo->w,bmpGUI_SkillInfo->h);

	draw_sprite(bmpWindow,bmpHeroSkill[(Skill-1)*5+Level],22,23);
	draw_sprite(bmpWindow,bmpHeroSkill[(Skill-1)*5+Level],397,23);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+205,y+352,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,240,29,COLOR_1,-1,"%s",arSkills[Skill].sName);
	textprintf_centre_ex(bmpWindow,Font1,240,59,COLOR_1,-1,"%s: %d",(pFirstPhrase+5)->str,Level);
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+35,y+114,405,205,2,COLOR_1,Font2,WObj,224+x,86+y,224+x,324+y);
	TextBox.Activate(arSkillText[Skill].msg);

	while (!buttExit.Check())
	{
		TextBox.Check();
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно информации о свойстве юнита **************
//***************************************************
void WindowAbilityInfo (int Ability, int Power)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_AbilityInfo->w/2;
	int y=384-bmpGUI_AbilityInfo->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_AbilityInfo->w,bmpGUI_AbilityInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_AbilityInfo,bmpWindow,0,0,0,0,bmpGUI_AbilityInfo->w,bmpGUI_AbilityInfo->h);
	stAbilityNum *pAbilNum;
	for (int i=0; i<=iMaxAbilityNum; i++)
	{
		if ((pFirstAbilityNum+i)->Num==Ability)
		{pAbilNum=(pFirstAbilityNum+i);};
	};
	draw_sprite(bmpWindow,pAbilNum->Pic,21,20);
	draw_sprite(bmpWindow,pAbilNum->Pic,328,20);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+168,y+342,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);	
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,200,36,COLOR_1,-1,"%s",pAbilNum->sName);	
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+31,y+112,342,193,2,COLOR_1,Font2,WObj,185+x,86+y,185+x,313+y);
	char Mess[1000];
	if ((pAbilNum->bNumeric)||(pAbilNum->bPercent))
	{sprintf(Mess,pAbilNum->sText,Power);}
	else
	{sprintf(Mess,pAbilNum->sText);};
	TextBox.Activate(Mess);
	while (!buttExit.Check())
	{
		TextBox.Check();
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно информации о награде юнита ***************
//***************************************************
void WindowMedalInfo (int Medal)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_AbilityInfo->w/2;
	int y=384-bmpGUI_AbilityInfo->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_AbilityInfo->w,bmpGUI_AbilityInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_AbilityInfo,bmpWindow,0,0,0,0,bmpGUI_AbilityInfo->w,bmpGUI_AbilityInfo->h);
	draw_sprite(bmpWindow,(pFirstMedal+Medal)->Pic,21,20);
	draw_sprite(bmpWindow,(pFirstMedal+Medal)->Pic,328,20);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+168,y+342,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);	
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,200,36,COLOR_1,-1,"%s",(pFirstMedal+Medal)->sName);
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+31,y+112,342,193,2,COLOR_1,Font2,WObj,185+x,86+y,185+x,313+y);	
	TextBox.Activate((pFirstMedal+Medal)->sText);
	while (!buttExit.Check())
	{
		TextBox.Check();
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно информации о страже **********************
//***************************************************
void WindowDefenderInfo (int Defender, int GoldPayment, int GemPayment)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_DefenderInfo->w/2;
	int y=384-bmpGUI_DefenderInfo->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_DefenderInfo->w,bmpGUI_DefenderInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_DefenderInfo,bmpWindow,0,0,0,0,bmpGUI_DefenderInfo->w,bmpGUI_DefenderInfo->h);
	draw_sprite(bmpWindow,(pFirstDefender+Defender)->Pic,21,20);
	draw_sprite(bmpWindow,(pFirstDefender+Defender)->Pic,328,20);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+168,y+372,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);	
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,200,22,COLOR_1,-1,"%s",(pFirstDefender+Defender)->sName);
	UI_Mask1 mskRes[8];
	if ((GoldPayment+GemPayment)==0)
	{
		GoldPayment=(pFirstDefender+Defender)->iGoldPayment;
		GemPayment=(pFirstDefender+Defender)->iGemPayment;
	}
	textprintf_centre_ex(bmpWindow,Font1,157,54,COLOR_2,-1,"%d",GoldPayment);
	mskRes[4].Init(x+110,y+47,89,23,1);
	textprintf_centre_ex(bmpWindow,Font1,253,54,COLOR_2,-1,"%d",GemPayment);
	mskRes[5].Init(x+206,y+47,89,23,1);
	textprintf_centre_ex(bmpWindow,Font1,74,89,COLOR_2,-1,"%d",(pFirstDefender+Defender)->iGoldCost);
	mskRes[6].Init(x+27,y+83,89,23,1);
	textprintf_centre_ex(bmpWindow,Font1,169,89,COLOR_2,-1,"%d",(pFirstDefender+Defender)->iGemCost);
	mskRes[7].Init(x+121,y+83,89,23,1);
	for (int i=0; i<MAX_DEFENDER_RES; i++)
	{
		if ((pFirstDefender+Defender)->arRes[i]>0)
		{
			draw_sprite(bmpWindow,(pFirstResource+(pFirstDefender+Defender)->arRes[i])->Pic,220+i*26,83);
			mskRes[i].Init(x+220+i*26,y+83,20,20,1);
		};
	};
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+34,y+140,340,195,2,COLOR_1,Font2,WObj,185+x,115+y,185+x,342+y);	
	TextBox.Activate((pFirstDefender+Defender)->sText);
	int Check;
	while (!buttExit.Check())
	{
		for (int i=0; i<4; i++)
		{
			if ((pFirstDefender+Defender)->arRes[i]>0)
			{
				Check=mskRes[i].Check();
				if (Check>1)
				{
					if ((Check==30)||(Check==40))
					{
						bHint=false;
						WObj=ShowActiveObjects(WObj,6,0);
						WindowResourceInfo((pFirstDefender+Defender)->arRes[i]);
					}
					else
					{
						bHint=true;
						MakeStringHint((pFirstResource+(pFirstDefender+Defender)->arRes[i])->sName);
					};
				};
			};
		};
		Check=mskRes[4].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(90);
		};
		Check=mskRes[5].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(91);
		};
		Check=mskRes[6].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(307);
		};
		Check=mskRes[7].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(308);
		};
		TextBox.Check();
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно информации о внешней постройке ***********
//***************************************************
void WindowOuterBldInfo (int Bld)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_OuterBuildInfo->w/2;
	int y=384-bmpGUI_OuterBuildInfo->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_OuterBuildInfo->w,bmpGUI_OuterBuildInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_OuterBuildInfo,bmpWindow,0,0,0,0,bmpGUI_OuterBuildInfo->w,bmpGUI_OuterBuildInfo->h);	
	draw_sprite(bmpWindow,(pFirstOuterBld+Bld)->Pic,21,20);
	draw_sprite(bmpWindow,(pFirstOuterBld+Bld)->Pic,328,20);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+168,y+372,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);	
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,200,22,COLOR_1,-1,"%s",(pFirstOuterBld+Bld)->sName);
	UI_Mask1 mskRes[8];
	textprintf_centre_ex(bmpWindow,Font1,74,89,COLOR_2,-1,"%d",(pFirstOuterBld+Bld)->iGoldCost);
	mskRes[6].Init(x+27,y+83,89,23,1);
	textprintf_centre_ex(bmpWindow,Font1,169,89,COLOR_2,-1,"%d",(pFirstOuterBld+Bld)->iGemCost);
	mskRes[7].Init(x+121,y+83,89,23,1);
	for (int i=0; i<MAX_BUILDING_RES; i++)
	{
		if ((pFirstOuterBld+Bld)->arRes[i]>0)
		{
			draw_sprite(bmpWindow,(pFirstResource+(pFirstOuterBld+Bld)->arRes[i])->Pic,220+i*26,83);
			mskRes[i].Init(x+220+i*26,y+83,20,20,1);
		};
	};
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+34,y+140,340,195,2,COLOR_1,Font2,WObj,185+x,115+y,185+x,342+y);	
	TextBox.Activate((pFirstOuterBld+Bld)->sText);
	int Check;
	while (!buttExit.Check())
	{
		for (int i=0; i<6; i++)
		{
			if ((pFirstOuterBld+Bld)->arRes[i]>0)
			{
				Check=mskRes[i].Check();
				if (Check>1)
				{
					if ((Check==30)||(Check==40))
					{
						bHint=false;
						WObj=ShowActiveObjects(WObj,6,0);
						WindowResourceInfo((pFirstOuterBld+Bld)->arRes[i]);
					}
					else
					{
						bHint=true;
						MakeStringHint((pFirstResource+(pFirstOuterBld+Bld)->arRes[i])->sName);
					};
				};
			};
		};
		Check=mskRes[6].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(307);
		};
		Check=mskRes[7].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(308);
		};
		TextBox.Check();
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно информации о ритуале *********************
//***************************************************
void WindowRitualInfo (int Rit)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_OuterBuildInfo->w/2;
	int y=384-bmpGUI_OuterBuildInfo->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_OuterBuildInfo->w,bmpGUI_OuterBuildInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_OuterBuildInfo,bmpWindow,0,0,0,0,bmpGUI_OuterBuildInfo->w,bmpGUI_OuterBuildInfo->h);
	draw_sprite(bmpWindow,bmpStuff[35],298,81);
	draw_sprite(bmpWindow,(pFirstRitual+Rit)->Pic,21,20);
	draw_sprite(bmpWindow,(pFirstRitual+Rit)->Pic,328,20);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+168,y+372,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);	
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,200,22,COLOR_1,-1,"%s",(pFirstRitual+Rit)->sName);
	UI_Mask1 mskRes[8];
	textprintf_centre_ex(bmpWindow,Font1,74,89,COLOR_2,-1,"%d",(pFirstRitual+Rit)->iGoldCost);
	mskRes[6].Init(x+27,y+83,89,23,1);
	textprintf_centre_ex(bmpWindow,Font1,169,89,COLOR_2,-1,"%d",(pFirstRitual+Rit)->iGemCost);
	mskRes[7].Init(x+121,y+83,89,23,1);
	mskRes[5].Init(x+303,y+83,75,23,1);
	textprintf_centre_ex(bmpWindow,Font1,343,89,COLOR_2,-1,"%d",(pFirstRitual+Rit)->iCooldown);

	for (int i=0; i<MAX_RITUAL_RES; i++)
	{
		if ((pFirstRitual+Rit)->arRes[i]>0)
		{
			draw_sprite(bmpWindow,(pFirstResource+(pFirstRitual+Rit)->arRes[i])->Pic,220+i*26,83);
			mskRes[i].Init(x+220+i*26,y+83,20,20,1);
		};
	};
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+34,y+140,340,195,2,COLOR_1,Font2,WObj,185+x,115+y,185+x,342+y);	
	TextBox.Activate((pFirstRitual+Rit)->sText);
	int Check;
	while (!buttExit.Check())
	{
		for (int i=0; i<MAX_RITUAL_RES; i++)
		{
			if ((pFirstRitual+Rit)->arRes[i]>0)
			{
				Check=mskRes[i].Check();
				if (Check>1)
				{
					if ((Check==30)||(Check==40))
					{
						bHint=false;
						WObj=ShowActiveObjects(WObj,6,0);
						WindowResourceInfo((pFirstRitual+Rit)->arRes[i]);
					}
					else
					{
						bHint=true;
						MakeStringHint((pFirstResource+(pFirstRitual+Rit)->arRes[i])->sName);
					};
				};
			};
		};
		Check=mskRes[5].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(394);
		};
		Check=mskRes[6].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(307);
		};
		Check=mskRes[7].Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(308);
		};
		TextBox.Check();
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Универсальное окно с текстом ******************
//***************************************************
void WindowInfo (char * sName, char * sText, BITMAP *Pic)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_AbilityInfo->w/2;
	int y=384-bmpGUI_AbilityInfo->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_AbilityInfo->w,bmpGUI_AbilityInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_AbilityInfo,bmpWindow,0,0,0,0,bmpGUI_AbilityInfo->w,bmpGUI_AbilityInfo->h);	
	draw_sprite(bmpWindow,Pic,21,20);
	draw_sprite(bmpWindow,Pic,328,20);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+168,y+342,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);	
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,200,36,COLOR_1,-1,sName);	
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+31,y+112,342,193,2,COLOR_1,Font2,WObj,185+x,86+y,185+x,313+y);	
	TextBox.Activate(sText);
	while (!buttExit.Check())
	{
		TextBox.Check();
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно информации о расе ************************
//***************************************************
void WindowRaceInfo (int Race)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_AbilityInfo->w/2;
	int y=384-bmpGUI_AbilityInfo->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_AbilityInfo->w,bmpGUI_AbilityInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_AbilityInfo,bmpWindow,0,0,0,0,bmpGUI_AbilityInfo->w,bmpGUI_AbilityInfo->h);	
	draw_sprite(bmpWindow,(pFirstRace+Race)->Pic,21,20);
	draw_sprite(bmpWindow,(pFirstRace+Race)->Pic,328,20);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+168,y+342,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);	
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,200,36,COLOR_1,-1,"%s",(pFirstRace+Race)->sName);	
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+31,y+112,342,193,2,COLOR_1,Font2,WObj,185+x,86+y,185+x,313+y);	
	TextBox.Activate((pFirstRace+Race)->sText);
	while (!buttExit.Check())
	{
		TextBox.Check();
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно информации об эффекте ********************
//***************************************************
void WindowEffectInfo (clUnitUpgrade *Ability, clSpell *Spell, int Time, int Power)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_AbilityInfo->w/2;
	int y=384-bmpGUI_AbilityInfo->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_AbilityInfo->w,bmpGUI_AbilityInfo->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_AbilityInfo,bmpWindow,0,0,0,0,bmpGUI_AbilityInfo->w,bmpGUI_AbilityInfo->h);
	int Num=0;
	if (Ability)
	{
		draw_sprite(bmpWindow,Ability->Pic,21,20);
		draw_sprite(bmpWindow,Ability->Pic,328,20);
		for (int i=1; i<=iMaxEffect; i++)
		{
			if ((pFirstEffect+i)->ipAbility==Ability->arUpg[0][0])
			{Num=i;};
		};
	}
	else
	{
		draw_sprite(bmpWindow,Spell->Pic,21,20);
		draw_sprite(bmpWindow,Spell->Pic,328,20);		
		int iSpell=0;
		for (int i=0; i<=iMaxSpell; i++)
		{
			if ((pFirstSpell+i)==Spell)
			{iSpell=i;};
		};
		for (int i=1; i<=iMaxEffect; i++)
		{
			if ((pFirstEffect+i)->ipSpell==iSpell)
			{Num=i;};
		};
	};
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+168,y+342,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);	
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,200,36,COLOR_1,-1,"%s",(pFirstEffect+Num)->sName);	
	if (Time>0)
	{textprintf_centre_ex(bmpWindow,Font1,200,112,COLOR_1,-1,"%s: %d",(pFirstPhrase+86)->str,Time);}
	else
	{textprintf_centre_ex(bmpWindow,Font1,200,112,COLOR_1,-1,"%s: %s",(pFirstPhrase+86)->str,(pFirstPhrase+87)->str);};
	UI_TextBox TextBox;
	WObj=TextBox.Init(x+31,y+132,342,173,2,COLOR_1,Font2,WObj,185+x,86+y,185+x,313+y);	
	char Mess[500];
	if ((pFirstEffect+Num)->bNumeric)
	{
		if (Power!=0)
		{sprintf(Mess,(pFirstEffect+Num)->sText,abs(Power));}
		else
		{sprintf(Mess,(pFirstEffect+Num)->sText,Ability->arUpg[0][1]);};
	}
	else
	{sprintf(Mess,(pFirstEffect+Num)->sText);};
	TextBox.Activate(Mess);
	while (!buttExit.Check())
	{
		TextBox.Check();
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};


//***************************************************
//*** Окно информации о предмете ********************
//***************************************************
void WindowItemInfo (int Item, int Dur)
{
	iAntisleep=iAntisleepDelay;
	int Count=0;
	clItem * It=(pFirstItem+Item);
	for (int i=1; i<=iMaxItemSet; i++)
	{
		for (int j=0; j<MAX_SET_ITEMS; j++)
		{
			if ((pFirstItemSet+i)->arItems[j]==Item)
			{	
				Count++;
			};
		};
	};
	for (int i=0; i<MAX_ITEM_EFFECT; i++)
	{
		if (It->arEffect[i][0]>0)
		{			
			Count++;
		};
		switch (It->arEffect[i][0])
		{
		case 83:		
		case 86:			
		case 87:
		case 94:
			Count++;
			break;
		};
	};
	int H=190+bmpGUI_ItemEffect->h*Count;
	int x=512-bmpGUI_ItemInfo->w/2;
	int y=384-H/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_ItemInfo->w,H);
	BITMAP *bmpEff=create_bitmap(363,61);
	rectfill(bmpEff,0,0,363,61,COLOR_1);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_ItemInfo,bmpWindow,0,0,0,0,bmpGUI_ItemInfo->w,120+bmpGUI_ItemEffect->h*Count);
	blit(bmpGUI_ItemInfo,bmpWindow,0,bmpGUI_ItemInfo->h-75,0,H-75,bmpGUI_ItemInfo->w,75);
	

	draw_sprite(bmpWindow,It->Pic,20,23);	
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+193,H-58+y,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
//	UI_Button1 buttScrollUp;
//	WObj=buttScrollUp.Init(x+124,y+408,1,3,bmpButton[76],bmpButton[77],bmpButton[78],bmpButton[79],WObj);
//	buttScrollUp.Activate(0);
//	UI_Button1 buttScrollDown;
//	WObj=buttScrollDown.Init(x+214,y+408,1,3,bmpButton[80],bmpButton[81],bmpButton[82],bmpButton[83],WObj);
//	buttScrollDown.Activate(0);
	UI_Mask arMask[9];


	int Color=COLOR_2;
	switch (It->iRarity)
	{
		case 5: Color=makecol(180,70,250);
				break;
		case 4: Color=makecol(130,130,255);
				break;
		case 3: Color=makecol(50,220,220);
				break;
		case 2: Color=makecol(50,250,50);
				break;
		case 1: Color=makecol(220,220,50);
				break;
		case 7: Color=makecol(240,240,240);
				break;
		case 6: Color=makecol(250,50,50);
				break;
	};	
	textprintf_centre_ex(bmpWindow,Font1,211,22,COLOR_1,-1,"%s",It->sName);
	textprintf_centre_ex(bmpWindow,Font1,210,21,Color,-1,"%s",It->sName);
	textprintf_centre_ex(bmpWindow,Font1,210,50,COLOR_2,-1,"%s",(pFirstItemType+It->ipClass)->str);
	Color=It->ipSlot;
	if (Color==100)
	{Color=12;};
	textprintf_centre_ex(bmpWindow,Font1,210,74,COLOR_2,-1,"%s",ItemSlotName[Color].str);
	textprintf_centre_ex(bmpWindow,Font1,210,98,COLOR_2,-1,"%s: %d/%d",(pFirstPhrase+16)->str,Dur,It->iDurability);

	UI_Mask1 mskRes[8];
	textprintf_centre_ex(bmpWindow,Font1,390,23,COLOR_2,-1,"%d",It->iGoldCost);
	textprintf_centre_ex(bmpWindow,Font1,390,48,COLOR_2,-1,"%d",It->iGemCost);
	for (int i=0; i<MAX_ITEM_RES; i++)
	{
		if (It->arRes[i]>0)
		{
			draw_sprite(bmpWindow,(pFirstResource+It->arRes[i])->Pic,352+26*(i%3),69+25*((i-i%3)/3));
			mskRes[i].Init(x+352+26*(i%3),y+69+25*((i-i%3)/3),20,20,1);
		};
	};
	mskRes[6].Init(x+343,y+18,88,23,1);
	mskRes[7].Init(x+343,y+41,88,23,1);
	int arAbil[MAX_ITEM_EFFECT+1];
	Count=0;	
	for (int i=1; i<=iMaxItemSet; i++)
	{
		for (int j=0; j<MAX_SET_ITEMS; j++)
		{
			if ((pFirstItemSet+i)->arItems[j]==Item)
			{				
				arAbil[Count]=-i;
				Count++;
			};
		};
	};
	for (int i=0; i<MAX_ITEM_EFFECT; i++)
	{
		if (It->arEffect[i][0]>0)
		{
			arAbil[Count]=i;
			Count++;
		};
		switch (It->arEffect[i][0])
		{
		case 83:
			arAbil[Count]=-1000;
			Count++;
			break;
		case 86:
			arAbil[Count]=-1001;
			Count++;
			break;
		case 87:
			arAbil[Count]=-1002;
			Count++;
			break;
		case 94:
			arAbil[Count]=-1003;
			Count++;
			break;
		};
	};
//	int iMsk=Count;
//	if (iMsk>5)
//	{iMsk=5;};
	for (int i=0; i<Count; i++)
	{WObj=arMask[i].Init(x+11,y+122+i*bmpGUI_ItemEffect->h,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);};
//	if (Count>5)
//	{buttScrollDown.Activate(1);};
//	int Shift=0;
//	bool bContinue=true;
//	while (bContinue)
//	{
//		blit(bmpGUI_ItemInfo,bmpWindow,0,85,0,85,bmpGUI_ItemInfo->w,bmpGUI_ItemInfo->h-85);
	set_trans_blender(140,140,140,140);
		for (int ABL=0; ABL<Count; ABL++)
		{
			bool bNum, bPercent;
			int iEff;
			char sName[60];
			draw_sprite(bmpWindow,bmpGUI_ItemEffect,11,121+ABL*bmpGUI_ItemEffect->h);
			draw_trans_sprite(bmpWindow,bmpEff,11+65,121+3+ABL*bmpGUI_ItemEffect->h);
			if (arAbil[ABL]<0)
			{
				switch (arAbil[ABL])
				{
				case -1000:
					draw_sprite(bmpWindow,(pFirstSpell+It->arEffect[0][1])->Pic,17,128+ABL*bmpGUI_ItemEffect->h);
					textprintf_centre_ex(bmpWindow,Font1,250,150+ABL*bmpGUI_ItemEffect->h,COLOR_2,-1,"%s",(pFirstSpell+It->arEffect[0][1])->sName);
					break;
				case -1001:
					draw_sprite(bmpWindow,(pFirstDefender+It->arEffect[0][1])->Pic,17,128+ABL*bmpGUI_ItemEffect->h);
					textprintf_centre_ex(bmpWindow,Font1,250,150+ABL*bmpGUI_ItemEffect->h,COLOR_2,-1,"%s",(pFirstDefender+It->arEffect[0][1])->sName);
					break;
				case -1002:
					draw_sprite(bmpWindow,(pFirstOuterBld+It->arEffect[0][1])->Pic,17,128+ABL*bmpGUI_ItemEffect->h);
					textprintf_centre_ex(bmpWindow,Font1,250,150+ABL*bmpGUI_ItemEffect->h,COLOR_2,-1,"%s",(pFirstOuterBld+It->arEffect[0][1])->sName);
					break;
				case -1003:
					draw_sprite(bmpWindow,(pFirstRitual+It->arEffect[0][1])->Pic,17,128+ABL*bmpGUI_ItemEffect->h);
					textprintf_centre_ex(bmpWindow,Font1,250,150+ABL*bmpGUI_ItemEffect->h,COLOR_2,-1,"%s",(pFirstRitual+It->arEffect[0][1])->sName);
					break;
				default:
					draw_sprite(bmpWindow,bmpGUI_ItemSet,17,128+ABL*bmpGUI_ItemEffect->h);
					textprintf_centre_ex(bmpWindow,Font1,250,150+ABL*bmpGUI_ItemEffect->h,COLOR_2,-1,"%s",(pFirstItemSet+abs(arAbil[ABL]))->sName);
					break;
				};				
			}
			else
			{				
				for (int j=1; j<=iMaxAbilityNum; j++)
				{
					if ((pFirstAbilityNum+j)->Num==It->arEffect[arAbil[ABL]][0])
					{
						iEff=(pFirstAbilityNum+j)->Effect;
						bNum=(pFirstAbilityNum+j)->bNumeric;
						bPercent=(pFirstAbilityNum+j)->bPercent;
						sprintf (sName,"%s",(pFirstAbilityNum+j)->sName);
						draw_sprite(bmpWindow,(pFirstAbilityNum+j)->Pic,17,128+ABL*bmpGUI_ItemEffect->h);
					};
				};						
				switch (iEff)
				{
				case -1: Color=makecol(220,0,0);
					break;
				case 0: Color=makecol(220,220,0);
					break;
				case 1: 
					if (It->arEffect[arAbil[ABL]][1]<0)
					{Color=makecol(220,0,0);}
				else
					{Color=makecol(50,220,50);};
					break;
				};
				if (bNum)
				{
					if (It->arEffect[arAbil[ABL]][1]>0)
					{
						if (iEff==0)
						{sprintf(sName,"%s (%d)",sName,It->arEffect[arAbil[ABL]][1]);}
						else
						{sprintf(sName,"%s +%d",sName,It->arEffect[arAbil[ABL]][1]);};
					}
					else
					{sprintf(sName,"%s %d",sName,It->arEffect[arAbil[ABL]][1]);};
					if (bPercent)
					{sprintf(sName,"%s%s",sName,"%");};
				};
				if (It->arEffect[arAbil[ABL]][2]>0)
				{sprintf(sName,"%s (%s)",sName,(pFirstPhrase+17)->str);};
				textprintf_centre_ex(bmpWindow,Font1,250,150+ABL*bmpGUI_ItemEffect->h,Color,-1,sName);
			};
		};
		set_trans_blender(128,128,128,128);
		bool Cycle=true;
		int Check;
		while (Cycle)
		{
			for (int i=0; i<Count; i++)
			{
				Check=arMask[i].Check();
				if ((Check==30)||(Check==40))
				{
					bHint=false;
					WObj=ShowActiveObjects(WObj,6,0);
					if (arAbil[i]>=0)
					{
						WindowAbilityInfo(It->arEffect[arAbil[i]][0],It->arEffect[arAbil[i]][1]);
					}
					else
					{
						switch (arAbil[i])
						{
						case -1000:
							WindowSpellInfo(It->arEffect[0][1]);
							break;
						case -1001:
							WindowDefenderInfo(It->arEffect[0][1], 0, 0);
							break;
						case -1002:
							WindowOuterBldInfo(It->arEffect[0][1]);
							break;
						case -1003:
							WindowRitualInfo(It->arEffect[0][1]);
							break;
							default:
							//Set Info
							break;
						};
					};
				};
			};
			for (int i=0; i<MAX_ITEM_RES; i++)
			{
				if (It->arRes[i]>0)
				{
					Check=mskRes[i].Check();
					if (Check>1)
					{
						if ((Check==30)||(Check==40))
						{
							bHint=false;
							WObj=ShowActiveObjects(WObj,6,0);
							WindowResourceInfo(It->arRes[i]);
						}
						else
						{
							bHint=true;
							MakeStringHint((pFirstResource+It->arRes[i])->sName);
						};
					};
				};
			};
			Check=mskRes[6].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(307);
			};
			Check=mskRes[7].Check();
			if (Check>1)
			{
				bHint=true;
				MakeStringHint(308);
			};
		/*	if (buttScrollUp.Check())
			{
				Shift--;
				buttScrollDown.Activate(1);
				if (Shift==0)
				{buttScrollUp.Activate(0);}
				Cycle=false;
			};
			if (buttScrollDown.Check())
			{
				Shift++;
				buttScrollUp.Activate(1);
				if (Shift>=Count-5)
				{buttScrollDown.Activate(0);}
				Cycle=false;
			};*/
			if (buttExit.Check())
			{
				Cycle=false;
		//		bContinue=false;
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
//	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	destroy_bitmap(bmpEff);
	set_mouse_sprite(OldMouse);
	iAntisleep=iAntisleepDelay;
};
int MemoryPrice=0;

//Создание всплывающей подсказки по предметам
void MakeItemHint (int Item, int Dur, dcHero *Hero, int PriceText, int Gold, int Gem)
{
	if (((HintType!=1)||(HintMember!=Item)||(HintDur!=Dur)||(PriceText!=MemoryPrice))&&(Item>0))
	{
	HintType=1; HintMember=Item; HintDur=Dur; MemoryPrice=PriceText;
	clItem * It=(pFirstItem+Item);
	rectfill(bmpHint,0,0,MAX_HINT_W,MAX_HINT_H,COLOR_0);
	if (It->ipClass<=10)
	{HintH=90;}
	else
	{HintH=50;};
	HintH=136;	
	if (PriceText>0)
	{HintH+=54;};
	for (int i=0; i<MAX_ITEM_EFFECT; i++)
	{
		if (It->arEffect[i][0]>0)
		{HintH+=20;};
	};
	for (int i=1; i<=iMaxItemSet; i++)
	{
		for (int j=0; j<MAX_SET_ITEMS; j++)
		{
			if ((pFirstItemSet+i)->arItems[j]==Item)
			{
				HintH+=60+(pFirstItemSet+i)->iSize*20;
				for (int k=0; k<MAX_SET_BONUS; k++)
				{
					for (int k1=0; k1<MAX_SET_ABILITY; k1++)
					{
						if ((pFirstItemSet+i)->arBonus[k][k1][0]>0)
						{HintH+=20;};
					};
				};
			};
		};
	};
	if (HintH>MAX_HINT_H){HintH=MAX_HINT_H;};
	HintW=bmpGUI_ItemHint->w;
	if (PriceText>0)
	{
		blit(bmpGUI_ItemHint,bmpHint,0,0,0,0,HintW,HintH-74);
		blit(bmpGUI_ItemHint,bmpHint,0,bmpGUI_ItemHint->h-74,0,HintH-74,HintW,74);
		textprintf_centre_ex(bmpHint,Font1,150,HintH-52,COLOR_1,-1,"%s",(pFirstPhrase+PriceText)->str);
		if (((PriceText==13)||(PriceText==14))&&(arPlayer[CurrentPlayer].Gold<Gold))
		{textprintf_centre_ex(bmpHint,Font1,97,HintH-25,makecol(250,30,30),-1,"%d",Gold);}
		else
		{textprintf_centre_ex(bmpHint,Font1,97,HintH-25,COLOR_2,-1,"%d",Gold);};
		if (((PriceText==13)||(PriceText==14))&&(arPlayer[CurrentPlayer].Gem<Gem))
		{textprintf_centre_ex(bmpHint,Font1,207,HintH-25,makecol(250,30,30),-1,"%d",Gem);}
		else
		{textprintf_centre_ex(bmpHint,Font1,207,HintH-25,COLOR_2,-1,"%d",Gem);};
	}
	else
	{
		blit(bmpGUI_ItemHint,bmpHint,0,0,0,0,HintW,HintH-20);
		blit(bmpGUI_ItemHint,bmpHint,0,696,0,HintH-20,HintW,11);
		blit(bmpGUI_ItemHint,bmpHint,0,bmpGUI_ItemHint->h-9,0,HintH-9,HintW,9);
	};	
	int Color=COLOR_2;
	switch (It->iRarity)
	{
		case 5: Color=makecol(180,70,250);
				break;
		case 4: Color=makecol(130,130,255);
				break;
		case 3: Color=makecol(50,220,220);
				break;
		case 2: Color=makecol(50,250,50);
				break;
		case 1: Color=makecol(220,220,50);
				break;
		case 7: Color=makecol(240,240,240);
				break;
		case 6: Color=makecol(250,50,50);
	};
	textprintf_centre_ex(bmpHint,Font1,151,20,COLOR_1,-1,"%s",It->sName);
	textprintf_centre_ex(bmpHint,Font1,150,19,Color,-1,"%s",It->sName);
	if ((Hero)&&(It->ipClass<10)&&(!Hero->CanUse(Item)))
	{textprintf_centre_ex(bmpHint,Font1,150,50,makecol(250,30,30),-1,"%s",(pFirstItemType+It->ipClass)->str);}
	else
	{textprintf_centre_ex(bmpHint,Font1,150,50,COLOR_2,-1,"%s",(pFirstItemType+It->ipClass)->str);};
	if (It->ipClass<=10)
	{
	Color=It->ipSlot;
	if (Color==100)
	{Color=12;};
	textprintf_centre_ex(bmpHint,Font1,150,74,COLOR_2,-1,"%s",ItemSlotName[Color].str);
	textprintf_centre_ex(bmpHint,Font1,150,98,COLOR_2,-1,"%s: %d/%d",(pFirstPhrase+16)->str,Dur,It->iDurability);	
	
	int StrY=128;	
	bool bNum, bPercent;
	int iEff;
	char sName[60];
	for (int i=0; i<MAX_ITEM_EFFECT; i++)
	{
		if (It->arEffect[i][0]>0)
		{
			/*if (It->arEffect[i][0]<=iMaxAbility)
			{
				iEff=(pFirstAbility+It->arEffect[i][0])->Effect;
				bNum=(pFirstAbility+It->arEffect[i][0])->bNumeric;
				bPercent=false;
				sprintf (sName,"%s",(pFirstAbility+It->arEffect[i][0])->sName);
			}
			else*/
			{
				for (int j=1; j<=iMaxAbilityNum; j++)
				{
					if ((pFirstAbilityNum+j)->Num==It->arEffect[i][0])
					{
						iEff=(pFirstAbilityNum+j)->Effect;
						bNum=(pFirstAbilityNum+j)->bNumeric;
						bPercent=(pFirstAbilityNum+j)->bPercent;
						sprintf (sName,"%s",(pFirstAbilityNum+j)->sName);
					};
				};
			};			
			switch (iEff)
			{
			case -1: Color=makecol(220,0,0);
					break;
			case 0: Color=makecol(220,220,0);
					break;
			case 1: 
				if (It->arEffect[i][1]<0)
				{Color=makecol(220,0,0);}
				else
				{Color=makecol(50,220,50);};
				break;
			};
			if (bNum)
			{
				if (It->arEffect[i][1]>0)
				{sprintf(sName,"%s +%d",sName,It->arEffect[i][1]);}
				else
				{sprintf(sName,"%s %d",sName,It->arEffect[i][1]);};
				if (bPercent)
				{sprintf(sName,"%s%s",sName,"%");};
			};
			if (It->arEffect[i][2]>0)
			{sprintf(sName,"%s (%s)",sName,(pFirstPhrase+17)->str);};			
			textprintf_centre_ex(bmpHint,Font1,150,StrY,Color,-1,sName);
			StrY+=20;
		};
	};
	StrY+=10;

	clItemSet *pSet=NULL;
	for (int i=1; i<=iMaxItemSet; i++)
	{
		for (int j=0; j<MAX_SET_ITEMS; j++)
		{
			if ((pFirstItemSet+i)->arItems[j]==Item)
			{
				textprintf_centre_ex(bmpHint,Font1,150,StrY,makecol(220,220,220),-1,"%s",(pFirstItemSet+i)->sName);
				StrY+=20;
				pSet=(pFirstItemSet+i);
			};
		};
	};
	int Items=0;
	if (pSet)
	{
		
		for (int i=0; i<MAX_SET_ITEMS; i++)
		{
			if (pSet->arItems[i])
			{
				Color=makecol(100,100,100);
				if (Hero)
				{				
					for (int j=1; j<16; j++)
					{
						if (Hero->arInventory[j][0]==pSet->arItems[i])
						{
							Color=makecol(220,220,50);
							Items++;
						};
					};
				};
				textprintf_centre_ex(bmpHint,Font1,150,StrY,Color,-1,"%s",(pFirstItem+pSet->arItems[i])->sName);
				StrY+=20;
			};
		};
		StrY+=10;
		textprintf_centre_ex(bmpHint,Font1,150,StrY,makecol(220,220,220),-1,"%s",(pFirstPhrase+72)->str);
		StrY+=20;
		for (int k=0; k<MAX_SET_BONUS; k++)
		{
			for (int k1=0; k1<MAX_SET_ABILITY; k1++)
			{
				if (pSet->arBonus[k][k1][0]>0)
				{					
					/*if (pSet->arBonus[k][k1][0]<=iMaxAbility)
					{
						iEff=(pFirstAbility+pSet->arBonus[k][k1][0])->Effect;
						bNum=(pFirstAbility+pSet->arBonus[k][k1][0])->bNumeric;
						bPercent=false;
						sprintf (sName,"%s",(pFirstAbility+pSet->arBonus[k][k1][0])->sName);
					}
					else*/
					{
						for (int j=1; j<=iMaxAbilityNum; j++)
						{
							if ((pFirstAbilityNum+j)->Num==pSet->arBonus[k][k1][0])
							{
								iEff=(pFirstAbilityNum+j)->Effect;
								bNum=(pFirstAbilityNum+j)->bNumeric;
								bPercent=(pFirstAbilityNum+j)->bPercent;
								sprintf (sName,"%s",(pFirstAbilityNum+j)->sName);
							};
						};
					};
					if (Items<pSet->arBonusQ[k])
					{Color=makecol(100,100,100);}
					else
					{Color=makecol(220,220,50);};						
					if (bNum)
					{
						if (pSet->arBonus[k][k1][1]>0)
						{sprintf(sName,"%s +%d",sName,pSet->arBonus[k][k1][1]);}
						else
						{sprintf(sName,"%s %d",sName,pSet->arBonus[k][k1][1]);};
						if (bPercent)
						{sprintf(sName,"%s%s",sName,"%");};
					};
					if (pSet->arBonus[k][k1][2]>0)
					{sprintf(sName,"%s (%s)",sName,(pFirstPhrase+17)->str);};			
					textprintf_centre_ex(bmpHint,Font1,150,StrY,Color,-1,"(%d) %s",pSet->arBonusQ[k],sName);
					StrY+=20;					
				};
			};
		};
	};
	};
	};
};

void MakeInnBldHint(int Build, int Player)
{
	if ((HintType!=4)||(HintMember!=Build))
	{
		HintType=4; HintMember=Build;
		rectfill(bmpHint,0,0,MAX_HINT_W,MAX_HINT_H,COLOR_0);
		blit(bmpGUI_InnBldHint,bmpHint,0,0,0,0,bmpGUI_InnBldHint->w,bmpGUI_InnBldHint->h);
		HintW=bmpGUI_InnBldHint->w;
		HintH=bmpGUI_InnBldHint->h;
		draw_sprite(bmpHint,(pFirstInnerBld+Build)->Pic,14,44);
		draw_sprite(bmpHint,(pFirstInnerBld+Build)->Pic,290,44);
		textprintf_centre_ex(bmpHint,Font1,176,15,COLOR_1,-1,(pFirstInnerBld+Build)->sName);
		if (Player<0)
		{
			textprintf_centre_ex(bmpHint,Font1,141,52,COLOR_2,-1,"%d",(pFirstInnerBld+Build)->iGoldCost);
			textprintf_centre_ex(bmpHint,Font1,141,77,COLOR_2,-1,"%d",(pFirstInnerBld+Build)->iGemCost);
		}
		else
		{
			if (arPlayer[Player].Gold>=arPlayer[Player].GetInBldGoldPrice(Build))
			{textprintf_centre_ex(bmpHint,Font1,141,52,COLOR_2,-1,"%d",arPlayer[Player].GetInBldGoldPrice(Build));}
			else
			{textprintf_centre_ex(bmpHint,Font1,141,52,makecol(250,0,0),-1,"%d",arPlayer[Player].GetInBldGoldPrice(Build));};
			if ((arPlayer[Player].Gem>=arPlayer[Player].GetInBldGemPrice(Build))||(arPlayer[Player].GetInBldGemPrice(Build)==0))
			{textprintf_centre_ex(bmpHint,Font1,141,77,COLOR_2,-1,"%d",arPlayer[Player].GetInBldGemPrice(Build));}
			else
			{textprintf_centre_ex(bmpHint,Font1,141,77,makecol(250,0,0),-1,"%d",arPlayer[Player].GetInBldGemPrice(Build));};
		};
		for (int i=0; i<MAX_BUILDING_RES; i++)
		{
			if ((pFirstInnerBld+Build)->arRes[i]>0)
			{
				draw_sprite(bmpHint,(pFirstResource+(pFirstInnerBld+Build)->arRes[i])->Pic,205+(i%3)*26,47+((i-i%3)/3)*25);
				if ((Player>=0)&&(arPlayer[Player].GetResource((pFirstInnerBld+Build)->arRes[i])<=0))
				{
					draw_sprite(bmpHint,bmpGUI_NoRes,205+(i%3)*26,47+((i-i%3)/3)*25);
				};

			};
		};
		UI_TextBox txtBox;
		txtBox.bShadow=false;
		txtBox.Init(0,0,312,142,-1,COLOR_1,Font2,NULL);
		txtBox.Activate((pFirstInnerBld+Build)->sText);
		draw_sprite(bmpHint,txtBox.Pic,22,130);
	};
};

//Создание всплывающей подсказки по параметрам и свойствам юнитов
void MakeAbilityHint (int Ability)
{
	if (((HintType!=2)||(HintMember!=Ability))&&(Ability>0))
	{
		HintType=2; HintMember=Ability;
		int AbNum=0;
		for (int i=0; i<=iMaxAbilityNum; i++)
		{
			if ((pFirstAbilityNum+i)->Num==Ability)
			{
				AbNum=i;
				break;
			};
		};
		stAbilityNum * AB=(pFirstAbilityNum+AbNum);
		HintH=bmpGUI_StrHint->h;
		HintW=text_length(Font1,AB->sName)+60;
		if (HintW>MAX_HINT_W)
		{HintW=MAX_HINT_W;};
		rectfill(bmpHint,0,0,MAX_HINT_W,MAX_HINT_H,COLOR_0);
		blit(bmpGUI_StrHint,bmpHint,0,0,0,0,HintW-23,HintH);
		blit(bmpGUI_StrHint,bmpHint,bmpGUI_StrHint->w-23,0,HintW-23,0,23,HintH);		
		textprintf_centre_ex(bmpHint,Font1,HintW/2,9,COLOR_1,-1,AB->sName);
	};
};

//Создание всплывающей подсказки с указанной строкой
void MakeStringHint (int Phrase)
{
	if (((HintType!=3)||(HintMember!=Phrase))&&(Phrase>0))
	{
		HintType=3; HintMember=Phrase;
		HintH=bmpGUI_StrHint->h;
		HintW=text_length(Font1,(pFirstPhrase+Phrase)->str)+60;
		if (HintW>MAX_HINT_W)
		{HintW=MAX_HINT_W;};
		rectfill(bmpHint,0,0,MAX_HINT_W,MAX_HINT_H,COLOR_0);
		blit(bmpGUI_StrHint,bmpHint,0,0,0,0,HintW-23,HintH);
		blit(bmpGUI_StrHint,bmpHint,bmpGUI_StrHint->w-23,0,HintW-23,0,23,HintH);		
		textprintf_centre_ex(bmpHint,Font1,HintW/2,9,COLOR_1,-1,(pFirstPhrase+Phrase)->str);
	};
};

void MakeStringHint (char *Str)
{
	HintH=bmpGUI_StrHint->h;
	HintType=0;
	HintW=text_length(Font1,Str)+60;
	if (HintW>MAX_HINT_W)
	{HintW=MAX_HINT_W;};
	rectfill(bmpHint,0,0,MAX_HINT_W,MAX_HINT_H,COLOR_0);
	blit(bmpGUI_StrHint,bmpHint,0,0,0,0,HintW-23,HintH);
	blit(bmpGUI_StrHint,bmpHint,bmpGUI_StrHint->w-23,0,HintW-23,0,23,HintH);	
	textprintf_centre_ex(bmpHint,Font1,HintW/2,9,COLOR_1,-1,Str);
	
};

void MakeTextHint (int ID)
{
	if (((HintType!=6)||(HintMember!=ID))&&(ID>0))
	{
		HintType=6; HintMember=ID;
		HintH=bmpGUI_SmallHint->h;
		HintW=bmpGUI_SmallHint->w;		
		rectfill(bmpHint,0,0,MAX_HINT_W,MAX_HINT_H,COLOR_0);
		blit(bmpGUI_SmallHint,bmpHint,0,0,0,0,HintW,HintH);
		UI_TextBox Txt;
		Txt.Init(0,0,234,107,1,COLOR_1,Font2,NULL);
		Txt.Activate((pFirstMessage+ID)->msg);
		int LN=0;
		if (Txt.Lines<6)
		{LN+=(6-Txt.Lines)*10;};
		draw_sprite(bmpHint,Txt.Pic,15,21+LN);
	};
};

void MakeTextHint (int ID, char *Str)
{
	if (((HintType!=6)||(HintMember!=ID))&&(ID>0))
	{
		HintType=6; HintMember=ID;
		HintH=bmpGUI_SmallHint->h;
		HintW=bmpGUI_SmallHint->w;		
		rectfill(bmpHint,0,0,MAX_HINT_W,MAX_HINT_H,COLOR_0);
		blit(bmpGUI_SmallHint,bmpHint,0,0,0,0,HintW,HintH);
		UI_TextBox Txt;
		Txt.Init(0,0,234,107,1,COLOR_1,Font2,NULL);
		Txt.Activate(Str);
		int LN=0;
		if (Txt.Lines<6)
		{LN+=(6-Txt.Lines)*10;};
		draw_sprite(bmpHint,Txt.Pic,15,21+LN);
	};
};

void MakeGroupHint (int Group, int Player)
{
	if ((HintType!=5)||(HintMember!=Group))
	{
		HintType=5;
		HintMember=Group;
		HintW=bmpGUI_GroupHint->w;
		HintH=bmpGUI_GroupHint->h;	
		rectfill(bmpHint,0,0,MAX_HINT_W,MAX_HINT_H,COLOR_0);
		draw_sprite(bmpHint,bmpGUI_GroupHint,0,0);		
		textprintf_centre_ex(bmpHint,Font1,HintW/2,17,COLOR_1,-1,(pFirstPhrase+250)->str,arBldGroup[Group]);
		textprintf_centre_ex(bmpHint,Font1,HintW/2,37,COLOR_1,-1,GroupName[Group].str);
//		textprintf_centre_ex(bmpHint,Font1,HintW/2,57,COLOR_1,-1,"%s: %d",(pFirstPhrase+251)->str, arBldGroup[Group]);
		int Count=0;
		for (int i=1; i<=iMaxInnerBld; i++)
		{
			if (((pFirstInnerBld+i)->iGroup==Group)&&(arPlayer[Player].arBld[i]))
			{Count++;};
		};
		textprintf_centre_ex(bmpHint,Font1,HintW/2,67,COLOR_1,-1,"(%s: %d)",(pFirstPhrase+244)->str, Count);
	};	
};

void MakeSiteHint(dcProvince *PR, int Site)
{
	if ((HintType!=9)||(HintMember!=PR->iNumber)||(HintDur!=Site))
	{
		HintType=9; HintMember=PR->iNumber; HintDur=Site;
		rectfill(bmpHint,0,0,MAX_HINT_W,MAX_HINT_H,COLOR_0);
		blit(bmpStuff[76],bmpHint,0,0,0,0,bmpStuff[76]->w,bmpStuff[76]->h);
		HintW=bmpStuff[76]->w;
		HintH=bmpStuff[76]->h;		
		textprintf_centre_ex(bmpHint,Font1,143,14,COLOR_1,-1,(pFirstSite+PR->arSite[Site][0])->sName);
		if (PR->arSiteGuard[Site])
		{
			textprintf_centre_ex(bmpHint,Font1,143,39,COLOR_2,-1,"%s:",(pFirstPhrase+25)->str);
			textprintf_centre_ex(bmpHint,Font1,143,64,COLOR_2,-1,(pFirstGuardType+PR->arSite[Site][4])->sName);
		}
		else
		{
			textprintf_centre_ex(bmpHint,Font1,146,39,COLOR_2,-1,(pFirstPhrase+532)->str);
		};		
	};
};

//***************************************************
//*** Окно нахождения сокровищ **********************
//***************************************************

void WindowTreasure (int Level, int Player, dcHero * Hero, int Site, int Index)
{
	iAntisleep=iAntisleepDelay;
	if (Level<0)
	{
		FileLog("!!!!Site warning!!!!");
		return;
	};
	int x=512-bmpGUI_Treasure->w/2;
	int y=384-bmpGUI_Treasure->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_Treasure->w,bmpGUI_Treasure->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_Treasure,bmpWindow,0,0,0,0,bmpGUI_Treasure->w,bmpGUI_Treasure->h);
	UI_Mask mskItem[MAX_LOOT];
	UI_Button1 * buttExit=new UI_Button1();
	WObj=buttExit->Init(x+425,y+222,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit->Activate(1);
	buttExit->ipHotkey=KEY_ESC;
	UI_TextBox txtMessage;
	WObj=txtMessage.Init(x+198,y+41,277,96,2,COLOR_1,Font2,WObj);
	txtMessage.Activate((pFirstMessage+1)->msg);
	int Dropped=0, DropLevel=0, Gold=0, Gem=0;
	int arLoot[MAX_LOOT][2];
	for (int i=0; i<MAX_LOOT; i++)
	{arLoot[i][0]=0;};
	int iShop=0;
	for (int i=0; i<MAX_SITE_ABILITY; i++)
	{
		if ((pFirstSite+Site)->arAbility[i][0]==13)
		{iShop=1;};
		if ((pFirstSite+Site)->arAbility[i][0]==22)
		{iShop=2;};
	};
	if ((Tutorial.Map)&&(Tutorial.Site>=0))
	{
		switch (Tutorial.Site)
		{
		case 0:
			Gold=10+Random(20);
			Gem=Random(10);
			break;
		case 1:
			Gold=20+Random(20);
			Gem=3+Random(10);
			break;
		case 2:
			Gold=50+Random(50);
			Gem=5+Random(10);
			Dropped=1;
			switch (arPlayer[1].arHero[0]->ipClass1)
			{
			case 1:
				arLoot[0][0]=168;
				break;
			case 2:
				arLoot[0][0]=191;
				break;
			case 3:
				arLoot[0][0]=146;
				break;
			case 4:
				arLoot[0][0]=237;
				break;
			};
			break;
		};
		for (int i=0; i<Dropped; i++)
		{				
			arLoot[i][1]=Random(1+(pFirstItem+arLoot[i][0])->iDurability/3)+1;				
			WObj=mskItem[i].Init(x+303+(i%3)*60,y+158+((i-i%3)/3)*60,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
			draw_sprite(bmpWindow,(pFirstItem+arLoot[i][0])->Pic,309+(i%3)*60,164+((i-i%3)/3)*60);
		};
	}
	else
	{
	if (iShop>0)
	{
		dcShop *Shop;
		if (iShop==1)
		{Shop=(pFirstShop+Level);}
		else
		{
			Shop=new dcShop();			
			int Stuff=8+Random(5);
			ClearRoll();
			int Count=0;
			for (int i=1; i<=iMaxPureItem; i++)
			{
				if (((pFirstItem+i)->iRarity>1)&&((pFirstItem+i)->iShopLevel<5))
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
		};
		if (bStaticLoot)
		{
			Gold=400+Random(400, Index, Hero->ipProvince);
			Gem=80+Random(80, Hero->ipProvince, Index);
		}
		else
		{
			Gold=400+Random(400);
			Gem=80+Random(80);
		}
		int Count=0;
		ClearRoll();
		for (int i=0; i<SHOP_SIZE; i++)
		{
			if (Shop->Items[i][0]>0)
			{
				RollerN[Count]=Shop->Items[i][0];
				RollerP[Count]=10;
				Count++;
			};
		};
		if (bStaticLoot)
		{Dropped=3+Random(3, Index, Hero->ipProvince);}
		else
		{Dropped=3+Random(3);}
		if (Dropped>Count)
		{Dropped=Count;};
		if (iShop==2)
		{delete Shop;};
	}
	else
	{	
	if (bStaticLoot)
	{
	Gold=Level*50+Random(1+Level*50,Hero->ipProvince, Index);
	Gem=(Level-1)*5+Random(10+Level, CurGame.iSeed, Hero->ipProvince, Index);
	switch (Level)
	{
	case 1:
		Dropped=Random(2, Index, Hero->ipProvince);
		DropLevel=1;	
		break;	
	case 2:
		if (Random(10, Index, Hero->ipProvince)>4)
		{
			Dropped=2;
			DropLevel=1;
		}
		else
		{
			Dropped=1;
			DropLevel=2;
		};		
		break;
	case 3:
		if (Random(10, Index, Hero->ipProvince)>2)
		{
			Dropped=2;
			DropLevel=2;
		}
		else
		{
			Dropped=1;
			DropLevel=3;
		};		
		break;
	case 4:
		if (Random(10, Index, Hero->ipProvince)>6)
		{
			Dropped=2;
			DropLevel=2;
		}
		else
		{
			Dropped=1;
			DropLevel=3;
		};		
		break;
	case 5:
		if (Random(10, Index, Hero->ipProvince)>2)
		{
			Dropped=2;
			DropLevel=3;
		}
		else
		{
			Dropped=1;
			DropLevel=4;
		};		
		break;
	case 6:
		if (Random(10, Index, Hero->ipProvince)>6)
		{
			Dropped=2;
			DropLevel=3;
		}
		else
		{
			Dropped=1;
			DropLevel=4;
		};		
		break;
	case 7:
		Dropped=1;
		DropLevel=4;
		break;
	case 8:
		if (Random(10, Index, Hero->ipProvince)>2)
		{
			Dropped=2;
			DropLevel=4;
		}
		else
		{
			Dropped=1;
			DropLevel=5;
		};		
		break;
	case 9:
		if (Random(10, Index, Hero->ipProvince)>4)
		{
			Dropped=2;
			DropLevel=4;
		}
		else
		{
			Dropped=1;
			DropLevel=5;
		};		
		break;
	case 10:
		if (Random(10, Index, Hero->ipProvince)>6)
		{
			Dropped=2;
			DropLevel=4;
			Gold+=200;
			Gem+=25;
		}
		else
		{
			Dropped=1;
			DropLevel=5;
		};		
		break;
	case 11:
		Dropped=1;
		DropLevel=5;
		break;
	case 12:
		if (Random(10, Index, Hero->ipProvince)>2)
		{
			Dropped=1;
			DropLevel=5;
			Gold+=400;
			Gem+=50;
		}
		else
		{
			Dropped=1;
			DropLevel=6;
		};		
		break;
	case 13:
		if (Random(10, Index, Hero->ipProvince)>6)
		{
			Dropped=2;
			DropLevel=5;			
		}
		else
		{
			Dropped=1;
			DropLevel=6;
		};		
		break;
	default:
		Dropped=1+(Level-13)/2+Random((Level-12)/2, Index, Hero->ipProvince);
		DropLevel=6;
		break;
	}
	}
	else
	{
	Gold=Level*50+Random(1+Level*50);
	Gem=(Level-1)*5+Random(10+Level);
	switch (Level)
	{
	case 1:
		Dropped=Random(2);
		DropLevel=1;	
		break;	
	case 2:
		if (Random(10)>4)
		{
			Dropped=2;
			DropLevel=1;
		}
		else
		{
			Dropped=1;
			DropLevel=2;
		};		
		break;
	case 3:
		if (Random(10)>2)
		{
			Dropped=2;
			DropLevel=2;
		}
		else
		{
			Dropped=1;
			DropLevel=3;
		};		
		break;
	case 4:
		if (Random(10)>6)
		{
			Dropped=2;
			DropLevel=2;
		}
		else
		{
			Dropped=1;
			DropLevel=3;
		};		
		break;
	case 5:
		if (Random(10)>2)
		{
			Dropped=2;
			DropLevel=3;
		}
		else
		{
			Dropped=1;
			DropLevel=4;
		};		
		break;
	case 6:
		if (Random(10)>6)
		{
			Dropped=2;
			DropLevel=3;
		}
		else
		{
			Dropped=1;
			DropLevel=4;
		};		
		break;
	case 7:
		Dropped=1;
		DropLevel=4;
		break;
	case 8:
		if (Random(10)>2)
		{
			Dropped=2;
			DropLevel=4;
		}
		else
		{
			Dropped=1;
			DropLevel=5;
		};		
		break;
	case 9:
		if (Random(10)>4)
		{
			Dropped=2;
			DropLevel=4;
		}
		else
		{
			Dropped=1;
			DropLevel=5;
		};		
		break;
	case 10:
		if (Random(10)>6)
		{
			Dropped=2;
			DropLevel=4;
			Gold+=200;
			Gem+=25;
		}
		else
		{
			Dropped=1;
			DropLevel=5;
		};		
		break;
	case 11:
		Dropped=1;
		DropLevel=5;
		break;
	case 12:
		if (Random(10)>2)
		{
			Dropped=1;
			DropLevel=5;
			Gold+=400;
			Gem+=50;
		}
		else
		{
			Dropped=1;
			DropLevel=6;
		};		
		break;
	case 13:
		if (Random(10)>6)
		{
			Dropped=2;
			DropLevel=5;			
		}
		else
		{
			Dropped=1;
			DropLevel=6;
		};		
		break;
	default:
		Dropped=1+(Level-13)/2+Random((Level-12)/2);
		DropLevel=6;
		break;
	}
	}
	if (Dropped>MAX_LOOT)
	{Dropped=MAX_LOOT;};
	ClearRoll();
	int Count=0;
	if (Hero->pSquad[0]->iLife>0)
	{
		Gold+=Gold*(Hero->CheckHeroBonus(987)/10)/10;
		Gem+=Gem*(Hero->CheckHeroBonus(987)/10)/10;
	};
	for (int i=1; i<=iMaxItem; i++)
	{
		if ((pFirstItem+i)->iShopLevel==DropLevel)
		{
			bool bAdd=true;
			switch ((pFirstItem+i)->ipClass)
			{
			case 11:
				if (*(arPlayer[Player].pDefender+(pFirstItem+i)->arEffect[0][1])<0)
				{bAdd=false;};
				break;
			case 12:
				if (*(arPlayer[Player].pOuterBld+(pFirstItem+i)->arEffect[0][1])<0)
				{bAdd=false;};
				break;
			case 13:
				if ((Tutorial.Map)||(*(arPlayer[Player].pRitual+(pFirstItem+i)->arEffect[0][1])<0))
				{bAdd=false;};
				break;
			};
			if (bAdd)
			{
				RollerN[Count]=i;
				RollerP[Count]=7-(pFirstItem+i)->iRarity;
				Count++;
			};
		};
	};
	};

	for (int i=0; i<Dropped; i++)
	{		
		if (bStaticLoot)
		{arLoot[i][0]=Roll(true,i,Index*5,Hero->ipProvince);}
		else
		{arLoot[i][0]=Roll(true);}
		if (((pFirstItem+arLoot[i][0])->iDurability>1)&&(Random(30)>10))
		{
			arLoot[i][1]=(pFirstItem+arLoot[i][0])->iDurability/2+Random(1+(pFirstItem+arLoot[i][0])->iDurability/2);
		}
		else
		{
			arLoot[i][1]=(pFirstItem+arLoot[i][0])->iDurability;
		};
		WObj=mskItem[i].Init(x+303+(i%3)*60,y+158+((i-i%3)/3)*60,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
		draw_sprite(bmpWindow,(pFirstItem+arLoot[i][0])->Pic,309+(i%3)*60,164+((i-i%3)/3)*60);
		switch ((pFirstItem+arLoot[i][0])->iRarity)
		{
		case 4:
			arPlayer[Player].arLoot[0]++;
			break;
		case 5:
			arPlayer[Player].arLoot[1]++;
			break;
		case 6:
			arPlayer[Player].arLoot[2]++;
			break;
		};
	};
	if ((arPlayer[Player].Control==1)&&(Dropped<MAX_LOOT)&&(CurGame.iGameType==2)&&(!Tutorial.Map)&&(Campaign.GetActiveShard())&&(iShop==0)&&(Level>1)&&((Level>14)||(Random(30-Level*2)==0)))
	{
		ClearRoll();
		int Count=0;
		for (int i=0; i<MAX_SHARD_BUILD; i++)
		{
			if ((Campaign.GetActiveShard()->arBuilding[i]>0)&&(Campaign.arCampPlayer[0].arBld[Campaign.GetActiveShard()->arBuilding[i]]==0))
			{
				RollerN[Count]=Campaign.GetActiveShard()->arBuilding[i];
				RollerP[Count]=10;
				Count++;
			};
		};
		if (Count>0)
		{
			int Bld=Roll(false);
			Campaign.arCampPlayer[0].arBld[Bld]=1;
			arLoot[Dropped][0]=-Bld;
			WObj=mskItem[Dropped].Init(x+303+(Dropped%3)*60,y+158+((Dropped-Dropped%3)/3)*60,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
			draw_sprite(bmpWindow,(pFirstInnerBld+Bld)->Pic,309+(Dropped%3)*60,164+((Dropped-Dropped%3)/3)*60);
			Dropped++;
		};
	};
	}; //Tutorial
	if (arPlayer[Player].Control==1)
	{
		Gold=Gold*arDifficulty[arPlayer[Player].iDifficulty].iLoot/100;
		Gem=Gem*arDifficulty[arPlayer[Player].iDifficulty].iLoot/100;
	};
	if ((pFirstSite+Site)->CheckAbility(3)==2)
	{
		Gem+=Gold*2/5;
		Gold=0;
	};
	textprintf_centre_ex(bmpWindow,Font1,258,195,COLOR_2,-1,"%d",Gold);
	textprintf_centre_ex(bmpWindow,Font1,258,256,COLOR_2,-1,"%d",Gem);


	if (arPlayer[Player].Control==1)
	{		
		while (!buttExit->Check())
		{
			bHint=false;
			for (int i=0; i<Dropped; i++)
			{
				int Check=mskItem[i].Check();
				if (arLoot[i][0]>0)
				{
					if (Check>1)
					{
						MakeItemHint(arLoot[i][0],arLoot[i][1],Hero,0,0,0);
						bHint=true;
					};
					if ((Check==30)||(Check==40))
					{
						bHint=false;
						WObj=ShowActiveObjects(WObj,6,0);
						WindowItemInfo(arLoot[i][0],arLoot[i][1]);
					};
				}
				else
				{
					if (Check>1)
					{
						MakeInnBldHint(abs(arLoot[i][0]),Player);
						bHint=true;
					};
					if ((Check==30)||(Check==40))
					{
						bHint=false;
						WObj=ShowActiveObjects(WObj,6,0);
						WindowInfo((pFirstPhrase+438)->str,(pFirstMessage+50)->msg,(pFirstInnerBld+abs(arLoot[i][0]))->Pic);
					};
				};
			};
			if (Timer)
			{
				Timer=false;
				WObj=ShowActiveObjects(WObj,6,1);
			}
			else
			{Sleeper(24);};
		};
		blit(bmpBack,screen1,0,0,0,0,1024,768);
		blit(bmpBack,screen,0,0,0,0,1024,768);
	};	
	bHint=false;
	HintType=0;
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};	
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	delete buttExit;	
	set_mouse_sprite(OldMouse);
	arPlayer[Player].Gold+=Gold;
	arPlayer[Player].Gem+=Gem;
	//	for (int i=0; i<Dropped; i++)
//	{
//		Hero->TakeLoot(arLoot[i][0],arLoot[i][1]);
//	};
	bool NeedLoot=false;
	for (int i=0; i<Dropped; i++)
	{
		if (arLoot[i][0]>0)
		{
			if (Hero->TakeLoot(arLoot[i][0],arLoot[i][1]))
			{
				NeedLoot=true;
			}
			else
			{
				arLoot[i][0]=0;
			};
		}
		else
		{arLoot[i][0]=0;};
	};	
	if (NeedLoot)
	{		
		for (int i=0; i<9; i++)
		{
			int Itm=0;
			while ((arLoot[Itm][0]==0)&&(Itm<Dropped))
			{
				Itm++;
			};
			if (Itm<Dropped)
			{
				arLootTable[i][0]=arLoot[Itm][0];
				arLootTable[i][1]=arLoot[Itm][1];
				arLoot[Itm][0]=0;
				arLoot[Itm][1]=0;
			}
			else
			{
				arLootTable[i][0]=0;
				arLootTable[i][1]=0;
			};			
		};
		WindowTakeLoot(Hero);
	};	
	iAntisleep=iAntisleepDelay;
};

bool CampAllowBuild(int Build, int Player)
{
	if ((CurGame.iGameType==2)&&((pFirstInnerBld+Build)->iLevel!=0))
	{
		int Num=0;
		if (arPlayer[Player].Control!=1)
		{Num=arPlayer[Player].ipProfile;};
		if (Campaign.arCampPlayer[Num].arBld[Build])
		{return true;};
		dcShard *SH=pFirstShard;
		while (SH)
		{
			if (SH->ipPlayer==Num)
			{
				for (int i=0; i<MAX_SHARD_BUILD; i++)
				{
					if (SH->arBuilding[i]==Build)
					{return true;};
				};				
			};
			SH=SH->pNext;
		};		
		return false;
	}
	else
	{
		if ((Tutorial.Map)&&((pFirstInnerBld+Build)->arAbility[0][0]==47))
		{return false;}
		else
		{return true;};
	};
};

//Возвращает ложь, если постройка или её предшественник требует союз с расой, с которой он не заключен
bool NoAllyFlag(int Build, int Player)
{
	if (!CampAllowBuild(Build, Player))
	{return false;};
	for (int i=0; i<MAX_BUILDING_ABILITY; i++)
	{
		if ((pFirstInnerBld+Build)->arAbility[i][0]==47)
		{
			if (arPlayer[Player].iUnion!=(pFirstInnerBld+Build)->arAbility[i][1])
			{return false;};
		};
	};
	for (int i=0; i<MAX_BUILDING_NEED; i++)
	{
		if (((pFirstInnerBld+Build)->arBuildings[i]>0)&&((pFirstInnerBld+Build)->arBuildings[i]<1000))
		{
			if (!NoAllyFlag((pFirstInnerBld+Build)->arBuildings[i], Player))
			{return false;};
		};
	};
	return true;
};

int WindowInnBldInfo(int Build, int Player)
{
	iAntisleep=iAntisleepDelay;
	int Result=0;
	const int SPELL=6;
	const int BREED=25;
	clInnerBuilding *BLD=(pFirstInnerBld+Build);
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);	
	int Height=390;
	int iUnit=0, iSpell=0;
	for (int i=0; i<MAX_BUILDING_ABILITY; i++)
	{
		switch (BLD->arAbility[i][0])
		{		
		case 11:
		case 15:
		case 27:

			iSpell=70;
			break;
		case 8:
			iUnit=90;
			break;

		};
	};
	bool bSpell=false, bItem=false, bBreed=false;
	for (int i=1; i<=iMaxSpell; i++)
	{
		if ((pFirstSpell+i)->ipBuilding==Build)
		{
			bSpell=true;
			iSpell=70;
		};
	};
	for (int i=1; i<=iMaxItem; i++)
	{
		if ((pFirstItem+i)->ipBuilding==Build)
		{
			bItem=true;
			iSpell=70;
		};
	};
	Height+=iUnit+iSpell;
	if (BLD->arBuildings[0]>0)
	{Height+=100;};	
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		for (int j=0; j<MAX_BUILDING_NEED; j++)
		{
			if (((pFirstInnerBld+i)->arBuildings[j]==Build)&&(CampAllowBuild(i, Player))&&(NoAllyFlag(i,Player)))
			{bBreed=true;};
		};
	};
	if (bBreed)
	{Height+=100;};
	int x=512-bmpGUI_InnBldInfo->w/2;
	int y=384-Height/2;	
	BITMAP *bmpWindow=create_bitmap(bmpGUI_InnBldInfo->w,Height);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_InnBldInfo,bmpWindow,0,0,0,0,bmpGUI_InnBldInfo->w,Height);
	blit(bmpGUI_InnBldInfo,bmpWindow,0,717,0,Height-51,bmpGUI_InnBldInfo->w,51);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+300,y+Height-49,1,3,bmpButton[247],bmpButton[248],bmpButton[249],bmpButton[249],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;
	UI_Button1 buttBuild;
	WObj=buttBuild.Init(x+18,y+Height-49,1,3,bmpButton[239],bmpButton[240],bmpButton[241],bmpButton[242],WObj);
	if ((!arPlayer[Player].arBld[Build])&&(arPlayer[Player].CanBuild(Build))&&(arPlayer[Player].iCanInnerBld>0)&&
		(arPlayer[Player].Gold>=arPlayer[Player].GetInBldGoldPrice(Build))&&
		((arPlayer[Player].Gem>=arPlayer[Player].GetInBldGemPrice(Build)||(arPlayer[Player].GetInBldGemPrice(Build)==0))))
	{buttBuild.Activate(1);}
	else
	{buttBuild.Activate(0);};
	buttBuild.iHint=93;
	buttBuild.wavSound=wavBuild;
	UI_Button1 buttPlan;
	WObj=buttPlan.Init(x+159,y+Height-49,1,3,bmpButton[243],bmpButton[244],bmpButton[245],bmpButton[246],WObj);
	if ((!arPlayer[Player].arBld[Build])&&(!arPlayer[Player].CanBuild(Build))&&(!IsRestricted(Build,Player)))
//Добавить проверку необязательных построек
	{buttPlan.Activate(1);}
	else
	{buttPlan.Activate(0);};
	buttPlan.iHint=251;

	draw_sprite(bmpWindow,BLD->Pic,23,54);
	draw_sprite(bmpWindow,BLD->Pic,299,54);
	textprintf_centre_ex(bmpWindow,Font1,185,24,COLOR_1,-1,BLD->sName);
	if (arPlayer[Player].Gold>=arPlayer[Player].GetInBldGoldPrice(Build))
	{textprintf_centre_ex(bmpWindow,Font1,150,62,COLOR_2,-1,"%d",arPlayer[Player].GetInBldGoldPrice(Build));}
	else
	{textprintf_centre_ex(bmpWindow,Font1,150,62,makecol(250,0,0),-1,"%d",arPlayer[Player].GetInBldGoldPrice(Build));};
	if (arPlayer[Player].Gem>=arPlayer[Player].GetInBldGemPrice(Build))
	{textprintf_centre_ex(bmpWindow,Font1,150,87,COLOR_2,-1,"%d",arPlayer[Player].GetInBldGemPrice(Build));}
	else
	{textprintf_centre_ex(bmpWindow,Font1,150,87,makecol(250,0,0),-1,"%d",arPlayer[Player].GetInBldGemPrice(Build));};

	UI_Mask1 mskRes[MAX_BUILDING_RES];
	for (int i=0; i<MAX_BUILDING_RES; i++)
	{
		if ((pFirstInnerBld+Build)->arRes[i]>0)
		{
			draw_sprite(bmpWindow,(pFirstResource+(pFirstInnerBld+Build)->arRes[i])->Pic,214+(i%3)*26,57+((i-i%3)/3)*25);
			mskRes[i].Init(x+214+(i%3)*26,y+57+((i-i%3)/3)*25,20,20,1);
			if (arPlayer[Player].GetResource((pFirstInnerBld+Build)->arRes[i])<=0)
			{
				draw_sprite(bmpWindow,bmpGUI_NoRes,214+(i%3)*26,57+((i-i%3)/3)*25);
			};

		};
	};
	if (arPlayer[Player].arBld[Build])
	{textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,120,COLOR_1,-1,(pFirstPhrase+244)->str);}
	else
	{
		if (arPlayer[Player].CanBuild(Build))
		{
			if (arPlayer[Player].iCanInnerBld>0)
			{
				if ((arPlayer[Player].Gold>=arPlayer[Player].GetInBldGoldPrice(Build))&&(arPlayer[Player].Gem>=arPlayer[Player].GetInBldGemPrice(Build)))
				{
					textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,120,makecol(0,250,0),-1,(pFirstPhrase+245)->str);
					textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2+1,120+1,COLOR_1,-1,(pFirstPhrase+245)->str);
				}
				else
				{
					textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,120,makecol(250,0,0),-1,(pFirstPhrase+246)->str);
					textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2+1,120+1,COLOR_1,-1,(pFirstPhrase+246)->str);
				};
			}
			else
			{
				textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,120,makecol(250,0,0),-1,(pFirstPhrase+249)->str);
				textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2+1,120+1,COLOR_1,-1,(pFirstPhrase+249)->str);
			};
		}
		else
		{
			//Добавить проверку необязательных построек
			if (IsRestricted(Build,Player))
			{
				textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,120,makecol(250,0,0),-1,(pFirstPhrase+248)->str);
				textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2+1,120+1,COLOR_1,-1,(pFirstPhrase+248)->str);
			}
			else
			{
				textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,120,makecol(250,0,0),-1,(pFirstPhrase+247)->str);
				textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2+1,120+1,COLOR_1,-1,(pFirstPhrase+247)->str);
			};
		};
	};
	UI_TextBox txtBox;
	txtBox.bShadow=false;
	WObj=txtBox.Init(x+31,y+170,312,142,3,COLOR_1,Font2,WObj);
	txtBox.Activate((pFirstInnerBld+Build)->sText);
	int Shift=342;

//Маска юнита
	UI_Mask mskUnit;	
	if (iUnit>0)
	{
		draw_sprite(bmpWindow,bmpGUI_UnitSlot,bmpWindow->w/2-bmpGUI_UnitSlot->w/2,Shift);
		WObj=mskUnit.Init(x+bmpWindow->w/2-bmpGUI_UnitSlot->w/2-3,y+Shift-6,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,WObj,3);
		for (int i=0; i<MAX_BUILDING_ABILITY; i++)
		{
			if (BLD->arAbility[i][0]==8) 
			{iUnit=BLD->arAbility[i][1];};
		};
		draw_sprite(bmpWindow,(pFirstUnitType+iUnit)->Pic,bmpWindow->w/2-(pFirstUnitType+iUnit)->Pic->w/2-2,Shift);
		Shift+=90;
	};

//Маски объектов
	UI_Mask mskSpell[SPELL];
	int arSpell[SPELL][2];
	int SpellShift;
	for (int i=0; i<SPELL; i++)
	{arSpell[i][0]=0;};
	if (iSpell>0)
	{
		iSpell=0;
		if (bSpell)
		{
			for (int i=1; i<=iMaxSpell; i++)
			{
				if ((pFirstSpell+i)->ipBuilding==Build)
				{
					arSpell[iSpell][0]=1;
					arSpell[iSpell][1]=i;
					iSpell++;					
				};
			};
		};
		if (bItem)
		{	
			for (int i=1; i<=iMaxItem; i++)
			{
				if ((pFirstItem+i)->ipBuilding==Build)
				{
					arSpell[iSpell][0]=2;
					arSpell[iSpell][1]=i;
					iSpell++;			
				};
			};
		};
		for (int i=0; i<MAX_BUILDING_ABILITY; i++)
		{
			switch (BLD->arAbility[i][0])
			{		
			case 11:
				arSpell[iSpell][0]=3;
				arSpell[iSpell][1]=BLD->arAbility[i][1];
				iSpell++;
				break;
			case 15:
				arSpell[iSpell][0]=4;
				arSpell[iSpell][1]=BLD->arAbility[i][1];
				iSpell++;
				break;
			case 27:
				arSpell[iSpell][0]=5;
				arSpell[iSpell][1]=BLD->arAbility[i][1];
				iSpell++;
				break;
			};
		};
		SpellShift=bmpWindow->w/2-iSpell*30;
		for (int i=0; i<iSpell; i++)
		{
			draw_sprite(bmpWindow,bmpGUI_SpellSlot,SpellShift+i*60,Shift);
			WObj=mskSpell[i].Init(x+SpellShift+i*60-1,y+Shift-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);
			switch (arSpell[i][0])
			{
			case 1:
				draw_sprite(bmpWindow,(pFirstSpell+arSpell[i][1])->Pic,SpellShift+i*60+5,Shift+5);
				break;
			case 2:
				draw_sprite(bmpWindow,(pFirstItem+arSpell[i][1])->Pic,SpellShift+i*60+5,Shift+5);
				break;
			case 3:
				draw_sprite(bmpWindow,(pFirstOuterBld+arSpell[i][1])->Pic,SpellShift+i*60+5,Shift+5);
				break;
			case 4:
				draw_sprite(bmpWindow,(pFirstDefender+arSpell[i][1])->Pic,SpellShift+i*60+5,Shift+5);
				break;
			case 5:
				draw_sprite(bmpWindow,(pFirstRitual+arSpell[i][1])->Pic,SpellShift+i*60+5,Shift+5);
				break;
			};
		};
		Shift+=70;
	};

//Маски предшественников
	UI_Mask mskParent[MAX_BUILDING_NEED];
	int Parent=0;
	if (BLD->arBuildings[0]>0)
	{
		draw_sprite(bmpWindow,bmpGUI_String,bmpWindow->w/2-bmpGUI_String->w/2,Shift);
		textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,Shift+9,COLOR_1,-1,(pFirstPhrase+242)->str);
		BITMAP *bmpDD=create_bitmap(52,52);
		rectfill(bmpDD,0,0,52,52,COLOR_1);

		for (int i=0; i<MAX_BUILDING_NEED; i++)
		{
			if (BLD->arBuildings[i]>0)
			{Parent++;};
		};
		Shift+=30;	
		int ParentShift=bmpWindow->w/2-Parent*30;
		for (int i=0; i<Parent; i++)
		{
			draw_sprite(bmpWindow,bmpGUI_SpellSlot,ParentShift+i*60,Shift);
			WObj=mskParent[i].Init(x+ParentShift+i*60-1,y+Shift-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);
			if (BLD->arBuildings[i]<1000)
			{
				draw_sprite(bmpWindow,(pFirstInnerBld+BLD->arBuildings[i])->Pic,ParentShift+i*60+5,Shift+5);
				if (!arPlayer[Player].arBld[BLD->arBuildings[i]])
				{draw_trans_sprite(bmpWindow,bmpDD,ParentShift+i*60+5,Shift+5);};
			}
			else
			{
				//Добавить требование всех построек из группы
				draw_sprite(bmpWindow,bmpGUI_GroupIcon,ParentShift+i*60+5,Shift+5);
				if (!GroupIsFull(Player,BLD->arBuildings[i]-1000))
				{draw_trans_sprite(bmpWindow,bmpDD,ParentShift+i*60+5,Shift+5);};
			};

			
		};
		destroy_bitmap(bmpDD);
		Shift+=70;
	};

	UI_Mask mskBreed[5];
	int arBreed[BREED];
	int iBreed=0, BreedLine=0;
	UI_Button1 buttLeft, buttRight;
	for (int i=0; i<BREED; i++)
	{arBreed[i]=0;};
	if (bBreed)
	{
		draw_sprite(bmpWindow,bmpGUI_String,bmpWindow->w/2-bmpGUI_String->w/2,Shift);
		textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,Shift+9,COLOR_1,-1,(pFirstPhrase+243)->str);
		Shift+=30;				
		for (int i=1; i<=iMaxInnerBld; i++)
		{
			for (int j=0; j<MAX_BUILDING_NEED; j++)
			{
				if (((pFirstInnerBld+i)->arBuildings[j]==Build)&&(CampAllowBuild(i,Player))&&(NoAllyFlag(i,Player)))
				{
					arBreed[iBreed]=i;
					iBreed++;
				};
			};
		};
		if (iBreed>5)
		{
			WObj=buttLeft.Init(x+15,y+Shift+15,1,3,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],WObj);
			buttLeft.Activate(0);
			WObj=buttRight.Init(x+340,y+Shift+15,1,3,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],WObj);
			buttRight.Activate(1);
		};
		for (int i=0; i<5; i++)
		{WObj=mskBreed[i].Init(0,0,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);};
	};


	int Check;
	while ((Result==0)&&(!buttExit.Check()))
	{
		if (bBreed)
		{
			int iBShift=bmpWindow->w/2-GetMin(5,iBreed-BreedLine*5)*30;
			blit(bmpGUI_InnBldInfo,bmpWindow,0,Shift,0,Shift,bmpWindow->w,70);
			for (int i=0; i<GetMin(5,iBreed-BreedLine*5); i++)
			{
				draw_sprite(bmpWindow,bmpGUI_SpellSlot,iBShift+i*60,Shift);
				mskBreed[i].Replace(x+iBShift+i*60-1,y+Shift-1);
				draw_sprite(bmpWindow,(pFirstInnerBld+arBreed[BreedLine*5+i])->Pic,iBShift+i*60+5,Shift+5);			
			};
			bBreed=false;
		};

		if (iUnit>0)
		{
			Check=mskUnit.Check();
			if ((Check==30)||(Check==40))
			{
				WindowUnitTypeInfo(iUnit);
		//		dcUnit *pUnit=new dcUnit(iUnit,0);
		//		WindowUnitInfo(pUnit,0);
		//		delete pUnit;
			};
		};
		for (int i=0; i<iSpell; i++)
		{
			Check=mskSpell[i].Check();
			if ((Check==30)||(Check==40))
			{
				bHint=false;
				WObj=ShowActiveObjects(WObj,6,false);
				switch (arSpell[i][0])
				{
				case 1:
					WindowSpellInfo(arSpell[i][1]);
					break;
				case 2:
					WindowItemInfo(arSpell[i][1],(pFirstItem+arSpell[i][1])->iDurability);
					break;
				case 3:
					WindowOuterBldInfo(arSpell[i][1]);
					break;					
				case 4:
					WindowDefenderInfo(arSpell[i][1], 0, 0);
					break;
				case 5:
					WindowRitualInfo(arSpell[i][1]);
					break;
				};
			}
			else
			{
				if (Check>1)
				{
					switch (arSpell[i][0])
					{
					case 1:
						MakeStringHint((pFirstSpell+arSpell[i][1])->sName);
						bHint=true;
						break;
					case 2:
						MakeItemHint(arSpell[i][1],(pFirstItem+arSpell[i][1])->iDurability,NULL,0,0,0);
						bHint=true;
						break;
					case 3:
						MakeStringHint((pFirstOuterBld+arSpell[i][1])->sName);
						bHint=true;
						break;
					case 4:
						MakeStringHint((pFirstDefender+arSpell[i][1])->sName);
						bHint=true;
						break;
					case 5:
						MakeStringHint((pFirstRitual+arSpell[i][1])->sName);
						bHint=true;
						break;
					};
				};
			};
		};
		for (int i=0; i<Parent; i++)
		{
			Check=mskParent[i].Check();
			if ((Check==30)||(Check==40))
			{
				if (BLD->arBuildings[i]<1000)
				{Result=BLD->arBuildings[i];}
				else
				{
					//Добавить описание группы
				};
			}
			else
			{
				if (Check>1)
				{
					if (BLD->arBuildings[i]<1000)
					{
						MakeInnBldHint(BLD->arBuildings[i],Player);
						bHint=true;
					}
					else
					{
						MakeGroupHint(BLD->arBuildings[i]-1000, Player);
						bHint=true;
						//Добавить описание группы
					};
				};
			};
		};
		for (int i=0; i<GetMin(5,iBreed-BreedLine*5); i++)
		{
			Check=mskBreed[i].Check();
			if ((Check==30)||(Check==40))
			{
				Result=arBreed[i+BreedLine*5];
			}
			else
			{
				if (Check>1)
				{
					MakeInnBldHint(arBreed[i+BreedLine*5],Player);
					bHint=true;
				};
			};
		};
		for (int i=0; i<MAX_BUILDING_RES; i++)
		{
			if ((pFirstInnerBld+Build)->arRes[i]>0)
			{
				Check=mskRes[i].Check();
				if (Check>1)
				{
					if ((Check==30)||(Check==40))
					{
						bHint=false;
						WObj=ShowActiveObjects(WObj,6,false);
						WindowResourceInfo((pFirstInnerBld+Build)->arRes[i]);
					}
					else
					{
						bHint=true;
						MakeStringHint((pFirstResource+(pFirstInnerBld+Build)->arRes[i])->sName);
					};
				};
			};
		};
		if (iBreed>5)
		{
			if (buttLeft.Check())
			{
				BreedLine--;
				if (BreedLine==0)
				{buttLeft.Activate(0);};
				buttRight.Activate(1);
				bBreed=true;
			};
			if (buttRight.Check())
			{
				BreedLine++;
				if ((BreedLine+1)*5>=iBreed)
				{buttRight.Activate(0);};
				buttLeft.Activate(1);
				bBreed=true;
			};
		};
		if (buttBuild.Check())
		{Result=-1;};
		if (buttPlan.Check())
		{Result=-2;};
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
//	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	blit(bmpBack,screen1,0,0,0,0,1024,768);
//	blit(bmpBack,screen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	iAntisleep=iAntisleepDelay;
	return Result;
};

int WindowMessage(int Player, int Sender)
{
	iAntisleep=iAntisleepDelay;
	dcMessage *Mess=&arPlayer[Sender].Politic[Player].Message;
	int Result;
	int x=512-bmpStuff[21]->w/2;
	int y=384-bmpStuff[21]->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);	
	int Height=400;
	bool bGold=false;
	if ((Mess->Trade.Gem>0)||(Mess->Trade.Gold>0)||(Mess->Trade1.Gem>0)||(Mess->Trade1.Gold>0))
	{
		bGold=true;
		Height+=92;
	};
	int Give=0, Take=0;
	for (int i=0; i<16; i++)
	{
		if (Mess->Trade.Stuff[i]>0)
		{Give++;};
		if (Mess->Trade1.Stuff[i]>0)
		{Take++;};
	};
	int MaxTrade;
	if (Give>Take)
	{MaxTrade=Give-1;}
	else
	{MaxTrade=Take-1;};
	bool bTrade=false;
	if (MaxTrade>=0)
	{
		bTrade=true;
		Height+=((MaxTrade-MaxTrade%4)/4+1)*60;
	};
	if ((bTrade)||(bGold))
	{Height+=36;};
	y=384-Height/2;
	BITMAP *bmpWindow=create_bitmap(bmpStuff[21]->w,Height);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpStuff[21],bmpWindow,0,0,0,0,bmpStuff[21]->w,Height);
	draw_sprite(bmpWindow,bmpStuff[22],0,Height-bmpStuff[22]->h);
	
	
	UI_TextBox txtMessage;
	txtMessage.bShadow=false;
	WObj=txtMessage.Init(x+193,y+100,416,187,2,COLOR_1,Font2,WObj);
	txtMessage.Activate(Mess->Text);
	draw_sprite(bmpWindow,arPlayer[Sender].GetAvatar(),25,54);
	draw_sprite(bmpWindow,bmpHerb[arPlayer[Sender].iColor],81,15);
	draw_sprite(bmpWindow,bmpHerb[arPlayer[Sender].iColor],546,15);

	if (Mess->Type==6)
	{textprintf_centre_ex(bmpWindow,Font1,330,23,COLOR_1,-1,(pFirstPhrase+202)->str);}
	else
	{
		char Header[150];
		sprintf(Header,(pFirstPhrase+196)->str,arPlayer[Sender].GetName());
		textprintf_centre_ex(bmpWindow,Font1,330,23,COLOR_1,-1,Header);
	};

	if ((bGold)||(bTrade))
	{
		draw_sprite(bmpWindow,bmpStuff[23],66,340);
		textprintf_centre_ex(bmpWindow,Font1,160,347,COLOR_1,-1,(pFirstPhrase+197)->str);
		draw_sprite(bmpWindow,bmpStuff[23],409,340);
		textprintf_centre_ex(bmpWindow,Font1,503,347,COLOR_1,-1,(pFirstPhrase+198)->str);

	};

	UI_Mask mskTrade[2][18];
	int Adder=0;
	if (bGold)
	{
		Adder+=92;
		int Shift=0;
		if (Mess->Trade.Gem==0)
		{Shift=58;};
		if (Mess->Trade.Gold>0)
		{			
			draw_sprite(bmpWindow,bmpStuff[26],41+Shift,373);
			draw_sprite(bmpWindow,bmpStuff[5],76+Shift,379);
			WObj=mskTrade[0][16].Init(x+70+Shift,y+373,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
			textprintf_centre_ex(bmpWindow,Font1,100+Shift,443,COLOR_2,-1,"%d",Mess->Trade.Gold);
			Shift=116;
		};
		if (Mess->Trade.Gold==0)
		{Shift=58;};
		if (Mess->Trade.Gem>0)
		{			
			draw_sprite(bmpWindow,bmpStuff[26],41+Shift,373);
			draw_sprite(bmpWindow,bmpStuff[6],76+Shift,379);
			WObj=mskTrade[0][17].Init(x+70+Shift,y+373,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
			textprintf_centre_ex(bmpWindow,Font1,100+Shift,443,COLOR_2,-1,"%d",Mess->Trade.Gem);
		};
		if (Mess->Trade1.Gem==0)
		{Shift=58;}
		else
		{Shift=0;};
		if (Mess->Trade1.Gold>0)
		{			
			draw_sprite(bmpWindow,bmpStuff[26],383+Shift,373);
			draw_sprite(bmpWindow,bmpStuff[5],418+Shift,379);
			WObj=mskTrade[1][16].Init(x+412+Shift,y+373,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
			textprintf_centre_ex(bmpWindow,Font1,442+Shift,443,COLOR_2,-1,"%d",Mess->Trade1.Gold);
			Shift=116;
		};
		if (Mess->Trade1.Gold==0)
		{Shift=58;};
		if (Mess->Trade1.Gem>0)
		{			
			draw_sprite(bmpWindow,bmpStuff[26],383+Shift,373);
			draw_sprite(bmpWindow,bmpStuff[6],418+Shift,379);
			WObj=mskTrade[1][17].Init(x+412+Shift,y+373,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
			textprintf_centre_ex(bmpWindow,Font1,442+Shift,443,COLOR_2,-1,"%d",Mess->Trade1.Gem);
		};
	};
	int arTrade[2][16];
	if (bTrade)
	{
		for (int i=0; i<(MaxTrade-MaxTrade%4)/4+1; i++)
		{
			draw_sprite(bmpWindow,bmpStuff[25],40,373+Adder+i*60);
			draw_sprite(bmpWindow,bmpStuff[25],383,373+Adder+i*60);
		};
		Give=0; Take=0;
		for (int i=0; i<16; i++)
		{
			if (Mess->Trade.Stuff[i]>0)
			{
				arTrade[0][Give]=i;
				WObj=mskTrade[0][Give].Init(x+37+(Give%4)*60,y+372+Adder+((Give-Give%4)/4)*60,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
				if (Mess->Type==2)
				{draw_sprite(bmpWindow,(pFirstResource+Mess->Trade.Stuff[i])->BigPic,43+(Give%4)*60,378+Adder+((Give-Give%4)/4)*60);}
				else
				{
					if (i<8)
					{draw_sprite(bmpWindow,bmpStuff[8],43+(Give%4)*60,378+Adder+((Give-Give%4)/4)*60);}
					else
					{draw_sprite(bmpWindow,(pFirstItem+Mess->Trade.Stuff[i])->Pic,43+(Give%4)*60,378+Adder+((Give-Give%4)/4)*60);};
				};
				Give++;
			};
			if (Mess->Trade1.Stuff[i]>0)
			{
				arTrade[1][Take]=i;
				WObj=mskTrade[1][Take].Init(x+380+(Take%4)*60,y+372+Adder+((Take-Take%4)/4)*60,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
				if (Mess->Type==2)
				{draw_sprite(bmpWindow,(pFirstResource+Mess->Trade1.Stuff[i])->BigPic,386+(Take%4)*60,378+Adder+((Take-Take%4)/4)*60);}
				else
				{
					if (i<8)
					{draw_sprite(bmpWindow,bmpStuff[8],386+(Take%4)*60,378+Adder+((Take-Take%4)/4)*60);}
					else
					{draw_sprite(bmpWindow,(pFirstItem+Mess->Trade1.Stuff[i])->Pic,386+(Take%4)*60,378+Adder+((Take-Take%4)/4)*60);};
				};
				Take++;
			};
		};
	};
	if (Mess->Treaty>0)
	{
		draw_sprite(bmpWindow,bmpStuff[24],65,230);
		switch (Mess->Treaty)
		{
		case 1:
			draw_sprite(bmpWindow,bmpStuff[1],70,235);
			break;
		case 2:
		case 4:
			draw_sprite(bmpWindow,bmpStuff[0],70,235);
			break;
		case 3:
			draw_sprite(bmpWindow,bmpStuff[2],70,235);
			break;
		};
	};
	BITMAP *bmpAns[2];
	UI_Mask mskAns[2];
	int MaxAns;	
	if ((Mess->Type==4)||(Mess->Type==5)||(Mess->Type==9)||(Mess->Treaty==1)||(Mess->Treaty==4))
	{
		MaxAns=1;
		textprintf_centre_ex(bmpWindow,Font1,330,Height-35,COLOR_2,-1,"%s",(pFirstPhrase+199)->str);
		bmpAns[0]=create_bitmap(220,20);
		rectfill(bmpAns[0],0,0,220,20,COLOR_1);
		textprintf_centre_ex(bmpAns[0],Font1,110,3,makecol(220,220,0),-1,"%s",(pFirstPhrase+199)->str);	
		WObj=mskAns[0].Init(x+220,y+Height-38,1,bmpAns[0],bmpAns[0],WObj,2);
	}
	else
	{
		MaxAns=2;		
		bmpAns[0]=create_bitmap(220,20);
		rectfill(bmpAns[0],0,0,220,20,COLOR_1);		
		WObj=mskAns[0].Init(x+220,y+Height-50,1,bmpAns[0],bmpAns[0],WObj,2);		
		bmpAns[1]=create_bitmap(220,20);
		rectfill(bmpAns[1],0,0,220,20,COLOR_1);		
		WObj=mskAns[1].Init(x+220,y+Height-30,1,bmpAns[1],bmpAns[1],WObj,2);
		switch (Mess->Type)
		{
		case 6:
			textprintf_centre_ex(bmpWindow,Font1,330,Height-47,COLOR_2,-1,"%s",(pFirstPhrase+200)->str);
			textprintf_centre_ex(bmpAns[0],Font1,110,3,makecol(220,220,0),-1,"%s",(pFirstPhrase+200)->str);
			textprintf_centre_ex(bmpWindow,Font1,330,Height-27,COLOR_2,-1,"%s",(pFirstPhrase+201)->str);
			textprintf_centre_ex(bmpAns[1],Font1,110,3,makecol(220,220,0),-1,"%s",(pFirstPhrase+201)->str);
			break;
		case 7:
			textprintf_centre_ex(bmpWindow,Font1,330,Height-47,COLOR_2,-1,"%s",(pFirstPhrase+391)->str);
			textprintf_centre_ex(bmpAns[0],Font1,110,3,makecol(220,220,0),-1,"%s",(pFirstPhrase+391)->str);
			textprintf_centre_ex(bmpWindow,Font1,330,Height-27,COLOR_2,-1,"%s",(pFirstPhrase+392)->str);
			textprintf_centre_ex(bmpAns[1],Font1,110,3,makecol(220,220,0),-1,"%s",(pFirstPhrase+392)->str);
			break;
		case 8:
			textprintf_centre_ex(bmpWindow,Font1,330,Height-47,COLOR_2,-1,"%s",(pFirstPhrase+393)->str);
			textprintf_centre_ex(bmpAns[0],Font1,110,3,makecol(220,220,0),-1,"%s",(pFirstPhrase+393)->str);
			textprintf_centre_ex(bmpWindow,Font1,330,Height-27,COLOR_2,-1,"%s",(pFirstPhrase+392)->str);
			textprintf_centre_ex(bmpAns[1],Font1,110,3,makecol(220,220,0),-1,"%s",(pFirstPhrase+392)->str);
			break;
		default:		
			textprintf_centre_ex(bmpWindow,Font1,330,Height-47,COLOR_2,-1,"%s",(pFirstPhrase+194)->str);
			textprintf_centre_ex(bmpAns[0],Font1,110,3,makecol(220,220,0),-1,"%s",(pFirstPhrase+194)->str);
			textprintf_centre_ex(bmpWindow,Font1,330,Height-27,COLOR_2,-1,"%s",(pFirstPhrase+195)->str);
			textprintf_centre_ex(bmpAns[1],Font1,110,3,makecol(220,220,0),-1,"%s",(pFirstPhrase+195)->str);
			break;
		};
	};
	int Check;
	Result=-1;
	while (Result<0)
	{
		for (int i=0; i<MaxAns; i++)
		{
			Check=mskAns[i].Check();
			if ((Check==30)||(Check==40))
			{Result=i;};
		};
		for (int i=0; i<Give; i++)
		{
			Check=mskTrade[0][i].Check();			
			if (Check==2)
			{
				if (Mess->Type==2)					
				{
					bHint=true;
					MakeStringHint((pFirstResource+Mess->Trade.Stuff[arTrade[0][i]])->sName);
				}
				else
				{
					if (arTrade[0][i]<8)
					{
						bHint=true;
						MakeStringHint((pFirstProvince+Mess->Trade.Stuff[arTrade[0][i]])->sName);
					}
					else
					{
						bHint=true;
						MakeItemHint(Mess->Trade.Stuff[arTrade[0][i]],Mess->arDurability[arTrade[0][i]-8],NULL,312,(pFirstItem+Mess->Trade.Stuff[arTrade[0][i]])->iGoldCost,(pFirstItem+Mess->Trade.Stuff[arTrade[0][i]])->iGemCost);
						
					};						
				};
			};
			if (Check==40)
			{
				if (Mess->Type==2)					
				{
					WindowResourceInfo(Mess->Trade.Stuff[arTrade[0][i]]);					
				}
				else
				{
					if (arTrade[0][i]<8)
					{
					//	MakeStringHint((pFirstProvince+Mess->Trade.Stuff[arTrade[0][i]])->sName);
					}
					else
					{
						WindowItemInfo(Mess->Trade.Stuff[arTrade[0][i]],Mess->arDurability[arTrade[0][i]-8]);
					};						
				};
			};
		};
		for (int i=0; i<Take; i++)
		{
			Check=mskTrade[1][i].Check();
			if (Check==2)
			{
				if (Mess->Type==2)					
				{
					bHint=true;
					MakeStringHint((pFirstResource+Mess->Trade1.Stuff[arTrade[1][i]])->sName);
				}
				else
				{					
					bHint=true;
					MakeStringHint((pFirstProvince+Mess->Trade1.Stuff[arTrade[1][i]])->sName);
				};
			};
		};
		if (Mess->Trade.Gold>0)
		{
			Check=mskTrade[0][16].Check();
			if (Check==2)
			{
				if (Mess->Type==2)					
				{
					bHint=true;
					MakeStringHint(187);
				}
				else
				{
					bHint=true;
					MakeStringHint(189);
				};
			};
		};
		if (Mess->Trade.Gem>0)
		{			
			Check=mskTrade[0][17].Check();
			if (Check==2)
			{
				if (Mess->Type==2)					
				{
					bHint=true;
					MakeStringHint(188);
				}
				else
				{
					bHint=true;
					MakeStringHint(190);
				};
			};
		};
		if (Mess->Trade1.Gold>0)
		{
			Check=mskTrade[1][16].Check();
			if (Check==2)
			{
				if (Mess->Type==2)					
				{
					bHint=true;
					MakeStringHint(187);
				}
				else
				{
					bHint=true;
					MakeStringHint(189);
				};
			};
		};
		if (Mess->Trade1.Gem>0)
		{			
			Check=mskTrade[1][17].Check();
			if (Check==2)
			{
				if (Mess->Type==2)					
				{
					bHint=true;
					MakeStringHint(188);
				}
				else
				{
					bHint=true;
					MakeStringHint(190);
				};
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
//	arPlayer[Player].MessageEntry(Sender,1-Result);
	if (Result==0)
	{
		if ((Mess->Type==8)&&(CurGame.iGameType==2)&&(arPlayer[Sender].ipProfile==3)&&(Campaign.arDialog[103]>0))
		{Campaign.arItems[4]++;};
		arPlayer[Player].AcceptDeal(Sender);
		if ((arPlayer[Sender].Control==1)&&(MaxAns>1))
		{arPlayer[Player].Politic[Sender].Answer=2;};
	}
	else
	{
		if (arPlayer[Sender].Control==1)
		{arPlayer[Player].Politic[Sender].Answer=-2;};
		switch (Mess->Treaty)
		{
		case 2:
		case 3:
			arPlayer[Sender].Politic[Player].Attitude-=2;
			break;
		case 0:
			if (((Mess->Type==2)||(Mess->Type==3))&&(Random(10)<5))
			{arPlayer[Sender].Politic[Player].Attitude--;};
			break;
		};
		if ((Mess->Type==8)&&(CurGame.iGameType==2)&&(arPlayer[Sender].ipProfile==3)&&(Campaign.arDialog[103]>0))
		{
			Campaign.arCampPlayer[3].iTreaty=0;
			Campaign.arDialog[103]=0;
			Campaign.arDialog[104]=1;
			arPlayer[Sender].Politic[Player].Attitude-=400;
		};
		arPlayer[Player].CancelDeal(Sender);
	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,screen1,0,0,0,0,1024,768);
	blit(bmpDarkScreen,screen,0,0,0,0,1024,768);
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	for (int i=0; i<MaxAns; i++)
	{destroy_bitmap(bmpAns[i]);};
	iAntisleep=iAntisleepDelay;
	return Result;
};

void DrawTownFlag(int Col1, int Col2, int Sign, int x, int y, BITMAP *Target)
{	
	set_trans_blender(arHerbColors[Col1][0],arHerbColors[Col1][1],arHerbColors[Col1][2],0);			
	draw_lit_sprite(Target,bmpTownFlag[0][1],x,y,255);	
	set_trans_blender(arHerbColors[Col2][0],arHerbColors[Col2][1],arHerbColors[Col2][2],0);
	draw_lit_sprite(Target,bmpHerbSignS[Sign],x+50,y+15,255);	
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(Target,bmpTownFlag[0][1],x,y);	
};


//***************************************************
//*** Таблица рекордов ******************************
//***************************************************
void WindowHighScore()
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_HighScore->w/2;
	int y=384-bmpGUI_HighScore->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_HighScore->w,bmpGUI_HighScore->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_HighScore,bmpWindow,0,0,0,0,bmpGUI_HighScore->w,bmpGUI_HighScore->h);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+318,y+661,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,363,24,COLOR_1,-1,"%s",(pFirstPhrase+299)->str);
	for (int i=0; i<10; i++)
	{
		textprintf_centre_ex(bmpWindow,Font1,388,67+i*60,COLOR_1,-1,"%s",arRecords[i].sName);
		textprintf_centre_ex(bmpWindow,Font1,388,95+i*60,COLOR_1,-1,"%d",arRecords[i].Score);
		draw_sprite(bmpWindow,bmpSmallAvatar[arRecords[i].ipPortrait],93,62+i*60);
		DrawHerb(arRecords[i].Col1,arRecords[i].Col2,arRecords[i].Sign,152,70+i*60,bmpWindow);
		DrawHerb(arRecords[i].Col1,arRecords[i].Col2,arRecords[i].Sign,606,70+i*60,bmpWindow);
		for (int j=0; j<8; j++)
		{
			if (arRecords[i].arMedal[j]>0)
			{
				if (j<4)
				{draw_sprite(bmpWindow,(pFirstMedal+arRecords[i].arMedal[j])->SmallPic,208+j*13,91+i*60);}
				else
				{draw_sprite(bmpWindow,(pFirstMedal+arRecords[i].arMedal[j])->SmallPic,506+(j-4)*13,91+i*60);};
			};
		};
	};
	while (!buttExit.Check())
	{		
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Таблица рекордов в кампании *******************
//***************************************************
void WindowCampHighScore()
{
	iAntisleep=iAntisleepDelay;
	if (arCampRecords[0].Score==-30000)
	{
		(pFirstDialog+1236)->Show(1,1,0,0,NULL,NULL);
		return;
	};
	int x=512-bmpGUI_HighScore->w/2;
	int y=384-bmpGUI_HighScore->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_HighScore->w,bmpGUI_HighScore->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_HighScore,bmpWindow,0,0,0,0,bmpGUI_HighScore->w,bmpGUI_HighScore->h);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+318,y+661,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,363,24,COLOR_1,-1,"%s",(pFirstPhrase+482)->str);
	for (int i=0; i<10; i++)
	{
		if (arCampRecords[i].Score>-30000)
		{
			textprintf_centre_ex(bmpWindow,Font1,388,67+i*60,COLOR_1,-1,"%s",arCampRecords[i].sName);
			textprintf_centre_ex(bmpWindow,Font1,388,95+i*60,COLOR_1,-1,"%d",arCampRecords[i].Score);
			switch (arCampRecords[i].ipPortrait)
			{
			case 0:
			case 1:
				draw_sprite(bmpWindow,bmpStuff[63],93,62+i*60);
				break;
			case 2:
				draw_sprite(bmpWindow,bmpStuff[58],93,62+i*60);
				break;
			case 3:
				draw_sprite(bmpWindow,bmpSmallAvatar[15],93,62+i*60);
				break;
			case 4:
				draw_sprite(bmpWindow,bmpSmallAvatar[2],93,62+i*60);
				break;
			case 5:
			case 6:
				draw_sprite(bmpWindow,bmpStuff[60],93,62+i*60);
				break;
			case 7:
				draw_sprite(bmpWindow,bmpStuff[59],93,62+i*60);
				break;
			case 8:
				draw_sprite(bmpWindow,bmpSmallAvatar[18],93,62+i*60);
				break;
			case 9:
				draw_sprite(bmpWindow,bmpSmallAvatar[6],93,62+i*60);
				break;
			case 10:
				draw_sprite(bmpWindow,bmpStuff[62],93,62+i*60);
				break;
			case 11:
				draw_sprite(bmpWindow,bmpStuff[64],93,62+i*60);
				break;
			};
			DrawHerb(arCampRecords[i].Col1,arCampRecords[i].Col2,arCampRecords[i].Sign,152,70+i*60,bmpWindow);
			DrawHerb(arCampRecords[i].Col1,arCampRecords[i].Col2,arCampRecords[i].Sign,606,70+i*60,bmpWindow);
		}
		else
		{
			textprintf_centre_ex(bmpWindow,Font1,388,67+i*60,makecol(100,100,100),-1,(pFirstPhrase+481)->str);
			draw_sprite(bmpWindow,bmpSmallAvatar[16],93,62+i*60);			
			draw_sprite(bmpWindow,bmpHerbRandom,152,70+i*60);
			draw_sprite(bmpWindow,bmpHerbRandom,606,70+i*60);
		};
		
	};
	while (!buttExit.Check())
	{		
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

bool WindowEnter (char * Str, int Phrase, int Pic)
{
	iAntisleep=iAntisleepDelay;
	bool Result=false;
	int x=512-bmpGUI_EnterWindow->w/2;
	int y=384-bmpGUI_EnterWindow->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);	
	BITMAP *bmpWindow=create_bitmap(bmpGUI_EnterWindow->w,bmpGUI_EnterWindow->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	if (Pic>=0)
	{draw_sprite(bmpWindow,bmpAvatar[Pic],87,19);}
	else
	{draw_sprite(bmpWindow,bmpGUI_HomeGarrison,-89,8);};
	draw_sprite(bmpWindow,bmpGUI_EnterWindow,0,0);
//	blit(bmpGUI_Profile,bmpWindow,0,0,0,0,bmpGUI_EnterWindow->w,bmpGUI_EnterWindow->h);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+116,y+237,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	UI_Input inpTxt;
	inpTxt.bCenter=true;
	WObj=inpTxt.Init(x+55,y+209,2,20,2,COLOR_2,Font1,WObj);	
	inpTxt.Activate(1);

	textprintf_centre_ex(bmpWindow,Font1,152,183,COLOR_1,-1,(pFirstPhrase+Phrase)->str);

	while (!buttExit.Check())
	{	
		inpTxt.Check(true);
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,screen1,0,0,0,0,1024,768);
	blit(bmpBack,screen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	sprintf(Str,inpTxt.Text);
	Result=(inpTxt.Cursor>0);
	iAntisleep=iAntisleepDelay;
	return Result;
};

void WindowNextTurn (int Player)
{	
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_TurnWindow->w/2;
	int y=384-bmpGUI_TurnWindow->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);	
	BITMAP *bmpWindow=create_bitmap(bmpGUI_TurnWindow->w,bmpGUI_TurnWindow->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);	
	draw_sprite(bmpWindow,arPlayer[Player].GetAvatar(),87,19);
	draw_sprite(bmpWindow,bmpGUI_TurnWindow,0,0);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+116,y+237,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	textprintf_centre_ex(bmpWindow,Font1,152,183,COLOR_1,-1,arPlayer[Player].GetName());
	textprintf_centre_ex(bmpWindow,Font1,152,212,COLOR_1,-1,(pFirstPhrase+448)->str);
	draw_sprite(bmpWindow,bmpHerb[arPlayer[Player].iColor],42,36);
	draw_sprite(bmpWindow,bmpHerb[arPlayer[Player].iColor],231,36);
	

	while (!buttExit.Check())
	{
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,screen1,0,0,0,0,1024,768);
	blit(bmpBack,screen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

void WindowHerb (clHumanProfile *Hum)
{	
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_HerbWindow->w/2;
	int y=384-bmpGUI_HerbWindow->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);	
	BITMAP *bmpWindow=create_bitmap(bmpGUI_HerbWindow->w,bmpGUI_HerbWindow->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);		
	blit(bmpGUI_HerbWindow,bmpWindow,0,0,0,0,bmpGUI_HerbWindow->w,bmpGUI_HerbWindow->h);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+118,y+102,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	UI_Button1 buttHerb[3][2];	
	WObj=buttHerb[0][0].Init(x+24,y+55,1,3,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],WObj);
	WObj=buttHerb[0][1].Init(x+75,y+55,1,3,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],WObj);
	WObj=buttHerb[1][0].Init(x+100,y+55,1,3,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],WObj);
	WObj=buttHerb[1][1].Init(x+183,y+55,1,3,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],WObj);
	WObj=buttHerb[2][0].Init(x+208,y+55,1,3,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],WObj);
	WObj=buttHerb[2][1].Init(x+259,y+55,1,3,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],WObj);

	textprintf_centre_ex(bmpWindow,Font1,152,27,COLOR_1,-1,(pFirstPhrase+332)->str);

	bool bRepeat=true, bRedraw=true;
	while (bRepeat)
	{
		rectfill(bmpWindow,48,60,68,80,makecol(arHerbColors[Hum->iColor1][0],arHerbColors[Hum->iColor1][1],arHerbColors[Hum->iColor1][2]));
		rectfill(bmpWindow,232,60,252,80,makecol(arHerbColors[Hum->iColor2][0],arHerbColors[Hum->iColor2][1],arHerbColors[Hum->iColor2][2]));
		DrawHerb(Hum->iColor1,Hum->iColor2,Hum->iSign,135,54,bmpWindow);
		bRedraw=true;
		while (bRedraw)
		{	
			if (buttExit.Check())
			{
				bRepeat=false;
				bRedraw=false;
			};
			if (buttHerb[0][0].Check())
			{				
				if (Hum->iColor1==0)
				{Hum->iColor1=MAX_HERB_COLOR-1;}
				else
				{Hum->iColor1--;};
				bRedraw=false;
			};
			if (buttHerb[0][1].Check())
			{				
				if (Hum->iColor1==MAX_HERB_COLOR-1)
				{Hum->iColor1=0;}
				else
				{Hum->iColor1++;};
				bRedraw=false;
			};

			if (buttHerb[1][0].Check())
			{				
				if (Hum->iSign==0)
				{Hum->iSign=iMaxHerbSign-1;}
				else
				{Hum->iSign--;};
				bRedraw=false;
			};
			if (buttHerb[1][1].Check())
			{				
				if (Hum->iSign==iMaxHerbSign-1)
				{Hum->iSign=0;}
				else
				{Hum->iSign++;};
				bRedraw=false;
			};

			if (buttHerb[2][0].Check())
			{				
				if (Hum->iColor2==0)
				{Hum->iColor2=MAX_HERB_COLOR-1;}
				else
				{Hum->iColor2--;};
				bRedraw=false;
			};
			if (buttHerb[2][1].Check())
			{				
				if (Hum->iColor2==MAX_HERB_COLOR-1)
				{Hum->iColor2=0;}
				else
				{Hum->iColor2++;};
				bRedraw=false;
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
	};
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,screen1,0,0,0,0,1024,768);
	blit(bmpBack,screen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Окно победы в кампании ************************
//***************************************************
void WindowCampaignVictory()
{
	iAntisleep=iAntisleepDelay;
	if ((Campaign.iActiveShard<0)&&(Campaign.bDefender))
	{
		ScreenEndGame(1,true,true,false);
		//Передача данных в кампанию
		Campaign.arCampPlayer[0].iMapScore+=arPlayer[1].arRes[0];
	//	Campaign.arCampPlayer[0].iScore+=arPlayer[1].arRes[0];
		Campaign.arCampPlayer[0].iKarma+=(arPlayer[1].iKarma-Campaign.arCampPlayer[0].iKarma)/10;
		for (int i=2; i<=CurGame.iMaxPlayers; i++)
		{
			if (arPlayer[i].Control==2)
			{
				Campaign.arCampPlayer[arPlayer[i].ipProfile].arAttitude[0]+=(arPlayer[i].Politic[1].Attitude-Campaign.arCampPlayer[arPlayer[i].ipProfile].arAttitude[0])/3;
			};
		};
		return;
	};
	ScreenEndGame(1,true,false,false);
	int x=512-bmpGUI_CampVictory->w/2;
	int y=384-bmpGUI_CampVictory->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_CampVictory->w,bmpGUI_CampVictory->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_CampVictory,bmpWindow,0,0,0,0,bmpGUI_CampVictory->w,bmpGUI_CampVictory->h);
	
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+164,y+431,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;

	UI_Button1 buttStat;
	WObj=buttStat.Init(x+318,y+377,1,3,bmpButton[87],bmpButton[88],bmpButton[90],bmpButton[89],WObj);
	buttStat.Activate(1);
	buttStat.iHint=99;
	dcShard *SH=NULL;
	if (Campaign.iActiveShard>=0)
	{

		SH=Campaign.GetActiveShard();
		textprintf_centre_ex(bmpWindow,Font1,200,18,COLOR_1,-1,(pFirstPhrase+344)->str,SH->sName);
	}
	else
	{		
		textprintf_centre_ex(bmpWindow,Font1,200,18,COLOR_1,-1,(pFirstPhrase+439)->str,arPlayer[2].GetName());
		int Lvl=4+GetWorldSize(abs(Campaign.iActiveShard))/20;
		if (Lvl>8)
		{Lvl=8;};
		pFirstShard=new dcShard(pFirstShard,Random(6),4,Lvl);
		SH=pFirstShard;
		int Count=0;
		ClearRoll();
		for (int i=1; i<=iMaxInnerBld; i++)
		{
			if ((GetBuildingStatus(i,0)==1)&&(IsBuildingFree(i)))
			{
				RollerN[Count]=i;
				RollerP[Count]=10;
				Count++;
			};			
		};
		for (int i=0; i<GetMin(4,Count); i++)
		{
			SH->arBuilding[i]=Roll(true);
		};
		SH->FillBonus();
		SH->ipPlayer=0;
	};
//	while (SH->Num!=Campaign.iActiveShard)
//	{SH=SH->pNext;};

	
	textprintf_centre_ex(bmpWindow,Font1,200,170,COLOR_1,-1,(pFirstPhrase+345)->str);
	textprintf_centre_ex(bmpWindow,Font1,200,278,COLOR_1,-1,(pFirstPhrase+346)->str);
	textprintf_centre_ex(bmpWindow,Font1,160,386,COLOR_1,-1,"%s: %d",(pFirstPhrase+347)->str,arPlayer[1].arRes[0]);

	UI_Mask mskBld[4], mskBonus[4];
	int MaxBld=0, MaxBonus=0;
	for (int i=0; i<4; i++)
	{
		if (SH->arBuilding[i]>0)
		{MaxBld++;};
		if (SH->arBonus[i][0]>0)
		{MaxBonus++;};
	};
	for (int i=0; i<MaxBld; i++)
	{
		draw_sprite(bmpWindow,bmpGUI_SpellSlot,200-MaxBld*30-(MaxBld-1)*11+i*82,196);
		draw_sprite(bmpWindow,(pFirstInnerBld+SH->arBuilding[i])->Pic,205-MaxBld*30-(MaxBld-1)*11+i*82,201);
		WObj=mskBld[i].Init(x+199-MaxBld*30-(MaxBld-1)*11+i*82,y+195,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
	};
	for (int i=0; i<MaxBonus; i++)
	{
		draw_sprite(bmpWindow,bmpGUI_SpellSlot,200-MaxBonus*30-(MaxBonus-1)*11+i*82,304);
		switch (SH->arBonus[i][0])
		{
			case 1:
			case 2:
			case 3:
			case 4:
				draw_sprite(bmpWindow,bmpShardBonus[SH->arBonus[i][0]-1],205-MaxBonus*30-(MaxBonus-1)*11+i*82,309);
				break;
			case 6:
				draw_sprite(bmpWindow,bmpShardBonus[4],205-MaxBonus*30-(MaxBonus-1)*11+i*82,309);
				break;
			case 5:
				draw_sprite(bmpWindow,(pFirstItem+SH->arBonus[i][1])->Pic,205-MaxBonus*30-(MaxBonus-1)*11+i*82,309);
				break;
			case 7:
				draw_sprite(bmpWindow,(pFirstInnerBld+SH->arBonus[i][1])->Pic,205-MaxBonus*30-(MaxBonus-1)*11+i*82,309);
				break;
			case 8:
				draw_sprite(bmpWindow,(pFirstResource+SH->arBonus[i][1])->BigPic,205-MaxBonus*30-(MaxBonus-1)*11+i*82,309);
				break;
			case 9:
				draw_sprite(bmpWindow,(pFirstDefender+SH->arBonus[i][1])->Pic,205-MaxBonus*30-(MaxBonus-1)*11+i*82,309);
				break;
		};		
		WObj=mskBonus[i].Init(x+199-MaxBonus*30-(MaxBonus-1)*11+i*82,y+303,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
	};

	int Check;
	while (!buttExit.Check())
	{
		for (int i=0; i<MaxBld; i++)
		{
			Check=mskBld[i].Check();
			if (Check>1)
			{
				MakeInnBldHint(SH->arBuilding[i],-1);
				bHint=true;
			};
		};
		for (int i=0; i<MaxBonus; i++)
		{
			Check=mskBonus[i].Check();
			if (Check>1)
			{
				char Txt[200];
				switch(SH->arBonus[i][0])
				{
				case 1:
				case 2:
				case 3:
				case 4:
					sprintf(Txt,"%s %d",(pFirstShardBonus+SH->arBonus[i][0])->sName,SH->arBonus[i][1]);
					break;
				case 5:
					sprintf(Txt,"%s %s",(pFirstShardBonus+SH->arBonus[i][0])->sName,(pFirstItem+SH->arBonus[i][1])->sName);
					break;
				case 6:
					sprintf(Txt,"%s %s",(pFirstShardBonus+SH->arBonus[i][0])->sName,(pFirstUnitType+SH->arBonus[i][1])->sName);
					break;
				case 7:
					sprintf(Txt,"%s %s",(pFirstShardBonus+SH->arBonus[i][0])->sName,(pFirstInnerBld+SH->arBonus[i][1])->sName);
					break;
				case 8:
					sprintf(Txt,"%s %s (10~)",(pFirstShardBonus+SH->arBonus[i][0])->sName,(pFirstResource+SH->arBonus[i][1])->sName);
					break;
				case 9:
					sprintf(Txt,"%s %s",(pFirstShardBonus+SH->arBonus[i][0])->sName,(pFirstDefender+SH->arBonus[i][1])->sName);
					break;
				};
				MakeStringHint(Txt);
				bHint=true;
			};
		};

		if (buttStat.Check())
		{ScreenEndGame(1,true,true,false);};
		
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};

//Передача данных в кампанию
	Campaign.arCampPlayer[0].iMapScore+=arPlayer[1].arRes[0];
//	Campaign.arCampPlayer[0].iScore+=arPlayer[1].arRes[0];
	Campaign.arCampPlayer[0].iKarma+=(arPlayer[1].iKarma-Campaign.arCampPlayer[0].iKarma)/10;
	for (int i=2; i<=CurGame.iMaxPlayers; i++)
	{
		if (arPlayer[i].Control==2)
		{
			Campaign.arCampPlayer[arPlayer[i].ipProfile].arAttitude[0]+=(arPlayer[i].Politic[1].Attitude-Campaign.arCampPlayer[arPlayer[i].ipProfile].arAttitude[0])/3;
		};
	};



	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};

void ChangeMusicVolume()
{
	trSound[MainTrack].iVolume=iMusicVolume;
	trSound[MainTrack].fVolume=iMusicVolume;
	if (trSound[MainTrack].fFading>0)
	{
		trSound[MainTrack].fFading=MAX_FADE_SPEED*iMusicVolume;
		trSound[MainTrack].fFading/=255;
	};
	if (trSound[MainTrack].fFading<0)
	{
		trSound[MainTrack].fFading=-MAX_FADE_SPEED*iMusicVolume;
		trSound[MainTrack].fFading/=255;
	};
};


//***************************************************
//*** Окно редактирования профиля игрока ************
//***************************************************
bool WindowProfile (int Index, bool bList)
{
	iAntisleep=iAntisleepDelay;
	bool bResult=false;
	int x=512-bmpGUI_Profile->w/2;
	int y=384-bmpGUI_Profile->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * objGUI=new G_Object(NULL,0,0,0,bmpDarkScreen);	
	BITMAP *bmpWindow=create_bitmap(bmpGUI_Profile->w,bmpGUI_Profile->h);
	objGUI=new G_Object(objGUI,x,y,1,bmpWindow);
	

	UI_Button1 buttList;
	objGUI=buttList.Init(x+13,y+374,1,2,bmpButton[87],bmpButton[88],bmpButton[90],bmpButton[89],objGUI);
	buttList.Activate(1);
	buttList.iHint=349;
	
	UI_Button1 buttExit;
	objGUI=buttExit.Init(x+211,y+374,1,2,bmpButton[305],bmpButton[306],bmpButton[307],bmpButton[308],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;

	UI_Button1 buttHerbL;
	objGUI=buttHerbL.Init(x+89,y+141,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	buttHerbL.Activate(1);
	
	UI_Button1 buttHerbR;
	objGUI=buttHerbR.Init(x+172,y+141,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
	buttHerbR.Activate(1);

	UI_Button1 buttCol1L;
	objGUI=buttCol1L.Init(x+11,y+141,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	buttCol1L.Activate(1);
	
	UI_Button1 buttCol1R;
	objGUI=buttCol1R.Init(x+65,y+141,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
	buttCol1R.Activate(1);

	UI_Button1 buttCol2L;
	objGUI=buttCol2L.Init(x+192,y+141,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	buttCol2L.Activate(1);
	
	UI_Button1 buttCol2R;
	objGUI=buttCol2R.Init(x+246,y+141,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
	buttCol2R.Activate(1);

	UI_Button1 buttDiffL;
	objGUI=buttDiffL.Init(x+49,y+181,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	buttDiffL.Activate(1);
	
	UI_Button1 buttDiffR;
	objGUI=buttDiffR.Init(x+206,y+181,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
	buttDiffR.Activate(1);

	UI_Button1 buttFaceL;
	objGUI=buttFaceL.Init(x+44,y+275,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	buttFaceL.Activate(1);
	if (bBetha)
	{buttFaceL.Activate(0);};
	
	UI_Button1 buttFaceR;
	objGUI=buttFaceR.Init(x+214,y+275,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
	buttFaceR.Activate(1);
	if (bBetha)
	{buttFaceR.Activate(0);};

	if (!bList)
	{
		buttList.Activate(0);
		buttDiffL.Activate(0);
		buttDiffR.Activate(0);
	};

	UI_Mask1 mskName;
	mskName.Init(x+37,y+52,202,29,1);

	UI_Mask1 mskLand;
	mskLand.Init(x+37,y+90,202,29,1);

	UI_Mask1 mskDiff;
	mskDiff.Init(x+68,y+184,140,25,1);

	UI_Input inpName;
	inpName.bCenter=true;
	objGUI=inpName.Init(x+50,y+56,0,19,2,COLOR_2,Font1,objGUI);
	
	UI_Input inpLand;
	inpLand.bCenter=true;
	objGUI=inpLand.Init(x+50,y+94,0,19,2,COLOR_2,Font1,objGUI);	

	bool bRepeat=true, bClick=true, bFirst=true, bSave=false, bName=true, bLand=true;
//	srand(iSeed);
//	int Color1=Random(16), Color2=Random(16), Sign=Random(iMaxHerbSign), Face=17, Diff=0;
	/*int Color1=PL->iColor1, Color2=PL->iColor2, Sign=PL->iSign, Face=PL->ipAvatar, Diff=PL->iDifficulty;
	DrawHerb(Color1, Color2, Sign, 124, 140, bmpWindow);
	DrawHerb(Color1, Color2, Sign, 22, 220, bmpWindow);
	DrawHerb(Color1, Color2, Sign, 217, 220, bmpWindow);
	DrawTownFlag(Color1, Color2, Sign, 219-50+15, 6, bmpWindow);
	DrawTownFlag(Color1, Color2, Sign, 17-50+3, 6, bmpWindow);
	rectfill(bmpWindow,38,147,57,166,makecol(arHerbColors[Color1][0],arHerbColors[Color1][1],arHerbColors[Color1][2]));
	rectfill(bmpWindow,219,147,238,166,makecol(arHerbColors[Color2][0],arHerbColors[Color2][1],arHerbColors[Color2][2]));*/
	blit(bmpGUI_Profile,bmpWindow,0,0,0,0,bmpGUI_Profile->w,bmpGUI_Profile->h);
//	textprintf_centre_ex(bmpWindow,Font1,140,191,COLOR_1,-1,"%s",(pFirstPhrase+219+arHumans[Index].iDiff)->str);			
	if (arHumans[Index].iFileIndex==-1)
	{textprintf_centre_ex(bmpWindow,Font1,141,25,COLOR_1,-1,(pFirstPhrase+212)->str);}
	else
	{textprintf_centre_ex(bmpWindow,Font1,141,25,COLOR_1,-1,(pFirstPhrase+348)->str);};
	bClick=true;
	textprintf_centre_ex(bmpWindow,Font1,140,191,COLOR_1,-1,"%s",arDifficulty[arHumans[Index].iDifficulty].sName);
	draw_sprite(bmpWindow,bmpAvatar[arHumans[Index].ipAvatar],73,215);

	sprintf(inpName.Text,"                   ");
//	sprintf(inpName.Text,(pFirstPhrase+214)->str);
	sprintf(inpName.Text,arHumans[Index].sName);
	inpName.Length=0;
	while (arHumans[Index].sName[inpName.Length]!='\0')
	{inpName.Length++;};

//	inpName.Length=sizeof PL->sName;
	inpName.Cursor=inpName.Length;
	inpName.Show();

	sprintf(inpLand.Text,"                   ");
//	sprintf(inpLand.Text,(pFirstPhrase+215)->str);	
	sprintf(inpLand.Text,arHumans[Index].sLand);
	
	inpLand.Length=0;
	while (arHumans[Index].sLand[inpLand.Length]!='\0')
	{inpLand.Length++;};

	inpLand.Cursor=inpLand.Length;
	inpLand.Show();
	if (arHumans[Index].iDifficulty==6){buttDiffR.Activate(0);};
	if (arHumans[Index].iDifficulty==0){buttDiffL.Activate(0);};
	int Check;	
	while (bRepeat)
	{		
		if (buttExit.Check())
		{
			bRepeat=false;
		//	bResult=false;
		//	bSave=false;
		};
		if (bList)
		{
		if (buttList.Check())
		{
			bRepeat=false;
			bResult=true;		
		};
		};
		if (buttCol1L.Check())
		{
			if (arHumans[Index].iColor1>0)
			{arHumans[Index].iColor1--;}
			else
			{arHumans[Index].iColor1=15;};
	//		rectfill(bmpWindow,38,147,57,166,makecol(arHerbColors[Color1][0],arHerbColors[Color1][1],arHerbColors[Color1][2]));
			bClick=true;
		};

		if (buttCol1R.Check())
		{
			if (arHumans[Index].iColor1<15)
			{arHumans[Index].iColor1++;}
			else
			{arHumans[Index].iColor1=0;};
	//		rectfill(bmpWindow,38,147,57,166,makecol(arHerbColors[Color1][0],arHerbColors[Color1][1],arHerbColors[Color1][2]));			
			bClick=true;
		};

		if (buttCol2L.Check())
		{
			if (arHumans[Index].iColor2>0)
			{arHumans[Index].iColor2--;}
			else
			{arHumans[Index].iColor2=15;};
	//		rectfill(bmpWindow,219,147,238,166,makecol(arHerbColors[Color2][0],arHerbColors[Color2][1],arHerbColors[Color2][2]));			
			bClick=true;
		};

		if (buttCol2R.Check())
		{
			if (arHumans[Index].iColor2<15)
			{arHumans[Index].iColor2++;}
			else
			{arHumans[Index].iColor2=0;};
	//		rectfill(bmpWindow,219,147,238,166,makecol(arHerbColors[Color2][0],arHerbColors[Color2][1],arHerbColors[Color2][2]));			
			bClick=true;
		};		

		if (buttHerbL.Check())
		{
			if (arHumans[Index].iSign>0)
			{arHumans[Index].iSign--;}
			else
			{arHumans[Index].iSign=iMaxHerbSign-1;};			
			bClick=true;
		};

		if (buttHerbR.Check())
		{
			if (arHumans[Index].iSign<iMaxHerbSign-1)
			{arHumans[Index].iSign++;}
			else
			{arHumans[Index].iSign=0;};			
			bClick=true;
		};

		if (bList)
		{

		if (buttDiffL.Check())
		{
			if (arHumans[Index].iDifficulty>0)
			{
				buttDiffR.Activate(1);
				arHumans[Index].iDifficulty--;
				if (arHumans[Index].iDifficulty==0)
				{buttDiffL.Activate(0);};
			};
		/*	else
			{Diff=6;};*/
			blit(bmpGUI_Profile,bmpWindow,78,186,78,186,126,20);
			textprintf_centre_ex(bmpWindow,Font1,140,191,COLOR_1,-1,"%s",arDifficulty[arHumans[Index].iDifficulty].sName);
			bClick=true;
		};
		if (buttDiffR.Check())
		{
			if (arHumans[Index].iDifficulty<6)
			{
				buttDiffL.Activate(1);
				arHumans[Index].iDifficulty++;
				if (arHumans[Index].iDifficulty==6)
				{buttDiffR.Activate(0);};
			};
			/*else
			{Diff=0;};*/
			blit(bmpGUI_Profile,bmpWindow,78,186,78,186,126,20);
			textprintf_centre_ex(bmpWindow,Font1,140,191,COLOR_1,-1,"%s",arDifficulty[arHumans[Index].iDifficulty].sName);
			bClick=true;
		};
		};

		if (buttFaceL.Check())
		{
			if (arHumans[Index].ipAvatar>0)
			{arHumans[Index].ipAvatar--;}
			else
			{arHumans[Index].ipAvatar=iMaxAvatarPic-1;};
			draw_sprite(bmpWindow,bmpAvatar[arHumans[Index].ipAvatar],73,215);
			bClick=true;
		};
		if (buttFaceR.Check())
		{
			if (arHumans[Index].ipAvatar<iMaxAvatarPic-1)
			{arHumans[Index].ipAvatar++;}
			else
			{arHumans[Index].ipAvatar=0;};
			// FIXME: error where arHumans[Index].ipAvatar == 21, but iMaxAvatarPic == 41
			//        open the character in the main menu and scroll through avatars
			draw_sprite(bmpWindow,bmpAvatar[arHumans[Index].ipAvatar],73,215);
			bClick=true;
		};


		Check=mskName.Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(214);
		};
		if (Check==30)
		{
			bHint=false;
			if (bName)
			{
				inpName.Clear();
				bName=false;
			};
			if (inpLand.Flag>0)
			{
				inpLand.Activate(0);
			};
			inpName.Activate(1);
			bSave=true;
		};

		Check=mskLand.Check();
		if (Check>1)
		{
			bHint=true;
			MakeStringHint(215);
		};
		if (Check==30)
		{
			bHint=false;
			if (bLand)
			{
				inpLand.Clear();
				bLand=false;
			};
			if (inpName.Flag>0)
			{
				inpName.Activate(0);
			};
			inpLand.Activate(1);
			bSave=true;
		};
		if (mskDiff.Check()>1)
		{
			bHint=true;
			MakeStringHint(218);
		};
		inpName.Check(true);
		inpLand.Check(true);

		if (bClick)
		{
			DrawHerb(arHumans[Index].iColor1, arHumans[Index].iColor2, arHumans[Index].iSign, 124, 140, bmpWindow);
			DrawHerb(arHumans[Index].iColor1, arHumans[Index].iColor2, arHumans[Index].iSign, 22, 220, bmpWindow);
			DrawHerb(arHumans[Index].iColor1, arHumans[Index].iColor2, arHumans[Index].iSign, 217, 220, bmpWindow);
			DrawTownFlag(arHumans[Index].iColor1, arHumans[Index].iColor2, arHumans[Index].iSign, 219-50+15, 6, bmpWindow);
			DrawTownFlag(arHumans[Index].iColor1, arHumans[Index].iColor2, arHumans[Index].iSign, 17-50+3, 6, bmpWindow);
			rectfill(bmpWindow,38,147,57,166,makecol(arHerbColors[arHumans[Index].iColor1][0],arHerbColors[arHumans[Index].iColor1][1],arHerbColors[arHumans[Index].iColor1][2]));
			rectfill(bmpWindow,219,147,238,166,makecol(arHerbColors[arHumans[Index].iColor2][0],arHerbColors[arHumans[Index].iColor2][1],arHerbColors[arHumans[Index].iColor2][2]));
			if (inpName.Flag>0)
			{
				inpName.Activate(0);
			};
			if (inpLand.Flag>0)
			{
				inpLand.Activate(0);
			};
			bClick=false;
			if (bFirst)
			{bFirst=false;}
			else
			{bSave=true;};
		};

		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,10,true);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	if (bSave)
	{		
		sprintf(arHumans[Index].sName,inpName.Text);
		sprintf(arHumans[Index].sLand,inpLand.Text);
		arHumans[Index].iEnterData=3;				
	};
	arHumans[Index].Save();
	/*if (Index==0)
	{
		iActivePlayer=arHumans[0].iFileIndex;
		PACKFILE *WorkFile;
		WorkFile=pack_fopen("save/game.con","w");
		pack_fwrite(&iActivePlayer,sizeof iActivePlayer,WorkFile);
		pack_fclose(WorkFile);
	};*/
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	blit(bmpBack,screen1,0,0,0,0,1024,768);
	blit(bmpBack,screen,0,0,0,0,1024,768);
//	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	iAntisleep=iAntisleepDelay;
	return bResult;
};



//***************************************************
//*** Окно выбора профиля игрока ********************
//***************************************************

void WindowChoosePlayer (int Index)
{	
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_ChoosePlayer->w/2;
	int y=384-bmpGUI_ChoosePlayer->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);	
	BITMAP *bmpWindow=create_bitmap(bmpGUI_ChoosePlayer->w,bmpGUI_ChoosePlayer->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);	
	draw_sprite(bmpWindow,bmpGUI_ChoosePlayer,0,0);

	UI_Button1 buttExit;
	WObj=buttExit.Init(x+157,y+451,1,3,bmpButton[84],bmpButton[85],bmpButton[86],bmpButton[86],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=106;

	UI_Button1 buttScrollUp;
	WObj=buttScrollUp.Init(x+73,y+451,1,3,bmpButton[76],bmpButton[77],bmpButton[78],bmpButton[79],WObj);
	buttScrollUp.Activate(-1);
	
	UI_Button1 buttScrollDown;
	WObj=buttScrollDown.Init(x+241,y+451,1,3,bmpButton[80],bmpButton[81],bmpButton[82],bmpButton[83],WObj);
	buttScrollDown.Activate(-1);

	al_ffblk ProFile;
	PACKFILE *WorkFile;
	clHumanProfile Prof;
	int Max=0;
	
	if (al_findfirst("save/player*.prf",&ProFile,FA_ALL)==0)
	{		
		do {Max++;} while (al_findnext(&ProFile)==0);		
		al_findclose(&ProFile);
	};	

	UI_Mask mskRul[7];
	BITMAP *bmpRul[7];
	textprintf_centre_ex(bmpWindow,Font1,190,19,COLOR_1,-1,(pFirstPhrase+444)->str);
	for (int i=0; i<7; i++)
	{
		bmpRul[i]=create_bitmap(bmpGUI_RulerBar->w,bmpGUI_RulerBar->h);
		WObj=mskRul[i].Init(x+20,y+37+i*59,1,bmpGUI_RulerBar,bmpRul[i],WObj,3);
	};
	draw_sprite(bmpWindow,bmpGUI_RulerBar,20,37);
	draw_sprite(bmpWindow,bmpSmallAvatar[16],25,41);
	draw_sprite(bmpWindow,bmpHerbRandom,88,51);
	draw_sprite(bmpWindow,bmpHerbRandom,312,51);
	textprintf_centre_ex(bmpWindow,Font1,217,61,COLOR_1,-1,(pFirstPhrase+212)->str);
	blit(bmpWindow,bmpRul[0],20,37,0,0,bmpGUI_RulerBar->w,bmpGUI_RulerBar->h);
	set_trans_blender(200,200,200,0);
	draw_lit_sprite(bmpRul[0],bmpRul[0],0,0,100);
	char FName[500];


	bool bShow=true; int Shift=0, Result=-2, MaxMask;

	while (Result==-2)
	{
		if (buttExit.Check())
		{Result=-1;};
		if (bShow)
		{
			blit(bmpGUI_ChoosePlayer,bmpWindow,20,37+59,20,37+59,330,59*6);
			int Count=0;
			MaxMask=1;
			if (al_findfirst("save/player*.prf",&ProFile,FA_ALL)==0)
			{		
				do 
				{
					if ((Count>=Shift*6)&&(Count<Shift*6+6))
					{
						sprintf(FName,"save/%s",ProFile.name);
						WorkFile=pack_fopen(FName,"rp");
						pack_fread(&Prof,sizeof Prof,WorkFile);
						pack_fclose(WorkFile);
						int DY=(Count-Shift*6+1)*59;
						draw_sprite(bmpWindow,bmpGUI_RulerBar,20,37+DY);
						draw_sprite(bmpWindow,bmpSmallAvatar[Prof.ipAvatar],25,41+DY);
						DrawHerb(Prof.iColor1,Prof.iColor2,Prof.iSign,88,51+DY,bmpWindow);
						DrawHerb(Prof.iColor1,Prof.iColor2,Prof.iSign,312,51+DY,bmpWindow);	
						textprintf_centre_ex(bmpWindow,Font1,217,51+DY,COLOR_1,-1,Prof.sName);
						textprintf_centre_ex(bmpWindow,Font1,217,71+DY,COLOR_1,-1,arDifficulty[Prof.iDifficulty].sName);
						blit(bmpWindow,bmpRul[Count-Shift*6+1],20,37+DY,0,0,bmpGUI_RulerBar->w,bmpGUI_RulerBar->h);
						draw_lit_sprite(bmpRul[Count-Shift*6+1],bmpRul[Count-Shift*6+1],0,0,100);
						MaxMask++;
					};
					Count++;
				}while (al_findnext(&ProFile)==0);
				al_findclose(&ProFile);				
			};
			if (Max>6)
			{
				if (Shift>0)
				{buttScrollUp.Activate(1);}
				else
				{buttScrollUp.Activate(0);};
				if (Count>Shift*6+6)
				{buttScrollDown.Activate(1);}
				else
				{buttScrollDown.Activate(0);};				
			};
			bShow=false;
		};

		for (int i=0; i<MaxMask; i++)
		{
			if (mskRul[i].Check()==30)
			{Result=i;};
		};

		if (buttScrollUp.Check())
		{
			Shift--;
			bShow=true;
		};

		if (buttScrollDown.Check())
		{
			Shift++;
			bShow=true;
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

	if (Result==0)
	{
		arHumans[Index].iFileIndex=-1;
		arHumans[Index].ClearNames();
	};

	if (Result>0)
	{
		int CN=0;
		Result--;
		if (al_findfirst("save/player*.prf",&ProFile,FA_ALL)==0)
		{
			while (CN<Result+Shift*6)
			{
				CN++;
				al_findnext(&ProFile);
			};
		};
		sprintf(FName,"save/%s",ProFile.name);
		WorkFile=pack_fopen(FName,"rp");
		pack_fread(&arHumans[Index],sizeof arHumans[Index],WorkFile);
		pack_fclose(WorkFile);
		al_findclose(&ProFile);
	};	
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,screen1,0,0,0,0,1024,768);
	blit(bmpBack,screen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	for (int i=0; i<7; i++)
	{
		destroy_bitmap(bmpRul[i]);
	};
	set_mouse_sprite(OldMouse);
	set_trans_blender(128,128,128,128);	
	iAntisleep=iAntisleepDelay;
};

void ProfileOperator (int Index)
{
	bool bRepeat=true;
	if (Index==0)
	{bRepeat=WindowProfile(Index, true);};
	while (bRepeat)
	{
		WindowChoosePlayer(Index);
		bRepeat=WindowProfile(Index, true);
	};
	iActivePlayer=arHumans[0].iFileIndex;
	PACKFILE *WorkFile;
	WorkFile=pack_fopen("save/game.con","w");
	pack_fwrite(&iActivePlayer,sizeof iActivePlayer,WorkFile);
	pack_fclose(WorkFile);
	iSoundVolume=arHumans[0].iSndVolume;
	iMusicVolume=arHumans[0].iMusVolume;
	if (MainTrack>=0)
	{ChangeMusicVolume();};
};

//***************************************************
//*** Окно Алхимии **********************************
//***************************************************
void WindowAlchemy (int Player)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_Alchemy->w/2;
	int y=384-bmpGUI_Alchemy->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_Alchemy->w,bmpGUI_Alchemy->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_Alchemy,bmpWindow,0,0,0,0,bmpGUI_Alchemy->w,bmpGUI_Alchemy->h);

	
	UI_Button1 buttCancel;
	WObj=buttCancel.Init(x+159,y+126,1,3,bmpButton[84],bmpButton[85],bmpButton[86],bmpButton[86],WObj);
	buttCancel.Activate(1);
	buttCancel.ipHotkey=KEY_ESC;
	buttCancel.iHint=106;
	UI_Button1 buttAccept;
	WObj=buttAccept.Init(x+90,y+126,1,3,bmpButton[73],bmpButton[74],bmpButton[75],bmpButton[120],WObj);
	buttAccept.Activate(1);	
	buttAccept.iHint=353;
	textprintf_centre_ex(bmpWindow,Font1,150,26,COLOR_1,-1,"%s",(pFirstPhrase+352)->str);
	int Gold=arPlayer[Player].Gold, Alch=arPlayer[Player].Gem, Min=0, Max=Alch+Gold/25;
	UI_ScrollBar Scroll;
	WObj=Scroll.Init(x+44,y+112,200,7,3,false,true,WObj);
	Scroll.AttachValue(&Alch,Min,Max);
	Scroll.Flag=1;
	textprintf_centre_ex(bmpWindow,Font1,110,54,COLOR_2,-1,"%d",Gold);
	textprintf_centre_ex(bmpWindow,Font1,193,54,COLOR_2,-1,"%d",Alch);
	textprintf_centre_ex(bmpWindow,Font1,110,86,makecol(220,220,20),-1,"0");
	textprintf_centre_ex(bmpWindow,Font1,193,86,makecol(220,220,20),-1,"0");
	int ExitCode=-1;
	

	while (ExitCode<0)
	{
		if (Scroll.Check()>2)
		{
			blit(bmpGUI_Alchemy,bmpWindow,73,54,73,54,154,50);				
			if (Alch<arPlayer[Player].Gem)
			{
				Gold=arPlayer[Player].Gold+arPlayer[Player].Gem-Alch;				
				textprintf_centre_ex(bmpWindow,Font1,110,86,makecol(20,220,20),-1,"+%d",arPlayer[Player].Gem-Alch);
				textprintf_centre_ex(bmpWindow,Font1,193,86,makecol(220,20,20),-1,"%d",Alch-arPlayer[Player].Gem);
			};
			if (Alch>arPlayer[Player].Gem)
			{
				Gold=arPlayer[Player].Gold+(arPlayer[Player].Gem-Alch)*25;				
				textprintf_centre_ex(bmpWindow,Font1,110,86,makecol(220,20,20),-1,"%d",(arPlayer[Player].Gem-Alch)*25);
				textprintf_centre_ex(bmpWindow,Font1,193,86,makecol(20,220,20),-1,"+%d",Alch-arPlayer[Player].Gem);
			};
			if (Alch==arPlayer[Player].Gem)
			{
				Gold=arPlayer[Player].Gold;
				textprintf_centre_ex(bmpWindow,Font1,110,86,makecol(220,220,20),-1,"0");
				textprintf_centre_ex(bmpWindow,Font1,193,86,makecol(220,220,20),-1,"0");
			};
			textprintf_centre_ex(bmpWindow,Font1,110,54,COLOR_2,-1,"%d",Gold);
			textprintf_centre_ex(bmpWindow,Font1,193,54,COLOR_2,-1,"%d",Alch);
		};
		if (Scroll.Flag!=3)
		{
			if (buttCancel.Check())
			{ExitCode=0;};
			if (buttAccept.Check())
			{
				ExitCode=0;
				arPlayer[Player].Gold=Gold;
				arPlayer[Player].Gem=Alch;
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
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	iAntisleep=iAntisleepDelay;
};


//***************************************************
//*** Внутриигровое меню ****************************
//***************************************************
bool WindowMenu (int Player)
{
	iAntisleep=iAntisleepDelay;
	bool Result=false;
	int x=512-bmpGUI_Menu->w/2;
	int y=384-bmpGUI_Menu->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_Menu->w,bmpGUI_Menu->h);
	WObj=new G_Object(WObj,x,y,2,bmpWindow);
	blit(bmpGUI_Menu,bmpWindow,0,0,0,0,bmpGUI_Menu->w,bmpGUI_Menu->h);
	WObj=new G_Object(WObj,x+44,y+4,1,bmpStuff[37]);
	WObj->bTrans=true;

	
	UI_Mask mskMenu[5];
	BITMAP *arMenu[5];
	bool bMenu[5];
	for (int i=0; i<5; i++)
	{
		arMenu[i]=create_bitmap(bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
		blit(bmpGUI_MenuScrA,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
		WObj=mskMenu[i].Init(x+38,y+134+i*30,1,bmpGUI_MenuScrN,arMenu[i],WObj,3);
		draw_sprite(bmpWindow,bmpGUI_MenuScrN,38,134+i*30);
		bMenu[i]=true;
	};
	bool bAstralShift=false;
	if (iGameMode==1)
	{
		if (Campaign.arJournal[979]<0)
		{bMenu[2]=false;}
	}
	else
	{
		if (Tutorial.Map)
		{
			bMenu[2]=false;
			bMenu[3]=false;
		};
		if ((!bTimeShift)&&(CurGame.iGameType!=2))
		{
			bMenu[2]=false;
		};
		if ((bMenu[2])&&(CurGame.iGameType==2))
		{
			char sFile[150];
			sprintf(sFile,"save/game%d.bka",arHumans[0].iFileIndex);
			al_ffblk Fil;
			if (al_findfirst(sFile,&Fil,FA_ALL)==0)
			{bAstralShift=true;};
			al_findclose(&Fil);
			if ((!bTimeShift)&&(!bAstralShift))
			{bMenu[2]=false;};
		};
	};
	if (CurGame.iGameType>2)
	{bMenu[2]=false;};
	
	textprintf_centre_ex(bmpWindow,Font1,177,143,COLOR_1,-1,"%s",(pFirstPhrase+199)->str);
	textprintf_centre_ex(arMenu[0],Font1,177-38,143-134,COLOR_1,-1,"%s",(pFirstPhrase+199)->str);
	textprintf_centre_ex(bmpWindow,Font1,177,143+1*30,COLOR_1,-1,"%s",(pFirstPhrase+206)->str);
	textprintf_centre_ex(arMenu[1],Font1,177-38,143-134,COLOR_1,-1,"%s",(pFirstPhrase+206)->str);
	if (iGameMode==1)
	{
		if (bMenu[2])
		{textprintf_centre_ex(bmpWindow,Font1,177,143+2*30,COLOR_1,-1,"%s",(pFirstPhrase+525)->str);}
		else
		{textprintf_centre_ex(bmpWindow,Font1,177,143+2*30,makecol(120,120,120),-1,"%s",(pFirstPhrase+525)->str);};
		textprintf_centre_ex(arMenu[2],Font1,177-38,143-134,COLOR_1,-1,"%s",(pFirstPhrase+525)->str);
	}
	else
	{
		if (bMenu[2])
		{textprintf_centre_ex(bmpWindow,Font1,177,143+2*30,COLOR_1,-1,"%s",(pFirstPhrase+395)->str);}	
		else
		{textprintf_centre_ex(bmpWindow,Font1,177,143+2*30,makecol(120,120,120),-1,"%s",(pFirstPhrase+395)->str);};
		textprintf_centre_ex(arMenu[2],Font1,177-38,143-134,COLOR_1,-1,"%s",(pFirstPhrase+395)->str);
	}	
	if (bMenu[3])
	{textprintf_centre_ex(bmpWindow,Font1,177,143+3*30,COLOR_1,-1,"%s",(pFirstPhrase+393)->str);}
	else
	{textprintf_centre_ex(bmpWindow,Font1,177,143+3*30,makecol(120,120,120),-1,"%s",(pFirstPhrase+393)->str);};
	textprintf_centre_ex(arMenu[3],Font1,177-38,143-134,COLOR_1,-1,"%s",(pFirstPhrase+393)->str);
	textprintf_centre_ex(bmpWindow,Font1,177,143+4*30,COLOR_1,-1,"%s",(pFirstPhrase+135)->str);
	textprintf_centre_ex(arMenu[4],Font1,177-38,143-134,COLOR_1,-1,"%s",(pFirstPhrase+135)->str);

	
	int Exit=-1, Check;
	bool bEsc=false;
	

	while ((Exit<0)||(key[KEY_ESC]))
	{
		if (!key[KEY_ESC])
		{bEsc=true;};
		if ((bEsc)&&(key[KEY_ESC])){Exit=1;};
		for (int i=0; i<5; i++)
		{
			if (bMenu[i])
			{
				Check=mskMenu[i].Check();
				if (Check==30)
				{
					int Answer;
					switch (i)
					{
					case 0:
						Exit=1;
						break;
					case 1:
						if (WindowSettings(arPlayer[Player].iColor))
						{Result=true;};
						break;
					case 2:
						if (iGameMode==1)
						{
							Answer=DifficultyWindow();
							if ((Answer!=Campaign.iDifficulty)&&((pFirstDialog+1258)->Show(1,1,0,0,NULL,NULL)==1))
							{
								Campaign.iDifficulty=Answer;
								if (Campaign.iDifficulty<Campaign.arJournal[979])
								{Campaign.arJournal[979]=Campaign.iDifficulty;};
							};
						}
						else
						{						
							Answer=1;
							if (CurGame.iGameType==2)
							{
								Answer=(pFirstDialog+1245)->Show(bTimeShift,bAstralShift,1,0,NULL,NULL);
								if (Answer==2)
								{
									if ((pFirstDialog+831)->Show(1,1,0,0,NULL,NULL)==2)
									{Answer=3;};
								};
							};
							if (Answer<3)
							{
								Exit=1;
								bExitGame=true;	
								if (Answer==1)
								{iExitFlag=-1;}
								else
								{iExitFlag=-2;};
							};
						};
						break;
					case 3:
						if (iGameMode==1)
						{
							if ((pFirstDialog+1183)->Show(1,1,0,0,NULL,NULL)==1)
							{
								FinishCampaign(0);
								bExitGame=true;
								Exit=1;
							};
						}
						else
						{
							if ((pFirstDialog+1163)->Show(1,1,0,0,NULL,NULL)==1)
							{
								Lose(Player);						
								Exit=1;
							};
						};
						break;
					case 4:
						Exit=1;
						bExitGame=true;	
						iExitFlag=0;
						break;
					};
				};
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
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	for (int i=0; i<5; i++)
	{
		destroy_bitmap(arMenu[i]);	
	};
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	iAntisleep=iAntisleepDelay;
	return Result;
};

//***************************************************
//*** Окно редактирования свойств карты *************
//***************************************************
void WindowMapSettings(clMap *Map, int *BRD)
{	
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_MapSettings->w/2;
	int y=384-bmpGUI_MapSettings->h/2;
	int dx=204, dy=74, leng=140;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * objGUI=new G_Object(NULL,0,0,0,bmpDarkScreen);	
	BITMAP *bmpWindow=create_bitmap(bmpGUI_MapSettings->w,bmpGUI_MapSettings->h);
	objGUI=new G_Object(objGUI,x,y,1,bmpWindow);
	blit(bmpGUI_MapSettings,bmpWindow,0,0,0,0,bmpGUI_MapSettings->w,bmpGUI_MapSettings->h);

	UI_Button1 buttAccept;
	objGUI=buttAccept.Init(x+370,y+336,1,2,bmpButton[305],bmpButton[306],bmpButton[307],bmpButton[308],objGUI);
	buttAccept.iHint=354;

	UI_Button1 buttCancel;
	objGUI=buttCancel.Init(x+305,y+336,1,2,bmpButton[84],bmpButton[85],bmpButton[86],bmpButton[86],objGUI);
	buttCancel.iHint=106;

	UI_Button1 buttDefault;
	objGUI=buttDefault.Init(x+239,y+336,1,2,bmpButton[87],bmpButton[88],bmpButton[90],bmpButton[89],objGUI);
	buttDefault.iHint=355;

	UI_Button1 buttTypeL, buttTypeR, buttBorderL, buttBorderR;
	objGUI=buttTypeL.Init(x+34,y+70,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	objGUI=buttTypeR.Init(x+200,y+70,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
	objGUI=buttBorderL.Init(x+72,y+310,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	objGUI=buttBorderR.Init(x+153,y+310,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);

	UI_Mask1 mskHint[12];
	for (int i=0; i<12; i++)
	{
		if (i==3)
		{mskHint[i].Init(x+42+(i/4)*204,y+47+(i%4)*74,175,87,1);}
		else
		{mskHint[i].Init(x+42+(i/4)*204,y+47+(i%4)*74,175,55,1);};		
	};

	textprintf_centre_ex(bmpWindow,Font1,332,20,COLOR_1,-1,(pFirstPhrase+356)->str);

	UI_ScrollBar scrMain[10];
	int arSet[10];
	arSet[0]=Map->Isles/5;
	arSet[1]=Map->Dissolve/10-1;
	arSet[2]=(Map->Water-5)/10;	
	arSet[3]=(Map->Forest-10)/5;
	arSet[4]=(Map->Hill-15)/5;
	arSet[5]=(Map->Swamp-10)/5;
	arSet[6]=Map->Population;
	switch (Map->Neutrals)
	{
	case 1:
		arSet[7]=0;
		break;
	case 4:
		arSet[7]=1;
		break;
	case 8:
		arSet[7]=2;
		break;
	case 12:
		arSet[7]=3;
		break;
	case 20:
		arSet[7]=4;
		break;
	};
	arSet[8]=Map->Resource;
	arSet[9]=Map->Site;
	int Type=Map->Type;
	int Border=*BRD;

	objGUI=scrMain[0].Init(x+50,y+101+dy,leng,7,2,false,true,objGUI);
	objGUI=scrMain[1].Init(x+50,y+101+2*dy,leng,7,2,false,true,objGUI);
	for (int i=0; i<8; i++)
	{objGUI=scrMain[i+2].Init(x+50+(1+i/4)*dx,y+101+(i%4)*dy,leng,7,2,false,true,objGUI);};
	for (int i=0; i<10; i++)
	{scrMain[i].AttachValue(&arSet[i],0,4);};
	/*scrMain[0].AttachValue(&(Map.Isles),0,20);
	scrMain[1].AttachValue(&(Map.Dissolve),10,50);
	scrMain[2].AttachValue(&(Map.Water),0,45);
	scrMain[3].AttachValue(&(Map.Forest),5,30);
	scrMain[4].AttachValue(&(Map.Hill),10,35);
	scrMain[5].AttachValue(&(Map.Swamp),5,30);
	scrMain[6].AttachValue(&(Map.Population),0,20);
	scrMain[7].AttachValue(&(Map.Neutrals),1,20);
	scrMain[8].AttachValue(&(Map.Resource),0,20);
	scrMain[9].AttachValue(&(Map.Site),0,20);*/

	int ExitCode=-1, Active=-1, Check;
	bool bRedraw=true;
	

	while (ExitCode<0)
	{
		if (bRedraw)
		{
			blit(bmpGUI_MapSettings,bmpWindow,50,70,50,70,563,255);
			for (int i=0; i<12; i++)
			{
				textprintf_centre_ex(bmpWindow,Font1,127+(i/4)*dx,55+(i%4)*dy,COLOR_1,-1,(pFirstPhrase+357+i)->str);
			};
			textprintf_centre_ex(bmpWindow,Font1,127,81,COLOR_2,-1,(pFirstPhrase+382+Type)->str);
			textprintf_centre_ex(bmpWindow,Font1,127,81+dy,COLOR_2,-1,(pFirstPhrase+373+arSet[0])->str);
			textprintf_centre_ex(bmpWindow,Font1,127,81+2*dy,COLOR_2,-1,(pFirstPhrase+373+arSet[1])->str);
			for (int i=0; i<8; i++)
			{
				if (i==5)
				{textprintf_centre_ex(bmpWindow,Font1,127+(1+i/4)*dx,81+(i%4)*dy,COLOR_2,-1,(pFirstPhrase+378+arSet[2+i])->str);}
				else
				{textprintf_centre_ex(bmpWindow,Font1,127+(1+i/4)*dx,81+(i%4)*dy,COLOR_2,-1,(pFirstPhrase+373+arSet[2+i])->str);};
			};
			draw_sprite(bmpWindow,bmpStuff[27+Border],96,300);
		//	textprintf_centre_ex(bmpWindow,Font1,122,319,COLOR_2,-1,"%d",Border);
			bRedraw=false;
		};
		if (Active<0)
		{
		if (buttCancel.Check())
		{ExitCode=0;};
		if (buttTypeL.Check())
		{
			Type--;
			if (Type<1)
			{Type=MAX_MAP_TYPE;};
			bRedraw=true;
		};
		if (buttTypeR.Check())
		{
			Type++;
			if (Type>MAX_MAP_TYPE)
			{Type=1;};
			bRedraw=true;
		};
		if (buttBorderL.Check())
		{
			Border--;
			if (Border<0)
			{Border=3;};
			bRedraw=true;
		};
		if (buttBorderR.Check())
		{
			Border++;
			if (Border>3)
			{Border=0;};
			bRedraw=true;
		};
		if (buttAccept.Check())
		{
			ExitCode=1;
			Map->Isles=arSet[0]*5;
			Map->Dissolve=(arSet[1]+1)*10;
			Map->Water=arSet[2]*10+5;	
			Map->Forest=arSet[3]*5+10;
			Map->Hill=arSet[4]*5+15;
			Map->Swamp=arSet[5]*5+10;
			Map->Population=arSet[6];
			switch (arSet[7])
			{
			case 0:
				Map->Neutrals=1;
				break;
			case 1:
				Map->Neutrals=4;
				break;
			case 2:
				Map->Neutrals=8;
				break;
			case 3:
				Map->Neutrals=12;
				break;
			case 4:
				Map->Neutrals=20;
				break;
			};
			Map->Resource=arSet[8];
			Map->Site=arSet[9];
			Map->Type=Type;
			*BRD=Border;
		};
		if (buttDefault.Check())
		{
			for (int i=0; i<10; i++)
			{arSet[i]=2;};
			arSet[7]=4;			
			Type=2;
			Border=0;
			for (int i=0; i<10; i++)
			{scrMain[i].Show();};
			bRedraw=true;
		};
		for (int i=0; i<10; i++)
		{
			Check=scrMain[i].Check();
			if (Check>2)
			{
				bRedraw=true;
				if (Check==3)
				{Active=i;};
			};
		};
		for (int i=0; i<12; i++)
		{
			Check=mskHint[i].Check();
			if (Check>1)
			{
				MakeTextHint(i+95);
				bHint=true;				
			};
		};
		}
		else
		{
			if (scrMain[Active].Check()==3)
			{bRedraw=true;}
			else
			{Active=-1;};
		};
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
};


//***************************************************
//*** Окно настроек *********************************
//***************************************************
bool WindowSettings(int ipPlayer)
{
	iAntisleep=iAntisleepDelay;
	bool bResult=false;
	int x=512-bmpGUI_Settings->w/2;
	int y=384-bmpGUI_Settings->h/2;
	int dx=204, dy=74, leng=140;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * objGUI=new G_Object(NULL,0,0,0,bmpDarkScreen);	
	BITMAP *bmpWindow=create_bitmap(bmpGUI_Settings->w,bmpGUI_Settings->h);
	objGUI=new G_Object(objGUI,x,y,1,bmpWindow);
	blit(bmpGUI_Settings,bmpWindow,0,0,0,0,bmpGUI_Settings->w,bmpGUI_Settings->h);

	UI_Button1 buttAccept;
	objGUI=buttAccept.Init(x+386,y+351,1,2,bmpButton[305],bmpButton[306],bmpButton[307],bmpButton[308],objGUI);
	buttAccept.iHint=354;

	UI_Button1 buttCancel;
	objGUI=buttCancel.Init(x+321,y+351,1,2,bmpButton[84],bmpButton[85],bmpButton[86],bmpButton[86],objGUI);
	buttCancel.iHint=106;

	UI_Button1 buttDefault;
	objGUI=buttDefault.Init(x+255,y+351,1,2,bmpButton[87],bmpButton[88],bmpButton[90],bmpButton[89],objGUI);
	buttDefault.iHint=355;

//	buttDefault.wavSound=wavCrush;

	UI_Button1 buttLvlL, buttLvlR, buttBattL, buttBattR;
	objGUI=buttLvlL.Init(x+253,y+253,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	objGUI=buttLvlR.Init(x+421,y+253,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);
	objGUI=buttBattL.Init(x+253,y+315,1,2,bmpButton[236],bmpButton[237],bmpButton[238],bmpButton[288],objGUI);
	objGUI=buttBattR.Init(x+421,y+315,1,2,bmpButton[233],bmpButton[234],bmpButton[235],bmpButton[287],objGUI);

	UI_ScrollBar scrBar[3];
	for (int i=0; i<3; i++)
	{
		objGUI=scrBar[i].Init(x+267,y+99+i*54,145,7,2,false,true,objGUI);
	};
	scrBar[0].AttachValue(&iSoundVolume,0,255);
	scrBar[1].AttachValue(&iMusicVolume,0,255);
	int ScrollSpeed=arHumans[ipPlayer].iScrollSpeed;
	scrBar[2].AttachValue(&ScrollSpeed,1,20);
	int AutoLvl=arHumans[ipPlayer].iAutoLvlUp;
	int AutoCombat=arHumans[ipPlayer].iAutoCombat;

	textprintf_centre_ex(bmpWindow,Font1,349,20,COLOR_1,-1,(pFirstPhrase+206)->str);
	textprintf_centre_ex(bmpWindow,Font1,133,65,COLOR_1,-1,(pFirstPhrase+408)->str);
	textprintf_centre_ex(bmpWindow,Font1,556,65,COLOR_1,-1,(pFirstPhrase+407)->str);

	UI_Mask1 mskCombat, mskLevel;
	mskLevel.Init(x+264,y+234,168,46,1);
	mskCombat.Init(x+264,y+294,168,46,1);

	for (int i=0; i<4; i++)
	{
		textprintf_centre_ex(bmpWindow,Font1,346,77+i*54,COLOR_1,-1,(pFirstPhrase+409+i)->str);
	};
	textprintf_centre_ex(bmpWindow,Font1,346,300,COLOR_1,-1,(pFirstPhrase+413)->str);

	BITMAP *bmpDark=create_bitmap(195,20);
	rectfill(bmpDark,0,0,195,20,COLOR_1);

	bool arMap[10], arMess[10];
	G_Object *MapBird[10], *MessBird[10];
	BITMAP *bmpMap[10], *bmpMess[10];
	UI_Mask mskMap[10], mskMess[10];

	for (int i=0; i<10; i++)
	{
		textprintf_ex(bmpWindow,Font1,45,101+i*21,COLOR_1,-1,(pFirstPhrase+417+i)->str);
		draw_sprite(bmpWindow,bmpCheckBox,210,100+i*21);
		arMap[i]=arHumans[ipPlayer].arMap[i];
		bmpMap[i]=create_bitmap(195,20);
		blit(bmpWindow,bmpMap[i],40,97+i*21,0,0,195,20);
		draw_trans_sprite(bmpMap[i],bmpDark,0,0);
		objGUI=new G_Object(objGUI,x+210,y+96+i*21,-1,bmpCheckBird);
		MapBird[i]=objGUI;
		if (arMap[i])
		{MapBird[i]->iLayer=3;};
		objGUI=mskMap[i].Init(x+40,y+97+i*21,1,bmpMap[i],bmpMap[i],objGUI,2);
	};
	for (int i=0; i<7; i++)
	{
		textprintf_ex(bmpWindow,Font1,465,101+i*21,COLOR_1,-1,(pFirstPhrase+427+i)->str);
		draw_sprite(bmpWindow,bmpCheckBox,630,100+i*21);
		arMess[i]=arHumans[ipPlayer].arMess[i];
		bmpMess[i]=create_bitmap(195,20);
		blit(bmpWindow,bmpMess[i],460,97+i*21,0,0,195,20);
		draw_trans_sprite(bmpMess[i],bmpDark,0,0);
		objGUI=new G_Object(objGUI,x+630,y+96+i*21,-1,bmpCheckBird);
		MessBird[i]=objGUI;
		if (arMess[i])
		{MessBird[i]->iLayer=3;};
		objGUI=mskMess[i].Init(x+460,y+97+i*21,1,bmpMess[i],bmpMess[i],objGUI,2);
	};

	if (AutoLvl==0)
	{buttLvlL.Activate(0);};	
	if (AutoLvl==4)
	{buttLvlR.Activate(0);};	
	if (AutoCombat==0)
	{buttBattL.Activate(0);};	
	if (AutoCombat==2)
	{buttBattR.Activate(0);};	
	
	
	int ExitCode=-1, Active=-1;
	bool bRedraw=true;
	

	while (ExitCode<0)
	{
		if (bRedraw)
		{
			blit(bmpGUI_Settings,bmpWindow,277,257,277,257,140,26);
			blit(bmpGUI_Settings,bmpWindow,277,319,277,319,140,26);
			switch (AutoLvl)
			{
			case 0:
				textprintf_centre_ex(bmpWindow,Font1,346,263,COLOR_2,-1,(pFirstPhrase+414)->str);
				break;
			case 1:
				textprintf_centre_ex(bmpWindow,Font1,346,263,COLOR_2,-1,"1 %s",(pFirstPhrase+10)->str);
				break;
			case 2:
				textprintf_centre_ex(bmpWindow,Font1,346,263,COLOR_2,-1,"1-2 %s",(pFirstPhrase+10)->str);
				break;
			case 3:				
				textprintf_centre_ex(bmpWindow,Font1,346,263,COLOR_2,-1,"1-3 %s",(pFirstPhrase+10)->str);
				break;
			case 4:
				textprintf_centre_ex(bmpWindow,Font1,346,263,COLOR_2,-1,(pFirstPhrase+416)->str);
				break;
			};
			textprintf_centre_ex(bmpWindow,Font1,346,326,COLOR_2,-1,(pFirstPhrase+414+AutoCombat)->str);
			bRedraw=false;
		};
		if (Active<0)
		{
		if (buttCancel.Check())
		{
			ExitCode=0;
			iSoundVolume=arHumans[ipPlayer].iSndVolume;
			iMusicVolume=arHumans[ipPlayer].iMusVolume;
			if (MainTrack>=0)
			{ChangeMusicVolume();};
			/*if (fFading==0)
			{				
				float Vol=iMusicVolume;
				Vol/=255;
				Track[0]->setVolume(Vol);
			};*/
		};
		if (buttLvlL.Check())
		{
			AutoLvl--;
			buttLvlR.Activate(1);
			if (AutoLvl==0)
			{buttLvlL.Activate(0);};	
			bRedraw=true;
		};
		if (buttLvlR.Check())
		{
			AutoLvl++;
			buttLvlL.Activate(1);
			if (AutoLvl==4)
			{buttLvlR.Activate(0);};	
			bRedraw=true;
		};
		if (buttBattL.Check())
		{
			AutoCombat--;
			buttBattR.Activate(1);
			if (AutoCombat==0)
			{buttBattL.Activate(0);};	
			bRedraw=true;			
		};
		if (buttBattR.Check())
		{
			AutoCombat++;
			buttBattL.Activate(1);
			if (AutoCombat==2)
			{buttBattR.Activate(0);};	
			bRedraw=true;
		};
		if (buttAccept.Check())
		{
			ExitCode=1;
			arHumans[ipPlayer].iSndVolume=iSoundVolume;
			arHumans[ipPlayer].iMusVolume=iMusicVolume;
			arHumans[ipPlayer].iScrollSpeed=ScrollSpeed;
			arHumans[ipPlayer].iAutoLvlUp=AutoLvl;
			arHumans[ipPlayer].iAutoCombat=AutoCombat;
			for (int i=0; i<10; i++)
			{		
				arHumans[ipPlayer].arMap[i]=arMap[i];
			};
			for (int i=0; i<7; i++)
			{		
				arHumans[ipPlayer].arMess[i]=arMess[i];
			};
			arHumans[ipPlayer].Save();
			Tutorial.Mode=arHumans[ipPlayer].arMess[0];
		};
		if (buttDefault.Check())
		{
			iSoundVolume=225;
			iMusicVolume=255;
			ScrollSpeed=SCROLL_SPEED;
			if (MainTrack>=0)
			{ChangeMusicVolume();};
			/*if (fFading==0)
			{
				float Vol=iMusicVolume;
				Vol/=255;
				Track[0]->setVolume(Vol);
			};*/
			for (int i=0; i<3; i++)
			{scrBar[i].Show();};
			AutoCombat=0;
			buttBattR.Activate(1);
			buttBattL.Activate(0);
			AutoLvl=0;
			buttLvlR.Activate(1);			
			buttLvlL.Activate(0);
			bRedraw=true;
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
			for (int i=0; i<10; i++)
			{				
				if (arMap[i])
				{MapBird[i]->iLayer=3;}
				else
				{MapBird[i]->iLayer=-1;};
			};
			for (int i=0; i<7; i++)
			{				
				if (arMess[i])
				{MessBird[i]->iLayer=3;}
				else
				{MessBird[i]->iLayer=-1;};
			};
		};

		for (int i=0; i<3; i++)
		{
			int Check=scrBar[i].Check();
			if (Check>2)
			{
		//		bRedraw=true;
				if (Check==3)
				{Active=i;};
				if ((i==1)&&(MainTrack>=0))
				{ChangeMusicVolume();};
				/*if ((i==1)&&(fFading==0))
				{
					float Vol=iMusicVolume;
					Vol/=255;
					Track[0]->setVolume(Vol);
				};*/
			};
		};
		for (int i=0; i<10; i++)
		{
			int Check=mskMap[i].Check();
			if (Check==30)
			{
				arMap[i]=!arMap[i];
				if (arMap[i])
				{MapBird[i]->iLayer=3;}
				else
				{MapBird[i]->iLayer=-1;};
				bResult=true;
			};
			if (Check>1)
			{
				MakeTextHint(i+56);
				bHint=true;
			};
		};
		for (int i=0; i<7; i++)
		{
			int Check=mskMess[i].Check();
			if (Check==30)
			{
				arMess[i]=!arMess[i];
				if (arMess[i])
				{MessBird[i]->iLayer=3;}
				else
				{MessBird[i]->iLayer=-1;};
			};
			if (Check>1)
			{
				MakeTextHint(i+66);
				bHint=true;
			};
		};
		if (mskLevel.Check()>1)
		{
			MakeTextHint(73);
			bHint=true;
		};
		if (mskCombat.Check()>1)
		{
			MakeTextHint(74);
			bHint=true;
		};
		}
		else
		{
			if (scrBar[Active].Check()==3)
			{
				if ((Active==1)&&(MainTrack>=0))
				{ChangeMusicVolume();};
				/*if ((Active==1)&&(fFading==0))
				{
					float Vol=iMusicVolume;
					Vol/=255;
					Track[0]->setVolume(Vol);
				};*/
			//	bRedraw=true;
			}
			else
			{Active=-1;};
		};
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,6,1);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	destroy_bitmap(bmpDark);
	for (int i=0; i<10; i++)
	{	
		destroy_bitmap(bmpMap[i]);
	};
	for (int i=0; i<7; i++)
	{
		destroy_bitmap(bmpMess[i]);
	};
	set_mouse_sprite(OldMouse);
	iAntisleep=iAntisleepDelay;
	if (ExitCode>0)
	{return bResult;}
	else
	{return false;};
};

int DifficultyWindow()
{
	iAntisleep=iAntisleepDelay;
	int Result=-1;	
	int x=512-bmpGUI_DiffWindow->w/2;
	int y=384-bmpGUI_DiffWindow->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_DiffWindow->w,bmpGUI_DiffWindow->h);
	WObj=new G_Object(WObj,x,y,2,bmpWindow);
	draw_sprite(bmpWindow,bmpAvatar[18],87,54);
	draw_sprite(bmpWindow,bmpGUI_DiffWindow,0,0);
//	blit(bmpGUI_DiffWindow,bmpWindow,0,0,0,0,bmpGUI_DiffWindow->w,bmpGUI_DiffWindow->h);
		
	UI_Mask mskMenu[7];
	BITMAP *arMenu[7];

	textprintf_centre_ex(bmpWindow,Font1,150,27,COLOR_1,-1,"%s",(pFirstPhrase+218)->str);
	
	for (int i=0; i<7; i++)
	{
		arMenu[i]=create_bitmap(bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
		blit(bmpGUI_MenuScrA,arMenu[i],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
		textprintf_centre_ex(bmpWindow,Font1,150,216+i*30,COLOR_1,-1,"%s",(pFirstPhrase+219+i)->str);
		textprintf_centre_ex(arMenu[i],Font1,150-12,216-207,COLOR_1,-1,"%s",(pFirstPhrase+219+i)->str);
		WObj=mskMenu[i].Init(x+12,y+207+i*30,1,bmpGUI_MenuScrN,arMenu[i],WObj,3);
	//	draw_sprite(bmpWindow,bmpGUI_MenuScrN,38,134+i*30);		
	};
	
	int Check;
	

	while (Result<0)
	{
		for (int i=0; i<7; i++)
		{
			Check=mskMenu[i].Check();
			if (Check==30)
			{
				Result=i;
			};
			if (Check>1)
			{
				MakeStringHint(459+i);
				bHint=true;
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
	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	blit(bmpBack,screen1,0,0,0,0,1024,768);
//	blit(bmpBack,screen,0,0,0,0,1024,768);
	for (int i=0; i<7; i++)
	{
		destroy_bitmap(arMenu[i]);	
	};
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	iAntisleep=iAntisleepDelay;
	return Result;
};

bool WindowChangeWeapon(dcHero *Hero)
{
	iAntisleep=iAntisleepDelay;
	bool Result=false;
	int x=512-bmpGUI_ChangeWeapon->w/2;
	int y=384-bmpGUI_ChangeWeapon->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_ChangeWeapon->w,bmpGUI_ChangeWeapon->h);
	WObj=new G_Object(WObj,x,y,2,bmpWindow);
//	draw_sprite(bmpWindow,bmpAvatar[18],87,54);
	draw_sprite(bmpWindow,bmpGUI_ChangeWeapon,0,0);
	UI_Button1 buttCancel;
	WObj=buttCancel.Init(x+31,y+236,1,3,bmpButton[25],bmpButton[26],bmpButton[27],bmpButton[28],WObj);
	buttCancel.Activate(1);
	buttCancel.ipHotkey=KEY_ESC;
	buttCancel.iHint=106;

	UI_Button1 buttAccept;
	WObj=buttAccept.Init(x+242,y+236,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttAccept.Activate(1);	
	buttAccept.iHint=354;

	UI_Button1 buttTrash;
	WObj=buttTrash.Init(x+108,y+236,1,3,bmpButton[229],bmpButton[230],bmpButton[231],bmpButton[232],WObj);
	buttTrash.Activate(0);
	buttTrash.wavSound=wavDelete;
//	buttAccept.iHint=354;
	BITMAP * bmpMouse=create_bitmap(52,52);

	

	
	int Hand[2][2], Pack[10][2];
	UI_Mask mskHand[2], mskPack[10];
	for (int i=0; i<2; i++)
	{
		Hand[i][0]=Hero->arInventory[1][0];
		Hand[i][1]=Hero->arInventory[1][1];
		WObj=mskHand[i].Init(x+80+126*i-6,y+45-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,3);		
	};
	for (int i=0; i<2; i++)
	{
		Hand[i][0]=Hero->arInventory[i+1][0];
		Hand[i][1]=Hero->arInventory[i+1][1];
		WObj=mskHand[i].Init(x+80+126*i-6,y+45-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,bmpGUI_SpellGlow,WObj,3);
	};

	for (int i=0; i<10; i++)
	{
		Pack[i][0]=Hero->arInventory[i+16][0];
		Pack[i][1]=Hero->arInventory[i+16][1];
		WObj=mskPack[i].Init(x+23+(i%5)*60-6,y+114+(i/5)*60-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,bmpGUI_SpellGlow,WObj,3);
	};

	bool bCycle=true, bRedraw=true;
	int Select=0, Dur, Slot=0, Check;
	while (bCycle)
	{
		if (bRedraw)
		{
			bRedraw=false;
			draw_sprite(bmpWindow,bmpGUI_ChangeWeapon,0,0);
			textprintf_centre_ex(bmpWindow,Font1,170,18,COLOR_1,-1,(pFirstPhrase+452)->str);
			for (int i=0; i<2; i++)
			{				
				if (Hand[i][0]>0)
				{
					draw_sprite(bmpWindow,(pFirstItem+Hand[i][0])->Pic,80+i*126,45);
					if (Hand[i][1]==0)
					{draw_sprite(bmpWindow,bmpGUI_SpellShadow,80+i*126,45);};
				};
				mskHand[i].bGlow=false;
				if (Select>0)
				{
					draw_sprite(bmpMouse,(pFirstItem+Select)->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash.Activate(1);
					if (Hero->CanUse(Select))
					{
						if ((((pFirstItem+Select)->ipSlot==1)||((pFirstItem+Select)->ipSlot==100))&&(i==0))
						{mskHand[i].bGlow=true;};
						if (((pFirstItem+Select)->ipSlot==2)&&(i==1))
						{mskHand[i].bGlow=true;};
					};
				}
				else
				{
					set_mouse_sprite(bmpMousePointer[0]);
					buttTrash.Activate(0);
				};
			};
			for (int i=0; i<10; i++)
			{
				if (Pack[i][0]>0)
				{		
					draw_sprite(bmpWindow,(pFirstItem+Pack[i][0])->Pic,23+(i%5)*60,114+(i/5)*60);
					if (Pack[i][1]==0)
					{draw_sprite(bmpWindow,bmpGUI_SpellShadow,23+(i%5)*60,114+(i/5)*60);};
				};
			};
		};

		for (int i=0; i<2; i++)
		{
			Check=mskHand[i].Check();
			if (Check==30)
			{
				if (Select>0)
				{
					if (Hero->CanUse(Select))
					{
						if (i==0)
						{
							if ((pFirstItem+Select)->ipSlot==1)
							{
								bRedraw=true;
								if (Slot>=0)
								{
									Pack[Slot][0]=Hand[0][0];
									Pack[Slot][1]=Hand[0][1];
									Hand[0][0]=Select;
									Hand[0][1]=Dur;
									Select=0;
									Dur=0;
								}
								else
								{
									int S1=Hand[0][0], S2=Hand[0][1];
									Hand[0][0]=Select;
									Hand[0][1]=Dur;									
									for (int j=0; j<10; j++)
									{
										if (Pack[j][0]==0)
										{
											Slot=j;
											break;
										};
									};
									if (Slot>=0)
									{
										Pack[Slot][0]=S1;
										Pack[Slot][1]=S2;
										Select=0;
										Dur=0;
									}
									else
									{
										Select=S1;
										Dur=S2;
										Slot=-1;
									};
								};
							};
							if ((pFirstItem+Select)->ipSlot==100)
							{
								bRedraw=true;
								if (Slot>=0)
								{
									Pack[Slot][0]=Hand[0][0];
									Pack[Slot][1]=Hand[0][1];									
								};
								Hand[0][0]=Select;
								Hand[0][1]=Dur;
								Select=0;
								Dur=0;
								if (Hand[1][0]>0)								
								{
									Slot=-1;
									for (int j=0; j<10; j++)
									{
										if (Pack[j][0]==0)
										{
											Slot=j;
											break;
										};
									};
									if (Slot>=0)
									{
										Pack[Slot][0]=Hand[1][0];
										Pack[Slot][1]=Hand[1][1];
									}
									else
									{
										Select=Hand[1][0];
										Dur=Hand[1][1];
										Slot=-1;
									};
									Hand[1][0]=0;
									Hand[1][1]=0;
								};
							};
						};
						if (i==1)
						{
							if ((pFirstItem+Select)->ipSlot==2)
							{
								bRedraw=true;
								if (Slot>=0)
								{
									Pack[Slot][0]=Hand[1][0];
									Pack[Slot][1]=Hand[1][1];									
								};
								Hand[1][0]=Select;
								Hand[1][1]=Dur;
								Select=0;
								Dur=0;
								Slot=-1;
								if ((Hand[0][0]>0)&&((pFirstItem+Hand[0][0])->ipSlot==100))
								{
									for (int j=0; j<10; j++)
									{
										if (Pack[j][0]==0)
										{
											Slot=j;
											break;
										};
									};
									if (Slot>=0)
									{
										Pack[Slot][0]=Hand[0][0];
										Pack[Slot][1]=Hand[0][1];
										Select=0;
										Dur=0;
									}
									else
									{
										Select=Hand[0][0];
										Dur=Hand[0][1];
										Slot=-1;
									};
									Hand[0][0]=0;
									Hand[0][1]=0;
								};
							};

						};
					};
				}
				else
				{
					if (Hand[i][0]>0)
					{
						Select=Hand[i][0];
						Dur=Hand[i][1];
						Hand[i][0]=0;
						Hand[i][1]=0;
						bRedraw=true;
						Slot=-2-i;
					};
				};
				Check=0;
			};

			if (Check==40)
			{
				if (Hand[i][0]>0)
				{WindowItemInfo(Hand[i][0],Hand[i][1]);};
			};

			if (Check>1)
			{
				if (Hand[i][0]>0)
				{
					MakeItemHint(Hand[i][0],Hand[i][1],Hero,0,0,0);
					bHint=true;
				};
			};

		};

		for (int i=0; i<10; i++)
		{
			Check=mskPack[i].Check();
			if (Check==30)
			{
				if (Select>0)
				{
					if (Pack[i][0]>0)
					{
						if (Slot>=0)
						{
							Pack[Slot][0]=Pack[i][0];
							Pack[Slot][1]=Pack[i][1];
							Pack[i][0]=Select;
							Pack[i][1]=Dur;
							Select=0;
							Dur=0;
							bRedraw=true;
						};

						if (Slot==-1)
						{
							int S1=Pack[i][0], S2=Pack[i][1];
							Pack[i][0]=Select;
							Pack[i][1]=Dur;
							Select=S1;
							Dur=S2;
							bRedraw=true;

						};
						
						if (Slot==-2)
						{
							if ((Hero->CanUse(Pack[i][0]))&&((pFirstItem+Pack[i][0])->ipSlot==1))
							{
								Hand[0][0]=Pack[i][0];
								Hand[0][1]=Pack[i][1];
								Pack[i][0]=Select;
								Pack[i][1]=Dur;
								Select=0;
								Dur=0;
								bRedraw=true;								
							}
							else
							{
							if ((Hero->CanUse(Pack[i][0]))&&((pFirstItem+Pack[i][0])->ipSlot==100))
							{
								Hand[0][0]=Pack[i][0];
								Hand[0][1]=Pack[i][1];
								Pack[i][0]=Select;
								Pack[i][1]=Dur;
								Select=0;
								Dur=0;
								if (Hand[1][0]>0)
								{
									Slot=-1;
									for (int j=0; j<10; j++)
									{
										if (Pack[j][0]==0)
										{
											Slot=j;
											break;
										};
									};
									if (Slot>=0)
									{
										Pack[Slot][0]=Hand[1][0];
										Pack[Slot][1]=Hand[1][1];
									}
									else
									{
										Select=Hand[1][0];
										Dur=Hand[1][1];
										Slot=-1;
									};
									Hand[1][0]=0;
									Hand[1][1]=0;
								};
								bRedraw=true;								
							};
							};
						};

						if (Slot==-3)
						{
							if ((Hero->CanUse(Pack[i][0]))&&((pFirstItem+Pack[i][0])->ipSlot==2))
							{
								Hand[1][0]=Pack[i][0];
								Hand[1][1]=Pack[i][1];
								Pack[i][0]=Select;
								Pack[i][1]=Dur;
								Select=0;
								Dur=0;
								bRedraw=true;
								if ((Hand[0][0]>0)&&((pFirstItem+Hand[0][0])->ipSlot==100))
								{
									Slot=-1;
									for (int j=0; j<10; j++)
									{
										if (Pack[j][0]==0)
										{
											Slot=j;
											break;
										};
									};
									if (Slot>=0)
									{
										Pack[Slot][0]=Hand[0][0];
										Pack[Slot][1]=Hand[0][1];
									}
									else
									{
										Select=Hand[0][0];
										Dur=Hand[0][1];
										Slot=-1;
									};
									Hand[0][0]=0;
									Hand[0][1]=0;
								};
							};
						};
					}
					else
					{						
						Pack[i][0]=Select;
						Pack[i][1]=Dur;
						bRedraw=true;
						Select=0;
						Dur=0;
					};
				}
				else
				{
					if (Pack[i][0]>0)
					{
						Select=Pack[i][0];
						Dur=Pack[i][1];
						Pack[i][0]=0;
						Pack[i][1]=0;
						bRedraw=true;
						Slot=i;
					};
				};
			};

			if (Check==40)
			{
				if (Pack[i][0]>0)
				{WindowItemInfo(Pack[i][0],Pack[i][1]);};
			};

			if (Check>1)
			{
				if (Pack[i][0]>0)
				{
					MakeItemHint(Pack[i][0],Pack[i][1],Hero,0,0,0);
					bHint=true;
				};
			};
		};

		if (buttTrash.Check())
		{
			Select=0;
			Dur=0;
			bRedraw=true;
		};

		if (buttCancel.Check())
		{
			bCycle=false;
			Result=false;
		};

		if (buttAccept.Check())
		{
			bCycle=false;
			Result=false;
			if (Select>0)
			{
				if (Slot>=0)
				{
					Pack[Slot][0]=Select;
					Pack[Slot][1]=Dur;
					Select=0;
					Dur=0;
				}
				else
				{
					for (int j=0; j<10; j++)
					{
						if (Pack[j][0]==0)
						{
							Slot=j;
							break;
						};
					};
					if (Slot>=0)
					{
						Pack[Slot][0]=Select;
						Pack[Slot][1]=Dur;
						Select=0;
						Dur=0;
					};
				};
			};
			for (int i=0; i<2; i++)
			{
				if ((Hand[i][0]!=Hero->arInventory[i+1][0])||(Hand[i][1]!=Hero->arInventory[i+1][1]))
				{Result=true;};
				Hero->arInventory[i+1][0]=Hand[i][0];
				Hero->arInventory[i+1][1]=Hand[i][1];
			};
			for (int i=0; i<10; i++)
			{
				Hero->arInventory[i+16][0]=Pack[i][0];
				Hero->arInventory[i+16][1]=Pack[i][1];
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

	G_Object * Temp;
	while (WObj)
	{
		Temp=WObj->pNext;		
		delete WObj;
		WObj=Temp;
	};
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);	
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	destroy_bitmap(bmpMouse);
	set_mouse_sprite(OldMouse);
	iAntisleep=iAntisleepDelay;
	return Result;
};

