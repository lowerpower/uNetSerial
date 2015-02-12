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

#ifndef	__MSG_H__
#define __MSG_H__   
/*															
 *---------------------------------------------------------------------------
 * msg.h - Message Result File												-
 *---------------------------------------------------------------------------
 * Original Version                                                         -
 *  Nov 7 2000	- mwj                                                       -
 *---------------------------------------------------------------------------    
 *                                                             				_
 * Copyright (C) 2000, Mycal Labs, All Rights Reserved.		        		_
 *                                                                         	_
 *---------------------------------------------------------------------------
*/    

/*					*/ 
/* include files 	*/
/*					*/ 
#include "mTypes.h"
//#include "flash.h"  
//#include "timer.h"
//#include "conio.h"   
//#include "resolve.h"
//#include "ircmd.h"  

#define	MSG_OK				0
#define MSG_CONNECT			1
#define	MSG_ERROR			4
#define MSG_NOCARRIER		5
#define MSG_FAIL			6
#define MSG_PPPFAIL			128						//80
#define MSG_PPPAUTHFAIL		129
#define MSG_PPPDOWN			130						//81
#define MSG_TCPFAIL			140						//82
#define MSG_SOCDOWN			141						//84
#define MSG_DNSFAIL			150						//87  

#define MSG_NODATA			133                    	//125
#define MSG_EEERROR			134
#define MSG_VERSION			200
#define MSG_EMPTY			0xfd
#define MSG_NULL			0xfe
#define MSG_NONE			0xff


void	displayMessage(U8);     

#endif /* msg_H */     



