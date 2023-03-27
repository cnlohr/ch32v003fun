/*
 * Example for using SysTick with IRQs
 * 03-25-2023 E. Brombaugh
 */

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>
#include "systick.h"

int main()
{
	uint32_t count = 0;
	
	SystemInit48HSI();

	// start serial @ default 115200bps
	SetupUART( UART_BRR );
	printf("\r\r\n\nsystick_irq example\n\r");

	printf("SysTick_Handler = 0x%08X\n\r", SysTick_Handler);

	// init systick @ 1ms rate
	printf("initializing systick...");
	systick_init();
	printf("done.\n\r");
	
#if 0
	// Debugging - does ISR run & inc counter?
	// this section shows that IRQs during print will reset system, but during
	// inf while() do not.
	
	// does cnt inc?
	uint32_t pcnt = systick_cnt;
	while(1)
	{
		if(pcnt != systick_cnt)
		{
			printf("CNT = 0x%08X ", SysTick->CNT);
			printf("systick_cnt = %d\n\r", systick_cnt);
			pcnt = systick_cnt;
		}
	}
#else
	// Testing 1ms Systick
	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	// GPIO D0 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	// GPIO D4 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	// GPIO C0 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	//printf("looping...\n\r");
	while(1)
	{
		GPIOD->BSHR = 1 | (1<<4);	 // Turn on GPIOs
		GPIOC->BSHR = 1;
		systick_delay_ms( 250 );
		GPIOD->BSHR = (1<<16) | (1<<(16+4)); // Turn off GPIODs
		GPIOC->BSHR = (1<<16);
		systick_delay_ms( 250 );
		printf( "Count: %lu\n\r", count++ );
	}
#endif
}
