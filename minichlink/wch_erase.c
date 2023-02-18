#include <stdio.h>
#include "wch_link_base.h"

// UNTESTED

int main()
{
	libusb_device_handle * devh = wch_link_base_setup();
	int transferred;
	int status;
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0d\x01\x01", 4, &transferred, WCHTIMEOUT ) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0c\x02\x09\x01", 5, &transferred, WCHTIMEOUT ) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0d\x01\x02", 4, &transferred, WCHTIMEOUT) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x11\x01\x09", 4, &transferred, WCHTIMEOUT) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x02\x01\x01", 4, &transferred, WCHTIMEOUT) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0d\x01\x02", 4, &transferred, WCHTIMEOUT) );
	WCHCHECK( libusb_bulk_transfer( devh, 0x01, "\x81\x0d\x01\xff", 4, &transferred, WCHTIMEOUT) );
}
