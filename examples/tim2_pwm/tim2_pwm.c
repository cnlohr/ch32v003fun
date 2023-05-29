/*
 * Example for using Advanced Control Timer (TIM2) for PWM generation
 * 03-28-2023 E. Brombaugh
 * 05-29-2023 recallmenot adapted from Timer1 to Timer2
 */

/*
Timer 2 pin mappings by AFIO->PCFR1
	00 (default)
		D4		T2CH1ETR
		D3		T2CH2
		C0		T2CH3
		D7		T2CH4
	01
		C5		T2CH1ETR_
		C2		T2CH2_
		D2		T2CH3_
		C1		T2CH4_
	10
		C1		T2CH1ETR_
		D3		T2CH2
		C0		T2CH3
		D7		T2CH4
	11
		C1		T2CH1ETR_
		C7		T2CH2_
		D6		T2CH3_
		D5		T2CH4_
*/

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include <stdio.h>

/*
 * initialize TIM2 for PWM
 */
void t2pwm_init( void )
{
	// Enable GPIOD and TIM2
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	// PD4 is T2CH1, 10MHz Output alt func, push-pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*4);
	
	// PD3 is T2CH2, 10MHz Output alt func, push-pull
	GPIOD->CFGLR &= ~(0xf<<(4*3));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*3);
		
	// Reset TIM2 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;
	
	// SMCFGR: default clk input is CK_INT
	// set TIM2 clock prescaler divider 
	TIM2->PSC = 0x0000;
	// set PWM total cycle width
	TIM2->ATRLR = 255;
	
	// for channel 1 and 2, let CCxS stay 00 (output), set OCxM to 110 (PWM I)
	// enabling preload causes the new pulse width in compare capture register only to come into effect when UG bit in SWEVGR is set (= initiate update) (auto-clears)
	TIM2->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1PE | TIM_OC2M_2 | TIM_OC2M_1 | TIM_OC2PE;

	// CTLR1: default is up, events generated, edge align
	// enable auto-reload of preload
	TIM2->CTLR1 |= TIM_ARPE;

	// Enable CH1 output, positive pol
	TIM2->CCER |= TIM_CC1E | TIM_CC1P;
	// Enable CH2 output, positive pol
	TIM2->CCER |= TIM_CC2E | TIM_CC2P;

	// initialize counter
	TIM2->SWEVGR |= TIM_UG;

	// Enable TIM2
	TIM2->CTLR1 |= TIM_CEN;
}


/*
 * set timer channel PW
 */
void t2pwm_setpw(uint8_t chl, uint16_t width)
{
	switch(chl&3)
	{
		case 0: TIM2->CH1CVR = width; break;
		case 1: TIM2->CH2CVR = width; break;
		case 2: TIM2->CH3CVR = width; break;
		case 3: TIM2->CH4CVR = width; break;
	}
	TIM2->SWEVGR |= TIM_UG; // load new value in compare capture register
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
	Delay_Ms( 100 );
	printf("\r\r\n\ntim2_pwm example\n\r");

	// init TIM2 for PWM
	printf("initializing tim2...");
	t2pwm_init();
	printf("done.\n\r");
		
	printf("looping...\n\r");
	while(1)
	{
		t2pwm_setpw(0, count);			// Chl 1
		t2pwm_setpw(1, (count + 128)&255);	// Chl 2 180° out-of-phase
		count++;
		count &= 255;
		Delay_Ms( 5 );
	}
}
