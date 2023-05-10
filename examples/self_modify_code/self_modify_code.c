/* Small example showing how to use the SWIO programming pin to 
   do printf through the debug interface */

#define SYSTEM_CORE_CLOCK 24000000
#include "ch32v003fun.h"
#include <stdio.h>

uint32_t count;


// This is a complicated way to do it from C land, as a demonstration

// Tell the compiler to put this code in the .srodata section.  That
// will cause the startup code to copy it from flash into RAM where
// it can be easily modified at runtime.
uint32_t ReadCSRSelfModify( uint16_t whichcsr ) __attribute__(( section(".srodata"))) __attribute__((noinline));
uint32_t ReadCSRSelfModify( uint16_t whichcsr )
{
	uint32_t ret;

	// Tricky: GCC will make this variable "point to" the opcode
	// of the csrr instruction below.
	volatile extern uint32_t readCSRLabel;

	// We have to put this here to "force" the compiler to order the
	// instructions in this way.  Otherwise, the compiler will try
	// to optimize the code and inline the assembly into something where
	// our global handle into assembly code becomes meaningless.
	// Annoyingly, it has to contain at least one instruction :(
	asm volatile( "nop" );

	// 000026f3 is csrrs a3, 0x000, x0; So, we modify it, placing the
	// CSR we want to read in the top 12 bits of the instruction.

	readCSRLabel = 0x000026f3 | (whichcsr << 20);

	// The actual assembly block inserted into the C function.  This
	// defines the local label, globally, so the linker will be able to
	// pick it up.  We also need to used a fixed register, a3, so we
	// can know what opcode we want to use, then we can let C tell us
	// what register it would like the value in.
	//
	// The fence is needed to make sure the CPU knows to not use
	// cached instructions.
	//
	// The constraints are "ret" is a "write" register, and register a3
	// is going to be clobbered by the assembly code.
	asm volatile( 
#if __GNUC__ > 10
		".option arch, +zicsr\n"
#endif
		".global readCSRLabel   \n"
		"	fence               \n"
		"readCSRLabel:          \n"
		"	csrrs a3, 0x000, x0 \n"
		"	addi %[ret], a3, 0  \n"
		 : [ret]"=r"(ret) : : "a3" );

	return ret;
}


uint32_t ReadCSRSelfModifySimple( uint16_t whichcsr ) __attribute__(( section(".data"))) __attribute__((noinline));
uint32_t ReadCSRSelfModifySimple( uint16_t whichcsr )
{
	uint32_t ret;
	uint32_t csrcmd = 0x000026f3 | ( whichcsr << 20);
	asm volatile( 
#if __GNUC__ > 10
		".option arch, +zicsr\n"
#endif
		".global readCSRLabel   \n"
		"   la a3, readCSRLabel \n"
		"   sw %[csrcmd], 0(a3) \n"
		"   fence               \n"
		"readCSRLabel:          \n"
		"	csrrs a3, 0x000, x0 \n"
		"	addi %[ret], a3, 0  \n"
		 : [ret]"=r"(ret) : [csrcmd]"r"(csrcmd) : "a3" );

	return ret;
}


int main()
{
	SystemInit48HSI();
	SetupDebugPrintf();

	WaitForDebuggerToAttach();

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;
	puts( "Print all non-zero CSRs:" );
	int i;
	for( i = 0x000; i < 0x1000; i++ )
	{
		uint32_t rv =  ReadCSRSelfModifySimple( i );
		if( rv )
			printf( "%03x = %08lx\n", i, rv );
	}
	printf( "Done\n" );
	for(;;);
}

