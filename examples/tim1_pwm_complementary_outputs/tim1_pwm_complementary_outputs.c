/*
 * Example for using Advanced Control Timer (TIM1) for PWM generation
 * with complementary outputs. Varies deadtime setting repeatedly.
 * 1 December 2024 Ned Konz
 */

#include "ch32fun.h"
#include <stdio.h>

/*
 * AFIO mapping of TIM1
 *
 Timer 1 pin mappings by AFIO->PCFR1
    00	AFIO_PCFR1_TIM1_REMAP_NOREMAP
        (ETR/PC5, BKIN/PC2)
        CH1/CH1N PD2/PD0
        CH2/CH2N PA1/PA2
        CH3/CH3N PC3/PD1
        CH4 PC4
    01	AFIO_PCFR1_TIM1_REMAP_PARTIALREMAP1
        (ETR/PA12, CH1/PA8, CH2/PA9, CH3/PA10, CH4/PA11, BKIN/PA6, CH1N/PA7, CH2N/PB0, CH3N/PB1)
        CH1/CH1N PC6/PC3
        CH2/CH2N PC7/PC4
        CH3/CH3N PC0/PD1
        CH4 PD3
    10	AFIO_PCFR1_TIM1_REMAP_PARTIALREMAP2
        (ETR/PD4, CH1/PD2, CH2/PA1, CH3/PC3, CH4/PC4, BKIN/PC2, CH1N/PD0, CN2N/PA2, CH3N/PD1)
        CH1/CH1N PD2/PD0
        CH2/CH2N PA1/PA2
        CH3/CH3N PC3/PD1
        CH4 PC4
    11	AFIO_PCFR1_TIM1_REMAP_FULLREMAP
        (ETR/PE7, CH1/PE9, CH2/PE11, CH3/PE13, CH4/PE14, BKIN/PE15, CH1N/PE8, CH2N/PE10, CH3N/PE12)
        CH1/CH1N PC4/PC3
        CH2/CH2N PC7/PD2
        CH3/CH3N PC5/PC6
        CH4 PD2
*/

#define TIM1_PERIOD 10000 // 10000/4.8MHz = 2.083ms
#define TIM1_DUTY_CYCLE 5000 // 50% duty cycle

/*
 * initialize TIM1 for PWM
 */
void t1pwm_init( void )
{
	// Enable GPIOC, TIM1, and AFIO clocks
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1 | RCC_APB2Periph_AFIO;

	// Re-map AFIO to allow PC3=TIM1_CH1N and PC6=TIM1_CH1 to be used
	AFIO->PCFR1 &= ~AFIO_PCFR1_TIM1_REMAP_FULLREMAP;
	AFIO->PCFR1 |= AFIO_PCFR1_TIM1_REMAP_PARTIALREMAP1;

	// PC3 is T1CH1_N, 10MHz Output alt func, push-pull
	funPinMode( PC3, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );

	// PC6 is T1CH1, 10MHz Output alt func, push-pull
	funPinMode( PC6, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );

	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;

	// CTLR1: default is up, events generated, edge align
	TIM1->CTLR1 = 0;

	// CTLR2: set output idle states (MOE off) via OIS1 and OIS1N bits
	TIM1->CTLR2 = 0;

	// SMCFGR: default clk input is 48MHz CK_INT

	// Prescaler: divide by 10 => 4.8MHz
	TIM1->PSC = 0x0009;

	// Auto Reload - sets period = 10000/4.8MHz = 2.083ms
	TIM1->ATRLR = TIM1_PERIOD;

	// Reload immediately
	TIM1->SWEVGR |= TIM_UG;

	// Enable CH1 output, positive polarity
	TIM1->CCER |= TIM_CC1E | TIM_CC1P;

	// Enable CH1N output, positive polarity
	TIM1->CCER |= TIM_CC1NE | TIM_CC1NP;

	// CH1 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
	TIM1->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1;

	// Set the Capture Compare Register value to 50% initially
	TIM1->CH1CVR = TIM1_DUTY_CYCLE;

	// Enable TIM1 outputs (also see OSSI and OSSR bits)
	TIM1->BDTR |= TIM_MOE;

	// Enable TIM1
	TIM1->CTLR1 |= TIM_CEN;
}

// low 5 bits are deadtime, upper 3 bits are deadtime prescaler and offset
void t1pwm_set_deadtime( uint16_t deadtime )
{
	TIM1->BDTR = ( TIM1->BDTR & ~TIM_DTG ) | ( deadtime & TIM_DTG );
}

/*
 * entry
 */
int main()
{
	uint32_t count = 0;

	SystemInit();
	Delay_Ms( 100 );

	// init TIM1 for PWM
	t1pwm_init();

	// Repeat changing deadtime every 20ms
	for ( ;; )
	{
		t1pwm_set_deadtime( count );
		count++;
		count &= 255;
		Delay_Ms( 20 );
	}
}
