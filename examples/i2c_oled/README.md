# I2C OLED demonstration
This example code demonstrates use of the CH32V003 I2C peripheral with an SSD1306
OLED (128x32 pixel type). It provides a generic I2C port initialization and
transmit (write-only) low level interface and a high-level graphics driver with
pixels, lines, circles, rectangles and 8x8 character font rendering. Out of the
box this demo cycles through a few different graphic screens to test out the
various drawing primitives.

## Use
Connect an SSD1306-based OLED in I2C interface mode to pins PC1 (SCL) and PC2 (SDA)
of the CH32V003 with proper I2C pullup resistors and observe the various graphic
images that cycle at approximately 2 second intervals.

Note - I used an Adafruit 0.91" 128x32 OLED breakout (stock #4440) for my testing
and found that the built-in 10k pullup resistors were too weak for reliable I2C
bus transactions. I had to add 2.2k resistors to the SCL and SDA pads to allow
proper operation.

