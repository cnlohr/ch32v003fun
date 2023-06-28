/*
 * Example for using AFIO to remap peripheral outputs to alternate configuration
 * 06-01-2023 B. Roy, based on previous work by:
 * 03-28-2023 E. Brombaugh
 * 05-29-2023 recallmenot adapted from Timer1 to Timer2
 *
 * Usage: 
 * Connect LEDs between PD3 and GND, PD4 and GND, PC1 and GND, and PC7 and GND
 * Observe activity on PD3 and PD4, then activity on PC1 and PC7, and back
 *
 * Nutshell:
 * 1. Ensure you're providing a clock to the AFIO peripheral! Save yourself an 
 * 	hour of troubleshooting!
 *	RCC->APB2PCENR |= RCC_APB2Periph_AFIO
 * 2. Apply the remapping configuration bits to the AFIO register:
 * 	AFIO->PCFR1 |= AFIO_PCFR1_TIM2_REMAP_FULLREMAP
 * 3. Go on about your business.
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
void t2pwm_init( void )
{
	// Enable GPIOC, GPIOD, TIM2, and AFIO *very important!*
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	// PD4 is T2CH1, 10MHz Output alt func, push-pull (also works in oepn drain OD_AF)
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*4);

	// PD3 is T2CH2, 10MHz Output alt func, push-pull (also works in oepn drain OD_AF)
	GPIOD->CFGLR &= ~(0xf<<(4*3));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*3);

	// PC1 is T2CH1_, 10MHz Output alt func, push-pull (also works in oepn drain OD_AF)
	GPIOC->CFGLR &= ~(0xf<<(4*1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*1);

	// PC7 is T2CH2_, 10MHz Output alt func, push-pull (also works in oepn drain OD_AF)
	GPIOC->CFGLR &= ~(0xf<<(4*7));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*7);
	
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
	TIM2->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1PE;
	TIM2->CHCTLR1 |= TIM_OC2M_2 | TIM_OC2M_1 | TIM_OC2PE;

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

/*****************************************************************************************
 * set timer channel PW
 *****************************************************************************************/
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

/*****************************************************************************************
 * Remap T1CH1/T1CH2 from PD4/PD3 to PC1/PC7
 *
 * Can remap on-the fly; no need to re-initialize timers, reset GPIO mode/config, etc.
 *
 * Leaves the previous pins configured as 'alternate function' mode - i.e. disconnected
 * from the GPIO peripheral, and floating.
 *
 *****************************************************************************************/
void ToggleRemap(void) {
	if(AFIO->PCFR1 & AFIO_PCFR1_TIM2_REMAP_FULLREMAP) {
		AFIO->PCFR1 &= AFIO_PCFR1_TIM2_REMAP_NOREMAP;   //clear remapping bits
		printf("Standard Mapping!\r\n");
	}
	else {
		AFIO->PCFR1 |= AFIO_PCFR1_TIM2_REMAP_FULLREMAP; //set fullremap mode
		printf("Full Remapping!\r\n");
	}
};

/*****************************************************************************************
 * entry
 *****************************************************************************************/
int main()
{
	uint32_t count = 0;
	
	SystemInit();
	Delay_Ms( 100 );
	printf("\r\r\n\ntim2_pwm example, with remap\n\r");

	// init TIM2 for PWM
	printf("initializing tim2...");
	t2pwm_init();
	printf("done.\n\r");
		
	printf("looping...\n\r");
	
	while(1)
	{
		for(;count<255;count++){
			t2pwm_setpw(0, count);		// Chl 1
			t2pwm_setpw(1, 255-count);	// Chl 2 180° out-of-phase
			Delay_Ms( 5 );
		}
		count = 0;
		ToggleRemap();
	}
}
