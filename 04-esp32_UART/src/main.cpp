#include <Arduino.h>
#define d2 2

void setup() {
    //初始化灯光
    pinMode(d2,OUTPUT);
    //初始化UART
    Serial.begin(115200);
}

void loop() {
  //灯光的控制
  if(Serial.available()>0)
  {
    uint8_t cmd=0;
    cmd=Serial.read();
    switch(cmd)
    {
        case 0xA1:
            digitalWrite(d2,HIGH);
            Serial.println("d2 is ON!");
            break;
        case 0xA2:
            digitalWrite(d2,LOW);
            Serial.println("d2 is OFF!");
            break;
        default:
            Serial.println("输入命令错误");
    }
  }
}

