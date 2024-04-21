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

int main()
{
	SystemInit();

	funGpioInitAll();

	funPinMode( PA0, GPIO_CFGLR_OUT_10Mhz_PP );

	FSUSBSetup();


void USBFS_IRQHandler();
//extern int USBFS_IRQHandler;
	while(1)
	{
		printf( "%lu %08x %lu %d %d\n", USBDEBUG0, USBDEBUG1, USBDEBUG2, 0, 0 );
		int i;
		//printf( "!! %d %d %d\n", NUM_EP, USBFS_Endp_Busy[0], USBFS_Endp_Busy[1] );
		for( i = 1; i < 3; i++ )
		{
			if( !FSUSBCTX.USBFS_Endp_Busy[i] )
			{

/*
USBDEBUG0+= 100;
                /* tx length */
				//memset( USBFSD_UEP_BUF( i ), 0xaa, 16 );
//                USBFSD_UEP_TLEN( i ) = 16;
  //              USBFSD_UEP_TX_CTRL( i ) = ( USBFSD_UEP_TX_CTRL( i ) & ~USBFS_UEP_T_RES_MASK ) | USBFS_UEP_T_RES_ACK;
	//			USBFS_Endp_Busy[i] = 1;

				char pbuf[16] = { 0xaa, 1, 1 };
/*				static iter;
				if( i == 2 )
				{
					iter++;
					switch( (iter>>3) & 3 )
					{
					case 0: pbuf[1] = 1; break;
					case 1: pbuf[2] = 1; break;
					case 2: pbuf[1] = -1; break;
					case 3: pbuf[2] = -1; break;
					}
				}*/
				uint8_t r = USBFS_Endp_DataUp( i, pbuf, (i==1)?8:4, DEF_UEP_CPY_LOAD);
//USBDEBUG1+=r;
			}
		}
	}

#if 0
	while(1)
	{
		GPIOA->BSHR = 1;	 // Turn on GPIOs
		printf( "+%lu %lu\n", count++ );
		Delay_Ms(100);
		int i;
		for( i = 0; i < 10000; i++ )
			poll_input();
		GPIOA->BSHR = (1<<16); // Turn off GPIODs
		printf( "-%lu[%c]\n", count++, last );
		Delay_Ms(100);
	}
#endif
}

