# DMA SPI TX example
This example can be used as a baseline for implementing SPI using DMA.
It uses a basic finite state machine to control the CS line, where it is
set by the user before initiating a transfer and then cleared by the DMA ISR. 
Due to the DMA flagging Transfer Complete before the SPI has clocked out the 
last byte, I have implemented some code to check for the BSY (busy) flag on SPI
to clear before subsequently clearing the CS line.

![Screenshot_20240718_232950](https://github.com/user-attachments/assets/67d0bbe0-f40c-4aeb-bf76-60632292363a)

## License
MIT License

Copyright (c) 2024 Fredrik Saevland

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Building
**make** - build the project

## Use
Flash the binary to the CH32V003 board of your choice and connect an oscilloscope
or logic analyzer to PC5 (SCK), PC6 (MOSI) and PC0 (CS). You should then see a
continuous bursts of the sequence {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}
being transmitted over the SPI bus.

To initiate a transfer manually the user must set the CS line low, set spi_state to TRANSMITTING,
disable the DMA for register access, load the data length into the DMA transfer counter and
then re-enable the DMA. The DMA will then take care of the rest. All of this is done in the
initiate_transfer() function.
