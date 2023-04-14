# GPIO Libaray
On the shoulders of the Blink and the adc_polled example, this Arduino-like GPIO + ADC library stands.

All pins are adressable as outputs, inputs, with pull-up, etc. but now you can also read the eight muxed inputs.
The pins are in an enum, so you can call them by their name and iterate over them.

It has been extended by an arduino-like analogRead function.

It's your responsibility to not blow up a pin.
Only use one pin for one thing and you should be fine.

# GPIO Example
Connect LED + 1k resistor to each pin (C0 to C7 and D4) and GND.
Connect a 10k pot between GND and VCC, wiper to D6.
Marvel at the colorful glory.

https://user-images.githubusercontent.com/104343143/231814680-d41ae68f-dc7b-4c9c-a3c7-0b88cc82e541.mp4
