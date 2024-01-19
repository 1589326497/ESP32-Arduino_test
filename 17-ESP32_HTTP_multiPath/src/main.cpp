#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
//没能运行成功
const char *ssid = "HONOR-031K0S";
const char *password = "7503030516HJH";

WebServer server(80);

void handleUserProfile()
{
  String username  = server.pathArg(0);
  String profileID = server.pathArg(1);
  String response = "用户 " + username + "的资料,ID为:" + profileID;
  server.send(200, "text/html;charset=utf-8", response);
  Serial.println(response);
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("连接成功");
  Serial.print("IP 地址：");
  Serial.println(WiFi.localIP());

  server.on("/user/{}/profile/{}", handleUserProfile);
  server.begin();
  Serial.println("Web 服务器已启动");
  server.onNotFound([](){
    server.send(200, "text/html;charset=utf-8","找不到路径");
  });
}

void loop()
{
  server.handleClient();
}