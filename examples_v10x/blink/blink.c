// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 80000000

#include "ch32v003fun.h"
#include <stdio.h>

int main()
{
	SystemInit80HSE();

	// Enable GPIOs
	//RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;

	// GPIO A0 Push-Pull
	GPIOA->CFGLR &= ~(0xf<<(4*0));
	GPIOA->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	while(1)
	{
		GPIOA->BSHR = (1<<0);	 // Turn on GPIO
		Delay_Ms( 1000 );
		GPIOA->BSHR = (1<<(16+0)); // Turn off GPIO
		Delay_Ms( 1000 );
	}
}
