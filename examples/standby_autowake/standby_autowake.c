// based on https://paste.sr.ht/blob/b9b4fb45cbc70f2db7e31a77a6ef7dd2a7f220fb

#include "ch32v003fun.h"
#include <stdio.h>

/* somehow this ISR won't get called??
void AWU_IRQHandler( void ) __attribute__((interrupt));
void AWU_IRQHandler( void ) {
	GPIOD->OUTDR ^= (1 << 4);
}
*/

int main()
{
	SystemInit();

	// This delay gives us some time to reprogram the device. 
	// Otherwise if the device enters standby mode we can't 
	// program it any more.
	Delay_Ms(5000);

	printf("\r\n\r\nlow power example\r\n\r\n");

	// Set all GPIOs to input pull up
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
	GPIOA->CFGLR = 0b10001000100010001000100010001000;
	GPIOA->OUTDR = 0b11111111;
	GPIOC->CFGLR = 0b10001000100010001000100010001000;
	GPIOC->OUTDR = 0b11111111;
	GPIOD->CFGLR = 0b10001000100010001000100010001000;
	GPIOD->OUTDR = 0b11111111;

	// enable power interface module clock
	RCC->APB1PCENR |= RCC_APB1Periph_PWR;

	// enable low speed oscillator (LSI)
	RCC->RSTSCKR |= RCC_LSION;
	while ((RCC->RSTSCKR & RCC_LSIRDY) == 0) {}

	// enable AutoWakeUp event
	EXTI->EVENR |= EXTI_Line9;
	EXTI->FTENR |= EXTI_Line9;

	// configure AWU prescaler
	PWR->AWUPSC |= PWR_AWU_Prescaler_4096;

	// configure AWU window comparison value
	PWR->AWUWR &= ~0x3f;
	PWR->AWUWR |= 63;

	// enable AWU
	PWR->AWUCSR |= (1 << 1);

	// select standby on power-down
	PWR->CTLR |= PWR_CTLR_PDDS;

	// peripheral interrupt controller send to deep sleep
	PFIC->SCTLR |= (1 << 2);

	uint16_t counter = 0;
	printf("entering sleep loop\r\n");

	for (;;) {
		__WFE();
		// restore clock to full speed
		SystemInit();
		printf("\r\nawake, %u\r\n", counter++);
	}
}
