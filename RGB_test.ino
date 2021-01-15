#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
//3V3
//GND
#define D5 14 
#define D6 12
#define D7 13
#define D8 15
#define RX 3
#define TX 1
//3V3
//GND

                //    led_1          led_2
int leds[2][3] = {{D2, D1, D0}, {D7, D6, D5}};


void setup() {
  for(int led = 0; led < 2; led++) {
    for(int pin = 0; pin < 3; pin++) {
      pinMode(leds[led][pin], OUTPUT);
    }
  }
}

void loop() {
 color_cycle(0);
 color_cycle(1);
}

void color_cycle(int led_index) {
  RGB_color(led_index, HIGH, LOW, LOW); // Red
  delay(1000);
  RGB_color(led_index, LOW, HIGH, LOW); // Green
  delay(1000);
  RGB_color(led_index, LOW, LOW, HIGH); // Blue
  delay(1000);
  RGB_color(led_index, HIGH, HIGH, HIGH); // Raspberry
  delay(1000);
  RGB_color(led_index, LOW, HIGH, HIGH); // Cyan
  delay(1000);
  RGB_color(led_index, HIGH, LOW, HIGH); // Magenta
  delay(1000);
  RGB_color(led_index, HIGH, HIGH, LOW); // Yellow
  delay(1000);
  RGB_color(led_index, HIGH, HIGH, HIGH); // White
  delay(1000);
}

void RGB_color(int led_index, bool red_light_value, bool green_light_value, bool blue_light_value)
 {
  digitalWrite(leds[led_index][0], red_light_value);
  digitalWrite(leds[led_index][1], green_light_value);
  digitalWrite(leds[led_index][2], blue_light_value);
}
