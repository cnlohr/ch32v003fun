// Small example showing how to use the USB HS interface of the CH32V30x
// A composite HID device + A bulk in and out.

#include "ch32fun.h"
#include <stdio.h>
#include <string.h>
#include "hsusb_v30x.h"

uint32_t count;

#define PWM_PERIOD 29

#define ADC_BUFFSIZE 4096
uint32_t adc_buffer[ADC_BUFFSIZE];

int last = 0;
void handle_debug_input( int numbytes, uint8_t * data )
{
	last = data[0];
	count += numbytes;
}

int lrx = 0;
uint32_t scratchpad[128];

int HandleHidUserSetReportSetup( struct _USBState * ctx, tusb_control_request_t * req )
{
	int id = req->wValue & 0xff;
	if( id == 0xaa && req->wLength <= sizeof(scratchpad) )
	{
		ctx->pCtrlPayloadPtr = (uint8_t*)scratchpad;
		lrx = req->wLength;
		return req->wLength;
	}
	return 0;
}

int HandleHidUserGetReportSetup( struct _USBState * ctx, tusb_control_request_t * req )
{
	int id = req->wValue & 0xff;
	if( id == 0xaa )
	{
		ctx->pCtrlPayloadPtr = (uint8_t*)scratchpad;
		if( sizeof(scratchpad) - 1 < lrx )
			return sizeof(scratchpad) - 1;
		else
			return lrx;
	}
	return 0;
}

void HandleHidUserReportDataOut( struct _USBState * ctx, uint8_t * data, int len )
{
}

int HandleHidUserReportDataIn( struct _USBState * ctx, uint8_t * data, int len )
{
	return len;
}

void HandleHidUserReportOutComplete( struct _USBState * ctx ) { return; }
void HandleGotEPComplete( struct _USBState * ctx, int ep ) { }


void SetupDualADC()
{
	// ADC CLK is chained off of APB2.

	// Reset the ADC to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

	// ADCCLK = 12 MHz => RCC_ADCPRE divide by 4
	RCC->CFGR0 &= ~RCC_ADCPRE;  // Clear out the bis in case they were set
	RCC->CFGR0 |= RCC_ADCPRE_DIV2; // Fastest possible (PCLK2/2) (this = 0) NOTE: This is OUTSIDE the specified value in the datasheet.

	// Set up single conversion on chl 7
	ADC1->RSQR1 = 0;
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = 5;

	ADC2->RSQR1 = 0;
	ADC2->RSQR2 = 0;
	ADC2->RSQR3 = 6;

	// Maximum speed.
	ADC1->SAMPTR2 = (0<<(3*5)); 
	ADC2->SAMPTR2 = (0<<(3*6)); 

	// Turn on ADC and set rule group to sw trig
	// 0 = Use TRGO event for Timer 1 to fire ADC rule.
	ADC1->CTLR2 = ADC_ADON | ADC_EXTTRIG | ADC_DMA; 
	ADC2->CTLR2 = ADC_ADON | ADC_EXTTRIG; 

	// Reset calibration
	ADC1->CTLR2 |= ADC_RSTCAL;
	ADC2->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);
	while(ADC2->CTLR2 & ADC_RSTCAL);

	// Calibrate ADC
	ADC1->CTLR2 |= ADC_CAL;
	ADC2->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);
	while(ADC2->CTLR2 & ADC_CAL);

	// ADC_SCAN: Allow scanning.
//	ADC1->CTLR1 = ADC_Pga_1 | (1<<26) | ADC_SCAN | ADC_DUALMOD_0;
//	ADC2->CTLR1 = ADC_Pga_1 | (1<<26) | ADC_SCAN | ADC_DUALMOD_0;
	// ADC_DUALMOD_0 = Simultaneously sample both channels.
	// ADC_DUALMOD_2 | ADC_DUALMOD_1 | ADC_DUALMOD_0 = Interleave
	ADC1->CTLR1 = ADC_SCAN | ADC_DUALMOD_0; //| ADC_DUALMOD_2 | ADC_DUALMOD_1 | ADC_DUALMOD_0;
	ADC2->CTLR1 = ADC_SCAN | ADC_DUALMOD_0; //| ADC_DUALMOD_2 | ADC_DUALMOD_1 | ADC_DUALMOD_0;

	// Turn on DMA
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;

	//DMA1_Channel1 is for ADC
	DMA1_Channel1->PADDR = (uint32_t)&ADC1->RDATAR;
	DMA1_Channel1->MADDR = (uint32_t)adc_buffer;
	DMA1_Channel1->CNTR  = ADC_BUFFSIZE; // In words.
	DMA1_Channel1->CFGR  =
		DMA_M2M_Disable |		 
		DMA_Priority_VeryHigh |
		DMA_MemoryDataSize_Word |
		DMA_PeripheralDataSize_Word |
		DMA_MemoryInc_Enable |
		DMA_Mode_Circular |
		DMA_DIR_PeripheralSRC;

	// Turn on DMA channel 1
	DMA1_Channel1->CFGR |= DMA_CFGR1_EN;
	
	// Enable continuous conversion and DMA
	ADC1->CTLR2 |= ADC_DMA; // | ADC_CONT;

	// start conversion
	ADC1->CTLR2 |= ADC_SWSTART;// | ADC_CONT;
}


static void SetupTimer1()
{
	// Enable Timer 1
	RCC->APB2PRSTR |=  RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;

	TIM1->PSC = 0;  // Prescalar to 0x0000
	TIM1->ATRLR = PWM_PERIOD;

#ifdef PWM_OUTPUT
	// PA10 = T1CH3.
	GPIOA->CFGHR &= ~(0xf<<(4*2));
	GPIOA->CFGHR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*2);

	TIM1->CCER = TIM_CC3E | TIM_CC3P;
	TIM1->CHCTLR2 = TIM_OC3M_2 | TIM_OC3M_1;
	TIM1->CH3CVR = 5;  // Actual duty cycle (Off to begin with)
#endif

	TIM1->CCER = TIM_CC1E;
	TIM1->CHCTLR1 = TIM_OC1M_2 | TIM_OC1M_1;
	TIM1->CH1CVR = 1;

	// Setup TRGO to trigger for ADC (NOTE: Not on the 203! TIM1_TRGO is only connected to injection)
	//TIM1->CTLR2 = TIM_MMS_1;

	// Enable TIM1 outputs
	TIM1->BDTR = TIM_MOE;
	TIM1->CTLR1 = TIM_CEN;
}


void InnerLoop() __attribute__((noreturn));

void InnerLoop()
{
	int i = 0;
	int q = 0;
	int tpl = 0;

	// Timer goes backwards when we are moving forwards.
	volatile uint32_t * adc_buffer_end = 0;
	volatile uint32_t * adc_buffer_top = adc_buffer + ADC_BUFFSIZE;
	volatile uint32_t * adc = adc_buffer;
	uint32_t * spad = scratchpad;
	int frcnt = 0;

	// We can put up to 168 ADC reads in one USB slough if we do 128b96b
	#define Q 128

	while( 1 )
	{
		tpl = ADC_BUFFSIZE - DMA1_Channel1->CNTR; // Warning, sometimes this is == to the base, or == 0 (i.e. might be 256, if top is 255)
		if( tpl == ADC_BUFFSIZE ) tpl = 0;

		adc_buffer_end = adc_buffer + ( ( tpl / 4) * 4 );


		// Whenever we have more data to read (well at least 4-more samples)
		// Start blasting through them.
		while( adc != adc_buffer_end )
		{
			uint32_t L0 = adc[0];
			uint32_t L1 = adc[1];
			uint32_t L2 = adc[2];
			uint32_t L3 = adc[3];

			// Do processing here if you want.
			// This could do 128b96b but eh...
			//spad[0] = L0 | (( L3 & 0x0F000F00) << 4);
			//spad[1] = L1 | (( L3 & 0x00F000F0) << 8);
			//spad[2] = L2 | (( L3 & 0x000F000F) << 12);
			spad[0] = L0;
			spad[1] = L1;
			spad[2] = L2;
			spad[3] = L3;

			adc += 4;
			frcnt += 4;
			spad += 4;

			if( adc == adc_buffer_top ) adc = adc_buffer;
			if( frcnt >= Q ) break;

			// Incoming data:
			//   0AAA0BBB
			//   0CCC0DDD
			//   0EEE0FFF
			//   0GGG0HHH
			//
			// Outgoing data:
			//   GAAAHBBB
			//   GCCCHDDD
			//   GEEEHFFF

		}

		if( frcnt >= Q )
		{
			while( HSUSBCTX.USBHS_Endp_Busy[2] & 1 );
			USBHS_SendEndpoint( 2, 512, (uint8_t*)scratchpad );
			spad = scratchpad;
			frcnt = 0;
		}
	}

}
int main()
{
	SystemInit();

	funGpioInitAll();

	funPinMode( PA3, GPIO_CFGLR_OUT_10Mhz_PP );
	funPinMode( PB10, GPIO_CFGLR_OUT_10Mhz_PP );

	// Turn up power to core, to make it work better
	// at higher frequencies.
	EXTEN->EXTEN_CTR = (3<<8) | (3<<10);

	//#define SET_CLOCK_TO_180MHz 1
	//#define SET_CLOCK_TO_120MHz 1

#if SET_CLOCK_TO_180MHz
	// Try overclocking to 180MHz, then target 6.0MSPS (Hint, it's not great :()

	RCC->INTR  = 0x009F0000;                               // Clear PLL, CSSC, HSE, HSI and LSI ready flags.

	// Switch processor back to HSI so we don't eat dirt.
	RCC->CFGR0 = 0;

	// Disable PLL so we can play with it.
	RCC->CTLR &= ~( RCC_PLL3ON | RCC_PLL2ON | RCC_PLLON );

	// Not sure why, need to reset here, otherwise PLLXTPRE is set.
	RCC->CFGR0 = RCC_PLLSRC;

	// Setup clock tree.
	RCC->CFGR2 |= 
		(0<<RCC_PREDIV2_OFFSET) | // PREDIV2
		(4<<RCC_PLL2MUL_OFFSET) | // PLL2
		(3<<RCC_PREDIV1_OFFSET) | // PREDIV1
		RCC_PREDIV1SRC | // PLLSRC = PREDIV1 = from PLL, not HSE.
		0;

	// Power on PLLs
	RCC->CTLR |= RCC_PLL3ON | RCC_PLL2ON;
	int timeout;

	for( timeout = 10000; timeout > 0; timeout--) if (RCC->CTLR & RCC_PLL3RDY) break;
	if( timeout == 0 ) return -5;
	for( timeout = 10000; timeout > 0; timeout--) if (RCC->CTLR & RCC_PLL2RDY) break;
	if( timeout == 0 ) return -6;

	// Set PLL multiplier. (whatever is before the <<18)
	RCC->CFGR0 = ( RCC->CFGR0 & ~(0xf<<18)) | (14<<18);
	RCC->CTLR |= RCC_PLLON;

	for( timeout = 10000; timeout > 0; timeout--) if (RCC->CTLR & RCC_PLLRDY) break;
	if( timeout == 0 ) return -7;

	// If we wanted, we could output PLL3/2 on PA8.
	//RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_PLL | (9<<24); // And output clock on PA8
	RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_PLL;

#elif SET_CLOCK_TO_120MHz
	// Try underclocking to 120MHz, then target 4.0MSPS

	RCC->INTR  = 0x009F0000;                               // Clear PLL, CSSC, HSE, HSI and LSI ready flags.

	// Switch processor back to HSI so we don't eat dirt.
	RCC->CFGR0 = 0;

	// Disable PLL so we can play with it.
	RCC->CTLR &= ~( RCC_PLL3ON | RCC_PLL2ON | RCC_PLLON );

	// Not sure why, need to reset here, otherwise PLLXTPRE is set.
	RCC->CFGR0 = RCC_PLLSRC;

	// Setup clock tree.
	RCC->CFGR2 |= 
		(0<<RCC_PREDIV2_OFFSET) | // PREDIV2
		(4<<RCC_PLL2MUL_OFFSET) | // PLL2
		(3<<RCC_PREDIV1_OFFSET) | // PREDIV1
		RCC_PREDIV1SRC | // PLLSRC = PREDIV1 = from PLL, not HSE.
		0;

	// Power on PLLs
	RCC->CTLR |= RCC_PLL3ON | RCC_PLL2ON;
	int timeout;

	for( timeout = 10000; timeout > 0; timeout--) if (RCC->CTLR & RCC_PLL3RDY) break;
	if( timeout == 0 ) return -5;
	for( timeout = 10000; timeout > 0; timeout--) if (RCC->CTLR & RCC_PLL2RDY) break;
	if( timeout == 0 ) return -6;

	// Set PLL multiplier. (whatever is before the <<18)
	RCC->CFGR0 = ( RCC->CFGR0 & ~(0xf<<18)) | (8<<18);
	RCC->CTLR |= RCC_PLLON;

	for( timeout = 10000; timeout > 0; timeout--) if (RCC->CTLR & RCC_PLLRDY) break;
	if( timeout == 0 ) return -7;

	// If we wanted, we could output PLL3/2 on PA8.
	//RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_PLL | (9<<24); // And output clock on PA8
	RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_PLL;

#endif

	// Tweak ADC Clock duty (Seems not be that important)
	RCC->CFGR0 |= (1<<31);

	HSUSBSetup();


	funDigitalWrite( PA3, FUN_LOW );
	funDigitalWrite( PB10, FUN_HIGH ); // for debugging

	RCC->AHBPCENR |= 3; //DMA2EN | DMA1EN
	RCC->APB2PCENR |= RCC_APB2Periph_TIM1 | RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | 0x07; // Enable all GPIO
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	funPinMode( PA5, GPIO_CFGLR_IN_ANALOG );	// ADC_IN5 = PA5
	funPinMode( PA6, GPIO_CFGLR_IN_ANALOG );	// ADC_IN6 = PA6

	SetupDualADC();
	SetupTimer1();

	USBHSD->UEP2_MAX_LEN = 512;

	InnerLoop();
/*
	while(1)
	{
		//printf( "%lu %08lx %lu %d %d\n", USBDEBUG0, USBDEBUG1, USBDEBUG2, 0, 0 );
		//printf( "%d\n", DMA1_Channel1->CNTR );

		// Send data back to PC.
		if( !( HSUSBCTX.USBHS_Endp_Busy[2] & 1 ) )
		{
			USBHS_SendEndpoint( 2, 512, scratchpad );
			USBHSD->UEP2_TX_DMA = (uintptr_t)scratchpad;
		}
	}
*/
}

