/*
	C version of CH32V003 Startup .s file from WCH
	This file is public domain where possible or the following where not:
	Copyright 2023 Charles Lohr, under the MIT-x11 or NewBSD licenses, you choose.
*/

#include <stdint.h>

int main() __attribute__((used));
void SystemInit( void ) __attribute__((used));

void InterruptVector() __attribute__((naked)) __attribute((section(".init"))) __attribute__((used));
void handle_reset() __attribute__((naked)) __attribute((section(".text.handle_reset"))) __attribute__((used));
void DefaultIRQHandler( void ) __attribute__((section(".text.vector_handler"))) __attribute__((naked)) __attribute__((used));

extern uint32_t * _sbss;
extern uint32_t * _ebss;
extern uint32_t * _data_lma;
extern uint32_t * _data_vma;
extern uint32_t * _edata;


// If you don't override a specific handler, it will just spin forever.
void DefaultIRQHandler( void )
{
	// Infinite Loop
	asm volatile( "1: j 1b" );
}

void NMI_Handler( void )                 __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void HardFault_Handler( void )           __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void SysTick_Handler( void )             __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void SW_Handler( void )                  __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void WWDG_IRQHandler( void )             __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void PVD_IRQHandler( void )              __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void FLASH_IRQHandler( void )            __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void RCC_IRQHandler( void )              __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void EXTI7_0_IRQHandler( void )          __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void AWU_IRQHandler( void )              __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel1_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel2_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel3_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel4_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel5_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel6_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void DMA1_Channel7_IRQHandler( void )    __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void ADC1_IRQHandler( void )             __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void I2C1_EV_IRQHandler( void )          __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void I2C1_ER_IRQHandler( void )          __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void USART1_IRQHandler( void )           __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void SPI1_IRQHandler( void )             __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM1_BRK_IRQHandler( void )         __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM1_UP_IRQHandler( void )          __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM1_TRG_COM_IRQHandler( void )     __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM1_CC_IRQHandler( void )          __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));
void TIM2_IRQHandler( void )             __attribute__((section(".text.vector_handler"))) __attribute((weak,alias("DefaultIRQHandler"))) __attribute__((used));

void InterruptVector()
{
	asm volatile( "\n\
	.align  2\n\
	.option   norvc;\n\
	j	   handle_reset\n\
	.word   0\n\
	.word   NMI_Handler				  /* NMI Handler */		\n\
	.word   HardFault_Handler			/* Hard Fault Handler */ \n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   0\n\
	.word   SysTick_Handler			 /* SysTick Handler */	 \n\
	.word   0\n\
	.word   SW_Handler				  /* SW Handler */		  \n\
	.word   0\n\
	/* External Interrupts */									 \n\
	.word   WWDG_IRQHandler		 	/* Window Watchdog */				\n\
	.word   PVD_IRQHandler		  	/* PVD through EXTI Line detect */   \n\
	.word   FLASH_IRQHandler			/* Flash */						  \n\
	.word   RCC_IRQHandler		  	/* RCC */							\n\
	.word   EXTI7_0_IRQHandler	   	/* EXTI Line 7..0 */				 \n\
	.word   AWU_IRQHandler			  /* AWU */							\n\
	.word   DMA1_Channel1_IRQHandler   	/* DMA1 Channel 1 */				 \n\
	.word   DMA1_Channel2_IRQHandler   	/* DMA1 Channel 2 */				 \n\
	.word   DMA1_Channel3_IRQHandler   	/* DMA1 Channel 3 */				 \n\
	.word   DMA1_Channel4_IRQHandler   	/* DMA1 Channel 4 */				 \n\
	.word   DMA1_Channel5_IRQHandler   	/* DMA1 Channel 5 */				 \n\
	.word   DMA1_Channel6_IRQHandler   	/* DMA1 Channel 6 */				 \n\
	.word   DMA1_Channel7_IRQHandler   	/* DMA1 Channel 7 */				 \n\
	.word   ADC1_IRQHandler		  	/* ADC1 */						   \n\
	.word   I2C1_EV_IRQHandler		 	/* I2C1 Event */					 \n\
	.word   I2C1_ER_IRQHandler		 	/* I2C1 Error */					 \n\
	.word   USART1_IRQHandler		  	/* USART1 */						 \n\
	.word   SPI1_IRQHandler				/* SPI1 */						   \n\
	.word   TIM1_BRK_IRQHandler			/* TIM1 Break */					 \n\
	.word   TIM1_UP_IRQHandler		 	/* TIM1 Update */					\n\
	.word   TIM1_TRG_COM_IRQHandler		/* TIM1 Trigger and Commutation */   \n\
	.word   TIM1_CC_IRQHandler		 	/* TIM1 Capture Compare */		   \n\
	.word   TIM2_IRQHandler				/* TIM2 */						   \n" );
}



void handle_reset()
{
	asm volatile( "\n\
.option push\n\
.option norelax\n\
	la gp, __global_pointer$\n\
.option pop\n\
	la sp, _eusrstack\n"

	// Setup the interrupt vector.

"	li t0, 0x80\n\
	csrw mstatus, t0\n\
	li t0, 0x3\n\
	csrw 0x804, t0\n\
	la t0, InterruptVector\n\
	ori t0, t0, 3\n\
	csrw mtvec, t0\n"

	// Clear BSS
	// Has to be in assembly otherwise it will overwrite the stack.
    "la t0, _sbss\n\
    la t1, _ebss\n\
    bgeu t0, t1, 2f\n\
1:\n\
    sw zero, (t0)\n\
    addi t0, t0, 4\n\
    bltu t0, t1, 1b\n\
2:" );

	// Load data section from flash to RAM 
	uint32_t * tempin = _data_lma;
	uint32_t * tempout = _data_vma;
	while( tempout != _edata )
		*(tempout++) = *(tempin++); 

	SystemInit();

	// set mepc to be main as the root interrupt.
asm volatile( "\n\
	la t0, main\n\
	csrw mepc, t0\n\
	mret\n" );
}



