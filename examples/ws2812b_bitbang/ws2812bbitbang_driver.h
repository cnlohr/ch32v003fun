// Include necessary libraries
#include <stdlib.h>

#define BITBANG_WS2812B_PORT GPIOD
#define BITBANG_WS2812B_PIN 6

#define WAIT_ON_TIME_1  6
#define WAIT_ON_TIME_0  3
#define WAIT_OFF_TIME_0 2

// Define an array to buffer the color data for each LED in the strip
uint8_t BUFFER_LEDS[num_leds][3] = {};

// Send a single bit using the GD pinwiggle protocol.
// Check mark/space ratio of the data on GPIO D6 with an oscilloscope (or Logic analyser?).
static void led_send_bit(uint8_t bit)
{
    switch (bit) {
    case 1:
        /**** Send a 1 bit ****/
        BITBANG_WS2812B_PORT->BSHR = 1 << BITBANG_WS2812B_PIN; // put BITBANG_WS2812B_PIN high and wait for 800nS
        Delay_Tiny(WAIT_ON_TIME_1);
        BITBANG_WS2812B_PORT->BCR = 1 << BITBANG_WS2812B_PIN; // put low and exit, 400nS is taken up by other functions
        break;
    default:
        /**** Send a 0 bit ****/
        BITBANG_WS2812B_PORT->BSHR = 1 << BITBANG_WS2812B_PIN; // put pin BITBANG_WS2812B_PIN high and wait for 400nS
        Delay_Tiny(WAIT_ON_TIME_0);
        BITBANG_WS2812B_PORT->BCR = 1 << BITBANG_WS2812B_PIN; // put pin BITBANG_WS2812B_PIN low and wait for 400nS, 400nS is taken up by other functions
        Delay_Tiny(WAIT_OFF_TIME_0);
        break;
    }
}

// Send a single color for a single LED
// WS2812B LEDs want 24 bits per led in the string
void led_send_color(uint8_t red, uint8_t green, uint8_t blue)
{
    // Send the green component first (MSB)
    for (int i = 7; i >= 0; i--) {
        led_send_bit((green >> i) & 1);
    }
    // Send the red component next
    for (int i = 7; i >= 0; i--) {
        led_send_bit((red >> i) & 1);
    }
    // Send the blue component last (LSB)
    for (int i = 7; i >= 0; i--) {
        led_send_bit((blue >> i) & 1);
    }
}
