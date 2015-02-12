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

/*!																www.mycal.net			
 *---------------------------------------------------------------------------
 *! \file ahdl.c
 *  \brief Ahdlc receive and transmit processor for PPP engine.
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
 * Notes:
 *	adhlc_init could init and return a structure that would be used to call
 *	all the adhlc routines.  This would allow multiple engines to run at the
 *	same time.
 *
 *	Also I'd like to modify the use of the rx buffer to be a circular buffer.
 *	This would allow multiple packets to be queued up.
 *
 *  Current memory usage for this module:
 *
 *	ahdlc_tx_crc				2 bytes
 *  ahdlc_rx_crc				2 bytes
 *	ahdlc_rx_count				2 bytes
 *	ahdlc_flags					1 bytes
 *	ahdlc_max_rx_buffer_size	2 bytes		(this may not be needed)
 *	ahdlc_rx_buffer				2 bytes
 *
 *	Total						11 bytes
 *
 *  Total Stack Depth			22 bytes?
 *
 *  \brief AHDLC receive and transmit code for PPP engine.
 *
*/

/*
Nov29,2004 - adding auto PPP negotiate
*/


/*					*/ 
/* include files 	*/
/*					*/ 
#include	"pppconfig.h"
#include	"ircmd.h"
#include	"mTypes.h"
#include	"ppp.h"
#include	"ahdlc.h"

#include	"console.h"
#include	"conio.h"
#include	"modem.h"
#include	"memory.h"
#include	"globals.h"
#include	"debug.h"

//---------------------------------------------------------------------------
// ahdlc flags bit defins, for ahdlc_flags variable
//---------------------------------------------------------------------------
#define AHDLC_ESCAPED		0x1				// Escaped mode bit
#define	AHDLC_RX_READY		0x2				// Frame is ready bit
#define	AHDLC_RX_ASYNC_MAP	0x4
#define AHDLC_TX_ASYNC_MAP	0x8
#define AHDLC_PFC			0x10
#define AHDLC_ACFC			0x20
#define AHDLC_AUTO_SYNC		0x40			// Set if we try to auto negotiate
#define AHDLC_TO_BIG		0x80			// to big packet, wait for 0x7e

//---------------------------------------------------------------------------
// Private Local Globals
//	10 bytes	- standard
//				- with counters
//---------------------------------------------------------------------------
U16				ahdlc_tx_crc;		/*!< running tx CRC */
U16				ahdlc_rx_crc;		/*!< running rx CRC */
U16				ahdlc_rx_count;		/*!< number of rx bytes processed, cur frame */
U8				ahdlc_flags;		/*!< ahdlc state flags, see above */
S8				current_buffer;

//
// The following can be optimized out
//
U16				ahdlc_max_rx_buffer_size;
//MEMBLOCK		*rx_pkts;
//MEMBLOCK		*current_memblock;
U8				ahdlc_rx_buffer[MAX_RECEIVE_MTU];			// What to do here? +++
//U8				*ahdlc_rx_buffer;

//
// Optional statistics counters.
//
#ifdef AHDLC_COUNTERS
U8				ahdlc_rx_tobig_error;
#endif

//---------------------------------------------------------------------------
// ahdlc_init(buffer, buffersize) - this initializes the ahdlc engine to
//	allow for rx frames.
//---------------------------------------------------------------------------
void
ahdlc_init(U8 *buffer, U16 maxrxbuffersize)
{
	ahdlc_flags=0;
	//current_memblock=MEM_GetBlock();
//	ahdlc_rx_buffer=buffer;
	ahdlc_max_rx_buffer_size=MAX_RECEIVE_MTU;
	//ahdlc_async_map=0;
#ifdef AHDLC_COUNTERS
	ahdlc_rx_tobig_error=0;
#endif
}

//---------------------------------------------------------------------------
// ahdlc_rx_ready() - resets the ahdlc engine to the beginning of frame 
//	state.
//---------------------------------------------------------------------------
void
ahdlc_rx_ready(void)
{
	ahdlc_rx_count		=0;
	ahdlc_rx_crc		=0xffff;
	ahdlc_flags			|=AHDLC_RX_READY;
}

void
ahdlc_autosync(void)
{
	ahdlc_flags			|= AHDLC_AUTO_SYNC;
	ahdlc_flags			&=~AHDLC_RX_READY;
}

U8
ahdlc_is_ready(void)
{
	if (ahdlc_flags & AHDLC_RX_READY)
		return 1;
	return 0;
}

void
ahdlc_down(void)
{
	ahdlc_flags			&=~AHDLC_RX_READY;
}


//---------------------------------------------------------------------------
// ahdlc receive function - This routine processes incoming bytes and tries
//	to build a PPP frame.
//
//
//	Two possible reasons that ahdlc_rx will not process characters:
//		o Buffer is locked - in this case ahdlc_rx returns 1, char
//			sending routing should retry.
//
//	Returns 0 if char was processed
//			1 if char was not processed
//---------------------------------------------------------------------------
U8
ahdlc_rx(U8 c)   
{    
	int i;		// do not need this

//static	U16	protocol;

	//
	// See if we are up or not
	//
	if(!(ahdlc_flags & AHDLC_RX_READY))
	{
		// If we are not up, lets see if we think it is the start of a PPP frame
		if(ahdlc_flags & AHDLC_AUTO_SYNC)
		{
			// check for a PPP start of frame char from the peer 
			if((c==0x7e) || (c==0x7d))
			{
				// Set AHDLC Ready because we might be starting a frame.
				ahdlc_rx_ready();
			}
		}
		else
		{
			// We are not active, nor want to autosync
			return 1;
		}
	}
	/* check to see if PPP packet is useable, we should have hardware flow control set, but
		if host ignores it and sends us a char when the PPP Receive packet is in use, discard
		the character. +++ */
	if(ahdlc_flags & AHDLC_RX_READY)
	{
		// Check to make sure we have a buffer that we can store to, if not return "can't process"
		//if(0==ahdlc_rx_buffer)
		//	return(2);

		//
		// If we have just seen a too big frame, don't process until we see an 0x7e
		//
		if(ahdlc_flags & AHDLC_TO_BIG)
		{
			if(c==0x7e)
			{
				// we've seen a 0x7e start processing new frame
				ahdlc_flags&=~AHDLC_TO_BIG;
				ahdlc_rx_ready();
			}
				return 0;
		}

		/* check to see if character is less than 0x20 hex
			we really should set AHDLC_RX_ASYNC_MAP on by default
			and only turn it off when it is negotiated off to handle
			some buggy stacks.	we can do this by not dropping chars
			when in LCP state 
		
			Maybe this code should be:

			if(c<0x20)
				if(rx_accm & (1<<c))
					Return 0;
		
		*/
		if((c<0x20) && ((ahdlc_flags & AHDLC_RX_ASYNC_MAP)==0))
		{
			// discard character
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1("Discard because char is < 0x20 hex and asysnc map is 0\n");
			}
			return 0;
		}
		/* are we in escaped mode? */
		if(ahdlc_flags & AHDLC_ESCAPED)
		{
			/* set escaped to FALSE */
			ahdlc_flags &= ~AHDLC_ESCAPED;	
			
			/* if value is 0x7e then silently discard and reset receive packet */
			if(c==0x7e)
			{
				ahdlc_rx_ready();
				return 0;
			}
			c=c ^ 0x20;				// incomming char = itself xor 20
		}
		/* handle frame end */
		else if(c == 0x7e)
		{
			if(ahdlc_rx_crc==CRC_GOOD_VALUE)
			{
				//DEBUG3("\npacket with good crc value, len %d\n",ahdlc_rx_count);
				//DEBUG1("Good Packet"CRLF);
				/* we hae a good packet, turn off CTS until we are done with this packet */
				//CTS_OFF();			
				/* remove CRC bytes from packet */
				ahdlc_rx_count-=2;		

				/* lock PPP buffer */
				ahdlc_flags&=~AHDLC_RX_READY;
				//
				// upcall routine must fully process frame before return
				//	as returning signifies that buffer belongs to AHDLC again.
				//
				// We need to attach the buffer to the packet waiting to be processed queue
				// We cannot reuse this buffer until it has been freed, we need to pingpong
				// between 2 PPP rx buffers.
				//
//				if((c & 0x1) && (ahdlc_flags & PPP_PFC))			
//					// Send up packet
//					ppp_upcall((U16)ahdlc_rx_buffer[0], (U8 *)&ahdlc_rx_buffer[1], (U16)(ahdlc_rx_count-1));
//				else
					// Send up packet


				ppp_upcall( (U16)(ahdlc_rx_buffer[0]<<8 | ahdlc_rx_buffer[1]), 
								(U8 *)&ahdlc_rx_buffer[2], (U16)(ahdlc_rx_count-2));
				//
				// Here we need to swap ppp buffers and initilize the ppp engine to be
				// ready for the next buffer
				//					
				ahdlc_rx_ready();
				return 0;
			}
			else if(ahdlc_rx_count>3)
			{
//#ifdef WINDOWS
			//	printf("\npacket with bad crc value, was %x len %d\n",ahdlc_rx_crc, ahdlc_rx_count);
//#else
			if(mm.IMMConfig & PPP_DEBUG)
			{
				DEBUG1("packet length ");
				printShort(ahdlc_rx_count);
				DEBUG1(" with bad crc was ");
				printWord(ahdlc_rx_crc);
				DEBUG1(CRLF);
#if PACKET_RX_DEBUG
				dump_ppp_packet(ahdlc_rx_buffer, ahdlc_rx_count);
#endif
				ahdlc_rx_crc=0xffff;
				for(i=0;i<ahdlc_rx_count;i++)
					ahdlc_rx_crc=crcadd(ahdlc_rx_crc, ahdlc_rx_buffer[i]);
				DEBUG1(" recalc crc is ");
				printWord(ahdlc_rx_crc);
				DEBUG1(CRLF);
			}
					
//#endif
#ifdef AHDLC_COUNTERS
				ahdlc_crc_error++;				
#endif
				// Shouldn't we dump the packet and not pass it up?
				//ppp_upcall((U16)ahdlc_rx_buffer[0], (U8 *)&ahdlc_rx_buffer[0], (U16)(ahdlc_rx_count+2));
	//dump_ppp_packet(&ahdlc_rx_buffer[0],ahdlc_rx_count);

			}
			ahdlc_rx_ready();	
			return 0;
		}
		/* handle escaped chars*/
		else if(c==0x7d)
		{
			ahdlc_flags|=AHDLC_ESCAPED;
			return 0;
		}

		/* try to store char if not to big */
		if(ahdlc_rx_count>=ahdlc_max_rx_buffer_size /*PPP_RX_BUFFER_SIZE*/)
		{ 
#ifdef AHDLC_COUNTERS			
			ahdlc_rx_tobig_error++;
#endif
#ifdef WINDOWS
			DEBUG1("incoming PPP packet to big"CRLF);
#endif
			ahdlc_flags|=AHDLC_TO_BIG;
			ahdlc_rx_ready();
		}
		else
		{
			/* Add CRC in */
			ahdlc_rx_crc=crcadd(ahdlc_rx_crc,c);
			//	DEBUG1("Adding in ");
			//	printByte(c);
			//	DEBUG1(" - ");
			//	printWord(ahdlc_rx_crc);
			//	DEBUG1(CRLF);

			/* do auto ACFC, if packet len is zero discard 0xff and 0x03 */
			if(ahdlc_rx_count==0)
			{
				if((c==0xff) || (c==0x03))
					return 0;
			}
			/* Store char */
			ahdlc_rx_buffer[ahdlc_rx_count++]=c;
		}		
	}
	else
	{
		// We are not active
		return 1;
	}
	return 0;
}





//---------------------------------------------------------------------------
// ahdlc_tx_char(char) - write a character to the serial device, 
//	escape if necessary.
//
// Relies on local global vars	:	ahdlc_tx_crc, ahdlc_flags.
// Modifies local global vars	:	ahdlc_tx_crc.
//---------------------------------------------------------------------------
void
ahdlc_tx_char(U16 protocol,U8 c)
{
	// add in crc
	ahdlc_tx_crc=crcadd(ahdlc_tx_crc,c);
	//
	// See if we need to escape char, we always escape 0x7d and 0x7e, in the case
	// of char < 0x20 we only support async map of default or none, so escape if
	// ASYNC map is not set.  We may want to modify this to support a bitmap set
	// ASYNC map.
	//
	if( (c==0x7d) || (c==0x7e) || 
		( (c<0x20) && ((protocol==LCP) || (ahdlc_flags & PPP_TX_ASYNC_MAP)==0)) )
	
	{
		/* send escape char and xor byte by 0x20 */
		MODEM_putch(0x7d);
		c^=0x20;
	}
	MODEM_putch(c);
}



//																www.mycal.com
//---------------------------------------------------------------------------
// ahdlc_tx(protocol,buffer,len) - Transmit a PPP frame.
//	Buffer contains protocol data, ahdlc_tx addes address, control and
//	protocol data.
//
// Relies on local global vars	:	ahdlc_tx_crc, ahdlc_flags.
// Modifies local global vars	:	ahdlc_tx_crc.
//---------------------------------------------------------------------------
U8
ahdlc_tx(U16 protocol, U8 *header, U8 *buffer, U16 headerlen, U16 datalen)
{
U16	i;
//U8	c;

#ifdef DEBUG_LV4
	printf("\nAHDLC_TX - transmit frame, prot %d, len %d\n"CRLF,protocol,datalen+headerlen);
#endif

//	if(mm.IMMConfig & PPP_DEBUG)
//	{
		//DEBUG1("pw"CRLF);
//	}

	// Check to see that physical layer is up, we can assume is some cases
	
	/* write leading 0x7e */
	MODEM_putch(0x7e);

	/* set initial CRC value */
	ahdlc_tx_crc=0xffff;
	/* send HDLC address and control if not disabled or of LCP frame type */
	//if((0==(ahdlc_flags & PPP_ACFC)) || ((0xc0==buffer[0]) && (0x21==buffer[1]))) 
	if((0==(ahdlc_flags & PPP_ACFC)) || (protocol==LCP)) 
	{
		ahdlc_tx_char(protocol,0xff);
		ahdlc_tx_char(protocol,0x03);
	}
	/* Write Protocol */
	ahdlc_tx_char(protocol,(U8)(protocol>>8));
	ahdlc_tx_char(protocol,(U8)(protocol & 0xff));

	/* write header if it exists */
	for(i=0;i<headerlen;i++)
	{
		/* Get next byte from buffer */
		//c=header[i];
		/* Write it...*/
		ahdlc_tx_char(protocol,header[i]);
	}

	/* Write frame bytes */
	for(i=0;i<datalen;i++)
	{
		/* Get next byte from buffer */
		//c=buffer[i];
		/* Write it...*/
		//printByte(buffer[i]);
		ahdlc_tx_char(protocol,buffer[i]);
	}
	
	/* send crc, lsb then msb */
	i=ahdlc_tx_crc ^ 0xffff;
	ahdlc_tx_char(protocol,(U8)(i & 0xff));
	ahdlc_tx_char(protocol,(U8)((i>>8)&0xff));

	/* write trailing 0x7e, probably not needed but it doesn't hurt*/
	MODEM_putch(0x7e);
	return(0);
}

//																www.mycal.com
//---------------------------------------------------------------------------
// Simple and fast CRC16 routine for embedded processors.
//	Just slightly slower than the table lookup method but consumes
//	almost no space.  Much faster and smaller than the loop and
//	shift method that is widely used in the embedded space. 
//	Can be optimized even more in .ASM
//
//	data = (crcvalue ^ inputchar) & 0xff;
//	data = (data ^ (data << 4)) & 0xff;
//	crc = (crc >> 8) ^ ((data << 8) ^ (data <<3) ^ (data >> 4))
//---------------------------------------------------------------------------
U16
crcadd(U16 crcvalue, U8 c)
{
U16	b;

   b = (crcvalue ^ c) & 0xFF;
   b = (b ^ (b << 4)) & 0xFF;				
   b = (b << 8) ^ (b << 3) ^ (b >> 4);
   
   return((crcvalue >> 8) ^ b);
}


