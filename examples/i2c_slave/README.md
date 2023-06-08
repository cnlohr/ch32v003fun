# I2C peripheral in slave mode

This library and example show how to use the I2C peripheral in slave mode.

The library uses a one-byte address, allowing for up to 256 registers to be defined.

The first byte written to the device within a transaction determines the offset for following reads and writes, emulating a simple EEPROM.

The example will turn on a LED connected to PD0 when the LSB of register 0 is set to 1 and off when it's set to 0.
