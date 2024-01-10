#include <Arduino.h>
#include "WiFi.h"

const char* ssid="ESP32AP";
const char* password="123456789";
void scanNetworks(){						//扫描周边wifi网络，并显示wifi数量，打印它们的属性（ssid加密方式，mac地址）
  int numberOfNetworks= WiFi.scanNetworks();
  Serial.print("The number of networks found is:");
  Serial.println(numberOfNetworks);
  for(int i=0;i<numberOfNetworks;i++){
    Serial.print("Networkname: ");
    Serial.println(WiFi.SSID(i));
    Serial.print("Signalstrength: ");
    Serial.println(WiFi.RSSI(i));
    Serial.print("MACaddress: ");
    Serial.println(WiFi.BSSIDstr(i));
    Serial.println("-----------------------");
  }
}
void setup() {
 Serial.begin(115200);
 WiFi.softAP(ssid,password);
 scanNetworks();
 Serial.print("\n WiFi接入点ip:");
 Serial.println(WiFi.softAPIP());
}
  
void loop() {
  
}

