# RACom Branch FREERTOS

```
#include <Arduino_FreeRTOS.h>
#include <RACom.h>


RACom wireless;
void task1()
{
  for (;;) {
    wireless.comAlgo();
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(9600);

  // start th serial communication with the host computer

  Serial.println("Arduino with HC-12 is ready");
  while (!Serial);
  Serial.println("OK");
  // Before loading sketch, initialize the library with init method.
  wireless.init(1, 2); // First param: id of ANT, Second param: number of ANTS
  wireless.comunicationMode();
  xTaskCreate(task1, "task1", 128, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;);
}

void loop()
{

}
```
