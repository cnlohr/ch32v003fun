// Send the colour data for the entire strip of LEDs
// TODO add brightness control
void SHOWTIME(uint8_t BUFFER_LEDS[num_leds][3])
{
    // send LED info
    for (int i = 0; i < num_leds; i++) {
        LED_SendColour(BUFFER_LEDS[i][0], BUFFER_LEDS[i][1], BUFFER_LEDS[i][2]);
    }
    // Delay for minimum 50uS between pinwiggles to signal next frame to WS2812B LEDS
    Delay_Us(50);
}


void LED_OFF(void)
{
    for (int i = 0; i <  256; i++) {
        LED_SendColour(0, 0, 0);
    }
    Delay_Us(50);
}

void LED_RED(uint8_t brightness)
{
    for (int i = 0; i < num_leds; i++) {
        LED_SendColour(brightness, 0, 0);
    }
    Delay_Us(50);
    __WFI(); // uncomment to program static colour and go to sleep
}

void LED_GREEN(uint8_t brightness)
{
    for (int i = 0; i < num_leds; i++) {
        LED_SendColour(0, brightness, 0);
    }
    Delay_Us(50);
    __WFI(); // uncomment to program static colour and go to sleep
}

void LED_BLUE(uint8_t brightness)
{
    for (int i = 0; i < num_leds; i++) {
        LED_SendColour(0, 0, brightness);
    }
    Delay_Us(50);
    __WFI(); // uncomment to program static colour and go to sleep
}

void LED_WARM_WHITE(void)
{
    for (int i = 0; i < num_leds; i++) {
        LED_SendColour(255, 100, 50);
    }
    Delay_Ms(150);
    __WFI(); // uncomment to program static colour and go to sleep
}

void LED_COLD_WHITE(void)
{
    for (int i = 0; i < num_leds; i++) {
        LED_SendColour(255, 180, 235);
    }
    Delay_Ms(150);
    __WFI(); // uncomment to program static colour and go to sleep
}

void LED_CUSTOM(uint16_t red, uint16_t green, uint16_t blue)
{
    for (int i = 0; i < num_leds; i++) {
        LED_SendColour(red, green, blue);
    }
    Delay_Us(150);
    __WFI(); // uncomment to program static colour and go to sleep
}

void LED_RGB_FLASH(uint16_t speed, uint8_t brightness)
{
    for (int i = 0; i < num_leds; i++) {
    LED_SendColour(brightness, 0, 0); }
    Delay_Ms(speed);
    for (int i = 0; i < num_leds; i++) {
    LED_SendColour(0, brightness, 0); }
    Delay_Ms(speed);
    for (int i = 0; i < num_leds; i++) {
    LED_SendColour(0, 0, brightness);}
    Delay_Ms(speed);

}

// used by RAINBOWS
uint8_t * Wheel(uint8_t WheelPos) {
  static uint8_t c[3];

  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }
  return c;
}
void LED_RAINBOWS(uint16_t speed, uint16_t width) {
  uint8_t *c;
  uint16_t i, j;

  for(j=0; j<256; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< width; i++) {
      c=Wheel(((i * 256 / width) + j) & 255);
      BUFFER_LEDS[i][0] = *c ;
      BUFFER_LEDS[i][1] = *(c+1) ;
      BUFFER_LEDS[i][2] = *(c+2) ;
    }
    SHOWTIME(BUFFER_LEDS);
   Delay_Ms(speed);
  }
}

// This function applies the sine wave to the LED colours and updates the buffer
void LED_WHITE_FADE(uint16_t speed)
{
    // Increment count for sine wave animation
    if (flag==0) {
        count++;
    }
    if (flag==1) {
        count--;
        }
    if (count >= 85) {
        count = 85;
    flag = 1;
    }
    if (count <= 10) {
        count = 10;
        flag = 0;
    }

    // Apply the sine wave to the LED colors
    for (int i = 0; i < num_leds; i++) {

        /////////////////////////////////////////////
        // Update the LED colour values with the mapped
        // sine wave value
        /////////////////////////////////////////////

        // set red value to sine wave value
        BUFFER_LEDS[i][0] = sine_lookup_2[count];
        // set green value to sine wave value
        BUFFER_LEDS[i][1] = sine_lookup_2[count];
        // set blue value to sine wave value
        BUFFER_LEDS[i][2] = sine_lookup_2[count];
    }

    SHOWTIME(BUFFER_LEDS);
    Delay_Ms(speed);
}

void LED_WHITE_FLASH (uint16_t max, uint16_t min)
{
    BUFFER_LEDS[0][0] = 200;
    BUFFER_LEDS[0][1] = 200;
    BUFFER_LEDS[0][2] = 200;
    BUFFER_LEDS[1][0] = 0;
    BUFFER_LEDS[1][1] = 0;
    BUFFER_LEDS[1][2] = 0;

    // Show the updated buffer on the LED strip
    SHOWTIME(BUFFER_LEDS);
    Delay_Ms(count);
    BUFFER_LEDS[0][0] = 0;
    BUFFER_LEDS[0][1] = 0;
    BUFFER_LEDS[0][2] = 0;
    BUFFER_LEDS[1][0] = 200;
    BUFFER_LEDS[1][1] = 200;
    BUFFER_LEDS[1][2] = 200;

    SHOWTIME(BUFFER_LEDS);
    Delay_Ms(count);

    if (flag) {
        count++;
        if (count > max) {
        flag=0;
        }
    }
    else {
        count--;
        if (count < min) {
        flag=1;
        }
    }
}
