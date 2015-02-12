//
// ppp test program
//
//
//
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <winbase.h>
#include <time.h>

#include "bget.h"

#include "wserial.h"
#include "mTypes.h"
#include "ppp.h"
#include "ahdlc.h"
#include "mip.h"			// IP address is here

#include "mtcp.h"


HANDLE			ser;

// Test rx packet LCP requst with just magic packet
// 7e ff 7d 23 c0 21 7d 21 7d 20 7d 20 7d 2a 7d 25 7d 26 7d 20 7d 20 7d 20 7d 20 74 2a 7e
rx_pkt[]	= {0x7e, 0xff, 0x7d, 0x23, 0xc0, 0x21, 0x7d, 0x21,
				0x7d, 0x20, 0x7d, 0x20, 0x7d, 0x2a, 0x7d, 0x25, 
				0x7d, 0x26, 0x7d, 0x20, 0x7d, 0x20, 0x7d, 0x20,
				0x7d, 0x20, 0x74, 0x2a, 0x7e, 0x0};
/*
U16
get_seconds()
{
time_t			t;

	time(&t);
	return((U16)t);
}
*/
void
wait_seconds(U16 t)
{
U16 stime;

	stime=TIMER_Get_Second();
	while(1)
    {   
		poll_serial();
	   	if(t <  (U8)(TIMER_Get_Second()-stime)) 
    	{                 
   			break;
    	}
	}      
}


void
mputch(U8 c)
{
	//printf("%2x ",c);
	SendByte(ser,c);
}

void
poll_serial()
{
int ci;

	if((ci=GetByte(ser))!=-1)
	{
			//printf("-%x",ci);
			ahdlc_rx((U8) ci);
	}
}


void 
test_callback(U8 *data, U16 len, U16 urg)
{
U16 i;

	printf("+++Got a packet for our socket+++\n");
	for(i=0;i<len;i++)
		printf("%c",data[i]);
	printf("\n");

}

//
// test 
//

U8	memory[1024];

void
main(argc,argv)
int 	argc;
char 	**argv;
{
	U8		error;
	IPAddr	t;


    puts("Mycal's Embedded PPP stack (c)2001 Mycal Labs");
	puts("	Alpha Version build on " __DATE__ " at " __TIME__ "\n" );	


	printf("header buffer size = %d\n",sizeof(IP)+sizeof(TCP)+4);

	if(!(ser=OpenSerial(4)))
	{
		printf("\nCould not open Serial Port\n");
		exit(1);
	}

	// Initialize memory
	bpool(memory, 1024);

	// Empty serial port
	while(GetByte(ser)!=-1)
		;

#ifdef STATIC_PACKET_CHECK
	//
	// Initialize and start PPP
	//
	init_ppp();

	index=0;
	while(rx_pkt[index]!=0)
	{
		ppp_task();
		ahdlc_rx(rx_pkt[index++]);
	}

#endif


#ifndef STATIC_PACKET_CHECK

	
	if(error=raise_ppp(0,"hello","hello"))
	{
		printf("ppp did not come up, error = %d\n",error);
	}
	else
	{
		printf("ppp came up!\n");
	}

#endif

	printf("wait...\n");
	printf("\n");
	while(!kbhit())
	{
		poll_serial();
	}
	getch();
	// Empty serial port
	//while(GetByte(ser)!=-1)
	//	;

	if(RET_CODE_OK==	resolve_name(&t, "www.svn.net"))
	{
		printf("DNS resolved %d.%d.%d.%d\n",t.ipb1,t.ipb2,t.ipb3,
				t.ipb4);
	}
	else
		printf("DNS lookup failed\n");


	printf("tcpopen\n");
	error=tcp_open(t, 80,&test_callback);
	printf("outtcpopen\n");
	if(error)
	{
		printf("tcpsend\n");
		tcp_send(error, "hello dude", 6);
		wait_seconds(5);
		tcp_close(error);
	}

printf("thats all folks, press anykey to exit\n");
	while(!kbhit())
	{
		poll_serial();
	}

	// Empty serial port
	while(GetByte(ser)!=-1)
		;

	printf("were gone\n");


	//
	// Close Serial Port
	//
	CloseSerial(ser);


}



