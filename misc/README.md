# Supplementary non-official material

This is a copy of libgcc.a that I know works well with the part. Below are the instructions on how to get the source code and how to build it.

# Extra totally non-official info

## GPIO Drive Currents

At 3.3v, it seems that for all speed configurations, the short drive current is 50mA both for emitter and colletor.  But the current falls off rather quickly with some voltage.

At 5v the peak current power application is 90-100mA for emitter and collector.

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

