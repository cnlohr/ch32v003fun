# ADC with fixed sampling rate

This example shows the use of the ADC in combination with DMA and TIM1 to measure a signal with a fixed sampling rate with low cpu load. This is particularly useful for DSP, eg. Audio effects and filtering. 

The example has been tested for raw adc measurement from a script with sampling rates of 48kHz. The limiting factor is the baudrate, which needs to be 2MBaud for the given 48 kHz and full 12 bit range.
