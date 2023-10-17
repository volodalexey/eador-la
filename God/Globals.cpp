//	Added by Grey 4.09.2007
//	Source file - realisation of GLOBAL:  
//	Variables,  Functions,  etc.
#include "Globals.h"

// Global Main Variables


// Screen Output Services
// Colors && Fonts

int COLOR_BLACK	= makecol(0,0,0);	//Main color Set //grey
int COLOR_DKGREY= makecol(128,128,128);//grey
int COLOR_GREY	= makecol(192,192,192);//grey
int COLOR_WHITE	= makecol(255,255,255);//grey	
int COLOR_RED	= makecol(0,0,220); //Rainbow color Set //grey
int COLOR_ORANGE= makecol(250,150,0);//grey
int COLOR_YELLOW= makecol(220,220,0);//grey
int COLOR_GREEN = makecol(0,128,0);//grey
int COLOR_CYAN	= makecol(0,220,255);//grey
int COLOR_BLUE	= makecol(0,0,220);//grey
int COLOR_VI0LET= makecol(128,0,128);//grey
int COLOR_BROWN = makecol(175,100,0);//grey
int NullColor;

long int NewProgress, OldProgress;
bool bMessageFor;
int iMessageFor;
bool bDemo=false;
int iMaxHerbSign=26;
int iMaxAvatarPic=21;

BITMAP	*bmpGUI_SpellMask, 
		*bmpGUI_SpellSelect,
		*bmpGUI_HeroTrade, 
		*bmpGUI_HeroScreen, 
		*bmpGUI_SpellShadow,
		*bmpGUI_UnitSelect, 
		*bmpGUI_UnitMask, 
		*bmpGUI_UnitClosed,
		*bmpGUI_ButtTrashNorm, 
		*bmpGUI_ButtTrashAct, 
		*bmpGUI_ButtTrashInact,
		*bmpGUI_DiffWindow, 
		*bmpGUI_EndCamp, 
		*bmpGUI_ButtExitPress,
		*bmpGUI_Pointer,
		*bmpGUI_UnitLvlUp,
		*bmpGUI_ExpLow, 
		*bmpGUI_ResourceInfo,
		*bmpGUI_HealthLow, 
		*bmpGUI_DefenderInfo, 
		*bmpGUI_HeroLvlUp, 
		*bmpGUI_HeroLvl10, 
		*bmpGUI_Hint, 
		*bmpGUI_HeroAdvice, 
		*bmpGUI_RulerBar,
		*bmpGUI_SpellClosed,
		*bmpGUI_UnitInfo,
		*bmpGUI_SpellInfo,
		*bmpGUI_Garrison,
		*bmpGUI_RecruitClosed,
		*bmpGUI_NoRes,
		*bmpGUI_Library,
		*bmpGUI_SpellTrade,
		*bmpGUI_Shop,
		*bmpGUI_ItemTrade,
		*bmpGUI_ItemInfo,
		*bmpGUI_Victory,
		*bmpGUI_Lose, 
		*bmpGUI_Dead,
		*bmpGUI_LowPanel,
		*bmpGUI_ProvinceBar,
		*bmpGUI_BuildBar, 
		*bmpGUI_BuildPanel, 
		*bmpGUI_BuildSelect,
		*bmpGUI_BuildMask,
		*bmpGUI_Building,
		*bmpGUI_RazePanel,
		*bmpGUI_Defender,
		*bmpGUI_Site,
		*bmpGUI_Fort,
		*bmpGUI_Castle,
		*bmpGUI_HeroBar,
		*bmpGUI_OuterBuildInfo,
		*bmpGUI_Treasure,
		*bmpGUI_HeroRecruit, 
		*bmpGUI_HeroRecruitWindow, 
		*bmpGUI_HeroResurrectWindow,
		*bmpGUI_HeroDead,
		*bmpGUI_Profile,
		*bmpGUI_EndGameScreen,
		*bmpGUI_HighScore,
		*bmpGUI_UnitTypeInfo,
		*bmpGUI_Statistics,
		*bmpGUI_UnitReward,
		*bmpGUI_DefChoose,
		*bmpGUI_TakeLoot,
		*bmpGUI_Necro,
		*bmpGUI_StrHint,
		*bmpGUI_ItemEffect,
		*bmpGUI_SkillInfo,
		*bmpGUI_BattleUnitInfo,
		*bmpGUI_AbilityInfo,
		*bmpGUI_Gold,
		*bmpGUI_Crystal,
		*bmpGUI_ItemSet,
		*bmpGUI_NewTurn,
		*bmpGUI_Screen01,
		*bmpGUI_Diplomacy,
		*bmpGUI_Cursor,
		*bmpGUI_HeroPort, 
		*bmpGUI_MainMenu, 
		*bmpGUI_Eador, 
		*bmpGUI_MenuScrN, 
		*bmpGUI_MenuScrA,
		*bmpGUI_NewGame, 
		*bmpGUI_NGPlayer,
		*bmpGUI_Build,
		*bmpGUI_Built,
		*bmpGUI_NoGold,
		*bmpGUI_NoGem,
		*bmpGUI_InnBldHint,
		*bmpGUI_InnBldInfo, 
		*bmpGUI_UnitSlot, 
		*bmpGUI_SpellSlot, 
		*bmpGUI_String,
		*bmpGUI_GroupIcon, 
		*bmpGUI_GroupHint,
		*bmpGUI_Planned,
		*bmpGUI_AstralLow, 
		*bmpGUI_ShardBar,
		*bmpGUI_ItemHint, 
		*bmpGUI_HeroSelect[4],
		*bmpGUI_DeadSelect,
		*bmpGUI_Treasury,
		*bmpGUI_HomeGarrison, 
		*bmpGUI_HomeBarracks,
		*bmpGUI_HomeLibrary,
		*bmpGUI_ProgressMark, 
		*bmpGUI_ProgressFill,
		*bmpGUI_MainShardBar,
		*bmpGUI_SpellGlow,
		*bmpGUI_UnitGlow,
		*bmpGUI_EnterWindow,
		*bmpGUI_HerbWindow, 
		*bmpGUI_PickBonus, 
		*bmpGUI_PickUnit, 
		*bmpGUI_PickSpell,
		*bmpGUI_CampVictory,
		*bmpGUI_ChoosePlayer,
		*bmpGUI_ScrollN,
		*bmpGUI_ScrollA,
		*bmpGUI_Alchemy,
		*bmpGUI_MapSettings,
		*bmpGUI_Broken, 
		*bmpGUI_Anvil1, 
		*bmpGUI_Anvil2,
		*bmpGUI_Menu,
		*bmpGUI_Settings,
		*bmpGUI_TurnWindow,
		*bmpGUI_ChangeWeapon,
		*bmpGUI_SmallHint,
		*bmpGUI_CampStat,
		*bmpGUI_ParamHint,
		*bmpGUI_Credits;

BITMAP *bmpAnName[16];
BITMAP *bmpAnCrea[15];
BITMAP *bmpAnWeb[7];
BITMAP *bmpAnRoot[8];
BITMAP *bmpSkull;
BITMAP *bmpJaw;
BITMAP *bmpCloud[8];

//SAMPLE *wavClick;
//SAMPLE *wavCoins;
//SAMPLE *wavParchment;
//SAMPLE *wavBuild;
//SAMPLE *wavDelete;

//OutputStreamPtr wavTest;

// Progress Bar  && Wait(KEY_ESC)
int	 SCR_PROGRESSBAR_X = 10;//grey
int	 SCR_PROGRESSBAR_Y = 740;//grey
int	 SCR_PROGRESSBAR_CURX = 0;//grey
int	 SCR_PROGRESSBAR_CURY = 0;//grey
int	 SCR_PROGRESSBAR_STEP = 12;//grey

void PROCESSBAR(int x,int step /*=100*/){//grey
	int st=SCR_PROGRESSBAR_STEP;
	if(x==0) {
		SCR_PROGRESSBAR_CURX = SCR_PROGRESSBAR_X;
		SCR_PROGRESSBAR_CURY = SCR_PROGRESSBAR_Y;
		//textprintf_centre_ex(screen,Font1,SCR_PROGRESSBAR_CURX,SCR_PROGRESSBAR_Y,COLOR_1,-1,"[.");//black color
		textprintf_centre_ex(screen,Font1,SCR_PROGRESSBAR_CURX,SCR_PROGRESSBAR_CURY,COLOR_2,-1,"[");//white color
		SCR_PROGRESSBAR_CURX+=st;
		for(int i=SCR_PROGRESSBAR_X+st; i < 1024-st; i+=st){
			textprintf_centre_ex(screen,Font1,i,SCR_PROGRESSBAR_CURY,COLOR_BLUE,-1,".");}//new
	}
	if((x%step) == (step-1)) { 
		textprintf_centre_ex(screen,Font1,SCR_PROGRESSBAR_CURX,SCR_PROGRESSBAR_CURY,COLOR_2,-1,".");
		SCR_PROGRESSBAR_CURX+=st;
		//textprintf_centre_ex(screen,Font1,SCR_PROGRESSBAR_CURX,SCR_PROGRESSBAR_CURY,COLOR_BLUE,-1,".");//new
	}
};
void PROCESSBAR_XY(int _x,int _y, int x,int step /*=100*/){//grey
	SCR_PROGRESSBAR_CURX = _x;
	SCR_PROGRESSBAR_CURX = _y;
	PROCESSBAR(x+1, step);
};
/*void PROCESSBAR(int x){//grey
	PROCESSBAR(x,100); 
};*/
void WAIT(){//grey
	while( !key[KEY_ESC] ){;} 
	key[KEY_ESC] = 0; 
};
void WAIT1(){//grey
	textprintf_centre_ex(screen,Font1,/*1000*/SCR_PROGRESSBAR_CURX,SCR_PROGRESSBAR_Y,COLOR_2,-1,"]");
	while( !key[KEY_ESC] ){;} 
	key[KEY_ESC] = 0; 
};

int GetWaterPixel() {	
	return  makecol(50,50,180+Random(50)); 
};
int GetCoastPixel() {
//	int value = makecol(180+Random(40),140+Random(80),0); //dirty SAND color
//	
//	int NN = 30; //random chanses quantity
//	switch( Random(NN) ) {
//		//case 0:
//		//	value = COLOR_BLACK; break;//black stowns
//		//case 1:
//		//	value = COLOR_DKGREY; break;//dark grey stowns
//		//case 2:
//		//case 3:
//		//case 4:
//		//case 5:
//		//case 6:
//		//	value = COLOR_GREY; break;//grey stowns
//		//case 7:
//		//case 8:
//		//	value = COLOR_ORANGE; break;
//		//case 9:
//		//case 10:
//		//	value = COLOR_BROWN; break;
//		//case 11:
//		//case 12:
//		//	value = COLOR_YELLOW; break;
//		//case 13:
//		case 14:
//#define SHINESANDCOL	227+Random(20)
//			value = makecol(SHINESANDCOL,SHINESANDCOL,SHINESANDCOL); break;//shine sand
//		case 15:
//		//case 16:
//#define WETSANDCOL	227+Random(20)
//			value = makecol(121,97,84); break;//wet sand
//		case 17:
//		//case 18:
//			value = makecol(154,133,106); break;
//		case 19:
//		//case 20:
//			value = makecol(148,122,99); break;
//		case 21:
//		//case 22:
//			value = makecol(170,150,123); break;
//		case 23:
//		//case 24:
//			value = makecol(241,187,84); break;
//		/*default 25 26 27 28 29 */
//	}
//
//	return  value; //makecol(220,220,20); //

	int RG = 100+Random(100); //180+Random(40),140+Random(80)
//	return  makecol(RG,RG,0); //makecol(220,220,20); //
	return  makecol(180+Random(40),140+Random(80),0); //dirty SAND color
};


int Mix_Array(int *arr, int nn){
	return Mix_Array(arr, 0, nn);
};
//int Mix_Array(int *arr /line/, int pos1 /i/, int pos2 /lmi/, int len /LEN/, int step=1 /STEP/, int rnd=0 /random(rnd)/)
int Mix_Array(int *arr, int pos1, int pos2, int mix_len/*=1*/, int mix_step/*=1*/, int rnd/*=0*/)
{
	int nn=pos2, count=0;
	for(int i=pos1; i < nn/2 ; i+=(mix_len+mix_step)) {
		for(int j=0; j < mix_len; j++) {
			int k= (rnd==0? i+j: Random(rnd<nn? rnd: nn) );
			int value	= arr[k];
			arr[k]		= arr[nn-k];
			arr[nn-k]	= value;
			count++;
		}
	}
	//if(rnd) {
	//	if(rnd < 0)		rnd= -rnd;
	//	if(rnd > nn)	rnd= nn;
	//	for(int i=pos1; i < nn; i++) {
	//		int k= rand()%rnd, value = arr[k];
	//		arr[k] = arr[nn-k];
	//		arr[nn-k] = value;
	//	}
	//}
	return count; //return  count==0 - failed
};