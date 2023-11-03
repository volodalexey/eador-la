#include "Battlefield.h"

strHexx arGrid [8] [8];		//Карта проходимости
strHexx Field [8] [8];		//Карта для поиска пути
UI_Mask Grid [8] [8];		//Маски тайлов
BF_Unit Squad[2][MAX_SQUAD+MAX_SUMMONS];//Отряды
bool IsPlayed[2];
int Spells[2][2][SPELLBOOK_SIZE];		//Заклинания
int ActiveSpell=-1;
int ActivePlayer;
int FearVictory;						//Флаг сбежавших противников
bool bInPanic;							//Флаг того, что все враги запуганы
int BF_Player[2];						//Игроки
//dcHero * Herr[2];						//Герои
int LifeMod[2];							//Изменение максиума жизни нейтралов в завасимость от сложности (в %)
int SummonExp[2]={0, 0};				//Опыт, получаемый противником за вызванных юнитов
int SummonSurv[2]={0, 0};				//Сколько суммоны прожили
int SummonWork[2]={0, 0};				//Сколько суммоны заработали опыта
int GemSpent[2];						//Сколько кристаллов потрачено за бой
int SideCanCast[2];	
clSpell *pCast, *pCast1;
int iSacVictim;				//Цель жертвоприношения
int iTarget;				//Враг под курсором
int ProvinceMoraleBonus;	//Бонус боевого духа охранникам провинции
//	iTowerShoot;			//Сила выстрела из башни
BF_Unit * ActiveUnit=NULL;	//Указатель на активный юнит
BITMAP *jPanel, *objPanel, 	//Боковая панель (изображение и объект вывода данных)
*bmpUnitBar,				//Статус-бар юнита
*bmpCorpse,					//Труп юнита
*bmpFlipCorpse,
*bmpGrid,
*bmpSelect,					//Выделение юнита
*bmpTarget,					//Выделение цели
*bmpTargetH,				//Выделение цели для лечнгия
*bmpSpellbook,				//Книга заклинаний
*bmpSpellShadow,			//Затенение заклинания
*bmpSpellPanel,				//Боковая панель вызова заклинания
*bmpEmptyPanel,				//Пустая боковая панель
*bmpWarMenu,
*bmpCheckBox,
*bmpCheckBird,
*bmpWall[2],
*bmpTower,
*bmpHole[2],
*bmpWallShadow[2],
*bmpTowerShadow,
*bmpHoleShadow[2],
*bmpHoleBorder[2][2],
*bmpHoleBorderShadow[2][2],
*bmpBFSwamp[4],
*bmpFloor[6],
*bmpBFStone[6][2],
*bmpSack[2],
*bmpBarrel[3],
*bmpPlank[3][2],
*bmpCrate[3][2],
*bmpTorch[2][2],
*bmpSpot[2],
*bmpCrack,
*bmpPick,
//*bmpBFTree[7],
//*bmpBFHill[3],
*bmpLowString,
*bmpSideBar,
*bmpBFMount[2],
*bmpBFFen[2],
*bmpBFChasm[2],
*bmpBFThicket[2],
*bmpBFForest[2],
*bmpMissile[MAX_MISSILE];	//Картинки снарядов

//SAMPLE *wawHeal, *wavBerz, *wavMarch, *wavGather, *wavRound, *wavCrush, *wavSoul, *wavEat, *wavTrample, *wavRoot, *wavArmor, *wavWeapon, *wawEat;

BITMAP *jGrass[5];
BITMAP *jMask, *jHexx;
clBF_Object *pBF_Obj;
G_Object *objSelect;
G_Object *objLowString;
G_Object *objSideBar;
G_Object *objMain;			//Ссылка на основной список графики
G_Object *objTarget[8][8];		//Массив подсветок для целей
G_Object *objTargetH[8][8];		//Массив подсветок для целей лечения
G_Object *objGrid[8][8];		//Массив с сеткой
//int MinWeight, MinStamina;	//Минимальная длина пути (в единицах скорости) и необходимая стамина для найденного пути
bool bFirstMove,			//Флаг истинный, если активный юнит еще не двигался
	bArchmage,				//Флаг истинный, если герой может вызывать 2 заклинания в ход
	bFastDraw,				//Герой может сменить оружие
	bShowBattle,			//Истинна если в бою есть игрок-человек
	bShowAnim,				//Флаг видимости для графических объектов
	bAutoCombat,			//Флаг автобоя, включаемый игроком
	bCastle,				//Истинно, если это штурм замка
	bAltFlag,
	bSimulate=false;		//Флаг симуляции битвы
//clPath *TruePath;			//Указатель на найденный путь
//int Found;
int AA=0,					//Активный игрок
	iTurn=0,				//Счетчик передач хода
	IndexAI;
UI_Button1 buttRest, buttNext, buttEndTurn, buttGather, buttSpell, buttCancelSpell, buttChange,
buttMarch, buttEat, buttStart, buttGrid, buttMenu;
UI_Button2 buttHeal, buttCrush, buttDoubleShot, buttRoundAtt;
UI_TextBox txtSpName, txtSpTarget, txtSpDesc, txtTutorial;
UI_Mask mskEff[6];
BF_Effect *arEff[6];		
dcUnit *unTower;	//Эмуляция башни
BF_Unit bfTower;
G_Object *objTower;
BF_Unit *Gomunculus;	//Ссылка на только что призванного юнита
UI_Mask1 mskParam[13];

int MagTower;
dcUnit *unMagTower[3];	//Эмуляция магических башен
BF_Unit bfMagTower[3];
G_Object *objMagTower[3];
int MagTowerCoord[3]={470, 290, 650};
clSpell Teleport;			//Прототип заклинания телепортации с целеуказанием на свободный тайл
BF_Unit *pTeleportUnit;		//Указатель на телепортируемого юнита
G_Object *objSim;
G_Object *objTutorial;
UI_Mask mskTutorial;
bool bTutHint, bEndWarning;
int TutQue, TutMode;

void LoadBFPics()
{
	objSim=new G_Object(NULL,0,0,0,bmpEmpty);
	DATAFILE *datBF=load_datafile("dat\\battlefield.dat");

	mskParam[0].Init(895,98,61,21,1);

	mskParam[1].Init(962,17,57,26,1);
	mskParam[11].Init(962,17+26,57,26,1);
	mskParam[12].Init(962,17+26*2,57,26,1);
	mskParam[7].Init(962,17+26*3,57,26,1);

	mskParam[2].Init(893,122,62,31,1);
	mskParam[8].Init(893,122+31,62,31,1);
	mskParam[10].Init(893,122+31*2,62,31,1);
	mskParam[4].Init(893,122+31*3,62,31,1);

	mskParam[3].Init(893+62,122,62,31,1);
	mskParam[9].Init(893+62,122+31,62,31,1);
	mskParam[6].Init(893+62,122+31*2,62,31,1);
	mskParam[5].Init(893+62,122+31*3,62,31,1);
	
	jMask=(BITMAP *)datBF[0].dat;
	jHexx=(BITMAP *)datBF[1].dat;
	jPanel=(BITMAP *)datBF[2].dat;
	bmpWarMenu=(BITMAP *)datBF[10].dat;
	bmpCheckBox=(BITMAP *)datBF[11].dat;
	bmpCheckBird=(BITMAP *)datBF[12].dat;
	bmpUnitBar=(BITMAP *)datBF[13].dat;
	bmpTarget=(BITMAP *)datBF[14].dat;
	bmpCorpse=(BITMAP *)datBF[15].dat;
	bmpFlipCorpse=create_bitmap(bmpCorpse->w,bmpCorpse->h);
	rectfill(bmpFlipCorpse,0,0,bmpCorpse->w,bmpCorpse->h,COLOR_0);
	rotate_sprite_v_flip(bmpFlipCorpse,bmpCorpse,0,0,itofix(128));
	bmpSelect=(BITMAP *)datBF[16].dat;
	bmpSpellbook=(BITMAP *)datBF[17].dat;
	bmpTargetH=(BITMAP *)datBF[18].dat;
	bmpGrid=(BITMAP *)datBF[19].dat;
	bmpSpellShadow=(BITMAP *)datBF[20].dat;
	bmpSpellPanel=(BITMAP *)datBF[21].dat;
	bmpEmptyPanel=(BITMAP *)datBF[22].dat;
	bmpWall[0]=(BITMAP *)datBF[23].dat;
	bmpWall[1]=(BITMAP *)datBF[24].dat;
	bmpTower=(BITMAP *)datBF[25].dat;
	bmpHole[0]=(BITMAP *)datBF[26].dat;
	bmpHole[1]=(BITMAP *)datBF[27].dat;

	for (int i=0; i<3; i++)
	{
		jGrass[i]=(BITMAP *)datBF[3+i].dat;
	};	
	for (int i=0; i<4; i++)
	{
		bmpBFSwamp[i]=(BITMAP *)datBF[6+i].dat;
	};
	/*for (int i=0; i<3; i++)
	{
		bmpBFHill[i]=(BITMAP *)datBF[10+i].dat;
	};*/
	for (int i=0; i<MAX_MISSILE-1; i++)
	{
		bmpMissile[i]=(BITMAP *)datBF[28+i].dat;
	};
	bmpMissile[MAX_MISSILE-1]=(BITMAP *)datBF[111].dat;
	
	for (int i=0; i<6; i++)
	{
		bmpFloor[i]=(BITMAP *)datBF[39+i].dat;
	};	
	for (int i=0; i<2; i++)
	{
		bmpBFChasm[i]=(BITMAP *)datBF[51+i].dat;
	};
	for (int i=0; i<2; i++)
	{
		bmpBFThicket[i]=(BITMAP *)datBF[53+i].dat;
	};
	for (int i=0; i<2; i++)
	{
		bmpBFMount[i]=(BITMAP *)datBF[55+i].dat;
	};
	for (int i=0; i<2; i++)
	{
		bmpBFFen[i]=(BITMAP *)datBF[57+i].dat;
	};
//	bmpBFMount=(BITMAP *)datBF[56].dat;
//	bmpBFFen=(BITMAP *)datBF[57].dat;
	for (int i=0; i<2; i++)
	{
		bmpBFForest[i]=(BITMAP *)datBF[59+i].dat;
	};
	bmpWallShadow[0]=(BITMAP *)datBF[61].dat;
	bmpWallShadow[1]=(BITMAP *)datBF[62].dat;
	bmpTowerShadow=(BITMAP *)datBF[63].dat;
	bmpHoleShadow[0]=(BITMAP *)datBF[64].dat;
	bmpHoleShadow[1]=(BITMAP *)datBF[65].dat;
	bmpHoleBorder[0][0]=(BITMAP *)datBF[66].dat;
	bmpHoleBorder[0][1]=(BITMAP *)datBF[67].dat;
	bmpHoleBorder[1][0]=(BITMAP *)datBF[68].dat;
	bmpHoleBorder[1][1]=(BITMAP *)datBF[69].dat;
	bmpHoleBorderShadow[0][0]=(BITMAP *)datBF[70].dat;
	bmpHoleBorderShadow[0][1]=(BITMAP *)datBF[71].dat;
	bmpHoleBorderShadow[1][0]=(BITMAP *)datBF[72].dat;
	bmpHoleBorderShadow[1][1]=(BITMAP *)datBF[73].dat;

	for (int j=0; j<2; j++)
	{
		for (int i=0; i<6; i++)
		{
			bmpBFStone[i][j]=(BITMAP *)datBF[74+j*6+i].dat;
		};
	};
	for (int i=0; i<2; i++)
	{
		bmpSack[i]=(BITMAP *)datBF[86+i].dat;
	};
	for (int i=0; i<3; i++)
	{
		bmpBarrel[i]=(BITMAP *)datBF[88+i].dat;
	};
	for (int j=0; j<2; j++)
	{
		for (int i=0; i<3; i++)
		{
			bmpPlank[i][j]=(BITMAP *)datBF[91+j*3+i].dat;
		};
	};
	for (int j=0; j<2; j++)
	{
		for (int i=0; i<2; i++)
		{
			bmpCrate[i][j]=(BITMAP *)datBF[97+j*2+i].dat;
		};
	};
	bmpPick=(BITMAP *)datBF[101].dat;
	bmpCrate[2][0]=(BITMAP *)datBF[102].dat;
	bmpCrate[2][1]=(BITMAP *)datBF[103].dat;
	for (int j=0; j<2; j++)
	{
		for (int i=0; i<2; i++)
		{
			bmpTorch[j][i]=(BITMAP *)datBF[104+j*2+i].dat;
		};
	};
	bmpSpot[0]=(BITMAP *)datBF[108].dat;
	bmpSpot[1]=(BITMAP *)datBF[109].dat;
	bmpCrack=(BITMAP *)datBF[110].dat;

	pBF_Obj=pFirstBF_Obj+1;
	pBF_Obj->bmpTileset[0][0]=NULL;
	pBF_Obj++;
	pBF_Obj->bmpTileset[0][0]=(BITMAP *)datBF[8].dat;
	pBF_Obj++;
	pBF_Obj->bmpTileset[0][0]=(BITMAP *)datBF[9].dat;
	pBF_Obj++;
	pBF_Obj->bmpTileset[0][0]=(BITMAP *)datBF[10].dat;
	pBF_Obj++;
	pBF_Obj->bmpTileset[0][0]=(BITMAP *)datBF[14].dat;
	pBF_Obj++;	
	pBF_Obj->bmpTileset[0][0]=(BITMAP *)datBF[11].dat;
	pBF_Obj++;
	pBF_Obj->bmpTileset[0][0]=(BITMAP *)datBF[12].dat;
	pBF_Obj++;
	pBF_Obj->bmpTileset[0][0]=(BITMAP *)datBF[13].dat;
	pBF_Obj++;
	pBF_Obj->bmpTileset[0][0]=(BITMAP *)datBF[8].dat;
	pBF_Obj++;
	pBF_Obj->bmpTileset[0][0]=(BITMAP *)datBF[8].dat;
	pBF_Obj++;
	pBF_Obj->bmpTileset[0][0]=(BITMAP *)datBF[8].dat;
	for (int A=0; A<2; A++)
	{
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			Squad[A][i].bmpBar=create_bitmap(bmpUnitBar->w,bmpUnitBar->h);
		};
	};
	bmpLowString=create_bitmap(bmpGUI_StrHint->w,bmpGUI_StrHint->h);	
	blit(bmpGUI_StrHint,bmpLowString,0,0,0,0,bmpGUI_StrHint->w,bmpGUI_StrHint->h);
	bmpSideBar=create_bitmap(jPanel->w,700);
	textprintf_centre_ex(bmpLowString,Font1,bmpGUI_StrHint->w/2,8,COLOR_1,-1,(pFirstPhrase+491)->str);
};



void BF_RefreshScreen()
{
	if (Timer)
	{
		Timer=false;
		objMain=ShowActiveObjects(objMain,10,true);
		for (int j=0; j<2; j++)
		{
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if ((Squad[j][i].iLife)&&(!Squad[j][i].G_Obj->bAnim))
			{
				if (TransX(Squad[j][i].x, Squad[j][i].y, Squad[j][i].G_Obj->Pic->w)!=Squad[j][i].G_Obj->x)
				{Squad[j][i].G_Obj->x=TransX(Squad[j][i].x, Squad[j][i].y,Squad[j][i].G_Obj->Pic->w);};
				if (TransY(Squad[j][i].x, Squad[j][i].y,Squad[j][i].G_Obj->Pic->h)!=Squad[j][i].G_Obj->y)
				{Squad[j][i].G_Obj->y=TransY(Squad[j][i].x, Squad[j][i].y,Squad[j][i].G_Obj->Pic->h);};
			};
		/*	if ((Squad[1-AA][i].iLife)&&(!Squad[1-AA][i].G_Obj->bAnim))
			{
				if (TransX(Squad[1-AA][i].x, Squad[1-AA][i].y,Squad[1-AA][i].G_Obj->Pic->w)!=Squad[1-AA][i].G_Obj->x)
				{Squad[1-AA][i].G_Obj->x=TransX(Squad[1-AA][i].x, Squad[1-AA][i].y,Squad[1-AA][i].G_Obj->Pic->w);};
				if (TransY(Squad[1-AA][i].x, Squad[1-AA][i].y,Squad[1-AA][i].G_Obj->Pic->h)!=Squad[1-AA][i].G_Obj->y)
				{Squad[1-AA][i].G_Obj->y=TransY(Squad[1-AA][i].x, Squad[1-AA][i].y,Squad[1-AA][i].G_Obj->Pic->h);};
			};*/
		};
		};
	}
	else
	{
		if (!bAnimation)
		{Sleeper(24);}
	};
};

void HideButtons()
{
	buttRest.Activate(-1);
	buttNext.Activate(-1);
	buttChange.Activate(-1);
	buttEndTurn.Activate(0);
	buttMenu.Activate(0);
//	buttUnitBar.Activate(-1,buttUnitBar.bPress);
	buttGather.Activate(-1);
	buttSpell.Activate(-1);
	buttCancelSpell.Activate(-1);
	buttMarch.Activate(-1);
	buttEat.Activate(-1);
	buttStart.Activate(-1);
	buttHeal.Activate(-1,0);
	buttCrush.Activate(-1,0);
	buttDoubleShot.Activate(-1,0);
	buttRoundAtt.Activate(-1,0);
	for (int i=0; i<6; i++)
	{
		mskEff[i].Activate(-1);
		arEff[i]=NULL;
	};
};

void HideTargets()
{
	for (int x=0; x<8; x++)
	{
		for (int y=0; y<8; y++)
		{
			objTarget[x][y]->iLayer=-1;
			objTargetH[x][y]->iLayer=-1;
		};
	};
};

void ShowGrid(int X1, int X2)
{
	if (arHumans[arPlayer[ActivePlayer].iColor].bWarGrid)
	{
	//	for (int x=X1; x<=X2; x++)
	//	{
	//		for (int y=0; y<8; y++)
	//		{
	//			objGrid[x][y]->iLayer=1;
	//		};
	//	};
		for (int x=0; x<8; x++)
		{
			for (int y=0; y<8; y++)
			{
				objGrid[x][y]->iLayer=1;
				if ((x<X1)||(x>X2))
				{
					objGrid[x][y]->iOpacity=70;
					objGrid[x][y]->bTrans=true;
				}
				else
				{objGrid[x][y]->bTrans=false;};
			};
		};
	}
	else
	{
		for (int x=0; x<8; x++)
		{
			for (int y=0; y<8; y++)
			{
				objGrid[x][y]->iLayer=-1;
			};
		};
	};
};

long CalcDistance(int x, int y)
{
	return (52*(y%2)+52+x*104-mouse_x)*(52*(y%2)+52+x*104-mouse_x)+(y*90+45-mouse_y)*(y*90+45-mouse_y);
};

int Loss(int Side)
{
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		if (Squad[Side][i].iLife)
		{
			return i;
		};
	};
	return -1;
};

int FindCorpse(int Side, int x, int y)
{
	for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
	{
		if ((Squad[Side][k].pUnit)&&(Squad[Side][k].iLife==0)&&(Squad[Side][k].bInBattle)&&
			(Squad[Side][k].x==x)&&(Squad[Side][k].y==y))
		{return k;};
	};
	return -1;
};

int GetTileType(int x, int y) //Возвращает тип тайла, в котором находится точка с координатами x,y
{
	int j=(y-y%90)/90;
	if (j%2){x-=52;};
	int i=(x-x%104)/104;
	if (y%90<30)
	{
		if (abs(x%104-52)>(y%90)*52/30)
		{
			if ((j%2)&&(x%104>52))
			{i++;};
			if ((j%2==0)&&(x%104<52))
			{i--;};
			j--;
		};
	};
	if (i<0){i=0;};
	if (j<0){j=0;};
	if (i>7){i=7;};
	if (j>7){j=7;};
	return arGrid[i][j].Land;
};

void MakeBackground(int iNum)
{
	for (int i=0; i<12; i++)
	{
		for (int j=0; j<8; j++)
		{
			blit(jGrass[Random(3,i,j,CurGame.iSeed)],screen1,0,0,i*100,j*100,100,100);
		}
	};
	if (bCastle)
	{
		for (int i=0; i<3; i++)
		{
			for (int j=0; j<8; j++)
			{
				blit(bmpFloor[Random(6)],screen1,0,0,i*100+590,j*100,100,100);
			};
		};
	};
	//Текстура болота
	for (int y=0; y<768; y+=5)	
	{
		for (int x=0; x<884; x+=4)		
		{
			if (((GetTileType(x,y)==4)||(GetTileType(x,y)==8))&&(Random(100)>96))
			{
				draw_sprite(screen1,bmpBFSwamp[Random(4)],x-25,y-25);
			};
			if ((GetTileType(x,y)==2)&&(Random(100)>98))
			{
				draw_sprite(screen1,bmpBFForest[x%2],x-25,y-25);
			};
		};
	};


	set_trans_blender(128,128,128,128);
	bool bDraw;
	int Poss, PR, PR1;
	int arImpass[8][8];
	for (int i=0; i<8; i++)
	{
		for (int j=0; j<8; j++)
		{
			if ((arGrid[i][j].Land>4)&&(arGrid[i][j].Land<9))
			{arImpass[i][j]=1;}
			else
			{arImpass[i][j]=0;}
		};
	};
	for (int Layer=0; Layer<MAX_TERR_LAYER; Layer++)
	{
	for (int y=0; y<768; y+=5)	
	{
		if ((y%90==15)&&((y-15)/90<8))
		{
			for (int i=0; i<8; i++)
			{
				if (arImpass[i][(y-15)/90]>0)
				{
					arImpass[i][(y-15)/90]=0;
					int XX;
					if ((y/90)%2)
					{XX=52+i*104;}
					else
					{XX=i*104;};
					BITMAP *Pictur=bmpBFChasm[Random(2)];
					switch (arGrid[i][(y-15)/90].Land)
					{
					case 5:
						Pictur=bmpBFChasm[Random(2)];
						break;
					case 6:
						Pictur=bmpBFThicket[Random(2)];
						break;
					case 7:
						Pictur=bmpBFMount[Random(2)];
						break;
					case 8:
						Pictur=bmpBFFen[Random(2)];
						break;
					};
					draw_sprite(screen1,Pictur,XX-(Pictur->w-104)/2,y-(Pictur->h-100)/2);
				};
			};
		};
		for (int i=0; i<MagTower; i++)
		{
			if (y==MagTowerCoord[i])
			{draw_sprite(screen1, bmpTower, 843,MagTowerCoord[i]-114);};
		};
		for (int x=0; x<884; x+=5)		
		{	
			bool NeedRoll=true, bCentre=false;
			if ((y%90==15)&&((y-15)/90<10))
			{
				if ((y/90)%2)
				{
					if ((x+52)%104<5)
					{bCentre=true;}
				}
				else
				{
					if (x%104<5)
					{bCentre=true;};
				};				
			};
			Poss=0, PR=GetTileType(x,y);
			if (PR>4)
			{PR=0;};
			int iPic=0, Addum=0;
			if (y<50)
			{Addum=(60-y)/10;};
			if (y>700)
			{Addum=(y-700)/10;};
			if (x>840)
			{Addum+=(x-840)/10;};
			if ((bCastle)&&(x>620))
			{
				switch (Layer)
				{
				case 0:
					if (Random(100)==99)
					{
						iPic=Random(5);
						draw_trans_sprite(screen1,bmpBFStone[iPic][1],x-bmpBFStone[iPic][1]->w/2,y-bmpBFStone[iPic][1]->h);
						draw_sprite(screen1,bmpBFStone[iPic][0],x-bmpBFStone[iPic][1]->w/2,y-bmpBFStone[iPic][1]->h);
					}
					else
					{
						if (Random(1000)==99)
						{
							iPic=Random(2);							
							draw_sprite(screen1,bmpSpot[iPic],x-bmpSpot[iPic]->w/2,y-bmpSpot[iPic]->h);
						}
						else
						{
							if (Random(5000)<=Addum)
							{							
								draw_sprite(screen1,bmpPick,x-bmpPick->w/2,y-bmpPick->h);
							}
							else
							{
								if (Random(3000)<=Addum)
								{							
									draw_sprite(screen1,bmpCrack,x-bmpCrack->w/2,y-bmpCrack->h);
								};
							};
						};
					};
					break;
				case 1:
					if (Random(2000)<=Addum)
					{
						iPic=Random(3);
						draw_trans_sprite(screen1,bmpPlank[iPic][1],x-bmpPlank[iPic][1]->w/2,y-bmpPlank[iPic][1]->h);
						draw_sprite(screen1,bmpPlank[iPic][0],x-bmpPlank[iPic][1]->w/2,y-bmpPlank[iPic][1]->h);
					};
					break;
				case 2:
					if (Random(3000)<=Addum)
					{
						iPic=Random(2);
						draw_trans_sprite(screen1,bmpBarrel[2],x-bmpBarrel[2]->w/2,y-bmpBarrel[2]->h);
						draw_sprite(screen1,bmpBarrel[iPic],x-bmpBarrel[iPic]->w/2,y-bmpBarrel[iPic]->h);
					}
					else
					{
						if (Random(3000)<=Addum)
						{
							iPic=Random(3);
							draw_trans_sprite(screen1,bmpCrate[iPic][1],x-bmpCrate[iPic][1]->w/2,y-bmpCrate[iPic][1]->h);
							draw_sprite(screen1,bmpCrate[iPic][0],x-bmpCrate[iPic][0]->w/2,y-bmpCrate[iPic][0]->h);
						}
						else
						{
							if (Random(4000)<=Addum)
							{
								draw_trans_sprite(screen1,bmpSack[1],x-bmpSack[1]->w/2,y-bmpSack[1]->h);
								draw_sprite(screen1,bmpSack[0],x-bmpSack[0]->w/2,y-bmpSack[0]->h);
							}
							else
							{
								if (Random(8000)<=Addum)
								{
									iPic=Random(2);
									draw_trans_sprite(screen1,bmpTorch[iPic][1],x-bmpTorch[iPic][1]->w/2,y-bmpTorch[iPic][1]->h);
									draw_sprite(screen1,bmpTorch[iPic][0],x-bmpTorch[iPic][0]->w/2,y-bmpTorch[iPic][0]->h);
								};
							};
						};
					};
					break;


				};
			}
			else
			{
			if ((PR>0)&&(arTerrLayer[PR][Layer]>0)&&((Random(800,x+y,Layer,iNum,y)<arTerrLayer[PR][Layer])||((bCentre)&&(PR==3))))
			{				
				ClearRoll();
				int Counter=0;
				for (int i=1; i<=iMaxMapObject; i++)
				{
					if (((pFirstMapObject+i)->iLayer==Layer)&&(!(pFirstMapObject+i)->bLandmark)&&((pFirstMapObject+i)->arLand[PR]>0))
					{
						bDraw=true;
					//	if (((pFirstProvince+PR)->ipTerrain==2)&&((pFirstMapObject+i)->iType>0)&&((CurGame.iSeed+PR)%4==0))
					//	{
					//		if ((CurGame.iSeed/7+PR)%2+1!=(pFirstMapObject+i)->iType)
					//		{bDraw=false;};
					//	};
						PR1=GetTileType(x-(pFirstMapObject+i)->dx/2,y-(pFirstMapObject+i)->dy/2);
						if ((PR1>4)||((pFirstMapObject+i)->arLand[PR1]<=0))
						{bDraw=false;};
						PR1=GetTileType(x+(pFirstMapObject+i)->dx/2,y-(pFirstMapObject+i)->dy/2);
						if ((PR1>4)||((pFirstMapObject+i)->arLand[PR1]<=0))
						{bDraw=false;};
						PR1=GetTileType(x,y-(pFirstMapObject+i)->dy/2);
						if ((PR1>4)||((pFirstMapObject+i)->arLand[PR1]<=0))
						{bDraw=false;};
						if ((bDraw)||(bCentre))
						{
							RollerN[Counter]=i;
							RollerP[Counter]=(pFirstMapObject+i)->arLand[PR];
							Counter++;
						};
					};					
				};	
				if (Counter>0)
				{
					int iObj=Roll(false,iNum,y,x,Layer);
					int iPic=0;
					while ((iPic<MAX_MAPOBJ_PIC)&&((pFirstMapObject+iObj)->arPic[iPic])){iPic++;};
					iPic=Random(iPic,y,iNum,Layer,x);
					for (int i=1; i<=iMaxMapObject; i++)
					{
						if ((pFirstMapObject+i)->iType-1000==iObj)
						{
							if ((pFirstMapObject+i)->iLayer==0)
							{
								draw_sprite(screen1,(pFirstMapObject+i)->arPic[0],x-(pFirstMapObject+iObj)->arPic[iPic]->w/2,y-(pFirstMapObject+iObj)->arPic[iPic]->h);
							}
							else
							{
								draw_trans_sprite(screen1,(pFirstMapObject+i)->arPic[0],x-(pFirstMapObject+iObj)->arPic[iPic]->w/2,y-(pFirstMapObject+iObj)->arPic[iPic]->h);
							};
						};
					};				
					int XP=x-(pFirstMapObject+iObj)->arPic[iPic]->w/2, YP=y-(pFirstMapObject+iObj)->arPic[iPic]->h;					
					draw_sprite(screen1,(pFirstMapObject+iObj)->arPic[iPic],XP,YP);
				};
			};
			};
		};
		
	};
	};
	
	blit(jPanel,screen1,0,0,884,0,jPanel->w,jPanel->h);
	

	if (bCastle)
	{
		draw_sprite(screen1, bmpWall[1],52+5*104+4,7-90);
		draw_trans_sprite(screen1, bmpWallShadow[1],614,6-90);
		draw_trans_sprite(screen1, bmpWallShadow[0],614,25+8*90);
		for (int i=0; i<8; i++)
		{
			switch (arGrid[5][i].Land)
			{
			case 9:				
				if (i%2)
				{
					draw_trans_sprite(screen1, bmpWallShadow[1],614,6+i*90);
					draw_sprite(screen1, bmpWall[1],52+5*104+4,i*90+7);
				}
				else					
				{
					draw_trans_sprite(screen1, bmpWallShadow[0],614,25+i*90);
					draw_sprite(screen1, bmpWall[0],5*104+42,i*90-1);
				};
				break;
			case 10:
				draw_trans_sprite(screen1, bmpTowerShadow,614,25+i*90);
				draw_sprite(screen1, bmpTower,5*104+18,i*90+3);
				break;
			case 11:
				if (i%2)
				{
					draw_trans_sprite(screen1, bmpHoleShadow[1],52+5*104+4,i*90+7+3);
					draw_sprite(screen1, bmpHole[1],52+5*104+4,i*90+7+2);
				}
				else
				{
					draw_trans_sprite(screen1, bmpHoleShadow[0],5*104+42+14,i*90-1+8);
					draw_sprite(screen1, bmpHole[0],5*104+42+14,i*90-1+8);
				};
				if ((i==0)||(arGrid[5][i-1].Land!=11))
				{			
					if (i%2)
					{
						draw_trans_sprite(screen1, bmpHoleBorderShadow[1][0],614,6+i*90);
						draw_sprite(screen1, bmpHoleBorder[1][0],52+5*104+4,i*90+7-34);
					}
					else
					{			
						draw_trans_sprite(screen1, bmpHoleBorderShadow[0][0],576,25+i*90);
						draw_sprite(screen1, bmpHoleBorder[0][0],5*104+42,i*90-1-34);
					};
				};
				if ((i==7)||(arGrid[5][i+1].Land!=11))
				{			
					if (i%2)
					{
						draw_trans_sprite(screen1, bmpHoleBorderShadow[1][1],586,6+i*90+63);
						draw_sprite(screen1, bmpHoleBorder[1][1],52+5*104+4,i*90+7+81);
					}
					else
					{
						draw_trans_sprite(screen1, bmpHoleBorderShadow[0][1],586,25+i*90+45);
						draw_sprite(screen1, bmpHoleBorder[0][1],5*104+42+13,i*90-1+87);
					};
				};
				break;
			};
		};
		draw_sprite(screen1, bmpWall[0],5*104+42,8*90-1);
		
	};	
	blit(screen1,Background,0,0,0,0,1024,768);
	
};

int TrophyGold=0, TrophyGem=0;
int Loot [9][2];

void EndBattleScreen(int Result, int Win)
{
	bHourGlass=false;
	int x, y, LootY=99;
	BITMAP *bmpWindow;
	switch (Result)
	{
	case 1:
		x=512-bmpGUI_Victory->w/2;
		y=384-bmpGUI_Victory->h/2;
		bmpWindow=create_bitmap(bmpGUI_Victory->w,bmpGUI_Victory->h);
		blit(bmpGUI_Victory,bmpWindow,0,0,0,0,bmpGUI_Victory->w,bmpGUI_Victory->h);
		textprintf_centre_ex(bmpWindow,Font1,166,38,COLOR_1,-1,"%s",(pFirstPhrase+18)->str);
		textprintf_centre_ex(bmpWindow,Font1,611,38,COLOR_1,-1,"%s",(pFirstPhrase+19)->str);
		textprintf_centre_ex(bmpWindow,Font1,390,13,COLOR_1,-1,"%s",(pFirstPhrase+22)->str);
		textprintf_centre_ex(bmpWindow,Font1,390,71,COLOR_1,-1,"%s",(pFirstPhrase+20)->str);
		textprintf_centre_ex(bmpWindow,Font1,400,104,COLOR_2,-1,"%d",TrophyGold);
		textprintf_centre_ex(bmpWindow,Font1,400,150,COLOR_2,-1,"%d",TrophyGem);
		LootY=185;
		break;
	case 0:
		x=512-bmpGUI_Lose->w/2;
		y=384-bmpGUI_Lose->h/2;
		bmpWindow=create_bitmap(bmpGUI_Lose->w,bmpGUI_Lose->h);
		blit(bmpGUI_Lose,bmpWindow,0,0,0,0,bmpGUI_Lose->w,bmpGUI_Lose->h);
		textprintf_centre_ex(bmpWindow,Font1,166,38,COLOR_1,-1,"%s",(pFirstPhrase+19)->str);
		textprintf_centre_ex(bmpWindow,Font1,611,38,COLOR_1,-1,"%s",(pFirstPhrase+18)->str);
		textprintf_centre_ex(bmpWindow,Font1,380,13,COLOR_1,-1,"%s",(pFirstPhrase+23)->str);
		textprintf_centre_ex(bmpWindow,Font1,390,71,COLOR_1,-1,"%s",(pFirstPhrase+21)->str);
		break;
	case -1:
		x=512-bmpGUI_Lose->w/2;
		y=384-bmpGUI_Lose->h/2;
		bmpWindow=create_bitmap(bmpGUI_Lose->w,bmpGUI_Lose->h);
		blit(bmpGUI_Lose,bmpWindow,0,0,0,0,bmpGUI_Lose->w,bmpGUI_Lose->h);
		textprintf_centre_ex(bmpWindow,Font1,166,38,COLOR_1,-1,"%s",(pFirstPhrase+18)->str);
		textprintf_centre_ex(bmpWindow,Font1,611,38,COLOR_1,-1,"%s",(pFirstPhrase+19)->str);
		textprintf_centre_ex(bmpWindow,Font1,390,13,COLOR_1,-1,"%s",(pFirstPhrase+49)->str);
		break;
	};
	BITMAP *bmpExp=create_bitmap(bmpGUI_ExpLow->w,bmpGUI_ExpLow->h);
	BITMAP *bmpHealth=create_bitmap(bmpGUI_HealthLow->w,bmpGUI_HealthLow->h);
	set_mouse_sprite(bmpMousePointer[0]);			
	objMain=new G_Object(objMain,x,y,6,bmpWindow);
	G_Object *objWindow=objMain;		
	UI_Mask mskLoot[9];		
	for (int i=0; i<4; i++)
	{
		if (Loot[i][0]>0)
		{
			draw_sprite(bmpWindow,(pFirstItem+Loot[i][0])->Pic,333+60*(i%2),LootY+60*((i-i%2)/2));
			objMain=mskLoot[i].Init(x+327+60*(i%2),y+LootY-6+60*((i-i%2)/2),1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objMain,7);				
		};
	};
	UI_Mask mskUnit[2][MAX_SQUAD];
	int W1=0, W2=0;
	
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if (Squad[Win][i].pUnit)
		{	
			objMain=mskUnit[Win][i].Init(x+18+(W1%4)*72,y+56+((W1-W1%4)/4)*88,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,objMain,7);
			DrawUnit(bmpWindow, Squad[Win][i].pUnit, 26+(W1%4)*72,64+((W1-W1%4)/4)*88, -1000, bmpExp, bmpHealth, Squad[Win][i].pUnit->pHero);			
			W1++;
		};
		if (Squad[1-Win][i].pUnit)
		{
			objMain=mskUnit[1-Win][i].Init(x+463+(W2%4)*72,y+56+((W2-W2%4)/4)*88,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,objMain,7);
			DrawUnit(bmpWindow, Squad[1-Win][i].pUnit, 471+(W2%4)*72, 64+((W2-W2%4)/4)*88, -1000, bmpExp, bmpHealth, Squad[1-Win][i].pUnit->pHero);			
			W2++;
		};
	};
	set_trans_blender(128,128,128,128);

	UI_Button1 buttExit;
	objMain=buttExit.Init(x+359,y+361,1,7,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objMain);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;

	while (!buttExit.Check())
	{
		bHint=false;
		for (int i=0; i<9; i++)
		{
			if (Loot[i][0]>0)
			{
				int Check=mskLoot[i].Check();
				if (Check>1)
				{
					MakeItemHint(Loot[i][0],Loot[i][1],NULL,0,0,0);
					bHint=true;
				};
				if ((Check==30)||(Check==40))
				{
					bHint=false;
					WindowItemInfo(Loot[i][0],Loot[i][1]);
				};
			};
		};
		for (int i=0; i<2; i++)
		{
			for (int j=0; j<MAX_SQUAD; j++)
			{
				if (Squad[i][j].pUnit)
				{
					int Check=mskUnit[i][j].Check();
					if ((Check==30)||(Check==40))
						{
						Squad[i][j].ShowInfoScreen();
					};
				};
			};
		};
		if (Timer)
		{
			Timer=false;
			objMain=ShowActiveObjects(objMain,10,true);
		}
		else
		{Sleeper(24);};
	};
	
	bHint=false;
	HintType=0;	
	destroy_bitmap(bmpWindow);
	destroy_bitmap(bmpHealth);
	destroy_bitmap(bmpExp);
	buttExit.Activate(-1);
	for (int i=0; i<9; i++)
	{
		if (Loot[i][0]>0)
		{
			mskLoot[i].Activate(-1);				
		};
	};
	for (int i=0; i<2; i++)
	{
		for (int j=0; j<MAX_SQUAD; j++)
		{
			if (Squad[i][j].pUnit)
			{
				mskUnit[i][j].Activate(-1);
			};
		};
	};
	delete objWindow;
	objMain=ShowActiveObjects(objMain,10,false);
};


//Конец сражения
void EndBattle(int Win, int Site)
{
	if (bTutHint)
	{
		objTutorial->iLayer=-1;
		txtTutorial.Hide();
		bTutHint=false;
	};
	for (int k=0; k<2; k++)
	{
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{				
			if (Squad[k][i].pUnit)
			{
				BF_Effect *Eff=Squad[k][i].pEffect, *Eff1;
				int iFound=-1;
				while (Eff)
				{
					Eff1=Eff->pNext;
					if (Eff->iType==73)
					{
						//Возврат захваченных юнитов
						iFound=Eff->iPower;
						if (!Eff->pPrev)
						{
							Squad[k][i].pEffect=Eff1;
						};
					//	if (Squad[k][i].pEffect==Eff)
					//	{Squad[k][i].pEffect=Eff1;};
					//	delete Eff;
					}
					else
					{
						if (Eff->iType==90)
						{	
							//Снятие полиморфа
							Squad[k][i].iLife=Squad[k][i].iLife*Squad[k][i].pTrueForm->GetLife()/Squad[k][i].pUnit->GetLife();
							if (Squad[k][i].iLife<=0)
							{Squad[k][i].iLife=1;};						
							delete Squad[k][i].pUnit;
							Squad[k][i].pUnit=Squad[k][i].pTrueForm;
							if (Squad[k][i].iLife>Squad[k][i].GetLife())
							{Squad[k][i].iLife=Squad[k][i].GetLife();};
							if (Squad[k][i].iAmmo>Squad[k][i].GetAmmo())
							{Squad[k][i].iAmmo=Squad[k][i].GetAmmo();};
							Squad[k][i].G_Obj->Pic=Squad[k][i].pUnit->pType()->Pic;
						//	if (Squad[k][i].pEffect==Eff)
						//	{Squad[k][i].pEffect=Eff1;};
						//	delete Eff;
						};
					};
					Squad[k][i].pEffect=Eff1;
					delete Eff;
					Eff=Eff1;
				};
				Squad[k][i].pEffect=NULL;
				if (iFound>=0)
				{
					Squad[k][i].ChangeSide(iFound);
					Squad[1-k][iFound].iLife=0;
				};
			};
		};
	};
//Стираем графику при автобое
	if (bShowBattle){objLowString->iLayer=-1;};
	if ((!bShowBattle)&&(!bAIBattle))
	{
		G_Object * Temp;
		while (objMain)
		{
			Temp=objMain->pNext;		
			delete objMain;
			objMain=Temp;
		};
		blit(screen1,Background,0,0,0,0,1024,768);
		objMain=new G_Object(NULL,0,0,1,Background);
		if (bAutoCombat)
		{
			bShowBattle=true;
		};
	};

	if (Win>=0)
	{
	if (Heroes[Win]->ipClass1>0)
	{arPlayer[BF_Player[Win]].iWin++;};
	if (Heroes[1-Win]->ipClass1>0)
	{
		arPlayer[BF_Player[1-Win]].iLose++;
		arPlayer[BF_Player[1-Win]].bNoLoss=false;
	};
	int Exp[MAX_SQUAD], Reward[MAX_SQUAD];
	for (int i=0; i<MAX_SQUAD; i++)
	{
		Exp[i]=0;
		Reward[i]=0;
	};
//Подсчет общего опыта за бой
	int ExpPool=SummonExp[1-Win];
	TrophyGold=0, TrophyGem=0;
	
//Бонус за вражеского героя	
	if ((Heroes[1-Win]->ipClass1>0)&&(Squad[1-Win][0].iLife==0))
	{
		ExpPool+=Heroes[1-Win]->iLevel*10;
		TrophyGold+=Heroes[1-Win]->iLevel*(10-(Heroes[1-Win]->ipClass1+Heroes[1-Win]->ipClass1)/2);
		TrophyGem+=Heroes[1-Win]->iLevel*(Heroes[1-Win]->ipClass1+Heroes[1-Win]->ipClass1)/2;
	};

	int arQuest[2][HERO_QUEST_LIMIT];
	for (int i=0; i<HERO_QUEST_LIMIT; i++)
	{
		arQuest[0][i]=0;
		arQuest[1][i]=0;
	};
	
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((Squad[1-Win][i].pUnit)&&(Squad[1-Win][i].iLife==0))
		{
			ExpPool+=(Squad[1-Win][i].pUnit->pType()->iExp+Squad[1-Win][i].pUnit->pType()->iExp*Squad[1-Win][i].pUnit->iLevel/10);
			TrophyGold+=Squad[1-Win][i].pUnit->pType()->iGoldPrice+Squad[1-Win][i].pUnit->pType()->iGoldPrice*Squad[1-Win][i].pUnit->iLevel/10;
			TrophyGem+=Squad[1-Win][i].pUnit->pType()->iGemPrice+Squad[1-Win][i].pUnit->pType()->iGemPrice*Squad[1-Win][i].pUnit->iLevel/10;			
	//Проверка квестов			
			if (Heroes[Win]->ipClass1>0)
			{
				for (int Q=0; Q<HERO_QUEST_LIMIT; Q++)
				{
					if (((pFirstQuest+Heroes[Win]->arQuest[Q].Index)->iType==3)||((pFirstQuest+Heroes[Win]->arQuest[Q].Index)->iType==2))
					{
						for (int j=0; j<MAX_QUEST_TASK; j++)
						{
							if (((pFirstQuest+Heroes[Win]->arQuest[Q].Index)->Index[j]==Squad[1-Win][i].pUnit->GetUnitType())&&(!Heroes[Win]->arQuest[Q].bComplete))
							{
								if (((pFirstQuest+Heroes[Win]->arQuest[Q].Index)->Param[j]==0)||(Random(100)<(pFirstQuest+Heroes[Win]->arQuest[Q].Index)->Param[j]))
								{
									Heroes[Win]->arQuest[Q].arProgress[j]++;									
								};
								arQuest[Win][Q]++;
							};
						};
					};
					if ((pFirstQuest+Heroes[Win]->arQuest[Q].Index)->iType==8)
					{
						if (((pFirstProvince+Heroes[Win]->arQuest[Q].ipProvince)->arSite[Heroes[Win]->arQuest[Q].ipSite][2]==Squad[1-Win][i].pUnit->GetUnitType())&&(!Heroes[Win]->arQuest[Q].bComplete))
						{
							Heroes[Win]->arQuest[Q].arProgress[0]++;
							arQuest[Win][Q]++;
						};
					};
				};
			};
			if ((arPlayer[BF_Player[Win]].Control==1)&&(CurGame.iGameType==2)&&(Campaign.arDialog[687]>0))
			{
				switch (Squad[1-Win][i].pUnit->GetUnitType())
				{
				case 21:
					Campaign.arItems[3]++;
					break;
				case 37:
				case 38:
					Campaign.arItems[3]+=3;
					break;
				case 56:
					Campaign.arItems[3]+=7;
					break;
				case 65:
					Campaign.arItems[3]+=15;
					break;
				};
			};
			if ((arPlayer[BF_Player[Win]].Control==1)&&(WorldRules.iMode==2)&&(WorldRules.arTrigger[0]==2)&&(Squad[1-Win][i].pUnit->GetUnitKind()==2))
			{
				WorldRules.arTrigger[1]+=Squad[1-Win][i].pUnit->GetRank()*Squad[1-Win][i].pUnit->GetRank();
				WorldRules.arTrigger[3]=BF_Player[Win];
			}
		};
		//Немного опыта за сбежавших
		if ((Squad[1-Win][i].pUnit)&&(Squad[1-Win][i].iLife>0)&&(FearVictory>0))
		{
			ExpPool+=(Squad[1-Win][i].pUnit->pType()->iExp+Squad[1-Win][i].pUnit->pType()->iExp*Squad[1-Win][i].pUnit->iLevel/10)/10;
			if (Squad[1-Win][i].iLife<Squad[1-Win][i].GetLife())
			{ExpPool+=(Squad[1-Win][i].pUnit->pType()->iExp+Squad[1-Win][i].pUnit->pType()->iExp*Squad[1-Win][i].pUnit->iLevel/10)*(Squad[1-Win][i].GetLife()-Squad[1-Win][i].iLife)/Squad[1-Win][i].GetLife()/2;}
		};
		//Проверка квестов для проигравшей стороны
		if ((Squad[Win][i].pUnit)&&(Squad[Win][i].iLife==0))
		{
			if (Heroes[1-Win]->ipClass1>0)
			{
				for (int Q=0; Q<HERO_QUEST_LIMIT; Q++)
				{
					if ((pFirstQuest+Heroes[1-Win]->arQuest[Q].Index)->iType==3)
					{
						for (int j=0; j<MAX_QUEST_TASK; j++)
						{
							if (((pFirstQuest+Heroes[1-Win]->arQuest[Q].Index)->Index[j]==Squad[Win][i].pUnit->GetUnitType())&&(!Heroes[1-Win]->arQuest[Q].bComplete))
							{
								Heroes[1-Win]->arQuest[Q].arProgress[j]++;
								arQuest[1-Win][Q]++;
							};
						};
					};
					if ((pFirstQuest+Heroes[1-Win]->arQuest[Q].Index)->iType==8)
					{
						if (((pFirstProvince+Heroes[1-Win]->arQuest[Q].ipProvince)->arSite[Heroes[1-Win]->arQuest[Q].ipSite][2]==Squad[Win][i].pUnit->GetUnitType())&&(!Heroes[1-Win]->arQuest[Q].bComplete))
						{
							Heroes[1-Win]->arQuest[Q].arProgress[0]++;
							arQuest[1-Win][Q]++;
						};
					};
				};
			};
			if ((arPlayer[BF_Player[1-Win]].Control==1)&&(CurGame.iGameType==2)&&(Campaign.arDialog[687]>0))
			{
				switch (Squad[Win][i].pUnit->GetUnitType())
				{
				case 21:
					Campaign.arItems[3]++;
					break;
				case 37:
				case 38:
					Campaign.arItems[3]+=3;
					break;
				case 56:
					Campaign.arItems[3]+=7;
					break;
				case 65:
					Campaign.arItems[3]+=15;
					break;
				};
			};
			if ((arPlayer[BF_Player[1-Win]].Control==1)&&(WorldRules.iMode==2)&&(WorldRules.arTrigger[0]==2)&&(Squad[Win][i].pUnit->GetUnitKind()==2))
			{
				WorldRules.arTrigger[1]+=Squad[Win][i].pUnit->GetRank()*Squad[Win][i].pUnit->GetRank();
				WorldRules.arTrigger[3]=BF_Player[1-Win];
			}

		};
	};
	//Бонус опыта за особые умения
	if ((Squad[Win][0].iLife>0)&&(Heroes[Win]->ipClass1>0))
	{
		ExpPool+=ExpPool*Squad[Win][0].CheckSkill(990)/100;		
	};	

	//30% опыта герою
	if (Heroes[Win]->ipClass1>0)
	{Exp[0]+=ExpPool*3/10;};
	ExpPool-=ExpPool*3/10;

	//28% опыта всем за участие	
	int Work=SummonSurv[Win];
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		if (Squad[Win][i].pUnit)
		{
			Work+=Squad[Win][i].iSurvived;
		};
	};

	for (int i=0; i<MAX_SQUAD; i++)
	{
		if (Squad[Win][i].pUnit)
		{
			Exp[i]+=ExpPool*4*Squad[Win][i].iSurvived/(10*Work);
		};
	};
	ExpPool-=ExpPool*4/10;

	//42% опыта за проделанную работу	
	Work=SummonWork[Win];
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		if (Squad[Win][i].pUnit)
		{
Work+=Squad[Win][i].iMeleeDamageDone*2+Squad[Win][i].iRangeDamageDone+Squad[Win][i].iMeleeDamageTaken
+Squad[Win][i].iRangeDamageTaken+Squad[Win][i].iSpellDamageTaken+Squad[Win][i].iPoisonDamageTaken+
Squad[Win][i].iDamageHealed*2+Squad[Win][i].iSpellCast*3+Squad[Win][i].iEnemyShotted*10+
Squad[Win][i].iEnemySlayed*20+Squad[Win][i].iStaminaUsed+1;
		};
	};

	for (int i=0; i<MAX_SQUAD; i++)
	{
		if (Squad[Win][i].pUnit)
		{
			Exp[i]+=ExpPool*(Squad[Win][i].iMeleeDamageDone*2+Squad[Win][i].iRangeDamageDone+Squad[Win][i].iMeleeDamageTaken
+Squad[Win][i].iRangeDamageTaken+Squad[Win][i].iSpellDamageTaken+Squad[Win][i].iPoisonDamageTaken+
Squad[Win][i].iDamageHealed*2+Squad[Win][i].iSpellCast*3+Squad[Win][i].iEnemyShotted*10+Squad[Win][i].iEnemySlayed*20+
Squad[Win][i].iStaminaUsed+1)/Work;
			if (((i>0)||(Heroes[Win]->ipClass1==0))&&(Squad[Win][i].iLife>0)&&(Squad[Win][i].pUnit->GetUnitKind()!=2))
			{
			int GotIt;
			//За рвение
			if (Squad[Win][i].iStaminaUsed>=Squad[Win][i].pUnit->GetStamina())
			{
				if (Random(21+Squad[Win][i].iStaminaUsed-Squad[Win][i].pUnit->GetStamina())>=20)
				{Reward[i]=5;};
			};
		
			//Медаль за отвагу
			if ((Squad[Win][i].iMorale-Squad[Win][i].GetMorale()-Heroes[Win]->pSquad[i]->iMorale/2>5)&&(Squad[Win][i].iEnemyShotted+Squad[Win][i].iEnemySlayed>0))
			{
				if (Random(21+Squad[Win][i].iMorale-Squad[Win][i].GetMorale())>30)
				{Reward[i]=1;};
			};
			//Медаль за сноровку
			if ((Squad[Win][i].iEnemyShotted+Squad[Win][i].iEnemySlayed>0)&&(Squad[Win][i].iRangeDamageTaken>Squad[Win][i].iLife/2))
			{
				if (Random(21+Squad[Win][i].iRangeDamageTaken-Squad[Win][i].iLife/2)>20)
				{Reward[i]=11;};				
			};
			//Орден Защитника 
			if ((Squad[Win][i].iEnemySlayed>0)&&(Squad[Win][i].iMeleeDamageTaken>Squad[Win][i].iLife/2))
			{
				if (Random(21+Squad[Win][i].iMeleeDamageTaken-Squad[Win][i].iLife/2)>20)
				{Reward[i]=12;};				
			};
			//Медаль за непоколебимость
			if ((Squad[Win][i].iEnemyShotted+Squad[Win][i].iEnemySlayed>0)&&(Squad[Win][i].iSpellDamageTaken>Squad[Win][i].iLife/2))
			{
				if (Random(21+Squad[Win][i].iSpellDamageTaken-Squad[Win][i].iLife/2)>20)
				{Reward[i]=13;};				
			};
			//Орден Победителя
			if (Squad[Win][i].iEnemyShotted+Squad[Win][i].iEnemySlayed>0)
			{
				int Survived=0, Count=0;
				for (int j=0; j<MAX_SQUAD; j++)
				{
					if (Squad[Win][j].pUnit)
					{
						Count++;
						if ((Squad[Win][j].iLife>0)&&((j>0)||(Heroes[Win]->ipClass1==0)))
						{Survived++;};
					};
				};
				if (Survived>0)
				{
					if (Count/Survived>5)
					{
						if (Random(21+Count+Squad[Win][i].iEnemyShotted+Squad[Win][i].iEnemySlayed)>20)
						{Reward[i]=10;};
					};
				}
				else
				{FileLog("!!!!!Error: Survived!!!!");};
			};
			//За стойкость
			if ((Squad[Win][i].iMeleeDamageTaken+Squad[Win][i].iRangeDamageTaken+Squad[Win][i].iSpellDamageTaken+Squad[Win][i].iPoisonDamageTaken>=Squad[Win][i].GetLife())&&(Squad[Win][i].iEnemyShotted+Squad[Win][i].iEnemySlayed>0))
			{
				if (Random(21+Squad[Win][i].iMeleeDamageTaken+Squad[Win][i].iRangeDamageTaken+Squad[Win][i].iSpellDamageTaken+Squad[Win][i].iPoisonDamageTaken-Squad[Win][i].GetLife())>=20)
				{Reward[i]=2;};
			};
			//За волю к победе
			if (Squad[Win][i].iMeleeDamageDone>(Squad[Win][i].pUnit->GetAttack()+Squad[Win][i].pUnit->GetCounterAttack())*3)
			{
				if (Random(21+Squad[Win][i].iMeleeDamageDone-(Squad[Win][i].pUnit->GetAttack()+Squad[Win][i].pUnit->GetCounterAttack())*3)>20)
				{Reward[i]=4;};
			};
			//Медаль Целителя
			if ((Squad[Win][i].iDamageHealed>=Squad[Win][i].CheckSkill(24)*3)&&(Squad[Win][i].CheckSkill(24)>0))
			{
				if (Random(21+Squad[Win][i].iDamageHealed-Squad[Win][i].CheckSkill(24)*3)>=20)
				{
					GotIt=0;
					for (int k=0; k<3; k++)
					{
						if (Squad[Win][i].pUnit->arMedal[k]==9)
						{GotIt++;};
					};
					if (Random(1+GotIt)==0)
					{Reward[i]=9;};
				};
			};
			//Орден снайпера
			if ((Squad[Win][i].pUnit->GetAmmo()>2)&&(Squad[Win][i].pUnit->GetRangedAttack()>0)&&(Squad[Win][i].iRangeDamageDone>=Squad[Win][i].pUnit->GetRangedAttack()*3)&&(Squad[Win][i].CheckSkill(28)==0)&&(Squad[Win][i].CheckSkill(46)==0)&&(Squad[Win][i].CheckSkill(47)==0)&&(Squad[Win][i].iEnemyShotted>1))
			{
				if (Random(21+Squad[Win][i].iRangeDamageDone+Squad[Win][i].iEnemyShotted*5-Squad[Win][i].pUnit->GetRangedAttack()*3)>=30)
				{
					GotIt=0;
					for (int k=0; k<3; k++)
					{
						if (Squad[Win][i].pUnit->arMedal[k]==3)
						{GotIt++;};
					};
					if (Random(1+GotIt)==0)
					{Reward[i]=3;};
				};
			};
			//Знак Чародея
			if ((Squad[Win][i].iSpellCast>=3)&&(Squad[Win][i].iEnemyShotted>0))
			{
				if (Random(10+Squad[Win][i].iSpellCast+Squad[Win][i].iEnemyShotted)>=10)
				{
					GotIt=0;
					for (int k=0; k<3; k++)
					{
						if (Squad[Win][i].pUnit->arMedal[k]==14)
						{GotIt++;};
					};
					if (Random(1+GotIt)==0)
					{Reward[i]=14;};
				};
			};
			//Звезда Мага
			if ((Squad[Win][i].iRangeDamageDone>=Squad[Win][i].pUnit->GetRangedAttack()*3)&&(Squad[Win][i].CheckSkill(28)>0)&&(Squad[Win][i].iEnemyShotted>1))
			{
				if (Random(21+Squad[Win][i].iRangeDamageDone+Squad[Win][i].iEnemyShotted*5-Squad[Win][i].pUnit->GetRangedAttack()*3)>=30)
				{
					GotIt=0;
					for (int k=0; k<3; k++)
					{
						if (Squad[Win][i].pUnit->arMedal[k]==7)
						{GotIt++;};
					};
					if (Random(1+GotIt)==0)
					{Reward[i]=7;};
				};
			};
			//Крест Героя
			if ((Squad[Win][i].iEnemyShotted+Squad[Win][i].iEnemySlayed>4)&&
				(Squad[Win][i].iMeleeDamageTaken+Squad[Win][i].iRangeDamageTaken+Squad[Win][i].iSpellDamageTaken+Squad[Win][i].iPoisonDamageTaken>=Squad[Win][i].GetLife()*2/3))
			{
				if (Random(11+Squad[Win][i].iEnemyShotted+Squad[Win][i].iEnemySlayed)>=13)
				{Reward[i]=8;};
			};
			//Вымпел Пламени
			if ((Squad[Win][i].pUnit->GetAmmo()>2)&&(Squad[Win][i].pUnit->GetRangedAttack()>0)&&(Squad[Win][i].iRangeDamageDone>=Squad[Win][i].pUnit->GetRangedAttack()*3)&&(Squad[Win][i].CheckSkill(28)==0)&&(Squad[Win][i].iEnemyShotted>1)&&
				(Squad[Win][i].iMeleeDamageTaken+Squad[Win][i].iRangeDamageTaken+Squad[Win][i].iSpellDamageTaken+Squad[Win][i].iPoisonDamageTaken>=Squad[Win][i].GetLife()*2/3))
			{
				if (Random(21+Squad[Win][i].iRangeDamageDone+Squad[Win][i].iEnemyShotted*5-Squad[Win][i].pUnit->GetRangedAttack()*Squad[Win][i].pUnit->GetAmmo()*3)>=30)
				{
					GotIt=0;
					for (int k=0; k<3; k++)
					{
						if (Squad[Win][i].pUnit->arMedal[k]==15)
						{GotIt++;};
					};
					if (Random(1+GotIt)==0)
					{Reward[i]=15;};
				};
			};
			};
		};
	};
	//Бонус герою за особые умения
	if ((Squad[Win][0].iLife>0)&&(Heroes[Win]->ipClass1>0))
	{
		Exp[0]+=Exp[0]*Squad[Win][0].CheckSkill(991)/100;
		TrophyGold+=TrophyGold*Squad[Win][0].CheckSkill(992)/10;
		TrophyGem+=TrophyGem*Squad[Win][0].CheckSkill(992)/10;
	};
	TrophyGold+=TrophyGold*arPlayer[BF_Player[Win]].GetBldParam1(41)/100;
	TrophyGem+=TrophyGem*arPlayer[BF_Player[Win]].GetBldParam1(41)/100;

//Раздача опыта и изменение жизни и боевого духа
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((Heroes[Win]->pSquad[i])&&(Squad[Win][i].pUnit))
		{
			if ((i>0)||(Heroes[Win]->ipClass1==0))
			{Heroes[Win]->pSquad[i]->iExp+=Exp[i];};
			Heroes[Win]->pSquad[i]->iLife=Squad[Win][i].iLife*100/LifeMod[Win]-Heroes[Win]->CheckUnitBonus(1)*(100-(Squad[Win][i].GetLife()-Squad[Win][i].iLife)*100/Squad[Win][i].GetLife())/100;
			if ((Heroes[Win]->pSquad[i]->iLife<=0)&&(Squad[Win][i].iLife>0))
			{
				Heroes[Win]->pSquad[i]->iLife=1;
			};
			if (Heroes[Win]->pSquad[i]->iLife<0)
			{
				Heroes[Win]->pSquad[i]->iLife=0;
			};

			if (Heroes[Win]->pSquad[i]->CheckAbility(19)==0)
			{
				if (Squad[Win][i].iMorale-Squad[Win][i].GetMorale()>10)
				{
					Squad[Win][i].iMorale=Squad[Win][i].GetMorale()+10;
				};
				Heroes[Win]->pSquad[i]->iMorale=(Squad[Win][i].iMorale-Squad[Win][i].GetMorale())*2;
			};
			if ((Squad[Win][i].iLife>0)&&(Squad[Win][i].CheckSkill(56)>0))
			{
				TrophyGold-=TrophyGold*Squad[Win][i].CheckSkill(56)/100;
				TrophyGem-=TrophyGem*Squad[Win][i].CheckSkill(56)/100;
			};
		};
		if ((Heroes[1-Win]->pSquad[i])&&(Squad[1-Win][i].pUnit))
		{
			if (Squad[1-Win][i].iMorale-Squad[1-Win][i].GetMorale()>10)
			{
				Squad[1-Win][i].iMorale=Squad[1-Win][i].GetMorale()+10;
			};
			Heroes[1-Win]->pSquad[i]->iLife=Squad[1-Win][i].iLife*100/LifeMod[1-Win]-Heroes[1-Win]->CheckUnitBonus(1)*(100-(Squad[1-Win][i].GetLife()-Squad[1-Win][i].iLife)*100/Squad[1-Win][i].GetLife())/100;
			if ((Heroes[1-Win]->pSquad[i]->iLife<=0)&&(Squad[1-Win][i].iLife>0))
			{
				Heroes[1-Win]->pSquad[i]->iLife=1;
			};
			if (Heroes[1-Win]->pSquad[i]->iLife<0)
			{
				Heroes[1-Win]->pSquad[i]->iLife=0;
			};
			Heroes[1-Win]->pSquad[i]->iMorale=(Squad[1-Win][i].iMorale-Squad[1-Win][i].GetMorale())*2;
		};
	};
	if (Heroes[Win]->ipClass1!=0)
	{
		Heroes[Win]->iExp+=Exp[0];
	};

//Генерация лута	
	int Looter=0;
	if ((Squad[Win][0].iLife>0)&&(Heroes[Win]->ipClass1>0))
	{
		Looter=Squad[Win][0].CheckSkill(992);
	};

	for (int i=0; i<9; i++)
	{
		Loot[i][0]=0;
		Loot[i][1]=0;
	};
	
	if ((Heroes[Win]->ipClass1>0)&&(!bArena))
	{
		if (arPlayer[BF_Player[Win]].Control==1)
		{
			TrophyGold=TrophyGold*arDifficulty[arPlayer[BF_Player[Win]].iDifficulty].iLoot/100;
			TrophyGem=TrophyGem*arDifficulty[arPlayer[BF_Player[Win]].iDifficulty].iLoot/100;
		}
		TrophyGold/=5;
		TrophyGem/=5;
		arPlayer[BF_Player[Win]].Gold+=TrophyGold;
		arPlayer[BF_Player[Win]].Gem+=TrophyGem;
		if (bStaticLoot)
		{srand(Heroes[Win]->ipProvince+Site*10+CurGame.iSeed/9);};
		if (Heroes[1-Win]->ipClass1>0)
		{
			if (Squad[1-Win][0].iLife==0)
			{
				Loot[0][0]=Heroes[1-Win]->DropLoot(&Loot[0][1],Looter);
				if (Loot[0][0]>0)
				{
					Loot[1][0]=Heroes[1-Win]->DropLoot(&Loot[1][1],Looter);					
					if (Loot[1][0]==Loot[0][0])
					{
						Loot[1][0]=0;
					};
				}
				else
				{
					Loot[0][0]=Heroes[1-Win]->DropLoot(&Loot[0][1],Looter);
				};
			};
		}
		else
		{
			arPlayer[BF_Player[Win]].iKarma-=Heroes[1-Win]->ipName;
			if (Heroes[1-Win]->iExp!=0)
			{	
				int iPoss, iNumm;
				if (Heroes[1-Win]->iExp>0)
				{
					iPoss=(pFirstGuard+Heroes[1-Win]->iExp)->LootPoss+Looter;
					iNumm=(pFirstGuard+Heroes[1-Win]->iExp)->LootNum;
				}
				else
				{
					iPoss=(pFirstDefender+abs(Heroes[1-Win]->iExp))->iLootPoss+Looter;
					iNumm=(pFirstDefender+abs(Heroes[1-Win]->iExp))->iLootNum;
				};
				if (Random(100)<iPoss)
				{
					int LootItems=1;
					while ((LootItems<iNumm)&&(Random(100)+Looter>50))
					{LootItems++;};
					for (int R=0; R<LootItems; R++)
					{
						ClearRoll();
						int Count=0;
						for (int i=0; i<MAX_SQUAD; i++)
						{
							if ((Heroes[1-Win]->pSquad[i])&&
								(Heroes[1-Win]->pSquad[i]->pType()->arLoot[0][0][0]>0))
							{
								RollerN[Count]=i+1;
								RollerP[Count]=10;
								Count++;
							};
						};
						if (Count>0)
						{
							int Dropper=Roll(false)-1;							
							ClearRoll();
							Count=0;
							for (int i=0; i<=Heroes[1-Win]->pSquad[Dropper]->iLevel; i++)
							{
								if (i<20)
								{
									for (int j=0; j<MAX_LOOT_PER_LVL; j++)
									{
										if (Heroes[1-Win]->pSquad[Dropper]->pType()->arLoot[0][i][j]>0)
										{
											RollerN[Count]=Heroes[1-Win]->pSquad[Dropper]->pType()->arLoot[0][i][j];
											RollerP[Count]=Heroes[1-Win]->pSquad[Dropper]->pType()->arLoot[1][i][j]*2+i;
											Count++;
										};
									};
								};								
							};
							Loot[R][0]=Roll(false);
							Loot[R][1]=Random((pFirstItem+Loot[R][0])->iDurability)+1;
							switch ((pFirstItem+Loot[R][0])->iRarity)
							{
							case 4:
								arPlayer[BF_Player[Win]].arLoot[0]++;
								break;
							case 5:
								arPlayer[BF_Player[Win]].arLoot[1]++;
								break;
							case 6:
								arPlayer[BF_Player[Win]].arLoot[2]++;
								break;
							};
						};
					};					
				};				
			};
		};
	}
	else
	{
		TrophyGold=0;
		TrophyGem=0;
	};
	if (arPlayer[BF_Player[Win]].Control==1)
	{		
		if (iGameMode!=2)
		{
			iGameMode=6;
			ChangeMusic();
			iGameMode=2;
			iPause=30;
		};
		EndBattleScreen(1,Win);
	}
	else	
	{
		if (arPlayer[BF_Player[1-Win]].Control==1)
		{			
			if (iGameMode!=2)
			{
				iGameMode=7;
				ChangeMusic();
				iGameMode=2;
				iPause=30;
			};
			EndBattleScreen(0,Win);
		};
	};
		
//Проверка квестов
	if (Heroes[Win]->ipClass1>0)
	{
		for (int Q=0; Q<HERO_QUEST_LIMIT; Q++)
		{
			if (((pFirstQuest+Heroes[Win]->arQuest[Q].Index)->iType==3)||((pFirstQuest+Heroes[Win]->arQuest[Q].Index)->iType==2)||((pFirstQuest+Heroes[Win]->arQuest[Q].Index)->iType==8))
			{
				Heroes[Win]->CheckQuestDone(Q);
				if ((!Heroes[Win]->arQuest[Q].bComplete)&&(arQuest[Win][Q]>0)&&((pFirstQuest+Heroes[Win]->arQuest[Q].Index)->ipEncFound>0))
				{
					Heroes[Win]->CallEncounter((pFirstQuest+Heroes[Win]->arQuest[Q].Index)->ipEncFound,NULL,NULL,-1,Heroes[Win]->arQuest[Q].Index,0,0);
				};
			};
		};
	};	
	if (Heroes[1-Win]->ipClass1>0)
	{
		for (int Q=0; Q<HERO_QUEST_LIMIT; Q++)
		{
			if (((pFirstQuest+Heroes[1-Win]->arQuest[Q].Index)->iType==3)||((pFirstQuest+Heroes[1-Win]->arQuest[Q].Index)->iType==8))
			{
				Heroes[1-Win]->CheckQuestDone(Q);
				if ((!Heroes[1-Win]->arQuest[Q].bComplete)&&(arQuest[1-Win][Q]>0)&&((pFirstQuest+Heroes[1-Win]->arQuest[Q].Index)->ipEncFound>0))
				{
					Heroes[1-Win]->CallEncounter((pFirstQuest+Heroes[1-Win]->arQuest[Q].Index)->ipEncFound,NULL,NULL,-1,Heroes[1-Win]->arQuest[Q].Index,0,0);
				};
			};
		};
	};
	if ((WorldRules.iMode==2)&&(WorldRules.arTrigger[0]==2))
	{
		int NewPercent=WorldRules.arTrigger[1]*100/(pFirstWorldRules+WorldRules.iMode)->iParam2;
		if (NewPercent>WorldRules.arTrigger[2])
		{
			WorldRules.arTrigger[2]=NewPercent;
			if (NewPercent>=100)
			{(pFirstProvince+arPlayer[WorldRules.arTrigger[3]].ipProvince)->StaticEvent(1273,WorldRules.arTrigger[3]);}
			else
			{
				char sText[300];
				sprintf(sText,(pFirstDialog+1536)->sText, NewPercent);
				(pFirstDialog+1536)->Show(1,1,0,0,NULL,sText);
			};
		};

	};

	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((Heroes[Win]->pSquad[i])&&(Heroes[Win]->pSquad[i]->iLife>0))
		{
			if ((i==0)&&(Heroes[Win]->ipClass1>0))
			{
	//Повышение героя
				while (Heroes[Win]->iLevel<Heroes[Win]->CountLevel())
				{
					if (arPlayer[BF_Player[Win]].Control==1)
					{
						WindowHeroLvlUp(Heroes[Win]);
					}
					else
					{
						AIHeroLvlUp(Heroes[Win]);
					};
				};
			}
			else
			{
		//Повышение юнитов
				while (Heroes[Win]->pSquad[i]->iLevel<Heroes[Win]->pSquad[i]->CountLevel())
				{
					if ((bShowBattle)&&(arPlayer[BF_Player[Win]].Control==1))
					{
						WindowUnitLvlUp(Heroes[Win]->pSquad[i],BF_Player[Win]);
					}
					else
					{
						Heroes[Win]->pSquad[i]->MakeLevel(Heroes[Win]->pSquad[i]->LevelUp(Heroes[Win]->pSquad[i]->iLevel+1,0));
					};
				};
				if (Reward[i]>0)
				{
					bool bRew=false;
					for (int j=0; j<3; j++)
					{
						if (Heroes[Win]->pSquad[i]->arMedal[j]==0)
						{bRew=true;};
					};
					if (bRew)
					{
						if ((bShowBattle)&&(arPlayer[BF_Player[Win]].Control==1))
						{
							WindowUnitReward(Heroes[Win]->pSquad[i],Reward[i]);
						}
						else
						{
							for (int k=0; k<3; k++)
							{
								if (Heroes[Win]->pSquad[i]->arMedal[k]==0)
								{
									Heroes[Win]->pSquad[i]->arMedal[k]=Reward[i];
									break;
								};
							};							
						};
					};
				};
			};
		};			
	};

	bool NeedLoot=false;
	for (int i=0; i<9; i++)
	{
		if (Loot[i][0]>0)
		{
			if (Heroes[Win]->TakeLoot(Loot[i][0],Loot[i][1]))
			{
			NeedLoot=true;
			}
			else
			{
				Loot[i][0]=0;
			};
		};
	};
	if ((NeedLoot)&&(Heroes[Win]->ipClass1>0))
	{		
		for (int i=0; i<9; i++)
		{
			int Itm=0;
			while ((Loot[Itm][0]==0)&&(Itm<9))
			{
				Itm++;
			};
			if (Itm<9)
			{
				arLootTable[i][0]=Loot[Itm][0];
				arLootTable[i][1]=Loot[Itm][1];
				Loot[Itm][0]=0;
				Loot[Itm][1]=0;
			}
			else
			{
				arLootTable[i][0]=0;
				arLootTable[i][1]=0;
			};			
		};
		if (arPlayer[BF_Player[Win]].Control==1)
		{
			WindowTakeLoot(Heroes[Win]);
		}
		else
		{
			TakeLootAI(Heroes[Win]);
		};
	};

	//Удаление мертвых воинов
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((Heroes[Win]->pSquad[i])&&(Heroes[Win]->pSquad[i]->iLife==0)&&((Heroes[Win]->ipClass1>0)||(bCastle)))
		{
			if (Heroes[Win]->ipClass1>0)
			{arPlayer[BF_Player[Win]].bNoLoss=false;};
			if ((i>0)||(Heroes[Win]->ipClass1==0))
			{
				delete Heroes[Win]->pSquad[i];
				Heroes[Win]->pSquad[i]=NULL;
			};
		};
		if ((Heroes[1-Win]->pSquad[i])&&((Heroes[1-Win]->pSquad[i]->iLife==0)||(FearVictory>0)))
		{
			if ((i>0)||(Heroes[1-Win]->ipClass1==0))
			{
				delete Heroes[1-Win]->pSquad[i];
				Heroes[1-Win]->pSquad[i]=NULL;
			};
		};
	};
	//Некромантия
		if ((Squad[Win][0].iLife>0)&&(Heroes[Win]->ipClass1>0)&&(Heroes[Win]->CheckSkill(21)+Heroes[Win]->CheckHeroBonus(910)>0))
		{
			bool bNecro=false;
			int arNecro[5][MAX_SUMMONS];
			for (int i=0; i<5; i++)
			{
				for (int j=0; j<MAX_SUMMONS; j++)
				{
					arNecro[i][j]=0;
				};
			};
			//Поиск подходящих мертвяков
			for (int i=1; i<MAX_SQUAD+MAX_SUMMONS; i++)
			{
				int Lvl=0;
				if ((Squad[Win][i].pUnit)&&(Squad[Win][i].iLife>0)&&(Squad[Win][i].bSummoned)&&(Squad[Win][i].pUnit->GetUnitKind()==2))
				{
					if (Squad[Win][i].pUnit->GetUnitType()==19)
					{Lvl=1;}else{Lvl=Squad[Win][i].pUnit->GetRank()+1;}
					/*switch (Squad[Win][i].pUnit->GetUnitType())
					{
					case 19:
						Lvl=1;
						break;
					case 20:
						Lvl=2;
						break;
					case 36:
						Lvl=3;
						break;
					case 55:
						Lvl=4;
						break;
					case 64:
						Lvl=5;
						break;
					};*/
					if ((Lvl>0)&&(Heroes[Win]->CheckSkill(21)+Heroes[Win]->CheckHeroBonus(910)>=Lvl)&&(Heroes[Win]->GetUnitSlot(Squad[Win][i].pUnit->pType()->iLevel)>0))
					{
						int Count=0;
						while (arNecro[Lvl-1][Count]>0)
						{
							Count++;
						};
						arNecro[Lvl-1][Count]=i;
						Squad[Win][i].pUnit->iLife=Squad[Win][i].iLife-Heroes[Win]->CheckUnitBonus(1)*(100-(Squad[Win][i].GetLife()-Squad[Win][i].iLife)*100/Squad[Win][i].GetLife())/100;
						if ((Squad[Win][i].pUnit->iLife<=0)&&(Squad[Win][i].iLife>0))
						{
							Squad[Win][i].pUnit->iLife=1;
						};
						bNecro=true;
					};					
				};
			};
			WriteLog("Некромантия");
	//		clear_keybuf();
	//		readkey();
			//Создание окна некромантии
			if ((bNecro)&&(arPlayer[BF_Player[Win]].Control==1))
			{
				int x=512-bmpGUI_Necro->w/2;
				int y=384-bmpGUI_Necro->h/2;
				set_mouse_sprite(bmpMousePointer[1]);
				BITMAP *bmpExp=create_bitmap(bmpGUI_ExpLow->w,bmpGUI_ExpLow->h);
				BITMAP *bmpHealth=create_bitmap(bmpGUI_HealthLow->w,bmpGUI_HealthLow->h);
			//	BITMAP *bmpExp=create_bitmap(bmpGUI_ExpFull->w,bmpGUI_ExpFull->h);
			//	BITMAP *bmpHealth=create_bitmap(bmpGUI_HealthFull->w,bmpGUI_HealthFull->h);
				BITMAP *bmpWindow=create_bitmap(bmpGUI_Necro->w,bmpGUI_Necro->h);
				objMain=new G_Object(objMain,x,y,6,bmpWindow);
				G_Object *objWindow=objMain;
				bool bRepeat=true;
				UI_Button1 buttScroll[5], buttNecro[5], buttExit, buttHero;
				UI_Mask mskUnit[5];
				int Shift[5];
				for (int i=0; i<5; i++)
				{
					objMain=buttScroll[i].Init(x+45+i*90,y+300,1,7,bmpButton[80],bmpButton[81],bmpButton[82],bmpButton[83],objMain);
					buttScroll[i].iHint=321;

					objMain=buttNecro[i].Init(x+45+i*90,y+262,1,7,bmpButton[73],bmpButton[74],bmpButton[75],bmpButton[120],objMain);
					buttNecro[i].iHint=322;

					objMain=mskUnit[i].Init(x+32+i*90,y+149,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,objMain,7);
					Shift[i]=0;
				};
				objMain=buttExit.Init(x+338,y+351,1,7,bmpButton[256],bmpButton[257],bmpButton[258],bmpButton[258],objMain);
				buttExit.iHint=135;
				objMain=buttHero.Init(x+107,y+351,1,7,bmpButton[250],bmpButton[251],bmpButton[252],bmpButton[253],objMain);
				buttHero.iHint=146;				
				UI_TextBox txtNecro;
				objMain=txtNecro.Init(x+39,y+39,430,96,7,COLOR_1,Font2,objMain);
				txtNecro.Activate((pFirstMessage+17)->msg);
				while (bRepeat)
				{
					blit(bmpGUI_Necro,bmpWindow,0,0,0,0,bmpGUI_Necro->w,bmpGUI_Necro->h);
					textprintf_centre_ex(bmpWindow,Font1,252,13,COLOR_1,-1,"%s",(pFirstPhrase+320)->str);
					textprintf_centre_ex(bmpWindow,Font1,262,364,COLOR_2,-1,"%d",arPlayer[BF_Player[Win]].Gem);
					for (int i=0; i<5; i++)
					{
						if (arNecro[i][0]>0)
						{
							clUnitType *UT=(pFirstUnitType+Squad[Win][arNecro[i][Shift[i]]].pUnit->GetUnitType());
							DrawUnit(bmpWindow, Squad[Win][arNecro[i][Shift[i]]].pUnit, 38+i*90,155, -1000, bmpExp, bmpHealth, NULL);							
							mskUnit[i].Activate(7);
							if (arPlayer[BF_Player[Win]].Gem>=UT->iGoldPrice/2)
							{
								textprintf_centre_ex(bmpWindow,Font1,79+i*90,245,COLOR_2,-1,"%d",UT->iGoldPrice/2);
						//		if (Heroes[Win]->GetFreeUnitSlot(UT->iLevel)>0)
						//		{
									buttNecro[i].Activate(1);
						//		}
						//		else
						//		{
						//			buttNecro[i].Activate(0);
						//		};
							}
							else
							{
								textprintf_centre_ex(bmpWindow,Font1,79+i*90,245,makecol(250,20,20),-1,"%d",UT->iGoldPrice/2);
								buttNecro[i].Activate(0);
							};
							if (arNecro[i][1]>0)
							{
								buttScroll[i].Activate(1);
							}
							else
							{
								buttScroll[i].Activate(0);
							};
						}
						else
						{
							mskUnit[i].Activate(-1);
							buttScroll[i].Activate(0);
							buttNecro[i].Activate(0);
							if (Heroes[Win]->CheckSkill(21)+Heroes[Win]->CheckHeroBonus(910)<=i)
							{
								draw_sprite(bmpWindow,bmpGUI_UnitClosed,38+i*90,155);
							};
						};
					};
					bool bRedraw=true;
					while ((bRepeat)&&(bRedraw))
					{	
						for (int i=0; i<5; i++)
						{
							int Check=mskUnit[i].Check();
							if ((Check==30)||(Check==40))
							{
								Squad[Win][arNecro[i][Shift[i]]].ShowInfoScreen();
							//	WindowUnitInfo(Squad[Win][arNecro[i][Shift[i]]].pUnit,0);
							};
							if (buttScroll[i].Check())
							{
								Shift[i]++;
								if (arNecro[i][Shift[i]]==0)
								{
									Shift[i]=0;
								};
								DrawUnit(bmpWindow, Squad[Win][arNecro[i][Shift[i]]].pUnit, 38+i*90,155, -1000, bmpExp, bmpHealth, NULL);								
							};
							if (buttNecro[i].Check())
							{
								clUnitType *UT=(pFirstUnitType+Squad[Win][arNecro[i][Shift[i]]].pUnit->GetUnitType());
								int Slot=Heroes[Win]->GetFreeUnitSlot(UT->iLevel);
								if (Slot>0)
								{
									Heroes[Win]->PlaceUnit(Squad[Win][arNecro[i][Shift[i]]].pUnit,Slot);
									Heroes[Win]->pSquad[Slot]->iLife=Squad[Win][arNecro[i][Shift[i]]].pUnit->iLife;
								}
								else
								{
									ScreenHero(Heroes[Win],Squad[Win][arNecro[i][Shift[i]]].pUnit,false);
								};								
								Squad[Win][arNecro[i][Shift[i]]].pUnit=NULL;
								arPlayer[BF_Player[Win]].Gem-=UT->iGoldPrice/2;
						//		Squad[Win][arNecro[i][Shift[i]]].pUnit=NULL;
								int Count=Shift[i];
								while ((Count<MAX_SUMMONS-1)&&(arNecro[i][Count]>0))
								{
									arNecro[i][Count]=arNecro[i][Count+1];
									Count++;
								};
								arNecro[i][MAX_SUMMONS-1]=0;
								if (arNecro[i][Shift[i]]==0)
								{
									Shift[i]=0;
								};
								bRedraw=false;
							};								
						};
						if (buttExit.Check())
						{
							bRepeat=false;
						};
						if (buttHero.Check())
						{
							bRepeat=ScreenHero(Heroes[Win],NULL,false);
					//		bRedraw=false;
						};
						if (Timer)
						{
							Timer=false;
							objMain=ShowActiveObjects(objMain,10,1);
						}
						else
						{Sleeper(24);};
						bHint=false;
					};
				};//Repeat
				destroy_bitmap(bmpWindow);
				destroy_bitmap(bmpHealth);
				destroy_bitmap(bmpExp);		
				delete objWindow;
			}
			else
			{
				if (bNecro)
				{
		//Некромантия для AI
					bool bEvil=false, bGood=false;
					for (int i=1; i<MAX_SQUAD; i++)
					{
						if ((Heroes[Win]->pSquad[i])&&(Squad[Win][i].iLife>0))
						{
							if (Heroes[Win]->pSquad[i]->GetKarma()>0)
							{bGood=true;};
							if (Heroes[Win]->pSquad[i]->GetKarma()<0)
							{bEvil=true;};
						};
					};
					if ((bEvil)||(!bGood))
					{
						int Slot;
						ArrangeUnits(Heroes[Win],0);
						for (int Und=4; Und>=0; Und--)
						{
							if (Und>0)
							{Slot=Und;}
							else
							{Slot=1;};
							int Corpse=1;
							while ((Corpse>=0)&&(Heroes[Win]->GetFreeUnitSlot(Slot)>0))
							{
								Corpse=-1;
								int Num;
								for (int k=0; k<MAX_SUMMONS; k++)
								{
									if ((arNecro[Und][k]>0)&&((Corpse<0)||(Squad[Win][arNecro[Und][k]].iLife>Squad[Win][Corpse].iLife)))
									{
										Corpse=arNecro[Und][k];
										Num=k;
									};
								};
								if (Corpse>=0)
								{
									int FreeSlot=Heroes[Win]->GetFreeUnitSlot(Slot);									
									Heroes[Win]->PlaceUnit(Squad[Win][Corpse].pUnit,FreeSlot);
									Heroes[Win]->pSquad[FreeSlot]->iLife=Squad[Win][Corpse].pUnit->iLife;
									arNecro[Und][Num]=0;
									Squad[Win][Corpse].pUnit=NULL;
								};
							};
						};

					};
				};

			};
		};
	}
	else
//Ничья
	{
		WriteLog("Ничья");
		for (int j=0; j<2; j++)
		{			
			for (int i=0; i<MAX_SQUAD; i++)
			{
				if ((Heroes[j]->pSquad[i])&&(Squad[j][i].pUnit))
				{
					Heroes[j]->pSquad[i]->iLife=Squad[j][i].iLife*100/LifeMod[j]-Heroes[j]->CheckUnitBonus(1)*(100-(Squad[j][i].GetLife()-Squad[j][i].iLife)*100/Squad[j][i].GetLife())/100;
					if ((Heroes[j]->pSquad[i]->iLife<=0)&&(Squad[j][i].iLife>0))
					{
						Heroes[j]->pSquad[i]->iLife=1;
					};
					if (Heroes[j]->pSquad[i]->iLife<0)
					{
						Heroes[j]->pSquad[i]->iLife=0;
					};
		//			Heroes[j]->pSquad[i]->iLife=Squad[j][i].iLife;
					if (Squad[j][i].iMorale-Squad[j][i].GetMorale()>10)
					{
						Squad[j][i].iMorale=Squad[j][i].GetMorale()+10;
					};
					Heroes[j]->pSquad[i]->iMorale=(Squad[j][i].iMorale-Squad[j][i].GetMorale())*2;
				};
			};					
		};
		for (int i=0; i<9; i++)
		{
			Loot[i][0]=0;
			Loot[i][1]=0;
		};
		if (arPlayer[BF_Player[0]].Control==1)
		{			
			if (iGameMode!=2)
			{
				iGameMode=7;
				ChangeMusic();
				iGameMode=2;
				iPause=30;
			};
			EndBattleScreen(-1,0);
		}
		else
		{
			if (arPlayer[BF_Player[1]].Control==1)
			{				
				if (iGameMode!=2)
				{
					iGameMode=7;
					ChangeMusic();
				};
				EndBattleScreen(-1,1);
			};
		};
		//Проверка квестов
		for (int j=0; j<2; j++)
		{
			if (Heroes[j]->ipClass1>0)
			{
				int arQuest[HERO_QUEST_LIMIT];
				for (int Q=0; Q<HERO_QUEST_LIMIT; Q++)
				{
					arQuest[Q]=0;
				};
				for (int i=0; i<MAX_SQUAD; i++)
				{
					if ((Squad[1-j][i].pUnit)&&(Squad[1-j][i].iLife==0))
					{
						for (int Q=0; Q<HERO_QUEST_LIMIT; Q++)
						{
							if ((pFirstQuest+Heroes[j]->arQuest[Q].Index)->iType==3)
							{
								for (int k=0; k<MAX_QUEST_TASK; k++)
								{
									if (((pFirstQuest+Heroes[j]->arQuest[Q].Index)->Index[k]==Squad[1-j][i].pUnit->GetUnitType())&&(!Heroes[j]->arQuest[Q].bComplete))
									{
										Heroes[j]->arQuest[Q].arProgress[k]++;
										arQuest[Q]++;
									};
								};
							};
							if ((pFirstQuest+Heroes[j]->arQuest[Q].Index)->iType==8)
							{
								if (((pFirstProvince+Heroes[j]->arQuest[Q].ipProvince)->arSite[Heroes[j]->arQuest[Q].ipSite][2]==Squad[1-j][i].pUnit->GetUnitType())&&(!Heroes[j]->arQuest[Q].bComplete))
								{
									Heroes[j]->arQuest[Q].arProgress[0]++;
									arQuest[Q]++;
								};
							};
						};
					};
				};
				for (int Q=0; Q<HERO_QUEST_LIMIT; Q++)
				{
					if (((pFirstQuest+Heroes[j]->arQuest[Q].Index)->iType==3)||((pFirstQuest+Heroes[j]->arQuest[Q].Index)->iType==8))
					{
						Heroes[j]->CheckQuestDone(Q);
						if ((!Heroes[j]->arQuest[Q].bComplete)&&(arQuest[Q]>0)&&((pFirstQuest+Heroes[j]->arQuest[Q].Index)->ipEncFound>0))
						{
							Heroes[j]->CallEncounter((pFirstQuest+Heroes[j]->arQuest[Q].Index)->ipEncFound,NULL,NULL,-1,Heroes[j]->arQuest[Q].Index,0,0);
						};
					};
				};
			};
		};	
	
		for (int j=0; j<2; j++)
		{
			int iDead=0;
			ClearRoll();			
			for (int i=0; i<MAX_SQUAD; i++)
			{
				if ((Heroes[j]->pSquad[i])&&(Squad[j][i].pUnit))
				{						
					if (Heroes[j]->pSquad[i]->iLife==0)
					{
						if ((Heroes[j]->ipClass1>0)||(bCastle))
						{
							if ((i>0)||(Heroes[j]->ipClass1==0))
							{
								delete Heroes[j]->pSquad[i];
								Heroes[j]->pSquad[i]=NULL;
							};
						}
						else
						{							
							RollerN[iDead]=i+1;
							RollerP[iDead]=5-Heroes[j]->pSquad[i]->pType()->iLevel;
							iDead++;
						};
					}
					else
					{
						
					};
				};
			};
			if (iDead>0)
			{
				int Lucky=Roll(false)-1;
				Heroes[j]->pSquad[Lucky]->iLife=1;
			};
		};
	};

	for (int A=0; A<2; A++)
	{
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if ((Squad[A][i].pUnit)&&(Squad[A][i].bSummoned))
			{
				delete Squad[A][i].pUnit;
				Squad[A][i].pUnit=NULL;
			};
		};
	};
};

//****************************************************
//**** Выбор заклинания ******************************
//****************************************************

int Spellbook()
{
	iAntisleep=iAntisleepDelay;
	BITMAP *bmpSpellBack=create_bitmap(1024,768);
	int arCast[SPELLBOOK_SIZE][2];
	for (int i=0; i<SPELLBOOK_SIZE; i++)
	{
		arCast[i][0]=0;
		arCast[i][1]=0;
	};
	G_Object *objSpell=NULL;
	UI_Mask SpellMask[SPELLBOOK_SIZE];
	UI_Button1 buttExit;
	objSpell=buttExit.Init(262+431,112+549,1,3,bmpButton[312],bmpButton[313],bmpButton[314],bmpButton[314],objSpell);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	ShowActiveObjects(objMain,10,false);
	blit(screen1,bmpSpellBack,0,0,0,0,1024,768);
	draw_sprite(bmpSpellBack, bmpSpellbook, 262,112);	
	BITMAP *bmpBlack=create_bitmap(52,52);
	rectfill(bmpBlack,0,0,52,52,COLOR_1);
	BITMAP *bmpRed=create_bitmap(52,52);
	rectfill(bmpRed,0,0,52,52,makecol(100,0,0));
//	blit(bmpSpellbook,Crystall,290,490,0,0,177,30);
	if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
	{
	for (int i=0; i<SPELLBOOK_SIZE; i++)
	{
	//	SpellMask[i]=new UI_Mask();
		if (Spells[AA][0][i])
		{
			draw_sprite(bmpSpellBack,(pFirstSpell+Spells[AA][0][i])->Pic,302+(i%3)*64,146+(i-i%3)/3*66);
			objSpell=SpellMask[i].Init(296+(i%3)*64,140+(i-i%3)/3*66,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objSpell,2);
			if ((Spells[AA][1][i])&&(((pFirstSpell+Spells[AA][0][i])->iCost<=0)||(arPlayer[BF_Player[AA]].Gem>=(pFirstSpell+Spells[AA][0][i])->iCost*(100-ActiveUnit->pHero->CheckHeroBonus(983))/100)))
			{
				arCast[i][1]=1;
			//	textprintf_ex(screen,Font1,0+i*40,600,makecol(250,250,250),-1,"%d,",i);
				
			//	textprintf_ex(screen,Font1,200,600,makecol(250,250,250),-1,"passed");
			}
			else
			{
				
				if (Spells[AA][1][i])
				{
					draw_trans_sprite(bmpSpellBack,bmpRed,302+(i%3)*64,146+(i-i%3)/3*66);
					draw_sprite(bmpSpellBack,bmpGUI_NoGem,302+(i%3)*64+23,146+(i-i%3)/3*66+2);
				}
				else
				{
					draw_trans_sprite(bmpSpellBack,bmpBlack,302+(i%3)*64,146+(i-i%3)/3*66);
					draw_trans_sprite(bmpSpellBack,bmpStuff[40],302+(i%3)*64,146+(i-i%3)/3*66);
					draw_trans_sprite(bmpSpellBack,bmpStuff[41],302+(i%3)*64,146+(i-i%3)/3*66);					
				};
			};
		}
	};
	textprintf_centre_ex(bmpSpellBack,Font1,405,612,COLOR_1,-1,"%d",arPlayer[BF_Player[AA]].Gem);
//	textprintf_centre_ex(bmpSpellBack,Font1,405,612,makecol(30,130,150),-1,"%d",arPlayer[BF_Player[AA]].Gem);
	}
	else
	{
		int iCount=0;
		for (int i=1; i<=iMaxSpell; i++)
		{
			if (ActiveUnit->CheckSkill(2000+i))
			{
				arCast[iCount][0]=i;
				draw_sprite(bmpSpellBack,(pFirstSpell+i)->Pic,302+(iCount%3)*64,146+(iCount-iCount%3)/3*66);
				objSpell=SpellMask[iCount].Init(296+(iCount%3)*64,140+(iCount-iCount%3)/3*66,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objSpell,2);
				if (ActiveUnit->iAmmo<ActiveUnit->CheckSkill(2000+i))
				{
					draw_sprite(bmpSpellBack,bmpSpellShadow,302+(iCount%3)*64,146+(iCount-iCount%3)/3*66);
				}
				else
				{
					arCast[iCount][1]=1;
				};
				iCount++;
			};
		};
		textprintf_centre_ex(bmpSpellBack,Font1,406,613,COLOR_1,-1,"%d",ActiveUnit->iAmmo);
		textprintf_centre_ex(bmpSpellBack,Font1,405,612,makecol(20,130,150),-1,"%d",ActiveUnit->iAmmo);
	};
//	blit(screen,Background,0,0,0,0,1024,768);
	objSpell=new G_Object(objSpell,0,0,0,bmpSpellBack);
	UI_TextBox NameBox;
//	objSpell=new G_Object(objSpell,597,607,1,Crystall);
	objSpell=NameBox.Init(530,152,189,30,4,makecol(50,250,250),Font1,objSpell);
	NameBox.bCentre=true;
	NameBox.bShadow=true;

	UI_TextBox PowerBox;
	objSpell=PowerBox.Init(530,172,189,30,4,makecol(250,100,10),Font1,objSpell);
	PowerBox.bCentre=true;
	PowerBox.bShadow=true;

	UI_TextBox TextBox;
	objSpell=TextBox.Init(530,182,189,425,4,COLOR_1,Font2,objSpell);
//	objSpell=TextBox.Init(530,182,189,80,4,COLOR_2,Font1,objSpell,500,190,500,270);
	
	int Result=-1;
	bool bMouse, bHide;
	int Check;
	int Infoz=-1;
	while ((Result<0)&&(!buttExit.Check()))
	{
		bMouse=true;
		bHide=true;
		for (int i=0; i<SPELLBOOK_SIZE; i++)
		{
			Check=SpellMask[i].Check();
			if ((Check==30)&&(arCast[i][1]))
			{Result=i;};
			if (Check>1)
			{
				bHide=false;
				if (Infoz!=i)
				{
					int CurSpell;
					if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
					{CurSpell=Spells[AA][0][i];}
					else
					{CurSpell=arCast[i][0];}
				//	{
						NameBox.Activate((pFirstSpell+CurSpell)->sName);
						int CurEff=0, CurPhrase=552, CurPower=0;
						switch ((pFirstSpell+CurSpell)->arEffect[0][0])
						{
						case 31:
						case 47:
						case 89:
						case 92:
							TextBox.Obj->y=182;
							break;
						case 72:
						case 71:
							TextBox.Obj->y=212;
							CurEff=(pFirstSpell+CurSpell)->arEffect[0][1];
							CurPower=(pFirstSpell+CurSpell)->arEffect[1][1];
							break;
						case 73:
							if ((pFirstSpell+CurSpell)->iDurationMod==0)
							{
								TextBox.Obj->y=182;
								break;
							}							
						default:
							TextBox.Obj->y=212;
							CurEff=(pFirstSpell+CurSpell)->arEffect[0][0];
							CurPower=(pFirstSpell+CurSpell)->arEffect[1][0];
							break;
						}
						if (CurEff>0)
						{
							if ((CurEff>1000)&&((pFirstSpell+CurSpell)->iDurationMod<=0))
							{
								CurPhrase=554;
								CurPower=ActiveUnit->CheckSkill(907);
								if ((pFirstUnitType+CurEff%1000)->iUnitKind==2)
								{
									CurPower/=2;
									CurPower+=ActiveUnit->CheckSkill(909);
								}
							}
							else
							{
								if ((pFirstSpell+CurSpell)->iDurationMod>0)
								{
									CurPhrase=553;
									CurPower=(pFirstSpell+CurSpell)->arEffect[2][0]+ActiveUnit->CheckSkill(906)*(pFirstSpell+CurSpell)->iDurationMod/100;
								}
								else
								{
									CurPower+=ActiveUnit->CheckSkill(905)*(pFirstSpell+CurSpell)->iPowerMod/100;
								}
							}
							char txtPower[200];
							if ((CurEff==1)&&((pFirstSpell+CurSpell)->iTarget==4))
							{sprintf(txtPower,"%s: %d~",(pFirstPhrase+CurPhrase)->str,CurPower);}
							else
							{sprintf(txtPower,"%s: %d",(pFirstPhrase+CurPhrase)->str,CurPower);}							
							PowerBox.Activate(txtPower);
						};					
						TextBox.Activate((pFirstSpell+CurSpell)->sText);
						if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
					{
						blit(bmpSpellbook,bmpSpellBack,300,490,262+300,112+490,180,30);					
						textprintf_centre_ex(bmpSpellBack,Font1,262+316,112+498,COLOR_1,-1,"%d",(pFirstSpell+CurSpell)->iCost*(100-ActiveUnit->pHero->CheckHeroBonus(983))/100);					
					}
					else
					{
				//		NameBox.Activate((pFirstSpell+arCast[i][0])->sName);
				//		TextBox.Activate((pFirstSpell+arCast[i][0])->sText);
						blit(bmpSpellbook,bmpSpellBack,300,490,262+300,112+490,180,30);					
						textprintf_centre_ex(bmpSpellBack,Font1,262+316,112+498,COLOR_1,-1,"%d",ActiveUnit->CheckSkill(2000+arCast[i][0]));					
					};
					textprintf_centre_ex(bmpSpellBack,Font1,262+425,112+498,COLOR_1,-1,"%d",(pFirstSpell+CurSpell)->iStamCost);
				//	textprintf_centre_ex(bmpSpellBack,Font1,262+425,112+498,makecol(20,130,150),-1,"%d",(pFirstSpell+Spells[AA][0][i])->iStamCost);
					Infoz=i;					
				};				
				if (bMouse&&(mouse_pointer!=1))
				{
					set_mouse_sprite(bmpMousePointer[1]);
					mouse_pointer=1;					
				};
				bMouse=false;
			}
		};
		if ((bHide)&&(Infoz>=0))
		{
			NameBox.Hide();
			TextBox.Hide();
			PowerBox.Hide();
			blit(bmpSpellbook,bmpSpellBack,300,490,262+300,112+490,180,30);
	//		blit(bmpSpellbook,Crystall,335,495,0,0,60,30);
			Infoz=-1;
		};
		if (bMouse&&(mouse_pointer>0))
		{
			set_mouse_sprite(bmpMousePointer[0]);
			mouse_pointer=0;
		};
		if (Timer)
		{
			Timer=false;
			ShowActiveObjects(objSpell,5,true);
		}
		else
		{Sleeper(24);}		
	};

	G_Object *Temp;
	while (objSpell)
	{
		Temp=objSpell->pNext;		
		delete objSpell;
		objSpell=Temp;
	};
	destroy_bitmap(bmpBlack);
	destroy_bitmap(bmpRed);
	destroy_bitmap(bmpSpellBack);
//	MakeBackground();
	if (Result>=0)
	{
		HideTargets();
		blit(bmpSpellPanel,objPanel,0,0,0,0,jPanel->w,jPanel->h);		
		if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
		{			
			draw_sprite(objPanel,(pFirstSpell+Spells[AA][0][Result])->Pic,44,21);
			draw_sprite(objPanel,(pFirstSpell+Spells[AA][0][Result])->Pic,44,213);
			txtSpName.Activate((pFirstSpell+Spells[AA][0][Result])->sName);
			txtSpDesc.Activate((pFirstSpell+Spells[AA][0][Result])->sText);
			if ((pFirstSpell+Spells[AA][0][Result])->bSacrifice)
			{txtSpTarget.Activate((pFirstPhrase+253)->str);}
			else
			{txtSpTarget.Activate((pFirstPhrase+254)->str);};			
		}
		else
		{
			Result=arCast[Result][0];
			draw_sprite(objPanel,(pFirstSpell+Result)->Pic,44,21);
			draw_sprite(objPanel,(pFirstSpell+Result)->Pic,44,213);		
			txtSpName.Activate((pFirstSpell+Result)->sName);
			txtSpDesc.Activate((pFirstSpell+Result)->sText);
			if ((pFirstSpell+Result)->bSacrifice)
			{txtSpTarget.Activate((pFirstPhrase+253)->str);}
			else
			{txtSpTarget.Activate((pFirstPhrase+254)->str);};
		};
		HideButtons();		
		buttEndTurn.Activate(-1);
		buttMenu.Activate(-1);
		buttCancelSpell.Activate(1);
	};
	iAntisleep=iAntisleepDelay;
	return Result;
};

//Ищет способного действовать юнита и возвращает ссылку на него.
BF_Unit * FindNextUnit()
{	
	int Found=-1;
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{	
		if ((Found>=0)&&(Squad[AA][i].bCanAct)&&(Squad[AA][i].iLife>0))
		{			
			return &Squad[AA][i];
		}
		if (&Squad[AA][i]==ActiveUnit)
		{Found=i;};
	};
	if (Found>=0)
	{
		for (int i=0; i<Found; i++)
		{	
			if ((Squad[AA][i].bCanAct)&&(Squad[AA][i].iLife>0))
			{
				return &Squad[AA][i];
			};
		}
	}
	else
	{
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{	
			if ((Squad[AA][i].bCanAct)&&(Squad[AA][i].iLife>0))
			{
				return &Squad[AA][i];
			};
		};
	};
	return NULL;
};

int CalcColor(int Curr, int Max)
{
	if (Max==0)
	{
		Max=1;
		Curr++;
	};
	if (Curr<Max)
		{return makecol(220,220-(Max-Curr)*210/(Max),50);}
	else
	{
		if (Curr<=Max+5)
			{return makecol(220-(Curr-Max)*40,220,50);}
		else
			{return makecol(230,230,230);};
	}; 
};

void ShowEnemy(BF_Unit *Unit)
{
//Вывод информационной панели
	blit(jPanel,bmpSideBar,0,0,0,0,jPanel->w,700);
	draw_sprite(bmpSideBar,Unit->pUnit->pType()->Pic,7,15);
	textprintf_centre_ex(bmpSideBar,Font1,115,26,CalcColor(Unit->iLife,Unit->GetLife()),-1,"%d",Unit->iLife);
	textprintf_centre_ex(bmpSideBar,Font1,115,52,CalcColor(Unit->iStamina,Unit->GetStamina()),-1,"%d",Unit->iStamina);	
	textprintf_centre_ex(bmpSideBar,Font1,115,78,CalcColor(Unit->iMorale,Unit->GetMorale()),-1,"%d",Unit->iMorale);
	if ((Unit->pHero)&&(Unit->pHero->ipClass1>0))
	{textprintf_centre_ex(bmpSideBar,Font1,33,104,makecol(210,20,250),-1,"%d",Unit->pHero->iLevel);}
	else
	{textprintf_centre_ex(bmpSideBar,Font1,33,104,makecol(210,20,250),-1,"%d",Unit->pUnit->iLevel);};
	for (int i=0; i<3; i++)
	{		
		if (Unit->pUnit->arMedal[i]>0)
		{
			draw_sprite(bmpSideBar,(pFirstMedal+Unit->pUnit->arMedal[i])->SmallPic,45+i*13,102);
		};
	};
	textprintf_centre_ex(bmpSideBar,Font1,115,104,CalcColor(Unit->iSpeed,Unit->GetSpeed()),-1,"%d  ",Unit->iSpeed);
	textprintf_centre_ex(bmpSideBar,Font1,115,104,makecol(220,220,50),-1," /%d",Unit->GetSpeed());
	textprintf_centre_ex(bmpSideBar,Font1,50,132,CalcColor(Unit->GetAttack(),Unit->pUnit->GetAttack()),-1,"%d",Unit->GetAttack());
	textprintf_centre_ex(bmpSideBar,Font1,111,132,CalcColor(Unit->GetCounterAttack(),Unit->pUnit->GetCounterAttack()),-1,"%d",Unit->GetCounterAttack());
	textprintf_centre_ex(bmpSideBar,Font1,50,132+31,CalcColor(Unit->GetRangedAttack(),Unit->pUnit->GetRangedAttack()),-1,"%d",Unit->GetRangedAttack());
	textprintf_centre_ex(bmpSideBar,Font1,111,132+31,CalcColor(Unit->GetShootingRange(),Unit->pUnit->GetShootingRange()),-1,"%d",Unit->GetShootingRange());
	textprintf_centre_ex(bmpSideBar,Font1,50,132+31*2,CalcColor(Unit->iAmmo,Unit->GetAmmo()),-1,"%d",Unit->iAmmo);
	textprintf_centre_ex(bmpSideBar,Font1,111,132+31*2,CalcColor(Unit->GetResist(),Unit->pUnit->GetResist()),-1,"%d",Unit->GetResist());
	textprintf_centre_ex(bmpSideBar,Font1,50,132+31*3,CalcColor(Unit->GetDefence(),Unit->pUnit->GetDefence()),-1,"%d",Unit->GetDefence());
	textprintf_centre_ex(bmpSideBar,Font1,111,132+31*3,CalcColor(Unit->GetRangedDefence(),Unit->pUnit->GetRangedDefence()),-1,"%d",Unit->GetRangedDefence());
	textprintf_centre_ex(bmpSideBar,Font1,71,482,COLOR_1,-1,"%s:",(pFirstPhrase+252)->str);
	int dX=13, dY=498, dW=62, dH=68;
	BF_Effect *Eff=Unit->pEffect;
	int Spellz=0;
	for (int i=0; i<6; i++)
	{arEff[i]=NULL;};
	while ((Eff)&&(Spellz<6))
	{
		if (Eff->bVisible)
		{
			if (Eff->pSpell)
			{draw_sprite(bmpSideBar,Eff->pSpell->Pic,dX+dW*(Spellz%2),dY+dH*(Spellz-Spellz%2)/2);};
			if (Eff->pAbility)
			{draw_sprite(bmpSideBar,Eff->pAbility->Pic,dX+dW*(Spellz%2),dY+dH*((Spellz-Spellz%2)/2));};
			if (Eff->iTime>0)
			{
				textprintf_ex(bmpSideBar,Font1,dX+24+dW*(Spellz%2),dY+54+dH*(Spellz-Spellz%2)/2,makecol(0,0,0),-1,"%d",Eff->iTime);
				textprintf_ex(bmpSideBar,Font1,dX+23+dW*(Spellz%2),dY+53+dH*(Spellz-Spellz%2)/2,COLOR_2,-1,"%d",Eff->iTime);
			};
			arEff[Spellz]=Eff;
			mskEff[Spellz].Activate(1);
			Spellz++;
		};
		Eff=Eff->pNext;
	};
	int iAbil=0;
	if (Unit->bSummoned)
	{
		draw_sprite(bmpSideBar,(pFirstAbilityNum+103)->Pic,14+(iAbil%2)*62, 247+(iAbil/2)*56);
		iAbil++;
	};
	for (int i=13; ((i<3000)&&(iAbil<8)); i++)
	{
		if ((Unit->CheckSkill(i)>0)&&(iAbil<8))
		{			
			for (int j=1; j<=iMaxAbilityNum; j++)
			{
				if ((pFirstAbilityNum+j)->Num==i)
				{
					draw_sprite(bmpSideBar,(pFirstAbilityNum+j)->Pic,14+(iAbil%2)*62, 247+(iAbil/2)*56);
					if ((pFirstAbilityNum+j)->bNumeric)
					{
						ShowIconNumber(bmpSideBar, 14+(iAbil%2)*62, 247+(iAbil/2)*56, Unit->CheckSkill(i));					
					};
					iAbil++;
					break;
				};
			};
		};
	};
};


//***********************************************************
//*****  Выбор активного юнита  *****************************
//***********************************************************


void ShowUnit(BF_Unit *Unit)
{
//Вывод информационной панели
	blit(jPanel,objPanel,0,0,0,0,jPanel->w,jPanel->h);
	draw_sprite(objPanel,Unit->pUnit->pType()->Pic,7,15);
	textprintf_centre_ex(objPanel,Font1,115,26,CalcColor(Unit->iLife,Unit->GetLife()),-1,"%d",Unit->iLife);
	textprintf_centre_ex(objPanel,Font1,115,52,CalcColor(Unit->iStamina,Unit->GetStamina()),-1,"%d",Unit->iStamina);	
	textprintf_centre_ex(objPanel,Font1,115,78,CalcColor(Unit->iMorale,Unit->GetMorale()),-1,"%d",Unit->iMorale);
	if ((Unit->pHero)&&(Unit->pHero->ipClass1>0))
	{textprintf_centre_ex(objPanel,Font1,33,104,makecol(210,20,250),-1,"%d",Unit->pHero->iLevel);}
	else
	{textprintf_centre_ex(objPanel,Font1,33,104,makecol(210,20,250),-1,"%d",Unit->pUnit->iLevel);};
	for (int i=0; i<3; i++)
	{		
		if (Unit->pUnit->arMedal[i]>0)
		{
			draw_sprite(objPanel,(pFirstMedal+Unit->pUnit->arMedal[i])->SmallPic,45+i*13,102);
		};
	};
	textprintf_centre_ex(objPanel,Font1,115,104,CalcColor(Unit->iSpeed,Unit->GetSpeed()),-1,"%d",Unit->iSpeed);
	textprintf_centre_ex(objPanel,Font1,50,132,CalcColor(Unit->GetAttack(),Unit->pUnit->GetAttack()),-1,"%d",Unit->GetAttack());
	textprintf_centre_ex(objPanel,Font1,111,132,CalcColor(Unit->GetCounterAttack(),Unit->pUnit->GetCounterAttack()),-1,"%d",Unit->GetCounterAttack());
	textprintf_centre_ex(objPanel,Font1,50,132+31,CalcColor(Unit->GetRangedAttack(),Unit->pUnit->GetRangedAttack()),-1,"%d",Unit->GetRangedAttack());
	textprintf_centre_ex(objPanel,Font1,111,132+31,CalcColor(Unit->GetShootingRange(),Unit->pUnit->GetShootingRange()),-1,"%d",Unit->GetShootingRange());
	textprintf_centre_ex(objPanel,Font1,50,132+31*2,CalcColor(Unit->iAmmo,Unit->GetAmmo()),-1,"%d",Unit->iAmmo);
	textprintf_centre_ex(objPanel,Font1,111,132+31*2,CalcColor(Unit->GetResist(),Unit->pUnit->GetResist()),-1,"%d",Unit->GetResist());
	textprintf_centre_ex(objPanel,Font1,50,132+31*3,CalcColor(Unit->GetDefence(),Unit->pUnit->GetDefence()),-1,"%d",Unit->GetDefence());
	textprintf_centre_ex(objPanel,Font1,111,132+31*3,CalcColor(Unit->GetRangedDefence(),Unit->pUnit->GetRangedDefence()),-1,"%d",Unit->GetRangedDefence());	
	if (Unit!=ActiveUnit)
	{
		if ((ActiveUnit)&&(ActiveUnit->G_Obj))
		{
			ActiveUnit->G_Obj->RemoveCargo(objSelect);
		};
		ActiveUnit=Unit;	
		int addX=0, addY=0;
		if (Unit->G_Obj->Pic->w>64)
		{addX=(Unit->G_Obj->Pic->w-64)/2;};
		if (Unit->G_Obj->Pic->h>80)
		{addY=Unit->G_Obj->Pic->h-80;};
		Unit->G_Obj->AddCargo(objSelect,-5+addX,60+addY,1);
	};
};



void SelectUnit(BF_Unit *Unit)
{
//Вывод информационной панели
	ShowUnit(Unit);
	iTarget=-1;
	/*blit(jPanel,objPanel,0,0,0,0,jPanel->w,jPanel->h);
	draw_sprite(objPanel,(pFirstUnitType+Unit->pUnit->ipType)->Pic,7,15);
	textprintf_centre_ex(objPanel,Font1,115,26,CalcColor(Unit->iLife,Unit->GetLife()),-1,"%d",Unit->iLife);
	textprintf_centre_ex(objPanel,Font1,115,52,CalcColor(Unit->iStamina,Unit->GetStamina()),-1,"%d",Unit->iStamina);	
	textprintf_centre_ex(objPanel,Font1,115,78,CalcColor(Unit->iMorale,Unit->GetMorale()),-1,"%d",Unit->iMorale);			
	textprintf_centre_ex(objPanel,Font1,33,104,makecol(210,20,250),-1,"%d",Unit->pUnit->iLevel);
	for (int i=0; i<3; i++)
	{		
		switch (Unit->pUnit->arMedal[i])
		{
			case 0: break;
			case 6: draw_sprite(objPanel,bmpGUI_Medal2,45+i*13,102); break;
			default: draw_sprite(objPanel,bmpGUI_Medal1,45+i*13,102); break;
		};
	};
	textprintf_centre_ex(objPanel,Font1,115,104,CalcColor(Unit->iSpeed,Unit->GetSpeed()),-1,"%d",Unit->iSpeed);
	textprintf_centre_ex(objPanel,Font1,50,132,CalcColor(Unit->GetAttack(),Unit->pUnit->GetAttack()),-1,"%d",Unit->GetAttack());
	textprintf_centre_ex(objPanel,Font1,111,132,CalcColor(Unit->GetCounterAttack(),Unit->pUnit->GetCounterAttack()),-1,"%d",Unit->GetCounterAttack());
	textprintf_centre_ex(objPanel,Font1,50,132+31,CalcColor(Unit->GetRangedAttack(),Unit->pUnit->GetRangedAttack()),-1,"%d",Unit->GetRangedAttack());
	textprintf_centre_ex(objPanel,Font1,111,132+31,CalcColor(Unit->GetShootingRange(),Unit->pUnit->GetShootingRange()),-1,"%d",Unit->GetShootingRange());
	textprintf_centre_ex(objPanel,Font1,50,132+31*2,CalcColor(Unit->iAmmo,Unit->GetAmmo()),-1,"%d",Unit->iAmmo);
	textprintf_centre_ex(objPanel,Font1,111,132+31*2,CalcColor(Unit->GetResist(),Unit->pUnit->GetResist()),-1,"%d",Unit->GetResist());
	textprintf_centre_ex(objPanel,Font1,50,132+31*3,CalcColor(Unit->GetDefence(),Unit->pUnit->GetDefence()),-1,"%d",Unit->GetDefence());
	textprintf_centre_ex(objPanel,Font1,111,132+31*3,CalcColor(Unit->GetRangedDefence(),Unit->pUnit->GetRangedDefence()),-1,"%d",Unit->GetRangedDefence());*/

	TutQue=0;
	if ((Tutorial.Mode)&&(Loss(0)>=0)&&(Loss(1)>=0)&&(iTurn/2<=MAX_BATTLE_DURATION)&&(FearVictory==0))
	{
	if ((Tutorial.Mode)&&(Tutorial.Stamina)&&(Unit->iStamina<5))
	{
		if (bTutHint)
		{TutQue=36;}
		else
		{
			TutMode=36;
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+36)->msg);
			Tutorial.Stamina=0;
		};
	};
	if ((Tutorial.Mode)&&(Tutorial.Ranged)&&(Unit->GetRangedAttack()>0)&&(Unit->iAmmo>0))
	{
		if ((bTutHint)||((TutMode==0)&&((Tutorial.Site==0)||(Tutorial.Site==2))))
		{TutQue=38;}
		else
		{
			TutMode=38;
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+38)->msg);
			Tutorial.Ranged=0;
		};
	};
	if ((Tutorial.Mode)&&(Tutorial.Ammo)&&(Unit->GetAmmo()>0)&&(Unit->iAmmo==0))
	{
		if (bTutHint)
		{TutQue=39;}
		else
		{
			TutMode=39;
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+39)->msg);
			Tutorial.Ammo=0;
		};
	};
	if ((Tutorial.Mode)&&(Tutorial.Orc)&&(Unit->pEffect)&&(Unit->pEffect->pAbility==(pFirstUnitUpg+75)))
	{
		if (bTutHint)
		{TutQue=40;}
		else
		{
			TutMode=40;
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+40)->msg);
			Tutorial.Orc=0;
		};
	};
	if ((Tutorial.Mode)&&(Tutorial.Magic)&&(Unit->pHero)&&(Unit->pHero->arSpell[0]+Unit->pHero->arSpell[1]+Unit->pHero->arSpell[2]>0))
	{
		if ((bTutHint)||((TutMode==0)&&((Tutorial.Site==0)||(Tutorial.Site==2))))
		{TutQue=41;}
		else
		{
			TutMode=41;
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+41)->msg);
			Tutorial.Magic=0;
		};
	};
	if ((Tutorial.Mode)&&(Tutorial.Morale)&&(Unit->iMorale<5))
	{
		if (bTutHint)
		{TutQue=42;}
		else
		{
			TutMode=42;
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+42)->msg);
			Tutorial.Morale=0;
		};
	};
	if ((Tutorial.Mode)&&(Tutorial.Heal)&&(Unit->CheckSkill(24))&&(Unit->iAmmo>0))
	{
		if (bTutHint)
		{TutQue=72;}
		else
		{
			Tutorial.Heal=0;
			TutMode=72;
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+72)->msg);
			Tutorial.Ranged=0;
		};
	};
	};
	textprintf_centre_ex(objPanel,Font1,71,482,COLOR_1,-1,"%s:",(pFirstPhrase+252)->str);
	int dX=13, dY=498, dW=62, dH=68;
	BF_Effect *Eff=Unit->pEffect;
	int Spellz=0;
	for (int i=0; i<6; i++)
	{arEff[i]=NULL;};
	while ((Eff)&&(Spellz<6))
	{
		if (Eff->bVisible)
		{
			if (Eff->pSpell)
			{draw_sprite(objPanel,Eff->pSpell->Pic,dX+dW*(Spellz%2),dY+dH*(Spellz-Spellz%2)/2);};
			if (Eff->pAbility)
			{draw_sprite(objPanel,Eff->pAbility->Pic,dX+dW*(Spellz%2),dY+dH*((Spellz-Spellz%2)/2));};
			if (Eff->iTime>0)
			{
				textprintf_ex(objPanel,Font1,dX+24+dW*(Spellz%2),dY+54+dH*(Spellz-Spellz%2)/2,makecol(0,0,0),-1,"%d",Eff->iTime);
				textprintf_ex(objPanel,Font1,dX+23+dW*(Spellz%2),dY+53+dH*(Spellz-Spellz%2)/2,COLOR_2,-1,"%d",Eff->iTime);
			};
			arEff[Spellz]=Eff;
			mskEff[Spellz].Activate(1);
			Spellz++;
		};
		Eff=Eff->pNext;
	};

	for (int i=0; i<8; i++){
		for (int j=0; j<8; j++)
		{
			Field[i][j].Land=0;
			Field[i][j].Pass=0;
			Field[i][j].Shoot=0;
		};
	};

//Построение карты дальности выстрела
	HideTargets();

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
		if ((Unit->bCanAct)&&(Unit->GetRangedAttack()>0)&&(Unit->iSpeed>0))
		{
			for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
			{
				if ((Squad[1-AA][i].bInBattle)&&(Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0)&&(Field[Squad[1-AA][i].x][Squad[1-AA][i].y].Shoot>1))
				{objTarget[Squad[1-AA][i].x][Squad[1-AA][i].y]->iLayer=1;};
			};
		};
	};



//Построение карты поиска пути
	bFirstMove=(Unit->iSpeed>=Unit->GetSpeed());	
	if ((!Unit->bMarch)&&(Unit->iSpeed<=Unit->GetSpeed()))
	{bFirstMove=false;};
	
	for (int i=0; i<3; i++)
	{CanPass[i]=0;};

	if (Unit->CheckSkill(32))
	{CanPass[0]=2;};

	if (Unit->CheckSkill(33))
	{CanPass[1]=3;};

	if (Unit->CheckSkill(34))
	{CanPass[2]=4;};



	if ((Unit->iSpeed>0)&&(Unit->iStamina>0))
	{
		if (Unit->CheckSkill(14)) //Летающий
		{
			FlyMap(Unit->x,Unit->y,-1,Unit->iSpeed,true);
		}
		else
		{
			if (Unit->CheckSkill(15)) //Низколетающий
			{
				FlyMap(Unit->x,Unit->y,-1,Unit->iSpeed,false);
			}
			else
			{				
				StepMap(Unit->x,Unit->y,-1,Unit->iSpeed,0,Unit->iStamina-Unit->GetFatigure());
			};
		};
	};

	//if (Unit!=ActiveUnit)
	//{
	//	if (ActiveUnit)
	//	{
	//		ActiveUnit->G_Obj->RemoveCargo(objSelect);
	//	};
	//	ActiveUnit=Unit;
	//	int addX=0, addY=0;
	//	if (Unit->G_Obj->Pic->w>64)
	//	{addX=(Unit->G_Obj->Pic->w-64)/2;};
	//	if (Unit->G_Obj->Pic->h>80)
	//	{addY=Unit->G_Obj->Pic->h-80;};
	//	Unit->G_Obj->AddCargo(objSelect,-5-addX,60+addY,1);
	//};


//***** Инициализация кнопок *****
	
	
//Отдых
	if (Unit->bResting)
	{
		buttRest.Activate(-2);
	}
	else
	{
		if (((Unit->iStamina<Unit->GetStamina())
			||((Unit->CheckSkill(22))&&(Unit->iAmmo<Unit->GetAmmo()))
			||((Unit->CheckSkill(35))&&(Unit->iLife<Unit->GetLife()))
			)&&((Unit->iSpeed>=Unit->GetSpeed())||((Unit->bCanAct)&&(Unit->CheckSkill(88))))&&(!Unit->CheckSkill(18))&&
			((Unit->bMarch)||(Unit->CheckSkill(29)==0)))
		{
			buttRest.Activate(1);
		}
		else
		{
			buttRest.Activate(0);
		};
	};

//Следующий юнит
	if (FindNextUnit())
	{
		buttNext.Activate(1);
	}
	else
	{
		buttNext.Activate(0);
	};

	int ButtUsed=2;
	dX=1024-jPanel->w+14, dY=247, dW=62, dH=56;

//Магия
	bool bMagic=false;
	if (((Unit->pHero)&&(Unit->pHero->ipClass1>0))||(Unit->CheckCaster()))
	{bMagic=true;};
	
	if (bMagic)
	{
		ButtUsed++;
		if ((Unit->bCanAct)&&((Unit->iSpeed>0)||(Unit->CheckSkill(88)))&&(Unit->iStamina>0))
		{buttSpell.Activate(1);}
		else
		{buttSpell.Activate(0);}
	}
	else
	{buttSpell.Activate(-1);};

//Смена оружия
	if ((Unit->pHero)&&(Unit->pHero->ipClass1>0))
	{
		ButtUsed++;
		if ((bFastDraw)&&(Unit->bCanAct)&&(Unit->iSpeed>0))
		{buttChange.Activate(1);}
		else
		{buttChange.Activate(0);};
	}
	else
	{buttChange.Activate(-1);};

//Лечение
	if (Unit->CheckSkill(24))
	{
		buttHeal.Replace(dX+(ButtUsed%2)*dW,dY+((ButtUsed-ButtUsed%2)/2*dH));
		if ((Unit->bCanAct)&&(Unit->iAmmo>0)&&(Unit->iSpeed>0))
		{buttHeal.Activate(1, 0);}
		else
		{buttHeal.Activate(0, 0);};
		ButtUsed++;
	}
	else
	{buttHeal.Activate(-1, 0);};


//Сбор снарядов
	if (Unit->CheckSkill(23))
	{
		buttGather.Replace(dX+(ButtUsed%2)*dW,dY+((ButtUsed-ButtUsed%2)/2*dH));
		if ((Unit->bCanAct)&&(Unit->iAmmo<Unit->GetAmmo())&&(Unit->iSpeed>0))
		{buttGather.Activate(1);}
		else
		{buttGather.Activate(0);};
		ButtUsed++;
	}
	else
	{
		buttGather.Activate(-1);
	};

//Марш-бросок
	if (Unit->CheckSkill(29))
	{
		buttMarch.Replace(dX+(ButtUsed%2)*dW,dY+((ButtUsed-ButtUsed%2)/2*dH));
		if ((Unit->bMarch)&&(Unit->iStamina>Unit->CheckSkill(29))&&(Unit->bCanAct))
			{buttMarch.Activate(1);}
		else
			{buttMarch.Activate(0);}
		ButtUsed++;
	}
	else		
	{
		buttMarch.Activate(-1);
	};

//Трупоедство
	if (Unit->CheckSkill(49))
	{
		buttEat.Replace(dX+(ButtUsed%2)*dW,dY+((ButtUsed-ButtUsed%2)/2*dH));
		if (((FindCorpse(0,Unit->x,Unit->y)>=0)||(FindCorpse(1,Unit->x,Unit->y)>=0))
			&&(Unit->bCanAct)&&(Unit->iSpeed>0)&&(Unit->iLife<Unit->GetLife()))
		{buttEat.Activate(1);}
		else
		{buttEat.Activate(0);}
		ButtUsed++;
	}
	else		
	{
		buttEat.Activate(-1);
	};

//Сокрушающий удар
	if (Unit->CheckSkill(59))
	{
		buttCrush.Replace(dX+(ButtUsed%2)*dW,dY+((ButtUsed-ButtUsed%2)/2*dH));
		if ((Unit->bCanAct)&&(Unit->iStamina>Unit->CheckSkill(59))&&(Unit->iSpeed>0))
		{buttCrush.Activate(1, 0);}
		else
		{buttCrush.Activate(0, 0);};
		ButtUsed++;
	}
	else
	{buttCrush.Activate(-1, 0);};

//Круговая атака удар
	if (Unit->CheckSkill(66))
	{
		buttRoundAtt.Replace(dX+(ButtUsed%2)*dW,dY+((ButtUsed-ButtUsed%2)/2*dH));
		if ((Unit->bCanAct)&&(Unit->iStamina>Unit->CheckSkill(66))&&(Unit->iSpeed>0))
		{buttRoundAtt.Activate(1, 0);}
		else
		{buttRoundAtt.Activate(0, 0);};
		ButtUsed++;
	}
	else
	{buttRoundAtt.Activate(-1, 0);};

//Двойной выстрел
	if (Unit->CheckSkill(20))
	{
		buttDoubleShot.Replace(dX+(ButtUsed%2)*dW,dY+((ButtUsed-ButtUsed%2)/2*dH));
		if ((Unit->bCanAct)&&(Unit->iStamina>Unit->CheckSkill(20))&&(Unit->iSpeed>=Unit->GetSpeed())&&(Unit->iAmmo>=2))
		{buttDoubleShot.Activate(1, 0);}
		else
		{buttDoubleShot.Activate(0, 0);};
		ButtUsed++;
	}
	else
	{buttDoubleShot.Activate(-1, 0);};

//	buttUnitBar.Activate(1,buttUnitBar.bPress);
	buttMenu.Activate(1);

	//Подсказка о поломке оружмя
	if ((Tutorial.Mode)&&(!bTutHint)&&(Unit->pHero)&&(Unit->bCanAct)&&(Unit->pHero->ipClass1>0)&&(Tutorial.Broken==1)&&(
	((Unit->pHero->arInventory[1][0]>0)&&(Unit->pHero->arInventory[1][1]==0))||
	((Unit->pHero->arInventory[2][0]>0)&&(Unit->pHero->arInventory[2][1]==0)) ) )
	{
		bTutHint=true;
		ScrollSound();
		objTutorial->iLayer=5;
		txtTutorial.Activate((pFirstTutText+115)->msg);
		Tutorial.Broken=0;
		buttChange.bBlink=true;
	};

	
};


//***********************************************************
//*****  Конец хода  ****************************************
//***********************************************************

bool EndTurn(bool bNext, bool bAutoEnd)
{
	if (bShowBattle)
	{
		HideTargets();
		if ((bAutoCombat)&&(IsPlayed[AA]))
		{
			objLowString->iLayer=4;
			HideButtons();
		};
	};
	
	bHint=false;
	buttEndTurn.bBlink=false;
	if (bTutHint)
	{
		bTutHint=false;
		objTutorial->iLayer=-1;
		txtTutorial.Hide();
		bEndWarning=false;
	};
	if ((Loss(0)<0)||(Loss(1)<0))
	{return 0;};
	if ((IsPlayed[AA])&&(bShowBattle)&&(!bAutoCombat))
	{
		FearVictory=1;
		for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if ((Squad[1-AA][i].bInBattle)&&(Squad[1-AA][i].iLife>0)&&(Squad[1-AA][i].iMorale>0))
			{FearVictory=0;};
		};
		if ((Squad[1-AA][0].bInBattle)&&(Squad[1-AA][0].iLife>0)&&(Heroes[1-AA]->ipClass1>0))
		{FearVictory=0;};
		if (FearVictory>0)
		{
			if (!bInPanic)
			{
				bInPanic=true;
				FearVictory=(pFirstDialog+1185)->Show(1,1,0,0,NULL,NULL)-1;				
				if (FearVictory>0)
				{return 0;}
				else
				{arPlayer[BF_Player[AA]].iKarma-=Random(2);};
			}
			else
			{FearVictory=0;}
		}
		else
		{bInPanic=false;}

	};
//	ActiveUnit->G_Obj->RemoveCargo(objSelect);
	if (bNext)
	{
//		if (!bShowBattle)
//		{textprintf(screen,Font1,10,10,COLOR_2,"%d     ",iTurn);};
	AA=1-AA;
	bFastDraw=true;
	if (IsPlayed[AA])
	{
		ActivePlayer=BF_Player[AA];
		if (arPlayer[BF_Player[1-AA]].Control==1)
		{
			ShowGrid(0,7);
			for (int A=0; A<2; A++)
			{
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{
					Squad[A][i].DrawBar();
				};
			};
		};
	};
	
	if ((!IsPlayed[AA])||(bAutoCombat))
	{
		if (bShowBattle)
		{			
			HideTargets();
			HideButtons();
			blit(bmpEmptyPanel,objPanel,0,0,0,0,bmpEmptyPanel->w,bmpEmptyPanel->h);
			int Delay=500;	
			bTrueAnimation=true;
			while ((Delay>0)&&(bAutoCombat)&&(bTrueAnimation))
			{
				if (Timer)
				{
					BF_RefreshScreen();
					Delay--;			
				};
			};
			Delay=500;
			while((bTrueAnimation)&&(bAutoEnd)&&(Delay>0))
			{
				if (Timer)
				{
					BF_RefreshScreen();
					Delay--;
				};
			};
		};
	};
	int PanicDelay=0;
	if (Heroes[AA]->CheckHeroBonus(984))
	{bArchmage=true;}
	else
	{bArchmage=false;}
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		int iChangeSide=-1;
		if ((Squad[AA][i].iLife>0)&&(Squad[AA][i].bInBattle))
		{			
			Squad[AA][i].iSurvived++;
	//		Squad[AA][i].bWait=false;
			if (Squad[AA][i].CheckSkill(53)>0)
			{
				Squad[AA][i].bParry=true;
			};

			int Hit=0;

			//Отдых
			if ((Squad[AA][i].bResting)||((Squad[AA][i].bCanAct)&&(Squad[AA][i].iSpeed==Squad[AA][i].GetSpeed())))
			{
				Squad[AA][i].iStamina+=(2+Squad[AA][i].CheckSkill(21));
				if (Squad[AA][i].iStamina>Squad[AA][i].GetStamina())
				{Squad[AA][i].iStamina=Squad[AA][i].GetStamina();};
				Squad[AA][i].iAmmo+=Squad[AA][i].CheckSkill(22);
				if (Squad[AA][i].iAmmo>Squad[AA][i].GetAmmo())
				{Squad[AA][i].iAmmo=Squad[AA][i].GetAmmo();};
				Hit+=Squad[AA][i].CheckSkill(35);
				/*Squad[AA][i].iLife+=Squad[AA][i].CheckSkill(35);
				if (Squad[AA][i].iLife>Squad[AA][i].GetLife())
				{Squad[AA][i].iLife=Squad[AA][i].GetLife();};*/
				Squad[AA][i].bResting=false;
			};
			BF_Effect * Eff=Squad[AA][i].pEffect;
			BF_Effect * Eff1;
			

			

			//Таймеры эффектов
			while (Eff)
			{	
				if (Eff->iType==1)
				{
					Hit+=Eff->iPower;
				};
				if ((Eff->iType==11)&&(Squad[AA][i].CheckSkill(18)<1))
				{
					Squad[AA][i].iStamina+=Eff->iPower;
					if (Squad[AA][i].iStamina<0)
					{Squad[AA][i].iStamina=0;};
					if (Squad[AA][i].iStamina>Squad[AA][i].GetStamina())
					{Squad[AA][i].iStamina=Squad[AA][i].GetStamina();};
				};
				Eff1=Eff->pNext;
				if (Eff->iTime>0)
				{
					Eff->iTime--;
					if (Eff->iTime==0)
					{
						if (Eff->iType==73)
						{
							iChangeSide=Eff->iPower;
						};
						if (Eff->iType==90)
						{	
							Squad[AA][i].iLife=Squad[AA][i].iLife*Squad[AA][i].pTrueForm->GetLife()/Squad[AA][i].pUnit->GetLife();
							if (Squad[AA][i].iLife<=0)
							{Squad[AA][i].iLife=1;};
							delete Squad[AA][i].pUnit;
							Squad[AA][i].pUnit=Squad[AA][i].pTrueForm;
							if (Squad[AA][i].iLife>Squad[AA][i].GetLife())
							{Squad[AA][i].iLife=Squad[AA][i].GetLife();};
							if (Squad[AA][i].iAmmo>Squad[AA][i].GetAmmo())
							{Squad[AA][i].iAmmo=Squad[AA][i].GetAmmo();};
							if (Squad[AA][i].iSpeed>Squad[AA][i].GetSpeed())
							{Squad[AA][i].iSpeed=Squad[AA][i].GetSpeed();};
							Squad[AA][i].G_Obj->MoveCargo(Squad[AA][i].objBar,-Squad[AA][i].G_Obj->Pic->w/2+Squad[AA][i].pTrueForm->pType()->Pic->w/2,-Squad[AA][i].G_Obj->Pic->h+Squad[AA][i].pTrueForm->pType()->Pic->h);
							Squad[AA][i].G_Obj->Pic=Squad[AA][i].pUnit->pType()->Pic;
							Squad[AA][i].G_Obj->Shadow=Squad[AA][i].pUnit->pType()->Shadow;
							Squad[AA][i].G_Obj->FlipShadow=Squad[AA][i].pUnit->pType()->FlipShadow;
							Squad[AA][i].G_Obj->Make_Stars(Squad[AA][i].G_Obj,50,40,10,0,44,40,10,200,200);
						};
						if (Eff->pPrev)
						{
							delete Eff;
						}
						else
						{
							Squad[AA][i].pEffect=Eff->pNext;
							delete Eff;							
						};
					};
				};
				Eff=Eff1;				
			};
			//Исчезновение визуальных эффектов
			if ((Squad[AA][i].G_Obj->iWeb>0)||(Squad[AA][i].G_Obj->iRoot>0)||(Squad[AA][i].G_Obj->iIce>0)||(Squad[AA][i].G_Obj->bmpStone))
			{
				bool bNoWeb=true, bNoRoot=true, bNoIce=true, bNoStone=true;
				Eff=Squad[AA][i].pEffect;
				while (Eff)
				{
					if ((Eff->pAbility==(pFirstUnitUpg+77))||(Eff->pSpell==(pFirstSpell+18)))
					{
						bNoWeb=false;						
					};
					if (Eff->pAbility==(pFirstUnitUpg+109))
					{
						bNoRoot=false;
					};
					if (Eff->pSpell==(pFirstSpell+40))
					{
						bNoIce=false;						
					};
					if (Eff->pAbility==(pFirstUnitUpg+76))
					{
						bNoStone=false;
					};
					Eff=Eff->pNext;
				};
				if ((bNoWeb)&&(Squad[AA][i].G_Obj->iWeb>0))
				{
					Squad[AA][i].G_Obj->iWeb=0;
					Squad[AA][i].G_Obj->Make_Stars(Squad[AA][i].G_Obj,30,50,10,0,44,40,200,200,200);
				};
				if ((bNoRoot)&&(Squad[AA][i].G_Obj->iRoot>0))
				{
					Squad[AA][i].G_Obj->iRoot=0;
					Squad[AA][i].G_Obj->Make_Stars(Squad[AA][i].G_Obj,30,50,10,0,44,40,150,200,50);
				};
				if ((bNoIce)&&(Squad[AA][i].G_Obj->iIce>0))
				{
					Squad[AA][i].G_Obj->iIce=0;
					Squad[AA][i].G_Obj->Make_Stars(Squad[AA][i].G_Obj,30,50,10,0,44,40,0,200,250);
				};
				if ((bNoStone)&&(Squad[AA][i].G_Obj->bmpStone))
				{
					Squad[AA][i].G_Obj->iStone=0;
					destroy_bitmap(Squad[AA][i].G_Obj->bmpStone);
					Squad[AA][i].G_Obj->bmpStone=NULL;
					Squad[AA][i].G_Obj->Make_Stars(Squad[AA][i].G_Obj,30,50,10,0,44,40,200,200,200);
				};
			};

			Hit+=Squad[AA][i].CheckSkill(48);

			//Яд/регенерация
			if (Hit+Squad[AA][i].iLife<0)
			{
				Hit=-Squad[AA][i].iLife;
			};
			if ((Hit+Squad[AA][i].iLife>Squad[AA][i].GetLife())&&(Hit>0))
			{
				Hit=Squad[AA][i].GetLife()-Squad[AA][i].iLife;
			};
			bool bAlive=true;
			if (Hit<0)
			{
				int iType=Squad[AA][i].pUnit->GetUnitType();
				Squad[AA][i].G_Obj->AnimNumber(abs(Hit), makecol(200,50,40));
				G_Object * objDeath=Squad[AA][i].G_Obj;
				if (Squad[AA][i].TakeDamage(abs(Hit),3))
				{
				//	textprintf(screen,Font1,0,0,COLOR_2,"Unit: %s",(pFirstUnitType+Squad[AA][i].pUnit->ipType)->sName);
					objDeath->AnimDeath((pFirstUnitType+iType)->wavDeath);
					bAlive=false;
				};				
			};

			if (bAlive)
			{
			
			if (Hit>0)
			{
				Squad[AA][i].iLife+=Hit;
				Squad[AA][i].G_Obj->AnimNumber(Hit, makecol(60,230,20));
			};

			if ((Squad[AA][i].iLife>0)&&(!Squad[AA][i].CheckSkill(36)))
			{
				Squad[AA][i].bCanAct=true;
				if (!Squad[AA][i].CheckSkill(88))
				{
					Squad[AA][i].iSpeed=Squad[AA][i].GetSpeed();							
					Squad[AA][i].bMarch=true;
				}
				else
				{Squad[AA][i].bMarch=false;};
				if (Squad[AA][i].iStamina<1)
				{
					Squad[AA][i].bCanAct=false;
					Squad[AA][i].bResting=true;
					Squad[AA][i].iSpeed=0;
				};
				if (Squad[AA][i].iMorale<=0)
				{
					if (Squad[AA][i].iFear<50)
					{
						if (Random(100)>Squad[AA][i].iFear)
						{
							Squad[AA][i].ChangeMorale(1);
							Squad[AA][i].iFear=0;
						}
						else
						{
							Squad[AA][i].iFear-=10;
							Squad[AA][i].Panic();
							PanicDelay=30;
						};
					}
					else
					{
						Squad[AA][i].iFear-=10;
						Squad[AA][i].Panic();
						PanicDelay=30;
					};				
				};
			};
			Squad[AA][i].DrawBar();
			if ((iChangeSide>=0)&&(Squad[AA][i].iLife>0))
			{
				if (Squad[AA][i].iStamina>0)
				{Squad[AA][i].UseStamina(Squad[AA][i].iStamina/2);}
				if (Squad[AA][i].iMorale>0)
				{Squad[AA][i].ChangeMorale(-Squad[AA][i].iMorale/2);}
				Squad[AA][i].ChangeSide(iChangeSide);
			};
			};
		};		
	};
	while ((PanicDelay>0)&&(bShowBattle)&&((!IsPlayed[AA])||(bAutoCombat)))				
	{
		if (Timer)
		{
			BF_RefreshScreen();
			PanicDelay--;			
		};
	};
	
	iTurn++;
	ActiveUnit=NULL;
	if (iTurn/2>MAX_BATTLE_DURATION)
	{return 0;};
	if (Loss(AA)<0)
	{return 0;};	
	for (int i=0; i<8; i++)
	{
		if ((arGrid[5][i].Land==10)&&(AA==1))
		{
			ClearRoll();
			int Count=0;
			for (int j=0; j<MAX_SQUAD+MAX_SUMMONS; j++)
			{
				if ((Squad[1-AA][j].bInBattle)&&(Squad[1-AA][j].iLife>0))
				{
					RollerN[Count]=j+1;
					if (bfTower.GetRangedAttack()>Squad[1-AA][j].GetRangedDefence())
					{RollerP[Count]=bfTower.GetRangedAttack()-Squad[1-AA][j].GetRangedDefence();}
					else
					{RollerP[Count]=1;};
					Count++;
				};
			};
			if (Count>0)
			{
				int Target=Roll(false,iTurn+1,Count)-1;
				bfTower.x=5;
				bfTower.y=i;
				bfTower.G_Obj->x=TransX(bfTower.x,bfTower.y,bfTower.G_Obj->Pic->w);
				bfTower.G_Obj->y=TransY(bfTower.x,bfTower.y,bfTower.G_Obj->Pic->h);
				bfTower.G_Obj->fX=bfTower.G_Obj->x;
				bfTower.G_Obj->fY=bfTower.G_Obj->y;
				bfTower.iAmmo=bfTower.GetAmmo();
				bfTower.iStamina=bfTower.GetStamina();
				bfTower.Shoot(&Squad[1-AA][Target]);				
				bfTower.G_Obj->x=1025;
				int Delay=40;			
				while ((Delay>0)&&(bShowBattle))
				{
					if (Timer)
					{
						BF_RefreshScreen();
						Delay--;			
					};
				};
				if (Loss(1-AA)<0)
				{return 0;};
			};
		};
	};
	if (AA==1)
	{
		for (int i=0; i<MagTower; i++)
		{			
			ClearRoll();
			int Count=0;
			for (int j=0; j<MAX_SQUAD+MAX_SUMMONS; j++)
			{
				if ((Squad[1-AA][j].bInBattle)&&(Squad[1-AA][j].iLife>0))
				{
					RollerN[Count]=j+1;
					if (bfMagTower[i].GetRangedAttack()>Squad[1-AA][j].GetResist())
					{RollerP[Count]=bfMagTower[i].GetRangedAttack()-Squad[1-AA][j].GetResist();}
					else
					{RollerP[Count]=1;};
					Count++;
				};
			};
			if (Count>0)
			{
				int Target=Roll(false,iTurn+1,Count)-1;
				bfMagTower[i].x=8;
				bfMagTower[i].y=(MagTowerCoord[i]-MagTowerCoord[i]%100)/100;
				bfMagTower[i].G_Obj->x=TransX(bfMagTower[i].x,bfMagTower[i].y,bfMagTower[i].G_Obj->Pic->w);
				bfMagTower[i].G_Obj->y=TransY(bfMagTower[i].x,bfMagTower[i].y,bfMagTower[i].G_Obj->Pic->h);
				bfMagTower[i].G_Obj->fX=bfMagTower[i].G_Obj->x;
				bfMagTower[i].G_Obj->fY=bfMagTower[i].G_Obj->y;
				bfMagTower[i].iAmmo=bfMagTower[i].GetAmmo();
				bfMagTower[i].iStamina=bfMagTower[i].GetStamina();
				bfMagTower[i].Shoot(&Squad[1-AA][Target]);
				bfMagTower[i].G_Obj->x=1025;
				int Delay=40;			
				while ((Delay>0)&&(bShowBattle))
				{
					if (Timer)
					{
						BF_RefreshScreen();
						Delay--;			
					};
				};
				if (Loss(1-AA)<0)
				{return 0;};
			};
		};
	};
	};
	if ((bShowBattle)&&(arPlayer[BF_Player[AA]].Control==1)&&(key[KEY_ESC]))
	{
		bAutoCombat=false;		
	};
	if ((!IsPlayed[AA])||(!bShowBattle)||(bAutoCombat))
	{
		/*if (bShowBattle)
		{			
			HideTargets();
			HideButtons();		
			blit(bmpEmptyPanel,objPanel,0,0,0,0,bmpEmptyPanel->w,bmpEmptyPanel->h);
			int Delay=20;		
			bAnimation=true;
			while ((Delay>0)&&(bAutoCombat)&&(bAnimation))
			{
				if (Timer)
				{
					BF_RefreshScreen();
					Delay--;			
				};
			};
			Delay=500;
			while((bAnimation)&&(bAutoEnd)&&(Delay>0))
			{
				if (Timer)
				{
					BF_RefreshScreen();
					Delay--;
				};
			};
		};*/
		AI_CalcPower();
		AI_CalcDangerZone();
		ActiveUnit=NULL;
		for (int i=0; i<(MAX_SQUAD+MAX_SUMMONS)*2; i++)
		{
			if ((Loss(0)<0)||(Loss(1)<0))
			{
				return 0;
			};			
			int Delay=AI_Main(false)*80;
			bTrueAnimation=true;
			while ((Delay>0)&&(bShowBattle)&&(bTrueAnimation))
			{
				if (Timer)
				{
					if ((key[KEY_ESC])&&(bAutoCombat))
					{
						if (arPlayer[BF_Player[1-AA]].Control==1)
						{
							bAutoCombat=false;
							objLowString->iLayer=-1;
						};
						if (arPlayer[BF_Player[AA]].Control==1)
						{
							bAutoCombat=false;
							if (FindNextUnit())
							{SelectUnit(FindNextUnit());}
							else
							{SelectUnit(&Squad[AA][Loss(AA)]);};					
							Gomunculus=NULL;
							objLowString->iLayer=-1;
							return 1;
						};
					};
					BF_RefreshScreen();
					Delay--;			
				};
			};
		};
		ActiveUnit=NULL;
		IndexAI=-1;
		for (int i=0; (i<(MAX_SQUAD+MAX_SUMMONS)*4)&&(IndexAI!=-100); i++)
		{
			if ((Loss(0)<0)||(Loss(1)<0))
			{
				return 0;
			};
			int Delay=AI_Main(true)*80;
			bTrueAnimation=true;
			while ((Delay>0)&&(bShowBattle)&&(bTrueAnimation))
			{
				if (Timer)
				{
					if ((key[KEY_ESC])&&(bAutoCombat))
					{
						if (arPlayer[BF_Player[1-AA]].Control==1)
						{
							bAutoCombat=false;
							objLowString->iLayer=-1;
						};
						if (arPlayer[BF_Player[AA]].Control==1)
						{
							bAutoCombat=false;
							if (FindNextUnit())
							{SelectUnit(FindNextUnit());}
							else
							{SelectUnit(&Squad[AA][Loss(AA)]);};					
							Gomunculus=NULL;
							objLowString->iLayer=-1;
							return 1;
						};
					};
					BF_RefreshScreen();
					Delay--;
				};
			};			
		};
		if (!EndTurn(true, false))
		{
			return 0;
		};
	}
	else
	{
		if (FindNextUnit())
		{SelectUnit(FindNextUnit());}
		else
		{
			int Select=Loss(AA);
			if (Select>=0)
			{
				SelectUnit(&Squad[AA][Select]);
			}
			else
			{
				return 0;
			};
		};
	};
	objLowString->iLayer=-1;
	Gomunculus=NULL;
	return 1;
};

bool CanAttack(int x, int y) // Возвращает истину, если активный юнит может атаковать с данной клетки
{
	if ((x>=0)&&(x<8)&&(y>=0)&&(y<8)&&ActiveUnit->bCanAct)
	{
		if (((ActiveUnit->x==x)&&(ActiveUnit->y==y))||
			((arGrid[x][y].Pass)&&(Field[x][y].Pass)&&(ActiveUnit->iSpeed>0)&&(ActiveUnit->iStamina)))
		{			
			return 1;		
		};
	};
	return 0;
};


//Стандартные действия после вызова заклинания героем
void SpellCast()
{
	if (ActiveSpell==30000)
	{
		ActiveSpell=-1;
		return;
	};
	txtSpName.Hide();
	txtSpTarget.Hide();
	txtSpDesc.Hide();
	HideTargets();
	if (bShowAnim)
	{
		PlaySpellSound(pCast->wavCast);
	//	play_sample(pCast->wavCast,iSoundVolume,128,1000,0);
	};	
	if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
	{
		arPlayer[BF_Player[AA]].Gem-=pCast->iCost*(100-ActiveUnit->pHero->CheckHeroBonus(983))/100;
		GemSpent[AA]+=pCast->iCost*(100-ActiveUnit->pHero->CheckHeroBonus(983))/100;
		if (!bSimulate)
		{arPlayer[BF_Player[AA]].iKarma+=pCast->iKarma;};
		Spells[AA][1][ActiveSpell]=0;
	}
	else
	{
		ActiveUnit->iAmmo-=ActiveUnit->CheckSkill(2000+ActiveSpell);		
	};
	buttCancelSpell.Activate(-1);
	buttEndTurn.Activate(1);
	ActiveSpell=-1;
	if (ActiveUnit->iLife>0)
	{	
		ActiveUnit->UseStamina(pCast->iStamCost);
		ActiveUnit->iSpellCast+=pCast->iStamCost;
		if (ActiveUnit->iSpeed<ActiveUnit->GetSpeed())
		{ActiveUnit->UseStamina(1);};
		if ((bArchmage)&&(ActiveUnit->pHero)&&(ActiveUnit->iStamina>0)&&(ActiveUnit->iSpeed>1))
		{
			ActiveUnit->iSpeed/=2;
			SelectUnit(ActiveUnit);
		}
		else
		{
			ActiveUnit->bCanAct=false;
			ActiveUnit->iSpeed=0;
		};
		if (pCast->iLifeCost>0)
		{	
			int Hit=pCast->iLifeCost;
			if (Hit>ActiveUnit->iLife)
			{
				Hit=ActiveUnit->iLife;
			};
			int iType=ActiveUnit->pUnit->GetUnitType();
			G_Object *objDeath=ActiveUnit->G_Obj;
			if (ActiveUnit->TakeDamage(Hit,3))
			{
				objDeath->AnimDeath((pFirstUnitType+iType)->wavDeath);
			};
			ActiveUnit->G_Obj->AnimNumber(Hit, makecol(200,50,40));		
		};
	};
	if ((bArchmage)&&(ActiveUnit->pHero))
	{
		bArchmage=false;
	};	
	if (pCast->bSacrifice)
	{
		if (Squad[AA][iSacVictim].iLife>0)
		{
			Squad[AA][iSacVictim].iLife=0;
			Squad[AA][iSacVictim].G_Obj->AnimDeath(Squad[AA][iSacVictim].pUnit->pType()->wavDeath);
			Squad[AA][iSacVictim].Death();			
		};
	};
	ActiveUnit->DrawBar();
	if (!ActiveUnit->bCanAct)
	{
//	WriteLog("Ищем замену");
	if (Gomunculus)
	{
		SelectUnit(Gomunculus);
		Gomunculus=NULL;
	}
	else
	{
		if (FindNextUnit())
		{SelectUnit(FindNextUnit());}
		else	
		{
			if (ActiveUnit->iLife>0)
			{SelectUnit(ActiveUnit);}
			else
			{
				if (Loss(AA)>=0)
				{
					SelectUnit(&Squad[AA][Loss(AA)]);
				};
			};
		};
	};

	};
};


//Вызов заклинания на трупе юнита
void CorpseSpell(clSpell *Spell, BF_Unit *Corpse)
{	
//Вызов	
	if (Spell->arEffect[0][0]>1000)
	{
		for (int i=MAX_SQUAD; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if ((Squad[AA][i].pUnit==NULL)&&(Squad[AA][i].iSurvived==0))
			{
				int iPower=Spell->arEffect[1][0];
				iPower+=ActiveUnit->CheckSkill(907)/2+ActiveUnit->CheckSkill(909);
				/*if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
				{
					iPower+=ActiveUnit->pHero->CheckHeroBonus(907)/2+ActiveUnit->pHero->CheckHeroBonus(909);
				};*/
				objMain=Squad[AA][i].Summon(Spell->arEffect[0][0]%1000,iPower,Corpse->x,Corpse->y,objMain);
				Squad[AA][i].iSide=AA;
				if (AA)
				{
					Squad[AA][i].G_Obj->bFlip=true;
				}
				else
				{
					Squad[AA][i].G_Obj->bFlip=false;
				};
				Gomunculus=&Squad[AA][i];
				Corpse->bInBattle=false;
				if (!bSimulate)
				{
					Corpse->x=10;
					Corpse->y=10;
					Corpse->G_Obj->iLayer=-1;
				};
				break;
			};
		};
	};
//Воскрешение
	if (Spell->arEffect[0][0]==1)
	{
		Gomunculus=Corpse;
		int Power=Spell->arEffect[1][0];
		Power+=ActiveUnit->CheckSkill(905)*Spell->iPowerMod/10;
		/*if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
		{Power+=ActiveUnit->pHero->CheckHeroBonus(905)*Spell->iPowerMod/10;};*/
		if (Power>100)
		{Power=100;};
		Corpse->iLife=Corpse->GetLife()*Power/100;
		if (Corpse->iLife<1)
		{
			Corpse->iLife=1;
		};
		arGrid[Corpse->x][Corpse->y].Pass=0;
		Corpse->iFear=0;	
		Corpse->iSpeed=Corpse->pUnit->GetSpeed();
		Corpse->bCanAct=true;
		Corpse->bResting=false;	
		Corpse->bInBattle=true;		
		Corpse->pEffect=NULL;
		Corpse->bMarch=Corpse->CheckSkill(29);
		Corpse->G_Obj->Pic=Corpse->pUnit->pType()->Pic;
		Corpse->G_Obj->Shadow=Corpse->pUnit->pType()->Shadow;
		Corpse->G_Obj->FlipShadow=Corpse->pUnit->pType()->FlipShadow;
		Corpse->G_Obj->iLayer=2;
		Corpse->G_Obj->pRaise=new G_Raise(Corpse->G_Obj);
		Corpse->G_Obj->Make_Stars(Corpse->G_Obj,80,30,10,20,44,40,250,250,220);
		Corpse->DrawBar();
	};
};

//Вызов заклинания в тайле
void TileSpell(clSpell *Spell, int x, int y)
{
	if (Spell->arEffect[0][0]>1000)
	{
		//Вызов
		for (int i=MAX_SQUAD; i<MAX_SQUAD+MAX_SUMMONS; i++)
		{
			if ((Squad[AA][i].pUnit==NULL)&&(Squad[AA][i].iSurvived==0))
			{				
				int iPower=Spell->arEffect[1][0];
				iPower+=ActiveUnit->CheckSkill(907);
				/*if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
				{
					iPower+=ActiveUnit->pHero->CheckHeroBonus(907);
				};*/
				objMain=Squad[AA][i].Summon(Spell->arEffect[0][0]%1000,iPower,x,y,objMain);
				Squad[AA][i].iSide=AA;
				if (AA)
				{
					Squad[AA][i].G_Obj->bFlip=true;
				}
				else
				{
					Squad[AA][i].G_Obj->bFlip=false;
				};
				Gomunculus=&Squad[AA][i];
				break;
			};
		};
	};
	if (Spell->arEffect[0][0]==89)		
	{
		//Телепорт
		pTeleportUnit->Replace(x,y);
		pTeleportUnit->G_Obj->Make_Stars(pTeleportUnit->G_Obj,50,40,10,0,44,40,10,200,200);
	};
};

void SpellPrepare(int ipSpell)
{
//Построение карты дальности заклинания	
	if ((pCast->iTarget==5)||(pCast->iTarget==3))
	{
		HideTargets();
		for (int i=0; i<8; i++){
		for (int j=0; j<8; j++)
		{
			Field[i][j].Land=0;
			Field[i][j].Pass=0;
			Field[i][j].Shoot=0;
		};
		};
		ShootMap(ActiveUnit->x,ActiveUnit->y,-1,ActiveUnit->CheckSkill(907)/2+1);
		if (pCast->iTarget==5)
		{
			for (int i=0; i<8; i++){
			for (int j=0; j<8; j++)
			{
				if ((Field[i][j].Shoot>0)&&(arGrid[i][j].Pass)&&((pFirstBF_Obj+arGrid[i][j].Land)->iMoveCost>0))
				{
					{objTargetH[i][j]->iLayer=1;};
				};
			};
			};
		};
	}
	else
	{
		for (int i=0; i<8; i++){
		for (int j=0; j<8; j++)
		{			
			Field[i][j].Shoot=1;
		};
		};
	};
	if (pCast->iTarget==0)
	{
//Вызов заклятья без цели (на все поле)
		if (pCast->iArea==2)
		{
			if (pCast->bOnAlly)
			{
				for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
				{
					if ((Squad[AA][k].pUnit)&&(Squad[AA][k].iLife>0))
					{
						int AntiSkill=0;
						for (int i=0; i<MAX_ANTI_EFFECT; i++)
						{
							AntiSkill+=pCast->arAntiEffect[i]*Squad[AA][k].CheckSkill(pCast->arAntiEffect[i]);
						};
						for (int i=0; i<MAX_UNIT_KIND; i++)
						{
							if (pCast->arUnitKind[i]==Squad[AA][k].GetUnitKind())
							{AntiSkill=1;};
						};
						if ((!AntiSkill)||(Squad[AA][k].CheckSkill(205)==ipSpell))
						{
				//Найден свой подходящий юнит
							Squad[AA][k].UnderSpell(pCast, ActiveUnit, AA);
						};
					};
				};
			};
			if (pCast->bOnEnemy)
			{
				for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
				{
					if ((Squad[1-AA][k].pUnit)&&(Squad[1-AA][k].iLife>0))
					{
						int AntiSkill=0;
						for (int i=0; i<MAX_ANTI_EFFECT; i++)
						{
							AntiSkill+=pCast->arAntiEffect[i]*Squad[1-AA][k].CheckSkill(pCast->arAntiEffect[i]);
						};
						for (int i=0; i<MAX_UNIT_KIND; i++)
						{
							if (pCast->arUnitKind[i]==Squad[1-AA][k].GetUnitKind())
							{AntiSkill=1;};
						};
						if ((!AntiSkill)||(Squad[1-AA][k].CheckSkill(205)==ipSpell))
						{
				//Найден вражеский подходящий юнит
							Squad[1-AA][k].UnderSpell(pCast, ActiveUnit, AA);														
						};
					};
				};
			};
		};
//Вызов заклятья на себя
		if (pCast->iArea==0)
		{ActiveUnit->UnderSpell(pCast, ActiveUnit, AA);}

		SpellCast();
//		WriteLog("Вернулись в основной процесс");
	};
};


//*********************************************
//*** Расстановка войск ***********************
//*********************************************

void UnitPlacement (int Side)
{
	int iTarger=-1;
	for (int i=0; i<MAX_SQUAD-1; i++)
	{		
		if ((Heroes[Side]->pSquad[i])&&(Heroes[Side]->pSquad[i]->iLife>0))
		{
			for (int j=i+1; j<MAX_SQUAD; j++)
			{		
				if ((Heroes[Side]->pSquad[j])&&(Heroes[Side]->pSquad[j]->iLife>0)&&(Heroes[Side]->pSquad[j]->x==Heroes[Side]->pSquad[i]->x)&&(Heroes[Side]->pSquad[j]->y==Heroes[Side]->pSquad[i]->y))
				{
					for (int x=0; x<2; x++)
					{
						for (int y=0; y<8; y++)
						{
							bool bFind=true;
							for (int k=0; k<MAX_SQUAD; k++)
							{
								if ((Heroes[Side]->pSquad[k])&&(Heroes[Side]->pSquad[k]->iLife>0)&&(Heroes[Side]->pSquad[k]->x==x)&&(Heroes[Side]->pSquad[k]->y==y))
								{bFind=false;};								
							};
							if (bFind)
							{
								Heroes[Side]->pSquad[j]->x=x;
								Heroes[Side]->pSquad[j]->y=y;
							};
						};
					};
				};
			};

		};
	};
	for (int i=0; i<MAX_SQUAD; i++)
	{		
		if ((Heroes[Side]->pSquad[i])&&(Heroes[Side]->pSquad[i]->iLife>0))
		{
			if (Side)
			{
				Heroes[Side]->pSquad[i]->x=7-Heroes[Side]->pSquad[i]->x;
			};
			objMain=Squad[Side][i].Init(Heroes[Side]->pSquad[i], objMain, Side, i);
			if (i==0)
			{
				Squad[Side][i].pHero=Heroes[Side];
			};
			if (Side)
			{
				objMain->bFlip=true;				
			};
					
		};
	};
	for (int i=0; i<MAX_SQUAD; i++)
	{
		if ((Heroes[Side]->pSquad[i])&&(Heroes[Side]->pSquad[i]->iLife>0))
		{
			Squad[Side][i].iAmmo=Squad[Side][i].GetAmmo();
		};
	};

	if ((arPlayer[BF_Player[Side]].Control==1)&&(bShowBattle)
		//&&(!BF_Cheat)
		)
	{
	ActivePlayer=BF_Player[Side];
	if (Side)
	{ShowGrid(6,7);}
	else
	{
		ShowGrid(0,1);		
	};
	ActiveUnit=NULL;
	blit(bmpEmptyPanel,objPanel,0,0,0,0,bmpEmptyPanel->w,bmpEmptyPanel->h);

	clear_keybuf();

	bool bCursorChange;
	bool bMouseReleased=true;
	bool NeedUse;
	int Check;
	buttRest.Activate(-1);	
	buttNext.Activate(-1);

	int TutMode=0;
	if (Tutorial.Map)
	{
		if (Tutorial.Site==0)
		{
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+28)->msg);
			TutMode=1;
			buttMenu.Activate(0);
			buttStart.Activate(-1);
		};
		if (Tutorial.Site==1)
		{
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+30)->msg);			
		};
	};

	while((!buttStart.Check())&&(!bAutoCombat)&&(IsPlayed[Side]))
	{
		objSideBar->iLayer=-1;
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
				bEndWarning=false;
				if (TutMode==1)
				{
					bTutHint=true;
					ScrollSound();
					objTutorial->iLayer=5;
					txtTutorial.Activate((pFirstTutText+29)->msg);
					TutMode=2;
					buttStart.Activate(1);
					buttStart.bBlink=true;					
				};
			};
		};
		poll_keyboard();
		if ((key[KEY_F10])&&(!IsPlayed[1-Side]))
		{
			if ((key[KEY_LCONTROL])||(key[KEY_RCONTROL]))
			{SideCanCast[Side]=(pFirstDialog+1460)->Show(1,1,1,0,NULL,NULL)-1;};
			NewProgress=0;
			OldProgress=0;
			RunHourGlass(150, 0);
			bAutoCombat=true;
			bShowBattle=false;
			bShowAnim=false;			
		};
		if (key[KEY_F11])
		{
			if ((key[KEY_LCONTROL])||(key[KEY_RCONTROL]))
			{SideCanCast[Side]=(pFirstDialog+1460)->Show(1,1,1,0,NULL,NULL)-1;};
			if (IsPlayed[1-Side])
			{IsPlayed[Side]=false;}
			else
			{
				bAutoCombat=true;
				objLowString->iLayer=4;
			};
		};
		if (buttGrid.Check())
		{
			arHumans[arPlayer[ActivePlayer].iColor].bWarGrid=!arHumans[arPlayer[ActivePlayer].iColor].bWarGrid;
			if (Side)
			{ShowGrid(6,7);}
			else
			{ShowGrid(0,1);};
			arHumans[arPlayer[ActivePlayer].iColor].Save();
		};
		if (buttMenu.Check())
		{
			buttMenu.bBlink=false;
			switch (WarMenu(Side))
			{
			case 1:
				NewProgress=0;
				OldProgress=0;
				RunHourGlass(150, 0);
				bAutoCombat=true;
				bShowBattle=false;
				bShowAnim=false;
				break;
			case 2:
				if (IsPlayed[1-Side])
				{IsPlayed[Side]=false;}
				else
				{
					bAutoCombat=true;
					objLowString->iLayer=4;
				};
				break;
			case 3:
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{Squad[Side][i].iLife=0;};
				bAutoCombat=true;
				break;
			};
			
			if (Side)
			{ShowGrid(6,7);}
			else
			{ShowGrid(0,1);};
			for (int A=0; A<2; A++)
			{
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{
					Squad[A][i].DrawBar();
				};
			};
		};
		if (ActiveUnit)
		{
			for (int i=0; i<13; i++)
		{
			Check=mskParam[i].Check();
			if ((Check==30)||(Check==40))
			{
				switch (i)
				{
				case 0:
					if (ActiveUnit->pUnit->pHero)
					{WindowAbilityInfo(119,ActiveUnit->pUnit->pHero->iLevel);}
					else
					{WindowAbilityInfo(119,ActiveUnit->pUnit->iLevel);};
					break;
				case 1:
					WindowAbilityInfo(i,ActiveUnit->iLife);
					break;
				case 2:
					WindowAbilityInfo(i,ActiveUnit->GetAttack());
					break;
				case 3:
					WindowAbilityInfo(i,ActiveUnit->GetCounterAttack());
					break;
				case 4:
					WindowAbilityInfo(i,ActiveUnit->GetDefence());
					break;
				case 5:
					WindowAbilityInfo(i,ActiveUnit->GetRangedDefence());
					break;
				case 6:
					WindowAbilityInfo(i,ActiveUnit->GetResist());
					break;
				case 7:
					WindowAbilityInfo(i,ActiveUnit->iSpeed);
					break;
				case 8:
					WindowAbilityInfo(i,ActiveUnit->GetRangedAttack());
					break;
				case 9:
					WindowAbilityInfo(i,ActiveUnit->GetShootingRange());
					break;
				case 10:
					WindowAbilityInfo(i,ActiveUnit->iAmmo);
					break;
				case 11:
					WindowAbilityInfo(i,ActiveUnit->iStamina);
					break;
				case 12:
					WindowAbilityInfo(i,ActiveUnit->iMorale);
					break;
				};
			}
			else
			{
				if (Check>1)
				{
					if (i>0)
					{ActiveUnit->MakeParamHint(i);}
					else
					{MakeAbilityHint(119);};
					bHint=true;
				};
			};
		};
		};
	//	bool bView=buttUnitBar.bPress;
	//	buttUnitBar.Check();
		if (key[KEY_ALT]!=bAltFlag)
		{
			bAltFlag=key[KEY_ALT];
			for (int A=0; A<2; A++)
			{
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{
					Squad[A][i].DrawBar();
				};
			};
		};
		bCursorChange=true;		
		if ((!bMouseReleased)&&(!mouse_b & 1))
		{bMouseReleased=true;};
		if ((!bTutHint)||(mskTutorial.flag<2))
		{
		for (int i=0+Side*6; i<2+Side*6; i++)
		{
		for (int j=0; j<8; j++)
		{	
			Check=Grid[i][j].Check();
			switch (Check)
			{
case 30:	//Клик по тайлу					
			if (bMouseReleased)
			{
				NeedUse=true;
				for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
				{
					if ((Squad[Side][k].iLife)&&(Squad[Side][k].x==i)&&(Squad[Side][k].y==j))
					{		
						if (ActiveUnit)
		//Перемещение юнита в занятый тайл
						{
							Squad[Side][k].Replace(ActiveUnit->x,ActiveUnit->y);
							ActiveUnit->Replace(i,j);
							ActiveUnit->G_Obj->RemoveCargo(objSelect);
							ActiveUnit=NULL;
							blit(bmpEmptyPanel,objPanel,0,0,0,0,bmpEmptyPanel->w,bmpEmptyPanel->h);							
							break;
						}					
						else
		//Выбор юнита
						{
							ShowUnit(&Squad[Side][k]);							
						};
						NeedUse=false;
					};
				};
				if ((ActiveUnit)&&(NeedUse))
				{
		//Перемещение юнита в свободный тайл
					ActiveUnit->Replace(i,j);
					ActiveUnit->G_Obj->RemoveCargo(objSelect);
					ActiveUnit=NULL;
					blit(bmpEmptyPanel,objPanel,0,0,0,0,bmpEmptyPanel->w,bmpEmptyPanel->h);
				};
				bMouseReleased=false;
			};
			break;
	case 40:
		//Инфо-клик
		for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
		{
			if ((Squad[Side][k].pUnit)&&(Squad[Side][k].iLife>0)&&(Squad[Side][k].x==i)&&(Squad[Side][k].y==j))
			{
				objMain=ShowActiveObjects(objMain,10,false);
				Squad[Side][k].ShowInfoScreen();				
			};
			if ((Squad[1-Side][k].pUnit)&&(Squad[1-Side][k].iLife>0)&&(Squad[1-Side][k].x==i)&&(Squad[1-Side][k].y==j))
			{
				objMain=ShowActiveObjects(objMain,10,false);
				Squad[1-Side][k].ShowInfoScreen();				
			};
		};
		break;
	case 2:					
	case 3:			//Курсор над тайлом
		for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
		{
			if (((key[KEY_ALT])||(key[KEY_ALTGR]))&&(Squad[Side][k].iLife)&&(Squad[Side][k].x==i)&&(Squad[Side][k].y==j)&&(ActiveUnit!=&Squad[Side][k]))
			{
				if (k!=iTarget)
				{
					ShowEnemy(&Squad[Side][k]);
					iTarget=k;
				};
				objSideBar->iLayer=4;
			};
		}
			NeedUse=true;
			for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
			{
				if ((Squad[Side][k].iLife)&&(Squad[Side][k].x==i)&&(Squad[Side][k].y==j))
				{
					if(mouse_pointer!=1)
					{
						set_mouse_sprite(bmpMousePointer[1]);
						mouse_pointer=1;
					};		
					bCursorChange=false;					
					NeedUse=false;
				};
			};
			if ((ActiveUnit)&&NeedUse)
			{
				if(mouse_pointer!=2)
				{
					set_mouse_sprite(bmpMousePointer[2]);
					mouse_pointer=2;
				};
				bCursorChange=false;
			};
			break;							
			};
		};
	};
	}; //Tutorial
	if (bCursorChange&&(mouse_pointer>0))
	{
		set_mouse_sprite(bmpMousePointer[0]);
		mouse_pointer=0;
	};
	if ((!bEndWarning)&&(bTutHint)&&(!Tutorial.Mode))
	{
		objTutorial->iLayer=-1;
		txtTutorial.Hide();
		bTutHint=false;
	};
//	if (!Tutorial.Mode)
//	{objTutorial->iLayer=-1;};	
	if ((Timer)&&(bShowBattle))
	{
		Timer=false;
		objMain=ShowActiveObjects(objMain,10,true);	
	};
	bHint=false;
//		BF_RefreshScreen();
//		textprintf_ex(screen,Font1,0,0,makecol(220,220,50),-1,"%d",Squad[1][0].iLife);	
	};	
	if (ActiveUnit)
	{
		ActiveUnit->G_Obj->RemoveCargo(objSelect);
		ActiveUnit=NULL;
	};
	if (Side)
	{
		for (int i=0; i<MAX_SQUAD; i++)
		{
			if ((Heroes[Side]->pSquad[i])&&(Heroes[Side]->pSquad[i]->iLife>0))
			{
				Heroes[Side]->pSquad[i]->x=7-Squad[Side][i].x;
				Heroes[Side]->pSquad[i]->y=Squad[Side][i].y;
				arGrid[Squad[Side][i].x][Squad[Side][i].y].Pass=0;
			};
		};
	}
	else
	{
		for (int i=0; i<MAX_SQUAD; i++)
		{
			if ((Heroes[Side]->pSquad[i])&&(Heroes[Side]->pSquad[i]->iLife>0))
			{
				Heroes[Side]->pSquad[i]->x=Squad[Side][i].x;
				Heroes[Side]->pSquad[i]->y=Squad[Side][i].y;
				arGrid[Squad[Side][i].x][Squad[Side][i].y].Pass=0;
			};
		};
	};
	}
	else		
	{

//Расстановка войск АИ
		bool arPlace[MAX_SQUAD];
		int sqNN=0, sqNN2=0, Side2= (Side>0?0:1), sqConst=0;//new
		for (int i=0; i<MAX_SQUAD; i++)
		{
			if ((Heroes[Side]->pSquad[i])&&(Heroes[Side]->pSquad[i]->iLife>0))
				{ arPlace[i]=true; sqNN++;/*new*/}	else {arPlace[i]=false; };
			if ((Heroes[Side2]->pSquad[i])&&(Heroes[Side2]->pSquad[i]->iLife>0)) { sqNN2++; sqConst=Heroes[Side2]->pSquad[i]->iLife; }; //new
			
		};
		int Rx[2];
		if (Side) {
			Rx[0]=7; Rx[1]=6;} else {Rx[0]=0; Rx[1]=1; };
		int Choosen=-1, Range=3;
		int maxComID= 5; //new
		int ComSqdID= Random(100+sqConst,sqNN,sqNN2,Side)%maxComID;	//new ={ no=0, guard=1, veteran=2, division=3, hero platoon=4 }
		int Position= Random(100+sqConst,sqNN,sqNN2,Side)%3; //0;	//new ={ up=0, center=1, down=2 }
		int Pos0= 0, Pos2= 7, dPos= 1; //up new
		if( Position>0 ) {//new
			if( Position==1 ) {
				Pos0= (MAX_SQUAD-sqNN)/4; Pos2= Pos0+sqNN/2 +1; dPos= 1; //center    /*+sqNN%2*/
			}else{
				Pos0= 7; Pos2= 0; dPos= -1;	//down 
			}
		}
		if(Pos2>7) Pos2=7;//very new

		// test
		//Pos0= 0, Pos2= 7, dPos= 1; //up new
		//

		#define MODDD(x,y) (x-y>0?x-y:y-x)
		//Расстановка стрелков
		while (Range>2)
		{
			Range=2; Choosen=-1;
			for (int i=0; i<MAX_SQUAD; i++)
			{
				if ((arPlace[i])&&(Heroes[Side]->pSquad[i]->GetShootingRange()>Range))
				{
					Range=Heroes[Side]->pSquad[i]->GetShootingRange();
					Choosen=i;
				};
			};

			if (Choosen>=0)
			{
				//На холмах
				for (int x=0; x<2; x++)
				{
					for (int y=Pos0; MODDD((Pos2+dPos),y) > 0 ; y+=dPos)//new
					//for (int y=0; y<8; y++)
					{
						if ((arGrid[Rx[x]][y].Land==3)&&(arPlace[Choosen]))
						{
							bool bPut=true;
							for (int j=0; j<MAX_SQUAD; j++)
							{								
							if((Heroes[Side]->pSquad[j])&&(Heroes[Side]->pSquad[j]->iLife>0)&&(!arPlace[j])&&(Heroes[Side]->pSquad[j]->x==x)&&(Heroes[Side]->pSquad[j]->y==y))
								{bPut=false;};
							};
							if (bPut)
							{
								Heroes[Side]->pSquad[Choosen]->x=x;
								Heroes[Side]->pSquad[Choosen]->y=y;
								arPlace[Choosen]=false;							
							};
						};
					};
				};
				//Во втором ряду
				if (arPlace[Choosen])
				{
					for (int x=0; x<2; x++)
					{
						for (int y=Pos0; MODDD((Pos2+dPos),y) > 0 ; y+=dPos)//new
						//for (int y=0; y<8; y++)
						{
							if (arPlace[Choosen])
							{
								bool bPut=true;
								for (int j=0; j<MAX_SQUAD; j++)
								{									
							if((Heroes[Side]->pSquad[j])&&(Heroes[Side]->pSquad[j]->iLife>0)&&(!arPlace[j])&&(Heroes[Side]->pSquad[j]->x==x)&&(Heroes[Side]->pSquad[j]->y==y))
									{bPut=false;};
								};
								if (bPut)
								{
									Heroes[Side]->pSquad[Choosen]->x=x;
									Heroes[Side]->pSquad[Choosen]->y=y;
									arPlace[Choosen]=false;								
								};
							};
						};
					};
				};
			};
		};
		//Помещение героев командира или мага в задний ряд
		if ((arPlace[0])&&(Heroes[Side]->ipClass1>2))
		{
			for (int x=0; x<2; x++)
			{
				for (int y=Pos0; MODDD((Pos2+dPos),y) > 0 ; y+=dPos)//new
				//for (int y=0; y<8; y++)
				{
					if (arPlace[0])
					{
						bool bPut=true;
						for (int j=0; j<MAX_SQUAD; j++)
						{							
							if ((Heroes[Side]->pSquad[j])&&(!arPlace[j])&&(Heroes[Side]->pSquad[j]->iLife>0)&&(Heroes[Side]->pSquad[j]->x==x)&&(Heroes[Side]->pSquad[j]->y==y))
							{bPut=false;};
						};
						if (bPut)
						{
							Heroes[Side]->pSquad[0]->x=x;
							Heroes[Side]->pSquad[0]->y=y;
							arPlace[0]=false;						
						};
					};
				};
			};
		};
		Choosen=1;
		//Расстановка рукопашных войск
		while (Choosen>=0)
		{
			Choosen=-1;
			for (int i=0; i<MAX_SQUAD; i++)
			{
				if (arPlace[i])
				{
					Choosen=i;
				};
			};
			if (Choosen>=0)
			{
				for (int x=1; x>=0; x--)
				{
					for (int y=Pos0; MODDD((Pos2+dPos),y) > 0 ; y+=dPos)//new
					//for (int y=0; y<8; y++)
					{
						if (arPlace[Choosen])
						{
							bool bPut=true;
							for (int j=0; j<MAX_SQUAD; j++)
							{
							if ((Heroes[Side]->pSquad[j])&&(!arPlace[j])&&(Heroes[Side]->pSquad[j]->iLife>0)&&(Heroes[Side]->pSquad[j]->x==x)&&(Heroes[Side]->pSquad[j]->y==y))
								{bPut=false;};
							};
							if (bPut)
							{
								Heroes[Side]->pSquad[Choosen]->x=x;
								Heroes[Side]->pSquad[Choosen]->y=y;
								arPlace[Choosen]=false;							
							};
						};
					};
				};
			};
		};


		if (Side)
		{
			for (int x=6; x<8; x++)
			{
				for (int y=0; y<8; y++)
				{arGrid[x][y].Pass=1;};
			};
			for (int i=0; i<MAX_SQUAD; i++)
			{
				if ((Heroes[Side]->pSquad[i])&&(Heroes[Side]->pSquad[i]->iLife>0))
				{
					Squad[Side][i].x=7-Heroes[Side]->pSquad[i]->x;
					Squad[Side][i].y=Heroes[Side]->pSquad[i]->y;
					arGrid[Squad[Side][i].x][Squad[Side][i].y].Pass=0;
					Squad[Side][i].G_Obj->x=TransX(Squad[Side][i].x,Squad[Side][i].y,Squad[Side][i].G_Obj->Pic->w);
					Squad[Side][i].G_Obj->y=TransY(Squad[Side][i].x,Squad[Side][i].y,Squad[Side][i].G_Obj->Pic->h);
				};
			};
		}
		else
		{
			for (int x=0; x<2; x++)
			{
				for (int y=0; y<8; y++)
				{arGrid[x][y].Pass=1;};
			};
			for (int i=0; i<MAX_SQUAD; i++)
			{
				if ((Heroes[Side]->pSquad[i])&&(Heroes[Side]->pSquad[i]->iLife>0))
				{
					Squad[Side][i].x=Heroes[Side]->pSquad[i]->x;
					Squad[Side][i].y=Heroes[Side]->pSquad[i]->y;
					arGrid[Squad[Side][i].x][Squad[Side][i].y].Pass=0;
					Squad[Side][i].G_Obj->x=TransX(Squad[Side][i].x,Squad[Side][i].y,Squad[Side][i].G_Obj->Pic->w);
					Squad[Side][i].G_Obj->y=TransY(Squad[Side][i].x,Squad[Side][i].y,Squad[Side][i].G_Obj->Pic->h);
				};
			};
		};
	};	



	//	bool arPlace[MAX_SQUAD];
	//	for (int i=0; i<MAX_SQUAD; i++)
	//	{
	//		if ((Heroes[Side]->pSquad[i])&&(Heroes[Side]->pSquad[i]->iLife>0))
	//		{arPlace[i]=true;}	else {arPlace[i]=false;};
	//	};
	//	int Rx[2];
	//	if (Side)
	//	{Rx[0]=7; Rx[1]=6;} else {Rx[0]=0; Rx[1]=1;};
	//	int Choosen=-1, Range=3;
	//	//Расстановка стрелков
	//	while (Range>2)
	//	{
	//		Range=2; Choosen=-1;
	//		for (int i=0; i<MAX_SQUAD; i++)
	//		{
	//			if ((arPlace[i])&&(Heroes[Side]->pSquad[i]->GetShootingRange()>Range))
	//			{
	//				Range=Heroes[Side]->pSquad[i]->GetShootingRange();
	//				Choosen=i;
	//			};
	//		};
	//		if (Choosen>=0)
	//		{
	//			//На холмах
	//			for (int x=0; x<2; x++)
	//			{
	//				for (int y=0; y<8; y++)
	//				{
	//					if ((arGrid[Rx[x]][y].Land==3)&&(arPlace[Choosen]))
	//					{
	//						bool bPut=true;
	//						for (int j=0; j<MAX_SQUAD; j++)
	//						{								
	//							if ((Heroes[Side]->pSquad[j])&&(Heroes[Side]->pSquad[j]->iLife>0)&&(!arPlace[j])&&(Heroes[Side]->pSquad[j]->x==x)&&(Heroes[Side]->pSquad[j]->y==y))
	//							{bPut=false;};
	//						};
	//						if (bPut)
	//						{
	//							Heroes[Side]->pSquad[Choosen]->x=x;
	//							Heroes[Side]->pSquad[Choosen]->y=y;
	//							arPlace[Choosen]=false;							
	//						};
	//					};
	//				};
	//			};
	//			//Во втором ряду
	//			if (arPlace[Choosen])
	//			{
	//				for (int x=0; x<2; x++)
	//				{
	//					for (int y=0; y<8; y++)
	//					{
	//						if (arPlace[Choosen])
	//						{
	//							bool bPut=true;
	//							for (int j=0; j<MAX_SQUAD; j++)
	//							{									
	//								if ((Heroes[Side]->pSquad[j])&&(Heroes[Side]->pSquad[j]->iLife>0)&&(!arPlace[j])&&(Heroes[Side]->pSquad[j]->x==x)&&(Heroes[Side]->pSquad[j]->y==y))
	//								{bPut=false;};
	//							};
	//							if (bPut)
	//							{
	//								Heroes[Side]->pSquad[Choosen]->x=x;
	//								Heroes[Side]->pSquad[Choosen]->y=y;
	//								arPlace[Choosen]=false;								
	//							};
	//						};
	//					};
	//				};
	//			};
	//		};
	//	};
	//	//Помещение героев командира или мага в задний ряд
	//	if ((arPlace[0])&&(Heroes[Side]->ipClass1>2))
	//	{
	//		for (int x=0; x<2; x++)
	//		{
	//			for (int y=0; y<8; y++)
	//			{
	//				if (arPlace[0])
	//				{
	//					bool bPut=true;
	//					for (int j=0; j<MAX_SQUAD; j++)
	//					{							
	//						if ((Heroes[Side]->pSquad[j])&&(!arPlace[j])&&(Heroes[Side]->pSquad[j]->iLife>0)&&(Heroes[Side]->pSquad[j]->x==x)&&(Heroes[Side]->pSquad[j]->y==y))
	//						{bPut=false;};
	//					};
	//					if (bPut)
	//					{
	//						Heroes[Side]->pSquad[0]->x=x;
	//						Heroes[Side]->pSquad[0]->y=y;
	//						arPlace[0]=false;						
	//					};
	//				};
	//			};
	//		};
	//	};
	//	Choosen=1;
	//	//Расстановка рукопашных войск
	//	while (Choosen>=0)
	//	{
	//		Choosen=-1;
	//		for (int i=0; i<MAX_SQUAD; i++)
	//		{
	//			if (arPlace[i])
	//			{
	//				Choosen=i;
	//			};
	//		};
	//		if (Choosen>=0)
	//		{
	//			for (int x=1; x>=0; x--)
	//			{
	//				for (int y=0; y<8; y++)
	//				{
	//					if (arPlace[Choosen])
	//					{
	//						bool bPut=true;
	//						for (int j=0; j<MAX_SQUAD; j++)
	//						{
	//							if ((Heroes[Side]->pSquad[j])&&(!arPlace[j])&&(Heroes[Side]->pSquad[j]->iLife>0)&&(Heroes[Side]->pSquad[j]->x==x)&&(Heroes[Side]->pSquad[j]->y==y))
	//							{bPut=false;};
	//						};
	//						if (bPut)
	//						{
	//							Heroes[Side]->pSquad[Choosen]->x=x;
	//							Heroes[Side]->pSquad[Choosen]->y=y;
	//							arPlace[Choosen]=false;							
	//						};
	//					};
	//				};
	//			};
	//		};
	//	};
	//	if (Side)
	//	{
	//		for (int x=6; x<8; x++)
	//		{
	//			for (int y=0; y<8; y++)
	//			{arGrid[x][y].Pass=1;};
	//		};
	//		for (int i=0; i<MAX_SQUAD; i++)
	//		{
	//			if ((Heroes[Side]->pSquad[i])&&(Heroes[Side]->pSquad[i]->iLife>0))
	//			{
	//				Squad[Side][i].x=7-Heroes[Side]->pSquad[i]->x;
	//				Squad[Side][i].y=Heroes[Side]->pSquad[i]->y;
	//				arGrid[Squad[Side][i].x][Squad[Side][i].y].Pass=0;
	//				Squad[Side][i].G_Obj->x=TransX(Squad[Side][i].x,Squad[Side][i].y,Squad[Side][i].G_Obj->Pic->w);
	//				Squad[Side][i].G_Obj->y=TransY(Squad[Side][i].x,Squad[Side][i].y,Squad[Side][i].G_Obj->Pic->h);
	//			};
	//		};
	//	}
	//	else
	//	{
	//		for (int x=0; x<2; x++)
	//		{
	//			for (int y=0; y<8; y++)
	//			{arGrid[x][y].Pass=1;};
	//		};
	//		for (int i=0; i<MAX_SQUAD; i++)
	//		{
	//			if ((Heroes[Side]->pSquad[i])&&(Heroes[Side]->pSquad[i]->iLife>0))
	//			{
	//				Squad[Side][i].x=Heroes[Side]->pSquad[i]->x;
	//				Squad[Side][i].y=Heroes[Side]->pSquad[i]->y;
	//				arGrid[Squad[Side][i].x][Squad[Side][i].y].Pass=0;
	//				Squad[Side][i].G_Obj->x=TransX(Squad[Side][i].x,Squad[Side][i].y,Squad[Side][i].G_Obj->Pic->w);
	//				Squad[Side][i].G_Obj->y=TransY(Squad[Side][i].x,Squad[Side][i].y,Squad[Side][i].G_Obj->Pic->h);
	//			};
	//		};
	//	};
	//};




	/*if ((key[KEY_F10])&&(!IsPlayed[1-Side]))
	{
		NewProgress=0;
		OldProgress=0;
		RunHourGlass(150);
		bAutoCombat=true;
		bShowBattle=false;
		bShowAnim=false;			
	};
	if (key[KEY_F11])
	{
		if (IsPlayed[1-Side])
		{IsPlayed[Side]=false;}
		else
		{bAutoCombat=true;};
	};*/
	while (key[KEY_F11]){poll_keyboard();};
	while (key[KEY_F10]){poll_keyboard();};

	buttStart.bBlink=false;
};

int CheatAddUnit()
{
	const int MAX_UNIT_LOCAL=71;
	int WX=800, WY=360;
	int x=512-WX/2;
	int y=384-WY/2;
	int Targer=-1;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(WX,WY);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	rectfill(bmpWindow,0,0,WX,WY,makecol(0,0,120));	
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+550,y+300,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);	
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	UI_Mask mskUnit[MAX_UNIT_LOCAL];
	BITMAP *bmpUnit[MAX_UNIT_LOCAL];
	for (int i=0; i<MAX_UNIT_LOCAL; i++)
	{
		bmpUnit[i]=create_bitmap(140,20);
		rectfill(bmpUnit[i],0,0,140,20,COLOR_1);
		textprintf_ex(bmpWindow,Font1,20+((i-i%17)/17)*150,15+(i%17)*20,COLOR_2,-1,(pFirstUnitType+i+1)->sName);
		textprintf_ex(bmpUnit[i],Font1,10,5,makecol(220,220,20),-1,(pFirstUnitType+i+1)->sName);
		WObj=mskUnit[i].Init(x+10+((i-i%17)/17)*150,y+10+(i%17)*20,1,bmpUnit[i],bmpUnit[i],WObj,2);
	};
	int Result=-1;
	while (Result<0)
	{
		if (buttExit.Check())
		{Result=0;};
		for (int i=0; i<MAX_UNIT_LOCAL; i++)
		{
			if (mskUnit[i].Check()==30)
			{Result=i+1;};
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	for (int i=0; i<MAX_UNIT_LOCAL; i++)
	{destroy_bitmap(bmpUnit[i]);};
	return Result;
};


int CheatAddSpell()
{
	const int MAX_UNIT_LOCAL=77;
	int WX=800, WY=360;
	int x=512-WX/2;
	int y=384-WY/2;
	int Targer=-1;
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(WX,WY);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	rectfill(bmpWindow,0,0,WX,WY,makecol(0,0,120));	
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+550,y+300,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);	
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	UI_Mask mskUnit[MAX_UNIT_LOCAL];
	BITMAP *bmpUnit[MAX_UNIT_LOCAL];
	for (int i=0; i<MAX_UNIT_LOCAL; i++)
	{
		bmpUnit[i]=create_bitmap(140,20);
		rectfill(bmpUnit[i],0,0,140,20,COLOR_1);
		textprintf_ex(bmpWindow,Font1,20+((i-i%17)/17)*150,15+(i%17)*20,COLOR_2,-1,(pFirstSpell+i+1)->sName);
		textprintf_ex(bmpUnit[i],Font1,10,5,makecol(220,220,20),-1,(pFirstSpell+i+1)->sName);
		WObj=mskUnit[i].Init(x+10+((i-i%17)/17)*150,y+10+(i%17)*20,1,bmpUnit[i],bmpUnit[i],WObj,2);
	};
	int Result=-1;
	while (Result<0)
	{
		if (buttExit.Check())
		{Result=0;};
		for (int i=0; i<MAX_UNIT_LOCAL; i++)
		{
			if (mskUnit[i].Check()==30)
			{Result=i+1;};
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);
	for (int i=0; i<MAX_UNIT_LOCAL; i++)
	{destroy_bitmap(bmpUnit[i]);};
	return Result;
};

int InitBattle(dcProvince *Prov1, dcProvince *Prov2, dcHero *Hero1, dcHero *Hero2, int Site, bool bShow)
{	
	iAntisleep=iAntisleepDelay;
	if ((!bShow)&&(!bHourGlass)&&(!bSimulate))
	{		
		NewProgress=0;
		OldProgress=0;
		RunHourGlass(150, 0);
	};
	if ((!bShow)&&(Hero1)&&(Hero2)&&(arPlayer[Hero1->ipPlayer].Control!=1)&&(arPlayer[Hero2->ipPlayer].Control!=1))
	{
		bAIBattle=true;
		int RS=CompareStrength1(Hero1, Hero2);
		bAIBattle=false;
		return RS;
	};

	bEndWarning=false;
	iTarget=-1;
	SummonExp[0]=0;
	SummonExp[1]=0;
	SummonSurv[0]=0;		
	SummonSurv[1]=0;
	SummonWork[0]=0;
	SummonWork[1]=0;
	ActiveSpell=-1;
	ActiveUnit=NULL;
	Gomunculus=NULL;
	iTurn=0;
	Heroes[0]=Hero1;
	Heroes[1]=Hero2;
	BF_Player[0]=Hero1->ipPlayer;
	BF_Player[1]=Hero2->ipPlayer;
	SideCanCast[0]=2;
	SideCanCast[1]=2;
	for (int i=0; i<2; i++)
	{
		if (arPlayer[BF_Player[i]].Control==1)
		{IsPlayed[i]=true;}
		else
		{IsPlayed[i]=false;};
	};
	
	bShowBattle=bShow;
	bShowAnim=bShow;
		//		bShowBattle=true;
		//		bShowAnim=true;
		//		bHourGlass=false;
	bAutoCombat=false;
	if (bShowBattle)
	{
		for (int i=0; i<2; i++)
		{
			if (arPlayer[BF_Player[i]].Control==1)
			{
				if ((arHumans[arPlayer[BF_Player[i]].iColor].iAutoCombat>0)&&(BF_Player[1-i]==0))
				{bAutoCombat=true;};
				if ((arHumans[arPlayer[BF_Player[i]].iColor].iAutoCombat>1)&&(arPlayer[BF_Player[1-i]].Control>1))
				{bAutoCombat=true;};
				if ((arHumans[arPlayer[BF_Player[i]].iColor].iAutoCombat>1)&&(arPlayer[BF_Player[1-i]].Control==1)&&(arHumans[arPlayer[BF_Player[1-i]].iColor].iAutoCombat>1))
				{bAutoCombat=true;};
			};
		};
		if (bAutoCombat)
		{
			NewProgress=0;
			OldProgress=0;
			RunHourGlass(150, 0);
			bShowBattle=false;
			bShowAnim=false;
		};
	};
	for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
	{
		Squad[0][i].Clear();
		Squad[1][i].Clear();
	}; 
	int TowerShoot=0;
	ProvinceMoraleBonus=0;
	for (int i=0; i<3; i++)
	{unMagTower[i]=NULL;};
	MagTower=0;
	if ((Site<0)||((Prov2)&&(Prov2->ipPlayer==Hero2->ipPlayer)))
	{
		dcProvince *PR;
		if (Prov2)
		{PR=Prov2;}
		else
		{PR=Prov1;};
		int PL=0;
		for (int i=1; i<=CurGame.iMaxPlayers; i++)
		{
			if (arPlayer[i].ipProvince==PR->iNumber)
			{PL=i;};
		};
		if (PL>0)
		{
			ProvinceMoraleBonus=arPlayer[PL].GetBldParam1(19);
			for (int i=1; i<=iMaxInnerBld; i++)
			{
				if (arPlayer[PL].arBld[i])
				{
					for (int j=0; j<MAX_BUILDING_ABILITY; j++)
					{
						if ((pFirstInnerBld+i)->arAbility[j][0]==52)
						{
							unMagTower[MagTower]=new dcUnit((pFirstInnerBld+i)->arAbility[j][1],0);
							unMagTower[MagTower]->iLevel=0;
							for (int i=0; i<20; i++)
							{unMagTower[MagTower]->arUpg[i]=20;};
							while ((unMagTower[MagTower]->GetRangedAttack()<(pFirstInnerBld+i)->arAbility[j][2])&&(unMagTower[MagTower]->iLevel<20))
							{unMagTower[MagTower]->iLevel++;};
							bfMagTower[MagTower].Clear();
							objMagTower[MagTower]=bfMagTower[MagTower].Init(unMagTower[MagTower],NULL,1,1);
							bfMagTower[MagTower].bSummoned=true;
							objMagTower[MagTower]->x=1025;
							MagTower++;
						};
					};
				};
			};
		}
		else
		{
			for (int i=0; i<MAX_PROVINCE_BLD; i++)
			{
				if (PR->arBuilding[i]>0)
				{
					for (int j=0; j<MAX_BUILDING_ABILITY; j++)
					{
						if ((pFirstOuterBld+PR->arBuilding[i])->arAbility[j][0]==10)
						{
							ProvinceMoraleBonus+=(pFirstOuterBld+PR->arBuilding[i])->arAbility[j][1];
						};
					};
					if (((pFirstOuterBld+PR->arBuilding[i])->iTowerShoot>0)&&((pFirstOuterBld+PR->arBuilding[i])->iWall==0))
					{
						unMagTower[MagTower]=new dcUnit((pFirstOuterBld+PR->arBuilding[i])->iTowerType,0);
						unMagTower[MagTower]->iLevel=0;
						for (int k=0; k<20; k++)
						{unMagTower[MagTower]->arUpg[k]=20;};
						while ((unMagTower[MagTower]->GetRangedAttack()<(pFirstOuterBld+PR->arBuilding[i])->iTowerShoot)&&(unMagTower[MagTower]->iLevel<20))
						{unMagTower[MagTower]->iLevel++;};
						bfMagTower[MagTower].Clear();
						objMagTower[MagTower]=bfMagTower[MagTower].Init(unMagTower[MagTower],NULL,1,1);
						bfMagTower[MagTower].bSummoned=true;
						objMagTower[MagTower]->x=1025;
						MagTower++;
					};
				};
			};
		};
	};
	if (Site<0)
	{
		Site=100;
		bCastle=true;
	}
	else
	{bCastle=false;};	
	

	

//textprintf(screen,Font1,100,100,COLOR_2,"Генерация поля боя");	
//Генерация поля боя
	if (Prov2)
	//На границе провинций
	{		
		int LeftPass, LeftImpass, RightPass, RightImpass;
		LeftPass=(pFirstTerrain+Prov1->ipTerrain)->iBF_Pass;
		LeftImpass=(pFirstTerrain+Prov1->ipTerrain)->iBF_Impass;
		RightPass=(pFirstTerrain+Prov2->ipTerrain)->iBF_Pass;
		RightImpass=(pFirstTerrain+Prov2->ipTerrain)->iBF_Impass;

	//Столбцы 0,1
		ClearRoll();
		RollerN[0]=1;
		RollerP[0]=45;
		RollerN[1]=LeftPass;
		RollerP[1]=40;
		int Counter=1;
		for (int i=2; i<=iMaxBF_Object; i++)
		{
			if ((pFirstBF_Obj+i)->iMoveCost)
			{
				Counter++;
				RollerN[Counter]=i;
				RollerP[Counter]=5;
			};
		};
		for (int i=0; i<2; i++)
		{
			for (int j=0; j<8; j++)
			{
				arGrid[i][j].Land=Roll(0,i,j,Prov1->iNumber,Prov2->iNumber);
				arGrid[i][j].Pass=1;
			};
		};

	//Столбец 2
		ClearRoll();
		RollerN[0]=1;
		RollerP[0]=45;
		RollerN[1]=LeftPass;
		RollerP[1]=30;
		RollerN[2]=RightPass;
		RollerP[2]=6;
		RollerN[3]=LeftImpass;
		RollerP[3]=7;
		RollerN[4]=RightImpass;
		RollerP[4]=3;
		Counter=4;
		for (int i=2; i<=iMaxBF_Object; i++)
		{
			if ((pFirstBF_Obj+i)->iMoveCost)
			{
				Counter++;
				RollerN[Counter]=i;
				RollerP[Counter]=3;
			};
		};
		for (int j=0; j<8; j++)
		{
			arGrid[2][j].Land=Roll(0,2,j,Prov1->iNumber,Prov2->iNumber);
			arGrid[2][j].Pass=1;
		};

	//Столбец 3
		ClearRoll();
		RollerN[0]=1;
		RollerP[0]=45;
		RollerN[1]=LeftPass;
		RollerP[1]=26;
		RollerN[2]=RightPass;
		RollerP[2]=10;
		RollerN[3]=LeftImpass;
		RollerP[3]=6;
		RollerN[4]=RightImpass;
		RollerP[4]=4;
		Counter=4;
		for (int i=2; i<=iMaxBF_Object; i++)
		{
			if ((pFirstBF_Obj+i)->iMoveCost)
			{
				Counter++;
				RollerN[Counter]=i;
				RollerP[Counter]=3;
			};
		};
		for (int j=0; j<8; j++)
		{
			arGrid[3][j].Land=Roll(0,3,j,Prov1->iNumber,Prov2->iNumber);
			arGrid[3][j].Pass=1;
		};

	//Столбцы 6,7
		ClearRoll();
		RollerN[0]=1;
		RollerP[0]=45;		
		RollerN[1]=RightPass;
		RollerP[1]=40;
		Counter=1;
		for (int i=2; i<=iMaxBF_Object; i++)
		{
			if ((pFirstBF_Obj+i)->iMoveCost)
			{
				Counter++;
				RollerN[Counter]=i;
				RollerP[Counter]=5;
			};
		};		
		for (int i=6; i<8; i++)
		{
			for (int j=0; j<8; j++)
			{
				arGrid[i][j].Land=Roll(0,i,j,Prov1->iNumber,Prov2->iNumber);
				arGrid[i][j].Pass=1;
			};
		};

	//Столбец 5		
		ClearRoll();
		RollerN[0]=1;
		RollerP[0]=45;		
		RollerN[1]=RightPass;
		RollerP[1]=30;
		RollerN[2]=LeftPass;
		RollerP[2]=6;
		RollerN[3]=RightImpass;
		RollerP[3]=7;
		RollerN[4]=LeftImpass;
		RollerP[4]=3;
		Counter=4;
		for (int i=2; i<=iMaxBF_Object; i++)
		{
			if ((pFirstBF_Obj+i)->iMoveCost)
			{
				Counter++;
				RollerN[Counter]=i;
				RollerP[Counter]=3;
			};
		};		
		for (int j=0; j<8; j++)
		{
			arGrid[5][j].Land=Roll(0,5,j,Prov1->iNumber,Prov2->iNumber);
			arGrid[5][j].Pass=1;
		};
		
	//Столбец 4
		ClearRoll();
		RollerN[0]=1;
		RollerP[0]=45;		
		RollerN[1]=RightPass;
		RollerP[1]=26;
		RollerN[2]=LeftPass;
		RollerP[2]=10;
		RollerN[3]=RightImpass;
		RollerP[3]=6;
		RollerN[4]=LeftImpass;
		RollerP[4]=4;
		Counter=4;
		for (int i=2; i<=iMaxBF_Object; i++)
		{
			if ((pFirstBF_Obj+i)->iMoveCost)
			{
				Counter++;
				RollerN[Counter]=i;
				RollerP[Counter]=3;
			};
		};		
		for (int j=0; j<8; j++)
		{
			arGrid[4][j].Land=Roll(0,3,j,Prov1->iNumber,Prov2->iNumber);
			arGrid[4][j].Pass=1;
		};
	}
	else		
//Внутри провинции
	{
		int Pass, Impass;
		Pass=(pFirstTerrain+Prov1->ipTerrain)->iBF_Pass;
		Impass=(pFirstTerrain+Prov1->ipTerrain)->iBF_Impass;		

	//Столбцы 0,1,6,7
		ClearRoll();
		RollerN[0]=1;
		RollerP[0]=45;		
		RollerN[1]=Pass;
		RollerP[1]=40;
		int Counter=1;
		for (int i=2; i<=iMaxBF_Object; i++)
		{
			if ((pFirstBF_Obj+i)->iMoveCost)
			{
				Counter++;
				RollerN[Counter]=i;
				RollerP[Counter]=5;
			};
		};
		for (int i=0; i<2; i++)
		{
			for (int j=0; j<8; j++)
			{
				arGrid[i][j].Land=Roll(0,i,j,Prov1->iNumber, Site);
				arGrid[i][j].Pass=1;
			};
		};
		if (bCastle)
		{
			for (int i=6; i<8; i++)
			{
				for (int j=0; j<8; j++)
				{
					arGrid[i][j].Land=1;
					arGrid[i][j].Pass=1;
				};
			};
		}
		else
		{
			for (int i=6; i<8; i++)
			{
				for (int j=0; j<8; j++)
				{
					arGrid[i][j].Land=Roll(0,i,j,Prov1->iNumber, Site);
					arGrid[i][j].Pass=1;
				};
			};
		};

	//Столбцы 2-5
		ClearRoll();
		RollerN[0]=1;
		RollerP[0]=45;
		RollerN[1]=Pass;
		RollerP[1]=36;
		RollerN[2]=Impass;
		RollerP[2]=10;		
		Counter=2;
		for (int i=2; i<=iMaxBF_Object; i++)
		{
			if ((pFirstBF_Obj+i)->iMoveCost)
			{
				Counter++;
				RollerN[Counter]=i;
				RollerP[Counter]=3;
			};
		};
		for (int i=2; i<6; i++)
		{
			for (int j=0; j<8; j++)
			{
				if ((bCastle)&&(i>3))
				{
					arGrid[i][j].Land=1;
					arGrid[i][j].Pass=1;
				}
				else
				{
					arGrid[i][j].Land=Roll(0,i,j,Prov1->iNumber, Site);
					arGrid[i][j].Pass=1;
				};
			};
		};

	//Получение данных об укреплениях
		if (bCastle)
		{
			int Towerz, Fort, Build=0;			
			if (arPlayer[Prov1->ipPlayer].ipProvince==Prov1->iNumber)
			{
				Fort=100*Prov1->iFort/arPlayer[Prov1->ipPlayer].iWall;
				Towerz=arPlayer[Prov1->ipPlayer].iTower;
				TowerShoot=arPlayer[Prov1->ipPlayer].iTowerShoot;
			}
			else
			{
				for (int i=0; i<MAX_PROVINCE_BLD; i++)
				{
					if ((Prov1->arBuilding[i]>0)&&(pFirstOuterBld+Prov1->arBuilding[i])->iWall>0)
					{
							Build=Prov1->arBuilding[i];
					};
				};
//				textprintf(screen,Font1,10,100,COLOR_2,"провинция %s, игрок %d",Prov1->sName,Prov1->ipPlayer);
				Fort=100*Prov1->iFort/(pFirstOuterBld+Build)->iWall;
				Towerz=(pFirstOuterBld+Build)->iTowerType;
				TowerShoot=(pFirstOuterBld+Build)->iTowerShoot;
			};
	//Эмуляция башен
			if (TowerShoot>0)
			{
				unTower=new dcUnit(4,0);
				unTower->iLevel=0;
				for (int i=0; i<20; i++)
				{unTower->arUpg[i]=20;};
				while ((unTower->GetRangedAttack()<TowerShoot)&&(unTower->iLevel<20))
				{unTower->iLevel++;};
				bfTower.Clear();
				unTower->x=7;
				unTower->y=0;
				objTower=bfTower.Init(unTower,NULL,1,1);
				arGrid[7][0].Pass=1;
				bfTower.bSummoned=true;
				objTower->x=1025;
			};
	//Линия укреплений
			if (Fort==0)
			{arGrid[5][0].Land=11;}
			else
			{
				if (Towerz>=4)
				{
					arGrid[5][0].Land=10;
					arGrid[5][0].Pass=0;
				}
				else
				{
					arGrid[5][0].Land=9;
				};
			};
			if (Fort<=10)
			{arGrid[5][6].Land=11;}
			else
			{
				if (Towerz>=3)
				{
					arGrid[5][6].Land=10;
					arGrid[5][6].Pass=0;
				}
				else
				{
					arGrid[5][6].Land=9;
				};
			};
			if (Fort<=25)
			{arGrid[5][2].Land=11;}
			else
			{
				if (Towerz>=2)
				{
					arGrid[5][2].Land=10;
					arGrid[5][2].Pass=0;
				}
				else
				{
					arGrid[5][2].Land=9;
				};
			};
			if (Fort<=35)
			{arGrid[5][4].Land=11;}
			else
			{
				if (Towerz>=1)
				{
					arGrid[5][4].Land=10;
					arGrid[5][4].Pass=0;
				}
				else
				{
					arGrid[5][4].Land=9;
				};
			};			
			if (Fort<=30)
			{arGrid[5][1].Land=11;}
			else
			{arGrid[5][1].Land=9;};
			if (Fort<=50)
			{arGrid[5][3].Land=11;}
			else
			{arGrid[5][3].Land=9;};
			if (Fort<=40)
			{arGrid[5][5].Land=11;}
			else
			{arGrid[5][5].Land=9;};
			if (Fort<=20)
			{arGrid[5][7].Land=11;}
			else
			{arGrid[5][7].Land=9;};
		};
	};
	int Clear=Random(8,Prov1->iNumber,CurGame.iSeed);
	for (int i=2; i<6; i++)
	{
		if ((arGrid[i][Clear].Land<9)&&((pFirstBF_Obj+arGrid[i][Clear].Land)->iMoveCost==0))
		{arGrid[i][Clear].Land=1;};
	};
	
//Визуализация поля боя
	Background=create_bitmap(1024,768);
	if (bShowAnim)
	{
		MakeBackground(Prov1->iNumber);
		iGameMode=4;
		ChangeMusic();
	};//ShowAnim
	
	objPanel=create_bitmap(jPanel->w, jPanel->h);
	blit(jPanel,objPanel,0,0,0,0,jPanel->w,jPanel->h);
	
	objSelect=new G_Object(NULL,0,0,-1,bmpSelect);
	if (TowerShoot>0)
	{objMain=objTower;}
	else
	{objMain=NULL;};
	for (int i=0; i<MagTower; i++)
	{
		objMagTower[i]->pNext=objMain;
		objMain=objMagTower[i];
	};
	objMain=new G_Object(objMain,0,0,0,Background);
	objMain=new G_Object(objMain,884,0,1,objPanel);
	objMain=new G_Object(objMain,512-bmpLowString->w/2,768-bmpLowString->h-3,-1,bmpLowString);	
	objLowString=objMain;
	objMain=new G_Object(objMain,1024-bmpSideBar->w,0,-1,bmpSideBar);
	objSideBar=objMain;

	int XX;
	for (int x=0; x<8; x++)
	{
		for (int y=0; y<8; y++)
		{			
			if (y%2)
			{XX=70+x*104-5;}
			else
			{XX=18+x*104-5;};
			objMain=new G_Object(objMain,XX,15+y*90+60,-1,bmpTarget);
			objTarget[x][y]=objMain;
			objMain=new G_Object(objMain,XX,15+y*90+60,-1,bmpTargetH);
			objTargetH[x][y]=objMain;
		};
	};



	objMain=buttRest.Init(1024-jPanel->w+14,247,1,3,bmpButton[0],bmpButton[1],bmpButton[2],bmpButton[3],objMain);
	buttRest.Activate(1);
	buttRest.ipHotkey=KEY_R;
	buttRest.iHint=119;
	buttRest.wavSound=wavClick;

	objMain=buttNext.Init(1024-jPanel->w+14+62,247,1,3,bmpButton[4],bmpButton[5],bmpButton[6],bmpButton[7],objMain);
	buttNext.Activate(1);
	buttNext.ipHotkey=KEY_N;
	buttNext.ipHotkey2=KEY_SPACE;
	buttNext.iHint=120;
	buttNext.wavSound=wavClick;

	buttGrid.Init(1025,770,1,3,bmpButton[4],bmpButton[5],bmpButton[6],bmpButton[7],NULL);
	buttGrid.Activate(1);
	buttGrid.ipHotkey=KEY_G;

	objMain=buttEndTurn.Init(1024-jPanel->w+71,700,1,3,bmpButton[8],bmpButton[9],bmpButton[10],bmpButton[11],objMain);
	buttEndTurn.Activate(1);
	buttEndTurn.ipHotkey=KEY_E;
	buttEndTurn.iHint=100;
	buttEndTurn.wavSound=wavClick;

	objMain=buttGather.Init(889,279,1,3,bmpButton[12],bmpButton[13],bmpButton[14],bmpButton[15],objMain);
	buttGather.Activate(-1);
	buttGather.iHint=134;
	buttGather.ipHotkey=KEY_P;	
	buttGather.wavSound=wavGather;

	objMain=buttHeal.Init(889,279,1,3,bmpButton[16],bmpButton[17],bmpButton[18],bmpButton[19],bmpButton[20],objMain);
	buttHeal.Activate(-1,0);
	buttHeal.iHint=121;
	buttHeal.iHint1=122;
	buttHeal.ipHotkey=KEY_H;
	buttHeal.wavSound=wavClick;

	objMain=buttSpell.Init(1024-jPanel->w+14,247+56,1,3,bmpButton[21],bmpButton[22],bmpButton[23],bmpButton[24],objMain);
	buttSpell.Activate(-1);
	buttSpell.ipHotkey=KEY_S;
	buttSpell.iHint=123;
	buttSpell.wavSound=wavParchment;

	objMain=buttChange.Init(1024-jPanel->w+14+62,247+56,1,3,bmpButton[315],bmpButton[316],bmpButton[317],bmpButton[318],objMain);
	buttChange.Activate(-1);
	buttChange.ipHotkey=KEY_X;
	buttChange.iHint=451;
	buttChange.wavSound=wavClick;

	objMain=buttCancelSpell.Init(1024-jPanel->w+38,694,1,3,bmpButton[25],bmpButton[26],bmpButton[27],bmpButton[27],objMain);
	buttCancelSpell.Activate(-1);
	buttCancelSpell.ipHotkey=KEY_ESC;
	buttCancelSpell.iHint=124;
	buttCancelSpell.wavSound=wavClick;

	objMain=buttMarch.Init(927,107,1,3,bmpButton[28],bmpButton[29],bmpButton[30],bmpButton[31],objMain);
	buttMarch.Activate(-1);
	buttMarch.iHint=125;
	buttMarch.ipHotkey=KEY_M;	
	buttMarch.wavSound=wavMarch;

	objMain=buttEat.Init(927,107,1,3,bmpButton[32],bmpButton[33],bmpButton[34],bmpButton[35],objMain);
	buttEat.Activate(-1);
	buttEat.iHint=126;
	buttEat.ipHotkey=KEY_T;
	buttEat.wavSound=wavEat;

	objMain=buttStart.Init(1024-jPanel->w+71,700,1,4,bmpButton[36],bmpButton[37],bmpButton[254],bmpButton[254],objMain);
	buttStart.Activate(1);
	buttStart.iHint=127;
	buttStart.ipHotkey=KEY_E;
	buttStart.wavSound=wavClick;

	objMain=buttCrush.Init(889,279,1,3,bmpButton[38],bmpButton[39],bmpButton[40],bmpButton[42],bmpButton[41],objMain);
	buttCrush.Activate(-1,0);
	buttCrush.iHint=128;
	buttCrush.iHint1=129;
	buttCrush.ipHotkey=KEY_C;
	buttCrush.wavSound=wavClick;

	objMain=buttDoubleShot.Init(889,279,1,3,bmpButton[43],bmpButton[44],bmpButton[45],bmpButton[47],bmpButton[46],objMain);
	buttDoubleShot.Activate(-1,0);
	buttDoubleShot.iHint=130;
	buttDoubleShot.iHint1=131;
	buttDoubleShot.ipHotkey=KEY_D;
	buttDoubleShot.wavSound=wavClick;

	objMain=buttRoundAtt.Init(889,279,1,3,bmpButton[173],bmpButton[174],bmpButton[175],bmpButton[177],bmpButton[176],objMain);
	buttRoundAtt.Activate(-1,0);
	buttRoundAtt.iHint=132;
	buttRoundAtt.iHint1=133;
	buttRoundAtt.ipHotkey=KEY_A;
	buttRoundAtt.wavSound=wavClick;

//	objMain=buttUnitBar.Init(1024-jPanel->w+4,718,1,3,bmpButton[87],bmpButton[88],bmpButton[89],bmpButton[90],bmpButton[90],objMain);
//	buttUnitBar.Activate(1,0);

	objMain=buttMenu.Init(1024-jPanel->w+4,718,1,3,bmpButton[87],bmpButton[88],bmpButton[90],bmpButton[89],objMain);
	buttMenu.Activate(1);
	buttMenu.iHint=351;
	buttMenu.ipHotkey=KEY_O;
	buttMenu.wavSound=wavClick;

	objMain=txtSpName.Init(1024-jPanel->w+11,106,122,37,3,COLOR_1,Font1,objMain);
	txtSpName.bCentre=true;
	txtSpName.Hide();
	objMain=txtSpTarget.Init(1024-jPanel->w+11,144,122,37,3,COLOR_1,Font1,objMain);
	txtSpTarget.bCentre=true;
	txtSpTarget.Hide();
	objMain=txtSpDesc.Init(1024-jPanel->w+11,300,122,360,3,COLOR_1,Font2,objMain);
	txtSpDesc.Hide();

	objMain=new G_Object(objMain,20,20,-1,bmpGUI_Hint);
	objTutorial=objMain;	
	objMain=mskTutorial.Init(20,20,1,bmpGUI_Hint,bmpEmpty,objMain,-1);	
	objMain=txtTutorial.Init(40,50,297,200,7,COLOR_1,Font2,objMain);
	txtTutorial.bShadow=false;
	txtTutorial.Hide();
	bTutHint=false;
	TutQue=0;

	for (int i=0; i<6; i++)
	{
		objMain=mskEff[i].Init(1024-jPanel->w+13+62*(i%2)-6,498+68*((i-i%2)/2)-6,-1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objMain,3);
	};

	

	for (int i=0; i<8; i++)
	{
		for (int j=0; j<8; j++)
		{
			if (j%2){objMain=Grid[i][j].Init(52+i*104,j*90,1,jMask,jHexx,objMain,1);}
			else{objMain=Grid[i][j].Init(i*104,j*90,1,jMask,jHexx,objMain,1);};
			if (j%2){objMain=new G_Object(objMain,52+i*104,j*90,-1,bmpGrid);}
			else{objMain=new G_Object(objMain,i*104,j*90,-1,bmpGrid);};
			objGrid[i][j]=objMain;
		};
	};	
	
	
//Получение информации о юнитах и расстановка войск
	

//Модификаторы жизни нейтралов в зависимости от сложности!
	LifeMod[0]=100;
	LifeMod[1]=100;
	for (int i=0; i<2; i++)
	{
		//AI
		if ((arPlayer[BF_Player[0]].Control!=1)&&(arPlayer[BF_Player[1]].Control!=1))
		{
			if (arPlayer[BF_Player[i]].iDifficulty<arPlayer[BF_Player[1-i]].iDifficulty)
			{
				LifeMod[i]=(10+arPlayer[BF_Player[i]].iDifficulty-arPlayer[BF_Player[1-i]].iDifficulty)*10;
			};		
		};	
		//Humanz
		if ((arPlayer[BF_Player[1-i]].Control==1)&&(BF_Player[i]==0))
		{
			LifeMod[i]=arDifficulty[arPlayer[BF_Player[1-i]].iDifficulty].iNeutralLife;		
		};	
	};

	FileLog("Unit placement");
	if (Hero1->CheckHeroBonus(999)<Hero2->CheckHeroBonus(999))
	{
		AA=1;
		UnitPlacement(1);
		UnitPlacement(0);
	}
	else
	{
		AA=0;
		UnitPlacement(0);
		UnitPlacement(1);
	};
	FileLog("Unit placement end");
	//Диверсии
	if ((Heroes[0])&&(Squad[0][0].iLife>0)&&(Heroes[0]->ipClass1>0)&&((Heroes[0]->CheckSkill(24)>1)||(Squad[0][0].CheckSkill(981)>0)))
	{
		int Div[3];
		for (int i=0; i<3; i++)
		{Div[i]=0;};
		int Skill=Heroes[0]->CheckSkill(24);
		if (Squad[0][0].CheckSkill(981)>0)
		{Skill++;};
		for (int i=0; i<MAX_SQUAD; i++)
		{
			if ((Squad[1][i].bInBattle)&&(Squad[1][i].iLife>0))
			{
				if (Squad[1][i].CheckSkill(18)==0)
				{Div[0]+=Squad[1][i].pUnit->pType()->iLevel;};
				if ((Skill>2)&&(Squad[1][i].CheckSkill(42)==0))
				{Div[1]+=Squad[1][i].pUnit->pType()->iLevel;};					
				if ((Skill>3)&&(Squad[1][i].CheckSkill(19)==0))
				{Div[2]+=Squad[1][i].pUnit->pType()->iLevel;};
			};
		};			
		if (Div[0]>0)
		{Div[0]=15+Div[0]*4;};
		if (Div[1]>0)
		{Div[1]=20+Div[1]*5;};
		if (Div[2]>0)
		{Div[2]=30+Div[2]*6;};
		if (Heroes[0]->CheckSkill(24)>=5)
		{
			for (int i=0; i<3; i++)
			{Div[i]/=2;};
		};
		bool arAn[4];
		int arDiv[4];
		for (int i=0; i<4; i++)
		{
			arAn[i]=false;
			arDiv[i]=-1;
			sprintf((pFirstDialog+1206)->arAnswer[i],"###");
		};
		int Count=0;
		for (int i=0; i<3; i++)
		{
			if (Div[i]>0)
			{
				sprintf((pFirstDialog+1206)->arAnswer[Count],(pFirstPhrase+455+i)->str,Div[i]);
				if (Div[i]<=arPlayer[Heroes[0]->ipPlayer].Gold)
				{arAn[Count]=true;};
				arDiv[Count]=i;
				Count++;
			};
		};
		if (Count>0)
		{
			int Answer=4;
			if (arPlayer[Heroes[0]->ipPlayer].Control==1)
			{
				if (bShowAnim)
				{
					sprintf((pFirstDialog+1206)->arAnswer[Count],(pFirstPhrase+458)->str);
					arAn[Count]=true;
					char Txt[500];
					sprintf(Txt,(pFirstDialog+1206)->sText,Heroes[0]->pstName->str,arPlayer[Heroes[0]->ipPlayer].Gold);
					Answer=(pFirstDialog+1206)->Show(arAn[0],arAn[1],arAn[2],arAn[3],NULL,Txt);
				};
			}
			else
			{
				Answer=Random(Count)+1;
				if (arPlayer[Heroes[0]->ipPlayer].Gold<Div[arDiv[Answer-1]]*2)
				{Answer=4;};
			};
			if (arDiv[Answer-1]>=0)
			{
				arPlayer[Heroes[0]->ipPlayer].Gold-=Div[arDiv[Answer-1]];					
				switch (arDiv[Answer-1])
				{
				case 0:
					for (int i=0; i<MAX_SQUAD; i++)
					{
						if ((Squad[1][i].bInBattle)&&(Squad[1][i].iLife>0)&&(Squad[1][i].CheckSkill(18)==0))
						{									
							Squad[1][i].iStamina-=(6+Squad[0][0].CheckSkill(981));
							if (Squad[1][i].iStamina<2)
							{Squad[1][i].iStamina=2;};
							Squad[1][i].DrawBar();
						};
					};
					if ((bShowAnim)&&(arPlayer[Heroes[0]->ipPlayer].Control!=1)&&(arPlayer[Heroes[1]->ipPlayer].Control==1))
					{(pFirstDialog+1207)->Show(1,1,0,0,NULL,NULL);};
					break;
				case 1:
					for (int i=0; i<MAX_SQUAD; i++)
					{
						if ((Squad[1][i].bInBattle)&&(Squad[1][i].iLife>0))
						{
							if (Squad[1][i].CheckSkill(42)==0)
							{
								Squad[1][i].pEffect=new BF_Effect(Squad[1][i].pEffect,1,-1-Squad[0][0].CheckSkill(981),POISON_DURATION);
								Squad[1][i].pEffect->bVisible=true;
								Squad[1][i].pEffect->pAbility=(pFirstUnitUpg+71);
							};
						};
					};
					if ((bShowAnim)&&(arPlayer[Heroes[0]->ipPlayer].Control!=1)&&(arPlayer[Heroes[1]->ipPlayer].Control==1))
					{(pFirstDialog+1208)->Show(1,1,0,0,NULL,NULL);};
					break;
				case 2:
					for (int i=0; i<MAX_SQUAD; i++)
					{
						if ((Squad[1][i].bInBattle)&&(Squad[1][i].iLife>0)&&(Squad[1][i].CheckSkill(19)==0))
						{									
							Squad[1][i].iMorale-=(4+Squad[0][0].CheckSkill(981));
							if (Squad[1][i].iMorale<1)
							{Squad[1][i].iMorale=1;};
							Squad[1][i].DrawBar();
						};
					};
					if ((bShowAnim)&&(arPlayer[Heroes[0]->ipPlayer].Control!=1)&&(arPlayer[Heroes[1]->ipPlayer].Control==1))
					{(pFirstDialog+1209)->Show(1,1,0,0,NULL,NULL);};
					break;
				};
			};
		};
	};

	if (bShowAnim)
	{	
		ShowGrid(0,7);
		iGameMode=3;
		ChangeMusic();
	};

	buttStart.Activate(-1);
	buttEndTurn.Activate(1);

	
//Получение информации о заклинаниях

	for (int Sd=0; Sd<2; Sd++)
	{
		for (int i=0; i<SPELLBOOK_SIZE; i++)
		{
			Spells[Sd][0][i]=0;
			Spells[Sd][1][i]=0;
		};
		int Nmbr=0;
		for (int i=1; i<=SPELLBOOK_SIZE; i++)
		{
			if (Heroes[Sd]->arSpell[i])
			{
				Spells[Sd][0][Nmbr]=Heroes[Sd]->arSpell[i];
				Spells[Sd][1][Nmbr]=1;
				Nmbr++;
			};
		};
	};


//Основной процесс

	clear_keybuf();

	int arUseAmmo[2];
	for (int i=0; i<2; i++)
	{
		if ((Squad[i][0].bInBattle)&&(Squad[i][0].pUnit)&&(Squad[i][0].GetAmmo()>0))
		{arUseAmmo[i]=Squad[i][0].GetAmmo();}
		else
		{arUseAmmo[i]=-1;};
	};

	bool bCursorChange;
	bool bMouseReleased=false;
	bool bSacrifice;
	bool bEndWarning=false;
	int OldX=-1, OldY=-1, Check;
	AA=1-AA;
	TutMode=0;
	int TimeWarning=MAX_BATTLE_DURATION-5;
	EndTurn(true, false);
	buttEndTurn.Activate(1);	
	if (Tutorial.Map)
	{
		if (Tutorial.Site==0)
		{
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+31)->msg);
			TutMode=31;
			buttEndTurn.Activate(-1);
	//		buttMenu.Activate(-1);
		};
		if (Tutorial.Site==2)
		{
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+34)->msg);
			TutMode=34;
			buttEndTurn.Activate(-1);
		};
		if ((Prov2)&&(Prov2->iNumber==21))
		{
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+44)->msg);
			TutMode=44;
		};
		if ((Tutorial.Battle==1)&&(Hero1)&&(Hero1->iLevel>=4))
		{
			Tutorial.Battle=2;
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+55)->msg);
			TutMode=55;
			buttMenu.bBlink=true;
		};
		if ((bCastle)&&(Tutorial.Fortify==0))
		{
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			txtTutorial.Activate((pFirstTutText+95)->msg);
			TutMode=95;
			Tutorial.Fortify=1;
		};
	};
	FearVictory=0;
	bInPanic=false;

	FileLog("Start Fight!");
	while((Loss(AA)>=0)&&(Loss(1-AA)>=0)&&(iTurn/2<=MAX_BATTLE_DURATION)&&(FearVictory==0))
	{
		bHint=false;
		objSideBar->iLayer=-1;
		if (iTurn/2==TimeWarning)
		{
			bEndWarning=true;
			bTutHint=true;
			ScrollSound();
			objTutorial->iLayer=5;
			char Txt[500];
			sprintf(Txt,(pFirstMessage+25)->msg, MAX_BATTLE_DURATION-iTurn/2);
			txtTutorial.Activate(Txt);
			TimeWarning+=2;
			bEndWarning=true;
		};

		if (bTutHint)
		{			
			int Check;
			if ((Tutorial.Mode)||(bEndWarning))
			{Check=mskTutorial.Check();}
			else
			{Check=30;};
			if ((Check==30)||(Check==40))
			{
				bEndWarning=false;
				switch (TutMode)
				{
				case 31:
					txtTutorial.Activate((pFirstTutText+32)->msg);
					TutMode=32;
					break;
				case 32:
					txtTutorial.Activate((pFirstTutText+33)->msg);
					TutMode=33;
					buttEndTurn.Activate(1);
					//buttEndTurn.bBlink=true;
					break;
				case 34:
					txtTutorial.Activate((pFirstTutText+35)->msg);
					TutMode=35;
					buttEndTurn.Activate(1);
					break;
				case 36:
					txtTutorial.Activate((pFirstTutText+37)->msg);
					TutMode=37;
					break;
				case 42:
					txtTutorial.Activate((pFirstTutText+43)->msg);
					TutMode=43;
					break;
				default:
					if (TutQue>0)
					{
						txtTutorial.Activate((pFirstTutText+TutQue)->msg);
						TutMode=TutQue;
						switch (TutQue)
						{
						case 36:
							Tutorial.Stamina=0;
							break;
						case 38:
							Tutorial.Ranged=0;
							break;
						case 39:
							Tutorial.Ammo=0;
							break;
						case 40:
							Tutorial.Orc=0;
							break;
						case 41:
							Tutorial.Magic=0;
							break;
						case 42:
							Tutorial.Morale=0;
							break;
						case 72:
							Tutorial.Heal=0;
							break;
						};
						TutQue=0;
					}
					else
					{
						bTutHint=false;
						objTutorial->iLayer=-1;
						txtTutorial.Hide();
						bEndWarning=false;
					};
				};
			};
		};
		

///Проверка состояния кнопок
///*************************

		if (buttGrid.Check())
		{
			arHumans[arPlayer[ActivePlayer].iColor].bWarGrid=!arHumans[arPlayer[BF_Player[AA]].iColor].bWarGrid;
			ShowGrid(0,7);
			arHumans[arPlayer[ActivePlayer].iColor].Save();
		};

		int ipSpell;


//Вызов заклинания
		if (ActiveSpell>=0)
		{
			if (buttCancelSpell.Check())
			{
				ActiveSpell=-1;
				HideTargets();
				SelectUnit(ActiveUnit);
				buttCancelSpell.Activate(-1);
				buttEndTurn.Activate(1);
				txtSpName.Hide();
				txtSpTarget.Hide();
				txtSpDesc.Hide();				
			};
			if (key[KEY_ALT]!=bAltFlag)
			{
				bAltFlag=key[KEY_ALT];
				for (int A=0; A<2; A++)
				{
					for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
					{
						Squad[A][i].DrawBar();
					};
				};
			};
		}
		else
		{
//Отдых
		if (buttRest.Check())
		{
			ActiveUnit->bResting=true;
			ActiveUnit->iSpeed=0;
			ActiveUnit->bCanAct=0;
			ActiveUnit->DrawBar();
			if (FindNextUnit())
			{SelectUnit(FindNextUnit());}
			else
			{SelectUnit(ActiveUnit);};
		};

//Следующий
		if (buttNext.Check())
		{
			SelectUnit(FindNextUnit());
		};

//Смена оружия
		if (buttChange.Check())
		{
			buttChange.bBlink=false;
			if ((arUseAmmo[AA]>=0)&&(ActiveUnit->GetAmmo()>0))
			{arUseAmmo[AA]=ActiveUnit->iAmmo;};
			if (WindowChangeWeapon(Heroes[AA]))
			{
				bFastDraw=false;				
				if ((ActiveUnit->iAmmo==0)&&(arUseAmmo[AA]<0)&&(ActiveUnit->GetAmmo()>0))
				{
					arUseAmmo[AA]=ActiveUnit->GetAmmo();
				};
				ActiveUnit->iAmmo=arUseAmmo[AA];
				if (ActiveUnit->iAmmo>ActiveUnit->GetAmmo())
				{
					ActiveUnit->iAmmo=ActiveUnit->GetAmmo();					
				};
				if (ActiveUnit->iAmmo<0)
				{ActiveUnit->iAmmo=0;};
				if (ActiveUnit->CheckSkill(980)<=0)
				{
					ActiveUnit->bCanAct=false;
					ActiveUnit->iSpeed=0;
					ActiveUnit->DrawBar();
					if (FindNextUnit())
					{
						SelectUnit(FindNextUnit());
					}
					else
					{SelectUnit(ActiveUnit);};
				}
				else
				{SelectUnit(ActiveUnit);};
			};
		};

//Сбор снарядов
		if (buttGather.Check())
		{
	//		play_sample(wavGather,iSoundVolume,128,1000,0);
			int Found=Random(ActiveUnit->CheckSkill(23)+1);
			ActiveUnit->iAmmo+=Found;
			ActiveUnit->G_Obj->AnimNumber(Found,makecol(250,250,80));
			ActiveUnit->iSpeed=0;
			ActiveUnit->bCanAct=false;
			ActiveUnit->DrawBar();
			if (FindNextUnit())
			{
				SelectUnit(FindNextUnit());
			}
			else
			{SelectUnit(ActiveUnit);};
		};

//Кнопки-переключатели (Лечение, Сокрушающий удар, Двойной выстрел)
		bool bHL=buttHeal.bPress;
		buttHeal.Check();
		if (bHL!=buttHeal.bPress)
		{
			HideTargets();
			if (buttHeal.bPress)
			{
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{
					if ((Squad[AA][i].bInBattle)&&(Squad[AA][i].pUnit)&&(Squad[AA][i].iLife>0)&&(Squad[AA][i].iLife<Squad[AA][i].GetLife())&&
						(Field[Squad[AA][i].x][Squad[AA][i].y].Shoot)&&(Squad[AA][i].GetUnitKind()!=2)&&(Squad[AA][i].GetUnitKind()!=4))
					{objTargetH[Squad[AA][i].x][Squad[AA][i].y]->iLayer=1;};
				};
			};
			if ((ActiveUnit->bCanAct)&&(ActiveUnit->GetRangedAttack()>0)&&(ActiveUnit->GetShootingRange())&&(ActiveUnit->iAmmo))	
			{
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{
					if ((Squad[1-AA][i].bInBattle)&&(Squad[1-AA][i].pUnit)&&(Squad[1-AA][i].iLife>0)&&(Field[Squad[1-AA][i].x][Squad[1-AA][i].y].Shoot>1))
					{objTarget[Squad[1-AA][i].x][Squad[1-AA][i].y]->iLayer=1;};
				};
			};
		};
		buttCrush.Check();
		buttDoubleShot.Check();
		buttRoundAtt.Check();
//		bool bView=buttUnitBar.bPress;
//		buttUnitBar.Check();
		if (key[KEY_ALT]!=bAltFlag)
		{
			bAltFlag=key[KEY_ALT];
			for (int A=0; A<2; A++)
			{
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{
					Squad[A][i].DrawBar();
				};
			};
		};

		
//Маски эффектов
		for (int i=0; i<6; i++)
		{
			if (arEff[i])
			{
				Check=mskEff[i].Check();
				if ((Check==30)||(Check==40))
				{
					if ((arEff[i]->pAbility)&&((arEff[i]->pAbility->arUpg[0][0]==41)||(arEff[i]->pAbility->arUpg[0][0]==78)||(arEff[i]->pAbility->arUpg[0][0]==25)))
					{WindowEffectInfo(arEff[i]->pAbility,arEff[i]->pSpell,arEff[i]->iTime,arEff[i]->iPower);}
					else
					{WindowEffectInfo(arEff[i]->pAbility,arEff[i]->pSpell,arEff[i]->iTime,0);};
				};
			};
		};
//Маски параметров
		for (int i=0; i<13; i++)
		{
			Check=mskParam[i].Check();
			if ((Check==30)||(Check==40))
			{
				switch (i)
				{
				case 0:
					if (ActiveUnit->pUnit->pHero)
					{WindowAbilityInfo(119,ActiveUnit->pUnit->pHero->iLevel);}
					else
					{WindowAbilityInfo(119,ActiveUnit->pUnit->iLevel);};
					break;
				case 1:
					WindowAbilityInfo(i,ActiveUnit->iLife);
					break;
				case 2:
					WindowAbilityInfo(i,ActiveUnit->GetAttack());
					break;
				case 3:
					WindowAbilityInfo(i,ActiveUnit->GetCounterAttack());
					break;
				case 4:
					WindowAbilityInfo(i,ActiveUnit->GetDefence());
					break;
				case 5:
					WindowAbilityInfo(i,ActiveUnit->GetRangedDefence());
					break;
				case 6:
					WindowAbilityInfo(i,ActiveUnit->GetResist());
					break;
				case 7:
					WindowAbilityInfo(i,ActiveUnit->iSpeed);
					break;
				case 8:
					WindowAbilityInfo(i,ActiveUnit->GetRangedAttack());
					break;
				case 9:
					WindowAbilityInfo(i,ActiveUnit->GetShootingRange());
					break;
				case 10:
					WindowAbilityInfo(i,ActiveUnit->iAmmo);
					break;
				case 11:
					WindowAbilityInfo(i,ActiveUnit->iStamina);
					break;
				case 12:
					WindowAbilityInfo(i,ActiveUnit->iMorale);
					break;
				};
			}
			else
			{
				if (Check>1)
				{
					if (i>0)
					{ActiveUnit->MakeParamHint(i);}
					else
					{MakeAbilityHint(119);};
					bHint=true;
				};
			};
		};

//Марш-бросок
		if (buttMarch.Check())
		{
	//		play_sample(wavMarch,iSoundVolume,128,1000,0);
			ActiveUnit->iSpeed++;
			ActiveUnit->UseStamina(ActiveUnit->CheckSkill(29));
			ActiveUnit->bMarch=false;
			ActiveUnit->DrawBar();
			SelectUnit(ActiveUnit);
		};

		
//Трупоедство
		if (buttEat.Check())
		{
			int Corpse=FindCorpse(1-AA,ActiveUnit->x,ActiveUnit->y);
			int CSq=1-AA;
			if (Corpse<0)
			{
				Corpse=FindCorpse(AA,ActiveUnit->x,ActiveUnit->y);
				CSq=AA;
			};
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
			ActiveUnit->G_Obj->AnimNumber(Corpse,makecol(50,250,0));
			ActiveUnit->iSpeed=0;
			ActiveUnit->bCanAct=false;
			ActiveUnit->DrawBar();
			if (FindNextUnit())
			{
				SelectUnit(FindNextUnit());
			}
			else
			{SelectUnit(ActiveUnit);};
		};

//Заклинание		
		if (buttSpell.Check())
		{
			ActiveSpell=Spellbook();
			bMouseReleased=false;
			if (ActiveSpell>-1)
			{
				if ((ActiveUnit->pHero)&&(ActiveUnit->pHero->ipClass1>0))
				{
					pCast=pFirstSpell+Spells[AA][0][ActiveSpell];
					ipSpell=Spells[AA][0][ActiveSpell];
				}
				else
				{
					pCast=pFirstSpell+ActiveSpell;
					ipSpell=ActiveSpell;
				};
				OldX=-1; OldY=-1;
				bSacrifice=pCast->bSacrifice;
				if (bSacrifice)
				{					
					pCast1=pCast;
					pCast=new clSpell;
					pCast->iTarget=1;
					pCast->bOnAlly=true;
					pCast->bOnEnemy=false;
					pCast->arUnitKind[0]=2;
					pCast->arUnitKind[1]=3;
					pCast->arUnitKind[2]=4;
				}
				else
				{
					SpellPrepare(ipSpell);				
				};
			};
		};
		};
		if (Loss(AA)>=0)
		{

		bCursorChange=true;
		bool NeedUse;
		if (!bMouseReleased)
		{
			poll_mouse();
			if (!mouse_b & 1)
			{bMouseReleased=true;};
		};
		int Check, MaskX=-1, MaskY=-1;
		if ((!bTutHint)||(mskTutorial.flag<2))
		{
		for (int i=0; i<8; i++)
		{
		for (int j=0; j<8; j++)
		{	
			Check=Grid[i][j].Check();
	//		switch (Check)
	//		{
			if (Check==30)
			{
//case 30:	//Клик по тайлу		
			NeedUse=true;
			if (!bMouseReleased)
			{
				poll_mouse();
				if (!mouse_b & 1)
				{bMouseReleased=true;};
			};
			if (bMouseReleased)
			{
			if (ActiveSpell>=0)				
			{				
//ЗАКЛИНАНИЕ*******
				switch (pCast->iTarget)
				{
				case 3:
					//Вызов заклинания в тайле
					for (int x=0; x<8; x++){
					for (int y=0; y<8; y++)
					{
						Field[x][y].Land=0;
						Field[x][y].Pass=0;
						Field[x][y].Shoot=0;
					};
					};					
					ShootMap(i,j,-1,pCast->iRadius);
					if ((pCast->bOnAlly)||(pCast->bOnEnemy))
					{
						for (int x=0; x<8; x++)
						{
							for (int y=0; y<8; y++)
							{
								if ((Field[x][y].Shoot>0)||((x==i)&&(y==j)))
								{
									if (pCast->bOnAlly)
									{
										for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
										{
											if ((Squad[AA][k].pUnit)&&(Squad[AA][k].iLife>0)&&(Squad[AA][k].x==x)&&(Squad[AA][k].y==y))
											{
												int AntiSkill=0;
												for (int i=0; i<MAX_ANTI_EFFECT; i++)
												{
													AntiSkill+=pCast->arAntiEffect[i]*Squad[AA][k].CheckSkill(pCast->arAntiEffect[i]);
												};
												for (int i=0; i<MAX_UNIT_KIND; i++)
												{
													if (pCast->arUnitKind[i]==Squad[AA][k].GetUnitKind())
													{AntiSkill=1;};
												};
												if ((!AntiSkill)||(Squad[AA][k].CheckSkill(205)==ipSpell))
												{
										//Найден свой подходящий юнит
													Squad[AA][k].UnderSpell(pCast, ActiveUnit, AA);															
												};
											};
										};
									};
									if (pCast->bOnEnemy)
									{
										for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
										{
											if ((Squad[1-AA][k].pUnit)&&(Squad[1-AA][k].iLife>0)&&(Squad[1-AA][k].x==x)&&(Squad[1-AA][k].y==y))
											{
												int AntiSkill=0;
												for (int i=0; i<MAX_ANTI_EFFECT; i++)
												{
													AntiSkill+=pCast->arAntiEffect[i]*Squad[1-AA][k].CheckSkill(pCast->arAntiEffect[i]);
												};
												for (int i=0; i<MAX_UNIT_KIND; i++)
												{
													if (pCast->arUnitKind[i]==Squad[1-AA][k].GetUnitKind())
													{AntiSkill=1;};
												};
												if ((!AntiSkill)||(Squad[1-AA][k].CheckSkill(205)==ipSpell))
												{
										//Найден вражеский подходящий юнит
													Squad[1-AA][k].UnderSpell(pCast, ActiveUnit, AA);														
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
								if ((Field[x][y].Shoot>0)||((x==i)&&(y==j)))
								{
							//Найден тайл для заклинания
								};
							};
						};
					};
					SpellCast();
					break;
				case 5:
					if ((Field[i][j].Shoot>0)&&(arGrid[i][j].Pass)&&((pFirstBF_Obj+arGrid[i][j].Land)->iMoveCost>0))
					{
					//Вызов заклинания в свободном тайле
						TileSpell(pCast,i,j);
						SpellCast();
					};
					break;
				case 2:
				case 4:
					if (pCast->bOnAlly)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[AA][k].pUnit)&&(Squad[AA][k].iLife==0)&&(Squad[AA][k].bInBattle)&&(Squad[AA][k].x==i)&&(Squad[AA][k].y==j))
							{
								if ((pCast->iTarget==2)||(arGrid[i][j].Pass))
								{
									//Найден свой подходящий труп
									CorpseSpell(pCast,&Squad[AA][k]);
									SpellCast();					
									break;
								}
							}
						}
					};
					if (pCast->bOnEnemy)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[1-AA][k].pUnit)&&(Squad[1-AA][k].iLife==0)&&(Squad[1-AA][k].bInBattle)&&(Squad[1-AA][k].x==i)&&(Squad[1-AA][k].y==j))
							{
								if ((pCast->iTarget==2)||(arGrid[i][j].Pass))
								{
									//Найден вражеский подходящий труп
									CorpseSpell(pCast,&Squad[1-AA][k]);
									SpellCast();
									break;
								}
							}
						}
					};
					break;
				case 1:
					if (pCast->bOnAlly)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[AA][k].iLife>0)&&(Squad[AA][k].x==i)&&(Squad[AA][k].y==j))
								
							//	((pCast->arAntiEffect[0]==0)||(Squad[AA][k].CheckSkill(pCast->arAntiEffect[1]==0)))&&
							//	((pCast->arAntiEffect[0]==0)||(Squad[AA][k].CheckSkill(pCast->arAntiEffect[2]==0))))
							{
								int AntiSkill=0;
								for (int i=0; i<MAX_ANTI_EFFECT; i++)
								{
									AntiSkill+=pCast->arAntiEffect[i]*Squad[AA][k].CheckSkill(pCast->arAntiEffect[i]);
								};
								for (int i=0; i<MAX_UNIT_KIND; i++)
								{
									if (pCast->arUnitKind[i]==Squad[AA][k].GetUnitKind())
									{AntiSkill=1;};
								};
								if ((!AntiSkill)||(Squad[AA][k].CheckSkill(205)==ipSpell))
								{
							//Найден свой подходящий юнит
									if (bSacrifice)
									{
										//Жертва
										iSacVictim=k;
										bSacrifice=false;
										delete pCast;
										pCast=pCast1;
										txtSpTarget.Activate((pFirstPhrase+254)->str);
										SpellPrepare(ipSpell);										
									}
									else
									{
										if (pCast->arEffect[0][0]==89)										
										{
											//Телепорт
											Teleport=*pCast;
											Teleport.iTarget=5;
											pCast=&Teleport;
											pTeleportUnit=&Squad[AA][k];
										}
										else
										{
											//Обычное заклинание
											Squad[AA][k].UnderSpell(pCast, ActiveUnit, AA);
											SpellCast();
										};
									};
								};
							}
						}
					};
					if (pCast->bOnEnemy)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[1-AA][k].iLife>0)&&(Squad[1-AA][k].x==i)&&(Squad[1-AA][k].y==j))
							{	
								int AntiSkill=0;
								for (int i=0; i<MAX_ANTI_EFFECT; i++)
								{
									AntiSkill+=pCast->arAntiEffect[i]*Squad[1-AA][k].CheckSkill(pCast->arAntiEffect[i]);
								};
								for (int i=0; i<MAX_UNIT_KIND; i++)
								{
									if (pCast->arUnitKind[i]==Squad[1-AA][k].GetUnitKind())
									{AntiSkill=1;};
								};
								if ((!AntiSkill)||(Squad[1-AA][k].CheckSkill(205)==ipSpell))
								{
								//Найден вражеский подходящий юнит
									if (pCast->arEffect[0][0]==89)										
									{		
										//Телепорт
										if (ActiveUnit->CheckSkill(908)>=Squad[1-AA][k].GetResist())
										{
											Teleport=*pCast;
											Teleport.iTarget=5;
											pCast=&Teleport;
											pTeleportUnit=&Squad[1-AA][k];
										}
										else
										{
											Squad[1-AA][k].G_Obj->AnimText(4, makecol(250,150,20));
											SpellCast();
										};
									}
									else
									{
										Squad[1-AA][k].UnderSpell(pCast, ActiveUnit, AA);
										SpellCast();
									};
								};

							};
						};
					};
					break;					
				}
			}
			else
			{

			if (ActiveUnit->bCanAct)
			{
				for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
				{
					if ((Squad[1-AA][k].iLife)&&(Squad[1-AA][k].x==i)&&(Squad[1-AA][k].y==j))
					{
////ВЫСТРЕЛ*********
						if ((ActiveUnit->bCanAct)&&(ActiveUnit->iAmmo)&&(Field[i][j].Shoot>1)&&(!key[KEY_LCONTROL])&&(!key[KEY_RCONTROL])
							&&((ActiveUnit->iSpeed>0)||(ActiveUnit->CheckSkill(88)))&&(ActiveUnit->iStamina>0)&&(ActiveUnit->GetRangedAttack()>0))
						{					
							ActiveUnit->Shoot(&Squad[1-AA][k]);												
							if (FindNextUnit())
							{SelectUnit(FindNextUnit());}
							else
							{SelectUnit(ActiveUnit);};								
							NeedUse=false;
						}
						else
						{
//Поиск тайла для атаки*****
							if (!ActiveUnit->CheckSkill(38))
							{
							int goX=8, goY=8, Choose=0;
							long minDistance=1000000;

							if (CanAttack(i-1,j))
							{
								minDistance=CalcDistance(i-1,j);							
								goX=i-1;
								goY=j;
								Choose=6;
							};
							if (CanAttack(i-1+j%2,j-1))
							{
								if (minDistance>CalcDistance(i-1+j%2,j-1))
								{
									minDistance=CalcDistance(i-1+j%2,j-1);
									goX=i-1+j%2;
									goY=j-1;
									Choose=7;
								};
							};
							if (CanAttack(i+j%2,j-1))
							{
								if (minDistance>CalcDistance(i+j%2,j-1))
								{
									minDistance=CalcDistance(i+j%2,j-1);
									goX=i+j%2;
									goY=j-1;
									Choose=8;
								};
							};
							if (CanAttack(i+1,j))
							{
								if (minDistance>CalcDistance(i+1,j))
								{
									minDistance=CalcDistance(i+1,j);
									goX=i+1;
									goY=j;
									Choose=9;
								};
							};
							if (CanAttack(i+j%2,j+1))
							{
								if (minDistance>CalcDistance(i+j%2,j+1))
								{
									minDistance=CalcDistance(i+j%2,j+1);
									goX=i+j%2;
									goY=j+1;
									Choose=10;
								};
							};
							if (CanAttack(i-1+j%2,j+1))
							{
								if (minDistance>CalcDistance(i-1+j%2,j+1))
								{
									minDistance=CalcDistance(i-1+j%2,j+1);
									goX=i-1+j%2;
									goY=j+1;
									Choose=11;
								};
							};

///АТАКА*************
							if (goX<8)
							{
								if (ActiveUnit->Attack(goX,goY,&Squad[1-AA][k]))
								{											
									if (FindNextUnit())
									{SelectUnit(FindNextUnit());}
									else
									{SelectUnit(ActiveUnit);};
								}
								else
								{
									if (FindNextUnit())
									{
										SelectUnit(FindNextUnit());
									}
									else
									{
										int Select=Loss(AA);
										if (Select>=0)
										{
											SelectUnit(&Squad[AA][Select]);
										}
										else
										{
							
										};
									};
								};
							}
							else
							{
							//Клик по врагу на недоступном тайле							
							};							
							NeedUse=false;
							};
							};
						};
					};
				};
				if (NeedUse)
				{
					for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
					{
						if ((Squad[AA][k].iLife)&&(Squad[AA][k].x==i)&&(Squad[AA][k].y==j))
						{
//ЛЕЧЕНИЕ*****************
							if (buttHeal.Check())
							{
								NeedUse=false;
								if ((Field[i][j].Shoot)&&(Squad[AA][k].iLife<Squad[AA][k].GetLife())
									&&(Squad[AA][k].GetUnitKind()!=2)&&(Squad[AA][k].GetUnitKind()!=4))
								{
									ActiveUnit->Heal(&Squad[AA][k]);
									if (FindNextUnit())
										{SelectUnit(FindNextUnit());}
									else
										{SelectUnit(ActiveUnit);};
								};
							}
							else
//ВЫБОР ЮНИТА*******
							{
								SelectUnit(&Squad[AA][k]);
								NeedUse=false;
							};
						}
					};
				};
				if (ActiveUnit->bCanAct)
				{
					if (NeedUse)
					{
//ДВИЖЕНИЕ***********
						if (Field[i][j].Pass)
						{
							ActiveUnit->Move(i,j);
							if (FindNextUnit()&&(!ActiveUnit->bCanAct))
								{SelectUnit(FindNextUnit());}
							else
								{SelectUnit(ActiveUnit);};							
						};
					};
				};
				};
				};
		//		break;
		};
		if (Check==40)
		{
//	case 40:
		//Инфо-кнопка
//		{
			objMain=ShowActiveObjects(objMain,10,false);
			bool bTerra=true;
		for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
		{
			if ((Squad[AA][k].pUnit)&&(Squad[AA][k].iLife>0)&&(Squad[AA][k].x==i)&&(Squad[AA][k].y==j))
			{
				Squad[AA][k].ShowInfoScreen();
				bTerra=false;
			};
			if ((Squad[1-AA][k].pUnit)&&(Squad[1-AA][k].iLife>0)&&(Squad[1-AA][k].x==i)&&(Squad[1-AA][k].y==j))
			{
				Squad[1-AA][k].ShowInfoScreen();
				bTerra=false;
			};
		};
		if (bTerra)
		{
			if ((bCastle)&&(i>5))
			{WindowInfo((pFirstPhrase+435)->str,(pFirstMessage+42)->msg,bmpStuff[49]);}
			else
			{
				switch (arGrid[i][j].Land)
				{
				case 1:
				case 2:
				case 3:
				case 4:
					WindowInfo((pFirstTerrain+arGrid[i][j].Land)->sName,(pFirstMessage+37+arGrid[i][j].Land)->msg,bmpStuff[42+arGrid[i][j].Land]);
					break;
				case 10:
					WindowInfo((pFirstPhrase+436)->str,(pFirstMessage+43)->msg,bmpStuff[48]);
					break;
				default:
					WindowInfo((pFirstPhrase+434)->str,(pFirstMessage+35)->msg,bmpStuff[47]);
					break;
				};
			};
		};
		};
	//	break;
						

		if (Check>1)
//	case 2:					
//	case 3:			//Курсор над тайлом
		{
			NeedUse=true;
			for (int Side=0; Side<2; Side++)
			{
				for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
				{
					if (((key[KEY_ALT])||(key[KEY_ALTGR]))&&(Squad[1-Side][k].iLife)&&(Squad[1-Side][k].x==i)&&(Squad[1-Side][k].y==j)&&
						((ActiveUnit!=&Squad[1-Side][k])||(ActiveSpell>=0)))
					{
						if ((Side*100+k)!=iTarget)
						{
							ShowEnemy(&Squad[1-Side][k]);
							iTarget=Side*100+k;
						};
						objSideBar->iLayer=4;
					};
				}
			}
			if (ActiveSpell>=0)
			{
				mouse_pointer=16;
				switch (pCast->iTarget)
				{					
				case 3:
					//Вызов заклинания в тайле
					set_mouse_sprite(bmpMousePointer[15]);
					mouse_pointer=15;
					if ((!bSacrifice)&&(pCast->iRadius>0)&&((OldX!=i)||(OldY!=j)))
					{
						HideTargets();
						for (int x=0; x<8; x++){
						for (int y=0; y<8; y++)
						{
							Field[x][y].Land=0;
							Field[x][y].Pass=0;
							Field[x][y].Shoot=0;
						};
						};					
						ShootMap(i,j,-1,pCast->iRadius);
						for (int x=0; x<8; x++){
						for (int y=0; y<8; y++)
						{
							if (Field[x][y].Shoot>0)
							{objTarget[x][y]->iLayer=1;};							
						};
						};	
						OldX=i; OldY=j;
					};

					break;
				case 5:
					if ((Field[i][j].Shoot>0)&&(arGrid[i][j].Pass)&&((pFirstBF_Obj+arGrid[i][j].Land)->iMoveCost>0))
					{
						//Вызов заклинания в свободном тайле
						if(mouse_pointer!=15)
						{
							set_mouse_sprite(bmpMousePointer[15]);
							mouse_pointer=15;
						};					
					};
					break;
				case 2:
				case 4:
					if (pCast->bOnAlly)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[AA][k].pUnit)&&(Squad[AA][k].iLife==0)&&(Squad[AA][k].bInBattle)&&(Squad[AA][k].x==i)&&(Squad[AA][k].y==j))
							{
								if ((pCast->iTarget==2)||(arGrid[i][j].Pass))
								{
									//Найден свой подходящий труп
									if(mouse_pointer!=15)
									{
										set_mouse_sprite(bmpMousePointer[15]);
										mouse_pointer=15;
									};					
								}
							}
						}
					};
					if (pCast->bOnEnemy)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[1-AA][k].pUnit)&&(Squad[1-AA][k].iLife==0)&&(Squad[1-AA][k].bInBattle)&&(Squad[1-AA][k].x==i)&&(Squad[1-AA][k].y==j))
							{
								if ((pCast->iTarget==2)||(arGrid[i][j].Pass))
								{
									//Найден вражеский подходящий труп
									if(mouse_pointer!=15)
									{
										set_mouse_sprite(bmpMousePointer[15]);
										mouse_pointer=15;
									};					
								}
							}
						}
					};
					break;
				case 1:
					if (pCast->bOnAlly)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[AA][k].iLife>0)&&(Squad[AA][k].x==i)&&(Squad[AA][k].y==j))
							{
								int AntiSkill=0;
								for (int i=0; i<MAX_ANTI_EFFECT; i++)
								{
									AntiSkill+=pCast->arAntiEffect[i]*Squad[AA][k].CheckSkill(pCast->arAntiEffect[i]);
								};
								for (int i=0; i<MAX_UNIT_KIND; i++)
								{
									if (pCast->arUnitKind[i]==Squad[AA][k].GetUnitKind())
									{AntiSkill=1;};
								};
								if ((!AntiSkill)||(Squad[AA][k].CheckSkill(205)==ipSpell))
								{
							//Найден свой подходящий юнит
								if(mouse_pointer!=15)
								{
									set_mouse_sprite(bmpMousePointer[15]);
									mouse_pointer=15;
								};					
								}
							}
						}
					};
					if (pCast->bOnEnemy)
					{
						for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
						{
							if ((Squad[1-AA][k].iLife>0)&&(Squad[1-AA][k].x==i)&&(Squad[1-AA][k].y==j))
							{								
								int AntiSkill=0;
								for (int i=0; i<MAX_ANTI_EFFECT; i++)
								{
									AntiSkill+=pCast->arAntiEffect[i]*Squad[1-AA][k].CheckSkill(pCast->arAntiEffect[i]);
								};
								for (int i=0; i<MAX_UNIT_KIND; i++)
								{
									if (pCast->arUnitKind[i]==Squad[1-AA][k].GetUnitKind())
									{AntiSkill=1;};
								};
								if ((!AntiSkill)||(Squad[1-AA][k].CheckSkill(205)==ipSpell))
								{
								//Найден вражеский подходящий юнит
								if(mouse_pointer!=15)
								{
									set_mouse_sprite(bmpMousePointer[15]);
									mouse_pointer=15;
								};
								};
							};
						};
					};
					break;				
				};
				if(mouse_pointer==16)
				{
					set_mouse_sprite(bmpMousePointer[16]);
				};					
				bCursorChange=false;
			}
			else
			{
				if (ActiveUnit->bCanAct)
				{
					bool bMelee=true;
					for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
					{
						if ((Squad[1-AA][k].iLife)&&(Squad[1-AA][k].x==i)&&(Squad[1-AA][k].y==j))
						{
							if ((ActiveUnit->iAmmo)&&(!key[KEY_LCONTROL])&&(!key[KEY_RCONTROL])&&
								(Field[i][j].Shoot!=1)&&(ActiveUnit->GetRangedAttack()))
							{
								bMelee=false;
								if ((Field[i][j].Shoot>1)&&((ActiveUnit->iSpeed>0)||(ActiveUnit->CheckSkill(88)))&&(ActiveUnit->iStamina>0))
								{
									if(mouse_pointer!=4)
									{
										set_mouse_sprite(bmpMousePointer[4]);
										mouse_pointer=4;
									};
								}
								else
								{
									if(mouse_pointer!=5)
									{
										set_mouse_sprite(bmpMousePointer[5]);
										mouse_pointer=5;
										bMelee=true;
									};
								};
								bCursorChange=false;
								NeedUse=false;
							}
							if (bMelee)
							{
							bool bAttack=false;
							int goX=8, goY=8, Choose=0;
							long minDistance=1000000;

							if (CanAttack(i-1,j))
							{
								minDistance=CalcDistance(i-1,j);							
								goX=i-1;
								goY=j;
								Choose=6;
							};
							if (CanAttack(i-1+j%2,j-1))
							{
								if (minDistance>CalcDistance(i-1+j%2,j-1))
								{
									minDistance=CalcDistance(i-1+j%2,j-1);
									goX=i-1+j%2;
									goY=j-1;
									Choose=7;
								};
							};
							if (CanAttack(i+j%2,j-1))
							{
								if (minDistance>CalcDistance(i+j%2,j-1))
								{
									minDistance=CalcDistance(i+j%2,j-1);
									goX=i+j%2;
									goY=j-1;
									Choose=8;
								};
							};
							if (CanAttack(i+1,j))
							{
								if (minDistance>CalcDistance(i+1,j))
								{
									minDistance=CalcDistance(i+1,j);
									goX=i+1;
									goY=j;
									Choose=9;
								};
							};
							if (CanAttack(i+j%2,j+1))
							{
								if (minDistance>CalcDistance(i+j%2,j+1))
								{
									minDistance=CalcDistance(i+j%2,j+1);
									goX=i+j%2;
									goY=j+1;
									Choose=10;
								};
							};
							if (CanAttack(i-1+j%2,j+1))
							{
								if (minDistance>CalcDistance(i-1+j%2,j+1))
								{
									minDistance=CalcDistance(i-1+j%2,j+1);
									goX=i-1+j%2;
									goY=j+1;
									Choose=11;
								};
							};
							if ((Choose)&&(!ActiveUnit->CheckSkill(38)))
							{
								if (mouse_pointer!=Choose)
								{
									mouse_pointer=Choose;
									set_mouse_sprite(bmpMousePointer[Choose]);
								};			
								MaskX=goX;
								MaskY=goY;
							}
							else
							{
								if ((mouse_pointer!=12)&&(mouse_pointer!=5))
								{
									set_mouse_sprite(bmpMousePointer[12]);
									mouse_pointer=12;
								};
							};
							bCursorChange=false;
							NeedUse=false;
							};
						};
					};
				};
				if (NeedUse)
				{
					for (int k=0; k<MAX_SQUAD+MAX_SUMMONS; k++)
					{
						if ((Squad[AA][k].iLife)&&(Squad[AA][k].x==i)&&(Squad[AA][k].y==j))
						{
							if (buttHeal.Check())
							{
								if ((Field[i][j].Shoot)&&(Squad[AA][k].iLife<Squad[AA][k].GetLife())
									&&(Squad[AA][k].GetUnitKind()!=2)&&(Squad[AA][k].GetUnitKind()!=4))
								{
									if(mouse_pointer!=13)
									{
										set_mouse_sprite(bmpMousePointer[13]);
										mouse_pointer=13;
									};
									
								}
								else
								{
									if(mouse_pointer!=14)
									{
										set_mouse_sprite(bmpMousePointer[14]);
										mouse_pointer=14;
									};
								};

							}
							else
							{
								if(mouse_pointer!=1)
								{
									set_mouse_sprite(bmpMousePointer[1]);
									mouse_pointer=1;
								};								
							};
							bCursorChange=false;
							NeedUse=false;
						};
					};				
				};
				if ((ActiveUnit->bCanAct)&&NeedUse)
				{
					if (Field[i][j].Pass)
					{
						if(mouse_pointer!=2)
						{
							set_mouse_sprite(bmpMousePointer[2]);
							mouse_pointer=2;
						};
					}
					else
					{
						if(mouse_pointer!=3)
						{
							set_mouse_sprite(bmpMousePointer[3]);
							mouse_pointer=3;
						};
					};
					bCursorChange=false;
					NeedUse=false;
				};
				};
			//	break;				
			};
		};
		};
		}; //Tutorial
		if (MaskX>=0)
		{
			Grid[MaskX][MaskY].Show();
		};
		if (bCursorChange&&(mouse_pointer>0))
		{
			set_mouse_sprite(bmpMousePointer[0]);
			mouse_pointer=0;
		};
		if (ActiveSpell<0)
		{
		//Конец хода
			if (buttEndTurn.flag==0)
			{buttEndTurn.Activate(1);};
			if (buttEndTurn.Check())
			{
				ActiveUnit->G_Obj->RemoveCargo(objSelect);
				EndTurn(true, true);
				buttEndTurn.Activate(1);
			}
			else
			{
				bool bEnd=true;
				for (int i=0; ((bEnd)&&(i<MAX_SQUAD+MAX_SUMMONS)); i++)
				{
					if ((Squad[AA][i].pUnit)&&(Squad[AA][i].bInBattle)&&(Squad[AA][i].iLife>0)&&(Squad[AA][i].bCanAct))
					{bEnd=false;};
				};
				if (bEnd)
				{
					if ((arHumans[arPlayer[ActivePlayer].iColor].bWarEndTurn)&&((!IsPlayed[1-AA])||(CurGame.iGameType!=3)))
					{
						ActiveUnit->G_Obj->RemoveCargo(objSelect);
						EndTurn(true, true);
						buttEndTurn.Activate(1);						
					}
					else
					{
						buttEndTurn.bBlink=true;
					};
				};
			};
			if (buttMenu.Check())
			{
			bool bMenuCall=true;
			if ((BF_Cheat)&&(key[KEY_U]))
			{
				bMenuCall=false;
				int Unit=CheatAddUnit();
				if (Unit>0)
				{
					pCast=new clSpell;
					pCast->iTarget=5;
					pCast->arEffect[0][0]=1000+Unit;
					pCast->arEffect[1][0]=0;
					pCast->arEffect[2][0]=-1;
					pCast->iArea=1;
					pCast->iCost=0;
					pCast->iLifeCost=0;
					pCast->iStamCost=0;
					pCast->iKarma=0;
					pCast->iPowerMod=0;
					pCast->wavCast=pFirstSpell->wavCast;
					for (int i=0; i<8; i++){
					for (int j=0; j<8; j++)
					{			
						Field[i][j].Shoot=1;
					};
					};
					ActiveSpell=30000;
				};
			}
			if ((BF_Cheat)&&(key[KEY_S]))
			{
				bMenuCall=false;
				int Spell=CheatAddSpell();
				if (Spell>0)
				{
					Spells[AA][0][0]=Spell;
					Spells[AA][1][0]=1;					
				};
			}

			if (bMenuCall)
			{
			buttMenu.bBlink=false;
			bool bEnd=false;
			switch (WarMenu(AA))
			{
			case 1:
				NewProgress=0;
				OldProgress=0;
				RunHourGlass(150, 0);
				bAutoCombat=true;
				bShowBattle=false;
				bShowAnim=false;
				ActiveUnit->G_Obj->RemoveCargo(objSelect);
				bEnd=true;				
				break;
			case 2:
				if (IsPlayed[1-AA])
				{IsPlayed[AA]=false;}
				else
				{bAutoCombat=true;};
				ActiveUnit->G_Obj->RemoveCargo(objSelect);
				bEnd=true;				
				break;
			case 3:
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{Squad[AA][i].iLife=0;};
				bEnd=true;
				break;
			};
			ShowGrid(0,7);
			for (int A=0; A<2; A++)
			{
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{
					Squad[A][i].DrawBar();
				};
			};
			if (bEnd)
			{EndTurn(false,false);};
			};
			};

			if ((key[KEY_F10])&&(!IsPlayed[1-AA]))
			{
				if ((key[KEY_LCONTROL])||(key[KEY_RCONTROL]))
				{SideCanCast[AA]=(pFirstDialog+1460)->Show(1,1,1,0,NULL,NULL)-1;};
				NewProgress=0;
				OldProgress=0;
				RunHourGlass(150, 0);
				bAutoCombat=true;
				bShowBattle=false;
				bShowAnim=false;
				ActiveUnit->G_Obj->RemoveCargo(objSelect);
				EndTurn(false, false);
			};

			if (key[KEY_F11])
			{
				if ((key[KEY_LCONTROL])||(key[KEY_RCONTROL]))
				{SideCanCast[AA]=(pFirstDialog+1460)->Show(1,1,1,0,NULL,NULL)-1;};
				if (IsPlayed[1-AA])
				{IsPlayed[AA]=false;}
				else
				{bAutoCombat=true;};
				ActiveUnit->G_Obj->RemoveCargo(objSelect);
				EndTurn(false, false);
			};
		};
		}; //Loss
		if ((!bEndWarning)&&(bTutHint)&&(!Tutorial.Mode))
		{
			objTutorial->iLayer=-1;
			txtTutorial.Hide();
			bTutHint=false;
		};
//		if ()
//		{objTutorial->iLayer=-1;;}		
		if (bShowBattle)
		{
			BF_RefreshScreen();
		};
//		textprintf_ex(screen,Font1,0,0,makecol(220,220,50),-1,"%d",Squad[1][0].iLife);	
	};
	bTrueAnimation=true;
	int Delay=500;
	while ((bTrueAnimation)&&(bShowBattle)&&(Delay>0)&&(FearVictory==0))
	{
		if (Timer)
		{
			BF_RefreshScreen();
			Delay--;
		};
	};
	int Result=0;
	if ((!bSimulate)
	//	&&(!BF_Cheat)
		)
	{
		if (FearVictory>0)
		{
			/*if (FearVictory==1)
			{
				arPlayer[BF_Player[AA]].iKarma-=2;
				for (int i=0; i<MAX_SQUAD+MAX_SUMMONS; i++)
				{				
					if (Squad[1-AA][i].pUnit)
					{Squad[1-AA][i].iLife=0;};
				};
			}
			else*/
			{arPlayer[BF_Player[AA]].iKarma+=Random(3);};
			EndBattle(AA, Site);
			Result=1-AA;
		}
		else
		{

		if ((Loss(0)<0)&&(Loss(1)>=0))
		{		
			EndBattle(1, Site);
			Result=0;		
		};

		if ((Loss(0)>=0)&&(Loss(1)<0))
		{
			EndBattle(0, Site);
			Result=1;		
		};

		if ((Loss(0)<0)&&(Loss(1)<0))
		{
			EndBattle(-1, Site);
			Result=-1;		
		};

		if (iTurn/2>MAX_BATTLE_DURATION)
		{
			if (bShowBattle)
			{(pFirstDialog+1093)->Show(1,1,0,0,NULL,NULL);};
			EndBattle(-1, Site);			
			Result=-2;
		};
		};
	};
	
	G_Object * Temp;
	while (objMain)
	{
		Temp=objMain->pNext;		
		delete objMain;
		objMain=Temp;
	};

	destroy_bitmap(Background);	
	destroy_bitmap(objPanel);
	delete objSelect;
	if (TowerShoot>0)
	{
		delete unTower;
//		delete objTower;
	};
	for (int i=0; i<MagTower; i++)
	{
		delete unMagTower[i];
	};
	bShowAnim=true;
	/*if (arPlayer[3].pEvent)
	{
		textprintf(screen,Font1,0,120,COLOR_2,"Player 3 check: %d",arPlayer[3].pEvent->Hero2->ipClass1);
		clear_keybuf();
		readkey();
	};	*/
	if (iGameMode!=2)
	{
		iGameMode=2;
		ChangeMusic();
	};
	return Result;
};


int SimulateBattle(dcProvince *Prov1, dcProvince *Prov2, dcHero *Hero1, dcHero *Hero2, int Site)
{	
	bSimulate=true;
	int Result=InitBattle(Prov1, Prov2, Hero1, Hero2, Site, false);
	bSimulate=false;
	for (int i=0; i<2; i++)
	{arPlayer[BF_Player[i]].Gem+=GemSpent[i];};
	int Power[2], Power1[2];
	for (int i=0; i<2; i++)
	{
		Power[i]=1;
		Power1[i]=1;
		for (int j=0; j<MAX_SQUAD; j++)
		{
			if (Heroes[i]->pSquad[j])
			{
				int Price=0;
				if (Heroes[i]->pSquad[j]->pHero)
				{Price=10;}
				else
				{Price=Heroes[i]->pSquad[j]->pType()->iLevel;};
				if (Heroes[i]->pSquad[j]->iLife>0)
				{Power[i]+=Price;};
				if (Squad[i][j].iLife>0)
				{Power1[i]+=Price;};
				if (Squad[i][j].bPacked)
				{
					delete Squad[i][j].pTrueForm;
				};
			};
		};
	};
	Result=100+100*Power1[0]/Power[0]-100*Power1[1]/Power[1];
	iAntisleep=iAntisleepDelay;
	return Result;
};