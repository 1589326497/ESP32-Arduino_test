#include <Arduino.h>

#define LED_PIN 2
void setup() {

  pinMode(LED_PIN,OUTPUT);

}

void loop() {
  for(int i=0;i<255;i++)
  {
    analogWrite(LED_PIN,i);
    delay(10);
  }
  for(int i=255;i>=0;i--)
  {
    analogWrite(LED_PIN,i);
    delay(10);
  }
  

}
