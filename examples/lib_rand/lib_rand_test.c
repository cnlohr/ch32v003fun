/******************************************************************************
* Random Number Generator using a Linear Feedback Shift Register - Demo Program
* See the GutHub for more information:
* https://github.com/ADBeta/CH32V003_lib_rand
*
* Ver 1.0    08 Sep 2024
* 
* Released under the MIT Licence
* Copyright ADBeta (c) 2024
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to
* deal in the Software without restriction, including without limitation the 
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
* sell copies of the Software, and to permit persons to whom the Software is 
* furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in 
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE 
* USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/
#include "ch32v003fun.h"
#include "lib_rand.h"

#include <stdio.h>

// NOTE: RANDOM_STRENGTH is defined as 2 in funconfig.h. Change this if needed
int main() 
{
	SystemInit();

	// Optional: Seed the Random Number Generator.
	// It is pre-seeded with 0x747AA32F.
	seed(0x12345678);

	// Print 10 Random Numbers
	for(uint8_t x = 0; x < 10; x++)
	{
		printf("Random Number %u: %lu\n", x, rand());
	}


	return 0;
}
