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
	GPIO_CFGLR_set(GPIOD, (CFGLR_t) {
		.PIN0 = GPIO_CFGLR_OUT_10Mhz_PP,
		.PIN1 = GPIO_CFGLR_IN_FLOAT,
		.PIN4 = GPIO_CFGLR_OUT_10Mhz_PP,
	});

	// all unconfigured pins are now 0b0000, aka analog inputs with TTL Schmitttrigger disabled
	printf("CFGLR: %lX\n", GPIOD->CFGLR); // -> CFGLR: 10041

	// GPIO C0 Push-Pull with 2 volatile writes
	GPIO_CFGLR_set(GPIOC, (CFGLR_t) {
		.PIN0 = GPIO_CFGLR_OUT_10Mhz_PP,
	});

	// read modify write
	CFGLR_t ioc = GPIO_CFGLR_get(GPIOC);
	ioc.PIN1 = GPIO_CFGLR_IN_ANALOG;
	GPIO_CFGLR_set(GPIOC, ioc);

	while(1)
	{
		// Turn D0 on and D4 off at the same time
		GPIOsetReset(GPIOD, GPIO_Pin_0, GPIO_Pin_4);
		// could be GPIO_BSHR_set(GPIOD, (BSHR){.BS1=1, .BR4=1});

		// implicit read->modify->write
		// GPIOC->OUTDR_bits.ODR0 = 1; // no idea how to do this now
		Delay_Ms( 100 );

		// Turn D0 off and D4 on at the same time
		GPIOsetReset(GPIOD, GPIO_Pin_4, GPIO_Pin_1);
		// could be GPIO_BSHR_set(GPIOD, (BSHR){.BR1=1, .BS4=1});

		// implicit read->modify->write
		//GPIOC->OUTDR_bits.ODR0 = 0; // no idea how to do this now
		Delay_Ms( 100 );

		// to only set/reset use
		GPIOset(GPIOD, GPIO_Pin_0);
		Delay_Ms( 100 );
		GPIOreset(GPIOD, GPIO_Pin_0);
		Delay_Ms( 100 );
	}
}

