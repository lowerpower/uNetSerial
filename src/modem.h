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

#ifndef __MODEM_H__
#define __MODEM_H__

/*																www.nchip.com
 *---------------------------------------------------------------------------
 * modem.h - Modem and modem port functions, windows console version		-
 *---------------------------------------------------------------------------
 * Version                                                                  -                                            
 *		0.1 Original Version AUG 21, 2002									-        
 *
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2002, nChip/Mycal Labs, All Rights Reserved.				_
 *															               	_
 *---------------------------------------------------------------------------
*/
#include "mTypes.h"

//
// private
//
void	UART1_Init(U16 Baud_Rate );

//
// Public
//
void MODEM_putch(U8 c);
S16  MODEM_getch(void);
void MODEM_pushch(U8 c);
char MODEM_kbhit(void);
void MODEM_puts(char *buf);

#ifdef WINDOWS
void	MODEM_close(void);
S8		MODEM_init(U8 port,U16 speed);
#else
S8		MODEM_init(U16 speed);
#endif




#endif


