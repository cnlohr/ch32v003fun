/* Small example showing how to use structs for controling GPIO pins */

#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

uint32_t count;

int main()
{
	SystemInit48HSI();
	/*DYN_RCC_WRITE(CFGR0, (RCC_CFGR0_t) {.SW = 0, .HPRE = 0, .ADCPRE = 0, .PLLSRC = 0, .MCO = 0});
	DYN_RCC_WRITE(CTLR, (RCC_CTLR_t) {.HSION = 1, .HSITRIM = HSITRIM_DEFAULT, .HSEON = 0, .HSEBYP = 0, .CSSON = 0, .PLLON = 1});
	FLASH->ACTLR = FLASH_ACTLR_LATENCY_1;                  // >= 24MHz -> 1 Cycle Latency
	DYN_RCC_WRITE(INTR, (RCC_INTR_t){.HSIRDYIE=0,.HSERDYIE=0,.PLLRDYIE=0,.LSIRDYC=1,.HSIRDYC=1,.HSERDYC=1,.CSSC=1});
	// Wait till PLL is ready
	while(DYN_RCC_READ(CTLR).PLLRDY == 0){}
	// Select PLL as system clock source
	DYN_RCC_MOD(CFGR0, SW, RCC_SW_PLL);
	// Wait till PLL is used as system clock source
	while(DYN_RCC_READ(CFGR0).SWS != 2){}
	*/

	SetupDebugPrintf();

	Delay_Ms(100);

	printf("UP\n");

	// Enable peripherals
	DYN_RCC_WRITE(APB2PCENR, (RCC_APB2PCENR_t){.AFIOEN = 0, .IOPDEN = 1, .TIM1EN = 1}); // default disabled
	DYN_RCC_WRITE(APB1PCENR, (RCC_APB1PCENR_t){.TIM2EN = 0});

	DYN_RCC_WRITE(APB2PRSTR, (RCC_APB2PRSTR_t){.TIM1RST = 1});
	DYN_RCC_WRITE(APB2PRSTR, (RCC_APB2PRSTR_t){.TIM1RST = 0});

	// GPIO D0, D4 Push-Pull LEDs, D1/SWIO floating, D6 Capture Input, default analog input
	DYN_GPIO_WRITE(GPIOD, CFGLR, (GPIO_CFGLR_t){
									 // (GPIO_CFGLR_t) is optional but helps vscode with completion
									 .PIN0 = GPIO_CFGLR_OUT_10Mhz_PP,
									 .PIN1 = GPIO_CFGLR_IN_FLOAT,
									 .PIN4 = GPIO_CFGLR_OUT_10Mhz_PP,
									 .PIN6 = GPIO_CFGLR_IN_FLOAT,
								 });

	TIM1->ATRLR = 0xffff;

	DYN_TIM_WRITE(TIM1, CHCTLR1, (TIM_CHCTLR1_t){.chctlr1_input_bits = {
													 .IC1F = 0000, .IC2PSC = 0,
													 .CC1S = 0b01, // input from PD2(T1CH1)
												 }});
	DYN_TIM_WRITE(TIM1, CCER, (TIM_CCER_t){
								  .CC1E = 1,
								  .CC1P = 0, // CCxP polarity, 0 -> rising edge
							  });
	DYN_TIM_WRITE(TIM1, CTLR1, (TIM_CTLR1_t){
								   .CEN = 1,
							   });

	// capture
	DYN_TIM_READ(TIM1, INTFR).CC1IF;
	// clear
	// read ctr?

	// overflow
	DYN_TIM_READ(TIM1, INTFR).CC1OF;
	// clear
	DYN_TIM_WRITE(TIM1, INTFR, (TIM_INTFR_t){.CC1OF = 1});

	while (1)
	{
		// get capture
		printf("%lu, %lu\n", TIM1->CNT, TIM1->CH1CVR); // count, capture valur

		// Turn D0 on and D4 off at the same time
		DYN_GPIO_WRITE(GPIOD, BSHR, (GPIO_BSHR_t){.BS0 = 1, .BR4 = 1});
		//Delay_Ms(100);

		// Turn D0 off and D4 on at the same time
		DYN_GPIO_WRITE(GPIOD, BSHR, (GPIO_BSHR_t){.BR0 = 1, .BS4 = 1});
		//Delay_Ms(100);
	}
}
