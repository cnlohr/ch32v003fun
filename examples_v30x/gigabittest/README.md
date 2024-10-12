# Working ch32v307 gigabit ethernet example

This example uses the RTL8211E-VB-CG and the QFN68 CH32V307.

This example simply connects to the network at whatever speed it can negotiate.

It prints any broadcast packets it sees. 

It sends out a weird broadcast packet over and over incrementing one of the bytes.

You should be able to see the messages with wireshark.

## IO Mapping

| GMII Pin | Processor Pin | Reconfigurable |
| --- | --- | --- |
| RXC | PC0 | No |
| RXCTL | PC1 | No |
| RXD0 | PC2 | No |
| RXD1 | PC3 | No |
| RXD2 | PA0 | No |
| RXD3 | PA1 | No |
| TXC | PA2 | No |
| TXCTL | PA3 | No |
| TXD0 | PA7 | No |
| TXD1 | PC4 | No |
| TXD2 | PC5 | No |
| TXD3 | PB0 | No |
| CLK125 | PB1 | No |
| MDC | PB12 | No |
| MDIO | PB13 | No |
| 25MHz PHY Crystal | PA8 | From MCO, but could be independently provided by crystal |
| PHY RESET | PA10 | Yes |
| PHY INT (not currently used) | PC7 | Yes |

Strapping pins for the RTL8211E-VB-CG:

| Pin | How to strap |
| --- | --- |
| LED2/RXDLY | 4.7k to GND |
| MDIO | 4.7k to 3.3V |
| LED1 | LED connected to GND, 1k resistor |
| LED0 | LED connected to VCC, 1k resistor | 

