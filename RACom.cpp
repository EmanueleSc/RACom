#include "RACom.h"
// Branch RAComFreeRTOS
SoftwareSerial MySerial (RX, TX);

int initFlag = 0;
int MY_ID;
int NUM_ANTS; // Number of ants in the antNet
int currSucc;
unsigned long ticksAtStart;
unsigned long cmdTimeout;
String message = "";
StaticJsonDocument<200> doc;

/* FreeRtos Staff */
TickType_t xGlobal_Wait;
TimeOut_t xGlobal_TimeOut;

TickType_t xResponse_Wait;
TimeOut_t xResponse_TimeOut;


void RACom::init(int id, int number_of_ants) {
    MySerial.begin(BAUND_RATE);
    while(!MySerial) {
      ;
    }
    Serial.print("Wireless module serial started at ");
    Serial.println(BAUND_RATE);

    pinMode(SET_PIN, OUTPUT); // Connected to set input

    xGlobal_Wait = RING_ROUND_TRIP_TIMEOUT;
    xResponse_Wait = RESPONSE_TIMEOUT;
    MY_ID = id;
    NUM_ANTS = number_of_ants;
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
    Serial.println("INIT STATE");
    MySerial.flush();
    //startOperation(RING_ROUND_TRIP_TIMEOUT); // for global timeout
    vTaskSetTimeOutState( &xGlobal_TimeOut );
    initFlag = 1;
  }
}

void RACom::broadcastPhase() {
  bool isMyTurn;
  Serial.println("BROADCAST STATE");
  do 
  {
    isMyTurn = false;
    findMyNext();
    broadcast(MY_ID, currSucc);

    //startOperation(RESPONSE_TIMEOUT); // for response timeout
    vTaskSetTimeOutState( &xResponse_TimeOut );
    message = "";

    // iterate until response timeout is not expired
    //while(!isOperationTimedOut()) {
    
    while( xTaskCheckForTimeOut( &xResponse_TimeOut, &xResponse_Wait ) == pdFALSE ) {
      if(MySerial.available()) {
        
        if((char)MySerial.read() == '@') {
          message = MySerial.readStringUntil('$');
          MySerial.flush();
          break;
        }

      }
    }

    if(message != "" && getSucc(message) == MY_ID) {
      currSucc = MY_ID;
      isMyTurn = true;
    } 

    Serial.print("<--- Message received after broadcast: ");
    Serial.println(message);

  } 
  while(message == "" || isMyTurn == true);
}

void RACom::readPhase() {
    if(MySerial.available()) {
      Serial.println("QUALCOSA RICEVUTO..");

      if((char)MySerial.read() == '@') {
        message = MySerial.readStringUntil('$');
        MySerial.flush();
        Serial.print("<--- Message received: ");
        Serial.println(message);
        if(getSucc(message) == MY_ID) {
          currSucc = MY_ID;
          broadcastPhase();
        }
      }

      //startOperation(RING_ROUND_TRIP_TIMEOUT); // Restart global timeout
      vTaskSetTimeOutState( &xGlobal_TimeOut );
    }

}

void RACom::comAlgo() {
  initPhase();
  
  // Global timeout
  //if(!isOperationTimedOut()) {
  if( xTaskCheckForTimeOut( &xGlobal_TimeOut, &xGlobal_Wait ) == pdFALSE ) {
    Serial.println("READY STATE");
    readPhase();
  }
  else {
    broadcastPhase();
    //startOperation(RING_ROUND_TRIP_TIMEOUT); // Restart global timeout
    vTaskSetTimeOutState( &xGlobal_TimeOut );
  } 
  
}

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