// Interrupt timing test, to see how long it takes to service interrupts.
//
// This expects Pin A4 hard connected to A5, and will signal on A6.
//
//
// PLEASE SEE TEST CONFIGURATION IN funconfig.h !
//
// Genral notes about the NAKED_TEST:
//   This uses fixed register allocation (Please see Makefile for -ffixed-[register]
//   Please note that you can normally access these registers from C, but it becomes ... dangerous.
//   You should write C helper functions to access them.
//
// RESULTS (comparing from line drop on PA5 to line rise on PA6), using Saleae Pro 8, 500MSps, 3.3V
// Default behavior is 133ns
// Using HPE interrupts, 117ns (FUNCONF_ENABLE_HPE = 1)
// Naked (With or without HPE): 88ns
// Using the VTF + naked + (with or without HPE): 74ns
//
// Conclusion: 
//   Using the HPE shaves off 3 cycles.
//   Using the Naked interrupts saves about 4 cycles
//   Using the Vector-Table-Free Interrupt (VTF) System saves about 2 cycles.
//   Overall, you can reply to interrupts in about 10-11 cycles, depending on when the interrupt falls.

#include "ch32v003fun.h"
#include <stdio.h>

volatile int ok = 0;

#if NAKED_TEST

register volatile unsigned global_spare asm ("s6");
register volatile void * global_ok_addr asm ("s7");
register volatile void * global_GPIOA asm ("s8");
register volatile void * global_EXTI asm ("s9");
register volatile unsigned global_APIN asm ("s10");
register volatile unsigned global_EXITPIN asm ("s11");

void EXTI4_IRQHandler( void )	__attribute__((naked));

void EXTI4_IRQHandler( void )
{

	asm volatile( 
"	sw  s10, 0x10(s8) /* 0x10 is the offset of BSHR*/\n\
	sw  s11, 0x14(s9) /* 0x14 is the offset of INTFR*/\n\
	lw  s6, 0(s7)\n\
	addi s6,s6,1\n\
	sw  s6, 0(s7)\n\
	mret\
	" : : : );
}

void SetupRegisters( void )
{
	asm volatile( "\
		mv s7, %[okaddr]\n\
		li s8, %[global_GPIOA]\n\
		li s9, %[global_EXTI]\n\
		li s10, 1<<6\n\
		li s11, 1<<4\n\
	" : : [okaddr]"r"(&ok), [global_GPIOA]"g"(GPIOA), [global_EXTI]"g"(EXTI) );

	/*
		// We can't actually set these reliably because GCC might optimize them out.
		global_spare = 0;     // s6
		global_ok_addr = &ok; // s7
		global_GPIOA = GPIOA; // s8
		global_EXTI = EXTI; // s9
		global_APIN = 1<<6; // s10
		global_EXITPIN = 1<<4; // s11
	*/
}

#else

void EXTI4_IRQHandler( void ) INTERRUPT_DECORATOR;

void EXTI4_IRQHandler( void )
{
	funDigitalWrite( PA6, FUN_HIGH );
	EXTI->INTFR = EXTI_Line4;
	ok++;
#if FUNCONF_ENABLE_HPE
	asm volatile( "\
	mret\
	" : : : );
#endif
}

#endif

int main()
{

	SystemInit();
	funGpioInitAll();
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO;

	funPinMode( PA4, GPIO_CFGLR_IN_FLOAT );
	funPinMode( PA5, GPIO_CFGLR_OUT_50Mhz_PP );
	funPinMode( PA6, GPIO_CFGLR_OUT_50Mhz_PP );
	funDigitalWrite( PA5, FUN_LOW );
	funDigitalWrite( PA6, FUN_LOW );

	// Configure the IO as an interrupt.
	AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI4_PA;
	EXTI->INTENR = EXTI_INTENR_MR4; // Enable EXT4
	EXTI->FTENR = EXTI_FTENR_TR4;  // Rising edge trigger

#if NAKED_TEST
	SetupRegisters();
#endif

#if USE_VTF
	SetVTFIRQ( (uintptr_t)&EXTI4_IRQHandler, EXTI4_IRQn, 0, ENABLE );
#endif

	// enable interrupt
	NVIC_EnableIRQ( EXTI4_IRQn );

	ok = 0;
	printf( "Testing\n" );
	while( 1 )
	{
		printf( "Mark\n" );
		funDigitalWrite( PA5, FUN_LOW );
		printf( "Done %d\n", ok );
		funDigitalWrite( PA6, FUN_LOW );
		funDigitalWrite( PA5, FUN_HIGH );
		Delay_Ms(1);
	}

}

