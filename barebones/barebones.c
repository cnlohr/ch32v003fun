#include "ch32v00x.h"

void SystemInit(void)
{
	RCC->CTLR |= (uint32_t)0x00000001;
	RCC->CFGR0 &= (uint32_t)0xFCFF0000;
	RCC->CTLR &= (uint32_t)0xFEF6FFFF;
	RCC->CTLR &= (uint32_t)0xFFFBFFFF;
	RCC->CFGR0 &= (uint32_t)0xFFFEFFFF;
	RCC->INTR = 0x009F0000;

	// From SetSysClockTo_48MHZ_HSI
	FLASH->ACTLR = (FLASH->ACTLR & ((uint32_t)~FLASH_ACTLR_LATENCY)) | FLASH_ACTLR_LATENCY_1; 	// Flash 0 wait state
	RCC->CFGR0 |= (uint32_t)RCC_HPRE_DIV1; 														// HCLK = SYSCLK = APB1
	RCC->CFGR0 = ( RCC->CFGR0 & ((uint32_t)~(RCC_PLLSRC)) ) | (uint32_t)(RCC_PLLSRC_HSI_Mul2); 	// PLL configuration: PLLCLK = HSI * 2 = 48 MHz
	RCC->CTLR |= RCC_PLLON; 																	// Enable PLL
	while((RCC->CTLR & RCC_PLLRDY) == 0);														// Wait till PLL is ready
	RCC->CFGR0 = ( RCC->CFGR0 & ((uint32_t)~(RCC_SW))) | (uint32_t)RCC_SW_PLL;					// Select PLL as system clock source
	while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08);									// Wait till PLL is used as system clock source
}

static const uint8_t array[384] = { 0xff };

int main()
{
	// Enable GPIOD.
	RCC->APB2PCENR |= 0x20; //RCC_APB2Periph_GPIOD

	// Push-Pull, 50MHz Output
	GPIOD->CFGLR = (GPIOD->CFGLR & 0xfffffff0) | 3;
	
	
	GPIOD->BSHR = array[GPIOD->BSHR];
	
	while(1)
	{
		GPIOD->BSHR = 1;	 // Turn on GPIOD0
		GPIOD->BSHR = 1<<16; // Turn off GPIOD0
	}
}