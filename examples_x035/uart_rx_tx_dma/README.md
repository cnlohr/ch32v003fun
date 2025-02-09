# UART Echo Example with DMA and IDLE Interrupts

This example demonstrates how to use DMA for UART TX *and* RX for data transfer and IDLE line detection interrupts for identifying the end of a transmission.

## Hardware Setup

- Connect a UART Rx pin to PB10.
- Connect a UART Tx pin to PB11.
- Configure your UART for 115200 baud, 8 data bits, no parity, 1 stop bit.

## Running

Build and flash the project. Use another device (USB serial converter for example) to communicate over UART. Note that you need to use a terminal that supports sending a single line at once in order to properly see the effect of IDLE line detection.