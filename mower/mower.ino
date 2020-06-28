//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define SHITTY_RADIO_DELAY()   delay(5);

#define RADIO_CHECK_TIME    5000    // in ms
#define DEBUG               1
#define TEST                0

#define H1_IN1              1
#define H1_IN2              1
#define H1_IN3              1
#define H1_IN4              1
#define H1_PWM1             1
#define H1_PWM2             1

#define H2_IN1              1
#define H2_IN2              1
#define H2_IN3              1
#define H2_IN4              1
#define H2_PWM1             1   
#define H2_PWM2             1




uint32_t radioChekcTimer;
uint32_t radioNotAvailable = 1;
uint8_t  transmitterNotAvailable = 1;
uint32_t noRadioTimer;


//create an RF24 object
RF24 radio(9, 8);  // CE, CSN

struct dataStruct{
    int joystick[2];      // joystick[0] = X, joystick[1] = Y
} dataRx, dataTx;




void     MOWA_radio_reset();
uint32_t MOWA_radio_check();

void MOWA_motor_stop(void);
void MOWA_motor_go(int cmdX, int cmdY);
uint8_t MOWA_motor_forward(uint8_t speed);
uint8_t MOWA_motor_backward(uint8_t speed);
uint8_t MOWA_motor_right(uint8_t speed);
uint8_t MOWA_motor_left(uint8_t speed);


void setup()
{
    // Power-up delay
    delay(100);
    Serial.begin(115200);
    Serial.println("Mower!");

    MOWA_radio_reset();


}

void loop() {

    uint8_t ack;

    // Every x seconds verify the configuration of the radio.
    if(millis() - radioChekcTimer > RADIO_CHECK_TIME){
       radioChekcTimer = MOWA_radio_check();
    }

    // If faliure detected, reset it
    if(radio.failureDetected){
        delay(250);
        Serial.println("Radio failure detected, restarting radio");
        MOWA_radio_reset();
        delay(100);    
        radio.failureDetected = false;
        radioNotAvailable = 0;
        // Restart the "radio check timer" so it doesn't restart it right again 
        noRadioTimer = millis();
    }

    // If we received any packet, read it
    if (radio.available())
    {
        radioNotAvailable = 0;

        // Read the received data
        radio.read(&dataRx, sizeof(dataRx));

    #if DEBUG
        Serial.print("X=");
        Serial.print(dataTx.joystick[0]);
        Serial.print(" Y=");
        Serial.println(dataTx.joystick[1]);
    #endif

        // Wait a bit to transmit ACK till the end
        SHITTY_RADIO_DELAY();

        // Set to transmitter mode
        radio.stopListening();

        // Wait a bit for state transition
        SHITTY_RADIO_DELAY();

        // Send response back to RC - same values as conformation
        dataTx.joystick[0] = dataRx.joystick[0];
        dataTx.joystick[1] = dataRx.joystick[1];

        ack = radio.write(&dataTx, sizeof(dataTx));

    #if DEBUG
        if(!ack){
            Serial.println("------- No ACK");
        }
    #endif

        // A bit of delay to receive ACK
        SHITTY_RADIO_DELAY();

        // Back to receiving mode
        radio.startListening();

        /* TODO Test it on the end
        if(radio.available()){
            // TODO Not sure if this is OK --> maybe RC already sent something.
            // If radio.available() returns 1 more times, even after we read the data, there is something wrong
            Serial.println("--ERROR-------");
            radio.failureDetected = true;
        }
        */
    }
    // If radio not available
    else
    {
        radioNotAvailable++;
    }


    // If we received some data
    if(radioNotAvailable == 0){

        MOWA_motor_go(dataRx.joystick[0], dataRx.joystick[1]);

        // Every time we receive something, reset the timer
        noRadioTimer = millis();

        // Indicate that transmitter is here
        transmitterNotAvailable = 0;
    }

    // If radio is not avaliable for some time, something is wrong with transmitter
    // Turn off everything and hope for the best
    if(millis() - noRadioTimer  > 200){
        Serial.print("----- CRITICAL WARNING ");
        Serial.println(radioNotAvailable);

        // Restart the "radio check timer" so it doesn't restart it right again 
        noRadioTimer = millis();

        transmitterNotAvailable++;

        if(transmitterNotAvailable > 4){
            // After 5 times (second or so) try reseting the radio
            radio.failureDetected = true;
            transmitterNotAvailable = 1;      
        }
    }



}








void MOWA_radio_reset(){

    // Address through which two modules communicate.
    byte addresses[][6] = {"1Node","2Node"};

    radio.begin();

    // Set channel (0-125)
    radio.setChannel(20);
    // Enabled by default
    radio.setAutoAck(1);
    // Transmission power
    radio.setPALevel(RF24_PA_MAX);
    // Possible RF24_1MBPS and RF24_250KBPS
    radio.setDataRate(RF24_1MBPS);
    // Max CRC size
    radio.setCRCLength(RF24_CRC_16);

    //set the address
    radio.openReadingPipe(1, addresses[0]);
    radio.openWritingPipe(addresses[1]);

    //Set module as receiver
    radio.startListening();
}

uint32_t MOWA_radio_check(){

    if(radio.getDataRate() != RF24_1MBPS){
      radio.failureDetected = true;
      Serial.print("Radio configuration error detected");
    }

    return millis();
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
        MOWA_motor_backward(cmdY);
    }
    else if(cmdX > 0){
        MOWA_motor_right(cmdX);
    }
    else if(cmdX < 0){
        MOWA_motor_left(cmdX);
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
