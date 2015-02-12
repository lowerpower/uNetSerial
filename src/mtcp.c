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

/*																www.mycal.net			
 *---------------------------------------------------------------------------
 * mtcp.c - Mycal TCP Layer													-
 *																			-
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 3, 1998									-        
 *
 * Exports:
 *	void	
 *
 *---------------------------------------------------------------------------    
 *                                                             				-
 * Copyright (C) 1998, Mycal Labs www.mycal.net								-
 *                                                                         	-
 *---------------------------------------------------------------------------
 * Notes:
 *
*/
#include "pppconfig.h"
#include "mTypes.h"
#include "ircmd.h"
#include "mip.h"
#include "mtcp.h"
#include "ppp.h"
#include "time.h"
#include "memory.h"
#include "globals.h"
#include "debug.h"
//#include "bget.h"

//U8			test=0;

#ifndef DEMO
TCPSOC	tcp_socket[MAX_TCP_SOCKETS];
#endif

//-----------------------------------------------------------------------------
// tcp_init() - just init the tcp socket table.
//-----------------------------------------------------------------------------
void
tcp_init()
{
#ifndef DEMO
U8	i;
	
	for(i=0;i<MAX_TCP_SOCKETS;i++)
	{
		tcp_socket[i].state=0;
		tcp_socket[i].packets=0;
	}
#endif
}

/*
get_header_buffer()
{
}

free_packet()
{
}
*/
//-----------------------------------------------------------------------------
// tcp_upcall() - 
//
//	this handles incoming TCP packets/segments.  ++++ fix receiving retries no calling upcall
//
//-----------------------------------------------------------------------------
void
tcp_upcall(IP *ip, TCP *tcp, U16 len)
{
#ifndef DEMO
U16			tchk=0;
U16			protocol,i;
S16			data_len,processed_len;
U8			state,sendnow=0,ack=0;
TCPRSTSOC	rst_soc;
MEMBLOCK	*memblock;
	//
	// Check the TCP checksum 
	//	
	// First do psudo header
	//
	len-=sizeof(IP);
	i=htons(len);
	protocol=htons(PROT_TCP);
	//protocol=PROT_UDP;
	tchk=Checksum_16(0,(U16 *)&ip->source_ip,8);
	tchk=Checksum_16(tchk,(U16 *)&i,2);
	tchk=Checksum_16(tchk,(U16 *)&protocol,2);
	tchk=Checksum_16(tchk,(U16 *)&tcp->source_port,len);

	if(tchk!=0)
	{
		DEBUG2("Bad TCP Checksum"CRLF);
		return;
	}
	//
	// Find the data length of the segment, length of IP packet minus IP header minus
	//	TCP header.
	//
	data_len=htons(ip->length) - ( sizeof(struct ip_h) + (tcp->len >> 2) );

	//
	// Demux on dest socket, just search list
	//
	for(i=0;i<MAX_TCP_SOCKETS;i++)
	{
		// look for a match
		state=tcp_socket[i].state & TCP_STATE_MASK;
		if	((tcp_socket[i].dest_port==tcp->source_port) &&
			(tcp_socket[i].source_port==tcp->dest_port) &&
			(tcp_socket[i].dest_ip.ip32==ip->source_ip.ip32) &&
			(state!=0))
		{
			//
			// We have a Match, process the segment
			//
			//Call the app. (struct ip_h *header,U8 *data, U16 len));
			//tcp_socket[i].socket_callback(ip,tcp,len);
			DEBUG4("We have a match on socket %d \n"CRLF,i);	
			//
			// Handle Acking of data first
			//
			if(tcp->flags & TCP_ACK_FLAG)
			{
				// Set a local var that we have seen an ack flag.
				ack=1;
				//
				// Check incoming ACK, for now we only accept an expected              +++ Dangerous fix
				//	ACK.  This should be changed in the future to accept
				//	any ACK's that are in window.  For now we've just simplified. need to handle window wrap
				//
				if((htonl(tcp->acknum)<tcp_socket[i].un_ack) &&
					htonl(tcp->acknum>=tcp_socket[i].send_next  ))
				{
					// Bad Ack from peer +++ this should be cleaned up we should take into account window size
#ifdef DEBUG_LV4
					DEBUG4("Bad Ack? ack was ");
					printWord(htonl(tcp->acknum));
					DEBUG4(" Expected ");
					printWord(tcp_socket[i].send_next-1);
					DEBUG4(CRLF);
#endif					
					sendnow=1;
					ack+=2;
				}
				else
				{
					if(tcp_socket[i].un_ack==htonl(tcp->acknum))
					{
						DEBUG4("dup ACK"CRLF);
					}
					else
					{
	
						DEBUG4("process ACK"CRLF);
						//
						// Update unacked sequence number to reflect the current ack.
						//
						tcp_socket[i].un_ack=htonl(tcp->acknum);  //+++ dangerous fix
						//
						// Free up any tx buffer's that don't have any unacked data in them.
						//
						while(1)
						{
							// First check to see if there are any TX bufers waiting on this socket
							if(tcp_socket[i].packets)
							{
								// Check to see if this packet has no more unacked data in it
								//if((((S16)tcp_socket[i].un_ack-tcp_socket[i].packets->ack)>=0))
								if(  ((S16)((U16)tcp_socket[i].un_ack-tcp_socket[i].packets->ack)>=0)  )
								{
									// It has no more unacked data in it remove it from the list and free it.
									DEBUG4("free packet"CRLF);
									memblock=tcp_socket[i].packets;
									tcp_socket[i].packets=memblock->next;
									MEM_FreeBlock(memblock);
									continue;
								}
								// We still have packets, reset the retry timer
								tcp_socket[i].timer=TIMER_Get_TCP_HundredMS();//TIMER_Get_Tick();
							}
							else
							{
								// no more packets, set the timer to zero
								tcp_socket[i].timer=0;
							}
							break;
						}

						//
						// If ack is of a FIN
						//
						if(state==TCP_FIN_WAIT_1)
						{
							state=tcp_socket[i].state=TCP_FIN_WAIT_2;
							sendnow=1;							// we need to send an ack
						}
						if((state==TCP_LAST_ACK) || (state==TCP_CLOSE_WAIT))
						{
							tcp_socket[i].state=0;				// ack of our FIN, close socket
						}
					}
				}
			}
			//
			// Then check for RST packet
			//
			if(tcp->flags & TCP_RST_FLAG)
			{
				// We have a RST flag, for now just kill socket
				// We might want to look about freeing any memory used
				// by this socket here. +++
				tcp_clean_socket((TCPSOC *)&tcp_socket[i]);
				//tcp_socket[i].state=0; //TCP_RST;
				DEBUG2("Received Reset"CRLF);
				sendnow=0;
				break;
			}
			else
			{     // unnested begin
			//
			// Try to handle data on a socket, we can receive it in the open state and
			//	the FIN_WAIT states, we cannot accept data in any other state.
			//
			if((state==TCP_OPEN) || (state == TCP_FIN_WAIT_1) || (state == TCP_FIN_WAIT_2))
			{
				//
				// Lets look at the URG flag.  If set we will pass that info up to the
				// app as an length into the data to the URG byte.
				//
#ifdef TCP_URG_SUPPORT
				if(tcp->flags || TCP_URG_FLAG)
				{
					urg = htons(tcp->urg) - htonl(tcp->seqnum);
					DEBUG2("URG flag received, URG in %d bytes\n",);
				}
#endif
				//
				// Make sure that we only pass info that we havn't received before.this to fix
				//
				if(data_len > 0 )
				{
					//
					// Check the sequence number, we can only receive in order
					//
					if(htonl(tcp->seqnum)==tcp_socket[i].receve_next)
					{
						DEBUG2("packet data len is %d - original len %d\n",data_len,htons(ip->length));
						//
						// We need to call the upcall here to deliver the data to the
						// Application that owns the socket.
						//
						DEBUG2("tcp_upcall - We have data length %d\n on socket %d",data_len, i);
						processed_len=tcp_socket[i].socket_callback(i+1,tcp->data,data_len);
						//
						// Update the RCV.NXT sequence nubmer so we can ACK properly.
						//
						tcp_socket[i].receve_next+=processed_len;
						if(processed_len)
							sendnow=1;
					}
					else
					{
						DEBUG2("Dropped pkt expect seq %lu --> %lu\n",tcp_socket[i].receve_next,htonl(tcp->seqnum));
						sendnow=1;
					}
				}
			}

			//
			// Check for peer sending a FIN
			//
			if(tcp->flags & TCP_FIN_FLAG)
			{
				if(state==TCP_OPEN)
				{
					// we've received a FIN flag while in the ESTABLISHED state, move to
					// CLOSE_WAIT state.  Bump the ACK number by one so we can ACK the
					// Peers FIN.
					tcp_socket[i].state=TCP_CLOSE_WAIT;
//printf("tcp open fin +1 - %d\n",tcp_socket[i].receve_next);
					tcp_socket[i].receve_next+=1;			// inc to ack peer's FIN
//printf("tcp open fin +1 - %d\n",tcp_socket[i].receve_next);
					//
					// Bump up send_next(was next ack) since we expect them to ack our fin now
					//
					tcp_socket[i].send_next+=1;
					sendnow=1;
				}
				else if(state==TCP_FIN_WAIT_1)
				{
					// We've received a FIN flage while in the TCP_FIN_WAIT_1 state, move to
					// CLOSING state.  Bump the ACK nubmer by one so we can ACK the Peers FIN.
					tcp_socket[i].state=TCP_CLOSING;
					tcp_socket[i].receve_next+=1;			// inc to ack peer's FIN
//printf("tcp fin wait 1 fin +1\n");
					sendnow=1;
				}
				if(state==TCP_FIN_WAIT_2)
				{
					// We've received a FIN flage while in the TCP_FIN_WAIT_1 state, move to
					// CLOSING state.  Bump the ACK nubmer by one so we can ACK the Peers FIN.
					tcp_socket[i].state=TCP_TIME_WAIT;
					tcp_socket[i].receve_next+=1;			// inc to ack peer's FIN
//printf("tcp fin wait 2 fin +1\n");
					sendnow=1;
				}				 
			}			

			//
			// Take action if in SYN_SENT or Listen state
			//
			if((state==TCP_SYN_SENT) || (state==TCP_LISTEN))
			{
				//
				// When in TCP_SYN_SENT or TCP_LISTEN state we should get a packet with the SYN_FLAG
				//	set.
				//
				if((tcp->flags & TCP_SYN_FLAG))
				{
						// Store incoming SEQ
						tcp_socket[i].receve_next=htonl(tcp->seqnum)+1;
						if (state==TCP_LISTEN)
						{
							tcp_socket[i].state=TCP_SYN_RCVD;
						}
						else
						{
							tcp_socket[i].state=TCP_OPEN;
							//tcp_socket[i].send_next=tcp->acknum;		// Don't like this,we take whatever acks
 //tcp_socket[i].send_next+=5555;
						}
						sendnow=1;
				}
				else if(ack&2)
				{
					// send RST
					tcp_socket[i].state=TCP_RST;
					tcp_socket[i].send_next=htonl(tcp->acknum);
					sendnow=1;
				}

			} // end if((state==TCP_SYN_SENT) || (state==TCP_LISTEN))

			} // end of unnested begin
			// 
			// Should we do something now?
			//
			if(1==sendnow)
			{
				//
				// We need to send a packet now (either an ACK, RST, FIN or SYN), call 
				//	tcp_sendit(socket, data, len) - we have no data.
				//
				tcp_sendit((TCPSOC *)&tcp_socket[i],0,0);
			}
			break;	// remove this will cause a reset to be sent on socket, which can be used for testing
		}
		//break;	// exit here, don't try to loop on more sockets
	}
	//
	// If no match of socket on incoming segment, send a reset.  But don't
	// do anything if incoming packet is a reset.
	//
	if((i>=MAX_TCP_SOCKETS) && (!(tcp->flags & TCP_RST_FLAG)))
	{
		//
		// Send Reset, this is a no socket RST, use the RST socket, fill with current packet info
		//	so we can do a proper RST?
		//
		DEBUG4("no match send reset"CRLF);
		rst_soc.dest_ip.ip32	=ip->source_ip.ip32;
		rst_soc.source_port		=tcp->dest_port;
		rst_soc.dest_port		=tcp->source_port;
		rst_soc.send_next=0;
		//
		// Check ACK flag on incoming pkt
		//
	//	if(tcp->flags & TCP_ACK_FLAG)
		//	rst_soc.receve_next	=htonl(tcp->seqnum)+10;	// set seq number if ACK flag set
	//	else
	//		rst_soc.receve_next=0;						// else send zero for seq
		//
		// set acknum
		//
		if(tcp->flags & TCP_ACK_FLAG)
			rst_soc.receve_next	=htonl(tcp->seqnum)+data_len;
		else
			rst_soc.receve_next=0;  						// else send zero for seq


		rst_soc.state=TCP_RST | TCP_RST_SOCKET;
		tcp_sendit((TCPSOC *)&rst_soc,0,0);
	}
#endif
} // functgion


//-----------------------------------------------------------------------------
// tcp_listen(port, port_callback) - tcp listen on a port.  Requires a 
//		Callback routine per socket for receive packets.
//-----------------------------------------------------------------------------
U8
tcp_listen(U16 port, void (*callback)(struct ip_h *header,U8 *data, U16 len))
{
#ifndef DEMO
U8	i;
	
	for(i=0;i<MAX_TCP_SOCKETS;i++)
	{
		if (0==tcp_socket[i].state)
		{
			//tcp_socket[i].port=htons(port);			// we convert here once, not every time on demux
			//tcp_socket[i].socket_callback=callback;
			return (i+1);
		}
	}
#endif
	return 0;
}


//-----------------------------------------------------------------------------
// tcp_open(port, port_callback) - 
//
//	open up a tcp connection.  Requires a Callback routine per socket for 
//	receive packets.
//
//-----------------------------------------------------------------------------
U8
tcp_open(IPAddr dest_ip, U16 dest_port, void (*callback)(U8 socket, U8 *data, U16 len))
{
#ifndef DEMO
U8		i;
TCPSOC	*ts;
U8 temp=0;
	
	// we should search closed, then close_wait sockets, we just search closed ones now.
	for(i=0;i<MAX_TCP_SOCKETS;i++)
	{
		//
		// Find a closed socket to use.  We probably have to implement a 
		// TIME_WAIT socket recovery mechinism to use TIME_WAIT sockets 
		// when no closed sockets are found.
		//
		if (0==tcp_socket[i].state)
		{
			ts=&tcp_socket[i];
			//
			// We've found a closed socket, lets initiate a connection on it,
			//
			ts->socket_callback=callback;
			ts->dest_port=htons(dest_port);			// swap here
			ts->dest_ip.ip32=dest_ip.ip32;
			ts->source_port=TIMER_Get_Second();
//#ifdef DEBUG_LV3
#ifdef DEBUG_LV2
		if(mm.IMMConfig & PPP_DEBUG)
		{
				DEBUG3("Open TCP socket using Source Port ");
				printShort(htons(ts->source_port));
				DEBUG3(CRLF);
		}
#endif
//#endif
			ts->state=TCP_SYN_SENT | TCP_NEED_SEND;
			ts->send_next=1;						// Set ISN, zero for now +++
			ts->un_ack=0;
			ts->receve_next=0;
			ts->packets=0;

			ts->retry=0;
			ts->timer=0;
			//
			// Try to open with retry and timeout.
			//
			timer_set();
			tcp_sendit(ts,0,0);
			while(1)
			{
				poll_serial();
				tcp_process(0);
				if((ts->state & TCP_STATE_MASK) == TCP_OPEN)
				{
#ifdef DEBUG_LV2
					if(mm.IMMConfig & PPP_DEBUG)
					{
						DEBUG2("socket connected returning ");
						printShort(i+1);
						DEBUG2(CRLF);
					}
#endif
					return(i+1);
				}
				if((ts->state == 0))
				{
#ifdef DEBUG_LV2
					if(mm.IMMConfig & PPP_DEBUG)
					{
						DEBUG2("Socket closed or reset"CRLF);
					}
#endif				
					break;
				}

				if(timer_timeout(mm.tcp_connect_timeout))		
				{
					ts->state=TCP_CLOSED;
#ifdef DEBUG_LV2
					if(mm.IMMConfig & PPP_DEBUG)
					{
						DEBUG2("tcp connect timeout"CRLF);
					}
#endif	
					break;
				}
			}
			break;
		}
	}
#endif
	return 0;
}


//-----------------------------------------------------------------------------
// tcp_close(socket) - Close only fron open state, else RST.
//		Cleans up all allocated memory attached to socket.
//
//	returns 1 if socket closed
//  returns 0 if socket already closed
//-----------------------------------------------------------------------------
U8
tcp_close(U8 socket)
{
#ifndef DEMO
TCPSOC	*ts;
U8		ret=1;

#ifdef DEBUG_LV2
U16 tcount=0;				// remove when done dbugging
#endif

	while(socket>0)
	{
		socket-=1;	// ++ fix this?
		ts=&tcp_socket[socket];
		//
		// If socket is closed do nothing
		//
		if (/*(ts->state==0) ||*/ 0==(TCP_STATE_MASK & ts->state))
		{
			ret=0;
			break;
		}	
		if (TCP_TIME_WAIT==(TCP_STATE_MASK & ts->state))
		{
			tcp_clean_socket(ts);
			break;
		}
		
		if (TCP_OPEN==(TCP_STATE_MASK & ts->state))
		{
			ts->state=TCP_FIN_WAIT_1;
			ts->send_next+=1;						// was next_ack+=1
		}
 		else
		{
			// send a reset on socket
			//ts->state=0
			ts->state=TCP_RST;
			tcp_sendit(ts,0,0);
			tcp_clean_socket(ts);
			break;
		}
		tcp_sendit(ts,0,0);
		//
		// Wait for close with timeout.
		//
			timer_set();
			while(1)
			{
				poll_serial();
				tcp_process(0);
				if((ts->state == 0))
					break;
				if((ts->state == TCP_TIME_WAIT))
					break;
#ifdef DEBUG_LV2
				if(mm.IMMConfig & PPP_DEBUG)
				{
					if(tcount==0)
					{
						DEBUG2("Socket state = ");
						printWord(ts->state);
						DEBUG2(CRLF);
					}
					tcount++;
				}
#endif
				if(timer_timeout(7))		// 5 seconds, make constant
				{
					break;
				}
			}
		tcp_clean_socket(ts);
		break;
	}
	//ts->state==0;		// close no matter what
#endif
	return(ret);
}


//-----------------------------------------------------------------------------
// tcp_tx(socket, peer_ip, peer_port, buffer, len)
//
// returns number of bytes sent, or -1 for send failed.	
// 
//-----------------------------------------------------------------------------
S16
tcp_send(U8 socket, U8 *data, U16 len)
{
#ifndef DEMO
S16 ret=0;

	if(socket>0)
	{
		if (TCP_OPEN==(TCP_STATE_MASK & tcp_socket[socket-1].state))
		{
//test=1;
//CONSOLE_puts("send\n");
				ret=tcp_sendit(&tcp_socket[socket-1],data,len);
		}
	}
	return(ret);
#endif
}


//-----------------------------------------------------------------------------
// tcp_status(socket)
//
// returns socket status	
// 
//-----------------------------------------------------------------------------
S16
tcp_status(U8 socket)
{
#ifndef DEMO
	return(tcp_socket[socket-1].state);
#endif
}

//-----------------------------------------------------------------------------
// tcp_sport(socket)
//
// returns socket status	
// 
//-----------------------------------------------------------------------------
S16
tcp_sport(U8 socket)
{
#ifndef DEMO
	return(tcp_socket[socket-1].source_port);
#endif
}


/* 
  use this for non callback version
//-----------------------------------------------------------------------------
// udp_rx(socket, peer_ip, peer_port, buffer )
//	returns length if data, or zero if null packet, or -1 if no packet.
//
//-----------------------------------------------------------------------------
S16
udp_rx(U8 socket, IP *peer_ip, U16 *peer_port, U8 *buffer)
{



}
*/

//-----------------------------------------------------------------------------
// tcp_process() - 
//
//	This should be called every 250-500ms.  This function handles retransmits
//	and delayed acks.
// 
//-----------------------------------------------------------------------------
void
tcp_process(U8 flags)
{
#ifndef DEMO
U16		i;
	//
	// Loop through all sockets looking for work to do
	//
	for(i=0;i<MAX_TCP_SOCKETS;i++)
	{
		if(tcp_socket[i].state!=TCP_CLOSED)
		{
			// For now just check timer=0, we should check other timeouts here
			// tcp_retruansmit_timout is settable to user global, but we should
			// track in future and adjust per socket
			//if(((TIMER_Get_Tick()-tcp_socket[i].timer) > mm.tcp_retransmit_timeout)  +++
			//			&& (tcp_socket[i].timer))

			if(( (U16)(TIMER_Get_TCP_HundredMS()-tcp_socket[i].timer) > mm.tcp_retransmit_timeout) 
						&& (tcp_socket[i].timer))
			{
				DEBUG2(" retry!, socket state %x",tcp_socket[i].state);

				// lets rock, incerment the retry count, we should check and kill socket
				// if too many retrys? probably not but maybe tell user
				tcp_socket[i].retry++;	
				tcp_socket[i].timer=TIMER_Get_TCP_HundredMS();//TIMER_Get_Tick();
				//
				// If there is a buffer resend it, else just resend (probably a SYN or FIN)
				//
				if(0==tcp_socket[i].packets)
				{
					// retransmit with no data (a SYN or FIN))
					tcp_sendit((TCPSOC *)&tcp_socket[i],0,0);			
				}
				else
				{
					// Retransmit with data 
					//tcp_sendit((TCPSOC *)&tcp_socket[i],tcp_socket[i].packet->data,0);
					ip_send((IP *)tcp_socket[i].packets->data,0,tcp_socket[i].packets->len,0);
				//CONSOLE_puts("retran\n");
				}
			}
		}
	}
#endif
}

//	U16			timer;				// Retransmission Timer
//	U8			retry;				// Retransmission count
//	MEMBLOCK	*packets;			// packets in flight.

//--------------------------------------------------------------------------------------
//
// tcp_sendit (socket, data, len) -
//
//	This function handles the sending of a TCP packet.  Typically this is called
//	by tcp_send, or when an ACK, RST, or window advertizement needs to be sent out.
//
//--------------------------------------------------------------------------------------
S16
tcp_sendit(TCPSOC *ts,U8 *data, U16 len)
{
#ifndef DEMO

IP			*ip;
TCP			*tcp;
U16			protocol,tchk,tlen;
U8			t,flags,oplen=0,set_timer=0,*tptr;
MEMBLOCK	*memblock,*pkt;


	flags=0;	
	//
	// We need to get a buffer off the free list, and setup the header
	//
	memblock=MEM_GetBlock();
	if(0==memblock)
	{
		return(0);
	}

	//
	// Point into the data
	//
	ip=(IP *)memblock->data;
	tcp=(TCP *)ip->transport;

	//
	// Data?  If so copy it into the buffer
	//
	if(len)
	{
		tptr=tcp->data;
		for(tlen=0;tlen<len;tlen++)
			tptr[tlen]=data[tlen];
	}
	//
	// Build TCP header, first set flags
	// 
	t=ts->state & TCP_STATE_MASK;

	//
	// If SYN_RCVD sent or SYN_SENT set SYN, of SYN+SENT set ACK
	//
	if(t <= TCP_SYN_RCVD)
	{
		flags|=TCP_SYN_FLAG;
		set_timer=1;
	}
	// need to send FIN? we do if we are in TCP_FIN_WAIT_1 or TCP_CLOSE_WAIT
	if((t==TCP_CLOSE_WAIT) || (t==TCP_FIN_WAIT_1))
	{
		flags|=TCP_FIN_FLAG;
		set_timer=1;
	}
	// If data set push, we always set PSH on data
	if(len)
	{
		flags|=TCP_PSH_FLAG;
		set_timer=1;
	}
	// Should we set reset?
	if(ts->state & TCP_RST)
	{
		flags|=TCP_RST_FLAG;
		ts->state=TCP_CLOSED;
	}
	// else if not syn sent, set ack flag.
	else if(t != TCP_SYN_SENT)
		flags|=TCP_ACK_FLAG;

	//
	// Build TCP header
	//
	tcp->source_port=ts->source_port;
	tcp->dest_port	=ts->dest_port;
	tcp->seqnum		=htonl(ts->send_next);
	tcp->acknum		=htonl(ts->receve_next);
	tcp->flags		=flags;
	tcp->window		=htons(128);
	tcp->len		= 5 << 4;
	tcp->checksum	=0;

	// Special case, set seq number to send_next-1
	if((t==TCP_CLOSE_WAIT) || (t==TCP_SYN_SENT) || (t== TCP_FIN_WAIT_1) )
		tcp->seqnum		=htonl(ts->send_next-1);


	//ts->next_ack+=len;
	//  +++ dangerous fix
	ts->send_next+=len;

	
	//
	// Send the TCP MSS option if sending a SYN
	//
	if((t==TCP_SYN_RCVD) || (t==TCP_SYN_SENT))
	{
		//
		// Send the TCP MSS for our side of the connection.
		//
		tcp->data[0]=2;
		tcp->data[1]=4;
		tcp->data[2]=(TCP_MSS >> 8);
		tcp->data[3]=(TCP_MSS & 0xff);
		tcp->len		= 6 << 4;
		oplen=4;
	}
	//
	// Build IP header
	//
	ip->version=IP_VERSION;
	ip->tos=mm.ip_tos;
	ip->length=htons((len+sizeof(IP)+sizeof(TCP)+oplen));
	ip->id=ipid++;
	ip->ttl=mm.ip_ttl;
	ip->flags=0;
	ip->protocol=PROT_TCP;
	ip->source_ip.ip32=mm.our_ipaddr.ip32;
	ip->dest_ip.ip32=ts->dest_ip.ip32;
	ip->checksum=0;
	ip->checksum=Checksum_16(0, (U16 *)ip, 20);

    //
	// Do the checksum
	//
	// Build Psudo header checksum
	tchk=Checksum_16(0,(U16 *)&ip->source_ip,8);
	protocol=htons((len+sizeof(TCP)+oplen));
	tchk=Checksum_16(tchk,(U16 *)&protocol,2);
	protocol=htons(PROT_TCP);
	tchk=Checksum_16(tchk,(U16 *)&protocol,2);
	//
	// Build TCP checksum and store it
	tchk=Checksum_16(tchk,(U16 *)&tcp->source_port,(U16)(sizeof(TCP)+oplen));
	tchk=Checksum_16(tchk,(U16 *)data,len);
	tcp->checksum=tchk;
	DEBUG2("checksum = %x\n",tchk);

	//
	// Sent the packet
	//
//if(test==0)
	ip_send(ip,data,(U16)((sizeof(IP)+sizeof(TCP)+oplen)),len);	// need to add in option size if necessary
//test=0;
	
	//
	// If this is a data pkt put it on the resend list, and ack will clear it.
	//
	if(len)
	{
		// fill memblock ack so we can free it via acks
		memblock->ack=(U16)ts->send_next; //+len;   16 bits! warning
		memblock->next=0;
		memblock->len=len+sizeof(IP)+sizeof(TCP);
		// insert the
		if(0!=ts->packets)
		{
			// find the end of list
			pkt=ts->packets;
			while(0!=pkt->next)
				pkt=pkt->next;
			// insert at end of lest
			pkt->next=memblock;
		}
		else
		{
			// Put it at the front of the list
			ts->packets=memblock;
		}
	}
	else
	{
		// 
		// Not a datapacket, we don't need to keep it around.
		//
		MEM_FreeBlock(memblock);
	}
	//
	// Set the timer if needed
	//
	if(set_timer)
	{
		ts->timer=TIMER_Get_TCP_HundredMS();//TIMER_Get_Tick();
	}
#endif
	return(len);
}


//--------------------------------------------------------------------------------------
//
// tcp_clean_socket(socket_num) - clean up memory associated with socket and close socket
//
//--------------------------------------------------------------------------------------
void
tcp_clean_socket(TCPSOC *ts)
{
	MEMBLOCK	*memblock,*nxt;


		// reset state to zero
		ts->state=0;
		// clean up memory blocks hanging
		memblock=ts->packets;
		while(memblock)
		{
			nxt=memblock->next;
			MEM_FreeBlock(memblock);
			memblock=nxt;
		}
		ts->packets=0;

}




