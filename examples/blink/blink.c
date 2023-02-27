// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v00x.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

void SystemInit(void)
{
	// Values lifted from the EVT.  There is little to no documentation on what this does.
	RCC->CTLR |= (uint32_t)0x00000001;
	RCC->CFGR0 &= (uint32_t)0xFCFF0000;
	RCC->CTLR &= (uint32_t)0xFEF6FFFF;
	RCC->CTLR &= (uint32_t)0xFFFBFFFF;
	RCC->CFGR0 &= (uint32_t)0xFFFEFFFF;
	RCC->INTR = 0x009F0000;

	// From SetSysClockTo_48MHZ_HSI
	// This is some dark stuff.  But, I copy-pasted it and it seems towork.
	FLASH->ACTLR = (FLASH->ACTLR & ((uint32_t)~FLASH_ACTLR_LATENCY)) | FLASH_ACTLR_LATENCY_1; 	// Flash 0 wait state
	RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1; 														// HCLK = SYSCLK = APB1
	RCC->CFGR0 = ( RCC->CFGR0 & ((uint32_t)~(RCC_PLLSRC)) ) | (uint32_t)(RCC_PLLSRC_HSI_Mul2); 	// PLL configuration: PLLCLK = HSI * 2 = 48 MHz
	RCC->CTLR |= RCC_PLLON; 																	// Enable PLL
	while((RCC->CTLR & RCC_PLLRDY) == 0);														// Wait till PLL is ready
	RCC->CFGR0 = ( RCC->CFGR0 & ((uint32_t)~(RCC_SW))) | (uint32_t)RCC_SW_PLL;					// Select PLL as system clock source
	while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08);									// Wait till PLL is used as system clock source
}

int main()
{
	// Enable GPIOD.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D0 Push-Pull, 10MHz Slew Rate Setting
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	while(1)
	{
		GPIOD->BSHR = 1;	 // Turn on GPIOD0
		Delay_Ms( 100 );
		GPIOD->BSHR = 1<<16; // Turn off GPIOD0
		Delay_Ms( 100 );
	}
}
