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

/*															
 *---------------------------------------------------------------------------
 * conio.c - console i/o routines for the AVR proessor						-
 *	Developed for the IMM project											-
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 23, 2000									-        
 *																			-
 *  Rewrite by mwj August 22 2002											-
 *																			-
 *---------------------------------------------------------------------------    
 *                                                             				-
 * Copyright (C) 2002, nChip/Mycal Labs, All Rights Reserved.				-
 *                                                                         	-
 *---------------------------------------------------------------------------
*/

/*					*/ 
/* include files 	*/
/*					*/          
#include "pppconfig.h"                             
#include "mTypes.h"
#include "imm.h"
#include "conio.h"        
#include "console.h"
//#include "ports.h"


/*
wait()
{
//static d;

	if(!RTS())
		DCDI_OFF();
	else
		DCDI_ON();

	if(!DTR())
		DTRI_OFF();	
	else
		DTRI_ON();	

	if(d++ & 0x8)
	{
		//CTS_ON();
		DCDI_ON();
`		}
	else
	{
		//CTS_OFF();
		DCDI_OFF();
	}	
}
*/


/* byte 2 heX ascii */
char* 
b2X( unsigned char val, char *cp ) 
{
	*cp++ = n2X( val >>  4 );
	*cp++ = n2X( val );
	*cp = '\0';
	return cp;
}

/* short 2 heX ascii */
char* 
s2X( unsigned short val, char *cp ) 
{
	*cp++ = n2X( val >> 12 );
	*cp++ = n2X( val >>  8 );
	*cp++ = n2X( val >>  4 );
	*cp++ = n2X( val );
	*cp = '\0';
	return cp;
}


/* ascii to ip addr */
char *
s2ip( char *cp, IPAddr *ip ) 
{
    IPAddr lip;
    char *lcp = cp;  
    char *sp=cp;

	lip.ip32 = 0;

	/* eg: MS.ms.ls.LS */

    /* MS --> */
	while( ( '0' <= *lcp ) && ( '9' >= *lcp ) )
    {
		lip.ipb1 = ( lip.ipb1 * 10 ) + (*lcp++ - '0');
    }

	if (cp == lcp) return sp; /* watch for no number */
	//if (lip.ipb1 >= 0x100) return lcp;/* watch for .256. */
    /* <-- MS */

    cp = lcp;
	if ('.' != *lcp++) return sp; /* watch for no '.' */

    /* ms --> */
	while( ( '0' <= *lcp ) && ( '9' >= *lcp ) )
    {
		lip.ipb2 = ( lip.ipb2 * 10 ) + (*lcp++ - '0');
    }

	if (cp == lcp) return sp; /* watch for no number */
	//if (lip.ipb2 >= 0x100) return sp;/* watch for .256. */
    /* <-- ms */

    cp = lcp;
	if ('.' != *lcp++) return sp; /* watch for no '.' */

    /* ls --> */
	while( ( '0' <= *lcp ) && ( '9' >= *lcp ) )
    {
		lip.ipb3 = ( lip.ipb3 * 10 ) + (*lcp++ - '0');
    }

	if (cp == lcp) return sp; /* watch for no number */
	//if (lip.ipb3 >= 0x100) return sp;/* watch for .256. */
    /* <-- ls */

    cp = lcp;
	if ('.' != *lcp++) return sp; /* watch for no '.' */

    /* LS --> */
	while( ( '0' <= *lcp ) && ( '9' >= *lcp ) )
    {
		lip.ipb4 = ( lip.ipb4 * 10 ) + (*lcp++ - '0');
    }

	if (cp == lcp) return sp; /* watch for no number */
	//if (lip.ipb4 >= 0x100) return sp;/* watch for .256. */
    /* <-- LS */

    cp = lcp;
    *ip = lip;

	return cp;		/* OK */
}




/* decimal ascii string to short */
unsigned short 
d2s( char *cp ) 
{
	unsigned short val = 0;
	while((*cp>='0') && (*cp<='9')) 
    {
		val *= 10;
	    val += (unsigned short)(*cp-'0');			//d2n(*cp);
        ++cp;
	}
	return val;
}
 
/* ascii to ipport */
char
*s2p( char *str, unsigned short *port ) 
{
    *port = 0;
	while( ( '0' <= *str ) && ( '9' >= *str ) )
    {
		*port = ((Port)*port) * ((U16)10);
	    *port += ((U16)*str-'0');	//d2n(*str));
        ++str;
	}
    return str;
}


/* to decimal ascii */
char 
*_2d( unsigned short val, char *cp, unsigned short dec_digits, unsigned char pad ) 
{	
	unsigned theDigit;
	unsigned short dv;
    unsigned char fill = 0;   
    
	if( (0 == val) && (0 == pad) ) 
    {
		*cp++ = '0';
	}
	else 
    {
		for( dv = dec_digits; dv; dv /= 10 ) 
        {
			theDigit = val / dv;
			if( theDigit || fill ) 
            {
				*cp++ = n2d( theDigit );	/* ascii */
				val -= theDigit * dv;
                fill = 1;
			}
            else 
            {
                if( pad ) *cp++ = '0';
            }
		}
	}
	*cp = '\0';
	return cp;
}       



/* hex ascii string to short */
unsigned short 
X2s( char *cp ) 
{
	unsigned short val = 0;
	while(( *cp > 32 ) && (*cp != '='))
    {
		val *= 0x10;
	    val += X2n(*cp);
        ++cp;
	}
	return val;
}

void
printShort(U16 t)
{   
	//
	// Printout either ASCII dec short
	//
	s2d( t, smbuff );
	CONSOLE_puts(smbuff);
}

void
printByte(U8 t)
{   
	//
	// Printout either ASCII dec short
	//
	b2X(t, smbuff);
	CONSOLE_puts(smbuff);
}

void
printWord(U16 t)
{   
	//
	// Printout either ASCII dec short
	//
	s2X(t, smbuff);
	CONSOLE_puts(smbuff);
}

void
printDWord(U32 t)
{   
U8	*tptr;
	//
	// Printout either ASCII dec short
	//
	tptr=(U8 *)&t;
	printByte(tptr[3]);
	printByte(tptr[2]);
	printByte(tptr[1]);
	printByte(tptr[0]);
}


void
printIP(iIPAddr *ip)
{   
	//
	// printout either ASCII ip dot quad string
	//
		//
		// Write the IP address as ASC string
		//		
		b2d(ip->ipb1, smbuff);
		CONSOLE_puts(smbuff);
		CONSOLE_putch('.');
	
		b2d(ip->ipb2, smbuff);
		CONSOLE_puts(smbuff);
		CONSOLE_putch('.');

		b2d(ip->ipb3, smbuff);
		CONSOLE_puts(smbuff);
		CONSOLE_putch('.');
	
		b2d(ip->ipb4, smbuff);
		CONSOLE_puts(smbuff);
}



//
// 
//
U8
*Get_Byte(U8 *ptr, U8 *val)
{
	//
	// Dump Spaces
	//
	while(*ptr==' ')
			ptr++ ;

	smbuff[0]=(*ptr++ | 0x20);
	smbuff[1]=(*ptr++ | 0x20);
	smbuff[2]=0;
	
	if('='==smbuff[1])
	{
		ptr--;
	}

	// Convert
	*val=(unsigned char)X2s(&smbuff[0]); 
	return(ptr);
}

