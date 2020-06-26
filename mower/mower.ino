//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(9, 8);  // CE, CSN


// Address through which two modules communicate.
byte addresses[][6] = {"1Node","2Node"};

struct dataStruct{
    char cmd[4];
    float val;
} data;

void setup()
{
    delay(100);
    Serial.begin(115200);
    Serial.println("Mower!");

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

void loop() {

    uint8_t ack;

    if (radio.available())
    {
        // Read the received data
        radio.read(&data, sizeof(data));
        Serial.print(data.cmd);
        Serial.println(data.val);

        // Wait a bit to transmit ACK till the end
        delay(10);

        // Set to transmitter mode
        radio.stopListening();

        // Wait a bit for state transition
        delay(10);

        // Send response back to RC
        ack = radio.write(&data, sizeof(data));
        Serial.print("Respond :");
        Serial.println(ack);

        // A bit of delay to receive ACK
        delay(10);

        // Back to receiving mode
        radio.startListening();
    }
}