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

#ifndef __MIP_H__
#define __MIP_H__
/*																www.mycal.net			
 *---------------------------------------------------------------------------
 * mip.h - Mycal IP Layer header file										-
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

#include "mTypes.h"
#include "micmp.h"


//------------------------------
// Define IP header defaults   -
//------------------------------
#define	IP_VERSION	    0x45			// IP version 4 length 5 (20 U8s)    	
#define IP_TOS_DEFAULT  0x00			// default to normal
#define IP_TTL_DEFAULT  0x64            // Live for 64 hops      
#define IP_HEADER_SIZE	20
//
// Define IP Flags
//                 
#define IP_F_DF			0x4000			// Don't fragment
#define IP_F_MF			0x2000			// More Fragments
#define IP_FRAG_MASK	0x1fff			// Mask for fragmenting bits
//
// Define broadcast
//
#define IP_BROADCAST	0xffffffff
                    
//
// Define the ethernet type for IP
//                    
#define ETHERTYPE_IP	0x0008			// 0x0800 in network order

//
// IP protocol types, for IP injected header checksums
//
#define PROT_ICMP		1
#define PROT_TCP		6
#define PROT_UDP		17  

typedef struct ip_h
{ 
	U8				version;
	U8				tos;
    U16             length;
    U16             id;
	U16				flags;
	U8				ttl;
	U8				protocol; 
	U16 			checksum;
	IPAddr			source_ip;
	IPAddr			dest_ip;
	U8				transport[];
}IP;


//
// Exported globals
//
extern	U16		ipid;
extern	IPAddr			our_ipaddr;
extern	IPAddr			peer_ip_addr;
extern	IPAddr			pri_dns_addr;
extern	IPAddr			sec_dns_addr;
//extern	U8				headerbuffer[62];
//
// Exported function calls
//
void	ip_upcall(IP *ip, U16 len);
U16		Checksum_16(U16 start,U16 *data, U16 len);
void	icmp_upcall(IP *ip, ICMP *icmp, U16 len);
//
// Private Functions
//


//
// Application Calls (API)
//

void ip_send(IP *ip, U8 *data, U16 hlen, U16 dlen);
void ICMP_Echo_Callback(void (*callback)(struct ip_h *header,ICMP *data, U16 len));
void ICMP_Echo_Terminate(void);
U8	 ICMP_Send_Ping(IPAddr *dest_ip, U16 len);

#endif
