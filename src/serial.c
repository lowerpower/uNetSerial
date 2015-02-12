
//																	www.mycal.com
//-------------------------------------------------------------------------------
// Simple Windows Serial I/O Library
//
// (c)1999 Mycal Labs 
//-------------------------------------------------------------------------------
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <winbase.h>


HANDLE
OpenSerial(int port)
{
	HANDLE			cHandle;
	DCB				dcb;
	COMMTIMEOUTS	cto;
	char			name[5];

	wsprintf(name, "COM%d",port);

	cHandle = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
		OPEN_EXISTING, 0, NULL); 

	if (cHandle!=-1)
	{
		if (GetCommState(cHandle, &dcb))
		{
			dcb.DCBlength = sizeof(DCB);
			dcb.BaudRate = CBR_115200 ;
			dcb.ByteSize = 8;
			dcb.Parity = NOPARITY;			//0
			dcb.StopBits = ONESTOPBIT;		//1

			SetCommState(cHandle, &dcb);

			GetCommTimeouts(cHandle, &cto);
			cto.ReadIntervalTimeout = MAXDWORD;
			cto.ReadTotalTimeoutMultiplier = 0;
			cto.ReadTotalTimeoutConstant = 0;
			SetCommTimeouts(cHandle, &cto);
		}
	}

	return cHandle;
}




int
CloseSerial(HANDLE cHandle)
{
BOOL ret;

	if (!cHandle)
		return -1;

	ret = CloseHandle(cHandle);

	return ret ? 0 : -1;
}

int
SendBytes(HANDLE cHandle, char * Bytes, int Count)
{
	int	dwBytesWritten;
	
	if (!cHandle)
		return -1;

	if (!WriteFile(cHandle, (LPCVOID)Bytes, (DWORD)Count, &dwBytesWritten, 
		NULL) || Count != dwBytesWritten)
	{

		return -1;
	}

	return 0;
}

int
SendByte(HANDLE cHandle, char byte)
{
	int	dwBytesWritten;

	if (!cHandle)
		return -1;

	if (!WriteFile(cHandle, (LPCVOID)&byte, (DWORD)1, &dwBytesWritten, 
		NULL) || 1 != dwBytesWritten)
	{

		return -1;
	}

	return 0;
}



int
GetByte(HANDLE cHandle)
{
	char		cByte;
	DWORD		dwBytesReceived = 0;

	if (!ReadFile(cHandle, &cByte, 1, &dwBytesReceived, NULL))
		return -1;

	if (dwBytesReceived)
		return(cByte);
	else
		return-1;
}


