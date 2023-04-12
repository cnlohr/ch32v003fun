/*
 * Example for SPI with circular DMA for audio output
 * 04-10-2023 E. Brombaugh
 */

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>
#include "spidac.h"

int main()
{
	SystemInit48HSI();

	// start serial @ default 115200bps
	SetupUART( UART_BRR );
	Delay_Ms( 100 );
	printf("\r\r\n\nspi_dac example\n\r");

	// GPIO C0 Push-Pull for foreground blink
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);
	
	// init SPI DAC
	printf("initializing spi dac...");
	spidac_init();
	printf("done.\n\r");
		
	printf("looping...\n\r");
	while(1)
	{
		GPIOC->BSHR = 1;
		Delay_Ms( 250 );
		GPIOC->BSHR = (1<<16);
		Delay_Ms( 250 );
	}
}
