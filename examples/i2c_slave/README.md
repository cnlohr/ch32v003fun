# I2C peripheral in slave mode

This library and example show how to use the I2C peripheral in slave mode.

The library uses a one-byte address, allowing for up to 256 registers to be defined.

The first byte written to the device within a transaction determines the offset for following reads and writes, emulating a simple EEPROM.

The example will turn on a LED connected to PA2 when the LSB of register 0 is set to 1 and off when it's set to 0.

## Usage

Initialize the I2C1 SDA and SCL pins you want to use as open-drain outputs:
```
funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL
```

For chips other than the CH32V003 you will need to change the pin numbers to the pins corresponding with the I2C1 peripheral. If you want to use the alternative pins for the I2C periperal in addition to configuring the pins you have to configure the chip to use the the alternative pins using the `I2C1_RM` and `I2C1REMAP1` fields of the `AFIO_PCFR1` register.

Then initialize the I2C1 peripheral in slave mode using:

```
SetupI2CSlave(0x09, i2c_registers, sizeof(i2c_registers), onWrite, onRead, false);
```

In which `0x09` is the I2C address to listen on and i2c_registers is a pointer to a volitile uint8_t array.

The `onWrite` and `onRead` functions are optional callbacks used to react to the registers being written to or read from.

```
void onWrite(uint8_t reg, uint8_t length) {}
void onRead(uint8_t reg) {}
```

The last boolean argument is for making the registers read only via I2C.

You can also enable and disable writing using the functions

```
void SetI2CSlaveReadOnly(bool read_only);
void SetSecondaryI2CSlaveReadOnly(bool read_only);
```

The I2C1 peripheral can also listen on a secondary address. To enable this feature call the following function:

```
SetupSecondaryI2CSlave(0x42, i2c_registers2, sizeof(i2c_registers2), onWrite2, onRead2, false);
```

The function arguments are identical to the normal `SetupI2CSlave` function. The secondary I2C address acts like a completely separate I2C device with it's own registers.

Calling `SetupSecondaryI2CSlave` with the I2C address set to 0 disables listening on the secondary address.

It is recommended to react to register writes using the `onWrite` callback and not by reading the registers array from main(). There is a chance the compiler will optimize away your code if you do that.
