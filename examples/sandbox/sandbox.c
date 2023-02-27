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




uint32_t EHSVtoHEX( uint8_t hue, uint8_t sat, uint8_t val )
{
	#define SIXTH1 43
	#define SIXTH2 85
	#define SIXTH3 128
	#define SIXTH4 171
	#define SIXTH5 213

	uint16_t or = 0, og = 0, ob = 0;

	hue -= SIXTH1; //Off by 60 degrees.

	//TODO: There are colors that overlap here, consider 
	//tweaking this to make the best use of the colorspace.

	if( hue < SIXTH1 ) //Ok: Yellow->Red.
	{
		or = 255;
		og = 255 - ((uint16_t)hue * 255) / (SIXTH1);
	}
	else if( hue < SIXTH2 ) //Ok: Red->Purple
	{
		or = 255;
		ob = (uint16_t)hue*255 / SIXTH1 - 255;
	}
	else if( hue < SIXTH3 )  //Ok: Purple->Blue
	{
		ob = 255;
		or = ((SIXTH3-hue) * 255) / (SIXTH1);
	}
	else if( hue < SIXTH4 ) //Ok: Blue->Cyan
	{
		ob = 255;
		og = (hue - SIXTH3)*255 / SIXTH1;
	}
	else if( hue < SIXTH5 ) //Ok: Cyan->Green.
	{
		og = 255;
		ob = ((SIXTH5-hue)*255) / SIXTH1;
	}
	else //Green->Yellow
	{
		og = 255;
		or = (hue - SIXTH5) * 255 / SIXTH1;
	}

	uint16_t rv = val;
	if( rv > 128 ) rv++;
	uint16_t rs = sat;
	if( rs > 128 ) rs++;

	//or, og, ob range from 0...255 now.
	//Need to apply saturation and value.

	or = (or * val)>>8;
	og = (og * val)>>8;
	ob = (ob * val)>>8;

	//OR..OB == 0..65025
	or = or * rs + 255 * (256-rs);
	og = og * rs + 255 * (256-rs);
	ob = ob * rs + 255 * (256-rs);
//printf( "__%d %d %d =-> %d\n", or, og, ob, rs );

	or >>= 8;
	og >>= 8;
	ob >>= 8;

	return or | (og<<8) | ((uint32_t)ob<<16);
}

const unsigned char rands[] = {
	0x67, 0xc6, 0x69, 0x73, 0x51, 0xff, 0x4a, 0xec, 0x29, 0xcd, 0xba, 0xab, 0xf2, 0xfb, 0xe3, 0x46, 
	0x7c, 0xc2, 0x54, 0xf8, 0x1b, 0xe8, 0xe7, 0x8d, 0x76, 0x5a, 0x2e, 0x63, 0x33, 0x9f, 0xc9, 0x9a, 
	0x66, 0x32, 0x0d, 0xb7, 0x31, 0x58, 0xa3, 0x5a, 0x25, 0x5d, 0x05, 0x17, 0x58, 0xe9, 0x5e, 0xd4, 
	0xab, 0xb2, 0xcd, 0xc6, 0x9b, 0xb4, 0x54, 0x11, 0x0e, 0x82, 0x74, 0x41, 0x21, 0x3d, 0xdc, 0x87, 
	0x70, 0xe9, 0x3e, 0xa1, 0x41, 0xe1, 0xfc, 0x67, 0x3e, 0x01, 0x7e, 0x97, 0xea, 0xdc, 0x6b, 0x96, 
	0x8f, 0x38, 0x5c, 0x2a, 0xec, 0xb0, 0x3b, 0xfb, 0x32, 0xaf, 0x3c, 0x54, 0xec, 0x18, 0xdb, 0x5c, 
	0x02, 0x1a, 0xfe, 0x43, 0xfb, 0xfa, 0xaa, 0x3a, 0xfb, 0x29, 0xd1, 0xe6, 0x05, 0x3c, 0x7c, 0x94, 
	0x75, 0xd8, 0xbe, 0x61, 0x89, 0xf9, 0x5c, 0xbb, 0xa8, 0x99, 0x0f, 0x95, 0xb1, 0xeb, 0xf1, 0xb3, 
	0x05, 0xef, 0xf7, 0x00, 0xe9, 0xa1, 0x3a, 0xe5, 0xca, 0x0b, 0xcb, 0xd0, 0x48, 0x47, 0x64, 0xbd, 
	0x1f, 0x23, 0x1e, 0xa8, 0x1c, 0x7b, 0x64, 0xc5, 0x14, 0x73, 0x5a, 0xc5, 0x5e, 0x4b, 0x79, 0x63, 
	0x3b, 0x70, 0x64, 0x24, 0x11, 0x9e, 0x09, 0xdc, 0xaa, 0xd4, 0xac, 0xf2, 0x1b, 0x10, 0xaf, 0x3b, 
	0x33, 0xcd, 0xe3, 0x50, 0x48, 0x47, 0x15, 0x5c, 0xbb, 0x6f, 0x22, 0x19, 0xba, 0x9b, 0x7d, 0xf5, 
	0x0b, 0xe1, 0x1a, 0x1c, 0x7f, 0x23, 0xf8, 0x29, 0xf8, 0xa4, 0x1b, 0x13, 0xb5, 0xca, 0x4e, 0xe8, 
	0x98, 0x32, 0x38, 0xe0, 0x79, 0x4d, 0x3d, 0x34, 0xbc, 0x5f, 0x4e, 0x77, 0xfa, 0xcb, 0x6c, 0x05, 
	0xac, 0x86, 0x21, 0x2b, 0xaa, 0x1a, 0x55, 0xa2, 0xbe, 0x70, 0xb5, 0x73, 0x3b, 0x04, 0x5c, 0xd3, 
	0x36, 0x94, 0xb3, 0xaf, 0xe2, 0xf0, 0xe4, 0x9e, 0x4f, 0x32, 0x15, 0x49, 0xfd, 0x82, 0x4e, 0xa9, };


#define NR_LEDS 90
uint16_t bufferset[12+NR_LEDS*6+1];

// Note first 2 LEDs of DMA Buffer are 0's as a "break"
// Need one extra LED at end to leave line high. 
#define DMALEDS 16
#define DMA_BUFFER_LEN (16*6)

void EncodeLED( uint16_t * ptr, uint32_t ledval24bit )
{
	// This encodes 4 bits of incoming signal into one uint16_t
	const static uint16_t bitquartets[16] = {
		0b1000100010001000, 0b1000100010001110, 0b1000100011101000, 0b1000100011101110,
		0b1000111010001000, 0b1000111010001110, 0b1000111011101000, 0b1000111011101110,
		0b1110100010001000, 0b1110100010001110, 0b1110100011101000, 0b1110100011101110,
		0b1110111010001000, 0b1110111010001110, 0b1110111011101000, 0b1110111011101110, };
	ptr[0] = bitquartets[(ledval24bit>>20)&0xf];
	ptr[1] = bitquartets[(ledval24bit>>16)&0xf];
	ptr[2] = bitquartets[(ledval24bit>>12)&0xf];
	ptr[3] = bitquartets[(ledval24bit>>8)&0xf];
	ptr[4] = bitquartets[(ledval24bit>>4)&0xf];
	ptr[5] = bitquartets[(ledval24bit>>0)&0xf];
} 

void DMA1_Channel3_IRQHandler( void ) __attribute__((interrupt));
void DMA1_Channel3_IRQHandler( void ) 
{
	// Check DMA1->INTFR 
	// Clear all possible flags.
	GPIOD->BSHR = 1;	 // Turn on GPIOD0
	DMA1->INTFCR = DMA1_IT_GL3;
	GPIOD->BSHR = 1<<16; // Turn off GPIOD0
}

void ConfigSpeedyLED()
{
	// Enable DMA + Peripherals
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1;

	// MOSI, Configure GPIO Pin
	GPIOC->CFGLR &= ~(0xf<<(4*6));
	GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF)<<(4*6);

	// Configure SPI 
	SPI1->CTLR1 = 
		SPI_NSS_Soft | SPI_CPHA_1Edge | SPI_CPOL_Low | SPI_DataSize_16b |
		SPI_Mode_Master | SPI_Direction_1Line_Tx |
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
		DMA_Mode_Normal | // OR DMA_Mode_Circular or DMA_Mode_Normal
		DMA_DIR_PeripheralDST |
		DMA_IT_TC | DMA_IT_HT; // Transmission Complete + Half Empty Interrupts. 

//	NVIC_SetPriority( DMA1_Channel3_IRQn, 0<<4 ); // Regular priority.
	NVIC_EnableIRQ( DMA1_Channel3_IRQn );
	DMA1_Channel3->CFGR |= DMA_CFGR1_EN;
}

int main()
{

	// Enable GPIOD.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	ConfigSpeedyLED();

	printf( "CFG: %08x\n", SPI1->CTLR1 );

	uint16_t phases[NR_LEDS];

/*
#define NR_LEDS 30
uint16_t bufferset[4+30*6+1];
*/
	int frame = 0;

	while(1)
	{
		int k;
		Delay_Ms( 10 );
		frame++;

		// 50 us low pulse.
		bufferset[0] = 0x00;
		bufferset[1] = 0x00;
		bufferset[2] = 0x00;
		bufferset[3] = 0x00;
		bufferset[4] = 0x00;
		bufferset[5] = 0x00;
		bufferset[6] = 0x00;
		bufferset[7] = 0x00;
		bufferset[8] = 0x00;
		bufferset[9] = 0x00;
		bufferset[10] = 0x00;
		bufferset[11] = 0x00;

		uint16_t * ept = bufferset + 12;

		//Pre-compute 'whiteness'
		for( k = 0; k < NR_LEDS; k++ )
		{
			int phase = phases[k] += (((rands[k&0xff])+0xf)<<2) + (((rands[k&0xff])+0xf)<<1);

			EncodeLED( ept, k );
			ept += 6;
/*			uint8_t rs = ST(index)>>3;
			SEND_WS( HUE(rs ) );
			index = (buffer1[l])>>8;
			SEND_WS( HUE(rs + 30) );
			SEND_WS( HUE(rs + 190) );

			SEND_WS( HUE(rs ) );
			SEND_WS( HUE(rs + 30) );
			l++;
			SEND_WS( HUE(rs + 190) );*/
		}
		ept[0] = 0;//0xffff;

		DMA1_Channel3->CNTR  = sizeof( bufferset )/2; // Number of unique copies.
	}
}

