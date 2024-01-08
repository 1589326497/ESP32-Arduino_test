#include <Arduino.h>

#define d2 2
#define d3 3
#define d4 4

void service_timer1()
{
  digitalWrite(d2, !digitalRead(d2));
}
void service_timer2()
{
  digitalWrite(d3, !digitalRead(d3));
}
void service_timer3()
{
  digitalWrite(d4, !digitalRead(d4));
}
hw_timer_t *times[3] = {nullptr};
void Init_timers()
{
  times[0] = timerBegin(1, 80, true);
  timerAttachInterrupt(times[0], service_timer1, true);
  timerAlarmWrite(times[0], 1000000, true);
  timerAlarmEnable(times[0]);

  times[1] = timerBegin(1, 80, true);
  timerAttachInterrupt(times[1], service_timer2, true);
  timerAlarmWrite(times[1], 2000000, true);
  timerAlarmEnable(times[1]);

  times[2] = timerBegin(1, 80, true);
  timerAttachInterrupt(times[2], service_timer3, true);
  timerAlarmWrite(times[2], 3000000, true);
  timerAlarmEnable(times[2]);
}

void setup()
{

  pinMode(d2, OUTPUT);
  pinMode(d3, OUTPUT);
  pinMode(d4, OUTPUT);
  Init_timers();
}

void loop()
{
}
