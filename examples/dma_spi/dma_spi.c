/* 
SPI DMA example with software NSS
Based on cnlohr's example for the CH32V203
Author: Fredrik Saevland (fsaev)
		https://github.com/fsaev

MIT License

Copyright (c) 2024 Fredrik Saevland

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ch32fun.h"

#define SENDBUFF_BYTES 8
uint8_t sendbuff[] = {0, 1, 2, 3, 4, 5, 6, 7};

enum TransferState {
	IDLE,
	TRANSMITTING,
	DMA_DONE
};

volatile enum TransferState spi_state = IDLE;

void DMA1_Channel3_IRQHandler( void ) __attribute__((interrupt));

void DMA1_Channel3_IRQHandler( void ) 
{

	// Backup flags.
	volatile int intfr = DMA1->INTFR;
	do
	{
		// Clear all possible flags.
		DMA1->INTFCR = 0xFFFFFFFF;

		if(intfr & DMA1_IT_TC3) { // Transmission complete
			if(spi_state == TRANSMITTING) {
				spi_state = DMA_DONE;
			}
		}

		intfr = DMA1->INTFR;
	} while( intfr );
}

/* Starts a single transmission of sendbuff */
void initiate_transfer()
{
	funDigitalWrite(PC0, FUN_LOW); // Set CS low
	spi_state = TRANSMITTING;
	DMA1_Channel3->CFGR &= ~DMA_CFGR1_EN; // Disable DMA in order to write to CNTR
	DMA1_Channel3->CNTR  = SENDBUFF_BYTES; // Reload size of buffer
	DMA1_Channel3->CFGR |= DMA_CFGR1_EN; // Initate DMA transfer
}

int main()
{
	SystemInit();

	funGpioInitAll();

	/* Set up pins for Alternate Function*/
	// PC5 is SCK, 50MHz Output, alt func, p-p
	funPinMode( PC5, GPIO_CFGLR_OUT_50Mhz_AF_PP );
	
	// PC6 is MOSI, 50MHz Output, alt func, p-p
	funPinMode( PC6, GPIO_CFGLR_OUT_50Mhz_AF_PP );

	// PC0 is Software NSS, 50MHz Output, GPIO, p-p
	funPinMode( PC0, GPIO_CFGLR_OUT_50Mhz_PP );
	funDigitalWrite(PC0, FUN_HIGH);

	RCC->APB2PRSTR = RCC_SPI1RST;
	RCC->APB2PRSTR = 0;
	RCC->APB2PCENR |= RCC_APB2Periph_SPI1;
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;

	// Configure SPI
	SPI1->CTLR1 = 
		SPI_NSS_Soft | SPI_CPHA_1Edge | SPI_CPOL_Low | SPI_DataSize_8b |
		SPI_Mode_Master | SPI_Direction_1Line_Tx | SPI_BaudRatePrescaler_16;

	// Enable TX DMA
	SPI1->CTLR2 = SPI_CTLR2_TXDMAEN;

	//SPI1->HSCR = 1; // High-speed enable.
	SPI1->CTLR1 |= CTLR1_SPE_Set;

	//DMA1_Channel3 is for SPI1TX
	DMA1_Channel3->PADDR = (uint32_t)&SPI1->DATAR;
	DMA1_Channel3->MADDR = (uint32_t)sendbuff;
	DMA1_Channel3->CFGR  =
		DMA_M2M_Disable |
		DMA_Priority_VeryHigh |
		DMA_MemoryDataSize_Byte | // 8-bit mode
		DMA_PeripheralDataSize_Byte |
		DMA_MemoryInc_Enable |
		DMA_Mode_Normal | // DMA_Mode_Normal
		DMA_DIR_PeripheralDST |
		DMA_IT_TC; // Transmission Complete

	NVIC_EnableIRQ( DMA1_Channel3_IRQn );

	uint32_t timestamp = 0;

	while(1)
	{
		if(SysTick->CNT - timestamp > 10000)
		{
			timestamp = SysTick->CNT;

			// Send repeatedly
			initiate_transfer();
		}

		// When DMA is done and last byte is clocked out, set CS high
		if(spi_state == DMA_DONE && !(SPI1->STATR & SPI_I2S_FLAG_BSY)){
			funDigitalWrite(PC0, FUN_HIGH); // Set CS high
			spi_state = IDLE;
		}
	}
}

