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
 * mudp.c - Mycal UDP Layer													-
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
#include "mudp.h"
#include "ppp.h"
#include "console.h"
#include "memory.h"
#include "globals.h"
#include "udpcmd.h"

#include "debug.h"



UDPSOC	udp_socket[MAX_UDP_SOCKETS];

//-----------------------------------------------------------------------------
// udp_init() - just init the udp socket table.
//-----------------------------------------------------------------------------
void
udp_init()
{
U8	i;
	
	for(i=0;i<MAX_UDP_SOCKETS;i++)
		udp_socket[i].port=0;
}



//-----------------------------------------------------------------------------
// udp_upcall() - this handles incoming UDP packets.
//-----------------------------------------------------------------------------
void
udp_upcall(IP *ip, UDP *udp, U16 len)
{
U16		tchk=0;
U8		i;
U16		protocol;

#ifdef DEBUG_LV3
	printf("\nudp upcall port %d\n"CRLF,udp->dest_port);
#endif
	//
	// Check the UDP checksum (we can omit this if we want)
	//
	// Checksum is ok if zero
	if(udp->checksum)	
	{
		//
		// First do psudo header
		//
		protocol=htons(PROT_UDP);
		//protocol=PROT_UDP;
		tchk=Checksum_16(0,(U16 *)&ip->source_ip,8);
		tchk=Checksum_16(tchk,(U16 *)&udp->length,2);
		tchk=Checksum_16(tchk,(U16 *)&protocol,2);
		tchk=Checksum_16(tchk,(U16 *)&udp->source_port,htons(udp->length));

		if(tchk!=0)
		{
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1("Bad UDP Checksum\n");
			}
			return;
		}
	}
	//
	// Demux on dest socket, just search list
	//
	for(i=0;i<MAX_UDP_SOCKETS;i++)
	{
		// look for a match
		if(udp_socket[i].port==htons(udp->dest_port))
		{
			// We have a Match, Call the app. (struct ip_h *header,U8 *data, U16 len));
#ifdef DEBUG_LV3
	printf("\ncalling socket callback for port %d\n"CRLF,udp->dest_port);
#endif
			udp_socket[i].socket_callback(ip,udp,len);
		}
	}
	if(i>=MAX_UDP_SOCKETS)
	{
		// Send Port Not Reachable ICMP message
#ifdef DEBUG_LV3
	printf("\no udp port bound to %d\n"CRLF,udp->dest_port);
#endif
	}

}


//-----------------------------------------------------------------------------
// udp_bind(port, port_callback) - bind a port to a UDP socket.  Requires a 
//		Callback routine per socket for receive packets.
//-----------------------------------------------------------------------------
U8
udp_bind(U16 port, void (*callback)(struct ip_h *header,U8 *data, U16 len))
{
U8	i;
	
	for(i=0;i<MAX_UDP_SOCKETS;i++)
	{
		if (0==udp_socket[i].port)
		{
			udp_socket[i].port=port;//htons(port);			// we convert here once, not every time on demux
			udp_socket[i].socket_callback=callback;
			return (i+1);
		}
	}
	return 0;
}


//-----------------------------------------------------------------------------
// udp_port(socket)
//-----------------------------------------------------------------------------
U16
udp_port(U8 socket)
{
	if(socket==0)
		return(0);
	else
		return(udp_socket[socket-1].port);
}

//-----------------------------------------------------------------------------
// udp_close(socket)
//-----------------------------------------------------------------------------
void
udp_close(U8 socket)
{
	if(socket==0)
		return;
	else
		udp_socket[socket-1].port=0;
}


//-----------------------------------------------------------------------------
// udp_tx(socket, peer_ip, peer_port, buffer, len)
//
// returns number of bytes sent, or -1 for send failed.	
// 
//-----------------------------------------------------------------------------
S16
udp_tx(IPAddr dest_ip, U16 dest_port, U16 src_port, U8 *data, U16 len)
{
IP			*ip;
UDP			*udp;
U16			protocol,tchk;
MEMBLOCK	*memblock;

	memblock=MEM_GetBlock();

	if(!memblock)
	{
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			CONSOLE_puts("UDP failed to allocate memblock"CRLF);
		}
#endif
		return(0);
	}

	ip=(IP *)memblock->data;
	udp=(UDP *)ip->transport;
	//
	//
	ip->version=IP_VERSION;
	ip->tos=mm.ip_tos;
	ip->length=htons((len+sizeof(IP)+sizeof(UDP)));
	ip->id=ipid++;
	ip->ttl=mm.ip_ttl;
	ip->flags=0;
	ip->protocol=PROT_UDP;
	ip->source_ip.ip32=mm.our_ipaddr.ip32;
	ip->dest_ip.ip32=dest_ip.ip32;
	ip->checksum=0;
	ip->checksum=Checksum_16(0, (U16 *)ip, 20);


	// Build UDP header.
	udp->dest_port=htons(dest_port);
	udp->source_port=htons(src_port);
	udp->checksum=0;
	udp->length=htons((sizeof(UDP)+len));

	//
	// Build Psudo header checksum
	protocol=htons(PROT_UDP);
	//protocol=PROT_UDP;
	tchk=Checksum_16(0,(U16 *)&ip->source_ip,8);
	tchk=Checksum_16(tchk,(U16 *)&udp->length,2);
	tchk=Checksum_16(tchk,(U16 *)&protocol,2);
	//tchk=Checksum_16(tchk,(U16 *)&ip->flags,2);
	//
	// Build UDP checksum and store it
	tchk=Checksum_16(tchk,(U16 *)&udp->source_port,sizeof(UDP));
	tchk=Checksum_16(tchk,(U16 *)data,len);
	udp->checksum=tchk;
	//printf("checksum = %x\n",tchk);
	
	ip_send(ip,data,(sizeof(IP)+sizeof(UDP)),len);	// IP_Send(header,data,len);
	
	MEM_FreeBlock(memblock);

	return(len);
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


