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

#ifndef __PPPCONFIG_H__
#define __PPPCONFIG_H__

//---------------------------------------------------------------------------
// pppconfig.h - pppconfig header file  									-
//---------------------------------------------------------------------------
// This will configure some of the tuneable paramters for the ppp engine.	-
//	
//---------------------------------------------------------------------------
// Version                                                                  -
//		0.1 Original Version July 16, 2002									-
//																			-
// (c)2001 Mycal Labs, All Rights Reserved									-
//---------------------------------------------------------------------------
#include "mTypes.h"
#include "ports.h"

#define IMM_VERSION				"0.7p"
//#define	DEMO					1

//#define MEGA128					1
//#define MEGA64					1
//#define MEGA162					1

// Let the compiler determine which AVR chip:
#if defined (__AVR_ATmega64__)
#define MEGA64						1
#elif defined (__AVR_ATmega128__)
#define MEGA128						1
#define	DEMO					1
#elif defined (__AVR_ATmega162__)
#define MEGA162						1
#define DEMO						1
#endif


#define CPU_CLOCK 				7372800     /* 7.3728Mhz -or- 7372800 */
#define	CONSOLE_BAUD_RATE		19200 		/* terminal baud rate 9600bps if no autobaud*/
#define MODEM_BAUD_RATE 		19200		/* Default Modem baud rate 9600bps if not set in eeprom*/
#define	CPU_CLOCK_DIV100			CPU_CLOCK/100
#define CONSOLE_BAUD_RATE_DIV100	CONSOLE_BAUD_RATE/100
#define MODEM_BAUD_RATE_DIV100		MODEM_BAUD_RATE/100

#define CONSOLE_BAUD_STATIC		(CPU_CLOCK_DIV100/(CONSOLE_BAUD_RATE_DIV100*16)-1)
#define MODEM_BAUD_STATIC		((CPU_CLOCK_DIV100/(MODEM_BAUD_RATE_DIV100*16))-1)

#define MODEM_BAUD_1200			(CPU_CLOCK_DIV100/(12*16)-1)
#define MODEM_BAUD_2400			(CPU_CLOCK_DIV100/(24*16)-1)
#define MODEM_BAUD_4800			(CPU_CLOCK_DIV100/(48*16)-1)
#define MODEM_BAUD_9600			(CPU_CLOCK_DIV100/(96*16)-1)
#define MODEM_BAUD_19200		(CPU_CLOCK_DIV100/(192*16)-1)
#define MODEM_BAUD_38400		(CPU_CLOCK_DIV100/(384*16)-1)
#define MODEM_BAUD_57600		(CPU_CLOCK_DIV100/(576*16)-1)
#define MODEM_BAUD_115200		(CPU_CLOCK_DIV100/(1152*16)-1)
#define MODEM_BAUD_230400		(CPU_CLOCK_DIV100/(2304*16)-1)


#define	MAX_RECEIVE_MTU			384




#ifdef __AVR__

#ifdef MEGA128						
//#include <iom128.h>
#endif
#ifdef MEGA64						
//#include <iom64.h>
#endif

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/interrupt.h>
//#include <signal.h>
//nclude <avr/signal.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#define FLASHMEM __attribute__ ((progmem))

/*
#ifdef MEGA128						
//#include <iom128.h>
#endif
#ifdef MEGA64						
//#include <iom64.h>
#endif


#include <avr/interrupt.h>
//#include <signal.h>
#include <avr/signal.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#define FLASHMEM __attribute__ ((progmem))
*/


//#define CPU_CLOCK 				7372800     /* 7.3728Mhz -or- 7372800 */
//#define USE_SERIAL_FLOW_CNTROL	1
//#define USE_AUTOBAUD			0
//#define USE_DCD					0
//#define USE_DTR					0
//#define	USE_TTLESCAPE			0
//#define USE_SREG_BAUD			1

#define printf				CONSOLE_puts
#define CRLF				"\r\n"
//#define	CRLF	"\r\n"
//
// Clock constants
//		
/*
#define	CPU_CLOCK_DIV100			CPU_CLOCK/100
#define CONSOLE_BAUD_RATE_DIV100	CONSOLE_BAUD_RATE/100
#define MODEM_BAUD_RATE_DIV100		MODEM_BAUD_RATE/100

#define CONSOLE_BAUD_STATIC		(CPU_CLOCK_DIV100/(CONSOLE_BAUD_RATE_DIV100*16)-1)
#define MODEM_BAUD_STATIC		((CPU_CLOCK_DIV100/(MODEM_BAUD_RATE_DIV100*16))-1)

#define MODEM_BAUD_1200			(CPU_CLOCK_DIV100/(12*16)-1)
#define MODEM_BAUD_2400			(CPU_CLOCK_DIV100/(24*16)-1)
#define MODEM_BAUD_4800			(CPU_CLOCK_DIV100/(48*16)-1)
#define MODEM_BAUD_9600			(CPU_CLOCK_DIV100/(96*16)-1)
#define MODEM_BAUD_19200		(CPU_CLOCK_DIV100/(192*16)-1)
#define MODEM_BAUD_38400		(CPU_CLOCK_DIV100/(384*16)-1)
#define MODEM_BAUD_57600		(CPU_CLOCK_DIV100/(576*16)-1)
#define MODEM_BAUD_115200		(CPU_CLOCK_DIV100/(1152*16)-1)
#define MODEM_BAUD_230400		(CPU_CLOCK_DIV100/(2304*16)-1)
*/
#else

#define WINDOWS			1

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <winbase.h>
#include <conio.h>
#include <time.h>
#include <stdio.h> 
#include <sys/timeb.h>

#define	CRLF	"\n"
#endif
//#include "mTypes.h"

//#include "debug.h"
//
// Configuration Parameters
//
#define PPP_RX_BUFFER_SIZE		128
#define PPP_TX_BUFFER_SIZE		64

//#define	LCP_RETRY_COUNT			12
#define LCP_TIMEOUT				4

//#define PAP_RETRY_COUNT			12
#define PAP_TIMEOUT				4

//#define	IPCP_RETRY_COUNT		12
#define	IPCP_TIMEOUT			4

#define ESCAPE_TIMEOUT			200
//#define IPCP_TX_TIMEOUT			5
//#define NEG_TX_TIME				2

void poll_serial(void);


#endif 

