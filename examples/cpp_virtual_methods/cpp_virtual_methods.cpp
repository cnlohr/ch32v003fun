/*
 * Example for using virtual methods in C++
 * 05/21/2023 A. Mandera
 */

#include "ch32v003fun.h"
#include "example.h"
#include <stdio.h>

int main() {
	SystemInit();

	Delay_Ms(100);

	printf("Begin example\n");

	// Initialize variable in example class
	Example.begin();

	while (true) {
		Example.doPrint(10);
		Delay_Ms(1000);
	}
}
