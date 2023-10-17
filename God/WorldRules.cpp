#include "Unit.h"

dcWorldRules::dcWorldRules(int Index)
{
	iMode=Index;
	for (int i=0; i<MAX_TRIGGER; i++)
	{arTrigger[i]=0;}
}

dcWorldRules::~dcWorldRules()
{}

void dcWorldRules::Init(int Index)
{
	iMode=Index;
	for (int i=0; i<MAX_TRIGGER; i++)
	{arTrigger[i]=0;}
}