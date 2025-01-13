/******************************************************************************
* Psuedo Random Number Generator using a Linear Feedback Shift Register
* See the GitHub for more information:
* https://github.com/ADBeta/CH32V003_lib_rand
*
* Ver 1.1    09 Sep 2024
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
#ifndef CH32V003_LIB_RAND
#define CH32V003_LIB_RAND

// Define the strength of the random generation. Do this in funconfig.h
// Strength 1: Tap and shift the LFSR, then returns the LFSR value as is
// Strength 2: Generate 32 random bits using the LFSR
// Strength 3: Genetate two 32bit values using the LFSR, then XOR them together
// Example:    #define RANDOM_STRENGTH 2

#ifndef RANDOM_STRENGTH 
	#error "Error in lib_rand. Must define RANDOM_STRENGTH"
#endif

// @brief set the random LFSR values seed by default to a known-good value
static uint32_t _rand_lfsr = 0x747AA32F;


/*** Library specific Functions - Do Not Use *********************************/
/****************************************************************************/
/// @brief Updates the LFSR by getting a new tap bit, for MSB, then shifting
/// the LFSR >> 1, appending the new MSB.
/// Taps bits 0, 1, 21 and 31
/// @param None
/// @return 0x01 or 0x00, as a LSB translation of the tapped MSB for the LFSR
uint8_t _rand_lfsr_update(void)
{
	// Shifting to MSB to make calculations more efficient later
	uint32_t bit_31 =  _rand_lfsr        & 0x80000000;
	uint32_t bit_21 = (_rand_lfsr << 10) & 0x80000000;
	uint32_t bit_01 = (_rand_lfsr << 30) & 0x80000000;
	uint32_t bit_00 = (_rand_lfsr << 31) & 0x80000000;

	// Calculate the MSB to be put into the LFSR
	uint32_t msb = bit_31 ^ bit_21 ^ bit_01 ^ bit_00;
	// Shift the lfsr and append the MSB to it
	_rand_lfsr = (_rand_lfsr >> 1) | msb;
	// Return the LSB instead of MSB
	return msb >> 31;
}


/// @brief Generates a Random 32-bit number, using the LFSR - by generating
/// a random bit from LFSR taps, 32 times.
/// @param None
/// @return a (psuedo)random 32-bit value
uint32_t _rand_gen_32b(void)
{
	uint32_t rand_out = 0;
	
	uint8_t bits = 32;
	while(bits--)
	{
		// Shift the current rand value for the new LSB
		rand_out = rand_out << 1;
		// Append the LSB
		rand_out |= _rand_lfsr_update();
	}
	
	return rand_out;
}



/// @brief Generates a Random n-bit number, using the LFSR - by generating
/// a random bit from LFSR taps, n times.
/// @param None
/// @return a (psuedo)random n-bit value
uint32_t _rand_gen_nb( int bits)
{
	uint32_t rand_out = 0;

	while(bits--)
	{
		// Shift the current rand value for the new LSB
		rand_out = rand_out << 1;
		// Append the LSB
		rand_out |= _rand_lfsr_update();
	}

	return rand_out;
}


/*** API Functions ***********************************************************/
/*****************************************************************************/
/// @brief seeds the Random LFSR to the value passed
/// @param uint32_t seed
/// @return None
void seed(const uint32_t seed_val)
{
	_rand_lfsr = seed_val;
}


/// @brief Generates a Random (32-bit) Number, based on the RANDOM_STRENGTH
/// you have selected 
/// @param None
/// @return 32bit Random value
uint32_t rand(void)
{
	uint32_t rand_out = 0;

	// If RANDOM_STRENGTH is level 1, Update LFSR Once, then return it
	#if RANDOM_STRENGTH == 1
	// Update the LFSR, discard result, and return _lsfr raw
	(void)_rand_lfsr_update();
	rand_out = _rand_lfsr;
	#endif

	// If RANDOM_STRENGTH is level 2, generate a 32-bit output, using 32 random
	// bits from the LFSR
	#if RANDOM_STRENGTH == 2
	rand_out = _rand_gen_32b();
	#endif

	// If RANDOM_STRENGTH is level 3, generate 2 32-bit outputs, then XOR them
	// together
	#if RANDOM_STRENGTH == 3
	uint32_t rand_a = _rand_gen_32b();
	uint32_t rand_b = _rand_gen_32b();
	rand_out = rand_a ^ rand_b;
	#endif

	return rand_out;
}

#endif
