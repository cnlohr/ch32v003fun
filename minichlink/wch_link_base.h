// Tricky: You need to use wch link to use WCH-LinkRV.
//  you can always uninstall it in device manager.  It will be under USB devices or something like that at the bottom.
#ifndef _WCH_LINK_BASE_H
#define _WCH_LINK_BASE_H

#include "libusb.h"

#define WCHTIMEOUT 1000
#define WCHCHECK(x) if( status = x ) { fprintf( stderr, "Bad USB Operation on line %d (%d)\n", __LINE__, status ); exit( status ); }

static inline libusb_device_handle * wch_link_base_setup()
{
	libusb_context * ctx = 0;
	int status;
	status = libusb_init(&ctx);
	if (status < 0) {
		fprintf( stderr, "Error: libusb_init_context() returned %d\n", status );
		exit( status );
	}
	printf( "OK %d %p\n", status, ctx );
	//libusb_set_debug(ctx, 0);
	
	libusb_device **list;
	libusb_device *found = NULL;
	ssize_t cnt = libusb_get_device_list(ctx, &list);
	ssize_t i = 0;
	int err = 0;
	for (i = 0; i < cnt; i++) {
		libusb_device *device = list[i];
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(device,&desc);
		if( r == 0 && desc.idVendor == 0x1a86 && desc.idProduct == 0x8010 ) { found = device; }
	}

	if( !found )
	{
		fprintf( stderr, "Error: couldn't find wch link device\n" );
		exit( -9 );
	}

	//USB\VID_1A86&PID_8010&MI_00\6&3AA7447&0&0000
	//USB\VID_1A86&PID_8010&MI_00
	libusb_device_handle * devh;
	status = libusb_open( found, &devh );
	if( status )
	{
		fprintf( stderr, "Error: couldn't open wch link device (libusb_open() = %d)\n", status );
		exit( -9 );
	}
		
	WCHCHECK( libusb_claim_interface(devh, 0) );
	//uint8_t setup_magic_1[] = { 0xcc, 0x08, 0x38, 0xff, 0x80, 0x00, 0x0a };
	//status = libusb_control_transfer(devh, 0x21 /*bmRequestType*/, 0x09 /*bmRequest*/, 0x3cc, 0, setup_magic_1, sizeof(setup_magic_1), TIMEOUT);
	//printf( "Status0: %d\n", status );
	//uint8_t setup_magic_2[] = { 0xcc, 0x08, 0x0f, 0xff, 0x80, 0x00, 0x0a };
	//status = libusb_control_transfer(devh, 0x21 /*bmRequestType*/, 0x09 /*bmRequest*/, 0x3cc, 0, setup_magic_1, sizeof(setup_magic_1), TIMEOUT);
	//printf( "Status1: %d\n", status );
	return devh;
}

#endif
