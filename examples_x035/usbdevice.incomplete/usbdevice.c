/* Small example showing how to use the SWIO programming pin to 
   do printf through the debug interface */

#include "ch32v003fun.h"
#include <stdio.h>
#include "fsusb.h"

uint32_t count;

int last = 0;
void handle_debug_input( int numbytes, uint8_t * data )
{
	last = data[0];
	count += numbytes;
}

int HandleHidNonStandardSetup( struct _USBState * ctx, tusb_control_request_t * req )
{

/*
			int USBFS_SetupReqType = FSUSBCTX.USBFS_SetupReqType  = pUSBFS_SetupReqPak->bmRequestType;
			int USBFS_SetupReqCode = FSUSBCTX.USBFS_SetupReqCode  = pUSBFS_SetupReqPak->bRequest;
			int USBFS_SetupReqLen = FSUSBCTX.USBFS_SetupReqLen   = pUSBFS_SetupReqPak->wLength;
			int USBFS_SetupReqIndex = pUSBFS_SetupReqPak->wIndex;
			int USBFS_IndexValue = FSUSBCTX.USBFS_IndexValue = ( pUSBFS_SetupReqPak->wIndex << 16 ) | pUSBFS_SetupReqPak->wValue;
			len = 0;
	USBDEBUG1++;
*/
	int id = req->wValue & 0xff;
	if( id == 0xaa )
	{
		if( req->bRequest == HID_REQ_CONTROL_SET_REPORT )
		{
			//int i;
			//for( i = 0; i < 64; i++ )
			//{
			//	printf( "%02x ", CTRL0BUFF[i] );
			//}
			//printf( "\n" );
		}
		else
		{
		}
	}
	return 0;
}

int HandleHidNonStandardDataOut( struct _USBState * ctx, uint8_t * data, int len )
{
	int i;
	for( i = 0; i < len; i++ )
	{
//		printf( "%02x ", data[i] );
	}
//	printf( "\n" );
	printf( "OUT\n" );
	return 0;
}

void HandleHidNonStandardDataIn( struct _USBState * ctx, uint8_t * data, int len )
{
	memset( data, 0xcc, len );
}


int main()
{
	SystemInit();

	funGpioInitAll();

	funPinMode( PA0, GPIO_CFGLR_OUT_10Mhz_PP );

	FSUSBSetup();

	while(1)
	{
		printf( "%lu %08lx %lu %d %d\n", USBDEBUG0, USBDEBUG1, USBDEBUG2, 0, 0 );
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

