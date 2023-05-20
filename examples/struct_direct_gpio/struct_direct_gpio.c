// This tries to do exactly the same as direct_gpio but with structs

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

    // GPIO D0, D4 Push-Pull, D1/SWIO floating, default analog input
	GPIOC->CFGLR_bits = (const struct CFGLR_t) {
	// GPIO C1 Push-Pull
		.MODE1 = GPIO_CFGLR_MODE_OUT_10MHz,
		.CNF1 = GPIO_CFGLR_CNF_OUT_PP,
	// GPIO C2 Push-Pull
		.MODE2 = GPIO_CFGLR_MODE_OUT_10MHz,
		.CNF2 = GPIO_CFGLR_CNF_OUT_PP,
	// GPIO C4 Push-Pull
		.MODE4 = GPIO_CFGLR_MODE_OUT_10MHz,
		.CNF4 = GPIO_CFGLR_CNF_OUT_PP,
	};

	while(1)
	{
		// Use low bits of BSHR to SET output
		const struct BSHR_t BS1 = {
			.BS1 = 1,
		};
		GPIOC->BSHR_bits = BS1;
		const struct BSHR_t BS2 = {
			.BS2 = 1,
		};
		GPIOC->BSHR_bits = BS2;

		// Modify the OUTDR register directly to SET output
		GPIOC->OUTDR_bits.ODR4 = 1;

		Delay_Ms( 950 );

		// Use upper bits of BSHR to RESET output
		const struct BSHR_t BR1 =  {
			.BR1 = 1,
		};
		GPIOC->BSHR_bits = BR1;

		// Use BCR to RESET output
		const struct BCR_t BR2 = {
			.BR2 = 1,
		};
		GPIOC->BCR_bits = BR2;

		// Modify the OUTDR register directly to CLEAR output
		GPIOC->OUTDR_bits.ODR4 = 0;

		Delay_Ms( 50 );
		count++;
	}
}

