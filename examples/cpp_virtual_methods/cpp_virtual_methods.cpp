/*
 * Example for using virtual methods in C++
 * 05/21/2023 A. Mandera
 */

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"
#include "example.h"
#include <stdio.h>

int main() {
	SystemInit48HSI();

	// Setup UART @ 115200 baud
	SetupUART(UART_BRR);
	Delay_Ms(100);

	printf("Begin example\n");

	// Initialize variable in example class
	Example.begin();

	while (true) {
		Example.doPrint(10);
		Delay_Ms(1000);
	}
}