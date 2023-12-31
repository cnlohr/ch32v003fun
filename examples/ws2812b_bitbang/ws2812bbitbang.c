#include "ch32v003fun.h"
#include <stdio.h>

// Define the number of LEDs in the strip
#define num_leds 29

// Include the sine and rand lookup tables
#include "lookups.h"

#define WS_GPIO_PORT D
#define WS_GPIO_PIN 6
#define LOCAL_CONCAT(A, B) A##B
#define LOCAL_EXP(A, B) LOCAL_CONCAT(A,B)
#define WS_GPIO LOCAL_EXP( GPIO, WS_GPIO_PORT)

// Include the Green Dragon Bitbanging driver
#include "ws2812bbitbang_driver.h"

// Include the colour and animation functions
#include "led_functions.h"


// The main function initialises the Delay and GPIO, sets the initial color values of the LED strip to: off
int main(void)
{
    Delay_Ms ( 1000 ); // Delay to give us a chance to reprogram

    SystemInit();

    // Enable GPIO Port D
    RCC->APB2PCENR |= LOCAL_EXP( RCC_APB2Periph_GPIO, WS_GPIO_PORT );

    // GPIO D6 Push-Pull
    LOCAL_EXP( GPIO, WS_GPIO_PORT )->CFGLR =
        ( LOCAL_EXP( GPIO, WS_GPIO_PORT )->CFGLR &
          (~(0xf<<(WS_GPIO_PIN*4))))
        |
        (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP)<<(WS_GPIO_PIN*4);

    Delay_Ms(5);
    // Send initial LED colour values to the LED strip
    LED_OFF();
    SHOWTIME(BUFFER_LEDS);
    Delay_Ms(200);

    while (1) {

        // LED_COLD_WHITE();
        // LED_WARM_WHITE();
        LED_RAINBOWS(10, num_leds); // variables are delay speed in mS and width (number of leds)
        // LED_RED(255); // variable is brightness 0-255
        // LED_GREEN(10); // variable is brightness 0-255
        // LED_BLUE(10); // variable is brightness 0-255
        // LED_CUSTOM(255,255,255); // set custom clour (red, green, blue) 0-255 RGB
        // LED_RGB_FLASH(200, 2); // variables are speed in mS delay and brightness
        // LED_WHITE_FLASH(50, 10); // variables are max and min flash speed
        // LED_WHITE_FADE(4); // variable is speed in mS delay

    } // end of while loop
} // end of main
