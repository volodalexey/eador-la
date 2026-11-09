#include "Unit.h"
#include "time.h"

int RollerP[MAX_ROLL], RollerN[MAX_ROLL];			//Массивы вероятностей и ссылок для случайного выбора
//unsigned int iSeed;									//Семя для случайных чисел


void Randomizer()
{
	time_t tTime;
	time(&tTime);
	long int RND=tTime%10000;
	srand(RND);
};

int sign(int Zifer)
{
	if (Zifer==0)
	{return 0;};
	return Zifer/abs(Zifer);
};

L_INT Random(L_INT Max)
{
	if (Max<=0)
	{
		return 0;
	}
	else
	{		
		L_INT Result=rand(), Add=Max;
		while (Add>30000)
		{
			Add/=10;
			Result=Result*10+rand()%10;
		};
		
		return Result%Max;
	};
};

L_INT Random(L_INT Max, int Param1, int Param2)
{
	double Temp=CurGame.iSeed;
	while (Temp>10)
	{
		Temp/=10;
	};
	Temp=Temp*(Param1*100+Param2+1);
	Temp=log(Temp)*100000;
	L_INT iTemp = Temp;
	Temp=Temp-iTemp;
	Max*=Temp;
	return Max;
};

L_INT Random(L_INT Max, int Param1, int Param2, int Param3)
{

	long double Temp=CurGame.iSeed;
//	while (Temp>10)
//	{
//		Temp/=10;
//	};
	Temp=Temp*(Param1*100+Param2+1)/99;
	Temp=log(Temp)/(Param3+100)*10000000;
	if (Temp<0) {Temp*=-1;};
	L_INT iTemp = Temp;
	Temp=Temp-iTemp;
	Max*=Temp;
	return Max;
};

L_INT Random(L_INT Max, int Param1, int Param2, int Param3, int Param4)
{

	double Temp=CurGame.iSeed;
	while (Temp>10)
	{
		Temp/=10;
	};
	Temp=Temp*(Param1*100+Param2+1)+Param4;
	Temp=log(Temp)/(Param3+100)*10000000;
	L_INT iTemp = Temp;
	Temp=Temp-iTemp;
	Max*=Temp;
	return Max;
};

L_INT Roll(bool Exclude)
{
	L_INT Sum=0, Param;	
	int Index;
	for (int i=0; RollerN[i]; i++)
	{
		if (i==MAX_ROLL)
		{
			set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
			allegro_message("Roller limit reached!");
		};
		Param=RollerP[i];
		Sum+=Param;
	};
	L_INT Result=Random(Sum)+1;
	Sum=0;	
	for (int i=0; Sum<Result; i++)
	{
		Param=RollerP[i];
		Sum+=Param;		
		Index=i;
	};
	while (RollerP[Index]==0)
	{Index++;};
	Result=RollerN[Index];	
	if (Exclude)
	{
		for (int i=0; RollerN[i]; i++)
		{
			if (RollerN[i]==Result)
			{
				RollerP[i]=0;		
			};
		};
	};
	return Result;
};

L_INT Roll(bool Exclude, int Param1, int Param2)
{
	L_INT Sum=0;	
	int Index;
	for (int i=0; RollerN[i]; i++)
	{
		if (i==MAX_ROLL)
		{
			set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
			allegro_message("Roller limit reached!");
		};
		Sum+=RollerP[i];
	};
	L_INT Result=Random(Sum, Param1, Param2)+1;
	Sum=0;	
	for (int i=0; Sum<Result; i++)
	{
		Sum+=RollerP[i];		
		Index=i;
	};
	while (RollerP[Index]==0)
	{Index++;};
	Result=RollerN[Index];	
	if (Exclude)
	{
		for (int i=0; RollerN[i]; i++)
		{
			if (RollerN[i]==Result)
			{
				RollerP[i]=0;		
			};
		};
	};
	return Result;
};

L_INT Roll(bool Exclude, int Param1, int Param2, int Param3)
{
	L_INT Sum=0;	
	int Index;
	for (int i=0; RollerN[i]; i++)
	{
		if (i==MAX_ROLL)
		{
			set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
			allegro_message("Roller limit reached!");
		};
		Sum+=RollerP[i];
	};
	L_INT Result=Random(Sum, Param1, Param2, Param3)+1;
	Sum=0;		
	for (int i=0; Sum<Result; i++)
	{
		Sum+=RollerP[i];		
		Index=i;
	};
	while (RollerP[Index]==0)
	{Index++;};
	Result=RollerN[Index];	
	if (Exclude)
	{
		for (int i=0; RollerN[i]; i++)
		{
			if (RollerN[i]==Result)
			{
				RollerP[i]=0;		
			};
		};
	};
	return Result;
};

L_INT Roll(bool Exclude, int Param1, int Param2, int Param3, int Param4)
{
	L_INT Sum=0;	
	int Index;
	for (int i=0; RollerN[i]; i++)
	{
		if (i==MAX_ROLL)
		{
			set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
			allegro_message("Roller limit reached!");
		};
		Sum+=RollerP[i];
	};
	L_INT Result=Random(Sum, Param1, Param2, Param3, Param4)+1;
	Sum=0;	
	for (int i=0; Sum<Result; i++)
	{
		Sum+=RollerP[i];		
		Index=i;
	};
	while (RollerP[Index]==0)
	{Index++;};
	Result=RollerN[Index];
	
	if (Exclude)
	{
		for (int i=0; RollerN[i]; i++)
		{
			if (RollerN[i]==Result)
			{
				RollerP[i]=0;		
			};
		};
	};
	return Result;
};

void ClearRoll()
{
	for (int i=0; i<MAX_ROLL; i++)
	{
		RollerP[i]=0;
		RollerN[i]=0;
	};
};

int GetMin(int A1, int A2)
{
	if (A1>A2)
	{return A2;}
	else
	{return A1;};
};

int GetMax(int A1, int A2)
{
	if (A1<A2)
	{return A2;}
	else
	{return A1;};
};