//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define SHITTY_RADIO_DELAY()   delay(5);

#define RADIO_CHECK_TIME    5000    // in ms
#define DEBUG               1
#define TEST                0


uint32_t radioChekcTimer;
uint32_t radioNotAvailable = 1;
uint32_t noRadioTimer;


//create an RF24 object
RF24 radio(9, 8);  // CE, CSN

struct dataStruct{
    float joystick[2];      // joystick[0] = X, joystick[1] = Y
} dataRx, dataTx;




void     MOWA_radio_reset();
uint32_t MOWA_radio_check();



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
        radio.failureDetected = false;
        delay(250);
        Serial.println("Radio failure detected, restarting radio");
        MOWA_radio_reset();
        delay(100);    
        radioNotAvailable = 0;    
    }

    // If we received any packet, read it
    if (radio.available())
    {
        radioNotAvailable = 0;

        // Read the received data
        radio.read(&dataRx, sizeof(dataRx));

    #if DEBUG
        Serial.print("X =");
        Serial.print(dataTx.joystick[0]);
        Serial.print("Y =");
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
    else
    {
        radioNotAvailable ++;
    }


    // If we received some data
    if(radioNotAvailable == 0){
        // Every time we receive something, reset the timer
        noRadioTimer = millis();
    }

    // If radio is not avaliable for some time, something is wrong with transmitter
#if TEST
    if(millis() - noRadioTimer  > 500){
        Serial.print("----- CRITICAL WARNING");
        Serial.println(radioNotAvailable);
        radio.failureDetected = true;
    }
#else
    if(millis() - noRadioTimer  > 200){
        Serial.print("----- CRITICAL WARNING");
        Serial.println(radioNotAvailable);
        radio.failureDetected = true;       //TODO dont reset radio everytime...just every 5th time?
        // Or if you reset it everytime, put a bigger delay here, because it resets 2 or 3 times before it gets somethuing from transmitter
    }
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