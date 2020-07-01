

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



#define H1_IN1              7
#define H1_IN2              2
#define H1_IN3              3
#define H1_IN4              4
#define H1_PWM1             5
#define H1_PWM2             6

#define H2_IN1              0
#define H2_IN2              0
#define H2_IN3              0
#define H2_IN4              0
#define H2_PWM1             0   
#define H2_PWM2             0

void MOWA_motor_stop(void);
void MOWA_motor_go(int cmdX, int cmdY);
uint8_t MOWA_motor_forward(uint8_t speed);
uint8_t MOWA_motor_backward(uint8_t speed);
uint8_t MOWA_motor_right(uint8_t speed);
uint8_t MOWA_motor_left(uint8_t speed);


void setup(){

    // Power-up delay
    delay(100);
    Serial.begin(112500);

    pinMode(H1_IN1, OUTPUT);
    pinMode(H1_IN2, OUTPUT);
    pinMode(H1_IN3, OUTPUT);
    pinMode(H1_IN4, OUTPUT);
    pinMode(H1_PWM1, OUTPUT);
    pinMode(H1_PWM2, OUTPUT);

}

void loop(){
/*
    Serial.print("X=");
    Serial.print(analogRead(PIN_ANALOG_X));

    Serial.print(" ");

    Serial.print("Y=");
    Serial.println(analogRead(PIN_ANALOG_Y));
    */
   analogWrite(H1_PWM1, 225);
            analogWrite(H1_PWM2, 225);

    digitalWrite(H1_IN1, HIGH);
    digitalWrite(H1_IN2, LOW);

    digitalWrite(H1_IN3, HIGH);
    digitalWrite(H1_IN4, LOW);
    Serial.println("FORWARD");
    delay(5000);

    digitalWrite(H1_IN1, LOW);
    digitalWrite(H1_IN2, HIGH);

    digitalWrite(H1_IN3, LOW);
    digitalWrite(H1_IN4, HIGH);
    Serial.println("BACK");
    delay(5000);
}






void MOWA_motor_stop(void){
    // Set all inputs to same value to stop the mottor
    digitalWrite(H1_IN1, LOW);
    digitalWrite(H1_IN2, LOW);
    digitalWrite(H1_IN3, LOW);
    digitalWrite(H1_IN4, LOW);
    digitalWrite(H2_IN1, LOW);
    digitalWrite(H2_IN2, LOW);
    digitalWrite(H2_IN3, LOW);
    digitalWrite(H2_IN4, LOW);


    analogWrite(H1_PWM1, 0);
    analogWrite(H1_PWM2, 0);
    analogWrite(H2_PWM1, 0);
    analogWrite(H2_PWM2, 0);
}

void MOWA_motor_go(int cmdX, int cmdY){

    if ( (cmdX == 0) && (cmdY == 0) ){
        MOWA_motor_stop();
    }
    else if(cmdY > 0){
        MOWA_motor_forward(cmdY);
    }
    else if(cmdY < 0){
        MOWA_motor_backward(abs(cmdY));
    }
    else if(cmdX > 0){
        MOWA_motor_right(cmdX);
    }
    else if(cmdX < 0){
        MOWA_motor_left(abs(cmdX));
    }
}


/*
 *    H1-bridge
 * 
 *     +---------+
 *     |         |
 *  M1(in1)    M2(in2)
 *     |         |
 *     |         |
 *     |         |
 *  M3(in1)    M4(in2)
 *     |         |
 *     +---------+
 * 
 *     H2-bridge 
*/ 

// M1, M2, M3, M4 forward
uint8_t MOWA_motor_forward(uint8_t speed){
    // Set the speed (PWM)
    switch(speed){
        case 1:
            analogWrite(H1_PWM1, 125);
            analogWrite(H1_PWM2, 125);
            analogWrite(H2_PWM1, 125);
            analogWrite(H2_PWM2, 125);
            break;

        case 2:
            analogWrite(H1_PWM1, 225);
            analogWrite(H1_PWM2, 225);
            analogWrite(H2_PWM1, 225);
            analogWrite(H2_PWM2, 225);
            break;
        
        default:
            MOWA_motor_stop();
            return 0;
    }
    // Set the direction
    digitalWrite(H1_IN1, HIGH);
    digitalWrite(H1_IN2, LOW);

    digitalWrite(H1_IN3, HIGH);
    digitalWrite(H1_IN4, LOW);

    digitalWrite(H2_IN1, HIGH);
    digitalWrite(H2_IN2, LOW);

    digitalWrite(H2_IN3, HIGH);
    digitalWrite(H2_IN4, LOW);
    return 1;
}


// M1, M2, M3, M4 backward
uint8_t MOWA_motor_backward(uint8_t speed){
    // Set the speed (PWM)    
    switch(speed){
        case 1:
            analogWrite(H1_PWM1, 125);
            analogWrite(H1_PWM2, 125);
            analogWrite(H2_PWM1, 125);
            analogWrite(H2_PWM2, 125);
            break;

        case 2:
            analogWrite(H1_PWM1, 225);
            analogWrite(H1_PWM2, 225);
            analogWrite(H2_PWM1, 225);
            analogWrite(H2_PWM2, 225);
            break;
        
        default:
            MOWA_motor_stop();
            return 0;
    }

    // Set the direction
    digitalWrite(H1_IN1, LOW);
    digitalWrite(H1_IN2, HIGH);

    digitalWrite(H1_IN3, LOW);
    digitalWrite(H1_IN4, HIGH);

    digitalWrite(H2_IN1, LOW);
    digitalWrite(H2_IN2, HIGH);

    digitalWrite(H2_IN3, LOW);
    digitalWrite(H2_IN4, HIGH);
}


// Speed 1 --> M1 and M3 forward, M2 and M4 slow back
// Speed 2 --> M1 and M3 forward, M2 and M4 backward
uint8_t MOWA_motor_right(uint8_t speed){

    // Left motors to max speed
    analogWrite(H1_PWM1, 225);
    analogWrite(H2_PWM1, 225);

    switch(speed){
        case 1:
            // Right motors slow speed
            analogWrite(H1_PWM2, 100);
            analogWrite(H2_PWM2, 100);
            break;
        
        case 2:
            // Right motors to max speed
            analogWrite(H1_PWM2, 255);
            analogWrite(H2_PWM2, 255);
            break;

        default:
            MOWA_motor_stop();
            return 0;
    }

    // M1
    digitalWrite(H1_IN1, HIGH);
    digitalWrite(H1_IN2, LOW);
    // M3
    digitalWrite(H2_IN1, HIGH);
    digitalWrite(H2_IN2, LOW);
    // M2
    digitalWrite(H1_IN3, LOW);
    digitalWrite(H1_IN4, HIGH);
    // M4
    digitalWrite(H2_IN3, LOW);
    digitalWrite(H2_IN4, HIGH);

    return 1;
}


// Speed 1 --> M2 and M4 forward, M1 and M3 slow back
// Speed 2 --> M2 and M4 forward, M1 and M3 backward
uint8_t MOWA_motor_left(uint8_t speed){

    // Right motors to max speed
    analogWrite(H1_PWM2, 225);
    analogWrite(H2_PWM2, 225);

    switch(speed){
        case 1:
            // Left motors slow speed
            analogWrite(H1_PWM1, 100);
            analogWrite(H2_PWM1, 100);
        
        case 2:
            // Left motors to max speed
            analogWrite(H1_PWM1, 255);
            analogWrite(H2_PWM1, 255);
            
            break;

        default:
            MOWA_motor_stop();
            return 0;
    }

    // M2
    digitalWrite(H1_IN3, HIGH);
    digitalWrite(H1_IN4, LOW);
    // M4
    digitalWrite(H2_IN3, HIGH);
    digitalWrite(H2_IN4, LOW);
    // M1
    digitalWrite(H1_IN1, LOW);
    digitalWrite(H1_IN2, HIGH);
    // M3
    digitalWrite(H2_IN1, LOW);
    digitalWrite(H2_IN2, HIGH);

    return 1;
}
