#ifndef USED_WINDOWER
#define USED_WINDOWER

#include "Unit.h"
//#include "Interface.h"

const int SlotLvl2=7;
const int SlotLvl3=11;
const int SlotLvl4=14;
const int SpellLvl2=9;
const int SpellLvl3=15;
const int SpellLvl4=19;

class GUI_ItemBlock
{
public:
	static const int dx=-6;
	static const int dy=-6;
	static const int w=60;
	static const int h=59;//57
	static const int width=300;
	static const int height=470;
	static const int hMaskW=176;
	static const int hMaskH=279;
	static const int hMaskX=59;
	static const int hMaskY=0;
	int x, y;
	dcHero * pHero;
	BITMAP * Pic;
	BITMAP * MaskHero;
	UI_Mask arMask[INVENTORY_SIZE+1];

	GUI_ItemBlock();
	~GUI_ItemBlock();
	G_Object * Init(dcHero * Herr, int sx, int sy, G_Object * Obj);
	int Check(int Mode, dcShop *Shop, int Item);
	void Show();
};

//class GUI_UnitBlock
//{
//public:
//	static const int dx=-8;
//	static const int dy=-8;
//	static const int w=74;
//	static const int h=91;
//	static const int width=440;
//	static const int height=360;
//	int x, y;
//	dcHero * pHero;
//	BITMAP * Pic;
//	BITMAP *bmpHealth, *bmpExp;
//	UI_Mask arMask[MAX_SQUAD];
//
//	GUI_UnitBlock();
//	~GUI_UnitBlock();
//	G_Object * Init(dcHero * Herr, int sx, int sy, G_Object * Obj);
//	int Check();	
//	void Show();
//};
class GUI_UnitBlock
{
public:
	static const int dx=-8;
	static const int dy=-8;
	static const int w=72;
	static const int h=88;
	static const int width=440;
	static const int height=360;
	int x, y;
	dcHero * pHero;
	BITMAP * Pic;
	BITMAP *bmpHealth, *bmpExp;
	UI_Mask arMask[MAX_SQUAD];

	GUI_UnitBlock();
	~GUI_UnitBlock();
	G_Object * Init(dcHero * Herr, int sx, int sy, G_Object * Obj);
	int Check(dcUnit *pUnit);
	void Show();
};

class GUI_StatBlock
{
public:
	static const int dx=53;//53 46;
	static const int dy=10;//10 12;
	static const int w=60;
	static const int h=31;//30 29;
	static const int width=186;
	static const int height=116;
	int x, y;
	dcHero * pHero;
	BITMAP * Pic;
	UI_Mask1 arMask[12];
	GUI_StatBlock();
	~GUI_StatBlock();
	G_Object * Init(dcHero * Herr, int sx, int sy, G_Object * Obj);	
	void Show();
	void Check();
};

class GUI_SkillBlock
{
public:	
	static const int w=60;
	static const int h=60;
	static const int width=130;
	static const int height=420;
	int x, y;
	UI_Mask arMask[14];
	dcHero * pHero;
	BITMAP * Pic;
	GUI_SkillBlock();
	~GUI_SkillBlock();
	G_Object * Init(dcHero * Herr, int sx, int sy, G_Object * Obj);	
	void Show();
	void Check();
};

class GUI_QuestBlock
{
public:		
	static const int h=39;
	static const int width=340;
	static const int height=470;
	int x, y;
	BITMAP * Pic, *Cursor;
	dcHero * pHero;	
	UI_Mask arMask[HERO_QUEST_LIMIT];
	GUI_QuestBlock();
	~GUI_QuestBlock();
	G_Object * Init(dcHero * Herr, int sx, int sy, G_Object * Obj);	
	void Show();
	void Check();
};

class GUI_InfoBlock
{
public:
	static const int w=60;
	static const int width=195;
	static const int height=270;
	int x, y;
	float fHealth;	//Соотношение текущего и максимального здоровья героя
	int iOldLife, iOldMaxLife;
	dcHero * pHero;
	BITMAP * Pic;
	BITMAP *bmpHealth, *bmpExp;
	GUI_InfoBlock();
	~GUI_InfoBlock();
	G_Object * Init(dcHero * Herr, int sx, int sy, G_Object * Obj);
	UI_Mask1 mskHero;
	UI_Mask1 arMask[6], mskLevel, mskLife;
	void Show();
	void Check();
};

class GUI_ResourceBlock
{
public:	
	static const int w=26;	
	static const int width=512;
	static const int height=25;
	int x, y;
	dcPlayer * Player;
	BITMAP * Pic;
	UI_Mask1 mskRes[9];
	UI_Mask1 mskAlchemy;
	GUI_ResourceBlock();
	~GUI_ResourceBlock();
	G_Object * Init(dcPlayer * sPlayer, int sx, int sy, G_Object * Obj);	
	void Show();
	bool Check(G_Object * Obj);
};

class GUI_SpellBlock
{
public:
	static const int dx=-6;
	static const int dy=-6;
	static const int w=60;
	static const int h=60;//66
	static const int width=470;
	static const int height=270;
	int x, y;
	dcHero * pHero;
	BITMAP * Pic;
	UI_Mask arMask[SPELLBOOK_SIZE+1];

	GUI_SpellBlock();
	~GUI_SpellBlock();
	G_Object * Init(dcHero * Herr, int sx, int sy, G_Object * Obj);
	int Check(int Spell);
	void Show();
};

class GUI_RecruitBlock
{
public:
	static const int dx=-8;
	static const int dy=-8;
	static const int w=81;
	static const int rw=22;
	static const int h=179;
	static const int width=512;
	static const int height=370;
	int x, y, ipProv;
	int arCost[2][MAX_RECRUIT];
	int arRecr[MAX_RECRUIT];
	bool bLocalTab;
	dcPlayer * Player;
	BITMAP * Pic;
	UI_Mask arMask[MAX_RECRUIT];
	UI_Mask1 mskRes[MAX_RECRUIT][MAX_UNIT_RES];

	GUI_RecruitBlock();
	~GUI_RecruitBlock();
	G_Object * Init(dcPlayer * sPlayer, int sx, int sy, G_Object * Obj, bool UseLocalTab, int Prov);
	void Recalc();
	int Check();	
	void Show();
};

class GUI_LibraryBlock
{
public:
	static const int dx=90;
	static const int dy=30;
	static const int w=60;
	static const int L0=493;
	static const int L1=423;
	static const int L2=363;
	static const int L3=262;
	static const int L4=202;
	static const int L5=101;
	static const int width=360;
	static const int height=580;
	int x, y;
	dcPlayer * Player;
	BITMAP * Pic;
	UI_Mask arMask[LIBRARY_SIZE];

	GUI_LibraryBlock();
	~GUI_LibraryBlock();
	G_Object * Init(dcPlayer * sPlayer, int sx, int sy, G_Object * Obj);
	int Check();	
	void Show();
};

class GUI_ShopBlock
{
public:
	static const int dx=-6;
	static const int dy=-6;
	static const int w=55;
	static const int h=55;	
	static const int rw=26;
	static const int rh=25;
	static const int width=500;
	static const int height=750;
	int x, y,
		PickFlag, //Флаг сосотяния информационного окна. 0 - пустое, 1 - продажа, 2 - покупка, 3 - ремонт
		PickItem, PickDur; //Предмет в инфоокне

	dcHero * Hero;
	dcPlayer * Player;
	dcShop * Shop;
	BITMAP * Pic;
	UI_Mask arMask[SHOP_SIZE];
	UI_Mask1 mskSell;
	UI_Button1 buttRepairAll;
	UI_Button2 buttRepair;

	GUI_ShopBlock();
	~GUI_ShopBlock();
	G_Object * Init(dcHero * sHero, dcShop * sShop, int sx, int sy, G_Object * Obj);
	int Check(dcHero *Hero);	
	void Show();
	void Pick(int Flag, int Item, int Dur);
};

class GUI_TreasuryBlock
{
public:
	static const int dx=-6;
	static const int dy=-6;
	static const int w=55;
	static const int h=55;	
	static const int width=500;
	static const int height=225;
	int x, y;	
	dcPlayer * Player;	
	BITMAP * Pic;
	UI_Mask arMask[TREASURY_SIZE];
	
	GUI_TreasuryBlock();
	~GUI_TreasuryBlock();
	G_Object * Init(dcPlayer * sPlayer, int sx, int sy, G_Object * Obj);
	int Check(dcHero *Hero, int Mode, dcShop *Shop);	
	void Show();
};

class GUI_ProvBar
{
public:
	static const int MAX_SITE=12;
	int x, y,		
		Layer,
		Mode,
		Sites,
		SiteLimit,
		Shift,
		arSite[MAX_SITE];
	bool bCheckable;

	dcProvince *PR;
	BITMAP *Pic;
	G_Object *Obj;
	UI_Button1 buttSelect, buttLeft, buttRight;
	UI_Mask mskRace, mskRes, mskGuard, mskSite, mskBuild[3], mskFort, mskS[MAX_SITE];
	UI_Mask1 mskGold, mskGem, mskTerra, mskMood, mskLevel, mskHome;
	GUI_ProvBar();
	~GUI_ProvBar();
	G_Object * Init(int st_y, int st_Layer, dcProvince *Prov, bool Buttons, G_Object * st_Obj);
	void Show();
	void Activate(dcProvince *Prov);
	void Hide();
	int Check(G_Object * objMain, int MaxLayer);
};

int SlotNum(int Slot);	//Возвращает номер слота предмета для указанного слота на "кукле" героя
int SlotLvl(int Slot);	//Возвращает максимальный уровень юнита для указанного слота
int GetFreeUnitSlot(dcHero * Hero, int Level); //Возвращает свободный слот, подходящий для юнита указанного уровня

#endif
