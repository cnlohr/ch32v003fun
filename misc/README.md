# Supplementary non-official material

This is a copy of libgcc.a that I know works well with the part. Below are the instructions on how to get the source code and how to build it.

# Extra totally non-official info

## GPIO Drive Currents

At 3.3v, it seems that for all speed configurations, the short drive current is 50mA both for emitter and colletor.  But the current falls off rather quickly with some voltage.

At 5v the peak current power application is 90-100mA for emitter and collector.

## General notes about the CH32V003.

CPI/Processor Speed:

Ignoring branches and load/stores, compressed instructions run at 1 CPI. Non-compressed instructions run at 1 CPI for the first 2 instructions, then further ones take 2 CPI regardless of how many more you have.  Running from RAM and running from FLASH have slightly different performance characteristics depending on wait states that should be measured in-situation.

## Building libgcc.a from source.

1. Install prerequisites (for Debian-based systems; similar steps for other systems):

```
sudo apt-get install build-essential autoconf automake autotools-dev curl \
libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo \
gperf libtool patchutils bc zlib1g-dev git
```

2. Checkout sources:

```
git clone --recursive https://github.com/riscv/riscv-gnu-toolchain.git
cd riscv-gnu-toolchain
```

3. Configure and build:

```
./configure --prefix $(pwd)/build-ch32v003 --with-arch=rv32ec --with-abi=ilp32e
make -j8
```

4. Enjoy the built libgcc.a at ./build-ch32v003/lib/gcc/riscv32-unknown-elf/12.2.0/libgcc.a


## The vendor bytes section

```sh
../../ch32v003fun//../minichlink/minichlink -a -r + 0x1ffff700 36 -r + 0x1ffff7e0 20
```

For a ch32v307 CH32V307WCU6:
```
1ffff700: 34 fe 78 dc 18 05 73 30 29 c6 6a 09 85 c5 9f ff 
1ffff710: 55 a1 05 00 00 00 00 86 81 31 15 38 05 fa aa 55 
1ffff720: 62 05 1a 00 
Read 16 bytes
1ffff7e0: 20 01 ff ff ff ff ff ff db 4a aa 7b 54 50 d9 16 
1ffff7f0: ff ff ff ff
```

Another V307 CH32V307WCU6 (from the same lot)
```
1ffff700: 34 fe 78 dc 18 05 73 30 2a c6 8c 09 85 c5 9f ff 
1ffff710: 55 a1 05 00 00 00 00 86 81 31 15 38 05 fa aa 55 
1ffff720: 5c 05 1a 00 
Read 20 bytes
1ffff7e0: 20 01 ff ff ff ff ff ff ed 4a aa 7b 54 50 eb 16 
1ffff7f0: 39 e3 39 e3 
```

A CH32V307VCT6
```
1ffff700: 34 fe 78 dc 28 05 70 30 29 c6 8f 09 85 45 ff ff 
1ffff710: 55 a1 05 00 00 00 00 86 81 31 15 38 05 fa aa 55 
1ffff720: 7c 05 19 00 
Read 20 bytes
1ffff7e0: 20 01 ff ff ff ff ff ff 99 65 1a 7b 54 50 07 31 
1ffff7f0: 39 e3 39 e3 
```

CH32V305FBP6
```
1ffff700: 34 fe 78 dc 38 05 52 30 2a c6 a9 09 45 c5 9b 32 
1ffff710: 55 a1 05 00 00 00 00 86 81 31 15 38 05 fa aa 55 
1ffff720: 81 05 19 00 
Read 20 bytes
1ffff7e0: 80 00 ff ff ff ff ff ff cd ab bd 08 7b bc 05 71 
1ffff7f0: 39 e3 39 e3 
```

A CH32V208
```
1ffff700: 34 fe 78 dc 1c 05 80 20 29 86 51 09 55 00 3f c0 
1ffff710: 24 01 11 10 00 00 00 86 81 31 15 38 05 fa aa 55 
1ffff720: 71 05 1d 00 
Read 20 bytes
1ffff7e0: a0 00 ff ff ff ff ff ff 7a 8b d3 7b 54 50 a1 57 
1ffff7f0: 39 e3 39 e3 
```

A Ch32v203 GBU6
```
1ffff700: 34 fe 78 dc 38 05 5b 30 2d c6 86 09 45 c5 9f 3e 
1ffff710: 15 a1 05 00 00 00 00 86 81 31 15 38 05 fa aa 55 
1ffff720: 75 05 1b 00 
Read 20 bytes
1ffff7e0: 80 00 ff ff ff ff ff ff cd ab de b7 78 bc 23 20 
1ffff7f0: 39 e3 39 e3 
```

Another CH32v203GBU6
```
1ffff700: 34 fe 78 dc 38 05 5b 30 2d c6 72 09 45 c5 9f 3e 
1ffff710: 15 a1 05 00 00 00 00 86 81 31 15 38 05 fa aa 55 
1ffff720: 7c 05 1b 00 
Read 20 bytes
1ffff7e0: 80 00 ff ff ff ff ff ff cd ab c9 b7 78 bc 0e 20 
1ffff7f0: 39 e3 39 e3 
```

A ch32v203c8t6
```
1ffff700: 34 fe 78 dc 00 05 31 20 29 86 6d 09 05 00 3e c1 
1ffff710: ff ff ff ff 00 00 08 86 81 31 15 38 05 fa aa 55 
1ffff720: 7e 05 18 00 
Read 20 bytes
1ffff7e0: 40 00 ff ff ff ff ff ff cd ab 09 db 6a bc 40 43 
1ffff7f0: 39 e3 39 e3 
```

A ch32v203c8t6
```
1ffff700: 34 fe 78 dc 00 05 31 20 29 86 62 09 05 00 3e c1 
1ffff710: ff ff ff ff 00 00 08 86 81 31 15 38 05 fa aa 55 
1ffff720: 88 05 18 00 
Read 20 bytes
1ffff7e0: 40 00 ff ff ff ff ff ff cd ab e5 d6 6a bc 1c 3f 
1ffff7f0: 39 e3 39 e3 
```

CH32X035F8U6
```
1ffff700: 34 fe 78 dc 11 06 5e 03 08 10 69 74 03 5a 00 00 
1ffff710: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 
1ffff720: ff ff ff ff 
Read 20 bytes
1ffff7e0: 3e 00 ff ff ff ff ff ff cd ab 31 28 45 bc 43 90 
1ffff7f0: ff ff ff ff 
```

CH32X035C8T6
```
1ffff700: 34 fe 78 dc 01 06 51 03 08 10 41 05 03 5a 00 00 
1ffff710: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 
1ffff720: ff ff ff ff 
Read 20 bytes
1ffff7e0: 3e 00 ff ff ff ff ff ff cd ab bc 1f 48 bc d1 87 
1ffff7f0: ff ff ff ff 
```

For a ch32v003 (QFN)
```
1ffff700: 07 f8 98 a3 91 bb 13 07 10 0f 4d b1 13 07 e0 0f 
1ffff710: 71 b9 33 86 e4 00 08 22 36 c6 3a c4 32 c2 b9 34 
1ffff720: 12 46 b2 46 
...
1ffff7e0: 10 00 ff ff ff ff ff ff cd ab 0c 78 0c bc e5 df 
1ffff7f0: ff ff ff ff
```

Another ch32v003 (QFN)
```
1ffff700: 12 00 00 00 03 03 09 04 88 06 00 00 0a 00 00 00 
1ffff710: 05 01 09 ff a1 01 75 08 95 7f 85 aa 09 ff b1 00 
1ffff720: c0 00 00 00 
Read 20 bytes
1ffff7e0: 10 00 ff ff ff ff ff ff cd ab 2d 6f 3d bc 37 d7 
1ffff7f0: ff ff ff ff 
```
For a ch32v003 (SOIC-8) (Different Lot)
```
1ffff700: 07 f8 98 a3 91 bb 13 07 10 0f 4d b1 13 07 e0 0f 
1ffff710: 71 b9 33 86 e4 00 08 22 36 c6 3a c4 32 c2 b9 34 
1ffff720: 12 46 b2 46 
Read 20 bytes
1ffff7e0: 10 00 ff ff ff ff ff ff cd ab 8d 3f c6 bc 20 a8 
1ffff7f0: ff ff ff ff 
```
