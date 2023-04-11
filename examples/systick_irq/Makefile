TARGET:=systick_irq

all : flash

PREFIX:=riscv64-unknown-elf

GPIO_Toggle:=EXAM/GPIO/GPIO_Toggle/User

CH32V003FUN:=../../ch32v003fun
MINICHLINK:=../../minichlink

CFLAGS:= \
	-g -Os -flto -ffunction-sections \
	-static-libgcc \
	-march=rv32ec \
	-mabi=ilp32e \
	-I/usr/include/newlib \
	-I$(CH32V003FUN) \
	-nostdlib \
	-I. -DSTDOUT_UART -Wall

LDFLAGS:=-T $(CH32V003FUN)/ch32v003fun.ld -Wl,--gc-sections -L../../misc -lgcc

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

