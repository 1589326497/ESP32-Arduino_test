#include <Arduino.h>
#define d2 2
int num=0;

//中断服务函数
void service_Exit02()
{
  num++;
  Serial.printf("调用次数：%d",num);
  Serial.println();
  digitalWrite(d2,!digitalRead(d2));
}

void send_plus()
{
  digitalWrite(4,!digitalRead(4));
  delay(1000);
}
void setup() {
  Serial.begin(115200);
  pinMode(d2,OUTPUT);
  pinMode(4,OUTPUT);
  attachInterrupt(4,service_Exit02,FALLING);
}

void loop() {
  send_plus();
}

