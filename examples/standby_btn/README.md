# the deepest slumber

**WARNING: You MUST hard-reboot the CH32V003 to allow it to go into deep sleep. You cannot go from flashing to deep sleep without a hard power cycle.**

This example serves to show how to put the CH32V003 into its lowest power state (standby) and have it wake with a button press.  

Power consumption should be around 9uA.

To enter 10uA standby mode you must perform these steps:

1. GPIOs other than the wake up pin can be set to either input or output mode (see notes).
2. Set GPIO(s) for wake up to input mode with appropriate pull up/down.  
3. Enable AFIO clock and set AFIO_EXTICR to the wakeup channel.
4. Configure EXTI event.
5. Set PWR_CTLR_PDDS bit in PWR_CTLR (Setting PWREN in RCC_APB1PCENR is not required hum?)
6. Set SLEEPDEEP bit in PFIC_SCTLR
7. Call __WFE() to enter standby mode.

Note:
* GPIOs in output mode will retain state during standby.
* GPIO if set to input mode must have internal or external pulling resistor. Floating input pin will cause 100uA standby current. 
* Once CH32V003 enters standby mode, it won't respond to any SWDIO command, therefor cannot be reprogrammed. User must provide a way to have the processor stay awake for reprogramming, e.g. some delay at startup.
* Debug circuitry will consume power. If minichlink terminal is active (including immediately after flashing), standby current will stay around 1.2mA until power cycle.

Based on the groundwork of Marek M.  

## circuit

Connect button to GND and PD2.  
There is no debouncing but it should suffice for waking the chip.
