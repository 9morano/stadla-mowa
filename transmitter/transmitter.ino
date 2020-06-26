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

#define RESPONSE_TIMEOUT    200000  // in us
#define RADIO_CHECK_TIME    5000    // in ms
#define PIN_JOYSTICK_X      A0
#define PIN_JOYSTICK_Y      A1
#define DEBUG               1


uint32_t rxTimestamp, txTimestamp;
uint32_t radioChekcTimer;



//create an RF24 object
RF24 radio(9, 8);  // CE, CSN

struct dataStruct{
    float valX = 512;          // TODO why only float works, uint32_t displays random shit after number 9..
    float valY = 512;
} dataRx, dataTx;


void     MOWA_radio_reset();
uint32_t MOWA_radio_check();




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

    // Get the values from joystick
    dataTx.valX = analogRead(PIN_JOYSTICK_X);
    dataTx.valY = analogRead(PIN_JOYSTICK_Y);

#if DEBUG
    Serial.print("X =");
    Serial.print(dataTx.valX);
    Serial.print("Y =");
    Serial.println(dataTx.valY);
#endif

    // Send the data
    txTimestamp = micros();
    ack = radio.write(&dataTx, sizeof(dataTx));
    Serial.print("Transmitt :");
    Serial.println(ack);

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
          Serial.println("No response");
          response = 0;
          break;
      }      
    }

    // Dont read data if no response
    if(response){
        radio.read(&dataRx, sizeof(dataRx));
        rxTimestamp = micros();

        if( (dataRx.valX != dataTx.valX) || (dataRx.valY != dataTx.valY) ){
            Serial.println("DATA NOT THE SAME!");
            response = 0;
        }

    #if DEBUG
        Serial.print("Round-trip delay:");
        Serial.println(rxTimestamp - txTimestamp);
    #endif

        /* TODO test it on the end
        if(radio.available()){
            // TODO Not sure if this is OK --> if we call this even before radio reads the data
            // If radio.available() returns 1 more times, even after we read the data, there is something wrong
            Serial.println("--ERROR-------");
            radio.failureDetected = true;
        }*/
    }

/*
#if DEBUG
    delay(1000);
#else
    delay(50);
#endif
*/
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