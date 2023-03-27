/*
 * Single-File-Header for using SysTick with millisecond interrupts
 * 03-25-2023 E. Brombaugh
 */

#ifndef _SYSTICK_H
#define _SYSTICK_H

#include <stdint.h>

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

#if 1
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
#endif

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

#endif
