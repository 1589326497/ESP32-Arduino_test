#include <Arduino.h>

#define d2 2
void service_timer0()
{
  digitalWrite(d2,!digitalRead(d2));
}
hw_timer_t *tim0=NULL;
void Init_timer()
{
  tim0=timerBegin(0,80,true);
  timerAttachInterrupt(tim0,service_timer0,true);
  timerAlarmWrite(tim0,2000000,true);
  timerAlarmEnable(tim0);
}
void setup() {
  pinMode(d2,OUTPUT);
  Init_timer();
}

void loop() {

}

