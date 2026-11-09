//	Added by Grey 4.09.2007
//	Header file - declaration of GLOBAL:
//	Constant Variables,  Extern Variables,  Main Defines,  SERVICE Defines & Vars & Functions,  etc.

//#define YUPLAY 

#ifndef GLOBAL_USED
#define GLOBAL_USED
#ifdef _WIN32
#define ALOGG_DLL
#else
/* Native (non-Windows) builds: no __declspec import/export. */
#define ALOGG_DLL_DECLSPEC
#endif
#define ALOGG_DLL
#define SNOWBALL

#include <alogg.h>
#include <allegro.h>
#ifndef _WIN32
#include "NativeDisplay.h"
#endif
#undef bool
#undef false
#undef true
#undef int8_t
#undef int16_t
#undef int32_t
#undef int64_t
#undef uint8_t
#undef uint16_t
#undef uint32_t
#undef uint64_t

#include <cstdio>
#include <cmath>
//#include <audiere.h>

// Global Main  Const Variables
const int  MAP_PLAIN_RANGE	=  5;// отступ для равнин
const int  MAP_MOUNT_RANGE	=  5;// отступ для гор
const int  MAP_SWAMP_RANGE	= -5;// отступ для болот


// Extern Declarations
extern FONT *Font1, *Font2;
extern int COLOR_0, COLOR_1, COLOR_2, COLOR_3;
extern int COLOR_BLACK, COLOR_WHITE, COLOR_GREY, COLOR_DKGREY;//grey
extern int COLOR_RED, COLOR_ORANGE, COLOR_YELLOW, COLOR_GREEN, COLOR_CYAN, COLOR_BLUE, COLOR_VI0LET, COLOR_BROWN;//grey


// Global Main  Defined Values

#define WATER_ID		0
#define	PLAIN_ID		1
#define	FOREST_ID		2
#define	HILL_ID			3
#define	SWAMP_ID		4
#define	DESERT_ID		5
#define	TUNDRA_ID		6
#define	MOUNTAIN_ID		7

//Константы статичных классов
const int MAX_RACE=30;			//Максимум рас в игре
const int MAX_SQUAD=16;			//Максимальное количество воинов в отряде
const int MAX_ENEMY=5;			//Максимальное количество врагов у одной расы
const int MAX_TERRAIN=6;		//Максимальное количество обитаемых типов ландшафта
const int MAX_RACE_QUEST=10;	//Максимальное количество заданий для заключения союза у одной расы
const int MAX_UPGRADE_SIZE=5;	//Максимальное количество свойств в одном улучшении юнита
const int MAX_UPGRADE_NEED=3;	//Максимальное количество требуемых улучшений в дереве умений юнита
const int MAX_UNIT_RES=3;		//Максимальное количество ресурсов, необходимых для одного юнита
const int MAX_UNIT_ABIL=10;		//Максимальное количество начальных умений у юнита
const int MAX_UPG_PER_LVL=6;	//Максимальное количество возможных улучшений за уровень юнита
const int MAX_LOOT_PER_LVL=5;	//Максимальное количество предметов лута за уровень юнита
const int MAX_UNIT_KIND=5;		//Максимальное количество видов юнитов (используется для перечисления в в объекте заклинания)
const int MAX_ANTI_EFFECT=10;	//Максимальное количество эффектов, препятствующих заклинанию
const int MAX_SPELL_EFFECT=20;	//Максимальное количество эффектов, накладываемых заклинанием
const int MAX_HERO_SKILL=28;	//Максимальное количество умений
const int MAX_SKILL_UPG=3;		//Максимум улучшений за уровень умения героя
const int MAX_ITEM_EFFECT=6;	//Максимум свойств у предмета
const int MAX_ITEM_RES=4;		//Максимум ресурсов, требуемых для изготовления предмета
const int MAX_CLASS_NAME=36;	//Максимум классов героев
const int MAX_TALENT=4;			//Максимум талантов при получении героем новой специальности
const int MAX_ITEM_SLOT_NAME=13;//Максимальное количество типов слотов предметов
const int MAX_GUARD_UNIT=16;	//Максимум типов юнитов в охраннике
const int MAX_GUARD_LOOT=10;	//Максимум возможных предметов лута в охраннике
const int MAX_GUARD_LEVEL=4;	//Максимальный уровень охранника
const int MAX_DEFENDER_RES=4;	//Максимальное количество ресурсов, необходимых для найма стража
const int MAX_DEFENDER_ABILITY=6;//Максимальное количество особых свойств стража
const int MAX_PROVINCE_GUARD=4;	//Максимальное количество типов стражей в провинции
const int MAX_PROVINCE_EVENT=10;//Максимальное количество событий, генерируемых провинцией
const int MAX_PROVINCE_LEVEL=11;//Максимальный уровень провинции
const int MAX_PROVINCE_SITE=30;	//Максимальное количество особых мест в провинции
const int MAX_BUILDING_RES=6;	//Максимум ресурсов, требуемых для постройки здания
const int MAX_RESOURCE_BONUS=3;	//Максимальное количество ресурсов, доход от которых изменяет здание
const int MAX_BUILDING_ABILITY=7;//Максимальное количество особых свойств здания
const int MAX_SITE_GUARD=30;	//Максимальное количество типов охранников сайта
const int MAX_SITE_ABILITY=7;	//Максимальное колисчество особых свойств сайта
const int MAX_RITUAL_EFFECT=5;	//Максимальное количество эффектов ритуала
const int MAX_RITUAL_RES=3;		//Максимум ресурсов, требуемых для проведения ритуала
const int MAX_NAME_TYPE=5;		//Максимум допустимых типов провинции для названия
const int MAX_ANSWER=4;			//Максимум вариантов ответов в диалоговом окне
const int MAX_BUILDING_NEED=5;	//Максимум построек-предшественниц для постройки в замке
const int MAX_BLD=1000;			//Максимум внутренних построек
const int MAX_BLD_GROUP=30;		//Максимум групп внутренних построек
const int MAX_LOOT=5;			//Максимум предметов в сокровище
const int MAX_BODY_DISTANCE=6;	//Максимум ходов для транспортировки тела героя в замок
const int MAX_SET_ITEMS=10;		//Максимум предметов в одном сете
const int MAX_SET_BONUS=5;		//Максимум бонусов в одном сете
const int MAX_SET_ABILITY=5;	//Максимум свойств бонуса сета
const int MAX_MEDAL_EFFECT=10;	//Максимум свойств у награды
const int MAX_EVENT_EFFECT=10;	//Максимум эффектов у события
const int MAX_QUEST_TASK=4;		//Максимум составных частей в задании квеста
const int MAX_QUEST_ABILITY=4;	//Максимум особый свойств у квеста
const int MAX_MERCENARY=11;		//Максимум типов наемников в одной постройке
const int MAX_MERC_LEVEL=5;		//Максимум уровней наемников
const int MAX_MAPOBJ_TYPE=5;	//Максимум типов провинций для объекта карты
const int MAX_MAPOBJ_PIC=20;	//Максимум картинок для одного объекта карты
const int MAX_TERR_LAYER=4;		//Максимум слоёв при генерации карты
const int MUSIC_FADING=30;		//Продолжительность затухания музыки
const int MAX_SHARD_SIZE=8;		//Максимум видов осколков по размеру
const int MAX_SHARD_LEVEL=8;	//Максимум видов осколков по уровню
const int MAX_SHARD_BONUS_TYPE=20;	//Максимум видов бонусов для осколка
const int MAX_CASTLE_BLD=20;	//Максимум построек в замке, привязанных к графическому одному объекту
const int MAX_GROUP=7;			//Максимум групп зданий с разными координатами
const int MAX_GROUP_BUILD=4;	//Максимум зданий в группе
const int MAX_PLAYER_BLD=20;	//Максимум предпочтительных простроек у компьютерного игрока
const int MAX_PLAYER_DIALOG=50;	//Максимум диалогов, завязанных на компьютерного игрока
const int MAX_MAP_TYPE=2;		//Максимум типов генерируемой карты
const int MAX_CAMP_QUEST=20;	//Максимум заданий в кампании
const int MAX_TICKET_TYPE=17;	//Максимум разных лотерейных билетов
const int MAX_WORLD_EVENTS=20;	//Максимум модификаторов вероятностей событий для особого правила
const int MAX_WORLD_PROVTYPES=5;//Максимум модификаторов вероятностей типов провинций для особого правила
const int MAX_WORLD_SITES=5;	//Максимум модификаторов вероятностей сайтов для особого правила
const int MAX_DIFFICULTY=7;		//Количество уровней сложности

//Константы динамических классов
const int EXP_FIRST_LEVEL=20;	//Опыт, необходимый для достижения юнитом 1-го уровня
const int EXP_ADD=10;			//Увеличение требуемого опыта за каждый уровень
const int SPELLBOOK_SIZE=21;	//Максимум заклинаний, которые может запомнить герой
const int INVENTORY_SIZE=25;	//Максимальное количество предметов у героя (включая надетые)
const int HERO_SKILL_LIMIT=11;	//Максимум профессий, которые может изучить герой
const int HERO_QUEST_LIMIT=10;	//Максимум заданий, которые может взять герой
const int MAX_PLAYERS=17;		//Максимальное число игроков (Нулевой игрок-нейтрал)
const int MAX_HERO_PIC=16;		//Максимум портретов героев
const int MAX_AVATAR_PIC=500;	//Максимум портретов правителей
const int MAX_NEIGHBOUR=10;		//Максимум соседних провинций
const int SHOP_SIZE=108;		//Максимум предметов в магазине
const int TREASURY_SIZE=36;		//Максимум предметов в сокровищнице
const int LIBRARY_SIZE=33;		//Максимум заклинаний в библиотеке
const int MAX_PROVINCE_BLD=3;	//Максимумм построек в провинции
const int MAX_HERO=8;			//Максимум героев у одного игрока
const int MAX_RECRUIT=12;		//Максимум типов рекрутов в замке
const int MAX_DEAL=12;			//Максимум типов торговых сделок
const int MAX_SHARD_BONUS=5;	//Максимум бонусов с одного осколка
const int MAX_SHARD_EVENT=5;	//Максимум особых событий на осколке
const int MAX_SHARD_BUILD=4;	//Максимум открываемых осколком построек
const int MAX_SHARD_BONUS_PIC=7;//Максимум иконок бонусов осколков
const int MAX_CAMPERS=120;		//Максимум запоминаемых вражеских героев на охране провинций

//Константы интерфейса
const int MAX_SOUNDTRACK=8;
const int MAX_FADE_SPEED=3;
const int MAX_MOUSE_POINTER=20;
const int MAX_BUTTON_BMP=381;
const int MAX_SHARD_PIC=6;
const int MAX_STUFF=81;
const int MAX_DIALOG_WINDOW=4;
const int SCROLL_SPEED=9;
const int MAX_HINT_W=400, MAX_HINT_H=700;
const int MAX_MISSILE=12;

const int MAX_ILLUSTRATION_ARR=1000;	//Максимум картинок для диалогов и событий (размер массива)
const int MAX_HILL_PIC=3;		//Максимум картинок холмов для глобальной карты
const int MAX_TREE_PIC=7;		//Максимум картинок деревьев для глобальной карты
const int MAX_MOUNT_PIC=2;		//Максимум картинок гор для глобальной карты
const int MAX_SWAMP_PIC=7;		//Максимум картинок болотных объектов для глобальной карты
const int MAX_GRASS_PIC=7;		//Максимум картинок травяных объектов для глобальной карты
const int MAX_MUSIC=18;			//Количество музыкальных композиций
const int MAX_HERB_SIGN=1000;		//Количество знаков на гербе
const int MAX_HERB_COLOR=16;	//Количество гербовых цветов

//Константы игровых параметров
const int MAX_ROLL=1000;		//размер массива вероятностей
const int UNIT_LEVEL_CAP=30;	//Максимальный уровень опыта юнитов
const int ATTITUDE_TIME=20;		//Через сколько ходов жители забывают о подвигах правителя
const int GUARD_RESS_TIME=4;	//Через сколько ходов воскрешаются воины охранников и стражей
const int MAX_UNREST=500;		//Недовольство, при котором возникает бунт
const int NO_CORRUPTION=18;		//Максимум провинций без коррупции на лёгком уровне сложности
const int SCOUTING_PRICE=30;	//Стоимость разведки
const int EXPLORE_EXP=4;		//Макс. количество опыта за ход исследования провинции
const int HOUR_SHIFT=0;//315;	//Сдвиг песочных часов на некоторых экранах

//Константы музыки
const int MUZ_ASTRAL=0;
const int MUZ_MAIN=1;
const int MUZ_MAX_MAIN=11;
const int MUZ_MEMORY=MUZ_MAX_MAIN+1;
const int MUZ_MENU=MUZ_MAX_MAIN+2;
const int MUZ_PREPARE=MUZ_MAX_MAIN+3;
const int MUZ_WAR=MUZ_MAX_MAIN+4;
const int MUZ_MAX_WAR=6;
const int MUZ_VICTORY=MUZ_WAR+MUZ_MAX_WAR;
const int MUZ_LOSE=MUZ_WAR+MUZ_MAX_WAR+1;
const int MUZ_GREMLIN=MUZ_WAR+MUZ_MAX_WAR+2;

//Константы генерируемой карты
const int Map_w=175;		//Размеры генерируемых провинций в пикселях
const int Map_h=175;		//	--"--
const int MAP_SIZE=6000;	//Максимальный размер карты по вертикали (в пикселях)
const int MAX_OUTBLD_PIC=4;	//Количество картинок с внешними зданиями (для карты)

extern int arHerbColors[MAX_HERB_COLOR][3];

extern bool bBetha;
extern char stVersion[16];
extern bool bDebugMode;

//Настройки игры
extern int flgEnglish;
extern bool	bSound;					//Вкл/выкл звук
extern bool	bWindowMode;
extern bool	bBackgroundMode;
extern int	iRefreshDelay;
extern int	iMouseDelay;
extern int	iKeyboardDelay;
extern int	iKeyboardRepeat;
extern int	iOggBuffer;
extern int	iColorDepth;
extern int	iSleepTime;
extern bool bGlobalError;
extern bool bDemo;
extern bool bIntro;
extern bool bLakeShip;

extern bool bStaticLoot;
extern bool bStaticEvent;
extern bool bShowWallpaper;			//Показывать ли картинку между ходами
extern int RES_BUY_CHANGE;			//Изменение цены на ресурс при его покупке в %
extern int MAX_PROFIT;				//Максимальный доход в процентах от золота игрока
extern int MERCENARY_TIME;			//Через сколько ходов сменяются наемники
extern int EVENT_POSSIBILITY;		//Вероятность случайных событий (из 1000)
extern int MAX_BATTLE_DURATION;		//Максимальная продолжительность боя в ходах (после чего наступает ничья)
extern int MAX_ILLUSTRATION;		//Максимум картинок для диалогов и событий
extern int MAX_PATH;				//Максимальная длина поиска пути

extern bool bUseWorldRules;

extern int iAntisleep;
extern int iAntisleepDelay;


extern bool bFullLogMode;
extern bool bDarken;				//Флаг гаснущего экрана
extern int	RollerP[MAX_ROLL],		//Массив вероятностей
			RollerN[MAX_ROLL];		//Массив индексов
extern bool bGameInProcess;			//Флаг, принимающий ложное значение при выходе из программы (используется деструкторами)
extern int Tick;					//Счётчик тиков для анимаций (0-1000)
extern int iHeroPriceMult;			//Множитель цены для найма героя той же профессии
extern int arHeroPrice[4][2];		//Цены на найм героев с 1 по 4 (золото, кристаллы)
extern int arHeroRess[2][2];		//Цена на воскрешение героя 0 уровня и добавка цены за каждый уровень
extern int arStat [3][10][3];		//[Здоровье/Магия/Лидер][Величина параметра][+жизнь,выносливость/слоты]
extern int arStatProgress [3][20];	//Рост параметров с уровнем для каждого из 4-х базовых классов и 16-ти профессиональных
extern int arBldGroup [MAX_BLD_GROUP]; //Максимум доступных построек в каждой группе
extern int arTerrLayer[MAX_TERRAIN+2][MAX_TERR_LAYER];	//Массив вероятностей появления спрайта на указанном слою указанной территории
extern int MapSize;					//Размер карты мира в провинциях
extern int arMercenary[MAX_MERC_LEVEL][MAX_MERCENARY][2];	//Наемники {уровень постройки}(тип, вероятность)
extern int arHeroLvlUp[2][4];
extern int iMaxShop;
extern int iMaxHerbSign;
extern int iMaxAvatarPic;
extern int DialogDX, DialogDY;		//Смещение окна диалога относительно центра экрана
extern int EventDX;					//Смещение окна диалога, вызываемого из событий, относительно центра экрана
extern int LootTable[MAX_LOOT][2];	//Список сокровищ и т.п. (индекс, прочность)
extern int Treasurez[6][2];			//Список полученных в событии сокровищ для передачи в диалог [0][0] - золото, [0][1] кристаллы
extern int Recruitz[MAX_RECRUIT];	//Список воинов для найма в особых местах
extern int arGroupCoord[MAX_GROUP][MAX_GROUP_BUILD][2];	//Таблца координат построек в группе [группа][постройка][0-x 1-y]

extern int arComp[5];
extern int NullColor;

extern int MovePlayer, WalkType, WalkSpeed, MinLength;
extern int * pMoveMap;
extern int * pRunMap;
extern int * pMobilMap;				//Запас хода при достижении провинции

extern int iActivePlayer;			//Индекс профайла активного игрока
extern int CurrentPlayer;			//Индекс текущего игрока
extern bool bTimeShift;				//Флаг доступности возврата во времени
extern bool bMessageFor;			//Флаг выдачи подсказки о том, кому адресовано сообщение (для игры по HotSeat)
extern int iMessageFor;				//Индекс игрока, получающего сообщение
extern bool bSmartAI;				//Если истинно - AI думает дольше
extern bool bVerySmartAI;
extern bool bAIBattle;				//Флаг для симуляции боя с потерями для AI
extern bool bExitGame;				//Флаг выхода из игры
extern bool bQuestComplete;			//Флаг выполнения задания героем в приключении (для повышения уровня)
extern int flgEncResult;			//Флаг результатов переговоров с нейтралами
extern int arLootTable[9][2];		//Список предметов лута для передачи в окно взятия лута
extern int iMusicVolume;			//Громкость музыки
extern int iSoundVolume;			//Громкость звуковых эффектов
extern int iHourGlass;				//Счётчик кадров для песочных часов
extern bool bHourGlass;				//Флаг песочных часов
extern bool bScrShot;				//Флаг скриншотов
extern int	iScrShot;				//Счётчик скриншотов
extern int	iExitFlag;				//Флаг выхода из игры
extern bool bArena;					//Если истинно - не давать лут после боя
extern int iScouting;				//Плата золота за разведку. Флаг для вывода диалога перед атакой.
extern int iBribe;					//Плата золота за подкуп стража. Флаг для вывода диалога перед атакой.
extern int MainTrack;				//Номер текущей музыки

extern PACKFILE *LogFile;
extern int HintW, HintH, 
HintType, //1 - Item, 2 - Ability, 3 - String, 4 - Building, 5 - Group, 6 - Text, 7 - BF_Unit, 8 - YuPlay Ticket, 9 - Site
HintMember, HintDur;
extern bool bHint;
extern int Timer;						//Флаг опроса таймера
extern long int NewProgress, OldProgress;	//Счетчик прогресса для градусника
extern int mouse_pointer;	//Вид курсора мыши
extern bool bButtonDelay;
extern bool bShowAnim;
extern bool bAnimation;		//Флаг анимации графических объектов
extern bool bTrueAnimation;	//То же, но без учёта смерти
extern bool bInput;			//Флаг ввода текста с клавиатуры
extern bool bSimulate;		//Флаг симуляции битвы
extern int iHeroAdvice;		//Флаг подсказки перед боем
extern bool BF_Cheat;		//Разрешить мухляж в бою
class dcHero;
extern int iAdvice;
extern dcHero *pAdvice;
extern int iDialogButt;			//Флаг кнопки просмотра героя перед боем
extern char stScouting[2000];	//Текст с информацией о противнике
extern int MovePlayer, WalkType, WalkSpeed, MinLength, PathKicker; //переменные для поиска пути на глобальной карте


extern int	iGameMode,			//Текущий режим игры (0 меню, 1 - астрал, 2 - основной, 3 - битва, 4- перед битвой)
			iActiveTrack,		//№ текущего трека
			iPause;				//Длительность паузы
extern float fFading;			//Затухание звука

extern int CastleProv;			//Флаг для перехода от замка к провинции

extern unsigned int iSeed;

class clHumanProfile;
extern clHumanProfile arHumans[MAX_PLAYERS];

class dcGame;
extern dcGame CurGame;
class dcShop;
extern dcShop *pFirstShop;

//Элементы астрального уровня
extern BITMAP *bmpShard[MAX_SHARD_PIC];	//Изображения осколков
extern BITMAP *bmpShardSelect[MAX_SHARD_PIC];
extern BITMAP *bmpMainShard;			//Мир игрока
extern BITMAP *bmpMainShardSelect;
extern BITMAP *bmpAstral;
extern BITMAP *bmpShardBonus[MAX_SHARD_BONUS_PIC]; //Иконки бонусов осколков

//Элементы для отрисовки карты
extern BITMAP *bmpMapGrass;
extern BITMAP *bmpMapSwamp;
extern BITMAP *bmpMapSand;
extern BITMAP *bmpMapDark;
extern BITMAP *bmpMapWater[5];
extern BITMAP *bmpTown[MAX_RACE][9], *bmpTownShadow[MAX_RACE][9];
extern BITMAP *bmpMapSelect;	//Выделение провинции
extern BITMAP *bmpNoRoom, *bmpArena, *bmpLair, *bmpTemple, *bmpShop;
//Элементы гербов игроков
extern BITMAP *bmpHerbMain, *bmpHerbRandom, *bmpHerbBorder;
extern BITMAP *bmpFlagMain, *bmpFlagCol1, *bmpFlagCol2, *bmpTownFlag[3][3];
extern BITMAP *bmpHerbSign[MAX_HERB_SIGN], *bmpHerbSignS[MAX_HERB_SIGN];
extern BITMAP *bmpFlag[MAX_PLAYERS][4];
extern BITMAP *bmpHerb[MAX_PLAYERS];
//Элементы интерфейса
extern BITMAP *screen1;
extern BITMAP *bmpEmpty;		//Пустышка для невидимых объектов
extern BITMAP *bmpHourGlass[13];//Анимированные часы
extern BITMAP *Background, *bmpCorpse, *bmpDarkScreen;
extern BITMAP *bmpMousePointer[MAX_MOUSE_POINTER];
extern BITMAP *bmpButton[MAX_BUTTON_BMP];
extern BITMAP *bmpStuff[MAX_STUFF];
extern BITMAP *bmpDialogWindow[MAX_DIALOG_WINDOW];
extern BITMAP *bmpMap;
extern BITMAP *bmpHint;
extern BITMAP *bmpAnName[16];
extern BITMAP *bmpAnCrea[15];
extern BITMAP *bmpAnWeb[7];
extern BITMAP *bmpAnRoot[8];
extern BITMAP *bmpSkull;
extern BITMAP *bmpJaw;
extern BITMAP *bmpCloud[8];
extern BITMAP *bmpShadow;	//Картинка для временного хранения отзеркаленной тени
extern BITMAP *bmpExplo[9];	//Анимация взрыва
extern BITMAP *bmpMissile[MAX_MISSILE];				//Картинки снарядов
extern BITMAP *bmpIllustration[MAX_ILLUSTRATION_ARR];	//Массив картинок для диалогов и событий
extern BITMAP*bmpCheckBox, *bmpCheckBird;
extern BITMAP *bmpRed, *bmpBlack; //Красное и чёрное затенение для иконок
extern BITMAP *bmpGUI_SpellMask, *bmpGUI_SpellSelect, *bmpGUI_HeroTrade, *bmpGUI_HeroScreen, 
*bmpGUI_SpellShadow, *bmpGUI_UnitSelect, *bmpGUI_UnitMask, *bmpGUI_UnitClosed,
*bmpGUI_ButtTrashNorm, *bmpGUI_ButtTrashAct, *bmpGUI_ButtTrashInact,
*bmpGUI_DiffWindow, *bmpGUI_EndCamp, *bmpGUI_ButtExitPress,
*bmpGUI_Pointer, *bmpGUI_UnitLvlUp, *bmpGUI_ExpLow, *bmpGUI_ResourceInfo,
*bmpGUI_HealthLow, *bmpGUI_DefenderInfo, *bmpGUI_HeroLvlUp, *bmpGUI_HeroLvl10, 
*bmpGUI_Hint, *bmpGUI_HeroAdvice, *bmpGUI_RulerBar, *bmpGUI_SpellClosed, *bmpGUI_UnitInfo,
*bmpGUI_SpellInfo, *bmpGUI_Garrison, *bmpGUI_RecruitClosed, *bmpGUI_NoRes, *bmpGUI_Library,
*bmpGUI_SpellTrade,	*bmpGUI_Shop, *bmpGUI_ItemTrade, *bmpGUI_ItemInfo, *bmpGUI_Victory,
*bmpGUI_Lose, *bmpGUI_Dead, *bmpGUI_LowPanel, *bmpGUI_ProvinceBar, *bmpGUI_BuildBar, 
*bmpGUI_BuildPanel, *bmpGUI_BuildSelect, *bmpGUI_BuildMask, *bmpGUI_Building, *bmpGUI_RazePanel,
*bmpGUI_Defender, *bmpGUI_Site, *bmpGUI_Fort, *bmpGUI_Castle, *bmpGUI_HeroBar, *bmpGUI_OuterBuildInfo,
*bmpGUI_Treasure, *bmpGUI_HeroRecruit, *bmpGUI_HeroRecruitWindow, *bmpGUI_HeroResurrectWindow,
*bmpGUI_HeroDead, *bmpGUI_Profile, *bmpGUI_EndGameScreen, *bmpGUI_HighScore, *bmpGUI_UnitTypeInfo,
*bmpGUI_Statistics, *bmpGUI_UnitReward, *bmpGUI_DefChoose, *bmpGUI_TakeLoot, *bmpGUI_Necro,
*bmpGUI_StrHint, *bmpGUI_ItemEffect, *bmpGUI_SkillInfo, *bmpGUI_BattleUnitInfo, *bmpGUI_AbilityInfo,
*bmpGUI_Gold, *bmpGUI_Crystal, *bmpGUI_ItemSet, *bmpGUI_NewTurn, *bmpGUI_Screen01, *bmpGUI_Diplomacy,
*bmpGUI_Cursor, *bmpGUI_HeroPort, *bmpGUI_MainMenu, *bmpGUI_Eador, *bmpGUI_MenuScrN, *bmpGUI_MenuScrA,
*bmpGUI_NewGame, *bmpGUI_NGPlayer, *bmpGUI_Build, *bmpGUI_Built, *bmpGUI_NoGold, *bmpGUI_NoGem, *bmpGUI_InnBldHint,
*bmpGUI_InnBldInfo, *bmpGUI_UnitSlot, *bmpGUI_SpellSlot, *bmpGUI_String, *bmpGUI_GroupIcon, *bmpGUI_GroupHint, *bmpGUI_Planned,
*bmpGUI_AstralLow, *bmpGUI_ShardBar, *bmpGUI_ItemHint, *bmpGUI_HeroSelect[4], *bmpGUI_DeadSelect, *bmpGUI_Treasury,
*bmpGUI_HomeGarrison, *bmpGUI_HomeBarracks, *bmpGUI_HomeLibrary, *bmpGUI_ProgressMark, *bmpGUI_ProgressFill, *bmpGUI_MainShardBar,
*bmpGUI_SpellGlow, *bmpGUI_UnitGlow, *bmpGUI_EnterWindow, *bmpGUI_HerbWindow, *bmpGUI_PickBonus, *bmpGUI_PickUnit, *bmpGUI_PickSpell,
*bmpGUI_CampVictory, *bmpGUI_ChoosePlayer, *bmpGUI_ScrollN, *bmpGUI_ScrollA, *bmpGUI_Alchemy, *bmpGUI_MapSettings,
*bmpGUI_Broken, *bmpGUI_Anvil1, *bmpGUI_Anvil2, *bmpGUI_Menu, *bmpGUI_Settings, *bmpGUI_TurnWindow, *bmpGUI_ChangeWeapon, 
*bmpGUI_SmallHint, *bmpGUI_CampStat, *bmpGUI_ParamHint, *bmpGUI_Credits;
//Элементы карты
extern BITMAP *bmpHeroSelect;
extern BITMAP *bmpRebelDoll;
extern BITMAP *bmpOuterBld[MAX_OUTBLD_PIC];
extern BITMAP *bmpPath[4], *bmpQuest[2];
extern BITMAP *bmpUnhappy[3], *bmpUnrest, *bmpDefender;
extern BITMAP *bmpBorder;
extern BITMAP *bmpResBorder, *bmpResGuardBorder, *bmpResHidden;
//Графика юнитов и героев
extern BITMAP *bmpPortrait[MAX_HERO_PIC];
extern BITMAP *bmpSmallPort[MAX_HERO_PIC];
extern BITMAP *bmpAvatar[MAX_AVATAR_PIC];
extern BITMAP *bmpSmallAvatar[MAX_AVATAR_PIC];
extern BITMAP *bmpHeroSkill[MAX_HERO_SKILL*5+1];
extern BITMAP *bmpHeroDoll[4];
extern BITMAP *bmpHeroDollS[4];
extern BITMAP *bmpHeroDollShadow[4];
extern BITMAP *bmpFlipCorpse;

extern DATAFILE *datUnitSound;
extern DATAFILE *datSpellSound;
extern DATAFILE *datMusic;

class clAudioTrack;
extern ALOGG_OGG *oggTrack[MAX_MUSIC];
extern clAudioTrack trSound[MAX_SOUNDTRACK];

const int wawHeal=13;				//Звуковой эффект лечения
const int wavBerz=82;
const int wavMarch=83;
const int wavGather=84;
const int wavRound=25;
const int wavCrush=125;
const int wavClick=0;
const int wavSoul=130;
const int wavTrample=135;
const int wavEat=6;
const int wavRoot=4;
const int wavArmor=9;
const int wavWeapon=10;
const int wavCoins=155;
const int wavParchment=156;
const int wavBuild=157;
const int wavDelete=158;
const int wavRepair=28;
const int wavRepairAll=129;

//****** YouPlay ********

#ifdef YUPLAY
const int MAX_YUPLAY=15;

extern int iYuPlay;
extern int ipLottPrize, ipLottPrizeType;
extern bool bYuPlayClient;
extern BITMAP *bmpYuPlay[MAX_YUPLAY];
#endif

// Global  Service Defines
//округление любого дробного числа (полож/отриц) в меньшую (X<0.5) или большую (X>=0.5) сторону
#define ROUND(X)	( X>0 ? ROUND2(X) : -ROUND2(-X) )
#define ROUND2(X)	( (int(X*10)-int(X)*10) < 5 ? int(X): int(X)+1 )


// Global Services
// Global Service  Variables &&  Functions
extern int	 SCR_PROGRESSBAR_X;//grey
extern int	 SCR_PROGRESSBAR_Y;//grey
extern int	 SCR_PROGRESSBAR_STEP;//grey
extern int	 SCR_PROGRESSBAR_CURX;//grey
extern int	 SCR_PROGRESSBAR_CURY;//grey
//текстовая линейка прогресса для переменной Х с шагом step=100 а-ля Doom "."
void PROCESSBAR(int x,int step=100);//grey
void PROCESSBAR_XY(int _x,int _y, int x,int step=100);//grey
//пауза- ожидает нажатия ESC
void WAIT();//grey
void WAIT1();//grey

//
int GetWaterPixel();//grey
int GetCoastPixel();//grey

//перемешивание целочисленного массива arr с позиции pos1 до pos2 (nn)  (относительно середины массива pos/2)
//mix_len- кол-во эл-тов для переноса; mix_step- кол-во пропускаемых эл-тов (шаг смешивания); 
//rnd> 0 - случайное перемешивание эл-тов с i= Random(rnd)
int Mix_Array(int *arr, int pos1, int pos2, int mix_len=1, int mix_step=1, int rnd=0);
//перемешивание массива arr с позиции 0 до nn
int Mix_Array(int *arr, int nn);

typedef unsigned long L_INT;
void Randomizer();
int sign(int Zifer);
L_INT Random(L_INT Max);
L_INT Random(L_INT Max, int Param1, int Param2);
L_INT Random(L_INT Max, int Param1, int Param2, int Param3);
L_INT Random(L_INT Max, int Param1, int Param2, int Param3, int Param4);
L_INT Roll(bool Exclude);
L_INT Roll(bool Exclude, int Param1, int Param2);
L_INT Roll(bool Exclude, int Param1, int Param2, int Param3);
L_INT Roll(bool Exclude, int Param1, int Param2, int Param3, int Param4);
void ClearRoll();

int GetMin(int A1, int A2);
int GetMax(int A1, int A2);
extern bool bWorldRulesTest;

#endif

