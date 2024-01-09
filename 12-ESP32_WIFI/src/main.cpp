#include <Arduino.h>
#include <WiFi.h>
void setup()
{
  Serial.begin(115200);
  WiFi.begin("HUAWEI-NZX_2.4G", "987654321");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("正在连接");
    delay(1000);
  }
  Serial.print("ip:");
  Serial.println(WiFi.localIP());
  
}

void loop()
{
  
}
