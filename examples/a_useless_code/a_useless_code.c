/*
 * Example of a useless code. It doesn't do anything good except being great at what it does.
 * It demonstrates the power of repetition: if you repeat the samething many times over, it becomes you.
 * by unicab369
 */

#include "ch32v003fun.h"
#include <stdio.h>

int main()
{
	SystemInit();

	while(1)
	{
		printf("\nI'M USELESS");

      // Give the user's brain sometime to process
		Delay_Ms( 2000 );
	}
}
