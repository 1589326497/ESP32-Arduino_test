#include <Arduino.h>

#define d2 2
#define ain 35

uint16_t adc_value = 0;

void Init_PWM()
{
  ledcSetup(1, 128, 12);
  ledcAttachPin(d2, 1);
}
void adc_set_pwm()
{
  adc_value = analogRead(ain);
  ledcWrite(1, adc_value);
  Serial.println(adc_value);
  delay(100);
}

void setup()
{
  Init_PWM();
  Serial.begin(115200);
}

void loop()
{
  adc_set_pwm();
}
