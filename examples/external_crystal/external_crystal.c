// XXX XXX XXX XXX XXX THIS IS UNTESTED XXX XXX XXX XXX XXX

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 24000000

#include "ch32v003fun.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

uint32_t count;

int main()
{
	SystemInitHSE( 0 );

	// Enable GPIOD.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D0 Push-Pull, 10MHz Output
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	// GPIO D0 Push-Pull, 10MHz Output
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	while(1)
	{
		GPIOD->BSHR = 1 | (1<<4);	 // Turn on GPIOD0
		Delay_Ms( 200 );
		GPIOD->BSHR = (1<<16) | (1<<(16+4)); // Turn off GPIOD0
		Delay_Ms( 200 );
		count++;
	}
}
