/*
	ADC-aided capacitive touch sensing.

	This demonstrates use of the ADC with cap sense pads. By using the ADC, you
	can get much higher resolution than if you use the exti.  But, you are
	limited by the number of ADC lines, i.e. you can only support a maximum of
	eight (8) touch inputs with this method.

	In exchange for that, typically you can get > 1,000 LSBs per 1 CM^2 and it
	can convert faster.  Carefully written code can be as little as 2us per
	conversion!

	The mechanism of operation for the touch sensing on the CH32V003 is to:
		* Hold an IO low.
		* Start the ADC
		* Use the internal pull-up to pull the line high.
		* The ADC will sample the voltage on the slope.
		* Lower voltage = longer RC respone, so higher capacitance. 
*/

#include "ch32v003fun.h"
#include <stdio.h>

#include "ch32v003_touch.h"

int main()
{
	SystemInit();

	printf("Capacitive Touch ADC example\n");
	
	// Enable GPIOD, C and ADC
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1;

	InitTouchADC();
	
	// should be ready for SW conversion now
	while(1)
	{
		uint32_t sum[8] = { 0 };

		uint32_t start = SysTick->CNT;

		// Sampling all touch pads, 3x should take 6030 cycles, and runs at about 8kHz

		int iterations = 3;
		sum[0] += ReadTouchPin( GPIOA, 2, 0, iterations );
		sum[1] += ReadTouchPin( GPIOA, 1, 1, iterations );
		sum[2] += ReadTouchPin( GPIOC, 4, 2, iterations );
		sum[3] += ReadTouchPin( GPIOD, 2, 3, iterations );
		sum[4] += ReadTouchPin( GPIOD, 3, 4, iterations );
		sum[5] += ReadTouchPin( GPIOD, 5, 5, iterations );
		sum[6] += ReadTouchPin( GPIOD, 6, 6, iterations );
		sum[7] += ReadTouchPin( GPIOD, 4, 7, iterations );

		uint32_t end = SysTick->CNT;
	
		printf( "%d %d %d %d %d %d %d %d %d\n", (int)sum[0], (int)sum[1], (int)sum[2],
			(int)sum[3], (int)sum[4], (int)sum[5], (int)sum[6], (int)sum[7], (int)(end-start) );
	}
}

