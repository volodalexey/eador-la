#include "Unit.h"

DATAFILE *datUnitSound;
DATAFILE *datSpellSound;
DATAFILE *datMusic;
int MainTrack=-1;
ALOGG_OGG *oggTrack[MAX_MUSIC];
clAudioTrack trSound[MAX_SOUNDTRACK];


clAudioTrack::clAudioTrack()
{
	bActive=false;
	bFree=true;
	Track=NULL;
};

clAudioTrack::~clAudioTrack()
{};

void clAudioTrack::Start(int Volume, float Fading, int Pan, int Speed, int Loop)
{
//	Track=Sound;
	bFree=false;
	iVolume=Volume;
	fVolume=Volume;
	iOld=Volume;
	fFading=Fading;
	iPan=Pan;
	iSpeed=Speed;
	iLoop=Loop;
	ipUnit=-1;
	if (Track)
	{
		alogg_play_ex_ogg(Track,iOggBuffer,iVolume,iPan,iSpeed,iLoop);
		bActive=true;
	}
	else
	{
		FileLog("!!!!!!!!!Alogg error");
		bActive=false;
	};
	bFree=true;
//	Poll();
};

void clAudioTrack::Poll()
{
	if ((!bActive)||(!bFree))
	{return;};
	bFree=false;
	if (fFading!=0)
	{
		fVolume+=fFading;
		if (fVolume<=0)
		{
			Clear();			
			bFree=true;
			return;
		};
		if (fVolume>=iMusicVolume)
		{
			fVolume=iMusicVolume;
			fFading=0;
		};
		iVolume=fVolume;
	};
	if (iOld!=iVolume)
	{alogg_adjust_ogg(Track, iVolume, iPan, iSpeed, iLoop);};
	if (alogg_poll_ogg(Track)!=ALOGG_OK)
	{Clear();};
	iOld=iVolume;
	bFree=true;
};

void clAudioTrack::Clear()
{
	bActive=false;
	if (Track)
	{
		alogg_stop_ogg(Track);
//		alogg_rewind_ogg(Track);
	};
	alogg_destroy_ogg(Track);
	Track=NULL;
	if (Index==MainTrack)
	{MainTrack=-1;};	
};

int PlayUnitSound(int Index)
{
	if (bSound)
	{
		for (int i=0; i<MAX_SOUNDTRACK-1; i++)
		{
			if ((trSound[i].Track==NULL)&&(trSound[i].bFree))
			{
				trSound[i].Track=alogg_create_ogg_from_buffer(datUnitSound[Index].dat, datUnitSound[Index].size);				
				trSound[i].Start(iSoundVolume, 0, 128, 1000, 0);
				trSound[i].Index=i;
				trSound[i].ipUnit=Index;
				return i;
			}
			else
			{
				if ((trSound[i].bActive)&&(trSound[i].ipUnit==Index)
					&&(alogg_get_length_msecs_ogg(trSound[i].Track)<1200))
				{
			//		int AAA=alogg_get_length_msecs_ogg(trSound[i].Track);
					return i;
				};
			};
		};
	};
	return -1;
};

int PlaySpellSound(int Index)
{
	if (bSound)
	{
		for (int i=0; i<MAX_SOUNDTRACK; i++)
		{
			if ((trSound[i].Track==NULL)&&(trSound[i].bFree))
			{
				trSound[i].Track=alogg_create_ogg_from_buffer(datSpellSound[Index].dat, datSpellSound[Index].size);
				if (Index==0)
				{trSound[i].Start(iSoundVolume/2, 0, 128, 1000, 0);}
				else
				{trSound[i].Start(iSoundVolume, 0, 128, 1000, 0);};				
				trSound[i].Index=i;
				return i;
			};
		};
	};
	return -1;
};

int PlayMusic(int Index, int Volume, float Fading, int Pan, int Speed, int Loop)
{
	if (bSound)
	{
		for (int i=0; i<MAX_SOUNDTRACK; i++)
		{
			if ((trSound[i].Track==NULL)&&(trSound[i].bFree))
			{
				trSound[i].Track=alogg_create_ogg_from_buffer(datMusic[Index].dat, datMusic[Index].size);
				trSound[i].Start(Volume, Fading, Pan, Speed, Loop);
				trSound[i].Index=i;
				return i;
			};
		};
	};
	return -1;
};

int iMusicMode=-1;

//**************************************
//*** Звуковое сопровождение ***********
//**************************************

void ChangeMusic()
{
	if ((bSound)&&((iGameMode!=iMusicMode)||(MainTrack<0)))
	{
		iMusicMode=iGameMode;
		int Vol=iMusicVolume; float Fade=0;
		if ((MainTrack>=0)&&(iMusicVolume>0))
		{
			trSound[MainTrack].fFading=-MAX_FADE_SPEED*iMusicVolume; trSound[MainTrack].fFading/=255;
			Vol=1; Fade=MAX_FADE_SPEED*iMusicVolume; Fade/=255;
		};	
		int Wrk;
		switch (iGameMode)
		{
		case 0:
			MainTrack=PlayMusic(MUZ_MENU,Vol,Fade,128,1000,0);
			break;
		case 1:
			MainTrack=PlayMusic(MUZ_ASTRAL,Vol,Fade,128,1000,0);
			break;
		case 2:
			iActiveTrack++;
			if (iActiveTrack>MUZ_MAX_MAIN)
			{iActiveTrack=1;};
			MainTrack=PlayMusic(MUZ_MAIN+iActiveTrack-1,Vol,Fade,128,1000,0);			
			break;
		case 3:
			Wrk=Random(MUZ_MAX_WAR);
			MainTrack=PlayMusic(MUZ_WAR+Wrk,Vol,Fade,128,1000,0);
			break;
			/*switch (Random(4))
			{
			case 0:
				MainTrack=PlayMusic(11,Vol,Fade,128,1000,0);
				break;
			case 1:
				MainTrack=PlayMusic(12,Vol,Fade,128,1000,0);
				break;
			case 2:
				MainTrack=PlayMusic(13,Vol,Fade,128,1000,0);
				break;
			case 3:
				MainTrack=PlayMusic(14,Vol,Fade,128,1000,0);
				break;
			};
			break;*/
		case 4:
			MainTrack=PlayMusic(MUZ_PREPARE,Vol,Fade,128,1000,0);
			break;
		case 5:
			MainTrack=PlayMusic(MUZ_MEMORY,Vol,Fade,128,1000,0);
			break;
		case 6:
			MainTrack=PlayMusic(MUZ_VICTORY,Vol,Fade,128,1000,0);
			break;
		case 7:
			MainTrack=PlayMusic(MUZ_LOSE,Vol,Fade,128,1000,0);
			break;
		case 8:
			MainTrack=PlayMusic(MUZ_GREMLIN,Vol,Fade,128,1000,1);			
			break;
		};

		/*if ((Track[1])&&(Track[1]->isPlaying()))
		{Track[1]->stop();};	
		if (Track[0]->isPlaying())
		{
			Track[1]=Track[0];
			fFading=MUSIC_FADING;		
		}
		else
		{fFading=0;};
		switch (iGameMode)
		{
		case 0:
			Track[0]=Music[8];
			break;
		case 1:
			Track[0]=Music[9];
			break;
		case 2:
			iActiveTrack++;
			if (iActiveTrack>7)
			{iActiveTrack=1;};			
			Track[0]=Music[iActiveTrack-1];
			break;
		case 3:
			switch (Random(4))
			{
			case 0:
				Track[0]=Music[11];
				break;
			case 1:
				Track[0]=Music[12];
				break;
			case 2:
				Track[0]=Music[13];
				break;
			case 3:
				Track[0]=Music[14];
				break;
			};
			break;
		case 4:
			Track[0]=Music[10];
			break;
		case 5:
			Track[0]=Music[7];
			break;
		case 6:
			Track[0]=Music[15];
			break;
		case 7:
			Track[0]=Music[16];
			break;
		case 8:
			Track[0]=Music[17];
			Track[0]->setRepeat(true);
			break;
		};		
		if (fFading>0)
		{Track[0]->setVolume(0);}
		else
		{
			float Vol=iMusicVolume;
			Vol/=255;
			Track[0]->setVolume(Vol);
		};
		Track[0]->reset();
		Track[0]->play();*/
	};
};

void ScrollSound()
{
	if (Tutorial.Mode)
	{
		PlayUnitSound(wavParchment);
	//	play_sample(wavParchment,iSoundVolume,128,1000,0);
	};
};

void CoinSound()
{	
	PlayUnitSound(wavCoins);
//	play_sample(wavCoins,iSoundVolume/4,128,1000,0);
};

//**************************************
//*** Обновление экрана ****************
//**************************************

void ItIsTime()
{
	try{
	Timer=true;
	poll_keyboard();
	if ((!bScrShot)&&(key[KEY_F1]))
	{bScrShot=true;};
	if ((bDarken)&&(!bHourGlass))
	{
		draw_trans_sprite(screen1,bmpDarkScreen,0,0);
		blit(screen1,screen,0,0,0,0,1024,768);
	};	
	if (bSound)
	{
		for (int i=0; i<MAX_SOUNDTRACK; i++)
		{
			if (trSound[i].Track)
			{
				try
				{
					trSound[i].Poll();
				}
				catch(...)
				{FileLog("OggPoll Error");};
			};
		};
	};
	}
	catch(...)
	{FileLog("ItIsTime Error");};
};

int HourY=0;

void AnimHourglass()
{
	try
	{
	if (bHourGlass)
	{		
		draw_sprite(screen1,bmpHourGlass[iHourGlass],512-bmpGUI_NewTurn->w/2+48,384-bmpGUI_NewTurn->h/2+43+HourY);
		iHourGlass++;
		if (iHourGlass==13)
		{iHourGlass=0;};
		if (NewProgress/100>OldProgress/100)
		{
			for (int i=OldProgress/100; ((i<=NewProgress/100)&&(i<220)); i++)
			{draw_sprite(screen1,bmpGUI_ProgressFill,512-bmpGUI_NewTurn->w/2+122+i,384-bmpGUI_NewTurn->h/2+67+HourY);};
			draw_sprite(screen1,bmpGUI_ProgressMark,512-bmpGUI_NewTurn->w/2+121,384-bmpGUI_NewTurn->h/2+70+HourY);
			OldProgress=NewProgress;
		};
		blit(screen1,screen,0,0,0,0,1024,768);
		if (bDarken)
		{bHourGlass=false;};
	};
	
	if ((bSound)&&(iGameMode>=0))
	{		
		iPause--;
		if (iPause==0)
		{
			try
			{
			if (MainTrack<0)
			{ChangeMusic();};
			}
			catch(...)
			{};
			iPause=20+Random(100);
		};
	};
	Tick++;
	if (Tick>1000)
	{Tick=0;};
	}
	catch(...)
	{FileLog("HourGlass Error");};
};

void RunHourGlass(int iPhrase, int DY)
{
	HourY=DY;	
	if (bHourGlass)
	{blit(bmpGUI_NewTurn,screen1,122,42,122+512-bmpGUI_NewTurn->w/2,42+384-bmpGUI_NewTurn->h/2+HourY,226,23);}
	else
	{draw_sprite(screen1,bmpGUI_NewTurn,512-bmpGUI_NewTurn->w/2,384-bmpGUI_NewTurn->h/2+HourY);}
	textprintf_centre_ex(screen1,Font1,512-bmpGUI_NewTurn->w/2+234,384-bmpGUI_NewTurn->h/2+46+HourY,COLOR_1,-1,"%s",(pFirstPhrase+iPhrase)->str);	
	if (!bHourGlass)
	{iHourGlass=0;};
	draw_sprite(screen1,bmpHourGlass[iHourGlass],512-bmpGUI_NewTurn->w/2+48,384-bmpGUI_NewTurn->h/2+43+HourY);
	bHourGlass=true;
};