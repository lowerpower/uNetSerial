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

#ifndef __DEBUG_H__
#define __DEBUG_H__

//---------------------------------------------------------------------------
// debug.h - Debug equates for messaging at different debug levels			-
//---------------------------------------------------------------------------
// Version                                                                  -
//		0.1 Original Version Jan 14, 2001									-
//																			-
// (c)2001 Mycal Labs, All Rights Reserved									-
//---------------------------------------------------------------------------

//#define DEBUG_LV1	1
//#define DEBUG_LV2	1
//#define DEBUG_LV4	1

#define PACKET_RX_DEBUG 1
	

//#if DEBUG_LV1 || DEBUG_LV2
//#include <stdio.h> 
//#endif

#ifdef DEBUG_LV1
	#define	DEBUG1			CONSOLE_puts
#else
	#define DEBUG1				//
#endif

#ifdef DEBUG_LV2
	#define	DEBUG2			CONSOLE_puts
#else
	#define DEBUG2				//
#endif

#ifdef DEBUG_LV3
	#define	DEBUG3			CONSOLE_puts
#else
	#define DEBUG3				//
#endif

#ifdef DEBUG_LV4
	#define	DEBUG4			CONSOLE_puts
#else
	#define DEBUG4				//
#endif

#ifdef DEBUG_LV5
	#define	DEBUG5			CONSOLE_puts
#else
	#define DEBUG5				//
#endif

#endif
