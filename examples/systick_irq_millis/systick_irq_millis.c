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

	const uint16_t led_i = 250;
	uint32_t led_t_last;
	uint8_t led_progstep = 0;

	printf("looping...\n\r");
	while(1)
	{
		if (millis() - led_t_last >= led_i) {
			switch (led_progstep) {
				case 0:
					GPIOD->BSHR = 1 | (1<<4);	 // Turn on GPIOs
					break;
				case 1:
					GPIOC->BSHR = 1;
					break;
				case 2:
					GPIOD->BSHR = (1<<16) | (1<<(16+4)); // Turn off GPIODs
					break;
				case 3:
					GPIOC->BSHR = (1<<16);
					printf( "Count: %lu\n\r", count++ );
					break;
			}
			led_progstep++;
			led_t_last = millis();
			if (led_progstep > 3) {
				led_progstep = 0;
			}
		}
	}
}
