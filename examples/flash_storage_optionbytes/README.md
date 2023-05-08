# FLASH STORAGE: option bytes
Ever wanted to store user config variables in non-volatile memory so they can be restored after power loss?
Now you can; without an external flash chip!

The CH32V003 has option bytes that, when combined, can store 16 entire bits!
For some applications is sufficient, so here you go:

The catch: severely limited write cycles (flash endurance) and, well, 16 bits.
Still usefull if the values don't change often and you don't really need more.

Further information and details on usage in `ch32v003_flash.h`.

## origin:
This library has been written from the register descriptions in the reference manual and is NOT a copy of the manufacturer library.
