/*
 * Example for using SysTick with IRQs
 * 03-25-2023 E. Brombaugh
 */

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

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
	
	/* Clear any existing IRQ */
    SysTick->SR &= ~SYSTICK_SR_CNTIF;
	
	/* Set the tick interval to 1ms for normal op */
    SysTick->CMP = (SYSTEM_CORE_CLOCK/1000)-1;
	
	/* Start at zero */
    SysTick->CNT = 0;
	systick_cnt = 0;
	
	/* Enable SysTick counter, IRQ, HCLK/1, auto reload */
    SysTick->CTLR = SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE | 
					SYSTICK_CTLR_STCLK | SYSTICK_CTLR_STRE;
}

/*
 * SysTick ISR just counts ticks
 * note - the __attribute__((interrupt)) syntax is crucial!
 */
void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
	/* clear IRQ */
    SysTick->SR &= 0;
	
	/* update counter */
	systick_cnt++;
}

/*
 * Millisecond delay routine
 */
void systick_delay_ms(uint32_t milliseconds)
{
	/* compute end time */
	uint32_t etime = systick_cnt + milliseconds;
	
	/* wait for current time == end time */
	while(systick_cnt != etime);
}

/*
 * entry
 */
int main()
{
	uint32_t count = 0;
	
	SystemInit48HSI();

	// start serial @ default 115200bps
	SetupUART( UART_BRR );
	printf("\r\r\n\nsystick_irq example\n\r");

	// init systick @ 1ms rate
	printf("initializing systick...");
	systick_init();
	printf("done.\n\r");
	
	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	// GPIO D0 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	// GPIO D4 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	// GPIO C0 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	printf("looping...\n\r");
	while(1)
	{
		GPIOD->BSHR = 1 | (1<<4);	 // Turn on GPIOs
		systick_delay_ms( 250 );
		GPIOC->BSHR = 1;
		systick_delay_ms( 250 );
		GPIOD->BSHR = (1<<16) | (1<<(16+4)); // Turn off GPIODs
		systick_delay_ms( 250 );
		GPIOC->BSHR = (1<<16);
		systick_delay_ms( 250 );
		printf( "Count: %lu\n\r", count++ );
	}
}
