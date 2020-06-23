//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(9, 8);  // CE, CSN

//address through which two modules communicate.
const byte address[6] = "00001";

void setup()
{
    delay(100);
    Serial.begin(115200);
    Serial.println("Transmitter!");

    radio.begin();

    //set the address
    radio.openWritingPipe(address);

    //Set module as transmitter
    radio.stopListening();
}
void loop()
{
    uint8_t ack;

    //Send message to receiver
    const char text[] = "Hello World";
    ack = radio.write(&text, sizeof(text));
    Serial.print("Transmitt :");
    Serial.println(ack);

    delay(1000);
}