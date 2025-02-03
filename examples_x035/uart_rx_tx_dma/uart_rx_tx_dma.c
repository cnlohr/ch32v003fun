/*
 * Serial port echo example using DMA and IDLE interrupts.
 *
 * This example demonstrates how to efficiently echo data received on a UART
 * using DMA for data transfer and IDLE line detection interrupts for
 * identifying the end of a packet.
 *
 * Hardware setup:
 *   - Connect a UART Rx pin to PB10.
 *   - Connect a UART Tx pin to PB11.
 *   - Configure your UART for 115200 baud, 8 data bits, no parity, 1 stop bit (115200n1).
 *
 * Operation:
 *   - The program initializes the UART and DMA peripherals.
 *   - DMA is used to continuously receive data from the UART into a circular buffer.
 *   - The IDLE line detection interrupt signals the end of a data packet.
 *   - The program then echoes the received data back over the UART using DMA.
 */
#include "ch32fun.h"

// Set UART baud rate here
#define UART_BR 115200

// Define your RX buffer and its size
#define RX_BUFFER_SIZE 512
uint8_t rx_buffer[RX_BUFFER_SIZE];

// Variable to store the current RX buffer write position (updated by the IDLE interrupt)
volatile uint16_t rx_buffer_write_pos = 0;

// Variable to store the current RX buffer read position
volatile uint16_t rx_buffer_read_pos = 0;

/**
 * USART1 IDLE interrupt handler.
 */
__attribute__( ( interrupt ) ) __attribute__( ( section( ".srodata" ) ) ) void USART1_IRQHandler( void );
void USART1_IRQHandler( void )
{
	// Check for IDLE interrupt
	if ( USART1->STATR & USART_STATR_IDLE )
	{
		// Clear the IDLE interrupt flag.
		// Source, WCH documentation: reading status and then data register clears the idle bit;
		USART1->STATR;
		USART1->DATAR;

		// Update the position so the rx functions can read the data.
		rx_buffer_write_pos = ( RX_BUFFER_SIZE - DMA1_Channel5->CNTR ) % RX_BUFFER_SIZE;
	}
}

/**
 * Initialize the USART pheripheral
 */
static void uart_setup( void )
{
	// Enable UART and GPIOB
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOB | RCC_APB2Periph_USART1;

	// Setup PB10 as TX
	GPIOB->CFGHR =
		( GPIOB->CFGHR & ~( 0xF << ( 4 * 2 ) ) ) | ( ( GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF ) << ( 4 * 2 ) );

	// Setup PB11 as RX
	GPIOB->CFGHR = ( GPIOB->CFGHR & ~( 0xF << ( 4 * 3 ) ) ) | ( ( GPIO_Speed_In | GPIO_CNF_IN_FLOATING ) << ( 4 * 3 ) );

	// Setup UART for Tx 8n1
	USART1->CTLR1 = USART_WordLength_8b | USART_Parity_No | USART_Mode_Tx | USART_Mode_Rx;
	USART1->CTLR2 = USART_StopBits_1;

	// Enable Tx and Rx DMA event
	USART1->CTLR3 = USART_DMAReq_Tx | USART_DMAReq_Rx | USART_HardwareFlowControl_None;

	// Set baud rate and enable UART
	USART1->BRR = ( ( FUNCONF_SYSTEM_CORE_CLOCK ) + ( UART_BR ) / 2 ) / ( UART_BR );
	USART1->CTLR1 |= CTLR1_UE_Set;
}

/**
 * Initialize the DMA channels for UART transmission and reception
 */
static void dma_uart_dma_setup( void )
{
	// Enable DMA peripheral
	RCC->AHBPCENR = RCC_AHBPeriph_SRAM | RCC_AHBPeriph_DMA1;

	// --- Configure DMA Channel 4 for UART transmission ---

	// Disable channel just in case there is a transfer in progress
	DMA1_Channel4->CFGR &= ~DMA_CFGR1_EN;

	// USART1 TX uses DMA channel 4
	DMA1_Channel4->PADDR = (uint32_t)&USART1->DATAR;

	// MEM2MEM: 0 (memory to peripheral)
	// PL: 0 (low priority since UART is a relatively slow peripheral)
	// MSIZE/PSIZE: 0 (8-bit)
	// MINC: 1 (increase memory address)
	// CIRC: 0 (one shot)
	// DIR: 1 (read from memory)
	// TEIE: 0 (no tx error interrupt)
	// HTIE: 0 (no half tx interrupt)
	// TCIE: 0 (transmission complete interrupt enable)
	// EN: 0 (do not enable DMA yet)
	DMA1_Channel4->CFGR = DMA_CFGR1_MINC | DMA_CFGR1_DIR;

	// --- Configure DMA Channel 5 for UART reception ---

	// Disable channel just in case there is a transfer in progress
	DMA1_Channel5->CFGR &= ~DMA_CFGR1_EN;

	// Set the memory address to the RX buffer
	DMA1_Channel5->MADDR = (uint32_t)rx_buffer;

	// Set the peripheral address to the USART1 data register
	DMA1_Channel5->PADDR = (uint32_t)&USART1->DATAR;

	// Set the number of data to transfer to the size of the RX buffer
	DMA1_Channel5->CNTR = RX_BUFFER_SIZE;

	// MEM2MEM: 0 (memory to peripheral)
	// PL: 0 (low priority)
	// MSIZE/PSIZE: 0 (8-bit)
	// MINC: 1 (increase memory address)
	// CIRC: 1 (circular mode for continuous reception)
	// DIR: 0 (read from peripheral)
	// TEIE: 0 (no tx error interrupt)
	// HTIE: 0 (no half tx interrupt)
	// TCIE: 0 (transmission complete interrupt enable)
	// EN: 0 (do not enable DMA yet)
	DMA1_Channel5->CFGR = DMA_CFGR1_MINC | DMA_CFGR1_CIRC;
}

/**
 * Transmit data over UART using DMA
 */
static void dma_uart_tx( const void *data, uint32_t len )
{
	// Disable DMA channel (just in case a transfer is pending)
	DMA1_Channel4->CFGR &= ~DMA_CFGR1_EN;
	// Set transfer length and source address
	DMA1_Channel4->CNTR = len;
	DMA1_Channel4->MADDR = (uint32_t)data;
	// Enable DMA channel to start the transfer
	DMA1_Channel4->CFGR |= DMA_CFGR1_EN;
}

/**
 * Receive data from UART with timeout
 *
 * Due to the setup this function will block until the line is idle.
 */
static uint16_t dma_uart_rx( uint8_t *data, int len, uint16_t timeout_ms )
{
	uint32_t ticks = Ticks_from_Ms( timeout_ms );
	uint32_t targettime = SysTick->CNTL + ticks;
	uint16_t bytes_read = 0;

	// Read data from the circular buffer until the requested length is reached or timeout occurs
	while ( bytes_read < len && ( (int32_t)( SysTick->CNTL - targettime ) ) < 0 )
	{
		// Check if there is data available in the circular buffer
		if ( rx_buffer_read_pos != rx_buffer_write_pos )
		{
			// Copy a byte from the circular buffer to the output buffer
			data[bytes_read++] = rx_buffer[rx_buffer_read_pos];

			// Update the read position, handling wrap-around
			rx_buffer_read_pos = ( rx_buffer_read_pos + 1 ) % RX_BUFFER_SIZE;
		}
	}

	// Return the number of bytes actually read
	return bytes_read;
}

/**
 * Return the number of bytes available in the RX buffer
 */
static uint16_t dma_uart_available()
{
	return rx_buffer_write_pos - rx_buffer_read_pos;
}

/**
 * Enable UART reception with DMA and IDLE line detection interrupt
 */
static void dma_uart_rx_enable( void )
{
	// Enable DMA channel to start the transfer
	DMA1_Channel5->CFGR |= DMA_CFGR1_EN;

	// Enable DMA reception in USART1
	USART1->CTLR3 |= USART_CTLR3_DMAR;

	// Enable IDLE line detection interrupt
	USART1->CTLR1 |= USART_CTLR1_IDLEIE;
	NVIC_EnableIRQ( USART1_IRQn );
}

int main( void )
{
	SystemInit();

	uart_setup();
	dma_uart_dma_setup();
	dma_uart_rx_enable();

	static const uint8_t message[] = "Echo example\r\n";
	dma_uart_tx( message, sizeof( message ) );

	while ( 1 )
	{
		// Check if data is available in the RX buffer
		int bytes_available = dma_uart_available();
		if ( bytes_available > 0 )
		{
			uint8_t buffer[RX_BUFFER_SIZE + 6] = { 0 };

			buffer[0] = 'E';
			buffer[1] = 'c';
			buffer[2] = 'h';
			buffer[3] = 'o';
			buffer[4] = ':';
			buffer[5] = ' ';

			// Timeout is set to 10ms, but it shouldn't be necessary as data is already available
			int bytes_received = dma_uart_rx( &buffer[6], bytes_available, 10 );

			// Respond with 'Echo: ' + received data
			dma_uart_tx( buffer, bytes_received + 6 );
		}
	}
}
