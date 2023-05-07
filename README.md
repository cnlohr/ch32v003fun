# ch32v003fun

An open source development environment (tooling, headers, examples) for the [CH32V003](http://www.wch-ic.com/products/CH32V003.html) with gcc-riscv64 that can be used in Windows (Native), Linux and/or WSL.  The CH32V003 is 10-cent part with a RISC-V EC core that runs at 48MHz, has 16kB of flash and 2kB of RAM and a bunch of peripherals.  It also comes in SOP-8, QFN-20 and SOIC packages.  You can get the datasheet [here](http://www.wch-ic.com/downloads/CH32V003DS0_PDF.html).

The goal of this project is to develop the tooling and environment for efficient use of the CH32V003.  Avoid complicated HALs, and unleash the hardware! The existing EVT is massive, and dev environment weighty.  This project specifically avoids the HAL and makes it so you can just use the [TRM](http://www.wch-ic.com/downloads/CH32V003RM_PDF.html). In contrast, blinky is only 500 bytes with ch32v003fun, boots faster, and significantly simpler overall.

ch32v003fun contains:
1. Examples using ch32v003fun, but not as many as using the HAL.
2. "minichlink" which uses the WCH CH-Link with libusb, for cross-platform use.
  * An STM32F042 Programmer, the NHC-Link042
  * An ESP32S2 Programmer, the [esp32s2-funprog](https://github.com/cnlohr/esp32s2-cookbook/tree/master/ch32v003programmer)
  * The official WCH Link-E Programmer.
3. An extra copy of libgcc so you can use unusual risc-v build chains, located in the `misc/libgcc.a`.
4. A folder named "ch32v003fun" containing a single self-contained source file and header file for compling apps for the ch32v003.
5. On some systems ability to "printf" back through
6. A demo bootloader.

In Progress:
1. Other programmer support (ESP32-S2 works, currently)
2. OpenOCD-compatible build for `minichlink`.
3. Full-chip-write for faster flash.
4. Support for `NHC-Link042`
5. Write more demos.

## Features

### A fast "printf" debug over the programming interface.

And by fast I mean very fast. Typically around 36kBytes/sec. 

```
./minichlink -T | pv > /dev/null
Found ESP32S2 Programmer
 536KiB 0:00:15 [36.7KiB/s] [        <=>                     ]
```

You can just try out the `debugprintf` project, or call `SetupDebugPrintf();` and `printf()` away.

### todo;;


## System Prep

On WSL or Debian based OSes `apt-get install build-essential libnewlib-dev gcc-riscv64-unknown-elf libusb-1.0-0-dev libudev-dev`

On Arch/Manjaro, `sudo pacman -S base-devel libusb`, then from AUR install `riscv64-unknown-elf-gcc, riscv64-unknown-elf-binutils, riscv64-unknown-elf-newlib` (will compile for a long time).

On Windows, download and install (to system) this copy of GCC10. https://gnutoolchains.com/risc-v/

On macOS install the RISC-V toolchain with homebrew following the instructions at https://github.com/riscv-software-src/homebrew-riscv

You can use the pre-compiled minichlink or go to minichlink dir and `make` it.

## Building and Flashing

```
cd examples/blink
make
```

In Linux this will "just work"(TM) using `minichlink`.  
In Windows, if you want to use minichlink, you will need to use Zadig to install WinUSB to the WCH-Link interface 0.  
The generated .hex file is compatible with the official WCH flash tool.  

text = code, data = constants and initialization values, bss = uninitialized values.  
dec is the sum of the 3 and reflects the number of bytes in flash that will get taken up by the program.


## ESP32S2 Programming

## WCH-Link (E)

It enumerates as 2 interfaces.
0. the programming interface.  I can't get anything except the propreitary interface to work.
1. the built-in usb serial port. You can hook up UART D5=TX to RX and D6=RX to TX of the CH32V003 for printf/debugging, default speed is 115200. Both are optional, connect what you need.

If you want to mess with the programming code in Windows, you will have to install WinUSB to the interface 0.  Then you can uninstall it in Device Manager under USB Devices.

On linux you find the serial port with `ls -l /dev/ttyUSB* /dev/ttyACM*` and connect to it with `screen /dev/ttyACM0 115200`  
Disconnect with `CTRL+a` `:quit`.  

Adding your user to these groups will remove the need to `sudo` for access to the serial port:
debian-based
	`sudo usermod -a -G dialout $USER`
arch-based
	`sudo usermod -a -G uucp $USER`
 
 You'll need to log out and in to see the change.


## WCH-Link Hardware access in WSL
To use the WCH-Link in WSL, it is required to "attach" the USB hardware on the Windows side to WSL.  This is achieved using a tool called usbipd.

1. On windows side, install the following MSI https://github.com/dorssel/usbipd-win/releases
2. Install the WSL side client:
    * For Debian: 
        `sudo apt-get install usbip hwdata usbutils`
    * For Arch-based:
        `sudo pacman -S usbip hwdata usbutils`
    * For Ubuntu (not tested):
```
        sudo apt install linux-tools-5.4.0-77-generic linux-tools-virtual hwdata usbutils
        sudo update-alternatives --install /usr/local/bin/usbip usbip `ls /usr/lib/linux-tools/*/usbip | tail -n1` 20
```

3. Plug in the WCH-Link to USB
4. Run Powershell as admin and use the `usbipd list` command to list all connected devices
5. Find the this device: `1a86:8010  WCH-Link (Interface 0)` and note the busid it is attached to
6. In powershell, use the command `usbipd wsl attach --busid=<BUSID>` to attach the device at the busid from previous step
7. You will hear the windows sound for the USB device being removed (and silently attached to WSL instead)
8. In WSL, you will now be able to run `lsusb` and see that the SCH-Link is attached
9. For unknown reasons, you must run make under root access in order to connect to the programmer with minichlink.  Recommend running `sudo make` when building and programming projects using WSL. This may work too (to be confirmed):

### non-root access on linux
Unlike serial interfaces, by default, the USB device is owned by root, has group set to root and everyone else may only read.
1. Get the vendor:device ID of the WCH-Link from `lsusb`.
2. Create a udev rule with `sudo nano /etc/udev/rules.d/80-USB_WCH-Link.rules`, paste (CTRL+SHIFT+V) `SUBSYSTEM=="usb", ATTR{idVendor}=="1a86", ATTR{idProduct}=="8010", MODE="0666"` and save, replacing the idVendor and idProduct if what you got previously was different.
3. Reboot or reload the udev rules with `sudo udevadm control --reload-rules && sudo udevadm trigger` 
4. ???
5. profit
Now anyone on your PC has access to the WCH-Link device, so you can stop using sudo for make.  
I don't think there are any security risks here.
You may also tie this to the WCH-Link serial number or some other attribute from `udevadm info -a -n /dev/bus/usb/busid/deviceid` with the bus id and device id you got from lsusb earlier.


## minichlink

I wrote some libusb copies of some of the basic functionality from WCH-Link, so you can use the little programmer dongle they give you to program the ch32v003. 

Currently, it ignores all the respone codes, except when querying the chip.  But it's rather surprising how featured I could get in about 5 hours.

Anyone who wants to write a good/nice utility should probably look at the code in this folder.

## VSCode + PlatformIO

Note: This is genearlly used for CI on this repo.  However, note that this is **not** the path that allows for debugging on Windows.

This project can also be built, uploaded and debugged with VSCode and the PlatformIO extension. Simply clone and open this project in VSCode and have the PlatformIO extension installed.

See [here](https://github.com/Community-PIO-CH32V/platform-ch32v) for further details.

## Quick Reference
 * Needed for programming/debugging: `SWIO` is on `PD1`
 * Optional (not needed, can be configured as output if fuse set): `NRST` is on `PD7`
 * UART TX (optional) is on: `PD5`

## Support

You can open a github ticket or join my Discord in the #ch32v003fun channel. https://discord.gg/CCeyWyZ

## Contributing

Please check the `contribute` dir for guidance.

## Footnotes/links

 * https://raw.githubusercontent.com/openwch/ch32v003/main/RISC-V%20QingKeV2%20Microprocessor%20Debug%20Manual.pdf Debugging Manual
 * http://www.wch-ic.com/downloads/QingKeV2_Processor_Manual_PDF.html Processor Manual
 * http://www.wch-ic.com/downloads/CH32V003RM_PDF.html Technical Reference Manual
 * http://www.wch-ic.com/downloads/CH32V003DS0_PDF.html Datasheet
