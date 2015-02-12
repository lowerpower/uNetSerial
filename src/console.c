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
 * console.c - Console, fucntions											-
 *---------------------------------------------------------------------------
 * Version                                                                  -                                            
 *		0.1 Original Version AUG 21, 2002									-        
 *
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2002, nChip, All Rights Reserved.							_
 *															               	_
 *---------------------------------------------------------------------------
*/
#include "pppconfig.h"
#include "mTypes.h"
#include "console.h"
//#include "conio.c"
#include "ports.h"
#include "globals.h"


// push back char
static U16		pushch;

#ifdef __AVR__
//
// UART0 Rx FIFO
//                 
#define UART0_RX_BUFFER_SIZE	64
#define UART0_RX_FLOW_SIZE		58             
volatile U8		UART0_RxBuffer[UART0_RX_BUFFER_SIZE];
volatile U8		UART0_RxSize;
volatile U8		UART0_RxHead;
volatile U8		UART0_RxTail;
volatile U8		UART0_Ready;

// push back char
//static U16		pushch;

#define kbhit() ((UART0_RxSize))


/* UART0 Receive Complete Interrupt Function */
SIGNAL(SIG_UART0_RECV)      
{    
U8	c;             

	RXI0_ON();
    /* Get incomming char */
	c=UDR0;				//		c = inp(UDR0);
    
	/* check for room on the fifo, if so store the char, else we toss it. */
	if(UART0_RxSize<UART0_RX_BUFFER_SIZE)
	{
		UART0_RxBuffer[UART0_RxTail++]=c;
		if(UART0_RxTail>=UART0_RX_BUFFER_SIZE)
			UART0_RxTail=0;
		UART0_RxSize++;
		if(UART0_RxSize>=UART0_RX_FLOW_SIZE)
		{
			// Turn Cleart to send off    
			CTS0_OFF();
			DTRI_OFF();
		}
	}
}

/* UART0 Receive Complete Interrupt Function */
SIGNAL(SIG_UART0_TRANS)      
{    
    UART0_Ready = ((U8)0x1);
}

void 
UART0_Init(U16 Baud_Rate )
{
	//
	// Init Rx FIFO
	//    
	UART0_RxSize=UART0_RxHead=UART0_RxTail=0;
    UART0_Ready = ((U8)0x1);
//    pUART0_Buffer = ((U8*)0x0);
//    UART0_PgmMem = ((U8)0x0);
//    UART0_CRLF = ((U8)0x0);

    /* enable RxD/TxD and interrupts */
    UCSR0B=BV(RXCIE)|BV(TXCIE)|BV(RXEN)|BV(TXEN);	// outp( (BV(RXCIE)|BV(TXCIE)|BV(RXEN)|BV(TXEN)), UCSR0B );

    /* set baud rate */
    UBRR0L= Baud_Rate;					//outp( Baud_Rate, UBRR0L ); 
	UBRR0H= Baud_Rate>>8;				//outp( Baud_Rate >> 8, UBRR0H);
    
    /* Set Request to send on */ 
    /* Data Set Ready on */
    CTS0_ON();
	DTRI_ON();
    DSR0_ON();
	pushch=0;
#ifdef DEBUG_LV3
	CONSOLE_puts("console baud rate ");
	printShort(Baud_Rate);
	CONSOLE_puts(CRLF);
#endif
}


wait()
{
//static d;

	if(!RTS0())
		DCDI_OFF();
	else
		DCDI_ON();

//	if(!DTR0())
//		DTRI_OFF();	
//	else
//		DTRI_ON();	


	//putch(".");

/*	if(d++ & 0x8)
	{
		//CTS_ON();
		DCDI_ON();
`		}
	else
	{
		//CTS_OFF();
		DCDI_OFF();
	}	
*/
}


char 
getch( void ) 
{   
U8	c;
	    /* wait for UART to indicate that a character has been received */
    	while( !UART0_RxSize);
    
	    /* get byte from fifo */
		c=UART0_RxBuffer[UART0_RxHead++];
		if(UART0_RxHead >= UART0_RX_BUFFER_SIZE)
			UART0_RxHead=0;
	
		/* Decerment number of bytes in fifo, if we are under flow size make sure flow control is off   */
		/* this is a critical section, we don't want off flow control after the interrupt has turned    */
		/* it on after we've updated the UART_RxSize, so protect this section 							*/

		cli(); 
		UART0_RxSize--;
		if(UART0_RxSize < UART0_RX_FLOW_SIZE)
		{
			// Turn Request to Send on.
	        CTS0_ON();	
			DTRI_ON();
		}    
	    // End critical section
		sei();
	//
	// Return char
	return c;
}


/* putch(char c) - putchar for UART0 */
char 
putch( char c )
/* blocks */
{
    /* wait for UART to become available and we are not throttled by flow control */

	TXI0_ON();

	if(mm.SerialConfig & CONSOLE_RTS)
		while((!UART0_Ready) || !RTS0()) wait();		/* RTS not working on hyperterm? */
	else
		while( !UART0_Ready ) wait();

    UART0_Ready = 0;
	
    /* Send character */
    UDR0=c;					//outp( c, UDR0 );

    return c;
}
#endif

#ifdef WINDOWS
int
print_log_char(char c)
{
        static FILE *fp;
 
        if(!fp)
			if(NULL == (fp = fopen("unetserlog.txt", "a")) )
				return -1;
			else
				fprintf(fp,"\n\n--Log Started---\n");
  
        fprintf(fp, "%c", c);

        return 0;
}
#endif



void CONSOLE_init(U16 baud_rate)
{
#ifdef __AVR__
	UART0_Init(baud_rate);
#endif
}

void
CONSOLE_putch(char c)
{
#ifdef WINDOWS
		print_log_char(c);
#endif
	putch(c);
}	

char
CONSOLE_getch()
{
U8	ret;
	//
	// If a char has been pushed back return it, else return one from the serial port
	//
	if(pushch)
	{
		ret=pushch & 0xff;
		pushch=0;
	}
	else
	{
		ret=getch();
	}
	return(ret);
}

char
CONSOLE_kbhit()
{
	if((pushch) || kbhit())
		return(1);
	return(0);
}


void
CONSOLE_pushch(U8 c)
{
	// push the char back into the serial buffer, flag the buffer is full even if the
	// char is null.  We only handle one pushed back char (all we need to)
	pushch=c | 0x100;
}


//
// CONSOLE_cputs(buf) - this function prints out FLASH based memory strings
//
void
CONSOLE_cputs(char *buf)
{
#ifdef __AVR__
	while(pgm_read_byte(buf))
	//while(PRG_RDB(buf))
	{
		//CONSOLE_putch(PRG_RDB(buf++));
		CONSOLE_putch(pgm_read_byte(buf++) );
	}
#else
	CONSOLE_puts(buf);
#endif
}

void
CONSOLE_puts(char *buf)
{
	if(buf!=0)
	{
		while(*buf)
			CONSOLE_putch(*buf++);
	}
}
