/******************************************************************************
* lib_uart
* A simple but full-featured library for UART on the CH32V003
*
* See GitHub for details: https://github.com/ADBeta/CH32V003_lib_uart
* lib_uart is under the MIT License. See LICENSE for more information
*
* Copyright (c) 2024 ADBeta    Version 2.0.0
******************************************************************************/
#ifndef LIB_UART_H
#define LIB_UART_H

#include "ch32v003fun.h"
#include <stddef.h>

/*** Configuration Flags *****************************************************/
// Enable or disable the RX Ring Buffer. When Disabled, read() will read
// directly from the DATAR Register, using a timeout.
#define RING_BUFFER_ENABLE
//#define RING_BUFFER_DISABLE

// Enable or disable overwriting of the UART Ring Buffer. Rejects new bytes
// when disabled
#define RING_BUFFER_OVERWRITE
#define RING_BUFFER_SIZE 128

// If the ring buffer is disabled, use a timeout of variable (milliseconds)
#define READ_TIMEOUT_MS 100

/*** Macro Functions *********************************************************/
#define IS_POWER_OF_2(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))

/*** Configuration ***********************************************************/
// Make sure only one ring buffer setting is selected
#if defined(RING_BUFFER_ENABLE) && defined(RING_BUFFER_DISABLE)
	#error "CONFIG ERROR: Ring Buffer is enabled and disabled simultaniously"
#endif
// Make sure at least one ring buffer is selected
#if !defined(RING_BUFFER_ENABLE) && !defined(RING_BUFFER_DISABLE)
	#error "CONFIG ERROR: Must define one of RING_BUFFER_ENABLE or RING_BUFFER_DISABLE"
#endif

/*** Ring Buffer Enabled ***/
// If the Ring Buffer is enabled, configure it
#ifdef RING_BUFFER_ENABLE

// Make sure the size of the buffer is not 0, and is a power of 2
#if !IS_POWER_OF_2(RING_BUFFER_SIZE)
	#error "CONFIG ERROR: Ring Buffer Size must be a Power of 2"
#endif

// Create the buffer, head/tail, reset mask and other values
static const size_t uart_ring_mask = RING_BUFFER_SIZE - 1;
static uint8_t      uart_ring_buff[RING_BUFFER_SIZE];
static size_t       uart_ring_head = 0;
static size_t       uart_ring_tail = 0;


/*** Typedefs and structures *************************************************/
/// @brief UART Error Values
typedef enum {
	UART_OK              = 0,
	UART_TIMEOUT,
	UART_INVALID_ARGS,
	UART_BUFFER_EMPTY,
} uart_err_t;

/// @brief Defines some commonly used baud rates
/// DIV = (HCLK / (16 * BAUD)) * 16 (HCLK is 48MHz)
typedef enum {
	UART_BAUD_921600 = ((uint16_t)0x0034),    // 923076    0.16% Fast
	UART_BAUD_460800 = ((uint16_t)0x0068),    // 461538    0.16% Fast
	UART_BAUD_230400 = ((uint16_t)0x00D3),    // 227488    1.27% Fast
	UART_BAUD_115200 = ((uint16_t)0x01A1),    // 115107    0.08% Slow
	UART_BAUD_57600  = ((uint16_t)0x0341),    // 57623     0.04% Fast
	UART_BAUD_38400  = ((uint16_t)0x04E2),    // 38400     0.00% 
	UART_BAUD_19200  = ((uint16_t)0x09C4),    // 19200     0.00%
	UART_BAUD_9600   = ((uint16_t)0x1388),    // 9600      0.00%
	UART_BAUD_4800   = ((uint16_t)0x2710),    // 4800      0.00%
	UART_BAUD_2400   = ((uint16_t)0x4E20),    // 2400      0.00%
	UART_BAUD_1200   = ((uint16_t)0x9C40),    // 1200      0.00%
} uart_baudrate_t;

/// @brief UART Word Length Enum
typedef enum {
	UART_WORDLENGTH_8 = ((uint16_t)0x0000),
	UART_WORDLENGTH_9 = ((uint16_t)0x1000),
} uart_wordlength_t;

/// @brief UART Parity Enum
typedef enum {
	UART_PARITY_NONE = ((uint16_t)0x0000),
	UART_PARITY_EVEN = ((uint16_t)0x0400),
	UART_PARITY_ODD  = ((uint16_t)0x0600),
} uart_parity_t;

/// @brief UART Stop Bits Enum
typedef enum {
	UART_STOPBITS_ONE       = ((uint16_t)0x0000),
	UART_STOPBITS_HALF      = ((uint16_t)0x1000),
	UART_STOPBITS_TWO       = ((uint16_t)0x2000),
	UART_STOPBITS_ONE_HALF  = ((uint16_t)0x3000),
} uart_stopbits_t;


// TODO: Control Bits selection


// define and declare the UART IRQ Function
/// @brief UART Receiver Interrupt handler - Puts the data received into the
/// UART Ring Buffer
/// @param None
/// @return None
void USART1_IRQHandler(void) __attribute__((interrupt));
void USART1_IRQHandler(void)
{
	// Read from the DATAR Register to reset the flag
	uint8_t recv = (uint8_t)USART1->DATAR;

	// Calculate the next write position
	size_t next_head = (uart_ring_head + 1) & uart_ring_mask;

	// If the next position is the same as the tail, either reject the new data
	// or overwrite old data
	if(next_head == uart_ring_tail) 
	{
		#ifdef RING_BUFFER_OVERWRITE
			// Increment the tail position
			uart_ring_tail = (uart_ring_tail + 1) & uart_ring_mask;
		#else
			// Reject any data that overfills the buffer
			return;
		#endif
	}

	// Add the received data to the current head position
	uart_ring_buff[uart_ring_head] = recv;
	// Update the head position
	uart_ring_head = next_head;
}

#endif




/*** Initialisers ************************************************************/
/// @brief Initiliase the UART peripheral with the passed configuratiion.
/// Uses the default pins (PD5-TX  PD6-RX)
///
/// NOTE: if -buffer- is NULL, or -buffsize- is 0, the driver will fall-back
/// into realtime mode, where data is only read when requested - any data
/// received before the function is called will be lost.
///
/// @param buffer, the uint8_t buffer to use as an RX buffer (Optional)
/// @param buffsize, the size of the RX  Buffer in bytes. 
/// @param baud, buadrate of the interface (921600 - 1200)
/// @param wordlength, interface word length (8 or 9 bits)
/// @param parity, Parity variable (None, Even or Odd)
/// @param stopbits, how many stop bits to transmit (0.5, 1, 2, 1.5)
/// @param config, the uart_config_t configuration struct
/// @return uart_err_t status
static void uart_init(
	const uart_baudrate_t baud,
	const uart_wordlength_t wordlength,
	const uart_parity_t parity,
	const uart_stopbits_t stopbits)
{
	// Enable GPIOD and UART1 Clock
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1;

	// Set the RX and TX Pins on PORTD. RX INPUT_FLOATING, TX 10MHz PP AF
	GPIOD->CFGLR &= ~((0x0F << (4*6)) | (0x0F << (4*5)));  // Clear PD6 & PD5
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4*5); // PD5 TX
	GPIOD->CFGLR |= (GPIO_CNF_IN_FLOATING << (4*6));                  // PD6 RX
	
	// Set CTLR1 Register (Enable RX & TX, set Word Length and Parity)
	USART1->CTLR1 = USART_Mode_Tx | USART_Mode_Rx | wordlength | parity;
	// Set CTLR2 Register (Stopbits)
	USART1->CTLR2 = stopbits;
	// Set CTLR3 Register TODO: Interrupts and flow control
	USART1->CTLR3 = (uint16_t)0x0000;

	// Set the Baudrate, assuming 48KHz
	USART1->BRR = baud;

	// If the Ring Buffer is enabled, enable the UART RXNE Interrupt
	#ifdef RING_BUFFER_ENABLE
	USART1->CTLR1 |= USART_CTLR1_RXNEIE;
	NVIC_EnableIRQ(USART1_IRQn);
	#endif

	// Enable the UART
	USART1->CTLR1 |= CTLR1_UE_Set;
}
/*** Write *******************************************************************/
/// @brief writes raw bytes to the UART
/// @param buffer, raw buffer, can be any type
/// @param size, number of bytes to transmit.
/// @return uart_err_t status
static uart_err_t uart_write(const void *buffer, size_t size)
{
	if(buffer == NULL || size == 0) return UART_INVALID_ARGS;

	// Cast the input to a uint8_t
	const uint8_t *bytes = (const uint8_t *)buffer;
	// Send each byte
	while(size--)
	{
		// Wait for the current transmission to finish
		while(!(USART1->STATR & USART_FLAG_TC));
		USART1->DATAR = *bytes++;
	}

	return UART_OK;
}


/// @brief Prints a string to the UART, without any added ternination
/// @param string, input c string to print
/// @return uart_err_t status
static uart_err_t uart_print(const char *string)
{
	if(string == NULL) return UART_INVALID_ARGS;
	
	while(*string != '\0')
	{
		// Wait for the current transmission to finish
		while(!(USART1->STATR & USART_FLAG_TC));
		USART1->DATAR = *string++;
	}

	return UART_OK;
}


/// @brief Prints a string to the UART, and adds termination \r\n characters
/// @param string, input c string to print
/// @return uart_err_t status
static uart_err_t uart_println(const char *string)
{
	if(string == NULL) return UART_INVALID_ARGS;

	uart_err_t str_err = uart_print(string);
	if(str_err != UART_OK) return str_err;

	// Print the terminating characters
	while(!(USART1->STATR & USART_FLAG_TC));
	USART1->DATAR = '\r';
	while(!(USART1->STATR & USART_FLAG_TC));
	USART1->DATAR = '\n';

	return UART_OK;
}


/** Read *********************************************************************/
#ifdef RING_BUFFER_ENABLE
/// @brief reads len number of bytes from the RX Ring Buffer. 
/// Ring Buffer method is only enabled when RING_BUFFER_ENABLE is deinfed.
/// @param *buffer, the buffer to read to
/// @param len, the maximum number of bytes to read to the buffer
/// @return size_t number of bytes read
static size_t uart_read(uint8_t *buffer, size_t len)
{
	// Make sure the buffer passed and length are valid
	if(buffer == NULL || len == 0) return 0;

	size_t bytes_read = 0;
	while(len--)
	{
		// If the buffer has no more data, return how many bytes were read
		if(uart_ring_head == uart_ring_tail) break; 
		
		// Add the current tail byte to the buffer
		*buffer++ = uart_ring_buff[uart_ring_tail];
		// Increment the ring buffer tail position
		uart_ring_tail = (uart_ring_tail + 1) & uart_ring_mask;
		// Increment the count of bytes
		bytes_read++;
	}

	return bytes_read;
}

/// @brief reads from the RX Ring Buffer until it finds a newline delimiter
/// (\n or \r) then a non-delim char, or until it has read -len- bytes.
/// Ring Buffer method is only enabled when RING_BUFFER_ENABLE is defined.
/// @param *buffer, the buffer to read to
/// @param len, the maximum number of bytes to read to the buffer
/// @return size_t number of bytes read
/*
static size_t uart_readln(uint8_t *buffer, size_t len)
{
}
*/
#endif


#ifdef RING_BUFFER_DISABLE
/// @brief reads len number of bytes in realtime, until buffer is full, or
/// the timeout is reached.
/// @param *buffer, the buffer to read to
/// @param len, the maximum number of bytes to read to the buffer
/// @return size_t number of bytes read
static size_t uart_read(uint8_t *buffer, size_t len)
{
	// Make sure the buffer passed and length are valid
	if(buffer == NULL || len == 0) return 0;

	size_t bytes_read = 0;
	while(len--)
	{
		// TODO: Use systick or current_millis to timeout
		// Wait for a byte to be in the buffer. If it exceeds timeout, 
		// exit the function. Uses a poor timout method for now. Will fix when
		// there is a current_millis() fumction.
		uint32_t timeout_ticks = 0;
		while(!(USART1->STATR & USART_FLAG_RXNE))
		{
			if(timeout_ticks++ == READ_TIMEOUT_MS) return bytes_read;
			Delay_Ms(1);
		}

		*buffer++ = (uint8_t)USART1->DATAR;
		// Increment the count of bytes
		bytes_read++;
	}

	return bytes_read;
}

/// @brief reads from the RX Ring Buffer until it finds a newline delimiter
/// (\n or \r) then a non-delim char, or until it has read -len- bytes.
/// Ring Buffer method is only enabled when RING_BUFFER_ENABLE is defined.
/// @param *buffer, the buffer to read to
/// @param len, the maximum number of bytes to read to the buffer
/// @return size_t number of bytes read
/*
static size_t uart_readln(uint8_t *buffer, size_t len)
{
}
*/
#endif


#endif
