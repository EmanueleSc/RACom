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

    // Before loading sketch, insert the number of the ANT.
    wireless.init(1,3);
    wireless.comunicationMode();
}
 
void loop()
{
  wireless.comAlgo();
}
