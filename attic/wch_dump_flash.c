#include <stdio.h>
#include "wch_link_base.h"

// TODO Make me actually query data!

int main()
{
	int i;
	int transferred;
	int status;
	char rbuff[1024];
	libusb_device_handle * devh = wch_link_base_setup(0);

	wch_link_command( devh, "\x81\x06\x01\x01", 4, 0, 0, 0 );

	// Flush out any pending data.
	libusb_bulk_transfer( devh, 0x82, rbuff, 1024, &transferred, 1 );

	// 3/8 = Read Memory
	// First 4 bytes are big-endian location.
	// Next 4 bytes are big-endian amount.
	uint8_t readop[11] = { 0x81, 0x03, 0x08, };
	uint32_t readptr = 0x08000000;
	uint32_t amount = 16384;
	
	readop[3] = (readptr>>24)&0xff;
	readop[4] = (readptr>>16)&0xff;
	readop[5] = (readptr>>8)&0xff;
	readop[6] = (readptr>>0)&0xff;

	readop[7] = (amount>>24)&0xff;
	readop[8] = (amount>>16)&0xff;
	readop[9] = (amount>>8)&0xff;
	readop[10] = (amount>>0)&0xff;
	
	wch_link_command( devh, readop, 11, 0, 0, 0 );

	// Perform operation
	wch_link_command( devh, "\x81\x02\x01\x0c", 4, 0, 0, 0 );

	printf( "WARNING ENDIAN WILL BE REVERSED\n" );

	uint32_t remain = amount;
	while( remain )
	{
		WCHCHECK( libusb_bulk_transfer( devh, 0x82, rbuff, 1024, &transferred, WCHTIMEOUT ) );
		for( i = 0; i < transferred; i++ )
			printf( "%08x", (uint8_t)rbuff[i] );
		printf( "\n" );
		remain -= transferred;
	}

	wch_link_command( devh, "\x81\x0d\x01\xff", 4, 0, 0, 0 );
}
