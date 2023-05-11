# I2C OLED demonstration
This example code demonstrates use of the CH32V003 I2C peripheral with an SSD1306
OLED. Three different OLED sizes are supported - 64x32, 128x32, and 128x64.
It provides a generic I2C port initialization and transmit (write-only) low level
interface and a high-level graphics driver with pixels, lines, circles, rectangles
and 8x8 character font rendering. Out of the box this demo cycles through a few
different graphic screens to test out the various drawing primitives.


https://user-images.githubusercontent.com/1132011/230734071-dee305de-5aad-4ca0-a422-5fb31d2bb0e0.mp4

## Build options
There are a few build-time options in the source - 

In i2c_oled.c:
* SSD1306_64X32, SSD1306_128X32, SSD1306_128X64 - choose only one of these
depending on the type of OLED you've got.

In ssd1306.h
* SSD1306_PSZ - the number of bytes to send per I2C data packet. The default value
of 32 seems to work well. Smaller values are allowed but may result in slower
refresh rates.

In ssd1306_i2c.h
* SSD1306_I2C_ADDR - the I2C address of your OLED display. The default is 0x3c
which should work for most devices. Use 0x3d if you've pulled the SA0 line high.
* SSD1306_I2C_CLKRATE - defines the I2C bus clock rate. Both 100kHz and 400kHz
are supported. 800kHz has been seen to work when I2C_PRERATE is 1000000, but 1MHz
did not. To use higher bus rates you must increase I2C_PRERATE at the expense of
higher power consumption.
* SSD1306_I2C_PRERATE - defines the I2C logic clock rate. Must be higher than
I2C_CLKRATE. Keep this value as low as possible (but not lower than 1000000) to
ensure low power operaton.
* SSD1306_I2C_DUTY - for I2C_CLKRATE > 100kHz this specifies the duty cycle,
either 33% or 36%.
* TIMEOUT_MAX - the amount of tries in busy-wait loops before giving up. This
value depends on the I2C_CLKRATE and should not affect normal operation.
* SSD1306_I2C_IRQ - chooses IRQ-based operation instead of busy-wait polling.
Useful to free up CPU resources but should be used carefully since it has more
potential mysterious effects and less error checking.
* IRQ_DIAG - enables timing analysis via GPIO toggling. Don't enable this unless
you know what you're doing.

There are a few build-time options in the oled.h source:

## Use
Connect an SSD1306-based OLED in I2C interface mode to pins PC1 (SDA) and PC2 (SCL)
of the CH32V003 with proper I2C pullup resistors and observe the various graphic
images that cycle at approximately 2 second intervals.

Note - for part of my testing I used an Adafruit 0.91" 128x32 OLED breakout
(stock #4440) and found that the built-in 10k pullup resistors were too weak for
reliable I2C bus transactions. I had to add 2.2k resistors to the SCL and SDA
pads to allow proper operation. Generic OLED boards from ebay had stronger pullups
and did not need modification.

