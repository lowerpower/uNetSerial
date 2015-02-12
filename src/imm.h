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

#ifndef __AHDLC_H__
#define __AHDLC_H__
/*																www.nchip.com
 
   *---------------------------------------------------------------------------
 * imm.h - ir command processor	version 2									-
 *---------------------------------------------------------------------------
 * Version                                                                  -                                            
 *		0.1 Original Version AUG 21, 2002									-        
 *
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2002, nChip/Mycal Labs, All Rights Reserved.				_
 *															               	_
 *---------------------------------------------------------------------------

#ifdef WINDOWS
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <winbase.h>
#include <time.h>
#endif

*/
#include "mTypes.h"
//#include "wserial.h"
#include "ppp.h"
#include "ahdlc.h"
#include "mip.h"			// IP address is here
#include "mtcp.h"

//#define	IMM_VERSION	".0002"
extern	U8			smbuff[]; 

//extern void	boot_version(void);
extern int (*boot_version)(void);
extern int (*bootloader)(U16 baud);
#endif
