#include <Arduino.h>
#include "WiFi.h"

const char* ssid="ESP32AP";
const char* password="123456789";

void setup() {
 Serial.begin(115200);
 WiFi.softAP(ssid,password);
 
 Serial.print("\n WiFi接入点ip:");
 Serial.println(WiFi.softAPIP());
}
  
void loop() {
  
}

