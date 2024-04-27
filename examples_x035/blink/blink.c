#include "ch32v003fun.h"
#include <stdio.h>

int main()
{
	SystemInit();

	funGpioInitAll();

	funPinMode( PA0, GPIO_CFGLR_OUT_10Mhz_PP );

	while(1)
	{
		funDigitalWrite( PA0, FUN_HIGH );	 // Turn on GPIO
		//Delay_Ms( 1000 );
		funDigitalWrite( PA0, FUN_LOW );	 // Turn off GPIO
		//Delay_Ms( 1000 );
		funDigitalWrite( PA0, FUN_HIGH );	 // Turn on GPIO
		asm volatile( "c.nop" );
		funDigitalWrite( PA0, FUN_LOW );	 // Turn off GPIO
	}
}
