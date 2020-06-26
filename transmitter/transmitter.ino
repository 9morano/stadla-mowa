/*
 *
 * Because of shitty radio I added delays between state transitions.
 *
 *
*/




//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define RESPONSE_TIMEOUT    200000

//create an RF24 object
RF24 radio(9, 8);  // CE, CSN

struct dataStruct{
    char cmd[4];
    float val;
} data;

unsigned long rxTimestamp, txTimestamp;


void MOWA_radio_reset();


void setup()
{  
    // Power-up delay
    delay(100);
    Serial.begin(115200);
    Serial.println("Transmitter!");

    MOWA_radio_reset();

    data.cmd[0]='c';
    data.cmd[1] ='m';
    data.cmd[2] ='d';
    data.cmd[3] =' ';
    data.val = 1;
}

void loop()
{
    uint8_t ack, response;

    // Set module as transmitter
    radio.stopListening();

    // Send the data
    txTimestamp = micros();
    ack = radio.write(&data, sizeof(data));
    Serial.print("Transmitt :");
    Serial.println(ack);

    // A bit of delay to receive ACK 
    delay(10);

    // Set to receiver mode
    radio.startListening();

    // Wait a bit for state transition
    delay(10);

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
        radio.read(&data, sizeof(data));
        rxTimestamp = micros();

        Serial.print(data.cmd);
        Serial.println(data.val);
        Serial.print("Round-trip delay:");
        Serial.println(rxTimestamp - txTimestamp);
    }


    delay(1000);
    data.val ++;

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