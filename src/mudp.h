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

#ifndef __MUDP_H__
#define __MUDP_H__
/*																www.mycal.net			
 *---------------------------------------------------------------------------
 * mudp.h - Mycal UDP Layer header file										-
 *																			-
 *---------------------------------------------------------------------------
 * Version                                                                  -
 *		0.1 Original Version June 3, 1998									-        
 *
 * Exports:
 *	void	
 *
 *---------------------------------------------------------------------------    
 *                                                             				-
 * Copyright (C) 1998, Mycal Labs www.mycal.net								-
 *                                                                         	-
 *---------------------------------------------------------------------------
 * Notes:
 *
*/

#include "mTypes.h"
#include "mip.h"

    
#define MAX_UDP_SOCKETS		2



//
// Define TCP packet structure
//
typedef struct	udp_h
{  
	U16	source_port;
	U16	dest_port;
	U16	length;
	U16	checksum;
	U8	data[];
}UDP;    

//
// Define TCP socket structure
//
typedef struct	udp_socket_h
{  
	U16		port;
	void	(*socket_callback)(struct ip_h *header,U8 *data, U16 len);
	//void	(*socket_callback)(U8 *data, U16 len);
}UDPSOC;   


//
// Exported Functions
//
void	udp_upcall(IP *, UDP *, U16 );

//
// Application Calls (API);
//
void	udp_init(void);
//U8		udp_listen(U16 port, void (*callback)(struct ip_h *header,U8 *data, U16 len));
U8		udp_bind(U16 port, void (*callback)(struct ip_h *header,U8 *data, U16 len));
void	udp_close(U8 socket);
S16		udp_tx(IPAddr dest_ip, U16 dest_port, U16 src_port, U8 *data, U16 len);
U16		udp_port(U8 socket);

#endif
