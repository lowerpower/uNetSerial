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

#ifndef __PAP_H__
#define __PAP_H__
//																www.mycal.net
//---------------------------------------------------------------------------
// pap.h - pap header file      											-
//---------------------------------------------------------------------------
// Version                                                                  -
//		0.1 Original Version June 3, 2000									-
//																			-
// (c)2000 Mycal Labs, All Rights Reserved									-
//---------------------------------------------------------------------------
#include "mTypes.h"

// PAP state machine flags
#define PAP_TX_UP			0x01				// client only
#define PAP_RX_UP			0x02				// server only

#define PAP_RX_AUTH_FAIL	0x10
#define PAP_TX_AUTH_FAIL	0x20
#define PAP_RX_TIMEOUT		0x80
#define PAP_TX_TIMEOUT		0x80


typedef struct  _pappkt
{
	U8				code;
	U8				id;
	U16				len;
	U8				data[];	
} PAPPKT;

// Export pap_state
extern	U8				pap_state;


// Function prototypes
void	pap_init(void);
void	pap_rx(U8 *, U16);
//void	pap_task(void);
void	pap_task(U8 *, U8 *, U8 *);

#endif

