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

#define ADC_SAMPLE_TIME 2  // Tricky: Don't change this without a lot of experimentation.
#define MAX_SCALECHECK  4
void InitTouchADC( )
{
	// ADCCLK = 24 MHz => RCC_ADCPRE = 0: divide sys clock by 2
	RCC->CFGR0 &= ~(0x1F<<11);

	// Set up single conversion on chl 2
	ADC1->RSQR1 = 0;
	ADC1->RSQR2 = 0;

	// turn on ADC and set rule group to sw trig
	ADC1->CTLR2 |= ADC_ADON | ADC_EXTSEL;
	
	// Reset calibration
	ADC1->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);
	
	// Calibrate
	ADC1->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);
}

// Run from RAM to get even more stable timing.
// This function call takes about 8.1uS to execute.
uint32_t ReadTouchPin( GPIO_TypeDef * io, int portpin, int adcno ) __attribute__((noinline, section(".srodata")));
uint32_t ReadTouchPin( GPIO_TypeDef * io, int portpin, int adcno )
{
	uint32_t ret = 0;

	ADC1->RSQR3 = adcno;
	ADC1->SAMPTR2 = ADC_SAMPLE_TIME<<(3*adcno);

	uint32_t CFGBASE = io->CFGLR & (~(0xf<<(4*portpin)));
	uint32_t CFGFLOAT = ((GPIO_CFGLR_IN_PUPD)<<(4*portpin)) | CFGBASE;
	uint32_t CFGDRIVE = (GPIO_CFGLR_OUT_10Mhz_OD)<<(4*portpin) | CFGBASE;

	int n;

	// If we run multiple times with slightly different wait times, we can
	// reduce the impact of the ADC's DNL.
	for( n = MAX_SCALECHECK; n; n-- )
	{
		// Only lock IRQ for a very narrow window.
		__disable_irq();

		// Tricky - we start the ADC BEFORE we transition the pin.  By doing this
		// We are catching it onthe slope much more effectively.
		ADC1->CTLR2 = ADC_SWSTART | ADC_ADON | ADC_EXTSEL;
		switch( n )
		{
			case 4:
				asm volatile( "c.nop" );
			case 3:
				asm volatile( "c.nop" );
			case 2:
				asm volatile( "c.nop" );
			case 1:
				asm volatile( "c.nop" );
		}
		// Then we allow it to float.  By here, we typically have started sampling.
		io->CFGLR = CFGFLOAT;
		io->OUTDR = 1<<portpin;
		__enable_irq();

		while(!(ADC1->STATR & ADC_EOC));
		io->CFGLR = CFGDRIVE;
		io->OUTDR = 1<<(portpin+16);
		ret += ADC1->RDATAR;
	}

	return ret;
}

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
		uint32_t start = SysTick->CNT;
		uint32_t sum[8] = { 0 };

		int j;

		// Sampling all touch pads should take 3280 cycles, or about 68us.
		// So it is possible to sample all channels at 14kHz
		for( j = 0; j < 1; j++ )
		{
			sum[0] += ReadTouchPin( GPIOA, 2, 0 );
			sum[1] += ReadTouchPin( GPIOA, 1, 1 );
			sum[2] += ReadTouchPin( GPIOC, 4, 2 );
			sum[3] += ReadTouchPin( GPIOD, 2, 3 );
			sum[4] += ReadTouchPin( GPIOD, 3, 4 );
			sum[5] += ReadTouchPin( GPIOD, 5, 5 );
			sum[6] += ReadTouchPin( GPIOD, 6, 6 );
			sum[7] += ReadTouchPin( GPIOD, 4, 7 );
		}
	
		uint32_t end = SysTick->CNT;
	
		printf( "%d %d %d %d %d %d %d %d %d\n", (int)sum[0], (int)sum[1], (int)sum[2],
			(int)sum[3], (int)sum[4], (int)sum[5], (int)sum[6], (int)sum[7], (int)(end-start) );
	}
}

/*
 * MIT License
 * 
 * Copyright (c) 2023 Valve Corporation
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


