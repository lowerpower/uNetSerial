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

#ifndef __MEMORY_H__
#define __MEMORY_H__

/*																www.nchip.com
 *---------------------------------------------------------------------------
 * memory.h - block memory allocation stuff									-
 *---------------------------------------------------------------------------
 * Version                                                                  -                                            
 *		0.1 Original Version JUN 15, 2003									-        
 *
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2003, nChip/Mycal Labs, All Rights Reserved.				_
 *															               	_
 *---------------------------------------------------------------------------
*/
#include "mTypes.h"

#define NUM_MEM_BLOCKS	4
// IP header+TCP header + TCP data  20+ 20 +128
#define BLOCK_DATA_SIZE	170	


typedef struct memblock_h
{
	struct memblock_h	*next;
	U16					ack;					// Not used
	U16					timer;
	U16					len;
	U8					data[BLOCK_DATA_SIZE];
}MEMBLOCK;



// Module Prototypes
void		MEM_init(void);
MEMBLOCK	*MEM_GetBlock(void);
void		MEM_FreeBlock(MEMBLOCK *block);


#endif
