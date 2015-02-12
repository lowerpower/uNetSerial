/*															
 *---------------------------------------------------------------------------
 * ahdlc.c - Ahdlc receive and transmit processor for PPP engine.			-
 *																			-
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 3, 2000									-        
 *
 *---------------------------------------------------------------------------    
 *                                                             				-
 * Copyright (C) 2000, Mycal Labs www.mycal.com								-
 *                                                                         	-
 *---------------------------------------------------------------------------
*/
/*					*/ 
/* include files 	*/
/*					*/ 
                       
#include <io.h>  


#define CRC_GOOD_VALUE	0xf0b8


volatile U8		ppp_rx_buffer[PPP_RX_BUFFER_SIZE];
volatile U8		ppp_rx_count;

U8				ppp_status;
U8				ppp_flags;
U16				ppp_crc_error;
U16				ppp_rx_crc;
U16				ppp_tx_crc;


void
init_ppp()
{
	ppp_crc_error=0;
	ppp_ready();
}

/* Tell PPP engine to init buffer and be ready to receive frames */
void
ppp_ready()
{
	ppp_rx_count=0;
	ppp_status=PPP_READY;
	ppp_crc=0xffff;
	ppp_flags=0;
	CTS_ON();
}



/* ahdlc receive function */
void
ahdlc_rx(U8 c)   
{    
	/* check to see if PPP packet is useable, we should have hardware flow control set, but
		if host ignores it and sends us a char when the PPP Receive packet is in use, discard
		the character. */
	if(ppp_status==PPP_READY)
	{
		/* check to see if character is less than 0x20 hex */
		if((c<0x20) && ((ppp_flags & PPP_RX_ASYNC_MAP)==0))
		{
			// discard character
			return;
		}
		/* are we in escaped mode? */
		if(ppp_flags & PPP_ESCAPED)
		{
			/* set escaped to FALSE */
			ppp_flags &= ~PPP_ESCAPED;	
			
			/* if value is 0x7e then silently discard and reset receive packet */
			if(c==0x7e)
			{
				ppp_ready();
				return;
			}
			c=c ^ 0x20;				// incomming char = itself xor 20
		}
		/* handle frame end */
		else if(c == 0x7e)
		{
			if(ppp_crc==CRC_GOOD_VALUE)
			{
				/* we hae a good packet, turn off CTS until we are done with this packet */
				CTS_OFF();			
				/* remove CRC bytes from packet */
				ppp_rx_count-=2;		
				/* Set flag to signify CRC frame ready */
				ppp_status=PPP_RX_READY;
				/* lock PPP buffer */
				ppp_flags&=~PPP_READY;
				return;
			}
			else if(ppp_rx_count>3)
			{
				ppp_crc_error++;
			}
			ppp_ready();					
		}
		/* handle escaped chars*/
		else if(c==0x7d)
		{
			ppp_flags|=PPP_ESCAPED;
			return;
		}

		/* try to store char if not to big */
		if(ppp_rx_count>=PPP_RX_BUFFER_SIZE)
		{
			ppp_tobig_error++;
			ppp_ready();
		}
		else
		{
			/* Add CRC in */
			ppp_rx_crc=crcadd(ppp_rx_crc,c);
			/* do auto ACFC, if packet len is zero discard 0xff and 0x03 */
			if(ppp_rx_count==0)
			{
				if((c==0xff) || (c==0x03))
					return;
			}
			/* Store char */
			ppp_rx_buffer[ppp_rx_count++]=c;
		}		
	}
}


/* write a character to serial device, escape if necessary */
void
ppp_tx_char(U8 c)
{
	if((c==0x7d) || (c==0x7e) || ((c<0x20) && ((ppp_flags & PPP_TX_ASYNC_MAP)==0)))
	{
		/* send escape char and xor byte by 0x20 */
		mputch(0x7d);
		c^=0x20;
	}
	mputch(c);
}

/* PPP AHDLC transmit routine */
U8
ppp_tx(U8 *buffer, U16 len)
{
U16	i;
U8	c;

	/* send HDLC address and control if not disabled and set initial CRC value */
	if((ppp_flags & PPP_ACFC)==0)
	{
		mputch(0xff);
		mputch(0x03);
		ppp_tx_crc=0xffff;			/* we set initial crc preprocessed with 0xff and 0x03 here */
	}
	else
	{
		ppp_tx_crc=0xffff;
	}

	/* Write frame bytes */
	for(i=0;i<len;i++)
	{
		/* Get next byte from buffer */
		c=buffer[i];
		/* calculate CRC with byte */
		ppp_tx_crc=crcadd(ppp_tx_crc,c);
		/* check if escape needed...*/
		ppp_tx_char(c);
	}
	/* send crc, lsb then msb */
	ppp_tx_char(ppp_tx_crc & 0xff);
	ppp_tx_char((ppp_tx_crc>>8)&0xff);
	/* write trailing 0x7e */
	mputch(0x7e);
}









 