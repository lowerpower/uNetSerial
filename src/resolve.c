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

/*																www.nchip.com
 *---------------------------------------------------------------------------
 * Resolve.c - A very simple, very optimized DNS resolver.  Optimized for   -
 *	the callback operation of the mtcpip stack								-
 *																			-
 *	This is also a good example of how to use a UDP socket.					-
 *
 * Need to add a cache so we don't have to look up a name if we have 
 * before.
 *
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 23, 2000 - MWJ							-        
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2000, nchip/mycal labs , All Rights Reserved.				_
 *																			_
 *---------------------------------------------------------------------------
*/

#include "pppconfig.h"
#include "ircmd.h"
#include "mTypes.h"
#include "time.h"	
#include "mudp.h"
#include "resolve.h"   
#include "memory.h"
#include "globals.h"
#include "debug.h"
#include "utils.h"
#include <string.h>

//static iIPAddr	dnsserverip;    
static U8		dnsflags;
static U8		dns_id;				// note we only keep 8 bits of dns_id
static U8		namesize;
IPAddr			foundip;

// Cache Element
//
//
#define CACHE_NAME_SIZE			30
static	U8						cache_name[CACHE_NAME_SIZE];
static	IPAddr					cache_ip;
//
//

// this is temp, do away for embedded use
//U8				dnsbuffer[255];

/*    
void
Set_DNS_Server(IPAddr *serverip)
{
	dnsserverip.ip32 =serverip->ip32;
}
 
void
Get_DNS_Server(IPAddr *serverip)
{
	serverip->ip32=dnsserverip.ip32;
} 
*/
 
 
//---------------------------------------------------------------------------------------------------
// gobble_name - supports resolve_name, gobbles hostname strings and tosses them away, 
//---------------------------------------------------------------------------------------------------
U16
gobble_name(U8 *data, U16 index)
{
U8  t,i,count=0;

	while(1)
	{
		t=data[index++];
		count++;
		
		if(t==0)
		{    
			return(RET_CODE_OK);
		}
		else if(t==0xc0)
		{
			index++;
			count++;
			return(index);
		}                 
		else
		{
			for(i=0;i<t;i++)           
			{
				index++;
				count++;         
				if(count>250)
					return(0);
			}
		}
	}
}

 
//---------------------------------------------------------------------------------------------------
// dump_bytes()
//---------------------------------------------------------------------------------------------------
/*
U8
dump_bytes(U8	bytes)
{           
	U8	i,t=0;
	    
	if(bytes)
	{	    
		for(i=0;i<bytes;i++)
			t=readx(SOCKET_DATA_REG);
	}

	return(t);	
}
*/

 
//---------------------------------------------------------------------------------------------------
// resolve_callback(IP *header, U8 * data, U16 len) - this is called by mTCPIP when a UDP packet
//	has been received on a resolver bound socket.
//---------------------------------------------------------------------------------------------------
void 
resolve_callback(IP *header,U8 *data, U16 len)
{
S16		index;
U8		t,i;

#ifdef DEBUG_LV2
	if(mm.IMMConfig & PPP_DEBUG)
	{
		CONSOLE_puts("DNS Reply: ");
	}
#endif

	// Parse the reply, first skip the UDP header.
	index=sizeof(UDP);
	//
	// Sanity check length
	//
	if(len<index+16)
		return;
	//
	// Check dns_id of incoming packet, dump the first byte as we don't care
	if(dns_id!=data[++index])
	{
#ifdef DEBUG_LV2
		if(mm.IMMConfig & PPP_DEBUG)
		{
			CONSOLE_puts("wrong id .."CRLF);
		}
#endif
		return;
	}
	index++;
	//
	// Check for a reply
	//printf("resonse code %x\n",data[index]);
	if(!(data[index++] & 0x80))
	{
#ifdef DEBUG_LV2
		if(mm.IMMConfig & PPP_DEBUG)
		{
			CONSOLE_puts("no responce code .."CRLF);
		}
#endif
		return;
	}
	if(data[index] !=0x80)
	{
		// no answer
		//DEBUG1("no answer"CRLF);
#ifdef DEBUG_LV2
		if(mm.IMMConfig & PPP_DEBUG)
		{
			CONSOLE_puts("no anser code .."CRLF);
		}
#endif
		dnsflags=2;
		return;
	}
	// Toss  question count, answer count, authority count, and additional records, size of name we sent above, query type, query class
	//for(i=0;i<(13+namesize);i++)      
	index+=13+namesize;
	//
	//
	while(len>index)
	{
		index=gobble_name(data,index);
		if(0==index)
			break; 
		t=data[index++];
		i=data[index++];
		if((t==0) && (i==0x1))
		{	
			// We've found the host address, dump class, ttl and length of next object
		    index+=8;
			// Store the IP address
			foundip.ipb1=data[index++];
			foundip.ipb2=data[index++];
			foundip.ipb3=data[index++];
			foundip.ipb4=data[index++];		
#ifdef DEBUG_LV2
		if(mm.IMMConfig & PPP_DEBUG)
		{
			CONSOLE_puts("found ");
			printIP(&foundip);
			CONSOLE_puts("CRLF");
		}
#endif			

			dnsflags=1;
//printf("*** found ! ***");
			break;	
		}
		else
		{
			// Eat this section 
			index+=7;
			index+=data[index]+1; // Remember to add in size (+1) of length itself
		}
	}
}


//---------------------------------------------------------------------------------------------------
// DNS resolver, optimized for low memory requirements, code size.
//---------------------------------------------------------------------------------------------------
RET_CODE
resolve_name(IPAddr *ipaddr, U8 *hostname)
{   
	U8			s,count;              
	U16 		stime;
	U8			*tptr,*ptr,*buffer,*name;
	U16			i;
	RET_CODE	ret=RET_CODE_FAIL;	
	MEMBLOCK	*memblock;

	//
	// If no DNS server, fail  sec_dns_addr
	//
	if((mm.pri_dns_addr.ipw1 | mm.pri_dns_addr.ipw2)==0)
		return(ret);	
	//
	// Check DNS cache 
	//
	if(0==strcmp(cache_name,hostname))
	{

		// Found a match, return IP address from cache.
		ipaddr->ip32=cache_ip.ip32;

#ifdef DEBUG_LV2
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG2("DNS cache hit",CRLF);
		}
#endif
		return(RET_CODE_OK);
	}
	//
	// Clear the DNS flag, for now this means that only one pending process
	//
	dnsflags=0;
	//
	// Allocate a buffer, right now we do static, change in future.
	//
	memblock=MEM_GetBlock();
	if(0==memblock)
	{
		return(ret);
	}

	buffer=memblock->data;
	
	//
	// Bind the UDP port for the reply
	//
	s=udp_bind(2048, (void *)&resolve_callback);
	if(0==s)
	{
		MEM_FreeBlock(memblock);
		return(ret);
	}
		
	//
	// Try 2 times to resolve the name
	//
	for(count=0;count<MAX_DNS_RETRY;count++)
	{
#ifdef DEBUG_LV2
		if(mm.IMMConfig & PPP_DEBUG)
		{
			CONSOLE_puts("building DNS request to DNS server ");
			printIP(&mm.pri_dns_addr);
			CONSOLE_puts(" for ");
			CONSOLE_puts(hostname);
			CONSOLE_puts("CRLF");
		}
#endif

		//
		// point to our buffer
		//
		tptr=buffer;
		//
		// Build a DNS request packet
		//
   		// Build the reqest, [dnsid][querytype][questions][answers][rr's][ad rr's][hostname2resolve][querytype][queryclass]
		*tptr++=0;
		*tptr++=dns_id;
		// querytype (flags), always use 01 00 for query, this tels the server to do all the work.
		*tptr++=0x01;
		*tptr++=0x00;	
		// Number of questions, we always ask just one question
		*tptr++=0x00;
		*tptr++=0x01;		
		//  Store number of answers, always 0 
		*tptr++=0x00;
		*tptr++=0x00;	 
		// Store the number of Authority RR's, always zero
		*tptr++=0x00;
		*tptr++=0x00;	 
		// Store the number of aditional RR's, always zero
		*tptr++=0x00;
		*tptr++=0x00;	                                    
		//
		// Convert the name we want to resolve into the correct format (Stevens Vol1 Pg 193), keep
		//	tabs on the number of bytes we send here so we can skip them in the reply without having
		//	to parse them.
		//   
		namesize=1;           
		name=hostname;
		while((*name >=  46) && (*name!=':'))
		{          
			ptr=name;  

			i=0;
			//while((*ptr !='.') && (*ptr > 32) && (*ptr != ':'))
			while(((*ptr>=47) && (*ptr != ':')) || *ptr==45)
			{                 
				i++;
				ptr++;
			}   
			
			*tptr++=(U8)i;
			namesize++;

			while(i--)
			{
				*tptr++=*name++;
				namesize++;
			}
			 
			// Skip period if necessary 
			if(*name=='.')       
			{
				name++;
			}
		}
		*tptr++=0;
		// query type 00 01 if we want an IP address, use 00 05 if you want to resolve a n IP address to a name
		*tptr++=0;
		*tptr++=1;
		// Query Class, always 00 01 for IP 
		*tptr++=0;
		*tptr++=1;    
		// 
		// Send the data out the socket
		//
		if( (1==count) && (0!=(mm.sec_dns_addr.ipw1 | mm.sec_dns_addr.ipw2)) )
			udp_tx(mm.sec_dns_addr, 53, 2048, buffer, (U16)(namesize+16));
		else
			udp_tx(mm.pri_dns_addr, 53, 2048, buffer, (U16)(namesize+16));

		// Wait for a reply, up to 5 seconds. 
		
		stime=TIMER_Get_Second();  	
		// +++ make sure this works..
		while(0==dnsflags)
    	{   
			//poll_serial();
	    	if(mm.dns_timeout <  (U8)(TIMER_Get_Second()-stime)) 
    		{                 
				ret=RET_CODE_TIMEOUT;
#ifdef DEBUG_LV2
		if(mm.IMMConfig & PPP_DEBUG)
		{
			CONSOLE_puts("DNS timeout"CRLF);
		}
#endif	
				break;
    		}
			//printf("Timer = %x \n",TIMER_Get_Second());
			poll_serial();
	    } 
		//
		// Check DNS flags
		//
		if(1==dnsflags)
		{
#ifdef DEBUG_LV2
		if(mm.IMMConfig & PPP_DEBUG)
		{
			CONSOLE_puts("found return IP address "CRLF);
		}
#endif	
			ret=RET_CODE_OK;
			ipaddr->ip32=foundip.ip32;
			//
			// Update cache, if hostname fits in cache
			//
			i=strlen(hostname);
			if(i<CACHE_NAME_SIZE)
			{
				// store the cache element
				cache_ip.ip32=foundip.ip32;
				bytecopy(cache_name,hostname,i);
				cache_name[i]=0;
			}
			break;
		} else if(dnsflags==2)
		{
			ret=RET_CODE_FAIL;
			break;
		}
    }       
	// incerment dns_id so we don't get this session confused with another session.             
	dns_id++;

	// release bound socket
	udp_close(s);

	// Release memory
	MEM_FreeBlock(memblock);

	// Store Cache Element


	// return code
	return(ret);
}
   


