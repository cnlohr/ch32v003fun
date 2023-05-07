# SPI nRF24L01+ demo
This "example" consists of three parts:
A SPI Library for the ch32v003, a platform-agnostic library for the nRF24L01(+) and the actual example.

## SPI library
The SPI library helps you configure the SPI registers.
Initially the idea was to make it Arduino-compatible but GPL.
This is free-er.
Currently, only master-mode is supported, either R+W or W-only.
Then you can start talking on the SPI bus through SPI_read_x(), SPI_write_x() and, most importantly SPI_transfer_x(), where the x can be a 8 or 16 bit word (this is C, so no overloading).
When you're done you can even shut down the SPI peripheral to reduce current consumption.

I welcome any improvements you may choose to make, it's far from complete or good.
CRC, for example, is not implemented yet and I'm incapable of making DMA easy, as the spi dac example showed the processor can be mostly "braindead" and the tiny chip can still output loads of data.

## nRF24L01(+) library
This is the doing of [Reza Ebrahimi](https://github.com/ebrezadev), I've just included a copy of [his library](https://github.com/ebrezadev/nRF24L01-C-Driver) here and made some modifications.

## example
The example shows how to send either a uint8_t or a char[16].
To choose, move the comments.
The receiver-part is spi_24L01_rx, split in two because I don't know how to have the unified makefile recurse any differently.
You can enable / disable acknowledge mode by using "ACK_MODE" or "NO_ACK_MODE" in the call to nrf24_transmit().

### nRF module
Never connect the nRF module to 5V, it operates at 1.9-3.3V, much past that it will probably die!
Also, I've become convinced the briliant engineers at nordic are evil!!
Thanks to nordics specifications in the datasheet, the chinese module manufacturers don't even pack 100nF onto the board as bypass caps!
Four hours of trouble-shooting later I stumbled upon the magic of adding a capacitor!!
For you see, as the IC transmits its packets, it draws huge amounts.. wrong!
Even in RX-mode, additional capacitance is essential for basic operation, let alone the acknowledge mode where they need to switch roles within 500uS to exchange a "roger roger".
To the VCC and GND pins I added a 33uF 16V each. Though a 10uF ceramic capacitor would likely have been better due to lower ESR. The blue polyester caps are 220nF, straight onto the 10nF(!!) bypass cap, but I doubt they are strictly necessary.
Can you imagine they saved 0.006$ and now we have to get out ye olde soldering iron so these work properly!!

#### pinout for ch32v003
Please check whether your module looks like [THIS](https://www.circuitspecialists.com/content/552219/NRF24L01-RF-2.jpg) before using this pinout!
perspective: looking at the underside of the module, pin header in the top right corner

nRF		  | ch32		|	nRF		| ch32
--------|---------|-------|------
VCC     | 3V3     | GND   | GND
CSN/SS  |	C0			|	CE		| C4
MOSI	  | C6			|	SCK		| C5
IRQ		  | NC			|	MISO	| C7

And then wire D4 to LED1 on the evaluation board.

#### Arduino friend
If, like me, you only have the one ch32v003 evaluation board, you can find a [friend for it here](https://github.com/recallmenot/ch32v003fun_friends/tree/main/Arduino/NRF24L01_RX).
