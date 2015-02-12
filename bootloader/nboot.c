/*
 * Copyright (C) 2003 by egnite Software GmbH. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY EGNITE SOFTWARE GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL EGNITE
 * SOFTWARE GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
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
 * $Log$
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>


#define	LOADER_VERSION		0x0105				// Major/Minor

/*
		UBRR = (clock / 16 * BAUD)-1
  */

#define CPU_CLOCK 			7372800				/* 7.3728Mhz -or- 7372800 */
#define BAUD_RATE			19200
#define	CPU_CLOCK_DIV100	CPU_CLOCK/100
#define BAUD_RATE_DIV100	BAUD_RATE/100
#define BAUDRATE_FACTOR		(CPU_CLOCK_DIV100/(BAUD_RATE_DIV100*16)-1)
/*
 * This value will be written into the baudrate register. Consult your 
 * AVR data sheet for the correct value. On an ATmega128 running at
 * 14.7456 MHz a value of 7 will result in 115200 bps.
 */
//#define BAUDRATE_FACTOR     23

/*
 * Maximum number of consecutive protocol errors until we give up.
 */
#define MAX_PROTO_ERRORS    10

/*
 * We support the original XMODEM protocol only. If you change this,
 * you need to change a lot more.
 */
#define XMODEM_PACKET_SIZE  128

/*
 * Some ASCII code definitions for readability.
 */
#define ASC_TMO  0x00           /* Timeout special */
#define ASC_SOH  0x01           /* Ctrl-A */
#define ASC_EOT  0x04           /* Ctrl-D */
#define ASC_ACK  0x06           /* Ctrl-F */
#define ASC_NAK  0x15           /* Ctrl-U */
#define ASC_CAN  0x18           /* Ctrl-X */
#define ASC_ERR  0xFF           /* Error special */

/*
 * Indicates timeout if zero.
 */
volatile unsigned short	marker;
volatile unsigned short intime;

/*
 * Function prototypes.
 */
static void FlashPage(unsigned long address, unsigned char *data);
static void SendOctet(char ch);
static unsigned char RecvPacket(unsigned char pn, unsigned char *cp);

void start(void)
{
	asm volatile ("jmp main");
}

//																www.mycal.com
//---------------------------------------------------------------------------
// version() - return the boot loader version
//---------------------------------------------------------------------------
static short version(void)
{
	return(LOADER_VERSION);
}

//																www.mycal.com
//---------------------------------------------------------------------------
// Simple and fast CRC16 routine for embedded processors.
//	Just slightly slower than the table lookup method but consumes
//	almost no space.  Much faster and smaller than the loop and
//	shift method that is widely used in the embedded space. 
//	Can be optimized even more in .ASM
//
//	data = (crcvalue ^ inputchar) & 0xff;
//	data = (data ^ (data << 4)) & 0xff;
//	crc = (crc >> 8) ^ ((data << 8) ^ (data <<3) ^ (data >> 4))
//---------------------------------------------------------------------------
unsigned short
crcadd(unsigned short crcvalue, unsigned char c)
{
unsigned short	b;

   b = (crcvalue ^ c) & 0xFF;
   b = (b ^ (b << 4)) & 0xFF;				
   b = (b << 8) ^ (b << 3) ^ (b >> 4);
   
   return((crcvalue >> 8) ^ b);
}

#define IND_ON()		sbi(PORTB,4)
#define IND_OFF()		cbi(PORTB,4)
/*
 * Boot loader entry point.
 *
 * Remember that we are running without any initialization or library 
 * code. All CPU registers and global variables are uninitialized.
 *
 * No code is allowed above this point.
 *
 * Compatibility remarks:
 *
 * - I tried to keep the code compatible. However, when using it for
 *   other CPUs than the ATmega128, the it may not even compile.
 *
 * - This code has been tested on an ATmega128 running at 14.7456 MHz.
 *   For lower clock speeds you may have to reduce the baudrate.
 *
 * - It is assumed, that either one or two XMODEM packets fit into a
 *   single program memory page.
 *
 * - Two routines, SendOctet() and RecvOctet(), are used to read from
 *   or write to the ATmega USART0. You may need to change these
 *   routines on different chips.
 * 
 */

int main()
{
    unsigned char	ch;   /* Result of last packet receive or response. */
    unsigned char	ec;   /* Error counter. */
    unsigned short	pn;  /* Expected packet number. */
    unsigned short	bofs;/* Packet buffer offset. */
    unsigned long	addr; /* Program memory byte address. */
    unsigned char	buff[SPM_PAGESIZE];

    /*
     * GCC depends on register r1 set to 0.
     */
    asm volatile ("clr r1");

    /*
     * No interrupts used.
     */
    cli();

	// Set indicator
	outp(0x10,DDRB);
	// Set PINC6 to pullup
	outp(0xf0,PORTC);
	//outp(0x80,PORTB);

	// Set rxi on
	IND_ON();	
	for(pn=0;pn<2000;pn++);

	//
    // Initialize UART baudrate and enable transmitter and receiver if not
	// entering from bootcode.
    //
	if(marker != 0x51f5)
	{
		outb(UBRR0L, (unsigned char)BAUDRATE_FACTOR);
	    outb(UBRR0H, (unsigned char)(BAUDRATE_FACTOR >> 8));
		outb(UCSR0B, BV(RXEN) | BV(TXEN));
	}
	marker=0;

	//
	// Check if we have been here before or are entering from app
	//	code by checking marker
	//

	if(MCUCSR)
	{
		MCUCSR=0;
		if(bit_is_set (PINC, 7))
		{
			asm volatile ("jmp 0");
		}

	}   
	//outp( (BV(RXCIE)|BV(TXCIE)|BV(RXEN)|BV(TXEN)), UCSR0B );
	//
	// Reset Marker so we will boot back into main code on reset
	//
	marker=0x0;

SendOctet('\n');
SendOctet('\r');

SendOctet('n');
SendOctet('C');
SendOctet('h');
SendOctet('i');
SendOctet('p');
SendOctet(' ');
SendOctet('B');
SendOctet('o');
SendOctet('o');
SendOctet('t');
SendOctet('l');
SendOctet('o');
SendOctet('a');
SendOctet('d');
SendOctet('e');
SendOctet('r');
SendOctet(' ');
SendOctet('V');
SendOctet((LOADER_VERSION>>8)+'0');
SendOctet('.');
SendOctet((LOADER_VERSION &0xff)+'0');

SendOctet('\n');
SendOctet('\r');


	/*
     * Wait for the first packet, cancel or end-of-transmission to arrive. 
     * Continuously send NAKs to initiate the transfer.
     */
    pn = 1; /* XMODEM starts with packet number one. */
    for (;;) 
	{
		IND_ON();
        ch = RecvPacket(pn, buff);
		IND_OFF();
        if (ch != ASC_TMO && ch != ASC_ERR) 
		{
            break;
        }
        SendOctet(ASC_NAK);
    }

    /*
     * We received a packet or a request to stop the transfer. 
     */
    ec = 0;
    bofs = 0;
    addr = 0;
    for (;;) 
	{

        /*
         * Process a packet.
         */
        if (ch == ASC_SOH) 
		{

#if XMODEM_PACKET_SIZE != SPM_PAGESIZE
            /*
             * If the packet size differs from the size of the flash write
             * buffer, we assume, that two packets fit into one page. This
             * may not be true for all AVR devices.
             */
            if (pn & 1) 
			{
                bofs = XMODEM_PACKET_SIZE;
            } 
			else 
#endif
            {
                FlashPage(addr, buff);
                addr += SPM_PAGESIZE;
                bofs = 0;
            }
            ec = 0; /* Clear error counter. */
            pn++;   /* Increment packet number. */
            ch = ASC_ACK;  /* Respond with ACK. */
        } 
        
        /*
         * Process end-of-transmission or cancel.
         */
        else if (ch == ASC_EOT || ch == ASC_CAN) 
		{
            FlashPage(addr, buff);
            SendOctet(ASC_ACK);
            break;
        } 
        
        /*
         * Anything else is treated as an error.
         */
        else 
		{
            if (ec++ > MAX_PROTO_ERRORS) 
			{
                SendOctet(ASC_CAN);
                break;
            }
            ch = ASC_NAK;
        }

        /*
         * Send response and receive next packet.
         */
        SendOctet(ch);
		// Set rxi on
		IND_ON();

        ch = RecvPacket(pn, &buff[bofs]);
		
		// Set Rxi off
		IND_OFF();
	}

	// Reset into new code via bootloader, do not jump.  We use WDT to do this
	// By setting WDT to shortest timeout and loop forever.
	WDTCR=0x08;
    /*
     * Will jump into the loaded code.
     */
    //asm volatile ("jmp 0");

    /*
     * Never return to stop GCC to include a reference to the exit code.
     * Actually we will never reach this point, but the compiler doesn't 
     * understand the assembly statement above.
     */
    for (;;);
}

/*
 * Write the contents of a buffer to a specified program memory address.
 *
 * \param addr Program memory byte address to start writing.
 * \param data Points to a buffer which contains the data to be written.
 *
 * \todo Would be fine to verify the result and return a result to the
 *       caller.
 */
static void FlashPage(unsigned long addr, unsigned char *data)
{
    unsigned long i;

    /*
     * Erase page.
     */
    boot_page_erase(addr);
    while (boot_rww_busy()) {
        boot_rww_enable();
    }

    /*
     * Fill page buffer.
     */
    for (i = addr; i < addr + SPM_PAGESIZE; i += 2) {
        boot_page_fill(i, *data + (*(data + 1) << 8));
        data += 2;
    }

    /*
     * Write page.
     */
    boot_page_write(addr);
    while (boot_rww_busy()) {
        boot_rww_enable();
    }
}



/*
static void SendString(char *ch)
{
	while(*ch!=0)
		SendOctet(*ch++);

}
*/

/*
 * Send a byte to the RS232 interface.
 *
 * \param ch Byte to send.
 */
static void SendOctet(char ch)
{
    /* This may differ on your device. */
    while ((inb(UCSR0A) & _BV(UDRE)) == 0);
    outb(UDR0, ch);
}

/*
 * Receive a byte to the RS232 interface.
 *
 * \return Byte received or ASC_TMO on timeout.
 */
static unsigned char RecvOctet(void)
{
    intime = 1;
    /* This may differ on your device. */
    while ((inb(UCSR0A) & _BV(RXC)) == 0) {
        if (++intime == 0)
            return ASC_TMO;
    }
    return inb(UDR0);
}

/*
 * Receive a protocol packet.
 *
 * \param pn Expected packet number.
 * \param cp Pointer to packet buffer.
 *
 * \return - ASC_SOH if a packet has been received.
 *         - ASC_CAN if remote cancels transmission.
 *         - ASC_EOT if transmission finished.
 *         - ASC_ERR on packet format errors.
 *         - ASC_TMO on timeouts.
 *
 * \todo I think that this will fail if the remote missed a previously
 *       sent ACK.
 */
static unsigned char RecvPacket(unsigned char pn, unsigned char *cp)
{
    unsigned char rc; /* Function result. */
    unsigned char ch; /* Calculated checksum. */
    unsigned char i;

    /*
     * Wait for the first character. Ignore anything except SOH, EOT, CAN
     * or timeout.
     */
    for (;;) {
        if ((rc = RecvOctet()) == ASC_SOH) {
            break;
        }

        /*
         * Return if transmission stopped or timed out.
         */
        if (!intime || rc == ASC_EOT || rc == ASC_CAN) {
            return rc;
        }
    }

    /*
     * We got the start of the header (SOH). Next byte will be the packet
     * number, followed by the inverted packet number.
     */
    ch = RecvOctet();
    if (RecvOctet() + ch != 0xFF || ch != pn)
        rc = ASC_ERR;

    /*
     * Even if the packet number had been invalid, continue receiving. This
     * avoids too many NAK responses by the caller.
     */
    ch = 0;
    for (i = 0; i < 128; i++) {
        ch += (*cp++ = RecvOctet());
        if (!intime) {
            return ASC_TMO;
        }
    }

    /*
     * Finally receive the checksum.
     */
    if (ch != RecvOctet() || !intime)
        rc = ASC_ERR;

    return rc;
}

