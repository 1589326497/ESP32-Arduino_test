#include <Arduino.h>
#include <WiFi.h>

WiFiUDP udp;  //声明UDP对象，服务端开启，监听

//UDP无线控制LED亮灭
#define LEDPIN 2
void led(const char &sw){
  pinMode(LEDPIN,OUTPUT);
  if(sw){
    digitalWrite(LEDPIN,HIGH);
  }else{
    digitalWrite(LEDPIN,LOW);
  }
}

void setup() {
  WiFi.softAP("ESP32_UDP","987654321");
  udp.begin(1133);
  Serial.begin(115200);
  //串口打印AP-IP
  Serial.println(WiFi.softAPIP());
}

void loop() {
  //接收发送过来的UDP数据
  if(udp.parsePacket()){
    // //读取一个字节
    // char val=udp.read(); 
    // Serial.println(val);

    //读取多个字节
    char* val = new char[256];  // 使用 new 动态分配一个数组，假设你想读取最多 255 个字节，为 null 终止符留下空间
    int bytesRead = udp.read(val, 255);  // 读取最多 255 个字节

    val[bytesRead] = '\0';  // 加上 null 终止符

    // 处理并打印接收到的数据
    Serial.println(val);

    //点亮LED
    //led(val);

    //将接收到的数据处理在回发
    udp.beginPacket(udp.remoteIP(),udp.remotePort());
    udp.println(val);
    udp.endPacket();
    //释放内存
     delete[] val;
  }
}
