// Tricky: You need to use wch link to use WCH-LinkRV.
//  you can always uninstall it in device manager.  It will be under USB devices or something like that at the bottom.
#ifndef _WCH_LINK_BASE_H
#define _WCH_LINK_BASE_H

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "libusb.h"

#define WCHTIMEOUT 5000
#define WCHCHECK(x) if( status = x ) { fprintf( stderr, "Bad USB Operation on " __FILE__ ":%d (%d)\n", __LINE__, status ); exit( status ); }

void wch_link_command( libusb_device_handle * devh, const uint8_t * command, int commandlen, int * transferred, uint8_t * reply, int replymax )
{
	uint8_t buffer[1024];
	int got_to_recv = 0;
	int status;
	int transferred_local;
	if( !transferred ) transferred = &transferred_local;
	status = libusb_bulk_transfer( devh, 0x01, (char*)command, commandlen, transferred, WCHTIMEOUT );
	if( status ) goto sendfail;

	got_to_recv = 1;
	if( !reply )
	{
		reply = buffer; replymax = sizeof( buffer );
	}
	
	status = libusb_bulk_transfer( devh, 0x81, (char*)reply, replymax, transferred, WCHTIMEOUT );
	if( status ) goto sendfail;
	return;
sendfail:
	fprintf( stderr, "Error sending WCH command (%s): ", got_to_recv?"on recv":"on send" );
	int i;
	for( i = 0; i < commandlen; i++ )
	{
		printf( "%02x ", command[i] );
	}
	printf( "\n" );
	exit( status );
}

static void wch_link_multicommands( libusb_device_handle * devh, int nrcommands, ... )
{
	int i;
	va_list argp;
	va_start(argp, nrcommands);
	for( i = 0; i < nrcommands; i++ )
	{
		int clen = va_arg(argp, int);
		wch_link_command( devh, va_arg(argp, char *), clen, 0, 0, 0 );
	}
	va_end( argp );
}

static inline libusb_device_handle * wch_link_base_setup()
{
	libusb_context * ctx = 0;
	int status;
	status = libusb_init(&ctx);
	if (status < 0) {
		fprintf( stderr, "Error: libusb_init_context() returned %d\n", status );
		exit( status );
	}
	
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

	libusb_device_handle * devh;
	status = libusb_open( found, &devh );
	if( status )
	{
		fprintf( stderr, "Error: couldn't open wch link device (libusb_open() = %d)\n", status );
		exit( -9 );
	}
		
	WCHCHECK( libusb_claim_interface(devh, 0) );
	
	uint8_t rbuff[1024];
	int transferred;
	libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 1 ); // Clear out any pending transfers.  Don't wait though.

	// Place part into reset.
	wch_link_command( devh, "\x81\x0d\x01\x01", 4, &transferred, rbuff, 1024 );	// Reply is: "\x82\x0d\x04\x02\x08\x02\x00"

	// TODO: What in the world is this?  It doesn't appear to be needed.
	wch_link_command( devh, "\x81\x0c\x02\x09\x01", 5, 0, 0, 0 ); //Reply is: 820c0101

	// This puts the processor on hold to allow the debugger to run.
	wch_link_command( devh, "\x81\x0d\x01\x02", 4, 0, 0, 0 ); // Reply: Ignored, 820d050900300500

	wch_link_command( devh, "\x81\x11\x01\x09", 4, &transferred, rbuff, 1024 ); // Reply: Chip ID + Other data (see below)
	if( transferred != 20 )
	{
		fprintf( stderr, "Error: could not get part status\n" );
		exit( -99 );
	}
	fprintf( stderr, "Part Type (A): 0x%02x%02x\n", rbuff[2], rbuff[3] );  // Is this Flash size?
	fprintf( stderr, "Part UUID    : %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n", rbuff[4], rbuff[5], rbuff[6], rbuff[7], rbuff[8], rbuff[9], rbuff[10], rbuff[11] );
	fprintf( stderr, "PFlags       : %02x-%02x-%02x-%02x\n", rbuff[12], rbuff[13], rbuff[14], rbuff[15] );
	fprintf( stderr, "Part Type (B): %02x-%02x-%02x-%02x\n", rbuff[16], rbuff[17], rbuff[18], rbuff[19] );

	//for( i = 0; i < transferred; i++ )
	//	printf( "%02x ", rbuff[i] );
	//printf( "\n" );

	return devh;
}

#endif
