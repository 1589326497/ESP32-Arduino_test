#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "HUAWEI-NZX_2.4G";
const char *passwork = "987654321";

WebServer server(80);

void handelRoot()
{
  String HTML ="<!DOCTYPE html>\
  <html lang=\"zh\">\
  <head>\
      <meta charset=\"UTF-8\">\
      <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
      <title>Hello World</title>\
  </head>\
  <body>\
      <h1>Hello World</h1>\
  </body>\
  </html> ";
  server.send(200,"text/html",HTML);
}
void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passwork);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nIP地址:");
  Serial.println(WiFi.localIP());

  //用回调函数
  server.on("/", handelRoot);

  //用lambda函数
  server.on("/haha",[](){
    server.send(200,"text/html","<!DOCTYPE html>\
    <html lang=\"zh\">\
    <head>\
        <meta charset=\"UTF-8\">\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
        <title>你好啊世界</title>\
    </head>\
    <body>\
        <h1>你好啊世界</h1>\
    </body>\
    </html>\
  ");
  });

  //没有页面
  server.onNotFound([](){server.send(200,"text/html;charset=utf-8","没有找到");});

  server.begin();
  
}

void loop()
{
  server.handleClient();
}
