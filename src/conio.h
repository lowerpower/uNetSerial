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

#ifndef __CONIO_H__
#define __CONIO_H__

//---------------------------------------------------------------------------
// conio.h - Terminal IO header												-
//---------------------------------------------------------------------------
// Version                                                                  -
//		0.1 Original Version May 21, 2000									-
//---------------------------------------------------------------------------
//                                                             				_
// Copyright (C) 2000, Mycal Labs, All Rights Reserved.						_
//                                                                         	_
//---------------------------------------------------------------------------




//#include <string.h>        
//#include "iready.h"
//#include "eprom.h"

/* UART Baud rate calculation, careful that this is a valid value */
//#define UART_BAUD_SELECT       (CPU_CLOCK_DIV100/(UART_BAUD_RATE_DIV100*16)-1)

/* UART Rx Buffer Size */
//#define	UART_RX_BUFFER_SIZE 	16 					/* UART Rx Buffer Size */
//#define UART_RX_FLOW_SIZE		12					/* flow control is active at this level, must be less than buffer size  */


/*                 	*/
/* conio functions 	*/
/*					*/
extern	char			*gets( char *);
extern	char			*_2d( unsigned short val, char *cp, unsigned short dec_digits, unsigned char pad );
//extern	char			getch( void );               
extern	unsigned short 	d2s( char *cp );
extern	char			*s2p( char *str, unsigned short *port );
extern  unsigned short	X2s( char *cp ); 
//extern  char			cputs(char *buf); 
extern  char			_puts(char *buf);
//extern	char			puts(char *buf); 

//
//
//
extern	void			printShort(U16 t);
extern  void			printByte(U8 t);
extern	void			printWord(U16 t);
extern	void			printDWord(U32 t);	
extern	void			printIP(iIPAddr *ip);


//#define kbhit() ((UART_RxSize) | (EE_Read_Flag))
//#define kbhit() ((UART_RxSize))


    /* custom functions */
//#define newline() (putch((U8)0x0D))     

/* nibble 2 heX ascii */
#define n2X(ind) ( ((0x000F&(ind))<10)          \
                   ?('0'+(0x000F&(ind)))        \
                   :('A'+((0x000F&(ind))-10)) )


/* byte 2 heX ascii */
char* 
b2X( unsigned char val, char *cp );

/* short 2 heX ascii */
char* 
s2X( unsigned short val, char *cp );



/* case insensitive hex ascii digit to nibble */
#define X2n(c) ( (('0'<=(c))&&('9'>=(c)))       \
                 ?((c)-'0')                     \
                 :( (('a'<=(c))&&('f'>=(c)))    \
                    ?((c)-'a'+10)               \
                    :( (('A'<=(c))&&('F'>=(c))) \
                       ?((c)-'A'+10)            \
                       :(0xFF) ) ) )

/* hex ascii string to byte */
#define X2b(cp) ((unsigned char)X2s(cp))



/* nibble 2 decimal ascii */
#define n2d(ind) (n2X(ind))

/* byte to decimal ascii */
#define b2d( val, cp ) (_2d((unsigned short)(val),(cp),(100),(0)))

/* short to decimal ascii */
#define s2d( val, cp ) (_2d((val),(cp),(10000),(0)))

/* decimal ascii string to byte */
#define d2b( cp ) ((unsigned char)d2s(cp))

/* ascii to ip addr */
extern char *s2ip( char *cp, IPAddr *ip );

/* ip to ascii */
extern char *ip2s( IPAddr *ip, char *cp );

/* ipport to ascii */
#define p2s(port, str) (s2d((port),(str)))

U8 *Get_Byte(U8 *ptr, U8 *val); 
    

#endif  /* __CONIO_H__ */
