// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v00x.h"
#include <stdio.h>
#include <string.h>

#define WS2812DMA_IMPLEMENTATION

#include "ws2812b_dma_spi_led_driver.h"

#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "color_utilities.h"

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

#define NR_LEDS 191

uint16_t phases[NR_LEDS];
int frameno;
volatile int tween = 0;

// Callbacks that you must implement.
uint32_t WS2812BLEDCallback( int ledno )
{
	uint8_t index = (phases[ledno])>>8;
	uint8_t rsbase = sintable[index];
	uint8_t rs = rsbase>>3;
	uint32_t fire = huetable[(rs+190&0xff)] | (huetable[(rs+30&0xff)]<<8) | (huetable[(rs+0)]<<16);
	uint32_t ice  = 0xff | ((rsbase)<<8) | ((rsbase)<<16);

	// Because this chip doesn't natively support multiplies, this can be very slow.
	return TweenHexColors( fire, ice, tween ); // Where "tween" is a value from 0 ... 255
}

int main()
{
	int k;

	// Enable GPIOD (for debugging)
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	WS2812BDMAInit( );

	printf( "CFG: %08x\n", SPI1->CTLR1 );

	frameno = 0;

	for( k = 0; k < NR_LEDS; k++ ) phases[k] = k<<8;


	int tweendir = 0;

	while(1)
	{
		// Wait for LEDs to totally finish.
		Delay_Ms( 12 );

		while( WS2812BLEDInUse );

		frameno++;

		if( frameno == 1024 )
		{
			tweendir = 4;
		}
		if( frameno == 2048 )
		{
			tweendir = -4;
			frameno = 0;
		}

		if( tweendir )
		{
			int t = tween + tweendir;
			if( t > 255 ) t = 255;
			if( t < 0 ) t = 0;
			tween = t;
		}

		for( k = 0; k < NR_LEDS; k++ )
		{
			phases[k] += ((((rands[k&0xff])+0xf)<<2) + (((rands[k&0xff])+0xf)<<1))>>1;
		}

		WS2812BDMAStart( NR_LEDS );
	}
}

