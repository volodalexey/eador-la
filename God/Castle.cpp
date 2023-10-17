#include "Windower.h"

bool GroupIsFull(int Player, int Group)
{
	if (Player<0)
	{return false;};
	int Count=0;
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if (((pFirstInnerBld+i)->iGroup==Group)&&(arPlayer[Player].arBld[i]))
		{Count++;};
	};
	if (Count>=arBldGroup[Group])
	{return true;}
	else
	{return false;};
};

int IsParent(int Father, int Son, int Player)
{
	for (int i=0; i<MAX_BUILDING_NEED; i++)
	{
		if ((pFirstInnerBld+Son)->arBuildings[i]>0)
		{
		if ((pFirstInnerBld+Son)->arBuildings[i]<1000)
		{
			if ((pFirstInnerBld+Son)->arBuildings[i]==Father)
			{return 1;}
			else
			{
				if (IsParent(Father, (pFirstInnerBld+Son)->arBuildings[i],Player))
				{return 1;};
			};
		}
		else
		{
			if (!GroupIsFull(Player,(pFirstInnerBld+Son)->arBuildings[i]-1000))
			{
			for (int j=1; j<=iMaxInnerBld; j++)
			{
				if ((pFirstInnerBld+j)->iGroup==(pFirstInnerBld+Son)->arBuildings[i]-1000)
				{
					if (j==Father)
					{return 2;}
					else
					{
						if (IsParent(Father,j,Player))
						{return 2;};
					};
				};
			};
			};

		};
		};
	};
	return 0;
};

bool IsRestricted(int Build, int Player)
{	
	if (arPlayer[Player].arBld[Build])
	{return false;};
	if ((CurGame.iGameType==2)&&((pFirstInnerBld+Build)->iLevel!=0))
	{
		int Num=0;
	//	if (arPlayer[Player].Control>1)
	//	{Num=arPlayer[Player].ipProfile;};		
		dcShard *SH=pFirstShard;
		bool Restrict=true;
		while (SH)
		{
			if ((SH->ipPlayer==Num)&&(Restrict))
			{
				for (int i=0; i<MAX_SHARD_BUILD; i++)
				{
					if (SH->arBuilding[i]==Build)
					{Restrict=false;};
				};				
			};
			SH=SH->pNext;
		};
		if (Campaign.arCampPlayer[0].arBld[Build]>0)
		{Restrict=false;};
		if (Restrict)
		{return true;};
	};
	for (int k=0; k<MAX_BUILDING_ABILITY; k++)
	{
		if (((pFirstInnerBld+Build)->arAbility[k][0]==47)&&(arPlayer[Player].iUnion!=(pFirstInnerBld+Build)->arAbility[k][1]))
		{return true;};
	};
	if (((pFirstInnerBld+Build)->iGroup>0)&&(GroupIsFull(Player,(pFirstInnerBld+Build)->iGroup)))
	{return true;}
	else
	{
		for (int i=0; i<MAX_BUILDING_NEED; i++)
		{
			if ((pFirstInnerBld+Build)->arBuildings[i]>0)
			{
				if ((pFirstInnerBld+Build)->arBuildings[i]<1000)
				{
					if (IsRestricted((pFirstInnerBld+Build)->arBuildings[i],Player))
					{return true;}
				};
			};
		};
	};
	return false;
};

BITMAP * MakeCastlePic(int Player)
{
	BITMAP *Pic=create_bitmap(512,768);
	bool bAnti, bBld;
	int XP,YP, Build, arBLD[MAX_BLD];	
	for (int i=0; i<=iMaxCastle; i++)
	{
		arBLD[i]=-100;
		if ((pFirstCastle+i)->iGroup>0)
		{
			YP=-333;Build=(pFirstCastle+i)->arBuild[0];
			while (YP==-333)
			{
				if ((pFirstInnerBld+Build)->iUpg>0)
				{Build=(pFirstInnerBld+Build)->iUpg;}
				else
				{
					int iFree=-1;
					for (int j=0; j<MAX_GROUP_BUILD; j++)
					{
						if ((iFree<0)&&(arPlayer[Player].arGroup[(pFirstCastle+i)->iGroup][j]==0))
						{iFree=j;};
						if (arPlayer[Player].arGroup[(pFirstCastle+i)->iGroup][j]==Build)
						{									
							YP=arGroupCoord[(pFirstCastle+i)->iGroup][j][1];
						};
					};
					if (YP==-333)
					{
						if (iFree>=0)
						{
							YP=arGroupCoord[(pFirstCastle+i)->iGroup][iFree][1];						
						}
						else
						{
							YP=-100;
						};
					};					
				};				
			};
			arBLD[i]=YP;					
		}
		else
		{arBLD[i]=(pFirstCastle+i)->Yplace;};		
	};	
	for (int Layer=0; Layer<3; Layer++)
	{		
		for (int y=0; y<768; y++)
		{
			for (int i=0; i<=iMaxCastle; i++)
			{
				if (((pFirstCastle+i)->iLayer==Layer)&&(arBLD[i]==y))
				{
					
			/*for (int i=0; i<=iMaxCastle; i++)
			{
				if (((pFirstCastle+i)->iLayer==Layer)&&((pFirstCastle+i)->Yplace==y))
				{*/
					bBld=false; bAnti=false;
					for (int BL=0; BL<MAX_CASTLE_BLD; BL++)
					{
						if (((pFirstCastle+i)->arAntiBuild[BL]>0)&&(arPlayer[Player].arBld[(pFirstCastle+i)->arAntiBuild[BL]]))
						{bAnti=true;};
						if (((pFirstCastle+i)->arBuild[BL]>0)&&(arPlayer[Player].arBld[(pFirstCastle+i)->arBuild[BL]]))
						{bBld=true;};
						if (!(arPlayer[Player].bEnter)&&((pFirstCastle+i)->arBuild[BL]<0))
						{bBld=true;};
					};
					if ((pFirstCastle+i)->arBuild[0]==0)
					{bBld=true;};
					if ((bBld)&&(!bAnti))
					{
						if ((pFirstCastle+i)->iGroup>0)
						{
							XP=0;YP=0;Build=(pFirstCastle+i)->arBuild[0];
							while (XP==0)
							{
								if ((pFirstInnerBld+Build)->iUpg>0)
								{Build=(pFirstInnerBld+Build)->iUpg;}
								else
								{
									int iFree=-1;
									for (int j=0; j<MAX_GROUP_BUILD; j++)
									{
										if ((iFree<0)&&(arPlayer[Player].arGroup[(pFirstCastle+i)->iGroup][j]==0))
										{iFree=j;};
										if (arPlayer[Player].arGroup[(pFirstCastle+i)->iGroup][j]==Build)
										{
											XP=arGroupCoord[(pFirstCastle+i)->iGroup][j][0];
											YP=arGroupCoord[(pFirstCastle+i)->iGroup][j][1];
										};
									};
									if (XP==0)
									{
										if (iFree>=0)
										{
											XP=arGroupCoord[(pFirstCastle+i)->iGroup][iFree][0];
											YP=arGroupCoord[(pFirstCastle+i)->iGroup][iFree][1];
											arPlayer[Player].arGroup[(pFirstCastle+i)->iGroup][iFree]=Build;
										}
										else
										{
											XP=-100;
											YP=-100;
										};
									};									
								};
							};
							XP-=(pFirstCastle+i)->x;
							YP-=(pFirstCastle+i)->y;
						}
						else
						{
							XP=(pFirstCastle+i)->x;
							YP=(pFirstCastle+i)->y;
						};
						if ((pFirstCastle+i)->iOpacity>0)
						{
							int Blend=255*(pFirstCastle+i)->iOpacity/100;
							set_trans_blender(Blend,Blend,Blend,Blend);
							draw_trans_sprite(Pic,(pFirstCastle+i)->Pic,XP,YP);
						}
						else
						{
							draw_sprite(Pic,(pFirstCastle+i)->Pic,XP,YP);
						};
					};
				};
			};
		};
	};
	set_trans_blender(128,128,128,128);
	return Pic;
};

//***************************************************
//*** Экран строительства в замке *******************
//***************************************************
int ScreenCastleBuild (int Player)
{
	iAntisleep=iAntisleepDelay;
	const int BLD=63, dx=58;
	int arSlot[BLD][2] = {
		{56, 14}, {401, 14},	//0, 1 
		{16, 80}, {16, 137},	//2, 3
		{95, 80}, {95+dx, 80}, {95+dx*2, 80}, {95+dx*3, 80}, {95+dx*4, 80}, {95+dx*5, 80}, {95+dx*6, 80}, //4-10
		{95, 137}, {95+dx, 137}, {95+dx*2, 137}, {95+dx*3, 137}, {95+dx*4, 137}, {95+dx*5, 137}, //11-16
		{172, 213}, {172+dx, 213}, {172+dx*2, 213}, //17-19
		{28, 270}, {28+dx, 270}, {28+dx*2, 270}, {28+dx*3, 270}, {28+dx*4, 270}, {28+dx*5, 270}, {28+dx*6, 270}, {28+dx*7, 270}, //20-27
		{16, 346}, {16+dx, 346}, {150, 346}, {150+dx, 346}, {150+dx*2, 346}, {150+dx*3, 346}, {150+dx*4, 346}, {150+dx*5, 346}, //28-35
		{56, 421}, {56+dx, 421}, {56+dx*2, 421}, {56, 478}, {56+dx, 478}, {56+dx*2, 478},	//36-41
		{289, 421}, {289+dx, 421}, {289+dx*2, 421}, {319, 478}, {319+dx, 478},	//42-46
		{18, 553}, {18+dx, 553}, {18+dx*2, 553}, {18+dx*3, 553}, {18+dx*4, 553},	//47-51
		{323, 553}, {323+dx, 553}, {323+dx*2, 553},		//52-54
		{18, 629}, {18+dx, 629}, {18+dx*2, 629}, {18+dx*3, 629}, {18+dx*4, 629},	//55-59
		{323, 629}, {323+dx, 629}, {323+dx*2, 629}		//60-62
	};
	int arBld[BLD];
	int AnimOpa=0, Result=999;
	GUI_ResourceBlock blResource;	
	BITMAP *bmpBuild=create_bitmap(512,685);
	BITMAP *bmpBldInfo=create_bitmap(bmpGUI_BuildBar->w,bmpGUI_BuildBar->h);
	BITMAP *bmpCastleView=MakeCastlePic(Player);//=create_bitmap(512,768);
	BITMAP *bmpOldCastle=NULL;
	BITMAP *bmpLocked=create_bitmap(52,52);
	rectfill(bmpLocked,0,0,52,52,COLOR_1);	
//	blit(bmpGUI_Castle,bmpCastleView,0,0,0,0,512,768);
	G_Object *objGUI=new G_Object(NULL,512,0,2,bmpCastleView);
	G_Object *objCastle=objGUI;
	objGUI=new G_Object(objGUI,512,0,1,bmpCastleView);
	G_Object *objOldCastle=objGUI;

	objGUI=new G_Object(objGUI,532,20,-1,bmpGUI_Hint);
	G_Object *objTutorial=objGUI;
	UI_Mask mskTutorial;
	objGUI=mskTutorial.Init(532,20,1,bmpGUI_Hint,bmpEmpty,objGUI,-1);
	UI_TextBox txtTutorial;
	objGUI=txtTutorial.Init(552,50,297,200,7,COLOR_1,Font2,objGUI);
	txtTutorial.bShadow=false;
	txtTutorial.Hide();
	bool bTutHint=false;

	if (arPlayer[Player].bEnter)
	{
		arPlayer[Player].bEnter=false;
		bmpOldCastle=bmpCastleView;
		objOldCastle->Pic=bmpOldCastle;
		bmpCastleView=MakeCastlePic(Player);
		objCastle->Pic=bmpCastleView;
		AnimOpa=200;
	};

	objGUI=new G_Object(objGUI,0,0,1,bmpGUI_Build);	
	objGUI=blResource.Init(&arPlayer[Player],0,734,objGUI);
	objGUI=new G_Object(objGUI,0,0,2,bmpBuild);
	objGUI=new G_Object(objGUI,256-bmpGUI_BuildBar->w/2,384-bmpGUI_BuildBar->h/2,-1,bmpBldInfo);
	G_Object* objBLD=objGUI;
	UI_Mask mskBld[BLD];
	for (int i=0; i<BLD; i++)
	{objGUI=mskBld[i].Init(arSlot[i][0]-6,arSlot[i][1]-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,3);};
	UI_Button1 buttExit;
	objGUI=buttExit.Init(451,686,1,3,bmpButton[256],bmpButton[257],bmpButton[258],bmpButton[258],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;

	UI_Button1 buttBuild;	
	objGUI=buttBuild.Init(61,686,-2,3,bmpButton[267],bmpButton[268],bmpButton[269],bmpButton[270],objGUI);

	UI_Button1 buttHero;
	objGUI=buttHero.Init(121,686,1,3,bmpButton[250],bmpButton[251],bmpButton[252],bmpButton[253],objGUI);

	
	UI_Button1 buttGarrison;
	objGUI=buttGarrison.Init(181,686,1,3,bmpButton[263],bmpButton[264],bmpButton[265],bmpButton[266],objGUI);
	buttGarrison.iHint=94;
	buttGarrison.ipHotkey=KEY_D;

	UI_Button1 buttTreasury;
	objGUI=buttTreasury.Init(241,686,1,3,bmpButton[259],bmpButton[260],bmpButton[261],bmpButton[262],objGUI);	
	buttTreasury.iHint=102;
	buttTreasury.ipHotkey=KEY_A;

	UI_Button1 buttLibrary;
	objGUI=buttLibrary.Init(301,686,1,3,bmpButton[271],bmpButton[272],bmpButton[273],bmpButton[274],objGUI);	
	buttLibrary.iHint=318;
	buttLibrary.ipHotkey=KEY_S;

	UI_Button1 buttStat;
	objGUI=buttStat.Init(361,686,1,3,bmpButton[275],bmpButton[276],bmpButton[277],bmpButton[278],objGUI);	
	buttStat.iHint=99;



	if (Tutorial.Map)
	{
		buttHero.Activate(-1);
		if (!Tutorial.buttStat)
		{buttStat.Activate(-1);};
	}
	else
	{buttHero.Activate(1);};
	buttHero.iHint=136;
	buttHero.ipHotkey=KEY_F;

	

	if ((Tutorial.Map)&&(Tutorial.Build==0))
	{
		bTutHint=true;
		ScrollSound();
		objTutorial->iLayer=5;
		txtTutorial.Activate((pFirstTutText+52)->msg);
		Tutorial.Build=1;
		buttExit.Activate(-1);
	};

	if ((Tutorial.Map)&&(Tutorial.Build==5))
	{
		bTutHint=true;
		ScrollSound();
		objTutorial->iLayer=5;
		txtTutorial.Activate((pFirstTutText+58)->msg);
		Tutorial.Build=6;
	};

	if (Tutorial.Hero==1)
	{
		bTutHint=true;
		ScrollSound();
		objTutorial->iLayer=5;
		txtTutorial.Activate((pFirstTutText+74)->msg);
		Tutorial.Hero=2;
		buttHero.bBlink=true;
	};

	if ((!Tutorial.Map)&&(Tutorial.Mode)&&(Tutorial.Groups<=1))
	{
		int Count=0;
		for (int i=1; i<=iMaxInnerBld; i++)
		{
			if ((arPlayer[Player].arBld[i])&&((pFirstInnerBld+i)->iGroup==1))
			{Count++;};
		};
		if (Count>=3)
		{
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+83)->msg);
			Tutorial.Groups=1;
		};
	};	

	rectfill(bmpBuild,0,0,bmpBuild->w,bmpBuild->h,COLOR_0);
	textprintf_centre_ex(bmpBuild,Font1,252,19,COLOR_1,-1,(pFirstPhrase+232)->str);
	textprintf_centre_ex(bmpBuild,Font1,252,64,COLOR_1,-1,(pFirstPhrase+233)->str);
	textprintf_centre_ex(bmpBuild,Font1,252,198,COLOR_1,-1,(pFirstPhrase+234)->str);
	textprintf_centre_ex(bmpBuild,Font1,252,331,COLOR_1,-1,(pFirstPhrase+235)->str);
	textprintf_centre_ex(bmpBuild,Font1,134,406,COLOR_1,-1,(pFirstPhrase+236)->str);
	textprintf_centre_ex(bmpBuild,Font1,373,406,COLOR_1,-1,(pFirstPhrase+237)->str);
	textprintf_centre_ex(bmpBuild,Font1,150,538,COLOR_1,-1,(pFirstPhrase+238)->str);
	textprintf_centre_ex(bmpBuild,Font1,408,538,COLOR_1,-1,(pFirstPhrase+239)->str);
	textprintf_centre_ex(bmpBuild,Font1,150,614,COLOR_1,-1,(pFirstPhrase+240)->str);
	textprintf_centre_ex(bmpBuild,Font1,408,614,COLOR_1,-1,(pFirstPhrase+241)->str);
	bool bCycle=true;

	while (bCycle)
	{

	if ((Tutorial.Mode)&&(Tutorial.Alchemy==0)&&(arPlayer[Player].arBld[103]))
	{
		bTutHint=true;
		ScrollSound();
		objTutorial->iLayer=5;
		txtTutorial.Activate((pFirstTutText+94)->msg);
		Tutorial.Alchemy=1;
	};
	if (arPlayer[Player].iPlanned>0)
	{
		if ((arPlayer[Player].arBld[arPlayer[Player].iPlanned])||(arPlayer[Player].CanBuild(arPlayer[Player].iPlanned))||(IsRestricted(arPlayer[Player].iPlanned, Player)))
		{
			rectfill(bmpBuild,arSlot[0][0],arSlot[0][1],arSlot[0][0]+52,arSlot[0][1]+52,COLOR_0);
			arPlayer[Player].iPlanned=0;
		}
		else
		{
			draw_sprite(bmpBuild,(pFirstInnerBld+arPlayer[Player].iPlanned)->Pic,arSlot[0][0],arSlot[0][1]);
			draw_sprite(bmpBuild,bmpGUI_Planned,arSlot[0][0],arSlot[0][1]);
		};		
	};
	for (int i=2; i<BLD; i++)
	{
		arBld[i]=0;
		if ((i>=4)&&(i<=16))
		{
			//Военный квартал
			bool bSearcher=true;
			for (int GR=0; ((GR<4)&&(bSearcher)); GR++)
			{
				int SlotNum, GroupNum;
				switch (GR)
				{
				case 0:
					SlotNum=8;
					GroupNum=1;
					break;
				case 1:
					SlotNum=11;
					GroupNum=3;
					break;
				case 2:
					SlotNum=13;
					GroupNum=7;
					break;
				case 3:
					SlotNum=14;
					GroupNum=11;
					break;
				};
				if (GroupIsFull(Player,GroupNum))
				{
					if (i<SlotNum)
					{
						int Slot=2; bool Search=true, Inside=false;
						while ((Slot<17)&&(Search))
						{
							for (int k=1; k<=iMaxInnerBld; k++)
							{
								if ((pFirstInnerBld+k)->iSlot==Slot)
								{
									if ((pFirstInnerBld+k)->iGroup==GroupNum)
									{
										Inside=true;
										if(arPlayer[Player].arBld[k])
										{
											bool bFound=true;										
											for (int m=2; m<i; m++)
											{
												if (arBld[m]==k)
												{bFound=false;};
												int BL=arBld[m];
												while ((pFirstInnerBld+BL)->iUpg>0)
												{
													if ((pFirstInnerBld+BL)->iUpg==k)
													{bFound=false;};
													BL=(pFirstInnerBld+BL)->iUpg;
												};
											};
											if (bFound)
											{
												arBld[i]=k;
												bSearcher=false;
												Search=false;
												break;
											};
										};
									}
									else
									{
									//	if (Inside)
									//	{Search=false;};
									};
								};
							};
							Slot++;
						};
					}
				}
				else
				{
					for (int j=1; j<=iMaxInnerBld; j++)
					{
						if (((pFirstInnerBld+j)->iSlot==i)&&((pFirstInnerBld+j)->iUpg==0)&&((pFirstInnerBld+j)->iGroup==GroupNum)&&(!IsRestricted(j,Player)))
						{
							arBld[i]=j;							
							break;
						};
					};
					bSearcher=false;
				};
			};
		}
		else
		{
			//Квартал союзников
			if (i>59)
			{
				for (int j=1; j<=iMaxInnerBld; j++)
				{
					if (((pFirstInnerBld+j)->iSlot==i)&&(!IsRestricted(j,Player)))
						//&&((arPlayer[Player].arBld[j])||(arPlayer[Player].CanBuild(j))))
					{
						{arBld[i]=j;};
						break;
					};
				};
			}
			else
			{
				//Остальные кварталы
			for (int j=1; j<=iMaxInnerBld; j++)
			{
				if (((pFirstInnerBld+j)->iSlot==i)&&((pFirstInnerBld+j)->iUpg==0)&&(!IsRestricted(j,Player)))
				{
					if (((pFirstInnerBld+j)->iGroup>0)&&(GroupIsFull(Player,(pFirstInnerBld+j)->iGroup)))
					{
						int Slot=2; bool Search=true, Inside=false;
						while (Search)
						{
						for (int k=1; k<=iMaxInnerBld; k++)
						{
							if ((pFirstInnerBld+k)->iSlot==Slot)
							{
								if ((pFirstInnerBld+j)->iGroup==(pFirstInnerBld+k)->iGroup)
								{
								if(arPlayer[Player].arBld[k])
								{
									bool bFound=true;
									Inside=true;
									for (int m=2; m<i; m++)
									{
										if (arBld[m]==k)
										{bFound=false;};
										int BL=arBld[m];
										while ((pFirstInnerBld+BL)->iUpg>0)
										{
											if ((pFirstInnerBld+BL)->iUpg==k)
											{bFound=false;};
											BL=(pFirstInnerBld+BL)->iUpg;
										};
									};
									if (bFound)
									{
										arBld[i]=k;
										Search=false;
										break;
									};
								};
								}
								else
								{
									if (Inside)
									{Search=false;};
								};
							};
						};
						Slot++;
						};
					}
					else
					{arBld[i]=j;};
				};
			};
			};
			};
			if ((arBld[i]>0)&&(arPlayer[Player].arBld[arBld[i]]))
			{
				bool bFound=true;
				while ((bFound)&&(arPlayer[Player].arBld[arBld[i]]))
				{
					bFound=false;
					for (int k=1; k<=iMaxInnerBld; k++)
					{
						if ((pFirstInnerBld+k)->iUpg==arBld[i])
						{
							bool bAdd=false;
							if (((pFirstInnerBld+k)->iGroup>0)&&(GroupIsFull(Player,(pFirstInnerBld+k)->iGroup)))
							{
								if (arPlayer[Player].arBld[k])
								{bAdd=true;};
							}
							else
							{bAdd=true;};
							if (((((pFirstInnerBld+k)->iHidden)&&(!arPlayer[Player].CanBuild(k)))||(IsRestricted(k,Player)))&&(!arPlayer[Player].arBld[k]))
							{bAdd=false;};
							if (bAdd)
							{
								arBld[i]=k;
								bFound=true;
								break;
							};
						};
					};
				};
			};
		
		draw_sprite(bmpBuild,(pFirstInnerBld+arBld[i])->Pic,arSlot[i][0],arSlot[i][1]);
		if (arBld[i]>0)
		{
			if (arPlayer[Player].arBld[arBld[i]])
			{
				draw_sprite(bmpBuild,bmpGUI_Built,arSlot[i][0]+26,arSlot[i][1]);
			}
			else
			{
				if (arPlayer[Player].CanBuild(arBld[i]))
				{
					if (arPlayer[Player].iPlanned>0)
					{
						if (IsParent(arBld[i],arPlayer[Player].iPlanned,Player))
						{
							draw_sprite(bmpBuild,bmpGUI_Planned,arSlot[i][0],arSlot[i][1]);
						};
					};
					int Shift=26;
					if (arPlayer[Player].GetInBldGoldPrice(arBld[i])>arPlayer[Player].Gold)
					{
						draw_sprite(bmpBuild,bmpGUI_NoGold,arSlot[i][0]+Shift,arSlot[i][1]);
						Shift=0;
					};
					if ((arPlayer[Player].GetInBldGemPrice(arBld[i])>arPlayer[Player].Gem)&&(arPlayer[Player].GetInBldGemPrice(arBld[i])>0))
					{
						draw_sprite(bmpBuild,bmpGUI_NoGem,arSlot[i][0]+Shift,arSlot[i][1]);
						Shift=0;
					};					
				}
				else
				{
					draw_trans_sprite(bmpBuild,bmpLocked,arSlot[i][0],arSlot[i][1]);
					draw_trans_sprite(bmpBuild,bmpStuff[40],arSlot[i][0],arSlot[i][1]);
					draw_trans_sprite(bmpBuild,bmpStuff[41],arSlot[i][0],arSlot[i][1]);
				};
			};
		};
	};
	bool bRepeat=true;
	int Check;
	while (bRepeat)
	{
		if (bTutHint)
		{
			int Check;
			if (Tutorial.Mode)
			{Check=mskTutorial.Check();}
			else
			{Check=30;};
			if ((Check==30)||(Check==40))
			{
				if (Tutorial.Groups==1)
				{
				//	bTutHint=true;
				//	objTutorial->iLayer=5;
					txtTutorial.Activate((pFirstTutText+84)->msg);
					Tutorial.Groups=2;
				}
				else
				{
					bTutHint=false;
					txtTutorial.Hide();
					objTutorial->iLayer=-1;
				};
			};
		};
		if (buttExit.Check())
		{
			bCycle=false;
			bRepeat=false;
			Result=0;
		};
		if (buttHero.Check())
		{
			bRepeat=false;
			bCycle=false;
			Result=2;
		};
		if (buttGarrison.Check())
		{
			bRepeat=false;
			bCycle=false;
			Result=3;
		};
		if (buttTreasury.Check())
		{
			bRepeat=false;
			bCycle=false;
			Result=4;
		};
		if (buttLibrary.Check())
		{
			Result=5;
			bRepeat=false;
			bCycle=false;
		};
		if (buttStat.Check())
		{
			bRepeat=false;
			bCycle=false;
			Result=6;
		};

		for (int i=2; i<BLD; i++)
		{
			if (arBld[i]>0)
			{
				Check=mskBld[i].Check();
				if (Check>1)
				{
					if ((Check==30)||(Check==40))
					{
						bHint=false;						
						int Show=arBld[i];
						while (Show>0)
						{
							bHint=false;
							objGUI=ShowActiveObjects(objGUI,10,false);
							int Res=WindowInnBldInfo(Show,Player);
							if (Res==-1)
							{								
								arPlayer[Player].Build(Show);
								blResource.Show();
								if (bmpOldCastle)
								{destroy_bitmap(bmpOldCastle);};
								bmpOldCastle=bmpCastleView;								
								objOldCastle->Pic=bmpOldCastle;
								bmpCastleView=MakeCastlePic(Player);
								objCastle->Pic=bmpCastleView;
								AnimOpa=200;
								bRepeat=false;								
								if ((Tutorial.Map)&&(Tutorial.Build==1))
								{
									bTutHint=true;
									ScrollSound();
									objTutorial->iLayer=5;
									txtTutorial.Activate((pFirstTutText+53)->msg);
									Tutorial.Build=2;
									buttExit.Activate(1);
									buttExit.bBlink=true;
								};
								/*if ((Tutorial.Map)&&(Tutorial.Task<5))
								{Tutorial.Task++;};*/
							};
							if (Res==-2)
							{
								arPlayer[Player].iPlanned=Show;								
								bRepeat=false;
							};
							Show=Res;
						};
					}
					else
					{
						MakeInnBldHint(arBld[i],Player);					
						bHint=true;
					};
				};
			};
		};
		Check=mskBld[0].Check();
		if (Check>1)
		{
			if (arPlayer[Player].iPlanned>0)
			{
				MakeInnBldHint(arPlayer[Player].iPlanned,Player);					
				bHint=true;
			}
			else
			{
		//Подсказка о планировщике
			};
		};
		if (blResource.Check(objGUI))
		{bRepeat=false;};
		if (!Tutorial.Mode)
		{objTutorial->iLayer=-1;};
		if (Timer)
		{
			Timer=false;
			if (AnimOpa>0)
			{
				objCastle->bTrans=true;
				objCastle->iOpacity=255-AnimOpa;
			//	set_trans_blender(255-AnimOpa,255-AnimOpa,255-AnimOpa,255-AnimOpa);
				AnimOpa-=5;
			}
			else
			{
				objCastle->bTrans=false;
			};
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
	if (bmpOldCastle)
	{destroy_bitmap(bmpOldCastle);};
	destroy_bitmap(bmpBldInfo);
	destroy_bitmap(bmpBuild);
	destroy_bitmap(bmpCastleView);
	destroy_bitmap(bmpLocked);
	iAntisleep=iAntisleepDelay;
	return Result;
};

//***************************************************
//*** Окно найма героя ******************************
//***************************************************
int HeroRecruitWindow (int Player, int Class)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_HeroRecruitWindow->w/2;
	int y=384-bmpGUI_HeroRecruitWindow->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_HeroRecruitWindow->w,bmpGUI_HeroRecruitWindow->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_HeroRecruitWindow,bmpWindow,0,0,0,0,bmpGUI_HeroRecruitWindow->w,bmpGUI_HeroRecruitWindow->h);		
	UI_Button1 buttRecruit;
	WObj=buttRecruit.Init(x+160,y+78,1,2,bmpButton[158],bmpButton[159],bmpButton[160],bmpButton[161],WObj);
	buttRecruit.Activate(1);
	buttRecruit.iHint=137;
	UI_Button1 buttCancel;
	WObj=buttCancel.Init(x+279,y+78,1,2,bmpButton[84],bmpButton[85],bmpButton[86],bmpButton[86],WObj);
	buttCancel.Activate(1);
	buttCancel.ipHotkey=KEY_ESC;
	buttCancel.iHint=106;
	UI_TextBox txtHero;
	WObj=txtHero.Init(x+32,y+147,287,191,2,COLOR_1,Font2,WObj);
	txtHero.Activate(ClassText[Class].msg);

	draw_sprite(bmpWindow,(pFirstUnitType+40+Class)->Pic,53-(pFirstUnitType+40+Class)->Pic->w/2,67-(pFirstUnitType+40+Class)->Pic->h/2);
	textprintf_centre_ex(bmpWindow,Font1,211,25,COLOR_1,-1,"%s",ClassName[Class].str);
	if (arPlayer[Player].Gold>=arPlayer[Player].GetHeroGoldPrice(Class+1))
	{
		textprintf_centre_ex(bmpWindow,Font1,148,55,COLOR_2,-1,"%d",arPlayer[Player].GetHeroGoldPrice(Class+1));
	}
	else
	{
		textprintf_centre_ex(bmpWindow,Font1,148,55,makecol(220,20,20),-1,"%d",arPlayer[Player].GetHeroGoldPrice(Class+1));
	};
	if (arPlayer[Player].Gem>=arPlayer[Player].GetHeroGemPrice(Class+1))
	{
		textprintf_centre_ex(bmpWindow,Font1,231,55,COLOR_2,-1,"%d",arPlayer[Player].GetHeroGemPrice(Class+1));
	}
	else
	{
		textprintf_centre_ex(bmpWindow,Font1,231,55,makecol(220,20,20),-1,"%d",arPlayer[Player].GetHeroGemPrice(Class+1));
	};		
				
	if ((arPlayer[Player].Gold>=arPlayer[Player].GetHeroGoldPrice(Class+1))&&
	(arPlayer[Player].Gem>=arPlayer[Player].GetHeroGemPrice(Class+1)))
		
	{buttRecruit.Activate(1);}
	else
	{buttRecruit.Activate(0);};
	int Result=-1;
				
	
	while (Result<0)
	{
		if (buttCancel.Check())
		{Result=0;};
		if (buttRecruit.Check())
		{
			CoinSound();
			arPlayer[Player].Gold-=arPlayer[Player].GetHeroGoldPrice(Class+1);
			arPlayer[Player].Gem-=arPlayer[Player].GetHeroGemPrice(Class+1);
			Result=arPlayer[Player].RecruitHero(Class+1)+1;			
		};
	//	TextBox.Check();
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	iAntisleep=iAntisleepDelay;
	return Result;
};

//***************************************************
//*** Окно воскрешения героя ************************
//***************************************************
int ResurrectWindow (int Player, int Choosen)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpGUI_HeroResurrectWindow->w/2;
	int y=384-bmpGUI_HeroResurrectWindow->h/2;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_HeroResurrectWindow->w,bmpGUI_HeroResurrectWindow->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_HeroResurrectWindow,bmpWindow,0,0,0,0,bmpGUI_HeroResurrectWindow->w,bmpGUI_HeroResurrectWindow->h);	

	UI_Button1 buttResurrect;
	WObj=buttResurrect.Init(x+152,y+130,1,2,bmpButton[162],bmpButton[163],bmpButton[164],bmpButton[165],WObj);
	buttResurrect.Activate(1);
	buttResurrect.iHint=138;

	UI_Button1 buttExchange;
	WObj=buttExchange.Init(x+212,y+130,1,2,bmpButton[149],bmpButton[150],bmpButton[151],bmpButton[152],WObj);
	buttExchange.Activate(1);
	buttExchange.iHint=140;
	
	UI_Button1 buttBury;
	WObj=buttBury.Init(x+272,y+130,1,2,bmpButton[166],bmpButton[167],bmpButton[168],bmpButton[169],WObj);
	buttBury.Activate(1);
	buttBury.iHint=139;

	UI_Button1 buttCancel;
	WObj=buttCancel.Init(x+332,y+130,1,2,bmpButton[84],bmpButton[85],bmpButton[86],bmpButton[86],WObj);
	buttCancel.Activate(1);
	buttCancel.ipHotkey=KEY_ESC;
	buttCancel.iHint=106;



	draw_sprite(bmpWindow,arPlayer[Player].arHero[Choosen]->Pic,16,17);
	textprintf_centre_ex(bmpWindow,Font1,277,23,COLOR_1,-1,"%s",arPlayer[Player].arHero[Choosen]->pstName->str);
	textprintf_centre_ex(bmpWindow,Font1,277,50,COLOR_1,-1,"%s",ClassName[arPlayer[Player].arHero[Choosen]->GetClassName()].str);
	textprintf_centre_ex(bmpWindow,Font1,210,82,COLOR_2,-1,"%d",arPlayer[Player].arHero[Choosen]->iLevel);
	if (arPlayer[Player].Gold>=arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[Choosen]))
	{
		textprintf_centre_ex(bmpWindow,Font1,324,80,COLOR_2,-1,"%d",arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[Choosen]));
	}
	else
	{
		textprintf_centre_ex(bmpWindow,Font1,324,80,makecol(220,20,20),-1,"%d",arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[Choosen]));
	};
	if (arPlayer[Player].Gem>=arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[Choosen]))
	{
		textprintf_centre_ex(bmpWindow,Font1,324,106,COLOR_2,-1,"%d",arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[Choosen]));
	}
	else
	{
		textprintf_centre_ex(bmpWindow,Font1,324,106,makecol(220,20,20),-1,"%d",arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[Choosen]));
	};
				
	if ((arPlayer[Player].Gold>=arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[Choosen]))&&
		(arPlayer[Player].Gem>=arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[Choosen])))
	{buttResurrect.Activate(1);}
	else
	{buttResurrect.Activate(0);};
	
	int Result=999;
	
	while (Result==999)
	{
		if (buttCancel.Check())
		{Result=0;};
		if (buttBury.Check())
		{
			if ((pFirstDialog+28)->Show(true,true,false,false,NULL,NULL)==1)
			{
				delete arPlayer[Player].arHero[Choosen];
				Result=-1;
			};
		};
		if (buttExchange.Check())
		{
			ScreenHeroGarrison(arPlayer[Player].arHero[Choosen],(pFirstProvince+arPlayer[Player].ipProvince)->pGarrison,false);
			if ((arPlayer[Player].Gold>=arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[Choosen]))&&
			(arPlayer[Player].Gem>=arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[Choosen])))
			{buttResurrect.Activate(1);}
			else
			{buttResurrect.Activate(0);};
		};
		if (buttResurrect.Check())
		{
			PlaySpellSound(58);
		//	play_sample((pFirstSpell+49)->wavCast,iSoundVolume,128,1000,0);
			arPlayer[Player].Gold-=arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[Choosen]);
			arPlayer[Player].Gem-=arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[Choosen]);
			arPlayer[Player].arHero[Choosen]->iDead=-1;
			arPlayer[Player].arHero[Choosen]->ipProvince=arPlayer[Player].ipProvince;
			arPlayer[Player].arHero[Choosen]->pProvince=(pFirstProvince+arPlayer[Player].ipProvince);
			arPlayer[Player].arHero[Choosen]->pSquad[0]->iLife=arPlayer[Player].arHero[Choosen]->pSquad[0]->GetLife()/2+1;
			Result=Choosen+1;			
		};
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,6,1);
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	iAntisleep=iAntisleepDelay;
	return Result;
};


//***************************************************
//*** Экран найма героя *****************************
//***************************************************

int ScreenHeroRecruit (int Player)
{
	iAntisleep=iAntisleepDelay;
	const int dx=122;
	GUI_ResourceBlock blResource;		
	BITMAP *bmpRecruit=create_bitmap(512,768);
	BITMAP *bmpCastleView;
	bmpCastleView=MakeCastlePic(Player);
//	blit(bmpGUI_HeroRecruit,bmpRecruit,0,0,0,0,512,768);
	G_Object *objGUI=new G_Object(NULL,512,0,1,bmpCastleView);	
	objGUI=new G_Object(objGUI,0,0,1,bmpRecruit);		
	objGUI=blResource.Init(&arPlayer[Player],0,734,objGUI);	
	
	UI_Button1 buttCastle;
	objGUI=buttCastle.Init(61,686,1,3,bmpButton[267],bmpButton[268],bmpButton[269],bmpButton[270],objGUI);
	buttCastle.Activate(1);
	buttCastle.iHint=141;
	buttCastle.ipHotkey=KEY_C;
	UI_Button1 buttHero;
	objGUI=buttHero.Init(121,686,-2,3,bmpButton[250],bmpButton[251],bmpButton[252],bmpButton[253],objGUI);
	UI_Button1 buttGarrison;
	objGUI=buttGarrison.Init(181,686,1,3,bmpButton[263],bmpButton[264],bmpButton[265],bmpButton[266],objGUI);
	buttGarrison.iHint=94;
	buttGarrison.ipHotkey=KEY_D;
	UI_Button1 buttTreasury;
	objGUI=buttTreasury.Init(241,686,1,3,bmpButton[259],bmpButton[260],bmpButton[261],bmpButton[262],objGUI);	
	buttTreasury.Activate(1);
	buttTreasury.iHint=102;
	buttTreasury.ipHotkey=KEY_A;
	UI_Button1 buttLibrary;
	objGUI=buttLibrary.Init(301,686,1,3,bmpButton[271],bmpButton[272],bmpButton[273],bmpButton[274],objGUI);	
	buttLibrary.iHint=318;
	buttLibrary.ipHotkey=KEY_S;
	UI_Button1 buttStat;
	objGUI=buttStat.Init(361,686,1,3,bmpButton[275],bmpButton[276],bmpButton[277],bmpButton[278],objGUI);	
	buttStat.iHint=99;	
	UI_Button1 buttExit;
	objGUI=buttExit.Init(451,686,1,3,bmpButton[256],bmpButton[257],bmpButton[258],bmpButton[258],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;

//	if (!Tutorial.buttStat)
//	{buttStat.Activate(-1);};

	objGUI=new G_Object(objGUI,532,20,-1,bmpGUI_Hint);
	G_Object *objTutorial=objGUI;
	UI_Mask mskTutorial;
	objGUI=mskTutorial.Init(532,20,1,bmpGUI_Hint,bmpEmpty,objGUI,-1);
	UI_TextBox txtTutorial;
	objGUI=txtTutorial.Init(552,50,297,200,7,COLOR_1,Font2,objGUI);
	txtTutorial.bShadow=false;
	txtTutorial.Hide();
	bool bTutHint=false;
		
	UI_Mask mskRecruit[4];
	for (int i=0; i<4; i++)
	{
		objGUI=mskRecruit[i].Init(31+i*dx,68,1,bmpGUI_HeroSelect[i],bmpGUI_HeroSelect[i],objGUI,3);
		/*textprintf_centre_ex(bmpRecruit,Font1,72+i*dx,52,COLOR_1,-1,"%s",ClassName[i].str);
		if (arPlayer[Player].Gold>=arPlayer[Player].GetHeroGoldPrice(i+1))
		{
			textprintf_centre_ex(bmpRecruit,Font1,76+i*dx,167,COLOR_2,-1,"%d",arPlayer[Player].GetHeroGoldPrice(i+1));
		}
		else
		{
			textprintf_centre_ex(bmpRecruit,Font1,76+i*dx,167,makecol(220,20,20),-1,"%d",arPlayer[Player].GetHeroGoldPrice(i+1));
		};
		if (arPlayer[Player].Gem>=arPlayer[Player].GetHeroGemPrice(i+1))
		{
			textprintf_centre_ex(bmpRecruit,Font1,76+i*dx,193,COLOR_2,-1,"%d",arPlayer[Player].GetHeroGemPrice(i+1));
		}
		else
		{
			textprintf_centre_ex(bmpRecruit,Font1,76+i*dx,193,makecol(220,20,20),-1,"%d",arPlayer[Player].GetHeroGemPrice(i+1));
		};*/
	};
	UI_Mask mskResurrect[MAX_HERO];
	int arCorpses[MAX_HERO];	
	BITMAP *bmpDeadMask=create_bitmap(bmpGUI_HeroDead->w,bmpGUI_HeroDead->h);
	rectfill(bmpDeadMask,0,0,bmpGUI_HeroDead->w,bmpGUI_HeroDead->h,COLOR_1);
	BITMAP *bmpTime=create_bitmap(174,bmpGUI_StrHint->h);
	blit(bmpGUI_StrHint,bmpTime,0,0,0,0,151,bmpGUI_StrHint->h);
	blit(bmpGUI_StrHint,bmpTime,bmpGUI_StrHint->w-23,0,151,0,23,bmpGUI_StrHint->h);
	int MaxCorpse=0;
	for (int i=0; i<MAX_HERO; i++)
	{objGUI=mskResurrect[i].Init(14+(i%2)*243,254+((i-i%2)/2)*105,1,bmpDeadMask,bmpGUI_DeadSelect,objGUI,3);};

//	textprintf_centre_ex(bmpRecruit,Font1,260,19,COLOR_1,-1,"%s",(pFirstPhrase+47)->str);
//	textprintf_centre_ex(bmpRecruit,Font1,260,233,COLOR_1,-1,"%s",(pFirstPhrase+48)->str);
	

	set_mouse_sprite(bmpMousePointer[0]);
	bool bKeyFree=false;
	bool bRedraw=true;
	int Result=999;

	while (Result==999)
	{
		if (bRedraw)
		{
			blResource.Show();			
			blit(bmpGUI_HeroRecruit,bmpRecruit,0,0,0,0,512,768);
	//		blit(bmpGUI_HeroRecruit,bmpRecruit,0,252,0,252,512,425);
			textprintf_centre_ex(bmpRecruit,Font1,260,18,COLOR_1,-1,"%s",(pFirstPhrase+47)->str);
			textprintf_centre_ex(bmpRecruit,Font1,260,232,COLOR_1,-1,"%s",(pFirstPhrase+48)->str);
			for (int i=0; i<4; i++)
			{
			//	objGUI=mskRecruit[i].Init(31+i*dx,68,1,bmpGUI_HeroSelect[i],bmpGUI_HeroSelect[i],objGUI,3);
				textprintf_centre_ex(bmpRecruit,Font1,72+i*dx,52,COLOR_1,-1,"%s",ClassName[i].str);
				if (arPlayer[Player].Gold>=arPlayer[Player].GetHeroGoldPrice(i+1))
				{
					textprintf_centre_ex(bmpRecruit,Font1,76+i*dx,167,COLOR_2,-1,"%d",arPlayer[Player].GetHeroGoldPrice(i+1));
				}
				else
				{
					textprintf_centre_ex(bmpRecruit,Font1,76+i*dx,167,makecol(220,20,20),-1,"%d",arPlayer[Player].GetHeroGoldPrice(i+1));
				};
				if (arPlayer[Player].Gem>=arPlayer[Player].GetHeroGemPrice(i+1))
				{
					textprintf_centre_ex(bmpRecruit,Font1,76+i*dx,193,COLOR_2,-1,"%d",arPlayer[Player].GetHeroGemPrice(i+1));
				}
				else
				{
					textprintf_centre_ex(bmpRecruit,Font1,76+i*dx,193,makecol(220,20,20),-1,"%d",arPlayer[Player].GetHeroGemPrice(i+1));
				};
			};
			MaxCorpse=0;
			for (int i=0; i<MAX_HERO; i++)
			{
				if ((arPlayer[Player].arHero[i])&&(arPlayer[Player].arHero[i]->iDead>=0))
				{
					int x=14+(MaxCorpse%2)*243;
					int y=254+((MaxCorpse-MaxCorpse%2)/2)*105;
					arCorpses[MaxCorpse]=i;					
					draw_sprite(bmpRecruit,bmpGUI_HeroDead,x,y);
					draw_sprite(bmpRecruit,arPlayer[Player].arHero[i]->smPic,x+11,y+9);
					textprintf_centre_ex(bmpRecruit,Font1,x+154,y+15,COLOR_1,-1,"%s",arPlayer[Player].arHero[i]->pstName->str);
					textprintf_centre_ex(bmpRecruit,Font1,x+154,y+43,COLOR_1,-1,"%s",ClassName[arPlayer[Player].arHero[i]->GetClassName()].str);
					if (arPlayer[Player].arHero[i]->iDead>0)
					{
						draw_sprite(bmpRecruit,bmpTime,x+63,y+65);
						textprintf_centre_ex(bmpRecruit,Font1,x+154,y+75,COLOR_1,-1,"%s:%d",(pFirstPhrase+313)->str,arPlayer[Player].arHero[i]->iDead);
					}
					else
					{
						if (arPlayer[Player].Gold<arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[i]))
						{textprintf_centre_ex(bmpRecruit,Font1,x+114,y+76,makecol(220,20,20),-1,"%d",arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[i]));}
						else
						{textprintf_centre_ex(bmpRecruit,Font1,x+114,y+76,COLOR_2,-1,"%d",arPlayer[Player].GetResurrectGoldPrice(arPlayer[Player].arHero[i]));};
						if (arPlayer[Player].Gem<arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[i]))
						{textprintf_centre_ex(bmpRecruit,Font1,x+201,y+76,makecol(220,20,20),-1,"%d",arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[i]));}
						else
						{textprintf_centre_ex(bmpRecruit,Font1,x+201,y+76,COLOR_2,-1,"%d",arPlayer[Player].GetResurrectGemPrice(arPlayer[Player].arHero[i]));};
					};
					textprintf_centre_ex(bmpRecruit,Font1,x+42,y+76,COLOR_2,-1,"%d",arPlayer[Player].arHero[i]->iLevel);
					MaxCorpse++;
				};
			};
			if (Tutorial.Hero==2)
			{
				bTutHint=true;
				ScrollSound();
				objTutorial->iLayer=4;
				txtTutorial.Activate((pFirstTutText+75)->msg);
				Tutorial.Hero=3;
				buttHero.bBlink=true;
			};
			bRedraw=false;
		};
		if (!key[KEY_ESC])
		{bKeyFree=true;};
		for (int i=0; i<MaxCorpse; i++)
		{
			//Инициализация окна воскрешения
			if (mskResurrect[i].Check()==30)
			{
				if (arPlayer[Player].arHero[arCorpses[i]]->iDead==0)
				{
					int Check=ResurrectWindow(Player,arCorpses[i]);
					bRedraw=true;
					if (Check==-1)
					{
						if (i==MaxCorpse-1)
						{
							mskResurrect[i].G_Obj->iLayer=-1;
						};
						Result=2;
					};
					if (Check>0)
					{
						Result=-Check;						
					};
				}
				else
				{
					char Message[500];
					sprintf(Message,(pFirstDialog+29)->sText,arPlayer[Player].arHero[arCorpses[i]]->iDead);
					(pFirstDialog+29)->Show(true,false,false,false,NULL,Message);
				};
			};
		};
		for (int i=0; i<4; i++)
		{
			//Инициализация окна найма
			if (mskRecruit[i].Check()==30)
			{
				int Count=0;
				for (int i=0; i<MAX_HERO; i++)
				{
					if (arPlayer[Player].arHero[i])
					{Count++;};
				};
				if (Count<MAX_HERO) 
				{			
					Count=0;
					for (int j=0; j<MAX_HERO; j++)
					{
						if ((arPlayer[Player].arHero[j])&&(arPlayer[Player].arHero[j]->ipClass1==i+1))
						{Count++;};
					};
					if (Count<4)
					{
						int Check=HeroRecruitWindow(Player,i);
						if (Check>0)
						{
							Result=-Check;						
						};				
					}
					else
					{
						(pFirstDialog+1089)->Show(true,true,false,false,NULL,NULL);
					};
				}
				else
				{
					(pFirstDialog+1077)->Show(true,true,false,false,NULL,NULL);
				};				
			};
		};	
		if ((buttExit.Check())&&(bKeyFree))
		{
			Result=0;
		};
		if (buttCastle.Check())
		{
			Result=1;
		};
		if (buttGarrison.Check())
		{
			Result=3;
		};
		if (buttTreasury.Check())
		{
			Result=4;
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
				objTutorial->iLayer=-1;
				txtTutorial.Hide();
				bTutHint=false;
			};
		};
		if (buttLibrary.Check())
		{Result=5;};
		if (buttStat.Check())
		{Result=6;};
		if (blResource.Check(objGUI))
		{bRedraw=true;};
		if (!Tutorial.Mode)
		{objTutorial->iLayer=-1;};
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,10,true);
		};
		bHint=false;
	};
	set_mouse_sprite(bmpMousePointer[0]);
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
//	destroy_bitmap(bmpResurrectWindow);
//	destroy_bitmap(bmpRecruitWindow);
	destroy_bitmap(bmpRecruit);
	destroy_bitmap(bmpCastleView);
	destroy_bitmap(bmpDeadMask);
	destroy_bitmap(bmpTime);
//	destroy_bitmap(bmpBack);
	iAntisleep=iAntisleepDelay;
	return Result;
};


//***************************************************
//*** Экран сокровищницы ****************************
//***************************************************
int ScreenTreasury(int Player)
{
	iAntisleep=iAntisleepDelay;
	int Result=999;
	dcHero *Hero=new dcHero;
	Hero->ipPlayer=Player;	
	GUI_ResourceBlock blResource;		
	BITMAP *bmpScr=create_bitmap(512,768);	
	blit(bmpGUI_Treasury,bmpScr,0,0,0,0,512,768);		
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpScr);		
	objGUI=blResource.Init(&arPlayer[Player],0,734,objGUI);	
	UI_Button1 buttExit;
	objGUI=buttExit.Init(451,686,1,3,bmpButton[256],bmpButton[257],bmpButton[258],bmpButton[258],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;

	UI_Button1 buttCastle;
	objGUI=buttCastle.Init(61,686,1,3,bmpButton[267],bmpButton[268],bmpButton[269],bmpButton[270],objGUI);
	buttCastle.Activate(1);
	buttCastle.iHint=141;
	buttCastle.ipHotkey=KEY_C;
	UI_Button1 buttHero;
	objGUI=buttHero.Init(121,686,1,3,bmpButton[250],bmpButton[251],bmpButton[252],bmpButton[253],objGUI);
	buttHero.iHint=136;
	buttHero.ipHotkey=KEY_F;
	UI_Button1 buttGarrison;
	objGUI=buttGarrison.Init(181,686,1,3,bmpButton[263],bmpButton[264],bmpButton[265],bmpButton[266],objGUI);
	buttGarrison.iHint=94;
	buttGarrison.ipHotkey=KEY_D;
	UI_Button1 buttTreasury;
	objGUI=buttTreasury.Init(241,686,-2,3,bmpButton[259],bmpButton[260],bmpButton[261],bmpButton[262],objGUI);	
	buttTreasury.iHint=102;
	buttTreasury.ipHotkey=KEY_A;
	UI_Button1 buttLibrary;
	objGUI=buttLibrary.Init(301,686,1,3,bmpButton[271],bmpButton[272],bmpButton[273],bmpButton[274],objGUI);	
	buttLibrary.iHint=318;
	buttLibrary.ipHotkey=KEY_S;
	UI_Button1 buttStat;
	objGUI=buttStat.Init(361,686,1,3,bmpButton[275],bmpButton[276],bmpButton[277],bmpButton[278],objGUI);	
	buttStat.iHint=99;	

	
	GUI_ShopBlock blShop;	
	GUI_TreasuryBlock blTreasury;	
	dcPlayer * PL=&arPlayer[Player];
	objGUI=new G_Object(objGUI,512,0,1,bmpGUI_Shop);
	BITMAP * bmpMouse = create_bitmap(150,130);	
	int SelectedSlot=0;
	int SelectedItem=0, SelectedItemDur;
	int ItemOwner;
		
	objGUI=blShop.Init(Hero,&PL->Shop,519,13,objGUI);	
	objGUI=blTreasury.Init(PL,7,467,objGUI);
	if (Tutorial.Map)
	{
		buttHero.Activate(-1);
		if (!Tutorial.buttStat)
		{buttStat.Activate(-1);};
	};
	

	
	bool bBuy=false;


//	blInfo.Show();

	UI_Button1 buttTrash;
	objGUI=buttTrash.Init(512+94,689,1,3,bmpGUI_ButtTrashNorm,bmpGUI_ButtTrashAct,bmpButton[231],bmpButton[232],objGUI);
	buttTrash.Activate(0);
	buttTrash.ipHotkey=KEY_DEL;
	
	//UI_Button1 buttExit1;
	//objGUI=buttExit1.Init(934,687,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	//buttExit1.Activate(1);
	//buttExit1.ipHotkey=KEY_ESC;
	//buttExit1.iHint=135;


	set_mouse_sprite(bmpMousePointer[0]);
	int Under1=0, Under=-1;

	while (Result==999)
	{	
		int Click=0;		
//Манипулирование предметами
//**************************
		Click=0;
		Under1=Under;
		Under=-1;
		Click=blShop.Check(Hero);
	
		if ((SelectedItem)&&(blShop.buttRepair.bPress))
		{			
			if (ItemOwner==2)
			{
				PL->arTreasury[SelectedSlot][0]=SelectedItem;
				PL->arTreasury[SelectedSlot][1]=SelectedItemDur;
				blTreasury.Show();
				SelectedItem=0;
				set_mouse_sprite(bmpMousePointer[0]);
			};			
		};

		if ((SelectedItem==0)&&(Under>=0)&&(PL->Shop.Items[Under][0]>0)&&(Under!=Under1))
		{
			blShop.Pick(2,PL->Shop.Items[Under][0],PL->Shop.Items[Under][1]);
			blShop.Show();			
		};
		if (Under>=0)
		{Under1=Under;};
		if (Click>=0)
		{
		if (SelectedItem)
		{
			if (ItemOwner<3)
		//Продажа предмета
			{
				CoinSound();
				clItem * It=(pFirstItem+SelectedItem);
				PL->Gold+=It->iGoldCost/4;
				PL->Gem+=It->iGemCost/4;
				if (It->iDurability>0)
				{
					PL->Gold-=(It->iGoldCost*(It->iDurability-SelectedItemDur)/It->iDurability/8);
					PL->Gem-=(It->iGemCost*(It->iDurability-SelectedItemDur)/It->iDurability/8);
				};
				blShop.Pick(0,0,0);
				SelectedItem=0;
				SelectedSlot=0;
				blShop.Show();	
				blResource.Show();
				set_mouse_sprite(bmpMousePointer[0]);
				buttTrash.Activate(0);
			}
			else
	//Взятие предмета в магазине
			{
				if ((PL->Shop.Items[Click][0]>0)&&(PL->Shop.Items[Click][0]!=SelectedItem)&&
					(PL->Gold>=PL->GetItemGoldPrice(PL->Shop.Items[Click][0],&PL->Shop))&&
					((PL->Gem>=PL->GetItemGemPrice(PL->Shop.Items[Click][0],&PL->Shop))||(PL->GetItemGemPrice(PL->Shop.Items[Click][0],&PL->Shop)==0)))
				{
					SelectedItem=PL->Shop.Items[Click][0];
					SelectedItemDur=(pFirstItem+SelectedItem)->iDurability;
					SelectedSlot=Click;
					ItemOwner=3;
					blShop.Pick(2,SelectedItem,SelectedItemDur);
					blShop.Show();
					blResource.Show();					
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash.Activate(1);
				}
				else
				{
					SelectedItem=0;
					SelectedSlot=0;					
					set_mouse_sprite(bmpMousePointer[0]);
					buttTrash.Activate(0);
				};
			}
		}
		else
		{
			if (PL->Shop.Items[Click][0]>0&&
				(PL->Gold>=PL->GetItemGoldPrice(PL->Shop.Items[Click][0],&PL->Shop))&&
				((PL->Gem>=PL->GetItemGemPrice(PL->Shop.Items[Click][0],&PL->Shop))||(PL->GetItemGemPrice(PL->Shop.Items[Click][0],&PL->Shop)==0)))
			{
				SelectedItem=PL->Shop.Items[Click][0];
				SelectedItemDur=(pFirstItem+SelectedItem)->iDurability;
				SelectedSlot=Click;
				ItemOwner=3;
				blShop.Pick(2,SelectedItem,SelectedItemDur);
				blShop.Show();				
				blResource.Show();
				rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
				draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
				draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
				set_mouse_sprite(bmpMouse);
				buttTrash.Activate(1);
			};
		};
		};
	

		if (blShop.buttRepair.bPress)
		{
			Click=blTreasury.Check(Hero,14,&PL->Shop);
		}
		else
		{			
			Click=blTreasury.Check(Hero,12,NULL);
		};
		if ((SelectedItem==0)&&(Under>=0)&&(PL->arTreasury[Under][0]>0)&&(Under!=Under1))
		{
			if (blShop.buttRepair.bPress)
			{
				int Gold=PL->GetItemGoldRepPrice(PL->arTreasury[Under][0],PL->arTreasury[Under][1],&PL->Shop);
				int Gem=PL->GetItemGemRepPrice(PL->arTreasury[Under][0],PL->arTreasury[Under][1],&PL->Shop);
				blShop.Pick(3,PL->arTreasury[Under][0],PL->arTreasury[Under][1]);
				if (((Gold>0)||(Gem>0))&&((PL->Gem>=Gem)||(Gem==0))&&(PL->Gold>=Gold))
				{
					set_mouse_sprite(bmpMousePointer[17]);
				}
				else
				{
					set_mouse_sprite(bmpMousePointer[18]);
				};
			}
			else
			{
				blShop.Pick(1,PL->arTreasury[Under][0],PL->arTreasury[Under][1]);										
			};
			blShop.Show();
		};
		if (Under>=0)
		{Under1=Under;};
		if (Click>=0)
		{
		if (SelectedItem)
		{				
	//Попытка поместить предмет в...
			if (PL->arTreasury[Click][0]>0)
			{
		//Занятый слот			
				if (ItemOwner==2)
				{
				//Предмет из сокровищницы
					PL->arTreasury[SelectedSlot][0]=PL->arTreasury[Click][0];
					PL->arTreasury[SelectedSlot][1]=PL->arTreasury[Click][1];
					PL->arTreasury[Click][0]=SelectedItem;
					PL->arTreasury[Click][1]=SelectedItemDur;
					SelectedItem=0;
					SelectedSlot=0;
					blShop.Pick(0,0,0);					
					blShop.Show();	
					blTreasury.Show();
					set_mouse_sprite(bmpMousePointer[0]);
					buttTrash.Activate(0);
				};
			}
			else
			{	
	//Свободный слот
				if (ItemOwner==3)
				{
			//Покупка предмета
					CoinSound();
					PL->Gold-=PL->GetItemGoldPrice(SelectedItem, &PL->Shop);
					PL->Gem-=PL->GetItemGemPrice(SelectedItem, &PL->Shop);
					for (int i=0; i<MAX_ITEM_RES; i++)
					{
						if (((pFirstItem+SelectedItem)->arRes[i]>0)&&(PL->GetResource((pFirstItem+SelectedItem)->arRes[i])==0))
						{
							CurGame.ResPrices[(pFirstItem+SelectedItem)->arRes[i]]+=CurGame.ResBuy;
						};
					};
					if (PL->Shop.Items[SelectedSlot][1]>0)
					{
						PL->Shop.Items[SelectedSlot][1]--;
						if (PL->Shop.Items[SelectedSlot][1]==0)
						{
							PL->Shop.Items[SelectedSlot][0]=0;
						};
					};
				};
				PL->arTreasury[Click][0]=SelectedItem;
				PL->arTreasury[Click][1]=SelectedItemDur;	
				SelectedItem=0;
				SelectedSlot=0;
				blShop.Pick(0,0,0);				
				blShop.Show();
				blResource.Show();
				blTreasury.Show();
				set_mouse_sprite(bmpMousePointer[0]);
				buttTrash.Activate(0);					
			};
		}
		else
		{
	
			if (PL->arTreasury[Click][0]>0)
			{
			if (blShop.buttRepair.bPress)
		//Ремонт предмета
			{
				int Gold=PL->GetItemGoldRepPrice(PL->arTreasury[Click][0],PL->arTreasury[Click][1],&PL->Shop);
				int Gem=PL->GetItemGemRepPrice(PL->arTreasury[Click][0],PL->arTreasury[Click][1],&PL->Shop);
				blShop.Pick(3,PL->arTreasury[Click][0],PL->arTreasury[Click][1]);
				if (((Gold>0)||(Gem>0))&&((PL->Gem>=Gem)||(Gem==0))&&(PL->Gold>=Gold))
				{
					PL->Gem-=Gem;
					PL->Gold-=Gold;
					PlayUnitSound(wavRepair);
				//	play_sample(wavClick,iSoundVolume,128,1000,0);
					PL->arTreasury[Click][1]=(pFirstItem+PL->arTreasury[Click][0])->iDurability;					
					blShop.Pick(3,PL->arTreasury[Click][0],PL->arTreasury[Click][1]);
					blTreasury.Show();
					blShop.Show();					
					blResource.Show();
					set_mouse_sprite(bmpMousePointer[18]);					
				};
			}
			else
		//Взятие предмета
			{	
				SelectedItem=PL->arTreasury[Click][0];
				SelectedItemDur=PL->arTreasury[Click][1];
				SelectedSlot=Click;
				ItemOwner=2;
				PL->arTreasury[Click][0]=0;
				PL->arTreasury[Click][1]=0;
				blShop.Pick(1,SelectedItem,SelectedItemDur);
				blShop.Show();				
				blTreasury.Show();
				rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
				draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
				draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
				set_mouse_sprite(bmpMouse);
				buttTrash.Activate(1);					
			};
			};
		};	
		};		
		
		if ((SelectedItem==0)&&(Under<0)&&(Under!=Under1))
		{
			blShop.Pick(0,0,0);
			blShop.Show();
			set_mouse_sprite(bmpMousePointer[0]);			
		};
		
		if (buttTrash.Check())
		{
			SelectedItem=0;
			SelectedSlot=0;
			blShop.Pick(0,0,0);
			blShop.Show();
			buttTrash.Activate(0);			
			set_mouse_sprite(bmpMousePointer[0]);
		};
		if (buttExit.Check())
		{
			Result=0;
		};
		if (buttCastle.Check())
		{
			Result=1;
		};
		if (buttHero.Check())
		{
			Result=2;
		};
		if (buttGarrison.Check())
		{
			Result=3;
		};
		if (buttLibrary.Check())
		{Result=5;};
		if (buttStat.Check())
		{Result=6;};
		if (blResource.Check(objGUI))
		{blShop.Show();};
		
			
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,10,true);
		};
		bHint=false;
	};
	if (SelectedItem)
	{		
		if (ItemOwner==2)
		{
			PL->arTreasury[SelectedSlot][0]=SelectedItem;
			PL->arTreasury[SelectedSlot][1]=SelectedItemDur;
		};
	};
	
	set_mouse_sprite(bmpMousePointer[0]);
	destroy_bitmap(bmpMouse);	
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	bHint=false;
	HintType=0;
	//Сдвигаем товары если выкуплен какой-либо лот
	int i=0; bool bRunShop=true;
	while (bRunShop)
	{
		while ((i<SHOP_SIZE-1)&&(PL->Shop.Items[i][0]>0))
		{i++;};
		if (i>=SHOP_SIZE-1)
		{bRunShop=false;}
		else
		{
			bRunShop=false;
			for (int j=i+1; j<SHOP_SIZE; j++)
			{
				if (PL->Shop.Items[j][0]>0)
				{bRunShop=true;};
			};
			if (bRunShop)
			{
				for (int j=i; j<SHOP_SIZE-1; j++)
				{
					PL->Shop.Items[j][0]=PL->Shop.Items[j+1][0];
					PL->Shop.Items[j][1]=PL->Shop.Items[j+1][1];					
				};
				PL->Shop.Items[SHOP_SIZE-1][0]=0;
				PL->Shop.Items[SHOP_SIZE-1][1]=0;
			};
		};
	};
	destroy_bitmap(bmpScr);
	iAntisleep=iAntisleepDelay;
	return Result;
};

int HomeGarrison(int Player)
{
	iAntisleep=iAntisleepDelay;
	int Result=999;
	bool bMerc;
	bool CanHire=true;
	if ((pFirstProvince+arPlayer[Player].ipProvince)->GetSiegeIndex()>=0)
	{CanHire=false;};
	GUI_UnitBlock blUnit;	
	GUI_RecruitBlock blRecruit;
	GUI_ResourceBlock blResource;
	BITMAP * bmpCastle=MakeCastlePic(Player);
	draw_sprite(bmpCastle,bmpGUI_HomeBarracks,0,0);
	G_Object *objGUI=new G_Object(NULL,512,0,1,bmpCastle);
	dcHero *Garrison=(pFirstProvince+arPlayer[Player].ipProvince)->pGarrison;
	
	BITMAP * bmpMouse = create_bitmap(150,130);
	int SelectedSlot=0;
	dcUnit * SelectedUnit=NULL;
	objGUI=new G_Object(objGUI,0,0,1,bmpGUI_HomeGarrison);

	objGUI=blRecruit.Init(&arPlayer[Player],512,392,objGUI,false,Garrison->ipProvince);
	objGUI=blResource.Init(&arPlayer[Player],0,734,objGUI);
	objGUI=blUnit.Init(Garrison,59,339,objGUI);

	UI_Button1 * buttTrash=new UI_Button1();
	objGUI=buttTrash->Init(340,430,1,3,bmpGUI_ButtTrashNorm,bmpGUI_ButtTrashAct,bmpButton[231],bmpButton[232],objGUI);
	buttTrash->ipHotkey=KEY_DEL;
	buttTrash->Activate(0);

	UI_Button1 buttExit;
	objGUI=buttExit.Init(451,686,1,3,bmpButton[256],bmpButton[257],bmpButton[258],bmpButton[258],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;

	UI_Button1 buttCastle;
	objGUI=buttCastle.Init(61,686,1,3,bmpButton[267],bmpButton[268],bmpButton[269],bmpButton[270],objGUI);
	buttCastle.Activate(1);
	buttCastle.iHint=141;
	buttCastle.ipHotkey=KEY_C;
	UI_Button1 buttHero;
	objGUI=buttHero.Init(121,686,1,3,bmpButton[250],bmpButton[251],bmpButton[252],bmpButton[253],objGUI);
	buttHero.iHint=136;
	buttHero.ipHotkey=KEY_F;
	UI_Button1 buttGarrison;
	objGUI=buttGarrison.Init(181,686,-2,3,bmpButton[263],bmpButton[264],bmpButton[265],bmpButton[266],objGUI);
	buttGarrison.iHint=94;
	buttGarrison.ipHotkey=KEY_D;
	UI_Button1 buttTreasury;
	objGUI=buttTreasury.Init(241,686,1,3,bmpButton[259],bmpButton[260],bmpButton[261],bmpButton[262],objGUI);	
	buttTreasury.iHint=102;
	buttTreasury.ipHotkey=KEY_A;
	UI_Button1 buttLibrary;
	objGUI=buttLibrary.Init(301,686,1,3,bmpButton[271],bmpButton[272],bmpButton[273],bmpButton[274],objGUI);	
	buttLibrary.iHint=318;
	buttLibrary.ipHotkey=KEY_S;
	UI_Button1 buttStat;
	objGUI=buttStat.Init(361,686,1,3,bmpButton[275],bmpButton[276],bmpButton[277],bmpButton[278],objGUI);	
	buttStat.iHint=99;	

	if (Tutorial.Map)
	{
		buttHero.Activate(-1);
		if (!Tutorial.buttStat)
		{buttStat.Activate(-1);};
	};
	
/*	UI_Button1 buttExit;
	objGUI=buttExit.Init(895,540,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;	*/	

	set_mouse_sprite(bmpMousePointer[0]);

	while (Result==999)
	{	
		int Click=0;
		Click=blRecruit.Check();
		if (Click>0)
		{			
			if (SelectedUnit)
			{
				if (SelectedSlot>=0)
				{
					Garrison->PlaceUnit(SelectedUnit,SelectedSlot);
					blUnit.Show();
				}
				else
				{
					delete SelectedUnit;
				};				
			};
			if (Click>1000)
			{
				bMerc=true;
				Click-=1000;
				SelectedUnit=new dcUnit(Click,Random(GetUnitExp(Click,3*arPlayer[Garrison->ipPlayer].GetBldParam1(6))));
				SelectedUnit->arMedal[0]=6;
			}
			else
			{
				bMerc=false;
				SelectedUnit=new dcUnit(Click,arPlayer[Garrison->ipPlayer].GetBldParam1(20));
			};			
			SelectedSlot=-100;
			rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
			draw_sprite(bmpMouse,SelectedUnit->pType()->Pic,0,0);
			draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
			set_mouse_sprite(bmpMouse);
			buttTrash->Activate(1);
		};

				
				
	//Покупка юнитов
	//**************
		Click=0;
		Click=blUnit.Check(SelectedUnit);
		if (Click>0)
		{
			if (SelectedUnit)
			{				
				if (SelectedSlot<0)
				{
					if (CanHire)
					{
					if ((!Garrison->pSquad[Click])&&(SlotLvl(Click)>=SelectedUnit->pType()->iLevel))
					{
						CoinSound();
						Garrison->PlaceUnit(SelectedUnit,Click);					
						clUnitType Rookie=*(SelectedUnit->pType());
						bool bSkip=true;					
						if (bMerc)
						{
							arPlayer[Garrison->ipPlayer].Gold-=arPlayer[Garrison->ipPlayer].GetUnitGoldPrice(SelectedUnit->GetUnitType(),Garrison->ipProvince)*2;
							arPlayer[Garrison->ipPlayer].Gem-=arPlayer[Garrison->ipPlayer].GetUnitGemPrice(SelectedUnit->GetUnitType(),Garrison->ipProvince)*2;
							arPlayer[Garrison->ipPlayer].arRecruit[11]=0;
							blRecruit.Recalc();
						}
						else
						{
							arPlayer[Garrison->ipPlayer].Gold-=arPlayer[Garrison->ipPlayer].GetUnitGoldPrice(SelectedUnit->GetUnitType(),Garrison->ipProvince);
							arPlayer[Garrison->ipPlayer].Gem-=arPlayer[Garrison->ipPlayer].GetUnitGemPrice(SelectedUnit->GetUnitType(),Garrison->ipProvince);
							if ((key[KEY_LCONTROL])||(key[KEY_RCONTROL]))
							{bSkip=false;};
						};
						for (int i=0; i<MAX_UNIT_RES; i++)
						{							
							if (Rookie.arRes[i]>0)
							{
								if (arPlayer[Garrison->ipPlayer].GetResource(Rookie.arRes[i])==0)
								{
									CurGame.ResPrices[Rookie.arRes[i]]+=CurGame.ResBuy;
								};
							};
						};
						if ((arPlayer[Garrison->ipPlayer].Gold<arPlayer[Garrison->ipPlayer].GetUnitGoldPrice(SelectedUnit->GetUnitType(),Garrison->ipProvince))||((arPlayer[Garrison->ipPlayer].GetUnitGemPrice(SelectedUnit->GetUnitType(),Garrison->ipProvince)>0)&&
						(arPlayer[Garrison->ipPlayer].Gem<arPlayer[Garrison->ipPlayer].GetUnitGemPrice(SelectedUnit->GetUnitType(),Garrison->ipProvince))))
						{bSkip=true;};
						if (bSkip)
						{
							SelectedUnit=NULL;
							SelectedSlot=0;
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash->Activate(0);
						}
						blUnit.Show();						
						blRecruit.Show();
						blResource.Show();
					};
					}
					else
					{
						(pFirstDialog+387)->Show(true,true,false,false,NULL,NULL);
						blResource.Show();
						delete SelectedUnit;
						SelectedUnit=NULL;
						SelectedSlot=0;
						blUnit.Show();						
						blRecruit.Show();
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash->Activate(0);
					};
				}
				else

			//Обмен юнитами
			//*************
				{
			//Попытка поместить юнита в...
				if (SlotLvl(Click)>=SelectedUnit->pType()->iLevel)					
				{
					if (Garrison->pSquad[Click])
					{
	//Занятый слот
						if (GetFreeUnitSlot(Garrison,Garrison->pSquad[Click]->pType()->iLevel))
						{
							if (SlotLvl(SelectedSlot)<Garrison->pSquad[Click]->pType()->iLevel)
							//Перемещение юнита в свободный слот нужного уровня передающему герою
							{
								int Free=GetFreeUnitSlot(Garrison,Garrison->pSquad[Click]->pType()->iLevel);							
								Garrison->PlaceUnit(Garrison->pSquad[Click], Free);
							}
							else
							{							
								Garrison->PlaceUnit(Garrison->pSquad[Click], SelectedSlot);
							};
							Garrison->pSquad[Click]=NULL;
							Garrison->PlaceUnit(SelectedUnit,Click);							
							SelectedUnit=NULL;
							SelectedSlot=0;
							blUnit.Show();							
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash->Activate(0);							
						};
					}
					else
					{	
	//Свободный слот						
						Garrison->pSquad[Click]=NULL;
						Garrison->PlaceUnit(SelectedUnit,Click);					
						SelectedUnit=NULL;
						SelectedSlot=0;
						blUnit.Show();						
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash->Activate(0);						
					};
				};
				};
			}
			else
			{
	//Взятие юнита
				if (Garrison->pSquad[Click])
				{					
					SelectedUnit=Garrison->pSquad[Click];					
					SelectedSlot=Click;					
					Garrison->pSquad[Click]=NULL;					
					blUnit.Show();					
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,SelectedUnit->pType()->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash->Activate(1);					
				};
			};
		};
		if (blResource.Check(objGUI))
		{blRecruit.Show();};
		if (buttExit.Check())
		{
			Result=0;
		};
		if (buttCastle.Check())
		{
			Result=1;
		};
		if (buttHero.Check())
		{
			Result=2;
		};
		if (buttTreasury.Check())
		{
			Result=4;
		};
		if (buttLibrary.Check())
		{Result=5;};
		if (buttStat.Check())
		{Result=6;};
		if (buttTrash->Check())
		{
			if (SelectedUnit)
			{
				if (SelectedSlot>=0)
				{
					SelectedUnit->Dismiss();
				}
				else
				{
					delete SelectedUnit;
				}
				SelectedUnit=NULL;		
			};			
			SelectedSlot=0;
			buttTrash->Activate(0);			
			set_mouse_sprite(bmpMousePointer[0]);
		};
		
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,10,true);
		};
		bHint=false;
	};	
	if (SelectedUnit)
	{
		Garrison->pSquad[SelectedSlot]=SelectedUnit;
	};
	set_mouse_sprite(bmpMousePointer[0]);
	destroy_bitmap(bmpMouse);
	destroy_bitmap(bmpCastle);
	delete buttTrash;		
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	iAntisleep=iAntisleepDelay;
	return Result;
};

int HomeLibrary(int Player)
{
	iAntisleep=iAntisleepDelay;
	int Result=999;
	GUI_ResourceBlock blResource;
	GUI_LibraryBlock blLibrary;
	BITMAP * bmpCastle=MakeCastlePic(Player);	
	G_Object *objGUI=new G_Object(NULL,512,0,1,bmpCastle);	
	objGUI=new G_Object(objGUI,0,0,1,bmpGUI_HomeLibrary);

	objGUI=blResource.Init(&arPlayer[Player],0,734,objGUI);
	objGUI=blLibrary.Init(&arPlayer[Player],80,101,objGUI);	

	UI_Button1 buttExit;
	objGUI=buttExit.Init(451,686,1,3,bmpButton[256],bmpButton[257],bmpButton[258],bmpButton[258],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;

	UI_Button1 buttCastle;
	objGUI=buttCastle.Init(61,686,1,3,bmpButton[267],bmpButton[268],bmpButton[269],bmpButton[270],objGUI);
	buttCastle.Activate(1);
	buttCastle.iHint=141;
	buttCastle.ipHotkey=KEY_C;
	UI_Button1 buttHero;
	objGUI=buttHero.Init(121,686,1,3,bmpButton[250],bmpButton[251],bmpButton[252],bmpButton[253],objGUI);
	buttHero.iHint=136;
	buttHero.ipHotkey=KEY_F;
	UI_Button1 buttGarrison;
	objGUI=buttGarrison.Init(181,686,1,3,bmpButton[263],bmpButton[264],bmpButton[265],bmpButton[266],objGUI);
	buttGarrison.iHint=94;
	buttGarrison.ipHotkey=KEY_D;
	UI_Button1 buttTreasury;
	objGUI=buttTreasury.Init(241,686,1,3,bmpButton[259],bmpButton[260],bmpButton[261],bmpButton[262],objGUI);	
	buttTreasury.iHint=102;
	buttTreasury.ipHotkey=KEY_A;
	UI_Button1 buttLibrary;
	objGUI=buttLibrary.Init(301,686,-2,3,bmpButton[271],bmpButton[272],bmpButton[273],bmpButton[274],objGUI);	
	buttLibrary.iHint=318;
	buttLibrary.ipHotkey=KEY_S;
	UI_Button1 buttStat;
	objGUI=buttStat.Init(361,686,1,3,bmpButton[275],bmpButton[276],bmpButton[277],bmpButton[278],objGUI);	
	buttStat.iHint=99;	
	if (Tutorial.Map)
	{
		buttHero.Activate(-1);
		if (!Tutorial.buttStat)
		{buttStat.Activate(-1);};
	};
	
	while (Result==999)
	{
		blResource.Check(objGUI);
		blLibrary.Check();
		if (buttExit.Check())
		{
			Result=0;
		};
		if (buttCastle.Check())
		{
			Result=1;
		};
		if (buttHero.Check())
		{
			Result=2;
		};
		if (buttGarrison.Check())
		{
			Result=3;
		};
		if (buttTreasury.Check())
		{
			Result=4;
		};
		if (buttStat.Check())
		{
			Result=6;
		};		
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,10,true);
		};
		bHint=false;
	};		
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	destroy_bitmap(bmpCastle);
	iAntisleep=iAntisleepDelay;
	return Result;
};


int CastleMain(int Player)
{
	int Result=1, Old;
	while (Result>0)
	{
		Old=Result;
		switch (Result)
		{
		case 1:
			Result=ScreenCastleBuild(Player);
			break;
		case 2:
			Result=ScreenHeroRecruit(Player);
			break;
		case 3:
			Result=HomeGarrison(Player);
			break;
		case 4:
			Result=ScreenTreasury(Player);
			break;
		case 5:
			Result=HomeLibrary(Player);
			break;
		};
		if (Result==6)
		{
			Result=Old;
			CastleProv=ScreenStatistics(Player);
			if (CastleProv>0)
			{Result=0;};
		};
	};
	return abs (Result);
};
