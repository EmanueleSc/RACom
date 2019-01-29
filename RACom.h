#ifndef RACom_H
#define RACom_H

#include "Arduino.h"
#include "SoftwareSerial.h"
#include <limits.h>

enum
{
    BAUND_RATE = 9600,
    RX = 11,
    TX = 10,
    SET_PIN = 6
};

class RACom {
public:
    void init(int id);
    void comunicationMode();
    void commandMode();
    void testCom();

//private:
    
};

#endif