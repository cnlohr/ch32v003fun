// Include necessary libraries
#include <stdlib.h>

// Initialise the variables for animation
uint8_t flag = 0;
uint32_t count = 1;

// Initialise the variable for the srand seed
uint16_t ADC_val;

// Define an array to buffer the color data for each LED in the strip
uint8_t BUFFER_LEDS[num_leds][3] = {};

// Send a single bit using the GD pinwiggle protocol.
// Check mark/space ratio of the data on GPIO D6 with an oscilloscope (or Logic analyser?).
void LED_SendBit(uint8_t bit)
{
    if (bit) {
    //// Send a 1 bit
        GPIOD->BSHR = 1 << 6; // put pin C4 high and wait for 800nS
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");
        GPIOD->BCR = 1 << 6; // put pin C4 low and exit, 400nS is taken up by other functions
        return;
        }
//    else {
        // Send a 0 bit
        GPIOD->BSHR = 1 << 6; // put pin C4 high and wait for 400nS
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");
        GPIOD->BCR = 1 << 6; // put pin C4 low and wait for 400nS, 400nS is taken up by other functions
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
            __asm__("nop");__asm__("nop");__asm__("nop");__asm__("nop");
//    }
}

// Send a single colour for a single LED
//WS2812B LEDs want 24 bits per led in the string
void LED_SendColour(uint8_t red, uint8_t green, uint8_t blue)
{
    // Send the green component first (MSB)
    for (int i = 7; i >= 0; i--) {
        LED_SendBit((green >> i) & 1);
    }
    // Send the red component next
    for (int i = 7; i >= 0; i--) {
        LED_SendBit((red >> i) & 1);
    }
    // Send the blue component last (LSB)
    for (int i = 7; i >= 0; i--) {
        LED_SendBit((blue >> i) & 1);
    }
}
