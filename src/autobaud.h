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

#ifndef __AUTOBAUD_H__
#define __AUTOBAUD_H__
/*! \file autobaud.h
    \brief autobaud header file to autobaud AVR serial ports

*/

/*! \fn void autobaud_start(void)
    \brief Initializes the AHDLC engine.
*/
void 	autobaud_start(void); 

/*! \fn U8 autobaud_status(void)
    \brief Initializes the AHDLC engine.
*/
U8		autobaud_status(void);

/*! \fn U8 autobaud_rate(void)
    \brief Initializes the AHDLC engine.
*/
U8		autobaud_rate(void);

/*! \fn U8 autobaud_value(void)
    \brief Initializes the AHDLC engine.
*/
U8		autobaud_value(void);

/*! \fn U8 autobaud_setup(void)
    \brief Initializes the AHDLC engine.
*/
U8 		autobaud_setup(void);

#endif /* __AUTOBAUD_H__ */
