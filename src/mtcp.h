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

#ifndef __MTCP_H__
#define __MTCP_H__
/*! \file mtcp.h
    \brief tcp state machine and interface header file..

*/
/*																www.mycal.net			
 *---------------------------------------------------------------------------
 * mtcp.h - Mycal TCP Layer header file									-
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

#include "mTypes.h"
#include "mip.h"
#include "memory.h"

    
#define MAX_TCP_SOCKETS		2
#define TCP_MSS				128
#define TCP_CONNECT_TIMOUT	12				/* 12 seconds */
#define TCP_TIMETOUT		25				/* retransmit timeout 2.5 seconds (should be configurable/dynamic) */



//
// /struct TCP Header Structure
//
typedef struct	tcp_h
{  
	U16		source_port;
	U16		dest_port;
	U32		seqnum;
	U32		acknum;
	U8		len;
	U8		flags;
	U16		window;
	U16		checksum;
	U16		urg_ptr;
	U8		data[];
}TCP;  

//
// /struct TCP socket structure (TCP control block)
//
typedef struct	tcp_socket_h
{  
	U8			state;				// socket State
	U16			source_port;		// source port
	U16			dest_port;			// destination port
	IPAddr		dest_ip;			// Peers IP address
	U32			receve_next;		// Next Sequence number that we expect to receive
	U32			send_next;			// Last Sequence that we sent.
	U32			un_ack;				// oldest UN ACKed sequence
	S16			(*socket_callback)(U8 soc, U8 *data, U16 len);
	U16			timer;				// Retransmission Timer
	U8			retry;				// Retransmission count
	MEMBLOCK	*packets;			// packets in flight.
}TCPSOC;   

//
// /struct TCP RST socket struct (to send a reset on an an unknown port)
//
typedef struct	tcp_rst_socket_h
{  
	U8			state;				// socket State
	U16			source_port;		// source port
	U16			dest_port;			// destination port
	IPAddr		dest_ip;			// Peers IP address
	U32			receve_next;		// Next Sequence number that we expect to receive
	U32			send_next;			// Last Sequence that we sent.
}TCPRSTSOC; 


//
// TCP states
//
#define		TCP_CLOSED		0x0
#define		TCP_LISTEN		0x1
#define		TCP_SYN_SENT	0x2
#define		TCP_SYN_RCVD	0x3
#define		TCP_OPEN		0x4
#define		TCP_CLOSE_WAIT	0x5
#define		TCP_LAST_ACK	0x6
#define		TCP_FIN_WAIT_1	0x7
#define		TCP_FIN_WAIT_2	0x8
#define		TCP_CLOSING		0x9
#define		TCP_TIME_WAIT	0xa
#define		TCP_RST_SOCKET	0xb
#define		TCP_STATE_MASK	0x0f
//
// TCP action bits
//
#define		TCP_RST			0x10
#define		TCP_NEED_SEND	0x20

//
// TCP flags bitmask
//
#define		TCP_FIN_FLAG	0x1
#define		TCP_SYN_FLAG	0x2
#define		TCP_RST_FLAG	0x4
#define		TCP_PSH_FLAG	0x8
#define		TCP_ACK_FLAG	0x10
#define		TCP_URG_FLAG	0x20


//
// Private fucntions
//
S16		tcp_sendit(TCPSOC *ts,U8 * data, U16 len);

//
// Exported Functions
//
/*! \fn void tcp_upcall(struct ip_h * ipptr, TCP * tcpptr, U16 len)
    \brief Receive packet to feed into the tcp engine.

    \param ipptr pointer to the IP header
    \param tcpptr pointer to the TCP header
	\param len length of the packet
*/
void	tcp_upcall(struct ip_h *, TCP *, U16 );


//
// Application Calls (API);
//
/*! \fn void tcp_init(void)
    \brief Initialize the TCP engine
*/
void	tcp_init(void);

/*! \fn void tcp_process(U8 flags)
    \brief process timeouts and such, must call at least every 250ms

	\param flags unused right now
*/
void	tcp_process(U8 flags);

/*! \fn U8 tcp_open(IPAddr dest_ip, U16 dest_port, void (*callback)(U8 socket, U8 *data, U16 len))
    \brief open a tcp connection.
*/
U8		tcp_open(IPAddr dest_ip, U16 dest_port, void (*callback)(U8 socket, U8 *data, U16 len));

U8		tcp_listen(U16 port, void (*callback)(struct ip_h *header,U8 *data, U16 len));
U8		tcp_close(U8 socket);
S16		tcp_tx(U8 socket,U8 *data, U16 len);
S16		tcp_status(U8 socket);
S16		tcp_sport(U8 socket);
void	tcp_clean_socket(TCPSOC *ts);

#endif
