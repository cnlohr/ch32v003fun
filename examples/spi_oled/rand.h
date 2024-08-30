/*
 * rand.h - LFSR-based random number generation
 * Released to Public Domain 08-05-2024 E. Brombaugh
 */
 
/*
 * Common function
 * seed = 0 - advances LFSR state and returns result
 * seed = anything else - set seed to that value
 * bit = 1-32 - number of bits to generate
 */
uint32_t rnd_fun(uint32_t seed, uint8_t bits)
{
	static uint32_t rnd_state = 1;
	uint32_t lsb;
	
	if(seed)
	{
		rnd_state = seed;
		return 0;
	}
	
	while(bits--)
	{
		lsb = rnd_state & 1;
		rnd_state >>= 1;
		if(lsb)
			rnd_state ^= 0x80200003;
	}
	
	return rnd_state;
}

/*
 * stdlib random emulation
 */

#define RAND_MAX 0x7FFFFFFF

/* generates 31 bits PRN */
int rand(void)
{
	return rnd_fun(0,31)&RAND_MAX;
}

/* set seed */
void srand(unsigned int seed)
{
	rnd_fun(seed,0);
}

