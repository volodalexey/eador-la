#ifdef _DEBUG
#define ALLEGRO_NO_MAGIC_MAIN
#endif
#include "Mapper.h"

bool bFullLogMode=false;
char stVersion[16];
bool BF_Cheat=false;
bool bBetha=false;
bool bLog=false;
bool bDarken=false;
bool bTimeShift=false;
bool bDebugMode=false;

bool bUseWorldRules=false;

bool bSound=true;
bool bWindowMode=true;
bool bBackgroundMode=false;
int	iRefreshDelay=20;
int	iMouseDelay=5;
int	iKeyboardDelay=500;
int	iKeyboardRepeat=500;
int iOggBuffer=20000;
int iColorDepth=24;
bool bIntro=true;
bool bLakeShip=false;

bool bStaticLoot=false;
bool bStaticEvent=false;
bool bShowWallpaper=true;
int RES_BUY_CHANGE=5;		//Изменение цены на ресурс при его покупке в %
int MAX_PROFIT=500;			//Максимальный доход в процентах от золота игрока
int MERCENARY_TIME=7;		//Через сколько ходов сменяются наемники
int EVENT_POSSIBILITY=30;	//Вероятность случайных событий (из 1000)
int MAX_BATTLE_DURATION=40;	//Максимальная продолжительность боя в ходах (после чего наступает ничья)
int MAX_ILLUSTRATION=107;	//Максимум картинок для диалогов и событий
int MAX_PATH=10;

int iAntisleep=0;
int iAntisleepDelay=0;

int Timer=false;		//Флаг опроса таймера
int Tick=0;
int mouse_pointer=0;

 bool bGlobalError=true;
 

FONT *Font1, *Font2;
BITMAP *screen1, *Background, *bmpDarkScreen;
BITMAP *bmpEmpty;
BITMAP *bmpHourGlass[13];
BITMAP *bmpExplo[9];
BITMAP *bmpMousePointer[MAX_MOUSE_POINTER];
BITMAP *bmpButton[MAX_BUTTON_BMP];
BITMAP *bmpStuff[MAX_STUFF];
BITMAP *bmpPortrait[MAX_HERO_PIC];
BITMAP *bmpSmallPort[MAX_HERO_PIC];
BITMAP *bmpAvatar[MAX_AVATAR_PIC];
BITMAP *bmpSmallAvatar[MAX_AVATAR_PIC];
BITMAP *bmpHeroSkill[MAX_HERO_SKILL*5+1];
BITMAP *bmpDialogWindow[MAX_DIALOG_WINDOW];
BITMAP *bmpMapGrass, *bmpMapSwamp, *bmpMapSand, *bmpMapDark, *bmpMapWater[5];
BITMAP *bmpHerbMain, *bmpFlagMain, *bmpHerbBorder, *bmpFlagCol1, *bmpFlagCol2, *bmpHerbRandom;
BITMAP *bmpHerbSign[MAX_HERB_SIGN], *bmpHerbSignS[MAX_HERB_SIGN], *bmpTownFlag[3][3];
BITMAP *bmpTown[MAX_RACE][9], *bmpTownShadow[MAX_RACE][9];
BITMAP *bmpNoRoom, *bmpArena, *bmpLair, *bmpTemple, *bmpShop;
BITMAP *bmpResBorder, *bmpResGuardBorder, *bmpResHidden;
BITMAP *bmpMapSelect;

int arLootTable[9][2];
int iSoundVolume=225;
int iMusicVolume=255;
int iHourGlass=0;
bool bHourGlass=false;
int iExitFlag;
//SAMPLE *musTrack[MAX_MUSIC];
bool bButtonDelay=false;
bool bAIBattle=false;
int flgEnglish=0; //0 - старая рус. версия, 1  - английская, 2 - новая рус.

bool bGameInProcess=true;
bool bScrShot=false;
bool bArena=false;
bool bSmartAI=true;
bool bVerySmartAI=false;
int	iScrShot=0;
int iActivePlayer;
int CurrentPlayer;
clHumanProfile arHumans[MAX_PLAYERS];
int Treasurez[6][2];
int Recruitz[MAX_RECRUIT];
int iHeroAdvice=0;
int iAdvice;
dcHero *pAdvice;
int iDialogButt=0;
//using namespace audiere;
//AudioDevicePtr soundDevice;
//OutputStreamPtr Track[2];
//OutputStreamPtr Music[MAX_MUSIC];
int	iGameMode=-1,			//Текущий режим игры (-1 - загрузка и заставка, 0 - меню, 1 - астрал, 2 - основной, 3 - битва, 4 - перед битвой, 5 - титры, 6 - победа, 7 - поражение)
	iActiveTrack=0,			//№ текущего трека
	iPause=100;				//Длительность паузы
float fFading=0;			//Затухание звука
unsigned int iSeed=1;
int arHerbColors[MAX_HERB_COLOR][3]=
	{	
		{239,43,22},
		{45,45,233},
		{11,126,22},
		{0,0,0},
		{255,255,255},
		{226,244,46},
		{101,3,170},
		{76,237,274},
		{254,135,29},
		{106,63,25},
		{96,250,79},
		{247,81,233},
		{72,126,98},
		{113,23,5},
		{20,5,113},
		{97,102,24}
	};

int main(int argc, char **argv)
{
	sprintf(stVersion,"1.05.1");
	//Инициализация Аллегро
	LoadConfig();
	set_uformat(U_ASCII);
	if (allegro_init()!=0)
	{
//		std::cout << "Cannot initialize Allegro";
//		int X;
//		std::cin >> X;
		return 1;
	};

	install_int(ItIsTime,iRefreshDelay);
	install_int(AnimHourglass,200);
	install_int(MouseTime,iMouseDelay);
	LOCK_FUNCTION(ItIsTime);
	LOCK_FUNCTION(AnimHourglass);
	LOCK_FUNCTION(MouseTime);
	LOCK_VARIABLE(Timer);
	LOCK_FUNCTION(clAudioTrack::Poll());


	set_color_depth(iColorDepth);
	if (argc>1)
	{
		/*PACKFILE * WorkFile=pack_fopen("key.dat","w");		
		char eMail[30];
		long int iKey1;
		for (int i=0; i<30; i++)
		{eMail[i]=argv[1][i];};
		iKey1=MakeKey(eMail);
		pack_fwrite(eMail,sizeof eMail,WorkFile);
		pack_fwrite(&iKey1,sizeof iKey1,WorkFile);
		pack_fclose(WorkFile);
		return 0;*/
		if (argv[1][0]=='f')
		{bWindowMode=false;};
	};

	int iInitGfx=0;
	while (iInitGfx<5)
	{
		if (bWindowMode)
		{
			if (set_gfx_mode(GFX_AUTODETECT_WINDOWED,1024,768,0,0)==0)
			{iInitGfx=5;};
			if (iInitGfx<5)
			{
				set_color_depth(16);
				if (set_gfx_mode(GFX_AUTODETECT_WINDOWED,1024,768,0,0)==0)
				{iInitGfx=5;};
			};
			if (iInitGfx<5)
			{
				set_color_depth(24);
				if (set_gfx_mode(GFX_AUTODETECT_WINDOWED,1024,768,0,0)==0)
				{iInitGfx=5;};
			};
			if (iInitGfx<5)
			{
				set_color_depth(32);
				if (set_gfx_mode(GFX_AUTODETECT_WINDOWED,1024,768,0,0)==0)
				{iInitGfx=5;};
			};
			if (iInitGfx==5)
			{
				if (flgEnglish==1)
				{set_window_title("Eador. Genesis");}
				else
				{set_window_title("Эадор. Сотворение");}
				if (bBackgroundMode)
				{set_display_switch_mode(SWITCH_BACKGROUND);};
			};
		}
		else
		{
			if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,1024,768,0,0)==0)
			{iInitGfx=5;};
			if (iInitGfx<5)
			{
				set_color_depth(16);
				if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,1024,768,0,0)==0)
				{iInitGfx=5;};
			};
			if (iInitGfx<5)
			{
				set_color_depth(24);
				if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,1024,768,0,0)==0)
				{iInitGfx=5;};
			};
			if (iInitGfx<5)
			{
				set_color_depth(32);
				if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,1024,768,0,0)==0)
				{iInitGfx=5;};
			};
			if (iInitGfx==5)
			{
				if (bBackgroundMode)
				{set_display_switch_mode(SWITCH_BACKAMNESIA);};
			};
		};
		if (iInitGfx==1)
		{iInitGfx=6;};
		if (iInitGfx==0)
		{
			bWindowMode=!bWindowMode;
			iInitGfx=1;
		};
	};
	if (iInitGfx==6)
	{
		set_gfx_mode(GFX_TEXT,0,0,0,0);
		allegro_message("Error! Cannot initialise gfx");
		readkey();
		return 1;			
	};	
	
	install_timer();
	char F1[15], F2[15];
	sprintf(F1,"eador.lok");
	sprintf(F2,"eador.key");

///*******KEYGEN*********
	//Keygen();
	//bGameInProcess=false;	
	//allegro_exit();
	//return 0;


	install_mouse();
	install_keyboard();	
	char Garb[5];
	install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,Garb);
	set_keyboard_rate(iKeyboardDelay,iKeyboardRepeat);
	set_trans_blender(128,128,128,128);
	position_mouse(500,300);
	Font1=load_txt_font("font01.txt",NULL,NULL);
	Font2=load_txt_font("font02.txt",NULL,NULL);
	allegro_404_char='$';	

	screen1=create_bitmap(1024,768);
	NullColor=getpixel(screen1,10,10);
	bmpDarkScreen=create_bitmap(1024,768);	
	bmpHint=create_bitmap(MAX_HINT_W,MAX_HINT_H);
	bmpShadow=create_bitmap(100,100);
	bmpEmpty=create_bitmap(1,1);
	bmpMapSelect=create_bitmap(300,300);
	rectfill(bmpEmpty,0,0,1,1,COLOR_0);
	bHint=false; HintType=0;
	COLOR_0=makecol(255,0,255);
	COLOR_1=makecol(0,0,0);
	COLOR_2=makecol(220,220,220);
	COLOR_3=makecol(220,220,20);
	BeginLog();
	rectfill(screen,0,0,1024,768,COLOR_1);
	rectfill(screen1,0,0,1024,768,COLOR_1);
	FileLog("Loading Interface.dat...");
	DATAFILE *datGUI=load_datafile("dat\\interface.dat");
	if (!datGUI)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading interface.dat");		
		readkey();
		return 1;
	};
	bmpGUI_SpellMask=(BITMAP *)datGUI[0].dat;
	bmpGUI_SpellSelect=(BITMAP *)datGUI[1].dat;
	bmpGUI_HeroTrade=(BITMAP *)datGUI[2].dat;
	bmpGUI_HeroScreen=(BITMAP *)datGUI[3].dat;
	bmpGUI_SpellShadow=(BITMAP *)datGUI[4].dat;
	bmpGUI_UnitMask=(BITMAP *)datGUI[5].dat;
	bmpGUI_UnitSelect=(BITMAP *)datGUI[6].dat;
	bmpGUI_UnitClosed=(BITMAP *)datGUI[7].dat;
	bmpGUI_ButtTrashNorm=(BITMAP *)datGUI[8].dat;
	bmpGUI_ButtTrashAct=(BITMAP *)datGUI[9].dat;
	bmpGUI_ButtTrashInact=(BITMAP *)datGUI[10].dat;
	bmpGUI_DiffWindow=(BITMAP *)datGUI[11].dat;
	bmpGUI_EndCamp=(BITMAP *)datGUI[12].dat;
	bmpGUI_ButtExitPress=(BITMAP *)datGUI[13].dat;
	bmpGUI_Pointer=(BITMAP *)datGUI[14].dat;
	bmpGUI_UnitLvlUp=(BITMAP *)datGUI[15].dat;
	bmpGUI_ExpLow=(BITMAP *)datGUI[16].dat;
	bmpGUI_ResourceInfo=(BITMAP *)datGUI[17].dat;
	bmpGUI_HealthLow=(BITMAP *)datGUI[18].dat; 
	bmpGUI_DefenderInfo=(BITMAP *)datGUI[19].dat;
	bmpGUI_HeroLvlUp=(BITMAP *)datGUI[20].dat;
	bmpGUI_HeroLvl10=(BITMAP *)datGUI[21].dat; 
	bmpGUI_Hint=(BITMAP *)datGUI[22].dat;
	bmpGUI_HeroAdvice=(BITMAP *)datGUI[23].dat;
	bmpGUI_RulerBar=(BITMAP *)datGUI[24].dat;
	bmpGUI_SpellClosed=(BITMAP *)datGUI[25].dat;
	bmpGUI_UnitInfo=(BITMAP *)datGUI[26].dat;
	bmpGUI_SpellInfo=(BITMAP *)datGUI[27].dat;
	bmpGUI_Garrison=(BITMAP *)datGUI[28].dat;
	bmpGUI_RecruitClosed=(BITMAP *)datGUI[29].dat;
	bmpGUI_NoRes=(BITMAP *)datGUI[30].dat;
	bmpGUI_Library=(BITMAP *)datGUI[31].dat;
	bmpGUI_SpellTrade=(BITMAP *)datGUI[32].dat;
	bmpGUI_Shop=(BITMAP *)datGUI[33].dat;
	bmpGUI_ItemTrade=(BITMAP *)datGUI[34].dat;
	bmpGUI_ItemInfo=(BITMAP *)datGUI[35].dat;
	bmpGUI_Victory=(BITMAP *)datGUI[36].dat;
	bmpGUI_Lose=(BITMAP *)datGUI[37].dat;
	bmpGUI_Dead=(BITMAP *)datGUI[38].dat;
	bmpGUI_LowPanel=(BITMAP *)datGUI[39].dat;
	bmpGUI_ProvinceBar=(BITMAP *)datGUI[40].dat;
	bmpGUI_BuildBar=(BITMAP *)datGUI[41].dat;
	bmpGUI_BuildPanel=(BITMAP *)datGUI[42].dat;
	bmpGUI_BuildSelect=(BITMAP *)datGUI[43].dat;
	bmpGUI_BuildMask=(BITMAP *)datGUI[44].dat;
	bmpGUI_Building=(BITMAP *)datGUI[45].dat;
	bmpGUI_RazePanel=(BITMAP *)datGUI[46].dat;
	bmpGUI_Defender=(BITMAP *)datGUI[47].dat;
	bmpGUI_Site=(BITMAP *)datGUI[48].dat;
	bmpGUI_Fort=(BITMAP *)datGUI[49].dat;
	bmpGUI_Castle=(BITMAP *)datGUI[50].dat;
	bmpGUI_HeroBar=(BITMAP *)datGUI[51].dat;
	bmpGUI_OuterBuildInfo=(BITMAP *)datGUI[56].dat;
	bmpGUI_Treasure=(BITMAP *)datGUI[57].dat;
	bmpGUI_HeroRecruit=(BITMAP *)datGUI[58].dat;
	bmpGUI_HeroRecruitWindow=(BITMAP *)datGUI[59].dat;
	bmpGUI_HeroResurrectWindow=(BITMAP *)datGUI[60].dat;
	bmpGUI_HeroDead=(BITMAP *)datGUI[61].dat;
	bmpGUI_Profile=(BITMAP *)datGUI[62].dat;
	bmpGUI_EndGameScreen=(BITMAP *)datGUI[63].dat;
	bmpGUI_HighScore=(BITMAP *)datGUI[64].dat;
	bmpGUI_UnitTypeInfo=(BITMAP *)datGUI[65].dat;
	bmpGUI_Statistics=(BITMAP *)datGUI[66].dat;
	bmpGUI_UnitReward=(BITMAP *)datGUI[67].dat;
	bmpGUI_DefChoose=(BITMAP *)datGUI[68].dat;
	bmpGUI_TakeLoot=(BITMAP *)datGUI[69].dat;
	bmpGUI_Necro=(BITMAP *)datGUI[70].dat;
	bmpGUI_StrHint=(BITMAP *)datGUI[71].dat;
	bmpGUI_ItemEffect=(BITMAP *)datGUI[72].dat;
	bmpGUI_SkillInfo=(BITMAP *)datGUI[73].dat;
	bmpGUI_BattleUnitInfo=(BITMAP *)datGUI[74].dat;
	bmpGUI_AbilityInfo=(BITMAP *)datGUI[75].dat;
	bmpGUI_Gold=(BITMAP *)datGUI[76].dat;
	bmpGUI_Crystal=(BITMAP *)datGUI[77].dat;
	bmpGUI_ItemSet=(BITMAP *)datGUI[78].dat;
	bmpGUI_NewTurn=(BITMAP *)datGUI[79].dat;
	bmpGUI_Screen01=(BITMAP *)datGUI[80].dat;
	bmpGUI_Diplomacy=(BITMAP *)datGUI[81].dat;
	bmpGUI_Cursor=(BITMAP *)datGUI[82].dat;
	bmpGUI_HeroPort=(BITMAP *)datGUI[83].dat;
	bmpGUI_MainMenu=(BITMAP *)datGUI[84].dat;
	bmpGUI_Eador=(BITMAP *)datGUI[85].dat;
	bmpGUI_MenuScrN=(BITMAP *)datGUI[86].dat;
	bmpGUI_MenuScrA=(BITMAP *)datGUI[87].dat;
	bmpGUI_NewGame=(BITMAP *)datGUI[88].dat;
	bmpGUI_NGPlayer=(BITMAP *)datGUI[89].dat;
	bmpGUI_Build=(BITMAP *)datGUI[90].dat;
	bmpGUI_Built=(BITMAP *)datGUI[91].dat;
	bmpGUI_NoGold=(BITMAP *)datGUI[92].dat;
	bmpGUI_NoGem=(BITMAP *)datGUI[93].dat;
	bmpGUI_InnBldHint=(BITMAP *)datGUI[94].dat;
	bmpGUI_InnBldInfo=(BITMAP *)datGUI[95].dat;
	bmpGUI_UnitSlot=(BITMAP *)datGUI[96].dat;
	bmpGUI_SpellSlot=(BITMAP *)datGUI[97].dat;
	bmpGUI_String=(BITMAP *)datGUI[98].dat;
	bmpGUI_GroupIcon=(BITMAP *)datGUI[99].dat;
	bmpGUI_GroupHint=(BITMAP *)datGUI[100].dat;
	bmpGUI_Planned=(BITMAP *)datGUI[101].dat;
	bmpGUI_AstralLow=(BITMAP *)datGUI[102].dat;
	bmpGUI_ShardBar=(BITMAP *)datGUI[103].dat;
	bmpGUI_ItemHint=(BITMAP *)datGUI[104].dat;	
	for (int i=0; i<4; i++)
	{bmpGUI_HeroSelect[i]=(BITMAP *)datGUI[105+i].dat;};
	for (int i=0; i<MAX_DIALOG_WINDOW; i++)
	{
		bmpDialogWindow[i]=(BITMAP *)datGUI[52+i].dat;
	};
	bmpGUI_DeadSelect=(BITMAP *)datGUI[109].dat;
	bmpGUI_Treasury=(BITMAP *)datGUI[110].dat;
	bmpGUI_HomeGarrison=(BITMAP *)datGUI[111].dat;
	bmpGUI_HomeBarracks=(BITMAP *)datGUI[112].dat;
	bmpGUI_HomeLibrary=(BITMAP *)datGUI[113].dat;
	bmpGUI_ProgressMark=(BITMAP *)datGUI[114].dat;
	bmpGUI_ProgressFill=(BITMAP *)datGUI[115].dat;
	bmpGUI_MainShardBar=(BITMAP *)datGUI[116].dat;
	bmpGUI_SpellGlow=(BITMAP *)datGUI[117].dat;
	bmpGUI_UnitGlow=(BITMAP *)datGUI[118].dat;
	bmpGUI_EnterWindow=(BITMAP *)datGUI[119].dat;
	bmpGUI_HerbWindow=(BITMAP *)datGUI[120].dat;
	bmpGUI_PickBonus=(BITMAP *)datGUI[121].dat;
	bmpGUI_PickUnit=(BITMAP *)datGUI[122].dat;
	bmpGUI_PickSpell=(BITMAP *)datGUI[123].dat;
	bmpGUI_CampVictory=(BITMAP *)datGUI[124].dat;
	bmpGUI_ChoosePlayer=(BITMAP *)datGUI[125].dat;
	bmpGUI_ScrollN=(BITMAP *)datGUI[126].dat;
	bmpGUI_ScrollA=(BITMAP *)datGUI[127].dat;
	bmpGUI_Alchemy=(BITMAP *)datGUI[128].dat;
	bmpGUI_MapSettings=(BITMAP *)datGUI[129].dat;
	bmpGUI_Broken=(BITMAP *)datGUI[130].dat;
	bmpGUI_Anvil1=(BITMAP *)datGUI[131].dat;
	bmpGUI_Anvil2=(BITMAP *)datGUI[132].dat;
	bmpGUI_Menu=(BITMAP *)datGUI[133].dat;
	bmpGUI_Settings=(BITMAP *)datGUI[134].dat;
	bmpGUI_TurnWindow=(BITMAP *)datGUI[135].dat;
	bmpGUI_ChangeWeapon=(BITMAP *)datGUI[136].dat;
	bmpGUI_SmallHint=(BITMAP *)datGUI[137].dat;
	bmpGUI_CampStat=(BITMAP *)datGUI[138].dat;
	bmpGUI_ParamHint=(BITMAP *)datGUI[139].dat;
	bmpGUI_Credits=(BITMAP *)datGUI[140].dat;

	FileLog("Loading Anim.dat...");
	DATAFILE* datAnim = load_datafile("dat\\anim.dat");
	if (!datAnim)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading anim.dat");
		readkey();
		return 1;
	};
	for (int i=0; i<13; i++)
	{
		bmpHourGlass[i]=(BITMAP *)datAnim[i].dat;
	};
	for (int i=0; i<9; i++)
	{
		bmpExplo[i]=(BITMAP *)datAnim[i+13].dat;
	};
	for (int i=0; i<16; i++)
	{
		bmpAnName[i]=(BITMAP *)datAnim[i+22].dat;
	};
	for (int i=0; i<15; i++)
	{
		bmpAnCrea[i]=(BITMAP *)datAnim[i+38].dat;
	};
	for (int i=0; i<7; i++)
	{
		bmpAnWeb[i]=(BITMAP *)datAnim[i+53].dat;
	};
	for (int i=0; i<8; i++)
	{
		bmpAnRoot[i]=(BITMAP *)datAnim[i+60].dat;
	};
	bmpSkull=(BITMAP *)datAnim[68].dat;
	bmpJaw=(BITMAP *)datAnim[69].dat;
	for (int i=0; i<8; i++)
	{
		bmpCloud[i]=(BITMAP *)datAnim[i+70].dat;
	};



	if (!(LoadPhrases()))
	{
		readkey();
		return 1;
	};

	OldProgress=0;
	NewProgress=0;
	RunHourGlass(153,0);

	FileLog("Loading Prototypes...");
//Загрузка прототипов
	
	if (!(Loader()))
	{
		readkey();
		return 1;
	};	

	int TypeLine=0;
	/*textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading sound files...");
	TypeLine++;
	for (int i=0; i<MAX_MUSIC; i++)
	{
		char sFile[50];
		sprintf(sFile,"sound\\track0%d.wav",i+1);
		musTrack[i]=load_sample(sFile);
	};*/

	//textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading mouse.dat...");
	//TypeLine++;
	NewProgress+=100;
	FileLog("Loading mouse.dat...");
	
	DATAFILE* datMouse = load_datafile("dat\\mouse.dat");
	if (!datMouse)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading mouse.dat");		
		readkey();
		return 1;
	};
	
	for (int i=0; i<MAX_MOUSE_POINTER; i++)
	{
		bmpMousePointer[i]=(BITMAP *)datMouse[i].dat;
	};
	NewProgress+=100;

	DATAFILE* datButtons = load_datafile("dat\\buttons.dat");
	if (!datButtons)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading buttons.dat");		
		readkey();
		return 1;
	};
	for (int i=0; i<MAX_BUTTON_BMP; i++)
	{
		bmpButton[i]=(BITMAP *)datButtons[i].dat;
	};

//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading spells.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE* datSpells = load_datafile("dat\\spells.dat");
	if (!datSpells)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading spells.dat");		
		readkey();
		return 1;
	};
	for (int i=0; i<iMaxSpell; i++)
	{
		(pFirstSpell+i+1)->Pic=(BITMAP *)datSpells[i].dat;
	};

//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading resource.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE* datResource = load_datafile("dat\\resource.dat");
	if (!datResource)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading resource.dat");		
		readkey();
		return 1;
	};
	for (int i=0; i<iMaxResource; i++)
	{
		(pFirstResource+i+1)->Pic=(BITMAP *)datResource[i].dat;
		(pFirstResource+i+1)->BigPic=(BITMAP *)datResource[i+iMaxResource].dat;
		(pFirstResource+i+1)->MedPic=(BITMAP *)datResource[i+iMaxResource*2].dat;
	};

	NewProgress+=100;
	FileLog("Loading skills.dat...");
	DATAFILE* datSkills = load_datafile("dat\\skills.dat");
	if (!datSkills)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading skills.dat");		
		readkey();
		return 1;
	};
	for (int i=0; i<=iMaxUnitUpg; i++)
	{
		(pFirstUnitUpg+i)->Pic=(BITMAP *)datSkills[i].dat;
	};
	//draw_sprite(screen,(pFirstUnitUpg+107)->Pic,0,60);
	//readkey();

	NewProgress+=100;
	DATAFILE* datHeroSkills = load_datafile("dat\\heroskills.dat");
	if (!datHeroSkills)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading heroskills.dat");		
		readkey();
		return 1;
	};
	for (int i=0; i<=140; i++)
	{
		bmpHeroSkill[i]=(BITMAP *)datHeroSkills[i].dat;
	};	

	NewProgress+=100;
	FileLog("Loading stuff.dat...");
	DATAFILE* datStuff = load_datafile("dat\\stuff.dat");
	if (!datStuff)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading stuff.dat");		
		readkey();
		return 1;
	};
	for (int i=0; i<MAX_STUFF; i++)
	{
		bmpStuff[i]=(BITMAP *)datStuff[i].dat;
	};
	NewProgress+=100;
	LoadBFPics();	

//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading medal.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE* datMedal = load_datafile("dat\\medal.dat");
	if (!datMedal)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading medal.dat");
		readkey();
		return 1;
	};

//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading medal_small.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE* datSmallMedal = load_datafile("dat\\medal_small.dat");
	if (!datSmallMedal)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading medal_small.dat");
		readkey();
		return 1;
	};

	for (int i=0; i<iMaxMedal; i++)
	{
		(pFirstMedal+i+1)->Pic=(BITMAP *)datMedal[i].dat;
		(pFirstMedal+i+1)->SmallPic=(BITMAP *)datSmallMedal[i].dat;
	};

//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading anim.dat...");
//	TypeLine++;
	NewProgress+=100;

//	if (!BF_Cheat)
	{

	NewProgress+=100;
	DATAFILE* datRaces = load_datafile("dat\\races.dat");
	if (!datRaces)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading races.dat");		
		readkey();
		return 1;
	};
	for (int i=0; i<iMaxRace; i++)
	{
		(pFirstRace+i+1)->Pic=(BITMAP *)datRaces[i].dat;
		(pFirstRace+i+1)->SmallPic=(BITMAP *)datRaces[i+iMaxRace].dat;
	};


//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading site.dat...");
//	TypeLine++;
	NewProgress+=100;
	FileLog("Loading site.dat...");
	DATAFILE* datSite = load_datafile("dat\\site.dat");
	if (!datSite)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading site.dat");		
		readkey();
		return 1;
	};
	for (int i=0; i<iMaxSite; i++)
	{
		(pFirstSite+i+1)->Pic=(BITMAP *)datSite[i].dat;
	};

//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading ritual.dat...");
//	TypeLine++;	

	//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading mapobj.dat...");
//	TypeLine++;
	DATAFILE* datMapObj = load_datafile("dat\\mapobj.dat");
	if (!datMapObj)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading mapobj.dat");
		readkey();
		return 1;
	};
	for (int i=0; i<4; i++)
	{
		bmpHeroDoll[i]=(BITMAP *)datMapObj[i].dat;
		bmpHeroDollShadow[i]=(BITMAP *)datMapObj[i+4].dat;
		bmpHeroDollS[i]=(BITMAP *)datMapObj[i+20].dat;
	};

	//for (int i=0; i<16; i++)
	//{
	//	bmpFlag[i]=(BITMAP *)datMapObj[i+4].dat;
	//	bmpHerb[i]=(BITMAP *)datMapObj[i+24].dat;
	//};	

	bmpHeroSelect=(BITMAP *)datMapObj[40].dat;

	for (int i=0; i<4; i++)
	{
		bmpPath[i]=(BITMAP *)datMapObj[i+41].dat;
	};

	for (int i=0; i<MAX_OUTBLD_PIC; i++)
	{
		bmpOuterBld[i]=(BITMAP *)datMapObj[i+45].dat;
	};
	bmpRebelDoll=(BITMAP *)datMapObj[49].dat;
	for (int i=0; i<3; i++)
	{
		bmpUnhappy[i]=(BITMAP *)datMapObj[50+i].dat;
	};
	bmpNoRoom=(BITMAP *)datMapObj[39].dat;
	bmpArena=(BITMAP *)datMapObj[45].dat;
	bmpLair=(BITMAP *)datMapObj[46].dat;
	bmpTemple=(BITMAP *)datMapObj[47].dat;
	bmpShop=(BITMAP *)datMapObj[48].dat;
	bmpUnrest=(BITMAP *)datMapObj[53].dat;
	bmpDefender=(BITMAP *)datMapObj[54].dat;
	bmpQuest[0]=(BITMAP *)datMapObj[55].dat;
	bmpQuest[1]=(BITMAP *)datMapObj[56].dat;
	bmpBorder=(BITMAP *)datMapObj[57].dat;
	bmpResBorder=(BITMAP *)datMapObj[58].dat;
	bmpResGuardBorder=(BITMAP *)datMapObj[59].dat;
	bmpResHidden=(BITMAP *)datMapObj[60].dat;

//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading herb.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE* datHerb = load_datafile("dat\\herb.dat");
	if (!datHerb)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading herb.dat");
		readkey();
		return 1;
	};	
	for (int i=0; i<iMaxHerbSign; i++)
	{
		bmpHerbSign[i]=(BITMAP *)datHerb[i].dat;
	};

//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading herbS.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE* datHerbS = load_datafile("dat\\herbS.dat");
	if (!datHerbS)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading herbS.dat");
		readkey();
		return 1;
	};	
	for (int i=0; i<iMaxHerbSign; i++)
	{
		bmpHerbSignS[i]=(BITMAP *)datHerbS[i].dat;
	};

//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading flag.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE* datFlag = load_datafile("dat\\flag.dat");
	if (!datFlag)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading flag.dat");
		readkey();
		return 1;
	};
	bmpHerbMain=(BITMAP *)datFlag[0].dat;
	bmpHerbBorder=(BITMAP *)datFlag[1].dat;
	bmpFlagMain=(BITMAP *)datFlag[2].dat;
	bmpFlagCol1=(BITMAP *)datFlag[3].dat;
	bmpFlagCol2=(BITMAP *)datFlag[4].dat;
	bmpHerbRandom=(BITMAP *)datFlag[5].dat;
	for (int i=0; i<3; i++)
	{
		for (int j=0; j<3; j++)
		{
			bmpTownFlag[i][j]=(BITMAP *)datFlag[6+i*3+j].dat;
		};
	};

	for (int i=0; i<MAX_PLAYERS; i++)
	{
		bmpHerb[i]=create_bitmap(bmpHerbMain->w,bmpHerbMain->h);
		rectfill(bmpHerb[i],0,0,bmpHerbMain->w,bmpHerbMain->h,COLOR_0);
		bmpFlag[i][0]=create_bitmap(bmpFlagMain->w,bmpFlagMain->h);
		rectfill(bmpFlag[i][0],0,0,bmpFlagMain->w,bmpFlagMain->h,COLOR_0);
		bmpFlag[i][1]=create_bitmap(bmpTownFlag[0][0]->w,bmpTownFlag[0][0]->h);
		rectfill(bmpFlag[i][1],0,0,bmpTownFlag[0][0]->w,bmpTownFlag[0][0]->h,COLOR_0);
		bmpFlag[i][2]=create_bitmap(bmpTownFlag[1][0]->w,bmpTownFlag[1][0]->h);
		rectfill(bmpFlag[i][2],0,0,bmpTownFlag[1][0]->w,bmpTownFlag[1][0]->h,COLOR_0);
		bmpFlag[i][3]=create_bitmap(bmpTownFlag[2][0]->w,bmpTownFlag[2][0]->h);
		rectfill(bmpFlag[i][3],0,0,bmpTownFlag[2][0]->w,bmpTownFlag[2][0]->h,COLOR_0);
	};

	char sFile[50];
	for (int i=1; i<=iMaxRace; i++)
	{
		sprintf(sFile,"dat\\town\\town%d.dat",i);
	//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading %s...",sFile);
	//	TypeLine++;
		DATAFILE* datTown = load_datafile(sFile);
		NewProgress+=100;
		if (!datTown)
		{
			textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading %s",sFile);
			readkey();
			return 1;
		};
		for (int j=0; j<9; j++)
		{
			bmpTown[i-1][j]=(BITMAP *)datTown[j].dat;
			bmpTownShadow[i-1][j]=(BITMAP *)datTown[9+j].dat;
		};
	};

//	textprintf_ex(screen,Font1,300,TypeLine*20,makecol(220,220,20),-1,"Loading astral.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE* datAstral = load_datafile("dat\\astral.dat");
	if (!datAstral)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading astral.dat");
		readkey();
		return 1;
	};
	bmpAstral=(BITMAP *)datAstral[0].dat;
	set_trans_blender(200,200,200,0);
	for (int i=0; i<MAX_SHARD_PIC; i++)
	{
		bmpShard[i]=(BITMAP *)datAstral[i+1].dat;
		bmpShardSelect[i]=create_bitmap(bmpShard[i]->w,bmpShard[i]->h);		
		rectfill(bmpShardSelect[i],0,0,bmpShard[i]->w,bmpShard[i]->h,COLOR_0);
		draw_lit_sprite(bmpShardSelect[i],bmpShard[i],0,0,100);
	};
	bmpMainShard=(BITMAP *)datAstral[MAX_SHARD_PIC+1].dat;
	bmpMainShardSelect=create_bitmap(bmpMainShard->w,bmpMainShard->h);
	rectfill(bmpMainShardSelect,0,0,bmpMainShard->w,bmpMainShard->h,COLOR_0);
	draw_lit_sprite(bmpMainShardSelect,bmpMainShard,0,0,100);
	set_trans_blender(128,128,128,128);

//	extern BITMAP *bmpShard[MAX_SHARD_PIC];	//Изображения осколков
//	extern BITMAP *bmpAstral;

	NewProgress+=100;
	DATAFILE* datShardBonus = load_datafile("dat\\shardbonus.dat");
	if (!datShardBonus)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading ShardBonus.dat");
		readkey();
		return 1;
	};	
	for (int i=0; i<MAX_SHARD_BONUS_PIC; i++)
	{
		bmpShardBonus[i]=(BITMAP *)datShardBonus[i].dat;
	};

	NewProgress+=100;

	bmpRed=create_bitmap(52,52);
	rectfill(bmpRed,0,0,52,52,makecol(255,0,0));
	bmpBlack=create_bitmap(52,52);
	rectfill(bmpBlack,0,0,52,52,COLOR_1);

#ifdef YUPLAY
	if (YuPlayInit()!=0)
	{return 1;};
	NewProgress+=100;
#endif
	
	/*for (int i=0; i<MAX_MUSIC; i++)
	{
		char sFile[50];
		if (i+1<10)
		{sprintf(sFile,"Music\\track0%d.ogg",i+1);}
		else
		{sprintf(sFile,"Music\\track%d.ogg",i+1);};
		Music[i]=OpenSound(soundDevice,sFile,true);
		NewProgress+=100;
	};*/
//	bSound=true;

//**************Заставка*******************
	if (bIntro)
	{
	DATAFILE* datArt = load_datafile("dat\\art.dat");
	if (!datArt)
	{
		textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading art.dat");
		readkey();
		return 1;
	};
	BITMAP *bmpArt[14];
	for (int i=0; i<14; i++)
	{
		bmpArt[i]=(BITMAP *)datArt[i].dat;		
	};
	NewProgress+=100;
	bHourGlass=false;
	rectfill(screen1,0,0,1024,768,COLOR_1);
	clear_keybuf();	
	//play_sample((pFirstUnitType+29)->wavShootHit,iSoundVolume,128,1000,0);
	int Tick=0, Count=0, Count0=0, Logoz=0;
	bool bBabum=true;
	BITMAP *bmpBR=create_bitmap(600,400);
	rectfill(bmpBR,0,0,600,400,COLOR_1);
	set_trans_blender(30,30,30,30);
	
	while ((!keypressed())&&(Count<85))
	{
		if (Timer)
		{
			Timer=false;
			Tick++;
#ifdef SNOWBALL
			Count0=Tick/3;
			int MaxLogo=2;
	//		if (bEnglish)
	//		{MaxLogo=1;};
			
			if (Logoz<MaxLogo)
			{
			if (Count0==0)
			{			
				draw_sprite(screen1,bmpStuff[78+Logoz],512-bmpStuff[78+Logoz]->w/2,384-bmpStuff[78+Logoz]->h/2);
				set_trans_blender(30,30,30,30);
			};
			if (Count0>=20)
			{
				draw_trans_sprite(screen1,bmpBR,512-bmpBR->w/2,384-bmpBR->h/2);
			};
			if (Count0>30)
			{
				draw_sprite(screen1,bmpBR,512-bmpArt[0]->w/2,384-bmpArt[0]->h/2);
				Tick=0;
				Logoz++;
			};
			}
			else
#endif
			{


			Count=Tick/3;
			if (Count==0)
			{
				if (flgEnglish>0)
				{Tick=90;}
				else
				{
				draw_sprite(screen1,bmpArt[0],512-bmpArt[0]->w/2,384-bmpArt[0]->h/2);	
				PlayUnitSound(71);
				};
			}
			else
			{
				if (Count<14)
				{
					draw_sprite(screen1,bmpArt[Count],512-bmpArt[0]->w/2,384-bmpArt[0]->h/2);
				};
			}
			if (Count==20)
			{
			//	textprintf_centre_ex(screen,Font1,512,384+bmpArt[0]->h/2,COLOR_2,-1,"Совместно с...");
			//	if (bBabum)
			//	{
			//		PlayUnitSound(48);
				//	play_sample((pFirstUnitType+28)->wavHit,iSoundVolume,128,1000,0);
			//		bBabum=false;
			//	};
			};
			if ((Count>=30)&&(Count<40))
			{draw_trans_sprite(screen1,bmpBR,512-bmpBR->w/2,384-bmpBR->h/2);};
			if (Count==40)
			{
				draw_sprite(screen1,bmpBR,512-bmpArt[0]->w/2,384-bmpArt[0]->h/2);
			//	draw_trans_sprite(screen1,bmpStuff[67],512-bmpStuff[67]->w/2,384-bmpStuff[67]->h/2);
				PlaySpellSound(46);
			};
			set_trans_blender(50,50,50,50);
			if ((Count>=40)&&(Count<50))
			{
				set_trans_blender(250-(Count-40)*25,250-(Count-40)*25,250-(Count-40)*25,250-(Count-40)*25);			
				draw_sprite(screen1,bmpStuff[67],512-bmpStuff[67]->w/2,384-bmpStuff[67]->h/2);				
				draw_trans_sprite(screen1,bmpBR,512-bmpBR->w/2,384-bmpBR->h/2);
			};
			if (Count==50)
			{			
				draw_sprite(screen1,bmpStuff[67],512-bmpStuff[67]->w/2,384-bmpStuff[67]->h/2);
				set_trans_blender(30,30,30,30);
			};
		
			if (Count>=70)
			{
				draw_trans_sprite(screen1,bmpBR,512-bmpBR->w/2,384-bmpBR->h/2);
			};
			}
			blit(screen1,screen,0,0,0,0,1024,786);
		};
	};
	set_trans_blender(128,128,128,128);
	};
//**************Не Заставка*******************

	};//BF_Cheat

	
	





	bExitGame=true;



	set_mouse_sprite(bmpMousePointer[1]);
	mouse_pointer=1;
	bHourGlass=false;

	

	if (key[KEY_B])
	{
		BF_Cheat=true;
		arPlayer[2].Control=1;
		arPlayer[1].Control=1;		
		arPlayer[2].Init(1,1);
		arPlayer[1].Init(1,2);
		arPlayer[2].Num=2;
		arPlayer[1].Num=1;		
		arPlayer[2].Gem=5000;
		arPlayer[1].Gem=5000;
		arPlayer[2].ipProvince=0;
		arPlayer[1].ipProvince=0;
		pFirstProvince=new dcProvince();
		pFirstProvince->ipTerrain=1;
		pFirstProvince->iNumber=3;
		CurGame.iSeed=10;
		arPlayer[2].RecruitHero(3);
		arPlayer[2].arHero[0]->iLifeBonus=500;
		arPlayer[2].arHero[0]->pSquad[0]->iLife=arPlayer[2].arHero[0]->pSquad[0]->GetLife();
		arPlayer[1].RecruitHero(1);
		arPlayer[1].arHero[0]->iLeader=10;
		arPlayer[2].arHero[0]->iLeader=10;


		for (int i=0; i<3; i++)
		{
			arPlayer[2].arHero[0]->arInventory[16+i][0]=402+i;
			arPlayer[2].arHero[0]->arInventory[16+i][1]=(pFirstItem+402+i)->iDurability;
		};
		
	//	arPlayer[2].arHero[0]->arInventory[1][0]=159;
		arPlayer[2].arHero[0]->arInventory[1][0]=30;
		arPlayer[2].arHero[0]->arInventory[1][1]=20;
		arPlayer[2].arHero[0]->arInventory[3][1]=2;
		arPlayer[2].arHero[0]->arInventory[3][0]=58;
		arPlayer[2].arHero[0]->arInventory[3][1]=2;
		arPlayer[2].arHero[0]->arSpell[1]=72;
		arPlayer[2].arHero[0]->arSpell[2]=52;
		arPlayer[2].arHero[0]->arSpell[3]=74;
		arPlayer[2].arHero[0]->arSpell[4]=13;
		arPlayer[2].arHero[0]->arSpell[5]=41;
		arPlayer[2].arHero[0]->arSpell[6]=7;
		arPlayer[2].arHero[0]->arSpell[7]=2;
		arPlayer[2].arHero[0]->arSpell[8]=13;
		arPlayer[2].arHero[0]->iLeader=6;
		dcUnit * U1=new dcUnit(33,0);
		arPlayer[2].arHero[0]->PlaceUnit(U1,3);
		U1=new dcUnit(33,0);
		arPlayer[2].arHero[0]->PlaceUnit(U1,4);
		U1=new dcUnit(33,0);
		arPlayer[1].arHero[0]->PlaceUnit(U1,3);
		U1=new dcUnit(33,0);
		arPlayer[1].arHero[0]->PlaceUnit(U1,4);
		U1=new dcUnit(48,0);
		arPlayer[1].arHero[0]->PlaceUnit(U1,5);

		arPlayer[1].arHero[0]->arSpell[1]=16;
		arPlayer[1].arHero[0]->arSpell[2]=49;
		arPlayer[1].arHero[0]->arSpell[3]=58;
		arPlayer[1].arHero[0]->arSpell[4]=64;
		arPlayer[1].arHero[0]->arSpell[5]=74;
		arPlayer[1].arHero[0]->arSpell[6]=77;
		arPlayer[1].arHero[0]->arSpell[7]=76;
		arPlayer[1].arHero[0]->arSpell[8]=73;
		arPlayer[1].arHero[0]->arSpell[9]=18;

		

		ScreenHero(arPlayer[2].arHero[0],NULL,false);
		
		InitBattle(pFirstProvince,NULL,arPlayer[2].arHero[0],arPlayer[1].arHero[0],0,true);
	}
	else	
	{
		bool bOldDemo=bDemo;
		ScreenMenu();
		if ((bOldDemo)&&(!bDemo))
		{ScreenMenu();};
	};
	if (bDemo)
	{ScreenDemo();};
	EndLog();
	bGameInProcess=false;
//	remove_keyboard();
//	remove_mouse();
	allegro_exit();
	return 0;
}

END_OF_MAIN()

