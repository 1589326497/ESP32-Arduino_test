#include <Arduino.h>
#define LED_Pin 2
#define Touch_Pin 15

uint16_t touchValue = 0;
void setup()
{
  pinMode(LED_Pin, OUTPUT);
  Serial.begin(115200);
}

void loop()
{
  touchValue = touchRead(Touch_Pin);
  Serial.println(touchValue);

  if (touchValue < 35)
  {
    digitalWrite(LED_Pin, HIGH);
  }
  else
  {
    digitalWrite(LED_Pin, LOW);
  }
  
}

