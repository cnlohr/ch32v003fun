;/********************************** (C) COPYRIGHT *******************************
;* File Name          : startup_ch32v00x.s
;* Author             : WCH
;* Version            : V1.0.0
;* Date               : 2022/08/08
;* Description        : vector table for eclipse toolchain.
;*********************************************************************************
;* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
;* Attention: This software (modified or not) and binary are used for 
;* microcontroller manufactured by Nanjing Qinheng Microelectronics.
;*******************************************************************************/

	.section  .init, "ax", @progbits
	.globl  _start
	.align  2
_start:
	.option   norvc;
    j       handle_reset
    .word   0
    .word   NMI_Handler                  /* NMI Handler */
    .word   HardFault_Handler            /* Hard Fault Handler */
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   0
    .word   SysTick_Handler             /* SysTick Handler */
    .word   0
    .word   SW_Handler                  /* SW Handler */
    .word   0
    /* External Interrupts */
    .word   WWDG_IRQHandler         	/* Window Watchdog */
    .word   PVD_IRQHandler          	/* PVD through EXTI Line detect */
    .word   FLASH_IRQHandler        	/* Flash */
    .word   RCC_IRQHandler          	/* RCC */
    .word   EXTI7_0_IRQHandler       	/* EXTI Line 7..0 */
    .word   AWU_IRQHandler              /* AWU */
    .word   DMA1_Channel1_IRQHandler   	/* DMA1 Channel 1 */
    .word   DMA1_Channel2_IRQHandler   	/* DMA1 Channel 2 */
    .word   DMA1_Channel3_IRQHandler   	/* DMA1 Channel 3 */
    .word   DMA1_Channel4_IRQHandler   	/* DMA1 Channel 4 */
    .word   DMA1_Channel5_IRQHandler   	/* DMA1 Channel 5 */
    .word   DMA1_Channel6_IRQHandler   	/* DMA1 Channel 6 */
    .word   DMA1_Channel7_IRQHandler   	/* DMA1 Channel 7 */
    .word   ADC1_IRQHandler          	/* ADC1 */
    .word   I2C1_EV_IRQHandler         	/* I2C1 Event */
    .word   I2C1_ER_IRQHandler         	/* I2C1 Error */
    .word   USART1_IRQHandler          	/* USART1 */
	.word   SPI1_IRQHandler            	/* SPI1 */
	.word   TIM1_BRK_IRQHandler        	/* TIM1 Break */
    .word   TIM1_UP_IRQHandler         	/* TIM1 Update */
    .word   TIM1_TRG_COM_IRQHandler    	/* TIM1 Trigger and Commutation */
    .word   TIM1_CC_IRQHandler         	/* TIM1 Capture Compare */
    .word   TIM2_IRQHandler            	/* TIM2 */

	.option rvc;
	.section  .text.vector_handler, "ax", @progbits
	.weak   NMI_Handler
	.weak   HardFault_Handler
	.weak   SysTick_Handler
	.weak   SW_Handler
	.weak   WWDG_IRQHandler
	.weak   PVD_IRQHandler
	.weak   FLASH_IRQHandler
	.weak   RCC_IRQHandler
	.weak   EXTI7_0_IRQHandler
	.weak   AWU_IRQHandler
	.weak   DMA1_Channel1_IRQHandler
	.weak   DMA1_Channel2_IRQHandler
	.weak   DMA1_Channel3_IRQHandler
	.weak   DMA1_Channel4_IRQHandler
	.weak   DMA1_Channel5_IRQHandler
	.weak   DMA1_Channel6_IRQHandler
	.weak   DMA1_Channel7_IRQHandler
	.weak   ADC1_IRQHandler
	.weak   I2C1_EV_IRQHandler
	.weak   I2C1_ER_IRQHandler
	.weak   USART1_IRQHandler
	.weak   SPI1_IRQHandler
	.weak   TIM1_BRK_IRQHandler
	.weak   TIM1_UP_IRQHandler
	.weak   TIM1_TRG_COM_IRQHandler
	.weak   TIM1_CC_IRQHandler
	.weak   TIM2_IRQHandler

NMI_Handler:              1: j 1b
HardFault_Handler:        1: j 1b
SysTick_Handler:          1: j 1b
SW_Handler:               1: j 1b
WWDG_IRQHandler:          1: j 1b
PVD_IRQHandler:           1: j 1b
FLASH_IRQHandler:         1: j 1b
RCC_IRQHandler:           1: j 1b
EXTI7_0_IRQHandler:       1: j 1b
AWU_IRQHandler:           1: j 1b
DMA1_Channel1_IRQHandler: 1: j 1b
DMA1_Channel2_IRQHandler: 1: j 1b
DMA1_Channel3_IRQHandler: 1: j 1b
DMA1_Channel4_IRQHandler: 1: j 1b
DMA1_Channel5_IRQHandler: 1: j 1b
DMA1_Channel6_IRQHandler: 1: j 1b
DMA1_Channel7_IRQHandler: 1: j 1b
ADC1_IRQHandler:          1: j 1b
I2C1_EV_IRQHandler:       1: j 1b
I2C1_ER_IRQHandler:       1: j 1b
USART1_IRQHandler:        1: j 1b
SPI1_IRQHandler:          1: j 1b
TIM1_BRK_IRQHandler:      1: j 1b
TIM1_UP_IRQHandler:       1: j 1b
TIM1_TRG_COM_IRQHandler:  1: j 1b
TIM1_CC_IRQHandler:       1: j 1b
TIM2_IRQHandler:          1: j 1b


	.section  .text.handle_reset, "ax", @progbits
	.weak     handle_reset
	.align    1
handle_reset:
.option push
.option norelax
	la gp, __global_pointer$
.option pop
1:
	la sp, _eusrstack
2:
	/* Load data section from flash to RAM */
	la a0, _data_lma
	la a1, _data_vma
	la a2, _edata
	bgeu a1, a2, 2f
1:
	lw t0, (a0)
	sw t0, (a1)
	addi a0, a0, 4
	addi a1, a1, 4
	bltu a1, a2, 1b
2:
    /* clear bss section */
    la a0, _sbss
    la a1, _ebss
    bgeu a0, a1, 2f
1:
    sw zero, (a0)
    addi a0, a0, 4
    bltu a0, a1, 1b
2:
    li t0, 0x80
    csrw mstatus, t0
  
    li t0, 0x3
    csrw 0x804, t0
  
    la t0, _start
    ori t0, t0, 3
    csrw mtvec, t0
  
    jal   SystemInit
    la t0, main
    csrw mepc, t0
    mret

