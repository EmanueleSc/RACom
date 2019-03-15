// Branch RAComFreeRTOS
#include "RACom.h"
SoftwareSerial MySerial (RX, TX);

static int initFlag = 0;
int MY_ID;
int NUM_ANTS; // Number of ants in the antNet
static int currSucc;

//unsigned long ticksAtStart;
//unsigned long cmdTimeout;

int _bufsize;
static char _buffer[50];

/* FreeRtos Staff */
TimerHandle_t xGlobalTimer;
TimerHandle_t xResponseTimer;
static bool globalTimer_expired;
static bool responseTimer_expired;


void RACom::init(int id, int number_of_ants) {
    MySerial.begin(BAUND_RATE);
    while(!MySerial) {
      ;
    }
    Serial.print("Wireless module serial started at ");
    Serial.println(BAUND_RATE);

    pinMode(SET_PIN, OUTPUT); // Connected to set input

    MY_ID = id;
    NUM_ANTS = number_of_ants;
    currSucc = MY_ID;
    
    _bufsize = sizeof(_buffer)/sizeof(char);
    // flush the buffer
    memset(_buffer, 0, _bufsize);

    // Start softweare timers
    globalTimer_expired = false;
    responseTimer_expired = false;
    setupTimers();
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

void RACom::broadcastPhase() {
  bool isMyTurn;
  do 
  {
    isMyTurn = false;
    findMyNext();
    broadcast();
    startResponseTimer();
    memset(_buffer, 0, _bufsize);

    // iterate until response timeout is not expired
    while( !responseTimer_expired ) {
      if(MySerial.available()) {
        
        if((char)MySerial.read() == '@') {
          MySerial.readBytesUntil('$', _buffer, _bufsize);
          break;
        }

      }
    }

    if(strlen(_buffer) != 0 && getSucc() == MY_ID) {
      currSucc = MY_ID;
      isMyTurn = true;
    } 

    Serial.print("<--- Message received after broadcast: ");
    Serial.println(_buffer);

  } 
  while(strlen(_buffer) == 0 || isMyTurn == true);
}

void RACom::readPhase() {
    if(MySerial.available()) {

      if((char)MySerial.read() == '@') {
        MySerial.readBytesUntil('$', _buffer, _bufsize);
        
        Serial.print("<--- Message received: ");
        Serial.println(_buffer);
        
        if(getSucc() == MY_ID) {
          currSucc = MY_ID;
          broadcastPhase();
        }
      }

      startGlobalTimer();
    }

}

void RACom::comAlgo() {
  if(initFlag == 0) {
    MySerial.flush();
    startGlobalTimer();
    initFlag = 1;
  }
  
  // Global timeout
  if(!globalTimer_expired) {
    readPhase();
  }
  else {
    broadcastPhase();
    startGlobalTimer();
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

void RACom::broadcast() {
  MySerial.print('@');
  MySerial.print(MY_ID);
  MySerial.print('#');
  MySerial.print(currSucc);
  MySerial.print('$');

  Serial.print("<--- Message Sent: ");
  Serial.print(MY_ID);
  Serial.print('#');
  Serial.println(currSucc);
}

int RACom::getMit() {
  int i = 0;
  char * pch = strtok(_buffer, "#");

  while (pch != NULL)
  {
    if(i == 0) break;
    pch = strtok (NULL, "#");
    i++;
  }  
  
  return pch - '0';
}

int RACom::getSucc() {
  int i = 0;
  char * pch = strtok(_buffer, "#");

  while (pch != NULL)
  {
    if(i == 1) break;
    pch = strtok (NULL, "#");
    i++;
  }  
  
  return pch - '0';
}

/*void RACom::startOperation(unsigned long timeout) {
    ticksAtStart = millis();
    cmdTimeout = timeout;
}*/

/*bool RACom::isOperationTimedOut() const {
    return operationDuration() >= cmdTimeout;
}*/


/*unsigned long RACom::operationDuration() const {
    unsigned long current_ticks = millis();
    unsigned long elapsed_ticks;
    
    if (current_ticks >= ticksAtStart)
        elapsed_ticks = current_ticks - ticksAtStart;
    else
        elapsed_ticks = (ULONG_MAX - ticksAtStart) + current_ticks;

    return elapsed_ticks;
}*/

void RACom::setupTimers() {
  Serial.println("Setup timers");

  xGlobalTimer = xTimerCreate(
        "Global_Timer",               /* A text name, purely to help debugging. */
        ( RING_ROUND_TRIP_TIMEOUT ),  /* The timer period. */
		    pdFALSE,						            /* This is an auto-reload timer, so xAutoReload is set to pdTRUE. */
		    ( void * ) 0,				          /* The ID is not used, so can be set to anything. */
		    globalTimerCallback           /* The callback function that inspects the status of all the other tasks. */
  );

  xResponseTimer = xTimerCreate(
        "Response_Timer",             /* A text name, purely to help debugging. */
        ( RESPONSE_TIMEOUT ),         /* The timer period. */
		    pdFALSE,						          /* This is an auto-reload timer, so xAutoReload is set to pdTRUE. */
		    ( void * ) 0,				          /* The ID is not used, so can be set to anything. */
		    responseTimerCallback         /* The callback function that inspects the status of all the other tasks. */
  );    
}

void RACom::startGlobalTimer() {
  globalTimer_expired = false;
  xTimerStart( xGlobalTimer, 0 );
}

void RACom::startResponseTimer() {
  responseTimer_expired = false;
  xTimerStart( xResponseTimer, 0 );
}

static void RACom::globalTimerCallback( TimerHandle_t xTimer )
{
  Serial.println("Global Timer Expired");
	globalTimer_expired = true;
}

static void RACom::responseTimerCallback( TimerHandle_t xTimer )
{
  Serial.println("Response Timer Expired");
	responseTimer_expired = true;
}