# Systick IRQ demonstration
This example shows how to set up the built-in Systick IRQ to generate periodic
interrupts for timing. Many bare-metal and RTOS based embedded applications will
use the Systick IRQ for timing, periodic housekeeping and task arbitration so
knowing how to set that up is useful.

Note that this example is *NOT* compatible with the Delay_Ms() function that's
part of the ch32v003fun library - that function uses the Systick counter for
doing busy-wait delays and will interfere with its use in generating predictable
IRQs. Do not use the built-in Delay_Ms() and related functions when using Systick
for IRQs.

Note also the use of the  `__attribute__((interrupt))` syntax in declaring the
IRQ handler. Some of the IRQ examples from the WCH HAL library have slightly
different syntax to make use of a fast IRQ mode but which is not compatible with
generic RISC-V GCC so that feature is not used here.

# Use
Connect LEDs (with proper current limit resistors) to GPIO pins C0, D0, D4 and
a 3.3V logic level serial terminal to PD5. The LEDs will flash and an incrementing
count will be printed to the serial port at rates that are controlled by the
Systick IRQ.
