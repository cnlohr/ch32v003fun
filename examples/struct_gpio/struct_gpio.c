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
	DYN_GPIO_WRITE(GPIOD, CFGLR, (GPIO_CFGLR_t) { // (GPIO_CFGLR_t) is optional but helps vscode with completion
		.PIN0 = GPIO_CFGLR_OUT_10Mhz_PP,
		.PIN1 = GPIO_CFGLR_IN_FLOAT,
		.PIN4 = GPIO_CFGLR_OUT_10Mhz_PP,
	});

	// all unconfigured pins are now 0b0000, aka analog inputs with TTL Schmitttrigger disabled
	printf("CFGLR: %lX\n", GPIOD->CFGLR); // -> CFGLR: 10041

	// GPIO C0 Push-Pull
	// read modify write
	GPIO_CFGLR_t ioc = DYN_GPIO_READ(GPIOC, CFGLR);
	ioc.PIN0 = GPIO_CFGLR_OUT_10Mhz_PP,
	ioc.PIN1 = GPIO_CFGLR_IN_ANALOG;
	DYN_GPIO_WRITE(GPIOC, CFGLR, ioc);

	while(1)
	{
		// Turn D0 on and D4 off at the same time
		DYN_GPIO_WRITE(GPIOD, BSHR, (GPIO_BSHR_t) {.BS0 = 1, .BR4 = 1});

		// implicit read->modify->write
		DYN_GPIO_MOD(GPIOC, OUTDR, ODR0, 1);
		Delay_Ms( 100 );

		// Turn D0 off and D4 on at the same time
		DYN_GPIO_WRITE(GPIOD, BSHR, (GPIO_BSHR_t) {.BR0 = 1, .BS4 = 1});

		// implicit read->modify->write
		DYN_GPIO_MOD(GPIOC, OUTDR, ODR0, 0);
		Delay_Ms( 100 );
	}
}

