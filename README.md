# RACom MASTER BRANCH

```
#include <RACom.h>
RACom wireless;
 
void setup() 
{
    // start th serial communication with the host computer
    Serial.begin(9600);
    Serial.println("Arduino with HC-12 is ready");
    while (!Serial) {
      ;
    }
    // Before loading sketch, initialize the library with init method.
    wireless.init(1, 2); // First param: id of ANT, Second param: number of ANTS
    wireless.comunicationMode();
}
 
void loop()
{
  wireless.comAlgo();
}
```