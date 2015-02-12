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

/*! \file globals.c
    \brief globals definition and initialization memory map

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

/*					*/ 
/* include files 	*/
/*					*/ 
#include	"pppconfig.h"
#include	"mTypes.h"
#include	"ircmd.h"
#include	"ppp.h"
#include	"ahdlc.h"
#include	"mip.h"
#include	"mtcp.h"
#include	"mudp.h"
#include	"console.h"
#include	"resolve.h"
#include	"modem.h"
#include	"memory.h"
#include	"debug.h"
#include	"globals.h"

/*
// Serial config byte equates
#define MODEM_RTS 			0x1						// Set if using modem RTS
#define MODEM_CTS			0x2						// Set if using modem port CTS
#define CONSOLE_RTS			0x4						// Set if using console RTS
#define CONSOLE_CTS			0x8						// Set if using console CTS
#define RI_PASSTHROUGH	    0x10					// Pass through RI indicator
#define CD_PASSTHROUGH		0x20					// Pass through CD indicator 
*/

#if 0
typedef struct	MEMMAP_
{  
	U8			IMMConfig;				// IMM configuration Byte |DS|DPPP|x|DE|IE|R|ES|EC|
	U8			escapeChar;				// this is the char used for IMM escape to cmd mode
	U8			escape_timeout;			// this is the guard time between data and escap chars
	U8			TCPStreamTickTime;		// TCP send timer
	U8			UDPStreamTickTime;		// UDP send timer
	// Serial Ports
	U8			SerialConfig;			// Serial Config Byte |CAB|CD|CD|x|CCTS|CRTS|MCTS|MRTS|
	U8			ModemBaud;				// ModemBaudRate
	U8			ConsoleBaud;			// ConsoleBaudRate
	U8			DialTimeout				// Dialup Timeout. 
	// PPP configuration Vars -do we need retry counts
	U8			ppp_connect_timeout;	// PPP Connect timeout in seconds
	U32			ppp_ACCM;				// ACCM for link 
	// UDP config
	U8			udp_flags;				// |UDPDAVCHAR|DAVDSR|DAVDAV|DIRECT|LEN|IP|PORT
	// TCP configuration Vars
	U8			tcp_connect_timeout;	// In Seconds
	U16			tcp_retransmit_timeout;	// In 100's of MS
	// IP configuration vars
	U8			ip_ttl;
	U8			ip_tos;
	// IP address info
	IPAddr		our_ipaddr;
	IPAddr		peer_ip_addr;
	IPAddr		pri_dns_addr;
	IPAddr		sec_dns_addr;
	//
	// PPP username paramters
	//
	U8			user_name[40];
	U8			user_pass[40];
	//
	// Read Only, or status only regesters
	//
	U8			ircmd_state;			// command processor state (warning about writing)
	U8			ppp_flags;
	U8			ppp_flags2;
	U8			lcp_state;
	U8			pap_state;
	U8			ipcp_state;
	U16			ppp_tx_mru;
}MEMMAP; 
#endif


MEMMAP	mm;

//
// IMM Command Processor configuration Vars
//
/*
#pragma pack( 1)
U8			IMMConfig;				// IMM configuration Byte |DS|DPPP|x|DE|IE|R|ES|EC|
U8			escapeChar;				// this is the char used for IMM escape to cmd mode
U8			escapeTimeout;			// inband escape guard time in 10's MS
U8			TCPStreamTickTime;		// TCP send timer
U8			UDPStreamTickTime;		// UDP send timer

// Serial Ports
U8			SerialConfig;			// Serial Config Byte |CAB|CD|CD|x|CCTS|CRTS|MCTS|MRTS|
U8			ModemBaud;				// ModemBaudRate
U8			ConsoleBaud;			// ConsoleBaudRate

// PPP configuration Vars -do we need retry counts
U8			ppp_connect_timeout;	// PPP Connect timeout in seconds
U32			rx_accm;				// ACCM for link 
// TCP configuration Vars
U8			tcp_connect_timeout;	// In Seconds
U16			tcp_retransmit_timeout;	// In 100's of MS

// IP configuration vars
U8			ip_ttl;
U8			ip_tos;

// IP address info
IPAddr		our_ipaddr;
IPAddr		peer_ip_addr;
IPAddr		pri_dns_addr;
IPAddr		sec_dns_addr;

//
// Read Only, or status only regesters
//
U8			ircmd_state;			// command processor state (warning about writing)
U8			ppp_flags;
U8			lcp_state;
U8			pap_state;
U8			ipcp_state;
U16			ppp_tx_mru;
U32			tx_accm
*/

/*! \fn U8 global_init(U8 cmd)
    \brief Initializes the global vars

    \param cmd	0=load default	1=load eeprom
*/
U8
global_init(U8 cmd)
{
U8	count,*ptr=(U8 *)&mm;

#ifdef __AVR__
	if(1==cmd)
	{
		// Try to load eeprom, 0xa5 is the flag that means eeprom is good.
		// uint8_t 	eeprom_read_byte (const uint8_t *addr)
		if((cmd) && (0xa5==eeprom_read_byte(0)))
		{
		//	DEBUG1("Load EEPROM"CRLF);
			// Load the config from the eeprom
			for(count=0;count<sizeof(MEMMAP)-7;count++)
				ptr[count]=eeprom_read_byte((count+0xa));
		}
		else
		{
		//	DEBUG1("EEPROM not valid ");
			// eeprom is not valid, load defaults
			cmd=0;
		}
	}
	if(2==cmd)
	{
		// Validate the eeprom
		eeprom_write_byte (0,0xa5);
		//
		for(count=0;count<sizeof(MEMMAP)-7;count++)
			eeprom_write_byte((count+0xa),ptr[count]);

	}
#else
	cmd=0;
#endif
	if(0==cmd)
	{
		// 
		// Set defaults
		//
		//DEBUG1("Load Defaults"CRLF);
#ifdef __AVR__
		mm.IMMConfig					= BOOT_BANNER | ECHO_CMD | ECHO_STREAM | RESULT ;
#else
		mm.IMMConfig					= BOOT_BANNER | PPP_DEBUG | ECHO_CMD | ECHO_STREAM | RESULT ;		
#endif
		mm.escapeChar					= '+';
		mm.escape_timeout				= ESCAPE_TIMEOUT;
		mm.TCPStreamTickTime			= 200;		
		mm.UDPStreamTickTime			= 200;
		mm.SerialConfig					= 0;							// serial config
		mm.ModemBaud					= MODEM_BAUD_STATIC;			// ModemBaudRate
		mm.ConsoleBaud					= CONSOLE_BAUD_STATIC;			// ConsoleBaudRate
		
		mm.DialTimeout					= 30;	// 30 seconds.
		mm.ppp_connect_timeout			= 15;	// 30 seconds.
		mm.ppp_tx_accm					= 0;	// don't escape anything

		mm.ip_ttl						= IP_TTL_DEFAULT; 
		mm.ip_tos						= IP_TOS_DEFAULT;
		
		mm.tcp_connect_timeout			= TCP_CONNECT_TIMOUT;
		mm.tcp_retransmit_timeout		= TCP_TIMETOUT;
		mm.dns_timeout					= DNS_REPLY_WAIT_TIME;	
		mm.ppp_rx_accm					= 0;
	}

	return(cmd);
}


