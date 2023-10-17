
#include "Windower.h"
#ifdef YUPLAY
#include <yuGameClient.h>

//#define STOCK01           L"123"

int iYuPlay=1;
bool bYuPlayClient=false;
BITMAP *bmpYuPlay[MAX_YUPLAY];
YuClientItemInfo* arTicket[4][5];
bool bNeedLoadTicket=true;

int ipLottPrize, ipLottPrizeType;
dcUnit *pLottUnit;

YuplayClientStatus yuStatus;
clTicket arTicketType[MAX_TICKET_TYPE];

int YuPlayInit()
{
	if (iYuPlay)
	{
		DATAFILE* datYuPlay = load_datafile("dat\\yuplay.dat");
		if (!datYuPlay)
		{
			textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading YuPlay.dat");		
			readkey();
			return 1;
		};
		for (int i=0; i<MAX_YUPLAY; i++)
		{
			bmpYuPlay[i]=(BITMAP *)datYuPlay[i].dat;
		};
		yuStatus=yuplay_get_yuplay_client_status();
		if (yuStatus==YUC_NO_YUPLAY)
		{bYuPlayClient=false;}
		else
		{bYuPlayClient=true;}
	};
	return 0;
};

bool InstallYuPlay()
{
	if (iYuPlay==0)
	{return false;};
	if (bYuPlayClient)
	{return true;};
	yuStatus=yuplay_get_yuplay_client_status();
	if (yuStatus==YUC_NO_YUPLAY)
	{bYuPlayClient=false;}
	else
	{
		bYuPlayClient=true;
		return true;
	}
	al_ffblk Fil;
	bHourGlass=false;
	if (al_findfirst("YuPlay\\install_yuPlay_0.4.22.exe",&Fil,FA_ALL)==0)
	{
		if ((pFirstDialog+1257)->Show(1,1,0,0,NULL,NULL)==2)
		{RunYuInstall();}
		else
		{return false;};
	}
	else
	{
		if ((pFirstDialog+1246)->Show(1,1,0,0,NULL,NULL)==2)
		{OpenYuDownload();}
		else
		{return false;};
	};
	al_findclose(&Fil);

	yuStatus=yuplay_get_yuplay_client_status();
	if (yuStatus==YUC_NO_YUPLAY)
	{bYuPlayClient=false;}
	else
	{bYuPlayClient=true;}
	return bYuPlayClient;	
};

//Создание всплывающей подсказки по лотерейным билетам
void MakeTicketHint (int Item, float Price, int Rare)
{
	if ((HintType!=8)||(HintMember!=Item)||(HintDur!=Rare))
	{
	HintType=8; HintMember=Item; HintDur=Rare;
	rectfill(bmpHint,0,0,MAX_HINT_W,MAX_HINT_H,COLOR_0);	
	HintH=136;
	HintH+=arTicketType[Item].Number*20;	
	if (HintH>MAX_HINT_H){HintH=MAX_HINT_H;};
	HintW=bmpGUI_ItemHint->w;	
	blit(bmpGUI_ItemHint,bmpHint,0,0,0,0,HintW,HintH-20);
	blit(bmpGUI_ItemHint,bmpHint,0,696,0,HintH-20,HintW,11);
	blit(bmpGUI_ItemHint,bmpHint,0,bmpGUI_ItemHint->h-9,0,HintH-9,HintW,9);
	
	int Color=COLOR_2;
	switch (Rare)
	{
		case 1: Color=makecol(180,100,10);
				break;
		case 2: Color=makecol(200,200,205);
				break;
		case 3: Color=makecol(220,220,50);
				break;
		case 4: Color=makecol(200,20,0);
				break;		
	};	
	textprintf_centre_ex(bmpHint,Font1,151,20,COLOR_1,-1,"%s",(pFirstPhrase+514)->str);
	textprintf_centre_ex(bmpHint,Font1,150,19,Color,-1,"%s",(pFirstPhrase+514)->str);
	int Phrase=521;
	switch (Item/4)
	{
		case 0: Phrase=519;
				break;
		case 1: Phrase=333;
				break;
		case 2: Phrase=520;
				break;
		case 3: Phrase=420;
				break;
	};
	textprintf_centre_ex(bmpHint,Font1,150,50,Color,-1,"%s",(pFirstPhrase+Phrase)->str);
	textprintf_centre_ex(bmpHint,Font1,150,74,Color,-1,"%s",(pFirstPhrase+514+Rare)->str);
	int R1=Price, R2=Price*100, R3=Price*1000;
	R2=R2%100;
	if (R3%10>4)
	{R2++;};
	textprintf_centre_ex(bmpHint,Font1,150,98,Color,-1,"%s: %d.%.2d",(pFirstPhrase+312)->str,R1,R2);

	int StrY=128;

	for (int i=0; i<arTicketType[Item].Number; i++)
	{
		switch (arTicketType[Item].Str[i].iColor)
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
		textprintf_centre_ex(bmpHint,Font1,150,StrY,Color,-1,arTicketType[Item].Str[i].str);
		StrY+=20;
	};
	};
};


int PickItem(int Level, int Rarity)
{
//	char STR[100];
//	sprintf (STR,"Level: %d, Rare: %d...",Level,Rarity);
//	FileLog(STR);
	int Result=1, Count=0;
	ClearRoll();
	for (int i=1; i<=iMaxPureItem; i++)
	{
		if (((pFirstItem+i)->iShopLevel==Level)&&((pFirstItem+i)->iRarity==Rarity))
		{
			RollerN[Count]=i;
			RollerP[Count]=10;
			Count++;
		};
	};
	if (Count>0)
	{
		Result=Roll(false);
//		sprintf (STR,"Done! Item: %s",(pFirstItem+Result)->sName);
//		FileLog(STR);
	}
	else
	{
		if (Rarity>Level-1)
		{Result=PickItem(Level, Rarity-1);}
		if (Rarity<Level-1)
		{Result=PickItem(Level, Rarity+1);}
	};
	return Result;
};

int PickUnit(int Level, int Rarity)
{
//	char STR[100];
//	sprintf (STR,"Level: %d, Rare: %d...",Level,Rarity);
//	FileLog(STR);
	int Result=1, Count=0;
	bool bAdd;
	ClearRoll();
	for (int i=1; i<=iMaxUnitType; i++)
	{
		if (((pFirstUnitType+i)->iLevel==Level)&&((pFirstUnitType+i)->iUnitKind!=5))
		{
			bAdd=true;
			if (Rarity>0)
			{
				for (int BL=1; BL<=iMaxInnerBld; BL++)
				{
					if ((pFirstInnerBld+BL)->iGroup>0)
					{
						for (int A=0; A<MAX_BUILDING_ABILITY; A++)
						{
							if (((pFirstInnerBld+BL)->arAbility[A][0]==8)&&((pFirstInnerBld+BL)->arAbility[A][1]==i))
							{bAdd=false;};
						};
					};
				};
			};
			if (bAdd)
			{
				RollerN[Count]=i;
				RollerP[Count]=10;
				Count++;
			};
		};
	};
	if (Count>0)
	{
		Result=Roll(false);
//		sprintf (STR,"Done! Item: %s",(pFirstItem+Result)->sName);
//		FileLog(STR);
	};	
	return Result;
};

int PickSpell(int Level)
{
//	char STR[100];
//	sprintf (STR,"Level: %d, Rare: %d...",Level,Rarity);
//	FileLog(STR);
	int Result=1, Count=0;	
	ClearRoll();
	for (int i=1; i<=iMaxSpell; i++)
	{
		if ((pFirstSpell+i)->iLevel==Level)
		{			
			RollerN[Count]=i;
			RollerP[Count]=10;
			Count++;
		};
	};
	if (Count>0)
	{
		Result=Roll(false);
//		sprintf (STR,"Done! Item: %s",(pFirstItem+Result)->sName);
//		FileLog(STR);
	};	
	return Result;
};

int PickDefender(int Power)
{
	int Result=1, Count=0;	
	ClearRoll();
	for (int i=1; i<=iMaxDefender; i++)
	{
		if (((pFirstDefender+i)->iPower==Power)&&((pFirstDefender+i)->CheckAbility(10)>0))
		{			
			RollerN[Count]=i;
			RollerP[Count]=10;
			Count++;
		};
	};
	if (Count>0)
	{
		Result=Roll(false);
//		sprintf (STR,"Done! Item: %s",(pFirstItem+Result)->sName);
//		FileLog(STR);
	};	
	return Result;

};

void GeneratePrize(int Index, int Rarity)
{
	int Prize1=Random(100), Prize2=Random(100), Lev, Rar, Exp;
	switch (Index)
	{
	//Генерация предмета
	case 0:
		switch (Rarity)
		{
		case 0:
			if (Prize1<70)
			{Lev=2;}
			else
			{Lev=3;}
			Rar=1;
			if (Prize2>=40)
			{Rar=2;}
			if (Prize2>=80)
			{Rar=3;}						
			break;
		case 1:
			Lev=2;
			if (Prize1>=30)
			{Lev=3;}
			if (Prize1>=80)
			{Lev=4;}
			Rar=1;
			if (Prize2>=30)
			{Rar=2;}
			if (Prize2>=60)
			{Rar=3;}
			if (Prize2>=90)
			{Rar=4;}
			break;
		case 2:
			Lev=3;
			if (Prize1>=30)
			{Lev=4;}
			if (Prize1>=80)
			{Lev=5;}
			Rar=2;
			if (Prize2>=30)
			{Rar=3;}
			if (Prize2>=60)
			{Rar=4;}
			if (Prize2>=90)
			{Rar=5;}
			break;
		case 3:
			Lev=4;
			if (Prize1>=30)
			{Lev=5;}
			if (Prize1>=70)
			{Lev=6;}
			Rar=3;
			if (Prize2>=20)
			{Rar=4;}
			if (Prize2>=50)
			{Rar=5;}
			if (Prize2>=80)
			{Rar=6;}
			break;						
		}
		ipLottPrizeType=1;
		ipLottPrize=PickItem(Lev,Rar);
		break;
	//Генерация наёмника
	case 1:
		switch (Rarity)
		{
		case 0:
			if (Prize1<70)
			{Lev=1;}
			else
			{Lev=2;}						
			if (Prize2>=60)
			{Rar=1;}
			else
			{Rar=0;}
			Exp=1+Random(4);
			break;
		case 1:
			Lev=1;
			if (Prize1>=30)
			{Lev=2;}
			if (Prize1>=80)
			{Lev=3;}						
			if (Prize2>=50)
			{Rar=1;}
			else
			{Rar=0;}
			Exp=2+Random(5);
			break;
		case 2:
			Lev=2;
			if (Prize1>=20)
			{Lev=3;}
			if (Prize1>=70)
			{Lev=4;}						
			if (Prize2>=40)
			{Rar=1;}
			else
			{Rar=0;}
			Exp=4+Random(5);
			break;
		case 3:						
			if (Prize1<30)
			{Lev=3;}
			else						
			{Lev=4;}						
			if (Prize2>=30)
			{Rar=1;}
			else
			{Rar=0;}
			Exp=6+Random(5);
			break;
		};
		ipLottPrizeType=2;
		ipLottPrize=PickUnit(Lev,Rar);
		Exp-=(pFirstUnitType+ipLottPrize)->iLevel;
		int Exp1;
		if (Exp>0)
		{
			dcUnit *Temp=new dcUnit(ipLottPrize,0);
			Exp1=Temp->ExpForLevel(Exp)+Random(Temp->ExpForLevel(Exp+1)-Temp->ExpForLevel(Exp));
			delete Temp;
		}
		else
		{Exp1=0;}
		pLottUnit=new dcUnit(ipLottPrize,Exp1);
		pLottUnit->arMedal[0]=6;
		break;
	//Генерация заклинания
	case 2:
		switch (Rarity)
		{
		case 0:
			if (Prize1<50)
			{Lev=1;}
			else
			{Lev=2;}
			break;
		case 1:
			Lev=1;
			if (Prize1>=20)
			{Lev=2;}
			if (Prize1>=70)
			{Lev=3;}
			break;
		case 2:
			Lev=2;
			if (Prize1>=20)
			{Lev=3;}
			if (Prize1>=70)
			{Lev=4;}
			break;
		case 3:						
			if (Prize1<30)
			{Lev=3;}
			else						
			{Lev=4;}
			break;
		}
		ipLottPrizeType=3;
		ipLottPrize=PickSpell(Lev);
		break;
	case 3:
		if (Prize1<40)
		{Lev=Rarity+2;}
		else
		{Lev=Rarity+3;};
		ipLottPrizeType=4;
		ipLottPrize=PickDefender(Lev);
		break;
	case 4:
		if (Prize1<30)
		{GeneratePrize(0,Rarity);}
		else
		{
			if (Prize1<50)
			{GeneratePrize(1,Rarity);}
			else
			{
				if (Prize1<70)
				{GeneratePrize(2,Rarity);}
				else
				{
					if (Prize1<85)
					{GeneratePrize(3,Rarity);}
					else
					{
						if (Prize1<95)
						{
							ipLottPrizeType=5;
							switch (Rarity)
							{
							case 0:
								ipLottPrize=150+Random(150);
								break;
							case 1:
								ipLottPrize=500+Random(300);
								break;
							case 2:
								ipLottPrize=1500+Random(1000);
								break;
							case 3:
								ipLottPrize=4000+Random(3000);
								break;
							}
						}
						else
						{
							ipLottPrizeType=6;
							switch (Rarity)
							{
							case 0:
								ipLottPrize=50+Random(50);
								break;
							case 1:
								ipLottPrize=150+Random(100);
								break;
							case 2:
								ipLottPrize=450+Random(300);
								break;
							case 3:
								ipLottPrize=1200+Random(1000);
								break;
							}
						}
					}
				}
			}
		}
	};
};

bool WindowBuyTicket(int Index, int Rarity, BITMAP * Pic, float Price, wchar_t * Guid )
{
	bool Result=false;
	int x=512-bmpYuPlay[10]->w/2;
	int y=384-bmpYuPlay[10]->h/2;
	set_mouse_sprite(bmpMousePointer[1]);	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);	
	BITMAP *bmpWindow=create_bitmap(bmpYuPlay[10]->w,bmpYuPlay[10]->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);		
	draw_sprite(bmpWindow,bmpYuPlay[10],0,0);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+321,y+302,1,2,bmpButton[256],bmpButton[257],bmpButton[258],bmpButton[258],WObj);
	buttExit.iHint=135;
	buttExit.ipHotkey=KEY_ESC;
	UI_Button1 buttYu;
	WObj=buttYu.Init(x+95,y+302,1,2,bmpYuPlay[5],bmpYuPlay[6],bmpYuPlay[7],bmpYuPlay[8],WObj);
	buttYu.iHint=504;
	UI_TextBox txtDesc;
	WObj=txtDesc.Init(x+40,y+130,390,150,2,COLOR_1,Font2,WObj);
	txtDesc.Activate((pFirstMessage+115+Index)->msg);

	draw_sprite(bmpWindow,Pic,22,22);
	draw_sprite(bmpWindow,Pic,394,22);
	textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,31,COLOR_1,-1,(pFirstPhrase+514)->str);

	int R1=Price, R2=Price*100, R3=Price*1000;
	R2=R2%100;
	if (R3%10>4)
	{R2++;};	
	textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,64,COLOR_1,-1,"%s: %d.%.2d",(pFirstPhrase+312)->str, R1,R2);

	bool bCycle=true;

	while (bCycle)
	{
		if (buttExit.Check())
		{bCycle=false;};

		if (buttYu.Check())
		{
			YuClientRetCode Answer=yuplay_purchase_item(Guid);

			if (Answer==YUC_OK)
			{
				GeneratePrize(Index,Rarity);
				Result=true;
			}
			else
			{
				Result=false;
				if (Answer==YUC_NO_MONEY)
				{
					int An1=(pFirstDialog+1251)->Show(1,1,0,0,NULL,NULL);
					if (An1==1)
					{OpenYuPlaySite();};
				}
				else
				{(pFirstDialog+1252)->Show(1,1,0,0,NULL,NULL);};
			};
			bCycle=false;
		};

		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
			bHint=false;
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
//	blit(bmpBack,screen1,0,0,0,0,1024,768);
//	blit(bmpBack,screen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	return Result;
};

void WindowShowPrize()
{	
	int x=512-bmpYuPlay[11]->w/2;
	int y=384-bmpYuPlay[11]->h/2;
	set_mouse_sprite(bmpMousePointer[1]);	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);	
	BITMAP *bmpWindow=create_bitmap(bmpYuPlay[11]->w,bmpYuPlay[11]->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);		
	blit(bmpYuPlay[11],bmpWindow,0,0,0,0,bmpYuPlay[11]->w,bmpYuPlay[11]->h);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+123,y+239,1,2,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.iHint=135;
	buttExit.ipHotkey=KEY_ESC;
	UI_Mask mskPrize;

	textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,36,COLOR_1,-1,(pFirstPhrase+522)->str);
	switch (ipLottPrizeType)
	{
	case 1:
		draw_sprite(bmpWindow,bmpGUI_SpellSlot,bmpWindow->w/2-bmpGUI_SpellSlot->w/2,110);
		draw_sprite(bmpWindow,(pFirstItem+ipLottPrize)->Pic,bmpWindow->w/2-bmpGUI_SpellSlot->w/2+5,110+5);
		textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,212,COLOR_1,-1,(pFirstItem+ipLottPrize)->sName);
		WObj=mskPrize.Init(x+bmpWindow->w/2-bmpGUI_SpellSlot->w/2-1,y+110-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
		break;
	case 2:
		draw_sprite(bmpWindow,bmpGUI_UnitSlot,bmpWindow->w/2-bmpGUI_UnitSlot->w/2,95);
		draw_sprite(bmpWindow,(pFirstUnitType+ipLottPrize)->Pic,bmpWindow->w/2-(pFirstUnitType+ipLottPrize)->Pic->w/2,140-(pFirstUnitType+ipLottPrize)->Pic->h/2);
		textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,212,COLOR_1,-1,(pFirstUnitType+ipLottPrize)->sName);
		WObj=mskPrize.Init(x+bmpWindow->w/2-bmpGUI_UnitSlot->w/2-6,y+95-6,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,WObj,2);
		break;
	case 3:
		draw_sprite(bmpWindow,bmpGUI_SpellSlot,bmpWindow->w/2-bmpGUI_SpellSlot->w/2,110);
		draw_sprite(bmpWindow,(pFirstSpell+ipLottPrize)->Pic,bmpWindow->w/2-bmpGUI_SpellSlot->w/2+5,110+5);
		textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,212,COLOR_1,-1,(pFirstSpell+ipLottPrize)->sName);
		WObj=mskPrize.Init(x+bmpWindow->w/2-bmpGUI_SpellSlot->w/2-1,y+110-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
		break;
	case 4:
		draw_sprite(bmpWindow,bmpGUI_SpellSlot,bmpWindow->w/2-bmpGUI_SpellSlot->w/2,110);
		draw_sprite(bmpWindow,(pFirstDefender+ipLottPrize)->Pic,bmpWindow->w/2-bmpGUI_SpellSlot->w/2+5,110+5);
		textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,212,COLOR_1,-1,(pFirstDefender+ipLottPrize)->sName);
		WObj=mskPrize.Init(x+bmpWindow->w/2-bmpGUI_SpellSlot->w/2-1,y+110-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
		break;
	case 5:
		draw_sprite(bmpWindow,bmpGUI_SpellSlot,bmpWindow->w/2-bmpGUI_SpellSlot->w/2,110);
		draw_sprite(bmpWindow,bmpStuff[5],bmpWindow->w/2-bmpGUI_SpellSlot->w/2+5,110+5);
		textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,212,COLOR_1,-1,"%s: %d",(pFirstPhrase+189)->str,ipLottPrize);
		WObj=mskPrize.Init(x+bmpWindow->w/2-bmpGUI_SpellSlot->w/2-1,y+110-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
		break;
	case 6:
		draw_sprite(bmpWindow,bmpGUI_SpellSlot,bmpWindow->w/2-bmpGUI_SpellSlot->w/2,110);
		draw_sprite(bmpWindow,bmpStuff[6],bmpWindow->w/2-bmpGUI_SpellSlot->w/2+5,110+5);
		textprintf_centre_ex(bmpWindow,Font1,bmpWindow->w/2,212,COLOR_1,-1,"%s: %d",(pFirstPhrase+190)->str,ipLottPrize);
		WObj=mskPrize.Init(x+bmpWindow->w/2-bmpGUI_SpellSlot->w/2-1,y+110-1,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
		break;
	};

	int Check;
	PlaySpellSound((pFirstRitual+9)->Sound);

	while (!buttExit.Check())
	{
		Check=mskPrize.Check();
		if (Check>1)
		{
			switch (ipLottPrizeType)
			{
			case 1:
				if (Check>10)
				{
					bHint=false;
					WObj=ShowActiveObjects(WObj,6,0);
					WindowItemInfo(ipLottPrize,(pFirstItem+ipLottPrize)->iDurability);
				}
				else
				{
					MakeItemHint(ipLottPrize,(pFirstItem+ipLottPrize)->iDurability,NULL,0,0,0);
					bHint=true;
				};
				break;
			case 2:
				if (Check>10)
				{
					bHint=false;
					WObj=ShowActiveObjects(WObj,6,0);
					WindowUnitInfo(pLottUnit,0);
				}
				break;
			case 3:
				if (Check>10)
				{
					bHint=false;
					WObj=ShowActiveObjects(WObj,6,0);
					WindowSpellInfo(ipLottPrize);
				}
				break;
			case 4:
				if (Check>10)
				{
					bHint=false;
					WObj=ShowActiveObjects(WObj,6,0);
					WindowDefenderInfo(ipLottPrize, 0, 0);
				}
				break;
			};
		};
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
			bHint=false;
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
};

//void WindowLottery1()
//{};

void YuPlayBuyGame(bool bBuyer)
{
	wchar_t tcGameId[60]={L"7991F05F-63C3-4232-92C3-B178D23C9EB7"};
//	wchar_t tcGameId[60]={L"CBCE8555-F02F-41FA-BC17-3846EBD4094E"};
	NewProgress=0; OldProgress=0;
	RunHourGlass(153, 0);
	if (yuStatus==YUC_YUPLAY_INSTALLED)
	{		
		if (!yuplay_client_execute())
		{
			int Wait=50;
			while ((Wait>0)&&(yuplay_get_yuplay_client_status()==YUC_YUPLAY_INSTALLED))
			{
				if (Timer)
				{
					Timer=0;
					Wait--;
				};
			};
			if (yuplay_get_yuplay_client_status()==YUC_YUPLAY_INSTALLED)
			{
				bHourGlass=false;
				(pFirstDialog+1247)->Show(1,1,0,0,NULL,NULL);			
				return;
			};
		};
	};	
	if (yuplay_login()!=YUC_OK)
	{
		bHourGlass=false;
		(pFirstDialog+1248)->Show(1,1,0,0,NULL,NULL);		
		return;
	};
	yuplay_get_yuplay_logged_user_nick();
	YuClientItemInfo * yuGoods=new YuClientItemInfo;
	if (yuplay_get_item_information(tcGameId,yuGoods)!=YUC_OK)
	{
		bHourGlass=false;
		(pFirstDialog+1248)->Show(1,1,0,0,NULL,NULL);		
		return;
	}; 
	bHourGlass=false;

	if (yuplay_check_item_owner(tcGameId)==YUC_OK)
	{
		const wchar_t * tcUser=yuplay_get_logged_user_nick();
		PACKFILE * WorkFile=pack_fopen("key.dat","w");
		char eMail[30];
		long int iKey1;
		for (int i=0; i<30; i++)
		{eMail[i]=tcUser[i];};
		iKey1=MakeKey(eMail);
		pack_fwrite(eMail,sizeof eMail,WorkFile);
		pack_fwrite(&iKey1,sizeof iKey1,WorkFile);
		pack_fclose(WorkFile);		
		bDemo=false;
		(pFirstDialog+1256)->Show(1,1,0,0,NULL,NULL);
	}
	else
	{
	if (bBuyer)
	{
	char Txt[300];
	int R1=yuGoods->price, R2=yuGoods->price*100;
	R2=R2%100;
	int Rur=yuGoods->price*850;
	sprintf(Txt,(pFirstDialog+1255)->sText,R1,R2,Rur/100,Rur%100);
	int AN=(pFirstDialog+1255)->Show(1,1,0,0,NULL,Txt);
	if (AN==1)
	{
	YuClientRetCode Answer=yuplay_purchase_item(tcGameId);
	
	if (Answer==YUC_OK)
	{
		const wchar_t * tcUser=yuplay_get_logged_user_nick();
		PACKFILE * WorkFile=pack_fopen("key.dat","w");
		char eMail[30];
		long int iKey1;
		for (int i=0; i<30; i++)
		{eMail[i]=tcUser[i];};
		iKey1=MakeKey(eMail);
		pack_fwrite(eMail,sizeof eMail,WorkFile);
		pack_fwrite(&iKey1,sizeof iKey1,WorkFile);
		pack_fclose(WorkFile);		
		bDemo=false;
		(pFirstDialog+1256)->Show(1,1,0,0,NULL,NULL);
	}
	else
	{		
		if (Answer==YUC_NO_MONEY)
		{
			int An1=(pFirstDialog+1251)->Show(1,1,0,0,NULL,NULL);
			if (An1==1)
			{OpenYuPlaySite();};

		}
		else
		{(pFirstDialog+1252)->Show(1,1,0,0,NULL,NULL);};
	};
	}
	}
	}
};

void WindowLottery(int Index)
{

	wchar_t tcStock[20][60]=
	{
		{L"EF853ABA-DD68-4946-8BEE-16B70CF0EA98"},
		{L"7A3CCBC8-6A33-4C2A-8504-AE7E11F1D446"},
		{L"0983143E-6338-4B60-8DBB-6071BD978733"},
		{L"187FF4E0-0ED5-44DE-BF1C-B2AE837D243C"},

		{L"10622FC2-4C0C-442E-81AA-23BAA5440AD8"},
		{L"E0064232-B140-4BF4-97DD-DA9F53A8C309"},
		{L"AD0DE0DC-D7B3-47B0-B144-8F328D62780A"},
		{L"CC4E7284-FCF8-4B8D-994C-F0FEDFE959B9"},

		{L"4DFC92F1-047B-4175-AE4D-2990CC3363ED"},
		{L"3C5B05C6-9D0E-4BD8-A21A-6BF5E9275811"},
		{L"C5985502-42F9-4AD3-A9D6-5F433AF2F6F5"},
		{L"A3077710-DF2A-4C3C-8B2E-591B4F0F55E8"},

		{L"48956B3F-35DB-4C1B-90F4-CF5E39DE20AA"},
		{L"D99DEF76-ABFC-4D8F-A28F-72BD05F61B9A"},
		{L"15A37FF1-B0BD-49BC-8BFE-FB0C9440FF98"},
		{L"F415497B-81AF-4263-B85C-6F12BAEE65E4"},

		{L"EA8E50EE-D8CF-492B-843B-8A3E44810BEC"},
		{L"EF661726-DFE9-4BB1-8F96-17215796CA08"},
		{L"7646EFD7-5104-4B13-814A-877A01C62C2C"},
		{L"E4DBCC24-F362-44A0-B950-0224DA2233ED"}
	};
//	tcStock[0]=L"123";
	


	ipLottPrize=0; ipLottPrizeType=0;
	const int dx=63;
	const int dy=82;
	NewProgress=0; OldProgress=0;
	RunHourGlass(153, 0);
	if (yuStatus==YUC_YUPLAY_INSTALLED)
	{		
		if (!yuplay_client_execute())
		{
			int Wait=50;
			while ((Wait>0)&&(yuplay_get_yuplay_client_status()==YUC_YUPLAY_INSTALLED))
			{
				if (Timer)
				{
					Timer=0;
					Wait--;
				};
			};
			if (yuplay_get_yuplay_client_status()==YUC_YUPLAY_INSTALLED)
			{
				bHourGlass=false;
				(pFirstDialog+1247)->Show(1,1,0,0,NULL,NULL);			
				return;
			};
		};
	};
	NewProgress+=2000;
	if (yuplay_login()!=YUC_OK)
	{		
		bHourGlass=false;
		(pFirstDialog+1248)->Show(1,1,0,0,NULL,NULL);		
		return;
	};	
	if (bNeedLoadTicket)
	{	
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<5; j++)
		{
			arTicket[i][j]=new YuClientItemInfo;
	//		YuClientRetCode CCC;
			if (yuplay_get_item_information(tcStock[i+j*4],arTicket[i][j])!=YUC_OK)
			{
	//			CCC=yuplay_get_item_information(tcStock[i+j*4],arTicket[i][j]);

	//			int RR;
				bHourGlass=false;
				(pFirstDialog+1248)->Show(1,1,0,0,NULL,NULL);		
				return;
			}; 
			NewProgress+=1000;
		//	arTicket[i][j]->price=Random(10)+1;
		};
	};
	bNeedLoadTicket=false;
	};



	bHourGlass=false;

	int x=512-bmpYuPlay[4]->w/2;
	int y=384-bmpYuPlay[4]->h/2;
//	set_mouse_sprite(bmpMousePointer[1]);	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	set_trans_blender(128,128,128,128);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpYuPlay[4]->w,bmpYuPlay[4]->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);		
	draw_sprite(bmpWindow,bmpYuPlay[4],0,0);
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+285,y+449,1,2,bmpButton[256],bmpButton[257],bmpButton[258],bmpButton[258],WObj);
	buttExit.iHint=135;
	buttExit.ipHotkey=KEY_ESC;
	UI_Button1 buttYu;
	WObj=buttYu.Init(x+55,y+449,1,2,bmpYuPlay[5],bmpYuPlay[6],bmpYuPlay[7],bmpYuPlay[8],WObj);
	buttYu.iHint=523;

//	buttExit.iHint=135;
//	buttExit.ipHotkey=KEY_ESC;
	BITMAP *bmpTemp=create_bitmap(bmpYuPlay[9]->w,bmpYuPlay[9]->h);
	
	UI_Mask mskTicket[4][5];
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<5; j++)
		{
			WObj=mskTicket[i][j].Init(x+79+i*dx,y+39+j*dy,1,bmpYuPlay[9],bmpYuPlay[9],WObj,2);
			mskTicket[i][j].G_Obj->iOpacity=70;
			mskTicket[i][j].G_Obj->bTrans=true;
		};
	};

	textprintf_centre_ex(bmpWindow,Font1,bmpYuPlay[4]->w/2,18,COLOR_1,-1,(pFirstPhrase+513)->str);
	int Check;
	bool bCycle=true;

	if (Index>0)
	{
		blit(bmpYuPlay[4],bmpWindow,0,448,0,118,bmpYuPlay[4]->w,51);
		rectfill(bmpWindow,0,118+51,bmpYuPlay[4]->w,bmpYuPlay[4]->h,COLOR_0);
		if (Index>1)
		{
			blit(bmpYuPlay[4],bmpWindow,16,38+(Index-1)*82,16,38,368,81);
		};
		buttYu.G_Obj->y=119+y;
		buttExit.G_Obj->y=119+y;
	};

	while (bCycle)
	{
		if (buttExit.Check())
		{bCycle=false;}
		if (buttYu.Check())
		{WindowInfo((pFirstPhrase+513)->str,(pFirstMessage+114)->msg,bmpYuPlay[12]);}
		for (int i=0; i<4; i++)
		{
			for (int j=0; ((j<5)&&((j<1)||(Index==0))); j++)
			{
				Check=mskTicket[i][j].Check();
				if (Check>1)
				{
					if (Check>10)
					{
						blit(bmpWindow,bmpTemp,79+i*dx,39+j*dy,0,0,bmpYuPlay[9]->w,bmpYuPlay[9]->h);
						bHint=false;
						WObj=ShowActiveObjects(WObj,6,0);
						bool Buyer;
						if (Index==0)
						{Buyer=WindowBuyTicket(j,i,bmpTemp,arTicket[i][j]->price,tcStock[i+j*4]);}
						else
						{Buyer=WindowBuyTicket(Index-1,i,bmpTemp,arTicket[i][Index-1]->price,tcStock[i+(Index-1)*4]);}
						if (Buyer)
						{
							WindowShowPrize();
							bCycle=false;
						}
					}
					else
					{
						if (j<4)
						{
							if (Index==0)
							{MakeTicketHint(j*4+i,arTicket[i][j]->price,i+1);}
							else
							{MakeTicketHint((Index-1)*4+i,arTicket[i][Index-1]->price,i+1);}
						}
						else
						{MakeTicketHint(16,arTicket[i][j]->price,i+1);};
						bHint=true;
					};
				};
			};
		};
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
			bHint=false;
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
	destroy_bitmap(bmpTemp);
	set_mouse_sprite(OldMouse);	
	//for (int i=0; i<4; i++)
	//{
	//	for (int j=0; j<5; j++)
	//	{
	//		delete arTicket[i][j];
	//	};
	//};
};

#endif