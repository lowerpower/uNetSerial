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

#ifndef __CONSOLE_H__
#define __CONSOLE_H__
/*																www.nchip.com
 *---------------------------------------------------------------------------
 * console.h - Console, fucntions											-
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
#include "console.h"

void CONSOLE_init(U16 baud_rate);
void CONSOLE_putch(char c);
char CONSOLE_getch(void);
char CONSOLE_kbhit(void);
void CONSOLE_puts(char *buf);
void CONSOLE_cputs(char *buf);

#endif
