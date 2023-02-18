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