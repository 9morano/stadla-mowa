

/* SWITCH on sheild must be to 5V, NOT to 3.3V....Then the readings go to 660 and center is at 320
 *
 * ---------------------------------------------
 * Measurements on 26.6.2020
 * 
 * Center value for X = 506
 * Center value for Y = 492
 * 
 *          Y = 1023
 *             ^
 *             |
 *  X = 0  <---+---> X = 1023
 *             |
 *             v
 *           Y = 0
 * 
*/

#define PIN_BUTTON_SELECT   8
#define PIN_BUTTON_F        7
#define PIN_BUTTON_E        6
#define PIN_BUTTON_LEFT     5
#define PIN_BUTTON_DOWN     4
#define PIN_BUTTON_RIGHT    3
#define PIN_BUTTON_UP       2

#define PIN_ANALOG_X        A0
#define PIN_ANALOG_Y        A1


void setup(){

    // Power-up delay
    delay(100);
    Serial.begin(112500);

}

void loop(){

    Serial.print("X=");
    Serial.print(analogRead(PIN_ANALOG_X));

    Serial.print(" ");

    Serial.print("Y=");
    Serial.println(analogRead(PIN_ANALOG_Y));
}