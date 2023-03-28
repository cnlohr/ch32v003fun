/*
 * Example for using ADC with polling
 * 03-27-2023 E. Brombaugh
 */

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>
#include "adc.h"

int main()
{
	uint32_t count = 0;
	
	SystemInit48HSI();

	// start serial @ default 115200bps
	SetupUART( UART_BRR );
	printf("\r\r\n\nadc_polled example\n\r");

	// init systick @ 1ms rate
	printf("initializing adc...");
	adc_init();
	printf("done.\n\r");
	
	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D0 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	printf("looping...\n\r");
	while(1)
	{
		GPIOD->BSHR = 1;	 // Turn on GPIOs
		Delay_Ms( 100 );
		GPIOD->BSHR = (1<<16); // Turn off GPIODs
		Delay_Ms( 100 );
		printf( "Count: %lu adc: %d\n\r", count++, adc_get() );
	}
}
