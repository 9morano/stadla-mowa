/*
 *
 *
 *
 *
*/




//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Because of shitty radio I added delays between state transitions and ACK reception
#define SHITTY_RADIO_DELAY()   delay(5);

#define RESPONSE_TIMEOUT    150000  // in us
#define RADIO_CHECK_TIME    5000    // in ms
#define PIN_JOYSTICK_X      A0
#define PIN_JOYSTICK_Y      A1

#define DEBUG               1
#define TEST                0


uint32_t rxTimestamp, txTimestamp;
uint32_t radioChekcTimer;



//create an RF24 object
RF24 radio(9, 8);  // CE, CSN


// TODO why only float works, uint32_t displays random shit after number 9..
struct dataStruct{
    float joystick[2];      // joystick[0] = X, joystick[1] = Y
} dataRx, dataTx;


void     MOWA_radio_reset();
uint32_t MOWA_radio_check();

void     MOWA_joystick_get();



void setup()
{  
    // Power-up delay
    delay(100);
    Serial.begin(115200);
    Serial.println("Transmitter!");

    MOWA_radio_reset();

    radioChekcTimer = millis();
}

void loop()
{
    uint8_t ack, response;

    // Every x seconds verify the configuration of the radio.
    if(millis() - radioChekcTimer > RADIO_CHECK_TIME){
       radioChekcTimer = MOWA_radio_check();
    }

    // If radio failure detected, reset it
    if(radio.failureDetected){
        radio.failureDetected = false;
        delay(250);
        Serial.println("Radio failure detected, restarting radio");
        MOWA_radio_reset();        
    }


    // Set module as transmitter
    radio.stopListening();

    MOWA_joystick_get();

#if DEBUG
    Serial.print("X =");
    Serial.print(dataTx.joystick[0]);
    Serial.print("Y =");
    Serial.println(dataTx.joystick[1]);
#endif

    // Send the data
    txTimestamp = micros();
    ack = radio.write(&dataTx, sizeof(dataTx));

#if DEBUG
    if(!ack){
        Serial.println("------- No ACK");
    }
#endif

    // A bit of delay to receive ACK 
    SHITTY_RADIO_DELAY();

    // Set to receiver mode
    radio.startListening();

    // Wait a bit for state transition
    SHITTY_RADIO_DELAY();

    unsigned long started_waiting_at = micros();
    response = 1;

    // Wait for response - with timeout if no response
    while (!radio.available()){                 
      if (micros() - started_waiting_at > RESPONSE_TIMEOUT){
          Serial.println("------- No response");
          response = 0;
          break;
      }      
    }

    // Dont read data if no response
    if(response){
        radio.read(&dataRx, sizeof(dataRx));
        rxTimestamp = micros();

        if( (dataRx.joystick[0] != dataTx.joystick[0]) || (dataRx.joystick[1] != dataTx.joystick[1]) ){
            Serial.println("--------------DATA NOT THE SAME!");
            response = 0;
        }

    #if DEBUG
        //Serial.print("Round-trip delay:");
        //Serial.println(rxTimestamp - txTimestamp);
    #endif

        /* TODO test it on the end
        if(radio.available()){
            // TODO Not sure if this is OK --> if we call this even before radio reads the data
            // If radio.available() returns 1 more times, even after we read the data, there is something wrong
            Serial.println("--ERROR-------");
            radio.failureDetected = true;
        }*/
    }


#if TEST
    delay(1000);
#else
    delay(50);
#endif

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

    // Set the address...one for transmitting one for receiving
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);

    //Set module as transmitter
    radio.stopListening();
}

uint32_t MOWA_radio_check(){

    if(radio.getDataRate() != RF24_1MBPS){
      radio.failureDetected = true;
      Serial.print("Radio configuration error detected");
    }

    return millis();
}

void MOWA_joystick_get(){

    int joystickReadingX, joystickReadingY;

    // Get the values from joystick X
    joystickReadingX = analogRead(PIN_JOYSTICK_X);
    
    // Stay still
    if(joystickReadingX > 400 && joystickReadingX < 600){
        dataTx.joystick[0] = 0;
    } 
    // Turn right
    else if (joystickReadingX >= 600){
        // Determine speed
        if(joystickReadingX < 1000){
            dataTx.joystick[0] = 1;
        }
        else{
            dataTx.joystick[0] = 2;
        }
    }
    // Turn left
    else if(joystickReadingX <= 400){
        // Determine speed
        if(joystickReadingX > 50){
            dataTx.joystick[0] = -1;
        }
        else{
            dataTx.joystick[0] = -2;
        }
    }

    // Get the values from joystick X
    joystickReadingY = analogRead(PIN_JOYSTICK_Y);

    // Stay still
    if(joystickReadingY > 400 && joystickReadingY < 600){
        dataTx.joystick[1] = 0;
    } 
    // Go forward
    else if (joystickReadingY >= 600){
        // Determine speed
        if(joystickReadingY < 1000){
            dataTx.joystick[1] = 1;
        }
        else{
            dataTx.joystick[1] = 2;
        }
    }
    // Go backwards
    else if(joystickReadingY <= 400){
        // Determine speed
        if(joystickReadingY > 50){
            dataTx.joystick[1] = -1;
        }
        else{
            dataTx.joystick[1] = -2;
        }
    }
}