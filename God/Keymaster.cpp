//#define _AFXDLL
//#include "afxwin.h"
//
//void CWinApp::SetRegistryKeyOpen(LPCTSTR lpszRegistryKey)
//{SetRegistryKey(lpszRegistryKey);};
//
//void InitRegistry(char * sPath)
//{
//	char SS[1000];
//	sprintf(SS,"HKEY_CURRENT_USER\\Software\\%s",sPath);
//	CString stPath=sPath;
//	CWinApp* pApp = AfxGetApp();
//	pApp->SetRegistryKeyOpen(stPath);
//};
//
//void WriteRegistry(char * sDir, char * sKey, char * sVal)
//{
//	CString stDir=sDir, stKey=sKey, stVal=sVal;
//	CWinApp* pApp = AfxGetApp();
//	pApp->WriteProfileString(stDir, stKey, stVal);
//};

//#include "atlbase.h"
//#include "cstringt.h"
//#include "atlstr.h"

//CRegKey regMain;


#include <windows.h>
#include <stdio.h>
#include <strsafe.h>



#include <iostream>
#include <fstream>

using namespace std;
//int iSleepTime=24;

ofstream Log ("tracer.txt");

void FLog(char *Txt)
{
	Log << Txt << endl;	
};
/*
int InitRegistry(char * CompName)
{
	TCHAR chrComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD dwBufferSize = MAX_COMPUTERNAME_LENGTH + 1;
	if (GetComputerName(chrComputerName,&dwBufferSize))
	{
		for (int i=0; i<dwBufferSize; i++)
		{CompName[i]=chrComputerName[i];};
		return dwBufferSize;
	}
	else
	{return 0;};

};

bool WriteRegistry(char * sDir, char * sKey, char * sVal)
{
	HKEY hKey, hArtKey;
	bool bError=false;
	TCHAR tcDir[40];

	RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"Software",0,KEY_SET_VALUE,&hKey);
	MultiByteToWideChar(CP_ACP,0,sDir,strlen(sDir)+1,(LPWSTR)tcDir,sizeof(tcDir)/sizeof(tcDir[0]));	
	RegCreateKeyEx(hKey,tcDir,0,NULL,0,KEY_SET_VALUE,NULL,&hArtKey,NULL);	
	TCHAR tcKey[40];
	MultiByteToWideChar(CP_ACP,0,sKey,strlen(sKey)+1,(LPWSTR)tcKey,sizeof(tcKey)/sizeof(tcKey[0]));
	TCHAR tcVal[40];
	MultiByteToWideChar(CP_ACP,0,sVal,strlen(sVal)+1,(LPWSTR)tcVal,sizeof(tcVal)/sizeof(tcVal[0]));
	int Len=sizeof (tcVal)/sizeof (tcVal[0]);
	if (RegSetValueEx(hArtKey,tcKey,0,REG_SZ,(PBYTE)tcVal,Len)!=ERROR_SUCCESS)
	{bError=true;};
	RegCloseKey(hArtKey);
	RegCloseKey(hKey);
	FLog("W1");
	if (bError)
	{
		bError=false;
		FLog("fail");
		FLog("W2");
		RegOpenKeyEx(HKEY_CURRENT_USER,L"Software",0,KEY_SET_VALUE,&hKey);
		RegCreateKeyEx(hKey,tcDir,0,NULL,0,KEY_SET_VALUE,NULL,&hArtKey,NULL);
		Len=sizeof (tcVal)/sizeof (tcVal[0]);
		if (RegSetValueEx(hArtKey,tcKey,0,REG_SZ,(PBYTE)tcVal,Len)!=ERROR_SUCCESS)
		{
			bError=true;
			FLog("fail");
		};
		RegCloseKey(hArtKey);
		RegCloseKey(hKey);		
	};
	return bError;	
};

bool ReadRegistry(char * sDir, char * sKey, char * sVal)
{
	HKEY hKey, hArtKey;	
	TCHAR tcDir[40];
	TCHAR tcKey[40];
	TCHAR tcVal[40];
	ULONG Len = 40;
	bool bError=false;

	RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"Software",0,KEY_READ,&hKey);
	MultiByteToWideChar(CP_ACP,0,sDir,strlen(sDir)+1,(LPWSTR)tcDir,sizeof(tcDir)/sizeof(tcDir[0]));	
	int Err1=RegOpenKeyEx(hKey,tcDir,0,KEY_READ,&hArtKey);
	MultiByteToWideChar(CP_ACP,0,sKey,strlen(sKey)+1,(LPWSTR)tcKey,sizeof(tcKey)/sizeof(tcKey[0]));	
	if (RegQueryValueEx(hArtKey,tcKey,0,NULL,(LPBYTE)tcVal,&Len)!=ERROR_SUCCESS)
	{bError=true;};
	if (Len>40){Len=40;};
	for (int i=0; i<Len; i++)
	{sVal[i]=tcVal[i];};
	RegCloseKey(hArtKey);
	RegCloseKey(hKey);
	if (bError)
	{
		RegOpenKeyEx(HKEY_CURRENT_USER,L"Software",0,KEY_READ,&hKey);
		MultiByteToWideChar(CP_ACP,0,sDir,strlen(sDir)+1,(LPWSTR)tcDir,sizeof(tcDir)/sizeof(tcDir[0]));	
		int Err1=RegOpenKeyEx(hKey,tcDir,0,KEY_READ,&hArtKey);
		MultiByteToWideChar(CP_ACP,0,sKey,strlen(sKey)+1,(LPWSTR)tcKey,sizeof(tcKey)/sizeof(tcKey[0]));	
		if (RegQueryValueEx(hArtKey,tcKey,0,NULL,(LPBYTE)tcVal,&Len)!=ERROR_SUCCESS)
		{bError=true;};
		if (Len>40){Len=40;};
		for (int i=0; i<Len; i++)
		{sVal[i]=tcVal[i];};
		RegCloseKey(hArtKey);
		RegCloseKey(hKey);
	};
	return bError;
};
*/
void Sleeper(int Delay)
{
//	if (iSleepTime>0)
//	{Sleep(iSleepTime);};
};

void OpenWeb()
{

	ShellExecute(NULL, L"open", L"goweb.bat", NULL, NULL, SW_SHOWMINIMIZED);
};

void OpenYuPlaySite()
{

	ShellExecute(NULL, L"open", L"YuPlay\\replanish.bat", NULL, NULL, SW_SHOWMINIMIZED);
};

void OpenYuDownload()
{

	ShellExecute(NULL, L"open", L"YuPlay\\download.bat", NULL, NULL, SW_SHOWMINIMIZED);
};

void RunYuInstall()
{
	ShellExecute(NULL, L"open", L"YuPlay\\install_yuPlay_0.4.22.exe", NULL, NULL, SW_NORMAL);
};

int GetCompName(char * CompName)
{
	TCHAR chrComputerName[17];
	DWORD dwBufferSize = 17;
	if (GetComputerName(chrComputerName,&dwBufferSize))
	{
		for (int i=0; i<dwBufferSize; i++)
		{CompName[i]=chrComputerName[i];};
		return dwBufferSize;
	}
	else
	{return 0;};
};

void FileCopy(char *Name1, char *Name2)
{
	TCHAR tcName1[150], tcName2[150];
	MultiByteToWideChar(CP_ACP,0,Name1,strlen(Name1)+1,(LPWSTR)tcName1,sizeof(tcName1)/sizeof(tcName1[0]));
	MultiByteToWideChar(CP_ACP,0,Name2,strlen(Name2)+1,(LPWSTR)tcName2,sizeof(tcName2)/sizeof(tcName2[0]));
	CopyFile(tcName1,tcName2,false);
};

long int MakeKey(char * eMail)
{
	long int Result;
	//0
	Result=abs(8+eMail[0]+eMail[2]+eMail[3]-eMail[4]+eMail[7])%10;
	Result*=10;
	//1
	Result+=abs(3+eMail[1]+eMail[5]-eMail[3]-eMail[2]+eMail[8])%10;
	Result*=10;
	//2
	Result+=abs(2+eMail[6]*eMail[1]+eMail[3]+eMail[9])%10;
	Result*=10;
	//3
	Result+=abs(5+eMail[0]+eMail[5]-eMail[1]+eMail[10])%10;
	Result*=10;
	//4
	Result+=abs((eMail[2]-eMail[0])*eMail[3]+eMail[4])%10;
	Result*=10;
	//5
	Result+=abs(eMail[4]-eMail[6]-eMail[2]*eMail[3]+eMail[10])%10;
	Result*=10;
	//6
	Result+=abs(4+eMail[3]+eMail[5]-eMail[0]-eMail[7])%10;
	Result*=10;
	//7
	Result+=abs(5+eMail[5]+eMail[4]-eMail[3]+eMail[8])%10;
	Result*=10;
	//8
	Result+=abs(6+eMail[1]+eMail[2]+eMail[6]-eMail[9])%10;
//	Result*=10;
	//9
//	Result+=abs(1+eMail[3]+eMail[0]-eMail[2]-eMail[10])%10;
//	Result*=10;
	//10
//	Result+=abs(7+eMail[7]+eMail[8]+eMail[9]+eMail[0])%10;	

	return Result;
};