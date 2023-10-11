// based on https://paste.sr.ht/blob/b9b4fb45cbc70f2db7e31a77a6ef7dd2a7f220fb

#include "ch32v003fun.h"
#include <stdio.h>


int main()
{
	SystemInit();
	
	// This delay gives us some time to reprogram the device. 
	// Otherwise if the device enters standby mode we can't 
	// program it any more.
	Delay_Ms(5000);

	printf("\n\nlow power example\n\n");
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
	// GPIOA: Set to output
	GPIOA->CFGLR = ((GPIO_CNF_OUT_PP | GPIO_Speed_2MHz)<<(4*2)) |
				   ((GPIO_CNF_OUT_PP | GPIO_Speed_2MHz)<<(4*1));
	GPIOA->BSHR = GPIO_BSHR_BS2 | GPIO_BSHR_BR1;
	// GPIOC: Set to input with mixed pull-up / pull-down
	GPIOC->CFGLR = (GPIO_CNF_IN_PUPD<<(4*7)) |
				   (GPIO_CNF_IN_PUPD<<(4*6)) |
				   (GPIO_CNF_IN_PUPD<<(4*5)) |
				   (GPIO_CNF_IN_PUPD<<(4*4)) |
				   (GPIO_CNF_IN_PUPD<<(4*3)) |
				   (GPIO_CNF_IN_PUPD<<(4*2)) |
				   (GPIO_CNF_IN_PUPD<<(4*1)) |
				   (GPIO_CNF_IN_PUPD<<(4*0));
	GPIOC->BSHR = GPIO_BSHR_BS7 |
				  GPIO_BSHR_BR6 |
				  GPIO_BSHR_BS5 |
				  GPIO_BSHR_BR4 |
				  GPIO_BSHR_BS3 |
				  GPIO_BSHR_BR2 |
				  GPIO_BSHR_BS1 |
				  GPIO_BSHR_BR0;
	// GPIOD: D2 set to input pull-up
	GPIOD->CFGLR = (GPIO_CNF_IN_PUPD<<(4*7)) |
				   (GPIO_CNF_IN_PUPD<<(4*6)) |
				   (GPIO_CNF_IN_PUPD<<(4*5)) |
				   (GPIO_CNF_IN_PUPD<<(4*4)) |
				   (GPIO_CNF_IN_PUPD<<(4*3)) |
				   (GPIO_CNF_IN_PUPD<<(4*2)) |
				   (GPIO_CNF_IN_PUPD<<(4*0));
	GPIOD->BSHR = GPIO_BSHR_BR7 |
				  GPIO_BSHR_BS6 |
				  GPIO_BSHR_BR5 |
				  GPIO_BSHR_BS4 |
				  GPIO_BSHR_BR3 |
				  GPIO_BSHR_BS2 |
				  GPIO_BSHR_BR0;

	// AFIO is needed for EXTI
	RCC->APB2PCENR |= RCC_AFIOEN;

	// assign pin 2 interrupt from portD (0b11) to EXTI channel 2
	AFIO->EXTICR |= (uint32_t)(0b11 << (2*2));

	// enable line2 interrupt event
	EXTI->EVENR |= EXTI_Line2;
	EXTI->FTENR |= EXTI_Line2;

	// select standby on power-down
	PWR->CTLR |= PWR_CTLR_PDDS;

	// peripheral interrupt controller send to deep sleep
	PFIC->SCTLR |= (1 << 2);

	uint16_t counter = 0;
	printf("entering sleep loop\n");

	for (;;) {
		__WFE();
		// restore clock to full speed
		SystemInit();
		printf("\nawake, %u\n", counter++);
		Delay_Ms(5000);	// wake and reflash can happen here
	}
}
