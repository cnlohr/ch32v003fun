/* Small example showing how to use the SWIO programming pin to 
   do printf through the debug interface */

#include "ch32v003fun.h"
#include <stdio.h>

uint32_t count;

int last = 0;
void handle_debug_input( int numbytes, uint8_t * data )
{
	last = data[0];
	count += numbytes;
}

int main()
{
	SystemInit();

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	// GPIO D0 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	// GPIO C0 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	while(1)
	{
		GPIOD->BSHR = 1;	 // Turn on GPIOs
		GPIOC->BSHR = 1;
		printf( "+%lu\n", count++ );
		Delay_Ms(100);
		int i;
		for( i = 0; i < 10000; i++ )
			poll_input();
		GPIOD->BSHR = (1<<16); // Turn off GPIODs
		GPIOC->BSHR = (1<<16);
		printf( "-%lu[%c]\n", count++, last );
		Delay_Ms(100);
	}
}

