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

#ifndef __RESOLVE_H__
#define __RESOLVE_H__
/*															www.nchip.com
 *---------------------------------------------------------------------------
 * Resolve.h - Include file for resolve.c, a very simple, 					_
 *	very optimized DNS resolver.											_
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 23, 2000									-        
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2000, nChip/Mycal Labs, All Rights Reserved.				_
 *                                                                         	_
 *---------------------------------------------------------------------------
*/

//
// iTypes.h
//               
#include "mTypes.h"
 

//
// Tunable parametser
//                   
#define		MAX_DNS_RETRY			3	// Number of time to retry sending DNS request
#define		DNS_REPLY_WAIT_TIME		4	// How long in seconds to wait for server response
										// Valid range (2-250)
//
// Local Prototypes
//
U16			gobble_name(U8 *data, U16 index);
//
// API - External Prototypes
//
extern		void		Set_DNS_Server(IPAddr *);         
extern		void 		Get_DNS_Server(IPAddr *serverip);
extern 		RET_CODE	resolve_name(IPAddr *, U8 *);

#endif
