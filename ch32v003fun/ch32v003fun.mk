
PREFIX?=riscv64-unknown-elf

CH32V003FUN?=../../ch32v003fun
MINICHLINK?=../../minichlink

CFLAGS+= \
	-g -Os -flto -ffunction-sections \
	-static-libgcc \
	-march=rv32ec \
	-mabi=ilp32e \
	-I/usr/include/newlib \
	-I$(CH32V003FUN)/../extralibs \
	-I$(CH32V003FUN) \
	-nostdlib \
	-I. -Wall

LDFLAGS+=-T $(CH32V003FUN)/ch32v003fun.ld -Wl,--gc-sections -L$(CH32V003FUN)/../misc -lgcc

SYSTEM_C:=$(CH32V003FUN)/ch32v003fun.c
TARGET_EXT?=c

$(TARGET).elf : $(SYSTEM_C) $(TARGET).$(TARGET_EXT) $(ADDITIONAL_C_FILES)
	$(PREFIX)-gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(TARGET).bin : $(TARGET).elf
	$(PREFIX)-size $^
	$(PREFIX)-objdump -S $^ > $(TARGET).lst
	$(PREFIX)-objdump -t $^ > $(TARGET).map
	$(PREFIX)-objcopy -O binary $< $(TARGET).bin
	$(PREFIX)-objcopy -O ihex $< $(TARGET).hex

ifeq ($(OS),Windows_NT)
closechlink :
	-taskkill /F /IM minichlink.exe /T
else
closechlink :
	-killall minichlink
endif

terminal : monitor

monitor :
	$(MINICHLINK)/minichlink -T

gdbserver : 
	-$(MINICHLINK)/minichlink -baG

clangd :
	make clean
	bear -- make build

clangd_clean :
	rm -f compile_commands.json

cv_flash : $(TARGET).bin
	make -C $(MINICHLINK) all
	$(MINICHLINK)/minichlink -w $< flash -b

cv_clean :
	rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).lst $(TARGET).map $(TARGET).hex || true

build : $(TARGET).bin
