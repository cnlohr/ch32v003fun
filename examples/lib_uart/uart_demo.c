/******************************************************************************
* UART Library Tester program
* lib_uart is under the MIT License. See LICENSE for more information
*
* ADBeta (c)	12 Aug 2024
******************************************************************************/
#include "ch32v003fun.h"
#include "lib_uart.h"
#include <string.h>
#include <stdio.h>

// The internal ring buffer is 128 bytes by default. See `lib_uart.h` to adjust
// this value and change other settings.
char uart_buffer[128] = {0};

int main()
{
	SystemInit();

	// Init the UART system. See `lib_uart.h` for baud, and other config vars
	uart_init(
		UART_BAUD_115200,
		UART_WORDLENGTH_8, 
		UART_PARITY_NONE,
		UART_STOPBITS_ONE
	);

	// Simple string printing example
	uart_print("This string will be one line 1");
	uart_println(" -- This string will be on the same line");
	uart_println("This string will be on its own line");

	while(1)
	{
		// Clear the read buffer
		memset(uart_buffer, 0x00, 128);

		// Read up to 128 bytes into the UART Buffer.
		// Returns the number of bytes actualy read
		size_t bytes_read = uart_read((uint8_t *)uart_buffer, 128);

		// Only print/modify data if there was some read
		if(bytes_read != 0)
		{
			// Replace any \r with \r\n.
			// NOTE: This WILL corrupt data. It is only to allow the user to 
			// type normally and have newlines
			for(uint8_t chr = 0; chr < 127; chr++)
			{
				if(uart_buffer[chr] == '\r') uart_buffer[chr + 1] = '\n';
			}

			// Write the number of bytes read to the UART
			uart_write((uint8_t *)uart_buffer, 128);
		}
	}  
	
}
