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
/*! \file ahdlc.h
    \brief ahdlc header file, defines the ahdlc interface methods..

*/
//---------------------------------------------------------------------------
// ahdlc.h - ahdlc header file  											-
//---------------------------------------------------------------------------
// Version                                                                  -
//		0.1 Original Version Jan 14, 2001									-
//																			-
// (c)2001 Mycal Labs, All Rights Reserved									-
//---------------------------------------------------------------------------

//#include "mTypes.h"

/*! \fn void ahdlc_init(U8 *buffer, U16 max_buffer_size)
    \brief Initializes the AHDLC engine.

    \param buffer Not currently used
    \param max_buffer_size Not currently used.
*/
void	ahdlc_init(U8 *buffer, U16 max_buffer_size ); 

/*! \fn void	ahdlc_rx_ready(void)
    \brief Sets up the ahdlc engine for the next frame.
*/
void	ahdlc_rx_ready(void);

/*! \fn void	ahdlc_autosync(void)
    \brief  Sets the ahdlc engine into auto sync mode
*/
void	ahdlc_autosync(void);

/*! \fn U8	ahdlc_is_ready(void)
    \brief  Returns 1 if the ahdlc RX engine has automatically activated or no if set to 0
*/
U8		ahdlc_is_ready(void);


/*! \fn void	ahdlc_down(void);
    \brief Turns off ahdlc_rx processing.
*/
void	ahdlc_down(void);

/*! \fn U8		ahdlc_rx(U8 in_byte);  
    \brief ahdlc receive processor, processes one byte at a time

    \param in_byte byte to process by the AHDLC RX engine.
*/
U8		ahdlc_rx(U8);  

/*! \fn U8	ahdlc_tx(U16 protocol, U8 *header, U8 *buffer, U16 headerlen, U16 bufferlen);
    \brief AHDLC transmit processor, Transmits one AHDLC frame.

    \param protocol PPP protocol to transmit
	\prram header header of first element of frame to transmit
	\param buffer second element of frame to transmit
	\param headerlen length of header to transmit
	\param datalen length of second element to transmit
*/
U8		ahdlc_tx(U16 protocol, U8 *header, U8 *buffer, U16 headerlen, U16 datalen);

/*! \fn U16		crcadd(U16 current_crc, U8 in_byte);  
    \brief CRC16 processor

    \param current_crc current CRC to apply in_byte to
	\param in_byte byte to add to CRC
*/
U16		crcadd(U16, U8 c);

#endif 
