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

/*															
 *---------------------------------------------------------------------------
 * ppp.c - PPP Processor/Handler											-
 *																			-
 *---------------------------------------------------------------------------
 * 
 * Version                                                                  -
 *		0.1 Original Version Jun 3, 2000									-        
 *																			-
 *---------------------------------------------------------------------------    
-----------------------------------------------------------------------------
Copyright (c) 2000-2003, Mike Johnson/Mycal Labs (www.mycal.net)
All rights reserved. 

---------------------------------------------------------------------------
*/
/*					*/ 
/* include files 	*/
/*					*/ 

#include "pppconfig.h"
#include "debug.h"
#include "lcp.h"
#include "pap.h"
#include "ipcp.h"
#include "time.h"
#include "mip.h"
#include "mtcp.h"
#include "mudp.h"
#include "console.h"
#include "ircmd.h"
#include "conio.h"
#include "memory.h"
#include "msg.h"

//
// Set the debug message level
//
#define	PACKET_RX_DEBUG	1
#define DEBUG_LV1	1
#define DEBUG_LV2	2

#if PACKET_RX_DEBUG
//#include <stdio.h> 
#endif

//
// Include stuff
//
#include "mTypes.h"
#include "ppp.h"
#include "ahdlc.h"
#include "ipcp.h"
#include "lcp.h"
#include "globals.h"


//
// Other state storage (this can be placed in a struct and this could could
//	support multiple PPP connections, would have to embedded the other ppp
//	module state also)
//
//U8				ppp_flags;
U8				ppp_id;

//Local globals
static U16 ppp_start_time;
//U8				ppp_retry;
U8 *username, *password;

#if PACKET_RX_DEBUG
U16				ppp_rx_frame_count=0;
U16				ppp_rx_tobig_error;
U8				done;						//temp
#endif

//---------------------------------------------------------------------------
// Initialize and start PPP engine.  This just sets things up to starting 
//	values.  This can stay a private method. 
//---------------------------------------------------------------------------
void
init_ppp()
{
//	MEMBLOCK	*memblock;
#if PACKET_RX_DEBUG
	ppp_rx_frame_count=0;
	done=0;
#endif
	MEM_init();
	mm.ppp_flags=0;
	mm.ppp_flags2=0;
	pap_init();
	ipcp_init();
	lcp_init(/*ppp_tx_buffer*/);
	//memblock=MEM_GetBlock();
	ahdlc_init(0,0); //ppp_rx_buffer, PPP_RX_BUFFER_SIZE);
	ahdlc_rx_ready();
}


//---------------------------------------------------------------------------
// Force the PPP engine Off 
//---------------------------------------------------------------------------
void
ppp_down()
{
	// clear TCP sockets first, don't want to service them if PPP is down
	tcp_init();
	// shutdown the ahdlc layer so the PPP engine won't get the pkts.
	ahdlc_down();
	mm.ppp_flags=0;
}


//---------------------------------------------------------------------------
// Shutdown the PPP protocol nicely.
//---------------------------------------------------------------------------
S8
ppp_shut_down(void)
{
U16			start_time;
S8			status=0;
MEMBLOCK	*memblock;

	if(0==(mm.ppp_flags&PPP_UP))
		return(status);
	//
	// First Shutdown TCP
	//
	tcp_init();
	//
	// Set LCP shutdown
	//
	mm.lcp_state |= LCP_SHUTDOWN;
	
	/* we need to set a timeout here */
	memblock=MEM_GetBlock();

	// Just call IPCP_Task once "this is a hack"
	ipcp_task(memblock->data);
	/* Try to bring up the layers, we need to do this by timeout */
	//ppp_connect_timeout
	start_time=TIMER_Get_Second();
	while(!(status))
	{
		// This should be removed, the serial interrupt should be hooked
		// directly up to interrupt.
		poll_serial();
		
		// call the lcp task to bring down the LCP layer
		lcp_task(memblock->data);
		if((0==(mm.lcp_state &  LCP_RX_UP))) 
		{
			break;
		}
		//
		// Check Timeout
		//
		if(mm.ppp_connect_timeout <(TIMER_Get_Second()-start_time))
		{
			status=(U8)PPP_TIMEOUT;
			break;
		}
	}
	//
	// Finally force everything off
	// shutdown the ahdlc layer so the PPP engine won't get the pkts.
	ahdlc_down();
	//
	// Make sure PPP engine is turned off
	//
	mm.ppp_flags		&=!PPP_RX_READY;
	MEM_FreeBlock(memblock);
	return(status);

}

//---------------------------------------------------------------------------
// ppp_auto_enable() - This routine will setup ppp and let it auto connect
//	if the peer trys to bring up PPP.  It is non-blocking, for a blocking
//	initiate PPP try raise_PPP
//
S8
ppp_auto_enable(U8 *user, U8 *pass)
{
	// Reset upper layers
	tcp_init();
	udp_init();
	// Initialize PPP
	init_ppp();
	//
	// Setup the autosync in the ahdlc engine
	//
	ahdlc_autosync();
	//
	username=mm.user_name;
	password=mm.user_pass;

	// Enable Passive State Machine
	mm.ppp_flags2=PPP_PASSIVE;
	//
	mm.ppp_flags		|=PPP_RX_READY | PPP_STARTED;

//	ppp_task();

	return MSG_OK;
}

//---------------------------------------------------------------------------
// raise_ppp() - This routine will try to bring up a PPP connection,  
//	It is blocking.
//
//	In the future we probably want to pass a structure with all the options
//	on bringing up a PPP link, like server/client, DSN server, username
//	password for PAP... +++  for now just use config and bit defines
//---------------------------------------------------------------------------
S16
raise_ppp(U8 config, U8 *user, U8 *pass)
{
//U16			start_time;
S8			status=0;
//MEMBLOCK	*memblock;

	username=user;
	password=pass;
	//
	// Initialize TCP and UDP Engine
	tcp_init();
	udp_init();
	// Initialize PPP engine
	init_ppp();
	//
	// Enable it
	//
	mm.ppp_flags		|=PPP_RX_READY | PPP_STARTED;
	//
	// Set Timer
	//
	//ppp_connect_timeout
	ppp_start_time=TIMER_Get_Second();

	while(0==status)
	{
		// This should be removed, the serial interrupt should be hooked
		// directly up to interrupt.
		poll_serial();
		//
		status=ppp_task();
	}

	return(status);
}

#if 0

	/* we need to set a timeout here */
	//memblock=MEM_GetBlock();

	/* Try to bring up the layers, we need to do this by timeout */
	//ppp_connect_timeout
	//start_time=TIMER_Get_Second();
	while(!(status))
	{
		// This should be removed, the serial interrupt should be hooked
		// directly up to interrupt.
		poll_serial();
		// call the lcp task to bring up the LCP layer
		lcp_task(memblock->data);

		// 
		// If LCP is up, neg next layer
		//
		if((mm.lcp_state & LCP_TX_UP) && (mm.lcp_state & LCP_RX_UP)) 
		{
			//
			// if LCP wants PAP, try to authenticate, else bring up IPCP
			//
			if((mm.lcp_state & LCP_RX_AUTH) && (!(mm.pap_state & PAP_TX_UP)))
				pap_task(memblock->data,username,password);		
			else
				ipcp_task(memblock->data);			
		}
		//
		// If IPCP came up then our link should be up.
		if((mm.ipcp_state & IPCP_TX_UP) && (mm.ipcp_state & IPCP_RX_UP))
		{
			status=PPP_OK;
			mm.ppp_flags|=PPP_UP;
			break;
		}
		//
		// Check for auth fail
		//
		if((mm.pap_state & PAP_TX_AUTH_FAIL) || (mm.pap_state & PAP_RX_AUTH_FAIL))
		{
			status=PPP_AUTH_FAIL;
		}
		//
		// Check Timeout
		//
		if(mm.ppp_connect_timeout <(TIMER_Get_Second()-start_time))
		{
			status=PPP_TIMEOUT;
			break;
		}
	}
	//
	// Make sure PPP engine is turned off
	//
	if(status!=PPP_OK)
			mm.ppp_flags		&=!(PPP_RX_READY | PPP_RX_STARTED);

	MEM_FreeBlock(memblock);
	return(status);
}
#endif

//---------------------------------------------------------------------------
// ppp_passive() - 
//---------------------------------------------------------------------------
void
ppp_passive()
{
	U8	status;

	if(PPP_PASSIVE==mm.ppp_flags2)
	{
		if(ahdlc_is_ready())
		{

			status=ppp_task();
			if(status!=0)
			{
				if(PPP_OK==status) 
				{
					//  PPP has gone up, change to internet command mode.
					status=MSG_CONNECT;
					ircmd_state=IR_INTERNET_COMMAND;
					//mm.ppp_flags2&=~PPP_PASSIVE;
				}
				else
				{
					ppp_down();
					status=MSG_PPPFAIL;
				}
				//printShort(status);
				//CONSOLE_puts(CRLF);
				displayMessage(status);
				mm.ppp_flags2=0;
			}
		}
		else
			//ppp_connect_timeout, reset until ahdlc is ready
			ppp_start_time=TIMER_Get_Second();
	}
}

//---------------------------------------------------------------------------
// ppp_task() - This routine will handle the setting up and bringing down
//	of PPP.
//
// This will try to keep PPP up
//
// Returns 1 for PPP up condition, 0 for negotiating, -1 for error.
//
//---------------------------------------------------------------------------

S8
ppp_task(void)
{
MEMBLOCK	*memblock;
U8			status=0;

	memblock=MEM_GetBlock();

	if(0==memblock)
		return 0;

	//putch("x");

	// See if we have enabled PPP, do nothing if PPP is not enabled
	if(mm.ppp_flags & PPP_RX_READY)
	{
		//
		// Action based on state, first bring LCP up
		//
		if(!((mm.lcp_state & LCP_TX_UP) && (mm.lcp_state & LCP_RX_UP))) 
		{
			lcp_task(memblock->data);
			//putch("m");
		}
		else
		{
			//putch("p");
			//
			// Next bring PAP and/or IPCP up  
			//
			if((mm.lcp_state & LCP_RX_AUTH) && (!(mm.pap_state & PAP_TX_UP)))
				pap_task(memblock->data,username,password);		
			else
				ipcp_task(memblock->data);	
		}

		// Check for PPP up
		if((mm.ipcp_state & IPCP_TX_UP) && (mm.ipcp_state & IPCP_RX_UP))
		{
			status=PPP_OK;
			mm.ppp_flags|=PPP_UP;
		
		}
		//
		// check for auth fail
		//
		if((mm.pap_state & PAP_TX_AUTH_FAIL) || (mm.pap_state & PAP_RX_AUTH_FAIL))
		{
			status=PPP_AUTH_FAIL;
			mm.ppp_flags		=0;
		}
		//
		// Check Timeout
		//
		if(mm.ppp_connect_timeout <(TIMER_Get_Second()-ppp_start_time))
		{
			status=PPP_TIMEOUT;
			mm.ppp_flags		=0;
		}
	
	}
	// Free the memory
	MEM_FreeBlock(memblock);
	return(status);
}


//---------------------------------------------------------------------------
// ppp_upcall() - this is where valid PPP frames from the ahdlc layer are
//	sent to be processed and demuxed.
//---------------------------------------------------------------------------
void
ppp_upcall(U16 protocol, U8 *buffer, U16 len)
{


#if PACKET_RX_DEBUG
	ppp_rx_frame_count++;
//	DEBUG1("\n<<<<<<<<<<<<<<-----------\n");
//	dump_ppp_packet(buffer,len);
//	DEBUG1("\n<<<<<<<<<<<<<<-----------\n");
	if(ppp_rx_frame_count>18)
		done=1;
#endif



	/* check to see if we have a packet waiting to be processed */
	if(mm.ppp_flags & PPP_RX_READY)
	{	
		//
		// If PPP Negotiation has not started, Start it and set the timeout timer
		//
		if(!(mm.ppp_flags	& PPP_STARTED))
		{
			mm.ppp_flags |= PPP_STARTED;
			//ppp_connect_timeout
			ppp_start_time=TIMER_Get_Second();
		}


		/* demux on protocol field */
		switch(protocol)
		{
		case LCP:	// We must support some level of LCP
			DEBUG3("LCP - ");
			lcp_rx(buffer,len);
			DEBUG3(CRLF);
			break;
		case PAP:	// PAP should be compile in optional
			DEBUG3("PAP - ");
			pap_rx(buffer,len);
			DEBUG3(CRLF);
			break;
		case IPCP:	// IPCP should be compile in optional.
			DEBUG3("IPCP - ");
			ipcp_rx(buffer,len);
			DEBUG3(CRLF);
			break;
		case IPV4:	// We must support IPV4
			DEBUG3("IPV4 Packet---\n");
			ip_upcall((IP *)buffer,len);
			DEBUG3(CRLF);
			break;
		default:
#ifdef DEBUG_LV1
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1("rcvd [Unknown PPP Protocol Type ");
				printByte((U8)protocol);
				DEBUG1("]"CRLF);
			}
#endif
			ppp_reject_protocol(protocol,buffer,len);
			//DEBUG1("]\n");
			break;
		}
	}
}


// scan_packet(list,buffer,len)
//
// list = list of supported ID's
// *buffer pointer to the first code in the packet
// length of the codespace
//
U16
scan_packet(U16 protocol, U8 *list, U8 *buffer, U8 *options, U16 len)
{
U8	*tlist, *bptr;
U8	*tptr;
U8	bad=0;
U8	i,j,good;

	bptr=tptr=options;
	//
	// scan through the packet and see if it has any unsupported codes
	//
	while(bptr < options+len)
	{
		/* get code and see if it matches somwhere in the list, if not we don't support it */
		i=*bptr++;
	
		//printf("%x - ",i);
		tlist=list;
		good=0;
		while(*tlist)
		{
			//printf("%x ",*tlist);
			if(i==*tlist++)
			{
				good=1;
				break;
			}
		}
		if(!good)
		{
			/* we don't understand it, write it back */
#ifdef DEBUG_LV1
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1("<bad option ");
				printWord(i);
				DEBUG1("> ");
			}
#endif
			bad=1;
			*tptr++=i;
			j=*tptr++=*bptr++;
			for(i=0;i<j-2;i++)
			{
				*tptr++=*bptr++;
			}
		}
		else
		{
			/* advance over to next option */
			bptr+=*bptr-1;
		}
	}
	/* Bad? if we we need to send a config Reject */
	if(bad)
	{
		/* write the config Rej packet we've built above, take on the header */
		bptr=buffer;
		*bptr++=CONF_REJ;		// Write Conf_rej
		i=*bptr++;					// skip over ID
		*bptr++=0;
		*bptr=tptr-buffer;
		// length right here?
		
		/* write the reject frame */
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("]"CRLF);
			DEBUG1("Send [Conf_Rej ");
			printShort(i);
			DEBUG1(" ]"CRLF);
		}
#endif
		ahdlc_tx(protocol,buffer,0, (U16)(tptr-buffer),0);
		DEBUG3("\nEnd writing reject \n");
		
	}		
	return(bad);
}


//
// Unknown Protocol Handler, sends reject
//
void
ppp_reject_protocol(U16 protocol, U8 *buffer,U16 count)
{
U16	i;
U8	*dptr,*sptr;
LCPPKT	*pkt;
	
	// first copy rejected packet back, start from end and work forward,
	//
	//   +++ Pay attention to buffer managment when updated. Assumes fixed PPP blocks.
	//
	DEBUG2("!Rejecting Protocol"CRLF);
	if((count+6)>PPP_RX_BUFFER_SIZE)
	{
		// This is a fatal error +++ do somthing about it.
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Warning! Cannot Reject Protocol, PKT to big"CRLF);
		}
		return;
	}
	dptr=buffer+count+6;
	sptr=buffer+count;
	for(i=0;i<count;i++)
		*dptr--=*sptr--;

	pkt=(LCPPKT *)buffer;
	pkt->code=PROT_REJ;		// Write Conf_rej
	//pkt->id=tid++;			// write tid
	pkt->len=htons((count+6));
	*((U16 *)(&pkt->data[0])) = htons(protocol);

	ahdlc_tx(LCP,buffer,0, (U16)(count+6),0);
}


/*
U8
check_ppp_errors()
{
U8	ret=0;;

	// Check Errors
	if(pap_state & PAP_TX_AUTH_FAIL)
		ret=3;
	if(pap_state & PAP_RX_AUTH_FAIL)
		ret=4;
#ifdef DEBUG_LV1
	if(ret!=0)
	{
		DEBUG1("PPP failed Reason ");
		printShort(ret);
		DEBUG1(CRLF);
	}
#endif

	return(ret);
}
*/



#if PACKET_RX_DEBUG

void
dump_ppp_packet(U8 *buffer,U16 len)
{
int i;

	for(i=0;i<len;i++)
	{
		if((i & 0x1f) == 0x10)
			CONSOLE_puts(CRLF);
		printByte(buffer[i]);
		CONSOLE_putch(' ');
	}
	CONSOLE_puts(CRLF);;

}
#endif






