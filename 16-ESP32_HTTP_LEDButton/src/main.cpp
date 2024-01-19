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
//页面绘制
void handelRoot()
{
  String HTML =R"(
    <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LED按钮</title>
</head>
<body>
    <button onmousedown=sw('on')>开灯</button>
    <button onmousedown=sw('off')>关灯</button>
    <script>
        let xhttp=new XMLHttpRequest();
        function sw(arg){
            xhttp.open('GET','/sw?led='+arg,false);
            xhttp.send()
        }
    </script>
</body>
</html>
  )";
  server.send(200,"text/html",HTML);
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
  server.on("/", handelRoot);

  //没有页面
  server.onNotFound([](){server.send(200,"text/html;charset=utf-8","没有找到");});
  
  server.begin();
}

void loop()
{
  server.handleClient();
}
