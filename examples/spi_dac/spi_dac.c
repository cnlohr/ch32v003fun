/*
 * Example for SPI with circular DMA for audio output
 * 04-10-2023 E. Brombaugh
 */

#include "ch32v003fun.h"
#include <stdio.h>

// this table contains a 512-sample 16-bit sine waveform
#include "Sine16bit.h"

// uncomment this to enable GPIO diag
#define DAC_DIAG

// Length of the DAC DMA buffer
#define DACBUFSZ 32

uint16_t dac_buffer[DACBUFSZ];
uint32_t osc_phs[2], osc_frq[2];

/*
 * initialize SPI and DMA
 */
void spidac_init( void )
{
	// init two oscillators
	osc_phs[0] = 0;
	osc_phs[1] = 0;
	osc_frq[0] = 0x01000000;
	osc_frq[1] = 0x00400000;
	
	// Enable DMA + Peripherals
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1;

#ifdef DAC_DIAG
	// GPIO D0 10MHz Push-Pull for diags
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);
#endif
	
	// MOSI on PC6, 10MHz Output, alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*6));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*6);
	
	// SCK on PC5, 10MHz Output, alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*5));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);

	// Configure SPI 
	SPI1->CTLR1 = 
		SPI_NSS_Soft | SPI_CPHA_1Edge | SPI_CPOL_Low | SPI_DataSize_16b |
		SPI_Mode_Master | SPI_Direction_1Line_Tx |
		SPI_BaudRatePrescaler_16;

	// enable SPI port
	SPI1->CTLR1 |= CTLR1_SPE_Set;

	// TIM1 generates DMA Req and external signal
	// Enable TIM1
	RCC->APB2PCENR |= RCC_APB2Periph_TIM1;
	
	// PC4 is T1CH4, 10MHz Output, alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*4));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*4);
		
	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;
	
	// CTLR1: default is up, events generated, edge align
	// CTLR1: up/down, events on both edges
	TIM1->CTLR1 = TIM_CMS;
	// SMCFGR: default clk input is CK_INT
	
	// Prescaler 
	TIM1->PSC = 0x0000;
	
	// Auto Reload - sets period to ~47kHz
	TIM1->ATRLR = 499;
	
	// Reload immediately
	TIM1->SWEVGR |= TIM_UG;
	
	// Enable CH4 output, positive pol
	TIM1->CCER |= TIM_CC4E;// | TIM_CC4P;
	
	// CH2 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
	TIM1->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1;
	
	// Set the Capture Compare Register value to 50% initially
	TIM1->CH4CVR = 256;
	
	// Enable TIM1 outputs
	TIM1->BDTR |= TIM_MOE;
	
	// Enable CH4 DMA Req
	TIM1->DMAINTENR |= TIM_CC4DE;
	
	// Enable TIM1
	TIM1->CTLR1 |= TIM_CEN;

	//DMA1_Channel4 is for TIM1CH4 
	DMA1_Channel4->PADDR = (uint32_t)&SPI1->DATAR;
	DMA1_Channel4->MADDR = (uint32_t)dac_buffer;
	DMA1_Channel4->CNTR  = DACBUFSZ;
	DMA1_Channel4->CFGR  =
		DMA_M2M_Disable |		 
		DMA_Priority_VeryHigh |
		DMA_MemoryDataSize_HalfWord |
		DMA_PeripheralDataSize_HalfWord |
		DMA_MemoryInc_Enable |
		DMA_Mode_Circular |
		DMA_DIR_PeripheralDST |
		DMA_IT_TC | DMA_IT_HT;

	NVIC_EnableIRQ( DMA1_Channel4_IRQn );
	DMA1_Channel4->CFGR |= DMA_CFGR1_EN;
}

/*
 * DAC buffer updates - called at IRQ time
 */
void dac_update(uint16_t *buffer)
{
	int i;
	
	// fill the buffer with stereo data
	for(i=0;i<DACBUFSZ/2;i+=2)
	{
		// right chl
		*buffer++ = Sine16bit[osc_phs[0]>>24];
		osc_phs[0] += osc_frq[0];
		
		// left chl
		//*buffer++ = Sine16bit[osc_phs[1]>>24];
		*buffer++ = osc_phs[1]>>16;
		osc_phs[1] += osc_frq[1];
	}
}

/*
 * TIM1CH4 DMA IRQ Handler
 */
void DMA1_Channel4_IRQHandler( void ) __attribute__((interrupt));
void DMA1_Channel4_IRQHandler( void ) 
{
#ifdef DAC_DIAG
	GPIOD->BSHR = 1;
#endif
	
	// why is this needed? Can't just direct compare the reg in tests below
	volatile uint16_t intfr = DMA1->INTFR;

	if( intfr & DMA1_IT_TC4 )
	{
		// Transfer complete - update 2nd half
		dac_update(dac_buffer+DACBUFSZ/2);
		
		// clear TC IRQ
		DMA1->INTFCR = DMA1_IT_TC4;
		
		GPIOC->BSHR = (1<<1); // NSS 1
	}
	
	if( intfr & DMA1_IT_HT4 )
	{
		// Half transfer - update first half
		dac_update(dac_buffer);
		
		// clear HT IRQ
		DMA1->INTFCR = DMA1_IT_HT4;
		
		GPIOC->BSHR = (1<<(1+16)); // NSS 0
	}

	// clear the Global IRQ
	DMA1->INTFCR = DMA1_IT_GL4;
	
#ifdef DAC_DIAG
	GPIOD->BSHR = 1<<16;
#endif
}

/*
 * entry
 */
int main()
{
	SystemInit();

	// start serial @ default 115200bps
	Delay_Ms( 100 );
	printf("\r\r\n\nspi_dac example\n\r");

	// GPIO C0 Push-Pull for foreground blink
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
	GPIOC->CFGLR &= ~(0xf<<(4*0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);
	
	// init SPI DAC
	printf("initializing spi dac...");
	spidac_init();
	printf("done.\n\r");
		
	printf("looping...\n\r");
	while(1)
	{
		GPIOC->BSHR = 1;
		Delay_Ms( 250 );
		GPIOC->BSHR = (1<<16);
		Delay_Ms( 250 );
	}
}
