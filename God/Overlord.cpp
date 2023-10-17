#include <iostream>
#include <fstream>
#include "Terrain.h"

using namespace std;

ifstream CurrentFile("empty.dmb");

int ReadInt(char &ch); //—читывает из текущего файла целочисленную переменную
int ReadWord(char *ch); //—читывает из текущего файла текст до символов ';' или ':', возвращает длину строки

int main()

{
	char cSymbol=' ';
	char aWord[40];
	char *pWord= aWord;
	int iLength;
	CurrentFile.close();
	CurrentFile.open("terrain.txt");
	CurrentFile.clear();
	iLength=ReadWord(pWord);
	int iMaxTerrain=ReadInt(cSymbol);
	clTerrainType *oTerrain = new clTerrainType[iMaxTerrain];
	clTerrainType *pTerrain = oTerrain;
	for (int i=1; i<=iMaxTerrain; i++)
	{
		for (int j=1; j<=6; j++)
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
			}
			
		}
		pTerrain++;
	}
	CurrentFile.close();
	pTerrain = oTerrain;
	for (i=1; i<=iMaxTerrain; i++)
	{
		cout << pTerrain->sName << " Gold: " << pTerrain->iGoldInc << " Crystall: " << pTerrain->iGemInc << endl;
		pTerrain++;
	}



/*


	while (!CurrentFile.eof())
	{
		cout << ReadInt(cSymbol) << endl;
	}
	cout << endl;
	CurrentFile.close();

	CurrentFile.open("number");
	CurrentFile.clear();
	while (!CurrentFile.eof())
	{
		cout << ReadInt(cSymbol) << endl;
	}
	cout << endl;
	CurrentFile.close();

	CurrentFile.open("words.txt");
	CurrentFile.clear();
	
	while (!CurrentFile.eof())
	{
		iLength=ReadWord(pWord);
		cout << aWord << "=" << iLength << endl;
	}
	CurrentFile.close();

*/

	delete [] oTerrain;
	return 0;
}

int ReadInt(char &ch)
{
	int iVal=0;
	bool bRead=true, bNegative=false;

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
}

int ReadWord(char *ch)
{
	bool bRead=true;
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
