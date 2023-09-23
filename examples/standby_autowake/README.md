# the deepest slumber

**WARNING: You MUST hard-reboot the CH32V003 to allow it to go into deep sleep. You cannot go from flashing to deep sleep without a hard power cycle.**

This example serves to show how to put the CH32V003 into its lowest power state (standby) and have it wake periodically.  

Power consumption should be around 10uA.  

Refer to the standby_btn example for GPIO settings.

Based on the groundwork of Marek M.  

## time calculations

The autowakeup delay can be calculated by:

`t = AWUWR / (fLSI / AWUPSC)`  

Where AWUWR can be 1 to 63, fLSI is always 128000 and AWUPSC, for practical purposes is 2048, 4096, 10240 or 61440, though lower values are possible.  
The maximum autowakeup delay is 30s.
