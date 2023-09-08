// based on https://paste.sr.ht/blob/b9b4fb45cbc70f2db7e31a77a6ef7dd2a7f220fb

#include "ch32v003fun.h"
#include <stdio.h>

void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) {
	//GPIOD->OUTDR ^= (1 << 4);
}



int main()
{
	SystemInit();

	// This delay gives us some time to reprogram the device. 
	// Otherwise if the device enters standby mode we can't 
	// program it any more.
	Delay_Ms(5000);

	printf("\n\nlow power example\n\n");

	// Set all GPIOs to input pull up
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
	GPIOA->CFGLR = 0b10001000100010001000100010001000;
	GPIOA->OUTDR = 0b11111111;
	GPIOC->CFGLR = 0b10001000100010001000100010001000;
	GPIOC->OUTDR = 0b11111111;
	GPIOD->CFGLR = 0b10001000100010001000100010001000;
	GPIOD->OUTDR = 0b00000100;

	// AFIO is needed for EXTI
	RCC->APB2PCENR |= RCC_AFIOEN;

	// assign pin 2 interrupt from portD (0b11) to EXTI channel 2
	AFIO->EXTICR |= (uint32_t)(0b11 << (2 * 2));

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
