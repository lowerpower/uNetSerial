#ifndef MTYPES_H
#define MTYPES_H
//																www.mycal.com
//---------------------------------------------------------------------------
// wserial.h - windows raw serial port include file							-
//---------------------------------------------------------------------------
// Version                                                                  -
//		0.1 Original Version Feb 21, 1999									-
//---------------------------------------------------------------------------
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <winbase.h>


HANDLE	OpenSerial(int);
int		CloseSerial(HANDLE);

int		SendBytes(HANDLE , char * , int );
int		SendByte(HANDLE, char);
int		GetByte(HANDLE);



#endif