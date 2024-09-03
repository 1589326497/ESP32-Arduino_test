#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// 1. 替换网络凭据
const char* ssid = "HUAWEI-NZX_2.4G";
const char* password = "987654321";

// 2. 替换AI API密钥
const char* apiKey = "sk-10b0ed5ee463463bbca37d12727826e0";

// 向AI API发送请求
String inputText = "你好，通义千问！";
String apiUrl = "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation";

String answer;

String getGPTAnswer(String inputText) {
  HTTPClient http;
  http.setTimeout(10000);
  http.begin(apiUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String(apiKey));
  String payload = "{\"model\":\"qwen-turbo\",\"input\":{\"messages\":[{\"role\": \"system\",\"content\": \"你是鹏鹏的生活助手机器人，要求下面的回答严格控制在256字符以内。\"},{\"role\": \"user\",\"content\": \"" + inputText + "\"}]}}";
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode == 200) {
    String response = http.getString();
    http.end();
    Serial.println(response);

    // 解析JSON响应
    DynamicJsonDocument jsonDoc(1024);
    deserializeJson(jsonDoc, response);
    String outputText = jsonDoc["output"]["text"];
    return outputText;
    // Serial.println(outputText);
  } else {
    http.end();
    Serial.printf("Error %i \n", httpResponseCode);
    return "<error>";
  }
}

void setup() {
  // 初始化字符串
  Serial.begin(115200);

  // 连接Wi-Fi网络
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("链接wifi.....");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  answer = getGPTAnswer(inputText);
  Serial.println("回答: " + answer);
  Serial.println("输入提示:");

}

void loop() {
  
  if (Serial.available()) {
    inputText = Serial.readStringUntil('\n');
    // inputText.trim();
    Serial.println("\n 输入:"+inputText);

    answer = getGPTAnswer(inputText);
    Serial.println("回答: " + answer);
    Serial.println("输入提示符:");
  }
  // delay(2);
}


