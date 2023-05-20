/* Small example showing how to use the structs for controling GPIO pins */

#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

uint32_t count;

int main()
{
	SystemInit48HSI();
	SetupDebugPrintf();

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	Delay_Ms( 100 );

	// all floating inputs before
	printf("CFGLR: %lX\n", GPIOD->CFGLR); // -> CFGLR: 44444444

    // GPIO D0, D4 Push-Pull, D1/SWIO floating, default analog input
	GPIOD->CFGLR_bits = (struct CFGLR_t) {
		.MODE0 = GPIO_CFGLR_MODE_OUT_10MHz,
		.CNF0 = GPIO_CFGLR_CNF_OUT_PP,
		.MODE1 = GPIO_CFGLR_MODE_IN,
		.CNF1 = GPIO_CFGLR_CNF_IN_FLOAT,
		.MODE4 = GPIO_CFGLR_MODE_OUT_10MHz,
		.CNF4 = GPIO_CFGLR_CNF_OUT_PP,
	};

	// all unconfigured pins are not 0b0000, aka analog inputs with TTL Schmitttrigger disabled
	printf("CFGLR: %lX\n", GPIOD->CFGLR); // -> CFGLR: 10041

	// GPIO C0 Push-Pull with 2 volatile writes
	GPIOC->CFGLR_bits.MODE0 = GPIO_CFGLR_MODE_OUT_10MHz;
	GPIOC->CFGLR_bits.CNF0  = GPIO_CFGLR_CNF_OUT_PP;

	// read modify write
	struct CFGLR_t ioc = GPIOC->CFGLR_bits;
	ioc.MODE1 = GPIO_CFGLR_MODE_IN; // not volatile
	ioc.CNF1 = GPIO_CNF_IN_ANALOG;  // not volatile
	GPIOC->CFGLR_bits = ioc; // this should be one volatile write

	while(1)
	{
		// Turn D0 on and D4 off at the same time
		GPIOD->BSHR_bits = (struct BSHR_t) {
			.BS0 = 1,
			.BR4 = 1,
		};
		GPIOC->BSHR_bits.BS0 = 1;
		Delay_Ms( 100 );

		// Turn D0 off and D4 on at the same time
		GPIOD->BSHR_bits = (struct BSHR_t) {
			.BR0 = 1,
			.BS4 = 1,
		};
		// or clear in BCR
		GPIOC->BCR_bits.BR0 = 1;
		Delay_Ms( 100 );
	}
}

