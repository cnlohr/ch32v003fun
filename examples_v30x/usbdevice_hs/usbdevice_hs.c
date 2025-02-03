// Small example showing how to use the USB HS interface of the CH32V30x
// A composite HID device + A bulk in and out.

#include "ch32fun.h"
#include <stdio.h>
#include <string.h>
#include "hsusb_v30x.h"
#include "hsusb_v30x.c" // Normally would be in ADDITIONAL_C_FILES, but the PIO build system doesn't currently understand that.

uint32_t count;

int last = 0;
void handle_debug_input( int numbytes, uint8_t * data )
{
	last = data[0];
	count += numbytes;
}

int lrx = 0;
uint8_t scratchpad[16384];

int HandleHidUserSetReportSetup( struct _USBState * ctx, tusb_control_request_t * req )
{
	int id = req->wValue & 0xff;
	if( id == 0xaa && req->wLength <= sizeof(scratchpad) )
	{
		ctx->pCtrlPayloadPtr = scratchpad;
		lrx = req->wLength;
		return req->wLength;
	}
	return 0;
}

int HandleHidUserGetReportSetup( struct _USBState * ctx, tusb_control_request_t * req )
{
	int id = req->wValue & 0xff;
	if( id == 0xaa )
	{
		ctx->pCtrlPayloadPtr = scratchpad;
		if( sizeof(scratchpad) - 1 < lrx )
			return sizeof(scratchpad) - 1;
		else
			return lrx;
	}
	return 0;
}

void HandleHidUserReportDataOut( struct _USBState * ctx, uint8_t * data, int len )
{
}

int HandleHidUserReportDataIn( struct _USBState * ctx, uint8_t * data, int len )
{
	return len;
}

void HandleHidUserReportOutComplete( struct _USBState * ctx )
{
	return;
}


void HandleGotEPComplete( struct _USBState * ctx, int ep )
{
	if( ep == 5 )
	{
		HSUSBCTX.USBHS_Endp_Busy[5] = 0;
		// Received data is written into scratchpad,
		// and USBHSD->RX_LEN

		//printf( "%d\n", USBHSD->RX_LEN );

		// You must re-up these, not sure why but they get corrupt.
		USBHSD->UEP5_RX_DMA = (uintptr_t)scratchpad;
		USBHSD->UEP5_MAX_LEN = 1024;
		USBHSD->UEP5_RX_CTRL ^= USBHS_UEP_R_TOG_DATA1;
		USBHSD->UEP5_RX_CTRL = ((USBHSD->UEP5_RX_CTRL) & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_ACK;

		// for OUT commands (like this one) sizes are pretty flexible.
	}
}


int main()
{
	SystemInit();

	funGpioInitAll();

	funPinMode( PA3, GPIO_CFGLR_OUT_10Mhz_PP );

	HSUSBSetup();

	funDigitalWrite( PA3, FUN_LOW );


	// Override EP5
	USBHSD->UEP5_MAX_LEN = 1024; // Max allowed.
	USBHSD->UEP5_RX_DMA = (uintptr_t)scratchpad;

	int tickcount = 0;

	while(1)
	{
		//printf( "%lu %08lx %lu %d %d\n", USBDEBUG0, USBDEBUG1, USBDEBUG2, 0, 0 );

		// Send data back to PC.
		if( !( HSUSBCTX.USBHS_Endp_Busy[4] & 1 ) )
		{
			USBHS_SendEndpoint( 4, 512, scratchpad );
		}


		int i;
		for( i = 1; i < 3; i++ )
		{

			uint32_t * buffer = (uint32_t*)USBHS_GetEPBufferIfAvailable( i );
			if( buffer )
			{
				int tickDown = ((SysTick->CNT)&0x800000);
				static int wasTickMouse, wasTickKeyboard;
				if( i == 1 )
				{
					// Keyboard  (only press "8" 3 times)
					if( tickcount <= 3 && tickDown && !wasTickKeyboard )
					{
						buffer[0] = 0x00250000;
						tickcount++;
					}
					else
					{
						buffer[0] = 0x00000000;
					}
					buffer[1] = 0x00000000;
					wasTickKeyboard = tickDown;
				}
				else
				{
					buffer[0] = (tickDown && !wasTickMouse)?0x0010100:0x00000000;
					wasTickMouse = tickDown;
				}
				USBHS_SendEndpoint( i, (i==1)?8:4, 0 );
			}
		}
	}
}

