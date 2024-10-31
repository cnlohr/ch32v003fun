#include "ch32v003fun.h"
#include <stdio.h>

// use defines to make more meaningful names for our GPIO pins
#define PIN_1 PD0
#define PIN_K PD4
#define PIN_BOB PD6
#define PIN_KEVIN PC0

int main()
{
	SystemInit();

	// Enable GPIOs
	funGpioInitAll();
	
	funPinMode( PIN_1,     GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );
	funPinMode( PIN_K,     GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );
	funPinMode( PIN_BOB,   GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );
	funPinMode( PIN_KEVIN, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP );

	while(1)
	{
		funDigitalWrite( PIN_1,     FUN_HIGH );
		funDigitalWrite( PIN_K,     FUN_HIGH );
		funDigitalWrite( PIN_BOB,   FUN_HIGH );
		funDigitalWrite( PIN_KEVIN, FUN_HIGH );
		Delay_Ms( 250 );
		funDigitalWrite( PIN_1,     FUN_LOW );
		funDigitalWrite( PIN_K,     FUN_LOW );
		funDigitalWrite( PIN_BOB,   FUN_LOW );
		funDigitalWrite( PIN_KEVIN, FUN_LOW );
		Delay_Ms( 250 );
	}
}
