/*
	Copyright (c) 2023 Valve Corporation (see copyright notice at bottom)

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

// Can either be 0 or 1.
// If 0: Measurement low and rises high.  So more pressed is smaller number.
// If 1: Higher number = harder press. Good to pair with TOUCH_FLAT.
// If you are doing more prox, use mode 0, otherwise, use mode 1.
#define TOUCH_SLOPE     1

// If you set this to 1, it will glitch the line, so it will only read
// anything reasonable if the capacitance can overcome that initial spike.
// Typically, it seems if you use this you probbly don't need to do
// any pre-use calibration.
#define TOUCH_FLAT      0

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
uint32_t ReadTouchPin( GPIO_TypeDef * io, int portpin, int adcno, int iterations ) __attribute__((noinline, section(".srodata")));
uint32_t ReadTouchPin( GPIO_TypeDef * io, int portpin, int adcno, int iterations )
{
	uint32_t ret = 0;

	ADC1->RSQR3 = adcno;
	ADC1->SAMPTR2 = ADC_SAMPLE_TIME<<(3*adcno);

	uint32_t CFGBASE = io->CFGLR & (~(0xf<<(4*portpin)));
	uint32_t CFGFLOAT = ((GPIO_CFGLR_IN_PUPD)<<(4*portpin)) | CFGBASE;
	uint32_t CFGDRIVE = (GPIO_CFGLR_OUT_2Mhz_PP)<<(4*portpin) | CFGBASE;

	// If we run multiple times with slightly different wait times, we can
	// reduce the impact of the ADC's DNL.


#if TOUCH_FLAT == 1
#define RELEASEIO io->OUTDR = 1<<(portpin+16*TOUCH_SLOPE); io->CFGLR = CFGFLOAT;
#else
#define RELEASEIO io->CFGLR = CFGFLOAT; io->OUTDR = 1<<(portpin+16*TOUCH_SLOPE);
#endif

#define INNER_LOOP( n ) \
	{ \
		/* Only lock IRQ for a very narrow window. */                           \
		__disable_irq();                                                        \
                                                                                \
		/* Tricky - we start the ADC BEFORE we transition the pin.  By doing    \
			this We are catching it onthe slope much more effectively.  */      \
		ADC1->CTLR2 = ADC_SWSTART | ADC_ADON | ADC_EXTSEL;                      \
                                                                                \
		ADD_N_NOPS( n )                                                         \
                                                                                \
		RELEASEIO                                                               \
																			    \
		/* Sampling actually starts here, somewhere, so we can let other        \
			interrupts run */                                                   \
		__enable_irq();                                                         \
		while(!(ADC1->STATR & ADC_EOC));                                        \
		io->CFGLR = CFGDRIVE;                                                   \
		io->OUTDR = 1<<(portpin+(16*(1-TOUCH_SLOPE)));                          \
		ret += ADC1->RDATAR;                                                    \
	}

	int i;
	for( i = 0; i < iterations; i++ )
	{
		// Wait a variable amount of time based on loop iteration, in order
		// to get a variety of RC points and minimize DNL.

		INNER_LOOP( 0 );
		INNER_LOOP( 2 );
		INNER_LOOP( 4 );
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
		uint32_t sum[8] = { 0 };

		int j;

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


