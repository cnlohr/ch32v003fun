# I2C OLED demonstration
This example code demonstrates use of the CH32V003 I2C peripheral with an SSD1306
OLED (128x32 pixel type). It provides a generic I2C port initialization and
transmit (write-only) low level interface and a high-level graphics driver with
pixels, lines, circles, rectangles and 8x8 character font rendering. Out of the
box this demo cycles through a few different graphic screens to test out the
various drawing primitives.


https://user-images.githubusercontent.com/1132011/230734071-dee305de-5aad-4ca0-a422-5fb31d2bb0e0.mp4

## Build options
There are a few build-time options in the i2c.h source:
* I2C_CLKRATE - defines the I2C bus clock rate. Both 100kHz and 400kHz are supported.
800kHz has been seen to work when I2C_PRERATE is 1000000, but 1MHz did not. To
use higher bus rates you must increase I2C_PRERATE at the expense of higher power
consumption.
* I2C_PRERATE - defines the I2C logic clock rate. Must be higher than I2C_CLKRATE.
Keep this value as low as possible (but not lower than 1000000) to ensure low power
operaton.
* I2C_DUTY - for I2C_CLKRATE > 100kHz this specifies the duty cycle, either 33% or 36%.
* TIMEOUT_MAX - the amount of tries in busy-wait loops before giving up. This value
depends on the I2C_CLKRATE and should not affect normal operation.
* I2C_IRQ - chooses IRQ-based operation instead of busy-wait polling. Useful to
free up CPU resources but should be used carefully since it has more potential
mysterious effects and less error checking.
* IRQ_DIAG - enables timing analysis via GPIO toggling. Don't enable this unless
you know what you're doing.

## Use
Connect an SSD1306-based OLED in I2C interface mode to pins PC1 (SDA) and PC2 (SCL)
of the CH32V003 with proper I2C pullup resistors and observe the various graphic
images that cycle at approximately 2 second intervals.

Note - I used an Adafruit 0.91" 128x32 OLED breakout (stock #4440) for my testing
and found that the built-in 10k pullup resistors were too weak for reliable I2C
bus transactions. I had to add 2.2k resistors to the SCL and SDA pads to allow
proper operation.

