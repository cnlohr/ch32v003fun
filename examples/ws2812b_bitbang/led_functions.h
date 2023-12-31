// Initialise the variables for animation
uint8_t  flag  = 0;
uint32_t count = 1;

// Send the color data for the entire strip of LEDs
// TODO add brightness control
void showtime(uint8_t BUFFER_LEDS[num_leds][3])
{
    // send LED info
    for (int i = 0; i < num_leds; i++) {
        led_send_color(BUFFER_LEDS[i][0], BUFFER_LEDS[i][1], BUFFER_LEDS[i][2]);
    }
    // Delay for minimum 50uS between pinwiggles to signal next frame to WS2812B LEDS
    Delay_Us(50);
}


void led_off(void)
{
    for (int i = 0; i <  256; i++) {
        led_send_color(0, 0, 0);
    }
    Delay_Us(50);
}

void led_red(uint8_t brightness)
{
    for (int i = 0; i < num_leds; i++) {
        led_send_color(brightness, 0, 0);
    }
    Delay_Us(50);
    __WFI(); // uncomment to program static color and go to sleep
}

void led_green(uint8_t brightness)
{
    for (int i = 0; i < num_leds; i++) {
        led_send_color(0, brightness, 0);
    }
    Delay_Us(50);
    __WFI(); // uncomment to program static color and go to sleep
}

void led_blue(uint8_t brightness)
{
    for (int i = 0; i < num_leds; i++) {
        led_send_color(0, 0, brightness);
    }
    Delay_Us(50);
    __WFI(); // uncomment to program static color and go to sleep
}

void led_warm_white(void)
{
    for (int i = 0; i < num_leds; i++) {
        led_send_color(255, 100, 50);
    }
    Delay_Ms(150);
    __WFI(); // uncomment to program static color and go to sleep
}

void led_cold_white(void)
{
    for (int i = 0; i < num_leds; i++) {
        led_send_color(255, 180, 235);
    }
    Delay_Ms(150);
    __WFI(); // uncomment to program static color and go to sleep
}

void led_custom(uint16_t red, uint16_t green, uint16_t blue)
{
    for (int i = 0; i < num_leds; i++) {
        led_send_color(red, green, blue);
    }
    Delay_Us(150);
    __WFI(); // uncomment to program static color and go to sleep
}

void led_rgb_flash(uint16_t speed, uint8_t brightness)
{
    for (int i = 0; i < num_leds; i++) {
    led_send_color(brightness, 0, 0); }
    Delay_Ms(speed);
    for (int i = 0; i < num_leds; i++) {
    led_send_color(0, brightness, 0); }
    Delay_Ms(speed);
    for (int i = 0; i < num_leds; i++) {
    led_send_color(0, 0, brightness);}
    Delay_Ms(speed);

}

// used by RAINBOWS
uint8_t * wheel(uint8_t wheel_pos) {
  static uint8_t c[3];

  if(wheel_pos < 85) {
   c[0]=wheel_pos * 3;
   c[1]=255 - wheel_pos * 3;
   c[2]=0;
  } else if(wheel_pos < 170) {
   wheel_pos -= 85;
   c[0]=255 - wheel_pos * 3;
   c[1]=0;
   c[2]=wheel_pos * 3;
  } else {
   wheel_pos -= 170;
   c[0]=0;
   c[1]=wheel_pos * 3;
   c[2]=255 - wheel_pos * 3;
  }
  return c;
}

void led_rainbows(uint16_t speed, uint16_t width)
{
  uint8_t *c;
  uint16_t i, j;

  for(j=0; j<256; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< width; i++) {
      c=wheel(((i * 256 / width) + j) & 255);
      BUFFER_LEDS[i][0] = *c ;
      BUFFER_LEDS[i][1] = *(c+1) ;
      BUFFER_LEDS[i][2] = *(c+2) ;
    }
    showtime(BUFFER_LEDS);
    Delay_Ms(speed);
  }
}

// This function applies the sine wave to the LED colors and updates the buffer
void led_white_fade(uint16_t speed)
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
        // Update the LED color values with the mapped
        // sine wave value
        /////////////////////////////////////////////

        // set red value to sine wave value
        BUFFER_LEDS[i][0] = sine_lookup_2[count];
        // set green value to sine wave value
        BUFFER_LEDS[i][1] = sine_lookup_2[count];
        // set blue value to sine wave value
        BUFFER_LEDS[i][2] = sine_lookup_2[count];
    }

    showtime(BUFFER_LEDS);
    Delay_Ms(speed);
}

void led_white_flash(uint16_t max, uint16_t min)
{
    BUFFER_LEDS[0][0] = 200;
    BUFFER_LEDS[0][1] = 200;
    BUFFER_LEDS[0][2] = 200;
    BUFFER_LEDS[1][0] = 0;
    BUFFER_LEDS[1][1] = 0;
    BUFFER_LEDS[1][2] = 0;

    // Show the updated buffer on the LED strip
    showtime(BUFFER_LEDS);
    Delay_Ms(count);
    BUFFER_LEDS[0][0] = 0;
    BUFFER_LEDS[0][1] = 0;
    BUFFER_LEDS[0][2] = 0;
    BUFFER_LEDS[1][0] = 200;
    BUFFER_LEDS[1][1] = 200;
    BUFFER_LEDS[1][2] = 200;

    showtime(BUFFER_LEDS);
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
