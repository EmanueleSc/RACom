// Branch RAComFreeRTOS
#include "RACom.h"
static SoftwareSerial MySerial (RX, TX);
//static NeoSWSerial MySerial (RX, TX);

static byte initFlag = 0;
static byte MY_ID;
static byte NUM_ANTS; // Number of ants in the antNet
static byte currSucc;
static byte _bufsize;
static char _buffer[50];

/* FreeRtos Staff */
TimerHandle_t xGlobalTimer;
TimerHandle_t xResponseTimer;

static bool startupTimer_expired;
static bool globalTimer_expired;
static bool responseTimer_expired;

// Array of next positions 
static byte nextPositions[NUM_NEXT_POS] = { 225, 225, 225, 225, 225, 225, 225, 225  }; // my pos to brodcast

// One array for each ant
static byte recvPos1[NUM_NEXT_POS] = { 225, 225, 225, 225, 225, 225, 225, 225  }; // received pos for outside
static byte recvPos2[NUM_NEXT_POS] = { 225, 225, 225, 225, 225, 225, 225, 225  }; // received pos for outside
static byte recvPos3[NUM_NEXT_POS] = { 225, 225, 225, 225, 225, 225, 225, 225  }; // received pos for outside
static byte recvPos4[NUM_NEXT_POS] = { 225, 225, 225, 225, 225, 225, 225, 225  }; // received pos for outside
static byte recvPos5[NUM_NEXT_POS] = { 225, 225, 225, 225, 225, 225, 225, 225  }; // received pos for outside


void RACom::init(byte id, byte number_of_ants) {
    MySerial.begin(BAUND_RATE);
    while(!MySerial);
    
    Serial.print(F("Wireless module serial started at "));
    Serial.println(BAUND_RATE);

    pinMode(SET_PIN, OUTPUT); // Connected to set input

    MY_ID = id;
    NUM_ANTS = number_of_ants;
    currSucc = MY_ID;
    
    _bufsize = sizeof(_buffer)/sizeof(char);
    _buffer[0] = '\0'; // flush the buffer

    // Start softweare timers
    startupTimer_expired = false;
    globalTimer_expired = false;
    responseTimer_expired = false;
}

void RACom::comunicationMode() {
  digitalWrite(SET_PIN, HIGH);
  //analogWrite(SET_PIN, 255);
}

void RACom::commandMode() {
  digitalWrite(SET_PIN, LOW);
  //analogWrite(SET_PIN, 0);
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
    _buffer[0] = '\0';

    // iterate until response timeout is not expired
    while( !responseTimer_expired ) {
      if(MySerial.available()) {
        
        if((char)MySerial.read() == '@') {
          MySerial.readBytesUntil('$', _buffer, _bufsize);
          break;
        }

      }
    }

    if(getSucc() == MY_ID) {
      currSucc = MY_ID;
      isMyTurn = true;
    } 

    Serial.print(F("<--- Message received after broadcast: "));
    Serial.println(_buffer);

    // set recvPos array with outside data
    setRecvPosArray();

  } 
  while(strlen(_buffer) == 0 || isMyTurn == true);

  // At the end of brodcast phase, restart the global timer
  startGlobalTimer();
}

void RACom::comAlgo() {
  if(initFlag == 0) {
    MySerial.flush();
    startGlobalTimer();
    initFlag = 1;
  }
  
  // Global timeout
  if(!globalTimer_expired) {
    // Read phase
    if(MySerial.available()) {

      if((char)MySerial.read() == '@') {
        MySerial.readBytesUntil('$', _buffer, _bufsize);
        
        Serial.print(F("<--- Message received: "));
        Serial.println(_buffer);

        // set recvPos array with outside data
        setRecvPosArray();
        
        if(getSucc() == MY_ID) {
          currSucc = MY_ID;
          broadcastPhase();
        }
      }

    }
  }
  else {
    // I'm the only one in the network
    broadcastPhase();
  } 
  
}

void RACom::setNextPosArray(byte replace[]) {
  for(int i = 0; i < NUM_NEXT_POS; i++) {
    nextPositions[i] = replace[i];
  }
}

byte* RACom::getRecvPosArray(byte num_ant) {
  if(num_ant == 1) return recvPos1;
  if(num_ant == 2) return recvPos2;
  if(num_ant == 3) return recvPos3;
  if(num_ant == 4) return recvPos4;
  if(num_ant == 5) return recvPos5;
}

bool RACom::startupTimerExpired() {
  return startupTimer_expired;
}

void RACom::findMyNext() {
  currSucc++;

  if(NUM_ANTS == 2 || MY_ID == NUM_ANTS) {
    if(currSucc >= NUM_ANTS) currSucc = 1; 
  }
  else {
    if(currSucc > NUM_ANTS) currSucc = 1; 
  }
  
  if(currSucc == MY_ID) currSucc++;
}

void RACom::broadcast() {
  Serial.print(F("<--- Message Sent: "));
  Serial.print(MY_ID);
  Serial.print('#');
  Serial.print(currSucc);
  Serial.print('#');

  // Wireless send
  MySerial.print('@'); // start
  MySerial.print(MY_ID); // mit
  MySerial.print('#');
  MySerial.print(currSucc); // succ
  MySerial.print('#');
  
  for(int i = 0; i < NUM_NEXT_POS; i++) {
    MySerial.print(nextPositions[i]); // next pos
    Serial.print(nextPositions[i]);
  
    if(i != NUM_NEXT_POS - 1) {
      MySerial.print('#');
      Serial.print('#');
    }
  }

  MySerial.print('$'); // stop

  resetNextPosArray();
}

/*int RACom::getMit() {
  if( strlen(_buffer) != 0 ) { 
    char copy[50];
    size_t len = sizeof(copy);
    strncpy(copy, _buffer, len);
    copy[len-1] = '\0';

    char * pch = strtok(copy, "#");

    int i = 0;
    while (pch != NULL)
    {
      if(i == 0) break;
      pch = strtok (NULL, "#");
      i++;
    }  
    
    return atoi(pch);
  }
  
  return NUM_ANTS + 1; // not existing ANT
}*/

int RACom::getSucc() {
  if( strlen(_buffer) != 0 ) {
    char copy[50];
    size_t len = sizeof(copy);
    strncpy(copy, _buffer, len);
    copy[len-1] = '\0';

    char * pch = strtok(copy, "#");
    int i = 0;
    while (pch != NULL)
    {
      if(i == 1) break;
      pch = strtok (NULL, "#");
      i++;
    }

    return atoi(pch);
  }

  return NUM_ANTS + 1; // not existing ANT
}

void RACom::setRecvPosArray() {
  if( strlen(_buffer) != 0  ) {
    int mit;

    char copy[50];
    size_t len = sizeof(copy);
    strncpy(copy, _buffer, len);
    copy[len-1] = '\0';

    char * pch = strtok(copy, "#");
    int i = 0;
    
    while (pch != NULL) {
      if(i == 0) {
        mit = atoi(pch);
      }

      if(i >= 2 && i <= 9) {
        if(mit == 1) recvPos1[i - 2] = (byte) atoi(pch);
        if(mit == 2) recvPos2[i - 2] = (byte) atoi(pch);
        if(mit == 3) recvPos3[i - 2] = (byte) atoi(pch);
        if(mit == 4) recvPos4[i - 2] = (byte) atoi(pch);
        if(mit == 5) recvPos5[i - 2] = (byte) atoi(pch);

        if(i == 9) break;
      }
      pch = strtok (NULL, "#");
      i++;
    }

  }
} 

void RACom::resetNextPosArray() {
  for(int i = 0; i < NUM_NEXT_POS; i++) {
    nextPositions[i] = 225;
  }
}

void RACom::setupTimers() {
  Serial.println(F("Setup timers"));

  xGlobalTimer = xTimerCreate(
        "Global_Timer",               /* A text name, purely to help debugging. */
        ( RING_ROUND_TRIP_TIMEOUT ),  /* The timer period. */
		    pdFALSE,						          /* This is an auto-reload timer, so xAutoReload is set to pdTRUE. */
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

  if(xGlobalTimer == NULL || xResponseTimer == NULL) {
    Serial.println(F("failure creating Timers"));
    for(;;);
  }
}

void RACom::startGlobalTimer() {
  Serial.print('\n');
  Serial.println(F("G. timer started"));
  globalTimer_expired = false;
  xTimerStart( xGlobalTimer, 0 );
}

void RACom::startResponseTimer() {
  Serial.print('\n');
  Serial.println(F("R. timer started"));
  responseTimer_expired = false;
  xTimerStart( xResponseTimer, 0 );
}

void RACom::globalTimerCallback( TimerHandle_t xTimer )
{
  Serial.print('\n');
  Serial.println(F("Global Timer Expired"));
	startupTimer_expired = true;
  globalTimer_expired = true;
}

void RACom::responseTimerCallback( TimerHandle_t xTimer )
{
  Serial.print('\n');
  Serial.println(F("Response Timer Expired"));
	responseTimer_expired = true;
}