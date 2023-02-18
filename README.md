# ch32v003fun

My tests for the CH32V003 with gcc-riscv64

This contains:
1. EVT Folder for running demos from the ch32v003 EVT.
2. Barebones. For compiling without the EVT.

## System Prep

On WSL or Debian based OSes `apt-get install build-essential libnewlib-dev gcc-riscv64-unknown-elf`

## Running

```
cd barebones
make
```
Use the WCH-LinkUtility to flash the built hex file.

## For using EVT demos, check out the EVT folder.

## WCH-Link

It enumerates as 2 interfaces.
0. the programming interface.  I can't get anything except the propreitary interface to work.
1. the usb serial port built in.

If you want to mess with the programming code in Windows, you will have to install WinUSB to the interface 0.  Then you can uninstall it in Device Manager under USB Devices.

## miniwchlink

I wrote some libusb copies of some of the basic functionality from WCH-Link, so you can use the little programmer dongle they give you to program the ch32v003. 

Currently, it ignores all the respone codes, except when querying the chip.  But it's rather surprising how featured I could get in about 5 hours.

Anyone who wants to write a good/nice utility should probably look at the code in this folder.
