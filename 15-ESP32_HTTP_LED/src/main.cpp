#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#define LEDPIN 2

const char *ssid = "HONOR-031K0S";
const char *password = "7503030516HJH";

WebServer server(80);

// LED开关
void ledSwitch()
{
  String state = server.arg("led");
  if (state == "on")
  {
    digitalWrite(LEDPIN, HIGH);
  }
  else if (state == "off")
  {
    digitalWrite(LEDPIN, LOW);
  }
  server.send(200,"text/html;charset=utf-8","<h1>LED IS "+state+"</h1>");
}
void setup()
{

  Serial.begin(115200);
  // 初始化GPIO
  pinMode(LEDPIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // 等待连接
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.print("\nIP地址:");
  // 获取ip地址
  Serial.println(WiFi.localIP());

  server.on("/sw", ledSwitch);

  //没有页面
  server.onNotFound([](){server.send(200,"text/html;charset=utf-8","没有找到");});
  
  server.begin();
}

void loop()
{
  server.handleClient();
}
