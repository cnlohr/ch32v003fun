/*
 * Example for using General Purpose Timer (TIM2) for PWM generation
 * 03-28-2023 E. Brombaugh
 * 05-29-2023 recallmenot adapted from Timer1 to Timer2
 * 07-29-2023 frosty implemented CH3 and CH4
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

#include "ch32v003fun.h"
#include <stdio.h>

//By default this example only enables CH1(PD4) and CH2(PD3)
//Uncomment this line to enable CH3 (PC0)
//#define TIM2_EN_CH3

//CH4 uses PD7 which is also the NRST pin
//To use CH4 you must disable the NRST feature
//by using `minichlink -d` and uncomment this line.
#define TIM2_EN_CH4

// mask for the CCxP bits
// when set PWM outputs are held HIGH by default and pulled LOW
// when zero PWM outputs are held LOW by default and pulled HIGH
#define TIM2_DEFAULT 0xff
//#define TIM2_DEFAULT 0x00

/*
 * initialize TIM2 for PWM
 */
void t2pwm_init( void )
{
	// Enable GPIOD and TIM2
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	#ifdef TIM2_EN_CH3
	// If using T2CH3 must also enable GPIOC
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
	#endif

	// PD4 is T2CH1, 10MHz Output alt func, push-pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*4);
	
	// PD3 is T2CH2, 10MHz Output alt func, push-pull
	GPIOD->CFGLR &= ~(0xf<<(4*3));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*3);

	#ifdef TIM2_EN_CH3
	// PC0 is T2CH3, 10MHz Output alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*0);
	#endif


	#ifdef TIM2_EN_CH4
	// PD7 is T2CH4, 10MHz Output alt func, push-pull
	// PD7 is also the NRST (Reset) pin and you must use minichlink
	// or the WCH utility to disable the NRST function so the pin
	// can be used as a GPIO.
	GPIOD->CFGLR &= ~(0xf<<(4*7));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*7);
	#endif

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

	#ifdef TIM2_EN_CH3
	TIM2->CHCTLR2 |= TIM_OC3M_2 | TIM_OC3M_1 | TIM_OC3PE;
	#endif
	#ifdef TIM2_EN_CH4
	TIM2->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1 | TIM_OC4PE;
	#endif
	// CTLR1: default is up, events generated, edge align
	// enable auto-reload of preload
	TIM2->CTLR1 |= TIM_ARPE;

	// Enable Channel outputs, set default state (based on TIM2_DEFAULT)
	TIM2->CCER |= TIM_CC1E | (TIM_CC1P & TIM2_DEFAULT);
	TIM2->CCER |= TIM_CC2E | (TIM_CC2P & TIM2_DEFAULT);

	#ifdef TIM2_EN_CH3
	TIM2->CCER |= TIM_CC3E | (TIM_CC3P & TIM2_DEFAULT);
	#endif
	#ifdef TIM2_EN_CH4
	TIM2->CCER |= TIM_CC4E | (TIM_CC4P & TIM2_DEFAULT);
	#endif
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
		#ifdef TIM2_EN_CH3
		case 2: TIM2->CH3CVR = width; break;
		#endif
		#ifdef TIM2_EN_CH4
		case 3: TIM2->CH4CVR = width; break;
		#endif
	}
	//TIM2->SWEVGR |= TIM_UG; // load new value in compare capture register
}



/*
 * entry
 */
int main()
{
	uint32_t count = 0;
	
	SystemInit();
	Delay_Ms( 100 );

	printf("\r\r\n\ntim2_pwm example\n\r");

	// init TIM2 for PWM
	printf("initializing tim2...");
	t2pwm_init();
	printf("done.\n\r");
		
	printf("looping...\n\r");
	while(1)
	{
		t2pwm_setpw(0, count);			// CH1
		t2pwm_setpw(1, (count + 128)&255);	// CH2 180° out-of-phase
		#ifdef TIM2_EN_CH3
		t2pwm_setpw(2, count);			// CH3
		#endif
		#ifdef TIM2_EN_CH4
		t2pwm_setpw(3, (count + 128)&255);	// CH4 180° out-of-phase
		#endif
		count++;
		count &= 255;
		Delay_Ms( 5 );
	}
}
