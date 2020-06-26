//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define SHITTY_RADIO_DELAY()   delay(5);

#define RADIO_CHECK_TIME    5000    // in ms


uint32_t radioChekcTimer;
uint32_t radioNotAvailable = 1;
uint32_t noRadioTiming;


//create an RF24 object
RF24 radio(9, 8);  // CE, CSN

struct dataStruct{
    float valX;
    float valY;
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
    // If something is wrong, reset it
    if(millis() - radioChekcTimer > RADIO_CHECK_TIME){
       radioChekcTimer = MOWA_radio_check();
    }

    // If faliure detected, reset it
    if(radio.failureDetected){
        radio.failureDetected = false;
        delay(250);
        Serial.println("Radio failure detected, restarting radio");
        MOWA_radio_reset();        
    }

    // If we have any packet in the buffer
    if (radio.available())
    {
        radioNotAvailable = 0;

        // Read the received data
        radio.read(&dataRx, sizeof(dataRx));
        //Serial.print(data.cmd);
        Serial.print("X =");
        Serial.print(dataRx.valX);
        Serial.print("Y =");
        Serial.println(dataRx.valY);

        // Wait a bit to transmit ACK till the end
        SHITTY_RADIO_DELAY();

        // Set to transmitter mode
        radio.stopListening();

        // Wait a bit for state transition
        SHITTY_RADIO_DELAY();

        // Send response back to RC - same values as conformation
        dataTx.valX = dataRx.valX;
        dataTx.valY = dataRx.valY;

        ack = radio.write(&dataTx, sizeof(dataTx));
        Serial.print("Respond :");
        Serial.println(ack);

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
    else{
        radioNotAvailable ++;
    }

    if(radioNotAvailable == 0){
        noRadioTiming = millis();
    }

    if(millis() - noRadioTiming  > 500){
        Serial.println(radioNotAvailable);
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