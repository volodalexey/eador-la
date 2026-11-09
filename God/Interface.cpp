#include "Interface.h"

//**************************************
//*** Методы UI_Mask *******************
//**************************************

UI_Mask::UI_Mask()
{
	flag=0;
	iHint=0;
//	ipHotkey=-100;
	bGlow=false;
	Glow_Obj=NULL;
};

/*UI_Mask::UI_Mask(int st_x, int st_y, int st_flag, BITMAP *Mask, BITMAP *Active)
{
	x=st_x;
	y=st_y;
	flag=st_flag;
	bmpMask=Mask;
	bmpActive=Active;	
	bmpBackground=create_bitmap(bmpMask->w, bmpMask->h);
	show_mouse(NULL);
	blit(screen, bmpBackground, x, y, 0, 0, bmpBackground->w, bmpBackground->h);
	show_mouse(screen);
//	Check();
};*/

G_Object* UI_Mask::Init(int st_x, int st_y, int st_flag, BITMAP *Mask, BITMAP *Active, G_Object *Obj, int Layer)
{
	flag=st_flag;
	bmpMask=Mask;
	iDefLayer=Layer;
	G_Obj=new G_Object(Obj, st_x, st_y, -1, Active);
	return G_Obj;
};

G_Object* UI_Mask::Init(int st_x, int st_y, int st_flag, BITMAP *Mask, BITMAP *Active, BITMAP *Glow, G_Object *Obj, int Layer)
{
	flag=st_flag;
	bmpMask=Mask;
	iDefLayer=Layer;
	G_Obj=new G_Object(Obj, st_x, st_y, -1, Active);
	Glow_Obj=new G_Object(G_Obj, st_x, st_y, -1, Glow);
	Glow_Obj->iOpacity=200;
	Glow_Obj->bTrans=true;
	return Glow_Obj;
};

UI_Mask::~UI_Mask()
{};

void UI_Mask::Replace(int st_x, int st_y)
{
		G_Obj->x=st_x;
		G_Obj->y=st_y;
};

void UI_Mask::Activate(int st_flag)
{
	if (st_flag>=0)
	{
		if (!flag)
		{
			flag=1;
			Check();
		};
	}
	else
	{
		flag=0;
		G_Obj->iLayer=-1;
	};	
};

void UI_Mask::Show()
{
	G_Obj->iLayer=iDefLayer;
	flag=2;
};

int UI_Mask::Check()
{	
	int old_flag=flag;
	int Result=flag;
	if (flag>0)
	{
		if ((mouse_x>=G_Obj->x)&&(mouse_y>=G_Obj->y)&&
			(mouse_x<G_Obj->x+bmpMask->w)&&(mouse_y<G_Obj->y+bmpMask->h)&&
			(getpixel(bmpMask,mouse_x-G_Obj->x,mouse_y-G_Obj->y)!=makecol(255,0,255)))				
		{
			if (iHint>0)
			{
				bHint=true;
				MakeStringHint(iHint);
			};
			if (flag!=2)
			{
				G_Obj->iLayer=iDefLayer;
				flag=2;				
			};
			if (mouse_b & 1)
			{
				flag=3;
			};		
			if (mouse_b & 2)
			{
				flag=4;
			};		
		}		
		else
		{
			if (flag!=1)
			{
				G_Obj->iLayer=-1;
				flag=1;
				Result=1;
			};
		};
	};
	if ((flag==2)&&(old_flag==3))
	{
		bHint=false;
		Result=30;
	};
	if ((flag==2)&&(old_flag==4))
	{
		bHint=false;
		Result=40;
	};
	if (Glow_Obj)
	{
		if (bGlow)
		{Glow_Obj->iLayer=iDefLayer;}
		else
		{Glow_Obj->iLayer=-1;};
	};
	return Result;		
};

int UI_Mask::Check(int MX, int MY)
{
	if (G_Obj->bMap)
	{
		G_Obj->x=G_Obj->MapX-MX;
		G_Obj->y=G_Obj->MapY-MY;
	};
	int old_flag=flag;
	int Result=flag;
	if (flag)
	{
		if ((mouse_x>=G_Obj->x)&&(mouse_y>=G_Obj->y)&&
			(mouse_x<G_Obj->x+bmpMask->w)&&(mouse_y<G_Obj->y+bmpMask->h)&&
			(getpixel(bmpMask,mouse_x-G_Obj->x,mouse_y-G_Obj->y)!=makecol(255,0,255)))				
		{
			if (iHint>0)
			{
				bHint=true;
				MakeStringHint(iHint);
			};
			if (flag!=2)
			{
				G_Obj->iLayer=iDefLayer;
				flag=2;				
			};
			if (mouse_b & 1)
			{
				flag=3;
			};		
			if (mouse_b & 2)
			{
				flag=4;
			};		
		}		
		else
		{
			if (flag!=1)
			{
				G_Obj->iLayer=-1;
				flag=1;
				Result=1;
			};
		};
	};
	if ((flag==2)&&(old_flag==3))
	{
		bHint=false;
		Result=30;
	};
	if ((flag==2)&&(old_flag==4))
	{
		bHint=false;
		Result=40;
	};
	if (Glow_Obj)
	{
		if (bGlow)
		{
			Glow_Obj->iLayer=iDefLayer+1;
			Glow_Obj->x=G_Obj->x;
			Glow_Obj->y=G_Obj->y;
		}
		else
		{Glow_Obj->iLayer=-1;};
	};
	return Result;		
};


int UI_Mask::ColorCheck()
{
	int old_flag=flag;
	int Result=flag;
	if (flag)
	{
		if ((mouse_x>=G_Obj->x)&&(mouse_y>=G_Obj->y)&&
			(mouse_x<G_Obj->x+bmpMask->w)&&(mouse_y<G_Obj->y+bmpMask->h)&&
			(getpixel(bmpMask,mouse_x-G_Obj->x,mouse_y-G_Obj->y)!=makecol(255,0,255)))				
		{
			if (flag!=2)
			{
				G_Obj->iLayer=iDefLayer;
				flag=2;				
			};
			if (mouse_b & 1)
			{
				flag=3;
			};
			int Pixel=getpixel(bmpMask,mouse_x-G_Obj->x,mouse_y-G_Obj->y);
			Result=flag;
			if (Pixel==makecol(100,0,0))
			{
				Result=10;
			};
			if (Pixel==makecol(0,0,200))
			{
				Result=11;
			};
			if (Pixel==makecol(0,200,0))
			{
				Result=12;
			};
			if (Pixel==makecol(200,0,0))
			{
				Result=13;
			};
			if (Pixel==makecol(0,0,100))
			{
				Result=14;
			};
			if (Pixel==makecol(0,100,0))
			{
				Result=15;
			};					
		}		
		else
		{
			if (flag!=1)
			{
				G_Obj->iLayer=-1;
				flag=1;
				Result=1;
			};
		};
	};
	if ((flag==3)&&(old_flag!=3))
	{
		Result+=20;
	};
	return Result;		
};

//**************************************
//*** Методы UI_Mask1 ******************
//**************************************

UI_Mask1::UI_Mask1()
{
	flag=0;
	iHint=0;
};

void UI_Mask1::Init(int st_x, int st_y, int st_w, int st_h, int st_flag)
{
	x=st_x; y=st_y; w=st_w; h=st_h;
	flag=st_flag;
};

UI_Mask1::~UI_Mask1()
{};

void UI_Mask1::Replace(int st_x, int st_y)
{
	x=st_x;
	y=st_y;
};

void UI_Mask1::Activate(int st_flag)
{
	if (st_flag>=0)
	{
		if (!flag)
		{
			flag=1;
			Check();
		};
	}
	else
	{
		flag=0;		
	};	
};

int UI_Mask1::Check()
{	
	int old_flag=flag;
	int Result=flag;
	if (flag)
	{
		if ((mouse_x>=x)&&(mouse_y>=y)&&(mouse_x<x+w)&&(mouse_y<y+h))				
		{
			if (iHint>0)
			{
				bHint=true;
				MakeStringHint(iHint);
			};
			if (flag!=2)
			{				
				flag=2;				
			};
			if (mouse_b & 1)
			{
				flag=3;
			};		
			if (mouse_b & 2)
			{
				flag=4;
			};		
		}		
		else
		{
			if (flag!=1)
			{				
				flag=1;
				Result=1;
			};
		};
	};
	if ((flag==2)&&(old_flag==3))
	{
		bHint=false;
		Result=30;
	};
	if ((flag==2)&&(old_flag==4))
	{
		bHint=false;
		Result=40;
	};
	return Result;		
};


//**************************************
//*** Методы UI_Button1 ****************
//**************************************

UI_Button1::UI_Button1()
{
	flag=-1;
	iHint=0;
	ipHotkey=-100;
	ipHotkey2=-100;
	bHot=false;
	bBlink=false;
	wavSound=wavClick;
	Roller=mouse_z;
};

UI_Button1::~UI_Button1()
{};

G_Object * UI_Button1::Init(int st_x, int st_y, int st_flag, int Layer, BITMAP *Norm, BITMAP *Act, BITMAP *Press, BITMAP *Inact, G_Object *Obj)
{
	flag=-10;
	bmpNormal=Norm;
	bmpActive=Act;
	bmpPressed=Press;
	bmpInactive=Inact;
	iDefLayer=Layer;
	{G_Obj=new G_Object(Obj, st_x, st_y, Layer, Norm);}	
	Activate(st_flag);
	Check();
	return G_Obj;
};

void UI_Button1::Activate(int st_flag)
{
	if (st_flag!=flag)
	{
		switch (st_flag)
		{
		case -1:
			G_Obj->iLayer=-1;
			break;
		case 0:
			G_Obj->iLayer=iDefLayer;
			G_Obj->Pic=bmpInactive;
			break;
		case 1:
			G_Obj->iLayer=iDefLayer;
			G_Obj->Pic=bmpNormal;
			break;
		case -2:
			G_Obj->iLayer=iDefLayer;
			G_Obj->Pic=bmpPressed;
			break;
		};
		flag=st_flag;
	};
};

void UI_Button1::Replace(int st_x, int st_y)
{
	G_Obj->x=st_x;
	G_Obj->y=st_y;
}

int UI_Button1::Check()
{
	int Result=0;
	if ((bButtonDelay)&&(!(mouse_b & 1)))
	{bButtonDelay=false;};
	if (flag>0)
	{
		poll_keyboard();
		if ((((ipHotkey!=-100)&&(key[ipHotkey]))||((ipHotkey2!=-100)&&(key[ipHotkey2])))
			&&(!bInput))
		{
			if (flag!=3)
			{
				G_Obj->Pic=bmpPressed;
				flag=3;
		//	if (!bHot)
		//		{
		//			Result=1;					
		//		};			
			};
			bHot=true;
		}
		else
		{
		if (bHot)
		{
			Result=1;					
			bHot=false;
		};			

			
		
		if ((mouse_x>=G_Obj->x)&&(mouse_y>=G_Obj->y)&&
			(mouse_x<G_Obj->x+bmpNormal->w)&&(mouse_y<G_Obj->y+bmpNormal->h)&&
			(getpixel(bmpNormal,mouse_x-G_Obj->x,mouse_y-G_Obj->y)!=makecol(255,0,255)))				
		{
			if (iHint>0)
			{
				bHint=true;
				MakeStringHint(iHint);
			};
			if (mouse_b & 1)
			{				
				if (flag!=3)
				{
					G_Obj->Pic=bmpPressed;
					flag=3;
					if (!bButtonDelay)
					{
				//		Result=1;
					};
					bButtonDelay=true;
				};
			}
			else
			{
				if (flag!=2)
				{
					if (flag==3)
					{						
						Result=1;
					};
					G_Obj->Pic=bmpActive;
					flag=2;				
				};
			};			
		}		
		else
		{
			if (flag!=1)
			{
				G_Obj->Pic=bmpNormal;
				flag=1;				
			};
		};		
	};	
	};
	if ((bBlink)&&(flag==1))
	{
		if (Tick%4>1)
		{G_Obj->Pic=bmpActive;}
		else
		{G_Obj->Pic=bmpNormal;};
	};
	if (Result)
	{
		bHint=false;
		if (wavSound>0)
		{
			PlayUnitSound(wavSound);
		//	play_sample(wavSound,iSoundVolume,128,1000,0);
		}
		else
		{PlaySpellSound(abs(wavSound));};
	};	
	if (ipHotkey==-1)
	{
		if ((Roller<mouse_z)&&(flag>0))
		{Result=true;};
		Roller=mouse_z;
	};
	if (ipHotkey==-2)
	{
		if ((Roller>mouse_z)&&(flag>0))
		{Result=true;};
		Roller=mouse_z;
	};
	return Result;		
};

//**************************************
//*** Методы UI_Button2 ****************
//**************************************

UI_Button2::UI_Button2()
{
	flag=-1;
	iHint=0;
	ipHotkey=-100;
	ipHotkey2=-100;
	bHot=false;
	bPress=false;
	wavSound=wavClick;
};

G_Object * UI_Button2::Init(int st_x, int st_y, int st_flag, int Layer, BITMAP *Norm, BITMAP *Act, BITMAP *Press, BITMAP *Inact, BITMAP *PressAct, G_Object *Obj)
{
	flag=-10;
	bmpNormal=Norm;
	bmpActive=Act;
	bmpPressed=Press;
	bmpInactive=Inact;
	bmpPressedActive=PressAct;
	iDefLayer=Layer;
	bPress=false;
	{G_Obj=new G_Object(Obj, st_x, st_y, Layer, Norm);}	
	Activate(st_flag, false);
	Check();
	return G_Obj;
};

void UI_Button2::Activate(int st_flag, bool Pressed)
{
	if ((st_flag!=flag)||(Pressed!=bPress))
	{
		switch (st_flag)
		{
		case -1:
			G_Obj->iLayer=-1;
			break;
		case 0:
			G_Obj->iLayer=iDefLayer;
			G_Obj->Pic=bmpInactive;
			break;
		case 1:
			G_Obj->iLayer=iDefLayer;
			if (Pressed)
			{G_Obj->Pic=bmpPressed;}
			else
			{G_Obj->Pic=bmpNormal;};
			break;
//		case -2:
//			G_Obj->iLayer=iDefLayer;
//			G_Obj->Pic=bmpPressed;
//			break;
		};
		flag=st_flag;
		bPress=Pressed;
	};
};

int UI_Button2::Check()
{
//	int old_flag=flag;
	int Result=0;
	if ((bButtonDelay)&&(!(mouse_b & 1)))
	{bButtonDelay=false;};
	if (flag>0)
	{
		poll_keyboard();
		if ((((ipHotkey!=-100)&&(key[ipHotkey]))||((ipHotkey2!=-100)&&(key[ipHotkey2])))
			&&(!bInput))
		{
			if (flag!=2)
			{
				if (bPress)
					{G_Obj->Pic=bmpPressedActive;}
					else
					{G_Obj->Pic=bmpActive;}	
				flag=2;					
			};
			bHot=true;
		}
		else
		{
		if (bHot)
		{
			bPress=!bPress;
			if (bPress)
			{G_Obj->Pic=bmpPressedActive;}
			else
			{G_Obj->Pic=bmpActive;}
			flag=3;
			Result=1;					
			bHot=false;
		
		};
		if ((mouse_x>=G_Obj->x)&&(mouse_y>=G_Obj->y)&&
			(mouse_x<G_Obj->x+bmpNormal->w)&&(mouse_y<G_Obj->y+bmpNormal->h)&&
			(getpixel(bmpNormal,mouse_x-G_Obj->x,mouse_y-G_Obj->y)!=makecol(255,0,255)))				
		{
			if (iHint>0)
			{
				bHint=true;
				if (bPress)
				{MakeStringHint(iHint1);}
				else
				{MakeStringHint(iHint);};
			};
			if ((mouse_b & 1)&&(!bButtonDelay))
			{
				if (flag==2)
				{
					bPress=!bPress;
					if (bPress)
					{G_Obj->Pic=bmpPressedActive;}
					else
					{G_Obj->Pic=bmpActive;}
					flag=3;
					Result=1;
					bButtonDelay=true;
				};
			}
			else
			{
				if (flag!=2)
				{
					if (bPress)
					{G_Obj->Pic=bmpPressedActive;}
					else
					{G_Obj->Pic=bmpActive;}					
					flag=2;				
				};
			};			
		}		
		else
		{
			if (flag!=1)
			{
				if (bPress)
				{G_Obj->Pic=bmpPressed;}
				else
				{G_Obj->Pic=bmpNormal;}
				flag=1;				
			};
		};
		};
		if (Result>0)
		{
			if (wavSound>0)
			{
				PlayUnitSound(wavSound);
			//	play_sample(wavSound,iSoundVolume,128,1000,0);
			}
			else
			{PlaySpellSound(abs(wavSound));};
		};
		return bPress;
	};	
	return 0;		
};

//**************************************
//*** Методы UI_TextBox ****************
//**************************************

UI_TextBox::UI_TextBox()
{
	Pic=NULL;
	Pic1=NULL;	
	buttUp.Activate(-1);
	buttDown.Activate(-1);
	bScroll=false;
	bShadow=false;
	bCentre=false;
	bInside=false;
};

UI_TextBox::~UI_TextBox()
{
	if (bGameInProcess)
	{
	if (Pic)
	{destroy_bitmap(Pic);};
	if (Pic1)
	{destroy_bitmap(Pic1);};
	};
};

G_Object * UI_TextBox::Init(int st_x, int st_y, int st_w, int st_h, int st_Layer, int st_Color, FONT * st_Fnt, G_Object * st_Obj)
{
	x=st_x;
	y=st_y;
	w=st_w;
	h=st_h;
	int dh=h%14;
	h=h-dh;
	if (dh>7)
	{h+=14;};
	bInside=false;
	Color=st_Color;
	Layer=st_Layer;
	Fnt=st_Fnt;
	if (Pic)
	{
		destroy_bitmap(Pic);
	};
	Pic=create_bitmap(w,h);
	rectfill(Pic,0,0,w,h,COLOR_0);
	Obj=new G_Object(st_Obj,x,y,Layer,Pic);
	return Obj;
};

G_Object * UI_TextBox::Init(int st_x, int st_y, int st_w, int st_h, int st_Layer, int st_Color, FONT * st_Fnt, G_Object * st_Obj, int x_up, int y_up, int x_down, int y_down)
{
	bScroll=true;
	x=st_x;
	y=st_y;
	w=st_w;
	h=st_h;
	int dh=h%14;
	h=h-dh;
	if (dh>7)
	{h+=14;};
	Color=st_Color;
	Layer=st_Layer;
	Fnt=st_Fnt;
	if (Pic)
	{
		destroy_bitmap(Pic);
	};
	Pic=create_bitmap(w,h);
	rectfill(Pic,0,0,w,h,COLOR_0);	
	Obj=buttUp.Init(x_up, y_up, 0, Layer+1, bmpButton[48], bmpButton[49], bmpButton[50], bmpButton[51], st_Obj);
	Obj=buttDown.Init(x_down, y_down, 0, Layer+1, bmpButton[52], bmpButton[53], bmpButton[54], bmpButton[55], Obj);
	Obj=new G_Object(Obj,x,y,Layer,Pic);
	return Obj;
};

void UI_TextBox::Activate(char * Str)
{
	if (Pic1)
	{
		destroy_bitmap(Pic1);
		Pic1=NULL;
		rectfill(Pic,0,0,w,h,COLOR_0);
	};
	Text=Str;
	char sLine [500];
	char sWord [500];
	int R=0, Words=0, Cursor=0;
	Lines=0;
	bool bRead=true;
	sLine[0]='\0';
	sWord[0]='\0';
	while (bRead)
	{
		if (*(Text+R)=='\0')
		{
			bRead=false;
		};
		if ((*(Text+R)==' ')||(*(Text+R)=='\n')||(*(Text+R)=='\0'))
		{
			sWord[Words]='\0';
			if ((text_length(Fnt,sLine)+text_length(Fnt,sWord)>w))
			{				
				Cursor=0;				
				Lines++;
			};
			for (int i=0; i<Words; i++)
			{
				sLine[Cursor+i]=sWord[i];
			};
			Cursor+=Words;
			Words=0;
			sWord[0]='\0';
			if (*(Text+R)==' ')
			{
				sLine[Cursor]=' ';
				Cursor++;				
			}
			else
			{
				Cursor=0;				
				Lines++;
			};
			sLine[Cursor]='\0';
		}
		else
		{
			sWord[Words]=*(Text+R);
			Words++;
		};
		R++;
	};
	Pic1=create_bitmap(w,(Lines)*(text_height(Fnt)+2)+15);
	rectfill(Pic1,0,0,w,(Lines)*(text_height(Fnt)+2)+15,COLOR_0);
	R=0; Lines=0; Words=0; Cursor=0;
	sLine[0]='\0';
	sWord[0]='\0';
	bRead=true;
	while (bRead)
	{
		if (*(Text+R)=='\0')
		{
			bRead=false;
		};
		if ((*(Text+R)==' ')||(*(Text+R)=='\n')||(*(Text+R)=='\0'))
		{
			sWord[Words]='\0';
			if ((text_length(Fnt,sLine)+text_length(Fnt,sWord)>w))
			{				
				sLine[Cursor]='\0';
				Cursor=0;
				if (bCentre)
				{
					if (bShadow)
					{textprintf_centre_ex(Pic1,Fnt,w/2+4,Lines*(text_height(Fnt)+2)+1,COLOR_1,-1,sLine);};
					textprintf_centre_ex(Pic1,Fnt,w/2+3,Lines*(text_height(Fnt)+2),Color,-1,sLine);
				}
				else
				{
					if (bShadow)
					{textprintf_ex(Pic1,Fnt,1,Lines*(text_height(Fnt)+2)+1,COLOR_1,-1,sLine);};
					textprintf_ex(Pic1,Fnt,0,Lines*(text_height(Fnt)+2),Color,-1,sLine);
				};
				Lines++;
			};
			for (int i=0; i<Words; i++)
			{
				sLine[Cursor+i]=sWord[i];
			};
			Cursor+=Words;
			Words=0;
			sWord[0]='\0';
			if (*(Text+R)==' ')
			{
				sLine[Cursor]=' ';
				Cursor++;				
			}
			else
			{
				sLine[Cursor]='\0';
				Cursor=0;
				if (bCentre)
				{
					if (bShadow)
					{textprintf_centre_ex(Pic1,Fnt,w/2+4,Lines*(text_height(Fnt)+2)+1,COLOR_1,-1,sLine);};
					textprintf_centre_ex(Pic1,Fnt,w/2+3,Lines*(text_height(Fnt)+2),Color,-1,sLine);
				}
				else
				{
					if (bShadow)
					{textprintf_ex(Pic1,Fnt,1,Lines*(text_height(Fnt)+2)+1,COLOR_1,-1,sLine);};
					textprintf_ex(Pic1,Fnt,0,Lines*(text_height(Fnt)+2),Color,-1,sLine);
				};
				Lines++;
			};
			sLine[Cursor]='\0';
		}
		else
		{
			sWord[Words]=*(Text+R);
			Words++;
		};
		R++;
	};
	blit(Pic1,Pic,0,0,0,0,w,h);
	Obj->iLayer=Layer;
	if (bScroll)
	{
		if (Pic1->h-15-h>3)
		{
			CurrentLine=0;
			ScrollSize=((Pic1->h-h-15)-(Pic1->h-h-15)%(text_height(Fnt)+2))/(text_height(Fnt)+2);
			if ((Pic1->h-h-15)%(text_height(Fnt)+2)>3)
			{ScrollSize++;};
			buttUp.Activate(1);
			buttDown.Activate(1);
		}
		else
		{
			buttUp.Activate(0);
			buttDown.Activate(0);
		};
	};
};

void UI_TextBox::Hide()
{
	Obj->iLayer=-1;
	if (bScroll)
	{
		buttUp.Activate(-1);
		buttDown.Activate(-1);
	};
};

void UI_TextBox::Check()
{
	if ((bScroll)&&(Obj->iLayer>=0))
	{
		if ((buttUp.Check())&&(CurrentLine>0))
		{
			CurrentLine--;
			blit(Pic1,Pic,0,CurrentLine*(text_height(Fnt)+2),0,0,w,h);
		};
		if ((buttDown.Check())&&(CurrentLine<ScrollSize))
		{
			CurrentLine++;
			blit(Pic1,Pic,0,CurrentLine*(text_height(Fnt)+2),0,0,w,h);
		};
		if (Pic1->h-15-h>3)
		{
		if ((mouse_x>=x-5)&&(mouse_x<x+w+5)&&(mouse_y>y-10)&&(mouse_y<y+h+10))
		{
			if (bInside)
			{
				if ((mouse_z>Roller)&&(CurrentLine>0))
				{
					CurrentLine--;
					blit(Pic1,Pic,0,CurrentLine*(text_height(Fnt)+2),0,0,w,h);
				};
				if ((mouse_z<Roller)&&(CurrentLine<ScrollSize))
				{
					CurrentLine++;
					blit(Pic1,Pic,0,CurrentLine*(text_height(Fnt)+2),0,0,w,h);
				};
				Roller=mouse_z;
			}
			else
			{
				Roller=mouse_z;
				bInside=true;
			};
		}
		else
		{bInside=false;};
	};
	};
};

UI_Input::UI_Input()
{
	Pic=create_bitmap(1000,20);
	bCenter=false;
	Clear();
};

UI_Input::~UI_Input()
{
	bInput=false;
	if (bGameInProcess)
	{destroy_bitmap(Pic);};
};

G_Object *UI_Input::Init(int st_x, int st_y, int st_Type, int st_Max_Length, int st_Layer, int st_Color, FONT *st_Fnt, G_Object *st_Obj)
{
	x=st_x; y=st_y; Type=st_Type; Max_Length=st_Max_Length; Layer=st_Layer; Fnt=st_Fnt; Color=st_Color;
	Flag=0;
	Obj=new G_Object(st_Obj,x,y,Layer,Pic);
	return Obj;
};

void UI_Input::Clear()
{
	sprintf(Text,"");
	Length=0;
	Cursor=0;	
	rectfill(Pic,0,0,1000,20,COLOR_0);
};

void UI_Input::Activate(int FLG)
{	
	clear_keybuf();
	if (FLG<0)
	{Obj->iLayer=-1;}
	else
	{Obj->iLayer=Layer;};
	if ((Flag>0)&&(FLG<1))
	{bInput=false;};
	if (FLG>0)
	{bInput=true;};
	Flag=FLG;
	Show();
};

void UI_Input::Show()
{
	rectfill(Pic,0,0,1000,20,COLOR_0);
	char TXX[2];
//	StopLog("Curs   ",Cursor);
	sprintf(TXX," ");
//	StopLog("sprintf done   ",Cursor);
	for (int i=0; i<Cursor; i++)
	{		
		TXX[0]=Text[i];
		textprintf_ex(Pic,Fnt,i*9,5,Color,-1,TXX);
	};
//	textprintf_ex(Pic,Fnt,0,0,Color,-1,"%s",Text);
	if (Flag>0)
	{draw_sprite(Pic,bmpGUI_Cursor,Cursor*9,0);};
	if (bCenter)
	{Obj->x=x+(Max_Length-Cursor)*9/2;};
};

int UI_Input::Check(bool Active)
{
	if (Flag>0)
	{
		if (keypressed())
		{
			int C, Scan;
			C=ureadkey(&Scan);
			char CH;
			if (Scan==KEY_BACKSPACE)
			{
				if (Cursor>0)
				{
					Cursor--;
					Text[Cursor]=' ';
					Show();
				};
				return 1;
			};
			if ((Scan==KEY_ENTER)||(Scan==KEY_ENTER_PAD)||(Scan==KEY_ESC))
			{
				if (Active)
				{Activate(0);};
				return 2;
			};
			bool bType=false;
			if (Cursor<Max_Length)
			{
				if (Type==1)
				{
					if ((C>47)&&(C<58))
					{
						CH=C;
						bType=true;
					};
				}
				else
				{
					if (C>1000)
					{
						CH=C-1104;
						bType=true;
					}
					else
					{
						if ((C>31)&&(C<129))
						{
							CH=C;
							bType=true;
						};
					};
				};
				if (bType)
				{
					if (Cursor==Length)
					{
						sprintf(Text,"%s ",Text);
						Length++;
					};
					Text[Cursor]=CH;
					Cursor++;
					Show();
				};
			};
	//		textprintf(Pic,Fnt,500,0,Color,"(%d)     ",C);
		};
		return 1;
	};
	return 0;
};

void UI_Input::EnterInt(int Zifer)
{
	Clear();
	sprintf(Text,"%d",Zifer);
	while (Zifer>0)
	{
	//	sprintf(Text,"%d%s",Zifer%10,Text);
		Zifer=(Zifer-Zifer%10)/10;
		Cursor++;
	};
	Length=Cursor;
};

long int UI_Input::GetZyfer()
{
	int Result=0, Count=0, ZZ;
	if (Type==1)
	{
	while (Count<Cursor)
	{
		ZZ=Text[Count];
		ZZ-=48;
		Result=Result*10+ZZ;
//		textprintf(screen,Font1,0,0,COLOR_2,"%s: %d, %d",Str,Count, ZZ);
//		clear_keybuf();
//		readkey();
		Count++;
	};
	};
//	textprintf(screen,Font1,0,0,COLOR_2,"Результат: %d",Result);
//	clear_keybuf();
//	readkey();
	return Result;
};


//**************************************
//*** Методы UI_ScrollBar **************
//**************************************

UI_ScrollBar::UI_ScrollBar()
{
	PicN=bmpGUI_ScrollN;
	PicA=bmpGUI_ScrollA;
	pValue=NULL;
	pText=NULL;
	Flag=1;
};

UI_ScrollBar::~UI_ScrollBar()
{};

G_Object * UI_ScrollBar::Init(int st_x, int st_y, int st_Length, int st_Width, int st_Layer, bool Vertical, bool Buttons, G_Object * st_Obj)
{
	x=st_x; y=st_y; Length=st_Length; Width=st_Width; Layer=st_Layer; bVertical=Vertical;
	Obj=new G_Object(st_Obj,x-2,y-2,Layer,PicN);
	G_Object *NObj=Obj;
	if (Buttons)
	{		
		if (Vertical)
		{}
		else
		{
			NObj=buttSub.Init(x-15,y-3,1,Layer,bmpButton[293],bmpButton[294],bmpButton[295],bmpButton[296],NObj);
			NObj=buttAdd.Init(x+Length+14,y-3,1,Layer,bmpButton[289],bmpButton[290],bmpButton[291],bmpButton[292],NObj);
		};
	};
	return NObj;
};

void UI_ScrollBar::AttachValue(int *Zifer, int MinVal, int MaxVal)
{
	Min=MinVal;
	Max=MaxVal;	
	pValue=Zifer;	
	Show();
};

void UI_ScrollBar::Activate(int FLG)
{
	Flag=FLG;
	buttAdd.Activate(Flag);
	buttSub.Activate(Flag);
	if (FLG<0)
	{		
		Obj->iLayer=-1;		
	}
	else
	{		
		Obj->iLayer=Layer;
		Show();
	};	
};

void UI_ScrollBar::Show()
{
	if ((pValue==NULL)||(Max<=Min))
	{
		if (bVertical)
		{
			Obj->y=y+Length/2;
		}
		else
		{
			Obj->x=x+Length/2;
		};
	}
	else
	{
		if (*pValue>Max)
		{*pValue=Max;};
		if (*pValue<Min)
		{*pValue=Min;};
		if (bVertical)
		{
			Obj->y=y+Length-(*pValue-Min)*Length/(Max-Min);			
		}
		else
		{
			Obj->x=x+(*pValue-Min)*Length/(Max-Min);
		};
	};
};

int UI_ScrollBar::Check()
{
	if (Flag>0)
	{
		if (bVertical)
		{
			if (((mouse_x>=x-2)&&(mouse_y>=y)&&(mouse_x<x+Width)&&(mouse_y<y+Length))||((Flag==3)&&(mouse_b & 1)))
			{
				if (Flag<2)
				{
					Flag=2;
					Obj->Pic=PicA;
				};
				if (mouse_b & 1)
				{
					Flag=3;
					if (Max>Min)
					{
						*pValue=Min+(Max-Min)*(y-mouse_y+Length)/Length;
						if (*pValue<Min)
						{*pValue=Min;};
						if (*pValue>Max)
						{*pValue=Max;};
						Show();
					};					
				};
			}
			else
			{
				if (Flag>1)
				{
					Flag=1;
					Obj->Pic=PicN;
				};
			};
		}
		else
		{
			if (((mouse_x>=x)&&(mouse_y>=y-2)&&(mouse_x<x+Length)&&(mouse_y<y+Width)&&(mouse_y<y+Length))||((Flag==3)&&(mouse_b & 1)))
			{
				if (Flag<2)
				{
					Flag=2;
					Obj->Pic=PicA;
				};
				if (mouse_b & 1)
				{
					Flag=3;
					if (Max>Min)
					{
						*pValue=Min+(Max-Min)*(mouse_x-x)/Length;
						if (*pValue<Min)
						{*pValue=Min;};
						if (*pValue>Max)
						{*pValue=Max;};
						Show();
					};					
				};
			}
			else
			{
				if (Flag>1)
				{
					Flag=1;
					Obj->Pic=PicN;
				};
			};

		};
	};
	if ((*pValue==Min)&&(buttSub.flag>0))
	{buttSub.Activate(0);};
	if ((*pValue>Min)&&(buttSub.flag==0))
	{buttSub.Activate(1);};
	if ((*pValue==Max)&&(buttAdd.flag>0))
	{buttAdd.Activate(0);};
	if ((*pValue<Max)&&(buttAdd.flag==0))
	{buttAdd.Activate(1);};
	if (Flag!=3)
	{
		if (buttSub.Check())
		{
			*pValue-=1;
			Show();
			Flag=4;
		};
		if (buttAdd.Check())
		{
			*pValue+=1;
			Show();
			Flag=4;
		};
	};
	return Flag;
};

void ShowIconNumber(BITMAP *bmpPic, int x, int y, int Num)
{
	if (Num<10)
	{
		draw_sprite(bmpPic,bmpStuff[32],x+37,y+32);
		textprintf_ex(bmpPic,Font1,x+40,y+36,COLOR_1,-1,"%d",Num);
	}
	else
	{
		draw_sprite(bmpPic,bmpStuff[33],x+29,y+33);
		textprintf_centre_ex(bmpPic,Font1,x+39,y+37,COLOR_1,-1,"%d",Num);
	};
};

void ShowIconString(BITMAP *bmpPic, int x, int y, char * Str)
{
	int Length=text_length(Font1,Str);
	if (Length>40)
	{
		blit(bmpStuff[72],bmpPic,0,0,x+7,y+32,bmpStuff[72]->w,bmpStuff[72]->h);
		textprintf_centre_ex(bmpPic,Font1,x+32,y+36,COLOR_1,-1,Str);
	}
	else
	{
		blit(bmpStuff[72],bmpPic,0,0,x+7+(40-Length),y+32,Length+2,bmpStuff[72]->h);
		blit(bmpStuff[72],bmpPic,bmpStuff[72]->w-2,0,x+7+42,y+32,2,bmpStuff[72]->h);
		textprintf_centre_ex(bmpPic,Font1,x+49-Length/2,y+36,COLOR_1,-1,Str);
	};
};

void ShowIconPercent(BITMAP *bmpPic, int x, int y, int Num)
{
	char Str[100];
	sprintf(Str,"%d~",Num);
	ShowIconString(bmpPic, x, y, Str);
};

void MouseTime()
{poll_mouse();};
