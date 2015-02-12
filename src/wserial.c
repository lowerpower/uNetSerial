/*
 * Copyright (c) 2007 Mycal Labs <www.mycal.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


//																	www.mycal.net
//-------------------------------------------------------------------------------
// Simple Windows Serial I/O Library
//
// (c)1999 Mycal Labs 
//-------------------------------------------------------------------------------
#include <windows.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <winbase.h>

/*
dwComPort 
Indicates the number of the COM port to use. The value for this member can be 1, 2, 3, or 4. 
dwBaudRate 
Indicates the baud of the COM port. The value for this member can be one of the following: CBR_110 CBR_300 CBR_600  
CBR_1200	CBR_2400	CBR_4800  
CBR_9600	CBR_14400	CBR_19200  
CBR_38400	CBR_56000	CBR_57600  
CBR_115200	CBR_128000	CBR_256000  

dwStopBits 
Indicates the number of stop bits. The value for this member can be ONESTOPBIT, ONE5STOPBITS, or TWOSTOPBITS. 
dwParity 
Indicates the parity used on the COM port. The value for this member can be NOPARITY, ODDPARITY, EVENPARITY, or MARKPARITY. 
dwFlowControl 
Indicates the method of flow control used on the COM port. The following values can be used for this member: DPCPA_DTRFLOW  Indicates hardware flow control with DTR.  
DPCPA_NOFLOW  Indicates no flow control.  
DPCPA_RTSDTRFLOW  Indicates hardware flow control with RTS and DTR.  
DPCPA_RTSFLOW  Indicates hardware flow control with RTS.  
DPCPA_XONXOFFFLOW  Indicates software flow control (xon/xoff).  
*/

HANDLE
OpenSerial(int port, int rate)
{
	HANDLE			cHandle;
	DCB				dcb;
	COMMTIMEOUTS	cto;
	char			name[5];

	wsprintf(name, "COM%d",port);

	//printf("1200 %d 9600 %d 256000 %d\n",CBR_1200,CBR_9600,CBR_256000);

	cHandle = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | 0, NULL); 

	if ((int)cHandle!=-1)
	{
		if (GetCommState(cHandle, &dcb))
		{
			dcb.DCBlength = sizeof(DCB);
			dcb.BaudRate = rate;//CBR_9600;
			dcb.ByteSize = 8;
			dcb.Parity = NOPARITY;			//0
			dcb.StopBits = ONESTOPBIT;		//1
			dcb.fOutX = FALSE;                // No XON/XOFF out flow control
			dcb.fInX = FALSE;                 // No XON/XOFF in flow control
			dcb.fNull = FALSE;                // Disable null stripping.
			//dcb.fBinary = TRUE;               // Binary mode; no EOF check
			//dcb.FlowControl=4;
			dcb.fRtsControl = 1;//RTS_CONTROL_ENABLE;	// I think this just locks RTS ON
			dcb.fDtrControl = 1;//DTR_CONTROL_ENABLE;
	//		dcb.fOutxCtsFlow = 1;         // CTS output flow control
	//		dcb.fOutxDsrFlow = 1;         // DSR output flow control
	        dcb.fOutxCtsFlow = dcb.fOutxDsrFlow = TRUE;



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
	unsigned char		cByte;
	DWORD				dwBytesReceived = 0;

	if (!ReadFile(cHandle, &cByte, 1, &dwBytesReceived, NULL))
		return -1;

	if (dwBytesReceived)
		return(cByte);
	else
		return-1;
}


