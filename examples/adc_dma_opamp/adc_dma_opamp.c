/*
 * Example for using ADC with DMA and an op-amp
 * 04-13-2023 E. Brombaugh
 */

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>
#include "adc.h"
#include "opamp.h"

int main()
{
	SystemInit48HSI();

	// start serial @ default 115200bps
	SetupUART( UART_BRR );
	Delay_Ms(100);
	printf("\r\r\n\nadc_dma_opamp example\n\r");

	// init adc
	printf("initializing adc...");
	adc_init();
	printf("done.\n\r");
	
#if 0
	printf("MADDR = 0x%08X, Buffer = 0x%08X\n\r",
		DMA1_Channel1->CNTR, adc_buffer);
	while(1)
	{
		//printf("STATR = 0x%08X, RDATAR = 0x%04X, CNTR = 0x%04X\n\r",
			//ADC1->STATR, ADC1->RDATAR, DMA1_Channel1->CNTR);
	}
#endif

	// init op-amp
	printf("initializing op-amp...\n\r");
	opamp_init();
	printf("done.\n\r");
	
	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

	// GPIO C1 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*1);

	printf("looping...\n\r");
	while(1)
	{
		GPIOC->BSHR = (1<<1);	 // Turn on GPIOs
		Delay_Ms( 100 );
		GPIOC->BSHR = (1<<1+16); // Turn off GPIODs
		Delay_Ms( 100 );
		printf( "%4d %4d ", adc_buffer[0], adc_buffer[1]);
		printf( "%4d %4d\n\r", adc_buffer[2], adc_buffer[3]);
	}
}
