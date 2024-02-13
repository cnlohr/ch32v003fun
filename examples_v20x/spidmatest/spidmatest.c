#include "ch32v003fun.h"
#include <stdio.h>
#include <string.h>


#define SENDBUFF_WORDS 256
uint16_t sendbuff[SENDBUFF_WORDS];

// Bits are shifted out MSBit first, then to LSBit


void DMA1_Channel5_IRQHandler( void ) __attribute__((interrupt));
void DMA1_Channel5_IRQHandler( void ) 
{
	//GPIOD->BSHR = 1;	 // Turn on GPIOD0 for profiling

	// Backup flags.
	volatile int intfr = DMA1->INTFR;
	do
	{
		// Clear all possible flags.
		DMA1->INTFCR = DMA1_IT_GL5;


		uint16_t * sb = 0;
		if( intfr & DMA1_IT_HT5 )
		{
			sb = sendbuff;
		}
		else if( intfr & DMA1_IT_TC5 )
		{
			sb = sendbuff + SENDBUFF_WORDS/2;
		}

		if( sb )
		{
			//memset( sb, 0xf0, SENDBUFF_WORDS/2*2 );
			int i;
			for( i = 0; i < SENDBUFF_WORDS/2; i++ )
			{
				sb[i] = 0x0AAF;
			}
		}

		// Can abort send with  DMA1_Channel3->CFGR &= ~DMA_Mode_Circular;
		intfr = DMA1->INTFR;
	} while( intfr );

	//GPIOD->BSHR = 1<<16; // Turn off GPIOD0 for profiling
}


int main()
{
	SystemInit();

	funGpioInitAll();

	// Force HCLK to be nodiv. ch32vfun sets it to be a reasonbly high div.
	RCC->CFGR0 &= ~RCC_PPRE1_DIV16;

// MCO for testing.
//	funPinMode( PA8, GPIO_CFGLR_OUT_50Mhz_AF_PP );	RCC->CFGR0 |= RCC_CFGR0_MCO_PLL;

	funPinMode( PB12, GPIO_CFGLR_OUT_50Mhz_AF_PP );
	funPinMode( PB13, GPIO_CFGLR_OUT_50Mhz_AF_PP );
	funPinMode( PB14, GPIO_CFGLR_OUT_50Mhz_AF_PP );
	funPinMode( PB15, GPIO_CFGLR_OUT_50Mhz_AF_PP );

	memset( sendbuff, 0xac, sizeof(sendbuff) ); 

	RCC->APB1PRSTR = RCC_SPI2RST;
	RCC->APB1PRSTR = 0;
	RCC->APB1PCENR |= RCC_APB1Periph_SPI2;
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;

	// Configure SPI 
	SPI2->CTLR1 = 
		SPI_NSS_Soft | SPI_CPHA_1Edge | SPI_CPOL_Low | SPI_DataSize_16b |
		SPI_Mode_Master | SPI_Direction_1Line_Tx |
		0 |
		0<<3; // Divisior = 0

	// If using DMA may need this.
	SPI2->CTLR2 = SPI_CTLR2_TXDMAEN;


	SPI2->HSCR = 1; // High-speed enable.


	SPI2->I2SCFGR |= SPI_I2SCFGR_I2SE; // Enable
	SPI2->CTLR1 |= CTLR1_SPE_Set;
	//SPI2->DATAR = 0x55aa; // Set SPI line Low.

	//DMA1_Channel5 is for SPI2TX
	DMA1_Channel5->PADDR = (uint32_t)&SPI2->DATAR;
	DMA1_Channel5->MADDR = (uint32_t)sendbuff;
	DMA1_Channel5->CNTR  = 0;// sizeof( bufferset )/2; // Number of unique copies.  (Don't start, yet!)
	DMA1_Channel5->CFGR  =
		DMA_M2M_Disable |		 
		DMA_Priority_VeryHigh |
		DMA_MemoryDataSize_HalfWord |
		DMA_PeripheralDataSize_HalfWord |
		DMA_MemoryInc_Enable |
		DMA_Mode_Normal | // OR DMA_Mode_Circular or DMA_Mode_Normal
		DMA_DIR_PeripheralDST |
		DMA_IT_TC | DMA_IT_HT; // Transmission Complete + Half Empty Interrupts. 

	NVIC_EnableIRQ( DMA1_Channel5_IRQn );
	DMA1_Channel5->CFGR |= DMA_CFGR1_EN;


	memset( sendbuff, 0xaa, sizeof( sendbuff ) );


	// Enter critical section.
	//__disable_irq();
	DMA1_Channel5->CFGR &= ~DMA_Mode_Circular;
	DMA1_Channel5->CNTR  = 0;
	DMA1_Channel5->MADDR = (uint32_t)sendbuff;
	//__enable_irq();

	DMA1_Channel5->CNTR = SENDBUFF_WORDS; // Number of unique uint16_t entries.
	DMA1_Channel5->CFGR |= DMA_Mode_Circular;


	while(1)
	{
	}
}

