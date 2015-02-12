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
 * utils.c - generic utility functions.										-
 *																			-
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version October 17, 2002								-        
 *
 *---------------------------------------------------------------------------    
 *                                                             				-
 * Copyright (C) 2000, Mycal Labs www.mycal.net								-
 *                                                                         	-
 *---------------------------------------------------------------------------
*/
/*					*/ 
/* include files 	*/
/*					*/ 

#include "utils.h"



//
// copy source to dest, return end of dest
//

U8 *
bytecopy(U8 *dest,U8 *source, U16 length)
{
	U16 i=0;

	if((dest) && (source))
	{
		while(i<length)
		{
			dest[i]=source[i];
			i++;
		}
	}
	else
		return(dest);

	return(&dest[i]);
}