# SPI OLED demonstration
This example code demonstrates use of the CH32V003 SPI peripheral with an SSD1306
OLED. Three different OLED sizes are supported - 64x32, 128x32, and 128x64.
It provides a generic SPI port initialization and transmit (write-only) low level
interface and a high-level graphics driver with pixels, lines, circles, rectangles
and 8x8 character font rendering. Out of the box this demo cycles through a few
different graphic screens to test out the various drawing primitives.

## Build options
There are two build-time options in the oled.h source:
* SSD1306_PSZ - the number of bytes to send per SPI data packet. The default value
of 32 seems to work well. Smaller values are allowed but may result in slower
refresh rates.
* SSD1306_64X32, SSD1306_128X32, SSD1306_128X64 - choose only one of these
depending on the type of OLED you've got.

## Use
Connect an SSD1306-based OLED in SPI interface mode as follows:
* PC2 - RST
* PC3 - CS
* PC4 - DC
* PC5 - SCK / D0
* PC6 - MOSI / D1

Observe Conway's Life starting with random pixels, running for 500 iterations and
then restarting with new random pixels.

