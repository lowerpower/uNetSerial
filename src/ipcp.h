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

#ifndef __IPCP_H__
#define __IPCP_H__
//																www.mycal.net
//---------------------------------------------------------------------------
// IPCP.h - Internet Protocol Control Protocol header file      			-
//---------------------------------------------------------------------------
// Version                                                                  -
//		0.1 Original Version June 3, 2000									-
//																			-
// (c)2000 Mycal Labs, All Rights Reserved									-
//---------------------------------------------------------------------------
#include "mTypes.h"
#include "pppconfig.h"

// Config options (move to pppconfig)
//#define		IPCP_RETRY_COUNT		5
//#define			IPCP_TIMEOUT			5

// IPCP Option Types
#define IPCP_IPADDRESS		0x03
#define IPCP_PRIMARY_DNS	0x81
#define IPCP_SECONDARY_DNS	0x83

// IPCP state machine flags
#define	IPCP_TX_UP			0x01
#define IPCP_RX_UP			0x02
#define IPCP_IP_BIT			0x04
#define IPCP_TX_TIMEOUT		0x08
#define IPCP_PRI_DNS_BIT	0x08
#define IPCP_SEC_DNS_BIT	0x10
#define IPCP_SHUTDOWN		0x20

typedef struct  _ipcp
{
	U8				code;
	U8				id;
	U16				len;
	U8				data[];	
} IPCPPKT;

//
// Export IP addresses.
//
#if 0	// moved to mip.c
extern	IPAddr			our_ipaddr;
extern	IPAddr			peer_ip_addr;
extern	IPAddr			pri_dns_addr;
extern	IPAddr			sec_dns_addr;
#endif

extern	U8		ipcp_state;

void	ipcp_init(void);
void	ipcp_task(U8 *buffer);
void	ipcp_rx(U8 *, U16);

#endif

