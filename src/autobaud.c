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
 * autobaud.c - AVR Autobaud Detection Code									-
 *---------------------------------------------------------------------------   
 * Description-                                                             -
 *	Autobaud detects the serial port BAUD rate and configures it.   		-
 *  Only Useful for 8N1 format, eats character. Uses INT7 and Timer1		-
 *																			-
 *  This has been tested on a Mega128, Mega64 and a AT90S8515 (with some	-
 *	changes).																-
 *                                                                          -
 *  To use :   c=autobaud_setup(); Then set baud rate dividers to c			-
 *																			-
 *  How it works :															-
 *		The serial receive line is high at idle state, so we set an			-
 *		interrupt to fire when we see a falling edge on the serial port RX	-
 *		line (tie an interrupt line to the serial RX line).					-
 *																			-
 *		Once we see the start of a char (first falling edge) set a timer	-
 *		running and set the interrupt to fire on rising edges on the RX		-
 *		line.  Every time this interrupt fires store the count value.  Once -
 *		the timer times out find the last count value for the last rising	-
 *		edge and calculate the bit rate of the char (last rising edge 
 *		should be stop bit)													-
 *																			-
 * Possible Issues:															-
 *		If the char rate is faster than the timout value, you will get a	-
 *		false rate (this should be no issue for interactive users).			-
 *																			-
 *		The timeout rate should be longer than the slowest BAUD rate that	-
 *		you wish to mesure.													-
 *																			-
 *---------------------------------------------------------------------------
 * Version                                                                  -                                            
 *		0.1 Original Version Mike Johnson, Jan 3, 2001						-        
 * 																			-
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2001, Mycal Labs, www.mycal.net							_
 *                                                                         	_
 *---------------------------------------------------------------------------
*///
#ifndef WIN32

#include "pppconfig.h"
#include "autobaud.h"

//#define AUTOBAUD_DEBUG	1
  
//  
// Scale Autobaud Tick Value to uSeconds, we are using timer1, no prescale 
//
#define ATICK		(1000000/(CPU_CLOCK/256)) & 0xff

//  
// Calculate word times in uSeconds for 8N1       
//   
#define BT230400	(((10000/2304)*9)/ATICK)                         
#define BT115200	(((10000/1152)*9)/ATICK)
#define BT57600		(((10000/576)*9)/ATICK)
#define BT38400		(((10000/384)*9)/ATICK)
#define BT19200		(((10000/192)*9)/ATICK)         
#define BT9600		(((10000/96)*9)/ATICK)
#define BT4800		(((10000/48)*9)/ATICK)                   
#define BT2400		(((10000/24)*9)/ATICK)        			
#define BT1200		(((10000/12)*9)/ATICK) 
#define BT300		(((10000/3)*9)/ATICK)

//
// Calculate Baud Divisor Values for above rates
//                                              

#define BD230400	(CPU_CLOCK_DIV100/(2304*16)-1)
#define BD115200	(CPU_CLOCK_DIV100/(1152*16)-1)
#define BD57600     (CPU_CLOCK_DIV100/(576*16)-1)
#define BD38400     (CPU_CLOCK_DIV100/(384*16)-1)
#define BD19200   	(CPU_CLOCK_DIV100/(192*16)-1)
#define BD9600      (CPU_CLOCK_DIV100/(96*16)-1)
#define BD4800      (CPU_CLOCK_DIV100/(48*16)-1)
#define BD2400      (CPU_CLOCK_DIV100/(24*16)-1)
#define BD1200		(CPU_CLOCK_DIV100/(12*16)-1)
//#define BD300		(CPU_CLOCK_DIV100/(3*16)-1)


#ifdef __AVR__	
//
// Lookup table for setting, This table only allows standard BAUD rates,
//	these can be directly calculated in the software.  It is even possible to
//	calculate a correct baud rate divider using any xtal or incoming bit rate.
//
static U8 BTime[] FLASHMEM ={(BT115200+BT57600)/2,(BT57600+BT38400)/2,(BT38400+BT19200)/2,
							 (BT19200+BT9600)/2,(BT9600+BT4800)/2,(BT4800+BT2400)/2,
							 (BT2400+BT300)/2,0};
static U8 STime[] FLASHMEM ={BD115200,BD57600,BD38400,BD19200,BD9600,BD4800,BD2400,0};     

volatile static	U8	state;
volatile static U8	atime; 
volatile static	U8	icount;

#endif

//
// Note, have to rewrite to co-exist with other interrupt sources.
//
void
autobaud_start() 
{   
#ifdef __AVR__	
	// debug wouldn't allow this DEBUG1("autobaud start"CRLF);
	//
	// Initialize Vars
	//                
	state=0;    
	//
	// Make sure Modem HW flow control says we are ready, these macros control the HW
	// Flow Control Pins
	//                                                  
	DSR0_ON();
	CTS0_ON();	
	//
	// Initialize the autobaud detector by enabling the Interrupt7 on falling edge to detect
	// UART char start.
	//
	EICRB=0x80;							// outp(0x80,EICRB);					// Set interrupt7 on falling edge
	EIMSK=0x80;							// outp(0x80,EIMSK);					// Set interrupt7 enable
#endif
	//outp(0x08,MCUCR);					// Set interrupt on falling edge  (legacy)
	//outp(0x80,GIMSK);					// X1XXXXXX 1= enable int 0, ;0XXXXXXX 1= enable int 1		
} 


U8
autobaud_rate()
{  
#ifdef __AVR__
U8	t,i=0;

                          
	//while((t=PRG_RDB(&BTime[i]))!=0)
	while((t=pgm_read_byte(&BTime[i]))!=0)
	{   
		
#ifdef AUTOBAUD_DEBUG	
		b2X(atime, smbuff);
		puts(smbuff);
		b2X(t, smbuff);
		puts(smbuff);		
#endif			
		if(atime<=t)
			break;  
			
		i++;
	}
	return(i);
#else
	return(0);
#endif
}

U8
autobaud_value()
{	  
#ifdef __AVR__		
U8	t;     

	//t=autobaud_rate();     
	 t=pgm_read_byte(&STime[autobaud_rate()]);
	//t=PRG_RDB(&STime[autobaud_rate()]);
	return(t);
#else
	return(0);
#endif
}


//
// uInterent board will check for existance of autobaud wire.
//
//#define AUTOBTTL	0x08 --Mask, bit 3 is the one we are using
//
U8 autobaud_setup()
{            
#ifdef __AVR__	
	autobaud_start(); 
	while(1)
	{
		// Timeout?
		if(state==0)IND1_ON();
		if(state==1)IND2_ON();
		if(state==2)
			break;
	}  
				IND3_ON();
//	UART_Init(autobaud_value());

	return (autobaud_value());
#else

	return(0);
#endif
}    


#ifdef __AVR__	
/* Timer 1 overflow Signal */
SIGNAL(SIG_OVERFLOW1)
{   

	//
	// Set finished state and turn off timer and Interrupt7 signals
	//
	state=2;
	//
	// Turn off timer1 and interrupt7 signals.
	//
	EICRB=0x00;							//outp(0x00,EICRB);					// mask off interrupt7
	
	TIMSK=TIMSK & ~0x4;					//outp((inp(TIMSK) & ~0x4),TIMSK);	// Stop Timer
	
	TCCR1B=0x0;							//outp(0x0,TCCR1B);					// no divider					
	TCNT1H=0x0;							//outp(0,TCNT1H); 
	TCNT1L=0x0;							//outp(0,TCNT1L);
}



/* Interrupt 7 Signal */
SIGNAL(SIG_INTERRUPT7)      
{           
 	//
	// See what state we are in, init or cmp state
	//              
	//IND3_ON();
	if(0==state)
	{
		//
		// Set new state
		//
	    state=1;
		IND3_ON();

		// We have a start bit, reinit SIG_INTERRUPT7 signal to trigger on rising edges 
		EICRB=0xc0;							//outp(0xc0,EICRB);				

		//
		// Start Timer1 and enable Timer1 interrupt (always write TCNT1H first!)
		//     
		TCNT1H=0;							//outp(0x00,TCNT1H);                  
		TCNT1L=0;							//outp(0,TCNT1L);   					// Timer should roll in ~56ms		

		TCCR1B=0x1;							//outp(0x1,TCCR1B);		// no divider
		
		// turn on timer 1
		TIMSK=TIMSK | 0x4;					//outp((inp(TIMSK) | 0x4),TIMSK);

		
    /* count with cpu clock/1024 */
   // outp(0x07,TCCR0);

    /* reset TCNT0 */
//    outp((U8)TIMER_SELECT,TCNT0);		
		
		//
		// Set new state
		//
	    //state=1;
	}
	else if(state==1)
	{
		// We are in triggering on rising edges now,  Save timer value
        //icount=
        icount=TCNT1L;				//inp(TCNT1L);
        atime=TCNT1H;				//inp(TCNT1H);  
        //icount++;
	}
}

#endif

#endif








