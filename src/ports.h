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

#ifndef __PORTS_H__
#define __PORTS_H__

//---------------------------------------------------------------------------
// ports.h - i/o ports header for the IMMV2									-
//---------------------------------------------------------------------------
// Version                                                                  -
//		0.1 Original Version May 21, 2002									-
//---------------------------------------------------------------------------

//#include "config.h"

// Embedded functions that are null in windows
#ifdef WINDOWS

#define	cli()
#define sei()
#define	bit_is_set(x,y)		0
#define bit_is_clear(x,y)	0
#define sbi(x,y)
#define cbi(x,y)

#endif
 
#ifndef BV
#define BV(bit)         (1<<(bit))
#endif

#ifndef cbi
#define cbi(reg,bit)    reg &= ~(BV(bit))
#endif

#ifndef sbi
#define sbi(reg,bit)    reg |= (BV(bit))
#endif

//#define DEBUG_LED 1 

//
// PORT B port pins are as follows:
//
//	SS				PB0 - Slave Select SPI (input/system)
//	SCK				PB1 - SPI clock (input/system)
//	SI				PB2 - SPI Slave In (input/system)
//  SO				PB3 - SPI Slave Out (output)
//  IND1			PB4 - Indicator 1 (output)
//  IND2			PB5 - Indicator 2 (output)
//  IND3			PB6 - Indicator 3 (output)
//  IND4			PB7 - Indicator 4 (output)
//
// Directon Mask for Port B is
//                   
//#define PORTBDIR	0x3d
#define PORTBDIR	0xf8
//
// BIT Mask for Port B signals
//

//#define TXIND		0x04
//#define DTRIND		0x80

//
// PORT C port pins are as follows:
//
//	TXIND			PB2 - this is the Transmit Indicator(output)
//	DTRIND			PB7 - this is the DTR Indicator (output)
//
// Directon Mask for Port C is
//                   
#define PORTCDIR	0x00

//
// Port D pins are as follows 
//
// CTS1				PD4 - Modem CTS - output
//
// Direction Mask for Port D is
//
#define PORTDDIR	0x90
//
// BIT Mask for Port D signals
//


//
// Port E pins are as follows   1e
//
//					PE0	- RX in (input)
//					PE1 - TX out (output)
// CTS				PE2 - this is the Clear To Send Signal (output)
// DSR				PE3 - this is the Data Set Ready Signal  (output)
// DCD				PE4 - this is the ring indicator (RI) (output)
// DTR				PE5 - this is the Data Terminal Ready Signal (input/interrupt)
// RTS				PE6 - this is the Request to Send Signal (input)
// autobaud			PE7 - this is the autobaud interrupt pin (input)
//
//
// Direction Mask for Port E is
//
#define PORTEDIR	0x1e
//
// BIT Mask for Port E signals
//
#define CTSTLL		0x04
#define DSRTLL		0x08
#define RITLL		0x10
#define DTRTTL		0x20
#define RTSTTL		0x40
#define DCDTLL		0x80


//
// Port G pins are as follows  08
//
// DCD1i			PG0 - Carrier Detect Input Port 1 (input)
// DSR1i			PG1 - Data Set Ready Input Port 1 (input)
// RI0o				PG3	- Ring Output Serial Zero (output)
// SW1				PG4 - Switch/interrupt 1 (input)
//
// Direction Mask for Port G is
//
#define PORTGDIR	0x08

//
// Pin Macros
//
//#define ABTTL_ON()		sbi(PORTD,0)
//#define ABTTL_OFF()		cbi(PORTD,0)
//#define	ABCHK()			bit_is_set(PIND, 0) 

//
// Modem Reset
//
//#define	MODEM_RST_OFF()	sbi(PORTD,5)
//#define	MODEM_RST_ON()	cbi(PORTD,5)
//#define	MODEM_RST_CHK()	bit_is_set(PIND, 5) 

//
// Serial Port 0
//
// Clear to Send, PE2
#define CTS0_ON()		cbi(PORTE,2) 
#define CTS0_OFF()		sbi(PORTE,2)  
// Request to Send for console PE6 - (input)
#define	RTS0()			bit_is_clear (PINE, 6) 

// Data Set Ready, PE3
#define DSR0_ON()       cbi(PORTE,3) 
#define DSR0_OFF()		sbi(PORTE,3)  
// Data Terminal Ready for console PE5 (input)
#define	DTR0()			bit_is_clear(PINE, 5) 

// ~DCD Carrier Detect, PE4
#define DCD0_ON()		cbi(PORTE,4)
#define DCD0_OFF()		sbi(PORTE,4)   

// ~RI Ring Indicator, PG3 on AVR
#define RI0_ON()		cbi(PORTG,3)
#define RI0_OFF()		sbi(PORTG,3)

//
// Serial Port 1
//
// /* FIX Swap? */ Clear to Send for port1 is PD4, output
#define RTS1_ON()		cbi(PORTD,5) 
#define RTS1_OFF()		sbi(PORTD,5)  
// Request to Send for port1 is PD5. input
#define	CTS1()			bit_is_clear (PIND, 4) 

// Data Set Ready, PB4 on AVR, P1.4 on 8051, rs232 i/o port control
#define DSR0_ON()       cbi(PORTE,3) 
#define DSR0_OFF()		sbi(PORTE,3)      
// Data Terminal Ready for Modem Port
#define	RTS1()			bit_is_clear (PIND, 4) 
//#define DTR0_ON()		cbi(PORTE,3) 
//#define DTR0_OFF()		sbi(PORTE,3)  

// DTR1	PD7
#define DTR1_ON()       cbi(PORTD,7) 
#define DTR1_OFF()		sbi(PORTD,7)   

// Modem DCD carrier detect PG0
#define DCD1()			bit_is_clear(PING, 0)

// Modem RI	input PD6
#define RI1()			bit_is_clear(PIND, 6)

//
// Indicators
//
// Ind1
#define IND1_ON()		sbi(PORTB,4)
#define IND1_OFF()		cbi(PORTB,4)
// Ind2
#define IND2_ON()		sbi(PORTB,5)
#define IND2_OFF()		cbi(PORTB,5)
// Ind3
#define IND3_ON()		sbi(PORTB,6)
#define IND3_OFF()		cbi(PORTB,6)
// Ind4
#define IND4_ON()		sbi(PORTB,7)
#define IND4_OFF()		cbi(PORTB,7)


// TX Indicator,		+++FIX?+++
#define TXI0_ON()		IND1_ON()                                                      
#define TXI0_OFF()		IND1_OFF()

// RX Indicator			++++FIX++++
#define RXI0_ON()		IND2_ON()
#define RXI0_OFF()		IND2_OFF()

// DCD Indicator
#define DCDI_ON()		IND4_ON()
#define DCDI_OFF()		IND4_OFF()

// DTR Indicator
#define DTRI_ON()		IND3_ON()
#define DTRI_OFF()		IND3_OFF()

//
// Switches/Interrupts
//
#define SW1()			bit_is_clear(PING, 4)
#define SW2()			bit_is_clear(PINC, 4)
#define SW3()			bit_is_clear(PINC, 5)
#define SW4()			bit_is_clear(PINC, 6)
#define SW5()			bit_is_clear(PINC, 7)


// Check for RTS and DTR
//#if USE_SERIAL_FLOW_CNTROL
//#define	RTS()			bit_is_clear (PINE, 6) 
//#define DTR()			bit_is_clear (PINE, 5)
//#else
//#define	RTS()			1 
//#define DTR()			1
//#endif

#endif

