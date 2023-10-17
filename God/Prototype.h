
#ifndef USED_PROTOTYPE
#define USED_PROTOTYPE

#include "Globals.h"

class dcHero;

//Строка
class stString
{
public:
	char str[40];
};

class stColorString : public stString
{
public:
	int iColor;	
};

struct stMessage
{
	char * msg;
};

//Умение или свойство юнита
//struct stAbility
//{
//	char sName[20];	//Название
//	int Effect;		//Тип эффекта (-1 отрицательный, 0 нейтральный, 1 положительный)
//	bool bNumeric;	//Измеет ли свойство численное значение
//};

//Умение или свойство героя
struct stAbilityNum
{
	char sName[30];	//Название
	char * sText;	//Описание
	int Num,		//Индекс
		Effect;		//Тип эффекта (-1 отрицательный, 0 нейтральный, 1 положительный)
	bool bNumeric,	//Имеет ли свойство численное значение
		bPercent;	//Если истинно, значение в %
	BITMAP *Pic;	//Иконка
};

//Особый эффект заклинания или умения юнита
struct stEffect
{
	char sName[30];	//Название
	char * sText;	//Описание
	int ipAbility,	//Индекс свойства
		ipSpell;	//Индекс заклинания
	bool bNumeric;	//Истинно, если в описании присутствует параметр
		
};


//**************************************
//*** Тип ландшафта ********************
//**************************************
class clTerrainType
{
public:
	char sName[21];			//Название
	 
	int iPassibility,		//Проходимость 0 - вода, 1- суша, 2 - гора
		iGoldInc,			//Базовый доход в золоте
		iGemInc,			//Базовый доход в кристаллах
		iPopulationMod,		//Модификатор прироста населения
		iExploreMod,		//Модификатор скорости исследования
		iBF_Pass,			//Основная проходимая территоря боевого поля
		iBF_Impass;			//Основная непроходимая территоря боевого поля

	clTerrainType();
	~clTerrainType();
}; 



//**************************************
//*** Объекты поля боя *****************
//**************************************
class clBF_Object
{
public:
	char sName[21];		//Название	
	int	iMoveCost,		//Ходов на прохождение (0-непроходимая)
		iStamCost,		//Затраты энергии на прохождение
		iCounterAttack,	//Бонус к контратаке
		iDefence,		//Бонус к защите
		iRangedDefence,	//Бонус к защите от выстрела	
		iShootingRange;	//Бонус к дальности выстрела
	BITMAP *bmpTileset[3][10];	//Набор картинок для разных климатов

	clBF_Object();
	~clBF_Object();
}; 


//**************************************
//*** Ресурс ***************************
//**************************************
class clResource
{
public:
	char sName[21];			//Название
	char * sText;			//Описание

	int iRarity,			//Редкость 0 - частый, 1- обычный, 2 - редкий
		iGold,				//Цена в золоте
		iGem;				//Цена в кристаллах		
	BITMAP *Pic, *BigPic, *MedPic;

	clResource();
	~clResource();
}; 


//**************************************
//*** Раса *****************************
//**************************************
class clRace
{
public:
	char sName[21];			//Название
	char * sText;			//Описание
	 
	int ipProvType,			//Индекс родной провинции
		iKarma,				//Карма
		iCorrupt,			//Уровень коррупции
		iGrow,				//Прирост
		iTerra,				//Тип родной местности (нет штрафа к приросту населения)
		iUnrest,			//Непокорность
		iGold,				//Умение добывать золото
		iGem;				//Умение добывать кристаллы
	int arTerrain1[MAX_TERRAIN]; //Подходящая территория (список вероятностей, индекс - номер территории)
	int arEnemy[MAX_ENEMY];		//Расы-враги
	int arQuest[MAX_RACE_QUEST];	//Список индексов заданий для заключения союза
	BITMAP * Pic, *SmallPic;

	clRace();
	~clRace();
}; 


//**************************************
//*** Улучшение юнита ******************
//**************************************
class clUnitUpgrade
{
public:
	char sName[21];						//Название
	char * sText;						//Описание
	int ipNeed [MAX_UPGRADE_NEED];		//Необходимые улучшения
	int arUpg [MAX_UPGRADE_SIZE] [2];	//Максимум MAX_UPGRADE_SIZE улучшений (номер, величина)
	bool bOnlyOne;						//Флаг улучшения, возможного только 1 раз
	BITMAP * Pic;

	clUnitUpgrade();
	~clUnitUpgrade();
};

		
//**************************************
//*** Юнит *****************************
//**************************************

class clUnitType
{
public:
	char sName[21];			//Название
	char * sText;			//Описание
	 
	int iLevel,				//Уровень
		iLife,				//Жизнь
		iAttack,			//Атака
		iCounterAttack,		//Контрудар
		iDefence,			//Защита
		iRangedDefence,		//Защита от выстрела
		iResist,			//Сопротивление
		iSpeed,				//Скорость
		iRangedAttack,		//Сила выстрела
		iShootingRange,		//Дальность выстрела
		iAmmo,				//Количество снарядов
		iStamina,			//Выносливость
		iMorale,			//Боевой дух
		iExp,				//Опыт за убийство
		iExpMod,			//Модификатор опыта (определяет скорость набора опыта юнитом 
							//(более сильные растут медленней))
		iGoldPrice,			//Цена в золоте
		iGemPrice,			//Цена в кристаллах
		iGoldPayment,		//Плата в золоте
		iGemPayment,		//Плата в кристаллах
		iRace,				//Раса юнита
		iUnitKind,			//Тип юнита (гуманоид, механический, нежить, демон)
		iUnitClass,			//Род войск (пехота, кавалерия, техника, монстр)
		iKarma,				//Карма
		iBuilding,			//Необходимая постройка в замке
		iMissile;			//Тип снаряда
	int arRes [MAX_UNIT_RES];			//Необходимые ресурсы
	int arAbility [MAX_UNIT_ABIL];		//Особые умения - ссылки на улучшения
	int arUpg [2] [20] [MAX_UPG_PER_LVL];	//Список повышений  [улучшение/вероятность] [уровень] [количество]
	int arLoot [2] [20] [MAX_LOOT_PER_LVL];	//Список предметов лута  [предмет/вероятность] [уровень] [количество]
	BITMAP	*Pic,						//Картинка юнита
			*Shadow, *FlipShadow,		//Тень юнита
			*Icon;						//Иконка для вывода в интерфейс
	int	wavHit,			//Звук атаки юнита
		wavShoot,			//Звук выстрела юнита
		wavShootHit,		//Звук поражения цели
		wavDeath;			//Звук смерти юнита


	clUnitType();
	~clUnitType();
};


//**************************************
//*** Заклинание ***********************
//**************************************
class clSpell
{
public:
	char sName[21];		//Название
	char * sText;		//Описание
	int	iCost,			//Цена
		iLifeCost,		//Дополнительная плата здоровья
		iStamCost,		//Расход выносливости на вызов заклинания
		iLevel,			//Уровень
		iItemLevel,		//Ценность свитка с этим заклинанием
		iPowerMod,		//Модификатор силы мага
		iDurationMod,	//Модификатор концентрации
		iResistMod,		//Модификатор сопротивления
		iTarget,		//Тип целеуказания (0 - нет цели, 1 - живой юнит, 2 - труп юнита, 3 - тайл, 4 - труп юнита на свободном от живых тайле, 5 - свободный от юнитов тайл)
		iArea,			//Область действия (0 - сам маг, 1 - выбранная цель, 2 - все поле боя)
		iRadius,		//Радиус действия
		iKarma,			//Карма
		ipBuilding;		//Городская постройка
	bool bNegative,		//Негативный эффект
		bOnEnemy,		//Действует на врагов
		bOnAlly,		//Действует на своих
		bSacrifice,		//Требует жертву
		bResistPower,	//Сопротивление ослабляет силу
		bResistDuration,//Сопротивление ослабляет длительность
		bDefencePower,	//Защита ослабляет силу
		bRestoreOnly,	//Изменяемый параметр не может превысить максимум
		bCumulative;	//Кумулятивный эффект
	int arUnitKind [MAX_UNIT_KIND];		//Неподверженные заклинанию типы юнитов
	int arAntiEffect [MAX_ANTI_EFFECT]; //Препятствующие заклинанию эффекты
	int arEffect [3] [MAX_SPELL_EFFECT];//Эффекты (Идентификатор, сила, длительность)Длительность заклинания (-1 мгновенная, 0 постоянная)
	BITMAP *	Pic;	//Иконка заклинания
	int	wavCast;		//Звук вызова заклятья
	clSpell();
	~clSpell();
};


//**************************************
//*** Профессия ************************
//**************************************
class clSkill
{
public:
	char sName[21];			//Название
	int iClass;				//К какому базовому классу относится
	int arHeroUpg [5][MAX_SKILL_UPG][2];	//Улучшения самого героя (Уровень, № улучшения, индекс и величина)
	int arSquadUpg [5][MAX_SKILL_UPG][2];	//Улучшения юнитов в отряде (Уровень, № улучшения, индекс и величина)
	BITMAP * Pic;			//Иконка профессии
	clSkill();
	~clSkill();
};


//**************************************
//*** Предмет **************************
//**************************************

class clItem
{
public:
	char sName[31];		//Название
	int ipSlot,			//оружие, броня, плащ и т.п.
		ipClass,		//какое умение требует для использования
		iDurability,	//прочность
		iGoldCost,		//цена в золоте
		iGemCost,		//Цена в кристаллах 
		iShopLevel,		//уровень магазина, в котором данный предмет может появиться на продажу
		iRarity,		//Редкость
		ipBuilding;		//Cсылка на городскую постройку, дающую возможность купить этот предмет
	int arEffect [MAX_ITEM_EFFECT][3];	//Свойства предмета (индекс, сила, область действия 0 - герой, 1 - отряд)
	int arRes [MAX_ITEM_RES];			//Требуемые ресурсы
	BITMAP *Pic;

	clItem();
	~clItem();
};

//**************************************
//*** Охранник *************************
//**************************************

class clGuard
{
public:
	int Num, NumAdd,		//Минимальное количество воинов и случайная прибавка
		Quick, QuickAdd,	//Минимальная инициатива и случайная прибавка
		LootPoss,			//Вероятность выпадения лута (в%)
		LootNum,			//Максимальное количество предметоав в луте
		Power,				//Сила охранника
		arUnit[MAX_GUARD_UNIT][4],	//Список юнитов (индекс, уровень, мин. количество, вероятность)
		arLoot[MAX_GUARD_LOOT][2];		//Список предметов лута (индекс, вероятность)
};



//**************************************
//*** Тип охранника ********************
//**************************************

class clGuardType
{
public:
	char sName[21];					//Название
	int iLevel,						//Базовый уровень (сила) охраны
		iKarma;						//Карма охранника. ВЫЧИТАЕТСЯ из кармы убившего его игрока
	int arGuard[MAX_GUARD_LEVEL];	//Охранники по уровням
};

//**************************************
//*** Страж ****************************
//**************************************

class clDefender
{
public:
	char sName[21];		//Название
	char * sText;		//Описание
	int iPower,			//Сила (учитывается ИИ)
		ipPicture,		//Иллюстрация
		iGoldCost,		//Цена в золоте
		iGemCost,		//Цена в кристаллах
		iGoldPayment,	//Плата в золоте
		iGemPayment,	//Плата в кристаллах
		iBuilding,		//Cсылка на городскую постройку, дающую возможность нанимать этого стража
		iInitiative,	//Инициатива
		iKarma,			//Влияние на карму
		iLootPoss,		//Вероятность выпадения лута (в%)
		iLootNum;		//Максимальное количество предметоав в луте
	bool bNoDismiss;	//Нельзя увольнять
	bool arTerrain [MAX_TERRAIN];		//Типы допустимой местности
	int arRes[MAX_DEFENDER_RES],		//Ресурсы
		arUnit[MAX_GUARD_UNIT][3],		//Список юнитов (индекс, уровень, количество)
		arLoot[MAX_GUARD_LOOT][2],		//Список предметов лута (индекс, вероятность)
		arAbility[MAX_DEFENDER_ABILITY][3];	//Список особых эффектов (индекс, параметры)
	BITMAP * Pic;

	int CheckAbility(int Index);
	clDefender();
	~clDefender();

//Редкость (int) //определяет, насколько редко встречаются свитки вызова данного стража
};

//**************************************
//*** Тип провинции ********************
//**************************************

class clProvinceType
{
public:

	char sName[21];		//Название
	int ipRace,			//Раса
		iCoast,			//0 – все равно, 1 – только прибрежная, 2 – только не прибрежная, 3 - только на реке
		iSites,			//Среднее количество особых мест
		iGold,			//Модификатор дохода золота
		iGem,			//Модификатор дохода кристаллов
		iDifficult,		//Уровень сложности, чем выше, тем дальше от родовых замков будет расположена
		iPopulation,	//Начальное население
		iLevelPop,		//Влияние уровня провинции на население
		iDiplUnrest,	//Влияние на настроение при мирном захвате
		iConqUnrest,	//Влияние на настроение при военном захвате
		iDiplKarma,		//Влияние на карму при мирном захвате
		iConqKarma,		//Влияние на карму при военном захвате
		iExplored,		//Начальные разведанные земли
		iLevelExplored;	//Влияние уровня провинции на разведанные земли
	int arTerrain[MAX_TERRAIN];			//Типы допустимой местности (вероятность появления провинции на этой территории)
	int arGuard[MAX_PROVINCE_GUARD][4];	//Классы допустимых охранников (тип, вероятность, номер выдаваемого диалога при встрече, номер приключения при переговорах)
	int arEvent[MAX_PROVINCE_EVENT][2];	//События (индекс, вероятность)
};

//**************************************
//*** Уровни провинций *****************
//**************************************

class clProvinceLevel
{
public:
	char sName[21];		//Название
	int iPopulation,	//Максимум населения
		iIncome,		//Изменение дохода (в %)
		iExplored;		//Необходимое количество исследованных земель
};

//**************************************
//*** Названия провинций ***************
//**************************************

class clProvinceName
{
public:
	char sName[21];		//Название
	int iRarity,		//Редкость
		iCoast;			//0 – все равно, 1 – только прибрежная, 2 – только не прибрежная, 3 - только на реке
	int arTerrain[MAX_TERRAIN+1];	//Типы допустимой местности (0 - пустая, 8 - вода)
	int arType[MAX_NAME_TYPE];		//Типы допустимых провинций

	clProvinceName();
	~clProvinceName();
	bool CheckTerrain(int Terr);
	bool CheckType(int Type);	
};


//**************************************
//*** Внешняя постройка ****************
//**************************************

class clOuterBuilding
{
public:
	char sName[21];		//Название
	char * sText;		//Описание
	int ipUpgrade,		//Ссылка на предшествующую постройку (для построек-улучшений)
		ipBuilding,		//Ссылка на городскую постройку, дающую возможность строить это здание
		iGoldCost,		//Цена в золоте
		iGemCost,		//Цена в кристаллах
		iGoldAdd,		//Изменение дохода золота (абсолютное)
		iGemAdd,		//Изменение дохода кристаллов (абсолютное)
		iGoldPerc,		//Изменение дохода золота (в процентах)
		iGemPerc,		//Изменение дохода кристаллов (в процентах)
		iUnrest,		//Изменение настроения населения
		iAttitude,		//Изменение отношения населения
		iGrow,			//Изменение прироста населения
		iKarma,			//Влияние на карму
		iHeal,			//Влияние на здоровье войск
		iCoastal,		//Прибрежная //0 – все равно, 1 – только прибрежная, 2 – только не прибрежная, 3 - только на реке
		iPort,			//Порт (значение определяет максимальную дальность плавания)
						//Укрепления:
		iWall,			//Прочность стены
		iTowerHealth,	//Прочность башни
		iTowerShoot,	//Сила выстрела башни
		iTowerType;		//Тип башни (1 - 4 простая (значение равно количеству башен), 10 - магическая, стреляет раз в 2 хода)
	bool bLibrary,		//Библиотека
		bShop;			//Магазин
	bool arTerrain [MAX_TERRAIN];			//Типы допустимой местности
	int arRes [MAX_BUILDING_RES];			//Необходимые для строительства ресурсы
	int arResBonus [MAX_RESOURCE_BONUS][2];	//Изменение дохода с ресурса (ресурс, модификатор)
	int arAbility [MAX_BUILDING_ABILITY][3];//Особые свойства постройки (индекс, величина, доп. величина)
	BITMAP	* Pic,			//Иконка
			* bmpBld,		//Вид здания
			* bmpShadow;	//Тень



//int arGarrison [4];	//Гарнизон (каждое число соответствует количеству доступных слотов для юнитов соотв. уровня)
//Редкость (int) //определяет, насколько редко встречаются чертежи данной постройки
//Влияние на опыт войск (int)
};

//**************************************
//*** Особое место (сайт) **************
//**************************************

class clSite
{
public:
	char sName[21];		//Название
	char * sText,		//Описание
		*sFreeText;		//Описание сайта без охраны
	int iMinLevel,		//Минимальный уровень сайта
		iMaxLevel,		//Максимальный уровень сайта
		iGuarded,		//Вероятность того, что сайт охраняется
		iPlace,			//0 - не генерится, 1 - генерится, 2 - только на неисследованной территории
		ipEvent,		//Индекс генерируемого при входе события
		ipDialog,		//Индекс диалога при входе
		ipDialog2,		//Индекс диалога при обнаружении сайта без охраны (или для внутреннего диалога для сайтов с квестами)
		iTreasure,		//Отклонение уровня сокровищ от базового		
		iCoastal;		//Прибрежная (0 – все равно, 1 – только прибрежная, 2 – только не прибрежная, 3 - только на реке)
	bool bWorkHide,		//Эффекты в силе, если сайт не найден
		bWorkGuard,		//Эффекты в силе, если охрана жива
		bWorkFree,		//Эффекты в силе, если охрана уничтожена
		bCanEnter,		//Герой может посещать сайт
		bSelfDestroy;	//Уничтожается после использования
	int arTerrain [MAX_TERRAIN];			//Вероятности генерации на разной местности
	int arGuard[MAX_SITE_GUARD][2];			//Типы допустимых охранников (индекс, вероятность)
	int arAbility [MAX_SITE_ABILITY][3];	//Особые свойства сайта (индекс, величина, доп. величина)
	BITMAP * Pic;

	clSite();
	~clSite();
	int CheckAbility(int Index);

};

//**************************************
//*** Ритуал ***************************
//**************************************

class clRitual
{
public:
	char sName[21];		//Название
	char * sText;		//Описание
	int iGoldCost,		//Цена в золоте
		iGemCost,		//Цена в кристаллах		
		iKarma,			//Карма
		iTime,			//Время действия
		iCooldown,		//Перерыв в использовании
		ipEvent,		//Событие, генерируемое ритуалом (для уведомления владельца провинции на следующий ход)
		iItemLevel,		//Уровень свитка с ритуалом
		iItemRarity;	//Редкость свитка с ритуалом
		
	bool bOnAlly,		//Можно вызывать в своей провинции
		bOnEnemy;		//Можно вызывать во вражеской провинции		
	int arRes [MAX_RITUAL_RES];				//Необходимые для вызова ресурсы		
	int arEffect [MAX_RITUAL_EFFECT][3];	//Эффекты ритуала (индекс, величина, доп. величина)
		BITMAP * Pic;
		int Sound;

		clRitual();
		~clRitual();
		int GetAbility(int Index);
};


//**************************************
//*** Совет героя о силе врага *********
//**************************************

class clHeroAdvice
{
public:
	char sText[100];
	int Min, Max;	//Промежуток значений, при котором выдаётся текст
};

//**************************************
//*** Диалог ***************************
//**************************************

class clDialog
{
public:	
	char * sText;		//Текст
	int iWindow;		//Тип окна
		
	char arAnswer [MAX_ANSWER][301];	//Ответы
		BITMAP * Pic;

		clDialog();
//		~clDialog();
		int Show(int A1, int A2, int A3, int A4, BITMAP * Picture, char *sNewText);
};

//**************************************
//*** Вопрос ***************************
//**************************************

class clQuestion
{
public:	
	char * sText;		//Текст
	int iQuester,		//Индекс вопрошающего
		iTrigger,		//Номер флага, при котором вопрос виден
		iEvent;			//Событие, на которое ведёт вопрос
		
//		~clDialog();
	
};

//**************************************
//*** Внутренняя постройка *************
//**************************************

class clInnerBuilding
{
public:
	char sName[21];		//Название
	char * sText;		//Описание
	int iGoldCost,		//Цена в золоте
		iGemCost,		//Цена в кристаллах
		iGroup,			//Группа построек, к которой принадлежит
		iSlot,			//Слот на экране строительства
		iUpg,			//Заменяет собой постройку
		iHidden,		//Невидима, пока не будут выполнены условия
		iLevel;			//Уровень осколка, на котором появляется (0 - доступна сразу)
	int arBuildings[MAX_BUILDING_NEED];		//Массив необходимых предшественников
	int arRes [MAX_BUILDING_RES];			//Необходимые для строительства ресурсы		
	int arAbility [MAX_BUILDING_ABILITY][3];//Особые свойства постройки (индекс, величина, доп. величина)
	BITMAP * Pic;

	clInnerBuilding();
	~clInnerBuilding();
};

//****************************************
//* Графический объект постройки в замке *
//****************************************

class clCastle
{
public:	
	int iLayer,			//Слой для вывода
		iOpacity,		//Прозрачность
		x, y,			//Координаты			
		iGroup,			//К какой группе принадлежит
		Yplace;			//Точка вставки
	int arBuild[MAX_CASTLE_BLD],		//Постройки, открывающие эту картинку (0 - всегда, -1 при первом посещении)
		arAntiBuild[MAX_CASTLE_BLD];	//Постройки, закрывающие эту картинку
	BITMAP *Pic;						//Изображение

	clCastle();
	~clCastle();
};


//**************************************
//*** Тексты посланий ******************
//**************************************

class clDipText
{
public:
	char *sText;		//Текст
	int Index,			//Индекс типа сообщения
		Min,			//Минимально допустимые отношения
		Max,			//Максимально допустимые отношения
		Name,			//Флаг использования имени правителя в послании
		Param;			//Параметр для выбора сообщения (преимущества сторон, выгода и т.д.)
	clDipText * pNext;	//Следующий объект
};


//**************************************
//*** Настройки генератора карт ********
//**************************************
class clMap
{
public:
	int Players,
		Difficulty,
		X,
		Y,
		Type,
		Water,
		Hill,
		Forest,
		Swamp,
		Isles,
		Dissolve,
		Neutrals,
		Resource,
		Site,
		Population;
	clMap();
	~clMap();
	void SetDefault();	//Выставить значения по умолчанию
};

//**************************************
//*** Характеристики игрока ИИ *********
//**************************************

class clPlayerProfile
{
public:
	char sName[21];		//Имя
	char sLand[21];		//Название родовой провинции
	char * sText;		//Описание
	int iKarma,			//Карма
		iStrategy,		//Предпочтительный тип развития
		iMagic,			//Предпочтительный вид магии
		iTargeting,		//Предпочтительные цели
		iDiplomacy,		//Общительность
		iAggro,			//Агрессивность
		iGreed,			//Жадность
		iLoyal,			//Верность
		iRandom,		//Непредсказуемость
		iColor1,		//Цвет герба 1
		iColor2,		//Цвет герба 2		
		iSign,			//Гербовый знак
		Y,				//Расположение мира игрока в кампании
					//*Индексы событий в кампании*		
		ipEventFirstMeet,	//Первая встреча
		ipEventMeet,		//Диалог
		ipEventDefeat,		//Уничтожение игрока
		ipEventAttack,		//Атака игрока-человека
		iLandType,			//Тип земель в родном мире
		iLandPoss,			//Бонус к вероятности
		iProvType,			//Тип провинций в родном мире
		iProvPoss;			//Бонус к вероятности
	int arAtt[MAX_PLAYERS];				//Таблица отношений с другими компьютерными игроками
	int arBuild[MAX_PLAYER_BLD];		//Список предпочтительных построек
	int arEnterDlg[MAX_PLAYER_DIALOG];	//Список активируемых диалогов
	int arExitDlg[MAX_PLAYER_DIALOG];	//Список деактивируемых диалогов
	clDipText * pDipText; //Тексты посланий
	BITMAP * Pic;
	clPlayerProfile();
	~clPlayerProfile();	
};

//**************************************
//*** Характеристики игрока человека ***
//**************************************

class clHumanProfile
{
public:
	char sName[21];		//Имя
	char sLand[21];		//Название родовой провинции
	int iFileIndex,		//Индекс профайла игрока
		iDifficulty,	//Сложность
		iColor1,		//Цвет герба 1
		iColor2,		//Цвет герба 2		
		iSign,			//Гербовый знак
		ipAvatar,		//Индекс портрета
		iVictory,		//Победы
		iLost,			//Поражения
		iSingle,		//Пройдено одиночных карт
		iCampaign,		//Пройдено кампаний
		iHint,			//Номер текущего хинта
		iSndVolume,		//Громкость эффектов
		iMusVolume,		//Громкость музыки
		iScrollSpeed,	//Скорость скроллинга
		iBattleSpeed,	//Скорость движения в бою
		iMapBorders,	//Тип границ генерируемой карты
		iEnterData,		//Игрок редактировал свой профиль (0 - нет, 3 - да, промежуточные варианты для кампании)
		iAutoLvlUp,		//Автоматическиё апгрейды юнитов до указанного ранга
		iAutoCombat;	//Автобой 0 - никогда, 1 - с нейтралами, 2 - всегда
	bool	bFinishTutorial,	//Пройдена первая обучающая миссия
			bWarGrid,	//Сетка в бою
			bWarBar,	//Инфо панели в бою
			bWarEndTurn,//Автопередача хода в бою
			bHerbOnMap,	//Выводить гербы на карту			
			bAutosave,	//Автосохранение
			bDiplomacy,	//Дипломатия в одиночной игре
			bFog,		//Закрытая карта в одиночной игре
			bTutorialMode,	//Режим обучения
			arMap[10],	//Значки на карте (0: Гербы; 1: Настроение; 2: Недовольство; 3: Страж; 4: Исследование; 5: Задания; 6: Арены; 7: Лавки; 8: Логова; 9: Храмы;)
			arMess[10];	//Сообщения вкл/выкл (0 - Обучение; 1- Предел населения; 2- Бездельники; 3- Продажа воинов; 4- Удаление; 5- Увольнение стражей; 6- Снос построек;)
	clMap Map;			//Настройки генерируемой карты
			
	clHumanProfile();
	~clHumanProfile();
	void Save();
	void ClearNames();
};

//**************************************
//*** Кусок тумана войны ***************
//**************************************

class clFog
{
public:
	int Num,			//Номер провинции
		x, y, x1, y1;	//Координаты на карте
	BITMAP * Pic;
};

//**************************************
//*** Набор предметов ******************
//**************************************

class clItemSet
{
public:
	char sName[21];		//Название
//	char * sText;		//Описание
	int iSize;			//Количество предметов в наборе
	int arItems [MAX_SET_ITEMS];	//Список пермметов
	int arBonusQ [MAX_SET_BONUS];	//Количество предметов, небоходимых для бонусов
	int arBonus [MAX_SET_BONUS][MAX_SET_ABILITY][3];	//Номер бонуса, номер свойства, индекс\сила\область

	clItemSet();
	~clItemSet();
};


//**************************************
//*** Награда юнита ********************
//**************************************
class clMedal
{
public:
	char sName[31];		//Название
	char *sText;		//Описание
	int iGoldSpent,		//Повышение платы в золоте
		iGemSpent,		//Повышение платы в кристаллах		
		iRarity;		//Редкость
		
	int arEffect [MAX_MEDAL_EFFECT][2];	//Свойства предмета (индекс, сила)	
	BITMAP *Pic, *SmallPic;

	clMedal();
	~clMedal();
};

//**************************************
//*** Статичные события в провинциях ***
//**************************************
class clEvent
{
public:
	int iType,			//Тип события (хорошее-плохое)
		iPossibility,	//Вероятность		
		ipDialog,		//Индекс диалога
		iDlgParam1,		//Индекс первого параметра для диалога (0 - нет, 1 - имя провинции, 2 - величина из эффекта 1)
		iDlgParam2,		//Индекс второго параметра для диалога
		ipAttacker,		//Тип атакующего отряда
		ipEventWin,		//Событие при победе
		ipEventLose,	//Событие при поражении
		ipEventDraw,	//Событие при ничье
		ipProvType,		//Тип провинции, устанавливаемой при захвате нейтралами
		iKarma;			//Изменение кармы

	int arEvent[MAX_ANSWER];	//Индексы событий в зависимости от выбранного ответа
	int arEffect[MAX_EVENT_EFFECT][4];	//Массив эффектов (индекс, сила, параметры)

	clEvent();
	~clEvent();
	long int CalcEffect(int Index, int Mult, long int Value);	//Вычисляет силу эффекта Index с множителем Mult
	int GetParam1(int Index, int Power);	//Возвращает параметр для указанных Index и Power
	int GetParam2(int Index, int Power);	//--"--
};

//**************************************
//*** Приключения героев ***************
//**************************************
class clEncounter
{
public:
	int iType,			//Тип события (хорошее-плохое)
		iPossibility,	//Вероятность		
		ipDialog,		//Индекс диалога
		iDlgParam1,		//Индекс первого параметра для диалога (0 - нет, 1 - имя героя, 2 - величина из эффекта 1)
		iDlgParam2,		//Индекс второго параметра для диалога
		iParam1,		//Параметры, передаваемые из предыдущих приключений
		iParam2,		//		--"--
		ipAttacker,		//Тип атакующего отряда
		ipEncWin,		//Событие при победе
		ipEncLose,		//Событие при поражении
		ipEncDraw,		//Событие при ничье		
		iKarma;			//Изменение кармы

	int arEncounter[MAX_ANSWER];		//Индексы приключений в зависимости от выбранного ответа
	int arEffect[MAX_EVENT_EFFECT][4];	//Массив эффектов (индекс, сила, параметры)

	clEncounter();
	~clEncounter();
	int CalcEffect(int Index, int Mult, int Param1, int Param2, dcHero *Hero);	//Вычисляет силу эффекта Index с множителем Mult и полученными из предшественников параметрами
	int GetPower(int Index);				//Возвращает Power для указанного эффекта (без расчета изменений)
	int GetParam1(int Index, int Power);	//Возвращает параметр для указанных Index и Power
	int GetParam2(int Index, int Power);	//--"--
};

//**************************************
//*** Задания героев *******************
//**************************************
class clQuest
{
public:
	char *sText;	//Запись в дневнике
	char sName[30];	//Название
	int iGiver,	//Квестодатель (0 для расового, № типа сайта для сайта, отрицат. для кампании)
		iType,	//Тип задания:
				//1 - Обследовать сайт Index (вероятность Param) Quantity раз
				//2 - Добыть Quantity предметов Index (вероятность выпадения из монстра Target и сайта Place = Param)
				//3 - Убить Quantity юнитов Index, 
				//4 - Должно произойти событие Index, 
				//5 - Найти сайт Index в провинции Target, 
				//6 - Награбить Quantity золота с провинции Target, населенной расой Param		
				//7 - Добыть случайный предмет (Index: 0 - обычный, 1 - свиток)
				//8 - Убить Quantity+Random(Param) монстров ранга Target, Index - сложность
		iPossibility,	//Вероятность
		ipEncInit,		//Индекс события получения задания
		ipEncWait,		//Индекс события ожидания задания
		ipEncFound,		//Индекс события обнаружения необходимого предмета и т.п.
		ipEncNotFound,	//Индекс события неудачного поиска необходимого предмета и т.п.
		ipEncDone,		//Индекс события выполнения условий задания
		ipEncSuccess,	//Индекс события сдачи квеста
		ipEncFailGiver,	//Индекс события провала квеста из-за квестодателя
		ipEncFailTarget,//Индекс события провала квеста из-за цели квеста
		ipEncFailTime,	//Индекс события провала квеста из-за времени
		iTime,			//Ходов на выполнение
		iExp,			//Опыт за выполнение задания		
		Index	[MAX_QUEST_TASK],		
		Param	[MAX_QUEST_TASK],		
		Quantity[MAX_QUEST_TASK],
		Target	[MAX_QUEST_TASK],		
		Place	[MAX_QUEST_TASK];
	
	int arAbility[MAX_QUEST_ABILITY][4];	//Массив особых свойств (индекс, сила, параметры)

	clQuest();
	~clQuest();	
};


//**************************************
//*** Объект глобальной карты **********
//**************************************
class clMapObject
{
public:
	int iType,						//Тип (при генерации провинции выбирается один из случайных типов, и используются только подходящие объекты)
		iLayer,						//Слой для вывода
		dx, dy,						//Смещение от левого верхнего угла до точки вставки
		iSite,						//Сайт, необходимый для появления в провинции
		arLand[MAX_TERRAIN+2],		//Вероятность появления на определённых типах территории
		arType[MAX_MAPOBJ_TYPE];	//Типы провинций, в которых может появиться
	bool
		bLandmark;					//Уникальность
	BITMAP *arPic[MAX_MAPOBJ_PIC];	//Массив картинок

	clMapObject();
	~clMapObject();
};

//**************************************
//*** Объект Landmark ******************
//**************************************
class clLandmark
{
public:
	int iType,						//Тип 
		iLayer,						//Слой для вывода
		dx, dy,						//Смещение от левого верхнего угла до точки вставки
		arLand[MAX_TERRAIN+2],		//Вероятность появления на определённых типах территории
		arType[MAX_MAPOBJ_TYPE],	//Типы провинций, в которых может появиться
		iSite;						//Сайт, необходимый для появления в провинции
	
	BITMAP *Pic,					//Картинка
		*Shadow;					//Тень

	clLandmark();
	~clLandmark();
};


//**************************************
//* Параметры осколков разных размеров *
//**************************************
struct stShardSize
{
	char sName[20];	//Название
	int x,			//Размер по x
		y,			//Размер по y
		Players;	//Максимум игроков
};

//**************************************
//*** Бонусы осколков ******************
//**************************************
class clShardBonus
{
public:
	char sName[30];					//Название
	int iType,						//Тип: 0 - численная величина, 1 ссылка на предмет, 2 - юнита, 3 - постройку, 4 - ресурс
		iRarity,					//Редкость
		iPrice,						//Цена покупки бонуса в энергии (за 1 единицу или за 1 золотой цены объекта)
		arLevel[MAX_SHARD_LEVEL][MAX_SHARD_BONUS_TYPE][2];		//Максимум и минимум на осколках разных уровней, перечисление допустимых объектов и их редкость для не числовых бонусов
		
	BITMAP *Pic;					//Картинка

	clShardBonus();
	~clShardBonus();
};

//**************************************
//***** Запись в таблице рекордов ******
//**************************************
class clHighScore
{
public:
	char sName[100];
	int Score,
		ipPortrait,
		Sign,
		Col1,
		Col2,
		arMedal[8];
};

//**************************************
//***** Запись в журнале ***************
//**************************************
class clJournal
{
public:
	char sName[100];
	char *sText;
	int Index;
};

//**************************************
//***** Запись в титрах ****************
//**************************************
class clCredits
{
public:
	int Number;
	stString pTitle;	
	stString pName[20];	
};

//**************************************
//*** Особые правила мира **************
//**************************************
class clWorldRules
{
public:
	char sName[30];								//Название
	int iShardSize,								//Минимальный размер осколка, ля которого применимо правило
		ipStartEvent,							//События в начале игры
		iParam1,								//Особые модификаторы
		iParam2,
		ipSpecialEvent,							//Обязательное событие
		iSpecialEventTurn,						//Ход для обязательного события
		ipSiteEvent,							//Cобытие при захвате особого сайта
		arRandomEvents[MAX_WORLD_EVENTS][2],	//Изменение вероятностей особых событий (индекс, доп. вероятность)
		arProvTypes[MAX_WORLD_PROVTYPES][2],	//Изменение вероятностей генерации провинций
		arSites[MAX_WORLD_SITES][2];			//Изменение вероятностей генерации сайтов
	char *sText;								//Текст с описанием

	clWorldRules();
	~clWorldRules();
};

//**************************************
//*** Уровень сложности ****************
//**************************************
class clDifficulty
{
public:
	char sName[21];		//Название
//	char * sText;		//Описание
	int iGold,			//Стартовое золото
		iGem,			//Стартовые кристаллы
		iGoldAdd,		//Доход золота
		iGemAdd,		//Доход кристаллов
		iPopulation,	//Стартовое население
		iExp,			//Модификатор получаемого опыта
		iLoot,			//Модификатор найденного золота и кристаллов
		iNeutralLife,	//Модификатор жизней нейтралов
		iCorruptStart,	//Количество провинций, при котором начинается коррупция
		iCorruptCap,	//Максимальные потери от коррупции
		iScore;			//Модификатор набранных очков	
	bool bShowAdvice;	//Флаг подсказки героя

//	clDifficulty();
//	~clDifficulty();
};


//**************************************
//***** Лотерейный билет ***************
//**************************************
class clTicket
{
public:
	int Number;
	stColorString Str[10];
};

//void Keygen();
//void WriteKeys();
void FileCopy(char *Name1, char *Name2);
void Sleeper(int Delay);
void OpenWeb();
int InitRegistry(char * CompName);
bool WriteRegistry(char * sDir, char * sKey, char * sVal);
bool ReadRegistry(char * sDir, char * sKey, char * sVal);
int LoadConfig();
int LoadPhrases();
int Loader();			//Загружает прототипы (возвращает 0 если возникла ошибка)
void LoadBFPics();
void SaveRecords();		//Сохранение таблицы рекордов
void SaveCampRecords();	//Сохранение таблицы рекордов кампании
void FillTab();			//Заполнение таблицы рекордов базовыми значениями
extern bool bLog;
void WriteLog(char * Txt);
void FileLog(char * Txt);
void DFileLog(char * Txt);
void StopLog(char * Txt, int Value);
void BeginLog();
void EndLog();
void RunHourGlass(int iPhrase, int DY);	//Запускает анимацию песочных часов с фразой iPhrase
void AnimHourglass(); //Анимирует песочные часы
long int StrToInt(char *Str);
void IntToStr(int Zifer, char *Str);
extern clHighScore arRecords[10];
extern clHighScore arCampRecords[10];
extern clTicket arTicketType[MAX_TICKET_TYPE];
long int MakeKey(char * eMail);

//*******************************************************
//** Указатели на первые элементы массивов прототипов ***
//*******************************************************

extern stString			*pFirstPhrase;
extern stString			*pName[4];
extern stString			*pFirstUnitClass;
extern stString			*pFirstUnitKind;
extern stString			*pFirstItemType;
extern stMessage		*pFirstMessage;
extern stMessage		*pFirstTutText;
extern stMessage		*pFirstHint;
//extern stAbility		*pFirstAbility;
extern stAbilityNum		*pFirstAbilityNum;
extern stEffect			*pFirstEffect;
extern clTerrainType	*pFirstTerrain;
extern clBF_Object		*pFirstBF_Obj;
extern clResource		*pFirstResource;
extern clRace			*pFirstRace;
extern clUnitUpgrade	*pFirstUnitUpg;
extern clUnitType		*pFirstUnitType;
extern clSpell			*pFirstSpell;
extern clItem			*pFirstItem;
extern clGuard			*pFirstGuard;
extern clGuardType		*pFirstGuardType;
extern clProvinceType	*pFirstProvinceType;
extern clOuterBuilding	*pFirstOuterBld;
extern clDefender		*pFirstDefender;
extern clSite			*pFirstSite;
extern clRitual			*pFirstRitual;
extern clProvinceName	*pFirstProvinceName;
extern clDialog			*pFirstDialog;
extern clDialog			*pFirstCampDialog;
extern clQuestion		*pFirstQuestion;
extern clInnerBuilding	*pFirstInnerBld;
extern clPlayerProfile	*pFirstPlayerProfile;
extern clFog			*pFirstFog;
extern clItemSet		*pFirstItemSet;
extern clMedal			*pFirstMedal;
extern clEvent			*pFirstEvent;
extern clEvent			*pFirstCampEvent;
extern clEncounter		*pFirstEncounter;
extern clQuest			*pFirstQuest;
extern clMapObject		*pFirstMapObject;
extern clLandmark		*pFirstLandmark;
//extern clMap			MapSet;
extern clCastle			*pFirstCastle;
extern clHeroAdvice		*pFirstHeroAdvice;
extern clHeroAdvice		*pFirstNick;
extern clHeroAdvice		*pFirstTitle;
extern clHeroAdvice		*pFirstAttitude;
extern clShardBonus		*pFirstShardBonus;
extern stString			*pFirstPrefix;
extern stString			*pFirstSuffix;
extern clJournal		*pFirstJournal;
extern clCredits		*pFirstCredits;
extern clWorldRules		*pFirstWorldRules;
extern clDifficulty		arDifficulty[MAX_DIFFICULTY];

extern clDipText * pHumanDiplo;
extern int *pWorkSet;	//Динамический массив для поиска предметов в сетах
extern stString GroupName[MAX_BLD_GROUP];	//Название групп построек
extern stString ClassName[MAX_CLASS_NAME];
extern stMessage ClassText[MAX_CLASS_NAME], ClassDesc[MAX_CLASS_NAME];
extern BITMAP * ClassPic[MAX_CLASS_NAME];
extern stString	sMapFile;	//Имя файла текущей карты
extern int arTalent[MAX_CLASS_NAME][MAX_TALENT][2]; //Таланты героев (индекс класса, номер, индекс умения/величина)
extern stString ItemSlotName[MAX_ITEM_SLOT_NAME];
extern clSkill arSkills [MAX_HERO_SKILL+1];
extern stMessage arSkillText [MAX_HERO_SKILL+1];
extern clProvinceLevel ProvLevel [MAX_PROVINCE_LEVEL+1];
extern stShardSize arShardSize[MAX_SHARD_SIZE]; //Массив размеров осколков
extern int arCampDiff[MAX_DIFFICULTY][5]; //Изменение уровня сложности в кампании (начальный, конечный, интервал в ходах, лимит ходов, додификатор дохода энергии в %)


clUnitUpgrade * UnitUpg(int Index);
clUnitType * UnitType(int Index);


//Максимальное количество прототипов
extern int	iMaxPhrases,			
			iMaxName,
			iMaxUnitClass,
			iMaxUnitKind,
			iMaxItemType,			
//			iMaxAbility,
			iMaxAbilityNum,
			iMaxEffect,
			iMaxTerrain,
			iMaxBF_Object,
			iMaxResource,
			iMaxRace,
			iMaxUnitUpg,
			iMaxUnitType,
			iMaxSpell,
			iMaxItem,		//Общее количество предметов
			iMaxPureItem,	//Количество предметов в файле Item.var
			iMaxDefItem,	//Количество свитков для найма стражей
			iMaxBldItem,	//Количество чертежей построек
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
			iMaxNick,
			iMaxTitle,
			iMaxHeroAdvice,
			iMaxAttitude,
			iMaxShardBonus,
			iMaxPrefix,
			iMaxSuffix,
			iMaxJournal,
			iMaxCredits,
			iMaxWorldRules;
#endif
