#include "ch32v003fun.h"
#include "i2c_slave.h"
#include <stdio.h>

// The I2C slave library uses a one byte address so you can extend the size of this array up to 256 registers
// note that the register set is modified by interrupts, to prevent the compiler from accidently optimizing stuff
// away make sure to declare the register array volatile

volatile uint8_t i2c_registers[32] = {0x00};

void onWrite(uint8_t reg, uint8_t length) {
    funDigitalWrite(PA2, i2c_registers[0] & 1);
}

int main() {
    SystemInit();
    funGpioInitAll();

    // Initialize I2C slave
    funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
    funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL
    SetupI2CSlave(0x9, i2c_registers, sizeof(i2c_registers), onWrite, NULL, false);

    // Initialize LED
    funPinMode(PA2, GPIO_CFGLR_OUT_10Mhz_PP); // LED

    while (1) {} // Do not let main exit, you can do other things here
}
