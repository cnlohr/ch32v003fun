// Really basic self-contained demo for the ch32v003
// Doesn't rely on any of the weird HAL stuff from CH
// Final executable is ~1/4th the size.

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

uint32_t count;

int main()
{
	SystemInit48HSI();
	SetupDebugPrintf();

	// Enable GPIOD.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D0, D4 Push-Pull, 10MHz Output
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	while(1)
	{
		GPIOD->BSHR = 1 | (1<<4);	 // Turn on GPIOD0
		//Delay_Ms( 50 );
		GPIOD->BSHR = (1<<16) | ( 1<<(4+16); // Turn off GPIOD0
		//Delay_Ms( 50 );
		printf( "%d\n", count++ );
	}
}
