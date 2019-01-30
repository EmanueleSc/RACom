#include "RACom.h"

SoftwareSerial MySerial (RX, TX);

int initFlag;
int MY_ID;
int currSucc;
unsigned long ticksAtStart;
unsigned long cmdTimeout;
String message = "";

int bufsize;
char jsonStr[250];
StaticJsonBuffer<200> jsonBuffer;


void RACom::init(int id) {
    MySerial.begin(BAUND_RATE);
    while(!MySerial) {
      ;
    }
    Serial.print("Wireless module serial started at ");
    Serial.println(BAUND_RATE);

    pinMode(SET_PIN, OUTPUT); // Connected to set input
    
    initFlag = 0;
    MY_ID = id;
    currSucc = MY_ID;

    bufsize = sizeof(jsonStr)/sizeof(char);
    memset(jsonStr, 0, bufsize);
}

void RACom::comunicationMode() {
  digitalWrite(SET_PIN, HIGH);
}

void RACom::commandMode() {
  digitalWrite(SET_PIN, LOW);
}

void RACom::testCom() {
  if(MySerial.available()) {            // If HC-12 has data
    Serial.write(MySerial.read());      // Send the data to Serial monitor
  }
  if(Serial.available()) {              // If Serial monitor has data
    MySerial.write(Serial.read());      // Send that data to HC-12
  }
}

void RACom::comunicationAlgorithm() {
INIT:if(initFlag == 0) {
    MySerial.flush();
    startOperation(RING_ROUND_TRIP_TIMEOUT);
    initFlag = 1;
    message = "";
  }

  if(isOperationTimedOut() && message == "") {
STEP1:findMyNext();
    broadcast(MY_ID, currSucc);
    startOperation(RESPONSE_TIMEOUT);
    message = "";

    while(!isOperationTimedOut()) {
      if(MySerial.available()) {
        if((char)MySerial.read() == '@') {
          message = MySerial.readStringUntil('$');
          break;
        }
      }
    }
    Serial.print("<--- Message received: ");
    Serial.println(message);
    
    if(message == "") goto STEP1;
    if(getMit(message) == currSucc) {
      initFlag = 0;
      goto INIT;
    }

  }

  if(MySerial.available()) {
    if((char)MySerial.read() == '@') {
      message = MySerial.readStringUntil('$');
      
      Serial.print("<--- Message received: ");
      Serial.println(message);

      if(getSucc(message) == MY_ID) goto STEP1;
      initFlag = 0; // else
    }
    else message = "";
  }
}

void RACom::findMyNext() {
  currSucc++;

  if(currSucc > NUM_ANTS) currSucc = 1;
  if(currSucc == MY_ID) currSucc++;

}

void RACom::broadcast(int mit, int succ) {
  memset(jsonStr, 0, bufsize);

  JsonObject& root = jsonBuffer.createObject();
  root["mit"] = mit;
  root["succ"] = succ;
  root.printTo(Serial);
  root.printTo(jsonStr);

  MySerial.print('@');
  MySerial.print(jsonStr);
  MySerial.print('$');
}

int RACom::getMit(String json) {
  JsonObject& root = jsonBuffer.parseObject(json);
  return root["mit"].as<int>();
}

int RACom::getSucc(String json) {
  JsonObject& root = jsonBuffer.parseObject(json);
  return root["succ"].as<int>();
}

void RACom::startOperation(unsigned long timeout) {
    ticksAtStart = millis();
    cmdTimeout = timeout;
}

bool RACom::isOperationTimedOut() const {
    return operationDuration() >= cmdTimeout;
}


unsigned long RACom::operationDuration() const {
    unsigned long current_ticks = millis();
    unsigned long elapsed_ticks;
    
    if (current_ticks >= ticksAtStart)
        elapsed_ticks = current_ticks - ticksAtStart;
    else
        elapsed_ticks = (ULONG_MAX - ticksAtStart) + current_ticks;

    return elapsed_ticks;
}