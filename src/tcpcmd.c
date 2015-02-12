/*															
 *---------------------------------------------------------------------------
 * tcpcmd.c - TCP command processor											-
 *	All TCP commands and apps are here										-
 *---------------------------------------------------------------------------
 * Original Version                                                         -
 *  Oct 5, 2003	- mwj                                                       -
 *                                                                          -
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2003, nChip												_
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
#include "msg.h"       
#include "console.h"
#include "conio.h"
#include "time.h"
#include "ports.h"
#include "globals.h"
#include "utils.h"
#include "debug.h"

//#define UDP_ALLOCATE		'U'				//	U	=	AllocateUDP
//#define UDP_CLOSE			'V'				//	V	=	Close UDP
//#define UDP_GET			'G'				//	G	=	GetUDP
//#define UDP_PUT			'P'				//	P	=	PutUDP
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
		CONSOLE_putch(data[i]);
	}

// just eat all for now.
return(len);
	//
	// Else not active socket, here we return that no bytes were processed
	// this will cause no ACK to be returned, causing the peer to retransmit
	// (kinda like buffering it at the peer).  But when we select this socket
	//	again we will force an ack to get the peer to retransmit if it is
	// in a high retransmit time state.
	//


}





//
// tcpcmd - handle TCP commands here, can be (T)CP connect, (R)esume, (X)Release, -or- TCP Apps
//		(H)TTP Get and (F)TP Get and Put
//
U8
tcpcmd(U8 cmd,U8 *ptr)
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

	switch(cmd)
	{
	case	FTP:
				msg=MSG_OK;
				// Command IRF[G|P]<host>:[URI]
				if('G'==*ptr)
				;
				else if ('P'==*ptr)
				;
				else
					msg=ERROR;
				if(msg!=ERROR)
				{
					msg=MSG_OK;
				}
	case	HTTP_GET:		// 'H'     
				// commang IRH<host>:[URI]:[flags]
				//
				// Set socket index
				//
				msg=MSG_ERROR;
				tptr=ptr;
				if(tptr!=(ptr=SocketSelect(ptr)))
				{
					//
					// Then parse host IP
					//
					if(0==isalnum(*ptr))
					{
						msg=MSG_ERROR;
					}
					else if(Get_IP(ptr,&ip))
					{
						//advance over hostname and get URI
						while((*ptr!=':') && (*ptr!=0))
							ptr++;   
				        	
						if(*ptr==':')
						{  
							ptr++;
							uri=ptr;
							// Copy URI
							while((*ptr!=':') && (*ptr!=0))
							if(*ptr==':')
							{ 
								// Get options if available
            	        		ptr=s2p(ptr,&tp);                	    
							}
							//
							// Cleanup socket if already in use, this needs fix?
							//			   
							tcp_close(sockets[soc_num]);
							DEBUG2("*** Open the TCP socket dest %d.%d.%d.%d : %d\n ***",ip.ipb1,ip.ipb2,ip.ipb3,ip.ipb4,tp);	
							sockets[soc_num]=tcp_open(ip, tp, &tcp_socket_rx);
							//
							// Check if connected
							//
							if(sockets[soc_num])
							{
								// Generate GET URI
								

								
							}
							else
							{
								msg=MSG_TCPFAIL;
							}

					}
					else
						msg=MSG_ERROR;

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


