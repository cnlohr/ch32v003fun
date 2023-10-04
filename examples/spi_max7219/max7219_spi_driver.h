/* 
 * SPI based driver for the MAX7219 display driver (https://www.analog.com/media/en/technical-documentation/data-sheets/MAX7219-MAX7221.pdf)
 * 
 * The driver can be used for 64 discrete LED control, or for 8 digits of 7 segment (+ decimal) displays. It includes a basic 7 segment display font called Code B
 * which is made available with this library, as well as various other functions such as multiplexer scan limiting, digital brightness control and font decoding.
 * 
 * The one-byte segment arrangement in the digit registers appear like so:
 * 
 *     A
 *    ---
 * F | G | B
 *    --- 
 * E |   | C
 *    --- 
 *     D   . DP
 * 
 * +-----+----+----+----+----+----+----+----+----+
 * | Bit | B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0 |
 * +-----+----+----+----+----+----+----+----+----+
 * | Seg | DP |  A |  B |  C |  D |  E |  F |  G |
 * +-----+----+----+----+----+----+----+----+----+
 * 
 * On standard 8 digit boards, the 0th digit (MAX7219_REGISTER_DIGIT0) is on the right and the 7th digit (MAX7219_REGISTER_DIGIT7) is on the left.
 */

//Include guard
#ifndef MAX7219_SPI_DRIVER_H
#define MAX7219_SPI_DRIVER_H

//Includes
#include "ch32v003_SPI.h"
#include <stdbool.h>

//Instance struct
struct MAX7219_Display
{
    uint8_t displays_in_chain;

    GPIO_TypeDef* chip_select_port;
    uint8_t chip_select_pin;

    uint8_t last_set_decode_mode;
};

//Chip select methods
void MAX7219_select(struct MAX7219_Display display)
{
    display.chip_select_port->BSHR |= 0b00000001 << display.chip_select_pin << 16; //Reset pin (active low)
}

void MAX7219_deselect(struct MAX7219_Display display)
{
    display.chip_select_port->BSHR |= 0b00000001 << display.chip_select_pin; //Set pin (active low)
}

//Raw communication
#define MAX7219_REGISTER_NOOP 0x00
/*#define MAX7219_REGISTER_DIGIT0 0x01
#define MAX7219_REGISTER_DIGIT1 0x02
#define MAX7219_REGISTER_DIGIT2 0x03
#define MAX7219_REGISTER_DIGIT3 0x04
#define MAX7219_REGISTER_DIGIT4 0x05
#define MAX7219_REGISTER_DIGIT5 0x06
#define MAX7219_REGISTER_DIGIT6 0x07
#define MAX7219_REGISTER_DIGIT7 0x08*/
#define MAX7219_REGISTER_DECODE_MODE 0x09
#define MAX7219_REGISTER_INTENSITY 0x0A
#define MAX7219_REGISTER_SCANLIMIT 0x0B
#define MAX7219_REGISTER_SHUTDOWN 0x0C
#define MAX7219_REGISTER_DISPLAYTEST 0x0F

void MAX7219_write_register(struct MAX7219_Display display, uint8_t reg, uint8_t data)
{
    reg &= 0b00001111; //Remove the top 4 bits as they are not used for the register, and only retain the last 4 bits

    uint16_t packet = reg << 8; //Apply the register address to the final packet in the top 8 bits

    packet |= data; //Apply the data to the final packet in the bottom 8 bits

    MAX7219_select(display); //Select the chip select line

    //Write the packet to the display
    SPI_begin_16();

    SPI_write_16(packet);
    SPI_wait_transmit_finished();
    
    SPI_end();

    MAX7219_deselect(display); //Deselect the chip select line
}

//Register helpers
void MAX7219_shutdown(struct MAX7219_Display display, bool set)
{
    MAX7219_write_register(display, MAX7219_REGISTER_SHUTDOWN, !set);
}

void MAX7219_test(struct MAX7219_Display display, bool set)
{
    MAX7219_write_register(display, MAX7219_REGISTER_DISPLAYTEST, set);
}

#define MAX7219_DECODE_MODE_NONE 0x00
#define MAX7219_DECODE_MODE_0_ONLY 0x01
#define MAX7219_DECODE_MODE_0_TO_3_ONLY 0x0F
#define MAX7219_DECODE_MODE_ALL 0xFF

void MAX7219_set_decode_mode(struct MAX7219_Display display, uint8_t mode)
{
    MAX7219_write_register(display, MAX7219_REGISTER_DECODE_MODE, mode);
    display.last_set_decode_mode = mode;
}

#define MAX7219_SCANLIMIT_0_ONLY 0x00
#define MAX7219_SCANLIMIT_01 0x01
#define MAX7219_SCANLIMIT_012 0x02
#define MAX7219_SCANLIMIT_0123 0x03
#define MAX7219_SCANLIMIT_01234 0x04
#define MAX7219_SCANLIMIT_012345 0x05
#define MAX7219_SCANLIMIT_0123456 0x06
#define MAX7219_SCANLIMIT_ALL 0x07

void MAX7219_set_scan_limit(struct MAX7219_Display display, uint8_t limit)
{
    limit &= 0b00000111; //Only accept the 3 lsbs for this register
    MAX7219_write_register(display, MAX7219_REGISTER_SCANLIMIT, limit);
}

#define MAX7219_BRIGHTNESS_MAX 0x0F
#define MAX7219_MRIGHTNESS_MIN 0x00

void MAX7219_set_brightness(struct MAX7219_Display display, uint8_t brightness)
{
    brightness &= 0b00001111; //Only accept the 4 lsbs for this register
    MAX7219_write_register(display, MAX7219_REGISTER_INTENSITY, brightness);
}

//Built in CODE B font
#define MAX7219_CODEB_ADD_DECPOINT 0x80
#define MAX7219_CODEB_DASH 0x0A
#define MAX7219_CODEB_BLANK 0x0F
#define MAX7219_CODEB_0 0x00
#define MAX7219_CODEB_1 0x01
#define MAX7219_CODEB_2 0x02
#define MAX7219_CODEB_3 0x03
#define MAX7219_CODEB_4 0x04
#define MAX7219_CODEB_5 0x05
#define MAX7219_CODEB_6 0x06
#define MAX7219_CODEB_7 0x07
#define MAX7219_CODEB_8 0x08
#define MAX7219_CODEB_9 0x09
#define MAX7219_CODEB_E 0x0B
#define MAX7219_CODEB_H 0x0C
#define MAX7219_CODEB_L 0x0D
#define MAX7219_CODEB_P 0x0E

//Raw segment
#define MAX7219_SEGMENT_DP 0b10000000
#define MAX7219_SEGMENT_A  0b01000000
#define MAX7219_SEGMENT_B  0b00100000
#define MAX7219_SEGMENT_C  0b00010000
#define MAX7219_SEGMENT_D  0b00001000
#define MAX7219_SEGMENT_E  0b00000100
#define MAX7219_SEGMENT_F  0b00000010
#define MAX7219_SEGMENT_G  0b00000001

void MAX7219_set_digit(struct MAX7219_Display display, uint32_t digit, uint8_t value)
{
    uint32_t literalDigit = digit % 8;
    //uint32_t displayIndex = digit / 8;

    MAX7219_write_register(display, literalDigit + 1, value);
}

void MAX7219_reset(struct MAX7219_Display display)
{
    //Set display brightness to maximum
    MAX7219_set_brightness(display, MAX7219_BRIGHTNESS_MAX);

    //Set the scan limit to all 8 digits enabled
    MAX7219_set_scan_limit(display, MAX7219_SCANLIMIT_ALL);

    //Enable Code-B decode on all digits
    MAX7219_set_decode_mode(display, MAX7219_DECODE_MODE_ALL);

    //Clear all digits
    for (size_t digitPos = 0; digitPos < display.displays_in_chain * 8; digitPos++)
    {
        MAX7219_set_digit(display, digitPos, MAX7219_CODEB_BLANK);
    }

    //Take the display out of shutdown
    MAX7219_shutdown(display, false);

    //Take the display out of test mode
    MAX7219_test(display, false);
}

void MAX7219_init(struct MAX7219_Display display)
{
    //Default deselected
    MAX7219_deselect(display);

    //Ensure port is enabled
    uint32_t selectedPortAddress = (uint32_t)display.chip_select_port;

    switch (selectedPortAddress)
    {
        case GPIOA_BASE:
            RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;
            break;
        
        case GPIOC_BASE:
            RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
            break;

        case GPIOD_BASE:
            RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
            break;
        
        default: break;
    }

    //Enable push-pull on pin
    display.chip_select_port->CFGLR &= ~(0xf<<(4*display.chip_select_pin));
    display.chip_select_port->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*display.chip_select_pin);

    //Initialise SPI
    SPI_init();

    //Clear display to driver defaults
    MAX7219_reset(display);
}

#endif //MAX7219_SPI_DRIVER_H include guard