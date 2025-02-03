/* Small example showing how to use the SWIO programming pin to 
   do printf through the debug interface */

#include "ch32fun.h"
#include <stdio.h>

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

	while(1)
	{
		GPIOA->BSHR = 1;	 // Turn on GPIOs
		printf( "+%lu[%c]\n", count++, last );
		Delay_Ms(100);
		int i;
		for( i = 0; i < 10000; i++ )
			poll_input();
		GPIOA->BSHR = (1<<16); // Turn off GPIODs
		printf( "-%lu[%c]\n", count++, last );
		Delay_Ms(100);
	}
}

