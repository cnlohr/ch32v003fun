# Code adapted from https://github.com/Blakesands/CH32V003

This code drives WS2812B LEDs using the eight-pin SSOP CH32V003J4M6 £0.10
microcontroller. It adapts the code from
[Blakesands](https://github.com/Blakesands/CH32V003) to use ch32v003fun instead
of MounRiver, so you can customise, compile and program the microcontroller
without having the MounRiver IDE from WCH. It might work for other CH32V003
packages (e.g. 16-pin, 20-pin) but I have not tested this.

* Code and documentation for this chip: https://github.com/openwch/ch32v003

* Where to buy 50 of the chips: https://www.aliexpress.com/item/1005005036714708.html

The 8-pin version of this CH32V003 MCU is the J4M6. To program it you one of
the minichlink supported programmers: See [the ch32v003fun README for supported
programmers](https://github.com/cnlohr/ch32v003fun/blob/c788f9ae0a0d7c195af5a7441d3c97522470b247/README.md)
. It needs only 3 lines connecting. + voltage, - voltage and SWIO that goes to
pin 8 on the 8-pin IC.

# How it works

The driver for the WS2812B addressable LED strips is written in C language for
a CH32V003 microcontroller. It uses direct port access to toggle *GPIO pin D6
(pin 1 on the 8-pin IC)*, and assembly language to insert NOP (No Operation)
instructions, which are instructions that take up a clock cycle but do nothing,
to control the precise timing of the bit-banging operations.
