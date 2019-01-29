#ifndef RACom_H
#define RACom_H

#include "Arduino.h"
#include "SoftwareSerial.h"
#include <limits.h>

enum
{
    BAUND_RATE = 9600,
    RX = 10,
    TX = 11
};

class RACom {
public:
    void init(int id);
    void testCom();

//private:
    
};

#endif