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

/*															
 *---------------------------------------------------------------------------
 * sreg.c - S Register Processor											-
 *	All S register commands are handled here								-
 *---------------------------------------------------------------------------
 * Original Version                                                         -
 *  Nov 7 2000	- mwj                                                       -
 *                                                                          -
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2000, Mycal Labs, All Rights Reserved.	        			_
 *                                                                         	_
 *---------------------------------------------------------------------------
 * Changelog
 *
 *---------------------------------------------------------------------------
*/
#include "pppconfig.h"
#include "mTypes.h"
#include "imm.h"            
#include "ircmd.h"
#include "mip.h"				// IP addresses
#include "mtcp.h"				// tcp status
#include "sreg.h"   
#include "modem.h"
#include "console.h"
#include "conio.h"
#include "mtcp.h"  
#include "udpcmd.h"
#include "msg.h"       
#include "globals.h"
#include "debug.h"

/*
U8
*Get_Byte(U8 *ptr, U8 *val)
{
	//
	// Dump Spaces
	//
	while(*ptr==' ')
			ptr++ ;

	smbuff[0]=(*ptr++ | 0x20);
	smbuff[1]=(*ptr++ | 0x20);
	smbuff[2]=0;
	
	if('='==smbuff[1])
	{
		ptr--;
	}

	// Convert
	*val=X2s(&smbuff[0]); 
	return(ptr);
}
*/


//
//		0-->N				Config Register Memory Map in Hex
//
//		e					EEPROM
//
//		x					raw S7600 read/write
//
//		b					S7600 interface baud rate
///		0-1					Socket Status
//      2-3					Socket Source Port
//      4					Our IP address
//		5					DNS server pri
//		6					DNS server sec
//      7					Modes
//
U8
sRegister(U8 *ptr)
{
U8		msg=0xff; 
U8		soc=1,s=1;
U16		t=0;
U8		*tptr=(U8 *)&mm;
//U16     t;
IPAddr 	ip;
	//
	// Dump Spaces
	//
	while(*ptr==' ')
			ptr++ ;
	//
	// Decode 
	//
	ptr=Get_Byte(ptr,&s);

#ifdef DEBUG_LV4	
	DEBUG2("sreg=");
	printShort(s);
	DEBUG2(CRLF);
#endif
	//
	// Dump Spaces
	//
	while(*ptr==' ')
			ptr++ ;	

	if('='==*ptr++)
		t=1;
	//
	// Operate on sreg
	//
	switch(s)
	{
		case 1:
			// Return socket 1 status      
			soc=2;
		case 0:
			// Return socket 0 status   
			//
			// See if this socket is up,  Returns socket state and source port
			//		
			t=tcp_status(soc);
			printByte((U8)t);
			CONSOLE_putch(' ');
			t=tcp_sport(soc);
			printWord(t);
			break;	
		case 2:
			//
			// Return UDP status,   Returns Len of data ready, peer ip, peer source port
			//
			if(0==udp_ptr)
				printShort(0);
			else
				printShort(1);
			CONSOLE_putch(' ');
			printShort(udp_len);
			CONSOLE_putch(' ');
			printIP(&udp_ip);
			CONSOLE_putch(' ');
			printShort(udp_sp);
			break;
		case 3:
			//
			// Print SW version
			//
			msg=MSG_VERSION;
			break;
		case 4:
			//
			// Print boot loader Version
			//
#ifdef __AVR__	
#ifndef DEMO
			t=boot_version();
#else
			t=1;
#endif
#else
			t=1;
#endif
			printWord(t);
			//msg=MSG_NULL;
			break;
		case 5:
			//
			// Our IP address
			//                    
			// Check Read or Write
			//                    
			if(1==t)
			{
				// Convert the IP address    
				if(ptr!=(U8 *)s2ip(ptr, &ip ))
				{         
					mm.our_ipaddr.ipb1=ip.ipb1;
					mm.our_ipaddr.ipb2=ip.ipb2;
					mm.our_ipaddr.ipb3=ip.ipb3;
					mm.our_ipaddr.ipb4=ip.ipb4;
				}			
			}
			printIP(&mm.our_ipaddr);
			break;
		case 6:
			//
			// Primary DNS
			//                    
			// Check Read or Write
			//                    
			if(1==t)
			{
				// Convert the IP address    
				if(ptr!=(U8 *)s2ip(ptr, &ip ))
				{         
					mm.pri_dns_addr.ipb1=ip.ipb1;
					mm.pri_dns_addr.ipb2=ip.ipb2;
					mm.pri_dns_addr.ipb3=ip.ipb3;
					mm.pri_dns_addr.ipb4=ip.ipb4;
				}			
			}	
			printIP(&mm.pri_dns_addr);
			break;
		case 7:
			//
			// Secondary DNS
			//                    
			// Check Read or Write
			//                    
			if(1==t)
			{
				// Convert the IP address    
				if(ptr!=(U8 *)s2ip(ptr, &ip ))
				{         
					mm.sec_dns_addr.ipb1=ip.ipb1;
					mm.sec_dns_addr.ipb2=ip.ipb2;
					mm.sec_dns_addr.ipb3=ip.ipb3;
					mm.sec_dns_addr.ipb4=ip.ipb4;
				}			
			}
			else
				printIP(&mm.sec_dns_addr);
			break;
		case 8:
			// Modem Baud Rate
			if(1==t)
			{
				if(tptr!=(U8 *)s2p(ptr,&t))
				{
					//printShort(t);
					//DEBUG1(CRLF);
					mm.ModemBaud=(U8)t;
#ifndef WINDOWS
					MODEM_init(t);
#endif
				}
				else
					msg=MSG_ERROR;
			}
			if(msg!=MSG_ERROR)
			{
				printShort(mm.ModemBaud);
			}
			break;
		case 9:
			// Console Baud Rate
			if(1==t)
			{
				if(tptr!=(U8 *)s2p(ptr,&t))
				{
					//printShort(t);
					//DEBUG1(CRLF);
					mm.ConsoleBaud=(U8)t;
#ifndef WINDOWS
					MODEM_init(t);						// bug! UART0_Init(U16 Baud_Rate )
#endif
				}
				else
					msg=MSG_ERROR;
			}
			if(msg!=MSG_ERROR)
			{
				printShort(mm.ConsoleBaud);
			}
			break;
#ifdef __AVR__	
		case 0xa:
			// 
			// PortF GPIO Direction Pin
			//
			if(1==t)
			{
				Get_Byte(ptr,&soc);
				DDRF=soc;				//			outp(soc,DDRF);
			}
			soc=DDRF;					//			soc=inp(DDRF);
			printByte(soc);
			break;
		case 0xb:
			// 
			// PortF GPIO pin  PINF? +++
			//
			if(1==t)
			{
				Get_Byte(ptr,&soc);
				PORTF=soc;				//			outp(soc,PORTF);
			}
			soc=PORTF;					//			soc=inp(PORTF);
			printByte(soc);
			break;
#endif
		default:
			if(s<(sizeof(MEMMAP)+0x20+1) && (s>=0x20))
			{
				//
				// now see if it is set or display
				//
				if(1==t)
				{
					// Write to S-Register
					Get_Byte(ptr,&soc);
					tptr[s-0x20]=soc;
				}
				// Display register value always
				printByte(tptr[s-0x20]);
				msg=MSG_EMPTY;
			}
			else
			{
			//	DEBUG1("SREG is out of range"CRLF);
				msg=MSG_ERROR;
				break; 
			}
	}
	return(msg);
}


