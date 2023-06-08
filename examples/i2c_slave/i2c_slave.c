#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include "i2c_slave.h"
#include <stdio.h>

// The I2C slave library uses a one byte address so you can extend the size of this array up to 256 registers
// note that the register set is modified by interrupts, to prevent the compiler from accidently optimizing stuff
// away make sure to declare the register array volatile

volatile uint8_t i2c_registers[32] = {0x00};

int main() {
    SystemInit48HSI();
    SetupDebugPrintf();
    SetupI2CSlave(0x9, i2c_registers, sizeof(i2c_registers));

    // Enable GPIOD and set pin 0 to output
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
    GPIOD->CFGLR &= ~(0xf<<(4*0));
    GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

    while (1) {
        if (i2c_registers[0] & 1) { // Turn on LED (PD0) if bit 1 of register 0 is set
            GPIOD-> BSHR |= 1 << 16;
        } else {
            GPIOD-> BSHR |= 1;
        }
    }
}
