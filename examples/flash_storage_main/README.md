# FLASH STORAGE: MAIN
Ever wanted to store user config variables in non-volatile memory so they can be restored after power loss?
Now you can; without an external flash chip!

The CH32V003 has 16K of flash for the main program and we can section off a portion of that to store data generated during run time, like calibration values or user preferences.

The catch: severely limited write cycles (flash endurance) and all values have to be written after preparing for programming (erasing).

But for values that don't change often it's perfect!

Further information and details on usage in `ch32v003_flash.h`.

## origin:
This library has been written from the register descriptions in the reference manual and is NOT a copy of the manufacturer library.
