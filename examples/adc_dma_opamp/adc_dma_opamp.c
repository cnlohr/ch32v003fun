/*
 * Example for using ADC with DMA and an op-amp
 * 04-13-2023 E. Brombaugh
 */

#include "ch32v003fun.h"
#include <stdio.h>

#define ADC_NUMCHLS 4
volatile uint16_t adc_buffer[ADC_NUMCHLS];

/*
 * initialize adc for DMA
 */
void adc_init( void )
{
	// ADCCLK = 24 MHz => RCC_ADCPRE = 0: divide by 2
	RCC->CFGR0 &= ~(0x1F<<11);
	
	// Enable GPIOD and ADC
	RCC->APB2PCENR |=	RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
						RCC_APB2Periph_ADC1;
	
	// PD4 is analog input chl 7
	GPIOD->CFGLR &= ~(0xf<<(4*4));	// CNF = 00: Analog, MODE = 00: Input
	
	// PD3 is analog input chl 4
	GPIOD->CFGLR &= ~(0xf<<(4*3));	// CNF = 00: Analog, MODE = 00: Input
	
	// PD2 is analog input chl 3
	GPIOD->CFGLR &= ~(0xf<<(4*2));	// CNF = 00: Analog, MODE = 00: Input
	
	// PC4 is analog input chl 2
	GPIOC->CFGLR &= ~(0xf<<(4*4));	// CNF = 00: Analog, MODE = 00: Input
	
	// Reset the ADC to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;
	
	// Set up four conversions on chl 7, 4, 3, 2
	ADC1->RSQR1 = (ADC_NUMCHLS-1) << 20;	// four chls in the sequence
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = (7<<(5*0)) | (4<<(5*1)) | (3<<(5*2)) | (2<<(5*3));
	
	// set sampling time for chl 7, 4, 3, 2
	// 0:7 => 3/9/15/30/43/57/73/241 cycles
	ADC1->SAMPTR2 = (7<<(3*7)) | (7<<(3*4)) | (7<<(3*3)) | (7<<(3*2));

	// turn on ADC
	ADC1->CTLR2 |= ADC_ADON;
	
	// Reset calibration
	ADC1->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);
	
	// Calibrate
	ADC1->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);
	
	// Turn on DMA
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;
	
	//DMA1_Channel1 is for ADC
	DMA1_Channel1->PADDR = (uint32_t)&ADC1->RDATAR;
	DMA1_Channel1->MADDR = (uint32_t)adc_buffer;
	DMA1_Channel1->CNTR  = ADC_NUMCHLS;
	DMA1_Channel1->CFGR  =
		DMA_M2M_Disable |		 
		DMA_Priority_VeryHigh |
		DMA_MemoryDataSize_HalfWord |
		DMA_PeripheralDataSize_HalfWord |
		DMA_MemoryInc_Enable |
		DMA_Mode_Circular |
		DMA_DIR_PeripheralSRC;
	
	// Turn on DMA channel 1
	DMA1_Channel1->CFGR |= DMA_CFGR1_EN;
	
	// enable scanning
	ADC1->CTLR1 |= ADC_SCAN;
	
	// Enable continuous conversion and DMA
	ADC1->CTLR2 |= ADC_CONT | ADC_DMA | ADC_EXTSEL;
	
	// start conversion
	ADC1->CTLR2 |= ADC_SWSTART;
}

/*
 * turn on op-amp, select input pins
 */
void opamp_init( void )
{
	// turn on the op-amp
	EXTEN->EXTEN_CTR |= EXTEN_OPA_EN;
	
	// select op-amp pos pin: 0 = PA2, 1 = PD7
	//EXTEN->EXTEN_CTR |= EXTEN_OPA_PSEL;

	// select op-amp neg pin: 0 = PA1, 1 = PD0
	//EXTEN->EXTEN_CTR |= EXTEN_OPA_NSEL;
}

/*
 * entry
 */
int main()
{
	SystemInit();

	// start serial @ default 115200bps
	Delay_Ms(100);
	printf("\r\r\n\nadc_dma_opamp example\n\r");

	// init adc
	printf("initializing adc...");
	adc_init();
	printf("done.\n\r");
	
	// init op-amp
	printf("initializing op-amp...\n\r");
	opamp_init();
	printf("done.\n\r");
	
	// Enable GPIO for blinky diag
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

	// GPIO C1 Push-Pull for blinky diag
	GPIOC->CFGLR &= ~(0xf<<(4*1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*1);

	printf("looping...\n\r");
	while(1)
	{
		GPIOC->BSHR = 1<<1;	 // Turn on GPIOs
		Delay_Ms( 100 );
		GPIOC->BSHR = 1<<(1+16); // Turn off GPIODs
		Delay_Ms( 100 );
		printf( "%4d %4d ", adc_buffer[0], adc_buffer[1]);
		printf( "%4d %4d\n\r", adc_buffer[2], adc_buffer[3]);
	}
}
