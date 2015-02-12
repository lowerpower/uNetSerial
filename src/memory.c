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
 * memory.c - Memory Management												-
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

//#include <string.h>
#include "pppconfig.h"
#include "mTypes.h"
#include "memory.h"
#include "conio.h"
#include "console.h"
#include "ircmd.h"
#include "globals.h"
#include "debug.h"


MEMBLOCK	*free_list;					/*!< pointer to the memory block free list */
MEMBLOCK	memory[NUM_MEM_BLOCKS];		/*!< Memory block array */
static	U8			used_count;			/*!< Memory blocks currently in use */


/*! \fn void MEM_init(void)
    \brief Initializes the memory block allocator
*/
void
MEM_init()
{
int	i;

#ifdef DEBUG_LV2
	if(mm.IMMConfig & PPP_DEBUG)
	{
		DEBUG1("Createing ");
		printShort(NUM_MEM_BLOCKS);
		DEBUG1(" Memory blocks of ");
		printShort(BLOCK_DATA_SIZE);
		DEBUG1(" Size"CRLF);
		DEBUG1("]"CRLF);
	}
#endif
	// hook up memory blocks
	for(i=0;i<NUM_MEM_BLOCKS;i++)
	{
		memory[i].next=&memory[i+1];
		// tmp clean
		memset(memory[i].data,0,BLOCK_DATA_SIZE);
	
	}
	// Null last one
	memory[i-1].next=0;
	// Hook up head of free list pointer
	free_list=&memory[0];
		//
	used_count=0;

}

/*! \fn MEMBLOCK * MEM_GetBlock(void)
    \brief Returns a memory block of the freelist or 0 if none left

*/
MEMBLOCK *
MEM_GetBlock()
{
MEMBLOCK	*ret;

	// Check free list
	ret=free_list;
	//DEBUG1(CRLF"ret=");
	//printShort(ret);
	//DEBUG1(CRLF);
	if(0!=ret)
	{
		free_list=free_list->next;
		used_count++;
	//	if(used_count>1)
	//	{
	//	DEBUG1("Used Count ");
	//	printByte(used_count);
	//	DEBUG1(CRLF);
	//	}

	}
	// return
	return(ret);
}

/*! \fn void MEM_FreeBlock(MEMBLOCK *block)
    \brief Put a block back on the free list

    \param block pointer to memory block to free

*/
void
MEM_FreeBlock(MEMBLOCK *block)
{
	//
	// Verify block is ok
	//
/*		DEBUG1("free --");
		printShort(block);
		DEBUG1("attach -");
		printShort(free_list);
		DEBUG1(CRLF);
*/
  //
	// Add back onto the head of the free list
	//
	block->next=free_list;
	free_list=block;
	used_count--;
	
	//DEBUG1("Used Count ");
	//printByte(used_count);
	//DEBUG1(CRLF);

}





