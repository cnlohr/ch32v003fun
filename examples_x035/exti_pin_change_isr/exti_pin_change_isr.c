#include "ch32fun.h"

void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void )
{
    funDigitalWrite( PC14, !funDigitalRead(PC7));

    // Acknowledge the interrupt
    EXTI->INTFR = EXTI_Line7;
}

int main()
{
    SystemInit();

    // Enable GPIOs
    funGpioInitAll();

    // GPIO C7 for input pin change.
    funPinMode(PC7, GPIO_CFGLR_IN_PUPD);
    funDigitalWrite(PC7, FUN_HIGH);

    // GPIO C14 Push-Pull (our output)
    funPinMode(PC14,  GPIO_CFGLR_OUT_10Mhz_PP);

    // Configure the IO as an interrupt.
    AFIO->EXTICR1 = AFIO_EXTICR1_EXTI7_PC;
    EXTI->INTENR = EXTI_INTENR_MR7; // Enable EXT7
    EXTI->RTENR = EXTI_RTENR_TR7;  // Rising edge trigger
    EXTI->FTENR = EXTI_FTENR_TR7;  // Falling edge trigger

    // enable interrupt
    NVIC_EnableIRQ(EXTI7_0_IRQn);

    while(1)
    {
        asm volatile( "nop" );
    }
}
