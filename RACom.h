#ifndef RACom_H
#define RACom_H

#include "Arduino.h"
#include "SoftwareSerial.h"
#include <limits.h>
#include "ArduinoJson.h"

/**
*
Algoritmo pseudocodice:

    algorithm communication is 

        init <- 0
        message <- empty
        mit <- my_id

        while true do 

INIT:            if init = 0                        
                        start timeout_G <- 10sec
                        init <- 1
                

                if (timeout_G expired) and (message is empty)
STEP1:                  succ <- findMyNext
                        broadcast { mit, succ }
                        start timeout_R <- 200ms 
                        message <- empty

                        while not timeout_R is expired do 
                                When receive @ read message until $ then break
                        
                        if message is empty
                            go_to STEP1
                        
                        if message.mit = succ
                            init <- 0
                            go_to INIT
                
                if receive message
                        When receive @ read message until $ then break
                        
                        if message.succ = my_id
                            go_to STEP1
                        
                        init <- 0
                        go_to INIT
*/              


enum
{
    NUM_ANTS = 2, // Number of ants in the antNet
    BAUND_RATE = 9600,
    RX = 11,
    TX = 10,
    SET_PIN = 6,
    RING_ROUND_TRIP_TIMEOUT = 30000, // 30 sec for test
    RESPONSE_TIMEOUT = 500 // 500 millisec for test
};

class RACom {
public:
    void init(int id);
    void comunicationMode();
    void commandMode();
    void testCom();
    void comunicationAlgorithm();

private:
    void findMyNext();
    void broadcast(int mit, int succ);
    int getMit(String json);
    int getSucc(String json);

    void startOperation(unsigned long timeout);
    bool isOperationTimedOut() const;
    unsigned long operationDuration() const;
};

#endif