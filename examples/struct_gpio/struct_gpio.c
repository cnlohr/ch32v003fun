/* Small example showing how to use structs for controling GPIO pins */

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

	// all unconfigured pins are now 0b0000, aka analog inputs with TTL Schmitttrigger disabled
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
		// GCC sometimes wants to be dumb and turns this into: write zero, read back, or your value, write
		// this is more consistent
		GPIOsetReset(GPIOD, GPIO_Pin_1, GPIO_Pin_4);

		// implicit read->modify->write
		GPIOC->OUTDR_bits.ODR0 = 1;
		Delay_Ms( 100 );

		// Turn D0 off and D4 on at the same time
		GPIOD->BSHR_bits = (struct BSHR_t) {
			.BR0 = 1,
			.BS4 = 1,
		};

		// implicit read->modify->write
		GPIOC->OUTDR_bits.ODR0 = 0;
		Delay_Ms( 100 );

		// to only set/reset use
		GPIOset(GPIOD, GPIO_Pin_0);
		Delay_Ms( 100 );
		GPIOreset(GPIOD, GPIO_Pin_0);
		Delay_Ms( 100 );
	}
}

