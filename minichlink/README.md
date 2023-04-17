# minichlink

A free, open mechanism to use the CH-LinkE $4 programming dongle for the CH32V003.

If on Linux, be sure to type make sure to install the `99-WCH-LinkE.rules` build rule to `/etc/udev/rules.d/`

On Windows, if you need to you can install the WinUSB driver over the WCH interface 1.

The exe here is about 12kB and contains everything except for the libusb driver.  In Linux you need `libusb-1.0-dev`.

## Usage

```
Usage: minichlink [args]
 single-letter args may be combined, i.e. -3r
 multi-part args cannot.
	-3 Enable 3.3V
	-5 Enable 5V
	-t Disable 3.3V
	-f Disable 5V
	-r Release from reest
	-R Place into Reset
	-D Configure NRST as GPIO **WARNING** If you do this and you reconfig
		the SWIO pin (PD1) on boot, your part can never again be programmed!
	-d Configure NRST as NRST
	-w [binary image to write]
	-o [memory address, decimal or 0x, try 0x08000000] [size, decimal or 0x, try 16384] [output binary image]
```
 
