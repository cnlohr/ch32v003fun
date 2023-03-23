TARGET:=blink

all : flash

PREFIX:=riscv64-unknown-elf

GPIO_Toggle:=EXAM/GPIO/GPIO_Toggle/User

EVT:=../../ch32v003evt

MINICHLINK:=../../minichlink

ifeq ($(OS),Windows_NT)
# On Windows, all the major RISC-V GCC installs are missing the -ec libgcc.
LIB_GCC=../../misc/libgcc.a
else
LIB_GCC=-lgcc
endif

CH32V003FUN:=../../ch32v003fun

CFLAGS:= \
	-g -Os -flto -ffunction-sections \
	-static-libgcc $(LIB_GCC) \
	-march=rv32ec \
	-mabi=ilp32e \
	-I/usr/include/newlib \
	-I$(CH32V003FUN) \
	-nostdlib \
	-I. -DTINYVECTOR -Wall

LDFLAGS:=-T $(CH32V003FUN)/ch32v003fun.ld -Wl,--gc-sections

SYSTEM_C:=$(CH32V003FUN)/ch32v003fun.c

$(TARGET).elf : $(SYSTEM_C) $(TARGET).c
	$(PREFIX)-gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(TARGET).bin : $(TARGET).elf
	$(PREFIX)-size $^
	$(PREFIX)-objdump -S $^ > $(TARGET).lst
	$(PREFIX)-objdump -t $^ > $(TARGET).map
	$(PREFIX)-objcopy -O binary $< $(TARGET).bin
	$(PREFIX)-objcopy -O ihex $< $(TARGET).hex

flash : $(TARGET).bin
	$(MINICHLINK)/minichlink -w $< flash -b

clean :
	rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).lst $(TARGET).map $(TARGET).hex

