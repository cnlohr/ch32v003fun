# Demonstation of using TIM1 with DMA to generate two phase-shifted PWM signals.

This example shows using TIM1 with DMA to generate two phase-shifted PWM signals.
TIM1.CH3 controls PWM1_PIN (PC3) directly using the usual PWM modes.
TIM1.CH1 generates a DMA event that directly sets PWM2_PIN (PC6) high using the GPIO BSHR register,
and TIM1.CH2 generates a DMA event that directly sets PWM2_PIN (PC6) low.

There is a minimum phase shift of 1/8µs.

## Use

This uses the following pins:

| Pin | Function |
| --- | --- |
| PC3 | T1CH3 PWM output |
| PC6 | Phase-shifted PWM output|
