# GPIO Libaray
On the shoulders of the Blink example, this Arduino-like GPIO library stands.

All pins are adressable as outputs, inputs, with pull-up, etc.
The pins are in an enum, so you can call them by their name and iterate over them.

It's your responsibility to not blow up a pin.
Only use one pin for one thing and you should be fine.

# GPIO Example
Connect LED + 1k resistor to each pin (C0 to C7 and D4) and GND.
Marvel at the colorful glory.

https://user-images.githubusercontent.com/104343143/231585338-725f1197-dfa0-484d-8707-f0824af80b7e.mp4
