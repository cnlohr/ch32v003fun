#include "ch32v003fun.h"
#include <stdio.h>

uint32_t count;

int main()
{
	SystemInit();

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	// GPIO D0 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	// GPIO D4 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	// GPIO C0 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	while(1)
	{
		GPIOD->BSHR = 1 | (1<<4);	 // Turn on GPIOs
		GPIOC->BSHR = 1;
		Delay_Ms( 250 );
		GPIOD->BSHR = (1<<16) | (1<<(16+4)); // Turn off GPIODs
		GPIOC->BSHR = (1<<16);
		Delay_Ms( 250 );
		count++;
	}
}
