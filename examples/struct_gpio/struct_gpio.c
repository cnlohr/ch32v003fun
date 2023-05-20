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

    // GPIO D0, D4 Push-Pull
	GPIOD->CFGLR_bits = (struct CFGLR_t) {
		.MODE0 = GPIO_CFGLR_MODE_OUT_10MHz,
		.CNF0 = GPIO_CFGLR_CNF_OUT_PP,
		.MODE1 = GPIO_CFGLR_MODE_IN,
		.CNF1 = GPIO_CFGLR_CNF_IN_FLOAT,
		.MODE4 = GPIO_CFGLR_MODE_OUT_10MHz,
		.CNF4 = GPIO_CFGLR_CNF_OUT_PP,
	};

	// GPIO C0 Push-Pull
	GPIOC->CFGLR_bits = (struct CFGLR_t) {
		.MODE0 = GPIO_CFGLR_MODE_OUT_10MHz,
		.CNF0  = GPIO_CFGLR_CNF_OUT_PP,
	};

	while(1)
	{
		// Turn on GPIOs, set in BSHR
		GPIOD->BSHR_bits = (struct BSHR_t) {
			.BS0 = 1,
			.BS4 = 1,
		};
		GPIOC->BSHR_bits.BS0 = 1;
		printf( "+%lu\n", count++ );
		Delay_Ms( 100 );

		// Turn off GPIODs, clear in BSHR
		GPIOD->BSHR_bits = (struct BSHR_t) {
			.BR0 = 1,
			.BR4 = 1,
		};
		// or clear in BCR
		GPIOC->BCR_bits.BR0 = 1;
		printf( "-%lu\n", count++ );
		Delay_Ms( 100 );
	}
}

