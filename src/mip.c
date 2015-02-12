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
 * mip.c - Mycal IP Layer													-
 *																			-
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 3, 1998									-        
 *
 * Exports:
 *	void	ip_upcall(U8 * buffer, U16 len)
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
#include "ahdlc.h"
#include "mip.h"
#include "mudp.h"
#include "mtcp.h"
#include "memory.h"
#include "globals.h"

#include "ppp.h"
#include "debug.h"

void	(*ICMP_Callback)(struct ip_h *header,ICMP *data, U16 len);
U16				ipid;

//U8				headerbuffer[62];

//****************************************************************************
// ip_upcall()
//******************************************************************************
void
ip_upcall(IP *ip, U16 len)
{

	// we could check flags and frags here.

	// Check Checksum, checksum should be zero for the header.
	if(Checksum_16(0, (U16 *)ip, 20))
	{
		// bad IP checksum, toss
		return;
	}

	// Check frag flag, if so drop 
	if(ntohs(ip->flags) & IP_F_MF)
	{ 
		DEBUG3("Frag pkt\n");
		return;
	}

	// Demux based on protocol.
	switch(ip->protocol)
	{
	case PROT_ICMP:
		DEBUG2("ICMP pkt\n"CRLF);
		icmp_upcall(ip,(ICMP *)ip->transport , ntohs(ip->length));
		break;
	case PROT_UDP:
		DEBUG2("UDP pkt\n"CRLF);
		udp_upcall(ip, (UDP *)ip->transport, ntohs(ip->length));
		break;
	case PROT_TCP:
		DEBUG2("TCP pkt"CRLF);
		tcp_upcall(ip, (TCP *)ip->transport, ntohs(ip->length));
		break;
	default:
		// Silently Discard all other protocols, maybe a counter here in the future.
		break;
	}
}

//****************************************************************************
// ip_send() - IP send routine
//****************************************************************************
void
ip_send(IP *ip, U8 *data, U16 hlen, U16 dlen)
{
	// Send packet ahdlc_txz(procol,header,data,headerlen,datalen);
	ahdlc_tx(IPV4, (U8 *)ip, data, hlen, dlen);
	//ahdlc_tx(IPV4, header, data, U16 len);
}




//																	www.mycal.net
//*******************************************************************************
// Checksum_16(seed, buffer, len)
//	Calculates checksums for IP headers and TCP, UDP and ICMP packets.
//
//	Set seed to zero for new checksum calculation, or from previous result
//	to continue checking checksum.
//  
//*******************************************************************************
U16
Checksum_16(U16 start,U16 *data, U16 len)
{
U16				sum=~start;

	// Main Checksum Loop
	while(len > 1)
	{
		len-=2;
		sum += *data;
		if(sum < *data)
			sum++;		// Add in carry
 		data++;
	}
	// Check for odd byte, if there handle it.
	if(len == 1) 
	{
		len=*data & 0xff;
		sum+=len;
		if(sum<len)
			sum++;
    }
  	// Return the one's complement
	return (~sum);
}


void
ICMP_Echo_Callback(void (*callback)(struct ip_h *header,ICMP *data, U16 len))
{
	ICMP_Callback=callback;
}

void
ICMP_Echo_Terminate()
{
	ICMP_Callback=0;
}

U8
ICMP_Send_Ping(IPAddr *dest_ip, U16 len)
{
MEMBLOCK	*memblock;
IP			*ip;
ICMP		*icmp;
U16			i;

	if(len>64)
		len=64;

	memblock=MEM_GetBlock();
	if(memblock)
	{
		ip=(IP *)memblock->data;
		//
		ip->version=IP_VERSION;
		ip->tos=mm.ip_tos;
		ip->length=htons( (len+sizeof(IP)+sizeof(ICMP)) );
		ip->id=ipid++;
		ip->ttl=mm.ip_ttl;
		ip->flags=0;
		ip->protocol=PROT_ICMP;
		ip->source_ip.ip32=mm.our_ipaddr.ip32;
		ip->dest_ip.ip32=dest_ip->ip32;
		ip->checksum=0;
		ip->checksum=Checksum_16(0, (U16 *)ip, 20);

		// Build ECHO Request packet
		icmp=(ICMP *)ip->transport;
		icmp->type=ICMP_ECHO_REQUEST;
		icmp->code=0;
		icmp->checksum=0x0000;
		icmp->id=1;
		icmp->seq=1;

		for(i=0;i<len;i++)
			icmp->data[i]=(i&0xff);

		icmp->checksum=Checksum_16((U16)0, (U16 *)icmp, (U16)(len+sizeof(ICMP)) );
		
		ip_send(ip,(U8 *)0,(U16)(sizeof(IP)+sizeof(ICMP)+len),0);	// IP_Send(header,data,len);

		MEM_FreeBlock(memblock);
		return(1);
	}
	return(0);
}

//-----------------------------------------------------------------------------
// icmp_upcall()
//-----------------------------------------------------------------------------
void
icmp_upcall(IP *ip, ICMP *icmp, U16 len)
{
IPAddr	t;
	//
	// Check the ICMP checksum (we can omit this if we want)
	//

	switch(icmp->type)
	{
    case	ICMP_ECHO_REPLY:
			// Allow callback here, call it if defined.
			if((void *)ICMP_Callback!=0)
			{
				ICMP_Callback(ip,icmp,len);
			}
			break;
    case	ICMP_ECHO_REQUEST:
			//
			// Build reply inline
			//
			icmp->type=ICMP_ECHO_REPLY;
			icmp->checksum-=0x0800;				// Fixup checksum
			t.ip32=ip->source_ip.ip32;
			ip->source_ip.ip32=ip->dest_ip.ip32;
			ip->dest_ip.ip32=t.ip32;
			//
			// Send it out the link,  this should be a RAW IP Send, abstract lower layer there
			//
			// Send packet ahdlc_txz(procol,header,data,headerlen,datalen);
			ahdlc_tx(IPV4, 0, (U8 *)ip, 0, len);
	  		break;
#if 0
    case	ICMP_DEST_UNREACHABLE:
    case 	ICMP_SOURCE_QUENCH:
    case	ICMP_REDIRECT: 
    case	ICMP_ROUTER_ADVERT:
    case	ICMP_ROUTER_SOLICIT:
    case	ICMP_TIME_EXCEEDED:
    case	ICMP_PARAMETER_PROBLEM:
    case	ICMP_TIMESTAMP_REQEST:
    case	ICMP_TIMESTAMP_REPLY:
    case	ICMP_MASK_REQUEST:
    case	ICMP_MASK_REPLY:
		break;
#endif    
	default:
			//icmp_badtype++;
		break;
	}
}


