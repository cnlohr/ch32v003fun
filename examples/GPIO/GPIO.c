// blink, but with arduino-like HAL
// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "ch32v003fun.h"
#include "wiring_digital.h"
#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

uint32_t count;

int main() {
  SystemInit48HSI();

  // Enable GPIO ports
  enablePort(port_C);
  enablePort(port_D);

  for (int i = pin_C0; i <= pin_C7; i++) {
    pinMode(i, pinMode_O_pushPull);
  }

  // GPIO D4 Push-Pull
  pinMode(pin_D4, pinMode_O_pushPull);

  while (1) {
    // Turn on pins
    digitalWrite(pin_C0, high);
    digitalWrite(pin_D4, high);
    Delay_Ms(250);
    // Turn off pins
    digitalWrite(pin_C0, low);
    digitalWrite(pin_D4, low);
    Delay_Ms(250);
    for (int i = pin_C0; i <= pin_C7; i++) {
      digitalWrite(i, high);
      Delay_Ms(50);
    }
    for (int i = pin_C7; i >= pin_C0; i--) {
      digitalWrite(i, low);
      Delay_Ms(50);
    }
    Delay_Ms(250);
    count++;
  }
}
