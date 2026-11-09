#include <iostream>
#include <fstream>
#include "Unit.h"
#include "Logging.h"

//Указатели на первые элементы массивов прототипов:	
	stString		*pFirstPhrase;
	stString		*pName[4];
	stString		*pFirstUnitClass;
	stString		*pFirstUnitKind;
	stString		*pFirstItemType;
	stMessage		*pFirstMessage;
	stMessage		*pFirstTutText;
	stMessage		*pFirstHint;
//	stAbility		*pFirstAbility;
	stAbilityNum	*pFirstAbilityNum;
	stEffect		*pFirstEffect;
	clTerrainType	*pFirstTerrain;
	clBF_Object		*pFirstBF_Obj;
	clResource		*pFirstResource;
	clRace			*pFirstRace;
	clUnitUpgrade	*pFirstUnitUpg;
	clUnitType		*pFirstUnitType;
	clSpell			*pFirstSpell;
	clItem			*pFirstItem;
	clGuard			*pFirstGuard;
	clGuardType		*pFirstGuardType;
	clProvinceType	*pFirstProvinceType;
	clOuterBuilding	*pFirstOuterBld;
	clDefender		*pFirstDefender;
	clSite			*pFirstSite;
	clRitual		*pFirstRitual;
	clProvinceName	*pFirstProvinceName;
	clDialog		*pFirstDialog;
	clDialog		*pFirstCampDialog;
	clQuestion		*pFirstQuestion;
	clInnerBuilding	*pFirstInnerBld;
	clPlayerProfile	*pFirstPlayerProfile;
	clFog			*pFirstFog;
	clItemSet		*pFirstItemSet;
	clMedal			*pFirstMedal;
	clEvent			*pFirstEvent;
	clEvent			*pFirstCampEvent;
	clEncounter		*pFirstEncounter;
	clQuest			*pFirstQuest;
	clMapObject		*pFirstMapObject;
	clLandmark		*pFirstLandmark;
//	clMap			MapSet;
	clCastle		*pFirstCastle;
	clHeroAdvice	*pFirstHeroAdvice;
	clHeroAdvice	*pFirstNick;
	clHeroAdvice	*pFirstTitle;
	clHeroAdvice	*pFirstAttitude;
	clShardBonus	*pFirstShardBonus;
	stString		*pFirstPrefix;
	stString		*pFirstSuffix;
	clJournal		*pFirstJournal;
	clCredits		*pFirstCredits;
	clWorldRules	*pFirstWorldRules;
	clDifficulty	arDifficulty[MAX_DIFFICULTY];

	clDipText * pHumanDiplo;

//Максимальное количество прототипов
	int iMaxPhrases,
		iMaxName,
		iMaxUnitClass,
		iMaxUnitKind,
		iMaxItemType,
//		iMaxAbility,
		iMaxAbilityNum,
		iMaxEffect,
		iMaxTerrain,
		iMaxBF_Object,
		iMaxResource,
		iMaxRace,
		iMaxUnitUpg,
		iMaxUnitType,
		iMaxSpell,
		iMaxItem,
		iMaxPureItem,
		iMaxDefItem,
		iMaxBldItem,
		iMaxGuard,
		iMaxGuardType,
		iMaxProvinceType,
		iMaxOuterBld,
		iMaxDefender,
		iMaxSite,
		iMaxRitual,
		iMaxProvinceName,
		iMaxDialog,
		iMaxCampDialog,
		iMaxQuestion,
		iMaxInnerBld,
		iMaxBldGroup,
		iMaxPlayerProfile,
		iMaxMessage,
		iMaxItemSet,
		iMaxMedal,
		iMaxEvent,
		iMaxCampEvent,
		iMaxEncounter,
		iMaxQuest,
		iMaxMapObject,
		iMaxLandmark,
		iMaxCastle,
		iMaxTutText,
		iMaxHint,
		iMaxHeroAdvice,
		iMaxAttitude,
		iMaxNick,
		iMaxTitle,
		iMaxShardBonus,
		iMaxPrefix,
		iMaxSuffix,
		iMaxJournal,
		iMaxCredits,
		iMaxWorldRules;

int Check1=0, Check2=0, Check3=0, Check4=0, Pirate1=0, Pirate2=0, Pirate3=0, Pirate4=0;
//char stLock[40], stReg[40], stKey[40], stSub[40];
int arComp[5];
int iSleepTime=24;

BITMAP *bmpIllustration[MAX_ILLUSTRATION_ARR];
stString GroupName[MAX_BLD_GROUP];
stString ClassName[MAX_CLASS_NAME];
BITMAP * ClassPic[MAX_CLASS_NAME];
stMessage ClassText[MAX_CLASS_NAME], ClassDesc[MAX_CLASS_NAME];
//char sMapFile[100], sSaveFile[100];
stString sMapFile;
int arTalent[MAX_CLASS_NAME][MAX_TALENT][2];
stString ItemSlotName[MAX_ITEM_SLOT_NAME];
clSkill arSkills [MAX_HERO_SKILL+1];
stMessage arSkillText [MAX_HERO_SKILL+1];
clProvinceLevel ProvLevel [MAX_PROVINCE_LEVEL+1];
stShardSize arShardSize[MAX_SHARD_SIZE];
clHighScore arRecords[10], arCampRecords[10];
int *pWorkSet;
int iHeroPriceMult;			//Множитель цены для найма героя той же профессии
int arHeroPrice[4][2];		//Цены на найм героев с 1 по 4 (золото, кристаллы)
int arHeroRess[2][2];		//Цена на воскрешение героя 0 уровня и добавка цены за каждый уровень
int arStat [3][10][3];		//Параметры героев (индекс, уровень, влияние на определённую величину)
int arStatProgress [3][20];
int arBldGroup[MAX_BLD_GROUP];
int arMercenary[MAX_MERC_LEVEL][MAX_MERCENARY][2];
int arTerrLayer[MAX_TERRAIN+2][MAX_TERR_LAYER];
int arGroupCoord[MAX_GROUP][MAX_GROUP_BUILD][2];
int COLOR_0,COLOR_1,COLOR_2,COLOR_3;
int arCampDiff[MAX_DIFFICULTY][5]; //Изменение уровня сложности в кампании (начальный, конечный, интервал в ходах, лимит ходов)

int flgEncResult;
bool bExitGame;
bool bQuestComplete;

PACKFILE *LogFile;



clTerrainType::clTerrainType()
{};

clTerrainType::~clTerrainType()
{};

clResource::clResource()
{};

clResource::~clResource()
{};

clRace::clRace()
{
	for (int i=0; i<MAX_ENEMY; i++)
	{
		arEnemy[i]=0;
	};
	for (int i=0; i<MAX_RACE_QUEST; i++)
	{
		arQuest[i]=0;
	};
};

clRace::~clRace()
{};


 clUnitType::clUnitType()
{
	for (int i=0; i<MAX_UNIT_RES; i++)
	{
		arRes[i]=0;
	};

	for (int i=0; i<MAX_UNIT_ABIL; i++)
	{
		arAbility[i]=0;
	};
	
	for (int i=0; i<2; i++)
	{
		for (int j=0; j<20; j++)
		{
			for (int k=0; k<MAX_UPG_PER_LVL; k++)
			{
				arUpg[i][j][k]=0;
			}
		}
	};
	for (int i=0; i<2; i++)
	{
		for (int j=0; j<20; j++)
		{
			for (int k=0; k<MAX_LOOT_PER_LVL; k++)
			{
				arLoot[i][j][k]=0;
			}
		}
	};
	Pic=NULL;
	Shadow=NULL;
 };

clUnitType::~clUnitType()
{};

clUnitUpgrade::clUnitUpgrade()
{
	for (int i=0; i<MAX_UPGRADE_SIZE; i++)
	{
		for (int j=0; j<2; j++)
		{
			arUpg[i][j]=0;
		}
	}
	for (int i=0; i<MAX_UPGRADE_NEED; i++)
	{
		ipNeed[i]=0;
	};
};

clUnitUpgrade::~clUnitUpgrade()
{};

clBF_Object::clBF_Object()
{};

clBF_Object::~clBF_Object()
{};

clSpell::clSpell()
{
	for (int k=0; k<MAX_UNIT_KIND; k++)
	{
		arUnitKind[k]=0;
	};
	
	for (int k=0; k<MAX_SPELL_EFFECT; k++)
	{
		arEffect[0][k]=0;
	};
	for (int k=0; k<MAX_ANTI_EFFECT; k++)
	{
		arAntiEffect[k]=0;
	};
	iResistMod=0;
};

clSpell::~clSpell()
{};

clItem::clItem()
{
	for (int i=0; i<MAX_ITEM_RES; i++)
	{
		arRes[i]=0;
	};
	for (int i=0; i<MAX_ITEM_EFFECT; i++)
	{
		arEffect[i][0]=0;
		arEffect[i][1]=0;
	};
};

clItem::~clItem()
{};

clDefender::clDefender()
{
	for (int i=0; i<MAX_DEFENDER_RES; i++)
	{arRes[i]=0;};
	for (int i=0; i<MAX_GUARD_UNIT; i++)
	{arUnit[i][0]=0;};
	for (int i=0; i<MAX_GUARD_LOOT; i++)
	{arLoot[i][0]=0;};
	for (int i=0; i<MAX_DEFENDER_ABILITY; i++)
	{arAbility[i][0]=0;};

};

clDefender::~clDefender()
{};

int clDefender::CheckAbility(int Index)
{
	int Result=0;
	for (int i=0; i<MAX_DEFENDER_ABILITY; i++)
	{
		if (arAbility[i][0]==Index)
		{Result+=arAbility[i][1];};
	};
	return Result;
};

clSite::clSite()
{
	for (int i=0; i<MAX_SITE_GUARD; i++)
	{arGuard[i][0]=0;};
	for (int i=0; i<MAX_SITE_ABILITY; i++)
	{arAbility [i][0]=0;};
//	for (int i=0; i<MAX_SITE_EVENT; i++)
//	{arEvent[i][0]=0;};
};

clSite::~clSite()
{};

int clSite::CheckAbility(int Index)
{
	int Result=0;
	for (int i=0; i<MAX_SITE_ABILITY; i++)
	{
		if (arAbility[i][0]==Index)
		{Result+=arAbility[i][1];};
	};
	return Result;
};

clRitual::clRitual()
{
	for (int i=0; i<MAX_RITUAL_RES; i++)
	{arRes[i]=0;};
	for (int i=0; i<MAX_RITUAL_EFFECT; i++)
	{arEffect[i][0]=0;};
};

clRitual::~clRitual()
{};

int clRitual::GetAbility(int Index)
{
	int Result=0;
	for (int i=0; i<MAX_RITUAL_EFFECT; i++)
	{
		if (arEffect[i][0]==Index)
		{Result+=arEffect[i][1];};
	};
	return Result;
};

clProvinceName::clProvinceName()
{
	for (int i=0; i<=MAX_TERRAIN; i++)
	{arTerrain[i]=0;};
	for (int i=0; i<MAX_NAME_TYPE; i++)
	{arType[i]=0;};
};

clProvinceName::~clProvinceName()
{};

bool clProvinceName::CheckTerrain(int Terr)
{
	bool Result=true;
	for (int i=0; i<=MAX_TERRAIN; i++)
	{
		if (arTerrain[i]>0)
		{
			if (arTerrain[i]==Terr)
			{return true;};
			Result=false;
		};
	};
	return Result;
};

bool clProvinceName::CheckType(int Type)
{
	bool Result=true;
	for (int i=0; i<MAX_NAME_TYPE; i++)
	{
		if (arType[i]>0)
		{
			if (arType[i]==Type)
			{return true;};
			Result=false;
		};
	};
	return Result;
};

clDialog::clDialog()
{
	for (int i=0; i<MAX_ANSWER; i++)
	{
		arAnswer[i][1]='#';
	};
};

clInnerBuilding::clInnerBuilding()
{
	for (int i=0; i<MAX_BUILDING_NEED; i++)
	{arBuildings[i]=0;};
	for (int i=0; i<MAX_BUILDING_RES; i++)
	{arRes [i]=0;};
	for (int i=0; i<MAX_BUILDING_ABILITY; i++)
	{arAbility [i][0]=0;};
};

clInnerBuilding::~clInnerBuilding()
{};

clItemSet::clItemSet()
{
	for (int i=0; i<MAX_SET_ITEMS; i++)
	{
		arItems[i]=0;
	};

	for (int i=0; i<MAX_SET_BONUS; i++)
	{
		arBonusQ[i]=0;
		for (int j=0; j<MAX_SET_ABILITY; j++)
		{arBonus[i][j][0]=0;};
	};
};

clItemSet::~clItemSet()
{};

clMedal::clMedal()
{
	for (int i=0; i<MAX_MEDAL_EFFECT; i++)
	{
		arEffect[i][0]=0;
	};
};

clMedal::~clMedal()
{};

clEvent::clEvent()
{
	for (int i=0; i<MAX_ANSWER; i++)
	{
		arEvent[i]=0;
	};
	for (int i=0; i<MAX_EVENT_EFFECT; i++)
	{
		arEffect[i][0]=0;
	};
};

clEvent::~clEvent()
{};

clEncounter::clEncounter()
{
	for (int i=0; i<MAX_ANSWER; i++)
	{
		arEncounter[i]=0;
	};
	for (int i=0; i<MAX_EVENT_EFFECT; i++)
	{
		arEffect[i][0]=0;
	};
};

clEncounter::~clEncounter()
{};

clQuest::clQuest()
{
	for (int i=0; i<MAX_QUEST_TASK; i++)
	{
		Index[i]=0;
		Param[i]=0;
		Quantity[i]=0;		
		Target[i]=0;
		Place[i]=0;
	};
	for (int i=0; i<MAX_QUEST_ABILITY; i++)
	{
		for (int j=0; j<4; j++)
		{
			arAbility[i][j]=0;
		};
	};
};

clQuest::~clQuest()
{};

clMapObject::clMapObject()
{
	for (int i=0; i<MAX_TERRAIN+2; i++)
	{arLand[i]=0;};
	for (int i=0; i<MAX_MAPOBJ_TYPE; i++)
	{arType[i]=0;};
	for (int i=0; i<MAX_MAPOBJ_PIC; i++)
	{arPic[i]=NULL;};
};

clMapObject::~clMapObject()
{};

clLandmark::clLandmark()
{
	for (int i=0; i<MAX_TERRAIN+2; i++)
	{arLand[i]=-1;};
	for (int i=0; i<MAX_MAPOBJ_TYPE; i++)
	{arType[i]=0;};
	Shadow=NULL;
};

clLandmark::~clLandmark()
{};

clShardBonus::clShardBonus()
{
	for (int i=0; i<MAX_SHARD_LEVEL; i++)
	{
		for (int j=0; j<MAX_SHARD_BONUS_TYPE; j++)
		{
			arLevel[i][j][0]=0;
			arLevel[i][j][1]=0;
		};
	};
};

clShardBonus::~clShardBonus()
{};

clPlayerProfile::clPlayerProfile()
{
	for (int i=0; i<MAX_PLAYER_BLD; i++)
	{arBuild[i]=0;};
	for (int i=0; i<MAX_PLAYER_DIALOG; i++)
	{
		arEnterDlg[i]=0;
		arExitDlg[i]=0;
	};
};

clPlayerProfile::~clPlayerProfile()
{};

clWorldRules::clWorldRules()
{
	for (int i=0; i<MAX_WORLD_EVENTS; i++)
	{arRandomEvents[i][0]=0;};
	for (int i=0; i<MAX_WORLD_PROVTYPES; i++)
	{arProvTypes[i][0]=0;};
	for (int i=0; i<MAX_WORLD_SITES; i++)
	{arSites[i][0]=0;};
};

clWorldRules::~clWorldRules()
{};


using namespace std;

//*****************************************************************************************
//*********   Генератор ключей   **********************************************************
//*****************************************************************************************

void GenerateKeys()
{
	ofstream Keygen ("keys.txt");
	char txtKey[20];
	long int Num, iKey;
	long int arNum[12];
	for (long int i=0; i<100; i++)
	{
		Num=(012345+i*123+(i*123)/1000000)%1000000;
		arNum[4]=Num%10;
		Num/=10;
		arNum[0]=Num%10;
		Num/=10;
		arNum[10]=Num%10;
		Num/=10;
		arNum[9]=Num%10;
		Num/=10;
		arNum[3]=Num%10;
		Num/=10;
		arNum[7]=Num%10;
		
		arNum[1]=abs((arNum[7]+arNum[3]+arNum[9]+arNum[4])/(arNum[10]+1)+arNum[0]+3)%10;
		arNum[8]=abs(arNum[1]+arNum[10]+arNum[4]+arNum[7]*arNum[3]/(arNum[0]+1)+8)%10;
		arNum[2]=abs(arNum[1]+arNum[8]-arNum[4]+arNum[10]*arNum[9]/(arNum[3]+1)+7)%10;
		arNum[6]=abs(arNum[1]+arNum[7]-arNum[2]+arNum[3]+arNum[8]*arNum[9]/(arNum[4]+1)+3)%10;
		arNum[11]=abs(arNum[6]*arNum[8]*arNum[1]/(arNum[3]+2)+arNum[2]-arNum[10]-arNum[0]+arNum[4]+2)%10;
		arNum[5]=abs(arNum[10]+arNum[11]+arNum[0]-arNum[7]-arNum[3]+arNum[2]*arNum[10]/(arNum[3]+1)+9)%10;
		sprintf(txtKey,"0000-0000-0000");

		txtKey[0]=arNum[0]+48;
		txtKey[1]=arNum[1]+48;
		txtKey[2]=arNum[2]+48;
		txtKey[3]=arNum[3]+48;

		txtKey[5]=arNum[4]+48;
		txtKey[6]=arNum[5]+48;
		txtKey[7]=arNum[6]+48;
		txtKey[8]=arNum[7]+48;

		txtKey[10]=arNum[8]+48;
		txtKey[11]=arNum[9]+48;
		txtKey[12]=arNum[10]+48;
		txtKey[13]=arNum[11]+48;

		Keygen << txtKey << endl;
	};
	Keygen.close();
};



void BeginLog() {};

void WriteLog(const char * Txt)
{
	if (bLog)
	{
		textprintf(screen,Font1,10,100,COLOR_2,Txt);
		textprintf(screen1,Font1,10,100,COLOR_2,Txt);
	};	
};

void DFileLog(const char *Txt)
{
	if (bFullLogMode)
	{FileLog(Txt);};
};

void FileLog(const char *Txt) {
	log()->log(Txt);
};

void EndLog() {};

void StopLog(const char * Txt, int Value)
{
	if (bLog)
	{		
		textprintf(screen,Font1,10,100,COLOR_2,"%s%d",Txt,Value);
		textprintf(screen1,Font1,10,100,COLOR_2,"%s%d",Txt,Value);
		clear_keybuf();
		readkey();
	};
};
	
ifstream CurrentFile("var/empty.dmb");		//Общая переменная для чтения скриптов
int ReadInt(char &ch);	//Считывает из текущего файла целочисленную переменную
int ReadWord(char *ch); //Считывает из текущего файла текст до символов ';' или ':', возвращает длину строки


int ReadInt(char &ch)
{
	int iVal=0;
	int bRead=true, bNegative=false;

	while (bRead)
	{
		CurrentFile.get(ch);
		if (CurrentFile.eof())
		{bRead=false;}
		else
		{
			switch (ch)
			{
			case '-':
				bNegative=true;
				break;
			case '0':
				iVal=iVal*10;
				break;
			case '1':
				iVal=iVal*10+1;
				break;
			case '2':
				iVal=iVal*10+2;
				break;
			case '3':
				iVal=iVal*10+3;
				break;
			case '4':
				iVal=iVal*10+4;
				break;
			case '5':
				iVal=iVal*10+5;
				break;
			case '6':
				iVal=iVal*10+6;
				break;
			case '7':
				iVal=iVal*10+7;
				break;
			case '8':
				iVal=iVal*10+8;
				break;
			case '9':
				iVal=iVal*10+9;
				break;
			default:
				bRead=false;
			}
		}
	}
	if (bNegative) iVal=-iVal;
	return iVal;
};

int ReadWord(char *ch)
{
	int bRead=true;
	int iCounter=0;
	while (bRead)
	{
		CurrentFile.get(*ch);
		if ((*ch==':')||(*ch==';')||(CurrentFile.eof()))
		{
			*ch='\0';
			bRead=false;
		}
		else
		{
			ch++;
			iCounter++;
		}
	}
	if (!CurrentFile.eof())
	{
		char trash;
		CurrentFile.get(trash);
	};
	return iCounter;
}

char * ReadText()
{
	int bRead=true;
	int iCounter=0;
	char buffer [4000];
	char ch;
	while (bRead)
	{
		CurrentFile.get(ch);
		if ((ch=='#')||(CurrentFile.eof()))
		{
			buffer[iCounter]='\0';
			bRead=false;
			iCounter++;
		}
		else
		{
			buffer[iCounter]=ch;
			iCounter++;
		}
	};
	char * Result=new char[iCounter];
	for (int i=0; i<iCounter; i++)
	{
		*(Result+i)=buffer[i];
	};
	
	return Result;
};

void SkipText()
{
	int bRead=true;	
	char ch;
	while (bRead)
	{
		CurrentFile.get(ch);
		if ((ch=='#')||(CurrentFile.eof()))
		{			
			bRead=false;			
		}
	};		
};

long int StrToInt(char *Str)
{
	int Result=0, Count=0, ZZ;
	while ((Str[Count]>47)&&(Str[Count]<58))
	{
		ZZ=*(Str+Count);
		ZZ-=48;
		Result=Result*10+ZZ;
		textprintf(screen,Font1,0,0,COLOR_2,"%s: %d, %d",Str,Count, ZZ);
		clear_keybuf();
		readkey();
		Count++;
	};
	textprintf(screen,Font1,0,0,COLOR_2,"Результат: %d",Result);
	clear_keybuf();
	readkey();
	return Result;
};

void IntToStr(int Zifer, char *Str)
{
	sprintf(Str,"");
	while (Zifer>0)
	{
		sprintf(Str,"%d%s",Zifer%10,Str);
		Zifer=(Zifer-Zifer%10)/10;
	};
};

clUnitType * UnitType(int Index)
{
	return (pFirstUnitType+Index);
};

clUnitUpgrade * UnitUpg(int Index)
{
	return (pFirstUnitUpg+Index);
};

clSkill::clSkill()
{
	for (int i=0; i<5; i++)
	{
		for (int j=0; j<MAX_SKILL_UPG; j++)
		{
			arHeroUpg[i][j][0]=0;
			arSquadUpg[i][j][0]=0;
			arHeroUpg[i][j][1]=0;
			arSquadUpg[i][j][1]=0;
		};
	};
};

clSkill::~clSkill()
{};

clCastle::clCastle()
{
	for (int i=0; i<MAX_CASTLE_BLD; i++)
	{
		arBuild[i]=0;
		arAntiBuild[i]=0;
	};
};

clCastle::~clCastle()
{};

int Progress=0;
int OpenFile(const char *FileName)
{
//	int Result;
//	char sName[500];
//	sprintf(sName,FileName);
	CurrentFile.open(FileName);
//	rectfill(screen,0,700,1024,720,COLOR_1);
//	textprintf_centre_ex(screen,Font1,512,703,COLOR_2,-1,"Loading %s...",FileName);
//	rectfill(screen,Progress*10,690,Progress*10+1,691,COLOR_2);
	NewProgress+=100;
	Progress++;
	CurrentFile.clear();
	char CH;
	CurrentFile.get(CH);
	if (CurrentFile)
	{		
		return 1;
	}
	else
	{
		textprintf_centre_ex(screen,Font1,512,683,makecol(250,10,10),-1,"Error opening %s",FileName);
		textprintf_centre_ex(screen1,Font1,512,683,makecol(250,10,10),-1,"Error opening %s",FileName);
		return 0;
	};	
};

int LoadConfig()
{
	char cSymbol=' ';
	char aWord[400];
	char *pWord=aWord;
	int iLength;	
	CurrentFile.close();

	bDemo=true;
	CurrentFile.open("key.txt");	
	CurrentFile.clear();
	int Part=ReadInt(cSymbol);
	if (CurrentFile)
	{
		long int arNum[12];		
		arNum[3]=Part%10;
		Part/=10;
		arNum[2]=Part%10;
		Part/=10;
		arNum[1]=Part%10;
		Part/=10;
		arNum[0]=Part%10;

		Part=ReadInt(cSymbol);
		arNum[7]=Part%10;
		Part/=10;
		arNum[6]=Part%10;
		Part/=10;
		arNum[5]=Part%10;
		Part/=10;
		arNum[4]=Part%10;

		Part=ReadInt(cSymbol);
		arNum[11]=Part%10;
		Part/=10;
		arNum[10]=Part%10;
		Part/=10;
		arNum[9]=Part%10;
		Part/=10;
		arNum[8]=Part%10;

		bDemo=false;		
		if (arNum[1]!=abs((arNum[7]+arNum[3]+arNum[9]+arNum[4])/(arNum[10]+1)+arNum[0]+3)%10)
		{bDemo=true;};
		if (arNum[8]!=abs(arNum[1]+arNum[10]+arNum[4]+arNum[7]*arNum[3]/(arNum[0]+1)+8)%10)
		{bDemo=true;};
		if (arNum[2]!=abs(arNum[1]+arNum[8]-arNum[4]+arNum[10]*arNum[9]/(arNum[3]+1)+7)%10)
		{bDemo=true;};
		if (arNum[6]!=abs(arNum[1]+arNum[7]-arNum[2]+arNum[3]+arNum[8]*arNum[9]/(arNum[4]+1)+3)%10)
		{bDemo=true;};
		if (arNum[11]!=abs(arNum[6]*arNum[8]*arNum[1]/(arNum[3]+2)+arNum[2]-arNum[10]-arNum[0]+arNum[4]+2)%10)
		{bDemo=true;};
		if (arNum[5]!=abs(arNum[10]+arNum[11]+arNum[0]-arNum[7]-arNum[3]+arNum[2]*arNum[10]/(arNum[3]+1)+9)%10)
		{bDemo=true;};		
	};
	CurrentFile.close();

	CurrentFile.open("eador.cfg");	
	CurrentFile.clear();
	char CH;
	CurrentFile.get(CH);
	if (!CurrentFile)
//	if (OpenFile("eador.cfg")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	bSound=ReadInt(cSymbol);	
	iLength=ReadWord(pWord);
	bWindowMode=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	bBackgroundMode=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	iRefreshDelay=ReadInt(cSymbol);
	if (iRefreshDelay<5)
	{iRefreshDelay=5;};
	iLength=ReadWord(pWord);
	iMouseDelay=ReadInt(cSymbol);
	if (iMouseDelay<2)
	{iMouseDelay=2;};
	iLength=ReadWord(pWord);
	iKeyboardDelay=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	iKeyboardRepeat=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	iOggBuffer=ReadInt(cSymbol);
	if (iOggBuffer<100)
	{iOggBuffer=100;};
	iLength=ReadWord(pWord);
	iColorDepth=ReadInt(cSymbol);
	if ((iColorDepth!=24)&&(iColorDepth!=32))
	{iColorDepth=16;};
	iLength=ReadWord(pWord);
	iSleepTime=ReadInt(cSymbol);
	if (iSleepTime<10)
	{iSleepTime=0;};
	iLength=ReadWord(pWord);
	iAntisleepDelay=ReadInt(cSymbol);
	if (iAntisleepDelay>10)
	{iAntisleepDelay=10;};
	iLength=ReadWord(pWord);
	int Temp=ReadInt(cSymbol);
#ifdef YUPLAY
	iYuPlay=Temp;6
#endif
	iLength=ReadWord(pWord);
	bIntro=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	bShowWallpaper=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	bStaticLoot=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	bStaticEvent=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	RES_BUY_CHANGE=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MAX_PROFIT=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MERCENARY_TIME=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	EVENT_POSSIBILITY=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MAX_BATTLE_DURATION=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MAX_ILLUSTRATION=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	iMaxHerbSign=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	iMaxAvatarPic=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MAX_PATH=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	bLakeShip=ReadInt(cSymbol);
	CurrentFile.close();
	return 1;
};

int LoadPhrases()
{
	char cSymbol=' ';
	char aWord[400];
	char *pWord= aWord;
	int iLength;	
//	CurrentFile.close();
	//Загрузка фраз
	if (OpenFile("var/phrases.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxPhrases=ReadInt(cSymbol);
	pFirstPhrase = new stString[iMaxPhrases+1];
	stString *pPhrase = pFirstPhrase;
	for (int i=0; i<=iMaxPhrases; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pPhrase->str);
		pPhrase++;
	};
	CurrentFile.close();
//	GenerateKeys();
	return 1;	
};

//void WriteKeys()
//{
//	char STR[300], STR1[300];
//	sprintf(STR,"LOCK");
//	for (int i=0; i<18; i++)
//	{
//		sprintf(STR1,"%s  %d:%d",STR,i,stLock[i]);
//		sprintf(STR,STR1);
//	};
//	FileLog(STR);
//
//	sprintf(STR,"KEY ");
//	for (int i=0; i<18; i++)
//	{
//		sprintf(STR1,"%s  %d:%d",STR,i,stKey[i]);
//		sprintf(STR,STR1);
//	};
//	FileLog(STR);
//};

int Loader()
{
	char cSymbol=' ';
	char aWord[400];
	char *pWord= aWord;
	char stCompName[16];
	char sKetFile[12], sLocFile[12];
	int iLength;
	int iCounter=0;
	int TypeLine=0;
//	int iCompNameLen=InitRegistry(stCompName);
	char sPath[10], sKey[10];
	char FileName[100];
	sprintf(sPath,"var");
	sprintf(sKey,"Text");
//	WriteRegistry("SUB","Info","***");
//	CurrentFile.close();
	PACKFILE *WorkFile;
	al_ffblk Fil;

	sprintf(sKetFile,"music.dat");
	sprintf(sLocFile,"mouse.dat");



	//Загрузка талантов героев и названий классов
	if (OpenFile("var/hero_class.var")==0)
	{return 0;};
	sKetFile[7]='e';
	sLocFile[7]='o';
	for (int i=0; i<MAX_CLASS_NAME; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(ClassName[i].str);
		
		for (int j=0; j<MAX_TALENT; j++)
		{
			iLength=ReadWord(pWord);
			arTalent[i][j][0]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
			arTalent[i][j][1]=ReadInt(cSymbol);
		};
	};
	CurrentFile.close();
	sKetFile[0]='e';
	sLocFile[0]='e';
	sKetFile[8]='y';
	sLocFile[8]='k';

	//Загрузка имен
	if (OpenFile("var/names.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxName=ReadInt(cSymbol);
	sKetFile[3]='o';
	sLocFile[3]='o';
	for (int j=0; j<4; j++)
	{
		pName[j] = new stString[iMaxName];
		stString *pPhrase = pName[j];
		for (int i=0; i<iMaxName; i++)
		{
			iLength=ReadWord(pWord);
			iLength=ReadWord(pPhrase->str);
			pPhrase++;
		};
	};
	CurrentFile.close();

	sPath[2]='B';
	sKetFile[1]='a';
	sLocFile[1]='a';

	//Загрузка префиксов
	if (OpenFile("var/prefix.var")==0)
	{return 0;};
	sKetFile[4]='r';
	sLocFile[4]='r';
	iLength=ReadWord(pWord);
	iMaxPrefix=ReadInt(cSymbol);
	pFirstPrefix = new stString[iMaxPrefix];
	stString *pPhrase = pFirstPrefix;
	for (int i=0; i<iMaxPrefix; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pPhrase->str);
		pPhrase++;
	};
	CurrentFile.close();
	sKetFile[2]='d';
	sLocFile[2]='d';
	sKetFile[6]='k';
	sLocFile[6]='l';

	//if (al_findfirst(sKetFile,&Fil,FA_ALL)==0)
	//{
	//	WorkFile=pack_fopen(sKetFile,"rp");
	//	pack_fread(&stKey,sizeof stKey,WorkFile);
	//	pack_fclose(WorkFile);
	//};
	//al_findclose(&Fil);

//	int bChecker[17];

	//Загрузка суффиксов
	if (OpenFile("var/Suffix.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxSuffix=ReadInt(cSymbol);
	pFirstSuffix = new stString[iMaxSuffix];
	pPhrase = pFirstSuffix;
	for (int i=0; i<iMaxSuffix; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pPhrase->str);
		pPhrase++;
	};
	CurrentFile.close();
	sKey[2]='f';

	//Загрузка описаний и иконок классов героев
	NewProgress+=100;
	DATAFILE* datHeroClass = load_datafile("dat/Hero_class.dat");
	if (!datHeroClass)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading hero_class.dat");		
		readkey();
		return 1;
	};
	if (OpenFile("text/Hero_class.txt")==0)
	{return 0;};
	for (int i=0; i<MAX_CLASS_NAME; i++)
	{
		SkipText();
		ClassText[i].msg=ReadText();
		ClassPic[i]=(BITMAP *)datHeroClass[i].dat;
	};
	CurrentFile.close();

	if (OpenFile("text/Hero_class_d.txt")==0)
	{return 0;};
	for (int i=0; i<MAX_CLASS_NAME; i++)
	{
		SkipText();
		ClassDesc[i].msg=ReadText();		
	};			
	CurrentFile.close();

	sPath[1]='U';


	//Загрузка названий слотов предметов
	if (OpenFile("var/item_slot.var")==0)
	{return 0;};
	for (int i=0; i<MAX_ITEM_SLOT_NAME; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(ItemSlotName[i].str);
	};
	CurrentFile.close();
	
	//Загрузка родов войск
	if (OpenFile("var/unit_class.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxUnitClass=ReadInt(cSymbol);
	pFirstUnitClass = new stString[iMaxUnitClass+1];
	stString *pUnitClass = pFirstUnitClass;
	for (int i=0; i<=iMaxUnitClass; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pUnitClass->str);
		pUnitClass++;
	};
	CurrentFile.close();

	sKey[0]='i';

	//Загрузка типов войск
	if (OpenFile("var/unit_type.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxUnitKind=ReadInt(cSymbol);
	pFirstUnitKind = new stString[iMaxUnitKind+1];
	stString *pUnitKind = pFirstUnitKind;	
	for (int i=0; i<=iMaxUnitKind; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pUnitKind->str);
		pUnitKind++;
	};	
	CurrentFile.close();

	Randomizer();

	//Загрузка типов предметов
	if (OpenFile("var/item_type.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxItemType=ReadInt(cSymbol);
	pFirstItemType = new stString[iMaxItemType+1];
	stString *pItemType = pFirstItemType;
	for (int i=0; i<=iMaxItemType; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pItemType->str);
		pItemType++;
	};
	CurrentFile.close();

	sPath[0]='S';

	//Загрузка умений юнитов
	/*if (OpenFile("var/ability.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxAbility=ReadInt(cSymbol);
	pFirstAbility = new stAbility[iMaxAbility+1];
	stAbility *pAbility = pFirstAbility;
	for (int i=0; i<=iMaxAbility; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pAbility->sName);
		iLength=ReadWord(pWord);
		pAbility->bNumeric=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pAbility->Effect=ReadInt(cSymbol);
		pAbility++;
	};
	CurrentFile.close();*/

	//Загрузка умений героев
//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"Loading ability.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE *datAbility = load_datafile("dat/Ability.dat");
	if (!datAbility)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading ability.dat");
		return 0;
	};
	if (OpenFile("var/ability_num.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxAbilityNum=ReadInt(cSymbol);
	pFirstAbilityNum = new stAbilityNum[iMaxAbilityNum+1];
	stAbilityNum *pAbilityNum = pFirstAbilityNum;
	for (int i=0; i<=iMaxAbilityNum; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pAbilityNum->sName);
		iLength=ReadWord(pWord);
		pAbilityNum->Num=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pAbilityNum->bNumeric=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pAbilityNum->Effect=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pAbilityNum->bPercent=ReadInt(cSymbol);
		pAbilityNum->Pic=(BITMAP *)datAbility[i].dat;
		pAbilityNum++;
	};
	CurrentFile.close();

	if (OpenFile("text/ability.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxAbilityNum; i++)
	{
		SkipText();
		(pFirstAbilityNum+i)->sText=ReadText();
	};	
	CurrentFile.close();

	sKey[3]='o';
	

	//Загрузка эффектов
	
	if (OpenFile("var/effects.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxEffect=ReadInt(cSymbol);
	pFirstEffect = new stEffect[iMaxEffect+1];
	stEffect *pEffect = pFirstEffect;
	for (int i=0; i<=iMaxEffect; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pEffect->sName);
		iLength=ReadWord(pWord);
		pEffect->ipAbility=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pEffect->ipSpell=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pEffect->bNumeric=ReadInt(cSymbol);		
		pEffect++;
	};			
	CurrentFile.close();

	if (OpenFile("text/Effects.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxEffect; i++)
	{
		SkipText();
		(pFirstEffect+i)->sText=ReadText();
	};	
	CurrentFile.close();

	int bNeedLock=true;
	bDemo=true;
//*******************************LOCK CHECK**********************
	//sprintf(FileName,sLocFile);
	//sprintf(stLock,"music.dat unit.var");
	//sprintf(stReg,stLock);
	//sprintf(stSub,"race.var");
	//	
	//if (iCompNameLen>1)
	//{arComp[0]=abs(stCompName[0]);}
	//else
	//{arComp[0]='j';};

	//if (iCompNameLen>2)
	//{arComp[1]=abs(stCompName[1]);}
	//else
	//{arComp[1]='r';};

	//if (iCompNameLen>3)
	//{arComp[2]=abs(stCompName[2]);}
	//else
	//{arComp[2]='t';};

	//if (iCompNameLen>4)
	//{arComp[3]=abs(stCompName[3]);}
	//else
	//{arComp[3]='z';};

	//sKey[1]='n';

	//if (iCompNameLen>5)
	//{arComp[4]=abs(stCompName[4]);}
	//else
	//{arComp[4]='k';};	

	//if (al_findfirst(FileName,&Fil,FA_ALL)==0)
	//{
	//	WorkFile=pack_fopen(FileName,"rp");
	//	pack_fread(&stLock,sizeof stLock,WorkFile);
	//	pack_fclose(WorkFile);
	//	long int Time=Fil.time;		

	//	if (ReadRegistry(sPath,sKey,stSub))
	//	{
	//		al_ffblk FF;
	//		if (al_findfirst("p1.wrk",&FF,FA_ALL)==0)
	//		{
	//			WorkFile=pack_fopen("p1.wrk","rp");
	//			pack_fread(&stSub,sizeof stSub,WorkFile);
	//			pack_fclose(WorkFile);
	//		};
	//		al_findclose(&FF);
	//	};
	//	if (ReadRegistry("Artifactorium","Key",stReg))
	//	{
	//		al_ffblk FF;
	//		if (al_findfirst("p2.wrk",&FF,FA_ALL)==0)
	//		{
	//			WorkFile=pack_fopen("p2.wrk","rp");
	//			pack_fread(&stReg,sizeof stReg,WorkFile);
	//			pack_fclose(WorkFile);
	//		};
	//		al_findclose(&FF);
	//	};
	//	

	//	if (48+(Time/100)%10==stSub[1])
	//	{Check1++;}
	//	else
	//	{Pirate1++;};

	//	if (48+Time%10==stSub[6])
	//	{Check1++;}
	//	else
	//	{Pirate1++;};

	//	if (107+(Time/10)%10==stReg[0])
	//	{Check1+=4;}
	//	else
	//	{Pirate1+=4;};

	//	if (107+(Time/100000)%10==stReg[3])
	//	{Check1++;}
	//	else
	//	{Pirate1++;};

	//	if (107+(Time/1000)%10==stReg[6])
	//	{Check1++;}
	//	else
	//	{Pirate1++;};

	//	if (107+(Time/10000)%10==stReg[9])
	//	{Check1++;}
	//	else
	//	{Pirate1++;};


	//	if (stLock[0]==stReg[1]+stSub[0]-48)
	//	{Check2++;}
	//	else
	//	{Pirate2++;};
	//	
	//	if (stLock[4]==stReg[2]+stSub[4]-48) 
	//	{Check2+=3;}
	//	else
	//	{Pirate2+=3;};
	//	
	//	if (stLock[10]==stReg[10]-stSub[2]+48)
	//	{Check2++;}
	//	else
	//	{Pirate2++;};

	//	if (stLock[7]!=(stSub[2]-48)*10+(stSub[4]-48))
	//	{Pirate2++;}		
	//	else
	//	{Check2++;};		

	//	if (stLock[6]==stReg[5]+stSub[2]-48)
	//	{Check2+=2;}
	//	else
	//	{Pirate2+=2;};		

	//	
	//	if (stLock[2]==abs(arComp[0]+8)%128)
	//	{Check3++;}else{Pirate3++;};		
	//	if (stLock[8]==abs((stReg[7]+arComp[1])/2)%128)
	//	{Check3++;}else{Pirate3++;};
	//	if (stLock[9]==abs(arComp[3]-11)%128)
	//	{Check3++;}else{Pirate3++;};		
	//	if (stLock[14]==abs(arComp[2]+7)%128)
	//	{Check3++;}else{Pirate3++;};
	//	if (stLock[15]==abs((stReg[11]+arComp[4])/2)%128)
	//	{Check3++;}else{Pirate3++;};
	//	

	//	if (stSub[3]==48+arComp[0]%10)
	//	{Check4++;}else{Pirate4++;};
	//	if (stSub[5]==48+(arComp[0]/10)%10)
	//	{Check4++;}else{Pirate4++;};
	//	if (stSub[7]==48+iCompNameLen%10)
	//	{Check4++;}else{Pirate4++;};

	//	if (stReg[4]==abs(arComp[1]-2)%64+64)
	//	{Check4++;}else{Pirate4++;};
	//	if (stReg[8]==abs(arComp[2]+8)%64+64)
	//	{Check4++;}else{Pirate4++;};
	//	if (stReg[12]=abs(arComp[0]-stSub[2])%64+64)
	//	{Check4++;}else{Pirate4++;};
	//	if (stReg[13]=abs((arComp[3]+arComp[1])/2)%64+64)
	//	{Check4++;}else{Pirate4++;};

	//	if (Check1+Check2+Check3+Check4==29)
	//	{bNeedLock=false;};
	//};
	//al_findclose(&Fil);
//*************************************************	

	//Загрузка типов местности
	if (OpenFile("var/terrain.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxTerrain=ReadInt(cSymbol);
	pFirstTerrain = new clTerrainType[iMaxTerrain+1];
	clTerrainType *pTerrain = pFirstTerrain;	
	for (int i=0; i<=iMaxTerrain; i++)
	{
		for (int j=1; j<=8; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pTerrain->sName);
			case 2:
				pTerrain->iPassibility=ReadInt(cSymbol);
				break;
			case 3:
				pTerrain->iGoldInc=ReadInt(cSymbol);
				break;
			case 4:
				pTerrain->iGemInc=ReadInt(cSymbol);
				break;
			case 5:
				pTerrain->iPopulationMod=ReadInt(cSymbol);
				break;
			case 6:
				pTerrain->iExploreMod=ReadInt(cSymbol);
				break;
			case 7:
				pTerrain->iBF_Pass=ReadInt(cSymbol);
				break;
			case 8:
				pTerrain->iBF_Impass=ReadInt(cSymbol);
				break;
			};
			
		};
		pTerrain++;
	};
	CurrentFile.close();	

	//Загрузка объектов поля боя
	if (OpenFile("var/bf_object.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxBF_Object=ReadInt(cSymbol);
	pFirstBF_Obj = new clBF_Object[iMaxBF_Object+1];
	clBF_Object *pBF_Obj = pFirstBF_Obj;	
	for (int i=0; i<=iMaxBF_Object; i++)
	{
		for (int j=1; j<=7; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pBF_Obj->sName);
			case 2:
				pBF_Obj->iMoveCost=ReadInt(cSymbol);
				break;
			case 3:
				pBF_Obj->iStamCost=ReadInt(cSymbol);
				break;
			case 4:
				pBF_Obj->iCounterAttack=ReadInt(cSymbol);
				break;
			case 5:
				pBF_Obj->iDefence=ReadInt(cSymbol);
				break;
			case 6:
				pBF_Obj->iRangedDefence=ReadInt(cSymbol);
				break;
			case 7:
				pBF_Obj->iShootingRange=ReadInt(cSymbol);
				break;			
			};			
		};
		pBF_Obj++;
	};
//	bChecker[3]=!(stKey[3]==abs(17+(stLock[8]+stLock[10])/2+stLock[7])%128);
	CurrentFile.close();

//	CurGame.iSeed=stLock[2];

	
	//Загрузка ресурсов
	if (OpenFile("var/resource.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxResource=ReadInt(cSymbol);
	pFirstResource = new clResource[iMaxResource+1];
	clResource *pResource = pFirstResource;
	for (int i=0; i<=iMaxResource; i++)
	{
		for (int j=1; j<=4; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pResource->sName);
				break;
			case 2:
				pResource->iRarity=ReadInt(cSymbol);
				break;
			case 3:
				pResource->iGold=ReadInt(cSymbol);
				break;
			case 4:
				pResource->iGem=ReadInt(cSymbol);
				break;			
			};
			
		};
		pResource++;
	};
	CurrentFile.close();
	NewProgress+=100;
	if (OpenFile("text/Resource.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxResource; i++)
	{
		SkipText();
		(pFirstResource+i)->sText=ReadText();
	};	
	CurrentFile.close();

	//Загрузка рас
	if (OpenFile("var/race.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxRace=ReadInt(cSymbol);
	if (iMaxRace>MAX_RACE)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading race.var file, too many races! (maximum: %d)",MAX_RACE);
		return 0;
	};
	pFirstRace = new clRace[iMaxRace+1];
	clRace *pRace = pFirstRace;	
	for (int i=0; i<=iMaxRace; i++)
	{
		for (int j=1; j<11+MAX_TERRAIN; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pRace->sName);
			case 2:
				pRace->ipProvType=ReadInt(cSymbol);
				break;
			case 3:
				pRace->iGold=ReadInt(cSymbol);
				break;
			case 4:
				pRace->iGem=ReadInt(cSymbol);
				break;			
			case 5:
				pRace->iKarma=ReadInt(cSymbol);
				break;
			case 6:
				pRace->iCorrupt=ReadInt(cSymbol);
				break;
			case 7:
				pRace->iGrow=ReadInt(cSymbol);
				break;
			case 8:
				pRace->iTerra=ReadInt(cSymbol);
				break;
			case 9:
				pRace->iUnrest=ReadInt(cSymbol);
				break;
			case 10:				
				iCounter=0;
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_RACE_QUEST)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading race.var file, member: %d",i);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pRace->arQuest[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			default:
				pRace->arTerrain1[j-9]=ReadInt(cSymbol);
				break;
			};
			
		};
		pRace++;
	};
//	bChecker[4]=!(stKey[4]==abs(2+stLock[4]*stLock[5])%128);
	CurrentFile.close();

	if (OpenFile("text/races.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxRace; i++)
	{
		SkipText();
		(pFirstRace+i)->sText=ReadText();
	};	
	CurrentFile.close();

	//Загрузка улучшений юнитов
	if (OpenFile("var/unit_upg.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxUnitUpg=ReadInt(cSymbol);
	pFirstUnitUpg = new clUnitUpgrade[iMaxUnitUpg+1];
	clUnitUpgrade *pUnitUpg = pFirstUnitUpg;	
	for (int i=0; i<=iMaxUnitUpg; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pUnitUpg->sName);
		iLength=ReadWord(pWord);
		pUnitUpg->bOnlyOne=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		iCounter=0;
		cSymbol='x';
		while (cSymbol!=')')
		{			
			if (iCounter==MAX_UPGRADE_NEED)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading unit_upg.var file, member: %d",i);
				return 0;
			};
			CurrentFile.get(cSymbol);
			pUnitUpg->ipNeed[iCounter]=ReadInt(cSymbol);
			iCounter++;
		};
		iCounter=0;
		cSymbol='x';
		while (cSymbol!=';')
		{					
			if (iCounter==MAX_UPGRADE_SIZE)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading unit_upg.var file, member: %d",i);				
				return 0;
			};
			iLength=ReadWord(pWord);
			pUnitUpg->arUpg[iCounter][0]=ReadInt(cSymbol);
			iLength=ReadWord(pWord);
			pUnitUpg->arUpg[iCounter][1]=ReadInt(cSymbol);
			iCounter++;
		};
		pUnitUpg++;
	};
	CurrentFile.close();

	if (OpenFile("text/upgrade.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxUnitUpg; i++)
	{	
		SkipText();
		(pFirstUnitUpg+i)->sText=ReadText();	
	};
	CurrentFile.close();

	//Загрузка юнитов
//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"Loading units.dat...");
//	TypeLine++;	
	NewProgress+=100;
	DATAFILE *datUnit = load_datafile("dat/Units.dat");
	if (!datUnit)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading units.dat");		
		return 0;
	};
	NewProgress+=100;
	DATAFILE *datUnitShadow = load_datafile("dat/Unit_shadow.dat");
	if (!datUnitShadow)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading unit_shadow.dat");
		return 0;
	};
	NewProgress+=100;
	DATAFILE *datUnitShadowF = load_datafile("dat/Unit_shadowf.dat");
	if (!datUnitShadowF)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading unit_shadowf.dat");
		return 0;
	};
	NewProgress+=100;
	
	DATAFILE *datUnitIcons = load_datafile("dat/Unit_icons.dat");
	if (!datUnitIcons)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading unit_icons.dat");		
		return 0;
	};
	//wavRoot=alogg_create_ogg_from_buffer(datUnitSound[4].dat, datUnitSound[4].size);
	//wavEat=alogg_create_ogg_from_buffer(datUnitSound[6].dat, datUnitSound[6].size);
	//wavArmor=alogg_create_ogg_from_buffer(datUnitSound[9].dat, datUnitSound[9].size);
	//wavWeapon=alogg_create_ogg_from_buffer(datUnitSound[10].dat, datUnitSound[10].size);
	//wawHeal=alogg_create_ogg_from_buffer(datUnitSound[13].dat, datUnitSound[13].size);
	//wavRound=alogg_create_ogg_from_buffer(datUnitSound[25].dat, datUnitSound[25].size);
	//wavBerz=alogg_create_ogg_from_buffer(datUnitSound[82].dat, datUnitSound[82].size);
	//wavMarch=alogg_create_ogg_from_buffer(datUnitSound[83].dat, datUnitSound[83].size);
	//wavGather=alogg_create_ogg_from_buffer(datUnitSound[84].dat, datUnitSound[84].size);
	//wavCrush=alogg_create_ogg_from_buffer(datUnitSound[125].dat, datUnitSound[125].size);
	//wavSoul=alogg_create_ogg_from_buffer(datUnitSound[130].dat, datUnitSound[130].size);
	//wavTrample=alogg_create_ogg_from_buffer(datUnitSound[135].dat, datUnitSound[135].size);
	//wavCoins=alogg_create_ogg_from_buffer(datUnitSound[155].dat, datUnitSound[155].size);
	//wavParchment=alogg_create_ogg_from_buffer(datUnitSound[156].dat, datUnitSound[156].size);
	//wavBuild=alogg_create_ogg_from_buffer(datUnitSound[157].dat, datUnitSound[157].size);
	//wavDelete=alogg_create_ogg_from_buffer(datUnitSound[158].dat, datUnitSound[158].size);
	//wavRepair=alogg_create_ogg_from_buffer(datUnitSound[28].dat, datUnitSound[28].size);
	//wavRepairAll=alogg_create_ogg_from_buffer(datUnitSound[129].dat, datUnitSound[129].size);

	/*wavRoot=(SAMPLE *)datUnitSound[4].dat;
	wavEat=(SAMPLE *)datUnitSound[6].dat;
	wavArmor=(SAMPLE *)datUnitSound[9].dat;
	wavWeapon=(SAMPLE *)datUnitSound[10].dat;
	wawHeal=(SAMPLE *)datUnitSound[13].dat;
	wavRound=(SAMPLE *)datUnitSound[25].dat;
	wavBerz=(SAMPLE *)datUnitSound[82].dat;
	wavMarch=(SAMPLE *)datUnitSound[83].dat;
	wavGather=(SAMPLE *)datUnitSound[84].dat;
	wavCrush=(SAMPLE *)datUnitSound[125].dat;
	wavSoul=(SAMPLE *)datUnitSound[130].dat;
	wavTrample=(SAMPLE *)datUnitSound[135].dat;
	wavCoins=(SAMPLE *)datUnitSound[155].dat;
	wavParchment=(SAMPLE *)datUnitSound[156].dat;
	wavBuild=(SAMPLE *)datUnitSound[157].dat;
	wavDelete=(SAMPLE *)datUnitSound[158].dat;*/
	

	if (OpenFile("var/unit.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxUnitType=ReadInt(cSymbol);
	pFirstUnitType = new clUnitType[iMaxUnitType+1];
	clUnitType *pUnitType = pFirstUnitType;		
	for (int i=0; i<=iMaxUnitType; i++)
	{
		if (i>0)
		{
			pUnitType->Pic=(BITMAP *)datUnit[i-1].dat;
			pUnitType->Shadow=(BITMAP *)datUnitShadow[i-1].dat;
			pUnitType->FlipShadow=(BITMAP *)datUnitShadowF[i-1].dat;//create_bitmap(pUnitType->Shadow->w,pUnitType->Shadow->h);
			pUnitType->Icon=(BITMAP *)datUnitIcons[i-1].dat;
	//		rectfill(pUnitType->FlipShadow,0,0,pUnitType->Shadow->w,pUnitType->Shadow->h,COLOR_0);
	//		rotate_sprite_v_flip(pUnitType->FlipShadow, pUnitType->Shadow, 0, 0, itofix(128));
		};
		for (int j=1; j<72; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pUnitType->sName);
				break;
			case 2:
				pUnitType->iLevel=ReadInt(cSymbol);
				break;
			case 3:
				pUnitType->iLife=ReadInt(cSymbol);
				break;
			case 4:
				pUnitType->iAttack=ReadInt(cSymbol);
				break;			
			case 5:
				pUnitType->iCounterAttack=ReadInt(cSymbol);
				break;
			case 6:
				pUnitType->iDefence=ReadInt(cSymbol);
				break;
			case 7:
				pUnitType->iRangedDefence=ReadInt(cSymbol);
				break;
			case 8:
				pUnitType->iResist=ReadInt(cSymbol);
				break;
			case 9:
				pUnitType->iSpeed=ReadInt(cSymbol);
				break;
			case 10:
				pUnitType->iRangedAttack=ReadInt(cSymbol);
				break;
			case 11:
				pUnitType->iShootingRange=ReadInt(cSymbol);
				break;
			case 12:
				pUnitType->iAmmo=ReadInt(cSymbol);
				break;
			case 13:
				pUnitType->iStamina=ReadInt(cSymbol);
				break;
			case 14:
				pUnitType->iMorale=ReadInt(cSymbol);
				break;			
			case 15:
				pUnitType->iExp=ReadInt(cSymbol);
				break;
			case 16:
				pUnitType->iExpMod=ReadInt(cSymbol);
				break;
			case 17:
				pUnitType->iGoldPrice=ReadInt(cSymbol);
				break;
			case 18:
				pUnitType->iGemPrice=ReadInt(cSymbol);
				break;
			case 19:
				pUnitType->iGoldPayment=ReadInt(cSymbol);
				break;
			case 20:
				pUnitType->iGemPayment=ReadInt(cSymbol);
				break;
			case 21:
				pUnitType->iRace=ReadInt(cSymbol);
				break;
			case 22:
				pUnitType->iUnitKind=ReadInt(cSymbol);
				break;
			case 23:
				pUnitType->iUnitClass=ReadInt(cSymbol);
				break;
			case 24:
				pUnitType->iKarma=ReadInt(cSymbol);
				break;	
			case 25:
				pUnitType->wavHit=ReadInt(cSymbol);
				break;
			case 26:
				pUnitType->wavShoot=ReadInt(cSymbol);
				break;
			case 27:
				pUnitType->wavShootHit=ReadInt(cSymbol);
				break;
			case 28:
				pUnitType->wavDeath=ReadInt(cSymbol);
				break;
			case 29:
				pUnitType->iMissile=ReadInt(cSymbol);
				break;
			case 30:
				iCounter=0;
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_UNIT_RES)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading unit.var file (resource), member: %s",pUnitType->sName);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pUnitType->arRes[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			case 31:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_UNIT_ABIL)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading unit.var file (ability), member: %s",pUnitType->sName);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pUnitType->arAbility[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			default:
				iCounter=0;
				cSymbol='x';
				if (j<52)
				{
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_UPG_PER_LVL)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading unit.var file (upgrade), member: %s, level: %d",pUnitType->sName, j-31);						
						return 0;
					};
					CurrentFile.get(cSymbol);
					pUnitType->arUpg[0][j-32][iCounter]=ReadInt(cSymbol);
					CurrentFile.get(cSymbol);
					pUnitType->arUpg[1][j-32][iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				}
				else
				{				
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_LOOT_PER_LVL)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading unit.var file (loot), member: %s, level: %d",pUnitType->sName, j-52);						
						return 0;
					};
					CurrentFile.get(cSymbol);
					pUnitType->arLoot[0][j-52][iCounter]=ReadInt(cSymbol);
					CurrentFile.get(cSymbol);
					pUnitType->arLoot[1][j-52][iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				};
				break;
			};
		};
		pUnitType++;
	};
	CurrentFile.close();
//	bChecker[5]=!(stKey[5]==abs(stLock[9]+stLock[6]/3-13)%128);		

	if (OpenFile("text/unit.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxUnitType; i++)
	{
		SkipText();
		(pFirstUnitType+i)->sText=ReadText();
	};	
	CurrentFile.close();


	//Загрузка заклинаний

	NewProgress+=100;

	if (OpenFile("var/spell.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxSpell=ReadInt(cSymbol);
	pFirstSpell = new clSpell[iMaxSpell+1];
	clSpell *pSpell = pFirstSpell;	
	for (int i=0; i<=iMaxSpell; i++)
	{	
		for (int j=1; j<27; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pSpell->sName);
				break;
			case 2:
				pSpell->iCost=ReadInt(cSymbol);
				break;
			case 3:
				pSpell->iLifeCost=ReadInt(cSymbol);
				break;
			case 4:
				pSpell->iStamCost=ReadInt(cSymbol);
				break;
			case 5:
				pSpell->iItemLevel=ReadInt(cSymbol);
				break;
			case 6:
				pSpell->iLevel=ReadInt(cSymbol);
				break;			
			case 7:
				pSpell->iPowerMod=ReadInt(cSymbol);
				break;
			case 8:
				pSpell->iDurationMod=ReadInt(cSymbol);
				break;
			case 9:
				pSpell->iTarget=ReadInt(cSymbol);
				break;
			case 10:
				pSpell->iArea=ReadInt(cSymbol);
				break;
			case 11:
				pSpell->iRadius=ReadInt(cSymbol);
				break;
			case 12:
				pSpell->iKarma=ReadInt(cSymbol);
				break;
			case 13:
				pSpell->ipBuilding=ReadInt(cSymbol);
				break;
			case 14:
				pSpell->bNegative=ReadInt(cSymbol);
				break;				
			case 15:
				pSpell->bOnEnemy=ReadInt(cSymbol);
				break;
			case 16:
				pSpell->bOnAlly=ReadInt(cSymbol);
				break;			
			case 17:
				pSpell->bSacrifice=ReadInt(cSymbol);
				break;
			case 18:
				iCounter=ReadInt(cSymbol);
				if (iCounter>0)
				{
					pSpell->bResistPower=true;
					pSpell->iResistMod=iCounter;
				}
				else
				{
					pSpell->bResistPower=false;
				};
				break;
			case 19:
				iCounter=ReadInt(cSymbol);
				if (iCounter>0)
				{
					pSpell->bResistDuration=true;
					pSpell->iResistMod=iCounter;
				}
				else
				{pSpell->bResistDuration=false;};
				break;
			case 20:
				pSpell->bDefencePower=ReadInt(cSymbol);
				break;
			case 21:
				pSpell->bRestoreOnly=ReadInt(cSymbol);
				break;
			case 22:
				pSpell->bCumulative=ReadInt(cSymbol);
				break;
			case 23:
				pSpell->wavCast=ReadInt(cSymbol);
				break;
			case 24:								
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_UNIT_KIND)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading spell.var file (Unit kind), member: %s",pSpell->sName);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pSpell->arUnitKind[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			case 25:								
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_ANTI_EFFECT)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading spell.var file (Anti effect), member: %d",i);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pSpell->arAntiEffect[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			case 26:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_SPELL_EFFECT)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading spell.var file (spell effect), member: %s",pSpell->sName);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pSpell->arEffect[0][iCounter]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pSpell->arEffect[1][iCounter]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pSpell->arEffect[2][iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;			
			};
		};
		pSpell++;
	};
	CurrentFile.close();
	
	if (OpenFile("text/Spell.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxSpell; i++)
	{
		SkipText();
		(pFirstSpell+i)->sText=ReadText();
	};	
	CurrentFile.close();
//	bChecker[6]=!(stKey[6]==abs(stLock[14]-stLock[15]+stLock[17]/2)%128);
	

	//Загрузка параметров героев
	if (OpenFile("var/stat.var")==0)
	{return 0;};
	for (int i=0; i<3; i++)
	{		
		for (int j=0; j<10; j++)
		{
			iLength=ReadWord(pWord);
			arStat[i][j][0]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
			arStat[i][j][1]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
			arStat[i][j][2]=ReadInt(cSymbol);
		};
	};	
	CurrentFile.close();

	//Загрузка роста параметров героев в зависимости от класса
	if (OpenFile("var/stat_progress.var")==0)
	{return 0;};
	for (int i=0; i<20; i++)
	{		
		for (int j=0; j<3; j++)
		{
			iLength=ReadWord(pWord);
			arStatProgress[j][i]=ReadInt(cSymbol);			
		};
	};	
	CurrentFile.close();
	
	//Загрузка умений героев	
	if (OpenFile("var/skill.var")==0)
	{return 0;};
	for (int i=0; i<=MAX_HERO_SKILL; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(arSkills[i].sName);
		iLength=ReadWord(pWord);
		arSkills[i].iClass=ReadInt(cSymbol);		
		iLength=ReadWord(pWord);
		for (int j=0; j<5; j++)
		{
			iCounter=0;
			iLength=ReadWord(pWord);
			cSymbol='x';
			while (cSymbol!=')')
			{			
				if (iCounter==MAX_SKILL_UPG)
				{
					textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading skill.var file, member: %d",i);
					return 0;
				};
				CurrentFile.get(cSymbol);
				arSkills[i].arHeroUpg[j][iCounter][0]=ReadInt(cSymbol);
				CurrentFile.get(cSymbol);
				arSkills[i].arHeroUpg[j][iCounter][1]=ReadInt(cSymbol);
				iCounter++;
			};		
		};	
		iLength=ReadWord(pWord);
		for (int j=0; j<5; j++)
		{
			iCounter=0;
			iLength=ReadWord(pWord);
			cSymbol='x';
			while (cSymbol!=')')
			{			
				if (iCounter==MAX_SKILL_UPG)
				{
					textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading skill.var file, member: %d",i);
					return 0;
				};
				CurrentFile.get(cSymbol);
				arSkills[i].arSquadUpg[j][iCounter][0]=ReadInt(cSymbol);
				CurrentFile.get(cSymbol);
				arSkills[i].arSquadUpg[j][iCounter][1]=ReadInt(cSymbol);
				iCounter++;
			};		
		};
	};
//	bChecker[7]=!(stKey[7]==abs(stLock[8]+stLock[13]+stLock[6]-stLock[5]-stLock[2])%128);
	CurrentFile.close();

	if (OpenFile("text/Skill.txt")==0)
	{return 0;};
	for (int i=0; i<=MAX_HERO_SKILL; i++)
	{
		SkipText();
		arSkillText[i].msg=ReadText();		
	};
	CurrentFile.close();


	//Загрузка охранников
	if (OpenFile("var/guard.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxGuard=ReadInt(cSymbol);
	pFirstGuard = new clGuard[iMaxGuard+1];
	clGuard *pGuard = pFirstGuard;	
	for (int i=0; i<=iMaxGuard; i++)
	{	
		for (int j=0; j<MAX_GUARD_UNIT; j++)
		{
			pGuard->arUnit[j][0]=0;
		};
		for (int j=0; j<MAX_GUARD_LOOT; j++)
		{
			pGuard->arLoot[j][0]=0;
		};
		iLength=ReadWord(pWord);
		pGuard->Num=ReadInt(cSymbol);
		pGuard->NumAdd=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pGuard->Quick=ReadInt(cSymbol);
		pGuard->QuickAdd=ReadInt(cSymbol);
		cSymbol='x';
		int Number=0;
		while (cSymbol!=';')
		{
			if (Number==MAX_GUARD_UNIT)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading guard.var file (unit part), member: %d",i);
				return 0;
			};
			iLength=ReadWord(pWord);
			pGuard->arUnit[Number][0]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
			pGuard->arUnit[Number][1]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
			pGuard->arUnit[Number][2]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
			pGuard->arUnit[Number][3]=ReadInt(cSymbol);
			Number++;
		};
		iLength=ReadWord(pWord);
		pGuard->LootPoss=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pGuard->LootNum=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pGuard->Power=ReadInt(cSymbol);
		cSymbol='x';
		Number=0;
		while (cSymbol!=';')
		{
			if (Number==MAX_GUARD_LOOT)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading guard.var file (loot part), member: %d",i);
				return 0;
			};
			iLength=ReadWord(pWord);
			pGuard->arLoot[Number][0]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
			pGuard->arLoot[Number][1]=ReadInt(cSymbol);			
			Number++;
		};	
		pGuard++;
	};
	CurrentFile.close();
//	bChecker[8]=!(stKey[8]==abs(77+stLock[1]-stLock[4])%128);

	//Загрузка типов охранников
	if (OpenFile("var/guard_type.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxGuardType=ReadInt(cSymbol);
	pFirstGuardType = new clGuardType[iMaxGuardType+1];
	clGuardType *pGuardType = pFirstGuardType;
	for (int i=0; i<=iMaxGuardType; i++)
	{	
		iLength=ReadWord(pWord);
		iLength=ReadWord(pGuardType->sName);
		iLength=ReadWord(pWord);
		pGuardType->iLevel=ReadInt(cSymbol);				
		iLength=ReadWord(pWord);
		pGuardType->iKarma=ReadInt(cSymbol);
		for (int j=0; j<MAX_GUARD_LEVEL; j++)
		{
			iLength=ReadWord(pWord);
			pGuardType->arGuard[j]=ReadInt(cSymbol);
		};		
		pGuardType++;
	};
	CurrentFile.close();



	//Загрузка типов провинций
	if (OpenFile("var/province_type.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxProvinceType=ReadInt(cSymbol);
	pFirstProvinceType = new clProvinceType[iMaxProvinceType+1];
	clProvinceType *pProvinceType = pFirstProvinceType;	
	for (int i=0; i<=iMaxProvinceType; i++)
	{	
		for (int j=0; j<MAX_PROVINCE_GUARD; j++)
		{
			pProvinceType->arGuard[j][0]=0;
		};
		for (int j=0; j<MAX_PROVINCE_EVENT; j++)
		{
			pProvinceType->arEvent[j][0]=0;
		};
		
		for (int j=1; j<24; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pProvinceType->sName);
				break;
			case 2:
				pProvinceType->ipRace=ReadInt(cSymbol);
				break;
			case 3:
				pProvinceType->iCoast=ReadInt(cSymbol);
				break;
			case 4:
				pProvinceType->iSites=ReadInt(cSymbol);
				break;			
			case 5:
				pProvinceType->iGold=ReadInt(cSymbol);
				break;
			case 6:
				pProvinceType->iGem=ReadInt(cSymbol);
				break;
			case 7:
				pProvinceType->iDifficult=ReadInt(cSymbol);
				break;
			case 8:
				pProvinceType->iPopulation=ReadInt(cSymbol);
				break;
			case 9:
				pProvinceType->iLevelPop=ReadInt(cSymbol);
				break;
			case 10:
				pProvinceType->iDiplUnrest=ReadInt(cSymbol);
				break;
			case 11:
				pProvinceType->iConqUnrest=ReadInt(cSymbol);
				break;
			case 12:
				pProvinceType->iDiplKarma=ReadInt(cSymbol);
				break;				
			case 13:
				pProvinceType->iConqKarma=ReadInt(cSymbol);
				break;
			case 14:
				pProvinceType->iExplored=ReadInt(cSymbol);
				break;			
			case 15:
				pProvinceType->iLevelExplored=ReadInt(cSymbol);
				break;
			case 16:
				pProvinceType->arTerrain[1]=ReadInt(cSymbol);
				break;
			case 17:
				pProvinceType->arTerrain[2]=ReadInt(cSymbol);
				break;
			case 18:
				pProvinceType->arTerrain[3]=ReadInt(cSymbol);
				break;
			case 19:
				pProvinceType->arTerrain[4]=ReadInt(cSymbol);
				break;
			case 20:
				pProvinceType->arTerrain[5]=ReadInt(cSymbol);
				break;
			case 21:
				pProvinceType->arTerrain[6]=ReadInt(cSymbol);
				break;
			case 22:								
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_PROVINCE_GUARD)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading province_type.var file (Guards part), member: %s",pProvinceType->sName);
						return 0;
					};
					for (int k=0; k<4; k++)
					{
						iLength=ReadWord(pWord);
						pProvinceType->arGuard[iCounter][k]=ReadInt(cSymbol);
					};
					iCounter++;
				};
				break;
			case 23:								
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_PROVINCE_EVENT)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading province_type.var file (Events part), member: %s",pProvinceType->sName);
						return 0;
					};
					for (int k=0; k<2; k++)
					{
						iLength=ReadWord(pWord);
						pProvinceType->arEvent[iCounter][k]=ReadInt(cSymbol);
					};
					iCounter++;
				};
				break;			
			};
		};
		pProvinceType++;
	};
	CurrentFile.close();

	//Загрузка уровней провинций
	if (OpenFile("var/province_level.var")==0)
	{return 0;};
	for (int i=0; i<=MAX_PROVINCE_LEVEL; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(ProvLevel[i].sName);
		iLength=ReadWord(pWord);
		ProvLevel[i].iPopulation=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		ProvLevel[i].iIncome=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		ProvLevel[i].iExplored=ReadInt(cSymbol);		
	};
	CurrentFile.close();
//	bChecker[9]=!(stKey[9]==abs(3+stLock[0]-stLock[17]+stLock[7])%128);


	//Загрузка названий провинций
	if (OpenFile("var/province_name.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxProvinceName=ReadInt(cSymbol);
	pFirstProvinceName = new clProvinceName[iMaxProvinceName+1];
	clProvinceName *pProvinceName = pFirstProvinceName;
	for (int i=0; i<=iMaxProvinceName; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pProvinceName->sName);
		iLength=ReadWord(pWord);
		pProvinceName->iRarity=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pProvinceName->iCoast=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		iCounter=0;
		cSymbol='x';
		while (cSymbol!=')')
		{					
			if (iCounter>MAX_TERRAIN)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading province_name.var file (Terrain), member: %s",pProvinceName->sName);
				return 0;
			};
			CurrentFile.get(cSymbol);
			pProvinceName->arTerrain[iCounter]=ReadInt(cSymbol);
			iCounter++;
		};
		iLength=ReadWord(pWord);
		iCounter=0;
		cSymbol='x';
		while (cSymbol!=')')
		{					
			if (iCounter==MAX_NAME_TYPE)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading province_name.var file (Type), member: %s",pProvinceName->sName);
				return 0;
			};
			CurrentFile.get(cSymbol);
			pProvinceName->arType[iCounter]=ReadInt(cSymbol);
			iCounter++;
		};
		pProvinceName++;
	};

	CurrentFile.close();

	//Загрузка внешних зданий
	if (OpenFile("var/outer_build.var")==0)
	{return 0;};
	iMaxBldItem=0;
	iLength=ReadWord(pWord);
	iMaxOuterBld=ReadInt(cSymbol);
	pFirstOuterBld = new clOuterBuilding[iMaxOuterBld+1];
	clOuterBuilding *pOuterBld = pFirstOuterBld;
	for (int i=0; i<=iMaxOuterBld; i++)
	{	
		for (int j=0; j<MAX_BUILDING_RES; j++)
		{
			pOuterBld->arRes[j]=0;
		};
		for (int j=0; j<MAX_RESOURCE_BONUS; j++)
		{
			pOuterBld->arResBonus[j][0]=0;
		};
		for (int j=0; j<MAX_BUILDING_ABILITY; j++)
		{
			pOuterBld->arAbility[j][0]=0;
		};
		for (int j=1; j<27; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pOuterBld->sName);
				break;
			case 2:
				pOuterBld->ipUpgrade=ReadInt(cSymbol);
				break;
			case 3:
				pOuterBld->ipBuilding=ReadInt(cSymbol);
				break;
			case 4:
				pOuterBld->iGoldCost=ReadInt(cSymbol);
				break;			
			case 5:
				pOuterBld->iGemCost=ReadInt(cSymbol);
				break;
			case 6:
				pOuterBld->iGoldAdd=ReadInt(cSymbol);
				break;
			case 7:
				pOuterBld->iGemAdd=ReadInt(cSymbol);
				break;
			case 8:
				pOuterBld->iGoldPerc=ReadInt(cSymbol);
				break;
			case 9:
				pOuterBld->iGemPerc=ReadInt(cSymbol);
				break;
			case 10:
				pOuterBld->iUnrest=ReadInt(cSymbol);
				break;
			case 11:
				pOuterBld->iAttitude=ReadInt(cSymbol);
				break;
			case 12:
				pOuterBld->iGrow=ReadInt(cSymbol);
				break;				
			case 13:
				pOuterBld->iKarma=ReadInt(cSymbol);
				break;
			case 14:
				pOuterBld->iHeal=ReadInt(cSymbol);
				break;
			case 15:
				pOuterBld->iCoastal=ReadInt(cSymbol);
				break;			
			case 16:
				pOuterBld->iPort=ReadInt(cSymbol);
				break;
			case 17:
				pOuterBld->iWall=ReadInt(cSymbol);
				break;
			case 18:
				pOuterBld->iTowerHealth=ReadInt(cSymbol);
				break;
			case 19:
				pOuterBld->iTowerShoot=ReadInt(cSymbol);
				break;
			case 20:
				pOuterBld->iTowerType=ReadInt(cSymbol);
				break;
			case 21:
				pOuterBld->bLibrary=ReadInt(cSymbol);
				break;
			case 22:
				pOuterBld->bShop=ReadInt(cSymbol);
				break;
			case 23:
				for (int k=0; k<MAX_TERRAIN; k++)
				{
					iLength=ReadWord(pWord);
					pOuterBld->arTerrain[k]=ReadInt(cSymbol);
				};
				break;
			case 24:					
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_BUILDING_RES)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading outer_build.var file (Resource), member: %s",pOuterBld->sName);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pOuterBld->arRes[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;			
			case 25:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_RESOURCE_BONUS)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading outer_build.var file (Resource bonus), member: %s",pOuterBld->sName);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pOuterBld->arResBonus[iCounter][0]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pOuterBld->arResBonus[iCounter][1]=ReadInt(cSymbol);					
					iCounter++;
				};
				break;
			case 26:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_BUILDING_ABILITY)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading outer_build.var file (Ability), member: %s",pOuterBld->sName);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pOuterBld->arAbility[iCounter][0]=ReadInt(cSymbol);
					if (pOuterBld->arAbility[iCounter][0]==8)
					{iMaxBldItem++;};
					iLength=ReadWord(pWord);
					pOuterBld->arAbility[iCounter][1]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pOuterBld->arAbility[iCounter][2]=ReadInt(cSymbol);
					iCounter++;
				};
				break;	
			};
		};		
		pOuterBld++;
	};
	CurrentFile.close();

	if (OpenFile("text/Outer_Build.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxOuterBld; i++)
	{
		SkipText();
		(pFirstOuterBld+i)->sText=ReadText();
	};	
	CurrentFile.close();

//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"Loading outbuild.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE* datOutBuild = load_datafile("dat/OutBuild.dat");
	if (!datOutBuild)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading outbuild.dat");		
		readkey();
		return 1;
	};
//	bChecker[10]=!(stKey[10]==abs(12+stLock[10]*stLock[0]/10)%128);		

//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"Loading outbld.dat...");
//	TypeLine++;	
	NewProgress+=100;
	DATAFILE *datOutBld = load_datafile("dat/OutBld.dat");
	if (!datOutBld)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading outbld.dat");
		return 0;
	};

	for (int i=0; i<iMaxOuterBld; i++)
	{
		(pFirstOuterBld+i+1)->Pic=(BITMAP *)datOutBuild[i].dat;
		(pFirstOuterBld+i+1)->bmpBld=(BITMAP *)datOutBld[i*2].dat;
		(pFirstOuterBld+i+1)->bmpShadow=(BITMAP *)datOutBld[i*2+1].dat;
	};


	//Загрузка стражей
	if (OpenFile("var/defender.var")==0)
	{return 0;};
	iMaxDefItem=0;
	iLength=ReadWord(pWord);
	iMaxDefender=ReadInt(cSymbol);
	pFirstDefender = new clDefender[iMaxDefender+1];
	clDefender *pDefender = pFirstDefender;
	for (int i=0; i<=iMaxDefender; i++)
	{			
		for (int j=1; j<19; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pDefender->sName);
				break;
			case 2:
				pDefender->ipPicture=ReadInt(cSymbol);
				break;
			case 3:
				pDefender->iPower=ReadInt(cSymbol);
				break;
			case 4:
				pDefender->iGoldCost=ReadInt(cSymbol);
				break;
			case 5:
				pDefender->iGemCost=ReadInt(cSymbol);
				break;
			case 6:
				pDefender->iGoldPayment=ReadInt(cSymbol);
				break;			
			case 7:
				pDefender->iGemPayment=ReadInt(cSymbol);
				break;
			case 8:
				pDefender->iBuilding=ReadInt(cSymbol);
				break;
			case 9:
				pDefender->iInitiative=ReadInt(cSymbol);
				break;
			case 10:
				pDefender->iKarma=ReadInt(cSymbol);
				break;
			case 11:
				pDefender->iLootPoss=ReadInt(cSymbol);
				break;
			case 12:
				pDefender->iLootNum=ReadInt(cSymbol);
				break;
			case 13:
				pDefender->bNoDismiss=ReadInt(cSymbol);
				break;			
			case 14:
				for (int k=0; k<MAX_TERRAIN; k++)
				{
					iLength=ReadWord(pWord);
					pDefender->arTerrain[k]=ReadInt(cSymbol);
				};
				break;
			case 15:					
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_DEFENDER_RES)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading defender.var file (Resource), member: %s",pDefender->sName);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pDefender->arRes[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;			
			case 16:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_GUARD_UNIT)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading defender.var file (Units), member: %s",pDefender->sName);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pDefender->arUnit[iCounter][0]=ReadInt(cSymbol);
					CurrentFile.get(cSymbol);
					pDefender->arUnit[iCounter][1]=ReadInt(cSymbol);
					CurrentFile.get(cSymbol);
					pDefender->arUnit[iCounter][2]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			case 17:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_GUARD_LOOT)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading defender.var file (Loot), member: %s",pDefender->sName);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pDefender->arLoot[iCounter][0]=ReadInt(cSymbol);
					CurrentFile.get(cSymbol);
					pDefender->arLoot[iCounter][1]=ReadInt(cSymbol);					
					iCounter++;
				};
				break;
			case 18:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_DEFENDER_ABILITY)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading defender.var file (Ability), member: %s",pDefender->sName);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pDefender->arAbility[iCounter][0]=ReadInt(cSymbol);
					if (pDefender->arAbility[iCounter][0]==10)
					{iMaxDefItem++;};
					iLength=ReadWord(pWord);
					pDefender->arAbility[iCounter][1]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pDefender->arAbility[iCounter][2]=ReadInt(cSymbol);
					iCounter++;
				};
				break;	
			}
		}
		pDefender++;
	}
	CurrentFile.close();
//	bChecker[11]=!(stKey[11]==abs(stLock[5]+stLock[8]-stLock[2]-4)%128);

	if (OpenFile("text/Defender.txt")==0)
	{return 0;};

	for (int i=0; i<=iMaxDefender; i++)
	{
		SkipText();
		(pFirstDefender+i)->sText=ReadText();
	};	
	CurrentFile.close();

//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"Loading defenders.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE* datDefender = load_datafile("dat/Defenders.dat");
	if (!datDefender)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading defenders.dat");		
		readkey();
		return 1;
	};
	for (int i=0; i<iMaxDefender; i++)
	{
		(pFirstDefender+i+1)->Pic=(BITMAP *)datDefender[i].dat;
	};

	//Загрузка сайтов
	if (OpenFile("var/site.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxSite=ReadInt(cSymbol);
	pFirstSite = new clSite[iMaxSite+1];
	clSite *pSite = pFirstSite;
	for (int i=0; i<=iMaxSite; i++)
	{			
		for (int j=1; j<19; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pSite->sName);
				break;
			case 2:
				pSite->iMinLevel=ReadInt(cSymbol);
				break;
			case 3:
				pSite->iMaxLevel=ReadInt(cSymbol);
				break;
			case 4:
				pSite->iGuarded=ReadInt(cSymbol);
				break;			
			case 5:
				pSite->iPlace=ReadInt(cSymbol);
				break;
			case 6:
				pSite->ipEvent=ReadInt(cSymbol);
				break;
			case 7:
				pSite->ipDialog=ReadInt(cSymbol);
				break;
			case 8:
				pSite->ipDialog2=ReadInt(cSymbol);
				break;
			case 9:
				pSite->iTreasure=ReadInt(cSymbol);
				break;
			case 10:
				pSite->iCoastal=ReadInt(cSymbol);
				break;
			case 11:
				pSite->bWorkHide=ReadInt(cSymbol);
				break;
			case 12:
				pSite->bWorkGuard=ReadInt(cSymbol);
				break;
			case 13:
				pSite->bWorkFree=ReadInt(cSymbol);
				break;
			case 14:
				pSite->bCanEnter=ReadInt(cSymbol);
				break;
			case 15:
				pSite->bSelfDestroy=ReadInt(cSymbol);
				break;					
			case 16:
				for (int k=0; k<MAX_TERRAIN; k++)
				{
					iLength=ReadWord(pWord);
					pSite->arTerrain[k]=ReadInt(cSymbol);
				};
				break;
			case 17:					
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_SITE_GUARD)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading site.var file (Guard), member: %s",pSite->sName);
						return 0;
					};
					iLength=ReadWord(pWord);
					pSite->arGuard[iCounter][0]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pSite->arGuard[iCounter][1]=ReadInt(cSymbol);
					iCounter++;
				};
				break;			
			case 18:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_SITE_ABILITY)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading site.var file (Ability), member: %s",pSite->sName);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pSite->arAbility[iCounter][0]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pSite->arAbility[iCounter][1]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pSite->arAbility[iCounter][2]=ReadInt(cSymbol);
					iCounter++;
				};
				break;				
			};
		};
		pSite++;
	};
	CurrentFile.close();

	if (OpenFile("text/Site.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxSite; i++)
	{
		SkipText();
		(pFirstSite+i)->sText=ReadText();
	};	
	CurrentFile.close();

	if (OpenFile("text/Site1.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxSite; i++)
	{
		SkipText();
		(pFirstSite+i)->sFreeText=ReadText();
	};	
	CurrentFile.close();

	NewProgress+=100;
	DATAFILE* datRitual = load_datafile("dat/Ritual.dat");
	if (!datRitual)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading ritual.dat");
		readkey();
		return 1;
	};
	/*for (int i=0; i<iMaxRitual; i++)
	{
		(pFirstRitual+i+1)->Pic=(BITMAP *)datRitual[i].dat;
	};*/


	//Загрузка ритуалов
	if (OpenFile("var/ritual.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxRitual=ReadInt(cSymbol);
	pFirstRitual = new clRitual[iMaxRitual+1];
	clRitual *pRitual = pFirstRitual;
	for (int i=0; i<=iMaxRitual; i++)
	{	
		if (i>0)
		{pRitual->Pic=(BITMAP *)datRitual[i-1].dat;};
		for (int j=1; j<15; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pRitual->sName);
				break;
			case 2:
				pRitual->iGoldCost=ReadInt(cSymbol);
				break;			
			case 3:
				pRitual->iGemCost=ReadInt(cSymbol);
				break;
			case 4:
				pRitual->Sound=ReadInt(cSymbol);
				/*if (iCounter<0)
				{pRitual->Sound=(SAMPLE *)datUnitSound[abs(iCounter)].dat;}
				else
				{pRitual->Sound=(SAMPLE *)datSpellSound[iCounter].dat;};*/
				break;			
			case 5:
				pRitual->iKarma=ReadInt(cSymbol);
				break;
			case 6:
				pRitual->bOnAlly=ReadInt(cSymbol);
				break;
			case 7:
				pRitual->bOnEnemy=ReadInt(cSymbol);
				break;
			case 8:
				pRitual->iTime=ReadInt(cSymbol);
				break;	
			case 9:
				pRitual->iCooldown=ReadInt(cSymbol);
				break;
			case 10:
				pRitual->ipEvent=ReadInt(cSymbol);
				break;
			case 11:
				pRitual->iItemLevel=ReadInt(cSymbol);
				break;
			case 12:
				pRitual->iItemRarity=ReadInt(cSymbol);
				break;
			case 13:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_RITUAL_RES)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading ritual.var file (Resource), member: %s",pRitual->sName);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pRitual->arRes[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;					
			case 14:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_RITUAL_EFFECT)
					{
						textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading ritual.var file (Effect), member: %s",pRitual->sName);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pRitual->arEffect[iCounter][0]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pRitual->arEffect[iCounter][1]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pRitual->arEffect[iCounter][2]=ReadInt(cSymbol);
					iCounter++;
				};
				break;	
			};
		};
		pRitual++;
	};
	CurrentFile.close();

	if (OpenFile("text/Ritual.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxRitual; i++)
	{
		SkipText();
		(pFirstRitual+i)->sText=ReadText();
	};	
	CurrentFile.close();
//	bChecker[12]=!(stKey[12]==abs(stLock[9]+stLock[2]-stLock[13])%128);
		

	//Загрузка предметов
//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"Loading items.dat...");
//	TypeLine++;	
	NewProgress+=100;
	DATAFILE* datItems = load_datafile("dat/Items.dat");
	if (!datItems)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading items.dat");		
		readkey();
		return 1;
	};
	if (OpenFile("var/item.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxPureItem=ReadInt(cSymbol);
	iMaxItem=iMaxPureItem+iMaxSpell+iMaxDefItem+iMaxBldItem+iMaxRitual;
	pFirstItem = new clItem[iMaxItem+1];
	clItem *pItem = pFirstItem;	
	for (int i=0; i<=iMaxPureItem; i++)
	{	
		for (int j=1; j<13; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pItem->sName);
				break;
			case 2:
				pItem->ipSlot=ReadInt(cSymbol);
				break;
			case 3:
				pItem->ipClass=ReadInt(cSymbol);
				break;
			case 4:
				pItem->iDurability=ReadInt(cSymbol);
				break;
			case 5:
				pItem->iGoldCost=ReadInt(cSymbol);
				break;			
			case 6:
				pItem->iGemCost=ReadInt(cSymbol);
				break;
			case 7:
				pItem->iShopLevel=ReadInt(cSymbol);
				break;
			case 8:
				pItem->iRarity=ReadInt(cSymbol);
				break;
			case 9:
				pItem->ipBuilding=ReadInt(cSymbol);
				break;
			case 10:
				pItem->Pic=(BITMAP *)datItems[ReadInt(cSymbol)].dat;
				break;
			case 11:												
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_ITEM_RES)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading item.var file (Resource), member: %s (%d)",pItem->sName, i);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pItem->arRes[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;			
			case 12:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_ITEM_EFFECT)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading item.var file (item effect), member: %s",pItem->sName);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pItem->arEffect[iCounter][0]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pItem->arEffect[iCounter][1]=ReadInt(cSymbol);					
					iLength=ReadWord(pWord);
					pItem->arEffect[iCounter][2]=ReadInt(cSymbol);
					iCounter++;
				};
				break;			
			};
		};
		pItem++;
	};
	//Заполнение массива свитков
	for (int i=1; i<=iMaxSpell; i++)
	{
		sprintf(pItem->sName,"\"%s\"",(pFirstSpell+i)->sName);
		pItem->ipSlot=0;				
		pItem->ipClass=10;
		pItem->iDurability=1;
		pItem->iGoldCost=10;
		for (int j=1; j<=(pFirstSpell+i)->iLevel; j++)
		{
			pItem->iGoldCost*=3;
		};
		pItem->iGoldCost+=((pFirstSpell+i)->iItemLevel)*20;
		pItem->iGemCost=pItem->iGoldCost/5;
		pItem->iShopLevel=(pFirstSpell+i)->iItemLevel;
		pItem->iRarity=(pFirstSpell+i)->iLevel/2+1;
		pItem->ipBuilding=0;
		switch ((pFirstSpell+i)->iLevel)
		{
		case 0:
		case 1:
			pItem->Pic=(BITMAP *)datItems[75].dat;
			break;
		case 2:
			pItem->Pic=(BITMAP *)datItems[76].dat;
			pItem->arRes[0]=4;
			break;
		case 3:
			pItem->Pic=(BITMAP *)datItems[74].dat;
			pItem->arRes[0]=5;
			break;
		case 4:
			pItem->Pic=(BITMAP *)datItems[77].dat;
			if ((i>60)&&(i<70))
			{pItem->arRes[0]=9;}
			else
			{pItem->arRes[0]=8;};
			break;
		};		
		pItem->arEffect[0][0]=83;
		pItem->arEffect[0][1]=i;
		pItem->arEffect[0][2]=0;		
		pItem++;
	};

	//Заполнение массива договоров с охранниками	
	for (int i=1; i<=iMaxDefender; i++)
	{
		int ABL=-1;
		for (int j=0; j<MAX_DEFENDER_ABILITY; j++)
		{
			if ((pFirstDefender+i)->arAbility[j][0]==10)
			{ABL=j;};
		};
		if (ABL>=0)
		{
			sprintf(pItem->sName,"\"%s\"",(pFirstDefender+i)->sName);
			pItem->ipSlot=0;				
			pItem->ipClass=11;
			pItem->iDurability=1;
			pItem->iGoldCost=0;		
			pItem->iGemCost=0;
			pItem->iShopLevel=(pFirstDefender+i)->arAbility[ABL][1];
			pItem->iRarity=(pFirstDefender+i)->arAbility[ABL][2];
			pItem->ipBuilding=0;
			pItem->Pic=(pFirstDefender+i)->Pic;
			pItem->arEffect[0][0]=86;
			pItem->arEffect[0][1]=i;
			pItem->arEffect[0][2]=0;		
			pItem++;
		};
	};

	//Заполнение массива чертежей	
	for (int i=1; i<=iMaxOuterBld; i++)
	{
		int ABL=-1;
		for (int j=0; j<MAX_BUILDING_ABILITY; j++)
		{
			if ((pFirstOuterBld+i)->arAbility[j][0]==8)
			{ABL=j;};
		};
		if (ABL>=0)
		{
			sprintf(pItem->sName,"\"%s\"",(pFirstOuterBld+i)->sName);
			pItem->ipSlot=0;				
			pItem->ipClass=12;
			pItem->iDurability=1;
			pItem->iGoldCost=0;		
			pItem->iGemCost=0;
			pItem->iShopLevel=(pFirstOuterBld+i)->arAbility[ABL][1];
			pItem->iRarity=(pFirstOuterBld+i)->arAbility[ABL][2];
			pItem->ipBuilding=0;
			pItem->Pic=(pFirstOuterBld+i)->Pic;
			pItem->arEffect[0][0]=87;
			pItem->arEffect[0][1]=i;
			pItem->arEffect[0][2]=0;		
			pItem++;
		};
	};

	//Заполнение массива ритуалов
	for (int i=1; i<=iMaxRitual; i++)
	{
		sprintf(pItem->sName,"\"%s\"",(pFirstRitual+i)->sName);
		pItem->ipSlot=0;				
		pItem->ipClass=13;
		pItem->iDurability=1;
		pItem->iGoldCost=0;		
		pItem->iGemCost=0;
		pItem->iShopLevel=(pFirstRitual+i)->iItemLevel;
		pItem->iRarity=(pFirstRitual+i)->iItemRarity;
		pItem->ipBuilding=0;
		pItem->Pic=(pFirstRitual+i)->Pic;
		pItem->arEffect[0][0]=94;
		pItem->arEffect[0][1]=i;
		pItem->arEffect[0][2]=0;		
		pItem++;
	};
	CurrentFile.close();
//	bChecker[13]=!(stKey[13]==abs((stLock[10]+stLock[0]+stLock[15])/3-8)%128);


	//Загрузка иллюстраций к диалогам и событиям
//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"Loading illustration.dat...");
//	TypeLine++;
	NewProgress+=100;
	DATAFILE* datIllustration = load_datafile("dat/Illustration.dat");
	if (!datIllustration)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading illustration.dat");
		readkey();
		return 1;
	};
	for (int i=0; i<MAX_ILLUSTRATION; i++)
	{
		bmpIllustration[i]=(BITMAP *)datIllustration[i].dat;
	};
//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"Loading portraits.dat...");
//	TypeLine++;	
	NewProgress+=100;
	DATAFILE* datPortrait = load_datafile("dat/Portraits.dat");
	if (!datPortrait)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading portraits.dat");	
		readkey();
		return 1;
	};
	for (int i=0; i<MAX_HERO_PIC; i++)
	{
		bmpPortrait[i]=(BITMAP *)datPortrait[i].dat;
	};
	for (int i=0; i<iMaxAvatarPic; i++)
	{
		bmpAvatar[i]=(BITMAP *)datPortrait[i+MAX_HERO_PIC].dat;
	};

//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"Loading smallport.dat...");
//	TypeLine++;	
	NewProgress+=100;
	DATAFILE* datSmallPort = load_datafile("dat/SmallPort.dat");
	if (!datSmallPort)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading smallport.dat");		
		readkey();
		return 1;
	};
	for (int i=0; i<MAX_HERO_PIC; i++)
	{
		bmpSmallPort[i]=(BITMAP *)datSmallPort[i].dat;
	};	
	for (int i=0; i<iMaxAvatarPic; i++)
	{
		bmpSmallAvatar[i]=(BITMAP *)datSmallPort[i+MAX_HERO_PIC].dat;
	};

	//Загрузка диалогов
	if (OpenFile("var/dialog.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxDialog=ReadInt(cSymbol);
	pFirstDialog = new clDialog[iMaxDialog+1];
	clDialog *pDialog = pFirstDialog;
	for (int i=0; i<=iMaxDialog; i++)
	{
		if (i%100==99)
		{NewProgress+=100;};
		iLength=ReadWord(pWord);
		pDialog->iWindow=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		if (pDialog->iWindow==3)
		{pDialog->Pic=bmpAvatar[ReadInt(cSymbol)];}
		else
		{pDialog->Pic=bmpIllustration[ReadInt(cSymbol)];};
		SkipText();
		pDialog->sText=ReadText();
				
		iCounter=0;
		cSymbol='x';
		while (cSymbol!='#')
		{					
			if (iCounter>=MAX_ANSWER)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading dialog.var file (Answer), member: %d",i);
				return 0;
			};
			iLength=ReadWord(pWord);
			iLength=ReadWord(pDialog->arAnswer[iCounter]);
			CurrentFile.get(cSymbol);			
			iCounter++;
		};		
		pDialog++;
	};
//	bChecker[14]=!(stKey[14]==Random(128,stLock[0],stLock[1],stLock[7],stLock[8]));
	CurrentFile.close();

	if (OpenFile("var/campaign_dialog.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxCampDialog=ReadInt(cSymbol);
	pFirstCampDialog = new clDialog[iMaxCampDialog+1];
	pDialog = pFirstCampDialog;
	for (int i=0; i<=iMaxCampDialog; i++)
	{
		if (i%100==99)
		{NewProgress+=100;};
		iLength=ReadWord(pWord);
		pDialog->iWindow=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		if ((pDialog->iWindow==3)||(pDialog->iWindow==5))
		{pDialog->Pic=bmpAvatar[ReadInt(cSymbol)];}
		else
		{pDialog->Pic=bmpIllustration[ReadInt(cSymbol)];};
		SkipText();
		pDialog->sText=ReadText();
				
		iCounter=0;
		cSymbol='x';
		while (cSymbol!='#')
		{					
			if (iCounter>=MAX_ANSWER)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading campaign_dialog.var file (Answer), member: %d",i);
				return 0;
			};
			iLength=ReadWord(pWord);
			iLength=ReadWord(pDialog->arAnswer[iCounter]);
			CurrentFile.get(cSymbol);			
			iCounter++;
		};		
		pDialog++;
	};
	CurrentFile.close();

//Загрузка журнала
	if (OpenFile("text/journal.txt")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxJournal=ReadInt(cSymbol);
	pFirstJournal = new clJournal[iMaxJournal+1];
	clJournal *pJournal = pFirstJournal;
	for (int i=0; i<=iMaxJournal; i++)
	{
		pJournal->Index=i;
		iLength=ReadWord(pWord);
		iLength=ReadWord(pJournal->sName);		
		SkipText();
		pJournal->sText=ReadText();
		pJournal++;
	};
	CurrentFile.close();
//	bChecker[15]=!(stKey[15]==1+Random(122,stLock[2],stLock[4],stLock[9],stLock[10]));

	NewProgress+=100;
	if (OpenFile("var/question.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxQuestion=ReadInt(cSymbol);
	pFirstQuestion = new clQuestion[iMaxQuestion+1];
	clQuestion *pQuestion = pFirstQuestion;
	for (int i=0; i<=iMaxQuestion; i++)
	{		
		iLength=ReadWord(pWord);
		pQuestion->iQuester=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pQuestion->iTrigger=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pQuestion->iEvent=ReadInt(cSymbol);
	//	iLength=ReadWord(pWord);
		SkipText();
		pQuestion->sText=ReadText();
		pQuestion++;
	};
	CurrentFile.close();

//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"Loading innerbld.dat...");
//	TypeLine++;	
	NewProgress+=100;
	DATAFILE* datInnerBld = load_datafile("dat/InnerBld.dat");
	if (!datInnerBld)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading innerbld.dat");
		readkey();
		return 1;
	};

	//Загрузка внутренних зданий
	if (OpenFile("var/inner_build.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxInnerBld=ReadInt(cSymbol);
	if (iMaxInnerBld>MAX_BLD)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Too many inner buildings! (limit is %d)",MAX_BLD);
		return 0;
	};
	pFirstInnerBld = new clInnerBuilding[iMaxInnerBld+1];
	clInnerBuilding *pInnerBld = pFirstInnerBld;
	for (int i=0; i<=iMaxInnerBld; i++)
	{		
		for (int j=1; j<12; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pInnerBld->sName);
				break;
			case 2:				
				pInnerBld->iGoldCost=ReadInt(cSymbol);
				break;			
			case 3:
				pInnerBld->iGemCost=ReadInt(cSymbol);
				break;
			case 4:
				pInnerBld->iGroup=ReadInt(cSymbol);
				break;
			case 5:
				pInnerBld->iSlot=ReadInt(cSymbol);
				break;
			case 6:
				pInnerBld->iUpg=ReadInt(cSymbol);
				break;
			case 7:
				pInnerBld->iHidden=ReadInt(cSymbol);
				break;
			case 8:
				pInnerBld->iLevel=ReadInt(cSymbol);
				break;
			case 9:	
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_BUILDING_NEED)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading inner_build.var file (Buildings), member: %s",pInnerBld->sName);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pInnerBld->arBuildings[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			case 10:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_BUILDING_RES)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading inner_build.var file (Resource), member: %s",pInnerBld->sName);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pInnerBld->arRes[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;			
			case 11:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_BUILDING_ABILITY)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading inner_build.var file (Ability), member: %s",pInnerBld->sName);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pInnerBld->arAbility[iCounter][0]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pInnerBld->arAbility[iCounter][1]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pInnerBld->arAbility[iCounter][2]=ReadInt(cSymbol);
					iCounter++;
				};
				break;	
			};
		};
		pInnerBld->Pic=(BITMAP *)datInnerBld[i].dat;
		pInnerBld++;
	};
	CurrentFile.close();
//	bChecker[16]=!(stKey[16]==21+Random(100,stLock[5],stLock[6],stLock[13],stLock[14]));

	if (OpenFile("text/Inner_Build.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxInnerBld; i++)
	{
		SkipText();
		(pFirstInnerBld+i)->sText=ReadText();
	};	
	CurrentFile.close();

	//Загрузка групп построек
	if (OpenFile("var/build_group.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxBldGroup=ReadInt(cSymbol);	
	for (int i=0; i<=iMaxBldGroup; i++)
	{	
		iLength=ReadWord(pWord);		
		arBldGroup[i]=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		iLength=ReadWord(GroupName[i].str);
	};
	CurrentFile.close();

	//Загрузка наемников
	if (OpenFile("var/mercenary.var")==0)
	{return 0;};	
	for (int Lvl=0; Lvl<MAX_MERC_LEVEL; Lvl++)
	{
		iLength=ReadWord(pWord);
		for (int i=0; i<MAX_MERCENARY; i++)
		{		
			arMercenary[Lvl][i][0]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
			arMercenary[Lvl][i][1]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
		};
	};
	CurrentFile.close();
//	bChecker[17]=!(stKey[17]==6+Random(110,stLock[15],stLock[17]));		

	//Загрузка профилей игроков
	if (OpenFile("var/players.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxPlayerProfile=ReadInt(cSymbol);
	pFirstPlayerProfile = new clPlayerProfile[iMaxPlayerProfile+1];
	clPlayerProfile *pPlayerProfile = pFirstPlayerProfile;
	for (int i=0; i<=iMaxPlayerProfile; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord(pPlayerProfile->sName);
		iLength=ReadWord(pWord);
		iLength=ReadWord(pPlayerProfile->sLand);
		iLength=ReadWord(pWord);
		pPlayerProfile->iKarma=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->iStrategy=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->iMagic=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->iTargeting=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->iDiplomacy=ReadInt(cSymbol);		
		iLength=ReadWord(pWord);		
		pPlayerProfile->iAggro=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->iGreed=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->iLoyal=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->iRandom=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->iSign=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->iColor1=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->iColor2=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->Y=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->ipEventFirstMeet=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->ipEventMeet=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		pPlayerProfile->ipEventDefeat=ReadInt(cSymbol);		
		iLength=ReadWord(pWord);
		pPlayerProfile->ipEventAttack=ReadInt(cSymbol);		
		iLength=ReadWord(pWord);
		pPlayerProfile->iLandType=ReadInt(cSymbol);
		CurrentFile.get(cSymbol);
		pPlayerProfile->iLandPoss=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pPlayerProfile->iProvType=ReadInt(cSymbol);
		CurrentFile.get(cSymbol);
		pPlayerProfile->iProvPoss=ReadInt(cSymbol);
		pPlayerProfile->arAtt[0]=0;
		for (int i=1; i<MAX_PLAYERS; i++)
		{
			iLength=ReadWord(pWord);		
			pPlayerProfile->arAtt[i]=ReadInt(cSymbol);
		};
		iLength=ReadWord(pWord);
		iCounter=0;
		cSymbol='x';
		while (cSymbol!=')')
		{					
			if (iCounter==MAX_PLAYER_BLD)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading Players.var file (Buildings), member: %d",i);
				return 0;
			};
			CurrentFile.get(cSymbol);
			pPlayerProfile->arBuild[iCounter]=ReadInt(cSymbol);
			iCounter++;
		};
		iLength=ReadWord(pWord);
		iCounter=0;
		cSymbol='x';
		while (cSymbol!=')')
		{					
			if (iCounter==MAX_PLAYER_DIALOG)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading Players.var file (EnterDialogs), member: %d",i);
				return 0;
			};
			CurrentFile.get(cSymbol);
			pPlayerProfile->arEnterDlg[iCounter]=ReadInt(cSymbol);
			iCounter++;
		};
		iLength=ReadWord(pWord);
		iCounter=0;
		cSymbol='x';
		while (cSymbol!=')')
		{					
			if (iCounter==MAX_PLAYER_DIALOG)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading Players.var file (ExitDialogs), member: %d",i);
				return 0;
			};
			CurrentFile.get(cSymbol);
			pPlayerProfile->arExitDlg[iCounter]=ReadInt(cSymbol);
			iCounter++;
		};
		pPlayerProfile++;
	};
	CurrentFile.close();

	//Загрузка дипломатической переписки
	for (int i=0; i<=iMaxPlayerProfile; i++)
	{
		NewProgress+=100;
		char sFile[100];
		sprintf(sFile,"text/diplomacy/Player%d.txt",i);
		if (OpenFile(sFile)==0)
		{return 0;};
		cSymbol=' ';
		if (i==0)
		{pHumanDiplo=NULL;}
		else
		{(pFirstPlayerProfile+i)->pDipText=NULL;};
		clDipText *Dip;
		while (cSymbol!='&')
		{
			Dip=new clDipText;
			iLength=ReadWord(pWord);
			Dip->Index=ReadInt(cSymbol);
			iLength=ReadWord(pWord);
			Dip->Min=ReadInt(cSymbol);
			iLength=ReadWord(pWord);
			Dip->Max=ReadInt(cSymbol);
			iLength=ReadWord(pWord);
			Dip->Name=ReadInt(cSymbol);
			iLength=ReadWord(pWord);
			Dip->Param=ReadInt(cSymbol);
			SkipText();
			Dip->sText=ReadText();
			if (i==0)
			{
				Dip->pNext=pHumanDiplo;
				pHumanDiplo=Dip;
			}
			else
			{
				Dip->pNext=(pFirstPlayerProfile+i)->pDipText;
				(pFirstPlayerProfile+i)->pDipText=Dip;
			};
			ReadInt(cSymbol);
			ReadInt(cSymbol);
			ReadInt(cSymbol);
		};
		CurrentFile.close();
	};

	//Загрузка сообщений
	if (OpenFile("text/message.txt")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxMessage=ReadInt(cSymbol);
	pFirstMessage=new stMessage [iMaxMessage+1];
	for (int i=0; i<=iMaxMessage; i++)
	{	
		SkipText();
		(pFirstMessage+i)->msg=ReadText();	
	};
	CurrentFile.close();

	if (OpenFile("text/tutorial.txt")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxTutText=ReadInt(cSymbol);
	pFirstTutText=new stMessage [iMaxTutText+1];
	for (int i=0; i<=iMaxTutText; i++)
	{	
		SkipText();
		(pFirstTutText+i)->msg=ReadText();	
	};
	CurrentFile.close();
//	bChecker[2]=!(stKey[2]==abs(44+stLock[14]-stLock[2]+stLock[9])%128);

	if (OpenFile("text/hint.txt")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxHint=ReadInt(cSymbol);
	pFirstHint=new stMessage [iMaxHint+1];
	for (int i=0; i<=iMaxHint; i++)
	{	
		SkipText();
		(pFirstHint+i)->msg=ReadText();	
	};
	CurrentFile.close();

	//Загрузка цен героев
	if (OpenFile("var/heroPrice.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iHeroPriceMult=ReadInt(cSymbol);
	for (int i=0; i<4; i++)
	{	
		iLength=ReadWord(pWord);		
		arHeroPrice[i][0]=ReadInt(cSymbol);
		CurrentFile.get(cSymbol);
		arHeroPrice[i][1]=ReadInt(cSymbol);
	};
	for (int i=0; i<2; i++)
	{	
		iLength=ReadWord(pWord);		
		arHeroRess[i][0]=ReadInt(cSymbol);
		CurrentFile.get(cSymbol);
		arHeroRess[i][1]=ReadInt(cSymbol);
	};
	CurrentFile.close();
//	bChecker[0]=!(stKey[0]==abs(5+stLock[13]+stLock[17]-stLock[6])%128);

	
	//Загрузка цен героев
	if (OpenFile("var/CampDiff.var")==0)
	{return 0;};
//	iLength=ReadWord(pWord);	
	for (int i=0; i<MAX_DIFFICULTY; i++)
	{
		iLength=ReadWord(pWord);
		for (int j=0; j<5; j++)
		{		
			arCampDiff[i][j]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
		};
	};	
	CurrentFile.close();

	//Загрузка наборов предметов
	if (OpenFile("var/item_set.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxItemSet=ReadInt(cSymbol);
	pWorkSet= new int[iMaxItemSet+1];
	pFirstItemSet = new clItemSet[iMaxItemSet+1];
	clItemSet *pItemSet = pFirstItemSet;
	for (int i=0; i<=iMaxItemSet; i++)
	{			
		iLength=ReadWord(pWord);
		iLength=ReadWord(pItemSet->sName);
		iLength=ReadWord(pWord);
		pItemSet->iSize=ReadInt(cSymbol);
		if (pItemSet->iSize>MAX_SET_ITEMS)
		{
			textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading item_set.var file, member: %s. Too many items.",pItemSet->sName);
			return 0;
		};
		iLength=ReadWord(pWord);
		for (int j=0; j<pItemSet->iSize; j++)
		{
			pItemSet->arItems[j]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
		};
		iLength=ReadWord(pWord);
		int iBonus=ReadInt(cSymbol);
		if (iBonus>MAX_SET_BONUS)
		{
			textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading item_set.var file, member: %s. Too many bonuses.",pItemSet->sName);
			return 0;
		};
		for (int j=0; j<iBonus; j++)
		{
			iLength=ReadWord(pWord);
			pItemSet->arBonusQ[j]=ReadInt(cSymbol);
			iCounter=0;
			cSymbol='x';
			while (cSymbol!=';')
			{					
				if (iCounter==MAX_SET_ABILITY)
				{
					textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading item_set.var file, member: %s. Too many abilities.",pItemSet->sName);					
					return 0;
				};
				iLength=ReadWord(pWord);
				pItemSet->arBonus[j][iCounter][0]=ReadInt(cSymbol);
				iLength=ReadWord(pWord);
				pItemSet->arBonus[j][iCounter][1]=ReadInt(cSymbol);					
				iLength=ReadWord(pWord);
				pItemSet->arBonus[j][iCounter][2]=ReadInt(cSymbol);
				iCounter++;
			};
		};	
		pItemSet++;
	};
	CurrentFile.close();	

	//Загрузка наград юнитов
	if (OpenFile("var/medal.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxMedal=ReadInt(cSymbol);
	pFirstMedal = new clMedal[iMaxMedal+1];
	clMedal *pMedal = pFirstMedal;
	for (int i=0; i<=iMaxMedal; i++)
	{			
		iLength=ReadWord(pWord);
		iLength=ReadWord(pMedal->sName);
		iLength=ReadWord(pWord);
		pMedal->iGoldSpent=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pMedal->iGemSpent=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pMedal->iRarity=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		iCounter=0;
		cSymbol='x';
		while (cSymbol!=';')
		{					
			if (iCounter==MAX_MEDAL_EFFECT)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading medal_set.var file, member: %s. Too many abilities.",pMedal->sName);					
				return 0;
			};
			iLength=ReadWord(pWord);
			pMedal->arEffect[iCounter][0]=ReadInt(cSymbol);
			iLength=ReadWord(pWord);
			pMedal->arEffect[iCounter][1]=ReadInt(cSymbol);			
			iCounter++;		
		};	
		pMedal++;
	};
	CurrentFile.close();

	if (OpenFile("text/medal.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxMedal; i++)
	{	
		SkipText();
		(pFirstMedal+i)->sText=ReadText();	
	};
	CurrentFile.close();

	//Загрузка событий
	if (OpenFile("var/event.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxEvent=ReadInt(cSymbol);	
	pFirstEvent = new clEvent[iMaxEvent+1];
	clEvent *pEvent = pFirstEvent;
	for (int i=0; i<=iMaxEvent; i++)
	{
		if (i%70==69)
		{NewProgress+=100;};
		for (int j=1; j<14; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				pEvent->iType=ReadInt(cSymbol);				
				break;
			case 2:
				pEvent->iPossibility=ReadInt(cSymbol);				
				break;
			case 3:				
				pEvent->ipDialog=ReadInt(cSymbol);
				break;			
			case 4:
				pEvent->iDlgParam1=ReadInt(cSymbol);
				break;
			case 5:
				pEvent->iDlgParam2=ReadInt(cSymbol);
				break;
			case 6:
				pEvent->ipAttacker=ReadInt(cSymbol);
				break;
			case 7:
				pEvent->ipEventWin=ReadInt(cSymbol);
				break;
			case 8:
				pEvent->ipEventLose=ReadInt(cSymbol);
				break;
			case 9:
				pEvent->ipEventDraw=ReadInt(cSymbol);
				break;
			case 10:
				pEvent->ipProvType=ReadInt(cSymbol);
				break;
			case 11:
				pEvent->iKarma=ReadInt(cSymbol);
				break;
			case 12:				
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_ANSWER)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading event.var file (Answers), member: %d",i);
						return 0;
					};
					iLength=ReadWord(pWord);					
					pEvent->arEvent[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			case 13:			
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_EVENT_EFFECT)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading event.var file (Effects), member: %d",i);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pEvent->arEffect[iCounter][0]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pEvent->arEffect[iCounter][1]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pEvent->arEffect[iCounter][2]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pEvent->arEffect[iCounter][3]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			};
		};
		/*if (i>135)
		{
			textprintf(screen,Font1,20,400,COLOR_2,"Событие: %d, Вероятность: %d",i,pEvent->iPossibility);
			readkey();
		};*/
		pEvent++;
	};
	CurrentFile.close();

	//Загрузка событий кампании
	if (OpenFile("var/camp_event.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxCampEvent=ReadInt(cSymbol);	
	pFirstCampEvent = new clEvent[iMaxCampEvent+1];
	pEvent = pFirstCampEvent;
	for (int i=0; i<=iMaxCampEvent; i++)
	{		
		for (int j=1; j<14; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				pEvent->iType=ReadInt(cSymbol);				
				break;
			case 2:
				pEvent->iPossibility=ReadInt(cSymbol);				
				break;
			case 3:				
				pEvent->ipDialog=ReadInt(cSymbol);
				break;			
			case 4:
				pEvent->iDlgParam1=ReadInt(cSymbol);
				break;
			case 5:
				pEvent->iDlgParam2=ReadInt(cSymbol);
				break;
			case 6:
				pEvent->ipAttacker=ReadInt(cSymbol);
				break;
			case 7:
				pEvent->ipEventWin=ReadInt(cSymbol);
				break;
			case 8:
				pEvent->ipEventLose=ReadInt(cSymbol);
				break;
			case 9:
				pEvent->ipEventDraw=ReadInt(cSymbol);
				break;
			case 10:
				pEvent->ipProvType=ReadInt(cSymbol);
				break;
			case 11:
				pEvent->iKarma=ReadInt(cSymbol);
				break;
			case 12:				
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_ANSWER)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading camp_event.var file (Answers), member: %d",i);
						return 0;
					};
					iLength=ReadWord(pWord);					
					pEvent->arEvent[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			case 13:			
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_EVENT_EFFECT)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading camp_event.var file (Effects), member: %d",i);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pEvent->arEffect[iCounter][0]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pEvent->arEffect[iCounter][1]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pEvent->arEffect[iCounter][2]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pEvent->arEffect[iCounter][3]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			};
		};		
		pEvent++;
	};
	CurrentFile.close();

	//Загрузка приключений героев
	if (OpenFile("var/encounter.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxEncounter=ReadInt(cSymbol);	
	pFirstEncounter = new clEncounter[iMaxEncounter+1];
	clEncounter *pEncounter = pFirstEncounter;
	for (int i=0; i<=iMaxEncounter; i++)
	{
		if (i%70==69)
		{NewProgress+=100;};
		for (int j=1; j<13; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				pEncounter->iType=ReadInt(cSymbol);				
				break;
			case 2:
				pEncounter->iPossibility=ReadInt(cSymbol);				
				break;
			case 3:				
				pEncounter->ipDialog=ReadInt(cSymbol);
				break;			
			case 4:
				pEncounter->iDlgParam1=ReadInt(cSymbol);
				break;
			case 5:
				pEncounter->iDlgParam2=ReadInt(cSymbol);
				break;
			case 6:
				pEncounter->ipAttacker=ReadInt(cSymbol);
				break;
			case 7:
				pEncounter->ipEncWin=ReadInt(cSymbol);
				break;
			case 8:
				pEncounter->ipEncLose=ReadInt(cSymbol);
				break;
			case 9:
				pEncounter->ipEncDraw=ReadInt(cSymbol);
				break;			
			case 10:
				pEncounter->iKarma=ReadInt(cSymbol);
				break;
			case 11:				
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_ANSWER)
					{
						textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading encounter.var file (Answers), member: %d",i);
						return 0;
					};
					iLength=ReadWord(pWord);					
					pEncounter->arEncounter[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			case 12:			
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_EVENT_EFFECT)
					{
						textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading encounter.var file (Effects), member: %d",i);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pEncounter->arEffect[iCounter][0]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pEncounter->arEffect[iCounter][1]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pEncounter->arEffect[iCounter][2]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pEncounter->arEffect[iCounter][3]=ReadInt(cSymbol);
					iCounter++;
				};
				break;	
			};
		};
		pEncounter++;
	};
	CurrentFile.close();
	
	//Загрузка заданий героев
	if (OpenFile("var/quest.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxQuest=ReadInt(cSymbol);	
	pFirstQuest = new clQuest[iMaxQuest+1];
	clQuest *pQuest = pFirstQuest;
	for (int i=0; i<=iMaxQuest; i++)
	{		
		for (int j=1; j<18; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pQuest->sName);
				break;
			case 2:
				pQuest->iGiver=ReadInt(cSymbol);				
				break;
			case 3:
				pQuest->iType=ReadInt(cSymbol);				
				break;
			case 4:
				pQuest->iPossibility=ReadInt(cSymbol);				
				break;
			case 5:				
				pQuest->ipEncInit=ReadInt(cSymbol);
				break;			
			case 6:
				pQuest->ipEncWait=ReadInt(cSymbol);
				break;
			case 7:
				pQuest->ipEncFound=ReadInt(cSymbol);
				break;
			case 8:
				pQuest->ipEncNotFound=ReadInt(cSymbol);
				break;
			case 9:
				pQuest->ipEncDone=ReadInt(cSymbol);
				break;
			case 10:
				pQuest->ipEncSuccess=ReadInt(cSymbol);
				break;
			case 11:
				pQuest->ipEncFailGiver=ReadInt(cSymbol);
				break;			
			case 12:
				pQuest->ipEncFailTarget=ReadInt(cSymbol);
				break;
			case 13:
				pQuest->ipEncFailTime=ReadInt(cSymbol);
				break;
			case 14:
				pQuest->iTime=ReadInt(cSymbol);
				break;
			case 15:
				pQuest->iExp=ReadInt(cSymbol);
				break;				
			case 16:				
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_QUEST_TASK)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading quest.var file (Objective), member: %d",i);
						return 0;
					};
					iLength=ReadWord(pWord);
					pQuest->Index[iCounter]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pQuest->Param[iCounter]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pQuest->Quantity[iCounter]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pQuest->Target[iCounter]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pQuest->Place[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			case 17:			
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=';')
				{					
					if (iCounter==MAX_QUEST_ABILITY)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading encounter.var file (Effects), member: %d",i);						
						return 0;
					};
					iLength=ReadWord(pWord);
					pQuest->arAbility[iCounter][0]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pQuest->arAbility[iCounter][1]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pQuest->arAbility[iCounter][2]=ReadInt(cSymbol);
					iLength=ReadWord(pWord);
					pQuest->arAbility[iCounter][3]=ReadInt(cSymbol);
					iCounter++;
				};
				break;	
			};
		};
		pQuest++;
	};
	CurrentFile.close();

	if (OpenFile("text/Quest.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxQuest; i++)
	{	
		SkipText();
		(pFirstQuest+i)->sText=ReadText();	
	};
	CurrentFile.close();

	//Загрузка настроек случайной карты
	/*if (OpenFile("var/map.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	MapSet.Players=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MapSet.Difficulty=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MapSet.X=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MapSet.Y=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MapSet.Type=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MapSet.Water=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MapSet.Hill=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MapSet.Forest=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MapSet.Swamp=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MapSet.Isles=ReadInt(cSymbol);
	iLength=ReadWord(pWord);
	MapSet.Dissolve=ReadInt(cSymbol);		
	CurrentFile.close();*/
	

	//Загрузка объектов карты
//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"Loading MapObject.dat...");
//	TypeLine++;	
	NewProgress+=100;
	DATAFILE* datNature = load_datafile("dat/Nature.dat");
	if (!datNature)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading nature.dat");
		readkey();
		return 1;
	};
	bmpMapGrass=(BITMAP *)datNature[0].dat;
	bmpMapSwamp=(BITMAP *)datNature[1].dat;
	bmpMapSand=(BITMAP *)datNature[34].dat;
	for (int i=0; i<5; i++)
	{bmpMapWater[i]=(BITMAP *)datNature[253+i].dat;};
	bmpMapDark=(BITMAP *)datNature[36].dat;
	
	if (OpenFile("var/mapobject.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxMapObject=ReadInt(cSymbol);	
	pFirstMapObject = new clMapObject[iMaxMapObject+1];
	clMapObject *pMapObject = pFirstMapObject;
	for (int i=0; i<=iMaxMapObject; i++)
	{		
		for (int j=1; j<10; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{			
			case 1:
				pMapObject->iType=ReadInt(cSymbol);
				break;
			case 2:
				pMapObject->iLayer=ReadInt(cSymbol);
				break;
			case 3:
				pMapObject->dx=ReadInt(cSymbol);
				break;
			case 4:
				pMapObject->dy=ReadInt(cSymbol);
				break;
			case 5:
				pMapObject->iSite=ReadInt(cSymbol);
				break;
			case 6:
				for (int k=0; k<MAX_TERRAIN+2; k++)
				{
					iLength=ReadWord(pWord);
					pMapObject->arLand[k]=ReadInt(cSymbol);					
				};
				break;
			//	textprintf(screen, Font1,0,400,COLOR_2,"Desert: %d, Tundra: %d",pMapObject->arLand[0],pMapObject->arLand[1]);
			case 7:					
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_MAPOBJ_TYPE)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading MapObject.var file (ProvType), member: %d",i);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pMapObject->arType[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			case 8:
				pMapObject->bLandmark=ReadInt(cSymbol);
				break;
			case 9:					
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_MAPOBJ_PIC)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading MapObject.var file (Picture), member: %d",i);
						return 0;
					};
					CurrentFile.get(cSymbol);
					int iPic=ReadInt(cSymbol);
					if (iPic>0)
					{
						pMapObject->arPic[iCounter]=(BITMAP *)datNature[iPic].dat;
					};
					iCounter++;
				};
				break;
			};
		};
		pMapObject++;
	};
	CurrentFile.close();


	if (OpenFile("var/landmark.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxLandmark=ReadInt(cSymbol);	
	pFirstLandmark = new clLandmark[iMaxLandmark+1];
	clLandmark *pLandmark = pFirstLandmark;
	for (int i=0; i<=iMaxLandmark; i++)
	{		
		iLength=ReadWord(pWord);
		pLandmark->iType=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pLandmark->iLayer=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pLandmark->dx=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		pLandmark->dy=ReadInt(cSymbol);

		iLength=ReadWord(pWord);
		iCounter=0;
		cSymbol='x';
		while (cSymbol!=')')
		{					
			if (iCounter==MAX_TERRAIN+2)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading Landmark.var file (Terrain), member: %d",i);
				return 0;
			};
			CurrentFile.get(cSymbol);
			pLandmark->arLand[iCounter]=ReadInt(cSymbol);
			iCounter++;
		};

		iLength=ReadWord(pWord);
		iCounter=0;
		cSymbol='x';
		while (cSymbol!=')')
		{					
			if (iCounter==MAX_MAPOBJ_TYPE)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading Landmark.var file (ProvType), member: %d",i);
				return 0;
			};
			CurrentFile.get(cSymbol);
			pLandmark->arType[iCounter]=ReadInt(cSymbol);
			iCounter++;
		};

		iLength=ReadWord(pWord);
		pLandmark->iSite=ReadInt(cSymbol);

		iLength=ReadWord(pWord);
		int iPic=ReadInt(cSymbol);
		pLandmark->Pic=(BITMAP *)datNature[iPic].dat;

		iLength=ReadWord(pWord);
		iPic=ReadInt(cSymbol);
		if (iPic>0)
		{
			pLandmark->Shadow=(BITMAP *)datNature[iPic].dat;				
		};			
		pLandmark++;
	};
	CurrentFile.close();

	//Загрузка графики замка
	DATAFILE *datCastle = load_datafile("dat/Castle.dat");
	NewProgress+=100;
	if (!datCastle)
	{
		textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading castle.dat");
		return 0;
	};
	if (OpenFile("var/castle.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxCastle=ReadInt(cSymbol);
	pFirstCastle = new clCastle[iMaxCastle+1];
	clCastle *pCastle = pFirstCastle;
	for (int i=0; i<=iMaxCastle; i++)
	{
		for (int j=1; j<9; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{			
			case 1:
				pCastle->iLayer=ReadInt(cSymbol);
				break;
			case 2:
				pCastle->iOpacity=ReadInt(cSymbol);
				break;
			case 3:
				pCastle->x=ReadInt(cSymbol);
				break;
			case 4:
				pCastle->y=ReadInt(cSymbol);
				break;
			case 5:
				pCastle->iGroup=ReadInt(cSymbol);
				if ((pCastle->iGroup<0)||(pCastle->iGroup>=MAX_GROUP))
				{
					textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading Castle.var file (Groups), member: %d",i);
					return 0;
				};
				break;
			case 6:
				pCastle->Yplace=ReadInt(cSymbol);
				break;			
			case 7:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_CASTLE_BLD)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading Castle.var file (Buildings), member: %d",i);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pCastle->arBuild[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			case 8:
				iCounter=0;
				cSymbol='x';
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_CASTLE_BLD)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading Castle.var file (AntiBuildings), member: %d",i);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pCastle->arAntiBuild[iCounter]=ReadInt(cSymbol);
					iCounter++;
				};
				break;
			};
		};
		pCastle->Pic=(BITMAP *)datCastle[i].dat;
		pCastle++;
	};
	CurrentFile.close();

	//Загрузка массива вероятности появления спрайтов на слоях карты
	if (OpenFile("var/terr_layer.var")==0)
	{return 0;};
	for (int i=0; i<MAX_TERRAIN+2; i++)
	{
		iLength=ReadWord(pWord);
		for (int j=0; j<MAX_TERR_LAYER; j++)
		{			
			arTerrLayer[i][j]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
		};
	};
	CurrentFile.close();

	//Загрузка размеров осколков
	if (OpenFile("var/shard.var")==0)
	{return 0;};	
	for (int i=0; i<MAX_SHARD_SIZE; i++)
	{			
		iLength=ReadWord(pWord);
		iLength=ReadWord(arShardSize[i].sName);
		iLength=ReadWord(pWord);
		arShardSize[i].x=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arShardSize[i].y=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arShardSize[i].Players=ReadInt(cSymbol);
	};
	CurrentFile.close();

	for (int i=0; i<MAX_GROUP; i++)
	{
		for (int j=0; j<MAX_GROUP_BUILD; j++)
		{
			arGroupCoord[i][j][0]=0;
			arGroupCoord[i][j][1]=0;
		};
	};
	if (OpenFile("var/group_coord.var")==0)
	{return 0;};	
	for (int i=0; i<MAX_GROUP; i++)
	{
		iLength=ReadWord(pWord);
		iCounter=0;
		cSymbol='x';
		while (cSymbol!=')')
		{					
			if (iCounter==MAX_GROUP_BUILD)
			{
				textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading group_coord.var file (AntiBuildings), member: %d",i);
				return 0;
			};
			CurrentFile.get(cSymbol);
			arGroupCoord[i][iCounter][0]=ReadInt(cSymbol);
			CurrentFile.get(cSymbol);
			arGroupCoord[i][iCounter][1]=ReadInt(cSymbol);
			iCounter++;
		};
	};
	CurrentFile.close();

	//Загрузка советов героев
	if (OpenFile("var/hero_advice.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxHeroAdvice=ReadInt(cSymbol);
	pFirstHeroAdvice=new clHeroAdvice[iMaxHeroAdvice+1];
	for (int i=0; i<=iMaxHeroAdvice; i++)
	{	
		iLength=ReadWord(pWord);		
		(pFirstHeroAdvice+i)->Min=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		(pFirstHeroAdvice+i)->Max=ReadInt(cSymbol);
		iLength=ReadWord(pWord);				
		iLength=ReadWord((pFirstHeroAdvice+i)->sText);		
	};
	CurrentFile.close();

	//Загрузка прозвищ правителей
	if (OpenFile("var/ruler_nick.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxNick=ReadInt(cSymbol);
	pFirstNick=new clHeroAdvice[iMaxNick+1];
	for (int i=0; i<=iMaxNick; i++)
	{	
		iLength=ReadWord(pWord);		
		(pFirstNick+i)->Min=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		(pFirstNick+i)->Max=ReadInt(cSymbol);
		iLength=ReadWord(pWord);				
		iLength=ReadWord((pFirstNick+i)->sText);		
	};
	CurrentFile.close();

	//Загрузка титулов правителей
	if (OpenFile("var/ruler_title.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxTitle=ReadInt(cSymbol);
	pFirstTitle=new clHeroAdvice[iMaxTitle+1];
	for (int i=0; i<=iMaxTitle; i++)
	{	
		iLength=ReadWord(pWord);		
		(pFirstTitle+i)->Min=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		(pFirstTitle+i)->Max=ReadInt(cSymbol);
		iLength=ReadWord(pWord);				
		iLength=ReadWord((pFirstTitle+i)->sText);		
	};
	CurrentFile.close();

	//Загрузка названий отношений
	if (OpenFile("var/attitude.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxAttitude=ReadInt(cSymbol);
	pFirstAttitude=new clHeroAdvice[iMaxAttitude+1];
	for (int i=0; i<=iMaxAttitude; i++)
	{	
		iLength=ReadWord(pWord);		
		(pFirstAttitude+i)->Min=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		(pFirstAttitude+i)->Max=ReadInt(cSymbol);
		iLength=ReadWord(pWord);				
		iLength=ReadWord((pFirstAttitude+i)->sText);
	};
	CurrentFile.close();

	//Загрузка титров
	if (OpenFile("var/credits.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxCredits=ReadInt(cSymbol);
	pFirstCredits=new clCredits[iMaxCredits];
	for (int i=0; i<iMaxCredits; i++)
	{
		iLength=ReadWord(pWord);
		iLength=ReadWord((pFirstCredits+i)->pTitle.str);
		iLength=ReadWord(pWord);		
		(pFirstCredits+i)->Number=ReadInt(cSymbol);
		for (int j=0; j<(pFirstCredits+i)->Number; j++)
		{
			iLength=ReadWord(pWord);
			iLength=ReadWord((pFirstCredits+i)->pName[j].str);
		};			
	};
	CurrentFile.close();
//	bChecker[1]=!(stKey[1]==abs(11+stLock[0]+stLock[1]+stLock[8])%128);

	//Загрузка бонусов осколков
	if (OpenFile("var/shard_bonus.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxShardBonus=ReadInt(cSymbol);
	pFirstShardBonus=new clShardBonus[iMaxShardBonus+1];
	for (int i=0; i<=iMaxShardBonus; i++)
	{	
		iLength=ReadWord(pWord);
		iLength=ReadWord((pFirstShardBonus+i)->sName);
		iLength=ReadWord(pWord);		
		(pFirstShardBonus+i)->iType=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		(pFirstShardBonus+i)->iRarity=ReadInt(cSymbol);
		iLength=ReadWord(pWord);		
		(pFirstShardBonus+i)->iPrice=ReadInt(cSymbol);
		for (int Lvl=0; Lvl<MAX_SHARD_LEVEL; Lvl++)
		{
			int Bonus=0;
			iLength=ReadWord(pWord);
			CurrentFile.get(cSymbol);
			while (cSymbol!=';')
			{
				if (Bonus==MAX_SHARD_BONUS_TYPE)
				{
					textprintf_ex(screen,Font1,100,720,makecol(230,20,30),-1,"Error reading shard_bonus.var file , member: %s, level: %d",(pFirstShardBonus+i)->sName,Lvl+1);
					textprintf_ex(screen1,Font1,100,720,makecol(230,20,30),-1,"Error reading shard_bonus.var file , member: %s, level: %d",(pFirstShardBonus+i)->sName,Lvl+1);
					return 0;
				};				
				(pFirstShardBonus+i)->arLevel[Lvl][Bonus][0]=ReadInt(cSymbol);
				CurrentFile.get(cSymbol);
				(pFirstShardBonus+i)->arLevel[Lvl][Bonus][1]=ReadInt(cSymbol);
				CurrentFile.get(cSymbol);
				Bonus++;
			};
		};		
	};
	CurrentFile.close();

	//Загрузка уровней сложности
	if (OpenFile("var/difficulty.var")==0)
	{return 0;};	
	for (int i=0; i<MAX_DIFFICULTY; i++)
	{			
		iLength=ReadWord(pWord);
		iLength=ReadWord(arDifficulty[i].sName);
		iLength=ReadWord(pWord);
		arDifficulty[i].iGold=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arDifficulty[i].iGem=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arDifficulty[i].iGoldAdd=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arDifficulty[i].iGemAdd=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arDifficulty[i].iPopulation=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arDifficulty[i].iExp=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arDifficulty[i].iLoot=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arDifficulty[i].iNeutralLife=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arDifficulty[i].iCorruptStart=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arDifficulty[i].iCorruptCap=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arDifficulty[i].bShowAdvice=ReadInt(cSymbol);
		iLength=ReadWord(pWord);
		arDifficulty[i].iScore=ReadInt(cSymbol);		
	};
	CurrentFile.close();

	if (bUseWorldRules)
	{
//	Загрузка особых правил мира
	if (OpenFile("var/world_rules.var")==0)
	{return 0;};
	iLength=ReadWord(pWord);
	iMaxWorldRules=ReadInt(cSymbol);
	pFirstWorldRules = new clWorldRules[iMaxWorldRules+1];
	clWorldRules *pWorldRules = pFirstWorldRules;		
	for (int i=0; i<=iMaxWorldRules; i++)
	{
		for (int j=1; j<12; j++)
		{
			iLength=ReadWord(pWord);
			switch (j)
			{
			case 1:
				iLength=ReadWord(pWorldRules->sName);
				break;
			case 2:
				pWorldRules->iShardSize=ReadInt(cSymbol);
				break;
			case 3:
				pWorldRules->ipStartEvent=ReadInt(cSymbol);
				break;
			case 4:
				pWorldRules->iParam1=ReadInt(cSymbol);
				break;
			case 5:
				pWorldRules->iParam2=ReadInt(cSymbol);
				break;			
			case 6:
				pWorldRules->ipSpecialEvent=ReadInt(cSymbol);
				break;
			case 7:
				pWorldRules->iSpecialEventTurn=ReadInt(cSymbol);
				break;
			case 8:
				pWorldRules->ipSiteEvent=ReadInt(cSymbol);
				break;
			case 9:
				iCounter=0;
				cSymbol='x';				
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_WORLD_EVENTS)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading world_rules.var file, events, member: %s",pWorldRules->sName);
						return 0;
					};
					CurrentFile.get(cSymbol);
					pWorldRules->arRandomEvents[iCounter][0]=ReadInt(cSymbol);
					CurrentFile.get(cSymbol);
					pWorldRules->arRandomEvents[iCounter][1]=ReadInt(cSymbol);
					iCounter++;
				}				
				break;
			case 10:
				iCounter=0;
				cSymbol='x';				
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_WORLD_PROVTYPES)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading world_rules.var file, provtypes, member: %s",pWorldRules->sName);						
						return 0;
					};
					CurrentFile.get(cSymbol);
					pWorldRules->arProvTypes[iCounter][0]=ReadInt(cSymbol);
					CurrentFile.get(cSymbol);
					pWorldRules->arProvTypes[iCounter][1]=ReadInt(cSymbol);
					iCounter++;
				}				
				break;
			case 11:
				iCounter=0;
				cSymbol='x';				
				while (cSymbol!=')')
				{					
					if (iCounter==MAX_WORLD_SITES)
					{
						textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading world_rules.var file, sites, member: %s",pWorldRules->sName);						
						return 0;
					};
					CurrentFile.get(cSymbol);
					pWorldRules->arSites[iCounter][0]=ReadInt(cSymbol);
					CurrentFile.get(cSymbol);
					pWorldRules->arSites[iCounter][1]=ReadInt(cSymbol);
					iCounter++;
				}				
				break;
			};
		};
		pWorldRules++;
	};
	CurrentFile.close();	

	if (OpenFile("text/world_rules.txt")==0)
	{return 0;};
	for (int i=0; i<=iMaxWorldRules; i++)
	{
		SkipText();
		(pFirstWorldRules+i)->sText=ReadText();
	};	
	CurrentFile.close();

	}

#ifdef YUPLAY
	//Загрузка лотерейных билетов
	if (OpenFile("var/yuplay.var")==0)
	{return 0;};
	for (int i=0; i<MAX_TICKET_TYPE; i++)
	{
		iLength=ReadWord(pWord);
		arTicketType[i].Number=ReadInt(cSymbol);
		for (int j=0; j<arTicketType[i].Number; j++)
		{
			iLength=ReadWord(pWord);
			arTicketType[i].Str[j].iColor=ReadInt(cSymbol);
			iLength=ReadWord(pWord);
			iLength=ReadWord(arTicketType[i].Str[j].str);
		};
	};	
	CurrentFile.close();
#endif

	bDemo=false;

	//for (int i=0; i<17; i++)
	//{bDemo=(bDemo||(bChecker[i]));};

	NewProgress+=100;
//	WriteKeys();

	//********Загрузка звука*********
	if (bSound)
	{
		datMusic = load_datafile("dat/music.dat");
		if (!datMusic)
		{
			textprintf_ex(screen,Font1,100,240,makecol(230,20,30),-1,"Error reading music.dat");
			readkey();
			return 1;
		};
		NewProgress+=100;

		datUnitSound = load_datafile("dat/unit_sound.dat");
		if (!datUnitSound)
		{
			textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading unit_sound.dat");
			return 0;
		};
		NewProgress+=100;

		datSpellSound = load_datafile("dat/spell_sound.dat");
		if (!datSpellSound)
		{
			textprintf_ex(screen1,Font1,100,240,makecol(230,20,30),-1,"Error reading spell_sound.dat");
			return 0;
		};
		NewProgress+=100;
	};

	iActivePlayer=0;
	
	//№ Текущего игрока
	if (al_findfirst("save/game.con",&Fil,FA_ALL)==0)
	{
		WorkFile=pack_fopen("save/game.con","r");
		pack_fread(&iActivePlayer,sizeof iActivePlayer,WorkFile);
		pack_fclose(WorkFile);
	}
	else
	{
		WorkFile=pack_fopen("save/game.con","w");
		pack_fwrite(&iActivePlayer,sizeof iActivePlayer,WorkFile);
		pack_fclose(WorkFile);
	};
	al_findclose(&Fil);

	//Таблица рекордов
	if (al_findfirst("save/ruler.boo",&Fil,FA_ALL)==0)
	{
		WorkFile=pack_fopen("save/ruler.boo","rp");
		pack_fread(arRecords,(sizeof *arRecords)*10,WorkFile);
		pack_fclose(WorkFile);
	}
	else
	{
		FillTab();
		SaveRecords();
	};
	al_findclose(&Fil);

	if (al_findfirst("save/eador.boo",&Fil,FA_ALL)==0)
	{
		WorkFile=pack_fopen("save/eador.boo","rp");
		pack_fread(arCampRecords,(sizeof *arCampRecords)*10,WorkFile);
		pack_fclose(WorkFile);
	}
	else
	{
		for (int i=0; i<10; i++)
		{
			arCampRecords[i].Score=-30000;
		};
		SaveCampRecords();
	};
	al_findclose(&Fil);

//	al_ffblk ProFile;
	//Профайл текущего игрока
		
	sprintf(FileName,"save/player%d.prf",iActivePlayer);
	if (al_findfirst(FileName,&Fil,FA_ALL)==0)
	{
	//	PACKFILE *WorkFile;	
		WorkFile=pack_fopen(FileName,"rp");
		pack_fread(&arHumans[0],sizeof arHumans[0],WorkFile);
		pack_fclose(WorkFile);
		iMusicVolume=arHumans[0].iMusVolume;
		iSoundVolume=arHumans[0].iSndVolume;
	}
	else
	{
		arHumans[0].iFileIndex=-1;
		arHumans[0].ClearNames();
	};
	al_findclose(&Fil);	
	sprintf(FileName,sLocFile);
	
	/*if (bNeedLock)
	{
		sprintf(stLock,"herbs.dat herb.dat");
		sprintf(stReg,stLock);
		sprintf(stSub,"flag.dat");
		bDemo=true;
		Randomizer();
		int iSub=Random(1000);
		int iKey1=Random(1000);
		int iKey2=Random(1000);
		stSub[0]=48+iSub/100;
		stSub[2]=48+(iSub/10)%10;
		stSub[4]=48+iSub%10;
		Pirate1=0;
		
		stReg[1]=107+iKey1/100;
		stReg[2]=97+(iKey1/10)%10;
		stReg[5]=100+iKey1%10;
		stReg[7]=110+iKey2/100;
		stReg[10]=99+(iKey2/10)%10;
		stReg[11]=105+iKey2%10;
		Pirate2=0;

		stSub[3]=48+arComp[0]%10;
		stSub[5]=48+(arComp[0]/10)%10;
		stSub[7]=48+iCompNameLen%10;

		stReg[4]=abs(arComp[1]-2)%64+64;
		stReg[8]=abs(arComp[2]+8)%64+64;
		stReg[12]=abs(arComp[0]-(iSub/10)%10)%64+64;
		stReg[13]=abs((arComp[3]+arComp[1])/2)%64+64;
		Pirate3=0;

		stLock[0]=stReg[1]+iSub/100;
		stLock[1]=80+Random(40);
		stLock[2]=abs(arComp[0]+8)%128;
		stLock[3]=Random(20)+stReg[6];		
		stLock[4]=stReg[2]+iSub%10;
		stLock[5]=70+Random(50);		
		stLock[6]=stReg[5]+(iSub/10)%10;
		stLock[7]=iSub%100;		
		stLock[8]=abs((stReg[7]+arComp[1])/2)%128;
		stLock[9]=abs(arComp[3]-11)%128;
		stLock[10]=stReg[10]-(iSub/10)%10;
		stLock[11]=iKey2;
		stLock[12]=iKey1;		
		stLock[13]=60+Random(60);
		stLock[14]=abs(arComp[2]+7)%128;
		stLock[15]=abs((stReg[11]+arComp[4])/2)%128;
		stLock[16]=70+Random(50);
		stLock[17]=50+Random(70);
		Pirate4=0;

		WorkFile=pack_fopen(FileName,"wp");
		pack_fwrite(&stLock,sizeof stLock,WorkFile);
		pack_fclose(WorkFile);		
		al_findfirst(FileName,&Fil,FA_ALL);		
		long int Time=Fil.time, Wrk;
		Wrk=48+(Time/100)%10;
		stSub[1]=Wrk;
		Wrk=48+Time%10;
		stSub[6]=Wrk;
		Wrk=107+(Time/10)%10;
		stReg[0]=Wrk;
		Wrk=107+(Time/100000)%10;
		stReg[3]=Wrk;
		Wrk=107+(Time/1000)%10;
		stReg[6]=Wrk;
		Wrk=107+(Time/10000)%10;
		stReg[9]=Wrk;
		if (WriteRegistry(sPath,sKey,stSub))
		{
			WorkFile=pack_fopen("p1.wrk","wp");
			pack_fwrite(&stSub,sizeof stSub,WorkFile);
			pack_fclose(WorkFile);
		};
		if (WriteRegistry("Artifactorium","Key",stReg))
		{
			WorkFile=pack_fopen("p2.wrk","wp");
			pack_fwrite(&stReg,sizeof stReg,WorkFile);
			pack_fclose(WorkFile);
		};
		

		al_findclose(&Fil);
	};*/

	/*WorkFile=pack_fopen("key.dat","w");
	char eMail[30];
	long int iKey1;
	sprintf(eMail,"adrageron@inbox.ru");
	iKey1=MakeKey(eMail);
	pack_fwrite(eMail,sizeof eMail,WorkFile);
	pack_fwrite(&iKey1,sizeof iKey1,WorkFile);
	pack_fclose(WorkFile);*/

#ifndef SNOWBALL
	bDemo=true;
	if (al_findfirst("key.dat",&Fil,FA_ALL)==0)
	{
		WorkFile=pack_fopen("key.dat","r");
		char eMail[30];
		long int iKey;
		pack_fread(eMail,sizeof eMail,WorkFile);
		pack_fread(&iKey,sizeof iKey,WorkFile);
		pack_fclose(WorkFile);
		bDemo=false;
//		bDemo+=(iKey%10!=abs(7+eMail[7]+eMail[8]+eMail[9]+eMail[0])%10);
//		iKey/=10;
//		bDemo+=(iKey%10!=abs(1+eMail[3]+eMail[0]-eMail[2]-eMail[10])%10);
//		iKey/=10;
		bDemo+=(iKey%10!=abs(6+eMail[1]+eMail[2]+eMail[6]-eMail[9])%10);
		iKey/=10;
		bDemo+=(iKey%10!=abs(5+eMail[5]+eMail[4]-eMail[3]+eMail[8])%10);
		iKey/=10;
		bDemo+=(iKey%10!=abs(4+eMail[3]+eMail[5]-eMail[0]-eMail[7])%10);
		iKey/=10;
		bDemo+=(iKey%10!=abs(eMail[4]-eMail[6]-eMail[2]*eMail[3]+eMail[10])%10);
		iKey/=10;
		bDemo+=(iKey%10!=abs((eMail[2]-eMail[0])*eMail[3]+eMail[4])%10);
		iKey/=10;
		bDemo+=(iKey%10!=abs(5+eMail[0]+eMail[5]-eMail[1]+eMail[10])%10);
		iKey/=10;
		bDemo+=(iKey%10!=abs(2+eMail[6]*eMail[1]+eMail[3]+eMail[9])%10);
		iKey/=10;
		bDemo+=(iKey%10!=abs(3+eMail[1]+eMail[5]-eMail[3]-eMail[2]+eMail[8])%10);
		iKey/=10;
		bDemo+=(iKey%10!=abs(8+eMail[0]+eMail[2]+eMail[3]-eMail[4]+eMail[7])%10);
	}
	else
	{
		if (InstallYuPlay())
		{
			if (iYuPlay==2)
			{
				set_mouse_sprite(bmpMousePointer[1]);
				YuPlayBuyGame(false);
			};
		};
	};
	al_findclose(&Fil);
	
#endif	

//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"member found: %s %d, %d",arSkills[9].sName,arSkills[9].arHeroUpg[3][1][0],arSkills[9].arHeroUpg[3][1][1]);
//	readkey();

//	textprintf_ex(screen,Font1,0,TypeLine*20,makecol(220,220,20),-1,"%s",(pFirstSpell+10)->sText);
//	readkey();

	return 1;
};

//void Keygen()
//{	
//	
//	PACKFILE *WorkFile;
//	char stLock[18], stKey[18];
//	WorkFile=pack_fopen("eador.lok","rp");
//	pack_fread(&stLock,sizeof stLock,WorkFile);
//	pack_fclose(WorkFile);
//	CurGame.iSeed=stLock[2];
//	sprintf(stKey,"music.dat unit.var");
//	stKey[0]=abs(5+stLock[13]+stLock[17]-stLock[6])%128;
//	stKey[1]=abs(11+stLock[0]+stLock[1]+stLock[8])%128;
//	stKey[2]=abs(44+stLock[14]-stLock[2]+stLock[9])%128;
//	stKey[3]=abs(17+(stLock[8]+stLock[10])/2+stLock[7])%128;
//	stKey[4]=abs(2+stLock[4]*stLock[5])%128;
//	stKey[5]=abs(stLock[9]+stLock[6]/3-13)%128;
//	stKey[6]=abs(stLock[14]-stLock[15]+stLock[17]/2)%128;
//	stKey[7]=abs(stLock[8]+stLock[13]+stLock[6]-stLock[5]-stLock[2])%128;
//	stKey[8]=abs(77+stLock[1]-stLock[4])%128;
//	stKey[9]=abs(3+stLock[0]-stLock[17]+stLock[7])%128;
//	stKey[10]=abs(12+stLock[10]*stLock[0]/10)%128;
//	stKey[11]=abs(stLock[5]+stLock[8]-stLock[2]-4)%128;
//	stKey[12]=abs(stLock[9]+stLock[2]-stLock[13])%128;
//	stKey[13]=abs((stLock[10]+stLock[0]+stLock[15])/3-8)%128;
//	stKey[14]=Random(128,stLock[0],stLock[1],stLock[7],stLock[8]);
//	stKey[15]=1+Random(122,stLock[2],stLock[4],stLock[9],stLock[10]);
//	stKey[16]=21+Random(100,stLock[5],stLock[6],stLock[13],stLock[14]);
//	stKey[17]=6+Random(110,stLock[15],stLock[17]);
//	WorkFile=pack_fopen("eador.key","wp");
//	pack_fwrite(&stKey,sizeof stKey,WorkFile);
//	pack_fclose(WorkFile);
//};
