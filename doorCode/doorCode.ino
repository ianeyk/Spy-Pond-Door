#include "HX711.h"

#define CONTROL_READING 8000 // the values it seems to be reading while at rest
#define THRESH 4000       // at what point do we sense that strain is present?

// #define SCK 18
// #define SDA 5
// #define SCK 5
// #define SDA 18


// #define SCK 34
// #define SDA 5
#define SCK 5 // these last two work, but remember to plug it in!!!
#define SDA 34

const int LOADCELL_DOUT_PIN = SDA;
const int LOADCELL_SCK_PIN = SCK;
HX711 scale;

/////////////////////////////   leds   //////////////////////////////////
const int numLeds = 3;
#define RED_LED 25
#define GREEN_LED 26
#define BLUE_LED 27
const int leds[numLeds] = {RED_LED, GREEN_LED, BLUE_LED};

#define SWITCH_INPUT 15
#define DOOR_INPUT 2
#define OPEN_INPUT 4

#define POWER_RELAY 22
#define POLARITY_RELAY 23

enum Color
{
    red = 0,
    green = 1,
    blue = 2,
    yellow = 3,
    cyan = 4,
    purple = 5,
    white = 6,
    off = 7
};

void RGB_color(Color c)
{ // sets LED pins based on Color object: red, green, blue, yellow, cyan, putple, white, off
    switch (c)
    {
    case red:
        RGB(HIGH, LOW, LOW);
        break;
    case green:
        RGB(LOW, HIGH, LOW);
        break;
    case blue:
        RGB(LOW, LOW, HIGH);
        break;
    case yellow:
        RGB(HIGH, HIGH, LOW);
        break;
    case cyan:
        RGB(LOW, HIGH, HIGH);
        break;
    case purple:
        RGB(HIGH, LOW, HIGH);
        break;
    case white:
        RGB(HIGH, HIGH, HIGH);
        break;
    case off:
        RGB(LOW, LOW, LOW);
        break;
    };
}

void RGB(bool r, bool g, bool b)
{ // digitalWrite to LED pins
    bool ledValues[3] = {r, g, b};
    for (int i = 0; i < numLeds; i++)
    {
        digitalWrite(leds[i], ledValues[i]);
    }
}

enum pol
{
    opening = 1,
    closing = 0
};

bool door_moving = false;

pol polarity = closing;

bool door_closed;
bool fully_open;
bool switch_pressed;
bool tension;

void setup()
{ // start serial monitor and set pin modes
    Serial.begin(115200);
    Serial.println("Hello world!");
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

    // pinmodes for LEDs
    for (int pin = 0; pin < numLeds; pin++)
    {
        pinMode(leds[pin], OUTPUT);
    }

    // pinmodes for switches and relays
    pinMode(SWITCH_INPUT, INPUT);
    pinMode(DOOR_INPUT, INPUT);
    pinMode(OPEN_INPUT, INPUT);

    pinMode(POWER_RELAY, OUTPUT);
    pinMode(POLARITY_RELAY, OUTPUT);
}

void power_on()
{ // digitalWrite to the power pin
    digitalWrite(POWER_RELAY, HIGH);
    door_moving = true;
}

void power_off()
{ // digitalWrite to the power pin // also sets door_moving to false
    digitalWrite(POWER_RELAY, LOW);
    door_moving = false;
}

pol current_polarity = closing;
void spin_motor()
{ // calls power_on with the proper polarity according to the global polarity variable
    if (polarity == current_polarity)
    {
        power_on();
    }
    else
    {
        power_off();
        delay(10);
        digitalWrite(POLARITY_RELAY, polarity);
        delay(10);
        power_on();
    }
    current_polarity = polarity;
}

////////////////////////////////////////////////////////////////
long tension_time = 0;      // initial millis()
long tension_period = 1000; // debouncing period for tension sensor
bool last_updated_tension_present = false;

long tension_reading;
long normalized;

bool tension_present()
{ // reads and debounces strain sensor; returns true if tension is detected
    // check if strain sensor is responsive
    // for some reason, it seems to need about 100 ms between readings...
    if (scale.is_ready())
    {
        tension_reading = scale.read();
    }
    else
    {
        Serial.println("HX711 not found.");
        RGB_color(yellow);
        return last_updated_tension_present;
    }

    normalized = tension_reading - CONTROL_READING;
    //  Serial.println("Normalized tension reading = " + String(normalized));

    if (abs(normalized) > THRESH)
    {
        //    return true;
        last_updated_tension_present = true;
        tension_time = millis(); // reset the timer
    }
    else
    {
        if (tension_time - millis() > tension_period)
        {
            //    return false;
            last_updated_tension_present = false;
        }
    }

    Serial.print("HX711 reading: ");
    Serial.print(tension_reading);
    Serial.print(",   tension_present: ");
    Serial.println(last_updated_tension_present);

    return last_updated_tension_present;
}

////////////////////////////////////////////////////////////////////////////

long reset_time = 0;
long reset_period = 20 * 1000; // 30 seconds

void loop()
{

    // read sensor and switches

    tension = tension_present();                // check if tension is present on the strain sensor
    door_closed = 1 - digitalRead(DOOR_INPUT);  // switch at top of door, door_closed == 1 when door closed
    switch_pressed = digitalRead(SWITCH_INPUT); // switch being pressed , switch_pressed == 1 when switch is pressed
    fully_open = 1 - digitalRead(OPEN_INPUT);   // switch at  maximum extension, fully_open == 1 when fully open

    // Serial.println("Millis = " + String(millis()) + ",    Polarity = " + String(polarity) + ",    tension = " + String(tension));
    // Serial.println("door_closed = " + String(door_closed) + ",    switch_pressed = " + String(switch_pressed) + ",    fully_open = " + String(fully_open));

    if (fully_open)
    {
        RGB_color(white);
    }
    else if (switch_pressed)
    {
        RGB_color(cyan);
    }
    else if (door_closed)
    {
        RGB_color(purple);
    }

    if (millis() - reset_time > reset_period)
    { // reset after 20 second timeout period
        power_off();
    }

    if (door_moving)
    {

        if (door_closed)
        { // while the switch at top of door is pressed:
            if (polarity == closing)
            { // if it's in closing mode, it has reached its destination
                power_off();
            }
            else if (polarity == opening)
            { // if it's in opening mode, the door has just started opening
                // do nothing and keep opening the door
                RGB_color(green);
            }
        }

        if (fully_open)
        {
            if (polarity == opening)
            { // if it's in opening mode, it has reached its destination
                power_off();
            }
            else if (polarity == closing)
            { // if it's in closing mode, the door has just started closing
                // do nothing and keep closing the door
                RGB_color(red);
            }
        }

        if (door_closed == false && fully_open == false)
        {
            if (tension)
            {
                // do nothing and keep the door moving
                RGB_color(blue);
            }
            else
            {
                power_off();
                // if tension is lost, stop the motor
                // do not change the polarity
                RGB_color(off);
            }
        }
    }

    else
    { // NOT door_moving

        if (switch_pressed)
        {
            reset_time = millis(); // reset the timer

            if (door_closed)
            {
                polarity = opening;
                spin_motor(); // open the door
            }

            else
            { // NOT door_closed
                polarity = closing;
                spin_motor(); // close the door
            }
        }

        else
        { // NOT switch_pressed
            RGB_color(off);
            delay(10);
        }
    }
    delay(10);
}
