#include "HX711.h"

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

#define CONTROL_READING 171800 // the values it seems to be reading while at rest
#define THRESH 28000 // at what point do we sense that strain is present? 

const int LOADCELL_DOUT_PIN = D4; 
const int LOADCELL_SCK_PIN = D3;
HX711 scale;

const int switch_input = D0;
const int door_input = D1;
const int open_input = D2;

const int power_relay = D5;
const int polarity_relay = D6;



/////////////////////////////   leds   //////////////////////////////////
const int num_leads = 2;
int leds[num_leads] = {D7, D8};

/////////////////////////////   setup   ////////////////////////////////////
void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // pinmodes for LEDs
  for(int pin = 0; pin < num_leads; pin++) {
    pinMode(leds[pin], OUTPUT);
  }

  pinMode(switch_input, INPUT);
  pinMode(door_input, INPUT);
  pinMode(open_input, INPUT);

  pinMode(power_relay, OUTPUT);
  pinMode(polarity_relay, OUTPUT);
}

void power_on() {
  digitalWrite(power_relay, HIGH);
}

void power_off() {
  digitalWrite(power_relay, LOW);
}

bool current_polarity = 0;
void spin_motor(bool polarity) { // 0 is forwards; 1 is backwards
  if (polarity == current_polarity) {
    power_on();
  }
  else {
    power_off();
    delay(10);
    digitalWrite(polarity_relay, polarity);
    delay(10);
    power_on();
  }
  current_polarity = polarity;
}
////////////////////////////////////////////////////////////////
long tension_time = 0;
long tension_period = 500; // half a second
bool last_updated_tension_present = false;

bool tension_present() {
  // check if strain sensor is responsive
  // for some reason, it seems to need about 100 ms between readings...
  long tension_reading;
  if (scale.is_ready()) {
    tension_reading = scale.read();
    Serial.print("HX711 reading: ");
    Serial.println(tension_reading);    
  }
  else {
    Serial.println("HX711 not found.");
    error_code(0); // red
    return last_updated_tension_present;
  }
  
  long normalized = tension_reading - CONTROL_READING;
  Serial.println("Normalized tension reading = " + String(normalized));
  

  if (abs(normalized) > THRESH) {
//    return true;
    last_updated_tension_present = true;
    tension_time = millis(); // reset the timer
  }
  else {
    if (tension_time - millis() > tension_period) {
  //    return false;
      last_updated_tension_present = false;
    }
  }
  return last_updated_tension_present;
}
////////////////////////////////////////////////////////////////////////////
void error_code(int code) {
  RGB_color(code);
//  switch(code) {
//    case 0 :
//      RGB(HIGH, LOW, LOW);
//      break;
//    case 1 :
//      RGB(LOW, HIGH, LOW);
//      break;
//    case 2 :
//      RGB(LOW, LOW, HIGH);
//      break;
//    case3 :
//      RGB(HIGH, HIGH, LOW);
//      break;
//    case 4 :
//      RGB(LOW, HIGH, HIGH);
//      break;
//    case 5 :
//      RGB(HIGH, LOW, HIGH);
//      break;
//    case 6 :
//      RGB(HIGH, HIGH, HIGH);
//      break;
//    case 7 :
//      RGB(LOW, LOW, LOW);
//      break;
//  }
//  return;
}

void RGB(bool red_light_value, bool green_light_value, bool blue_light_value) {
  digitalWrite(leds[0], red_light_value);
  digitalWrite(leds[1], green_light_value);
//  digitalWrite(leds[2], blue_light_value);
}

enum Color {
  red,
  green,
  blue,
  yellow,
  cyan,
  purple,
  white,
  off
};

//  0     1     2      3      4       5       6     7
// red, green, blue, yellow, cyan, magenta, white, off
 
void RGB_color(int c) { 
  switch(c) {
    case 0 :
      RGB(HIGH, LOW, LOW);
      break;
    case 1 :
      RGB(LOW, HIGH, LOW);
      break;
//    case 2 :
//      RGB(LOW, LOW, HIGH);
//      break;
    case 3 :
      RGB(HIGH, HIGH, LOW);
      break;
//    case 4 :
//      RGB(LOW, HIGH, HIGH);
//      break;
//    case 5 :
//      RGB(HIGH, LOW, HIGH);
//      break;
//    case 6 :
//      RGB(HIGH, HIGH, HIGH);
//      break;
    case 7 :
      RGB(LOW, LOW, LOW);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////
bool door_moving = false;
bool door_opening = false;
bool door_closing = false;
bool door_closed;
bool door_open;
bool switch_down;
bool tension;


long reset_time = 0;
long reset_period = 20 * 1000; // 20 seconds

void loop() {

  
  tension = tension_present();
  door_closed = 1 - digitalRead(door_input);
  switch_down = digitalRead(switch_input);
  door_open = 1 - digitalRead(open_input);

  Serial.println("Millis = " + String(millis()) + ",    tension = " + String(tension));
  Serial.println("door_closed = " + String(door_closed) + ",    switch_down = " + String(switch_down));
  
  if (millis() - reset_time > reset_period) { // reset after 20 seconds
    power_off();
    door_moving = false;
    door_opening = false;
    door_closing = false;
  }
  
  if(door_moving)  {
    if(door_closed)  {
      if(door_closing) {
        power_off();
        door_moving = false;
        door_opening = false;
        door_closing = false;
      }
      else if(door_opening) {
        RGB_color(1); // green
        //keep opening the door
      }
    }
    else if(door_open) {
      if(door_opening)   {
        power_off();
        door_moving = false;
        door_opening = false;
        door_closing = false;
      }
      else if(door_closing) {
        RGB_color(0); // red
        //keep closing the door
      }
    }
    
    else { // door_closed == false && door_open == false  
      if (tension) {
        RGB_color(3); //yellow   
  //      delay(10); // keep power flowing to the motor
      }
      else {
        power_off();
        RGB_color(7); // off
        door_moving = false;
      }
    }
  }
  else { // door_moving == false
    if (switch_down) {
      RGB_color(0);
      reset_time = millis(); // reset the timer
      door_moving = true;
      if (door_closed) {
        spin_motor(1); // open the door
        door_opening = true;
        door_closing = false;
      }
      else {
        spin_motor(0); // close the door
        door_opening = false;
        door_closing = true;
      }
    }
    else {
      RGB_color(7); // off
      delay(10);
    }
  }  
}

/////////////////////////////   button   ////////////////////////////////
bool state_for_toggle = HIGH;      // the current state of the output pin
bool previous_for_toggle = LOW;    // the previous reading from the input pin
long previous_time_for_toggle = 0;         // the last time the output pin was  toggled

bool state_for_interrupt = HIGH;
bool previous_for_interrupt = LOW;
long previous_time_for_interrupt = 0;
long debounce = 200;   // the debounce time, increase if the output flickers


class Button {
  public:
    bool state;
    bool prev;
    long lastTime;
    const long debounce = 200;
    int pin;

  Button(int pin) {
    this -> pin = pin;
    state = HIGH;
    prev = LOW;
    lastTime = 0;
  }

  bool getState() {
    return state;
  }

  void onPress() {
    state = 1 - state;
  }

  bool updateButton() {
    bool reading = digitalRead(pin);
    if (reading == HIGH && prev == LOW && millis() - lastTime > debounce) {
      onPress();
      lastTime = millis();
    }
    prev = reading;
  
    return state;
  }
  
};

class ToggleButton: public Button {

  void onPress() {
    state = 1 - state;
  }
  
};

class OneTimeButton: public Button {

  void onPress() {
    state = 1;
  }
  
};


Button button = Button(switch_input); // reference




bool toggle(int inPin) {
  bool reading = digitalRead(inPin);

  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit,  toggle the output pin and remember the time
  
  if (reading == HIGH && previous_for_toggle == LOW && millis() - previous_time_for_toggle > debounce) {
    state_for_toggle = 1 - state_for_toggle;
    previous_time_for_toggle = millis();
  }
  previous_for_toggle = reading;

  return state_for_toggle;
}
