#ifndef USED_UNIT
#define USED_UNIT

#include "Interface.h"

//Параметры текущей игры
class dcGame
{
public:
	unsigned int iSeed;
	int iCurrentTurn,			//Счетчик ходов
		iGameType,				//Тип игры 1-обычная, 2-кампания, 3-хот-сит, 4-сетевая		
		ResBuy;					//Изменение цены на ресурс при его покупке (в %)
	int ResPrices[10];			//Отклонение цен на ресурсы от базовах в %
	int iMaxPlayers,			//Количество игроков на карте
		iActivePlayer;			//Игрок, ход которого был прерван
	int arPlayerSet[17];		//Список игроков, установленных при старте игры
	int arRaceQuest[MAX_RACE];	//Квесты, выдаваемые разными расами
	bool bDiplomacy,			//Флаг дипломатии
		 bAutoSave,				//Флаг сохранённой игры
		 bFog;					//Флаг тумана войны
	clMap Map;					//Настройки карты
};

//************************************************
//******* Провинция ******************************
//************************************************

class dcProvince
{
public:
	char *sName;			//Название
	int iNumber,			//Порядковый номер
		x,y,				//Координаты центра
		ipTerrain,			//Тип местности
		ipType,				//Тип провинции
		ipPlayer,			//Кому принадлежит
		iConquered,			//Флаг захвата (0 незахвачена, 1 военный, 2 мирный, 3 перезахвачена)
		iSize,				//Размер
		iGoldInc,			//Базовый доход в золоте
		iGemInc,			//Базовый доход в кристаллах
		iPopulation,		//Население
		ipRace,				//Раса
		iExplore,			//Исследование
		iMaxBuildings,		//Количество слотов под постройки
		iDefender,			//Установленный страж
		iFort,				//Состояние укреплений
		ipEvent,			//Событие, вызываемое после захвата провинции нейтралами
		PicX, PicY,			//Координаты вывода картинки провинции
		arUnrest [MAX_PLAYERS],				//Счетчик недовольства
		arAttitude [MAX_PLAYERS][2],		//Отношение к правителю (величина, счетчик ходов)
		arNeighbour[MAX_NEIGHBOUR];			//Соседние провинции
	bool Coastal,			//Прибрежная
		bWarning;			//Флаг предупреждения о перенаселении
	long Work;				//Служебная переменная
	dcHero * pGuard;		//Охранник
	dcHero * pGarrison;		//Гарнизон
	int arBuilding [MAX_PROVINCE_BLD+1];	//Постройки 
	int arBldPlace [MAX_PROVINCE_BLD+1][2];	//Расположение построек на карте
	int arSite [MAX_PROVINCE_SITE][6];	//Особые места (0 индекс, 1 положение на шкале исследования, 2 уровень(№ магазина для магазинов, индекс для квестов), 3 квест, 4 тип охранника, 5 данные для рандомных квестов (-1 если это особый сайт для правила мира))
	dcHero * arSiteGuard [MAX_PROVINCE_SITE];	//Охрана особых мест
	BITMAP *Pic, *TownPic, *BuildPic[MAX_PROVINCE_BLD];		//Картинки закрывающих город деревьев
	G_Object *Obj;
	
	dcProvince();
	~dcProvince();
	void PlaceGuard(int Guardian, int Karma);		//Установить охранника
	void PlaceDefender(int Defender);				//Установить стража
	void DismissDefender();							//Уволить стража
	void PlaceSiteGuard(int Guardian, int Site, int Karma);	//Установить охранника особого места
	int PlaceSite(int Index, int Level, int Quest, int Hidden, int Guard);	//Добавить в провинцию особое место (возвращает индекс сайта или -1 если невозможно) 
																			//Hidden== -1 на открытой территории, 1 на закрытой, 0 по умолчанию 
																			//Guard<0 не устанавливать охрану, Guard>0 - устанавливать, Guard==0 - по умолчанию
	void DismissSiteGuard(int Site);//Уволить охранника особого места
	int GetFreeSpace(int Hidden);	//Возвращает свободную точку на шкале исследований (на неисследованной территории если Hidden>0, на исследованной если Hidden<0) или 0 если таких нет	
	int GetLevel();					//Возвращает уровень провинции
	int GetResource(int Status);	//Возвращает находящийся в провинции ресурс. Status (-1 - не найденный, 0 - охраняемый, 1 - доступный)
	int GetGoldIncome();			//Вычисляет доход золота с провинции
	int GetGemIncome();				//Вычисляет доход кристаллов с провинции
	int GetPort();					//Возвращает бонус порта
	int GetStable();				//Возвращает бонус конюшни
	int GetHeal();					//Возвращает бонус к восстановлению жизни войска
	int GetGrow();					//Возвращает прирост населения за ход
	int GetMood();					//Возвращает настроение жителей
	int GetWall();					//Возвращает максимальную прочность укреплений провинции
	int GetUnrestSlowdown();		//Возвращает параметр замедления недовольства
	int GetEventPoss(int ipEvent);	//Возвращает вероятность происхождения события в провинции
	bool CanBuild(int Building);	//Можно ли строить данную постройку
	bool CanHire(int Defender);		//Можно ли нанимать данного стража
	bool CanCast(int Ritual, int Player);		//Можно ли проводить данный ритуал
	int CheckBldAbil(int Index);				//Возвращает значение указанного свойства внешних построек в провинции
	int GetUnitSkill(int Index);				//Возвращает суммарное значение указанного умения у юнитов гарнизона и проезжих героев
	int GetSiteParam1(int Index);				//Возвращает суммарное значение Param1 для сайтов со свойством Index
	int GetSiteParam2(int Index);				//Возвращает суммарное значение Param2 для сайтов со свойством Index
	int GetSiteParam2(int Index, int Param1);	//Возвращает суммарное значение Param2 для сайтов со свойством Index с параметром Param1
	void ChangeAttitude(int Player, int Change);//Изменяет отношенин жителей к игроку Player на Change
	int AttackDefender(dcHero *Attacker, int DX, int DefCount);		//Диалог при атаке стража игроком
	int UnderAttack(dcHero *Attacker, int DX);	//Сражение в провинции игрока или AI
	int AttackSiege(dcHero *Attacker, int DX);	//Атака осаждающего отряда в провинции
	void StaticEvent(int Index, int Player);	//В провинции происходит статичное событие с индексом Index
	void ConstructBuilding(int Build, bool bRedraw, bool Pay);	//Возвести внешнюю постройку в провинции
	void RazeBuilding(int Slot, bool Redraw);	//Снести внешнюю постройку в указанном слоте
	int GetSiegeIndex();						//Возвращает -1, если провинция свободна, иначе индекс осаждающего игрока
	void RepairWalls();							//Восстанавливает стены укреплений провинции
	void Conquer(int Player, dcHero *HR);		//Захват провинции героем HR игрока Player
	int GetRitualBonus(int Param, int Index);	//Возвращает сумму изменения параметра №Param эффекта Index для всех ритуалов, действующих в провинции
	void DrawPic(bool bRedraw);					//Создать картинку провинции
	int GetCorruption();
	void MakeNeutral();
	void ShowBar(int Y);
	int GetQuestUnit(int Rank);					//Возвращает индекс наиболее распространённых юнитов данного ранга в окрестностях провинции
}; 

//************************************************
//******* Юнит ***********************************
//************************************************

class dcUnit
{
private:
	int ipType;				//Указатель на тип юнита

public:
		
	int //ipType,				//Указатель на тип юнита
		iLife,				//Текущая жизнь
		iMorale,			//Текущий модификатор бевого духа
		iExp,				//Опыт	
		iLevel,				//Уровень опыта
		x, y,				//Координаты на поле боя (для расстановки войск)
//		iSlot,				//Занимаемый слот в интерфейсе
		arUpg[UNIT_LEVEL_CAP],			//Повышения
		arMedal[3];			//Награды
	dcHero *pHero;			//Ссылка на героя, нужна только для самогО юнита-героя
		
	dcUnit();
	dcUnit(int U_Type, int Expirience);
	~dcUnit();
	clUnitType * pType();
	int	GetUnitType();
	void SetUnitType(int NewType);
	int CheckUpg(int Index);	//Проверка повышений и наград
	int CheckAbility(int Index);//Проверка врожденных способностей

//Получение параметров в зависимости от повышений и наград
	int GetLife();
	int GetAttack();
	int GetCounterAttack();
	int GetDefence();
	int GetRangedDefence();
	int GetResist();
	int GetSpeed();	
	int GetRangedAttack();	
	int GetShootingRange();
	int GetAmmo();
	int GetStamina();
	int GetMorale();
	int GetExp();
	int GetGoldPayment();
	int GetGemPayment();
	int GetRank();
	
	int GetExpMod();
	int GetGoldPrice();
	int GetGemPrice();
	int GetRace();	
	int	GetUnitKind();
	int	GetUnitClass();
	int GetKarma();
	int CountLevel();
	int GetHealBonus();
	int ExpForLevel(int Level);
	int LevelUp(int Level, int Exclude);	//Получение увеличиваемого параметра при повышении на уровень
	void MakeLevel(int Upg);
	void Dismiss();
}; 

class dcQuest
{
public:
	int Index,		//Номер прототипа квеста
		ipProvince,	//Номер провинции квестодателя
		ipSite,		//Номер сайта квестодателя
		iTime;		//Счетчик ходов
	//	iGoal,		//Цель (для динамически генерируемых квестов)
	//	iQuantity;	//Количество (для динамически генерируемых квестов)
	bool bComplete;	//Флаг выполненного квеста
	int arProgress[MAX_QUEST_TASK];	//Счетчик выполнения	

	dcQuest();
	~dcQuest();
};
//************************************************
//******* Герой **********************************
//************************************************

class dcOldHero
{
public:	 
	int	ipName,				//Имя
		ipPortrait,			//Индекс портрета
		ipPlayer,			//Кому принадлежит
		iExp,				//Опыт (тип охранника для охраны)
		iLevel,				//Уровень
		NiB,				//Количество ходов вне боя (для AI)
		ipClass1,			//Первичный класс
		ipClass2,			//Вторичный класс
		iHealth,			//Здоровье
		iLeader,			//Командование
		iMagic,				//Магия
		iLifeBonus,			//Прибавка к жизни
		iMoveBonus,			//Прибавка к запасу хода
		ipProvince,			//Индекс провинции, где находится		
		iTask,				//Индекс задания (0-нету, 1-идти в iTarget, 2-исследовать, 3-грабить, 4-войти в iTarget, 5-атаковать). Инициатива охраны.
		iTarget,			//Цель задания
		iAITask,			//Индекс задания ИИ (если задание идтив провинцию) (0-атаковать, 1 - взять войско, 2-заклятья, 3-предметы, 4-исследовать провинцию, 100+ исследовать сайт %100, -5-снятие осады)
		iCamper,			//Индекс атакуемой провинции для учёта вражеских героев
		iCampTurn,			//Счётчик попыток атаки вражеской провинции
		iDead;				//Отрицательна если живой, иначе указывает расстояние от тела до замка
	
	BITMAP *Pic;			//Портрет
	BITMAP *smPic;			//Маленький портрет
	G_Object *ObjN, *ObjA;				//Ссылка на объект на карте (обычный и активный)
	stString * pstName;
	dcProvince *pProvince;				//Где находится
	dcUnit * pSquad [MAX_SQUAD];		//Отряд героя (первый юнит в отряде - это сам герой)
	dcQuest arQuest[HERO_QUEST_LIMIT];	//Массив заданий
	int arSpell [SPELLBOOK_SIZE+1];		//Запомненные заклинания
	int apSkill [HERO_SKILL_LIMIT][2];	//Изученные умения (индекс и уровень)
	int arInventory [INVENTORY_SIZE+1][2]; //Инвентарь героя (индекс, прочность)
	int arGarrison[4];					//Количество слотов под юнитов для эмуляции гарнизона
	int wavDeath;
	
};

class dcHero : public dcOldHero
{
public:
	int ipMoveProv,			//Индекс провинции для отступления
	ipEvent;			//Индекс произошедшего с героем событияне

	dcHero();	
	dcHero(int Player, int Class);
	~dcHero();
	void ConvertOld(dcOldHero OldHero);
	int CheckUnitBonus(int Index);
	int CheckHeroBonus(int Index);
	int CheckSkill(int Index);
	int GetSpellSlot(int Level);
	int GetUnitSlot(int Level);		//Возвращает количество доступных слотов данного ранга
	int GetFreeUnitSlotNum(int Level);	//Возвращает количество свободных слотов данного ранга и выше
	int GetFreeUnitSlot(int Level); //Возвращает свободный слот указанного уровня или выше, 0 если слотов нет
	bool CanUseUnitSlot (int Slot);	//Возвращает истину, если параметры позволяют герою использовать данный слот
	int GetSkillSlot();
	int GetClassName();
	int CountLevel();	
	int ExpForLevel(int Level);
	bool CanUse(int Item);
	void PlaceUnit(dcUnit * Unit, int Slot);//Добавить юнита в отряд
	int LevelUp();
	bool DamageArmor(int Dmg);			//Нанести повреждения на броню, возвращает истину при поломке
	bool DamageWeapon(int Dmg);			//Нанести повреждения на оружие, возвращает истину при поломке
	int DropLoot(int * Dur, int Skill);
	bool TakeLoot(int Item, int Dur);
	int GetMobility();
	int GetSiege();
	int GetSquadMorale(int Num);		//Возвращает изменение боевого духа для воина отряда с индексом Num
	int GetSquadHealBonus();			//Возвращает бонус лекарей в отряде
	bool Death(int Win);				//Герой погиб в бою, если Win=1 - бой выигран (возвращает истину, если герой был похоронен)
	void Encounter(int Index, dcHero *NPC, dcProvince *Prov, int Site, int Quest, int Param1, int Param2); //С героем происходит приключение Index в провинции Prov, на сайте Site
	void CallEncounter(int Index, dcHero *NPC, dcProvince *Prov, int Site, int Quest, int Param1, int Param2); //Интерфейс для вызова приключений
	void CheckQuestDone(int Quest);		//Проверяет, выполнен ли квест и выдает сообщение о выполнении	
//	void CheckQuestsFail();				//Проверяет, какие квесты стали недоступными и выдает соотв. сообщения
	void AbandonQuest (int Quest);		//Отменяет квест №Quest из списка взятых героем квестов
//	int CheckQuestStatus (int QuestID);	//Возвращает состояние квеста с данным индексом 0 - не получен, 1 - получен, 2 - выполнен
	int MakeDescription(char *txtSquad);//Создаёт строку, содержащую типы юнитов, входящих в отряд, возвращает общее количество юнитов
	void MakeScoutingText();			//Заносит в stScouting информацию об отряде
	void HealUnits (int HealRate);		//Изменить здоровье воинов на HealRate% от максимума. Если HealRate<0 - нанести повреждения в процентах от текущего здоровья
	void ChangeUnitsMorale (int Morale);//Изменить боевой дух воинов
	void Arena ();						//Герой входит на арену
	int GetDefenderGoldPayment();		//Возвращает плату в золоте отряду-охраннику с текущим составом юнитов
	int GetDefenderGemPayment();		//Возвращает плату в кристаллах отряду-охраннику с текущим составом юнитов
	void ChangeLife(int Power, int Type, int Targets);	//Лечит или наносит повреждения герою и отряду (при повреждениях учитывается сопротивление)
														//Power - количество жизни, Type - если 1 - в % от максимума,  Targets - если 1 - действует только на живых
	void ChangeMorale(int Power, int Karma);		//Изменить боевой дух отряда на Power, Если Karma>0 значение вычтется у злых воинов, а добрым прибавится, если<0 - наоборот, на нейтралов не подействует.
	void RestoreUnits(bool bHomeLand, int ProvBonus, bool bRaise);				//Восстанавливает жизнь и боевой дух юнитов в отряде (если bHomeLand false - отряд на вражеской территории, если bRaise true - воскрешает павших защитников) ProvBonus - бонус провинции к лечению
	void ChangeUnitType();
};


//**************************************
//*** Режим обучения *******************
//**************************************
class dcTutorial
{
public:
	bool	Mode,		//Включен или нет 
			Map;		//Первая карта
	int		Move,		//Режим обучения движению
			Trade,		//Режим обучения торговле
			Hire,		//Режим обучения найму
			Library,	//Режим обучения запоминанию заклинаний
			Site,		//Режим входа в особое место
			Task,		//Счётчик заданий учителя
			Turn,		//Счётчик ходов
			Mercenary,
			Garrison,
			Scroll,
			Egg,			
			PrvBuild,	
			PrvGuard,
			Castles,
			RitualHero,	//Ритуал над героем
			RitualProv,	//Ритуал в провинции
			RitualCast,	//Вызов ритуала
			Repair,
			Death,
			Battle,
			Build,
			Hero,
			Population,
			Resource,
			Groups,
			SpellRank,
			UnitRank,
			Stamina,	
			Ranged,
			Heal,
			Morale,
			Ammo,
			Orc,			
			Fortify,
			Magic,
			Bonus,
			Alchemy,
			Diplomacy,
			Corrupt,
			Quest,
			CastleDefense,
			NoDip,
			DipTrade,
			Exchange,
			War,
			Peace,
			Union,
			Broken,
			SquadMorale,
			LostShard,
			DefendWorld,
			DefendCastle,
			UnitPrice,
			Add1,
			Add2;
	int		buttEnd, buttCastle, buttStat, buttDiplo, buttNext, buttBuild, buttGuard, buttEnter, buttRitual, buttRitHero, buttShop, buttLibrary, buttGarrison, buttPillage, buttAttack, buttExplore;
	dcTutorial();
	void Init();
	void Run();
};

//**************************************
//*** Игрок в кампании *****************
//**************************************
class dcCampPlayer
{
public:
	int iEnergy,						//Энергия
		iEnergyBonus,					//Дополнительный доход энергии
		iEnergyPenalty,					//Расход энергии
		iEnergyLose,					//Потери дохода энергии с мира (в %)
		iEnergyDrain,					//Потери дохода энергии с мира (в чистом виде)
		iSizeBonus,						//Дополнительный размер
		ipProfile,						//Номер профайла
		iKarma,							//Карма
		iScore,							//Набранные очки
		iTreaty,						//Индекс соглашения с игроком
		iTreaty1,						//-"-
		X, Y,							//Координаты осколка
		arAttitude[MAX_PLAYERS],		//Отношения к другим игрокам
		arBld[1000];					//Массив доступных для строительства построек
	long int iMapScore,					//Набранные в войнах очки
		iQuestScore,					//Очки за выполнение квестов
		iDipScore,						//Очки за дипломатию
		iKillScore;						//Очки за побеждённых оппонентов
	bool bFirstMeet,					//Флаг, встречен игрок или нет
		bShardMeet,						//Состоялась ли встреча на осколке
		bInGame;						//Флаг, уничтожен игрок или нет
	BITMAP *ShardPic,					//Картинка осколка игрока
		*ShardMask;						//Картинка выделенного осколка
	void Clear();						//Очистка значений переменных
	void AddBuild();					//Добавляет доступную постройку в массив
	int GetAttitude();					//Возвращает отношение к игроку	
};

//**************************************
//*** Осколок мира *********************
//**************************************
class dcShard
{
public:	
	char sName[30];						//Название
	int x, y,							//Координаты
		Num,							//Индекс
		iLevel,							//Уровень осколка
		iSeed,							//Случайная величина, определяющя карту осколка
		Income,							//Доход энергии
		Terrain,						//Тип местности
		ipPlayer,						//Кому принадлежит
		arBonus	[MAX_SHARD_BONUS][2],	//Бонусы (тип, величина)
		arEvent	[MAX_SHARD_EVENT],		//Особые события, 0 - особое правило мира
		arBuilding [MAX_SHARD_BUILD];	//Доступ к постройкам
	clMap Map;							//Настройки карты
	dcShard *pNext;						//Ссылка на следующий осколок
	BITMAP *Pic, *Select;				//Картинка
	UI_Mask Mask;						//Маска

	dcShard();
	dcShard(dcShard * Next, int Terra, int Bld, int Lvl);
	~dcShard();	
	void FillBonus();		//Заполнение массива бонусов для нового осколка
};

//**************************************
//*** Игровая кампания *****************
//**************************************
class dcCampaign
{
public:		
	int iCurrentTurn,					//Текущий ход
		iActiveShard,					//Осколок, за который идёт сражение (если отрицат. - номер игрока, с которым битва)
		iDifficulty,					//Уровень сложности
		iMaxShards,						//Количество сгенерированных осколков
		iTimeBonus,						//Дополнительное время
		iReserv,						//0 - Запрет атаки на этом ходу
		arJournal[1000],				//Журнал событий, 980-1000 - награды, 978 - лимит ходов, 979 - минимальная сложность
		arDialog[2000],					//Состояние диалогов кампании
		arItems[20],					//Особые предметы (0 - ключи Истины, 1 - детали Разрушителя (у игрока), 2 - у Уль-дагана,  3 - убийство демонов, 4 - отдано осколков Белезу)
		Events[50],						//Очередь событий
		arQuest[MAX_CAMP_QUEST][4];		//Таблица состояния квестов
										//0 Индекс квеста
										//1 Состояние квеста (0 - пассивен, 1 взят, 2 выполнен, 3 провален, -1 - не найдена провинция для генерации)
										//2 Индекс квестовой провинции
										//3 Индекс квестового сайта
	bool bDefender;						//Если истинна - игрок защищается (при отрицат. iActiveShard)
	dcCampPlayer arCampPlayer[MAX_PLAYERS+16];		//Массив профилей игроков (16+ для эмуляции локальных лордов)
	dcCampaign();
	~dcCampaign();	
	void Clear();
	dcShard * GetActiveShard();
};

//**************************************
//*** Особые правила мира **************
//**************************************
class dcWorldRules
{
public:
	static const int MAX_TRIGGER=100;	//Количество триггеров
	int iMode;							//Индекс особого правила
										//0 Обычные правила	
										//1 Ржавый мир

	int arTrigger[MAX_TRIGGER];			//Таблица триггеров
	
	void Init(int Index);
	dcWorldRules(int Index);
	~dcWorldRules();
};


//************************************************
//******* Магазин ********************************
//************************************************
class dcShop
{
public:
	int Type;						//Тип предметов, которыми торгует
	int PriceModifier[MAX_PLAYERS]; //Изменения цен для разных игроков
	int Items[SHOP_SIZE][2];		//Массив вещей на продажу (индекс и количество, 0 - неограниченный запас)
	dcShop();
	~dcShop();
};

//************************************************
//******* Динамическое событие *******************
//************************************************

class dcEvent
{
public:
	int Index,		//Индекс (тип события)
		ipDialog,	//Ссылка на диалог
//		Owner,		//Игрок, которому принадлежит это событие 
		Player1,	
		Player2,
		iParam1;	//Номер сайта для исследования
	char sMessage [1000];
	dcHero *Hero1, *Hero2;
	dcProvince *Prov1, *Prov2;
	dcEvent *pNext, *pPrev;	//Ссылка на следующее в списке событие
	dcEvent *pTree;	//Ссылка на событие, вызываемое этим событием	
	dcEvent** Owner;

	dcEvent(int Indx, dcEvent** Own, int Dialog, int Play1, int Play2, dcHero *Her1, dcHero *Her2, dcProvince *Pr1, dcProvince *Pr2, dcEvent *Next);
	~dcEvent();
	void ChangeOwner (dcEvent** NewOwn);
	void Exclude();
	int Call(int DX); //Вызывает событие, DX-смещение окна диалога
};


//************************************************
//******* Последствия ритуалов *******************
//************************************************
class dcRitual
{
public:
	int Index,		//Индекс (тип ритуала)
		ipPlayer,	//Игрок, проведший ритуал
		ipProvince,	//Номер провинции
		Timer,		//Счетчик оставшихся ходов	
		iEvent;		//Произошло ли событие, генерируемое ритуалом
		
	dcRitual *pNext; //Ссылка на следующий и предыдущий ритуал	

	dcRitual();
	dcRitual(int Indx, int Player, int Prov);
	~dcRitual();	
};

//************************************************
//******* Торговля между игроками ****************
//************************************************
class dcTrade
{
public:
	int Gold,		//Золото
		Gem,		//Кристаллы
		Stuff[16];	//Предметы обмена (ресурсы, провинции, предметы)
	bool bTrade;
	dcTrade();
	void Clear();
	bool IsLegal(int Player, int Type); //Проверка, может ли игрок совершить сделку
	bool IsFree();	//Проверка, является ли торговля пустой
};

//************************************************
//******* Сообщение одного игрока другому ********
//************************************************
class dcMessage
{
public:
	int Type,		//Тип договора (0-нету, 1-только политический, 2 - торговля, 3 - обмен, 4 - отмена торговли, 5 - сообщение/ответ, 
					//6 - предупреждение о нарушении договора, 7 - жребий или война, 8 - сдаться или война, 9 - враг сдался)
		Treaty;				//Предлагаемый договор (0 - нету, 1 - война, 2 - мир, 3 - союз, 4 - отмена союза)
	dcTrade Trade, Trade1;	//Сделка Trade - отдаём, Trade1 - получаем
	int arDurability[8];	//Прочность отдаваемых в сделке вещей 
	char Text[500];			//Текст послания
	bool bDeal;
	dcMessage();
	void Clear();
};


//************************************************
//******* Отношения между игроками ***************
//************************************************
class dcPolitics
{
public:
	int Treaty,		//Политический договор (0-нет контакта, 1-война, 2-мир, 3-союз)
		Pass,		//Право прохода
		Attitude,	//Отношение к игроку
		Answer,		//Ответ на послание игрока
		CanKill,	//Насколько оправдано нападение на игрока
		Power;		//Память о былых заслугах
	dcTrade	Trade;	//Торговая сделка 
	dcMessage Message;	//Написанное послание
	int arTalk[8][3];	//Тип послания, ответ, таймер (1-война, 2-мир, 3-союз, 4-обмен, 5-торговля, 6-выкуп, 7-подарок)
	int arTraitor[4];	//Флаги предательства (0 атака без объявления войны, 1 союзника, 2 при уплаченной дани, 3 при открытых границах)
	dcPolitics();
	void Clear();
};


//************************************************
//******* Игрок **********************************
//************************************************

class dcPlayer
{
public:
	int Control,	//Кем управляется (0-отключен, 1 - игрок, 2 - AI-властитель, 3 - AI-местный правитель)
		Num,		//Порядковый номер в массиве игроков
		ipProfile,	//Номер профиля игрока
		iDifficulty,	//Уровень сложности
		iCorrupt,		//Изменение коррупции
		GoldInc,
		GemInc,
		iMember,		//Выбранный элемент на карте
		iCondition,		//Состояние панели на карте
		iKarma,
		iGoldAdd,		//Дополнительный доход золота в кампании
		iGemAdd,		//Дополнительный доход кристаллов в кампании
		iUnion,			//Раса, с которой заключен союз
		iMercTime,		//Время с появления нового наёмника
		iCalcCorrupt,	//Вычисленная коррупция за прощлый ход в % (для учёта в событиях)
		iCanInnerBld,	//Количество внутренних построек, которые можно возвести в этот ход
		iCanOuterBld,	//Количество вешних построек, которые можно возвести в этот ход
		iCanHire,		//Количество стражей, которых можно нанять в этот ход
		iCanCast,		//Количество ритуалов, которые можно провести в этот ход
		iPlanned,		//Запланированная постройка
		iColor,			//Цвет флага и герба
		ipProvince,		//Родовая провинция
		iWall,			//Прочность стен родового замка
		iTower,			//Количество башен в родовом замке
		iTowerHealth,	//Прочность башен в родовом замке
		iTowerShoot,	//Сила выстрела башен в родовом замке
		iWin,			//Количество побед
		iLose,			//Количество поражений
		iQuest,			//Выполнено заданий
		iDragonHunt,	//Разграблено драконьих обиталищ
		iExplore,		//Исследовано земель
		iTimeShift,		//Количество откатов во времени
		arRes[10],		//Снижение цен на ресурсы в кампании (в %)
		arKills[17],	//Повержено противников
		arLoot[3],		//Найдено редких предметов (редкость 4, 5, 6)
		AIUnion[2];		//Состояние союза с другой расой для ИИ (номер расы, счётчик ходов до союза), AIUnion[0] для игрока - флаг нажатия кнопки отряда из окна диалога
	long int 
		Gold,
		Gem,
		iPillage;		//Награблено золота
	bool bBldFilter,	//Флаг фильтра внешних построек
		bDefFilter,		//Флаг фильтра стражей
		bRitFilter,		//Флаг фильтра ритуалов
		bInGame,		//В игре (ложный если не участвует или выбыл)
		bEnter,			//Флаг первого входа в замок
		bSet,			//Флаг того, что собран редкий сет				
		bNoLoss;		//Нет потерь в боях

	bool arBld[MAX_BLD];//Массив построенных в замке зданий
	int arRecruit[MAX_RECRUIT]; //Массив доступных для найма в замке юнитов, каждая ячейка соответствует слоту на экране найма
	int arLibrary[LIBRARY_SIZE]; //Массив доступных для изучения заклинаний, каждая ячейка соответствует слоту на экране библиотеки
	int arTreasury[TREASURY_SIZE][2];	//Массив предметов в сокровищнице (индекс и прочность)
	int arGroup[MAX_GROUP][MAX_GROUP_BUILD];	//Порядок застройки групп зданий в родовом замке
	int arCamper[MAX_CAMPERS];			//Массив провинций с вражескими героями
	dcShop Shop;						//Доступные для покупки предметы
	dcHero *arHero[MAX_HERO];			//Массив героев
	dcEvent *pEvent;					//Список событий
	dcPolitics Politic[MAX_PLAYERS];	//Отношения с другими игроками
	UI_Mask arHeroMask[MAX_HERO];		//Массив масок для фигурок героев
	int * pOuterBld;					//Ссылка на массив доступных внешних построек
	int * pDefender;					//Ссылка на массив доступных стражей
	int * pRitual;						//Ссылка на массив доступных ритуалов
	int * pRitualCD;					//Ссылка на массив готовности ритуалов
	int * pFog;							//Ссылка на массив тумана войны
	int * pStory, * pPlace;				//Ссылка на массив истории событий и провинций, в которых они происходят
	dcPlayer();
	~dcPlayer();
	void Init(bool InGame, int Color);
	void Clear();						//Уничтожение данных и динамических переменных
	int GetResource(int Index);

	int GetResGoldPrice(int Index);		//Возвращает цены на
	int GetResGemPrice(int Index);		//	ресурс Index.

	int GetResGoldPriceStat(int Index);	//	То же, но без учёта
	int GetResGemPriceStat(int Index);	//наличия ресурса у игрока.

	int GetItemGoldPrice(int Item, dcShop * Shop);
	int GetItemGemPrice(int Item, dcShop * Shop);
	int GetItemGoldRepPrice(int Item, int Dur, dcShop * Shop);
	int GetItemGemRepPrice(int Item, int Dur, dcShop * Shop);
	int GetUnitGoldPrice(int Unit, int Prov);
	int GetUnitGemPrice(int Unit, int Prov);
	int GetOutBldGoldPrice(int Building);
	int GetOutBldGemPrice(int Building);
	int GetInBldGoldPrice(int Building);
	int GetInBldGemPrice(int Building);
	int GetDefenderGoldPrice(int Defender);
	int GetDefenderGemPrice(int Defender);
	int GetRitualGoldPrice(int Ritual);
	int GetRitualGemPrice(int Ritual);
	int GetHeroGoldPrice(int Class);
	int GetHeroGemPrice(int Class);
	int GetResurrectGoldPrice(dcHero * Hero);		
	int GetResurrectGemPrice(dcHero * Hero);
	int RecruitHero(int Class);		//Нанять героя указанного класса. Возвращает номер нанятого героя в списке, -1 если если список героев полон.
	bool CanBuild(int Building);	//Возвращает истину, если достигнуты условия для постройки данного внутреннего здания
	int GetBldParam1(int Ability);	//Возвращает сумму параметров 1 для построек с указанным свойством
	int GetBldParam2(int Ability);	//Возвращает сумму параметров 2 для построек с указанным свойством
	int GetBldParam2(int Ability, int Param1); //Возвращает сумму параметров 2 для построек с указанным свойством и параметром 1
	bool CompareBldParam1(int Ability, int Index); //Возвращает истину, если есть постройка с указанными свойством и параметром 1	
	void AddMercenary();			//Добавляет наемника в гарнизон
	void Build(int Choosen);		//Строит в замке постройку
	void SellItem(int Index, int Dur);	//Продажа предмета
	void CastRitual (int Index, int Prov, int Hero);	//Провести ритуал Index в провинции Prov (если Hero>-1 - на указанном герое)
	BITMAP* GetAvatar();
	BITMAP* GetSmallAvatar();
	char* GetName();
	void MessageEntry(int Sender, int Answer);	//Сделать запись о сообщении
	void AcceptDeal(int Sender);	//Принять дипломатическое предложение
	void CancelDeal(int Sender);	//Отказаться от дипломатического предложения
	void ReturnHero(int Num);		//Возвращает героя в соседнюю дружественную провинцию или в родовой замок
	int GetWars();					//Возвращает количество врагов
	void DeclareWar(int Enemy);		//Вступить в войну с игроком Enemy
	int GetAttitude(int Victim);	//Возвращает отношение к игроку Victim данным игроком
	int GetUnionCount();			//Возвращает количество союзников у данного игрока
	bool CheckMessage(int Victim);	//Возвращает истину, если послание к игроку Victim приемлемо
	void ShowHerb(int x, int y, BITMAP *Target);	//Выводит в указанный битмап герб игрока
	void ShowFlag(int x, int y, BITMAP *Target);
	void MakeFlags();	
	bool IsBestHero(int Hero); //Возвращает истину, если этот герой наибольшего уровня у данного игрока
	void CheckCampQuest(int ipProv, int ipSite);	//В режиме кампании проверяет выполнение квеста
};

int GetUnitSlotLevel(int Slot);
bool GroupIsFull(int Player, int Group);	//Возвращает истину, если группа зданий полностью отстроена
bool IsRestricted(int Build, int Player);	//Возвращает истину, если постройку уже не удастся возвести
void DrawUnit(BITMAP *Pic, dcUnit *Unit, int x, int y, int Moraler, BITMAP *bmpExp, BITMAP *bmpHealth, dcHero *Hero);
void DrawHerb(int Col1, int Col2, int Sign, int x, int y, BITMAP *Target);
void DrawFlag(int Col1, int Col2, int x, int y, BITMAP *Target);
void ScreenHeroHero (dcHero * Hero1, dcHero * Hero2);
bool ScreenHero (dcHero * Hero, dcUnit * Ally, bool CanDelete); //Возвращает ложь, если герой был удален
void ScreenHeroGarrison (dcHero * Hero, dcHero * Garrison, bool UseLocalTab);
void ScreenHeroLibrary (dcHero * Hero);
void ScreenHeroShop (dcHero * Hero, dcShop * Shop);
void ScreenGarrison (dcHero * Garrison, bool CanHire);
int ScreenProvList(int Player);
//void ScreenCastleBuild (int Player);
//bool ScreenHeroRecruit (int Player);
int CastleMain(int Player);
void ScreenMenu();
int ScreenStatistics (int Player);
bool ScreenEndGame (int Player, bool bWin, bool bShow, bool bWrite);
void ScreenDiplomacy(int Player);
void WindowUnitLvlUp (dcUnit * Unit, int Player);
void WindowUnitReward (dcUnit * Unit, int Reward);
void WindowHeroLvlUp (dcHero * Hero);
void WindowUnitInfo (dcUnit * Unit, int Morale);
void WindowUnitTypeInfo(int Unit);
void WindowResourceInfo (int Res);
void WindowSpellInfo (int Spell);
void WindowSkillInfo (int Skill, int Level);
void WindowInfo (char * sName, char * sText, BITMAP *Pic);
void WindowAbilityInfo (int Ability, int Power);
void WindowRaceInfo (int Race);
void WindowEffectInfo (clUnitUpgrade *Ability, clSpell *Spell, int Time, int Power);
void WindowMedalInfo (int Medal);
void WindowItemInfo (int Item, int Dur);
int WindowInnBldInfo(int Build, int Player);
void WindowDefenderInfo (int Defender, int GoldPayment, int GemPayment);
void WindowOuterBldInfo (int Bld);
void WindowRitualInfo (int Rit);
void WindowTreasure (int Level, int Player, dcHero * Hero, int Site, int Index);
void WindowTakeLoot (dcHero * Hero);
int WindowMessage(int Player, int Sender);
bool WindowEnter (char * Str, int Phrase, int Pic);
void WindowHerb (clHumanProfile *Hum);
void WindowAlchemy (int Player);
bool WindowMenu (int Player);
bool WindowSettings(int ipPlayer);
bool WindowProfile (int Index, bool bList);
void WindowHighScore();
void WindowCampaignVictory();
void ProfileOperator (int Index);
void WindowMapSettings(clMap *Map, int *BRD);
int DifficultyWindow();
bool WindowChangeWeapon(dcHero *Hero);

void MakeItemHint (int Item, int Dur, dcHero *Hero, int PriceText, int Gold, int Gem);
void MakeAbilityHint (int Ability);
void MakeStringHint (char *Str);
void MakeInnBldHint(int Build, int Player);
void MakeTextHint (int ID);
void MakeTextHint (int ID, char *Str);
void MakeSiteHint(dcProvince *PR, int Site);
//void MakeStringHint (int Phrase); перенесено в Interface.h

const char* GetAttitudeName(int Att);
const char* GetRulerNick(int Karma);

extern dcPlayer arPlayer[MAX_PLAYERS]; //Массив игроков
extern dcProvince * pFirstProvince;
extern dcRitual * pRitual;
extern dcShard * pFirstShard;
extern dcWorldRules WorldRules;
//extern dcCampPlayer arCampPlayer[MAX_PLAYERS]; //Массив игроков
extern dcCampaign Campaign;
extern dcTutorial Tutorial;
int GetUnitExp(int ipUnit, int Level);	//Возвращает количество опыта, необходимое для получения юнитом типа ipUnit уровня Level
int InitBattle(dcProvince *Prov1, dcProvince *Prov2, dcHero *Hero1, dcHero *Hero2, int Site, bool bShow);
//Site - номер сайта для генерации поля боя, bShow - если ложь - битва не показывается
int SimulateBattle(dcProvince *Prov1, dcProvince *Prov2, dcHero *Hero1, dcHero *Hero2, int Site);
void Rebel(dcProvince *PR, int DX);	//Генерирует событие восстания в провинции PR
void MakeFog(int Player);	//Вычисляет туман войны
void ClearMoveMap();	//Очистка карты движения перед новым поиском
int MoveMap (int Prov, int Weight, int Moves, int Port, int addMoves);
int CountUnitLevel(int Exp, int ExpMod);	//Вычисление уровня юнита
int CompareStrength(dcHero *Hero1, dcHero *Hero2); //Сравнение сил отрядов. Возвращает 100 если силы равны, если больше - первый отряд сильнее
int CompareStrength1(dcHero *Hero1, dcHero *Hero2);
void AIHeroLvlUp(dcHero * Hero);
void TakeLootAI(dcHero *HR);
int GetCurPlayers();			//Возвращает количество активных игроков
int CheckPlayerCondition(int Player, bool bLose); //Проверяет условия победы или поражения, возвращает 1, если для игрока игра окончена, -1 при возврате в прошлое
void EventServer(int Player);	//Обрабатывает и сортирует события, связанные с перемещением героев
dcEvent* EnterEnemyLand(int Player1, int Player2, dcHero *Hero1, dcHero *Hero2, dcProvince *Prov1, dcProvince *Prov2);
BITMAP *MakeObstracles(BITMAP * Pic, int x, int y);
int GetProvinceIndex(int x, int y); //Возвращает номер провинции, в которой находится указанная точка
void ArrangeUnits(dcHero * Hero, int Slot);
int IsParent(int Father, int Son, int Player);	//Возвращает 0 если Son не требует father для постройки, 1 если требует, 2 если требование групповое
int GetBuildingStatus(int Build, int Player);	//Возвращает 0 для недоступной постройки, 1 для доступной (или расположенной на нейтральном осколке) и 2 для уже имеющейся у игрока
bool IsBuildingFree(int Build);					//Возвращает истину, если указанной постройки нет ни на одном осколке
int GetWorldSize(int Player);					//Возвращает размер мира игрока в кампании (в осколках для человека, в постройках (осколки*3) для компа)
void WindowNextTurn (int Player);		//Сообщение о начале хода в игре HotSeat
void MakeDipText(int Player1, int Player2, int Index, int Param);	//Подобрать текст для дипломатического послания
void ScrollSound();
void CoinSound();
void RunCampaignEvent(int Index, int Player, int Param);
void ScreenCampStat();
const char* GetRulerTitle(int Score);
int GetEnergyIncome(); //Возвращает доход энергии с мира игрока
//int ShopRoller(int iShop, int iLevel); //Заполняет таблицу вероятностей для магазина типа iShop предметами уровня не выше iLevel
//iShop: 1, 2, 9 Оружие и щиты, 3 Луки и стрелы, 4 Жезлы и сферы, 5 Знамёна, 6 кожа и пояса, 7 Кольчуги, 8 Латы, 10 Свитки, -1 Одежда, -2 Украшения
void Lose(int Player);
void Victory(int Player);
void FinishCampaign(int Index);
void WinCampaign(int Index);
void WindowCampHighScore();
void ScreenCredits();
int GetWorldSize(int Player);
void ScreenDemo();
int ShowEventMessage(int Player, int Mode, int ipProv, int Dialog, int An1, int An2, int An3, int An4, char *Mess, BITMAP *Pic, bool HotSeat);

#ifdef YUPLAY
int YuPlayInit();
bool InstallYuPlay();
void WindowLottery(int Index);
void YuPlayBuyGame(bool bBuyer);
extern dcUnit *pLottUnit;
void OpenYuPlaySite();
void OpenYuDownload();
void RunYuInstall();
#endif

#endif
