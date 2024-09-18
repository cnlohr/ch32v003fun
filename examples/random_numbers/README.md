(Psuedo) Random Number Generation, using lib_rand

`lib_rand` provides a simple, but effective implementation of a Linear Feedback
Shift Register, to provide 32-bit (Psuedo)Random Number Generation for the 
CH32Vxxx Series of Microcontroller.  
This library provides 3 levels of Random Generation complexity:
* Level 1: Takes the LFSR variable, shifts and taps it then returns it as-is.
This is very fast, and provides basic Random Number Generation
* Level 2: Generates 32 random bits, which make up a 32-bit value. This gives
a larger number of random values before looping and makes patterns less obvious
* Level 3: Generates two 32-bit random values, then XORs them together. Provides
even more values before repeats, and is even harder to spot patterns. Uses more
CPU time.

----
MIT License
Copyright (c) 2024 ADBeta
