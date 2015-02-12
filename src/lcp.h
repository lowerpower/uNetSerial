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

#ifndef __ILCP_H__
#define __ILCP_H__
//																www.mycal.net
//---------------------------------------------------------------------------
// LCP.h - LCP header file      											-
//---------------------------------------------------------------------------
// Version                                                                  -
//		0.1 Original Version June 3, 2000									-
//																			-
// (c)2000 Mycal Labs, All Rights Reserved									-
//---------------------------------------------------------------------------
#include "mTypes.h"
#include "pppconfig.h"


// LCP Option Types
#define LPC_VENDERX			0x0
#define	LPC_MRU				0x1
#define LPC_ACCM			0x2
#define LPC_AUTH			0x3
#define LPC_QUALITY			0x4
#define LPC_MAGICNUMBER		0x5
#define LPC_PFC				0x7
#define LPC_ACFC			0x8

// LCP Negotiated options flag equates
//#define LCP_OPT_ACCM		0x1
//#define LCP_OPT_AUTH		0x2
//#define LCP_OPT_PFC			0x4
//#define LCP_OPT_ACFC		0x8
#//define LCP_OPT_MAGIC		0x10

// LCP state machine flags
#define	LCP_TX_UP			0x1
#define LCP_RX_UP			0x2
#define LCP_RX_AUTH			0x4						// LCP request for auth
#define LCP_TERM_PEER		0x8						// LCP Terminated by peer
#define LCP_OPT_MAGIC		0x10
#define LCP_OPT_PFC			0x20
#define LCP_OPT_ACFC		0x40
#define LCP_SHUTDOWN		0x80

typedef struct  _lcppkt
{
	U8				code;
	U8				id;
	U16				len;
	U8				data[];	
} LCPPKT;


// Exported Vars
extern	U8			lcp_state;


void	lcp_init(void);
void	lcp_rx(U8 *,U16);
void	lcp_task(U8 *buffer);


#endif
