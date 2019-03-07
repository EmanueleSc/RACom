#include "RACom.h"

SoftwareSerial MySerial (RX, TX);

int initFlag = 0;
int MY_ID;
int currSucc;
unsigned long ticksAtStart;
unsigned long cmdTimeout;
String message = "";
StaticJsonDocument<200> doc;


void RACom::init(int id) {
    MySerial.begin(BAUND_RATE);
    while(!MySerial) {
      ;
    }
    Serial.print("Wireless module serial started at ");
    Serial.println(BAUND_RATE);

    pinMode(SET_PIN, OUTPUT); // Connected to set input

    MY_ID = id;
    currSucc = MY_ID;
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

void RACom::initPhase() {
  if(initFlag == 0) {
    MySerial.flush();
    startOperation(RING_ROUND_TRIP_TIMEOUT); // for global timeout
    initFlag = 1;
  }
}

void RACom::broadcastPhase() {
  bool isMyTurn;

  do 
  {
    isMyTurn = false;
    findMyNext();
    broadcast(MY_ID, currSucc);

    startOperation(RESPONSE_TIMEOUT); // for response timeout
    message = "";

    // iterate until response timeout is not expired
    while(!isOperationTimedOut()) {
      if(MySerial.available()) {
        
        if((char)MySerial.read() == '@') {
          message = MySerial.readStringUntil('$');
          MySerial.flush();
          break;
        }

      }
    }

    if(message != "" && getSucc(message) == MY_ID) {
      isMyTurn = true;
    } 

    Serial.print("<--- Message received after broadcast: ");
    Serial.println(message);

  } 
  while(message == "" || isMyTurn == true);
}

void RACom::readPhase() {
    if(MySerial.available()) {

      if((char)MySerial.read() == '@') {
        message = MySerial.readStringUntil('$');
        MySerial.flush();
        Serial.print("<--- Message received: ");
        Serial.println(message);
        if(getSucc(message) == MY_ID) broadcastPhase();
      }

      startOperation(RING_ROUND_TRIP_TIMEOUT); // Restart global timeout
    }

}

void RACom::comAlgo() {
  initPhase();
  
  // Global timeout
  if(!isOperationTimedOut()) { 
    readPhase();
  }
  else {
    broadcastPhase();
    startOperation(RING_ROUND_TRIP_TIMEOUT); // Restart global timeout
  } 
  
}


/*void RACom::comunicationAlgorithm() {
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
    Serial.print("<--- Message received after send: ");
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
}*/

void RACom::findMyNext() {
  currSucc++;

  if(NUM_ANTS > 2) {
    if(currSucc > NUM_ANTS) currSucc = 1; 
  }
  else {
    if(currSucc >= NUM_ANTS) currSucc = 1; 
  }
  
  if(currSucc == MY_ID) currSucc++;
}

void RACom::broadcast(int mit, int succ) {
  String json;
  doc["mit"] = mit;
  doc["succ"] = succ;
  serializeJson(doc, json);

  MySerial.print('@');
  MySerial.print(json);
  MySerial.print('$');

  Serial.print("<--- Message Sent: ");
  Serial.println(json);
}

int RACom::getMit(String json) {
  deserializeJson(doc, json);
  int mit = doc["mit"];
  return mit;
}

int RACom::getSucc(String json) {
  deserializeJson(doc, json);
  int succ = doc["succ"];
  return succ;
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