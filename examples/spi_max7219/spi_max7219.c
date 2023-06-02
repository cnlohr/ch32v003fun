#define CH32V003_SPI_SPEED_HZ 1000000

#define CH32V003_SPI_IMPLEMENTATION
#define CH32V003_SPI_DIRECTION_1LINE_TX
#define CH32V003_SPI_CLK_MODE_POL0_PHA0
#define CH32V003_SPI_NSS_SOFTWARE_ANY_MANUAL

//Control which demos you want to see here
#define DEMO_DISPLAY_TEST
#define DEMO_CODEB
#define DEMO_EXTENDED_FONT
#define DEMO_HELLO_BLINK
#define DEMO_FIGURES_OF_EIGHT
#define DEMO_TOGETHER_TWIRLS
#define DEMO_COUNTER_TWIRLS
#define DEMO_KNIGHTRIDER_DECIMAL
#define DEMO_KNIGHTRIDER_DASH
#define DEMO_INTENSITY_FADE_DECIMALS
#define DEMO_INTENSITY_FADE_DIGITS
#define DEMO_LED_CHASE

#include "ch32v003fun.h"
#include <stdio.h>
#include "max7219_spi_driver.h"
#include "max7219_spi_driver_extended.h"

//MOSI on PC6, SCLK on PC5, software controlled CS on PD0

int main()
{
    SystemInit();

    struct MAX7219_Display display =
    {
        .displays_in_chain = 1,

        .chip_select_port = GPIOD,
        .chip_select_pin = 0
    };

    MAX7219_init(display);

    while(1)
    {
        #ifdef DEMO_DISPLAY_TEST
        //Display test on
        MAX7219_test(display, true);
        Delay_Ms(1000);

        //Display test off
        MAX7219_test(display, false);
        #endif

        #ifdef DEMO_CODEB
        //Code B non-decimal
        MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_ALL);

        MAX7219_set_digit(display, 7, MAX7219_CODEB_0);
        MAX7219_set_digit(display, 6, MAX7219_CODEB_1);
        MAX7219_set_digit(display, 5, MAX7219_CODEB_2);
        MAX7219_set_digit(display, 4, MAX7219_CODEB_3);
        MAX7219_set_digit(display, 3, MAX7219_CODEB_4);
        MAX7219_set_digit(display, 2, MAX7219_CODEB_5);
        MAX7219_set_digit(display, 1, MAX7219_CODEB_6);
        MAX7219_set_digit(display, 0, MAX7219_CODEB_7);

        Delay_Ms(3000);

        MAX7219_set_digit(display, 7, MAX7219_CODEB_8);
        MAX7219_set_digit(display, 6, MAX7219_CODEB_9);
        MAX7219_set_digit(display, 5, MAX7219_CODEB_DASH);
        MAX7219_set_digit(display, 4, MAX7219_CODEB_H);
        MAX7219_set_digit(display, 3, MAX7219_CODEB_E);
        MAX7219_set_digit(display, 2, MAX7219_CODEB_L);
        MAX7219_set_digit(display, 1, MAX7219_CODEB_P);
        MAX7219_set_digit(display, 0, MAX7219_CODEB_BLANK);

        Delay_Ms(3000);

        //Code B decimal
        MAX7219_set_digit(display, 7, MAX7219_CODEB_0 | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 6, MAX7219_CODEB_1 | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 5, MAX7219_CODEB_2 | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 4, MAX7219_CODEB_3 | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 3, MAX7219_CODEB_4 | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 2, MAX7219_CODEB_5 | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 1, MAX7219_CODEB_6 | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 0, MAX7219_CODEB_7 | MAX7219_CODEB_ADD_DECPOINT);

        Delay_Ms(3000);

        MAX7219_set_digit(display, 7, MAX7219_CODEB_8 | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 6, MAX7219_CODEB_9 | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 5, MAX7219_CODEB_DASH | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 4, MAX7219_CODEB_H | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 3, MAX7219_CODEB_E | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 2, MAX7219_CODEB_L | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 1, MAX7219_CODEB_P | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 0, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);

        Delay_Ms(3000);
        #endif

        #ifdef DEMO_EXTENDED_FONT
        //Extended font
        MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_NONE);

        MAX7219_set_digit(display, 7, MAX7219_EXTFONT_A);
        MAX7219_set_digit(display, 6, MAX7219_EXTFONT_B);
        MAX7219_set_digit(display, 5, MAX7219_EXTFONT_C);
        MAX7219_set_digit(display, 4, MAX7219_EXTFONT_C_LOWER);
        MAX7219_set_digit(display, 3, MAX7219_EXTFONT_D);
        MAX7219_set_digit(display, 2, MAX7219_EXTFONT_E);
        MAX7219_set_digit(display, 1, MAX7219_EXTFONT_F);
        MAX7219_set_digit(display, 0, MAX7219_EXTFONT_H);

        Delay_Ms(3000);

        MAX7219_set_digit(display, 7, MAX7219_EXTFONT_H_LOWER);
        MAX7219_set_digit(display, 6, MAX7219_EXTFONT_I);
        MAX7219_set_digit(display, 5, MAX7219_EXTFONT_I_LOWER);
        MAX7219_set_digit(display, 4, MAX7219_EXTFONT_L);
        MAX7219_set_digit(display, 3, MAX7219_EXTFONT_L_LOWER);
        MAX7219_set_digit(display, 2, MAX7219_EXTFONT_N);
        MAX7219_set_digit(display, 1, MAX7219_EXTFONT_O);
        MAX7219_set_digit(display, 0, MAX7219_EXTFONT_O_LOWER);

        Delay_Ms(3000);

        MAX7219_set_digit(display, 7, MAX7219_EXTFONT_P);
        MAX7219_set_digit(display, 6, MAX7219_EXTFONT_S);
        MAX7219_set_digit(display, 5, MAX7219_EXTFONT_T);
        MAX7219_set_digit(display, 4, MAX7219_EXTFONT_U);
        MAX7219_set_digit(display, 3, MAX7219_EXTFONT_U_LOWER);
        MAX7219_set_digit(display, 2, MAX7219_EXTFONT_X);
        MAX7219_set_digit(display, 1, MAX7219_EXTFONT_Y);
        MAX7219_set_digit(display, 0, MAX7219_EXTFONT_Z);

        Delay_Ms(3000);

        MAX7219_set_digit(display, 7, MAX7219_EXTFONT_0);
        MAX7219_set_digit(display, 6, MAX7219_EXTFONT_1);
        MAX7219_set_digit(display, 5, MAX7219_EXTFONT_2);
        MAX7219_set_digit(display, 4, MAX7219_EXTFONT_3);
        MAX7219_set_digit(display, 3, MAX7219_EXTFONT_4);
        MAX7219_set_digit(display, 2, MAX7219_EXTFONT_5);
        MAX7219_set_digit(display, 1, MAX7219_EXTFONT_6);
        MAX7219_set_digit(display, 0, MAX7219_EXTFONT_7);

        Delay_Ms(3000);

        MAX7219_set_digit(display, 7, MAX7219_EXTFONT_8);
        MAX7219_set_digit(display, 6, MAX7219_EXTFONT_9);
        MAX7219_set_digit(display, 5, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 4, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 3, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 2, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 1, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 0, MAX7219_EXTFONT_BLANK);

        Delay_Ms(3000);

        MAX7219_set_digit(display, 7, MAX7219_EXTFONT_DEGREES);
        MAX7219_set_digit(display, 6, MAX7219_EXTFONT_EQUALS_TOP);
        MAX7219_set_digit(display, 5, MAX7219_EXTFONT_EQUALS_BOTTOM);
        MAX7219_set_digit(display, 4, MAX7219_EXTFONT_TRIEQUALS);
        MAX7219_set_digit(display, 3, MAX7219_EXTFONT_HYPHEN);
        MAX7219_set_digit(display, 2, MAX7219_EXTFONT_UNDERSCORE);
        MAX7219_set_digit(display, 1, MAX7219_EXTFONT_OVERSCORE);
        MAX7219_set_digit(display, 0, MAX7219_EXTFONT_FORWARDSLASH);

        Delay_Ms(3000);

        MAX7219_set_digit(display, 7, MAX7219_EXTFONT_BACKSLASH);
        MAX7219_set_digit(display, 6, MAX7219_EXTFONT_DECIMAL);
        MAX7219_set_digit(display, 5, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 4, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 3, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 2, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 1, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 0, MAX7219_EXTFONT_BLANK);

        Delay_Ms(3000);
        #endif

        #ifdef DEMO_HELLO_BLINK
        MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_NONE);

        MAX7219_set_digit(display, 7, MAX7219_EXTFONT_H);
        MAX7219_set_digit(display, 6, MAX7219_EXTFONT_E);
        MAX7219_set_digit(display, 5, MAX7219_EXTFONT_L);
        MAX7219_set_digit(display, 4, MAX7219_EXTFONT_L);
        MAX7219_set_digit(display, 3, MAX7219_EXTFONT_O);
        MAX7219_set_digit(display, 2, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 1, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 0, MAX7219_EXTFONT_BLANK);

        Delay_Ms(500);

        MAX7219_shutdown(display, true);

        Delay_Ms(500);

        MAX7219_shutdown(display, false);

        Delay_Ms(500);

        MAX7219_shutdown(display, true);

        Delay_Ms(500);

        MAX7219_shutdown(display, false);

        Delay_Ms(500);

        MAX7219_shutdown(display, true);

        Delay_Ms(500);

        MAX7219_shutdown(display, false);
        #endif

        #ifdef DEMO_FIGURES_OF_EIGHT
        //Figures of 8 (HP style)
        MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_NONE);
        
        MAX7219_set_digit(display, 7, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 6, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 5, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 4, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 3, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 2, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 1, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 0, MAX7219_EXTFONT_BLANK);

        for (size_t figureeightcycles = 0; figureeightcycles < 8; figureeightcycles++)
        {
            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_G);
            Delay_Ms(75);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_B);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_B);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_B);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_B);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_B);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_B);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_B);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_B);
            Delay_Ms(75);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_A);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_A);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_A);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_A);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_A);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_A);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_A);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_A);
            Delay_Ms(75);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_F);
            Delay_Ms(75);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_G);
            Delay_Ms(75);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_C);
            Delay_Ms(75);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_D);
            Delay_Ms(75);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_E);
            Delay_Ms(75);
        }
        #endif

        #ifdef DEMO_TOGETHER_TWIRLS
        //Together twirls
        MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_NONE);

        for (size_t togethertwirlcycles = 0; togethertwirlcycles < 8; togethertwirlcycles++)
        {
            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            Delay_Ms(250);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_B | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_B | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_B | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_B | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_B | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_B | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_B | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_B | MAX7219_SEGMENT_C);
            Delay_Ms(250);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            Delay_Ms(250);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_E | MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_E | MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_E | MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_E | MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_E | MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_E | MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_E | MAX7219_SEGMENT_F);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_E | MAX7219_SEGMENT_F);
            Delay_Ms(250);
        }
        #endif

        #ifdef DEMO_COUNTER_TWIRLS
        //Counter twirls
        MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_NONE);

        for (size_t countertwirlcycles = 0; countertwirlcycles < 8; countertwirlcycles++)
        {
            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_F | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_F | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_F | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_F | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_F | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_F | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_F | MAX7219_SEGMENT_C);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_F | MAX7219_SEGMENT_C);
            Delay_Ms(250);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_A | MAX7219_SEGMENT_G);
            Delay_Ms(250);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_B | MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_B | MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_B | MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_B | MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_B | MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_B | MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_B | MAX7219_SEGMENT_E);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_B | MAX7219_SEGMENT_E);
            Delay_Ms(250);

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_G | MAX7219_SEGMENT_D);
            Delay_Ms(250);
        }
        #endif

        #ifdef DEMO_KNIGHTRIDER_DECIMAL
        //Decimal knight-rider-like bounce effect using Code B on decimals
        MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_ALL);

        MAX7219_set_digit(display, 7, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 6, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 5, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 4, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 3, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 2, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 1, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 0, MAX7219_CODEB_BLANK);

        Delay_Ms(125);

        for (size_t ridercycles = 0; ridercycles < 4; ridercycles++)
        {
            size_t position = 7;

            while (position > 0)
            {
                //If the next position is within bounds, place a DP there
                if (position - 1 >= 0) MAX7219_set_digit(display, position - 1, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);

                //Clear current position
                MAX7219_set_digit(display, position, MAX7219_CODEB_BLANK);
                
                //Move onto the next position
                position--;

                Delay_Ms(125);
            }

            while (position < 7)
            {
                //If the next position is within bounds, place a DP there
                if (position + 1 <= 7) MAX7219_set_digit(display, position + 1, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);

                //Clear current position
                MAX7219_set_digit(display, position, MAX7219_CODEB_BLANK);
                
                //Move onto the next position
                position++;

                Delay_Ms(125);
            }
        }
        #endif

        #ifdef DEMO_KNIGHTRIDER_DASH
        //Decimal knight-rider-like bounce effect using Code B on dashes
        MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_ALL);

        MAX7219_set_digit(display, 7, MAX7219_CODEB_DASH);
        MAX7219_set_digit(display, 6, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 5, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 4, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 3, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 2, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 1, MAX7219_CODEB_BLANK);
        MAX7219_set_digit(display, 0, MAX7219_CODEB_BLANK);

        Delay_Ms(125);

        for (size_t ridercycles = 0; ridercycles < 4; ridercycles++)
        {
            size_t position = 7;

            while (position > 0)
            {
                //If the next position is within bounds, place a DP there
                if (position - 1 >= 0) MAX7219_set_digit(display, position - 1, MAX7219_CODEB_DASH);

                //Clear current position
                MAX7219_set_digit(display, position, MAX7219_CODEB_BLANK);
                
                //Move onto the next position
                position--;

                Delay_Ms(125);
            }

            while (position < 7)
            {
                //If the next position is within bounds, place a DP there
                if (position + 1 <= 7) MAX7219_set_digit(display, position + 1, MAX7219_CODEB_DASH);

                //Clear current position
                MAX7219_set_digit(display, position, MAX7219_CODEB_BLANK);
                
                //Move onto the next position
                position++;

                Delay_Ms(125);
            }
        }
        #endif

        #ifdef DEMO_INTENSITY_FADE_DECIMALS
        //All decimals intensity fade effect
        MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_ALL);

        MAX7219_set_digit(display, 7, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 6, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 5, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 4, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 3, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 2, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 1, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);
        MAX7219_set_digit(display, 0, MAX7219_CODEB_BLANK | MAX7219_CODEB_ADD_DECPOINT);

        MAX7219_set_brightness(display, MAX7219_BRIGHTNESS_MAX);

        Delay_Ms(34); //~30FPS

        for (size_t intensitycycles = 0; intensitycycles < 4; intensitycycles++)
        {
            size_t intensity = 0x0E;

            while (intensity > 0x00)
            {
                MAX7219_set_brightness(display, intensity);

                intensity--;

                Delay_Ms(34); //~30FPS
            }

            while (intensity < 0x0F)
            {
                MAX7219_set_brightness(display, intensity);
                
                //Move onto the next position
                intensity++;

                Delay_Ms(34); //~30FPS
            }
        }
        #endif

        #ifdef DEMO_INTENSITY_FADE_DIGITS
        //Numeric intensity fade effect
        MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_ALL);

        MAX7219_set_digit(display, 7, MAX7219_CODEB_0);
        MAX7219_set_digit(display, 6, MAX7219_CODEB_1);
        MAX7219_set_digit(display, 5, MAX7219_CODEB_2);
        MAX7219_set_digit(display, 4, MAX7219_CODEB_3);
        MAX7219_set_digit(display, 3, MAX7219_CODEB_4);
        MAX7219_set_digit(display, 2, MAX7219_CODEB_5);
        MAX7219_set_digit(display, 1, MAX7219_CODEB_6);
        MAX7219_set_digit(display, 0, MAX7219_CODEB_7);

        MAX7219_set_brightness(display, MAX7219_BRIGHTNESS_MAX);

        Delay_Ms(34); //~30FPS

        for (size_t intensitycycles = 0; intensitycycles < 4; intensitycycles++)
        {
            size_t intensity = 0x0E;

            while (intensity > 0x00)
            {
                MAX7219_set_brightness(display, intensity);

                intensity--;

                Delay_Ms(34); //~30FPS
            }

            while (intensity < 0x0F)
            {
                MAX7219_set_brightness(display, intensity);
                
                //Move onto the next position
                intensity++;

                Delay_Ms(34); //~30FPS
            }
        }
        #endif

        #ifdef DEMO_LED_CHASE
        //Spinny LED chase effect using raw segment control
        MAX7219_reset(display);

        MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_NONE);

        MAX7219_set_digit(display, 7, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 6, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 5, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 4, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 3, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 2, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 1, MAX7219_EXTFONT_BLANK);
        MAX7219_set_digit(display, 0, MAX7219_EXTFONT_BLANK);

        for (size_t chasecycles = 0; chasecycles < 8; chasecycles++)
        {
            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_A);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 7, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_A);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 6, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_A);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 5, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_A);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 4, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_A);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 3, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_A);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 2, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_A);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 1, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_A);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_B);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_C);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 0, MAX7219_SEGMENT_D);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 0, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 1, MAX7219_SEGMENT_D);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 1, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 2, MAX7219_SEGMENT_D);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 2, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 3, MAX7219_SEGMENT_D);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 3, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 4, MAX7219_SEGMENT_D);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 4, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 5, MAX7219_SEGMENT_D);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 5, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 6, MAX7219_SEGMENT_D);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 6, MAX7219_EXTFONT_BLANK);
            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_D);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_E);
            Delay_Ms(34); //~30FPS

            MAX7219_set_digit(display, 7, MAX7219_SEGMENT_F);
            Delay_Ms(34); //~30FPS
        }
        #endif
    }
}
