#include <stdio.h>
#include "wch_link_base.h"

// TESTED

int main()
{
	libusb_device_handle * devh = wch_link_base_setup(0);

	// Issue reset
	wch_link_command( devh, "\x81\x0b\x01\x01", 4, 0, 0, 0 );

	// Close out.
	wch_link_command( devh, "\x81\x0d\x01\xff", 4, 0, 0, 0 );
}
