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
 * udpcmd.c - udp command processor											-
 *	All UDP commands are handled here										-
 *---------------------------------------------------------------------------
 * Original Version                                                         -
 *  Oct 5, 2003	- mwj                                                       -
 *                                                                          -
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2003, nChip/Mycal Labs										_
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
#include "mudp.h"				// tcp status
#include "msg.h"       
#include "console.h"
#include "conio.h"
#include "time.h"
#include "ports.h"
#include "globals.h"
#include "utils.h"
#include "debug.h"

//#define	UDP_ALLOCATE		'U'				//	U	=	AllocateUDP
//#define UDP_CLOSE			'V'				//	V	=	Close UDP
//#define	UDP_GET				'G'				//	G	=	GetUDP
//#define UDP_PUT				'P'				//	P	=	PutUDP
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
#define	UDP_BIND				'B'
#define	UDP_CLOSE				'C'
#define UDP_GET					'G'
#define UDP_PUT					'P'

//static U16			udplen;
U16					udp_sp;
IPAddr				udp_ip;
U16					udp_len;
MEMBLOCK			*udp_ptr;
U8					udp_active;

//
// UDP Socket Calback.
//
void 
udp_socket_rx(IP *header,UDP *udp, U16 len)
{
//MEMBLOCK	*memblock;

	DEBUG2("UDP socket callback"CRLF);

	//
	// If there is no current UDP buffer waiting, hook it up as
	//	ready to be sent. else just discard packet.
	//
	if(udp_ptr==0)
	{
		// Get a buffer
		udp_ptr=MEM_GetBlock();
		if(udp_ptr)
		{
			DEBUG2("UDP hookup"CRLF);
			udp_sp=htons(udp->source_port);
			udp_ip.ip32=header->source_ip.ip32;
			udp_len=htons(udp->length)-sizeof(UDP);
			if(udp_len>128)
				udp_len=128;
			bytecopy(udp_ptr->data,udp->data,udp_len);
			//memcpy(udp_ptr->data,udp->data,udp_len);	
			
			// Turn on RI to signify DAV
			RI0_ON();
		}
	}
}



//
// udpcmd - handle UDP commands here, can be (B)ind, (C)lose, (G)et, (P)ut
//
U8
udpcmd(U8 *ptr)
{
U8		*tptr;
U16		sp,tp;
IPAddr 	ip;
U8		msg=MSG_ERROR;
	//
	// Dump Spaces
	//
	while(*ptr==' ')
			ptr++ ;

	switch(toupper(*ptr++))
	{
	case	UDP_BIND:		// 'B'     
		while(*ptr==' ')
		ptr++ ;
		//
		// Parse port #
		//   
		tptr=ptr;
		if(tptr!=(U8 *)s2p(ptr,&tp))
		{
			//
			// If udp_active exists close it first
			//
			if(udp_active)
				udp_close(udp_active);
			//
			// Set socket to UDP mode and port number
			//  
			udp_active=udp_bind(tp, (void *)&udp_socket_rx);
			if(udp_active)
				msg=MSG_OK;
			else
				msg=MSG_SOCDOWN;
		}				            
		break;
	case	UDP_CLOSE:
		udp_close(udp_active);
		udp_active=0;
		msg=MSG_OK;
		break;
	case	UDP_GET:
		//
		// If data dump it.
		//
		if(udp_ptr!=0)
		{
			msg=MSG_OK;
			displayMessage(msg);
			printIP(&udp_ip);
			CONSOLE_puts(CRLF);
			printShort(udp_sp);
			CONSOLE_puts(CRLF);
			printShort(udp_len);
			CONSOLE_puts(CRLF);				
			for(tp=0;tp<udp_len;tp++)
				CONSOLE_putch(udp_ptr->data[tp]);
			CONSOLE_puts(CRLF);
			MEM_FreeBlock(udp_ptr);
			udp_ptr=0;
			udp_len=0;
			msg=MSG_NULL;
		}
		else
			msg=MSG_NODATA;
		break;
	case	UDP_PUT:     
		while(*ptr==' ')
		ptr++ ;
		//
		// send a udp packet IRP server:dport:sport
		//
		if(0==isalnum(*ptr))
		{
			msg=MSG_ERROR;
		}
		else if(Get_IP(ptr,&ip))
		{
			//
			// Advance pointer over name and extract port
			//
			while((*ptr!=':') && (*ptr!=0))
				ptr++;  
			ptr++;
			//
			// Get Dest Port
			//
			ptr=s2p(ptr,&tp);
			if(tp>1)
			{           
				//
				// Get Source Port or use current allocaed or zero
				//
				if(':'==*ptr)
				{
					ptr++;
					// Get Source Port
                   	ptr=s2p(ptr,&sp);
				}
				else
				{
					// use current socket or zero
					sp=udp_port(udp_active);
				}
				//
				// Get Ready to Send
				//
				msg=MSG_OK;
				displayMessage(msg);
				//
				// collect byte to send
				//
				snd_count=0;
				TCPSendTime=TIMER_Get_Tick();  
				while(1)
				{
					poll_serial();
					//
					// Check for full snd buffer
					//
					if(snd_count >= MAX_SND_SIZE)
						break;
					//
					// Check for timeout
					//
					if(abs(TIMER_Get_Tick()-TCPSendTime) > mm.UDPStreamTickTime)
						break;
					if(CONSOLE_kbhit())
					{
						DEBUG3("kbhit"CRLF);
						// get the char from the USER		
						msg=CONSOLE_getch();

						if(mm.IMMConfig & ECHO_STREAM)
							CONSOLE_putch(msg);

						// Store char in buffer
						snd_buff[snd_count++]=msg;
						// Reset the time
						TCPSendTime=TIMER_Get_Tick();  
					}
				}
								
				if(udp_tx(ip, tp, sp, snd_buff, snd_count))
				{
					msg=MSG_OK;
					CONSOLE_puts(CRLF);
				}
				else
					msg=MSG_FAIL;
						
				snd_count=0;
			}
			else
				msg=MSG_ERROR;
		}
		else
		{
			msg=MSG_DNSFAIL;
		}		
		break;
	} // endcase
	return(msg);
}


