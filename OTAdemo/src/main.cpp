#include <Arduino.h>
#include <TFT_eSPI.h>
#include <math.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <Update.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ESPmDNS.h>

// #include <ArduinoMDNS.h> // 引入mDNS库

const char* APssid = "ChatChatty"; // AP的名称
const char* APpassword = "123456789"; // AP的密码

IPAddress local_IP(192, 168, 4, 1); // 静态IP地址
IPAddress gateway(192, 168, 4, 1); // 网关IP地址

const char* deviceName = "ChatChatty"; // 设置设备的名称

// // Wi-Fi 网络配置
// const char* STAssid = "OnePlus Ace 2";
// const char* STApassword = "123456789";

// // 百度 API 鉴权配置
// const char* client_id = "052MvpYA09Ova9V8ovGobCbD";
// const char* client_secret = "pgKzq06IBHWvJJ5iFNk5oYMHetmp8ow7";

//MiniMax API密钥和id
// const char *mini_apiKey = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJHcm91cE5hbWUiOiLlroHlrZDluIwiLCJVc2VyTmFtZSI6IuWugeWtkOW4jCIsIkFjY291bnQiOiIiLCJTdWJqZWN0SUQiOiIxNzkzMjY2NTE2OTU5OTY5NzgwIiwiUGhvbmUiOiIxODYzNjI0NDE2NSIsIkdyb3VwSUQiOiIxNzkzMjY2NTE2OTUxNTgxMTcyIiwiUGFnZU5hbWUiOiIiLCJNYWlsIjoiIiwiQ3JlYXRlVGltZSI6IjIwMjQtMDYtMDYgMTY6NTI6MzIiLCJpc3MiOiJtaW5pbWF4In0.eKXSUQXmEWQwygwlG0pdoEvlw-G0SgThA_43P87be_a42a9rGw8GASVldEaFMSycczpdtpPoad4zObxGpY-EBC-naGJWQWSvMthN7GQ8kDqVjebJ3Vi2aU37rsd1itjrDtXSP30wXAE9y4moNuFoR4LgccPPIZZj-DMFx6Rg1aXh6niNpinKPFWjLwe9iizh1x8Yu6nkI-FO2_Q_cvZexE1pl6THGCQR68Wapy01hHzdXej1BhYZxTKb_AY70tSYE8GkUH3tY3d6AfcmW81U98Z0jQdnbjfpR5g5tfUiXGeJI8voJimeAacqkc-FIY-SjkFvofgjTUb1MAFMW0YwfA";
// const char *tts_id="1793266516951581172";


// ----------------------NVS 键名-------------------------------
const char* nvs_namespace = "config";
// // Wi-Fi 网络配置
const char* key_stassid = "STAssid";
const char* key_stapassword = "STApassword";
// minimax 配置
const char* key_minikey = "mini_apiKey";
const char* key_ttsid = "tts_id";

// 百度 API 鉴权配置
const char* key_clientid = "client_id";
const char* key_clientsecret = "client_secret";

//音色
const char* key_voice_id = "voice_id";
String voice_id;

// ----------------------函数声明-------------------------------
Preferences preferences;  //实例化Preferences 对象
String getAccessToken( String client_id, String client_secret);
bool connectToWiFi();
void performOTAUpdate();
void startConfigServer();
void setupToneConfig();
void serverTask(void *pvParameters);
//实例对象
WebServer server(80);
TFT_eSPI tft;

//-----------------------HTML 表单页面-----------------------------
//根目录表单
const char* rootPage = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ChatChatty：你的聊天伙伴</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            color: #333;
            text-align: center;
            margin: 20px;
        }
        h1 {
            color: #333;
            font-size: 2em;
            text-shadow: 0px 0px 5px rgba(0,0,0,0.3);
        }
        p {
            font-size: 1.2em;
            margin: 20px;
        }
        button {
            padding: 10px 20px;
            font-size: 18px;
            margin: 10px;
            border: none;
            border-radius: 5px;
            box-shadow: 0px 0px 5px rgba(0,0,0,0.3);
            transition: all 0.3s ease;
        }
        button:hover {
            box-shadow: 0px 0px 10px rgba(0,0,0,0.5);
            transform: scale(1.05);
        }
        @media (max-width: 600px) {
            h1 {
                font-size: 1.5em;
            }
            p {
                font-size: 1em;
            }
            button {
                padding: 5px 10px;
                font-size: 16px;
            }
        }
    </style>
</head>
<body>
    <h1>ChatChatty 等待你的发现</h1>
    <p>ChatChatty，一款基于 ESP32 的 AI 聊天平台，利用 MiniMax AI 技术，带来富有情感韵律的语音体验。我们致力于让聊天更生动、更愉快。欢迎来体验，感受 AI 聊天的独特魅力！</p>
    <button onclick="window.location='/update'">固件更新</button>
    <button onclick="window.location='/config'">配置页面</button>
    <button onclick="window.location='/tone'">音色配置</button>
    <p style="font-size: 0.8em; color: #888;">
        版本 v1.0 本项目由宁子希创建并维护，遵循 GPLv3 开源协议。未经作者明确许可，不得用于商业用途。所有权利保留。
    </p>
</body>
</html>


)rawliteral";

//音色配置表单
const char* tonePage = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ChatChatty 音色配置</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            color: #333;
            text-align: center;
            margin: 20px;
        }
        h1 {
            color: #333;
            font-size: 2em;
            text-shadow: 0px 0px 5px rgba(0,0,0,0.3);
        }
        form {
            display: inline-block;
            margin-top: 20px;
        }
        select {
            display: block;
            margin: 10px auto;
            padding: 10px;
            width: 80%;
            border: none;
            border-radius: 5px;
            box-shadow: 0px 0px 5px rgba(0,0,0,0.3);
        }
        input[type=submit] {
            padding: 10px 20px;
            font-size: 18px;
            margin: 10px;
            border: none;
            border-radius: 5px;
            box-shadow: 0px 0px 5px rgba(0,0,0,0.3);
            transition: all 0.3s ease;
        }
        input[type=submit]:hover {
            box-shadow: 0px 0px 10px rgba(0,0,0,0.5);
            transform: scale(1.05);
        }
        @media (max-width: 600px) {
            h1 {
                font-size: 1.5em;
            }
            select {
                width: 90%;
            }
            input[type=submit] {
                padding: 5px 10px;
                font-size: 16px;
            }
        }
    </style>
</head>
<body>
    <h1>ChatChatty 音色配置</h1>
    <form action="/tone" method="post">
        <select name="voice_id">
            <!-- 选项内容 -->
            <option value="male-qn-qingse">青涩青年音色</option>
            <option value="male-qn-jingying">精英青年音色</option>
            <option value="male-qn-badao">霸道青年音色</option>
            <option value="male-qn-daxuesheng">青年大学生音色</option>
            <option value="female-shaonv">少女音色</option>
            <option value="female-yujie">御姐音色</option>
            <option value="female-chengshu">成熟女性音色</option>
            <option value="female-tianmei">甜美女性音色</option>
            <option value="presenter_male">男性主持人</option>
            <option value="presenter_female">女性主持人</option>
            <option value="audiobook_male_1">男性有声书1</option>
            <option value="audiobook_male_2">男性有声书2</option>
            <option value="audiobook_female_1">女性有声书1</option>
            <option value="audiobook_female_2">女性有声书2</option>
            <option value="male-qn-qingse-jingpin">青涩青年音色-beta</option>
            <option value="male-qn-jingying-jingpin">精英青年音色-beta</option>
            <option value="male-qn-badao-jingpin">霸道青年音色-beta</option>
            <option value="male-qn-daxuesheng-jingpin">青年大学生音色-beta</option>
            <option value="female-shaonv-jingpin">少女音色-beta</option>
            <option value="female-yujie-jingpin">御姐音色-beta</option>
            <option value="female-chengshu-jingpin">成熟女性音色-beta</option>
            <option value="female-tianmei-jingpin">甜美女性音色-beta</option>             
        </select>
        <input type="submit" value="保存音色">
    </form>
</body>
</html>

)rawliteral";

//配置页面表单
const char* configPage = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ChatChatty 配置页面</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            color: #333;
            text-align: center;
            margin: 20px;
        }
        h1 {
            color: #333;
            font-size: 2em;
            text-shadow: 0px 0px 5px rgba(0,0,0,0.3);
        }
        form {
            display: inline-block;
            margin-top: 20px;
        }
        input[type=text], input[type=password] {
            display: block;
            margin: 10px auto;
            padding: 10px;
            width: 80%;
            border: none;
            border-radius: 5px;
            box-shadow: 0px 0px 5px rgba(0,0,0,0.3);
        }
        input[type=submit] {
            padding: 10px 20px;
            font-size: 18px;
            margin: 10px;
            border: none;
            border-radius: 5px;
            box-shadow: 0px 0px 5px rgba(0,0,0,0.3);
            transition: all 0.3s ease;
        }
        input[type=submit]:hover {
            box-shadow: 0px 0px 10px rgba(0,0,0,0.5);
            transform: scale(1.05);
        }
        @media (max-width: 600px) {
            h1 {
                font-size: 1.5em;
            }
            input[type=text], input[type=password] {
                width: 90%;
            }
            input[type=submit] {
                padding: 5px 10px;
                font-size: 16px;
            }
        }
    </style>
</head>
<body>
    <h1>ChatChatty 配置页面</h1>
    <form action="/config" method="post">
        <label for="stassid">Wi-Fi SSID：</label>
        <input type="text" name="stassid" id="stassid" placeholder="请输入 Wi-Fi SSID">
        
        <label for="stapassword">Wi-Fi 密码：</label>
        <input type="password" name="stapassword" id="stapassword" placeholder="请输入 Wi-Fi 密码">
        
    
        <label for="tts_id">Mini ID：</label>
        <input type="text" name="tts_id" id="tts_id" placeholder="请输入 Mini ID">
    
        <label for="mini_apiKey">Mini API 密钥：</label>
        <input type="text" name="mini_apiKey" id="mini_apiKey" placeholder="请输入 Mini API 密钥">
    
        
        <label for="client_id">百度TTS ID：</label>
        <input type="text" name="client_id" id="client_id" placeholder="请输入客户端 ID">
        
        <label for="client_secret">百度TTS 密钥：</label>
        <input type="text" name="client_secret" id="client_secret" placeholder="请输入客户端密钥">
        
        <input type="submit" value="保存配置">
    </form>
</body>
</html>
)rawliteral";



//OTA表单
const char* updateIndex =R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ChatChatty OTA 更新</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            color: #333;
            text-align: center;
            margin: 20px;
        }
        h1 {
            color: #333;
            font-size: 2em;
            text-shadow: 0px 0px 5px rgba(0,0,0,0.3);
        }
        form {
            display: inline-block;
            margin-top: 20px;
        }
        .custom-file-upload {
            display: block;
            margin: 10px auto;
            padding: 10px;
            width: 80%;
            border: none;
            border-radius: 5px;
            box-shadow: 0px 0px 5px rgba(0,0,0,0.3);
            transition: all 0.3s ease;
            text-align: center;
            background-color: #f0f0f0;
            cursor: pointer;
        }
        .custom-file-upload:hover {
            box-shadow: 0px 0px 10px rgba(0,0,0,0.5);
            transform: scale(1.05);
        }
        input[type=file] {
            display: none;
        }
        input[type=submit] {
            padding: 10px 20px;
            font-size: 18px;
            margin: 10px;
            border: none;
            border-radius: 5px;
            box-shadow: 0px 0px 5px rgba(0,0,0,0.3);
            transition: all 0.3s ease;
        }
        input[type=submit]:hover {
            box-shadow: 0px 0px 10px rgba(0,0,0,0.5);
            transform: scale(1.05);
        }
        @media (max-width: 600px) {
            h1 {
                font-size: 1.5em;
            }
            .custom-file-upload {
                width: 90%;
            }
            input[type=submit] {
                padding: 5px 10px;
                font-size: 16px;
            }
        }
    </style>
    <script>
        window.onload = function() {
            var fileInput = document.getElementById('file-upload');
            var fileDisplayArea = document.getElementById('file-display-area');

            fileInput.addEventListener('change', function(e) {
                var file = fileInput.files[0];
                fileDisplayArea.innerText = file.name;
            });
        }
    </script>
</head>
<body>
    <h1>欢迎使用 ChatChatty OTA 更新</h1>
    <form method='POST' action='/update' enctype='multipart/form-data'>
        <label for="file-upload" class="custom-file-upload">
            选择文件
        </label>
        <input id="file-upload" type='file' name='update' accept='.bin'>
        <span id="file-display-area">还未选择</span>
        <input type='submit' value='上传固件'>
    </form>
</body>
</html>

)rawliteral";



// void handleRoot() {
//   server.sendHeader("Location", "/update");
//   server.send(302, "text/plain", "");
// }


void setup() {
  Serial.begin(115200); // 初始化串口，波特率为115200

  // 将 ESP32 设置为 AP 模式并指定静态 IP 地址
  WiFi.softAP(APssid, APpassword);
  WiFi.softAPConfig(local_IP, gateway, IPAddress(255, 255, 255, 0));
  
  Serial.println("AP 模式启动");
  Serial.print("IP 地址: ");
  Serial.println(WiFi.softAPIP()); // 打印 ESP32 的 AP IP 地址
  Serial.println("Booting...");

  // 设置mDNS服务
  if (!MDNS.begin(deviceName)) {
    Serial.println("设置MDNS响应器错误!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS响应器启动");

  // 设置根目录的处理函数
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", rootPage);
  });

  //OTA页表启动
  performOTAUpdate();

  //配置页表启动
  startConfigServer();

  //音色页表启动
  setupToneConfig();

  server.begin();
  Serial.println("HTTP 服务器已打开");

  // 设置mDNS服务来解析设备名称到IP地址
  MDNS.addService("http", "tcp", 80);

  //单独任务处理web
  xTaskCreatePinnedToCore(
    serverTask,          
    "serverTask",        
    1024*20,               
    NULL,                
    1,                   
    NULL,                
    0                    
  );


  // 尝试连接到 Wi-Fi
  connectToWiFi();

  // 等待Wi-Fi连接完成
  Serial.println("等待Wi-Fi连接");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // 获取访问令牌
  preferences.begin(nvs_namespace, true);
  String client_id = preferences.getString(key_clientid, "");
  String client_secret = preferences.getString(key_clientsecret, "");
  preferences.end();

  String accessToken = getAccessToken(client_id,client_secret);

  if (accessToken != "") {
    Serial.print("访问令牌: ");
    Serial.println(accessToken);
  } else {
    Serial.println("获取访问令牌失败");
  }

  // 从NVS读取voice_id
  preferences.begin(nvs_namespace, true);
  voice_id = preferences.getString(key_voice_id, "default_voice");
  preferences.end();
  Serial.print("当前语音id: ");
  Serial.println(voice_id);


  //程序逻辑 版本1.1
  Serial.println();
  Serial.println("NEW ESP32C3!!");
  tft.begin();
  tft.setRotation(3);
  tft.setTextFont(2);
  tft.fillScreen(TFT_BLACK);
  tft.drawString("2.0 NEW ESP32C3!!", 0, 0);
  tft.drawRect(2, 20, 100, 20, TFT_BROWN);
}

void loop() {
  server.handleClient();
}
//处理来自客户端的 HTTP 请求
void serverTask(void *pvParameters) {
  for (;;) {
    server.handleClient(); // 处理客户端请求
    // MDNS.update(); // 更新mDNS服务
    vTaskDelay(10); // 留出时间给其他任务执行
  }
}

//----------------------百度TTS本地鉴权---------------------------------
String getAccessToken( String client_id, String client_secret) {

  // 动态生成获取访问令牌的 URL
  String token_url = "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=" + client_id + "&client_secret=" + client_secret;
  Serial.println("----------------百度TTS本地鉴权开始------------");
  // 创建 HTTP 客户端
  HTTPClient http;
  http.begin(token_url);

  int httpResponseCode = http.POST("");
  String accessToken = "";

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("访问令牌 JSON:");
    Serial.println(response);


    // 解析 JSON 响应以获取访问令牌
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);
    accessToken = doc["access_token"].as<String>();

  } else {
    Serial.print("发送POST错误: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  return accessToken;
}

//--------------------------连接WIFI------------------------
bool connectToWiFi() {

  preferences.begin(nvs_namespace, true);
  String stassid = preferences.getString(key_stassid, "");
  String stapassword = preferences.getString(key_stapassword, "");
  preferences.end();

  if (stassid.length() > 0 && stapassword.length() > 0) {
    WiFi.begin(stassid.c_str(), stapassword.c_str());
    Serial.println("连接到 Wi-Fi...");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println(" 已连接");
    Serial.print("IP 地址: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("没有找到 Wi-Fi 配置，请在配置页面检查Wi-Fi信息是否输入正确");
    return false;
  }
}


// -------------------------------OTA方法--------------------------------------
void performOTAUpdate(){
    // 设置服务器处理函数
 // server.on("/", HTTP_GET, handleRoot); // 根路由重定向到 OTA 页面

  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateIndex);
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");

    //动态显示结果
  String message = Update.hasError() ? "更新失败" : "更新成功。重新启动…";
  server.sendHeader("Content-Type", "text/html; charset=utf-8");
  server.send(200, "text/html", "<span style='font-size: 36px;'>" + message + "</span>");


    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload(); //用于处理上传的文件数据
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // 以最大可用大小开始
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      // 将接收到的数据写入Update对象
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { // 设置大小为当前大小
        Serial.printf("更新成功 : %u bytes\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
}

//-----------------------配置各参数----------------------------
void startConfigServer() {

  // 设置根路径 "/config" 的处理函数
  server.on("/config", HTTP_GET, []() {
    server.send(200, "text/html", configPage);
  });

  // 设置 "/config" 路径的处理函数
  server.on("/config", HTTP_POST, []() {
    String stassid = server.arg("stassid");
    String stapassword = server.arg("stapassword");
    String mini_apiKey = server.arg("mini_apiKey");
    String tts_id = server.arg("tts_id");
    String client_id = server.arg("client_id");
    String client_secret = server.arg("client_secret");

    // 保存到 NVS
    Serial.println("正在保存信息到NVS");
    Serial.println("空余nvs空间: ");
    Serial.print(preferences.freeEntries());//获取空余nvs空间
    Serial.println();

    preferences.begin(nvs_namespace, false);
    preferences.putString(key_stassid, stassid);
    preferences.putString(key_stapassword, stapassword);
    preferences.putString(key_minikey, mini_apiKey);
    preferences.putString(key_ttsid, tts_id);
    preferences.putString(key_clientid, client_id);
    preferences.putString(key_clientsecret, client_secret);
    preferences.end();

    Serial.println("配置已保存，重启设备... ");
    server.sendHeader("Content-Type", "text/html; charset=utf-8");
    server.send(200, "text/plain", "<span style='font-size: 36px;'> 配置已保存，重启设备... </span>");
    delay(2000);
    ESP.restart();
  });


}
// ---------------------------音色配置功能的函数--------------------------------------
void setupToneConfig() {
  // 显示音色配置页面的处理函数
  server.on("/tone", HTTP_GET, []() {
    server.send(200, "text/html", tonePage);
  });

  // 保存音色配置的处理函数
  server.on("/tone", HTTP_POST, []() {
    if (server.hasArg("voice_id")) {
      String voice_id = server.arg("voice_id");

      // 保存到 NVS
      preferences.begin(nvs_namespace, false);
      preferences.putString(key_voice_id, voice_id);
      preferences.end();

      server.sendHeader("Content-Type", "text/html; charset=utf-8");
      server.send(200, "text/plain", "<span style='font-size: 36px;'> 音色配置已保存。 </span>");
      Serial.print("New voice_id: ");
      Serial.println(voice_id);
    } else {
      server.sendHeader("Content-Type", "text/html; charset=utf-8");
      server.send(400, "text/plain", "<span style='font-size: 36px;'> 缺少配置参数 </span>");
    }
  });
}

