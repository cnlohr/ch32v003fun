/*
 * Example for using General Purpose Timer (TIM3) for PWM generation
 * Shows the usage of PWM mode 1 and 2
 *
 * If needed the mapping can be changed by using the AFIO pheripheral.
 *
 * Timer 3 pin mappings by AFIO->PCFR1
 *    00: Default mapping (CH1/PA6, CH2/PA7)
 *    01: Mapping (CH1/PB4, CH2/PB5)
 *    10: Mapping (CH1/PC19, CH2/PC18)
 *    11: Mapping (CH1/PA3, CH2/PA4)
 */

#include "ch32fun.h"
#include <stdio.h>

// mask for the CCxP bits
// when set PWM outputs are held HIGH by default and pulled LOW
// when zero PWM outputs are held LOW by default and pulled HIGH
#define TIM3_DEFAULT 0xff
// #define TIM3_DEFAULT 0x00

/*
 * initialize TIM3 for PWM
 */
void t3pwm_init( void )
{
	// Enable GPIOA and TIM3
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM3;

	// PA6 is T3CH1, 10MHz Output alt func, push-pull
	GPIOA->CFGLR &= ~( 0xf << ( 4 * 6 ) );
	GPIOA->CFGLR |= ( GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF ) << ( 4 * 6 );

	// PA7 is T3CH2, 10MHz Output alt func, push-pull
	GPIOA->CFGLR &= ~( 0xf << ( 4 * 7 ) );
	GPIOA->CFGLR |= ( GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF ) << ( 4 * 7 );

	// Reset TIM3 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_TIM3;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM3;

	// set TIM3 clock prescaler divider
	TIM3->PSC = 0x0000;
	// set PWM total cycle width
	TIM3->ATRLR = 255;

	// for channel 1 and 2, let CCxS stay 00 (output), set OC1M to 110 (PWM 1), set OC2M to 111 (PWM 2)
	// by using PWM 2 for the second channel we basically introduce a 180 degrees phase shift.
	// enabling preload causes the new pulse width in compare capture register only to come into effect when UG bit in
	// SWEVGR is set (= initiate update) (auto-clears)
	TIM3->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1PE | TIM_OC2M_2 | TIM_OC2M_1 | TIM_OC2M_0 | TIM_OC2PE;

	// Enable Channel outputs, set default state (based on TIM3_DEFAULT)
	TIM3->CCER |= TIM_CC1E | ( TIM_CC1P & TIM3_DEFAULT );
	TIM3->CCER |= TIM_CC2E | ( TIM_CC2P & TIM3_DEFAULT );

	// CTLR1: default is up, events generated, edge align
	// enable auto-reload of preload
	TIM3->CTLR1 |= TIM_ARPE;

	// initialize counter
	TIM3->SWEVGR |= TIM_UG;
	// Enable TIM3
	TIM3->CTLR1 |= TIM_CEN;
}

/*
 * set timer channel PW
 */
void t3pwm_setpw( uint8_t chl, uint16_t width )
{
	switch ( chl & 1 )
	{
		case 0: TIM3->CH1CVR = width; break;
		case 1: TIM3->CH2CVR = width; break;
	}
}

/*
 *  Generate a triangle wave
 */
uint8_t triangle_wave( uint16_t input )
{
	input = input & 0x1FF;
	return input ^ ( ( input >> 8 ) * 0xFF );
}

/*
 * entry
 */
int main()
{
	size_t count = 0;

	SystemInit();
	Delay_Ms( 100 );

	printf( "\r\r\n\ntim3_pwm example\n\r" );

	// init TIM3 for PWM
	printf( "initializing tim3..." );
	t3pwm_init();
	printf( "done.\n\r" );

	printf( "looping...\n\r" );

	while ( 1 )
	{
		t3pwm_setpw( 0, triangle_wave( count ) ); // CH1
		t3pwm_setpw( 1, triangle_wave( count ) ); // CH2 (180° out-of-phase)

		count++;

		Delay_Ms( 5 );
	}
}
