TARGET:=barebones

all : flash

PREFIX:=riscv64-unknown-elf

GPIO_Toggle:=EXAM/GPIO/GPIO_Toggle/User

EVT:=../ch32v003evt/

CFLAGS:= \
	-g -Os -flto -ffunction-sections \
	-static-libgcc -lgcc \
	-march=rv32ec \
	-mabi=ilp32e \
	-I/usr/include/newlib \
	-I$(EVT) \
	-nostdlib \
	-I.

LDFLAGS:=-T $(EVT)/ch32v003.ld

SYSTEM_C:=$(EVT)/startup_ch32v00x.S

$(TARGET).elf : barebones.c $(SYSTEM_C)
	$(PREFIX)-gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(TARGET).hex : $(TARGET).elf
	$(PREFIX)-size $^
	$(PREFIX)-objdump -S $^ > $(TARGET).lst
	$(PREFIX)-objdump -t $^ > $(TARGET).map
	$(PREFIX)-objcopy -O binary $< $(TARGET).bin
	$(PREFIX)-objcopy -O ihex $< $@


flash : $(TARGET).hex

clean :
	rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).lst $(TARGET).map $(TARGET).hex

