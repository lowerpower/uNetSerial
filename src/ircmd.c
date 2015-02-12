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
 * ircmd.c - ir command processor											-
 *---------------------------------------------------------------------------
 * Version                                                                  -                                            
 *		0.1 Original Version June 23, 2000									-        
 *
 *  Rewrite by mwj Nov 7 2000
 *
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2000, mycal labs, All Rights Reserved.	        			_
 *																			_
 *---------------------------------------------------------------------------
 *
 *
 *11/24/04 - mwj - sanity check
 *2/12/02 - mwj - added IRO command to disable command processor.
 *
*/

/*					*/ 
/* include files 	*/
/*					*/    
#include "pppconfig.h"
#include <ctype.h>
#include <stdlib.h>
#include "mTypes.h"
#include "imm.h"
#include "ppp.h"
#include "time.h"
#include "resolve.h"  
#include "sreg.h"
#include "ppp.h"       
#include "msg.h"    
#include "ports.h"   
#include "mudp.h"
#include "ircmd.h"   
#include "udpcmd.h"
#include "console.h"
#include "modem.h"
#include "globals.h"
#include "utils.h"
#include "debug.h"

//#define ESCAPE_TIMEOUT 200 //two seconds
                                                           
//                                                          
#if MODEM_INIT
static const char MODEM_INIT_STRING[] = "ats21=113\r\n";
#endif

U8			ircmd_state;      
static U8	carrier;

//
// Id almost think we would want 2 of these buffers and ping pong them, but
//	for now lets just use one.  Might want to make it bigger, but for
//  most small stuff this should be fine.
//
#define 			IR_BUF_LEN					64
char			ircmd_buf[IR_BUF_LEN+2];  


#define				MAX_SOCKETS					2
U8					soc_num;
U8					active_soc;
U8					sockets[MAX_SOCKETS];
//U8					udp_active;
U8					dtrflag;					// could compress this into another byte


static	U8			TCPSendFlag;
		U8			snd_buff[MAX_SND_SIZE];
		U16			TCPSendTime;
		U16			snd_count;
        
            
static U8			ccount=0; 
static U8			ircount=0;
//
//
static	U16			escape_tick;
static	U8 			escapeCount;
static	U8 			escapeTime;
		U8			tmpflag;

//
//
//
void
EscapeReset(void)
{
	escape_tick=TIMER_Get_Tick();
	escapeCount=0;
	escapeTime=0;
}


//
// return:
//			0xff	eat char
//			0		do nothing
//			1,2,3	unbuffer # of bytes
//
U8
EscapeSequence(U8 c)
{       
U8	ret=0;

	if((mm.IMMConfig & INBAND_ESCAPE)==0)
	{
		if((escapeTime>0) && (c==mm.escapeChar))
		{	
			escapeCount++;   
			
			if(escapeCount>3)
				ret=escapeCount;
			else
				ret=0xff;
				
			if(escapeCount==3)   
			{
				escapeTime=2;			
				escape_tick=TIMER_Get_Tick();
			}
		}	        
	    else
    	{
			if(escapeCount>0)
				ret=escapeCount;
		}		
	
		if(ret!=0xff)
		{			
			EscapeReset();
		}
	}			
	return ret;
}               


//
// +++ may need to change ESCAPE_TIMEOUT to variable
//
void
EscapeTime(void)
{   
U16	tick=TIMER_Get_Tick();
	
	if((escapeTime==0) || (escapeTime==2))
	{
		if(abs(escape_tick - tick) > mm.escape_timeout /*ESCAPE_TIMEOUT*/)    
			escapeTime++;
	}
}                


//
// Check if we have an inband or out of band escape (+++ or DTR toggle)
//		We probably should check config to see if we allow DTR toggle`
// 
U8 
EscapeAction(void)
{    
U8	ret=0;
	// Check DTR esc, only do it on a low to high transistion,
	// when dtr is low set dtrflag to 0 so we know when it first
	// transitions low to high.
	//
	if(DTR0())
	{
		if(dtrflag==0)
			ret=1;
		dtrflag=1;
	}
	else
		dtrflag=0;

	//
	// Check inband escape, need to check cfg flag?
	//
	EscapeTime();
	if((escapeCount==3) && (escapeTime==3))
		ret=1;

  return(ret);
}
 

//
// pppConnect(U8 *ptr) - irCommand wrapper for the raise_ppp function
//
//	Pointer can possibly point to a username,password string.  Username
//	and password are seperated by a comma.
//
//
U8
pppConnect(U8 type,U8 *ptr)
{
U8		msg;
U8		*username,*password;

	//
	// Only try if we have CD on the modem port, for now we don't check 
	//
#if USE_DCD
//	if(readx(IR_PORT_STAT_REG) & IR_PORT_DCD)
#else
//	if(readx(IR_PORT_STAT_REG) & IR_PORT_DCD & 0)
#endif
	if(0)
	{   
		msg=MSG_NOCARRIER;
	}
	else				
	{                                     
		//
		// Set a username and password if available, else set to NULL.
		//                                   
		// First Dump Spaces
		//
		while(*ptr==' ')
			ptr++ ;				
		//
		// Extract Username and Password if available     
		//	
		if(0==*ptr)
		{
			*ptr=0;
			username=password=ptr; 
		}
		else
		{
			// Looks like a username
			username=ptr;
			// Find the comma
			while((*ptr!=',') && (*ptr!=0))
			{
				ptr++;
			}  
			if(','==*ptr)
			{
				// We have comma, make it a null, and advance pointer over null
				*ptr=0;
				ptr++;		
			}
			password=ptr;
		}
		//
		// Try to raise PPP.
		//
	//	DEBUG3("username %s, password %s....\n",username,password);
		if(0==type)
			msg=raise_ppp(0, username, password);
		else
			msg=ppp_auto_enable(username, password);
	}
	DEBUG3("outpppup "CRLF);
	return(msg);
}




//
// tcp_socket_rx) - TCP socket RX handler
//
//	How this works: this is called when the TCP statemachine
//	has a segment of data to pass to the application.  The
//	data and length are passwd along with the socket ID it was
//	received on,  the application processes the data and returns
//	the number of bytes that it processed.  The TCP stack will send
//	an ACK based on the number of bytes processed.
//
//S16 (*callback)(U8 soc,U8 *data, U16 len)
//
S16
tcp_socket_rx(U8 soc, U8 * data, U16 len)
{
U16	i;
	//
	// Check to see if the active socket is the socket rx'in the data.
	//
	DEBUG2("TCP socket callback"CRLF);

	//
	// Make sure incoming packet belongs to current streaming socket
	//

	//
	// Make sure we are active in stream socket mode, else dump (send ack
	//	when entering back into stream socket mode to kick stuff back off.
	//


	// Turn on RI to signify DAV
	RI0_ON();
	
	// Dump the data	
	for(i=0;i<len;i++) 
	{
		//poll_serial();						// +++ experimental V0.7j this was bad experiment
		CONSOLE_putch(data[i]);
	}
	// !DAV - this will be turned off in main loop
	//RI0_OFF();


// just eat for now.
return(len);
	//
	// Else not active socket, here we return that no bytes were processed
	// this will cause no ACK to be returned, causing the peer to retransmit
	// (kinda like buffering it at the peer).  But when we select this socket
	//	again we will force an ack to get the peer to retransmit if it is
	// in a high retransmit time state.
	//


}



void 
icmp_echo_rx(IP *header,ICMP *icmp, U16 len)
{
	DEBUG2("ICMP Echo Reply"CRLF); //TCPSendTime
	CONSOLE_puts("OK - ");
	printShort((U16)(TIMER_Get_Tick()-TCPSendTime));
	CONSOLE_puts("0 ms");
	TCPSendTime-=1000;
	tmpflag=0;
}



U8
Get_IP(U8 *ptr, IPAddr *ip)
{

	if((*ptr>='0') && (*ptr<='9'))
	{                   
		// Parse and write the IP address 
		s2ip( ptr, ip );
	}
	else
	{           
		// Do DNS Resolution on passed name      
		if(resolve_name(ip, ptr)!=RET_CODE_OK)
		{          
			//dns failed
			return(0);
		}
    }
	return(1);
}

//
// Commands :
//					
#define SYNC_CHAR_1  		'I'
#define SYNC_CHAR_2  		'R'  

#define IMM_NULL			0				//  no command
#define PPP_CONNECT      	'C'				//	C	=	Connect			(irc1username,password)
#define IMM_DISABLE     	'D'				//	D	=	Disable  
#define TCP_CONNECT   		'T'				//	T   =	TCPconnect
#define TCP_RESUME    		'R'				//	R   =	Resume
#define HTTP_GET			'H'				//  H   =	HTTP get
#define IMM_FTP				'F'				//  F	=	FTP Get and Put
#define SOCKET_TERMINATE 	'X'				//	X   =	Release 
#define	UDP_ALLOCATE		'U'				//	U	=	AllocateUDP
#define UDP_CLOSE			'V'				//	V	=	Close UDP
#define	UDP_GET				'G'				//	G	=	GetUDP
#define IMM_PING			'P'				//	P	=	ICMP Ping
#define REGISTER_ACCESS  	'S'				//	S	=	Set/Show
#define RESOLVE_NAME		'N'				//	N	=	Resolve Name
#define IMM_OFF				'O'				//  O	=	OFF
#define MODEM_DTR_TOGGLE	'M'				//  M	=	Toggle Modem DTR
#define IMM_BOOTLOAD		'B'				//  B	=	Enter Boot Loader
#define EEPROM_CMD			'E'				//  E	=	EEPROM CMD
#ifdef WINDOWS
#define IMM_EXIT			'!'				//  !	=	Exit windows only
#endif

#define	IMM_STATE			'Z'				// TEMP DEBUG


//
//
//
U8
*SocketSelect(U8 *ptr)
{

	if((*ptr=='1') || (*ptr=='0'))
	{           
		//
		// Set the Current Socket, convert ASCII to bin
		//
		soc_num=*ptr-'0';
		ptr++;	 
#ifdef DEBUG_LV4
		DEBUG2("Socket Selected ");
		printByte(soc_num);
		DEBUG2(CRLF);
#endif		
		//
		// Gobble Spaces
		//
		while(*ptr==' ')
			ptr++ ;
	}
     
	return(ptr);     
}

void
modem_toggle_dtr(void)
{
	DTR1_OFF();			             
	TIMER_Wait_Tick(5);
	DTR1_ON();
}

//
// irCommandProcessor
//
//
void
irCommandProcessor(void)
{       
U8		msg=MSG_ERROR;
U8		*tptr;    
U8		*ptr=ircmd_buf;  
U16		tp,sp;
IPAddr 	ip;
                    
	ptr++;                    
	if((*ptr++ & 0xdf )==SYNC_CHAR_2)
	{            
		//
		// Get cmd
		//
		msg=(*ptr++ & 0xdf);
		//
		// Dump Spaces inbetween cmd and first paramter
		//
		while(*ptr==' ')
			ptr++ ;					
		switch(msg)
		{
			case	IMM_FTP:
			case	HTTP_GET:
				break;
			case	IMM_PING:
				// test ping, firt parse IP
				//
				if(0==isalnum(*ptr))
				{
					msg=MSG_ERROR;
				}
				else if(Get_IP(ptr,&ip))
				{
					// Set a flag to insure reply or error msg
					tmpflag=1;
					// Set callback,
					ICMP_Echo_Callback((void *)&icmp_echo_rx);
					// Send Ping of length 16
					ICMP_Send_Ping(&ip,56);
					// Wait
					TCPSendTime=TIMER_Get_Tick();
					while((U16)(TIMER_Get_Tick()-TCPSendTime)<400)
						poll_serial();
					// Clear callback		
					ICMP_Echo_Terminate();
					//
					// Ping reply?
					//
					if(tmpflag!=0)
						msg=MSG_FAIL;
				}
				else
					msg=MSG_DNSFAIL;
				break;
			case	IMM_NULL:
				msg=MSG_OK;
				break;
			case	IMM_STATE:
				b2X(ircmd_state, smbuff);
				CONSOLE_puts(smbuff);	
				msg=0xff;					
				break;
			case	PPP_CONNECT:   
				if('P'==toupper(*ptr))
				{
					ptr++;
					// Passive
					msg=pppConnect(1,ptr);
				}
				else
				{
					msg=pppConnect(0,ptr);			//pointer points to ppp connect string.
					if(PPP_OK==msg)
					{
						//  PPP has gone up, change to internet command mode.
						msg=MSG_CONNECT;
						ircmd_state=IR_INTERNET_COMMAND;
					}
					else
					{
						ppp_down();
						msg=MSG_PPPFAIL;
					}
				}
				break;
			case	IMM_OFF:					// shouldn't we shutdown ppp?
				ircmd_state=IR_MODEM_ONLY;
				msg=MSG_OK;
				break;
			case	MODEM_DTR_TOGGLE:
				modem_toggle_dtr();
				msg=MSG_OK;
				break;
			case	IMM_DISABLE:     
				/* Terminate the PPP connection */		
				ppp_shut_down();				       		

	    		/* Release hardware control on the communication port */  
				/* and bring modem into comand mode by toggling DTR bit */
				modem_toggle_dtr();
				
				/* Return to Modem Command or Data State */      
				ircmd_state=IR_MODEM_COMMAND;  			
				msg=MSG_OK;	
				break;  
			case	RESOLVE_NAME:
				//
				// First Dump Spaces (done above)
				//
				//while(*ptr==' ')
				//	ptr++ ;					//tptr=ptr;				
				//
				// Do DNS Resolution on passed name      
				//                    
				if(resolve_name(&ip, ptr)!=RET_CODE_OK)
				{     
					//dns failed
					msg=MSG_DNSFAIL;
				}
				else
				{           
					// Good lookup, return OK
					displayMessage(MSG_OK);
					//
					// Print Back the IP address 
					//	
					printIP(&ip);				        
					msg=MSG_NONE;
				}                
				break;	
#ifndef DEMO
			case	TCP_CONNECT:                        
				//
				// Set socket index
				//
				msg=MSG_ERROR;
				tptr=ptr;
				if(tptr!=(ptr=SocketSelect(ptr)))
				{
					//
					// Check if we need dns, if first char is 0-9 no DNS needed
					//        
					if(Get_IP(ptr,(U8 *)&ip))
					{
						//
						// Advance pointer over name and extract port
						//
						while((*ptr!=':') && (*ptr!=0))
							ptr++;   
				        	
						if(*ptr==':')
						{  
							ptr++;  
            	        	ptr=s2p(ptr,&tp);
                	    	if(tp>0)
                    			msg=MSG_OK;							
						}
					
					}
					else
						msg=MSG_DNSFAIL;

					/*
					if((*ptr>='0') && (*ptr<='9'))
					{                   

						// Parse and write the IP address 
						tptr=ptr;
						if((tptr=(U8*)s2ip( ptr, &ip ))!=ptr)
						{
							if(*tptr==':')
							{
								ptr=++tptr;  
            	        		ptr=s2p(ptr,&tp);
                	    		if(tp>0)
                    				msg=MSG_OK;
                    		}  							
						}
					}
					else
					{           
						// Do DNS Resolution on passed name      
						if(resolve_name(&ip, ptr)!=RET_CODE_OK)
						{          
							//dns failed
							msg=MSG_DNSFAIL;
						}
						else
						{ 
							//
							// Advance pointer over name and extract port
							//
							while((*ptr!=':') && (*ptr!=0))
								ptr++;   
				        	
							if(*ptr==':')
							{  
								ptr++;  
            	        		ptr=s2p(ptr,&tp);
                	    		if(tp>0)
                    				msg=MSG_OK;							
							}
						}                          
					}
					*/
					if(msg==MSG_OK)
					{

						//
						// Cleanup socket if already in use, this needs fix?
						//			   
						tcp_close(sockets[soc_num]);
						//DEBUG2("*** Open the TCP socket dest %d.%d.%d.%d : %d\n ***",ip.ipb1,ip.ipb2,ip.ipb3,ip.ipb4,tp);	
						sockets[soc_num]=tcp_open(ip, tp, &tcp_socket_rx);
#ifdef DEBUG_LV4
						DEBUG4("Socket open returned ");
						printShort(sockets[soc_num]);
						//DEBUG1("active socket ");
						//printByte(soc_num);
						//DEBUG1(CRLF);
#endif
						if(sockets[soc_num])
						{
							// Activate socket
							active_soc=soc_num;
#ifdef DEBUG_LV4
						DEBUG4("active socket ");
						printByte(soc_num);
						DEBUG4(CRLF);
#endif
							// Initialize snd_count to zero for socket
							snd_count=0;
							// Enter streaming socket mode.
							ircmd_state=IR_STREAM_SOCKET;
						}
						else
						{
							msg=MSG_TCPFAIL;
						}
					}   	
				}
				break;	
			case	TCP_RESUME: 
				//
				// Set socket index
				//
				tptr=ptr;			
				
				if(tptr!=SocketSelect(ptr))
				{
					//
					// Check current socket state [soc_num]
					//
					if(TCP_OPEN==tcp_status(sockets[soc_num]))
					{
						//
						// Resume the connection
						//
						active_soc=soc_num;
						// Initialize snd_count to zero for socket
						snd_count=0;
						// Enter streaming socket mode.
						ircmd_state=IR_STREAM_SOCKET;	          
    	                msg=MSG_OK;
					}
					else
					{
						// Socket not connected, return "socket down" message
						msg=MSG_SOCDOWN;
					}
				}
				break;
					
			case	SOCKET_TERMINATE: 
				//
				// Set socket index
				//
				tptr=ptr;			
				if(tptr!=SocketSelect(ptr))
				{
					tcp_close(sockets[soc_num]);
					//msg=MSG_OK;
				}
				else
				{
					printByte((U8)tcp_status(1));
					CONSOLE_putch('-');
					printByte((U8)tcp_status(2));
				}
				msg=MSG_OK;
				break;
#endif
			case	UDP_ALLOCATE:       
				//
				// UDP cmd processor
				//
				msg=udpcmd(ptr);		
				break;
			case	REGISTER_ACCESS:
				//
				// S-Register Access
				//
				msg=sRegister(ptr);
				break;
			case		EEPROM_CMD:
				// Dump Spaces inbetween cmd and first paramter
				while(*ptr==' ')
					ptr++ ;
				//
				// S, L, or numeric value in hex
				//
				if('I'==toupper(*ptr))
				{
					DEBUG2("Invalidate EEPROM"CRLF);
					//
					// invalidate the eeprom
					//
#ifdef __AVR__	
					eeprom_write_byte (0,0);
#endif
					msg=MSG_OK;
				}
				else if('S'==toupper(*ptr))
				{
					//
					// Save the eeprom
					//
					if(2==global_init(2))
						msg=MSG_OK;
					else
						msg=MSG_ERROR;
				}
				else if('L'==toupper(*ptr))
				{
					//
					// Try to load the eeprom
					//
					if(1==global_init(1))
						msg=MSG_OK;
					else
						msg=MSG_EEERROR;
				}
				else
				{
					tptr=ptr;
					if(tptr!=(U8 *)s2p(ptr,&tp))
					{
						if(*ptr=='=')
						{
							DEBUG2("write eeprom"CRLF);
							ptr=Get_Byte(ptr,&sp);
#ifdef __AVR__	
							eeprom_write_byte (tp,sp);
#endif
							msg=MSG_OK;
						}
						else
						{
#ifdef __AVR__	
							tp=eeprom_read_byte(tp);
							printByte(tp);
#endif
							msg=255;	//we just want crlf
						}
					}
					else
						msg=MSG_ERROR;
				}
				break;	
			case	IMM_BOOTLOAD:
				DEBUG1("Enter Bootloader"CRLF);		
				//
				// Jump to bootloader
				//
#ifdef __AVR__
#ifndef DEMO
				bootloader(0x51f6);
				//asm volatile ("nop");
				//asm volatile ("jmp 0xfc00");
#endif
#endif
				break;

#ifdef WINDOWS
			case	IMM_EXIT:
				//
				// Close the port cleanly for windows and do an orderly shutdown. (avoids reboots)
				//
				MODEM_close();
				exit(0);
				break;
#endif

			default:
				msg=MSG_ERROR;									/* error */
				break;
			}		
	}
	//
	// Decide whether to print a string or a value as a return code.
	//    
	displayMessage(msg);
}


//
// 
//    
U8
irCommand(U8 c)
{  
U8	ret=0;

    //
    // We are in command mode, we shouldn't get non ASC chars unless baud rate is wrong.
    //
    if(c>127)
    { 
#if USE_AUTOBAUD		
    	autobaud_setup();
#endif
     	ret=1;	
    }    
	//
	// check for a return 
	//    
	else if(c==0xd)                
	{             
		if(ircount>0)
		{     
			ircmd_buf[ircount]=0; 
			//
			// if echo is on, echo the character
			//
			if((mm.IMMConfig & ECHO_CMD))
				if((c!=0x0a) && (c!= 0x0d))
					CONSOLE_putch(c);       

			//
			//  Not in the future we should echo only chars on command including 0a 0d
			//  The wait(10) or 20 should get any trailers, which we should echo.
			//
			// sysdata
			CONSOLE_puts(CRLF);
			//CONSOLE_putch(0x0d); 
			//CONSOLE_putch(0x0a);
			//putch('\n');
			TIMER_Wait_Tick(10);
			// Flush input buffer
			while(CONSOLE_kbhit())
				CONSOLE_getch();

			irCommandProcessor(); 
			ret=1;
		}		       
		ircount=0;		       
		ccount=0;
	}
	else
	{  
		if(ircount>0)
		{          
			//
			// Check for BS
			//            
			if(c==0x08)
			{
				ircount--;
				if(ircount==0xff)
				{
					ircount=0;
					ccount=0;
				}
			}
			else
			{	
				if(ircount < IR_BUF_LEN-1)
					ircmd_buf[ircount++]=c; 
			}
			ret=1;	
		}		
		else if(ccount==0)
		{
			if( ((c & 0xdf) == SYNC_CHAR_1))
			{
				ircmd_buf[ircount++]=c;
				ret=1;
			}	
			else if(c == '\r' || c == '\n' || c ==' ')
			{
				ccount=0;
			}
			else
			{
				ccount=1;
			}
		}

/*
		else if( ((c & 0xdf) == SYNC_CHAR_1) && (ccount == 0))
		{
			ircmd_buf[ircount++]=c;
			ret=1;
		} 
		else if(c != '\r' || c != '\n' || c !=' ')
			ccount=0;
		else
			ccount=1;
*/
		//                                    
		if((mm.IMMConfig & ECHO_CMD) && ret)   
			if((c!=0x0a) && (c!= 0x0d))
				CONSOLE_putch(c); 		
	}	  

   	return(ret);
}  


//
// Pass through the modem Singnals, Private
//
U8
Modem_Signals()
{
	//
	// debug LEDs
	//
	if(bit_is_clear (PORTG,1))
		IND2_ON();
	else
		IND2_OFF();

	if(bit_is_clear (PORTD,7))
		IND3_ON();
	else
		IND3_OFF();


	//
	// Check CD and other flow control lines
	//    
	//
	// Feed through RI and CD
	//
	if(DCD1())
	{    
		DCD0_ON();
	}
	else
	{            
		DCD0_OFF();
	}

	//
	// IF PPP is up RI is DAV, else pass through modem RI
	//
	if(mm.ppp_flags & PPP_UP)
	{
		if(udp_ptr==0)
		{
			RI0_OFF();
		}
		else
		{
			RI0_ON();
		}
	}
	else
	{		
		if(RI1())
		{    
			RI0_ON();
		}
		else
		{	
			RI0_OFF();
		}       
	}
	//
	// Check for DTR Change and feed through in modem only mode
	//
	if( DTR0() )
	{
//+++		DTRI_ON();
		if(ircmd_state==IR_MODEM_ONLY)
		{
			// feed DTR to modem
			DTR1_ON();
		}
	}
	else
	{
// +++		DTRI_OFF();
		if(ircmd_state==IR_MODEM_ONLY)
		{
			// feed DTR to modem
			DTR1_OFF();
		}		
	}
	return(0);
}

//
//
//
void 
Check_Carrier(void)
{            

	Modem_Signals();
	//
	// Check for CD change
	//         
	if(ircmd_state!=IR_MODEM_ONLY)
	{
		if(carrier>0) 
		{     
			//
			// Check for Carrier Drop
			//
			if(!DCD1())
			{
				DCDI_OFF();
				//
				// We have lost the carrier, Drop PPP, set serial port back to software, and
				// chnge back to IR_MODEM_COMMAND, print lost carrier messages.
				//                                
				carrier=0;     
				ppp_down();
				ircmd_state=IR_MODEM_COMMAND;  
				displayMessage(MSG_NOCARRIER);
			}
		}
		else
		{
			//
			// Check for Carrier
			//
			if(DCD1())
			{
				DCDI_ON();
				//
				// We have a carrier, change state to IR_MODEM_DATA -or- IR_MODEM_COMMAND
				//
				carrier=1;           
				if(mm.IMMConfig & DATA_MODE_ESCAPE)				
			    {
					ircmd_state=IR_MODEM_DATA;
				}
				else
				{
					ircmd_state=IR_MODEM_COMMAND;
				}
			}	
		}
	}
}

//
// See if need to force a send of a TCP segment
//
// Sending a 1 as a command forces a send
//
int
TCPSend(U8 cmd)
{    
U8	go=0,ret=0;
#ifndef DEMO
	//
	// Check for force flush
	//
	if(cmd==1)
	{
		TCPSendFlag=0;
		DEBUG2("force flush\n");
		go=1;
	}
	else if(snd_count>=MAX_SND_SIZE)			// full buffer send
	{
			DEBUG1("Full buffer send \n");
			go=1;
	}
	else if(TCPSendFlag)						// timer send
	{   
		//
		// Check for Timeout
		//
		if(abs(TIMER_Get_Tick()-TCPSendTime) > mm.TCPStreamTickTime)
		{
			// We need to send
			DEBUG2("timeout send");
			//TCPSendFlag=0;
			//if(mm.IMMConfig & DEBUG_SEND)
			//	CONSOLE_putch('*');
			go=1;
		}
	}
	if(go)
	{
		TCPSendFlag=0;
		//
		// Only send if buffer contains data
		//
		if(snd_count)
		{
			ret=tcp_send(sockets[active_soc],snd_buff,snd_count);
			if(ret)
			{
				// Send OK reset send count, snd_count should be updated if sent less than
				snd_count=0;
				DEBUG1("@");
			}
			else
			{
				// The socket has failed send, flow control?  --- Experimental Code
				// Do nothing --
				DEBUG1("&");		
				// The socket has overflowed, abort the socket and go to cmd mode
			//	if(tcp_close(sockets[active_soc]))
			//	{
			//		CONSOLE_puts(CRLF);
			//		displayMessage(MSG_SOCDOWN);
			//	}
			//	ircmd_state=IR_INTERNET_COMMAND;
				//snd_count=0;
			}
			// Reset snd_count
			//snd_count=0;
		}
		return(1);
	}
#endif
	return(0);
}


//
// Returns 0 if active and OK, or 1 if socket down, to be called only in Streaming Socket Mode
//
U8 
TCPState(void)
{
U8	ret=1;
 
#ifndef DEMO
    //
    // Check Active Sockets for State Changes
    // 
	ret=(U8)tcp_status(sockets[active_soc]);
	if(TCP_OPEN==(TCP_STATE_MASK &ret))
	{
		// Socket is active
		ret=0;
	}
	else
	{
#ifdef DEBUG_LV2
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Socket Down state = ");
			printByte(ret);
			DEBUG1("Active socket = ");
			printByte(active_soc);
			DEBUG1(CRLF);
		}
#endif
		ircmd_state=IR_INTERNET_COMMAND;
		tcp_close(sockets[active_soc]);
		CONSOLE_puts(CRLF);
		displayMessage(MSG_SOCDOWN);	
	}

#endif
	return ret;
} 


//
// Writes a char to the currently open TCP socket, in this case
// we should buffer here and send on threashold or timeout.
//

U8
WriteTCPChar(U8 c)
{   
#ifndef DEMO
	//
	// Check for buffer full, if full should push char back
	//
	if(snd_count>=MAX_SND_SIZE)
	{
		// full push back char....
		//DEBUG1("** we shouldn't get here WriteTCPChar **"CRLF);
		return(1);
	}          
	TCPSendFlag=1;		    
    TCPSendTime=TIMER_Get_Tick();  

	if(mm.IMMConfig & ECHO_STREAM)
		CONSOLE_putch(c);

	// Store char in buffer
	snd_buff[snd_count++]=c;
#endif
	return(0);
}

void
passthrough(void)
{
S16	c;

#ifdef __AVR__
	while(MODEM_kbhit())
	{	
		c=MODEM_getch();
		CONSOLE_putch((U8)c);
	}
#else
	while(-1!=(c=MODEM_getch()))
		CONSOLE_putch((U8)c);

#endif

}

//
// Check_Reset_EEPROM()
//
void
Check_Reset_EEPROM()
{
static U8  reset_status;
static U16 reset_timer;

	if(SW5())
	{
		if(!reset_status)
		{
			// Set timer
			reset_timer=TIMER_Get_Second();
			reset_status=1;
		}
		else
		{
			// Check if it is time to reset
			if(abs(TIMER_Get_Second()-reset_timer)>10)
			{
				if(1==reset_status)
				{
					reset_status++;
#ifdef __AVR__
					eeprom_write_byte(0,0);
#endif
				}
				else
				{
					IND2_ON();			
				}
			}
		}
	}
	else
	{
		reset_status=0;
	}
}


//
//	Rewrite for 4 modes:
//
//		Modem command mode		= some IR commands, all AT commands
//		Modem Data Mode			= Standard Modem Data Mode
//		internet command mode	= IR command modes only   
//		Streaming Socket Mode	= data mode connected to a socket
//
//
void
irCommands(void)
{
    char 	c;
	U8		i;
	U8 		result;
	U8		tcp_count;

//	U16		ss;
    /* Initial Setup for S7600 serial port, hardware flow control, DTR and sofware control on the communication port */
//	writex(IR_PORT_CTRL_STAT, 0x20);
	//
    // Default Initialization section, some could be out of EEPROM
    //
    ircmd_state = IR_MODEM_COMMAND;
    ircmd_buf[0] = 0;                                                                    
    TCPSendFlag=0;
//	EE_Read_Address=0;
	//
	// Initialize UDP variables
	//
	udp_sp=0;
	udp_ip.ip32=0;
	udp_len=0;
	udp_ptr=0;

/*
#if UINTERNET
	EE_Read_Flag=EE_IDLE;
#else	
	EE_Read_Flag=EE_ACTIVE;
#endif
*/	
	//dtr_setup();
    TIMER_Wait_Tick(10);

	//
	// Pass through anything buffered from the devices.
	//

	DEBUG2("passthrough"CRLF);
	passthrough();

//ss=TIMER_Get_Second();

	DEBUG2("starting main loop"CRLF);
	//
	// Main Loop,  All actions are called from this loop, this loop never exits.
	// If you need a task it must be non blocking and called from this loop.
	//
	for(;;)
	{    
	
		//if(ss!=TIMER_Get_Second())
		//{
		//	ss=TIMER_Get_Second();
		//	DEBUG1(".");
		//}

		/* slow down the main loop for debugging purposes */
		//TIMER_Wait_Tick(1);     
		//
		// This should be removed, the serial interrupt should be hooked
		// directly up to interrupt.
		poll_serial();	
		//
		// Stuff we don't have to process alot.
		// For now TCP, ppp_passive, check carrier, check reset EEPROM
		//
		tcp_count++;
		if(0==tcp_count)				// TCP count rolls to zero every 255 counts
		{
			// TCP process background process
			tcp_process(0);
			//
			// Check the Carrier and other modem signals
			//
			Check_Carrier();	
			//
			// Check for a reset EEPROM action, press SW3 for 10 seconds
			//
			Check_Reset_EEPROM();
			//
			// Check Passive PPP states
			//
			ppp_passive();
			//
			// Check for escape action
			//
		}
		//
		// Check if we have an escape
		//
		if(EscapeAction())
		{    
			if((IR_MODEM_DATA==ircmd_state) || (IR_STREAM_SOCKET==ircmd_state ))
			{
     
				if(ircmd_state==IR_STREAM_SOCKET)  
				{
					ircmd_state=IR_INTERNET_COMMAND;
					TCPSend(1);							// currently streamed socket  					
				}
				else
				{
					ircmd_state=IR_MODEM_COMMAND;     
				}
				CONSOLE_puts(CRLF);
				displayMessage(0);
				EscapeReset();
			}
		}                 
        //
        // Check for data comming in from USER (CPU uart)
        //
		if(CONSOLE_kbhit())
		{
			DEBUG5("kbhit"CRLF);
			// get the char from the USER		
			c=CONSOLE_getch();

			// Depending on state do somthing
			switch(ircmd_state)
			{
			case 	IR_MODEM_COMMAND:   
				
				// Check irCommand processor
    			if(!irCommand(c))
    			{             
    				//
    				// Not irCommand sequence, pass through.
    				//
					DEBUG5("modemputch1");
    				MODEM_putch(c);    
    				DEBUG5("."CRLF);
    			}     
  				break;	
			case	IR_MODEM_ONLY:
				DEBUG5("modemputch2");
				MODEM_putch(c);
				DEBUG3("."CRLF);
				break;			
			case 	IR_MODEM_DATA: 
                //
                // Look for escape sequence
                //                         
				if(mm.IMMConfig & DATA_MODE_ESCAPE)				
                {
	                result=EscapeSequence(c);
    	            if(result!=0xff)
        	        {
            	    	if(result>0)
                		{
	                		for(i=0;i<result;i++)
    	            			MODEM_putch(mm.escapeChar);
        	        	}
            	   		MODEM_putch(c);
                	}
				}
				else
				{
					MODEM_putch(c);
				}
				break;
				
			case    IR_INTERNET_COMMAND:
				if(!irCommand(c))
    			{
					//
					// Not procession an IR command, do nothing  
					//
    			}
				EscapeReset();
				break;
								
			case IR_STREAM_SOCKET:			
				result=EscapeSequence(c);			                              
				if(result!=0xff)
				{				
					if(result>0)
					{
						for(i=0;i<result;i++) 
							WriteTCPChar(mm.escapeChar);
					}					
					if(0!=WriteTCPChar(c))
					{
						// Cannot write any more, push back char
						CONSOLE_pushch(c);
						//DEBUG1("*");
					}                
				}	
				break;	
            }     
		}		
		
		//
		// Check for data commin in from MODEM
		//		Only valid for modem command and data modes.    
		//                                
        if(ircmd_state & 0x1)
		{        	
			//passthrough();
        }
        else
        {
			//
			// In the streaming socket state send and receive data on the TCP streaming
			//	socket.
			//
        	if(ircmd_state==IR_STREAM_SOCKET)
        	{
				//
				// check for TCP send pending
				//             
				TCPSend(0);    
				TCPState();

			  }
        }		
	} // for
}// irCommands			



