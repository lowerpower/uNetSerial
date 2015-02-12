#ifndef __MIP_H__
#define __MIP_H__
/*																www.mycal.com			
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
 * Copyright (C) 1998, Mycal Labs www.mycal.com								-
 *                                                                         	-
 *---------------------------------------------------------------------------
 * Notes:
 *
*/

#include "mTypes.h"


//------------------------------
// Define IP header defaults   -
//------------------------------
#define	IP_VERSION	    0x45			// IP version 4 length 5 (20 U8s)    	
#define IP_TOS          0x00			// default to normal
#define IP_TTL          0x64            // Live for 64 hops      
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

typedef struct	ip_h
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
// Exports
//
void	ip_upcall(U8 *buffer, U16 len);


U16		Checksum_16(U16 start,U16 *data, U16 len);


//
// Private Functions
//


#endif