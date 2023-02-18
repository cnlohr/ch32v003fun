#include <stdio.h>
#include "wch_link_base.h"

// TODO Make me actually query data!

int main()
{
	
	libusb_device_handle * devh = wch_link_base_setup();
	int transferred;
	int status;
	char rbuff[1024];
	libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 1 ); // Clear out any pending transfers.

	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0d\x01\x01", 4, &transferred, WCHTIMEOUT ) ); // Checkme with BULK_IN
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 500 ) ); // Ignore respone.
// "\x1b\x00\x20\xda\x62\x67\x86\xe0\xff\xff\x00\x00\x00\x00\x09\x00" \
"\x01\x02\x00\x2f\x00\x81\x03\x07\x00\x00\x00\x82\x0d\x04\x02\x08" \
"\x02\x00" << Back in

	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0c\x02\x09\x01", 5, &transferred, WCHTIMEOUT ) ); // Checkme with BULK_IN
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 500 ) ); // Ignore respone.
//"\x82\x0c\x01\x01" << Back In


	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0d\x01\x02", 4, &transferred, WCHTIMEOUT) ); // Checkme with BULK_IN
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 500 ) ); // Ignore respone.
//"\x82\x0d\x05\x09\x00\x30\x05\x00" << back in.


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
