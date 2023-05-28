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
		DYN_GPIO_WRITE(GPIOC, BSHR, (GPIO_BSHR_t) {.BS1 = 1});
		DYN_GPIO_WRITE(GPIOC, BSHR, (GPIO_BSHR_t) {.BS2 = 1});

		// Modify the OUTDR register directly to SET output
		DYN_GPIO_MOD(GPIOC, OUTDR, ODR4, 1);
		Delay_Ms( 950 );

		// Use upper bits of BSHR to RESET output
		DYN_GPIO_WRITE(GPIOC, BSHR, (GPIO_BSHR_t) {.BR1 = 1});

		// Use BCR to RESET output
		DYN_GPIO_WRITE(GPIOC, BCR, (GPIO_BCR_t) {.BR2 = 1});

		// Modify the OUTDR register directly to CLEAR output
		DYN_GPIO_MOD(GPIOC, OUTDR, ODR4, 0);

		Delay_Ms( 50 );
		count++;
	}
}

