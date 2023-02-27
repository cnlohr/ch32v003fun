// Really basic self-contained demo for the ch32v003
// Doesn't rely on any of the weird HAL stuff from CH
// Final executable is ~1/4th the size.

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v00x.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

// For debug writing to the UART.
int _write(int fd, char *buf, int size)
{
    for(int i = 0; i < size; i++){
        while( !(USART1->STATR & USART_FLAG_TC));
        USART1->DATAR = *buf++;
    }
    return size;
}

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

	// Once clock is up and running, we can enable the UART for other debugging.

	// Enable GPIOD and UART.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1;

	// Push-Pull, 10MHz Output, GPIO D5, with AutoFunction
	GPIOD->CFGLR &= ~(0xf<<(4*5));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);
	
	// 115200, 8n1.  Note if you don't specify a mode, UART remains off even when UE_Set.
	USART1->CTLR1 = USART_WordLength_8b | USART_Parity_No | USART_Mode_Tx;
	USART1->CTLR2 = USART_StopBits_1;
	USART1->CTLR3 = USART_HardwareFlowControl_None;

	#define UART_BAUD_RATE 115200
	#define OVER8DIV 4
	#define INTEGER_DIVIDER (((25 * (APB_CLOCK)) / (OVER8DIV * (UART_BAUD_RATE))))
	#define FRACTIONAL_DIVIDER ((INTEGER_DIVIDER)%100)
	USART1->BRR = ((INTEGER_DIVIDER / 100) << 4) | ((((FRACTIONAL_DIVIDER * (OVER8DIV*2)) + 50)/100)&7);
	USART1->CTLR1 |= CTLR1_UE_Set;
}

int main()
{
	// Enable GPIOD.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D0 Push-Pull, 10MHz Output
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	while(1)
	{
		GPIOD->BSHR = 1;	 // Turn on GPIOD0
		puts( "Hello" );
		Delay_Ms( 50 );
		GPIOD->BSHR = 1<<16; // Turn off GPIOD0
		Delay_Ms( 50 );
	}
}
