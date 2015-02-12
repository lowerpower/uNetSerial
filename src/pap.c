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
 * pap.c - PAP processor for the PPP module									-
 *																			-
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version Jun 3, 2000									-        
 *																			-
 *---------------------------------------------------------------------------    
 *                                                             				-
 * Copyright (C) 2000, Mycal Labs www.mycal.net  							-
 *                                                                         	-
 *---------------------------------------------------------------------------
*/
/*					*/ 
/* include files 	*/
/*					*/ 

#include	"pppconfig.h"
#include	"ppp.h"
#include	"pap.h"
#include	"lcp.h"
#include	"globals.h"
#include	"debug.h"
#include	"time.h"
#include	"ircmd.h"
#include	"ahdlc.h"
#include	"console.h"
#include	"utils.h"

//
// State of the PAP state machine, this is global
//
//U8			pap_state;		Now in globals

void
pap_init()
{
	//ppp_retry=0;			// We reuse ppp_retry
	mm.pap_state=0;
}

//---------------------------------------------------------------------------
// pap_rx() - PAP RX protocol Handler
//---------------------------------------------------------------------------
void
pap_rx(U8 *buffer, U16 count)
{
U8	*bptr=buffer;
U8	len;

#ifdef DEBUG_LV1
	if(mm.IMMConfig & PPP_DEBUG)
	{
		DEBUG1("rcvd [");
	}
#endif
	switch(*bptr++)
	{
	case CONF_REQ:	
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Conf_Req, <no support");
		}
#endif
		break;
	case CONF_ACK:			// config Ack
		// Display message if debug
		len=*bptr++;
		*(bptr+len)=0;
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Conf Ack <msg ");
			CONSOLE_puts(bptr);
		}
#endif
		mm.pap_state |= PAP_TX_UP;
		// expire the timer to make things happen after a state change
		timer_expire();
		break;
	case CONF_NAK:
		mm.pap_state |= PAP_TX_AUTH_FAIL;
		// display message if debug
		len=*bptr++;
		*(bptr+len)=0;
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Conf_Nak <msg ");
			CONSOLE_puts(bptr);
			//DEBUG1(" >]"CRLF);
		}
#endif
		break;
	}
#ifdef DEBUG_LV1
	if(mm.IMMConfig & PPP_DEBUG)
	{
		DEBUG1(" >]"CRLF);
	}
#endif
}



//---------------------------------------------------------------------------
// pap_task() - This task needs to be called every so often during the PAP
//	negotiation phase.  This task sends PAP REQ packets.
//---------------------------------------------------------------------------
void
pap_task(U8 *ppp_tx_buffer, U8 *username, U8 *password)	
{
U8		*bptr;
U16		t;
PAPPKT	*pkt;

	// If LCP is up and PAP negotiated, try to bring up PAP
	if(!(mm.pap_state & PAP_TX_UP))
	{
		// Do we need to send a PAP auth packet?
		// Check if we have a request pending
		if(1==timer_timeout(PAP_TIMEOUT	))
		{
			// Check if we have a request pending
			//t=get_seconds()-pap_tx_time;
			//if(	t > pap_timeout)
			//{
				//
				// We need to send a PAP authentication request
				//
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("Send [PAP_REQ <");	
				}
#endif
				//
				// Build a PAP request packet
				//
				pkt=(PAPPKT *)ppp_tx_buffer;		
				//
				// Configure-Request only here, write id
				pkt->code=CONF_REQ;
				pkt->id=ppp_id;
				bptr=pkt->data;
				//
				// Write options
				//
				if(username)
					t=strlen(username);
				else
					t=0;
				*bptr++=(U8)t;						// Write peer length
				bptr=bytecopy(bptr,username,t);			// 

				if(password)
					t=strlen(password);
				else	
					t=0;
				*bptr++=(U8)t;
				bptr=bytecopy(bptr,password,t);
			
				// Write length
				t=bptr-ppp_tx_buffer;
				pkt->len=htons((U16)(bptr-ppp_tx_buffer));	// length here -  code and ID + 
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					CONSOLE_puts(username);
					DEBUG1(" ");
					CONSOLE_puts(password);
					DEBUG1(" > ]"CRLF);
				}
#endif		
				// Send packet
				ahdlc_tx(PAP,ppp_tx_buffer,0, t,0);

				// Set timer
				timer_set();
		}
	}
}
