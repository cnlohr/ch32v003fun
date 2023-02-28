# ch32v003fun

An open source development "environment" for the [CH32V003](http://www.wch-ic.com/products/CH32V003.html) with gcc-riscv64 that can be used in Windows, Linux and/or WSL.  The CH32V003 is 10-cent part with a RISC-V EC core that runs at 48MHz, has 16kB of flash and 2kB of RAM and a bunch of peripherals.  It also comes in SOP-8, QFN-20 and SOIC packages.  You can get the datasheet [here](http://www.wch-ic.com/downloads/CH32V003DS0_PDF.html).

The goal of this project is to develop the tooling and environment for efficient use of the CH32V003.  This means making it possible to have basic projects that are compact and requiire no propreitary tooling.

With this project, you don't require any propreitary tooling or [MounRiver Studio(MRS)](http://www.wch-ic.com/products/www.mounriver.com/).

As it currently stands it is still designed to use the WCH-Link to do the SDIO programming.

ch32v003fun contains:
1. EVT Folder for running demos from the ch32v003 EVT.
2. Barebones. A project that does not use the HAL or EVT.
3. "minichlink" which uses the WCH CH-Link with libusb, for cross-platform use.

## System Prep

On WSL or Debian based OSes `apt-get install build-essential libnewlib-dev gcc-riscv64-unknown-elf libusb-1.0-0-dev`

## Running

```
cd barebones
make
```

In Linux this will "just work" using the `miniwchlink`. In Windows, you can use this or you can use the WCH-LinkUtility to flash the built hex file.

If you are in Windows, you will need to use zadig to install the libusb driver to the WCH-Link interface 0.

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

###
 Footnots/links

 * https://raw.githubusercontent.com/openwch/ch32v003/main/RISC-V%20QingKeV2%20Microprocessor%20Debug%20Manual.pdf Debugging Manual
 * http://www.wch-ic.com/downloads/QingKeV2_Processor_Manual_PDF.html Processor Manual
 * http://www.wch-ic.com/downloads/CH32V003RM_PDF.html Technical Reference Manual
 * http://www.wch-ic.com/downloads/CH32V003DS0_PDF.html Datasheet

