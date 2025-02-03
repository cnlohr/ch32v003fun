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
//
//  Interrupt in Flash:
//     Default behavior is 133ns
//     Using HPE interrupts, 117ns (FUNCONF_ENABLE_HPE = 1)
//     Naked (With or without HPE): 88ns
//     Using the VTF + naked + (with or without HPE): 74ns
//  Interrupt in RAM, Using Naked VTF, no HPE.
//     Running from RAM about 81ns.
//
// Conclusion: 
//   Using the HPE shaves off 3 cycles.
//   Using the Naked interrupts saves about 4 cycles
//   Using the Vector-Table-Free Interrupt (VTF) System saves about 2 cycles.
//   Running from RAM adds about 1 cycle. 
//   Overall, you can reply to interrupts in about 10-11 cycles, depending on when the interrupt falls.

#include "ch32fun.h"
#include <stdio.h>

volatile int ok = 0;

#if INTERRUPT_IN_RAM
#define EXTRA_INTERRUPT_DECORATOR __attribute__((section(".sdata2.interrupts")))
#else
#define EXTRA_INTERRUPT_DECORATOR
#endif

#if NAKED_TEST

register volatile unsigned global_spare asm ("s6");
register volatile void * global_ok_addr asm ("s7");
register volatile void * global_GPIOA asm ("s8");
register volatile void * global_EXTI asm ("s9");
register volatile unsigned global_APIN asm ("s10");
register volatile unsigned global_EXITPIN asm ("s11");

void EXTI4_IRQHandler( void )	__attribute__((naked)) EXTRA_INTERRUPT_DECORATOR;

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

void EXTI4_IRQHandler( void ) INTERRUPT_DECORATOR EXTRA_INTERRUPT_DECORATOR;

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

#if OVERCLOCK
	EXTEND->CTR = ( EXTEND->CTR & (~(3<<10)) ) | 3<<10; // Turning it "to 1.0V according to datasheet" seems to make it most reliable.

	// Switch processor back to HSI so we don't eat dirt.
	RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_HSI;

	// Keep the HSI on but turn on the HSE.
	RCC->CTLR = RCC_HSION | RCC_HSEON;

	// It's soemthing like this: https://cnlohr.github.io/microclockoptimizer/?chipSelect=ch32vx05_7%2Cd8w&HSI=1,8&HSE=0,8&PREDIV2=1,0&PLL2CLK=1,7&PLL2VCO=0,144&PLL3CLK=0,1&PLL3VCO=0,200&PREDIV1SRC=1,0&PREDIV1=1,2&PLLSRC=1,0&PLL=1,4&PLLVCO=0,144&SYSCLK=1,2&
	// Assuming PLL_MUL_REG = 4 (so it's not overclocking)

	// Setup clock tree.
	RCC->CFGR2 |= 
		(0<<RCC_PREDIV2_OFFSET) | // PREDIV = not div 2, but really it is. Prediv Freq = 4MHz
		(6<<RCC_PLL3MUL_OFFSET) | // PLL3 = x12.5 (NOT USED)
		(7<<RCC_PLL2MUL_OFFSET) | // PLL2 = x9 (PLL2 = 36MHz) TBA
		(2<<RCC_PREDIV1_OFFSET) | // PREDIV1 = /3; Prediv freq = 12MHz
		RCC_PREDIV1_Source_PLL2 | // Use PLL2 to feed PLL.
		0;


	// PLL = x18 (0 in register)
	//  4 in register = x6 (or 144MHz)
	//  7 in register = x9 (or 216MHz)
	// Going above 7 causes the PLL not to kock, so I recommend keeping it at 6 or below (192MHz)
	// This is how we most tightly control overclocking.
#define PLL_MUL_REG 6
	RCC->CFGR0 = ( RCC->CFGR0 & ~(0xf<<18)) | (PLL_MUL_REG<<18) | RCC_PLLSRC;

	// Power on PLLs
	RCC->CTLR |= RCC_PLL3ON | RCC_PLL2ON;
	int timeout;

	for( timeout = 10000; timeout > 0; timeout--) if (RCC->CTLR & RCC_PLL2RDY) break;
	if( timeout == 0 ) goto lockfail;
	printf( "NEXT\n" );

	RCC->CTLR |= RCC_PLLON;
	for( timeout = 10000; timeout > 0; timeout--) if (RCC->CTLR & RCC_PLLRDY) break;
	if( timeout == 0 ) goto lockfail;

	RCC->CFGR0 = (RCC->CFGR0 & ~RCC_SW) | RCC_SW_PLL;
	printf( "RCC->CTLR  = %08x\n", RCC->CTLR );
	printf( "RCC->CFGR0 = %08x\n", RCC->CFGR0  );

	goto success;
lockfail:
	printf( "FAILED TO LOCK\n" );
success:
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

