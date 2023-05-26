/* Small example showing how to capture timer values on gpio edges */

#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

#define queuelen 16
volatile uint32_t captureVals[queuelen];
volatile size_t read = 0;
volatile size_t write = 0;

void TIM1_CC_IRQHandler(void) __attribute__((interrupt));
void TIM1_CC_IRQHandler(void)
{
	// capture
	if(TIM1->INTFR & TIM_CC1IF)
	{
		// get capture
		captureVals[write++] = 0x00010000 | TIM1->CH1CVR; // capture valur
		if (write == queuelen)
		{
			write = 0;
		}
		// overflow
		if(TIM1->INTFR & TIM_CC1OF)
		{
			// clear
			TIM1->INTFR = ~(TIM_CC1OF); // cleard by writing 0
			printf("OF1\n");
		}
	}
	else if (TIM1->INTFR & TIM_CC2IF)
	{
		// get capture
		captureVals[write++] = TIM1->CH2CVR; // capture valur
		if (write == queuelen)
		{
			write = 0;
		}
		// overflow
		if(TIM1->INTFR & TIM_CC2OF)
		{
			// clear
			TIM1->INTFR = ~(TIM_CC2OF); // cleard by writing 0
			printf("OF0\n");
		}
	}
	else
	{
		printf("badtrigger\n");
	}
}

int main()
{
	SystemInit48HSI();
	SetupDebugPrintf();

	Delay_Ms(100);

	printf("UP\n");

	// Enable peripherals
	RCC->APB2PCENR = RCC_IOPDEN | RCC_TIM1EN;
	RCC->APB2PRSTR = RCC_TIM1RST;
	RCC->APB2PRSTR = 0;

	// GPIO D0, D4 Push-Pull LEDs, D1/SWIO floating, D2 Capture Input(T1CH1), default analog input
	GPIOD->CFGLR = ((GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 0)) | 
				   ((GPIO_SPEED_IN | GPIO_CNF_IN_FLOATING) << (4 * 1)) | 
				   ((GPIO_SPEED_IN | GPIO_CNF_IN_FLOATING) << (4 * 2)) | 
				   ((GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 4));

	TIM1->ATRLR = 0xffff;
	TIM1->PSC = 47; // 48MHz/(47+1) -> 1µs resolution

	TIM1->CHCTLR1 = TIM_CC1S_0 | TIM_CC2S_1;

	TIM1->CCER = TIM_CC1E|TIM_CC2E|TIM_CC2P;

	TIM1->CTLR1 = TIM_CEN;

	NVIC_EnableIRQ(TIM1_CC_IRQn);
	TIM1->DMAINTENR = TIM_CC1IE|TIM_CC2IE;

	while (1)
	{
		if (read != write)
		{
			uint32_t val = captureVals[read++];
			printf("capture %ld %lu\n", val >> 16, val & 0xFFFF);
			if (read == queuelen)
			{
				read = 0;
			}
		}

		// Turn D0 on and D4 off at the same time
		GPIOD->BSHR = 1 | 1<<(16+4);
		Delay_Ms(1);

		// Turn D0 off and D4 on at the same time
		GPIOD->BSHR = 1<<16 | 1<<4;
		Delay_Ms(1);
	}
}