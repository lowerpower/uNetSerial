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
 * lcp.c - Link Configuration Protocol Handler.								-
 *																			-
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 3, 2000									-        
 *
 *---------------------------------------------------------------------------    
 *                                                             				-
 * Copyright (C) 2000, Mycal Labs www.mycal.net								-
 *                                                                         	-
 *---------------------------------------------------------------------------
 *
 *
*/
/*					*/ 
/* include files 	*/

#include "pppconfig.h"
#include "debug.h"

#include "time.h"
#include "globals.h"
#include "ppp.h"
#include "ahdlc.h"
#include "lcp.h"
#include "console.h"
#include "conio.h"
#include "ircmd.h"
#include "msg.h"

//
// Define the supported paramets for this module here.
//
U8		lcplist[]={	LPC_MAGICNUMBER,LPC_PFC, LPC_ACFC,LPC_AUTH, LPC_ACCM, LPC_MRU, 0};	

//---------------------------------------------------------------------------
// lcp_init() - Initialize the LCP engine to startup values 
//---------------------------------------------------------------------------
void
lcp_init()
{
		mm.lcp_state=0;
		mm.ppp_tx_mru=0;
		timer_expire();
}


//---------------------------------------------------------------------------
// lcp_rx() - Receive an LCP packet and process it.  
//	This routine receives a LCP packet in buffer of length count.
//	Process it here, support for CONF_REQ, CONF_ACK, CONF_NACK, CONF_REJ or
//	TERM_REQ.
//
//
//---------------------------------------------------------------------------
void
lcp_rx(U8	*buffer, U16 count)
{
U8	*bptr=buffer, *tptr,*taccm;
U8	error=0;
U8	id,j;
U16	len;
//U32 accm;

#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("rcvd [LCP ");
		}
#endif
	switch(*bptr++)
	{
	case CONF_REQ:			// config request
		/* parce request and see if we can ACK it */
		id=*bptr++;
		len=(*bptr++ << 8);
		len|=*bptr++;
		//len-=2;
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("ConfReq ");
			printShort(id);
			DEBUG1(" ");
		}
#endif
		if(scan_packet(LCP ,lcplist, buffer, bptr, (U16)(len-4)))		// must do the -4 here, !scan packet
		{
			DEBUG3("...options were rejected\n");
		}
		else
		{
			/* lets try to implement what peer wants */
			tptr=bptr;
			error=0;
			/* first scan for unknown values */
			while(bptr < buffer+len)
			{
				switch(*bptr++)
				{
				case LPC_MRU:	/* mru */
					j=*bptr++;
					j-=2;
					if(j==2)
					{
						mm.ppp_tx_mru=((*bptr++<<8) || (*bptr++));
#ifdef DEBUG_LV1
						if(mm.IMMConfig & PPP_DEBUG)
						{
							DEBUG1("<mru "); 
							printShort(mm.ppp_tx_mru);
							DEBUG1("> ");		
						}
#endif					
					}
					else
						DEBUG1("<mru ?> ");
					break;
				case LPC_ACCM:	/* I have to fix this!!!+++ */
					bptr++;		/* skip length */	
					taccm=(U8 *)&mm.ppp_tx_accm;
					taccm[3]=*bptr++;
					taccm[2]=*bptr++;
					taccm[1]=*bptr++;
					taccm[0]=*bptr++;
#ifdef DEBUG_LV1
					if(mm.IMMConfig & PPP_DEBUG)
					{
						DEBUG1("<asyncmap ");
						//printByte(*bptr);
						printByte(taccm[3]);
						printByte(taccm[2]);
						printByte(taccm[1]);
						printByte(taccm[0]);
						//
						// What we should do is see if this is less restrictive
						// than what the user wants, if so reject and request 
						// more restrictive.
						//
						CONSOLE_puts(" - ");
						printDWord(mm.ppp_tx_accm);
						CONSOLE_puts(" - ");
						//
						// We accept any accm value
						//
						DEBUG1(" ok>");
					}
#endif
					//taccm=&mm.ppp_tx_accm;
					//taccm[3]=*bptr++;
					//taccm[2]=*bptr++;
					//taccm[1]=*bptr++;
					//taccm[0]=*bptr++;
					break;
				case LPC_AUTH:
					bptr++; /* skip length? */
					if((*bptr++==0xc0) && (*bptr++==0x23))
					{
#ifdef DEBUG_LV1
						if(mm.IMMConfig & PPP_DEBUG)
						{
							DEBUG1("<auth pap> ");	
						}
#endif
						// negotiate PAP
						mm.lcp_state |= LCP_RX_AUTH;	
					}
					else
					{
						// we only support PAP
#ifdef DEBUG_LV1
						if(mm.IMMConfig & PPP_DEBUG)
						{
							DEBUG1("<auth ");
							printByte((U8)(*bptr-2));
							printByte((U8)(*bptr-1));
							DEBUG1(">");
						}
#endif
						error=1;
						*tptr++=LPC_AUTH;
						*tptr++=0x4;
						*tptr++=0xc0;
						*tptr++=0x23;
					}
					break;
				case LPC_MAGICNUMBER:
					bptr++;						/* skip length */
#ifdef DEBUG_LV1
					if(mm.IMMConfig & PPP_DEBUG)
					{
						DEBUG1("<magic ");
						//
						// Compair incoming number to our number (not implemented)
						//
						printByte((U8)*bptr);
						printByte((U8)(*bptr+1));
						printByte((U8)(*bptr+2));
						printByte((U8)(*bptr+3));
						DEBUG1(" >");
					}
#endif
					bptr++;		// for now just dump the magic number
					bptr++;
					bptr++;
					bptr++;
					break;
				case LPC_PFC:
					bptr++;
					if(mm.IMMConfig & PPP_DEBUG)
					{
						DEBUG1("<pcomp> ");
					}
					//tflag|=PPP_PFC;
					break;
				case LPC_ACFC:
					bptr++;
					if(mm.IMMConfig & PPP_DEBUG)
					{
						DEBUG1("<accomp> ");
					}
					//tflag|=PPP_ACFC;
					break;

				}
			}
			/* Error? if we we need to send a config Reject ++++ this is good for a subroutine*/
			if(error)
			{
				/* WE HAVE PROBLEMS HERE, ACCM FOR ONE */
				/* write the config NAK packet we've built above, take on the header */
				bptr=buffer;
				*bptr++=CONF_NAK;		// Write Conf_rej
				*bptr++;//tptr++;		// skip over ID
				//
				/* Write new length */
				*bptr++=0;
				*bptr=tptr-buffer;
				//
				/* write the reject frame */
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("]"CRLF"[Sending LCP NAK id ");
					printShort(id);
					//DEBUG1(CRLF);
				}
#endif
				// Send packet ahdlc_txz(procol,header,data,headerlen,datalen);				
				ahdlc_tx(LCP,0,buffer, 0,(U16)(tptr-buffer));	
			}
			else
			{
				//
				// If we get here then we are OK, lets send an ACK and tell the rest
				// of our modules our negotiated config.
				//
				// We are sending an CONF_ACK RX should be up.
				mm.lcp_state |= LCP_RX_UP;
				//
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("]"CRLF"Sending LCP ACK id ");
					printShort(id);
					DEBUG1(CRLF);
				}
#endif
				bptr=buffer;
				*bptr++=CONF_ACK;		// Write Conf_ACK
				bptr++;					// Skip ID (send same one)
				//
				// Set stuff
				//
				//ppp_flags|=tflag;
				//DEBUG2("SET- stuff -- are we up? c=%d dif=%d \n", count, (U16)(bptr-buffer));				

				/* write the ACK frame */
				//DEBUG2("Writing ACK frame \n");	
				// Send packet ahdlc_txz(procol,header,data,headerlen,datalen);				
				ahdlc_tx(LCP,0,buffer, 0,count /*bptr-buffer*/);
				//DEBUG2("- End ACK Write frame\n");

				// expire the timer to make things happen after a state change
			}
			timer_expire();
			//DEBUG1(CRLF);
		}
		break;
	case CONF_ACK:			// config Ack   Anytime we do an ack reset the timer to force send.
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Conf_Ack ");
		}
#endif
		/* check that ID matches one sent */
		if(*bptr++==ppp_id)
		{	
			// Change state to PPP up.
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					printShort(ppp_id);
					//DEBUG1(" ]"CRLF);
				}
#endif
			// copy negotiated values over

			// LCP TX direction should be up.
			mm.lcp_state |= LCP_TX_UP;		

			// expire the timer to make things happen after a state change
			timer_expire();
		}
		else
		{
#ifdef DEBUG_LV1
			if(mm.IMMConfig & PPP_DEBUG)
			{
				printShort(ppp_id);
				DEBUG1(" bad ");
			}
#endif			
		}
		break;
	case CONF_NAK:			// Config Nack, we only send Magic Number and it is based on ppp_id
		id=*bptr++;
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("LCP-CONF NAK id %d]"CRLF);
			printShort(id);
		}
#endif
		ppp_id++;			// Inc ppp_id which is used in magic number
		break;
	case CONF_REJ:			// Config Reject
		// Remove the offending options
		ppp_id++;								// +++ do we need this?
		id=*bptr++;								// dump the ID				
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("LCP-CONF REJ id ");		
			printShort(id);
			DEBUG1(" ");
		}
#endif
		len=(*bptr++ << 8);					// get the length
		len|=*bptr++;

		// Parse ACK and set data
		while(bptr < buffer+len)
		{
			switch(*bptr++)
			{
			case LPC_MAGICNUMBER:
				mm.lcp_state |= LCP_OPT_MAGIC;
				bptr+=5;
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("<No MAGIC> ");
				}
#endif				
				break;
			}
		}
		// expire the timer to make things happen after a state change
		//timer_expire();
		break;
	case TERM_REQ:			// Terminate Request
		id=*bptr++;	
#ifdef DEBUG_LV1	
		if(mm.IMMConfig & PPP_DEBUG)
		{		
			DEBUG1("LCP-TERM-REQ id ");
			printByte(id);
			DEBUG1(" ]"CRLF);
			DEBUG1("Send [TERM_ACK id ");
			printByte(id);
			//DEBUG1(" fCRLF);
		}
#endif
		bptr=buffer;
		*bptr++=TERM_ACK;	// Write TERM_ACK
		/* write the reject frame */
		// Send packet ahdlc_txz(procol,header,data,headerlen,datalen);		
		ahdlc_tx(LCP,0,buffer,0, count);
		mm.lcp_state &= ~LCP_TX_UP;	
		mm.lcp_state |= LCP_TERM_PEER;
		// shutdown PPP
		ppp_down();
		displayMessage(MSG_PPPDOWN);
		break;
	case TERM_ACK:
		id=*bptr++;
#ifdef DEBUG_LV1		
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Send [TERM_ACK id ");
			printByte(id);
			//DEBUG1(" ]"CRLF);
		}
#endif
		mm.lcp_state &=~LCP_RX_UP;
		break;
	default:
		break;
	}
#ifdef DEBUG_LV1
	if(mm.IMMConfig & PPP_DEBUG)
	{
		DEBUG1(" ]"CRLF);
	}
#endif
	TIMER_Wait_Tick(5);
}

//---------------------------------------------------------------------------
// lcp_task(buffer) - This routine see if a lcp request needs to be sent
//	out.  It uses the passed buffer to form the packet.  This formed LCP
//	request is what we negotiate for sending  on the link.
//
//	Currently we negotiate : Magic Number Only, but this will change.
//---------------------------------------------------------------------------
void
lcp_task(U8 *buffer)
{
U8		*bptr;
U16		t;
LCPPKT	*pkt;

	// lcp tx not up and hasn't timed out then lets see if we need to send a request
	if(!(mm.lcp_state & LCP_TX_UP))
	{
		// Check if we have a request pending
		//t=get_seconds()-lcp_tx_time;
		if(1==timer_timeout(LCP_TIMEOUT))
		{
#ifdef DEBUG_LV1	
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1("Send [LCP Conf_Req ");
				printShort(ppp_id);
				DEBUG1(" ");
			}
#endif
			//
			// No pending request, lets build one
			//
			pkt=(LCPPKT *)buffer;		
			//
			// Configure-Request only here, write id
			pkt->code=CONF_REQ;
			pkt->id=ppp_id;
			
			bptr=pkt->data;

			// Write options
			//
			
			// Write magic number
			if((mm.lcp_state & LCP_OPT_MAGIC) == 0)
			{
#ifdef DEBUG_LV1	
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("<MagicNumber 000001");
					printByte(ppp_id);
					DEBUG1(" ");
					printByte(mm.lcp_state);
					DEBUG1(" >");
				}
#endif
				*bptr++=LPC_MAGICNUMBER;
				*bptr++=0x6;
				*bptr++=0;
				*bptr++=0;
				*bptr++=1;
				*bptr++=ppp_id;
			}

			// ACCM
			//
#if 0
			if((lcp_tx_options & LCP_OPT_ACCM) & 0)
			{
				taccm=&mm.rx_accm;
				*bptr++=LPC_ACCM;
				*bptr++=0x6;
				*bptr++=taccm[3];
				*bptr++=taccm[2];
				*bptr++=taccm[1];
				*bptr++=taccm[0];
			}
			//
			// Authentication protocol
			//
			if((lcp_tx_options & LCP_OPT_AUTH) && 0)
			{
				//
				// If turned on, we only negotiate PAP
				//
				*bptr++=LPC_AUTH;
				*bptr++=0x4;
				*bptr++=0xc0;
				*bptr++=0x23;
			}
			//
			// PFC
			//
			if((lcp_tx_options & LCP_OPT_PFC) && 0)
			{
				//
				// If turned on, we only negotiate PAP
				//
				*bptr++=LPC_PFC;
				*bptr++=0x2;
			}
			//
			// ACFC
			//
			if((lcp_tx_options & LCP_OPT_ACFC) && 0) 
			{
				//
				// If turned on, we only negotiate PAP
				//
				*bptr++=LPC_ACFC;
				*bptr++=0x2;
			}
#endif
			// Write length
			t=bptr-buffer; //ppp_tx_buffer;
			pkt->len=htons(t);			// length here -  code and ID + 

#ifdef DEBUG_LV1	
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1("]"CRLF);
			}
#endif
			// Send packet
			// Send packet ahdlc_txz(procol,header,data,headerlen,datalen);
			ahdlc_tx(LCP,0,buffer/*ppp_tx_buffer*/, 0,t);

			// Set timer
			timer_set();
		}
	}
	else 	
	if((mm.lcp_state & LCP_SHUTDOWN))
	{
		// LCP is up, lets try to terminate
		// Check if we have a request pending
		//t=get_seconds()-lcp_tx_time;
		if(1==timer_timeout(LCP_TIMEOUT))
		{
#ifdef DEBUG_LV1	
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1("Send [LCP TERM_Req ");
				printShort(ppp_id);
				DEBUG1(" ");
			}
#endif
			//
			// No pending request, lets build one
			//
			pkt=(LCPPKT *)buffer;		
			//
			// Terminate-Request only here, write id
			pkt->code=TERM_REQ;
			pkt->id=ppp_id;
			bptr=pkt->data;
			// Write length
			t=bptr-buffer; //ppp_tx_buffer;
			pkt->len=htons(t);			// length here -  code and ID + 

#ifdef DEBUG_LV1	
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1("]"CRLF);
			}
#endif
			// Send packet
			// Send packet ahdlc_txz(procol,header,data,headerlen,datalen);
			ahdlc_tx(LCP,0,buffer/*ppp_tx_buffer*/, 0,t);

			// Set timer
			timer_set();
		}
	}
}

