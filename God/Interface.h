#ifndef USED_INTERFACE
#define USED_INTERFACE

#include "prototype.h"

int PlayUnitSound(int Index);
int PlaySpellSound(int Index);

class clAudioTrack
{
public:
	ALOGG_OGG * Track;
	float	fVolume,
			fFading;
	int		Index,
			iVolume,
			iOld,
			iPan,
			iSpeed,
			iLoop,
			ipUnit;
	bool	bActive, bFree;
	clAudioTrack();
	~clAudioTrack();
	void Start(int Volume, float Fading, int Pan, int Speed, int Loop);
	void Poll();
	void Clear();
};

class G_Object;

class G_Cloud
{
	public:
	static const int PicSize=50;
	int Type,
		Trans,
		Angle,dAngle;
	float
		x,y,dx,dy;
	BITMAP *Pic;

G_Cloud(int x1, int y1, int x2, int y2, int Time);
~G_Cloud();
void Run(int Time, BITMAP* Bmp);
};

class G_Death
{
public:
	static const int MaxTime=50;
	int Type,
		Timer,
		x,y,Start,	
		Line;
	BITMAP *Pic, *Skull;
	G_Cloud *Clouds[12];

G_Death(int sx, int sy, BITMAP *Dying);
~G_Death();
bool Run (BITMAP*Bmp);
};

class G_Raise
{
public:
	int x,y,	
		Line;
	BITMAP *Pic;
	G_Object *Obj;

G_Raise(G_Object * Target);
~G_Raise();
bool Run (BITMAP*Bmp);
};

class G_Stars
{
private:
	int SS [4][50]; //Массив звездочек 0-Время 1-x 2-y 3-направление
public:
	int iTimer, //Время
		iNumber, //Максимум звездочек
		x,y,      //Смещение от координат объекта
		w,h,	//Ширина и высота
		r,g,b;	//Базовый цвет
	G_Object *Obj;	//Ссылка на объект привязки
	BITMAP *Pic;
	
	G_Stars (G_Object *OBB, int Timer, int Number, int X, int Y, int W, int H, int R, int G, int B);
	~G_Stars ();
	bool Run(BITMAP * Bmp);
};

//Класс для хранения очереди эффектов
class clEffQue
{
public:
	int ipType,			//Тип эффектаа
		iDelay,			//Финальная жизнь цели
		iNum,			//Числовое значение
		iColor,			//Цвет
		Param1,			//Параметры
		Param2,
		x,y,w,h,
		r,g,b;
	int wavSound;

	G_Object *Obj;
	clEffQue *pNext;	//Следующий эффект

	clEffQue(int Type, int Delay, G_Object *OBB);
	~clEffQue();
	void Call();							//Вызов мгновенного эффекта
	void Proceed(G_Object *Linker);		//Вызов эффекта с добавлением в очередь
};

//Класс для хранения информации об атаке
class clAttackAnim
{
public:
	G_Object *pEnemy;	//Графический объект противника
	int iHit,			//Сила удара (если отрицательна  - противники сходятся)
		iLife,			//Финальная жизнь цели
		x,y,			//Исходная координата воина
		x1,y1,			//Исходная координата врага
		x2,y2,			//Координаты для перемещения после Trample
		iAttack;		//1 если наш воин атакует, 0 если атакует враг, -1 для выстрела
	bool bTrample;		//Истина, если в конце атаки воин становится на тайл противника
	clAttackAnim *pNext;	//Следующая анимация
	clEffQue *pUnitEffect,
			*pEnemyEffect;
	int		wavHit,	//Звук удара
			wavDeath;	//Звук смерти

	clAttackAnim(G_Object *Enemy, int Hit, int Attack, int Life, int wHit, int wDeath);
	~clAttackAnim();
};

//Класс для построения дерева пути
class clPath
{
public:
	int x, y;
	clPath *pPrev;

	clPath (int sx, int sy, clPath *p);
	~clPath();
};

//**************************************
//*** Графический объект ***************
//**************************************

class G_Object
{
//private:

public:
	G_Stars *pStars;	//Указатель на спецэффект "звездочки"
	int x,y,			//координаты
		MapX,MapY,
		iLayer;			//Поверхность отрисовки (0-фон, -1 - невидим, 1..х слои)
	BITMAP	*Pic,
			*Shadow,
			*FlipShadow,
			*bmpStone;
	
	int iStep,			//Шаг анимации
		iDelay,			//Задержка анимации
		iWait,			//Ожидание (объект невидим)
		iTimer,			//Таймер очереди объектов
		iExplo,			//Счётчик анимации взрыва
		iWeb,			//Счётчик анимации паутины
		iIce,			//Прозрачность заморозки
		iStone,			//Анимация окаменения в процессе
		iRoot,			//Счётчик анимации корней
		iAngle,			//Угол вращения спрайта
		iRotate,		//Скорость вращения
		iFading,		//Скорость накопления прозрачности исчезающего объекта		
		iOpacity;		//Коэффициент прозрачности
	bool	bAnim,		//Истина, если в очереди есть анимация
			bDestroyPic, //Если истинна, картинка уничтожается вместе с объектом
			bFlip,		//Если истинна, картинка будет отзеркалена
			bKillMe,	//Объект будет безжалостно удален
			bDestroyOnDeath, //Удалить объект по окончании спецэффекта смерти	
			bInvisOnDeath,	//Делает объект невидимым после смерти
			bMap,		//Объект глобальной карты
			bTempo,		//Временный объект
			bTrans,		//Прозрачный
			bReincarnate;	//Возродится после смерти

	float	fDeltaX,	//Смещение по X
			fDeltaY,	//Смещение по Y
			fX,	fY;		//Текущие координаты для анимации
	clPath *Track;		//Траектория анимации
	clPath *Waypoint;	//Текущий участок анимации
	clAttackAnim *pAttack;	//Очередь атак для анимации
	G_Object *pLink;		//Указатель на временные созданные объекты
	G_Object *pCargo;		//Указатель на прикрепленный объект	
	G_Death *pDeath;		//Указатель на спецэффект "смерть"
	G_Raise *pRaise;		//Указатель на спецэффект "восстание"
	clEffQue *pEffQue;		//Очередь эффектов для прилинкованных объектов


	G_Object *pNext, *pPrev;
	G_Object(G_Object *Next, int sx, int sy, int Layer, BITMAP *Bmp);
	G_Object(G_Object *Next, int sx, int sy, int Layer, BITMAP *Bmp, bool Destroy);
	~G_Object();
	void Make_Stars (G_Object *OBB, int Timer, int Number, int X, int Y, int W, int H, int R, int G, int B);
	void Show(BITMAP *Target);
	void Flip();
	void StoneForm();	//Начать обесцвечивание объекта
	void Move(clPath *Path, int Step, int Delay, bool Add); //Начать движение по ломаной траектории, если Add истинно - добавить траекторию к существующей
	void AnimAttack(G_Object *Enemy, int Hit, int Attacker, int Life, clUnitType *uAttacker, clUnitType *uDefender);
	void AnimShoot(G_Object *Enemy, int Hit, int Life, int Wait, clUnitType *Shooter, clUnitType *Target, int Miss, int Speed);	
	void AnimDeath(int wavDeath);
	void AnimText(int ipText, int Color);	//Выдает фразу с индексом ipText над объектом
	void AnimText(char * sText, int Color);
	void AnimNumber(int iNum, int Color);	//Выдает число над объектом
	void AnimNumber(int iNum, int Color, int dX);	//Выдает число над объектом со смещением вправо по Х
	void AnimIcon(BITMAP *bmpIcon, int Speed);				//Вывести исчезающую иконку
	void AddCargo(G_Object *Cargo, int dX, int dY, int Layer); //Добавить прикрепленный объект со смещением dX,dY на внутренний слой	
	bool RemoveCargo(G_Object *Cargo);
	void MoveCargo(G_Object *Cargo, int DX, int DY);
	bool PlayAnimation();
	bool ShowLink(int Layer);
	void PlaceOnMap(int mX, int mY);
	void SmartLog(const char *Txt);
	clEffQue * AddAttackUnitEffect(int ipType, int iDelay);
	clEffQue * AddAttackEnemyEffect(int ipType, int iDelay);
	clEffQue * AddAttackEnemyEffect(int ipType, int iDelay, G_Object *objTarget);
};


//**************************************
//*** Маска ****************************
//**************************************

class UI_Mask
{
public:
	int flag, //0-невидим, 1-видим, 2-активен, 3-нажат, 4-нажат (правый клик)
		iDefLayer,
//		ipHotkey,
		iHint;	//Номер фразы с подсказкой
	bool bGlow;
	BITMAP *bmpMask;
	G_Object *G_Obj, *Glow_Obj;
	UI_Mask();
	~UI_Mask();
	G_Object * Init(int st_x, int st_y, int st_flag, BITMAP *Mask, BITMAP* Active, G_Object *Obj, int Layer);
	G_Object * Init(int st_x, int st_y, int st_flag, BITMAP *Mask, BITMAP* Active, BITMAP *Glow, G_Object *Obj, int Layer);
	int Check();
	int Check(int MX, int MY);	//Проверка маски, привязанной к карте
	int ColorCheck();
	void Activate(int st_flag);
	void Replace(int st_x, int st_y);
	void Show();
};

//**************************************
//*** Невидимая маска ******************
//**************************************

class UI_Mask1
{
public:
	int flag, //0-отключен, 1-включен, 2-активен, 3-нажат, 4-нажат (правый клик)		
		x,y,w,h,	//координаты и размеры
		iHint;		//Номер фразы с подсказкой
	UI_Mask1();
	~UI_Mask1();
	void Init(int st_x, int st_y, int st_w, int st_h, int st_flag);
	int Check();
//	int Check(int MX, int MY);	//Проверка маски, привязанной к карте	
	void Activate(int st_Layer);
	void Replace(int st_x, int st_y);
//	void Show();
};



//**************************************
//*** Кнопка обычная *******************
//**************************************
class UI_Button1
{
public:
	int flag,		//-2 вечно нажат, -1-невидим, 0-выключен, 1-включен, 2-активен, 3-нажат	
		iDefLayer,	//Слой вывода
		ipHotkey,	//Горячая кнопка
		ipHotkey2,
		Roller,		//Сосотояние колёсика мыши
		iHint;		//Номер фразы с подсказкой
	bool bHot, bBlink;
	BITMAP *bmpNormal, *bmpActive, *bmpPressed, *bmpInactive;
	int wavSound;

	G_Object *G_Obj;
	UI_Button1();
	~UI_Button1();
	G_Object * Init(int st_x, int st_y, int st_flag, int Layer, BITMAP *Norm, BITMAP *Act, BITMAP *Pressed, BITMAP *Inact, G_Object *Obj);
	int Check();
	void Activate(int st_flag);
	void Replace(int st_x, int st_y);
};


//**************************************
//*** Кнопка переключатель *************
//**************************************

class UI_Button2 : public UI_Button1 
{
public:
	bool bPress;	//Текущее состояние кнопки
	int iHint1; //Хинт для нажатой кнопки
	BITMAP *bmpPressedActive;
	G_Object * Init(int st_x, int st_y, int st_flag, int Layer, BITMAP *Norm, BITMAP *Act, BITMAP *Pressed, BITMAP *Inact, BITMAP *PressAct, G_Object *Obj);
	void Activate(int st_flag, bool Pressed);
	int Check();
	UI_Button2();
};

//**************************************
//*** Блок текста **********************
//**************************************

class UI_TextBox
{
public:
	char * Text;
	int x, y,
		w, h,
		Roller,
		Layer,
		Color,
		Lines,		//Счётчик строк
		CurrentLine,
		ScrollSize;
	bool bScroll,
		bInside,
		bShadow,	//Выводить ли тень под текстом
		bCentre;	//Выравнивание по центру
	FONT * Fnt;
	BITMAP *Pic, *Pic1;
	UI_Button1 buttUp, buttDown;
	G_Object *Obj;
	UI_TextBox();
	~UI_TextBox();
	G_Object * Init(int st_x, int st_y, int st_w, int st_h, int st_Layer, int st_Color, FONT * st_Fnt, G_Object * st_Obj);
	G_Object * Init(int st_x, int st_y, int st_w, int st_h, int st_Layer, int st_Color, FONT * st_Fnt, G_Object * st_Obj, int x_up, int y_up, int x_down, int y_down);
	void Activate(char * Str);
	void Hide();
	void Check();
};

//**************************************
//*** Блок ввода текста ****************
//**************************************

class UI_Input
{
public:
	char Text[500];
	int x, y,
		Length,
		Max_Length,
		Layer,
		Color,
		Cursor,
		Type,		//1 - только числа
		Flag;
	bool bCenter;	//Отцентровать текст
	FONT * Fnt;
	BITMAP *Pic;
	G_Object *Obj;
	UI_Input();
	~UI_Input();
	G_Object * Init(int st_x, int st_y, int st_Type, int st_Max_Length, int st_Layer, int st_Color, FONT * st_Fnt, G_Object * st_Obj);
	void Clear();
	void Show();
	void EnterInt(int Zifer);
	long int GetZyfer();
	void Activate(int FLG);	
	int Check(bool Active);
};

//**************************************
//*** Скролл-бар ***********************
//**************************************

class UI_ScrollBar
{
public:	
	int x, y,
		Length,
		Width,
		Layer,
		Flag,
		DropDist;	//Расстояние курсора, при котором происходит сброс значения	
	bool bVertical,	//true для вертикального, false для горизонтального	
		Drag;

	int *pValue, Min, Max;
	UI_TextBox *pText;

	BITMAP *PicN, *PicA;
	G_Object *Obj;
	UI_Button1 buttAdd, buttSub;	
	UI_ScrollBar();
	~UI_ScrollBar();
	G_Object * Init(int st_x, int st_y, int st_Length, int st_Width, int st_Layer, bool Vertical, bool Buttons, G_Object * st_Obj);
	void AttachValue(int *Zifer, int MinVal, int MaxVal);
	void AttachText(UI_TextBox *NewText);
	void Show();	
	void Activate(int FLG);	
	int Check();
};


//**************************************
//*** Объект карты мира ****************
//**************************************
/*
class Map_Object
{
public:
	int x, y,	//Координаты на карте
		iLayer,	//Слой отрисовки
		iFlag;	//Флаг состояния
	BITMAP *Pic;
	Map_Object *pNext, *pPrev;
	
};
*/


void ItIsTime();		//Устанавливает флаг обновления экрана
//void ChangeMusic(int iTrack);
void ChangeMusic();
void MouseTime();

G_Object * ShowActiveObjects(G_Object *First, int MaxLayer, bool Mouse); //Выводит на экран все активные графические объекты
G_Object * ShowActiveObjects(G_Object *First, int MaxLayer, bool Mouse, int x, int y); //То же, но с выводом карты
void ShowIconNumber(BITMAP *bmpPic, int x, int y, int Num);
void ShowIconPercent(BITMAP *bmpPic, int x, int y, int Num);
void ShowIconString(BITMAP *bmpPic, int x, int y, char * Str);

void MakeStringHint (int Phrase);
void StupidLogger(G_Object * Obj, const char * Log);

#endif

