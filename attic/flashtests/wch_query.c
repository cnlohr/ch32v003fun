#include <stdio.h>
#include "wch_link_base.h"

// TODO Make me actually query data!

int main()
{
	
	libusb_device_handle * devh = wch_link_base_setup(0);
	int transferred;
	int status;
	char rbuff[1024];

	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x06\x01\x01", 4, &transferred, WCHTIMEOUT) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 500 ) ); // Ignore respone.
//"\x82\x06\x01\x02" << back in

	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x11\x01\x09", 4, &transferred, WCHTIMEOUT) ); // Checkme with BULK_IN	
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, WCHTIMEOUT) ); // Checkme with BULK_IN
	printf( "Query data: %d\n", transferred );
	int i;
	for( i = 0; i < transferred; i++ )
	{
		printf( "%02x ", (unsigned char)rbuff[i] );
	}
	printf( "\n" );
	
// Note this is for CH32V003, MCU UID 20-9e-ab-cd-88-b3-bc-84
//Respone is:
//"\xff\xff\x00\x10\x20\x9e\xab\xcd\x88\xb3\xbc\x48\xff\xff\xff\xff" \

//"\x00\x30\x05\x00" << Back in. <<<< QUERY DATA.
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0d\x01\x04", 4, &transferred, WCHTIMEOUT) ); // Odd that it did it twice - maybe something took too long on the proc.
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 500 ) ); // Ignore respone.
//"\x82\x0d\x01\x03" <<< Streamed back in.
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0d\x01\xff", 4, &transferred, WCHTIMEOUT) ); // Odd that it did it twice - maybe something took too long on the proc.
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 500 ) ); // Ignore respone.
//"\x82\x0d\x01\xff" <<< Streamed back in.
}
