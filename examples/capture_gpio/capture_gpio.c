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

    // GPIO D0, D4 Push-Pull LEDs, D1/SWIO floating, D6 Capture Input, default analog input
	DYN_GPIO_WRITE(GPIOD, CFGLR, (GPIO_CFGLR_t) { // (GPIO_CFGLR_t) is optional but helps vscode with completion
		.PIN0 = GPIO_CFGLR_OUT_10Mhz_PP,
		.PIN1 = GPIO_CFGLR_IN_FLOAT,
		.PIN4 = GPIO_CFGLR_OUT_10Mhz_PP,
		.PIN6 = GPIO_CFGLR_IN_FLOAT,
	});

	while(1)
	{
	}
}

