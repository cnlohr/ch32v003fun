TARGET:=GPIO_Toggle

all : flash

PREFIX:=riscv64-unknown-elf

GPIO_Toggle:=EXAM/GPIO/GPIO_Toggle/User

CFLAGS:= \
	-Os -flto -ffunction-sections \
	-march=rv32emc \
	-mabi=ilp32e \
	-I/usr/include/newlib \
	-nostdlib \
	-I.

LDFLAGS:=-T ch32v003.ld

SYSTEM_C:=startup_ch32v00x.S system_ch32v00x.c

$(TARGET).elf : barebones.c $(SYSTEM_C)
	$(PREFIX)-gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(TARGET).hex : GPIO_Toggle.elf
	$(PREFIX)-size $^
	$(PREFIX)-objdump -S $^ > $(TARGET).lst
	$(PREFIX)-objdump -t $^ > $(TARGET).map
	#$(PREFIX)-objcopy -O binary $< $@
	$(PREFIX)-objcopy -O ihex $< $@

	
flash : $(TARGET).hex

clean :
	rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).lst $(TARGET).map

