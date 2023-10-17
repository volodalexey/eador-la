#include "Interface.h"
bool bAnimation, bTrueAnimation;
BITMAP *bmpHint;
BITMAP *bmpShadow;
int HintW, HintH, HintType, HintMember, HintDur;
bool bHint;
bool bInput=false;
int RedrawTick=0;

void StupidLogger(G_Object * Obj, char * Log)
{
	/*if (bShowAnim)
	{
	char Txt [200];
	int Count=0;
	FileLog(Log);
	while (Obj)
	{
		Count++;
		sprintf(Txt,"Obj: %d, x: %d, y: %d, Pic: %dx%d",Count,Obj->x,Obj->y,Obj->Pic->w,Obj->Pic->h);
		FileLog(Txt);
		Obj=Obj->pNext;
	};
	};*/
};

void ShowWorkInfo()
{
	char Txt[100];
	sprintf(Txt,"Sound: %d",iSoundVolume);
	textprintf_ex(screen1,Font1,5,5,COLOR_1,-1,Txt);
	textprintf_ex(screen1,Font1,4,4,COLOR_2,-1,Txt);
	sprintf(Txt,"Music: %d",iMusicVolume);
	textprintf_ex(screen1,Font1,5,15,COLOR_1,-1,Txt);
	textprintf_ex(screen1,Font1,4,14,COLOR_2,-1,Txt);
};

clPath::clPath (int sx, int sy, clPath *p)
{
	x=sx;
	y=sy;
	pPrev=p;
};

clPath::~clPath()
{};

clAttackAnim::clAttackAnim(G_Object *Enemy, int Hit, int Attack, int Life, int wHit, int wDeath)
{
	pEnemy=Enemy;
	iHit=Hit;
	iAttack=Attack;
	iLife=Life;
	pNext=NULL;	
	pUnitEffect=NULL;
	pEnemyEffect=NULL;
	bTrample=false;
	wavHit=wHit;
	wavDeath=wDeath;	
};

clAttackAnim::~clAttackAnim()
{};

G_Cloud::G_Cloud(int x1, int y1, int x2, int y2, int Time)
{
	Type=0;
	Angle=Random(256);
	dAngle=Random(5)-2;
	Trans=20;
	x=x1;
	y=y1;
	float D1=x2-x1, D2=Time+10;
	dx=D1/D2;
	D1=y2-y1;
	dy=D1/D2;	
	Pic=create_bitmap(PicSize,PicSize);	
};

G_Cloud::~G_Cloud()
{
	if (bGameInProcess)
	{destroy_bitmap(Pic);};
};

void G_Cloud::Run(int Time, BITMAP *Bmp)
{
	rectfill(Pic,0,0,PicSize,PicSize,makecol(255,0,255));
	int Num=Time/6;
	if (Num>7)
	{Num=7;};
	Angle+=dAngle;
	rotate_sprite(Pic,bmpCloud[Num],10,10,Angle);
	x+=dx;
	y+=dy;
	if (Time<50)
	{Trans+=5;}
	else
	{Trans-=10;};
	if (Trans>200)
	{Trans=200;};
	if (Trans<1)
	{Trans=0;};
	set_trans_blender(Trans,Trans,Trans,Trans);
	draw_trans_sprite(Bmp,Pic,x-15,y-13);
};

G_Death::G_Death(int sx, int sy, BITMAP * Dying)
{
	Type=1;
	Timer=0;
	Pic=create_bitmap(Dying->w,Dying->h);
	rectfill(Pic,0,0,Dying->w,Dying->h,makecol(255,0,255));
	if (Type==0)
	{
		x=sx;
		y=sy;
		Start=0;		
		Line=0;
		for (int i=0; i<12; i++)
		{Clouds[i]=NULL;};
	};
	if (Type==1)
	{
		x=sx+Dying->w/2-22;
		y=sy+Dying->h/2-30;
		Skull=create_bitmap(45,70);
		Clouds[0]=new G_Cloud(x+11+Random(20),y+20+Random(20),x-21,y-10,MaxTime);
		Clouds[1]=new G_Cloud(x+11+Random(20),y+20+Random(20),x+7,y-14,MaxTime);
		Clouds[2]=new G_Cloud(x+11+Random(20),y+20+Random(20),x+37,y-10,MaxTime);
		Clouds[3]=new G_Cloud(x+11+Random(20),y+20+Random(20),x+41,y+23,MaxTime);
		Clouds[4]=new G_Cloud(x+11+Random(20),y+20+Random(20),x+37,y+53,MaxTime);
		Clouds[5]=new G_Cloud(x+11+Random(20),y+20+Random(20),x+7,y+57,MaxTime);
		Clouds[6]=new G_Cloud(x+11+Random(20),y+20+Random(20),x-21,y+53,MaxTime);
		Clouds[7]=new G_Cloud(x+11+Random(20),y+20+Random(20),x-25,y+23,MaxTime);
		Clouds[8]=new G_Cloud(x+11+Random(20),y+20+Random(20),x-10,y-2,MaxTime);
		Clouds[9]=new G_Cloud(x+11+Random(20),y+20+Random(20),x+27,y-2,MaxTime);
		Clouds[10]=new G_Cloud(x+11+Random(20),y+20+Random(20),x+27,y+40,MaxTime);
		Clouds[11]=new G_Cloud(x+11+Random(20),y+20+Random(20),x-10,y+40,MaxTime);
	};
};

G_Death::~G_Death()
{
	if (bGameInProcess)
	{destroy_bitmap(Pic);
	if (Type==1)
	{
		if (bGameInProcess)
		{destroy_bitmap(Skull);};
		for (int i=0; i<12; i++)
		{delete Clouds[i];};
	};
	};
};

bool G_Death::Run(BITMAP *Bmp)
{
	bool Result=true;
	if (Type==0)
	{
	Timer++;
	int Col, Col2;
	if (Line<Pic->h)
	{
		Result=false;
		for (int i=0; i<Pic->w; i++)
		{
			if (getpixel(Pic,i,Line)!=makecol(255,0,255))
			{
				putpixel(Pic,i,Line,makecol(255,100,0));
			};
		};
	};
	bool bFound;
	for (int j=Start; j<Line; j++)
	{
		bFound=true;
		for (int i=0; i<Pic->w; i++)
		{
			Col=getpixel(Pic,i,j);
			if (Col!=makecol(255,0,255))
			{
				Result=false;
				bFound=false;
				if (getr(Col)>0)
				{
					Col2=1+Random(20)+getr(Col)/10;
					if (Col2>getr(Col))
					{
						putpixel(Pic,i,j,makecol(0,0,0));
					}
					else
					{
						putpixel(Pic,i,j,makecol(getr(Col)-Col2,(getr(Col)-Col2)/4,0));
					}
				}
				else
				{
					putpixel(Pic,i,j,makecol(255,0,255));
				};
			};
		};
		if ((bFound)&&(j==Start))
		{
			Start++;
		};
	};
	if (Line<Pic->h)
	{
		Line++;
	};
	draw_sprite(Bmp,Pic,x,y);
	};
	if (Type==1)
	{
		int Trans=(51-Timer)*5;
		Timer++;
		if (Trans<1)
		{Trans=1;};
		set_trans_blender(Trans,Trans,Trans,Trans);
		draw_trans_sprite(Bmp,Pic,x-Pic->w/2+22,y-Pic->h/2+30);
		for (int i=0; i<12; i++)
		{Clouds[i]->Run(Timer, Bmp);};
		Trans=0;
		if (Timer>30)
		{Trans+=(Timer-30)/2;};
		if (Trans>10)
		{Trans=10;};
		rectfill(Skull,0,0,45,70,makecol(255,0,255));
		draw_sprite(Skull,bmpSkull,0,0);
		draw_sprite(Skull,bmpJaw,1,35+Trans);
		if (Timer<50)
		{Trans=30+5*Timer;}
		else
		{Trans=(76-Timer)*5;};
		if (Trans>255)
		{Trans=255;};
		if (Trans<1)
		{Trans=1;};
		set_trans_blender(Trans,Trans,Trans,Trans);
		draw_trans_sprite(Bmp,Skull,x,y);
		set_trans_blender(128,128,128,128);
		if (Timer<70)
		{Result=false;};
	};
	return Result;
};


G_Raise::G_Raise(G_Object * Target)
{
//	x=sx;
//	y=sy;
	Obj=Target;
	Pic=create_bitmap(Obj->Pic->w,Obj->Pic->h);
//	rectfill(Pic,0,0,Dying->w,Dying->h,makecol(255,0,255));
//	blit(Dying,Pic,0,0,0,0,Dying->w,Dying->h);
	Line=Obj->Pic->h;
};

G_Raise::~G_Raise()
{
	if (bGameInProcess)
	{destroy_bitmap(Pic);};
};

bool G_Raise::Run(BITMAP *Bmp)
{
	Line--;
	rectfill(Pic,0,0,Obj->Pic->w,Obj->Pic->h,makecol(255,0,255));
	if (Obj->bFlip)
		{rotate_sprite_v_flip(Pic, Obj->Pic, 0, Line, itofix(128));}
	else
		{draw_sprite(Pic, Obj->Pic, 0, Line);}
	draw_sprite(Bmp,Pic,Obj->x,Obj->y);
	if (Line)
	{return 0;}
	else
	{return 1;};
};


G_Stars::G_Stars(G_Object *OBB, int Timer, int Number, int X, int Y, int W, int H, int R, int G, int B)
{
	for (int i=0; i<50; i++)
	{
		for (int j=0; j<4; j++)
		{
			SS[j][i]=0;
		};
	};
	Obj=OBB;
	iTimer=Timer;
	iNumber=Number;
	if (Number>50)
	{
		Number=50;
	};
	x=X;
	y=Y;
	w=W;
	h=H;
	r=R;
	g=G;
	b=B;
	Pic=create_bitmap(7,7);
	rectfill(Pic,0,0,7,7,COLOR_0);
	putpixel(Pic,3,3,makecol(250,250,250));
	putpixel(Pic,2,3,makecol(R,G,B));
	putpixel(Pic,4,3,makecol(R,G,B));
	putpixel(Pic,3,2,makecol(R,G,B));
	putpixel(Pic,3,4,makecol(R,G,B));

	putpixel(Pic,2,2,makecol(R*2/3,G*2/3,B*2/3));
	putpixel(Pic,2,4,makecol(R*2/3,G*2/3,B*2/3));
	putpixel(Pic,4,2,makecol(R*2/3,G*2/3,B*2/3));
	putpixel(Pic,4,4,makecol(R*2/3,G*2/3,B*2/3));
	putpixel(Pic,1,3,makecol(R*2/3,G*2/3,B*2/3));
	putpixel(Pic,3,1,makecol(R*2/3,G*2/3,B*2/3));
	putpixel(Pic,3,5,makecol(R*2/3,G*2/3,B*2/3));
	putpixel(Pic,5,3,makecol(R*2/3,G*2/3,B*2/3));

	putpixel(Pic,1,1,makecol(R/2,G/2,B/2));
	putpixel(Pic,1,2,makecol(R/2,G/2,B/2));
	putpixel(Pic,0,3,makecol(R/2,G/2,B/2));
	putpixel(Pic,1,4,makecol(R/2,G/2,B/2));
	putpixel(Pic,1,5,makecol(R/2,G/2,B/2));
	putpixel(Pic,2,5,makecol(R/2,G/2,B/2));
	putpixel(Pic,3,6,makecol(R/2,G/2,B/2));
	putpixel(Pic,4,5,makecol(R/2,G/2,B/2));
	putpixel(Pic,5,5,makecol(R/2,G/2,B/2));
	putpixel(Pic,5,4,makecol(R/2,G/2,B/2));
	putpixel(Pic,6,3,makecol(R/2,G/2,B/2));
	putpixel(Pic,5,2,makecol(R/2,G/2,B/2));
	putpixel(Pic,5,1,makecol(R/2,G/2,B/2));
	putpixel(Pic,4,1,makecol(R/2,G/2,B/2));
	putpixel(Pic,3,0,makecol(R/2,G/2,B/2));
	putpixel(Pic,2,1,makecol(R/2,G/2,B/2));
};

G_Stars::~G_Stars()
{
	if (bGameInProcess)
	{destroy_bitmap(Pic);};
};

bool G_Stars::Run(BITMAP * Bmp)
{
	bool Result=true;
	if (iTimer>0)
	{
		iTimer--;
		for (int i=0; i<iNumber; i++)
		{
			if (SS[0][i]==0)
			{
				SS[0][i]=25;
				SS[1][i]=Random(w);
				SS[2][i]=Random(h);
				SS[3][i]=Random(5)-2;
				break;
			};
		};
	};
	for (int i=0; i<iNumber; i++)
	{
		if (SS[0][i]>0)
		{
			Result=false;
			switch (SS[0][i])
			{
			case 25:
			case 24:
			case 23:
			case 22:
			case 21:
				putpixel(Bmp,Obj->x+x+SS[1][i],Obj->y+y+SS[2][i],makecol(r/(SS[0][i]-20),g/(SS[0][i]-20),b/(SS[0][i]-20)));
				break;
			case 20:
			case 19:
			case 18:
			case 17:
			case 16:
				putpixel(Bmp,Obj->x+x+SS[1][i],Obj->y+y+SS[2][i],makecol(r+(255-r)*(21-SS[0][i])/5,g+(255-g)*(21-SS[0][i])/5,b+(255-b)*(21-SS[0][i])/5));
				putpixel(Bmp,Obj->x+x+SS[1][i]+1,Obj->y+y+SS[2][i],makecol(r/(SS[0][i]-15),g/(SS[0][i]-15),b/(SS[0][i]-15)));
				putpixel(Bmp,Obj->x+x+SS[1][i]-1,Obj->y+y+SS[2][i],makecol(r/(SS[0][i]-15),g/(SS[0][i]-15),b/(SS[0][i]-15)));
				putpixel(Bmp,Obj->x+x+SS[1][i],Obj->y+y+SS[2][i]+1,makecol(r/(SS[0][i]-15),g/(SS[0][i]-15),b/(SS[0][i]-15)));
				putpixel(Bmp,Obj->x+x+SS[1][i],Obj->y+y+SS[2][i]-1,makecol(r/(SS[0][i]-15),g/(SS[0][i]-15),b/(SS[0][i]-15)));
				break;
			case 15:
			case 14:
			case 13:
			case 12:
			case 11:
				draw_sprite(Bmp,Pic,Obj->x+x+SS[1][i]-3,Obj->y+y+SS[2][i]-3);				
				break;
			case 10:
			case 9:
			case 8:
			case 7:
			case 6:				
				putpixel(Bmp,Obj->x+x+SS[1][i],Obj->y+y+SS[2][i],makecol(r+(255-r)*(SS[0][i]-5)/5,g+(255-g)*(SS[0][i]-5)/5,b+(255-b)*(SS[0][i]-5)/5));
				putpixel(Bmp,Obj->x+x+SS[1][i]+1,Obj->y+y+SS[2][i],makecol(r/(11-SS[0][i]),g/(11-SS[0][i]),b/(11-SS[0][i])));
				putpixel(Bmp,Obj->x+x+SS[1][i]-1,Obj->y+y+SS[2][i],makecol(r/(11-SS[0][i]),g/(11-SS[0][i]),b/(11-SS[0][i])));
				putpixel(Bmp,Obj->x+x+SS[1][i],Obj->y+y+SS[2][i]+1,makecol(r/(11-SS[0][i]),g/(11-SS[0][i]),b/(11-SS[0][i])));
				putpixel(Bmp,Obj->x+x+SS[1][i],Obj->y+y+SS[2][i]-1,makecol(r/(11-SS[0][i]),g/(11-SS[0][i]),b/(11-SS[0][i])));
				break;
			case 5:
			case 4:
			case 3:
			case 2:
			case 1:
				putpixel(Bmp,Obj->x+x+SS[1][i],Obj->y+y+SS[2][i],makecol(r/(6-SS[0][i]),g/(6-SS[0][i]),b/(6-SS[0][i])));
				break;
			};
			SS[0][i]--;
			SS[1][i]+=SS[3][i];
			SS[2][i]++;
			if (Random(10)==9)
			{
				SS[3][i]=Random(5)-2;
			};
		};
	};
	return Result;
};
clEffQue::clEffQue(int Type, int Delay, G_Object *OBB)
{
	Obj=OBB;
	pNext=NULL;
	ipType=Type;
	iDelay=Delay;
	wavSound=NULL;
};

clEffQue::~clEffQue()
{};

void clEffQue::Call()
{
	switch (ipType)
	{
	case 1:
		Obj->AnimNumber(iNum,iColor);
		break;
	case 2:
		Obj->AnimText(iNum, iColor);
		break;
	case 3:
		Obj->Make_Stars(Obj,Param1,iNum,x,y,w,h,r,g,b);
		break;
	case 4:
		Obj->AnimDeath(NULL);
		break;
	};
	if (wavSound)
	{
		PlayUnitSound(wavSound);		
	//	play_sample(wavSound,iSoundVolume,128,1000,0);
	};
	if ((wavSound==wavRoot)&&(Obj->iRoot==0))
	{Obj->iRoot=1;};
};

void clEffQue::Proceed(G_Object *Linker)
{
	clEffQue *Eff=this, *Eff1=NULL, *Eff2, *MainEff=this;
	while (Eff)
	{
		if (Eff->iDelay==0)
		{
			Eff->Call();
			if (Eff1)
			{
				Eff1->pNext=Eff->pNext;
			}
			else
			{
				MainEff=Eff->pNext;
			};
			Eff2=Eff->pNext;
			delete Eff;
			Eff=Eff2;
		}
		else
		{
			Eff1=Eff;
			Eff=Eff->pNext;
		};								
	};
	if (MainEff)
	{
		if (Linker)
		{
			Linker->pEffQue=MainEff;
			Linker->iTimer=MainEff->iDelay;
		}
		else
		{
			MainEff->Call();
			Obj->pLink->pEffQue=MainEff->pNext;
			if (MainEff->pNext)
			{
				Obj->pLink->iTimer=MainEff->pNext->iDelay;	
			};
			delete MainEff;
		};
	};
};

bool Stoning(BITMAP *Pic)
{
	bool Result=false, Near;
	int Col, Sum, Sign, CL[3];
	for (int x=0; x<Pic->w; x++)
	{
		for (int y=0; y<Pic->h; y++)
		{
			Col=getpixel(Pic,x,y);
			if (Col!=COLOR_0)
			{
				CL[0]=getr(Col); CL[1]=getg(Col); CL[2]=getb(Col);
				Sum=(CL[0]+CL[1]+CL[2])/3;
				Near=true;
				for (int i=0; i<3; i++)
				{
					if (CL[i]!=Sum)
					{
						Result=true;
						Sign=sign(Sum-CL[i]);
						CL[i]+=5*Sign;
						if (abs(Sum-CL[i])>5)
						{Near=false;};						
					};
				};				
				if (Near)
				{
					for (int i=0; i<3; i++)
					{
						CL[i]=Sum;
					};
				};
				putpixel(Pic,x,y,makecol(CL[0],CL[1],CL[2]));
			};
		};
	};
	return Result;
};

G_Object::G_Object(G_Object *Next, int sx, int sy, int Layer, BITMAP *Bmp)
{
	pNext=Next;
	if (Next)
	{
		Next->pPrev=this;
	};
	pPrev=NULL;
	x=sx;
	y=sy;
	iLayer=Layer;
	Pic=Bmp;
	iStep=0;
	iExplo=0;
	iWeb=0;
	iIce=0;
	iRoot=0;
	iStone=0;
	Track=NULL;	
	iDelay=0;
	iWait=0;
	iAngle=0;
	iRotate=0;
	iFading=0;	
	iOpacity=0;
	Waypoint=NULL;
	fX=x;
	fY=y;	
	bMap=false;
	bTempo=false;
	bAnim=false;
	bDestroyPic=false;
	bDestroyOnDeath=false;
	bInvisOnDeath=false;
	bKillMe=false;
	bFlip=false;
	bTrans=false;
	bReincarnate=false;
	pAttack=NULL;
	pLink=NULL;
	pCargo=NULL;
	pStars=NULL;
	pDeath=NULL;
	pRaise=NULL;
	pEffQue=NULL;
	Shadow=NULL;
	bmpStone=NULL;
};

G_Object::G_Object(G_Object *Next, int sx, int sy, int Layer, BITMAP *Bmp, bool Destroy)
{
	pNext=Next;
	if (Next)
	{
		Next->pPrev=this;
	};
	pPrev=NULL;
	x=sx;
	y=sy;
	iLayer=Layer;
	Pic=Bmp;
	iStep=0;
	iExplo=0;
	iWeb=0;
	iIce=0;	
	iRoot=0;
	iStone=0;
	Track=NULL;	
	iDelay=0;
	iWait=0;
	iAngle=0;
	iRotate=0;
	iFading=0;	
	iOpacity=0;
	Waypoint=NULL;
	fX=x;
	fY=y;	
	bMap=false;
	bTempo=false;
	bAnim=false;
	bFlip=false;
	bTrans=false;
	bDestroyOnDeath=false;
	bInvisOnDeath=false;
	bKillMe=false;
	bReincarnate=false;
	bDestroyPic=Destroy;
	pAttack=NULL;
	pLink=NULL;
	pCargo=NULL;
	pStars=NULL;
	pDeath=NULL;
	pRaise=NULL;
	pEffQue=NULL;
	Shadow=NULL;
	bmpStone=NULL;
};

G_Object::~G_Object()
{
	if (bGameInProcess)
	{
	if (pPrev)
	{
		pPrev->pNext=pNext;
	};
	if (pNext)
	{
		pNext->pPrev=pPrev;
	};
	if ((bDestroyPic)&&(bGameInProcess))
	{
		destroy_bitmap(Pic);
	};
	if (pStars)
	{
		delete pStars;
	};
	if ((bmpStone)&&(bGameInProcess))
	{destroy_bitmap(bmpStone);};
	};
};

void G_Object::SmartLog(char *Txt)
{
	if (bGlobalError)
	{
		if (bDebugMode)
		{allegro_message(Txt);};
		char STR[1000];
		bGlobalError=false;
		sprintf(STR,"!!!!!SmartLog Report: %s !!!!!!!",Txt);
		FileLog(STR);
		sprintf(STR,"x: %d, y: %d, MX: %d, MY: %d, Layer: %d, Opacity: %d",x,y,MapX,MapY,iLayer,iOpacity);
		FileLog(STR);
		if (Pic)
		{
			FileLog("Acquiring bmp data...");
			sprintf(STR,"w: %d, h: %d",Pic->w,Pic->h);
			FileLog(STR);
			if (bDebugMode)
			{
				blit(Pic,screen,0,0,0,0,Pic->w,Pic->h);
				clear_keybuf();
				readkey();
			};
		};
		FileLog(STR);
	};
};

void G_Object::AddCargo(G_Object *Cargo, int dX, int dY, int Layer)
{
	Cargo->fDeltaX=dX;
	Cargo->fDeltaY=dY;
	Cargo->iLayer=Layer;
	if (pCargo)
	{
		G_Object *Temp=pCargo;
		pCargo=Cargo;
		pCargo->pNext=Temp;
		Temp->pPrev=pCargo;
	}
	else
	{
		pCargo=Cargo;
		pCargo->pNext=NULL;
	};
	pCargo->pPrev=NULL;
};

bool G_Object::RemoveCargo(G_Object *Cargo)
{
	G_Object *Temp=pCargo;
	while (Temp)
	{
		if (Temp==Cargo)
		{
			if (Temp->pPrev)
			{
				Temp->pPrev->pNext=Temp->pNext;
			};
			if (Temp->pNext)
			{
				Temp->pNext->pPrev=Temp->pPrev;
			};
			if (Temp==pCargo)
			{
				pCargo=Temp->pNext;
			};
			Cargo->pNext=NULL;
			Cargo->pPrev=NULL;
			Cargo->iLayer=-1;
			return 1;
		};
		Temp=Temp->pNext;
	};
	return 0;
};

void G_Object::MoveCargo(G_Object *Cargo, int DX, int DY)
{
	G_Object *Temp=pCargo;
	while (Temp)
	{
		if (Temp==Cargo)
		{
			Cargo->fDeltaX+=DX;
			Cargo->fDeltaY+=DY;			
			return;
		};
		Temp=Temp->pNext;
	};	
};

void G_Object::StoneForm()
{
	bmpStone=create_bitmap(Pic->w,Pic->h);
	rectfill(bmpStone,0,0,Pic->w,Pic->h,COLOR_0);
	if (bFlip)
	{rotate_sprite_v_flip(bmpStone,Pic,0,0,itofix(128));}
	else
	{draw_sprite(bmpStone,Pic,0,0);};
	iStone=20;
};

void G_Object::Show(BITMAP *Target)
{
	if ((pCargo)&&(iLayer>=0))
	{
		SmartLog("Cargo");		
		G_Object *Obj;
		for (int i=0; i<10; i++)
		{
			Obj = pCargo;
			while (Obj)
			{
				if (Obj->iLayer==i)
				{
					Obj->x=x+Obj->fDeltaX;
					Obj->y=y+Obj->fDeltaY;
		//			textprintf_ex(Background,Font1,1,600,makecol(0,0,0),-1,"%d, %d",Obj->fDeltaX,y);
					Obj->Show(Target);
				};
				Obj=Obj->pNext;
			};
		};	
	};
	if (pDeath)
	{
		SmartLog("Death");		
		bTrans=true;
		iOpacity=pDeath->Timer*4+1;
	};
	if (iStone>0)
	{
		Stoning(bmpStone);
		iStone--;
	};
	if (pRaise)
	{
		SmartLog("Raise");		
		if (pRaise->Run(Target))
		{
			delete pRaise;
			pRaise=NULL;
		};
	}
	else
	{
	if (Shadow)
	{
		if (bFlip)
		{
			draw_trans_sprite(Target, FlipShadow, x, y);
			/*rectfill(bmpShadow,0,0,100,100,COLOR_0);
			rotate_sprite_v_flip(bmpShadow, Shadow, 0, 0, itofix(128));
			draw_trans_sprite(Target, bmpShadow, x, y);*/
		}
		else
		{draw_trans_sprite(Target, Shadow, x, y);};

	};
	if (bmpStone)
	{draw_sprite(Target, bmpStone, x, y);}
	else
	{
	if ((bFlip)&&(Pic!=bmpFlipCorpse))
	{rotate_sprite_v_flip(Target, Pic, x, y, itofix(128));}
	else
	{
		if (bTrans)
		{
			if (iOpacity>0)
			{
				int OPP=iOpacity;
				if (OPP>255)
				{OPP=255;};
				set_trans_blender(OPP,OPP,OPP,OPP);
				draw_trans_sprite(Target, Pic, x, y);
				set_trans_blender(128,128,128,128);
			}
			else
			{
				draw_trans_sprite(Target, Pic, x, y);
			};
		}
		else
		{
			if ((iAngle>0)||(iRotate!=0))
			{
				rotate_sprite(Target, Pic, x, y, itofix(iAngle));
				iAngle+=iRotate;
				if (iAngle<0)
				{iAngle=255+iAngle;};
				if (iAngle>255)
				{iAngle=iAngle-255;};
			}
			else
			{draw_sprite(Target, Pic, x, y);};
		};
	};	
	};
	};
	if (pDeath)
	{
		if (pDeath->Run(Target))
		{
			bTrans=false;
			iOpacity=0;
			delete pDeath;
			pDeath=NULL;
			if (bReincarnate)
			{
		//		pRaise=new G_Raise(this);
		//		pStars=new G_Stars(this,80,30,10,20,44,40,250,250,220);
		//		iLayer+=10;
				bReincarnate=false;
			}
			else
			{
				if (bDestroyOnDeath)
				{
					bKillMe=true;
				};
				if (bInvisOnDeath)
				{iLayer=-1;};
			};
		};
	};
	if (pStars)
	{
		SmartLog("Stars");		
		if (pStars->Run(Target))
		{
			delete pStars;
			pStars=NULL;
		};
	};
	if (iExplo>0)
	{
		if (Pic==bmpEmpty)
		{draw_sprite(Target,bmpExplo[(18-iExplo-(18-iExplo)%3)/3],x+32-bmpExplo[0]->w/2,y+40-bmpExplo[0]->h/2);}
		else
		{draw_sprite(Target,bmpExplo[(18-iExplo-(18-iExplo)%3)/3],x+Pic->w/2-bmpExplo[0]->w/2,y+Pic->h/2-bmpExplo[0]->h/2);};
		iExplo--;
	};
	if (iWeb>0)
	{
		if (!bFlip)
		{rotate_sprite_v_flip(Target,bmpAnWeb[iWeb/3],x+Pic->w/2-bmpAnWeb[0]->w/2,y+Pic->h-bmpAnWeb[0]->h, itofix(128));}
		else
		{draw_sprite(Target,bmpAnWeb[iWeb/3],x+Pic->w/2-bmpAnWeb[0]->w/2,y+Pic->h-bmpAnWeb[0]->h);};
		if (iWeb<18)
		{iWeb++;};
	};
	if ((iRoot>0)&&(Pic!=bmpFlipCorpse)&&(Pic!=bmpCorpse))
	{		
		draw_sprite(Target,bmpAnRoot[iRoot/3],x+Pic->w/2-bmpAnRoot[0]->w/2,y+Pic->h-bmpAnRoot[0]->h+5);
		if (iRoot<21)
		{iRoot++;};
	};
	if (iIce>0)
	{
		set_trans_blender(iIce,iIce,iIce,iIce);
		draw_trans_sprite(Target,bmpStuff[54],x+Pic->w/2-bmpStuff[54]->w/2,y+Pic->h-bmpStuff[54]->h+5);
		set_trans_blender(128,128,128,128);
		if (iIce<140)
		{iIce+=5;};
	};
};

void G_Object::Move(clPath *Path, int Step, int Delay, bool Add)
{
	if (bShowAnim)
	{
	bAnim=true;
	iStep=Step;
	iDelay=Delay;
	if (!Track)
	{
		iLayer++;
	};
	if (Add&&(Track))
	{
		clPath *P1=Track;
		clPath *P2;
		while (P1)
		{
			P2=P1;
			P1=P2->pPrev;
		};
		P2->pPrev=Path->pPrev;
	}
	else
	{
		Track=Path;		
		Waypoint=Path->pPrev;
		fX=Path->x;
		fY=Path->y;
		if (fX==Waypoint->x)
		{
			fDeltaX=0;
			fDeltaY=Step;
			if (fY>Waypoint->y)
			{
				fDeltaY*=-1;
			};		
		}
		else
		{
			fDeltaX=Step/(1+fabs(Waypoint->y-fY)/fabs(Waypoint->x-fX));
			fDeltaY=Step-fDeltaX;
			if (fX>Waypoint->x)
			{
				fDeltaX*=-1;
			};
			if (fY>Waypoint->y)
			{
				fDeltaY*=-1;
			};
		};
	};
	};
};

void G_Object::AnimDeath(int wavDeath)
{
	if (bShowAnim)
	{
	iWeb=0;
	iIce=0;
	iRoot=0;
	if (bmpStone)
	{
		iStone=0;
		destroy_bitmap(bmpStone);
		bmpStone=NULL;
	};
	if (wavDeath)
	{
		PlayUnitSound(wavDeath);
	//	play_sample(wavDeath,iSoundVolume,128,1000,0);
	};
	pDeath = new G_Death(x,y,Pic);
	if (bFlip)
	{rotate_sprite_v_flip(pDeath->Pic, Pic, 0, 0, itofix(128));}
	else
	{
		draw_sprite(pDeath->Pic, Pic, 0, 0);
	};
	if (bReincarnate)	
	{
		Make_Stars(this,80,30,10,20,44,40,250,250,220);
	}
	else
	{
		if ((bDestroyOnDeath)||(bInvisOnDeath))
		{Pic=bmpEmpty;}
		else
		{
			if (bFlip)
			{Pic=bmpFlipCorpse;}
			else
			{Pic=bmpCorpse;};
		};
		Shadow=NULL;
		iLayer=1;
	};
	};
};


void G_Object::AnimShoot(G_Object *Enemy, int Hit, int Life, int Wait, clUnitType *Shooter, clUnitType *Target, int Miss, int Speed)
{
	if (bShowAnim)
	{
		PlayUnitSound(Shooter->wavShoot);
	//	play_sample(Shooter->wavShoot,iSoundVolume,128,1000,0);		
		if (pLink)
		{
			pLink= new G_Object(pLink, x+20, y+30, iLayer+2, bmpMissile[Miss]);						
		}
		else
		{
			pLink= new G_Object(NULL, x+20, y+30, iLayer+2, bmpMissile[Miss]);
		};
		float Coord=((Enemy->x-x)*(Enemy->x-x)+(Enemy->y-y)*(Enemy->y-y));
		float Angl=128*acos(abs(Enemy->x-x)/sqrt(Coord))/3.1415;
		pLink->iAngle=Angl;
		if ((x>Enemy->x)&&(y>Enemy->y))
		{pLink->iAngle+=128;}
		else
		{
			if (y>Enemy->y)
			{pLink->iAngle=256-pLink->iAngle;}
			else
			{
				if (x>Enemy->x)
				{pLink->iAngle=128-pLink->iAngle;}
			};
		};	
		pLink->iWait=Wait;
		switch (Miss)
		{
		case 1:
			pLink->iRotate=10;
			break;
		case 3:
			pLink->iRotate=20;
			break;
		case 4:
			pLink->iRotate=4;
			break;
		};
		clPath *P1 = new clPath(x+20, y+30, NULL);
		P1->pPrev = new clPath(Enemy->x+20, Enemy->y+30, NULL);
		pLink->Move(P1,Speed,1,0);
		pLink->pAttack=new clAttackAnim(Enemy, Hit, -1, Life, Shooter->wavShootHit, Target->wavDeath);
	};
};

void G_Object::AnimAttack(G_Object *Enemy, int Hit, int Attacker, int Life, clUnitType *uAttacker, clUnitType *uDefender)
{
	if (bShowAnim)
	{
	bAnim=true;
	if (pAttack)
	{
		clAttackAnim *Att=pAttack;
		while (Att->pNext)
		{
			Att=Att->pNext;
		};
		Att->pNext=new clAttackAnim(Enemy, -1, Attacker, Life, uAttacker->wavHit, uDefender->wavDeath);
		Att->pNext->pNext=new clAttackAnim(Enemy, Hit, Attacker, Life, uAttacker->wavHit, uDefender->wavDeath);
	}
	else
	{
		pAttack=new clAttackAnim(Enemy, -1, Attacker, Life, uAttacker->wavHit, uDefender->wavDeath);
		pAttack->pNext=new clAttackAnim(Enemy, Hit, Attacker, Life, uAttacker->wavHit, uDefender->wavDeath);
	};
	};
};

clEffQue * G_Object::AddAttackUnitEffect(int ipType,int iDelay)
{
	clEffQue *Eff;
	if (bShowAnim)
	{
	if (pAttack)
	{
//		WriteLog("Объект найден");
		clAttackAnim *Att=pAttack;
		while (Att->pNext)
		{
			Att=Att->pNext;
		};
		Eff=Att->pUnitEffect;
		if (Eff)
		{
			while (Eff->pNext)
			{
				Eff=Eff->pNext;
			};
			Eff->pNext=new clEffQue(ipType,iDelay,this);
			Eff=Eff->pNext;
		}
		else
		{
			Att->pUnitEffect=new clEffQue(ipType,iDelay,this);
			Eff=Att->pUnitEffect;
		};			
	};	
	};
	return Eff;
};

clEffQue * G_Object::AddAttackEnemyEffect(int ipType,int iDelay)
{
	clEffQue *Eff;
	if (bShowAnim)
	{
	if (pAttack)
	{
//		WriteLog("Объект найден");
		clAttackAnim *Att=pAttack;
		while (Att->pNext)
		{
			Att=Att->pNext;
		};
		Eff=Att->pEnemyEffect;
		if (Eff)
		{
			while (Eff->pNext)
			{
				Eff=Eff->pNext;
			};
			Eff->pNext=new clEffQue(ipType,iDelay,Att->pEnemy);
			Eff=Eff->pNext;
		}
		else
		{
			Att->pEnemyEffect=new clEffQue(ipType,iDelay,Att->pEnemy);
			Eff=Att->pEnemyEffect;
		};			
	};	
	};
	return Eff;
};

clEffQue * G_Object::AddAttackEnemyEffect(int ipType,int iDelay, G_Object *objTarget)
{
	clEffQue *Eff;
	if (bShowAnim)
	{
	if (pAttack)
	{
//		WriteLog("Объект найден");
		clAttackAnim *Att=pAttack;
		while (Att->pNext)
		{
			Att=Att->pNext;
		};
		Eff=Att->pEnemyEffect;
		if (Eff)
		{
			while (Eff->pNext)
			{
				Eff=Eff->pNext;
			};
			Eff->pNext=new clEffQue(ipType,iDelay,objTarget);
			Eff=Eff->pNext;
		}
		else
		{
			Att->pEnemyEffect=new clEffQue(ipType,iDelay,objTarget);
			Eff=Att->pEnemyEffect;
		};			
	};	
	};
	return Eff;
};

void G_Object::AnimText(int ipText, int Color)
{
	if (bShowAnim)
	{
	int NY=y;
	if (NY<30) {NY=30;};
	int Width=text_length(Font1,(pFirstPhrase+ipText)->str);
	BITMAP *bmpText=create_bitmap(Width+1,20);
	rectfill(bmpText,0,0,Width+1,20,makecol(255,0,255));
	textprintf_ex(bmpText,Font1,1,1,makecol(0,0,0),-1,(pFirstPhrase+ipText)->str);
	textprintf_ex(bmpText,Font1,0,0,Color,-1,(pFirstPhrase+ipText)->str);					
	clPath *P1 = new clPath(x+20, NY, NULL);
	P1->pPrev = new clPath(x+22, NY-30, NULL);
	if (pLink)
	{
		pLink= new G_Object(pLink, x+20, NY, iLayer+2, bmpText);						
	}
	else
	{
		pLink= new G_Object(NULL, x+20, NY, iLayer+2, bmpText);
	};
	pLink->bDestroyPic=true;
	pLink->Move(P1,1,1,0);
	};
};

void G_Object::AnimText(char * sText, int Color)
{
	if (bShowAnim)
	{
	int NY=y;
	if (NY<30) {NY=30;};	
	int Width=text_length(Font1,sText);
	BITMAP *bmpText=create_bitmap(Width+1,20);
	rectfill(bmpText,0,0,Width+1,20,makecol(255,0,255));
	textprintf_ex(bmpText,Font1,1,1,makecol(0,0,0),-1,sText);
	textprintf_ex(bmpText,Font1,0,0,Color,-1,sText);
	clPath *P1 = new clPath(x+20, NY, NULL);
	P1->pPrev = new clPath(x+22, NY-30, NULL);
	if (pLink)
	{
		pLink= new G_Object(pLink, x+20, NY, iLayer+2, bmpText);						
	}
	else
	{
		pLink= new G_Object(NULL, x+20, NY, iLayer+2, bmpText);
	};
	pLink->bDestroyPic=true;
	pLink->Move(P1,1,1,0);
	};
};

void G_Object::AnimNumber(int iNum, int Color)
{	
	if (bShowAnim)
	{
	int NY=y;
	if (NY<30) {NY=30;};
	int Width=1;
	int Temp=iNum;
	while (Temp>9)
	{
		Temp=Temp-Temp%10;
		Temp/=10;
		Width++;
	};
	Width*=20;	
	BITMAP *bmpText=create_bitmap(Width,20);
	rectfill(bmpText,0,0,Width,20,makecol(255,0,255));
	textprintf_ex(bmpText,Font1,1,1,makecol(0,0,0),-1,"%d",iNum);
	textprintf_ex(bmpText,Font1,0,0,Color,-1,"%d",iNum);					
	clPath *P1 = new clPath(x+20, NY, NULL);
	P1->pPrev = new clPath(x+22, NY-30, NULL);
	if (pLink)
	{
		pLink= new G_Object(pLink, x+20, NY, iLayer+2, bmpText);						
	}
	else
	{
		pLink= new G_Object(NULL, x+20, NY, iLayer+2, bmpText);
	};
	pLink->bDestroyPic=true;
	pLink->Move(P1,1,1,0);
	};
};

void G_Object::AnimNumber(int iNum, int Color, int dX)
{	
	if (bShowAnim)
	{
	int NY=y;
	if (NY<30) {NY=30;};
	int Width=1;
	int Temp=iNum;
	while (Temp>9)
	{
		Temp=Temp-Temp%10;
		Temp/=10;
		Width++;
	};
	Width*=20;	
	BITMAP *bmpText=create_bitmap(Width,20);
	rectfill(bmpText,0,0,Width,20,makecol(255,0,255));
	textprintf_ex(bmpText,Font1,1,1,makecol(0,0,0),-1,"%d",iNum);
	textprintf_ex(bmpText,Font1,0,0,Color,-1,"%d",iNum);					
	clPath *P1 = new clPath(x+20+dX, NY, NULL);
	P1->pPrev = new clPath(x+22+dX, NY-30, NULL);
	if (pLink)
	{
		pLink= new G_Object(pLink, x+20+dX, NY, iLayer+2, bmpText);						
	}
	else
	{
		pLink= new G_Object(NULL, x+20+dX, NY, iLayer+2, bmpText);
	};
	pLink->bDestroyPic=true;
	pLink->Move(P1,1,1,0);
	};
};

void G_Object::AnimIcon(BITMAP *bmpIcon, int Speed)
{
	if (bShowAnim)
	{
		BITMAP *NewIcon=create_bitmap(bmpIcon->w+3,bmpIcon->h+3);
		rectfill(NewIcon,0,0,NewIcon->w,NewIcon->h,COLOR_1);
		draw_sprite(NewIcon,bmpIcon,1,1);
		pLink= new G_Object(pLink, x+Pic->w/2-bmpIcon->w/2, y+Pic->h/2-bmpIcon->h/2, iLayer+2, NewIcon);
		pLink->iFading=Speed;
		pLink->iOpacity=500;
		pLink->bTrans=true;
		pLink->bAnim=true;
		pLink->bDestroyPic=true;
	};
};

void G_Object::Make_Stars(G_Object *OBB, int Timer, int Number, int X, int Y, int W, int H, int R, int G, int B)
{
	if (bShowAnim)
	{
	if (pStars)
	{
		delete pStars;
	};
	pStars=new G_Stars(OBB, Timer, Number, X, Y, W, H, R, G, B);
	};
};

bool G_Object::ShowLink(int Layer)
{
	bool Result=0;
	if (pLink)
	{
		SmartLog("Link");		
		G_Object *Lnk=pLink;
		G_Object *Temp;
		while (Lnk)
		{
			if (Lnk->iLayer==Layer)
			{
				Result=1;
				if (Lnk->pEffQue)
				{
					Lnk->iTimer--;
					if (Lnk->iTimer<=0)
					{
						Lnk->pEffQue->Proceed(NULL);
						Lnk->pEffQue=NULL;
					};
				};
				Lnk->PlayAnimation();
			};
			if (!Lnk->bAnim)
			{
				if (!Lnk->pPrev)
				{
					pLink=Lnk->pNext;
				}
				Temp=Lnk;
				Lnk=Lnk->pNext;
				delete Temp;
			}
			else
			{
				Lnk=Lnk->pNext;
			};			
		};
	};
	return Result;
};

bool G_Object::PlayAnimation()
{
	if (iWait)
	{iWait--;}
	else
	{
	if ((iFading>0)&&(iOpacity>0))
	{
		SmartLog("Fading");		
		iOpacity-=iFading;
		if (iOpacity<=0)
		{
			iOpacity=1;
			iFading=0;
		};
	};
	if (Track)
	{
		SmartLog("Track");		
		fX+=fDeltaX;
		fY+=fDeltaY;
		x=fX;
		y=fY;
		if (((abs(x-Waypoint->x)<=fabs(fDeltaX))&&(abs(y-Waypoint->y)<=fabs(fDeltaY)))||(fabs(fX-Waypoint->x)<fabs(fX-Waypoint->x+fDeltaX))||(fabs(fY-Waypoint->y)<fabs(fY-Waypoint->y+fDeltaY)))
		{
			clPath *P=Track;
			if (Waypoint->pPrev)
			{
				Move(Waypoint,iStep,iDelay,false);
			}
			else
			{
				Track=NULL;
				if (iLayer>1)
				{iLayer--;};
			};
			delete P;
		};
	}
	else
	{
	//	if (pWaitEnemy)
	//	{
	//		if (!pWaitEnemy->bAnim)
	//		{
	//			pWaitEnemy=NULL;
	//		};
	//	};
		if ((pAttack)&&((!pAttack->pEnemy->bAnim)||(pAttack->iAttack)))
		{
			SmartLog("Attack");			
			if (pAttack->iAttack==-1)
		//Выстрел
			{
				int Width=1;
				int Temp=pAttack->iHit;
				while (Temp>9)
				{
					Temp=Temp-Temp%10;
					Temp/=10;
					Width++;
				};
				if (pAttack->iLife==0)
				{
			//		play_sample(,iSoundVolume,128,1000,0);
					pAttack->pEnemy->AnimDeath(pAttack->wavDeath);
		//			pAttack->pEnemy->Pic=bmpCorpse;
		//			pAttack->pEnemy->iLayer=1;
				};
				PlayUnitSound(pAttack->wavHit);
			//	play_sample(pAttack->wavHit,iSoundVolume,128,1000,0);
				Width*=20;
				BITMAP *bmpText=create_bitmap(Width,20);
				rectfill(bmpText,0,0,Width,20,makecol(255,0,255));
				textprintf_ex(bmpText,Font1,1,1,makecol(0,0,0),-1,"%d",pAttack->iHit);
				textprintf_ex(bmpText,Font1,0,0,makecol(220,50,5),-1,"%d",pAttack->iHit);
				int NY=pAttack->pEnemy->y;
				if (NY<30) {NY=30;};
				clPath *P1 = new clPath(pAttack->pEnemy->x+20, NY, NULL);
				P1->pPrev = new clPath(pAttack->pEnemy->x+22, NY-30, NULL);
				Pic=bmpText;
				bDestroyPic=true;
				iAngle=0;
				iRotate=0;
				Move(P1,1,1,0);
				if (pAttack->pEnemyEffect)
				{
					pAttack->pEnemyEffect->Proceed(this);
				};				
				if (pAttack->pUnitEffect)
				{
					pAttack->pUnitEffect->Proceed(this);
				};
				delete pAttack;
				pAttack=NULL;
				

			}
			else
		//Рукопашная
			{
			if (pAttack->iHit<0) //Первая стадия
			{					
				if (pAttack->pNext)
				{
					pAttack->pNext->x=x;
					pAttack->pNext->y=y;
					pAttack->pNext->x1=pAttack->pEnemy->x;
					pAttack->pNext->y1=pAttack->pEnemy->y;
				};
				if (pAttack->iAttack)
				{
					clPath *P1=new clPath(x,y,NULL);					
					P1->pPrev = new clPath(x+(pAttack->pEnemy->x-x)/3,y+(pAttack->pEnemy->y-y)/3,NULL);
					Move(P1, 5, 1, true);					
				}
				else
				{
					clPath *P1=new clPath(pAttack->pEnemy->x,pAttack->pEnemy->y,NULL);
					P1->pPrev = new clPath(pAttack->pEnemy->x+(x-pAttack->pEnemy->x)/3,pAttack->pEnemy->y+(y-pAttack->pEnemy->y)/3,NULL);
					pAttack->pEnemy->Move(P1, 5, 1, true);
				};
			}				
			else //Вторая стадия
			{
				if (pAttack->iAttack)
				{
					clPath *P1=new clPath(pAttack->x+(pAttack->x1-pAttack->x)/3,pAttack->y+(pAttack->y1-pAttack->y)/3,NULL);
					P1->pPrev = new clPath(pAttack->x,pAttack->y,NULL);					
					Move(P1, 5, 1, true);					

					int Width=1;
					int Temp=pAttack->iHit;
					while (Temp>9)
					{
						Temp=Temp-Temp%10;
						Temp/=10;
						Width++;
					};
					if (pAttack->iLife==0)
					{
			//			play_sample(pAttack->wavDeath,iSoundVolume,128,1000,0);
						pAttack->pEnemy->AnimDeath(pAttack->wavDeath);
			//			pAttack->pEnemy->Pic=bmpCorpse;
			//			pAttack->pEnemy->iLayer=1;
					};
					PlayUnitSound(pAttack->wavHit);
				//	play_sample(pAttack->wavHit,iSoundVolume,128,1000,0);
					Width*=20;
					BITMAP *bmpText=create_bitmap(Width,20);
					rectfill(bmpText,0,0,Width,20,makecol(255,0,255));
					textprintf_ex(bmpText,Font1,1,1,makecol(0,0,0),-1,"%d",pAttack->iHit);
					textprintf_ex(bmpText,Font1,0,0,makecol(220,50,5),-1,"%d",pAttack->iHit);					
					if (pLink)
					{
						pLink= new G_Object(pLink, pAttack->x1+20, pAttack->y1, iLayer+2, bmpText, true);						
					}
					else
					{
						pLink= new G_Object(NULL, pAttack->x1+20, pAttack->y1, iLayer+2, bmpText, true);
					};
					int NY=pAttack->y1;
					if (NY<30) {NY=30;};
					P1 = new clPath(pAttack->x1+20, NY, NULL);
					P1->pPrev = new clPath(pAttack->x1+22, NY-30, NULL);
					pLink->Move(P1,1,1,0);
					if (pAttack->pEnemyEffect)
					{
						pAttack->pEnemyEffect->Proceed(pLink);
					};				
					if (pAttack->pUnitEffect)
					{
						pAttack->pUnitEffect->Proceed(pLink);							
					};
				}
				else
				{
					clPath *P1=new clPath(pAttack->x1+(pAttack->x-pAttack->x1)/3,pAttack->y1+(pAttack->y-pAttack->y1)/3,NULL);
					P1->pPrev = new clPath(pAttack->x1,pAttack->y1,NULL);
					pAttack->pEnemy->Move(P1, 5, 1, true);
					int Width=1;
					int Temp=pAttack->iHit;
					while (Temp>9)
					{
						Temp=Temp-Temp%10;
						Temp/=10;
						Width++;
					};
					if (pAttack->iLife==0)
					{
		//				play_sample(pAttack->wavDeath,iSoundVolume,128,1000,0);
						AnimDeath(pAttack->wavDeath);
		//				Pic=bmpCorpse;
		//				iLayer=1;
					};
					PlayUnitSound(pAttack->wavHit);
				//	play_sample(pAttack->wavHit,iSoundVolume,128,1000,0);
					Width*=20;
					BITMAP *bmpText=create_bitmap(Width,20);
					rectfill(bmpText,0,0,Width,20,makecol(255,0,255));
					textprintf_ex(bmpText,Font1,1,1,makecol(0,0,0),-1,"%d",pAttack->iHit);
					textprintf_ex(bmpText,Font1,0,0,makecol(220,50,5),-1,"%d",pAttack->iHit);					
					if (pLink)
					{
						pLink= new G_Object(pLink, pAttack->x+20, pAttack->y, iLayer+2, bmpText, true);						
					}
					else
					{
						pLink= new G_Object(NULL, pAttack->x+20, pAttack->y, iLayer+2, bmpText, true);
					};
					int NY=pAttack->y;
					if (NY<30) {NY=30;};
					P1 = new clPath(pAttack->x+20, NY, NULL);
					P1->pPrev = new clPath(pAttack->x+22, NY-30, NULL);
					pLink->Move(P1,1,1,0);
					if (pAttack->pEnemyEffect)
					{
						pAttack->pEnemyEffect->Proceed(pLink);
					};				
					if (pAttack->pUnitEffect)
					{
						pAttack->pUnitEffect->Proceed(pLink);
					};
				};
			};
			if (pAttack->bTrample)
			{
				clPath *P1=new clPath(x,y,NULL);
				P1->pPrev = new clPath(pAttack->x2,pAttack->y2,NULL);
				Move(P1,5,1,true);
				if (bShowAnim)
				{
					PlayUnitSound(wavTrample);
					//play_sample(wavTrample,iSoundVolume,128,1000,0);
				};
			};
			clAttackAnim *Att=pAttack;
			pAttack=pAttack->pNext;			
			delete Att;
			};
		};
	};
	if ((!Track)&&(!pAttack)&&(iFading<=0))
	{
		bAnim=false;
	};
	Show(screen1);
	};
	return bAnim;
};

void G_Object::PlaceOnMap(int mX, int mY)
{
	MapX=mX;
	MapY=mY;
	bMap=true;
};

G_Object * ShowActiveObjects(G_Object *First, int MaxLayer, bool Mouse)
{
	bAnimation=false;
	bTrueAnimation=false;
	G_Object *Cur, *Temp;
	for (int i=0; i<MaxLayer; i++)
	{
		Cur=First;
		while (Cur)
		{
			if (Cur->iLayer==i)
			{				
				Cur->PlayAnimation();
				if ((Cur->bAnim)||(Cur->pLink))
				{
					bAnimation=true;
					bTrueAnimation=true;
				};
				if (Cur->pDeath)
				{bAnimation=true;}
			};
			Cur->ShowLink(i);
			Cur=Cur->pNext;			
		};
	};

	Cur=First;
	while (Cur)
	{
		if (Cur->bKillMe)
		{	
			if (Cur==First)	{First=Cur->pNext;};
			Temp=Cur->pNext;
			delete (Cur);
			Cur=Temp;
		}
		else
		{
			Cur=Cur->pNext;
		};
	};
	if (Mouse)
	{
		draw_sprite(screen1,mouse_sprite,mouse_x,mouse_y);
		if (bHint)
		{
			int x=mouse_x+mouse_sprite->w, y=mouse_y+mouse_sprite->h;
			if ((x+HintW<=1024)&&(y+HintH<=768))
			{draw_sprite(screen1,bmpHint,x,y);}
			else
			{
				if (x+HintW<=1024)
				{draw_sprite(screen1,bmpHint,x,768-HintH);}
				else
				{
					if (y+HintH<=768)
					{draw_sprite(screen1,bmpHint,1024-HintW,y);}
					else
					{draw_sprite(screen1,bmpHint,x-HintW-mouse_sprite->w,768-HintH);};
				};
			};
		};
	};
	//ShowWorkInfo();
	if (iAntisleep>0)
	{iAntisleep--;}
	else
	{		
		if ((iAntisleepDelay==0)||(RedrawTick>3)||(getpixel(screen1,10,10)!=NullColor)||(getpixel(screen1,700,700)!=NullColor)||(getpixel(screen1,500,760)!=NullColor)||(getpixel(screen1,500,350)!=NullColor))
		{
			blit(screen1, screen, 0 , 0 , 0 , 0 , 1024 , 768);
//			draw_sprite(screen,(pFirstMedal+6)->Pic,0,0);
//			textprintf_ex(screen,Font1,0,40,COLOR_2,-1,"%d",(pFirstMedal+6)->arEffect[0][1]);
			RedrawTick=0;
		}
		else
		{RedrawTick++;};
	if ((bScrShot)&&(!key[KEY_F1]))
	{
		al_ffblk Fil;	
		char ScrName[100];
		sprintf(ScrName,"screens\\Screen%d.bmp",iScrShot);
		while(al_findfirst(ScrName,&Fil,FA_ALL)==0)
		{
			al_findclose(&Fil);
			iScrShot++;
			sprintf(ScrName,"screens\\Screen%d.bmp",iScrShot);
		};
		PALETTE Palet;
		get_palette(Palet);
		save_bmp(ScrName,screen1,Palet);
		bScrShot=false;
		iScrShot++;
	};
	};
	return First;

};


G_Object * ShowActiveObjects(G_Object *First, int MaxLayer, bool Mouse, int x, int y)
{
//	FileLog("Entr");
	bAnimation=false;
	bTrueAnimation=false;
	if (bmpMap)
	{
		blit(bmpMap,screen1,x,y,0,0,1024,723);	
		if (bmpMap->w<x+1024)
		{
			rectfill(screen1,1024-(x+1024-bmpMap->w),0,1024,768,COLOR_1);
		};
	};
	G_Object *Cur, *Temp;
	for (int i=0; i<MaxLayer; i++)
	{
		Cur=First;
		while (Cur)
		{
			if (Cur->iLayer==i)
			{	
				try
				{
				if (Cur->Pic)
				{
				if (Cur->bMap)
				{
					Cur->x=Cur->MapX-x;
					Cur->y=Cur->MapY-y;
				};
				if ((Cur->x+Cur->Pic->w>0)&&(Cur->x<1024)&&(Cur->y+Cur->Pic->h>0)&&(Cur->y<768))
				{
					Cur->PlayAnimation();
					if ((Cur->bAnim)||(Cur->pLink))
					{
						bAnimation=true;
						bTrueAnimation=true;
					};
					if ((Cur->pDeath)||(Cur->pStars))
					{bAnimation=true;}
				};
				}
				else
				{
					if (bGlobalError)
					{
						char ST [1000];
						sprintf (ST,"Error in G_Obj (No Picture), Layer: %d, x: %d, y: %d, MapX: %d, MapY: %d, Opacity: %d",i,Cur->x,Cur->y,Cur->MapX,Cur->MapY,Cur->iOpacity);
						FileLog(ST);
						bGlobalError=false;

					};
				};
				}
				catch(...)
				{
				//	if (bGlobalError)
					{
						FileLog("G_Obj exception");
						char ST [1000];
						sprintf (ST,"Error in G_Obj, Layer: %d, x: %d, y: %d, MapX: %d, MapY: %d, Opacity: %d (%d)",i,Cur->x,Cur->y,Cur->MapX,Cur->MapY,Cur->iOpacity,Cur->bTrans);
						FileLog(ST);						
						allegro_message("Internal error! After exiting game, please, send gamelog.txt to bugreport@eador.com. We are really sorry. Thank you for your patience.");
						bGlobalError=false;						
						if (Cur->pDeath)
						{FileLog("Death active");};
						if (Cur->Track)
						{FileLog("Track active");};
						if (Cur->pLink)
						{
							sprintf (ST,"Link params: x: %d, y: %d",Cur->pLink->x,Cur->pLink->y);
							FileLog(ST);
						};
						if (Cur->Pic)
						{
							sprintf (ST,"Pic params: w: %d, h: %d",Cur->Pic->w,Cur->Pic->h);
							FileLog(ST);
						};
						FileLog("Error log finished");
					};
				};
			};
			try
			{
			Cur->ShowLink(i);
			}
			catch(...)				
			{
				FileLog("Link exception");
			};

			Cur=Cur->pNext;			
		};
	};

	Cur=First;
	while (Cur)
	{
		if (Cur->bKillMe)
		{	
			FileLog("KillMe");
			if (Cur==First)	{First=Cur->pNext;};
			Temp=Cur->pNext;
			delete (Cur);
			Cur=Temp;
		}
		else
		{
			Cur=Cur->pNext;
		};
	};
//	throw Mouse;
	if (Mouse)
	{
		draw_sprite(screen1,mouse_sprite,mouse_x,mouse_y);
		if (bHint)
		{
			int x=mouse_x+mouse_sprite->w, y=mouse_y+mouse_sprite->h;
			if ((x+HintW<=1024)&&(y+HintH<=768))
			{draw_sprite(screen1,bmpHint,x,y);}
			else
			{
				if (x+HintW<=1024)
				{draw_sprite(screen1,bmpHint,x,768-HintH);}
				else
				{
					if (y+HintH<=768)
					{draw_sprite(screen1,bmpHint,1024-HintW,y);}
					else
					{draw_sprite(screen1,bmpHint,x-HintW-mouse_sprite->w,768-HintH);};
				};
			};
		};
	};
	//ShowWorkInfo();
	if (iAntisleep>0)
	{iAntisleep--;}
	else
	{
	if ((iAntisleepDelay==0)||(RedrawTick>3)||(getpixel(screen1,10,10)!=NullColor)||(getpixel(screen1,700,700)!=NullColor)||(getpixel(screen1,500,760)!=NullColor)||(getpixel(screen1,500,350)!=NullColor))
	{
		blit(screen1, screen, 0 , 0 , 0 , 0 , 1024 , 768);
	//	draw_sprite(screen,(pFirstMedal+6)->SmallPic,0,0);
		RedrawTick=0;
	}
	else
	{RedrawTick++;};
	if ((bScrShot)&&(!key[KEY_F1]))
	{
		al_ffblk Fil;	
		char ScrName[100];
		sprintf(ScrName,"screens\\Screen%d.bmp",iScrShot);
		while(al_findfirst(ScrName,&Fil,FA_ALL)==0)
		{
			al_findclose(&Fil);
			iScrShot++;
			sprintf(ScrName,"screens\\Screen%d.bmp",iScrShot);
		};
		PALETTE Palet;
		get_palette(Palet);
		save_bmp(ScrName,screen1,Palet);
		bScrShot=false;
		iScrShot++;
	};
	};
//	FileLog("Exit");
	return First;
};