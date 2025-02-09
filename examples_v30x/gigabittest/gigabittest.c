#include "ch32fun.h"
#include <stdio.h>

// If EVT R2 or cnlohr's Rev G or later... otherwise uncomment.
// It isn't really needed anyway.
//#define CH32V307GIGABIT_PHY_RSTB PC6
// PA10 for Rev F or earlier of cnlohr's board
#define CH32V307GIGABIT_PHY_RSTB PA10

#include "ch32v307gigabit.h"

int ch32v307ethInitHandlePacket( uint8_t * data, int frame_length, ETH_DMADESCTypeDef * dmadesc )
{
	printf( "Rx: %d\n", (int)frame_length );
	int i;
	for( i = 0; i < frame_length; i++ )
	{
		printf( "%02x%c", data[i], (i&0xf)?' ':'\n' );
	}
	printf( "\n" );
	return 0;
}

void ch32v307ethHandleReconfig( int link, int speed, int duplex )
{
	printf( "Link Change: %d %d %d\n", link, speed, duplex );
}

void ch32v307ethInitHandleTXC( void )
{
	printf( "TX Ok\n" );
}

int main()
{
	SystemInit();
	funGpioInitAll();

	Delay_Ms(50);
	printf( "Startup\n" );
	int r = ch32v307ethInit();
	printf( "R: %d\n",r );
	printf( "%02x:%02x:%02x:%02x:%02x:%02x\n", ch32v307eth_mac[0], ch32v307eth_mac[1], ch32v307eth_mac[2], ch32v307eth_mac[3], ch32v307eth_mac[4], ch32v307eth_mac[5] );

	uint8_t testframe[] = { 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination
		0x02, 0xcd, 0xef, 0x12, 0x34, 0x56, // Source
		0x08, 0x00, // IP
		0x45, 0x00, // IP version + ToS
		0x00, 0x32, // 50-byte full payload.
		0x00, 0x00, // Identification
		0x40, 0x00, // Flags (Don't fragment) and offset.
		0x40, //TTL
		0x11, // UDP
		0x00, 0x00, // Header Checksum
		0x01, 0x02, 0x03, 0x04, // Source Address
		0xff, 0xff, 0xff, 0xff, // Destionation Address
		0x04, 0x01, // Port 1025 Source
		0x04, 0x02, // Port 1026 Destination
		0x00, 0x1E, // 30-byte payload
		0x00, 0x00, // Checksum
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
	};

	while(1)
	{
		ch32v307ethTickPhy();
		testframe[sizeof(testframe)-10]++;
		int r1 = ch32v307ethTransmitStatic(testframe, sizeof(testframe), 1 );
		printf( "%d\n", r1 );
		Delay_Ms( 100 );
	}
}
