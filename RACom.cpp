#include "RACom.h"

SoftwareSerial MySerial (RX, TX);

int MY_ID;
int _bufsize;
char _buffer[32];


void RACom::init(int id) {
    MySerial.begin(BAUND_RATE);
    while(!MySerial) {
      ;
    }
    Serial.print("Wireless module serial started at ");
    Serial.println(BAUND_RATE);

    //pinMode(A0, OUTPUT); // Connected to Bluetooth vcc
    //pinMode(A1, OUTPUT); // Connected to pin 34 (command mode enable pin)

    _bufsize = sizeof(_buffer)/sizeof(char);
    MY_ID = id;

    // flush the buffer
    memset(_buffer, 0, _bufsize);
}

void RACom::testCom() {
  if(MySerial.available()) {            // If HC-12 has data
    Serial.write(MySerial.read());      // Send the data to Serial monitor
  }
  if(Serial.available()) {              // If Serial monitor has data
    MySerial.write(Serial.read());      // Send that data to HC-12
  }
}