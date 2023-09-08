# the deepest slumber

**WARNING: You MUST hard-reboot the CH32V003 to allow it to go into deep sleep. You cannot go from flashing to deep sleep without a hard power cycle.**

This example serves to show how to put the CH32V003 into its lowest power state (standby) and have it wake with a button press.  

Power consumption should be around 10uA.

To enter 10uA standby mode you must perform these steps:

1. Put all GPIOs in input mode (thus no output state can be preserved).
2. Enable AFIO clock and set AFIO_EXTICR to the wakeup channel.
3. Configure EXTI event.
4. Set PWR_CTLR_PDDS bit in PWR_CTLR (Setting PWREN in RCC_APB1PCENR is not required hum?)
5. Set SLEEPDEEP bit in PFIC_SCTLR
6. Call __WFE() to enter standby mode.

Note:
* All GPIOs must be in input pull-up or pull-down mode before entering standby. Input floating mode result in 100uA current. 
* Once CH32V003 enters standby mode, it won't respond to any SWDIO command, therefor cannot be reprogrammed. User must provide a way to have the processor stay awake for reprogramming, e.g. some delay at startup.
* Debug circuitry will consume power. If minichlink terminal is active (including immediately after flashing), standby current will stay around 1.2mA until power cycle.

Based on the groundwork of Marek M.  

## circuit

Connect button to GND and PD2.  
There is no debouncing but it should suffice for waking the chip.
