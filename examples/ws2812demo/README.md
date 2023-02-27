# WS2812B SPI DMA example

This example uses SPI-DMA to output WS2812B LEDs.  By chunking the outputs using the center-interrupt, it is possible to double-buffer the WS2812B output data while only storing a few LEDs worth of data in memory at once.

This outputs the LED data on the MOSI (PC6) pin of the CH32V003.

Additionally, this demo only uses 6% CPU while it's outputting LEDs and free while not and it doesn't require precise interrupt timing, increasing or decreasing `DMALEDS` will adjust how lineant the timing is on LED catpures.

The timing on the SPI bus is not terribly variable, the best I was able to find was:

Ton0 = 324ns
Ton1 = 990ns
Toff0 = 990ns
Toff1 = 324ns
Treset = 68us


