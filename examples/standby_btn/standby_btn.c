// based on https://paste.sr.ht/blob/b9b4fb45cbc70f2db7e31a77a6ef7dd2a7f220fb
// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) {
	//GPIOD->OUTDR ^= (1 << 4);
}



int main()
{
	SystemInit48HSI();
	SetupUART( UART_BRR );

	printf("\r\n\r\nlow power example\r\n\r\n");

	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	// GPIO D4 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);
	GPIOD->OUTDR |= (1 << 4);

	// give the user time to open the terminal connection
	//Delay_Ms(5000);
	//printf("5000ms wait over\r\n");
	
	// enable alternate IO function module clock
	RCC->APB2PCENR |= RCC_AFIOEN;

	// configure button on PD2 as input, pullup
	GPIOD->CFGLR &= ~(0xf<<(2*4));
	GPIOD->CFGLR |= (GPIO_CNF_IN_PUPD)<<(2*4);
	GPIOD->BSHR = (1 << 2);

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
	printf("entering sleep loop\r\n");

	for (;;) {
		__WFE();
		// restore clock to full speed
		SystemInit48HSI();
		printf("\r\nawake, %u\r\n", counter++);
		GPIOD->OUTDR ^= (1 << 4);
	}
}
