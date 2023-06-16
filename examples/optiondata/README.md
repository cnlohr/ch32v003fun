# Write to the user fields in the Option Data flash

This is a simple example code for writing to the two 8-bit data0/data1 bytes that are a part of the Flash "User-selected words" (aka Option bytes).

To reduce the code footpint no timeouts are implemented in the code. It simply waits for the busy-flag to be cleaed. The two wait-loops could be replaced with a Delay_Ms(10) if speed is not an issue.

The two bytes could be used for remembering user settings between power-offs.

After flashing you can test it with the command
`../../minichlink/minichlink -a -b -T` 
that will Halt, Reboot and enter Terminal mode. For each run the count will increment by one.
