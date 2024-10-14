#include "ch32v003fun.h"
#include <stdio.h>

int main()
{
	SystemInit();

	funGpioInitAll();

	funPinMode( PA15, GPIO_CFGLR_OUT_10Mhz_PP );

	while(1)
	{
		funDigitalWrite( PA15, FUN_HIGH );	 // Turn on GPIO
		Delay_Ms( 1000 );
		funDigitalWrite( PA15, FUN_LOW );	 // Turn off GPIO
		Delay_Ms( 1000 );
	}
}
