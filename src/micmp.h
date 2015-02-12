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

#ifndef __MICMP_H__
#define __MICMP_H__
/*																www.mycal.net			
 *---------------------------------------------------------------------------
 * micmp.h - Mycal ICMP Layer header file									-
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

//
// ICMP types
//           
#define	ICMP_ECHO_REPLY			0
#define ICMP_DEST_UNREACHABLE   3
#define ICMP_SOURCE_QUENCH		4
#define	ICMP_REDIRECT			5
#define	ICMP_ECHO_REQUEST		8
#define ICMP_ROUTER_ADVERT		9
#define ICMP_ROUTER_SOLICIT		10
#define ICMP_TIME_EXCEEDED		11
#define ICMP_PARAMETER_PROBLEM	12
#define ICMP_TIMESTAMP_REQEST	13
#define ICMP_TIMESTAMP_REPLY	14
#define ICMP_MASK_REQUEST		17
#define ICMP_MASK_REPLY			18       


//
// Define the ICMP packet type
//
typedef struct	icmp_h
{
	U8	type;               	// the above defined types
	U8	code;
	U16	checksum;  
	U16	id;
	U16	seq; 
	U8	data[];
}ICMP;





#endif
