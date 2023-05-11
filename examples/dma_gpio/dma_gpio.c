// DMA GPIO Output Example - this example shows
// how you can output 8 pins all simultaneously
// with a planned bit pattern at 4MSamples/s.
//
// It outputs a pattern of repeating 010101 and
// 00000 alternating "frames".
//
// The interrupt fires once at the beginning and
// once at the end.
//
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

volatile uint32_t count;

#define MBSAMPS 1024
uint8_t memory_buffer[1024];

void DMA1_Channel2_IRQHandler( void ) __attribute__((interrupt)) __attribute__((section(".srodata")));
void DMA1_Channel2_IRQHandler( void ) 
{
	int i;
	static int frameno;
	volatile int intfr = DMA1->INTFR;
	do
	{
		DMA1->INTFCR = DMA1_IT_GL2;

		// Gets called at the end-of-a frame.
		if( intfr & DMA1_IT_TC2 )
		{
			uint32_t fv = (frameno&1)?0:0xaa55aa55;
			uint32_t * mbb = (uint32_t*)( memory_buffer + MBSAMPS/2 );
			for( i = 0; i < MBSAMPS/8; i++ )
			{
				mbb[i] = fv; // Fill in the frame data
			}
			frameno++;
		}
		
		// Gets called halfway through the frame
		if( intfr & DMA1_IT_HT2 )
		{
			uint32_t fv = (frameno&1)?0:0xaa55aa55;
			uint32_t * mbb = (uint32_t*)( memory_buffer );
			for( i = 0; i < MBSAMPS/8; i++ )
			{
				mbb[i] = fv; // Fill in the frame data.
			}
		}
		intfr = DMA1->INTFR;
	} while( intfr );
}

int main()
{
	int i;

	SystemInit48HSI();

	// Reset all the peripherals we care about.
	RCC->APB2PRSTR = 0xffffffff;
	RCC->APB2PRSTR = 0;
	
	SetupDebugPrintf();

	// Enable DMA
	RCC->AHBPCENR = RCC_AHBPeriph_SRAM | RCC_AHBPeriph_DMA1;

	// Enable GPIO and Timer 1
	RCC->APB2PCENR = RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA;

	// GPIO D0/D4 Push-Pull (LED)
	GPIOD->CFGLR =
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*1) |    // PD1 = SWIO (so we don't go off-bus)
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*2) | // PD2 = T1CH1
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	// GPIO C All output.
	GPIOC->CFGLR =
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*1) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*2) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*3) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*5) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*6) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*7);


	// Fill in the plan of what we will be sending out.
	for( i = 0; i < sizeof(memory_buffer) / sizeof(memory_buffer[0]); i++ )
	{
		memory_buffer[i] = (i&1)?0xaa:0x55;
	}

	// DMA2 can be configured to attach to T1CH1
	// The system can only DMA out at ~2.2MSPS.  2MHz is stable.
	DMA1_Channel2->CNTR = sizeof(memory_buffer) / sizeof(memory_buffer[0]);
	DMA1_Channel2->MADDR = (uint32_t)memory_buffer;
	DMA1_Channel2->PADDR = (uint32_t)&GPIOC->OUTDR;
	DMA1_Channel2->CFGR = 
		DMA_CFGR1_DIR |                      // MEM2PERIPHERAL
		DMA_CFGR1_PL |                       // High priority.
		0 |                                  // 8-bit memory
		0 |                                  // 8-bit peripheral
		DMA_CFGR1_MINC |                     // Increase memory.
		DMA_CFGR1_CIRC |                     // Circular mode.
		DMA_CFGR1_HTIE |                     // Half-trigger
		DMA_CFGR1_TCIE |                     // Whole-trigger
		DMA_CFGR1_EN;                        // Enable

	NVIC_EnableIRQ( DMA1_Channel2_IRQn );
	DMA1_Channel2->CFGR |= DMA_CFGR1_EN;

	// NOTE: You can also hook up DMA1 Channel 3 to T1C2,
	// if you want to output to multiple IO ports at
	// at the same time.  Just be aware you have to offset
	// the time they read at by at least 1/8Mth of a second.

	// Setup Timer1.
	RCC->APB2PRSTR = RCC_APB2Periph_TIM1;    // Reset Timer
	RCC->APB2PRSTR = 0;

	// Timer 1 setup.
	TIM1->PSC = 0x0000;                      // Prescaler 
	TIM1->ATRLR = 11;                        // Auto Reload - sets period (4MHz)
	TIM1->SWEVGR = TIM_UG | TIM_TG;          // Reload immediately + Trigger DMA
	TIM1->CCER = TIM_CC1E | TIM_CC1P;        // Enable CH1 output, positive pol
	TIM1->CHCTLR1 = TIM_OC1M_2 | TIM_OC1M_1; // CH1 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
	TIM1->CH1CVR = 6;                        // Set the Capture Compare Register value to 50% initially
	TIM1->BDTR = TIM_MOE;                    // Enable TIM1 outputs
	TIM1->CTLR1 = TIM_CEN;                   // Enable TIM1
	TIM1->DMAINTENR = TIM_UDE | TIM_CC1DE;   // Trigger DMA on TC match 1 (DMA Ch2) and TC match 2 (DMA Ch3)

	// Just debug stuff.
	printf( "Setup OK\n" );

	while(1)
	{
		GPIOD->BSHR = 1 | (1<<4);	 // Turn on GPIOs
		printf( "%lu\n", GPIOD->OUTDR );
		Delay_Ms( 250 );
		GPIOD->BSHR = (1<<16) | (1<<(16+4)); // Turn off GPIODs
		printf( "%lu\n", GPIOD->OUTDR );
		Delay_Ms( 250 );
		printf( "Step\n" );
	}
}

