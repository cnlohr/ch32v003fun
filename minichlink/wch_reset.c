#include <stdio.h>
#include "wch_link_base.h"

// TESTED

// Note - should probably do reads between the writes.

int main()
{
	libusb_device_handle * devh = wch_link_base_setup();
	int transferred;
	uint8_t rbuff[1024];
	int status;
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0b\x01\x01", 4, &transferred, WCHTIMEOUT) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 500 ) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0d\x01\xff", 4, &transferred, WCHTIMEOUT) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 500 ) );
}