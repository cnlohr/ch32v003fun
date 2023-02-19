// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v00x.h"
#include <stdio.h>
#include <string.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

// Working on WS2812 driving.

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


uint16_t bufferset[128];

void ConfigSpeedyLED()
{
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1;

	// MOSI
	GPIOC->CFGLR &= ~(0xf<<(4*6));
	GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF)<<(4*6);

	SPI1->CTLR1 = 
		SPI_NSS_Hard | SPI_CPHA_1Edge | SPI_CPOL_Low | SPI_DataSize_16b |
		SPI_Mode_Master | SPI_NSS_Soft | SPI_Direction_1Line_Tx |
		3<<3; // Dvisior

	SPI1->CTLR2 = SPI_CTLR2_TXDMAEN;
	SPI1->HSCR = 1;

	SPI1->CTLR1 |= CTLR1_SPE_Set;

	memset( bufferset, 0xaa, sizeof( bufferset ) );

	//DMA1_Channel3 is for SPI1TX
	DMA1_Channel3->PADDR = (uint32_t)&SPI1->DATAR;
	DMA1_Channel3->MADDR = (uint32_t)bufferset;
	DMA1_Channel3->CNTR  = sizeof( bufferset )/2; // Number of unique copies.
	DMA1_Channel3->CFGR  =
		DMA_M2M_Disable |		 
		DMA_Priority_VeryHigh |
		DMA_MemoryDataSize_HalfWord |
		DMA_PeripheralDataSize_HalfWord |
		DMA_MemoryInc_Enable |
		DMA_Mode_Normal | // OR DMA_Mode_Circular
		DMA_DIR_PeripheralDST;

	DMA1_Channel3->CFGR |= DMA_CFGR1_EN;
}

int main()
{
	// PLL + Prescale down ADC
//	RCC->CFGR0 = RCC_SW_PLL | RCC_SWS_PLL | RCC_ADCPRE_DIV8;
//	RCC->CTLR = RCC_PLLON | RCC_HSION | 0xff00;
//	RCC->CTLR = RCC_PLLON | RCC_HSION | 0xff00;

	// wow, it seems we
	RCC->CTLR = RCC_PLLON | RCC_HSION | 0xf8;

	// Enable GPIOD.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D0 Push-Pull, 10MHz Output
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	// Testing with reset
	GPIOD->CFGLR &= ~(0xf<<(4*7));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*7);

	// Testing with PD6
	GPIOD->CFGLR &= ~(0xf<<(4*6));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*6);

	ConfigSpeedyLED();

	printf( "CFG: %08x\n", SPI1->CTLR1 );

	while(1)
	{

		GPIOD->BSHR = 128+64+1;
		GPIOD->BSHR = 1<<16;
		GPIOD->BSHR = 1;
		GPIOD->BSHR = 1<<16;
		GPIOD->BSHR = 1;
		Delay_Ms( 1 );
		GPIOD->BSHR = (128+64+1)<<16;
		Delay_Ms( 1 );
	}
}
