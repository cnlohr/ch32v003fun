# Demonstration of a 3-phase gimbal motor controller

![gimbal working](https://github.com/cnlohr/ch32v003fun/blob/master/examples/bldc_gimbal/gimbal_working.jpg?raw=true)

This demo uses:
 * TIM2 PWM
 * ADC triggered from PWM (to comb-filter any timer aligned noise)
 * Sys Tick Interrupt for Gimbal Motor controller
 * Handling of input from the user over the debug interface.

This is designed to run ultra low current 3-phase motors (I.e. gimbals) with DC resistance of >=10 ohms.  With motors like this you typically can drive them directly off the GPIO on the ch32v003.

This specific design is designed to use GPIO's - connect motor to PD5, PD6 and PC7.

It uses PD6 to the sensing to detect if the stater is moving.

This demonstrates usage of the ADC being triggered off of a timer, and systick interrupts.

Currently demonstrates speed control, but can be trivially converted to positional control if there was a way to close the loop, i.e. with a gyro or accelerometer.

## Tested motors:
 * BDUAV - 2204 - 260KV - tested to ~45RPM
 * MiToot - 2206 - 100T - tested to ~20RPM


