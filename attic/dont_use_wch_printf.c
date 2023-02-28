#include <stdio.h>
#include "wch_link_base.h"

// TESTED

int main()
{
	libusb_device_handle * devh = wch_link_base_setup(1);

	// Place part into reset.
	wch_link_command( devh, "\x81\x0d\x01\x01", 4, 0, 0, 0 );	// Reply is: "\x82\x0d\x04\x02\x08\x02\x00"
	// TODO: What in the world is this?  It doesn't appear to be needed.
	wch_link_command( devh, "\x81\x0c\x02\x09\x02", 5, 0, 0, 0 ); //Reply is: 820c0101

//		// This puts the processor on hold to allow the debugger to run.
//		wch_link_command( devh, "\x81\x0d\x01\x02", 4, 0, 0, 0 ); // Reply: Ignored, 820d050900300500

	wch_link_command( devh, "\x81\x0d\x01\x02", 4, 0, 0, 0 );

	// Issue reset
//	wch_link_command( devh, "\x81\x0b\x01\x01", 4, 0, 0, 0 );
	// Why does db[1] = 6 appear to be some sort of query?
	// Also 0x0b appears to be a query.  But it wrecks up the chip.
	// db[1] = 0xd DOES WRITE TO 0xe0000000.  But is it predictable?
	// db[1] = 8 => get some sort of status back.  This is VERY INTERESTING
	// db[11]=Get chip status.  Only works in reboot mode.
	// DO NOT 0x0f!

	// when in pause mode
	// d 0 2 => Go into reset mode,and do more stuff.
	// d 0 10 => Force chip into mode but no preamble.
	//    Found one: 0x0d 0x00 0x03 will write something into the data reg when in debug mode.
	// Will write other data into the debug register.

	// 0x81, 0x00 = ??? No ops.
	// 0x81, 0x01 = ??? No ops.
	// 0x81, 0x01 = ??? No ops.
	// 0x81, 0x02 = Lots of ops, if in debug mode, wacks chip.  If regular, no bug.
	// 0x81, 0x03 = ??? No ops.
	// 0x81, 0x04 = ??? No ops.
	// 0x81, 0x05 = ??? No ops.
	// 0x81, 0x06 = Lots of ops ,no clarity.
	// 0x81, 0x07 = ??? No ops.
	// 0x81, 0x08 = Lots of ops.
	//   In debug mode: 82 08 06 00 00 00 00 00 00
	//   In run mode:   82 08 06 7f ff ff ff ff 03 
	// 0x81, 0x09 = ??? No ops.
	// 0x81, 0x0a = ??? No ops
	// 0x81, 0x0b = Lots of chatter but no go.
	// 0x81, 0x0c = ??? No ops
	// 0x81, 0x0d, 0x00, 0x01  --> Halts in such a way d,1,ff does't unhalt.
	// 0x81, 0x0d, 0x01, 0x01  --> Halts in such a way d,1,ff does't unhalt.
	// 0x81, 0x0d, 0x01, 0x02  --> Normal Halt.
	// 0x81, 0x0d, 0x00, 0x03  --> If in debug mode, will write cdef89ab into f4 (data0)
	// 0x81, 0x0e = ??? No ops
	// 0x81, 0x10 = ??? No ops
	// 0x81, 0x11, 0, 0: ==> 20: 82 0d 04 02 08 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	//      or sometimes ==> 20: 82 0d 04 02 08 02 00 00 88 b3 bc 48 ff ff ff ff 00 30 05 00 
	//  Point is 0x11 actually comms with the device in non-reset mode. 
	// 0x81, 0x12 = ??? No ops
	// 0x81, 0x13 = ??? No ops
	// 0x81, 0x14 = ??? No ops
	// 0x81, 0x15 = ??? No ops

	unsigned char databuff[11] = { 0x81, 0x08, 0x08, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	unsigned char rbuff[1024];
	int i, j;
//	for( i = 0; i < 20; i++ )
//	for( j = 0; j < 20; j++ )
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
