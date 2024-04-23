/* Small example showing how to use the SWIO programming pin to 
   do printf through the debug interface */

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
	if( id == 0xaa )
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
		return 255;
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

	while(1)
	{
		//printf( "%lu %08lx %lu %d %d\n", USBDEBUG0, USBDEBUG1, USBDEBUG2, 0, 0 );
		int i;
		for( i = 1; i < 3; i++ )
		{

			uint32_t * buffer = (uint32_t*)USBFS_GetEPBufferIfAvailable( i );
			if( buffer )
			{
				buffer[0] = 0x000000aa;
				USBFS_SendEndpoint( i, (i==1)?8:4 );
			}
		}
	}
}

