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

#ifndef __GLOBAL_H__
#define __GLOBAL_H__
/*! \file globals.h
    \brief globals header file, global vars for memory map

*/
/*!																www.mycal.net			
 *---------------------------------------------------------------------------
 *! \file globals.c
 *  \brief Global Config Vars for the IMM project, configurable memory map
 *																			-
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 3, 2003									-        
 *
 *---------------------------------------------------------------------------    
 *                                                             				-
 * Copyright (C) 2003, Mycal Labs www.mycal.net								-
 *                                                                         	-
 *---------------------------------------------------------------------------
*/

// SerialConfig byte equates
#define MODEM_RTS 			0x1						// Set if using modem RTS
#define MODEM_CTS			0x2						// Set if using modem port CTS
#define CONSOLE_RTS			0x4						// Set if using console RTS
#define CONSOLE_CTS			0x8						// Set if using console CTS
#define RI_PASSTHROUGH	    0x10					// Pass through RI indicator
#define CD_PASSTHROUGH		0x20					// Pass through CD indicator 
#define AUTOBAUD			0x80					// Autobaud Console Port





#ifdef WINDOWS
#pragma pack(1)
#endif
typedef struct	MEMMAP_
{  
	U8			IMMConfig;				// 0x20 IMM configuration Byte |DS|DPPP|BB|DE|IE|R|ECHO_STREAM|ECHO_CMD|
	U8			escapeChar;				// 0x21 this is the char used for IMM escape to cmd mode
	U8			escape_timeout;			// 0x22 Guard Time
	U8			TCPStreamTickTime;		// 0x23 TCP send timer
	U8			UDPStreamTickTime;		// 0x24 UDP send timer
	U8			dns_timeout;			// 0x25 DNS retry time
	// Serial Ports
	U8			SerialConfig;			// 0x26 Serial Config Byte |CAB|CD|CD|x|CCTS|CRTS|MCTS|MRTS|
	U8			ModemBaud;				// 0x27 ModemBaudRate
	U8			ConsoleBaud;			// 0x28 ConsoleBaudRate
	U8			DialTimeout;			// 0x29 Dialup Timeout. 
	// PPP configuration Vars -do we need retry counts
	U8			ppp_connect_timeout;	// 0x2a PPP Connect timeout in seconds
	U32			ppp_rx_accm;			// 0x2b ACCM for rx link, we neg this
	// UDP config
	U8			udp_flags;				// 0x2f
	// TCP configuration Vars
	U8			tcp_connect_timeout;	// 0x30 In Seconds
	U16			tcp_retransmit_timeout;	// 0x31 In 100's of MS
	// IP configuration vars
	U8			ip_ttl;					// 0x33 IP time to live
	U8			ip_tos;					// 0x34 IP type of service
	// IP address info
	IPAddr		our_ipaddr;				// 0x35
	IPAddr		peer_ip_addr;			// 0x39
	IPAddr		pri_dns_addr;			// 0x3d
	IPAddr		sec_dns_addr;			// 0x41
		//
	// PPP username paramters
	//
	U8			user_name[40];			// 0x45
	U8			user_pass[40];			// 0x6D
	//
	// Read Only, or status only regesters
	//
	U8			ircmd_state;			// 0x95 command processor state (warning about writing)
	U8			ppp_flags;
	U8			ppp_flags2;
	U8			lcp_state;
	U8			pap_state;
	U8			ipcp_state;
	U16			ppp_tx_mru;
	U32			ppp_tx_accm;				// This is what they want us to xmit we RX conf_req
#ifdef __AVR__	
}MEMMAP __attribute__ ((packed));
#else
}MEMMAP;
#endif

extern	MEMMAP mm;

/*
//
// IMM Command Processor configuration Vars
//
extern U8			IMMConfig;				// IMM configuration Byte
extern U8			escapeChar;				// this is the char used for IMM escape to cmd mode
extern U8			TCPStreamTickTime;		// TCP send timer
extern U8			UDPStreamTickTime;		// UDP send timer

// PPP configuration Vars -do we need retry counts
extern U8			ppp_connect_timeout;	// PPP Connect timeout in seconds
extern U32			ppp_ACCM;				// ACCM for link


// IP configuration vars
extern IPAddr		our_ipaddr;
extern IPAddr		peer_ip_addr;
extern IPAddr		pri_dns_addr;
extern IPAddr		sec_dns_addr;
extern U8			ip_ttl;
extern U8			ip_tos;

// TCP configuration Vars

extern U8			tcp_connect_timeout;	// In Seconds
extern U16			tcp_retransmit_timeout;	// In 100's of MS

//
// Read Only, or status only regesters
//
extern U8			ircmd_state;			// command processor state (warning about writing)
extern U8			ppp_flags;
extern U8			lcp_state;
extern U8			pap_state;
extern U8			ipcp_state;
extern U16			ppp_tx_mru;
*/

U8	global_init(U8 cmd);

#endif

