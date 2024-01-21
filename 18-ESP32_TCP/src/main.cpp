#include <Arduino.h>
#include <WiFi.h>
// ESP32作为tcp服务端，手机或者电脑的调试助手
// 作为客户端，进行通信

const char *ssid = "esp32_tcp";
const char *password = "987654321";
// tcp服务端对象
WiFiServer server(1133);

void setup()
{
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  server.begin();
  // 串口打印AP_IP
  Serial.print("IP:");
  Serial.println(WiFi.softAPIP());
}

void loop()
{
  // 循环检测有无客户端  有请求-能连接-有数据
  WiFiClient client = server.available(); // client指向发出请求的客户端对象
  if (client)
  {
    Serial.println("新客户端.");
    Serial.println(client.localIP());
    while (client.connected())
    {
      // 连接成功进入循环
      if (client.available())//检测有无数据可读
      {
        char val = client.read(); // 读取一个字节数据
        Serial.println(val);
        client.print(val); // 回发数据给客户端
      }
    }
  }
  else
  {
    client.stop();
    delay(2000);
  }
}
