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

#ifndef __TIME_H__
#define __TIME_H__
/*																www.mycal.net			
 *---------------------------------------------------------------------------
 * time.h - Time/timer header file											-
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
 */

#include "mTypes.h"


#ifdef __AVR__

/* TIMER rate calculation */
#define TIMER_INTERVAL         10000        /* 10ms in microseconds */
#define TIMER_SCALAR           1024         
#define TIMER_SELECT           (256-((TIMER_INTERVAL*(CPU_CLOCK/TIMER_SCALAR))/1000000))
/* Be careful that this is a valid value!!! */


#define TICKS_PER_SECOND    72 /* 10ms Ticks */
#define SECONDS_PER_MINUTE  60
#define MINUTES_PER_HOUR    60
#define HOURS_PER_DAY       24

#endif




U16 get_seconds(void);

void	timer_set(void);
U8		timer_timeout(U16);
U8		timer_expire(void);


//
// Standard timer API
//
void	TIMER_Init(void);
U16		TIMER_Get_Tick(void);
U16		TIMER_Get_Second(void);
void	TIMER_Wait_Tick(U16 wait_ticks);

#endif
