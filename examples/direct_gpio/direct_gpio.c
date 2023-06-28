#include "ch32v003fun.h"
#include <stdio.h>

uint32_t count;

int main()
{
	SystemInit();

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

	// GPIO C1 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*1);
	// GPIO C2 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*2));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*2);
	// GPIO C4 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*4));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	while(1)
	{
		// Use low bits of BSHR to SET output
		GPIOC->BSHR = 1<<(1);      // SET GPIO C1
		GPIOC->BSHR = 1<<(2);      // SET GPIO C2

		// Modify the OUTDR register directly to SET output
		GPIOC->OUTDR |= 1<<(4);    // SET GPIO C4
		Delay_Ms( 950 );


		// Use upper bits of BSHR to RESET output
		GPIOC->BSHR = (1<<(16+1)); // RESET GPIO C1

		// Use BCR to RESET output
		GPIOC->BCR = (1<<(2));     // RESET GPIO C2

		// Modify the OUTDR register directly to CLEAR output
		GPIOC->OUTDR &= ~(1<<(4)); // CLEAR GPIO C4

		Delay_Ms( 50 );
		count++;
	}
}

