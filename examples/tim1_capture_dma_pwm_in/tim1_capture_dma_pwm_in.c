// Example using Timer 1 capture to look up- and down- going events
// on PD2.  Using DMA to capture that event and write it to a circular
// queue that can be read out later, from when the counter overflows.
//
// Please note that this causes resetting of timer1
//
// For it to produce interesting output, you can wire PD2 to PD3, PD3
// is outputting pulses.

#include "ch32fun.h"
#include <stdio.h>

uint32_t count;
uint16_t buffer1[256];
uint16_t buffer2[256];

int main()
{
	SystemInit();
	funGpioInitAll();

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_TIM1 | RCC_APB2Periph_AFIO;
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;

	// TIM2_CH1 -> PD2 -> DMA1_CH2
	#define DMA_IN1  DMA1_Channel2 // 1 also resets.
	#define DMA_IN2  DMA1_Channel5 // So instead of using DMA for TIM2 CH2, we use TIM_UP

	// T1C1 -> What we are sampling.
	funPinMode( PD2, GPIO_CFGLR_IN_PUPD );
	funDigitalWrite( PD2, 1 );

	// PD3 output to send something back into PD2.
	funPinMode( PD3, GPIO_CFGLR_OUT_2Mhz_PP );

	int samples_in_buffer = sizeof(buffer1) / sizeof(buffer1[0]);

	// TIM1_TRIG/TIM1_COM/TIM1_CH4
	DMA_IN1->MADDR = (uint32_t)buffer1;
	DMA_IN1->PADDR = (uint32_t)&TIM1->CH1CVR; // Input
	DMA_IN1->CFGR = 
		0                 |                  // PERIPHERAL to MEMORY
		0                 |                  // Low priority.
		DMA_CFGR1_MSIZE_0 |                  // 16-bit memory
		DMA_CFGR1_PSIZE_0 |                  // 16-bit peripheral
		DMA_CFGR1_MINC    |                  // Increase memory.
		DMA_CFGR1_CIRC    |                  // Circular mode.
		0                 |                  // NO Half-trigger
		0                 |                  // NO Whole-trigger
		DMA_CFGR1_EN;                        // Enable
	DMA_IN1->CNTR = samples_in_buffer;

	DMA_IN2->MADDR = (uint32_t)buffer2;
	DMA_IN2->PADDR = (uint32_t)&TIM1->CH2CVR; // Input
	DMA_IN2->CFGR = 
		0                 |                  // PERIPHERAL to MEMORY
		0                 |                  // Low priority.
		DMA_CFGR1_MSIZE_0 |                  // 16-bit memory
		DMA_CFGR1_PSIZE_0 |                  // 16-bit peripheral
		DMA_CFGR1_MINC    |                  // Increase memory.
		DMA_CFGR1_CIRC    |                  // Circular mode.
		0                 |                  // NO Half-trigger
		0                 |                  // NO Whole-trigger
		DMA_CFGR1_EN;                        // Enable
	DMA_IN2->CNTR = samples_in_buffer;


	TIM1->PSC = 0x01ff;		// set TIM1 clock prescaler divider (Massive prescaler)
	TIM1->ATRLR = 65535;	// set PWM total cycle width

	// Tim 1 input / capture (CC1S = 01) / Tim 2 / capture input CC2S = 10)
	TIM1->CHCTLR1 = TIM_CC1S_0 | TIM_CC2S_1;

	// Set CC1 and CC2 to have opposite polarities.
	TIM1->CCER = TIM_CC1E | TIM_CC2E | TIM_CC2P;
	
	// initialize counter
	TIM1->SWEVGR = TIM_UG;

	// Setup slave mode for tim1 input to go to tim2.
	TIM1->SMCFGR = TIM_TS_0 | TIM_TS_2 | TIM_SMS_2;

	// Enable TIM2
	TIM1->CTLR1 = TIM_ARPE | TIM_CEN;
	TIM1->DMAINTENR = TIM_CC1DE | TIM_CC2DE | TIM_UDE; // Enable DMA for T1CC1 + T1CC2

	int tail = 0;
	int frame = 0;

	while(1)
	{
		// Must perform modulus here, in case DMA_IN->CNTR == 0.
		int head1 = (samples_in_buffer - DMA_IN1->CNTR) & (samples_in_buffer-1);
		int head2 = (samples_in_buffer - DMA_IN2->CNTR) & (samples_in_buffer-1);
		int head = head2;
		// Just in case head
		if( head1 == head2-1 ) head = head1;
		else if( head == head2 || head2 == head1-1 );
		else
		{
			// Should never ever be possible.
			printf( "Timer DMA Fault\n" );
			continue;
		}

		while( head != tail )
		{
			uint32_t time_of_event0 = buffer1[tail];
			uint32_t time_of_event1 = buffer2[tail];
			printf( "A %d, %d\n", (int)time_of_event0, (int)time_of_event1 );

			// Performs modulus to loop back.
			tail = (tail+1)&(samples_in_buffer-1);
		}

		funDigitalWrite( PD3, 1 );
		Delay_Ms(frame & 31);
		funDigitalWrite( PD3, 0 );
		printf( "." );

		frame++;
	}
}

