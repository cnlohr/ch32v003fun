# the deepest slumber

This example serves to show how to put the CH32V003 into its lowest power state (standby) and have it wake with a button press.  

Power consumption should be around 10uA.  

The MCU only toggles the LED and prints a message, then it goes back to sleep.  
The LED staying on demonstrates that GPIO keeps its state even when the rest of the mcu is in a coma.  

Based on the groundwork of Marek M.  

## circuit

Connect LED to PD4 (with resistor), connect button to GND and PD2.  
There is no debouncing but it should suffice for waking the chip.
