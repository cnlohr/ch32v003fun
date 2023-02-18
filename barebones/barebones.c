#include "ch32v00x.h"

int main()
{
	// Enable GPIOD.
	RCC->APB2PCENR |= 0x20; //RCC_APB2Periph_GPIOD

	// Push-Pull, 50MHz Output
	GPIOD->CFGLR = (GPIOD->CFGLR & 0xfffffff0) | 3;
	
	while(1)
	{
		GPIOD->BSHR = 1;     // Turn on GPIOD0
		GPIOD->BSHR = 1<<16; // Turn off GPIOD0
	}
}