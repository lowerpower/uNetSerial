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

#ifndef __PPP_H__
#define __PPP_H__
//																www.mycal.net
//---------------------------------------------------------------------------
// ppp.h - ppp header file      											-
//---------------------------------------------------------------------------
// Version                                                                  -
//		0.1 Original Version June 3, 2000									-
//																			-
// (c)2000 Mycal Labs, All Rights Reserved									-
//---------------------------------------------------------------------------

#include "mTypes.h"
//#include "pppconfig.h"
//#include "ahdlc.h"
//#include "pap.h"
//#include "lcp.h"
//#include "ipcp.h"
//#include "mip.h"
#include "debug.h"

// moved to pppconfig.h
//#define PPP_RX_BUFFER_SIZE	1024 
//#define PPP_TX_BUFFER_SIZE	64

#define CRC_GOOD_VALUE		0xf0b8

// ppp_rx_status values
#define	PPP_RX_IDLE			0
#define PPP_READY			1

// ppp flags
#define PPP_ESCAPED			0x1
#define	PPP_RX_READY		0x2
#define	PPP_RX_ASYNC_MAP	0x8
#define PPP_TX_ASYNC_MAP	0x8
#define PPP_PFC				0x10		// PFC
#define PPP_ACFC			0x20		//
#define PPP_STARTED			0x40
#define PPP_UP				0x80

// ppp flags2
#define PPP_PASSIVE			0x1

// Supported PPP Protocols
#define LCP					0xc021
#define PAP					0xc023
#define IPCP				0x8021
#define	IPV4				0x0021

// LCP codes packet types
#define CONF_REQ			0x1			
#define CONF_ACK			0x2
#define CONF_NAK			0x3
#define CONF_REJ			0x4
#define TERM_REQ			0x5
#define TERM_ACK			0x6
#define PROT_REJ			0x8

// Raise PPP config bits
#define USE_PAP				0x1
#define USE_NOACCMBUG		0x2
#define USE_GETDNS			0x4

// PPP Error Codes
#define PPP_OK				1
#define PPP_TIMEOUT			0xff
#define PPP_AUTH_FAIL		0xfe

// PAP constants

// PAP state machine flags (moved to pppconfig.h)
//#define PAP_TX_TIMEOUT		0x80


extern U8	ppp_rx_buffer[];
extern U8	ppp_rx_count;

extern U16	ppp_crc_error;

extern U8	ppp_flags;
extern U8	ppp_status;
extern U16	ppp_rx_crc;
extern U16	ppp_rx_tobig_error;
extern U8	ppp_lcp_state;

extern U8	ppp_id;
extern U8	ppp_retry;

extern U8	ppp_tx_buffer[];



void	init_ppp(void);
void	ppp_down(void);
S16		raise_ppp(U8,U8*, U8*);
S8		ppp_auto_enable(U8 *, U8 *);
void	ppp_passive(void);
S8		ppp_shut_down(void);
S8		ppp_task(void);
U8		ppp_sendpacket(U8 *,U16);
void	ppp_rx_ready(void);
void	ppp_upcall(U16, U8 *, U16);

void	ppp_rx_lcp(U8 *, U16);
void	ppp_reject_protocol(U16 protocol,U8 *buffer,U16 len);

#ifdef PACKET_RX_DEBUG
void	dump_ppp_packet(U8 *,U16);
#endif


void	reset_ppp(void);
void	mputch(U8);

U16		scan_packet(U16, U8 *list, U8 *buffer, U8 *options, U16 len);
U8		check_ppp_errors(void);

#endif 
