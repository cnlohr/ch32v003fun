#include "ch32v003fun.h"
#include "fun_timer.h"
#include "fun_gpio.h"
#include <stdio.h>

#define TICK_DEBOUNCE_DUR 10
#define TICK_CLICK_DUR 30

enum {
   BUTTON_LO = 0,
   BUTTON_HI,
   BUTTON_LO2,
   BUTTON_HI2,
   BUTTON_END
};

uint32_t ref_debounce = 0;
uint32_t ref_click = 0;
uint8_t ref_read = 1;
uint8_t ref_state = BUTTON_END;
uint8_t button_pin = 0xFF;

void button_onSingleClick();
void button_onDoubleClick();

void button_setup(uint8_t pin) {
   if (pin == 0xFF) return; 
   button_pin = pin;

	pinMode(pin, INPUT_PULLUP);
   timer_setup();
   _reset_timers(BUTTON_END);
}

void _reset_timers(uint8_t newState) {
   ref_state = newState;
   ref_debounce = timer_getTick();
   ref_click = timer_getTick();
}

void button_run() {
   if (button_pin == 0xFF) return;

   uint32_t currentTick = timer_getTick();
   uint32_t tickCount = timer_getCount();
   uint8_t read = digitalRead(button_pin);

   uint8_t checkDebounce = currentTick - ref_debounce > TICK_DEBOUNCE_DUR;
   //! for debugging
   // Delay_Ms(1000);
   // printf( "\nMilliseconds: %lu / CNT: %lu", currentTick, tickCount );
   // printf("\nread = %d", read);

   if (read == 1 && ref_state == BUTTON_END) {
      // read == 1 button contact is opened
      _reset_timers(BUTTON_HI);
   }

   else if (ref_state == BUTTON_HI && checkDebounce) {
      if (read) {
         // TODO: 
      } else {
         _reset_timers(BUTTON_LO);
      }
   }

   else if (ref_state == BUTTON_LO && checkDebounce) {
      if (read) {
         // button contacts are opened
         _reset_timers(BUTTON_HI2);
      } else {
         // button contacts are closed
         if (currentTick - ref_click > TICK_CLICK_DUR) {
            button_onSingleClick();
            ref_state = BUTTON_END;
         }
      }
   }

   else if (ref_state == BUTTON_HI2 && checkDebounce) {
      if (read) {
         // when button contact is opened
         if (currentTick - ref_click > TICK_CLICK_DUR) {
            button_onSingleClick();
            ref_state = BUTTON_END;
         }
      } else {
         // button contacts are closed
         _reset_timers(BUTTON_LO2);
      }
   }

   else if (ref_state == BUTTON_LO2 && checkDebounce) {
      if (currentTick - ref_click > TICK_CLICK_DUR) {
         button_onDoubleClick();
         ref_state = BUTTON_END;
      }
   }

   else {
      // TODO: 
   }
}