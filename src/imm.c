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

/*																www.nchip.com
 *---------------------------------------------------------------------------
 * imm.c - ir command processor	version 2									-
 *---------------------------------------------------------------------------
 * Version                                                                  -                                            
 *		0.1 Original Version AUG 21, 2002									-        
 *
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2002, nChip/Mycal Labs, All Rights Reserved.				_
 *															               	_
 *---------------------------------------------------------------------------
*/
#include "pppconfig.h"
#include "mTypes.h"

#ifdef WINDOWS
#include <signal.h>
#include <time.h>
#endif

//#include "wserial.h"
#include "ircmd.h"
#include "mip.h"			// IP address is here
#include "console.h"
#include "modem.h"
#include "memory.h"

#include "time.h"
#include "globals.h"

#include "autobaud.h"
#include "ports.h"
#include "globals.h"
#include "debug.h"

//
// Boot Loader Call
//
#ifdef MEGA128
short (*boot_version)(void)=(void *)0xfE03;
int (*crc_boot)(void)=(void *)0xfE06;
int (*bootloader)(U16 baud)=(void *)0xfE00;
#endif

#ifdef MEGA64
int (*boot_version)(void)=(void *)0x7003;
int (*crc_boot)(void)=(void *)0x7006;
int (*bootloader)(U16 baud)=(void *)0x7000;
#endif

//
// 
//
U8	smbuff[16];


void
setup_hw(void)
{
#ifdef __AVR__
//		DDRA=PORTADIR;
	DDRB=PORTBDIR;				//	outp(PORTBDIR,DDRB);
	DDRC=PORTCDIR;				//  outp(PORTCDIR,DDRC);
	DDRD=PORTDDIR;				//	outp(PORTDDIR,DDRD); 
	DDRE=PORTEDIR;				//	outp(PORTEDIR,DDRE);
	DDRG=PORTGDIR;				//	outp(PORTGDIR,DDRG);

	// weak pullups on inputs
	PORTB=~PORTBDIR;			//	outp(~PORTBDIR,PORTB);
	PORTC=~PORTCDIR;			//	outp(~PORTCDIR,PORTC);
	PORTD=~PORTDDIR;			//	outp(~PORTDDIR,PORTD);
	PORTE=~PORTEDIR;			//	outp(~PORTEDIR,PORTE);
	PORTG=~PORTGDIR;			//	outp(~PORTGDIR,PORTG);
	
#endif
}


#ifdef WINDOWS

void
eexit(void)
{
	printf("**********");
	MODEM_close();
	exit(1);
}

void
main(argc,argv)
int 	argc;
char 	**argv;
{
int		port=1;
int		speed=19200;
//char	t[10];
U16		crc=0xffff;

char	t[] = {1,2,3,4,5,6,7,8,9};
	signal(SIGINT,eexit);


	for(port=0;port<9;port++)
	{
		crc=crcadd(crc, t[port]);
		printf("CRC after adding %d is %d\n",t[port],crc);
	}

	while(1);

	if (argc==3)
	{   
		// Get com port
		port=atoi(argv[1]);
		// Get com speed
		speed=atoi(argv[2]);
    }
	else
	{
		printf("Warning: must specify both com port and speed to not use defaults.\n");
		printf("	Correct format is ppp <port> <speed>\n");
		printf("	Example: ppp 2 19200\n\n");
	}


#else

int
main()
{
int	port=1;
int c;

//eeprom_write_byte (0,0);
#endif

	// Setup the hardware pins
	setup_hw();
	// Initialize the Timer
	TIMER_Init();
	// Enable the interrupts
	sei();
	// Intialize the config from EEPROM or defaults
	global_init(1);

	// If set to autobaud, autobaud the Console Serial Port and save it to mm
#ifdef __AVR__	

#ifdef DEMO
	c=mm.ConsoleBaud;
#else
	if(mm.SerialConfig & AUTOBAUD)
	{
		c=autobaud_setup();
		mm.ConsoleBaud=c;
	}
	else
	{
		// Load default baud/ no autobaud
		c=mm.ConsoleBaud;
	}
#endif
	// Initialize the Console Port 
	CONSOLE_init(c);
	//CONSOLE_init(CONSOLE_BAUD_STATIC);
#endif

#ifdef __AVR__
#ifdef DEBUG_LV3	
	CONSOLE_puts("autobaud = ");
	printShort(c);
	DEBUG1(CRLF);
#endif
#endif

#ifdef DEMO
	//
	// Print the hello message
	//
	TIMER_Wait_Tick(30);
	if(mm.IMMConfig & BOOT_BANNER)
	{
		CONSOLE_puts("uNetSerial (DEMO), Embedded internet TCP/IP stack over PPP, Beta - Version "IMM_VERSION CRLF);
   		CONSOLE_puts("	-(c)2001-2004 nChip" CRLF);
		CONSOLE_puts("	-Beta Version build on " __DATE__ " at " __TIME__ CRLF);
		CONSOLE_puts("	-See www.nchip.com for More info."CRLF);
	}
	TIMER_Wait_Tick(60);
#else

	//
	// Print the hello message
	//
	//CONSOLE_init(mm.ConsoleBaud);
	if(mm.IMMConfig & BOOT_BANNER)
	{
		TIMER_Wait_Tick(20);
		CONSOLE_puts("uNetSerial, Embedded internet TCP/IP stack over PPP, Beta - Version "IMM_VERSION CRLF);
		CONSOLE_puts("	-Beta Version build on " __DATE__ " at " __TIME__ CRLF);
	}
	TIMER_Wait_Tick(20);
#endif

#ifdef __AVR__	
#ifndef DEMO
	//
	// This is where the AVR bootloader checker is, we should also put the CRC routing
	// in the bootloader since it will never change.
	//
	if(mm.IMMConfig & BOOT_BANNER)
	{
		CONSOLE_puts("	-Bootloader Version ");
		c=boot_version();
		printWord(c);
		CONSOLE_puts(CRLF);
	}
#endif
#endif
	/* Initializer Modem Port */
#ifdef WINDOWS
	MODEM_init(port,speed);//MODEM_BAUD_RATE);
#else
#ifdef DEBUG_LV4
	DEBUG2("Initialize Modem Serial Port at ");
	printShort(mm.ModemBaud);
	DEBUG2(CRLF);
#endif
	MODEM_init(mm.ModemBaud);
#endif

	// Initialize memory
	DEBUG2("Initialize Memory Pool"CRLF);
	MEM_init();
	
	//
	// Enable interrupts and flash Carrier Detect pin on serial iface
	//
	//MODEM_RST_ON();
	DTRI_ON();
	TIMER_Wait_Tick(20);
	DTRI_OFF();
	TIMER_Wait_Tick(20);
	DTRI_ON();
	//MODEM_RST_OFF();
	TIMER_Wait_Tick(20);
	//
	// Initialize user serial port
	//
	DEBUG2("Starting irCommand Processor"CRLF);

/*
	while(1)
	{
		U16 temp;
		temp=TIMER_Get_TCP_HundredMS();
		CONSOLE_puts("hunms timer= ");
		printShort(temp);
		CONSOLE_puts(CRLF);
	}
*/

    /* Start the irCommand Processor, runs forever */
	irCommands();
}


