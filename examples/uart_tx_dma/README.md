# UART Tx DMA example

This example shows how to use DMA to send data through the UART peripheral. The sample code does the following:

* Send the "Hello World!\r\n" string each second through the UART peripheral, using DMA.
* Toggling a LED each time the DMA transfer is completed, by using the transmission complete interrupt.

The code is extensively commented, so it should be easy to follow and modify.

# Usage

## Wiring

This code can be run as-is in the [nanoch32V003 board](https://github.com/wuxx/nanoCH32V003). The GPIO pin used for the UART Tx funcion is `D5`. It must be connected to a UART receiver (such as a CP2102 board or similar) configured for 115200 bps, no parity and one stop bit.

The example also uses the nanoch32V003 on-board LED connected to `D6`. On any other board either connect a LED to `D6` or edit the code to remap the pin to match your hardware setup.

## Running

Build and flash the project. You should see the LED toggling each second and the UART receiver should get the "Hello World!" message once per second.
