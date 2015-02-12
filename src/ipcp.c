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
 * ipcp.c - PPP IPCP (intrnet protocol) Processor/Handler					-
 *																			-
 *---------------------------------------------------------------------------
 *
 * Version                                                                  -
 *		0.1 Original Version Jun 3, 2000									-        
 *																			-
 *---------------------------------------------------------------------------    
 *                                                             				-
 * Copyright (C) 2000, Mycal Labs www.mycal.net								-
 *                                                                         	-
 *---------------------------------------------------------------------------
*/
/*					*/ 
/* include files 	*/
/*					*/ 
#include "pppconfig.h"
#include "mTypes.h"
#include "time.h"
#include "ipcp.h"
#include "ppp.h"
#include "mip.h"
#include "ahdlc.h"
#include "globals.h"
#include "console.h"
#include "conio.h"
#include "ircmd.h"
#include "debug.h"

//
// Local IPCP state
//
//U8				ipcp_state;

//
// in the future add copression protocol and name servers (possibly for servers only)
//
//U8		ipcplist[]={0x3,0};	

/*
void
printip(IPAddr ip)
{
	printf(" %d.%d.%d.%d ",ip.ipb1,ip.ipb2,ip.ipb3,ip.ipb4);
}
*/

void
ipcp_init()
{
	//DEBUG1("ipcp init\n");
	mm.ipcp_state=0;
	//ppp_retry=0;
	mm.our_ipaddr.ip32=0;
	mm.pri_dns_addr.ip32=0;
	mm.sec_dns_addr.ip32=0;
}

//
// Stuff we can receive
//
U8		ipcplist[]={IPCP_IPADDRESS,IPCP_PRIMARY_DNS,IPCP_SECONDARY_DNS,0};
//
// IPCP RX protocol Handler
//
void
ipcp_rx(U8 *buffer, U16 count)
{
U8			*bptr=buffer,id;
//IPCPPKT		*pkt=(IPCPPKT *)buffer;
U16			len;

#ifdef DEBUG_LV1	
	if(mm.IMMConfig & PPP_DEBUG)
	{
		DEBUG1("rcvd [IPCP ");
	}
#endif
	switch(*bptr++)
	{
	case CONF_REQ:
		/* parce request and see if we can ACK it */
		id=*bptr++;
		len=(*bptr++ << 8);
		len|=*bptr++;
		//len-=2;
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Conf_Req ");
			printShort(id);
			DEBUG1(" ");
		}
#endif		
		if(scan_packet(IPCP, ipcplist, buffer, bptr, (U16)(len-4)))		
		{
			DEBUG3("Options were rejected\n");
		}
		else
		{
			// skip code, id, len
			//	tptr=bptr;
			//error=0;
			/* first scan for unknown values */
			while(bptr < buffer+len)
			{
				if(IPCP_IPADDRESS==*bptr)
				{
					bptr++;							// Dump Type
					bptr++;							// dump length
					mm.peer_ip_addr.ipb1=*bptr++;
					mm.peer_ip_addr.ipb2=*bptr++;
					mm.peer_ip_addr.ipb3=*bptr++;
					mm.peer_ip_addr.ipb4=*bptr++;
#ifdef DEBUG_LV1
					if(mm.IMMConfig & PPP_DEBUG)
					{	
						CONSOLE_puts("<PIPAddr ");
						printIP(&mm.peer_ip_addr);
						CONSOLE_puts("> ]"CRLF);	
					}
#endif
				}
				else
				{
					// Handle Unknown Type
#ifdef DEBUG_LV1
					if(mm.IMMConfig & PPP_DEBUG)
					{
						DEBUG1("HMMMM this shouldn't happen IPCP received Type ");
						printByte((U8)*bptr++);
						printByte((U8)*bptr);
						DEBUG1(CRLF);
						// Inc over unknown type data
						bptr=bptr+(*bptr);
					}
					else
#endif
					{
						bptr++;							// Dump Type
						// Inc over unknown type data
						bptr=bptr+(*bptr);					
					}
				}
			}//end while
				//
				// If we get here then we are OK, lets send an ACK and tell the rest
				// of our modules our negotiated config.
				//
				mm.ipcp_state |= IPCP_RX_UP;
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("Send [IPCP Conf_Ack ");
					printShort(id);
					//DEBUG1(" ]"CRLF);
				}
#endif
				bptr=buffer;
				*bptr++=CONF_ACK;		// Write Conf_ACK
				bptr++;					// Skip ID (send same one)
				//
				// Set stuff
				//
				//ppp_flags|=tflag;
				//DEBUG1("SET- stuff -- are we up? c=%d dif=%d \n", count, (U16)(bptr-buffer));				

				/* write the ACK frame */
				//DEBUG1("Writing ACK frame \n");	
				// Send packet ahdlc_txz(procol,header,data,headerlen,datalen);				
				ahdlc_tx(IPCP,0,buffer, 0, count /*bptr-buffer*/);
				//DEBUG1("- End ACK Write frame\n");
				
				// expire the timer to make things happen after a state change
				//timer_expire();		// hold off this one.		

//			}
		}
		break;
	case CONF_ACK:			// config Ack
		//
		// Parse out the results  ---- Do we even need to do this?
		//
		// Dump the ID and get the length. 
		id=*bptr++;								// dump the ID		
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Conf_Ack ");		
			printShort(id);
			DEBUG1(" ");
		}
#endif
		len=(*bptr++ << 8);					// get the length
		len|=*bptr++;
//
// May have to add this code back in some day, but we shouldn't need it.
//
#if 0
		// Parse ACK and set data
		while(bptr < buffer+len)
		{
			switch(*bptr++)
			{
			case IPCP_IPADDRESS:
				DEBUG1("<IPAddr ");
				bptr++;						// dump length 
				mm.our_ipaddr.ipb1=*bptr++;
				mm.our_ipaddr.ipb2=*bptr++;
				mm.our_ipaddr.ipb3=*bptr++;
				mm.our_ipaddr.ipb4=*bptr++;
#ifdef DEBUG_LV1
				DEBUG1("<IPAddr ");
				printIP(&mm.peer_ip_addr);
				CONSOLE_puts("> ");
#endif
				break;
			case IPCP_PRIMARY_DNS:
				bptr++;
				mm.pri_dns_addr.ipb1=*bptr++;
				mm.pri_dns_addr.ipb2=*bptr++;
				mm.pri_dns_addr.ipb3=*bptr++;
				mm.pri_dns_addr.ipb4=*bptr++;
#ifdef DEBUG_LV1
				DEBUG1("<Pri-DNS ");
				printIP(&mm.pri_dns_addr);
				CONSOLE_puts("> ");
#endif
				break;
			case IPCP_SECONDARY_DNS:
				bptr++;
				DEBUG1("<Sec-DNS> ");
				mm.sec_dns_addr.ipb1=*bptr++;
				mm.sec_dns_addr.ipb2=*bptr++;
				mm.sec_dns_addr.ipb3=*bptr++;
				mm.sec_dns_addr.ipb4=*bptr++;
#ifdef DEBUG_LV1
				DEBUG1("<Sec-DNS ");
				printIP(&mm.sec_dns_addr);
				CONSOLE_puts("> ");
#endif
				break;
			default:
				DEBUG1("<?> "); 
			}
		}
#endif	


		mm.ipcp_state |= IPCP_TX_UP;	
		//DEBUG1("<?> "); 
		// expire the timer to make things happen after a state change
		timer_expire();
		break;
	case CONF_NAK:			// Config Nack
		// 
		ppp_id++;
		id=*bptr++;								// dump the ID				
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Conf_Nak ");		
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
			case IPCP_IPADDRESS:
				bptr++;						// dump length
				mm.our_ipaddr.ipb1=*bptr++;
				mm.our_ipaddr.ipb2=*bptr++;
				mm.our_ipaddr.ipb3=*bptr++;
				mm.our_ipaddr.ipb4=*bptr++;
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("<IPAddr ");
					printIP(&mm.peer_ip_addr);
					CONSOLE_puts("> ");
				}
#endif
				break;
			case IPCP_PRIMARY_DNS:
				bptr++;
				mm.pri_dns_addr.ipb1=*bptr++;
				mm.pri_dns_addr.ipb2=*bptr++;
				mm.pri_dns_addr.ipb3=*bptr++;
				mm.pri_dns_addr.ipb4=*bptr++;
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("<Pri-DNS ");
					printIP(&mm.pri_dns_addr);
					CONSOLE_puts("> ");
				}
#endif
				break;
			case IPCP_SECONDARY_DNS:
				bptr++;
				mm.sec_dns_addr.ipb1=*bptr++;
				mm.sec_dns_addr.ipb2=*bptr++;
				mm.sec_dns_addr.ipb3=*bptr++;
				mm.sec_dns_addr.ipb4=*bptr++;
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("<Sec-DNS ");
					printIP(&mm.sec_dns_addr);
					CONSOLE_puts("> ");
				}
#endif
				break;
			default:
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("<?> "); 
				}
#endif
				break;
			}
		}

		// expire the timer to make things happen after a state change
		timer_expire();
		break;
	case CONF_REJ:			// Config Reject
		// Remove the offending options
		ppp_id++;								// +++ do we need this?
		id=*bptr++;								// dump the ID				
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Conf_Rej ");		
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
			case IPCP_IPADDRESS:
				mm.ipcp_state |= IPCP_IP_BIT;
				bptr+=5;
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("<No IP> ");
				}
#endif
				break;
			case IPCP_PRIMARY_DNS:
				mm.ipcp_state |= IPCP_PRI_DNS_BIT;
				bptr+=5;
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("<NO PRI DNS> ");
				}
#endif
				break;
			case IPCP_SECONDARY_DNS:
				mm.ipcp_state |= IPCP_SEC_DNS_BIT;
				bptr+=5;
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("<NO SEC DNS> ");
				}
#endif
				break;

			default:
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("IPCP this shoudln't happen 3");
				}
#endif
				break;
			}
		}
		// expire the timer to make things happen after a state change
		timer_expire(); //+++
		break;
  default:
#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("-Unknown IPCP type ");
		}
#endif
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


void
ipcp_task(U8 *buffer)
{
U8		*bptr;
U16		t;
IPCPPKT	*pkt;

	// IPCP tx not up and hasn't timed out then lets see if we need to send a request
	//if(!(mm.ipcp_state & IPCP_TX_UP) && !(mm.ipcp_state & IPCP_TX_TIMEOUT))
	if(!(mm.ipcp_state & IPCP_TX_UP))
	{
		
		// Check if we have a request pending
		//t=get_seconds()-ipcp_tx_time;
		if(1==timer_timeout(IPCP_TIMEOUT))
		{

#ifdef DEBUG_LV1
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("Send [IPCP Conf_Req ");
			printShort(ppp_id);
			DEBUG1(" ");
		}
#endif
			//
			// No pending request, lets build one
			//
			pkt=(IPCPPKT *)buffer;		
			//
			// Configure-Request only here, write id
			pkt->code=CONF_REQ;
			pkt->id=ppp_id;		
			bptr=pkt->data;
			//
			// Write options, we want IP address, and DNS addresses if set.
			//		
			// Write zeros for IP address the first time
			*bptr++=IPCP_IPADDRESS;
			*bptr++=0x6;
			*bptr++=mm.our_ipaddr.ipb1;
			*bptr++=mm.our_ipaddr.ipb2;
			*bptr++=mm.our_ipaddr.ipb3;
			*bptr++=mm.our_ipaddr.ipb4;
#ifdef DEBUG_LV1
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1("<IPAddr ");
				printIP(&mm.our_ipaddr);
				CONSOLE_puts("> ");
			}
#endif
			if(!(mm.ipcp_state & IPCP_PRI_DNS_BIT))
			{
				// Write zeros for IP address the first time
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("<Pri-DNS> ");
					printIP(&mm.pri_dns_addr);
					CONSOLE_puts("> ");
				}
#endif
				*bptr++=IPCP_PRIMARY_DNS;
				*bptr++=0x6;
				*bptr++=mm.pri_dns_addr.ipb1;
				*bptr++=mm.pri_dns_addr.ipb2;
				*bptr++=mm.pri_dns_addr.ipb3;
				*bptr++=mm.pri_dns_addr.ipb4;
			}
			if(!(mm.ipcp_state & IPCP_SEC_DNS_BIT))
			{
				// Write zeros for IP address the first time
#ifdef DEBUG_LV1
				if(mm.IMMConfig & PPP_DEBUG)
				{
					DEBUG1("<Sec-DNS ");
					printIP(&mm.sec_dns_addr);
					CONSOLE_puts("> ");
				}
#endif
				*bptr++=IPCP_SECONDARY_DNS;
				*bptr++=0x6;
				*bptr++=mm.sec_dns_addr.ipb1;
				*bptr++=mm.sec_dns_addr.ipb2;
				*bptr++=mm.sec_dns_addr.ipb3;
				*bptr++=mm.sec_dns_addr.ipb4;
			}

			// Write length
			t=bptr-buffer; //ppp_tx_buffer;
			pkt->len=htons(t);			// length here -  code and ID + 

#ifdef DEBUG_LV1
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1(" ]"CRLF);	
			}
#endif
			// Send packet ahdlc_txz(procol,header,data,headerlen,datalen);
			ahdlc_tx(IPCP,0,buffer/*ppp_tx_buffer*/,0, t);

			// Set timer
			timer_set();
		}
	}
	else 	
	if((mm.ipcp_state & IPCP_SHUTDOWN))
	{
		// IPCP is up, lets try to terminate
		// Check if we have a request pending
		//t=get_seconds()-lcp_tx_time;
		if(1==timer_timeout(IPCP_TIMEOUT))
		{
#ifdef DEBUG_LV1	
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1("Send [IPCP TERM_Req ");
				printShort(ppp_id);
				DEBUG1(" ");
			}
#endif
			//
			// No pending request, lets build one
			//
			pkt=(IPCPPKT*)buffer;		
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
			ahdlc_tx(IPCP,0,buffer/*ppp_tx_buffer*/, 0,t);

			// Set timer
			timer_set();
		}
	}
}


