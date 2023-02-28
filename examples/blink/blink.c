// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v00x.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

int main()
{
	SystemInit48HSI();

	// Enable GPIOD.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D0 Push-Pull, 10MHz Slew Rate Setting
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	while(1)
	{
		GPIOD->BSHR = 1;	 // Turn on GPIOD0
		Delay_Ms( 100 );
		GPIOD->BSHR = 1<<16; // Turn off GPIOD0
		Delay_Ms( 100 );
	}
}
