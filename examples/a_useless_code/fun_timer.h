#include "ch32v003fun.h"
#include <stdio.h>

/* some bit definitions for systick regs */
#define SYSTICK_SR_CNTIF (1<<0)
#define SYSTICK_CTLR_STE (1<<0)
#define SYSTICK_CTLR_STIE (1<<1)
#define SYSTICK_CTLR_STCLK (1<<2)
#define SYSTICK_CTLR_STRE (1<<3)
#define SYSTICK_CTLR_SWIE (1<<31)

volatile uint32_t systick_cnt;

/*
 * Start up the SysTick IRQ
 */
void systick_init(void)
{
	/* disable default SysTick behavior */
	SysTick->CTLR = 0;
	
	/* enable the SysTick IRQ */
	NVIC_EnableIRQ(SysTicK_IRQn);
	
	/* Set the tick interval to 1ms for normal op */
	SysTick->CMP = (FUNCONF_SYSTEM_CORE_CLOCK/1000)-1;
	
	/* Start at zero */
	SysTick->CNT = 0;
	systick_cnt = 0;
	
	/* Enable SysTick counter, IRQ, HCLK/1 */
	SysTick->CTLR = SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE |
					SYSTICK_CTLR_STCLK;
}

/*
 * SysTick ISR just counts ticks
 * note - the __attribute__((interrupt)) syntax is crucial!
 */
void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
	// move the compare further ahead in time.
	// as a warning, if more than this length of time
	// passes before triggering, you may miss your
	// interrupt.
	SysTick->CMP += (FUNCONF_SYSTEM_CORE_CLOCK/1000);

	/* clear IRQ */
	SysTick->SR = 0;

	/* update counter */
	systick_cnt++;
}

void timer_setup() {
	// init systick @ 1ms rate
	printf("initializing systick...");
	systick_init();
	printf("done.\n\r");
}

uint32_t timer_getTick() {
	return systick_cnt;
}

uint32_t timer_getCount() {
	return SysTick->CNT;
}


// GPIO pin states
enum {
	OUTPUT = 0,
	INPUT,
	INPUT_PULLUP,
	INPUT_PULLDOWN
};

void pinMode(uint8_t u8Pin, int iMode)
{
	GPIO_TypeDef *pGPIO;

   if (u8Pin < 0xa0 || u8Pin > 0xdf) return; // invalid pin number

	switch (u8Pin & 0xf0) {
	case 0xa0:
RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;
pGPIO = GPIOA;
	break;
	case 0xc0:
		RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
pGPIO = GPIOC;
	break;
	case 0xd0:
RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
pGPIO = GPIOD;
	break;
	}
	u8Pin &= 0xf; // isolate the pin from this port
	pGPIO->CFGLR &= ~(0xf << (4 * u8Pin)); // unset all flags

	switch (iMode) {
	case OUTPUT:
		pGPIO->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4*u8Pin);
		break;
	case INPUT:
		pGPIO->CFGLR |= (GPIO_CNF_IN_FLOATING << (4*u8Pin));
		break;
	case INPUT_PULLUP:
		pGPIO->CFGLR |= (GPIO_CNF_IN_PUPD << (4*u8Pin));
		pGPIO->BSHR = (1 << u8Pin);
		break;
	case INPUT_PULLDOWN:
		pGPIO->CFGLR |= (GPIO_CNF_IN_PUPD << (4 * u8Pin));
		pGPIO->BCR = (1 << u8Pin);
		break;
    } // switch on iMode
} /* pinMode() */

void digitalWrite(uint8_t u8Pin, uint8_t u8Value)
{
	uint32_t u32Value = 1 << (u8Pin & 0xf); // turn on bit
	if (!u8Value)
		u32Value <<= 16; // turn off bit 

	switch (u8Pin & 0xf0) {
	case 0xa0:
		GPIOA->BSHR = u32Value;
		break;
	case 0xc0:
		GPIOC->BSHR = u32Value;
		break;
	case 0xd0:
		GPIOD->BSHR = u32Value;
		break;
	}
} /* digitalWrite() */

uint8_t digitalRead(uint8_t u8Pin)
{
	uint32_t u32GPIO = 1 << (u8Pin & 0xf);
	uint32_t u32Value = 0;
	switch (u8Pin & 0xf0) {
	case 0xa0:
		u32Value = GPIOA->INDR & u32GPIO;
		break;
	case 0xc0:
		u32Value = GPIOC->INDR & u32GPIO;
		break;
	case 0xd0:
		u32Value = GPIOD->INDR & u32GPIO;
		break;
	}
	return (u32Value != 0);
} /* digitalRead() */