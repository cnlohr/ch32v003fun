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


		//DCSR
		asm volatile("\n\
			li t0, 0x4\n\
			csrw 0x7B0, t0\n\
		");

	while(1)
	{
		Delay_Ms( 2 );
	    *(uint32_t*)(0xe0000100) = 2;  //Hopefully enable debug (dmcontrol .0) --> Doesn't work.

		uint32_t val = *(uint32_t*)0xe00000f4;
		*(uint32_t*)0xe00000f4 = 0xaabbccdd;

		printf( "0xe00000f4: %08x %08x\n", val, __get_dscratch0() );

		// Write to dscratch0
		asm volatile("\n\
			li t0, 0xa8b8c8d8\n\
			csrw 0x7B2, t0\n\
			csrw 0x7B3, t0\n\
		");
	}
}

