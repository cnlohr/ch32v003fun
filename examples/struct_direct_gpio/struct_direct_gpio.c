// This tries to generates exacly the same assambly as direct_gpio but using structs as much as possible

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

uint32_t count;

int main()
{
	SystemInit48HSI();

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

	// this code modifies two fields at the same time

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
		GPIOset(GPIOC, GPIO_Pin_1);
		GPIOset(GPIOC, GPIO_Pin_2);

		// Modify the OUTDR register directly to SET output
		OUTDR_t tmp = GPIO_OUTDR_get(GPIOC);
		tmp.ODR4 = 1;
		GPIO_OUTDR_set(GPIOC, tmp);
		Delay_Ms( 950 );

		// Use upper bits of BSHR to RESET output
		GPIOsetReset(GPIOC, 0, GPIO_Pin_1);

		// Use BCR to RESET output
		GPIOset(GPIOC, GPIO_Pin_2);

		// Modify the OUTDR register directly to CLEAR output
		tmp = GPIO_OUTDR_get(GPIOC);
		tmp.ODR4 = 0;
		GPIO_OUTDR_set(GPIOC, tmp);

		Delay_Ms( 50 );
		count++;
	}
}

