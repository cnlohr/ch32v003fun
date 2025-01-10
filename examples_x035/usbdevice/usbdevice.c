/* Example showing the USB Device port on the CH32x03x (ch32v035 / ch32v033)
   This example is primarily for use with hidapi on the host.
*/

#include "ch32v003fun.h"
#include <stdio.h>
#include <string.h>
#include "fsusb.h"

uint32_t count;

int last = 0;
void handle_debug_input( int numbytes, uint8_t * data )
{
	last = data[0];
	count += numbytes;
}

uint8_t scratchpad[256];

int HandleHidUserSetReportSetup( struct _USBState * ctx, tusb_control_request_t * req )
{
	int id = req->wValue & 0xff;
	if( id == 0xaa && req->wLength <= sizeof(scratchpad) )
	{
	//	memset( scratchpad, 0x55, sizeof(scratchpad) );
	//	printf( "SET REPORT! %d [%02x]\n", req->wLength, scratchpad[200] );
		ctx->pCtrlPayloadPtr = scratchpad;
		return req->wLength;
	}
	return 0;
}

int HandleHidUserGetReportSetup( struct _USBState * ctx, tusb_control_request_t * req )
{
	int id = req->wValue & 0xff;
	if( id == 0xaa )
	{
		//printf( "GET REPORT! %d\n", req->wLength );
		ctx->pCtrlPayloadPtr = scratchpad;
		return sizeof(scratchpad) - 1;
	}
	return 0;
}

void HandleHidUserReportDataOut( struct _USBState * ctx, uint8_t * data, int len )
{
}

int HandleHidUserReportDataIn( struct _USBState * ctx, uint8_t * data, int len )
{
//	printf( "IN %d %d %08x %08x\n", len, ctx->USBFS_SetupReqLen, data, FSUSBCTX.ENDPOINTS[0] );
//	memset( data, 0xcc, len );
	return len;
}

void HandleHidUserReportOutComplete( struct _USBState * ctx )
{
	return;
}

int main()
{
	SystemInit();

	funGpioInitAll();

	funPinMode( PA0, GPIO_CFGLR_OUT_10Mhz_PP );

	FSUSBSetup();

	int clicks = 0;

	while(1)
	{
		//printf( "%lu %08lx %lu %d %d\n", USBDEBUG0, USBDEBUG1, USBDEBUG2, 0, 0 );
		int i;
		for( i = 1; i < 3; i++ )
		{

			uint32_t * buffer = (uint32_t*)USBFS_GetEPBufferIfAvailable( i );
			if( buffer )
			{
				int tickDown =  ((SysTick->CNTL)&0x10000);
				static int wasTickMouse, wasTickKeyboard;
				if( i == 1 )
				{
					// Keyboard
					buffer[0] = 0x00000000;
					if(tickDown && !wasTickKeyboard && clicks < 50 )
					{
						buffer[0] = 0x00250000;
						clicks++;
					}
					buffer[1] = 0x00000000;
					wasTickKeyboard = tickDown;
				}
				else
				{
					buffer[0] = (tickDown && !wasTickMouse)?0x0010100:0x00000000;
					wasTickMouse = tickDown;
				}
				USBFS_SendEndpoint( i, (i==1)?8:4 );
			}
		}
	}
}

