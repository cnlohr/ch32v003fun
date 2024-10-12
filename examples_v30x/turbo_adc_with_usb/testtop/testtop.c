#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <libusb-1.0/libusb.h>
#include "os_generic.h"

#define CNFG_IMPLEMENTATION
#include "rawdraw_sf.h"

#define USB_VENDOR_ID  0xabcd
#define USB_PRODUCT_ID 0xD335

#define USB_TIMEOUT 1024


#define block_size 512

#define TRANSFERS 64
uint8_t buffers[TRANSFERS][block_size];

static libusb_context *ctx = NULL;
static libusb_device_handle *handle;

int abortLoop = 0;

static void sighandler(int signum)
{
	printf( "\nInterrupt signal received\n" );
	abortLoop = 1;
}

int xfertotal = 0;

int windowhead = 0;
uint16_t window[8192][2];

void xcallback (struct libusb_transfer *transfer)
{
	xfertotal += transfer->actual_length;
	//printf( "%d\n", transfer->actual_length );

	int al = transfer->actual_length;

	int k;
	uint32_t * b = (uint32_t*)transfer->buffer;
	for( k = 0; k < al/4; k++ )
	{
		*((uint32_t*)&window[windowhead++][0]) = b[k];
		if( windowhead == 8192 ) windowhead = 0;
	}
	libusb_submit_transfer( transfer );
}


void HandleKey( int keycode, int bDown ) { }
void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
int HandleDestroy() { return 0; }

int main(int argc, char **argv)
{
	//Pass Interrupt Signal to our handler
	signal(SIGINT, sighandler);

	libusb_init(&ctx);
	libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, 3);

	handle = libusb_open_device_with_vid_pid( ctx, USB_VENDOR_ID, USB_PRODUCT_ID );
	if ( !handle )
	{
		fprintf( stderr, "Error: couldn't find handle\n" );
		return 1;
	}


	CNFGSetup( "Example App", 1024, 768 );

	libusb_detach_kernel_driver(handle, 1);

	int r = 1;
	r = libusb_claim_interface(handle, 1 );
	if( r < 0 )
	{
		fprintf(stderr, "usb_claim_interface error %d\n", r);
		return 2;
	}

	double dRecvTotalTime = 0;
	double dSendTotalTime = 0;
	double dLastPrint = OGGetAbsoluteTime();
	int rtotal = 0, stotal = 0;

	// Async (Fast, bulk)
	// About 260-320 Mbit/s

	struct libusb_transfer * transfers[TRANSFERS];
	int n;
	for( n = 0; n < TRANSFERS; n++ )
	{
		int k;
		for( k = 0; k < block_size; k++ )
		{
			buffers[n][k] = k;
		}
		struct libusb_transfer * t = transfers[n] = libusb_alloc_transfer( 0 );
		libusb_fill_bulk_transfer( t, handle, 0x82, buffers[n], block_size, xcallback, (void*)(intptr_t)n, 1000 );
		libusb_submit_transfer( t );
	}


	while(!abortLoop)
	{
		double dNow = OGGetAbsoluteTime();

		CNFGClearFrame();
		short w, h;
		CNFGGetDimensions( &w, &h );
		CNFGHandleInput();
		libusb_handle_events(ctx);

		int i;
		int whs = (windowhead-1+8192)%8192;
		int w0 = ((int)window[whs][0]) * h / 4096;
		int w1 = ((int)window[whs][1]) * h / 4096;
		CNFGColor( 0xffffffff );
		for( i = 0; i < w-1; i++ )
		{
			whs = (whs+1)%8192;
			int v0 = ((int)window[whs][0]) * h / 4096;
			int v1 = ((int)window[whs][1]) * h / 4096;
			CNFGTackSegment( i, w0, i+1, v0 );
			CNFGTackSegment( i, w1, i+1, v1 );
			w0 = v0;
			w1 = v1; 
		}
		
		libusb_handle_events(ctx);
		CNFGSwapBuffers();

		if( dNow - dLastPrint > 1 )
		{
			dSendTotalTime = dNow - dLastPrint;
			printf( "%f MSPS (%f MB/sec) RX\n", xfertotal / (dSendTotalTime * 1000 * 1000) * 512 / 512 / 4, xfertotal / (dSendTotalTime * 1024 * 1024) );

			int k;
			for( k = 0; k < 512; k++ )
			{
				printf( "%02x ", buffers[0][k] );
			}
			printf( "\n" );
			xfertotal = 0;
			dSendTotalTime = 0;
			dLastPrint = dNow;
		}
	}

	for( n = 0; n < TRANSFERS; n++ )
	{
		libusb_cancel_transfer( transfers[n] );
	 	libusb_free_transfer( transfers[n] );
	}


	libusb_release_interface (handle, 0);
	libusb_close(handle);
	libusb_exit(NULL);

	return 0;
}

