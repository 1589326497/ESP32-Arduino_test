#include <Arduino.h>
#define ain 35

uint16_t adc_value = 0;
float adc_voltage = 0.0;
void read_adc()
{
  adc_value = analogRead(ain);
  adc_voltage = adc_value * (3.3 / 4095);

  Serial.printf("采样值：%d 电压值：%.2f", adc_value, adc_voltage);
  Serial.println();
  delay(500);
}
void setup()
{
  Serial.begin(115200);
}
void loop()
{
  read_adc();
}
