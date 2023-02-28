// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v00x.h"
#include <stdio.h>
#include <string.h>

// Working on WS2812 driving.

int main()
{
	SystemInit48HSI();
	SetupUART( UART_BRR );

	int k;

	// Enable GPIOD (for debugging)
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);
	GPIOD->BSHR = 1; // Turn on GPIOD0
	GPIOD->BSHR = 1<<16; // Turn off GPIOD0

	while(1)
	{
		Delay_Ms( 10 );
		uint32_t val = *(uint32_t*)0xe0000000;
		*(uint32_t*)0xe0000000 = 0xaabbccdd;
		printf( "0xe0000000: %08x\n", val);
	}
}

