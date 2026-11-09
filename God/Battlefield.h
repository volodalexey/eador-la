#ifndef USED_BFUNIT
#define USED_BFUNIT

#include "Windower.h"

extern int MinWeight;		//Минимальная длина пути (в единицах скорости)
extern int MinStamina;		//Необходимая стамина для найденного пути
extern BITMAP *bmpPathPic;
extern BITMAP *bmpUnitBar;
extern BITMAP *bmpWarMenu, *bmpCheckBox, *bmpCheckBird;
extern int ActivePlayer;	//Активный игрок-человек

extern clPath *TruePath;	//Указатель на найденный путь

class BF_Unit;
extern BF_Unit * arNeigh [6];	//Массив для хранения соседних юнитов
extern dcHero * Heroes[2];		//Участвующие в бою герои
extern int LifeMod[2];			//Изменение максиума жизни нейтралов в завасимость от сложности (в %)
extern int SummonExp[2];		//Опыт за вызванных существ
extern int SummonSurv[2];		//Сколько суммоны прожили
extern int SummonWork[2];		//Сколько наработали
extern int GemSpent[2];
extern bool IsPlayed[2];		//Флаги игроков-людей
extern int SideCanCast[2];		//Флаги использования заклятий при автобое 0 - полный запрет, 1 - без жертв, 2 - любые

const int MAX_SUMMONS=SPELLBOOK_SIZE;
const int POISON_DURATION=3;		//Продолжительность действия яда
extern bool bFirstMove;				//Флаг истинный, если активный юнит еще не двигался
extern bool bShowBattle;			//Истинна если в бою есть игрок-человек
extern bool bAltFlag;				//Флаг нажатия клавиши Alt
extern bool bCastle;
extern UI_Button2 buttCrush, buttDoubleShot, buttRoundAtt;
extern G_Object *objMain;
extern G_Object *objSim;			//Объект для эмуляции битв
extern G_Object *objSelect;
//extern int arSimCrystal[2];		//Количество кристаллов для симуляции боя




//Ячейка поля боя
struct strHexx
{
	int Land,	//Тип ландшафта (или затраты хода для массива поиска пути)
	Shoot;		//Флаг области обстрела
	bool Pass;	//Проходимость
};


extern strHexx arGrid [8] [8];	//Карта проходимости
extern strHexx Field [8] [8];	//Карта для поиска пути
extern int CanPass[3];			//Флаги проходимости территорий для учета знания местности при поиске пути
extern int ProvinceMoraleBonus;



//Эффекты на юнитах
class BF_Effect
{
public:

//	char sName[20];		//Название
//	bool bVisible;		//Отображается ли в инфо окне
	int iType,			//Тип эффекта
		iPower,			//Сила эффекта (множитель)
		iTime;			//Сколько ходов будет действовать (0-постоянный)
	bool bVisible,		//Виден ли на панели
		bDestroyOnHit;	//Уничтожается если цель получила повреждения
	BF_Effect *pNext;	//Ссылка на следующий эффект в списке
	BF_Effect *pPrev;	//Ссылка на предыдущий эффект в списке
	clSpell * pSpell;	
	clUnitUpgrade * pAbility;
	BF_Effect(BF_Effect *prEffect, int Type, int Power, int Time);
//	BF_Effect(BF_Effect *prEffect, int Type, int Power, int Time, char Name[20]);
	~BF_Effect();
};

//Преобразование координат юнита в координаты графического объекта
int TransX (int x, int y, int w);
int TransY (int x, int y, int h);

class BF_Unit
{
public:
		
	int iLife,				//Текущая жизнь
		iSpeed,				//Запас хода
		iMorale,			//Текущий бевой дух
		iFear,				//Показатель страха (чем выше, тем ниже шанс восстановить боевой дух)
		iStamina,			//Текущая выносливость
		iAmmo,				//Количество выстрелов
		iMeleeDamageDone,	//Нанесено урона в рукопашной
		iRangeDamageDone,	//Получено урона в рукопашной
		iMeleeDamageTaken,	//Нанесено урона выстрелами
		iRangeDamageTaken,	//Получено урона выстрелами
		iSpellDamageTaken,	//Получено урона от заклинаний
		iPoisonDamageTaken,	//Получено урона от яда
		iDamageHealed,		//Излечено урона
		iSpellCast,			//Потрачено выносливости на вызов заклинаний
		iEnemyShotted,		//Застрелено врагов
		iEnemySlayed,		//Убито в рукопашную врагов
		iStaminaUsed,		//Потрачено выносливости
		x, y,				//Координаты на поле боя
		iSurvived,			//Сколько ходов сражался
		iSide,				//За какую сторону сражается
		iNumm,				//Порядковый номер в отряде
		ipSummonType;		//Тип вызванного юнита
	bool bCanAct,			//Может ли действовать на данном ходу
		bResting,			//Выполняет команду "Отдыхать"
		bMarch,				//Можно ли использовать марш-бросок
		bParry,				//Можно ли использовать парирование
		bInBattle,			//Находится в бою
		bPacked,			//Флаг превращения в демона (для очистки памяти при симуляции)
//		bWait,				//Для АИ, ожидание движения
		bSummoned;			//Призванный юнит
	G_Object *G_Obj;		//Графическая часть
	G_Object *objBar;		//Объект с хинтом
	BITMAP * bmpBar;		//Картинка хинта
	dcUnit *pUnit;			//Указатель на юнит отряда
	dcUnit *pTrueForm;		//Хранит ссылку на иситинного юнита на случай полиморфа
	dcHero *pHero;			//Указатель на героя, к отряду которого юнит принедлежит
	BF_Effect *pEffect;		//Список активных эффектов
		
	BF_Unit();
	~BF_Unit();
	G_Object * Init(dcUnit *Un, G_Object *Obj, int Sider, int Numm);
	G_Object * Summon(int U_Type, int Power, int x, int y, G_Object *Obj);
	void Clear(); //Очистка информации о юните
	void ChangeSide(int NewNum);	//Переход на сторону врага, занять слот NewNum
	void DrawBar();					//Отрисовка хинта под юниитом
	int CheckEffect(int Index);		//Проверка наличия выбранного эффекта
	int GetEffectDuration(int Index);	//Проверка длительности эффекта
	int CheckSkill(int Index);		//Проверка наличия улучшения в свойствах, повышениях и эффетках
	bool CheckCaster();				//Проверка, умеет ли юнит колдовать
	int FindNeighbour(int Side);	//Находит соседних юнитов стороны Side и заносит их в массив arNeigh, возвращает количество найденных
//	void AddEffect(int Eff, int Power, int Time);
//	void RemoveEffect ( Eff);
	bool TakeDamage (int Hit, int dType);  //Юнит получает повреждения, dType 0 - рукопашные, 1 - дистанционные, 2 - от заклинания. Возвращает 1 если он погиб
	void Move(int x1, int y1);  //Переместить юнита
	void Replace(int x1, int y1); //Мгновенно переместить юнита
	int SoulSteal (BF_Unit *Enemy);	//Выпить душу врага, возвращает количество полученной жизни
	void RaiseCorpse(BF_Unit *Enemy);	//Создать скелета из поверженного врага
	int Attack(int x1, int y1, BF_Unit *Enemy); //Атаковать юнита Enemy с клетки x1, y1, возвращает 0 если атакующий погиб
	int UnderSpell(clSpell *Spell, BF_Unit * Hero, int Caster); //Воздействие заклинания Spell, примененного магом Hero, принадлежащим стороне Caster
	void Shoot(BF_Unit *Enemy);	//Стрелять в юнита Enemy
	void Heal(BF_Unit *Target); //Лечить юнита Target	
	void Panic();
	void Death();
	void UseStamina(unsigned int Num);	//Тратит выносливость юнита
	void ChangeMorale(int Num);			//Изменяет боевой дух юнита
	void GoMarch();		//Включает марш-бросок для управляемого АИ юнита
	bool IsNeighbour(int x, int y);	//Возвращает истину, если юнит находится в соседнем с указанным тайле.
	void ShowInfoScreen();		//Вывести окно с информацией о юните
	void MakeParamHint(int Param);	//Отрисовать хинт о параметре Param

	//Получение параметров в зависимости от повышений, наград и эффектов
	int GetLife();
	int GetAttack();
	int GetCounterAttack();
	int GetDefence();
	int GetRangedDefence();
	int GetResist();
	int GetSpeed();	
	int GetFatigure(); //Возвращает 1 если скорость меньше 1
	int GetRangedAttack();	
	int GetShootingRange();
	int GetAmmo();
	int GetStamina();
	int GetMorale();
	int GetExp();
	int GetRace();
	int	GetUnitKind();
	int	GetUnitClass();
	int GetKarma();
};

extern	int AA;									//Активный игрок
extern	int BF_Player[2];						//Игроки
extern	int Spells[2][2][SPELLBOOK_SIZE];		//Заклинания
extern BF_Unit Squad[2][MAX_SQUAD+MAX_SUMMONS];	//Отряды
extern BF_Unit * ActiveUnit;					//Указатель на активный юнит
extern bool bArchmage;							//Флаг истинный, если герой может вызывать 2 заклинания в ход
extern int IndexAI;								//Используется для оптимизации хода ИИ

int GetMoveCost(int x, int y);
int GetStamCost(int x, int y);
void SimulateMap(int x, int y, int Weight, int Speed, int SpeedLeft, bool Flight);
void SimulatePath (int x, int y, int Weight, clPath *Path, int x1, int y1, int Speed, int SpeedLeft, bool Flight);
int StepMap (int x, int y, int Weight, int Speed, int StamCost, int StamLeft); //Рекурсивная функция для создания карты поиска пути
int StepPath (int x, int y, int Weight, clPath *Path, int x1, int y1, int StamCost, int StamLeft); //Рекурсивная функция для поиска пути
//int StepPath (int x, int y, int Weight, clPath *Path, int x1, int y1, int StamCost, int StamLeft, bool NoTrans);
int FlyMap (int x, int y, int Weight, int Speed, bool Over); //Рекурсивная функция для создания карты поиска пути для летающих юнитов
int FlyPath (int x, int y, int Weight, clPath *Path, int x1, int y1); //Поиск пути для летающих юнитов
//int FlyPath (int x, int y, int Weight, clPath *Path, int x1, int y1, bool NoTrans);
int ShootMap (int x, int y, int Weight, int Range); //Рекурсивная функция для создания карты дальности выстрела

//int StepAIMap (int x, int y, int Weight, int Speed, int SpeedLeft, int StamCost, int StamLeft);
//int StepAIPath (int x, int y, int Weight, clPath *Path, int x1, int y1, int Speed, int SpeedLeft, int StamCost, int StamLeft);
//int FlyAIMap (int x, int y, int Weight, int Speed, int SpeedLeft, bool Over);
//int FlyAIPath (int x, int y, int Weight, clPath *Path, int x1, int y1, int Speed, int SpeedLeft);

int GetHit(int Attack, int Defence);
int GetMeleeHit(BF_Unit * Attacker, BF_Unit * Defender, bool bAttacking);	//Вычисление рукопашных повреждений
int GetRangedHit(BF_Unit * Attacker, BF_Unit * Defender);	//Вычисление дистанционных повреждений
int CalcMeleeHit(BF_Unit * Attacker, BF_Unit * Defender, bool bAttacking);	//Вычисление рукопашных повреждений без рандома
int CalcRangedHit(BF_Unit * Attacker, BF_Unit * Defender);	//Вычисление дистанционных повреждений без рандома

void AI_CalcPower();
void AI_CalcDangerZone();
int AI_Main(bool CanMove);
void BF_RefreshScreen();
int FindCorpse(int Side, int x, int y);
BF_Unit * FindNextUnit();
void CorpseSpell(clSpell *Spell, BF_Unit *Corpse);
void TileSpell(clSpell *Spell, int x, int y);
int Loss(int Side); //Проверка при потере юнита (возвращает индекс живого юнита или -1 если юнитов нет)
int WarMenu(int AA);
void EndBattle(int Win, int Site);

void AI_SimulateDangerZone();
void AI_SimulatePower();
void AI_Simulate();


#endif
