#include <stdio.h>
#include "wch_link_base.h"

// TESTED

int main()
{
	libusb_device_handle * devh = wch_link_base_setup();

	// Issue reset
//	wch_link_command( devh, "\x81\x0b\x01\x01", 4, 0, 0, 0 );
	// Why does db[1] = 6 appear to be some sort of query?
	// Also 0x0b appears to be a query.  But it wrecks up the chip.
	// db[1] = 0xd DOES WRITE TO 0xe0000000.  But is it predictable?
	// DO NOT 0x0f!
	unsigned char databuff[11] = { 0x81, 0x0d, 0x08, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	unsigned char rbuff[1024];
	int i, j;
	for( i = 1; i < 20; i++ )
	for( j = 1; j < 20; j++ )
	{
		databuff[2] = i;
		databuff[3] = j;
		int transferred;
		wch_link_command( devh, databuff, 11, &transferred, rbuff, 1024 );
		int k;
		printf( "%d, %d: %d: ", i, j, transferred );
		for( k = 0; k < transferred; k++ ) printf( "%02x ", rbuff[k] );
		printf( "\n" );
		usleep(10000);
	}

	// Close out.
	wch_link_command( devh, "\x81\x0d\x01\xff", 4, 0, 0, 0 );
}
