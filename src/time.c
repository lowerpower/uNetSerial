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
 * timeer.c - Time/timer related fucntions. (intel port versions)			-
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
*/
/*					*/ 
/* include files 	*/
/*	
*/
#include "pppconfig.h"
#include "mTypes.h"
#include "ircmd.h"
#include "ports.h"
#include "time.h"
#include "console.h"
#include "globals.h"
#include "debug.h"

//
// Timer start for timer, if we need more we can index and allocate.
//
static	U16		time_start;

#ifdef WINDOWS
//
// Windows get seconds
//
U16 get_seconds()
{
time_t			t;

	time(&t);
	return((U16)t);
}
#else
//
// AVR timer stuff
//
#define TICKS_PER_SECOND    70
/* TIMER global variables */
volatile U8	 TIMER_Counter;
volatile U8	 TIMER_HunMS_Count;
volatile U16 TIMER_Tick;
volatile U16 TIMER_HundredMS;
volatile U8  TIMER_Second;


/* AVR Timer 0 overflow Signal */
SIGNAL(SIG_OVERFLOW0)
{   
	/* incerment timer */
	TIMER_Counter++;
	TIMER_Tick++;
	TIMER_HunMS_Count++;

	// Every 10 incerment
	if(TIMER_HunMS_Count>=10)
	{
		TIMER_HunMS_Count=0;
		TIMER_HundredMS++;
	}

	/* seconds need to be updated? */
	if(TIMER_Counter>TICKS_PER_SECOND)
	{
		TIMER_Second++;
		TIMER_Counter=0;
	}
	
	/* Reset Timer */ 
	TCNT0=TIMER_SELECT;//140;				//outp((U8)(150/*-TICKS_PER_SECOND*/)/*TIMER_SELECT*/,TCNT0);
	//outp((U8)TIMER_SELECT,TCNT0);

	/* turn off indicators */
	TXI0_OFF();
	RXI0_OFF();

}

#endif






void
timer_set(void)
{
	//printf("set timer\n");
	time_start=TIMER_Get_Second();
	if(0==time_start)
		time_start++;
}

U8
timer_timeout(U16 timeout_val)
{
	if(0==time_start)
	{
		//DEBUG1("timerturned off\n");
		return 1;
	}
	if(timeout_val < (U16)(TIMER_Get_Second()-time_start))
	{
		//printf("timeout %d  --  %d \n",timeout_val, TIMER_Get_Second()-time_start);
		if(mm.IMMConfig & PPP_DEBUG)
		{
			DEBUG1("timer timeout"CRLF);
		}
		return 1;
	}
	else
	{
		//printf("notimeout %d  --  %d \n",timeout_val, TIMER_Get_Second()-time_start);
		return 0;
	}
}

U8
timer_expire(void)
{
	///DEBUG1("turn off\n");
	time_start=0;
	return(0);
}


//#define TICKS_PER_SECOND    100
//U16 TIMER_Counter;
//U8  TIMER_Tick;
//U8  TIMER_Second;

//
// This function initilizes timer 0 on the Mega128 (and possibly other avr chips)
//
//
void 
TIMER_Init(void)
{
#ifdef __AVR__
	    /* enable TCNT0 overflow */
	TIMSK=1<<TOIE0;					//  outp((1<<TOIE0),TIMSK);

    /* count with cpu clock/1024 */
    TCCR0=0x07;						//outp(0x07,TCCR0);

    /* reset TCNT0 */
    TCNT0=TIMER_SELECT; //255-TICKS_PER_SECOND;		//outp((U8)(255-TICKS_PER_SECOND)/*TIMER_SELECT*/,TCNT0);


#endif
}


U16
TIMER_Get_Tick()
{
#ifdef WINDOWS
	struct _timeb timebuffer;
	time_t			t;
	U16		ticks;
  
	_ftime( &timebuffer );
	time(&t);

	ticks=0;
	ticks=(U16)(t<<6);
	ticks=ticks + ((timebuffer.millitm  & 0xff)>>2);

	//printf("%d\n",ticks);

	return(ticks);
#else
	// AVR
	return(TIMER_Tick);
#endif   
}


U16
TIMER_Get_TCP_HundredMS()
{
#ifdef WINDOWS
	// windows needs to be fixed +++
	struct _timeb timebuffer;
	time_t			t;
	U16		ticks;
  
	_ftime( &timebuffer );
	time(&t);

	// Calculate
	ticks=0;
	ticks=(U16)(t<<6);
	ticks=ticks + ((timebuffer.millitm  & 0xff)>>2);

	//printf("%d\n",ticks);

	return(ticks);
#else
	// AVR
	return(TIMER_HundredMS);
#endif   
}




U16
TIMER_Get_Second()
{
#ifdef WINDOWS
	return(get_seconds());
#else
	// AVR
	return(TIMER_Second);
#endif
} 

void
TIMER_Wait_Tick(U16 wait_ticks)
{     
U16	tick_start;

	tick_start=TIMER_Get_Tick(); 
    while(1)	
    {   
		//poll_serial();
	   	if(abs(TIMER_Get_Tick()-tick_start) > wait_ticks )	
			break;
	}
}      
