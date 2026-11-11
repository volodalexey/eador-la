#include "Windower.h"

const int BLD_IN=18;	//Максимум построек на экране замка
int LootTable[MAX_LOOT][2];
int arBuilding[BLD_IN];

int Under; //Флаг объекта под курсором мыши

//Возвращает номер слота предмета для указанного слота на "кукле" героя
int SlotNum(int Slot) 
{
	int Result=Slot;
	switch (Slot)
	{
	case 11:
		Result=10;
		break;	
	case 12:
	case 13:
	case 14:
	case 15:
		Result=11;
	break;
	};
	return Result;
};

//Возвращает номер первого свободного слота в сумке героя или 0 если сумка заполнена
int GetFreeSlot(dcHero *Hero, int Exclude)
{
	for (int i=16; i<INVENTORY_SIZE+1; i++)
	{
		if ((Hero->arInventory[i][0]==0)&&(i!=Exclude))
		{return i;};
	};
	return 0;
};


//Возвращает максимальный уровень юнита для указанного слота
int SlotLvl(int Slot)
{
	if (Slot<SlotLvl2)
	{return 1;};
	if (Slot<SlotLvl3)
	{return 2;};
	if (Slot<SlotLvl4)
	{return 3;};
	return 4;
};

//Возвращает максимальный уровень заклинания для указанного слота
int SpellSlotLvl(int Slot)
{
	if (Slot<SpellLvl2)
	{return 1;};
	if (Slot<SpellLvl3)
	{return 2;};
	if (Slot<SpellLvl4)
	{return 3;};
	return 4;
};

//Возвращает свободный слот, подходящий для юнита указанного уровня
int GetFreeUnitSlot(dcHero * Hero, int Level)
{
	switch (Level)
	{
	case 1:
		for (int i=1; i<=Hero->GetUnitSlot(1); i++)
		{
			if (!Hero->pSquad[i])
			{return i;};
		};
	case 2:
		for (int i=SlotLvl2; i<SlotLvl2+Hero->GetUnitSlot(2); i++)
		{
			if (!Hero->pSquad[i])
			{return i;};
		};
	case 3:
		for (int i=SlotLvl3; i<SlotLvl3+Hero->GetUnitSlot(3); i++)
		{
			if (!Hero->pSquad[i])
			{return i;};
		};
	case 4:
		for (int i=SlotLvl4; i<SlotLvl4+Hero->GetUnitSlot(4); i++)
		{
			if (!Hero->pSquad[i])
			{return i;};
		};
	};
	return 0;
};

//Возвращает свободный слот, подходящий для заклинания указанного уровня
int GetFreeSpellSlot(dcHero * Hero, int Level)
{
	switch (Level)
	{
	case 0:
	case 1:
		for (int i=1; i<=Hero->GetSpellSlot(1); i++)
		{
			if (Hero->arSpell[i]==0)
			{return i;};
		};
	case 2:
		for (int i=SpellLvl2; i<SpellLvl2+Hero->GetSpellSlot(2); i++)
		{
			if (Hero->arSpell[i]==0)
			{return i;};
		};
	case 3:
		for (int i=SpellLvl3; i<SpellLvl3+Hero->GetSpellSlot(3); i++)
		{
			if (Hero->arSpell[i]==0)
			{return i;};
		};
	case 4:
		for (int i=SpellLvl4; i<SpellLvl4+Hero->GetSpellSlot(4); i++)
		{
			if (Hero->arSpell[i]==0)
			{return i;};
		};
	};
	return 0;
};

void DrawUnit(BITMAP *Pic, dcUnit *Unit, int x, int y, int Moraler, BITMAP *bmpExp, BITMAP *bmpHealth, dcHero *Hero)
{
//	draw_sprite(Pic,bmpGUI_HealthFull,x,y);
 	draw_sprite(Pic,Unit->pType()->Icon,x,y);
//	set_trans_blender(60,60,60,60);
//	draw_trans_sprite(Pic,bmpGUI_ExpFull,x,y+65);	
	if (Moraler!=-1000)
	{
		if (Moraler>0)
		{
			textprintf_ex(Pic,Font1,x+23,y+3,COLOR_1,-1,"+%d",Moraler);
			textprintf_ex(Pic,Font1,x+22,y+2,makecol(50,220,50),-1,"+%d",Moraler);
		};
		if (Moraler<0)
		{
			textprintf_ex(Pic,Font1,x+23,y+3,COLOR_1,-1,"%d",Moraler);
			textprintf_ex(Pic,Font1,x+22,y+2,makecol(220,50,50),-1,"%d",Moraler);
		};

	};
	blit(bmpGUI_HealthLow,bmpHealth,0,0,0,0,bmpGUI_HealthLow->w,bmpGUI_HealthLow->h);
	blit(Pic,bmpHealth,x+59,y+2,0,0,bmpGUI_HealthLow->w,bmpGUI_HealthLow->h*(Unit->GetLife()-Unit->iLife)/Unit->GetLife());
	draw_sprite(Pic,bmpHealth,x+59,y+2);
	if (Unit->iLife==0)
	{
		set_trans_blender(80,80,80,80);
		draw_trans_sprite(Pic,bmpGUI_Dead,x,y);
	};
	blit(Pic,bmpExp,x+1,y+1,0,0,bmpGUI_ExpLow->w,bmpGUI_ExpLow->h);
	int YExp;
	if (Hero)
	{
		YExp=bmpGUI_ExpLow->h*(Hero->ExpForLevel(Hero->iLevel+1)-Hero->iExp)/
		(Hero->ExpForLevel(Hero->iLevel+1)-Hero->ExpForLevel(Hero->iLevel));
		textprintf_centre_ex(Pic,Font1,x+23,y+70,COLOR_1,-1,"%d",Hero->iLevel);
		textprintf_centre_ex(Pic,Font1,x+22,y+69,makecol(220,100,255),-1,"%d",Hero->iLevel);
	}
	else
	{
		YExp=bmpGUI_ExpLow->h*(Unit->ExpForLevel(Unit->iLevel+1)-Unit->iExp)/
		(Unit->ExpForLevel(Unit->iLevel+1)-Unit->ExpForLevel(Unit->iLevel));
		textprintf_centre_ex(Pic,Font1,x+23,y+70,COLOR_1,-1,"%d",Unit->iLevel);
		textprintf_centre_ex(Pic,Font1,x+22,y+69,makecol(200,100,255),-1,"%d",Unit->iLevel);
		for (int j=0; j<3; j++)
		{
			if (Unit->arMedal[j]>0)
			{
				draw_sprite(Pic,(pFirstMedal+Unit->arMedal[j])->SmallPic,34+x+j*10,y+67);				
			};
		};	
	};
	blit(bmpGUI_ExpLow,bmpExp,0,YExp,0,YExp,bmpGUI_ExpLow->w,bmpGUI_ExpLow->h-YExp);
	draw_sprite(Pic,bmpExp,x+1,y+2);
	
};

//***************************************************
//*** Методы ItemBlock ******************************
//***************************************************

GUI_ItemBlock::GUI_ItemBlock()
{
	pHero=NULL;
	Pic=create_bitmap(width,height);
	MaskHero=create_bitmap(hMaskW,hMaskH);
	rectfill(MaskHero,0,0,hMaskW,hMaskH,makecol(0,0,0));
};

GUI_ItemBlock::~GUI_ItemBlock()
{
	if (bGameInProcess)
	{
		destroy_bitmap(Pic);
		destroy_bitmap(MaskHero);
	};
};

G_Object * GUI_ItemBlock::Init(dcHero *Herr, int sx, int sy,  G_Object * Obj)
{	
	x=sx;
	y=sy;
	pHero=Herr;
	Obj=new G_Object(Obj,x,y,2,Pic);
	Obj=arMask[0].Init(x+hMaskX, y+hMaskY, 1, MaskHero, bmpMousePointer[1], Obj, -1);
	Obj=arMask[1].Init(x+dx, y+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	Obj=arMask[2].Init(x+w*4+dx, y+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	Obj=arMask[3].Init(x+dx, y+h*2+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	Obj=arMask[4].Init(x+dx, y+h+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	Obj=arMask[5].Init(x+dx, y+h*3+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	Obj=arMask[6].Init(x+w*4+dx, y+h*2+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	Obj=arMask[7].Init(x+dx, y+h*5+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	Obj=arMask[8].Init(x+w*4+dx, y+h*3+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	Obj=arMask[9].Init(x+w*4+dx, y+h+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	Obj=arMask[10].Init(x+dx, y+h*4+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	Obj=arMask[11].Init(x+w*4+dx, y+h*4+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	for (int i=1; i<5; i++)
	{
		Obj=arMask[11+i].Init(x+w*i+dx, y+h*5+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, bmpGUI_SpellGlow, Obj, 3);
	};
	for (int i=0; i<5; i++)
	{
		Obj=arMask[16+i].Init(x+w*i+dx, y+359+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, Obj, 3);
		Obj=arMask[21+i].Init(x+w*i+dx, y+419+dy, 1, bmpGUI_SpellMask, bmpGUI_SpellSelect, Obj, 3);
	};
	Show();
	return Obj;
};

int GUI_ItemBlock::Check(int Mode, dcShop *Shop, int Item)
{
	for (int i=0; i<16; i++)
	{
		if ((Item>0)&&(pHero->CanUse(Item))&&(
			(SlotNum(i)==(pFirstItem+Item)->ipSlot)||(((pFirstItem+Item)->ipSlot==100)&&(SlotNum(i)==1))))
		{arMask[i].bGlow=true;}
		else
		{arMask[i].bGlow=false;};
	};
	for (int i=0; i<INVENTORY_SIZE+1; i++)
	{
		int Check=arMask[i].Check();
		if (Check>1)
		{
			Under=i;
			if (pHero->arInventory[i][0]>0)
			{
				int Gold=0, Gem=0;
				if ((Mode==14)&&(Shop))
				{
					
					Gold=arPlayer[pHero->ipPlayer].GetItemGoldRepPrice(pHero->arInventory[i][0],pHero->arInventory[Under][1],Shop);
					Gem=arPlayer[pHero->ipPlayer].GetItemGemRepPrice(pHero->arInventory[i][0],pHero->arInventory[Under][1],Shop);
				};
				if (Mode==12)
				{
		
					Gold=(pFirstItem+pHero->arInventory[i][0])->iGoldCost/4; 
					Gem=(pFirstItem+pHero->arInventory[i][0])->iGemCost/4;
					if ((pFirstItem+pHero->arInventory[i][0])->iDurability>0)
					{
						Gold-=(pFirstItem+pHero->arInventory[i][0])->iGoldCost*((pFirstItem+pHero->arInventory[i][0])->iDurability-pHero->arInventory[i][1])/(pFirstItem+pHero->arInventory[i][0])->iDurability/8;
						Gem-=(pFirstItem+pHero->arInventory[i][0])->iGemCost*((pFirstItem+pHero->arInventory[i][0])->iDurability-pHero->arInventory[i][1])/(pFirstItem+pHero->arInventory[i][0])->iDurability/8;
					};
				};
				MakeItemHint(pHero->arInventory[i][0],pHero->arInventory[i][1],pHero,Mode,Gold,Gem);
				bHint=true;
			};
		};
		if (Check==30)
		{
			HintType=0;
			return i;
		};
		if (Check==40)
		{
			if (pHero->arInventory[i][0]>0)
			{
				bHint=false;				
				WindowItemInfo(pHero->arInventory[i][0],pHero->arInventory[i][1]);
			};
	//		return 1000+i;
		};		
	};
	return -1;
};

void GUI_ItemBlock::Show()
{
	rectfill(Pic,0,0,width,height,makecol(255,0,255));
	if (pHero->arInventory[1][0]>0)
	{
		blit((pFirstItem+pHero->arInventory[1][0])->Pic,Pic,0,0,0,0,52,52);
		if (pHero->arInventory[1][1]==0)
		{
			draw_sprite(Pic, bmpGUI_SpellShadow, 0, 0);
		};
	};
	if (pHero->arInventory[2][0]>0)
	{
		blit((pFirstItem+pHero->arInventory[2][0])->Pic,Pic,0,0,w*4,0,52,52);
		if (pHero->arInventory[2][1]==0)
		{
			draw_sprite(Pic, bmpGUI_SpellShadow, w*4, 0);
		};
	};
	if (pHero->arInventory[3][0]>0)
	{
		blit((pFirstItem+pHero->arInventory[3][0])->Pic,Pic,0,0,0,h*2,52,52);
		if (pHero->arInventory[3][1]==0)
		{
			draw_sprite(Pic, bmpGUI_SpellShadow, 0, h*2);
		};
	};
	if (pHero->arInventory[4][0]>0)
	{
		blit((pFirstItem+pHero->arInventory[4][0])->Pic,Pic,0,0,0,h,52,52);
		if (pHero->arInventory[4][1]==0)
		{
			draw_sprite(Pic, bmpGUI_SpellShadow, 0, h);
		};
	};
	if (pHero->arInventory[5][0]>0)
	{
		blit((pFirstItem+pHero->arInventory[5][0])->Pic,Pic,0,0,0,h*3,52,52);
		if (pHero->arInventory[5][1]==0)
		{
			draw_sprite(Pic, bmpGUI_SpellShadow, 0, h*3);
		};
	};
	if (pHero->arInventory[6][0]>0)
	{
		blit((pFirstItem+pHero->arInventory[6][0])->Pic,Pic,0,0,w*4,h*2,52,52);
		if (pHero->arInventory[6][1]==0)
		{
			draw_sprite(Pic, bmpGUI_SpellShadow, w*4, h*2);
		};
	};
	if (pHero->arInventory[7][0]>0)
	{
		blit((pFirstItem+pHero->arInventory[7][0])->Pic,Pic,0,0,0,h*5,52,52);
		if (pHero->arInventory[7][1]==0)
		{
			draw_sprite(Pic, bmpGUI_SpellShadow, 0, h*5);
		};
	};
	if (pHero->arInventory[8][0]>0)
	{
		blit((pFirstItem+pHero->arInventory[8][0])->Pic,Pic,0,0,w*4,h*3,52,52);
		if (pHero->arInventory[8][1]==0)
		{
			draw_sprite(Pic, bmpGUI_SpellShadow, w*4, h*3);
		};
	};
	if (pHero->arInventory[9][0]>0)
	{
		blit((pFirstItem+pHero->arInventory[9][0])->Pic,Pic,0,0,w*4,h,52,52);
		if (pHero->arInventory[9][1]==0)
		{
			draw_sprite(Pic, bmpGUI_SpellShadow, w*4, h);
		};
	};
	if (pHero->arInventory[10][0]>0)
	{
		blit((pFirstItem+pHero->arInventory[10][0])->Pic,Pic,0,0,0,h*4,52,52);
		if (pHero->arInventory[10][1]==0)
		{
			draw_sprite(Pic, bmpGUI_SpellShadow, 0, h*4);
		};
	};
	if (pHero->arInventory[11][0]>0)
	{
		blit((pFirstItem+pHero->arInventory[11][0])->Pic,Pic,0,0,w*4,h*4,52,52);
		if (pHero->arInventory[11][1]==0)
		{
			draw_sprite(Pic, bmpGUI_SpellShadow, w*4, h*4);
		};
	};
	for (int i=1; i<5; i++)
	{
		if (pHero->arInventory[11+i][0]>0)
		{
			blit((pFirstItem+pHero->arInventory[11+i][0])->Pic,Pic,0,0,w*i,h*5,52,52);
			if (pHero->arInventory[11+i][1]==0)
			{
				draw_sprite(Pic, bmpGUI_SpellShadow, w*i, h*5);
			};
		};
	};
	for (int i=0; i<5; i++)
	{
		if (pHero->arInventory[16+i][0]>0)
		{
			blit((pFirstItem+pHero->arInventory[16+i][0])->Pic,Pic,0,0,w*i,359,52,52);
			if (pHero->arInventory[16+i][1]==0)
			{
				draw_sprite(Pic, bmpGUI_SpellShadow, w*i, 359);
			};
		};
		if (pHero->arInventory[21+i][0]>0)
		{
			blit((pFirstItem+pHero->arInventory[21+i][0])->Pic,Pic,0,0,w*i,419,52,52);
			if (pHero->arInventory[21+i][1]==0)
			{
				draw_sprite(Pic, bmpGUI_SpellShadow, w*i, 419);
			};
		};
	};
};

//***************************************************
//*** Методы UnitBlock ******************************
//***************************************************

GUI_UnitBlock::GUI_UnitBlock()
{
	pHero=NULL;
	Pic=create_bitmap(width,height);
	bmpExp=create_bitmap(bmpGUI_ExpLow->w,bmpGUI_ExpLow->h);
	bmpHealth=create_bitmap(bmpGUI_HealthLow->w,bmpGUI_HealthLow->h);
};

GUI_UnitBlock::~GUI_UnitBlock()
{
	if (bGameInProcess)
	{
		destroy_bitmap(Pic);
		destroy_bitmap(bmpExp);
		destroy_bitmap(bmpHealth);
	};
};

G_Object * GUI_UnitBlock::Init(dcHero * Herr, int sx, int sy, G_Object * Obj)
{
	x=sx;
	y=sy;
	pHero=Herr;
	Obj=new G_Object(Obj,x,y,2,Pic);
	for (int i=1; i<=pHero->GetUnitSlot(1); i++)
	{
		Obj=arMask[i].Init(x+dx+(i-1)*w,y+dy+h*3,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,bmpGUI_UnitGlow,Obj,3);
	};
	for (int i=SlotLvl2; i<pHero->GetUnitSlot(2)+SlotLvl2; i++)
	{
		Obj=arMask[i].Init(x+dx+(i-SlotLvl2)*w,y+dy+h*2,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,bmpGUI_UnitGlow,Obj,3);
	};
	for (int i=SlotLvl3; i<pHero->GetUnitSlot(3)+SlotLvl3; i++)
	{
		Obj=arMask[i].Init(x+dx+(i-SlotLvl3)*w,y+dy+h,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,bmpGUI_UnitGlow,Obj,3);
	};
	for (int i=SlotLvl4; i<pHero->GetUnitSlot(4)+SlotLvl4; i++)
	{
		Obj=arMask[i].Init(x+dx+(i-SlotLvl4)*w,y+dy,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,bmpGUI_UnitGlow,Obj,3);
	};
	Show();
	return Obj;
};

int GUI_UnitBlock::Check(dcUnit *pUnit)
{
	int Lvl=1;
	for (int i=1; i<MAX_SQUAD; i++)
	{
		if (i>=SlotLvl2)
		{Lvl=2;};
		if (i>=SlotLvl3)
		{Lvl=3;};
		if (i>=SlotLvl4)
		{Lvl=4;};
		
		if (arMask[i].Glow_Obj)
		{
			if ((pUnit)&&(pUnit->pType()->iLevel<=Lvl))
			{arMask[i].bGlow=true;}
			else
			{arMask[i].bGlow=false;};
		};
	};
	for (int i=1; i<MAX_SQUAD; i++)
	{
		int Check=arMask[i].Check();
		if ((Check>1)&&(pHero->pSquad[i]))
		{
			char Txt[100];
			sprintf(Txt,"%s (%s %d)",pHero->pSquad[i]->pType()->sName,(pFirstPhrase+10)->str,pHero->pSquad[i]->pType()->iLevel);
			MakeStringHint(Txt);
			bHint=true;
		};
		if (Check==30)
		{
			return i;
		};
		if (Check==40)
		{
			if (pHero->pSquad[i])
			{
				bHint=false;
				WindowUnitInfo(pHero->pSquad[i],pHero->GetSquadMorale(i));
			};
		//	return i+1000;
		};

	};
	return 0;
};

void GUI_UnitBlock::Show()
{
	set_trans_blender(60,60,60,60);
	rectfill(Pic,0,0,width,height,makecol(255,0,255));
	for (int i=1; i<=pHero->GetUnitSlot(1); i++)
	{
		if (pHero->pSquad[i])
		{
			DrawUnit(Pic,pHero->pSquad[i],(i-1)*w,h*3,pHero->GetSquadMorale(i),bmpExp,bmpHealth,NULL);
		};
	};
	for (int i=pHero->GetUnitSlot(1)+1; i<SlotLvl2; i++)
	{		
		draw_sprite(Pic,bmpGUI_UnitClosed,(i-1)*w,h*3);
		
	};
	
	for (int i=SlotLvl2; i<pHero->GetUnitSlot(2)+SlotLvl2; i++)
	{
		if (pHero->pSquad[i])
		{
			DrawUnit(Pic,pHero->pSquad[i],(i-SlotLvl2)*w,h*2,pHero->GetSquadMorale(i),bmpExp,bmpHealth,NULL);		
		};
	};
	for (int i=pHero->GetUnitSlot(2)+SlotLvl2; i<SlotLvl3; i++)
	{		
		draw_sprite(Pic,bmpGUI_UnitClosed,(i-SlotLvl2)*w,h*2);
		
	};

	for (int i=SlotLvl3; i<pHero->GetUnitSlot(3)+SlotLvl3; i++)
	{
		if (pHero->pSquad[i])
		{
			DrawUnit(Pic,pHero->pSquad[i],(i-SlotLvl3)*w,h,pHero->GetSquadMorale(i),bmpExp,bmpHealth,NULL);
		};
	};
	for (int i=pHero->GetUnitSlot(3)+SlotLvl3; i<SlotLvl4; i++)
	{		
		draw_sprite(Pic,bmpGUI_UnitClosed,(i-SlotLvl3)*w,h);
		
	};

	for (int i=SlotLvl4; i<pHero->GetUnitSlot(4)+SlotLvl4; i++)
	{
		if (pHero->pSquad[i])
		{
			DrawUnit(Pic,pHero->pSquad[i],(i-SlotLvl4)*w,0,pHero->GetSquadMorale(i),bmpExp,bmpHealth,NULL);
		};
	};
	for (int i=pHero->GetUnitSlot(4)+SlotLvl4; i<MAX_SQUAD; i++)
	{		
		draw_sprite(Pic,bmpGUI_UnitClosed,(i-SlotLvl4)*w,0);		
	};
	set_trans_blender(128,128,128,128);
};

//***************************************************
//*** Методы SpellBlock *****************************
//***************************************************

GUI_SpellBlock::GUI_SpellBlock()
{
	pHero=NULL;
	Pic=create_bitmap(width,height);	
};

GUI_SpellBlock::~GUI_SpellBlock()
{
	if (bGameInProcess)
	{
		destroy_bitmap(Pic);
	};
};

G_Object * GUI_SpellBlock::Init(dcHero *Herr, int sx, int sy,  G_Object * Obj)
{	
	x=sx;
	y=sy;
	pHero=Herr;
	Obj=new G_Object(Obj,x,y,2,Pic);
	for (int i=1; i<=pHero->GetSpellSlot(1); i++)
	{
		Obj=arMask[i].Init(x+dx+(i-1)*w,y+dy+h*3,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,bmpGUI_SpellGlow,Obj,3);
	};
	for (int i=SpellLvl2; i<pHero->GetSpellSlot(2)+SpellLvl2; i++)
	{
		Obj=arMask[i].Init(x+dx+(i-SpellLvl2)*w,y+dy+h*2,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,bmpGUI_SpellGlow,Obj,3);
	};
	for (int i=SpellLvl3; i<pHero->GetSpellSlot(3)+SpellLvl3; i++)
	{
		Obj=arMask[i].Init(x+dx+(i-SpellLvl3)*w,y+dy+h,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,bmpGUI_SpellGlow,Obj,3);
	};
	for (int i=SpellLvl4; i<pHero->GetSpellSlot(4)+SpellLvl4; i++)
	{
		Obj=arMask[i].Init(x+dx+(i-SpellLvl4)*w,y+dy,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,bmpGUI_SpellGlow,Obj,3);
	};
	Show();
	return Obj;
};

int GUI_SpellBlock::Check(int Spell)
{
	for (int i=1; i<SPELLBOOK_SIZE+1; i++)
	{
		if (arMask[i].Glow_Obj)
		{
			int Lvl=1;
			if (i>=SpellLvl2)
			{Lvl=2;};
			if (i>=SpellLvl3)
			{Lvl=3;};
			if (i>=SpellLvl4)
			{Lvl=4;};
			if ((Spell>0)&&((pFirstSpell+Spell)->iLevel<=Lvl))
			{arMask[i].bGlow=true;}
			else
			{arMask[i].bGlow=false;};
		};
	};

	for (int i=1; i<SPELLBOOK_SIZE+1; i++)
	{
		int Check=arMask[i].Check();
		if ((Check>1)&&(pHero->arSpell[i]>0))
		{
			MakeStringHint((pFirstSpell+pHero->arSpell[i])->sName);
			bHint=true;
		};
		if (Check==30)
		{
			return i;
		};				
		if (Check==40)
		{
			if (pHero->arSpell[i]>0)
			{
				bHint=false;
				WindowSpellInfo(pHero->arSpell[i]);
			};
	//		return 1000+i;
		};	
	};
	return -1;
};

void GUI_SpellBlock::Show()
{
	rectfill(Pic,0,0,width,height,makecol(255,0,255));
	for (int i=1; i<=pHero->GetSpellSlot(1); i++)
	{
		if (pHero->arSpell[i]>0)
		{	
			draw_sprite(Pic,(pFirstSpell+pHero->arSpell[i])->Pic,(i-1)*w,h*3);
		};
	};
	for (int i=pHero->GetSpellSlot(1)+1; i<=SpellLvl2; i++)
	{		
		draw_sprite(Pic,bmpGUI_SpellClosed,(i-1)*w,h*3);
		
	};
	
	for (int i=SpellLvl2; i<pHero->GetSpellSlot(2)+SpellLvl2; i++)
	{
		if (pHero->arSpell[i]>0)
		{
			draw_sprite(Pic,(pFirstSpell+pHero->arSpell[i])->Pic,(i-SpellLvl2)*w,h*2);
		};
	};
	for (int i=pHero->GetSpellSlot(2)+SpellLvl2; i<SpellLvl3; i++)
	{		
		draw_sprite(Pic,bmpGUI_SpellClosed,(i-SpellLvl2)*w,h*2);
		
	};

	for (int i=SpellLvl3; i<pHero->GetSpellSlot(3)+SpellLvl3; i++)
	{
		if (pHero->arSpell[i]>0)
		{
			draw_sprite(Pic,(pFirstSpell+pHero->arSpell[i])->Pic,(i-SpellLvl3)*w,h);
		};
	};
	for (int i=pHero->GetSpellSlot(3)+SpellLvl3; i<SpellLvl4; i++)
	{		
		draw_sprite(Pic,bmpGUI_SpellClosed,(i-SpellLvl3)*w,h);
		
	};

	for (int i=SpellLvl4; i<pHero->GetSpellSlot(4)+SpellLvl4; i++)
	{
		if (pHero->arSpell[i]>0)
		{
			draw_sprite(Pic,(pFirstSpell+pHero->arSpell[i])->Pic,(i-SpellLvl4)*w,0);			
		};
	};
	for (int i=pHero->GetSpellSlot(4)+SpellLvl4; i<=SPELLBOOK_SIZE; i++)
	{		
		draw_sprite(Pic,bmpGUI_SpellClosed,(i-SpellLvl4)*w,0);		
	};	
};

//***************************************************
//*** Методы StatBlock ******************************
//***************************************************

GUI_StatBlock::GUI_StatBlock()
{
	pHero=NULL;
	Pic=create_bitmap(width,height);
};

GUI_StatBlock::~GUI_StatBlock()
{
	if (bGameInProcess)
	{destroy_bitmap(Pic);};
};

G_Object * GUI_StatBlock::Init(dcHero *Herr, int sx, int sy,  G_Object * Obj)
{	
	x=sx;
	y=sy;
	pHero=Herr;
	Obj=new G_Object(Obj,x,y,2,Pic);
	for (int i=0; i<12; i++)
	{
		arMask[i].Init(10+x+(i%3)*w,y+(i-i%3)/3*h,w-1,h-1,1);
	};
	Show();
	return Obj;
};

void GUI_StatBlock::Show()
{
	rectfill(Pic, 0, 0, width, height, makecol(255,0,255));
	textprintf_centre_ex(Pic,Font1,dx,dy,COLOR_2,-1,"%d",pHero->pSquad[0]->GetLife());
	textprintf_centre_ex(Pic,Font1,dx+w,dy,COLOR_2,-1,"%d",pHero->pSquad[0]->GetStamina());
	textprintf_centre_ex(Pic,Font1,dx+w*2,dy,COLOR_2,-1,"%d",pHero->pSquad[0]->GetMorale());
	textprintf_centre_ex(Pic,Font1,dx,dy+h,COLOR_2,-1,"%d",pHero->pSquad[0]->GetAttack());
	textprintf_centre_ex(Pic,Font1,dx+w,dy+h,COLOR_2,-1,"%d",pHero->pSquad[0]->GetCounterAttack());
	textprintf_centre_ex(Pic,Font1,dx+w*2,dy+h,COLOR_2,-1,"%d",pHero->pSquad[0]->GetSpeed());
	textprintf_centre_ex(Pic,Font1,dx,dy+h*2,COLOR_2,-1,"%d",pHero->pSquad[0]->GetDefence());
	textprintf_centre_ex(Pic,Font1,dx+w,dy+h*2,COLOR_2,-1,"%d",pHero->pSquad[0]->GetRangedDefence());
	textprintf_centre_ex(Pic,Font1,dx+w*2,dy+h*2,COLOR_2,-1,"%d",pHero->pSquad[0]->GetResist());
	textprintf_centre_ex(Pic,Font1,dx,dy+h*3,COLOR_2,-1,"%d",pHero->pSquad[0]->GetRangedAttack());
	textprintf_centre_ex(Pic,Font1,dx+w,dy+h*3,COLOR_2,-1,"%d",pHero->pSquad[0]->GetShootingRange());
	textprintf_centre_ex(Pic,Font1,dx+w*2,dy+h*3,COLOR_2,-1,"%d",pHero->pSquad[0]->GetAmmo());
};

void GUI_StatBlock::Check()
{
	int Check, Param, Power;
	for (int i=0; i<12; i++)
	{
		Check=arMask[i].Check();
		if (Check>1)
		{
			switch (i)
			{
			case 0:
				Param=1;
				Power=pHero->pSquad[0]->GetLife();
				break;
			case 1:
				Param=11;
				Power=pHero->pSquad[0]->GetStamina();
				break;
			case 2:
				Param=12;
				Power=pHero->pSquad[0]->GetMorale();
				break;
			case 3:
				Param=2;
				Power=pHero->pSquad[0]->GetAttack();
				break;
			case 4:
				Param=3;
				Power=pHero->pSquad[0]->GetCounterAttack();
				break;
			case 5:
				Param=7;
				Power=pHero->pSquad[0]->GetSpeed();
				break;
			case 6:
				Param=4;
				Power=pHero->pSquad[0]->GetDefence();
				break;
			case 7:
				Param=5;
				Power=pHero->pSquad[0]->GetRangedDefence();
				break;
			case 8:
				Param=6;
				Power=pHero->pSquad[0]->GetResist();
				break;
			case 9:
				Param=8;
				Power=pHero->pSquad[0]->GetRangedAttack();
				break;
			case 10:
				Param=9;
				Power=pHero->pSquad[0]->GetShootingRange();
				break;
			case 11:
				Param=10;
				Power=pHero->pSquad[0]->GetAmmo();
				break;
		};
		if ((Check==30)||(Check==40))
		{
			WindowAbilityInfo(Param,Power);
		}
		else
		{
			MakeAbilityHint(Param);
			bHint=true;
		};
	};
	};
};

//***************************************************
//*** Методы SkillBlock *****************************
//***************************************************

GUI_SkillBlock::GUI_SkillBlock()
{
	pHero=NULL;
	Pic=create_bitmap(width,height);		
};

GUI_SkillBlock::~GUI_SkillBlock()
{
	if (bGameInProcess)
	{destroy_bitmap(Pic);};
};

G_Object * GUI_SkillBlock::Init(dcHero *Herr, int sx, int sy,  G_Object * Obj)
{	
	x=sx;
	y=sy;
	pHero=Herr;
	Obj=new G_Object(Obj,x,y,2,Pic);
	for (int i=0; i<=pHero->GetSkillSlot(); i++)
	{		
		if ((i==0)||(pHero->apSkill[i-1][0]>0))
		{
			Obj=arMask[i].Init(i%2*w-6+x,(i-i%2)/2*h-6+y,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,3);
		};
	};
	Show();
	return Obj;
};

void GUI_SkillBlock::Show()
{
	rectfill(Pic, 0, 0, width, height, makecol(255,0,255));
	draw_sprite(Pic,ClassPic[pHero->GetClassName()],0,0);
	for (int i=1; i<=pHero->GetSkillSlot(); i++)
	{
	//	if (pHero->apSkill[i-1][0]>1000)
	//	{pHero->apSkill[i-1][0]=0;};
		if (pHero->apSkill[i-1][0]>0)
		{
			draw_sprite(Pic,bmpHeroSkill[(pHero->apSkill[i-1][0]-1)*5+pHero->apSkill[i-1][1]],i%2*w,(i-i%2)/2*h);
		};
	};
	for (int i=pHero->GetSkillSlot()+1; i<14; i++)
	{
		draw_sprite(Pic,bmpGUI_SpellClosed,i%2*w,(i-i%2)/2*h);
	};	
};

void GUI_SkillBlock::Check()
{	
	for (int i=0; i<=pHero->GetSkillSlot(); i++)
	{
		int Check=arMask[i].Check();
		if (Check>1)
		{
			char Txt[100];
			if (i==0)
			{sprintf(Txt,ClassName[pHero->GetClassName()].str);}
			else
			{sprintf(Txt,"%s %d",arSkills[pHero->apSkill[i-1][0]].sName,pHero->apSkill[i-1][1]);};

			MakeStringHint(Txt);
			bHint=true;
		};
		if ((Check==30)||(Check==40))
		{
			bHint=false;
			if (i==0)
			{
				char Txt[500];
				if (pHero->ipClass1==2)
				{sprintf(Txt,ClassDesc[pHero->GetClassName()].msg,(pFirstAbilityNum+(pHero->pSquad[0]->arUpg[0]-30))->sName);}
				else
				{sprintf(Txt,ClassDesc[pHero->GetClassName()].msg);};
				WindowInfo(ClassName[pHero->GetClassName()].str,Txt,ClassPic[pHero->GetClassName()]);
			}
			else
			{WindowSkillInfo(pHero->apSkill[i-1][0],pHero->apSkill[i-1][1]);};
		};
	};	
};

//***************************************************
//*** Методы QuestBlock *****************************
//***************************************************

GUI_QuestBlock::GUI_QuestBlock()
{
	pHero=NULL;
	Pic=create_bitmap(width,height);
	Cursor=create_bitmap(338,36);
	rectfill(Cursor,0,0,338,36,makecol(200,200,200));
};

GUI_QuestBlock::~GUI_QuestBlock()
{
	if (bGameInProcess)
	{
		destroy_bitmap(Pic);
		destroy_bitmap(Cursor);
	};
};

G_Object * GUI_QuestBlock::Init(dcHero *Herr, int sx, int sy,  G_Object * Obj)
{	
	x=sx;
	y=sy;
	pHero=Herr;
	Obj=new G_Object(Obj,x,y,2,Pic);
	for (int i=0; i<HERO_QUEST_LIMIT; i++)
	{
		if (pHero->arQuest[i].Index>0)
		{
			Obj=arMask[i].Init(x+1,y+41+i*h,1,Cursor,Cursor,Obj,3);
			arMask[i].G_Obj->bTrans=true;
		};
	};
	Show();
	return Obj;
};

void GUI_QuestBlock::Show()
{
	rectfill(Pic, 0, 0, width, height, makecol(255,0,255));
	textprintf_centre_ex(Pic,Font1,width/2,12,COLOR_1,-1,"%s:",(pFirstPhrase+65)->str);
	int Count=0;
	for (int i=0; i<HERO_QUEST_LIMIT; i++)
	{
		if (pHero->arQuest[i].Index>0)
		{
			pHero->CheckQuestDone(i);
			char Mess[1000];
			int Progress=pHero->arQuest[i].arProgress[0];			
			if ((pFirstQuest+pHero->arQuest[i].Index)->iType==8)
			{
				sprintf(Mess,"%s: %s",(pFirstQuest+pHero->arQuest[i].Index)->sName,(pFirstUnitType+(pFirstProvince+pHero->arQuest[i].ipProvince)->arSite[pHero->arQuest[i].ipSite][2])->sName);
				if (Progress>(pFirstProvince+pHero->arQuest[i].ipProvince)->arSite[pHero->arQuest[i].ipSite][5])
				{Progress=(pFirstProvince+pHero->arQuest[i].ipProvince)->arSite[pHero->arQuest[i].ipSite][5];};
				if (pHero->arQuest[i].bComplete)
				{
					textprintf_centre_ex(Pic,Font1,width/2,45+Count*h,makecol(0,120,0),-1,"%s",Mess);
					textprintf_centre_ex(Pic,Font1,width/2,63+Count*h,makecol(0,120,0),-1,"%s: %d/%d",(pFirstPhrase+73)->str,Progress,(pFirstProvince+pHero->arQuest[i].ipProvince)->arSite[pHero->arQuest[i].ipSite][5]);
				}
				else
				{
					textprintf_centre_ex(Pic,Font1,width/2,45+Count*h,COLOR_1,-1,"%s",Mess);					
					textprintf_centre_ex(Pic,Font1,width/2,63+Count*h,COLOR_1,-1,"%s: %d/%d",(pFirstPhrase+73)->str,Progress,(pFirstProvince+pHero->arQuest[i].ipProvince)->arSite[pHero->arQuest[i].ipSite][5]);					
				};
			}
			else
			{
			if ((pFirstQuest+pHero->arQuest[i].Index)->iType==7)
			{sprintf(Mess,(pFirstQuest+pHero->arQuest[i].Index)->sName,(pFirstItem+(pFirstProvince+pHero->arQuest[i].ipProvince)->arSite[pHero->arQuest[i].ipSite][5])->sName);}
			else
			{sprintf(Mess,(pFirstQuest+pHero->arQuest[i].Index)->sName);};
			if (Progress>(pFirstQuest+pHero->arQuest[i].Index)->Quantity[0])
			{Progress=(pFirstQuest+pHero->arQuest[i].Index)->Quantity[0];};
			if (pHero->arQuest[i].bComplete)
			{
				textprintf_centre_ex(Pic,Font1,width/2,45+Count*h,makecol(0,120,0),-1,"%s",Mess);
				if ((pFirstQuest+pHero->arQuest[i].Index)->Quantity[0]>1)
				{textprintf_centre_ex(Pic,Font1,width/2,63+Count*h,makecol(0,120,0),-1,"%s: %d/%d",(pFirstPhrase+73)->str,pHero->arQuest[i].arProgress[0],(pFirstQuest+pHero->arQuest[i].Index)->Quantity[0]);}
				else
				{textprintf_centre_ex(Pic,Font1,width/2,63+Count*h,makecol(0,120,0),-1,"%s",(pFirstPhrase+73)->str);};
			}
			else
			{
				textprintf_centre_ex(Pic,Font1,width/2,45+Count*h,COLOR_1,-1,"%s",Mess);
				if ((pFirstQuest+pHero->arQuest[i].Index)->Quantity[0]>1)
				{textprintf_centre_ex(Pic,Font1,width/2,63+Count*h,COLOR_1,-1,"%s: %d/%d",(pFirstPhrase+73)->str,pHero->arQuest[i].arProgress[0],(pFirstQuest+pHero->arQuest[i].Index)->Quantity[0]);}
				else
				{textprintf_centre_ex(Pic,Font1,width/2,63+Count*h,COLOR_1,-1,"%s",(pFirstPhrase+74)->str);};
			};
			};
		};
		Count++;
	};	
};

void GUI_QuestBlock::Check()
{
	for (int i=0; i<HERO_QUEST_LIMIT; i++)
	{
		if (pHero->arQuest[i].Index>0)
		{
			if (arMask[i].Check()==30)
			{
				char Mess[500];
				if ((pFirstQuest+pHero->arQuest[i].Index)->iType==7)
				{
					sprintf(Mess,(pFirstQuest+pHero->arQuest[i].Index)->sText,(pFirstProvince+pHero->arQuest[i].ipProvince)->sName,(pFirstItem+(pFirstProvince+pHero->arQuest[i].ipProvince)->arSite[pHero->arQuest[i].ipSite][5])->sName);
				}
				else
				{
					if ((pFirstQuest+pHero->arQuest[i].Index)->iType==8)
					{sprintf(Mess,(pFirstQuest+pHero->arQuest[i].Index)->sText,(pFirstUnitType+(pFirstProvince+pHero->arQuest[i].ipProvince)->arSite[pHero->arQuest[i].ipSite][2])->sName,(pFirstProvince+pHero->arQuest[i].ipProvince)->arSite[pHero->arQuest[i].ipSite][5]);}
					else
					{sprintf(Mess,(pFirstQuest+pHero->arQuest[i].Index)->sText,(pFirstProvince+pHero->arQuest[i].ipProvince)->sName);};
				};
				if ((pFirstDialog+494)->Show(1,1,0,0,NULL,Mess)==1)
				{
					if ((pFirstDialog+495)->Show(1,1,0,0,NULL,NULL)==1)
					{
						pHero->AbandonQuest(i);
						Show();
						arMask[i].Check();
					};
				};
			};
		};
	};
};


//***************************************************
//*** Методы InfoBlock ******************************
//***************************************************

GUI_InfoBlock::GUI_InfoBlock()
{
	pHero=NULL;
	Pic=create_bitmap(width,height);
	bmpExp=create_bitmap(bmpGUI_ExpLow->w,bmpGUI_ExpLow->h);
	bmpHealth=create_bitmap(bmpGUI_HealthLow->w,bmpGUI_HealthLow->h);
};

GUI_InfoBlock::~GUI_InfoBlock()
{
	if (bGameInProcess)
	{
		destroy_bitmap(Pic);
		destroy_bitmap(bmpExp);
		destroy_bitmap(bmpHealth);
	};
};

G_Object * GUI_InfoBlock::Init(dcHero *Herr, int sx, int sy,  G_Object * Obj)
{	
	x=sx;
	y=sy;
	pHero=Herr;
	fHealth=Herr->pSquad[0]->iLife;
	fHealth/=Herr->pSquad[0]->GetLife();
	iOldLife=Herr->pSquad[0]->iLife;
	iOldMaxLife=Herr->pSquad[0]->GetLife();
	Obj=new G_Object(Obj,x,y,2,Pic);
	mskHero.Init(38+x,10+y,pHero->Pic->w,pHero->Pic->h,1);
	for (int i=0; i<6; i++)
	{
		arMask[i].Init(20+x+(i%3)*w,211+y+(i-i%3)/3*30,w-1,30-1,1);
	};
	mskLevel.Init(x+23,y+12,18,150,1);
	mskLife.Init(x+175,y+12,18,150,1);
	
	Show();
	return Obj;
};

void GUI_InfoBlock::Show()
{
	if (pHero->iDead<0)
	{
		if (pHero->pSquad[0]->GetLife()==iOldMaxLife)
		{pHero->pSquad[0]->iLife=iOldLife;}
		else
		{
			pHero->pSquad[0]->iLife=pHero->pSquad[0]->GetLife()*fHealth;
			if (pHero->pSquad[0]->iLife<=0)
			{pHero->pSquad[0]->iLife=1;};
		};
	};
	
	rectfill(Pic, 0, 0, width, height, makecol(255,0,255));

	/*draw_sprite(Pic, pHero->Pic,38,10);
	textprintf_centre_ex(Pic,Font1,100,175,makecol(220,220,0),-1,"%s",(pName[pHero->ipClass1-1]+pHero->ipName)->str);
	textprintf_centre_ex(Pic,Font1,100,205,makecol(220,220,0),-1,"%s",ClassName[pHero->GetClassName()].str);
	textprintf_centre_ex(Pic,Font1,50,231,makecol(220,220,0),-1,"%d",pHero->iLeader);
	textprintf_centre_ex(Pic,Font1,50+w,231,makecol(220,220,0),-1,"%d",pHero->iMagic);
	textprintf_centre_ex(Pic,Font1,50+w*2,231,makecol(220,220,0),-1,"%d",pHero->iHealth);
	textprintf_centre_ex(Pic,Font1,50,260,makecol(220,220,0),-1,"%d",pHero->CheckHeroBonus(999));
	textprintf_centre_ex(Pic,Font1,50+w,260,makecol(220,220,0),-1,"%d",pHero->GetSiege());
	textprintf_centre_ex(Pic,Font1,50+w*2,260,makecol(220,220,0),-1,"%d",pHero->GetMobility());
	for (int i=0; i<fHealth*100; i++)
	{rectfill(Pic,179,112-i,188,111-i,makecol(100+i,i/2,i/2));};
	int iExp=100*(pHero->iExp-pHero->ExpForLevel(pHero->iLevel))/(pHero->ExpForLevel(pHero->iLevel+1)-pHero->ExpForLevel(pHero->iLevel));
	if (iExp>100)
	{iExp=100;};
	for (int i=0; i<iExp; i++)
	{rectfill(Pic,16,112-i,25,111-i,makecol(100+i,i/2,120+i));};
	textprintf_centre_ex(Pic,Font1,20,145,makecol(220,220,0),-1,"%d",pHero->iLevel);
	textprintf_centre_ex(Pic,Font1,183,145,makecol(220,220,0),-1,"%d",pHero->pSquad[0]->iLife);*/

	draw_sprite(Pic, pHero->Pic,43,12);
	draw_sprite(Pic, bmpGUI_HeroPort,42,116);
	textprintf_centre_ex(Pic,Font1,109,169,COLOR_1,-1,"%s",pHero->pstName->str);
	textprintf_centre_ex(Pic,Font1,109,193,COLOR_1,-1,"%s",ClassName[pHero->GetClassName()].str);
	textprintf_centre_ex(Pic,Font1,63,222,COLOR_2,-1,"%d",pHero->iLeader);
	textprintf_centre_ex(Pic,Font1,63+w,222,COLOR_2,-1,"%d",pHero->iMagic);
	textprintf_centre_ex(Pic,Font1,63+w*2,222,COLOR_2,-1,"%d",pHero->iHealth);
	textprintf_centre_ex(Pic,Font1,63,252,COLOR_2,-1,"%d",pHero->CheckHeroBonus(999));
	textprintf_centre_ex(Pic,Font1,63+w,252,COLOR_2,-1,"%d",pHero->GetSiege());
	textprintf_centre_ex(Pic,Font1,63+w*2,252,COLOR_2,-1,"%d",pHero->GetMobility());
	for (int i=0; i<fHealth*100; i++)
	{rectfill(Pic,179,112-i,188,111-i,makecol(100+i,i/2,i/2));};
	int iExp=100*(pHero->iExp-pHero->ExpForLevel(pHero->iLevel))/(pHero->ExpForLevel(pHero->iLevel+1)-pHero->ExpForLevel(pHero->iLevel));
	if (iExp>100)
	{iExp=100;};
	for (int i=0; i<iExp; i++)
	{rectfill(Pic,28,112-i,37,111-i,makecol(100+i,i/2,120+i));};
	textprintf_centre_ex(Pic,Font1,35,146,COLOR_2,-1,"%d",pHero->iLevel);
	textprintf_centre_ex(Pic,Font1,180,146,COLOR_2,-1,"%d",pHero->pSquad[0]->iLife);
};

void GUI_InfoBlock::Check()
{
	int Check=mskHero.Check();
	if ((Check==30)||(Check==40))
	{
		WindowUnitInfo(pHero->pSquad[0],0);
	};
	int Param, Power;
	for (int i=0; i<6; i++)
	{
		Check=arMask[i].Check();
		if (Check>1)
		{
			switch (i)
			{
			case 0:
				Param=116;
				Power=pHero->iLeader;
				break;
			case 1:
				Param=117;
				Power=pHero->iMagic;
				break;
			case 2:
				Param=118;
				Power=pHero->iHealth;
				break;
			case 3:
				Param=999;
				Power=pHero->CheckHeroBonus(999);
				break;
			case 4:
				Param=55;
				Power=pHero->GetSiege();
				break;
			case 5:
				Param=998;
				Power=pHero->GetMobility();
				break;
		};
		if ((Check==30)||(Check==40))
		{
			WindowAbilityInfo(Param,Power);
		}
		else
		{
			MakeAbilityHint(Param);
			bHint=true;
		};
	};
	};
	Check=mskLevel.Check();
	if (Check>1)
	{
		if ((Check==30)||(Check==40))
		{
			WindowAbilityInfo(119,pHero->iLevel);
		}
		else
		{
			MakeAbilityHint(119);
			bHint=true;
		};
	};
	Check=mskLife.Check();
	if (Check>1)
	{
		if ((Check==30)||(Check==40))
		{
			WindowAbilityInfo(1,pHero->pSquad[0]->iLife);
		}
		else
		{
			MakeAbilityHint(1);
			bHint=true;
		};
	};
};


//***************************************************
//*** Методы ResourceBlock **************************
//***************************************************

GUI_ResourceBlock::GUI_ResourceBlock()
{
	Player=NULL;
	Pic=create_bitmap(width,height);	
};

GUI_ResourceBlock::~GUI_ResourceBlock()
{
	if (bGameInProcess)
	{destroy_bitmap(Pic);};
};

G_Object * GUI_ResourceBlock::Init(dcPlayer *sPlayer, int sx, int sy,  G_Object * Obj)
{	
	x=sx;
	y=sy;
	Player=sPlayer;
	Obj=new G_Object(Obj,x,y,2,Pic);
	for (int i=0; i<9; i++)
	{
		mskRes[i].Init(x+255+w*i,y,21,21,1);		
	};
	mskAlchemy.Init(x+11,y,240,40,1);
	mskAlchemy.iHint=352;
	Show();
	return Obj;
};

void GUI_ResourceBlock::Show()
{
	rectfill(Pic, 0, 0, width, height, makecol(255,0,255));	
	textprintf_centre_ex(Pic,Font1,86,5,makecol(220,220,0),-1,"%d",Player->Gold);
	textprintf_centre_ex(Pic,Font1,210,5,makecol(220,220,0),-1,"%d",Player->Gem);
	set_trans_blender(100,100,100,100);
	for (int i=1; i<=iMaxResource; i++)
	{
		if (Player->GetResource(i)>0)
		{
			draw_sprite(Pic,(pFirstResource+i)->Pic,255+w*(i-1),0);
		}
		else
		{
			rectfill(Pic,255+w*(i-1),0,255+w*(i-1)+20,20,COLOR_1);
			draw_trans_sprite(Pic,(pFirstResource+i)->Pic,255+w*(i-1),0);
		};
	};	
	set_trans_blender(128,128,128,128);
};

bool GUI_ResourceBlock::Check(G_Object * Obj)
{
	bool Result=false;
	int Check;
	if (Player->GetBldParam1(34)>0)
	{
		Check=mskAlchemy.Check();
		if ((Check==30)||(Check==40))
		{
			ShowActiveObjects(Obj,10,false);
			WindowAlchemy(Player->Num);
			Show();
			Result=true;
		};
	};
	for (int i=0; i<9; i++)
	{
		Check=mskRes[i].Check();
		if (Check>1)
		{
			if ((Check==30)||(Check==40))
			{
				ShowActiveObjects(Obj,10,false);
				WindowResourceInfo(i+1);
			}
			else
			{
				char HNT[100];
				sprintf (HNT,"%s (%d)",(pFirstResource+1+i)->sName,Player->GetResource(i+1));
				bHint=true;
				MakeStringHint(HNT);
			};
		};
		
	};
	return Result;
};


//***************************************************
//*** Методы RecruitBlock ***************************
//***************************************************

GUI_RecruitBlock::GUI_RecruitBlock()
{
	Player=NULL;
	Pic=create_bitmap(width,height);	
};

GUI_RecruitBlock::~GUI_RecruitBlock()
{
	if (bGameInProcess)
	{destroy_bitmap(Pic);};
};

G_Object * GUI_RecruitBlock::Init(dcPlayer * sPlayer, int sx, int sy, G_Object * Obj, bool UseLocalTab, int Prov)
{
	x=sx;
	y=sy;
	ipProv=Prov;
	Player=sPlayer;
	for (int i=0; i<MAX_RECRUIT; i++)
	{
		if (UseLocalTab)
		{arRecr[i]=Recruitz[i];}		
		else
		{arRecr[i]=Player->arRecruit[i];};
	};
	bLocalTab=UseLocalTab;
	Obj=new G_Object(Obj,x,y,2,Pic);
	for (int i=0; i<6; i++)
	{
		if (arRecr[i]>0)
		{
			Obj=arMask[i].Init(x+dx+i*w+21,y+dy+h+16,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,Obj,3);
			for (int j=0; j<MAX_UNIT_RES; j++)
			{
				if ((pFirstUnitType+arRecr[i])->arRes[j]>0)
				{
					mskRes[i][j].Init(x+dx+i*w+26+j*rw,y+dy+h+166,21,21,1);
				};
			};
		};
	};
	for (int i=6; i<MAX_RECRUIT; i++)
	{
		if (arRecr[i]>0)
		{
			Obj=arMask[i].Init(x+dx+(i-6)*w+21,y+dy+16,1,bmpGUI_UnitMask,bmpGUI_UnitSelect,Obj,3);
			for (int j=0; j<MAX_UNIT_RES; j++)
			{
				if ((pFirstUnitType+arRecr[i])->arRes[j]>0)
				{
					mskRes[i][j].Init(x+dx+(i-6)*w+26+j*rw,y+dy+166,21,21,1);
				};
			};
		};
	};	
	Show();
	return Obj;
};

void GUI_RecruitBlock::Recalc()
{
	for (int i=0; i<MAX_RECRUIT; i++)
	{
		if (bLocalTab)
		{arRecr[i]=Recruitz[i];}		
		else
		{arRecr[i]=Player->arRecruit[i];};
	};
	Show();
};

int GUI_RecruitBlock::Check()
{
	for (int i=0; i<MAX_RECRUIT; i++)
	{
		int Check=0;
		if (arRecr[i]>0)
		{
			Check=arMask[i].Check();
			if (Check>1)
			{
				char Txt[100];
				if (i==11)
				{sprintf(Txt,"%s (%s, %s %d)",(pFirstUnitType+arRecr[i])->sName,(pFirstPhrase+333)->str,(pFirstPhrase+10)->str,(pFirstUnitType+arRecr[i])->iLevel);}
				else
				{sprintf(Txt,"%s (%s %d)",(pFirstUnitType+arRecr[i])->sName,(pFirstPhrase+10)->str,(pFirstUnitType+arRecr[i])->iLevel);};
				MakeStringHint(Txt);
				bHint=true;
			};
			if (Check==30)
			{
				if ((arCost[0][i]<=Player->Gold)&&((arCost[1][i]<=Player->Gem)||(arCost[1][i]==0)))
				{
					if (i==11)
					{
						return arRecr[i]+1000;
					}
					else
					{
						return arRecr[i];
					}
				};
			};
			if (Check==40)
			{
				bHint=false;
				WindowUnitTypeInfo(arRecr[i]);
			};
			for (int j=0; j<MAX_UNIT_RES; j++)
			{
				if ((pFirstUnitType+arRecr[i])->arRes[j]>0)
				{
					Check=mskRes[i][j].Check();
					if (Check>1)
					{
						if ((Check==30)||(Check==40))
						{
							WindowResourceInfo((pFirstUnitType+arRecr[i])->arRes[j]);
						}
						else
						{
							bHint=true;
							MakeStringHint((pFirstResource+(pFirstUnitType+arRecr[i])->arRes[j])->sName);
						};
					};
				};
			};
		};

	};
	return 0;
};

void GUI_RecruitBlock::Show()
{
	int Gold, Gem, Res;	
	rectfill(Pic,0,0,width,height,makecol(255,0,255));
	for (int i=0; i<6; i++)
	{
		if (arRecr[i]>0)
		{	
 			draw_sprite(Pic,(pFirstUnitType+arRecr[i])->Pic,i*w+21,h+16);			
			Gold=Player->GetUnitGoldPrice(arRecr[i], ipProv);
			Gem=Player->GetUnitGemPrice(arRecr[i], ipProv);
			
			for (int j=0; j<MAX_UNIT_RES; j++)
			{
				Res=(pFirstUnitType+arRecr[i])->arRes[j];
				if (Res>0)
				{
					draw_sprite(Pic,(pFirstResource+Res)->Pic,i*w+20+j*rw,h+160);
					if (Player->GetResource(Res)==0)
					{
						draw_sprite(Pic,bmpGUI_NoRes,i*w+20+j*rw,h+160);
//						Gold+=(pFirstResource+Res)->iGold*CurGame.ResPrices[Res]/100;
//						Gem+=(pFirstResource+Res)->iGem*CurGame.ResPrices[Res]/100;
					};
				};
			};
			arCost[0][i]=Gold;
			arCost[1][i]=Gem;
			if (Player->Gold>=Gold)
				{textprintf_centre_ex(Pic,Font1,i*w+60,h+109,makecol(220,220,0),-1,"%d",Gold);}
			else
				{textprintf_centre_ex(Pic,Font1,i*w+60,h+109,makecol(220,20,0),-1,"%d",Gold);};
			if ((Player->Gem>=Gem)||(Gem==0))
				{textprintf_centre_ex(Pic,Font1,i*w+60,h+138,makecol(220,220,0),-1,"%d",Gem);}
			else
				{textprintf_centre_ex(Pic,Font1,i*w+60,h+138,makecol(220,20,0),-1,"%d",Gem);};
		}
		else
		{
			draw_sprite(Pic,bmpGUI_RecruitClosed,17+i*w,12+h);
		};
	};
	for (int i=0; i<6; i++)
	{
		if (arRecr[i+6]>0)
		{	
 			draw_sprite(Pic,(pFirstUnitType+arRecr[i+6])->Pic,i*w+21,16);			
			Gold=Player->GetUnitGoldPrice(arRecr[i+6], ipProv);
			Gem=Player->GetUnitGemPrice(arRecr[i+6], ipProv);
			if (i==5)
			{
				Gold*=2;
				Gem*=2;
			};			
			for (int j=0; j<MAX_UNIT_RES; j++)
			{
				Res=(pFirstUnitType+arRecr[i+6])->arRes[j];
				if (Res>0)
				{
					draw_sprite(Pic,(pFirstResource+Res)->Pic,i*w+20+j*rw,160);
					if (Player->GetResource(Res)==0)
					{
						draw_sprite(Pic,bmpGUI_NoRes,i*w+20+j*rw,160);
	//					Gold+=(pFirstResource+Res)->iGold*CurGame.ResPrices[Res]/100;
	//					Gem+=(pFirstResource+Res)->iGem*CurGame.ResPrices[Res]/100;
					};
				};
			};
			arCost[0][i+6]=Gold;
			arCost[1][i+6]=Gem;
			if (Player->Gold>=Gold)
				{textprintf_centre_ex(Pic,Font1,i*w+60,109,makecol(220,220,0),-1,"%d",Gold);}
			else
				{textprintf_centre_ex(Pic,Font1,i*w+60,109,makecol(220,20,0),-1,"%d",Gold);};
			if (Player->Gem>=Gem)
				{textprintf_centre_ex(Pic,Font1,i*w+60,138,makecol(220,220,0),-1,"%d",Gem);}
			else
				{textprintf_centre_ex(Pic,Font1,i*w+60,138,makecol(220,20,0),-1,"%d",Gem);};
		}
		else
		{
			draw_sprite(Pic,bmpGUI_RecruitClosed,17+i*w,12);
		};
	};
};

//***************************************************
//*** Методы LibraryBlock ***************************
//***************************************************

GUI_LibraryBlock::GUI_LibraryBlock()
{
	Player=NULL;
	Pic=create_bitmap(width,height);	
};

GUI_LibraryBlock::~GUI_LibraryBlock()
{
	if (bGameInProcess)
	{destroy_bitmap(Pic);};
};

G_Object * GUI_LibraryBlock::Init(dcPlayer * sPlayer, int sx, int sy, G_Object * Obj)
{
	x=sx;
	y=sy;
	Player=sPlayer;
	Obj=new G_Object(Obj,x,y,2,Pic);
	rectfill(Pic,0,0,width,height,makecol(255,0,255));
	for (int i=0; i<3; i++)
	{
		if (Player->arLibrary[i]>0)
		{
			Obj=arMask[i].Init(x+dx+i*w-6,y+dy+L0-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,3);
			draw_sprite(Pic,(pFirstSpell+Player->arLibrary[i])->Pic,dx+i*w,L0+dy);
		};
	};
	for (int i=0; i<6; i++)
	{
		if (Player->arLibrary[i+3]>0)
		{
			Obj=arMask[i+3].Init(x+i*w-6,y+dy+L1-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,3);
			draw_sprite(Pic,(pFirstSpell+Player->arLibrary[i+3])->Pic,i*w,L1+dy);
		};
	};
	for (int i=0; i<6; i++)
	{
		if (Player->arLibrary[i+9]>0)
		{
			Obj=arMask[i+9].Init(x+i*w-6,y+dy+L2-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,3);
			draw_sprite(Pic,(pFirstSpell+Player->arLibrary[i+9])->Pic,i*w,L2+dy);
		};
	};
	for (int i=0; i<6; i++)
	{
		if (Player->arLibrary[i+15]>0)
		{
			Obj=arMask[i+15].Init(x+i*w-6,y+dy+L3-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,3);
			draw_sprite(Pic,(pFirstSpell+Player->arLibrary[i+15])->Pic,i*w,L3+dy);
		};
	};
	for (int i=0; i<3; i++)
	{
		if (Player->arLibrary[i+21]>0)
		{
			Obj=arMask[i+21].Init(x+i*w+dx-6,y+dy+L4-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,3);
			draw_sprite(Pic,(pFirstSpell+Player->arLibrary[i+21])->Pic,i*w+dx,L4+dy);
		};
	};
	for (int i=0; i<6; i++)
	{
		if (Player->arLibrary[i+24]>0)
		{
			Obj=arMask[i+24].Init(x+i*w-6,y+dy+L5-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,3);
			draw_sprite(Pic,(pFirstSpell+Player->arLibrary[i+24])->Pic,i*w,L5+dy);
		};
	};
	for (int i=0; i<3; i++)
	{
		if (Player->arLibrary[i+30]>0)
		{
			Obj=arMask[i+30].Init(x+i*w+dx-6,y+dy-6,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,3);
			draw_sprite(Pic,(pFirstSpell+Player->arLibrary[i+30])->Pic,i*w+dx,dy);
		};
	};
	textprintf_centre_ex(Pic,Font1,177,370,COLOR_1,-1,(pFirstPhrase+314)->str);
	textprintf_centre_ex(Pic,Font1,177,210,COLOR_1,-1,(pFirstPhrase+315)->str);
	textprintf_centre_ex(Pic,Font1,177,109,COLOR_1,-1,(pFirstPhrase+316)->str);
	textprintf_centre_ex(Pic,Font1,177,8,COLOR_1,-1,(pFirstPhrase+317)->str);

	return Obj;
};

int GUI_LibraryBlock::Check()
{
	for (int i=0; i<LIBRARY_SIZE; i++)
	{
		int Check=arMask[i].Check();
		if (Check>1)
		{
			MakeStringHint((pFirstSpell+Player->arLibrary[i])->sName);
			bHint=true;
		};
		if (Check==30)
		{
			return Player->arLibrary[i];
			
		};
		if (Check==40)
		{	
			bHint=false;
			WindowSpellInfo(Player->arLibrary[i]);			
	//		return i+1000;
		};

	};
	return 0;
};

//***************************************************
//*** Методы ShopBlock ******************************
//***************************************************

GUI_ShopBlock::GUI_ShopBlock()
{
	Hero=NULL;
	PickFlag=0;
	Pic=create_bitmap(width,height);
	buttRepairAll.iHint=143;
	buttRepairAll.ipHotkey=KEY_T;
	buttRepair.iHint=144;
	buttRepair.iHint1=145;
	buttRepair.ipHotkey=KEY_R;
};

GUI_ShopBlock::~GUI_ShopBlock()
{
	if (bGameInProcess)
	{destroy_bitmap(Pic);};
};

G_Object * GUI_ShopBlock::Init(dcHero * sHero, dcShop * sShop, int sx, int sy, G_Object * Obj)
{
	x=sx;
	y=sy;
	Shop=sShop;
	Hero=sHero;
	Player=&arPlayer[Hero->ipPlayer];
	Obj=new G_Object(Obj,x,y,2,Pic);
	Obj=buttRepair.Init(x+13,y+676,1,3,bmpButton[56],bmpButton[57],bmpButton[58],bmpButton[59],bmpButton[58],Obj);
	if (Hero->ipClass1>0)
	{Obj=buttRepairAll.Init(x+78,y+676,1,3,bmpButton[60],bmpButton[61],bmpButton[62],bmpButton[63],Obj);};
	for (int i=0; i<SHOP_SIZE; i++)
	{
		Obj=arMask[i].Init(x+dx+(i%9)*w,y+dy+(i-i%9)/9*h,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,3);		
	};
	mskSell.Init(515,9,500,666,1);
	Show();
	return Obj;
};

int GUI_ShopBlock::Check(dcHero *Hero)
{
	buttRepair.Check();
	if (Hero->ipClass1>0)
	{
		if (buttRepairAll.Check())
		{
			return 10000;
		};
	};
	bool bSell=true;
	for (int i=0; i<SHOP_SIZE; i++)
	{
		int Check=arMask[i].Check();
		if (Check>1)
		{
			Under=i;
			bSell=false;
			if (Shop->Items[i][0]>0)
			{
				MakeItemHint(Shop->Items[i][0],(pFirstItem+Shop->Items[i][0])->iDurability,Hero,13,Player->GetItemGoldPrice(Shop->Items[i][0],Shop),Player->GetItemGemPrice(Shop->Items[i][0],Shop));
				bHint=true;
			};
		};
		if (Check==30)
		{
			return i;
	//		if ((Shop->Items[i]>0)&&
	//			(Player->GetItemGoldPrice(Shop->Items[i][0],Shop)<=Player->Gold)&&
	//			(Player->GetItemGemPrice(Shop->Items[i][0],Shop)<=Player->Gem))
	//		{
	//			return Shop->Items[i][0];
	//		}
	//		else
	//		{
	//			return 0;
	//		};
		};
		if (Check==40)
		{
			if (Shop->Items[i][0]>0)
			{
				bHint=false;
				WindowItemInfo(Shop->Items[i][0],(pFirstItem+Shop->Items[i][0])->iDurability);
			};	
		};
	};
	if ((bSell)&&(mskSell.Check()==30))
	{return 1000;};
	
	return -1;
};

void GUI_ShopBlock::Show()
{
	rectfill(Pic,0,0,width,height,makecol(255,0,255));
	for (int i=0; i<SHOP_SIZE; i++)
	{
		if (Shop->Items[i][0]>0)
		{
//			StopLog("Шмотка: ", Shop->Items[i][0]);
			draw_sprite(Pic,(pFirstItem+Shop->Items[i][0])->Pic,(i%9)*w,(i-i%9)/9*h);
			if (Shop->Items[i][1]>0)
			{
				textprintf_ex(Pic,Font1,(i%9)*w+38,(i-i%9)/9*h+38,COLOR_1,-1,"%d",Shop->Items[i][1]);
				textprintf_ex(Pic,Font1,(i%9)*w+37,(i-i%9)/9*h+37,COLOR_2,-1,"%d",Shop->Items[i][1]);
			};
			if ((Player->GetItemGoldPrice(Shop->Items[i][0],Shop)>Player->Gold)||
				((Player->GetItemGemPrice(Shop->Items[i][0],Shop)>Player->Gem)&&(Player->GetItemGemPrice(Shop->Items[i][0],Shop)>0)))
			{
				draw_sprite(Pic,bmpGUI_SpellShadow,(i%9)*w,(i-i%9)/9*h);
			};			
		};
	};
	if (Hero->ipClass1>0)
	{
	int RepGold=0, RepGem=0;
	for (int i=1; i<=INVENTORY_SIZE; i++)
	{
		//		rectfill(screen, 0,0,300,20,COLOR_1);
		//		textprintf_ex(screen,Font1,0,0,makecol(250,250,250),-1,"i=%d, itm %d",i, Hero->arInventory[i][0]);
		if ((Hero->arInventory[i][0]>0)&&
			(Player->GetItemGoldRepPrice(Hero->arInventory[i][0],Hero->arInventory[i][1],Shop)))
		{
			RepGold+=Player->GetItemGoldRepPrice(Hero->arInventory[i][0],Hero->arInventory[i][1],Shop);
			RepGem+=Player->GetItemGemRepPrice(Hero->arInventory[i][0],Hero->arInventory[i][1],Shop);
		};
	};
	buttRepairAll.Activate(1);
	if (RepGold==0)
	{
		buttRepairAll.Activate(0);
	};
	if (RepGold>Player->Gold)
	{
		textprintf_centre_ex(Pic,Font1,180,686,makecol(220,40,20),-1,"%d",RepGold);
		buttRepairAll.Activate(0);
	}
	else
	{
		textprintf_centre_ex(Pic,Font1,180,686,COLOR_2,-1,"%d",RepGold);
	};
	if ((RepGem>Player->Gem)&&(RepGem>0))
	{
		textprintf_centre_ex(Pic,Font1,180,715,makecol(220,40,20),-1,"%d",RepGem);
		buttRepairAll.Activate(0);
	}
	else
	{
		textprintf_centre_ex(Pic,Font1,180,715,COLOR_2,-1,"%d",RepGem);
	};
	};
	int Price=0;
	switch (PickFlag)
	{
	case 1:
		textprintf_centre_ex(Pic,Font1,313,673,COLOR_1,-1,"%s",(pFirstPhrase+12)->str);
		Price=(pFirstItem+PickItem)->iGoldCost/4;
		if ((pFirstItem+PickItem)->iDurability>0)
		{
			Price-=((pFirstItem+PickItem)->iGoldCost*((pFirstItem+PickItem)->iDurability-PickDur)/(pFirstItem+PickItem)->iDurability/8);
		};
		textprintf_centre_ex(Pic,Font1,280,700,COLOR_2,-1,"%d",Price);
		Price=(pFirstItem+PickItem)->iGemCost/4;
		if ((pFirstItem+PickItem)->iDurability>0)
		{
			Price-=((pFirstItem+PickItem)->iGemCost*((pFirstItem+PickItem)->iDurability-PickDur)/(pFirstItem+PickItem)->iDurability/8);
		};
		textprintf_centre_ex(Pic,Font1,280,724,COLOR_2,-1,"%d",Price);
		break;
	case 2:
		textprintf_centre_ex(Pic,Font1,313,673,COLOR_1,-1,"%s",(pFirstPhrase+13)->str);
		Price=Player->GetItemGoldPrice(PickItem, Shop);
		if (Price>Player->Gold)
			{textprintf_centre_ex(Pic,Font1,280,700,makecol(220,50,20),-1,"%d",Price);}
		else
			{textprintf_centre_ex(Pic,Font1,280,700,COLOR_2,-1,"%d",Price);};
		Price=Player->GetItemGemPrice(PickItem, Shop);
		if (Price>Player->Gem)
			{textprintf_centre_ex(Pic,Font1,280,724,makecol(220,50,20),-1,"%d",Price);}
		else
			{textprintf_centre_ex(Pic,Font1,280,724,COLOR_2,-1,"%d",Price);};		
		for (int i=0; i<MAX_ITEM_RES; i++)
		{
			if ((pFirstItem+PickItem)->arRes[i]>0)
			{
				draw_sprite(Pic,(pFirstResource+(pFirstItem+PickItem)->arRes[i])->Pic, 323+rw*(i%3), 694+rh*((i-i%3)/3));
				if (Player->GetResource((pFirstItem+PickItem)->arRes[i])==0)
				{
					draw_sprite(Pic,bmpGUI_NoRes, 323+rw*(i%3), 694+rh*((i-i%3)/3));
				};
			};
		};
		break;
	case 3:
		Price=Player->GetItemGoldRepPrice(PickItem, PickDur, Shop)+Player->GetItemGemRepPrice(PickItem, PickDur, Shop);
		if (Price>0)
		{
			textprintf_centre_ex(Pic,Font1,313,673,COLOR_1,-1,"%s",(pFirstPhrase+14)->str);
			Price=Player->GetItemGoldRepPrice(PickItem, PickDur, Shop);
			if (Price>Player->Gold)
				{textprintf_centre_ex(Pic,Font1,280,700,makecol(220,50,20),-1,"%d",Price);}
			else
				{textprintf_centre_ex(Pic,Font1,280,700,COLOR_2,-1,"%d",Price);};
			Price=Player->GetItemGemRepPrice(PickItem, PickDur, Shop);
			if (Price>Player->Gem)
				{textprintf_centre_ex(Pic,Font1,280,724,makecol(220,50,20),-1,"%d",Price);}
			else
				{textprintf_centre_ex(Pic,Font1,280,724,COLOR_2,-1,"%d",Price);};		
			for (int i=0; i<MAX_ITEM_RES; i++)
			{
				if ((pFirstItem+PickItem)->arRes[i]>0)
				{
					draw_sprite(Pic,(pFirstResource+(pFirstItem+PickItem)->arRes[i])->Pic, 323+rw*(i%3), 694+rh*((i-i%3)/3));
					if (Player->GetResource((pFirstItem+PickItem)->arRes[i])==0)
					{
						draw_sprite(Pic,bmpGUI_NoRes, 323+rw*(i%3), 694+rh*((i-i%3)/3));
					};
				};
			};
		}
		else
		{
			textprintf_centre_ex(Pic,Font1,313,673,COLOR_1,-1,"%s",(pFirstPhrase+15)->str);
		};
		break;


	};	
};

void GUI_ShopBlock::Pick(int Flag, int Item, int Dur)
{
	PickFlag=Flag;
	PickItem=Item;
	PickDur=Dur;
};


//***************************************************
//*** Методы TreasuryBlock **************************
//***************************************************

GUI_TreasuryBlock::GUI_TreasuryBlock()
{
	Player=NULL;
	Pic=create_bitmap(width,height);	
};

GUI_TreasuryBlock::~GUI_TreasuryBlock()
{
	if (bGameInProcess)
	{destroy_bitmap(Pic);};
};

G_Object * GUI_TreasuryBlock::Init(dcPlayer * sPlayer, int sx, int sy, G_Object * Obj)
{
	x=sx;
	y=sy;	
	Player=sPlayer;
	Obj=new G_Object(Obj,x,y,2,Pic);	
	for (int i=0; i<TREASURY_SIZE; i++)
	{
		Obj=arMask[i].Init(x+dx+(i%9)*w,y+dy+(i-i%9)/9*h,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,Obj,3);		
	};	
	Show();
	return Obj;
};

int GUI_TreasuryBlock::Check(dcHero *Hero, int Mode, dcShop *Shop)
{
	for (int i=0; i<TREASURY_SIZE; i++)
	{
		
		int Check=arMask[i].Check();
		if (Check>1)
		{
			Under=i;
			if (Player->arTreasury[i][0]>0)
			{
				int Gold=0, Gem=0;
				if ((Mode==14)&&(Shop))
				{
					Gold=Player->GetItemGoldRepPrice(Player->arTreasury[Under][0],Player->arTreasury[Under][1],Shop);
					Gem=Player->GetItemGemRepPrice(Player->arTreasury[Under][0],Player->arTreasury[Under][1],Shop);
				};
				if (Mode==12)
				{
					Gold=(pFirstItem+Player->arTreasury[Under][0])->iGoldCost/4;
					Gem=(pFirstItem+Player->arTreasury[Under][0])->iGemCost/4;
					if ((pFirstItem+Player->arTreasury[Under][0])->iDurability>0)
					{
						Gold-=(pFirstItem+Player->arTreasury[Under][0])->iGoldCost*((pFirstItem+Player->arTreasury[Under][0])->iDurability-Player->arTreasury[Under][1])/(pFirstItem+Player->arTreasury[Under][0])->iDurability/8;
						Gem-=(pFirstItem+Player->arTreasury[Under][0])->iGemCost*((pFirstItem+Player->arTreasury[Under][0])->iDurability-Player->arTreasury[Under][1])/(pFirstItem+Player->arTreasury[Under][0])->iDurability/8;
					};
				};
				MakeItemHint(Player->arTreasury[i][0],Player->arTreasury[i][1],Hero, Mode, Gold, Gem);
				bHint=true;
			};
		};
		if (Check==30)
		{
			return i;			
		};
		if (Check==40)
		{
			if (Player->arTreasury[i][0]>0)
			{
				bHint=false;
				WindowItemInfo(Player->arTreasury[i][0],Player->arTreasury[i][1]);
			};
	
		};

	};
	return -1;
};

void GUI_TreasuryBlock::Show()
{
	rectfill(Pic,0,0,width,height,makecol(255,0,255));
	for (int i=0; i<TREASURY_SIZE; i++)
	{
		if (Player->arTreasury[i][0]>0)
		{
			draw_sprite(Pic,(pFirstItem+Player->arTreasury[i][0])->Pic,(i%9)*w,(i-i%9)/9*h);
			if (Player->arTreasury[i][1]==0)
			{
				draw_sprite(Pic,bmpGUI_SpellShadow,(i%9)*w,(i-i%9)/9*h);
				
			};
		};
	};
};


//***************************************************
//*** Экран обмена между героями ********************
//***************************************************

void ScreenHeroHero(dcHero * Hero1, dcHero * Hero2)
{
	iAntisleep=iAntisleepDelay;
	GUI_InfoBlock blInfo[2];
	GUI_ItemBlock blItem[2];
	GUI_UnitBlock blUnit[2];
	GUI_StatBlock blStat[2];
	dcHero * Herr[2];
	Herr[0]=Hero1; 
	Herr[1]=Hero2;
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpGUI_HeroTrade);
	BITMAP * bmpMouse = create_bitmap(150,130);
	int SelectedHero;
	int SelectedSlot=0;
	int SelectedItem=0, SelectedItemDur;
	dcUnit * SelectedUnit=NULL;
	objGUI=new G_Object(objGUI,512,0,1,bmpGUI_HeroTrade);
	objGUI=blItem[0].Init(Hero1,207,14,objGUI);//205,11 207,13
	objGUI=blItem[1].Init(Hero2,719,14,objGUI);//717 719
	objGUI=blUnit[0].Init(Hero1,59,411,objGUI);//12,405 59,411
	objGUI=blUnit[1].Init(Hero2,571,411,objGUI);//524 571
	objGUI=blStat[0].Init(Hero1,8,278,objGUI);
	objGUI=blStat[1].Init(Hero2,520,278,objGUI);
	objGUI=blInfo[0].Init(Hero1,0,0,objGUI);
	objGUI=blInfo[1].Init(Hero2,512,0,objGUI);
	blInfo[0].Show();
	blInfo[1].Show();

	//UI_Button1 * buttTrash1=new UI_Button1();
	//objGUI=buttTrash1->Init(330,595,1,3,bmpGUI_ButtTrashNorm,bmpGUI_ButtTrashAct,bmpGUI_ButtTrashInact,bmpGUI_ButtTrashInact,objGUI);
	//buttTrash1->Activate(0);
	//buttTrash1->ipHotkey=KEY_DEL;

	//UI_Button1 * buttTrash2=new UI_Button1();
	//objGUI=buttTrash2->Init(842,595,1,3,bmpGUI_ButtTrashNorm,bmpGUI_ButtTrashAct,bmpGUI_ButtTrashInact,bmpGUI_ButtTrashInact,objGUI);
	//buttTrash2->Activate(0);

	//UI_Button1 buttExit1;
	//objGUI=buttExit1.Init(453,725,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	//buttExit1.Activate(1);
	//buttExit1.ipHotkey=KEY_ESC;
	//buttExit1.iHint=135;

	//UI_Button1 buttExit2;
	//objGUI=buttExit2.Init(965,725,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	//buttExit2.Activate(1);
	//buttExit2.iHint=135;


	UI_Button1 * buttTrash1=new UI_Button1();
	objGUI=buttTrash1->Init(355,504,1,3,bmpButton[229],bmpButton[230],bmpButton[231],bmpButton[232],objGUI);
	buttTrash1->Activate(0);
	buttTrash1->ipHotkey=KEY_DEL;
	buttTrash1->wavSound=wavDelete;

	UI_Button1 * buttTrash2=new UI_Button1();
	objGUI=buttTrash2->Init(867,504,1,3,bmpButton[229],bmpButton[230],bmpButton[231],bmpButton[232],objGUI);
	buttTrash2->Activate(0);
	buttTrash2->wavSound=wavDelete;

	UI_Button1 buttExit1;
	objGUI=buttExit1.Init(383,582,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[226],objGUI);
	buttExit1.Activate(1);
	buttExit1.ipHotkey=KEY_ESC;
	buttExit1.iHint=135;

	UI_Button1 buttExit2;
	objGUI=buttExit2.Init(895,582,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[226],objGUI);
	buttExit2.Activate(1);
	buttExit2.iHint=135;


	set_mouse_sprite(bmpMousePointer[0]);

	while (!((buttExit1.Check())||(buttExit2.Check())))
	{		
		for (int A=0; A<2; A++)
		{
//Обмен предметами
//****************

		int Click=0;
		Click=blItem[A].Check(0,NULL,SelectedItem);
		if (Click>=0)
		{
	//Клик на кукле героя		
			if (Click==0)
			{
			Click=(pFirstItem+SelectedItem)->ipSlot;
			if ((Herr[A]->CanUse(SelectedItem))&&(Click>0))
			{
				switch (Click)
				{
				case 10:
					if ((Herr[A]->arInventory[11][0]==0)&&(Herr[A]->arInventory[10][0]!=0))
					{Click=11;};
					break;
				case 11:
					Click=12;
					if (Herr[A]->arInventory[12][0]==0)
					{break;}				
					if (Herr[A]->arInventory[13][0]==0)
					{
						Click=13;
						break;
					};
					if (Herr[A]->arInventory[14][0]==0)
					{
						Click=14;
						break;
					};
					if (Herr[A]->arInventory[15][0]==0)
					{
						Click=15;						
					};
					break;
				case 100:
					Click=1;
					break;
				};
			}
			else
			{
				Click=16;
				for (int i=16; i<INVENTORY_SIZE+1; i++)
				{
					if (Herr[A]->arInventory[i][0]==0)
					{
						Click=i;
						break;
					};
				};
			};
			};


			if (SelectedItem)
			{				
	//Попытка поместить предмет в...
				if (((SlotNum(Click)==(pFirstItem+SelectedItem)->ipSlot)&&(Herr[A]->CanUse(SelectedItem)))
					&&((SlotNum(Click)!=2)||(Herr[A]->arInventory[1][0]==0)||((pFirstItem+Herr[A]->arInventory[1][0])->ipSlot!=100)||(GetFreeSlot(Herr[A],0)))
					||(((pFirstItem+SelectedItem)->ipSlot==100)&&(SlotNum(Click)==1)&&(Herr[A]->CanUse(SelectedItem))&&
					((Herr[A]->arInventory[Click][0]==0)||(Herr[A]->arInventory[2][0]==0)||(GetFreeSlot(Herr[A],SelectedSlot*(1-abs(A-SelectedHero))))))
					||(Click>15))
				{
					if (Herr[A]->arInventory[Click][0]>0)
					{
	//Занятый слот
						if (((SlotNum(SelectedSlot)==(pFirstItem+Herr[A]->arInventory[Click][0])->ipSlot)&&
							((Herr[SelectedHero]->CanUse(Herr[A]->arInventory[Click][0])))||(GetFreeSlot(Herr[SelectedHero],0)))
							||(((pFirstItem+Herr[A]->arInventory[Click][0])->ipSlot==100)&&(SlotNum(SelectedSlot)==1)&&
							((Herr[SelectedHero]->arInventory[2][0]==0)||(GetFreeSlot(Herr[SelectedHero],SelectedSlot))))
							||(SelectedSlot>15))
						{
							if (((!Herr[SelectedHero]->CanUse(Herr[A]->arInventory[Click][0]))||
								(SlotNum(SelectedSlot)!=(pFirstItem+Herr[A]->arInventory[Click][0])->ipSlot))&&(SelectedSlot<16))
							//Перемещение предмета в инвентарь, если отдающий герой не может его использовать
							{
								int Free=GetFreeSlot(Herr[SelectedHero],0);
								Herr[SelectedHero]->arInventory[Free][0]=Herr[A]->arInventory[Click][0];
								Herr[SelectedHero]->arInventory[Free][1]=Herr[A]->arInventory[Click][1];								
							}
							else
							{
								Herr[SelectedHero]->arInventory[SelectedSlot][0]=Herr[A]->arInventory[Click][0];
								Herr[SelectedHero]->arInventory[SelectedSlot][1]=Herr[A]->arInventory[Click][1];
							};
							//Перемещение щита в инвентарь, если отдающий герой получил двуручный предмет
							if (((pFirstItem+Herr[A]->arInventory[Click][0])->ipSlot==100)&&(Herr[SelectedHero]->arInventory[2][0]>0)
								&&Herr[SelectedHero]->CanUse(Herr[A]->arInventory[Click][0])&&(SelectedSlot==1))
							{
								int Free=GetFreeSlot(Herr[SelectedHero],SelectedSlot);
								Herr[SelectedHero]->arInventory[Free][0]=Herr[SelectedHero]->arInventory[2][0];
								Herr[SelectedHero]->arInventory[Free][1]=Herr[SelectedHero]->arInventory[2][1];
								Herr[SelectedHero]->arInventory[2][1]=0;
								Herr[SelectedHero]->arInventory[2][0]=0;
							};
							Herr[A]->arInventory[Click][0]=SelectedItem;
							Herr[A]->arInventory[Click][1]=SelectedItemDur;
							//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
							if ((Click==1)&&((pFirstItem+SelectedItem)->ipSlot==100)&&(Herr[A]->arInventory[2][0]>0))
							{
								int Free=GetFreeSlot(Herr[A],SelectedSlot*(1-abs(A-SelectedHero)));
								Herr[A]->arInventory[Free][0]=Herr[A]->arInventory[2][0];
								Herr[A]->arInventory[Free][1]=Herr[A]->arInventory[2][1];
								Herr[A]->arInventory[2][1]=0;
								Herr[A]->arInventory[2][0]=0;
							};
							//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
							if ((Click==2)&&(Herr[A]->arInventory[1][0]>0)&&((pFirstItem+Herr[A]->arInventory[1][0])->ipSlot==100))
							{
								int Free=GetFreeSlot(Herr[A],0);
								Herr[A]->arInventory[Free][0]=Herr[A]->arInventory[1][0];
								Herr[A]->arInventory[Free][1]=Herr[A]->arInventory[1][1];
								Herr[A]->arInventory[1][1]=0;
								Herr[A]->arInventory[1][0]=0;
							};
							SelectedItem=0;
							SelectedSlot=0;
							blItem[A].Show();
							blItem[SelectedHero].Show();
							blStat[A].Show();
							blStat[SelectedHero].Show();
							blInfo[0].Show();
							blInfo[1].Show();
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash1->Activate(0);
							buttTrash2->Activate(0);
						};
					}
					else
					{	
	//Свободный слот
						if (((pFirstItem+SelectedItem)->ipSlot==100)&&(Herr[A]->arInventory[2][0]>0)&&(Click==1)&&(GetFreeSlot(Herr[A],0)==0))
						{}
						else
						{
						Herr[A]->arInventory[Click][0]=SelectedItem;
						Herr[A]->arInventory[Click][1]=SelectedItemDur;	
						//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
						if (((pFirstItem+SelectedItem)->ipSlot==100)&&(Herr[A]->arInventory[2][0]>0)&&(Click==1))
						{
							int Free=GetFreeSlot(Herr[A],0);
							Herr[A]->arInventory[Free][0]=Herr[A]->arInventory[2][0];
							Herr[A]->arInventory[Free][1]=Herr[A]->arInventory[2][1];
							Herr[A]->arInventory[2][1]=0;
							Herr[A]->arInventory[2][0]=0;
						};
						//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
						if ((Click==2)&&(Herr[A]->arInventory[1][0]>0)&&((pFirstItem+Herr[A]->arInventory[1][0])->ipSlot==100))
						{
							int Free=GetFreeSlot(Herr[A],0);
							Herr[A]->arInventory[Free][0]=Herr[A]->arInventory[1][0];
							Herr[A]->arInventory[Free][1]=Herr[A]->arInventory[1][1];
							Herr[A]->arInventory[1][1]=0;
							Herr[A]->arInventory[1][0]=0;
						};
						SelectedItem=0;
						SelectedSlot=0;
						blItem[A].Show();
						blStat[A].Show();	
						blInfo[0].Show();
						blInfo[1].Show();
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash1->Activate(0);
						buttTrash2->Activate(0);
						};
					};
				};
			}
			else
			{
	//Взятие предмета
				if (Herr[A]->arInventory[Click][0]>0)
				{
					if (SelectedUnit)
					{
						Herr[SelectedHero]->PlaceUnit(SelectedUnit,SelectedSlot);
						SelectedUnit=NULL;
						blUnit[SelectedHero].Show();
					};
					SelectedItem=Herr[A]->arInventory[Click][0];
					SelectedItemDur=Herr[A]->arInventory[Click][1];
					SelectedSlot=Click;
					SelectedHero=A;
					Herr[A]->arInventory[Click][0]=0;
					Herr[A]->arInventory[Click][1]=0;
					blItem[0].Show();
					blItem[1].Show();
					blStat[A].Show();
					blInfo[0].Show();
					blInfo[1].Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash1->Activate(1);
					buttTrash2->Activate(1);
				};
			};
		};

//Обмен юнитами
//*************
		Click=0;
		Click=blUnit[A].Check(SelectedUnit);
		if (Click>0)
		{
			if (SelectedUnit)
			{				
			//Попытка поместить юнита в...
				if (SlotLvl(Click)>=SelectedUnit->pType()->iLevel)					
				{
					if (Herr[A]->pSquad[Click])
					{
	//Занятый слот
						if (GetFreeUnitSlot(Herr[SelectedHero],Herr[A]->pSquad[Click]->pType()->iLevel))
						{
							if (SlotLvl(SelectedSlot)<Herr[A]->pSquad[Click]->pType()->iLevel)
							//Перемещение юнита в свободный слот нужного уровня передающему герою
							{
								int Free=GetFreeUnitSlot(Herr[SelectedHero],Herr[A]->pSquad[Click]->pType()->iLevel);
							//	Herr[SelectedHero]->pSquad[Free]=Herr[A]->pSquad[Click];
								Herr[SelectedHero]->PlaceUnit(Herr[A]->pSquad[Click], Free);
							}
							else
							{
							//	Herr[SelectedHero]->pSquad[SelectedSlot]=Herr[A]->pSquad[Click];
								Herr[SelectedHero]->PlaceUnit(Herr[A]->pSquad[Click], SelectedSlot);
							};
							Herr[A]->pSquad[Click]=NULL;
							Herr[A]->PlaceUnit(SelectedUnit,Click);
							//Herr[A]->pSquad[Click]=SelectedUnit;							
							SelectedUnit=NULL;
							SelectedSlot=0;
							blUnit[A].Show();
							blInfo[0].Show();
							blInfo[1].Show();
							blUnit[SelectedHero].Show();
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash1->Activate(0);
							buttTrash2->Activate(0);
						};
					}
					else
					{	
	//Свободный слот						
						Herr[A]->pSquad[Click]=NULL;
						Herr[A]->PlaceUnit(SelectedUnit,Click);
					//	Herr[A]->pSquad[Click]=SelectedUnit;							
						SelectedUnit=NULL;
						SelectedSlot=0;
						blInfo[0].Show();
						blInfo[1].Show();
						blUnit[A].Show();
						blUnit[SelectedHero].Show();
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash1->Activate(0);
						buttTrash2->Activate(0);
					};
				};
			}
			else
			{
	//Взятие юнита
				if (Herr[A]->pSquad[Click])
				{
					if (SelectedItem)
					{
						Herr[SelectedHero]->arInventory[SelectedSlot][0]=SelectedItem;
						Herr[SelectedHero]->arInventory[SelectedSlot][1]=SelectedItemDur;	
						SelectedItem=0;
						blItem[SelectedHero].Show();
					};
					SelectedUnit=Herr[A]->pSquad[Click];					
					SelectedSlot=Click;
					SelectedHero=A;
					Herr[A]->pSquad[Click]=NULL;					
					blUnit[A].Show();
					blInfo[0].Show();
					blInfo[1].Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,SelectedUnit->pType()->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash1->Activate(1);
					buttTrash2->Activate(1);
				};
			};
		};
		};
		if ((buttTrash1->Check())||(buttTrash2->Check()))
		{
			if (SelectedUnit)
			{
				int WillDel=1;
				if (arHumans[arPlayer[Herr[0]->ipPlayer].iColor].arMess[4])
				{WillDel=(pFirstDialog+1461)->Show(1,1,0,0,NULL,NULL);}
				if (WillDel==1)
				{
					SelectedUnit->Dismiss();
					SelectedUnit=NULL;
					SelectedSlot=0;
					buttTrash1->Activate(0);
					buttTrash2->Activate(0);
					set_mouse_sprite(bmpMousePointer[0]);
				};
			};
			if (SelectedItem>0)
			{
				int WillDel=1;
				if (arHumans[arPlayer[Herr[0]->ipPlayer].iColor].arMess[4])
				{WillDel=(pFirstDialog+1462)->Show(1,1,0,0,NULL,NULL);}
				if (WillDel==1)
				{
					SelectedItem=0;					
					SelectedSlot=0;
					buttTrash1->Activate(0);
					buttTrash2->Activate(0);
					set_mouse_sprite(bmpMousePointer[0]);
				};
			};
		};
		blInfo[0].Check();
		blInfo[1].Check();
		blStat[0].Check();
		blStat[1].Check();
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,10,true);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	if (SelectedItem)
	{
		Herr[SelectedHero]->arInventory[SelectedSlot][0]=SelectedItem;
		Herr[SelectedHero]->arInventory[SelectedSlot][1]=SelectedItemDur;
	};
	if (SelectedUnit)
	{
		Herr[SelectedHero]->pSquad[SelectedSlot]=SelectedUnit;
	};
	set_mouse_sprite(bmpMousePointer[0]);
	destroy_bitmap(bmpMouse);
	delete buttTrash1;
	delete buttTrash2;	
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	bHint=false;
	HintType=0;
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Экран просмотра героя *************************
//***************************************************

bool ScreenHero(dcHero * Hero, dcUnit *Ally, bool CanDelete)
{
	iAntisleep=iAntisleepDelay;
	bool NoExit=false;
	GUI_InfoBlock blInfo;
	GUI_ItemBlock blItem;
	GUI_UnitBlock blUnit;
	GUI_StatBlock blStat;
	GUI_SpellBlock blSpell;	
	GUI_SkillBlock blSkill;
	GUI_QuestBlock blQuest;
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpGUI_HeroScreen);
	BITMAP * bmpMouse = create_bitmap(150,130);	
	int SelectedSlot=0;
	int SelectedItem=0, SelectedItemDur;
	int SelectedSpell=0;
	dcUnit * SelectedUnit=NULL;	
	objGUI=blItem.Init(Hero,356,13,objGUI);	
	objGUI=blUnit.Init(Hero,59,411,objGUI);	
	objGUI=blStat.Init(Hero,8,278,objGUI);	
	objGUI=blInfo.Init(Hero,0,0,objGUI);	
	objGUI=blSpell.Init(Hero,536,520,objGUI);
	objGUI=blSkill.Init(Hero,218,16,objGUI);
	objGUI=blQuest.Init(Hero,664,16,objGUI);
	blInfo.Show();
	blStat.Show();
	blUnit.Show();
	blItem.Show();
	blSpell.Show();
	blSkill.Show();
	blQuest.Show();
	

	UI_Button1 buttYuPlay;
	UI_Button1 * buttTrash=new UI_Button1();
#ifdef YUPLAY
	if (iYuPlay)
	{
		objGUI=buttTrash->Init(347,510,1,3,bmpButton[229],bmpButton[230],bmpButton[231],bmpButton[232],objGUI);
		objGUI=buttYuPlay.Init(276,510,1,3,bmpYuPlay[0],bmpYuPlay[1],bmpYuPlay[2],bmpYuPlay[3],objGUI);
		if ((!Tutorial.Map)&&(CurGame.iGameType<3))
		{buttYuPlay.Activate(1);}
		else
		{buttYuPlay.Activate(0);};
		buttYuPlay.iHint=513;
	}
	else
#endif
	{objGUI=buttTrash->Init(319,510,1,3,bmpButton[229],bmpButton[230],bmpButton[231],bmpButton[232],objGUI);};
	buttTrash->Activate(0);
	buttTrash->ipHotkey=KEY_DEL;
	buttTrash->wavSound=wavDelete;

	UI_Button1 buttExit;
	objGUI=buttExit.Init(920,623,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;

	UI_Button1 buttDismiss;
	objGUI=buttDismiss.Init(246,444,1,3,bmpButton[166],bmpButton[167],bmpButton[168],bmpButton[169],objGUI);
	if ((!CanDelete)||(Tutorial.Map))
	{buttDismiss.Activate(-1);}
	else
	{buttDismiss.Activate(1);};
	buttDismiss.iHint=142;


	set_mouse_sprite(bmpMousePointer[0]);

	bool bRepeat=true;
	if (Ally)
	{
		SelectedUnit=Ally;
		SelectedSlot=0;		
		rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
		draw_sprite(bmpMouse,SelectedUnit->pType()->Pic,0,0);
		draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
		set_mouse_sprite(bmpMouse);
		buttTrash->Activate(1);	
	};
	while (((!buttExit.Check())||(NoExit))&&(bRepeat))
	{
		if	(!key[KEY_ESC])
		{NoExit=false;};

#ifdef YUPLAY
		if ((iYuPlay))
		{
			if (buttYuPlay.Check())
			{
			if (InstallYuPlay())
			{
				objGUI=ShowActiveObjects(objGUI,10,false);
				WindowLottery(0);
				if (ipLottPrize>0)
				{
					switch (ipLottPrizeType)
					{
					case 1:
						if (SelectedUnit)
						{
							Hero->PlaceUnit(SelectedUnit,SelectedSlot);
							SelectedUnit=NULL;
							blUnit.Show();
						};
						if (SelectedSpell)
						{
							Hero->arSpell[SelectedSlot]=SelectedSpell;		
							SelectedSpell=0;
							blSpell.Show();
						};
						if ((SelectedItem)&&(SelectedSlot>0))
						{
							Hero->arInventory[SelectedSlot][0]=SelectedItem;
							Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
							blItem.Show();
						};
						SelectedItem=ipLottPrize;
						SelectedItemDur=(pFirstItem+ipLottPrize)->iDurability;
						SelectedSlot=0;
						blStat.Show();
						blInfo.Show();
						rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
						draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
						draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
						set_mouse_sprite(bmpMouse);
						buttTrash->Activate(1);						
						break;
					case 2:
						if (Hero->GetUnitSlot(pLottUnit->GetRank())>0)
						{
						if (SelectedUnit)
						{
							Hero->PlaceUnit(SelectedUnit,SelectedSlot);
							SelectedUnit=NULL;
							blUnit.Show();
						};
						if (SelectedSpell)
						{
							Hero->arSpell[SelectedSlot]=SelectedSpell;		
							SelectedSpell=0;
							blSpell.Show();
						};
						if ((SelectedItem)&&(SelectedSlot>0))
						{
							Hero->arInventory[SelectedSlot][0]=SelectedItem;
							Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
							SelectedItem=0;
							blItem.Show();
						};
						SelectedUnit=pLottUnit;						
						SelectedSlot=0;
						blStat.Show();
						blInfo.Show();
						rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
						draw_sprite(bmpMouse,SelectedUnit->pType()->Pic,0,0);
						draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
						set_mouse_sprite(bmpMouse);
						buttTrash->Activate(1);
						}
						else
						{
							int Pay=pLottUnit->GetRank()*(pLottUnit->GetGoldPrice()+pLottUnit->GetGemPrice())+Random(pLottUnit->GetGoldPrice());
							char Txt[500];
							sprintf(Txt,(pFirstDialog+1249)->sText,Pay);
							(pFirstDialog+1249)->Show(1,1,0,0,NULL,Txt);
							arPlayer[Hero->ipPlayer].Gold+=Pay;		
							delete pLottUnit;
							pLottUnit=NULL;
						}
						break;
					case 3:
						if (Hero->GetSpellSlot((pFirstSpell+ipLottPrize)->iLevel)>0)
						{
						if (SelectedUnit)
						{
							Hero->PlaceUnit(SelectedUnit,SelectedSlot);
							SelectedUnit=NULL;
							blUnit.Show();
						};
						if ((SelectedSpell)&&(SelectedSlot>0))
						{
							Hero->arSpell[SelectedSlot]=SelectedSpell;		
							SelectedSpell=0;
							blSpell.Show();
						};
						if ((SelectedItem)&&(SelectedSlot>0))
						{
							Hero->arInventory[SelectedSlot][0]=SelectedItem;
							Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
							SelectedItem=0;
							blItem.Show();
						};
						SelectedSpell=ipLottPrize;						
						SelectedSlot=0;
						blStat.Show();
						blInfo.Show();
						rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
						draw_sprite(bmpMouse,(pFirstSpell+SelectedSpell)->Pic,0,0);
						draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
						set_mouse_sprite(bmpMouse);
						buttTrash->Activate(1);
						}
						else
						{
							int Pay=(pFirstSpell+ipLottPrize)->iLevel*(pFirstSpell+ipLottPrize)->iLevel*((pFirstSpell+ipLottPrize)->iItemLevel+1)*4+Random((pFirstSpell+ipLottPrize)->iLevel*(pFirstSpell+ipLottPrize)->iLevel*((pFirstSpell+ipLottPrize)->iItemLevel+1)*2);
							char Txt[500];
							sprintf(Txt,(pFirstDialog+1250)->sText,Pay);
							(pFirstDialog+1250)->Show(1,1,0,0,NULL,Txt);
							arPlayer[Hero->ipPlayer].Gem+=Pay;							
						}						
						break;
					case 4:
						if (*(arPlayer[Hero->ipPlayer].pDefender+ipLottPrize)>=0)
						{*(arPlayer[Hero->ipPlayer].pDefender+ipLottPrize)+=1;}
						break;
					case 5:
						arPlayer[Hero->ipPlayer].Gold+=ipLottPrize;
						break;
					case 6:
						arPlayer[Hero->ipPlayer].Gem+=ipLottPrize;
						break;
					};
				};
			}
			};
		};
#endif
		

//Манипулирование предметами
//**************************		
		int Click=0;
		Click=blItem.Check(0,NULL,SelectedItem);
		if (Click>=0)
		{
	//Клик на кукле героя		
			if (Click==0)
			{
			Click=(pFirstItem+SelectedItem)->ipSlot;
			if ((Hero->CanUse(SelectedItem))&&(Click>0))
			{
				switch (Click)
				{
				case 10:
					if ((Hero->arInventory[11][0]==0)&&(Hero->arInventory[10][0]!=0))
					{Click=11;};
					break;
				case 11:
					Click=12;
					if (Hero->arInventory[12][0]==0)
					{break;}				
					if (Hero->arInventory[13][0]==0)
					{
						Click=13;
						break;
					};
					if (Hero->arInventory[14][0]==0)
					{
						Click=14;
						break;
					};
					if (Hero->arInventory[15][0]==0)
					{
						Click=15;						
					};
					break;
				case 100:
					Click=1;
					break;
				};
			}
			else
			{
				Click=16;
				for (int i=16; i<INVENTORY_SIZE+1; i++)
				{
					if (Hero->arInventory[i][0]==0)
					{
						Click=i;
						break;
					};
				};
			};
			};


			if (SelectedItem)
			{				
	//Попытка поместить предмет в...
				if (((SlotNum(Click)==(pFirstItem+SelectedItem)->ipSlot)&&(Hero->CanUse(SelectedItem)))
					&&((SlotNum(Click)!=2)||(Hero->arInventory[1][0]==0)||((pFirstItem+Hero->arInventory[1][0])->ipSlot!=100)||(GetFreeSlot(Hero,0)))
					||(((pFirstItem+SelectedItem)->ipSlot==100)&&(SlotNum(Click)==1)&&(Hero->CanUse(SelectedItem))&&
					((Hero->arInventory[Click][0]==0)||(Hero->arInventory[2][0]==0)||(GetFreeSlot(Hero,SelectedSlot))))
					||(Click>15))
				{
					if (Hero->arInventory[Click][0]>0)
					{
						if (SelectedSlot==0)
						{
							//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
							if ((Click==1)&&((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0))
							{
								int Free=GetFreeSlot(Hero,SelectedSlot);
								Hero->arInventory[Free][0]=Hero->arInventory[2][0];
								Hero->arInventory[Free][1]=Hero->arInventory[2][1];
								Hero->arInventory[2][1]=0;
								Hero->arInventory[2][0]=0;
							};
							//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
							if ((Click==2)&&(Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100))
							{
								int Free=GetFreeSlot(Hero,0);
								Hero->arInventory[Free][0]=Hero->arInventory[1][0];
								Hero->arInventory[Free][1]=Hero->arInventory[1][1];
								Hero->arInventory[1][1]=0;
								Hero->arInventory[1][0]=0;
							};
							int Temp=Hero->arInventory[Click][0], Temp1=Hero->arInventory[Click][1];
							Hero->arInventory[Click][0]=SelectedItem;
							Hero->arInventory[Click][1]=SelectedItemDur;
							SelectedItem=Temp;
							SelectedItemDur=Temp1;
							rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
							draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
							draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
							set_mouse_sprite(bmpMouse);
							blItem.Show();
							blStat.Show();
							blInfo.Show();
						};
	//Занятый слот
						if (((SlotNum(SelectedSlot)==(pFirstItem+Hero->arInventory[Click][0])->ipSlot)&&
							((Hero->CanUse(Hero->arInventory[Click][0])))||(GetFreeSlot(Hero,0)))
							||(((pFirstItem+Hero->arInventory[Click][0])->ipSlot==100)&&(SlotNum(SelectedSlot)==1)&&
							((Hero->arInventory[2][0]==0)||(GetFreeSlot(Hero,SelectedSlot))))
							||(SelectedSlot>15))
						{
							if (((!Hero->CanUse(Hero->arInventory[Click][0]))||
								(SlotNum(SelectedSlot)!=(pFirstItem+Hero->arInventory[Click][0])->ipSlot))&&(SelectedSlot<16))
							//Перемещение предмета в инвентарь, если отдающий герой не может его использовать
							{
								int Free=GetFreeSlot(Hero,0);
								Hero->arInventory[Free][0]=Hero->arInventory[Click][0];
								Hero->arInventory[Free][1]=Hero->arInventory[Click][1];								
							}
							else
							{
								Hero->arInventory[SelectedSlot][0]=Hero->arInventory[Click][0];
								Hero->arInventory[SelectedSlot][1]=Hero->arInventory[Click][1];
							};
							//Перемещение щита в инвентарь, если отдающий герой получил двуручный предмет
							if (((pFirstItem+Hero->arInventory[Click][0])->ipSlot==100)&&(Hero->arInventory[2][0]>0)
								&&Hero->CanUse(Hero->arInventory[Click][0])&&(SelectedSlot==1))
							{
								int Free=GetFreeSlot(Hero,SelectedSlot);
								Hero->arInventory[Free][0]=Hero->arInventory[2][0];
								Hero->arInventory[Free][1]=Hero->arInventory[2][1];
								Hero->arInventory[2][1]=0;
								Hero->arInventory[2][0]=0;
							};
							Hero->arInventory[Click][0]=SelectedItem;
							Hero->arInventory[Click][1]=SelectedItemDur;
							//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
							if ((Click==1)&&((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0))
							{
								int Free=GetFreeSlot(Hero,SelectedSlot);
								Hero->arInventory[Free][0]=Hero->arInventory[2][0];
								Hero->arInventory[Free][1]=Hero->arInventory[2][1];
								Hero->arInventory[2][1]=0;
								Hero->arInventory[2][0]=0;
							};
							//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
							if ((Click==2)&&(Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100))
							{
								int Free=GetFreeSlot(Hero,0);
								Hero->arInventory[Free][0]=Hero->arInventory[1][0];
								Hero->arInventory[Free][1]=Hero->arInventory[1][1];
								Hero->arInventory[1][1]=0;
								Hero->arInventory[1][0]=0;
							};
							SelectedItem=0;
							SelectedSlot=0;
							blItem.Show();							
							blStat.Show();
							blInfo.Show();
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash->Activate(0);							
						};
					}
					else
					{	
	//Свободный слот
						Hero->arInventory[Click][0]=SelectedItem;
						Hero->arInventory[Click][1]=SelectedItemDur;	
						//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
						if (((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0)&&(Click==1))
						{
							int Free=GetFreeSlot(Hero,0);
							Hero->arInventory[Free][0]=Hero->arInventory[2][0];
							Hero->arInventory[Free][1]=Hero->arInventory[2][1];
							Hero->arInventory[2][1]=0;
							Hero->arInventory[2][0]=0;
						};
						//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
						if ((Click==2)&&(Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100))
						{
							int Free=GetFreeSlot(Hero,0);
							Hero->arInventory[Free][0]=Hero->arInventory[1][0];
							Hero->arInventory[Free][1]=Hero->arInventory[1][1];
							Hero->arInventory[1][1]=0;
							Hero->arInventory[1][0]=0;
						};
						SelectedItem=0;
						SelectedSlot=0;
						blItem.Show();
						blStat.Show();
						blInfo.Show();
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash->Activate(0);						
					};
				};
			}
			else
			{
	//Взятие предмета
				if (Hero->arInventory[Click][0]>0)
				{
					if (SelectedUnit)
					{
						Hero->PlaceUnit(SelectedUnit,SelectedSlot);
						SelectedUnit=NULL;
						blUnit.Show();
					};
					if (SelectedSpell)
					{
						Hero->arSpell[SelectedSlot]=SelectedSpell;		
						SelectedSpell=0;
						blSpell.Show();
					};
					SelectedItem=Hero->arInventory[Click][0];
					SelectedItemDur=Hero->arInventory[Click][1];
					SelectedSlot=Click;					
					Hero->arInventory[Click][0]=0;
					Hero->arInventory[Click][1]=0;
					blItem.Show();
					blStat.Show();
					blInfo.Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash->Activate(1);					
				};
			};
			
		};

//Манипулирование юнитами
//***********************
		Click=0;
		Click=blUnit.Check(SelectedUnit);
		if ((Click>0)&&(Click<1000))
		{
			if (SelectedUnit)
			{				
			//Попытка поместить юнита в...
				if (SlotLvl(Click)>=SelectedUnit->pType()->iLevel)					
				{
					if (Hero->pSquad[Click])
					{
	//Занятый слот
						if (GetFreeUnitSlot(Hero,Hero->pSquad[Click]->pType()->iLevel))
						{
							if ((SelectedSlot==0)||(SlotLvl(SelectedSlot)<Hero->pSquad[Click]->pType()->iLevel))
							//Перемещение юнита в свободный слот нужного уровня передающему герою
							{
								int Free=GetFreeUnitSlot(Hero,Hero->pSquad[Click]->pType()->iLevel);							
								Hero->PlaceUnit(Hero->pSquad[Click], Free);
							}
							else
							{							
								Hero->PlaceUnit(Hero->pSquad[Click], SelectedSlot);
							};
							Hero->pSquad[Click]=NULL;
							Hero->PlaceUnit(SelectedUnit,Click);							
							SelectedUnit=NULL;
							SelectedSlot=0;
							blUnit.Show();
							blInfo.Show();
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash->Activate(0);							
						}
						else
						{
							dcUnit *Temp=Hero->pSquad[Click];
							Hero->pSquad[Click]=NULL;
							Hero->PlaceUnit(SelectedUnit,Click);
							SelectedUnit=Temp;
							SelectedSlot=0;
							blUnit.Show();
							blInfo.Show();
							rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
							draw_sprite(bmpMouse,SelectedUnit->pType()->Pic,0,0);
							draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
						};
					}
					else
					{	
	//Свободный слот						
						Hero->pSquad[Click]=NULL;
						Hero->PlaceUnit(SelectedUnit,Click);
					//	Herr[A]->pSquad[Click]=SelectedUnit;							
						SelectedUnit=NULL;
						SelectedSlot=0;
						blUnit.Show();
						blInfo.Show();
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash->Activate(0);						
					};
				};
			}
			else
			{
	//Взятие юнита
				if (Hero->pSquad[Click])
				{
					if (SelectedItem)
					{
						Hero->arInventory[SelectedSlot][0]=SelectedItem;
						Hero->arInventory[SelectedSlot][1]=SelectedItemDur;	
						SelectedItem=0;
						blItem.Show();
					};
					if (SelectedSpell)
					{
						Hero->arSpell[SelectedSlot]=SelectedSpell;		
						SelectedSpell=0;
						blSpell.Show();
					};
					SelectedUnit=Hero->pSquad[Click];					
					SelectedSlot=Click;					
					Hero->pSquad[Click]=NULL;					
					blUnit.Show();
					blInfo.Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,SelectedUnit->pType()->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash->Activate(1);					
				};
			};
		};
		if (Click>1000)
		{
			if (Hero->pSquad[Click%1000])
			{
				WindowUnitInfo(Hero->pSquad[Click%1000],Hero->GetSquadMorale(Click%1000));
				NoExit=true;
			};
		};

//Манипулирование заклинаниями
//****************************
		Click=0;
		Click=blSpell.Check(SelectedSpell);
		if ((Click>0)&&(Click<1000))
		{
//			textprintf_ex(screen,Font1,900,40,makecol(200,200,200),-1,"%d",Click);
			if (SelectedSpell)
			{				
			//Попытка поместить заклинание в...
				if (SpellSlotLvl(Click)>=(pFirstSpell+SelectedSpell)->iLevel)					
				{
					if (Hero->arSpell[Click]>0)
					{
	//Занятый слот
						if (GetFreeSpellSlot(Hero,(pFirstSpell+Hero->arSpell[Click])->iLevel))
						{
							if ((SelectedSlot==0)||(SpellSlotLvl(SelectedSlot)<(pFirstSpell+Hero->arSpell[Click])->iLevel))
							//Перемещение заклинания в свободный слот нужного уровня передающему герою
							{
								int Free=GetFreeSpellSlot(Hero,(pFirstSpell+Hero->arSpell[Click])->iLevel);
								Hero->arSpell[Free]=Hero->arSpell[Click];
							}
							else
							{							
								Hero->arSpell[SelectedSlot]=Hero->arSpell[Click];
							};
							Hero->arSpell[Click]=SelectedSpell;									
							SelectedSpell=0;
							SelectedSlot=0;
							blSpell.Show();							
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash->Activate(0);							
						}
						else
						{
							int Temp=Hero->arSpell[Click];
							Hero->arSpell[Click]=SelectedSpell;
							SelectedSpell=Temp;
							rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
							draw_sprite(bmpMouse,(pFirstSpell+SelectedSpell)->Pic,0,0);
							draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
							set_mouse_sprite(bmpMouse);
							blSpell.Show();
						};
					}
					else
					{	
	//Свободный слот						
						Hero->arSpell[Click]=SelectedSpell;													
						SelectedSpell=0;
						SelectedSlot=0;
						blSpell.Show();						
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash->Activate(0);						
					};
				};
			}
			else
			{
	//Взятие заклинания
				if (Hero->arSpell[Click]>0)
				{
					if (SelectedItem)
					{
						Hero->arInventory[SelectedSlot][0]=SelectedItem;
						Hero->arInventory[SelectedSlot][1]=SelectedItemDur;	
						SelectedItem=0;
						blItem.Show();
						blInfo.Show();
					};
					if (SelectedUnit)
					{
						Hero->PlaceUnit(SelectedUnit,SelectedSlot);
						SelectedUnit=NULL;
						blUnit.Show();
						blInfo.Show();
					};
					SelectedSpell=Hero->arSpell[Click];					
					SelectedSlot=Click;					
					Hero->arSpell[Click]=0;					
					blSpell.Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstSpell+SelectedSpell)->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash->Activate(1);					
				};
			};
		};
		if (Click>1000)
		{
			if (Hero->arSpell[Click%1000]>0)
			{
				WindowSpellInfo(Hero->arSpell[Click%1000]);
				NoExit=true;
			};
		};


		if (buttTrash->Check())
		{
			if (SelectedUnit)
			{
				int WillDel=1;
				if (arHumans[arPlayer[Hero->ipPlayer].iColor].arMess[4])
				{WillDel=(pFirstDialog+1461)->Show(1,1,0,0,NULL,NULL);}
				if (WillDel==1)
				{
					SelectedUnit->Dismiss();
					SelectedUnit=NULL;
					SelectedSlot=0;
					buttTrash->Activate(0);
					set_mouse_sprite(bmpMousePointer[0]);
				};
			};
			if (SelectedItem>0)
			{
				int WillDel=1;
				if (arHumans[arPlayer[Hero->ipPlayer].iColor].arMess[4])
				{WillDel=(pFirstDialog+1462)->Show(1,1,0,0,NULL,NULL);}
				if (WillDel==1)
				{
					SelectedItem=0;					
					SelectedSlot=0;
					buttTrash->Activate(0);
					set_mouse_sprite(bmpMousePointer[0]);
				};
			};
			if (SelectedSpell>0)
			{
				int WillDel=1;
				if (arHumans[arPlayer[Hero->ipPlayer].iColor].arMess[4])
				{
					WillDel=2;
					for (int i=0; i<LIBRARY_SIZE; i++)
					{
						if (arPlayer[Hero->ipPlayer].arLibrary[i]==SelectedSpell)
						{WillDel=1;};
					};
					if (WillDel==2)
					{WillDel=(pFirstDialog+1463)->Show(1,1,0,0,NULL,NULL);};
				}
				if (WillDel==1)
				{
					SelectedSpell=0;
					SelectedSlot=0;
					buttTrash->Activate(0);
					set_mouse_sprite(bmpMousePointer[0]);
				};
			};
		};

		blQuest.Check();
		blSkill.Check();
		blInfo.Check();
		blStat.Check();
		if (buttDismiss.Check())
		{
			if ((pFirstDialog+28)->Show(1,1,0,0,NULL,NULL)==1)
			{
				delete Hero;
				bRepeat=false;
			};
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
	if (bRepeat)
	{

		if ((SelectedItem)&&(SelectedSlot>0))
		{
			Hero->arInventory[SelectedSlot][0]=SelectedItem;
			Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
		};
		if ((SelectedUnit)&&(SelectedSlot>0))
		{
			Hero->PlaceUnit(SelectedUnit,SelectedSlot);
		};
		if ((SelectedSpell)&&(SelectedSlot>0)&&(Hero->arSpell[SelectedSlot]==0)&&(SpellSlotLvl(SelectedSlot)>=(pFirstSpell+SelectedSpell)->iLevel))
		{
			Hero->arSpell[SelectedSlot]=SelectedSpell;		
		};
	};	
	set_mouse_sprite(bmpMousePointer[0]);
	destroy_bitmap(bmpMouse);
	delete buttTrash;
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	bHint=false;
	HintType=0;
	iAntisleep=iAntisleepDelay;
	return bRepeat;
};


//***************************************************
//*** Экран героя в гарнизоне ***********************
//***************************************************

void ScreenHeroGarrison (dcHero * Hero, dcHero * Garrison, bool UseLocalTab)
{
	iAntisleep=iAntisleepDelay;
	bool bMerc;
	GUI_InfoBlock blInfo;
	GUI_ItemBlock blItem;
	GUI_UnitBlock blUnit[2];
	GUI_StatBlock blStat;
	GUI_RecruitBlock blRecruit;
	GUI_ResourceBlock blResource;

	dcHero * Herr[2];
	Herr[0]=Hero; 
	Herr[1]=Garrison;
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpGUI_HeroTrade);
	BITMAP * bmpMouse = create_bitmap(150,130);
	int SelectedHero;
	int SelectedSlot=0;
	int SelectedItem=0, SelectedItemDur;
	dcUnit * SelectedUnit=NULL;
	objGUI=new G_Object(objGUI,512,0,1,bmpGUI_Garrison);
	objGUI=blItem.Init(Hero,207,14,objGUI);
	objGUI=blRecruit.Init(&arPlayer[Hero->ipPlayer],512,0,objGUI,UseLocalTab,Hero->ipProvince);
	objGUI=blResource.Init(&arPlayer[Hero->ipPlayer],512,734,objGUI);
	objGUI=blUnit[0].Init(Hero,59,411,objGUI);
	objGUI=blUnit[1].Init(Garrison,571,373,objGUI);
	objGUI=blStat.Init(Hero,8,278,objGUI);	
	objGUI=blInfo.Init(Hero,0,0,objGUI);	
	blInfo.Show();

	UI_Button1 buttYuPlay;
	UI_Button1 * buttTrash1=new UI_Button1();
#ifdef YUPLAY
	if (iYuPlay)
	{
		objGUI=buttTrash1->Init(362,504,1,3,bmpButton[229],bmpButton[230],bmpButton[231],bmpButton[232],objGUI);
		objGUI=buttYuPlay.Init(284,504,1,3,bmpYuPlay[0],bmpYuPlay[1],bmpYuPlay[2],bmpYuPlay[3],objGUI);
		if ((!Tutorial.Map)&&(CurGame.iGameType<3))
		{buttYuPlay.Activate(1);}
		else
		{buttYuPlay.Activate(0);};
		buttYuPlay.iHint=513;
	}
	else
#endif
	{objGUI=buttTrash1->Init(355,504,1,3,bmpButton[229],bmpButton[230],bmpButton[231],bmpButton[232],objGUI);};
	
	buttTrash1->Activate(0);
	buttTrash1->ipHotkey=KEY_DEL;
	buttTrash1->wavSound=wavDelete;

	UI_Button1 * buttTrash2=new UI_Button1();
	objGUI=buttTrash2->Init(867,460,1,3,bmpButton[229],bmpButton[230],bmpButton[231],bmpButton[232],objGUI);
	buttTrash2->Activate(0);
	buttTrash2->wavSound=wavDelete;

	UI_Button1 buttExit1;
	objGUI=buttExit1.Init(383,582,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[226],objGUI);
	buttExit1.Activate(1);
	buttExit1.ipHotkey=KEY_ESC;
	buttExit1.iHint=135;

	UI_Button1 buttExit2;
	objGUI=buttExit2.Init(895,540,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[226],objGUI);
	buttExit2.Activate(1);
	buttExit2.iHint=135;

	/*UI_Mask1 mskAlchemy;
	mskAlchemy.Init(512+11,727,240,40,1);
	mskAlchemy.iHint=352;*/

	objGUI=new G_Object(objGUI,20,20,-1,bmpGUI_Hint);
	G_Object *objTutorial=objGUI;
	UI_Mask mskTutorial;
	objGUI=mskTutorial.Init(20,20,1,bmpGUI_Hint,bmpEmpty,objGUI,-1);
	UI_TextBox txtTutorial;
	objGUI=txtTutorial.Init(40,50,297,200,5,COLOR_1,Font2,objGUI);
	txtTutorial.bShadow=false;
	txtTutorial.Hide();
	bool bTutHint=false;
	bool bBuy=false;


	set_mouse_sprite(bmpMousePointer[0]);
	if ((Tutorial.Mode)&&(Tutorial.UnitRank==0))
	{
		bool bShow=false;
		for (int i=0; i<MAX_RECRUIT; i++)
		{
			if (UseLocalTab)
			{
				if ((Recruitz[i]>0)&&((pFirstUnitType+Recruitz[i])->iLevel>1))
				{bShow=true;};
			}
			else
			{
				if ((arPlayer[Hero->ipPlayer].arRecruit[i]>0)&&((pFirstUnitType+arPlayer[Hero->ipPlayer].arRecruit[i])->iLevel>1))
				{bShow=true;};
			};
		};
		if (bShow)
		{
			Tutorial.UnitRank=1;
			txtTutorial.Activate((pFirstTutText+86)->msg);
			objTutorial->iLayer=4;
			bTutHint=true;
			ScrollSound();
		};
	};

	if ((Tutorial.Mode)&&(Tutorial.UnitPrice==0)&&(Hero->ipProvince!=arPlayer[Hero->ipPlayer].ipProvince))
	{		
		Tutorial.UnitPrice=1;
		txtTutorial.Activate((pFirstTutText+93)->msg);
		objTutorial->iLayer=4;
		bTutHint=true;
		ScrollSound();
	};
	

	while (!((buttExit1.Check())||(buttExit2.Check())))
	{	
		int Click=0;
		if ((Tutorial.Map)&&(Tutorial.Hire==1))
		{
			buttExit1.Activate(-1);
			buttExit2.Activate(-1);
			Tutorial.Hire=2;
			txtTutorial.Activate((pFirstTutText+15)->msg);
			objTutorial->iLayer=4;
			bTutHint=true;
			ScrollSound();
		};

		if ((Tutorial.Mode)&&(Tutorial.Mercenary==0)&&(!bTutHint)&&(arPlayer[Hero->ipPlayer].arRecruit[11]>0))
		{
			Tutorial.Mercenary=1;
			txtTutorial.Activate((pFirstTutText+67)->msg);
			objTutorial->iLayer=4;
			bTutHint=true;
			ScrollSound();
		};

		if ((Tutorial.Map)&&(Tutorial.Garrison==0)&&(!bTutHint)&&(Tutorial.buttCastle))
		{
			bool bShow=false;
			for (int i=0; i<MAX_SQUAD; i++)
			{				
				if ((Hero->pSquad[i])&&(Hero->pSquad[i]->iLife<Hero->pSquad[i]->GetLife()/3))
				{bShow=true;};
			};
			if (bShow)
			{
				Tutorial.Garrison=1;
				txtTutorial.Activate((pFirstTutText+68)->msg);
				objTutorial->iLayer=4;
				bTutHint=true;
				ScrollSound();
			};
		};

		if ((Tutorial.Mode)&&(Tutorial.SquadMorale==0)&&(!bTutHint))
		{
			for (int i=1; i<MAX_SQUAD; i++)
			{
				if ((Hero->pSquad[i])&&(Hero->GetSquadMorale(i)!=0))
				{Tutorial.SquadMorale=1;};
			};
			if (Tutorial.SquadMorale==1)
			{		
				txtTutorial.Activate((pFirstTutText+114)->msg);
				objTutorial->iLayer=4;
				bTutHint=true;
				ScrollSound();
			};
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
				Tutorial.Hire++;
				switch (Tutorial.Hire)
				{
				case 3:
					txtTutorial.Activate((pFirstTutText+16)->msg);
					break;
				case 4:
					txtTutorial.Activate((pFirstTutText+17)->msg);
					break;				
				case 5:
				case 8:
					txtTutorial.Hide();
					objTutorial->iLayer=-1;
					bTutHint=false;
					break;
				case 7:
					txtTutorial.Activate((pFirstTutText+19)->msg);
					buttExit1.Activate(1);
					buttExit1.bBlink=true;
					buttExit2.Activate(1);
					buttExit2.bBlink=true;
					break;
				default:
					txtTutorial.Hide();
					objTutorial->iLayer=-1;
					bTutHint=false;
					break;

				};
			};
		};
		if (!Tutorial.Mode)
		{objTutorial->iLayer=-1;};
		if ((bBuy)&&(Tutorial.Map)&&((Tutorial.Hire==5)||(Tutorial.Hire==4)))
		{
			Tutorial.Hire++;
			txtTutorial.Activate((pFirstTutText+18)->msg);
			objTutorial->iLayer=4;
			bTutHint=true;
			ScrollSound();
		};
		if ((!bTutHint)||(mskTutorial.flag<2))
		{
#ifdef YUPLAY
		if ((iYuPlay))
		{
			if (buttYuPlay.Check())
			{
			if (InstallYuPlay())
			{
				objGUI=ShowActiveObjects(objGUI,10,false);
				WindowLottery(2);
				if (ipLottPrize>0)
				{					
					if ((Herr[0]->GetUnitSlot(pLottUnit->GetRank())>0)||(Herr[1]->GetUnitSlot(pLottUnit->GetRank())>0))
					{
						if (SelectedUnit)
						{
							Herr[SelectedHero]->PlaceUnit(SelectedUnit,SelectedSlot);
							blUnit[SelectedHero].Show();
							blInfo.Show();
							SelectedUnit=NULL;
						};						
						if ((SelectedItem)&&(SelectedSlot>0))
						{
							Hero->arInventory[SelectedSlot][0]=SelectedItem;
							Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
							SelectedItem=0;
							blItem.Show();
						};
						SelectedUnit=pLottUnit;						
						SelectedSlot=Herr[0]->GetFreeUnitSlot(pLottUnit->GetRank());
						SelectedHero=0;
						if (SelectedSlot<=0)
						{
							SelectedSlot=Herr[1]->GetFreeUnitSlot(pLottUnit->GetRank());
							SelectedHero=1;
						};
						blStat.Show();
						blInfo.Show();
						rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
						draw_sprite(bmpMouse,SelectedUnit->pType()->Pic,0,0);
						draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
						set_mouse_sprite(bmpMouse);
						buttTrash1->Activate(1);
						buttTrash2->Activate(1);
					}
					else
					{
						int Pay=pLottUnit->GetRank()*(pLottUnit->GetGoldPrice()+pLottUnit->GetGemPrice())+Random(pLottUnit->GetGoldPrice());
						char Txt[500];
						sprintf(Txt,(pFirstDialog+1249)->sText,Pay);
						(pFirstDialog+1249)->Show(1,1,0,0,NULL,Txt);
						arPlayer[Hero->ipPlayer].Gold+=Pay;		
						delete pLottUnit;
						pLottUnit=NULL;
						blResource.Show();
					}						
				};
			}
			};
		};
#endif
		Click=blRecruit.Check();
		if (Click>0)
		{		
			if (SelectedItem)
			{
				Hero->arInventory[SelectedSlot][0]=SelectedItem;
				Hero->arInventory[SelectedSlot][1]=SelectedItemDur;	
				SelectedItem=0;
				blItem.Show();
				blInfo.Show();
			};
			if (SelectedUnit)
			{
				if (SelectedSlot>0)
				{
					Herr[SelectedHero]->PlaceUnit(SelectedUnit,SelectedSlot);
					blUnit[SelectedHero].Show();
					blInfo.Show();
				}
				else
				{
					delete SelectedUnit;
				};				
			};
			if (Click>1000)
			{
				Click-=1000;
				bMerc=true;
				SelectedUnit=new dcUnit(Click,Random(GetUnitExp(Click,3*arPlayer[Hero->ipPlayer].GetBldParam1(6))));
				SelectedUnit->arMedal[0]=6;
			}
			else
			{
				bMerc=false;
				SelectedUnit=new dcUnit(Click,arPlayer[Hero->ipPlayer].GetBldParam1(20));
			};			
			SelectedSlot=-100;
			rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
			draw_sprite(bmpMouse,SelectedUnit->pType()->Pic,0,0);
			draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
			set_mouse_sprite(bmpMouse);
			buttTrash1->Activate(1);
			buttTrash2->Activate(1);

		};		

		//Манипулирование предметами
		//**************************
		Click=0;
		Click=blItem.Check(0,NULL,SelectedItem);
		if (Click>=0)
		{
	//Клик на кукле героя		
			if (Click==0)
			{
			Click=(pFirstItem+SelectedItem)->ipSlot;
			if ((Hero->CanUse(SelectedItem))&&(Click>0))
			{
				switch (Click)
				{
				case 10:
					if ((Hero->arInventory[11][0]==0)&&(Hero->arInventory[10][0]!=0))
					{Click=11;};
					break;
				case 11:
					Click=12;
					if (Hero->arInventory[12][0]==0)
					{break;}				
					if (Hero->arInventory[13][0]==0)
					{
						Click=13;
						break;
					};
					if (Hero->arInventory[14][0]==0)
					{
						Click=14;
						break;
					};
					if (Hero->arInventory[15][0]==0)
					{
						Click=15;						
					};
					break;
				case 100:
					Click=1;
					break;
				};
			}
			else
			{
				Click=16;
				for (int i=16; i<INVENTORY_SIZE+1; i++)
				{
					if (Hero->arInventory[i][0]==0)
					{
						Click=i;
						break;
					};
				};
			};
			};


			if (SelectedItem)
			{				
	//Попытка поместить предмет в...
				if (((SlotNum(Click)==(pFirstItem+SelectedItem)->ipSlot)&&(Hero->CanUse(SelectedItem)))
					&&((SlotNum(Click)!=2)||(Hero->arInventory[1][0]==0)||((pFirstItem+Hero->arInventory[1][0])->ipSlot!=100)||(GetFreeSlot(Hero,0)))
					||(((pFirstItem+SelectedItem)->ipSlot==100)&&(SlotNum(Click)==1)&&(Hero->CanUse(SelectedItem))&&
					((Hero->arInventory[Click][0]==0)||(Hero->arInventory[2][0]==0)||(GetFreeSlot(Hero,SelectedSlot))))
					||(Click>15))
				{
					if (Hero->arInventory[Click][0]>0)
					{
	//Занятый слот
						if (((SlotNum(SelectedSlot)==(pFirstItem+Hero->arInventory[Click][0])->ipSlot)&&
							((Hero->CanUse(Hero->arInventory[Click][0])))||(GetFreeSlot(Hero,0)))
							||(((pFirstItem+Hero->arInventory[Click][0])->ipSlot==100)&&(SlotNum(SelectedSlot)==1)&&
							((Hero->arInventory[2][0]==0)||(GetFreeSlot(Hero,SelectedSlot))))
							||(SelectedSlot>15))
						{
							if (((!Hero->CanUse(Hero->arInventory[Click][0]))||
								(SlotNum(SelectedSlot)!=(pFirstItem+Hero->arInventory[Click][0])->ipSlot))&&(SelectedSlot<16))
							//Перемещение предмета в инвентарь, если отдающий герой не может его использовать
							{
								int Free=GetFreeSlot(Hero,0);
								Hero->arInventory[Free][0]=Hero->arInventory[Click][0];
								Hero->arInventory[Free][1]=Hero->arInventory[Click][1];								
							}
							else
							{
								Hero->arInventory[SelectedSlot][0]=Hero->arInventory[Click][0];
								Hero->arInventory[SelectedSlot][1]=Hero->arInventory[Click][1];
							};
							//Перемещение щита в инвентарь, если отдающий герой получил двуручный предмет
							if (((pFirstItem+Hero->arInventory[Click][0])->ipSlot==100)&&(Hero->arInventory[2][0]>0)
								&&Hero->CanUse(Hero->arInventory[Click][0])&&(SelectedSlot==1))
							{
								int Free=GetFreeSlot(Hero,SelectedSlot);
								Hero->arInventory[Free][0]=Hero->arInventory[2][0];
								Hero->arInventory[Free][1]=Hero->arInventory[2][1];
								Hero->arInventory[2][1]=0;
								Hero->arInventory[2][0]=0;
							};
							Hero->arInventory[Click][0]=SelectedItem;
							Hero->arInventory[Click][1]=SelectedItemDur;
							//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
							if ((Click==1)&&((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0))
							{
								int Free=GetFreeSlot(Hero,SelectedSlot);
								Hero->arInventory[Free][0]=Hero->arInventory[2][0];
								Hero->arInventory[Free][1]=Hero->arInventory[2][1];
								Hero->arInventory[2][1]=0;
								Hero->arInventory[2][0]=0;
							};
							//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
							if ((Click==2)&&(Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100))
							{
								int Free=GetFreeSlot(Hero,0);
								Hero->arInventory[Free][0]=Hero->arInventory[1][0];
								Hero->arInventory[Free][1]=Hero->arInventory[1][1];
								Hero->arInventory[1][1]=0;
								Hero->arInventory[1][0]=0;
							};
							if (SelectedUnit)
							{
								delete SelectedUnit;						
								SelectedUnit=NULL;
							};
							SelectedItem=0;
							SelectedSlot=0;
							blItem.Show();							
							blStat.Show();
							blInfo.Show();
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash1->Activate(0);
							buttTrash2->Activate(0);
						};
					}
					else
					{	
	//Свободный слот
						Hero->arInventory[Click][0]=SelectedItem;
						Hero->arInventory[Click][1]=SelectedItemDur;	
						//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
						if (((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0)&&(Click==1))
						{
							int Free=GetFreeSlot(Hero,0);
							Hero->arInventory[Free][0]=Hero->arInventory[2][0];
							Hero->arInventory[Free][1]=Hero->arInventory[2][1];
							Hero->arInventory[2][1]=0;
							Hero->arInventory[2][0]=0;
						};
						//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
						if ((Click==2)&&(Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100))
						{
							int Free=GetFreeSlot(Hero,0);
							Hero->arInventory[Free][0]=Hero->arInventory[1][0];
							Hero->arInventory[Free][1]=Hero->arInventory[1][1];
							Hero->arInventory[1][1]=0;
							Hero->arInventory[1][0]=0;
						};
						if (SelectedUnit)
						{
							delete SelectedUnit;						
							SelectedUnit=NULL;
						};
						SelectedItem=0;
						SelectedSlot=0;
						blItem.Show();
						blStat.Show();
						blInfo.Show();
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash1->Activate(0);
						buttTrash2->Activate(0);
					};
				};
			}
			else
			{
	//Взятие предмета
				if (Hero->arInventory[Click][0]>0)
				{
					if (SelectedUnit)
					{
						if (SelectedSlot>=0)
						{
							Herr[SelectedHero]->PlaceUnit(SelectedUnit,SelectedSlot);
							blUnit[SelectedHero].Show();							
						}
						else
						{
							delete SelectedUnit;
						};
						SelectedUnit=NULL;
					};
					SelectedItem=Hero->arInventory[Click][0];
					SelectedItemDur=Hero->arInventory[Click][1];
					SelectedSlot=Click;					
					Hero->arInventory[Click][0]=0;
					Hero->arInventory[Click][1]=0;
					blItem.Show();
					blStat.Show();
					blInfo.Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
					if (arPlayer[Hero->ipPlayer].GetBldParam2(31,SelectedItem)>0)
					{
						int Monster=1;
						for (int i=0; i<MAX_ITEM_EFFECT; i++)
						{
							if ((pFirstItem+SelectedItem)->arEffect[i][0]==84)
							{Monster=(pFirstItem+SelectedItem)->arEffect[i][1];};
						};
						SelectedUnit=new dcUnit(Monster,0);
						draw_sprite(bmpMouse,(pFirstUnitType+Monster)->Pic,0,0);						
					};					
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash1->Activate(1);
					buttTrash2->Activate(1);
				};
			};
			
		};

		for (int A=0; A<2; A++)
		{		
	//Покупка юнитов и получение из предметов
	//***************************************
		Click=0;
		Click=blUnit[A].Check(SelectedUnit);
		if (Click>0)
		{
			if (SelectedUnit)
			{				
				if ((SelectedSlot<0)||(SelectedItem>0))
				{
					if ((!Herr[A]->pSquad[Click])&&(SlotLvl(Click)>=SelectedUnit->pType()->iLevel))
					{						
						Herr[A]->PlaceUnit(SelectedUnit,Click);					
						clUnitType Rookie=*(SelectedUnit->pType());
						bBuy=true;
						bool bSkip=true;
						if (SelectedSlot<0)
						{
							CoinSound();
						//	play_sample(wavCoins,iSoundVolume/2,128,1000,0);
							if (bMerc)
							{
								arPlayer[Hero->ipPlayer].Gold-=arPlayer[Hero->ipPlayer].GetUnitGoldPrice(SelectedUnit->GetUnitType(),Hero->ipProvince)*2;
								arPlayer[Hero->ipPlayer].Gem-=arPlayer[Hero->ipPlayer].GetUnitGemPrice(SelectedUnit->GetUnitType(),Hero->ipProvince)*2;
								arPlayer[Hero->ipPlayer].arRecruit[11]=0;
								blRecruit.Recalc();
							}
							else
							{						
								arPlayer[Hero->ipPlayer].Gold-=arPlayer[Hero->ipPlayer].GetUnitGoldPrice(SelectedUnit->GetUnitType(),Hero->ipProvince);
								arPlayer[Hero->ipPlayer].Gem-=arPlayer[Hero->ipPlayer].GetUnitGemPrice(SelectedUnit->GetUnitType(),Hero->ipProvince);
								if ((key[KEY_LCONTROL])||(key[KEY_RCONTROL]))
								{bSkip=false;};
							};
							for (int i=0; i<MAX_UNIT_RES; i++)
							{
								if (Rookie.arRes[i]>0)
								{
									if (arPlayer[Hero->ipPlayer].GetResource(Rookie.arRes[i])==0)
									{								
										CurGame.ResPrices[Rookie.arRes[i]]+=CurGame.ResBuy;
									};
								};
							};
							if ((arPlayer[Hero->ipPlayer].Gold<arPlayer[Hero->ipPlayer].GetUnitGoldPrice(SelectedUnit->GetUnitType(),Hero->ipProvince))||((arPlayer[Hero->ipPlayer].GetUnitGemPrice(SelectedUnit->GetUnitType(),Hero->ipProvince)>0)&&
								(arPlayer[Hero->ipPlayer].Gem<arPlayer[Hero->ipPlayer].GetUnitGemPrice(SelectedUnit->GetUnitType(),Hero->ipProvince))))
							{bSkip=true;};
						}
						else
						{
							SelectedItem=0;
						};
						blResource.Show();						
						blUnit[A].Show();
						blRecruit.Show();
						blInfo.Show();
						if (bSkip)
						{
							SelectedUnit=NULL;
							SelectedSlot=0;
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash1->Activate(0);
							buttTrash2->Activate(0);
						}
						else
						{SelectedUnit=new dcUnit(SelectedUnit->GetUnitType(),arPlayer[Hero->ipPlayer].GetBldParam1(20));};
					};
				}
				else

			//Обмен юнитами
			//*************
				{
			//Попытка поместить юнита в...
				if (SlotLvl(Click)>=SelectedUnit->pType()->iLevel)					
				{
					if (Herr[A]->pSquad[Click])
					{
	//Занятый слот
						if (GetFreeUnitSlot(Herr[SelectedHero],Herr[A]->pSquad[Click]->pType()->iLevel))
						{
							if (SlotLvl(SelectedSlot)<Herr[A]->pSquad[Click]->pType()->iLevel)
							//Перемещение юнита в свободный слот нужного уровня передающему герою
							{
								int Free=GetFreeUnitSlot(Herr[SelectedHero],Herr[A]->pSquad[Click]->pType()->iLevel);
							//	Herr[SelectedHero]->pSquad[Free]=Herr[A]->pSquad[Click];
								Herr[SelectedHero]->PlaceUnit(Herr[A]->pSquad[Click], Free);
							}
							else
							{
							//	Herr[SelectedHero]->pSquad[SelectedSlot]=Herr[A]->pSquad[Click];
								Herr[SelectedHero]->PlaceUnit(Herr[A]->pSquad[Click], SelectedSlot);
							};
							Herr[A]->pSquad[Click]=NULL;
							Herr[A]->PlaceUnit(SelectedUnit,Click);
							//Herr[A]->pSquad[Click]=SelectedUnit;							
							SelectedUnit=NULL;
							SelectedSlot=0;
							blUnit[A].Show();
							blUnit[SelectedHero].Show();
							blInfo.Show();
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash1->Activate(0);
							buttTrash2->Activate(0);
						};
					}
					else
					{	
	//Свободный слот						
						Herr[A]->pSquad[Click]=NULL;
						Herr[A]->PlaceUnit(SelectedUnit,Click);
					//	Herr[A]->pSquad[Click]=SelectedUnit;							
						SelectedUnit=NULL;
						SelectedSlot=0;
						blUnit[A].Show();
						blUnit[SelectedHero].Show();
						blInfo.Show();
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash1->Activate(0);
						buttTrash2->Activate(0);
					};
				};
				};
			}
			else
			{
	//Взятие юнита
				if (Herr[A]->pSquad[Click])
				{
					if (SelectedItem)
					{
						Hero->arInventory[SelectedSlot][0]=SelectedItem;
						Hero->arInventory[SelectedSlot][1]=SelectedItemDur;	
						SelectedItem=0;
						blItem.Show();
					};
					SelectedUnit=Herr[A]->pSquad[Click];					
					SelectedSlot=Click;
					SelectedHero=A;
					Herr[A]->pSquad[Click]=NULL;					
					blUnit[A].Show();
					blInfo.Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,SelectedUnit->pType()->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash1->Activate(1);
					buttTrash2->Activate(1);
				};
			};
		};
		};
		if ((buttTrash1->Check())||(buttTrash2->Check()))
		{
			if (SelectedUnit)
			{
				if ((SelectedSlot>=0)&&(SelectedItem==0))
				{
					int WillDel=1;
					if (arHumans[arPlayer[Hero->ipPlayer].iColor].arMess[4])
					{WillDel=(pFirstDialog+1461)->Show(1,1,0,0,NULL,NULL);}
					if (WillDel==1)
					{
					if ((arPlayer[Hero->ipPlayer].GetBldParam1(24)>0)&&(SelectedUnit->arMedal[0]!=6))
					{
						int Price=(SelectedUnit->GetGoldPrice()+SelectedUnit->GetGemPrice()*2)*(SelectedUnit->iLevel+1)/10;
						if (Price<1)
						{Price=1;};
						arPlayer[Hero->ipPlayer].Gold+=Price;
						if (arHumans[arPlayer[Hero->ipPlayer].iColor].arMess[3])
						{
							char Txt[200];
							sprintf(Txt,(pFirstDialog+1090)->sText,Price);
							(pFirstDialog+1090)->Show(1,1,0,0,NULL,Txt);
						};
						blResource.Show();
					};
					SelectedUnit->Dismiss();
					SelectedSlot=0;
					buttTrash1->Activate(0);
					buttTrash2->Activate(0);
					set_mouse_sprite(bmpMousePointer[0]);
					SelectedUnit=NULL;
					};
				}
				else
				{
					delete SelectedUnit;
					SelectedSlot=0;
					buttTrash1->Activate(0);
					buttTrash2->Activate(0);
					set_mouse_sprite(bmpMousePointer[0]);
					SelectedUnit=NULL;
				}				
			};
			if (SelectedItem>0)
			{
				int WillDel=1;
				if (arHumans[arPlayer[Hero->ipPlayer].iColor].arMess[4])
				{WillDel=(pFirstDialog+1462)->Show(1,1,0,0,NULL,NULL);}
				if (WillDel==1)
				{
					SelectedItem=0;
					SelectedSlot=0;
					buttTrash1->Activate(0);
					buttTrash2->Activate(0);
					set_mouse_sprite(bmpMousePointer[0]);
				};
			};
		};
		blInfo.Check();
		blStat.Check();
		if (blResource.Check(objGUI))
		{blRecruit.Show();};
		/*if (arPlayer[Hero->ipPlayer].GetBldParam1(34)>0)
		{
			int Check=mskAlchemy.Check();
			if ((Check==30)||(Check==40))
			{
				objGUI=ShowActiveObjects(objGUI,10,false);
				WindowAlchemy(Hero->ipPlayer);
				blResource.Show();
			};
			
		};*/
		}; //tutorial mask
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,10,true);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	if (SelectedItem)
	{
		Hero->arInventory[SelectedSlot][0]=SelectedItem;
		Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
		SelectedSlot=-1;
	};
	if (SelectedUnit)
	{
		if (SelectedSlot<0)
		{
			delete SelectedUnit;
		}
		else
		{
			Herr[SelectedHero]->pSquad[SelectedSlot]=SelectedUnit;
		};
	};
	set_mouse_sprite(bmpMousePointer[0]);
	destroy_bitmap(bmpMouse);
	delete buttTrash1;
	delete buttTrash2;	
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	bHint=false;
	HintType=0;
	iAntisleep=iAntisleepDelay;
};


//***************************************************
//*** Экран героя в библиотеке **********************
//***************************************************
void ScreenHeroLibrary (dcHero * Hero)
{
	iAntisleep=iAntisleepDelay;
	GUI_InfoBlock blInfo;
	GUI_ItemBlock blItem;
	GUI_StatBlock blStat;
	GUI_SpellBlock blSpell;
	GUI_LibraryBlock blLibrary;
	
	
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpGUI_SpellTrade);
	objGUI=new G_Object(objGUI,512,0,1,bmpGUI_Library);
	BITMAP * bmpMouse = create_bitmap(150,130);	
	int SelectedSlot=0;
	int SelectedItem=0, SelectedItemDur, iScroll=0;
	int SelectedSpell=0;
	
	objGUI=blItem.Init(Hero,207,14,objGUI);	
	objGUI=blStat.Init(Hero,8,278,objGUI);	
	objGUI=blInfo.Init(Hero,0,0,objGUI);
	objGUI=blSpell.Init(Hero, 34, 520, objGUI);
	objGUI=blLibrary.Init(&arPlayer[Hero->ipPlayer],592,101,objGUI);

	blInfo.Show();

	UI_Button1 buttYuPlay;
	UI_Button1 * buttTrash1=new UI_Button1();
#ifdef YUPLAY
	if (iYuPlay)
	{
		objGUI=buttTrash1->Init(367,534,1,3,bmpButton[229],bmpButton[230],bmpButton[231],bmpButton[232],objGUI);
		objGUI=buttYuPlay.Init(288,534,1,3,bmpYuPlay[0],bmpYuPlay[1],bmpYuPlay[2],bmpYuPlay[3],objGUI);
		if ((!Tutorial.Map)&&(CurGame.iGameType<3))
		{buttYuPlay.Activate(1);}
		else
		{buttYuPlay.Activate(0);};
		buttYuPlay.iHint=513;
	}
	else
#endif
	{objGUI=buttTrash1->Init(314,534,1,3,bmpButton[229],bmpButton[230],bmpButton[231],bmpButton[232],objGUI);};
	
	buttTrash1->Activate(0);
	buttTrash1->ipHotkey=KEY_DEL;
	buttTrash1->wavSound=wavDelete;

	
	UI_Button1 buttExit1;
	objGUI=buttExit1.Init(412,634,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit1.Activate(1);
	buttExit1.ipHotkey=KEY_ESC;
	buttExit1.iHint=135;

	UI_Button1 buttExit2;
	objGUI=buttExit2.Init(512+224,689,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit2.Activate(1);
	buttExit2.iHint=135;

	objGUI=new G_Object(objGUI,20,20,-1,bmpGUI_Hint);
	G_Object *objTutorial=objGUI;
	UI_Mask mskTutorial;
	objGUI=mskTutorial.Init(20,20,1,bmpGUI_Hint,bmpEmpty,objGUI,-1);
	UI_TextBox txtTutorial;
	objGUI=txtTutorial.Init(40,50,297,200,5,COLOR_1,Font2,objGUI);
	txtTutorial.bShadow=false;
	txtTutorial.Hide();
	bool bTutHint=false;
	bool bBuy=false;

	set_mouse_sprite(bmpMousePointer[0]);

	if ((Tutorial.Mode)&&(Tutorial.SpellRank==0))
	{
		bool bShow=false;
		if (arPlayer[Hero->ipPlayer].arLibrary[15]>0)
		{bShow=true;};
		for (int i=0; i<10; i++)
		{
			if ((Hero->arInventory[16+i][0]>0)&&((pFirstItem+Hero->arInventory[16+i][0])->ipClass==10)&&((pFirstSpell+(pFirstItem+Hero->arInventory[16+i][0])->arEffect[0][1])->iLevel>1))
			{bShow=true;};
		};
		if (bShow)
		{
			txtTutorial.Activate((pFirstTutText+85)->msg);
			objTutorial->iLayer=4;
			bTutHint=true;
			ScrollSound();
			Tutorial.SpellRank=1;
		};
	};

	while (!((buttExit1.Check())||(buttExit2.Check())))
	{	
		int Click=0;

		if ((Tutorial.Map)&&(Tutorial.Library==1))
		{
			buttExit1.Activate(-1);
			buttExit2.Activate(-1);
			Tutorial.Library=2;
			txtTutorial.Activate((pFirstTutText+21)->msg);
			objTutorial->iLayer=4;
			bTutHint=true;
			ScrollSound();
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
				Tutorial.Library++;
				switch (Tutorial.Library)
				{
				case 3:
					txtTutorial.Activate((pFirstTutText+22)->msg);
					break;				
				case 4:
				case 7:
					txtTutorial.Hide();
					objTutorial->iLayer=-1;
					bTutHint=false;
					break;
				case 6:
					txtTutorial.Activate((pFirstTutText+24)->msg);
					buttExit1.Activate(1);
					buttExit1.bBlink=true;
					buttExit2.Activate(1);
					buttExit2.bBlink=true;
					break;
				default:
					txtTutorial.Hide();
					objTutorial->iLayer=-1;
					bTutHint=false;
					break;
				};
			};
		};
		if (!Tutorial.Mode)
		{objTutorial->iLayer=-1;};
		if ((bBuy)&&(Tutorial.Map)&&((Tutorial.Library==3)||(Tutorial.Library==4)))
		{
			Tutorial.Library++;
			txtTutorial.Activate((pFirstTutText+23)->msg);
			objTutorial->iLayer=4;
			bTutHint=true;
			ScrollSound();
		};
		if ((!bTutHint)||(mskTutorial.flag<2))
		{
#ifdef YUPLAY
		if ((iYuPlay))
		{
			if (buttYuPlay.Check())
			{
			if (InstallYuPlay())
			{
				objGUI=ShowActiveObjects(objGUI,10,false);
				WindowLottery(3);
				if (ipLottPrize>0)
				{					
					if (Hero->GetSpellSlot((pFirstSpell+ipLottPrize)->iLevel)>0)
					{						
					if ((SelectedSpell)&&(SelectedSlot>0))
					{
						Hero->arSpell[SelectedSlot]=SelectedSpell;		
						SelectedSpell=0;
						blSpell.Show();
					};
					if ((SelectedItem)&&(SelectedSlot>0))
					{
						Hero->arInventory[SelectedSlot][0]=SelectedItem;
						Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
						SelectedItem=0;
						blItem.Show();
					};
					SelectedSpell=ipLottPrize;						
					SelectedSlot=0;
					blStat.Show();
					blInfo.Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstSpell+SelectedSpell)->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash1->Activate(1);						
					}
					else
					{
						int Pay=(pFirstSpell+ipLottPrize)->iLevel*(pFirstSpell+ipLottPrize)->iLevel*((pFirstSpell+ipLottPrize)->iItemLevel+1)*4+Random((pFirstSpell+ipLottPrize)->iLevel*(pFirstSpell+ipLottPrize)->iLevel*((pFirstSpell+ipLottPrize)->iItemLevel+1)*2);
						char Txt[500];
						sprintf(Txt,(pFirstDialog+1250)->sText,Pay);
						(pFirstDialog+1250)->Show(1,1,0,0,NULL,Txt);
						arPlayer[Hero->ipPlayer].Gem+=Pay;							
					}
				};
			}
			};
		};
#endif
		
	//Взятие заклинания в библиотеке
	//******************************
		Click=blLibrary.Check();
		/*if ((Tutorial.Map)&&(Tutorial.Hire==1))
		{
			buttExit1.Activate(-1);
			buttExit2.Activate(-1);
			Tutorial.Hire=2;
			txtTutorial.Activate((pFirstTutText+15)->msg);
			objTutorial->iLayer=4;
			bTutHint=true;
		};*/
		
		if (Click>0)
		{
			if (SelectedItem)
			{
				Hero->arInventory[SelectedSlot][0]=SelectedItem;
				Hero->arInventory[SelectedSlot][1]=SelectedItemDur;	
				SelectedItem=0;
				iScroll=0;
				blItem.Show();
			};
			if ((SelectedSpell)&&(Hero->arSpell[SelectedSlot]==0))
			{
				if (SelectedSlot>=0)
				{
					Hero->arSpell[SelectedSlot]=SelectedSpell;
					blSpell.Show();
				};
				SelectedSpell=0;				
			};
			SelectedSpell=Click;					
			SelectedSlot=-100;					
			rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
			draw_sprite(bmpMouse,(pFirstSpell+SelectedSpell)->Pic,0,0);
			draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
			set_mouse_sprite(bmpMouse);
			buttTrash1->Activate(1);		
		};
			

//Манипулирование заклинаниями
//****************************
		Click=0;
		if (iScroll>0)
		{Click=blSpell.Check(iScroll);}
		else
		{Click=blSpell.Check(SelectedSpell);};
		if (Click>0)
		{
			if ((SelectedSpell)||(iScroll>0))
			{
				if (iScroll>0)
				{
				//Свиток с заклинанием
					if (SpellSlotLvl(Click)>=(pFirstSpell+iScroll)->iLevel)					
					{
						if (Hero->arSpell[Click]>0)
						{							
							SelectedSpell=Hero->arSpell[Click];
							Hero->arSpell[Click]=iScroll;
							rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
							draw_sprite(bmpMouse,(pFirstSpell+SelectedSpell)->Pic,0,0);
							draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
							SelectedSlot=-100;
							blSpell.Show();
						}
						else
						{
							Hero->arSpell[Click]=iScroll;													
							SelectedSpell=0;							
							SelectedSlot=0;
							blSpell.Show();						
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash1->Activate(0);
						};
						SelectedItem=0;
						iScroll=0;
					};
				}
				else
				{
				if (SelectedSlot<0)
				{					
				//Заклинание из библиотеки
					if (SpellSlotLvl(Click)>=(pFirstSpell+SelectedSpell)->iLevel)					
					{
						bBuy=true;
						if (Hero->arSpell[Click]>0)
						{
							int TempSpell=SelectedSpell;
							SelectedSpell=Hero->arSpell[Click];
							Hero->arSpell[Click]=TempSpell;
							rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
							draw_sprite(bmpMouse,(pFirstSpell+SelectedSpell)->Pic,0,0);
							draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
							SelectedSlot=-100;
							blSpell.Show();
						}
						else
						{
							Hero->arSpell[Click]=SelectedSpell;
							if ((!key[KEY_LCONTROL])&&(!key[KEY_RCONTROL]))
							{
								SelectedSpell=0;
								SelectedSlot=0;								
								set_mouse_sprite(bmpMousePointer[0]);
								buttTrash1->Activate(0);
							};
							blSpell.Show();
						};
					};
				}
				else
				{

			//Попытка поместить заклинание в...
				if (SpellSlotLvl(Click)>=(pFirstSpell+SelectedSpell)->iLevel)					
				{
					if (Hero->arSpell[Click]>0)
					{
			//Занятый слот
						if (GetFreeSpellSlot(Hero,(pFirstSpell+Hero->arSpell[Click])->iLevel))
						{
							if (SpellSlotLvl(SelectedSlot)<(pFirstSpell+Hero->arSpell[Click])->iLevel)
							{
								int Free=GetFreeSpellSlot(Hero,(pFirstSpell+Hero->arSpell[Click])->iLevel);							//	
								Hero->arSpell[Free]=Hero->arSpell[Click];
							}
							else
							{							
								Hero->arSpell[SelectedSlot]=Hero->arSpell[Click];
							};
							Hero->arSpell[Click]=SelectedSpell;									
							SelectedSpell=0;
							SelectedSlot=0;
							blSpell.Show();							
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash1->Activate(0);							
						}
						else
						{
							int Temp=Hero->arSpell[Click];
							Hero->arSpell[Click]=SelectedSpell;									
							SelectedSpell=Temp;
						//	SelectedSlot=Click;
							blSpell.Show();
							rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
							draw_sprite(bmpMouse,(pFirstSpell+SelectedSpell)->Pic,0,0);
							draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
							set_mouse_sprite(bmpMouse);
						};
					}
					else
					{	
			//Свободный слот						
						Hero->arSpell[Click]=SelectedSpell;													
						SelectedSpell=0;
						SelectedSlot=0;
						blSpell.Show();						
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash1->Activate(0);						
					};
				};
				};
				};
			}
			else
			{
	//Взятие заклинания
				if (Hero->arSpell[Click]>0)
				{
					if (SelectedItem)
					{
						Hero->arInventory[SelectedSlot][0]=SelectedItem;
						Hero->arInventory[SelectedSlot][1]=SelectedItemDur;	
						SelectedItem=0;
						iScroll=0;
						blItem.Show();
						blInfo.Show();
						blStat.Show();
					};					
					SelectedSpell=Hero->arSpell[Click];					
					SelectedSlot=Click;					
					Hero->arSpell[Click]=0;					
					blSpell.Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstSpell+SelectedSpell)->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash1->Activate(1);					
				};
			};
		};

//Манипулирование предметами
//**************************
		Click=0;
		Click=blItem.Check(0,NULL,SelectedItem);
		if (Click>=0)
		{
	//Клик на кукле героя		
			if (Click==0)
			{
			Click=(pFirstItem+SelectedItem)->ipSlot;
			if ((Hero->CanUse(SelectedItem))&&(Click>0))
			{
				switch (Click)
				{
				case 10:
					if ((Hero->arInventory[11][0]==0)&&(Hero->arInventory[10][0]!=0))
					{Click=11;};
					break;
				case 11:
					Click=12;
					if (Hero->arInventory[12][0]==0)
					{break;}				
					if (Hero->arInventory[13][0]==0)
					{
						Click=13;
						break;
					};
					if (Hero->arInventory[14][0]==0)
					{
						Click=14;
						break;
					};
					if (Hero->arInventory[15][0]==0)
					{
						Click=15;						
					};
					break;
				case 100:
					Click=1;
					break;
				};
			}
			else
			{
				Click=16;
				for (int i=16; i<INVENTORY_SIZE+1; i++)
				{
					if (Hero->arInventory[i][0]==0)
					{
						Click=i;
						break;
					};
				};
			};
			};


			if (SelectedItem)
			{				
	//Попытка поместить предмет в...
				if (((SlotNum(Click)==(pFirstItem+SelectedItem)->ipSlot)&&(Hero->CanUse(SelectedItem)))
					&&((SlotNum(Click)!=2)||(Hero->arInventory[1][0]==0)||((pFirstItem+Hero->arInventory[1][0])->ipSlot!=100)||(GetFreeSlot(Hero,0)))
					||(((pFirstItem+SelectedItem)->ipSlot==100)&&(SlotNum(Click)==1)&&(Hero->CanUse(SelectedItem))&&
					((Hero->arInventory[Click][0]==0)||(Hero->arInventory[2][0]==0)||(GetFreeSlot(Hero,SelectedSlot))))
					||(Click>15))
				{
					if (Hero->arInventory[Click][0]>0)
					{
	//Занятый слот
						if (((SlotNum(SelectedSlot)==(pFirstItem+Hero->arInventory[Click][0])->ipSlot)&&
							((Hero->CanUse(Hero->arInventory[Click][0])))||(GetFreeSlot(Hero,0)))
							||(((pFirstItem+Hero->arInventory[Click][0])->ipSlot==100)&&(SlotNum(SelectedSlot)==1)&&
							((Hero->arInventory[2][0]==0)||(GetFreeSlot(Hero,SelectedSlot))))
							||(SelectedSlot>15))
						{
							if (((!Hero->CanUse(Hero->arInventory[Click][0]))||
								(SlotNum(SelectedSlot)!=(pFirstItem+Hero->arInventory[Click][0])->ipSlot))&&(SelectedSlot<16))
							//Перемещение предмета в инвентарь, если отдающий герой не может его использовать
							{
								int Free=GetFreeSlot(Hero,0);
								Hero->arInventory[Free][0]=Hero->arInventory[Click][0];
								Hero->arInventory[Free][1]=Hero->arInventory[Click][1];								
							}
							else
							{
								Hero->arInventory[SelectedSlot][0]=Hero->arInventory[Click][0];
								Hero->arInventory[SelectedSlot][1]=Hero->arInventory[Click][1];
							};
							//Перемещение щита в инвентарь, если отдающий герой получил двуручный предмет
							if (((pFirstItem+Hero->arInventory[Click][0])->ipSlot==100)&&(Hero->arInventory[2][0]>0)
								&&Hero->CanUse(Hero->arInventory[Click][0])&&(SelectedSlot==1))
							{
								int Free=GetFreeSlot(Hero,SelectedSlot);
								Hero->arInventory[Free][0]=Hero->arInventory[2][0];
								Hero->arInventory[Free][1]=Hero->arInventory[2][1];
								Hero->arInventory[2][1]=0;
								Hero->arInventory[2][0]=0;
							};
							Hero->arInventory[Click][0]=SelectedItem;
							Hero->arInventory[Click][1]=SelectedItemDur;
							//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
							if ((Click==1)&&((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0))
							{
								int Free=GetFreeSlot(Hero,SelectedSlot);
								Hero->arInventory[Free][0]=Hero->arInventory[2][0];
								Hero->arInventory[Free][1]=Hero->arInventory[2][1];
								Hero->arInventory[2][1]=0;
								Hero->arInventory[2][0]=0;
							};
							//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
							if ((Click==2)&&(Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100))
							{
								int Free=GetFreeSlot(Hero,0);
								Hero->arInventory[Free][0]=Hero->arInventory[1][0];
								Hero->arInventory[Free][1]=Hero->arInventory[1][1];
								Hero->arInventory[1][1]=0;
								Hero->arInventory[1][0]=0;
							};
							SelectedItem=0;
							SelectedSlot=0;
							iScroll=0;
							blItem.Show();							
							blStat.Show();
							blInfo.Show();
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash1->Activate(0);						
						};
					}
					else
					{	
	//Свободный слот
						Hero->arInventory[Click][0]=SelectedItem;
						Hero->arInventory[Click][1]=SelectedItemDur;
						//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
						if (((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0)&&(Click==1))
						{
							int Free=GetFreeSlot(Hero,0);
							Hero->arInventory[Free][0]=Hero->arInventory[2][0];
							Hero->arInventory[Free][1]=Hero->arInventory[2][1];
							Hero->arInventory[2][1]=0;
							Hero->arInventory[2][0]=0;
						};
						//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
						if ((Click==2)&&(Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100))
						{
							int Free=GetFreeSlot(Hero,0);
							Hero->arInventory[Free][0]=Hero->arInventory[1][0];
							Hero->arInventory[Free][1]=Hero->arInventory[1][1];
							Hero->arInventory[1][1]=0;
							Hero->arInventory[1][0]=0;
						};
						iScroll=0;
						SelectedItem=0;
						SelectedSlot=0;
						blItem.Show();
						blStat.Show();
						blInfo.Show();
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash1->Activate(0);					
					};
				};
			}
			else
			{
	//Взятие предмета
				if (Hero->arInventory[Click][0]>0)
				{
					if (SelectedSpell)
					{
						if ((SelectedSlot>=0)&&(Hero->arSpell[SelectedSlot]==0))
						{
							Hero->arSpell[SelectedSlot]=SelectedSpell;
							blSpell.Show();
						};
						SelectedSpell=0;				
					};
					SelectedItem=Hero->arInventory[Click][0];
					SelectedItemDur=Hero->arInventory[Click][1];
					if ((pFirstItem+SelectedItem)->ipClass==10)
					{
						iScroll=(pFirstItem+SelectedItem)->arEffect[0][1];
					}
					else
					{
						iScroll=0;
					};
					SelectedSlot=Click;					
					Hero->arInventory[Click][0]=0;
					Hero->arInventory[Click][1]=0;
					blItem.Show();
					blStat.Show();
					blInfo.Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
					if (iScroll>0)
					{
						draw_sprite(bmpMouse,(pFirstSpell+iScroll)->Pic,10,10);
					};
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash1->Activate(1);					
				};
			};
			
		};
		
		if (buttTrash1->Check())
		{
			if (SelectedItem>0)
			{
				int WillDel=1;
				if (arHumans[arPlayer[Hero->ipPlayer].iColor].arMess[4])
				{WillDel=(pFirstDialog+1462)->Show(1,1,0,0,NULL,NULL);}
				if (WillDel==1)
				{
					SelectedItem=0;
					SelectedSpell=0;
					SelectedSlot=0;
					buttTrash1->Activate(0);
					set_mouse_sprite(bmpMousePointer[0]);
				};
			};
			if (SelectedSpell>0)
			{
				int WillDel=1;
				if (arHumans[arPlayer[Hero->ipPlayer].iColor].arMess[4])
				{
					WillDel=2;
					for (int i=0; i<LIBRARY_SIZE; i++)
					{
						if (arPlayer[Hero->ipPlayer].arLibrary[i]==SelectedSpell)
						{WillDel=1;};
					};
					if (WillDel==2)
					{WillDel=(pFirstDialog+1463)->Show(1,1,0,0,NULL,NULL);};
				}
				if (WillDel==1)
				{
					SelectedSpell=0;
					SelectedSlot=0;
					buttTrash1->Activate(0);
					set_mouse_sprite(bmpMousePointer[0]);
				};
			};			
		};

		blInfo.Check();
		blStat.Check();
		}; //Tutorial
		
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,10,true);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	if (SelectedItem)
	{
		Hero->arInventory[SelectedSlot][0]=SelectedItem;
		Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
	};
	if (SelectedSpell)
	{
		if ((SelectedSlot>=0)&&(Hero->arSpell[SelectedSlot]==0)&&(SpellSlotLvl(SelectedSlot)>=(pFirstSpell+SelectedSpell)->iLevel))
		{
			Hero->arSpell[SelectedSlot]=SelectedSpell;
			blSpell.Show();
		};
			SelectedSpell=0;				
	};
	set_mouse_sprite(bmpMousePointer[0]);
	destroy_bitmap(bmpMouse);
	delete buttTrash1;
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	bHint=false;
	HintType=0;
	iAntisleep=iAntisleepDelay;
};


//***************************************************
//*** Экран героя в магазине ************************
//***************************************************
void ScreenHeroShop (dcHero * Hero, dcShop * Shop)
{
	iAntisleep=iAntisleepDelay;
	int OldPrice=Shop->PriceModifier[Hero->ipPlayer];
	if ((Shop->PriceModifier[Hero->ipPlayer]>100)&&(Hero->CheckSkill(27)>0))
	{Shop->PriceModifier[Hero->ipPlayer]=Shop->PriceModifier[Hero->ipPlayer]*(10-Hero->CheckSkill(27))/10;};
	GUI_InfoBlock blInfo;
	GUI_ItemBlock blItem;
	GUI_StatBlock blStat;
	GUI_ShopBlock blShop;	
	GUI_TreasuryBlock blTreasury;
	GUI_ResourceBlock blResource;
	dcPlayer * PL=&arPlayer[Hero->ipPlayer];
	
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpGUI_ItemTrade);
	objGUI=new G_Object(objGUI,512,0,1,bmpGUI_Shop);
	BITMAP * bmpMouse = create_bitmap(150,130);	
	int SelectedSlot=0;
	int SelectedItem=0, SelectedItemDur;
	int ItemOwner;
	
	objGUI=blItem.Init(Hero,207,14,objGUI);	
	objGUI=blStat.Init(Hero,8,278,objGUI);	
	objGUI=blInfo.Init(Hero,0,0,objGUI);
	objGUI=blShop.Init(Hero,Shop,519,13,objGUI);
	objGUI=blResource.Init(PL,0,734,objGUI);
	objGUI=blTreasury.Init(PL,7,500,objGUI);

	/*UI_Mask1 mskAlchemy;
	mskAlchemy.Init(11,727,240,40,1);
	mskAlchemy.iHint=352;*/

	objGUI=new G_Object(objGUI,20,20,-1,bmpGUI_Hint);
	G_Object *objTutorial=objGUI;
	UI_Mask mskTutorial;
	objGUI=mskTutorial.Init(20,20,1,bmpGUI_Hint,bmpEmpty,objGUI,-1);
	UI_TextBox txtTutorial;
	objGUI=txtTutorial.Init(40,50,297,200,5,COLOR_1,Font2,objGUI);
	txtTutorial.bShadow=false;
	txtTutorial.Hide();
	bool bTutHint=false;
	bool bBuy=false;


//	blInfo.Show();

	UI_Button1 * buttTrash1=new UI_Button1();
	UI_Button1 buttYuPlay;
//	UI_Button1 * buttTrash=new UI_Button1();
#ifdef YUPLAY
	if (iYuPlay)
	{
		objGUI=buttTrash1->Init(78,420,1,3,bmpButton[229],bmpButton[230],bmpButton[231],bmpButton[232],objGUI);
		objGUI=buttYuPlay.Init(13,420,1,3,bmpYuPlay[0],bmpYuPlay[1],bmpYuPlay[2],bmpYuPlay[3],objGUI);
		if ((!Tutorial.Map)&&(CurGame.iGameType<3))
		{buttYuPlay.Activate(1);}
		else
		{buttYuPlay.Activate(0);};
		buttYuPlay.iHint=513;
	}
	else
#endif
	{objGUI=buttTrash1->Init(32,420,1,3,bmpGUI_ButtTrashNorm,bmpGUI_ButtTrashAct,bmpButton[231],bmpButton[232],objGUI);};

	buttTrash1->Activate(0);
	buttTrash1->ipHotkey=KEY_DEL;
	buttTrash1->wavSound=wavDelete;
	
	UI_Button1 buttExit1;
	objGUI=buttExit1.Init(934,687,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit1.Activate(1);
	buttExit1.ipHotkey=KEY_ESC;
	buttExit1.iHint=135;


	set_mouse_sprite(bmpMousePointer[0]);
	int Under1=0;

	while (!(buttExit1.Check()))
	{	
		int Click=0;
		if ((Tutorial.Map)&&(Tutorial.Trade==1))
		{
			buttExit1.Activate(-1);
			Tutorial.Trade=2;
			txtTutorial.Activate((pFirstTutText+5)->msg);
			objTutorial->iLayer=4;
			bTutHint=true;
			ScrollSound();
		};
		if ((Tutorial.Map)&&(Tutorial.Repair==1))
		{
			buttExit1.Activate(-1);
			Tutorial.Repair=2;
			txtTutorial.Activate((pFirstTutText+46)->msg);
			objTutorial->iLayer=4;
			bTutHint=true;
			ScrollSound();
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
				Tutorial.Trade++;
				switch (Tutorial.Trade)
				{
				case 3:
					txtTutorial.Activate((pFirstTutText+6)->msg);
					break;
				case 4:
					txtTutorial.Activate((pFirstTutText+7)->msg);
					break;
				case 5:
					txtTutorial.Activate((pFirstTutText+8)->msg);
					break;
				case 6:
				case 9:
					txtTutorial.Hide();
					objTutorial->iLayer=-1;
					bTutHint=false;
					break;
				case 8:
					txtTutorial.Activate((pFirstTutText+10)->msg);
					buttExit1.Activate(1);
					buttExit1.bBlink=true;
					break;
				};
				if (Tutorial.Repair>0)
				{
					Tutorial.Repair++;
					switch (Tutorial.Repair)
					{
					case 3:
						txtTutorial.Activate((pFirstTutText+47)->msg);
						blShop.buttRepair.bBlink=true;
						blShop.buttRepairAll.bBlink=true;
						objTutorial->iLayer=4;
						bTutHint=true;
						break;
					case 4:
						txtTutorial.Activate((pFirstTutText+48)->msg);
						blShop.buttRepair.bBlink=false;
						blShop.buttRepairAll.bBlink=false;
						objTutorial->iLayer=4;
						bTutHint=true;
						buttExit1.Activate(1);
						break;
					case 5:
						txtTutorial.Hide();
						objTutorial->iLayer=-1;
						bTutHint=false;
						break;
					};	
				};
			};
		};
		if (!Tutorial.Mode)
		{objTutorial->iLayer=-1;};
		if ((bBuy)&&(Tutorial.Map)&&((Tutorial.Trade==6)||(Tutorial.Trade==5)))
		{
			Tutorial.Trade++;
			txtTutorial.Activate((pFirstTutText+9)->msg);
			objTutorial->iLayer=4;
			bTutHint=true;
			ScrollSound();
		};

		if ((!bTutHint)||(mskTutorial.flag<2))
		{
#ifdef YUPLAY
		if ((iYuPlay))
		{
			if (buttYuPlay.Check())
			{
			if (InstallYuPlay())
			{
				objGUI=ShowActiveObjects(objGUI,10,false);
				WindowLottery(1);
				if (ipLottPrize>0)
				{
					if ((SelectedItem)&&(SelectedSlot>0))
					{
						if (ItemOwner==1)
						{
							Hero->arInventory[SelectedSlot][0]=SelectedItem;
							Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
							blItem.Show();
						};
						if (ItemOwner==2)
						{
							arPlayer[Hero->ipPlayer].arTreasury[SelectedSlot][0]=SelectedItem;
							arPlayer[Hero->ipPlayer].arTreasury[SelectedSlot][1]=SelectedItemDur;
							blTreasury.Show();
						};
					};					
					SelectedItem=ipLottPrize;
					SelectedItemDur=(pFirstItem+ipLottPrize)->iDurability;
					SelectedSlot=0;
					ItemOwner=1;
					for (int i=16; i<26; i++)
					{
						if (Hero->arInventory[i][0]==0)
						{
							SelectedSlot=i;
							break;
						};
					};
					if (SelectedSlot==0)
					{
						ItemOwner=2;
						for (int i=1; i<=TREASURY_SIZE; i++)
						{
							if (arPlayer[Hero->ipPlayer].arTreasury[i][0]==0)
							{
								SelectedSlot=i;
								break;
							};
						};
					};
					blStat.Show();
					blInfo.Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash1->Activate(1);					
				};
			}
			};
		};
#endif

//Манипулирование предметами
//**************************
		Click=0;
		Under1=Under;
		Under=-1;
		Click=blShop.Check(Hero);
	//Общий ремонт
		if (Click==10000)
		{
			PlayUnitSound(wavRepairAll);
		//	CoinSound();
		//	play_sample(wavCoins,iSoundVolume/2,128,1000,0);
			for (int i=1; i<=INVENTORY_SIZE; i++)
			{		
				if ((Hero->arInventory[i][0]>0)&&
				(PL->GetItemGoldRepPrice(Hero->arInventory[i][0],Hero->arInventory[i][1],Shop)))
				{
					PL->Gold-=PL->GetItemGoldRepPrice(Hero->arInventory[i][0],Hero->arInventory[i][1],Shop);
					PL->Gem-=PL->GetItemGemRepPrice(Hero->arInventory[i][0],Hero->arInventory[i][1],Shop);
					Hero->arInventory[i][1]=(pFirstItem+Hero->arInventory[i][0])->iDurability;
				};
			};
			Click=-1;
			blShop.Show();
			blItem.Show();
			blStat.Show();
			blInfo.Show();
			blResource.Show();
		};
		if ((SelectedItem)&&(blShop.buttRepair.bPress))
		{
			if (ItemOwner==1)
			{
				Hero->arInventory[SelectedSlot][0]=SelectedItem;
				Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
				blItem.Show();
				blInfo.Show();
				blStat.Show();
				SelectedItem=0;
				set_mouse_sprite(bmpMousePointer[0]);
			};
			if (ItemOwner==2)
			{
				PL->arTreasury[SelectedSlot][0]=SelectedItem;
				PL->arTreasury[SelectedSlot][1]=SelectedItemDur;
				blTreasury.Show();
				SelectedItem=0;
				set_mouse_sprite(bmpMousePointer[0]);
			};			
		};

		if ((SelectedItem==0)&&(Under>=0)&&(Shop->Items[Under][0]>0)&&(Under!=Under1))
		{
			blShop.Pick(2,Shop->Items[Under][0],Shop->Items[Under][1]);
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
			//	play_sample(wavCoins,iSoundVolume/2,128,1000,0);
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
				buttTrash1->Activate(0);
			}
			else
	//Взятие предмета в магазине
			{
				if ((Click<1000)&&(Shop->Items[Click][0]>0)&&(Shop->Items[Click][0]!=SelectedItem)&&
					(PL->Gold>=PL->GetItemGoldPrice(Shop->Items[Click][0],Shop))&&
					((PL->Gem>=PL->GetItemGemPrice(Shop->Items[Click][0],Shop))||(PL->GetItemGemPrice(Shop->Items[Click][0],Shop)==0)))
				{
					SelectedItem=Shop->Items[Click][0];
					SelectedItemDur=(pFirstItem+SelectedItem)->iDurability;
					SelectedSlot=Click;
					ItemOwner=3;
					blShop.Pick(2,SelectedItem,SelectedItemDur);
					blShop.Show();
					blResource.Show();
					blItem.Show();
					blStat.Show();
					blInfo.Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash1->Activate(1);
				}
				else
				{
					SelectedItem=0;
					SelectedSlot=0;					
					set_mouse_sprite(bmpMousePointer[0]);
					buttTrash1->Activate(0);
				};
			}
		}
		else
		{
			if ((Click<1000)&&(Shop->Items[Click][0]>0)&&
				(PL->Gold>=PL->GetItemGoldPrice(Shop->Items[Click][0],Shop))&&
				((PL->Gem>=PL->GetItemGemPrice(Shop->Items[Click][0],Shop))||(PL->GetItemGemPrice(Shop->Items[Click][0],Shop)==0)))
			{
				SelectedItem=Shop->Items[Click][0];
				SelectedItemDur=(pFirstItem+SelectedItem)->iDurability;
				SelectedSlot=Click;
				ItemOwner=3;
				blShop.Pick(2,SelectedItem,SelectedItemDur);
				blShop.Show();
				blItem.Show();
				blStat.Show();
				blInfo.Show();
				blResource.Show();
				rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
				draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
				draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
				set_mouse_sprite(bmpMouse);
				buttTrash1->Activate(1);
			};
		};
		};
	

		if (blShop.buttRepair.bPress)
		{
			Click=blTreasury.Check(Hero,14,Shop);
		}
		else
		{			
			Click=blTreasury.Check(Hero,12,NULL);
		};
		if ((SelectedItem==0)&&(Under>=0)&&(PL->arTreasury[Under][0]>0)&&(Under!=Under1))
		{
			if (blShop.buttRepair.bPress)
			{
				int Gold=PL->GetItemGoldRepPrice(PL->arTreasury[Under][0],PL->arTreasury[Under][1],Shop);
				int Gem=PL->GetItemGemRepPrice(PL->arTreasury[Under][0],PL->arTreasury[Under][1],Shop);
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
				if (ItemOwner==1)
				{
			//Предмет героя
					if (((SlotNum(SelectedSlot)==(pFirstItem+PL->arTreasury[Click][0])->ipSlot)&&
						((Hero->CanUse(PL->arTreasury[Click][0])))||(GetFreeSlot(Hero,0)))
						||(((pFirstItem+PL->arTreasury[Click][0])->ipSlot==100)&&(SlotNum(SelectedSlot)==1)&&
						((Hero->arInventory[2][0]==0)||(GetFreeSlot(Hero,SelectedSlot))))
						||(SelectedSlot>15))
					{
						if (((!Hero->CanUse(PL->arTreasury[Click][0]))||
							(SlotNum(SelectedSlot)!=(pFirstItem+PL->arTreasury[Click][0])->ipSlot))&&(SelectedSlot<16))
						//Перемещение предмета в инвентарь, если отдающий герой не может его использовать
						{
							int Free=GetFreeSlot(Hero,0);
							Hero->arInventory[Free][0]=PL->arTreasury[Click][0];
							Hero->arInventory[Free][1]=PL->arTreasury[Click][1];
						}
						else
						{
							Hero->arInventory[SelectedSlot][0]=PL->arTreasury[Click][0];
							Hero->arInventory[SelectedSlot][1]=PL->arTreasury[Click][1];
						};
						//Перемещение щита в инвентарь, если отдающий герой получил двуручный предмет
						if (((pFirstItem+PL->arTreasury[Click][0])->ipSlot==100)&&(Hero->arInventory[2][0]>0)
							&&Hero->CanUse(PL->arTreasury[Click][0])&&(SelectedSlot==1))
						{
							int Free=GetFreeSlot(Hero,SelectedSlot);
							Hero->arInventory[Free][0]=Hero->arInventory[2][0];;
							Hero->arInventory[Free][1]=Hero->arInventory[2][1];;
							Hero->arInventory[2][1]=0;
							Hero->arInventory[2][0]=0;
						};
						PL->arTreasury[Click][0]=SelectedItem;
						PL->arTreasury[Click][1]=SelectedItemDur;							
						SelectedItem=0;
						SelectedSlot=0;
						blShop.Pick(0,0,0);
						blItem.Show();							
						blStat.Show();
						blInfo.Show();
						blShop.Show();	
						blTreasury.Show();
						set_mouse_sprite(bmpMousePointer[0]);
						buttTrash1->Activate(0);						
					};
				};
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
					blItem.Show();
					blStat.Show();
					blInfo.Show();
					blShop.Show();	
					blTreasury.Show();
					set_mouse_sprite(bmpMousePointer[0]);
					buttTrash1->Activate(0);
				};
			}
			else
			{	
	//Свободный слот
				bool bSkip=true;
				if (ItemOwner==3)
				{
			//Покупка предмета
					CoinSound();
				//	play_sample(wavCoins,iSoundVolume/2,128,1000,0);
					bBuy=true;					
					if ((key[KEY_LCONTROL])||(key[KEY_RCONTROL]))
					{bSkip=false;};
					PL->Gold-=PL->GetItemGoldPrice(SelectedItem, Shop);
					PL->Gem-=PL->GetItemGemPrice(SelectedItem, Shop);
					for (int i=0; i<MAX_ITEM_RES; i++)
					{
						if (((pFirstItem+SelectedItem)->arRes[i]>0)&&(PL->GetResource((pFirstItem+SelectedItem)->arRes[i])==0))
						{
							CurGame.ResPrices[(pFirstItem+SelectedItem)->arRes[i]]+=CurGame.ResBuy;
						};
					};
					if (Shop->Items[SelectedSlot][1]>0)
					{
						Shop->Items[SelectedSlot][1]--;
						if (Shop->Items[SelectedSlot][1]==0)
						{
							Shop->Items[SelectedSlot][0]=0;
							bSkip=true;
						};
					};
					if ((PL->Gold<PL->GetItemGoldPrice(SelectedItem, Shop))||((PL->GetItemGemPrice(SelectedItem, Shop)>0)&&(PL->Gem<PL->GetItemGemPrice(SelectedItem, Shop))))
					{bSkip=true;};
				};
				PL->arTreasury[Click][0]=SelectedItem;
				PL->arTreasury[Click][1]=SelectedItemDur;
				blShop.Pick(0,0,0);
				blItem.Show();
				blStat.Show();
				blInfo.Show();
				blShop.Show();
				blResource.Show();
				blTreasury.Show();
				if (bSkip)
				{
					SelectedItem=0;
					SelectedSlot=0;
					set_mouse_sprite(bmpMousePointer[0]);
					buttTrash1->Activate(0);
				};
			};
		}
		else
		{
	
			if (PL->arTreasury[Click][0]>0)
			{
			if (blShop.buttRepair.bPress)
		//Ремонт предмета
			{
				int Gold=PL->GetItemGoldRepPrice(PL->arTreasury[Click][0],PL->arTreasury[Click][1],Shop);
				int Gem=PL->GetItemGemRepPrice(PL->arTreasury[Click][0],PL->arTreasury[Click][1],Shop);
				blShop.Pick(3,PL->arTreasury[Click][0],PL->arTreasury[Click][1]);
				if (((Gold>0)||(Gem>0))&&((PL->Gem>=Gem)||(Gem==0))&&(PL->Gold>=Gold))
				{
					PL->Gem-=Gem;
					PL->Gold-=Gold;
					PlayUnitSound(wavRepair);
					//play_sample(wavClick,iSoundVolume,128,1000,0);
					PL->arTreasury[Click][1]=(pFirstItem+PL->arTreasury[Click][0])->iDurability;					
					blShop.Pick(3,PL->arTreasury[Click][0],PL->arTreasury[Click][1]);
					blTreasury.Show();
					blShop.Show();
					blStat.Show();
					blInfo.Show();
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
				blItem.Show();
				blStat.Show();
				blInfo.Show();
				blTreasury.Show();
				rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
				draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
				draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
				set_mouse_sprite(bmpMouse);
				buttTrash1->Activate(1);					
			};
			};
		};	
		};		
		if (blShop.buttRepair.bPress)
		{
			Click=blItem.Check(14,Shop,SelectedItem);
		}
		else
		{			
			Click=blItem.Check(12,NULL,SelectedItem);
		};		
		if ((SelectedItem==0)&&(Under>=0)&&(Hero->arInventory[Under][0]>0)&&(Under!=Under1))
		{
			if (blShop.buttRepair.bPress)
			{
				int Gold=PL->GetItemGoldRepPrice(Hero->arInventory[Under][0],Hero->arInventory[Under][1],Shop);
				int Gem=PL->GetItemGemRepPrice(Hero->arInventory[Under][0],Hero->arInventory[Under][1],Shop);
				blShop.Pick(3,Hero->arInventory[Under][0],Hero->arInventory[Under][1]);
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
				blShop.Pick(1,Hero->arInventory[Under][0],Hero->arInventory[Under][1]);				
			};
			blShop.Show();
		};
		if (Under>=0)
		{Under1=Under;};
		if (Click>=0)
		{
	//Клик на кукле героя		
			if (Click==0)
			{
			Click=(pFirstItem+SelectedItem)->ipSlot;
			if ((Hero->CanUse(SelectedItem))&&(Click>0))
			{
				switch (Click)
				{
				case 10:
					if ((Hero->arInventory[11][0]==0)&&(Hero->arInventory[10][0]!=0))
					{Click=11;};
					break;
				case 11:
					Click=12;
					if (Hero->arInventory[12][0]==0)
					{break;}				
					if (Hero->arInventory[13][0]==0)
					{
						Click=13;
						break;
					};
					if (Hero->arInventory[14][0]==0)
					{
						Click=14;
						break;
					};
					if (Hero->arInventory[15][0]==0)
					{
						Click=15;						
					};
					break;
				case 100:
					Click=1;
					break;
				};
			}
			else
			{
				Click=16;
				for (int i=16; i<INVENTORY_SIZE+1; i++)
				{
					if (Hero->arInventory[i][0]==0)
					{
						Click=i;
						break;
					};
				};
			};
			};

		//	if (ItemOwner==1)
		//	{
			if (SelectedItem)
			{				
	//Попытка поместить предмет в...
				if (((SlotNum(Click)==(pFirstItem+SelectedItem)->ipSlot)&&(Hero->CanUse(SelectedItem)))
					&&((SlotNum(Click)!=2)||(Hero->arInventory[1][0]==0)||((pFirstItem+Hero->arInventory[1][0])->ipSlot!=100)||(GetFreeSlot(Hero,0)))
					||(((pFirstItem+SelectedItem)->ipSlot==100)&&(SlotNum(Click)==1)&&(Hero->CanUse(SelectedItem))&&
					((Hero->arInventory[Click][0]==0)||(Hero->arInventory[2][0]==0)||(GetFreeSlot(Hero,SelectedSlot))))
					||(Click>15))
				{
					if (Hero->arInventory[Click][0]>0)
					{
	//Занятый слот
					//	if (ItemOwner<3)
					//	{
						if (((SlotNum(SelectedSlot)==(pFirstItem+Hero->arInventory[Click][0])->ipSlot)&&
							((Hero->CanUse(Hero->arInventory[Click][0])))||(GetFreeSlot(Hero,0)))
							||(((pFirstItem+Hero->arInventory[Click][0])->ipSlot==100)&&(SlotNum(SelectedSlot)==1)&&
							((Hero->arInventory[2][0]==0)||(GetFreeSlot(Hero,SelectedSlot))))
							||(SelectedSlot>15)||(ItemOwner>1))
						{
							if (ItemOwner==1)
							{
							if (((!Hero->CanUse(Hero->arInventory[Click][0]))||
								(SlotNum(SelectedSlot)!=(pFirstItem+Hero->arInventory[Click][0])->ipSlot))&&(SelectedSlot<16))
							//Перемещение предмета в инвентарь, если отдающий герой не может его использовать
							{
								int Free=GetFreeSlot(Hero,0);
								Hero->arInventory[Free][0]=Hero->arInventory[Click][0];
								Hero->arInventory[Free][1]=Hero->arInventory[Click][1];								
							}
							else
							{
								Hero->arInventory[SelectedSlot][0]=Hero->arInventory[Click][0];
								Hero->arInventory[SelectedSlot][1]=Hero->arInventory[Click][1];
							};
							//Перемещение щита в инвентарь, если отдающий герой получил двуручный предмет
							if (((pFirstItem+Hero->arInventory[Click][0])->ipSlot==100)&&(Hero->arInventory[2][0]>0)
								&&Hero->CanUse(Hero->arInventory[Click][0])&&(SelectedSlot==1))
							{
								int Free=GetFreeSlot(Hero,SelectedSlot);
								Hero->arInventory[Free][0]=Hero->arInventory[2][0];
								Hero->arInventory[Free][1]=Hero->arInventory[2][1];
								Hero->arInventory[2][1]=0;
								Hero->arInventory[2][0]=0;
							};
							};
							if (ItemOwner==2)
							{
								PL->arTreasury[SelectedSlot][0]=Hero->arInventory[Click][0];
								PL->arTreasury[SelectedSlot][1]=Hero->arInventory[Click][1];
							};
							int Old[2];
							if (ItemOwner==3)
							{
								Old[0]=Hero->arInventory[Click][0];
								Old[1]=Hero->arInventory[Click][1];
							};
							Hero->arInventory[Click][0]=SelectedItem;
							Hero->arInventory[Click][1]=SelectedItemDur;
							//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
							if ((Click==1)&&((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0))
							{
								int Free=GetFreeSlot(Hero,SelectedSlot);
								Hero->arInventory[Free][0]=Hero->arInventory[2][0];
								Hero->arInventory[Free][1]=Hero->arInventory[2][1];
								Hero->arInventory[2][1]=0;
								Hero->arInventory[2][0]=0;
							};
							//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
							if ((Click==2)&&(Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100))
							{
								int Free=GetFreeSlot(Hero,0);
								Hero->arInventory[Free][0]=Hero->arInventory[1][0];
								Hero->arInventory[Free][1]=Hero->arInventory[1][1];
								Hero->arInventory[1][1]=0;
								Hero->arInventory[1][0]=0;
							};
							if (ItemOwner==3)
							{
								//Покупка предмета в занятый слот
								CoinSound();						
								bBuy=true;							
								PL->Gold-=PL->GetItemGoldPrice(SelectedItem, Shop);
								PL->Gem-=PL->GetItemGemPrice(SelectedItem, Shop);
								for (int i=0; i<MAX_ITEM_RES; i++)
								{
									if (((pFirstItem+SelectedItem)->arRes[i]>0)&&(PL->GetResource((pFirstItem+SelectedItem)->arRes[i])==0))
									{
										CurGame.ResPrices[(pFirstItem+SelectedItem)->arRes[i]]+=CurGame.ResBuy;
									};
								};
								if (Shop->Items[SelectedSlot][1]>0)
								{
									Shop->Items[SelectedSlot][1]--;
									if (Shop->Items[SelectedSlot][1]==0)
									{Shop->Items[SelectedSlot][0]=0;};
								};
								SelectedItem=Old[0];
								SelectedItemDur=Old[1];
								draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
								draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
								set_mouse_sprite(bmpMouse);
								SelectedSlot=-1;
								for (int i=16; i<26; i++)
								{
									if (Hero->arInventory[i][0]==0)
									{
										SelectedSlot=i;
										break;
									};
								};								
								ItemOwner=1;
								if (SelectedSlot<0)
								{
									SelectedSlot=35;
									for (int i=0; i<TREASURY_SIZE; i++)
									{
										if (arPlayer[Hero->ipPlayer].arTreasury[i][0]==0)
										{
											SelectedSlot=i;
											break;
										};
									};								
									ItemOwner=2;
								};
								blResource.Show();
							}
							else
							{
								SelectedItem=0;
								SelectedSlot=0;
								set_mouse_sprite(bmpMousePointer[0]);
								buttTrash1->Activate(0);
							};
							blShop.Pick(0,0,0);
							blItem.Show();							
							blStat.Show();
							blInfo.Show();
							blShop.Show();
							blTreasury.Show();
						};
					//	}
					//	else
					//	{

					//	};
					}
					else
					{	
	//Свободный слот
						if (((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0)&&(Click==1)&&(GetFreeSlot(Hero,0)==0))
						{}
						else
						{
						bool bSkip=true;
						if (ItemOwner==3)
						{
			//Покупка предмета
							CoinSound();
						//	play_sample(wavCoins,iSoundVolume/2,128,1000,0);
							bBuy=true;
							if ((key[KEY_LCONTROL])||(key[KEY_RCONTROL]))
							{bSkip=false;};
							PL->Gold-=PL->GetItemGoldPrice(SelectedItem, Shop);
							PL->Gem-=PL->GetItemGemPrice(SelectedItem, Shop);
							for (int i=0; i<MAX_ITEM_RES; i++)
							{
								if (((pFirstItem+SelectedItem)->arRes[i]>0)&&(PL->GetResource((pFirstItem+SelectedItem)->arRes[i])==0))
								{
									CurGame.ResPrices[(pFirstItem+SelectedItem)->arRes[i]]+=CurGame.ResBuy;
								};
							};
							if (Shop->Items[SelectedSlot][1]>0)
							{
								Shop->Items[SelectedSlot][1]--;
								if (Shop->Items[SelectedSlot][1]==0)
								{
									Shop->Items[SelectedSlot][0]=0;
									bSkip=true;
								};
							};
							if ((PL->Gold<PL->GetItemGoldPrice(SelectedItem, Shop))||((PL->GetItemGemPrice(SelectedItem, Shop)>0)&&(PL->Gem<PL->GetItemGemPrice(SelectedItem, Shop))))
							{bSkip=true;};
						};
						Hero->arInventory[Click][0]=SelectedItem;
						Hero->arInventory[Click][1]=SelectedItemDur;	
						//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
						if (((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0)&&(Click==1))
						{
							int Free=GetFreeSlot(Hero,0);
							Hero->arInventory[Free][0]=Hero->arInventory[2][0];
							Hero->arInventory[Free][1]=Hero->arInventory[2][1];
							Hero->arInventory[2][1]=0;
							Hero->arInventory[2][0]=0;
						};
						//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
						if ((Click==2)&&(Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100))
						{
							int Free=GetFreeSlot(Hero,0);
							Hero->arInventory[Free][0]=Hero->arInventory[1][0];
							Hero->arInventory[Free][1]=Hero->arInventory[1][1];
							Hero->arInventory[1][1]=0;
							Hero->arInventory[1][0]=0;
						};						
						if (bSkip)
						{
							SelectedItem=0;
							SelectedSlot=0;
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash1->Activate(0);
						};
						blShop.Pick(0,0,0);
						blItem.Show();
						blStat.Show();
						blInfo.Show();
						blShop.Show();
						blResource.Show();
					};
					};
				};
			}
			else
			{
	
				if (Hero->arInventory[Click][0]>0)
				{
					if (blShop.buttRepair.bPress)
					{
				//Ремонт предмета
						int Gold=PL->GetItemGoldRepPrice(Hero->arInventory[Click][0],Hero->arInventory[Click][1],Shop);
						int Gem=PL->GetItemGemRepPrice(Hero->arInventory[Click][0],Hero->arInventory[Click][1],Shop);
						blShop.Pick(3,Hero->arInventory[Click][0],Hero->arInventory[Click][1]);
						if (((Gold>0)||(Gem>0))&&((PL->Gem>=Gem)||(Gem==0))&&(PL->Gold>=Gold))
						{
							PL->Gem-=Gem;
							PL->Gold-=Gold;
							PlayUnitSound(wavRepair);
						//	play_sample(wavClick,iSoundVolume,128,1000,0);
							Hero->arInventory[Click][1]=(pFirstItem+Hero->arInventory[Click][0])->iDurability;
							blShop.Pick(3,Hero->arInventory[Click][0],Hero->arInventory[Click][1]);
							blItem.Show();
							blShop.Show();
							blStat.Show();
							blInfo.Show();
							blResource.Show();
							set_mouse_sprite(bmpMousePointer[18]);							
						}
					}
					else
				//Взятие предмета
				{	
					SelectedItem=Hero->arInventory[Click][0];
					SelectedItemDur=Hero->arInventory[Click][1];
					SelectedSlot=Click;
					ItemOwner=1;
					Hero->arInventory[Click][0]=0;
					Hero->arInventory[Click][1]=0;
					blShop.Pick(1,SelectedItem,SelectedItemDur);
					blShop.Show();
					blItem.Show();
					blStat.Show();
					blInfo.Show();
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);
					buttTrash1->Activate(1);					
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
		
		if (buttTrash1->Check())
		{
			if (SelectedItem>0)
			{
				if (ItemOwner<3)
				{
					int WillDel=1;
					if (arHumans[arPlayer[Hero->ipPlayer].iColor].arMess[4])
					{WillDel=(pFirstDialog+1462)->Show(1,1,0,0,NULL,NULL);}
					if (WillDel==1)
					{
						SelectedItem=0;					
						SelectedSlot=0;
						buttTrash1->Activate(0);
						set_mouse_sprite(bmpMousePointer[0]);
					};
				}
				else
				{
					SelectedItem=0;					
					SelectedSlot=0;
					blShop.Pick(0,0,0);
					blShop.Show();
					buttTrash1->Activate(0);			
					set_mouse_sprite(bmpMousePointer[0]);
				};
			};
		};

		blInfo.Check();
		blStat.Check();
		if (blResource.Check(objGUI))
		{blShop.Show();};
		/*if (arPlayer[Hero->ipPlayer].GetBldParam1(34)>0)
		{
			int Check=mskAlchemy.Check();
			if ((Check==30)||(Check==40))
			{
				objGUI=ShowActiveObjects(objGUI,10,false);
				WindowAlchemy(Hero->ipPlayer);
				blResource.Show();
			};
		};*/
		}; //Tutorial
		
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,10,true);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	if (SelectedItem)
	{
		if (ItemOwner==1)
		{
			Hero->arInventory[SelectedSlot][0]=SelectedItem;
			Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
		};
		if (ItemOwner==2)
		{
			PL->arTreasury[SelectedSlot][0]=SelectedItem;
			PL->arTreasury[SelectedSlot][1]=SelectedItemDur;
		};
	};
	
	set_mouse_sprite(bmpMousePointer[0]);
	destroy_bitmap(bmpMouse);
	delete buttTrash1;
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	bHint=false;
	HintType=0;
	iAntisleep=iAntisleepDelay;
	Shop->PriceModifier[Hero->ipPlayer]=OldPrice;
	//Сдвигаем товары если выкуплен какой-либо лот
	int i=0; bool bRunShop=true;
	while (bRunShop)
	{
		while ((i<SHOP_SIZE-1)&&(Shop->Items[i][0]>0))
		{i++;};
		if (i>=SHOP_SIZE-1)
		{bRunShop=false;}
		else
		{
			bRunShop=false;
			for (int j=i+1; j<SHOP_SIZE; j++)
			{
				if (Shop->Items[j][0]>0)
				{bRunShop=true;};
			};
			if (bRunShop)
			{
				for (int j=i; j<SHOP_SIZE-1; j++)
				{
					Shop->Items[j][0]=Shop->Items[j+1][0];
					Shop->Items[j][1]=Shop->Items[j+1][1];					
				};
				Shop->Items[SHOP_SIZE-1][0]=0;
				Shop->Items[SHOP_SIZE-1][1]=0;
			};
		};
	};
};


//***************************************************
//*** Экран гарнизона *******************************
//***************************************************

void ScreenGarrison (dcHero * Garrison, bool CanHire)
{
	iAntisleep=iAntisleepDelay;
	bool bMerc;
	GUI_UnitBlock blUnit;	
	GUI_RecruitBlock blRecruit;
	GUI_ResourceBlock blResource;
	BITMAP *bmpBack=create_bitmap(512,768);
	blit(screen1,bmpBack,0,0,0,0,512,768);
	draw_sprite(bmpBack,bmpGUI_Castle,0,0);
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpBack);
	BITMAP * bmpMouse = create_bitmap(150,130);
//	int SelectedHero;
	int SelectedSlot=0;
	dcUnit * SelectedUnit=NULL;
	objGUI=new G_Object(objGUI,512,0,1,bmpGUI_Garrison);

	objGUI=blRecruit.Init(&arPlayer[Garrison->ipPlayer],512,0,objGUI,false,Garrison->ipProvince);
	objGUI=blResource.Init(&arPlayer[Garrison->ipPlayer],512,734,objGUI);
	objGUI=blUnit.Init(Garrison,571,373,objGUI);

	UI_Button1 * buttTrash=new UI_Button1();
	objGUI=buttTrash->Init(867,460,1,3,bmpGUI_ButtTrashNorm,bmpGUI_ButtTrashAct,bmpButton[231],bmpButton[232],objGUI);
	buttTrash->ipHotkey=KEY_DEL;
	buttTrash->Activate(0);
	buttTrash->wavSound=wavDelete;

	/*UI_Mask1 mskAlchemy;
	mskAlchemy.Init(512+11,727,240,40,1);
	mskAlchemy.iHint=352;*/

	UI_Button1 buttExit;
	objGUI=buttExit.Init(895,540,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;		

	set_mouse_sprite(bmpMousePointer[0]);

	while (!(buttExit.Check()))
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
					bool bSkip=true;					
					if ((!Garrison->pSquad[Click])&&(SlotLvl(Click)>=SelectedUnit->pType()->iLevel))
					{						
						Garrison->PlaceUnit(SelectedUnit,Click);					
						clUnitType Rookie=*(SelectedUnit->pType());
						CoinSound();
					//	play_sample(wavCoins,iSoundVolume/2,128,1000,0);
					//	int Gold=Rookie.iGoldPrice;
					//	int Gem=Rookie.iGemPrice;
						if (bMerc)
						{
							arPlayer[Garrison->ipPlayer].Gold-=arPlayer[Garrison->ipPlayer].GetUnitGoldPrice(SelectedUnit->GetUnitType(),Garrison->ipProvince)*2;
							arPlayer[Garrison->ipPlayer].Gem-=arPlayer[Garrison->ipPlayer].GetUnitGemPrice(SelectedUnit->GetUnitType(),Garrison->ipProvince)*2;
							arPlayer[Garrison->ipPlayer].arRecruit[11]=0;
							blRecruit.Recalc();
						}
						else
						{
							if ((key[KEY_LCONTROL])||(key[KEY_RCONTROL]))
							{bSkip=false;};
							arPlayer[Garrison->ipPlayer].Gold-=arPlayer[Garrison->ipPlayer].GetUnitGoldPrice(SelectedUnit->GetUnitType(),Garrison->ipProvince);
							arPlayer[Garrison->ipPlayer].Gem-=arPlayer[Garrison->ipPlayer].GetUnitGemPrice(SelectedUnit->GetUnitType(),Garrison->ipProvince);
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
					//	arPlayer[Garrison->ipPlayer].Gold-=Gold;
					//	arPlayer[Garrison->ipPlayer].Gem-=Gem;
						blResource.Show();						
						blUnit.Show();						
						blRecruit.Show();
						if (bSkip)
						{
							SelectedUnit=NULL;
							SelectedSlot=0;
							set_mouse_sprite(bmpMousePointer[0]);
							buttTrash->Activate(0);
						}
						else
						{SelectedUnit=new dcUnit(SelectedUnit->GetUnitType(),arPlayer[Garrison->ipPlayer].GetBldParam1(20));};
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
		if (buttTrash->Check())
		{
			if (SelectedUnit)
			{
				if (SelectedSlot>=0)
				{
					if ((arPlayer[Garrison->ipPlayer].GetBldParam1(24)>0)&&(SelectedUnit->arMedal[0]!=6))
					{
						int Price=(SelectedUnit->GetGoldPrice()+SelectedUnit->GetGemPrice()*2)*(SelectedUnit->iLevel+1)/10;
						if (Price<1)
						{Price=1;};
						arPlayer[Garrison->ipPlayer].Gold+=Price;
						if (arHumans[arPlayer[Garrison->ipPlayer].iColor].arMess[3])
						{
							char Txt[200];
							sprintf(Txt,(pFirstDialog+1090)->sText,Price);
							(pFirstDialog+1090)->Show(1,1,0,0,NULL,Txt);
						};
						blResource.Show();
					};
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
		/*if (arPlayer[Garrison->ipPlayer].GetBldParam1(34)>0)
		{
			int Check=mskAlchemy.Check();
			if ((Check==30)||(Check==40))
			{
				objGUI=ShowActiveObjects(objGUI,10,false);
				WindowAlchemy(Garrison->ipPlayer);
				blResource.Show();
			};
		};*/
		
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,10,true);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};	
	if (SelectedUnit)
	{
		Garrison->pSquad[SelectedSlot]=SelectedUnit;
	};
	set_mouse_sprite(bmpMousePointer[0]);
	destroy_bitmap(bmpMouse);
	destroy_bitmap(bmpBack);
	delete buttTrash;		
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	iAntisleep=iAntisleepDelay;
};

const char* GetRulerNick(int Karma)
{
	for (int i=0; i<=iMaxNick; i++)
	{
		if (((pFirstNick+i)->Min<=Karma)&&((pFirstNick+i)->Max>=Karma))
		{return (pFirstNick+i)->sText;};
	};
	return "";
};

const char* GetRulerTitle(int Score)
{
	for (int i=0; i<=iMaxTitle; i++)
	{
		if (((pFirstTitle+i)->Min<=Score)&&((pFirstTitle+i)->Max>=Score))
		{return (pFirstTitle+i)->sText;};
	};
	return "";
};

//***************************************************
//*** Экран статистики ******************************
//***************************************************
int ScreenStatistics (int Player)
{
	iAntisleep=iAntisleepDelay;
	int Result=0;	
	BITMAP *bmpStat=create_bitmap(1024,768);
	draw_sprite(bmpStat,arPlayer[Player].GetAvatar(),452,43);
	draw_sprite(bmpStat,bmpGUI_Statistics,0,0);
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpStat);	
	UI_Button1 buttExit;
	objGUI=buttExit.Init(918,682,1,2,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;

	UI_Button1 buttProv;
	objGUI=buttProv.Init(190,700,1,2,bmpButton[87],bmpButton[88],bmpButton[89],bmpButton[90],objGUI);
	buttProv.Activate(1);
	buttProv.ipHotkey=KEY_P;
	buttProv.iHint=551;

	char STR[500];
	arPlayer[Player].ShowHerb(419,54,bmpStat);
	arPlayer[Player].ShowHerb(585,54,bmpStat);
	sprintf(STR,"%s %s",arPlayer[Player].GetName(),GetRulerNick(arPlayer[Player].iKarma));
	textprintf_centre_ex(bmpStat,Font1,517,23,COLOR_1,-1,STR);
	textprintf_centre_ex(bmpStat,Font1,185,49,COLOR_1,-1,(pFirstPhrase+264)->str);
	textprintf_centre_ex(bmpStat,Font1,838,49,COLOR_1,-1,(pFirstPhrase+58)->str);
	textprintf_centre_ex(bmpStat,Font1,517,204,COLOR_1,-1,arDifficulty[arPlayer[Player].iDifficulty].sName);
	textprintf_centre_ex(bmpStat,Font1,517,252,COLOR_1,-1,"%s: %d",(pFirstPhrase+51)->str,CurGame.iCurrentTurn);
	if (arPlayer[Player].iUnion>0)
	{textprintf_centre_ex(bmpStat,Font1,517,272,COLOR_1,-1,"%s: %s",(pFirstPhrase+186)->str,(pFirstRace+arPlayer[Player].iUnion)->sName);}
	else
	{textprintf_centre_ex(bmpStat,Font1,517,272,COLOR_1,-1,"%s: %s",(pFirstPhrase+186)->str,(pFirstPhrase+40)->str);};
	textprintf_centre_ex(bmpStat,Font1,517,292,COLOR_1,-1,"%s: %d",(pFirstPhrase+267)->str,arPlayer[Player].iWin);
	textprintf_centre_ex(bmpStat,Font1,517,312,COLOR_1,-1,"%s: %d",(pFirstPhrase+268)->str,arPlayer[Player].iLose);	
	int Calc=0;
	for (int i=1; i<=MapSize; i++)
	{
		if ((pFirstProvince+i)->ipPlayer==Player)
		{Calc++;};
	};
	textprintf_centre_ex(bmpStat,Font1,517,332,COLOR_1,-1,"%s: %d",(pFirstPhrase+269)->str,Calc);
	Calc=0;
	for (int i=0; i<MAX_HERO; i++)
	{
		if (arPlayer[Player].arHero[i])
		{Calc++;};
	};
	textprintf_centre_ex(bmpStat,Font1,517,352,COLOR_1,-1,"%s: %d",(pFirstPhrase+270)->str,Calc);
	Calc=0;
	for (int i=1; i<17; i++)
	{
		if (arPlayer[Player].arKills[i]>0)
		{Calc++;};
	};
	textprintf_centre_ex(bmpStat,Font1,517,372,COLOR_1,-1,"%s: %d",(pFirstPhrase+273)->str,Calc);
	textprintf_centre_ex(bmpStat,Font1,517,395,COLOR_1,-1,"%s",(pFirstPhrase+274)->str);
	textprintf_centre_ex(bmpStat,Font1,517,415,COLOR_1,-1,"%s: %d",(pFirstPhrase+275)->str,arPlayer[Player].arLoot[0]);
	textprintf_centre_ex(bmpStat,Font1,517,435,COLOR_1,-1,"%s: %d",(pFirstPhrase+276)->str,arPlayer[Player].arLoot[1]);
	textprintf_centre_ex(bmpStat,Font1,517,455,COLOR_1,-1,"%s: %d",(pFirstPhrase+277)->str,arPlayer[Player].arLoot[2]);
	textprintf_centre_ex(bmpStat,Font1,517,480,COLOR_1,-1,"%s: %d",(pFirstPhrase+278)->str,arPlayer[Player].iQuest);
	textprintf_centre_ex(bmpStat,Font1,517,505,COLOR_1,-1,"%s: %d",(pFirstPhrase+396)->str,arPlayer[Player].iTimeShift);

//	textprintf_centre_ex(bmpStat,Font1,820,39,COLOR_2,-1,"%s %s",(pFirstPhrase+52)->str,sMapFile.str);

	int GoldInc=0, GoldSpent=0;
	int GoldProvInc=0, GemProvInc=0;
	int GoldCastInc=0, GemCastInc=0;
	int GoldOppInc=0, GemOppInc=0;
	int GoldHeroInc=0, GemHeroInc=0;
	int GoldGuardSpent=0, GemGuardSpent=0;
	int GoldGarrSpent=0, GemGarrSpent=0;
	int GoldOppSpent=0, GemOppSpent=0;
	int GoldHeroSpent=0, GemHeroSpent=0;
	int GoldCorrupt=0, GemCorrupt=0, ProvNum=0;
	int GemInc=0, GemSpent=0;	
//Провинции
	for (int i=1; i<=MapSize; i++)
	{
		int DefFourage=0;
		if ((pFirstProvince+i)->ipPlayer==Player)
		{
			if ((pFirstProvince+i)->iPopulation>0)
			{
				ProvNum+=(pFirstProvince+i)->GetCorruption();
			};
			if ((pFirstProvince+i)->GetSiegeIndex()<0)
			{
			/*if ((pFirstProvince+i)->iNumber==arPlayer[Player].ipProvince)
			{
				GoldCastInc+=(pFirstProvince+i)->GetGoldIncome();
				GemCastInc+=(pFirstProvince+i)->GetGemIncome();
			}
			else*/
			{
				GoldProvInc+=(pFirstProvince+i)->GetGoldIncome();
				GemProvInc+=(pFirstProvince+i)->GetGemIncome();
			};
				
		//Защитник		
		if ((pFirstProvince+i)->iDefender>0)
		{
			if ((pFirstDefender+(pFirstProvince+i)->iDefender)->iGoldPayment<0)
			{
				GoldGuardSpent+=(pFirstProvince+i)->pGuard->GetDefenderGoldPayment();
				GemGuardSpent+=(pFirstProvince+i)->pGuard->GetDefenderGemPayment();
			}
			else
			{
				GoldGuardSpent+=(pFirstDefender+(pFirstProvince+i)->iDefender)->iGoldPayment;
				GemGuardSpent+=(pFirstDefender+(pFirstProvince+i)->iDefender)->iGemPayment;
			};
			for (int AB=0; AB<MAX_DEFENDER_ABILITY; AB++)
			{
				if ((pFirstDefender+(pFirstProvince+i)->iDefender)->arAbility[AB][0]==4)
				{
					DefFourage=(pFirstDefender+(pFirstProvince+i)->iDefender)->arAbility[AB][1];
				};
			};

		};
		};
		//Гарнизон
		if ((pFirstProvince+i)->pGarrison)
		{
			int TempGold=0;
			for (int k=0; k<MAX_SQUAD; k++)
			{
				if ((pFirstProvince+i)->pGarrison->pSquad[k])
				{
					TempGold+=(pFirstProvince+i)->pGarrison->pSquad[k]->GetGoldPayment();
					GemGarrSpent+=(pFirstProvince+i)->pGarrison->pSquad[k]->GetGemPayment();					
				};
			};
			TempGold-=TempGold*(DefFourage+arPlayer[Player].GetBldParam1(28))/100;
			if (TempGold<0)
			{TempGold=0;};
			GoldGarrSpent+=TempGold;
		};
		};
	};	
	int Perc=0;
//Родовой замок
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if (arPlayer[Player].arBld[i])
		{
			for (int j=0; j<MAX_BUILDING_ABILITY; j++)
			{
				if ((pFirstInnerBld+i)->arAbility[j][0]==5)
				{
					GoldCastInc+=(pFirstInnerBld+i)->arAbility[j][1];
					GemCastInc+=(pFirstInnerBld+i)->arAbility[j][2];
				};
				if ((pFirstInnerBld+i)->arAbility[j][0]==9)
				{
					GoldCastInc+=arPlayer[Player].GetResource((pFirstInnerBld+i)->arAbility[j][1])*(pFirstInnerBld+i)->arAbility[j][2];
				};
				if ((pFirstInnerBld+i)->arAbility[j][0]==22)
				{
					Perc+=(pFirstInnerBld+i)->arAbility[j][1];
				};				
			};
		};		
	};
	int Profit=arPlayer[Player].Gold*arPlayer[Player].GetBldParam1(32)/100;
	if (Profit>MAX_PROFIT)
	{Profit=MAX_PROFIT;};
	GoldCastInc+=Profit;
	GoldCastInc+=arPlayer[Player].iGoldAdd;
	GemCastInc+=arPlayer[Player].iGemAdd;
//Оппоненты
	for (int PL=1; PL<=CurGame.iMaxPlayers; PL++)
	{
		if ((PL!=Player)&&(arPlayer[PL].bInGame))
		{
			if (arPlayer[Player].Politic[PL].Trade.bTrade)
			{
				GoldOppSpent+=arPlayer[Player].Politic[PL].Trade.Gold;
				GoldOppInc+=arPlayer[PL].Politic[Player].Trade.Gold;
				GemOppSpent+=arPlayer[Player].Politic[PL].Trade.Gem;
				GemOppInc+=arPlayer[PL].Politic[Player].Trade.Gem;
			};
			for (int i=1; i<=iMaxInnerBld; i++)
			{
				if (arPlayer[PL].arBld[i])
				{
					for (int j=0; j<MAX_BUILDING_ABILITY; j++)
					{
						if ((pFirstInnerBld+i)->arAbility[j][0]==29)
						{
							if ((pFirstInnerBld+i)->arAbility[j][1]>0)
							{
								GoldOppInc+=(pFirstInnerBld+i)->arAbility[j][1];
							}
							else
							{
								GoldOppSpent-=(pFirstInnerBld+i)->arAbility[j][1];
							};
						};
					};
				};
			};			
		};
	};
//Герои
	for (int Hero=0; Hero<MAX_HERO; Hero++)
	{
		if (arPlayer[Player].arHero[Hero])
		{
			int Payment=0;
			for (int k=0; k<MAX_SQUAD; k++)
			{				
				if (arPlayer[Player].arHero[Hero]->pSquad[k])
				{
					Payment+=arPlayer[Player].arHero[Hero]->pSquad[k]->GetGoldPayment();
					GemHeroSpent+=arPlayer[Player].arHero[Hero]->pSquad[k]->GetGemPayment();					
				};
			};
			if (arPlayer[Player].arHero[Hero]->iDead<0)
			{
				Payment-=Payment*arPlayer[Player].arHero[Hero]->CheckHeroBonus(997)/100;
				GoldHeroInc+=arPlayer[Player].arHero[Hero]->CheckHeroBonus(81);
				GemHeroInc+=arPlayer[Player].arHero[Hero]->CheckHeroBonus(82);
			};
			GoldHeroSpent+=Payment;
		};
	};
	GoldInc=GoldProvInc+GoldCastInc+GoldHeroInc+GoldOppInc;
	GemInc=GemProvInc+GemCastInc+GemHeroInc+GemOppInc;
	GoldCastInc+=GoldInc*Perc/100;
	GoldInc+=GoldInc*Perc/100;
	//Коррупция
	int Corrupt=(arPlayer[Player].iCorrupt+ProvNum)/10-arDifficulty[arPlayer[Player].iDifficulty].iCorruptStart;
	if ((arPlayer[Player].Control==1)&&(Corrupt>0))
	{
		if (Corrupt>arDifficulty[arPlayer[Player].iDifficulty].iCorruptCap)
		{Corrupt=arDifficulty[arPlayer[Player].iDifficulty].iCorruptCap;};
		Corrupt=Corrupt*(100+arPlayer[Player].GetBldParam1(51))/100;
		GoldCorrupt=GoldInc*Corrupt/100;
		GemCorrupt=GemInc*Corrupt/100;
	};
	GoldSpent=GoldGuardSpent+GoldGarrSpent+GoldHeroSpent+GoldOppSpent+GoldCorrupt;
	GemSpent=GemGuardSpent+GemGarrSpent+GemHeroSpent+GemOppSpent+GemCorrupt;

	textprintf_centre_ex(bmpStat,Font1,120,84,COLOR_1,-1,"%s",(pFirstPhrase+53)->str);	
	textprintf_centre_ex(bmpStat,Font1,117,108,COLOR_2,-1,"%s",(pFirstPhrase+55)->str);
	textprintf_centre_ex(bmpStat,Font1,117,132,COLOR_2,-1,"%s",(pFirstPhrase+56)->str);
	textprintf_centre_ex(bmpStat,Font1,117,156,COLOR_2,-1,"%s",(pFirstPhrase+57)->str);
	textprintf_centre_ex(bmpStat,Font1,117,180,COLOR_2,-1,"%s",(pFirstPhrase+58)->str);
	textprintf_centre_ex(bmpStat,Font1,117,204,COLOR_2,-1,"%s",(pFirstPhrase+61)->str);
	textprintf_centre_ex(bmpStat,Font1,120,228,COLOR_1,-1,"%s",(pFirstPhrase+54)->str);
	textprintf_centre_ex(bmpStat,Font1,117,252,COLOR_2,-1,"%s",(pFirstPhrase+266)->str);
	textprintf_centre_ex(bmpStat,Font1,117,276,COLOR_2,-1,"%s",(pFirstPhrase+59)->str);
	textprintf_centre_ex(bmpStat,Font1,117,300,COLOR_2,-1,"%s",(pFirstPhrase+60)->str);
	textprintf_centre_ex(bmpStat,Font1,117,324,COLOR_2,-1,"%s",(pFirstPhrase+58)->str);
	textprintf_centre_ex(bmpStat,Font1,117,348,COLOR_2,-1,"%s",(pFirstPhrase+263)->str);
	textprintf_centre_ex(bmpStat,Font1,117,372,COLOR_2,-1,"%s",(pFirstPhrase+61)->str);
	textprintf_centre_ex(bmpStat,Font1,120,396,COLOR_1,-1,"%s",(pFirstPhrase+62)->str);

	textprintf_centre_ex(bmpStat,Font1,253,108,makecol(50,220,50),-1,"%d",GoldCastInc);
	textprintf_centre_ex(bmpStat,Font1,253,132,makecol(50,220,50),-1,"%d",GoldProvInc);
	textprintf_centre_ex(bmpStat,Font1,253,156,makecol(50,220,50),-1,"%d",GoldHeroInc);
	textprintf_centre_ex(bmpStat,Font1,253,180,makecol(50,220,50),-1,"%d",GoldOppInc);
	textprintf_centre_ex(bmpStat,Font1,253,204,makecol(50,220,50),-1,"%d",GoldInc);
	textprintf_centre_ex(bmpStat,Font1,253,252,makecol(220,20,20),-1,"%d",GoldHeroSpent);
	textprintf_centre_ex(bmpStat,Font1,253,276,makecol(220,20,20),-1,"%d",GoldGuardSpent);
	textprintf_centre_ex(bmpStat,Font1,253,300,makecol(220,20,20),-1,"%d",GoldGarrSpent);
	textprintf_centre_ex(bmpStat,Font1,253,324,makecol(220,20,20),-1,"%d",GoldOppSpent);
	textprintf_centre_ex(bmpStat,Font1,253,348,makecol(220,20,20),-1,"%d",GoldCorrupt);
	textprintf_centre_ex(bmpStat,Font1,253,372,makecol(220,20,20),-1,"%d",GoldSpent);
	if (GoldInc>GoldSpent)
	{
		textprintf_centre_ex(bmpStat,Font1,253,396,makecol(20,220,20),-1,"+%d",GoldInc-GoldSpent);
	}
	else
	{
		textprintf_centre_ex(bmpStat,Font1,253,396,makecol(220,20,20),-1,"%d",GoldInc-GoldSpent);
	};

	textprintf_centre_ex(bmpStat,Font1,339,108,makecol(50,220,50),-1,"%d",GemCastInc);
	textprintf_centre_ex(bmpStat,Font1,339,132,makecol(50,220,50),-1,"%d",GemProvInc);
	textprintf_centre_ex(bmpStat,Font1,339,156,makecol(50,220,50),-1,"%d",GemHeroInc);
	textprintf_centre_ex(bmpStat,Font1,339,180,makecol(50,220,50),-1,"%d",GemOppInc);
	textprintf_centre_ex(bmpStat,Font1,339,204,makecol(50,220,50),-1,"%d",GemInc);
	textprintf_centre_ex(bmpStat,Font1,339,252,makecol(220,20,20),-1,"%d",GemHeroSpent);
	textprintf_centre_ex(bmpStat,Font1,339,276,makecol(220,20,20),-1,"%d",GemGuardSpent);
	textprintf_centre_ex(bmpStat,Font1,339,300,makecol(220,20,20),-1,"%d",GemGarrSpent);
	textprintf_centre_ex(bmpStat,Font1,339,324,makecol(220,20,20),-1,"%d",GemOppSpent);
	textprintf_centre_ex(bmpStat,Font1,339,348,makecol(220,20,20),-1,"%d",GemCorrupt);
	textprintf_centre_ex(bmpStat,Font1,339,372,makecol(220,20,20),-1,"%d",GemSpent);
	if (GemInc>GemSpent)
	{
		textprintf_centre_ex(bmpStat,Font1,339,396,makecol(20,220,20),-1,"+%d",GemInc-GemSpent);
	}
	else
	{
		textprintf_centre_ex(bmpStat,Font1,339,396,makecol(220,20,20),-1,"%d",GemInc-GemSpent);
	};

	textprintf_centre_ex(bmpStat,Font1,117,452,COLOR_1,-1,"%s",(pFirstPhrase+63)->str);
	for (int i=1; i<=iMaxResource; i++)
	{
		textprintf_centre_ex(bmpStat,Font1,125,475+(i-1)*24,COLOR_2,-1,"%s",(pFirstResource+i)->sName);
		textprintf_centre_ex(bmpStat,Font1,211,475+(i-1)*24,COLOR_2,-1,"%d",arPlayer[Player].GetResource(i));	
		textprintf_centre_ex(bmpStat,Font1,266,475+(i-1)*24,COLOR_2,-1,"%d",arPlayer[Player].GetResGoldPriceStat(i));	
		textprintf_centre_ex(bmpStat,Font1,339,475+(i-1)*24,COLOR_2,-1,"%d",arPlayer[Player].GetResGemPriceStat(i));
	};

	textprintf_centre_ex(bmpStat,Font1,470,730,COLOR_2,-1,"%d",arPlayer[Player].Gold);
	textprintf_centre_ex(bmpStat,Font1,593,730,COLOR_2,-1,"%d",arPlayer[Player].Gem);

	bool bDrawPlayer=true;
	int iShift=1;
	if (Player==1)
	{iShift++;};
	UI_Button1 buttScrollUp;
	objGUI=buttScrollUp.Init(766,675,1,2,bmpButton[76],bmpButton[77],bmpButton[78],bmpButton[79],objGUI);	
	buttScrollUp.Activate(0);	
	UI_Button1 buttScrollDown;
	objGUI=buttScrollDown.Init(847,675,1,2,bmpButton[80],bmpButton[81],bmpButton[82],bmpButton[83],objGUI);
	buttScrollDown.Activate(1);
	if (CurGame.iMaxPlayers<12)
	{
		buttScrollDown.Activate(-1);
		buttScrollUp.Activate(-1);
	};

	while ((!buttExit.Check())&&(Result==0))
	{
		if (bDrawPlayer)
		{
			int Count=0;
			blit(bmpGUI_Statistics,bmpStat,655,135,655,135,330,600);
	//		int iStart=iShift;
	//		if (iShift==Player)
	//		{iStart++;};
			for (int i=iShift; i<=CurGame.iMaxPlayers; i++)
			{				
				if (i!=Player)
				{
					if (Count<10)
					{
						draw_sprite(bmpStat,bmpGUI_RulerBar,655,76+Count*59);
						if ((CurGame.arPlayerSet[i]==0)&&(arPlayer[Player].Politic[i].Treaty==0)&&(arPlayer[i].bInGame)&&(arPlayer[i].Control!=1))
						{
							draw_sprite(bmpStat,bmpSmallAvatar[16],660,80+Count*59);
							draw_sprite(bmpStat,bmpHerbRandom,723,90+Count*59);
							draw_sprite(bmpStat,bmpHerbRandom,947,90+Count*59);
							textprintf_centre_ex(bmpStat,Font1,852,90+Count*59,COLOR_1,-1,"%s",(pFirstPhrase+271)->str);
							textprintf_centre_ex(bmpStat,Font1,852,110+Count*59,COLOR_1,-1,"%s",arDifficulty[arPlayer[i].iDifficulty].sName);
						}
						else
						{							
							draw_sprite(bmpStat,arPlayer[i].GetSmallAvatar(),660,80+Count*59);
							arPlayer[i].ShowHerb(723,90+Count*59,bmpStat);
							arPlayer[i].ShowHerb(947,90+Count*59,bmpStat);
							textprintf_centre_ex(bmpStat,Font1,852,90+Count*59,COLOR_1,-1,"%s",arPlayer[i].GetName());
							if (arPlayer[i].bInGame)
							{
								textprintf_centre_ex(bmpStat,Font1,852,110+Count*59,COLOR_1,-1,"%s",arDifficulty[arPlayer[i].iDifficulty].sName);
								/*switch (arPlayer[Player].Politic[i].Treaty)
								{									
								case 0:
									textprintf_centre_ex(bmpStat,Font1,852,110+Count*59,COLOR_1,-1,"%s",(pFirstPhrase+272)->str);
									break;
								case 1:
									textprintf_centre_ex(bmpStat,Font1,852,110+Count*59,COLOR_1,-1,"%s",(pFirstPhrase+184)->str);
									break;
								case 2:
									textprintf_centre_ex(bmpStat,Font1,852,110+Count*59,COLOR_1,-1,"%s",(pFirstPhrase+185)->str);
									break;
								case 3:
									textprintf_centre_ex(bmpStat,Font1,852,110+Count*59,COLOR_1,-1,"%s",(pFirstPhrase+186)->str);
									break;

								};*/
							}
							else
							{textprintf_centre_ex(bmpStat,Font1,852,110+Count*59,COLOR_1,-1,"%s",(pFirstPhrase+265)->str);};
						};
						Count++;
					}
					else
					{
						buttScrollDown.Activate(1);
					};
				};
			};
			bDrawPlayer=false;
		};
		if (buttScrollUp.Check())
		{
			iShift--;
			if ((iShift==Player)&&(iShift>1))
			{iShift--;};
			if ((iShift==1)||((iShift==2)&&(Player==1)))
			{buttScrollUp.Activate(0);};
			buttScrollDown.Activate(1);
			bDrawPlayer=true;
		};
		if (buttScrollDown.Check())
		{			
			iShift++;
			if ((iShift==Player)&&(CurGame.iMaxPlayers>iShift+9))			
			{iShift++;};			
			buttScrollDown.Activate(0);
			buttScrollUp.Activate(1);
			bDrawPlayer=true;
		};
		if (buttProv.Check())
		{Result=ScreenProvList(Player);};
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,3,true);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	destroy_bitmap(bmpStat);
	iAntisleep=iAntisleepDelay;
	return Result;	
};


//***************************************************
//*** Экран статистики в кампании *******************
//***************************************************
void ScreenCampStat()
{	
	iAntisleep=iAntisleepDelay;
	BITMAP *bmpStat=create_bitmap(1024,768);
	draw_sprite(bmpStat,bmpAvatar[18],452,43);
	draw_sprite(bmpStat,bmpGUI_CampStat,0,0);
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpStat);	
	UI_Button1 buttExit;
	objGUI=buttExit.Init(918,682,1,2,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;
	char STR[500];
	DrawHerb(arHumans[0].iColor1,arHumans[0].iColor2,arHumans[0].iSign,419,54,bmpStat);
	DrawHerb(arHumans[0].iColor1,arHumans[0].iColor2,arHumans[0].iSign,584,54,bmpStat);	
	sprintf(STR,"%s %s",arHumans[0].sName,GetRulerNick(Campaign.arCampPlayer[0].iKarma));
	textprintf_centre_ex(bmpStat,Font1,517,23,COLOR_1,-1,STR);
	textprintf_centre_ex(bmpStat,Font1,200,49,COLOR_1,-1,(pFirstPhrase+279)->str);
	textprintf_centre_ex(bmpStat,Font1,834,49,COLOR_1,-1,(pFirstPhrase+466)->str);
	textprintf_centre_ex(bmpStat,Font1,517,204,COLOR_1,-1,arDifficulty[Campaign.iDifficulty].sName);
	textprintf_centre_ex(bmpStat,Font1,517,252,COLOR_1,-1,"%s: %d",(pFirstPhrase+51)->str,Campaign.iCurrentTurn);
	textprintf_centre_ex(bmpStat,Font1,517,275,COLOR_1,-1,(pFirstPhrase+280)->str);
	int Reward=0;
	for (int i=1; i<20; i++)
	{
		if (Campaign.arJournal[980+i]>0)
		{
			draw_sprite(bmpStat,(pFirstMedal+i)->SmallPic,405,293+Reward*20);
			switch (i)
			{
			case 1:
				textprintf_ex(bmpStat,Font2,420,295+Reward*20,COLOR_1,-1,"%s: %d",(pFirstPhrase+292)->str,Campaign.arJournal[980+i]);
				break;
			case 2:
				textprintf_ex(bmpStat,Font2,420,295+Reward*20,COLOR_1,-1,"%s: %d",(pFirstPhrase+287)->str,Campaign.arJournal[980+i]);
				break;
			case 3:
				textprintf_ex(bmpStat,Font2,420,295+Reward*20,COLOR_1,-1,"%s: %d",(pFirstPhrase+289)->str,Campaign.arJournal[980+i]);
				break;
			case 4:
				textprintf_ex(bmpStat,Font2,420,295+Reward*20,COLOR_1,-1,"%s: %d",(pFirstPhrase+293)->str,Campaign.arJournal[980+i]);
				break;
			case 5:
				textprintf_ex(bmpStat,Font2,420,295+Reward*20,COLOR_1,-1,"%s: %d",(pFirstPhrase+294)->str,Campaign.arJournal[980+i]);
				break;
			case 8:
				textprintf_ex(bmpStat,Font2,420,295+Reward*20,COLOR_1,-1,"%s: %d",(pFirstPhrase+288)->str,Campaign.arJournal[980+i]);
				break;
			case 10:
				textprintf_ex(bmpStat,Font2,420,295+Reward*20,COLOR_1,-1,"%s: %d",(pFirstPhrase+290)->str,Campaign.arJournal[980+i]);
				break;
			case 13:
				textprintf_ex(bmpStat,Font2,420,295+Reward*20,COLOR_1,-1,"%s: %d",(pFirstPhrase+291)->str,Campaign.arJournal[980+i]);
				break;
			case 15:
				textprintf_ex(bmpStat,Font2,420,295+Reward*20,COLOR_1,-1,"%s: %d",(pFirstPhrase+526)->str,Campaign.arJournal[980+i]);
				break;
			};
			Reward++;
		};
	};
	if (Reward==0)
	{
		textprintf_centre_ex(bmpStat,Font1,517,295,COLOR_1,-1,(pFirstPhrase+40)->str);
		Reward++;
	};

	if (Campaign.arJournal[980]>=0)
	{
		textprintf_centre_ex(bmpStat,Font1,517,295+Reward*20+10,COLOR_1,-1,(pFirstPhrase+503)->str);
		textprintf_centre_ex(bmpStat,Font2,517,295+Reward*20+30,COLOR_1,-1,GetRulerTitle(Campaign.arJournal[980]));
	};


	textprintf_centre_ex(bmpStat,Font1,517,559,COLOR_1,-1,(pFirstPhrase+467)->str);
	textprintf_centre_ex(bmpStat,Font1,522,729,COLOR_2,-1,"%d",Campaign.arCampPlayer[0].iEnergy);
	int iWork=0, iShard=0;
	dcShard *SH=pFirstShard;	
	while (SH)
	{
		if (SH->ipPlayer==0)
		{
		//	iWork+=SH->Income;
			iShard++;
		};
		SH=SH->pNext;		
	};
	iWork=GetEnergyIncome();
//	iWork-=iWork*Campaign.arCampPlayer[0].iEnergyLose/100;
//	if (iWork<0)
//	{iWork=0;};
	textprintf_centre_ex(bmpStat,Font1,478,589,COLOR_2,-1,(pFirstPhrase+468)->str);
	if (iWork>0)
	{textprintf_centre_ex(bmpStat,Font1,603,589,makecol(50,220,50),-1,"+%d",iWork);}
	else
	{textprintf_centre_ex(bmpStat,Font1,603,589,makecol(50,220,50),-1,"%d",iWork);};
	textprintf_centre_ex(bmpStat,Font1,478,589+24,COLOR_2,-1,(pFirstPhrase+469)->str);
	if (Campaign.arCampPlayer[0].iEnergyBonus>0)
	{textprintf_centre_ex(bmpStat,Font1,603,589+24,makecol(50,220,50),-1,"+%d",Campaign.arCampPlayer[0].iEnergyBonus);}
	else
	{textprintf_centre_ex(bmpStat,Font1,603,589+24,makecol(50,220,50),-1,"%d",Campaign.arCampPlayer[0].iEnergyBonus);};
	textprintf_centre_ex(bmpStat,Font1,478,589+24*2,COLOR_2,-1,(pFirstPhrase+54)->str);
	textprintf_centre_ex(bmpStat,Font1,603,589+24*2,makecol(220,50,50),-1,"%d",Campaign.arCampPlayer[0].iEnergyPenalty);
	textprintf_centre_ex(bmpStat,Font1,478,589+24*4,COLOR_2,-1,(pFirstPhrase+61)->str);
	iWork+=Campaign.arCampPlayer[0].iEnergyBonus+Campaign.arCampPlayer[0].iEnergyPenalty;
	if (iWork>0)
	{textprintf_centre_ex(bmpStat,Font1,603,589+24*4,makecol(50,220,50),-1,"+%d",iWork);}
	else
	{textprintf_centre_ex(bmpStat,Font1,603,589+24*4,makecol(220,50,50),-1,"%d",iWork);};

	iWork=0;
	draw_sprite(bmpStat,bmpGUI_RulerBar,38,76+iWork*59);
	draw_sprite(bmpStat,bmpStuff[58],43,80+iWork*59);
	textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s: %d",(pFirstPhrase+442)->str,iShard);
	iWork++;
	if (Campaign.arCampPlayer[0].iKillScore>0)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,38,76+iWork*59);
		draw_sprite(bmpStat,bmpStuff[1],43,80+iWork*59);
		textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s: %d",(pFirstPhrase+273)->str,Campaign.arCampPlayer[0].iKillScore);
		iWork++;
	};
	if (Campaign.arCampPlayer[0].iMapScore>0)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,38,76+iWork*59);
		draw_sprite(bmpStat,bmpStuff[2],43,80+iWork*59);
		textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s: %d",(pFirstPhrase+441)->str,Campaign.arCampPlayer[0].iMapScore);
		iWork++;
	};
	if (Campaign.arDialog[103]>0)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,38,76+iWork*59);
		draw_sprite(bmpStat,bmpSmallAvatar[2],43,80+iWork*59);
		textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s",(pFirstPhrase+470)->str);
		iWork++;
	};
	if (Campaign.arDialog[350]>0)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,38,76+iWork*59);
		draw_sprite(bmpStat,bmpSmallAvatar[6],43,80+iWork*59);
		textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s",(pFirstPhrase+471)->str);
		iWork++;
	};
	if (Campaign.arItems[0]>0)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,38,76+iWork*59);
		draw_sprite(bmpStat,bmpStuff[59],43,80+iWork*59);
		textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s: %d",(pFirstPhrase+475)->str,Campaign.arItems[0]);
		iWork++;
	};
	if (Campaign.arItems[1]>0)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,38,76+iWork*59);
		draw_sprite(bmpStat,bmpStuff[60],43,80+iWork*59);
		if ((Campaign.arDialog[659]>0)||(Campaign.arDialog[34]>0))
		{textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s",(pFirstPhrase+474)->str);}
		else
		{
			if (Campaign.arDialog[655]>0)
			{textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s: %d",(pFirstPhrase+473)->str,Campaign.arItems[1]);}
			else
			{textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s: %d",(pFirstPhrase+472)->str,Campaign.arItems[1]);};
			
		};		
		iWork++;
	};
	if (Campaign.arDialog[661]>0)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,38,76+iWork*59);
		draw_sprite(bmpStat,bmpStuff[62],43,80+iWork*59);
		int Complete=10;
		if (Campaign.arDialog[664]>0)
		{Complete=20;};
		if (Campaign.arDialog[666]>0)
		{Complete=40;};
		if (Campaign.arDialog[668]>0)
		{Complete=70;};
		if (Campaign.arDialog[670]>0)
		{Complete=100;};
		textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,(pFirstPhrase+477)->str,Complete);
		iWork++;
	};
	if (Campaign.arDialog[687]>0)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,38,76+iWork*59);
		draw_sprite(bmpStat,bmpStuff[64],43,80+iWork*59);
		int Pers=Campaign.arItems[3]/10;		
		textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s: %d~",(pFirstPhrase+478)->str,Pers);
		iWork++;
	};
	if (Campaign.arDialog[25]>0)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,38,76+iWork*59);
		draw_sprite(bmpStat,bmpStuff[61],43,80+iWork*59);
		if (Campaign.arDialog[39]>0)
		{textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s",(pFirstPhrase+484)->str);}
		else
		{
			if (Campaign.arDialog[37]>0)
			{textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s",(pFirstPhrase+485)->str);}
			else
			{
				if (Campaign.arDialog[36]>0)
				{textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s",(pFirstPhrase+487)->str);}
				else
				{
					if (Campaign.arDialog[35]>0)
					{textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s",(pFirstPhrase+486)->str);}
					else
					{textprintf_centre_ex(bmpStat,Font1,235,100+iWork*59,COLOR_1,-1,"%s",(pFirstPhrase+479)->str);};
				};
			};
		};
		iWork++;
	};

	int iJRN=0, arMess[20];
	BITMAP *bmpJrnN[20], *bmpJrnA[20];
	UI_Button1 buttJrn[20];
	
	for (int i=0; i<=iMaxJournal; i++)
	{
		if (Campaign.arJournal[i]>0)
		{
			bmpJrnN[iJRN]=create_bitmap(bmpGUI_MenuScrN->w,bmpGUI_MenuScrN->h);
			blit(bmpGUI_MenuScrN,bmpJrnN[iJRN],0,0,0,0,bmpGUI_MenuScrN->w,bmpGUI_MenuScrN->h);
			textprintf_centre_ex(bmpJrnN[iJRN],Font1,146,8,COLOR_1,-1,(pFirstJournal+i)->sName);

			bmpJrnA[iJRN]=create_bitmap(bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
			blit(bmpGUI_MenuScrA,bmpJrnA[iJRN],0,0,0,0,bmpGUI_MenuScrA->w,bmpGUI_MenuScrA->h);
			textprintf_centre_ex(bmpJrnA[iJRN],Font1,146,8,COLOR_1,-1,(pFirstJournal+i)->sName);

			objGUI=buttJrn[iJRN].Init(691,74+iJRN*27,0,2,bmpJrnN[iJRN],bmpJrnA[iJRN],bmpJrnA[iJRN],bmpEmpty,objGUI);
			buttJrn[iJRN].Activate(1);
			arMess[iJRN]=i;
			iJRN++;
		};
	};

	while (!buttExit.Check())
	{
		for (int i=0; i<iJRN; i++)
		{
			if (buttJrn[i].Check())
			{
				objGUI=ShowActiveObjects(objGUI,3,false);
				WindowInfo((pFirstJournal+arMess[i])->sName,(pFirstJournal+arMess[i])->sText,bmpStuff[59]);
			};
		};
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,3,true);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	destroy_bitmap(bmpStat);
	for (int i=0; i<iJRN; i++)
	{
		destroy_bitmap(bmpJrnN[i]);
		destroy_bitmap(bmpJrnA[i]);
	};
	iAntisleep=iAntisleepDelay;
};



//***************************************************
//** Окно получения предметов при полном инвентаре **
//***************************************************

void WindowTakeLoot(dcHero * Hero)
{
	iAntisleep=iAntisleepDelay;
	if (arPlayer[Hero->ipPlayer].Control!=1)
	{	
		TakeLootAI(Hero);
		return;
	};
	bool NoExit=false;	
	GUI_ItemBlock blItem;
	int x=512-bmpGUI_TakeLoot->w/2;
	int y=384-bmpGUI_TakeLoot->h/2;	
	BITMAP* OldMouse=mouse_sprite;
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpBack);
	BITMAP *bmpWindow=create_bitmap(bmpGUI_TakeLoot->w,bmpGUI_TakeLoot->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpGUI_TakeLoot,bmpWindow,0,0,0,0,bmpGUI_TakeLoot->w,bmpGUI_TakeLoot->h);
	UI_Mask mskItem[9];
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+78,y+437,1,3,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;
	BITMAP * bmpMouse = create_bitmap(150,130);
	int SelectedSlot=0;
	int SelectedItem=0, SelectedItemDur;
	int SelectedSpell=0;
	WObj=blItem.Init(Hero,x+213,y+34,WObj);
	draw_sprite(bmpWindow,Hero->Pic,45,42);
//	textprintf_centre_ex(bmpWindow,Font1,100,180,COLOR_2,-1,"%s",Hero->pstName->str);
	textprintf_centre_ex(bmpWindow,Font1,266,14,COLOR_1,-1,"%s",(pFirstPhrase+319)->str);
	textprintf_centre_ex(bmpWindow,Font1,111,214,COLOR_1,-1,"%s:",(pFirstPhrase+66)->str);
	blItem.Show();
	for (int i=0; i<9; i++)
	{
		WObj=mskItem[i].Init(x+18+(i%3)*60,y+231+(i-i%3)/3*60,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
		if (arLootTable[i][0]>0)
		{
			draw_sprite(bmpWindow,(pFirstItem+arLootTable[i][0])->Pic,24+(i%3)*60,237+(i-i%3)/3*60);
		};
	};
	

	set_mouse_sprite(bmpMousePointer[0]);

	while ((!buttExit.Check())||(NoExit))
	{
		if	(!key[KEY_ESC])
		{NoExit=false;};		

//Манипулирование предметами
//**************************
		int Click=0;
		Click=blItem.Check(0,NULL,SelectedItem);
		if (Click>=0)
		{
	//Клик на кукле героя		
			if (Click==0)
			{
			Click=(pFirstItem+SelectedItem)->ipSlot;
			if ((Hero->CanUse(SelectedItem))&&(Click>0))
			{
				switch (Click)
				{
				case 10:
					if ((Hero->arInventory[11][0]==0)&&(Hero->arInventory[10][0]!=0))
					{Click=11;};
					break;
				case 11:
					Click=12;
					if (Hero->arInventory[12][0]==0)
					{break;}				
					if (Hero->arInventory[13][0]==0)
					{
						Click=13;
						break;
					};
					if (Hero->arInventory[14][0]==0)
					{
						Click=14;
						break;
					};
					if (Hero->arInventory[15][0]==0)
					{
						Click=15;						
					};
					break;
				case 100:
					Click=1;
					break;
				};
			}
			else
			{
				Click=16;
				for (int i=16; i<INVENTORY_SIZE+1; i++)
				{
					if (Hero->arInventory[i][0]==0)
					{
						Click=i;
						break;
					};
				};
			};
			};


			if (SelectedItem)
			{				
	//Попытка поместить предмет в...
				if (((SlotNum(Click)==(pFirstItem+SelectedItem)->ipSlot)&&(Hero->CanUse(SelectedItem)))
					&&((SlotNum(Click)!=2)||(Hero->arInventory[1][0]==0)||((pFirstItem+Hero->arInventory[1][0])->ipSlot!=100)||(GetFreeSlot(Hero,0)))
					||(((pFirstItem+SelectedItem)->ipSlot==100)&&(SlotNum(Click)==1)&&(Hero->CanUse(SelectedItem))&&
					((Hero->arInventory[Click][0]==0)||(Hero->arInventory[2][0]==0)||(GetFreeSlot(Hero,SelectedSlot))))
					||(Click>15))
				{
					if (Hero->arInventory[Click][0]>0)
					{
	//Занятый слот
						if (((SlotNum(SelectedSlot)==(pFirstItem+Hero->arInventory[Click][0])->ipSlot)&&
							((Hero->CanUse(Hero->arInventory[Click][0])))||(GetFreeSlot(Hero,0)))
							||(((pFirstItem+Hero->arInventory[Click][0])->ipSlot==100)&&(SlotNum(SelectedSlot)==1)&&
							((Hero->arInventory[2][0]==0)||(GetFreeSlot(Hero,SelectedSlot))))
							||(SelectedSlot>15))
						{
							if (((!Hero->CanUse(Hero->arInventory[Click][0]))||
								(SlotNum(SelectedSlot)!=(pFirstItem+Hero->arInventory[Click][0])->ipSlot))&&(SelectedSlot<16))
							//Перемещение предмета в инвентарь, если отдающий герой не может его использовать
							{
								int Free=GetFreeSlot(Hero,0);
								Hero->arInventory[Free][0]=Hero->arInventory[Click][0];
								Hero->arInventory[Free][1]=Hero->arInventory[Click][1];								
							}
							else
							{
								if (SelectedSlot<90)
								{
									Hero->arInventory[SelectedSlot][0]=Hero->arInventory[Click][0];
									Hero->arInventory[SelectedSlot][1]=Hero->arInventory[Click][1];
								}
								else
								{
									arLootTable[SelectedSlot-200][0]=Hero->arInventory[Click][0];
									arLootTable[SelectedSlot-200][1]=Hero->arInventory[Click][1];
									draw_sprite(bmpWindow,(pFirstItem+arLootTable[SelectedSlot-200][0])->Pic,24+((SelectedSlot-200)%3)*60,237+((SelectedSlot-200)-(SelectedSlot-200)%3)/3*60);
								};
							};
							//Перемещение щита в инвентарь, если отдающий герой получил двуручный предмет
							if (((pFirstItem+Hero->arInventory[Click][0])->ipSlot==100)&&(Hero->arInventory[2][0]>0)
								&&Hero->CanUse(Hero->arInventory[Click][0])&&(SelectedSlot==1))
							{
								int Free=GetFreeSlot(Hero,SelectedSlot);
								Hero->arInventory[Free][0]=Hero->arInventory[2][0];
								Hero->arInventory[Free][1]=Hero->arInventory[2][1];
								Hero->arInventory[2][1]=0;
								Hero->arInventory[2][0]=0;
							};
							Hero->arInventory[Click][0]=SelectedItem;
							Hero->arInventory[Click][1]=SelectedItemDur;
							//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
							if ((Click==1)&&((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0))
							{
								int Free=GetFreeSlot(Hero,SelectedSlot);
								if (Free>0)
								{
									Hero->arInventory[Free][0]=Hero->arInventory[2][0];
									Hero->arInventory[Free][1]=Hero->arInventory[2][1];
								}
								else
								{
									for (int i=0; i<9; i++)
									{
										if (arLootTable[i][0]==0)
										{
											Free=i;
											break;
										};
									};
									arLootTable[Free][0]=Hero->arInventory[2][0];
									arLootTable[Free][1]=Hero->arInventory[2][1];
									draw_sprite(bmpWindow,(pFirstItem+arLootTable[Free][0])->Pic,24+(Free%3)*60,237+(Free-Free%3)/3*60);
								};
								Hero->arInventory[2][1]=0;
								Hero->arInventory[2][0]=0;
							};
							//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
							if ((Click==2)&&(Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100))
							{
								int Free=GetFreeSlot(Hero,0);
								if (Free>0)
								{
									Hero->arInventory[Free][0]=Hero->arInventory[1][0];
									Hero->arInventory[Free][1]=Hero->arInventory[1][1];
								}
								else
								{
									for (int i=0; i<9; i++)
									{
										if (arLootTable[i][0]==0)
										{
											Free=i;
											break;
										};
									};
									arLootTable[Free][0]=Hero->arInventory[1][0];
									arLootTable[Free][1]=Hero->arInventory[1][1];
									draw_sprite(bmpWindow,(pFirstItem+arLootTable[Free][0])->Pic,24+(Free%3)*60,237+(Free-Free%3)/3*60);
								};
								Hero->arInventory[1][1]=0;
								Hero->arInventory[1][0]=0;
							};
							SelectedItem=0;
							SelectedSlot=0;
							blItem.Show();								
							set_mouse_sprite(bmpMousePointer[0]);							
						};
					}
					else
					{	
	//Свободный слот
						Hero->arInventory[Click][0]=SelectedItem;
						Hero->arInventory[Click][1]=SelectedItemDur;	
						//Перемещение щита в инвентарь, если получающий герой получил двуручный предмет
						if (((pFirstItem+SelectedItem)->ipSlot==100)&&(Hero->arInventory[2][0]>0)&&(Click==1))
						{
							int Free=GetFreeSlot(Hero,0);
							if (Free>0)
							{
								Hero->arInventory[Free][0]=Hero->arInventory[2][0];
								Hero->arInventory[Free][1]=Hero->arInventory[2][1];
							}
							else
							{
								for (int i=0; i<9; i++)
								{
									if (arLootTable[i][0]==0)
									{
										Free=i;
										break;
									};
								};
								arLootTable[Free][0]=Hero->arInventory[2][0];
								arLootTable[Free][1]=Hero->arInventory[2][1];
								draw_sprite(bmpWindow,(pFirstItem+arLootTable[Free][0])->Pic,24+(Free%3)*60,237+(Free-Free%3)/3*60);
							};
							Hero->arInventory[2][1]=0;
							Hero->arInventory[2][0]=0;
						};
						//Перемещение двуручного оружия в инвентарь, если принимающий герой получил щит
						if ((Click==2)&&(Hero->arInventory[1][0]>0)&&((pFirstItem+Hero->arInventory[1][0])->ipSlot==100))
						{
							int Free=GetFreeSlot(Hero,0);
							if (Free>0)
							{
								Hero->arInventory[Free][0]=Hero->arInventory[1][0];
								Hero->arInventory[Free][1]=Hero->arInventory[1][1];
							}
							else
							{
								for (int i=0; i<9; i++)
								{
									if (arLootTable[i][0]==0)
									{
										Free=i;
										break;
									};
								};
								arLootTable[Free][0]=Hero->arInventory[1][0];
								arLootTable[Free][1]=Hero->arInventory[1][1];
								draw_sprite(bmpWindow,(pFirstItem+arLootTable[Free][0])->Pic,24+(Free%3)*60,237+(Free-Free%3)/3*60);
							};
							Hero->arInventory[1][1]=0;
							Hero->arInventory[1][0]=0;
						};
						SelectedItem=0;
						SelectedSlot=0;
						blItem.Show();						
						set_mouse_sprite(bmpMousePointer[0]);						
					};
				};
			}
			else
			{
	//Взятие предмета
				if (Hero->arInventory[Click][0]>0)
				{					
					SelectedItem=Hero->arInventory[Click][0];
					SelectedItemDur=Hero->arInventory[Click][1];
					SelectedSlot=Click;					
					Hero->arInventory[Click][0]=0;
					Hero->arInventory[Click][1]=0;
					blItem.Show();					
					rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
					draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
					draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
					set_mouse_sprite(bmpMouse);					
				};
			};			
		};

		for (int Itm=0; Itm<9; Itm++)
		{
			int CH=mskItem[Itm].Check();
			if (CH>1)
			{
				if (arLootTable[Itm][0]>0)
				{
					MakeItemHint(arLootTable[Itm][0],arLootTable[Itm][1],Hero,0,0,0);
					bHint=true;
				};
			};
			if (CH==30)
			{
				if (SelectedItem>0)
				{
					if (arLootTable[Itm][0]==0)
					{
						arLootTable[Itm][0]=SelectedItem;
						arLootTable[Itm][1]=SelectedItemDur;
						draw_sprite(bmpWindow,(pFirstItem+SelectedItem)->Pic,24+(Itm%3)*60,237+(Itm-Itm%3)/3*60);
						SelectedItem=0;
						SelectedSlot=0;						
						set_mouse_sprite(bmpMousePointer[0]);
					};
				}
				else
				{
					if (arLootTable[Itm][0]>0)
					{
						SelectedItem=arLootTable[Itm][0];
						SelectedItemDur=arLootTable[Itm][1];
						SelectedSlot=200+Itm;					
						arLootTable[Itm][0]=0;
						arLootTable[Itm][0]=0;
						blit(bmpGUI_TakeLoot,bmpWindow,24+(Itm%3)*60,237+(Itm-Itm%3)/3*60,24+(Itm%3)*60,237+(Itm-Itm%3)/3*60,52,52);
						rectfill(bmpMouse,0,0,bmpMouse->w,bmpMouse->h,makecol(255,0,255));
						draw_sprite(bmpMouse,(pFirstItem+SelectedItem)->Pic,0,0);
						draw_sprite(bmpMouse,bmpGUI_Pointer,0,0);
						set_mouse_sprite(bmpMouse);
					};
				};
			};
			if (CH==40)
			{
				if (arLootTable[Itm][0]>0)
				{
					bHint=false;
					WindowItemInfo(arLootTable[Itm][0],arLootTable[Itm][1]);
				};
			};
		};
		
		if (Timer)
		{
			Timer=false;
			WObj=ShowActiveObjects(WObj,10,true);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};

	if ((SelectedItem)&&(SelectedSlot<90))
	{
		Hero->arInventory[SelectedSlot][0]=SelectedItem;
		Hero->arInventory[SelectedSlot][1]=SelectedItemDur;
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
	destroy_bitmap(bmpMouse);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);	
	set_mouse_sprite(OldMouse);	
	bHint=false;
	HintType=0;
	iAntisleep=iAntisleepDelay;
};

int arPicked[16];

int WindowResChoose(int Player, int Amount)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpStuff[18]->w/2;
	int y=384-bmpStuff[18]->h/2;	
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpStuff[18]->w,bmpStuff[18]->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpStuff[18],bmpWindow,0,0,0,0,bmpStuff[18]->w,bmpStuff[18]->h);
	int Result=0;
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+521,y+36,1,2,bmpButton[186],bmpButton[187],bmpButton[188],bmpButton[189],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;
	textprintf_centre_ex(bmpWindow,Font1,311,46,COLOR_1,-1,"%s",(pFirstPhrase+167)->str);
	UI_Mask mskRes[9];
	int Count=0;
	int arRes[9];
	for (int i=1; i<=9; i++)
	{
		int Add=0;
		for (int j=0; j<16; j++)
		{
			if (arPicked[j]==i)
			{Add++;};
		};
		if (arPlayer[Player].GetResource(i)>(Amount+Add))
		{
			arRes[Count]=i;
			draw_sprite(bmpWindow,(pFirstResource+i)->BigPic,44+Count*60,77);
			WObj=mskRes[Count].Init(x+38+Count*60,y+71,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
			textprintf_centre_ex(bmpWindow,Font1,70+Count*60,136,COLOR_2,-1,"%d",arPlayer[Player].GetResource(i)-Amount-Add);
			Count++;
		};
	};
	while ((Result==0)&&(!buttExit.Check()))
	{
		for (int i=0; i<Count; i++)
		{
			int Check=mskRes[i].Check();
			if (Check==30)
			{Result=arRes[i];};
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
	iAntisleep=iAntisleepDelay;
	return Result;
};

int WindowItemChoose(int Player)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpStuff[19]->w/2;
	int y=384-bmpStuff[19]->h/2;	
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpStuff[19]->w,bmpStuff[19]->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);
	blit(bmpStuff[19],bmpWindow,0,0,0,0,bmpStuff[19]->w,bmpStuff[19]->h);
	int Result=-1;
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+521,y+36,1,2,bmpButton[186],bmpButton[187],bmpButton[188],bmpButton[189],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;
	textprintf_centre_ex(bmpWindow,Font1,311,46,COLOR_1,-1,"%s",(pFirstPhrase+170)->str);
	UI_Mask mskItem[36];
//	int Count=0;
//	int arRes[9];
	for (int i=0; i<36; i++)
	{
		bool bShow=false;
		if (arPlayer[Player].arTreasury[i][0]>0)
		{
			bShow=true;
			for (int j=8; j<16; j++)
			{
				if (arPicked[j]==i)
				{bShow=false;};
			};
			if (bShow)
			{			
				draw_sprite(bmpWindow,(pFirstItem+arPlayer[Player].arTreasury[i][0])->Pic,44+(i%9)*60,77+((i-i%9)/9)*60);
				WObj=mskItem[i].Init(x+38+(i%9)*60,y+71+((i-i%9)/9)*60,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,2);
			};
		};
		if (!bShow)
		{WObj=mskItem[i].Init(x+38+(i%9)*60,y+71+((i-i%9)/9)*60,0,bmpGUI_SpellMask,bmpGUI_SpellSelect,WObj,-1);};
	};
	while ((Result==-1)&&(!buttExit.Check()))
	{
		for (int i=0; i<36; i++)
		{
			int Check=mskItem[i].Check();
			if (Check==30)
			{Result=i;}
			else
			{
				if (Check>1)
				{
					MakeItemHint(arPlayer[Player].arTreasury[i][0],arPlayer[Player].arTreasury[i][1],NULL,312,(pFirstItem+arPlayer[Player].arTreasury[i][0])->iGoldCost,(pFirstItem+arPlayer[Player].arTreasury[i][0])->iGemCost);
					bHint=true;
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
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	iAntisleep=iAntisleepDelay;
	return Result;
};

int WindowProvChoose(int Player, int Player2)
{
	iAntisleep=iAntisleepDelay;
	int x=512-bmpStuff[20]->w/2;
	int y=384-bmpStuff[20]->h/2;	
	set_mouse_sprite(bmpMousePointer[0]);
	BITMAP *bmpBack=create_bitmap(1024,768);
	blit(screen1,bmpBack,0,0,0,0,1024,768);
	rectfill(bmpDarkScreen,0,0,1024,768,COLOR_1);
	draw_trans_sprite(bmpDarkScreen,bmpBack,0,0);
	G_Object * WObj=new G_Object(NULL,0,0,0,bmpDarkScreen);
	BITMAP *bmpWindow=create_bitmap(bmpStuff[20]->w,bmpStuff[20]->h);
	WObj=new G_Object(WObj,x,y,1,bmpWindow);	
	int Result=-1;
	UI_Button1 buttExit;
	WObj=buttExit.Init(x+98,y+281,1,2,bmpButton[186],bmpButton[187],bmpButton[188],bmpButton[189],WObj);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;
	UI_Button1 buttScrollUp;
	WObj=buttScrollUp.Init(x+26,y+281,1,2,bmpButton[190],bmpButton[191],bmpButton[192],bmpButton[193],WObj);
	buttScrollUp.Activate(0);
	buttScrollUp.ipHotkey=KEY_PGUP;
	UI_Button1 buttScrollDown;
	WObj=buttScrollDown.Init(x+170,y+281,1,2,bmpButton[194],bmpButton[195],bmpButton[196],bmpButton[197],WObj);
	buttScrollDown.Activate(0);
	buttScrollDown.ipHotkey=KEY_PGDN;
	textprintf_centre_ex(bmpWindow,Font1,311,46,COLOR_1,-1,"%s",(pFirstPhrase+170)->str);
	UI_Mask mskProv[9];
	BITMAP *bmpProv[9];
	int arProv[9];
	for (int i=0; i<9; i++)
	{
		bmpProv[i]=create_bitmap(155,21);
		WObj=mskProv[i].Init(x+25,y+45+i*21,1,bmpProv[i],bmpProv[i],WObj,2);
	};
	int Count;
	int Shift=0;
	bool bRepeat=true;
	while (bRepeat)
	{
	Count=0;
	for (int i=0; i<9; i++)
	{
		mskProv[i].Activate(-1);
	};
	blit(bmpStuff[20],bmpWindow,0,0,0,0,bmpStuff[20]->w,bmpStuff[20]->h);
	for (int i=1; i<=MapSize; i++)
	{
		bool bShow=false;
		if (((pFirstProvince+i)->ipPlayer==Player)&&(arPlayer[Player].ipProvince!=i)&&((pFirstProvince+i)->GetSiegeIndex()<0))
		{bShow=true;};
		if ((Player2>0)&&(*(arPlayer[Player2].pFog+i)>1))
		{bShow=false;};
		for (int j=0; j<8; j++)
		{
			if (arPicked[j]==i)
			{bShow=false;};
		};
		if (bShow)
		{
			if ((Count-Count%9)/9==Shift)
			{
				textprintf_ex(bmpWindow,Font1,27,50+(Count%9)*21,COLOR_1,-1,"%s",(pFirstProvince+i)->sName);
				draw_sprite(bmpWindow,(pFirstRace+(pFirstProvince+i)->ipRace)->SmallPic,180,45+(Count%9)*21);
				if ((pFirstProvince+i)->GetResource(0)>0)
				{
					draw_sprite(bmpWindow,(pFirstResource+(pFirstProvince+i)->GetResource(0))->Pic,201,45+(Count%9)*21);
				};
				mskProv[Count%9].Activate(1);
				rectfill(bmpProv[Count%9],0,0,155,21,COLOR_1);
				textprintf_ex(bmpProv[Count%9],Font1,2,5,COLOR_2,-1,"%s",(pFirstProvince+i)->sName);
				arProv[Count%9]=i;
			};
			Count++;
		};		
	};
	if (Shift>0)
	{buttScrollUp.Activate(1);}
	else
	{buttScrollUp.Activate(0);};
	if ((Shift+1)*9<Count)
	{buttScrollDown.Activate(1);}
	else
	{buttScrollDown.Activate(0);};
	bool bRedraw=true;
	while ((bRepeat)&&(bRedraw))
	{
		if (buttExit.Check())
		{bRepeat=false;};
		for (int i=0; i<9; i++)
		{
			int Check=mskProv[i].Check();
			if (Check==30)
			{
				Result=arProv[i];
				bRepeat=false;
			};
		};
		if (buttScrollUp.Check())
		{
			Shift--;
			bRedraw=false;
		};
		if (buttScrollDown.Check())
		{
			Shift++;
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
	blit(bmpBack,bmpDarkScreen,0,0,0,0,1024,768);
	destroy_bitmap(bmpBack);
	destroy_bitmap(bmpWindow);
	for (int i=0; i<9; i++)
	{destroy_bitmap(bmpProv[i]);};
	iAntisleep=iAntisleepDelay;
	return Result;
};

void ScreenDiplomacy(int Player)
{
	iAntisleep=iAntisleepDelay;
	BITMAP *bmpScr=create_bitmap(1024,768);	
	blit(bmpGUI_Diplomacy,bmpScr,0,0,0,0,1024,768);
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpScr);
	UI_Button1 buttExit;
	objGUI=buttExit.Init(935,670,1,2,bmpButton[186],bmpButton[187],bmpButton[188],bmpButton[189],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;
	UI_Button1 buttMessage;
	objGUI=buttMessage.Init(480,670,1,2,bmpButton[182],bmpButton[183],bmpButton[184],bmpButton[185],objGUI);
	buttMessage.Activate(-1);
	buttMessage.iHint=154;
	

	UI_Button1 buttTrade;
	objGUI=buttTrade.Init(31,670,1,2,bmpButton[198],bmpButton[199],bmpButton[200],bmpButton[201],objGUI);
	buttTrade.Activate(-1);
	buttTrade.iHint=163;
	UI_Button1 buttExchange;
	objGUI=buttExchange.Init(97,670,1,2,bmpButton[202],bmpButton[203],bmpButton[204],bmpButton[205],objGUI);
	buttExchange.Activate(-1);
	buttExchange.iHint=164;
	UI_Button1 buttSkipTrade;
	objGUI=buttSkipTrade.Init(163,670,1,2,bmpButton[206],bmpButton[207],bmpButton[208],bmpButton[209],objGUI);
	buttSkipTrade.Activate(-1);
	buttSkipTrade.iHint=176;

	UI_Button1 buttWar;
	objGUI=buttWar.Init(262,670,1,2,bmpButton[210],bmpButton[211],bmpButton[212],bmpButton[213],objGUI);
	buttWar.Activate(-1);
	buttWar.iHint=161;
	UI_Button1 buttPeace;
	objGUI=buttPeace.Init(328,670,1,2,bmpButton[214],bmpButton[215],bmpButton[216],bmpButton[217],objGUI);
	buttPeace.Activate(-1);
	buttPeace.iHint=160;
	UI_Button1 buttUnion;
	objGUI=buttUnion.Init(328,670,1,2,bmpButton[218],bmpButton[219],bmpButton[220],bmpButton[221],objGUI);
	buttUnion.Activate(-1);
	buttUnion.iHint=162;
	UI_Button1 buttSkipUnion;
	objGUI=buttSkipUnion.Init(328,670,1,2,bmpButton[222],bmpButton[223],bmpButton[224],bmpButton[225],objGUI);
	buttSkipUnion.Activate(-1);
	buttSkipUnion.iHint=177;

	UI_Button1 buttCancelTreaty;
	objGUI=buttCancelTreaty.Init(328,670,1,2,bmpButton[186],bmpButton[187],bmpButton[188],bmpButton[189],objGUI);
	buttCancelTreaty.Activate(-1);
	buttCancelTreaty.iHint=178;
	UI_Button1 buttCancelTrade;
	objGUI=buttCancelTrade.Init(328,670,1,2,bmpButton[186],bmpButton[187],bmpButton[188],bmpButton[189],objGUI);
	buttCancelTrade.Activate(-1);
	buttCancelTrade.iHint=179;

	int TradeTime, PeaceTime, PeaceType, UnionTime;	//Флаги ограничителей договоров по времени


	objGUI=new G_Object(objGUI,620,20,-1,bmpGUI_Hint);
	G_Object *objTutorial=objGUI;
	UI_Mask mskTutorial;
	objGUI=mskTutorial.Init(620,20,1,bmpGUI_Hint,bmpEmpty,objGUI,-1);
	UI_TextBox txtTutorial;
	objGUI=txtTutorial.Init(640,50,297,200,5,COLOR_1,Font2,objGUI);
	txtTutorial.bShadow=false;
	txtTutorial.Hide();
	bool bTutHint=false;
	int TutNext=0;


	UI_Mask mskPlayer[15];

//	buttMessage.ipHotkey=KEY_ESC;
	
	set_mouse_sprite(bmpMousePointer[0]);
	textprintf_centre_ex(bmpScr,Font1,126,725,COLOR_2,-1,"%d",arPlayer[Player].Gold);
	textprintf_centre_ex(bmpScr,Font1,295,725,COLOR_2,-1,"%d",arPlayer[Player].Gem);
	for (int i=0; i<9; i++)
	{textprintf_centre_ex(bmpScr,Font1,410+i*73,743,COLOR_2,-1,"%d",arPlayer[Player].GetResource(i+1));};
	int Count=0;
	int arOpp[15];
	//Вывод портретов оппонентов
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if ((Player!=i)&&(arPlayer[Player].Politic[i].Treaty>0)&&(arPlayer[i].Control<3))
		{
			draw_sprite(bmpScr,arPlayer[i].GetSmallAvatar(),21+Count*66,19);
			draw_sprite(bmpScr,bmpFlag[arPlayer[i].iColor][1],-12+Count*66,86);
			if (arPlayer[i].bInGame)
			{
				switch (arPlayer[Player].Politic[i].Treaty)
				{
				case 1:
					draw_sprite(bmpScr,bmpStuff[10],22+Count*66,78);
					break;
				case 2:
					draw_sprite(bmpScr,bmpStuff[9],22+Count*66,78);
					break;
				case 3:
					draw_sprite(bmpScr,bmpStuff[11],22+Count*66,78);
					break;
				};
				if (arPlayer[Player].Politic[i].Trade.bTrade)
				{draw_sprite(bmpScr,bmpStuff[12],50+Count*66,78);};
			}
			else
			{draw_sprite(bmpScr,bmpGUI_SpellShadow,21+Count*66,19);};
			objGUI=mskPlayer[Count].Init(15+Count*66,13,1,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);
			arOpp[Count]=i;
			Count++;
		};
	};
	blit(bmpGUI_Diplomacy,bmpScr,0,122,0,122,1024,20);
	
	UI_Mask mskGive[18], mskTake[18];
	for (int j=0; j<16; j++)
	{
		objGUI=mskGive[j].Init(27+(j%8)*60,539+((j-j%8)/8)*60,0,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);
		objGUI=mskTake[j].Init(508+(j%8)*60,539+((j-j%8)/8)*60,0,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);
	};
	objGUI=mskGive[16].Init(75,438,0,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);
	objGUI=mskGive[17].Init(398,438,0,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);
	objGUI=mskTake[16].Init(556,438,0,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);
	objGUI=mskTake[17].Init(879,438,0,bmpGUI_SpellMask,bmpGUI_SpellSelect,objGUI,2);
	UI_Mask1 mskInput[4];
	mskInput[0].Init(50,500,115,30,1);
	mskInput[1].Init(372,500,115,30,1);
	mskInput[2].Init(530,500,115,30,1);
	mskInput[3].Init(852,500,115,30,1);

	UI_Input inpGoldGive;
	objGUI=inpGoldGive.Init(87,503,1,4,2,COLOR_2,Font1,objGUI);
	inpGoldGive.bCenter=true;
	UI_Input inpGemGive;
	objGUI=inpGemGive.Init(411,503,1,4,2,COLOR_2,Font1,objGUI);
	inpGemGive.bCenter=true;
	UI_Input inpGoldTake;
	objGUI=inpGoldTake.Init(569,503,1,4,2,COLOR_2,Font1,objGUI);
	inpGoldTake.bCenter=true;
	UI_Input inpGemTake;
	objGUI=inpGemTake.Init(891,503,1,4,2,COLOR_2,Font1,objGUI);
	inpGemTake.bCenter=true;
	UI_ScrollBar scrGoldGive;
	objGUI=scrGoldGive.Init(50,530,101,7,4,false,true,objGUI);
	scrGoldGive.Activate(-1);
	UI_ScrollBar scrGemGive;
	objGUI=scrGemGive.Init(373,530,101,7,4,false,true,objGUI);
	scrGemGive.Activate(-1);
	UI_ScrollBar scrGoldTake;
	objGUI=scrGoldTake.Init(531,530,101,7,4,false,true,objGUI);
	scrGoldTake.Activate(-1);
	UI_ScrollBar scrGemTake;
	objGUI=scrGemTake.Init(854,530,101,7,4,false,true,objGUI);
	scrGemTake.Activate(-1);

	int arGive[18], arTake[18];

	int Mode=0, //Режим экрана
				//0-нет договоров
				//1-выбор договора и торговли
				//2-договор установлен, выбор торговли
				//3-объявление войны
				//4-торговля, договора нет
				//5-обмен, договора нет
				//6-договор и торговля
				//7-договор и обмен
		Oppo=0, iTreaty=0, CloseInput=-1;
	bool bRepeat=true, bShowTrade=false, bShowDeal=false, bPickDeal=false, 
		bNormalizeButtons=false, bMessage=false, bCanExit=true;
	int Check=0;


	while (bRepeat)
	{		
	
		if ((!key[KEY_ESC])&&(buttExit.ipHotkey==-100))
		{buttExit.ipHotkey=KEY_ESC;};
		if (Tutorial.Diplomacy==1)
		{
			bTutHint=true;
			ScrollSound();
			txtTutorial.Activate((pFirstTutText+101)->msg);
			objTutorial->iLayer=2;
			Tutorial.Diplomacy=2;
		};
		//Вывод информации о текущей торговле
		if (bShowTrade)
		{
			blit(bmpGUI_Diplomacy,bmpScr,0,360,0,360,1024,310);
			if (arPlayer[Player].Politic[Oppo].Trade.bTrade)
			{
				TradeTime=0;
				if ((arPlayer[Player].Politic[Oppo].arTalk[5][1]>0)&&(CurGame.iCurrentTurn-arPlayer[Player].Politic[Oppo].arTalk[5][2]<11))
				{TradeTime=11-CurGame.iCurrentTurn+arPlayer[Player].Politic[Oppo].arTalk[5][2];};
				if ((arPlayer[Oppo].Politic[Player].arTalk[5][1])&&(11-CurGame.iCurrentTurn+arPlayer[Oppo].Politic[Player].arTalk[5][2]>TradeTime))
				{TradeTime=11-CurGame.iCurrentTurn+arPlayer[Oppo].Politic[Player].arTalk[5][2];};
				draw_sprite(bmpScr,bmpStuff[17],11,406);
				if (TradeTime>0)
				{textprintf_centre_ex(bmpScr,Font1,512,414,COLOR_1,-1,"%s (%d)",(pFirstPhrase+183)->str,TradeTime);}
				else
				{textprintf_centre_ex(bmpScr,Font1,512,414,COLOR_1,-1,"%s",(pFirstPhrase+183)->str);};

				textprintf_centre_ex(bmpScr,Font1,267,450,COLOR_1,-1,"%s",(pFirstPhrase+165)->str);
				textprintf_centre_ex(bmpScr,Font1,755,450,COLOR_1,-1,"%s",(pFirstPhrase+166)->str);
				for (int j=0; j<16; j++)
				{
					if (arPlayer[Player].Politic[Oppo].Trade.Stuff[j]>0)
					{draw_sprite(bmpScr,(pFirstResource+arPlayer[Player].Politic[Oppo].Trade.Stuff[j])->BigPic,33+(j%8)*60,545+((j-j%8)/8)*60);}
					else
					{draw_sprite(bmpScr,bmpStuff[13],33+(j%8)*60,545+((j-j%8)/8)*60);};
					if (arPlayer[Oppo].Politic[Player].Trade.Stuff[j]>0)
					{draw_sprite(bmpScr,(pFirstResource+arPlayer[Oppo].Politic[Player].Trade.Stuff[j])->BigPic,514+(j%8)*60,545+((j-j%8)/8)*60);}
					else
					{draw_sprite(bmpScr,bmpStuff[13],514+(j%8)*60,545+((j-j%8)/8)*60);};
				};
				if (arPlayer[Player].Politic[Oppo].Trade.Gold>0)
				{
					draw_sprite(bmpScr,bmpStuff[5],81,444);
					textprintf_centre_ex(bmpScr,Font1,107,509,COLOR_2,-1,"%d",arPlayer[Player].Politic[Oppo].Trade.Gold);
				};
				if (arPlayer[Player].Politic[Oppo].Trade.Gem>0)
				{
					draw_sprite(bmpScr,bmpStuff[6],404,444);
					textprintf_centre_ex(bmpScr,Font1,430,509,COLOR_2,-1,"%d",arPlayer[Player].Politic[Oppo].Trade.Gem);
				};
				if (arPlayer[Oppo].Politic[Player].Trade.Gold>0)
				{
					draw_sprite(bmpScr,bmpStuff[5],562,444);
					textprintf_centre_ex(bmpScr,Font1,588,509,COLOR_2,-1,"%d",arPlayer[Oppo].Politic[Player].Trade.Gold);
				};
				if (arPlayer[Oppo].Politic[Player].Trade.Gem>0)
				{
					draw_sprite(bmpScr,bmpStuff[6],885,444);
					textprintf_centre_ex(bmpScr,Font1,911,509,COLOR_2,-1,"%d",arPlayer[Oppo].Politic[Player].Trade.Gem);
				};
			}
			else
			{
				draw_sprite(bmpScr,bmpStuff[16],30,429);
				textprintf_centre_ex(bmpScr,Font1,512,450,COLOR_2,-1,"%s",(pFirstPhrase+171)->str);
			};			
			if (arPlayer[Player].Politic[Oppo].Message.Type>0)
			{
				textprintf_centre_ex(bmpScr,Font1,512,382,COLOR_2,-1,"%s",(pFirstPhrase+172)->str);				
			}
			else
			{
				textprintf_centre_ex(bmpScr,Font1,512,382,COLOR_2,-1,"%s",(pFirstPhrase+173)->str);				
			};
			bNormalizeButtons=true;
			bShowTrade=false;
		};

		//Активизация договора для редактирования
		if (bPickDeal)
		{
			Mode=arPlayer[Player].Politic[Oppo].Message.Type;
			for (int i=0; i<18; i++)
			{
				arGive[i]=0;
				arTake[i]=0;
			};
			iTreaty=arPlayer[Player].Politic[Oppo].Message.Treaty;
			if ((Mode==2)||(Mode==3))
			{
				arGive[16]=arPlayer[Player].Politic[Oppo].Message.Trade.Gold;
				if (arGive[16]>0)
				{inpGoldGive.EnterInt(arGive[16]);}
				else
				{inpGoldGive.Clear();};				
				arGive[17]=arPlayer[Player].Politic[Oppo].Message.Trade.Gem;
				if (arGive[17]>0)
				{inpGemGive.EnterInt(arGive[17]);}
				else
				{inpGemGive.Clear();};
				arTake[16]=arPlayer[Player].Politic[Oppo].Message.Trade1.Gold;
				if (arTake[16]>0)
				{inpGoldTake.EnterInt(arTake[16]);}
				else
				{inpGemTake.Clear();};
				arTake[17]=arPlayer[Player].Politic[Oppo].Message.Trade1.Gem;
				if (arTake[17]>0)
				{inpGemTake.EnterInt(arTake[17]);}
				else
				{inpGemTake.Clear();};
			};
			if (arPlayer[Player].Politic[Oppo].Message.Type==2)
			{
				scrGoldGive.AttachValue(&arGive[16],0,arPlayer[Player].Gold+arPlayer[Player].GoldInc);
				scrGemGive.AttachValue(&arGive[17],0,arPlayer[Player].Gem+arPlayer[Player].GemInc);
				scrGoldTake.AttachValue(&arTake[16],0,9999);
				scrGemTake.AttachValue(&arTake[17],0,9999);
				for (int j=0; j<16; j++)
				{
					arGive[j]=arPlayer[Player].Politic[Oppo].Message.Trade.Stuff[j];
					arTake[j]=arPlayer[Player].Politic[Oppo].Message.Trade1.Stuff[j];					
				};
			};
			if (Mode==3)
			{
				scrGoldGive.AttachValue(&arGive[16],0,arPlayer[Player].Gold);
				scrGemGive.AttachValue(&arGive[17],0,arPlayer[Player].Gem);
				scrGoldTake.AttachValue(&arTake[16],0,9999);
				scrGemTake.AttachValue(&arTake[17],0,9999);
				for (int j=0; j<8; j++)
				{
					arGive[j]=arPlayer[Player].Politic[Oppo].Message.Trade.Stuff[j];
					arTake[j]=arPlayer[Player].Politic[Oppo].Message.Trade1.Stuff[j];					
				};
				for (int j=8; j<16; j++)
				{
					if (arPlayer[Player].Politic[Oppo].Message.Trade.Stuff[j]>0)
					{
						for (int i=0; i<TREASURY_SIZE; i++)
						{
							if ((arPlayer[Player].arTreasury[i][0]==arPlayer[Player].Politic[Oppo].Message.Trade.Stuff[j])&&
								(arPlayer[Player].arTreasury[i][1]==arPlayer[Player].Politic[Oppo].Message.arDurability[j-8]))
							{
								arGive[j]=i;
							};
						};
					}
					else
					{
						arGive[j]=-1;
					};
					arTake[j]=0;
				};				
			};
			bNormalizeButtons=true;
			bPickDeal=false;
		};

		//Вывод информации о текущем договоре
		if (bShowDeal)
		{
			blit(bmpGUI_Diplomacy,bmpScr,0,360,0,360,1024,310);
			if (Mode>0)
			{
				if ((Mode==1)||(Mode==4)||(Mode==5))
				{
					draw_sprite(bmpScr,bmpStuff[16],30,429);
					if (Mode==4)
					{
						textprintf_centre_ex(bmpScr,Font1,513,383,COLOR_1,-1,"%s",(pFirstPhrase+176)->str);
						textprintf_centre_ex(bmpScr,Font1,512,382,makecol(250,20,20),-1,"%s",(pFirstPhrase+176)->str);
					};
				}
				else
				{
					draw_sprite(bmpScr,bmpStuff[17],11,406);					
					textprintf_centre_ex(bmpScr,Font1,267,450,COLOR_1,-1,"%s",(pFirstPhrase+165)->str);
					textprintf_centre_ex(bmpScr,Font1,755,450,COLOR_1,-1,"%s",(pFirstPhrase+166)->str);
					if (Mode==2)
					{
						textprintf_centre_ex(bmpScr,Font1,512,414,COLOR_1,-1,"%s",(pFirstPhrase+181)->str);
						for (int j=0; j<16; j++)
						{
							if (arGive[j]>0)
							{draw_sprite(bmpScr,(pFirstResource+arGive[j])->BigPic,33+(j%8)*60,545+((j-j%8)/8)*60);}
							else
							{draw_sprite(bmpScr,bmpStuff[13],33+(j%8)*60,545+((j-j%8)/8)*60);};
							if (arTake[j]>0)
							{draw_sprite(bmpScr,(pFirstResource+arTake[j])->BigPic,514+(j%8)*60,545+((j-j%8)/8)*60);}
							else
							{draw_sprite(bmpScr,bmpStuff[13],514+(j%8)*60,545+((j-j%8)/8)*60);};
						};
					}
					else
					{
						textprintf_centre_ex(bmpScr,Font1,512,414,COLOR_1,-1,"%s",(pFirstPhrase+182)->str);
						for (int j=0; j<8; j++)
						{
							if (arGive[j]>0)
							{draw_sprite(bmpScr,bmpStuff[8],33+(j%8)*60,545+((j-j%8)/8)*60);}
							else
							{draw_sprite(bmpScr,bmpStuff[15],33+(j%8)*60,545+((j-j%8)/8)*60);};
							if (arTake[j]>0)
							{draw_sprite(bmpScr,bmpStuff[8],514+(j%8)*60,545+((j-j%8)/8)*60);}
							else
							{draw_sprite(bmpScr,bmpStuff[15],514+(j%8)*60,545+((j-j%8)/8)*60);};							
						};
						for (int j=8; j<16; j++)
						{
							if (arGive[j]>=0)
							{draw_sprite(bmpScr,(pFirstItem+arPlayer[Player].arTreasury[arGive[j]][0])->Pic,33+(j%8)*60,545+((j-j%8)/8)*60);}
							else
							{
								draw_sprite(bmpScr,bmpStuff[14],33+(j%8)*60,545+((j-j%8)/8)*60);								
							};
						};
					};
					if (arGive[16]>0)
					{
						draw_sprite(bmpScr,bmpStuff[5],81,444);
			//			textprintf_centre_ex(bmpScr,Font1,107,506,COLOR_1,-1,"%d",arGive[16]);
					};
					if (arGive[17]>0)
					{
						draw_sprite(bmpScr,bmpStuff[6],404,444);
			//			textprintf_centre_ex(bmpScr,Font1,430,506,COLOR_1,-1,"%d",arGive[17]);
					};
					if (arTake[16]>0)
					{
						draw_sprite(bmpScr,bmpStuff[5],562,444);
			//			textprintf_centre_ex(bmpScr,Font1,588,506,COLOR_1,-1,"%d",arTake[16]);
					};
					if (arTake[17]>0)
					{
						draw_sprite(bmpScr,bmpStuff[6],885,444);
			//			textprintf_centre_ex(bmpScr,Font1,911,506,COLOR_1,-1,"%d",arTake[17]);
					};
				};
				switch (iTreaty)
				{				
				case 1:
					textprintf_centre_ex(bmpScr,Font1,513,383,COLOR_1,-1,"%s",(pFirstPhrase+161)->str);
					textprintf_centre_ex(bmpScr,Font1,512,382,makecol(250,20,20),-1,"%s",(pFirstPhrase+161)->str);
					break;
				case 2:
					textprintf_centre_ex(bmpScr,Font1,512,382,COLOR_2,-1,"%s",(pFirstPhrase+160)->str);
				break;
				case 3:
					textprintf_centre_ex(bmpScr,Font1,512,382,COLOR_2,-1,"%s",(pFirstPhrase+162)->str);
					break;
				case 4:
					textprintf_centre_ex(bmpScr,Font1,513,383,COLOR_1,-1,"%s",(pFirstPhrase+177)->str);
					textprintf_centre_ex(bmpScr,Font1,512,382,makecol(250,20,20),-1,"%s",(pFirstPhrase+177)->str);
					break;
				};
			};
	//		StopLog("Show   ",1);
			bNormalizeButtons=true;
			bShowDeal=false;
		};

		//Приведение в порядок кнопок
		if (bNormalizeButtons)
		{
			//Режим отсутствия послания
			if (Mode==-1)
			{			
				buttTrade.Activate(-1);
				buttExchange.Activate(-1);
				buttSkipTrade.Activate(-1);
				buttWar.Activate(-1);
				buttPeace.Activate(-1);
				buttUnion.Activate(-1);
				buttSkipUnion.Activate(-1);
				buttCancelTrade.Activate(-1);
				buttCancelTreaty.Activate(-1);
				if (arPlayer[Oppo].bInGame)
				{
					buttMessage.Activate(1);
					buttMessage.iHint=180;
				};
			}
			else
			{
				if (Mode==0)
				{
					if (arPlayer[Player].Politic[Oppo].Message.Type>0)
					{buttMessage.Activate(1);}
					else
					{buttMessage.Activate(0);}
					buttMessage.iHint=168;
				}
				else
				{
					buttMessage.Activate(1);
					buttMessage.iHint=169;
				};
				switch (iTreaty)
				//Кнопки договоров
				{
				case 0:
					buttCancelTreaty.Activate(-1);
					switch (arPlayer[Player].Politic[Oppo].Treaty)
					{
					case 1:				
						buttWar.Activate(-1);						
						buttPeace.Activate(1);
						buttUnion.Activate(-1);
						buttSkipUnion.Activate(-1);
						break;
					case 2:				
						buttWar.Activate(1);
						buttPeace.Activate(-1);
						if (Mode==4)
						{buttUnion.Activate(0);}
						else
						{buttUnion.Activate(1);};
						buttSkipUnion.Activate(-1);
						break;
					case 3:				
						buttWar.Activate(-1);
						buttPeace.Activate(-1);
						buttUnion.Activate(-1);
						buttSkipUnion.Activate(1);
						break;
					};
					break;
				case 1:
					buttCancelTreaty.G_Obj->x=262;
					buttCancelTreaty.Activate(1);
					buttWar.Activate(-1);
					buttUnion.Activate(0);
					buttPeace.Activate(-1);
					buttSkipUnion.Activate(-1);					
					break;
				case 2:
				case 4:
					buttCancelTreaty.G_Obj->x=328;
					buttCancelTreaty.Activate(1);
					buttWar.Activate(-1);
					buttUnion.Activate(-1);
					buttPeace.Activate(-1);
					buttSkipUnion.Activate(-1);					
				break;
				case 3:
					buttCancelTreaty.G_Obj->x=328;
					buttCancelTreaty.Activate(1);
					buttWar.Activate(0);
					buttUnion.Activate(-1);
					buttPeace.Activate(-1);
					buttSkipUnion.Activate(-1);					
					break;
				};
				if (iTreaty==4)
				{
					buttCancelTrade.Activate(-1);
					buttTrade.Activate(0);
					buttExchange.Activate(0);						
					if (arPlayer[Player].Politic[Oppo].Trade.bTrade)
					{buttSkipTrade.Activate(0);}
					else
					{buttSkipTrade.Activate(-1);};
				}
				else
				{
	//				StopLog("Mode   ",2);
				switch (Mode)
				{
				//Кнопки торговли
				case 2:
					buttCancelTrade.G_Obj->x=31;
					buttCancelTrade.Activate(1);
					buttTrade.Activate(-1);
					buttExchange.Activate(0);
					if (arPlayer[Player].Politic[Oppo].Trade.bTrade)
					{buttSkipTrade.Activate(0);}
					else
					{buttSkipTrade.Activate(-1);};
					break;
				case 3:
					buttCancelTrade.G_Obj->x=97;
					buttCancelTrade.Activate(1);
					buttTrade.Activate(0);
					buttExchange.Activate(-1);
					if (arPlayer[Player].Politic[Oppo].Trade.bTrade)
					{buttSkipTrade.Activate(0);}
					else
					{buttSkipTrade.Activate(-1);};
					break;
				case 4:
					buttCancelTrade.G_Obj->x=163;
					buttCancelTrade.Activate(1);
					buttTrade.Activate(0);
					buttExchange.Activate(0);						
					buttSkipTrade.Activate(-1);
					break;
				case 5:
					buttCancelTrade.Activate(-1);
					buttTrade.Activate(0);
					buttExchange.Activate(0);
					if (arPlayer[Player].Politic[Oppo].Trade.bTrade)
					{buttSkipTrade.Activate(0);}
					else
					{buttSkipTrade.Activate(-1);};
					break;
				default:
					buttCancelTrade.Activate(-1);
					buttTrade.Activate(1);
					if (arPlayer[Player].Politic[Oppo].Trade.bTrade)
					{
						buttSkipTrade.Activate(1);
						buttTrade.iHint=175;
					}
					else
					{
						buttSkipTrade.Activate(-1);
						buttTrade.iHint=163;
					};
					buttExchange.Activate(1);
					break;
				};
			};
			};
			//Маски			
			switch (Mode)
			{
			case 2:
				//Торговля
				if (mskGive[0].flag<1)
				{	
					inpGoldTake.Activate(0);
					inpGemTake.Activate(0);
					inpGoldGive.Activate(0);
					inpGemGive.Activate(0);
					scrGoldTake.Activate(1);
					scrGemTake.Activate(1);
					scrGoldGive.Activate(1);
					scrGemGive.Activate(1);
					for (int j=0; j<18; j++)
					{
						mskGive[j].Activate(1);
						mskTake[j].Activate(1);
					};
				};
				break;
			case 3:
				//Обмен
				if (mskGive[0].flag<1)
				{	
		//			StopLog("Inpu   ",3);
					inpGoldTake.Activate(0);
					inpGemTake.Activate(0);
					inpGoldGive.Activate(0);
					inpGemGive.Activate(0);
					scrGoldTake.Activate(1);
					scrGemTake.Activate(1);
					scrGoldGive.Activate(1);
					scrGemGive.Activate(1);
		//			StopLog("Mask   ",4);
					for (int j=0; j<8; j++)
					{
						mskGive[j].Activate(1);
						mskTake[j].Activate(1);
					};
					for (int j=8; j<16; j++)
					{mskGive[j].Activate(1);};
					for (int j=16; j<18; j++)
					{
						mskGive[j].Activate(1);
						mskTake[j].Activate(1);
					};
				};
				break;
			default:
				if (mskGive[0].flag>0)
				{
					for (int j=0; j<18; j++)
					{
						mskGive[j].Activate(-1);
						mskTake[j].Activate(-1);
					};					
					CloseInput=5;
				};
			};
	//		StopLog("Norm   ",2);
			bNormalizeButtons=false;
		};

		//Закрытие окон ввода
		if (CloseInput>=0)
		{
			if (inpGoldGive.Flag>0)
			{
				arGive[16]=inpGoldGive.GetZyfer();
				if ((arGive[16]>arPlayer[Player].Gold)&&(Mode==3))
				{
					arGive[16]=arPlayer[Player].Gold;
					if (arGive[16]>0)
					{inpGoldGive.EnterInt(arGive[16]);}
					else
					{inpGoldGive.Clear();};					
				};				
				if ((arGive[16]>arPlayer[Player].Gold/10+arPlayer[Player].GoldInc)&&(Mode==2))
				{
					arGive[16]=arPlayer[Player].Gold/10+arPlayer[Player].GoldInc;
					if (arGive[16]>0)
					{inpGoldGive.EnterInt(arGive[16]);}
					else
					{inpGoldGive.Clear();};					
				};
				if ((arTake[16]>0)&&(arGive[16]>0))
				{
					arTake[16]=0;
					inpGoldTake.Clear();
					scrGoldTake.Show();
				};
				inpGoldGive.Activate(0);
				if (CloseInput!=5)
				{bShowDeal=true;};
				scrGoldGive.Show();				
			};
			if (inpGemGive.Flag>0)
			{
				arGive[17]=inpGemGive.GetZyfer();
				if ((arGive[17]>arPlayer[Player].Gem)&&(Mode==3))
				{
					arGive[17]=arPlayer[Player].Gem;					
					if (arGive[17]>0)
					{inpGemGive.EnterInt(arGive[17]);}
					else
					{inpGemGive.Clear();};
				};
				if ((arGive[17]>arPlayer[Player].Gem/10+arPlayer[Player].GemInc)&&(Mode==2))
				{
					arGive[17]=arPlayer[Player].Gem/10+arPlayer[Player].GemInc;					
					if (arGive[17]>0)
					{inpGemGive.EnterInt(arGive[17]);}
					else
					{inpGemGive.Clear();};
				};
				if ((arTake[17]>0)&&(arGive[17]>0))
				{
					arTake[17]=0;
					inpGemTake.Clear();
					scrGemTake.Show();
				};
				inpGemGive.Activate(0);
				if (CloseInput!=5)
				{bShowDeal=true;};
				scrGemGive.Show();				
			};
			if (inpGoldTake.Flag>0)
			{
				arTake[16]=inpGoldTake.GetZyfer();
				if ((arTake[16]>0)&&(arGive[16]>0))
				{
					arGive[16]=0;
					inpGoldGive.Clear();
					scrGoldGive.Show();
				};
				inpGoldTake.Activate(0);
				if (CloseInput!=5)
				{bShowDeal=true;};
				scrGoldTake.Show();				
			};
			if (inpGemTake.Flag>0)
			{
				arTake[17]=inpGemTake.GetZyfer();
				if ((arTake[17]>0)&&(arGive[17]>0))
				{
					arGive[17]=0;
					inpGemGive.Clear();
					scrGemGive.Show();
				};
				inpGemTake.Activate(0);
				if (CloseInput!=5)
				{bShowDeal=true;};				
				scrGemTake.Show();
			};
			switch (CloseInput)
			{
			case 1:
				inpGoldGive.Activate(1);
				break;
			case 2:
				inpGemGive.Activate(1);
				break;
			case 3:
				inpGoldTake.Activate(1);
				break;
			case 4:
				inpGemTake.Activate(1);
				break;
			case 5:
				inpGoldTake.Activate(-1);
				inpGemTake.Activate(-1);
				inpGoldGive.Activate(-1);
				inpGemGive.Activate(-1);
				scrGoldTake.Activate(-1);
				scrGemTake.Activate(-1);
				scrGoldGive.Activate(-1);
				scrGemGive.Activate(-1);
				break;
			};					
			CloseInput=-1;
		};

		
		Check=0;
		if (bTutHint)
		{
			if (Tutorial.Mode==0)
			{
				txtTutorial.Hide();
				objTutorial->iLayer=-1;
				bTutHint=false;
			}
			else
			{Check=mskTutorial.Check();};			
		};

		if (Check>1)
		{
			if (Check>20)
			{
				if (TutNext==0)
				{
					txtTutorial.Hide();
					objTutorial->iLayer=-1;
					bTutHint=false;
				}
				else
				{
					txtTutorial.Activate((pFirstTutText+TutNext)->msg);
					TutNext=0;
				};
			};
		}
		else
		{


		//Кнопка выхода/отмены
		if (buttExit.Check())
		{				
			bRepeat=false;
			if (Mode>0)
			{
				if ((pFirstDialog+826)->Show(true,true,false,false,NULL,NULL)==2)
				{bRepeat=true;};
			};
		};

		//Кнопка отправки сообщения
		if (bMessage)
		{
			bMessage=false;
			if (Mode==-1)
			//Редактировать сообщение
			{
				bPickDeal=true;
				bShowDeal=true;
			}
			else
			{
			//Отправить сообщение
	//			arPlayer[Player].Politic[Oppo].Message.bDeal=true;
				bool bSend=true, bGive=false, bTake=false, bDrawer=true;
				for (int i=0; i<18; i++)
				{
					if (arGive[i]>0)
					{bGive=true;};
					if (arTake[i]>0)
					{bTake=true;};
				};
				if ((bTake)&&(!bGive)&&(iTreaty==0))
				{
					int Delay=0;
					if ((CurGame.iCurrentTurn-arPlayer[Player].Politic[Oppo].arTalk[6][2]<=10)&&(arPlayer[Player].Politic[Oppo].arTalk[6][1]>0))
					{Delay=10+arPlayer[Player].Politic[Oppo].arTalk[6][2]-CurGame.iCurrentTurn;};
					if ((10+arPlayer[Oppo].Politic[Player].arTalk[7][2]-CurGame.iCurrentTurn>Delay)&&(arPlayer[Oppo].Politic[Player].arTalk[7][1]>0))
					{Delay=10+arPlayer[Oppo].Politic[Player].arTalk[7][2]-CurGame.iCurrentTurn;};
					if (Delay>0)
					{
						bSend=false;
						char Mess[1000];
						sprintf(Mess,(pFirstDialog+1164)->sText,Delay);
						(pFirstDialog+1164)->Show(1,1,0,0,NULL,Mess);
						bDrawer=false;
					};					
				//	if ((!bTake)&&(!bGive)&&(iTreaty==0)&&(Mode<4))
				//	{bSend=false;}
				};
				if (bSend)
				{
				if (Mode==5)
				{arPlayer[Player].Politic[Oppo].Message.Type=1;}
				else
				{arPlayer[Player].Politic[Oppo].Message.Type=Mode;};
				arPlayer[Player].Politic[Oppo].Message.Treaty=iTreaty;
				for (int i=0; i<16; i++)
				{
					arPlayer[Player].Politic[Oppo].Message.Trade.Stuff[i]=arGive[i];
					arPlayer[Player].Politic[Oppo].Message.Trade1.Stuff[i]=arTake[i];
				};
				arPlayer[Player].Politic[Oppo].Message.Trade.Gold=arGive[16];
				arPlayer[Player].Politic[Oppo].Message.Trade.Gem=arGive[17];
				arPlayer[Player].Politic[Oppo].Message.Trade1.Gold=arTake[16];
				arPlayer[Player].Politic[Oppo].Message.Trade1.Gem=arTake[17];
				if (Mode==3)
				{
					for (int i=0; i<8; i++)
					{
						if (arGive[i+8]>=0)
						{
							arPlayer[Player].Politic[Oppo].Message.Trade.Stuff[i+8]=arPlayer[Player].arTreasury[arGive[i+8]][0];
							arPlayer[Player].Politic[Oppo].Message.arDurability[i]=arPlayer[Player].arTreasury[arGive[i+8]][1];
						}
						else
						{arPlayer[Player].Politic[Oppo].Message.Trade.Stuff[i+8]=0;};
					};
				};
				//Создание текста сообщения
				switch (iTreaty)
				{
				case 1:
					MakeDipText(Player,Oppo,4,0);
					break;
				case 2:
					MakeDipText(Player,Oppo,5,0);
					break;
				case 3:
					MakeDipText(Player,Oppo,6,0);
					break;
				case 4:
					MakeDipText(Player,Oppo,11,0);
					break;				
				case 0:
					if (Mode==4)
					{
						MakeDipText(Player,Oppo,10,0);
						break;
					};					
					if ((bGive)&&(!bTake))
					{MakeDipText(Player,Oppo,13,0);};
					if ((!bGive)&&(bTake))
					{MakeDipText(Player,Oppo,12,0);};
					if ((bGive)&&(bTake))
					{
						if (Mode==2)
						{MakeDipText(Player,Oppo,7,0);};
						if (Mode==3)
						{MakeDipText(Player,Oppo,8,0);};
					};
					break;
				};
				
				};
				if (bDrawer)
				{
					blit(bmpGUI_Diplomacy,bmpScr,0,360,0,360,1024,310);
				/*inpGoldGive.Activate(-1);
				inpGemGive.Activate(-1);
				inpGoldTake.Activate(-1);
				inpGemTake.Activate(-1);				
				for (int j=0; j<18; j++)
				{
					mskGive[j].Activate(-1);
					mskTake[j].Activate(-1);
				};*/
					bShowTrade=true;
					iTreaty=0;
					if (Mode>0)
					{Mode=-1;};
				};
			};
		};		

		//Клик по портрету противника
		for (int i=0; i<Count; i++)
		{
			Check=mskPlayer[i].Check();
			if (Check==2)
			{
				MakeStringHint(arPlayer[arOpp[i]].GetName());
				bHint=true;
			};
			if ((Check==30)&&(Oppo!=arOpp[i]))
			{
				bool bPick=true;
				if (Tutorial.Diplomacy==2)
				{
					bTutHint=true;
					ScrollSound();
					txtTutorial.Activate((pFirstTutText+102)->msg);
					objTutorial->iLayer=2;
					Tutorial.Diplomacy=3;
				};
				if (Mode>0)
				{
					if ((pFirstDialog+826)->Show(true,true,false,false,NULL,NULL)==2)
					{bPick=false;};
				};
				if (bPick)
				{
				Oppo=arOpp[i];
				blit(bmpGUI_Diplomacy,bmpScr,0,122,0,122,1024,244);
				draw_sprite(bmpScr,arPlayer[arOpp[i]].GetAvatar(),446,155);
				textprintf_centre_ex(bmpScr,Font1,512,133,COLOR_1,-1,"%s",arPlayer[arOpp[i]].GetName());
				draw_sprite(bmpScr,bmpHerb[arPlayer[arOpp[i]].iColor],381,137);
				draw_sprite(bmpScr,bmpHerb[arPlayer[arOpp[i]].iColor],610,137);
				draw_sprite(bmpScr,bmpHerb[arPlayer[arOpp[i]].iColor],41,316);
				draw_sprite(bmpScr,bmpHerb[arPlayer[arOpp[i]].iColor],377,316);
				draw_sprite(bmpScr,bmpHerb[arPlayer[arOpp[i]].iColor],617,316);
				draw_sprite(bmpScr,bmpHerb[arPlayer[arOpp[i]].iColor],946,316);

				iTreaty=0;
				if (arPlayer[arOpp[i]].bInGame)
				{
					int Enemy=0, Ally=0;
					for (int j=1; j<=CurGame.iMaxPlayers; j++)
					{
						if ((j!=arOpp[i])&&(j!=Player)&&(arPlayer[j].bInGame)&&(arPlayer[Player].Politic[j].Treaty>0)&&(arPlayer[j].Control!=3))
						{
							if (arPlayer[arOpp[i]].Politic[j].Treaty==1)
							{
								draw_sprite(bmpScr,arPlayer[j].GetSmallAvatar(),21+(Enemy%8)*60,184+((Enemy-Enemy%8)/8)*60);
								Enemy++;
							};
							if (arPlayer[arOpp[i]].Politic[j].Treaty==3)
							{
								draw_sprite(bmpScr,arPlayer[j].GetSmallAvatar(),591+(Ally%8)*60,184+((Ally-Ally%8)/8)*60);
								Ally++;
							};
						};
					};
					textprintf_centre_ex(bmpScr,Font1,226,150,COLOR_1,-1,"%s",(pFirstPhrase+155)->str);
					textprintf_centre_ex(bmpScr,Font1,797,150,COLOR_1,-1,"%s",(pFirstPhrase+156)->str);
					textprintf_centre_ex(bmpScr,Font1,226,315,COLOR_1,-1,"%s",(pFirstPhrase+157)->str);
					textprintf_centre_ex(bmpScr,Font1,797,315,COLOR_1,-1,"%s",(pFirstPhrase+158)->str);
					PeaceTime=0; TradeTime=0; UnionTime=0;
					switch (arPlayer[Player].Politic[Oppo].Treaty)
					{
					case 1:
						textprintf_centre_ex(bmpScr,Font1,512,313,COLOR_1,-1,"%s",(pFirstPhrase+184)->str);
						break;
					case 2:
						PeaceType=0;
						PeaceTime=0;
						if ((arPlayer[Player].Politic[Oppo].arTalk[2][1]>0)&&(CurGame.iCurrentTurn-arPlayer[Player].Politic[Oppo].arTalk[2][2]<11))
						{PeaceTime=11-CurGame.iCurrentTurn+arPlayer[Player].Politic[Oppo].arTalk[2][2];};
						if ((arPlayer[Oppo].Politic[Player].arTalk[2][1]>0)&&(11-CurGame.iCurrentTurn+arPlayer[Oppo].Politic[Player].arTalk[2][2]>PeaceTime))
						{PeaceTime=11-CurGame.iCurrentTurn+arPlayer[Oppo].Politic[Player].arTalk[2][2];};
						if ((arPlayer[Oppo].Politic[Player].arTalk[7][1]>0)&&(11-CurGame.iCurrentTurn+arPlayer[Oppo].Politic[Player].arTalk[7][2]>PeaceTime))
						{
							PeaceTime=11-CurGame.iCurrentTurn+arPlayer[Oppo].Politic[Player].arTalk[7][2];
							PeaceType=1;
						};
						if ((arPlayer[Player].Politic[Oppo].arTalk[6][1]>0)&&(11-CurGame.iCurrentTurn+arPlayer[Player].Politic[Oppo].arTalk[6][2]>PeaceTime))
						{
							PeaceTime=11-CurGame.iCurrentTurn+arPlayer[Player].Politic[Oppo].arTalk[6][2];
							PeaceType=1;
						};
						if (PeaceTime>0)
						{textprintf_centre_ex(bmpScr,Font1,512,313,COLOR_1,-1,"%s (%d)",(pFirstPhrase+185)->str,PeaceTime);}
						else
						{textprintf_centre_ex(bmpScr,Font1,512,313,COLOR_1,-1,"%s",(pFirstPhrase+185)->str);};
						break;
					case 3:
						UnionTime=0;
						if (CurGame.iCurrentTurn-arPlayer[Player].Politic[Oppo].arTalk[3][2]<11)
						{UnionTime=11-CurGame.iCurrentTurn+arPlayer[Player].Politic[Oppo].arTalk[3][2];};
						if (11-CurGame.iCurrentTurn+arPlayer[Oppo].Politic[Player].arTalk[3][2]>UnionTime)
						{UnionTime=11-CurGame.iCurrentTurn+arPlayer[Oppo].Politic[Player].arTalk[3][2];};
						if (UnionTime>0)
						{textprintf_centre_ex(bmpScr,Font1,512,313,COLOR_1,-1,"%s (%d)",(pFirstPhrase+186)->str,UnionTime);}
						else
						{textprintf_centre_ex(bmpScr,Font1,512,313,COLOR_1,-1,"%s",(pFirstPhrase+186)->str);};
						break;
					};
				//	textprintf_centre_ex(bmpScr,Font1,512,337,COLOR_1,-1,"%s (%d)",GetAttitudeName(arPlayer[Oppo].GetAttitude(Player)),arPlayer[Oppo].GetAttitude(Player));
					textprintf_centre_ex(bmpScr,Font1,512,337,COLOR_1,-1,"%s",GetAttitudeName(arPlayer[Oppo].GetAttitude(Player)));
					int RBuy=0, RSell=0;
					for (int j=0; j<9; j++)
					{
						if (arPlayer[arOpp[i]].GetResource(j+1)==0)
						{
							draw_sprite(bmpScr,(pFirstResource+j+1)->Pic,94+RBuy*28,339);
							RBuy++;
						};
						if (arPlayer[arOpp[i]].GetResource(j+1)>1)
						{
							draw_sprite(bmpScr,(pFirstResource+j+1)->Pic,678+RSell*28,339);
							RSell++;
						};
					};
					for (int j=0; j<18; j++)
					{
						arGive[j]=0;
						arTake[j]=0;
					};
					buttMessage.Activate(1);
					if (arPlayer[Player].Politic[Oppo].Message.Type>0)
					{Mode=-1;}
					else
					{Mode=0;};
					bShowTrade=true;
				}
				else
				{
					textprintf_centre_ex(bmpScr,Font1,512,313,COLOR_1,-1,"%s",(pFirstPhrase+159)->str);
					buttMessage.Activate(0);
					Mode=-1;
				};
				};
				bNormalizeButtons=true;
			};
		};	


		//Проверка масок торговли
		if ((Mode==2)||(Mode==3))
		{			
			//Окна ввода
			if ((inpGoldGive.Check(false)==2)||(inpGemGive.Check(false)==2)||(inpGoldTake.Check(false)==2)||(inpGemTake.Check(false)==2))
			{
				CloseInput=0;
				buttExit.ipHotkey=-100;
			};

			bool WillCheck=true;
			if ((WillCheck)&&(scrGoldGive.Flag==3))
			{
				scrGoldGive.Check();
				WillCheck=false;
				inpGoldGive.EnterInt(arGive[16]);
				inpGoldGive.Show();
				bShowDeal=true;
			};
			if ((WillCheck)&&(scrGemGive.Flag==3))
			{
				scrGemGive.Check();
				WillCheck=false;
				inpGemGive.EnterInt(arGive[17]);
				inpGemGive.Show();
				bShowDeal=true;
			};
			if ((WillCheck)&&(scrGoldTake.Flag==3))
			{
				scrGoldTake.Check();
				WillCheck=false;
				inpGoldTake.EnterInt(arTake[16]);
				inpGoldTake.Show();
				bShowDeal=true;
			};
			if ((WillCheck)&&(scrGemTake.Flag==3))
			{
				scrGemTake.Check();
				WillCheck=false;
				inpGemTake.EnterInt(arTake[17]);
				inpGemTake.Show();
				bShowDeal=true;
			};
			if (WillCheck)
			{
				Check=scrGoldGive.Check();
				if (Check>2)
				{
					CloseInput=0;
					if (inpGoldGive.Flag<1)
					{
						inpGoldGive.EnterInt(arGive[16]);
						inpGoldGive.Show();
						arTake[16]=0;
						inpGoldTake.Clear();
						scrGoldTake.Show();
					};
					bShowDeal=true;
				};
				Check=scrGemGive.Check();
				if (Check>2)
				{
					CloseInput=0;
					if (inpGemGive.Flag<1)
					{
						inpGemGive.EnterInt(arGive[17]);
						inpGemGive.Show();
						arTake[17]=0;
						inpGemTake.Clear();
						scrGemTake.Show();
					};
					bShowDeal=true;
				};
				Check=scrGoldTake.Check();
				if (Check>2)
				{
					CloseInput=0;
					if (inpGoldTake.Flag<1)
					{
						inpGoldTake.EnterInt(arTake[16]);
						inpGoldTake.Show();
						arGive[16]=0;
						inpGoldGive.Clear();
						scrGoldGive.Show();
					};
					bShowDeal=true;
				};
				Check=scrGemTake.Check();
				if (Check>2)
				{
					CloseInput=0;
					if (inpGemTake.Flag<1)
					{
						inpGemTake.EnterInt(arTake[17]);
						inpGemTake.Show();
						arGive[17]=0;
						inpGemGive.Clear();
						scrGemGive.Show();
					};
					bShowDeal=true;
				};
				
			

			for (int i=0; i<4; i++)
			{
				Check=mskInput[i].Check();
				if (Check==30)
				{CloseInput=i+1;};			
			};
						
			for (int j=0; j<18; j++)
			{
				Check=mskGive[j].Check();
				if (Check==30)
				//Отдаём
				{
					bShowDeal=true;
					CloseInput=0;
					if (j==16)
					{CloseInput=1;};
					if (j==17)
					{CloseInput=2;};
					if (Mode==2)
					//Ресурсы
					{
						if (j<16)
						{							
							if (arGive[j]>0)
							{
						//		draw_sprite(bmpScr,bmpStuff[13],33+(j%8)*60,545+((j-j%8)/8)*60);
								arGive[j]=0;
							}
							else
							{
								for (int i=0; i<16; i++)
								{arPicked[i]=arGive[i];};
								int Res=WindowResChoose(Player,0);
								if (Res>0)
								{									
						//			draw_sprite(bmpScr,(pFirstResource+Res)->BigPic,33+(j%8)*60,545+((j-j%8)/8)*60);
									arGive[j]=Res;
								};
							};
						};
					}
					else
					{
						if (j<8)
						{
						//Провинции
							if (arGive[j]>0)
							{
						//		draw_sprite(bmpScr,bmpStuff[15],33+(j%8)*60,545+((j-j%8)/8)*60);
								arGive[j]=0;
							}
							else
							{
								for (int i=0; i<16; i++)
								{arPicked[i]=arGive[i];};
								int Prov=WindowProvChoose(Player,0);
								if (Prov>0)
								{
						//			draw_sprite(bmpScr,bmpStuff[8],33+(j%8)*60,545+((j-j%8)/8)*60);
									arGive[j]=Prov;
								};
							};
						};
						if ((j>7)&&(j<16))
						{
						//Предметы
							if (arGive[j]>=0)
							{
						//		draw_sprite(bmpScr,bmpStuff[14],33+(j%8)*60,545+((j-j%8)/8)*60);
								arGive[j]=-1;
							}
							else
							{
								for (int i=0; i<16; i++)
								{arPicked[i]=arGive[i];};
								int Item=WindowItemChoose(Player);
								if (Item>=0)
								{
						//			draw_sprite(bmpScr,(pFirstItem+arPlayer[Player].arTreasury[Item][0])->Pic,33+(j%8)*60,545+((j-j%8)/8)*60);
									arGive[j]=Item;
								};
							};
						};
					};
				};

				//Всплывающие подсказки
				if (Check==2)
				{
					if (Mode==2)					
					{
						if (j==16)
						{
							bHint=true;
							MakeStringHint(187);
						};
						if (j==17)
						{
							bHint=true;
							MakeStringHint(188);
						};
						if (j<16)
						{							
							if (arGive[j]>0)
							{
								bHint=true;
								MakeStringHint((pFirstResource+arGive[j])->sName);
							}
							else
							{
								bHint=true;
								MakeStringHint(191);
							};
						};
					}
					else
					{
						if (j==16)
						{
							bHint=true;
							MakeStringHint(189);
						};
						if (j==17)
						{
							bHint=true;
							MakeStringHint(190);
						};
						if (j<8)
						{
							if (arGive[j]>0)
							{
								bHint=true;
								MakeStringHint((pFirstProvince+arGive[j])->sName);
							}
							else
							{
								bHint=true;
								MakeStringHint(192);
							};
						};
						if ((j>7)&&(j<16))
						{
						
							if (arGive[j]>=0)
							{
								bHint=true;
								MakeItemHint(arPlayer[Player].arTreasury[arGive[j]][0],arPlayer[Player].arTreasury[arGive[j]][1],NULL,312,(pFirstItem+arPlayer[Player].arTreasury[arGive[j]][0])->iGoldCost,(pFirstItem+arPlayer[Player].arTreasury[arGive[j]][0])->iGemCost);
							}
							else
							{
								bHint=true;
								MakeStringHint(193);
							};
						};
					};
				};
				
				Check=mskTake[j].Check();

				if (Check==30)
				{
				//Требуем
					bShowDeal=true;
					CloseInput=0;
					if (j==16)
					{
						CloseInput=3;						
					};
					if (j==17)
					{
						CloseInput=4;
					};
					if (Mode==2)
					{
					//Ресурсы
						if (j<16)
						{							
							if (arTake[j]>0)
							{
						//		draw_sprite(bmpScr,bmpStuff[13],514+(j%8)*60,545+((j-j%8)/8)*60);
								arTake[j]=0;
							}
							else
							{
								for (int i=0; i<16; i++)
								{arPicked[i]=arTake[i];};
								int Res=WindowResChoose(Oppo,1);
								if (Res>0)
								{									
						//			draw_sprite(bmpScr,(pFirstResource+Res)->BigPic,514+(j%8)*60,545+((j-j%8)/8)*60);
									arTake[j]=Res;
								};
							};
						};
					}
					else
					{
					//Провинции
						if (j<8)
						{
							if (arTake[j]>0)
							{
					//			draw_sprite(bmpScr,bmpStuff[15],514+(j%8)*60,545+((j-j%8)/8)*60);
								arTake[j]=0;
							}
							else
							{
								for (int i=0; i<16; i++)
								{arPicked[i]=arTake[i];};
								int Prov=WindowProvChoose(Oppo,Player);
								if (Prov>=0)
								{
					//				draw_sprite(bmpScr,bmpStuff[8],514+(j%8)*60,545+((j-j%8)/8)*60);
									arTake[j]=Prov;
								};
							};
						};
					};
				};

				if (Check==2)
				{					
					if (Mode==2)
					{
						if (j==16)
						{
							bHint=true;
							MakeStringHint(187);
						};
						if (j==17)
						{
							bHint=true;
							MakeStringHint(188);
						};
						if (j<16)
						{							
							if (arTake[j]>0)
							{
								bHint=true;
								MakeStringHint((pFirstResource+arTake[j])->sName);						
							}
							else
							{
								bHint=true;
								MakeStringHint(191);
							};
						};
					}
					else
					{
						if (j==16)
						{
							bHint=true;
							MakeStringHint(189);
						};
						if (j==17)
						{
							bHint=true;
							MakeStringHint(190);
						};
						if (j<8)
						{
							if (arTake[j]>0)
							{
								bHint=true;
								MakeStringHint((pFirstProvince+arTake[j])->sName);
							}
							else
							{
								bHint=true;
								MakeStringHint(192);
							};
						};
					};
				};
			};
			};
		};
		//Проверка состояния кнопок
		if (buttTrade.Check())
		{
		//Торговля			
			if ((TradeTime>0)&&(arPlayer[Player].Politic[Oppo].Treaty>1))
			{
				char Mess[1000];
				sprintf(Mess,(pFirstDialog+1149)->sText,TradeTime);
				(pFirstDialog+1149)->Show(1,1,0,0,NULL,Mess);
			}
			else
			{
			Mode=2;
			if ((Tutorial.Mode)&&(Tutorial.DipTrade==0))
			{
				Tutorial.DipTrade=1;
				txtTutorial.Activate((pFirstTutText+105)->msg);
				TutNext=106;
				objTutorial->iLayer=2;
				bTutHint=true;
				ScrollSound();
			};
			if (arPlayer[Player].Politic[Oppo].Treaty==1)
			{iTreaty=2;};
			if (arPlayer[Player].Gold/10+arPlayer[Player].GoldInc>9999)
			{scrGoldGive.AttachValue(&arGive[16],0,9999);}
			else
			{scrGoldGive.AttachValue(&arGive[16],0,arPlayer[Player].Gold/10+arPlayer[Player].GoldInc);};
			if (arPlayer[Player].Gem/10+arPlayer[Player].GemInc>9999)
			{scrGemGive.AttachValue(&arGive[17],0,9999);}
			else
			{scrGemGive.AttachValue(&arGive[17],0,arPlayer[Player].Gem/10+arPlayer[Player].GemInc);};
			scrGoldTake.AttachValue(&arTake[16],0,9999);
			scrGemTake.AttachValue(&arTake[17],0,9999);
			if (arPlayer[Player].Politic[Oppo].Trade.bTrade)
			{
				for (int i=0; i<16; i++)
				{				
					arGive[i]=arPlayer[Player].Politic[Oppo].Trade.Stuff[i];
					arTake[i]=arPlayer[Oppo].Politic[Player].Trade.Stuff[i];
				};
				arGive[16]=arPlayer[Player].Politic[Oppo].Trade.Gold;
				if (arGive[16]>0)
				{inpGoldGive.EnterInt(arGive[16]);}
				else
				{inpGoldGive.Clear();};

				arGive[17]=arPlayer[Player].Politic[Oppo].Trade.Gem;
				if (arGive[17]>0)
				{inpGemGive.EnterInt(arGive[17]);}
				else
				{inpGemGive.Clear();};

				arTake[16]=arPlayer[Oppo].Politic[Player].Trade.Gold;
				if (arTake[16]>0)
				{inpGoldTake.EnterInt(arTake[16]);}
				else
				{inpGoldTake.Clear();};

				arTake[17]=arPlayer[Oppo].Politic[Player].Trade.Gem;
				if (arTake[17]>0)
				{inpGemTake.EnterInt(arTake[17]);}
				else
				{inpGemTake.Clear();};
			}
			else
			{
				for (int i=0; i<18; i++)
				{
					arGive[i]=0;
					arTake[i]=0;
				};
				inpGoldTake.Clear();
				inpGemTake.Clear();
				inpGoldGive.Clear();
				inpGemGive.Clear();
			};
			bShowDeal=true;
			};
		};

		if (buttExchange.Check())
		{
		//Обмен
			Mode=3;
			if ((Tutorial.Mode)&&(Tutorial.Exchange==0))
			{
				Tutorial.Exchange=1;
				txtTutorial.Activate((pFirstTutText+103)->msg);
				TutNext=104;
				objTutorial->iLayer=2;
				bTutHint=true;
				ScrollSound();
			};
			if (arPlayer[Player].Politic[Oppo].Treaty==1)
			{iTreaty=2;};
			if (arPlayer[Player].Gold>9999)
			{scrGoldGive.AttachValue(&arGive[16],0,9999);}
			else
			{scrGoldGive.AttachValue(&arGive[16],0,arPlayer[Player].Gold);};
			if (arPlayer[Player].Gem>9999)
			{scrGemGive.AttachValue(&arGive[17],0,9999);}
			else
			{scrGemGive.AttachValue(&arGive[17],0,arPlayer[Player].Gem);};
			scrGoldTake.AttachValue(&arTake[16],0,9999);
			scrGemTake.AttachValue(&arTake[17],0,9999);
			bShowDeal=true;
			for (int i=0; i<18; i++)
			{
				arGive[i]=0;
				arTake[i]=0;
			};
			for (int i=8; i<16; i++)
			{
				arGive[i]=-1;				
			};
			inpGoldTake.Clear();
			inpGemTake.Clear();
			inpGoldGive.Clear();
			inpGemGive.Clear();
//			StopLog("Butt   ",0);
		};

		if (buttSkipTrade.Check())
		{
			if (TradeTime>0)
			{
				char Mess[1000];
				sprintf(Mess,(pFirstDialog+1149)->sText,TradeTime);
				(pFirstDialog+1149)->Show(1,1,0,0,NULL,Mess);
			}
			else
			{
				Mode=4;
				iTreaty=0;
				bShowDeal=true;
			};
		};
		if (buttWar.Check())
		{
			if (PeaceTime>0)
			{
				char Mess[1000];
				if (PeaceType==0)
				{
					sprintf(Mess,(pFirstDialog+1150)->sText,PeaceTime);
					(pFirstDialog+1150)->Show(1,1,0,0,NULL,Mess);
				}
				else
				{
					sprintf(Mess,(pFirstDialog+1151)->sText,PeaceTime);
					(pFirstDialog+1151)->Show(1,1,0,0,NULL,Mess);
				};
			}
			else
			{
				Mode=5;
				for (int j=0; j<18; j++)
				{
					arGive[j]=0;
					arTake[j]=0;
				};
				if ((Tutorial.Mode)&&(Tutorial.War==0))
				{
					Tutorial.War=1;
					txtTutorial.Activate((pFirstTutText+107)->msg);		
					objTutorial->iLayer=2;
					bTutHint=true;
					ScrollSound();
				};
				iTreaty=1;
				bShowDeal=true;
			};
		};
		if (buttPeace.Check())
		{
			if (Mode==0)
			{Mode=1;};
			if ((Tutorial.Mode)&&(Tutorial.Peace==0))
			{
				Tutorial.Peace=1;
				txtTutorial.Activate((pFirstTutText+109)->msg);		
				objTutorial->iLayer=2;
				bTutHint=true;
				ScrollSound();
			};
			iTreaty=2;
			bShowDeal=true;
		};
		if (buttUnion.Check())
		{
			if (arPlayer[Player].GetUnionCount()>0)
			{(pFirstDialog+1153)->Show(1,1,0,0,NULL,NULL);}
			else
			{
				if (Mode==0)
				{Mode=1;};
				if ((Tutorial.Mode)&&(Tutorial.Union==0))
				{
					Tutorial.Union=1;
					txtTutorial.Activate((pFirstTutText+108)->msg);
					TutNext=110;
					objTutorial->iLayer=2;
					bTutHint=true;
					ScrollSound();
				};
				iTreaty=3;
				bShowDeal=true;
				CloseInput=0;
			};
		};
		if (buttSkipUnion.Check())
		{
			if (UnionTime>0)
			{
				char Mess[1000];
				sprintf(Mess,(pFirstDialog+1152)->sText,UnionTime);
				(pFirstDialog+1152)->Show(1,1,0,0,NULL,Mess);
			}
			else
			{
				Mode=1;
				iTreaty=4;
				bShowDeal=true;
				CloseInput=5;
				for (int j=0; j<18; j++)
				{
					arGive[j]=0;
					arTake[j]=0;
				};
			};
		};
		if (buttCancelTrade.Check())
		{
			if (iTreaty>0)
			{Mode=1;}
			else
			{Mode=0;};
			for (int i=0; i<18; i++)
			{
				arGive[i]=0;
				arTake[i]=0;
			};
			inpGoldTake.Clear();
			inpGemTake.Clear();
			inpGoldGive.Clear();
			inpGemGive.Clear();
			bShowDeal=true;
		};
		if (buttCancelTreaty.Check())
		{
			if (arPlayer[Player].Politic[Oppo].Treaty==1)
			{
				Mode=0;
				for (int i=0; i<18; i++)
				{
					arGive[i]=0;
					arTake[i]=0;
				};
				inpGoldTake.Clear();
				inpGemTake.Clear();
				inpGoldGive.Clear();
				inpGemGive.Clear();
			};
			if ((Mode==1)||(Mode==5))
			{Mode=0;};
			iTreaty=0;
			bShowDeal=true;
			CloseInput=0;
		};

		if (buttMessage.Check())
		{
			CloseInput=5;
			bMessage=true;
		};
		};//Tutorial Check

				
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,10,true);
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
	destroy_bitmap(bmpScr);	
	bHint=false;
	HintType=0;
	iAntisleep=iAntisleepDelay;
};

//***************************************************
//*** Экран окончания игры **************************
//***************************************************
bool ScreenEndGame (int Player, bool bWin, bool bShow, bool bWrite)
{
	iAntisleep=iAntisleepDelay;
	if (iGameMode!=5)
	{
		iGameMode=5;
		ChangeMusic();
	};
	if (CurGame.iGameType>2)
	{arPlayer[Player].iTimeShift=0;};
	BITMAP *bmpStat=create_bitmap(1024,768);
	draw_sprite(bmpStat,arPlayer[Player].GetAvatar(),452,43);
	draw_sprite(bmpStat,bmpGUI_EndGameScreen,0,0);
	G_Object *objGUI=new G_Object(NULL,0,0,1,bmpStat);	
	UI_Button1 buttExit;
	objGUI=buttExit.Init(484,497,1,2,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;	
	textprintf_centre_ex(bmpStat,Font1,198,58,COLOR_1,-1,(pFirstPhrase+279)->str);
	textprintf_centre_ex(bmpStat,Font1,837,50,COLOR_1,-1,(pFirstPhrase+280)->str);
	textprintf_centre_ex(bmpStat,Font1,517,204,COLOR_1,-1,arDifficulty[arPlayer[Player].iDifficulty].sName);
	
	//Подсчёт очков
	long int Score=0, Work1, Work2, Max, Reward=0, RewardScore=0, Bonus;
	int arMedal[8];
	for (int i=0; i<8; i++)
	{arMedal[i]=0;};

	//Особые награды

	//Нет потерь (2)
	if ((bWin)&&(arPlayer[Player].bNoLoss)&&(arPlayer[Player].iTimeShift==0))
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,655,76+Reward*59);
		draw_sprite(bmpStat,(pFirstMedal+2)->Pic,660,80+Reward*59);
		arMedal[Reward]=2;
		Bonus=200*(CurGame.Map.X-2)/8;
		textprintf_centre_ex(bmpStat,Font1,852,90+Reward*59,COLOR_1,-1,"%s",(pFirstPhrase+287)->str);
		textprintf_centre_ex(bmpStat,Font1,852,110+Reward*59,COLOR_1,-1,"+%d %s",Bonus, (pFirstPhrase+295)->str);
		RewardScore+=Bonus;
		Reward++;
	};

	//Нет поражений (8)
	if ((bWin)&&(arPlayer[Player].iLose==0)&&(arPlayer[Player].iTimeShift==0))
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,655,76+Reward*59);
		draw_sprite(bmpStat,(pFirstMedal+8)->Pic,660,80+Reward*59);
		arMedal[Reward]=8;
		Bonus=100*(CurGame.Map.X-2)/8;
		textprintf_centre_ex(bmpStat,Font1,852,90+Reward*59,COLOR_1,-1,"%s",(pFirstPhrase+288)->str);
		textprintf_centre_ex(bmpStat,Font1,852,110+Reward*59,COLOR_1,-1,"+%d %s",Bonus,(pFirstPhrase+295)->str);
		RewardScore+=Bonus;
		Reward++;
	};

	//Чемпион Арены (15)
	if (arPlayer[Player].arCamper[0]>=10)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,655,76+Reward*59);
		draw_sprite(bmpStat,(pFirstMedal+15)->Pic,660,80+Reward*59);
		arMedal[Reward]=15;
		textprintf_centre_ex(bmpStat,Font1,852,90+Reward*59,COLOR_1,-1,"%s",(pFirstPhrase+526)->str);
		textprintf_centre_ex(bmpStat,Font1,852,110+Reward*59,COLOR_1,-1,"+50 %s",(pFirstPhrase+295)->str);
		RewardScore+=50;
		Reward++;
	};

	//Исследовано много земель (3)
	if (arPlayer[Player].iExplore>1000)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,655,76+Reward*59);
		draw_sprite(bmpStat,(pFirstMedal+3)->Pic,660,80+Reward*59);
		arMedal[Reward]=3;
		textprintf_centre_ex(bmpStat,Font1,852,90+Reward*59,COLOR_1,-1,"%s",(pFirstPhrase+289)->str);
		textprintf_centre_ex(bmpStat,Font1,852,110+Reward*59,COLOR_1,-1,"+30 %s",(pFirstPhrase+295)->str);
		RewardScore+=30;
		Reward++;
	};

	//Открыта вся карта (10)
	bool bReward=true;
	for (int i=1; i<=MapSize; i++)
	{
		if (*(arPlayer[Player].pFog+i)>=2)
		{bReward=false;};
	};
	if ((bReward)&&(CurGame.bFog))
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,655,76+Reward*59);
		draw_sprite(bmpStat,(pFirstMedal+10)->Pic,660,80+Reward*59);
		arMedal[Reward]=10;
		Bonus=40*(CurGame.Map.X-2)/8;
		textprintf_centre_ex(bmpStat,Font1,852,90+Reward*59,COLOR_1,-1,"%s",(pFirstPhrase+290)->str);
		textprintf_centre_ex(bmpStat,Font1,852,110+Reward*59,COLOR_1,-1,"+%d %s",Bonus,(pFirstPhrase+295)->str);
		RewardScore+=Bonus;
		Reward++;
	};

	//Замок полностью отстроен (13)
	bReward=true;
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if (arPlayer[Player].CanBuild(i))
		{bReward=false;};
	};
	if (bReward)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,655,76+Reward*59);
		draw_sprite(bmpStat,(pFirstMedal+13)->Pic,660,80+Reward*59);
		arMedal[Reward]=13;
		textprintf_centre_ex(bmpStat,Font1,852,90+Reward*59,COLOR_1,-1,"%s",(pFirstPhrase+291)->str);
		textprintf_centre_ex(bmpStat,Font1,852,110+Reward*59,COLOR_1,-1,"+25 %s",(pFirstPhrase+295)->str);
		RewardScore+=25;
		Reward++;
	};

	//Убито много драконов (1)
	if (arPlayer[Player].iDragonHunt>=10)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,655,76+Reward*59);
		draw_sprite(bmpStat,(pFirstMedal+1)->Pic,660,80+Reward*59);
		arMedal[Reward]=1;
		textprintf_centre_ex(bmpStat,Font1,852,90+Reward*59,COLOR_1,-1,"%s",(pFirstPhrase+292)->str);
		textprintf_centre_ex(bmpStat,Font1,852,110+Reward*59,COLOR_1,-1,"+40 %s",(pFirstPhrase+295)->str);
		RewardScore+=40;
		Reward++;
	};

	//Награблено много золота (4)
	if (arPlayer[Player].iPillage>1000)
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,655,76+Reward*59);
		draw_sprite(bmpStat,(pFirstMedal+4)->Pic,660,80+Reward*59);
		arMedal[Reward]=4;
		textprintf_centre_ex(bmpStat,Font1,852,90+Reward*59,COLOR_1,-1,"%s",(pFirstPhrase+293)->str);
		textprintf_centre_ex(bmpStat,Font1,852,110+Reward*59,COLOR_1,-1,"+25 %s",(pFirstPhrase+295)->str);
		RewardScore+=25;
		Reward++;
	};

	//Собран сет (5)
	if ((arPlayer[Player].bSet)&&(Reward<8))
	{
		draw_sprite(bmpStat,bmpGUI_RulerBar,655,76+Reward*59);
		draw_sprite(bmpStat,(pFirstMedal+5)->Pic,660,80+Reward*59);
		arMedal[Reward]=5;
		textprintf_centre_ex(bmpStat,Font1,852,90+Reward*59,COLOR_1,-1,"%s",(pFirstPhrase+294)->str);
		textprintf_centre_ex(bmpStat,Font1,852,110+Reward*59,COLOR_1,-1,"+20 %s",(pFirstPhrase+295)->str);
		RewardScore+=20;
		Reward++;
	};
	Score+=RewardScore;
						

	textprintf_centre_ex(bmpStat,Font1,215,84,COLOR_1,-1,"%s",(pFirstPhrase+281)->str);

	//Количество оппонентов
	Work1=0; Work2=0; Max=0;
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if (Player!=i)
		{
			if (Max<arPlayer[i].iDifficulty)
			{Max=arPlayer[i].iDifficulty;};
			Work1+=4+arPlayer[i].iDifficulty;
		};
	};
	Work1+=Max*5;
	Score+=Work1;
	textprintf_centre_ex(bmpStat,Font1,145,108,COLOR_2,-1,"%s",(pFirstPhrase+58)->str);
	textprintf_centre_ex(bmpStat,Font1,281,108,COLOR_2,-1,"%d",CurGame.iMaxPlayers-1);
	textprintf_centre_ex(bmpStat,Font1,342,108,COLOR_2,-1,"%d",Work1);

	int RulerPercent=Max+2;

	//Повержено противников
	Work1=0; Work2=0;
	for (int i=1; i<=CurGame.iMaxPlayers; i++)
	{
		if (arPlayer[Player].arKills[i]>0)
		{
			Work1++;
			Work2+=5+(arPlayer[i].iDifficulty)*4;
		};
	};
	Score+=Work2;
	textprintf_centre_ex(bmpStat,Font1,145,132,COLOR_2,-1,"%s",(pFirstPhrase+273)->str);
	textprintf_centre_ex(bmpStat,Font1,281,132,COLOR_2,-1,"%d",Work1);
	textprintf_centre_ex(bmpStat,Font1,342,132,COLOR_2,-1,"%d",Work2);
	
	//Побед	
	Score+=arPlayer[Player].iWin/2;
	textprintf_centre_ex(bmpStat,Font1,145,156,COLOR_2,-1,"%s",(pFirstPhrase+267)->str);
	textprintf_centre_ex(bmpStat,Font1,281,156,COLOR_2,-1,"%d",arPlayer[Player].iWin);
	textprintf_centre_ex(bmpStat,Font1,342,156,COLOR_2,-1,"%d",arPlayer[Player].iWin/2);

	//Поражений
	Work1=-arPlayer[Player].iLose/2;
	Score+=Work1;
	textprintf_centre_ex(bmpStat,Font1,145,180,COLOR_2,-1,"%s",(pFirstPhrase+268)->str);
	textprintf_centre_ex(bmpStat,Font1,281,180,COLOR_2,-1,"%d",arPlayer[Player].iLose);
	textprintf_centre_ex(bmpStat,Font1,342,180,COLOR_2,-1,"%d",Work1);

	//Провинций
	Work1=0; Work2=0;
	for (int i=1; i<=MapSize; i++)
	{
		if ((pFirstProvince+i)->ipPlayer==Player)
		{
			Work1++;
			Work2+=2+(pFirstProvince+i)->GetLevel();
		};
	};
	Work2/=2;
	Score+=Work2;
	textprintf_centre_ex(bmpStat,Font1,145,204,COLOR_2,-1,"%s",(pFirstPhrase+269)->str);
	textprintf_centre_ex(bmpStat,Font1,281,204,COLOR_2,-1,"%d",Work1);
	textprintf_centre_ex(bmpStat,Font1,342,204,COLOR_2,-1,"%d",Work2);

	//Героев
	Work1=0; Work2=0;
	for (int i=0; i<MAX_HERO; i++)
	{
		if (arPlayer[Player].arHero[i])
		{
			Work1++;
			Work2+=3+arPlayer[Player].arHero[i]->iLevel;
		};
	};
	Score+=Work2;
	textprintf_centre_ex(bmpStat,Font1,145,228,COLOR_2,-1,"%s",(pFirstPhrase+270)->str);
	textprintf_centre_ex(bmpStat,Font1,281,228,COLOR_2,-1,"%d",Work1);
	textprintf_centre_ex(bmpStat,Font1,342,228,COLOR_2,-1,"%d",Work2);

	//Воинов-ветеранов
	Work1=0; Work2=0;
	for (int i=0; i<MAX_HERO; i++)
	{
		if (arPlayer[Player].arHero[i])
		{
			for (int j=1; j<MAX_SQUAD; j++)
			{
				if ((arPlayer[Player].arHero[i]->pSquad[j])&&(arPlayer[Player].arHero[i]->pSquad[j]->iLevel>=10)&&(arPlayer[Player].arHero[i]->pSquad[j]->GetUnitKind()!=2))
				{
					Work1++;
					Work2+=(arPlayer[Player].arHero[i]->pSquad[j]->iLevel-7)/3;
					for (int k=0; k<3; k++)
					{
						if ((arPlayer[Player].arHero[i]->pSquad[j]->arMedal[k]>0)&&(arPlayer[Player].arHero[i]->pSquad[j]->arMedal[k]!=6))
						{Work2+=2;};
					};
				};
			};
		};
	};
	for (int i=1; i<=MapSize; i++)
	{
		if (((pFirstProvince+i)->ipPlayer==Player)&&((pFirstProvince+i)->pGarrison))
		{
			for (int j=0; j<MAX_SQUAD; j++)
			{
				if (((pFirstProvince+i)->pGarrison->pSquad[j])&&((pFirstProvince+i)->pGarrison->pSquad[j]->iLevel>=10))
				{
					Work1++;
					Work2+=((pFirstProvince+i)->pGarrison->pSquad[j]->iLevel-7)/3;
					for (int k=0; k<3; k++)
					{
						if (((pFirstProvince+i)->pGarrison->pSquad[j]->arMedal[k]>0)&&((pFirstProvince+i)->pGarrison->pSquad[j]->arMedal[k]!=6))
						{Work2+=2;};
					};
				};
			};
			
		};
	};
	Score+=Work2;
	textprintf_centre_ex(bmpStat,Font1,145,252,COLOR_2,-1,"%s",(pFirstPhrase+283)->str);
	textprintf_centre_ex(bmpStat,Font1,281,252,COLOR_2,-1,"%d",Work1);
	textprintf_centre_ex(bmpStat,Font1,342,252,COLOR_2,-1,"%d",Work2);

	//Построек в замке
	Work1=0;
	for (int i=1; i<=iMaxInnerBld; i++)
	{
		if (arPlayer[Player].arBld[i])
		{
			Work1++;			
		};
	};
	Score+=Work1/4;
	textprintf_centre_ex(bmpStat,Font1,145,276,COLOR_2,-1,"%s",(pFirstPhrase+284)->str);
	textprintf_centre_ex(bmpStat,Font1,281,276,COLOR_2,-1,"%d",Work1);
	textprintf_centre_ex(bmpStat,Font1,342,276,COLOR_2,-1,"%d",Work1/4);

	//Сокровищ найдено
	Work1=0; Work2=0;
	for (int i=0; i<3; i++)
	{
		Work1+=arPlayer[Player].arLoot[i];
		Work2+=arPlayer[Player].arLoot[i]*(i+1);
	};
	Score+=Work2;
	textprintf_centre_ex(bmpStat,Font1,145,300,COLOR_2,-1,"%s",(pFirstPhrase+274)->str);
	textprintf_centre_ex(bmpStat,Font1,281,300,COLOR_2,-1,"%d",Work1);
	textprintf_centre_ex(bmpStat,Font1,342,300,COLOR_2,-1,"%d",Work2);

	//Заданий выполнено
	Score+=arPlayer[Player].iQuest*10;
	textprintf_centre_ex(bmpStat,Font1,145,324,COLOR_2,-1,"%s",(pFirstPhrase+278)->str);
	textprintf_centre_ex(bmpStat,Font1,281,324,COLOR_2,-1,"%d",arPlayer[Player].iQuest);
	textprintf_centre_ex(bmpStat,Font1,342,324,COLOR_2,-1,"%d",arPlayer[Player].iQuest*10);

	//Награды	
	textprintf_centre_ex(bmpStat,Font1,145,348,COLOR_2,-1,"%s",(pFirstPhrase+280)->str);
	textprintf_centre_ex(bmpStat,Font1,281,348,COLOR_2,-1,"%d",Reward);
	textprintf_centre_ex(bmpStat,Font1,342,348,COLOR_2,-1,"%d",RewardScore);

	//Откаты во времени
	Score-=arPlayer[Player].iTimeShift*20;
	textprintf_centre_ex(bmpStat,Font1,145,372,COLOR_2,-1,"%s",(pFirstPhrase+396)->str);	
	textprintf_centre_ex(bmpStat,Font1,281,372,COLOR_2,-1,"%d",arPlayer[Player].iTimeShift);
	textprintf_centre_ex(bmpStat,Font1,342,372,COLOR_2,-1,"%d",-arPlayer[Player].iTimeShift*20);

	//Характер
	Score+=abs(arPlayer[Player].iKarma)/10;
	textprintf_centre_ex(bmpStat,Font1,145,396,COLOR_2,-1,"%s",(pFirstPhrase+285)->str);	
	textprintf_centre_ex(bmpStat,Font1,342,396,COLOR_2,-1,"%d",abs(arPlayer[Player].iKarma)/10);

	textprintf_centre_ex(bmpStat,Font1,215,420,COLOR_1,-1,"%s",(pFirstPhrase+282)->str);

	//Время
	int TrueTime=CurGame.iCurrentTurn+(150-((CurGame.Map.X+CurGame.Map.Y-12)/4)*25);
	if (TrueTime<200)
	{Work1=150-TrueTime/4;}
	else
	{
		if (TrueTime<400)
		{Work1=100-(TrueTime-200)/4;}
		else
		{
			if (TrueTime<600)
			{Work1=50-(TrueTime-400)/8;}
			else
			{
				if (TrueTime<800)
				{Work1=25-(TrueTime-600)/20;}
				else
				{
					if (TrueTime<1000)
					{Work1=15-(TrueTime-800)/40;}
					else
					{
						if (TrueTime<2000)
						{Work1=10-(TrueTime-1000)/200;}
						else						
						{Work1=4;};
					};
				};
			};
		};
	};
	textprintf_centre_ex(bmpStat,Font1,145,444,COLOR_2,-1,"%s",(pFirstPhrase+51)->str);
	textprintf_centre_ex(bmpStat,Font1,281,444,COLOR_2,-1,"%d",CurGame.iCurrentTurn);
	if (Work1>100)
	{textprintf_centre_ex(bmpStat,Font1,342,444,COLOR_2,-1,"+%d~",Work1-100);}
	else
	{textprintf_centre_ex(bmpStat,Font1,342,444,COLOR_2,-1,"%d~",Work1-100);};
	Score=Score*Work1/100;

	//Сложность
	Work1=arDifficulty[arPlayer[Player].iDifficulty].iScore;
	if (!CurGame.bFog)
	{Work1=Work1*4/5;};
	Work1=Work1*RulerPercent/4;
	textprintf_centre_ex(bmpStat,Font1,145,468,COLOR_2,-1,"%s",(pFirstPhrase+218)->str);	
	/*if (Work1>100)
	{textprintf_centre_ex(bmpStat,Font1,342,468,COLOR_2,-1,"+%d~",Work1-100);}
	else
	{textprintf_centre_ex(bmpStat,Font1,342,468,COLOR_2,-1,"%d~",Work1-100);};*/

	textprintf_centre_ex(bmpStat,Font1,342,468,COLOR_2,-1,"%d~",Work1);
	Score=Score*Work1/100;
	if (Score<0)
	{Score=0;};

	//Победа/Поражение
	if (bWin)
	{
		textprintf_centre_ex(bmpStat,Font1,145,492,COLOR_2,-1,"%s",(pFirstPhrase+22)->str);
		textprintf_centre_ex(bmpStat,Font1,342,492,COLOR_2,-1,"100~",Work1-100);
	}
	else
	{
		textprintf_centre_ex(bmpStat,Font1,145,492,COLOR_2,-1,"%s",(pFirstPhrase+23)->str);
		textprintf_centre_ex(bmpStat,Font1,342,492,COLOR_2,-1,"-80~",Work1-100);
		Score/=5;
	};

	textprintf_centre_ex(bmpStat,Font1,145,516,COLOR_1,-1,"%s",(pFirstPhrase+61)->str);
	textprintf_centre_ex(bmpStat,Font1,342,516,COLOR_2,-1,"%d",Score);
	arPlayer[1].arRes[0]=Score;
	

	bool Result=false;
	if (bShow)
	{
	
	int Place;
	char STR[500];
	sprintf(STR,"%s %s %s",GetRulerTitle(Score),arPlayer[Player].GetName(),GetRulerNick(arPlayer[Player].iKarma));
	if (bWrite)
	{
	for (int i=0; i<10; i++)
	{
		if (Score>arRecords[i].Score)
		{
			Result=true;
			Place=i;
			break;
		};
	};
	if (Result)
	{
		for (int i=9; i>Place; i--)
		{
			arRecords[i]=arRecords[i-1];
		};
		sprintf(arRecords[Place].sName,STR);
		arRecords[Place].Score=Score;
		arRecords[Place].ipPortrait=arHumans[arPlayer[Player].iColor].ipAvatar;
		arRecords[Place].Col1=arHumans[arPlayer[Player].iColor].iColor1;
		arRecords[Place].Col2=arHumans[arPlayer[Player].iColor].iColor2;
		arRecords[Place].Sign=arHumans[arPlayer[Player].iColor].iSign;
		for (int i=0; i<8; i++)
		{
			arRecords[Place].arMedal[i]=arMedal[i];
		};
		SaveRecords();
	};
	};//bWrite

	
	arPlayer[Player].ShowHerb(419,54,bmpStat);
	arPlayer[Player].ShowHerb(585,54,bmpStat);	
	textprintf_centre_ex(bmpStat,Font1,517,23,COLOR_1,-1,STR);
	int Year=298;
	if ((Score%100<11)||(Score%100>15))
	{
	switch (Score%10)
	{
	case 1:
		Year=296;
		break;
	case 2:
	case 3:
	case 4:		
		Year=297;
		break;
	};
	};
	UI_TextBox txtSage;
	objGUI=txtSage.Init(409,257,217,212,2,COLOR_1,Font2,objGUI);
	char TXT[1000];
	if ((Score>0)&&(arPlayer[Player].iWin>0))
	{
		sprintf(TXT,(pFirstMessage+22)->msg,STR,Score,(pFirstPhrase+Year)->str);
		if (Result)
		{
			sprintf(TXT,(pFirstMessage+23)->msg,TXT,Place+1);
		};
	}
	else
	{sprintf(TXT,(pFirstMessage+29)->msg);};
	txtSage.Activate(TXT);
	while (!buttExit.Check())
	{	
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,3,true);
		}
		else
		{Sleeper(24);};
		bHint=false;
	};
	};//bShow
	if ((CurGame.iGameType==2)&&((!bShow)||((Campaign.iActiveShard<0)&&(Campaign.bDefender))))	
	{
		for (int i=0; i<8; i++)
		{
			if (arMedal[i]>0)
			{
				Campaign.arJournal[980+arMedal[i]]++;
			};
		};
		if (Score>Campaign.arJournal[980])
		{Campaign.arJournal[980]=Score;};
	};
	G_Object * Temp;
	while (objGUI)
	{
		Temp=objGUI->pNext;		
		delete objGUI;
		objGUI=Temp;
	};
	destroy_bitmap(bmpStat);
	iAntisleep=iAntisleepDelay;
	return Result;
};

void PickRuler(int Rul, int Num)
{
	arRecords[Num].ipPortrait=Rul-1;
	arRecords[Num].Col1=(pFirstPlayerProfile+Rul)->iColor1;
	arRecords[Num].Col2=(pFirstPlayerProfile+Rul)->iColor2;
	arRecords[Num].Sign=(pFirstPlayerProfile+Rul)->iSign;
};

void FillTab()
{
	for (int i=0; i<10; i++)
	{
		for (int j=0; j<8; j++)
		{
			arRecords[i].arMedal[j]=0;
		};
	};
	arRecords[9].Score=15;
	PickRuler(7, 9);
	sprintf(arRecords[9].sName,"%s %s %s",GetRulerTitle(15),(pFirstPlayerProfile+7)->sName,GetRulerNick(-80));
	arRecords[8].Score=25;
	PickRuler(13, 8);
	sprintf(arRecords[8].sName,"%s %s %s",GetRulerTitle(25),(pFirstPlayerProfile+13)->sName,GetRulerNick(-5));
	arRecords[7].Score=40;
	PickRuler(15, 7);
	sprintf(arRecords[7].sName,"%s %s %s",GetRulerTitle(40),(pFirstPlayerProfile+15)->sName,GetRulerNick(-40));
	arRecords[6].Score=60;
	PickRuler(8, 6);
	sprintf(arRecords[6].sName,"%s %s %s",GetRulerTitle(60),(pFirstPlayerProfile+8)->sName,GetRulerNick(5));
	arRecords[5].Score=80;
	PickRuler(5, 5);
	sprintf(arRecords[5].sName,"%s %s %s",GetRulerTitle(80),(pFirstPlayerProfile+5)->sName,GetRulerNick(50));
	arRecords[4].Score=110;
	PickRuler(9, 4);
	sprintf(arRecords[4].sName,"%s %s %s",GetRulerTitle(110),(pFirstPlayerProfile+9)->sName,GetRulerNick(-20));
	arRecords[3].Score=140;
	PickRuler(10, 3);
	sprintf(arRecords[3].sName,"%s %s %s",GetRulerTitle(140),(pFirstPlayerProfile+10)->sName,GetRulerNick(25));
	arRecords[2].Score=180;
	PickRuler(4, 2);
	sprintf(arRecords[2].sName,"%s %s %s",GetRulerTitle(180),(pFirstPlayerProfile+4)->sName,GetRulerNick(80));
	arRecords[2].arMedal[0]=13;
	arRecords[1].Score=250;
	PickRuler(2, 1);
	sprintf(arRecords[1].sName,"%s %s %s",GetRulerTitle(250),(pFirstPlayerProfile+2)->sName,GetRulerNick(-120));
	arRecords[1].arMedal[0]=5;
	arRecords[0].Score=350;
	PickRuler(1, 0);
	sprintf(arRecords[0].sName,"%s %s %s",GetRulerTitle(350),(pFirstPlayerProfile+1)->sName,GetRulerNick(-170));
	arRecords[0].arMedal[0]=4;
};

//***************************************************
//*** Экран титров **********************************
//***************************************************
void ScreenCredits()
{	
	iAntisleep=iAntisleepDelay;
	if (iGameMode!=5)
	{
		iGameMode=5;
		ChangeMusic();
	};
	G_Object *objGUI=new G_Object(NULL,0,0,2,bmpGUI_Credits);
	objGUI=new G_Object(objGUI,253,0,0,bmpAstral);
	G_Object *objAnim[6][20];
	for (int i=0; i<6; i++)
	{
		for (int j=0; j<20; j++)
		{
			objGUI=new G_Object(objGUI,0,0,1,bmpEmpty);
			objGUI->bTrans=true;
			objGUI->iOpacity=128;
			objAnim[i][j]=objGUI;
		};
	};
	BITMAP *bmpCredits, *bmpPic;
	bmpCredits=create_bitmap(512,1700);
	bmpPic=create_bitmap(512,740);
	rectfill(bmpCredits,0,0,bmpCredits->w,bmpCredits->h,COLOR_0);

	objGUI=new G_Object(objGUI,0,14,3,bmpPic);

	UI_Button1 buttExit;
	objGUI=buttExit.Init(1024-bmpButton[226]->w,768-bmpButton[226]->h,1,2,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit.Activate(1);
	buttExit.ipHotkey=KEY_ESC;
	buttExit.iHint=135;
	int dY=0;
	for (int Title=0; Title<iMaxCredits; Title++)
	{
		if ((pFirstCredits+Title)->Number==0)
		{textprintf_centre_ex(bmpCredits,Font1,256,dY,makecol(250,210,20),-1,(pFirstCredits+Title)->pTitle.str);}
		else
		{
		textprintf_centre_ex(bmpCredits,Font1,256,dY,makecol(220,220,20),-1,(pFirstCredits+Title)->pTitle.str);
		dY+=18;
		for (int i=0; i<(pFirstCredits+Title)->Number/2; i++)
		{
			textprintf_centre_ex(bmpCredits,Font2,512/4+10,dY,COLOR_2,-1,(pFirstCredits+Title)->pName[i*2].str);
			textprintf_centre_ex(bmpCredits,Font2,512*3/4-10,dY,COLOR_2,-1,(pFirstCredits+Title)->pName[i*2+1].str);
			dY+=15;
		};
		if ((pFirstCredits+Title)->Number%2>0)
		{
			textprintf_centre_ex(bmpCredits,Font2,256,dY,COLOR_2,-1,(pFirstCredits+Title)->pName[(pFirstCredits+Title)->Number-1].str);
			dY+=16;
		};
		}
		dY+=22;
	};
	int Tick=0, Tick1=0, Scroller=739;


	while (!buttExit.Check())
	{	
		if (Timer)
		{
			rectfill(bmpPic,0,0,bmpPic->w,bmpPic->h,COLOR_0);
			blit(bmpCredits,bmpPic,0,0,0,Scroller,512,740-Scroller);
			Scroller--;
			if (Scroller<-1400)
			{Scroller=740;};
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,4,true);
			Tick++; Tick1++;
			for (int i=0; i<6; i++)
			{
				for (int j=0; j<20; j++)
				{
					if (objAnim[i][j]->x>0)
					{
						objAnim[i][j]->x++;
						objAnim[i][j]->iOpacity=200-abs(768-objAnim[i][j]->x)/2;
						if ((mouse_x>objAnim[i][j]->x)&&(mouse_y>objAnim[i][j]->y)&&(mouse_x<objAnim[i][j]->x+objAnim[i][j]->Pic->w)&&(mouse_y<objAnim[i][j]->y+objAnim[i][j]->Pic->h))
						{objAnim[i][j]->iOpacity=250;};
					};
				};
			};
			if (Tick1==100)
			{
				Tick1=0;
				for (int i=0; i<20; i++)
				{
					if ((objAnim[5][i]->x==0)||(objAnim[5][i]->x>1024))
					{
						objAnim[5][i]->Pic=(pFirstUnitType+1+Random(iMaxUnitType))->Pic;
						objAnim[5][i]->x=512-objAnim[5][i]->Pic->w;
						objAnim[5][i]->y=550;
						break;
					};
				};
			};
			if (Tick==60)
			{
				for (int i=0; i<20; i++)
				{
					if ((objAnim[0][i]->x==0)||(objAnim[0][i]->x>1024))
					{
						objAnim[0][i]->Pic=(pFirstSpell+1+Random(iMaxSpell))->Pic;
						objAnim[0][i]->x=512-objAnim[0][i]->Pic->w;
						objAnim[0][i]->y=50;
						break;
					};
				};
				for (int i=0; i<20; i++)
				{
					if ((objAnim[1][i]->x==0)||(objAnim[1][i]->x>1024))
					{
						objAnim[1][i]->Pic=(pFirstItem+1+Random(iMaxPureItem))->Pic;
						objAnim[1][i]->x=512-objAnim[1][i]->Pic->w;
						objAnim[1][i]->y=150;
						break;
					};
				};
				for (int i=0; i<20; i++)
				{
					if ((objAnim[2][i]->x==0)||(objAnim[2][i]->x>1024))
					{
						objAnim[2][i]->Pic=(pFirstInnerBld+1+Random(iMaxInnerBld))->Pic;
						objAnim[2][i]->x=512-objAnim[2][i]->Pic->w;
						objAnim[2][i]->y=250;
						break;
					};
				};
				for (int i=0; i<20; i++)
				{
					if ((objAnim[3][i]->x==0)||(objAnim[3][i]->x>1024))
					{
						int Num=1+Random(iMaxAbilityNum);
						if ((pFirstAbilityNum+Num)->Num>1000)
						{objAnim[3][i]->Pic=bmpHeroSkill[1+Random(140)];}
						else
						{objAnim[3][i]->Pic=(pFirstAbilityNum+Num)->Pic;};
						objAnim[3][i]->x=512-objAnim[3][i]->Pic->w;
						objAnim[3][i]->y=350;
						break;
					};
				};
				for (int i=0; i<20; i++)
				{
					if ((objAnim[4][i]->x==0)||(objAnim[4][i]->x>1024))
					{
						objAnim[4][i]->Pic=(pFirstOuterBld+1+Random(iMaxOuterBld))->Pic;
						objAnim[4][i]->x=512-objAnim[4][i]->Pic->w;
						objAnim[4][i]->y=450;
						break;
					};
				};
				Tick=0;
			};
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
	destroy_bitmap(bmpCredits);
	destroy_bitmap(bmpPic);
	iGameMode=0;
	ChangeMusic();
};


void ScreenDemo()
{
	if (iGameMode!=5)
	{
		iGameMode=5;
		ChangeMusic();
	};
	DATAFILE* datDemo = load_datafile("dat/Demo.dat");
	if (!datDemo)
	{
		OpenWeb();
		return;
	};
	BITMAP * bmpDemo=(BITMAP *)datDemo[0].dat;
	G_Object *objGUI=new G_Object(NULL,0,0,0,bmpDemo);
	UI_Button1 buttExit;
	objGUI=buttExit.Init(944,700,1,2,bmpButton[226],bmpButton[227],bmpButton[228],bmpButton[228],objGUI);
	buttExit.Activate(1);	
	buttExit.iHint=135;
	UI_Button1 buttSite;
	objGUI=buttSite.Init(457,717,1,2,bmpButton[279],bmpButton[280],bmpButton[281],bmpButton[282],objGUI);
	buttSite.Activate(1);	
	buttSite.iHint=504;
	textprintf_centre_ex(bmpDemo,Font1,515,20,COLOR_1,-1,(pFirstPhrase+505)->str);
	UI_Mask1 mskPic[6];
	int dx=326, dy=333;
	for (int i=0; i<6; i++)
	{
		int x=dx*(i%3), y=dy*(i/3);
		textprintf_centre_ex(bmpDemo,Font1,196+x,81+y,COLOR_1,-1,(pFirstPhrase+506+i)->str);
		mskPic[i].Init(41+x,77+y,303,247,1);
	};
	bool bCycle=true;
	int Check;
	while (bCycle)
	{
		if (buttExit.Check())
		{bCycle=false;};
		if (buttSite.Check())
		{
			bCycle=false;
#ifdef YUPLAY
			if ((iYuPlay>0)&&(bYuPlayClient))
			{YuPlayBuyGame(true);}
			else
#endif
			{OpenWeb();};
		};
		for (int i=0; i<6; i++)
		{
			Check=mskPic[i].Check();
			if (Check>1)
			{
				MakeTextHint(i+107);
				bHint=true;
			};
		};
		if (Timer)
		{
			Timer=false;
			objGUI=ShowActiveObjects(objGUI,3,true);
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
	iAntisleep=iAntisleepDelay;
};