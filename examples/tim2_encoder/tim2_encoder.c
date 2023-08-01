/*
 * Example for Timer 2 in "encoder" mode
 * 07-01-2023 frosty adapted example from tim2_pwm_remap
 *
 * Usage: 
 * Connect A/B pins of encoder to PC2/PC5
 * Spin the encoder and see the count increase/decrease on the debug monitor
 *
 * Points of interest:
 * 1. Port D pins and C0,C4, and C7 have a diode that clamps the pin to Vcc
 * 	and are not 5V tolerant when Vcc is 3V3
 * 2. Port C pins 1,2,3,5, and 6 are not clamped to Vcc and are should be
 * 	5V tolerant.
 * 2. Encoder mode uses CH1/CH2 so Timer2 must be used with a PARTIALREMAP1 so
 * 	CH1/CH2 pins are on 5V tolerant pins.
 * 3. If you don't require 5V tolerance you can use any of the REMAP options
 *
 * /


Timer 2 pin mappings by AFIO->PCFR1
	00	AFIO_PCFR1_TIM2_REMAP_NOREMAP
		D4		T2CH1ETR
		D3		T2CH2
		C0		T2CH3
		D7		T2CH4  --note: requires disabling nRST in opt
	01	AFIO_PCFR1_TIM2_REMAP_PARTIALREMAP1
		C5		T2CH1ETR_
		C2		T2CH2_
		D2		T2CH3_
		C1		T2CH4_
	10	AFIO_PCFR1_TIM2_REMAP_PARTIALREMAP2
		C1		T2CH1ETR_
		D3		T2CH2
		C0		T2CH3
		D7		T2CH4  --note: requires disabling nRST in opt
	11	AFIO_PCFR1_TIM2_REMAP_FULLREMAP
		C1		T2CH1ETR_
		C7		T2CH2_
		D6		T2CH3_
		D5		T2CH4_
*/

#include "ch32v003fun.h"
#include <stdio.h>

/******************************************************************************************
 * initialize TIM2 for PWM
 ******************************************************************************************/
void t2encoder_init( void )
{
	// Enable GPIOC, TIM2, and AFIO *very important!*
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	AFIO->PCFR1 |= AFIO_PCFR1_TIM2_REMAP_PARTIALREMAP1; //set partial remap mode 1

	// PC2 is T2CH1_, Input w/ Pullup/down
	GPIOC->CFGLR &= ~(0xf<<(4*2)); //clear old values
	GPIOC->CFGLR |= (GPIO_CNF_IN_PUPD)<<(4*2); //set new ones
	//1 = pull-up, 0 = pull-down
	GPIOC->OUTDR |= 1<<2;

	// PC5 is T2CH2_, Input w/ Pullup/down
	GPIOC->CFGLR &= ~(0xf<<(4*5)); //clear values
	GPIOC->CFGLR |= (GPIO_CNF_IN_PUPD)<<(4*5); //set new ones
	//1 = pull-up, 0 = pull-down
	GPIOC->OUTDR |= 1<<5;
	
	// Reset TIM2 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;
	
	// set TIM2 clock prescaler If you want to reduce the resolution of the encoder
	//TIM2->PSC = 0x0000;

	// set a automatic reload if you want the counter to wrap earlier than 0xffff
	//TIM2->ATRLR = 0xffff;

	// SMCFGR: set encoder mode SMS=011b
	TIM2->SMCFGR |= TIM_EncoderMode_TI12;

	// initialize timer
	TIM2->SWEVGR |= TIM_UG;

	// set count to about mid-scale to avoid wrap-around
	TIM2->CNT = 0x8fff;

	// Enable TIM2
	TIM2->CTLR1 |= TIM_CEN;
};

/*****************************************************************************************
 * entry
 *****************************************************************************************/
int main()
{
	SystemInit();


	Delay_Ms( 100 );
	t2encoder_init();

	uint16_t initial_count = TIM2->CNT;
	uint16_t last_count = TIM2->CNT;
	while(1)
	{
		uint16_t count = TIM2->CNT;
		if( count != last_count) {
			printf("Position relative=%ld absolute=%u delta=%ld\n",(int32_t)count - initial_count, count, (int32_t)count-last_count);
			last_count = count;
		}
		Delay_Ms(50);
	}
}
