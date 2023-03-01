#include <stdio.h>
#include "wch_link_base.h"

// UNTESTED

int main()
{
	uint8_t rbuff[1024];
	libusb_device_handle * devh = wch_link_base_setup(0);
	int transferred;
	int status;
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x02\x01\x01", 4, &transferred, WCHTIMEOUT) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 500 ) ); // Ignore respone.
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0d\x01\x02", 4, &transferred, WCHTIMEOUT) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 500 ) ); // Ignore respone.
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0d\x01\xff", 4, &transferred, WCHTIMEOUT) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 500 ) ); // Ignore respone.
}
